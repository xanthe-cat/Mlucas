/*******************************************************************************
*                                                                              *
*   (C) 1997-2013 by Ernst W. Mayer.                                           *
*                                                                              *
*  This program is free software; you can redistribute it and/or modify it     *
*  under the terms of the GNU General Public License as published by the       *
*  Free Software Foundation; either version 2 of the License, or (at your      *
*  option) any later version.                                                  *
*                                                                              *
*  This program is distributed in the hope that it will be useful, but WITHOUT *
*  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
*  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for   *
*  more details.                                                               *
*                                                                              *
*  You should have received a copy of the GNU General Public License along     *
*  with this program; see the file GPL.txt.  If not, you may view one at       *
*  http://www.fsf.org/licenses/licenses.html, or obtain one by writing to the  *
*  Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA     *
*  02111-1307, USA.                                                            *
*                                                                              *
*******************************************************************************/

/*******************************************************************************
   We now include this header file if it was not included before.
*******************************************************************************/
#ifndef sse2_macro_gcc_h_included
#define sse2_macro_gcc_h_included

/*
SSE2-ified version of PAIR_SQUARE_4. Data enter in [__tAr, ~tAr], [__tAi, ~tAi], pairs, where the imaginary part
of each input pair is assumed offset +0x10 in memory from the real part, i.e. needs no explicit pointer reference.

For the sincos twiddles: using the notation of the scalar PAIR_SQUARE_4() macro,"__c" means [c0,s1], "__s" means [s0,c1].
For these, due to the buterfly indexing pattern, we cannot assume that __s = __c + 0x10, so feed both pointers explicitly.

We use shufpd xmm, xmm, 1 to swap lo and hi doubles of an xmm register for the various operations with one swapped input.
*/

#ifdef USE_AVX

  // FMA-based versions of selected macros in this file for Intel AVX2/FMA3
  #ifdef USE_AVX2

	/* Complex multiply of 2 roots of unity - use e.g. for "multiply up" of sincos twiddles. */
	#define SSE2_CMUL_EXPO(XcA,XcB,XcAmB,XcApB)\
	{\
	__asm__ volatile (\
		"movq	%[__cA]		,%%rax\n\t"\
		"movq	%[__cB]		,%%rbx\n\t"\
		"movq	%[__cAmB]	,%%rcx\n\t"\
		"movq	%[__cApB]	,%%rdx\n\t"\
		"\n\t"\
		"vmovaps	    (%%rax),%%ymm0\n\t"\
		"vmovaps	0x20(%%rax),%%ymm2\n\t"\
		"vmovaps	    (%%rbx),%%ymm4\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm5\n\t"\
		"vmovaps	%%ymm0,%%ymm1\n\t"\
		"vmovaps	%%ymm2,%%ymm3\n\t"\
		"\n\t"\
		"vmulpd	%%ymm4,%%ymm0,%%ymm0\n\t"\
		"vmulpd	%%ymm5,%%ymm1,%%ymm1\n\t"\
		"vmovaps	%%ymm0,%%ymm6\n\t"\
		"vmovaps	%%ymm1,%%ymm7\n\t"\
	" vfmadd231pd	%%ymm5,%%ymm3,%%ymm0	\n\t"\
	"vfnmadd231pd	%%ymm4,%%ymm2,%%ymm1	\n\t"\
	"vfnmadd231pd	%%ymm5,%%ymm3,%%ymm6	\n\t"\
	" vfmadd231pd	%%ymm4,%%ymm2,%%ymm7	\n\t"\
		"vmovaps	%%ymm0,    (%%rcx)\n\t"\
		"vmovaps	%%ymm1,0x20(%%rcx)\n\t"\
		"vmovaps	%%ymm6,    (%%rdx)\n\t"\
		"vmovaps	%%ymm7,0x20(%%rdx)\n\t"\
		:					/* outputs: none */\
		: [__cA]  "m" (XcA)	/* All inputs from memory addresses here */\
		 ,[__cB]  "m" (XcB)\
		 ,[__cAmB] "m" (XcAmB)\
		 ,[__cApB] "m" (XcApB)\
		: "cc","memory","rax","rbx","rcx","rdx","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

	#define SSE2_RADIX_03_DFT_X2(Xcc0, Xi0,Xi1,Xi2, Xo0,Xo1,Xo2, Xj0,Xj1,Xj2, Xu0,Xu1,Xu2)\
	{\
	__asm__ volatile (\
		"movq	%[__i0],%%rax	\n\t	movq	%[__j0],%%r10	\n\t"/*	movq	$0x3ff0000000000000, %%rdx	\n\t@// Int64 form of double-float 1.0 */\
		"movq	%[__i1],%%rbx	\n\t	movq	%[__j1],%%r11	\n\t"/*	vmovq	%%rdx, %%xmm15	\n\t@// Load into bottom half of xmm */\
		"movq	%[__i2],%%rcx	\n\t	movq	%[__j2],%%r12	\n\t"/*	vpbroadcastq	%%xmm15,%%ymm15	\n\t@// Broadcast lo half of xmm to all 4 64-bit slots of ymm */\
		"movq	%[__cc0],%%rdx	\n\t							\n\t"/* ewm: My version of gcc/asm gives assembler error: 'no such instruction' ... either update tools or use alternate */\
		"vmovaps	    (%%rbx),%%ymm2		\n\t	vmovaps	    (%%r11),%%ymm10	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3		\n\t	vmovaps	0x20(%%r11),%%ymm11	\n\t"\
		"vmovaps	    (%%rax),%%ymm0		\n\t	vmovaps	    (%%r10),%%ymm8 	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1		\n\t	vmovaps	0x20(%%r10),%%ymm9 	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6		\n\t	vmovaps	    (%%r12),%%ymm14	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7		\n\t	vmovaps	0x20(%%r12),%%ymm15	\n\t"\
		"vmovaps	%%ymm2,%%ymm4			\n\t	vmovaps	%%ymm10,%%ymm12		\n\t"\
		"vmovaps	%%ymm3,%%ymm5			\n\t	vmovaps	%%ymm11,%%ymm13		\n\t"\
		"movq	%[__o0],%%rax				\n\t	movq	%[__u0],%%r10		\n\t"\
		"movq	%[__o1],%%rbx				\n\t	movq	%[__u1],%%r11		\n\t"\
		"movq	%[__o2],%%rcx				\n\t	movq	%[__u2],%%r12		\n\t"\
		"vaddpd	%%ymm6,%%ymm2,%%ymm2		\n\t	vaddpd	%%ymm14,%%ymm10,%%ymm10		\n\t"\
		"vaddpd	%%ymm7,%%ymm3,%%ymm3		\n\t	vaddpd	%%ymm15,%%ymm11,%%ymm11		\n\t"\
		"vsubpd	%%ymm6,%%ymm4,%%ymm4		\n\t	vsubpd	%%ymm14,%%ymm12,%%ymm12		\n\t"\
		"vsubpd	%%ymm7,%%ymm5,%%ymm5		\n\t	vsubpd	%%ymm15,%%ymm13,%%ymm13		\n\t"\
		"vaddpd	%%ymm2,%%ymm0,%%ymm0		\n\t	vaddpd	%%ymm10,%%ymm8 ,%%ymm8 		\n\t"\
		"vaddpd	%%ymm3,%%ymm1,%%ymm1		\n\t	vaddpd	%%ymm11,%%ymm9 ,%%ymm9 		\n\t"\
		"vmovaps	    (%%rdx),%%ymm6		\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7		\n\t"\
		"vmovaps	%%ymm0,    (%%rax)		\n\t	vmovaps	%%ymm8 ,    (%%r10)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rax)		\n\t	vmovaps	%%ymm9 ,0x20(%%r10)	\n\t"\
	" vfmadd132pd	%%ymm6,%%ymm0,%%ymm2 	\n\t  vfmadd132pd	%%ymm6,%%ymm8,%%ymm10	\n\t"\
	" vfmadd132pd	%%ymm6,%%ymm1,%%ymm3 	\n\t  vfmadd132pd	%%ymm6,%%ymm9,%%ymm11	\n\t"\
		"vmovaps	%%ymm2,%%ymm0			\n\t	vmovaps	%%ymm10,%%ymm8 		\n\t"\
		"vmovaps	%%ymm3,%%ymm1			\n\t	vmovaps	%%ymm11,%%ymm9 		\n\t"\
	" vfmadd231pd	%%ymm7,%%ymm5,%%ymm0 	\n\t  vfmadd231pd	%%ymm7,%%ymm13,%%ymm8 	\n\t"\
	"vfnmadd231pd	%%ymm7,%%ymm4,%%ymm1 	\n\t vfnmadd231pd	%%ymm7,%%ymm12,%%ymm9 	\n\t"\
	"vfnmadd231pd	%%ymm7,%%ymm5,%%ymm2 	\n\t vfnmadd231pd	%%ymm7,%%ymm13,%%ymm10	\n\t"\
	" vfmadd231pd	%%ymm7,%%ymm4,%%ymm3 	\n\t  vfmadd231pd	%%ymm7,%%ymm12,%%ymm11	\n\t"\
		"vmovaps	%%ymm0,    (%%rcx)		\n\t	vmovaps	%%ymm8 ,    (%%r12)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rcx)		\n\t	vmovaps	%%ymm9 ,0x20(%%r12)	\n\t"\
		"vmovaps	%%ymm2,    (%%rbx)		\n\t	vmovaps	%%ymm10,    (%%r11)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rbx)		\n\t	vmovaps	%%ymm11,0x20(%%r11)	\n\t"\
		:					/* outputs: none */\
		: [__cc0] "m" (Xcc0)	/* All inputs from memory addresses here */\
		 ,[__i0] "m" (Xi0)\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__j0] "m" (Xj0)\
		 ,[__j1] "m" (Xj1)\
		 ,[__j2] "m" (Xj2)\
		 ,[__u0] "m" (Xu0)\
		 ,[__u1] "m" (Xu1)\
		 ,[__u2] "m" (Xu2)\
		: "cc","memory","rax","rbx","rcx","rdx","r10","r11","r12","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

	#define SSE2_RADIX_05_DFT_0TWIDDLE_X2(Xcc1,Xtwo, Xi0,Xi1,Xi2,Xi3,Xi4, Xo0,Xo1,Xo2,Xo3,Xo4, Xj0,Xj1,Xj2,Xj3,Xj4, Xu0,Xu1,Xu2,Xu3,Xu4)\
	{\
	__asm__ volatile (\
		"movq	%[__i1],%%rax				\n\t	movq	%[__j1],%%r11		\n\t"\
		"movq	%[__i4],%%rdx				\n\t	movq	%[__j4],%%r14		\n\t"\
		"vmovaps	    (%%rax),%%ymm0		\n\t	vmovaps	    (%%r11),%%ymm8 	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1		\n\t	vmovaps	0x20(%%r11),%%ymm9 	\n\t"\
		"vmovaps	    (%%rdx),%%ymm6		\n\t	vmovaps	    (%%r14),%%ymm14	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7		\n\t	vmovaps	0x20(%%r14),%%ymm15	\n\t"\
		"movq	%[__i2],%%rbx				\n\t	movq	%[__j2],%%r12		\n\t"\
		"movq	%[__i3],%%rcx				\n\t	movq	%[__j3],%%r13		\n\t"\
		"vsubpd	%%ymm6,%%ymm0,%%ymm0		\n\t	vsubpd		%%ymm14,%%ymm8 ,%%ymm8 		\n\t"\
		"vsubpd	%%ymm7,%%ymm1,%%ymm1		\n\t	vsubpd		%%ymm15,%%ymm9 ,%%ymm9 		\n\t"\
		"vmovaps	    (%%rbx),%%ymm2		\n\t	vmovaps	    (%%r12),%%ymm10	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3		\n\t	vmovaps	0x20(%%r12),%%ymm11	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4		\n\t	vmovaps	    (%%r13),%%ymm12	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5		\n\t"/*	vmovaps	0x20(%%r13),%%ymm13	\n\t Use for two in 8 ensuing FMAs instead */\
	"movq		%[__two],%%rcx		\n\t"\
	"vmovaps	(%%rcx),%%ymm13		\n\t"/* two */\
		"vsubpd	%%ymm4,%%ymm2,%%ymm2		\n\t	vsubpd		%%ymm12,%%ymm10,%%ymm10		\n\t"\
		"vsubpd	%%ymm5,%%ymm3,%%ymm3		\n\t	vsubpd	0x20(%%r13),%%ymm11,%%ymm11		\n\t"\
	" vfmadd132pd	%%ymm13,%%ymm0,%%ymm6 	\n\t  vfmadd132pd		%%ymm13,%%ymm8,%%ymm14	\n\t"\
	" vfmadd132pd	%%ymm13,%%ymm1,%%ymm7 	\n\t  vfmadd132pd		%%ymm13,%%ymm9,%%ymm15	\n\t"\
	" vfmadd132pd	%%ymm13,%%ymm2,%%ymm4 	\n\t  vfmadd132pd		%%ymm13,%%ymm10,%%ymm12	\n\t"\
	" vfmadd132pd	%%ymm13,%%ymm3,%%ymm5 	\n\t  vfmadd132pd	0x20(%%r13),%%ymm11,%%ymm13	\n\t"\
	/*==== spill ymm2,3 here (still use once as dest of add/sub below, but pvsly carried copies of values-here around) =====*/\
	"vmovaps	%%ymm2,    (%%rbx)		\n\t	vmovaps	%%ymm10,    (%%r12)		\n\t"\
	"vmovaps	%%ymm3,0x20(%%rbx)		\n\t	vmovaps	%%ymm11,0x20(%%r12)		\n\t"\
		"movq	%[__cc1],%%rax				\n\t"\
		"vsubpd	%%ymm4,%%ymm6,%%ymm6		\n\t	vsubpd	%%ymm12,%%ymm14,%%ymm14		\n\t"\
		"vsubpd	%%ymm5,%%ymm7,%%ymm7		\n\t	vsubpd	%%ymm13,%%ymm15,%%ymm15		\n\t"\
		"movq	%[__i0],%%rsi				\n\t	movq	%[__j0],%%r10		\n\t"\
	" vfmadd132pd	(%%rcx),%%ymm6,%%ymm4 	\n\t  vfmadd132pd	(%%rcx),%%ymm14,%%ymm12	\n\t"\
	" vfmadd132pd	(%%rcx),%%ymm7,%%ymm5 	\n\t  vfmadd132pd	(%%rcx),%%ymm15,%%ymm13	\n\t"\
		"vaddpd	    (%%rsi),%%ymm4,%%ymm2	\n\t	vaddpd	    (%%r10),%%ymm12,%%ymm10	\n\t"\
		"vaddpd	0x20(%%rsi),%%ymm5,%%ymm3	\n\t	vaddpd	0x20(%%r10),%%ymm13,%%ymm11	\n\t"\
		"movq	%[__o0],%%rdi				\n\t	movq	%[__u0],%%r15		\n\t"\
		"vmovaps	%%ymm2,    (%%rdi)		\n\t	vmovaps	%%ymm10,    (%%r15)		\n\t"\
		"vmovaps	%%ymm3,0x20(%%rdi)		\n\t	vmovaps	%%ymm11,0x20(%%r15)		\n\t"\
	"vmovaps	    (%%rax),%%ymm2	\n\t"/* each of these 2 mults used 4x, so trade 8 loads for 4: */\
	"vmovaps	0x20(%%rax),%%ymm3	\n\t"/* 2 of mults, 2 to read clobbered ymm2,3 values from mem */\
		"vmulpd			%%ymm3 ,%%ymm6,%%ymm6	\n\t	vmulpd		%%ymm3,%%ymm14,%%ymm14	\n\t"\
		"vmulpd			%%ymm3 ,%%ymm7,%%ymm7	\n\t	vmulpd		%%ymm3,%%ymm15,%%ymm15	\n\t"\
	" vfmadd213pd	    (%%rdi),%%ymm2,%%ymm4 	\n\t  vfmadd132pd	%%ymm2,%%ymm10,%%ymm12	\n\t"\
	" vfmadd213pd	0x20(%%rdi),%%ymm2,%%ymm5 	\n\t  vfmadd132pd	%%ymm2,%%ymm11,%%ymm13	\n\t"\
		"vsubpd	%%ymm6,%%ymm4,%%ymm4		\n\t	vsubpd	%%ymm14,%%ymm12,%%ymm12		\n\t"\
		"vsubpd	%%ymm7,%%ymm5,%%ymm5		\n\t	vsubpd	%%ymm15,%%ymm13,%%ymm13		\n\t"\
	" vfmadd132pd	(%%rcx),%%ymm4,%%ymm6 	\n\t  vfmadd132pd	(%%rcx),%%ymm12,%%ymm14	\n\t"\
	" vfmadd132pd	(%%rcx),%%ymm5,%%ymm7 	\n\t  vfmadd132pd	(%%rcx),%%ymm13,%%ymm15	\n\t"\
		"vmovaps	%%ymm4,    (%%rsi)		\n\t	vmovaps	%%ymm12,    (%%r10)		\n\t"\
		"vmovaps	%%ymm5,0x20(%%rsi)		\n\t	vmovaps	%%ymm13,0x20(%%r10)		\n\t"\
		"vmovaps	%%ymm0,%%ymm4			\n\t	vmovaps	%%ymm8 ,%%ymm12			\n\t"\
		"vmovaps	%%ymm1,%%ymm5			\n\t	vmovaps	%%ymm9 ,%%ymm13			\n\t"\
	/*==== restore spill of ymm2,3 here =====*/\
	"vmovaps	    (%%rbx),%%ymm2		\n\t	vmovaps	    (%%r12),%%ymm10		\n\t"\
	"vmovaps	0x20(%%rbx),%%ymm3		\n\t	vmovaps	0x20(%%r12),%%ymm11		\n\t"\
		"vsubpd	%%ymm2,%%ymm0,%%ymm0		\n\t	vsubpd	%%ymm10,%%ymm8,%%ymm8 		\n\t"\
		"vsubpd	%%ymm3,%%ymm1,%%ymm1		\n\t	vsubpd	%%ymm11,%%ymm9,%%ymm9 		\n\t"\
		"vmulpd	0x40(%%rax),%%ymm0,%%ymm0	\n\t	vmulpd	0x40(%%rax),%%ymm8 ,%%ymm8 	\n\t"\
		"vmulpd	0x40(%%rax),%%ymm1,%%ymm1	\n\t	vmulpd	0x40(%%rax),%%ymm9 ,%%ymm9 	\n\t"\
	/*==== spill ymm4,5 here (still use once as mult in 2 of the 8 FMAs below, but now read those values from the spill addrs) =====*/\
	"vmovaps	%%ymm4,    (%%rbx)	\n\t"\
	"vmovaps	%%ymm5,0x20(%%rbx)	\n\t"\
	"vmovaps	0x60(%%rax),%%ymm4	\n\t"/* each of these 2 mults used 4x, so trade 8 loads for 4: */\
	"vmovaps	0x80(%%rax),%%ymm5	\n\t"/* 2 of mults, 2 to read clobbered ymm2,3 values from mem */\
	" vfmadd132pd		%%ymm4 ,%%ymm0,%%ymm2 	\n\t  vfmadd132pd	%%ymm4 ,%%ymm8 ,%%ymm10	\n\t"\
	" vfmadd132pd		%%ymm4 ,%%ymm1,%%ymm3 	\n\t  vfmadd132pd	%%ymm4 ,%%ymm9 ,%%ymm11	\n\t"\
	"vfnmadd231pd	    (%%rbx),%%ymm5,%%ymm0 	\n\t vfnmadd231pd	%%ymm5 ,%%ymm12,%%ymm8 	\n\t"\
	"vfnmadd231pd	0x20(%%rbx),%%ymm5,%%ymm1 	\n\t vfnmadd231pd	%%ymm5 ,%%ymm13,%%ymm9 	\n\t"\
		"vmovaps	    (%%rsi),%%ymm4		\n\t	vmovaps	    (%%r10),%%ymm12		\n\t"\
		"vmovaps	0x20(%%rsi),%%ymm5		\n\t	vmovaps	0x20(%%r10),%%ymm13		\n\t"\
		"movq	%[__o1],%%rax				\n\t	movq	%[__u1],%%r11			\n\t"\
		"movq	%[__o4],%%rdx				\n\t	movq	%[__u4],%%r14			\n\t"\
		"vsubpd	%%ymm3,%%ymm6,%%ymm6		\n\t	vsubpd	%%ymm11,%%ymm14,%%ymm14		\n\t"\
		"vsubpd	%%ymm2,%%ymm7,%%ymm7		\n\t	vsubpd	%%ymm10,%%ymm15,%%ymm15		\n\t"\
		"vmovaps	%%ymm6,    (%%rax)		\n\t	vmovaps	%%ymm14,    (%%r11)		\n\t"\
		"vmovaps	%%ymm7,0x20(%%rdx)		\n\t	vmovaps	%%ymm15,0x20(%%r14)		\n\t"\
	"vmovaps	(%%rcx),%%ymm6		\n\t"/* two ... ymm6 still used once below, but now read that value from above (%%rax) write-address */\
	" vfmadd213pd	(%%rax),%%ymm6,%%ymm3 	\n\t  vfmadd132pd	%%ymm6 ,%%ymm14,%%ymm11	\n\t"\
	" vfmadd132pd	%%ymm6 ,%%ymm7,%%ymm2 	\n\t  vfmadd132pd	%%ymm6 ,%%ymm15,%%ymm10	\n\t"\
		"vmovaps	%%ymm3,    (%%rdx)		\n\t	vmovaps	%%ymm11,    (%%r14)		\n\t"\
		"vmovaps	%%ymm2,0x20(%%rax)		\n\t	vmovaps	%%ymm10,0x20(%%r11)		\n\t"\
		"movq	%[__o2],%%rbx				\n\t	movq	%[__u2],%%r12			\n\t"\
		"movq	%[__o3],%%rcx				\n\t	movq	%[__u3],%%r13			\n\t"\
		"vsubpd	%%ymm1,%%ymm4,%%ymm4		\n\t	vsubpd	%%ymm9 ,%%ymm12,%%ymm12		\n\t"\
		"vsubpd	%%ymm0,%%ymm5,%%ymm5		\n\t	vsubpd	%%ymm8 ,%%ymm13,%%ymm13		\n\t"\
		"vmovaps	%%ymm4,    (%%rbx)		\n\t	vmovaps	%%ymm12,    (%%r12)		\n\t"\
		"vmovaps	%%ymm5,0x20(%%rcx)		\n\t	vmovaps	%%ymm13,0x20(%%r13)		\n\t"\
	" vfmadd132pd	%%ymm6 ,%%ymm4,%%ymm1 	\n\t  vfmadd132pd	%%ymm6 ,%%ymm12,%%ymm9	\n\t"\
	" vfmadd132pd	%%ymm6 ,%%ymm5,%%ymm0 	\n\t  vfmadd132pd	%%ymm6 ,%%ymm13,%%ymm8	\n\t"\
		"vmovaps	%%ymm1,    (%%rcx)		\n\t	vmovaps	%%ymm9 ,    (%%r13)		\n\t"\
		"vmovaps	%%ymm0,0x20(%%rbx)		\n\t	vmovaps	%%ymm8 ,0x20(%%r12)		\n\t"\
		:					/* outputs: none */\
		: [__cc1] "m" (Xcc1)	/* All inputs from memory addresses here */\
		 ,[__two] "m" (Xtwo)\
		 ,[__i0] "m" (Xi0)\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__i3] "m" (Xi3)\
		 ,[__i4] "m" (Xi4)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__o3] "m" (Xo3)\
		 ,[__o4] "m" (Xo4)\
		 ,[__j0] "m" (Xj0)\
		 ,[__j1] "m" (Xj1)\
		 ,[__j2] "m" (Xj2)\
		 ,[__j3] "m" (Xj3)\
		 ,[__j4] "m" (Xj4)\
		 ,[__u0] "m" (Xu0)\
		 ,[__u1] "m" (Xu1)\
		 ,[__u2] "m" (Xu2)\
		 ,[__u3] "m" (Xu3)\
		 ,[__u4] "m" (Xu4)\
		: "cc","memory","rax","rbx","rcx","rdx","rdi","rsi","r10","r11","r12","r13","r14","r15","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

	// Here are the array-of-doubles index offsets w.r.to the __c = cc0 base-root address of the various derived sincos terms:
	// Datum	Offset	Datum	Offset
	// ------	------	------	------
	// __c1_c	0x00	__sc	0x01
	// __c1i2	0x02	__c2i2	0x03
	// __c8		0x04	__r8	0x05
	// __c4		0x06	__r4	0x07
	// __cC4	0x08	__rC	0x09
	// __c2		0x0a	__r2	0x0b
	// __cA2	0x0c	__rA	0x0d
	// __c62	0x0e	__r6	0x0f
	// __cE6	0x10	__rE	0x11
	// __c1		0x12	__r1	0x13
	// __c91	0x14	__r9	0x15
	// __c51	0x16	__r5	0x17
	// __cD5	0x18	__rD	0x19
	// __c31	0x1a	__r3	0x1b
	// __cB3	0x1c	__rB	0x1d
	// __c73	0x1e	__r7	0x1f
	// __cF7	0x20	__rF	0x21

	// Remember that for AVX2-style 3-operand FMA in AT&T syntax, the result overwrites the rightmost input!
	#define SSE2_RADIX16_DIF_FMA_OOP(Xin0,Xi1,Xi2,Xi3,Xi4, Xout0,Xout1,Xout2,Xout3,Xout4,Xout5,Xout6,Xout7,Xout8,Xout9,Xouta,Xoutb,Xoutc,Xoutd,Xoute,Xoutf, Xcc0)\
	{\
	__asm__ volatile (\
	/*...Block 0: Do in-place, i.e. outputs into __in0 + [0,1,2,3]*istride: */\
		"movq	%[__in0],%%rax		\n\t"\
		"leaq	%c[__i1](%%rax),%%rcx	\n\t"/* __in0 +   istride */\
		"leaq	%c[__i2](%%rax),%%rbx	\n\t"/* __in0 + 2*istride */\
		"leaq	%c[__i3](%%rax),%%rdx	\n\t"/* __in0 + 3*istride */\
		"movq	%[__cc0],%%rsi 			\n\t"\
		"vbroadcastsd 0x28(%%rsi),%%ymm13 \n\t vbroadcastsd 0x38(%%rsi),%%ymm14 \n\t vbroadcastsd 0x48(%%rsi),%%ymm15 \n\t"/* load __r8,r4,rC into ymm13-15 */\
		"vmovaps		     (%%rcx),%%ymm4 		\n\t	vmovaps			0x020(%%rcx),%%ymm5 		\n\t"/*	t04 =__A8r;					t05 =__A8i; */\
		"vmovaps		     (%%rax),%%ymm0 		\n\t	vmovaps			0x020(%%rax),%%ymm1 		\n\t"/*	t00 =__A0r;					t01 =__A0i; */\
		"vmovaps		%%ymm4,%%ymm6				\n\t"/*	t06 = t04; */\
		"vfnmadd231pd	%%ymm5 ,%%ymm13,%%ymm4 		\n\t	 vfmadd231pd	%%ymm6 ,%%ymm13,%%ymm5 		\n\t"/*	FNMA231(  t05,__r8,t04);	 FMA231(  t06,__r8,t05); */\
		"vmovaps		     (%%rbx),%%ymm8			\n\t	vmovaps			0x020(%%rbx),%%ymm9 		\n\t"/*	_a =__A4r;					_b =__A4i; */\
		"vfnmadd231pd	0x020(%%rbx),%%ymm14,%%ymm8 \n\t	 vfmadd231pd	     (%%rbx),%%ymm14,%%ymm9 \n\t"/*	FNMA231(__A4i,__r4,_a );	 FMA231(__A4r,__r4,_b ); */\
		"vbroadcastsd	0x040(%%rsi),%%ymm13		\n\t	vbroadcastsd	0x020(%%rsi),%%ymm14		\n\t"/* load __cC4,c8 into pair of regs */\
		"vmovaps		     (%%rdx),%%ymm6			\n\t	vmovaps			0x020(%%rdx),%%ymm7 		\n\t"/*	t06 =__ACr;					t07 =__ACi; */\
		"vfnmadd231pd	0x020(%%rdx),%%ymm15,%%ymm6 \n\t	 vfmadd231pd	     (%%rdx),%%ymm15,%%ymm7 \n\t"/*	FNMA231(__ACi,__rC,t06);	 FMA231(__ACr,__rC,t07); */\
		"vbroadcastsd	0x030(%%rsi),%%ymm15		\n\t"/* load __c4 */\
		"vmovaps		%%ymm8 ,%%ymm10				\n\t	vmovaps			%%ymm0,%%ymm2 				\n\t"/*	_c = _a;	t02 = t00; */\
		" vfmadd231pd	%%ymm6 ,%%ymm13,%%ymm8 		\n\t	 vfmadd231pd	%%ymm4 ,%%ymm14,%%ymm0 		\n\t"/*	 FMA231(t06,__cC4,_a);		 FMA231(t04,__c8,t00); */\
		"vmovaps		%%ymm9 ,%%ymm11				\n\t	vmovaps			%%ymm1 ,%%ymm3 				\n\t"/*	_d = _b;	t03 = t01; */\
		" vfmadd231pd	%%ymm7 ,%%ymm13,%%ymm9 		\n\t	 vfmadd231pd	%%ymm5 ,%%ymm14,%%ymm1 		\n\t"/*	 FMA231(t07,__cC4,_b);		 FMA231(t05,__c8,t01); */\
		"vfnmadd231pd	%%ymm6 ,%%ymm13,%%ymm10		\n\t	vfnmadd231pd	%%ymm4 ,%%ymm14,%%ymm2 		\n\t"/*	FNMA231(t06,__cC4,_c);		FNMA231(t04,__c8,t02); */\
		"vfnmadd231pd	%%ymm7 ,%%ymm13,%%ymm11		\n\t	vfnmadd231pd	%%ymm5 ,%%ymm14,%%ymm3 		\n\t"/*	FNMA231(t07,__cC4,_d);		FNMA231(t05,__c8,t03); */\
		"vmovaps		%%ymm0 ,%%ymm4 				\n\t	vmovaps			%%ymm1 ,%%ymm5 				\n\t"/*	t04 =t00; t05 =t01; */\
		"vfnmadd231pd	%%ymm8 ,%%ymm15,%%ymm4 		\n\t	 vfmadd231pd	%%ymm8 ,%%ymm15,%%ymm0 		\n\t"/*	FNMA231(_a ,__c4 ,t04);		 FMA231(_a ,__c4 ,t00); */\
		"vfnmadd231pd	%%ymm9 ,%%ymm15,%%ymm5 		\n\t	 vfmadd231pd	%%ymm9 ,%%ymm15,%%ymm1 		\n\t"/*	FNMA231(_b ,__c4 ,t05);		 FMA231(_b ,__c4 ,t01); */\
		"vmovaps		%%ymm2 ,%%ymm6 				\n\t	vmovaps			%%ymm3 ,%%ymm7 				\n\t"/*	t06 =t02;	t07 =t03; */\
		" vfmadd231pd	%%ymm11,%%ymm15,%%ymm6 		\n\t	vfnmadd231pd	%%ymm11,%%ymm15,%%ymm2 		\n\t"/*	 FMA231(_d ,__c4 ,t06);		FNMA231(_d ,__c4 ,t02); */\
		"vfnmadd231pd	%%ymm10,%%ymm15,%%ymm7 		\n\t	 vfmadd231pd	%%ymm10,%%ymm15,%%ymm3 		\n\t"/*	FNMA231(_c ,__c4 ,t07);		 FMA231(_c ,__c4 ,t03); */\
		"vmovaps		%%ymm4 ,     (%%rcx)		\n\t	vmovaps			%%ymm0 ,     (%%rax)		\n\t"/* Write outputs into local store */\
		"vmovaps		%%ymm5 ,0x020(%%rcx)		\n\t	vmovaps			%%ymm1 ,0x020(%%rax)		\n\t"\
		"vmovaps		%%ymm6 ,     (%%rdx)		\n\t	vmovaps			%%ymm2 ,     (%%rbx)		\n\t"\
		"vmovaps		%%ymm7 ,0x020(%%rdx)		\n\t	vmovaps			%%ymm3 ,0x020(%%rbx)		\n\t"\
		"\n\t"\
	/*...Block 1: outputs into __in0 + [4,5,6,7]*istride: */\
		"addq	$%c[__i4],%%rax	\n\t"/* __in0 + 4*istride */\
		"addq	$%c[__i4],%%rcx	\n\t"/* __in0 + 5*istride */\
		"addq	$%c[__i4],%%rbx	\n\t"/* __in0 + 6*istride */\
		"addq	$%c[__i4],%%rdx	\n\t"/* __in0 + 7*istride */\
		"addq	$0x040,%%rsi 		/* cc += 8 */\n\t"\
		"vbroadcastsd 0x018(%%rsi),%%ymm12 	\n\t"/* load __r2 into ymm12 */\
		"vbroadcastsd 0x028(%%rsi),%%ymm13 	\n\t"/* load __rA into ymm13 */\
		"vbroadcastsd 0x038(%%rsi),%%ymm14 	\n\t"/* load __r6 into ymm14 */\
		"vbroadcastsd 0x048(%%rsi),%%ymm15 	\n\t"/* load __rE into ymm15 */\
		"vmovaps (%%rax),%%ymm0  \n\t vmovaps 0x020(%%rax),%%ymm1  \n\t"/* t08 =__A2r; t09 =__A2i; */\
		"vmovaps (%%rcx),%%ymm4  \n\t vmovaps 0x020(%%rcx),%%ymm5  \n\t"/* t12 =__AAr; t13 =__AAi; */\
		"vmovaps (%%rbx),%%ymm8  \n\t vmovaps 0x020(%%rbx),%%ymm9  \n\t"/* _a  =__A6r; _b  =__A6i; */\
		"vmovaps (%%rdx),%%ymm6  \n\t vmovaps 0x020(%%rdx),%%ymm7  \n\t"/* t14 =__AEr; t15 =__AEi; */\
		"vfnmadd231pd	0x020(%%rax),%%ymm12,%%ymm0 \n\t	 vfmadd231pd	(%%rax),%%ymm12,%%ymm1  	\n\t"/* FNMA231(__A2i,__r2,t08); FMA231(__A2r,__r2,t09); */\
		"vfnmadd231pd	0x020(%%rcx),%%ymm13,%%ymm4 \n\t	 vfmadd231pd	(%%rcx),%%ymm13,%%ymm5  	\n\t"/* FNMA231(__AAi,__rA,t12); FMA231(__AAr,__rA,t13); */\
		"vbroadcastsd	0x040(%%rsi),%%ymm13		\n\t"/* load __cE6 */\
		"vfnmadd231pd	0x020(%%rbx),%%ymm14,%%ymm8 \n\t	 vfmadd231pd	(%%rbx),%%ymm14,%%ymm9  	\n\t"/* FNMA231(__A6i,__r6,_a ); FMA231(__A6r,__r6,_b ); */\
		"vbroadcastsd	0x020(%%rsi),%%ymm14		\n\t"/* load __cA2 */\
		"vfnmadd231pd	0x020(%%rdx),%%ymm15,%%ymm6 \n\t	 vfmadd231pd	(%%rdx),%%ymm15,%%ymm7  	\n\t"/* FNMA231(__AEi,__rE,t14); FMA231(__AEr,__rE,t15); */\
		"vbroadcastsd	0x030(%%rsi),%%ymm15		\n\t"/* load __c62 */\
		"vmovaps		%%ymm8 ,%%ymm10				\n\t	vmovaps			%%ymm0,%%ymm2 				\n\t"/*	_c = _a;	t10 = t08; */\
		" vfmadd231pd	%%ymm6 ,%%ymm13,%%ymm8 		\n\t	 vfmadd231pd	%%ymm4 ,%%ymm14,%%ymm0 		\n\t"/*	 FMA231(t14,__cE6,_a);		 FMA231(t12,__cA2,t08); */\
		"vmovaps		%%ymm9 ,%%ymm11				\n\t	vmovaps			%%ymm1 ,%%ymm3 				\n\t"/*	_d = _b;	t11 = t09; */\
		" vfmadd231pd	%%ymm7 ,%%ymm13,%%ymm9 		\n\t	 vfmadd231pd	%%ymm5 ,%%ymm14,%%ymm1 		\n\t"/*	 FMA231(t15,__cE6,_b);		 FMA231(t13,__cA2,t09); */\
		"vfnmadd231pd	%%ymm6 ,%%ymm13,%%ymm10		\n\t	vfnmadd231pd	%%ymm4 ,%%ymm14,%%ymm2 		\n\t"/*	FNMA231(t14,__cE6,_c);		FNMA231(t12,__cA2,t10); */\
		"vfnmadd231pd	%%ymm7 ,%%ymm13,%%ymm11		\n\t	vfnmadd231pd	%%ymm5 ,%%ymm14,%%ymm3 		\n\t"/*	FNMA231(t15,__cE6,_d);		FNMA231(t13,__cA2,t11); */\
		"vmovaps		%%ymm0 ,%%ymm4 				\n\t	vmovaps			%%ymm1 ,%%ymm5 				\n\t"/*	t12 =t08 ;	t13 =t09; */\
		"vfnmadd231pd	%%ymm8 ,%%ymm15,%%ymm4 		\n\t	 vfmadd231pd	%%ymm8 ,%%ymm15,%%ymm0 		\n\t"/*	FNMA231(_a,__c62,t12);		 FMA231( _a,__c62,t08); */\
		"vfnmadd231pd	%%ymm9 ,%%ymm15,%%ymm5 		\n\t	 vfmadd231pd	%%ymm9 ,%%ymm15,%%ymm1 		\n\t"/*	FNMA231(_b,__c62,t13);		 FMA231( _b,__c62,t09); */\
		"vmovaps		%%ymm2 ,%%ymm6 				\n\t	vmovaps			%%ymm3 ,%%ymm7 				\n\t"/*	t14 =t10;	t15 =t11; */\
		" vfmadd231pd	%%ymm11,%%ymm15,%%ymm6 		\n\t	vfnmadd231pd	%%ymm11,%%ymm15,%%ymm2 		\n\t"/*	 FMA231(_d,__c62,t14);		FNMA231( _d,__c62,t10); */\
		"vfnmadd231pd	%%ymm10,%%ymm15,%%ymm7 		\n\t	 vfmadd231pd	%%ymm10,%%ymm15,%%ymm3 		\n\t"/*	FNMA231(_c,__c62,t15);		 FMA231( _c,__c62,t11); */\
		"vmovaps		%%ymm4 ,     (%%rcx)		\n\t	vmovaps			%%ymm0 ,     (%%rax)		\n\t"/* Write outputs into local store */\
		"vmovaps		%%ymm5 ,0x020(%%rcx)		\n\t	vmovaps			%%ymm1 ,0x020(%%rax)		\n\t"\
		"vmovaps		%%ymm6 ,     (%%rdx)		\n\t	vmovaps			%%ymm2 ,     (%%rbx)		\n\t"\
		"vmovaps		%%ymm7 ,0x020(%%rdx)		\n\t	vmovaps			%%ymm3 ,0x020(%%rbx)		\n\t"\
		"\n\t"\
	/*...Block 2: outputs into __in0 + [8,9,a,b]*istride: */\
		"addq	$%c[__i4],%%rax	\n\t"/* __in0 + 8*istride */\
		"addq	$%c[__i4],%%rcx	\n\t"/* __in0 + 9*istride */\
		"addq	$%c[__i4],%%rbx	\n\t"/* __in0 + a*istride */\
		"addq	$%c[__i4],%%rdx	\n\t"/* __in0 + b*istride */\
		"addq	$0x040,%%rsi 		/* cc += 8 */\n\t"\
		"vbroadcastsd 0x018(%%rsi),%%ymm12 	\n\t"/* load __r1 into ymm12 */\
		"vbroadcastsd 0x028(%%rsi),%%ymm13 	\n\t"/* load __r9 into ymm13 */\
		"vbroadcastsd 0x038(%%rsi),%%ymm14 	\n\t"/* load __r5 into ymm14 */\
		"vbroadcastsd 0x048(%%rsi),%%ymm15 	\n\t"/* load __rD into ymm15 */\
		"vmovaps (%%rax),%%ymm0  \n\t vmovaps 0x020(%%rax),%%ymm1  \n\t"/* t16 =__A1r;	t17 =__A1i; */\
		"vmovaps (%%rcx),%%ymm4  \n\t vmovaps 0x020(%%rcx),%%ymm5  \n\t"/* t20 =__A9r;	t21 =__A9i; */\
		"vmovaps (%%rbx),%%ymm8  \n\t vmovaps 0x020(%%rbx),%%ymm9  \n\t"/* _a=  __A5r;	_b  =__A5i; */\
		"vmovaps (%%rdx),%%ymm6  \n\t vmovaps 0x020(%%rdx),%%ymm7  \n\t"/* t22 =__ADr;	t23 =__ADi; */\
		"vfnmadd231pd	0x020(%%rax),%%ymm12,%%ymm0 \n\t	 vfmadd231pd	(%%rax),%%ymm12,%%ymm1  	\n\t"/* FNMA231(__A1i,__r1,t16);	 FMA231(__A1r,__r1,t17); */\
		"vfnmadd231pd	0x020(%%rcx),%%ymm13,%%ymm4 \n\t	 vfmadd231pd	(%%rcx),%%ymm13,%%ymm5  	\n\t"/* FNMA231(__A9i,__r9,t20);	 FMA231(__A9r,__r9,t21); */\
		"vbroadcastsd	0x040(%%rsi),%%ymm13		\n\t"/* load __cD5 */\
		"vfnmadd231pd	0x020(%%rbx),%%ymm14,%%ymm8 \n\t	 vfmadd231pd	(%%rbx),%%ymm14,%%ymm9  	\n\t"/* FNMA231(__A5i,__r5,_a );	 FMA231(__A5r,__r5,_b ); */\
		"vbroadcastsd	0x020(%%rsi),%%ymm14		\n\t"/* load __c91 */\
		"vfnmadd231pd	0x020(%%rdx),%%ymm15,%%ymm6 \n\t	 vfmadd231pd	(%%rdx),%%ymm15,%%ymm7  	\n\t"/* FNMA231(__ADi,__rD,t22);	 FMA231(__ADr,__rD,t23); */\
		"vbroadcastsd	0x030(%%rsi),%%ymm15		\n\t"/* load __c51 */\
		"vmovaps		%%ymm8 ,%%ymm10				\n\t	vmovaps			%%ymm0,%%ymm2 				\n\t"/*	_c= _a;	t18= t16; */\
		" vfmadd231pd	%%ymm6 ,%%ymm13,%%ymm8 		\n\t	 vfmadd231pd	%%ymm4 ,%%ymm14,%%ymm0 		\n\t"/*	 FMA231(t22,__cD5,_a);		 FMA231(t20,__c91,t16); */\
		"vmovaps		%%ymm9 ,%%ymm11				\n\t	vmovaps			%%ymm1 ,%%ymm3 				\n\t"/*	_d= _b;	t19= t17; */\
		" vfmadd231pd	%%ymm7 ,%%ymm13,%%ymm9 		\n\t	 vfmadd231pd	%%ymm5 ,%%ymm14,%%ymm1 		\n\t"/*	 FMA231(t23,__cD5,_b);		 FMA231(t21,__c91,t17); */\
		"vfnmadd231pd	%%ymm6 ,%%ymm13,%%ymm10		\n\t	vfnmadd231pd	%%ymm4 ,%%ymm14,%%ymm2 		\n\t"/*	FNMA231(t22,__cD5,_c);		FNMA231(t20,__c91,t18); */\
		"vfnmadd231pd	%%ymm7 ,%%ymm13,%%ymm11		\n\t	vfnmadd231pd	%%ymm5 ,%%ymm14,%%ymm3 		\n\t"/*	FNMA231(t23,__cD5,_d);		FNMA231(t21,__c91,t19); */\
		"vmovaps		%%ymm0 ,%%ymm4 				\n\t	vmovaps			%%ymm1 ,%%ymm5 				\n\t"/*	t20 =t16;	t21 =t17; */\
		"vfnmadd231pd	%%ymm8 ,%%ymm15,%%ymm4 		\n\t	 vfmadd231pd	%%ymm8 ,%%ymm15,%%ymm0 		\n\t"/*	FNMA231(_a,__c51,t20);		 FMA231(_a,__c51,t16); */\
		"vfnmadd231pd	%%ymm9 ,%%ymm15,%%ymm5 		\n\t	 vfmadd231pd	%%ymm9 ,%%ymm15,%%ymm1 		\n\t"/*	FNMA231(_b,__c51,t21);		 FMA231(_b,__c51,t17); */\
		"vmovaps		%%ymm2 ,%%ymm6 				\n\t	vmovaps			%%ymm3 ,%%ymm7 				\n\t"/*	t22 =t18;	t23 =t19; */\
		" vfmadd231pd	%%ymm11,%%ymm15,%%ymm6 		\n\t	vfnmadd231pd	%%ymm11,%%ymm15,%%ymm2 		\n\t"/*	 FMA231(_d,__c51,t22);		FNMA231(_d,__c51,t18); */\
		"vfnmadd231pd	%%ymm10,%%ymm15,%%ymm7 		\n\t	 vfmadd231pd	%%ymm10,%%ymm15,%%ymm3 		\n\t"/*	FNMA231(_c,__c51,t23);		 FMA231(_c,__c51,t19); */\
		"vmovaps		%%ymm4 ,     (%%rcx)		\n\t	vmovaps			%%ymm0 ,     (%%rax)		\n\t"/* Write outputs into local store */\
		"vmovaps		%%ymm5 ,0x020(%%rcx)		\n\t	vmovaps			%%ymm1 ,0x020(%%rax)		\n\t"\
		"vmovaps		%%ymm6 ,     (%%rdx)		\n\t	vmovaps			%%ymm2 ,     (%%rbx)		\n\t"\
		"vmovaps		%%ymm7 ,0x020(%%rdx)		\n\t	vmovaps			%%ymm3 ,0x020(%%rbx)		\n\t"\
		"\n\t"\
	/*...Block 3: outputs into __in0 + [c,d,e,f]*istride: */\
		"addq	$%c[__i4],%%rax	\n\t"/* __in0 + c*istride */\
		"addq	$%c[__i4],%%rcx	\n\t"/* __in0 + d*istride */\
		"addq	$%c[__i4],%%rbx	\n\t"/* __in0 + e*istride */\
		"addq	$%c[__i4],%%rdx	\n\t"/* __in0 + f*istride */\
		"addq	$0x040,%%rsi 		/* cc += 8 */\n\t"\
		"vbroadcastsd 0x018(%%rsi),%%ymm12 	\n\t"/* load __r3 into ymm12 */\
		"vbroadcastsd 0x028(%%rsi),%%ymm13 	\n\t"/* load __rB into ymm13 */\
		"vbroadcastsd 0x038(%%rsi),%%ymm14 	\n\t"/* load __r7 into ymm14 */\
		"vbroadcastsd 0x048(%%rsi),%%ymm15 	\n\t"/* load __rF into ymm15 */\
		"vmovaps (%%rax),%%ymm0  \n\t vmovaps 0x020(%%rax),%%ymm1  \n\t"/* t24 =__A3r;	t25 =__A3i; */\
		"vmovaps (%%rcx),%%ymm4  \n\t vmovaps 0x020(%%rcx),%%ymm5  \n\t"/* t28 =__ABr;	t29 =__ABi; */\
		"vmovaps (%%rbx),%%ymm8  \n\t vmovaps 0x020(%%rbx),%%ymm9  \n\t"/* _a  =__A7r;	_b  =__A7i; */\
		"vmovaps (%%rdx),%%ymm6  \n\t vmovaps 0x020(%%rdx),%%ymm7  \n\t"/* t30 =__AFr;	t31 =__AFi; */\
		"vfnmadd231pd	0x020(%%rax),%%ymm12,%%ymm0 \n\t	 vfmadd231pd	(%%rax),%%ymm12,%%ymm1  	\n\t"/* FNMA231(__A3i,__r3,t24);	 FMA231(__A3r,__r3,t25); */\
		"vfnmadd231pd	0x020(%%rcx),%%ymm13,%%ymm4 \n\t	 vfmadd231pd	(%%rcx),%%ymm13,%%ymm5  	\n\t"/* FNMA231(__ABi,__rB,t28 );	 FMA231(__ABr,__rB,t29 ); */\
		"vbroadcastsd	0x040(%%rsi),%%ymm13		\n\t"/* load __cF7 */\
		"vfnmadd231pd	0x020(%%rbx),%%ymm14,%%ymm8 \n\t	 vfmadd231pd	(%%rbx),%%ymm14,%%ymm9  	\n\t"/* FNMA231(__A7i,__r7,_a);		 FMA231(__A7r,__r7,_b); */\
		"vbroadcastsd	0x020(%%rsi),%%ymm14		\n\t"/* load __cB3 */\
		"vfnmadd231pd	0x020(%%rdx),%%ymm15,%%ymm6 \n\t	 vfmadd231pd	(%%rdx),%%ymm15,%%ymm7  	\n\t"/* FNMA231(__AFi,__rF,t30 );	 FMA231(__AFr,__rF,t31 ); */\
		"vbroadcastsd	0x030(%%rsi),%%ymm15		\n\t"/* load __c73 */\
		"vmovaps		%%ymm8 ,%%ymm10				\n\t	vmovaps			%%ymm0,%%ymm2 				\n\t"/*	_c= _a;	t26= t24; */\
		" vfmadd231pd	%%ymm6 ,%%ymm13,%%ymm8 		\n\t	 vfmadd231pd	%%ymm4 ,%%ymm14,%%ymm0 		\n\t"/*	 FMA231(t30,__cF7,_a);		 FMA231(t28,__cB3,t24); */\
		"vmovaps		%%ymm9 ,%%ymm11				\n\t	vmovaps			%%ymm1 ,%%ymm3 				\n\t"/*	_d= _b;	t27= t25; */\
		" vfmadd231pd	%%ymm7 ,%%ymm13,%%ymm9 		\n\t	 vfmadd231pd	%%ymm5 ,%%ymm14,%%ymm1 		\n\t"/*	 FMA231(t31,__cF7,_b);		 FMA231(t29,__cB3,t25); */\
		"vfnmadd231pd	%%ymm6 ,%%ymm13,%%ymm10		\n\t	vfnmadd231pd	%%ymm4 ,%%ymm14,%%ymm2 		\n\t"/*	FNMA231(t30,__cF7,_c);		FNMA231(t28,__cB3,t26); */\
		"vfnmadd231pd	%%ymm7 ,%%ymm13,%%ymm11		\n\t	vfnmadd231pd	%%ymm5 ,%%ymm14,%%ymm3 		\n\t"/*	FNMA231(t31,__cF7,_d);		FNMA231(t29,__cB3,t27); */\
		"vmovaps		%%ymm0 ,%%ymm4 				\n\t	vmovaps			%%ymm1 ,%%ymm5 				\n\t"/*	t28 =t24;	t29 =t25; */\
		"vfnmadd231pd	%%ymm8 ,%%ymm15,%%ymm4 		\n\t	 vfmadd231pd	%%ymm8 ,%%ymm15,%%ymm0 		\n\t"/*	FNMA231(_a,__c73,t28);		 FMA231(_a,__c73,t24); */\
		"vfnmadd231pd	%%ymm9 ,%%ymm15,%%ymm5 		\n\t	 vfmadd231pd	%%ymm9 ,%%ymm15,%%ymm1 		\n\t"/*	FNMA231(_b,__c73,t29);		 FMA231(_b,__c73,t25); */\
		"vmovaps		%%ymm2 ,%%ymm6 				\n\t	vmovaps			%%ymm3 ,%%ymm7 				\n\t"/*	t30 =t26;	t31 =t27; */\
		" vfmadd231pd	%%ymm11,%%ymm15,%%ymm6 		\n\t	vfnmadd231pd	%%ymm11,%%ymm15,%%ymm2 		\n\t"/*	 FMA231(_d,__c73,t30);		FNMA231(_d,__c73,t26); */\
		"vfnmadd231pd	%%ymm10,%%ymm15,%%ymm7 		\n\t	 vfmadd231pd	%%ymm10,%%ymm15,%%ymm3 		\n\t"/*	FNMA231(_c,__c73,t31);		 FMA231(_c,__c73,t27); */\
		"vmovaps		%%ymm4 ,     (%%rcx)		\n\t	vmovaps			%%ymm0 ,     (%%rax)		\n\t"/* Write outputs into local store */\
		"vmovaps		%%ymm5 ,0x020(%%rcx)		\n\t	vmovaps			%%ymm1 ,0x020(%%rax)		\n\t"\
		"vmovaps		%%ymm6 ,     (%%rdx)		\n\t	vmovaps			%%ymm2 ,     (%%rbx)		\n\t"\
		"vmovaps		%%ymm7 ,0x020(%%rdx)		\n\t	vmovaps			%%ymm3 ,0x020(%%rbx)		\n\t"\
		"\n\t"\
	/*************************************************************************************/\
	/*  And now do four more radix-4 transforms, including the internal twiddle factors: */\
	/*************************************************************************************/\
	/* Block 0: Combine 0-output of each radix-4, i.e. inputs from __in0 + [0,4,8,c]*istride: */\
		"movq	%[__in0],%%rax		\n\t"\
		"leaq	%c[__i4](%%rax),%%rbx	\n\t"/* __in0 +   [4*istride] */\
		"leaq	%c[__i4](%%rbx),%%rcx	\n\t"/* __in0 + 2*[4*istride] */\
		"leaq	%c[__i4](%%rcx),%%rdx	\n\t"/* __in0 + 3*[4*istride] */\
		"subq	$0x0c0,%%rsi 		/* revert cc-ptr to base value */\n\t"\
		"\n\t"\
		/*...Read t0,8,16,24 from local store ... Do the 4 Im-part FMAs first, because their results needed 1st below */\
		"vbroadcastsd	0x050(%%rsi),%%ymm14		\n\t	vbroadcastsd	0x0d0(%%rsi),%%ymm15		\n\t"/* load __c2,c31 into pair of regs */\
		"vmovaps		     (%%rax),%%ymm0 		\n\t	vmovaps			     (%%rbx),%%ymm2 		\n\t"/*    t00;    t08; */\
		"vmovaps		     (%%rcx),%%ymm4 		\n\t	vmovaps			     (%%rdx),%%ymm6 		\n\t"/*    t16;    t24; */\
		"vmovaps			 %%ymm0 ,%%ymm8 		\n\t	vmovaps				 %%ymm4 ,%%ymm10		\n\t"/* _a=t00; _c=t16; */\
		" vfmadd231pd	%%ymm2 ,%%ymm14,%%ymm0 		\n\t	 vfmadd231pd	%%ymm6 ,%%ymm15,%%ymm4 		\n\t"/*	 FMA231(t08,__c2 ,t00);		 FMA231(t24,__c31,t16); */\
		"vmovaps		0x020(%%rax),%%ymm1 		\n\t	vmovaps			0x020(%%rbx),%%ymm3 		\n\t"/*    t01;    t09; */\
		"vmovaps		0x020(%%rcx),%%ymm5 		\n\t	vmovaps			0x020(%%rdx),%%ymm7 		\n\t"/*    t17;    t25; */\
		"vmovaps			 %%ymm1 ,%%ymm9 		\n\t	vmovaps				 %%ymm5 ,%%ymm11		\n\t"/* _b=t01; _d=t17; */\
		" vfmadd231pd	%%ymm3 ,%%ymm14,%%ymm1 		\n\t	 vfmadd231pd	%%ymm7 ,%%ymm15,%%ymm5 		\n\t"/*	 FMA231(t09,__c2 ,t01);		 FMA231(t25,__c31,t17); */\
		"vfnmadd231pd	%%ymm2 ,%%ymm14,%%ymm8 		\n\t	vfnmadd231pd	%%ymm6 ,%%ymm15,%%ymm10		\n\t"/*	FNMA231(t08,__c2 ,_a );		FNMA231(t24,__c31,_c ); */\
		"vbroadcastsd	0x090(%%rsi),%%ymm6 		\n\t"/* load __c1 */\
		"vfnmadd231pd	%%ymm3 ,%%ymm14,%%ymm9 		\n\t	vfnmadd231pd	%%ymm7 ,%%ymm15,%%ymm11		\n\t"/*	FNMA231(t09,__c2 ,_b );		FNMA231(t25,__c31,_d ); */\
		"vmovaps			 %%ymm0 ,%%ymm12		\n\t	vmovaps				 %%ymm1 ,%%ymm13		\n\t"/* _e = t00; _f = t01; */\
		" vfmadd231pd	%%ymm4 ,%%ymm6 ,%%ymm0 		\n\t	 vfmadd231pd	%%ymm5 ,%%ymm6 ,%%ymm1 		\n\t"/*	 FMA231(t16,__c1 ,t00);		 FMA231(t17,__c1 ,t01); */\
		"vfnmadd231pd	%%ymm4 ,%%ymm6 ,%%ymm12		\n\t	vfnmadd231pd	%%ymm5 ,%%ymm6 ,%%ymm13		\n\t"/*	FNMA231(t16,__c1 ,_e );		FNMA231(t17,__c1 ,_f ); */\
		"vmovaps			 %%ymm8 ,%%ymm2 		\n\t	vmovaps				 %%ymm9 ,%%ymm3 		\n\t"/* t08 = _a ; t09 = _b; */\
		"vfnmadd231pd	%%ymm11,%%ymm6 ,%%ymm2 		\n\t	 vfmadd231pd	%%ymm10,%%ymm6 ,%%ymm3 		\n\t"/*	FNMA231(_d ,__c1 ,t08);		 FMA231(_c ,__c1 ,t09); */\
		" vfmadd231pd	%%ymm11,%%ymm6 ,%%ymm8 		\n\t	vfnmadd231pd	%%ymm10,%%ymm6 ,%%ymm9 		\n\t"/*	 FMA231(_d ,__c1 ,_a );		FNMA231(_c ,__c1 ,_b ); */\
		/* Write outputs: */\
		"movq	%[__out0],%%r10		\n\t"\
		"movq	%[__out1],%%r11		\n\t"\
		"movq	%[__out2],%%r12		\n\t"\
		"movq	%[__out3],%%r13		\n\t"\
		"vmovaps		%%ymm0 ,     (%%r10)		\n\t	vmovaps			%%ymm1 ,0x020(%%r10)		\n\t"/* __B0r= t00;		__B0i= t01; */\
		"vmovaps		%%ymm12,     (%%r11)		\n\t	vmovaps			%%ymm13,0x020(%%r11)		\n\t"/* __B1r= _e ;		__B1i= _f ; */\
		"vmovaps		%%ymm2 ,     (%%r12)		\n\t	vmovaps			%%ymm3 ,0x020(%%r12)		\n\t"/* __B2r= t08;		__B2i= t09; */\
		"vmovaps		%%ymm8 ,     (%%r13)		\n\t	vmovaps			%%ymm9 ,0x020(%%r13)		\n\t"/* __B3r= _a ;		__B3i= _b ; */\
		"\n\t"\
		/*...Block 2: t4,12,20,28 */\
		"vbroadcastsd	0x110(%%rsi),%%ymm13	\n\t"/* cc0 + 0x22 = __two; Actually holds 1.0 in AVX2 mode */\
		"addq	$%c[__i1],%%rax	\n\t"/* __in0 + 1*istride */\
		"addq	$%c[__i1],%%rbx	\n\t"/* __in0 + 5*istride */\
		"addq	$%c[__i1],%%rcx	\n\t"/* __in0 + 9*istride */\
		"addq	$%c[__i1],%%rdx	\n\t"/* __in0 + d*istride */\
		"vbroadcastsd	0x050(%%rsi),%%ymm14		\n\t	vbroadcastsd	0x0d0(%%rsi),%%ymm15		\n\t"/* load __c2,31 into pair of regs */\
		"vmovaps		     (%%rcx),%%ymm4 		\n\t	vmovaps			0x020(%%rcx),%%ymm5 		\n\t"/*    t20;    t21; */\
		"vmovaps		     (%%rdx),%%ymm6 		\n\t	vmovaps			0x020(%%rdx),%%ymm7 		\n\t"/*    t28;    t29; */\
		"vmovaps			 %%ymm4 ,%%ymm10 		\n\t	vmovaps				 %%ymm7 ,%%ymm11		\n\t"/* _c=t20; _d=t29; */\
		"vfnmadd231pd	%%ymm5 ,%%ymm13,%%ymm10		\n\t	 vfmadd231pd	%%ymm5 ,%%ymm13,%%ymm4 		\n\t"/*	FNMA231(t21,1.0,_c );		 FMA231(t21,1.0,t20); */\
		" vfmadd231pd	%%ymm6 ,%%ymm13,%%ymm11		\n\t	vfnmadd231pd	%%ymm6 ,%%ymm13,%%ymm7 		\n\t"/*	 FMA231(t28,1.0,_d );		FNMA231(t28,1.0,t29); */\
		"vbroadcastsd	0x010(%%rsi),%%ymm13		\n\t"/* load __c1i2 */\
		"vmovaps		     (%%rax),%%ymm0 		\n\t	vmovaps			0x020(%%rax),%%ymm1 		\n\t"/*    t04;    t05; */\
		"vmovaps		     (%%rbx),%%ymm2 		\n\t	vmovaps			0x020(%%rbx),%%ymm3 		\n\t"/*    t12;    t13; */\
		"vmovaps			 %%ymm0 ,%%ymm8 		\n\t	vmovaps				 %%ymm1 ,%%ymm9 		\n\t"/* _a = t04; _b = t05; */\
		"vmovaps			 %%ymm10,%%ymm5 		\n\t	vmovaps				 %%ymm4 ,%%ymm12		\n\t"/* t21 = _c; _e = t20; */\
		" vfmadd231pd	%%ymm3 ,%%ymm14,%%ymm8 		\n\t	 vfmadd231pd	%%ymm11,%%ymm15,%%ymm5 		\n\t"/*	 FMA231(t13,__c2 ,_a );		 FMA231(_d ,__c31,t21); */\
		"vfnmadd231pd	%%ymm2 ,%%ymm14,%%ymm9 		\n\t	 vfmadd231pd	%%ymm7 ,%%ymm15,%%ymm4 		\n\t"/*	FNMA231(t12,__c2 ,_b );		 FMA231(t29,__c31,t20); */\
		"vfnmadd231pd	%%ymm3 ,%%ymm14,%%ymm0 		\n\t	vfnmadd231pd	%%ymm11,%%ymm15,%%ymm10		\n\t"/*	FNMA231(t13,__c2 ,t04);		FNMA231(_d ,__c31,_c ); */\
		" vfmadd231pd	%%ymm2 ,%%ymm14,%%ymm1 		\n\t	vfnmadd231pd	%%ymm7 ,%%ymm15,%%ymm12		\n\t"/*	 FMA231(t12,__c2 ,t05);		FNMA231(t29,__c31,_e ); */\
		"vmovaps			 %%ymm8 ,%%ymm2 		\n\t	vmovaps				 %%ymm9 ,%%ymm3 		\n\t"/* t12 = _a; t13 = _b; */\
		"vfnmadd231pd	%%ymm4 ,%%ymm13,%%ymm2 		\n\t	 vfmadd231pd	%%ymm5 ,%%ymm13,%%ymm3 		\n\t"/*	FNMA231(t20,__c1i2,t12);	 FMA231(t21,__c1i2,t13); */\
		" vfmadd231pd	%%ymm4 ,%%ymm13,%%ymm8 		\n\t	vfnmadd231pd	%%ymm5 ,%%ymm13,%%ymm9 		\n\t"/*	 FMA231(t20,__c1i2,_a );	FNMA231(t21,__c1i2,_b ); */\
		"vmovaps			 %%ymm0 ,%%ymm11		\n\t	vmovaps				 %%ymm1 ,%%ymm7 		\n\t"/* _d = t04; t29 = t05; */\
		" vfmadd231pd	%%ymm10,%%ymm13,%%ymm0 		\n\t	 vfmadd231pd	%%ymm12,%%ymm13,%%ymm1 		\n\t"/*	 FMA231(_c ,__c1i2,t04);	 FMA231(_e ,__c1i2,t05); */\
		"vfnmadd231pd	%%ymm10,%%ymm13,%%ymm11		\n\t	vfnmadd231pd	%%ymm12,%%ymm13,%%ymm7 		\n\t"/*	FNMA231(_c ,__c1i2,_d );	FNMA231(_e ,__c1i2,t29); */\
		/* Write outputs: */\
		"movq	%[__out4],%%r10		\n\t"\
		"movq	%[__out5],%%r11		\n\t"\
		"movq	%[__out6],%%r12		\n\t"\
		"movq	%[__out7],%%r13		\n\t"\
		"vmovaps		%%ymm2 ,     (%%r12)		\n\t	vmovaps			%%ymm3 ,0x020(%%r12)		\n\t"/* __B6r= t12;		__B6i= t13; */\
		"vmovaps		%%ymm8 ,     (%%r13)		\n\t	vmovaps			%%ymm9 ,0x020(%%r13)		\n\t"/* __B7r= _a ;		__B7i= _b ; */\
		"vmovaps		%%ymm0 ,     (%%r10)		\n\t	vmovaps			%%ymm1 ,0x020(%%r10)		\n\t"/* __B4r= t04;		__B4i= t05; */\
		"vmovaps		%%ymm11,     (%%r11)		\n\t	vmovaps			%%ymm7 ,0x020(%%r11)		\n\t"/* __B5r= _d ;		__B5i= t29; */\
		"\n\t"\
		/*...Block 1: t2,10,18,26 */\
		"addq	$%c[__i1],%%rax	\n\t"/* __in0 + 2*istride */\
		"addq	$%c[__i1],%%rbx	\n\t"/* __in0 + 6*istride */\
		"addq	$%c[__i1],%%rcx	\n\t"/* __in0 + a*istride */\
		"addq	$%c[__i1],%%rdx	\n\t"/* __in0 + e*istride */\
		"vmovaps		     (%%rbx),%%ymm2 		\n\t	vmovaps			0x020(%%rbx),%%ymm3 		\n\t"/*    t10;    t11; */\
		"vbroadcastsd	0x008(%%rsi),%%ymm15		\n\t"/* load __sc  */\
		"vbroadcastsd	0x0d0(%%rsi),%%ymm14		\n\t"/* load __c31 */\
		"vmovaps		     (%%rcx),%%ymm4 		\n\t	vmovaps			0x020(%%rcx),%%ymm5 		\n\t"/*    t18;    t19; */\
		"vmovaps		     (%%rdx),%%ymm6 		\n\t	vmovaps			0x020(%%rdx),%%ymm7 		\n\t"/*    t26;    t27; */\
		"vsubpd		%%ymm3 ,%%ymm2 ,%%ymm12 		\n\t"/* _e = t10-t11; */\
		"vaddpd		%%ymm2 ,%%ymm3 ,%%ymm13			\n\t"/* _f = t10+t11; */\
		"vmovaps			 %%ymm4 ,%%ymm10 		\n\t	vmovaps				 %%ymm7 ,%%ymm8 		\n\t"/* _c = t18; _a = t27; */\
		"vfnmadd231pd	%%ymm5 ,%%ymm15,%%ymm10		\n\t	 vfmsub231pd	%%ymm6 ,%%ymm15,%%ymm8 		\n\t"/*	FNMA231(t19,__sc,_c );		 FMS231(t26,__sc,_a ); */\
		"vmovaps			 %%ymm5 ,%%ymm11 		\n\t	vmovaps				 %%ymm6 ,%%ymm9 		\n\t"/* _d = t19; _b = t26; */\
		"vbroadcastsd	0x018(%%rsi),%%ymm6 		\n\t"/* load __c2i2 */\
		" vfmadd231pd	%%ymm4 ,%%ymm15,%%ymm11		\n\t	 vfmadd231pd	%%ymm7 ,%%ymm15,%%ymm9 		\n\t"/*	 FMA231(t18,__sc,_d );		 FMA231(t27,__sc,_b ); */\
		"vbroadcastsd	(%%rsi),%%ymm15				\n\t"/* load __c1_c */\
		"vmovaps		     (%%rax),%%ymm0 		\n\t	vmovaps			0x020(%%rax),%%ymm1 		\n\t"/*    t02;    t03; */\
		"vmovaps			 %%ymm10,%%ymm4 		\n\t	vmovaps				 %%ymm0 ,%%ymm2 		\n\t"/* t18 = _c;	t10 = t02; */\
		" vfmadd231pd	%%ymm8 ,%%ymm14,%%ymm4 		\n\t	 vfmadd231pd	%%ymm12,%%ymm6 ,%%ymm0 		\n\t"/*	 FMA231(_a ,__c31,t18);		 FMA231(_e ,__c2i2,t02); */\
		"vmovaps			 %%ymm11,%%ymm5 		\n\t	vmovaps				 %%ymm1 ,%%ymm3 		\n\t"/* t19 = _d;	t11 = t03; */\
		" vfmadd231pd	%%ymm9 ,%%ymm14,%%ymm5 		\n\t	 vfmadd231pd	%%ymm13,%%ymm6 ,%%ymm1 		\n\t"/*	 FMA231(_b ,__c31,t19);		 FMA231(_f ,__c2i2,t03); */\
		"vfnmadd231pd	%%ymm8 ,%%ymm14,%%ymm10		\n\t	vfnmadd231pd	%%ymm12,%%ymm6 ,%%ymm2 		\n\t"/*	FNMA231(_a ,__c31,_c );		FNMA231(_e ,__c2i2,t10); */\
		"vfnmadd231pd	%%ymm9 ,%%ymm14,%%ymm11		\n\t	vfnmadd231pd	%%ymm13,%%ymm6 ,%%ymm3 		\n\t"/*	FNMA231(_b ,__c31,_d );		FNMA231(_f ,__c2i2,t11); */\
		"vmovaps			 %%ymm0 ,%%ymm8 		\n\t	vmovaps				 %%ymm1 ,%%ymm9 		\n\t"/* _a = t02; _b = t03; */\
		" vfmadd231pd	%%ymm4 ,%%ymm15,%%ymm0 		\n\t	 vfmadd231pd	%%ymm5 ,%%ymm15,%%ymm1 		\n\t"/*	 FMA231(t18,__c1_c,t02);	 FMA231(t19,__c1_c,t03); */\
		"vfnmadd231pd	%%ymm4 ,%%ymm15,%%ymm8 		\n\t	vfnmadd231pd	%%ymm5 ,%%ymm15,%%ymm9 		\n\t"/*	FNMA231(t18,__c1_c,_a );	FNMA231(t19,__c1_c,_b ); */\
		"vmovaps			 %%ymm2 ,%%ymm12		\n\t	vmovaps				 %%ymm3 ,%%ymm13		\n\t"/* _e = t10; _f = t11; */\
		"vfnmadd231pd	%%ymm11,%%ymm15,%%ymm2 		\n\t	 vfmadd231pd	%%ymm10,%%ymm15,%%ymm3 		\n\t"/*	FNMA231(_d ,__c1_c,t10);	 FMA231(_c ,__c1_c,t11); */\
		" vfmadd231pd	%%ymm11,%%ymm15,%%ymm12		\n\t	vfnmadd231pd	%%ymm10,%%ymm15,%%ymm13		\n\t"/*	 FMA231(_d ,__c1_c,_e );	FNMA231(_c ,__c1_c,_f ); */\
		/* Write outputs: */\
		"movq	%[__out8],%%r10		\n\t"\
		"movq	%[__out9],%%r11		\n\t"\
		"movq	%[__outa],%%r12		\n\t"\
		"movq	%[__outb],%%r13		\n\t"\
		"vmovaps		%%ymm0 ,     (%%r10)		\n\t	vmovaps			%%ymm1 ,0x020(%%r10)		\n\t"/* __B8r= t02;		__B8i= t03; */\
		"vmovaps		%%ymm8 ,     (%%r11)		\n\t	vmovaps			%%ymm9 ,0x020(%%r11)		\n\t"/* __B9r= _a ;		__B9i= _b ; */\
		"vmovaps		%%ymm2 ,     (%%r12)		\n\t	vmovaps			%%ymm3 ,0x020(%%r12)		\n\t"/* __BAr= t10;		__BAi= t11; */\
		"vmovaps		%%ymm12,     (%%r13)		\n\t	vmovaps			%%ymm13,0x020(%%r13)		\n\t"/* __BBr= _e ;		__BBi= _f ; */\
		"\n\t"\
		/*...Block 3: t6,14,22,30 */\
		"addq	$%c[__i1],%%rax	\n\t"/* __in0 + 3*istride */\
		"addq	$%c[__i1],%%rbx	\n\t"/* __in0 + 7*istride */\
		"addq	$%c[__i1],%%rcx	\n\t"/* __in0 + b*istride */\
		"addq	$%c[__i1],%%rdx	\n\t"/* __in0 + f*istride */\
		"vmovaps		     (%%rbx),%%ymm2 		\n\t	vmovaps			0x020(%%rbx),%%ymm3 		\n\t"/*    t14;    t15; */\
		"vbroadcastsd	0x008(%%rsi),%%ymm15		\n\t"/* load __sc  */\
		"vmovaps		     (%%rcx),%%ymm4 		\n\t	vmovaps			0x020(%%rcx),%%ymm5 		\n\t"/*    t22;    t23; */\
		"vmovaps		     (%%rdx),%%ymm6 		\n\t	vmovaps			0x020(%%rdx),%%ymm7 		\n\t"/*    t30;    t31; */\
		"vaddpd		%%ymm2 ,%%ymm3 ,%%ymm10 		\n\t"/* _c = t14+t15; */\
		"vsubpd		%%ymm2 ,%%ymm3 ,%%ymm11			\n\t"/* _d = t15-t14; */\
		"vbroadcastsd	0x0d0(%%rsi),%%ymm14		\n\t"/* load __c31 */\
		"vmovaps			 %%ymm5 ,%%ymm12 		\n\t	vmovaps				 %%ymm4 ,%%ymm13		\n\t"/* _e = t23; _f = t22;*/\
		" vfmsub231pd	%%ymm4 ,%%ymm15,%%ymm12		\n\t	 vfmadd231pd	%%ymm5 ,%%ymm15,%%ymm13		\n\t"/*	 FMS231(t22,__sc,_e );		 FMA231(t23,__sc,_f );*/\
		"vmovaps			 %%ymm6 ,%%ymm8  		\n\t	vmovaps				 %%ymm7 ,%%ymm9 		\n\t"/* _a = t30; _b = t31; */\
		"vfnmadd231pd	%%ymm7 ,%%ymm15,%%ymm8 		\n\t	 vfmadd231pd	%%ymm6 ,%%ymm15,%%ymm9 		\n\t"/*	FNMA231(t31,__sc,_a );		 FMA231(t30,__sc,_b );*/\
		"vbroadcastsd	0x018(%%rsi),%%ymm6 		\n\t"/* load __c2i2 */\
		"vbroadcastsd	(%%rsi),%%ymm15				\n\t"/* load __c1_c */\
		"vmovaps		     (%%rax),%%ymm0 		\n\t	vmovaps			0x020(%%rax),%%ymm1 		\n\t"/*    t06;    t07; */\
		"vmovaps			 %%ymm1 ,%%ymm3 		\n\t	vmovaps				 %%ymm0 ,%%ymm2 		\n\t"/* t15= t07;	t14= t06; */\
		"vfnmadd231pd	%%ymm11,%%ymm6 ,%%ymm1 		\n\t	vfnmadd231pd	%%ymm10,%%ymm6 ,%%ymm0 		\n\t"/*	FNMA231(_d ,__c2i2,t07);	FNMA231(_c ,__c2i2,t06); */\
		"vmovaps			 %%ymm12,%%ymm4 		\n\t	vmovaps				 %%ymm13,%%ymm5 		\n\t"/* t22= _e; t23= _f; */\
		"vfnmadd231pd	%%ymm8 ,%%ymm14,%%ymm4 		\n\t	vfnmadd231pd	%%ymm9 ,%%ymm14,%%ymm5 		\n\t"/*	FNMA231(_a ,__c31 ,t22);	FNMA231(_b ,__c31 ,t23); */\
		" vfmadd231pd	%%ymm8 ,%%ymm14,%%ymm12		\n\t	 vfmadd231pd	%%ymm9 ,%%ymm14,%%ymm13		\n\t"/*	 FMA231(_a ,__c31 ,_e );	 FMA231(_b ,__c31 ,_f ); */\
		" vfmadd231pd	%%ymm10,%%ymm6 ,%%ymm2 		\n\t	 vfmadd231pd	%%ymm11,%%ymm6 ,%%ymm3 		\n\t"/*	 FMA231(_c ,__c2i2,t14);	 FMA231(_d ,__c2i2,t15); */\
		"vmovaps			 %%ymm0 ,%%ymm8 		\n\t	vmovaps				 %%ymm1 ,%%ymm9 		\n\t"/* _a = t06; _b = t07; */\
		" vfmadd231pd	%%ymm4 ,%%ymm15,%%ymm0 		\n\t	 vfmadd231pd	%%ymm5 ,%%ymm15,%%ymm1 		\n\t"/*	 FMA231(t22,__c1_c,t06);	 FMA231(t23,__c1_c,t07); */\
		"vfnmadd231pd	%%ymm4 ,%%ymm15,%%ymm8 		\n\t	vfnmadd231pd	%%ymm5 ,%%ymm15,%%ymm9 		\n\t"/*	FNMA231(t22,__c1_c,_a );	FNMA231(t23,__c1_c,_b ); */\
		"vmovaps			 %%ymm2 ,%%ymm10		\n\t	vmovaps				 %%ymm3 ,%%ymm11		\n\t"/* _c = t14; _d = t15; */\
		"vfnmadd231pd	%%ymm13,%%ymm15,%%ymm2 		\n\t	 vfmadd231pd	%%ymm12,%%ymm15,%%ymm3 		\n\t"/*	FNMA231(_f ,__c1_c,t14);	 FMA231(_e ,__c1_c,t15); */\
		" vfmadd231pd	%%ymm13,%%ymm15,%%ymm10		\n\t	vfnmadd231pd	%%ymm12,%%ymm15,%%ymm11		\n\t"/*	 FMA231(_f ,__c1_c,_c );	FNMA231(_e ,__c1_c,_d ); */\
		/* Write outputs: */\
		"movq	%[__outc],%%r10		\n\t"\
		"movq	%[__outd],%%r11		\n\t"\
		"movq	%[__oute],%%r12		\n\t"\
		"movq	%[__outf],%%r13		\n\t"\
		"vmovaps		%%ymm0 ,     (%%r10)		\n\t	vmovaps			%%ymm1 ,0x020(%%r10)		\n\t"/* __BCr= t06;		__BCi= t07; */\
		"vmovaps		%%ymm8 ,     (%%r11)		\n\t	vmovaps			%%ymm9 ,0x020(%%r11)		\n\t"/* __BDr= _a ;		__BDi= _b ; */\
		"vmovaps		%%ymm2 ,     (%%r12)		\n\t	vmovaps			%%ymm3 ,0x020(%%r12)		\n\t"/* __BEr= t14;		__BEi= t15; */\
		"vmovaps		%%ymm10,     (%%r13)		\n\t	vmovaps			%%ymm11,0x020(%%r13)		\n\t"/* __BFr= _c ;		__BFi= _d ; */\
		:					/* outputs: none */\
		: [__in0] "m" (Xin0)	/* All inputs from memory addresses here */\
		 ,[__i1] "e" (Xi1)\
		 ,[__i2] "e" (Xi2)\
		 ,[__i3] "e" (Xi3)\
		 ,[__i4] "e" (Xi4)\
		 ,[__out0] "m" (Xout0)\
		 ,[__out1] "m" (Xout1)\
		 ,[__out2] "m" (Xout2)\
		 ,[__out3] "m" (Xout3)\
		 ,[__out4] "m" (Xout4)\
		 ,[__out5] "m" (Xout5)\
		 ,[__out6] "m" (Xout6)\
		 ,[__out7] "m" (Xout7)\
		 ,[__out8] "m" (Xout8)\
		 ,[__out9] "m" (Xout9)\
		 ,[__outa] "m" (Xouta)\
		 ,[__outb] "m" (Xoutb)\
		 ,[__outc] "m" (Xoutc)\
		 ,[__outd] "m" (Xoutd)\
		 ,[__oute] "m" (Xoute)\
		 ,[__outf] "m" (Xoutf)\
		 ,[__cc0] "m" (Xcc0)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"	/* Clobbered registers */\
	);\
	}

	#define SSE2_RADIX16_DIT_FMA_OOP(Xin0,Xi1,Xi2,Xi3,Xi4, Xout0,Xo1,Xo2,Xo3,Xo4, Xcc0)\
	{\
	__asm__ volatile (\
	/*...Block 0: Do in-place, i.e. outputs into __in0 + [0,1,2,3]*istride: */\
		"movq	%[__in0],%%rax		\n\t"\
		"leaq	%c[__i1](%%rax),%%rcx	\n\t"/* __in0 +   istride */\
		"leaq	%c[__i2](%%rax),%%rbx	\n\t"/* __in0 + 2*istride */\
		"leaq	%c[__i3](%%rax),%%rdx	\n\t"/* __in0 + 3*istride */\
		"movq	%[__cc0],%%rsi 			\n\t"\
		"vbroadcastsd 0x28(%%rsi),%%ymm13 \n\t vbroadcastsd 0x38(%%rsi),%%ymm14 \n\t vbroadcastsd 0x48(%%rsi),%%ymm15 \n\t"/* load __r8,r4,rC into ymm13-15 */\
		"vmovaps		     (%%rcx),%%ymm4 		\n\t	vmovaps			0x020(%%rcx),%%ymm5 		\n\t"/*	t04 =__A8r;					t05 =__A8i; */\
		"vmovaps		     (%%rax),%%ymm0 		\n\t	vmovaps			0x020(%%rax),%%ymm1 		\n\t"/*	t00 =__A0r;					t01 =__A0i; */\
		"vmovaps		%%ymm4,%%ymm6				\n\t"/*	t06 = t04; */\
		"vfmadd231pd	%%ymm5 ,%%ymm13,%%ymm4 		\n\t	vfnmadd231pd	%%ymm6 ,%%ymm13,%%ymm5 		\n\t"/*	FNMA231(  t05,__r8,t04);	 FMA231(  t06,__r8,t05); */\
		"vmovaps		     (%%rbx),%%ymm8			\n\t	vmovaps			0x020(%%rbx),%%ymm9 		\n\t"/*	_a =__A4r;					_b =__A4i; */\
		"vfmadd231pd	0x020(%%rbx),%%ymm14,%%ymm8 \n\t	vfnmadd231pd	     (%%rbx),%%ymm14,%%ymm9 \n\t"/*	FNMA231(__A4i,__r4,_a );	 FMA231(__A4r,__r4,_b ); */\
		"vbroadcastsd	0x040(%%rsi),%%ymm13		\n\t	vbroadcastsd	0x020(%%rsi),%%ymm14		\n\t"/* load __cC4,c8 into pair of regs */\
		"vmovaps		     (%%rdx),%%ymm6			\n\t	vmovaps			0x020(%%rdx),%%ymm7 		\n\t"/*	t06 =__ACr;					t07 =__ACi; */\
		"vfmadd231pd	0x020(%%rdx),%%ymm15,%%ymm6 \n\t	vfnmadd231pd	     (%%rdx),%%ymm15,%%ymm7 \n\t"/*	FNMA231(__ACi,__rC,t06);	 FMA231(__ACr,__rC,t07); */\
		"vbroadcastsd	0x030(%%rsi),%%ymm15		\n\t"/* load __c4 */\
		"vmovaps		%%ymm8 ,%%ymm10				\n\t	vmovaps			%%ymm0,%%ymm2 				\n\t"/*	_c = _a;	t02 = t00; */\
		" vfmadd231pd	%%ymm6 ,%%ymm13,%%ymm8 		\n\t	 vfmadd231pd	%%ymm4 ,%%ymm14,%%ymm0 		\n\t"/*	 FMA231(t06,__cC4,_a);		 FMA231(t04,__c8,t00); */\
		"vmovaps		%%ymm9 ,%%ymm11				\n\t	vmovaps			%%ymm1 ,%%ymm3 				\n\t"/*	_d = _b;	t03 = t01; */\
		" vfmadd231pd	%%ymm7 ,%%ymm13,%%ymm9 		\n\t	 vfmadd231pd	%%ymm5 ,%%ymm14,%%ymm1 		\n\t"/*	 FMA231(t07,__cC4,_b);		 FMA231(t05,__c8,t01); */\
		"vfnmadd231pd	%%ymm6 ,%%ymm13,%%ymm10		\n\t	vfnmadd231pd	%%ymm4 ,%%ymm14,%%ymm2 		\n\t"/*	FNMA231(t06,__cC4,_c);		FNMA231(t04,__c8,t02); */\
		"vfnmadd231pd	%%ymm7 ,%%ymm13,%%ymm11		\n\t	vfnmadd231pd	%%ymm5 ,%%ymm14,%%ymm3 		\n\t"/*	FNMA231(t07,__cC4,_d);		FNMA231(t05,__c8,t03); */\
		"vmovaps		%%ymm0 ,%%ymm4 				\n\t	vmovaps			%%ymm1 ,%%ymm5 				\n\t"/*	t04 =t00; t05 =t01; */\
		"vfnmadd231pd	%%ymm8 ,%%ymm15,%%ymm4 		\n\t	 vfmadd231pd	%%ymm8 ,%%ymm15,%%ymm0 		\n\t"/*	FNMA231(_a ,__c4 ,t04);		 FMA231(_a ,__c4 ,t00); */\
		"vfnmadd231pd	%%ymm9 ,%%ymm15,%%ymm5 		\n\t	 vfmadd231pd	%%ymm9 ,%%ymm15,%%ymm1 		\n\t"/*	FNMA231(_b ,__c4 ,t05);		 FMA231(_b ,__c4 ,t01); */\
		"vmovaps		%%ymm2 ,%%ymm6 				\n\t	vmovaps			%%ymm3 ,%%ymm7 				\n\t"/*	t06 =t02;	t07 =t03; */\
		" vfmadd231pd	%%ymm11,%%ymm15,%%ymm2 		\n\t	vfnmadd231pd	%%ymm11,%%ymm15,%%ymm6 		\n\t"/*	 FMA231(_d ,__c4 ,t06);		FNMA231(_d ,__c4 ,t02); */\
		"vfnmadd231pd	%%ymm10,%%ymm15,%%ymm3 		\n\t	 vfmadd231pd	%%ymm10,%%ymm15,%%ymm7 		\n\t"/*	FNMA231(_c ,__c4 ,t07);		 FMA231(_c ,__c4 ,t03); */\
		"vmovaps		%%ymm4 ,     (%%rcx)		\n\t	vmovaps			%%ymm0 ,     (%%rax)		\n\t"/* Write outputs into local store */\
		"vmovaps		%%ymm5 ,0x020(%%rcx)		\n\t	vmovaps			%%ymm1 ,0x020(%%rax)		\n\t"\
		"vmovaps		%%ymm6 ,     (%%rdx)		\n\t	vmovaps			%%ymm2 ,     (%%rbx)		\n\t"\
		"vmovaps		%%ymm7 ,0x020(%%rdx)		\n\t	vmovaps			%%ymm3 ,0x020(%%rbx)		\n\t"\
		"\n\t"\
	/*...Block 1: outputs into __in0 + [4,5,6,7]*istride: */\
		"addq	$%c[__i4],%%rax	\n\t"/* __in0 + 4*istride */\
		"addq	$%c[__i4],%%rcx	\n\t"/* __in0 + 5*istride */\
		"addq	$%c[__i4],%%rbx	\n\t"/* __in0 + 6*istride */\
		"addq	$%c[__i4],%%rdx	\n\t"/* __in0 + 7*istride */\
		"addq	$0x040,%%rsi 		/* cc += 8 */\n\t"\
		"vbroadcastsd 0x018(%%rsi),%%ymm12 	\n\t"/* load __r2 into ymm12 */\
		"vbroadcastsd 0x028(%%rsi),%%ymm13 	\n\t"/* load __rA into ymm13 */\
		"vbroadcastsd 0x038(%%rsi),%%ymm14 	\n\t"/* load __r6 into ymm14 */\
		"vbroadcastsd 0x048(%%rsi),%%ymm15 	\n\t"/* load __rE into ymm15 */\
		"vmovaps (%%rax),%%ymm0  \n\t vmovaps 0x020(%%rax),%%ymm1  \n\t"/* t08 =__A2r; t09 =__A2i; */\
		"vmovaps (%%rcx),%%ymm4  \n\t vmovaps 0x020(%%rcx),%%ymm5  \n\t"/* t12 =__AAr; t13 =__AAi; */\
		"vmovaps (%%rbx),%%ymm8  \n\t vmovaps 0x020(%%rbx),%%ymm9  \n\t"/* _a  =__A6r; _b  =__A6i; */\
		"vmovaps (%%rdx),%%ymm6  \n\t vmovaps 0x020(%%rdx),%%ymm7  \n\t"/* t14 =__AEr; t15 =__AEi; */\
		"vfmadd231pd	0x020(%%rax),%%ymm12,%%ymm0 \n\t	vfnmadd231pd	(%%rax),%%ymm12,%%ymm1  	\n\t"/* FNMA231(__A2i,__r2,t08); FMA231(__A2r,__r2,t09); */\
		"vfmadd231pd	0x020(%%rcx),%%ymm13,%%ymm4 \n\t	vfnmadd231pd	(%%rcx),%%ymm13,%%ymm5  	\n\t"/* FNMA231(__AAi,__rA,t12); FMA231(__AAr,__rA,t13); */\
		"vbroadcastsd	0x040(%%rsi),%%ymm13		\n\t"/* load __cE6 */\
		"vfmadd231pd	0x020(%%rbx),%%ymm14,%%ymm8 \n\t	vfnmadd231pd	(%%rbx),%%ymm14,%%ymm9  	\n\t"/* FNMA231(__A6i,__r6,_a ); FMA231(__A6r,__r6,_b ); */\
		"vbroadcastsd	0x020(%%rsi),%%ymm14		\n\t"/* load __cA2 */\
		"vfmadd231pd	0x020(%%rdx),%%ymm15,%%ymm6 \n\t	vfnmadd231pd	(%%rdx),%%ymm15,%%ymm7  	\n\t"/* FNMA231(__AEi,__rE,t14); FMA231(__AEr,__rE,t15); */\
		"vbroadcastsd	0x030(%%rsi),%%ymm15		\n\t"/* load __c62 */\
		"vmovaps		%%ymm8 ,%%ymm10				\n\t	vmovaps			%%ymm0,%%ymm2 				\n\t"/*	_c = _a;	t10 = t08; */\
		" vfmadd231pd	%%ymm6 ,%%ymm13,%%ymm8 		\n\t	 vfmadd231pd	%%ymm4 ,%%ymm14,%%ymm0 		\n\t"/*	 FMA231(t14,__cE6,_a);		 FMA231(t12,__cA2,t08); */\
		"vmovaps		%%ymm9 ,%%ymm11				\n\t	vmovaps			%%ymm1 ,%%ymm3 				\n\t"/*	_d = _b;	t11 = t09; */\
		" vfmadd231pd	%%ymm7 ,%%ymm13,%%ymm9 		\n\t	 vfmadd231pd	%%ymm5 ,%%ymm14,%%ymm1 		\n\t"/*	 FMA231(t15,__cE6,_b);		 FMA231(t13,__cA2,t09); */\
		"vfnmadd231pd	%%ymm6 ,%%ymm13,%%ymm10		\n\t	vfnmadd231pd	%%ymm4 ,%%ymm14,%%ymm2 		\n\t"/*	FNMA231(t14,__cE6,_c);		FNMA231(t12,__cA2,t10); */\
		"vfnmadd231pd	%%ymm7 ,%%ymm13,%%ymm11		\n\t	vfnmadd231pd	%%ymm5 ,%%ymm14,%%ymm3 		\n\t"/*	FNMA231(t15,__cE6,_d);		FNMA231(t13,__cA2,t11); */\
		"vmovaps		%%ymm0 ,%%ymm4 				\n\t	vmovaps			%%ymm1 ,%%ymm5 				\n\t"/*	t12 =t08 ;	t13 =t09; */\
		"vfnmadd231pd	%%ymm8 ,%%ymm15,%%ymm4 		\n\t	 vfmadd231pd	%%ymm8 ,%%ymm15,%%ymm0 		\n\t"/*	FNMA231(_a,__c62,t12);		 FMA231( _a,__c62,t08); */\
		"vfnmadd231pd	%%ymm9 ,%%ymm15,%%ymm5 		\n\t	 vfmadd231pd	%%ymm9 ,%%ymm15,%%ymm1 		\n\t"/*	FNMA231(_b,__c62,t13);		 FMA231( _b,__c62,t09); */\
		"vmovaps		%%ymm2 ,%%ymm6 				\n\t	vmovaps			%%ymm3 ,%%ymm7 				\n\t"/*	t14 =t10;	t15 =t11; */\
		" vfmadd231pd	%%ymm11,%%ymm15,%%ymm2 		\n\t	vfnmadd231pd	%%ymm11,%%ymm15,%%ymm6 		\n\t"/*	 FMA231(_d,__c62,t14);		FNMA231( _d,__c62,t10); */\
		"vfnmadd231pd	%%ymm10,%%ymm15,%%ymm3 		\n\t	 vfmadd231pd	%%ymm10,%%ymm15,%%ymm7 		\n\t"/*	FNMA231(_c,__c62,t15);		 FMA231( _c,__c62,t11); */\
		"vmovaps		%%ymm4 ,     (%%rcx)		\n\t	vmovaps			%%ymm0 ,     (%%rax)		\n\t"/* Write outputs into local store */\
		"vmovaps		%%ymm5 ,0x020(%%rcx)		\n\t	vmovaps			%%ymm1 ,0x020(%%rax)		\n\t"\
		"vmovaps		%%ymm6 ,     (%%rdx)		\n\t	vmovaps			%%ymm2 ,     (%%rbx)		\n\t"\
		"vmovaps		%%ymm7 ,0x020(%%rdx)		\n\t	vmovaps			%%ymm3 ,0x020(%%rbx)		\n\t"\
		"\n\t"\
	/*...Block 2: outputs into __in0 + [8,9,a,b]*istride: */\
		"addq	$%c[__i4],%%rax	\n\t"/* __in0 + 8*istride */\
		"addq	$%c[__i4],%%rcx	\n\t"/* __in0 + 9*istride */\
		"addq	$%c[__i4],%%rbx	\n\t"/* __in0 + a*istride */\
		"addq	$%c[__i4],%%rdx	\n\t"/* __in0 + b*istride */\
		"addq	$0x040,%%rsi 		/* cc += 8 */\n\t"\
		"vbroadcastsd 0x018(%%rsi),%%ymm12 	\n\t"/* load __r1 into ymm12 */\
		"vbroadcastsd 0x028(%%rsi),%%ymm13 	\n\t"/* load __r9 into ymm13 */\
		"vbroadcastsd 0x038(%%rsi),%%ymm14 	\n\t"/* load __r5 into ymm14 */\
		"vbroadcastsd 0x048(%%rsi),%%ymm15 	\n\t"/* load __rD into ymm15 */\
		"vmovaps (%%rax),%%ymm0  \n\t vmovaps 0x020(%%rax),%%ymm1  \n\t"/* t16 =__A1r;	t17 =__A1i; */\
		"vmovaps (%%rcx),%%ymm4  \n\t vmovaps 0x020(%%rcx),%%ymm5  \n\t"/* t20 =__A9r;	t21 =__A9i; */\
		"vmovaps (%%rbx),%%ymm8  \n\t vmovaps 0x020(%%rbx),%%ymm9  \n\t"/* _a=  __A5r;	_b  =__A5i; */\
		"vmovaps (%%rdx),%%ymm6  \n\t vmovaps 0x020(%%rdx),%%ymm7  \n\t"/* t22 =__ADr;	t23 =__ADi; */\
		"vfmadd231pd	0x020(%%rax),%%ymm12,%%ymm0 \n\t	vfnmadd231pd	(%%rax),%%ymm12,%%ymm1  	\n\t"/* FNMA231(__A1i,__r1,t16);	 FMA231(__A1r,__r1,t17); */\
		"vfmadd231pd	0x020(%%rcx),%%ymm13,%%ymm4 \n\t	vfnmadd231pd	(%%rcx),%%ymm13,%%ymm5  	\n\t"/* FNMA231(__A9i,__r9,t20);	 FMA231(__A9r,__r9,t21); */\
		"vbroadcastsd	0x040(%%rsi),%%ymm13		\n\t"/* load __cD5 */\
		"vfmadd231pd	0x020(%%rbx),%%ymm14,%%ymm8 \n\t	vfnmadd231pd	(%%rbx),%%ymm14,%%ymm9  	\n\t"/* FNMA231(__A5i,__r5,_a );	 FMA231(__A5r,__r5,_b ); */\
		"vbroadcastsd	0x020(%%rsi),%%ymm14		\n\t"/* load __c91 */\
		"vfmadd231pd	0x020(%%rdx),%%ymm15,%%ymm6 \n\t	vfnmadd231pd	(%%rdx),%%ymm15,%%ymm7  	\n\t"/* FNMA231(__ADi,__rD,t22);	 FMA231(__ADr,__rD,t23); */\
		"vbroadcastsd	0x030(%%rsi),%%ymm15		\n\t"/* load __c51 */\
		"vmovaps		%%ymm8 ,%%ymm10				\n\t	vmovaps			%%ymm0,%%ymm2 				\n\t"/*	_c= _a;	t18= t16; */\
		" vfmadd231pd	%%ymm6 ,%%ymm13,%%ymm8 		\n\t	 vfmadd231pd	%%ymm4 ,%%ymm14,%%ymm0 		\n\t"/*	 FMA231(t22,__cD5,_a);		 FMA231(t20,__c91,t16); */\
		"vmovaps		%%ymm9 ,%%ymm11				\n\t	vmovaps			%%ymm1 ,%%ymm3 				\n\t"/*	_d= _b;	t19= t17; */\
		" vfmadd231pd	%%ymm7 ,%%ymm13,%%ymm9 		\n\t	 vfmadd231pd	%%ymm5 ,%%ymm14,%%ymm1 		\n\t"/*	 FMA231(t23,__cD5,_b);		 FMA231(t21,__c91,t17); */\
		"vfnmadd231pd	%%ymm6 ,%%ymm13,%%ymm10		\n\t	vfnmadd231pd	%%ymm4 ,%%ymm14,%%ymm2 		\n\t"/*	FNMA231(t22,__cD5,_c);		FNMA231(t20,__c91,t18); */\
		"vfnmadd231pd	%%ymm7 ,%%ymm13,%%ymm11		\n\t	vfnmadd231pd	%%ymm5 ,%%ymm14,%%ymm3 		\n\t"/*	FNMA231(t23,__cD5,_d);		FNMA231(t21,__c91,t19); */\
		"vmovaps		%%ymm0 ,%%ymm4 				\n\t	vmovaps			%%ymm1 ,%%ymm5 				\n\t"/*	t20 =t16;	t21 =t17; */\
		"vfnmadd231pd	%%ymm8 ,%%ymm15,%%ymm4 		\n\t	 vfmadd231pd	%%ymm8 ,%%ymm15,%%ymm0 		\n\t"/*	FNMA231(_a,__c51,t20);		 FMA231(_a,__c51,t16); */\
		"vfnmadd231pd	%%ymm9 ,%%ymm15,%%ymm5 		\n\t	 vfmadd231pd	%%ymm9 ,%%ymm15,%%ymm1 		\n\t"/*	FNMA231(_b,__c51,t21);		 FMA231(_b,__c51,t17); */\
		"vmovaps		%%ymm2 ,%%ymm6 				\n\t	vmovaps			%%ymm3 ,%%ymm7 				\n\t"/*	t22 =t18;	t23 =t19; */\
		" vfmadd231pd	%%ymm11,%%ymm15,%%ymm2 		\n\t	vfnmadd231pd	%%ymm11,%%ymm15,%%ymm6 		\n\t"/*	 FMA231(_d,__c51,t22);		FNMA231(_d,__c51,t18); */\
		"vfnmadd231pd	%%ymm10,%%ymm15,%%ymm3 		\n\t	 vfmadd231pd	%%ymm10,%%ymm15,%%ymm7 		\n\t"/*	FNMA231(_c,__c51,t23);		 FMA231(_c,__c51,t19); */\
		"vmovaps		%%ymm4 ,     (%%rcx)		\n\t	vmovaps			%%ymm0 ,     (%%rax)		\n\t"/* Write outputs into local store */\
		"vmovaps		%%ymm5 ,0x020(%%rcx)		\n\t	vmovaps			%%ymm1 ,0x020(%%rax)		\n\t"\
		"vmovaps		%%ymm6 ,     (%%rdx)		\n\t	vmovaps			%%ymm2 ,     (%%rbx)		\n\t"\
		"vmovaps		%%ymm7 ,0x020(%%rdx)		\n\t	vmovaps			%%ymm3 ,0x020(%%rbx)		\n\t"\
		"\n\t"\
	/*...Block 3: outputs into __in0 + [c,d,e,f]*istride: */\
		"addq	$%c[__i4],%%rax	\n\t"/* __in0 + c*istride */\
		"addq	$%c[__i4],%%rcx	\n\t"/* __in0 + d*istride */\
		"addq	$%c[__i4],%%rbx	\n\t"/* __in0 + e*istride */\
		"addq	$%c[__i4],%%rdx	\n\t"/* __in0 + f*istride */\
		"addq	$0x040,%%rsi 		/* cc += 8 */\n\t"\
		"vbroadcastsd 0x018(%%rsi),%%ymm12 	\n\t"/* load __r3 into ymm12 */\
		"vbroadcastsd 0x028(%%rsi),%%ymm13 	\n\t"/* load __rB into ymm13 */\
		"vbroadcastsd 0x038(%%rsi),%%ymm14 	\n\t"/* load __r7 into ymm14 */\
		"vbroadcastsd 0x048(%%rsi),%%ymm15 	\n\t"/* load __rF into ymm15 */\
		"vmovaps (%%rax),%%ymm0  \n\t vmovaps 0x020(%%rax),%%ymm1  \n\t"/* t24 =__A3r;	t25 =__A3i; */\
		"vmovaps (%%rcx),%%ymm4  \n\t vmovaps 0x020(%%rcx),%%ymm5  \n\t"/* t28 =__ABr;	t29 =__ABi; */\
		"vmovaps (%%rbx),%%ymm8  \n\t vmovaps 0x020(%%rbx),%%ymm9  \n\t"/* _a  =__A7r;	_b  =__A7i; */\
		"vmovaps (%%rdx),%%ymm6  \n\t vmovaps 0x020(%%rdx),%%ymm7  \n\t"/* t30 =__AFr;	t31 =__AFi; */\
		"vfmadd231pd	0x020(%%rax),%%ymm12,%%ymm0 \n\t	vfnmadd231pd	(%%rax),%%ymm12,%%ymm1  	\n\t"/* FNMA231(__A3i,__r3,t24);	 FMA231(__A3r,__r3,t25); */\
		"vfmadd231pd	0x020(%%rcx),%%ymm13,%%ymm4 \n\t	vfnmadd231pd	(%%rcx),%%ymm13,%%ymm5  	\n\t"/* FNMA231(__ABi,__rB,t28 );	 FMA231(__ABr,__rB,t29 ); */\
		"vbroadcastsd	0x040(%%rsi),%%ymm13		\n\t"/* load __cF7 */\
		"vfmadd231pd	0x020(%%rbx),%%ymm14,%%ymm8 \n\t	vfnmadd231pd	(%%rbx),%%ymm14,%%ymm9  	\n\t"/* FNMA231(__A7i,__r7,_a);		 FMA231(__A7r,__r7,_b); */\
		"vbroadcastsd	0x020(%%rsi),%%ymm14		\n\t"/* load __cB3 */\
		"vfmadd231pd	0x020(%%rdx),%%ymm15,%%ymm6 \n\t	vfnmadd231pd	(%%rdx),%%ymm15,%%ymm7  	\n\t"/* FNMA231(__AFi,__rF,t30 );	 FMA231(__AFr,__rF,t31 ); */\
		"vbroadcastsd	0x030(%%rsi),%%ymm15		\n\t"/* load __c73 */\
		"vmovaps		%%ymm8 ,%%ymm10				\n\t	vmovaps			%%ymm0,%%ymm2 				\n\t"/*	_c= _a;	t26= t24; */\
		" vfmadd231pd	%%ymm6 ,%%ymm13,%%ymm8 		\n\t	 vfmadd231pd	%%ymm4 ,%%ymm14,%%ymm0 		\n\t"/*	 FMA231(t30,__cF7,_a);		 FMA231(t28,__cB3,t24); */\
		"vmovaps		%%ymm9 ,%%ymm11				\n\t	vmovaps			%%ymm1 ,%%ymm3 				\n\t"/*	_d= _b;	t27= t25; */\
		" vfmadd231pd	%%ymm7 ,%%ymm13,%%ymm9 		\n\t	 vfmadd231pd	%%ymm5 ,%%ymm14,%%ymm1 		\n\t"/*	 FMA231(t31,__cF7,_b);		 FMA231(t29,__cB3,t25); */\
		"vfnmadd231pd	%%ymm6 ,%%ymm13,%%ymm10		\n\t	vfnmadd231pd	%%ymm4 ,%%ymm14,%%ymm2 		\n\t"/*	FNMA231(t30,__cF7,_c);		FNMA231(t28,__cB3,t26); */\
		"vfnmadd231pd	%%ymm7 ,%%ymm13,%%ymm11		\n\t	vfnmadd231pd	%%ymm5 ,%%ymm14,%%ymm3 		\n\t"/*	FNMA231(t31,__cF7,_d);		FNMA231(t29,__cB3,t27); */\
		"vmovaps		%%ymm0 ,%%ymm4 				\n\t	vmovaps			%%ymm1 ,%%ymm5 				\n\t"/*	t28 =t24;	t29 =t25; */\
		"vfnmadd231pd	%%ymm8 ,%%ymm15,%%ymm4 		\n\t	 vfmadd231pd	%%ymm8 ,%%ymm15,%%ymm0 		\n\t"/*	FNMA231(_a,__c73,t28);		 FMA231(_a,__c73,t24); */\
		"vfnmadd231pd	%%ymm9 ,%%ymm15,%%ymm5 		\n\t	 vfmadd231pd	%%ymm9 ,%%ymm15,%%ymm1 		\n\t"/*	FNMA231(_b,__c73,t29);		 FMA231(_b,__c73,t25); */\
		"vmovaps		%%ymm2 ,%%ymm6 				\n\t	vmovaps			%%ymm3 ,%%ymm7 				\n\t"/*	t30 =t26;	t31 =t27; */\
		" vfmadd231pd	%%ymm11,%%ymm15,%%ymm2 		\n\t	vfnmadd231pd	%%ymm11,%%ymm15,%%ymm6 		\n\t"/*	 FMA231(_d,__c73,t30);		FNMA231(_d,__c73,t26); */\
		"vfnmadd231pd	%%ymm10,%%ymm15,%%ymm3 		\n\t	 vfmadd231pd	%%ymm10,%%ymm15,%%ymm7 		\n\t"/*	FNMA231(_c,__c73,t31);		 FMA231(_c,__c73,t27); */\
		"vmovaps		%%ymm4 ,     (%%rcx)		\n\t	vmovaps			%%ymm0 ,     (%%rax)		\n\t"/* Write outputs into local store */\
		"vmovaps		%%ymm5 ,0x020(%%rcx)		\n\t	vmovaps			%%ymm1 ,0x020(%%rax)		\n\t"\
		"vmovaps		%%ymm6 ,     (%%rdx)		\n\t	vmovaps			%%ymm2 ,     (%%rbx)		\n\t"\
		"vmovaps		%%ymm7 ,0x020(%%rdx)		\n\t	vmovaps			%%ymm3 ,0x020(%%rbx)		\n\t"\
		"\n\t"\
	/*************************************************************************************/\
	/*  And now do four more radix-4 transforms, including the internal twiddle factors: */\
	/*************************************************************************************/\
	/* Block 0: Combine 0-output of each radix-4, i.e. inputs from __in0 + [0,4,8,c]*istride: */\
		"movq	%[__in0],%%rax		\n\t"\
		"leaq	%c[__i4](%%rax),%%rbx	\n\t"/* __in0 +   [4*istride] */\
		"leaq	%c[__i4](%%rbx),%%rcx	\n\t"/* __in0 + 2*[4*istride] */\
		"leaq	%c[__i4](%%rcx),%%rdx	\n\t"/* __in0 + 3*[4*istride] */\
		"subq	$0x0c0,%%rsi 		/* revert cc-ptr to base value */\n\t"\
		"\n\t"\
		/*...Read t0,8,16,24 from local store ... Do the 4 Im-part FMAs first, because their results needed 1st below */\
		"vbroadcastsd	0x050(%%rsi),%%ymm14		\n\t	vbroadcastsd	0x0d0(%%rsi),%%ymm15		\n\t"/* load __c2,c31 into pair of regs */\
		"vmovaps		     (%%rax),%%ymm0 		\n\t	vmovaps			     (%%rbx),%%ymm2 		\n\t"/*    t00;    t08; */\
		"vmovaps		     (%%rcx),%%ymm4 		\n\t	vmovaps			     (%%rdx),%%ymm6 		\n\t"/*    t16;    t24; */\
		"vmovaps			 %%ymm0 ,%%ymm8 		\n\t	vmovaps				 %%ymm4 ,%%ymm10		\n\t"/* _a=t00; _c=t16; */\
		" vfmadd231pd	%%ymm2 ,%%ymm14,%%ymm0 		\n\t	 vfmadd231pd	%%ymm6 ,%%ymm15,%%ymm4 		\n\t"/*	 FMA231(t08,__c2 ,t00);		 FMA231(t24,__c31,t16); */\
		"vmovaps		0x020(%%rax),%%ymm1 		\n\t	vmovaps			0x020(%%rbx),%%ymm3 		\n\t"/*    t01;    t09; */\
		"vmovaps		0x020(%%rcx),%%ymm5 		\n\t	vmovaps			0x020(%%rdx),%%ymm7 		\n\t"/*    t17;    t25; */\
		"vmovaps			 %%ymm1 ,%%ymm9 		\n\t	vmovaps				 %%ymm5 ,%%ymm11		\n\t"/* _b=t01; _d=t17; */\
		" vfmadd231pd	%%ymm3 ,%%ymm14,%%ymm1 		\n\t	 vfmadd231pd	%%ymm7 ,%%ymm15,%%ymm5 		\n\t"/*	 FMA231(t09,__c2 ,t01);		 FMA231(t25,__c31,t17); */\
		"vfnmadd231pd	%%ymm2 ,%%ymm14,%%ymm8 		\n\t	vfnmadd231pd	%%ymm6 ,%%ymm15,%%ymm10		\n\t"/*	FNMA231(t08,__c2 ,_a );		FNMA231(t24,__c31,_c ); */\
		"vbroadcastsd	0x090(%%rsi),%%ymm6 		\n\t"/* load __c1 */\
		"vfnmadd231pd	%%ymm3 ,%%ymm14,%%ymm9 		\n\t	vfnmadd231pd	%%ymm7 ,%%ymm15,%%ymm11		\n\t"/*	FNMA231(t09,__c2 ,_b );		FNMA231(t25,__c31,_d ); */\
		"vmovaps			 %%ymm0 ,%%ymm12		\n\t	vmovaps				 %%ymm1 ,%%ymm13		\n\t"/* _e = t00; _f = t01; */\
		" vfmadd231pd	%%ymm4 ,%%ymm6 ,%%ymm0 		\n\t	 vfmadd231pd	%%ymm5 ,%%ymm6 ,%%ymm1 		\n\t"/*	 FMA231(t16,__c1 ,t00);		 FMA231(t17,__c1 ,t01); */\
		"vfnmadd231pd	%%ymm4 ,%%ymm6 ,%%ymm12		\n\t	vfnmadd231pd	%%ymm5 ,%%ymm6 ,%%ymm13		\n\t"/*	FNMA231(t16,__c1 ,_e );		FNMA231(t17,__c1 ,_f ); */\
		"vmovaps			 %%ymm8 ,%%ymm2 		\n\t	vmovaps				 %%ymm9 ,%%ymm3 		\n\t"/* t08 = _a ; t09 = _b; */\
		"vfnmadd231pd	%%ymm11,%%ymm6 ,%%ymm2 		\n\t	 vfmadd231pd	%%ymm10,%%ymm6 ,%%ymm3 		\n\t"/*	FNMA231(_d ,__c1 ,t08);		 FMA231(_c ,__c1 ,t09); */\
		" vfmadd231pd	%%ymm11,%%ymm6 ,%%ymm8 		\n\t	vfnmadd231pd	%%ymm10,%%ymm6 ,%%ymm9 		\n\t"/*	 FMA231(_d ,__c1 ,_a );		FNMA231(_c ,__c1 ,_b ); */\
		/* Write outputs - Swap 4/C outputs for DIT */\
		"movq	%[__out0],%%r10		\n\t"\
		"leaq	%c[__o4](%%r10),%%r12	\n\t"/* __out0 +   [4*ostride] */\
		"leaq	%c[__o4](%%r12),%%r11	\n\t"/* __out0 + 2*[4*ostride] */\
		"leaq	%c[__o4](%%r11),%%r13	\n\t"/* __out0 + 3*[4*ostride] */\
		"vmovaps		%%ymm0 ,     (%%r10)		\n\t	vmovaps			%%ymm1 ,0x020(%%r10)		\n\t"/* __B0r= t00;		__B0i= t01; */\
		"vmovaps		%%ymm12,     (%%r11)		\n\t	vmovaps			%%ymm13,0x020(%%r11)		\n\t"/* __B8r= _e ;		__B8i= _f ; */\
		"vmovaps		%%ymm2 ,     (%%r13)		\n\t	vmovaps			%%ymm3 ,0x020(%%r13)		\n\t"/* __Bcr= t08;		__Bci= t09; */\
		"vmovaps		%%ymm8 ,     (%%r12)		\n\t	vmovaps			%%ymm9 ,0x020(%%r12)		\n\t"/* __B4r= _a ;		__B4i= _b ; */\
		"\n\t"\
		/*...Block 2: t4,12,20,28 */\
		"vbroadcastsd	0x110(%%rsi),%%ymm13	\n\t"/* cc0 + 0x22 = __two; Actually holds 1.0 in AVX2 mode */\
		"addq	$%c[__i1],%%rax	\n\t"/* __in0 + 1*istride */\
		"addq	$%c[__i1],%%rbx	\n\t"/* __in0 + 5*istride */\
		"addq	$%c[__i1],%%rcx	\n\t"/* __in0 + 9*istride */\
		"addq	$%c[__i1],%%rdx	\n\t"/* __in0 + d*istride */\
		"vbroadcastsd	0x050(%%rsi),%%ymm14		\n\t	vbroadcastsd	0x0d0(%%rsi),%%ymm15		\n\t"/* load __c2,31 into pair of regs */\
		"vmovaps		     (%%rcx),%%ymm4 		\n\t	vmovaps			0x020(%%rcx),%%ymm5 		\n\t"/*    t20;    t21; */\
		"vmovaps		     (%%rdx),%%ymm6 		\n\t	vmovaps			0x020(%%rdx),%%ymm7 		\n\t"/*    t28;    t29; */\
		"vmovaps			 %%ymm4 ,%%ymm10 		\n\t	vmovaps				 %%ymm7 ,%%ymm11		\n\t"/* _c=t20; _d=t29; */\
		" vfmadd231pd	%%ymm5 ,%%ymm13,%%ymm10		\n\t	 vfmsub231pd	%%ymm5 ,%%ymm13,%%ymm4 		\n\t"/*	FNMA231(t21,1.0,_c );		 FMA231(t21,1.0,t20); */\
		" vfmsub231pd	%%ymm6 ,%%ymm13,%%ymm11		\n\t	 vfmadd231pd	%%ymm6 ,%%ymm13,%%ymm7 		\n\t"/*	 FMA231(t28,1.0,_d );		FNMA231(t28,1.0,t29); */\
		"vbroadcastsd	0x010(%%rsi),%%ymm13		\n\t"/* load __c1i2 */\
		"vmovaps		     (%%rax),%%ymm0 		\n\t	vmovaps			0x020(%%rax),%%ymm1 		\n\t"/*    t04;    t05; */\
		"vmovaps		     (%%rbx),%%ymm2 		\n\t	vmovaps			0x020(%%rbx),%%ymm3 		\n\t"/*    t12;    t13; */\
		"vmovaps			 %%ymm0 ,%%ymm8 		\n\t	vmovaps				 %%ymm1 ,%%ymm9 		\n\t"/* _a = t04; _b = t05; */\
		"vmovaps			 %%ymm10,%%ymm5 		\n\t	vmovaps				 %%ymm4 ,%%ymm12		\n\t"/* t21 = _c; _e = t20; */\
		"vfnmadd231pd	%%ymm3 ,%%ymm14,%%ymm8 		\n\t	 vfmadd231pd	%%ymm11,%%ymm15,%%ymm5 		\n\t"/*	 FMA231(t13,__c2 ,_a );		 FMA231(_d ,__c31,t21); */\
		" vfmadd231pd	%%ymm2 ,%%ymm14,%%ymm9 		\n\t	 vfmadd231pd	%%ymm7 ,%%ymm15,%%ymm4 		\n\t"/*	FNMA231(t12,__c2 ,_b );		 FMA231(t29,__c31,t20); */\
		" vfmadd231pd	%%ymm3 ,%%ymm14,%%ymm0 		\n\t	vfnmadd231pd	%%ymm11,%%ymm15,%%ymm10		\n\t"/*	FNMA231(t13,__c2 ,t04);		FNMA231(_d ,__c31,_c ); */\
		"vfnmadd231pd	%%ymm2 ,%%ymm14,%%ymm1 		\n\t	vfnmadd231pd	%%ymm7 ,%%ymm15,%%ymm12		\n\t"/*	 FMA231(t12,__c2 ,t05);		FNMA231(t29,__c31,_e ); */\
		"vmovaps			 %%ymm8 ,%%ymm2 		\n\t	vmovaps				 %%ymm9 ,%%ymm3 		\n\t"/* t12 = _a; t13 = _b; */\
		"vfnmadd231pd	%%ymm4 ,%%ymm13,%%ymm2 		\n\t	 vfmadd231pd	%%ymm5 ,%%ymm13,%%ymm3 		\n\t"/*	FNMA231(t20,__c1i2,t12);	 FMA231(t21,__c1i2,t13); */\
		" vfmadd231pd	%%ymm4 ,%%ymm13,%%ymm8 		\n\t	vfnmadd231pd	%%ymm5 ,%%ymm13,%%ymm9 		\n\t"/*	 FMA231(t20,__c1i2,_a );	FNMA231(t21,__c1i2,_b ); */\
		"vmovaps			 %%ymm0 ,%%ymm11		\n\t	vmovaps				 %%ymm1 ,%%ymm7 		\n\t"/* _d = t04; t29 = t05; */\
		" vfmadd231pd	%%ymm10,%%ymm13,%%ymm0 		\n\t	 vfmadd231pd	%%ymm12,%%ymm13,%%ymm1 		\n\t"/*	 FMA231(_c ,__c1i2,t04);	 FMA231(_e ,__c1i2,t05); */\
		"vfnmadd231pd	%%ymm10,%%ymm13,%%ymm11		\n\t	vfnmadd231pd	%%ymm12,%%ymm13,%%ymm7 		\n\t"/*	FNMA231(_c ,__c1i2,_d );	FNMA231(_e ,__c1i2,t29); */\
		/* Write outputs - Not sure why, but need apply 6/E swap here *and* then pairwise swap, i.e. 2A[6][E] => [2A][E6] => E62A: */\
		"addq	$%c[__o2],%%r10	\n\t"/* __out0 + 2*ostride */\
		"addq	$%c[__o2],%%r12	\n\t"/* __out0 + 6*ostride */\
		"addq	$%c[__o2],%%r11	\n\t"/* __out0 + a*ostride */\
		"addq	$%c[__o2],%%r13	\n\t"/* __out0 + e*ostride */\
		"vmovaps		%%ymm2 ,     (%%r13)		\n\t	vmovaps			%%ymm3 ,0x020(%%r13)		\n\t"/* __BEr= t12;		__BEi= t13; */\
		"vmovaps		%%ymm8 ,     (%%r12)		\n\t	vmovaps			%%ymm9 ,0x020(%%r12)		\n\t"/* __B6r= _a ;		__B6i= _b ; */\
		"vmovaps		%%ymm0 ,     (%%r10)		\n\t	vmovaps			%%ymm1 ,0x020(%%r10)		\n\t"/* __B2r= t04;		__B2i= t05; */\
		"vmovaps		%%ymm11,     (%%r11)		\n\t	vmovaps			%%ymm7 ,0x020(%%r11)		\n\t"/* __BAr= _d ;		__BAi= t29; */\
		"\n\t"\
		/*...Block 1: t2,10,18,26 */\
		"addq	$%c[__i1],%%rax	\n\t"/* __in0 + 2*istride */\
		"addq	$%c[__i1],%%rbx	\n\t"/* __in0 + 6*istride */\
		"addq	$%c[__i1],%%rcx	\n\t"/* __in0 + a*istride */\
		"addq	$%c[__i1],%%rdx	\n\t"/* __in0 + e*istride */\
		"vmovaps		     (%%rbx),%%ymm2 		\n\t	vmovaps			0x020(%%rbx),%%ymm3 		\n\t"/*    t10;    t11; */\
		"vbroadcastsd	0x008(%%rsi),%%ymm15		\n\t"/* load __sc  */\
		"vbroadcastsd	0x0d0(%%rsi),%%ymm14		\n\t"/* load __c31 */\
		"vmovaps		     (%%rcx),%%ymm4 		\n\t	vmovaps			0x020(%%rcx),%%ymm5 		\n\t"/*    t18;    t19; */\
		"vmovaps		     (%%rdx),%%ymm6 		\n\t	vmovaps			0x020(%%rdx),%%ymm7 		\n\t"/*    t26;    t27; */\
		"vaddpd		%%ymm3 ,%%ymm2 ,%%ymm12 		\n\t"/* _e = t11+t10; */\
		"vsubpd		%%ymm2 ,%%ymm3 ,%%ymm13			\n\t"/* _f = t11-t10; */\
		"vmovaps			 %%ymm4 ,%%ymm10 		\n\t	vmovaps				 %%ymm7 ,%%ymm8 		\n\t"/* _c = t18; _a = t27; */\
		" vfmadd231pd	%%ymm5 ,%%ymm15,%%ymm10		\n\t	 vfmadd231pd	%%ymm6 ,%%ymm15,%%ymm8 		\n\t"/*	FNMA231(t19,__sc,_c );		 FMS231(t26,__sc,_a ); */\
		"vmovaps			 %%ymm5 ,%%ymm11 		\n\t	vmovaps				 %%ymm6 ,%%ymm9 		\n\t"/* _d = t19; _b = t26; */\
		"vbroadcastsd	0x018(%%rsi),%%ymm6 		\n\t"/* load __c2i2 */\
		"vfnmadd231pd	%%ymm4 ,%%ymm15,%%ymm11		\n\t	 vfmsub231pd	%%ymm7 ,%%ymm15,%%ymm9 		\n\t"/*	 FMA231(t18,__sc,_d );		 FMA231(t27,__sc,_b ); */\
		"vbroadcastsd	(%%rsi),%%ymm15				\n\t"/* load __c1_c */\
		"vmovaps		     (%%rax),%%ymm0 		\n\t	vmovaps			0x020(%%rax),%%ymm1 		\n\t"/*    t02;    t03; */\
		"vmovaps			 %%ymm10,%%ymm4 		\n\t	vmovaps				 %%ymm0 ,%%ymm2 		\n\t"/* t18 = _c;	t10 = t02; */\
		" vfmadd231pd	%%ymm8 ,%%ymm14,%%ymm4 		\n\t	 vfmadd231pd	%%ymm12,%%ymm6 ,%%ymm0 		\n\t"/*	 FMA231(_a ,__c31,t18);		 FMA231(_e ,__c2i2,t02); */\
		"vmovaps			 %%ymm11,%%ymm5 		\n\t	vmovaps				 %%ymm1 ,%%ymm3 		\n\t"/* t19 = _d;	t11 = t03; */\
		" vfmadd231pd	%%ymm9 ,%%ymm14,%%ymm5 		\n\t	 vfmadd231pd	%%ymm13,%%ymm6 ,%%ymm1 		\n\t"/*	 FMA231(_b ,__c31,t19);		 FMA231(_f ,__c2i2,t03); */\
		"vfnmadd231pd	%%ymm8 ,%%ymm14,%%ymm10		\n\t	vfnmadd231pd	%%ymm12,%%ymm6 ,%%ymm2 		\n\t"/*	FNMA231(_a ,__c31,_c );		FNMA231(_e ,__c2i2,t10); */\
		"vfnmadd231pd	%%ymm9 ,%%ymm14,%%ymm11		\n\t	vfnmadd231pd	%%ymm13,%%ymm6 ,%%ymm3 		\n\t"/*	FNMA231(_b ,__c31,_d );		FNMA231(_f ,__c2i2,t11); */\
		"vmovaps			 %%ymm0 ,%%ymm8 		\n\t	vmovaps				 %%ymm1 ,%%ymm9 		\n\t"/* _a = t02; _b = t03; */\
		" vfmadd231pd	%%ymm4 ,%%ymm15,%%ymm0 		\n\t	 vfmadd231pd	%%ymm5 ,%%ymm15,%%ymm1 		\n\t"/*	 FMA231(t18,__c1_c,t02);	 FMA231(t19,__c1_c,t03); */\
		"vfnmadd231pd	%%ymm4 ,%%ymm15,%%ymm8 		\n\t	vfnmadd231pd	%%ymm5 ,%%ymm15,%%ymm9 		\n\t"/*	FNMA231(t18,__c1_c,_a );	FNMA231(t19,__c1_c,_b ); */\
		"vmovaps			 %%ymm2 ,%%ymm12		\n\t	vmovaps				 %%ymm3 ,%%ymm13		\n\t"/* _e = t10; _f = t11; */\
		"vfnmadd231pd	%%ymm11,%%ymm15,%%ymm2 		\n\t	 vfmadd231pd	%%ymm10,%%ymm15,%%ymm3 		\n\t"/*	FNMA231(_d ,__c1_c,t10);	 FMA231(_c ,__c1_c,t11); */\
		" vfmadd231pd	%%ymm11,%%ymm15,%%ymm12		\n\t	vfnmadd231pd	%%ymm10,%%ymm15,%%ymm13		\n\t"/*	 FMA231(_d ,__c1_c,_e );	FNMA231(_c ,__c1_c,_f ); */\
		/* Write outputs: Swap 5/D outputs for DIT */\
		"subq	$%c[__o1],%%r10	\n\t"/* __out0 + 1*ostride */\
		"subq	$%c[__o1],%%r12	\n\t"/* __out0 + 5*ostride */\
		"subq	$%c[__o1],%%r11	\n\t"/* __out0 + 9*ostride */\
		"subq	$%c[__o1],%%r13	\n\t"/* __out0 + d*ostride */\
		"vmovaps		%%ymm0 ,     (%%r10)		\n\t	vmovaps			%%ymm1 ,0x020(%%r10)		\n\t"/* __B1r= t02;		__B1i= t03; */\
		"vmovaps		%%ymm8 ,     (%%r11)		\n\t	vmovaps			%%ymm9 ,0x020(%%r11)		\n\t"/* __B9r= _a ;		__B9i= _b ; */\
		"vmovaps		%%ymm2 ,     (%%r13)		\n\t	vmovaps			%%ymm3 ,0x020(%%r13)		\n\t"/* __BDr= t10;		__BDi= t11; */\
		"vmovaps		%%ymm12,     (%%r12)		\n\t	vmovaps			%%ymm13,0x020(%%r12)		\n\t"/* __B5r= _e ;		__B5i= _f ; */\
		"\n\t"\
		/*...Block 3: t6,14,22,30 */\
		"addq	$%c[__i1],%%rax	\n\t"/* __in0 + 3*istride */\
		"addq	$%c[__i1],%%rbx	\n\t"/* __in0 + 7*istride */\
		"addq	$%c[__i1],%%rcx	\n\t"/* __in0 + b*istride */\
		"addq	$%c[__i1],%%rdx	\n\t"/* __in0 + f*istride */\
		"vmovaps		     (%%rbx),%%ymm2 		\n\t	vmovaps			0x020(%%rbx),%%ymm3 		\n\t"/*    t14;    t15; */\
		"vbroadcastsd	0x008(%%rsi),%%ymm15		\n\t"/* load __sc  */\
		"vmovaps		     (%%rcx),%%ymm4 		\n\t	vmovaps			0x020(%%rcx),%%ymm5 		\n\t"/*    t22;    t23; */\
		"vmovaps		     (%%rdx),%%ymm6 		\n\t	vmovaps			0x020(%%rdx),%%ymm7 		\n\t"/*    t30;    t31; */\
		"vsubpd		%%ymm3 ,%%ymm2 ,%%ymm10 		\n\t"/* _c = t14-t15; */\
		"vaddpd		%%ymm3 ,%%ymm2 ,%%ymm11			\n\t"/* _d = t14+t15; */\
		"vbroadcastsd	0x0d0(%%rsi),%%ymm14		\n\t"/* load __c31 */\
		"vmovaps			 %%ymm5 ,%%ymm12 		\n\t	vmovaps				 %%ymm4 ,%%ymm13		\n\t"/* _e = t23; _f = t22;*/\
		" vfmadd231pd	%%ymm4 ,%%ymm15,%%ymm12		\n\t	 vfmsub231pd	%%ymm5 ,%%ymm15,%%ymm13		\n\t"/*	 FMS231(t22,__sc,_e );		 FMA231(t23,__sc,_f );*/\
		"vmovaps			 %%ymm6 ,%%ymm8  		\n\t	vmovaps				 %%ymm7 ,%%ymm9 		\n\t"/* _a = t30; _b = t31; */\
		" vfmadd231pd	%%ymm7 ,%%ymm15,%%ymm8 		\n\t	vfnmadd231pd	%%ymm6 ,%%ymm15,%%ymm9 		\n\t"/*	FNMA231(t31,__sc,_a );		 FMA231(t30,__sc,_b );*/\
		"vbroadcastsd	0x018(%%rsi),%%ymm6 		\n\t"/* load __c2i2 */\
		"vbroadcastsd	(%%rsi),%%ymm15				\n\t"/* load __c1_c */\
		"vmovaps		     (%%rax),%%ymm0 		\n\t	vmovaps			0x020(%%rax),%%ymm1 		\n\t"/*    t06;    t07; */\
		"vmovaps			 %%ymm1 ,%%ymm3 		\n\t	vmovaps				 %%ymm0 ,%%ymm2 		\n\t"/* t15= t07;	t14= t06; */\
		"vfnmadd231pd	%%ymm11,%%ymm6 ,%%ymm1 		\n\t	vfnmadd231pd	%%ymm10,%%ymm6 ,%%ymm0 		\n\t"/*	FNMA231(_d ,__c2i2,t07);	FNMA231(_c ,__c2i2,t06); */\
		"vmovaps			 %%ymm12,%%ymm4 		\n\t	vmovaps				 %%ymm13,%%ymm5 		\n\t"/* t22= _e; t23= _f; */\
		"vfnmadd231pd	%%ymm8 ,%%ymm14,%%ymm4 		\n\t	vfnmadd231pd	%%ymm9 ,%%ymm14,%%ymm5 		\n\t"/*	FNMA231(_a ,__c31 ,t22);	FNMA231(_b ,__c31 ,t23); */\
		" vfmadd231pd	%%ymm8 ,%%ymm14,%%ymm12		\n\t	 vfmadd231pd	%%ymm9 ,%%ymm14,%%ymm13		\n\t"/*	 FMA231(_a ,__c31 ,_e );	 FMA231(_b ,__c31 ,_f ); */\
		" vfmadd231pd	%%ymm10,%%ymm6 ,%%ymm2 		\n\t	 vfmadd231pd	%%ymm11,%%ymm6 ,%%ymm3 		\n\t"/*	 FMA231(_c ,__c2i2,t14);	 FMA231(_d ,__c2i2,t15); */\
		"vmovaps			 %%ymm0 ,%%ymm8 		\n\t	vmovaps				 %%ymm1 ,%%ymm9 		\n\t"/* _a = t06; _b = t07; */\
		" vfmadd231pd	%%ymm4 ,%%ymm15,%%ymm0 		\n\t	 vfmadd231pd	%%ymm5 ,%%ymm15,%%ymm1 		\n\t"/*	 FMA231(t22,__c1_c,t06);	 FMA231(t23,__c1_c,t07); */\
		"vfnmadd231pd	%%ymm4 ,%%ymm15,%%ymm8 		\n\t	vfnmadd231pd	%%ymm5 ,%%ymm15,%%ymm9 		\n\t"/*	FNMA231(t22,__c1_c,_a );	FNMA231(t23,__c1_c,_b ); */\
		"vmovaps			 %%ymm2 ,%%ymm10		\n\t	vmovaps				 %%ymm3 ,%%ymm11		\n\t"/* _c = t14; _d = t15; */\
		"vfnmadd231pd	%%ymm13,%%ymm15,%%ymm2 		\n\t	 vfmadd231pd	%%ymm12,%%ymm15,%%ymm3 		\n\t"/*	FNMA231(_f ,__c1_c,t14);	 FMA231(_e ,__c1_c,t15); */\
		" vfmadd231pd	%%ymm13,%%ymm15,%%ymm10		\n\t	vfnmadd231pd	%%ymm12,%%ymm15,%%ymm11		\n\t"/*	 FMA231(_f ,__c1_c,_c );	FNMA231(_e ,__c1_c,_d ); */\
		/* Write outputs: Swap 7/F outputs for DIT */\
		"addq	$%c[__o2],%%r10	\n\t"/* __out0 + 3*ostride */\
		"addq	$%c[__o2],%%r12	\n\t"/* __out0 + 7*ostride */\
		"addq	$%c[__o2],%%r11	\n\t"/* __out0 + b*ostride */\
		"addq	$%c[__o2],%%r13	\n\t"/* __out0 + f*ostride */\
		"vmovaps		%%ymm0 ,     (%%r10)		\n\t	vmovaps			%%ymm1 ,0x020(%%r10)		\n\t"/* __B3r= t06;		__B3i= t07; */\
		"vmovaps		%%ymm8 ,     (%%r11)		\n\t	vmovaps			%%ymm9 ,0x020(%%r11)		\n\t"/* __BBr= _a ;		__BBi= _b ; */\
		"vmovaps		%%ymm2 ,     (%%r13)		\n\t	vmovaps			%%ymm3 ,0x020(%%r13)		\n\t"/* __BFr= t14;		__BFi= t15; */\
		"vmovaps		%%ymm10,     (%%r12)		\n\t	vmovaps			%%ymm11,0x020(%%r12)		\n\t"/* __B7r= _c ;		__B7i= _d ; */\
		:					/* outputs: none */\
		: [__in0] "m" (Xin0)	/* All inputs from memory addresses here */\
		 ,[__i1] "e" (Xi1)\
		 ,[__i2] "e" (Xi2)\
		 ,[__i3] "e" (Xi3)\
		 ,[__i4] "e" (Xi4)\
		 ,[__out0] "m" (Xout0)\
		 ,[__o1] "e" (Xo1)\
		 ,[__o2] "e" (Xo2)\
		 ,[__o3] "e" (Xo3)\
		 ,[__o4] "e" (Xo4)\
		 ,[__cc0] "m" (Xcc0)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"	/* Clobbered registers */\
	);\
	}

	// Doubled-data version of PAIR_SQUARE_4_SSE2, taking advantage of 2-per-cycle thruput of AVX2 FMAs:
	#define PAIR_SQUARE_4_AVX2(XtAr, XtBr, XtCr, XtDr, Xc0, Xs0, XuAr, XuBr, XuCr, XuDr, Xc1, Xs1, Xforth)\
	{\
	__asm__ volatile (\
		"movq	%[__tDr]	,%%rdx							\n\t	movq	%[__uDr]	,%%r13		\n\t"\
		"movq	%[__tAr]	,%%rax							\n\t	movq	%[__uAr]	,%%r10		\n\t"\
			"movq	%[__tCr]	,%%rcx							\n\t	movq	%[__uCr]	,%%r12		\n\t"\
			"movq	%[__tBr]	,%%rbx							\n\t	movq	%[__uBr]	,%%r11		\n\t"\
		"vmovaps	    (%%rdx),%%ymm4						\n\t	vmovaps	    (%%r13),%%ymm12		\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5						\n\t	vmovaps	0x20(%%r13),%%ymm13		\n\t"\
		"vshufpd	$5,%%ymm4,%%ymm4,%%ymm4					\n\t	vshufpd	$5,%%ymm12,%%ymm12,%%ymm12		\n\t"\
		"vshufpd	$5,%%ymm5,%%ymm5,%%ymm5					\n\t	vshufpd	$5,%%ymm13,%%ymm13,%%ymm13		\n\t"\
		"vmovaps	    (%%rcx),%%ymm6						\n\t	vmovaps	    (%%r12),%%ymm14		\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7						\n\t	vmovaps	0x20(%%r12),%%ymm15		\n\t"\
		"vshufpd	$5,%%ymm6,%%ymm6,%%ymm6					\n\t	vshufpd	$5,%%ymm14,%%ymm14,%%ymm14		\n\t"\
		"vshufpd	$5,%%ymm7,%%ymm7,%%ymm7					\n\t	vshufpd	$5,%%ymm15,%%ymm15,%%ymm15		\n\t"\
		"vmulpd			    (%%rax),%%ymm4,%%ymm0			\n\t	vmulpd			    (%%r10),%%ymm12,%%ymm8 	\n\t"\
		"vmulpd			0x20(%%rax),%%ymm4,%%ymm1			\n\t	vmulpd			0x20(%%r10),%%ymm12,%%ymm9 	\n\t"\
		"vmulpd			    (%%rbx),%%ymm6,%%ymm2			\n\t	vmulpd			    (%%r11),%%ymm14,%%ymm10	\n\t"\
		"vmulpd			0x20(%%rbx),%%ymm6,%%ymm3			\n\t	vmulpd			0x20(%%r11),%%ymm14,%%ymm11	\n\t"\
		" vfmadd231pd	0x20(%%rax),%%ymm5,%%ymm0			\n\t	 vfmadd231pd	0x20(%%r10),%%ymm13,%%ymm8 	\n\t"\
		"vfnmadd231pd	    (%%rax),%%ymm5,%%ymm1			\n\t	vfnmadd231pd	    (%%r10),%%ymm13,%%ymm9 	\n\t"\
		" vfmadd231pd	0x20(%%rbx),%%ymm7,%%ymm2			\n\t	 vfmadd231pd	0x20(%%r11),%%ymm15,%%ymm10	\n\t"\
		"vfnmadd231pd	    (%%rbx),%%ymm7,%%ymm3			\n\t	vfnmadd231pd	    (%%r11),%%ymm15,%%ymm11	\n\t"\
		"\n\t"\
	"movq		%[__forth],%%rdi	\n\t"\
	"leaq	-0x20(%%rdi),%%rdi		\n\t"/* two */\
		"vmovaps	    (%%rax)	,%%ymm4			\n\t	vmovaps	    (%%rbx)	,%%ymm6					\n\t	vmovaps	    (%%r10)	,%%ymm12			\n\t	vmovaps	    (%%r11)	,%%ymm14			\n\t"\
		"vmovaps	0x20(%%rax)	,%%ymm5			\n\t	vmovaps	0x20(%%rbx)	,%%ymm7					\n\t	vmovaps	0x20(%%r10)	,%%ymm13			\n\t	vmovaps	0x20(%%r11)	,%%ymm15			\n\t"\
		"vmulpd		%%ymm4,%%ymm4,%%ymm4		\n\t	vmulpd			%%ymm6,%%ymm6,%%ymm6		\n\t	vmulpd		%%ymm12,%%ymm12,%%ymm12		\n\t	vmulpd			%%ymm14,%%ymm14,%%ymm14	\n\t"\
		/* x^2 - y^2: */\
		"vfnmadd231pd	%%ymm5,%%ymm5,%%ymm4	\n\t	vfnmadd231pd	%%ymm7,%%ymm7,%%ymm6		\n\t	vfnmadd231pd	%%ymm13,%%ymm13,%%ymm12	\n\t	vfnmadd231pd	%%ymm15,%%ymm15,%%ymm14	\n\t"\
		"vmovaps	    (%%rax)	,%%ymm5			\n\t	vmovaps	    (%%rbx)	,%%ymm7					\n\t	vmovaps	    (%%r10)	,%%ymm13			\n\t	vmovaps	    (%%r11)	,%%ymm15			\n\t"\
		"vmulpd		0x20(%%rax)	,%%ymm5,%%ymm5	\n\t	vmulpd		0x20(%%rbx)	,%%ymm7,%%ymm7		\n\t	vmulpd		0x20(%%r10)	,%%ymm13,%%ymm13	\n\t	vmulpd		0x20(%%r11)	,%%ymm15,%%ymm15	\n\t"\
		"vaddpd		%%ymm5		,%%ymm5,%%ymm5	\n\t	vaddpd		%%ymm7		,%%ymm7,%%ymm7		\n\t	vaddpd		%%ymm13		,%%ymm13,%%ymm13	\n\t	vaddpd		%%ymm15		,%%ymm15,%%ymm15	\n\t"\
		"vmovaps	%%ymm4	,    (%%rax)		\n\t	vmovaps	%%ymm6	,    (%%rbx)				\n\t	vmovaps	%%ymm12	,    (%%r10)		\n\t	vmovaps	%%ymm14	,    (%%r11)		\n\t"\
		"vmovaps	%%ymm5	,0x20(%%rax)		\n\t	vmovaps	%%ymm7	,0x20(%%rbx)				\n\t	vmovaps	%%ymm13	,0x20(%%r10)		\n\t	vmovaps	%%ymm15	,0x20(%%r11)		\n\t"\
		" vfmsub132pd	(%%rdi),%%ymm4,%%ymm0	\n\t	 vfmsub132pd	(%%rdi),%%ymm6,%%ymm2		\n\t	 vfmsub132pd	(%%rdi),%%ymm12,%%ymm8 	\n\t	 vfmsub132pd	(%%rdi),%%ymm14,%%ymm10	\n\t"\
		" vfmsub132pd	(%%rdi),%%ymm5,%%ymm1	\n\t	 vfmsub132pd	(%%rdi),%%ymm7,%%ymm3		\n\t	 vfmsub132pd	(%%rdi),%%ymm13,%%ymm9 	\n\t	 vfmsub132pd	(%%rdi),%%ymm15,%%ymm11	\n\t"\
		"										\n\t"\
		"vmovaps	    (%%rdx)	,%%ymm4			\n\t	vmovaps	    (%%rcx)	,%%ymm6					\n\t	vmovaps	    (%%r13)	,%%ymm12			\n\t	vmovaps	    (%%r12)	,%%ymm14			\n\t"\
		"vmovaps	0x20(%%rdx)	,%%ymm5			\n\t	vmovaps	0x20(%%rcx)	,%%ymm7					\n\t	vmovaps	0x20(%%r13)	,%%ymm13			\n\t	vmovaps	0x20(%%r12)	,%%ymm15			\n\t"\
		"vmulpd			%%ymm4,%%ymm4,%%ymm4	\n\t	vmulpd			%%ymm6,%%ymm6,%%ymm6		\n\t	vmulpd			%%ymm12,%%ymm12,%%ymm12	\n\t	vmulpd			%%ymm14,%%ymm14,%%ymm14	\n\t"\
		"vfnmadd231pd	%%ymm5,%%ymm5,%%ymm4	\n\t	vfnmadd231pd	%%ymm7,%%ymm7,%%ymm6		\n\t	vfnmadd231pd	%%ymm13,%%ymm13,%%ymm12	\n\t	vfnmadd231pd	%%ymm15,%%ymm15,%%ymm14	\n\t"\
		"vmovaps	    (%%rdx)	,%%ymm5			\n\t	vmovaps	    (%%rcx)	,%%ymm7					\n\t	vmovaps	    (%%r13)	,%%ymm13			\n\t	vmovaps	    (%%r12)	,%%ymm15			\n\t"\
		"vmulpd		0x20(%%rdx)	,%%ymm5,%%ymm5	\n\t	vmulpd		0x20(%%rcx)	,%%ymm7,%%ymm7		\n\t	vmulpd		0x20(%%r13)	,%%ymm13,%%ymm13	\n\t	vmulpd		0x20(%%r12)	,%%ymm15,%%ymm15	\n\t"\
		"vaddpd		%%ymm5		,%%ymm5,%%ymm5	\n\t	vaddpd		%%ymm7		,%%ymm7,%%ymm7		\n\t	vaddpd		%%ymm13		,%%ymm13,%%ymm13	\n\t	vaddpd		%%ymm15		,%%ymm15,%%ymm15	\n\t"\
		"vmovaps	%%ymm4	,    (%%rdx)		\n\t	vmovaps	%%ymm6	,    (%%rcx)				\n\t	vmovaps	%%ymm12	,    (%%r13)		\n\t	vmovaps	%%ymm14	,    (%%r12)		\n\t"\
		"vmovaps	%%ymm5	,0x20(%%rdx)		\n\t	vmovaps	%%ymm7	,0x20(%%rcx)				\n\t	vmovaps	%%ymm13	,0x20(%%r13)		\n\t	vmovaps	%%ymm15	,0x20(%%r12)		\n\t"\
		"vshufpd	$5,%%ymm4,%%ymm4,%%ymm4		\n\t	vshufpd	$5,%%ymm6,%%ymm6,%%ymm6				\n\t	vshufpd	$5,%%ymm12,%%ymm12,%%ymm12		\n\t	vshufpd	$5,%%ymm14,%%ymm14,%%ymm14		\n\t"\
		"vshufpd	$5,%%ymm5,%%ymm5,%%ymm5		\n\t	vshufpd	$5,%%ymm7,%%ymm7,%%ymm7				\n\t	vshufpd	$5,%%ymm13,%%ymm13,%%ymm13		\n\t	vshufpd	$5,%%ymm15,%%ymm15,%%ymm15		\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0			\n\t	vsubpd	%%ymm6,%%ymm2,%%ymm2				\n\t	vsubpd	%%ymm12,%%ymm8 ,%%ymm8 			\n\t	vsubpd	%%ymm14,%%ymm10,%%ymm10			\n\t"\
		"vaddpd	%%ymm5,%%ymm1,%%ymm1			\n\t	vaddpd	%%ymm7,%%ymm3,%%ymm3				\n\t	vaddpd	%%ymm13,%%ymm9 ,%%ymm9 			\n\t	vaddpd	%%ymm15,%%ymm11,%%ymm11			\n\t"\
		"\n\t"\
		"movq	%[__c0]		,%%rax							\n\t	movq	%[__c1]		,%%r10		\n\t"\
		"movq	%[__s0]		,%%rbx							\n\t	movq	%[__s1]		,%%r11		\n\t"\
	"leaq	0x20(%%rdi),%%rdi	\n\t"/* forth, from two */\
		"vmovaps	%%ymm0		,%%ymm4						\n\t	vmovaps	%%ymm8 		,%%ymm12		\n\t"\
		"vmovaps	%%ymm1		,%%ymm5						\n\t	vmovaps	%%ymm9 		,%%ymm13		\n\t"\
		"vmovaps	%%ymm2	,%%ymm6							\n\t	vmovaps	%%ymm10	,%%ymm14		\n\t"\
		"vmovaps	%%ymm3	,%%ymm7							\n\t	vmovaps	%%ymm11	,%%ymm15		\n\t"\
		" vfmadd132pd	(%%rax)	,%%ymm4,%%ymm0				\n\t	 vfmadd132pd	(%%r10)	,%%ymm12,%%ymm8 		\n\t"\
		" vfmadd132pd	(%%rax)	,%%ymm5,%%ymm1				\n\t	 vfmadd132pd	(%%r10)	,%%ymm13,%%ymm9 		\n\t"\
		" vfmsub132pd	(%%rbx)	,%%ymm6,	%%ymm2			\n\t	 vfmsub132pd	(%%r11)	,%%ymm14,	%%ymm10	\n\t"\
		" vfmsub132pd	(%%rbx)	,%%ymm7,	%%ymm3			\n\t	 vfmsub132pd	(%%r11)	,%%ymm15,	%%ymm11	\n\t"\
		"vfnmadd231pd	(%%rbx)	,%%ymm5,%%ymm0				\n\t	vfnmadd231pd	(%%r11)	,%%ymm13,%%ymm8 		\n\t"\
		" vfmadd231pd	(%%rbx)	,%%ymm4,%%ymm1				\n\t	 vfmadd231pd	(%%r11)	,%%ymm12,%%ymm9 		\n\t"\
		" vfmadd231pd	(%%rax)	,%%ymm7,	%%ymm2			\n\t	 vfmadd231pd	(%%r10)	,%%ymm15,	%%ymm10	\n\t"\
		"vfnmadd231pd	(%%rax)	,%%ymm6,	%%ymm3			\n\t	vfnmadd231pd	(%%r10)	,%%ymm14,	%%ymm11	\n\t"\
		"vmovaps	(%%rdi),%%ymm4	\n\t"/* 0.25 */\
		"vmulpd	%%ymm4,%%ymm0,%%ymm0						\n\t	vmulpd	%%ymm4,%%ymm8 ,%%ymm8 		\n\t"\
		"vmulpd	%%ymm4,%%ymm1,%%ymm1						\n\t	vmulpd	%%ymm4,%%ymm9 ,%%ymm9 		\n\t"\
		"vmulpd	%%ymm4,%%ymm2,%%ymm2						\n\t	vmulpd	%%ymm4,%%ymm10,%%ymm10		\n\t"\
		"vmulpd	%%ymm4,%%ymm3,%%ymm3						\n\t	vmulpd	%%ymm4,%%ymm11,%%ymm11		\n\t"\
		"\n\t"\
		"movq	%[__tAr]	,%%rax							\n\t	movq	%[__uAr]	,%%r10		\n\t"\
		"movq	%[__tBr]	,%%rbx							\n\t	movq	%[__uBr]	,%%r11		\n\t"\
		"\n\t"\
		"vmovaps	    (%%rax)	,%%ymm4						\n\t	vmovaps	    (%%r10)	,%%ymm12		\n\t"\
		"vmovaps	0x20(%%rax)	,%%ymm5						\n\t	vmovaps	0x20(%%r10)	,%%ymm13		\n\t"\
		"vmovaps	    (%%rbx)	,%%ymm6						\n\t	vmovaps	    (%%r11)	,%%ymm14		\n\t"\
		"vmovaps	0x20(%%rbx)	,%%ymm7						\n\t	vmovaps	0x20(%%r11)	,%%ymm15		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4						\n\t	vaddpd	%%ymm8 ,%%ymm12,%%ymm12		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5						\n\t	vaddpd	%%ymm9 ,%%ymm13,%%ymm13		\n\t"\
		"vsubpd	%%ymm2,%%ymm6,%%ymm6						\n\t	vsubpd	%%ymm10,%%ymm14,%%ymm14		\n\t"\
		"vsubpd	%%ymm3,%%ymm7,%%ymm7						\n\t	vsubpd	%%ymm11,%%ymm15,%%ymm15		\n\t"\
		"vmovaps	%%ymm4	,    (%%rax)					\n\t	vmovaps	%%ymm12	,    (%%r10)	\n\t"\
		"vmovaps	%%ymm5	,0x20(%%rax)					\n\t	vmovaps	%%ymm13	,0x20(%%r10)	\n\t"\
		"vmovaps	%%ymm6	,    (%%rbx)					\n\t	vmovaps	%%ymm14	,    (%%r11)	\n\t"\
		"vmovaps	%%ymm7	,0x20(%%rbx)					\n\t	vmovaps	%%ymm15	,0x20(%%r11)	\n\t"\
		"\n\t"\
		"movq	%[__tCr]	,%%rcx							\n\t	movq	%[__uCr]	,%%r12		\n\t"\
		"movq	%[__tDr]	,%%rdx							\n\t	movq	%[__uDr]	,%%r13		\n\t"\
		"\n\t"\
		"vshufpd	$5,%%ymm0,%%ymm0,%%ymm0					\n\t	vshufpd	$5,%%ymm8 ,%%ymm8 ,%%ymm8 		\n\t"\
		"vshufpd	$5,%%ymm1,%%ymm1,%%ymm1					\n\t	vshufpd	$5,%%ymm9 ,%%ymm9 ,%%ymm9 		\n\t"\
		"vshufpd	$5,%%ymm2,%%ymm2,%%ymm2					\n\t	vshufpd	$5,%%ymm10,%%ymm10,%%ymm10		\n\t"\
		"vshufpd	$5,%%ymm3,%%ymm3,%%ymm3					\n\t	vshufpd	$5,%%ymm11,%%ymm11,%%ymm11		\n\t"\
		"\n\t"\
		"vmovaps	    (%%rdx)	,%%ymm4						\n\t	vmovaps	    (%%r13)	,%%ymm12		\n\t"\
		"vmovaps	0x20(%%rdx)	,%%ymm5						\n\t	vmovaps	0x20(%%r13)	,%%ymm13		\n\t"\
		"vmovaps	    (%%rcx)	,%%ymm6						\n\t	vmovaps	    (%%r12)	,%%ymm14		\n\t"\
		"vmovaps	0x20(%%rcx)	,%%ymm7						\n\t	vmovaps	0x20(%%r12)	,%%ymm15		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4						\n\t	vaddpd	%%ymm8 ,%%ymm12,%%ymm12		\n\t"\
		"vsubpd	%%ymm1,%%ymm5,%%ymm5						\n\t	vsubpd	%%ymm9 ,%%ymm13,%%ymm13		\n\t"\
		"vsubpd	%%ymm2,%%ymm6,%%ymm6						\n\t	vsubpd	%%ymm10,%%ymm14,%%ymm14		\n\t"\
		"vaddpd	%%ymm3,%%ymm7,%%ymm7						\n\t	vaddpd	%%ymm11,%%ymm15,%%ymm15		\n\t"\
		"vmovaps	%%ymm4	,    (%%rdx)					\n\t	vmovaps	%%ymm12	,    (%%r13)	\n\t"\
		"vmovaps	%%ymm5	,0x20(%%rdx)					\n\t	vmovaps	%%ymm13	,0x20(%%r13)	\n\t"\
		"vmovaps	%%ymm6	,    (%%rcx)					\n\t	vmovaps	%%ymm14	,    (%%r12)	\n\t"\
		"vmovaps	%%ymm7	,0x20(%%rcx)					\n\t	vmovaps	%%ymm15	,0x20(%%r12)	\n\t"\
		:					/* outputs: none */\
		: [__tAr] "m" (XtAr)	/* All inputs from memory addresses here */\
		 ,[__tBr] "m" (XtBr)\
		 ,[__tCr] "m" (XtCr)\
		 ,[__tDr] "m" (XtDr)\
		 ,[__c0] "m" (Xc0)\
		 ,[__s0] "m" (Xs0)\
		 ,[__uAr] "m" (XuAr)\
		 ,[__uBr] "m" (XuBr)\
		 ,[__uCr] "m" (XuCr)\
		 ,[__uDr] "m" (XuDr)\
		 ,[__c1] "m" (Xc1)\
		 ,[__s1] "m" (Xs1)\
		 ,[__forth] "m" (Xforth)\
		: "cc","memory","rax","rbx","rcx","rdx","rdi","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"	/* Clobbered registers */\
	);\
	}

	// AVX version has shufpd immediate = 5 = 0101_2, which is the doubled analog of the SSE2 imm8 = 1 = 01_2:
	#define PAIR_SQUARE_4_SSE2(XtAr, XtBr, XtCr, XtDr, Xc, Xs, Xforth)\
	{\
	__asm__ volatile (\
		"movq	%[__tDr]	,%%rdx		\n\t"\
		"movq	%[__tAr]	,%%rax		\n\t"\
			"movq	%[__tCr]	,%%rcx		\n\t"\
			"movq	%[__tBr]	,%%rbx		\n\t"\
	/* Processing of data in regs 0145, 2367 independent -
	   overlap to mitigate latencies: */\
		"vmovaps	    (%%rdx),%%ymm4		\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5		\n\t"\
		"vshufpd	$5,%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vshufpd	$5,%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	    (%%rcx),%%ymm6		\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7		\n\t"\
		"vshufpd	$5,%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vshufpd	$5,%%ymm7,%%ymm7,%%ymm7		\n\t"\
	/* Overlap of the 2 seqs is why we don't store rax/rabx data in xmm-registers here - already used all available xmm-regs: */\
		"vmulpd			    (%%rax),%%ymm4,%%ymm0	\n\t"\
		"vmulpd			0x20(%%rax),%%ymm4,%%ymm1	\n\t"\
		"vmulpd			    (%%rbx),%%ymm6,%%ymm2	\n\t"\
		"vmulpd			0x20(%%rbx),%%ymm6,%%ymm3	\n\t"\
		" vfmadd231pd	0x20(%%rax),%%ymm5,%%ymm0	\n\t"\
		"vfnmadd231pd	    (%%rax),%%ymm5,%%ymm1	\n\t"\
		" vfmadd231pd	0x20(%%rbx),%%ymm7,%%ymm2	\n\t"\
		"vfnmadd231pd	    (%%rbx),%%ymm7,%%ymm3	\n\t"\
		"\n\t"\
	/* now calculate square terms and __store back in the same temporaries: */\
	"movq		%[__forth],%%rdi		\n\t"\
	"leaq	-0x20(%%rdi),%%rdi		\n\t"/* two */\
	/*	lcol: __tmp=(__tAr+__tAi)*(__tAr-__tAi); __tAi=__tAr*__tAi; __tAi=__tAi+__tAi; __tAr=__tmp;	*/\
	/*	rcol: __tmp=(__tBr+__tBi)*(__tBr-__tBi); __tBi=__tBr*__tBi; __tBi=__tBi+__tBi; __tBr=__tmp;	*/\
		"vmovaps	    (%%rax)	,%%ymm4			\n\t	vmovaps	    (%%rbx)	,%%ymm6			\n\t"\
		"vmovaps	0x20(%%rax)	,%%ymm5			\n\t	vmovaps	0x20(%%rbx)	,%%ymm7			\n\t"\
		"vmulpd		%%ymm4,%%ymm4,%%ymm4		\n\t	vmulpd			%%ymm6,%%ymm6,%%ymm6	\n\t"\
		/* x^2 - y^2: */\
		"vfnmadd231pd	%%ymm5,%%ymm5,%%ymm4	\n\t	vfnmadd231pd	%%ymm7,%%ymm7,%%ymm6	\n\t"\
		"vmovaps	    (%%rax)	,%%ymm5			\n\t	vmovaps	    (%%rbx)	,%%ymm7			\n\t"\
		"vmulpd		0x20(%%rax)	,%%ymm5,%%ymm5	\n\t	vmulpd		0x20(%%rbx)	,%%ymm7,%%ymm7	\n\t"\
		"vaddpd		%%ymm5		,%%ymm5,%%ymm5	\n\t	vaddpd		%%ymm7		,%%ymm7,%%ymm7	\n\t"\
		"vmovaps	%%ymm4	,    (%%rax)		\n\t	vmovaps	%%ymm6	,    (%%rbx)		\n\t"\
		"vmovaps	%%ymm5	,0x20(%%rax)		\n\t	vmovaps	%%ymm7	,0x20(%%rbx)		\n\t"\
		" vfmsub132pd	(%%rdi),%%ymm4,%%ymm0	\n\t	 vfmsub132pd	(%%rdi),%%ymm6,%%ymm2	\n\t"\
		" vfmsub132pd	(%%rdi),%%ymm5,%%ymm1	\n\t	 vfmsub132pd	(%%rdi),%%ymm7,%%ymm3	\n\t"\
		"\n\t"\
	/*	lcol: __tmp=(__tDr+__tDi)*(__tDr-__tDi); __tDi=__tDr*__tDi; __tDi=__tDi+__tDi; __tDr=__tmp;	*/\
	/*	rcol: __tmp=(__tCr+__tCi)*(__tCr-__tCi); __tCi=__tCr*__tCi; __tCi=__tCi+__tCi; __tCr=__tmp;	*/\
		"vmovaps	    (%%rdx)	,%%ymm4			\n\t	vmovaps	    (%%rcx)	,%%ymm6			\n\t"\
		"vmovaps	0x20(%%rdx)	,%%ymm5			\n\t	vmovaps	0x20(%%rcx)	,%%ymm7			\n\t"\
		"vmulpd			%%ymm4,%%ymm4,%%ymm4	\n\t	vmulpd			%%ymm6,%%ymm6,%%ymm6	\n\t"\
		"vfnmadd231pd	%%ymm5,%%ymm5,%%ymm4	\n\t	vfnmadd231pd	%%ymm7,%%ymm7,%%ymm6	\n\t"\
		"vmovaps	    (%%rdx)	,%%ymm5			\n\t	vmovaps	    (%%rcx)	,%%ymm7			\n\t"\
		"vmulpd		0x20(%%rdx)	,%%ymm5,%%ymm5	\n\t	vmulpd		0x20(%%rcx)	,%%ymm7,%%ymm7	\n\t"\
		"vaddpd		%%ymm5		,%%ymm5,%%ymm5	\n\t	vaddpd		%%ymm7		,%%ymm7,%%ymm7	\n\t"\
		"vmovaps	%%ymm4	,    (%%rdx)		\n\t	vmovaps	%%ymm6	,    (%%rcx)		\n\t"\
		"vmovaps	%%ymm5	,0x20(%%rdx)		\n\t	vmovaps	%%ymm7	,0x20(%%rcx)		\n\t"\
		"vshufpd	$5,%%ymm4,%%ymm4,%%ymm4		\n\t	vshufpd	$5,%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vshufpd	$5,%%ymm5,%%ymm5,%%ymm5		\n\t	vshufpd	$5,%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0			\n\t	vsubpd	%%ymm6,%%ymm2,%%ymm2			\n\t"\
		"vaddpd	%%ymm5,%%ymm1,%%ymm1			\n\t	vaddpd	%%ymm7,%%ymm3,%%ymm3			\n\t"\
		"\n\t"\
		"movq	%[__c]		,%%rax		\n\t"\
		"movq	%[__s]		,%%rbx		\n\t"\
	"leaq	0x20(%%rdi),%%rdi		\n\t"/* forth, from two */\
		"vmovaps	%%ymm0		,%%ymm4		\n\t"\
		"vmovaps	%%ymm1		,%%ymm5		\n\t"\
		"vmovaps	%%ymm2	,%%ymm6		\n\t"\
		"vmovaps	%%ymm3	,%%ymm7		\n\t"\
		" vfmadd132pd	(%%rax)	,%%ymm4,%%ymm0		\n\t"\
		" vfmadd132pd	(%%rax)	,%%ymm5,%%ymm1		\n\t"\
		" vfmsub132pd	(%%rbx)	,%%ymm6,	%%ymm2	\n\t"\
		" vfmsub132pd	(%%rbx)	,%%ymm7,	%%ymm3	\n\t"\
		"vfnmadd231pd	(%%rbx)	,%%ymm5,%%ymm0		\n\t"\
		" vfmadd231pd	(%%rbx)	,%%ymm4,%%ymm1		\n\t"\
		" vfmadd231pd	(%%rax)	,%%ymm7,	%%ymm2	\n\t"\
		"vfnmadd231pd	(%%rax)	,%%ymm6,	%%ymm3	\n\t"\
		"vmovaps	(%%rdi),%%ymm4		\n\t"/* 0.25 */\
		"vmulpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vmulpd	%%ymm4,%%ymm1,%%ymm1		\n\t"\
		"vmulpd	%%ymm4,%%ymm2,%%ymm2		\n\t"\
		"vmulpd	%%ymm4,%%ymm3,%%ymm3		\n\t"\
		"\n\t"\
		"movq	%[__tAr]	,%%rax		\n\t"\
		"movq	%[__tBr]	,%%rbx		\n\t"\
		"\n\t"\
		"vmovaps	    (%%rax)	,%%ymm4		\n\t"\
		"vmovaps	0x20(%%rax)	,%%ymm5		\n\t"\
		"vmovaps	    (%%rbx)	,%%ymm6		\n\t"\
		"vmovaps	0x20(%%rbx)	,%%ymm7		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vsubpd	%%ymm2,%%ymm6,%%ymm6		\n\t"\
		"vsubpd	%%ymm3,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	%%ymm4	,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5	,0x20(%%rax)	\n\t"\
		"vmovaps	%%ymm6	,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm7	,0x20(%%rbx)	\n\t"\
		"\n\t"\
		"movq	%[__tCr]	,%%rcx		\n\t"\
		"movq	%[__tDr]	,%%rdx		\n\t"\
		"\n\t"\
		"vshufpd	$5,%%ymm0,%%ymm0,%%ymm0		\n\t"\
		"vshufpd	$5,%%ymm1,%%ymm1,%%ymm1		\n\t"\
		"vshufpd	$5,%%ymm2,%%ymm2,%%ymm2		\n\t"\
		"vshufpd	$5,%%ymm3,%%ymm3,%%ymm3		\n\t"\
		"\n\t"\
		"vmovaps	    (%%rdx)	,%%ymm4		\n\t"\
		"vmovaps	0x20(%%rdx)	,%%ymm5		\n\t"\
		"vmovaps	    (%%rcx)	,%%ymm6		\n\t"\
		"vmovaps	0x20(%%rcx)	,%%ymm7		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t"\
		"vsubpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vsubpd	%%ymm2,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm3,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	%%ymm4	,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm5	,0x20(%%rdx)	\n\t"\
		"vmovaps	%%ymm6	,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm7	,0x20(%%rcx)	\n\t"\
		:					/* outputs: none */\
		: [__tAr] "m" (XtAr)	/* All inputs from memory addresses here */\
		 ,[__tBr] "m" (XtBr)\
		 ,[__tCr] "m" (XtCr)\
		 ,[__tDr] "m" (XtDr)\
		 ,[__c] "m" (Xc)\
		 ,[__s] "m" (Xs)\
		 ,[__forth] "m" (Xforth)\
		: "cc","memory","rax","rbx","rcx","rdx","rdi","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

	//...Radix-7 DFT: Inputs in memlocs __i0-6, outputs into __o0-6, possibly coincident with inputs.

   #ifdef ALL_FMA	// FMAs used for all arithmetic, including 'trivial' ones (one mult = 1.0) to replace ADD/SUB:

	// Aggressive-FMA: replace [6 ADD, 12 SUB, 6 MUL, 42 FMA, 58 memref] ==> [6 MUL, 60 FMA (42 nontrivial), 66 memref].
	// I.e. trade 18 ADD/SUB for same #FMA + 8 LOAD-of-vector-const-1.0, those where spill of a reg-datum in favor of 1.0 not justifiable.
	//
	#define SSE2_RADIX_07_DFT(Xi0,Xi1,Xi2,Xi3,Xi4,Xi5,Xi6, Xcc,Xtwo, Xo0,Xo1,Xo2,Xo3,Xo4,Xo5,Xo6)\
	{\
	__asm__ volatile (\
		"movq	%[__two],%%r8		\n\t	leaq	0x20(%%r8),%%r9		\n\t"/* two,one */\
		"movq	%[__i1],%%rax		\n\t"\
		"movq	%[__i2],%%rbx		\n\t"\
		"movq	%[__i3],%%rcx		\n\t"\
		"movq	%[__i4],%%rdx		\n\t"\
		"movq	%[__i5],%%rsi		\n\t"\
		"movq	%[__i6],%%rdi		\n\t"		/*** Rcol does Imaginary Parts: ***/\
		"vmovaps	(%%rax),%%ymm6		\n\t	vmovaps	0x20(%%rax),%%ymm14	\n\t"	/* x1 */\
		"vmovaps	(%%rdi),%%ymm1		\n\t	vmovaps	0x20(%%rdi),%%ymm9 	\n\t"	/* x6 */\
		"vmovaps	(%%rbx),%%ymm5		\n\t	vmovaps	0x20(%%rbx),%%ymm13	\n\t"	/* x2 */\
		"vmovaps	(%%rsi),%%ymm2		\n\t	vmovaps	0x20(%%rsi),%%ymm10	\n\t"	/* x5 */\
		"vmovaps	(%%rcx),%%ymm4		\n\t	vmovaps	0x20(%%rcx),%%ymm12	\n\t"	/* x3 */\
		"vmovaps	(%%rdx),%%ymm3		\n\t	vmovaps	0x20(%%rdx),%%ymm11	\n\t"	/* x4 */\
	"vmovaps	(%%r8),%%ymm0		\n\t	vmovaps	(%%r9),%%ymm7		\n\t"/* two,one */\
		"movq	%[__i0],%%rdi		\n\t"\
	"vfmsub132pd %%ymm7,%%ymm1,%%ymm6	\n\t vfmsub132pd %%ymm7,%%ymm9 ,%%ymm14	\n\t"	/* t6 = x1 - x6 */\
	"vfmsub132pd %%ymm7,%%ymm2,%%ymm5	\n\t vfmsub132pd %%ymm7,%%ymm10,%%ymm13	\n\t"	/* t5 = x2 - x5 */\
	"vfmsub132pd %%ymm7,%%ymm3,%%ymm4	\n\t vfmsub132pd %%ymm7,%%ymm11,%%ymm12	\n\t"	/* t4 = x3 - x4 */\
	"vfmadd132pd %%ymm0,%%ymm6,%%ymm1	\n\t vfmadd132pd %%ymm0,%%ymm14,%%ymm9 	\n\t"	/* t1 = x1 + x6 */\
	"vfmadd132pd %%ymm0,%%ymm5,%%ymm2	\n\t vfmadd132pd %%ymm0,%%ymm13,%%ymm10	\n\t"	/* t2 = x2 + x5 */\
	"vfmadd132pd %%ymm0,%%ymm4,%%ymm3	\n\t vfmadd132pd %%ymm0,%%ymm12,%%ymm11	\n\t"	/* t3 = x3 + x4 */\
		"vmovaps	(%%rdi),%%ymm0		\n\t	vmovaps	0x20(%%rdi),%%ymm8 	\n\t"	/* t0 = x0 */\
		"movq	%[__o1],%%rax		\n\t"\
		"movq	%[__o2],%%rbx		\n\t"\
		"movq	%[__o3],%%rcx		\n\t"\
	/* Spill  xi - xj combos to o-slots; these won't be needed until we get to the sine terms: */\
		"vmovaps	%%ymm6,    (%%rax)	\n\t	vmovaps	%%ymm14,0x20(%%rax)	\n\t"/* t6 */\
		"vmovaps	%%ymm5,    (%%rbx)	\n\t	vmovaps	%%ymm13,0x20(%%rbx)	\n\t"/* t5 */\
		"vmovaps	%%ymm4,    (%%rcx)	\n\t	vmovaps	%%ymm12,0x20(%%rcx)	\n\t"/* t4 */\
		"vmovaps	%%ymm0,%%ymm6		\n\t	vmovaps	%%ymm8 ,%%ymm14		\n\t"/* Br0 = t0 (only show real parts in comments) */\
		"vmovaps	%%ymm0,%%ymm5		\n\t	vmovaps	%%ymm8 ,%%ymm13		\n\t"/* rt  = t0 */\
		"vmovaps	%%ymm0,%%ymm4		\n\t	vmovaps	%%ymm8 ,%%ymm12		\n\t"/* re  = t0 */\
\
		"movq	%[__cc],%%rsi		\n\t"\
		"movq	%[__o0],%%rdi		\n\t"\
		"vmovaps	0x40(%%rsi),%%ymm7		\n\t	vmovaps	0x80(%%rsi),%%ymm15		\n\t"/* cc2,cc3 */\
	"vfmadd231pd (%%rsi),%%ymm1,%%ymm5	\n\t vfmadd231pd (%%rsi),%%ymm9 ,%%ymm13	\n\t"/* rt  = FMADD(cc1,tr1, rt ); */\
	"vfmadd231pd %%ymm7 ,%%ymm1,%%ymm4	\n\t vfmadd231pd %%ymm7 ,%%ymm9 ,%%ymm12	\n\t"/* re  = FMADD(cc2,tr1, re ); */\
	"vfmadd231pd %%ymm15,%%ymm1,%%ymm0	\n\t vfmadd231pd %%ymm15,%%ymm9 ,%%ymm8 	\n\t"/* tr0 = FMADD(cc3,tr1, tr0); */\
	"vfmadd132pd (%%r9),%%ymm1,%%ymm6	\n\t vfmadd132pd (%%r9),%%ymm9 ,%%ymm14			\n\t"/* Br0 += tr1; */\
\
	"vfmadd231pd %%ymm7 ,%%ymm2,%%ymm5	\n\t vfmadd231pd %%ymm7 ,%%ymm10,%%ymm13	\n\t"/* rt  = FMADD(cc2,tr2, rt ); */\
	"vfmadd231pd %%ymm15,%%ymm2,%%ymm4	\n\t vfmadd231pd %%ymm15,%%ymm10,%%ymm12	\n\t"/* re  = FMADD(cc3,tr2, re ); */\
	"vfmadd231pd (%%rsi),%%ymm2,%%ymm0	\n\t vfmadd231pd (%%rsi),%%ymm10,%%ymm8 	\n\t"/* tr0 = FMADD(cc1,tr2, tr0); */\
	"vfmadd132pd (%%r9),%%ymm2,%%ymm6	\n\t vfmadd132pd (%%r9),%%ymm10,%%ymm14			\n\t"/* Br0 += tr2; */\
\
	"vfmadd231pd %%ymm15,%%ymm3,%%ymm5	\n\t vfmadd231pd %%ymm15,%%ymm11,%%ymm13	\n\t"/* rt  = FMADD(cc3,tr3, rt ); */\
	"vfmadd231pd (%%rsi),%%ymm3,%%ymm4	\n\t vfmadd231pd (%%rsi),%%ymm11,%%ymm12	\n\t"/* re  = FMADD(cc1,tr3, re ); */\
	"vfmadd231pd %%ymm7 ,%%ymm3,%%ymm0	\n\t vfmadd231pd %%ymm7 ,%%ymm11,%%ymm8 	\n\t"/* tr0 = FMADD(cc2,tr3, tr0); */\
	"vfmadd132pd (%%r9),%%ymm3,%%ymm6	\n\t vfmadd132pd (%%r9),%%ymm11,%%ymm14			\n\t"/* Br0 += tr3; */\
		"vmovaps	%%ymm6,    (%%rdi)	\n\t	vmovaps	%%ymm14,0x20(%%rdi)	\n\t"/* B0 */\
\
		"addq	$0x20,%%rsi		\n\t"/* Incr trig ptr: cc0 -> ss0 */\
		"vmovaps	0x40(%%rsi),%%ymm7	\n\t	vmovaps	0x80(%%rsi),%%ymm15		\n\t"/* ss2,ss3 */\
		"vmovaps		(%%rax),%%ymm1	\n\t	vmovaps	0x20(%%rax),%%ymm9 		\n\t"/* Restore: tr1 = tr6 */\
		"vmovaps		%%ymm1 ,%%ymm2	\n\t	vmovaps		%%ymm9 ,%%ymm10		\n\t"/* tr2 = tr6 */\
		"vmovaps		%%ymm1 ,%%ymm3	\n\t	vmovaps		%%ymm9 ,%%ymm11		\n\t"/* tr3 = tr6 */\
		"vmulpd	(%%rsi),%%ymm1,%%ymm1	\n\t	vmulpd (%%rsi),%%ymm9 ,%%ymm9 	\n\t"/* tr1 = ss1*tr6; */\
		"vmulpd	%%ymm7 ,%%ymm2,%%ymm2	\n\t	vmulpd %%ymm7 ,%%ymm10,%%ymm10	\n\t"/* tr2 = ss2*tr6; */\
		"vmulpd	%%ymm15,%%ymm3,%%ymm3	\n\t	vmulpd %%ymm15,%%ymm11,%%ymm11	\n\t"/* tr3 = ss3*tr6; */\
\
		"vmovaps		(%%rbx),%%ymm6	\n\t	vmovaps	0x20(%%rbx),%%ymm14		\n\t"/* Restore t5 */\
	" vfmadd231pd %%ymm7 ,%%ymm6,%%ymm1	\n\t  vfmadd231pd %%ymm7 ,%%ymm14,%%ymm9 	\n\t"/* tr1 =  FMADD(ss2,tr5, tr1); */\
	"vfnmadd231pd %%ymm15,%%ymm6,%%ymm2	\n\t vfnmadd231pd %%ymm15,%%ymm14,%%ymm10	\n\t"/* tr2 = FNMADD(ss3,tr5, tr2); */\
	"vfnmadd231pd (%%rsi),%%ymm6,%%ymm3	\n\t vfnmadd231pd (%%rsi),%%ymm14,%%ymm11	\n\t"/* tr3 = FNMADD(ss1,tr5, tr3); */\
\
		"vmovaps		(%%rcx),%%ymm6	\n\t	vmovaps	0x20(%%rcx),%%ymm14		\n\t"/* Restore t4 */\
	" vfmadd231pd %%ymm15,%%ymm6,%%ymm1	\n\t  vfmadd231pd %%ymm15,%%ymm14,%%ymm9 	\n\t"/* tr1 =  FMADD(ss3,tr4, tr1); */\
	"vfnmadd231pd (%%rsi),%%ymm6,%%ymm2	\n\t vfnmadd231pd (%%rsi),%%ymm14,%%ymm10	\n\t"/* tr2 = FNMADD(ss1,tr4, tr2); */\
	" vfmadd231pd %%ymm7 ,%%ymm6,%%ymm3	\n\t  vfmadd231pd %%ymm7 ,%%ymm14,%%ymm11	\n\t"/* tr3 =  FMADD(ss2,tr4, tr3); */\
\
		"\n\t"\
		"movq	%[__o4],%%rdx		\n\t"\
		"movq	%[__o5],%%rsi		\n\t"\
		"movq	%[__o6],%%rdi		\n\t"\
	"vmovaps	(%%r8),%%ymm6		\n\t	vmovaps	(%%r9),%%ymm7		\n\t"/* two,one */\
	/* Output permutation causes signs to get flipped here: */\
	"vfmsub132pd %%ymm7,%%ymm9 ,%%ymm5		\n\t vfmsub132pd %%ymm7,%%ymm1 ,%%ymm13	\n\t"/* Br1 = rt  - ti1;	Bi6 = it  - tr1; */\
	"vfmsub132pd %%ymm7,%%ymm10,%%ymm4		\n\t vfmsub132pd %%ymm7,%%ymm2 ,%%ymm12	\n\t"/* Br2 = re  - ti2;	Bi5 = im  - tr2; */\
	"vfmsub132pd %%ymm7,%%ymm11,%%ymm0		\n\t vfmsub132pd %%ymm7,%%ymm3 ,%%ymm8 	\n\t"/* Br3 = tr0 - ti3;	Bi4 = ti0 - tr3; */\
	"vfmadd132pd %%ymm6,%%ymm5,%%ymm9 		\n\t vfmadd132pd %%ymm6,%%ymm13,%%ymm1 	\n\t"/* Br6 = rt  + ti1;	Bi1 = it  + tr1; */\
	"vfmadd132pd %%ymm6,%%ymm4,%%ymm10		\n\t vfmadd132pd %%ymm6,%%ymm12,%%ymm2 	\n\t"/* Br5 = re  + ti2;	Bi2 = im  + tr2; */\
	"vfmadd132pd %%ymm6,%%ymm0,%%ymm11		\n\t vfmadd132pd %%ymm6,%%ymm8 ,%%ymm3 	\n\t"/* Br4 = tr0 + ti3;	Bi3 = ti0 + tr3; */\
		"vmovaps	%%ymm5	,   (%%rax)		\n\t	vmovaps	%%ymm13,0x20(%%rdi)	\n\t"/* Br1,Bi6 */\
		"vmovaps	%%ymm4	,   (%%rbx)		\n\t	vmovaps	%%ymm12,0x20(%%rsi)	\n\t"/* Br2,Bi5 */\
		"vmovaps	%%ymm0	,   (%%rcx)		\n\t	vmovaps	%%ymm8 ,0x20(%%rdx)	\n\t"/* Br3,Bi4 */\
		"vmovaps	%%ymm9 	,   (%%rdi)		\n\t	vmovaps	%%ymm1 ,0x20(%%rax)	\n\t"/* Br6,Bi1 */\
		"vmovaps	%%ymm10	,   (%%rsi)		\n\t	vmovaps	%%ymm2 ,0x20(%%rbx)	\n\t"/* Br5,Bi2 */\
		"vmovaps	%%ymm11	,   (%%rdx)		\n\t	vmovaps	%%ymm3 ,0x20(%%rcx)	\n\t"/* Br4,Bi3 */\
		:					/* outputs: none */\
		: [__i0] "m" (Xi0)	/* All inputs from memory addresses here */\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__i3] "m" (Xi3)\
		 ,[__i4] "m" (Xi4)\
		 ,[__i5] "m" (Xi5)\
		 ,[__i6] "m" (Xi6)\
		 ,[__cc] "m" (Xcc)\
		 ,[__two] "m" (Xtwo)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__o3] "m" (Xo3)\
		 ,[__o4] "m" (Xo4)\
		 ,[__o5] "m" (Xo5)\
		 ,[__o6] "m" (Xo6)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","rdi","r8","r9","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

   #else	// ALL_FMA = False: FMAs used only for nontrivial MUL/ADD combos:

	// AVX -> FMA version: replace [54 ADD, 34 SUB, 16 MUL, 54 memref] ==> [6 ADD, 12 SUB, 6 MUL, 42 FMA, 58 memref].
	// I.e. trade [88 ADD, 10 MUL] for 42 FMA. FMA version also better at preserving floating-point accuracy.
	//
	#define SSE2_RADIX_07_DFT(Xi0,Xi1,Xi2,Xi3,Xi4,Xi5,Xi6, Xcc,Xtwo, Xo0,Xo1,Xo2,Xo3,Xo4,Xo5,Xo6)\
	{\
	__asm__ volatile (\
		"movq	%[__two],%%r8		\n\t"\
		"movq	%[__i1],%%rax		\n\t"\
		"movq	%[__i2],%%rbx		\n\t"\
		"movq	%[__i3],%%rcx		\n\t"\
		"movq	%[__i4],%%rdx		\n\t"\
		"movq	%[__i5],%%rsi		\n\t"\
		"movq	%[__i6],%%rdi		\n\t"		/*** Rcol does Imaginary Parts: ***/\
		"vmovaps	(%%rax),%%ymm6		\n\t	vmovaps	0x20(%%rax),%%ymm14	\n\t"	/* x1 */\
		"vmovaps	(%%rdi),%%ymm1		\n\t	vmovaps	0x20(%%rdi),%%ymm9 	\n\t"	/* x6 */\
		"vmovaps	(%%rbx),%%ymm5		\n\t	vmovaps	0x20(%%rbx),%%ymm13	\n\t"	/* x2 */\
		"vmovaps	(%%rsi),%%ymm2		\n\t	vmovaps	0x20(%%rsi),%%ymm10	\n\t"	/* x5 */\
		"vmovaps	(%%rcx),%%ymm4		\n\t	vmovaps	0x20(%%rcx),%%ymm12	\n\t"	/* x3 */\
		"vmovaps	(%%rdx),%%ymm3		\n\t	vmovaps	0x20(%%rdx),%%ymm11	\n\t"	/* x4 */\
		"vmovaps	(%%r8),%%ymm0		\n\t"/* two */\
		"movq	%[__i0],%%rdi		\n\t"\
		"vsubpd	%%ymm1,%%ymm6,%%ymm6	\n\t	vsubpd	%%ymm9 ,%%ymm14,%%ymm14		\n\t"	/* t6 = x1 - x6 */\
		"vsubpd	%%ymm2,%%ymm5,%%ymm5	\n\t	vsubpd	%%ymm10,%%ymm13,%%ymm13		\n\t"	/* t5 = x2 - x5 */\
		"vsubpd	%%ymm3,%%ymm4,%%ymm4	\n\t	vsubpd	%%ymm11,%%ymm12,%%ymm12		\n\t"	/* t4 = x3 - x4 */\
	"vfmadd132pd %%ymm0,%%ymm6,%%ymm1	\n\t vfmadd132pd %%ymm0,%%ymm14,%%ymm9 		\n\t"	/* t1 = x1 + x6 */\
	"vfmadd132pd %%ymm0,%%ymm5,%%ymm2	\n\t vfmadd132pd %%ymm0,%%ymm13,%%ymm10		\n\t"	/* t2 = x2 + x5 */\
	"vfmadd132pd %%ymm0,%%ymm4,%%ymm3	\n\t vfmadd132pd %%ymm0,%%ymm12,%%ymm11		\n\t"	/* t3 = x3 + x4 */\
		"vmovaps	(%%rdi),%%ymm0		\n\t	vmovaps	0x20(%%rdi),%%ymm8 	\n\t"	/* t0 = x0 */\
		"movq	%[__o1],%%rax		\n\t"\
		"movq	%[__o2],%%rbx		\n\t"\
		"movq	%[__o3],%%rcx		\n\t"\
	/* Spill  xi - xj combos to o-slots; these won't be needed until we get to the sine terms: */\
		"vmovaps	%%ymm6,    (%%rax)	\n\t	vmovaps	%%ymm14,0x20(%%rax)	\n\t"/* t6 */\
		"vmovaps	%%ymm5,    (%%rbx)	\n\t	vmovaps	%%ymm13,0x20(%%rbx)	\n\t"/* t5 */\
		"vmovaps	%%ymm4,    (%%rcx)	\n\t	vmovaps	%%ymm12,0x20(%%rcx)	\n\t"/* t4 */\
		"vmovaps	%%ymm0,%%ymm6		\n\t	vmovaps	%%ymm8 ,%%ymm14		\n\t"/* Br0 = t0 (only show real parts in comments) */\
		"vmovaps	%%ymm0,%%ymm5		\n\t	vmovaps	%%ymm8 ,%%ymm13		\n\t"/* rt  = t0 */\
		"vmovaps	%%ymm0,%%ymm4		\n\t	vmovaps	%%ymm8 ,%%ymm12		\n\t"/* re  = t0 */\
\
		"movq	%[__cc],%%rsi		\n\t"\
		"movq	%[__o0],%%rdi		\n\t"\
		"vmovaps	0x40(%%rsi),%%ymm7		\n\t	vmovaps	0x80(%%rsi),%%ymm15		\n\t"/* cc2,cc3 */\
	"vfmadd231pd     (%%rsi),%%ymm1,%%ymm5	\n\t vfmadd231pd     (%%rsi),%%ymm9 ,%%ymm13	\n\t"/* rt  = FMADD(cc1,tr1, rt ); */\
	"vfmadd231pd %%ymm7 ,%%ymm1,%%ymm4	\n\t vfmadd231pd %%ymm7 ,%%ymm9 ,%%ymm12	\n\t"/* re  = FMADD(cc2,tr1, re ); */\
	"vfmadd231pd %%ymm15,%%ymm1,%%ymm0	\n\t vfmadd231pd %%ymm15,%%ymm9 ,%%ymm8 	\n\t"/* tr0 = FMADD(cc3,tr1, tr0); */\
		"vaddpd	%%ymm1,%%ymm6,%%ymm6		\n\t	vaddpd	%%ymm9 ,%%ymm14,%%ymm14			\n\t"/* Br0 += tr1; */\
\
	"vfmadd231pd %%ymm7 ,%%ymm2,%%ymm5	\n\t vfmadd231pd %%ymm7 ,%%ymm10,%%ymm13	\n\t"/* rt  = FMADD(cc2,tr2, rt ); */\
	"vfmadd231pd %%ymm15,%%ymm2,%%ymm4	\n\t vfmadd231pd %%ymm15,%%ymm10,%%ymm12	\n\t"/* re  = FMADD(cc3,tr2, re ); */\
	"vfmadd231pd     (%%rsi),%%ymm2,%%ymm0	\n\t vfmadd231pd     (%%rsi),%%ymm10,%%ymm8 	\n\t"/* tr0 = FMADD(cc1,tr2, tr0); */\
		"vaddpd	%%ymm2,%%ymm6,%%ymm6		\n\t	vaddpd	%%ymm10,%%ymm14,%%ymm14			\n\t"/* Br0 += tr2; */\
\
	"vfmadd231pd %%ymm15,%%ymm3,%%ymm5	\n\t vfmadd231pd %%ymm15,%%ymm11,%%ymm13	\n\t"/* rt  = FMADD(cc3,tr3, rt ); */\
	"vfmadd231pd     (%%rsi),%%ymm3,%%ymm4	\n\t vfmadd231pd     (%%rsi),%%ymm11,%%ymm12	\n\t"/* re  = FMADD(cc1,tr3, re ); */\
	"vfmadd231pd %%ymm7 ,%%ymm3,%%ymm0	\n\t vfmadd231pd %%ymm7 ,%%ymm11,%%ymm8 	\n\t"/* tr0 = FMADD(cc2,tr3, tr0); */\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6		\n\t	vaddpd	%%ymm11,%%ymm14,%%ymm14			\n\t"/* Br0 += tr3; */\
		"vmovaps	%%ymm6,    (%%rdi)		\n\t	vmovaps	%%ymm14,0x20(%%rdi)	\n\t"/* B0 */\
\
		"addq	$0x20,%%rsi		\n\t"/* Incr trig ptr: cc0 -> ss0 */\
		"vmovaps	0x40(%%rsi),%%ymm7		\n\t	vmovaps	0x80(%%rsi),%%ymm15		\n\t"/* ss2,ss3 */\
		"vmovaps		(%%rax),%%ymm1		\n\t	vmovaps	0x20(%%rax),%%ymm9 		\n\t"/* Restore: tr1 = tr6 */\
		"vmovaps		%%ymm1 ,%%ymm2		\n\t	vmovaps		%%ymm9 ,%%ymm10		\n\t"/* tr2 = tr6 */\
		"vmovaps		%%ymm1 ,%%ymm3		\n\t	vmovaps		%%ymm9 ,%%ymm11		\n\t"/* tr3 = tr6 */\
		"vmulpd     (%%rsi),%%ymm1,%%ymm1	\n\t	vmulpd     (%%rsi),%%ymm9 ,%%ymm9 	\n\t"/* tr1 = ss1*tr6; */\
		"vmulpd %%ymm7 ,%%ymm2,%%ymm2	\n\t	vmulpd %%ymm7 ,%%ymm10,%%ymm10	\n\t"/* tr2 = ss2*tr6; */\
		"vmulpd %%ymm15,%%ymm3,%%ymm3	\n\t	vmulpd %%ymm15,%%ymm11,%%ymm11	\n\t"/* tr3 = ss3*tr6; */\
\
		"vmovaps		(%%rbx),%%ymm6		\n\t	vmovaps	0x20(%%rbx),%%ymm14		\n\t"/* Restore t5 */\
	" vfmadd231pd %%ymm7 ,%%ymm6,%%ymm1	\n\t  vfmadd231pd %%ymm7 ,%%ymm14,%%ymm9 	\n\t"/* tr1 =  FMADD(ss2,tr5, tr1); */\
	"vfnmadd231pd %%ymm15,%%ymm6,%%ymm2	\n\t vfnmadd231pd %%ymm15,%%ymm14,%%ymm10	\n\t"/* tr2 = FNMADD(ss3,tr5, tr2); */\
	"vfnmadd231pd     (%%rsi),%%ymm6,%%ymm3	\n\t vfnmadd231pd     (%%rsi),%%ymm14,%%ymm11	\n\t"/* tr3 = FNMADD(ss1,tr5, tr3); */\
\
		"vmovaps		(%%rcx),%%ymm6		\n\t	vmovaps	0x20(%%rcx),%%ymm14		\n\t"/* Restore t4 */\
	" vfmadd231pd %%ymm15,%%ymm6,%%ymm1	\n\t  vfmadd231pd %%ymm15,%%ymm14,%%ymm9 	\n\t"/* tr1 =  FMADD(ss3,tr4, tr1); */\
	"vfnmadd231pd     (%%rsi),%%ymm6,%%ymm2	\n\t vfnmadd231pd     (%%rsi),%%ymm14,%%ymm10	\n\t"/* tr2 = FNMADD(ss1,tr4, tr2); */\
	" vfmadd231pd %%ymm7 ,%%ymm6,%%ymm3	\n\t  vfmadd231pd %%ymm7 ,%%ymm14,%%ymm11	\n\t"/* tr3 =  FMADD(ss2,tr4, tr3); */\
\
		"\n\t"\
		"movq	%[__o4],%%rdx		\n\t"\
		"movq	%[__o5],%%rsi		\n\t"\
		"movq	%[__o6],%%rdi		\n\t"\
		"vmovaps	(%%r8),%%ymm6	\n\t"/* two */\
	/* Output permutation causes signs to get flipped here: */\
		"vsubpd	%%ymm9 ,%%ymm5,%%ymm5		\n\t	vsubpd	%%ymm1 ,%%ymm13,%%ymm13	\n\t"/* Br1 = rt  - ti1;	Bi6 = it  - tr1; */\
		"vsubpd	%%ymm10,%%ymm4,%%ymm4		\n\t	vsubpd	%%ymm2 ,%%ymm12,%%ymm12	\n\t"/* Br2 = re  - ti2;	Bi5 = im  - tr2; */\
		"vsubpd	%%ymm11,%%ymm0,%%ymm0		\n\t	vsubpd	%%ymm3 ,%%ymm8 ,%%ymm8 	\n\t"/* Br3 = tr0 - ti3;	Bi4 = ti0 - tr3; */\
	"vfmadd132pd %%ymm6,%%ymm5,%%ymm9 		\n\t vfmadd132pd %%ymm6,%%ymm13,%%ymm1 	\n\t"/* Br6 = rt  + ti1;	Bi1 = it  + tr1; */\
	"vfmadd132pd %%ymm6,%%ymm4,%%ymm10		\n\t vfmadd132pd %%ymm6,%%ymm12,%%ymm2 	\n\t"/* Br5 = re  + ti2;	Bi2 = im  + tr2; */\
	"vfmadd132pd %%ymm6,%%ymm0,%%ymm11		\n\t vfmadd132pd %%ymm6,%%ymm8 ,%%ymm3 	\n\t"/* Br4 = tr0 + ti3;	Bi3 = ti0 + tr3; */\
		"vmovaps	%%ymm5	,   (%%rax)		\n\t	vmovaps	%%ymm13,0x20(%%rdi)	\n\t"/* Br1,Bi6 */\
		"vmovaps	%%ymm4	,   (%%rbx)		\n\t	vmovaps	%%ymm12,0x20(%%rsi)	\n\t"/* Br2,Bi5 */\
		"vmovaps	%%ymm0	,   (%%rcx)		\n\t	vmovaps	%%ymm8 ,0x20(%%rdx)	\n\t"/* Br3,Bi4 */\
		"vmovaps	%%ymm9 	,   (%%rdi)		\n\t	vmovaps	%%ymm1 ,0x20(%%rax)	\n\t"/* Br6,Bi1 */\
		"vmovaps	%%ymm10	,   (%%rsi)		\n\t	vmovaps	%%ymm2 ,0x20(%%rbx)	\n\t"/* Br5,Bi2 */\
		"vmovaps	%%ymm11	,   (%%rdx)		\n\t	vmovaps	%%ymm3 ,0x20(%%rcx)	\n\t"/* Br4,Bi3 */\
		:					/* outputs: none */\
		: [__i0] "m" (Xi0)	/* All inputs from memory addresses here */\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__i3] "m" (Xi3)\
		 ,[__i4] "m" (Xi4)\
		 ,[__i5] "m" (Xi5)\
		 ,[__i6] "m" (Xi6)\
		 ,[__cc] "m" (Xcc)\
		 ,[__two] "m" (Xtwo)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__o3] "m" (Xo3)\
		 ,[__o4] "m" (Xo4)\
		 ,[__o5] "m" (Xo5)\
		 ,[__o6] "m" (Xo6)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","rdi","r8","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

   #endif	// ALL_FMA ?

	/* Twiddleless version of SSE2_RADIX8_DIF_TWIDDLE. Inputs enter in memory locations __r0 + [__i1,__i2,__i3,__i4,__i5,__i6,__i7],;
	where r0 is a memory address and the i's are LITERAL [BYTE] OFFSETS. Outputs go into memory locations __o0,__o1,__o2,__o3,__o4,__o5,__o6,__o7, assumed disjoint with inputs:\
	*/
	#define SSE2_RADIX8_DIF_0TWIDDLE(Xr0, Xi1,Xi2,Xi3,Xi4,Xi5,Xi6,Xi7, Xo0,Xo1,Xo2,Xo3,Xo4,Xo5,Xo6,Xo7, Xisrt2,Xtwo)\
	{\
	__asm__ volatile (\
	/* 1st of 2 radix-4 subtransforms, data in ymm0-7: */	/* 2nd of 2 radix-4 subtransforms, data in ymm8-15: */\
		"movq	%[__isrt2],%%rsi				\n\t		movq	%[__two],%%r9	\n\t"/* r9 holds 2.0 throughout */\
		"movq	%[__r0],%%rax	/* i0 = r00 */	\n\t		leaq	%c[__i1](%%rax),%%r10	/* i1 */\n\t"\
		"leaq	%c[__i2](%%rax),%%rbx			\n\t		leaq	%c[__i3](%%rax),%%r11	/* i3 */\n\t"\
		"leaq	%c[__i4](%%rax),%%rcx			\n\t		leaq	%c[__i5](%%rax),%%r12	/* i5 */\n\t"\
		"leaq	%c[__i6](%%rax),%%rdx			\n\t		leaq	%c[__i7](%%rax),%%r13	/* i7 */\n\t"\
	/* p0,4 combo: x+-y into ymm0/1, 2/3, resp: */		/* p1,5 combo: x+y into ymm8 /1, x-y in ymm10/3: */\
	/* p2,6 combo: x+-y into ymm4/5, 6/7, resp: */		/* p3,7 combo: x+y into ymm14/7, x-y in ymm12/5: */\
		"vmovaps	     (%%rcx),%%ymm0			\n\t		vmovaps	     (%%r12),%%ymm8 			\n\t"\
		"vmovaps	0x020(%%rcx),%%ymm1			\n\t		vmovaps	0x020(%%r12),%%ymm9 			\n\t"\
		"vmovaps	     (%%rax),%%ymm2			\n\t		vmovaps	     (%%r10),%%ymm10			\n\t"\
		"vmovaps	0x020(%%rax),%%ymm3			\n\t		vmovaps	0x020(%%r10),%%ymm11			\n\t"\
		"vmovaps	     (%%rdx),%%ymm4			\n\t		vmovaps	     (%%r11),%%ymm12			\n\t"\
		"vmovaps	0x020(%%rdx),%%ymm5			\n\t		vmovaps	0x020(%%r11),%%ymm13			\n\t"\
		"vmovaps	     (%%rbx),%%ymm6			\n\t		vmovaps	     (%%r13),%%ymm14			\n\t"\
		"vmovaps	0x020(%%rbx),%%ymm7			\n\t		vmovaps	0x020(%%r13),%%ymm15			\n\t"\
		"vsubpd	%%ymm0,%%ymm2,%%ymm2			\n\t		vsubpd	%%ymm8 ,%%ymm10,%%ymm10			\n\t"\
		"vsubpd	%%ymm1,%%ymm3,%%ymm3			\n\t		vsubpd	%%ymm9 ,%%ymm11,%%ymm11			\n\t"\
		"vsubpd	%%ymm4,%%ymm6,%%ymm6			\n\t		vsubpd	%%ymm14,%%ymm12,%%ymm12			\n\t"\
		"vsubpd	%%ymm5,%%ymm7,%%ymm7			\n\t		vsubpd	%%ymm15,%%ymm13,%%ymm13			\n\t"\
	"vmovaps	%%ymm15,(%%rax) 	\n\t"/* spill ymm15 to make room for 2.0 */"	vmovaps	 (%%r9),%%ymm15	\n\t"/* two */\
	"vfmadd132pd	%%ymm15,%%ymm2,%%ymm0		\n\t	vfmadd132pd	%%ymm15,%%ymm10,%%ymm8 	\n\t"\
	"vfmadd132pd	%%ymm15,%%ymm3,%%ymm1		\n\t	vfmadd132pd	%%ymm15,%%ymm11,%%ymm9 	\n\t"\
	"vfmadd132pd	%%ymm15,%%ymm6,%%ymm4		\n\t	vfmadd132pd	%%ymm15,%%ymm12,%%ymm14	\n\t"\
	"vfmadd132pd	%%ymm15,%%ymm7,%%ymm5		\n\t	vfmadd132pd	(%%rax),%%ymm13,%%ymm15	\n\t"\
		/* Finish radix-4 butterfly and store results into temporary-array slots: */\
		"vsubpd		%%ymm4,%%ymm0,%%ymm0		\n\t		vsubpd		%%ymm14,%%ymm8 ,%%ymm8 		\n\t"\
		"vsubpd		%%ymm5,%%ymm1,%%ymm1		\n\t		vsubpd		%%ymm15,%%ymm9 ,%%ymm9 		\n\t"\
		"vsubpd		%%ymm7,%%ymm2,%%ymm2		\n\t		vsubpd		%%ymm13,%%ymm10,%%ymm10		\n\t"\
		"vsubpd		%%ymm6,%%ymm3,%%ymm3		\n\t		vsubpd		%%ymm12,%%ymm11,%%ymm11		\n\t"\
	"vmovaps	%%ymm12,(%%rax) 	\n\t"/* spill ymm12 to make room for 2.0 */"	vmovaps	 (%%r9),%%ymm12	\n\t"/* two */\
	"vfmadd132pd	%%ymm12,%%ymm0,%%ymm4		\n\t	vfmadd132pd		%%ymm12,%%ymm8 ,%%ymm14		\n\t"\
	"vfmadd132pd	%%ymm12,%%ymm1,%%ymm5		\n\t	vfmadd132pd		%%ymm12,%%ymm9 ,%%ymm15		\n\t"\
	"vfmadd132pd	%%ymm12,%%ymm2,%%ymm7		\n\t	vfmadd132pd		%%ymm12,%%ymm10,%%ymm13		\n\t"\
	"vfmadd132pd	%%ymm12,%%ymm3,%%ymm6		\n\t	vfmadd132pd		(%%rax),%%ymm11,%%ymm12		\n\t"\
		"													vsubpd		%%ymm12,%%ymm10,%%ymm10		\n\t"\
		"													vsubpd		%%ymm11,%%ymm13,%%ymm13		\n\t"\
		"												vfmadd132pd		(%%r9 ),%%ymm10,%%ymm12		\n\t"/* .two */\
		"												vfmadd132pd		(%%r9 ),%%ymm13,%%ymm11		\n\t"\
		/* SSE2_RADIX8_DIF_COMBINE_RAD4_SUBS(r00,r10,r20,r30,r08,r18,r28,r38): */\
		"\n\t"\
		"movq	%[__o0],%%rax					\n\t		movq	%[__o4],%%r10				\n\t"\
		"movq	%[__o1],%%rbx					\n\t		movq	%[__o5],%%r11				\n\t"\
		"movq	%[__o2],%%rcx					\n\t		movq	%[__o6],%%r12				\n\t"\
		"movq	%[__o3],%%rdx					\n\t		movq	%[__o7],%%r13				\n\t"\
		/* Combine r00,r08,r20,r28: */						/* Combine r10,r18,r30,r38: */\
		"vsubpd		%%ymm14,%%ymm4 ,%%ymm4 		\n\t	vfnmadd231pd	(%%rsi),%%ymm10,%%ymm2 		\n\t"/* .isrt2 */\
		"vsubpd		%%ymm9 ,%%ymm0 ,%%ymm0 		\n\t	vfnmadd231pd	(%%rsi),%%ymm13,%%ymm3 		\n\t"\
		"vsubpd		%%ymm15,%%ymm5 ,%%ymm5 		\n\t	vfnmadd231pd	(%%rsi),%%ymm12,%%ymm6 		\n\t"\
		"vsubpd		%%ymm8 ,%%ymm1 ,%%ymm1 		\n\t	vfnmadd231pd	(%%rsi),%%ymm11,%%ymm7 		\n\t"\
	"vmovaps	%%ymm8 ,(%%rax) 	\n\t"/* spill ymm8  to make room for 2.0 */"	vmovaps	 (%%r9),%%ymm8 	\n\t"/* two */\
	"vmovaps	%%ymm11,(%%r10) 	\n\t"/* spill ymm11 to make room for sqrt2 */"	vmovaps	0x40(%%r9),%%ymm11 \n\t"/* sqrt2 */\
		"vmovaps	%%ymm4 ,    (%%rbx)			\n\t		vmovaps	%%ymm2 ,    (%%r11)			\n\t"\
		"vmovaps	%%ymm0 ,    (%%rcx)			\n\t		vmovaps	%%ymm3 ,0x20(%%r13)			\n\t"\
		"vmovaps	%%ymm5 ,0x20(%%rbx)			\n\t		vmovaps	%%ymm6 ,0x20(%%r11)			\n\t"\
		"vmovaps	%%ymm1 ,0x20(%%rdx)			\n\t		vmovaps	%%ymm7 ,    (%%r12)			\n\t"\
	"vfmadd132pd	%%ymm8 ,%%ymm4 ,%%ymm14		\n\t	vfmadd132pd		%%ymm11,%%ymm2 ,%%ymm10		\n\t"\
	"vfmadd132pd	%%ymm8 ,%%ymm0 ,%%ymm9 		\n\t	vfmadd132pd		%%ymm11,%%ymm3 ,%%ymm13		\n\t"\
	"vfmadd132pd	%%ymm8 ,%%ymm5 ,%%ymm15		\n\t	vfmadd132pd		%%ymm11,%%ymm6 ,%%ymm12		\n\t"\
	"vfmadd132pd	(%%rax),%%ymm1 ,%%ymm8 		\n\t	vfmadd132pd		(%%r10),%%ymm7 ,%%ymm11		\n\t"\
		"vmovaps	%%ymm14,    (%%rax)			\n\t		vmovaps	%%ymm10,    (%%r10)			\n\t"\
		"vmovaps	%%ymm9 ,    (%%rdx)			\n\t		vmovaps	%%ymm13,0x20(%%r12)			\n\t"\
		"vmovaps	%%ymm15,0x20(%%rax)			\n\t		vmovaps	%%ymm12,0x20(%%r10)			\n\t"\
		"vmovaps	%%ymm8 ,0x20(%%rcx)			\n\t		vmovaps	%%ymm11,    (%%r13)			\n\t"\
		:					/* outputs: none */\
		: [__r0] "m" (Xr0)	/* All inputs from memory addresses here */\
		 ,[__i1] "e" (Xi1)\
		 ,[__i2] "e" (Xi2)\
		 ,[__i3] "e" (Xi3)\
		 ,[__i4] "e" (Xi4)\
		 ,[__i5] "e" (Xi5)\
		 ,[__i6] "e" (Xi6)\
		 ,[__i7] "e" (Xi7)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__o3] "m" (Xo3)\
		 ,[__o4] "m" (Xo4)\
		 ,[__o5] "m" (Xo5)\
		 ,[__o6] "m" (Xo6)\
		 ,[__o7] "m" (Xo7)\
		 ,[__isrt2] "m" (Xisrt2)\
		 ,[__two] "m" (Xtwo)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","r9","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

	// Need a 2nd version of above which takes the i-strides as intvars rather than literal bytes:
	#define SSE2_RADIX8_DIF_0TWIDDLE_B(Xr0, Xi1,Xi2,Xi3,Xi4,Xi5,Xi6,Xi7, Xo0,Xo1,Xo2,Xo3,Xo4,Xo5,Xo6,Xo7, Xisrt2,Xtwo)\
	{\
	__asm__ volatile (\
	/* 1st of 2 radix-4 subtransforms, data in ymm0-7: */	/* 2nd of 2 radix-4 subtransforms, data in ymm8-15: */\
		"movq	%[__isrt2],%%rsi				\n\t		movq	%[__two],%%r9	\n\t"/* r9 holds 2.0 throughout */\
		"movq	%[__r0],%%rax	/* i0 = r00 */	\n\t		movslq	%[__i1],%%r10		/* i1 */	\n\t"\
		"movslq	%[__i2],%%rbx	/* i2 */		\n\t		movslq	%[__i3],%%r11		/* i3 */	\n\t"\
		"movslq	%[__i4],%%rcx	/* i4 */		\n\t		movslq	%[__i5],%%r12		/* i5 */	\n\t"\
		"movslq	%[__i6],%%rdx	/* i6 */		\n\t		movslq	%[__i7],%%r13		/* i7 */	\n\t"\
		"addq	%%rax,%%rbx						\n\t		addq	%%rax,%%r10						\n\t"\
		"addq	%%rax,%%rcx						\n\t		addq	%%rax,%%r11						\n\t"\
		"addq	%%rax,%%rdx						\n\t		addq	%%rax,%%r12						\n\t"\
		"													addq	%%rax,%%r13						\n\t"\
	/* p0,4 combo: x+-y into ymm0/1, 2/3, resp: */		/* p1,5 combo: x+y into ymm8 /1, x-y in ymm10/3: */\
	/* p2,6 combo: x+-y into ymm4/5, 6/7, resp: */		/* p3,7 combo: x+y into ymm14/7, x-y in ymm12/5: */\
		"vmovaps	     (%%rcx),%%ymm0			\n\t		vmovaps	     (%%r12),%%ymm8 			\n\t"\
		"vmovaps	0x020(%%rcx),%%ymm1			\n\t		vmovaps	0x020(%%r12),%%ymm9 			\n\t"\
		"vmovaps	     (%%rax),%%ymm2			\n\t		vmovaps	     (%%r10),%%ymm10			\n\t"\
		"vmovaps	0x020(%%rax),%%ymm3			\n\t		vmovaps	0x020(%%r10),%%ymm11			\n\t"\
		"vmovaps	     (%%rdx),%%ymm4			\n\t		vmovaps	     (%%r11),%%ymm12			\n\t"\
		"vmovaps	0x020(%%rdx),%%ymm5			\n\t		vmovaps	0x020(%%r11),%%ymm13			\n\t"\
		"vmovaps	     (%%rbx),%%ymm6			\n\t		vmovaps	     (%%r13),%%ymm14			\n\t"\
		"vmovaps	0x020(%%rbx),%%ymm7			\n\t		vmovaps	0x020(%%r13),%%ymm15			\n\t"\
		"vsubpd	%%ymm0,%%ymm2,%%ymm2			\n\t		vsubpd	%%ymm8 ,%%ymm10,%%ymm10			\n\t"\
		"vsubpd	%%ymm1,%%ymm3,%%ymm3			\n\t		vsubpd	%%ymm9 ,%%ymm11,%%ymm11			\n\t"\
		"vsubpd	%%ymm4,%%ymm6,%%ymm6			\n\t		vsubpd	%%ymm14,%%ymm12,%%ymm12			\n\t"\
		"vsubpd	%%ymm5,%%ymm7,%%ymm7			\n\t		vsubpd	%%ymm15,%%ymm13,%%ymm13			\n\t"\
	"vmovaps	%%ymm15,(%%rax) 	\n\t"/* spill ymm15 to make room for 2.0 */"	vmovaps	 (%%r9),%%ymm15	\n\t"/* two */\
	"vfmadd132pd	%%ymm15,%%ymm2,%%ymm0		\n\t	vfmadd132pd	%%ymm15,%%ymm10,%%ymm8 	\n\t"\
	"vfmadd132pd	%%ymm15,%%ymm3,%%ymm1		\n\t	vfmadd132pd	%%ymm15,%%ymm11,%%ymm9 	\n\t"\
	"vfmadd132pd	%%ymm15,%%ymm6,%%ymm4		\n\t	vfmadd132pd	%%ymm15,%%ymm12,%%ymm14	\n\t"\
	"vfmadd132pd	%%ymm15,%%ymm7,%%ymm5		\n\t	vfmadd132pd	(%%rax),%%ymm13,%%ymm15	\n\t"\
		/* Finish radix-4 butterfly and store results into temporary-array slots: */\
		"vsubpd		%%ymm4,%%ymm0,%%ymm0		\n\t		vsubpd		%%ymm14,%%ymm8 ,%%ymm8 		\n\t"\
		"vsubpd		%%ymm5,%%ymm1,%%ymm1		\n\t		vsubpd		%%ymm15,%%ymm9 ,%%ymm9 		\n\t"\
		"vsubpd		%%ymm7,%%ymm2,%%ymm2		\n\t		vsubpd		%%ymm13,%%ymm10,%%ymm10		\n\t"\
		"vsubpd		%%ymm6,%%ymm3,%%ymm3		\n\t		vsubpd		%%ymm12,%%ymm11,%%ymm11		\n\t"\
	"vmovaps	%%ymm12,(%%rax) 	\n\t"/* spill ymm12 to make room for 2.0 */"	vmovaps	 (%%r9),%%ymm12	\n\t"/* two */\
	"vfmadd132pd	%%ymm12,%%ymm0,%%ymm4		\n\t	vfmadd132pd		%%ymm12,%%ymm8 ,%%ymm14		\n\t"\
	"vfmadd132pd	%%ymm12,%%ymm1,%%ymm5		\n\t	vfmadd132pd		%%ymm12,%%ymm9 ,%%ymm15		\n\t"\
	"vfmadd132pd	%%ymm12,%%ymm2,%%ymm7		\n\t	vfmadd132pd		%%ymm12,%%ymm10,%%ymm13		\n\t"\
	"vfmadd132pd	%%ymm12,%%ymm3,%%ymm6		\n\t	vfmadd132pd		(%%rax),%%ymm11,%%ymm12		\n\t"\
		"													vsubpd		%%ymm12,%%ymm10,%%ymm10		\n\t"\
		"													vsubpd		%%ymm11,%%ymm13,%%ymm13		\n\t"\
		"												vfmadd132pd		(%%r9 ),%%ymm10,%%ymm12		\n\t"/* .two */\
		"												vfmadd132pd		(%%r9 ),%%ymm13,%%ymm11		\n\t"\
		/* SSE2_RADIX8_DIF_COMBINE_RAD4_SUBS(r00,r10,r20,r30,r08,r18,r28,r38): */\
		"\n\t"\
		"movq	%[__o0],%%rax					\n\t		movq	%[__o4],%%r10				\n\t"\
		"movq	%[__o1],%%rbx					\n\t		movq	%[__o5],%%r11				\n\t"\
		"movq	%[__o2],%%rcx					\n\t		movq	%[__o6],%%r12				\n\t"\
		"movq	%[__o3],%%rdx					\n\t		movq	%[__o7],%%r13				\n\t"\
		/* Combine r00,r08,r20,r28: */						/* Combine r10,r18,r30,r38: */\
		"vsubpd		%%ymm14,%%ymm4 ,%%ymm4 		\n\t	vfnmadd231pd	(%%rsi),%%ymm10,%%ymm2 		\n\t"/* .isrt2 */\
		"vsubpd		%%ymm9 ,%%ymm0 ,%%ymm0 		\n\t	vfnmadd231pd	(%%rsi),%%ymm13,%%ymm3 		\n\t"\
		"vsubpd		%%ymm15,%%ymm5 ,%%ymm5 		\n\t	vfnmadd231pd	(%%rsi),%%ymm12,%%ymm6 		\n\t"\
		"vsubpd		%%ymm8 ,%%ymm1 ,%%ymm1 		\n\t	vfnmadd231pd	(%%rsi),%%ymm11,%%ymm7 		\n\t"\
	"vmovaps	%%ymm8 ,(%%rax) 	\n\t"/* spill ymm8  to make room for 2.0 */"	vmovaps	 (%%r9),%%ymm8 	\n\t"/* two */\
	"vmovaps	%%ymm11,(%%r10) 	\n\t"/* spill ymm11 to make room for sqrt2 */"	vmovaps	0x40(%%r9),%%ymm11 \n\t"/* sqrt2 */\
		"vmovaps	%%ymm4 ,    (%%rbx)			\n\t		vmovaps	%%ymm2 ,    (%%r11)			\n\t"\
		"vmovaps	%%ymm0 ,    (%%rcx)			\n\t		vmovaps	%%ymm3 ,0x20(%%r13)			\n\t"\
		"vmovaps	%%ymm5 ,0x20(%%rbx)			\n\t		vmovaps	%%ymm6 ,0x20(%%r11)			\n\t"\
		"vmovaps	%%ymm1 ,0x20(%%rdx)			\n\t		vmovaps	%%ymm7 ,    (%%r12)			\n\t"\
	"vfmadd132pd	%%ymm8 ,%%ymm4 ,%%ymm14		\n\t	vfmadd132pd		%%ymm11,%%ymm2 ,%%ymm10		\n\t"\
	"vfmadd132pd	%%ymm8 ,%%ymm0 ,%%ymm9 		\n\t	vfmadd132pd		%%ymm11,%%ymm3 ,%%ymm13		\n\t"\
	"vfmadd132pd	%%ymm8 ,%%ymm5 ,%%ymm15		\n\t	vfmadd132pd		%%ymm11,%%ymm6 ,%%ymm12		\n\t"\
	"vfmadd132pd	(%%rax),%%ymm1 ,%%ymm8 		\n\t	vfmadd132pd		(%%r10),%%ymm7 ,%%ymm11		\n\t"\
		"vmovaps	%%ymm14,    (%%rax)			\n\t		vmovaps	%%ymm10,    (%%r10)			\n\t"\
		"vmovaps	%%ymm9 ,    (%%rdx)			\n\t		vmovaps	%%ymm13,0x20(%%r12)			\n\t"\
		"vmovaps	%%ymm15,0x20(%%rax)			\n\t		vmovaps	%%ymm12,0x20(%%r10)			\n\t"\
		"vmovaps	%%ymm8 ,0x20(%%rcx)			\n\t		vmovaps	%%ymm11,    (%%r13)			\n\t"\
		:					/* outputs: none */\
		: [__r0] "m" (Xr0)	/* All inputs from memory addresses here */\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__i3] "m" (Xi3)\
		 ,[__i4] "m" (Xi4)\
		 ,[__i5] "m" (Xi5)\
		 ,[__i6] "m" (Xi6)\
		 ,[__i7] "m" (Xi7)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__o3] "m" (Xo3)\
		 ,[__o4] "m" (Xo4)\
		 ,[__o5] "m" (Xo5)\
		 ,[__o6] "m" (Xo6)\
		 ,[__o7] "m" (Xo7)\
		 ,[__isrt2] "m" (Xisrt2)\
		 ,[__two] "m" (Xtwo)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","r9","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

	// AVX analog of dft_macro.h::RADIX_08_DIF_TWIDDLE_OOP - Result of adding separate I/O addressing to
	// radix8_dif_dit_pass_gcc64.h::SSE2_RADIX8_DIF_TWIDDLE.
	//
	// [rsi] (and if needed rdi) points to sine components of each sincos pair, which is not really a pair here in terms of relative addressing.
	//
	#define SSE2_RADIX8_DIF_TWIDDLE_OOP(Xi0,Xi1,Xi2,Xi3,Xi4,Xi5,Xi6,Xi7, Xo0,Xo1,Xo2,Xo3,Xo4,Xo5,Xo6,Xo7 ,Xc1,Xs1,Xc2,Xs2,Xc3,Xs3,Xc4,Xs4,Xc5,Xs5,Xc6,Xs6,Xc7,Xs7)\
	{\
	__asm__ volatile (\
		"movq		%[i0]	,%%rax				\n\t		movq		%[i1]	,%%r10				\n\t"\
		"movq		%[i4]	,%%rbx				\n\t		movq		%[i5]	,%%r11				\n\t"\
		"movq		%[c4]	,%%rcx				\n\t	movq	%[c1],%%r12	\n\t	movq	%[c5],%%r14	\n\t"\
		"movq		%[s4]	,%%rsi				\n\t	movq	%[s1],%%r13	\n\t	movq	%[s5],%%r15	\n\t"\
		"vmovaps	    (%%rbx)	,%%ymm2			\n\t		vmovaps	    (%%r10)	,%%ymm8 			\n\t"\
		"vmovaps	0x20(%%rbx)	,%%ymm3			\n\t		vmovaps	0x20(%%r10)	,%%ymm10			\n\t"\
		"vmovaps		%%ymm2	,%%ymm4			\n\t		vmovaps		%%ymm8 	,%%ymm9 			\n\t"\
		"vmovaps		%%ymm3	,%%ymm5			\n\t		vmovaps		%%ymm10	,%%ymm11			\n\t"\
		"vmulpd		(%%rcx)	,%%ymm2,%%ymm2		\n\t		vmulpd		(%%r12)	,%%ymm8 ,%%ymm8 	\n\t"\
		"vmulpd		(%%rcx)	,%%ymm3,%%ymm3		\n\t		vmulpd		(%%r13)	,%%ymm9 ,%%ymm9 	\n\t"\
	"vfnmadd231pd	(%%rsi)	,%%ymm5,%%ymm2		\n\t	vfnmadd231pd	(%%r13)	,%%ymm10,%%ymm8 	\n\t"\
	" vfmadd231pd	(%%rsi)	,%%ymm4,%%ymm3		\n\t	 vfmadd231pd	(%%r12)	,%%ymm11,%%ymm9 	\n\t"\
		"vmovaps	    (%%rax)	,%%ymm0			\n\t		vmovaps	    (%%r11)	,%%ymm10			\n\t"\
		"vmovaps	0x20(%%rax)	,%%ymm1			\n\t		vmovaps	0x20(%%r11)	,%%ymm11			\n\t"\
		"vmovaps		%%ymm0	,%%ymm6			\n\t		vmovaps	    (%%r11)	,%%ymm12			\n\t"\
		"vmovaps		%%ymm1	,%%ymm7			\n\t		vmovaps	0x20(%%r11)	,%%ymm13			\n\t"\
		"vaddpd	%%ymm2		,%%ymm0,%%ymm0		\n\t		vmulpd		(%%r14)	,%%ymm10,%%ymm10	\n\t"\
		"vaddpd	%%ymm3		,%%ymm1,%%ymm1		\n\t		vmulpd		(%%r15)	,%%ymm12,%%ymm12	\n\t"\
		"vsubpd	%%ymm2		,%%ymm6,%%ymm6		\n\t	vfnmadd231pd	(%%r15)	,%%ymm11,%%ymm10	\n\t	vmovaps	%%ymm10,%%ymm11	\n\t"\
		"vsubpd	%%ymm3		,%%ymm7,%%ymm7		\n\t	 vfmadd231pd	(%%r14)	,%%ymm13,%%ymm12	\n\t	vmovaps	%%ymm12,%%ymm13	\n\t"\
		"vmovaps	%%ymm0		,    (%%rax)	\n\t		vaddpd	%%ymm8 		,%%ymm10,%%ymm10	\n\t"\
		"vmovaps	%%ymm1		,0x20(%%rax)	\n\t		vsubpd	%%ymm11		,%%ymm8 ,%%ymm8 	\n\t"\
		"vmovaps	%%ymm6		,    (%%rbx)	\n\t		vaddpd	%%ymm9 		,%%ymm12,%%ymm12	\n\t"\
		"vmovaps	%%ymm7		,0x20(%%rbx)	\n\t		vsubpd	%%ymm13		,%%ymm9 ,%%ymm9 	\n\t"\
		"movq		%[i2]	,%%rax				\n\t		vmovaps	%%ymm10		,    (%%r10)	\n\t"\
		"movq		%[i6]	,%%rbx				\n\t		vmovaps	%%ymm12		,0x20(%%r10)	\n\t"\
		"movq		%[c2]	,%%rcx				\n\t		vmovaps	%%ymm8 		,    (%%r11)	\n\t"\
		"movq		%[c6]	,%%rdx				\n\t		vmovaps	%%ymm9 		,0x20(%%r11)	\n\t"\
		"movq		%[s2]	,%%rsi				\n\t	movq %[i3],%%r10	\n\t	movq %[c3],%%r12	\n\t	movq %[s3],%%r13	\n\t"\
		"movq		%[s6]	,%%rdi				\n\t	movq %[i7],%%r11	\n\t	movq %[c7],%%r14	\n\t	movq %[s7],%%r15	\n\t"\
		"vmovaps	    (%%rax)	,%%ymm0			\n\t		vmovaps	    (%%r10)	,%%ymm8 			\n\t"\
		"vmovaps	0x20(%%rax)	,%%ymm2			\n\t		vmovaps	0x20(%%r10)	,%%ymm10			\n\t"\
		"vmovaps		%%ymm0	,%%ymm1			\n\t		vmovaps		%%ymm8 	,%%ymm9 			\n\t"\
		"vmovaps		%%ymm2	,%%ymm3			\n\t		vmovaps		%%ymm10	,%%ymm11			\n\t"\
		"vmulpd		(%%rcx)	,%%ymm0,%%ymm0		\n\t		vmulpd		(%%r12)	,%%ymm8 ,%%ymm8 	\n\t"\
		"vmulpd		(%%rsi)	,%%ymm1,%%ymm1		\n\t		vmulpd		(%%r13)	,%%ymm9 ,%%ymm9 	\n\t"\
	"vfnmadd231pd	(%%rsi)	,%%ymm2,%%ymm0		\n\t	vfnmadd231pd	(%%r13)	,%%ymm10,%%ymm8 	\n\t"\
	" vfmadd231pd	(%%rcx)	,%%ymm3,%%ymm1		\n\t	 vfmadd231pd	(%%r12)	,%%ymm11,%%ymm9 	\n\t"\
		"vmovaps	    (%%rbx)	,%%ymm2			\n\t		vmovaps	    (%%r11)	,%%ymm10			\n\t"\
		"vmovaps	0x20(%%rbx)	,%%ymm3			\n\t		vmovaps	0x20(%%r11)	,%%ymm11			\n\t"\
		"vmovaps		%%ymm2	,%%ymm4			\n\t		vmovaps		%%ymm10	,%%ymm12			\n\t"\
		"vmovaps		%%ymm3	,%%ymm5			\n\t		vmovaps		%%ymm11	,%%ymm13			\n\t"\
		"vmulpd		(%%rdx)	,%%ymm2,%%ymm2		\n\t		vmulpd		(%%r14)	,%%ymm10,%%ymm10	\n\t"\
		"vmulpd		(%%rdi)	,%%ymm4,%%ymm4		\n\t		vmulpd		(%%r15)	,%%ymm12,%%ymm12	\n\t"\
	"vfnmadd231pd	(%%rdi)	,%%ymm3,%%ymm2		\n\t	vfnmadd231pd	(%%r15)	,%%ymm11,%%ymm10	\n\t"\
	" vfmadd231pd	(%%rdx)	,%%ymm5,%%ymm4		\n\t	 vfmadd231pd	(%%r14)	,%%ymm13,%%ymm12	\n\t"\
		"vmovaps	%%ymm2		,%%ymm3			\n\t		vmovaps	%%ymm10		,%%ymm11			\n\t"\
		"vmovaps	%%ymm4		,%%ymm5			\n\t		vmovaps	%%ymm12		,%%ymm13			\n\t"\
		"vaddpd	%%ymm0		,%%ymm2,%%ymm2		\n\t		vaddpd	%%ymm8 		,%%ymm10,%%ymm10	\n\t"\
		"vsubpd	%%ymm3		,%%ymm0,%%ymm0		\n\t		vsubpd	%%ymm11		,%%ymm8 ,%%ymm8 	\n\t"\
		"vaddpd	%%ymm1		,%%ymm4,%%ymm4		\n\t		vaddpd	%%ymm9 		,%%ymm12,%%ymm12	\n\t"\
		"vsubpd	%%ymm5		,%%ymm1,%%ymm1		\n\t		vsubpd	%%ymm13		,%%ymm9 ,%%ymm9 	\n\t"\
		"vmovaps	%%ymm2		,    (%%rax)	\n\t		vmovaps	%%ymm10		,    (%%r10)	\n\t"\
		"vmovaps	%%ymm4		,0x20(%%rax)	\n\t		vmovaps	%%ymm12		,0x20(%%r10)	\n\t"\
		"vmovaps	%%ymm0		,    (%%rbx)	\n\t		vmovaps	%%ymm8 		,    (%%r11)	\n\t"\
		"vmovaps	%%ymm1		,0x20(%%rbx)	\n\t		vmovaps	%%ymm9 		,0x20(%%r11)	\n\t"\
	/* combine to get 2 length-4 output subtransforms... */\
		"movq		%[i0]	,%%rax				\n\t		movq		%[i4]	,%%r10				\n\t"\
		"movq		%[i2]	,%%rbx				\n\t		movq		%[i6]	,%%r11				\n\t"\
		"vmovaps	    (%%rbx)	,%%ymm0			\n\t		vmovaps	    (%%r11)	,%%ymm9 			\n\t"\
		"vmovaps	0x20(%%rbx)	,%%ymm1			\n\t		vmovaps	0x20(%%r11)	,%%ymm12			\n\t"\
		"vmovaps	    (%%rax)	,%%ymm4			\n\t		vmovaps	    (%%r10)	,%%ymm8 			\n\t"\
		"vmovaps	0x20(%%rax)	,%%ymm5			\n\t		vmovaps	0x20(%%r10)	,%%ymm13			\n\t"\
		"movq		%[i1]	,%%rcx				\n\t		movq		%[i5]	,%%r12				\n\t"\
		"movq		%[i3]	,%%rdx				\n\t		movq		%[i7]	,%%r13				\n\t"\
		"vmovaps	    (%%rdx)	,%%ymm2			\n\t		vmovaps	    (%%r13)	,%%ymm11			\n\t"\
		"vmovaps	0x20(%%rdx)	,%%ymm3			\n\t		vmovaps	0x20(%%r13)	,%%ymm14			\n\t"\
		"vmovaps	    (%%rcx)	,%%ymm6			\n\t		vmovaps	    (%%r12)	,%%ymm10			\n\t"\
		"vmovaps	0x20(%%rcx)	,%%ymm7			\n\t		vmovaps	0x20(%%r12)	,%%ymm15			\n\t"\
		"movq		%[o0]	,%%rax				\n\t		movq		%[o4]	,%%r10				\n\t"/* [o0] has 2.0 on input */\
		"movq		%[o2]	,%%rbx				\n\t		movq		%[o6]	,%%r11				\n\t"\
		"vsubpd		%%ymm0	,%%ymm4,%%ymm4		\n\t		vsubpd		%%ymm9 ,%%ymm13,%%ymm13		\n\t"\
		"vsubpd		%%ymm1	,%%ymm5,%%ymm5		\n\t		vsubpd		%%ymm12,%%ymm8 ,%%ymm8 		\n\t"\
		"vsubpd		%%ymm2	,%%ymm6,%%ymm6		\n\t		vsubpd		%%ymm11,%%ymm15,%%ymm15		\n\t"\
		"vsubpd		%%ymm3	,%%ymm7,%%ymm7		\n\t		vsubpd		%%ymm14,%%ymm10,%%ymm10		\n\t"\
	"vmovaps	%%ymm14,(%%rbx) 	\n\t"/* spill ymm14 to make room for 2.0 */"	vmovaps	 (%%rax),%%ymm14	\n\t"/* two */\
	"vfmadd132pd	%%ymm14,%%ymm4,%%ymm0		\n\t	vfmadd132pd		%%ymm14,%%ymm13,%%ymm9 		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm5,%%ymm1		\n\t	vfmadd132pd		%%ymm14,%%ymm8 ,%%ymm12		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm6,%%ymm2		\n\t	vfmadd132pd		%%ymm14,%%ymm15,%%ymm11		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm7,%%ymm3		\n\t	vfmadd132pd		(%%rbx),%%ymm10,%%ymm14		\n\t"\
		"vsubpd	%%ymm2		,%%ymm0,%%ymm0		\n\t		vsubpd	%%ymm11		,%%ymm10,%%ymm10	\n\t"\
		"vsubpd	%%ymm3		,%%ymm1,%%ymm1		\n\t		vsubpd	%%ymm14		,%%ymm15,%%ymm15	\n\t"\
		"vsubpd	%%ymm7		,%%ymm4,%%ymm4		\n\t		vfmadd132pd	(%%rax) ,%%ymm10,%%ymm11	\n\t"\
		"vsubpd	%%ymm6		,%%ymm5,%%ymm5		\n\t		vfmadd132pd	(%%rax) ,%%ymm15,%%ymm14	\n\t"\
		"movq		%[o1]	,%%rcx				\n\t		movq		%[o5]	,%%r12				\n\t"/* [o1] has SQRT2 on input */\
		"movq		%[o3]	,%%rdx				\n\t		movq		%[o7]	,%%r13				\n\t"\
	/* Use the cosine term of the [c1,s1] pair, which is the *middle* [4th of 7] of our 7 input pairs, in terms \
	of the input-arg bit-reversal reordering defined in the __X[c,s] --> [c,s] mapping below and happens to \
	always in fact *be* a true cosine term, which is a requirement for our "decr 1 gives isrt2" data-copy scheme: */\
		"movq	%[c1],%%r14	\n\t				\n\t"\
		"subq	$0x20,%%r14	\n\t"/* isrt2 in [c1]-1 */\
	"vfmadd132pd	(%%rax),%%ymm0,%%ymm2		\n\t	vfnmadd231pd	(%%r14),%%ymm10,%%ymm8 		\n\t"/* .isrt2 */\
	"vfmadd132pd	(%%rax),%%ymm1,%%ymm3		\n\t	vfnmadd231pd	(%%r14),%%ymm15,%%ymm13		\n\t"\
	"vfmadd132pd	(%%rax),%%ymm5,%%ymm6		\n\t	vfnmadd231pd	(%%r14),%%ymm11,%%ymm9 		\n\t"\
	"vfmadd132pd	(%%rax),%%ymm4,%%ymm7		\n\t	vfnmadd231pd	(%%r14),%%ymm14,%%ymm12		\n\t"\
		"vmovaps	%%ymm2,    (%%rax)	\n\t"/* [o0].re */"	vmovaps	%%ymm8 ,    (%%r12)	\n\t"/* [o5].re */\
		"vmovaps	%%ymm3,0x20(%%rax)	\n\t"/* [o0].im */"	vmovaps	%%ymm13,0x20(%%r11)	\n\t"/* [o6].im */\
		"vmovaps	%%ymm6,0x20(%%rbx)	\n\t"/* [o2].im */"	vmovaps	%%ymm9 ,0x20(%%r12)	\n\t"/* [o5].im */\
		"vmovaps	%%ymm7,    (%%rdx)	\n\t"/* [o3].re */"	vmovaps	%%ymm12,    (%%r11)	\n\t"/* [o6].re */\
		"												 vfmadd132pd	(%%rcx),%%ymm8 ,%%ymm10		\n\t"/* .sqrt2 */\
		"												 vfmadd132pd	(%%rcx),%%ymm13,%%ymm15		\n\t"\
		"												 vfmadd132pd	(%%rcx),%%ymm9 ,%%ymm11		\n\t"\
		"												 vfmadd132pd	(%%rcx),%%ymm12,%%ymm14		\n\t"\
		"vmovaps	%%ymm0,    (%%rcx)	\n\t"/* [o1].re */"	vmovaps	%%ymm10,    (%%r10)	\n\t"/* [o4].re */\
		"vmovaps	%%ymm1,0x20(%%rcx)	\n\t"/* [o1].im */"	vmovaps	%%ymm15,0x20(%%r13)	\n\t"/* [o7].im */\
		"vmovaps	%%ymm4,    (%%rbx)	\n\t"/* [o2].re */"	vmovaps	%%ymm11,0x20(%%r10)	\n\t"/* [o4].im */\
		"vmovaps	%%ymm5,0x20(%%rdx)	\n\t"/* [o3].im */"	vmovaps	%%ymm14,    (%%r13)	\n\t"/* [o7].re */\
		:					/* outputs: none */\
		: [i0] "m" (Xi0)	/* All inputs from memory addresses here */\
		 ,[i1] "m" (Xi1)\
		 ,[i2] "m" (Xi2)\
		 ,[i3] "m" (Xi3)\
		 ,[i4] "m" (Xi4)\
		 ,[i5] "m" (Xi5)\
		 ,[i6] "m" (Xi6)\
		 ,[i7] "m" (Xi7)\
		 ,[o0] "m" (Xo0)\
		 ,[o1] "m" (Xo1)\
		 ,[o2] "m" (Xo2)\
		 ,[o3] "m" (Xo3)\
		 ,[o4] "m" (Xo4)\
		 ,[o5] "m" (Xo5)\
		 ,[o6] "m" (Xo6)\
		 ,[o7] "m" (Xo7)\
		 ,[c4] "m" (Xc1),[s4] "m" (Xs1)\
		 ,[c2] "m" (Xc2),[s2] "m" (Xs2)\
		 ,[c6] "m" (Xc3),[s6] "m" (Xs3)\
		 ,[c1] "m" (Xc4),[s1] "m" (Xs4)\
		 ,[c5] "m" (Xc5),[s5] "m" (Xs5)\
		 ,[c3] "m" (Xc6),[s3] "m" (Xs6)\
		 ,[c7] "m" (Xc7),[s7] "m" (Xs7)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","rdi","r10","r11","r12","r13","r14","r15","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"	/* Clobbered registers */\
	);\
	}

	/* Twiddleless version of SSE2_RADIX8_DIT_TWIDDLE. Inputs enter in memory locations __i0,__i1,__i2,__i3,__i4,__i5,__i6,__i7.
	Outputs go into 16 contiguous 32-byte memory locations starting at __out and assumed disjoint with inputs.
	This macro built on the same code template as SSE2_RADIX8_DIF_TWIDDLE0, but with the I/O-location indices mutually bit reversed:
	01234567 <--> 04261537, which can be effected via the pairwise swaps 1 <--> 4 and 3 <--> 6.
	*/
	#define	SSE2_RADIX8_DIT_0TWIDDLE(Xi0,Xi1,Xi2,Xi3,Xi4,Xi5,Xi6,Xi7, Xout, Xisrt2,Xtwo)\
	{\
	__asm__ volatile (\
		"movq	%[__isrt2],%%rdi				\n\t		movq	%[__two],%%r9	\n\t"/* r9 holds 2.0 throughout */\
		"movq	%[__out],%%rsi	\n\t"\
	/* 1st of 2 radix-4 subtransforms, data in ymm0-7: *//* 2nd of 2 radix-4 subtransforms, data in ymm8-15: */\
		"movq	%[__i0],%%rax					\n\t		movq	%[__i4],%%r10					\n\t"\
		"movq	%[__i1],%%rbx					\n\t		movq	%[__i5],%%r11					\n\t"\
		"movq	%[__i2],%%rcx					\n\t		movq	%[__i6],%%r12					\n\t"\
		"movq	%[__i3],%%rdx					\n\t		movq	%[__i7],%%r13					\n\t"\
		"vmovaps	    (%%rax),%%ymm2				\n\t		vmovaps	    (%%r10),%%ymm10	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3				\n\t		vmovaps	0x20(%%r10),%%ymm11	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6				\n\t		vmovaps	    (%%r11),%%ymm8 	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7				\n\t		vmovaps	0x20(%%r11),%%ymm9 	\n\t"\
		"vmovaps	    (%%rbx),%%ymm0				\n\t		vmovaps	    (%%r12),%%ymm14	\n\t"\
		"vmovaps	    (%%rdx),%%ymm4				\n\t		vmovaps	0x20(%%r12),%%ymm15	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm1				\n\t		vmovaps	    (%%r13),%%ymm12	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5				\n\t"	/*	vmovaps	0x20(%%r13),%%ymm13	Instead use ymm13 for 2.0: */"	vmovaps	(%%r9),%%ymm13 	\n\t"\
		"vsubpd		%%ymm0,%%ymm2,%%ymm2			\n\t		vsubpd		%%ymm8 ,%%ymm10,%%ymm10		\n\t"\
		"vsubpd		%%ymm1,%%ymm3,%%ymm3			\n\t		vsubpd		%%ymm9 ,%%ymm11,%%ymm11		\n\t"\
		"vsubpd		%%ymm4,%%ymm6,%%ymm6			\n\t		vsubpd		%%ymm12,%%ymm14,%%ymm14		\n\t"\
		"vsubpd		%%ymm5,%%ymm7,%%ymm7			\n\t		vsubpd	0x20(%%r13),%%ymm15,%%ymm15		\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm2,%%ymm0			\n\t	vfmadd132pd		%%ymm13,%%ymm10,%%ymm8 		\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm3,%%ymm1			\n\t	vfmadd132pd		%%ymm13,%%ymm11,%%ymm9 		\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm6,%%ymm4			\n\t	vfmadd132pd		%%ymm13,%%ymm14,%%ymm12		\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm7,%%ymm5			\n\t	vfmadd132pd	0x20(%%r13),%%ymm15,%%ymm13		\n\t"\
		"vsubpd		%%ymm7,%%ymm2,%%ymm2			\n\t		vsubpd		%%ymm12,%%ymm8 ,%%ymm8 		\n\t"\
		"vsubpd		%%ymm6,%%ymm3,%%ymm3			\n\t		vsubpd		%%ymm13,%%ymm9 ,%%ymm9 		\n\t"\
		"vsubpd		%%ymm4,%%ymm0,%%ymm0			\n\t		vsubpd		%%ymm14,%%ymm11,%%ymm11		\n\t"\
		"vsubpd		%%ymm5,%%ymm1,%%ymm1			\n\t		vsubpd		%%ymm15,%%ymm10,%%ymm10		\n\t"\
	"vmovaps	%%ymm14,(%%rsi) 	\n\t"/* spill ymm14 to make room for 2.0 */"	vmovaps	(%%r9),%%ymm14 	\n\t"/* two */\
	"vfmadd132pd	%%ymm14,%%ymm0,%%ymm4			\n\t	vfmadd132pd		%%ymm14,%%ymm8 ,%%ymm12		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm1,%%ymm5			\n\t	vfmadd132pd		%%ymm14,%%ymm9 ,%%ymm13		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm2,%%ymm7			\n\t	vfmadd132pd		%%ymm14,%%ymm10,%%ymm15		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm3,%%ymm6			\n\t	vfmadd132pd		(%%rsi),%%ymm11,%%ymm14		\n\t"\
		"														vsubpd		%%ymm15,%%ymm11,%%ymm11		\n\t"\
		"														vsubpd		%%ymm10,%%ymm14,%%ymm14		\n\t"\
		"													vfmadd132pd		(%%r9 ),%%ymm11,%%ymm15		\n\t"/* .two */\
		"													vfmadd132pd		(%%r9 ),%%ymm14,%%ymm10		\n\t"\
		/* Outputs 1-7 order-reversed in the SIMD version of this macro: Thus swap output byte-offset pairs */\
		/* 0x[40,60] <-> [1c0,1e0], [80,a0] <-> [180,1a0], [c0,e0] <-> [140,160] : */\
		"vsubpd		%%ymm9 ,%%ymm0 ,%%ymm0 			\n\t	vfnmadd231pd	(%%rdi),%%ymm14,%%ymm2 		\n\t"/* .isrt2 */\
		"vsubpd		%%ymm13,%%ymm5 ,%%ymm5 			\n\t	vfnmadd231pd	(%%rdi),%%ymm11,%%ymm3 		\n\t"\
		"vsubpd		%%ymm12,%%ymm4 ,%%ymm4 			\n\t	vfnmadd231pd	(%%rdi),%%ymm15,%%ymm7 		\n\t"\
		"vsubpd		%%ymm8 ,%%ymm1 ,%%ymm1 			\n\t	vfnmadd231pd	(%%rdi),%%ymm10,%%ymm6 		\n\t"\
	"vmovaps	%%ymm8 ,0x20(%%rsi) 	\n\t"/* spill ymm8  to make room for 2.0 */"	vmovaps	 (%%r9),%%ymm8	\n\t"/* two */\
	"vmovaps	%%ymm10,(%%rsi) 	\n\t"/* spill ymm10 to make room for sqrt2 */"	vmovaps	0x40(%%r9),%%ymm10	\n\t"/* sqrt2 */\
		"vmovaps	%%ymm0 ,0x080(%%rsi)			\n\t		vmovaps		%%ymm2 ,0x040(%%rsi)		\n\t"\
		"vmovaps	%%ymm5 ,0x120(%%rsi)			\n\t		vmovaps		%%ymm3 ,0x0e0(%%rsi)		\n\t"\
		"vmovaps	%%ymm4 ,0x100(%%rsi)			\n\t		vmovaps		%%ymm7 ,0x0c0(%%rsi)		\n\t"\
		"vmovaps	%%ymm1 ,0x1a0(%%rsi)			\n\t		vmovaps		%%ymm6 ,0x160(%%rsi)		\n\t"\
	"vfmadd132pd		%%ymm8 ,%%ymm0 ,%%ymm9 		\n\t	vfmadd132pd		%%ymm10,%%ymm2 ,%%ymm14		\n\t"\
	"vfmadd132pd		%%ymm8 ,%%ymm5 ,%%ymm13		\n\t	vfmadd132pd		%%ymm10,%%ymm3 ,%%ymm11		\n\t"\
	"vfmadd132pd		%%ymm8 ,%%ymm4 ,%%ymm12		\n\t	vfmadd132pd		%%ymm10,%%ymm7 ,%%ymm15		\n\t"\
	"vfmadd132pd	0x20(%%rsi),%%ymm1 ,%%ymm8 		\n\t	vfmadd132pd		(%%rsi),%%ymm6 ,%%ymm10		\n\t"\
		"vmovaps	%%ymm9 ,0x180(%%rsi)			\n\t		vmovaps		%%ymm14,0x140(%%rsi)		\n\t"\
		"vmovaps	%%ymm13,0x020(%%rsi)			\n\t		vmovaps		%%ymm11,0x1e0(%%rsi)		\n\t"\
		"vmovaps	%%ymm12,     (%%rsi)			\n\t		vmovaps		%%ymm15,0x1c0(%%rsi)		\n\t"\
		"vmovaps	%%ymm8 ,0x0a0(%%rsi)			\n\t		vmovaps		%%ymm10,0x060(%%rsi)		\n\t"\
		:					/* outputs: none */\
		: [__i0] "m" (Xi0)	/* All iputs from memory addresses here */\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__i3] "m" (Xi3)\
		 ,[__i4] "m" (Xi4)\
		 ,[__i5] "m" (Xi5)\
		 ,[__i6] "m" (Xi6)\
		 ,[__i7] "m" (Xi7)\
		 ,[__out] "m" (Xout)\
		 ,[__isrt2] "m" (Xisrt2)\
		 ,[__two] "m" (Xtwo)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","rdi","r9","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

	// Same as SSE2_RADIX8_DIT_0TWIDDLE but with user-specifiable [i.e. not nec. contiguous] output addresses:
	#define	SSE2_RADIX8_DIT_0TWIDDLE_OOP(Xi0,Xi1,Xi2,Xi3,Xi4,Xi5,Xi6,Xi7, Xo0,Xo1,Xo2,Xo3,Xo4,Xo5,Xo6,Xo7, Xisrt2,Xtwo)\
	{\
	__asm__ volatile (\
		"movq	%[__isrt2],%%rdi				\n\t		movq	%[__two],%%r9	\n\t"/* r9 holds 2.0 throughout */\
	/* 1st of 2 radix-4 subtransforms, data in ymm0-7: *//* 2nd of 2 radix-4 subtransforms, data in ymm8-15: */\
		"movq	%[__i0],%%rax					\n\t		movq	%[__i4],%%r10					\n\t"\
		"movq	%[__i1],%%rbx					\n\t		movq	%[__i5],%%r11					\n\t"\
		"movq	%[__i2],%%rcx					\n\t		movq	%[__i6],%%r12					\n\t"\
		"movq	%[__i3],%%rdx					\n\t		movq	%[__i7],%%r13					\n\t"\
		"vmovaps	    (%%rax),%%ymm2				\n\t		vmovaps	    (%%r10),%%ymm10	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3				\n\t		vmovaps	0x20(%%r10),%%ymm11	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6				\n\t		vmovaps	    (%%r11),%%ymm8 	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7				\n\t		vmovaps	0x20(%%r11),%%ymm9 	\n\t"\
		"vmovaps	    (%%rbx),%%ymm0				\n\t		vmovaps	    (%%r12),%%ymm14	\n\t"\
		"vmovaps	    (%%rdx),%%ymm4				\n\t		vmovaps	0x20(%%r12),%%ymm15	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm1				\n\t		vmovaps	    (%%r13),%%ymm12	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5				\n\t"	/*	vmovaps	0x20(%%r13),%%ymm13	Instead use ymm13 for 2.0: */"	vmovaps	(%%r9),%%ymm13 	\n\t"\
		"vsubpd		%%ymm0,%%ymm2,%%ymm2			\n\t		vsubpd		%%ymm8 ,%%ymm10,%%ymm10		\n\t"\
		"vsubpd		%%ymm1,%%ymm3,%%ymm3			\n\t		vsubpd		%%ymm9 ,%%ymm11,%%ymm11		\n\t"\
		"vsubpd		%%ymm4,%%ymm6,%%ymm6			\n\t		vsubpd		%%ymm12,%%ymm14,%%ymm14		\n\t"\
		"vsubpd		%%ymm5,%%ymm7,%%ymm7			\n\t		vsubpd	0x20(%%r13),%%ymm15,%%ymm15		\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm2,%%ymm0			\n\t	vfmadd132pd		%%ymm13,%%ymm10,%%ymm8 		\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm3,%%ymm1			\n\t	vfmadd132pd		%%ymm13,%%ymm11,%%ymm9 		\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm6,%%ymm4			\n\t	vfmadd132pd		%%ymm13,%%ymm14,%%ymm12		\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm7,%%ymm5			\n\t	vfmadd132pd	0x20(%%r13),%%ymm15,%%ymm13		\n\t"\
		"vsubpd		%%ymm7,%%ymm2,%%ymm2			\n\t		vsubpd		%%ymm12,%%ymm8 ,%%ymm8 		\n\t"\
		"vsubpd		%%ymm6,%%ymm3,%%ymm3			\n\t		vsubpd		%%ymm13,%%ymm9 ,%%ymm9 		\n\t"\
		"vsubpd		%%ymm4,%%ymm0,%%ymm0			\n\t		vsubpd		%%ymm14,%%ymm11,%%ymm11		\n\t"\
		"vsubpd		%%ymm5,%%ymm1,%%ymm1			\n\t		vsubpd		%%ymm15,%%ymm10,%%ymm10		\n\t"\
	"vmovaps	%%ymm14,(%%rax) 	\n\t"/* spill ymm14 to make room for 2.0 */"	vmovaps	(%%r9),%%ymm14 	\n\t"/* two */\
	"vfmadd132pd	%%ymm14,%%ymm0,%%ymm4			\n\t	vfmadd132pd		%%ymm14,%%ymm8 ,%%ymm12		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm1,%%ymm5			\n\t	vfmadd132pd		%%ymm14,%%ymm9 ,%%ymm13		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm2,%%ymm7			\n\t	vfmadd132pd		%%ymm14,%%ymm10,%%ymm15		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm3,%%ymm6			\n\t	vfmadd132pd		(%%rax),%%ymm11,%%ymm14		\n\t"\
		"														vsubpd		%%ymm15,%%ymm11,%%ymm11		\n\t"\
		"														vsubpd		%%ymm10,%%ymm14,%%ymm14		\n\t"\
		"													vfmadd132pd		(%%r9 ),%%ymm11,%%ymm15		\n\t"/* .two */\
		"													vfmadd132pd		(%%r9 ),%%ymm14,%%ymm10		\n\t"\
		/* Outputs 1-7 order-reversed in the SIMD version of this macro: Thus swap output byte-offset pairs */\
		/* 0x[40,60] <-> [1c0,1e0], [80,a0] <-> [180,1a0], [c0,e0] <-> [140,160] : */\
		"movq	%[__o0],%%rax						\n\t		movq	%[__o4],%%r10					\n\t"\
		"movq	%[__o1],%%rbx						\n\t		movq	%[__o5],%%r11					\n\t"\
		"vsubpd		%%ymm9 ,%%ymm0 ,%%ymm0 			\n\t	vfnmadd231pd	(%%rdi),%%ymm14,%%ymm2 		\n\t"/* .isrt2 */\
		"vsubpd		%%ymm13,%%ymm5 ,%%ymm5 			\n\t	vfnmadd231pd	(%%rdi),%%ymm11,%%ymm3 		\n\t"\
		"vsubpd		%%ymm12,%%ymm4 ,%%ymm4 			\n\t	vfnmadd231pd	(%%rdi),%%ymm15,%%ymm7 		\n\t"\
		"vsubpd		%%ymm8 ,%%ymm1 ,%%ymm1 			\n\t	vfnmadd231pd	(%%rdi),%%ymm10,%%ymm6 		\n\t"\
		"movq	%[__o2],%%rcx						\n\t		movq	%[__o6],%%r12					\n\t"\
		"movq	%[__o3],%%rdx						\n\t		movq	%[__o7],%%r13					\n\t"\
	"vmovaps	%%ymm8 ,0x20(%%rax) 	\n\t"/* spill ymm8  to make room for 2.0 */"	vmovaps	 (%%r9),%%ymm8	\n\t"/* two */\
	"vmovaps	%%ymm10,(%%rax) 	\n\t"/* spill ymm10 to make room for sqrt2 */"	vmovaps	0x40(%%r9),%%ymm10	\n\t"/* sqrt2 */\
		"vmovaps	%%ymm0 ,    (%%rcx)				\n\t		vmovaps		%%ymm2 ,    (%%rbx)		\n\t"\
		"vmovaps	%%ymm5 ,0x20(%%r10)				\n\t		vmovaps		%%ymm3 ,0x20(%%rdx)		\n\t"\
		"vmovaps	%%ymm4 ,    (%%r10)				\n\t		vmovaps		%%ymm7 ,    (%%rdx)		\n\t"\
		"vmovaps	%%ymm1 ,0x20(%%r12)				\n\t		vmovaps		%%ymm6 ,0x20(%%r11)		\n\t"\
	"vfmadd132pd		%%ymm8 ,%%ymm0 ,%%ymm9 		\n\t	vfmadd132pd		%%ymm10,%%ymm2 ,%%ymm14		\n\t"\
	"vfmadd132pd		%%ymm8 ,%%ymm5 ,%%ymm13		\n\t	vfmadd132pd		%%ymm10,%%ymm3 ,%%ymm11		\n\t"\
	"vfmadd132pd		%%ymm8 ,%%ymm4 ,%%ymm12		\n\t	vfmadd132pd		%%ymm10,%%ymm7 ,%%ymm15		\n\t"\
	"vfmadd132pd	0x20(%%rax),%%ymm1 ,%%ymm8 		\n\t	vfmadd132pd		(%%rax),%%ymm6 ,%%ymm10		\n\t"\
		"vmovaps	%%ymm9 ,    (%%r12)				\n\t		vmovaps		%%ymm14,    (%%r11)		\n\t"\
		"vmovaps	%%ymm13,0x20(%%rax)				\n\t		vmovaps		%%ymm11,0x20(%%r13)		\n\t"\
		"vmovaps	%%ymm12,    (%%rax)				\n\t		vmovaps		%%ymm15,    (%%r13)		\n\t"\
		"vmovaps	%%ymm8 ,0x20(%%rcx)				\n\t		vmovaps		%%ymm10,0x20(%%rbx)		\n\t"\
		:					/* outputs: none */\
		: [__i0] "m" (Xi0)	/* All iputs from memory addresses here */\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__i3] "m" (Xi3)\
		 ,[__i4] "m" (Xi4)\
		 ,[__i5] "m" (Xi5)\
		 ,[__i6] "m" (Xi6)\
		 ,[__i7] "m" (Xi7)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__o3] "m" (Xo3)\
		 ,[__o4] "m" (Xo4)\
		 ,[__o5] "m" (Xo5)\
		 ,[__o6] "m" (Xo6)\
		 ,[__o7] "m" (Xo7)\
		 ,[__isrt2] "m" (Xisrt2)\
		 ,[__two] "m" (Xtwo)\
		: "cc","memory","rax","rbx","rcx","rdx","rdi","r9","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

	// AVX2 Opcount: 84 vec MEM [30 implicit], 31 ADD/SUB, 50 MUL, 36 FMA, i.e. trade 36 ADD+MUL for 36 FMA (plus one more ADD to generate SQRT2 from ISRT2).
	#define SSE2_RADIX8_DIT_TWIDDLE_OOP(Xi0,Xi1,Xi2,Xi3,Xi4,Xi5,Xi6,Xi7, Xo0,Xo1,Xo2,Xo3,Xo4,Xo5,Xo6,Xo7 ,Xc1,Xs1,Xc2,Xs2,Xc3,Xs3,Xc4,Xs4,Xc5,Xs5,Xc6,Xs6,Xc7,Xs7)\
	{\
	__asm__ volatile (																		/* Blocks 2/3 use separate register subset, can be done overlapped with 0/1: */\
	/* Block 0/1 has just one twiddle-CMUL: */											"	movq		%[i2],%%rcx				\n\t	movq		%[i3],%%rdx				\n\t"\
	"movq		%[i0],%%rax				\n\t												movq		%[c2],%%r10				\n\t	movq		%[c3],%%r12				\n\t"\
	"movq		%[i1],%%rbx				\n\t												movq		%[s2],%%r11				\n\t	movq		%[s3],%%r13				\n\t"\
	"movq		%[c1],%%rdi	\n\t/* [rdi,rsi] -> [c,s] components of each sincos pair, */	vmovaps		(%%rcx),%%ymm8 			\n\t	vmovaps		0x20(%%rcx),%%ymm9 		\n\t"\
	"movq		%[s1],%%rsi	\n\t/* (not truly a pair here in terms of rel-addresses). */	vmovaps	%%ymm9 ,%%ymm10				\n\t	vmovaps		%%ymm8 ,%%ymm11			\n\t"\
	"vmovaps	    (%%rbx),%%ymm4 		\n\t	vmovaps		0x20(%%rbx),%%ymm5 		\n\t	vmovaps		(%%rdx),%%ymm12			\n\t	vmovaps		0x20(%%rdx),%%ymm13		\n\t"\
	"vmovaps	    (%%rax),%%ymm0 		\n\t	vmovaps		0x20(%%rax),%%ymm1 		\n\t	vmovaps	%%ymm13,%%ymm14				\n\t	vmovaps		%%ymm12,%%ymm15			\n\t"\
	"vmovaps	%%ymm5 ,%%ymm6 			\n\t	vmovaps		%%ymm4 ,%%ymm7 			\n\t	vmulpd		(%%r10),%%ymm8 ,%%ymm8 	\n\t	vmulpd		(%%r10),%%ymm9 ,%%ymm9 	\n\t"\
	"vmulpd		 (%%rdi),%%ymm4 ,%%ymm4 \n\t	vmulpd		(%%rdi),%%ymm5 ,%%ymm5 	\n\t	vmulpd		(%%r12),%%ymm12,%%ymm12	\n\t	vmulpd		(%%r12),%%ymm13,%%ymm13	\n\t"\
	"vfmadd231pd (%%rsi),%%ymm6 ,%%ymm4 \n\t   vfnmadd231pd (%%rsi),%%ymm7 ,%%ymm5 	\n\t	vfmadd231pd (%%r11),%%ymm10,%%ymm8 	\n\t   vfnmadd231pd (%%r11),%%ymm11,%%ymm9 	\n\t"\
	"vmovaps	%%ymm0 ,%%ymm2 			\n\t	vmovaps		%%ymm1 ,%%ymm3 			\n\t	vfmadd231pd (%%r13),%%ymm14,%%ymm12	\n\t   vfnmadd231pd (%%r13),%%ymm15,%%ymm13	\n\t"\
	"vaddpd		%%ymm4 ,%%ymm0 ,%%ymm0 	\n\t	vaddpd		%%ymm5 ,%%ymm1 ,%%ymm1 	\n\t	vmovaps		%%ymm8 ,%%ymm10			\n\t	vmovaps		%%ymm9 ,%%ymm11			\n\t"\
	"vsubpd		%%ymm4 ,%%ymm2 ,%%ymm2 	\n\t	vsubpd		%%ymm5 ,%%ymm3 ,%%ymm3 	\n\t	vaddpd		%%ymm12,%%ymm8 ,%%ymm8 	\n\t	vaddpd		%%ymm13,%%ymm9 ,%%ymm9 	\n\t"\
	"vmovaps	%%ymm0 ,    (%%rax)		\n\t	vmovaps		%%ymm1 ,0x20(%%rax)		\n\t	vsubpd		%%ymm12,%%ymm10,%%ymm10	\n\t	vsubpd		%%ymm13,%%ymm11,%%ymm11	\n\t"\
	"vmovaps	%%ymm2 ,    (%%rbx)		\n\t	vmovaps		%%ymm3 ,0x20(%%rbx)		\n\t	vmovaps		%%ymm8 ,    (%%rcx)		\n\t	vmovaps		%%ymm9 ,0x20(%%rcx)		\n\t"\
														/* Now do radix-2 butterfly: */	"	vmovaps		%%ymm10,    (%%rdx)		\n\t	vmovaps		%%ymm11,0x20(%%rdx)		\n\t"\
	/* Blocks 4/5: */																		/* Blocks 6/7 use separate register subset, can be done overlapped with 4/5: */\
	"movq		%[i4],%%rax				\n\t												movq		%[i6],%%rcx				\n\t"\
	"movq		%[i5],%%rbx				\n\t												movq		%[i7],%%rdx				\n\t"\
	"movq		%[c4],%%rdi				\n\t												movq		%[c6],%%r10				\n\t"\
	"movq		%[c5],%%r8 				\n\t												movq		%[c7],%%r12				\n\t"\
	"movq		%[s4],%%rsi				\n\t												movq		%[s6],%%r11				\n\t"\
	"movq		%[s5],%%r9 				\n\t												movq		%[s7],%%r13				\n\t"\
	"vmovaps		(%%rax),%%ymm0 		\n\t	vmovaps		0x20(%%rax),%%ymm1 		\n\t	vmovaps		(%%rcx),%%ymm8 			\n\t	vmovaps		0x20(%%rcx),%%ymm9 		\n\t"\
	"vmovaps	%%ymm1 ,%%ymm2 			\n\t	vmovaps		%%ymm0 ,%%ymm3 			\n\t	vmovaps		%%ymm9 ,%%ymm10			\n\t	vmovaps		%%ymm8 ,%%ymm11			\n\t"\
	"vmovaps		(%%rbx),%%ymm4 		\n\t	vmovaps		0x20(%%rbx),%%ymm5 		\n\t	vmovaps		(%%rdx),%%ymm12			\n\t	vmovaps		0x20(%%rdx),%%ymm13		\n\t"\
	"vmovaps	%%ymm5 ,%%ymm6 			\n\t	vmovaps		%%ymm4 ,%%ymm7 			\n\t	vmovaps		%%ymm13,%%ymm14			\n\t	vmovaps		%%ymm12,%%ymm15			\n\t"\
	"vmulpd		 (%%rdi),%%ymm0 ,%%ymm0 \n\t	vmulpd		 (%%rdi),%%ymm1 ,%%ymm1 \n\t	vmulpd		(%%r10),%%ymm8 ,%%ymm8 	\n\t	vmulpd		 (%%r10),%%ymm9 ,%%ymm9 \n\t"\
	"vmulpd		 (%%r8 ),%%ymm4 ,%%ymm4 \n\t	vmulpd		 (%%r8 ),%%ymm5 ,%%ymm5 \n\t	vmulpd		(%%r12),%%ymm12,%%ymm12	\n\t	vmulpd		 (%%r12),%%ymm13,%%ymm13\n\t"\
	"vfmadd231pd (%%rsi),%%ymm2 ,%%ymm0 \n\t	vfnmadd231pd (%%rsi),%%ymm3 ,%%ymm1 \n\t	vfmadd231pd (%%r11),%%ymm10,%%ymm8 	\n\t	vfnmadd231pd (%%r11),%%ymm11,%%ymm9 \n\t"\
	"vfmadd231pd (%%r9 ),%%ymm6 ,%%ymm4 \n\t	vfnmadd231pd (%%r9 ),%%ymm7 ,%%ymm5 \n\t	vfmadd231pd (%%r13),%%ymm14,%%ymm12	\n\t	vfnmadd231pd (%%r13),%%ymm15,%%ymm13\n\t"\
	/* Now do radix-2 butterfly: */\
	"vmovaps	%%ymm0 ,%%ymm2 			\n\t	vmovaps		%%ymm1 ,%%ymm3 			\n\t	vmovaps		%%ymm8 ,%%ymm10			\n\t	vmovaps		%%ymm9 ,%%ymm11			\n\t"\
	"vaddpd		%%ymm4 ,%%ymm0 ,%%ymm0 	\n\t	vaddpd		%%ymm5 ,%%ymm1 ,%%ymm1 	\n\t	vaddpd		%%ymm12,%%ymm8 ,%%ymm8 	\n\t	vaddpd		%%ymm13,%%ymm9 ,%%ymm9 	\n\t"\
	"vsubpd		%%ymm4 ,%%ymm2 ,%%ymm2 	\n\t	vsubpd		%%ymm5 ,%%ymm3 ,%%ymm3 	\n\t	vsubpd		%%ymm12,%%ymm10,%%ymm10	\n\t	vsubpd		%%ymm13,%%ymm11,%%ymm11	\n\t"\
	/* Reload Block 0-3 outputs into r4-7,c-f, combine to get the 2 length-4 subtransform... */\
	"movq		%[i0],%%rax				\n\t"\
	"movq		%[i1],%%rbx				\n\t"\
	"movq		%[i2],%%rcx				\n\t"\
	"movq		%[i3],%%rdx				\n\t"\
	"vmovaps		(%%rax),%%ymm4 		\n\t	vmovaps		0x20(%%rax),%%ymm5 		\n\t"\
	"vmovaps		(%%rbx),%%ymm6 		\n\t	vmovaps		0x20(%%rbx),%%ymm7 		\n\t"\
	"vmovaps		(%%rcx),%%ymm12		\n\t	vmovaps		0x20(%%rcx),%%ymm13		\n\t"\
	"vmovaps		(%%rdx),%%ymm14		\n\t	vmovaps		0x20(%%rdx),%%ymm15		\n\t"\
	"movq		%[o0],%%rax				\n\t"/* Assumes user stuck a (vec_dbl)2.0 into this output slot prior to macro call. */\
	"vsubpd		%%ymm12,%%ymm4 ,%%ymm4 	\n\t	vsubpd		%%ymm13,%%ymm5 ,%%ymm5 	\n\t"\
	"vsubpd		%%ymm15,%%ymm6 ,%%ymm6 	\n\t	vsubpd		%%ymm14,%%ymm7 ,%%ymm7 	\n\t"\
	"vsubpd		%%ymm8 ,%%ymm0 ,%%ymm0 	\n\t	vsubpd		%%ymm9 ,%%ymm1 ,%%ymm1 	\n\t"\
	"vsubpd		%%ymm11,%%ymm2 ,%%ymm2 	\n\t	vsubpd		%%ymm10,%%ymm3 ,%%ymm3 	\n\t"\
	/* We hope the microcode execution engine inlines the MULs with the above SUBs: */\
	"vmovaps	%%ymm10,(%%rdx) 	\n\t"/* spill ymm14 to make room for 2.0 */"	vmovaps	(%%rax),%%ymm10 \n\t"/* two */\
	"vfmadd132pd %%ymm10,%%ymm4,%%ymm12	\n\t	vfmadd132pd %%ymm10,%%ymm5 ,%%ymm13	\n\t"\
	"vfmadd132pd %%ymm10,%%ymm6,%%ymm15	\n\t	vfmadd132pd %%ymm10,%%ymm7 ,%%ymm14	\n\t"\
	"vfmadd132pd %%ymm10,%%ymm0,%%ymm8 	\n\t	vfmadd132pd %%ymm10,%%ymm1 ,%%ymm9 	\n\t"\
	"vfmadd132pd %%ymm10,%%ymm2,%%ymm11	\n\t	vfmadd132pd (%%rdx),%%ymm3 ,%%ymm10	\n\t"\
	/* In terms of our original scalar-code prototyping macro, the data are: __tr0 = _r[c,f,4,6,8,b,0,2], __ti0 = _r[d,7,5,e,9,3,1,a]; */\
	/* Now combine the two half-transforms: */\
	/* Need r2/3+- a/b combos for the *ISRT2 preceding the output 4-7 radix-2 butterflies, so start them first: */\
	"vsubpd		%%ymm3 ,%%ymm11,%%ymm11	\n\t	vsubpd		%%ymm10,%%ymm2 ,%%ymm2 	\n\t"\
	"vsubpd		%%ymm8 ,%%ymm12,%%ymm12	\n\t	vsubpd		%%ymm9 ,%%ymm13,%%ymm13	\n\t"\
	"vsubpd		%%ymm1 ,%%ymm4 ,%%ymm4 	\n\t	vsubpd		%%ymm0 ,%%ymm5 ,%%ymm5 	\n\t"\
	"vmovaps	%%ymm0 ,(%%rdx) 	\n\t"/* spill ymm14 to make room for 2.0 */"	vmovaps	(%%rax),%%ymm0  \n\t"/* two */\
	"vfmadd132pd %%ymm0,%%ymm11,%%ymm3 	\n\t	vfmadd132pd %%ymm0 ,%%ymm2 ,%%ymm10	\n\t"\
	"vfmadd132pd %%ymm0,%%ymm12,%%ymm8 	\n\t	vfmadd132pd %%ymm0 ,%%ymm13,%%ymm9 	\n\t"\
	"vfmadd132pd %%ymm0,%%ymm4 ,%%ymm1 	\n\t	vfmadd132pd (%%rdx),%%ymm5 ,%%ymm0 	\n\t"\
	/*movq		%[o0],%%rax		[o0] already in rax */	\
	"movq		%[o1],%%rbx				\n\t"\
	"movq		%[o2],%%rcx				\n\t"\
	"movq		%[o3],%%rdx				\n\t"\
	"vmovaps	%%ymm12,    (%%rbx)		\n\t	vmovaps		%%ymm13,0x20(%%rbx)		\n\t"/* __Br1 = _rc;	__Bi1 = _rd; */\
	/* Use that _rc,d free to stick 2.0 into _rc and that [c4] in rdi to load ISRT2 from c4-1 into _rd: */\
	"vmovaps	-0x20(%%rdi),%%ymm12	\n\t	vaddpd	%%ymm12,%%ymm12,%%ymm13		\n\t"/* ymm12 = ISRT2;	ymm13 = SQRT2; */\
	"vmovaps	%%ymm4 ,    (%%rdx)		\n\t	vmovaps		%%ymm0 ,0x20(%%rdx)		\n\t"/* __Br3 = _r4;	__Bi3 = _r0; */\
	"vmovaps	%%ymm8 ,    (%%rax)		\n\t	vmovaps		%%ymm9 ,0x20(%%rax)		\n\t"/* __Br0 = _r8;	__Bi0 = _r9; */\
	"vmovaps	%%ymm1 ,    (%%rcx)		\n\t	vmovaps		%%ymm5 ,0x20(%%rcx)		\n\t"/* __Br2 = _r1;	__Bi2 = _r5; */\
	"vfnmadd231pd %%ymm12,%%ymm3,%%ymm15\n\t	vfnmadd231pd %%ymm12,%%ymm11,%%ymm7 \n\t"\
	"vfnmadd231pd %%ymm12,%%ymm2,%%ymm6	\n\t	vfnmadd231pd %%ymm12,%%ymm10,%%ymm14\n\t"\
	" vfmadd132pd %%ymm13,%%ymm15,%%ymm3\n\t	 vfmadd132pd %%ymm13,%%ymm7 ,%%ymm11\n\t"\
	" vfmadd132pd %%ymm13,%%ymm6 ,%%ymm2\n\t	 vfmadd132pd %%ymm13,%%ymm14,%%ymm10\n\t"\
	"movq		%[o4],%%rax				\n\t"\
	"movq		%[o5],%%rbx				\n\t"\
	"movq		%[o6],%%rcx				\n\t"\
	"movq		%[o7],%%rdx				\n\t"\
	"vmovaps	%%ymm3 ,    (%%rax)		\n\t	vmovaps		%%ymm7 ,0x20(%%rax)		\n\t"/* __Br4 = _r3;	__Bi4 = _r7; */\
	"vmovaps	%%ymm15,    (%%rbx)		\n\t	vmovaps		%%ymm11,0x20(%%rbx)		\n\t"/* __Br5 = _rf;	__Bi5 = _rb; */\
	"vmovaps	%%ymm6 ,    (%%rcx)		\n\t	vmovaps		%%ymm14,0x20(%%rcx)		\n\t"/* __Br6 = _r6;	__Bi6 = _re; */\
	"vmovaps	%%ymm2 ,    (%%rdx)		\n\t	vmovaps		%%ymm10,0x20(%%rdx)		\n\t"/* __Br7 = _r2;	__Bi7 = _ra; */\
		:					/* outputs: none */\
		: [i0] "m" (Xi0)	/* All inputs from memory addresses here */\
		 ,[i1] "m" (Xi1)\
		 ,[i2] "m" (Xi2)\
		 ,[i3] "m" (Xi3)\
		 ,[i4] "m" (Xi4)\
		 ,[i5] "m" (Xi5)\
		 ,[i6] "m" (Xi6)\
		 ,[i7] "m" (Xi7)\
		 ,[o0] "m" (Xo0)\
		 ,[o1] "m" (Xo1)\
		 ,[o2] "m" (Xo2)\
		 ,[o3] "m" (Xo3)\
		 ,[o4] "m" (Xo4)\
		 ,[o5] "m" (Xo5)\
		 ,[o6] "m" (Xo6)\
		 ,[o7] "m" (Xo7)\
		 ,[c1] "m" (Xc1),[s1] "m" (Xs1)\
		 ,[c2] "m" (Xc2),[s2] "m" (Xs2)\
		 ,[c3] "m" (Xc3),[s3] "m" (Xs3)\
		 ,[c4] "m" (Xc4),[s4] "m" (Xs4)\
		 ,[c5] "m" (Xc5),[s5] "m" (Xs5)\
		 ,[c6] "m" (Xc6),[s6] "m" (Xs6)\
		 ,[c7] "m" (Xc7),[s7] "m" (Xs7)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","rdi","r8","r9","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"	/* Clobbered registers */\
	);\
	}

/*** Prefetch odd-index iaddresses in DIF below, even-index oaddresses in SSE2_RADIX16_DIF_TWIDDLE_OOP ***/

	// Based on the SSE2_RADIX16_DIF_NOTWIDDLE macro in radix16_ditN_cy_dif1_gcc64.h, but with completely
	// specifiable 16-output addressing required for usage as the power-of-2 component of a twiddleless
	// radix = [odd*2^n] DFT routine.
	#define SSE2_RADIX16_DIF_0TWIDDLE(Xin0,Xi1,Xi2,Xi3,Xi4, Xisrt2,Xtwo, Xout0,Xout1,Xout2,Xout3,Xout4,Xout5,Xout6,Xout7,Xout8,Xout9,Xouta,Xoutb,Xoutc,Xoutd,Xoute,Xoutf)\
	{\
	__asm__ volatile (\
		"movq	%[__two],%%r15	\n\t"/* two, used for FMA-based double-and-ADD/SUBs */\
	/* Block 0: SSE2_RADIX4_DIF_IN_PLACE(r1 , r17, r9 , r25): */	/* Block 2: SSE2_RADIX4_DIF_IN_PLACE(r5 , r21, r13, r29): */\
	"movq	%[__in0],%%rax	\n\t"/* Note BR of r[abcd]x: b<-->c */	"	leaq	%c[__i2](%%rax),%%r10	\n\t"/* addr += 2*ostride */\
	"leaq	%c[__i4](%%rax),%%rcx	\n\t"/* __in0+  [4*istride] */	"	leaq	%c[__i2](%%rcx),%%r12	\n\t"/* w.r.to to Block 0 */\
	"leaq	%c[__i4](%%rcx),%%rbx	\n\t"/* __in0+2*[4*istride] */	"	leaq	%c[__i2](%%rbx),%%r11	\n\t"/* Note BR of r1[0123]: r11<-->r12 */\
	"leaq	%c[__i4](%%rbx),%%rdx	\n\t"/* __in0+3*[4*istride] */	"	leaq	%c[__i2](%%rdx),%%r13	\n\t"\
		"vmovaps	    (%%rbx),%%ymm0							\n\t	vmovaps	    (%%r11),%%ymm8 	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm1							\n\t	vmovaps	0x20(%%r11),%%ymm9 	\n\t"\
		"vmovaps	    (%%rax),%%ymm2							\n\t	vmovaps	    (%%r10),%%ymm10	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3							\n\t	vmovaps	0x20(%%r10),%%ymm11	\n\t"\
		"vmovaps	    (%%rdx),%%ymm4							\n\t	vmovaps	    (%%r13),%%ymm12	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5							\n\t	vmovaps	0x20(%%r13),%%ymm13	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6							\n\t	vmovaps	    (%%r12),%%ymm14	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7							\n\t	vmovaps	0x20(%%r12),%%ymm15	\n\t"\
		"vsubpd		%%ymm0 ,%%ymm2,%%ymm2						\n\t	vsubpd		%%ymm8 ,%%ymm10,%%ymm10	\n\t"\
		"vsubpd		%%ymm1 ,%%ymm3,%%ymm3						\n\t	vsubpd		%%ymm9 ,%%ymm11,%%ymm11	\n\t"\
		"vsubpd		%%ymm4 ,%%ymm6,%%ymm6						\n\t	vsubpd		%%ymm12,%%ymm14,%%ymm14	\n\t"\
		"vsubpd		%%ymm5 ,%%ymm7,%%ymm7						\n\t	vsubpd		%%ymm13,%%ymm15,%%ymm15	\n\t"\
	"vmovaps	%%ymm13,(%%rax) 	\n\t"/* spill ymm13 to make room for 2.0 */"	vmovaps	(%%r15),%%ymm13	\n\t"/* two */\
	"vfmadd132pd	%%ymm13,%%ymm2,%%ymm0						\n\t	vfmadd132pd	%%ymm13,%%ymm10,%%ymm8 	\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm3,%%ymm1						\n\t	vfmadd132pd	%%ymm13,%%ymm11,%%ymm9 	\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm6,%%ymm4						\n\t	vfmadd132pd	%%ymm13,%%ymm14,%%ymm12	\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm7,%%ymm5						\n\t	vfmadd132pd	(%%rax),%%ymm15,%%ymm13	\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0							\n\t	vsubpd	%%ymm12,%%ymm8 ,%%ymm8 		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1							\n\t	vsubpd	%%ymm13,%%ymm9 ,%%ymm9 		\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2							\n\t	vsubpd	%%ymm15,%%ymm10,%%ymm10		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3							\n\t	vsubpd	%%ymm14,%%ymm11,%%ymm11		\n\t"\
		"vmovaps	%%ymm0,    (%%rbx)							\n\t	vmovaps	%%ymm8 ,    (%%r11)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)							\n\t	vmovaps	%%ymm9 ,0x20(%%r11)	\n\t"\
		"vmovaps	%%ymm2,    (%%rcx)							\n\t	vmovaps	%%ymm10,    (%%r12)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rdx)							\n\t	vmovaps	%%ymm11,0x20(%%r13)	\n\t"\
	"vmovaps	%%ymm14,(%%rax) 	\n\t"/* spill ymm14 to make room for 2.0 */"	vmovaps	(%%r15),%%ymm14	\n\t"/* two */\
	"vfmadd132pd	%%ymm14,%%ymm0,%%ymm4						\n\t	vfmadd132pd	%%ymm14,%%ymm8 ,%%ymm12		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm1,%%ymm5						\n\t	vfmadd132pd	%%ymm14,%%ymm9 ,%%ymm13		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm2,%%ymm7						\n\t	vfmadd132pd	%%ymm14,%%ymm10,%%ymm15		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm3,%%ymm6						\n\t	vfmadd132pd	(%%rax),%%ymm11,%%ymm14		\n\t"\
		"vmovaps	%%ymm4,    (%%rax)							\n\t	vmovaps	%%ymm12,    (%%r10)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)							\n\t	vmovaps	%%ymm13,0x20(%%r10)	\n\t"\
		"vmovaps	%%ymm7,    (%%rdx)							\n\t	vmovaps	%%ymm15,    (%%r13)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rcx)							\n\t	vmovaps	%%ymm14,0x20(%%r12)	\n\t"\
	/* Block 1: SSE2_RADIX4_DIF_IN_PLACE(r3 , r19, r11, r27): */	/* Block 3: SSE2_RADIX4_DIF_IN_PLACE(r7 , r23, r15, r31): */\
		"leaq	%c[__i1](%%rax),%%rax	\n\t"/* addr += 1*ostride */"	leaq	%c[__i2](%%rax),%%r10	\n\t"/* addr += 2*ostride */\
		"leaq	%c[__i1](%%rbx),%%rbx	\n\t"/* w.r.to to Block 0 */"	leaq	%c[__i2](%%rbx),%%r11	\n\t"/* w.r.to to Block 1 */\
		"leaq	%c[__i1](%%rcx),%%rcx							\n\t	leaq	%c[__i2](%%rcx),%%r12	\n\t"\
		"leaq	%c[__i1](%%rdx),%%rdx							\n\t	leaq	%c[__i2](%%rdx),%%r13	\n\t"\
		"vmovaps	    (%%rbx),%%ymm0							\n\t	vmovaps	    (%%r11),%%ymm8 	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm1							\n\t	vmovaps	0x20(%%r11),%%ymm9 	\n\t"\
		"vmovaps	    (%%rax),%%ymm2							\n\t	vmovaps	    (%%r10),%%ymm10	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3							\n\t	vmovaps	0x20(%%r10),%%ymm11	\n\t"\
		"vmovaps	    (%%rdx),%%ymm4							\n\t	vmovaps	    (%%r13),%%ymm12	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5							\n\t	vmovaps	0x20(%%r13),%%ymm13	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6							\n\t	vmovaps	    (%%r12),%%ymm14	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7							\n\t	vmovaps	0x20(%%r12),%%ymm15	\n\t"\
		"vsubpd		%%ymm0 ,%%ymm2,%%ymm2						\n\t	vsubpd		%%ymm8 ,%%ymm10,%%ymm10	\n\t"\
		"vsubpd		%%ymm1 ,%%ymm3,%%ymm3						\n\t	vsubpd		%%ymm9 ,%%ymm11,%%ymm11	\n\t"\
		"vsubpd		%%ymm4 ,%%ymm6,%%ymm6						\n\t	vsubpd		%%ymm12,%%ymm14,%%ymm14	\n\t"\
		"vsubpd		%%ymm5 ,%%ymm7,%%ymm7						\n\t	vsubpd		%%ymm13,%%ymm15,%%ymm15	\n\t"\
	"vmovaps	%%ymm13,(%%rax) 	\n\t"/* spill ymm13 to make room for 2.0 */"	vmovaps	(%%r15),%%ymm13	\n\t"/* two */\
	"vfmadd132pd	%%ymm13,%%ymm2,%%ymm0						\n\t	vfmadd132pd	%%ymm13,%%ymm10,%%ymm8 	\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm3,%%ymm1						\n\t	vfmadd132pd	%%ymm13,%%ymm11,%%ymm9 	\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm6,%%ymm4						\n\t	vfmadd132pd	%%ymm13,%%ymm14,%%ymm12	\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm7,%%ymm5						\n\t	vfmadd132pd	(%%rax),%%ymm15,%%ymm13	\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0							\n\t	vsubpd	%%ymm12,%%ymm8 ,%%ymm8 		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1							\n\t	vsubpd	%%ymm13,%%ymm9 ,%%ymm9 		\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2							\n\t	vsubpd	%%ymm15,%%ymm10,%%ymm10		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3							\n\t	vsubpd	%%ymm14,%%ymm11,%%ymm11		\n\t"\
		"vmovaps	%%ymm0,    (%%rbx)							\n\t	vmovaps	%%ymm8 ,    (%%r11)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)							\n\t	vmovaps	%%ymm9 ,0x20(%%r11)	\n\t"\
		"vmovaps	%%ymm2,    (%%rcx)							\n\t	vmovaps	%%ymm10,    (%%r12)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rdx)							\n\t	vmovaps	%%ymm11,0x20(%%r13)	\n\t"\
	"vmovaps	%%ymm14,(%%rax) 	\n\t"/* spill ymm14 to make room for 2.0 */"	vmovaps	(%%r15),%%ymm14	\n\t"/* two */\
	"vfmadd132pd	%%ymm14,%%ymm0,%%ymm4						\n\t	vfmadd132pd	%%ymm14,%%ymm8 ,%%ymm12		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm1,%%ymm5						\n\t	vfmadd132pd	%%ymm14,%%ymm9 ,%%ymm13		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm2,%%ymm7						\n\t	vfmadd132pd	%%ymm14,%%ymm10,%%ymm15		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm3,%%ymm6						\n\t	vfmadd132pd	(%%rax),%%ymm11,%%ymm14		\n\t"\
		"vmovaps	%%ymm4,    (%%rax)							\n\t	vmovaps	%%ymm12,    (%%r10)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)							\n\t	vmovaps	%%ymm13,0x20(%%r10)	\n\t"\
		"vmovaps	%%ymm7,    (%%rdx)							\n\t	vmovaps	%%ymm15,    (%%r13)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rcx)							\n\t	vmovaps	%%ymm14,0x20(%%r12)	\n\t"\
	/******************************************************************************/\
	/*** Now do 4 DFTs with internal twiddles on the 4*stride - separated data. ***/\
	/*** Order 0,2,1,3 allows incr-only of rsi-datum from 1 block to the next: ****/\
	/******************************************************************************/\
	/* Block 0: r0-3 */												/* Block 1: r8-b */\
		"movq	%[__in0],%%rsi	\n\t	leaq %c[__i4](%%rsi),%%r8 \n\t leaq %c[__i4](%%r8 ),%%r8 \n\t"/* __in0+8*ostride */\
		"movq	%[__out0],%%rax									\n\t	movq	%[__out4],%%r10		\n\t"\
		"movq	%[__out1],%%rbx									\n\t	movq	%[__out5],%%r11		\n\t"\
		"movq	%[__out2],%%rcx									\n\t	movq	%[__out6],%%r12		\n\t"\
		"movq	%[__out3],%%rdx									\n\t	movq	%[__out7],%%r13		\n\t"\
	/* Need separate address for Im parts of outputs due to literal-offsets below: */\
		"leaq	0x20(%%rsi),%%rdi								\n\t	leaq	0x20(%%r8 ),%%r9 	\n\t"\
		"vmovaps	        (%%rsi),%%ymm0						\n\t	vmovaps	        (%%r8 ),%%ymm8 	\n\t"/* ar */\
		"vmovaps	        (%%rdi),%%ymm1						\n\t	vmovaps	        (%%r9 ),%%ymm9 	\n\t"/* ai */\
		"vmovaps	%c[__i2](%%rsi),%%ymm2						\n\t	vmovaps	%c[__i2](%%r8 ),%%ymm10	\n\t"/* br */\
		"vmovaps	%c[__i2](%%rdi),%%ymm3						\n\t	vmovaps	%c[__i2](%%r9 ),%%ymm11	\n\t"/* bi */\
		"vmovaps	%c[__i1](%%rsi),%%ymm4						\n\t	vmovaps	%c[__i1](%%r8 ),%%ymm12	\n\t"/* cr */\
		"vmovaps	%c[__i1](%%rdi),%%ymm5						\n\t	vmovaps	%c[__i1](%%r9 ),%%ymm13	\n\t"/* ci */\
		"vmovaps	%c[__i3](%%rsi),%%ymm6						\n\t	vmovaps	%c[__i3](%%r8 ),%%ymm14	\n\t"/* dr */\
		"vmovaps	%c[__i3](%%rdi),%%ymm7						\n\t	vmovaps	%c[__i3](%%r9 ),%%ymm15	\n\t"/* di */\
		"																movq	%[__isrt2],%%r9 	\n\t"\
		"vsubpd		%%ymm2 ,%%ymm0,%%ymm0						\n\t	vsubpd		%%ymm11,%%ymm8 ,%%ymm8 	\n\t"/* ar-bi */\
		"vsubpd		%%ymm3 ,%%ymm1,%%ymm1						\n\t	vsubpd		%%ymm10,%%ymm9 ,%%ymm9 	\n\t"/* ai-br */\
		"vsubpd		%%ymm6 ,%%ymm4,%%ymm4						\n\t	vsubpd		%%ymm13,%%ymm12,%%ymm12	\n\t"/* cr-ci */\
		"vsubpd		%%ymm7 ,%%ymm5,%%ymm5						\n\t	vsubpd		%%ymm14,%%ymm15,%%ymm15	\n\t"/* di-dr */\
	"vmovaps	%%ymm14,(%%rax) 	\n\t"/* spill ymm14 to make room for 2.0 */"	vmovaps	(%%r15),%%ymm14	\n\t"/* two */\
	"vfmadd132pd	%%ymm14,%%ymm0,%%ymm2						\n\t	vfmadd132pd	%%ymm14,%%ymm8 ,%%ymm11	\n\t"/* ar+bi */\
	"vfmadd132pd	%%ymm14,%%ymm1,%%ymm3						\n\t	vfmadd132pd	%%ymm14,%%ymm9 ,%%ymm10	\n\t"/* ai+br */\
	"vfmadd132pd	%%ymm14,%%ymm4,%%ymm6						\n\t	vfmadd132pd	%%ymm14,%%ymm12,%%ymm13	\n\t"/* cr+ci */\
	"vfmadd132pd	%%ymm14,%%ymm5,%%ymm7						\n\t	vfmadd132pd	(%%rax),%%ymm15,%%ymm14	\n\t"/* di+dr */\
		"																	vsubpd	%%ymm14,%%ymm12,%%ymm12		\n\t"\
		"																	vsubpd	%%ymm15,%%ymm13,%%ymm13		\n\t"\
		"vsubpd		%%ymm6,%%ymm2,%%ymm2						\n\t	vfmadd132pd	(%%r15),%%ymm12,%%ymm14		\n\t"\
		"vsubpd		%%ymm7,%%ymm3,%%ymm3						\n\t	vfmadd132pd	(%%r15),%%ymm13,%%ymm15		\n\t"\
		"vsubpd		%%ymm5,%%ymm0,%%ymm0						\n\t	vfnmadd231pd	(%%r9 ),%%ymm12,%%ymm8 	\n\t"/* x = x - y.isrt2 */\
		"vsubpd		%%ymm4,%%ymm1,%%ymm1						\n\t	vfnmadd231pd	(%%r9 ),%%ymm13,%%ymm10	\n\t"\
		"vmovaps	%%ymm2,    (%%rbx)							\n\t	vfnmadd231pd	(%%r9 ),%%ymm14,%%ymm9 	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rbx)							\n\t	vfnmadd231pd	(%%r9 ),%%ymm15,%%ymm11	\n\t"\
		"vmovaps	%%ymm0,    (%%rcx)							\n\t	vmovaps	%%ymm8 ,    (%%r11)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rdx)							\n\t	vmovaps	%%ymm10,0x20(%%r11)	\n\t"\
	"vfmadd132pd	(%%r15),%%ymm2,%%ymm6						\n\t	vmovaps	%%ymm9 ,0x20(%%r13)	\n\t"\
	"vfmadd132pd	(%%r15),%%ymm3,%%ymm7						\n\t	vmovaps	%%ymm11,    (%%r12)	\n\t"\
	"vfmadd132pd	(%%r15),%%ymm0,%%ymm5						\n\t	vfmadd132pd	-0x20(%%r9),%%ymm8 ,%%ymm12	\n\t"/* y = x + y.sqrt2 = x + y.isrt2 */\
	"vfmadd132pd	(%%r15),%%ymm1,%%ymm4						\n\t	vfmadd132pd	-0x20(%%r9),%%ymm10,%%ymm13	\n\t"\
		"vmovaps	%%ymm6,    (%%rax)							\n\t	vfmadd132pd	-0x20(%%r9),%%ymm11,%%ymm15	\n\t"\
		"vmovaps	%%ymm7,0x20(%%rax)							\n\t	vfmadd132pd	-0x20(%%r9),%%ymm9 ,%%ymm14	\n\t"\
		"vmovaps	%%ymm5,    (%%rdx)							\n\t	vmovaps	%%ymm12,    (%%r10)	\n\t"\
		"vmovaps	%%ymm4,0x20(%%rcx)							\n\t	vmovaps	%%ymm13,0x20(%%r10)	\n\t"\
		"																vmovaps	%%ymm15,    (%%r13)	\n\t"\
		"																vmovaps	%%ymm14,0x20(%%r12)	\n\t"\
	/* Block 2: */													/* Block 3: */\
		"leaq	%c[__i4](%%rsi),%%rsi	\n\t"/* __in0+4*ostride */	"	leaq	%c[__i4](%%r8 ),%%r8 	\n\t"/* __in0+c*ostride */\
		"movq	%[__out8],%%rax									\n\t	movq	%[__outc],%%r10		\n\t"\
		"movq	%[__out9],%%rbx									\n\t	movq	%[__outd],%%r11		\n\t"\
		"movq	%[__outa],%%rcx									\n\t	movq	%[__oute],%%r12		\n\t"\
		"movq	%[__outb],%%rdx									\n\t	movq	%[__outf],%%r13		\n\t"\
		"leaq	0x20(%%rsi),%%rdi								\n\t	leaq	0x20(%%r8 ),%%r9 	\n\t"\
		"vmovaps	%c[__i1](%%rsi),%%ymm4						\n\t	vmovaps	%c[__i1](%%r8 ),%%ymm12	\n\t"\
		"vmovaps	%c[__i3](%%rsi),%%ymm6						\n\t	vmovaps	%c[__i3](%%r8 ),%%ymm14	\n\t"\
		"vmovaps	%c[__i1](%%rdi),%%ymm5						\n\t	vmovaps	%c[__i1](%%r9 ),%%ymm13	\n\t"\
		"vmovaps	%c[__i3](%%rdi),%%ymm7						\n\t	vmovaps	%c[__i3](%%r9 ),%%ymm15	\n\t"\
		"movq	%[__isrt2],%%rdi	\n\t	addq	$0x20,%%rdi	\n\t"/* cc0, from isrt2 [rdi,rsi shared by both cols] */\
		"vmovaps	%%ymm4,%%ymm0								\n\t	vmovaps	%%ymm12,%%ymm8 		\n\t"\
	/*	"vmovaps	%%ymm6,%%ymm2								\n\t	vmovaps	%%ymm14,%%ymm10		\n\t"*/\
		"vmovaps	(%%rdi),%%ymm2								\n\t	vmovaps	0x20(%%rdi),%%ymm10	\n\t"/* Instead use these to store [c,s] */\
		"vmovaps	%%ymm5,%%ymm1								\n\t	vmovaps	%%ymm13,%%ymm9 		\n\t"\
		"vmovaps	%%ymm7,%%ymm3								\n\t	vmovaps	%%ymm15,%%ymm11		\n\t"\
		"vmulpd		    %%ymm2 ,%%ymm4,%%ymm4					\n\t	vmulpd		    %%ymm10,%%ymm12,%%ymm12	\n\t"\
		"vmulpd		    %%ymm2 ,%%ymm5,%%ymm5					\n\t	vmulpd		    %%ymm10,%%ymm13,%%ymm13	\n\t"\
		"vmulpd		    %%ymm10,%%ymm6,%%ymm6					\n\t	vmulpd		    %%ymm2 ,%%ymm14,%%ymm14	\n\t"\
		"vmulpd		    %%ymm10,%%ymm7,%%ymm7					\n\t	vmulpd		    %%ymm2 ,%%ymm15,%%ymm15	\n\t"\
	"vfnmadd231pd	    %%ymm10,%%ymm1,%%ymm4				\n\t	vfnmadd231pd	    %%ymm2 ,%%ymm9 ,%%ymm12		\n\t"\
	" vfmadd231pd	    %%ymm10,%%ymm0,%%ymm5				\n\t	 vfmadd231pd	    %%ymm2 ,%%ymm8 ,%%ymm13		\n\t"\
	"vfnmadd231pd	    %%ymm2 ,%%ymm3,%%ymm6				\n\t	vfnmadd231pd	    %%ymm10,%%ymm11,%%ymm14		\n\t"\
	" vfmadd231pd %c[__i3](%%rsi),%%ymm2,%%ymm7				\n\t	 vfmadd231pd %c[__i3](%%r8 ),%%ymm10,%%ymm15	\n\t"\
		"vsubpd	%%ymm6,%%ymm4,%%ymm4							\n\t	vsubpd	%%ymm14,%%ymm12,%%ymm12		\n\t"\
		"vsubpd	%%ymm7,%%ymm5,%%ymm5							\n\t	vsubpd	%%ymm15,%%ymm13,%%ymm13		\n\t"\
	"vfmadd132pd	(%%r15),%%ymm4,%%ymm6						\n\t	vfmadd132pd	(%%r15),%%ymm12,%%ymm14		\n\t"\
	"vfmadd132pd	(%%r15),%%ymm5,%%ymm7						\n\t	vfmadd132pd	(%%r15),%%ymm13,%%ymm15		\n\t"\
		"leaq	0x20(%%rsi),%%rdi								\n\t	leaq	0x20(%%r8 ),%%r9 	\n\t"\
		"vmovaps	%c[__i2](%%rsi),%%ymm2						\n\t	vmovaps	%c[__i2](%%r8 ),%%ymm10	\n\t"\
		"vmovaps	%c[__i2](%%rdi),%%ymm3						\n\t	vmovaps	%c[__i2](%%r9 ),%%ymm11	\n\t"\
		"vmovaps	        (%%rsi),%%ymm0						\n\t	vmovaps	        (%%r8 ),%%ymm8 	\n\t"\
		"vmovaps	    0x20(%%rsi),%%ymm1						\n\t	vmovaps	    0x20(%%r8 ),%%ymm9 	\n\t"\
		"vsubpd		  %%ymm3,%%ymm2,%%ymm2						\n\t	vaddpd	%%ymm11,%%ymm10,%%ymm10	\n\t"\
		"vaddpd	%c[__i2](%%rsi),%%ymm3,%%ymm3					\n\t	vsubpd	%c[__i2](%%r8 ),%%ymm11,%%ymm11	\n\t"\
		"movq	%[__isrt2],%%r9 	\n\t"\
	"vfnmadd231pd		 (%%r9),%%ymm2,%%ymm0				\n\t	vfnmadd231pd		 (%%r9),%%ymm10,%%ymm8 	\n\t"/* x = x - y.isrt2 */\
	"vfnmadd231pd		 (%%r9),%%ymm3,%%ymm1				\n\t	vfnmadd231pd		 (%%r9),%%ymm11,%%ymm9 	\n\t"\
	" vfmadd132pd	-0x20(%%r9),%%ymm0,%%ymm2				\n\t	 vfmadd132pd	-0x20(%%r9),%%ymm8 ,%%ymm10	\n\t"/* y = x + y.sqrt2 = x + y.isrt2 */\
	" vfmadd132pd	-0x20(%%r9),%%ymm1,%%ymm3				\n\t	 vfmadd132pd	-0x20(%%r9),%%ymm9 ,%%ymm11	\n\t"\
		"vsubpd	%%ymm6,%%ymm2,%%ymm2							\n\t	vsubpd	%%ymm12,%%ymm8 ,%%ymm8 		\n\t"\
		"vsubpd	%%ymm7,%%ymm3,%%ymm3							\n\t	vsubpd	%%ymm13,%%ymm9 ,%%ymm9 		\n\t"\
		"vsubpd	%%ymm5,%%ymm0,%%ymm0							\n\t	vsubpd	%%ymm15,%%ymm10,%%ymm10	\n\t"\
		"vsubpd	%%ymm4,%%ymm1,%%ymm1							\n\t	vsubpd	%%ymm14,%%ymm11,%%ymm11	\n\t"\
	"vmovaps	%%ymm14,(%%rax) 	\n\t"/* spill ymm14 to make room for 2.0 */"	vmovaps	(%%r15),%%ymm14	\n\t"/* two */\
		"vmovaps	%%ymm2,    (%%rbx)							\n\t	vmovaps	%%ymm8 ,    (%%r11)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rbx)							\n\t	vmovaps	%%ymm9 ,0x20(%%r11)	\n\t"\
		"vmovaps	%%ymm0,    (%%rcx)							\n\t	vmovaps	%%ymm10,    (%%r12)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rdx)							\n\t	vmovaps	%%ymm11,0x20(%%r13)	\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm2,%%ymm6						\n\t	vfmadd132pd	%%ymm14,%%ymm8 ,%%ymm12		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm3,%%ymm7						\n\t	vfmadd132pd	%%ymm14,%%ymm9 ,%%ymm13		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm0,%%ymm5						\n\t	vfmadd132pd	%%ymm14,%%ymm10,%%ymm15		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm1,%%ymm4						\n\t	vfmadd132pd	(%%rax),%%ymm11,%%ymm14		\n\t"\
		"vmovaps	%%ymm6,    (%%rax)							\n\t	vmovaps	%%ymm12,    (%%r10)	\n\t"\
		"vmovaps	%%ymm7,0x20(%%rax)							\n\t	vmovaps	%%ymm13,0x20(%%r10)	\n\t"\
		"vmovaps	%%ymm5,    (%%rdx)							\n\t	vmovaps	%%ymm15,    (%%r13)	\n\t"\
		"vmovaps	%%ymm4,0x20(%%rcx)							\n\t	vmovaps	%%ymm14,0x20(%%r12)	\n\t"\
		:					/* outputs: none */\
		:[__in0] "m" (Xin0)	/* All inputs from memory addresses here */\
		,[__i1] "e" (Xi1)\
		,[__i2] "e" (Xi2)\
		,[__i3] "e" (Xi3)\
		,[__i4] "e" (Xi4)\
		,[__isrt2] "m" (Xisrt2)\
		,[__two] "m" (Xtwo)\
		,[__out0] "m" (Xout0)\
		,[__out1] "m" (Xout1)\
		,[__out2] "m" (Xout2)\
		,[__out3] "m" (Xout3)\
		,[__out4] "m" (Xout4)\
		,[__out5] "m" (Xout5)\
		,[__out6] "m" (Xout6)\
		,[__out7] "m" (Xout7)\
		,[__out8] "m" (Xout8)\
		,[__out9] "m" (Xout9)\
		,[__outa] "m" (Xouta)\
		,[__outb] "m" (Xoutb)\
		,[__outc] "m" (Xoutc)\
		,[__outd] "m" (Xoutd)\
		,[__oute] "m" (Xoute)\
		,[__outf] "m" (Xoutf)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","rdi","r8","r9","r10","r11","r12","r13","r15","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"	/* Clobbered registers */\
	);\
	}

	// Same as above, but with specifiable I-addresses and regularly spaced O-addresses:
	//
	#define SSE2_RADIX16_DIF_0TWIDDLE_B(Xin0,Xi1,Xi2,Xi3,Xi4, Xisrt2,Xtwo, Xout0)\
	{\
	__asm__ volatile (\
		"movq	%[__two],%%r15	\n\t"/* two, used for FMA-based double-and-ADD/SUBs */\
	/* Block 0: SSE2_RADIX4_DIF_IN_PLACE(r1 , r17, r9 , r25): */	/* Block 2: SSE2_RADIX4_DIF_IN_PLACE(r5 , r21, r13, r29): */\
	"movq	%[__in0],%%rax	\n\t"/* Note BR of r[abcd]x: b<-->c */	"	leaq	%c[__i2](%%rax),%%r10	\n\t"/* addr += 2*ostride */\
	"leaq	%c[__i4](%%rax),%%rcx	\n\t"/* __in0+  [4*istride] */	"	leaq	%c[__i2](%%rcx),%%r12	\n\t"/* w.r.to to Block 0 */\
	"leaq	%c[__i4](%%rcx),%%rbx	\n\t"/* __in0+2*[4*istride] */	"	leaq	%c[__i2](%%rbx),%%r11	\n\t"/* Note BR of r1[0123]: r11<-->r12 */\
	"leaq	%c[__i4](%%rbx),%%rdx	\n\t"/* __in0+3*[4*istride] */	"	leaq	%c[__i2](%%rdx),%%r13	\n\t"\
		"vmovaps	    (%%rbx),%%ymm0							\n\t	vmovaps	    (%%r11),%%ymm8 	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm1							\n\t	vmovaps	0x20(%%r11),%%ymm9 	\n\t"\
		"vmovaps	    (%%rax),%%ymm2							\n\t	vmovaps	    (%%r10),%%ymm10	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3							\n\t	vmovaps	0x20(%%r10),%%ymm11	\n\t"\
		"vmovaps	    (%%rdx),%%ymm4							\n\t	vmovaps	    (%%r13),%%ymm12	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5							\n\t	vmovaps	0x20(%%r13),%%ymm13	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6							\n\t	vmovaps	    (%%r12),%%ymm14	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7							\n\t	vmovaps	0x20(%%r12),%%ymm15	\n\t"\
		"vsubpd		%%ymm0 ,%%ymm2,%%ymm2						\n\t	vsubpd		%%ymm8 ,%%ymm10,%%ymm10	\n\t"\
		"vsubpd		%%ymm1 ,%%ymm3,%%ymm3						\n\t	vsubpd		%%ymm9 ,%%ymm11,%%ymm11	\n\t"\
		"vsubpd		%%ymm4 ,%%ymm6,%%ymm6						\n\t	vsubpd		%%ymm12,%%ymm14,%%ymm14	\n\t"\
		"vsubpd		%%ymm5 ,%%ymm7,%%ymm7						\n\t	vsubpd		%%ymm13,%%ymm15,%%ymm15	\n\t"\
	"vmovaps	%%ymm13,(%%rax) 	\n\t"/* spill ymm13 to make room for 2.0 */"	vmovaps	(%%r15),%%ymm13	\n\t"/* two */\
	"vfmadd132pd	%%ymm13,%%ymm2,%%ymm0						\n\t	vfmadd132pd	%%ymm13,%%ymm10,%%ymm8 	\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm3,%%ymm1						\n\t	vfmadd132pd	%%ymm13,%%ymm11,%%ymm9 	\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm6,%%ymm4						\n\t	vfmadd132pd	%%ymm13,%%ymm14,%%ymm12	\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm7,%%ymm5						\n\t	vfmadd132pd	(%%rax),%%ymm15,%%ymm13	\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0							\n\t	vsubpd	%%ymm12,%%ymm8 ,%%ymm8 		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1							\n\t	vsubpd	%%ymm13,%%ymm9 ,%%ymm9 		\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2							\n\t	vsubpd	%%ymm15,%%ymm10,%%ymm10		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3							\n\t	vsubpd	%%ymm14,%%ymm11,%%ymm11		\n\t"\
		"vmovaps	%%ymm0,    (%%rbx)							\n\t	vmovaps	%%ymm8 ,    (%%r11)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)							\n\t	vmovaps	%%ymm9 ,0x20(%%r11)	\n\t"\
		"vmovaps	%%ymm2,    (%%rcx)							\n\t	vmovaps	%%ymm10,    (%%r12)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rdx)							\n\t	vmovaps	%%ymm11,0x20(%%r13)	\n\t"\
	"vmovaps	%%ymm14,(%%rax) 	\n\t"/* spill ymm14 to make room for 2.0 */"	vmovaps	(%%r15),%%ymm14	\n\t"/* two */\
	"vfmadd132pd	%%ymm14,%%ymm0,%%ymm4						\n\t	vfmadd132pd	%%ymm14,%%ymm8 ,%%ymm12		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm1,%%ymm5						\n\t	vfmadd132pd	%%ymm14,%%ymm9 ,%%ymm13		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm2,%%ymm7						\n\t	vfmadd132pd	%%ymm14,%%ymm10,%%ymm15		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm3,%%ymm6						\n\t	vfmadd132pd	(%%rax),%%ymm11,%%ymm14		\n\t"\
		"vmovaps	%%ymm4,    (%%rax)							\n\t	vmovaps	%%ymm12,    (%%r10)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)							\n\t	vmovaps	%%ymm13,0x20(%%r10)	\n\t"\
		"vmovaps	%%ymm7,    (%%rdx)							\n\t	vmovaps	%%ymm15,    (%%r13)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rcx)							\n\t	vmovaps	%%ymm14,0x20(%%r12)	\n\t"\
	/* Block 1: SSE2_RADIX4_DIF_IN_PLACE(r3 , r19, r11, r27): */	/* Block 3: SSE2_RADIX4_DIF_IN_PLACE(r7 , r23, r15, r31): */\
		"leaq	%c[__i1](%%rax),%%rax	\n\t"/* addr += 1*ostride */"	leaq	%c[__i2](%%rax),%%r10	\n\t"/* addr += 2*ostride */\
		"leaq	%c[__i1](%%rbx),%%rbx	\n\t"/* w.r.to to Block 0 */"	leaq	%c[__i2](%%rbx),%%r11	\n\t"/* w.r.to to Block 1 */\
		"leaq	%c[__i1](%%rcx),%%rcx							\n\t	leaq	%c[__i2](%%rcx),%%r12	\n\t"\
		"leaq	%c[__i1](%%rdx),%%rdx							\n\t	leaq	%c[__i2](%%rdx),%%r13	\n\t"\
		"vmovaps	    (%%rbx),%%ymm0							\n\t	vmovaps	    (%%r11),%%ymm8 	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm1							\n\t	vmovaps	0x20(%%r11),%%ymm9 	\n\t"\
		"vmovaps	    (%%rax),%%ymm2							\n\t	vmovaps	    (%%r10),%%ymm10	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3							\n\t	vmovaps	0x20(%%r10),%%ymm11	\n\t"\
		"vmovaps	    (%%rdx),%%ymm4							\n\t	vmovaps	    (%%r13),%%ymm12	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5							\n\t	vmovaps	0x20(%%r13),%%ymm13	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6							\n\t	vmovaps	    (%%r12),%%ymm14	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7							\n\t	vmovaps	0x20(%%r12),%%ymm15	\n\t"\
		"vsubpd		%%ymm0 ,%%ymm2,%%ymm2						\n\t	vsubpd		%%ymm8 ,%%ymm10,%%ymm10	\n\t"\
		"vsubpd		%%ymm1 ,%%ymm3,%%ymm3						\n\t	vsubpd		%%ymm9 ,%%ymm11,%%ymm11	\n\t"\
		"vsubpd		%%ymm4 ,%%ymm6,%%ymm6						\n\t	vsubpd		%%ymm12,%%ymm14,%%ymm14	\n\t"\
		"vsubpd		%%ymm5 ,%%ymm7,%%ymm7						\n\t	vsubpd		%%ymm13,%%ymm15,%%ymm15	\n\t"\
	"vmovaps	%%ymm13,(%%rax) 	\n\t"/* spill ymm13 to make room for 2.0 */"	vmovaps	(%%r15),%%ymm13	\n\t"/* two */\
	"vfmadd132pd	%%ymm13,%%ymm2,%%ymm0						\n\t	vfmadd132pd	%%ymm13,%%ymm10,%%ymm8 	\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm3,%%ymm1						\n\t	vfmadd132pd	%%ymm13,%%ymm11,%%ymm9 	\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm6,%%ymm4						\n\t	vfmadd132pd	%%ymm13,%%ymm14,%%ymm12	\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm7,%%ymm5						\n\t	vfmadd132pd	(%%rax),%%ymm15,%%ymm13	\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0							\n\t	vsubpd	%%ymm12,%%ymm8 ,%%ymm8 		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1							\n\t	vsubpd	%%ymm13,%%ymm9 ,%%ymm9 		\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2							\n\t	vsubpd	%%ymm15,%%ymm10,%%ymm10		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3							\n\t	vsubpd	%%ymm14,%%ymm11,%%ymm11		\n\t"\
		"vmovaps	%%ymm0,    (%%rbx)							\n\t	vmovaps	%%ymm8 ,    (%%r11)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)							\n\t	vmovaps	%%ymm9 ,0x20(%%r11)	\n\t"\
		"vmovaps	%%ymm2,    (%%rcx)							\n\t	vmovaps	%%ymm10,    (%%r12)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rdx)							\n\t	vmovaps	%%ymm11,0x20(%%r13)	\n\t"\
	"vmovaps	%%ymm14,(%%rax) 	\n\t"/* spill ymm14 to make room for 2.0 */"	vmovaps	(%%r15),%%ymm14	\n\t"/* two */\
	"vfmadd132pd	%%ymm14,%%ymm0,%%ymm4						\n\t	vfmadd132pd	%%ymm14,%%ymm8 ,%%ymm12		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm1,%%ymm5						\n\t	vfmadd132pd	%%ymm14,%%ymm9 ,%%ymm13		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm2,%%ymm7						\n\t	vfmadd132pd	%%ymm14,%%ymm10,%%ymm15		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm3,%%ymm6						\n\t	vfmadd132pd	(%%rax),%%ymm11,%%ymm14		\n\t"\
		"vmovaps	%%ymm4,    (%%rax)							\n\t	vmovaps	%%ymm12,    (%%r10)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)							\n\t	vmovaps	%%ymm13,0x20(%%r10)	\n\t"\
		"vmovaps	%%ymm7,    (%%rdx)							\n\t	vmovaps	%%ymm15,    (%%r13)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rcx)							\n\t	vmovaps	%%ymm14,0x20(%%r12)	\n\t"\
	/******************************************************************************/\
	/*** Now do 4 DFTs with internal twiddles on the 4*stride - separated data. ***/\
	/*** Order 0,2,1,3 allows incr-only of rsi-datum from 1 block to the next: ****/\
	/******************************************************************************/\
	/* Block 0: r0-3 */												/* Block 1: r8-b */\
		"movq	%[__in0],%%rsi	\n\t	leaq %c[__i4](%%rsi),%%r8 \n\t leaq %c[__i4](%%r8 ),%%r8 \n\t"/* __in0+8*ostride */\
		"movq	%[__out0],%%rax									\n\t	leaq	0x100(%%rax),%%r10		\n\t"/* out4 */\
		"leaq	0x040(%%rax),%%rbx		/* out1 */				\n\t	leaq	0x040(%%r10),%%r11		\n\t"/* out5 */\
		"leaq	0x080(%%rax),%%rcx		/* out2 */				\n\t	leaq	0x080(%%r10),%%r12		\n\t"/* out6 */\
		"leaq	0x0c0(%%rax),%%rdx		/* out3 */				\n\t	leaq	0x0c0(%%r10),%%r13		\n\t"/* out7 */\
	/* Need separate address for Im parts of outputs due to literal-offsets below: */\
		"leaq	0x20(%%rsi),%%rdi								\n\t	leaq	0x20(%%r8 ),%%r9 	\n\t"\
		"vmovaps	        (%%rsi),%%ymm0						\n\t	vmovaps	        (%%r8 ),%%ymm8 	\n\t"/* ar */\
		"vmovaps	        (%%rdi),%%ymm1						\n\t	vmovaps	        (%%r9 ),%%ymm9 	\n\t"/* ai */\
		"vmovaps	%c[__i2](%%rsi),%%ymm2						\n\t	vmovaps	%c[__i2](%%r8 ),%%ymm10	\n\t"/* br */\
		"vmovaps	%c[__i2](%%rdi),%%ymm3						\n\t	vmovaps	%c[__i2](%%r9 ),%%ymm11	\n\t"/* bi */\
		"vmovaps	%c[__i1](%%rsi),%%ymm4						\n\t	vmovaps	%c[__i1](%%r8 ),%%ymm12	\n\t"/* cr */\
		"vmovaps	%c[__i1](%%rdi),%%ymm5						\n\t	vmovaps	%c[__i1](%%r9 ),%%ymm13	\n\t"/* ci */\
		"vmovaps	%c[__i3](%%rsi),%%ymm6						\n\t	vmovaps	%c[__i3](%%r8 ),%%ymm14	\n\t"/* dr */\
		"vmovaps	%c[__i3](%%rdi),%%ymm7						\n\t	vmovaps	%c[__i3](%%r9 ),%%ymm15	\n\t"/* di */\
		"																movq	%[__isrt2],%%r9 	\n\t"\
		"vsubpd		%%ymm2 ,%%ymm0,%%ymm0						\n\t	vsubpd		%%ymm11,%%ymm8 ,%%ymm8 	\n\t"/* ar-bi */\
		"vsubpd		%%ymm3 ,%%ymm1,%%ymm1						\n\t	vsubpd		%%ymm10,%%ymm9 ,%%ymm9 	\n\t"/* ai-br */\
		"vsubpd		%%ymm6 ,%%ymm4,%%ymm4						\n\t	vsubpd		%%ymm13,%%ymm12,%%ymm12	\n\t"/* cr-ci */\
		"vsubpd		%%ymm7 ,%%ymm5,%%ymm5						\n\t	vsubpd		%%ymm14,%%ymm15,%%ymm15	\n\t"/* di-dr */\
	"vmovaps	%%ymm14,(%%rax) 	\n\t"/* spill ymm14 to make room for 2.0 */"	vmovaps	(%%r15),%%ymm14	\n\t"/* two */\
	"vfmadd132pd	%%ymm14,%%ymm0,%%ymm2						\n\t	vfmadd132pd	%%ymm14,%%ymm8 ,%%ymm11	\n\t"/* ar+bi */\
	"vfmadd132pd	%%ymm14,%%ymm1,%%ymm3						\n\t	vfmadd132pd	%%ymm14,%%ymm9 ,%%ymm10	\n\t"/* ai+br */\
	"vfmadd132pd	%%ymm14,%%ymm4,%%ymm6						\n\t	vfmadd132pd	%%ymm14,%%ymm12,%%ymm13	\n\t"/* cr+ci */\
	"vfmadd132pd	%%ymm14,%%ymm5,%%ymm7						\n\t	vfmadd132pd	(%%rax),%%ymm15,%%ymm14	\n\t"/* di+dr */\
		"																	vsubpd	%%ymm14,%%ymm12,%%ymm12		\n\t"\
		"																	vsubpd	%%ymm15,%%ymm13,%%ymm13		\n\t"\
		"vsubpd		%%ymm6,%%ymm2,%%ymm2						\n\t	vfmadd132pd	(%%r15),%%ymm12,%%ymm14		\n\t"\
		"vsubpd		%%ymm7,%%ymm3,%%ymm3						\n\t	vfmadd132pd	(%%r15),%%ymm13,%%ymm15		\n\t"\
		"vsubpd		%%ymm5,%%ymm0,%%ymm0						\n\t	vfnmadd231pd	(%%r9 ),%%ymm12,%%ymm8 	\n\t"/* x = x - y.isrt2 */\
		"vsubpd		%%ymm4,%%ymm1,%%ymm1						\n\t	vfnmadd231pd	(%%r9 ),%%ymm13,%%ymm10	\n\t"\
		"vmovaps	%%ymm2,    (%%rbx)							\n\t	vfnmadd231pd	(%%r9 ),%%ymm14,%%ymm9 	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rbx)							\n\t	vfnmadd231pd	(%%r9 ),%%ymm15,%%ymm11	\n\t"\
		"vmovaps	%%ymm0,    (%%rcx)							\n\t	vmovaps	%%ymm8 ,    (%%r11)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rdx)							\n\t	vmovaps	%%ymm10,0x20(%%r11)	\n\t"\
	"vfmadd132pd	(%%r15),%%ymm2,%%ymm6						\n\t	vmovaps	%%ymm9 ,0x20(%%r13)	\n\t"\
	"vfmadd132pd	(%%r15),%%ymm3,%%ymm7						\n\t	vmovaps	%%ymm11,    (%%r12)	\n\t"\
	"vfmadd132pd	(%%r15),%%ymm0,%%ymm5						\n\t	vfmadd132pd	-0x20(%%r9),%%ymm8 ,%%ymm12	\n\t"/* y = x + y.sqrt2 = x + y.isrt2 */\
	"vfmadd132pd	(%%r15),%%ymm1,%%ymm4						\n\t	vfmadd132pd	-0x20(%%r9),%%ymm10,%%ymm13	\n\t"\
		"vmovaps	%%ymm6,    (%%rax)							\n\t	vfmadd132pd	-0x20(%%r9),%%ymm11,%%ymm15	\n\t"\
		"vmovaps	%%ymm7,0x20(%%rax)							\n\t	vfmadd132pd	-0x20(%%r9),%%ymm9 ,%%ymm14	\n\t"\
		"vmovaps	%%ymm5,    (%%rdx)							\n\t	vmovaps	%%ymm12,    (%%r10)	\n\t"\
		"vmovaps	%%ymm4,0x20(%%rcx)							\n\t	vmovaps	%%ymm13,0x20(%%r10)	\n\t"\
		"																vmovaps	%%ymm15,    (%%r13)	\n\t"\
		"																vmovaps	%%ymm14,0x20(%%r12)	\n\t"\
	/* Block 2: */													/* Block 3: */\
		"leaq	%c[__i4](%%rsi),%%rsi	\n\t"/* __in0+4*ostride */	"	leaq	%c[__i4](%%r8 ),%%r8 	\n\t"/* __in0+c*ostride */\
		"leaq	0x20(%%rsi),%%rdi								\n\t	leaq	0x20(%%r8 ),%%r9 	\n\t"\
		"addq	$0x200,%%rax		/* out8 */					\n\t	addq	$0x200,%%r10		\n\t"/* outc */\
		"addq	$0x200,%%rbx		/* out9 */					\n\t	addq	$0x200,%%r11		\n\t"/* outd */\
		"addq	$0x200,%%rcx		/* outa */					\n\t	addq	$0x200,%%r12		\n\t"/* oute */\
		"addq	$0x200,%%rdx		/* outb */					\n\t	addq	$0x200,%%r13		\n\t"/* outf */\
		"vmovaps	%c[__i1](%%rsi),%%ymm4						\n\t	vmovaps	%c[__i1](%%r8 ),%%ymm12	\n\t"\
		"vmovaps	%c[__i3](%%rsi),%%ymm6						\n\t	vmovaps	%c[__i3](%%r8 ),%%ymm14	\n\t"\
		"vmovaps	%c[__i1](%%rdi),%%ymm5						\n\t	vmovaps	%c[__i1](%%r9 ),%%ymm13	\n\t"\
		"vmovaps	%c[__i3](%%rdi),%%ymm7						\n\t	vmovaps	%c[__i3](%%r9 ),%%ymm15	\n\t"\
		"movq	%[__isrt2],%%rdi	\n\t	addq	$0x20,%%rdi	\n\t"/* cc0, from isrt2 [rdi,rsi shared by both cols] */\
		"vmovaps	%%ymm4,%%ymm0								\n\t	vmovaps	%%ymm12,%%ymm8 		\n\t"\
	/*	"vmovaps	%%ymm6,%%ymm2								\n\t	vmovaps	%%ymm14,%%ymm10		\n\t"*/\
		"vmovaps	(%%rdi),%%ymm2								\n\t	vmovaps	0x20(%%rdi),%%ymm10	\n\t"/* Instead use these to store [c,s] */\
		"vmovaps	%%ymm5,%%ymm1								\n\t	vmovaps	%%ymm13,%%ymm9 		\n\t"\
		"vmovaps	%%ymm7,%%ymm3								\n\t	vmovaps	%%ymm15,%%ymm11		\n\t"\
		"vmulpd		    %%ymm2 ,%%ymm4,%%ymm4					\n\t	vmulpd		    %%ymm10,%%ymm12,%%ymm12	\n\t"\
		"vmulpd		    %%ymm2 ,%%ymm5,%%ymm5					\n\t	vmulpd		    %%ymm10,%%ymm13,%%ymm13	\n\t"\
		"vmulpd		    %%ymm10,%%ymm6,%%ymm6					\n\t	vmulpd		    %%ymm2 ,%%ymm14,%%ymm14	\n\t"\
		"vmulpd		    %%ymm10,%%ymm7,%%ymm7					\n\t	vmulpd		    %%ymm2 ,%%ymm15,%%ymm15	\n\t"\
	"vfnmadd231pd	    %%ymm10,%%ymm1,%%ymm4				\n\t	vfnmadd231pd	    %%ymm2 ,%%ymm9 ,%%ymm12		\n\t"\
	" vfmadd231pd	    %%ymm10,%%ymm0,%%ymm5				\n\t	 vfmadd231pd	    %%ymm2 ,%%ymm8 ,%%ymm13		\n\t"\
	"vfnmadd231pd	    %%ymm2 ,%%ymm3,%%ymm6				\n\t	vfnmadd231pd	    %%ymm10,%%ymm11,%%ymm14		\n\t"\
	" vfmadd231pd %c[__i3](%%rsi),%%ymm2,%%ymm7				\n\t	 vfmadd231pd %c[__i3](%%r8 ),%%ymm10,%%ymm15	\n\t"\
		"vsubpd	%%ymm6,%%ymm4,%%ymm4							\n\t	vsubpd	%%ymm14,%%ymm12,%%ymm12		\n\t"\
		"vsubpd	%%ymm7,%%ymm5,%%ymm5							\n\t	vsubpd	%%ymm15,%%ymm13,%%ymm13		\n\t"\
	"vfmadd132pd	(%%r15),%%ymm4,%%ymm6						\n\t	vfmadd132pd	(%%r15),%%ymm12,%%ymm14		\n\t"\
	"vfmadd132pd	(%%r15),%%ymm5,%%ymm7						\n\t	vfmadd132pd	(%%r15),%%ymm13,%%ymm15		\n\t"\
		"leaq	0x20(%%rsi),%%rdi								\n\t	leaq	0x20(%%r8 ),%%r9 	\n\t"\
		"vmovaps	%c[__i2](%%rsi),%%ymm2						\n\t	vmovaps	%c[__i2](%%r8 ),%%ymm10	\n\t"\
		"vmovaps	%c[__i2](%%rdi),%%ymm3						\n\t	vmovaps	%c[__i2](%%r9 ),%%ymm11	\n\t"\
		"vmovaps	        (%%rsi),%%ymm0						\n\t	vmovaps	        (%%r8 ),%%ymm8 	\n\t"\
		"vmovaps	    0x20(%%rsi),%%ymm1						\n\t	vmovaps	    0x20(%%r8 ),%%ymm9 	\n\t"\
		"vsubpd		  %%ymm3,%%ymm2,%%ymm2						\n\t	vaddpd	%%ymm11,%%ymm10,%%ymm10	\n\t"\
		"vaddpd	%c[__i2](%%rsi),%%ymm3,%%ymm3					\n\t	vsubpd	%c[__i2](%%r8 ),%%ymm11,%%ymm11	\n\t"\
		"movq	%[__isrt2],%%r9 	\n\t"\
	"vfnmadd231pd		 (%%r9),%%ymm2,%%ymm0				\n\t	vfnmadd231pd		 (%%r9),%%ymm10,%%ymm8 	\n\t"/* x = x - y.isrt2 */\
	"vfnmadd231pd		 (%%r9),%%ymm3,%%ymm1				\n\t	vfnmadd231pd		 (%%r9),%%ymm11,%%ymm9 	\n\t"\
	" vfmadd132pd	-0x20(%%r9),%%ymm0,%%ymm2				\n\t	 vfmadd132pd	-0x20(%%r9),%%ymm8 ,%%ymm10	\n\t"/* y = x + y.sqrt2 = x + y.isrt2 */\
	" vfmadd132pd	-0x20(%%r9),%%ymm1,%%ymm3				\n\t	 vfmadd132pd	-0x20(%%r9),%%ymm9 ,%%ymm11	\n\t"\
		"vsubpd	%%ymm6,%%ymm2,%%ymm2							\n\t	vsubpd	%%ymm12,%%ymm8 ,%%ymm8 		\n\t"\
		"vsubpd	%%ymm7,%%ymm3,%%ymm3							\n\t	vsubpd	%%ymm13,%%ymm9 ,%%ymm9 		\n\t"\
		"vsubpd	%%ymm5,%%ymm0,%%ymm0							\n\t	vsubpd	%%ymm15,%%ymm10,%%ymm10	\n\t"\
		"vsubpd	%%ymm4,%%ymm1,%%ymm1							\n\t	vsubpd	%%ymm14,%%ymm11,%%ymm11	\n\t"\
	"vmovaps	%%ymm14,(%%rax) 	\n\t"/* spill ymm14 to make room for 2.0 */"	vmovaps	(%%r15),%%ymm14	\n\t"/* two */\
		"vmovaps	%%ymm2,    (%%rbx)							\n\t	vmovaps	%%ymm8 ,    (%%r11)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rbx)							\n\t	vmovaps	%%ymm9 ,0x20(%%r11)	\n\t"\
		"vmovaps	%%ymm0,    (%%rcx)							\n\t	vmovaps	%%ymm10,    (%%r12)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rdx)							\n\t	vmovaps	%%ymm11,0x20(%%r13)	\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm2,%%ymm6						\n\t	vfmadd132pd	%%ymm14,%%ymm8 ,%%ymm12		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm3,%%ymm7						\n\t	vfmadd132pd	%%ymm14,%%ymm9 ,%%ymm13		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm0,%%ymm5						\n\t	vfmadd132pd	%%ymm14,%%ymm10,%%ymm15		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm1,%%ymm4						\n\t	vfmadd132pd	(%%rax),%%ymm11,%%ymm14		\n\t"\
		"vmovaps	%%ymm6,    (%%rax)							\n\t	vmovaps	%%ymm12,    (%%r10)	\n\t"\
		"vmovaps	%%ymm7,0x20(%%rax)							\n\t	vmovaps	%%ymm13,0x20(%%r10)	\n\t"\
		"vmovaps	%%ymm5,    (%%rdx)							\n\t	vmovaps	%%ymm15,    (%%r13)	\n\t"\
		"vmovaps	%%ymm4,0x20(%%rcx)							\n\t	vmovaps	%%ymm14,0x20(%%r12)	\n\t"\
		:					/* outputs: none */\
		:[__in0] "m" (Xin0)	/* All inputs from memory addresses here */\
		,[__i1] "e" (Xi1)\
		,[__i2] "e" (Xi2)\
		,[__i3] "e" (Xi3)\
		,[__i4] "e" (Xi4)\
		,[__isrt2] "m" (Xisrt2)\
		,[__two] "m" (Xtwo)\
		,[__out0] "m" (Xout0)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","rdi","r8","r9","r10","r11","r12","r13","r15","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"	/* Clobbered registers */\
	);\
	}

	// Based on the SSE2_RADIX16_DIT_NOTWIDDLE macro in radix16_ditN_cy_dif1_gcc64.h, but with completely
	// specifiable 16-input addressing required for usage as the power-of-2 component of a twiddleless
	// radix = [odd*2^n] DFT routine.
	// We use just a single output base-pointer plus literal ostrides which are [1,2,3,4]-multiples of
	// __01; this allows us to cut GP-register usage, which is absolutely a must for the 32-bit version
	// of the macro, and is a benefit to the 64-bit versions which code-fold to yield 2 side-by-side
	// streams of independently executable instructions, one for data in xmm0-7, the other using xmm8-15.
	//
	#define SSE2_RADIX16_DIT_0TWIDDLE(Xin0,Xin1,Xin2,Xin3,Xin4,Xin5,Xin6,Xin7,Xin8,Xin9,Xina,Xinb,Xinc,Xind,Xine,Xinf, Xisrt2,Xtwo, Xout0,Xo1,Xo2,Xo3,Xo4)\
	{\
	__asm__ volatile (\
		/* Need separate address Im parts of outputs due to literal-offsets below */\
		"movq	%[__two],%%r15	\n\t"/* two, used for FMA-based double-and-ADD/SUBs */\
		"movq	%[__out0],%%rsi									\n\t	leaq	%c[__o4](%%rsi),%%r8 	\n\t"\
		"leaq	0x20(%%rsi),%%rdi								\n\t	leaq	%c[__o4](%%r8 ),%%r8 	\n\t"/* out0+8*ostride */\
		"movq	%[__in0],%%rax									\n\t	leaq	0x20(%%r8 ),%%r9 	\n\t"\
		"movq	%[__in1],%%rbx									\n\t	movq	%[__in8],%%r10			\n\t"\
		"movq	%[__in2],%%rcx									\n\t	movq	%[__in9],%%r11			\n\t"\
		"movq	%[__in3],%%rdx									\n\t	movq	%[__ina],%%r12			\n\t"\
	"prefetcht1	0x100(%%rax)									\n\t	movq	%[__inb],%%r13			\n\t"\
		/* SSE2_RADIX4_DIT_0TWIDDLE_B(r0 ): */							/* SSE2_RADIX4_DIT_0TWIDDLE_B(r16): */\
		"vmovaps	    (%%rax),%%ymm2							\n\t	vmovaps	    (%%r10),%%ymm10	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6							\n\t	vmovaps	    (%%r12),%%ymm14	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3							\n\t	vmovaps	0x20(%%r10),%%ymm11	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7							\n\t	vmovaps	0x20(%%r12),%%ymm15	\n\t"\
		"vmovaps	    (%%rbx),%%ymm0							\n\t	vmovaps	    (%%r11),%%ymm8 	\n\t"\
		"vmovaps	    (%%rdx),%%ymm4							\n\t	vmovaps	    (%%r13),%%ymm12	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm1							\n\t	vmovaps	0x20(%%r11),%%ymm9 	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5							\n\t	vmovaps	0x20(%%r13),%%ymm13	\n\t"\
		"vsubpd	%%ymm0,%%ymm2,%%ymm2							\n\t	vsubpd	%%ymm8 ,%%ymm10,%%ymm10		\n\t"\
		"vsubpd	%%ymm4,%%ymm6,%%ymm6							\n\t	vsubpd	%%ymm12,%%ymm14,%%ymm14		\n\t"\
		"vsubpd	%%ymm1,%%ymm3,%%ymm3							\n\t	vsubpd	%%ymm9 ,%%ymm11,%%ymm11		\n\t"\
		"vsubpd	%%ymm5,%%ymm7,%%ymm7							\n\t	vsubpd	%%ymm13,%%ymm15,%%ymm15		\n\t"\
	"vmovaps	%%ymm13,(%%rax) 	\n\t"/* spill ymm13 to make room for 2.0 */"	vmovaps	(%%r15),%%ymm13	\n\t"/* two */\
	"vfmadd132pd	%%ymm13,%%ymm2,%%ymm0						\n\t	vfmadd132pd	%%ymm13,%%ymm10,%%ymm8 		\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm6,%%ymm4						\n\t	vfmadd132pd	%%ymm13,%%ymm14,%%ymm12		\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm3,%%ymm1						\n\t	vfmadd132pd	%%ymm13,%%ymm11,%%ymm9 		\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm7,%%ymm5						\n\t	vfmadd132pd	(%%rax),%%ymm15,%%ymm13		\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0							\n\t	vsubpd	%%ymm12,%%ymm8 ,%%ymm8 		\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2							\n\t	vsubpd	%%ymm15,%%ymm10,%%ymm10		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1							\n\t	vsubpd	%%ymm13,%%ymm9 ,%%ymm9 		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3							\n\t	vsubpd	%%ymm14,%%ymm11,%%ymm11		\n\t"\
	"prefetcht1	0x100(%%rcx)								\n\t	prefetcht1	0x100(%%r12)	\n\t"\
	"vmovaps	%%ymm14,(%%rax) 	\n\t"/* spill ymm14 to make room for 2.0 */"	vmovaps	(%%r15),%%ymm14	\n\t"/* two */\
		"vmovaps	%%ymm0,%c[__o2](%%rsi)						\n\t	vmovaps	%%ymm8 ,%c[__o2](%%r8 )	\n\t"\
		"vmovaps	%%ymm2,%c[__o3](%%rsi)						\n\t	vmovaps	%%ymm10,%c[__o3](%%r8 )	\n\t"\
		"vmovaps	%%ymm1,%c[__o2](%%rdi)						\n\t	vmovaps	%%ymm9 ,%c[__o2](%%r9 )	\n\t"\
		"vmovaps	%%ymm3,%c[__o1](%%rdi)						\n\t	vmovaps	%%ymm11,%c[__o1](%%r9 )	\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm0,%%ymm4						\n\t	vfmadd132pd	%%ymm14,%%ymm8 ,%%ymm12		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm2,%%ymm7						\n\t	vfmadd132pd	%%ymm14,%%ymm10,%%ymm15		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm1,%%ymm5						\n\t	vfmadd132pd	%%ymm14,%%ymm9 ,%%ymm13		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm3,%%ymm6						\n\t	vfmadd132pd	(%%rax),%%ymm11,%%ymm14		\n\t"\
		"vmovaps	%%ymm4,        (%%rsi)						\n\t	vmovaps	%%ymm12,        (%%r8 )	\n\t"\
		"vmovaps	%%ymm7,%c[__o1](%%rsi)						\n\t	vmovaps	%%ymm15,%c[__o1](%%r8 )	\n\t"\
		"vmovaps	%%ymm5,        (%%rdi)						\n\t	vmovaps	%%ymm13,        (%%r9 )	\n\t"\
		"vmovaps	%%ymm6,%c[__o3](%%rdi)						\n\t	vmovaps	%%ymm14,%c[__o3](%%r9 )	\n\t"\
	"prefetcht1	0x100(%%rax)									\n\tprefetcht1	0x100(%%r10)\n\t"\
		/* SSE2_RADIX4_DIT_0TWIDDLE_B(r8 ): */						/* SSE2_RADIX4_DIT_0TWIDDLE_B(r24): */\
		"leaq %c[__o4](%%rsi),%%rsi	\n\t"/* out0+4*ostride */"	\n\t	leaq	%c[__o4](%%r8 ),%%r8 	\n\t"/* out0+c*ostride */\
		"movq	%[__in4],%%rax									\n\t	movq	%[__inc],%%r10			\n\t"\
		"movq	%[__in5],%%rbx									\n\t	movq	%[__ind],%%r11			\n\t"\
		"movq	%[__in6],%%rcx									\n\t	movq	%[__ine],%%r12			\n\t"\
		"movq	%[__in7],%%rdx									\n\t	movq	%[__inf],%%r13			\n\t"\
		"vmovaps	    (%%rax),%%ymm2							\n\t	vmovaps	    (%%r10),%%ymm10	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6							\n\t	vmovaps	    (%%r12),%%ymm14	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3							\n\t	vmovaps	0x20(%%r10),%%ymm11	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7							\n\t	vmovaps	0x20(%%r12),%%ymm15	\n\t"\
		"vmovaps	    (%%rbx),%%ymm0							\n\t	vmovaps	    (%%r11),%%ymm8 	\n\t"\
		"vmovaps	    (%%rdx),%%ymm4							\n\t	vmovaps	    (%%r13),%%ymm12	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm1							\n\t	vmovaps	0x20(%%r11),%%ymm9 	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5							\n\t	vmovaps	0x20(%%r13),%%ymm13	\n\t"\
		"vsubpd	%%ymm0,%%ymm2,%%ymm2							\n\t	vsubpd	%%ymm8 ,%%ymm10,%%ymm10		\n\t"\
		"vsubpd	%%ymm4,%%ymm6,%%ymm6							\n\t	vsubpd	%%ymm12,%%ymm14,%%ymm14		\n\t"\
		"vsubpd	%%ymm1,%%ymm3,%%ymm3							\n\t	vsubpd	%%ymm9 ,%%ymm11,%%ymm11		\n\t"\
		"vsubpd	%%ymm5,%%ymm7,%%ymm7							\n\t	vsubpd	%%ymm13,%%ymm15,%%ymm15		\n\t"\
	"vmovaps	%%ymm13,(%%rax) 	\n\t"/* spill ymm13 to make room for 2.0 */"	vmovaps	(%%r15),%%ymm13	\n\t"/* two */\
	"vfmadd132pd	%%ymm13,%%ymm2,%%ymm0						\n\t	vfmadd132pd	%%ymm13,%%ymm10,%%ymm8 		\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm6,%%ymm4						\n\t	vfmadd132pd	%%ymm13,%%ymm14,%%ymm12		\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm3,%%ymm1						\n\t	vfmadd132pd	%%ymm13,%%ymm11,%%ymm9 		\n\t"\
	"vfmadd132pd	%%ymm13,%%ymm7,%%ymm5						\n\t	vfmadd132pd	(%%rax),%%ymm15,%%ymm13		\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0							\n\t	vsubpd	%%ymm12,%%ymm8 ,%%ymm8 		\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2							\n\t	vsubpd	%%ymm15,%%ymm10,%%ymm10		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1							\n\t	vsubpd	%%ymm13,%%ymm9 ,%%ymm9 		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3							\n\t	vsubpd	%%ymm14,%%ymm11,%%ymm11		\n\t"\
	"prefetcht1	0x100(%%rcx)								\n\t	prefetcht1	0x100(%%r12)	\n\t"\
		"leaq	0x20(%%rsi),%%rdi								\n\t	leaq	0x20(%%r8 ),%%r9 	\n\t"\
	"vmovaps	%%ymm14,(%%rax) 	\n\t"/* spill ymm14 to make room for 2.0 */"	vmovaps	(%%r15),%%ymm14	\n\t"/* two */\
		"vmovaps	%%ymm0,%c[__o2](%%rsi)						\n\t	vmovaps	%%ymm8 ,%c[__o2](%%r8 )	\n\t"\
		"vmovaps	%%ymm2,%c[__o3](%%rsi)						\n\t	vmovaps	%%ymm10,%c[__o3](%%r8 )	\n\t"\
		"vmovaps	%%ymm1,%c[__o2](%%rdi)						\n\t	vmovaps	%%ymm9 ,%c[__o2](%%r9 )	\n\t"\
		"vmovaps	%%ymm3,%c[__o1](%%rdi)						\n\t	vmovaps	%%ymm11,%c[__o1](%%r9 )	\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm0,%%ymm4						\n\t	vfmadd132pd	%%ymm14,%%ymm8 ,%%ymm12		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm2,%%ymm7						\n\t	vfmadd132pd	%%ymm14,%%ymm10,%%ymm15		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm1,%%ymm5						\n\t	vfmadd132pd	%%ymm14,%%ymm9 ,%%ymm13		\n\t"\
	"vfmadd132pd	%%ymm14,%%ymm3,%%ymm6						\n\t	vfmadd132pd	(%%rax),%%ymm11,%%ymm14		\n\t"\
		"vmovaps	%%ymm4,        (%%rsi)						\n\t	vmovaps	%%ymm12,        (%%r8 )	\n\t"\
		"vmovaps	%%ymm7,%c[__o1](%%rsi)						\n\t	vmovaps	%%ymm15,%c[__o1](%%r8 )	\n\t"\
		"vmovaps	%%ymm5,        (%%rdi)						\n\t	vmovaps	%%ymm13,        (%%r9 )	\n\t"\
		"vmovaps	%%ymm6,%c[__o3](%%rdi)						\n\t	vmovaps	%%ymm14,%c[__o3](%%r9 )	\n\t"\
	"prefetcht1	0x100(%%rax)	\n\t"\
	/******************************************************************************/\
	/*** Now do 4 DFTs with internal twiddles on the 4*stride - separated data: ***/\
	/******************************************************************************/\
		/* Block 0: */												/* Block 2: */\
		"movq	%[__out0],%%rax									\n\t	leaq	%c[__o2](%%rax),%%r10	\n\t"/* All addresses += 2*ostride */\
	"leaq %c[__o4](%%rax),%%rbx \n\t"/* out0+  [4*ostride] */"	\n\t	leaq	%c[__o2](%%rbx),%%r11	\n\t"\
	"leaq %c[__o4](%%rbx),%%rcx \n\t"/* out0+2*[4*ostride] */"	\n\t	leaq	%c[__o2](%%rcx),%%r12	\n\t"\
	"leaq %c[__o4](%%rcx),%%rdx \n\t"/* out0+3*[4*ostride] */"	\n\t	leaq	%c[__o2](%%rdx),%%r13	\n\t"\
		"														\n\t	movq	%[__isrt2],%%rdi	\n\t"\
		"vmovaps	    (%%rax),%%ymm0							\n\t	vmovaps	    (%%r10),%%ymm8 	\n\t"/* ar */\
		"vmovaps	0x20(%%rax),%%ymm1							\n\t	vmovaps	0x20(%%r10),%%ymm9 	\n\t"/* ai */\
		"vmovaps	    (%%rbx),%%ymm2							\n\t	vmovaps	    (%%r11),%%ymm10	\n\t"/* br */\
		"vmovaps	0x20(%%rbx),%%ymm3							\n\t	vmovaps	0x20(%%r11),%%ymm11	\n\t"/* bi */\
		"vmovaps	    (%%rcx),%%ymm4							\n\t	vmovaps	    (%%r12),%%ymm12	\n\t"/* cr */\
		"vmovaps	0x20(%%rcx),%%ymm5							\n\t	vmovaps	0x20(%%r12),%%ymm13	\n\t"/* ci */\
		"vmovaps	    (%%rdx),%%ymm6							\n\t	vmovaps	    (%%r13),%%ymm14	\n\t"/* dr */\
		"vmovaps	0x20(%%rdx),%%ymm7							\n\t	vmovaps	0x20(%%r13),%%ymm15	\n\t"/* di */\
		"vsubpd	    %%ymm2 ,%%ymm0,%%ymm0						\n\t	vsubpd	    %%ymm11,%%ymm8 ,%%ymm8 	\n\t"/* ar-bi */\
		"vsubpd	    %%ymm3 ,%%ymm1,%%ymm1						\n\t	vsubpd	    %%ymm10,%%ymm9 ,%%ymm9 	\n\t"/* ai-br */\
		"vsubpd	    %%ymm6 ,%%ymm4,%%ymm4						\n\t	vsubpd	    %%ymm12,%%ymm13,%%ymm13	\n\t"/* ci-cr */\
		"vsubpd	    %%ymm7 ,%%ymm5,%%ymm5						\n\t	vsubpd	    %%ymm15,%%ymm14,%%ymm14	\n\t"/* dr-di */\
	"vmovaps	%%ymm15,(%%rax) 	\n\t"/* spill ymm15 to make room for 2.0 */"	vmovaps	(%%r15),%%ymm15	\n\t"/* two */\
		"vfmadd132pd	%%ymm15,%%ymm0,%%ymm2					\n\t	vfmadd132pd	%%ymm15,%%ymm8 ,%%ymm11	\n\t"/* ar+bi */\
		"vfmadd132pd	%%ymm15,%%ymm1,%%ymm3					\n\t	vfmadd132pd	%%ymm15,%%ymm9 ,%%ymm10	\n\t"/* ai+br */\
		"vfmadd132pd	%%ymm15,%%ymm4,%%ymm6					\n\t	vfmadd132pd	%%ymm15,%%ymm13,%%ymm12	\n\t"/* ci+cr */\
		"vfmadd132pd	%%ymm15,%%ymm5,%%ymm7					\n\t	vfmadd132pd	(%%rax),%%ymm14,%%ymm15	\n\t"/* dr+di */\
		"														\n\t		vsubpd	%%ymm14,%%ymm12,%%ymm12		\n\t"\
		"														\n\t		vsubpd	%%ymm15,%%ymm13,%%ymm13		\n\t"\
		"vsubpd	%%ymm5,%%ymm0,%%ymm0							\n\t	vfmadd132pd	(%%r15),%%ymm12,%%ymm14		\n\t"\
		"vsubpd	%%ymm4,%%ymm1,%%ymm1							\n\t	vfmadd132pd	(%%r15),%%ymm13,%%ymm15		\n\t"\
		"vsubpd	%%ymm6,%%ymm2,%%ymm2							\n\t	vfnmadd231pd	(%%rdi),%%ymm12,%%ymm11		\n\t"/* x = x - y.isrt2 */\
		"vsubpd	%%ymm7,%%ymm3,%%ymm3							\n\t	vfnmadd231pd	(%%rdi),%%ymm13,%%ymm9 		\n\t"\
		"vmovaps	%%ymm0,    (%%rdx)							\n\t	vfnmadd231pd	(%%rdi),%%ymm14,%%ymm10		\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)							\n\t	vfnmadd231pd	(%%rdi),%%ymm15,%%ymm8 		\n\t"\
		"vmovaps	%%ymm2,    (%%rcx)							\n\t		vmovaps	%%ymm11,    (%%r12)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rcx)							\n\t		vmovaps	%%ymm9 ,0x20(%%r12)	\n\t"\
	"vfmadd132pd	(%%r15),%%ymm0,%%ymm5						\n\t		vmovaps	%%ymm10,0x20(%%r11)	\n\t"\
	"vfmadd132pd	(%%r15),%%ymm1,%%ymm4						\n\t		vmovaps	%%ymm8 ,    (%%r13)	\n\t"\
	"vfmadd132pd	(%%r15),%%ymm2,%%ymm6						\n\t	vfmadd132pd	-0x20(%%rdi),%%ymm11,%%ymm12	\n\t"/* y = x + y.sqrt2 = x + y.isrt2 */\
	"vfmadd132pd	(%%r15),%%ymm3,%%ymm7						\n\t	vfmadd132pd	-0x20(%%rdi),%%ymm9 ,%%ymm13	\n\t"\
		"vmovaps	%%ymm5,    (%%rbx)							\n\t	vfmadd132pd	-0x20(%%rdi),%%ymm10,%%ymm14	\n\t"\
		"vmovaps	%%ymm4,0x20(%%rdx)							\n\t	vfmadd132pd	-0x20(%%rdi),%%ymm8 ,%%ymm15	\n\t"\
		"vmovaps	%%ymm6,    (%%rax)							\n\t		vmovaps	%%ymm12,    (%%r10)	\n\t"\
		"vmovaps	%%ymm7,0x20(%%rax)							\n\t		vmovaps	%%ymm13,0x20(%%r10)	\n\t"\
		"														\n\t		vmovaps	%%ymm14,0x20(%%r13)	\n\t"\
		"														\n\t		vmovaps	%%ymm15,    (%%r11)	\n\t"\
		/* Block 1: */												/* Block 3: */\
	"leaq %c[__o1](%%rax),%%rax\n\t"/* addr += 1*ostride */"\n\t	leaq	%c[__o2](%%rax),%%r10	\n\t"/* All addresses += 1*ostride */\
	"leaq %c[__o1](%%rbx),%%rbx\n\t"/* relative to Block 0 */"\n\t	leaq	%c[__o2](%%rbx),%%r11	\n\t"/* relative to Block 1 */\
	"leaq %c[__o1](%%rcx),%%rcx								\n\t	leaq	%c[__o2](%%rcx),%%r12	\n\t"\
	"leaq %c[__o1](%%rdx),%%rdx								\n\t	leaq	%c[__o2](%%rdx),%%r13	\n\t"\
		"leaq	0x20(%%rdi),%%rsi	\n\t"/* cc0, from isrt2 [rdi,rsi shared by both cols] */\
		"vmovaps	    (%%rdx),%%ymm0							\n\t	vmovaps	    (%%r13),%%ymm8 	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm1							\n\t	vmovaps	0x20(%%r13),%%ymm9 	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4							\n\t	vmovaps	    (%%r12),%%ymm12	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5							\n\t	vmovaps	0x20(%%r12),%%ymm13	\n\t"\
		"vmovaps	    %%ymm0 ,%%ymm2							\n\t	vmovaps	    %%ymm8 ,%%ymm10	\n\t"\
		"vmovaps	    %%ymm1 ,%%ymm3							\n\t	vmovaps	    %%ymm9 ,%%ymm11	\n\t"\
		"vmovaps	    %%ymm4 ,%%ymm6							\n\t	vmovaps	    %%ymm12,%%ymm14	\n\t"\
	/*	"vmovaps	    %%ymm5 ,%%ymm7							\n\t	vmovaps	    %%ymm13,%%ymm15	\n\t"*/\
		"vmovaps	0x20(%%rsi),%%ymm7							\n\t	vmovaps	    (%%rsi),%%ymm15	\n\t"/* Instead use these to store [c,s] */\
		"vmulpd		    %%ymm7 ,%%ymm0,%%ymm0					\n\t	vmulpd		    %%ymm15,%%ymm8 ,%%ymm8 	\n\t"\
		"vmulpd		    %%ymm7 ,%%ymm1,%%ymm1					\n\t	vmulpd		    %%ymm15,%%ymm9 ,%%ymm9 	\n\t"\
		"vmulpd		    %%ymm15,%%ymm4,%%ymm4					\n\t	vmulpd		    %%ymm7 ,%%ymm12,%%ymm12	\n\t"\
		"vmulpd		    %%ymm15,%%ymm5,%%ymm5					\n\t	vmulpd		    %%ymm7 ,%%ymm13,%%ymm13	\n\t"\
	"vfnmadd231pd	    %%ymm15,%%ymm2,%%ymm1				\n\t	vfnmadd231pd	    %%ymm7 ,%%ymm10,%%ymm9 	\n\t"\
	" vfmadd231pd	    %%ymm15,%%ymm3,%%ymm0				\n\t	 vfmadd231pd	    %%ymm7 ,%%ymm11,%%ymm8 	\n\t"\
	"vfnmadd231pd	    %%ymm7 ,%%ymm6,%%ymm5				\n\t	vfnmadd231pd	    %%ymm15,%%ymm14,%%ymm13	\n\t"\
	" vfmadd231pd	0x20(%%rcx),%%ymm7,%%ymm4				\n\t	 vfmadd231pd	0x20(%%r12),%%ymm15,%%ymm12	\n\t"\
		"vmovaps	%%ymm5,%%ymm7								\n\t	vmovaps	%%ymm13,%%ymm15		\n\t"\
		"vmovaps	%%ymm4,%%ymm6								\n\t	vmovaps	%%ymm12,%%ymm14		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4							\n\t	vaddpd	%%ymm8 ,%%ymm12,%%ymm12		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5							\n\t	vaddpd	%%ymm9 ,%%ymm13,%%ymm13		\n\t"\
		"vsubpd	%%ymm0,%%ymm6,%%ymm6							\n\t	vsubpd	%%ymm8 ,%%ymm14,%%ymm14		\n\t"\
		"vsubpd	%%ymm1,%%ymm7,%%ymm7							\n\t	vsubpd	%%ymm9 ,%%ymm15,%%ymm15		\n\t"\
		"vmovaps	    (%%rbx),%%ymm2							\n\t	vmovaps	    (%%r11),%%ymm10	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3							\n\t	vmovaps	0x20(%%r11),%%ymm11	\n\t"\
		"vmovaps	    (%%rax),%%ymm0							\n\t	vmovaps	    (%%r10),%%ymm8 	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1							\n\t	vmovaps	0x20(%%r10),%%ymm9 	\n\t"\
		"vaddpd	0x20(%%rbx),%%ymm2,%%ymm2						\n\t	vsubpd	0x20(%%r11),%%ymm10,%%ymm10	\n\t"\
		"vsubpd	    (%%rbx),%%ymm3,%%ymm3						\n\t	vaddpd	    (%%r11),%%ymm11,%%ymm11	\n\t"\
	"vfnmadd231pd		 (%%rdi),%%ymm2,%%ymm0				\n\t	vfnmadd231pd		 (%%rdi),%%ymm10,%%ymm8 	\n\t"/* x = x - y.isrt2 */\
	"vfnmadd231pd		 (%%rdi),%%ymm3,%%ymm1				\n\t	vfnmadd231pd		 (%%rdi),%%ymm11,%%ymm9 	\n\t"\
	" vfmadd132pd	-0x20(%%rdi),%%ymm0,%%ymm2				\n\t	 vfmadd132pd	-0x20(%%rdi),%%ymm8 ,%%ymm10	\n\t"/* y = x + y.sqrt2 = x + y.isrt2 */\
	" vfmadd132pd	-0x20(%%rdi),%%ymm1,%%ymm3				\n\t	 vfmadd132pd	-0x20(%%rdi),%%ymm9 ,%%ymm11	\n\t"\
		"vsubpd	%%ymm7,%%ymm0,%%ymm0							\n\t	vsubpd	%%ymm13,%%ymm10,%%ymm10		\n\t"\
		"vsubpd	%%ymm6,%%ymm1,%%ymm1							\n\t	vsubpd	%%ymm12,%%ymm11,%%ymm11		\n\t"\
		"vsubpd	%%ymm4,%%ymm2,%%ymm2							\n\t	vsubpd	%%ymm14,%%ymm8 ,%%ymm8 		\n\t"\
		"vsubpd	%%ymm5,%%ymm3,%%ymm3							\n\t	vsubpd	%%ymm15,%%ymm9 ,%%ymm9 		\n\t"\
		"vmovaps	%%ymm0,    (%%rdx)							\n\t	vmovaps	%%ymm10,    (%%r13)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)							\n\t	vmovaps	%%ymm11,0x20(%%r11)	\n\t"\
		"vmovaps	%%ymm2,    (%%rcx)							\n\t	vmovaps	%%ymm8 ,    (%%r12)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rcx)							\n\t	vmovaps	%%ymm9 ,0x20(%%r12)	\n\t"\
	"vmovaps	%%ymm15,(%%rax) 	\n\t"/* spill ymm15 to make room for 2.0 */"	vmovaps	(%%r15),%%ymm15	\n\t"/* two */\
	"vfmadd132pd	%%ymm15,%%ymm0,%%ymm7						\n\t	vfmadd132pd	%%ymm15,%%ymm10,%%ymm13		\n\t"\
	"vfmadd132pd	%%ymm15,%%ymm1,%%ymm6						\n\t	vfmadd132pd	%%ymm15,%%ymm11,%%ymm12		\n\t"\
	"vfmadd132pd	%%ymm15,%%ymm2,%%ymm4						\n\t	vfmadd132pd	%%ymm15,%%ymm8 ,%%ymm14		\n\t"\
	"vfmadd132pd	%%ymm15,%%ymm3,%%ymm5						\n\t	vfmadd132pd	(%%rax),%%ymm9 ,%%ymm15		\n\t"\
		"vmovaps	%%ymm7,    (%%rbx)							\n\t	vmovaps	%%ymm13,    (%%r11)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rdx)							\n\t	vmovaps	%%ymm12,0x20(%%r13)	\n\t"\
		"vmovaps	%%ymm4,    (%%rax)							\n\t	vmovaps	%%ymm14,    (%%r10)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)							\n\t	vmovaps	%%ymm15,0x20(%%r10)	\n\t"\
		:					/* outputs: none */\
		:[__in0] "m" (Xin0)	/* All inputs from memory addresses here */\
		,[__in1] "m" (Xin1)\
		,[__in2] "m" (Xin2)\
		,[__in3] "m" (Xin3)\
		,[__in4] "m" (Xin4)\
		,[__in5] "m" (Xin5)\
		,[__in6] "m" (Xin6)\
		,[__in7] "m" (Xin7)\
		,[__in8] "m" (Xin8)\
		,[__in9] "m" (Xin9)\
		,[__ina] "m" (Xina)\
		,[__inb] "m" (Xinb)\
		,[__inc] "m" (Xinc)\
		,[__ind] "m" (Xind)\
		,[__ine] "m" (Xine)\
		,[__inf] "m" (Xinf)\
		,[__isrt2] "m" (Xisrt2)\
		,[__two] "m" (Xtwo)\
		,[__out0] "m" (Xout0)\
		,[__o1] "e" (Xo1)\
		,[__o2] "e" (Xo2)\
		,[__o3] "e" (Xo3)\
		,[__o4] "e" (Xo4)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","rdi","r8","r9","r10","r11","r12","r13","r15","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"	/* Clobbered registers */\
	);\
	}

  #else	// USE_AVX2 = False, i.e. AVX sans FMA3:

	/* Complex multiply of 2 roots of unity - use e.g. for "multiply up" of sincos twiddles. */
	#define SSE2_CMUL_EXPO(XcA,XcB,XcAmB,XcApB)\
	{\
	__asm__ volatile (\
		"movq	%[__cA]		,%%rax\n\t"\
		"movq	%[__cB]		,%%rbx\n\t"\
		"movq	%[__cAmB]	,%%rcx\n\t"\
		"movq	%[__cApB]	,%%rdx\n\t"\
		"\n\t"\
		"vmovaps	    (%%rax),%%ymm0\n\t"\
		"vmovaps	0x20(%%rax),%%ymm2\n\t"\
		"vmovaps	    (%%rbx),%%ymm4\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm5\n\t"\
		"vmovaps	%%ymm0,%%ymm1\n\t"\
		"vmovaps	%%ymm2,%%ymm3\n\t"\
		"\n\t"\
		"vmulpd	%%ymm4,%%ymm0,%%ymm0\n\t"\
		"vmulpd	%%ymm5,%%ymm1,%%ymm1\n\t"\
		"vmulpd	%%ymm4,%%ymm2,%%ymm2\n\t"\
		"vmulpd	%%ymm5,%%ymm3,%%ymm3\n\t"\
		"vmovaps	%%ymm0,%%ymm4\n\t"\
		"vmovaps	%%ymm1,%%ymm5\n\t"\
		"vaddpd	%%ymm3,%%ymm0,%%ymm0\n\t"\
		"vsubpd	%%ymm2,%%ymm1,%%ymm1\n\t"\
		"vsubpd	%%ymm3,%%ymm4,%%ymm4\n\t"\
		"vaddpd	%%ymm2,%%ymm5,%%ymm5\n\t"\
		"vmovaps	%%ymm0,    (%%rcx)\n\t"\
		"vmovaps	%%ymm1,0x20(%%rcx)\n\t"\
		"vmovaps	%%ymm4,    (%%rdx)\n\t"\
		"vmovaps	%%ymm5,0x20(%%rdx)\n\t"\
		:					/* outputs: none */\
		: [__cA]  "m" (XcA)	/* All inputs from memory addresses here */\
		 ,[__cB]  "m" (XcB)\
		 ,[__cAmB] "m" (XcAmB)\
		 ,[__cApB] "m" (XcApB)\
		: "cc","memory","rax","rbx","rcx","rdx","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5"		/* Clobbered registers */\
	);\
	}

	#define SSE2_RADIX_03_DFT_X2(Xcc0, Xi0,Xi1,Xi2, Xo0,Xo1,Xo2, Xj0,Xj1,Xj2, Xu0,Xu1,Xu2)\
	{\
	__asm__ volatile (\
		"movq	%[__i0],%%rax				\n\t	movq	%[__j0],%%r10		\n\t"\
		"movq	%[__i1],%%rbx				\n\t	movq	%[__j1],%%r11		\n\t"\
		"movq	%[__i2],%%rcx				\n\t	movq	%[__j2],%%r12		\n\t"\
		"movq	%[__cc0],%%rdx				\n\t"\
		"vmovaps	    (%%rbx),%%ymm2		\n\t	vmovaps	    (%%r11),%%ymm10	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3		\n\t	vmovaps	0x20(%%r11),%%ymm11	\n\t"\
		"vmovaps	    (%%rax),%%ymm0		\n\t	vmovaps	    (%%r10),%%ymm8 	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1		\n\t	vmovaps	0x20(%%r10),%%ymm9 	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6		\n\t	vmovaps	    (%%r12),%%ymm14	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7		\n\t	vmovaps	0x20(%%r12),%%ymm15	\n\t"\
		"vmovaps	%%ymm2,%%ymm4			\n\t	vmovaps	%%ymm10,%%ymm12		\n\t"\
		"vmovaps	%%ymm3,%%ymm5			\n\t	vmovaps	%%ymm11,%%ymm13		\n\t"\
		"movq	%[__o0],%%rax				\n\t	movq	%[__u0],%%r10		\n\t"\
		"movq	%[__o1],%%rbx				\n\t	movq	%[__u1],%%r11		\n\t"\
		"movq	%[__o2],%%rcx				\n\t	movq	%[__u2],%%r12		\n\t"\
		"vaddpd	%%ymm6,%%ymm2,%%ymm2		\n\t	vaddpd	%%ymm14,%%ymm10,%%ymm10		\n\t"\
		"vaddpd	%%ymm7,%%ymm3,%%ymm3		\n\t	vaddpd	%%ymm15,%%ymm11,%%ymm11		\n\t"\
		"vsubpd	%%ymm6,%%ymm4,%%ymm4		\n\t	vsubpd	%%ymm14,%%ymm12,%%ymm12		\n\t"\
		"vsubpd	%%ymm7,%%ymm5,%%ymm5		\n\t	vsubpd	%%ymm15,%%ymm13,%%ymm13		\n\t"\
		"vaddpd	%%ymm2,%%ymm0,%%ymm0		\n\t	vaddpd	%%ymm10,%%ymm8 ,%%ymm8 		\n\t"\
		"vaddpd	%%ymm3,%%ymm1,%%ymm1		\n\t	vaddpd	%%ymm11,%%ymm9 ,%%ymm9 		\n\t"\
		"vmovaps	    (%%rdx),%%ymm6		\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7		\n\t"\
		"vmovaps	%%ymm0,    (%%rax)		\n\t	vmovaps	%%ymm8 ,    (%%r10)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rax)		\n\t	vmovaps	%%ymm9 ,0x20(%%r10)	\n\t"\
		"vmulpd	%%ymm6,%%ymm2,%%ymm2		\n\t	vmulpd	%%ymm6 ,%%ymm10,%%ymm10		\n\t"\
		"vmulpd	%%ymm6,%%ymm3,%%ymm3		\n\t	vmulpd	%%ymm6 ,%%ymm11,%%ymm11		\n\t"\
		"vmulpd	%%ymm7,%%ymm4,%%ymm4		\n\t	vmulpd	%%ymm7 ,%%ymm12,%%ymm12		\n\t"\
		"vmulpd	%%ymm7,%%ymm5,%%ymm5		\n\t	vmulpd	%%ymm7 ,%%ymm13,%%ymm13		\n\t"\
		"vaddpd	%%ymm0,%%ymm2,%%ymm2		\n\t	vaddpd	%%ymm8 ,%%ymm10,%%ymm10		\n\t"\
		"vaddpd	%%ymm1,%%ymm3,%%ymm3		\n\t	vaddpd	%%ymm9 ,%%ymm11,%%ymm11		\n\t"\
		"vmovaps	%%ymm2,%%ymm0			\n\t	vmovaps	%%ymm10,%%ymm8 		\n\t"\
		"vmovaps	%%ymm3,%%ymm1			\n\t	vmovaps	%%ymm11,%%ymm9 		\n\t"\
		"vsubpd	%%ymm5,%%ymm2,%%ymm2		\n\t	vsubpd	%%ymm13,%%ymm10,%%ymm10		\n\t"\
		"vaddpd	%%ymm4,%%ymm3,%%ymm3		\n\t	vaddpd	%%ymm12,%%ymm11,%%ymm11		\n\t"\
		"vaddpd	%%ymm5,%%ymm0,%%ymm0		\n\t	vaddpd	%%ymm13,%%ymm8 ,%%ymm8 		\n\t"\
		"vsubpd	%%ymm4,%%ymm1,%%ymm1		\n\t	vsubpd	%%ymm12,%%ymm9 ,%%ymm9 		\n\t"\
		"vmovaps	%%ymm2,    (%%rbx)		\n\t	vmovaps	%%ymm10,    (%%r11)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rbx)		\n\t	vmovaps	%%ymm11,0x20(%%r11)	\n\t"\
		"vmovaps	%%ymm0,    (%%rcx)		\n\t	vmovaps	%%ymm8 ,    (%%r12)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rcx)		\n\t	vmovaps	%%ymm9 ,0x20(%%r12)	\n\t"\
		:					/* outputs: none */\
		: [__cc0] "m" (Xcc0)	/* All inputs from memory addresses here */\
		 ,[__i0] "m" (Xi0)\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__j0] "m" (Xj0)\
		 ,[__j1] "m" (Xj1)\
		 ,[__j2] "m" (Xj2)\
		 ,[__u0] "m" (Xu0)\
		 ,[__u1] "m" (Xu1)\
		 ,[__u2] "m" (Xu2)\
		: "cc","memory","rax","rbx","rcx","rdx","r10","r11","r12","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

	#define SSE2_RADIX_05_DFT_0TWIDDLE_X2(Xcc1,Xtwo, Xi0,Xi1,Xi2,Xi3,Xi4, Xo0,Xo1,Xo2,Xo3,Xo4, Xj0,Xj1,Xj2,Xj3,Xj4, Xu0,Xu1,Xu2,Xu3,Xu4)\
	{\
	__asm__ volatile (\
		"movq	%[__i0],%%rsi				\n\t	movq	%[__j0],%%r10		\n\t"\
		"movq	%[__i1],%%rax				\n\t	movq	%[__j1],%%r11		\n\t"\
		"movq	%[__i2],%%rbx				\n\t	movq	%[__j2],%%r12		\n\t"\
		"movq	%[__i3],%%rcx				\n\t	movq	%[__j3],%%r13		\n\t"\
		"movq	%[__i4],%%rdx				\n\t	movq	%[__j4],%%r14		\n\t"\
		"movq	%[__o0],%%rdi				\n\t	movq	%[__u0],%%r15		\n\t"\
		"vmovaps	    (%%rax),%%ymm0		\n\t	vmovaps	    (%%r11),%%ymm8 	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1		\n\t	vmovaps	0x20(%%r11),%%ymm9 	\n\t"\
		"vmovaps	    (%%rbx),%%ymm2		\n\t	vmovaps	    (%%r12),%%ymm10	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3		\n\t	vmovaps	0x20(%%r12),%%ymm11	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4		\n\t	vmovaps	    (%%r13),%%ymm12	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5		\n\t	vmovaps	0x20(%%r13),%%ymm13	\n\t"\
		"vmovaps	    (%%rdx),%%ymm6		\n\t	vmovaps	    (%%r14),%%ymm14	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7		\n\t	vmovaps	0x20(%%r14),%%ymm15	\n\t"\
		"vsubpd	%%ymm6,%%ymm0,%%ymm0		\n\t	vsubpd	%%ymm14,%%ymm8 ,%%ymm8 		\n\t"\
		"vsubpd	%%ymm7,%%ymm1,%%ymm1		\n\t	vsubpd	%%ymm15,%%ymm9 ,%%ymm9 		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t	vaddpd	%%ymm14,%%ymm14,%%ymm14		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t	vaddpd	%%ymm15,%%ymm15,%%ymm15		\n\t"\
		"vaddpd	%%ymm0,%%ymm6,%%ymm6		\n\t	vaddpd	%%ymm8 ,%%ymm14,%%ymm14		\n\t"\
		"vaddpd	%%ymm1,%%ymm7,%%ymm7		\n\t	vaddpd	%%ymm9 ,%%ymm15,%%ymm15		\n\t"\
		"vsubpd	%%ymm4,%%ymm2,%%ymm2		\n\t	vsubpd	%%ymm12,%%ymm10,%%ymm10		\n\t"\
		"vsubpd	%%ymm5,%%ymm3,%%ymm3		\n\t	vsubpd	%%ymm13,%%ymm11,%%ymm11		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t	vaddpd	%%ymm12,%%ymm12,%%ymm12		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t	vaddpd	%%ymm13,%%ymm13,%%ymm13		\n\t"\
		"vaddpd	%%ymm2,%%ymm4,%%ymm4		\n\t	vaddpd	%%ymm10,%%ymm12,%%ymm12		\n\t"\
		"vaddpd	%%ymm3,%%ymm5,%%ymm5		\n\t	vaddpd	%%ymm11,%%ymm13,%%ymm13		\n\t"\
		"movq	%[__cc1],%%rax				\n\t"\
		"vsubpd	%%ymm4,%%ymm6,%%ymm6		\n\t	vsubpd	%%ymm12,%%ymm14,%%ymm14		\n\t"\
		"vsubpd	%%ymm5,%%ymm7,%%ymm7		\n\t	vsubpd	%%ymm13,%%ymm15,%%ymm15		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t	vaddpd	%%ymm12,%%ymm12,%%ymm12		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t	vaddpd	%%ymm13,%%ymm13,%%ymm13		\n\t"\
		"vaddpd	%%ymm6,%%ymm4,%%ymm4		\n\t	vaddpd	%%ymm14,%%ymm12,%%ymm12		\n\t"\
		"vaddpd	%%ymm7,%%ymm5,%%ymm5		\n\t	vaddpd	%%ymm15,%%ymm13,%%ymm13		\n\t"\
		"vaddpd	    (%%rsi),%%ymm4,%%ymm4	\n\t	vaddpd	    (%%r10),%%ymm12,%%ymm12	\n\t"\
		"vaddpd	0x20(%%rsi),%%ymm5,%%ymm5	\n\t	vaddpd	0x20(%%r10),%%ymm13,%%ymm13	\n\t"\
		"vmovaps	%%ymm4,    (%%rdi)		\n\t	vmovaps	%%ymm12,    (%%r15)		\n\t"\
		"vmovaps	%%ymm5,0x20(%%rdi)		\n\t	vmovaps	%%ymm13,0x20(%%r15)		\n\t"\
		"vmulpd	0x20(%%rax),%%ymm6,%%ymm6	\n\t	vmulpd	0x20(%%rax),%%ymm14,%%ymm14	\n\t"\
		"vmulpd	0x20(%%rax),%%ymm7,%%ymm7	\n\t	vmulpd	0x20(%%rax),%%ymm15,%%ymm15	\n\t"\
		"vsubpd	    (%%rsi),%%ymm4,%%ymm4	\n\t	vsubpd	    (%%r10),%%ymm12,%%ymm12	\n\t"\
		"vsubpd	0x20(%%rsi),%%ymm5,%%ymm5	\n\t	vsubpd	0x20(%%r10),%%ymm13,%%ymm13	\n\t"\
		"vmulpd	    (%%rax),%%ymm4,%%ymm4	\n\t	vmulpd	    (%%rax),%%ymm12,%%ymm12	\n\t"\
		"vmulpd	    (%%rax),%%ymm5,%%ymm5	\n\t	vmulpd	    (%%rax),%%ymm13,%%ymm13	\n\t"\
		"vaddpd	    (%%rdi),%%ymm4,%%ymm4	\n\t	vaddpd	    (%%r15),%%ymm12,%%ymm12	\n\t"\
		"vaddpd	0x20(%%rdi),%%ymm5,%%ymm5	\n\t	vaddpd	0x20(%%r15),%%ymm13,%%ymm13	\n\t"\
		"vsubpd	%%ymm6,%%ymm4,%%ymm4		\n\t	vsubpd	%%ymm14,%%ymm12,%%ymm12		\n\t"\
		"vsubpd	%%ymm7,%%ymm5,%%ymm5		\n\t	vsubpd	%%ymm15,%%ymm13,%%ymm13		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t	vaddpd	%%ymm14,%%ymm14,%%ymm14		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t	vaddpd	%%ymm15,%%ymm15,%%ymm15		\n\t"\
		"vaddpd	%%ymm4,%%ymm6,%%ymm6		\n\t	vaddpd	%%ymm12,%%ymm14,%%ymm14		\n\t"\
		"vaddpd	%%ymm5,%%ymm7,%%ymm7		\n\t	vaddpd	%%ymm13,%%ymm15,%%ymm15		\n\t"\
		"vmovaps	%%ymm4,    (%%rsi)		\n\t	vmovaps	%%ymm12,    (%%r10)		\n\t"\
		"vmovaps	%%ymm5,0x20(%%rsi)		\n\t	vmovaps	%%ymm13,0x20(%%r10)		\n\t"\
		"vmovaps	%%ymm0,%%ymm4			\n\t	vmovaps	%%ymm8 ,%%ymm12			\n\t"\
		"vmovaps	%%ymm1,%%ymm5			\n\t	vmovaps	%%ymm9 ,%%ymm13			\n\t"\
		"vsubpd	%%ymm2,%%ymm0,%%ymm0		\n\t	vsubpd	%%ymm10,%%ymm8,%%ymm8 		\n\t"\
		"vsubpd	%%ymm3,%%ymm1,%%ymm1		\n\t	vsubpd	%%ymm11,%%ymm9,%%ymm9 		\n\t"\
		"vmulpd	0x40(%%rax),%%ymm0,%%ymm0	\n\t	vmulpd	0x40(%%rax),%%ymm8 ,%%ymm8 	\n\t"\
		"vmulpd	0x40(%%rax),%%ymm1,%%ymm1	\n\t	vmulpd	0x40(%%rax),%%ymm9 ,%%ymm9 	\n\t"\
		"vmulpd	0x60(%%rax),%%ymm2,%%ymm2	\n\t	vmulpd	0x60(%%rax),%%ymm10,%%ymm10	\n\t"\
		"vmulpd	0x60(%%rax),%%ymm3,%%ymm3	\n\t	vmulpd	0x60(%%rax),%%ymm11,%%ymm11	\n\t"\
		"vmulpd	0x80(%%rax),%%ymm4,%%ymm4	\n\t	vmulpd	0x80(%%rax),%%ymm12,%%ymm12	\n\t"\
		"vmulpd	0x80(%%rax),%%ymm5,%%ymm5	\n\t	vmulpd	0x80(%%rax),%%ymm13,%%ymm13	\n\t"\
		"vaddpd	%%ymm0,%%ymm2,%%ymm2		\n\t	vaddpd	%%ymm8 ,%%ymm10,%%ymm10		\n\t"\
		"vaddpd	%%ymm1,%%ymm3,%%ymm3		\n\t	vaddpd	%%ymm9 ,%%ymm11,%%ymm11		\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t	vsubpd	%%ymm12,%%ymm8 ,%%ymm8 		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1		\n\t	vsubpd	%%ymm13,%%ymm9 ,%%ymm9 		\n\t"\
		"vmovaps	    (%%rsi),%%ymm4		\n\t	vmovaps	    (%%r10),%%ymm12		\n\t"\
		"vmovaps	0x20(%%rsi),%%ymm5		\n\t	vmovaps	0x20(%%r10),%%ymm13		\n\t"\
		"movq	%[__o1],%%rax				\n\t	movq	%[__u1],%%r11			\n\t"\
		"movq	%[__o4],%%rdx				\n\t	movq	%[__u4],%%r14			\n\t"\
		"vsubpd	%%ymm3,%%ymm6,%%ymm6		\n\t	vsubpd	%%ymm11,%%ymm14,%%ymm14		\n\t"\
		"vsubpd	%%ymm2,%%ymm7,%%ymm7		\n\t	vsubpd	%%ymm10,%%ymm15,%%ymm15		\n\t"\
		"vaddpd	%%ymm3,%%ymm3,%%ymm3		\n\t	vaddpd	%%ymm11,%%ymm11,%%ymm11		\n\t"\
		"vaddpd	%%ymm2,%%ymm2,%%ymm2		\n\t	vaddpd	%%ymm10,%%ymm10,%%ymm10		\n\t"\
		"vmovaps	%%ymm6,    (%%rax)		\n\t	vmovaps	%%ymm14,    (%%r11)		\n\t"\
		"vmovaps	%%ymm7,0x20(%%rdx)		\n\t	vmovaps	%%ymm15,0x20(%%r14)		\n\t"\
		"vaddpd	%%ymm6,%%ymm3,%%ymm3		\n\t	vaddpd	%%ymm14,%%ymm11,%%ymm11		\n\t"\
		"vaddpd	%%ymm7,%%ymm2,%%ymm2		\n\t	vaddpd	%%ymm15,%%ymm10,%%ymm10		\n\t"\
		"vmovaps	%%ymm3,    (%%rdx)		\n\t	vmovaps	%%ymm11,    (%%r14)		\n\t"\
		"vmovaps	%%ymm2,0x20(%%rax)		\n\t	vmovaps	%%ymm10,0x20(%%r11)		\n\t"\
		"movq	%[__o2],%%rbx				\n\t	movq	%[__u2],%%r12			\n\t"\
		"movq	%[__o3],%%rcx				\n\t	movq	%[__u3],%%r13			\n\t"\
		"vsubpd	%%ymm1,%%ymm4,%%ymm4		\n\t	vsubpd	%%ymm9 ,%%ymm12,%%ymm12		\n\t"\
		"vsubpd	%%ymm0,%%ymm5,%%ymm5		\n\t	vsubpd	%%ymm8 ,%%ymm13,%%ymm13		\n\t"\
		"vaddpd	%%ymm1,%%ymm1,%%ymm1		\n\t	vaddpd	%%ymm9 ,%%ymm9 ,%%ymm9 		\n\t"\
		"vaddpd	%%ymm0,%%ymm0,%%ymm0		\n\t	vaddpd	%%ymm8 ,%%ymm8 ,%%ymm8 		\n\t"\
		"vmovaps	%%ymm4,    (%%rbx)		\n\t	vmovaps	%%ymm12,    (%%r12)		\n\t"\
		"vmovaps	%%ymm5,0x20(%%rcx)		\n\t	vmovaps	%%ymm13,0x20(%%r13)		\n\t"\
		"vaddpd	%%ymm4,%%ymm1,%%ymm1		\n\t	vaddpd	%%ymm12,%%ymm9,%%ymm9 		\n\t"\
		"vaddpd	%%ymm5,%%ymm0,%%ymm0		\n\t	vaddpd	%%ymm13,%%ymm8,%%ymm8 		\n\t"\
		"vmovaps	%%ymm1,    (%%rcx)		\n\t	vmovaps	%%ymm9 ,    (%%r13)		\n\t"\
		"vmovaps	%%ymm0,0x20(%%rbx)		\n\t	vmovaps	%%ymm8 ,0x20(%%r12)		\n\t"\
		:					/* outputs: none */\
		: [__cc1] "m" (Xcc1)	/* All inputs from memory addresses here */\
		 ,[__two] "m" (Xtwo)\
		 ,[__i0] "m" (Xi0)\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__i3] "m" (Xi3)\
		 ,[__i4] "m" (Xi4)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__o3] "m" (Xo3)\
		 ,[__o4] "m" (Xo4)\
		 ,[__j0] "m" (Xj0)\
		 ,[__j1] "m" (Xj1)\
		 ,[__j2] "m" (Xj2)\
		 ,[__j3] "m" (Xj3)\
		 ,[__j4] "m" (Xj4)\
		 ,[__u0] "m" (Xu0)\
		 ,[__u1] "m" (Xu1)\
		 ,[__u2] "m" (Xu2)\
		 ,[__u3] "m" (Xu3)\
		 ,[__u4] "m" (Xu4)\
		: "cc","memory","rax","rbx","rcx","rdx","rdi","rsi","r10","r11","r12","r13","r14","r15","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

	// AVX version has shufpd immediate = 5 = 0101_2, which is the doubled analog of the SSE2 imm8 = 1 = 01_2:
	#define PAIR_SQUARE_4_SSE2(XtAr, XtBr, XtCr, XtDr, Xc, Xs, Xforth)\
	{\
	__asm__ volatile (\
		"movq	%[__tDr]	,%%rdx		\n\t"\
		"movq	%[__tAr]	,%%rax		\n\t"\
		"\n\t"\
		"vmovaps	    (%%rdx)	,%%ymm6		\n\t"\
		"vmovaps	0x20(%%rdx)	,%%ymm7		\n\t"\
		"vmovaps	    (%%rax)	,%%ymm0		\n\t"\
		"vmovaps	0x20(%%rax)	,%%ymm3		\n\t"\
		"vshufpd	$5,%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vshufpd	$5,%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	    (%%rax)	,%%ymm2		\n\t"\
		"vmovaps	0x20(%%rax)	,%%ymm1		\n\t"\
		"\n\t"\
		"vmulpd	%%ymm6,%%ymm0,%%ymm0		\n\t"\
		"vmulpd	%%ymm7,%%ymm3,%%ymm3		\n\t"\
		"vmulpd	%%ymm6,%%ymm1,%%ymm1		\n\t"\
		"vmulpd	%%ymm7,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm3,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm2,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm0,%%ymm0,%%ymm0		\n\t"\
		"vaddpd	%%ymm1,%%ymm1,%%ymm1		\n\t"\
		"\n\t"\
		"movq	%[__tCr]	,%%rcx		\n\t"\
		"movq	%[__tBr]	,%%rbx		\n\t"\
		"\n\t"\
		"vmovaps	    (%%rcx)	,%%ymm6		\n\t"\
		"vmovaps	0x20(%%rcx)	,%%ymm7		\n\t"\
		"vmovaps	    (%%rbx)	,%%ymm2		\n\t"\
		"vmovaps	0x20(%%rbx)	,%%ymm5		\n\t"\
		"vshufpd	$5,%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vshufpd	$5,%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	    (%%rbx)	,%%ymm4		\n\t"\
		"vmovaps	0x20(%%rbx)	,%%ymm3		\n\t"\
		"\n\t"\
		"vmulpd	%%ymm6,%%ymm2,%%ymm2		\n\t"\
		"vmulpd	%%ymm7,%%ymm5,%%ymm5		\n\t"\
		"vmulpd	%%ymm6,%%ymm3,%%ymm3		\n\t"\
		"vmulpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm5,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm4,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm2,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm3,%%ymm3,%%ymm3		\n\t"\
		"\n\t"\
		"vmovaps	    (%%rax)	,%%ymm4		\n\t"\
		"vmovaps	0x20(%%rax)	,%%ymm5		\n\t"\
		"vsubpd	%%ymm5,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm4,%%ymm5,%%ymm5		\n\t"\
		"vmulpd	%%ymm5,%%ymm4,%%ymm4		\n\t"\
		"\n\t"\
		"vmovaps	    (%%rax)	,%%ymm5		\n\t"\
		"vmulpd	0x20(%%rax)	,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm5		,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm4	,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5	,0x20(%%rax)	\n\t"\
		"\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"\n\t"\
		"vmovaps	    (%%rbx)	,%%ymm6		\n\t"\
		"vmovaps	0x20(%%rbx)	,%%ymm7		\n\t"\
		"vsubpd	%%ymm7,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm6,%%ymm7,%%ymm7		\n\t"\
		"vmulpd	%%ymm7,%%ymm6,%%ymm6		\n\t"\
		"\n\t"\
		"vmovaps	    (%%rbx)	,%%ymm7		\n\t"\
		"vmulpd	0x20(%%rbx)	,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm7		,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	%%ymm6	,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm7	,0x20(%%rbx)	\n\t"\
		"\n\t"\
		"vsubpd	%%ymm6,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm7,%%ymm3,%%ymm3		\n\t"\
		"\n\t"\
		"vmovaps	    (%%rdx)	,%%ymm4		\n\t"\
		"vmovaps	0x20(%%rdx)	,%%ymm5		\n\t"\
		"vsubpd	%%ymm5,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm4,%%ymm5,%%ymm5		\n\t"\
		"vmulpd	%%ymm5,%%ymm4,%%ymm4		\n\t"\
		"\n\t"\
		"vmovaps	    (%%rdx)	,%%ymm5		\n\t"\
		"vmulpd	0x20(%%rdx)	,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm5		,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm4	,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm5	,0x20(%%rdx)	\n\t"\
		"vshufpd	$5,%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vshufpd	$5,%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vaddpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"\n\t"\
		"vmovaps	    (%%rcx)	,%%ymm6		\n\t"\
		"vmovaps	0x20(%%rcx)	,%%ymm7		\n\t"\
		"vsubpd	%%ymm7,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm6,%%ymm7,%%ymm7		\n\t"\
		"vmulpd	%%ymm7,%%ymm6,%%ymm6		\n\t"\
		"\n\t"\
		"vmovaps	    (%%rcx)	,%%ymm7		\n\t"\
		"vmulpd	0x20(%%rcx)	,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm7		,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	%%ymm6	,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm7	,0x20(%%rcx)	\n\t"\
		"vshufpd	$5,%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vshufpd	$5,%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"\n\t"\
		"vsubpd	%%ymm6,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm7,%%ymm3,%%ymm3		\n\t"\
		"\n\t"\
		"/*** Can do in || with above segment ***/\n\t"\
		"movq	%[__c]		,%%rax		\n\t"\
		"movq	%[__s]		,%%rbx		\n\t"\
		"movq	%[__forth]	,%%rdx		\n\t"\
		"vmovaps	%%ymm0		,%%ymm4		\n\t"\
		"vmovaps	%%ymm1		,%%ymm5		\n\t"\
		"vmulpd	(%%rax)	,%%ymm0,%%ymm0		\n\t"\
		"vmulpd	(%%rax)	,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm4	,%%ymm0,%%ymm0		\n\t"\
		"vaddpd	%%ymm5	,%%ymm1,%%ymm1		\n\t"\
		"vmulpd	(%%rbx)	,%%ymm4,%%ymm4		\n\t"\
		"vmulpd	(%%rbx)	,%%ymm5,%%ymm5		\n\t"\
		"vsubpd	%%ymm5	,%%ymm0,%%ymm0		\n\t"\
		"vaddpd	%%ymm4	,%%ymm1,%%ymm1		\n\t"\
		"vmulpd	(%%rdx)	,%%ymm0,%%ymm0		\n\t"\
		"vmulpd	(%%rdx)	,%%ymm1,%%ymm1		\n\t"\
		"\n\t"\
		"/*** Can do in || wjth above segment ***/\n\t"\
		"vmovaps	%%ymm2	,%%ymm6		\n\t"\
		"vmovaps	%%ymm3	,%%ymm7		\n\t"\
		"vmulpd	(%%rbx)	,%%ymm2,%%ymm2		\n\t"\
		"vmulpd	(%%rbx)	,%%ymm3,%%ymm3		\n\t"\
		"vsubpd	%%ymm6	,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm7	,%%ymm3,%%ymm3		\n\t"\
		"vmulpd	(%%rax)	,%%ymm6,%%ymm6		\n\t"\
		"vmulpd	(%%rax)	,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm7	,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm6	,%%ymm3,%%ymm3		\n\t"\
		"vmulpd	(%%rdx)	,%%ymm2,%%ymm2		\n\t"\
		"vmulpd	(%%rdx)	,%%ymm3,%%ymm3		\n\t"\
		"\n\t"\
		"movq	%[__tAr]	,%%rax		\n\t"\
		"movq	%[__tBr]	,%%rbx		\n\t"\
		"\n\t"\
		"vmovaps	    (%%rax)	,%%ymm4		\n\t"\
		"vmovaps	0x20(%%rax)	,%%ymm5		\n\t"\
		"vmovaps	    (%%rbx)	,%%ymm6		\n\t"\
		"vmovaps	0x20(%%rbx)	,%%ymm7		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vsubpd	%%ymm2,%%ymm6,%%ymm6		\n\t"\
		"vsubpd	%%ymm3,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	%%ymm4	,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5	,0x20(%%rax)	\n\t"\
		"vmovaps	%%ymm6	,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm7	,0x20(%%rbx)	\n\t"\
		"\n\t"\
		"movq	%[__tCr]	,%%rcx		\n\t"\
		"movq	%[__tDr]	,%%rdx		\n\t"\
		"\n\t"\
		"vshufpd	$5,%%ymm0,%%ymm0,%%ymm0		\n\t"\
		"vshufpd	$5,%%ymm1,%%ymm1,%%ymm1		\n\t"\
		"vshufpd	$5,%%ymm2,%%ymm2,%%ymm2		\n\t"\
		"vshufpd	$5,%%ymm3,%%ymm3,%%ymm3		\n\t"\
		"\n\t"\
		"vmovaps	    (%%rdx)	,%%ymm4		\n\t"\
		"vmovaps	0x20(%%rdx)	,%%ymm5		\n\t"\
		"vmovaps	    (%%rcx)	,%%ymm6		\n\t"\
		"vmovaps	0x20(%%rcx)	,%%ymm7		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t"\
		"vsubpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vsubpd	%%ymm2,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm3,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	%%ymm4	,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm5	,0x20(%%rdx)	\n\t"\
		"vmovaps	%%ymm6	,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm7	,0x20(%%rcx)	\n\t"\
		:					/* outputs: none */\
		: [__tAr] "m" (XtAr)	/* All inputs from memory addresses here */\
		 ,[__tBr] "m" (XtBr)\
		 ,[__tCr] "m" (XtCr)\
		 ,[__tDr] "m" (XtDr)\
		 ,[__c] "m" (Xc)\
		 ,[__s] "m" (Xs)\
		 ,[__forth] "m" (Xforth)\
		: "cc","memory","rax","rbx","rcx","rdx","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

	/*...Radix-7 DFT: Inputs in memlocs __i0-6, outputs into __o0-6, possibly coincident with inputs:\ */\
	// AVX version: [54 ADD, 34 SUB, 16 MUL, 54 memref]
	//
	#define SSE2_RADIX_07_DFT(Xi0,Xi1,Xi2,Xi3,Xi4,Xi5,Xi6, Xcc, Xo0,Xo1,Xo2,Xo3,Xo4,Xo5,Xo6)\
	{\
	__asm__ volatile (\
		"movq	%[__i1],%%rax		\n\t"\
		"movq	%[__i2],%%rbx		\n\t"\
		"movq	%[__i3],%%rcx		\n\t"\
		"movq	%[__i4],%%rdx		\n\t"\
		"movq	%[__i5],%%rsi		\n\t"\
		"movq	%[__i6],%%rdi		\n\t	/*** Imaginary Parts: ***/	\n\t"\
		"vmovaps	(%%rax),%%ymm6		\n\t	vmovaps	0x20(%%rax),%%ymm14	\n\t"\
		"vmovaps	(%%rdi),%%ymm1		\n\t	vmovaps	0x20(%%rdi),%%ymm9 	\n\t"\
		"vmovaps	(%%rbx),%%ymm5		\n\t	vmovaps	0x20(%%rbx),%%ymm13	\n\t"\
		"vmovaps	(%%rsi),%%ymm2		\n\t	vmovaps	0x20(%%rsi),%%ymm10	\n\t"\
		"vmovaps	(%%rcx),%%ymm4		\n\t	vmovaps	0x20(%%rcx),%%ymm12	\n\t"\
		"vmovaps	(%%rdx),%%ymm3		\n\t	vmovaps	0x20(%%rdx),%%ymm11	\n\t"\
		"movq	%[__i0],%%rbx		\n\t"\
		"vsubpd	%%ymm1,%%ymm6,%%ymm6	\n\t	vsubpd	%%ymm9 ,%%ymm14,%%ymm14		\n\t"\
		"vsubpd	%%ymm2,%%ymm5,%%ymm5	\n\t	vsubpd	%%ymm10,%%ymm13,%%ymm13		\n\t"\
		"vsubpd	%%ymm3,%%ymm4,%%ymm4	\n\t	vsubpd	%%ymm11,%%ymm12,%%ymm12		\n\t"\
		"vaddpd	%%ymm1,%%ymm1,%%ymm1	\n\t	vaddpd	%%ymm9 ,%%ymm9 ,%%ymm9 		\n\t"\
		"vaddpd	%%ymm2,%%ymm2,%%ymm2	\n\t	vaddpd	%%ymm10,%%ymm10,%%ymm10		\n\t"\
		"vaddpd	%%ymm3,%%ymm3,%%ymm3	\n\t	vaddpd	%%ymm11,%%ymm11,%%ymm11		\n\t"\
		"vaddpd	%%ymm6,%%ymm1,%%ymm1	\n\t	vaddpd	%%ymm14,%%ymm9 ,%%ymm9  	\n\t"\
		"vaddpd	%%ymm5,%%ymm2,%%ymm2	\n\t	vaddpd	%%ymm13,%%ymm10,%%ymm10		\n\t"\
		"vaddpd	%%ymm4,%%ymm3,%%ymm3	\n\t	vaddpd	%%ymm12,%%ymm11,%%ymm11		\n\t"\
		"vmovaps	(%%rbx),%%ymm0		\n\t	vmovaps	0x20(%%rbx),%%ymm8 	\n\t"\
		"\n\t"\
		"movq	%[__o0],%%rcx		\n\t"\
		"movq	%[__cc],%%rsi		\n\t"\
		"vmovaps	%%ymm0,0x100(%%rsi)	\n\t	vmovaps	%%ymm8 ,0x140(%%rsi)	\n\t"\
		"vmovaps	%%ymm6,0x120(%%rsi)	\n\t	vmovaps	%%ymm14,0x160(%%rsi)	\n\t"\
		"vaddpd	%%ymm1,%%ymm0,%%ymm0	\n\t	vaddpd	%%ymm9 ,%%ymm8 ,%%ymm8 	\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t	vmovaps	%%ymm13,%%ymm15		\n\t"\
		"vaddpd	%%ymm2,%%ymm3,%%ymm3	\n\t	vaddpd	%%ymm10,%%ymm11,%%ymm11		\n\t"\
		"vsubpd	%%ymm4,%%ymm5,%%ymm5	\n\t	vsubpd	%%ymm12,%%ymm13,%%ymm13		\n\t"\
		"vsubpd	%%ymm2,%%ymm1,%%ymm1	\n\t	vsubpd	%%ymm10,%%ymm9 ,%%ymm9  	\n\t"\
		"vsubpd	%%ymm7,%%ymm6,%%ymm6	\n\t	vsubpd	%%ymm15,%%ymm14,%%ymm14		\n\t"\
		"vaddpd	%%ymm2,%%ymm2,%%ymm2	\n\t	vaddpd	%%ymm10,%%ymm10,%%ymm10		\n\t"\
		"vaddpd	%%ymm7,%%ymm4,%%ymm4	\n\t	vaddpd	%%ymm15,%%ymm12,%%ymm12		\n\t"\
		"vaddpd	%%ymm3,%%ymm0,%%ymm0	\n\t	vaddpd	%%ymm11,%%ymm8 ,%%ymm8  	\n\t"\
	"vaddpd	0x120(%%rsi),%%ymm5,%%ymm5	\n\t	vaddpd	0x160(%%rsi),%%ymm13,%%ymm13\n\t"\
		"vsubpd	%%ymm2,%%ymm3,%%ymm3	\n\t	vsubpd	%%ymm10,%%ymm11,%%ymm11		\n\t"\
		"vmovaps	%%ymm4,%%ymm7		\n\t	vmovaps	%%ymm12,%%ymm15		\n\t"\
		"vmovaps	%%ymm0,    (%%rcx)	\n\t	vmovaps	%%ymm8 ,0x20(%%rcx)	\n\t"/* B0 */\
		"vsubpd	%%ymm6,%%ymm4,%%ymm4	\n\t	vsubpd		%%ymm14,%%ymm12,%%ymm12		\n\t"\
		"vmovaps	%%ymm1,%%ymm2		\n\t	vmovaps	%%ymm9 ,%%ymm10		\n\t"\
	"vsubpd 0x100(%%rsi),%%ymm0,%%ymm0	\n\t	vsubpd 0x140(%%rsi),%%ymm8 ,%%ymm8  \n\t"\
	"vmulpd	0x20(%%rsi),%%ymm5,%%ymm5	\n\t	vmulpd	0x20(%%rsi),%%ymm13,%%ymm13	\n\t"\
	"vaddpd	%%ymm3,%%ymm2,%%ymm2		\n\t	vaddpd	%%ymm11,%%ymm10,%%ymm10		\n\t"\
	"vmulpd	0x80(%%rsi),%%ymm3,%%ymm3	\n\t	vmulpd	0x80(%%rsi),%%ymm11,%%ymm11	\n\t"\
	"vmulpd	0xe0(%%rsi),%%ymm4,%%ymm4	\n\t	vmulpd	0xe0(%%rsi),%%ymm12,%%ymm12	\n\t"\
	"vmulpd	0x40(%%rsi),%%ymm1,%%ymm1	\n\t	vmulpd	0x40(%%rsi),%%ymm9 ,%%ymm9  \n\t"\
	"vmulpd	0x60(%%rsi),%%ymm6,%%ymm6	\n\t	vmulpd	0x60(%%rsi),%%ymm14,%%ymm14	\n\t"\
	"vmulpd	    (%%rsi),%%ymm0,%%ymm0	\n\t	vmulpd	    (%%rsi),%%ymm8 ,%%ymm8  \n\t"\
	"vmulpd	0xa0(%%rsi),%%ymm7,%%ymm7	\n\t	vmulpd	0xa0(%%rsi),%%ymm15,%%ymm15	\n\t"\
	"vmulpd	0xc0(%%rsi),%%ymm2,%%ymm2	\n\t	vmulpd	0xc0(%%rsi),%%ymm10,%%ymm10	\n\t"\
	"vaddpd	    (%%rcx),%%ymm0,%%ymm0	\n\t	vaddpd	0x20(%%rcx),%%ymm8 ,%%ymm8  \n\t"\
	"vaddpd	%%ymm4,%%ymm6,%%ymm6		\n\t	vaddpd	%%ymm12,%%ymm14,%%ymm14		\n\t"\
	"vsubpd	%%ymm2,%%ymm1,%%ymm1		\n\t	vsubpd	%%ymm10,%%ymm9 ,%%ymm9  	\n\t"\
	"vsubpd	%%ymm7,%%ymm4,%%ymm4		\n\t	vsubpd	%%ymm15,%%ymm12,%%ymm12		\n\t"\
	"vsubpd	%%ymm2,%%ymm3,%%ymm3		\n\t	vsubpd	%%ymm10,%%ymm11,%%ymm11		\n\t"\
		"movq	%[__o1],%%rax		\n\t"\
		"movq	%[__o2],%%rbx		\n\t"\
		"movq	%[__o3],%%rcx		\n\t"\
		"movq	%[__o4],%%rdx		\n\t"\
		"movq	%[__o5],%%rsi		\n\t"\
		"movq	%[__o6],%%rdi		\n\t"\
		"vmovaps	%%ymm0,%%ymm2		\n\t	vmovaps	%%ymm8 ,%%ymm10		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t	vmovaps	%%ymm13,%%ymm15		\n\t"\
		"vaddpd	%%ymm1,%%ymm0,%%ymm0	\n\t	vaddpd	%%ymm9 ,%%ymm8 ,%%ymm8  	\n\t"\
		"vaddpd	%%ymm6,%%ymm5,%%ymm5	\n\t	vaddpd	%%ymm14,%%ymm13,%%ymm13		\n\t"\
		"vaddpd	%%ymm3,%%ymm1,%%ymm1	\n\t	vaddpd	%%ymm11,%%ymm9 ,%%ymm9  	\n\t"\
		"vaddpd	%%ymm4,%%ymm6,%%ymm6	\n\t	vaddpd	%%ymm12,%%ymm14,%%ymm14		\n\t"\
		"vaddpd	%%ymm2,%%ymm3,%%ymm3	\n\t	vaddpd	%%ymm10,%%ymm11,%%ymm11		\n\t"\
		"vaddpd	%%ymm7,%%ymm4,%%ymm4	\n\t	vaddpd	%%ymm15,%%ymm12,%%ymm12		\n\t"\
		"vsubpd	%%ymm1,%%ymm2,%%ymm2	\n\t	vsubpd	%%ymm9 ,%%ymm10,%%ymm10		\n\t"\
		"vsubpd	%%ymm6,%%ymm7,%%ymm7	\n\t	vsubpd	%%ymm14,%%ymm15,%%ymm15		\n\t"\
		/* ymm1,6,9,14 free ... Note the order reversal on the 3rd pair of outputs: */\
		"vsubpd	%%ymm13,%%ymm0 ,%%ymm0 	\n\t	vsubpd	%%ymm15,%%ymm2 ,%%ymm2 		\n\t	vsubpd	%%ymm12,%%ymm3 ,%%ymm3 		\n\t"\
		"vsubpd	%%ymm5 ,%%ymm8 ,%%ymm8  \n\t	vsubpd	%%ymm7 ,%%ymm10,%%ymm10		\n\t	vsubpd	%%ymm4 ,%%ymm11,%%ymm11		\n\t"\
		"vaddpd	%%ymm13,%%ymm13,%%ymm13	\n\t	vaddpd	%%ymm15,%%ymm15,%%ymm15		\n\t	vaddpd	%%ymm12,%%ymm12,%%ymm12		\n\t"\
		"vaddpd	%%ymm5 ,%%ymm5 ,%%ymm5 	\n\t	vaddpd	%%ymm7 ,%%ymm7 ,%%ymm7 		\n\t	vaddpd	%%ymm4 ,%%ymm4 ,%%ymm4 		\n\t"\
		"vaddpd	%%ymm0 ,%%ymm13,%%ymm13	\n\t	vaddpd	%%ymm2 ,%%ymm15,%%ymm15		\n\t	vaddpd	%%ymm3 ,%%ymm12,%%ymm12		\n\t"\
		"vaddpd	%%ymm8 ,%%ymm5 ,%%ymm5 	\n\t	vaddpd	%%ymm10,%%ymm7 ,%%ymm7 		\n\t	vaddpd	%%ymm11,%%ymm4 ,%%ymm4 		\n\t"\
		"vmovaps	%%ymm0 ,    (%%rax)	\n\t	vmovaps	%%ymm2 ,    (%%rbx)	\n\t	vmovaps	%%ymm3 ,    (%%rdx)	\n\t"/* B124r */\
		"vmovaps	%%ymm8 ,0x20(%%rdi)	\n\t	vmovaps	%%ymm10,0x20(%%rsi)	\n\t	vmovaps	%%ymm11,0x20(%%rcx)	\n\t"/* B653i */\
		"vmovaps	%%ymm13,    (%%rdi)	\n\t	vmovaps	%%ymm15,    (%%rsi)	\n\t	vmovaps	%%ymm12,    (%%rcx)	\n\t"/* B653r */\
		"vmovaps	%%ymm5 ,0x20(%%rax)	\n\t	vmovaps	%%ymm7 ,0x20(%%rbx)	\n\t	vmovaps	%%ymm4 ,0x20(%%rdx)	\n\t"/* B124i */\
		"										\n\t"\
		:					/* outputs: none */\
		: [__i0] "m" (Xi0)	/* All inputs from memory addresses here */\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__i3] "m" (Xi3)\
		 ,[__i4] "m" (Xi4)\
		 ,[__i5] "m" (Xi5)\
		 ,[__i6] "m" (Xi6)\
		 ,[__cc] "m" (Xcc)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__o3] "m" (Xo3)\
		 ,[__o4] "m" (Xo4)\
		 ,[__o5] "m" (Xo5)\
		 ,[__o6] "m" (Xo6)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","rdi","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

	/* Twiddleless version of SSE2_RADIX8_DIF_TWIDDLE. Inputs enter in memory locations __r0 + [__i1,__i2,__i3,__i4,__i5,__i6,__i7],;
	where r0 is a memory address and the i's are LITERAL [BYTE] OFFSETS. Outputs go into memory locations __o0,__o1,__o2,__o3,__o4,__o5,__o6,__o7, assumed disjoint with inputs:\
	*/
	#define SSE2_RADIX8_DIF_0TWIDDLE(Xr0, Xi1,Xi2,Xi3,Xi4,Xi5,Xi6,Xi7, Xo0,Xo1,Xo2,Xo3,Xo4,Xo5,Xo6,Xo7, Xisrt2)\
	{\
	__asm__ volatile (\
		"/* 1st of 2 radix-4 subtransforms, data in ymm0-7: */\n\t	/* 2nd of 2 radix-4 subtransforms, data in ymm8-15: */\n\t"\
		"movq	%[__r0],%%rax	/* i0 = r00 */	\n\t		leaq	%c[__i1](%%rax),%%r10	/* i1 */\n\t"\
		"leaq	%c[__i2](%%rax),%%rbx			\n\t		leaq	%c[__i3](%%rax),%%r11	/* i3 */\n\t"\
		"leaq	%c[__i4](%%rax),%%rcx			\n\t		leaq	%c[__i5](%%rax),%%r12	/* i5 */\n\t"\
		"leaq	%c[__i6](%%rax),%%rdx			\n\t		leaq	%c[__i7](%%rax),%%r13	/* i7 */\n\t"\
		"movq	%[__isrt2],%%rsi				\n\t		/* p1,5 combo: x+y into ymm8 /1, x-y in ymm10/3: */	\n\t"\
		"/* p0,4 combo: x+-y into ymm0/1, 2/3, resp: */\n\t	vmovaps	     (%%r12),%%ymm8 			\n\t"\
		"										\n\t		vmovaps	0x020(%%r12),%%ymm9 			\n\t"\
		"vmovaps	     (%%rcx),%%ymm0			\n\t		vmovaps	     (%%r10),%%ymm10			\n\t"\
		"vmovaps	0x020(%%rcx),%%ymm1			\n\t		vmovaps	0x020(%%r10),%%ymm11			\n\t"\
		"vmovaps	     (%%rax),%%ymm2			\n\t		vsubpd	%%ymm8 ,%%ymm10,%%ymm10			\n\t"\
		"vmovaps	0x020(%%rax),%%ymm3			\n\t		vsubpd	%%ymm9 ,%%ymm11,%%ymm11			\n\t"\
		"vsubpd	%%ymm0,%%ymm2,%%ymm2			\n\t		vaddpd	%%ymm8 ,%%ymm8 ,%%ymm8 			\n\t"\
		"vsubpd	%%ymm1,%%ymm3,%%ymm3			\n\t		vaddpd	%%ymm9 ,%%ymm9 ,%%ymm9 			\n\t"\
		"vaddpd	%%ymm0,%%ymm0,%%ymm0			\n\t		vaddpd	%%ymm10,%%ymm8 ,%%ymm8 			\n\t"\
		"vaddpd	%%ymm1,%%ymm1,%%ymm1			\n\t		vaddpd	%%ymm11,%%ymm9 ,%%ymm9 			\n\t"\
		"vaddpd	%%ymm2,%%ymm0,%%ymm0			\n\t		/* p3,7 combo: x+y into ymm14/7, x-y in ymm12/5: */	\n\t"\
		"vaddpd	%%ymm3,%%ymm1,%%ymm1			\n\t		vmovaps	     (%%r11),%%ymm12			\n\t"\
		"										\n\t		vmovaps	0x020(%%r11),%%ymm13			\n\t"\
		"/* p2,6 combo: x+-y into ymm4/5, 6/7, resp: */\n\t	vmovaps	     (%%r13),%%ymm14			\n\t"\
		"										\n\t		vmovaps	0x020(%%r13),%%ymm15			\n\t"\
		"vmovaps	     (%%rdx),%%ymm4			\n\t		vsubpd	%%ymm14,%%ymm12,%%ymm12			\n\t"\
		"vmovaps	0x020(%%rdx),%%ymm5			\n\t		vsubpd	%%ymm15,%%ymm13,%%ymm13			\n\t"\
		"vmovaps	     (%%rbx),%%ymm6			\n\t		vaddpd	%%ymm14,%%ymm14,%%ymm14			\n\t"\
		"vmovaps	0x020(%%rbx),%%ymm7			\n\t		vaddpd	%%ymm15,%%ymm15,%%ymm15			\n\t"\
		"vsubpd	%%ymm4,%%ymm6,%%ymm6			\n\t		vaddpd	%%ymm12,%%ymm14,%%ymm14			\n\t"\
		"vsubpd	%%ymm5,%%ymm7,%%ymm7			\n\t		vaddpd	%%ymm13,%%ymm15,%%ymm15			\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4			\n\t		/* Finish radix-4 butterfly, tmp-store 1st of 4 outputs to free up 2 registers: */\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5			\n\t		vsubpd	%%ymm14,%%ymm8 ,%%ymm8 			\n\t"\
		"vaddpd	%%ymm6,%%ymm4,%%ymm4			\n\t		vsubpd	%%ymm15,%%ymm9 ,%%ymm9 			\n\t"\
		"vaddpd	%%ymm7,%%ymm5,%%ymm5			\n\t		vsubpd	%%ymm13,%%ymm10,%%ymm10			\n\t"\
		"										\n\t		vsubpd	%%ymm12,%%ymm11,%%ymm11			\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0			\n\t		vaddpd	%%ymm14,%%ymm14,%%ymm14			\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2			\n\t		vaddpd	%%ymm13,%%ymm13,%%ymm13			\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1			\n\t		vaddpd	%%ymm15,%%ymm15,%%ymm15			\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3			\n\t		vaddpd	%%ymm12,%%ymm12,%%ymm12			\n\t"\
		"													vaddpd	%%ymm8 ,%%ymm14,%%ymm14			\n\t"\
		"													vaddpd	%%ymm10,%%ymm13,%%ymm13			\n\t"\
		"													vaddpd	%%ymm9 ,%%ymm15,%%ymm15			\n\t"\
		"													vaddpd	%%ymm11,%%ymm12,%%ymm12			\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4			\n\t		vmovaps	%%ymm14,     (%%r10)			\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7			\n\t		vmovaps	%%ymm15,0x020(%%r10)			\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5			\n\t		vmovaps	%%ymm10,%%ymm14					\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6			\n\t		vmovaps	%%ymm13,%%ymm15					\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4			\n\t		vsubpd	%%ymm12,%%ymm10,%%ymm10			\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7			\n\t		vsubpd	%%ymm11,%%ymm13,%%ymm13			\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5			\n\t		vaddpd	%%ymm14,%%ymm12,%%ymm12			\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6			\n\t		vaddpd	%%ymm15,%%ymm11,%%ymm11			\n\t"\
		"													vmovaps	(%%rsi),%%ymm14	/* isrt2 */		\n\t"\
		"													vmulpd	%%ymm14,%%ymm10,%%ymm10			\n\t"\
		"													vmulpd	%%ymm14,%%ymm13,%%ymm13			\n\t"\
		"													vmulpd	%%ymm14,%%ymm12,%%ymm12			\n\t"\
		"										\n\t		vmulpd	%%ymm14,%%ymm11,%%ymm11			\n\t"\
		"vmovaps      (%%r10),%%ymm14 /* reload spill */\n\t"\
		"vmovaps 0x020(%%r10),%%ymm15 /* reload spill */\n\t"\
		"										\n\t"\
		"/* Inline of SSE2_RADIX8_DIF_COMBINE_RAD4_SUBS_A(r0): Combine radix-4 subtransforms and write outputs: */\n\t"\
		"/***** t0,1,2,3,4,5,6,7 in ymm[ 4, 5| 2,6| 0, 1| 7,3] *****/\n\t"\
		"/***** t8,9,a,b,c,d,e,f in ymm[14,15|10,12| 8, 9|13,11] */\n\t"\
		"movq	%[__o4],%%rax					\n\t		vsubpd   %%ymm10,%%ymm2	,%%ymm2			\n\t"\
		"movq	%[__o5],%%rbx					\n\t		vsubpd   %%ymm12,%%ymm6	,%%ymm6			\n\t"\
		"movq	%[__o6],%%rcx					\n\t		vaddpd   %%ymm10,%%ymm10,%%ymm10		\n\t"\
		"movq	%[__o7],%%rdx					\n\t		vaddpd   %%ymm12,%%ymm12,%%ymm12		\n\t"\
		"										\n\t		vaddpd   %%ymm2 ,%%ymm10,%%ymm10		\n\t"\
		"vsubpd   %%ymm11,%%ymm7 ,%%ymm7 		\n\t		vaddpd   %%ymm6 ,%%ymm12,%%ymm12		\n\t"\
		"vsubpd   %%ymm13,%%ymm3 ,%%ymm3 		\n\t"\
		"vaddpd   %%ymm11,%%ymm11,%%ymm11		\n\t		vmovaps	%%ymm2 ,     (%%rbx)	/* o5r */	\n\t"\
		"vaddpd   %%ymm13,%%ymm13,%%ymm13		\n\t		vmovaps	%%ymm6 ,0x020(%%rbx)	/* o5i */	\n\t"\
		"vaddpd   %%ymm7 ,%%ymm11,%%ymm11		\n\t		vmovaps	%%ymm10,     (%%rax)	/* o4r */	\n\t"\
		"vaddpd   %%ymm3 ,%%ymm13,%%ymm13		\n\t		vmovaps	%%ymm12,0x020(%%rax)	/* o4i */	\n\t"\
		"										\n\t"\
		"vmovaps	%%ymm7 ,     (%%rcx)	/* o6r */\n\t"\
		"vmovaps	%%ymm3 ,0x020(%%rdx)	/* o7i */\n\t"\
		"vmovaps	%%ymm11,    (%%rdx)	/* o7r */	\n\t"\
		"vmovaps	%%ymm13,0x020(%%rcx)	/* o6i */\n\t"\
		"										\n\t"\
		"movq	%[__o0],%%rax					\n\t"\
		"movq	%[__o1],%%rbx					\n\t"\
		"movq	%[__o2],%%rcx					\n\t"\
		"movq	%[__o3],%%rdx					\n\t"\
		"										\n\t"\
		"vsubpd	%%ymm14,%%ymm4 ,%%ymm4 			\n\t"\
		"vsubpd	%%ymm15,%%ymm5 ,%%ymm5 			\n\t"\
		"vsubpd	%%ymm9 ,%%ymm0 ,%%ymm0 			\n\t"\
		"vsubpd	%%ymm8 ,%%ymm1 ,%%ymm1 			\n\t"\
		"vaddpd	%%ymm14,%%ymm14,%%ymm14			\n\t		vmovaps	%%ymm4 ,     (%%rbx)	/* o1r */	\n\t"\
		"vaddpd	%%ymm15,%%ymm15,%%ymm15			\n\t		vmovaps	%%ymm5 ,0x020(%%rbx)	/* o1i */	\n\t"\
		"vaddpd	%%ymm9 ,%%ymm9 ,%%ymm9 			\n\t		vmovaps	%%ymm0 ,     (%%rcx)	/* o2r */	\n\t"\
		"vaddpd	%%ymm8 ,%%ymm8 ,%%ymm8 			\n\t		vmovaps	%%ymm1 ,0x020(%%rdx)	/* o3i */	\n\t"\
		"vaddpd	%%ymm4 ,%%ymm14,%%ymm14			\n\t"\
		"vaddpd	%%ymm5 ,%%ymm15,%%ymm15			\n\t"\
		"vaddpd	%%ymm0 ,%%ymm9 ,%%ymm9 			\n\t"\
		"vaddpd	%%ymm1 ,%%ymm8 ,%%ymm8 			\n\t"\
		"										\n\t"\
		"vmovaps	%%ymm14,    (%%rax)	/* o0r */	\n\t"\
		"vmovaps	%%ymm15,0x020(%%rax)	/* o0r */\n\t"\
		"vmovaps	%%ymm9 ,     (%%rdx)	/* o3r */\n\t"\
		"vmovaps	%%ymm8 ,0x020(%%rcx)	/* o2i */\n\t"\
		"										\n\t"\
		:					/* outputs: none */\
		: [__r0] "m" (Xr0)	/* All inputs from memory addresses here */\
		 ,[__i1] "e" (Xi1)\
		 ,[__i2] "e" (Xi2)\
		 ,[__i3] "e" (Xi3)\
		 ,[__i4] "e" (Xi4)\
		 ,[__i5] "e" (Xi5)\
		 ,[__i6] "e" (Xi6)\
		 ,[__i7] "e" (Xi7)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__o3] "m" (Xo3)\
		 ,[__o4] "m" (Xo4)\
		 ,[__o5] "m" (Xo5)\
		 ,[__o6] "m" (Xo6)\
		 ,[__o7] "m" (Xo7)\
		 ,[__isrt2] "m" (Xisrt2)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

	// Need a 2nd version of above which takes the i-strides as intvars rather than literal bytes:
	#define SSE2_RADIX8_DIF_0TWIDDLE_B(Xr0, Xi1,Xi2,Xi3,Xi4,Xi5,Xi6,Xi7, Xo0,Xo1,Xo2,Xo3,Xo4,Xo5,Xo6,Xo7, Xisrt2)\
	{\
	__asm__ volatile (\
		"/* 1st of 2 radix-4 subtransforms, data in ymm0-7: */\n\t	/* 2nd of 2 radix-4 subtransforms, data in ymm8-15: */\n\t"\
		"movq	%[__r0],%%rax	/* i0 = r00 */	\n\t		movslq	%[__i1],%%r10		/* i1 */	\n\t"\
		"movslq	%[__i2],%%rbx	/* i2 */		\n\t		movslq	%[__i3],%%r11		/* i3 */	\n\t"\
		"movslq	%[__i4],%%rcx	/* i4 */		\n\t		movslq	%[__i5],%%r12		/* i5 */	\n\t"\
		"movslq	%[__i6],%%rdx	/* i6 */		\n\t		movslq	%[__i7],%%r13		/* i7 */	\n\t"\
		"addq	%%rax,%%rbx						\n\t		addq	%%rax,%%r10						\n\t"\
		"addq	%%rax,%%rcx						\n\t		addq	%%rax,%%r11						\n\t"\
		"addq	%%rax,%%rdx						\n\t		addq	%%rax,%%r12						\n\t"\
		"movq	%[__isrt2],%%rsi				\n\t		addq	%%rax,%%r13						\n\t"\
		"										\n\t		/* p1,5 combo: x+y into ymm8 /1, x-y in ymm10/3: */	\n\t"\
		"/* p0,4 combo: x+-y into ymm0/1, 2/3, resp: */\n\t	vmovaps	     (%%r12),%%ymm8 			\n\t"\
		"										\n\t		vmovaps	0x020(%%r12),%%ymm9 			\n\t"\
		"vmovaps	     (%%rcx),%%ymm0			\n\t		vmovaps	     (%%r10),%%ymm10			\n\t"\
		"vmovaps	0x020(%%rcx),%%ymm1			\n\t		vmovaps	0x020(%%r10),%%ymm11			\n\t"\
		"vmovaps	     (%%rax),%%ymm2			\n\t		vsubpd	%%ymm8 ,%%ymm10,%%ymm10			\n\t"\
		"vmovaps	0x020(%%rax),%%ymm3			\n\t		vsubpd	%%ymm9 ,%%ymm11,%%ymm11			\n\t"\
		"vsubpd	%%ymm0,%%ymm2,%%ymm2			\n\t		vaddpd	%%ymm8 ,%%ymm8 ,%%ymm8 			\n\t"\
		"vsubpd	%%ymm1,%%ymm3,%%ymm3			\n\t		vaddpd	%%ymm9 ,%%ymm9 ,%%ymm9 			\n\t"\
		"vaddpd	%%ymm0,%%ymm0,%%ymm0			\n\t		vaddpd	%%ymm10,%%ymm8 ,%%ymm8 			\n\t"\
		"vaddpd	%%ymm1,%%ymm1,%%ymm1			\n\t		vaddpd	%%ymm11,%%ymm9 ,%%ymm9 			\n\t"\
		"vaddpd	%%ymm2,%%ymm0,%%ymm0			\n\t		/* p3,7 combo: x+y into ymm14/7, x-y in ymm12/5: */	\n\t"\
		"vaddpd	%%ymm3,%%ymm1,%%ymm1			\n\t		vmovaps	     (%%r11),%%ymm12			\n\t"\
		"										\n\t		vmovaps	0x020(%%r11),%%ymm13			\n\t"\
		"/* p2,6 combo: x+-y into ymm4/5, 6/7, resp: */\n\t	vmovaps	     (%%r13),%%ymm14			\n\t"\
		"										\n\t		vmovaps	0x020(%%r13),%%ymm15			\n\t"\
		"vmovaps	     (%%rdx),%%ymm4			\n\t		vsubpd	%%ymm14,%%ymm12,%%ymm12			\n\t"\
		"vmovaps	0x020(%%rdx),%%ymm5			\n\t		vsubpd	%%ymm15,%%ymm13,%%ymm13			\n\t"\
		"vmovaps	     (%%rbx),%%ymm6			\n\t		vaddpd	%%ymm14,%%ymm14,%%ymm14			\n\t"\
		"vmovaps	0x020(%%rbx),%%ymm7			\n\t		vaddpd	%%ymm15,%%ymm15,%%ymm15			\n\t"\
		"vsubpd	%%ymm4,%%ymm6,%%ymm6			\n\t		vaddpd	%%ymm12,%%ymm14,%%ymm14			\n\t"\
		"vsubpd	%%ymm5,%%ymm7,%%ymm7			\n\t		vaddpd	%%ymm13,%%ymm15,%%ymm15			\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4			\n\t		/* Finish radix-4 butterfly, tmp-store 1st of 4 outputs to free up 2 registers: */\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5			\n\t		vsubpd	%%ymm14,%%ymm8 ,%%ymm8 			\n\t"\
		"vaddpd	%%ymm6,%%ymm4,%%ymm4			\n\t		vsubpd	%%ymm15,%%ymm9 ,%%ymm9 			\n\t"\
		"vaddpd	%%ymm7,%%ymm5,%%ymm5			\n\t		vsubpd	%%ymm13,%%ymm10,%%ymm10			\n\t"\
		"										\n\t		vsubpd	%%ymm12,%%ymm11,%%ymm11			\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0			\n\t		vaddpd	%%ymm14,%%ymm14,%%ymm14			\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2			\n\t		vaddpd	%%ymm13,%%ymm13,%%ymm13			\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1			\n\t		vaddpd	%%ymm15,%%ymm15,%%ymm15			\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3			\n\t		vaddpd	%%ymm12,%%ymm12,%%ymm12			\n\t"\
		"													vaddpd	%%ymm8 ,%%ymm14,%%ymm14			\n\t"\
		"													vaddpd	%%ymm10,%%ymm13,%%ymm13			\n\t"\
		"													vaddpd	%%ymm9 ,%%ymm15,%%ymm15			\n\t"\
		"													vaddpd	%%ymm11,%%ymm12,%%ymm12			\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4			\n\t		vmovaps	%%ymm14,     (%%r10)			\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7			\n\t		vmovaps	%%ymm15,0x020(%%r10)			\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5			\n\t		vmovaps	%%ymm10,%%ymm14					\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6			\n\t		vmovaps	%%ymm13,%%ymm15					\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4			\n\t		vsubpd	%%ymm12,%%ymm10,%%ymm10			\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7			\n\t		vsubpd	%%ymm11,%%ymm13,%%ymm13			\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5			\n\t		vaddpd	%%ymm14,%%ymm12,%%ymm12			\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6			\n\t		vaddpd	%%ymm15,%%ymm11,%%ymm11			\n\t"\
		"													vmovaps	(%%rsi),%%ymm14	/* isrt2 */		\n\t"\
		"													vmulpd	%%ymm14,%%ymm10,%%ymm10			\n\t"\
		"													vmulpd	%%ymm14,%%ymm13,%%ymm13			\n\t"\
		"													vmulpd	%%ymm14,%%ymm12,%%ymm12			\n\t"\
		"										\n\t		vmulpd	%%ymm14,%%ymm11,%%ymm11			\n\t"\
		"vmovaps      (%%r10),%%ymm14 /* reload spill */\n\t"\
		"vmovaps 0x020(%%r10),%%ymm15 /* reload spill */\n\t"\
		"										\n\t"\
		"/* Inline of SSE2_RADIX8_DIF_COMBINE_RAD4_SUBS_A(r0): Combine radix-4 subtransforms and write outputs: */\n\t"\
		"/***** t0,1,2,3,4,5,6,7 in ymm[ 4, 5| 2,6| 0, 1| 7,3] *****/\n\t"\
		"/***** t8,9,a,b,c,d,e,f in ymm[14,15|10,12| 8, 9|13,11] */\n\t"\
		"movq	%[__o4],%%rax					\n\t		vsubpd   %%ymm10,%%ymm2	,%%ymm2			\n\t"\
		"movq	%[__o5],%%rbx					\n\t		vsubpd   %%ymm12,%%ymm6	,%%ymm6			\n\t"\
		"movq	%[__o6],%%rcx					\n\t		vaddpd   %%ymm10,%%ymm10,%%ymm10		\n\t"\
		"movq	%[__o7],%%rdx					\n\t		vaddpd   %%ymm12,%%ymm12,%%ymm12		\n\t"\
		"										\n\t		vaddpd   %%ymm2 ,%%ymm10,%%ymm10		\n\t"\
		"vsubpd   %%ymm11,%%ymm7 ,%%ymm7 		\n\t		vaddpd   %%ymm6 ,%%ymm12,%%ymm12		\n\t"\
		"vsubpd   %%ymm13,%%ymm3 ,%%ymm3 		\n\t"\
		"vaddpd   %%ymm11,%%ymm11,%%ymm11		\n\t		vmovaps	%%ymm2 ,     (%%rbx)	/* o5r */	\n\t"\
		"vaddpd   %%ymm13,%%ymm13,%%ymm13		\n\t		vmovaps	%%ymm6 ,0x020(%%rbx)	/* o5i */	\n\t"\
		"vaddpd   %%ymm7 ,%%ymm11,%%ymm11		\n\t		vmovaps	%%ymm10,     (%%rax)	/* o4r */	\n\t"\
		"vaddpd   %%ymm3 ,%%ymm13,%%ymm13		\n\t		vmovaps	%%ymm12,0x020(%%rax)	/* o4i */	\n\t"\
		"										\n\t"\
		"vmovaps	%%ymm7 ,     (%%rcx)	/* o6r */\n\t"\
		"vmovaps	%%ymm3 ,0x020(%%rdx)	/* o7i */\n\t"\
		"vmovaps	%%ymm11,     (%%rdx)	/* o7r */\n\t"\
		"vmovaps	%%ymm13,0x020(%%rcx)	/* o6i */\n\t"\
		"										\n\t"\
		"movq	%[__o0],%%rax					\n\t"\
		"movq	%[__o1],%%rbx					\n\t"\
		"movq	%[__o2],%%rcx					\n\t"\
		"movq	%[__o3],%%rdx					\n\t"\
		"										\n\t"\
		"vsubpd	%%ymm14,%%ymm4 ,%%ymm4 			\n\t"\
		"vsubpd	%%ymm15,%%ymm5 ,%%ymm5 			\n\t"\
		"vsubpd	%%ymm9 ,%%ymm0 ,%%ymm0 			\n\t"\
		"vsubpd	%%ymm8 ,%%ymm1 ,%%ymm1 			\n\t"\
		"vaddpd	%%ymm14,%%ymm14,%%ymm14			\n\t		vmovaps	%%ymm4 ,     (%%rbx)	/* o1r */	\n\t"\
		"vaddpd	%%ymm15,%%ymm15,%%ymm15			\n\t		vmovaps	%%ymm5 ,0x020(%%rbx)	/* o1i */	\n\t"\
		"vaddpd	%%ymm9 ,%%ymm9 ,%%ymm9 			\n\t		vmovaps	%%ymm0 ,     (%%rcx)	/* o2r */	\n\t"\
		"vaddpd	%%ymm8 ,%%ymm8 ,%%ymm8 			\n\t		vmovaps	%%ymm1 ,0x020(%%rdx)	/* o3i */	\n\t"\
		"vaddpd	%%ymm4 ,%%ymm14,%%ymm14			\n\t"\
		"vaddpd	%%ymm5 ,%%ymm15,%%ymm15			\n\t"\
		"vaddpd	%%ymm0 ,%%ymm9 ,%%ymm9 			\n\t"\
		"vaddpd	%%ymm1 ,%%ymm8 ,%%ymm8 			\n\t"\
		"										\n\t"\
		"vmovaps	%%ymm14,     (%%rax)	/* o0r */\n\t"\
		"vmovaps	%%ymm15,0x020(%%rax)	/* o0r */\n\t"\
		"vmovaps	%%ymm9 ,     (%%rdx)	/* o3r */\n\t"\
		"vmovaps	%%ymm8 ,0x020(%%rcx)	/* o2i */\n\t"\
		"										\n\t"\
		:					/* outputs: none */\
		: [__r0] "m" (Xr0)	/* All inputs from memory addresses here */\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__i3] "m" (Xi3)\
		 ,[__i4] "m" (Xi4)\
		 ,[__i5] "m" (Xi5)\
		 ,[__i6] "m" (Xi6)\
		 ,[__i7] "m" (Xi7)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__o3] "m" (Xo3)\
		 ,[__o4] "m" (Xo4)\
		 ,[__o5] "m" (Xo5)\
		 ,[__o6] "m" (Xo6)\
		 ,[__o7] "m" (Xo7)\
		 ,[__isrt2] "m" (Xisrt2)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

	// AVX analog of dft_macro.h::RADIX_08_DIF_TWIDDLE_OOP - Result of adding separate I/O addressing to
	// radix8_dif_dit_pass_gcc64.h::SSE2_RADIX8_DIF_TWIDDLE:
	#define SSE2_RADIX8_DIF_TWIDDLE_OOP(Xi0,Xi1,Xi2,Xi3,Xi4,Xi5,Xi6,Xi7, Xo0,Xo1,Xo2,Xo3,Xo4,Xo5,Xo6,Xo7 ,Xc1,Xs1,Xc2,Xs2,Xc3,Xs3,Xc4,Xs4,Xc5,Xs5,Xc6,Xs6,Xc7,Xs7)\
	{\
	__asm__ volatile (\
		"													movq		%[i1]	,%%r10				\n\t"\
		"													movq		%[i5]	,%%r11				\n\t"\
		"											movq	%[c1],%%r12	\n\t	movq	%[c5],%%r14	\n\t"\
		"											movq	%[s1],%%r13	\n\t	movq	%[s5],%%r15	\n\t"\
		"													vmovaps	    (%%r10)	,%%ymm8 			\n\t"\
		"movq		%[i0]	,%%rax				\n\t		vmovaps	0x20(%%r10)	,%%ymm10			\n\t"\
		"movq		%[i4]	,%%rbx				\n\t		vmovaps	    (%%r10)	,%%ymm9 			\n\t"\
		"movq		%[c4]	,%%rcx				\n\t		vmovaps	0x20(%%r10)	,%%ymm11			\n\t"\
		"movq		%[s4]	,%%rsi			\n\t"\
	/* [rsi] (and if needed rdi) points to sine components of each sincos pair, which is not really a pair here in terms of relative addressing: */\
		"vmovaps	    (%%rax)	,%%ymm0			\n\t		vmulpd	    (%%r12)	,%%ymm8 ,%%ymm8 	\n\t"\
		"vmovaps	0x20(%%rax)	,%%ymm1			\n\t		vmulpd	    (%%r13)	,%%ymm10,%%ymm10	\n\t"\
		"vmovaps	    (%%rax)	,%%ymm6			\n\t		vmulpd	    (%%r13)	,%%ymm9 ,%%ymm9 	\n\t"\
		"vmovaps	0x20(%%rax)	,%%ymm7			\n\t		vmulpd	    (%%r12)	,%%ymm11,%%ymm11	\n\t"\
		"vmovaps	    (%%rbx)	,%%ymm2			\n\t		vsubpd	%%ymm10		,%%ymm8 ,%%ymm8 	\n\t"\
		"vmovaps	0x20(%%rbx)	,%%ymm3			\n\t		vaddpd	%%ymm11		,%%ymm9 ,%%ymm9 	\n\t"\
		"vmovaps	    (%%rbx)	,%%ymm4			\n\t		vmovaps	    (%%r11)	,%%ymm10			\n\t"\
		"vmovaps	0x20(%%rbx)	,%%ymm5			\n\t		vmovaps	0x20(%%r11)	,%%ymm11			\n\t"\
		"vmulpd	    (%%rcx)	,%%ymm2,%%ymm2		\n\t		vmovaps	    (%%r11)	,%%ymm12			\n\t"\
		"vmulpd	    (%%rcx)	,%%ymm3,%%ymm3		\n\t		vmovaps	0x20(%%r11)	,%%ymm13			\n\t"\
		"vmulpd	    (%%rsi)	,%%ymm4,%%ymm4		\n\t		vmulpd	    (%%r14)	,%%ymm10,%%ymm10	\n\t"\
		"vmulpd	    (%%rsi)	,%%ymm5,%%ymm5		\n\t		vmulpd	    (%%r15)	,%%ymm11,%%ymm11	\n\t"\
		"vsubpd	%%ymm5		,%%ymm2,%%ymm2		\n\t		vmulpd	    (%%r15)	,%%ymm12,%%ymm12	\n\t"\
		"vaddpd	%%ymm4		,%%ymm3,%%ymm3		\n\t		vmulpd	    (%%r14)	,%%ymm13,%%ymm13	\n\t"\
		"vaddpd	%%ymm2		,%%ymm0,%%ymm0		\n\t		vsubpd	%%ymm11		,%%ymm10,%%ymm10	\n\t"\
		"vaddpd	%%ymm3		,%%ymm1,%%ymm1		\n\t		vaddpd	%%ymm13		,%%ymm12,%%ymm12	\n\t"\
		"vsubpd	%%ymm2		,%%ymm6,%%ymm6		\n\t		vmovaps	%%ymm10		,%%ymm11			\n\t"\
		"vsubpd	%%ymm3		,%%ymm7,%%ymm7		\n\t		vmovaps	%%ymm12		,%%ymm13			\n\t"\
		"vmovaps	%%ymm0		,    (%%rax)	\n\t		vaddpd	%%ymm8 		,%%ymm10,%%ymm10	\n\t"\
		"vmovaps	%%ymm1		,0x20(%%rax)	\n\t		vsubpd	%%ymm11		,%%ymm8 ,%%ymm8 	\n\t"\
		"vmovaps	%%ymm6		,    (%%rbx)	\n\t		vaddpd	%%ymm9 		,%%ymm12,%%ymm12	\n\t"\
		"vmovaps	%%ymm7		,0x20(%%rbx)	\n\t		vsubpd	%%ymm13		,%%ymm9 ,%%ymm9 	\n\t"\
		"movq		%[i2]	,%%rax				\n\t		vmovaps	%%ymm10		,    (%%r10)	\n\t"\
		"movq		%[i6]	,%%rbx				\n\t		vmovaps	%%ymm12		,0x20(%%r10)	\n\t"\
		"movq %[c2],%%rcx \n\t movq %[s2],%%rsi	\n\t		vmovaps	%%ymm8 		,    (%%r11)	\n\t"\
		"movq %[c6],%%rdx \n\t movq %[s6],%%rdi	\n\t		vmovaps	%%ymm9 		,0x20(%%r11)	\n\t"\
		"vmovaps	    (%%rax)	,%%ymm0			\n\t		movq		%[i3]	,%%r10				\n\t"\
		"vmovaps	0x20(%%rax)	,%%ymm2			\n\t		movq		%[i7]	,%%r11				\n\t"\
		"vmovaps	    (%%rax)	,%%ymm1			\n\t		movq %[c3],%%r12 \n\t movq %[s3],%%r13	\n\t"\
		"vmovaps	0x20(%%rax)	,%%ymm3			\n\t		movq %[c7],%%r14 \n\t movq %[s7],%%r15	\n\t"\
		"vmulpd	    (%%rcx)	,%%ymm0,%%ymm0		\n\t		vmovaps	    (%%r10)	,%%ymm8 			\n\t"\
		"vmulpd	    (%%rsi)	,%%ymm2,%%ymm2		\n\t		vmovaps	0x20(%%r10)	,%%ymm10			\n\t"\
		"vmulpd	    (%%rsi)	,%%ymm1,%%ymm1		\n\t		vmovaps	    (%%r10)	,%%ymm9 			\n\t"\
		"vmulpd	    (%%rcx)	,%%ymm3,%%ymm3		\n\t		vmovaps	0x20(%%r10)	,%%ymm11			\n\t"\
		"vsubpd	%%ymm2		,%%ymm0,%%ymm0		\n\t		vmulpd	    (%%r12)	,%%ymm8 ,%%ymm8 	\n\t"\
		"vaddpd	%%ymm3		,%%ymm1,%%ymm1		\n\t		vmulpd	    (%%r13)	,%%ymm10,%%ymm10	\n\t"\
		"vmovaps	    (%%rbx)	,%%ymm2			\n\t		vmulpd	    (%%r13)	,%%ymm9 ,%%ymm9 	\n\t"\
		"vmovaps	0x20(%%rbx)	,%%ymm3			\n\t		vmulpd	    (%%r12)	,%%ymm11,%%ymm11	\n\t"\
		"vmovaps	    (%%rbx)	,%%ymm4			\n\t		vsubpd	%%ymm10		,%%ymm8 ,%%ymm8 	\n\t"\
		"vmovaps	0x20(%%rbx)	,%%ymm5			\n\t		vaddpd	%%ymm11		,%%ymm9 ,%%ymm9 	\n\t"\
		"vmulpd	    (%%rdx)	,%%ymm2,%%ymm2		\n\t		vmovaps	    (%%r11)	,%%ymm10			\n\t"\
		"vmulpd	    (%%rdi)	,%%ymm3,%%ymm3		\n\t		vmovaps	0x20(%%r11)	,%%ymm11			\n\t"\
		"vmulpd	    (%%rdi)	,%%ymm4,%%ymm4		\n\t		vmovaps	    (%%r11)	,%%ymm12			\n\t"\
		"vmulpd	    (%%rdx)	,%%ymm5,%%ymm5		\n\t		vmovaps	0x20(%%r11)	,%%ymm13			\n\t"\
		"vsubpd	%%ymm3		,%%ymm2,%%ymm2		\n\t		vmulpd	    (%%r14)	,%%ymm10,%%ymm10	\n\t"\
		"vaddpd	%%ymm5		,%%ymm4,%%ymm4		\n\t		vmulpd	    (%%r15)	,%%ymm11,%%ymm11	\n\t"\
		"vmovaps	%%ymm2		,%%ymm3			\n\t		vmulpd	    (%%r15)	,%%ymm12,%%ymm12	\n\t"\
		"vmovaps	%%ymm4		,%%ymm5			\n\t		vmulpd	    (%%r14)	,%%ymm13,%%ymm13	\n\t"\
		"vaddpd	%%ymm0		,%%ymm2,%%ymm2		\n\t		vsubpd	%%ymm11		,%%ymm10,%%ymm10	\n\t"\
		"vsubpd	%%ymm3		,%%ymm0,%%ymm0		\n\t		vaddpd	%%ymm13		,%%ymm12,%%ymm12	\n\t"\
		"vaddpd	%%ymm1		,%%ymm4,%%ymm4		\n\t		vmovaps	%%ymm10		,%%ymm11			\n\t"\
		"vsubpd	%%ymm5		,%%ymm1,%%ymm1		\n\t		vmovaps	%%ymm12		,%%ymm13			\n\t"\
		"vmovaps	%%ymm2		,    (%%rax)	\n\t		vaddpd	%%ymm8 		,%%ymm10,%%ymm10	\n\t"\
		"vmovaps	%%ymm4		,0x20(%%rax)	\n\t		vsubpd	%%ymm11		,%%ymm8 ,%%ymm8 	\n\t"\
		"vmovaps	%%ymm0		,    (%%rbx)	\n\t		vaddpd	%%ymm9 		,%%ymm12,%%ymm12	\n\t"\
		"vmovaps	%%ymm1		,0x20(%%rbx)	\n\t		vsubpd	%%ymm13		,%%ymm9 ,%%ymm9 	\n\t"\
		"													vmovaps	%%ymm10		,    (%%r10)	\n\t"\
		"													vmovaps	%%ymm12		,0x20(%%r10)	\n\t"\
		"													vmovaps	%%ymm8 		,    (%%r11)	\n\t"\
		"													vmovaps	%%ymm9 		,0x20(%%r11)	\n\t"\
/* combine to get 2 length-4 output subtransforms... */\
		"movq		%[i0]	,%%rax				\n\t		movq		%[i4]	,%%r10				\n\t"\
		"movq		%[i2]	,%%rbx				\n\t		movq		%[i6]	,%%r11				\n\t"\
		"vmovaps	    (%%rax)	,%%ymm0			\n\t		vmovaps	    (%%r10)	,%%ymm8 			\n\t"\
		"vmovaps	0x20(%%rax)	,%%ymm1			\n\t		vmovaps	0x20(%%r10)	,%%ymm9 			\n\t"\
		"vmovaps	%%ymm0		,%%ymm4			\n\t		vmovaps	%%ymm8 		,%%ymm12			\n\t"\
		"vmovaps	%%ymm1		,%%ymm5			\n\t		vmovaps	%%ymm9 		,%%ymm13			\n\t"\
		"vaddpd	    (%%rbx)	,%%ymm0,%%ymm0		\n\t		vsubpd	0x20(%%r11)	,%%ymm8 ,%%ymm8 	\n\t"\
		"vsubpd	    (%%rbx)	,%%ymm4,%%ymm4		\n\t		vaddpd	0x20(%%r11)	,%%ymm12,%%ymm12	\n\t"\
		"vaddpd	0x20(%%rbx)	,%%ymm1,%%ymm1		\n\t		vaddpd	    (%%r11)	,%%ymm9 ,%%ymm9 	\n\t"\
		"vsubpd	0x20(%%rbx)	,%%ymm5,%%ymm5		\n\t		vsubpd	    (%%r11)	,%%ymm13,%%ymm13	\n\t"\
		"movq		%[o0]	,%%rax				\n\t		movq		%[o4]	,%%r10				\n\t"\
		"movq		%[o2]	,%%rbx				\n\t		movq		%[o6]	,%%r11				\n\t"\
		"vmovaps	%%ymm0		,    (%%rax)	\n\t		vmovaps	%%ymm8 		,    (%%r10)	\n\t"\
		"vmovaps	%%ymm1		,0x20(%%rax)	\n\t		vmovaps	%%ymm9 		,0x20(%%r10)	\n\t"\
		"vmovaps	%%ymm4		,    (%%rbx)	\n\t		vmovaps	%%ymm12		,    (%%r11)	\n\t"\
		"vmovaps	%%ymm5		,0x20(%%rbx)	\n\t		vmovaps	%%ymm13		,0x20(%%r11)	\n\t"\
		"movq		%[i1]	,%%rcx				\n\t		movq		%[i5]	,%%r12				\n\t"\
		"movq		%[i3]	,%%rdx				\n\t		movq		%[i7]	,%%r13				\n\t"\
		"vmovaps	    (%%rcx)	,%%ymm2			\n\t		vmovaps	    (%%r12)	,%%ymm10			\n\t"\
		"vmovaps	0x20(%%rcx)	,%%ymm3			\n\t		vmovaps	0x20(%%r12)	,%%ymm11			\n\t"\
		"vmovaps	%%ymm2		,%%ymm6			\n\t		vmovaps	%%ymm10		,%%ymm14			\n\t"\
		"vmovaps	%%ymm3		,%%ymm7			\n\t		vmovaps	%%ymm11		,%%ymm15			\n\t"\
		"vaddpd	    (%%rdx)	,%%ymm2,%%ymm2		\n\t		vsubpd	0x20(%%r13)	,%%ymm10,%%ymm10	\n\t"\
		"vsubpd	    (%%rdx)	,%%ymm6,%%ymm6		\n\t		vaddpd	0x20(%%r13)	,%%ymm14,%%ymm14	\n\t"\
		"vaddpd	0x20(%%rdx)	,%%ymm3,%%ymm3		\n\t		vaddpd	    (%%r13)	,%%ymm11,%%ymm11	\n\t"\
		"vsubpd	0x20(%%rdx)	,%%ymm7,%%ymm7		\n\t		vsubpd	    (%%r13)	,%%ymm15,%%ymm15	\n\t"\
		"movq		%[o1]	,%%rcx				\n\t		movq		%[o5]	,%%r12				\n\t"\
		"movq		%[o3]	,%%rdx				\n\t		movq		%[o7]	,%%r13				\n\t"\
		"vsubpd	%%ymm2		,%%ymm0,%%ymm0		\n\t		vmovaps	%%ymm12		,    (%%r13)	\n\t"\
		"vsubpd	%%ymm3		,%%ymm1,%%ymm1		\n\t		vmovaps	%%ymm13		,0x20(%%r13)	\n\t"\
	/* Use the cosine term of the [c1,s1] pair, which is the *middle* [4th of 7] of our 7 input pairs, in terms \
	of the input-arg bit-reversal reordering defined in the __X[c,s] --> [c,s] mapping below and happens to \
	always in fact *be* a true cosine term, which is a requirement for our "decr 1 gives isrt2" data-copy scheme: */\
		"												movq	%[c1],%%r14	\n\t"\
		"vsubpd	%%ymm7		,%%ymm4,%%ymm4		\n\t	subq	$0x20,%%r14	\n\t"/* isrt2 in [c1]-1 */\
		"vsubpd	%%ymm6		,%%ymm5,%%ymm5		\n\t		vmovaps	%%ymm10		,%%ymm13			\n\t"\
		"vaddpd	    (%%rax)	,%%ymm2,%%ymm2		\n\t		vsubpd	%%ymm11		,%%ymm10,%%ymm10	\n\t"\
		"vaddpd	0x20(%%rax)	,%%ymm3,%%ymm3		\n\t		vaddpd	%%ymm11		,%%ymm13,%%ymm13	\n\t"\
		"vaddpd	    (%%rbx)	,%%ymm7,%%ymm7		\n\t		vmulpd	    (%%r14)	,%%ymm10,%%ymm10	\n\t"\
		"vaddpd	0x20(%%rbx)	,%%ymm6,%%ymm6		\n\t		vmulpd	    (%%r14)	,%%ymm13,%%ymm13	\n\t"\
		"vmovaps	%%ymm2,    (%%rax)	/* [o0].re */\n\t	vmovaps	0x20(%%r13)	,%%ymm11			\n\t"\
		"vmovaps	%%ymm3,0x20(%%rax)	/* [o0].im */\n\t	vmovaps	%%ymm15		,%%ymm12			\n\t"\
		"vmovaps	%%ymm4,    (%%rbx)	/* [o2].re */\n\t	vaddpd	%%ymm14		,%%ymm12,%%ymm12	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rbx)	/* [o2].im */\n\t	vsubpd	%%ymm14		,%%ymm15,%%ymm15	\n\t"\
		"vmovaps	%%ymm0,    (%%rcx)	/* [o1].re */\n\t	vmulpd	    (%%r14)	,%%ymm12,%%ymm12	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rcx)	/* [o1].im */\n\t	vmulpd	    (%%r14)	,%%ymm15,%%ymm15	\n\t"\
		"vmovaps	%%ymm7,    (%%rdx)	/* [o3].re */\n\t	vmovaps		(%%r13)	,%%ymm14			\n\t"\
		"vmovaps	%%ymm5,0x20(%%rdx)	/* [o3].im */\n\t	vsubpd	%%ymm10		,%%ymm8 ,%%ymm8 	\n\t"\
		"													vsubpd	%%ymm13		,%%ymm9 ,%%ymm9 	\n\t"\
		"													vsubpd	%%ymm12		,%%ymm14,%%ymm14	\n\t"\
		"													vsubpd	%%ymm15		,%%ymm11,%%ymm11	\n\t"\
		"													vaddpd	    (%%r10)	,%%ymm10,%%ymm10	\n\t"\
		"													vaddpd	0x20(%%r10)	,%%ymm13,%%ymm13	\n\t"\
		"													vaddpd	    (%%r11)	,%%ymm12,%%ymm12	\n\t"\
		"													vaddpd	0x20(%%r11)	,%%ymm15,%%ymm15	\n\t"\
		"													vmovaps	%%ymm10,    (%%r10)	\n\t"/* [o4].re */\
		"													vmovaps	%%ymm13,0x20(%%r10)	\n\t"/* [o4].im */\
		"													vmovaps	%%ymm14,    (%%r11)	\n\t"/* [o6].re */\
		"													vmovaps	%%ymm11,0x20(%%r11)	\n\t"/* [o6].im */\
		"													vmovaps	%%ymm8 ,    (%%r12)	\n\t"/* [o5].re */\
		"													vmovaps	%%ymm9 ,0x20(%%r12)	\n\t"/* [o5].im */\
		"													vmovaps	%%ymm12,    (%%r13)	\n\t"/* [o7].re */\
		"													vmovaps	%%ymm15,0x20(%%r13)	\n\t"/* [o7].im */\
		:					/* outputs: none */\
		: [i0] "m" (Xi0)	/* All inputs from memory addresses here */\
		 ,[i1] "m" (Xi1)\
		 ,[i2] "m" (Xi2)\
		 ,[i3] "m" (Xi3)\
		 ,[i4] "m" (Xi4)\
		 ,[i5] "m" (Xi5)\
		 ,[i6] "m" (Xi6)\
		 ,[i7] "m" (Xi7)\
		 ,[o0] "m" (Xo0)\
		 ,[o1] "m" (Xo1)\
		 ,[o2] "m" (Xo2)\
		 ,[o3] "m" (Xo3)\
		 ,[o4] "m" (Xo4)\
		 ,[o5] "m" (Xo5)\
		 ,[o6] "m" (Xo6)\
		 ,[o7] "m" (Xo7)\
		 ,[c4] "m" (Xc1),[s4] "m" (Xs1)\
		 ,[c2] "m" (Xc2),[s2] "m" (Xs2)\
		 ,[c6] "m" (Xc3),[s6] "m" (Xs3)\
		 ,[c1] "m" (Xc4),[s1] "m" (Xs4)\
		 ,[c5] "m" (Xc5),[s5] "m" (Xs5)\
		 ,[c3] "m" (Xc6),[s3] "m" (Xs6)\
		 ,[c7] "m" (Xc7),[s7] "m" (Xs7)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","rdi","r10","r11","r12","r13","r14","r15","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"	/* Clobbered registers */\
	);\
	}

	/* Twiddleless version of SSE2_RADIX8_DIT_TWIDDLE. Inputs enter in memory locations __i0,__i1,__i2,__i3,__i4,__i5,__i6,__i7.
	Outputs go into 16 contiguous 32-byte memory locations starting at __out and assumed disjoint with inputs.
	This macro built on the same code template as SSE2_RADIX8_DIF_TWIDDLE0, but with the I/O-location indices mutually bit reversed:
	01234567 <--> 04261537, which can be effected via the pairwise swaps 1 <--> 4 and 3 <--> 6.
	*/
	#define	SSE2_RADIX8_DIT_0TWIDDLE(Xi0,Xi1,Xi2,Xi3,Xi4,Xi5,Xi6,Xi7, Xout, Xisrt2)\
	{\
	__asm__ volatile (\
		"/* 1st of 2 radix-4 subtransforms, data in ymm0-7: */\n\t	/* 2nd of 2 radix-4 subtransforms, data in ymm8-15: */\n\t"\
		"movq	%[__i0],%%rax					\n\t		movq	%[__i4],%%r10					\n\t"\
		"movq	%[__i1],%%rbx					\n\t		movq	%[__i5],%%r11					\n\t"\
		"movq	%[__i2],%%rcx					\n\t		movq	%[__i6],%%r12					\n\t"\
		"movq	%[__i3],%%rdx					\n\t		movq	%[__i7],%%r13					\n\t"\
		"										\n\t		/* p1,5 combo: x+-y into ymm8/1, 10/3, resp: */	\n\t"\
		"/* p0,4 combo: x+-y into ymm0/1, 2/3, resp: */\n\tvmovaps	     (%%r11),%%ymm8 			\n\t"\
		"										\n\t		vmovaps	0x020(%%r11),%%ymm9 			\n\t"\
		"vmovaps	     (%%rbx),%%ymm0			\n\t		vmovaps	     (%%r10),%%ymm10			\n\t"\
		"vmovaps	0x020(%%rbx),%%ymm1			\n\t		vmovaps	0x020(%%r10),%%ymm11			\n\t"\
		"vmovaps	     (%%rax),%%ymm2			\n\t		vsubpd	%%ymm8 ,%%ymm10,%%ymm10			\n\t"\
		"vmovaps	0x020(%%rax),%%ymm3			\n\t		vsubpd	%%ymm9 ,%%ymm11,%%ymm11			\n\t"\
		"vsubpd	%%ymm0,%%ymm2,%%ymm2			\n\t		vaddpd	%%ymm8 ,%%ymm8 ,%%ymm8 			\n\t"\
		"vsubpd	%%ymm1,%%ymm3,%%ymm3			\n\t		vaddpd	%%ymm9 ,%%ymm9 ,%%ymm9 			\n\t"\
		"vaddpd	%%ymm0,%%ymm0,%%ymm0			\n\t		vaddpd	%%ymm10,%%ymm8 ,%%ymm8 			\n\t"\
		"vaddpd	%%ymm1,%%ymm1,%%ymm1			\n\t		vaddpd	%%ymm11,%%ymm9 ,%%ymm9 			\n\t"\
		"vaddpd	%%ymm2,%%ymm0,%%ymm0			\n\t		/* p3,7 combo: x+-y into ymm14/7, 12/5, resp: */	\n\t"\
		"vaddpd	%%ymm3,%%ymm1,%%ymm1			\n\t		vmovaps	     (%%r12),%%ymm12			\n\t"\
		"										\n\t		vmovaps	0x020(%%r12),%%ymm13			\n\t"\
		"/* p2,6 combo: x+-y into ymm4/5, 6/7, resp: */\n\t	vmovaps	     (%%r13),%%ymm14			\n\t"\
		"										\n\t		vmovaps	0x020(%%r13),%%ymm15			\n\t"\
		"vmovaps	     (%%rdx),%%ymm4			\n\t		vsubpd	%%ymm14,%%ymm12,%%ymm12			\n\t"\
		"vmovaps	0x020(%%rdx),%%ymm5			\n\t		vsubpd	%%ymm15,%%ymm13,%%ymm13			\n\t"\
		"vmovaps	     (%%rcx),%%ymm6			\n\t		vaddpd	%%ymm14,%%ymm14,%%ymm14			\n\t"\
		"vmovaps	0x020(%%rcx),%%ymm7			\n\t		vaddpd	%%ymm15,%%ymm15,%%ymm15			\n\t"\
		"vsubpd	%%ymm4,%%ymm6,%%ymm6			\n\t		vaddpd	%%ymm12,%%ymm14,%%ymm14			\n\t"\
		"vsubpd	%%ymm5,%%ymm7,%%ymm7			\n\t		vaddpd	%%ymm13,%%ymm15,%%ymm15			\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4			\n\t		/* Finish radix-4 butterfly, tmp-store 1st of 4 outputs to free up 2 registers: */\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5			\n\t		vsubpd	%%ymm14,%%ymm8 ,%%ymm8 			\n\t"\
		"vaddpd	%%ymm6,%%ymm4,%%ymm4			\n\t		vsubpd	%%ymm15,%%ymm9 ,%%ymm9 			\n\t"\
		"vaddpd	%%ymm7,%%ymm5,%%ymm5			\n\t		vsubpd	%%ymm13,%%ymm10,%%ymm10			\n\t"\
		"										\n\t		vsubpd	%%ymm12,%%ymm11,%%ymm11			\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0			\n\t		vaddpd	%%ymm14,%%ymm14,%%ymm14			\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2			\n\t		vaddpd	%%ymm13,%%ymm13,%%ymm13			\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1			\n\t		vaddpd	%%ymm15,%%ymm15,%%ymm15			\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3			\n\t		vaddpd	%%ymm12,%%ymm12,%%ymm12			\n\t"\
		"													vaddpd	%%ymm8 ,%%ymm14,%%ymm14			\n\t"\
		"													vaddpd	%%ymm10,%%ymm13,%%ymm13			\n\t"\
		"													vaddpd	%%ymm9 ,%%ymm15,%%ymm15			\n\t"\
		"													vaddpd	%%ymm11,%%ymm12,%%ymm12			\n\t"\
		"													movq	%[__isrt2],%%rsi	/* isrt2 */	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4			\n\t		vmovaps	%%ymm14,     (%%rax)	/* spill*/	\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7			\n\t		vmovaps	%%ymm15,0x020(%%rax)	/* spill*/	\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5			\n\t		vmovaps	%%ymm10,%%ymm14					\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6			\n\t		vmovaps	%%ymm13,%%ymm15					\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4			\n\t		vsubpd	%%ymm12,%%ymm10,%%ymm10			\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7			\n\t		vsubpd	%%ymm11,%%ymm13,%%ymm13			\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5			\n\t		vaddpd	%%ymm14,%%ymm12,%%ymm12			\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6			\n\t		vaddpd	%%ymm15,%%ymm11,%%ymm11			\n\t"\
		"													vmovaps	(%%rsi),%%ymm14		/* isrt2 */	\n\t"\
		"													vmulpd	%%ymm14,%%ymm10,%%ymm10			\n\t"\
		"													vmulpd	%%ymm14,%%ymm13,%%ymm13			\n\t"\
		"													vmulpd	%%ymm14,%%ymm12,%%ymm12			\n\t"\
		"													vmulpd	%%ymm14,%%ymm11,%%ymm11			\n\t"\
		"/* Combine radix-4 subtransforms and write outputs: */\n\t"\
		"\n\t"\
		"vmovaps      (%%rax),%%ymm14/* reload spill */\n\t	vsubpd   %%ymm10,%%ymm2	,%%ymm2			\n\t"\
		"vmovaps 0x020(%%rax),%%ymm15/* reload spill */\n\t	vsubpd   %%ymm12,%%ymm6	,%%ymm6			\n\t"\
		"													vaddpd   %%ymm10,%%ymm10,%%ymm10		\n\t"\
		"movq	%[__out],%%rax					\n\t		vaddpd   %%ymm12,%%ymm12,%%ymm12		\n\t"\
		"										\n\t		vaddpd   %%ymm2 ,%%ymm10,%%ymm10		\n\t"\
		"vsubpd   %%ymm11,%%ymm7 ,%%ymm7 		\n\t		vaddpd   %%ymm6 ,%%ymm12,%%ymm12		\n\t"\
		"vsubpd   %%ymm13,%%ymm3 ,%%ymm3 		\n\t"\
		"vaddpd   %%ymm11,%%ymm11,%%ymm11		\n\t		vmovaps	%%ymm2 ,0x140(%%rax)	/* o5r */	\n\t"\
		"vaddpd   %%ymm13,%%ymm13,%%ymm13		\n\t		vmovaps	%%ymm6 ,0x160(%%rax)	/* o5i */	\n\t"\
		"vaddpd   %%ymm7 ,%%ymm11,%%ymm11		\n\t		vmovaps	%%ymm10,0x040(%%rax)	/* o1r */	\n\t"\
		"vaddpd   %%ymm3 ,%%ymm13,%%ymm13		\n\t		vmovaps	%%ymm12,0x060(%%rax)	/* o1i */	\n\t"\
		"										\n\t"\
		"vmovaps	%%ymm7 ,0x0c0(%%rax)	/* o3r */\n\t"\
		"vmovaps	%%ymm3 ,0x1e0(%%rax)	/* o7i */\n\t"\
		"vmovaps	%%ymm11,0x1c0(%%rax)	/* o7r */\n\t"\
		"vmovaps	%%ymm13,0x0e0(%%rax)	/* o3i */\n\t"\
		"										\n\t"\
		"vsubpd	%%ymm14,%%ymm4 ,%%ymm4 			\n\t"\
		"vsubpd	%%ymm15,%%ymm5 ,%%ymm5 			\n\t"\
		"vsubpd	%%ymm9 ,%%ymm0 ,%%ymm0 			\n\t"\
		"vsubpd	%%ymm8 ,%%ymm1 ,%%ymm1 			\n\t"\
		"vaddpd	%%ymm14,%%ymm14,%%ymm14			\n\t		vmovaps	%%ymm4 ,0x100(%%rax)	/* o4r */	\n\t"\
		"vaddpd	%%ymm15,%%ymm15,%%ymm15			\n\t		vmovaps	%%ymm5 ,0x120(%%rax)	/* o4i */	\n\t"\
		"vaddpd	%%ymm9 ,%%ymm9 ,%%ymm9 			\n\t		vmovaps	%%ymm0 ,0x080(%%rax)	/* o2r */	\n\t"\
		"vaddpd	%%ymm8 ,%%ymm8 ,%%ymm8 			\n\t		vmovaps	%%ymm1 ,0x1a0(%%rax)	/* o6i */	\n\t"\
		"vaddpd	%%ymm4 ,%%ymm14,%%ymm14			\n\t"\
		"vaddpd	%%ymm5 ,%%ymm15,%%ymm15			\n\t"\
		"vaddpd	%%ymm0 ,%%ymm9 ,%%ymm9 			\n\t"\
		"vaddpd	%%ymm1 ,%%ymm8 ,%%ymm8 			\n\t"\
		"										\n\t"\
		"vmovaps	%%ymm14,     (%%rax)	/* o0r */\n\t"\
		"vmovaps	%%ymm15,0x020(%%rax)	/* o0r */\n\t"\
		"vmovaps	%%ymm9 ,0x180(%%rax)	/* o6r */\n\t"\
		"vmovaps	%%ymm8 ,0x0a0(%%rax)	/* o2i */\n\t"\
		:					/* outputs: none */\
		: [__i0] "m" (Xi0)	/* All iputs from memory addresses here */\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__i3] "m" (Xi3)\
		 ,[__i4] "m" (Xi4)\
		 ,[__i5] "m" (Xi5)\
		 ,[__i6] "m" (Xi6)\
		 ,[__i7] "m" (Xi7)\
		 ,[__out] "m" (Xout)\
		 ,[__isrt2] "m" (Xisrt2)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

	// Same as SSE2_RADIX8_DIT_0TWIDDLE but with user-specifiable [i.e. not nec. contiguous] output addresses:
	#define	SSE2_RADIX8_DIT_0TWIDDLE_OOP(Xi0,Xi1,Xi2,Xi3,Xi4,Xi5,Xi6,Xi7, Xo0,Xo1,Xo2,Xo3,Xo4,Xo5,Xo6,Xo7, Xisrt2)\
	{\
	__asm__ volatile (\
		"/* 1st of 2 radix-4 subtransforms, data in ymm0-7: */\n\t	/* 2nd of 2 radix-4 subtransforms, data in ymm8-15: */\n\t"\
		"movq	%[__i0],%%rax					\n\t		movq	%[__i4],%%r10					\n\t"\
		"movq	%[__i1],%%rbx					\n\t		movq	%[__i5],%%r11					\n\t"\
		"movq	%[__i2],%%rcx					\n\t		movq	%[__i6],%%r12					\n\t"\
		"movq	%[__i3],%%rdx					\n\t		movq	%[__i7],%%r13					\n\t"\
		"										\n\t		/* p1,5 combo: x+-y into ymm8/1, 10/3, resp: */	\n\t"\
		"/* p0,4 combo: x+-y into ymm0/1, 2/3, resp: */\n\tvmovaps	     (%%r11),%%ymm8 			\n\t"\
		"										\n\t		vmovaps	0x020(%%r11),%%ymm9 			\n\t"\
		"vmovaps	     (%%rbx),%%ymm0			\n\t		vmovaps	     (%%r10),%%ymm10			\n\t"\
		"vmovaps	0x020(%%rbx),%%ymm1			\n\t		vmovaps	0x020(%%r10),%%ymm11			\n\t"\
		"vmovaps	     (%%rax),%%ymm2			\n\t		vsubpd	%%ymm8 ,%%ymm10,%%ymm10			\n\t"\
		"vmovaps	0x020(%%rax),%%ymm3			\n\t		vsubpd	%%ymm9 ,%%ymm11,%%ymm11			\n\t"\
		"vsubpd	%%ymm0,%%ymm2,%%ymm2			\n\t		vaddpd	%%ymm8 ,%%ymm8 ,%%ymm8 			\n\t"\
		"vsubpd	%%ymm1,%%ymm3,%%ymm3			\n\t		vaddpd	%%ymm9 ,%%ymm9 ,%%ymm9 			\n\t"\
		"vaddpd	%%ymm0,%%ymm0,%%ymm0			\n\t		vaddpd	%%ymm10,%%ymm8 ,%%ymm8 			\n\t"\
		"vaddpd	%%ymm1,%%ymm1,%%ymm1			\n\t		vaddpd	%%ymm11,%%ymm9 ,%%ymm9 			\n\t"\
		"vaddpd	%%ymm2,%%ymm0,%%ymm0			\n\t		/* p3,7 combo: x+-y into ymm14/7, 12/5, resp: */	\n\t"\
		"vaddpd	%%ymm3,%%ymm1,%%ymm1			\n\t		vmovaps	     (%%r12),%%ymm12			\n\t"\
		"										\n\t		vmovaps	0x020(%%r12),%%ymm13			\n\t"\
		"/* p2,6 combo: x+-y into ymm4/5, 6/7, resp: */\n\t	vmovaps	     (%%r13),%%ymm14			\n\t"\
		"										\n\t		vmovaps	0x020(%%r13),%%ymm15			\n\t"\
		"vmovaps	     (%%rdx),%%ymm4			\n\t		vsubpd	%%ymm14,%%ymm12,%%ymm12			\n\t"\
		"vmovaps	0x020(%%rdx),%%ymm5			\n\t		vsubpd	%%ymm15,%%ymm13,%%ymm13			\n\t"\
		"vmovaps	     (%%rcx),%%ymm6			\n\t		vaddpd	%%ymm14,%%ymm14,%%ymm14			\n\t"\
		"vmovaps	0x020(%%rcx),%%ymm7			\n\t		vaddpd	%%ymm15,%%ymm15,%%ymm15			\n\t"\
		"vsubpd	%%ymm4,%%ymm6,%%ymm6			\n\t		vaddpd	%%ymm12,%%ymm14,%%ymm14			\n\t"\
		"vsubpd	%%ymm5,%%ymm7,%%ymm7			\n\t		vaddpd	%%ymm13,%%ymm15,%%ymm15			\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4			\n\t		/* Finish radix-4 butterfly, tmp-store 1st of 4 outputs to free up 2 registers: */\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5			\n\t		vsubpd	%%ymm14,%%ymm8 ,%%ymm8 			\n\t"\
		"vaddpd	%%ymm6,%%ymm4,%%ymm4			\n\t		vsubpd	%%ymm15,%%ymm9 ,%%ymm9 			\n\t"\
		"vaddpd	%%ymm7,%%ymm5,%%ymm5			\n\t		vsubpd	%%ymm13,%%ymm10,%%ymm10			\n\t"\
		"										\n\t		vsubpd	%%ymm12,%%ymm11,%%ymm11			\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0			\n\t		vaddpd	%%ymm14,%%ymm14,%%ymm14			\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2			\n\t		vaddpd	%%ymm13,%%ymm13,%%ymm13			\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1			\n\t		vaddpd	%%ymm15,%%ymm15,%%ymm15			\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3			\n\t		vaddpd	%%ymm12,%%ymm12,%%ymm12			\n\t"\
		"													vaddpd	%%ymm8 ,%%ymm14,%%ymm14			\n\t"\
		"													vaddpd	%%ymm10,%%ymm13,%%ymm13			\n\t"\
		"													vaddpd	%%ymm9 ,%%ymm15,%%ymm15			\n\t"\
		"													vaddpd	%%ymm11,%%ymm12,%%ymm12			\n\t"\
		"													movq	%[__isrt2],%%rsi	/* isrt2 */	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4			\n\t		vmovaps	%%ymm14,     (%%rax)	/* spill*/	\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7			\n\t		vmovaps	%%ymm15,0x020(%%rax)	/* spill*/	\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5			\n\t		vmovaps	%%ymm10,%%ymm14					\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6			\n\t		vmovaps	%%ymm13,%%ymm15					\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4			\n\t		vsubpd	%%ymm12,%%ymm10,%%ymm10			\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7			\n\t		vsubpd	%%ymm11,%%ymm13,%%ymm13			\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5			\n\t		vaddpd	%%ymm14,%%ymm12,%%ymm12			\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6			\n\t		vaddpd	%%ymm15,%%ymm11,%%ymm11			\n\t"\
		"													vmovaps	(%%rsi),%%ymm14		/* isrt2 */	\n\t"\
		"													vmulpd	%%ymm14,%%ymm10,%%ymm10			\n\t"\
		"													vmulpd	%%ymm14,%%ymm13,%%ymm13			\n\t"\
		"													vmulpd	%%ymm14,%%ymm12,%%ymm12			\n\t"\
		"													vmulpd	%%ymm14,%%ymm11,%%ymm11			\n\t"\
		"/* Combine radix-4 subtransforms and write outputs: */\n\t"\
		"\n\t"\
		"vmovaps      (%%rax),%%ymm14/* reload spill */\n\t	vsubpd   %%ymm10,%%ymm2	,%%ymm2			\n\t"\
		"vmovaps 0x020(%%rax),%%ymm15/* reload spill */\n\t	vsubpd   %%ymm12,%%ymm6	,%%ymm6			\n\t"\
		"movq	%[__o1],%%rax					\n\t		movq	%[__o5],%%rcx					\n\t"\
		"													vaddpd   %%ymm10,%%ymm10,%%ymm10		\n\t"\
		"													vaddpd   %%ymm12,%%ymm12,%%ymm12		\n\t"\
		"										\n\t		vaddpd   %%ymm2 ,%%ymm10,%%ymm10		\n\t"\
		"vsubpd   %%ymm11,%%ymm7 ,%%ymm7 		\n\t		vaddpd   %%ymm6 ,%%ymm12,%%ymm12		\n\t"\
		"vsubpd   %%ymm13,%%ymm3 ,%%ymm3 		\n\t"\
		"movq	%[__o3],%%rbx					\n\t		movq	%[__o7],%%rdx					\n\t"\
		"vaddpd   %%ymm11,%%ymm11,%%ymm11		\n\t		vmovaps	%%ymm2 ,    (%%rcx)	/* o5r */	\n\t"\
		"vaddpd   %%ymm13,%%ymm13,%%ymm13		\n\t		vmovaps	%%ymm6 ,0x20(%%rcx)	/* o5i */	\n\t"\
		"vaddpd   %%ymm7 ,%%ymm11,%%ymm11		\n\t		vmovaps	%%ymm10,    (%%rax)	/* o1r */	\n\t"\
		"vaddpd   %%ymm3 ,%%ymm13,%%ymm13		\n\t		vmovaps	%%ymm12,0x20(%%rax)	/* o1i */	\n\t"\
		"movq	%[__o0],%%rax					\n\t		movq	%[__o4],%%rcx					\n\t"\
		"										\n\t"\
		"vmovaps	%%ymm7 ,    (%%rbx)	/* o3r */	\n\t"\
		"vmovaps	%%ymm3 ,0x20(%%rdx)	/* o7i */	\n\t"\
		"vmovaps	%%ymm11,    (%%rdx)	/* o7r */	\n\t"\
		"vmovaps	%%ymm13,0x20(%%rbx)	/* o3i */	\n\t"\
		"										\n\t"\
		"movq	%[__o2],%%rbx					\n\t		movq	%[__o6],%%rdx					\n\t"\
		"vsubpd	%%ymm14,%%ymm4 ,%%ymm4 			\n\t"\
		"vsubpd	%%ymm15,%%ymm5 ,%%ymm5 			\n\t"\
		"vsubpd	%%ymm9 ,%%ymm0 ,%%ymm0 			\n\t"\
		"vsubpd	%%ymm8 ,%%ymm1 ,%%ymm1 			\n\t"\
		"vaddpd	%%ymm14,%%ymm14,%%ymm14			\n\t		vmovaps	%%ymm4 ,    (%%rcx)	/* o4r */	\n\t"\
		"vaddpd	%%ymm15,%%ymm15,%%ymm15			\n\t		vmovaps	%%ymm5 ,0x20(%%rcx)	/* o4i */	\n\t"\
		"vaddpd	%%ymm9 ,%%ymm9 ,%%ymm9 			\n\t		vmovaps	%%ymm0 ,    (%%rbx)	/* o2r */	\n\t"\
		"vaddpd	%%ymm8 ,%%ymm8 ,%%ymm8 			\n\t		vmovaps	%%ymm1 ,0x20(%%rdx)	/* o6i */	\n\t"\
		"vaddpd	%%ymm4 ,%%ymm14,%%ymm14			\n\t"\
		"vaddpd	%%ymm5 ,%%ymm15,%%ymm15			\n\t"\
		"vaddpd	%%ymm0 ,%%ymm9 ,%%ymm9 			\n\t"\
		"vaddpd	%%ymm1 ,%%ymm8 ,%%ymm8 			\n\t"\
		"										\n\t"\
		"vmovaps	%%ymm14,    (%%rax)	/* o0r */	\n\t"\
		"vmovaps	%%ymm15,0x20(%%rax)	/* o0i */	\n\t"\
		"vmovaps	%%ymm9 ,    (%%rdx)	/* o6r */	\n\t"\
		"vmovaps	%%ymm8 ,0x20(%%rbx)	/* o2i */	\n\t"\
		:					/* outputs: none */\
		: [__i0] "m" (Xi0)	/* All iputs from memory addresses here */\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__i3] "m" (Xi3)\
		 ,[__i4] "m" (Xi4)\
		 ,[__i5] "m" (Xi5)\
		 ,[__i6] "m" (Xi6)\
		 ,[__i7] "m" (Xi7)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__o3] "m" (Xo3)\
		 ,[__o4] "m" (Xo4)\
		 ,[__o5] "m" (Xo5)\
		 ,[__o6] "m" (Xo6)\
		 ,[__o7] "m" (Xo7)\
		 ,[__isrt2] "m" (Xisrt2)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

	// AVX Opcount: 84 vec MEM [30 implicit], 66 ADD/SUB, 50 MUL.
	#define SSE2_RADIX8_DIT_TWIDDLE_OOP(Xi0,Xi1,Xi2,Xi3,Xi4,Xi5,Xi6,Xi7, Xo0,Xo1,Xo2,Xo3,Xo4,Xo5,Xo6,Xo7 ,Xc1,Xs1,Xc2,Xs2,Xc3,Xs3,Xc4,Xs4,Xc5,Xs5,Xc6,Xs6,Xc7,Xs7)\
	{\
	__asm__ volatile (																		/* Blocks 2/3 use separate register subset, can be done overlapped with 0/1: */\
	/* Block 0/1 has just one twiddle-CMUL: */											"	movq		%[i2],%%rcx				\n\t"\
	"movq		%[i0],%%rax				\n\t												movq		%[i3],%%rdx				\n\t"\
	"movq		%[i1],%%rbx				\n\t												movq		%[c2],%%r10				\n\t"\
	"movq		%[c1],%%rdi	\n\t/* [rdi,rsi] -> [c,s] components of each sincos pair, */	movq		%[c3],%%r12				\n\t"\
	"movq		%[s1],%%rsi	\n\t/* (not truly a pair here in terms of rel-addresses). */	movq		%[s2],%%r11				\n\t"\
	"vmovaps	    (%%rbx),%%ymm4 		\n\t	vmovaps		0x20(%%rbx),%%ymm5 		\n\t	movq		%[s3],%%r13				\n\t"\
	"vmovaps	    (%%rax),%%ymm0 		\n\t	vmovaps		0x20(%%rax),%%ymm1 		\n\t	vmovaps		(%%rcx),%%ymm8 			\n\t	vmovaps		0x20(%%rcx),%%ymm9 		\n\t"\
	"vmovaps	%%ymm5 ,%%ymm6 			\n\t	vmovaps		%%ymm4 ,%%ymm7 			\n\t	vmovaps	%%ymm9 ,%%ymm10				\n\t	vmovaps		%%ymm8 ,%%ymm11			\n\t"\
	"vmulpd		(%%rdi),%%ymm4 ,%%ymm4 	\n\t	vmulpd		(%%rdi),%%ymm5 ,%%ymm5 	\n\t	vmovaps		(%%rdx),%%ymm12			\n\t	vmovaps		0x20(%%rdx),%%ymm13		\n\t"\
	"vmulpd		(%%rsi),%%ymm6 ,%%ymm6 	\n\t	vmulpd		(%%rsi),%%ymm7 ,%%ymm7 	\n\t	vmovaps	%%ymm13,%%ymm14				\n\t	vmovaps		%%ymm12,%%ymm15			\n\t"\
	"vaddpd		%%ymm6 ,%%ymm4 ,%%ymm4 	\n\t	vsubpd		%%ymm7 ,%%ymm5 ,%%ymm5 	\n\t	vmulpd		(%%r10),%%ymm8 ,%%ymm8 	\n\t	vmulpd		(%%r10),%%ymm9 ,%%ymm9 	\n\t"\
	"vmovaps	%%ymm0 ,%%ymm2 			\n\t	vmovaps		%%ymm1 ,%%ymm3 			\n\t	vmulpd		(%%r12),%%ymm12,%%ymm12	\n\t	vmulpd		(%%r12),%%ymm13,%%ymm13	\n\t"\
	"vaddpd		%%ymm4 ,%%ymm0 ,%%ymm0 	\n\t	vaddpd		%%ymm5 ,%%ymm1 ,%%ymm1 	\n\t	vmulpd		(%%r11),%%ymm10,%%ymm10	\n\t	vmulpd		(%%r11),%%ymm11,%%ymm11	\n\t"\
	"vsubpd		%%ymm4 ,%%ymm2 ,%%ymm2 	\n\t	vsubpd		%%ymm5 ,%%ymm3 ,%%ymm3 	\n\t	vmulpd		(%%r13),%%ymm14,%%ymm14	\n\t	vmulpd		(%%r13),%%ymm15,%%ymm15	\n\t"\
	"vmovaps	%%ymm0 ,    (%%rax)		\n\t	vmovaps		%%ymm1 ,0x20(%%rax)		\n\t	vaddpd		%%ymm10,%%ymm8 ,%%ymm8 	\n\t	vsubpd		%%ymm11,%%ymm9 ,%%ymm9 	\n\t"\
	"vmovaps	%%ymm2 ,    (%%rbx)		\n\t	vmovaps		%%ymm3 ,0x20(%%rbx)		\n\t	vaddpd		%%ymm14,%%ymm12,%%ymm12	\n\t	vsubpd		%%ymm15,%%ymm13,%%ymm13	\n\t"\
																							/* Now do radix-2 butterfly: */\
																						"	vmovaps		%%ymm8 ,%%ymm10			\n\t	vmovaps		%%ymm9 ,%%ymm11			\n\t"\
																						"	vaddpd		%%ymm12,%%ymm8 ,%%ymm8 	\n\t	vaddpd		%%ymm13,%%ymm9 ,%%ymm9 	\n\t"\
																						"	vsubpd		%%ymm12,%%ymm10,%%ymm10	\n\t	vsubpd		%%ymm13,%%ymm11,%%ymm11	\n\t"\
																						"	vmovaps		%%ymm8 ,    (%%rcx)		\n\t	vmovaps		%%ymm9 ,0x20(%%rcx)		\n\t"\
																						"	vmovaps		%%ymm10,    (%%rdx)		\n\t	vmovaps		%%ymm11,0x20(%%rdx)		\n\t"\
	/* Blocks 4/5: */																		/* Blocks 6/7 use separate register subset, can be done overlapped with 4/5: */\
	"movq		%[i4],%%rax				\n\t												movq		%[i6],%%rcx				\n\t"\
	"movq		%[i5],%%rbx				\n\t												movq		%[i7],%%rdx				\n\t"\
	"movq		%[c4],%%rdi				\n\t												movq		%[c6],%%r10				\n\t"\
	"movq		%[c5],%%r8 				\n\t												movq		%[c7],%%r12				\n\t"\
	"movq		%[s4],%%rsi				\n\t												movq		%[s6],%%r11				\n\t"\
	"movq		%[s5],%%r9 				\n\t												movq		%[s7],%%r13				\n\t"\
	"vmovaps		(%%rax),%%ymm0 		\n\t	vmovaps		0x20(%%rax),%%ymm1 		\n\t	vmovaps		(%%rcx),%%ymm8 			\n\t	vmovaps		0x20(%%rcx),%%ymm9 		\n\t"\
	"vmovaps	%%ymm1 ,%%ymm2 			\n\t	vmovaps		%%ymm0 ,%%ymm3 			\n\t	vmovaps		%%ymm9 ,%%ymm10			\n\t	vmovaps		%%ymm8 ,%%ymm11			\n\t"\
	"vmovaps		(%%rbx),%%ymm4 		\n\t	vmovaps		0x20(%%rbx),%%ymm5 		\n\t	vmovaps		(%%rdx),%%ymm12			\n\t	vmovaps		0x20(%%rdx),%%ymm13		\n\t"\
	"vmovaps	%%ymm5 ,%%ymm6 			\n\t	vmovaps		%%ymm4 ,%%ymm7 			\n\t	vmovaps		%%ymm13,%%ymm14			\n\t	vmovaps		%%ymm12,%%ymm15			\n\t"\
	"vmulpd		(%%rdi),%%ymm0 ,%%ymm0 	\n\t	vmulpd		(%%rdi),%%ymm1 ,%%ymm1 	\n\t	vmulpd		(%%r10),%%ymm8 ,%%ymm8 	\n\t	vmulpd		(%%r10),%%ymm9 ,%%ymm9 	\n\t"\
	"vmulpd		(%%r8 ),%%ymm4 ,%%ymm4 	\n\t	vmulpd		(%%r8 ),%%ymm5 ,%%ymm5 	\n\t	vmulpd		(%%r12),%%ymm12,%%ymm12	\n\t	vmulpd		(%%r12),%%ymm13,%%ymm13	\n\t"\
	"vmulpd		(%%r9 ),%%ymm6 ,%%ymm6 	\n\t	vmulpd		(%%r9 ),%%ymm7 ,%%ymm7 	\n\t	vmulpd		(%%r13),%%ymm14,%%ymm14	\n\t	vmulpd		(%%r13),%%ymm15,%%ymm15	\n\t"\
	"vmulpd		(%%rsi),%%ymm2 ,%%ymm2 	\n\t	vmulpd		(%%rsi),%%ymm3 ,%%ymm3 	\n\t	vmulpd		(%%r11),%%ymm10,%%ymm10	\n\t	vmulpd		(%%r11),%%ymm11,%%ymm11	\n\t"\
	"vaddpd		%%ymm2 ,%%ymm0 ,%%ymm0 	\n\t	vsubpd		%%ymm3 ,%%ymm1 ,%%ymm1 	\n\t	vaddpd		%%ymm10,%%ymm8 ,%%ymm8 	\n\t	vsubpd		%%ymm11,%%ymm9 ,%%ymm9 	\n\t"\
	"vaddpd		%%ymm6 ,%%ymm4 ,%%ymm4 	\n\t	vsubpd		%%ymm7 ,%%ymm5 ,%%ymm5 	\n\t	vaddpd		%%ymm14,%%ymm12,%%ymm12	\n\t	vsubpd		%%ymm15,%%ymm13,%%ymm13	\n\t"\
	/* Now do radix-2 butterfly: */\
	"vmovaps	%%ymm0 ,%%ymm2 			\n\t	vmovaps		%%ymm1 ,%%ymm3 			\n\t	vmovaps		%%ymm8 ,%%ymm10			\n\t	vmovaps		%%ymm9 ,%%ymm11			\n\t"\
	"vaddpd		%%ymm4 ,%%ymm0 ,%%ymm0 	\n\t	vaddpd		%%ymm5 ,%%ymm1 ,%%ymm1 	\n\t	vaddpd		%%ymm12,%%ymm8 ,%%ymm8 	\n\t	vaddpd		%%ymm13,%%ymm9 ,%%ymm9 	\n\t"\
	"vsubpd		%%ymm4 ,%%ymm2 ,%%ymm2 	\n\t	vsubpd		%%ymm5 ,%%ymm3 ,%%ymm3 	\n\t	vsubpd		%%ymm12,%%ymm10,%%ymm10	\n\t	vsubpd		%%ymm13,%%ymm11,%%ymm11	\n\t"\
	/* Reload Block 0-3 outputs into r4-7,c-f, combine to get the 2 length-4 subtransform... */\
	"movq		%[i0],%%rax				\n\t"\
	"movq		%[i1],%%rbx				\n\t"\
	"movq		%[i2],%%rcx				\n\t"\
	"movq		%[i3],%%rdx				\n\t"\
	"vmovaps		(%%rax),%%ymm4 		\n\t	vmovaps		0x20(%%rax),%%ymm5 		\n\t"\
	"vmovaps		(%%rbx),%%ymm6 		\n\t	vmovaps		0x20(%%rbx),%%ymm7 		\n\t"\
	"vmovaps		(%%rcx),%%ymm12		\n\t	vmovaps		0x20(%%rcx),%%ymm13		\n\t"\
	"vmovaps		(%%rdx),%%ymm14		\n\t	vmovaps		0x20(%%rdx),%%ymm15		\n\t"\
	"movq		%[o0],%%rax				\n\t"/* Assumes user stuck a (vec_dbl)2.0 into this output slot prior to macro call. */\
	"vsubpd		%%ymm12,%%ymm4 ,%%ymm4 	\n\t	vsubpd		%%ymm13,%%ymm5 ,%%ymm5 	\n\t"\
	"vsubpd		%%ymm15,%%ymm6 ,%%ymm6 	\n\t	vsubpd		%%ymm14,%%ymm7 ,%%ymm7 	\n\t"\
	"vsubpd		%%ymm8 ,%%ymm0 ,%%ymm0 	\n\t	vsubpd		%%ymm9 ,%%ymm1 ,%%ymm1 	\n\t"\
	"vsubpd		%%ymm11,%%ymm2 ,%%ymm2 	\n\t	vsubpd		%%ymm10,%%ymm3 ,%%ymm3 	\n\t"\
	/* We hope the microcode execution engine inlines the MULs with the above SUBs: */\
	"vmovaps	%%ymm10,(%%rdx) 	\n\t"/* spill ymm14 to make room for 2.0 */"	vmovaps	(%%rax),%%ymm10 	\n\t"/* two */\
	"vmulpd		%%ymm10,%%ymm12,%%ymm12	\n\t	vmulpd		%%ymm10,%%ymm13,%%ymm13	\n\t"\
	"vmulpd		%%ymm10,%%ymm15,%%ymm15	\n\t	vmulpd		%%ymm10,%%ymm14,%%ymm14	\n\t"\
	"vmulpd		%%ymm10,%%ymm8 ,%%ymm8 	\n\t	vmulpd		%%ymm10,%%ymm9 ,%%ymm9 	\n\t"\
	"vmulpd		%%ymm10,%%ymm11,%%ymm11	\n\t	vmulpd		(%%rdx),%%ymm10,%%ymm10	\n\t"\
	"vaddpd		%%ymm4 ,%%ymm12,%%ymm12	\n\t	vaddpd		%%ymm5 ,%%ymm13,%%ymm13	\n\t"\
	"vaddpd		%%ymm6 ,%%ymm15,%%ymm15	\n\t	vaddpd		%%ymm7 ,%%ymm14,%%ymm14	\n\t"\
	"vaddpd		%%ymm0 ,%%ymm8 ,%%ymm8 	\n\t	vaddpd		%%ymm1 ,%%ymm9 ,%%ymm9 	\n\t"\
	"vaddpd		%%ymm2 ,%%ymm11,%%ymm11	\n\t	vaddpd		%%ymm3 ,%%ymm10,%%ymm10	\n\t"\
	/* In terms of our original scalar-code prototyping macro, the data are: __tr0 = _r[c,f,4,6,8,b,0,2], __ti0 = _r[d,7,5,e,9,3,1,a]; */\
	/* Now combine the two half-transforms: */\
	/* Need r2/3+- a/b combos for the *ISRT2 preceding the output 4-7 radix-2 butterflies, so start them first: */\
	"vsubpd		%%ymm3 ,%%ymm11,%%ymm11	\n\t	vsubpd		%%ymm10,%%ymm2 ,%%ymm2 	\n\t"\
	"vsubpd		%%ymm8 ,%%ymm12,%%ymm12	\n\t	vsubpd		%%ymm9 ,%%ymm13,%%ymm13	\n\t"\
	"vsubpd		%%ymm1 ,%%ymm4 ,%%ymm4 	\n\t	vsubpd		%%ymm0 ,%%ymm5 ,%%ymm5 	\n\t"\
	"vmovaps	%%ymm0 ,(%%rdx) 	\n\t"/* spill ymm14 to make room for 2.0 */"	vmovaps	(%%rax),%%ymm0  	\n\t"/* two */\
	"vmulpd		%%ymm0 ,%%ymm3 ,%%ymm3 	\n\t	vmulpd		%%ymm0 ,%%ymm10,%%ymm10	\n\t"\
	"vmulpd		%%ymm0 ,%%ymm8 ,%%ymm8 	\n\t	vmulpd		%%ymm0 ,%%ymm9 ,%%ymm9 	\n\t"\
	"vmulpd		%%ymm0 ,%%ymm1 ,%%ymm1 	\n\t	vmulpd		(%%rdx),%%ymm0 ,%%ymm0 	\n\t"\
	"vaddpd		%%ymm11,%%ymm3 ,%%ymm3 	\n\t	vaddpd		%%ymm2 ,%%ymm10,%%ymm10	\n\t"\
	"vaddpd		%%ymm12,%%ymm8 ,%%ymm8 	\n\t	vaddpd		%%ymm13,%%ymm9 ,%%ymm9 	\n\t"\
	"vaddpd		%%ymm4 ,%%ymm1 ,%%ymm1 	\n\t	vaddpd		%%ymm5 ,%%ymm0 ,%%ymm0 	\n\t"\
	/*movq		%[o0],%%rax		[o0] already in rax */	\
	"movq		%[o1],%%rbx				\n\t"\
	"movq		%[o2],%%rcx				\n\t"\
	"movq		%[o3],%%rdx				\n\t"\
	"vmovaps	%%ymm12,    (%%rbx)		\n\t	vmovaps		%%ymm13,0x20(%%rbx)		\n\t"/* __Br1 = _rc;	__Bi1 = _rd; */\
	/* Use that _rc,d free to stick 2.0 into _rc and that [c4] in rdi to load ISRT2 from c4-1 into _rd: */\
	"vmovaps		(%%rax),%%ymm12		\n\t	vmovaps		-0x20(%%rdi),%%ymm13	\n\t"/* _rc = 2.0;		_rd = ISRT2; */\
	"vmovaps	%%ymm4 ,    (%%rdx)		\n\t	vmovaps		%%ymm0 ,0x20(%%rdx)		\n\t"/* __Br3 = _r4;	__Bi3 = _r0; */\
	"vmovaps	%%ymm8 ,    (%%rax)		\n\t	vmovaps		%%ymm9 ,0x20(%%rax)		\n\t"/* __Br0 = _r8;	__Bi0 = _r9; */\
	"vmovaps	%%ymm1 ,    (%%rcx)		\n\t	vmovaps		%%ymm5 ,0x20(%%rcx)		\n\t"/* __Br2 = _r1;	__Bi2 = _r5; */\
	"vmulpd		%%ymm13,%%ymm3 ,%%ymm3 	\n\t	vmulpd		%%ymm13,%%ymm11,%%ymm11	\n\t"\
	"vmulpd		%%ymm13,%%ymm2 ,%%ymm2 	\n\t	vmulpd		%%ymm13,%%ymm10,%%ymm10	\n\t"\
	"vsubpd		%%ymm3 ,%%ymm15,%%ymm15	\n\t	vsubpd		%%ymm11,%%ymm7 ,%%ymm7 	\n\t"\
	"vsubpd		%%ymm2 ,%%ymm6 ,%%ymm6 	\n\t	vsubpd		%%ymm10,%%ymm14,%%ymm14	\n\t"\
	"vmulpd		%%ymm12,%%ymm3 ,%%ymm3 	\n\t	vmulpd		%%ymm12,%%ymm11,%%ymm11	\n\t"\
	"vmulpd		%%ymm12,%%ymm2 ,%%ymm2 	\n\t	vmulpd		%%ymm12,%%ymm10,%%ymm10	\n\t"\
	"vaddpd		%%ymm15,%%ymm3 ,%%ymm3 	\n\t	vaddpd		%%ymm7 ,%%ymm11,%%ymm11	\n\t"\
	"vaddpd		%%ymm6 ,%%ymm2 ,%%ymm2 	\n\t	vaddpd		%%ymm14,%%ymm10,%%ymm10	\n\t"\
	"movq		%[o4],%%rax				\n\t"\
	"movq		%[o5],%%rbx				\n\t"\
	"movq		%[o6],%%rcx				\n\t"\
	"movq		%[o7],%%rdx				\n\t"\
	"vmovaps	%%ymm3 ,    (%%rax)		\n\t	vmovaps		%%ymm7 ,0x20(%%rax)		\n\t"/* __Br4 = _r3;	__Bi4 = _r7; */\
	"vmovaps	%%ymm15,    (%%rbx)		\n\t	vmovaps		%%ymm11,0x20(%%rbx)		\n\t"/* __Br5 = _rf;	__Bi5 = _rb; */\
	"vmovaps	%%ymm6 ,    (%%rcx)		\n\t	vmovaps		%%ymm14,0x20(%%rcx)		\n\t"/* __Br6 = _r6;	__Bi6 = _re; */\
	"vmovaps	%%ymm2 ,    (%%rdx)		\n\t	vmovaps		%%ymm10,0x20(%%rdx)		\n\t"/* __Br7 = _r2;	__Bi7 = _ra; */\
		:					/* outputs: none */\
		: [i0] "m" (Xi0)	/* All inputs from memory addresses here */\
		 ,[i1] "m" (Xi1)\
		 ,[i2] "m" (Xi2)\
		 ,[i3] "m" (Xi3)\
		 ,[i4] "m" (Xi4)\
		 ,[i5] "m" (Xi5)\
		 ,[i6] "m" (Xi6)\
		 ,[i7] "m" (Xi7)\
		 ,[o0] "m" (Xo0)\
		 ,[o1] "m" (Xo1)\
		 ,[o2] "m" (Xo2)\
		 ,[o3] "m" (Xo3)\
		 ,[o4] "m" (Xo4)\
		 ,[o5] "m" (Xo5)\
		 ,[o6] "m" (Xo6)\
		 ,[o7] "m" (Xo7)\
		 ,[c1] "m" (Xc1),[s1] "m" (Xs1)\
		 ,[c2] "m" (Xc2),[s2] "m" (Xs2)\
		 ,[c3] "m" (Xc3),[s3] "m" (Xs3)\
		 ,[c4] "m" (Xc4),[s4] "m" (Xs4)\
		 ,[c5] "m" (Xc5),[s5] "m" (Xs5)\
		 ,[c6] "m" (Xc6),[s6] "m" (Xs6)\
		 ,[c7] "m" (Xc7),[s7] "m" (Xs7)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","rdi","r8","r9","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"	/* Clobbered registers */\
	);\
	}

/*** Prefetch odd-index iaddresses in DIF below, even-index oaddresses in SSE2_RADIX16_DIF_TWIDDLE_OOP ***/

	// Based on the SSE2_RADIX16_DIF_NOTWIDDLE macro in radix16_ditN_cy_dif1_gcc64.h, but with completely
	// specifiable 16-output addressing required for usage as the power-of-2 component of a twiddleless
	// radix = [odd*2^n] DFT routine.
	#define SSE2_RADIX16_DIF_0TWIDDLE(Xin0,Xi1,Xi2,Xi3,Xi4, Xisrt2,Xtwo, Xout0,Xout1,Xout2,Xout3,Xout4,Xout5,Xout6,Xout7,Xout8,Xout9,Xouta,Xoutb,Xoutc,Xoutd,Xoute,Xoutf)\
	{\
	__asm__ volatile (\
		/* SSE2_RADIX4_DIF_IN_PLACE(r1 , r17, r9 , r25): */\
		"movq	%[__in0],%%rax		\n\t"\
		"leaq	%c[__i4](%%rax),%%rcx	\n\t"/* __in0 +   [4*istride]; note BR of [a,b,c,d]-ptrs, i.e. b/c swap */\
		"leaq	%c[__i4](%%rcx),%%rbx	\n\t"/* __in0 + 2*[4*istride] */\
		"leaq	%c[__i4](%%rbx),%%rdx	\n\t"/* __in0 + 3*[4*istride] */\
		"vmovaps	    (%%rax),%%ymm0		\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1		\n\t"\
		"vmovaps	    (%%rax),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3	\n\t"\
		"vaddpd	    (%%rbx),%%ymm0,%%ymm0	\n\t"\
		"vaddpd	0x20(%%rbx),%%ymm1,%%ymm1	\n\t"\
		"vsubpd	    (%%rbx),%%ymm2,%%ymm2	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm3,%%ymm3	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6		\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7		\n\t"\
		"vaddpd	    (%%rdx),%%ymm4,%%ymm4	\n\t"\
		"vaddpd	0x20(%%rdx),%%ymm5,%%ymm5	\n\t"\
		"vsubpd	    (%%rdx),%%ymm6,%%ymm6	\n\t"\
		"vsubpd	0x20(%%rdx),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm0,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm4,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)	\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm2,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rdx)	\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	%%ymm7,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rcx)	\n\t"\
		/* SSE2_RADIX4_DIF_IN_PLACE(r5 , r21, r13, r29): */\
		"leaq	%c[__i2](%%rax),%%rax	\n\t"/* All addresses += 2*ostride */\
		"leaq	%c[__i2](%%rbx),%%rbx	\n\t"\
		"leaq	%c[__i2](%%rcx),%%rcx	\n\t"\
		"leaq	%c[__i2](%%rdx),%%rdx	\n\t"\
		"vmovaps	    (%%rax),%%ymm0		\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1		\n\t"\
		"vmovaps	    (%%rax),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3	\n\t"\
		"vaddpd	    (%%rbx),%%ymm0,%%ymm0	\n\t"\
		"vaddpd	0x20(%%rbx),%%ymm1,%%ymm1	\n\t"\
		"vsubpd	    (%%rbx),%%ymm2,%%ymm2	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm3,%%ymm3	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7	\n\t"\
		"vaddpd	    (%%rdx),%%ymm4,%%ymm4	\n\t"\
		"vaddpd	0x20(%%rdx),%%ymm5,%%ymm5	\n\t"\
		"vsubpd	    (%%rdx),%%ymm6,%%ymm6	\n\t"\
		"vsubpd	0x20(%%rdx),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm0,    (%%rbx)		\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm4,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)	\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm2,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rdx)	\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	%%ymm7,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rcx)	\n\t"\
		/* SSE2_RADIX4_DIF_IN_PLACE(r3 , r19, r11, r27): */\
		"leaq	-%c[__i1](%%rax),%%rax	\n\t"/* All addresses -= 1*ostride */\
		"leaq	-%c[__i1](%%rbx),%%rbx	\n\t"\
		"leaq	-%c[__i1](%%rcx),%%rcx	\n\t"\
		"leaq	-%c[__i1](%%rdx),%%rdx	\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps	    (%%rax),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3	\n\t"\
		"vaddpd	    (%%rbx),%%ymm0,%%ymm0	\n\t"\
		"vaddpd	0x20(%%rbx),%%ymm1,%%ymm1	\n\t"\
		"vsubpd	    (%%rbx),%%ymm2,%%ymm2	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm3,%%ymm3	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7	\n\t"\
		"vaddpd	    (%%rdx),%%ymm4,%%ymm4	\n\t"\
		"vaddpd	0x20(%%rdx),%%ymm5,%%ymm5	\n\t"\
		"vsubpd	    (%%rdx),%%ymm6,%%ymm6	\n\t"\
		"vsubpd	0x20(%%rdx),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0			\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1			\n\t"\
		"vmovaps	%%ymm0,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5			\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4			\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5			\n\t"\
		"vmovaps	%%ymm4,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)	\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm2,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rdx)	\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	%%ymm7,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rcx)	\n\t"\
		/* SSE2_RADIX4_DIF_IN_PLACE(r7 , r23, r15, r31): */\
		"leaq	%c[__i2](%%rax),%%rax	\n\t"/* All addresses += 2*ostride */\
		"leaq	%c[__i2](%%rbx),%%rbx	\n\t"\
		"leaq	%c[__i2](%%rcx),%%rcx	\n\t"\
		"leaq	%c[__i2](%%rdx),%%rdx	\n\t"\
		"vmovaps	    (%%rax),%%ymm0		\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1		\n\t"\
		"vmovaps	    (%%rax),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3	\n\t"\
		"vaddpd	    (%%rbx),%%ymm0,%%ymm0	\n\t"\
		"vaddpd	0x20(%%rbx),%%ymm1,%%ymm1	\n\t"\
		"vsubpd	    (%%rbx),%%ymm2,%%ymm2	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm3,%%ymm3	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7	\n\t"\
		"vaddpd	    (%%rdx),%%ymm4,%%ymm4	\n\t"\
		"vaddpd	0x20(%%rdx),%%ymm5,%%ymm5	\n\t"\
		"vsubpd	    (%%rdx),%%ymm6,%%ymm6	\n\t"\
		"vsubpd	0x20(%%rdx),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm0,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm4,    (%%rax)		\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)		\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm2,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rdx)	\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	%%ymm7,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rcx)	\n\t"\
	/*** Now do 4 DFTs with internal twiddles on the 1*stride - separated data. Do blocks in order 0,2,1,3 to allow increment-only of rsi-datum from 1 block to the next: ***/\
		/* Block 0: r0-3 */\
		"movq	%[__in0],%%rsi	\n\t"\
		"movq	%[__out0],%%rax		\n\t"\
		"movq	%[__out1],%%rbx		\n\t"\
		"movq	%[__out2],%%rcx		\n\t"\
		"movq	%[__out3],%%rdx		\n\t"\
		"leaq	0x20(%%rsi),%%rdi	\n\t"/* Need separate address Im parts of outputs due to literal-offsets below */\
		"vmovaps	        (%%rsi),%%ymm0	\n\t"\
		"vmovaps	        (%%rdi),%%ymm1	\n\t"\
		"vmovaps	%c[__i2](%%rsi),%%ymm2	\n\t"\
		"vmovaps	%c[__i2](%%rdi),%%ymm3	\n\t"\
		"vsubpd	%c[__i2](%%rsi),%%ymm0,%%ymm0	\n\t"\
		"vsubpd	%c[__i2](%%rdi),%%ymm1,%%ymm1	\n\t"\
		"vaddpd	        (%%rsi),%%ymm2,%%ymm2	\n\t"\
		"vaddpd	        (%%rdi),%%ymm3,%%ymm3	\n\t"\
		"vmovaps	%c[__i1](%%rsi),%%ymm4	\n\t"\
		"vmovaps	%c[__i1](%%rdi),%%ymm5	\n\t"\
		"vmovaps	%c[__i3](%%rsi),%%ymm6	\n\t"\
		"vmovaps	%c[__i3](%%rdi),%%ymm7	\n\t"\
		"vsubpd	%c[__i3](%%rsi),%%ymm4,%%ymm4	\n\t"\
		"vsubpd	%c[__i3](%%rdi),%%ymm5,%%ymm5	\n\t"\
		"vaddpd	%c[__i1](%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vaddpd	%c[__i1](%%rdi),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm6,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm7,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	%%ymm2,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rbx)	\n\t"\
		"vaddpd	%%ymm2,%%ymm6,%%ymm6	\n\t"\
		"vaddpd	%%ymm3,%%ymm7,%%ymm7	\n\t"\
		"vmovaps	%%ymm6,    (%%rax)	\n\t"\
		"vmovaps	%%ymm7,0x20(%%rax)	\n\t"\
		"vsubpd	%%ymm5,%%ymm0,%%ymm0	\n\t"\
		"vsubpd	%%ymm4,%%ymm1,%%ymm1	\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4	\n\t"\
		"vmovaps	%%ymm0,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rdx)	\n\t"\
		"vaddpd	%%ymm0,%%ymm5,%%ymm5	\n\t"\
		"vaddpd	%%ymm1,%%ymm4,%%ymm4	\n\t"\
		"vmovaps	%%ymm5,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm4,0x20(%%rcx)	\n\t"\
		/* Block 2: */\
		"movq	%[__out8],%%rax		\n\t	leaq	%c[__i4](%%rsi),%%rsi	\n\t"/* __in0 + 4*ostride */\
		"movq	%[__out9],%%rbx		\n\t"\
		"movq	%[__outa],%%rcx		\n\t"\
		"movq	%[__outb],%%rdx		\n\t"\
		"leaq	0x20(%%rsi),%%rdi	\n\t"\
		"vmovaps	%c[__i1](%%rsi),%%ymm4	\n\t"\
		"vmovaps	%c[__i3](%%rsi),%%ymm6	\n\t"\
		"vmovaps	%c[__i1](%%rdi),%%ymm5	\n\t"\
		"vmovaps	%c[__i3](%%rdi),%%ymm7	\n\t"\
		"vmovaps	%%ymm4,%%ymm0		\n\t"\
		"vmovaps	%%ymm6,%%ymm2		\n\t"\
		"vmovaps	%%ymm5,%%ymm1		\n\t"\
		"vmovaps	%%ymm7,%%ymm3		\n\t"\
		"movq	%[__isrt2],%%rdi	\n\t"\
		"addq	$0x20,%%rdi	\n\t"/* cc0 */\
		"vmulpd	    (%%rdi),%%ymm4,%%ymm4	\n\t"\
		"vmulpd	0x20(%%rdi),%%ymm6,%%ymm6	\n\t"\
		"vmulpd	0x20(%%rdi),%%ymm1,%%ymm1	\n\t"\
		"vmulpd	    (%%rdi),%%ymm3,%%ymm3	\n\t"\
		"vmulpd	    (%%rdi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x20(%%rdi),%%ymm7,%%ymm7	\n\t"\
		"vmulpd	0x20(%%rdi),%%ymm0,%%ymm0	\n\t"\
		"vmulpd	    (%%rdi),%%ymm2,%%ymm2	\n\t"\
		"vsubpd	%%ymm1,%%ymm4,%%ymm4		\n\t"\
		"vsubpd	%%ymm3,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm0,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7		\n\t"\
		"vsubpd	%%ymm6,%%ymm4,%%ymm4		\n\t"\
		"vsubpd	%%ymm7,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm4,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm5,%%ymm7,%%ymm7		\n\t"\
		"leaq	0x20(%%rsi),%%rdi	\n\t"\
		"vmovaps	%c[__i2](%%rsi),%%ymm2	\n\t"\
		"vmovaps	%c[__i2](%%rdi),%%ymm3	\n\t"\
		"vsubpd	%c[__i2](%%rdi),%%ymm2,%%ymm2	\n\t"\
		"vaddpd	%c[__i2](%%rsi),%%ymm3,%%ymm3	\n\t"\
		"movq	%[__isrt2],%%rdi	\n\t"\
		"vmulpd	(%%rdi),%%ymm2,%%ymm2	\n\t"/* mul by isrt2 */\
		"vmulpd	(%%rdi),%%ymm3,%%ymm3	\n\t"\
		"leaq	0x20(%%rsi),%%rdi	\n\t"\
		"vmovaps	        (%%rsi),%%ymm0	\n\t"\
		"vmovaps	        (%%rdi),%%ymm1	\n\t"\
		"vsubpd	%%ymm2,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm3,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	        (%%rsi),%%ymm2,%%ymm2	\n\t"\
		"vaddpd	        (%%rdi),%%ymm3,%%ymm3	\n\t"\
		"vsubpd	%%ymm6,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm7,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	%%ymm2,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rbx)	\n\t"\
		"vaddpd	%%ymm2,%%ymm6,%%ymm6	\n\t"\
		"vaddpd	%%ymm3,%%ymm7,%%ymm7	\n\t"\
		"vmovaps	%%ymm6,    (%%rax)	\n\t"\
		"vmovaps	%%ymm7,0x20(%%rax)	\n\t"\
		"vsubpd	%%ymm5,%%ymm0,%%ymm0	\n\t"\
		"vsubpd	%%ymm4,%%ymm1,%%ymm1	\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4	\n\t"\
		"vmovaps	%%ymm0,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rdx)	\n\t"\
		"vaddpd	%%ymm0,%%ymm5,%%ymm5	\n\t"\
		"vaddpd	%%ymm1,%%ymm4,%%ymm4	\n\t"\
		"vmovaps	%%ymm5,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm4,0x20(%%rcx)	\n\t"\
		/* Block 1: r8-b */\
		"movq	%[__out4],%%rax		\n\t	leaq	%c[__i4](%%rsi),%%rsi	\n\t"/* __in0 + 8*ostride */\
		"movq	%[__out5],%%rbx		\n\t"\
		"movq	%[__out6],%%rcx		\n\t"\
		"movq	%[__out7],%%rdx		\n\t"\
		"leaq	0x20(%%rsi),%%rdi	\n\t"\
		"vmovaps	        (%%rsi),%%ymm0	\n\t"\
		"vmovaps	        (%%rdi),%%ymm1	\n\t"\
		"vmovaps	%c[__i2](%%rsi),%%ymm2	\n\t"\
		"vmovaps	%c[__i2](%%rdi),%%ymm3	\n\t"\
		"vsubpd	%c[__i2](%%rdi),%%ymm0,%%ymm0	\n\t"\
		"vsubpd	%c[__i2](%%rsi),%%ymm1,%%ymm1	\n\t"\
		"vaddpd	        (%%rdi),%%ymm2,%%ymm2	\n\t"\
		"vaddpd	        (%%rsi),%%ymm3,%%ymm3	\n\t"\
		"vmovaps	%c[__i1](%%rsi),%%ymm4	\n\t"\
		"vmovaps	%c[__i1](%%rdi),%%ymm5	\n\t"\
		"vmovaps	%c[__i3](%%rsi),%%ymm6	\n\t"\
		"vmovaps	%c[__i3](%%rdi),%%ymm7	\n\t"\
		"vsubpd	%c[__i1](%%rdi),%%ymm4,%%ymm4	\n\t"\
		"vaddpd	%c[__i1](%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vaddpd	%c[__i3](%%rdi),%%ymm6,%%ymm6	\n\t"\
		"vsubpd	%c[__i3](%%rsi),%%ymm7,%%ymm7	\n\t"\
		"movq	%[__isrt2],%%rdi	\n\t"\
		"vmulpd	(%%rdi),%%ymm4,%%ymm4		\n\t"\
		"vmulpd	(%%rdi),%%ymm5,%%ymm5		\n\t"\
		"vmulpd	(%%rdi),%%ymm6,%%ymm6		\n\t"\
		"vmulpd	(%%rdi),%%ymm7,%%ymm7		\n\t"\
		"vsubpd	%%ymm6,%%ymm4,%%ymm4		\n\t"\
		"vsubpd	%%ymm7,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm4,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm5,%%ymm7,%%ymm7		\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm5,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm0,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm2,0x20(%%rbx)	\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4	\n\t"\
		"vaddpd	%%ymm2,%%ymm5,%%ymm5	\n\t"\
		"vmovaps	%%ymm4,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)	\n\t"\
		"vsubpd	%%ymm7,%%ymm3,%%ymm3	\n\t"\
		"vsubpd	%%ymm6,%%ymm1,%%ymm1	\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6	\n\t"\
		"vmovaps	%%ymm3,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rdx)	\n\t"\
		"vaddpd	%%ymm3,%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm1,%%ymm6,%%ymm6	\n\t"\
		"vmovaps	%%ymm7,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rcx)	\n\t"\
		/* Block 3: */\
		"movq	%[__outc],%%rax		\n\t	leaq	%c[__i4](%%rsi),%%rsi	\n\t"/* __in0 + c*ostride */\
		"movq	%[__outd],%%rbx		\n\t"\
		"movq	%[__oute],%%rcx		\n\t"\
		"movq	%[__outf],%%rdx		\n\t"\
		"leaq	0x20(%%rsi),%%rdi	\n\t"\
		"vmovaps	%c[__i1](%%rsi),%%ymm4	\n\t"\
		"vmovaps	%c[__i3](%%rsi),%%ymm6	\n\t"\
		"vmovaps	%c[__i1](%%rdi),%%ymm5	\n\t"\
		"vmovaps	%c[__i3](%%rdi),%%ymm7	\n\t"\
		"vmovaps	%%ymm4,%%ymm0		\n\t"\
		"vmovaps	%%ymm6,%%ymm2		\n\t"\
		"vmovaps	%%ymm5,%%ymm1		\n\t"\
		"vmovaps	%%ymm7,%%ymm3		\n\t"\
		"movq	%[__isrt2],%%rdi	\n\t"\
		"addq	$0x20,%%rdi	\n\t"/* cc0 */\
		"vmulpd	0x20(%%rdi),%%ymm4,%%ymm4	\n\t"\
		"vmulpd	    (%%rdi),%%ymm6,%%ymm6	\n\t"\
		"vmulpd	    (%%rdi),%%ymm1,%%ymm1	\n\t"\
		"vmulpd	0x20(%%rdi),%%ymm3,%%ymm3	\n\t"\
		"vmulpd	0x20(%%rdi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	    (%%rdi),%%ymm7,%%ymm7	\n\t"\
		"vmulpd	    (%%rdi),%%ymm0,%%ymm0	\n\t"\
		"vmulpd	0x20(%%rdi),%%ymm2,%%ymm2	\n\t"\
		"vsubpd	%%ymm1,%%ymm4,%%ymm4		\n\t"\
		"vsubpd	%%ymm3,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm0,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7		\n\t"\
		"vsubpd	%%ymm6,%%ymm4,%%ymm4		\n\t"\
		"vsubpd	%%ymm7,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm4,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm5,%%ymm7,%%ymm7		\n\t"\
		"leaq	0x20(%%rsi),%%rdi	\n\t"\
		"vmovaps	%c[__i2](%%rsi),%%ymm2	\n\t"\
		"vmovaps	%c[__i2](%%rdi),%%ymm3	\n\t"\
		"vaddpd	%c[__i2](%%rdi),%%ymm2,%%ymm2	\n\t"\
		"vsubpd	%c[__i2](%%rsi),%%ymm3,%%ymm3	\n\t"\
		"movq	%[__isrt2],%%rdi	\n\t"\
		"vmulpd	(%%rdi),%%ymm2,%%ymm2	\n\t"/* mul by isrt2 */\
		"vmulpd	(%%rdi),%%ymm3,%%ymm3	\n\t"\
		"leaq	0x20(%%rsi),%%rdi	\n\t"\
		"vmovaps	        (%%rsi),%%ymm0	\n\t"\
		"vmovaps	        (%%rdi),%%ymm1	\n\t"\
		"vsubpd	%%ymm2,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm3,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	        (%%rsi),%%ymm2,%%ymm2	\n\t"\
		"vaddpd	        (%%rdi),%%ymm3,%%ymm3	\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm0,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)	\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4	\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5	\n\t"\
		"vmovaps	%%ymm4,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)	\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2	\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3	\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6	\n\t"\
		"vmovaps	%%ymm2,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rdx)	\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6	\n\t"\
		"vmovaps	%%ymm7,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rcx)	\n\t"\
		:					/* outputs: none */\
		:[__in0] "m" (Xin0)	/* All inputs from memory addresses here */\
		,[__i1] "e" (Xi1)\
		,[__i2] "e" (Xi2)\
		,[__i3] "e" (Xi3)\
		,[__i4] "e" (Xi4)\
		,[__isrt2] "m" (Xisrt2)\
		,[__two] "m" (Xtwo)\
		,[__out0] "m" (Xout0)\
		,[__out1] "m" (Xout1)\
		,[__out2] "m" (Xout2)\
		,[__out3] "m" (Xout3)\
		,[__out4] "m" (Xout4)\
		,[__out5] "m" (Xout5)\
		,[__out6] "m" (Xout6)\
		,[__out7] "m" (Xout7)\
		,[__out8] "m" (Xout8)\
		,[__out9] "m" (Xout9)\
		,[__outa] "m" (Xouta)\
		,[__outb] "m" (Xoutb)\
		,[__outc] "m" (Xoutc)\
		,[__outd] "m" (Xoutd)\
		,[__oute] "m" (Xoute)\
		,[__outf] "m" (Xoutf)\
		: "cc","memory","rax","rbx","rcx","rdx","rdi","rsi","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

	// Same as above, but with specifiable I-addresses and regularly spaced O-addresses:

   #if 0	// 16-register version: This proves slower than 8-reg in both my SSE2/Core2 ad AVX/Haswell tests:

	#define SSE2_RADIX16_DIF_0TWIDDLE_B(Xin0,Xin1,Xin2,Xin3,Xin4,Xin5,Xin6,Xin7,Xin8,Xin9,Xina,Xinb,Xinc,Xind,Xine,Xinf, Xisrt2,Xtwo, Xout0,Xout1,Xout2,Xout3)\
	{\
	__asm__ volatile (\
	/* SSE2_RADIX4_DIF_IN_PLACE(r1,r17,r9,r25): */	/* SSE2_RADIX4_DIF_IN_PLACE(r3,r19,r11,r27): */\
		"movq	%[__in0],%%rax				\n\t		movq	%[__in1],%%r10		\n\t"\
		"movq	%[__in8],%%rbx				\n\t		movq	%[__in9],%%r11		\n\t"\
		"movq	%[__in4],%%rcx				\n\t		movq	%[__in5],%%r12		\n\t"\
		"movq	%[__inc],%%rdx				\n\t		movq	%[__ind],%%r13		\n\t"\
		"vmovaps	    (%%rax),%%ymm0		\n\t		vmovaps	    (%%r10),%%ymm8	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1		\n\t		vmovaps	0x20(%%r10),%%ymm9	\n\t"\
		"vmovaps	    (%%rax),%%ymm2		\n\t		vmovaps	    (%%r10),%%ymm10	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3		\n\t		vmovaps	0x20(%%r10),%%ymm11	\n\t"\
		"vaddpd	    (%%rbx),%%ymm0,%%ymm0	\n\t		vaddpd	    (%%r11),%%ymm8 ,%%ymm8	\n\t"\
		"vaddpd	0x20(%%rbx),%%ymm1,%%ymm1	\n\t		vaddpd	0x20(%%r11),%%ymm9 ,%%ymm9	\n\t"\
		"vsubpd	    (%%rbx),%%ymm2,%%ymm2	\n\t		vsubpd	    (%%r11),%%ymm10,%%ymm10	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm3,%%ymm3	\n\t		vsubpd	0x20(%%r11),%%ymm11,%%ymm11	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4		\n\t		vmovaps	    (%%r12),%%ymm12	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5		\n\t		vmovaps	0x20(%%r12),%%ymm13	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6		\n\t		vmovaps	    (%%r12),%%ymm14	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7		\n\t		vmovaps	0x20(%%r12),%%ymm15	\n\t"\
		"vaddpd	    (%%rdx),%%ymm4,%%ymm4	\n\t		vaddpd	    (%%r13),%%ymm12,%%ymm12	\n\t"\
		"vaddpd	0x20(%%rdx),%%ymm5,%%ymm5	\n\t		vaddpd	0x20(%%r13),%%ymm13,%%ymm13	\n\t"\
		"vsubpd	    (%%rdx),%%ymm6,%%ymm6	\n\t		vsubpd	    (%%r13),%%ymm14,%%ymm14	\n\t"\
		"vsubpd	0x20(%%rdx),%%ymm7,%%ymm7	\n\t		vsubpd	0x20(%%r13),%%ymm15,%%ymm15	\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t		vsubpd	%%ymm12,%%ymm8,%%ymm8		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1		\n\t		vsubpd	%%ymm13,%%ymm9,%%ymm9		\n\t"\
		"vmovaps	%%ymm0,    (%%rbx)		\n\t		vmovaps	%%ymm8,    (%%r11)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)		\n\t		vmovaps	%%ymm9,0x20(%%r11)	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t		vaddpd	%%ymm12,%%ymm12,%%ymm12		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t		vaddpd	%%ymm13,%%ymm13,%%ymm13		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t		vaddpd	%%ymm8 ,%%ymm12,%%ymm12		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t		vaddpd	%%ymm9 ,%%ymm13,%%ymm13		\n\t"\
		"vmovaps	%%ymm4,    (%%rax)		\n\t		vmovaps	%%ymm12,    (%%r10)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)		\n\t		vmovaps	%%ymm13,0x20(%%r10)	\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2		\n\t		vsubpd	%%ymm15,%%ymm10,%%ymm10		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3		\n\t		vsubpd	%%ymm14,%%ymm11,%%ymm11		\n\t"\
		"vmovaps	%%ymm2,    (%%rcx)		\n\t		vmovaps	%%ymm10,    (%%r12)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rdx)		\n\t		vmovaps	%%ymm11,0x20(%%r13)	\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t		vaddpd	%%ymm15,%%ymm15,%%ymm15		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t		vaddpd	%%ymm14,%%ymm14,%%ymm14		\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7		\n\t		vaddpd	%%ymm10,%%ymm15,%%ymm15		\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6		\n\t		vaddpd	%%ymm11,%%ymm14,%%ymm14		\n\t"\
		"vmovaps	%%ymm7,    (%%rdx)		\n\t		vmovaps	%%ymm15,    (%%r13)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rcx)		\n\t		vmovaps	%%ymm14,0x20(%%r12)	\n\t"\
	/* SSE2_RADIX4_DIF_IN_PLACE(r5,r21,r13,r29): */	/* SSE2_RADIX4_DIF_IN_PLACE(r7,r23,r15,r31): */\
		"movq	%[__in2],%%rax				\n\t		movq	%[__in3],%%r10		\n\t"\
		"movq	%[__ina],%%rbx				\n\t		movq	%[__inb],%%r11		\n\t"\
		"movq	%[__in6],%%rcx				\n\t		movq	%[__in7],%%r12		\n\t"\
		"movq	%[__ine],%%rdx				\n\t		movq	%[__inf],%%r13		\n\t"\
		"vmovaps	    (%%rax),%%ymm0		\n\t		vmovaps	    (%%r10),%%ymm8	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1		\n\t		vmovaps	0x20(%%r10),%%ymm9	\n\t"\
		"vmovaps	    (%%rax),%%ymm2		\n\t		vmovaps	    (%%r10),%%ymm10	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3		\n\t		vmovaps	0x20(%%r10),%%ymm11	\n\t"\
		"vaddpd	    (%%rbx),%%ymm0,%%ymm0	\n\t		vaddpd	    (%%r11),%%ymm8 ,%%ymm8	\n\t"\
		"vaddpd	0x20(%%rbx),%%ymm1,%%ymm1	\n\t		vaddpd	0x20(%%r11),%%ymm9 ,%%ymm9	\n\t"\
		"vsubpd	    (%%rbx),%%ymm2,%%ymm2	\n\t		vsubpd	    (%%r11),%%ymm10,%%ymm10	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm3,%%ymm3	\n\t		vsubpd	0x20(%%r11),%%ymm11,%%ymm11	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4		\n\t		vmovaps	    (%%r12),%%ymm12	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5		\n\t		vmovaps	0x20(%%r12),%%ymm13	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6		\n\t		vmovaps	    (%%r12),%%ymm14	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7		\n\t		vmovaps	0x20(%%r12),%%ymm15	\n\t"\
		"vaddpd	    (%%rdx),%%ymm4,%%ymm4	\n\t		vaddpd	    (%%r13),%%ymm12,%%ymm12	\n\t"\
		"vaddpd	0x20(%%rdx),%%ymm5,%%ymm5	\n\t		vaddpd	0x20(%%r13),%%ymm13,%%ymm13	\n\t"\
		"vsubpd	    (%%rdx),%%ymm6,%%ymm6	\n\t		vsubpd	    (%%r13),%%ymm14,%%ymm14	\n\t"\
		"vsubpd	0x20(%%rdx),%%ymm7,%%ymm7	\n\t		vsubpd	0x20(%%r13),%%ymm15,%%ymm15	\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t		vsubpd	%%ymm12,%%ymm8,%%ymm8		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1		\n\t		vsubpd	%%ymm13,%%ymm9,%%ymm9		\n\t"\
		"vmovaps	%%ymm0,    (%%rbx)		\n\t		vmovaps	%%ymm8,    (%%r11)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)		\n\t		vmovaps	%%ymm9,0x20(%%r11)	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t		vaddpd	%%ymm12,%%ymm12,%%ymm12		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t		vaddpd	%%ymm13,%%ymm13,%%ymm13		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t		vaddpd	%%ymm8 ,%%ymm12,%%ymm12		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t		vaddpd	%%ymm9 ,%%ymm13,%%ymm13		\n\t"\
		"vmovaps	%%ymm4,    (%%rax)		\n\t		vmovaps	%%ymm12,    (%%r10)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)		\n\t		vmovaps	%%ymm13,0x20(%%r10)	\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2		\n\t		vsubpd	%%ymm15,%%ymm10,%%ymm10		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3		\n\t		vsubpd	%%ymm14,%%ymm11,%%ymm11		\n\t"\
		"vmovaps	%%ymm2,    (%%rcx)		\n\t		vmovaps	%%ymm10,    (%%r12)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rdx)		\n\t		vmovaps	%%ymm11,0x20(%%r13)	\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t		vaddpd	%%ymm15,%%ymm15,%%ymm15		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t		vaddpd	%%ymm14,%%ymm14,%%ymm14		\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7		\n\t		vaddpd	%%ymm10,%%ymm15,%%ymm15		\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6		\n\t		vaddpd	%%ymm11,%%ymm14,%%ymm14		\n\t"\
		"vmovaps	%%ymm7,    (%%rdx)		\n\t		vmovaps	%%ymm15,    (%%r13)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rcx)		\n\t		vmovaps	%%ymm14,0x20(%%r12)	\n\t"\
	/*** Now do 4 DFTs with internal twiddles on the 1*stride - separated data. Do blocks in order 0,2,1,3 to allow increment-only of rsi-datum from 1 block to the next: ***/\
		"movq	%[__isrt2],%%rdi			\n\t"\
		/* Block 0: r0-3 */								/* Block 1: r8-b */\
		"movq	%[__out0],%%rsi				\n\t		movq	%[__out1],%%r8	\n\t"\
		"movq	%[__in0],%%rax				\n\t		movq	%[__in8],%%r10		\n\t"\
		"movq	%[__in2],%%rbx				\n\t		movq	%[__ina],%%r11		\n\t"\
		"movq	%[__in1],%%rcx				\n\t		movq	%[__in9],%%r12		\n\t"\
		"movq	%[__in3],%%rdx				\n\t		movq	%[__inb],%%r13		\n\t"\
		"vmovaps	    (%%rax),%%ymm0		\n\t		vmovaps	    (%%r10),%%ymm8	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1		\n\t		vmovaps	0x20(%%r10),%%ymm9	\n\t"\
		"vmovaps		(%%rbx),%%ymm2		\n\t		vmovaps		(%%r11),%%ymm10	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3		\n\t		vmovaps	0x20(%%r11),%%ymm11	\n\t"\
		"vsubpd		(%%rbx),%%ymm0,%%ymm0	\n\t		vsubpd	0x20(%%r11),%%ymm8 ,%%ymm8	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm1,%%ymm1	\n\t		vsubpd		(%%r11),%%ymm9 ,%%ymm9	\n\t"\
		"vaddpd	    (%%rax),%%ymm2,%%ymm2	\n\t		vaddpd	0x20(%%r10),%%ymm10,%%ymm10	\n\t"\
		"vaddpd	0x20(%%rax),%%ymm3,%%ymm3	\n\t		vaddpd		(%%r10),%%ymm11,%%ymm11	\n\t"\
		"vmovaps		(%%rcx),%%ymm4		\n\t		vmovaps		(%%r12),%%ymm12	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5		\n\t		vmovaps	0x20(%%r12),%%ymm13	\n\t"\
		"vmovaps	    (%%rdx),%%ymm6		\n\t		vmovaps	    (%%r13),%%ymm14	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7		\n\t		vmovaps	0x20(%%r13),%%ymm15	\n\t"\
		"vsubpd		(%%rdx),%%ymm4,%%ymm4	\n\t		vsubpd	0x20(%%r12),%%ymm12,%%ymm12	\n\t"\
		"vsubpd	0x20(%%rdx),%%ymm5,%%ymm5	\n\t		vaddpd		(%%r12),%%ymm13,%%ymm13	\n\t"\
		"vaddpd	    (%%rcx),%%ymm6,%%ymm6	\n\t		vaddpd	0x20(%%r13),%%ymm14,%%ymm14	\n\t"\
		"vaddpd	0x20(%%rcx),%%ymm7,%%ymm7	\n\t		vsubpd		(%%r13),%%ymm15,%%ymm15	\n\t"\
		"vsubpd	%%ymm6,%%ymm2,%%ymm2		\n\t		vmulpd	(%%rdi),%%ymm12,%%ymm12		\n\t"\
		"vsubpd	%%ymm7,%%ymm3,%%ymm3		\n\t		vmulpd	(%%rdi),%%ymm13,%%ymm13		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t		vmulpd	(%%rdi),%%ymm14,%%ymm14		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t		vmulpd	(%%rdi),%%ymm15,%%ymm15		\n\t"\
		"vmovaps	%%ymm2,0x40(%%rsi)		\n\t		vsubpd	%%ymm14,%%ymm12,%%ymm12		\n\t"\
		"vmovaps	%%ymm3,0x60(%%rsi)		\n\t		vsubpd	%%ymm15,%%ymm13,%%ymm13		\n\t"\
		"vaddpd	%%ymm2,%%ymm6,%%ymm6		\n\t		vaddpd	%%ymm14,%%ymm14,%%ymm14		\n\t"\
		"vaddpd	%%ymm3,%%ymm7,%%ymm7		\n\t		vaddpd	%%ymm15,%%ymm15,%%ymm15		\n\t"\
		"vmovaps	%%ymm6,    (%%rsi)		\n\t		vaddpd	%%ymm12,%%ymm14,%%ymm14		\n\t"\
		"vmovaps	%%ymm7,0x20(%%rsi)		\n\t		vaddpd	%%ymm13,%%ymm15,%%ymm15		\n\t"\
		"vsubpd	%%ymm5,%%ymm0,%%ymm0		\n\t		vsubpd	%%ymm12,%%ymm8 ,%%ymm8		\n\t"\
		"vsubpd	%%ymm4,%%ymm1,%%ymm1		\n\t		vsubpd	%%ymm13,%%ymm10,%%ymm10		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t		vaddpd	%%ymm12,%%ymm12,%%ymm12		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t		vaddpd	%%ymm13,%%ymm13,%%ymm13		\n\t"\
		"vmovaps	%%ymm0,0x80(%%rsi)		\n\t		vmovaps	%%ymm8 ,0x40(%%r8)	\n\t"\
		"vmovaps	%%ymm1,0xe0(%%rsi)		\n\t		vmovaps	%%ymm10,0x60(%%r8)	\n\t"\
		"vaddpd	%%ymm0,%%ymm5,%%ymm5		\n\t		vaddpd	%%ymm8 ,%%ymm12,%%ymm12	\n\t"\
		"vaddpd	%%ymm1,%%ymm4,%%ymm4		\n\t		vaddpd	%%ymm10,%%ymm13,%%ymm13	\n\t"\
		"vmovaps	%%ymm5,0xc0(%%rsi)		\n\t		vmovaps	%%ymm12,    (%%r8)	\n\t"\
		"vmovaps	%%ymm4,0xa0(%%rsi)		\n\t		vmovaps	%%ymm13,0x20(%%r8)	\n\t"\
		/* Block 2: */								"	vsubpd	%%ymm15,%%ymm11,%%ymm11	\n\t"\
		"movq	%[__out2],%%rsi				\n\t		vsubpd	%%ymm14,%%ymm9 ,%%ymm9	\n\t"\
		"movq	%[__in4],%%rax				\n\t		vaddpd	%%ymm15,%%ymm15,%%ymm15	\n\t"\
		"movq	%[__in6],%%rbx				\n\t		vaddpd	%%ymm14,%%ymm14,%%ymm14	\n\t"\
		"movq	%[__in5],%%rcx				\n\t		vmovaps	%%ymm11,0x80(%%r8)	\n\t"\
		"movq	%[__in7],%%rdx				\n\t		vmovaps	%%ymm9 ,0xe0(%%r8)	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4		\n\t		vaddpd	%%ymm11,%%ymm15,%%ymm15	\n\t"\
		"vmovaps		(%%rdx),%%ymm6		\n\t		vaddpd	%%ymm9 ,%%ymm14,%%ymm14	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5		\n\t		vmovaps	%%ymm15,0xc0(%%r8)	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7		\n\t		vmovaps	%%ymm14,0xa0(%%r8)	\n\t"\
		"vmovaps	%%ymm4,%%ymm0			\n\t	"	/* Block 3: */\
		"vmovaps	%%ymm6,%%ymm2			\n\t		movq	%[__out3],%%r8	\n\t"\
		"vmovaps	%%ymm5,%%ymm1			\n\t		movq	%[__inc],%%r10		\n\t"\
		"vmovaps	%%ymm7,%%ymm3			\n\t		movq	%[__ine],%%r11		\n\t"\
		"vmulpd	0x20(%%rdi),%%ymm4,%%ymm4	\n\t		movq	%[__ind],%%r12		\n\t"\
		"vmulpd	0x40(%%rdi),%%ymm6,%%ymm6	\n\t		movq	%[__inf],%%r13		\n\t"\
		"vmulpd	0x40(%%rdi),%%ymm1,%%ymm1	\n\t		vmovaps	    (%%r12),%%ymm12	\n\t"\
		"vmulpd	0x20(%%rdi),%%ymm3,%%ymm3	\n\t		vmovaps		(%%r13),%%ymm14	\n\t"\
		"vmulpd	0x20(%%rdi),%%ymm5,%%ymm5	\n\t		vmovaps	0x20(%%r12),%%ymm13	\n\t"\
		"vmulpd	0x40(%%rdi),%%ymm7,%%ymm7	\n\t		vmovaps	0x20(%%r13),%%ymm15	\n\t"\
		"vmulpd	0x40(%%rdi),%%ymm0,%%ymm0	\n\t		vmovaps	%%ymm12,%%ymm8		\n\t"\
		"vmulpd	0x20(%%rdi),%%ymm2,%%ymm2	\n\t		vmovaps	%%ymm14,%%ymm10		\n\t"\
		"vsubpd	%%ymm1,%%ymm4,%%ymm4		\n\t		vmovaps	%%ymm13,%%ymm9		\n\t"\
		"vsubpd	%%ymm3,%%ymm6,%%ymm6		\n\t		vmovaps	%%ymm15,%%ymm11		\n\t"\
		"vaddpd	%%ymm0,%%ymm5,%%ymm5		\n\t		vmulpd	0x40(%%rdi),%%ymm12,%%ymm12	\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7		\n\t		vmulpd	0x20(%%rdi),%%ymm14,%%ymm14	\n\t"\
		"vsubpd	%%ymm6,%%ymm4,%%ymm4		\n\t		vmulpd	0x20(%%rdi),%%ymm9 ,%%ymm9	\n\t"\
		"vsubpd	%%ymm7,%%ymm5,%%ymm5		\n\t		vmulpd	0x40(%%rdi),%%ymm11,%%ymm11	\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t		vmulpd	0x40(%%rdi),%%ymm13,%%ymm13	\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t		vmulpd	0x20(%%rdi),%%ymm15,%%ymm15	\n\t"\
		"vaddpd	%%ymm4,%%ymm6,%%ymm6		\n\t		vmulpd	0x20(%%rdi),%%ymm8 ,%%ymm8	\n\t"\
		"vaddpd	%%ymm5,%%ymm7,%%ymm7		\n\t		vmulpd	0x40(%%rdi),%%ymm10,%%ymm10	\n\t"\
		"vmovaps	    (%%rbx),%%ymm2		\n\t		vsubpd	%%ymm9 ,%%ymm12,%%ymm12	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3		\n\t		vsubpd	%%ymm11,%%ymm14,%%ymm14	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm2,%%ymm2	\n\t		vaddpd	%%ymm8 ,%%ymm13,%%ymm13	\n\t"\
		"vaddpd		(%%rbx),%%ymm3,%%ymm3	\n\t		vaddpd	%%ymm10,%%ymm15,%%ymm15	\n\t"\
		"vmulpd	(%%rdi),%%ymm2,%%ymm2		\n\t		vsubpd	%%ymm14,%%ymm12,%%ymm12	\n\t"\
		"vmulpd	(%%rdi),%%ymm3,%%ymm3		\n\t		vsubpd	%%ymm15,%%ymm13,%%ymm13	\n\t"\
		"vmovaps	    (%%rax),%%ymm0		\n\t		vaddpd	%%ymm14,%%ymm14,%%ymm14	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1		\n\t		vaddpd	%%ymm15,%%ymm15,%%ymm15	\n\t"\
		"vsubpd	%%ymm2,%%ymm0,%%ymm0		\n\t		vaddpd	%%ymm12,%%ymm14,%%ymm14	\n\t"\
		"vsubpd	%%ymm3,%%ymm1,%%ymm1		\n\t		vaddpd	%%ymm13,%%ymm15,%%ymm15	\n\t"\
		"vaddpd	    (%%rax),%%ymm2,%%ymm2	\n\t		vmovaps	    (%%r11),%%ymm10	\n\t"\
		"vaddpd	0x20(%%rax),%%ymm3,%%ymm3	\n\t		vmovaps	0x20(%%r11),%%ymm11	\n\t"\
		"vsubpd	%%ymm6,%%ymm2,%%ymm2		\n\t		vaddpd	0x20(%%r11),%%ymm10,%%ymm10	\n\t"\
		"vsubpd	%%ymm7,%%ymm3,%%ymm3		\n\t		vsubpd	    (%%r11),%%ymm11,%%ymm11	\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t		vmulpd	(%%rdi),%%ymm10,%%ymm10	\n\t"/* mul by isrt2 */\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t		vmulpd	(%%rdi),%%ymm11,%%ymm11	\n\t"\
		"vmovaps	%%ymm2,0x40(%%rsi)		\n\t		vmovaps	    (%%r10),%%ymm8	\n\t"\
		"vmovaps	%%ymm3,0x60(%%rsi)		\n\t		vmovaps	0x20(%%r10),%%ymm9	\n\t"\
		"vaddpd	%%ymm2,%%ymm6,%%ymm6		\n\t		vsubpd	%%ymm10,%%ymm8,%%ymm8		\n\t"\
		"vaddpd	%%ymm3,%%ymm7,%%ymm7		\n\t		vsubpd	%%ymm11,%%ymm9,%%ymm9		\n\t"\
		"vmovaps	%%ymm6,    (%%rsi)		\n\t		vaddpd	    (%%r10),%%ymm10,%%ymm10	\n\t"\
		"vmovaps	%%ymm7,0x20(%%rsi)		\n\t		vaddpd	0x20(%%r10),%%ymm11,%%ymm11	\n\t"\
		"vsubpd	%%ymm5,%%ymm0,%%ymm0		\n\t		vsubpd	%%ymm12,%%ymm8 ,%%ymm8		\n\t"\
		"vsubpd	%%ymm4,%%ymm1,%%ymm1		\n\t		vsubpd	%%ymm13,%%ymm9 ,%%ymm9		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t		vaddpd	%%ymm12,%%ymm12,%%ymm12		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t		vaddpd	%%ymm13,%%ymm13,%%ymm13		\n\t"\
		"vmovaps	%%ymm0,0x80(%%rsi)		\n\t		vmovaps	%%ymm8,0x40(%%r8)	\n\t"\
		"vmovaps	%%ymm1,0xe0(%%rsi)		\n\t		vmovaps	%%ymm9,0x60(%%r8)	\n\t"\
		"vaddpd	%%ymm0,%%ymm5,%%ymm5		\n\t		vaddpd	%%ymm8,%%ymm12,%%ymm12	\n\t"\
		"vaddpd	%%ymm1,%%ymm4,%%ymm4		\n\t		vaddpd	%%ymm9,%%ymm13,%%ymm13	\n\t"\
		"vmovaps	%%ymm5,0xc0(%%rsi)		\n\t		vmovaps	%%ymm12,    (%%r8)	\n\t"\
		"vmovaps	%%ymm4,0xa0(%%rsi)		\n\t		vmovaps	%%ymm13,0x20(%%r8)	\n\t"\
		"												vsubpd	%%ymm15,%%ymm10,%%ymm10	\n\t"\
		"												vsubpd	%%ymm14,%%ymm11,%%ymm11	\n\t"\
		"												vaddpd	%%ymm15,%%ymm15,%%ymm15	\n\t"\
		"												vaddpd	%%ymm14,%%ymm14,%%ymm14	\n\t"\
		"												vmovaps	%%ymm10,0x80(%%r8)	\n\t"\
		"												vmovaps	%%ymm11,0xe0(%%r8)	\n\t"\
		"												vaddpd	%%ymm10,%%ymm15,%%ymm15	\n\t"\
		"												vaddpd	%%ymm11,%%ymm14,%%ymm14	\n\t"\
		"												vmovaps	%%ymm15,0xc0(%%r8)	\n\t"\
		"												vmovaps	%%ymm14,0xa0(%%r8)	\n\t"\
		:					/* outputs: none */\
		:[__in0] "m" (Xin0)	/* All inputs from memory addresses here */\
		,[__in1] "m" (Xin1)\
		,[__in2] "m" (Xin2)\
		,[__in3] "m" (Xin3)\
		,[__in4] "m" (Xin4)\
		,[__in5] "m" (Xin5)\
		,[__in6] "m" (Xin6)\
		,[__in7] "m" (Xin7)\
		,[__in8] "m" (Xin8)\
		,[__in9] "m" (Xin9)\
		,[__ina] "m" (Xina)\
		,[__inb] "m" (Xinb)\
		,[__inc] "m" (Xinc)\
		,[__ind] "m" (Xind)\
		,[__ine] "m" (Xine)\
		,[__inf] "m" (Xinf)\
		,[__two] "m" (Xtwo)\
		,[__isrt2] "m" (Xisrt2)\
		,[__out0] "m" (Xout0)\
		,[__out1] "m" (Xout1)\
		,[__out2] "m" (Xout2)\
		,[__out3] "m" (Xout3)\
		: "cc","memory","rax","rbx","rcx","rdx","rdi","rsi","r8","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

   #else

	#define SSE2_RADIX16_DIF_0TWIDDLE_B(Xin0,Xi1,Xi2,Xi3,Xi4, Xisrt2,Xtwo, Xout0)\
	{\
	__asm__ volatile (\
		/* SSE2_RADIX4_DIF_IN_PLACE(r1 , r17, r9 , r25): */\
		"movq	%[__in0],%%rax		\n\t"\
		"leaq	%c[__i4](%%rax),%%rcx	\n\t"/* __in0 +   [4*istride]; note BR of [a,b,c,d]-ptrs, i.e. b/c swap */\
		"leaq	%c[__i4](%%rcx),%%rbx	\n\t"/* __in0 + 2*[4*istride] */\
		"leaq	%c[__i4](%%rbx),%%rdx	\n\t"/* __in0 + 3*[4*istride] */\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps	    (%%rax),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3	\n\t"\
		"vaddpd	    (%%rbx),%%ymm0,%%ymm0	\n\t"\
		"vaddpd	0x20(%%rbx),%%ymm1,%%ymm1	\n\t"\
		"vsubpd	    (%%rbx),%%ymm2,%%ymm2	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm3,%%ymm3	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7	\n\t"\
		"vaddpd	    (%%rdx),%%ymm4,%%ymm4	\n\t"\
		"vaddpd	0x20(%%rdx),%%ymm5,%%ymm5	\n\t"\
		"vsubpd	    (%%rdx),%%ymm6,%%ymm6	\n\t"\
		"vsubpd	0x20(%%rdx),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm0,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm4,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)	\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm2,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rdx)	\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	%%ymm7,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rcx)	\n\t"\
		/* SSE2_RADIX4_DIF_IN_PLACE(r5 , r21, r13, r29): */\
		"addq	$%c[__i2],%%rax	\n\t"/* All addresses += 2*ostride */\
		"addq	$%c[__i2],%%rbx	\n\t"\
		"addq	$%c[__i2],%%rcx	\n\t"\
		"addq	$%c[__i2],%%rdx	\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps	    (%%rax),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3	\n\t"\
		"vaddpd	    (%%rbx),%%ymm0,%%ymm0	\n\t"\
		"vaddpd	0x20(%%rbx),%%ymm1,%%ymm1	\n\t"\
		"vsubpd	    (%%rbx),%%ymm2,%%ymm2	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm3,%%ymm3	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7	\n\t"\
		"vaddpd	    (%%rdx),%%ymm4,%%ymm4	\n\t"\
		"vaddpd	0x20(%%rdx),%%ymm5,%%ymm5	\n\t"\
		"vsubpd	    (%%rdx),%%ymm6,%%ymm6	\n\t"\
		"vsubpd	0x20(%%rdx),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm0,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm4,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)	\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm2,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rdx)	\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	%%ymm7,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rcx)	\n\t"\
		/* SSE2_RADIX4_DIF_IN_PLACE(r3 , r19, r11, r27): */\
		"subq	$%c[__i1],%%rax	\n\t"/* All addresses -= 1*ostride */\
		"subq	$%c[__i1],%%rbx	\n\t"\
		"subq	$%c[__i1],%%rcx	\n\t"\
		"subq	$%c[__i1],%%rdx	\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps	    (%%rax),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3	\n\t"\
		"vaddpd	    (%%rbx),%%ymm0,%%ymm0	\n\t"\
		"vaddpd	0x20(%%rbx),%%ymm1,%%ymm1	\n\t"\
		"vsubpd	    (%%rbx),%%ymm2,%%ymm2	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm3,%%ymm3	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7	\n\t"\
		"vaddpd	    (%%rdx),%%ymm4,%%ymm4	\n\t"\
		"vaddpd	0x20(%%rdx),%%ymm5,%%ymm5	\n\t"\
		"vsubpd	    (%%rdx),%%ymm6,%%ymm6	\n\t"\
		"vsubpd	0x20(%%rdx),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm0,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm4,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)	\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm2,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rdx)	\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	%%ymm7,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rcx)	\n\t"\
		/* SSE2_RADIX4_DIF_IN_PLACE(r7 , r23, r15, r31): */\
		"addq	$%c[__i2],%%rax	\n\t"/* All addresses += 2*ostride */\
		"addq	$%c[__i2],%%rbx	\n\t"\
		"addq	$%c[__i2],%%rcx	\n\t"\
		"addq	$%c[__i2],%%rdx	\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps	    (%%rax),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3	\n\t"\
		"vaddpd	    (%%rbx),%%ymm0,%%ymm0	\n\t"\
		"vaddpd	0x20(%%rbx),%%ymm1,%%ymm1	\n\t"\
		"vsubpd	    (%%rbx),%%ymm2,%%ymm2	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm3,%%ymm3	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7	\n\t"\
		"vaddpd	    (%%rdx),%%ymm4,%%ymm4	\n\t"\
		"vaddpd	0x20(%%rdx),%%ymm5,%%ymm5	\n\t"\
		"vsubpd	    (%%rdx),%%ymm6,%%ymm6	\n\t"\
		"vsubpd	0x20(%%rdx),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm0,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm4,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)	\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm2,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rdx)	\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	%%ymm7,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rcx)	\n\t"\
	/*** Now do 4 DFTs with internal twiddles on the 1*stride - separated data. Do blocks in order 0,2,1,3 to allow increment-only of rsi-datum from 1 block to the next: ***/\
		"movq	%[__isrt2],%%rdi	\n\t"\
		"vmovaps	(%%rdi),%%ymm10	\n\t"/* isrt2 */\
		/* Block 0: r0-3 */\
		"movq	%[__out0],%%rsi	\n\t"\
		"movq	%[__in0],%%rax		\n\t"\
		"leaq	%c[__i1](%%rax),%%rcx	\n\t"/* __in0 +   istride; note BR of [a,b,c,d]-ptrs, i.e. b/c swap */\
		"leaq	%c[__i1](%%rcx),%%rbx	\n\t"/* __in0 + 2*istride */\
		"leaq	%c[__i1](%%rbx),%%rdx	\n\t"/* __in0 + 3*istride */\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps		(%%rbx),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3	\n\t"\
		"vsubpd		(%%rbx),%%ymm0,%%ymm0	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm1,%%ymm1	\n\t"\
		"vaddpd	    (%%rax),%%ymm2,%%ymm2	\n\t"\
		"vaddpd	0x20(%%rax),%%ymm3,%%ymm3	\n\t"\
		"vmovaps		(%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rdx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7	\n\t"\
		"vsubpd		(%%rdx),%%ymm4,%%ymm4	\n\t"\
		"vsubpd	0x20(%%rdx),%%ymm5,%%ymm5	\n\t"\
		"vaddpd	    (%%rcx),%%ymm6,%%ymm6	\n\t"\
		"vaddpd	0x20(%%rcx),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm6,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm7,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	%%ymm2,0x40(%%rsi)	\n\t"\
		"vmovaps	%%ymm3,0x60(%%rsi)	\n\t"\
		"vaddpd	%%ymm2,%%ymm6,%%ymm6	\n\t"\
		"vaddpd	%%ymm3,%%ymm7,%%ymm7	\n\t"\
		"vmovaps	%%ymm6,    (%%rsi)	\n\t"\
		"vmovaps	%%ymm7,0x20(%%rsi)	\n\t"\
		"vsubpd	%%ymm5,%%ymm0,%%ymm0	\n\t"\
		"vsubpd	%%ymm4,%%ymm1,%%ymm1	\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4	\n\t"\
		"vmovaps	%%ymm0,0x80(%%rsi)	\n\t"\
		"vmovaps	%%ymm1,0xe0(%%rsi)	\n\t"\
		"vaddpd	%%ymm0,%%ymm5,%%ymm5	\n\t"\
		"vaddpd	%%ymm1,%%ymm4,%%ymm4	\n\t"\
		"vmovaps	%%ymm5,0xc0(%%rsi)	\n\t"\
		"vmovaps	%%ymm4,0xa0(%%rsi)	\n\t"\
		/* Block 2: */\
		"addq	$0x200,%%rsi	\n\t"\
		"addq	$%c[__i4],%%rax	\n\t"/* All addresses += 4*ostride */\
		"addq	$%c[__i4],%%rbx	\n\t"\
		"addq	$%c[__i4],%%rcx	\n\t"\
		"addq	$%c[__i4],%%rdx	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps		(%%rdx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7	\n\t"\
		"vmovaps	%%ymm4,%%ymm0		\n\t"\
		"vmovaps	%%ymm6,%%ymm2		\n\t"\
		"vmovaps	%%ymm5,%%ymm1		\n\t"\
		"vmovaps	%%ymm7,%%ymm3		\n\t"\
		"vmovaps	0x20(%%rdi),%%ymm8	\n\t"/* cc0 */\
		"vmovaps	0x40(%%rdi),%%ymm9	\n\t"/* ss0 */\
		"vmulpd	%%ymm8,%%ymm4,%%ymm4	\n\t"\
		"vmulpd	%%ymm9,%%ymm6,%%ymm6	\n\t"\
		"vmulpd	%%ymm9,%%ymm1,%%ymm1	\n\t"\
		"vmulpd	%%ymm8,%%ymm3,%%ymm3	\n\t"\
		"vmulpd	%%ymm8,%%ymm5,%%ymm5	\n\t"\
		"vmulpd	%%ymm9,%%ymm7,%%ymm7	\n\t"\
		"vmulpd	%%ymm9,%%ymm0,%%ymm0	\n\t"\
		"vmulpd	%%ymm8,%%ymm2,%%ymm2	\n\t"\
		"vsubpd	%%ymm1,%%ymm4,%%ymm4		\n\t"\
		"vsubpd	%%ymm3,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm0,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7		\n\t"\
		"vsubpd	%%ymm6,%%ymm4,%%ymm4		\n\t"\
		"vsubpd	%%ymm7,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm4,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm5,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	    (%%rbx),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm2,%%ymm2	\n\t"\
		"vaddpd		(%%rbx),%%ymm3,%%ymm3	\n\t"\
		"vmulpd	%%ymm10,%%ymm2,%%ymm2	\n\t"/* mul by isrt2 */\
		"vmulpd	%%ymm10,%%ymm3,%%ymm3	\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vsubpd	%%ymm2,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm3,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	    (%%rax),%%ymm2,%%ymm2	\n\t"\
		"vaddpd	0x20(%%rax),%%ymm3,%%ymm3	\n\t"\
		"vsubpd	%%ymm6,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm7,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	%%ymm2,0x40(%%rsi)	\n\t"\
		"vmovaps	%%ymm3,0x60(%%rsi)	\n\t"\
		"vaddpd	%%ymm2,%%ymm6,%%ymm6	\n\t"\
		"vaddpd	%%ymm3,%%ymm7,%%ymm7	\n\t"\
		"vmovaps	%%ymm6,    (%%rsi)	\n\t"\
		"vmovaps	%%ymm7,0x20(%%rsi)	\n\t"\
		"vsubpd	%%ymm5,%%ymm0,%%ymm0	\n\t"\
		"vsubpd	%%ymm4,%%ymm1,%%ymm1	\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4	\n\t"\
		"vmovaps	%%ymm0,0x80(%%rsi)	\n\t"\
		"vmovaps	%%ymm1,0xe0(%%rsi)	\n\t"\
		"vaddpd	%%ymm0,%%ymm5,%%ymm5	\n\t"\
		"vaddpd	%%ymm1,%%ymm4,%%ymm4	\n\t"\
		"vmovaps	%%ymm5,0xc0(%%rsi)	\n\t"\
		"vmovaps	%%ymm4,0xa0(%%rsi)	\n\t"\
		/* Block 1: r8-b */\
		"subq	$0x100,%%rsi	\n\t"\
		"addq	$%c[__i4],%%rax	\n\t"/* All addresses += 4*ostride */\
		"addq	$%c[__i4],%%rbx	\n\t"\
		"addq	$%c[__i4],%%rcx	\n\t"\
		"addq	$%c[__i4],%%rdx	\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps		(%%rbx),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm0,%%ymm0	\n\t"\
		"vsubpd		(%%rbx),%%ymm1,%%ymm1	\n\t"\
		"vaddpd	0x20(%%rax),%%ymm2,%%ymm2	\n\t"\
		"vaddpd		(%%rax),%%ymm3,%%ymm3	\n\t"\
		"vmovaps		(%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rdx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7	\n\t"\
		"vsubpd	0x20(%%rcx),%%ymm4,%%ymm4	\n\t"\
		"vaddpd		(%%rcx),%%ymm5,%%ymm5	\n\t"\
		"vaddpd	0x20(%%rdx),%%ymm6,%%ymm6	\n\t"\
		"vsubpd		(%%rdx),%%ymm7,%%ymm7	\n\t"\
		"vmulpd	%%ymm10,%%ymm4,%%ymm4		\n\t"\
		"vmulpd	%%ymm10,%%ymm5,%%ymm5		\n\t"\
		"vmulpd	%%ymm10,%%ymm6,%%ymm6		\n\t"\
		"vmulpd	%%ymm10,%%ymm7,%%ymm7		\n\t"\
		"vsubpd	%%ymm6,%%ymm4,%%ymm4		\n\t"\
		"vsubpd	%%ymm7,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm4,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm5,%%ymm7,%%ymm7		\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm5,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm0,0x40(%%rsi)	\n\t"\
		"vmovaps	%%ymm2,0x60(%%rsi)	\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4	\n\t"\
		"vaddpd	%%ymm2,%%ymm5,%%ymm5	\n\t"\
		"vmovaps	%%ymm4,    (%%rsi)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rsi)	\n\t"\
		"vsubpd	%%ymm7,%%ymm3,%%ymm3	\n\t"\
		"vsubpd	%%ymm6,%%ymm1,%%ymm1	\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6	\n\t"\
		"vmovaps	%%ymm3,0x80(%%rsi)	\n\t"\
		"vmovaps	%%ymm1,0xe0(%%rsi)	\n\t"\
		"vaddpd	%%ymm3,%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm1,%%ymm6,%%ymm6	\n\t"\
		"vmovaps	%%ymm7,0xc0(%%rsi)	\n\t"\
		"vmovaps	%%ymm6,0xa0(%%rsi)	\n\t"\
		/* Block 3: */\
		"addq	$0x200,%%rsi	\n\t"\
		"addq	$%c[__i4],%%rax	\n\t"/* All addresses += 4*ostride */\
		"addq	$%c[__i4],%%rbx	\n\t"\
		"addq	$%c[__i4],%%rcx	\n\t"\
		"addq	$%c[__i4],%%rdx	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps		(%%rdx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7	\n\t"\
		"vmovaps	%%ymm4,%%ymm0		\n\t"\
		"vmovaps	%%ymm6,%%ymm2		\n\t"\
		"vmovaps	%%ymm5,%%ymm1		\n\t"\
		"vmovaps	%%ymm7,%%ymm3		\n\t"\
		"vmulpd	%%ymm9,%%ymm4,%%ymm4	\n\t"\
		"vmulpd	%%ymm8,%%ymm6,%%ymm6	\n\t"\
		"vmulpd	%%ymm8,%%ymm1,%%ymm1	\n\t"\
		"vmulpd	%%ymm9,%%ymm3,%%ymm3	\n\t"\
		"vmulpd	%%ymm9,%%ymm5,%%ymm5	\n\t"\
		"vmulpd	%%ymm8,%%ymm7,%%ymm7	\n\t"\
		"vmulpd	%%ymm8,%%ymm0,%%ymm0	\n\t"\
		"vmulpd	%%ymm9,%%ymm2,%%ymm2	\n\t"\
		"vsubpd	%%ymm1,%%ymm4,%%ymm4		\n\t"\
		"vsubpd	%%ymm3,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm0,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7		\n\t"\
		"vsubpd	%%ymm6,%%ymm4,%%ymm4		\n\t"\
		"vsubpd	%%ymm7,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm4,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm5,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	    (%%rbx),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3	\n\t"\
		"vaddpd	0x20(%%rbx),%%ymm2,%%ymm2	\n\t"\
		"vsubpd	    (%%rbx),%%ymm3,%%ymm3	\n\t"\
		"vmulpd	%%ymm10,%%ymm2,%%ymm2	\n\t"/* mul by isrt2 */\
		"vmulpd	%%ymm10,%%ymm3,%%ymm3	\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vsubpd	%%ymm2,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm3,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	    (%%rax),%%ymm2,%%ymm2	\n\t"\
		"vaddpd	0x20(%%rax),%%ymm3,%%ymm3	\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm0,0x40(%%rsi)	\n\t"\
		"vmovaps	%%ymm1,0x60(%%rsi)	\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4	\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5	\n\t"\
		"vmovaps	%%ymm4,    (%%rsi)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rsi)	\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2	\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3	\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6	\n\t"\
		"vmovaps	%%ymm2,0x80(%%rsi)	\n\t"\
		"vmovaps	%%ymm3,0xe0(%%rsi)	\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6	\n\t"\
		"vmovaps	%%ymm7,0xc0(%%rsi)	\n\t"\
		"vmovaps	%%ymm6,0xa0(%%rsi)	\n\t"\
		:					/* outputs: none */\
		:[__in0] "m" (Xin0)	/* All inputs from memory addresses here */\
		,[__i1] "e" (Xi1)\
		,[__i2] "e" (Xi2)\
		,[__i3] "e" (Xi3)\
		,[__i4] "e" (Xi4)\
		,[__isrt2] "m" (Xisrt2)\
		,[__two] "m" (Xtwo)\
		,[__out0] "m" (Xout0)\
		: "cc","memory","rax","rbx","rcx","rdx","rdi","rsi","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10"		/* Clobbered registers */\
	);\
	}

   #endif	// 8/16-reg versions of SSE2_RADIX16_DIF_0TWIDDLE_B

	// Based on the SSE2_RADIX16_DIT_NOTWIDDLE macro in radix16_ditN_cy_dif1_gcc64.h, but with completely
	// specifiable 16-input addressing required for usage as the power-of-2 component of a twiddleless
	// radix = [odd*2^n] DFT routine.
	// We use just a single output base-pointer plus literal ostrides which are [1,2,3,4]-multiples of
	// __01; this allows us to cut GP-register usage, which is absolutely a must for the 32-bit version
	// of the macro, and is a benefit to the 64-bit versions which code-fold to yield 2 side-by-side
	// streams of independently executable instructions, one for data in xmm0-7, the other using xmm8-15.
	#define SSE2_RADIX16_DIT_0TWIDDLE(Xin0,Xin1,Xin2,Xin3,Xin4,Xin5,Xin6,Xin7,Xin8,Xin9,Xina,Xinb,Xinc,Xind,Xine,Xinf, Xisrt2,Xtwo, Xout0,Xo1,Xo2,Xo3,Xo4)\
	{\
	__asm__ volatile (\
		"movq	%[__in0],%%rax		\n\t"\
		"movq	%[__in1],%%rbx		\n\t"\
		"movq	%[__in2],%%rcx		\n\t"\
		"movq	%[__in3],%%rdx		\n\t"\
	"prefetcht1	0x100(%%rax)\n\t"\
		/* SSE2_RADIX4_DIT_0TWIDDLE_B(r0 ): */\
		"vmovaps	    (%%rax),%%ymm2	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6		\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7		\n\t"\
		"vmovaps	    (%%rbx),%%ymm0	\n\t"\
		"vmovaps	    (%%rdx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm1	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5	\n\t"\
		"vsubpd	%%ymm0,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm4,%%ymm6,%%ymm6		\n\t"\
		"vsubpd	%%ymm1,%%ymm3,%%ymm3		\n\t"\
		"vsubpd	%%ymm5,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm0,%%ymm0,%%ymm0		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm1,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm2,%%ymm0,%%ymm0		\n\t"\
		"vaddpd	%%ymm6,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm3,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm7,%%ymm5,%%ymm5		\n\t"\
		"movq	%[__out0],%%rsi		\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3		\n\t"\
	"prefetcht1	0x100(%%rcx)\n\t"\
		"leaq	0x20(%%rsi),%%rdi	\n\t"/* Need separate address Im parts of outputs due to literal-offsets below */\
		"vmovaps	%%ymm0,%c[__o2](%%rsi)	\n\t"\
		"vmovaps	%%ymm2,%c[__o3](%%rsi)	\n\t"\
		"vmovaps	%%ymm1,%c[__o2](%%rdi)	\n\t"\
		"vmovaps	%%ymm3,%c[__o1](%%rdi)	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	%%ymm4,        (%%rsi)	\n\t"\
		"vmovaps	%%ymm7,%c[__o1](%%rsi)	\n\t"\
		"vmovaps	%%ymm5,        (%%rdi)	\n\t"\
		"vmovaps	%%ymm6,%c[__o3](%%rdi)	\n\t"\
	"prefetcht1	0x100(%%rax)\n\t"\
		/* SSE2_RADIX4_DIT_0TWIDDLE_B(r8 ): */\
		"movq	%[__in4],%%rax		\n\t	leaq	%c[__o4](%%rsi),%%rsi	\n\t"/* __out0 + 4*ostride */\
		"movq	%[__in5],%%rbx		\n\t"\
		"movq	%[__in6],%%rcx		\n\t"\
		"movq	%[__in7],%%rdx		\n\t"\
		"vmovaps	    (%%rax),%%ymm2	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7	\n\t"\
		"vmovaps	    (%%rbx),%%ymm0	\n\t"\
		"vmovaps	    (%%rdx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm1	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5	\n\t"\
		"vsubpd	%%ymm0,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm4,%%ymm6,%%ymm6		\n\t"\
		"vsubpd	%%ymm1,%%ymm3,%%ymm3		\n\t"\
		"vsubpd	%%ymm5,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm0,%%ymm0,%%ymm0		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm1,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm2,%%ymm0,%%ymm0		\n\t"\
		"vaddpd	%%ymm6,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm3,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm7,%%ymm5,%%ymm5		\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3		\n\t"\
	"prefetcht1	0x100(%%rcx)\n\t"\
		"leaq	0x20(%%rsi),%%rdi	\n\t"\
		"vmovaps	%%ymm0,%c[__o2](%%rsi)	\n\t"\
		"vmovaps	%%ymm2,%c[__o3](%%rsi)	\n\t"\
		"vmovaps	%%ymm1,%c[__o2](%%rdi)	\n\t"\
		"vmovaps	%%ymm3,%c[__o1](%%rdi)	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	%%ymm4,        (%%rsi)	\n\t"\
		"vmovaps	%%ymm7,%c[__o1](%%rsi)	\n\t"\
		"vmovaps	%%ymm5,        (%%rdi)	\n\t"\
		"vmovaps	%%ymm6,%c[__o3](%%rdi)	\n\t"\
	"prefetcht1	0x100(%%rax)\n\t"\
		/* SSE2_RADIX4_DIT_0TWIDDLE_B(r16): */\
		"movq	%[__in8],%%rax			\n\t	leaq	%c[__o4](%%rsi),%%rsi	\n\t"/* __out0 + 8*ostride */\
		"movq	%[__in9],%%rbx			\n\t"\
		"movq	%[__ina],%%rcx			\n\t"\
		"movq	%[__inb],%%rdx			\n\t"\
		"vmovaps	    (%%rax),%%ymm2	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7	\n\t"\
		"vmovaps	    (%%rbx),%%ymm0	\n\t"\
		"vmovaps	    (%%rdx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm1	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5	\n\t"\
		"vsubpd	%%ymm0,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm4,%%ymm6,%%ymm6		\n\t"\
		"vsubpd	%%ymm1,%%ymm3,%%ymm3		\n\t"\
		"vsubpd	%%ymm5,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm0,%%ymm0,%%ymm0		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm1,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm2,%%ymm0,%%ymm0		\n\t"\
		"vaddpd	%%ymm6,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm3,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm7,%%ymm5,%%ymm5		\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0			\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2			\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1			\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3			\n\t"\
	"prefetcht1	0x100(%%rcx)\n\t"\
		"leaq	0x20(%%rsi),%%rdi	\n\t"\
		"vmovaps	%%ymm0,%c[__o2](%%rsi)	\n\t"\
		"vmovaps	%%ymm2,%c[__o3](%%rsi)	\n\t"\
		"vmovaps	%%ymm1,%c[__o2](%%rdi)	\n\t"\
		"vmovaps	%%ymm3,%c[__o1](%%rdi)	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4			\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7			\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5			\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6			\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4			\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7			\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5			\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6			\n\t"\
		"vmovaps	%%ymm4,        (%%rsi)	\n\t"\
		"vmovaps	%%ymm7,%c[__o1](%%rsi)	\n\t"\
		"vmovaps	%%ymm5,        (%%rdi)	\n\t"\
		"vmovaps	%%ymm6,%c[__o3](%%rdi)	\n\t"\
	"prefetcht1	0x100(%%rax)\n\t"\
		/* SSE2_RADIX4_DIT_0TWIDDLE_B(r24): */\
		"movq	%[__inc],%%rax			\n\t	leaq	%c[__o4](%%rsi),%%rsi	\n\t"/* __out0 + c*ostride */\
		"movq	%[__ind],%%rbx			\n\t"\
		"movq	%[__ine],%%rcx			\n\t"\
		"movq	%[__inf],%%rdx			\n\t"\
		"vmovaps	    (%%rax),%%ymm2	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm3	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7	\n\t"\
		"vmovaps	    (%%rbx),%%ymm0	\n\t"\
		"vmovaps	    (%%rdx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm1	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5	\n\t"\
		"vsubpd	%%ymm0,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm4,%%ymm6,%%ymm6		\n\t"\
		"vsubpd	%%ymm1,%%ymm3,%%ymm3		\n\t"\
		"vsubpd	%%ymm5,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm0,%%ymm0,%%ymm0		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm1,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm2,%%ymm0,%%ymm0		\n\t"\
		"vaddpd	%%ymm6,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm3,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm7,%%ymm5,%%ymm5		\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3		\n\t"\
	"prefetcht1	0x100(%%rcx)\n\t"\
		"leaq	0x20(%%rsi),%%rdi	\n\t"\
		"vmovaps	%%ymm0,%c[__o2](%%rsi)	\n\t"\
		"vmovaps	%%ymm2,%c[__o3](%%rsi)	\n\t"\
		"vmovaps	%%ymm1,%c[__o2](%%rdi)	\n\t"\
		"vmovaps	%%ymm3,%c[__o1](%%rdi)	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	%%ymm4,        (%%rsi)	\n\t"\
		"vmovaps	%%ymm7,%c[__o1](%%rsi)	\n\t"\
		"vmovaps	%%ymm5,        (%%rdi)	\n\t"\
		"vmovaps	%%ymm6,%c[__o3](%%rdi)	\n\t"\
	/*** Now do 4 DFTs with internal twiddles on the 4*stride - separated data: ***/\
		"movq	%[__out0],%%rax		\n\t"\
		"leaq	%c[__o4](%%rax),%%rbx	\n\t"/* __out0 +   [4*ostride] */\
		"leaq	%c[__o4](%%rbx),%%rcx	\n\t"/* __out0 + 2*[4*ostride] */\
		"leaq	%c[__o4](%%rcx),%%rdx	\n\t"/* __out0 + 3*[4*ostride] */\
		/* Block 0: */\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps	    (%%rbx),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3	\n\t"\
		"vsubpd	    (%%rbx),%%ymm0,%%ymm0	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm1,%%ymm1	\n\t"\
		"vaddpd	    (%%rax),%%ymm2,%%ymm2	\n\t"\
		"vaddpd	0x20(%%rax),%%ymm3,%%ymm3	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rdx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7	\n\t"\
		"vsubpd	    (%%rdx),%%ymm4,%%ymm4	\n\t"\
		"vsubpd	0x20(%%rdx),%%ymm5,%%ymm5	\n\t"\
		"vaddpd	    (%%rcx),%%ymm6,%%ymm6	\n\t"\
		"vaddpd	0x20(%%rcx),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm6,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm7,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm2,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rcx)	\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm2,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm3,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	%%ymm6,    (%%rax)	\n\t"\
		"vmovaps	%%ymm7,0x20(%%rax)	\n\t"\
		"vsubpd	%%ymm5,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm4,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm0,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)	\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm0,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm1,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm4,0x20(%%rdx)	\n\t"\
		/* Block 2: */\
		"leaq	%c[__o2](%%rax),%%rax	\n\t"/* All addresses += 2*ostride */\
		"leaq	%c[__o2](%%rbx),%%rbx	\n\t"\
		"leaq	%c[__o2](%%rcx),%%rcx	\n\t"\
		"leaq	%c[__o2](%%rdx),%%rdx	\n\t"\
		"movq	%[__isrt2],%%rdi	\n\t"\
		"vmovaps	(%%rdi),%%ymm2		\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rdx),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm1	\n\t"\
		"vaddpd	0x20(%%rcx),%%ymm4,%%ymm4	\n\t"\
		"vsubpd	    (%%rcx),%%ymm5,%%ymm5	\n\t"\
		"vsubpd	0x20(%%rdx),%%ymm0,%%ymm0	\n\t"\
		"vaddpd	    (%%rdx),%%ymm1,%%ymm1	\n\t"\
		"vmulpd	%%ymm2,%%ymm4,%%ymm4					\n\t"\
		"vmulpd	%%ymm2,%%ymm5,%%ymm5					\n\t"\
		"vmulpd	%%ymm2,%%ymm0,%%ymm0					\n\t"\
		"vmulpd	%%ymm2,%%ymm1,%%ymm1					\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vsubpd	%%ymm0,%%ymm4,%%ymm4					\n\t"\
		"vsubpd	%%ymm1,%%ymm5,%%ymm5					\n\t"\
		"vaddpd	%%ymm0,%%ymm6,%%ymm6					\n\t"\
		"vaddpd	%%ymm1,%%ymm7,%%ymm7					\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps	    (%%rbx),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm0,%%ymm0	\n\t"\
		"vsubpd	    (%%rbx),%%ymm1,%%ymm1	\n\t"\
		"vaddpd	    (%%rax),%%ymm3,%%ymm3	\n\t"\
		"vaddpd	0x20(%%rax),%%ymm2,%%ymm2	\n\t"\
		"vsubpd	%%ymm4,%%ymm3,%%ymm3		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm3,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rcx)	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm3,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm4,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)	\n\t"\
		"vsubpd	%%ymm7,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm6,%%ymm2,%%ymm2		\n\t"\
		"vmovaps	%%ymm0,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm2,0x20(%%rbx)	\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm0,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm2,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	%%ymm7,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rdx)	\n\t"\
		/* Block 1: */\
		"leaq	-%c[__o1](%%rax),%%rax	\n\t"/* All addresses -= 1*ostride */\
		"leaq	-%c[__o1](%%rbx),%%rbx	\n\t"\
		"leaq	-%c[__o1](%%rcx),%%rcx	\n\t"\
		"leaq	-%c[__o1](%%rdx),%%rdx	\n\t"\
		"leaq	0x20(%%rdi),%%rsi	\n\t"/* cc0 */\
		"vmovaps	    (%%rdx),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm1	\n\t"\
		"vmovaps	    (%%rdx),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm3	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm0,%%ymm0	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm1,%%ymm1	\n\t"\
		"vmulpd	    (%%rsi),%%ymm2,%%ymm2	\n\t"\
		"vmulpd	    (%%rsi),%%ymm3,%%ymm3	\n\t"\
		"vsubpd	%%ymm2,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm3,%%ymm0,%%ymm0		\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7	\n\t"\
		"vmulpd	    (%%rsi),%%ymm4,%%ymm4	\n\t"\
		"vmulpd	    (%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vsubpd	%%ymm0,%%ymm6,%%ymm6		\n\t"\
		"vsubpd	%%ymm1,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	    (%%rbx),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3	\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vaddpd	0x20(%%rbx),%%ymm2,%%ymm2	\n\t"\
		"vsubpd	    (%%rbx),%%ymm3,%%ymm3	\n\t"\
		"vmulpd	    (%%rdi),%%ymm2,%%ymm2	\n\t"\
		"vmulpd	    (%%rdi),%%ymm3,%%ymm3	\n\t"\
		"vsubpd	%%ymm2,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm3,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm2,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm3,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm0,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm1,%%ymm3,%%ymm3		\n\t"\
		"vsubpd	%%ymm4,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm5,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm2,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rcx)	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm2,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm3,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm4,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)	\n\t"\
		"vsubpd	%%ymm7,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm6,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm0,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)	\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm0,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm1,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	%%ymm7,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rdx)	\n\t"\
		/* Block 3: */\
		"leaq	%c[__o2](%%rax),%%rax	\n\t"/* All addresses += 2*ostride */\
		"leaq	%c[__o2](%%rbx),%%rbx	\n\t"\
		"leaq	%c[__o2](%%rcx),%%rcx	\n\t"\
		"leaq	%c[__o2](%%rdx),%%rdx	\n\t"\
		"vmovaps	    (%%rdx),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm1	\n\t"\
		"vmovaps	    (%%rdx),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm3	\n\t"\
		"vmulpd	    (%%rsi),%%ymm0,%%ymm0	\n\t"\
		"vmulpd	    (%%rsi),%%ymm1,%%ymm1	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm2,%%ymm2	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm3,%%ymm3	\n\t"\
		"vsubpd	%%ymm2,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm3,%%ymm0,%%ymm0		\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm4,%%ymm4	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	    (%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmulpd	    (%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vsubpd	%%ymm0,%%ymm6,%%ymm6		\n\t"\
		"vsubpd	%%ymm1,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	    (%%rbx),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3	\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm2,%%ymm2	\n\t"\
		"vaddpd	    (%%rbx),%%ymm3,%%ymm3	\n\t"\
		"vmulpd	    (%%rdi),%%ymm2,%%ymm2	\n\t"\
		"vmulpd	    (%%rdi),%%ymm3,%%ymm3	\n\t"\
		"vsubpd	%%ymm2,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm3,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm2,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm3,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm0,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm1,%%ymm3,%%ymm3		\n\t"\
		"vsubpd	%%ymm6,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm7,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm0,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rcx)	\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm0,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm1,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	%%ymm6,    (%%rax)	\n\t"\
		"vmovaps	%%ymm7,0x20(%%rax)	\n\t"\
		"vsubpd	%%ymm5,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm4,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm2,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rbx)	\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm2,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm3,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm4,0x20(%%rdx)	\n\t"\
		:					/* outputs: none */\
		:[__in0] "m" (Xin0)	/* All inputs from memory addresses here */\
		,[__in1] "m" (Xin1)\
		,[__in2] "m" (Xin2)\
		,[__in3] "m" (Xin3)\
		,[__in4] "m" (Xin4)\
		,[__in5] "m" (Xin5)\
		,[__in6] "m" (Xin6)\
		,[__in7] "m" (Xin7)\
		,[__in8] "m" (Xin8)\
		,[__in9] "m" (Xin9)\
		,[__ina] "m" (Xina)\
		,[__inb] "m" (Xinb)\
		,[__inc] "m" (Xinc)\
		,[__ind] "m" (Xind)\
		,[__ine] "m" (Xine)\
		,[__inf] "m" (Xinf)\
		,[__isrt2] "m" (Xisrt2)\
		,[__two] "m" (Xtwo)\
		,[__out0] "m" (Xout0)\
		,[__o1] "e" (Xo1)\
		,[__o2] "e" (Xo2)\
		,[__o3] "e" (Xo3)\
		,[__o4] "e" (Xo4)\
		: "cc","memory","rax","rbx","rcx","rdx","rdi","rsi","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

  #endif	// AVX2/FMA3?

	/* With-twiddles out-of-place analog of above twiddleless DIT macro: 15 nontrivial complex input twiddles E1-f [E0 assumed = 1],
	The DIT version of this macro processes the twiddles in-order.
	NOTE: SINCE THIS MACRO IS SPECIFICALLY DESIGNED AS THE 2ND-PASS OF LARGE-POWER-OF-2-TWIDDLELESS DFT SYNTHESIS, THE
	"TWIDDLES" HERE ARE PURE OF THE DFT-INTERNAL VARIETY, AND THUS APPLIED TO THE INPUTS, JUST AS FOR THE ABOVE DIF COUNTERPART.

	Sincos layout - Two portions:
	[NOTE: bytewise offsets below are w.r.to SSE2 version of code; AVX doubles these]

	Radix-16 shared consts anchored at isrt2:

	  isrt2 + 0x000;	cc0 + 0x010;	ss0 + 0x020;

	Per-block-specific set of 15 complex twiddles anchored at c1:

		c1  + 0x000;	s1  + 0x010;
		c2  + 0x020;	s2  + 0x030;
		c3  + 0x040;	s3  + 0x050;
		c4  + 0x060;	s4  + 0x070;
		c5  + 0x080;	s5  + 0x090;
		c6  + 0x0a0;	s6  + 0x0b0;
		c7  + 0x0c0;	s7  + 0x0d0;
		c8  + 0x0e0;	s8  + 0x0f0;
		c9  + 0x100;	s9  + 0x110;
		c10 + 0x120;	s10 + 0x130;
		c11 + 0x140;	s11 + 0x150;
		c12 + 0x160;	s12 + 0x170;
		c13 + 0x180;	s13 + 0x190;
		c14 + 0x1a0;	s14 + 0x1b0;
		c15 + 0x1c0;	s15 + 0x1d0;

	Use radix-16 DIF as template for DIT/OOP here, since need a pre-twiddles algorithm:
	*/
	#define SSE2_RADIX16_DIT_TWIDDLE_OOP(Xin0,Xi1,Xi2,Xi3,Xi4, Xout0,Xo1,Xo2,Xo3,Xo4, Xisrt2,Xc1)\
	{\
	__asm__ volatile (\
	/*...Block 0: Do in-place, i.e. outputs into __in0 + [0,1,2,3]*istride: */\
		"movq	%[__in0],%%rax		\n\t"\
		"leaq	%c[__i1](%%rax),%%rcx	\n\t"/* __in0 +   istride */\
		"leaq	%c[__i2](%%rax),%%rbx	\n\t"/* __in0 + 2*istride */\
		"leaq	%c[__i3](%%rax),%%rdx	\n\t"/* __in0 + 3*istride */\
		"/* Do	the p0,1 combo: */	\n\t"\
		"movq	%[__c1],%%rsi 	/* c1 */\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmulpd	    (%%rsi),%%ymm4,%%ymm4	\n\t"\
		"vmulpd	    (%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmovaps	%%ymm0,%%ymm2		\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm1,%%ymm3		\n\t"\
		"vaddpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vaddpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm4,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm5,%%ymm3,%%ymm3		\n\t"\
		"/* Do	the p2,3 combo: */	\n\t"\
		"addq	$0x40,%%rsi 	/* c2,3 */\n\t"\
		"vmovaps	    (%%rdx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5	\n\t"\
		"vmovaps	    (%%rdx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7	\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm4,%%ymm4	/* c3 */\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)	\n\t"\
		"vmovaps	%%ymm4,    (%%rax)	\n\t"\
		"vmovaps	    (%%rbx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm5	\n\t"\
		"vmovaps	    (%%rbx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm7	\n\t"\
		"vmulpd	    (%%rsi),%%ymm4,%%ymm4	/* c2 */\n\t"\
		"vmulpd	    (%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vsubpd	    (%%rax),%%ymm4,%%ymm4	\n\t"\
		"vsubpd	0x20(%%rax),%%ymm5,%%ymm5	\n\t"\
		"vaddpd	    (%%rax),%%ymm6,%%ymm6	\n\t"\
		"vaddpd	0x20(%%rax),%%ymm7,%%ymm7	\n\t"\
		"/* Finish radix-4 bfly, store results: */\n\t"\
		"vsubpd	%%ymm6,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm5,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm7,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm4,%%ymm3,%%ymm3		\n\t"\
		/* DIT has outputs (indexed in real-temp form as 0-7) 2/6,3/7 swapped, i.e. swap oregs c/d vs DIF: */\
		"vmovaps	%%ymm0,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm2,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rcx)	\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm0,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm2,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm1,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm3,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm6,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm7,0x20(%%rax)	\n\t"\
		"vmovaps	%%ymm4,0x20(%%rdx)	\n\t"\
		"\n\t"\
	/*...Block 1: outputs into __in0 + [4,5,6,7]*istride: */\
		"addq	$%c[__i4],%%rax	\n\t"/* __in0 + 4*istride */\
		"addq	$%c[__i4],%%rcx	\n\t"/* __in0 + 5*istride */\
		"addq	$%c[__i4],%%rbx	\n\t"/* __in0 + 6*istride */\
		"addq	$%c[__i4],%%rdx	\n\t"/* __in0 + 7*istride */\
		"/* Do	the p0,1 combo: */	\n\t"\
		"addq	$0x80,%%rsi 	/* c4,5 */\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rsi),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rsi),%%ymm7	\n\t"\
		"vmovaps	%%ymm0,%%ymm2		\n\t"\
		"vmovaps	%%ymm1,%%ymm3		\n\t"\
		"vmulpd	%%ymm6,%%ymm0,%%ymm0		/* c4 */\n\t"\
		"vmulpd	%%ymm6,%%ymm1,%%ymm1		\n\t"\
		"vmulpd	%%ymm7,%%ymm2,%%ymm2		\n\t"\
		"vmulpd	%%ymm7,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vsubpd	%%ymm2,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm4,%%ymm4	/* c5 */\n\t"\
		"vaddpd	%%ymm3,%%ymm0,%%ymm0		\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmovaps	%%ymm0,%%ymm2		\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm1,%%ymm3		\n\t"\
		"vaddpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vaddpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm4,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm5,%%ymm3,%%ymm3		\n\t"\
		"/* Do	the p2,3 combo: */	\n\t"\
		"addq	$0x80,%%rsi 	/* c6,7 */\n\t"\
		"vmovaps	    (%%rdx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5	\n\t"\
		"vmovaps	    (%%rdx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7	\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm4,%%ymm4	/* c7 */\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)	\n\t"\
		"vmovaps	%%ymm4,    (%%rax)	\n\t"\
		"vmovaps	    (%%rbx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm5	\n\t"\
		"vmovaps	    (%%rbx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm7	\n\t"\
		"vmulpd	    (%%rsi),%%ymm4,%%ymm4	/* c6 */\n\t"\
		"vmulpd	    (%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vsubpd	    (%%rax),%%ymm4,%%ymm4	\n\t"\
		"vsubpd	0x20(%%rax),%%ymm5,%%ymm5	\n\t"\
		"vaddpd	    (%%rax),%%ymm6,%%ymm6	\n\t"\
		"vaddpd	0x20(%%rax),%%ymm7,%%ymm7	\n\t"\
		"/* Finish radix-4 bfly, store results: */\n\t"\
		"vsubpd	%%ymm6,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm5,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm7,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm4,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm0,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm2,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rcx)	\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm0,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm2,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm1,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm3,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm6,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm7,0x20(%%rax)	\n\t"\
		"vmovaps	%%ymm4,0x20(%%rdx)	\n\t"\
		"\n\t"\
	/*...Block 2: outputs into __in0 + [8,9,a,b]*istride: */\
		"addq	$%c[__i4],%%rax	\n\t"/* __in0 + 8*istride */\
		"addq	$%c[__i4],%%rcx	\n\t"/* __in0 + 9*istride */\
		"addq	$%c[__i4],%%rbx	\n\t"/* __in0 + a*istride */\
		"addq	$%c[__i4],%%rdx	\n\t"/* __in0 + b*istride */\
		"/* Do	the p0,1 combo: */	\n\t"\
		"addq	$0x80,%%rsi 	/* c8,9 */\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rsi),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rsi),%%ymm7	\n\t"\
		"vmovaps	%%ymm0,%%ymm2		\n\t"\
		"vmovaps	%%ymm1,%%ymm3		\n\t"\
		"vmulpd	%%ymm6,%%ymm0,%%ymm0		/* c8 */\n\t"\
		"vmulpd	%%ymm6,%%ymm1,%%ymm1		\n\t"\
		"vmulpd	%%ymm7,%%ymm2,%%ymm2		\n\t"\
		"vmulpd	%%ymm7,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vsubpd	%%ymm2,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm4,%%ymm4	/* c9 */\n\t"\
		"vaddpd	%%ymm3,%%ymm0,%%ymm0		\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmovaps	%%ymm0,%%ymm2		\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm1,%%ymm3		\n\t"\
		"vaddpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vaddpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm4,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm5,%%ymm3,%%ymm3		\n\t"\
		"/* Do	the p2,3 combo: */	\n\t"\
		"addq	$0x80,%%rsi 	/* ca,b */\n\t"\
		"vmovaps	    (%%rdx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5	\n\t"\
		"vmovaps	    (%%rdx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7	\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm4,%%ymm4	/* cb */\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)	\n\t"\
		"vmovaps	%%ymm4,    (%%rax)	\n\t"\
		"vmovaps	    (%%rbx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm5	\n\t"\
		"vmovaps	    (%%rbx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm7	\n\t"\
		"vmulpd	    (%%rsi),%%ymm4,%%ymm4	/* ca */\n\t"\
		"vmulpd	    (%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vsubpd	    (%%rax),%%ymm4,%%ymm4	\n\t"\
		"vsubpd	0x20(%%rax),%%ymm5,%%ymm5	\n\t"\
		"vaddpd	    (%%rax),%%ymm6,%%ymm6	\n\t"\
		"vaddpd	0x20(%%rax),%%ymm7,%%ymm7	\n\t"\
		"/* Finish radix-4 bfly, store results: */\n\t"\
		"vsubpd	%%ymm6,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm5,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm7,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm4,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm0,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm2,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rcx)	\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm0,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm2,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm1,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm3,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm6,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm7,0x20(%%rax)	\n\t"\
		"vmovaps	%%ymm4,0x20(%%rdx)	\n\t"\
		"\n\t"\
	/*...Block 3: outputs into __in0 + [c,d,e,f]*istride: */\
		"addq	$%c[__i4],%%rax	\n\t"/* __in0 + c*istride */\
		"addq	$%c[__i4],%%rcx	\n\t"/* __in0 + d*istride */\
		"addq	$%c[__i4],%%rbx	\n\t"/* __in0 + e*istride */\
		"addq	$%c[__i4],%%rdx	\n\t"/* __in0 + f*istride */\
		"/* Do	the p0,1 combo: */	\n\t"\
		"addq	$0x80,%%rsi 	/* cc,d */\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rsi),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rsi),%%ymm7	\n\t"\
		"vmovaps	%%ymm0,%%ymm2		\n\t"\
		"vmovaps	%%ymm1,%%ymm3		\n\t"\
		"vmulpd	%%ymm6,%%ymm0,%%ymm0		/* cc */\n\t"\
		"vmulpd	%%ymm6,%%ymm1,%%ymm1		\n\t"\
		"vmulpd	%%ymm7,%%ymm2,%%ymm2		\n\t"\
		"vmulpd	%%ymm7,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vsubpd	%%ymm2,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm4,%%ymm4	/* cd */\n\t"\
		"vaddpd	%%ymm3,%%ymm0,%%ymm0		\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmovaps	%%ymm0,%%ymm2		\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm1,%%ymm3		\n\t"\
		"vaddpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vaddpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm4,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm5,%%ymm3,%%ymm3		\n\t"\
		"/* Do	the p2,3 combo: */	\n\t"\
		"addq	$0x80,%%rsi 	/* ce,f */\n\t"\
		"vmovaps	    (%%rdx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5	\n\t"\
		"vmovaps	    (%%rdx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7	\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm4,%%ymm4	/* cf */\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)	\n\t"\
		"vmovaps	%%ymm4,    (%%rax)	\n\t"\
		"vmovaps	    (%%rbx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm5	\n\t"\
		"vmovaps	    (%%rbx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm7	\n\t"\
		"vmulpd	    (%%rsi),%%ymm4,%%ymm4	/* ce */\n\t"\
		"vmulpd	    (%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vsubpd	    (%%rax),%%ymm4,%%ymm4	\n\t"\
		"vsubpd	0x20(%%rax),%%ymm5,%%ymm5	\n\t"\
		"vaddpd	    (%%rax),%%ymm6,%%ymm6	\n\t"\
		"vaddpd	0x20(%%rax),%%ymm7,%%ymm7	\n\t"\
		"/* Finish radix-4 bfly, store results: */\n\t"\
		"vsubpd	%%ymm6,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm5,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm7,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm4,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm0,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm2,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rcx)	\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm0,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm2,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm1,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm3,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm6,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm7,0x20(%%rax)	\n\t"\
		"vmovaps	%%ymm4,0x20(%%rdx)	\n\t"\
	"/*************************************************************************************/\n\t"\
	"/*  And now do four more radix-4 transforms, including the internal twiddle factors: */\n\t"\
	"/*************************************************************************************/\n\t"\
		"movq	%[__isrt2],%%rsi 	\n\t"\
	/* Block 0: Combine 0-output of each radix-4, i.e. inputs from __in0 + [0,4,8,c]*istride: */\
		"movq	%[__in0],%%rax		\n\t"\
		"leaq	%c[__i4](%%rax),%%rbx	\n\t"/* __in0 +   [4*istride] */\
		"leaq	%c[__i4](%%rbx),%%rcx	\n\t"/* __in0 + 2*[4*istride] */\
		"leaq	%c[__i4](%%rcx),%%rdx	\n\t"/* __in0 + 3*[4*istride] */\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rbx),%%ymm2	\n\t"\
		"vmovaps	    (%%rdx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7	\n\t"\
		"vsubpd	%%ymm2,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm6,%%ymm4,%%ymm4		\n\t"\
		"vsubpd	%%ymm3,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm7,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm2,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm3,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm0,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm4,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm1,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm5,%%ymm7,%%ymm7		\n\t"\
		"vsubpd	%%ymm6,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm7,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"movq	%[__out0],%%r10		\n\t"\
		"leaq	%c[__o4](%%r10),%%r11	\n\t"/* __out0 + 4*ostride */\
		"leaq	%c[__o4](%%r11),%%r12	\n\t"/* __out0 + 8*ostride */\
		"leaq	%c[__o4](%%r12),%%r13	\n\t"/* __out0 + c*ostride */\
		"vmovaps	%%ymm2,    (%%r12)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%r12)	\n\t"\
		"vaddpd	%%ymm2,%%ymm6,%%ymm6	\n\t"\
		"vaddpd	%%ymm3,%%ymm7,%%ymm7	\n\t"\
		"vmovaps	%%ymm6,    (%%r10)	\n\t"\
		"vmovaps	%%ymm7,0x20(%%r10)	\n\t"\
		"vsubpd	%%ymm5,%%ymm0,%%ymm0	\n\t"\
		"vsubpd	%%ymm4,%%ymm1,%%ymm1	\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4	\n\t"\
		"vmovaps	%%ymm0,    (%%r13)	\n\t"/* These 2 outputs [4/c] swapped w.r.to dif [2/3] due to +-I sign diff */\
		"vmovaps	%%ymm1,0x20(%%r11)	\n\t"\
		"vaddpd	%%ymm0,%%ymm5,%%ymm5	\n\t"\
		"vaddpd	%%ymm1,%%ymm4,%%ymm4	\n\t"\
		"vmovaps	%%ymm5,    (%%r11)	\n\t"\
		"vmovaps	%%ymm4,0x20(%%r13)	\n\t"\
	/* Block 1: Combine 1-output of each radix-4, i.e. inputs from __in0 + [1,5,9,d]*istride: */\
		"addq	$%c[__i1],%%rax	\n\t"/* __in0 + 1*istride */\
		"addq	$%c[__i1],%%rcx	\n\t"/* __in0 + 5*istride */\
		"addq	$%c[__i1],%%rbx	\n\t"/* __in0 + 9*istride */\
		"addq	$%c[__i1],%%rdx	\n\t"/* __in0 + d*istride */\
		"vmovaps	    (%%rdx),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm1	\n\t"\
		"vmovaps	    (%%rdx),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm3	\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm0,%%ymm0	\n\t"/* ss0 */\
		"vmulpd	0x40(%%rsi),%%ymm1,%%ymm1	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm2,%%ymm2	\n\t"/* cc0 */\
		"vmulpd	0x20(%%rsi),%%ymm3,%%ymm3	\n\t"\
		"vsubpd	%%ymm2,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm3,%%ymm0,%%ymm0		\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm4,%%ymm4	\n\t"/* cc0 */\
		"vmulpd	0x20(%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm6,%%ymm6	\n\t"/* ss0 */\
		"vmulpd	0x40(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vsubpd	%%ymm0,%%ymm6,%%ymm6		\n\t"\
		"vsubpd	%%ymm1,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	    (%%rbx),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3	\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vaddpd	0x20(%%rbx),%%ymm2,%%ymm2	\n\t"\
		"vsubpd	    (%%rbx),%%ymm3,%%ymm3	\n\t"\
		"vmulpd	    (%%rsi),%%ymm2,%%ymm2	\n\t"/* isrt2 */\
		"vmulpd	    (%%rsi),%%ymm3,%%ymm3	\n\t"\
		"vsubpd	%%ymm2,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm3,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm2,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm3,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm0,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm1,%%ymm3,%%ymm3		\n\t"\
		"addq	$%c[__o1],%%r10	\n\t"/* __out0 + 1*ostride */\
		"addq	$%c[__o1],%%r11	\n\t"/* __out0 + 5*ostride */\
		"addq	$%c[__o1],%%r12	\n\t"/* __out0 + 9*ostride */\
		"addq	$%c[__o1],%%r13	\n\t"/* __out0 + d*ostride */\
		"vsubpd	%%ymm4,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm5,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm2,    (%%r12)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%r12)	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm2,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm3,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm4,    (%%r10)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%r10)	\n\t"\
		"vsubpd	%%ymm7,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm6,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm0,    (%%r13)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%r11)	\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm0,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm1,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	%%ymm7,    (%%r11)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%r13)	\n\t"\
	/* Block 2: Combine 2-output of each radix-4, i.e. inputs from __in0 + [2,6,a,e]*istride: */\
		"vmovaps	(%%rsi),%%ymm2	/* isrt2 */\n\t"\
		"addq	$%c[__i1],%%rax	\n\t"/* __in0 + 2*istride */\
		"addq	$%c[__i1],%%rbx	\n\t"/* __in0 + 6*istride */\
		"addq	$%c[__i1],%%rcx	\n\t"/* __in0 + a*istride */\
		"addq	$%c[__i1],%%rdx	\n\t"/* __in0 + e*istride */\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rdx),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm1	\n\t"\
		"vaddpd	0x20(%%rcx),%%ymm4,%%ymm4	\n\t"\
		"vsubpd	    (%%rcx),%%ymm5,%%ymm5	\n\t"\
		"vsubpd	0x20(%%rdx),%%ymm0,%%ymm0	\n\t"\
		"vaddpd	    (%%rdx),%%ymm1,%%ymm1	\n\t"\
		"vmulpd	%%ymm2,%%ymm4,%%ymm4		\n\t"\
		"vmulpd	%%ymm2,%%ymm5,%%ymm5		\n\t"\
		"vmulpd	%%ymm2,%%ymm0,%%ymm0		\n\t"\
		"vmulpd	%%ymm2,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vsubpd	%%ymm0,%%ymm4,%%ymm4		\n\t"\
		"vsubpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm0,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm1,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps	    (%%rbx),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm0,%%ymm0	\n\t"\
		"vsubpd	    (%%rbx),%%ymm1,%%ymm1	\n\t"\
		"vaddpd	    (%%rax),%%ymm3,%%ymm3	\n\t"\
		"vaddpd	0x20(%%rax),%%ymm2,%%ymm2	\n\t"\
		"addq	$%c[__o1],%%r10	\n\t"/* __out0 + 2*ostride */\
		"addq	$%c[__o1],%%r11	\n\t"/* __out0 + 6*ostride */\
		"addq	$%c[__o1],%%r12	\n\t"/* __out0 + a*ostride */\
		"addq	$%c[__o1],%%r13	\n\t"/* __out0 + e*ostride */\
		"vsubpd	%%ymm4,%%ymm3,%%ymm3		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm3,    (%%r12)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%r12)	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm3,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm4,    (%%r10)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%r10)	\n\t"\
		"vsubpd	%%ymm7,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm6,%%ymm2,%%ymm2		\n\t"\
		"vmovaps	%%ymm0,    (%%r13)	\n\t"\
		"vmovaps	%%ymm2,0x20(%%r11)	\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm0,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm2,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	%%ymm7,    (%%r11)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%r13)	\n\t"\
	/* Block 3: Combine 3-output of each radix-4, i.e. inputs from __in0 + [3,7,b,f]*istride: */\
		"addq	$%c[__i1],%%rax	\n\t"/* __in0 + 3*istride */\
		"addq	$%c[__i1],%%rcx	\n\t"/* __in0 + 7*istride */\
		"addq	$%c[__i1],%%rbx	\n\t"/* __in0 + b*istride */\
		"addq	$%c[__i1],%%rdx	\n\t"/* __in0 + f*istride */\
		"vmovaps	    (%%rdx),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm1	\n\t"\
		"vmovaps	    (%%rdx),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm3	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm0,%%ymm0	\n\t"/* cc0 */\
		"vmulpd	0x20(%%rsi),%%ymm1,%%ymm1	\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm2,%%ymm2	\n\t"/* ss0 */\
		"vmulpd	0x40(%%rsi),%%ymm3,%%ymm3	\n\t"\
		"vsubpd	%%ymm2,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm3,%%ymm0,%%ymm0		\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rcx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7	\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm4,%%ymm4	\n\t"/* ss0 */\
		"vmulpd	0x40(%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm6,%%ymm6	\n\t"/* cc0 */\
		"vmulpd	0x20(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vsubpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vsubpd	%%ymm0,%%ymm6,%%ymm6		\n\t"\
		"vsubpd	%%ymm1,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	    (%%rbx),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3	\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm2,%%ymm2	\n\t"\
		"vaddpd	    (%%rbx),%%ymm3,%%ymm3	\n\t"\
		"vmulpd	    (%%rsi),%%ymm2,%%ymm2	\n\t"/* isrt2 */\
		"vmulpd	    (%%rsi),%%ymm3,%%ymm3	\n\t"\
		"vsubpd	%%ymm2,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm3,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm2,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm3,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm0,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm1,%%ymm3,%%ymm3		\n\t"\
		"addq	$%c[__o1],%%r10	\n\t"/* __out0 + 3*ostride */\
		"addq	$%c[__o1],%%r12	\n\t"/* __out0 + 7*ostride */\
		"addq	$%c[__o1],%%r11	\n\t"/* __out0 + b*ostride */\
		"addq	$%c[__o1],%%r13	\n\t"/* __out0 + f*ostride */\
		"vsubpd	%%ymm6,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm7,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm0,    (%%r12)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%r12)	\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm0,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm1,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	%%ymm6,    (%%r10)	\n\t"\
		"vmovaps	%%ymm7,0x20(%%r10)	\n\t"\
		"vsubpd	%%ymm5,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm4,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm2,    (%%r13)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%r11)	\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm2,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm3,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,    (%%r11)	\n\t"\
		"vmovaps	%%ymm4,0x20(%%r13)	\n\t"\
		:					/* outputs: none */\
		: [__in0] "m" (Xin0)	/* All inputs from memory addresses here */\
		 ,[__i1] "e" (Xi1)\
		 ,[__i2] "e" (Xi2)\
		 ,[__i3] "e" (Xi3)\
		 ,[__i4] "e" (Xi4)\
		 ,[__out0] "m" (Xout0)\
		 ,[__o1] "e" (Xo1)\
		 ,[__o2] "e" (Xo2)\
		 ,[__o3] "e" (Xo3)\
		 ,[__o4] "e" (Xo4)\
		 ,[__isrt2] "m" (Xisrt2)\
		 ,[__c1] "m" (Xc1)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

	// DIF version of above shares same sincos layout & data:
	#define SSE2_RADIX16_DIF_TWIDDLE_OOP(Xin0,Xi1,Xi2,Xi3,Xi4, Xout0,Xout1,Xout2,Xout3,Xout4,Xout5,Xout6,Xout7,Xout8,Xout9,Xouta,Xoutb,Xoutc,Xoutd,Xoute,Xoutf, Xisrt2,Xc1)\
	{\
	__asm__ volatile (\
	/*...Block 0: Do in-place, i.e. outputs into __in0 + [0,1,2,3]*istride: */\
		"movq	%[__in0],%%rax		\n\t"\
		"leaq	%c[__i1](%%rax),%%rcx	\n\t"/* __in0 +   istride */\
		"leaq	%c[__i2](%%rax),%%rbx	\n\t"/* __in0 + 2*istride */\
		"leaq	%c[__i3](%%rax),%%rdx	\n\t"/* __in0 + 3*istride */\
		"/* Do	the p0,1 combo: */	\n\t"\
		"movq	%[__c1],%%rsi 	/* Roots sets c1-15 same as for DIT, w/c1 as base-ptr */\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmulpd	    (%%rsi),%%ymm4,%%ymm4	\n\t"\
		"vmulpd	    (%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmovaps	%%ymm0,%%ymm2		\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm1,%%ymm3		\n\t"\
		"vsubpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vaddpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm4,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm5,%%ymm3,%%ymm3		\n\t"\
		"/* Do	the p2,3 combo: */	\n\t"\
		"addq	$0x40,%%rsi 	/* c2,3 */\n\t"\
		"vmovaps	    (%%rdx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5	\n\t"\
		"vmovaps	    (%%rdx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7	\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm4,%%ymm4	/* c3 */\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vsubpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)	\n\t"\
		"vmovaps	%%ymm4,    (%%rax)	\n\t"\
		"vmovaps	    (%%rbx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm5	\n\t"\
		"vmovaps	    (%%rbx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm7	\n\t"\
		"vmulpd	    (%%rsi),%%ymm4,%%ymm4	/* c2 */\n\t"\
		"vmulpd	    (%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vsubpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vsubpd	    (%%rax),%%ymm4,%%ymm4	\n\t"\
		"vsubpd	0x20(%%rax),%%ymm5,%%ymm5	\n\t"\
		"vaddpd	    (%%rax),%%ymm6,%%ymm6	\n\t"\
		"vaddpd	0x20(%%rax),%%ymm7,%%ymm7	\n\t"\
		"/* Finish radix-4 bfly, store results: */\n\t"\
		"vsubpd	%%ymm6,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm5,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm7,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm4,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm0,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm2,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rcx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rdx)	\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm0,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm2,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm1,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm3,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm6,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm7,0x20(%%rax)	\n\t"\
		"vmovaps	%%ymm4,0x20(%%rbx)	\n\t"\
		"\n\t"\
	/*...Block 1: outputs into __in0 + [4,5,6,7]*istride: */\
		"addq	$%c[__i4],%%rax	\n\t"/* __in0 + 4*istride */\
		"addq	$%c[__i4],%%rcx	\n\t"/* __in0 + 5*istride */\
		"addq	$%c[__i4],%%rbx	\n\t"/* __in0 + 6*istride */\
		"addq	$%c[__i4],%%rdx	\n\t"/* __in0 + 7*istride */\
		"/* Do	the p0,1 combo: */	\n\t"\
		"addq	$0x80,%%rsi 	/* c4,5 */\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rsi),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rsi),%%ymm7	\n\t"\
		"vmovaps	%%ymm0,%%ymm2		\n\t"\
		"vmovaps	%%ymm1,%%ymm3		\n\t"\
		"vmulpd	%%ymm6,%%ymm0,%%ymm0		/* c4 */\n\t"\
		"vmulpd	%%ymm6,%%ymm1,%%ymm1		\n\t"\
		"vmulpd	%%ymm7,%%ymm2,%%ymm2		\n\t"\
		"vmulpd	%%ymm7,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vaddpd	%%ymm2,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm4,%%ymm4	/* c5 */\n\t"\
		"vsubpd	%%ymm3,%%ymm0,%%ymm0		\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmovaps	%%ymm0,%%ymm2		\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm1,%%ymm3		\n\t"\
		"vsubpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vaddpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm4,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm5,%%ymm3,%%ymm3		\n\t"\
		"/* Do	the p2,3 combo: */	\n\t"\
		"addq	$0x80,%%rsi 	/* c6,7 */\n\t"\
		"vmovaps	    (%%rdx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5	\n\t"\
		"vmovaps	    (%%rdx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7	\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm4,%%ymm4	/* c7 */\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vsubpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)	\n\t"\
		"vmovaps	%%ymm4,    (%%rax)	\n\t"\
		"vmovaps	    (%%rbx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm5	\n\t"\
		"vmovaps	    (%%rbx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm7	\n\t"\
		"vmulpd	    (%%rsi),%%ymm4,%%ymm4	/* c6 */\n\t"\
		"vmulpd	    (%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vsubpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vsubpd	    (%%rax),%%ymm4,%%ymm4	\n\t"\
		"vsubpd	0x20(%%rax),%%ymm5,%%ymm5	\n\t"\
		"vaddpd	    (%%rax),%%ymm6,%%ymm6	\n\t"\
		"vaddpd	0x20(%%rax),%%ymm7,%%ymm7	\n\t"\
		"/* Finish radix-4 bfly, store results: */\n\t"\
		"vsubpd	%%ymm6,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm5,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm7,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm4,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm0,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm2,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rcx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rdx)	\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm0,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm2,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm1,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm3,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm6,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm7,0x20(%%rax)	\n\t"\
		"vmovaps	%%ymm4,0x20(%%rbx)	\n\t"\
		"\n\t"\
	/*...Block 2: outputs into __in0 + [8,9,a,b]*istride: */\
		"addq	$%c[__i4],%%rax	\n\t"/* __in0 + 8*istride */\
		"addq	$%c[__i4],%%rcx	\n\t"/* __in0 + 9*istride */\
		"addq	$%c[__i4],%%rbx	\n\t"/* __in0 + a*istride */\
		"addq	$%c[__i4],%%rdx	\n\t"/* __in0 + b*istride */\
		"/* Do	the p0,1 combo: */	\n\t"\
		"addq	$0x80,%%rsi 	/* c8,9 */\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rsi),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rsi),%%ymm7	\n\t"\
		"vmovaps	%%ymm0,%%ymm2		\n\t"\
		"vmovaps	%%ymm1,%%ymm3		\n\t"\
		"vmulpd	%%ymm6,%%ymm0,%%ymm0		/* c8 */\n\t"\
		"vmulpd	%%ymm6,%%ymm1,%%ymm1		\n\t"\
		"vmulpd	%%ymm7,%%ymm2,%%ymm2		\n\t"\
		"vmulpd	%%ymm7,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vaddpd	%%ymm2,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm4,%%ymm4	/* c9 */\n\t"\
		"vsubpd	%%ymm3,%%ymm0,%%ymm0		\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmovaps	%%ymm0,%%ymm2		\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm1,%%ymm3		\n\t"\
		"vsubpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vaddpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm4,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm5,%%ymm3,%%ymm3		\n\t"\
		"/* Do	the p2,3 combo: */	\n\t"\
		"addq	$0x80,%%rsi 	/* ca,b */\n\t"\
		"vmovaps	    (%%rdx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5	\n\t"\
		"vmovaps	    (%%rdx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7	\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm4,%%ymm4	/* cb */\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vsubpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)	\n\t"\
		"vmovaps	%%ymm4,    (%%rax)	\n\t"\
		"vmovaps	    (%%rbx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm5	\n\t"\
		"vmovaps	    (%%rbx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm7	\n\t"\
		"vmulpd	    (%%rsi),%%ymm4,%%ymm4	/* ca */\n\t"\
		"vmulpd	    (%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vsubpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vsubpd	    (%%rax),%%ymm4,%%ymm4	\n\t"\
		"vsubpd	0x20(%%rax),%%ymm5,%%ymm5	\n\t"\
		"vaddpd	    (%%rax),%%ymm6,%%ymm6	\n\t"\
		"vaddpd	0x20(%%rax),%%ymm7,%%ymm7	\n\t"\
		"/* Finish radix-4 bfly, store results: */\n\t"\
		"vsubpd	%%ymm6,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm5,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm7,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm4,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm0,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm2,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rcx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rdx)	\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm0,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm2,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm1,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm3,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm6,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm7,0x20(%%rax)	\n\t"\
		"vmovaps	%%ymm4,0x20(%%rbx)	\n\t"\
		"\n\t"\
	/*...Block 3: outputs into __in0 + [c,d,e,f]*istride: */\
		"addq	$%c[__i4],%%rax	\n\t"/* __in0 + c*istride */\
		"addq	$%c[__i4],%%rcx	\n\t"/* __in0 + d*istride */\
		"addq	$%c[__i4],%%rbx	\n\t"/* __in0 + e*istride */\
		"addq	$%c[__i4],%%rdx	\n\t"/* __in0 + f*istride */\
		"/* Do	the p0,1 combo: */	\n\t"\
		"addq	$0x80,%%rsi 	/* cc,d */\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rsi),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rsi),%%ymm7	\n\t"\
		"vmovaps	%%ymm0,%%ymm2		\n\t"\
		"vmovaps	%%ymm1,%%ymm3		\n\t"\
		"vmulpd	%%ymm6,%%ymm0,%%ymm0		/* cc */\n\t"\
		"vmulpd	%%ymm6,%%ymm1,%%ymm1		\n\t"\
		"vmulpd	%%ymm7,%%ymm2,%%ymm2		\n\t"\
		"vmulpd	%%ymm7,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vaddpd	%%ymm2,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm4,%%ymm4	/* cd */\n\t"\
		"vsubpd	%%ymm3,%%ymm0,%%ymm0		\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmovaps	%%ymm0,%%ymm2		\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm1,%%ymm3		\n\t"\
		"vsubpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vaddpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm4,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm5,%%ymm3,%%ymm3		\n\t"\
		"/* Do	the p2,3 combo: */	\n\t"\
		"addq	$0x80,%%rsi 	/* ce,f */\n\t"\
		"vmovaps	    (%%rdx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm5	\n\t"\
		"vmovaps	    (%%rdx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7	\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm4,%%ymm4	/* cf */\n\t"\
		"vmulpd	0x40(%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmulpd	0x60(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vsubpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)	\n\t"\
		"vmovaps	%%ymm4,    (%%rax)	\n\t"\
		"vmovaps	    (%%rbx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm5	\n\t"\
		"vmovaps	    (%%rbx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm7	\n\t"\
		"vmulpd	    (%%rsi),%%ymm4,%%ymm4	/* ce */\n\t"\
		"vmulpd	    (%%rsi),%%ymm5,%%ymm5	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm6,%%ymm6	\n\t"\
		"vmulpd	0x20(%%rsi),%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vsubpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vsubpd	    (%%rax),%%ymm4,%%ymm4	\n\t"\
		"vsubpd	0x20(%%rax),%%ymm5,%%ymm5	\n\t"\
		"vaddpd	    (%%rax),%%ymm6,%%ymm6	\n\t"\
		"vaddpd	0x20(%%rax),%%ymm7,%%ymm7	\n\t"\
		"/* Finish radix-4 bfly, store results: */\n\t"\
		"vsubpd	%%ymm6,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm5,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm7,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm4,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm0,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm2,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rcx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rdx)	\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm0,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm2,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm1,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm3,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm6,    (%%rax)	\n\t"\
		"vmovaps	%%ymm5,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm7,0x20(%%rax)	\n\t"\
		"vmovaps	%%ymm4,0x20(%%rbx)	\n\t"\
	"/*************************************************************************************/\n\t"\
	"/*  And now do four more radix-4 transforms, including the internal twiddle factors: */\n\t"\
	"/*************************************************************************************/\n\t"\
	/* Block 0: Combine 0-output of each radix-4, i.e. inputs from __in0 + [0,4,8,c]*istride: */\
		"movq	%[__in0],%%rax		\n\t"\
		"leaq	%c[__i4](%%rax),%%rbx	\n\t"/* __in0 +   [4*istride] */\
		"leaq	%c[__i4](%%rbx),%%rcx	\n\t"/* __in0 + 2*[4*istride] */\
		"leaq	%c[__i4](%%rcx),%%rdx	\n\t"/* __in0 + 3*[4*istride] */\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rbx),%%ymm2	\n\t"\
		"vmovaps	    (%%rdx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7	\n\t"\
		"vsubpd	%%ymm2,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm6,%%ymm4,%%ymm4		\n\t"\
		"vsubpd	%%ymm3,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm7,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm2,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm3,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm0,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm4,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm1,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm5,%%ymm7,%%ymm7		\n\t"\
		"vsubpd	%%ymm6,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm7,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"movq	%[__out0],%%r10		\n\t"\
		"movq	%[__out1],%%r11		\n\t"\
		"movq	%[__out2],%%r12		\n\t"\
		"movq	%[__out3],%%r13		\n\t"\
	"prefetcht1	0x100(%%r11)\n\t"\
		"vmovaps	%%ymm2,    (%%r11)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%r11)	\n\t"\
		"vaddpd	%%ymm2,%%ymm6,%%ymm6	\n\t"\
		"vaddpd	%%ymm3,%%ymm7,%%ymm7	\n\t"\
		"vmovaps	%%ymm6,    (%%r10)	\n\t"\
		"vmovaps	%%ymm7,0x20(%%r10)	\n\t"\
		"vsubpd	%%ymm5,%%ymm0,%%ymm0	\n\t"\
		"vsubpd	%%ymm4,%%ymm1,%%ymm1	\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4	\n\t"\
		"vmovaps	%%ymm0,    (%%r12)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%r13)	\n\t"\
		"vaddpd	%%ymm0,%%ymm5,%%ymm5	\n\t"\
		"vaddpd	%%ymm1,%%ymm4,%%ymm4	\n\t"\
		"vmovaps	%%ymm5,    (%%r13)	\n\t"\
		"vmovaps	%%ymm4,0x20(%%r12)	\n\t"\
	"/* Block 2: Combine 2-output of each radix-4, i.e. inputs from __in0 + [4,5,6,7]*istride: */\n\t"\
		"movq	%[__isrt2],%%rsi 	\n\t"\
		"vmovaps	(%%rsi),%%ymm3	/* isrt2 */\n\t"\
		"addq	$%c[__i1],%%rax	\n\t"/* __in0 + 1*istride */\
		"addq	$%c[__i1],%%rbx	\n\t"/* __in0 + 5*istride */\
		"addq	$%c[__i1],%%rcx	\n\t"/* __in0 + 9*istride */\
		"addq	$%c[__i1],%%rdx	\n\t"/* __in0 + d*istride */\
	"prefetcht1	0x100(%%r13)\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	    (%%rdx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7	\n\t"\
		"vmulpd	%%ymm3,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmulpd	%%ymm3,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmulpd	%%ymm3,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	    (%%rbx),%%ymm2	\n\t"\
		"vmulpd	%%ymm3,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3	\n\t"\
		"vsubpd	%%ymm3,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm5,%%ymm4,%%ymm4		\n\t"\
		"vsubpd	%%ymm2,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm6,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm3,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm2,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm0,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm4,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm1,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm7,%%ymm6,%%ymm6		\n\t"\
		"vsubpd	%%ymm6,%%ymm4,%%ymm4		\n\t"\
		"vsubpd	%%ymm7,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm4,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm5,%%ymm7,%%ymm7		\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm7,%%ymm3,%%ymm3		\n\t"\
		"vsubpd	%%ymm5,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm6,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"movq	%[__out4],%%r10		\n\t"\
		"movq	%[__out5],%%r11		\n\t"\
		"movq	%[__out6],%%r12		\n\t"\
		"movq	%[__out7],%%r13		\n\t"\
	"prefetcht1	0x100(%%r11)\n\t"\
		"vmovaps	%%ymm0,    (%%r11)	\n\t"\
		"vmovaps	%%ymm3,    (%%r12)	\n\t"\
		"vmovaps	%%ymm2,0x20(%%r11)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%r13)	\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4	\n\t"\
		"vaddpd	%%ymm3,%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm2,%%ymm5,%%ymm5	\n\t"\
		"vaddpd	%%ymm1,%%ymm6,%%ymm6	\n\t"\
		"vmovaps	%%ymm4,    (%%r10)	\n\t"\
		"vmovaps	%%ymm7,    (%%r13)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%r10)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%r12)	\n\t"\
	"/* Block 1: Combine 1-output of each radix-4, i.e. inputs from __in0 + [8,9,a,b]*istride: */\n\t"\
		"addq	$%c[__i1],%%rax	\n\t"/* __in0 + 2*istride */\
		"addq	$%c[__i1],%%rbx	\n\t"/* __in0 + 6*istride */\
		"addq	$%c[__i1],%%rcx	\n\t"/* __in0 + a*istride */\
		"addq	$%c[__i1],%%rdx	\n\t"/* __in0 + e*istride */\
	"prefetcht1	0x100(%%r13)\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	0x20(%%rsi),%%ymm3	/* cc0, using isrt2 as base-ptr */\n\t"\
		"vmovaps	0x40(%%rsi),%%ymm2	/* ss0, using isrt2 as base-ptr */\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmulpd	%%ymm3,%%ymm4,%%ymm4		\n\t"\
		"vmulpd	%%ymm3,%%ymm5,%%ymm5		\n\t"\
		"vmulpd	%%ymm2,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	    (%%rdx),%%ymm0	\n\t"\
		"vmulpd	%%ymm2,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm1	\n\t"\
		"vaddpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm0,%%ymm6		\n\t"\
		"vsubpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm1,%%ymm7		\n\t"\
		"vmulpd	%%ymm2,%%ymm6,%%ymm6		\n\t"\
		"vmulpd	%%ymm2,%%ymm7,%%ymm7		\n\t"\
		"vmulpd	%%ymm3,%%ymm0,%%ymm0		\n\t"\
		"vmulpd	%%ymm3,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm0,%%ymm7,%%ymm7		\n\t"\
		"vsubpd	%%ymm1,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	%%ymm4,%%ymm2		\n\t"\
		"vmovaps	%%ymm5,%%ymm3		\n\t"\
		"vsubpd	%%ymm6,%%ymm4,%%ymm4		\n\t"\
		"vsubpd	%%ymm7,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm2,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm3,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	    (%%rbx),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3	\n\t"\
		"vmovaps	    (%%rsi),%%ymm1	/* isrt2 */\n\t"\
		"vmovaps	%%ymm2,%%ymm0		\n\t"\
		"vsubpd	%%ymm3,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm0,%%ymm3,%%ymm3		\n\t"\
		"vmulpd	%%ymm1,%%ymm2,%%ymm2		\n\t"\
		"vmulpd	%%ymm1,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vsubpd	%%ymm2,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm3,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm2,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm3,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm0,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm1,%%ymm3,%%ymm3		\n\t"\
		"vsubpd	%%ymm6,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm5,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm7,%%ymm3,%%ymm3		\n\t"\
		"vsubpd	%%ymm4,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"movq	%[__out8],%%r10		\n\t"\
		"movq	%[__out9],%%r11		\n\t"\
		"movq	%[__outa],%%r12		\n\t"\
		"movq	%[__outb],%%r13		\n\t"\
	"prefetcht1	0x100(%%r11)\n\t"\
		"vmovaps	%%ymm2,    (%%r11)	\n\t"\
		"vmovaps	%%ymm0,    (%%r12)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%r11)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%r13)	\n\t"\
		"vaddpd	%%ymm2,%%ymm6,%%ymm6	\n\t"\
		"vaddpd	%%ymm0,%%ymm5,%%ymm5	\n\t"\
		"vaddpd	%%ymm3,%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm1,%%ymm4,%%ymm4	\n\t"\
		"vmovaps	%%ymm6,    (%%r10)	\n\t"\
		"vmovaps	%%ymm5,    (%%r13)	\n\t"\
		"vmovaps	%%ymm7,0x20(%%r10)	\n\t"\
		"vmovaps	%%ymm4,0x20(%%r12)	\n\t"\
	"/* Block 3: Combine 3-output of each radix-4, i.e. inputs from __in0 + [c,d,e,f]*istride: */\n\t"\
		"addq	$%c[__i1],%%rax	\n\t"/* __in0 + 3*istride */\
		"addq	$%c[__i1],%%rbx	\n\t"/* __in0 + 7*istride */\
		"addq	$%c[__i1],%%rcx	\n\t"/* __in0 + b*istride */\
		"addq	$%c[__i1],%%rdx	\n\t"/* __in0 + f*istride */\
	"prefetcht1	0x100(%%r13)\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	0x20(%%rsi),%%ymm2	/* cc0, using isrt2 as base-ptr */\n\t"\
		"vmovaps	0x40(%%rsi),%%ymm3	/* ss0, using isrt2 as base-ptr */\n\t"\
		"vmovaps	%%ymm4,%%ymm6		\n\t"\
		"vmovaps	%%ymm5,%%ymm7		\n\t"\
		"vmulpd	%%ymm3,%%ymm4,%%ymm4		\n\t"\
		"vmulpd	%%ymm3,%%ymm5,%%ymm5		\n\t"\
		"vmulpd	%%ymm2,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	    (%%rdx),%%ymm0	\n\t"\
		"vmulpd	%%ymm2,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm1	\n\t"\
		"vaddpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm0,%%ymm6		\n\t"\
		"vsubpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmovaps	%%ymm1,%%ymm7		\n\t"\
		"vmulpd	%%ymm2,%%ymm6,%%ymm6		\n\t"\
		"vmulpd	%%ymm2,%%ymm7,%%ymm7		\n\t"\
		"vmulpd	%%ymm3,%%ymm0,%%ymm0		\n\t"\
		"vmulpd	%%ymm3,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm0,%%ymm7,%%ymm7		\n\t"\
		"vsubpd	%%ymm1,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	%%ymm4,%%ymm2		\n\t"\
		"vmovaps	%%ymm5,%%ymm3		\n\t"\
		"vsubpd	%%ymm6,%%ymm4,%%ymm4		\n\t"\
		"vsubpd	%%ymm7,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm2,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm3,%%ymm7,%%ymm7		\n\t"\
		"vmovaps	    (%%rbx),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3	\n\t"\
		"vmovaps	    (%%rsi),%%ymm1		/* isrt2 */\n\t"\
		"vmovaps	%%ymm2,%%ymm0		\n\t"\
		"vaddpd	%%ymm3,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm0,%%ymm3,%%ymm3		\n\t"\
		"vmulpd	%%ymm1,%%ymm2,%%ymm2		\n\t"\
		"vmulpd	%%ymm1,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vsubpd	%%ymm2,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm3,%%ymm1,%%ymm1		\n\t"\
		"vaddpd	%%ymm2,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm3,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm0,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm1,%%ymm3,%%ymm3		\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"movq	%[__outc],%%r10		\n\t"\
		"movq	%[__outd],%%r11		\n\t"\
		"movq	%[__oute],%%r12		\n\t"\
		"movq	%[__outf],%%r13		\n\t"\
	"prefetcht1	0x100(%%r11)\n\t"\
		"vmovaps	%%ymm0,    (%%r11)	\n\t"\
		"vmovaps	%%ymm2,    (%%r12)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%r11)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%r13)	\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4	\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7	\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5	\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6	\n\t"\
		"vmovaps	%%ymm4,    (%%r10)	\n\t"\
		"vmovaps	%%ymm7,    (%%r13)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%r10)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%r12)	\n\t"\
	"prefetcht1	0x100(%%r13)\n\t"\
		:					/* outputs: none */\
		: [__in0] "m" (Xin0)	/* All inputs from memory addresses here */\
		 ,[__i1] "e" (Xi1)\
		 ,[__i2] "e" (Xi2)\
		 ,[__i3] "e" (Xi3)\
		 ,[__i4] "e" (Xi4)\
		 ,[__out0] "m" (Xout0)\
		 ,[__out1] "m" (Xout1)\
		 ,[__out2] "m" (Xout2)\
		 ,[__out3] "m" (Xout3)\
		 ,[__out4] "m" (Xout4)\
		 ,[__out5] "m" (Xout5)\
		 ,[__out6] "m" (Xout6)\
		 ,[__out7] "m" (Xout7)\
		 ,[__out8] "m" (Xout8)\
		 ,[__out9] "m" (Xout9)\
		 ,[__outa] "m" (Xouta)\
		 ,[__outb] "m" (Xoutb)\
		 ,[__outc] "m" (Xoutc)\
		 ,[__outd] "m" (Xoutd)\
		 ,[__oute] "m" (Xoute)\
		 ,[__outf] "m" (Xoutf)\
		 ,[__isrt2] "m" (Xisrt2)\
		 ,[__c1] "m" (Xc1)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

	#define SSE2_RADIX_03_DFT(Xi0,Xi1,Xi2, Xcc1, Xo0,Xo1,Xo2)\
	{\
	__asm__ volatile (\
		"movq	%[__i0],%%rax				\n\t"\
		"movq	%[__i1],%%rbx				\n\t"\
		"movq	%[__i2],%%rcx				\n\t"\
		"movq	%[__cc1],%%rdx				\n\t"\
		"vmovaps	    (%%rbx),%%ymm2		\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3		\n\t"\
		"vmovaps	    (%%rax),%%ymm0		\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1		\n\t"\
		"vmovaps	    (%%rcx),%%ymm6		\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm7		\n\t"\
		"vmovaps	%%ymm2,%%ymm4			\n\t"\
		"vmovaps	%%ymm3,%%ymm5			\n\t"\
		"movq	%[__o0],%%rax				\n\t"\
		"movq	%[__o1],%%rbx				\n\t"\
		"movq	%[__o2],%%rcx				\n\t"\
		"vaddpd	%%ymm6,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm7,%%ymm3,%%ymm3		\n\t"\
		"vsubpd	%%ymm6,%%ymm4,%%ymm4		\n\t"\
		"vsubpd	%%ymm7,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm2,%%ymm0,%%ymm0		\n\t"\
		"vaddpd	%%ymm3,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	    (%%rdx),%%ymm6		\n\t"\
		"vmovaps	0x20(%%rdx),%%ymm7		\n\t"\
		"vmovaps	%%ymm0,    (%%rax)		\n\t"\
		"vmovaps	%%ymm1,0x20(%%rax)		\n\t"\
		"vmulpd	%%ymm6,%%ymm2,%%ymm2		\n\t"\
		"vmulpd	%%ymm6,%%ymm3,%%ymm3		\n\t"\
		"vmulpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vmulpd	%%ymm7,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm0,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm1,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm2,%%ymm0			\n\t"\
		"vmovaps	%%ymm3,%%ymm1			\n\t"\
		"vsubpd	%%ymm5,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm4,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm5,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm4,%%ymm1,%%ymm1		\n\t"\
		"vmovaps	%%ymm2,    (%%rbx)		\n\t"\
		"vmovaps	%%ymm3,0x20(%%rbx)		\n\t"\
		"vmovaps	%%ymm0,    (%%rcx)		\n\t"\
		"vmovaps	%%ymm1,0x20(%%rcx)		\n\t"\
		:					/* outputs: none */\
		: [__i0] "m" (Xi0)	/* All inputs from memory addresses here */\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__cc1] "m" (Xcc1)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		: "cc","memory","rax","rbx","rcx","rdx","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

	#define SSE2_RADIX4_DIF_0TWIDDLE_STRIDE(Xadd0, Xadd1, Xadd2, Xadd3, Xtmp, Xstride)\
	{\
	__asm__ volatile (\
		"movq	%[__tmp]   ,%%rax	\n\t"\
		"movq	%[__stride],%%rsi	\n\t"\
		"movq	%%rax,%%rbx			\n\t"\
		"addq	%%rsi,%%rbx			/* add_in1  */\n\t"\
		"shlq	$1,%%rsi			/* stride*2 */\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	    (%%rbx),%%ymm2	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm3	\n\t"\
		"vmovaps	    (%%rax),%%ymm4	\n\t"\
		"vmovaps	    (%%rbx),%%ymm6	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm5	\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm7	\n\t"\
		"addq	%%rsi,%%rax			/* add_in2  */\n\t"\
		"addq	%%rsi,%%rbx			/* add_in3  */\n\t"\
		"vaddpd	    (%%rax),%%ymm0,%%ymm0	\n\t"\
		"vaddpd	    (%%rbx),%%ymm2,%%ymm2	\n\t"\
		"vaddpd	0x20(%%rax),%%ymm1,%%ymm1	\n\t"\
		"vaddpd	0x20(%%rbx),%%ymm3,%%ymm3	\n\t"\
		"vsubpd	    (%%rax),%%ymm4,%%ymm4	\n\t"\
		"vsubpd	    (%%rbx),%%ymm6,%%ymm6	\n\t"\
		"vsubpd	0x20(%%rax),%%ymm5,%%ymm5	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm7,%%ymm7	\n\t"\
		"/* Finish radix-4 butterfly and store results into main-array slots: */\n\t"\
		"movq	%[__add0],%%rax		\n\t"\
		"movq	%[__add1],%%rbx		\n\t"\
		"movq	%[__add2],%%rcx		\n\t"\
		"movq	%[__add3],%%rdx		\n\t"\
		"vsubpd	%%ymm2,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm7,%%ymm4,%%ymm4		\n\t"\
		"vsubpd	%%ymm3,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm6,%%ymm5,%%ymm5		\n\t"\
		"vmovaps	%%ymm0,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm4,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rdx)	\n\t"\
		"vaddpd	%%ymm2,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm3,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm0,%%ymm2,%%ymm2		\n\t"\
		"vaddpd	%%ymm4,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm1,%%ymm3,%%ymm3		\n\t"\
		"vaddpd	%%ymm5,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	%%ymm2,    (%%rax)	\n\t"\
		"vmovaps	%%ymm7,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rax)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rcx)	\n\t"\
		:					/* outputs: none */\
		: [__add0] "m" (Xadd0)	/* All inputs from memory addresses here */\
		 ,[__add1] "m" (Xadd1)\
		 ,[__add2] "m" (Xadd2)\
		 ,[__add3] "m" (Xadd3)\
		 ,[__tmp] "m" (Xtmp)\
		 ,[__stride] "e" (Xstride)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

	/* DIF radix-4 subconvolution, sans twiddles, inputs in __i0-3, outputs in __o0-3, possibly coincident with inputs: */
	#define SSE2_RADIX4_DIF_0TWIDDLE_STRIDE_E(Xi0,Xi1,Xi2,Xi3, Xo0,Xo1,Xo2,Xo3)\
	{\
	__asm__ volatile (\
		"movq	%[__i0],%%rax		\n\t"\
		"movq	%[__i1],%%rbx		\n\t"\
		"movq	%[__i2],%%rcx		\n\t"\
		"movq	%[__i3],%%rdx		\n\t"\
		"vmovaps	    (%%rax),%%ymm0		\n\t"\
		"vmovaps	    (%%rbx),%%ymm4		\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1		\n\t"\
		"vmovaps	0x20(%%rbx),%%ymm5		\n\t"\
		"vmovaps	%%ymm0,%%ymm2			\n\t"\
		"vmovaps	%%ymm4,%%ymm6			\n\t"\
		"vmovaps	%%ymm1,%%ymm3			\n\t"\
		"vmovaps	%%ymm5,%%ymm7			\n\t"\
		"vaddpd	    (%%rcx),%%ymm0,%%ymm0	\n\t"\
		"vaddpd	    (%%rdx),%%ymm4,%%ymm4	\n\t"\
		"vaddpd	0x20(%%rcx),%%ymm1,%%ymm1	\n\t"\
		"vaddpd	0x20(%%rdx),%%ymm5,%%ymm5	\n\t"\
		"vsubpd	    (%%rcx),%%ymm2,%%ymm2	\n\t"\
		"vsubpd	    (%%rdx),%%ymm6,%%ymm6	\n\t"\
		"vsubpd	0x20(%%rcx),%%ymm3,%%ymm3	\n\t"\
		"vsubpd	0x20(%%rdx),%%ymm7,%%ymm7	\n\t"\
		"/* Finish radix-4 butterfly and store results into main-array slots: */\n\t"\
		"movq	%[__o0],%%rax					\n\t"\
		"movq	%[__o1],%%rbx					\n\t"\
		"movq	%[__o2],%%rcx					\n\t"\
		"movq	%[__o3],%%rdx					\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm0,    (%%rbx)		\n\t"\
		"vmovaps	%%ymm2,    (%%rcx)		\n\t"\
		"vmovaps	%%ymm1,0x20(%%rbx)		\n\t"\
		"vmovaps	%%ymm3,0x20(%%rdx)		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	%%ymm4,    (%%rax)		\n\t"\
		"vmovaps	%%ymm7,    (%%rdx)		\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)		\n\t"\
		"vmovaps	%%ymm6,0x20(%%rcx)		\n\t"\
		:					/* outputs: none */\
		: [__i0] "m" (Xi0)	/* All inputs from memory addresses here */\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__i3] "m" (Xi3)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__o3] "m" (Xo3)\
		: "cc","memory","rax","rbx","rcx","rdx","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

	#define SSE2_RADIX4_DIT_0TWIDDLE_STRIDE(Xadd0, Xadd1, Xadd2, Xadd3, Xtmp, Xstride)\
	{\
	__asm__ volatile (\
		"movq	%[__add0],%%rax		\n\t"\
		"movq	%[__add1],%%rbx		\n\t"\
		"movq	%[__add2],%%rcx		\n\t"\
		"movq	%[__add3],%%rdx		\n\t"\
		"vmovaps	    (%%rax),%%ymm0	\n\t"\
		"vmovaps	    (%%rcx),%%ymm4	\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1	\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5	\n\t"\
		"vmovaps	%%ymm0,%%ymm2			\n\t"\
		"vmovaps	%%ymm4,%%ymm6			\n\t"\
		"vmovaps	%%ymm1,%%ymm3			\n\t"\
		"vmovaps	%%ymm5,%%ymm7			\n\t"\
		"movq	%[__tmp]   ,%%rax	\n\t"\
		"movq	%[__stride],%%rcx	\n\t"\
		"vaddpd	    (%%rbx),%%ymm0,%%ymm0	\n\t"\
		"vaddpd	    (%%rdx),%%ymm4,%%ymm4	\n\t"\
		"vaddpd	0x20(%%rbx),%%ymm1,%%ymm1	\n\t"\
		"vaddpd	0x20(%%rdx),%%ymm5,%%ymm5	\n\t"\
		"vsubpd	    (%%rbx),%%ymm2,%%ymm2	\n\t"\
		"vsubpd	    (%%rdx),%%ymm6,%%ymm6	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm3,%%ymm3	\n\t"\
		"vsubpd	0x20(%%rdx),%%ymm7,%%ymm7	\n\t"\
		"movq	%%rax,%%rbx			\n\t"\
		"addq	%%rcx,%%rbx			\n\t"\
		"movq	%%rbx,%%rdx			\n\t"\
		"addq	%%rcx,%%rcx			\n\t"\
		"addq	%%rcx,%%rdx			\n\t"\
		"addq	%%rax,%%rcx			\n\t"\
		"/* Finish radix-4 butterfly and store results into temp-array slots: */\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0			\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2			\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1			\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3			\n\t"\
		"vmovaps	%%ymm0,    (%%rcx)	\n\t"\
		"vmovaps	%%ymm2,    (%%rdx)	\n\t"\
		"vmovaps	%%ymm1,0x20(%%rcx)	\n\t"\
		"vmovaps	%%ymm3,0x20(%%rbx)	\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4			\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7			\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5			\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6			\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4			\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7			\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5			\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6			\n\t"\
		"vmovaps	%%ymm4,    (%%rax)	\n\t"\
		"vmovaps	%%ymm7,    (%%rbx)	\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)	\n\t"\
		"vmovaps	%%ymm6,0x20(%%rdx)	\n\t"\
		:					/* outputs: none */\
		: [__add0] "m" (Xadd0)	/* All inputs from memory addresses here */\
		 ,[__add1] "m" (Xadd1)\
		 ,[__add2] "m" (Xadd2)\
		 ,[__add3] "m" (Xadd3)\
		 ,[__tmp] "m" (Xtmp)\
		 ,[__stride] "e" (Xstride)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

	/* DIT radix-4 subconvolution, sans twiddles, inputs in __i0-3, outputs in __o0-3, possibly coincident with inputs: */
	#define SSE2_RADIX4_DIT_0TWIDDLE_STRIDE_E(Xi0,Xi1,Xi2,Xi3, Xo0,Xo1,Xo2,Xo3)\
	{\
	__asm__ volatile (\
		"movq	%[__i0],%%rax		\n\t"\
		"movq	%[__i1],%%rbx		\n\t"\
		"movq	%[__i2],%%rcx		\n\t"\
		"movq	%[__i3],%%rdx		\n\t"\
		"vmovaps	    (%%rax),%%ymm0		\n\t"\
		"vmovaps	    (%%rcx),%%ymm4		\n\t"\
		"vmovaps	0x20(%%rax),%%ymm1		\n\t"\
		"vmovaps	0x20(%%rcx),%%ymm5		\n\t"\
		"vmovaps	%%ymm0,%%ymm2			\n\t"\
		"vmovaps	%%ymm4,%%ymm6			\n\t"\
		"vmovaps	%%ymm1,%%ymm3			\n\t"\
		"vmovaps	%%ymm5,%%ymm7			\n\t"\
		"vaddpd	    (%%rbx),%%ymm0,%%ymm0	\n\t"\
		"vaddpd	    (%%rdx),%%ymm4,%%ymm4	\n\t"\
		"vaddpd	0x20(%%rbx),%%ymm1,%%ymm1	\n\t"\
		"vaddpd	0x20(%%rdx),%%ymm5,%%ymm5	\n\t"\
		"vsubpd	    (%%rbx),%%ymm2,%%ymm2	\n\t"\
		"vsubpd	    (%%rdx),%%ymm6,%%ymm6	\n\t"\
		"vsubpd	0x20(%%rbx),%%ymm3,%%ymm3	\n\t"\
		"vsubpd	0x20(%%rdx),%%ymm7,%%ymm7	\n\t"\
		"/* Finish radix-4 butterfly and store results into output-array slots: */\n\t"\
		"movq	%[__o0],%%rax					\n\t"\
		"movq	%[__o1],%%rbx					\n\t"\
		"movq	%[__o2],%%rcx					\n\t"\
		"movq	%[__o3],%%rdx					\n\t"\
		"vsubpd	%%ymm4,%%ymm0,%%ymm0		\n\t"\
		"vsubpd	%%ymm7,%%ymm2,%%ymm2		\n\t"\
		"vsubpd	%%ymm5,%%ymm1,%%ymm1		\n\t"\
		"vsubpd	%%ymm6,%%ymm3,%%ymm3		\n\t"\
		"vmovaps	%%ymm0,    (%%rcx)		\n\t"\
		"vmovaps	%%ymm2,    (%%rdx)		\n\t"\
		"vmovaps	%%ymm1,0x20(%%rcx)		\n\t"\
		"vmovaps	%%ymm3,0x20(%%rbx)		\n\t"\
		"vaddpd	%%ymm4,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm7,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm5,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm6,%%ymm6,%%ymm6		\n\t"\
		"vaddpd	%%ymm0,%%ymm4,%%ymm4		\n\t"\
		"vaddpd	%%ymm2,%%ymm7,%%ymm7		\n\t"\
		"vaddpd	%%ymm1,%%ymm5,%%ymm5		\n\t"\
		"vaddpd	%%ymm3,%%ymm6,%%ymm6		\n\t"\
		"vmovaps	%%ymm4,    (%%rax)		\n\t"\
		"vmovaps	%%ymm7,    (%%rbx)		\n\t"\
		"vmovaps	%%ymm5,0x20(%%rax)		\n\t"\
		"vmovaps	%%ymm6,0x20(%%rdx)		\n\t"\
		:					/* outputs: none */\
		: [__i0] "m" (Xi0)	/* All inputs from memory addresses here */\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__i3] "m" (Xi3)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__o3] "m" (Xo3)\
		: "cc","memory","rax","rbx","rcx","rdx","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

#elif defined(USE_SSE2)

/*** Prefetch even-index iaddresses in DIT below, odd-index oaddresses in SSE2_RADIX16_DIF_TWIDDLE_OOP ***/

	// Based on the SSE2_RADIX16_DIT_NOTWIDDLE macro in radix16_ditN_cy_dif1_gcc64.h, but with completely
	// specifiable 16-input addressing required for usage as the power-of-2 component of a twiddleless
	// radix = [odd*2^n] DFT routine.
	// We use just a single output base-pointer plus literal ostrides which are [1,2,3,4]-multiples of
	// __01; this allows us to cut GP-register usage, which is absolutely a must for the 32-bit version
	// of the macro, and is a benefit to the 64-bit versions which code-fold to yield 2 side-by-side
	// streams of independently executable instructions, one for data in xmm0-7, the other using xmm8-15.
	#define SSE2_RADIX16_DIT_0TWIDDLE(Xin0,Xin1,Xin2,Xin3,Xin4,Xin5,Xin6,Xin7,Xin8,Xin9,Xina,Xinb,Xinc,Xind,Xine,Xinf, Xisrt2,Xtwo, Xout0,Xo1,Xo2,Xo3,Xo4)\
	{\
	__asm__ volatile (\
		"movq	%[__in0],%%rax		\n\t"\
		"movq	%[__in1],%%rbx		\n\t"\
		"movq	%[__in2],%%rcx		\n\t"\
		"movq	%[__in3],%%rdx		\n\t"\
	"prefetcht1	0x100(%%rax)\n\t"\
		"/* SSE2_RADIX4_DIT_0TWIDDLE_B(r0 ): */\n\t"\
		"movaps	    (%%rax),%%xmm2	\n\t"\
		"movaps	    (%%rcx),%%xmm6	\n\t"\
		"movaps	0x10(%%rax),%%xmm3	\n\t"\
		"movaps	0x10(%%rcx),%%xmm7	\n\t"\
		"movaps	    (%%rbx),%%xmm0	\n\t"\
		"movaps	    (%%rdx),%%xmm4	\n\t"\
		"movaps	0x10(%%rbx),%%xmm1	\n\t"\
		"movaps	0x10(%%rdx),%%xmm5	\n\t"\
		"subpd	%%xmm0,%%xmm2		\n\t"\
		"subpd	%%xmm4,%%xmm6		\n\t"\
		"subpd	%%xmm1,%%xmm3		\n\t"\
		"subpd	%%xmm5,%%xmm7		\n\t"\
		"addpd	%%xmm0,%%xmm0		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm1,%%xmm1		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm2,%%xmm0		\n\t"\
		"addpd	%%xmm6,%%xmm4		\n\t"\
		"addpd	%%xmm3,%%xmm1		\n\t"\
		"addpd	%%xmm7,%%xmm5		\n\t"\
		"movq	%[__out0],%%rsi		\n\t"\
		"subpd	%%xmm4,%%xmm0		\n\t"\
		"subpd	%%xmm7,%%xmm2		\n\t"\
		"subpd	%%xmm5,%%xmm1		\n\t"\
		"subpd	%%xmm6,%%xmm3		\n\t"\
	"prefetcht1	0x100(%%rcx)\n\t"\
		"leaq	0x10(%%rsi),%%rdi	\n\t"/* Need separate address Im parts of outputs due to literal-offsets below */\
		"movaps	%%xmm0,%c[__o2](%%rsi)	\n\t"\
		"movaps	%%xmm2,%c[__o3](%%rsi)	\n\t"\
		"movaps	%%xmm1,%c[__o2](%%rdi)	\n\t"\
		"movaps	%%xmm3,%c[__o1](%%rdi)	\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm0,%%xmm4		\n\t"\
		"addpd	%%xmm2,%%xmm7		\n\t"\
		"addpd	%%xmm1,%%xmm5		\n\t"\
		"addpd	%%xmm3,%%xmm6		\n\t"\
		"movaps	%%xmm4,        (%%rsi)	\n\t"\
		"movaps	%%xmm7,%c[__o1](%%rsi)	\n\t"\
		"movaps	%%xmm5,        (%%rdi)	\n\t"\
		"movaps	%%xmm6,%c[__o3](%%rdi)	\n\t"\
		"/* SSE2_RADIX4_DIT_0TWIDDLE_B(r8 ): */\n\t"\
		"movq	%[__in4],%%rax		\n\t	leaq	%c[__o4](%%rsi),%%rsi	\n\t"/* __out0 + 4*ostride */\
		"movq	%[__in5],%%rbx		\n\t"\
		"movq	%[__in6],%%rcx		\n\t"\
		"movq	%[__in7],%%rdx		\n\t"\
	"prefetcht1	0x100(%%rax)\n\t"\
		"movaps	    (%%rax),%%xmm2	\n\t"\
		"movaps	    (%%rcx),%%xmm6	\n\t"\
		"movaps	0x10(%%rax),%%xmm3	\n\t"\
		"movaps	0x10(%%rcx),%%xmm7	\n\t"\
		"movaps	    (%%rbx),%%xmm0	\n\t"\
		"movaps	    (%%rdx),%%xmm4	\n\t"\
		"movaps	0x10(%%rbx),%%xmm1	\n\t"\
		"movaps	0x10(%%rdx),%%xmm5	\n\t"\
		"subpd	%%xmm0,%%xmm2		\n\t"\
		"subpd	%%xmm4,%%xmm6		\n\t"\
		"subpd	%%xmm1,%%xmm3		\n\t"\
		"subpd	%%xmm5,%%xmm7		\n\t"\
		"addpd	%%xmm0,%%xmm0		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm1,%%xmm1		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm2,%%xmm0		\n\t"\
		"addpd	%%xmm6,%%xmm4		\n\t"\
		"addpd	%%xmm3,%%xmm1		\n\t"\
		"addpd	%%xmm7,%%xmm5		\n\t"\
		"subpd	%%xmm4,%%xmm0		\n\t"\
		"subpd	%%xmm7,%%xmm2		\n\t"\
		"subpd	%%xmm5,%%xmm1		\n\t"\
		"subpd	%%xmm6,%%xmm3		\n\t"\
	"prefetcht1	0x100(%%rcx)\n\t"\
		"leaq	0x10(%%rsi),%%rdi	\n\t"\
		"movaps	%%xmm0,%c[__o2](%%rsi)	\n\t"\
		"movaps	%%xmm2,%c[__o3](%%rsi)	\n\t"\
		"movaps	%%xmm1,%c[__o2](%%rdi)	\n\t"\
		"movaps	%%xmm3,%c[__o1](%%rdi)	\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm0,%%xmm4		\n\t"\
		"addpd	%%xmm2,%%xmm7		\n\t"\
		"addpd	%%xmm1,%%xmm5		\n\t"\
		"addpd	%%xmm3,%%xmm6		\n\t"\
		"movaps	%%xmm4,        (%%rsi)	\n\t"\
		"movaps	%%xmm7,%c[__o1](%%rsi)	\n\t"\
		"movaps	%%xmm5,        (%%rdi)	\n\t"\
		"movaps	%%xmm6,%c[__o3](%%rdi)	\n\t"\
		"/* SSE2_RADIX4_DIT_0TWIDDLE_B(r16): */\n\t"\
		"movq	%[__in8],%%rax			\n\t	leaq	%c[__o4](%%rsi),%%rsi	\n\t"/* __out0 + 8*ostride */\
		"movq	%[__in9],%%rbx			\n\t"\
		"movq	%[__ina],%%rcx			\n\t"\
		"movq	%[__inb],%%rdx			\n\t"\
	"prefetcht1	0x100(%%rax)\n\t"\
		"movaps	    (%%rax),%%xmm2	\n\t"\
		"movaps	    (%%rcx),%%xmm6	\n\t"\
		"movaps	0x10(%%rax),%%xmm3	\n\t"\
		"movaps	0x10(%%rcx),%%xmm7	\n\t"\
		"movaps	    (%%rbx),%%xmm0	\n\t"\
		"movaps	    (%%rdx),%%xmm4	\n\t"\
		"movaps	0x10(%%rbx),%%xmm1	\n\t"\
		"movaps	0x10(%%rdx),%%xmm5	\n\t"\
		"subpd	%%xmm0,%%xmm2		\n\t"\
		"subpd	%%xmm4,%%xmm6		\n\t"\
		"subpd	%%xmm1,%%xmm3		\n\t"\
		"subpd	%%xmm5,%%xmm7		\n\t"\
		"addpd	%%xmm0,%%xmm0		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm1,%%xmm1		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm2,%%xmm0		\n\t"\
		"addpd	%%xmm6,%%xmm4		\n\t"\
		"addpd	%%xmm3,%%xmm1		\n\t"\
		"addpd	%%xmm7,%%xmm5		\n\t"\
		"subpd	%%xmm4,%%xmm0		\n\t"\
		"subpd	%%xmm7,%%xmm2		\n\t"\
		"subpd	%%xmm5,%%xmm1		\n\t"\
		"subpd	%%xmm6,%%xmm3		\n\t"\
	"prefetcht1	0x100(%%rcx)\n\t"\
		"leaq	0x10(%%rsi),%%rdi	\n\t"\
		"movaps	%%xmm0,%c[__o2](%%rsi)	\n\t"\
		"movaps	%%xmm2,%c[__o3](%%rsi)	\n\t"\
		"movaps	%%xmm1,%c[__o2](%%rdi)	\n\t"\
		"movaps	%%xmm3,%c[__o1](%%rdi)	\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm0,%%xmm4		\n\t"\
		"addpd	%%xmm2,%%xmm7		\n\t"\
		"addpd	%%xmm1,%%xmm5		\n\t"\
		"addpd	%%xmm3,%%xmm6		\n\t"\
		"movaps	%%xmm4,        (%%rsi)	\n\t"\
		"movaps	%%xmm7,%c[__o1](%%rsi)	\n\t"\
		"movaps	%%xmm5,        (%%rdi)	\n\t"\
		"movaps	%%xmm6,%c[__o3](%%rdi)	\n\t"\
		"/* SSE2_RADIX4_DIT_0TWIDDLE_B(r24): */\n\t"\
		"movq	%[__inc],%%rax			\n\t	leaq	%c[__o4](%%rsi),%%rsi	\n\t"/* __out0 + c*ostride */\
		"movq	%[__ind],%%rbx			\n\t"\
		"movq	%[__ine],%%rcx			\n\t"\
		"movq	%[__inf],%%rdx			\n\t"\
	"prefetcht1	0x100(%%rax)\n\t"\
		"movaps	    (%%rax),%%xmm2	\n\t"\
		"movaps	    (%%rcx),%%xmm6	\n\t"\
		"movaps	0x10(%%rax),%%xmm3	\n\t"\
		"movaps	0x10(%%rcx),%%xmm7	\n\t"\
		"movaps	    (%%rbx),%%xmm0	\n\t"\
		"movaps	    (%%rdx),%%xmm4	\n\t"\
		"movaps	0x10(%%rbx),%%xmm1	\n\t"\
		"movaps	0x10(%%rdx),%%xmm5	\n\t"\
		"subpd	%%xmm0,%%xmm2		\n\t"\
		"subpd	%%xmm4,%%xmm6		\n\t"\
		"subpd	%%xmm1,%%xmm3		\n\t"\
		"subpd	%%xmm5,%%xmm7		\n\t"\
		"addpd	%%xmm0,%%xmm0		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm1,%%xmm1		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm2,%%xmm0		\n\t"\
		"addpd	%%xmm6,%%xmm4		\n\t"\
		"addpd	%%xmm3,%%xmm1		\n\t"\
		"addpd	%%xmm7,%%xmm5		\n\t"\
		"subpd	%%xmm4,%%xmm0		\n\t"\
		"subpd	%%xmm7,%%xmm2		\n\t"\
		"subpd	%%xmm5,%%xmm1		\n\t"\
		"subpd	%%xmm6,%%xmm3		\n\t"\
	"prefetcht1	0x100(%%rcx)\n\t"\
		"leaq	0x10(%%rsi),%%rdi	\n\t"\
		"movaps	%%xmm0,%c[__o2](%%rsi)	\n\t"\
		"movaps	%%xmm2,%c[__o3](%%rsi)	\n\t"\
		"movaps	%%xmm1,%c[__o2](%%rdi)	\n\t"\
		"movaps	%%xmm3,%c[__o1](%%rdi)	\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm0,%%xmm4		\n\t"\
		"addpd	%%xmm2,%%xmm7		\n\t"\
		"addpd	%%xmm1,%%xmm5		\n\t"\
		"addpd	%%xmm3,%%xmm6		\n\t"\
		"movaps	%%xmm4,        (%%rsi)	\n\t"\
		"movaps	%%xmm7,%c[__o1](%%rsi)	\n\t"\
		"movaps	%%xmm5,        (%%rdi)	\n\t"\
		"movaps	%%xmm6,%c[__o3](%%rdi)	\n\t"\
	/*** Now do 4 DFTs with internal twiddles on the 4*stride - separated data: ***/\
		"movq	%[__out0],%%rax		\n\t"\
		"leaq	%c[__o4](%%rax),%%rbx	\n\t"/* __out0 +   [4*ostride] */\
		"leaq	%c[__o4](%%rbx),%%rcx	\n\t"/* __out0 + 2*[4*ostride] */\
		"leaq	%c[__o4](%%rcx),%%rdx	\n\t"/* __out0 + 3*[4*ostride] */\
		/* Block 0: */\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	    (%%rbx),%%xmm2	\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t"\
		"subpd	    (%%rbx),%%xmm0	\n\t"\
		"subpd	0x10(%%rbx),%%xmm1	\n\t"\
		"addpd	    (%%rax),%%xmm2	\n\t"\
		"addpd	0x10(%%rax),%%xmm3	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rdx),%%xmm6	\n\t"\
		"movaps	0x10(%%rdx),%%xmm7	\n\t"\
		"subpd	    (%%rdx),%%xmm4	\n\t"\
		"subpd	0x10(%%rdx),%%xmm5	\n\t"\
		"addpd	    (%%rcx),%%xmm6	\n\t"\
		"addpd	0x10(%%rcx),%%xmm7	\n\t"\
		"							\n\t"\
		"subpd	%%xmm6,%%xmm2		\n\t"\
		"subpd	%%xmm7,%%xmm3		\n\t"\
		"movaps	%%xmm2,    (%%rcx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rcx)	\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm2,%%xmm6		\n\t"\
		"addpd	%%xmm3,%%xmm7		\n\t"\
		"movaps	%%xmm6,    (%%rax)	\n\t"\
		"movaps	%%xmm7,0x10(%%rax)	\n\t"\
		"							\n\t"\
		"subpd	%%xmm5,%%xmm0		\n\t"\
		"subpd	%%xmm4,%%xmm1		\n\t"\
		"movaps	%%xmm0,    (%%rdx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rbx)	\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm0,%%xmm5		\n\t"\
		"addpd	%%xmm1,%%xmm4		\n\t"\
		"movaps	%%xmm5,    (%%rbx)	\n\t"\
		"movaps	%%xmm4,0x10(%%rdx)	\n\t"\
		"							\n\t"\
		/* Block 2: */\
		"leaq	%c[__o2](%%rax),%%rax	\n\t"/* All addresses += 2*ostride */\
		"leaq	%c[__o2](%%rbx),%%rbx	\n\t"\
		"leaq	%c[__o2](%%rcx),%%rcx	\n\t"\
		"leaq	%c[__o2](%%rdx),%%rdx	\n\t"\
		"movq	%[__isrt2],%%rdi	\n\t"\
		"movaps	(%%rdi),%%xmm2		\n\t"\
		"							\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rdx),%%xmm0	\n\t"\
		"movaps	0x10(%%rdx),%%xmm1	\n\t"\
		"						\n\t"\
		"addpd	0x10(%%rcx),%%xmm4	\n\t"\
		"subpd	    (%%rcx),%%xmm5	\n\t"\
		"subpd	0x10(%%rdx),%%xmm0	\n\t"\
		"addpd	    (%%rdx),%%xmm1	\n\t"\
		"mulpd	%%xmm2,%%xmm4		\n\t"\
		"mulpd	%%xmm2,%%xmm5		\n\t"\
		"mulpd	%%xmm2,%%xmm0		\n\t"\
		"mulpd	%%xmm2,%%xmm1		\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"							\n\t"\
		"subpd	%%xmm0,%%xmm4		\n\t"\
		"subpd	%%xmm1,%%xmm5		\n\t"\
		"addpd	%%xmm0,%%xmm6		\n\t"\
		"addpd	%%xmm1,%%xmm7		\n\t"\
		"							\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	    (%%rbx),%%xmm2	\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t"\
		"							\n\t"\
		"subpd	0x10(%%rbx),%%xmm0	\n\t"\
		"subpd	    (%%rbx),%%xmm1	\n\t"\
		"addpd	    (%%rax),%%xmm3	\n\t"\
		"addpd	0x10(%%rax),%%xmm2	\n\t"\
		"							\n\t"\
		"subpd	%%xmm4,%%xmm3		\n\t"\
		"subpd	%%xmm5,%%xmm1		\n\t"\
		"movaps	%%xmm3,    (%%rcx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rcx)	\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm3,%%xmm4		\n\t"\
		"addpd	%%xmm1,%%xmm5		\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t"\
		"							\n\t"\
		"subpd	%%xmm7,%%xmm0		\n\t"\
		"subpd	%%xmm6,%%xmm2		\n\t"\
		"movaps	%%xmm0,    (%%rdx)	\n\t"\
		"movaps	%%xmm2,0x10(%%rbx)	\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm0,%%xmm7		\n\t"\
		"addpd	%%xmm2,%%xmm6		\n\t"\
		"movaps	%%xmm7,    (%%rbx)	\n\t"\
		"movaps	%%xmm6,0x10(%%rdx)	\n\t"\
		"							\n\t"\
		/* Block 1: */\
		"leaq	-%c[__o1](%%rax),%%rax	\n\t"/* All addresses -= 1*ostride */\
		"leaq	-%c[__o1](%%rbx),%%rbx	\n\t"\
		"leaq	-%c[__o1](%%rcx),%%rcx	\n\t"\
		"leaq	-%c[__o1](%%rdx),%%rdx	\n\t"\
		"leaq	0x10(%%rdi),%%rsi	\n\t"/* cc0 */\
		"							\n\t"\
		"movaps	    (%%rdx),%%xmm0	\n\t"\
		"movaps	0x10(%%rdx),%%xmm1	\n\t"\
		"movaps	    (%%rdx),%%xmm2	\n\t"\
		"movaps	0x10(%%rdx),%%xmm3	\n\t"\
		"							\n\t"\
		"mulpd	0x10(%%rsi),%%xmm0	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm1	\n\t"\
		"mulpd	    (%%rsi),%%xmm2	\n\t"\
		"mulpd	    (%%rsi),%%xmm3	\n\t"\
		"subpd	%%xmm2,%%xmm1		\n\t"\
		"addpd	%%xmm3,%%xmm0		\n\t"\
		"							\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rcx),%%xmm6	\n\t"\
		"movaps	0x10(%%rcx),%%xmm7	\n\t"\
		"							\n\t"\
		"mulpd	    (%%rsi),%%xmm4	\n\t"\
		"mulpd	    (%%rsi),%%xmm5	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm6	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm7	\n\t"\
		"subpd	%%xmm6,%%xmm5		\n\t"\
		"addpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"							\n\t"\
		"addpd	%%xmm0,%%xmm4		\n\t"\
		"addpd	%%xmm1,%%xmm5		\n\t"\
		"subpd	%%xmm0,%%xmm6		\n\t"\
		"subpd	%%xmm1,%%xmm7		\n\t"\
		"							\n\t"\
		"movaps	    (%%rbx),%%xmm2	\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"addpd	0x10(%%rbx),%%xmm2	\n\t"\
		"subpd	    (%%rbx),%%xmm3	\n\t"\
		"mulpd	    (%%rdi),%%xmm2	\n\t"\
		"mulpd	    (%%rdi),%%xmm3	\n\t"\
		"							\n\t"\
		"subpd	%%xmm2,%%xmm0		\n\t"\
		"subpd	%%xmm3,%%xmm1		\n\t"\
		"addpd	%%xmm2,%%xmm2		\n\t"\
		"addpd	%%xmm3,%%xmm3		\n\t"\
		"addpd	%%xmm0,%%xmm2		\n\t"\
		"addpd	%%xmm1,%%xmm3		\n\t"\
		"							\n\t"\
		"subpd	%%xmm4,%%xmm2		\n\t"\
		"subpd	%%xmm5,%%xmm3		\n\t"\
		"movaps	%%xmm2,    (%%rcx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rcx)	\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm2,%%xmm4		\n\t"\
		"addpd	%%xmm3,%%xmm5		\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t"\
		"							\n\t"\
		"subpd	%%xmm7,%%xmm0		\n\t"\
		"subpd	%%xmm6,%%xmm1		\n\t"\
		"movaps	%%xmm0,    (%%rdx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rbx)	\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm0,%%xmm7		\n\t"\
		"addpd	%%xmm1,%%xmm6		\n\t"\
		"movaps	%%xmm7,    (%%rbx)	\n\t"\
		"movaps	%%xmm6,0x10(%%rdx)	\n\t"\
		"							\n\t"\
		/* Block 3: */\
		"leaq	%c[__o2](%%rax),%%rax	\n\t"/* All addresses += 2*ostride */\
		"leaq	%c[__o2](%%rbx),%%rbx	\n\t"\
		"leaq	%c[__o2](%%rcx),%%rcx	\n\t"\
		"leaq	%c[__o2](%%rdx),%%rdx	\n\t"\
		"							\n\t"\
		"movaps	    (%%rdx),%%xmm0	\n\t"\
		"movaps	0x10(%%rdx),%%xmm1	\n\t"\
		"movaps	    (%%rdx),%%xmm2	\n\t"\
		"movaps	0x10(%%rdx),%%xmm3	\n\t"\
		"						\n\t"\
		"mulpd	    (%%rsi),%%xmm0	\n\t"\
		"mulpd	    (%%rsi),%%xmm1	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm2	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm3	\n\t"\
		"subpd	%%xmm2,%%xmm1		\n\t"\
		"addpd	%%xmm3,%%xmm0		\n\t"\
		"							\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rcx),%%xmm6	\n\t"\
		"movaps	0x10(%%rcx),%%xmm7	\n\t"\
		"						\n\t"\
		"mulpd	0x10(%%rsi),%%xmm4	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm5	\n\t"\
		"mulpd	    (%%rsi),%%xmm6	\n\t"\
		"mulpd	    (%%rsi),%%xmm7	\n\t"\
		"subpd	%%xmm6,%%xmm5		\n\t"\
		"addpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"							\n\t"\
		"addpd	%%xmm0,%%xmm4		\n\t"\
		"addpd	%%xmm1,%%xmm5		\n\t"\
		"subpd	%%xmm0,%%xmm6		\n\t"\
		"subpd	%%xmm1,%%xmm7		\n\t"\
		"							\n\t"\
		"movaps	    (%%rbx),%%xmm2	\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"subpd	0x10(%%rbx),%%xmm2	\n\t"\
		"addpd	    (%%rbx),%%xmm3	\n\t"\
		"mulpd	    (%%rdi),%%xmm2	\n\t"\
		"mulpd	    (%%rdi),%%xmm3	\n\t"\
		"							\n\t"\
		"subpd	%%xmm2,%%xmm0		\n\t"\
		"subpd	%%xmm3,%%xmm1		\n\t"\
		"addpd	%%xmm2,%%xmm2		\n\t"\
		"addpd	%%xmm3,%%xmm3		\n\t"\
		"addpd	%%xmm0,%%xmm2		\n\t"\
		"addpd	%%xmm1,%%xmm3		\n\t"\
		"							\n\t"\
		"subpd	%%xmm6,%%xmm0		\n\t"\
		"subpd	%%xmm7,%%xmm1		\n\t"\
		"movaps	%%xmm0,    (%%rcx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rcx)	\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm0,%%xmm6		\n\t"\
		"addpd	%%xmm1,%%xmm7		\n\t"\
		"movaps	%%xmm6,    (%%rax)	\n\t"\
		"movaps	%%xmm7,0x10(%%rax)	\n\t"\
		"							\n\t"\
		"subpd	%%xmm5,%%xmm2		\n\t"\
		"subpd	%%xmm4,%%xmm3		\n\t"\
		"movaps	%%xmm2,    (%%rdx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rbx)	\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm2,%%xmm5		\n\t"\
		"addpd	%%xmm3,%%xmm4		\n\t"\
		"movaps	%%xmm5,    (%%rbx)	\n\t"\
		"movaps	%%xmm4,0x10(%%rdx)	\n\t"\
		:					/* outputs: none */\
		:[__in0] "m" (Xin0)	/* All inputs from memory addresses here */\
		,[__in1] "m" (Xin1)\
		,[__in2] "m" (Xin2)\
		,[__in3] "m" (Xin3)\
		,[__in4] "m" (Xin4)\
		,[__in5] "m" (Xin5)\
		,[__in6] "m" (Xin6)\
		,[__in7] "m" (Xin7)\
		,[__in8] "m" (Xin8)\
		,[__in9] "m" (Xin9)\
		,[__ina] "m" (Xina)\
		,[__inb] "m" (Xinb)\
		,[__inc] "m" (Xinc)\
		,[__ind] "m" (Xind)\
		,[__ine] "m" (Xine)\
		,[__inf] "m" (Xinf)\
		,[__isrt2] "m" (Xisrt2)\
		,[__two] "m" (Xtwo)\
		,[__out0] "m" (Xout0)\
		,[__o1] "e" (Xo1)\
		,[__o2] "e" (Xo2)\
		,[__o3] "e" (Xo3)\
		,[__o4] "e" (Xo4)\
		: "cc","memory","rax","rbx","rcx","rdx","rdi","rsi","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

	// Based on the SSE2_RADIX16_DIF_NOTWIDDLE macro in radix16_ditN_cy_dif1_gcc64.h, but with completely
	// specifiable 16-output addressing required for usage as the power-of-2 component of a twiddleless
	// radix = [odd*2^n] DFT routine.
	#define SSE2_RADIX16_DIF_0TWIDDLE(Xin0,Xi1,Xi2,Xi3,Xi4, Xisrt2,Xtwo, Xout0,Xout1,Xout2,Xout3,Xout4,Xout5,Xout6,Xout7,Xout8,Xout9,Xouta,Xoutb,Xoutc,Xoutd,Xoute,Xoutf)\
	{\
	__asm__ volatile (\
		"/* SSE2_RADIX4_DIF_IN_PLACE(r1 , r17, r9 , r25): */\n\t"\
		"movq	%[__in0],%%rax		\n\t"\
		"leaq	%c[__i4](%%rax),%%rcx	\n\t"/* __in0 +   [4*istride]; note BR of [a,b,c,d]-ptrs, i.e. b/c swap */\
		"leaq	%c[__i4](%%rcx),%%rbx	\n\t"/* __in0 + 2*[4*istride] */\
		"leaq	%c[__i4](%%rbx),%%rdx	\n\t"/* __in0 + 3*[4*istride] */\
		"							\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	    (%%rax),%%xmm2	\n\t"\
		"movaps	0x10(%%rax),%%xmm3	\n\t"\
		"							\n\t"\
		"addpd	    (%%rbx),%%xmm0	\n\t"\
		"addpd	0x10(%%rbx),%%xmm1	\n\t"\
		"subpd	    (%%rbx),%%xmm2	\n\t"\
		"subpd	0x10(%%rbx),%%xmm3	\n\t"\
		"							\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rcx),%%xmm6	\n\t"\
		"movaps	0x10(%%rcx),%%xmm7	\n\t"\
		"							\n\t"\
		"addpd	    (%%rdx),%%xmm4	\n\t"\
		"addpd	0x10(%%rdx),%%xmm5	\n\t"\
		"subpd	    (%%rdx),%%xmm6	\n\t"\
		"subpd	0x10(%%rdx),%%xmm7	\n\t"\
		"							\n\t"\
		"subpd	%%xmm4,%%xmm0		\n\t"\
		"subpd	%%xmm5,%%xmm1		\n\t"\
		"movaps	%%xmm0,    (%%rbx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rbx)	\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm0,%%xmm4		\n\t"\
		"addpd	%%xmm1,%%xmm5		\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t"\
		"							\n\t"\
		"subpd	%%xmm7,%%xmm2		\n\t"\
		"subpd	%%xmm6,%%xmm3		\n\t"\
		"movaps	%%xmm2,    (%%rcx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rdx)	\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm2,%%xmm7		\n\t"\
		"addpd	%%xmm3,%%xmm6		\n\t"\
		"movaps	%%xmm7,    (%%rdx)	\n\t"\
		"movaps	%%xmm6,0x10(%%rcx)	\n\t"\
		"/* SSE2_RADIX4_DIF_IN_PLACE(r5 , r21, r13, r29): */\n\t"\
		"leaq	%c[__i2](%%rax),%%rax	\n\t"/* All addresses += 2*ostride */\
		"leaq	%c[__i2](%%rbx),%%rbx	\n\t"\
		"leaq	%c[__i2](%%rcx),%%rcx	\n\t"\
		"leaq	%c[__i2](%%rdx),%%rdx	\n\t"\
		"							\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	    (%%rax),%%xmm2	\n\t"\
		"movaps	0x10(%%rax),%%xmm3	\n\t"\
		"							\n\t"\
		"addpd	    (%%rbx),%%xmm0	\n\t"\
		"addpd	0x10(%%rbx),%%xmm1	\n\t"\
		"subpd	    (%%rbx),%%xmm2	\n\t"\
		"subpd	0x10(%%rbx),%%xmm3	\n\t"\
		"							\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rcx),%%xmm6	\n\t"\
		"movaps	0x10(%%rcx),%%xmm7	\n\t"\
		"							\n\t"\
		"addpd	    (%%rdx),%%xmm4	\n\t"\
		"addpd	0x10(%%rdx),%%xmm5	\n\t"\
		"subpd	    (%%rdx),%%xmm6	\n\t"\
		"subpd	0x10(%%rdx),%%xmm7	\n\t"\
		"							\n\t"\
		"subpd	%%xmm4,%%xmm0		\n\t"\
		"subpd	%%xmm5,%%xmm1		\n\t"\
		"movaps	%%xmm0,    (%%rbx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rbx)	\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm0,%%xmm4		\n\t"\
		"addpd	%%xmm1,%%xmm5		\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t"\
		"							\n\t"\
		"subpd	%%xmm7,%%xmm2		\n\t"\
		"subpd	%%xmm6,%%xmm3		\n\t"\
		"movaps	%%xmm2,    (%%rcx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rdx)	\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm2,%%xmm7		\n\t"\
		"addpd	%%xmm3,%%xmm6		\n\t"\
		"movaps	%%xmm7,    (%%rdx)	\n\t"\
		"movaps	%%xmm6,0x10(%%rcx)	\n\t"\
		"/* SSE2_RADIX4_DIF_IN_PLACE(r3 , r19, r11, r27): */\n\t"\
		"leaq	-%c[__i1](%%rax),%%rax	\n\t"/* All addresses -= 1*ostride */\
		"leaq	-%c[__i1](%%rbx),%%rbx	\n\t"\
		"leaq	-%c[__i1](%%rcx),%%rcx	\n\t"\
		"leaq	-%c[__i1](%%rdx),%%rdx	\n\t"\
		"							\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	    (%%rax),%%xmm2	\n\t"\
		"movaps	0x10(%%rax),%%xmm3	\n\t"\
		"							\n\t"\
		"addpd	    (%%rbx),%%xmm0	\n\t"\
		"addpd	0x10(%%rbx),%%xmm1	\n\t"\
		"subpd	    (%%rbx),%%xmm2	\n\t"\
		"subpd	0x10(%%rbx),%%xmm3	\n\t"\
		"							\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rcx),%%xmm6	\n\t"\
		"movaps	0x10(%%rcx),%%xmm7	\n\t"\
		"							\n\t"\
		"addpd	    (%%rdx),%%xmm4	\n\t"\
		"addpd	0x10(%%rdx),%%xmm5	\n\t"\
		"subpd	    (%%rdx),%%xmm6	\n\t"\
		"subpd	0x10(%%rdx),%%xmm7	\n\t"\
		"							\n\t"\
		"subpd	%%xmm4,%%xmm0		\n\t"\
		"subpd	%%xmm5,%%xmm1		\n\t"\
		"movaps	%%xmm0,    (%%rbx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rbx)	\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm0,%%xmm4		\n\t"\
		"addpd	%%xmm1,%%xmm5		\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t"\
		"							\n\t"\
		"subpd	%%xmm7,%%xmm2		\n\t"\
		"subpd	%%xmm6,%%xmm3		\n\t"\
		"movaps	%%xmm2,    (%%rcx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rdx)	\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm2,%%xmm7		\n\t"\
		"addpd	%%xmm3,%%xmm6		\n\t"\
		"movaps	%%xmm7,    (%%rdx)	\n\t"\
		"movaps	%%xmm6,0x10(%%rcx)	\n\t"\
		"/* SSE2_RADIX4_DIF_IN_PLACE(r7 , r23, r15, r31): */\n\t"\
		"leaq	%c[__i2](%%rax),%%rax	\n\t"/* All addresses += 2*ostride */\
		"leaq	%c[__i2](%%rbx),%%rbx	\n\t"\
		"leaq	%c[__i2](%%rcx),%%rcx	\n\t"\
		"leaq	%c[__i2](%%rdx),%%rdx	\n\t"\
		"							\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	    (%%rax),%%xmm2	\n\t"\
		"movaps	0x10(%%rax),%%xmm3	\n\t"\
		"							\n\t"\
		"addpd	    (%%rbx),%%xmm0	\n\t"\
		"addpd	0x10(%%rbx),%%xmm1	\n\t"\
		"subpd	    (%%rbx),%%xmm2	\n\t"\
		"subpd	0x10(%%rbx),%%xmm3	\n\t"\
		"							\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rcx),%%xmm6	\n\t"\
		"movaps	0x10(%%rcx),%%xmm7	\n\t"\
		"							\n\t"\
		"addpd	    (%%rdx),%%xmm4	\n\t"\
		"addpd	0x10(%%rdx),%%xmm5	\n\t"\
		"subpd	    (%%rdx),%%xmm6	\n\t"\
		"subpd	0x10(%%rdx),%%xmm7	\n\t"\
		"							\n\t"\
		"subpd	%%xmm4,%%xmm0		\n\t"\
		"subpd	%%xmm5,%%xmm1		\n\t"\
		"movaps	%%xmm0,    (%%rbx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rbx)	\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm0,%%xmm4		\n\t"\
		"addpd	%%xmm1,%%xmm5		\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t"\
		"							\n\t"\
		"subpd	%%xmm7,%%xmm2		\n\t"\
		"subpd	%%xmm6,%%xmm3		\n\t"\
		"movaps	%%xmm2,    (%%rcx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rdx)	\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm2,%%xmm7		\n\t"\
		"addpd	%%xmm3,%%xmm6		\n\t"\
		"movaps	%%xmm7,    (%%rdx)	\n\t"\
		"movaps	%%xmm6,0x10(%%rcx)	\n\t"\
		"							\n\t"\
	/*** Now do 4 DFTs with internal twiddles on the 1*stride - separated data. Do blocks in order 0,2,1,3 to allow increment-only of rsi-datum from 1 block to the next: ***/\
		/* Block 0: r0-3 */\
		"movq	%[__in0],%%rsi	\n\t"\
		"movq	%[__out0],%%rax		\n\t"\
		"movq	%[__out1],%%rbx		\n\t"\
		"movq	%[__out2],%%rcx		\n\t"\
		"movq	%[__out3],%%rdx		\n\t"\
		"							\n\t"\
		"leaq	0x10(%%rsi),%%rdi	\n\t"/* Need separate address Im parts of outputs due to literal-offsets below */\
		"movaps	        (%%rsi),%%xmm0	\n\t"\
		"movaps	        (%%rdi),%%xmm1	\n\t"\
		"movaps	%c[__i2](%%rsi),%%xmm2	\n\t"\
		"movaps	%c[__i2](%%rdi),%%xmm3	\n\t"\
		"						\n\t"\
		"subpd	%c[__i2](%%rsi),%%xmm0	\n\t"\
		"subpd	%c[__i2](%%rdi),%%xmm1	\n\t"\
		"addpd	        (%%rsi),%%xmm2	\n\t"\
		"addpd	        (%%rdi),%%xmm3	\n\t"\
		"						\n\t"\
		"movaps	%c[__i1](%%rsi),%%xmm4	\n\t"\
		"movaps	%c[__i1](%%rdi),%%xmm5	\n\t"\
		"movaps	%c[__i3](%%rsi),%%xmm6	\n\t"\
		"movaps	%c[__i3](%%rdi),%%xmm7	\n\t"\
		"						\n\t"\
		"subpd	%c[__i3](%%rsi),%%xmm4	\n\t"\
		"subpd	%c[__i3](%%rdi),%%xmm5	\n\t"\
		"addpd	%c[__i1](%%rsi),%%xmm6	\n\t"\
		"addpd	%c[__i1](%%rdi),%%xmm7	\n\t"\
		"							\n\t"\
		"subpd	%%xmm6,%%xmm2		\n\t"\
		"subpd	%%xmm7,%%xmm3		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"movaps	%%xmm2,    (%%rbx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rbx)	\n\t"\
		"addpd	%%xmm2,		%%xmm6	\n\t"\
		"addpd	%%xmm3,		%%xmm7	\n\t"\
		"movaps	%%xmm6,    (%%rax)	\n\t"\
		"movaps	%%xmm7,0x10(%%rax)	\n\t"\
		"subpd	%%xmm5,		%%xmm0	\n\t"\
		"subpd	%%xmm4,		%%xmm1	\n\t"\
		"addpd	%%xmm5,		%%xmm5	\n\t"\
		"addpd	%%xmm4,		%%xmm4	\n\t"\
		"movaps	%%xmm0,    (%%rcx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rdx)	\n\t"\
		"addpd	%%xmm0,		%%xmm5	\n\t"\
		"addpd	%%xmm1,		%%xmm4	\n\t"\
		"movaps	%%xmm5,    (%%rdx)	\n\t"\
		"movaps	%%xmm4,0x10(%%rcx)	\n\t"\
		/* Block 2: */\
		"movq	%[__out8],%%rax		\n\t	leaq	%c[__i4](%%rsi),%%rsi	\n\t"/* __in0 + 4*ostride */\
		"movq	%[__out9],%%rbx		\n\t"\
		"movq	%[__outa],%%rcx		\n\t"\
		"movq	%[__outb],%%rdx		\n\t"\
		"leaq	0x10(%%rsi),%%rdi	\n\t"\
		"movaps	%c[__i1](%%rsi),%%xmm4	\n\t"\
		"movaps	%c[__i3](%%rsi),%%xmm6	\n\t"\
		"movaps	%c[__i1](%%rdi),%%xmm5	\n\t"\
		"movaps	%c[__i3](%%rdi),%%xmm7	\n\t"\
		"movaps	%%xmm4,%%xmm0		\n\t"\
		"movaps	%%xmm6,%%xmm2		\n\t"\
		"movaps	%%xmm5,%%xmm1		\n\t"\
		"movaps	%%xmm7,%%xmm3		\n\t"\
		"							\n\t"\
		"movq	%[__isrt2],%%rdi	\n\t"\
		"addq	$0x10,%%rdi	\n\t"/* cc0 */\
		"mulpd	    (%%rdi),%%xmm4	\n\t"\
		"mulpd	0x10(%%rdi),%%xmm6	\n\t"\
		"mulpd	0x10(%%rdi),%%xmm1	\n\t"\
		"mulpd	    (%%rdi),%%xmm3	\n\t"\
		"mulpd	    (%%rdi),%%xmm5	\n\t"\
		"mulpd	0x10(%%rdi),%%xmm7	\n\t"\
		"mulpd	0x10(%%rdi),%%xmm0	\n\t"\
		"mulpd	    (%%rdi),%%xmm2	\n\t"\
		"subpd	%%xmm1,%%xmm4		\n\t"\
		"subpd	%%xmm3,%%xmm6		\n\t"\
		"addpd	%%xmm0,%%xmm5		\n\t"\
		"addpd	%%xmm2,%%xmm7		\n\t"\
		"							\n\t"\
		"subpd	%%xmm6,%%xmm4		\n\t"\
		"subpd	%%xmm7,%%xmm5		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm4,%%xmm6		\n\t"\
		"addpd	%%xmm5,%%xmm7		\n\t"\
		"							\n\t"\
		"leaq	0x10(%%rsi),%%rdi	\n\t"\
		"movaps	%c[__i2](%%rsi),%%xmm2	\n\t"\
		"movaps	%c[__i2](%%rdi),%%xmm3	\n\t"\
		"subpd	%c[__i2](%%rdi),%%xmm2	\n\t"\
		"addpd	%c[__i2](%%rsi),%%xmm3	\n\t"\
		"movq	%[__isrt2],%%rdi	\n\t"\
		"mulpd	(%%rdi),%%xmm2	\n\t"/* mul by isrt2 */\
		"mulpd	(%%rdi),%%xmm3	\n\t"\
		"							\n\t"\
		"leaq	0x10(%%rsi),%%rdi	\n\t"\
		"movaps	        (%%rsi),%%xmm0	\n\t"\
		"movaps	        (%%rdi),%%xmm1	\n\t"\
		"subpd	%%xmm2,%%xmm0		\n\t"\
		"subpd	%%xmm3,%%xmm1		\n\t"\
		"addpd	        (%%rsi),%%xmm2	\n\t"\
		"addpd	        (%%rdi),%%xmm3	\n\t"\
		"							\n\t"\
		"subpd	%%xmm6,%%xmm2		\n\t"\
		"subpd	%%xmm7,%%xmm3		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"movaps	%%xmm2,    (%%rbx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rbx)	\n\t"\
		"addpd	%%xmm2,		%%xmm6	\n\t"\
		"addpd	%%xmm3,		%%xmm7	\n\t"\
		"movaps	%%xmm6,    (%%rax)	\n\t"\
		"movaps	%%xmm7,0x10(%%rax)	\n\t"\
		"subpd	%%xmm5,		%%xmm0	\n\t"\
		"subpd	%%xmm4,		%%xmm1	\n\t"\
		"addpd	%%xmm5,		%%xmm5	\n\t"\
		"addpd	%%xmm4,		%%xmm4	\n\t"\
		"movaps	%%xmm0,    (%%rcx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rdx)	\n\t"\
		"addpd	%%xmm0,		%%xmm5	\n\t"\
		"addpd	%%xmm1,		%%xmm4	\n\t"\
		"movaps	%%xmm5,    (%%rdx)	\n\t"\
		"movaps	%%xmm4,0x10(%%rcx)	\n\t"\
		/* Block 1: r8-b */\
		"movq	%[__out4],%%rax		\n\t	leaq	%c[__i4](%%rsi),%%rsi	\n\t"/* __in0 + 8*ostride */\
		"movq	%[__out5],%%rbx		\n\t"\
		"movq	%[__out6],%%rcx		\n\t"\
		"movq	%[__out7],%%rdx		\n\t"\
		"leaq	0x10(%%rsi),%%rdi	\n\t"\
		"movaps	        (%%rsi),%%xmm0	\n\t"\
		"movaps	        (%%rdi),%%xmm1	\n\t"\
		"movaps	%c[__i2](%%rsi),%%xmm2	\n\t"\
		"movaps	%c[__i2](%%rdi),%%xmm3	\n\t"\
		"						\n\t"\
		"subpd	%c[__i2](%%rdi),%%xmm0	\n\t"\
		"subpd	%c[__i2](%%rsi),%%xmm1	\n\t"\
		"addpd	        (%%rdi),%%xmm2	\n\t"\
		"addpd	        (%%rsi),%%xmm3	\n\t"\
		"						\n\t"\
		"movaps	%c[__i1](%%rsi),%%xmm4	\n\t"\
		"movaps	%c[__i1](%%rdi),%%xmm5	\n\t"\
		"movaps	%c[__i3](%%rsi),%%xmm6	\n\t"\
		"movaps	%c[__i3](%%rdi),%%xmm7	\n\t"\
		"						\n\t"\
		"subpd	%c[__i1](%%rdi),%%xmm4	\n\t"\
		"addpd	%c[__i1](%%rsi),%%xmm5	\n\t"\
		"addpd	%c[__i3](%%rdi),%%xmm6	\n\t"\
		"subpd	%c[__i3](%%rsi),%%xmm7	\n\t"\
		"							\n\t"\
		"movq	%[__isrt2],%%rdi	\n\t"\
		"mulpd	(%%rdi),%%xmm4		\n\t"\
		"mulpd	(%%rdi),%%xmm5		\n\t"\
		"mulpd	(%%rdi),%%xmm6		\n\t"\
		"mulpd	(%%rdi),%%xmm7		\n\t"\
		"							\n\t"\
		"subpd	%%xmm6,%%xmm4		\n\t"\
		"subpd	%%xmm7,%%xmm5		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm4,%%xmm6		\n\t"\
		"addpd	%%xmm5,%%xmm7		\n\t"\
		"							\n\t"\
		"subpd	%%xmm4,%%xmm0		\n\t"\
		"subpd	%%xmm5,%%xmm2		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"							\n\t"\
		"movaps	%%xmm0,    (%%rbx)	\n\t"\
		"movaps	%%xmm2,0x10(%%rbx)	\n\t"\
		"addpd	%%xmm0,		%%xmm4	\n\t"\
		"addpd	%%xmm2,		%%xmm5	\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t"\
		"subpd	%%xmm7,		%%xmm3	\n\t"\
		"subpd	%%xmm6,		%%xmm1	\n\t"\
		"addpd	%%xmm7,		%%xmm7	\n\t"\
		"addpd	%%xmm6,		%%xmm6	\n\t"\
		"movaps	%%xmm3,    (%%rcx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rdx)	\n\t"\
		"addpd	%%xmm3,		%%xmm7	\n\t"\
		"addpd	%%xmm1,		%%xmm6	\n\t"\
		"movaps	%%xmm7,    (%%rdx)	\n\t"\
		"movaps	%%xmm6,0x10(%%rcx)	\n\t"\
		/* Block 3: */\
		"movq	%[__outc],%%rax		\n\t	leaq	%c[__i4](%%rsi),%%rsi	\n\t"/* __in0 + c*ostride */\
		"movq	%[__outd],%%rbx		\n\t"\
		"movq	%[__oute],%%rcx		\n\t"\
		"movq	%[__outf],%%rdx		\n\t"\
		"leaq	0x10(%%rsi),%%rdi	\n\t"\
		"movaps	%c[__i1](%%rsi),%%xmm4	\n\t"\
		"movaps	%c[__i3](%%rsi),%%xmm6	\n\t"\
		"movaps	%c[__i1](%%rdi),%%xmm5	\n\t"\
		"movaps	%c[__i3](%%rdi),%%xmm7	\n\t"\
		"movaps	%%xmm4,%%xmm0		\n\t"\
		"movaps	%%xmm6,%%xmm2		\n\t"\
		"movaps	%%xmm5,%%xmm1		\n\t"\
		"movaps	%%xmm7,%%xmm3		\n\t"\
		"							\n\t"\
		"movq	%[__isrt2],%%rdi	\n\t"\
		"addq	$0x10,%%rdi	\n\t"/* cc0 */\
		"mulpd	0x10(%%rdi),%%xmm4	\n\t"\
		"mulpd	    (%%rdi),%%xmm6	\n\t"\
		"mulpd	    (%%rdi),%%xmm1	\n\t"\
		"mulpd	0x10(%%rdi),%%xmm3	\n\t"\
		"mulpd	0x10(%%rdi),%%xmm5	\n\t"\
		"mulpd	    (%%rdi),%%xmm7	\n\t"\
		"mulpd	    (%%rdi),%%xmm0	\n\t"\
		"mulpd	0x10(%%rdi),%%xmm2	\n\t"\
		"subpd	%%xmm1,%%xmm4		\n\t"\
		"subpd	%%xmm3,%%xmm6		\n\t"\
		"addpd	%%xmm0,%%xmm5		\n\t"\
		"addpd	%%xmm2,%%xmm7		\n\t"\
		"							\n\t"\
		"subpd	%%xmm6,%%xmm4		\n\t"\
		"subpd	%%xmm7,%%xmm5		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm4,%%xmm6		\n\t"\
		"addpd	%%xmm5,%%xmm7		\n\t"\
		"							\n\t"\
		"leaq	0x10(%%rsi),%%rdi	\n\t"\
		"movaps	%c[__i2](%%rsi),%%xmm2	\n\t"\
		"movaps	%c[__i2](%%rdi),%%xmm3	\n\t"\
		"addpd	%c[__i2](%%rdi),%%xmm2	\n\t"\
		"subpd	%c[__i2](%%rsi),%%xmm3	\n\t"\
		"movq	%[__isrt2],%%rdi	\n\t"\
		"mulpd	(%%rdi),%%xmm2	\n\t"/* mul by isrt2 */\
		"mulpd	(%%rdi),%%xmm3	\n\t"\
		"							\n\t"\
		"leaq	0x10(%%rsi),%%rdi	\n\t"\
		"movaps	        (%%rsi),%%xmm0	\n\t"\
		"movaps	        (%%rdi),%%xmm1	\n\t"\
		"subpd	%%xmm2,%%xmm0		\n\t"\
		"subpd	%%xmm3,%%xmm1		\n\t"\
		"addpd	        (%%rsi),%%xmm2	\n\t"\
		"addpd	        (%%rdi),%%xmm3	\n\t"\
		"							\n\t"\
		"subpd	%%xmm4,%%xmm0		\n\t"\
		"subpd	%%xmm5,%%xmm1		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"movaps	%%xmm0,    (%%rbx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rbx)	\n\t"\
		"addpd	%%xmm0,		%%xmm4	\n\t"\
		"addpd	%%xmm1,		%%xmm5	\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t"\
		"subpd	%%xmm7,		%%xmm2	\n\t"\
		"subpd	%%xmm6,		%%xmm3	\n\t"\
		"addpd	%%xmm7,		%%xmm7	\n\t"\
		"addpd	%%xmm6,		%%xmm6	\n\t"\
		"movaps	%%xmm2,    (%%rcx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rdx)	\n\t"\
		"addpd	%%xmm2,		%%xmm7	\n\t"\
		"addpd	%%xmm3,		%%xmm6	\n\t"\
		"movaps	%%xmm7,    (%%rdx)	\n\t"\
		"movaps	%%xmm6,0x10(%%rcx)	\n\t"\
		:					/* outputs: none */\
		:[__in0] "m" (Xin0)	/* All inputs from memory addresses here */\
		,[__i1] "e" (Xi1)\
		,[__i2] "e" (Xi2)\
		,[__i3] "e" (Xi3)\
		,[__i4] "e" (Xi4)\
		,[__isrt2] "m" (Xisrt2)\
		,[__two] "m" (Xtwo)\
		,[__out0] "m" (Xout0)\
		,[__out1] "m" (Xout1)\
		,[__out2] "m" (Xout2)\
		,[__out3] "m" (Xout3)\
		,[__out4] "m" (Xout4)\
		,[__out5] "m" (Xout5)\
		,[__out6] "m" (Xout6)\
		,[__out7] "m" (Xout7)\
		,[__out8] "m" (Xout8)\
		,[__out9] "m" (Xout9)\
		,[__outa] "m" (Xouta)\
		,[__outb] "m" (Xoutb)\
		,[__outc] "m" (Xoutc)\
		,[__outd] "m" (Xoutd)\
		,[__oute] "m" (Xoute)\
		,[__outf] "m" (Xoutf)\
		: "cc","memory","rax","rbx","rcx","rdx","rdi","rsi","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

	// Same as above, but with specifiable I-addresses and regularly spaced O-addresses:

  #if 0	// 16-register version: This proves slower than 8-reg in both my SSE2/Core2 ad AVX/Haswell tests:

	#define SSE2_RADIX16_DIF_0TWIDDLE_B(Xin0,Xin1,Xin2,Xin3,Xin4,Xin5,Xin6,Xin7,Xin8,Xin9,Xina,Xinb,Xinc,Xind,Xine,Xinf, Xisrt2,Xtwo, Xout0,Xout1,Xout2,Xout3)\
	{\
	__asm__ volatile (\
	/* SSE2_RADIX4_DIF_IN_PLACE(r1,r17,r9,r25) 	SSE2_RADIX4_DIF_IN_PLACE(r3,r19,r11,r27): */\
		"movq	%[__in0],%%rax		\n\t		movq	%[__in1],%%r10		\n\t"\
		"movq	%[__in8],%%rbx		\n\t		movq	%[__in9],%%r11		\n\t"\
		"movq	%[__in4],%%rcx		\n\t		movq	%[__in5],%%r12		\n\t"\
		"movq	%[__inc],%%rdx		\n\t		movq	%[__ind],%%r13		\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t		movaps	    (%%r10),%%xmm8	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t		movaps	0x10(%%r10),%%xmm9	\n\t"\
		"movaps	    (%%rax),%%xmm2	\n\t		movaps	    (%%r10),%%xmm10	\n\t"\
		"movaps	0x10(%%rax),%%xmm3	\n\t		movaps	0x10(%%r10),%%xmm11	\n\t"\
		"addpd	    (%%rbx),%%xmm0	\n\t		addpd	    (%%r11),%%xmm8	\n\t"\
		"addpd	0x10(%%rbx),%%xmm1	\n\t		addpd	0x10(%%r11),%%xmm9	\n\t"\
		"subpd	    (%%rbx),%%xmm2	\n\t		subpd	    (%%r11),%%xmm10	\n\t"\
		"subpd	0x10(%%rbx),%%xmm3	\n\t		subpd	0x10(%%r11),%%xmm11	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t		movaps	    (%%r12),%%xmm12	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t		movaps	0x10(%%r12),%%xmm13	\n\t"\
		"movaps	    (%%rcx),%%xmm6	\n\t		movaps	    (%%r12),%%xmm14	\n\t"\
		"movaps	0x10(%%rcx),%%xmm7	\n\t		movaps	0x10(%%r12),%%xmm15	\n\t"\
		"addpd	    (%%rdx),%%xmm4	\n\t		addpd	    (%%r13),%%xmm12	\n\t"\
		"addpd	0x10(%%rdx),%%xmm5	\n\t		addpd	0x10(%%r13),%%xmm13	\n\t"\
		"subpd	    (%%rdx),%%xmm6	\n\t		subpd	    (%%r13),%%xmm14	\n\t"\
		"subpd	0x10(%%rdx),%%xmm7	\n\t		subpd	0x10(%%r13),%%xmm15	\n\t"\
		"subpd	%%xmm4,%%xmm0		\n\t		subpd	%%xmm12,%%xmm8		\n\t"\
		"subpd	%%xmm5,%%xmm1		\n\t		subpd	%%xmm13,%%xmm9		\n\t"\
		"movaps	%%xmm0,    (%%rbx)	\n\t		movaps	%%xmm8,    (%%r11)	\n\t"\
		"movaps	%%xmm1,0x10(%%rbx)	\n\t		movaps	%%xmm9,0x10(%%r11)	\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t		addpd	%%xmm12,%%xmm12		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t		addpd	%%xmm13,%%xmm13		\n\t"\
		"addpd	%%xmm0,%%xmm4		\n\t		addpd	%%xmm8,%%xmm12		\n\t"\
		"addpd	%%xmm1,%%xmm5		\n\t		addpd	%%xmm9,%%xmm13		\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t		movaps	%%xmm12,    (%%r10)	\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t		movaps	%%xmm13,0x10(%%r10)	\n\t"\
		"subpd	%%xmm7,%%xmm2		\n\t		subpd	%%xmm15,%%xmm10		\n\t"\
		"subpd	%%xmm6,%%xmm3		\n\t		subpd	%%xmm14,%%xmm11		\n\t"\
		"movaps	%%xmm2,    (%%rcx)	\n\t		movaps	%%xmm10,    (%%r12)	\n\t"\
		"movaps	%%xmm3,0x10(%%rdx)	\n\t		movaps	%%xmm11,0x10(%%r13)	\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t		addpd	%%xmm15,%%xmm15		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t		addpd	%%xmm14,%%xmm14		\n\t"\
		"addpd	%%xmm2,%%xmm7		\n\t		addpd	%%xmm10,%%xmm15		\n\t"\
		"addpd	%%xmm3,%%xmm6		\n\t		addpd	%%xmm11,%%xmm14		\n\t"\
		"movaps	%%xmm7,    (%%rdx)	\n\t		movaps	%%xmm15,    (%%r13)	\n\t"\
		"movaps	%%xmm6,0x10(%%rcx)	\n\t		movaps	%%xmm14,0x10(%%r12)	\n\t"\
	/* SSE2_RADIX4_DIF_IN_PLACE(r5,r21,r13,r29)	SSE2_RADIX4_DIF_IN_PLACE(r7,r23,r15,r31): */\
		"movq	%[__in2],%%rax		\n\t		movq	%[__in3],%%r10		\n\t"\
		"movq	%[__ina],%%rbx		\n\t		movq	%[__inb],%%r11		\n\t"\
		"movq	%[__in6],%%rcx		\n\t		movq	%[__in7],%%r12		\n\t"\
		"movq	%[__ine],%%rdx		\n\t		movq	%[__inf],%%r13		\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t		movaps	    (%%r10),%%xmm8	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t		movaps	0x10(%%r10),%%xmm9	\n\t"\
		"movaps	    (%%rax),%%xmm2	\n\t		movaps	    (%%r10),%%xmm10	\n\t"\
		"movaps	0x10(%%rax),%%xmm3	\n\t		movaps	0x10(%%r10),%%xmm11	\n\t"\
		"addpd	    (%%rbx),%%xmm0	\n\t		addpd	    (%%r11),%%xmm8	\n\t"\
		"addpd	0x10(%%rbx),%%xmm1	\n\t		addpd	0x10(%%r11),%%xmm9	\n\t"\
		"subpd	    (%%rbx),%%xmm2	\n\t		subpd	    (%%r11),%%xmm10	\n\t"\
		"subpd	0x10(%%rbx),%%xmm3	\n\t		subpd	0x10(%%r11),%%xmm11	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t		movaps	    (%%r12),%%xmm12	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t		movaps	0x10(%%r12),%%xmm13	\n\t"\
		"movaps	    (%%rcx),%%xmm6	\n\t		movaps	    (%%r12),%%xmm14	\n\t"\
		"movaps	0x10(%%rcx),%%xmm7	\n\t		movaps	0x10(%%r12),%%xmm15	\n\t"\
		"addpd	    (%%rdx),%%xmm4	\n\t		addpd	    (%%r13),%%xmm12	\n\t"\
		"addpd	0x10(%%rdx),%%xmm5	\n\t		addpd	0x10(%%r13),%%xmm13	\n\t"\
		"subpd	    (%%rdx),%%xmm6	\n\t		subpd	    (%%r13),%%xmm14	\n\t"\
		"subpd	0x10(%%rdx),%%xmm7	\n\t		subpd	0x10(%%r13),%%xmm15	\n\t"\
		"subpd	%%xmm4,%%xmm0		\n\t		subpd	%%xmm12,%%xmm8		\n\t"\
		"subpd	%%xmm5,%%xmm1		\n\t		subpd	%%xmm13,%%xmm9		\n\t"\
		"movaps	%%xmm0,    (%%rbx)	\n\t		movaps	%%xmm8,    (%%r11)	\n\t"\
		"movaps	%%xmm1,0x10(%%rbx)	\n\t		movaps	%%xmm9,0x10(%%r11)	\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t		addpd	%%xmm12,%%xmm12		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t		addpd	%%xmm13,%%xmm13		\n\t"\
		"addpd	%%xmm0,%%xmm4		\n\t		addpd	%%xmm8,%%xmm12		\n\t"\
		"addpd	%%xmm1,%%xmm5		\n\t		addpd	%%xmm9,%%xmm13		\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t		movaps	%%xmm12,    (%%r10)	\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t		movaps	%%xmm13,0x10(%%r10)	\n\t"\
		"subpd	%%xmm7,%%xmm2		\n\t		subpd	%%xmm15,%%xmm10		\n\t"\
		"subpd	%%xmm6,%%xmm3		\n\t		subpd	%%xmm14,%%xmm11		\n\t"\
		"movaps	%%xmm2,    (%%rcx)	\n\t		movaps	%%xmm10,    (%%r12)	\n\t"\
		"movaps	%%xmm3,0x10(%%rdx)	\n\t		movaps	%%xmm11,0x10(%%r13)	\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t		addpd	%%xmm15,%%xmm15		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t		addpd	%%xmm14,%%xmm14		\n\t"\
		"addpd	%%xmm2,%%xmm7		\n\t		addpd	%%xmm10,%%xmm15		\n\t"\
		"addpd	%%xmm3,%%xmm6		\n\t		addpd	%%xmm11,%%xmm14		\n\t"\
		"movaps	%%xmm7,    (%%rdx)	\n\t		movaps	%%xmm15,    (%%r13)	\n\t"\
		"movaps	%%xmm6,0x10(%%rcx)	\n\t		movaps	%%xmm14,0x10(%%r12)	\n\t"\
	/*** Now do 4 DFTs with internal twiddles on the 1*stride - separated data. Do blocks in order 0,2,1,3 to allow increment-only of rsi-datum from 1 block to the next: ***/\
		"movq	%[__isrt2],%%rdi	\n\t"\
	/* Block 0: r0-3 */						/* Block 1: r8-b */\
		"movq	%[__out0],%%rsi		\n\t		movq	%[__out1],%%r8		\n\t"\
		"movq	%[__in0],%%rax		\n\t		movq	%[__in8],%%r10		\n\t"\
		"movq	%[__in2],%%rbx		\n\t		movq	%[__ina],%%r11		\n\t"\
		"movq	%[__in1],%%rcx		\n\t		movq	%[__in9],%%r12		\n\t"\
		"movq	%[__in3],%%rdx		\n\t		movq	%[__inb],%%r13		\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t		movaps	    (%%r10),%%xmm8	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t		movaps	0x10(%%r10),%%xmm9	\n\t"\
		"movaps		(%%rbx),%%xmm2	\n\t		movaps		(%%r11),%%xmm10	\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t		movaps	0x10(%%r11),%%xmm11	\n\t"\
		"subpd		(%%rbx),%%xmm0	\n\t		subpd	0x10(%%r11),%%xmm8	\n\t"\
		"subpd	0x10(%%rbx),%%xmm1	\n\t		subpd		(%%r11),%%xmm9	\n\t"\
		"addpd	    (%%rax),%%xmm2	\n\t		addpd	0x10(%%r10),%%xmm10	\n\t"\
		"addpd	0x10(%%rax),%%xmm3	\n\t		addpd		(%%r10),%%xmm11	\n\t"\
		"movaps		(%%rcx),%%xmm4	\n\t		movaps		(%%r12),%%xmm12	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t		movaps	0x10(%%r12),%%xmm13	\n\t"\
		"movaps	    (%%rdx),%%xmm6	\n\t		movaps	    (%%r13),%%xmm14	\n\t"\
		"movaps	0x10(%%rdx),%%xmm7	\n\t		movaps	0x10(%%r13),%%xmm15	\n\t"\
		"subpd		(%%rdx),%%xmm4	\n\t		subpd	0x10(%%r12),%%xmm12	\n\t"\
		"subpd	0x10(%%rdx),%%xmm5	\n\t		addpd		(%%r12),%%xmm13	\n\t"\
		"addpd	    (%%rcx),%%xmm6	\n\t		addpd	0x10(%%r13),%%xmm14	\n\t"\
		"addpd	0x10(%%rcx),%%xmm7	\n\t		subpd		(%%r13),%%xmm15	\n\t"\
		"subpd	%%xmm6,%%xmm2		\n\t		mulpd	(%%rdi),%%xmm12		\n\t"\
		"subpd	%%xmm7,%%xmm3		\n\t		mulpd	(%%rdi),%%xmm13		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t		mulpd	(%%rdi),%%xmm14		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t		mulpd	(%%rdi),%%xmm15		\n\t"\
		"movaps	%%xmm2,0x20(%%rsi)	\n\t		subpd	%%xmm14,%%xmm12		\n\t"\
		"movaps	%%xmm3,0x30(%%rsi)	\n\t		subpd	%%xmm15,%%xmm13		\n\t"\
		"addpd	%%xmm2,%%xmm6		\n\t		addpd	%%xmm14,%%xmm14		\n\t"\
		"addpd	%%xmm3,%%xmm7		\n\t		addpd	%%xmm15,%%xmm15		\n\t"\
		"movaps	%%xmm6,    (%%rsi)	\n\t		addpd	%%xmm12,%%xmm14		\n\t"\
		"movaps	%%xmm7,0x10(%%rsi)	\n\t		addpd	%%xmm13,%%xmm15		\n\t"\
		"subpd	%%xmm5,%%xmm0		\n\t		subpd	%%xmm12,%%xmm8		\n\t"\
		"subpd	%%xmm4,%%xmm1		\n\t		subpd	%%xmm13,%%xmm10		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t		addpd	%%xmm12,%%xmm12		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t		addpd	%%xmm13,%%xmm13		\n\t"\
		"movaps	%%xmm0,0x40(%%rsi)	\n\t		movaps	%%xmm8,0x20(%%r8)	\n\t"\
		"movaps	%%xmm1,0x70(%%rsi)	\n\t		movaps	%%xmm10,0x30(%%r8)	\n\t"\
		"addpd	%%xmm0,%%xmm5		\n\t		addpd	%%xmm8,%%xmm12		\n\t"\
		"addpd	%%xmm1,%%xmm4		\n\t		addpd	%%xmm10,%%xmm13		\n\t"\
		"movaps	%%xmm5,0x60(%%rsi)	\n\t		movaps	%%xmm12,    (%%r8)	\n\t"\
		"movaps	%%xmm4,0x50(%%rsi)	\n\t		movaps	%%xmm13,0x10(%%r8)	\n\t"\
	/* Block 2: */							"	subpd	%%xmm15,%%xmm11		\n\t"\
		"movq	%[__out2],%%rsi		\n\t		subpd	%%xmm14,%%xmm9		\n\t"\
		"movq	%[__in4],%%rax		\n\t		addpd	%%xmm15,%%xmm15		\n\t"\
		"movq	%[__in6],%%rbx		\n\t		addpd	%%xmm14,%%xmm14		\n\t"\
		"movq	%[__in5],%%rcx		\n\t		movaps	%%xmm11,0x40(%%r8)	\n\t"\
		"movq	%[__in7],%%rdx		\n\t		movaps	%%xmm9,0x70(%%r8)	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t		addpd	%%xmm11,%%xmm15		\n\t"\
		"movaps		(%%rdx),%%xmm6	\n\t		addpd	%%xmm9,%%xmm14		\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t		movaps	%%xmm15,0x60(%%r8)	\n\t"\
		"movaps	0x10(%%rdx),%%xmm7	\n\t		movaps	%%xmm14,0x50(%%r8)	\n\t"\
		"movaps	%%xmm4,%%xmm0		\n\t"	/* Block 3: */\
		"movaps	%%xmm6,%%xmm2		\n\t		movq	%[__out3],%%r8		\n\t"\
		"movaps	%%xmm5,%%xmm1		\n\t		movq	%[__inc],%%r10		\n\t"\
		"movaps	%%xmm7,%%xmm3		\n\t		movq	%[__ine],%%r11		\n\t"\
		/* cc0,ss0 = 0x[1,2]0(%%rdi): */	"	movq	%[__ind],%%r12		\n\t"\
		"mulpd	0x10(%%rdi),%%xmm4	\n\t		movq	%[__inf],%%r13		\n\t"\
		"mulpd	0x20(%%rdi),%%xmm6	\n\t		movaps	    (%%r12),%%xmm12	\n\t"\
		"mulpd	0x20(%%rdi),%%xmm1	\n\t		movaps		(%%r13),%%xmm14	\n\t"\
		"mulpd	0x10(%%rdi),%%xmm3	\n\t		movaps	0x10(%%r12),%%xmm13	\n\t"\
		"mulpd	0x10(%%rdi),%%xmm5	\n\t		movaps	0x10(%%r13),%%xmm15	\n\t"\
		"mulpd	0x20(%%rdi),%%xmm7	\n\t		movaps	%%xmm12,%%xmm8		\n\t"\
		"mulpd	0x20(%%rdi),%%xmm0	\n\t		movaps	%%xmm14,%%xmm10		\n\t"\
		"mulpd	0x10(%%rdi),%%xmm2	\n\t		movaps	%%xmm13,%%xmm9		\n\t"\
		"subpd	%%xmm1,%%xmm4		\n\t		movaps	%%xmm15,%%xmm11		\n\t"\
		"subpd	%%xmm3,%%xmm6		\n\t		mulpd	0x20(%%rdi),%%xmm12	\n\t"\
		"addpd	%%xmm0,%%xmm5		\n\t		mulpd	0x10(%%rdi),%%xmm14	\n\t"\
		"addpd	%%xmm2,%%xmm7		\n\t		mulpd	0x10(%%rdi),%%xmm9	\n\t"\
		"subpd	%%xmm6,%%xmm4		\n\t		mulpd	0x20(%%rdi),%%xmm11	\n\t"\
		"subpd	%%xmm7,%%xmm5		\n\t		mulpd	0x20(%%rdi),%%xmm13	\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t		mulpd	0x10(%%rdi),%%xmm15	\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t		mulpd	0x10(%%rdi),%%xmm8	\n\t"\
		"addpd	%%xmm4,%%xmm6		\n\t		mulpd	0x20(%%rdi),%%xmm10	\n\t"\
		"addpd	%%xmm5,%%xmm7		\n\t		subpd	%%xmm9,%%xmm12		\n\t"\
		"movaps	    (%%rbx),%%xmm2	\n\t		subpd	%%xmm11,%%xmm14		\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t		addpd	%%xmm8,%%xmm13		\n\t"\
		"subpd	0x10(%%rbx),%%xmm2	\n\t		addpd	%%xmm10,%%xmm15		\n\t"\
		"addpd		(%%rbx),%%xmm3	\n\t		subpd	%%xmm14,%%xmm12		\n\t"\
		"mulpd	(%%rdi),%%xmm2		\n\t		subpd	%%xmm15,%%xmm13		\n\t"\
		"mulpd	(%%rdi),%%xmm3		\n\t		addpd	%%xmm14,%%xmm14		\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t		addpd	%%xmm15,%%xmm15		\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t		addpd	%%xmm12,%%xmm14		\n\t"\
		"subpd	%%xmm2,%%xmm0		\n\t		addpd	%%xmm13,%%xmm15		\n\t"\
		"subpd	%%xmm3,%%xmm1		\n\t		movaps	    (%%r11),%%xmm10	\n\t"\
		"addpd	    (%%rax),%%xmm2	\n\t		movaps	0x10(%%r11),%%xmm11	\n\t"\
		"addpd	0x10(%%rax),%%xmm3	\n\t		addpd	0x10(%%r11),%%xmm10	\n\t"\
		"subpd	%%xmm6,%%xmm2		\n\t		subpd	    (%%r11),%%xmm11	\n\t"\
		"subpd	%%xmm7,%%xmm3		\n\t		mulpd	(%%rdi),%%xmm10		\n\t"/* mul by isrt2 */\
		"addpd	%%xmm6,%%xmm6		\n\t		mulpd	(%%rdi),%%xmm11		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t		movaps	    (%%r10),%%xmm8	\n\t"\
		"movaps	%%xmm2,0x20(%%rsi)	\n\t		movaps	0x10(%%r10),%%xmm9	\n\t"\
		"movaps	%%xmm3,0x30(%%rsi)	\n\t		subpd	%%xmm10,%%xmm8		\n\t"\
		"addpd	%%xmm2,%%xmm6		\n\t		subpd	%%xmm11,%%xmm9		\n\t"\
		"addpd	%%xmm3,%%xmm7		\n\t		addpd	    (%%r10),%%xmm10	\n\t"\
		"movaps	%%xmm6,    (%%rsi)	\n\t		addpd	0x10(%%r10),%%xmm11	\n\t"\
		"movaps	%%xmm7,0x10(%%rsi)	\n\t		subpd	%%xmm12,%%xmm8		\n\t"\
		"subpd	%%xmm5,%%xmm0		\n\t		subpd	%%xmm13,%%xmm9		\n\t"\
		"subpd	%%xmm4,%%xmm1		\n\t		addpd	%%xmm12,%%xmm12		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t		addpd	%%xmm13,%%xmm13		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t		movaps	%%xmm8,0x20(%%r8)	\n\t"\
		"movaps	%%xmm0,0x40(%%rsi)	\n\t		movaps	%%xmm9,0x30(%%r8)	\n\t"\
		"movaps	%%xmm1,0x70(%%rsi)	\n\t		addpd	%%xmm8,%%xmm12		\n\t"\
		"addpd	%%xmm0,%%xmm5		\n\t		addpd	%%xmm9,%%xmm13		\n\t"\
		"addpd	%%xmm1,%%xmm4		\n\t		movaps	%%xmm12,    (%%r8)	\n\t"\
		"movaps	%%xmm5,0x60(%%rsi)	\n\t		movaps	%%xmm13,0x10(%%r8)	\n\t"\
		"movaps	%%xmm4,0x50(%%rsi)	\n\t		subpd	%%xmm15,%%xmm10		\n\t"\
		"										subpd	%%xmm14,%%xmm11		\n\t"\
		"										addpd	%%xmm15,%%xmm15		\n\t"\
		"										addpd	%%xmm14,%%xmm14		\n\t"\
		"										movaps	%%xmm10,0x40(%%r8)	\n\t"\
		"										movaps	%%xmm11,0x70(%%r8)	\n\t"\
		"										addpd	%%xmm10,%%xmm15		\n\t"\
		"										addpd	%%xmm11,%%xmm14		\n\t"\
		"										movaps	%%xmm15,0x60(%%r8)	\n\t"\
		"										movaps	%%xmm14,0x50(%%r8)	\n\t"\
		:					/* outputs: none */\
		:[__in0] "m" (Xin0)	/* All inputs from memory addresses here */\
		,[__in1] "m" (Xin1)\
		,[__in2] "m" (Xin2)\
		,[__in3] "m" (Xin3)\
		,[__in4] "m" (Xin4)\
		,[__in5] "m" (Xin5)\
		,[__in6] "m" (Xin6)\
		,[__in7] "m" (Xin7)\
		,[__in8] "m" (Xin8)\
		,[__in9] "m" (Xin9)\
		,[__ina] "m" (Xina)\
		,[__inb] "m" (Xinb)\
		,[__inc] "m" (Xinc)\
		,[__ind] "m" (Xind)\
		,[__ine] "m" (Xine)\
		,[__inf] "m" (Xinf)\
		,[__isrt2] "m" (Xisrt2)\
		,[__two] "m" (Xtwo)\
		,[__out0] "m" (Xout0)\
		,[__out1] "m" (Xout1)\
		,[__out2] "m" (Xout2)\
		,[__out3] "m" (Xout3)\
		: "cc","memory","rax","rbx","rcx","rdx","rdi","rsi","r8","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

  #else

	#define SSE2_RADIX16_DIF_0TWIDDLE_B(Xin0,Xi1,Xi2,Xi3,Xi4, Xisrt2,Xtwo, Xout0)\
	{\
	__asm__ volatile (\
		/* SSE2_RADIX4_DIF_IN_PLACE(r1 , r17, r9 , r25): */\
		"movq	%[__in0],%%rax		\n\t"\
		"leaq	%c[__i4](%%rax),%%rcx	\n\t"/* __in0 +   [4*istride]; note BR of [a,b,c,d]-ptrs, i.e. b/c swap */\
		"leaq	%c[__i4](%%rcx),%%rbx	\n\t"/* __in0 + 2*[4*istride] */\
		"leaq	%c[__i4](%%rbx),%%rdx	\n\t"/* __in0 + 3*[4*istride] */\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	    (%%rax),%%xmm2	\n\t"\
		"movaps	0x10(%%rax),%%xmm3	\n\t"\
		"addpd	    (%%rbx),%%xmm0	\n\t"\
		"addpd	0x10(%%rbx),%%xmm1	\n\t"\
		"subpd	    (%%rbx),%%xmm2	\n\t"\
		"subpd	0x10(%%rbx),%%xmm3	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rcx),%%xmm6	\n\t"\
		"movaps	0x10(%%rcx),%%xmm7	\n\t"\
		"addpd	    (%%rdx),%%xmm4	\n\t"\
		"addpd	0x10(%%rdx),%%xmm5	\n\t"\
		"subpd	    (%%rdx),%%xmm6	\n\t"\
		"subpd	0x10(%%rdx),%%xmm7	\n\t"\
		"subpd	%%xmm4,%%xmm0	\n\t"\
		"subpd	%%xmm5,%%xmm1	\n\t"\
		"movaps	%%xmm0,    (%%rbx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rbx)	\n\t"\
		"addpd	%%xmm4,%%xmm4	\n\t"\
		"addpd	%%xmm5,%%xmm5	\n\t"\
		"addpd	%%xmm0,%%xmm4	\n\t"\
		"addpd	%%xmm1,%%xmm5	\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t"\
		"subpd	%%xmm7,%%xmm2	\n\t"\
		"subpd	%%xmm6,%%xmm3	\n\t"\
		"movaps	%%xmm2,    (%%rcx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rdx)	\n\t"\
		"addpd	%%xmm7,%%xmm7	\n\t"\
		"addpd	%%xmm6,%%xmm6	\n\t"\
		"addpd	%%xmm2,%%xmm7	\n\t"\
		"addpd	%%xmm3,%%xmm6	\n\t"\
		"movaps	%%xmm7,    (%%rdx)	\n\t"\
		"movaps	%%xmm6,0x10(%%rcx)	\n\t"\
		/* SSE2_RADIX4_DIF_IN_PLACE(r5 , r21, r13, r29): */\
		"addq	$%c[__i2],%%rax	\n\t"/* All addresses += 2*ostride */\
		"addq	$%c[__i2],%%rbx	\n\t"\
		"addq	$%c[__i2],%%rcx	\n\t"\
		"addq	$%c[__i2],%%rdx	\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	    (%%rax),%%xmm2	\n\t"\
		"movaps	0x10(%%rax),%%xmm3	\n\t"\
		"addpd	    (%%rbx),%%xmm0	\n\t"\
		"addpd	0x10(%%rbx),%%xmm1	\n\t"\
		"subpd	    (%%rbx),%%xmm2	\n\t"\
		"subpd	0x10(%%rbx),%%xmm3	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rcx),%%xmm6	\n\t"\
		"movaps	0x10(%%rcx),%%xmm7	\n\t"\
		"addpd	    (%%rdx),%%xmm4	\n\t"\
		"addpd	0x10(%%rdx),%%xmm5	\n\t"\
		"subpd	    (%%rdx),%%xmm6	\n\t"\
		"subpd	0x10(%%rdx),%%xmm7	\n\t"\
		"subpd	%%xmm4,%%xmm0	\n\t"\
		"subpd	%%xmm5,%%xmm1	\n\t"\
		"movaps	%%xmm0,    (%%rbx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rbx)	\n\t"\
		"addpd	%%xmm4,%%xmm4	\n\t"\
		"addpd	%%xmm5,%%xmm5	\n\t"\
		"addpd	%%xmm0,%%xmm4	\n\t"\
		"addpd	%%xmm1,%%xmm5	\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t"\
		"subpd	%%xmm7,%%xmm2	\n\t"\
		"subpd	%%xmm6,%%xmm3	\n\t"\
		"movaps	%%xmm2,    (%%rcx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rdx)	\n\t"\
		"addpd	%%xmm7,%%xmm7	\n\t"\
		"addpd	%%xmm6,%%xmm6	\n\t"\
		"addpd	%%xmm2,%%xmm7	\n\t"\
		"addpd	%%xmm3,%%xmm6	\n\t"\
		"movaps	%%xmm7,    (%%rdx)	\n\t"\
		"movaps	%%xmm6,0x10(%%rcx)	\n\t"\
		/* SSE2_RADIX4_DIF_IN_PLACE(r3 , r19, r11, r27): */\
		"subq	$%c[__i1],%%rax	\n\t"/* All addresses -= 1*ostride */\
		"subq	$%c[__i1],%%rbx	\n\t"\
		"subq	$%c[__i1],%%rcx	\n\t"\
		"subq	$%c[__i1],%%rdx	\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	    (%%rax),%%xmm2	\n\t"\
		"movaps	0x10(%%rax),%%xmm3	\n\t"\
		"addpd	    (%%rbx),%%xmm0	\n\t"\
		"addpd	0x10(%%rbx),%%xmm1	\n\t"\
		"subpd	    (%%rbx),%%xmm2	\n\t"\
		"subpd	0x10(%%rbx),%%xmm3	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rcx),%%xmm6	\n\t"\
		"movaps	0x10(%%rcx),%%xmm7	\n\t"\
		"addpd	    (%%rdx),%%xmm4	\n\t"\
		"addpd	0x10(%%rdx),%%xmm5	\n\t"\
		"subpd	    (%%rdx),%%xmm6	\n\t"\
		"subpd	0x10(%%rdx),%%xmm7	\n\t"\
		"subpd	%%xmm4,%%xmm0	\n\t"\
		"subpd	%%xmm5,%%xmm1	\n\t"\
		"movaps	%%xmm0,    (%%rbx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rbx)	\n\t"\
		"addpd	%%xmm4,%%xmm4	\n\t"\
		"addpd	%%xmm5,%%xmm5	\n\t"\
		"addpd	%%xmm0,%%xmm4	\n\t"\
		"addpd	%%xmm1,%%xmm5	\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t"\
		"subpd	%%xmm7,%%xmm2	\n\t"\
		"subpd	%%xmm6,%%xmm3	\n\t"\
		"movaps	%%xmm2,    (%%rcx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rdx)	\n\t"\
		"addpd	%%xmm7,%%xmm7	\n\t"\
		"addpd	%%xmm6,%%xmm6	\n\t"\
		"addpd	%%xmm2,%%xmm7	\n\t"\
		"addpd	%%xmm3,%%xmm6	\n\t"\
		"movaps	%%xmm7,    (%%rdx)	\n\t"\
		"movaps	%%xmm6,0x10(%%rcx)	\n\t"\
		/* SSE2_RADIX4_DIF_IN_PLACE(r7 , r23, r15, r31): */\
		"addq	$%c[__i2],%%rax	\n\t"/* All addresses += 2*ostride */\
		"addq	$%c[__i2],%%rbx	\n\t"\
		"addq	$%c[__i2],%%rcx	\n\t"\
		"addq	$%c[__i2],%%rdx	\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	    (%%rax),%%xmm2	\n\t"\
		"movaps	0x10(%%rax),%%xmm3	\n\t"\
		"addpd	    (%%rbx),%%xmm0	\n\t"\
		"addpd	0x10(%%rbx),%%xmm1	\n\t"\
		"subpd	    (%%rbx),%%xmm2	\n\t"\
		"subpd	0x10(%%rbx),%%xmm3	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rcx),%%xmm6	\n\t"\
		"movaps	0x10(%%rcx),%%xmm7	\n\t"\
		"addpd	    (%%rdx),%%xmm4	\n\t"\
		"addpd	0x10(%%rdx),%%xmm5	\n\t"\
		"subpd	    (%%rdx),%%xmm6	\n\t"\
		"subpd	0x10(%%rdx),%%xmm7	\n\t"\
		"subpd	%%xmm4,%%xmm0	\n\t"\
		"subpd	%%xmm5,%%xmm1	\n\t"\
		"movaps	%%xmm0,    (%%rbx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rbx)	\n\t"\
		"addpd	%%xmm4,%%xmm4	\n\t"\
		"addpd	%%xmm5,%%xmm5	\n\t"\
		"addpd	%%xmm0,%%xmm4	\n\t"\
		"addpd	%%xmm1,%%xmm5	\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t"\
		"subpd	%%xmm7,%%xmm2	\n\t"\
		"subpd	%%xmm6,%%xmm3	\n\t"\
		"movaps	%%xmm2,    (%%rcx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rdx)	\n\t"\
		"addpd	%%xmm7,%%xmm7	\n\t"\
		"addpd	%%xmm6,%%xmm6	\n\t"\
		"addpd	%%xmm2,%%xmm7	\n\t"\
		"addpd	%%xmm3,%%xmm6	\n\t"\
		"movaps	%%xmm7,    (%%rdx)	\n\t"\
		"movaps	%%xmm6,0x10(%%rcx)	\n\t"\
	/*** Now do 4 DFTs with internal twiddles on the 1*stride - separated data. Do blocks in order 0,2,1,3 to allow increment-only of rsi-datum from 1 block to the next: ***/\
		"movq	%[__isrt2],%%rdi	\n\t"\
		"movaps	(%%rdi),%%xmm10	\n\t"/* isrt2 */\
		/* Block 0: r0-3 */\
		"movq	%[__out0],%%rsi	\n\t"\
		"movq	%[__in0],%%rax		\n\t"\
		"leaq	%c[__i1](%%rax),%%rcx	\n\t"/* __in0 +   istride; note BR of [a,b,c,d]-ptrs, i.e. b/c swap */\
		"leaq	%c[__i1](%%rcx),%%rbx	\n\t"/* __in0 + 2*istride */\
		"leaq	%c[__i1](%%rbx),%%rdx	\n\t"/* __in0 + 3*istride */\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps		(%%rbx),%%xmm2	\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t"\
		"subpd		(%%rbx),%%xmm0	\n\t"\
		"subpd	0x10(%%rbx),%%xmm1	\n\t"\
		"addpd	    (%%rax),%%xmm2	\n\t"\
		"addpd	0x10(%%rax),%%xmm3	\n\t"\
		"movaps		(%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rdx),%%xmm6	\n\t"\
		"movaps	0x10(%%rdx),%%xmm7	\n\t"\
		"subpd		(%%rdx),%%xmm4	\n\t"\
		"subpd	0x10(%%rdx),%%xmm5	\n\t"\
		"addpd	    (%%rcx),%%xmm6	\n\t"\
		"addpd	0x10(%%rcx),%%xmm7	\n\t"\
		"subpd	%%xmm6,%%xmm2	\n\t"\
		"subpd	%%xmm7,%%xmm3	\n\t"\
		"addpd	%%xmm6,%%xmm6	\n\t"\
		"addpd	%%xmm7,%%xmm7	\n\t"\
		"movaps	%%xmm2,0x20(%%rsi)	\n\t"\
		"movaps	%%xmm3,0x30(%%rsi)	\n\t"\
		"addpd	%%xmm2,%%xmm6	\n\t"\
		"addpd	%%xmm3,%%xmm7	\n\t"\
		"movaps	%%xmm6,    (%%rsi)	\n\t"\
		"movaps	%%xmm7,0x10(%%rsi)	\n\t"\
		"subpd	%%xmm5,%%xmm0	\n\t"\
		"subpd	%%xmm4,%%xmm1	\n\t"\
		"addpd	%%xmm5,%%xmm5	\n\t"\
		"addpd	%%xmm4,%%xmm4	\n\t"\
		"movaps	%%xmm0,0x40(%%rsi)	\n\t"\
		"movaps	%%xmm1,0x70(%%rsi)	\n\t"\
		"addpd	%%xmm0,%%xmm5	\n\t"\
		"addpd	%%xmm1,%%xmm4	\n\t"\
		"movaps	%%xmm5,0x60(%%rsi)	\n\t"\
		"movaps	%%xmm4,0x50(%%rsi)	\n\t"\
		/* Block 2: */\
		"addq	$0x100,%%rsi	\n\t"\
		"addq	$%c[__i4],%%rax	\n\t"/* All addresses += 4*ostride */\
		"addq	$%c[__i4],%%rbx	\n\t"\
		"addq	$%c[__i4],%%rcx	\n\t"\
		"addq	$%c[__i4],%%rdx	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps		(%%rdx),%%xmm6	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	0x10(%%rdx),%%xmm7	\n\t"\
		"movaps	%%xmm4,%%xmm0		\n\t"\
		"movaps	%%xmm6,%%xmm2		\n\t"\
		"movaps	%%xmm5,%%xmm1		\n\t"\
		"movaps	%%xmm7,%%xmm3		\n\t"\
		"movaps	0x10(%%rdi),%%xmm8	\n\t"/* cc0 */\
		"movaps	0x20(%%rdi),%%xmm9	\n\t"/* ss0 */\
		"mulpd	%%xmm8,%%xmm4	\n\t"\
		"mulpd	%%xmm9,%%xmm6	\n\t"\
		"mulpd	%%xmm9,%%xmm1	\n\t"\
		"mulpd	%%xmm8,%%xmm3	\n\t"\
		"mulpd	%%xmm8,%%xmm5	\n\t"\
		"mulpd	%%xmm9,%%xmm7	\n\t"\
		"mulpd	%%xmm9,%%xmm0	\n\t"\
		"mulpd	%%xmm8,%%xmm2	\n\t"\
		"subpd	%%xmm1,%%xmm4	\n\t"\
		"subpd	%%xmm3,%%xmm6	\n\t"\
		"addpd	%%xmm0,%%xmm5	\n\t"\
		"addpd	%%xmm2,%%xmm7	\n\t"\
		"subpd	%%xmm6,%%xmm4	\n\t"\
		"subpd	%%xmm7,%%xmm5	\n\t"\
		"addpd	%%xmm6,%%xmm6	\n\t"\
		"addpd	%%xmm7,%%xmm7	\n\t"\
		"addpd	%%xmm4,%%xmm6	\n\t"\
		"addpd	%%xmm5,%%xmm7	\n\t"\
		"movaps	    (%%rbx),%%xmm2	\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t"\
		"subpd	0x10(%%rbx),%%xmm2	\n\t"\
		"addpd		(%%rbx),%%xmm3	\n\t"\
		"mulpd	%%xmm10,%%xmm2	\n\t"/* mul by isrt2 */\
		"mulpd	%%xmm10,%%xmm3	\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"subpd	%%xmm2,%%xmm0	\n\t"\
		"subpd	%%xmm3,%%xmm1	\n\t"\
		"addpd	    (%%rax),%%xmm2	\n\t"\
		"addpd	0x10(%%rax),%%xmm3	\n\t"\
		"subpd	%%xmm6,%%xmm2	\n\t"\
		"subpd	%%xmm7,%%xmm3	\n\t"\
		"addpd	%%xmm6,%%xmm6	\n\t"\
		"addpd	%%xmm7,%%xmm7	\n\t"\
		"movaps	%%xmm2,0x20(%%rsi)	\n\t"\
		"movaps	%%xmm3,0x30(%%rsi)	\n\t"\
		"addpd	%%xmm2,%%xmm6	\n\t"\
		"addpd	%%xmm3,%%xmm7	\n\t"\
		"movaps	%%xmm6,    (%%rsi)	\n\t"\
		"movaps	%%xmm7,0x10(%%rsi)	\n\t"\
		"subpd	%%xmm5,%%xmm0	\n\t"\
		"subpd	%%xmm4,%%xmm1	\n\t"\
		"addpd	%%xmm5,%%xmm5	\n\t"\
		"addpd	%%xmm4,%%xmm4	\n\t"\
		"movaps	%%xmm0,0x40(%%rsi)	\n\t"\
		"movaps	%%xmm1,0x70(%%rsi)	\n\t"\
		"addpd	%%xmm0,%%xmm5	\n\t"\
		"addpd	%%xmm1,%%xmm4	\n\t"\
		"movaps	%%xmm5,0x60(%%rsi)	\n\t"\
		"movaps	%%xmm4,0x50(%%rsi)	\n\t"\
		/* Block 1: r8-b */\
		"subq	$0x80,%%rsi	\n\t"\
		"addq	$%c[__i4],%%rax	\n\t"/* All addresses += 4*ostride */\
		"addq	$%c[__i4],%%rbx	\n\t"\
		"addq	$%c[__i4],%%rcx	\n\t"\
		"addq	$%c[__i4],%%rdx	\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps		(%%rbx),%%xmm2	\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t"\
		"subpd	0x10(%%rbx),%%xmm0	\n\t"\
		"subpd		(%%rbx),%%xmm1	\n\t"\
		"addpd	0x10(%%rax),%%xmm2	\n\t"\
		"addpd		(%%rax),%%xmm3	\n\t"\
		"movaps		(%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rdx),%%xmm6	\n\t"\
		"movaps	0x10(%%rdx),%%xmm7	\n\t"\
		"subpd	0x10(%%rcx),%%xmm4	\n\t"\
		"addpd		(%%rcx),%%xmm5	\n\t"\
		"addpd	0x10(%%rdx),%%xmm6	\n\t"\
		"subpd		(%%rdx),%%xmm7	\n\t"\
		"mulpd	%%xmm10,%%xmm4	\n\t"\
		"mulpd	%%xmm10,%%xmm5	\n\t"\
		"mulpd	%%xmm10,%%xmm6	\n\t"\
		"mulpd	%%xmm10,%%xmm7	\n\t"\
		"subpd	%%xmm6,%%xmm4	\n\t"\
		"subpd	%%xmm7,%%xmm5	\n\t"\
		"addpd	%%xmm6,%%xmm6	\n\t"\
		"addpd	%%xmm7,%%xmm7	\n\t"\
		"addpd	%%xmm4,%%xmm6	\n\t"\
		"addpd	%%xmm5,%%xmm7	\n\t"\
		"subpd	%%xmm4,%%xmm0	\n\t"\
		"subpd	%%xmm5,%%xmm2	\n\t"\
		"addpd	%%xmm4,%%xmm4	\n\t"\
		"addpd	%%xmm5,%%xmm5	\n\t"\
		"movaps	%%xmm0,0x20(%%rsi)	\n\t"\
		"movaps	%%xmm2,0x30(%%rsi)	\n\t"\
		"addpd	%%xmm0,%%xmm4	\n\t"\
		"addpd	%%xmm2,%%xmm5	\n\t"\
		"movaps	%%xmm4,    (%%rsi)	\n\t"\
		"movaps	%%xmm5,0x10(%%rsi)	\n\t"\
		"subpd	%%xmm7,%%xmm3	\n\t"\
		"subpd	%%xmm6,%%xmm1	\n\t"\
		"addpd	%%xmm7,%%xmm7	\n\t"\
		"addpd	%%xmm6,%%xmm6	\n\t"\
		"movaps	%%xmm3,0x40(%%rsi)	\n\t"\
		"movaps	%%xmm1,0x70(%%rsi)	\n\t"\
		"addpd	%%xmm3,%%xmm7	\n\t"\
		"addpd	%%xmm1,%%xmm6	\n\t"\
		"movaps	%%xmm7,0x60(%%rsi)	\n\t"\
		"movaps	%%xmm6,0x50(%%rsi)	\n\t"\
		/* Block 3: */\
		"addq	$0x100,%%rsi	\n\t"\
		"addq	$%c[__i4],%%rax	\n\t"/* All addresses += 4*ostride */\
		"addq	$%c[__i4],%%rbx	\n\t"\
		"addq	$%c[__i4],%%rcx	\n\t"\
		"addq	$%c[__i4],%%rdx	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps		(%%rdx),%%xmm6	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	0x10(%%rdx),%%xmm7	\n\t"\
		"movaps	%%xmm4,%%xmm0		\n\t"\
		"movaps	%%xmm6,%%xmm2		\n\t"\
		"movaps	%%xmm5,%%xmm1		\n\t"\
		"movaps	%%xmm7,%%xmm3		\n\t"\
		"mulpd	%%xmm9,%%xmm4	\n\t"\
		"mulpd	%%xmm8,%%xmm6	\n\t"\
		"mulpd	%%xmm8,%%xmm1	\n\t"\
		"mulpd	%%xmm9,%%xmm3	\n\t"\
		"mulpd	%%xmm9,%%xmm5	\n\t"\
		"mulpd	%%xmm8,%%xmm7	\n\t"\
		"mulpd	%%xmm8,%%xmm0	\n\t"\
		"mulpd	%%xmm9,%%xmm2	\n\t"\
		"subpd	%%xmm1,%%xmm4	\n\t"\
		"subpd	%%xmm3,%%xmm6	\n\t"\
		"addpd	%%xmm0,%%xmm5	\n\t"\
		"addpd	%%xmm2,%%xmm7	\n\t"\
		"subpd	%%xmm6,%%xmm4	\n\t"\
		"subpd	%%xmm7,%%xmm5	\n\t"\
		"addpd	%%xmm6,%%xmm6	\n\t"\
		"addpd	%%xmm7,%%xmm7	\n\t"\
		"addpd	%%xmm4,%%xmm6	\n\t"\
		"addpd	%%xmm5,%%xmm7	\n\t"\
		"movaps	    (%%rbx),%%xmm2	\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t"\
		"addpd	0x10(%%rbx),%%xmm2	\n\t"\
		"subpd	    (%%rbx),%%xmm3	\n\t"\
		"mulpd	%%xmm10,%%xmm2	\n\t"/* mul by isrt2 */\
		"mulpd	%%xmm10,%%xmm3	\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"subpd	%%xmm2,%%xmm0	\n\t"\
		"subpd	%%xmm3,%%xmm1	\n\t"\
		"addpd	    (%%rax),%%xmm2	\n\t"\
		"addpd	0x10(%%rax),%%xmm3	\n\t"\
		"subpd	%%xmm4,%%xmm0	\n\t"\
		"subpd	%%xmm5,%%xmm1	\n\t"\
		"addpd	%%xmm4,%%xmm4	\n\t"\
		"addpd	%%xmm5,%%xmm5	\n\t"\
		"movaps	%%xmm0,0x20(%%rsi)	\n\t"\
		"movaps	%%xmm1,0x30(%%rsi)	\n\t"\
		"addpd	%%xmm0,%%xmm4	\n\t"\
		"addpd	%%xmm1,%%xmm5	\n\t"\
		"movaps	%%xmm4,    (%%rsi)	\n\t"\
		"movaps	%%xmm5,0x10(%%rsi)	\n\t"\
		"subpd	%%xmm7,%%xmm2	\n\t"\
		"subpd	%%xmm6,%%xmm3	\n\t"\
		"addpd	%%xmm7,%%xmm7	\n\t"\
		"addpd	%%xmm6,%%xmm6	\n\t"\
		"movaps	%%xmm2,0x40(%%rsi)	\n\t"\
		"movaps	%%xmm3,0x70(%%rsi)	\n\t"\
		"addpd	%%xmm2,%%xmm7	\n\t"\
		"addpd	%%xmm3,%%xmm6	\n\t"\
		"movaps	%%xmm7,0x60(%%rsi)	\n\t"\
		"movaps	%%xmm6,0x50(%%rsi)	\n\t"\
		:					/* outputs: none */\
		:[__in0] "m" (Xin0)	/* All inputs from memory addresses here */\
		,[__i1] "e" (Xi1)\
		,[__i2] "e" (Xi2)\
		,[__i3] "e" (Xi3)\
		,[__i4] "e" (Xi4)\
		,[__isrt2] "m" (Xisrt2)\
		,[__two] "m" (Xtwo)\
		,[__out0] "m" (Xout0)\
		: "cc","memory","rax","rbx","rcx","rdx","rdi","rsi","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10"		/* Clobbered registers */\
	);\
	}

  #endif	// 8/16-reg versions of SSE2_RADIX16_DIF_0TWIDDLE_B

	/* With-twiddles out-of-place analog of above twiddleless DIT macro: 15 nontrivial complex input twiddles E1-f [E0 assumed = 1],
	The DIT version of this macro processes the twiddles in-order.
	NOTE: SINCE THIS MACRO IS SPECIFICALLY DESIGNED AS THE 2ND-PASS OF LARGE-POWER-OF-2-TWIDDLELESS DFT SYNTHESIS, THE
	"TWIDDLES" HERE ARE PURE OF THE DFT-INTERNAL VARIETY, AND THUS APPLIED TO THE INPUTS, JUST AS FOR THE ABOVE DIF COUNTERPART.

	Sincos layout: Two portions:

	Radix-16 shared consts anchored at isrt2:

	  isrt2 + 0x000;	cc0 + 0x010;	ss0 + 0x020;

	Per-block-specific set of 15 complex twiddles anchored at c1:

		c1  + 0x000;	s1  + 0x010;
		c2  + 0x020;	s2  + 0x030;
		c3  + 0x040;	s3  + 0x050;
		c4  + 0x060;	s4  + 0x070;
		c5  + 0x080;	s5  + 0x090;
		c6  + 0x0a0;	s6  + 0x0b0;
		c7  + 0x0c0;	s7  + 0x0d0;
		c8  + 0x0e0;	s8  + 0x0f0;
		c9  + 0x100;	s9  + 0x110;
		c10 + 0x120;	s10 + 0x130;
		c11 + 0x140;	s11 + 0x150;
		c12 + 0x160;	s12 + 0x170;
		c13 + 0x180;	s13 + 0x190;
		c14 + 0x1a0;	s14 + 0x1b0;
		c15 + 0x1c0;	s15 + 0x1d0;

	Use radix-16 DIF as template for DIT/OOP here, since need a pre-twiddles algorithm:
	*/
	#define SSE2_RADIX16_DIT_TWIDDLE_OOP(Xin0,Xi1,Xi2,Xi3,Xi4, Xout0,Xo1,Xo2,Xo3,Xo4, Xisrt2,Xc1)\
	{\
	__asm__ volatile (\
	/*...Block 0: Do in-place, i.e. outputs into __in0 + [0,1,2,3]*istride: */\
		"movq	%[__in0],%%rax		\n\t"\
		"leaq	%c[__i1](%%rax),%%rcx	\n\t"/* __in0 +   istride */\
		"leaq	%c[__i2](%%rax),%%rbx	\n\t"/* __in0 + 2*istride */\
		"leaq	%c[__i3](%%rax),%%rdx	\n\t"/* __in0 + 3*istride */\
		"/* Do	the p0,1 combo: */	\n\t"\
		"movq	%[__c1],%%rsi 	/* c1 */\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"mulpd	    (%%rsi),%%xmm4	\n\t"\
		"mulpd	    (%%rsi),%%xmm5	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm6	\n\t"\
		"movaps	%%xmm0,%%xmm2		\n\t"\
		"mulpd	0x10(%%rsi),%%xmm7	\n\t"\
		"subpd	%%xmm6,%%xmm5		\n\t"\
		"movaps	%%xmm1,%%xmm3		\n\t"\
		"addpd	%%xmm7,%%xmm4		\n\t"\
		"addpd	%%xmm4,%%xmm0		\n\t"\
		"addpd	%%xmm5,%%xmm1		\n\t"\
		"subpd	%%xmm4,%%xmm2		\n\t"\
		"subpd	%%xmm5,%%xmm3		\n\t"\
		"/* Do	the p2,3 combo: */	\n\t"\
		"addq	$0x20,%%rsi 	/* c2,3 */\n\t"\
		"movaps	    (%%rdx),%%xmm4	\n\t"\
		"movaps	0x10(%%rdx),%%xmm5	\n\t"\
		"movaps	    (%%rdx),%%xmm6	\n\t"\
		"movaps	0x10(%%rdx),%%xmm7	\n\t"\
		"mulpd	0x20(%%rsi),%%xmm4	/* c3 */\n\t"\
		"mulpd	0x20(%%rsi),%%xmm5	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm6	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm7	\n\t"\
		"subpd	%%xmm6,%%xmm5		\n\t"\
		"addpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t"\
		"movaps	    (%%rbx),%%xmm4	\n\t"\
		"movaps	0x10(%%rbx),%%xmm5	\n\t"\
		"movaps	    (%%rbx),%%xmm6	\n\t"\
		"movaps	0x10(%%rbx),%%xmm7	\n\t"\
		"mulpd	    (%%rsi),%%xmm4	/* c2 */\n\t"\
		"mulpd	    (%%rsi),%%xmm5	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm6	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm7	\n\t"\
		"subpd	%%xmm6,%%xmm5		\n\t"\
		"addpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"subpd	    (%%rax),%%xmm4	\n\t"\
		"subpd	0x10(%%rax),%%xmm5	\n\t"\
		"addpd	    (%%rax),%%xmm6	\n\t"\
		"addpd	0x10(%%rax),%%xmm7	\n\t"\
		"/* Finish radix-4 butterfly and store results into temporary-array slots: */\n\t"\
		"subpd	%%xmm6,%%xmm0		\n\t"\
		"subpd	%%xmm5,%%xmm2		\n\t"\
		"subpd	%%xmm7,%%xmm1		\n\t"\
		"subpd	%%xmm4,%%xmm3		\n\t"\
		/* DIT has outputs (indexed in real-temp form as 0-7) 2/6,3/7 swapped, i.e. swap oregs c/d vs DIF: */\
		"movaps	%%xmm0,    (%%rbx)	\n\t"\
		"movaps	%%xmm2,    (%%rdx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rbx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rcx)	\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm0,%%xmm6		\n\t"\
		"addpd	%%xmm2,%%xmm5		\n\t"\
		"addpd	%%xmm1,%%xmm7		\n\t"\
		"addpd	%%xmm3,%%xmm4		\n\t"\
		"movaps	%%xmm6,    (%%rax)	\n\t"\
		"movaps	%%xmm5,    (%%rcx)	\n\t"\
		"movaps	%%xmm7,0x10(%%rax)	\n\t"\
		"movaps	%%xmm4,0x10(%%rdx)	\n\t"\
		"\n\t"\
	/*...Block 1: outputs into __in0 + [4,5,6,7]*istride: */\
		"addq	$%c[__i4],%%rax	\n\t"/* __in0 + 4*istride */\
		"addq	$%c[__i4],%%rcx	\n\t"/* __in0 + 5*istride */\
		"addq	$%c[__i4],%%rbx	\n\t"/* __in0 + 6*istride */\
		"addq	$%c[__i4],%%rdx	\n\t"/* __in0 + 7*istride */\
		"/* Do	the p0,1 combo: */	\n\t"\
		"addq	$0x40,%%rsi 	/* c4,5 */\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rsi),%%xmm6	\n\t"\
		"movaps	0x10(%%rsi),%%xmm7	\n\t"\
		"movaps	%%xmm0,%%xmm2		\n\t"\
		"movaps	%%xmm1,%%xmm3		\n\t"\
		"mulpd	%%xmm6,%%xmm0		/* c4 */\n\t"\
		"mulpd	%%xmm6,%%xmm1		\n\t"\
		"mulpd	%%xmm7,%%xmm2		\n\t"\
		"mulpd	%%xmm7,%%xmm3		\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"subpd	%%xmm2,%%xmm1		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"mulpd	0x20(%%rsi),%%xmm4	/* c5 */\n\t"\
		"addpd	%%xmm3,%%xmm0		\n\t"\
		"mulpd	0x20(%%rsi),%%xmm5	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm6	\n\t"\
		"movaps	%%xmm0,%%xmm2		\n\t"\
		"mulpd	0x30(%%rsi),%%xmm7	\n\t"\
		"subpd	%%xmm6,%%xmm5		\n\t"\
		"movaps	%%xmm1,%%xmm3		\n\t"\
		"addpd	%%xmm7,%%xmm4		\n\t"\
		"addpd	%%xmm4,%%xmm0		\n\t"\
		"addpd	%%xmm5,%%xmm1		\n\t"\
		"subpd	%%xmm4,%%xmm2		\n\t"\
		"subpd	%%xmm5,%%xmm3		\n\t"\
		"/* Do	the p2,3 combo: */	\n\t"\
		"addq	$0x40,%%rsi 	/* c6,7 */\n\t"\
		"movaps	    (%%rdx),%%xmm4	\n\t"\
		"movaps	0x10(%%rdx),%%xmm5	\n\t"\
		"movaps	    (%%rdx),%%xmm6	\n\t"\
		"movaps	0x10(%%rdx),%%xmm7	\n\t"\
		"mulpd	0x20(%%rsi),%%xmm4	/* c7 */\n\t"\
		"mulpd	0x20(%%rsi),%%xmm5	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm6	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm7	\n\t"\
		"subpd	%%xmm6,%%xmm5		\n\t"\
		"addpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t"\
		"movaps	    (%%rbx),%%xmm4	\n\t"\
		"movaps	0x10(%%rbx),%%xmm5	\n\t"\
		"movaps	    (%%rbx),%%xmm6	\n\t"\
		"movaps	0x10(%%rbx),%%xmm7	\n\t"\
		"mulpd	    (%%rsi),%%xmm4	/* c6 */\n\t"\
		"mulpd	    (%%rsi),%%xmm5	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm6	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm7	\n\t"\
		"subpd	%%xmm6,%%xmm5		\n\t"\
		"addpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"subpd	    (%%rax),%%xmm4	\n\t"\
		"subpd	0x10(%%rax),%%xmm5	\n\t"\
		"addpd	    (%%rax),%%xmm6	\n\t"\
		"addpd	0x10(%%rax),%%xmm7	\n\t"\
		"/* Finish radix-4 butterfly and store results into temporary-array slots: */\n\t"\
		"subpd	%%xmm6,%%xmm0		\n\t"\
		"subpd	%%xmm5,%%xmm2		\n\t"\
		"subpd	%%xmm7,%%xmm1		\n\t"\
		"subpd	%%xmm4,%%xmm3		\n\t"\
		"movaps	%%xmm0,    (%%rbx)	\n\t"\
		"movaps	%%xmm2,    (%%rdx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rbx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rcx)	\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm0,%%xmm6		\n\t"\
		"addpd	%%xmm2,%%xmm5		\n\t"\
		"addpd	%%xmm1,%%xmm7		\n\t"\
		"addpd	%%xmm3,%%xmm4		\n\t"\
		"movaps	%%xmm6,    (%%rax)	\n\t"\
		"movaps	%%xmm5,    (%%rcx)	\n\t"\
		"movaps	%%xmm7,0x10(%%rax)	\n\t"\
		"movaps	%%xmm4,0x10(%%rdx)	\n\t"\
		"\n\t"\
	/*...Block 2: outputs into __in0 + [8,9,a,b]*istride: */\
		"addq	$%c[__i4],%%rax	\n\t"/* __in0 + 8*istride */\
		"addq	$%c[__i4],%%rcx	\n\t"/* __in0 + 9*istride */\
		"addq	$%c[__i4],%%rbx	\n\t"/* __in0 + a*istride */\
		"addq	$%c[__i4],%%rdx	\n\t"/* __in0 + b*istride */\
		"/* Do	the p0,1 combo: */	\n\t"\
		"addq	$0x40,%%rsi 	/* c8,9 */\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rsi),%%xmm6	\n\t"\
		"movaps	0x10(%%rsi),%%xmm7	\n\t"\
		"movaps	%%xmm0,%%xmm2		\n\t"\
		"movaps	%%xmm1,%%xmm3		\n\t"\
		"mulpd	%%xmm6,%%xmm0		/* c8 */\n\t"\
		"mulpd	%%xmm6,%%xmm1		\n\t"\
		"mulpd	%%xmm7,%%xmm2		\n\t"\
		"mulpd	%%xmm7,%%xmm3		\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"subpd	%%xmm2,%%xmm1		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"mulpd	0x20(%%rsi),%%xmm4	/* c9 */\n\t"\
		"addpd	%%xmm3,%%xmm0		\n\t"\
		"mulpd	0x20(%%rsi),%%xmm5	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm6	\n\t"\
		"movaps	%%xmm0,%%xmm2		\n\t"\
		"mulpd	0x30(%%rsi),%%xmm7	\n\t"\
		"subpd	%%xmm6,%%xmm5		\n\t"\
		"movaps	%%xmm1,%%xmm3		\n\t"\
		"addpd	%%xmm7,%%xmm4		\n\t"\
		"addpd	%%xmm4,%%xmm0		\n\t"\
		"addpd	%%xmm5,%%xmm1		\n\t"\
		"subpd	%%xmm4,%%xmm2		\n\t"\
		"subpd	%%xmm5,%%xmm3		\n\t"\
		"/* Do	the p2,3 combo: */	\n\t"\
		"addq	$0x40,%%rsi 	/* ca,b */\n\t"\
		"movaps	    (%%rdx),%%xmm4	\n\t"\
		"movaps	0x10(%%rdx),%%xmm5	\n\t"\
		"movaps	    (%%rdx),%%xmm6	\n\t"\
		"movaps	0x10(%%rdx),%%xmm7	\n\t"\
		"mulpd	0x20(%%rsi),%%xmm4	/* cb */\n\t"\
		"mulpd	0x20(%%rsi),%%xmm5	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm6	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm7	\n\t"\
		"subpd	%%xmm6,%%xmm5		\n\t"\
		"addpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t"\
		"movaps	    (%%rbx),%%xmm4	\n\t"\
		"movaps	0x10(%%rbx),%%xmm5	\n\t"\
		"movaps	    (%%rbx),%%xmm6	\n\t"\
		"movaps	0x10(%%rbx),%%xmm7	\n\t"\
		"mulpd	    (%%rsi),%%xmm4	/* ca */\n\t"\
		"mulpd	    (%%rsi),%%xmm5	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm6	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm7	\n\t"\
		"subpd	%%xmm6,%%xmm5		\n\t"\
		"addpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"subpd	    (%%rax),%%xmm4	\n\t"\
		"subpd	0x10(%%rax),%%xmm5	\n\t"\
		"addpd	    (%%rax),%%xmm6	\n\t"\
		"addpd	0x10(%%rax),%%xmm7	\n\t"\
		"/* Finish radix-4 butterfly and store results into temporary-array slots: */\n\t"\
		"subpd	%%xmm6,%%xmm0		\n\t"\
		"subpd	%%xmm5,%%xmm2		\n\t"\
		"subpd	%%xmm7,%%xmm1		\n\t"\
		"subpd	%%xmm4,%%xmm3		\n\t"\
		"movaps	%%xmm0,    (%%rbx)	\n\t"\
		"movaps	%%xmm2,    (%%rdx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rbx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rcx)	\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm0,%%xmm6		\n\t"\
		"addpd	%%xmm2,%%xmm5		\n\t"\
		"addpd	%%xmm1,%%xmm7		\n\t"\
		"addpd	%%xmm3,%%xmm4		\n\t"\
		"movaps	%%xmm6,    (%%rax)	\n\t"\
		"movaps	%%xmm5,    (%%rcx)	\n\t"\
		"movaps	%%xmm7,0x10(%%rax)	\n\t"\
		"movaps	%%xmm4,0x10(%%rdx)	\n\t"\
		"\n\t"\
	/*...Block 3: outputs into __in0 + [c,d,e,f]*istride: */\
		"addq	$%c[__i4],%%rax	\n\t"/* __in0 + c*istride */\
		"addq	$%c[__i4],%%rcx	\n\t"/* __in0 + d*istride */\
		"addq	$%c[__i4],%%rbx	\n\t"/* __in0 + e*istride */\
		"addq	$%c[__i4],%%rdx	\n\t"/* __in0 + f*istride */\
		"/* Do	the p0,1 combo: */	\n\t"\
		"addq	$0x40,%%rsi 	/* cc,d */\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rsi),%%xmm6	\n\t"\
		"movaps	0x10(%%rsi),%%xmm7	\n\t"\
		"movaps	%%xmm0,%%xmm2		\n\t"\
		"movaps	%%xmm1,%%xmm3		\n\t"\
		"mulpd	%%xmm6,%%xmm0		/* cc */\n\t"\
		"mulpd	%%xmm6,%%xmm1		\n\t"\
		"mulpd	%%xmm7,%%xmm2		\n\t"\
		"mulpd	%%xmm7,%%xmm3		\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"subpd	%%xmm2,%%xmm1		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"mulpd	0x20(%%rsi),%%xmm4	/* cd */\n\t"\
		"addpd	%%xmm3,%%xmm0		\n\t"\
		"mulpd	0x20(%%rsi),%%xmm5	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm6	\n\t"\
		"movaps	%%xmm0,%%xmm2		\n\t"\
		"mulpd	0x30(%%rsi),%%xmm7	\n\t"\
		"subpd	%%xmm6,%%xmm5		\n\t"\
		"movaps	%%xmm1,%%xmm3		\n\t"\
		"addpd	%%xmm7,%%xmm4		\n\t"\
		"addpd	%%xmm4,%%xmm0		\n\t"\
		"addpd	%%xmm5,%%xmm1		\n\t"\
		"subpd	%%xmm4,%%xmm2		\n\t"\
		"subpd	%%xmm5,%%xmm3		\n\t"\
		"/* Do	the p2,3 combo: */	\n\t"\
		"addq	$0x40,%%rsi 	/* ce,f */\n\t"\
		"movaps	    (%%rdx),%%xmm4	\n\t"\
		"movaps	0x10(%%rdx),%%xmm5	\n\t"\
		"movaps	    (%%rdx),%%xmm6	\n\t"\
		"movaps	0x10(%%rdx),%%xmm7	\n\t"\
		"mulpd	0x20(%%rsi),%%xmm4	/* cf */\n\t"\
		"mulpd	0x20(%%rsi),%%xmm5	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm6	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm7	\n\t"\
		"subpd	%%xmm6,%%xmm5		\n\t"\
		"addpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t"\
		"movaps	    (%%rbx),%%xmm4	\n\t"\
		"movaps	0x10(%%rbx),%%xmm5	\n\t"\
		"movaps	    (%%rbx),%%xmm6	\n\t"\
		"movaps	0x10(%%rbx),%%xmm7	\n\t"\
		"mulpd	    (%%rsi),%%xmm4	/* ce */\n\t"\
		"mulpd	    (%%rsi),%%xmm5	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm6	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm7	\n\t"\
		"subpd	%%xmm6,%%xmm5		\n\t"\
		"addpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"subpd	    (%%rax),%%xmm4	\n\t"\
		"subpd	0x10(%%rax),%%xmm5	\n\t"\
		"addpd	    (%%rax),%%xmm6	\n\t"\
		"addpd	0x10(%%rax),%%xmm7	\n\t"\
		"/* Finish radix-4 butterfly and store results into temporary-array slots: */\n\t"\
		"subpd	%%xmm6,%%xmm0		\n\t"\
		"subpd	%%xmm5,%%xmm2		\n\t"\
		"subpd	%%xmm7,%%xmm1		\n\t"\
		"subpd	%%xmm4,%%xmm3		\n\t"\
		"movaps	%%xmm0,    (%%rbx)	\n\t"\
		"movaps	%%xmm2,    (%%rdx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rbx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rcx)	\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm0,%%xmm6		\n\t"\
		"addpd	%%xmm2,%%xmm5		\n\t"\
		"addpd	%%xmm1,%%xmm7		\n\t"\
		"addpd	%%xmm3,%%xmm4		\n\t"\
		"movaps	%%xmm6,    (%%rax)	\n\t"\
		"movaps	%%xmm5,    (%%rcx)	\n\t"\
		"movaps	%%xmm7,0x10(%%rax)	\n\t"\
		"movaps	%%xmm4,0x10(%%rdx)	\n\t"\
	"/*************************************************************************************/\n\t"\
	"/*  And now do four more radix-4 transforms, including the internal twiddle factors: */\n\t"\
	"/*************************************************************************************/\n\t"\
		"movq	%[__isrt2],%%rsi 	\n\t"\
	/* Block 0: Combine 0-output of each radix-4, i.e. inputs from __in0 + [0,4,8,c]*istride: */\
		"movq	%[__in0],%%rax		\n\t"\
		"leaq	%c[__i4](%%rax),%%rbx	\n\t"/* __in0 +   [4*istride] */\
		"leaq	%c[__i4](%%rbx),%%rcx	\n\t"/* __in0 + 2*[4*istride] */\
		"leaq	%c[__i4](%%rcx),%%rdx	\n\t"/* __in0 + 3*[4*istride] */\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rbx),%%xmm2	\n\t"\
		"movaps	    (%%rdx),%%xmm6	\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t"\
		"movaps	0x10(%%rdx),%%xmm7	\n\t"\
		"subpd	%%xmm2,%%xmm0		\n\t"\
		"subpd	%%xmm6,%%xmm4		\n\t"\
		"subpd	%%xmm3,%%xmm1		\n\t"\
		"subpd	%%xmm7,%%xmm5		\n\t"\
		"addpd	%%xmm2,%%xmm2		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm3,%%xmm3		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm0,%%xmm2		\n\t"\
		"addpd	%%xmm4,%%xmm6		\n\t"\
		"addpd	%%xmm1,%%xmm3		\n\t"\
		"addpd	%%xmm5,%%xmm7		\n\t"\
		"subpd	%%xmm6,%%xmm2		\n\t"\
		"subpd	%%xmm7,%%xmm3		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"movq	%[__out0],%%r10		\n\t"\
		"leaq	%c[__o4](%%r10),%%r11	\n\t"/* __out0 + 4*ostride */\
		"leaq	%c[__o4](%%r11),%%r12	\n\t"/* __out0 + 8*ostride */\
		"leaq	%c[__o4](%%r12),%%r13	\n\t"/* __out0 + c*ostride */\
		"movaps	%%xmm2,    (%%r12)	\n\t"\
		"movaps	%%xmm3,0x10(%%r12)	\n\t"\
		"addpd	%%xmm2,	%%xmm6	\n\t"\
		"addpd	%%xmm3,	%%xmm7	\n\t"\
		"movaps	%%xmm6,    (%%r10)	\n\t"\
		"movaps	%%xmm7,0x10(%%r10)	\n\t"\
		"subpd	%%xmm5,	%%xmm0	\n\t"\
		"subpd	%%xmm4,	%%xmm1	\n\t"\
		"addpd	%%xmm5,	%%xmm5	\n\t"\
		"addpd	%%xmm4,	%%xmm4	\n\t"\
		"movaps	%%xmm0,    (%%r13)	\n\t"/* These 2 outputs [4/c] swapped w.r.to dif [2/3] due to +-I sign diff */\
		"movaps	%%xmm1,0x10(%%r11)	\n\t"\
		"addpd	%%xmm0,	%%xmm5	\n\t"\
		"addpd	%%xmm1,	%%xmm4	\n\t"\
		"movaps	%%xmm5,    (%%r11)	\n\t"\
		"movaps	%%xmm4,0x10(%%r13)	\n\t"\
	/* Block 1: Combine 1-output of each radix-4, i.e. inputs from __in0 + [1,5,9,d]*istride: */\
		"addq	$%c[__i1],%%rax	\n\t"/* __in0 + 1*istride */\
		"addq	$%c[__i1],%%rcx	\n\t"/* __in0 + 5*istride */\
		"addq	$%c[__i1],%%rbx	\n\t"/* __in0 + 9*istride */\
		"addq	$%c[__i1],%%rdx	\n\t"/* __in0 + d*istride */\
		"movaps	    (%%rdx),%%xmm0	\n\t"\
		"movaps	0x10(%%rdx),%%xmm1	\n\t"\
		"movaps	    (%%rdx),%%xmm2	\n\t"\
		"movaps	0x10(%%rdx),%%xmm3	\n\t"\
		"mulpd	0x20(%%rsi),%%xmm0	\n\t"/* ss0 */\
		"mulpd	0x20(%%rsi),%%xmm1	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm2	\n\t"/* cc0 */\
		"mulpd	0x10(%%rsi),%%xmm3	\n\t"\
		"subpd	%%xmm2,%%xmm1		\n\t"\
		"addpd	%%xmm3,%%xmm0		\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rcx),%%xmm6	\n\t"\
		"movaps	0x10(%%rcx),%%xmm7	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm4	\n\t"/* cc0 */\
		"mulpd	0x10(%%rsi),%%xmm5	\n\t"\
		"mulpd	0x20(%%rsi),%%xmm6	\n\t"/* ss0 */\
		"mulpd	0x20(%%rsi),%%xmm7	\n\t"\
		"subpd	%%xmm6,%%xmm5		\n\t"\
		"addpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"addpd	%%xmm0,%%xmm4		\n\t"\
		"addpd	%%xmm1,%%xmm5		\n\t"\
		"subpd	%%xmm0,%%xmm6		\n\t"\
		"subpd	%%xmm1,%%xmm7		\n\t"\
		"movaps	    (%%rbx),%%xmm2	\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"addpd	0x10(%%rbx),%%xmm2	\n\t"\
		"subpd	    (%%rbx),%%xmm3	\n\t"\
		"mulpd	    (%%rsi),%%xmm2	\n\t"/* isrt2 */\
		"mulpd	    (%%rsi),%%xmm3	\n\t"\
		"subpd	%%xmm2,%%xmm0		\n\t"\
		"subpd	%%xmm3,%%xmm1		\n\t"\
		"addpd	%%xmm2,%%xmm2		\n\t"\
		"addpd	%%xmm3,%%xmm3		\n\t"\
		"addpd	%%xmm0,%%xmm2		\n\t"\
		"addpd	%%xmm1,%%xmm3		\n\t"\
		"addq	$%c[__o1],%%r10	\n\t"/* __out0 + 1*ostride */\
		"addq	$%c[__o1],%%r11	\n\t"/* __out0 + 5*ostride */\
		"addq	$%c[__o1],%%r12	\n\t"/* __out0 + 9*ostride */\
		"addq	$%c[__o1],%%r13	\n\t"/* __out0 + d*ostride */\
		"subpd	%%xmm4,%%xmm2		\n\t"\
		"subpd	%%xmm5,%%xmm3		\n\t"\
		"movaps	%%xmm2,    (%%r12)	\n\t"\
		"movaps	%%xmm3,0x10(%%r12)	\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm2,%%xmm4		\n\t"\
		"addpd	%%xmm3,%%xmm5		\n\t"\
		"movaps	%%xmm4,    (%%r10)	\n\t"\
		"movaps	%%xmm5,0x10(%%r10)	\n\t"\
		"subpd	%%xmm7,%%xmm0		\n\t"\
		"subpd	%%xmm6,%%xmm1		\n\t"\
		"movaps	%%xmm0,    (%%r13)	\n\t"\
		"movaps	%%xmm1,0x10(%%r11)	\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm0,%%xmm7		\n\t"\
		"addpd	%%xmm1,%%xmm6		\n\t"\
		"movaps	%%xmm7,    (%%r11)	\n\t"\
		"movaps	%%xmm6,0x10(%%r13)	\n\t"\
	/* Block 2: Combine 2-output of each radix-4, i.e. inputs from __in0 + [2,6,a,e]*istride: */\
		"movaps	(%%rsi),%%xmm2	/* isrt2 */\n\t"\
		"addq	$%c[__i1],%%rax	\n\t"/* __in0 + 2*istride */\
		"addq	$%c[__i1],%%rbx	\n\t"/* __in0 + 6*istride */\
		"addq	$%c[__i1],%%rcx	\n\t"/* __in0 + a*istride */\
		"addq	$%c[__i1],%%rdx	\n\t"/* __in0 + e*istride */\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rdx),%%xmm0	\n\t"\
		"movaps	0x10(%%rdx),%%xmm1	\n\t"\
		"addpd	0x10(%%rcx),%%xmm4	\n\t"\
		"subpd	    (%%rcx),%%xmm5	\n\t"\
		"subpd	0x10(%%rdx),%%xmm0	\n\t"\
		"addpd	    (%%rdx),%%xmm1	\n\t"\
		"mulpd	%%xmm2,%%xmm4		\n\t"\
		"mulpd	%%xmm2,%%xmm5		\n\t"\
		"mulpd	%%xmm2,%%xmm0		\n\t"\
		"mulpd	%%xmm2,%%xmm1		\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"subpd	%%xmm0,%%xmm4		\n\t"\
		"subpd	%%xmm1,%%xmm5		\n\t"\
		"addpd	%%xmm0,%%xmm6		\n\t"\
		"addpd	%%xmm1,%%xmm7		\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	    (%%rbx),%%xmm2	\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t"\
		"subpd	0x10(%%rbx),%%xmm0	\n\t"\
		"subpd	    (%%rbx),%%xmm1	\n\t"\
		"addpd	    (%%rax),%%xmm3	\n\t"\
		"addpd	0x10(%%rax),%%xmm2	\n\t"\
		"addq	$%c[__o1],%%r10	\n\t"/* __out0 + 2*ostride */\
		"addq	$%c[__o1],%%r11	\n\t"/* __out0 + 6*ostride */\
		"addq	$%c[__o1],%%r12	\n\t"/* __out0 + a*ostride */\
		"addq	$%c[__o1],%%r13	\n\t"/* __out0 + e*ostride */\
		"subpd	%%xmm4,%%xmm3		\n\t"\
		"subpd	%%xmm5,%%xmm1		\n\t"\
		"movaps	%%xmm3,    (%%r12)	\n\t"\
		"movaps	%%xmm1,0x10(%%r12)	\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm3,%%xmm4		\n\t"\
		"addpd	%%xmm1,%%xmm5		\n\t"\
		"movaps	%%xmm4,    (%%r10)	\n\t"\
		"movaps	%%xmm5,0x10(%%r10)	\n\t"\
		"subpd	%%xmm7,%%xmm0		\n\t"\
		"subpd	%%xmm6,%%xmm2		\n\t"\
		"movaps	%%xmm0,    (%%r13)	\n\t"\
		"movaps	%%xmm2,0x10(%%r11)	\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm0,%%xmm7		\n\t"\
		"addpd	%%xmm2,%%xmm6		\n\t"\
		"movaps	%%xmm7,    (%%r11)	\n\t"\
		"movaps	%%xmm6,0x10(%%r13)	\n\t"\
	/* Block 3: Combine 3-output of each radix-4, i.e. inputs from __in0 + [3,7,b,f]*istride: */\
		"addq	$%c[__i1],%%rax	\n\t"/* __in0 + 3*istride */\
		"addq	$%c[__i1],%%rcx	\n\t"/* __in0 + 7*istride */\
		"addq	$%c[__i1],%%rbx	\n\t"/* __in0 + b*istride */\
		"addq	$%c[__i1],%%rdx	\n\t"/* __in0 + f*istride */\
		"movaps	    (%%rdx),%%xmm0	\n\t"\
		"movaps	0x10(%%rdx),%%xmm1	\n\t"\
		"movaps	    (%%rdx),%%xmm2	\n\t"\
		"movaps	0x10(%%rdx),%%xmm3	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm0	\n\t"/* cc0 */\
		"mulpd	0x10(%%rsi),%%xmm1	\n\t"\
		"mulpd	0x20(%%rsi),%%xmm2	\n\t"/* ss0 */\
		"mulpd	0x20(%%rsi),%%xmm3	\n\t"\
		"subpd	%%xmm2,%%xmm1		\n\t"\
		"addpd	%%xmm3,%%xmm0		\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rcx),%%xmm6	\n\t"\
		"movaps	0x10(%%rcx),%%xmm7	\n\t"\
		"mulpd	0x20(%%rsi),%%xmm4	\n\t"/* ss0 */\
		"mulpd	0x20(%%rsi),%%xmm5	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm6	\n\t"/* cc0 */\
		"mulpd	0x10(%%rsi),%%xmm7	\n\t"\
		"subpd	%%xmm6,%%xmm5		\n\t"\
		"addpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"addpd	%%xmm0,%%xmm4		\n\t"\
		"addpd	%%xmm1,%%xmm5		\n\t"\
		"subpd	%%xmm0,%%xmm6		\n\t"\
		"subpd	%%xmm1,%%xmm7		\n\t"\
		"movaps	    (%%rbx),%%xmm2	\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"subpd	0x10(%%rbx),%%xmm2	\n\t"\
		"addpd	    (%%rbx),%%xmm3	\n\t"\
		"mulpd	    (%%rsi),%%xmm2	\n\t"/* isrt2 */\
		"mulpd	    (%%rsi),%%xmm3	\n\t"\
		"subpd	%%xmm2,%%xmm0		\n\t"\
		"subpd	%%xmm3,%%xmm1		\n\t"\
		"addpd	%%xmm2,%%xmm2		\n\t"\
		"addpd	%%xmm3,%%xmm3		\n\t"\
		"addpd	%%xmm0,%%xmm2		\n\t"\
		"addpd	%%xmm1,%%xmm3		\n\t"\
		"addq	$%c[__o1],%%r10	\n\t"/* __out0 + 3*ostride */\
		"addq	$%c[__o1],%%r12	\n\t"/* __out0 + 7*ostride */\
		"addq	$%c[__o1],%%r11	\n\t"/* __out0 + b*ostride */\
		"addq	$%c[__o1],%%r13	\n\t"/* __out0 + f*ostride */\
		"subpd	%%xmm6,%%xmm0		\n\t"\
		"subpd	%%xmm7,%%xmm1		\n\t"\
		"movaps	%%xmm0,    (%%r12)	\n\t"\
		"movaps	%%xmm1,0x10(%%r12)	\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm0,%%xmm6		\n\t"\
		"addpd	%%xmm1,%%xmm7		\n\t"\
		"movaps	%%xmm6,    (%%r10)	\n\t"\
		"movaps	%%xmm7,0x10(%%r10)	\n\t"\
		"subpd	%%xmm5,%%xmm2		\n\t"\
		"subpd	%%xmm4,%%xmm3		\n\t"\
		"movaps	%%xmm2,    (%%r13)	\n\t"\
		"movaps	%%xmm3,0x10(%%r11)	\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm2,%%xmm5		\n\t"\
		"addpd	%%xmm3,%%xmm4		\n\t"\
		"movaps	%%xmm5,    (%%r11)	\n\t"\
		"movaps	%%xmm4,0x10(%%r13)	\n\t"\
		:					/* outputs: none */\
		: [__in0] "m" (Xin0)	/* All inputs from memory addresses here */\
		 ,[__i1] "e" (Xi1)\
		 ,[__i2] "e" (Xi2)\
		 ,[__i3] "e" (Xi3)\
		 ,[__i4] "e" (Xi4)\
		 ,[__out0] "m" (Xout0)\
		 ,[__o1] "e" (Xo1)\
		 ,[__o2] "e" (Xo2)\
		 ,[__o3] "e" (Xo3)\
		 ,[__o4] "e" (Xo4)\
		 ,[__isrt2] "m" (Xisrt2)\
		 ,[__c1] "m" (Xc1)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

	// DIF version of above shares same sincos layout & data:
	#define SSE2_RADIX16_DIF_TWIDDLE_OOP(Xin0,Xi1,Xi2,Xi3,Xi4, Xout0,Xout1,Xout2,Xout3,Xout4,Xout5,Xout6,Xout7,Xout8,Xout9,Xouta,Xoutb,Xoutc,Xoutd,Xoute,Xoutf, Xisrt2,Xc1)\
	{\
	__asm__ volatile (\
	/*...Block 0: Do in-place, i.e. outputs into __in0 + [0,1,2,3]*istride: */\
		"movq	%[__in0],%%rax		\n\t"\
		"leaq	%c[__i1](%%rax),%%rcx	\n\t"/* __in0 +   istride */\
		"leaq	%c[__i2](%%rax),%%rbx	\n\t"/* __in0 + 2*istride */\
		"leaq	%c[__i3](%%rax),%%rdx	\n\t"/* __in0 + 3*istride */\
		"/* Do	the p0,1 combo: */	\n\t"\
		"movq	%[__c1],%%rsi 	/* Roots sets c1-15 same as for DIT, w/c1 as base-ptr */\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"mulpd	    (%%rsi),%%xmm4	\n\t"\
		"mulpd	    (%%rsi),%%xmm5	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm6	\n\t"\
		"movaps	%%xmm0,%%xmm2		\n\t"\
		"mulpd	0x10(%%rsi),%%xmm7	\n\t"\
		"addpd	%%xmm6,%%xmm5		\n\t"\
		"movaps	%%xmm1,%%xmm3		\n\t"\
		"subpd	%%xmm7,%%xmm4		\n\t"\
		"addpd	%%xmm4,%%xmm0		\n\t"\
		"addpd	%%xmm5,%%xmm1		\n\t"\
		"subpd	%%xmm4,%%xmm2		\n\t"\
		"subpd	%%xmm5,%%xmm3		\n\t"\
		"/* Do	the p2,3 combo: */	\n\t"\
		"addq	$0x20,%%rsi 	/* c2,3 */\n\t"\
		"movaps	    (%%rdx),%%xmm4	\n\t"\
		"movaps	0x10(%%rdx),%%xmm5	\n\t"\
		"movaps	    (%%rdx),%%xmm6	\n\t"\
		"movaps	0x10(%%rdx),%%xmm7	\n\t"\
		"mulpd	0x20(%%rsi),%%xmm4	/* c3 */\n\t"\
		"mulpd	0x20(%%rsi),%%xmm5	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm6	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm7	\n\t"\
		"addpd	%%xmm6,%%xmm5		\n\t"\
		"subpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t"\
		"movaps	    (%%rbx),%%xmm4	\n\t"\
		"movaps	0x10(%%rbx),%%xmm5	\n\t"\
		"movaps	    (%%rbx),%%xmm6	\n\t"\
		"movaps	0x10(%%rbx),%%xmm7	\n\t"\
		"mulpd	    (%%rsi),%%xmm4	/* c2 */\n\t"\
		"mulpd	    (%%rsi),%%xmm5	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm6	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm7	\n\t"\
		"addpd	%%xmm6,%%xmm5		\n\t"\
		"subpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"subpd	    (%%rax),%%xmm4	\n\t"\
		"subpd	0x10(%%rax),%%xmm5	\n\t"\
		"addpd	    (%%rax),%%xmm6	\n\t"\
		"addpd	0x10(%%rax),%%xmm7	\n\t"\
		"/* Finish radix-4 butterfly and store results into temporary-array slots: */\n\t"\
		"subpd	%%xmm6,%%xmm0		\n\t"\
		"subpd	%%xmm5,%%xmm2		\n\t"\
		"subpd	%%xmm7,%%xmm1		\n\t"\
		"subpd	%%xmm4,%%xmm3		\n\t"\
		"movaps	%%xmm0,    (%%rcx)	\n\t"\
		"movaps	%%xmm2,    (%%rbx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rcx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rdx)	\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm0,%%xmm6		\n\t"\
		"addpd	%%xmm2,%%xmm5		\n\t"\
		"addpd	%%xmm1,%%xmm7		\n\t"\
		"addpd	%%xmm3,%%xmm4		\n\t"\
		"movaps	%%xmm6,    (%%rax)	\n\t"\
		"movaps	%%xmm5,    (%%rdx)	\n\t"\
		"movaps	%%xmm7,0x10(%%rax)	\n\t"\
		"movaps	%%xmm4,0x10(%%rbx)	\n\t"\
		"\n\t"\
	/*...Block 1: outputs into __in0 + [4,5,6,7]*istride: */\
		"addq	$%c[__i4],%%rax	\n\t"/* __in0 + 4*istride */\
		"addq	$%c[__i4],%%rcx	\n\t"/* __in0 + 5*istride */\
		"addq	$%c[__i4],%%rbx	\n\t"/* __in0 + 6*istride */\
		"addq	$%c[__i4],%%rdx	\n\t"/* __in0 + 7*istride */\
		"/* Do	the p0,1 combo: */	\n\t"\
		"addq	$0x40,%%rsi 	/* c4,5 */\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rsi),%%xmm6	\n\t"\
		"movaps	0x10(%%rsi),%%xmm7	\n\t"\
		"movaps	%%xmm0,%%xmm2		\n\t"\
		"movaps	%%xmm1,%%xmm3		\n\t"\
		"mulpd	%%xmm6,%%xmm0		/* c4 */\n\t"\
		"mulpd	%%xmm6,%%xmm1		\n\t"\
		"mulpd	%%xmm7,%%xmm2		\n\t"\
		"mulpd	%%xmm7,%%xmm3		\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"addpd	%%xmm2,%%xmm1		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"mulpd	0x20(%%rsi),%%xmm4	/* c5 */\n\t"\
		"subpd	%%xmm3,%%xmm0		\n\t"\
		"mulpd	0x20(%%rsi),%%xmm5	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm6	\n\t"\
		"movaps	%%xmm0,%%xmm2		\n\t"\
		"mulpd	0x30(%%rsi),%%xmm7	\n\t"\
		"addpd	%%xmm6,%%xmm5		\n\t"\
		"movaps	%%xmm1,%%xmm3		\n\t"\
		"subpd	%%xmm7,%%xmm4		\n\t"\
		"addpd	%%xmm4,%%xmm0		\n\t"\
		"addpd	%%xmm5,%%xmm1		\n\t"\
		"subpd	%%xmm4,%%xmm2		\n\t"\
		"subpd	%%xmm5,%%xmm3		\n\t"\
		"/* Do	the p2,3 combo: */	\n\t"\
		"addq	$0x40,%%rsi 	/* c6,7 */\n\t"\
		"movaps	    (%%rdx),%%xmm4	\n\t"\
		"movaps	0x10(%%rdx),%%xmm5	\n\t"\
		"movaps	    (%%rdx),%%xmm6	\n\t"\
		"movaps	0x10(%%rdx),%%xmm7	\n\t"\
		"mulpd	0x20(%%rsi),%%xmm4	/* c7 */\n\t"\
		"mulpd	0x20(%%rsi),%%xmm5	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm6	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm7	\n\t"\
		"addpd	%%xmm6,%%xmm5		\n\t"\
		"subpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t"\
		"movaps	    (%%rbx),%%xmm4	\n\t"\
		"movaps	0x10(%%rbx),%%xmm5	\n\t"\
		"movaps	    (%%rbx),%%xmm6	\n\t"\
		"movaps	0x10(%%rbx),%%xmm7	\n\t"\
		"mulpd	    (%%rsi),%%xmm4	/* c6 */\n\t"\
		"mulpd	    (%%rsi),%%xmm5	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm6	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm7	\n\t"\
		"addpd	%%xmm6,%%xmm5		\n\t"\
		"subpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"subpd	    (%%rax),%%xmm4	\n\t"\
		"subpd	0x10(%%rax),%%xmm5	\n\t"\
		"addpd	    (%%rax),%%xmm6	\n\t"\
		"addpd	0x10(%%rax),%%xmm7	\n\t"\
		"/* Finish radix-4 butterfly and store results into temporary-array slots: */\n\t"\
		"subpd	%%xmm6,%%xmm0		\n\t"\
		"subpd	%%xmm5,%%xmm2		\n\t"\
		"subpd	%%xmm7,%%xmm1		\n\t"\
		"subpd	%%xmm4,%%xmm3		\n\t"\
		"movaps	%%xmm0,    (%%rcx)	\n\t"\
		"movaps	%%xmm2,    (%%rbx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rcx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rdx)	\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm0,%%xmm6		\n\t"\
		"addpd	%%xmm2,%%xmm5		\n\t"\
		"addpd	%%xmm1,%%xmm7		\n\t"\
		"addpd	%%xmm3,%%xmm4		\n\t"\
		"movaps	%%xmm6,    (%%rax)	\n\t"\
		"movaps	%%xmm5,    (%%rdx)	\n\t"\
		"movaps	%%xmm7,0x10(%%rax)	\n\t"\
		"movaps	%%xmm4,0x10(%%rbx)	\n\t"\
		"\n\t"\
	/*...Block 2: outputs into __in0 + [8,9,a,b]*istride: */\
		"addq	$%c[__i4],%%rax	\n\t"/* __in0 + 8*istride */\
		"addq	$%c[__i4],%%rcx	\n\t"/* __in0 + 9*istride */\
		"addq	$%c[__i4],%%rbx	\n\t"/* __in0 + a*istride */\
		"addq	$%c[__i4],%%rdx	\n\t"/* __in0 + b*istride */\
		"/* Do	the p0,1 combo: */	\n\t"\
		"addq	$0x40,%%rsi 	/* c8,9 */\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rsi),%%xmm6	\n\t"\
		"movaps	0x10(%%rsi),%%xmm7	\n\t"\
		"movaps	%%xmm0,%%xmm2		\n\t"\
		"movaps	%%xmm1,%%xmm3		\n\t"\
		"mulpd	%%xmm6,%%xmm0		/* c8 */\n\t"\
		"mulpd	%%xmm6,%%xmm1		\n\t"\
		"mulpd	%%xmm7,%%xmm2		\n\t"\
		"mulpd	%%xmm7,%%xmm3		\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"addpd	%%xmm2,%%xmm1		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"mulpd	0x20(%%rsi),%%xmm4	/* c9 */\n\t"\
		"subpd	%%xmm3,%%xmm0		\n\t"\
		"mulpd	0x20(%%rsi),%%xmm5	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm6	\n\t"\
		"movaps	%%xmm0,%%xmm2		\n\t"\
		"mulpd	0x30(%%rsi),%%xmm7	\n\t"\
		"addpd	%%xmm6,%%xmm5		\n\t"\
		"movaps	%%xmm1,%%xmm3		\n\t"\
		"subpd	%%xmm7,%%xmm4		\n\t"\
		"addpd	%%xmm4,%%xmm0		\n\t"\
		"addpd	%%xmm5,%%xmm1		\n\t"\
		"subpd	%%xmm4,%%xmm2		\n\t"\
		"subpd	%%xmm5,%%xmm3		\n\t"\
		"/* Do	the p2,3 combo: */	\n\t"\
		"addq	$0x40,%%rsi 	/* ca,b */\n\t"\
		"movaps	    (%%rdx),%%xmm4	\n\t"\
		"movaps	0x10(%%rdx),%%xmm5	\n\t"\
		"movaps	    (%%rdx),%%xmm6	\n\t"\
		"movaps	0x10(%%rdx),%%xmm7	\n\t"\
		"mulpd	0x20(%%rsi),%%xmm4	/* cb */\n\t"\
		"mulpd	0x20(%%rsi),%%xmm5	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm6	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm7	\n\t"\
		"addpd	%%xmm6,%%xmm5		\n\t"\
		"subpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t"\
		"movaps	    (%%rbx),%%xmm4	\n\t"\
		"movaps	0x10(%%rbx),%%xmm5	\n\t"\
		"movaps	    (%%rbx),%%xmm6	\n\t"\
		"movaps	0x10(%%rbx),%%xmm7	\n\t"\
		"mulpd	    (%%rsi),%%xmm4	/* ca */\n\t"\
		"mulpd	    (%%rsi),%%xmm5	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm6	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm7	\n\t"\
		"addpd	%%xmm6,%%xmm5		\n\t"\
		"subpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"subpd	    (%%rax),%%xmm4	\n\t"\
		"subpd	0x10(%%rax),%%xmm5	\n\t"\
		"addpd	    (%%rax),%%xmm6	\n\t"\
		"addpd	0x10(%%rax),%%xmm7	\n\t"\
		"/* Finish radix-4 butterfly and store results into temporary-array slots: */\n\t"\
		"subpd	%%xmm6,%%xmm0		\n\t"\
		"subpd	%%xmm5,%%xmm2		\n\t"\
		"subpd	%%xmm7,%%xmm1		\n\t"\
		"subpd	%%xmm4,%%xmm3		\n\t"\
		"movaps	%%xmm0,    (%%rcx)	\n\t"\
		"movaps	%%xmm2,    (%%rbx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rcx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rdx)	\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm0,%%xmm6		\n\t"\
		"addpd	%%xmm2,%%xmm5		\n\t"\
		"addpd	%%xmm1,%%xmm7		\n\t"\
		"addpd	%%xmm3,%%xmm4		\n\t"\
		"movaps	%%xmm6,    (%%rax)	\n\t"\
		"movaps	%%xmm5,    (%%rdx)	\n\t"\
		"movaps	%%xmm7,0x10(%%rax)	\n\t"\
		"movaps	%%xmm4,0x10(%%rbx)	\n\t"\
		"\n\t"\
	/*...Block 3: outputs into __in0 + [c,d,e,f]*istride: */\
		"addq	$%c[__i4],%%rax	\n\t"/* __in0 + c*istride */\
		"addq	$%c[__i4],%%rcx	\n\t"/* __in0 + d*istride */\
		"addq	$%c[__i4],%%rbx	\n\t"/* __in0 + e*istride */\
		"addq	$%c[__i4],%%rdx	\n\t"/* __in0 + f*istride */\
		"/* Do	the p0,1 combo: */	\n\t"\
		"addq	$0x40,%%rsi 	/* cc,d */\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rsi),%%xmm6	\n\t"\
		"movaps	0x10(%%rsi),%%xmm7	\n\t"\
		"movaps	%%xmm0,%%xmm2		\n\t"\
		"movaps	%%xmm1,%%xmm3		\n\t"\
		"mulpd	%%xmm6,%%xmm0		/* cc */\n\t"\
		"mulpd	%%xmm6,%%xmm1		\n\t"\
		"mulpd	%%xmm7,%%xmm2		\n\t"\
		"mulpd	%%xmm7,%%xmm3		\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"addpd	%%xmm2,%%xmm1		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"mulpd	0x20(%%rsi),%%xmm4	/* cd */\n\t"\
		"subpd	%%xmm3,%%xmm0		\n\t"\
		"mulpd	0x20(%%rsi),%%xmm5	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm6	\n\t"\
		"movaps	%%xmm0,%%xmm2		\n\t"\
		"mulpd	0x30(%%rsi),%%xmm7	\n\t"\
		"addpd	%%xmm6,%%xmm5		\n\t"\
		"movaps	%%xmm1,%%xmm3		\n\t"\
		"subpd	%%xmm7,%%xmm4		\n\t"\
		"addpd	%%xmm4,%%xmm0		\n\t"\
		"addpd	%%xmm5,%%xmm1		\n\t"\
		"subpd	%%xmm4,%%xmm2		\n\t"\
		"subpd	%%xmm5,%%xmm3		\n\t"\
		"/* Do	the p2,3 combo: */	\n\t"\
		"addq	$0x40,%%rsi 	/* ce,f */\n\t"\
		"movaps	    (%%rdx),%%xmm4	\n\t"\
		"movaps	0x10(%%rdx),%%xmm5	\n\t"\
		"movaps	    (%%rdx),%%xmm6	\n\t"\
		"movaps	0x10(%%rdx),%%xmm7	\n\t"\
		"mulpd	0x20(%%rsi),%%xmm4	/* cf */\n\t"\
		"mulpd	0x20(%%rsi),%%xmm5	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm6	\n\t"\
		"mulpd	0x30(%%rsi),%%xmm7	\n\t"\
		"addpd	%%xmm6,%%xmm5		\n\t"\
		"subpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t"\
		"movaps	    (%%rbx),%%xmm4	\n\t"\
		"movaps	0x10(%%rbx),%%xmm5	\n\t"\
		"movaps	    (%%rbx),%%xmm6	\n\t"\
		"movaps	0x10(%%rbx),%%xmm7	\n\t"\
		"mulpd	    (%%rsi),%%xmm4	/* ce */\n\t"\
		"mulpd	    (%%rsi),%%xmm5	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm6	\n\t"\
		"mulpd	0x10(%%rsi),%%xmm7	\n\t"\
		"addpd	%%xmm6,%%xmm5		\n\t"\
		"subpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"subpd	    (%%rax),%%xmm4	\n\t"\
		"subpd	0x10(%%rax),%%xmm5	\n\t"\
		"addpd	    (%%rax),%%xmm6	\n\t"\
		"addpd	0x10(%%rax),%%xmm7	\n\t"\
		"/* Finish radix-4 butterfly and store results into temporary-array slots: */\n\t"\
		"subpd	%%xmm6,%%xmm0		\n\t"\
		"subpd	%%xmm5,%%xmm2		\n\t"\
		"subpd	%%xmm7,%%xmm1		\n\t"\
		"subpd	%%xmm4,%%xmm3		\n\t"\
		"movaps	%%xmm0,    (%%rcx)	\n\t"\
		"movaps	%%xmm2,    (%%rbx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rcx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rdx)	\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm0,%%xmm6		\n\t"\
		"addpd	%%xmm2,%%xmm5		\n\t"\
		"addpd	%%xmm1,%%xmm7		\n\t"\
		"addpd	%%xmm3,%%xmm4		\n\t"\
		"movaps	%%xmm6,    (%%rax)	\n\t"\
		"movaps	%%xmm5,    (%%rdx)	\n\t"\
		"movaps	%%xmm7,0x10(%%rax)	\n\t"\
		"movaps	%%xmm4,0x10(%%rbx)	\n\t"\
	"/*************************************************************************************/\n\t"\
	"/*  And now do four more radix-4 transforms, including the internal twiddle factors: */\n\t"\
	"/*************************************************************************************/\n\t"\
	/* Block 0: Combine 0-output of each radix-4, i.e. inputs from __in0 + [0,4,8,c]*istride: */\
		"movq	%[__in0],%%rax		\n\t"\
		"leaq	%c[__i4](%%rax),%%rbx	\n\t"/* __in0 +   [4*istride] */\
		"leaq	%c[__i4](%%rbx),%%rcx	\n\t"/* __in0 + 2*[4*istride] */\
		"leaq	%c[__i4](%%rcx),%%rdx	\n\t"/* __in0 + 3*[4*istride] */\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rbx),%%xmm2	\n\t"\
		"movaps	    (%%rdx),%%xmm6	\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t"\
		"movaps	0x10(%%rdx),%%xmm7	\n\t"\
		"subpd	%%xmm2,%%xmm0		\n\t"\
		"subpd	%%xmm6,%%xmm4		\n\t"\
		"subpd	%%xmm3,%%xmm1		\n\t"\
		"subpd	%%xmm7,%%xmm5		\n\t"\
		"addpd	%%xmm2,%%xmm2		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm3,%%xmm3		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm0,%%xmm2		\n\t"\
		"addpd	%%xmm4,%%xmm6		\n\t"\
		"addpd	%%xmm1,%%xmm3		\n\t"\
		"addpd	%%xmm5,%%xmm7		\n\t"\
		"subpd	%%xmm6,%%xmm2		\n\t"\
		"subpd	%%xmm7,%%xmm3		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"movq	%[__out0],%%r10		\n\t"\
		"movq	%[__out1],%%r11		\n\t"\
		"movq	%[__out2],%%r12		\n\t"\
		"movq	%[__out3],%%r13		\n\t"\
	"prefetcht1	0x100(%%r11)\n\t"\
		"movaps	%%xmm2,    (%%r11)	\n\t"\
		"movaps	%%xmm3,0x10(%%r11)	\n\t"\
		"addpd	%%xmm2,	%%xmm6	\n\t"\
		"addpd	%%xmm3,	%%xmm7	\n\t"\
		"movaps	%%xmm6,    (%%r10)	\n\t"\
		"movaps	%%xmm7,0x10(%%r10)	\n\t"\
		"subpd	%%xmm5,	%%xmm0	\n\t"\
		"subpd	%%xmm4,	%%xmm1	\n\t"\
		"addpd	%%xmm5,	%%xmm5	\n\t"\
		"addpd	%%xmm4,	%%xmm4	\n\t"\
		"movaps	%%xmm0,    (%%r12)	\n\t"\
		"movaps	%%xmm1,0x10(%%r13)	\n\t"\
		"addpd	%%xmm0,	%%xmm5	\n\t"\
		"addpd	%%xmm1,	%%xmm4	\n\t"\
		"movaps	%%xmm5,    (%%r13)	\n\t"\
		"movaps	%%xmm4,0x10(%%r12)	\n\t"\
	"/* Block 2: Combine 2-output of each radix-4, i.e. inputs from __in0 + [4,5,6,7]*istride: */\n\t"\
		"movq	%[__isrt2],%%rsi 	\n\t"\
		"movaps	(%%rsi),%%xmm3	/* isrt2 */\n\t"\
		"addq	$%c[__i1],%%rax	\n\t"/* __in0 + 1*istride */\
		"addq	$%c[__i1],%%rbx	\n\t"/* __in0 + 5*istride */\
		"addq	$%c[__i1],%%rcx	\n\t"/* __in0 + 9*istride */\
		"addq	$%c[__i1],%%rdx	\n\t"/* __in0 + d*istride */\
	"prefetcht1	0x100(%%r13)\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rdx),%%xmm6	\n\t"\
		"movaps	0x10(%%rdx),%%xmm7	\n\t"\
		"mulpd	%%xmm3,%%xmm4		\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"mulpd	%%xmm3,%%xmm5		\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"mulpd	%%xmm3,%%xmm6		\n\t"\
		"movaps	    (%%rbx),%%xmm2	\n\t"\
		"mulpd	%%xmm3,%%xmm7		\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t"\
		"subpd	%%xmm3,%%xmm0		\n\t"\
		"subpd	%%xmm5,%%xmm4		\n\t"\
		"subpd	%%xmm2,%%xmm1		\n\t"\
		"subpd	%%xmm6,%%xmm7		\n\t"\
		"addpd	%%xmm3,%%xmm3		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm2,%%xmm2		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm0,%%xmm3		\n\t"\
		"addpd	%%xmm4,%%xmm5		\n\t"\
		"addpd	%%xmm1,%%xmm2		\n\t"\
		"addpd	%%xmm7,%%xmm6		\n\t"\
		"subpd	%%xmm6,%%xmm4		\n\t"\
		"subpd	%%xmm7,%%xmm5		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm4,%%xmm6		\n\t"\
		"addpd	%%xmm5,%%xmm7		\n\t"\
		"subpd	%%xmm4,%%xmm0		\n\t"\
		"subpd	%%xmm7,%%xmm3		\n\t"\
		"subpd	%%xmm5,%%xmm2		\n\t"\
		"subpd	%%xmm6,%%xmm1		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"movq	%[__out4],%%r10		\n\t"\
		"movq	%[__out5],%%r11		\n\t"\
		"movq	%[__out6],%%r12		\n\t"\
		"movq	%[__out7],%%r13		\n\t"\
	"prefetcht1	0x100(%%r11)\n\t"\
		"movaps	%%xmm0,    (%%r11)	\n\t"\
		"movaps	%%xmm3,    (%%r12)	\n\t"\
		"movaps	%%xmm2,0x10(%%r11)	\n\t"\
		"movaps	%%xmm1,0x10(%%r13)	\n\t"\
		"addpd	%%xmm0,%%xmm4	\n\t"\
		"addpd	%%xmm3,%%xmm7	\n\t"\
		"addpd	%%xmm2,%%xmm5	\n\t"\
		"addpd	%%xmm1,%%xmm6	\n\t"\
		"movaps	%%xmm4,    (%%r10)	\n\t"\
		"movaps	%%xmm7,    (%%r13)	\n\t"\
		"movaps	%%xmm5,0x10(%%r10)	\n\t"\
		"movaps	%%xmm6,0x10(%%r12)	\n\t"\
	"/* Block 1: Combine 1-output of each radix-4, i.e. inputs from __in0 + [8,9,a,b]*istride: */\n\t"\
		"addq	$%c[__i1],%%rax	\n\t"/* __in0 + 2*istride */\
		"addq	$%c[__i1],%%rbx	\n\t"/* __in0 + 6*istride */\
		"addq	$%c[__i1],%%rcx	\n\t"/* __in0 + a*istride */\
		"addq	$%c[__i1],%%rdx	\n\t"/* __in0 + e*istride */\
	"prefetcht1	0x100(%%r13)\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	0x10(%%rsi),%%xmm3	/* cc0, using isrt2 as base-ptr */\n\t"\
		"movaps	0x20(%%rsi),%%xmm2	/* ss0, using isrt2 as base-ptr */\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"mulpd	%%xmm3,%%xmm4		\n\t"\
		"mulpd	%%xmm3,%%xmm5		\n\t"\
		"mulpd	%%xmm2,%%xmm6		\n\t"\
		"movaps	    (%%rdx),%%xmm0	\n\t"\
		"mulpd	%%xmm2,%%xmm7		\n\t"\
		"movaps	0x10(%%rdx),%%xmm1	\n\t"\
		"addpd	%%xmm6,%%xmm5		\n\t"\
		"movaps	%%xmm0,%%xmm6		\n\t"\
		"subpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm1,%%xmm7		\n\t"\
		"mulpd	%%xmm2,%%xmm6		\n\t"\
		"mulpd	%%xmm2,%%xmm7		\n\t"\
		"mulpd	%%xmm3,%%xmm0		\n\t"\
		"mulpd	%%xmm3,%%xmm1		\n\t"\
		"addpd	%%xmm0,%%xmm7		\n\t"\
		"subpd	%%xmm1,%%xmm6		\n\t"\
		"movaps	%%xmm4,%%xmm2		\n\t"\
		"movaps	%%xmm5,%%xmm3		\n\t"\
		"subpd	%%xmm6,%%xmm4		\n\t"\
		"subpd	%%xmm7,%%xmm5		\n\t"\
		"addpd	%%xmm2,%%xmm6		\n\t"\
		"addpd	%%xmm3,%%xmm7		\n\t"\
		"movaps	    (%%rbx),%%xmm2	\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t"\
		"movaps	    (%%rsi),%%xmm1	/* isrt2 */\n\t"\
		"movaps	%%xmm2,%%xmm0		\n\t"\
		"subpd	%%xmm3,%%xmm2		\n\t"\
		"addpd	%%xmm0,%%xmm3		\n\t"\
		"mulpd	%%xmm1,%%xmm2		\n\t"\
		"mulpd	%%xmm1,%%xmm3		\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"subpd	%%xmm2,%%xmm0		\n\t"\
		"subpd	%%xmm3,%%xmm1		\n\t"\
		"addpd	%%xmm2,%%xmm2		\n\t"\
		"addpd	%%xmm3,%%xmm3		\n\t"\
		"addpd	%%xmm0,%%xmm2		\n\t"\
		"addpd	%%xmm1,%%xmm3		\n\t"\
		"subpd	%%xmm6,%%xmm2		\n\t"\
		"subpd	%%xmm5,%%xmm0		\n\t"\
		"subpd	%%xmm7,%%xmm3		\n\t"\
		"subpd	%%xmm4,%%xmm1		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"movq	%[__out8],%%r10		\n\t"\
		"movq	%[__out9],%%r11		\n\t"\
		"movq	%[__outa],%%r12		\n\t"\
		"movq	%[__outb],%%r13		\n\t"\
	"prefetcht1	0x100(%%r11)\n\t"\
		"movaps	%%xmm2,    (%%r11)	\n\t"\
		"movaps	%%xmm0,    (%%r12)	\n\t"\
		"movaps	%%xmm3,0x10(%%r11)	\n\t"\
		"movaps	%%xmm1,0x10(%%r13)	\n\t"\
		"addpd	%%xmm2,%%xmm6	\n\t"\
		"addpd	%%xmm0,%%xmm5	\n\t"\
		"addpd	%%xmm3,%%xmm7	\n\t"\
		"addpd	%%xmm1,%%xmm4	\n\t"\
		"movaps	%%xmm6,    (%%r10)	\n\t"\
		"movaps	%%xmm5,    (%%r13)	\n\t"\
		"movaps	%%xmm7,0x10(%%r10)	\n\t"\
		"movaps	%%xmm4,0x10(%%r12)	\n\t"\
	"/* Block 3: Combine 3-output of each radix-4, i.e. inputs from __in0 + [c,d,e,f]*istride: */\n\t"\
		"addq	$%c[__i1],%%rax	\n\t"/* __in0 + 3*istride */\
		"addq	$%c[__i1],%%rbx	\n\t"/* __in0 + 7*istride */\
		"addq	$%c[__i1],%%rcx	\n\t"/* __in0 + b*istride */\
		"addq	$%c[__i1],%%rdx	\n\t"/* __in0 + f*istride */\
	"prefetcht1	0x100(%%r13)\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	0x10(%%rsi),%%xmm2	/* cc0, using isrt2 as base-ptr */\n\t"\
		"movaps	0x20(%%rsi),%%xmm3	/* ss0, using isrt2 as base-ptr */\n\t"\
		"movaps	%%xmm4,%%xmm6		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t"\
		"mulpd	%%xmm3,%%xmm4		\n\t"\
		"mulpd	%%xmm3,%%xmm5		\n\t"\
		"mulpd	%%xmm2,%%xmm6		\n\t"\
		"movaps	    (%%rdx),%%xmm0	\n\t"\
		"mulpd	%%xmm2,%%xmm7		\n\t"\
		"movaps	0x10(%%rdx),%%xmm1	\n\t"\
		"addpd	%%xmm6,%%xmm5		\n\t"\
		"movaps	%%xmm0,%%xmm6		\n\t"\
		"subpd	%%xmm7,%%xmm4		\n\t"\
		"movaps	%%xmm1,%%xmm7		\n\t"\
		"mulpd	%%xmm2,%%xmm6		\n\t"\
		"mulpd	%%xmm2,%%xmm7		\n\t"\
		"mulpd	%%xmm3,%%xmm0		\n\t"\
		"mulpd	%%xmm3,%%xmm1		\n\t"\
		"addpd	%%xmm0,%%xmm7		\n\t"\
		"subpd	%%xmm1,%%xmm6		\n\t"\
		"movaps	%%xmm4,%%xmm2		\n\t"\
		"movaps	%%xmm5,%%xmm3		\n\t"\
		"subpd	%%xmm6,%%xmm4		\n\t"\
		"subpd	%%xmm7,%%xmm5		\n\t"\
		"addpd	%%xmm2,%%xmm6		\n\t"\
		"addpd	%%xmm3,%%xmm7		\n\t"\
		"movaps	    (%%rbx),%%xmm2	\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t"\
		"movaps	    (%%rsi),%%xmm1		/* isrt2 */\n\t"\
		"movaps	%%xmm2,%%xmm0		\n\t"\
		"addpd	%%xmm3,%%xmm2		\n\t"\
		"subpd	%%xmm0,%%xmm3		\n\t"\
		"mulpd	%%xmm1,%%xmm2		\n\t"\
		"mulpd	%%xmm1,%%xmm3		\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"subpd	%%xmm2,%%xmm0		\n\t"\
		"subpd	%%xmm3,%%xmm1		\n\t"\
		"addpd	%%xmm2,%%xmm2		\n\t"\
		"addpd	%%xmm3,%%xmm3		\n\t"\
		"addpd	%%xmm0,%%xmm2		\n\t"\
		"addpd	%%xmm1,%%xmm3		\n\t"\
		"subpd	%%xmm4,%%xmm0		\n\t"\
		"subpd	%%xmm7,%%xmm2		\n\t"\
		"subpd	%%xmm5,%%xmm1		\n\t"\
		"subpd	%%xmm6,%%xmm3		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"movq	%[__outc],%%r10		\n\t"\
		"movq	%[__outd],%%r11		\n\t"\
		"movq	%[__oute],%%r12		\n\t"\
		"movq	%[__outf],%%r13		\n\t"\
	"prefetcht1	0x100(%%r11)\n\t"\
		"movaps	%%xmm0,    (%%r11)	\n\t"\
		"movaps	%%xmm2,    (%%r12)	\n\t"\
		"movaps	%%xmm1,0x10(%%r11)	\n\t"\
		"movaps	%%xmm3,0x10(%%r13)	\n\t"\
		"addpd	%%xmm0,%%xmm4	\n\t"\
		"addpd	%%xmm2,%%xmm7	\n\t"\
		"addpd	%%xmm1,%%xmm5	\n\t"\
		"addpd	%%xmm3,%%xmm6	\n\t"\
		"movaps	%%xmm4,    (%%r10)	\n\t"\
		"movaps	%%xmm7,    (%%r13)	\n\t"\
		"movaps	%%xmm5,0x10(%%r10)	\n\t"\
		"movaps	%%xmm6,0x10(%%r12)	\n\t"\
	"prefetcht1	0x100(%%r13)\n\t"\
		:					/* outputs: none */\
		: [__in0] "m" (Xin0)	/* All inputs from memory addresses here */\
		 ,[__i1] "e" (Xi1)\
		 ,[__i2] "e" (Xi2)\
		 ,[__i3] "e" (Xi3)\
		 ,[__i4] "e" (Xi4)\
		 ,[__out0] "m" (Xout0)\
		 ,[__out1] "m" (Xout1)\
		 ,[__out2] "m" (Xout2)\
		 ,[__out3] "m" (Xout3)\
		 ,[__out4] "m" (Xout4)\
		 ,[__out5] "m" (Xout5)\
		 ,[__out6] "m" (Xout6)\
		 ,[__out7] "m" (Xout7)\
		 ,[__out8] "m" (Xout8)\
		 ,[__out9] "m" (Xout9)\
		 ,[__outa] "m" (Xouta)\
		 ,[__outb] "m" (Xoutb)\
		 ,[__outc] "m" (Xoutc)\
		 ,[__outd] "m" (Xoutd)\
		 ,[__oute] "m" (Xoute)\
		 ,[__outf] "m" (Xoutf)\
		 ,[__isrt2] "m" (Xisrt2)\
		 ,[__c1] "m" (Xc1)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}


	#define SSE2_CMUL_EXPO(XcA,XcB,XcAmB,XcApB)\
	{\
	__asm__ volatile (\
		"movq	%[__cA]		,%%rax\n\t"\
		"movq	%[__cB]		,%%rbx\n\t"\
		"movq	%[__cAmB]	,%%rcx\n\t"\
		"movq	%[__cApB]	,%%rdx\n\t"\
		"\n\t"\
		"movaps	    (%%rax),%%xmm0\n\t"\
		"movaps	0x10(%%rax),%%xmm2\n\t"\
		"movaps	    (%%rbx),%%xmm4\n\t"\
		"movaps	0x10(%%rbx),%%xmm5\n\t"\
		"movaps	%%xmm0,%%xmm1\n\t"\
		"movaps	%%xmm2,%%xmm3\n\t"\
		"\n\t"\
		"mulpd	%%xmm4,%%xmm0\n\t"\
		"mulpd	%%xmm5,%%xmm1\n\t"\
		"mulpd	%%xmm4,%%xmm2\n\t"\
		"mulpd	%%xmm5,%%xmm3\n\t"\
		"movaps	%%xmm0,%%xmm4\n\t"\
		"movaps	%%xmm1,%%xmm5\n\t"\
		"addpd	%%xmm3,%%xmm0\n\t"\
		"subpd	%%xmm2,%%xmm1\n\t"\
		"subpd	%%xmm3,%%xmm4\n\t"\
		"addpd	%%xmm2,%%xmm5\n\t"\
		"movaps	%%xmm0,    (%%rcx)\n\t"\
		"movaps	%%xmm1,0x10(%%rcx)\n\t"\
		"movaps	%%xmm4,    (%%rdx)\n\t"\
		"movaps	%%xmm5,0x10(%%rdx)\n\t"\
		:					/* outputs: none */\
		: [__cA]  "m" (XcA)	/* All inputs from memory addresses here */\
		 ,[__cB]  "m" (XcB)\
		 ,[__cAmB] "m" (XcAmB)\
		 ,[__cApB] "m" (XcApB)\
		: "cc","memory","rax","rbx","rcx","rdx","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5"		/* Clobbered registers */\
	);\
	}

	#define PAIR_SQUARE_4_SSE2(XtAr, XtBr, XtCr, XtDr, Xc, Xs, Xforth)\
	{\
	__asm__ volatile (\
		"/*   calculate cross-product terms...\n\t"\
		"	__rt=__tAr* ~tDr+__tAi* ~tDi; __rt=__rt+__rt;\n\t"\
		"	__it=__tAi* ~tDr-__tAr* ~tDi; __it=__it+__it;\n\t"\
		"*/\n\t"\
		"movq	%[__tDr]	,%%rdx\n\t"\
		"movq	%[__tAr]	,%%rax\n\t"\
		"\n\t"\
		"movaps	    (%%rdx)	,%%xmm6		/* tDr */\n\t"\
		"movaps	0x10(%%rdx)	,%%xmm7		/* tDi */\n\t"\
		"movaps	    (%%rax)	,%%xmm0		/* tAr */\n\t"\
		"movaps	0x10(%%rax)	,%%xmm3		/* tAi */\n\t"\
		"shufpd	$1	,%%xmm6	,%%xmm6	/*~tDr */\n\t"\
		"shufpd	$1	,%%xmm7	,%%xmm7	/*~tDi */\n\t"\
		"movaps	    (%%rax)	,%%xmm2		/* cpy tAr */\n\t"\
		"movaps	0x10(%%rax)	,%%xmm1		/* cpy tAi */\n\t"\
		"\n\t"\
		"mulpd	%%xmm6		,%%xmm0	/* tAr*~tDr */\n\t"\
		"mulpd	%%xmm7		,%%xmm3	/* tAi*~tDi */\n\t"\
		"mulpd	%%xmm6		,%%xmm1	/* tAi*~tDr */\n\t"\
		"mulpd	%%xmm7		,%%xmm2	/* tAr*~tDi */\n\t"\
		"addpd	%%xmm3		,%%xmm0	/* rt */\n\t"\
		"subpd	%%xmm2		,%%xmm1	/* it */\n\t"\
		"addpd	%%xmm0		,%%xmm0	/* rt=rt+rt */\n\t"\
		"addpd	%%xmm1		,%%xmm1	/* it=it+it; xmm2-7 free */\n\t"\
		"/*\n\t"\
		"	__st=__tBr* ~tCr+__tBi* ~tCi; __st=__st+__st;\n\t"\
		"	__jt=__tBi* ~tCr-__tBr* ~tCi; __jt=__jt+__jt;\n\t"\
		"*/\n\t"\
		"movq	%[__tCr]	,%%rcx\n\t"\
		"movq	%[__tBr]	,%%rbx\n\t"\
		"\n\t"\
		"movaps	    (%%rcx)	,%%xmm6		/* tCr */\n\t"\
		"movaps	0x10(%%rcx)	,%%xmm7		/* tCi */\n\t"\
		"movaps	    (%%rbx)	,%%xmm2		/* tBr */\n\t"\
		"movaps	0x10(%%rbx)	,%%xmm5		/* tBi */\n\t"\
		"shufpd	$1	,%%xmm6	,%%xmm6	/*~tCr */\n\t"\
		"shufpd	$1	,%%xmm7	,%%xmm7	/*~tCi */\n\t"\
		"movaps	    (%%rbx)	,%%xmm4		/* cpy tBr */\n\t"\
		"movaps	0x10(%%rbx)	,%%xmm3		/* cpy tBi */\n\t"\
		"\n\t"\
		"mulpd	%%xmm6		,%%xmm2	/* tBr*~tCr */\n\t"\
		"mulpd	%%xmm7		,%%xmm5	/* tBi*~tCi */\n\t"\
		"mulpd	%%xmm6		,%%xmm3	/* tBi*~tCr */\n\t"\
		"mulpd	%%xmm7		,%%xmm4	/* tBr*~tCi */\n\t"\
		"addpd	%%xmm5		,%%xmm2	/* st */\n\t"\
		"subpd	%%xmm4		,%%xmm3	/* jt */\n\t"\
		"addpd	%%xmm2		,%%xmm2	/* st=st+st */\n\t"\
		"addpd	%%xmm3		,%%xmm3	/* jt=jt+jt; xmm4-7 free */\n\t"\
		"\n\t"\
		"/*   now calculate square terms and __store back in the same temporaries:	*/\n\t"\
		"/*	__tmp=(__tAr+__tAi)*(__tAr-__tAi); __tAi=__tAr*__tAi; __tAi=__tAi+__tAi; __tAr=__tmp;	*/\n\t"\
		"\n\t"\
		"movaps	    (%%rax)	,%%xmm4		/* __tAr */\n\t"\
		"movaps	0x10(%%rax)	,%%xmm5		/* __tAi */\n\t"\
		"subpd	%%xmm5		,%%xmm4		/* (__tAr-__tAi) */\n\t"\
		"addpd	%%xmm5		,%%xmm5		/*      2*__tAi  */\n\t"\
		"addpd	%%xmm4		,%%xmm5		/* (__tAr+__tAi) */\n\t"\
		"mulpd	%%xmm5		,%%xmm4		/*>__tAr */\n\t"\
		"\n\t"\
		"movaps	    (%%rax)	,%%xmm5		/* __tAr */\n\t"\
		"mulpd	0x10(%%rax)	,%%xmm5		/* __tAr*__tAi */\n\t"\
		"addpd	%%xmm5		,%%xmm5		/*>__tAi */\n\t"\
		"movaps	%%xmm4	,    (%%rax)	/* tmp store >__tAr */\n\t"\
		"movaps	%%xmm5	,0x10(%%rax)	/* tmp store >__tAi */\n\t"\
		"\n\t"\
		"subpd	%%xmm4		,%%xmm0	/* rt-__tAr */\n\t"\
		"subpd	%%xmm5		,%%xmm1	/* it-__tAi; xmm4-7 free */\n\t"\
		"\n\t"\
		"/*	__tmp=(__tBr+__tBi)*(__tBr-__tBi); __tBi=__tBr*__tBi; __tBi=__tBi+__tBi; __tBr=__tmp;	*/\n\t"\
		"/*** [Can be done in parallel with above segment] ***/\n\t"\
		"\n\t"\
		"movaps	    (%%rbx)	,%%xmm6		/* __tBr */\n\t"\
		"movaps	0x10(%%rbx)	,%%xmm7		/* __tBi */\n\t"\
		"subpd	%%xmm7		,%%xmm6		/* (__tBr-__tBi) */\n\t"\
		"addpd	%%xmm7		,%%xmm7		/*      2*__tBi  */\n\t"\
		"addpd	%%xmm6		,%%xmm7		/* (__tBr+__tBi) */\n\t"\
		"mulpd	%%xmm7		,%%xmm6		/*>__tBr */\n\t"\
		"\n\t"\
		"movaps	    (%%rbx)	,%%xmm7		/* __tBr */\n\t"\
		"mulpd	0x10(%%rbx)	,%%xmm7		/* __tBr*__tBi */\n\t"\
		"addpd	%%xmm7		,%%xmm7		/*>__tBi */\n\t"\
		"movaps	%%xmm6	,    (%%rbx)	/* tmp store >__tBr */\n\t"\
		"movaps	%%xmm7	,0x10(%%rbx)	/* tmp store >__tBi */\n\t"\
		"\n\t"\
		"subpd	%%xmm6		,%%xmm2	/* st-__tBr */\n\t"\
		"subpd	%%xmm7		,%%xmm3	/* jt-__tBi; xmm4-7 free */\n\t"\
		"\n\t"\
		"/*	__tmp=(__tDr+__tDi)*(__tDr-__tDi); __tDi=__tDr*__tDi; __tDi=__tDi+__tDi; __tDr=__tmp;	*/\n\t"\
		"\n\t"\
		"movaps	    (%%rdx)	,%%xmm4		/* __tDr */\n\t"\
		"movaps	0x10(%%rdx)	,%%xmm5		/* __tDi */\n\t"\
		"subpd	%%xmm5		,%%xmm4		/* (__tDr-__tDi) */\n\t"\
		"addpd	%%xmm5		,%%xmm5		/*      2*__tDi  */\n\t"\
		"addpd	%%xmm4		,%%xmm5		/* (__tDr+__tDi) */\n\t"\
		"mulpd	%%xmm5		,%%xmm4		/*>__tDr */\n\t"\
		"\n\t"\
		"movaps	    (%%rdx)	,%%xmm5		/* __tDr */\n\t"\
		"mulpd	0x10(%%rdx)	,%%xmm5		/* __tDr*__tDi */\n\t"\
		"addpd	%%xmm5		,%%xmm5		/*>__tDi */\n\t"\
		"movaps	%%xmm4	,    (%%rdx)	/* tmp store ~tDr */\n\t"\
		"movaps	%%xmm5	,0x10(%%rdx)	/* tmp store ~tDi */\n\t"\
		"shufpd	$1	,%%xmm4	,%%xmm4	/*~tDr */\n\t"\
		"shufpd	$1	,%%xmm5	,%%xmm5	/*~tDi */\n\t"\
		"\n\t"\
		"subpd	%%xmm4		,%%xmm0	/* rt-__tAr- ~tDr */\n\t"\
		"addpd	%%xmm5		,%%xmm1	/* it-__tAi+ ~tDi; xmm4-7 free */\n\t"\
		"\n\t"\
		"/*	__tmp=(__tCr+__tCi)*(__tCr-__tCi); __tCi=__tCr*__tCi; __tCi=__tCi+__tCi; __tCr=__tmp;	*/\n\t"\
		"/*** [Can be done in parallel with above segment] ***/\n\t"\
		"\n\t"\
		"movaps	    (%%rcx)	,%%xmm6		/* __tCr */\n\t"\
		"movaps	0x10(%%rcx)	,%%xmm7		/* __tCi */\n\t"\
		"subpd	%%xmm7		,%%xmm6		/* (__tCr-__tCi) */\n\t"\
		"addpd	%%xmm7		,%%xmm7		/*      2*__tCi  */\n\t"\
		"addpd	%%xmm6		,%%xmm7		/* (__tCr+__tCi) */\n\t"\
		"mulpd	%%xmm7		,%%xmm6		/*>__tCr */\n\t"\
		"\n\t"\
		"movaps	    (%%rcx)	,%%xmm7		/* __tCr */\n\t"\
		"mulpd	0x10(%%rcx)	,%%xmm7		/* __tCr*__tCi */\n\t"\
		"addpd	%%xmm7		,%%xmm7		/*>__tCi */\n\t"\
		"movaps	%%xmm6	,    (%%rcx)	/* tmp store ~tCr */\n\t"\
		"movaps	%%xmm7	,0x10(%%rcx)	/* tmp store ~tCi */\n\t"\
		"shufpd	$1	,%%xmm6	,%%xmm6	/*~tCr */\n\t"\
		"shufpd	$1	,%%xmm7	,%%xmm7	/*~tCi */\n\t"\
		"\n\t"\
		"subpd	%%xmm6		,%%xmm2	/* st-__tBr- ~tCr */\n\t"\
		"addpd	%%xmm7		,%%xmm3	/* jt-__tBi+ ~tCi; xmm4-7 free */\n\t"\
		"/*\n\t"\
		"	__tmp=((1.0+__c)*__rt-__s*__it)*0.25;\n\t"\
		"	__it =((1.0+__c)*__it+__s*__rt)*0.25;	__rt=__tmp;\n\t"\
		"*/\n\t"\
		"/*** [Can be done in parallel with above segment] ***/\n\t"\
		"movq	%[__c]		,%%rax\n\t"\
		"movq	%[__s]		,%%rbx\n\t"\
		"movq	%[__forth]	,%%rdx\n\t"\
		"movaps	%%xmm0		,%%xmm4		/* cpy rt */\n\t"\
		"movaps	%%xmm1		,%%xmm5		/* cpy it */\n\t"\
		"mulpd	(%%rax)		,%%xmm0		/* c*rt */\n\t"\
		"mulpd	(%%rax)		,%%xmm1		/* c*it */\n\t"\
		"addpd	%%xmm4		,%%xmm0		/* (c+1.0)*rt */\n\t"\
		"addpd	%%xmm5		,%%xmm1		/* (c+1.0)*it */\n\t"\
		"mulpd	(%%rbx)		,%%xmm4		/* s*rt */\n\t"\
		"mulpd	(%%rbx)		,%%xmm5		/* s*it */\n\t"\
		"subpd	%%xmm5		,%%xmm0		/* (c+1.0)*rt-s*it */\n\t"\
		"addpd	%%xmm4		,%%xmm1		/* (c+1.0)*it+s*rt; xmm4,5 free */\n\t"\
		"mulpd	(%%rdx)		,%%xmm0	/* -rt Both of these inherit the sign flip [w.r.to the non-SSE2 PAIR_SQUARE_4 macro] */\n\t"\
		"mulpd	(%%rdx)		,%%xmm1	/* -it that resulted from the in-place-friendlier (rt-__tAr- ~tDr) reordering above. */\n\t"\
		"/*\n\t"\
		"	__tmp=((1.0-__s)*__st-__c*__jt)*0.25;\n\t"\
		"	__jt =((1.0-__s)*__jt+__c*__st)*0.25	__st=__tmp;\n\t"\
		"*/\n\t"\
		"/*** [Can be done in parallel wjth above segment] ***/\n\t"\
		"movaps	%%xmm2		,%%xmm6		/* cpy st */\n\t"\
		"movaps	%%xmm3		,%%xmm7		/* cpy jt */\n\t"\
		"mulpd	(%%rbx)		,%%xmm2		/* s*st */\n\t"\
		"mulpd	(%%rbx)		,%%xmm3		/* s*jt */\n\t"\
		"subpd	%%xmm6		,%%xmm2		/* (s-1.0)*st, note sign flip! */\n\t"\
		"subpd	%%xmm7		,%%xmm3		/* (s-1.0)*jt, note sign flip! */\n\t"\
		"mulpd	(%%rax)		,%%xmm6		/* c*st */\n\t"\
		"mulpd	(%%rax)		,%%xmm7		/* c*jt */\n\t"\
		"addpd	%%xmm7		,%%xmm2		/* -[(1.0-s)*st-c*jt] */\n\t"\
		"subpd	%%xmm6		,%%xmm3		/* -[(1.0-s)*jt+c*st]; xmm6,7 free */\n\t"\
		"mulpd	(%%rdx)		,%%xmm2	/* +st Sign flip due to (s-1.0) reordering here */\n\t"\
		"mulpd	(%%rdx)		,%%xmm3	/* +jt cancels earlier one due to in-place-friendlier (st-__tBr- ~tCr) reordering above. */\n\t"\
		"/*...and now complete and store the results. We flip the signs on st and jt here to undo the above -st,-jt negations. */\n\t"\
		"/*	__tAr = (__tAr+__rt);\n\t"\
		"	__tAi = (__tAi+__it);\n\t"\
		"	__tBr = (__tBr-__st);\n\t"\
		"	__tBi = (__tBi-__jt);\n\t"\
		"*/\n\t"\
		"movq	%[__tAr]	,%%rax\n\t"\
		"movq	%[__tBr]	,%%rbx\n\t"\
		"\n\t"\
		"movaps	    (%%rax)	,%%xmm4		/* __tAr */\n\t"\
		"movaps	0x10(%%rax)	,%%xmm5		/* __tAi */\n\t"\
		"movaps	    (%%rbx)	,%%xmm6		/* __tBr */\n\t"\
		"movaps	0x10(%%rbx)	,%%xmm7		/* __tBi */\n\t"\
		"addpd	%%xmm0		,%%xmm4		/* (__tAr+__rt) */\n\t"\
		"addpd	%%xmm1		,%%xmm5		/* (__tAi+__it) */\n\t"\
		"subpd	%%xmm2		,%%xmm6		/* (__tBr-__st) */\n\t"\
		"subpd	%%xmm3		,%%xmm7		/* (__tBi-__jt) */\n\t"\
		"movaps	%%xmm4	,    (%%rax)	/* store >__tAr */\n\t"\
		"movaps	%%xmm5	,0x10(%%rax)	/* store >__tAi */\n\t"\
		"movaps	%%xmm6	,    (%%rbx)	/* store >__tBr */\n\t"\
		"movaps	%%xmm7	,0x10(%%rbx)	/* store >__tBi */\n\t"\
		"/*...N-j terms are as above, but with the replacements: __tAr<--> ~tDr, __tAi<--> ~tDi, __it|-->-__it. */\n\t"\
		"/*	__tDr = (__tDr+ ~rt);\n\t"\
		"	__tDi = (__tDi- ~it);\n\t"\
		"	__tCr = (__tCr- ~st);\n\t"\
		"	__tCi = (__tCi+ ~jt);\n\t"\
		"*/\n\t"\
		"movq	%[__tCr]	,%%rcx\n\t"\
		"movq	%[__tDr]	,%%rdx\n\t"\
		"\n\t"\
		"shufpd	$1	,%%xmm0	,%%xmm0		/* ~rt */\n\t"\
		"shufpd	$1	,%%xmm1	,%%xmm1		/* ~it */\n\t"\
		"shufpd	$1	,%%xmm2	,%%xmm2		/* ~st */\n\t"\
		"shufpd	$1	,%%xmm3	,%%xmm3		/* ~jt */\n\t"\
		"\n\t"\
		"movaps	    (%%rdx)	,%%xmm4		/* __tDr */\n\t"\
		"movaps	0x10(%%rdx)	,%%xmm5		/* __tDi */\n\t"\
		"movaps	    (%%rcx)	,%%xmm6		/* __tCr */\n\t"\
		"movaps	0x10(%%rcx)	,%%xmm7		/* __tCi */\n\t"\
		"addpd	%%xmm0		,%%xmm4		/* (__tDr+ ~rt) */\n\t"\
		"subpd	%%xmm1		,%%xmm5		/* (__tDi- ~it) */\n\t"\
		"subpd	%%xmm2		,%%xmm6		/* (__tCr- ~st) */\n\t"\
		"addpd	%%xmm3		,%%xmm7		/* (__tCi+ ~jt) */\n\t"\
		"movaps	%%xmm4	,    (%%rdx)	/* store >__tDr */\n\t"\
		"movaps	%%xmm5	,0x10(%%rdx)	/* store >__tDi */\n\t"\
		"movaps	%%xmm6	,    (%%rcx)	/* store >__tCr */\n\t"\
		"movaps	%%xmm7	,0x10(%%rcx)	/* store >__tCi */\n\t"\
		:					/* outputs: none */\
		: [__tAr] "m" (XtAr)	/* All inputs from memory addresses here */\
		 ,[__tBr] "m" (XtBr)\
		 ,[__tCr] "m" (XtCr)\
		 ,[__tDr] "m" (XtDr)\
		 ,[__c] "m" (Xc)\
		 ,[__s] "m" (Xs)\
		 ,[__forth] "m" (Xforth)\
		: "cc","memory","rax","rbx","rcx","rdx","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

	#define SSE2_RADIX_03_DFT(Xi0,Xi1,Xi2, Xcc1, Xo0,Xo1,Xo2)\
	{\
	__asm__ volatile (\
			"movq	%[__i0],%%rax		\n\t"\
			"movq	%[__i1],%%rbx		\n\t"\
			"movq	%[__i2],%%rcx		\n\t"\
			"movq	%[__cc1],%%rdx		\n\t"\
			"\n\t"\
			"movaps	    (%%rbx),%%xmm2	\n\t"\
			"movaps	0x10(%%rbx),%%xmm3	\n\t"\
			"movaps	    (%%rax),%%xmm0	\n\t"\
			"movaps	0x10(%%rax),%%xmm1	\n\t"\
			"movaps	    (%%rcx),%%xmm6	\n\t"\
			"movaps	0x10(%%rcx),%%xmm7	\n\t"\
			"movaps	%%xmm2,%%xmm4		\n\t"\
			"movaps	%%xmm3,%%xmm5		\n\t"\
			"\n\t"\
			"movq	%[__o0],%%rax		\n\t"\
			"movq	%[__o1],%%rbx		\n\t"\
			"movq	%[__o2],%%rcx		\n\t"\
			"addpd	%%xmm6,%%xmm2		\n\t"\
			"addpd	%%xmm7,%%xmm3		\n\t"\
			"subpd	%%xmm6,%%xmm4		\n\t"\
			"subpd	%%xmm7,%%xmm5		\n\t"\
			"addpd	%%xmm2,%%xmm0		\n\t"\
			"addpd	%%xmm3,%%xmm1		\n\t"\
			"movaps	    (%%rdx),%%xmm6	\n\t"\
			"movaps	0x10(%%rdx),%%xmm7	\n\t"\
			"movaps	%%xmm0,    (%%rax)	\n\t"\
			"movaps	%%xmm1,0x10(%%rax)	\n\t"\
			"\n\t"\
			"mulpd	%%xmm6,%%xmm2		\n\t"\
			"mulpd	%%xmm6,%%xmm3		\n\t"\
			"mulpd	%%xmm7,%%xmm4		\n\t"\
			"mulpd	%%xmm7,%%xmm5		\n\t"\
			"addpd	%%xmm0,%%xmm2		\n\t"\
			"addpd	%%xmm1,%%xmm3		\n\t"\
			"\n\t"\
			"movaps	%%xmm2,%%xmm0		\n\t"\
			"movaps	%%xmm3,%%xmm1		\n\t"\
			"\n\t"\
			"subpd	%%xmm5,%%xmm2		\n\t"\
			"addpd	%%xmm4,%%xmm3		\n\t"\
			"addpd	%%xmm5,%%xmm0		\n\t"\
			"subpd	%%xmm4,%%xmm1		\n\t"\
			"\n\t"\
			"movaps	%%xmm2,    (%%rbx)	\n\t"\
			"movaps	%%xmm3,0x10(%%rbx)	\n\t"\
			"movaps	%%xmm0,    (%%rcx)	\n\t"\
			"movaps	%%xmm1,0x10(%%rcx)	\n\t"\
		:					/* outputs: none */\
		: [__i0] "m" (Xi0)	/* All inputs from memory addresses here */\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__cc1] "m" (Xcc1)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		: "cc","memory","rax","rbx","rcx","rdx","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

	#define SSE2_RADIX_03_DFT_X2(Xcc0, Xi0,Xi1,Xi2, Xo0,Xo1,Xo2, Xj0,Xj1,Xj2, Xu0,Xu1,Xu2)\
	{\
	__asm__ volatile (\
		"movq	%[__i0],%%rax		\n\t	movq	%[__j0],%%r10		\n\t"\
		"movq	%[__i1],%%rbx		\n\t	movq	%[__j1],%%r11		\n\t"\
		"movq	%[__i2],%%rcx		\n\t	movq	%[__j2],%%r12		\n\t"\
		"movq	%[__cc0],%%rdx		\n\t"\
		"movaps	    (%%rbx),%%xmm2	\n\t	movaps	    (%%r11),%%xmm10	\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t	movaps	0x10(%%r11),%%xmm11	\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t	movaps	    (%%r10),%%xmm8 	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t	movaps	0x10(%%r10),%%xmm9 	\n\t"\
		"movaps	    (%%rcx),%%xmm6	\n\t	movaps	    (%%r12),%%xmm14	\n\t"\
		"movaps	0x10(%%rcx),%%xmm7	\n\t	movaps	0x10(%%r12),%%xmm15	\n\t"\
		"movaps	%%xmm2,%%xmm4		\n\t	movaps	%%xmm10,%%xmm12		\n\t"\
		"movaps	%%xmm3,%%xmm5		\n\t	movaps	%%xmm11,%%xmm13		\n\t"\
		"movq	%[__o0],%%rax		\n\t	movq	%[__u0],%%r10		\n\t"\
		"movq	%[__o1],%%rbx		\n\t	movq	%[__u1],%%r11		\n\t"\
		"movq	%[__o2],%%rcx		\n\t	movq	%[__u2],%%r12		\n\t"\
		"addpd	%%xmm6,%%xmm2		\n\t	addpd	%%xmm14,%%xmm10		\n\t"\
		"addpd	%%xmm7,%%xmm3		\n\t	addpd	%%xmm15,%%xmm11		\n\t"\
		"subpd	%%xmm6,%%xmm4		\n\t	subpd	%%xmm14,%%xmm12		\n\t"\
		"subpd	%%xmm7,%%xmm5		\n\t	subpd	%%xmm15,%%xmm13		\n\t"\
		"addpd	%%xmm2,%%xmm0		\n\t	addpd	%%xmm10,%%xmm8 		\n\t"\
		"addpd	%%xmm3,%%xmm1		\n\t	addpd	%%xmm11,%%xmm9 		\n\t"\
		"movaps	    (%%rdx),%%xmm6	\n\t"\
		"movaps	0x10(%%rdx),%%xmm7	\n\t"\
		"movaps	%%xmm0,     (%%rax)	\n\t	movaps	%%xmm8 ,     (%%r10)\n\t"\
		"movaps	%%xmm1,0x010(%%rax)	\n\t	movaps	%%xmm9 ,0x010(%%r10)\n\t"\
		"mulpd	%%xmm6,%%xmm2		\n\t	mulpd	%%xmm6 ,%%xmm10		\n\t"\
		"mulpd	%%xmm6,%%xmm3		\n\t	mulpd	%%xmm6 ,%%xmm11		\n\t"\
		"mulpd	%%xmm7,%%xmm4		\n\t	mulpd	%%xmm7 ,%%xmm12		\n\t"\
		"mulpd	%%xmm7,%%xmm5		\n\t	mulpd	%%xmm7 ,%%xmm13		\n\t"\
		"addpd	%%xmm0,%%xmm2		\n\t	addpd	%%xmm8 ,%%xmm10		\n\t"\
		"addpd	%%xmm1,%%xmm3		\n\t	addpd	%%xmm9 ,%%xmm11		\n\t"\
		"movaps	%%xmm2,%%xmm0		\n\t	movaps	%%xmm10,%%xmm8 		\n\t"\
		"movaps	%%xmm3,%%xmm1		\n\t	movaps	%%xmm11,%%xmm9 		\n\t"\
		"subpd	%%xmm5,%%xmm2		\n\t	subpd	%%xmm13,%%xmm10		\n\t"\
		"addpd	%%xmm4,%%xmm3		\n\t	addpd	%%xmm12,%%xmm11		\n\t"\
		"addpd	%%xmm5,%%xmm0		\n\t	addpd	%%xmm13,%%xmm8 		\n\t"\
		"subpd	%%xmm4,%%xmm1		\n\t	subpd	%%xmm12,%%xmm9 		\n\t"\
		"movaps	%%xmm2,     (%%rbx)	\n\t	movaps	%%xmm10,     (%%r11)\n\t"\
		"movaps	%%xmm3,0x010(%%rbx)	\n\t	movaps	%%xmm11,0x010(%%r11)\n\t"\
		"movaps	%%xmm0,     (%%rcx)	\n\t	movaps	%%xmm8 ,     (%%r12)\n\t"\
		"movaps	%%xmm1,0x010(%%rcx)	\n\t	movaps	%%xmm9 ,0x010(%%r12)\n\t"\
		:					/* outputs: none */\
		: [__cc0] "m" (Xcc0)	/* All inputs from memory addresses here */\
		 ,[__i0] "m" (Xi0)\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__j0] "m" (Xj0)\
		 ,[__j1] "m" (Xj1)\
		 ,[__j2] "m" (Xj2)\
		 ,[__u0] "m" (Xu0)\
		 ,[__u1] "m" (Xu1)\
		 ,[__u2] "m" (Xu2)\
		: "cc","memory","rax","rbx","rcx","rdx","r10","r11","r12","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

	#define SSE2_RADIX4_DIF_0TWIDDLE_STRIDE(Xadd0, Xadd1, Xadd2, Xadd3, Xtmp, Xstride)\
	{\
	__asm__ volatile (\
		"movq	%[__tmp]   ,%%rax	\n\t"\
		"movq	%[__stride],%%rsi	\n\t"\
		"movq	%%rax,%%rbx			\n\t"\
		"addq	%%rsi,%%rbx			/* add_in1  */\n\t"\
		"shlq	$1,%%rsi			/* stride*2 */\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	    (%%rbx),%%xmm2	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t"\
		"movaps	    (%%rax),%%xmm4	\n\t"\
		"movaps	    (%%rbx),%%xmm6	\n\t"\
		"movaps	0x10(%%rax),%%xmm5	\n\t"\
		"movaps	0x10(%%rbx),%%xmm7	\n\t"\
		"addq	%%rsi,%%rax			/* add_in2  */\n\t"\
		"addq	%%rsi,%%rbx			/* add_in3  */\n\t"\
		"addpd	    (%%rax),%%xmm0	\n\t"\
		"addpd	    (%%rbx),%%xmm2	\n\t"\
		"addpd	0x10(%%rax),%%xmm1	\n\t"\
		"addpd	0x10(%%rbx),%%xmm3	\n\t"\
		"subpd	    (%%rax),%%xmm4	\n\t"\
		"subpd	    (%%rbx),%%xmm6	\n\t"\
		"subpd	0x10(%%rax),%%xmm5	\n\t"\
		"subpd	0x10(%%rbx),%%xmm7	\n\t"\
		"/* Finish radix-4 butterfly and store results into main-array slots: */\n\t"\
		"movq	%[__add0],%%rax		\n\t"\
		"movq	%[__add1],%%rbx		\n\t"\
		"movq	%[__add2],%%rcx		\n\t"\
		"movq	%[__add3],%%rdx		\n\t"\
		"subpd	%%xmm2,%%xmm0		\n\t"\
		"subpd	%%xmm7,%%xmm4		\n\t"\
		"subpd	%%xmm3,%%xmm1		\n\t"\
		"subpd	%%xmm6,%%xmm5		\n\t"\
		"movaps	%%xmm0,     (%%rbx)	\n\t"\
		"movaps	%%xmm4,     (%%rcx)	\n\t"\
		"movaps	%%xmm1,0x010(%%rbx)	\n\t"\
		"movaps	%%xmm5,0x010(%%rdx)	\n\t"\
		"addpd	%%xmm2,%%xmm2		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm3,%%xmm3		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm0,%%xmm2		\n\t"\
		"addpd	%%xmm4,%%xmm7		\n\t"\
		"addpd	%%xmm1,%%xmm3		\n\t"\
		"addpd	%%xmm5,%%xmm6		\n\t"\
		"movaps	%%xmm2,     (%%rax)	\n\t"\
		"movaps	%%xmm7,     (%%rdx)	\n\t"\
		"movaps	%%xmm3,0x010(%%rax)	\n\t"\
		"movaps	%%xmm6,0x010(%%rcx)	\n\t"\
		:					/* outputs: none */\
		: [__add0] "m" (Xadd0)	/* All inputs from memory addresses here */\
		 ,[__add1] "m" (Xadd1)\
		 ,[__add2] "m" (Xadd2)\
		 ,[__add3] "m" (Xadd3)\
		 ,[__tmp] "m" (Xtmp)\
		 ,[__stride] "e" (Xstride)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

	/* DIF radix-4 subconvolution, sans twiddles, inputs in __i0-3, outputs in __o0-3, possibly coincident with inputs: */
	#define SSE2_RADIX4_DIF_0TWIDDLE_STRIDE_E(Xi0,Xi1,Xi2,Xi3, Xo0,Xo1,Xo2,Xo3)\
	{\
	__asm__ volatile (\
		"movq	%[__i0],%%rax		\n\t"\
		"movq	%[__i1],%%rbx		\n\t"\
		"movq	%[__i2],%%rcx		\n\t"\
		"movq	%[__i3],%%rdx		\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	    (%%rbx),%%xmm4	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	0x10(%%rbx),%%xmm5	\n\t"\
		"movaps	%%xmm0,%%xmm2	\n\t"\
		"movaps	%%xmm4,%%xmm6	\n\t"\
		"movaps	%%xmm1,%%xmm3	\n\t"\
		"movaps	%%xmm5,%%xmm7	\n\t"\
		"addpd	    (%%rcx),%%xmm0	\n\t"\
		"addpd	    (%%rdx),%%xmm4	\n\t"\
		"addpd	0x10(%%rcx),%%xmm1	\n\t"\
		"addpd	0x10(%%rdx),%%xmm5	\n\t"\
		"subpd	    (%%rcx),%%xmm2	\n\t"\
		"subpd	    (%%rdx),%%xmm6	\n\t"\
		"subpd	0x10(%%rcx),%%xmm3	\n\t"\
		"subpd	0x10(%%rdx),%%xmm7	\n\t"\
		"/* Finish radix-4 butterfly and store results into main-array slots: */\n\t"\
		"movq	%[__o0],%%rax		\n\t"\
		"movq	%[__o1],%%rbx		\n\t"\
		"movq	%[__o2],%%rcx		\n\t"\
		"movq	%[__o3],%%rdx		\n\t"\
		"subpd	%%xmm4,%%xmm0		\n\t"\
		"subpd	%%xmm7,%%xmm2		\n\t"\
		"subpd	%%xmm5,%%xmm1		\n\t"\
		"subpd	%%xmm6,%%xmm3		\n\t"\
		"movaps	%%xmm0,    (%%rbx)	\n\t"\
		"movaps	%%xmm2,    (%%rcx)	\n\t"\
		"movaps	%%xmm1,0x10(%%rbx)	\n\t"\
		"movaps	%%xmm3,0x10(%%rdx)	\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm0,%%xmm4		\n\t"\
		"addpd	%%xmm2,%%xmm7		\n\t"\
		"addpd	%%xmm1,%%xmm5		\n\t"\
		"addpd	%%xmm3,%%xmm6		\n\t"\
		"movaps	%%xmm4,    (%%rax)	\n\t"\
		"movaps	%%xmm7,    (%%rdx)	\n\t"\
		"movaps	%%xmm5,0x10(%%rax)	\n\t"\
		"movaps	%%xmm6,0x10(%%rcx)	\n\t"\
		:					/* outputs: none */\
		: [__i0] "m" (Xi0)	/* All inputs from memory addresses here */\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__i3] "m" (Xi3)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__o3] "m" (Xo3)\
		: "cc","memory","rax","rbx","rcx","rdx","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

	#define SSE2_RADIX4_DIT_0TWIDDLE_STRIDE(Xadd0, Xadd1, Xadd2, Xadd3, Xtmp, Xstride)\
	{\
	__asm__ volatile (\
		"movq	%[__add0],%%rax		\n\t"\
		"movq	%[__add1],%%rbx		\n\t"\
		"movq	%[__add2],%%rcx		\n\t"\
		"movq	%[__add3],%%rdx		\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	%%xmm0,%%xmm2			\n\t"\
		"movaps	%%xmm4,%%xmm6			\n\t"\
		"movaps	%%xmm1,%%xmm3			\n\t"\
		"movaps	%%xmm5,%%xmm7			\n\t"\
		"movq	%[__tmp]   ,%%rax	\n\t"\
		"movq	%[__stride],%%rcx	\n\t"\
		"addpd	    (%%rbx),%%xmm0	\n\t"\
		"addpd	    (%%rdx),%%xmm4	\n\t"\
		"addpd	0x10(%%rbx),%%xmm1	\n\t"\
		"addpd	0x10(%%rdx),%%xmm5	\n\t"\
		"subpd	    (%%rbx),%%xmm2	\n\t"\
		"subpd	    (%%rdx),%%xmm6	\n\t"\
		"subpd	0x10(%%rbx),%%xmm3	\n\t"\
		"subpd	0x10(%%rdx),%%xmm7	\n\t"\
		"movq	%%rax,%%rbx			\n\t"\
		"addq	%%rcx,%%rbx			\n\t"\
		"movq	%%rbx,%%rdx			\n\t"\
		"addq	%%rcx,%%rcx			\n\t"\
		"addq	%%rcx,%%rdx			\n\t"\
		"addq	%%rax,%%rcx			\n\t"\
		"/* Finish radix-4 butterfly and store results into temp-array slots: */\n\t"\
		"subpd	%%xmm4,%%xmm0			\n\t"\
		"subpd	%%xmm7,%%xmm2			\n\t"\
		"subpd	%%xmm5,%%xmm1			\n\t"\
		"subpd	%%xmm6,%%xmm3			\n\t"\
		"movaps	%%xmm0,     (%%rcx)	\n\t"\
		"movaps	%%xmm2,     (%%rdx)	\n\t"\
		"movaps	%%xmm1,0x010(%%rcx)	\n\t"\
		"movaps	%%xmm3,0x010(%%rbx)	\n\t"\
		"addpd	%%xmm4,%%xmm4			\n\t"\
		"addpd	%%xmm7,%%xmm7			\n\t"\
		"addpd	%%xmm5,%%xmm5			\n\t"\
		"addpd	%%xmm6,%%xmm6			\n\t"\
		"addpd	%%xmm0,%%xmm4			\n\t"\
		"addpd	%%xmm2,%%xmm7			\n\t"\
		"addpd	%%xmm1,%%xmm5			\n\t"\
		"addpd	%%xmm3,%%xmm6			\n\t"\
		"movaps	%%xmm4,     (%%rax)	\n\t"\
		"movaps	%%xmm7,     (%%rbx)	\n\t"\
		"movaps	%%xmm5,0x010(%%rax)	\n\t"\
		"movaps	%%xmm6,0x010(%%rdx)	\n\t"\
		:					/* outputs: none */\
		: [__add0] "m" (Xadd0)	/* All inputs from memory addresses here */\
		 ,[__add1] "m" (Xadd1)\
		 ,[__add2] "m" (Xadd2)\
		 ,[__add3] "m" (Xadd3)\
		 ,[__tmp] "m" (Xtmp)\
		 ,[__stride] "e" (Xstride)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

	/* DIT radix-4 subconvolution, sans twiddles, inputs in __i0-3, outputs in __o0-3, possibly coincident with inputs: */
	#define SSE2_RADIX4_DIT_0TWIDDLE_STRIDE_E(Xi0,Xi1,Xi2,Xi3, Xo0,Xo1,Xo2,Xo3)\
	{\
	__asm__ volatile (\
		"movq	%[__i0],%%rax		\n\t"\
		"movq	%[__i1],%%rbx		\n\t"\
		"movq	%[__i2],%%rcx		\n\t"\
		"movq	%[__i3],%%rdx		\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	%%xmm0,%%xmm2			\n\t"\
		"movaps	%%xmm4,%%xmm6			\n\t"\
		"movaps	%%xmm1,%%xmm3			\n\t"\
		"movaps	%%xmm5,%%xmm7			\n\t"\
		"addpd	    (%%rbx),%%xmm0	\n\t"\
		"addpd	    (%%rdx),%%xmm4	\n\t"\
		"addpd	0x10(%%rbx),%%xmm1	\n\t"\
		"addpd	0x10(%%rdx),%%xmm5	\n\t"\
		"subpd	    (%%rbx),%%xmm2	\n\t"\
		"subpd	    (%%rdx),%%xmm6	\n\t"\
		"subpd	0x10(%%rbx),%%xmm3	\n\t"\
		"subpd	0x10(%%rdx),%%xmm7	\n\t"\
		"/* Finish radix-4 butterfly and store results into output-array slots: */\n\t"\
		"movq	%[__o0],%%rax		\n\t"\
		"movq	%[__o1],%%rbx		\n\t"\
		"movq	%[__o2],%%rcx		\n\t"\
		"movq	%[__o3],%%rdx		\n\t"\
		"subpd	%%xmm4,%%xmm0			\n\t"\
		"subpd	%%xmm7,%%xmm2			\n\t"\
		"subpd	%%xmm5,%%xmm1			\n\t"\
		"subpd	%%xmm6,%%xmm3			\n\t"\
		"movaps	%%xmm0,     (%%rcx)	\n\t"\
		"movaps	%%xmm2,     (%%rdx)	\n\t"\
		"movaps	%%xmm1,0x010(%%rcx)	\n\t"\
		"movaps	%%xmm3,0x010(%%rbx)	\n\t"\
		"addpd	%%xmm4,%%xmm4			\n\t"\
		"addpd	%%xmm7,%%xmm7			\n\t"\
		"addpd	%%xmm5,%%xmm5			\n\t"\
		"addpd	%%xmm6,%%xmm6			\n\t"\
		"addpd	%%xmm0,%%xmm4			\n\t"\
		"addpd	%%xmm2,%%xmm7			\n\t"\
		"addpd	%%xmm1,%%xmm5			\n\t"\
		"addpd	%%xmm3,%%xmm6			\n\t"\
		"movaps	%%xmm4,     (%%rax)	\n\t"\
		"movaps	%%xmm7,     (%%rbx)	\n\t"\
		"movaps	%%xmm5,0x010(%%rax)	\n\t"\
		"movaps	%%xmm6,0x010(%%rdx)	\n\t"\
		:					/* outputs: none */\
		: [__i0] "m" (Xi0)	/* All inputs from memory addresses here */\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__i3] "m" (Xi3)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__o3] "m" (Xo3)\
		: "cc","memory","rax","rbx","rcx","rdx","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

	#define SSE2_RADIX_05_DFT_0TWIDDLE(Xi0,Xi1,Xi2,Xi3,Xi4, Xcc1, Xo0,Xo1,Xo2,Xo3,Xo4)\
	{\
	__asm__ volatile (\
		"movq	%[__i0],%%rsi		\n\t"\
		"movq	%[__i1],%%rax		\n\t"\
		"movq	%[__i2],%%rbx		\n\t"\
		"movq	%[__i3],%%rcx		\n\t"\
		"movq	%[__i4],%%rdx		\n\t"\
		"movq	%[__o0],%%rdi		\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t"\
		"movaps	    (%%rbx),%%xmm2	\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t"\
		"movaps	    (%%rdx),%%xmm6	\n\t"\
		"movaps	0x10(%%rdx),%%xmm7	\n\t"\
		"subpd	%%xmm6,%%xmm0		\n\t"\
		"subpd	%%xmm7,%%xmm1		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm0,%%xmm6		\n\t"\
		"addpd	%%xmm1,%%xmm7		\n\t"\
		"subpd	%%xmm4,%%xmm2		\n\t"\
		"subpd	%%xmm5,%%xmm3		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm2,%%xmm4		\n\t"\
		"addpd	%%xmm3,%%xmm5		\n\t"\
	"movq	%[__cc1],%%rax		\n\t"\
		"subpd	%%xmm4,%%xmm6		\n\t"\
		"subpd	%%xmm5,%%xmm7		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t"\
		"addpd	%%xmm6,%%xmm4		\n\t"\
		"addpd	%%xmm7,%%xmm5		\n\t"\
		"addpd	    (%%rsi),%%xmm4	\n\t"\
		"addpd	0x10(%%rsi),%%xmm5	\n\t"\
		"movaps	%%xmm4,    (%%rdi)	\n\t"\
		"movaps	%%xmm5,0x10(%%rdi)	\n\t"\
		"mulpd	0x10(%%rax),%%xmm6	\n\t"\
		"mulpd	0x10(%%rax),%%xmm7	\n\t"\
		"subpd	     (%%rsi),%%xmm4	\n\t"\
		"subpd	0x010(%%rsi),%%xmm5	\n\t"\
		"mulpd	    (%%rax),%%xmm4	\n\t"\
		"mulpd	    (%%rax),%%xmm5	\n\t"\
		"addpd	     (%%rdi),%%xmm4	\n\t"\
		"addpd	0x010(%%rdi),%%xmm5	\n\t"\
		"subpd	%%xmm6,%%xmm4		\n\t"\
		"subpd	%%xmm7,%%xmm5		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t"\
		"addpd	%%xmm4,%%xmm6		\n\t"\
		"addpd	%%xmm5,%%xmm7		\n\t"\
		"movaps	%%xmm4,    (%%rsi)	\n\t"\
		"movaps	%%xmm5,0x10(%%rsi)	\n\t"\
		"movaps	%%xmm0,%%xmm4		\n\t"\
		"movaps	%%xmm1,%%xmm5		\n\t"\
		"subpd	%%xmm2,%%xmm0		\n\t"\
		"subpd	%%xmm3,%%xmm1		\n\t"\
		"mulpd	0x20(%%rax),%%xmm0	\n\t"\
		"mulpd	0x20(%%rax),%%xmm1	\n\t"\
		"mulpd	0x30(%%rax),%%xmm2	\n\t"\
		"mulpd	0x30(%%rax),%%xmm3	\n\t"\
		"mulpd	0x40(%%rax),%%xmm4	\n\t"\
		"mulpd	0x40(%%rax),%%xmm5	\n\t"\
		"addpd	%%xmm0,%%xmm2		\n\t"\
		"addpd	%%xmm1,%%xmm3		\n\t"\
		"subpd	%%xmm4,%%xmm0		\n\t"\
		"subpd	%%xmm5,%%xmm1		\n\t"\
		"movaps	    (%%rsi),%%xmm4	\n\t"\
		"movaps	0x10(%%rsi),%%xmm5	\n\t"\
		"movq	%[__o1],%%rax		\n\t"\
		"movq	%[__o4],%%rdx		\n\t"\
		"subpd	%%xmm3,%%xmm6		\n\t"\
		"subpd	%%xmm2,%%xmm7		\n\t"\
		"addpd	%%xmm3,%%xmm3		\n\t"\
		"addpd	%%xmm2,%%xmm2		\n\t"\
		"movaps	%%xmm6,    (%%rax)	\n\t"\
		"movaps	%%xmm7,0x10(%%rdx)	\n\t"\
		"addpd	%%xmm6,%%xmm3		\n\t"\
		"addpd	%%xmm7,%%xmm2		\n\t"\
		"movaps	%%xmm3,    (%%rdx)	\n\t"\
		"movaps	%%xmm2,0x10(%%rax)	\n\t"\
		"movq	%[__o2],%%rbx		\n\t"\
		"movq	%[__o3],%%rcx		\n\t"\
		"subpd	%%xmm1,%%xmm4		\n\t"\
		"subpd	%%xmm0,%%xmm5		\n\t"\
		"addpd	%%xmm1,%%xmm1		\n\t"\
		"addpd	%%xmm0,%%xmm0		\n\t"\
		"movaps	%%xmm4,    (%%rbx)	\n\t"\
		"movaps	%%xmm5,0x10(%%rcx)	\n\t"\
		"addpd	%%xmm4,%%xmm1		\n\t"\
		"addpd	%%xmm5,%%xmm0		\n\t"\
		"movaps	%%xmm1,    (%%rcx)	\n\t"\
		"movaps	%%xmm0,0x10(%%rbx)	\n\t"\
		"							\n\t"\
		:					/* outputs: none */\
		: [__i0] "m" (Xi0)	/* All inputs from memory addresses here */\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__i3] "m" (Xi3)\
		 ,[__i4] "m" (Xi4)\
		 ,[__cc1] "m" (Xcc1)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__o3] "m" (Xo3)\
		 ,[__o4] "m" (Xo4)\
		: "cc","memory","rax","rbx","rcx","rdx","rdi","rsi","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7"		/* Clobbered registers */\
	);\
	}

	/* 16-xmm-register version does 2 of the above side-by-side: */
	#define SSE2_RADIX_05_DFT_0TWIDDLE_X2(Xcc1,Xtwo, Xi0,Xi1,Xi2,Xi3,Xi4, Xo0,Xo1,Xo2,Xo3,Xo4, Xj0,Xj1,Xj2,Xj3,Xj4, Xu0,Xu1,Xu2,Xu3,Xu4)\
	{\
	__asm__ volatile (\
		"movq	%[__i0],%%rsi		\n\t	movq	%[__j0],%%r10		\n\t"\
		"movq	%[__i1],%%rax		\n\t	movq	%[__j1],%%r11		\n\t"\
		"movq	%[__i2],%%rbx		\n\t	movq	%[__j2],%%r12		\n\t"\
		"movq	%[__i3],%%rcx		\n\t	movq	%[__j3],%%r13		\n\t"\
		"movq	%[__i4],%%rdx		\n\t	movq	%[__j4],%%r14		\n\t"\
		"movq	%[__o0],%%rdi		\n\t	movq	%[__u0],%%r15		\n\t"\
		"movaps	    (%%rax),%%xmm0	\n\t	movaps	    (%%r11),%%xmm8 	\n\t"\
		"movaps	0x10(%%rax),%%xmm1	\n\t	movaps	0x10(%%r11),%%xmm9 	\n\t"\
		"movaps	    (%%rbx),%%xmm2	\n\t	movaps	    (%%r12),%%xmm10	\n\t"\
		"movaps	0x10(%%rbx),%%xmm3	\n\t	movaps	0x10(%%r12),%%xmm11	\n\t"\
		"movaps	    (%%rcx),%%xmm4	\n\t	movaps	    (%%r13),%%xmm12	\n\t"\
		"movaps	0x10(%%rcx),%%xmm5	\n\t	movaps	0x10(%%r13),%%xmm13	\n\t"\
		"movaps	    (%%rdx),%%xmm6	\n\t	movaps	    (%%r14),%%xmm14	\n\t"\
		"movaps	0x10(%%rdx),%%xmm7	\n\t	movaps	0x10(%%r14),%%xmm15	\n\t"\
		"subpd	%%xmm6,%%xmm0		\n\t	subpd	%%xmm14,%%xmm8 		\n\t"\
		"subpd	%%xmm7,%%xmm1		\n\t	subpd	%%xmm15,%%xmm9 		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t	addpd	%%xmm14,%%xmm14		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t	addpd	%%xmm15,%%xmm15		\n\t"\
		"addpd	%%xmm0,%%xmm6		\n\t	addpd	%%xmm8 ,%%xmm14		\n\t"\
		"addpd	%%xmm1,%%xmm7		\n\t	addpd	%%xmm9 ,%%xmm15		\n\t"\
		"subpd	%%xmm4,%%xmm2		\n\t	subpd	%%xmm12,%%xmm10		\n\t"\
		"subpd	%%xmm5,%%xmm3		\n\t	subpd	%%xmm13,%%xmm11		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t	addpd	%%xmm12,%%xmm12		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t	addpd	%%xmm13,%%xmm13		\n\t"\
		"addpd	%%xmm2,%%xmm4		\n\t	addpd	%%xmm10,%%xmm12		\n\t"\
		"addpd	%%xmm3,%%xmm5		\n\t	addpd	%%xmm11,%%xmm13		\n\t"\
		"movq	%[__cc1],%%rax		\n\t"\
		"subpd	%%xmm4,%%xmm6		\n\t	subpd	%%xmm12,%%xmm14		\n\t"\
		"subpd	%%xmm5,%%xmm7		\n\t	subpd	%%xmm13,%%xmm15		\n\t"\
		"addpd	%%xmm4,%%xmm4		\n\t	addpd	%%xmm12,%%xmm12		\n\t"\
		"addpd	%%xmm5,%%xmm5		\n\t	addpd	%%xmm13,%%xmm13		\n\t"\
		"addpd	%%xmm6,%%xmm4		\n\t	addpd	%%xmm14,%%xmm12		\n\t"\
		"addpd	%%xmm7,%%xmm5		\n\t	addpd	%%xmm15,%%xmm13		\n\t"\
		"addpd	    (%%rsi),%%xmm4	\n\t	addpd	    (%%r10),%%xmm12	\n\t"\
		"addpd	0x10(%%rsi),%%xmm5	\n\t	addpd	0x10(%%r10),%%xmm13	\n\t"\
		"movaps	%%xmm4,    (%%rdi)	\n\t	movaps	%%xmm12,    (%%r15)	\n\t"\
		"movaps	%%xmm5,0x10(%%rdi)	\n\t	movaps	%%xmm13,0x10(%%r15)	\n\t"\
		"mulpd	0x10(%%rax),%%xmm6	\n\t	mulpd	0x10(%%rax),%%xmm14	\n\t"\
		"mulpd	0x10(%%rax),%%xmm7	\n\t	mulpd	0x10(%%rax),%%xmm15	\n\t"\
		"subpd	    (%%rsi),%%xmm4	\n\t	subpd	    (%%r10),%%xmm12	\n\t"\
		"subpd	0x10(%%rsi),%%xmm5	\n\t	subpd	0x10(%%r10),%%xmm13	\n\t"\
		"mulpd	    (%%rax),%%xmm4	\n\t	mulpd	    (%%rax),%%xmm12	\n\t"\
		"mulpd	    (%%rax),%%xmm5	\n\t	mulpd	    (%%rax),%%xmm13	\n\t"\
		"addpd	    (%%rdi),%%xmm4	\n\t	addpd	    (%%r15),%%xmm12	\n\t"\
		"addpd	0x10(%%rdi),%%xmm5	\n\t	addpd	0x10(%%r15),%%xmm13	\n\t"\
		"subpd	%%xmm6,%%xmm4		\n\t	subpd	%%xmm14,%%xmm12		\n\t"\
		"subpd	%%xmm7,%%xmm5		\n\t	subpd	%%xmm15,%%xmm13		\n\t"\
		"addpd	%%xmm6,%%xmm6		\n\t	addpd	%%xmm14,%%xmm14		\n\t"\
		"addpd	%%xmm7,%%xmm7		\n\t	addpd	%%xmm15,%%xmm15		\n\t"\
		"addpd	%%xmm4,%%xmm6		\n\t	addpd	%%xmm12,%%xmm14		\n\t"\
		"addpd	%%xmm5,%%xmm7		\n\t	addpd	%%xmm13,%%xmm15		\n\t"\
		"movaps	%%xmm4,    (%%rsi)	\n\t	movaps	%%xmm12,    (%%r10)	\n\t"\
		"movaps	%%xmm5,0x10(%%rsi)	\n\t	movaps	%%xmm13,0x10(%%r10)	\n\t"\
		"movaps	%%xmm0,%%xmm4		\n\t	movaps	%%xmm8 ,%%xmm12		\n\t"\
		"movaps	%%xmm1,%%xmm5		\n\t	movaps	%%xmm9 ,%%xmm13		\n\t"\
		"subpd	%%xmm2,%%xmm0		\n\t	subpd	%%xmm10,%%xmm8 		\n\t"\
		"subpd	%%xmm3,%%xmm1		\n\t	subpd	%%xmm11,%%xmm9 		\n\t"\
		"mulpd	0x20(%%rax),%%xmm0	\n\t	mulpd	0x20(%%rax),%%xmm8 	\n\t"\
		"mulpd	0x20(%%rax),%%xmm1	\n\t	mulpd	0x20(%%rax),%%xmm9 	\n\t"\
		"mulpd	0x30(%%rax),%%xmm2	\n\t	mulpd	0x30(%%rax),%%xmm10	\n\t"\
		"mulpd	0x30(%%rax),%%xmm3	\n\t	mulpd	0x30(%%rax),%%xmm11	\n\t"\
		"mulpd	0x40(%%rax),%%xmm4	\n\t	mulpd	0x40(%%rax),%%xmm12	\n\t"\
		"mulpd	0x40(%%rax),%%xmm5	\n\t	mulpd	0x40(%%rax),%%xmm13	\n\t"\
		"addpd	%%xmm0,%%xmm2		\n\t	addpd	%%xmm8 ,%%xmm10		\n\t"\
		"addpd	%%xmm1,%%xmm3		\n\t	addpd	%%xmm9 ,%%xmm11		\n\t"\
		"subpd	%%xmm4,%%xmm0		\n\t	subpd	%%xmm12,%%xmm8 		\n\t"\
		"subpd	%%xmm5,%%xmm1		\n\t	subpd	%%xmm13,%%xmm9 		\n\t"\
		"movaps	    (%%rsi),%%xmm4	\n\t	movaps	    (%%r10),%%xmm12	\n\t"\
		"movaps	0x10(%%rsi),%%xmm5	\n\t	movaps	0x10(%%r10),%%xmm13	\n\t"\
		"movq	%[__o1],%%rax		\n\t	movq	%[__u1],%%r11		\n\t"\
		"movq	%[__o4],%%rdx		\n\t	movq	%[__u4],%%r14		\n\t"\
		"subpd	%%xmm3,%%xmm6		\n\t	subpd	%%xmm11,%%xmm14		\n\t"\
		"subpd	%%xmm2,%%xmm7		\n\t	subpd	%%xmm10,%%xmm15		\n\t"\
		"addpd	%%xmm3,%%xmm3		\n\t	addpd	%%xmm11,%%xmm11		\n\t"\
		"addpd	%%xmm2,%%xmm2		\n\t	addpd	%%xmm10,%%xmm10		\n\t"\
		"movaps	%%xmm6,    (%%rax)	\n\t	movaps	%%xmm14,    (%%r11)	\n\t"\
		"movaps	%%xmm7,0x10(%%rdx)	\n\t	movaps	%%xmm15,0x10(%%r14)	\n\t"\
		"addpd	%%xmm6,%%xmm3		\n\t	addpd	%%xmm14,%%xmm11		\n\t"\
		"addpd	%%xmm7,%%xmm2		\n\t	addpd	%%xmm15,%%xmm10		\n\t"\
		"movaps	%%xmm3,    (%%rdx)	\n\t	movaps	%%xmm11,    (%%r14)	\n\t"\
		"movaps	%%xmm2,0x10(%%rax)	\n\t	movaps	%%xmm10,0x10(%%r11)	\n\t"\
		"movq	%[__o2],%%rbx		\n\t	movq	%[__u2],%%r12		\n\t"\
		"movq	%[__o3],%%rcx		\n\t	movq	%[__u3],%%r13		\n\t"\
		"subpd	%%xmm1,%%xmm4		\n\t	subpd	%%xmm9 ,%%xmm12		\n\t"\
		"subpd	%%xmm0,%%xmm5		\n\t	subpd	%%xmm8 ,%%xmm13		\n\t"\
		"addpd	%%xmm1,%%xmm1		\n\t	addpd	%%xmm9 ,%%xmm9 		\n\t"\
		"addpd	%%xmm0,%%xmm0		\n\t	addpd	%%xmm8 ,%%xmm8 		\n\t"\
		"movaps	%%xmm4,    (%%rbx)	\n\t	movaps	%%xmm12,    (%%r12)	\n\t"\
		"movaps	%%xmm5,0x10(%%rcx)	\n\t	movaps	%%xmm13,0x10(%%r13)	\n\t"\
		"addpd	%%xmm4,%%xmm1		\n\t	addpd	%%xmm12,%%xmm9 		\n\t"\
		"addpd	%%xmm5,%%xmm0		\n\t	addpd	%%xmm13,%%xmm8 		\n\t"\
		"movaps	%%xmm1,    (%%rcx)	\n\t	movaps	%%xmm9 ,    (%%r13)	\n\t"\
		"movaps	%%xmm0,0x10(%%rbx)	\n\t	movaps	%%xmm8 ,0x10(%%r12)	\n\t"\
		:					/* outputs: none */\
		: [__cc1] "m" (Xcc1)	/* All inputs from memory addresses here */\
		 ,[__two] "m" (Xtwo)\
		 ,[__i0] "m" (Xi0)\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__i3] "m" (Xi3)\
		 ,[__i4] "m" (Xi4)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__o3] "m" (Xo3)\
		 ,[__o4] "m" (Xo4)\
		 ,[__j0] "m" (Xj0)\
		 ,[__j1] "m" (Xj1)\
		 ,[__j2] "m" (Xj2)\
		 ,[__j3] "m" (Xj3)\
		 ,[__j4] "m" (Xj4)\
		 ,[__u0] "m" (Xu0)\
		 ,[__u1] "m" (Xu1)\
		 ,[__u2] "m" (Xu2)\
		 ,[__u3] "m" (Xu3)\
		 ,[__u4] "m" (Xu4)\
		: "cc","memory","rax","rbx","rcx","rdx","rdi","rsi","r10","r11","r12","r13","r14","r15","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

	/*...Radix-7 DFT: Inputs in memlocs __i0-6, outputs into __o0-6, possibly coincident with inputs:\ */\
	#define SSE2_RADIX_07_DFT(Xi0,Xi1,Xi2,Xi3,Xi4,Xi5,Xi6, Xcc, Xo0,Xo1,Xo2,Xo3,Xo4,Xo5,Xo6)\
	{\
	__asm__ volatile (\
		"movq	%[__i1],%%rax		\n\t"\
		"movq	%[__i2],%%rbx		\n\t"\
		"movq	%[__i3],%%rcx		\n\t"\
		"movq	%[__i4],%%rdx		\n\t"\
		"movq	%[__i5],%%rsi		\n\t"\
		"movq	%[__i6],%%rdi		\n\t	/*** Imaginary Parts: ***/	\n\t"\
		"movaps	(%%rax),%%xmm6		\n\t	movaps	0x10(%%rax),%%xmm14	\n\t"\
		"movaps	(%%rdi),%%xmm1		\n\t	movaps	0x10(%%rdi),%%xmm9 	\n\t"\
		"movaps	(%%rbx),%%xmm5		\n\t	movaps	0x10(%%rbx),%%xmm13	\n\t"\
		"movaps	(%%rsi),%%xmm2		\n\t	movaps	0x10(%%rsi),%%xmm10	\n\t"\
		"movaps	(%%rcx),%%xmm4		\n\t	movaps	0x10(%%rcx),%%xmm12	\n\t"\
		"movaps	(%%rdx),%%xmm3		\n\t	movaps	0x10(%%rdx),%%xmm11	\n\t"\
		"movq	%[__i0],%%rbx		\n\t"\
		"subpd	%%xmm1,%%xmm6		\n\t	subpd	%%xmm9 ,%%xmm14		\n\t"\
		"addpd	%%xmm1,%%xmm1		\n\t	addpd	%%xmm9 ,%%xmm9 		\n\t"\
		"addpd	%%xmm6,%%xmm1		\n\t	addpd	%%xmm14,%%xmm9  	\n\t"\
		"subpd	%%xmm2,%%xmm5		\n\t	subpd	%%xmm10,%%xmm13		\n\t"\
		"addpd	%%xmm2,%%xmm2		\n\t	addpd	%%xmm10,%%xmm10		\n\t"\
		"addpd	%%xmm5,%%xmm2		\n\t	addpd	%%xmm13,%%xmm10		\n\t"\
		"movaps	(%%rbx),%%xmm0		\n\t	movaps	0x10(%%rbx),%%xmm8 	\n\t"\
		"subpd	%%xmm3,%%xmm4		\n\t	subpd	%%xmm11,%%xmm12		\n\t"\
		"addpd	%%xmm3,%%xmm3		\n\t	addpd	%%xmm11,%%xmm11		\n\t"\
		"addpd	%%xmm4,%%xmm3		\n\t	addpd	%%xmm12,%%xmm11		\n\t"\
		"\n\t"\
		"movq	%[__o0],%%rcx		\n\t"\
		"movq	%[__cc],%%rsi		\n\t"\
		"movaps	%%xmm0,0x80(%%rsi)	\n\t	movaps	%%xmm8 ,0xa0(%%rsi)	\n\t"\
		"movaps	%%xmm6,0x90(%%rsi)	\n\t	movaps	%%xmm14,0xb0(%%rsi)	\n\t"\
		"addpd	%%xmm1,%%xmm0		\n\t	addpd	%%xmm9 ,%%xmm8  	\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t	movaps	%%xmm13,%%xmm15		\n\t"\
		"addpd	%%xmm2,%%xmm3		\n\t	addpd	%%xmm10,%%xmm11		\n\t"\
		"subpd	%%xmm4,%%xmm5		\n\t	subpd	%%xmm12,%%xmm13		\n\t"\
		"subpd	%%xmm2,%%xmm1		\n\t	subpd	%%xmm10,%%xmm9  	\n\t"\
		"subpd	%%xmm7,%%xmm6		\n\t	subpd	%%xmm15,%%xmm14		\n\t"\
		"addpd	%%xmm2,%%xmm2		\n\t	addpd	%%xmm10,%%xmm10		\n\t"\
		"addpd	%%xmm7,%%xmm4		\n\t	addpd	%%xmm15,%%xmm12		\n\t"\
		"addpd	%%xmm3,%%xmm0		\n\t	addpd	%%xmm11,%%xmm8  	\n\t"\
		"addpd	0x90(%%rsi),%%xmm5	\n\t	addpd	0xb0(%%rsi),%%xmm13	\n\t"\
		"subpd	%%xmm2,%%xmm3		\n\t	subpd	%%xmm10,%%xmm11		\n\t"\
		"movaps	%%xmm4,%%xmm7		\n\t	movaps	%%xmm12,%%xmm15		\n\t"\
		"movaps	%%xmm0,    (%%rcx)	\n\t	movaps	%%xmm8 ,0x10(%%rcx)	\n\t"/* B0 */\
		"subpd	%%xmm6,%%xmm4		\n\t	subpd	%%xmm14,%%xmm12		\n\t"\
		"movaps	%%xmm1,%%xmm2		\n\t	movaps	%%xmm9 ,%%xmm10		\n\t"\
		"subpd	0x80(%%rsi),%%xmm0	\n\t	subpd	0xa0(%%rsi),%%xmm8  \n\t"\
		"mulpd	0x10(%%rsi),%%xmm5	\n\t	mulpd	0x10(%%rsi),%%xmm13	\n\t"\
		"addpd	%%xmm3,%%xmm2		\n\t	addpd	%%xmm11,%%xmm10		\n\t"\
		"mulpd	0x40(%%rsi),%%xmm3	\n\t	mulpd	0x40(%%rsi),%%xmm11	\n\t"\
		"mulpd	0x70(%%rsi),%%xmm4	\n\t	mulpd	0x70(%%rsi),%%xmm12	\n\t"\
		"mulpd	0x20(%%rsi),%%xmm1	\n\t	mulpd	0x20(%%rsi),%%xmm9  \n\t"\
		"mulpd	0x30(%%rsi),%%xmm6	\n\t	mulpd	0x30(%%rsi),%%xmm14	\n\t"\
		"mulpd	    (%%rsi),%%xmm0	\n\t	mulpd	    (%%rsi),%%xmm8  \n\t"\
		"mulpd	0x50(%%rsi),%%xmm7	\n\t	mulpd	0x50(%%rsi),%%xmm15	\n\t"\
		"mulpd	0x60(%%rsi),%%xmm2	\n\t	mulpd	0x60(%%rsi),%%xmm10	\n\t"\
		"addpd	    (%%rcx),%%xmm0	\n\t	addpd	0x10(%%rcx),%%xmm8  \n\t"\
		"addpd	%%xmm4,%%xmm6		\n\t	addpd	%%xmm12,%%xmm14		\n\t"\
		"subpd	%%xmm2,%%xmm1		\n\t	subpd	%%xmm10,%%xmm9  	\n\t"\
		"subpd	%%xmm7,%%xmm4		\n\t	subpd	%%xmm15,%%xmm12		\n\t"\
		"subpd	%%xmm2,%%xmm3		\n\t	subpd	%%xmm10,%%xmm11		\n\t"\
		"movq	%[__o1],%%rax		\n\t"\
		"movq	%[__o2],%%rbx		\n\t"\
		"movq	%[__o3],%%rcx		\n\t"\
		"movq	%[__o4],%%rdx		\n\t"\
		"movq	%[__o5],%%rsi		\n\t"\
		"movq	%[__o6],%%rdi		\n\t"\
		"movaps	%%xmm0,%%xmm2		\n\t	movaps	%%xmm8 ,%%xmm10		\n\t"\
		"movaps	%%xmm5,%%xmm7		\n\t	movaps	%%xmm13,%%xmm15		\n\t"\
		"addpd	%%xmm1,%%xmm0		\n\t	addpd	%%xmm9 ,%%xmm8  	\n\t"\
		"addpd	%%xmm6,%%xmm5		\n\t	addpd	%%xmm14,%%xmm13		\n\t"\
		"addpd	%%xmm3,%%xmm1		\n\t	addpd	%%xmm11,%%xmm9  	\n\t"\
		"addpd	%%xmm4,%%xmm6		\n\t	addpd	%%xmm12,%%xmm14		\n\t"\
		"addpd	%%xmm2,%%xmm3		\n\t	addpd	%%xmm10,%%xmm11		\n\t"\
		"addpd	%%xmm7,%%xmm4		\n\t	addpd	%%xmm15,%%xmm12		\n\t"\
		"subpd	%%xmm1,%%xmm2		\n\t	subpd	%%xmm9 ,%%xmm10		\n\t"\
		"subpd	%%xmm6,%%xmm7		\n\t	subpd	%%xmm14,%%xmm15		\n\t"\
		"/* xmm1,6,9,14 free ... Note the order reversal on the 3rd pair of outputs: */\n\t"\
		"subpd	%%xmm13,%%xmm0		\n\t	subpd	%%xmm15,%%xmm2		\n\t	subpd	%%xmm12,%%xmm3 		\n\t"\
		"subpd	%%xmm5 ,%%xmm8  	\n\t	subpd	%%xmm7 ,%%xmm10		\n\t	subpd	%%xmm4 ,%%xmm11		\n\t"\
		"addpd	%%xmm13,%%xmm13		\n\t	addpd	%%xmm15,%%xmm15		\n\t	addpd	%%xmm12,%%xmm12		\n\t"\
		"addpd	%%xmm5 ,%%xmm5		\n\t	addpd	%%xmm7 ,%%xmm7		\n\t	addpd	%%xmm4 ,%%xmm4 		\n\t"\
		"addpd	%%xmm0 ,%%xmm13		\n\t	addpd	%%xmm2 ,%%xmm15		\n\t	addpd	%%xmm3 ,%%xmm12		\n\t"\
		"addpd	%%xmm8 ,%%xmm5		\n\t	addpd	%%xmm10,%%xmm7		\n\t	addpd	%%xmm11,%%xmm4 		\n\t"\
		"movaps	%%xmm0 ,    (%%rax)	\n\t	movaps	%%xmm2 ,    (%%rbx)	\n\t	movaps	%%xmm3 ,    (%%rdx)	\n\t"/* B124r */\
		"movaps	%%xmm8 ,0x10(%%rdi)	\n\t	movaps	%%xmm10,0x10(%%rsi)	\n\t	movaps	%%xmm11,0x10(%%rcx)	\n\t"/* B653i */\
		"movaps	%%xmm13,    (%%rdi)	\n\t	movaps	%%xmm15,    (%%rsi)	\n\t	movaps	%%xmm12,    (%%rcx)	\n\t"/* B653r */\
		"movaps	%%xmm5 ,0x10(%%rax)	\n\t	movaps	%%xmm7 ,0x10(%%rbx)	\n\t	movaps	%%xmm4 ,0x10(%%rdx)	\n\t"/* B124i */\
		"\n\t"\
		:					/* outputs: none */\
		: [__i0] "m" (Xi0)	/* All inputs from memory addresses here */\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__i3] "m" (Xi3)\
		 ,[__i4] "m" (Xi4)\
		 ,[__i5] "m" (Xi5)\
		 ,[__i6] "m" (Xi6)\
		 ,[__cc] "m" (Xcc)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__o3] "m" (Xo3)\
		 ,[__o4] "m" (Xo4)\
		 ,[__o5] "m" (Xo5)\
		 ,[__o6] "m" (Xo6)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","rdi","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

	/* Twiddleless version of SSE2_RADIX8_DIF_TWIDDLE. Inputs enter in memory locations __r0 + [__i1,__i2,__i3,__i4,__i5,__i6,__i7],;
	where r0 is a memory address and the i's are LITERAL [BYTE] OFFSETS. Outputs go into memory locations __o0,__o1,__o2,__o3,__o4,__o5,__o6,__o7, assumed disjoint with inputs:\
	*/
	#define SSE2_RADIX8_DIF_0TWIDDLE(Xr0, Xi1,Xi2,Xi3,Xi4,Xi5,Xi6,Xi7, Xo0,Xo1,Xo2,Xo3,Xo4,Xo5,Xo6,Xo7, Xisrt2)\
	{\
	__asm__ volatile (\
	/* 1st of 2 radix-4 subtransforms, data in xmm0-7: **** 2nd of 2 radix-4 subtransforms, data in xmm8-15: */\
		"movq	%[__r0],%%rax	/* i0 = r00 */	\n\t			leaq	%c[__i1](%%rax),%%r10			\n\t"\
		"leaq	%c[__i2](%%rax),%%rbx			\n\t			leaq	%c[__i3](%%rax),%%r11			\n\t"\
		"leaq	%c[__i4](%%rax),%%rcx			\n\t			leaq	%c[__i5](%%rax),%%r12			\n\t"\
		"leaq	%c[__i6](%%rax),%%rdx			\n\t			leaq	%c[__i7](%%rax),%%r13			\n\t"\
		/* p0,4 combo: x+y into xmm0/1, x-y in xmm2/3: **** p1,5 combo: x+y into xmm8/1, x-y in xmm10/3: */\
		"							movq %[__isrt2],%%rsi \n\t	movaps	    (%%r12),%%xmm8 				\n\t"\
		"										\n\t			movaps	0x10(%%r12),%%xmm9 				\n\t"\
		"movaps	    (%%rcx),%%xmm0				\n\t			movaps	    (%%r10),%%xmm10				\n\t"\
		"movaps	0x10(%%rcx),%%xmm1				\n\t			movaps	0x10(%%r10),%%xmm11				\n\t"\
		"movaps	    (%%rax),%%xmm2				\n\t			subpd	%%xmm8 ,%%xmm10					\n\t"\
		"movaps	0x10(%%rax),%%xmm3				\n\t			subpd	%%xmm9 ,%%xmm11					\n\t"\
		"subpd	%%xmm0,%%xmm2					\n\t			addpd	%%xmm8 ,%%xmm8 					\n\t"\
		"subpd	%%xmm1,%%xmm3					\n\t			addpd	%%xmm9 ,%%xmm9 					\n\t"\
		"addpd	%%xmm0,%%xmm0					\n\t			addpd	%%xmm10,%%xmm8 					\n\t"\
		"addpd	%%xmm1,%%xmm1					\n\t			addpd	%%xmm11,%%xmm9 					\n\t"\
		"addpd	%%xmm2,%%xmm0					\n\t"	/* p3,7 combo: x+y into xmm14/7, x-y in xmm12/5: */\
		"addpd	%%xmm3,%%xmm1					\n\t			movaps	    (%%r11),%%xmm12				\n\t"\
		"										\n\t			movaps	0x10(%%r11),%%xmm13				\n\t"\
		/* p2,6 combo: x+y into xmm4/5, x-y in xmm6/7: */"\n\t	movaps	    (%%r13),%%xmm14				\n\t"\
		"										\n\t			movaps	0x10(%%r13),%%xmm15				\n\t"\
		"movaps	    (%%rdx),%%xmm4				\n\t			subpd	%%xmm14,%%xmm12					\n\t"\
		"movaps	0x10(%%rdx),%%xmm5				\n\t			subpd	%%xmm15,%%xmm13					\n\t"\
		"movaps	    (%%rbx),%%xmm6				\n\t			addpd	%%xmm14,%%xmm14					\n\t"\
		"movaps	0x10(%%rbx),%%xmm7				\n\t			addpd	%%xmm15,%%xmm15					\n\t"\
		"subpd	%%xmm4,%%xmm6					\n\t			addpd	%%xmm12,%%xmm14					\n\t"\
		"subpd	%%xmm5,%%xmm7					\n\t			addpd	%%xmm13,%%xmm15					\n\t"\
		"addpd	%%xmm4,%%xmm4					\n\t"	/* Finish radix-4 butterfly, tmp-store 1st of 4 outputs to free up 2 registers: */\
		"addpd	%%xmm5,%%xmm5					\n\t			subpd	%%xmm14,%%xmm8 					\n\t"\
		"addpd	%%xmm6,%%xmm4					\n\t			subpd	%%xmm15,%%xmm9 					\n\t"\
		"addpd	%%xmm7,%%xmm5					\n\t			subpd	%%xmm13,%%xmm10					\n\t"\
		"										\n\t			subpd	%%xmm12,%%xmm11					\n\t"\
		"subpd	%%xmm4,%%xmm0					\n\t			addpd	%%xmm14,%%xmm14					\n\t"\
		"subpd	%%xmm7,%%xmm2					\n\t			addpd	%%xmm13,%%xmm13					\n\t"\
		"subpd	%%xmm5,%%xmm1					\n\t			addpd	%%xmm15,%%xmm15					\n\t"\
		"subpd	%%xmm6,%%xmm3					\n\t			addpd	%%xmm12,%%xmm12					\n\t"\
		"														addpd	%%xmm8 ,%%xmm14					\n\t"\
		"														addpd	%%xmm10,%%xmm13					\n\t"\
		"														addpd	%%xmm9 ,%%xmm15					\n\t"\
		"														addpd	%%xmm11,%%xmm12					\n\t"\
		"addpd	%%xmm4,%%xmm4					\n\t			movaps	%%xmm14,    (%%r10)				\n\t"\
		"addpd	%%xmm7,%%xmm7					\n\t			movaps	%%xmm15,0x10(%%r10)				\n\t"\
		"addpd	%%xmm5,%%xmm5					\n\t			movaps	%%xmm10,%%xmm14					\n\t"\
		"addpd	%%xmm6,%%xmm6					\n\t			movaps	%%xmm13,%%xmm15					\n\t"\
		"addpd	%%xmm0,%%xmm4					\n\t			subpd	%%xmm12,%%xmm10					\n\t"\
		"addpd	%%xmm2,%%xmm7					\n\t			subpd	%%xmm11,%%xmm13					\n\t"\
		"addpd	%%xmm1,%%xmm5					\n\t			addpd	%%xmm14,%%xmm12					\n\t"\
		"addpd	%%xmm3,%%xmm6					\n\t			addpd	%%xmm15,%%xmm11					\n\t"\
		"														movaps	(%%rsi),%%xmm14		\n\t"/* isrt2 */\
		"														mulpd	%%xmm14,%%xmm10					\n\t"\
		"														mulpd	%%xmm14,%%xmm13					\n\t"\
		"														mulpd	%%xmm14,%%xmm12					\n\t"\
		"														mulpd	%%xmm14,%%xmm11					\n\t"\
		"movaps	    (%%r10),%%xmm14	\n\t"/* restore spilled */\
		"movaps	0x10(%%r10),%%xmm15	\n\t"/* restore spilled */\
		"\n\t"\
	/* Inline of SSE2_RADIX8_DIF_COMBINE_RAD4_SUBS_A(r0): Combine radix-4 subtransforms and write outputs: */\
		/***** t0,1,2,3,4,5,6,7 in xmm[ 4, 5| 2, 6| 0, 1| 7, 3] *****/\
		/***** t8,9,a,b,c,d,e,f in xmm[14,15|10,12| 8, 9|13,11] *****/\
		"\n\t"\
		"\n\t"\
		"movq	%[__o4],%%rax					\n\t			subpd   %%xmm10,%%xmm2			\n\t"\
		"movq	%[__o5],%%rbx					\n\t			subpd   %%xmm12,%%xmm6			\n\t"\
		"movq	%[__o6],%%rcx					\n\t			addpd   %%xmm10,%%xmm10			\n\t"\
		"movq	%[__o7],%%rdx					\n\t			addpd   %%xmm12,%%xmm12			\n\t"\
		"subpd   %%xmm11,%%xmm7					\n\t			addpd   %%xmm2,%%xmm10			\n\t"\
		"subpd   %%xmm13,%%xmm3					\n\t			addpd   %%xmm6,%%xmm12			\n\t"\
		"addpd   %%xmm11,%%xmm11				\n\t			movaps	%%xmm2 ,    (%%rbx)		\n\t"/* o5r */\
		"addpd   %%xmm13,%%xmm13				\n\t			movaps	%%xmm6 ,0x10(%%rbx)		\n\t"/* o5i */\
		"addpd   %%xmm7,%%xmm11					\n\t			movaps	%%xmm10,    (%%rax)		\n\t"/* o4r */\
		"addpd   %%xmm3,%%xmm13					\n\t			movaps	%%xmm12,0x10(%%rax)		\n\t"/* o4i */\
		"movaps	%%xmm7 ,    (%%rcx)		\n\t"/* o6r */\
		"movaps	%%xmm3 ,0x10(%%rdx)		\n\t"/* o7i */\
		"movaps	%%xmm11,    (%%rdx)		\n\t"/* o7r */\
		"movaps	%%xmm13,0x10(%%rcx)		\n\t"/* o6i */\
		"movq	%[__o0],%%rax					\n\t"\
		"movq	%[__o1],%%rbx					\n\t"\
		"movq	%[__o2],%%rcx					\n\t"\
		"movq	%[__o3],%%rdx					\n\t"\
		"subpd	%%xmm14,%%xmm4 					\n\t"\
		"subpd	%%xmm15,%%xmm5 					\n\t"\
		"subpd	%%xmm9 ,%%xmm0 					\n\t"\
		"subpd	%%xmm8 ,%%xmm1 					\n\t"\
		"addpd	%%xmm14,%%xmm14					\n\t			movaps	%%xmm4 ,    (%%rbx)		\n\t"/* o1r */\
		"addpd	%%xmm15,%%xmm15					\n\t			movaps	%%xmm5 ,0x10(%%rbx)		\n\t"/* o1i */\
		"addpd	%%xmm9 ,%%xmm9 					\n\t			movaps	%%xmm0 ,    (%%rcx)		\n\t"/* o2r */\
		"addpd	%%xmm8 ,%%xmm8 					\n\t			movaps	%%xmm1 ,0x10(%%rdx)		\n\t"/* o3i */\
		"addpd	%%xmm4 ,%%xmm14					\n\t"\
		"addpd	%%xmm5 ,%%xmm15					\n\t"\
		"addpd	%%xmm0 ,%%xmm9 					\n\t"\
		"addpd	%%xmm1 ,%%xmm8 					\n\t"\
		"movaps	%%xmm14,    (%%rax)		\n\t"/* o0r */\
		"movaps	%%xmm15,0x10(%%rax)		\n\t"/* o0r */\
		"movaps	%%xmm9 ,    (%%rdx)		\n\t"/* o3r */\
		"movaps	%%xmm8 ,0x10(%%rcx)		\n\t"/* o2i */\
		:					/* outputs: none */\
		: [__r0] "m" (Xr0)	/* All inputs from memory addresses here */\
		 ,[__i1] "e" (Xi1)\
		 ,[__i2] "e" (Xi2)\
		 ,[__i3] "e" (Xi3)\
		 ,[__i4] "e" (Xi4)\
		 ,[__i5] "e" (Xi5)\
		 ,[__i6] "e" (Xi6)\
		 ,[__i7] "e" (Xi7)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__o3] "m" (Xo3)\
		 ,[__o4] "m" (Xo4)\
		 ,[__o5] "m" (Xo5)\
		 ,[__o6] "m" (Xo6)\
		 ,[__o7] "m" (Xo7)\
		 ,[__isrt2] "m" (Xisrt2)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

	// Need a 2nd version of above which takes the i-strides as intvars rather than literal bytes:
	#define SSE2_RADIX8_DIF_0TWIDDLE_B(Xr0, Xi1,Xi2,Xi3,Xi4,Xi5,Xi6,Xi7, Xo0,Xo1,Xo2,Xo3,Xo4,Xo5,Xo6,Xo7, Xisrt2)\
	{\
	__asm__ volatile (\
		"/* 1st of 2 radix-4 subtransforms, data in xmm0-7: */\n\t	/* 2nd of 2 radix-4 subtransforms, data in xmm8-15: */\n\t"\
		"movq	%[__r0],%%rax	/* i0 = r00 */	\n\t			movslq	%[__i1],%%r10		/* i1 */	\n\t"\
		"movslq	%[__i2],%%rbx	/* i2 */		\n\t			movslq	%[__i3],%%r11		/* i3 */	\n\t"\
		"movslq	%[__i4],%%rcx	/* i4 */		\n\t			movslq	%[__i5],%%r12		/* i5 */	\n\t"\
		"movslq	%[__i6],%%rdx	/* i6 */		\n\t			movslq	%[__i7],%%r13		/* i7 */	\n\t"\
		"addq	%%rax,%%rbx						\n\t			addq	%%rax,%%r10						\n\t"\
		"addq	%%rax,%%rcx						\n\t			addq	%%rax,%%r11						\n\t"\
		"addq	%%rax,%%rdx						\n\t			addq	%%rax,%%r12						\n\t"\
		"movq	%[__isrt2],%%rsi				\n\t			addq	%%rax,%%r13						\n\t"\
		"										\n\t			/* p1,5 combo: x+y into xmm8 /1, x-y in xmm10/3: */	\n\t"\
		"/* p0,4 combo: x+y into xmm0/1, x-y in xmm2/3: */\n\t	movaps	    (%%r12),%%xmm8 				\n\t"\
		"										\n\t			movaps	0x10(%%r12),%%xmm9 				\n\t"\
		"movaps	    (%%rcx),%%xmm0				\n\t			movaps	    (%%r10),%%xmm10				\n\t"\
		"movaps	0x10(%%rcx),%%xmm1				\n\t			movaps	0x10(%%r10),%%xmm11				\n\t"\
		"movaps	    (%%rax),%%xmm2				\n\t			subpd	%%xmm8 ,%%xmm10					\n\t"\
		"movaps	0x10(%%rax),%%xmm3				\n\t			subpd	%%xmm9 ,%%xmm11					\n\t"\
		"subpd	%%xmm0,%%xmm2					\n\t			addpd	%%xmm8 ,%%xmm8 					\n\t"\
		"subpd	%%xmm1,%%xmm3					\n\t			addpd	%%xmm9 ,%%xmm9 					\n\t"\
		"addpd	%%xmm0,%%xmm0					\n\t			addpd	%%xmm10,%%xmm8 					\n\t"\
		"addpd	%%xmm1,%%xmm1					\n\t			addpd	%%xmm11,%%xmm9 					\n\t"\
		"addpd	%%xmm2,%%xmm0					\n\t			/* p3,7 combo: x+y into xmm14/7, x-y in xmm12/5: */	\n\t"\
		"addpd	%%xmm3,%%xmm1					\n\t			movaps	    (%%r11),%%xmm12				\n\t"\
		"										\n\t			movaps	0x10(%%r11),%%xmm13				\n\t"\
		"/* p2,6 combo: x+y into xmm4/5, x-y in xmm6/7: */\n\t	movaps	    (%%r13),%%xmm14				\n\t"\
		"										\n\t			movaps	0x10(%%r13),%%xmm15				\n\t"\
		"movaps	    (%%rdx),%%xmm4				\n\t			subpd	%%xmm14,%%xmm12					\n\t"\
		"movaps	0x10(%%rdx),%%xmm5				\n\t			subpd	%%xmm15,%%xmm13					\n\t"\
		"movaps	    (%%rbx),%%xmm6				\n\t			addpd	%%xmm14,%%xmm14					\n\t"\
		"movaps	0x10(%%rbx),%%xmm7				\n\t			addpd	%%xmm15,%%xmm15					\n\t"\
		"subpd	%%xmm4,%%xmm6					\n\t			addpd	%%xmm12,%%xmm14					\n\t"\
		"subpd	%%xmm5,%%xmm7					\n\t			addpd	%%xmm13,%%xmm15					\n\t"\
		"addpd	%%xmm4,%%xmm4					\n\t			/* Finish radix-4 butterfly, tmp-store 1st of 4 outputs to free up 2 registers: */\n\t"\
		"addpd	%%xmm5,%%xmm5					\n\t			subpd	%%xmm14,%%xmm8 					\n\t"\
		"addpd	%%xmm6,%%xmm4					\n\t			subpd	%%xmm15,%%xmm9 					\n\t"\
		"addpd	%%xmm7,%%xmm5					\n\t			subpd	%%xmm13,%%xmm10					\n\t"\
		"										\n\t			subpd	%%xmm12,%%xmm11					\n\t"\
		"subpd	%%xmm4,%%xmm0					\n\t			addpd	%%xmm14,%%xmm14					\n\t"\
		"subpd	%%xmm7,%%xmm2					\n\t			addpd	%%xmm13,%%xmm13					\n\t"\
		"subpd	%%xmm5,%%xmm1					\n\t			addpd	%%xmm15,%%xmm15					\n\t"\
		"subpd	%%xmm6,%%xmm3					\n\t			addpd	%%xmm12,%%xmm12					\n\t"\
		"														addpd	%%xmm8 ,%%xmm14					\n\t"\
		"														addpd	%%xmm10,%%xmm13					\n\t"\
		"														addpd	%%xmm9 ,%%xmm15					\n\t"\
		"														addpd	%%xmm11,%%xmm12					\n\t"\
		"addpd	%%xmm4,%%xmm4					\n\t			movaps	%%xmm14,    (%%r10)				\n\t"\
		"addpd	%%xmm7,%%xmm7					\n\t			movaps	%%xmm15,0x10(%%r10)				\n\t"\
		"addpd	%%xmm5,%%xmm5					\n\t			movaps	%%xmm10,%%xmm14					\n\t"\
		"addpd	%%xmm6,%%xmm6					\n\t			movaps	%%xmm13,%%xmm15					\n\t"\
		"addpd	%%xmm0,%%xmm4					\n\t			subpd	%%xmm12,%%xmm10					\n\t"\
		"addpd	%%xmm2,%%xmm7					\n\t			subpd	%%xmm11,%%xmm13					\n\t"\
		"addpd	%%xmm1,%%xmm5					\n\t			addpd	%%xmm14,%%xmm12					\n\t"\
		"addpd	%%xmm3,%%xmm6					\n\t			addpd	%%xmm15,%%xmm11					\n\t"\
		"														movaps	(%%rsi),%%xmm14	/* isrt2 */		\n\t"\
		"														mulpd	%%xmm14,%%xmm10					\n\t"\
		"														mulpd	%%xmm14,%%xmm13					\n\t"\
		"														mulpd	%%xmm14,%%xmm12					\n\t"\
		"										\n\t			mulpd	%%xmm14,%%xmm11					\n\t"\
		"movaps	    (%%r10),%%xmm14	/* restore spilled */\n\t"\
		"movaps	0x10(%%r10),%%xmm15	/* restore spilled */\n\t"\
		"										\n\t"\
		"/* Inline of SSE2_RADIX8_DIF_COMBINE_RAD4_SUBS_A(r0): Combine radix-4 subtransforms and write outputs: */\n\t"\
		"/***** t0,1,2,3,4,5,6,7 in xmm[ 4, 5| 2, 6| 0, 1| 7, 3] *****/\n\t"\
		"/***** t8,9,a,b,c,d,e,f in xmm[14,15|10,12| 8, 9|13,11] */\n\t"\
		"movq	%[__o4],%%rax					\n\t			subpd   %%xmm10,%%xmm2					\n\t"\
		"movq	%[__o5],%%rbx					\n\t			subpd   %%xmm12,%%xmm6					\n\t"\
		"movq	%[__o6],%%rcx					\n\t			addpd   %%xmm10,%%xmm10					\n\t"\
		"movq	%[__o7],%%rdx					\n\t			addpd   %%xmm12,%%xmm12					\n\t"\
		"										\n\t			addpd   %%xmm2,%%xmm10					\n\t"\
		"subpd   %%xmm11,%%xmm7					\n\t			addpd   %%xmm6,%%xmm12					\n\t"\
		"subpd   %%xmm13,%%xmm3					\n\t													\n\t"\
		"addpd   %%xmm11,%%xmm11				\n\t			movaps	%%xmm2 ,    (%%rbx)	/* o5r */	\n\t"\
		"addpd   %%xmm13,%%xmm13				\n\t			movaps	%%xmm6 ,0x10(%%rbx)	/* o5i */	\n\t"\
		"addpd   %%xmm7,%%xmm11					\n\t			movaps	%%xmm10,    (%%rax)	/* o4r */	\n\t"\
		"addpd   %%xmm3,%%xmm13					\n\t			movaps	%%xmm12,0x10(%%rax)	/* o4i */	\n\t"\
		"										\n\t"\
		"movaps	%%xmm7 ,    (%%rcx)	/* o6r */	\n\t"\
		"movaps	%%xmm3 ,0x10(%%rdx)	/* o7i */	\n\t"\
		"movaps	%%xmm11,    (%%rdx)	/* o7r */	\n\t"\
		"movaps	%%xmm13,0x10(%%rcx)	/* o6i */	\n\t"\
		"										\n\t"\
		"movq	%[__o0],%%rax					\n\t"\
		"movq	%[__o1],%%rbx					\n\t"\
		"movq	%[__o2],%%rcx					\n\t"\
		"movq	%[__o3],%%rdx					\n\t"\
		"										\n\t"\
		"subpd	%%xmm14,%%xmm4 					\n\t"\
		"subpd	%%xmm15,%%xmm5 					\n\t"\
		"subpd	%%xmm9 ,%%xmm0 					\n\t"\
		"subpd	%%xmm8 ,%%xmm1 					\n\t"\
		"addpd	%%xmm14,%%xmm14					\n\t			movaps	%%xmm4 ,    (%%rbx)	/* o1r */	\n\t"\
		"addpd	%%xmm15,%%xmm15					\n\t			movaps	%%xmm5 ,0x10(%%rbx)	/* o1i */	\n\t"\
		"addpd	%%xmm9 ,%%xmm9 					\n\t			movaps	%%xmm0 ,    (%%rcx)	/* o2r */	\n\t"\
		"addpd	%%xmm8 ,%%xmm8 					\n\t			movaps	%%xmm1 ,0x10(%%rdx)	/* o3i */	\n\t"\
		"addpd	%%xmm4 ,%%xmm14					\n\t"\
		"addpd	%%xmm5 ,%%xmm15					\n\t"\
		"addpd	%%xmm0 ,%%xmm9 					\n\t"\
		"addpd	%%xmm1 ,%%xmm8 					\n\t"\
		"										\n\t"\
		"movaps	%%xmm14,    (%%rax)	/* o0r */	\n\t"\
		"movaps	%%xmm15,0x10(%%rax)	/* o0r */	\n\t"\
		"movaps	%%xmm9 ,    (%%rdx)	/* o3r */	\n\t"\
		"movaps	%%xmm8 ,0x10(%%rcx)	/* o2i */	\n\t"\
		"										\n\t"\
		:					/* outputs: none */\
		: [__r0] "m" (Xr0)	/* All inputs from memory addresses here */\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__i3] "m" (Xi3)\
		 ,[__i4] "m" (Xi4)\
		 ,[__i5] "m" (Xi5)\
		 ,[__i6] "m" (Xi6)\
		 ,[__i7] "m" (Xi7)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__o3] "m" (Xo3)\
		 ,[__o4] "m" (Xo4)\
		 ,[__o5] "m" (Xo5)\
		 ,[__o6] "m" (Xo6)\
		 ,[__o7] "m" (Xo7)\
		 ,[__isrt2] "m" (Xisrt2)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}


	/* Twiddleless version of SSE2_RADIX8_DIT_TWIDDLE. Inputs enter in memory locations __i0,__i1,__i2,__i3,__i4,__i5,__i6,__i7.
	Outputs go into 16 contiguous 32-byte memory locations starting at __out and assumed disjoint with inputs.
	This macro built on the same code template as SSE2_RADIX8_DIF_TWIDDLE0, but with the I/O-location indices mutually bit reversed:
	01234567 <--> 04261537, which can be effected via the pairwise swaps 1 <--> 4 and 3 <--> 6.
	*/
	#define	SSE2_RADIX8_DIT_0TWIDDLE(Xi0,Xi1,Xi2,Xi3,Xi4,Xi5,Xi6,Xi7, Xout, Xisrt2)\
	{\
	__asm__ volatile (\
		"/* 1st of 2 radix-4 subtransforms, data in xmm0-7: */\n\t	/* 2nd of 2 radix-4 subtransforms, data in xmm8-15: */\n\t"\
		"movq	%[__i0],%%rax					\n\t			movq	%[__i4],%%r10					\n\t"\
		"movq	%[__i1],%%rbx					\n\t			movq	%[__i5],%%r11					\n\t"\
		"movq	%[__i2],%%rcx					\n\t			movq	%[__i6],%%r12					\n\t"\
		"movq	%[__i3],%%rdx					\n\t			movq	%[__i7],%%r13					\n\t"\
		"										\n\t			/* p1,5 combo: x+y into xmm8 /1, x-y in xmm10/3: */	\n\t"\
		"/* p0,4 combo: x+y into xmm0/1, x-y in xmm2/3: */\n\t	movaps	    (%%r11),%%xmm8 				\n\t"\
		"										\n\t			movaps	0x10(%%r11),%%xmm9 				\n\t"\
		"movaps	    (%%rbx),%%xmm0				\n\t			movaps	    (%%r10),%%xmm10				\n\t"\
		"movaps	0x10(%%rbx),%%xmm1				\n\t			movaps	0x10(%%r10),%%xmm11				\n\t"\
		"movaps	    (%%rax),%%xmm2				\n\t			subpd	%%xmm8 ,%%xmm10					\n\t"\
		"movaps	0x10(%%rax),%%xmm3				\n\t			subpd	%%xmm9 ,%%xmm11					\n\t"\
		"subpd	%%xmm0,%%xmm2					\n\t			addpd	%%xmm8 ,%%xmm8 					\n\t"\
		"subpd	%%xmm1,%%xmm3					\n\t			addpd	%%xmm9 ,%%xmm9 					\n\t"\
		"addpd	%%xmm0,%%xmm0					\n\t			addpd	%%xmm10,%%xmm8 					\n\t"\
		"addpd	%%xmm1,%%xmm1					\n\t			addpd	%%xmm11,%%xmm9 					\n\t"\
		"addpd	%%xmm2,%%xmm0					\n\t			/* p3,7 combo: x+y into xmm14/7, x-y in xmm12/5: */	\n\t"\
		"addpd	%%xmm3,%%xmm1					\n\t			movaps	    (%%r12),%%xmm12				\n\t"\
		"										\n\t			movaps	0x10(%%r12),%%xmm13				\n\t"\
		"/* p2,6 combo: x+y into xmm4/5, x-y in xmm6/7: */\n\t	movaps	    (%%r13),%%xmm14				\n\t"\
		"										\n\t			movaps	0x10(%%r13),%%xmm15				\n\t"\
		"movaps	    (%%rdx),%%xmm4				\n\t			subpd	%%xmm14,%%xmm12					\n\t"\
		"movaps	0x10(%%rdx),%%xmm5				\n\t			subpd	%%xmm15,%%xmm13					\n\t"\
		"movaps	    (%%rcx),%%xmm6				\n\t			addpd	%%xmm14,%%xmm14					\n\t"\
		"movaps	0x10(%%rcx),%%xmm7				\n\t			addpd	%%xmm15,%%xmm15					\n\t"\
		"subpd	%%xmm4,%%xmm6					\n\t			addpd	%%xmm12,%%xmm14					\n\t"\
		"subpd	%%xmm5,%%xmm7					\n\t			addpd	%%xmm13,%%xmm15					\n\t"\
		"addpd	%%xmm4,%%xmm4					\n\t			/* Finish radix-4 butterfly, tmp-store 1st of 4 outputs to free up 2 registers: */\n\t"\
		"addpd	%%xmm5,%%xmm5					\n\t			subpd	%%xmm14,%%xmm8 					\n\t"\
		"addpd	%%xmm6,%%xmm4					\n\t			subpd	%%xmm15,%%xmm9 					\n\t"\
		"addpd	%%xmm7,%%xmm5					\n\t			subpd	%%xmm13,%%xmm10					\n\t"\
		"										\n\t			subpd	%%xmm12,%%xmm11					\n\t"\
		"subpd	%%xmm4,%%xmm0					\n\t			addpd	%%xmm14,%%xmm14					\n\t"\
		"subpd	%%xmm7,%%xmm2					\n\t			addpd	%%xmm13,%%xmm13					\n\t"\
		"subpd	%%xmm5,%%xmm1					\n\t			addpd	%%xmm15,%%xmm15					\n\t"\
		"subpd	%%xmm6,%%xmm3					\n\t			addpd	%%xmm12,%%xmm12					\n\t"\
		"														addpd	%%xmm8 ,%%xmm14					\n\t"\
		"														addpd	%%xmm10,%%xmm13					\n\t"\
		"														addpd	%%xmm9 ,%%xmm15					\n\t"\
		"														addpd	%%xmm11,%%xmm12					\n\t"\
		"														movq	%[__isrt2],%%rsi	/* isrt2 */	\n\t"\
		"addpd	%%xmm4,%%xmm4					\n\t			movaps	%%xmm14,    (%%rax)	/* spill */	\n\t"\
		"addpd	%%xmm7,%%xmm7					\n\t			movaps	%%xmm15,0x10(%%rax)	/* spill */	\n\t"\
		"addpd	%%xmm5,%%xmm5					\n\t			movaps	%%xmm10,%%xmm14					\n\t"\
		"addpd	%%xmm6,%%xmm6					\n\t			movaps	%%xmm13,%%xmm15					\n\t"\
		"addpd	%%xmm0,%%xmm4					\n\t			subpd	%%xmm12,%%xmm10					\n\t"\
		"addpd	%%xmm2,%%xmm7					\n\t			subpd	%%xmm11,%%xmm13					\n\t"\
		"addpd	%%xmm1,%%xmm5					\n\t			addpd	%%xmm14,%%xmm12					\n\t"\
		"addpd	%%xmm3,%%xmm6					\n\t			addpd	%%xmm15,%%xmm11					\n\t"\
		"														movaps	(%%rsi),%%xmm14		/* isrt2 */	\n\t"\
		"														mulpd	%%xmm14,%%xmm10					\n\t"\
		"														mulpd	%%xmm14,%%xmm13					\n\t"\
		"														mulpd	%%xmm14,%%xmm12					\n\t"\
		"														mulpd	%%xmm14,%%xmm11					\n\t"\
		"/* Combine radix-4 subtransforms and write outputs: */\n\t"\
		"\n\t"\
		"movaps	    (%%rax),%%xmm14	/* restore spilled */\n\t	subpd   %%xmm10,%%xmm2					\n\t"\
		"movaps	0x10(%%rax),%%xmm15	/* restore spilled */\n\t	subpd   %%xmm12,%%xmm6					\n\t"\
		"														addpd   %%xmm10,%%xmm10					\n\t"\
		"movq	%[__out],%%rax					\n\t			addpd   %%xmm12,%%xmm12					\n\t"\
		"										\n\t			addpd   %%xmm2,%%xmm10					\n\t"\
		"subpd   %%xmm11,%%xmm7					\n\t			addpd   %%xmm6,%%xmm12					\n\t"\
		"subpd   %%xmm13,%%xmm3					\n\t													\n\t"\
		"addpd   %%xmm11,%%xmm11				\n\t			movaps	%%xmm2 ,0xa0(%%rax)	/* o5r */	\n\t"\
		"addpd   %%xmm13,%%xmm13				\n\t			movaps	%%xmm6 ,0xb0(%%rax)	/* o5i */	\n\t"\
		"addpd   %%xmm7,%%xmm11					\n\t			movaps	%%xmm10,0x20(%%rax)	/* o1r */	\n\t"\
		"addpd   %%xmm3,%%xmm13					\n\t			movaps	%%xmm12,0x30(%%rax)	/* o1i */	\n\t"\
		"										\n\t"\
		"movaps	%%xmm7 ,0x60(%%rax)	/* o3r */	\n\t"\
		"movaps	%%xmm3 ,0xf0(%%rax)	/* o7i */	\n\t"\
		"movaps	%%xmm11,0xe0(%%rax)	/* o7r */	\n\t"\
		"movaps	%%xmm13,0x70(%%rax)	/* o3i */	\n\t"\
		"										\n\t"\
		"subpd	%%xmm14,%%xmm4 					\n\t"\
		"subpd	%%xmm15,%%xmm5 					\n\t"\
		"subpd	%%xmm9 ,%%xmm0 					\n\t"\
		"subpd	%%xmm8 ,%%xmm1 					\n\t"\
		"addpd	%%xmm14,%%xmm14					\n\t			movaps	%%xmm4 ,0x80(%%rax)	/* o4r */	\n\t"\
		"addpd	%%xmm15,%%xmm15					\n\t			movaps	%%xmm5 ,0x90(%%rax)	/* o4i */	\n\t"\
		"addpd	%%xmm9 ,%%xmm9 					\n\t			movaps	%%xmm0 ,0x40(%%rax)	/* o2r */	\n\t"\
		"addpd	%%xmm8 ,%%xmm8 					\n\t			movaps	%%xmm1 ,0xd0(%%rax)	/* o6i */	\n\t"\
		"addpd	%%xmm4 ,%%xmm14					\n\t"\
		"addpd	%%xmm5 ,%%xmm15					\n\t"\
		"addpd	%%xmm0 ,%%xmm9 					\n\t"\
		"addpd	%%xmm1 ,%%xmm8 					\n\t"\
		"										\n\t"\
		"movaps	%%xmm14,    (%%rax)	/* o0r */	\n\t"\
		"movaps	%%xmm15,0x10(%%rax)	/* o0r */	\n\t"\
		"movaps	%%xmm9 ,0xc0(%%rax)	/* o6r */	\n\t"\
		"movaps	%%xmm8 ,0x50(%%rax)	/* o2i */	\n\t"\
		"										\n\t"\
		:					/* outputs: none */\
		: [__i0] "m" (Xi0)	/* All iputs from memory addresses here */\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__i3] "m" (Xi3)\
		 ,[__i4] "m" (Xi4)\
		 ,[__i5] "m" (Xi5)\
		 ,[__i6] "m" (Xi6)\
		 ,[__i7] "m" (Xi7)\
		 ,[__out] "m" (Xout)\
		 ,[__isrt2] "m" (Xisrt2)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

	// Same as SSE2_RADIX8_DIT_0TWIDDLE but with user-specifiable [i.e. not nec. contiguous] output addresses:
	#define	SSE2_RADIX8_DIT_0TWIDDLE_OOP(Xi0,Xi1,Xi2,Xi3,Xi4,Xi5,Xi6,Xi7, Xo0,Xo1,Xo2,Xo3,Xo4,Xo5,Xo6,Xo7, Xisrt2)\
	{\
	__asm__ volatile (\
		"/* 1st of 2 radix-4 subtransforms, data in xmm0-7: */\n\t	/* 2nd of 2 radix-4 subtransforms, data in xmm8-15: */\n\t"\
		"movq	%[__i0],%%rax					\n\t			movq	%[__i4],%%r10					\n\t"\
		"movq	%[__i1],%%rbx					\n\t			movq	%[__i5],%%r11					\n\t"\
		"movq	%[__i2],%%rcx					\n\t			movq	%[__i6],%%r12					\n\t"\
		"movq	%[__i3],%%rdx					\n\t			movq	%[__i7],%%r13					\n\t"\
		"										\n\t			/* p1,5 combo: x+y into xmm8 /1, x-y in xmm10/3: */	\n\t"\
		"/* p0,4 combo: x+y into xmm0/1, x-y in xmm2/3: */\n\t	movaps	    (%%r11),%%xmm8 				\n\t"\
		"										\n\t			movaps	0x10(%%r11),%%xmm9 				\n\t"\
		"movaps	    (%%rbx),%%xmm0				\n\t			movaps	    (%%r10),%%xmm10				\n\t"\
		"movaps	0x10(%%rbx),%%xmm1				\n\t			movaps	0x10(%%r10),%%xmm11				\n\t"\
		"movaps	    (%%rax),%%xmm2				\n\t			subpd	%%xmm8 ,%%xmm10					\n\t"\
		"movaps	0x10(%%rax),%%xmm3				\n\t			subpd	%%xmm9 ,%%xmm11					\n\t"\
		"subpd	%%xmm0,%%xmm2					\n\t			addpd	%%xmm8 ,%%xmm8 					\n\t"\
		"subpd	%%xmm1,%%xmm3					\n\t			addpd	%%xmm9 ,%%xmm9 					\n\t"\
		"addpd	%%xmm0,%%xmm0					\n\t			addpd	%%xmm10,%%xmm8 					\n\t"\
		"addpd	%%xmm1,%%xmm1					\n\t			addpd	%%xmm11,%%xmm9 					\n\t"\
		"addpd	%%xmm2,%%xmm0					\n\t			/* p3,7 combo: x+y into xmm14/7, x-y in xmm12/5: */	\n\t"\
		"addpd	%%xmm3,%%xmm1					\n\t			movaps	    (%%r12),%%xmm12				\n\t"\
		"										\n\t			movaps	0x10(%%r12),%%xmm13				\n\t"\
		"/* p2,6 combo: x+y into xmm4/5, x-y in xmm6/7: */\n\t	movaps	    (%%r13),%%xmm14				\n\t"\
		"										\n\t			movaps	0x10(%%r13),%%xmm15				\n\t"\
		"movaps	    (%%rdx),%%xmm4				\n\t			subpd	%%xmm14,%%xmm12					\n\t"\
		"movaps	0x10(%%rdx),%%xmm5				\n\t			subpd	%%xmm15,%%xmm13					\n\t"\
		"movaps	    (%%rcx),%%xmm6				\n\t			addpd	%%xmm14,%%xmm14					\n\t"\
		"movaps	0x10(%%rcx),%%xmm7				\n\t			addpd	%%xmm15,%%xmm15					\n\t"\
		"subpd	%%xmm4,%%xmm6					\n\t			addpd	%%xmm12,%%xmm14					\n\t"\
		"subpd	%%xmm5,%%xmm7					\n\t			addpd	%%xmm13,%%xmm15					\n\t"\
		"addpd	%%xmm4,%%xmm4					\n\t			/* Finish radix-4 butterfly, tmp-store 1st of 4 outputs to free up 2 registers: */\n\t"\
		"addpd	%%xmm5,%%xmm5					\n\t			subpd	%%xmm14,%%xmm8 					\n\t"\
		"addpd	%%xmm6,%%xmm4					\n\t			subpd	%%xmm15,%%xmm9 					\n\t"\
		"addpd	%%xmm7,%%xmm5					\n\t			subpd	%%xmm13,%%xmm10					\n\t"\
		"										\n\t			subpd	%%xmm12,%%xmm11					\n\t"\
		"subpd	%%xmm4,%%xmm0					\n\t			addpd	%%xmm14,%%xmm14					\n\t"\
		"subpd	%%xmm7,%%xmm2					\n\t			addpd	%%xmm13,%%xmm13					\n\t"\
		"subpd	%%xmm5,%%xmm1					\n\t			addpd	%%xmm15,%%xmm15					\n\t"\
		"subpd	%%xmm6,%%xmm3					\n\t			addpd	%%xmm12,%%xmm12					\n\t"\
		"														addpd	%%xmm8 ,%%xmm14					\n\t"\
		"														addpd	%%xmm10,%%xmm13					\n\t"\
		"														addpd	%%xmm9 ,%%xmm15					\n\t"\
		"														addpd	%%xmm11,%%xmm12					\n\t"\
		"														movq	%[__isrt2],%%rsi	/* isrt2 */	\n\t"\
		"addpd	%%xmm4,%%xmm4					\n\t			movaps	%%xmm14,    (%%rax)	/* spill */	\n\t"\
		"addpd	%%xmm7,%%xmm7					\n\t			movaps	%%xmm15,0x10(%%rax)	/* spill */	\n\t"\
		"addpd	%%xmm5,%%xmm5					\n\t			movaps	%%xmm10,%%xmm14					\n\t"\
		"addpd	%%xmm6,%%xmm6					\n\t			movaps	%%xmm13,%%xmm15					\n\t"\
		"addpd	%%xmm0,%%xmm4					\n\t			subpd	%%xmm12,%%xmm10					\n\t"\
		"addpd	%%xmm2,%%xmm7					\n\t			subpd	%%xmm11,%%xmm13					\n\t"\
		"addpd	%%xmm1,%%xmm5					\n\t			addpd	%%xmm14,%%xmm12					\n\t"\
		"addpd	%%xmm3,%%xmm6					\n\t			addpd	%%xmm15,%%xmm11					\n\t"\
		"														movaps	(%%rsi),%%xmm14		/* isrt2 */	\n\t"\
		"														mulpd	%%xmm14,%%xmm10					\n\t"\
		"														mulpd	%%xmm14,%%xmm13					\n\t"\
		"														mulpd	%%xmm14,%%xmm12					\n\t"\
		"														mulpd	%%xmm14,%%xmm11					\n\t"\
		"/* Combine radix-4 subtransforms and write outputs: */\n\t"\
		"\n\t"\
		"movaps	    (%%rax),%%xmm14	/* restore spilled */\n\t	subpd   %%xmm10,%%xmm2					\n\t"\
		"movaps	0x10(%%rax),%%xmm15	/* restore spilled */\n\t	subpd   %%xmm12,%%xmm6					\n\t"\
		"movq	%[__o1],%%rax					\n\t			movq	%[__o5],%%rcx					\n\t"\
		"														addpd   %%xmm10,%%xmm10					\n\t"\
		"										\n\t			addpd   %%xmm12,%%xmm12					\n\t"\
		"										\n\t			addpd   %%xmm2,%%xmm10					\n\t"\
		"subpd   %%xmm11,%%xmm7					\n\t			addpd   %%xmm6,%%xmm12					\n\t"\
		"subpd   %%xmm13,%%xmm3					\n\t													\n\t"\
		"movq	%[__o3],%%rbx					\n\t			movq	%[__o7],%%rdx					\n\t"\
		"addpd   %%xmm11,%%xmm11				\n\t			movaps	%%xmm2 ,    (%%rcx)	/* o5r */	\n\t"\
		"addpd   %%xmm13,%%xmm13				\n\t			movaps	%%xmm6 ,0x10(%%rcx)	/* o5i */	\n\t"\
		"addpd   %%xmm7,%%xmm11					\n\t			movaps	%%xmm10,    (%%rax)	/* o1r */	\n\t"\
		"addpd   %%xmm3,%%xmm13					\n\t			movaps	%%xmm12,0x10(%%rax)	/* o1i */	\n\t"\
		"movq	%[__o0],%%rax					\n\t			movq	%[__o4],%%rcx					\n\t"\
		"										\n\t"\
		"movaps	%%xmm7 ,    (%%rbx)	/* o3r */	\n\t"\
		"movaps	%%xmm3 ,0x10(%%rdx)	/* o7i */	\n\t"\
		"movaps	%%xmm11,    (%%rdx)	/* o7r */	\n\t"\
		"movaps	%%xmm13,0x10(%%rbx)	/* o3i */	\n\t"\
		"										\n\t"\
		"movq	%[__o2],%%rbx					\n\t			movq	%[__o6],%%rdx					\n\t"\
		"subpd	%%xmm14,%%xmm4 					\n\t"\
		"subpd	%%xmm15,%%xmm5 					\n\t"\
		"subpd	%%xmm9 ,%%xmm0 					\n\t"\
		"subpd	%%xmm8 ,%%xmm1 					\n\t"\
		"addpd	%%xmm14,%%xmm14					\n\t			movaps	%%xmm4 ,    (%%rcx)	/* o4r */	\n\t"\
		"addpd	%%xmm15,%%xmm15					\n\t			movaps	%%xmm5 ,0x10(%%rcx)	/* o4i */	\n\t"\
		"addpd	%%xmm9 ,%%xmm9 					\n\t			movaps	%%xmm0 ,    (%%rbx)	/* o2r */	\n\t"\
		"addpd	%%xmm8 ,%%xmm8 					\n\t			movaps	%%xmm1 ,0x10(%%rdx)	/* o6i */	\n\t"\
		"addpd	%%xmm4 ,%%xmm14					\n\t"\
		"addpd	%%xmm5 ,%%xmm15					\n\t"\
		"addpd	%%xmm0 ,%%xmm9 					\n\t"\
		"addpd	%%xmm1 ,%%xmm8 					\n\t"\
		"										\n\t"\
		"movaps	%%xmm14,    (%%rax)	/* o0r */	\n\t"\
		"movaps	%%xmm15,0x10(%%rax)	/* o0i */	\n\t"\
		"movaps	%%xmm9 ,    (%%rdx)	/* o6r */	\n\t"\
		"movaps	%%xmm8 ,0x10(%%rbx)	/* o2i */	\n\t"\
		"										\n\t"\
		:					/* outputs: none */\
		: [__i0] "m" (Xi0)	/* All iputs from memory addresses here */\
		 ,[__i1] "m" (Xi1)\
		 ,[__i2] "m" (Xi2)\
		 ,[__i3] "m" (Xi3)\
		 ,[__i4] "m" (Xi4)\
		 ,[__i5] "m" (Xi5)\
		 ,[__i6] "m" (Xi6)\
		 ,[__i7] "m" (Xi7)\
		 ,[__o0] "m" (Xo0)\
		 ,[__o1] "m" (Xo1)\
		 ,[__o2] "m" (Xo2)\
		 ,[__o3] "m" (Xo3)\
		 ,[__o4] "m" (Xo4)\
		 ,[__o5] "m" (Xo5)\
		 ,[__o6] "m" (Xo6)\
		 ,[__o7] "m" (Xo7)\
		 ,[__isrt2] "m" (Xisrt2)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
	);\
	}

	// SSE2 analog of dft_macro.h::RADIX_08_DIF_TWIDDLE_OOP - Result of adding separate I/O addressing to
	// radix8_dif_dit_pass_gcc64.h::SSE2_RADIX8_DIF_TWIDDLE:

  #if 1	// 16-register version:

	#define SSE2_RADIX8_DIF_TWIDDLE_OOP(Xi0,Xi1,Xi2,Xi3,Xi4,Xi5,Xi6,Xi7, Xo0,Xo1,Xo2,Xo3,Xo4,Xo5,Xo6,Xo7 ,Xc1,Xs1,Xc2,Xs2,Xc3,Xs3,Xc4,Xs4,Xc5,Xs5,Xc6,Xs6,Xc7,Xs7)\
	{\
	__asm__ volatile (\
		"											movq		%[i1]	,%%r10		\n\t"\
		"											movq		%[i5]	,%%r11		\n\t"\
		"							movq	%[c1],%%r12	\n\t	movq	%[c5],%%r14	\n\t"\
		"							movq	%[s1],%%r13	\n\t	movq	%[s5],%%r15	\n\t"\
		"											movaps	    (%%r10)	,%%xmm8 	\n\t"\
		"movq		%[i0]	,%%rax		\n\t		movaps	0x10(%%r10)	,%%xmm10	\n\t"\
		"movq		%[i4]	,%%rbx		\n\t		movaps	    (%%r10)	,%%xmm9 	\n\t"\
		"movq		%[c4]	,%%rcx		\n\t		movaps	0x10(%%r10)	,%%xmm11	\n\t"\
		"movq		%[s4]	,%%rsi		\n\t"\
	/* [rsi] (and if needed rdi) points to sine components of each sincos pair, which is not really a pair here in terms of relative addressing: */\
		"movaps	    (%%rax)	,%%xmm0		\n\t		mulpd	    (%%r12)	,%%xmm8 	\n\t"\
		"movaps	0x10(%%rax)	,%%xmm1		\n\t		mulpd	    (%%r13)	,%%xmm10	\n\t"\
		"movaps	    (%%rax)	,%%xmm6		\n\t		mulpd	    (%%r13)	,%%xmm9 	\n\t"\
		"movaps	0x10(%%rax)	,%%xmm7		\n\t		mulpd	    (%%r12)	,%%xmm11	\n\t"\
		"movaps	    (%%rbx)	,%%xmm2		\n\t		subpd	%%xmm10		,%%xmm8 	\n\t"\
		"movaps	0x10(%%rbx)	,%%xmm3		\n\t		addpd	%%xmm11		,%%xmm9 	\n\t"\
		"movaps	    (%%rbx)	,%%xmm4		\n\t		movaps	    (%%r11)	,%%xmm10	\n\t"\
		"movaps	0x10(%%rbx)	,%%xmm5		\n\t		movaps	0x10(%%r11)	,%%xmm11	\n\t"\
		"mulpd	    (%%rcx)	,%%xmm2		\n\t		movaps	    (%%r11)	,%%xmm12	\n\t"\
		"mulpd	    (%%rcx)	,%%xmm3		\n\t		movaps	0x10(%%r11)	,%%xmm13	\n\t"\
		"mulpd	    (%%rsi)	,%%xmm4		\n\t		mulpd	    (%%r14)	,%%xmm10	\n\t"\
		"mulpd	    (%%rsi)	,%%xmm5		\n\t		mulpd	    (%%r15)	,%%xmm11	\n\t"\
		"subpd	%%xmm5		,%%xmm2		\n\t		mulpd	    (%%r15)	,%%xmm12	\n\t"\
		"addpd	%%xmm4		,%%xmm3		\n\t		mulpd	    (%%r14)	,%%xmm13	\n\t"\
		"addpd	%%xmm2		,%%xmm0		\n\t		subpd	%%xmm11		,%%xmm10	\n\t"\
		"addpd	%%xmm3		,%%xmm1		\n\t		addpd	%%xmm13		,%%xmm12	\n\t"\
		"subpd	%%xmm2		,%%xmm6		\n\t		movaps	%%xmm10		,%%xmm11	\n\t"\
		"subpd	%%xmm3		,%%xmm7		\n\t		movaps	%%xmm12		,%%xmm13	\n\t"\
		"movaps	%%xmm0	,    (%%rax)	\n\t		addpd	%%xmm8 	,%%xmm10		\n\t"\
		"movaps	%%xmm1	,0x10(%%rax)	\n\t		subpd	%%xmm11	,%%xmm8 		\n\t"\
		"movaps	%%xmm6	,    (%%rbx)	\n\t		addpd	%%xmm9 	,%%xmm12		\n\t"\
		"movaps	%%xmm7	,0x10(%%rbx)	\n\t		subpd	%%xmm13	,%%xmm9 		\n\t"\
		"movq		%[i2]	,%%rax		\n\t		movaps	%%xmm10	,    (%%r10)	\n\t"\
		"movq		%[i6]	,%%rbx		\n\t		movaps	%%xmm12	,0x10(%%r10)	\n\t"\
		"movq %[c2],%%rcx \n\t movq %[s2],%%rsi	\n\t	movaps	%%xmm8,    (%%r11)	\n\t"\
		"movq %[c6],%%rdx \n\t movq %[s6],%%rdi	\n\t	movaps	%%xmm9,0x10(%%r11)	\n\t"\
		"movaps	    (%%rax)	,%%xmm0		\n\t		movq		%[i3]	,%%r10		\n\t"\
		"movaps	0x10(%%rax)	,%%xmm2		\n\t		movq		%[i7]	,%%r11		\n\t"\
		"movaps	    (%%rax)	,%%xmm1		\n\t	movq %[c3],%%r12 \n\t movq %[s3],%%r13	\n\t"\
		"movaps	0x10(%%rax)	,%%xmm3		\n\t	movq %[c7],%%r14 \n\t movq %[s7],%%r15	\n\t"\
		"mulpd	    (%%rcx)	,%%xmm0		\n\t		movaps	    (%%r10)	,%%xmm8 	\n\t"\
		"mulpd	    (%%rsi)	,%%xmm2		\n\t		movaps	0x10(%%r10)	,%%xmm10	\n\t"\
		"mulpd	    (%%rsi)	,%%xmm1		\n\t		movaps	    (%%r10)	,%%xmm9 	\n\t"\
		"mulpd	    (%%rcx)	,%%xmm3		\n\t		movaps	0x10(%%r10)	,%%xmm11	\n\t"\
		"subpd	%%xmm2		,%%xmm0		\n\t		mulpd	    (%%r12)	,%%xmm8 	\n\t"\
		"addpd	%%xmm3		,%%xmm1		\n\t		mulpd	    (%%r13)	,%%xmm10	\n\t"\
		"movaps	    (%%rbx)	,%%xmm2		\n\t		mulpd	    (%%r13)	,%%xmm9 	\n\t"\
		"movaps	0x10(%%rbx)	,%%xmm3		\n\t		mulpd	    (%%r12)	,%%xmm11	\n\t"\
		"movaps	    (%%rbx)	,%%xmm4		\n\t		subpd	%%xmm10		,%%xmm8 	\n\t"\
		"movaps	0x10(%%rbx)	,%%xmm5		\n\t		addpd	%%xmm11		,%%xmm9 	\n\t"\
		"mulpd	    (%%rdx)	,%%xmm2		\n\t		movaps	    (%%r11)	,%%xmm10	\n\t"\
		"mulpd	    (%%rdi)	,%%xmm3		\n\t		movaps	0x10(%%r11)	,%%xmm11	\n\t"\
		"mulpd	    (%%rdi)	,%%xmm4		\n\t		movaps	    (%%r11)	,%%xmm12	\n\t"\
		"mulpd	    (%%rdx)	,%%xmm5		\n\t		movaps	0x10(%%r11)	,%%xmm13	\n\t"\
		"subpd	%%xmm3		,%%xmm2		\n\t		mulpd	    (%%r14)	,%%xmm10	\n\t"\
		"addpd	%%xmm5		,%%xmm4		\n\t		mulpd	    (%%r15)	,%%xmm11	\n\t"\
		"movaps	%%xmm2		,%%xmm3		\n\t		mulpd	    (%%r15)	,%%xmm12	\n\t"\
		"movaps	%%xmm4		,%%xmm5		\n\t		mulpd	    (%%r14)	,%%xmm13	\n\t"\
		"addpd	%%xmm0		,%%xmm2		\n\t		subpd	%%xmm11		,%%xmm10	\n\t"\
		"subpd	%%xmm3		,%%xmm0		\n\t		addpd	%%xmm13		,%%xmm12	\n\t"\
		"addpd	%%xmm1		,%%xmm4		\n\t		movaps	%%xmm10		,%%xmm11	\n\t"\
		"subpd	%%xmm5		,%%xmm1		\n\t		movaps	%%xmm12		,%%xmm13	\n\t"\
		"movaps	%%xmm2	,    (%%rax)	\n\t		addpd	%%xmm8 		,%%xmm10	\n\t"\
		"movaps	%%xmm4	,0x10(%%rax)	\n\t		subpd	%%xmm11		,%%xmm8 	\n\t"\
		"movaps	%%xmm0	,    (%%rbx)	\n\t		addpd	%%xmm9 		,%%xmm12	\n\t"\
		"movaps	%%xmm1	,0x10(%%rbx)	\n\t		subpd	%%xmm13		,%%xmm9 	\n\t"\
		"											movaps	%%xmm10	,    (%%r10)	\n\t"\
		"											movaps	%%xmm12	,0x10(%%r10)	\n\t"\
		"											movaps	%%xmm8 	,    (%%r11)	\n\t"\
		"											movaps	%%xmm9 	,0x10(%%r11)	\n\t"\
/* combine to get 2 length-4 output subtransforms... */\
		"movq		%[i0]	,%%rax		\n\t		movq		%[i4]	,%%r10		\n\t"\
		"movq		%[i2]	,%%rbx		\n\t		movq		%[i6]	,%%r11		\n\t"\
		"movaps	    (%%rax)	,%%xmm0		\n\t		movaps	    (%%r10)	,%%xmm8 	\n\t"\
		"movaps	0x10(%%rax)	,%%xmm1		\n\t		movaps	0x10(%%r10)	,%%xmm9 	\n\t"\
		"movaps	%%xmm0		,%%xmm4		\n\t		movaps	%%xmm8 		,%%xmm12	\n\t"\
		"movaps	%%xmm1		,%%xmm5		\n\t		movaps	%%xmm9 		,%%xmm13	\n\t"\
		"addpd	    (%%rbx)	,%%xmm0		\n\t		subpd	0x10(%%r11)	,%%xmm8 	\n\t"\
		"subpd	    (%%rbx)	,%%xmm4		\n\t		addpd	0x10(%%r11)	,%%xmm12	\n\t"\
		"addpd	0x10(%%rbx)	,%%xmm1		\n\t		addpd	    (%%r11)	,%%xmm9 	\n\t"\
		"subpd	0x10(%%rbx)	,%%xmm5		\n\t		subpd	    (%%r11)	,%%xmm13	\n\t"\
		"movq		%[o0]	,%%rax		\n\t		movq		%[o4]	,%%r10		\n\t"\
		"movq		%[o2]	,%%rbx		\n\t		movq		%[o6]	,%%r11		\n\t"\
		"movaps	%%xmm0,    (%%rax)		\n\t		movaps	%%xmm8 	,    (%%r10)	\n\t"\
		"movaps	%%xmm1,0x10(%%rax)		\n\t		movaps	%%xmm9 	,0x10(%%r10)	\n\t"\
		"movaps	%%xmm4,    (%%rbx)		\n\t		movaps	%%xmm12	,    (%%r11)	\n\t"\
		"movaps	%%xmm5,0x10(%%rbx)		\n\t		movaps	%%xmm13	,0x10(%%r11)	\n\t"\
		"movq		%[i1]	,%%rcx		\n\t		movq		%[i5]	,%%r12		\n\t"\
		"movq		%[i3]	,%%rdx		\n\t		movq		%[i7]	,%%r13		\n\t"\
		"movaps	    (%%rcx)	,%%xmm2		\n\t		movaps	    (%%r12)	,%%xmm10	\n\t"\
		"movaps	0x10(%%rcx)	,%%xmm3		\n\t		movaps	0x10(%%r12)	,%%xmm11	\n\t"\
		"movaps	%%xmm2		,%%xmm6		\n\t		movaps	%%xmm10		,%%xmm14	\n\t"\
		"movaps	%%xmm3		,%%xmm7		\n\t		movaps	%%xmm11		,%%xmm15	\n\t"\
		"addpd	    (%%rdx)	,%%xmm2		\n\t		subpd	0x10(%%r13)	,%%xmm10	\n\t"\
		"subpd	    (%%rdx)	,%%xmm6		\n\t		addpd	0x10(%%r13)	,%%xmm14	\n\t"\
		"addpd	0x10(%%rdx)	,%%xmm3		\n\t		addpd	    (%%r13)	,%%xmm11	\n\t"\
		"subpd	0x10(%%rdx)	,%%xmm7		\n\t		subpd	    (%%r13)	,%%xmm15	\n\t"\
		"movq		%[o1]	,%%rcx		\n\t		movq		%[o5]	,%%r12		\n\t"\
		"movq		%[o3]	,%%rdx		\n\t		movq		%[o7]	,%%r13		\n\t"\
		"subpd	%%xmm2		,%%xmm0		\n\t		movaps	%%xmm12	,    (%%r13)	\n\t"\
		"subpd	%%xmm3		,%%xmm1		\n\t		movaps	%%xmm13	,0x10(%%r13)	\n\t"\
	/* Use the cosine term of the [c1,s1] pair, which is the *middle* [4th of 7] of our 7 input pairs, in terms \
	of the input-arg bit-reversal reordering defined in the __X[c,s] --> [c,s] mapping below and happens to \
	always in fact *be* a true cosine term, which is a requirement for our "decr 1 gives isrt2" data-copy scheme: */\
		"										movq	%[c1],%%r14	\n\t"\
		"subpd	%%xmm7		,%%xmm4		\n\t	subq	$0x10,%%r14	\n\t"/* isrt2 in [c1]-1 */\
		"subpd	%%xmm6		,%%xmm5		\n\t		movaps	%%xmm10		,%%xmm13	\n\t"\
		"addpd	    (%%rax)	,%%xmm2		\n\t		subpd	%%xmm11		,%%xmm10	\n\t"\
		"addpd	0x10(%%rax)	,%%xmm3		\n\t		addpd	%%xmm11		,%%xmm13	\n\t"\
		"addpd	    (%%rbx)	,%%xmm7		\n\t		mulpd	    (%%r14)	,%%xmm10	\n\t"\
		"addpd	0x10(%%rbx)	,%%xmm6		\n\t		mulpd	    (%%r14)	,%%xmm13	\n\t"\
		"movaps	%%xmm2,    (%%rax) /* [o0].r */\n\t	movaps	0x10(%%r13)	,%%xmm11	\n\t"\
		"movaps	%%xmm3,0x10(%%rax) /* [o0].i */\n\t	movaps	%%xmm15		,%%xmm12	\n\t"\
		"movaps	%%xmm4,    (%%rbx) /* [o2].r */\n\t	addpd	%%xmm14		,%%xmm12	\n\t"\
		"movaps	%%xmm6,0x10(%%rbx) /* [o2].i */\n\t	subpd	%%xmm14		,%%xmm15	\n\t"\
		"movaps	%%xmm0,    (%%rcx) /* [o1].r */\n\t	mulpd	    (%%r14)	,%%xmm12	\n\t"\
		"movaps	%%xmm1,0x10(%%rcx) /* [o1].i */\n\t	mulpd	    (%%r14)	,%%xmm15	\n\t"\
		"movaps	%%xmm7,    (%%rdx) /* [o3].r */\n\t	movaps		(%%r13)	,%%xmm14	\n\t"\
		"movaps	%%xmm5,0x10(%%rdx) /* [o3].i */\n\t	subpd	%%xmm10		,%%xmm8 	\n\t"\
		"											subpd	%%xmm13		,%%xmm9 	\n\t"\
		"											subpd	%%xmm12		,%%xmm14	\n\t"\
		"											subpd	%%xmm15		,%%xmm11	\n\t"\
		"											addpd	    (%%r10)	,%%xmm10	\n\t"\
		"											addpd	0x10(%%r10)	,%%xmm13	\n\t"\
		"											addpd	    (%%r11)	,%%xmm12	\n\t"\
		"											addpd	0x10(%%r11)	,%%xmm15	\n\t"\
		"											movaps	%%xmm10,    (%%r10) \n\t"/* [o4].r */\
		"											movaps	%%xmm13,0x10(%%r10) \n\t"/* [o4].i */\
		"											movaps	%%xmm14,    (%%r11) \n\t"/* [o6].r */\
		"											movaps	%%xmm11,0x10(%%r11) \n\t"/* [o6].i */\
		"											movaps	%%xmm8 ,    (%%r12) \n\t"/* [o5].r */\
		"											movaps	%%xmm9 ,0x10(%%r12) \n\t"/* [o5].i */\
		"											movaps	%%xmm12,    (%%r13) \n\t"/* [o7].r */\
		"											movaps	%%xmm15,0x10(%%r13) \n\t"/* [o7].i */\
		:					/* outputs: none */\
		: [i0] "m" (Xi0)	/* All inputs from memory addresses here */\
		 ,[i1] "m" (Xi1)\
		 ,[i2] "m" (Xi2)\
		 ,[i3] "m" (Xi3)\
		 ,[i4] "m" (Xi4)\
		 ,[i5] "m" (Xi5)\
		 ,[i6] "m" (Xi6)\
		 ,[i7] "m" (Xi7)\
		 ,[o0] "m" (Xo0)\
		 ,[o1] "m" (Xo1)\
		 ,[o2] "m" (Xo2)\
		 ,[o3] "m" (Xo3)\
		 ,[o4] "m" (Xo4)\
		 ,[o5] "m" (Xo5)\
		 ,[o6] "m" (Xo6)\
		 ,[o7] "m" (Xo7)\
		 ,[c4] "m" (Xc1),[s4] "m" (Xs1)\
		 ,[c2] "m" (Xc2),[s2] "m" (Xs2)\
		 ,[c6] "m" (Xc3),[s6] "m" (Xs3)\
		 ,[c1] "m" (Xc4),[s1] "m" (Xs4)\
		 ,[c5] "m" (Xc5),[s5] "m" (Xs5)\
		 ,[c3] "m" (Xc6),[s3] "m" (Xs6)\
		 ,[c7] "m" (Xc7),[s7] "m" (Xs7)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","rdi","r10","r11","r12","r13","r14","r15","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"	/* Clobbered registers */\
	);\
	}

  #else

	#define SSE2_RADIX8_DIF_TWIDDLE_OOP(Xi0,Xi1,Xi2,Xi3,Xi4,Xi5,Xi6,Xi7, Xo0,Xo1,Xo2,Xo3,Xo4,Xo5,Xo6,Xo7 ,Xc1,Xs1,Xc2,Xs2,Xc3,Xs3,Xc4,Xs4,Xc5,Xs5,Xc6,Xs6,Xc7,Xs7)\
	{\
	__asm__ volatile (\
		/* Block 0,4: */\
		"movq		%[i0]	,%%rax			\n\t"\
		"movq		%[i4]	,%%rbx			\n\t"\
		"movq		%[c4]	,%%rcx			\n\t"\
		"movq		%[s4]	,%%rsi			\n\t"\
	/* [rsi] (and if needed rdi) points to sine components of each sincos pair, which is not really a pair here in terms of relative addressing: */\
		"movaps		    (%%rbx)	,%%xmm2		\n\t"\
		"movaps		0x10(%%rbx)	,%%xmm3		\n\t"\
		"movaps		(%%rcx)	,%%xmm0		\n\t"\
		"movaps		(%%rsi)	,%%xmm1		\n\t"\
		"movaps		%%xmm2	,%%xmm4		\n\t"\
		"movaps		%%xmm3	,%%xmm5		\n\t"\
		"mulpd		%%xmm0	,%%xmm2		\n\t"\
		"mulpd		%%xmm0	,%%xmm3		\n\t"\
		"mulpd		%%xmm1	,%%xmm4		\n\t"\
		"mulpd		%%xmm1	,%%xmm5		\n\t"\
		"movaps		    (%%rax)	,%%xmm0		\n\t"\
		"movaps		0x10(%%rax)	,%%xmm1		\n\t"\
		"subpd		%%xmm5		,%%xmm2		\n\t"\
		"addpd		%%xmm4		,%%xmm3		\n\t"\
		"movaps		%%xmm0	,%%xmm6		\n\t"\
		"movaps		%%xmm1	,%%xmm7		\n\t"\
		"addpd		%%xmm2		,%%xmm0		\n\t"\
		"addpd		%%xmm3		,%%xmm1		\n\t"\
		"subpd		%%xmm2		,%%xmm6		\n\t"\
		"subpd		%%xmm3		,%%xmm7		\n\t"\
		"movaps		%%xmm0		,    (%%rax)	\n\t"\
		"movaps		%%xmm1		,0x10(%%rax)	\n\t"\
		"movaps		%%xmm6		,    (%%rbx)	\n\t"\
		"movaps		%%xmm7		,0x10(%%rbx)	\n\t"\
		/* Block 2,6: */\
		"movq		%[i2]	,%%rax			\n\t"\
		"movq		%[i6]	,%%rbx			\n\t"\
		"movq		%[c2]	,%%rcx			\n\t"\
		"movq		%[c6]	,%%rdx			\n\t"\
		"movq		%[s2]	,%%rsi			\n\t"\
		"movq		%[s6]	,%%rdi			\n\t"\
		"movaps		    (%%rax)	,%%xmm0		\n\t"\
		"movaps		0x10(%%rax)	,%%xmm2		\n\t"\
		"movaps		(%%rcx)	,%%xmm6		\n\t"\
		"movaps		(%%rsi)	,%%xmm7		\n\t"\
		"movaps		%%xmm0	,%%xmm1		\n\t"\
		"movaps		%%xmm2	,%%xmm3		\n\t"\
		"mulpd		%%xmm6	,%%xmm0		\n\t"\
		"mulpd		%%xmm7	,%%xmm2		\n\t"\
		"mulpd		%%xmm7	,%%xmm1		\n\t"\
		"mulpd		%%xmm6	,%%xmm3		\n\t"\
		"subpd		%%xmm2		,%%xmm0		\n\t"\
		"addpd		%%xmm3		,%%xmm1		\n\t"\
		"movaps		    (%%rbx)	,%%xmm2		\n\t"\
		"movaps		0x10(%%rbx)	,%%xmm3		\n\t"\
		"movaps		(%%rdx)	,%%xmm6		\n\t"\
		"movaps		(%%rdi)	,%%xmm7		\n\t"\
		"movaps		%%xmm2	,%%xmm4		\n\t"\
		"movaps		%%xmm3	,%%xmm5		\n\t"\
		"mulpd		%%xmm6	,%%xmm2		\n\t"\
		"mulpd		%%xmm7	,%%xmm3		\n\t"\
		"mulpd		%%xmm7	,%%xmm4		\n\t"\
		"mulpd		%%xmm6	,%%xmm5		\n\t"\
		"subpd		%%xmm3		,%%xmm2		\n\t"\
		"addpd		%%xmm5		,%%xmm4		\n\t"\
		"movaps		%%xmm2		,%%xmm3		\n\t"\
		"movaps		%%xmm4		,%%xmm5		\n\t"\
		"addpd		%%xmm0		,%%xmm2		\n\t"\
		"subpd		%%xmm3		,%%xmm0		\n\t"\
		"addpd		%%xmm1		,%%xmm4		\n\t"\
		"subpd		%%xmm5		,%%xmm1		\n\t"\
		"movaps		%%xmm2		,    (%%rax)	\n\t"\
		"movaps		%%xmm4		,0x10(%%rax)	\n\t"\
		"movaps		%%xmm0		,    (%%rbx)	\n\t"\
		"movaps		%%xmm1		,0x10(%%rbx)	\n\t"\
		/* Block 1,5: */\
		"movq		%[i1]	,%%rax			\n\t"\
		"movq		%[i5]	,%%rbx			\n\t"\
		"movq		%[c1]	,%%rcx			\n\t"\
		"movq		%[c5]	,%%rdx			\n\t"\
		"movq		%[s1]	,%%rsi			\n\t"\
		"movq		%[s5]	,%%rdi			\n\t"\
		"movaps		    (%%rax)	,%%xmm0		\n\t"\
		"movaps		0x10(%%rax)	,%%xmm2		\n\t"\
		"movaps		(%%rcx)	,%%xmm6		\n\t"\
		"movaps		(%%rsi)	,%%xmm7		\n\t"\
		"movaps		%%xmm0	,%%xmm1		\n\t"\
		"movaps		%%xmm2	,%%xmm3		\n\t"\
		"mulpd		%%xmm6	,%%xmm0		\n\t"\
		"mulpd		%%xmm7	,%%xmm2		\n\t"\
		"mulpd		%%xmm7	,%%xmm1		\n\t"\
		"mulpd		%%xmm6	,%%xmm3		\n\t"\
		"subpd		%%xmm2		,%%xmm0		\n\t"\
		"addpd		%%xmm3		,%%xmm1		\n\t"\
		"movaps		    (%%rbx)	,%%xmm2		\n\t"\
		"movaps		0x10(%%rbx)	,%%xmm3		\n\t"\
		"movaps		(%%rdx)	,%%xmm6		\n\t"\
		"movaps		(%%rdi)	,%%xmm7		\n\t"\
		"movaps		%%xmm2	,%%xmm4		\n\t"\
		"movaps		%%xmm3	,%%xmm5		\n\t"\
		"mulpd		%%xmm6	,%%xmm2		\n\t"\
		"mulpd		%%xmm7	,%%xmm3		\n\t"\
		"mulpd		%%xmm7	,%%xmm4		\n\t"\
		"mulpd		%%xmm6	,%%xmm5		\n\t"\
		"subpd		%%xmm3		,%%xmm2		\n\t"\
		"addpd		%%xmm5		,%%xmm4		\n\t"\
		"movaps		%%xmm2		,%%xmm3		\n\t"\
		"movaps		%%xmm4		,%%xmm5		\n\t"\
		"addpd		%%xmm0		,%%xmm2		\n\t"\
		"subpd		%%xmm3		,%%xmm0		\n\t"\
		"addpd		%%xmm1		,%%xmm4		\n\t"\
		"subpd		%%xmm5		,%%xmm1		\n\t"\
		"movaps		%%xmm2		,    (%%rax)	\n\t"\
		"movaps		%%xmm4		,0x10(%%rax)	\n\t"\
		"movaps		%%xmm0		,    (%%rbx)	\n\t"\
		"movaps		%%xmm1		,0x10(%%rbx)	\n\t"\
		/* Block 3,7: */\
		"movq		%[i3]	,%%rax			\n\t"\
		"movq		%[i7]	,%%rbx			\n\t"\
		"movq		%[c3]	,%%rcx			\n\t"\
		"movq		%[c7]	,%%rdx			\n\t"\
		"movq		%[s3]	,%%rsi			\n\t"\
		"movq		%[s7]	,%%rdi			\n\t"\
		"movaps		    (%%rax)	,%%xmm0		\n\t"\
		"movaps		0x10(%%rax)	,%%xmm2		\n\t"\
		"movaps		(%%rcx)	,%%xmm6		\n\t"\
		"movaps		(%%rsi)	,%%xmm7		\n\t"\
		"movaps		%%xmm0	,%%xmm1		\n\t"\
		"movaps		%%xmm2	,%%xmm3		\n\t"\
		"mulpd		%%xmm6	,%%xmm0		\n\t"\
		"mulpd		%%xmm7	,%%xmm2		\n\t"\
		"mulpd		%%xmm7	,%%xmm1		\n\t"\
		"mulpd		%%xmm6	,%%xmm3		\n\t"\
		"subpd		%%xmm2		,%%xmm0		\n\t"\
		"addpd		%%xmm3		,%%xmm1		\n\t"\
		"movaps		    (%%rbx)	,%%xmm2		\n\t"\
		"movaps		0x10(%%rbx)	,%%xmm3		\n\t"\
		"movaps		(%%rdx)	,%%xmm6		\n\t"\
		"movaps		(%%rdi)	,%%xmm7		\n\t"\
		"movaps		%%xmm2	,%%xmm4		\n\t"\
		"movaps		%%xmm3	,%%xmm5		\n\t"\
		"mulpd		%%xmm6	,%%xmm2		\n\t"\
		"mulpd		%%xmm7	,%%xmm3		\n\t"\
		"mulpd		%%xmm7	,%%xmm4		\n\t"\
		"mulpd		%%xmm6	,%%xmm5		\n\t"\
		"subpd		%%xmm3		,%%xmm2		\n\t"\
		"addpd		%%xmm5		,%%xmm4		\n\t"\
		"movaps		%%xmm2		,%%xmm3		\n\t"\
		"movaps		%%xmm4		,%%xmm5		\n\t"\
		"addpd		%%xmm0		,%%xmm2		\n\t"\
		"subpd		%%xmm3		,%%xmm0		\n\t"\
		"addpd		%%xmm1		,%%xmm4		\n\t"\
		"subpd		%%xmm5		,%%xmm1		\n\t"\
		"movaps		%%xmm2		,    (%%rax)	\n\t"\
		"movaps		%%xmm4		,0x10(%%rax)	\n\t"\
		"movaps		%%xmm0		,    (%%rbx)	\n\t"\
		"movaps		%%xmm1		,0x10(%%rbx)	\n\t"\
		/* Now combine the 2 half-transforms: */\
		/* Combo 1: */\
		"movq		%[i0]	,%%rax			\n\t"\
		"movq		%[i2]	,%%rbx			\n\t"\
		"movaps		    (%%rax)	,%%xmm0		\n\t"\
		"movaps		0x10(%%rax)	,%%xmm1		\n\t"\
		"movaps		    (%%rbx)	,%%xmm2		\n\t"\
		"movaps		0x10(%%rbx)	,%%xmm3		\n\t"\
		"movaps		%%xmm0		,%%xmm4		\n\t"\
		"movaps		%%xmm1		,%%xmm5		\n\t"\
		"addpd		%%xmm2	,%%xmm0		\n\t"\
		"subpd		%%xmm2	,%%xmm4		\n\t"\
		"addpd		%%xmm3	,%%xmm1		\n\t"\
		"subpd		%%xmm3	,%%xmm5		\n\t"\
		"movaps		%%xmm0		,    (%%rax)	\n\t"\
		"movaps		%%xmm1		,0x10(%%rax)	\n\t"\
		"movaps		%%xmm4		,    (%%rbx)	\n\t"\
		"movaps		%%xmm5		,0x10(%%rbx)	\n\t"\
		"movq		%[i1]	,%%rcx			\n\t"\
		"movq		%[i3]	,%%rdx			\n\t"\
		"movaps		    (%%rcx)	,%%xmm2		\n\t"\
		"movaps		0x10(%%rcx)	,%%xmm3		\n\t"\
		"movaps		    (%%rdx)	,%%xmm8		\n\t"/* Use xtra 64-bit reg xmm8 here */\
		"movaps		0x10(%%rdx)	,%%xmm9		\n\t"/* Use xtra 64-bit reg xmm9 here */\
		"movq		%[o1]	,%%rcx			\n\t"\
		"movq		%[o3]	,%%rdx			\n\t"\
		"movaps		%%xmm2		,%%xmm6		\n\t"\
		"movaps		%%xmm3		,%%xmm7		\n\t"\
		"addpd		%%xmm8	,%%xmm2		\n\t"\
		"subpd		%%xmm8	,%%xmm6		\n\t"\
		"addpd		%%xmm9	,%%xmm3		\n\t"\
		"subpd		%%xmm9	,%%xmm7		\n\t"\
		"subpd		%%xmm2		,%%xmm0		\n\t"\
		"subpd		%%xmm3		,%%xmm1		\n\t"\
		"subpd		%%xmm7		,%%xmm4		\n\t"\
		"subpd		%%xmm6		,%%xmm5		\n\t"\
		"addpd			(%%rax)	,%%xmm2		\n\t"\
		"addpd		0x10(%%rax)	,%%xmm3		\n\t"\
		"addpd			(%%rbx)	,%%xmm7		\n\t"\
		"addpd		0x10(%%rbx)	,%%xmm6		\n\t"\
		"movq		%[o0]	,%%rax			\n\t"\
		"movq		%[o2]	,%%rbx			\n\t"\
		"movaps		%%xmm2		,    (%%rax)	\n\t"/* [o0].re */\
		"movaps		%%xmm3		,0x10(%%rax)	\n\t"/* [o0].im */\
		"movaps		%%xmm4		,    (%%rbx)	\n\t"/* [o2].re */\
		"movaps		%%xmm6		,0x10(%%rbx)	\n\t"/* [o2].im */\
		"movaps		%%xmm0		,    (%%rcx)	\n\t"/* [o1].re */\
		"movaps		%%xmm1		,0x10(%%rcx)	\n\t"/* [o1].im */\
		"movaps		%%xmm7		,    (%%rdx)	\n\t"/* [o3].re */\
		"movaps		%%xmm5		,0x10(%%rdx)	\n\t"/* [o3].im */\
		/* Combo 2: */\
		"movq		%[i4]	,%%rax			\n\t"\
		"movq		%[i6]	,%%rbx			\n\t"\
		"movaps		    (%%rax)	,%%xmm0		\n\t"\
		"movaps		0x10(%%rax)	,%%xmm1		\n\t"\
		"movaps		    (%%rbx)	,%%xmm2		\n\t"\
		"movaps		0x10(%%rbx)	,%%xmm3		\n\t"\
		"movaps		%%xmm0		,%%xmm4		\n\t"\
		"movaps		%%xmm1		,%%xmm5		\n\t"\
		"subpd		%%xmm3	,%%xmm0		\n\t"\
		"addpd		%%xmm3	,%%xmm4		\n\t"\
		"addpd		%%xmm2	,%%xmm1		\n\t"\
		"subpd		%%xmm2	,%%xmm5		\n\t"\
		"movaps		%%xmm0		,    (%%rax)	\n\t"\
		"movaps		%%xmm1		,0x10(%%rax)	\n\t"\
		"movaps		%%xmm4		,    (%%rbx)	\n\t"\
		"movaps		%%xmm5		,0x10(%%rbx)	\n\t"\
		"movq		%[i5]	,%%rcx			\n\t"\
		"movq		%[i7]	,%%rdx			\n\t"\
		"movaps		    (%%rcx)	,%%xmm2		\n\t"\
		"movaps		0x10(%%rcx)	,%%xmm3		\n\t"\
		"movaps		    (%%rdx)	,%%xmm8		\n\t"/* Use xtra 64-bit reg xmm8 here */\
		"movaps		0x10(%%rdx)	,%%xmm9		\n\t"/* Use xtra 64-bit reg xmm9 here */\
		"movaps		%%xmm2		,%%xmm6		\n\t"\
		"movaps		%%xmm3		,%%xmm7		\n\t"\
		"subpd		%%xmm9	,%%xmm2		\n\t"\
		"addpd		%%xmm9	,%%xmm6		\n\t"\
		"addpd		%%xmm8	,%%xmm3		\n\t"\
		"subpd		%%xmm8	,%%xmm7		\n\t"\
		"movaps		%%xmm4		,    (%%rdx)	\n\t"\
		"movaps		%%xmm5		,0x10(%%rdx)	\n\t"\
	/* Use the cosine term of the [c1,s1] pair, which is the *middle* [4th of 7] of our 7 input pairs, in terms \
	of the input-arg bit-reversal reordering defined in the __X[c,s] --> [c,s] mapping below and happens to \
	always in fact *be* a true cosine term, which is a requirement for our "decr 1 gives isrt2" data-copy scheme: */\
		"movq		%[c1],%%rsi			\n\t"/* isrt2 in [c1]-1 */\
		"movaps	-0x10(%%rsi),%%xmm8		\n\t"/* Use xtra 64-bit reg xmm8 here */\
		"movaps		%%xmm2		,%%xmm5		\n\t"\
		"subpd		%%xmm3		,%%xmm2		\n\t"\
		"addpd		%%xmm3		,%%xmm5		\n\t"\
		"mulpd			%%xmm8	,%%xmm2		\n\t"\
		"mulpd			%%xmm8	,%%xmm5		\n\t"\
		"movaps		0x10(%%rdx)	,%%xmm3		\n\t"\
		"movaps		%%xmm7		,%%xmm4		\n\t"\
		"addpd		%%xmm6		,%%xmm4		\n\t"\
		"subpd		%%xmm6		,%%xmm7		\n\t"\
		"mulpd			%%xmm8	,%%xmm4		\n\t"\
		"mulpd			%%xmm8	,%%xmm7		\n\t"\
		"movaps			(%%rdx)	,%%xmm6		\n\t"/* last ref to rdx-for-in-address */\
		"movq		%[o5]	,%%rcx			\n\t"\
		"movq		%[o7]	,%%rdx			\n\t"\
		"subpd		%%xmm2		,%%xmm0		\n\t"\
		"subpd		%%xmm5		,%%xmm1		\n\t"\
		"subpd		%%xmm4		,%%xmm6		\n\t"\
		"subpd		%%xmm7		,%%xmm3		\n\t"\
		"addpd			(%%rax)	,%%xmm2		\n\t"\
		"addpd		0x10(%%rax)	,%%xmm5		\n\t"\
		"addpd			(%%rbx)	,%%xmm4		\n\t"\
		"addpd		0x10(%%rbx)	,%%xmm7		\n\t"\
		"movq		%[o4]	,%%rax			\n\t"\
		"movq		%[o6]	,%%rbx			\n\t"\
		"movaps		%%xmm2		,    (%%rax)	\n\t"/* [o4].re */\
		"movaps		%%xmm5		,0x10(%%rax)	\n\t"/* [o4].im */\
		"movaps		%%xmm6		,    (%%rbx)	\n\t"/* [o6].re */\
		"movaps		%%xmm3		,0x10(%%rbx)	\n\t"/* [o6].im */\
		"movaps		%%xmm0		,    (%%rcx)	\n\t"/* [o5].re */\
		"movaps		%%xmm1		,0x10(%%rcx)	\n\t"/* [o5].im */\
		"movaps		%%xmm4		,    (%%rdx)	\n\t"/* [o7].re */\
		"movaps		%%xmm7		,0x10(%%rdx)	\n\t"/* [o7].im */\
		:					/* outputs: none */\
		: [i0] "m" (Xi0)	/* All inputs from memory addresses here */\
		 ,[i1] "m" (Xi1)\
		 ,[i2] "m" (Xi2)\
		 ,[i3] "m" (Xi3)\
		 ,[i4] "m" (Xi4)\
		 ,[i5] "m" (Xi5)\
		 ,[i6] "m" (Xi6)\
		 ,[i7] "m" (Xi7)\
		 ,[o0] "m" (Xo0)\
		 ,[o1] "m" (Xo1)\
		 ,[o2] "m" (Xo2)\
		 ,[o3] "m" (Xo3)\
		 ,[o4] "m" (Xo4)\
		 ,[o5] "m" (Xo5)\
		 ,[o6] "m" (Xo6)\
		 ,[o7] "m" (Xo7)\
		 ,[c4] "m" (Xc1),[s4] "m" (Xs1)\
		 ,[c2] "m" (Xc2),[s2] "m" (Xs2)\
		 ,[c6] "m" (Xc3),[s6] "m" (Xs3)\
		 ,[c1] "m" (Xc4),[s1] "m" (Xs4)\
		 ,[c5] "m" (Xc5),[s5] "m" (Xs5)\
		 ,[c3] "m" (Xc6),[s3] "m" (Xs6)\
		 ,[c7] "m" (Xc7),[s7] "m" (Xs7)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","rdi","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9"	/* Clobbered registers */\
	);\
	}

  #endif	// 8/16-reg version

	// SSE2 analog of dft_macro.h::RADIX_08_DIT_TWIDDLE_OOP - Result of sign-flippage and adding separate I/O addressing to
	// radix8_dif_dit_pass_gcc64.h::SSE2_RADIX8_DIF_TWIDDLE. We begin with the DIF macro here because we need a pre-twiddles
	// implementation for our purposes, whereas SSE2_RADIX8_DIT_TWIDDLE is post-twiddles.
	//
	// SIMD Opcount: 102 load/store [30 implicit], 66 add/sub, 50 mul. Compare to DFT macros used for radix-8-pass-with-twiddles:
	// DIF opcount : 140 load/store [56 implicit], 66 add/sub, 32 mul
	// DIT opcount :  85 load/store [36 implicit], 68 add/sub, 32 mul .
	//
	#define SSE2_RADIX8_DIT_TWIDDLE_OOP(Xi0,Xi1,Xi2,Xi3,Xi4,Xi5,Xi6,Xi7, Xo0,Xo1,Xo2,Xo3,Xo4,Xo5,Xo6,Xo7 ,Xc1,Xs1,Xc2,Xs2,Xc3,Xs3,Xc4,Xs4,Xc5,Xs5,Xc6,Xs6,Xc7,Xs7)\
	{\
	__asm__ volatile (\
	/* Block 0/1 has just one twiddle-CMUL: */\
		"movq		%[i0],%%rax			\n\t"\
		"movq		%[i1],%%rbx			\n\t"\
		"movq		%[c1],%%rdi			\n\t"/* [rdi,rsi] point to [cos,sin] components of each sincos pair, */\
		"movq		%[s1],%%rsi			\n\t"/* which is not really a pair here in terms of relative addressing: */\
		"movaps		    (%%rbx),%%xmm4 	\n\t	movaps		0x10(%%rbx),%%xmm5 	\n\t"/* _r4  = __tr1;	_r5  = __ti1; */\
		"movaps		    (%%rax),%%xmm0 	\n\t	movaps		0x10(%%rax),%%xmm1 	\n\t"/* _r0  = __tr0;	_r1  = __ti0; */\
		"movaps		%%xmm5 ,%%xmm6 		\n\t	movaps		%%xmm4 ,%%xmm7 		\n\t"/* _r6  = _r5;		_r7  = _r4;		** [r4,r5] = CMUL(__t1,__W1): */\
		"mulpd		(%%rdi),%%xmm4 		\n\t	mulpd		(%%rdi),%%xmm5 		\n\t"/* _r4 *= __Wr1;	_r5 *= __Wr1; */\
		"mulpd		(%%rsi),%%xmm6 		\n\t	mulpd		(%%rsi),%%xmm7 		\n\t"/* _r6 *= __Wi1;	_r7 *= __Wi1; */\
		"addpd		%%xmm6 ,%%xmm4 		\n\t	subpd		%%xmm7 ,%%xmm5 		\n\t"/* _r4 += _r6;		_r5 -= _r7; */\
		"movaps		%%xmm0 ,%%xmm2 		\n\t	movaps		%%xmm1 ,%%xmm3 		\n\t"/* _r2  = _r0;		_r3  = _r1; */\
		"addpd		%%xmm4 ,%%xmm0 		\n\t	addpd		%%xmm5 ,%%xmm1 		\n\t"/* _r0 += _r4;		_r1 += _r5; */\
		"subpd		%%xmm4 ,%%xmm2 		\n\t	subpd		%%xmm5 ,%%xmm3 		\n\t"/* _r2 -= _r4;		_r3 -= _r5; */\
		"movaps		%%xmm0 ,    (%%rax)	\n\t	movaps		%%xmm1 ,0x10(%%rax)	\n\t"/* __tr0 = _r0;	__ti0 = _r1; */\
		"movaps		%%xmm2 ,    (%%rbx)	\n\t	movaps		%%xmm3 ,0x10(%%rbx)	\n\t"/* __tr1 = _r2;	__ti1 = _r3; */\
	/* Blocks 2/3 use separate register subset, can be done overlapped with 0/1: */\
		"movq		%[i2],%%rcx			\n\t"\
		"movq		%[c2],%%r10			\n\t"\
		"movq		%[s2],%%r11			\n\t"/* [r8,r9] = CMUL(__t2,__W2): */\
		"movaps		    (%%rcx),%%xmm8 	\n\t	movaps		0x10(%%rcx),%%xmm9 	\n\t"/* _r8  = __tr2;	_r9  = __ti2; */\
		"movaps		%%xmm9 ,%%xmm10		\n\t	movaps		%%xmm8 ,%%xmm11		\n\t"/* _ra  = _r9;		_rb  = _r8; */\
		"mulpd		(%%r10),%%xmm8 		\n\t	mulpd		(%%r10),%%xmm9 		\n\t"/* _r8 *= __Wr2;	_r9 *= __Wr2; */\
		"mulpd		(%%r11),%%xmm10		\n\t	mulpd		(%%r11),%%xmm11		\n\t"/* _ra *= __Wi2;	_rb *= __Wi2; */\
		"addpd		%%xmm10,%%xmm8 		\n\t	subpd		%%xmm11,%%xmm9 		\n\t"/* _r8 += _ra;		_r9 -= _rb; */\
		"movq		%[i3],%%rdx			\n\t"\
		"movq		%[c3],%%r12			\n\t"\
		"movq		%[s3],%%r13			\n\t"/* [rc,rd] = CMUL(__t3,__W3): */\
		"movaps		    (%%rdx),%%xmm12	\n\t	movaps		0x10(%%rdx),%%xmm13	\n\t"/* _rc  = __tr3;	_rd  = __ti3; */\
		"movaps		%%xmm13,%%xmm14		\n\t	movaps		%%xmm12,%%xmm15		\n\t"/* _re  = _rd;		_rf  = _rc; */\
		"mulpd		(%%r12),%%xmm12		\n\t	mulpd		(%%r12),%%xmm13		\n\t"/* _rc *= __Wr3;	_rd *= __Wr3; */\
		"mulpd		(%%r13),%%xmm14		\n\t	mulpd		(%%r13),%%xmm15		\n\t"/* _re *= __Wi3;	_rf *= __Wi3; */\
		"addpd		%%xmm14,%%xmm12		\n\t	subpd		%%xmm15,%%xmm13		\n\t"/* _rc += _re;		_rd -= _rf; */\
		/* Now do radix-2 butterfly: */\
		"movaps		%%xmm8 ,%%xmm10		\n\t	movaps		%%xmm9 ,%%xmm11		\n\t"/* _ra  = _r8;		_rb  = _r9; */\
		"addpd		%%xmm12,%%xmm8 		\n\t	addpd		%%xmm13,%%xmm9 		\n\t"/* _r8 += _rc;		_r9 += _rd; */\
		"subpd		%%xmm12,%%xmm10		\n\t	subpd		%%xmm13,%%xmm11		\n\t"/* _ra -= _rc;		_rb -= _rd; */\
		"movaps		%%xmm8 ,    (%%rcx)	\n\t	movaps		%%xmm9 ,0x10(%%rcx)	\n\t"/* __tr2 = _r8;	__ti2 = _r9; */\
		"movaps		%%xmm10,    (%%rdx)	\n\t	movaps		%%xmm11,0x10(%%rdx)	\n\t"/* __tr3 = _ra;	__ti3 = _rb; */\
	/* Blocks 4/5: */\
		"movq		%[i4],%%rax			\n\t"\
		"movq		%[c4],%%rdi			\n\t"\
		"movq		%[s4],%%rsi			\n\t"/* [r0,r1] = CMUL(__t4,__W4): */\
		"movaps		    (%%rax),%%xmm0 	\n\t	movaps		0x10(%%rax),%%xmm1 	\n\t"/* _r0  = __tr4;	_r1  = __ti4; */\
		"movaps		%%xmm1 ,%%xmm2 		\n\t	movaps		%%xmm0 ,%%xmm3 		\n\t"/* _r2  = _r1;		_r3  = _r0; */\
		"mulpd		(%%rdi),%%xmm0 		\n\t	mulpd		(%%rdi),%%xmm1 		\n\t"/* _r0 *= __Wr4;	_r1 *= __Wr4; */\
		"mulpd		(%%rsi),%%xmm2 		\n\t	mulpd		(%%rsi),%%xmm3 		\n\t"/* _r2 *= __Wi4;	_r3 *= __Wi4; */\
		"addpd		%%xmm2 ,%%xmm0 		\n\t	subpd		%%xmm3 ,%%xmm1 		\n\t"/* _r0 += _r2;		_r1 -= _r3; */\
		"movq		%[i5],%%rbx			\n\t"\
		"movq		%[c5],%%r8 			\n\t"\
		"movq		%[s5],%%r9 			\n\t"/* [r4,r5] = CMUL(__t5,__W5): */\
		"movaps		    (%%rbx),%%xmm4 	\n\t	movaps		0x10(%%rbx),%%xmm5 	\n\t"/* _r4  = __tr5;	_r5  = __ti5; */\
		"movaps		%%xmm5 ,%%xmm6 		\n\t	movaps		%%xmm4 ,%%xmm7 		\n\t"/* _r6  = _r5;		_r7  = _r4; */\
		"mulpd		(%%r8 ),%%xmm4 		\n\t	mulpd		(%%r8 ),%%xmm5 		\n\t"/* _r4 *= __Wr5;	_r5 *= __Wr5; */\
		"mulpd		(%%r9 ),%%xmm6 		\n\t	mulpd		(%%r9 ),%%xmm7 		\n\t"/* _r6 *= __Wi5;	_r7 *= __Wi5; */\
		"addpd		%%xmm6 ,%%xmm4 		\n\t	subpd		%%xmm7 ,%%xmm5 		\n\t"/* _r4 += _r6;		_r5 -= _r7; */\
		/* Now do radix-2 butterfly: */\
		"movaps		%%xmm0 ,%%xmm2 		\n\t	movaps		%%xmm1 ,%%xmm3 		\n\t"/* _r2  = _r0;		_r3  = _r1; */\
		"addpd		%%xmm4 ,%%xmm0 		\n\t	addpd		%%xmm5 ,%%xmm1 		\n\t"/* _r0 += _r4;		_r1 += _r5; */\
		"subpd		%%xmm4 ,%%xmm2 		\n\t	subpd		%%xmm5 ,%%xmm3 		\n\t"/* _r2 -= _r4;		_r3 -= _r5; */\
	/* Blocks 6/7 use separate register subset, can be done overlapped with 4/5: */\
		"movq		%[i6],%%rcx			\n\t"\
		"movq		%[c6],%%r10			\n\t"\
		"movq		%[s6],%%r11			\n\t"/* [r8,r9] = CMUL(__t6,__W6): */\
		"movaps		    (%%rcx),%%xmm8 	\n\t	movaps		0x10(%%rcx),%%xmm9 	\n\t"/* _r8  = __tr6;	_r9  = __ti6; */\
		"movaps		%%xmm9 ,%%xmm10		\n\t	movaps		%%xmm8 ,%%xmm11		\n\t"/* _ra  = _r9;		_rb  = _r8; */\
		"mulpd		(%%r10),%%xmm8 		\n\t	mulpd		(%%r10),%%xmm9 		\n\t"/* _r8 *= __Wr6;	_r9 *= __Wr6; */\
		"mulpd		(%%r11),%%xmm10		\n\t	mulpd		(%%r11),%%xmm11		\n\t"/* _ra *= __Wi6;	_rb *= __Wi6; */\
		"addpd		%%xmm10,%%xmm8 		\n\t	subpd		%%xmm11,%%xmm9 		\n\t"/* _r8 += _ra;		_r9 -= _rb; */\
		"movq		%[i7],%%rdx			\n\t"\
		"movq		%[c7],%%r12			\n\t"\
		"movq		%[s7],%%r13			\n\t"/* [rc,rd] = CMUL(__t7,__W7): */\
		"movaps		    (%%rdx),%%xmm12	\n\t	movaps		0x10(%%rdx),%%xmm13	\n\t"/* _rc  = __tr7;	_rd  = __ti7; */\
		"movaps		%%xmm13,%%xmm14		\n\t	movaps		%%xmm12,%%xmm15		\n\t"/* _re  = _rd;		_rf  = _rc; */\
		"mulpd		(%%r12),%%xmm12		\n\t	mulpd		(%%r12),%%xmm13		\n\t"/* _rc *= __Wr7;	_rd *= __Wr7; */\
		"mulpd		(%%r13),%%xmm14		\n\t	mulpd		(%%r13),%%xmm15		\n\t"/* _re *= __Wi7;	_rf *= __Wi7; */\
		"addpd		%%xmm14,%%xmm12		\n\t	subpd		%%xmm15,%%xmm13		\n\t"/* _rc += _re;		_rd -= _rf; */\
		/* Now do radix-2 butterfly: */\
		"movaps		%%xmm8 ,%%xmm10		\n\t	movaps		%%xmm9 ,%%xmm11		\n\t"/* _ra  = _r8;		_rb  = _r9; */\
		"addpd		%%xmm12,%%xmm8 		\n\t	addpd		%%xmm13,%%xmm9 		\n\t"/* _r8 += _rc;		_r9 += _rd; */\
		"subpd		%%xmm12,%%xmm10		\n\t	subpd		%%xmm13,%%xmm11		\n\t"/* _ra -= _rc;		_rb -= _rd; */\
	/* Reload Block 0-3 outputs into r4-7,c-f, combine to get the 2 length-4 subtransform... */\
		"movq		%[i0],%%rax			\n\t"\
		"movq		%[i1],%%rbx			\n\t"\
		"movq		%[i2],%%rcx			\n\t"\
		"movq		%[i3],%%rdx			\n\t"\
		"movaps		    (%%rax),%%xmm4 	\n\t	movaps		0x10(%%rax),%%xmm5 	\n\t"/* _r4 = __tr0;	_r5 = __ti0; */\
		"movaps		    (%%rbx),%%xmm6 	\n\t	movaps		0x10(%%rbx),%%xmm7 	\n\t"/* _r6 = __tr1;	_r7 = __ti1; */\
		"movaps		    (%%rcx),%%xmm12	\n\t	movaps		0x10(%%rcx),%%xmm13	\n\t"/* _rc = __tr2;	_rd = __ti2; */\
		"movaps		    (%%rdx),%%xmm14	\n\t	movaps		0x10(%%rdx),%%xmm15	\n\t"/* _re = __tr3;	_rf = __ti3; */\
		"movq		%[o0],%%rax			\n\t"/* Assumes user stuck a (vec_dbl)2.0 into this output slot prior to macro call. */\
		"subpd		%%xmm12,%%xmm4 		\n\t	subpd		%%xmm13,%%xmm5 		\n\t"/* _r4 -= _rc;		_r5 -= _rd; */\
		"subpd		%%xmm15,%%xmm6 		\n\t	subpd		%%xmm14,%%xmm7 		\n\t"/* _r6 -= _rf;		_r7 -= _re; */\
		"subpd		%%xmm8 ,%%xmm0 		\n\t	subpd		%%xmm9 ,%%xmm1 		\n\t"/* _r0 -= _r8;		_r1 -= _r9; */\
		"subpd		%%xmm11,%%xmm2 		\n\t	subpd		%%xmm10,%%xmm3 		\n\t"/* _r2 -= _rb;		_r3 -= _ra; */\
		/* We hope the microcode execution engine sticks the datum at (%%rax) into a virtual register and inlines the MULs with the above SUBs: */\
		"mulpd		(%%rax),%%xmm12		\n\t	mulpd		(%%rax),%%xmm13		\n\t"/* _rc *= _two;	_rd *= _two; */\
		"mulpd		(%%rax),%%xmm15		\n\t	mulpd		(%%rax),%%xmm14		\n\t"/* _rf *= _two;	_re *= _two; */\
		"mulpd		(%%rax),%%xmm8 		\n\t	mulpd		(%%rax),%%xmm9 		\n\t"/* _r8 *= _two;	_r9 *= _two; */\
		"mulpd		(%%rax),%%xmm11		\n\t	mulpd		(%%rax),%%xmm10		\n\t"/* _rb *= _two;	_ra *= _two; */\
		"addpd		%%xmm4 ,%%xmm12		\n\t	addpd		%%xmm5 ,%%xmm13		\n\t"/* _rc += _r4;		_rd += _r5; */\
		"addpd		%%xmm6 ,%%xmm15		\n\t	addpd		%%xmm7 ,%%xmm14		\n\t"/* _rf += _r6;		_re += _r7; */\
		"addpd		%%xmm0 ,%%xmm8 		\n\t	addpd		%%xmm1 ,%%xmm9 		\n\t"/* _r8 += _r0;		_r9 += _r1; */\
		"addpd		%%xmm2 ,%%xmm11		\n\t	addpd		%%xmm3 ,%%xmm10		\n\t"/* _rb += _r2;		_ra += _r3; */\
		/* In terms of our original scalar-code prototyping macro, the data are: __tr0 = _r[c,f,4,6,8,b,0,2], __ti0 = _r[d,7,5,e,9,3,1,a]; */\
	/* Now combine the two half-transforms: */\
		/* Need r2/3 +- a/b combos for the *ISRT2 preceding the output 4-7 radix-2 butterflies, so start them first: */\
		"subpd		%%xmm3 ,%%xmm11		\n\t	subpd		%%xmm10,%%xmm2 		\n\t"/* _rb -= _r3;		_r2 -= _ra; */\
		"subpd		%%xmm8 ,%%xmm12		\n\t	subpd		%%xmm9 ,%%xmm13		\n\t"/* _rc -= _r8;		_rd -= _r9; */\
		"subpd		%%xmm1 ,%%xmm4 		\n\t	subpd		%%xmm0 ,%%xmm5 		\n\t"/* _r4 -= _r1;		_r5 -= _r0; */\
		"mulpd		(%%rax),%%xmm3 		\n\t	mulpd		(%%rax),%%xmm10		\n\t"/* _r3 *= _two;	_ra *= _two; */\
		"mulpd		(%%rax),%%xmm8 		\n\t	mulpd		(%%rax),%%xmm9 		\n\t"/* _r8 *= _two;	_r9 *= _two; */\
		"mulpd		(%%rax),%%xmm1 		\n\t	mulpd		(%%rax),%%xmm0 		\n\t"/* _r1 *= _two;	_r0 *= _two; */\
		"addpd		%%xmm11,%%xmm3 		\n\t	addpd		%%xmm2 ,%%xmm10		\n\t"/* _r3 += _rb;		_ra += _r2; */\
		"addpd		%%xmm12,%%xmm8 		\n\t	addpd		%%xmm13,%%xmm9 		\n\t"/* _r8 += _rc;		_r9 += _rd; */\
		"addpd		%%xmm4 ,%%xmm1 		\n\t	addpd		%%xmm5 ,%%xmm0 		\n\t"/* _r1 += _r4;		_r0 += _r5; */\
		/*movq		%[o0],%%rax		[o0] already in rax */	\
		"movq		%[o1],%%rbx			\n\t"\
		"movq		%[o2],%%rcx			\n\t"\
		"movq		%[o3],%%rdx			\n\t"\
		"movaps		%%xmm12,    (%%rbx)	\n\t	movaps		%%xmm13,0x10(%%rbx)	\n\t"/* __Br1 = _rc;	__Bi1 = _rd; */\
		/* Use that _rc,d free to stick 2.0 into _rc and that [c4] in rdi to load ISRT2 from c4-1 into _rd: */\
		"movaps		    (%%rax),%%xmm12	\n\t	movaps		-0x10(%%rdi),%%xmm13\n\t"/* _rc = 2.0;		_rd = ISRT2; */\
		"movaps		%%xmm4 ,    (%%rdx)	\n\t	movaps		%%xmm0 ,0x10(%%rdx)	\n\t"/* __Br3 = _r4;	__Bi3 = _r0; */\
		"movaps		%%xmm8 ,    (%%rax)	\n\t	movaps		%%xmm9 ,0x10(%%rax)	\n\t"/* __Br0 = _r8;	__Bi0 = _r9; */\
		"movaps		%%xmm1 ,    (%%rcx)	\n\t	movaps		%%xmm5 ,0x10(%%rcx)	\n\t"/* __Br2 = _r1;	__Bi2 = _r5; */\
		"mulpd		%%xmm13,%%xmm3 		\n\t	mulpd		%%xmm13,%%xmm11		\n\t"/* _r3 *= ISRT2;	_rb *= ISRT2; */\
		"mulpd		%%xmm13,%%xmm2 		\n\t	mulpd		%%xmm13,%%xmm10		\n\t"/* _r2 *= ISRT2;	_ra *= ISRT2; */\
		"subpd		%%xmm3 ,%%xmm15		\n\t	subpd		%%xmm11,%%xmm7 		\n\t"/* _rf -= _r3;		_r7 -= _rb; */\
		"subpd		%%xmm2 ,%%xmm6 		\n\t	subpd		%%xmm10,%%xmm14		\n\t"/* _r6 -= _r2;		_re -= _ra; */\
		"mulpd		%%xmm12,%%xmm3 		\n\t	mulpd		%%xmm12,%%xmm11		\n\t"/* _r3 *= _two;	_rb *= _two; */\
		"mulpd		%%xmm12,%%xmm2 		\n\t	mulpd		%%xmm12,%%xmm10		\n\t"/* _r2 *= _two;	_ra *= _two; */\
		"addpd		%%xmm15,%%xmm3 		\n\t	addpd		%%xmm7 ,%%xmm11		\n\t"/* _r3 += _rf;		_rb += _r7; */\
		"addpd		%%xmm6 ,%%xmm2 		\n\t	addpd		%%xmm14,%%xmm10		\n\t"/* _r2 += _r6;		_ra += _re; */\
		"movq		%[o4],%%rax			\n\t"\
		"movq		%[o5],%%rbx			\n\t"\
		"movq		%[o6],%%rcx			\n\t"\
		"movq		%[o7],%%rdx			\n\t"\
		"movaps		%%xmm3 ,    (%%rax)	\n\t	movaps		%%xmm7 ,0x10(%%rax)	\n\t"/* __Br4 = _r3;	__Bi4 = _r7; */\
		"movaps		%%xmm15,    (%%rbx)	\n\t	movaps		%%xmm11,0x10(%%rbx)	\n\t"/* __Br5 = _rf;	__Bi5 = _rb; */\
		"movaps		%%xmm6 ,    (%%rcx)	\n\t	movaps		%%xmm14,0x10(%%rcx)	\n\t"/* __Br6 = _r6;	__Bi6 = _re; */\
		"movaps		%%xmm2 ,    (%%rdx)	\n\t	movaps		%%xmm10,0x10(%%rdx)	\n\t"/* __Br7 = _r2;	__Bi7 = _ra; */\
		:					/* outputs: none */\
		: [i0] "m" (Xi0)	/* All inputs from memory addresses here */\
		 ,[i1] "m" (Xi1)\
		 ,[i2] "m" (Xi2)\
		 ,[i3] "m" (Xi3)\
		 ,[i4] "m" (Xi4)\
		 ,[i5] "m" (Xi5)\
		 ,[i6] "m" (Xi6)\
		 ,[i7] "m" (Xi7)\
		 ,[o0] "m" (Xo0)\
		 ,[o1] "m" (Xo1)\
		 ,[o2] "m" (Xo2)\
		 ,[o3] "m" (Xo3)\
		 ,[o4] "m" (Xo4)\
		 ,[o5] "m" (Xo5)\
		 ,[o6] "m" (Xo6)\
		 ,[o7] "m" (Xo7)\
		 ,[c1] "m" (Xc1),[s1] "m" (Xs1)\
		 ,[c2] "m" (Xc2),[s2] "m" (Xs2)\
		 ,[c3] "m" (Xc3),[s3] "m" (Xs3)\
		 ,[c4] "m" (Xc4),[s4] "m" (Xs4)\
		 ,[c5] "m" (Xc5),[s5] "m" (Xs5)\
		 ,[c6] "m" (Xc6),[s6] "m" (Xs6)\
		 ,[c7] "m" (Xc7),[s7] "m" (Xs7)\
		: "cc","memory","rax","rbx","rcx","rdx","rsi","rdi","r8","r9","r10","r11","r12","r13","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"	/* Clobbered registers */\
	);\
	}

#endif	// AVX / SSE2 toggle

#endif	/* sse2_macro_gcc_h_included */
