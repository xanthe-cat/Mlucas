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
#ifndef radix44_ditN_cy_dif1_gcc_h_included
#define radix44_ditN_cy_dif1_gcc_h_included

	#define	SSE2_RADIX44_DIT_NOTWIDDLE(Xadd,Xp01,Xp02,Xp03,Xp04, Xt00r, Xcc, Xout)\
	{\
	__asm__ volatile (\
		"/*	Block 1: add0,1,3,2 = &a[j1+p00]+p0,1,2,3: SSE2_RADIX4_DIT_0TWIDDLE_STRIDE_C(eax,ebx,edx,ecx, 0x160, t00r) */\n\t"\
			"movq	%[__t00r],%%rsi		\n\t"\
			"movq	%[__add],%%rax		\n\t"\
			"movslq	%[__p01],%%rbx		\n\t"\
			"movslq	%[__p02],%%rcx		\n\t"\
			"movslq	%[__p03],%%rdx		\n\t"\
			"shlq	$3,%%rbx		/* Pointer offset for floating doubles */\n\t"\
			"shlq	$3,%%rcx			\n\t"\
			"shlq	$3,%%rdx			\n\t"\
			"addq	%%rax,%%rbx			\n\t"\
			"addq	%%rax,%%rcx			\n\t							movq	%[__cc],%%r8		\n\t"\
			"addq	%%rax,%%rdx			\n\t"\
			"/* ecx <-> edx */			\n\t"\
			"movaps	     (%%rax),%%xmm2	\n\t"\
			"movaps	     (%%rdx),%%xmm6	\n\t"\
			"movaps	0x010(%%rax),%%xmm3	\n\t							subq	$0x20,%%r8	/* two */\n\t"\
			"movaps	0x010(%%rdx),%%xmm7	\n\t"\
			"movaps	     (%%rbx),%%xmm0	\n\t"\
			"movaps	     (%%rcx),%%xmm4	\n\t"\
			"movaps	0x010(%%rbx),%%xmm1	\n\t"\
			"movaps	0x010(%%rcx),%%xmm5	\n\t						/*	Block 2: add2,3,0,1 = &a[j1+p04]+p0,1,2,3: SSE2_RADIX4_DIT_0TWIDDLE_STRIDE_C(ecx,edx,eax,ebx, 0x160, t08r) */\n\t"\
			"subpd	%%xmm0,%%xmm2		\n\t							movslq	%[__p04],%%rdi\n\t"\
			"subpd	%%xmm4,%%xmm6		\n\t							shlq	$3,%%rdi \n\t"\
			"subpd	%%xmm1,%%xmm3		\n\t							addq	%%rdi,%%rax	/* add04 */\n\t"\
			"subpd	%%xmm5,%%xmm7		\n\t							addq	%%rdi,%%rbx\n\t"\
			"addpd	%%xmm0,%%xmm0		\n\t							addq	%%rdi,%%rcx\n\t"\
			"addpd	%%xmm4,%%xmm4		\n\t							addq	%%rdi,%%rdx\n\t"\
			"addpd	%%xmm1,%%xmm1		\n\t							/* e[ab]x <-> e[cd]x */\n\t"\
			"addpd	%%xmm5,%%xmm5		\n\t							movaps	     (%%rcx),%%xmm10	\n\t"\
			"addpd	%%xmm2,%%xmm0		\n\t							movaps	     (%%rax),%%xmm14	\n\t"\
			"addpd	%%xmm6,%%xmm4		\n\t							movaps	0x010(%%rcx),%%xmm11	\n\t"\
			"addpd	%%xmm3,%%xmm1		\n\t							movaps	0x010(%%rax),%%xmm15	\n\t"\
			"addpd	%%xmm7,%%xmm5		\n\t							movaps	     (%%rdx),%%xmm8 	\n\t"\
			"subpd	%%xmm4,%%xmm0		\n\t							movaps	     (%%rbx),%%xmm12	\n\t"\
			"subpd	%%xmm7,%%xmm2		\n\t							movaps	0x010(%%rdx),%%xmm9 	\n\t"\
			"subpd	%%xmm5,%%xmm1		\n\t							movaps	0x010(%%rbx),%%xmm13	\n\t"\
			"subpd	%%xmm6,%%xmm3		\n\t							subpd	%%xmm8 ,%%xmm10		\n\t"\
			"movaps	%%xmm0,0x2c0(%%rsi)	\n\t							subpd	%%xmm12,%%xmm14		\n\t"\
			"movaps	%%xmm2,0x420(%%rsi)	\n\t							subpd	%%xmm9 ,%%xmm11		\n\t"\
			"movaps	%%xmm1,0x2d0(%%rsi)	\n\t							subpd	%%xmm13,%%xmm15		\n\t"\
			"movaps	%%xmm3,0x170(%%rsi)	\n\t							mulpd	(%%r8),%%xmm8 		\n\t"\
			"addpd	%%xmm4,%%xmm4		\n\t							mulpd	(%%r8),%%xmm12		\n\t"\
			"addpd	%%xmm7,%%xmm7		\n\t							mulpd	(%%r8),%%xmm9 		\n\t"\
			"addpd	%%xmm5,%%xmm5		\n\t							mulpd	(%%r8),%%xmm13		\n\t"\
			"addpd	%%xmm6,%%xmm6		\n\t							addpd	%%xmm10,%%xmm8 		\n\t"\
			"addpd	%%xmm0,%%xmm4		\n\t							addpd	%%xmm14,%%xmm12		\n\t"\
			"addpd	%%xmm2,%%xmm7		\n\t							addpd	%%xmm11,%%xmm9 		\n\t"\
			"addpd	%%xmm1,%%xmm5		\n\t							addpd	%%xmm15,%%xmm13		\n\t"\
			"addpd	%%xmm3,%%xmm6		\n\t							subpd	%%xmm12,%%xmm8 		\n\t"\
			"movaps	%%xmm4,     (%%rsi)	\n\t							subpd	%%xmm15,%%xmm10		\n\t"\
			"movaps	%%xmm7,0x160(%%rsi)	\n\t							subpd	%%xmm13,%%xmm9 		\n\t"\
			"movaps	%%xmm5,0x010(%%rsi)	\n\t							subpd	%%xmm14,%%xmm11		\n\t"\
			"movaps	%%xmm6,0x430(%%rsi)	\n\t"\
		"/*	Block 3: add1,0,2,3 = &a[j1+p08]+p0,1,2,3: */\n\t			addq	$0x100,%%rsi	/* t08r */\n\t"\
			"addq	%%rdi,%%rax	/* add08 */\n\t							movaps	%%xmm8 ,0x2c0(%%rsi)	\n\t"\
			"addq	%%rdi,%%rbx	\n\t									movaps	%%xmm10,0x420(%%rsi)	\n\t"\
			"addq	%%rdi,%%rcx	\n\t									movaps	%%xmm9 ,0x2d0(%%rsi)	\n\t"\
			"addq	%%rdi,%%rdx	\n\t									movaps	%%xmm11,0x170(%%rsi)	\n\t"\
			"/* eax <-> ebx */	\n\t									addpd	%%xmm12,%%xmm12		\n\t"\
			"movaps	     (%%rbx),%%xmm2	\n\t							addpd	%%xmm15,%%xmm15		\n\t"\
			"movaps	     (%%rcx),%%xmm6	\n\t							addpd	%%xmm13,%%xmm13		\n\t"\
			"movaps	0x010(%%rbx),%%xmm3	\n\t							addpd	%%xmm14,%%xmm14		\n\t"\
			"movaps	0x010(%%rcx),%%xmm7	\n\t							addpd	%%xmm8 ,%%xmm12		\n\t"\
			"movaps	     (%%rax),%%xmm0	\n\t							addpd	%%xmm10,%%xmm15		\n\t"\
			"movaps	     (%%rdx),%%xmm4	\n\t							addpd	%%xmm9 ,%%xmm13		\n\t"\
			"movaps	0x010(%%rax),%%xmm1	\n\t							addpd	%%xmm11,%%xmm14		\n\t"\
			"movaps	0x010(%%rdx),%%xmm5	\n\t							movaps	%%xmm12,     (%%rsi)	\n\t"\
			"subpd	%%xmm0,%%xmm2		\n\t							movaps	%%xmm15,0x160(%%rsi)	\n\t"\
			"subpd	%%xmm4,%%xmm6		\n\t							movaps	%%xmm13,0x010(%%rsi)	\n\t"\
			"subpd	%%xmm1,%%xmm3		\n\t							movaps	%%xmm14,0x430(%%rsi)	\n\t"\
			"subpd	%%xmm5,%%xmm7		\n\t						/*	Block 4: add3,2,1,0 = &a[j1+p12]+p0,1,2,3: SSE2_RADIX4_DIT_0TWIDDLE_STRIDE_C(edx,ecx,ebx,eax, 0x160, t02r) */\n\t"\
			"subq	$0x060,%%rsi	/* t05r */\n\t						addq	%%rdi,%%rax	/* add12 */\n\t"\
			"addpd	%%xmm0,%%xmm0		\n\t							addq	%%rdi,%%rbx\n\t"\
			"addpd	%%xmm4,%%xmm4		\n\t							addq	%%rdi,%%rcx\n\t"\
			"addpd	%%xmm1,%%xmm1		\n\t							addq	%%rdi,%%rdx\n\t"\
			"addpd	%%xmm5,%%xmm5		\n\t							/* eax <-> edx, ebx <-> ecx */\n\t"\
			"addpd	%%xmm2,%%xmm0		\n\t							movaps	     (%%rdx),%%xmm10	\n\t"\
			"addpd	%%xmm6,%%xmm4		\n\t							movaps	     (%%rbx),%%xmm14	\n\t"\
			"addpd	%%xmm3,%%xmm1		\n\t							movaps	0x010(%%rdx),%%xmm11	\n\t"\
			"addpd	%%xmm7,%%xmm5		\n\t							movaps	0x010(%%rbx),%%xmm15	\n\t"\
			"subpd	%%xmm4,%%xmm0		\n\t							movaps	     (%%rcx),%%xmm8 	\n\t"\
			"subpd	%%xmm7,%%xmm2		\n\t							movaps	     (%%rax),%%xmm12	\n\t"\
			"subpd	%%xmm5,%%xmm1		\n\t							movaps	0x010(%%rcx),%%xmm9 	\n\t"\
			"subpd	%%xmm6,%%xmm3		\n\t							movaps	0x010(%%rax),%%xmm13	\n\t"\
			"movaps	%%xmm0,0x2c0(%%rsi)	\n\t							subpd	%%xmm8 ,%%xmm10		\n\t"\
			"movaps	%%xmm2,0x420(%%rsi)	\n\t							subpd	%%xmm12,%%xmm14		\n\t"\
			"movaps	%%xmm1,0x2d0(%%rsi)	\n\t							subpd	%%xmm9 ,%%xmm11		\n\t"\
			"movaps	%%xmm3,0x170(%%rsi)	\n\t							subpd	%%xmm13,%%xmm15		\n\t"\
			"addpd	%%xmm4,%%xmm4		\n\t							mulpd	(%%r8),%%xmm8 		\n\t"\
			"addpd	%%xmm7,%%xmm7		\n\t							mulpd	(%%r8),%%xmm12		\n\t"\
			"addpd	%%xmm5,%%xmm5		\n\t							mulpd	(%%r8),%%xmm9 		\n\t"\
			"addpd	%%xmm6,%%xmm6		\n\t							mulpd	(%%r8),%%xmm13		\n\t"\
			"addpd	%%xmm0,%%xmm4		\n\t							addpd	%%xmm10,%%xmm8 		\n\t"\
			"addpd	%%xmm2,%%xmm7		\n\t							addpd	%%xmm14,%%xmm12		\n\t"\
			"addpd	%%xmm1,%%xmm5		\n\t							addpd	%%xmm11,%%xmm9 		\n\t"\
			"addpd	%%xmm3,%%xmm6		\n\t							addpd	%%xmm15,%%xmm13		\n\t"\
			"movaps	%%xmm4,     (%%rsi)	\n\t							subpd	%%xmm12,%%xmm8 		\n\t"\
			"movaps	%%xmm7,0x160(%%rsi)	\n\t							subpd	%%xmm15,%%xmm10		\n\t"\
			"movaps	%%xmm5,0x010(%%rsi)	\n\t							subpd	%%xmm13,%%xmm9 		\n\t"\
			"movaps	%%xmm6,0x430(%%rsi)	\n\t							subpd	%%xmm14,%%xmm11		\n\t"\
		"/*	Block 5: add0,1,3,2 = &a[j1+p16]+p0,1,2,3: */\n\t			subq	$0x060,%%rsi	/* t02r */\n\t"\
			"addq	%%rdi,%%rax	/* add16 */\n\t							movaps	%%xmm8 ,0x2c0(%%rsi)	\n\t"\
			"addq	%%rdi,%%rbx\n\t										movaps	%%xmm10,0x420(%%rsi)	\n\t"\
			"addq	%%rdi,%%rcx\n\t										movaps	%%xmm9 ,0x2d0(%%rsi)	\n\t"\
			"addq	%%rdi,%%rdx\n\t										movaps	%%xmm11,0x170(%%rsi)	\n\t"\
			"/* ecx <-> edx */\n\t										addpd	%%xmm12,%%xmm12		\n\t"\
			"movaps	     (%%rax),%%xmm2	\n\t							addpd	%%xmm15,%%xmm15		\n\t"\
			"movaps	     (%%rdx),%%xmm6	\n\t							addpd	%%xmm13,%%xmm13		\n\t"\
			"movaps	0x010(%%rax),%%xmm3	\n\t							addpd	%%xmm14,%%xmm14		\n\t"\
			"movaps	0x010(%%rdx),%%xmm7	\n\t							addpd	%%xmm8 ,%%xmm12		\n\t"\
			"movaps	     (%%rbx),%%xmm0	\n\t							addpd	%%xmm10,%%xmm15		\n\t"\
			"movaps	     (%%rcx),%%xmm4	\n\t							addpd	%%xmm9 ,%%xmm13		\n\t"\
			"movaps	0x010(%%rbx),%%xmm1	\n\t							addpd	%%xmm11,%%xmm14		\n\t"\
			"movaps	0x010(%%rcx),%%xmm5	\n\t							movaps	%%xmm12,     (%%rsi)	\n\t"\
			"subpd	%%xmm0,%%xmm2		\n\t							movaps	%%xmm15,0x160(%%rsi)	\n\t"\
			"subpd	%%xmm4,%%xmm6		\n\t							movaps	%%xmm13,0x010(%%rsi)	\n\t"\
			"subpd	%%xmm1,%%xmm3		\n\t							movaps	%%xmm14,0x430(%%rsi)	\n\t"\
			"subpd	%%xmm5,%%xmm7		\n\t						/*	Block 6: add2,3,0,1 = &a[j1+p20]+p0,1,2,3: SSE2_RADIX4_DIT_0TWIDDLE_STRIDE_C(ecx,edx,eax,ebx, 0x160, t07r) */\n\t"\
			"addq	$0x100,%%rsi	/* t0ar */\n\t						addq	%%rdi,%%rax	/* add20 */\n\t"\
			"addpd	%%xmm0,%%xmm0		\n\t							addq	%%rdi,%%rbx\n\t"\
			"addpd	%%xmm4,%%xmm4		\n\t							addq	%%rdi,%%rcx\n\t"\
			"addpd	%%xmm1,%%xmm1		\n\t							addq	%%rdi,%%rdx\n\t"\
			"addpd	%%xmm5,%%xmm5		\n\t							/* e[ab]x <-> e[cd]x */\n\t"\
			"addpd	%%xmm2,%%xmm0		\n\t							movaps	     (%%rcx),%%xmm10	\n\t"\
			"addpd	%%xmm6,%%xmm4		\n\t							movaps	     (%%rax),%%xmm14	\n\t"\
			"addpd	%%xmm3,%%xmm1		\n\t							movaps	0x010(%%rcx),%%xmm11	\n\t"\
			"addpd	%%xmm7,%%xmm5		\n\t							movaps	0x010(%%rax),%%xmm15	\n\t"\
			"subpd	%%xmm4,%%xmm0		\n\t							movaps	     (%%rdx),%%xmm8 	\n\t"\
			"subpd	%%xmm7,%%xmm2		\n\t							movaps	     (%%rbx),%%xmm12	\n\t"\
			"subpd	%%xmm5,%%xmm1		\n\t							movaps	0x010(%%rdx),%%xmm9 	\n\t"\
			"subpd	%%xmm6,%%xmm3		\n\t							movaps	0x010(%%rbx),%%xmm13	\n\t"\
			"movaps	%%xmm0,0x2c0(%%rsi)	\n\t							subpd	%%xmm8 ,%%xmm10		\n\t"\
			"movaps	%%xmm2,0x420(%%rsi)	\n\t							subpd	%%xmm12,%%xmm14		\n\t"\
			"movaps	%%xmm1,0x2d0(%%rsi)	\n\t							subpd	%%xmm9 ,%%xmm11		\n\t"\
			"movaps	%%xmm3,0x170(%%rsi)	\n\t							subpd	%%xmm13,%%xmm15		\n\t"\
			"addpd	%%xmm4,%%xmm4		\n\t							mulpd	(%%r8),%%xmm8 		\n\t"\
			"addpd	%%xmm7,%%xmm7		\n\t							mulpd	(%%r8),%%xmm12		\n\t"\
			"addpd	%%xmm5,%%xmm5		\n\t							mulpd	(%%r8),%%xmm9 		\n\t"\
			"addpd	%%xmm6,%%xmm6		\n\t							mulpd	(%%r8),%%xmm13		\n\t"\
			"addpd	%%xmm0,%%xmm4		\n\t							addpd	%%xmm10,%%xmm8 		\n\t"\
			"addpd	%%xmm2,%%xmm7		\n\t							addpd	%%xmm14,%%xmm12		\n\t"\
			"addpd	%%xmm1,%%xmm5		\n\t							addpd	%%xmm11,%%xmm9 		\n\t"\
			"addpd	%%xmm3,%%xmm6		\n\t							addpd	%%xmm15,%%xmm13		\n\t"\
			"movaps	%%xmm4,     (%%rsi)	\n\t							subpd	%%xmm12,%%xmm8 		\n\t"\
			"movaps	%%xmm7,0x160(%%rsi)	\n\t							subpd	%%xmm15,%%xmm10		\n\t"\
			"movaps	%%xmm5,0x010(%%rsi)	\n\t							subpd	%%xmm13,%%xmm9 		\n\t"\
			"movaps	%%xmm6,0x430(%%rsi)	\n\t							subpd	%%xmm14,%%xmm11		\n\t"\
		"/*	Block 7: add1,0,2,3 = &a[j1+p24]+p0,1,2,3: */\n\t			subq	$0x060,%%rsi	/* t07r */\n\t"\
			"addq	%%rdi,%%rax	/* add24 */\n\t							movaps	%%xmm8 ,0x2c0(%%rsi)	\n\t"\
			"addq	%%rdi,%%rbx\n\t										movaps	%%xmm10,0x420(%%rsi)	\n\t"\
			"addq	%%rdi,%%rcx\n\t										movaps	%%xmm9 ,0x2d0(%%rsi)	\n\t"\
			"addq	%%rdi,%%rdx\n\t										movaps	%%xmm11,0x170(%%rsi)	\n\t"\
			"/* eax <-> ebx */\n\t										addpd	%%xmm12,%%xmm12		\n\t"\
			"movaps	     (%%rbx),%%xmm2	\n\t							addpd	%%xmm15,%%xmm15		\n\t"\
			"movaps	     (%%rcx),%%xmm6	\n\t							addpd	%%xmm13,%%xmm13		\n\t"\
			"movaps	0x010(%%rbx),%%xmm3	\n\t							addpd	%%xmm14,%%xmm14		\n\t"\
			"movaps	0x010(%%rcx),%%xmm7	\n\t							addpd	%%xmm8 ,%%xmm12		\n\t"\
			"movaps	     (%%rax),%%xmm0	\n\t							addpd	%%xmm10,%%xmm15		\n\t"\
			"movaps	     (%%rdx),%%xmm4	\n\t							addpd	%%xmm9 ,%%xmm13		\n\t"\
			"movaps	0x010(%%rax),%%xmm1	\n\t							addpd	%%xmm11,%%xmm14		\n\t"\
			"movaps	0x010(%%rdx),%%xmm5	\n\t							movaps	%%xmm12,     (%%rsi)	\n\t"\
			"subpd	%%xmm0,%%xmm2		\n\t							movaps	%%xmm15,0x160(%%rsi)	\n\t"\
			"subpd	%%xmm4,%%xmm6		\n\t							movaps	%%xmm13,0x010(%%rsi)	\n\t"\
			"subpd	%%xmm1,%%xmm3		\n\t							movaps	%%xmm14,0x430(%%rsi)	\n\t"\
			"subpd	%%xmm5,%%xmm7		\n\t						/*	Block 8: add3,2,1,0 = &a[j1+p28]+p0,1,2,3: SSE2_RADIX4_DIT_0TWIDDLE_STRIDE_C(edx,ecx,ebx,eax, 0x160, t01r) */\n\t"\
			"subq	$0x060,%%rsi	/* t04r */\n\t						addq	%%rdi,%%rax	/* add28 */\n\t"\
			"addpd	%%xmm0,%%xmm0		\n\t							addq	%%rdi,%%rbx\n\t"\
			"addpd	%%xmm4,%%xmm4		\n\t							addq	%%rdi,%%rcx\n\t"\
			"addpd	%%xmm1,%%xmm1		\n\t							addq	%%rdi,%%rdx\n\t"\
			"addpd	%%xmm5,%%xmm5		\n\t							/* eax <-> edx, ebx <-> ecx */\n\t"\
			"addpd	%%xmm2,%%xmm0		\n\t							movaps	     (%%rdx),%%xmm10	\n\t"\
			"addpd	%%xmm6,%%xmm4		\n\t							movaps	     (%%rbx),%%xmm14	\n\t"\
			"addpd	%%xmm3,%%xmm1		\n\t							movaps	0x010(%%rdx),%%xmm11	\n\t"\
			"addpd	%%xmm7,%%xmm5		\n\t							movaps	0x010(%%rbx),%%xmm15	\n\t"\
			"subpd	%%xmm4,%%xmm0		\n\t							movaps	     (%%rcx),%%xmm8 	\n\t"\
			"subpd	%%xmm7,%%xmm2		\n\t							movaps	     (%%rax),%%xmm12	\n\t"\
			"subpd	%%xmm5,%%xmm1		\n\t							movaps	0x010(%%rcx),%%xmm9 	\n\t"\
			"subpd	%%xmm6,%%xmm3		\n\t							movaps	0x010(%%rax),%%xmm13	\n\t"\
			"movaps	%%xmm0,0x2c0(%%rsi)	\n\t							subpd	%%xmm8 ,%%xmm10		\n\t"\
			"movaps	%%xmm2,0x420(%%rsi)	\n\t							subpd	%%xmm12,%%xmm14		\n\t"\
			"movaps	%%xmm1,0x2d0(%%rsi)	\n\t							subpd	%%xmm9 ,%%xmm11		\n\t"\
			"movaps	%%xmm3,0x170(%%rsi)	\n\t							subpd	%%xmm13,%%xmm15		\n\t"\
			"addpd	%%xmm4,%%xmm4		\n\t							mulpd	(%%r8),%%xmm8 		\n\t"\
			"addpd	%%xmm7,%%xmm7		\n\t							mulpd	(%%r8),%%xmm12		\n\t"\
			"addpd	%%xmm5,%%xmm5		\n\t							mulpd	(%%r8),%%xmm9 		\n\t"\
			"addpd	%%xmm6,%%xmm6		\n\t							mulpd	(%%r8),%%xmm13		\n\t"\
			"addpd	%%xmm0,%%xmm4		\n\t							addpd	%%xmm10,%%xmm8 		\n\t"\
			"addpd	%%xmm2,%%xmm7		\n\t							addpd	%%xmm14,%%xmm12		\n\t"\
			"addpd	%%xmm1,%%xmm5		\n\t							addpd	%%xmm11,%%xmm9 		\n\t"\
			"addpd	%%xmm3,%%xmm6		\n\t							addpd	%%xmm15,%%xmm13		\n\t"\
			"movaps	%%xmm4,     (%%rsi)	\n\t							subpd	%%xmm12,%%xmm8 		\n\t"\
			"movaps	%%xmm7,0x160(%%rsi)	\n\t							subpd	%%xmm15,%%xmm10		\n\t"\
			"movaps	%%xmm5,0x010(%%rsi)	\n\t							subpd	%%xmm13,%%xmm9 		\n\t"\
			"movaps	%%xmm6,0x430(%%rsi)	\n\t							subpd	%%xmm14,%%xmm11		\n\t"\
		"/*	Block 9: add0,1,3,2 = &a[j1+p32]+p0,1,2,3: */\n\t			subq	$0x060,%%rsi	/* t01r */\n\t"\
			"addq	%%rdi,%%rax	/* add32 */\n\t							movaps	%%xmm8 ,0x2c0(%%rsi)	\n\t"\
			"addq	%%rdi,%%rbx\n\t										movaps	%%xmm10,0x420(%%rsi)	\n\t"\
			"addq	%%rdi,%%rcx\n\t										movaps	%%xmm9 ,0x2d0(%%rsi)	\n\t"\
			"addq	%%rdi,%%rdx\n\t										movaps	%%xmm11,0x170(%%rsi)	\n\t"\
			"/* ecx <-> edx */\n\t										addpd	%%xmm12,%%xmm12		\n\t"\
			"movaps	     (%%rax),%%xmm2	\n\t							addpd	%%xmm15,%%xmm15		\n\t"\
			"movaps	     (%%rdx),%%xmm6	\n\t							addpd	%%xmm13,%%xmm13		\n\t"\
			"movaps	0x010(%%rax),%%xmm3	\n\t							addpd	%%xmm14,%%xmm14		\n\t"\
			"movaps	0x010(%%rdx),%%xmm7	\n\t							addpd	%%xmm8 ,%%xmm12		\n\t"\
			"movaps	     (%%rbx),%%xmm0	\n\t							addpd	%%xmm10,%%xmm15		\n\t"\
			"movaps	     (%%rcx),%%xmm4	\n\t							addpd	%%xmm9 ,%%xmm13		\n\t"\
			"movaps	0x010(%%rbx),%%xmm1	\n\t							addpd	%%xmm11,%%xmm14		\n\t"\
			"movaps	0x010(%%rcx),%%xmm5	\n\t							movaps	%%xmm12,     (%%rsi)	\n\t"\
			"subpd	%%xmm0,%%xmm2		\n\t							movaps	%%xmm15,0x160(%%rsi)	\n\t"\
			"subpd	%%xmm4,%%xmm6		\n\t							movaps	%%xmm13,0x010(%%rsi)	\n\t"\
			"subpd	%%xmm1,%%xmm3		\n\t							movaps	%%xmm14,0x430(%%rsi)	\n\t"\
			"subpd	%%xmm5,%%xmm7		\n\t						/*	Block 10: add2,3,0,1 = &a[j1+p36]+p0,1,2,3: SSE2_RADIX4_DIT_0TWIDDLE_STRIDE_C(ecx,edx,eax,ebx, 0x160, t06r) */\n\t"\
			"addq	$0x100,%%rsi	/* t09r */\n\t						addq	%%rdi,%%rax	/* add36 */\n\t"\
			"addpd	%%xmm0,%%xmm0		\n\t							addq	%%rdi,%%rbx\n\t"\
			"addpd	%%xmm4,%%xmm4		\n\t							addq	%%rdi,%%rcx\n\t"\
			"addpd	%%xmm1,%%xmm1		\n\t							addq	%%rdi,%%rdx\n\t"\
			"addpd	%%xmm5,%%xmm5		\n\t							/* e[ab]x <-> e[cd]x */\n\t"\
			"addpd	%%xmm2,%%xmm0		\n\t							movaps	     (%%rcx),%%xmm10	\n\t"\
			"addpd	%%xmm6,%%xmm4		\n\t							movaps	     (%%rax),%%xmm14	\n\t"\
			"addpd	%%xmm3,%%xmm1		\n\t							movaps	0x010(%%rcx),%%xmm11	\n\t"\
			"addpd	%%xmm7,%%xmm5		\n\t							movaps	0x010(%%rax),%%xmm15	\n\t"\
			"subpd	%%xmm4,%%xmm0		\n\t							movaps	     (%%rdx),%%xmm8 	\n\t"\
			"subpd	%%xmm7,%%xmm2		\n\t							movaps	     (%%rbx),%%xmm12	\n\t"\
			"subpd	%%xmm5,%%xmm1		\n\t							movaps	0x010(%%rdx),%%xmm9 	\n\t"\
			"subpd	%%xmm6,%%xmm3		\n\t							movaps	0x010(%%rbx),%%xmm13	\n\t"\
			"movaps	%%xmm0,0x2c0(%%rsi)	\n\t							subpd	%%xmm8 ,%%xmm10		\n\t"\
			"movaps	%%xmm2,0x420(%%rsi)	\n\t							subpd	%%xmm12,%%xmm14		\n\t"\
			"movaps	%%xmm1,0x2d0(%%rsi)	\n\t							subpd	%%xmm9 ,%%xmm11		\n\t"\
			"movaps	%%xmm3,0x170(%%rsi)	\n\t							subpd	%%xmm13,%%xmm15		\n\t"\
			"addpd	%%xmm4,%%xmm4		\n\t							mulpd	(%%r8),%%xmm8 		\n\t"\
			"addpd	%%xmm7,%%xmm7		\n\t							mulpd	(%%r8),%%xmm12		\n\t"\
			"addpd	%%xmm5,%%xmm5		\n\t							mulpd	(%%r8),%%xmm9 		\n\t"\
			"addpd	%%xmm6,%%xmm6		\n\t							mulpd	(%%r8),%%xmm13		\n\t"\
			"addpd	%%xmm0,%%xmm4		\n\t							addpd	%%xmm10,%%xmm8 		\n\t"\
			"addpd	%%xmm2,%%xmm7		\n\t							addpd	%%xmm14,%%xmm12		\n\t"\
			"addpd	%%xmm1,%%xmm5		\n\t							addpd	%%xmm11,%%xmm9 		\n\t"\
			"addpd	%%xmm3,%%xmm6		\n\t							addpd	%%xmm15,%%xmm13		\n\t"\
			"movaps	%%xmm4,     (%%rsi)	\n\t							subpd	%%xmm12,%%xmm8 		\n\t"\
			"movaps	%%xmm7,0x160(%%rsi)	\n\t							subpd	%%xmm15,%%xmm10		\n\t"\
			"movaps	%%xmm5,0x010(%%rsi)	\n\t							subpd	%%xmm13,%%xmm9 		\n\t"\
			"movaps	%%xmm6,0x430(%%rsi)	\n\t							subpd	%%xmm14,%%xmm11		\n\t"\
		"/*	Block 11: add1,0,2,3 = &a[j1+p40]+p0,1,2,3: */\n\t			subq	$0x060,%%rsi	/* t06r */\n\t"\
			"addq	%%rdi,%%rax	/* add36 */\n\t							movaps	%%xmm8 ,0x2c0(%%rsi)	\n\t"\
			"addq	%%rdi,%%rbx\n\t										movaps	%%xmm10,0x420(%%rsi)	\n\t"\
			"addq	%%rdi,%%rcx\n\t										movaps	%%xmm9 ,0x2d0(%%rsi)	\n\t"\
			"addq	%%rdi,%%rdx\n\t										movaps	%%xmm11,0x170(%%rsi)	\n\t"\
			"/* eax <-> ebx */\n\t										addpd	%%xmm12,%%xmm12		\n\t"\
			"movaps	     (%%rbx),%%xmm2	\n\t							addpd	%%xmm15,%%xmm15		\n\t"\
			"movaps	     (%%rcx),%%xmm6	\n\t							addpd	%%xmm13,%%xmm13		\n\t"\
			"movaps	0x010(%%rbx),%%xmm3	\n\t							addpd	%%xmm14,%%xmm14		\n\t"\
			"movaps	0x010(%%rcx),%%xmm7	\n\t							addpd	%%xmm8 ,%%xmm12		\n\t"\
			"movaps	     (%%rax),%%xmm0	\n\t							addpd	%%xmm10,%%xmm15		\n\t"\
			"movaps	     (%%rdx),%%xmm4	\n\t							addpd	%%xmm9 ,%%xmm13		\n\t"\
			"movaps	0x010(%%rax),%%xmm1	\n\t							addpd	%%xmm11,%%xmm14		\n\t"\
			"movaps	0x010(%%rdx),%%xmm5	\n\t							movaps	%%xmm12,     (%%rsi)	\n\t"\
			"subpd	%%xmm0,%%xmm2		\n\t							movaps	%%xmm15,0x160(%%rsi)	\n\t"\
			"subpd	%%xmm4,%%xmm6		\n\t							movaps	%%xmm13,0x010(%%rsi)	\n\t"\
			"subpd	%%xmm1,%%xmm3		\n\t							movaps	%%xmm14,0x430(%%rsi)	\n\t"\
			"subpd	%%xmm5,%%xmm7		\n\t							\n\t"\
			"subq	$0x060,%%rsi	/* t03r */\n\t						\n\t"\
			"addpd	%%xmm0,%%xmm0		\n\t							/* RADIX_11(t0[0-A], s1p[00,12,24,36,04,16,28,40,08,20,32]):*/\n\t"\
			"addpd	%%xmm4,%%xmm4		\n\t							/********************************************/\n\t"\
			"addpd	%%xmm1,%%xmm1		\n\t							/*       Here are the 5 cosine terms:       */\n\t"\
			"addpd	%%xmm5,%%xmm5		\n\t							/********************************************/\n\t"\
			"addpd	%%xmm2,%%xmm0		\n\t							movq	%[__t00r],%%rdx			\n\t"\
			"addpd	%%xmm6,%%xmm4		\n\t							movq	%[__out],%%rdi	/* s1p00r */\n\t"\
			"addpd	%%xmm3,%%xmm1		\n\t							movq	%%rdx,%%rax	/* cpy t00r */	\n\t"\
			"addpd	%%xmm7,%%xmm5		\n\t							movq	%%rdi,%%rcx	/* cpy s1p00r */\n\t"\
			"subpd	%%xmm4,%%xmm0		\n\t							movaps	0x020(%%rdx),%%xmm9 	\n\t"\
			"subpd	%%xmm7,%%xmm2		\n\t							movaps	0x140(%%rdx),%%xmm13	\n\t"\
			"subpd	%%xmm5,%%xmm1		\n\t							movaps	0x040(%%rdx),%%xmm10	\n\t"\
			"subpd	%%xmm6,%%xmm3		\n\t							movaps	0x120(%%rdx),%%xmm14	\n\t"\
			"movaps	%%xmm0,0x2c0(%%rsi)	/* t23r */\n\t				/*	movaps	0x060(%%rdx),%%xmm11	// t03r */\n\t"\
			"movaps	%%xmm2,0x420(%%rsi)	/* t33r */\n\t					movaps	0x100(%%rdx),%%xmm15	\n\t"\
			"movaps	%%xmm1,0x2d0(%%rsi)	/* t23i */\n\t					movaps	0x080(%%rdx),%%xmm12	\n\t"\
			"movaps	%%xmm3,0x170(%%rsi)	/* t13i */\n\t					movaps	0x0e0(%%rdx),%%xmm8 	\n\t"\
			"mulpd	(%%r8),%%xmm4		\n\t							addq	$0x10,%%rdi				\n\t"\
			"mulpd	(%%r8),%%xmm7		\n\t							subpd	%%xmm13,%%xmm9 			\n\t"\
			"addpd	%%xmm0,%%xmm4		/* t03r */\n\t					subpd	%%xmm14,%%xmm10			\n\t	movaps	%%xmm4,%%xmm11	/* t03r */\n\t"\
			"addpd	%%xmm2,%%xmm7		\n\t							subpd	%%xmm15,%%xmm11			\n\t"\
			"mulpd	(%%r8),%%xmm5		\n\t							subpd	%%xmm8 ,%%xmm12			\n\t"\
			"mulpd	(%%r8),%%xmm6		\n\t							movaps	%%xmm9 ,0x400(%%rdi)	\n\t"\
			"addpd	%%xmm1,%%xmm5		\n\t							movaps	%%xmm10,0x280(%%rdi)	\n\t"\
			"addpd	%%xmm3,%%xmm6		\n\t							movaps	%%xmm11,0x100(%%rdi)	\n\t"\
		"/*	movaps	%%xmm4,     (%%rsi)	// t03r */\n\t					movaps	%%xmm12,0x500(%%rdi)	\n\t"\
			"movaps	%%xmm7,0x160(%%rsi)	/* t13r */\n\t					addpd	%%xmm13,%%xmm13			\n\t"\
			"movaps	%%xmm5,0x010(%%rsi)	/* t03i */\n\t					addpd	%%xmm14,%%xmm14			\n\t"\
			"movaps	%%xmm6,0x430(%%rsi)	/* t33i */\n\t					addpd	%%xmm15,%%xmm15			\n\t"\
			"\n\t														addpd	%%xmm8 ,%%xmm8 			\n\t"\
			"/* RADIX_11(t1[0-A],s[11,23,35,03,15,27,39,07,19,31,43]):*/addpd	%%xmm13,%%xmm9 			\n\t"\
			"/********************************************/\n\t			addpd	%%xmm14,%%xmm10			\n\t"\
			"/*       Here are the 5 cosine terms:       */\n\t			movaps	0x0a0(%%rdx),%%xmm13	\n\t"\
			"/********************************************/\n\t			movaps	0x0c0(%%rdx),%%xmm14	\n\t"\
			"addq	$0x160,%%rax	/* t10r */	\n\t					addpd	%%xmm15,%%xmm11			\n\t"\
			"movq	%[__cc],%%rbx	/* cc */	\n\t					addpd	%%xmm8 ,%%xmm12			\n\t"\
			"addq	$0x160,%%rcx	/* s1p11r */\n\t					subpd	%%xmm14,%%xmm13			\n\t"\
			"movaps	0x020(%%rax),%%xmm1	\n\t							movaps	%%xmm13,0x380(%%rdi)	\n\t"\
			"movaps	0x140(%%rax),%%xmm5	\n\t							addpd	%%xmm14,%%xmm14			\n\t"\
			"movaps	0x040(%%rax),%%xmm2	\n\t							movaps	(%%rdx),%%xmm8 			\n\t"\
			"movaps	0x120(%%rax),%%xmm6	\n\t							addpd	%%xmm14,%%xmm13			\n\t"\
			"movaps	0x060(%%rax),%%xmm3	\n\t							movq	%[__cc],%%rsi			\n\t"\
			"movaps	0x100(%%rax),%%xmm7	\n\t							/********************************************/\n\t"\
			"movaps	0x080(%%rax),%%xmm4	\n\t							/*               Real Parts:                */\n\t"\
			"movaps	0x0e0(%%rax),%%xmm0	\n\t							/********************************************/\n\t"\
			"addq	$0x10,%%rcx				\n\t						subq	$0x10,%%rdi				\n\t"\
			"subpd	%%xmm5,%%xmm1			\n\t						subpd	%%xmm10,%%xmm9 			\n\t"\
			"subpd	%%xmm6,%%xmm2			\n\t						subpd	%%xmm10,%%xmm13			\n\t"\
			"subpd	%%xmm7,%%xmm3			\n\t						movaps	%%xmm9 ,%%xmm14			\n\t"\
			"subpd	%%xmm0,%%xmm4			\n\t						subpd	%%xmm10,%%xmm11			\n\t"\
			"movaps	%%xmm1,0x400(%%rcx)	\n\t							movaps	%%xmm9 ,%%xmm15			\n\t"\
			"movaps	%%xmm2,0x280(%%rcx)	\n\t							mulpd	     (%%rsi),%%xmm9 		\n\t"\
			"movaps	%%xmm3,0x100(%%rcx)	\n\t							movaps	%%xmm9 ,0x180(%%rdi)	\n\t"\
			"movaps	%%xmm4,-0x080(%%rcx)	\n\t						subpd	%%xmm10,%%xmm12			\n\t"\
			"addpd	%%xmm5,%%xmm5			\n\t						mulpd	-0x10(%%rsi),%%xmm10		\n\t"\
			"mulpd	(%%r8),%%xmm6			\n\t						addpd	%%xmm13,%%xmm14			\n\t"\
			"mulpd	(%%r8),%%xmm7			\n\t						subpd	%%xmm11,%%xmm15			\n\t"\
			"addpd	%%xmm0,%%xmm0			\n\t						mulpd	 0x60(%%rsi),%%xmm15		\n\t"\
			"addpd	%%xmm5,%%xmm1			\n\t						movaps	%%xmm15,0x480(%%rdi)	\n\t"\
			"addpd	%%xmm6,%%xmm2			\n\t						movaps	%%xmm11,%%xmm9 			\n\t"\
			"movaps	0x0a0(%%rax),%%xmm5	\n\t							addpd	%%xmm12,%%xmm11			\n\t"\
			"movaps	0x0c0(%%rax),%%xmm6	\n\t							mulpd	 0x30(%%rsi),%%xmm9 		\n\t"\
			"addpd	%%xmm7,%%xmm3			\n\t						movaps	%%xmm9 ,0x300(%%rdi)	\n\t"\
			"addpd	%%xmm0,%%xmm4			\n\t						movaps	%%xmm13,%%xmm15			\n\t"\
			"subpd	%%xmm6,%%xmm5			\n\t						subpd	%%xmm12,%%xmm13			\n\t"\
			"movaps	%%xmm5,0x380(%%rcx)	\n\t							mulpd	 0x40(%%rsi),%%xmm12		\n\t"\
			"addpd	%%xmm6,%%xmm6			\n\t						mulpd	 0x70(%%rsi),%%xmm13		\n\t"\
			"movaps	(%%rax),%%xmm0			\n\t						mulpd	 0x10(%%rsi),%%xmm15		\n\t"\
			"addpd	%%xmm6,%%xmm5			\n\t						addpd	%%xmm11,%%xmm10			\n\t"\
			"/********************************************/\n\t			movaps	%%xmm14,%%xmm9 			\n\t"\
			"/*               Real Parts:                */\n\t			subpd	%%xmm11,%%xmm14			\n\t"\
			"/********************************************/\n\t			mulpd	 0x80(%%rsi),%%xmm14		\n\t"\
			"subq	$0x10,%%rcx				\n\t						mulpd	 0x50(%%rsi),%%xmm11		\n\t"\
			"subpd	%%xmm2,%%xmm1			\n\t						addpd	%%xmm9 ,%%xmm10			\n\t"\
			"subpd	%%xmm2,%%xmm5			\n\t						mulpd	 0x20(%%rsi),%%xmm9 		\n\t"\
			"movaps	%%xmm1,%%xmm6			\n\t						addpd	%%xmm10,%%xmm8 			\n\t"\
			"subpd	%%xmm2,%%xmm3			\n\t						mulpd	 0x90(%%rsi),%%xmm10		\n\t"\
			"movaps	%%xmm1,%%xmm7			\n\t						movaps	%%xmm8 ,(%%rdi)			\n\t"\
			"mulpd	     (%%rbx),%%xmm1		\n\t						addpd	%%xmm8 ,%%xmm10			\n\t"\
			"movaps	%%xmm1,0x180(%%rcx)	\n\t							addpd	%%xmm9 ,%%xmm15			\n\t"\
			"subpd	%%xmm2,%%xmm4			\n\t						addpd	0x180(%%rdi),%%xmm9 	\n\t"\
			"mulpd	-0x10(%%rbx),%%xmm2		\n\t						addpd	%%xmm11,%%xmm12			\n\t"\
			"addpd	%%xmm5,%%xmm6			\n\t						addpd	0x300(%%rdi),%%xmm11	\n\t"\
			"subpd	%%xmm3,%%xmm7			\n\t						addpd	%%xmm14,%%xmm13			\n\t"\
			"mulpd	 0x60(%%rbx),%%xmm7		\n\t						addpd	0x480(%%rdi),%%xmm14	\n\t"\
			"movaps	%%xmm7,-0x100(%%rcx)	\n\t						movaps	%%xmm10,%%xmm8 			\n\t"\
			"movaps	%%xmm3,%%xmm1			\n\t						subpd	%%xmm9 ,%%xmm10			\n\t"\
			"addpd	%%xmm4,%%xmm3			\n\t						addpd	%%xmm8 ,%%xmm9 			\n\t"\
			"mulpd	 0x30(%%rbx),%%xmm1		\n\t						subpd	%%xmm15,%%xmm10			\n\t"\
			"movaps	%%xmm1,0x300(%%rcx)	\n\t							addpd	%%xmm8 ,%%xmm15			\n\t"\
			"movaps	%%xmm5,%%xmm7			\n\t						subpd	%%xmm14,%%xmm9 			\n\t"\
			"subpd	%%xmm4,%%xmm5			\n\t						subpd	%%xmm11,%%xmm10			\n\t"\
			"mulpd	 0x40(%%rbx),%%xmm4		\n\t						movaps	%%xmm9 ,0x180(%%rdi)	\n\t"\
			"mulpd	 0x70(%%rbx),%%xmm5		\n\t						addpd	%%xmm8 ,%%xmm11			\n\t"\
			"mulpd	 0x10(%%rbx),%%xmm7		\n\t						subpd	%%xmm13,%%xmm15			\n\t"\
			"addpd	%%xmm3,%%xmm2			\n\t						subpd	%%xmm12,%%xmm10			\n\t"\
			"movaps	%%xmm6,%%xmm1			\n\t						movaps	%%xmm15,0x200(%%rdi)	\n\t"\
			"subpd	%%xmm3,%%xmm6			\n\t						movaps	%%xmm10,0x300(%%rdi)	\n\t"\
			"mulpd	 0x80(%%rbx),%%xmm6		\n\t						addpd	%%xmm8 ,%%xmm12			\n\t"\
			"mulpd	 0x50(%%rbx),%%xmm3		\n\t						addpd	%%xmm14,%%xmm11			\n\t"\
			"addpd	%%xmm1,%%xmm2			\n\t						addpd	%%xmm13,%%xmm12			\n\t"\
			"mulpd	 0x20(%%rbx),%%xmm1		\n\t						movaps	%%xmm11,0x480(%%rdi)	\n\t"\
			"addpd	%%xmm2,%%xmm0			\n\t						movaps	%%xmm12,0x080(%%rdi)	\n\t"\
			"mulpd	 0x90(%%rbx),%%xmm2		\n\t						/********************************************/\n\t"\
			"movaps	%%xmm0,(%%rcx)			\n\t						/*          Imaginary Parts:                */\n\t"\
			"addpd	%%xmm0,%%xmm2			\n\t						/********************************************/\n\t"\
			"addpd	%%xmm1,%%xmm7			\n\t						addq	$0x10,%%rdx				\n\t"\
			"addpd	0x180(%%rcx),%%xmm1	\n\t							movaps	0x020(%%rdx),%%xmm9 	\n\t"\
			"addpd	%%xmm3,%%xmm4			\n\t						movaps	0x140(%%rdx),%%xmm13	\n\t"\
			"addpd	0x300(%%rcx),%%xmm3	\n\t							movaps	0x040(%%rdx),%%xmm10	\n\t"\
			"addpd	%%xmm6,%%xmm5			\n\t						movaps	0x120(%%rdx),%%xmm14	\n\t"\
			"addpd	-0x100(%%rcx),%%xmm6	\n\t						movaps	0x060(%%rdx),%%xmm11	\n\t"\
			"movaps	%%xmm2,%%xmm0			\n\t						movaps	0x100(%%rdx),%%xmm15	\n\t"\
			"subpd	%%xmm1,%%xmm2			\n\t						movaps	0x080(%%rdx),%%xmm12	\n\t"\
			"addpd	%%xmm0,%%xmm1			\n\t						movaps	0x0e0(%%rdx),%%xmm8 	\n\t"\
			"subpd	%%xmm7,%%xmm2			\n\t						subpd	%%xmm13,%%xmm9 			\n\t"\
			"addpd	%%xmm0,%%xmm7			\n\t						subpd	%%xmm14,%%xmm10			\n\t"\
			"subpd	%%xmm6,%%xmm1			\n\t						subpd	%%xmm15,%%xmm11			\n\t"\
			"subpd	%%xmm3,%%xmm2			\n\t						subpd	%%xmm8 ,%%xmm12			\n\t"\
			"movaps	%%xmm1,0x180(%%rcx)	\n\t							movaps	%%xmm9 ,0x400(%%rdi)	\n\t"\
			"addpd	%%xmm0,%%xmm3			\n\t						movaps	%%xmm10,0x280(%%rdi)	\n\t"\
			"subpd	%%xmm5,%%xmm7			\n\t						movaps	%%xmm11,0x100(%%rdi)	\n\t"\
			"subpd	%%xmm4,%%xmm2			\n\t						movaps	%%xmm12,0x500(%%rdi)	\n\t"\
			"movaps	%%xmm7,0x200(%%rcx)	\n\t							addpd	%%xmm13,%%xmm13			\n\t"\
			"movaps	%%xmm2,0x300(%%rcx)	\n\t							addpd	%%xmm14,%%xmm14			\n\t"\
			"addpd	%%xmm0,%%xmm4			\n\t						mulpd	(%%r8),%%xmm15			\n\t"\
			"addpd	%%xmm6,%%xmm3			\n\t						mulpd	(%%r8),%%xmm8 			\n\t"\
			"addpd	%%xmm5,%%xmm4			\n\t						addpd	%%xmm13,%%xmm9 			\n\t"\
			"movaps	%%xmm3,-0x100(%%rcx)	\n\t						addpd	%%xmm14,%%xmm10			\n\t"\
			"movaps	%%xmm4,0x080(%%rcx)	\n\t							movaps	0x0a0(%%rdx),%%xmm13	\n\t"\
			"/********************************************/\n\t			movaps	0x0c0(%%rdx),%%xmm14	\n\t"\
			"/*          Imaginary Parts:                */\n\t			addpd	%%xmm15,%%xmm11			\n\t"\
			"/********************************************/\n\t			addpd	%%xmm8 ,%%xmm12			\n\t"\
			"addq	$0x10,%%rax				\n\t						subpd	%%xmm14,%%xmm13			\n\t"\
			"movaps	0x020(%%rax),%%xmm1	\n\t							movaps	%%xmm13,0x380(%%rdi)	\n\t"\
			"movaps	0x140(%%rax),%%xmm5	\n\t							addpd	%%xmm14,%%xmm14			\n\t"\
			"movaps	0x040(%%rax),%%xmm2	\n\t							movaps	(%%rdx),%%xmm8 			\n\t"\
			"movaps	0x120(%%rax),%%xmm6	\n\t							addpd	%%xmm14,%%xmm13			\n\t"\
			"movaps	0x060(%%rax),%%xmm3	\n\t							addq	$0x10,%%rdi				\n\t"\
			"movaps	0x100(%%rax),%%xmm7	\n\t							subpd	%%xmm10,%%xmm9 			\n\t"\
			"movaps	0x080(%%rax),%%xmm4	\n\t							subpd	%%xmm10,%%xmm13			\n\t"\
			"movaps	0x0e0(%%rax),%%xmm0	\n\t							movaps	%%xmm9 ,%%xmm14			\n\t"\
			"subpd	%%xmm5,%%xmm1			\n\t						subpd	%%xmm10,%%xmm11			\n\t"\
			"subpd	%%xmm6,%%xmm2			\n\t						movaps	%%xmm9 ,%%xmm15			\n\t"\
			"subpd	%%xmm7,%%xmm3			\n\t						mulpd	     (%%rsi),%%xmm9 		\n\t"\
			"subpd	%%xmm0,%%xmm4			\n\t						movaps	%%xmm9 ,0x180(%%rdi)	\n\t"\
			"movaps	%%xmm1,0x400(%%rcx)	\n\t							subpd	%%xmm10,%%xmm12			\n\t"\
			"movaps	%%xmm2,0x280(%%rcx)	\n\t							mulpd	-0x10(%%rsi),%%xmm10		\n\t"\
			"movaps	%%xmm3,0x100(%%rcx)	\n\t							addpd	%%xmm13,%%xmm14			\n\t"\
			"movaps	%%xmm4,-0x080(%%rcx)	\n\t						subpd	%%xmm11,%%xmm15			\n\t"\
			"addpd	%%xmm5,%%xmm5			\n\t						mulpd	 0x60(%%rsi),%%xmm15		\n\t"\
			"addpd	%%xmm6,%%xmm6			\n\t						movaps	%%xmm15,0x480(%%rdi)	\n\t"\
			"mulpd	(%%r8),%%xmm7			\n\t						movaps	%%xmm11,%%xmm9 			\n\t"\
			"mulpd	(%%r8),%%xmm0			\n\t						addpd	%%xmm12,%%xmm11			\n\t"\
			"addpd	%%xmm5,%%xmm1			\n\t						mulpd	 0x30(%%rsi),%%xmm9 		\n\t"\
			"addpd	%%xmm6,%%xmm2			\n\t						movaps	%%xmm9 ,0x300(%%rdi)	\n\t"\
			"movaps	0x0a0(%%rax),%%xmm5	\n\t							movaps	%%xmm13,%%xmm15			\n\t"\
			"movaps	0x0c0(%%rax),%%xmm6	\n\t							subpd	%%xmm12,%%xmm13			\n\t"\
			"addpd	%%xmm7,%%xmm3			\n\t						mulpd	 0x40(%%rsi),%%xmm12		\n\t"\
			"addpd	%%xmm0,%%xmm4			\n\t						mulpd	 0x70(%%rsi),%%xmm13		\n\t"\
			"subpd	%%xmm6,%%xmm5			\n\t						mulpd	 0x10(%%rsi),%%xmm15		\n\t"\
			"movaps	%%xmm5,0x380(%%rcx)	\n\t							addpd	%%xmm11,%%xmm10			\n\t"\
			"addpd	%%xmm6,%%xmm6			\n\t						movaps	%%xmm14,%%xmm9 			\n\t"\
			"movaps	(%%rax),%%xmm0			\n\t						subpd	%%xmm11,%%xmm14			\n\t"\
			"addpd	%%xmm6,%%xmm5			\n\t						mulpd	 0x80(%%rsi),%%xmm14		\n\t"\
			"addq	$0x10,%%rcx				\n\t						mulpd	 0x50(%%rsi),%%xmm11		\n\t"\
			"subpd	%%xmm2,%%xmm1			\n\t						addpd	%%xmm9 ,%%xmm10			\n\t"\
			"subpd	%%xmm2,%%xmm5			\n\t						mulpd	 0x20(%%rsi),%%xmm9 		\n\t"\
			"movaps	%%xmm1,%%xmm6			\n\t						addpd	%%xmm10,%%xmm8 			\n\t"\
			"subpd	%%xmm2,%%xmm3			\n\t						mulpd	 0x90(%%rsi),%%xmm10		\n\t"\
			"movaps	%%xmm1,%%xmm7			\n\t						movaps	%%xmm8 ,(%%rdi)			\n\t"\
			"mulpd	     (%%rbx),%%xmm1		\n\t						addpd	%%xmm8 ,%%xmm10			\n\t"\
			"movaps	%%xmm1,0x180(%%rcx)	\n\t							addpd	%%xmm9 ,%%xmm15			\n\t"\
			"subpd	%%xmm2,%%xmm4			\n\t						addpd	0x180(%%rdi),%%xmm9 	\n\t"\
			"mulpd	-0x10(%%rbx),%%xmm2		\n\t						addpd	%%xmm11,%%xmm12			\n\t"\
			"addpd	%%xmm5,%%xmm6			\n\t						addpd	0x300(%%rdi),%%xmm11	\n\t"\
			"subpd	%%xmm3,%%xmm7			\n\t						addpd	%%xmm14,%%xmm13			\n\t"\
			"mulpd	 0x60(%%rbx),%%xmm7		\n\t						addpd	0x480(%%rdi),%%xmm14	\n\t"\
			"movaps	%%xmm7,-0x100(%%rcx)	\n\t						movaps	%%xmm10,%%xmm8 			\n\t"\
			"movaps	%%xmm3,%%xmm1			\n\t						subpd	%%xmm9 ,%%xmm10			\n\t"\
			"addpd	%%xmm4,%%xmm3			\n\t						addpd	%%xmm8 ,%%xmm9 			\n\t"\
			"mulpd	 0x30(%%rbx),%%xmm1		\n\t						subpd	%%xmm15,%%xmm10			\n\t"\
			"movaps	%%xmm1,0x300(%%rcx)	\n\t							addpd	%%xmm8 ,%%xmm15			\n\t"\
			"movaps	%%xmm5,%%xmm7			\n\t						subpd	%%xmm14,%%xmm9 			\n\t"\
			"subpd	%%xmm4,%%xmm5			\n\t						subpd	%%xmm11,%%xmm10			\n\t"\
			"mulpd	 0x40(%%rbx),%%xmm4		\n\t						movaps	%%xmm9 ,0x180(%%rdi)	\n\t"\
			"mulpd	 0x70(%%rbx),%%xmm5		\n\t						addpd	%%xmm8 ,%%xmm11			\n\t"\
			"mulpd	 0x10(%%rbx),%%xmm7		\n\t						subpd	%%xmm13,%%xmm15			\n\t"\
			"addpd	%%xmm3,%%xmm2			\n\t						subpd	%%xmm12,%%xmm10			\n\t"\
			"movaps	%%xmm6,%%xmm1			\n\t						movaps	%%xmm15,0x200(%%rdi)	\n\t"\
			"subpd	%%xmm3,%%xmm6			\n\t						movaps	%%xmm10,0x300(%%rdi)	\n\t"\
			"mulpd	 0x80(%%rbx),%%xmm6		\n\t						addpd	%%xmm8 ,%%xmm12			\n\t"\
			"mulpd	 0x50(%%rbx),%%xmm3		\n\t						addpd	%%xmm14,%%xmm11			\n\t"\
			"addpd	%%xmm1,%%xmm2			\n\t						addpd	%%xmm13,%%xmm12			\n\t"\
			"mulpd	 0x20(%%rbx),%%xmm1		\n\t						movaps	%%xmm11,0x480(%%rdi)	\n\t"\
			"addpd	%%xmm2,%%xmm0			\n\t						movaps	%%xmm12,0x080(%%rdi)	\n\t"\
			"mulpd	 0x90(%%rbx),%%xmm2		\n\t						/********************************************/\n\t"\
			"movaps	%%xmm0,(%%rcx)			\n\t						/*        Here are the 5 sine terms:        */\n\t"\
			"addpd	%%xmm0,%%xmm2			\n\t						/********************************************/\n\t"\
			"addpd	%%xmm1,%%xmm7			\n\t						subq	$0x10,%%rdx				\n\t"\
			"addpd	0x180(%%rcx),%%xmm1	\n\t							addq	$0xa0,%%rsi				\n\t"\
			"addpd	%%xmm3,%%xmm4			\n\t						/********************************************/\n\t"\
			"addpd	0x300(%%rcx),%%xmm3	\n\t							/*               Real Parts:                */\n\t"\
			"addpd	%%xmm6,%%xmm5			\n\t						/********************************************/\n\t"\
			"addpd	-0x100(%%rcx),%%xmm6	\n\t						movaps	0x400(%%rdi),%%xmm9 	\n\t"\
			"movaps	%%xmm2,%%xmm0			\n\t						movaps	0x280(%%rdi),%%xmm10	\n\t"\
			"subpd	%%xmm1,%%xmm2			\n\t						movaps	0x100(%%rdi),%%xmm11	\n\t"\
			"addpd	%%xmm0,%%xmm1			\n\t						movaps	0x500(%%rdi),%%xmm12	\n\t"\
			"subpd	%%xmm7,%%xmm2			\n\t						movaps	0x380(%%rdi),%%xmm13	\n\t"\
			"addpd	%%xmm0,%%xmm7			\n\t						addpd	%%xmm10,%%xmm9 			\n\t"\
			"subpd	%%xmm6,%%xmm1			\n\t						addpd	%%xmm10,%%xmm13			\n\t"\
			"subpd	%%xmm3,%%xmm2			\n\t						movaps	%%xmm9 ,%%xmm14			\n\t"\
			"movaps	%%xmm1,0x180(%%rcx)	\n\t							addpd	%%xmm10,%%xmm11			\n\t"\
			"addpd	%%xmm0,%%xmm3			\n\t						movaps	%%xmm9 ,%%xmm15			\n\t"\
			"subpd	%%xmm5,%%xmm7			\n\t						mulpd	     (%%rsi),%%xmm9 		\n\t"\
			"subpd	%%xmm4,%%xmm2			\n\t						movaps	%%xmm9 ,0x380(%%rdi)	\n\t"\
			"movaps	%%xmm7,0x200(%%rcx)	\n\t							addpd	%%xmm10,%%xmm12			\n\t"\
			"movaps	%%xmm2,0x300(%%rcx)	\n\t							mulpd	-0xb0(%%rsi),%%xmm10		\n\t"\
			"addpd	%%xmm0,%%xmm4			\n\t						addpd	%%xmm13,%%xmm14			\n\t"\
			"addpd	%%xmm6,%%xmm3			\n\t						subpd	%%xmm11,%%xmm15			\n\t"\
			"addpd	%%xmm5,%%xmm4			\n\t						mulpd	 0x60(%%rsi),%%xmm15		\n\t"\
			"movaps	%%xmm3,-0x100(%%rcx)	\n\t						movaps	%%xmm15,0x100(%%rdi)	\n\t"\
			"movaps	%%xmm4,0x080(%%rcx)	\n\t							movaps	%%xmm11,%%xmm9 			\n\t"\
			"/********************************************/\n\t			addpd	%%xmm12,%%xmm11			\n\t"\
			"/*        Here are the 5 sine terms:        */\n\t			mulpd	 0x30(%%rsi),%%xmm9 		\n\t"\
			"/********************************************/\n\t			movaps	%%xmm9 ,0x500(%%rdi)	\n\t"\
			"subq	$0x10,%%rax				\n\t						movaps	%%xmm13,%%xmm15			\n\t"\
			"addq	$0xa0,%%rbx				\n\t						subpd	%%xmm12,%%xmm13			\n\t"\
			"/********************************************/\n\t			mulpd	 0x40(%%rsi),%%xmm12		\n\t"\
			"/*               Real Parts:                */\n\t			mulpd	 0x70(%%rsi),%%xmm13		\n\t"\
			"/********************************************/\n\t			mulpd	 0x10(%%rsi),%%xmm15		\n\t"\
			"movaps	0x400(%%rcx),%%xmm1	\n\t							subpd	%%xmm11,%%xmm10			\n\t"\
			"movaps	0x280(%%rcx),%%xmm2	\n\t							movaps	%%xmm14,%%xmm9 			\n\t"\
			"movaps	0x100(%%rcx),%%xmm3	\n\t							subpd	%%xmm11,%%xmm14			\n\t"\
			"movaps	-0x080(%%rcx),%%xmm4	\n\t						mulpd	 0x80(%%rsi),%%xmm14		\n\t"\
			"movaps	0x380(%%rcx),%%xmm5	\n\t							mulpd	 0x50(%%rsi),%%xmm11		\n\t"\
			"addpd	%%xmm2,%%xmm1			\n\t						subpd	%%xmm9 ,%%xmm10			\n\t"\
			"addpd	%%xmm2,%%xmm5			\n\t						mulpd	 0x20(%%rsi),%%xmm9 		\n\t"\
			"movaps	%%xmm1,%%xmm6			\n\t						mulpd	 0x90(%%rsi),%%xmm10		\n\t"\
			"addpd	%%xmm2,%%xmm3			\n\t						addpd	%%xmm9 ,%%xmm15			\n\t"\
			"movaps	%%xmm1,%%xmm7			\n\t						addpd	0x380(%%rdi),%%xmm9 	\n\t"\
			"mulpd	     (%%rbx),%%xmm1		\n\t						addpd	%%xmm11,%%xmm12			\n\t"\
			"movaps	%%xmm1,0x380(%%rcx)	\n\t							addpd	0x500(%%rdi),%%xmm11	\n\t"\
			"addpd	%%xmm2,%%xmm4			\n\t						addpd	%%xmm14,%%xmm13			\n\t"\
			"mulpd	-0xb0(%%rbx),%%xmm2		\n\t						addpd	0x100(%%rdi),%%xmm14	\n\t"\
			"addpd	%%xmm5,%%xmm6			\n\t						xorpd	%%xmm8 ,%%xmm8 			\n\t"\
			"subpd	%%xmm3,%%xmm7			\n\t						subpd	%%xmm10,%%xmm8 			\n\t"\
			"mulpd	 0x60(%%rbx),%%xmm7		\n\t						addpd	%%xmm9 ,%%xmm8 			\n\t"\
			"movaps	%%xmm7,0x100(%%rcx)	\n\t							addpd	%%xmm10,%%xmm9 			\n\t"\
			"movaps	%%xmm3,%%xmm1			\n\t						addpd	%%xmm15,%%xmm8 			\n\t"\
			"addpd	%%xmm4,%%xmm3			\n\t						addpd	%%xmm10,%%xmm15			\n\t"\
			"mulpd	 0x30(%%rbx),%%xmm1		\n\t						subpd	%%xmm14,%%xmm9 			\n\t"\
			"movaps	%%xmm1,-0x080(%%rcx)	\n\t						addpd	%%xmm11,%%xmm8 			\n\t"\
			"movaps	%%xmm5,%%xmm7			\n\t						addpd	%%xmm10,%%xmm11			\n\t"\
			"subpd	%%xmm4,%%xmm5			\n\t						subpd	%%xmm13,%%xmm15			\n\t"\
			"mulpd	 0x40(%%rbx),%%xmm4		\n\t						addpd	%%xmm12,%%xmm8 			\n\t"\
			"mulpd	 0x70(%%rbx),%%xmm5		\n\t						addpd	%%xmm10,%%xmm12			\n\t"\
			"mulpd	 0x10(%%rbx),%%xmm7		\n\t						addpd	%%xmm14,%%xmm11			\n\t"\
			"subpd	%%xmm3,%%xmm2			\n\t						addpd	%%xmm13,%%xmm12			\n\t"\
			"movaps	%%xmm6,%%xmm1			\n\t						movaps	%%xmm9 ,%%xmm10			\n\t"\
			"subpd	%%xmm3,%%xmm6			\n\t						addpd	0x180(%%rdi),%%xmm9 	\n\t"\
			"mulpd	 0x80(%%rbx),%%xmm6		\n\t						addpd	%%xmm10,%%xmm10			\n\t"\
			"mulpd	 0x50(%%rbx),%%xmm3		\n\t						movaps	%%xmm9 ,0x180(%%rdi)	\n\t"\
			"subpd	%%xmm1,%%xmm2			\n\t						subpd	%%xmm10,%%xmm9 			\n\t"\
			"mulpd	 0x20(%%rbx),%%xmm1		\n\t						movaps	%%xmm9 ,0x400(%%rdi)	\n\t"\
			"mulpd	 0x90(%%rbx),%%xmm2		\n\t						movaps	%%xmm8 ,%%xmm13			\n\t"\
			"addpd	%%xmm1,%%xmm7			\n\t						addpd	0x300(%%rdi),%%xmm8 	\n\t"\
			"addpd	0x380(%%rcx),%%xmm1	\n\t							mulpd	(%%r8),%%xmm13			\n\t"\
			"addpd	%%xmm3,%%xmm4			\n\t						movaps	%%xmm8 ,0x300(%%rdi)	\n\t"\
			"addpd	-0x080(%%rcx),%%xmm3	\n\t						subpd	%%xmm13,%%xmm8 			\n\t"\
			"addpd	%%xmm6,%%xmm5			\n\t						movaps	%%xmm8 ,0x280(%%rdi)	\n\t"\
			"addpd	0x100(%%rcx),%%xmm6	\n\t							movaps	%%xmm11,%%xmm14			\n\t"\
			"xorpd	%%xmm0,%%xmm0			\n\t						addpd	0x480(%%rdi),%%xmm11	\n\t"\
			"subpd	%%xmm2,%%xmm0			\n\t						mulpd	(%%r8),%%xmm14			\n\t"\
			"addpd	%%xmm1,%%xmm0			\n\t						movaps	%%xmm11,0x480(%%rdi)	\n\t"\
			"addpd	%%xmm2,%%xmm1			\n\t						subpd	%%xmm14,%%xmm11			\n\t"\
			"addpd	%%xmm7,%%xmm0			\n\t						movaps	%%xmm11,0x100(%%rdi)	\n\t"\
			"addpd	%%xmm2,%%xmm7			\n\t						movaps	%%xmm12,%%xmm10			\n\t"\
			"subpd	%%xmm6,%%xmm1			\n\t						addpd	0x080(%%rdi),%%xmm12	\n\t"\
			"addpd	%%xmm3,%%xmm0			\n\t						mulpd	(%%r8),%%xmm10			\n\t"\
			"addpd	%%xmm2,%%xmm3			\n\t						movaps	%%xmm12,0x080(%%rdi)	\n\t"\
			"subpd	%%xmm5,%%xmm7			\n\t						subpd	%%xmm10,%%xmm12			\n\t"\
			"addpd	%%xmm4,%%xmm0			\n\t						movaps	%%xmm12,0x500(%%rdi)	\n\t"\
			"addpd	%%xmm2,%%xmm4			\n\t						movaps	%%xmm15,%%xmm13			\n\t"\
			"addpd	%%xmm6,%%xmm3			\n\t						addpd	0x200(%%rdi),%%xmm15	\n\t"\
			"addpd	%%xmm5,%%xmm4			\n\t						mulpd	(%%r8),%%xmm13			\n\t"\
			"movaps	%%xmm1,%%xmm2			\n\t						movaps	%%xmm15,0x200(%%rdi)	\n\t"\
			"addpd	0x180(%%rcx),%%xmm1	\n\t							subpd	%%xmm13,%%xmm15			\n\t"\
			"mulpd	(%%r8),%%xmm2			\n\t						movaps	%%xmm15,0x380(%%rdi)	\n\t"\
			"movaps	%%xmm1,0x180(%%rcx)	\n\t							/********************************************/\n\t"\
			"subpd	%%xmm2,%%xmm1			\n\t						/*          Imaginary Parts:                */\n\t"\
			"movaps	%%xmm1,0x400(%%rcx)	\n\t							/********************************************/\n\t"\
			"movaps	%%xmm0,%%xmm5			\n\t						subq	$0x10,%%rdi				\n\t"\
			"addpd	0x300(%%rcx),%%xmm0	\n\t							movaps	0x400(%%rdi),%%xmm9 	\n\t"\
			"addpd	%%xmm5,%%xmm5			\n\t						movaps	0x280(%%rdi),%%xmm10	\n\t"\
			"movaps	%%xmm0,0x300(%%rcx)	\n\t							movaps	0x100(%%rdi),%%xmm11	\n\t"\
			"subpd	%%xmm5,%%xmm0			\n\t						movaps	0x500(%%rdi),%%xmm12	\n\t"\
			"movaps	%%xmm0,0x280(%%rcx)	\n\t							movaps	0x380(%%rdi),%%xmm13	\n\t"\
			"movaps	%%xmm3,%%xmm6			\n\t						addpd	%%xmm10,%%xmm9 			\n\t"\
			"addpd	-0x100(%%rcx),%%xmm3	\n\t						addpd	%%xmm10,%%xmm13			\n\t"\
			"mulpd	(%%r8),%%xmm6			\n\t						movaps	%%xmm9 ,%%xmm14			\n\t"\
			"movaps	%%xmm3,-0x100(%%rcx)	\n\t						addpd	%%xmm10,%%xmm11			\n\t"\
			"subpd	%%xmm6,%%xmm3			\n\t						movaps	%%xmm9 ,%%xmm15			\n\t"\
			"movaps	%%xmm3,0x100(%%rcx)	\n\t							mulpd	     (%%rsi),%%xmm9 		\n\t"\
			"movaps	%%xmm4,%%xmm2			\n\t						movaps	%%xmm9 ,0x380(%%rdi)	\n\t"\
			"addpd	0x080(%%rcx),%%xmm4	\n\t							addpd	%%xmm10,%%xmm12			\n\t"\
			"addpd	%%xmm2,%%xmm2			\n\t						mulpd	-0xb0(%%rsi),%%xmm10		\n\t"\
			"movaps	%%xmm4,0x080(%%rcx)	\n\t							addpd	%%xmm13,%%xmm14			\n\t"\
			"subpd	%%xmm2,%%xmm4			\n\t						subpd	%%xmm11,%%xmm15			\n\t"\
			"movaps	%%xmm4,-0x080(%%rcx)	\n\t						mulpd	 0x60(%%rsi),%%xmm15		\n\t"\
			"movaps	%%xmm7,%%xmm5			\n\t						movaps	%%xmm15,0x100(%%rdi)	\n\t"\
			"addpd	0x200(%%rcx),%%xmm7	\n\t							movaps	%%xmm11,%%xmm9 			\n\t"\
			"mulpd	(%%r8),%%xmm5			\n\t						addpd	%%xmm12,%%xmm11			\n\t"\
			"movaps	%%xmm7,0x200(%%rcx)	\n\t							mulpd	 0x30(%%rsi),%%xmm9 		\n\t"\
			"subpd	%%xmm5,%%xmm7			\n\t						movaps	%%xmm9 ,0x500(%%rdi)	\n\t"\
			"movaps	%%xmm7,0x380(%%rcx)	\n\t							movaps	%%xmm13,%%xmm15			\n\t"\
			"/********************************************/\n\t			subpd	%%xmm12,%%xmm13			\n\t"\
			"/*          Imaginary Parts:                */\n\t			mulpd	 0x40(%%rsi),%%xmm12		\n\t"\
			"/********************************************/\n\t			mulpd	 0x70(%%rsi),%%xmm13		\n\t"\
			"subq	$0x10,%%rcx				\n\t						mulpd	 0x10(%%rsi),%%xmm15		\n\t"\
			"movaps	0x400(%%rcx),%%xmm1	\n\t							subpd	%%xmm11,%%xmm10			\n\t"\
			"movaps	0x280(%%rcx),%%xmm2	\n\t							movaps	%%xmm14,%%xmm9 			\n\t"\
			"movaps	0x100(%%rcx),%%xmm3	\n\t							subpd	%%xmm11,%%xmm14			\n\t"\
			"movaps	-0x080(%%rcx),%%xmm4	\n\t						mulpd	 0x80(%%rsi),%%xmm14		\n\t"\
			"movaps	0x380(%%rcx),%%xmm5	\n\t							mulpd	 0x50(%%rsi),%%xmm11		\n\t"\
			"addpd	%%xmm2,%%xmm1			\n\t						subpd	%%xmm9 ,%%xmm10			\n\t"\
			"addpd	%%xmm2,%%xmm5			\n\t						mulpd	 0x20(%%rsi),%%xmm9 		\n\t"\
			"movaps	%%xmm1,%%xmm6			\n\t						mulpd	 0x90(%%rsi),%%xmm10		\n\t"\
			"addpd	%%xmm2,%%xmm3			\n\t						addpd	%%xmm9 ,%%xmm15			\n\t"\
			"movaps	%%xmm1,%%xmm7			\n\t						addpd	0x380(%%rdi),%%xmm9 	\n\t"\
			"mulpd	     (%%rbx),%%xmm1		\n\t						addpd	%%xmm11,%%xmm12			\n\t"\
			"movaps	%%xmm1,0x380(%%rcx)	\n\t							addpd	0x500(%%rdi),%%xmm11	\n\t"\
			"addpd	%%xmm2,%%xmm4			\n\t						addpd	%%xmm14,%%xmm13			\n\t"\
			"mulpd	-0xb0(%%rbx),%%xmm2		\n\t						addpd	0x100(%%rdi),%%xmm14	\n\t"\
			"addpd	%%xmm5,%%xmm6			\n\t						xorpd	%%xmm8 ,%%xmm8 			\n\t"\
			"subpd	%%xmm3,%%xmm7			\n\t						subpd	%%xmm10,%%xmm8 			\n\t"\
			"mulpd	 0x60(%%rbx),%%xmm7		\n\t						addpd	%%xmm9 ,%%xmm8 			\n\t"\
			"movaps	%%xmm7,0x100(%%rcx)	\n\t							addpd	%%xmm10,%%xmm9 			\n\t"\
			"movaps	%%xmm3,%%xmm1			\n\t						addpd	%%xmm15,%%xmm8 			\n\t"\
			"addpd	%%xmm4,%%xmm3			\n\t						addpd	%%xmm10,%%xmm15			\n\t"\
			"mulpd	 0x30(%%rbx),%%xmm1		\n\t						subpd	%%xmm14,%%xmm9 			\n\t"\
			"movaps	%%xmm1,-0x080(%%rcx)	\n\t						addpd	%%xmm11,%%xmm8 			\n\t"\
			"movaps	%%xmm5,%%xmm7			\n\t						addpd	%%xmm10,%%xmm11			\n\t"\
			"subpd	%%xmm4,%%xmm5			\n\t						subpd	%%xmm13,%%xmm15			\n\t"\
			"mulpd	 0x40(%%rbx),%%xmm4		\n\t						addpd	%%xmm12,%%xmm8 			\n\t"\
			"mulpd	 0x70(%%rbx),%%xmm5		\n\t						addpd	%%xmm10,%%xmm12			\n\t"\
			"mulpd	 0x10(%%rbx),%%xmm7		\n\t						addpd	%%xmm14,%%xmm11			\n\t"\
			"subpd	%%xmm3,%%xmm2			\n\t						addpd	%%xmm13,%%xmm12			\n\t"\
			"movaps	%%xmm6,%%xmm1			\n\t						movaps	%%xmm9 ,%%xmm10			\n\t"\
			"subpd	%%xmm3,%%xmm6			\n\t						addpd	0x180(%%rdi),%%xmm9 	\n\t"\
			"mulpd	 0x80(%%rbx),%%xmm6		\n\t						addpd	%%xmm10,%%xmm10			\n\t"\
			"mulpd	 0x50(%%rbx),%%xmm3		\n\t						movaps	%%xmm9 ,0x400(%%rdi)	\n\t"\
			"subpd	%%xmm1,%%xmm2			\n\t						subpd	%%xmm10,%%xmm9 			\n\t"\
			"mulpd	 0x20(%%rbx),%%xmm1		\n\t						movaps	%%xmm9 ,0x180(%%rdi)	\n\t"\
			"mulpd	 0x90(%%rbx),%%xmm2		\n\t						movaps	%%xmm8 ,%%xmm13			\n\t"\
			"addpd	%%xmm1,%%xmm7			\n\t						addpd	0x300(%%rdi),%%xmm8 	\n\t"\
			"addpd	0x380(%%rcx),%%xmm1	\n\t							mulpd	(%%r8),%%xmm13			\n\t"\
			"addpd	%%xmm3,%%xmm4			\n\t						movaps	%%xmm8 ,0x280(%%rdi)	\n\t"\
			"addpd	-0x080(%%rcx),%%xmm3	\n\t						subpd	%%xmm13,%%xmm8 			\n\t"\
			"addpd	%%xmm6,%%xmm5			\n\t						movaps	%%xmm8 ,0x300(%%rdi)	\n\t"\
			"addpd	0x100(%%rcx),%%xmm6	\n\t							movaps	%%xmm11,%%xmm14			\n\t"\
			"xorpd	%%xmm0,%%xmm0			\n\t						addpd	0x480(%%rdi),%%xmm11	\n\t"\
			"subpd	%%xmm2,%%xmm0			\n\t						mulpd	(%%r8),%%xmm14			\n\t"\
			"addpd	%%xmm1,%%xmm0			\n\t						movaps	%%xmm11,0x100(%%rdi)	\n\t"\
			"addpd	%%xmm2,%%xmm1			\n\t						subpd	%%xmm14,%%xmm11			\n\t"\
			"addpd	%%xmm7,%%xmm0			\n\t						movaps	%%xmm11,0x480(%%rdi)	\n\t"\
			"addpd	%%xmm2,%%xmm7			\n\t						movaps	%%xmm12,%%xmm10			\n\t"\
			"subpd	%%xmm6,%%xmm1			\n\t						addpd	0x080(%%rdi),%%xmm12	\n\t"\
			"addpd	%%xmm3,%%xmm0			\n\t						mulpd	(%%r8),%%xmm10			\n\t"\
			"addpd	%%xmm2,%%xmm3			\n\t						movaps	%%xmm12,0x500(%%rdi)	\n\t"\
			"subpd	%%xmm5,%%xmm7			\n\t						subpd	%%xmm10,%%xmm12			\n\t"\
			"addpd	%%xmm4,%%xmm0			\n\t						movaps	%%xmm12,0x080(%%rdi)	\n\t"\
			"addpd	%%xmm2,%%xmm4			\n\t						movaps	%%xmm15,%%xmm13			\n\t"\
			"addpd	%%xmm6,%%xmm3			\n\t						addpd	0x200(%%rdi),%%xmm15	\n\t"\
			"addpd	%%xmm5,%%xmm4			\n\t						mulpd	(%%r8),%%xmm13			\n\t"\
			"movaps	%%xmm1,%%xmm2			\n\t						movaps	%%xmm15,0x380(%%rdi)	\n\t"\
			"addpd	0x180(%%rcx),%%xmm1	\n\t							subpd	%%xmm13,%%xmm15			\n\t"\
			"mulpd	(%%r8),%%xmm2			\n\t						movaps	%%xmm15,0x200(%%rdi)	\n\t"\
			"movaps	%%xmm1,0x400(%%rcx)	\n\t							\n\t"\
			"subpd	%%xmm2,%%xmm1			\n\t						/* RADIX_11(t2[0-A], s1p[22,34,02,14,26,38,06,18,30,42,10]):*/\n\t"\
			"movaps	%%xmm1,0x180(%%rcx)	\n\t							/********************************************/\n\t"\
			"movaps	%%xmm0,%%xmm5			\n\t						/*       Here are the 5 cosine terms:       */\n\t"\
			"addpd	0x300(%%rcx),%%xmm0	\n\t							/********************************************/\n\t"\
			"addpd	%%xmm5,%%xmm5			\n\t						addq	$0x2c0,%%rdx	/* t20r */	\n\t"\
			"movaps	%%xmm0,0x280(%%rcx)	\n\t							subq	$0x0a0,%%rsi	/* cc */	\n\t"\
			"subpd	%%xmm5,%%xmm0			\n\t						addq	$0x2c0,%%rdi	/* s1p22r */\n\t"\
			"movaps	%%xmm0,0x300(%%rcx)	\n\t							movaps	0x020(%%rdx),%%xmm9 	\n\t"\
			"movaps	%%xmm3,%%xmm6			\n\t						movaps	0x140(%%rdx),%%xmm13	\n\t"\
			"addpd	-0x100(%%rcx),%%xmm3	\n\t						movaps	0x040(%%rdx),%%xmm10	\n\t"\
			"addpd	%%xmm6,%%xmm6			\n\t						movaps	0x120(%%rdx),%%xmm14	\n\t"\
			"movaps	%%xmm3,0x100(%%rcx)	\n\t							movaps	0x060(%%rdx),%%xmm11	\n\t"\
			"subpd	%%xmm6,%%xmm3			\n\t						movaps	0x100(%%rdx),%%xmm15	\n\t"\
			"movaps	%%xmm3,-0x100(%%rcx)	\n\t						movaps	0x080(%%rdx),%%xmm12	\n\t"\
			"movaps	%%xmm4,%%xmm2			\n\t						movaps	0x0e0(%%rdx),%%xmm8 	\n\t"\
			"addpd	0x080(%%rcx),%%xmm4	\n\t							addq	$0x10,%%rdi				\n\t"\
			"mulpd	(%%r8),%%xmm2			\n\t						subpd	%%xmm13,%%xmm9 			\n\t"\
			"movaps	%%xmm4,-0x080(%%rcx)	\n\t						subpd	%%xmm14,%%xmm10			\n\t"\
			"subpd	%%xmm2,%%xmm4			\n\t						subpd	%%xmm15,%%xmm11			\n\t"\
			"movaps	%%xmm4,0x080(%%rcx)	\n\t							subpd	%%xmm8 ,%%xmm12			\n\t"\
			"movaps	%%xmm7,%%xmm5			\n\t						movaps	%%xmm9 ,-0x180(%%rdi)	\n\t"\
			"addpd	0x200(%%rcx),%%xmm7	\n\t							movaps	%%xmm10,0x280(%%rdi)	\n\t"\
			"addpd	%%xmm5,%%xmm5			\n\t						movaps	%%xmm11,0x100(%%rdi)	\n\t"\
			"movaps	%%xmm7,0x380(%%rcx)	\n\t							movaps	%%xmm12,-0x080(%%rdi)	\n\t"\
			"subpd	%%xmm5,%%xmm7			\n\t						mulpd	(%%r8),%%xmm13			\n\t"\
			"movaps	%%xmm7,0x200(%%rcx)	\n\t							mulpd	(%%r8),%%xmm14			\n\t"\
			"\n\t														addpd	%%xmm15,%%xmm15			\n\t"\
			"/* RADIX_11(t3[0-A],s[33,01,13,25,37,05,17,29,41,09,21]):*/addpd	%%xmm8 ,%%xmm8 			\n\t"\
			"/********************************************/\n\t			addpd	%%xmm13,%%xmm9 			\n\t"\
			"/*       Here are the 5 cosine terms:       */\n\t			addpd	%%xmm14,%%xmm10			\n\t"\
			"/********************************************/\n\t			movaps	0x0a0(%%rdx),%%xmm13	\n\t"\
			"addq	$0x2c0,%%rax	/* t30r */	\n\t					movaps	0x0c0(%%rdx),%%xmm14	\n\t"\
			"subq	$0x0a0,%%rbx	/* cc */	\n\t					addpd	%%xmm15,%%xmm11			\n\t"\
			"addq	$0x2c0,%%rcx	/* s1p33r */\n\t					addpd	%%xmm8 ,%%xmm12			\n\t"\
			"movaps	0x020(%%rax),%%xmm1	\n\t							subpd	%%xmm14,%%xmm13			\n\t"\
			"movaps	0x140(%%rax),%%xmm5	\n\t							movaps	%%xmm13,-0x200(%%rdi)	\n\t"\
			"movaps	0x040(%%rax),%%xmm2	\n\t							addpd	%%xmm14,%%xmm14			\n\t"\
			"movaps	0x120(%%rax),%%xmm6	\n\t							movaps	(%%rdx),%%xmm8 			\n\t"\
			"movaps	0x060(%%rax),%%xmm3	\n\t							addpd	%%xmm14,%%xmm13			\n\t"\
			"movaps	0x100(%%rax),%%xmm7	\n\t							/********************************************/\n\t"\
			"movaps	0x080(%%rax),%%xmm4	\n\t							/*               Real Parts:                */\n\t"\
			"movaps	0x0e0(%%rax),%%xmm0	\n\t							/********************************************/\n\t"\
			"addq	$0x10,%%rcx				\n\t						subq	$0x10,%%rdi				\n\t"\
			"subpd	%%xmm5,%%xmm1			\n\t						subpd	%%xmm10,%%xmm9 			\n\t"\
			"subpd	%%xmm6,%%xmm2			\n\t						subpd	%%xmm10,%%xmm13			\n\t"\
			"subpd	%%xmm7,%%xmm3			\n\t						movaps	%%xmm9 ,%%xmm14			\n\t"\
			"subpd	%%xmm0,%%xmm4			\n\t						subpd	%%xmm10,%%xmm11			\n\t"\
			"movaps	%%xmm1,-0x180(%%rcx)	\n\t						movaps	%%xmm9 ,%%xmm15			\n\t"\
			"movaps	%%xmm2,-0x300(%%rcx)	\n\t						mulpd	     (%%rsi),%%xmm9 		\n\t"\
			"movaps	%%xmm3,0x100(%%rcx)	\n\t							movaps	%%xmm9 ,0x180(%%rdi)	\n\t"\
			"movaps	%%xmm4,-0x080(%%rcx)	\n\t						subpd	%%xmm10,%%xmm12			\n\t"\
			"addpd	%%xmm5,%%xmm5			\n\t						mulpd	-0x10(%%rsi),%%xmm10		\n\t"\
			"mulpd	(%%r8),%%xmm6			\n\t						addpd	%%xmm13,%%xmm14			\n\t"\
			"mulpd	(%%r8),%%xmm7			\n\t						subpd	%%xmm11,%%xmm15			\n\t"\
			"addpd	%%xmm0,%%xmm0			\n\t						mulpd	 0x60(%%rsi),%%xmm15		\n\t"\
			"addpd	%%xmm5,%%xmm1			\n\t						movaps	%%xmm15,-0x100(%%rdi)	\n\t"\
			"addpd	%%xmm6,%%xmm2			\n\t						movaps	%%xmm11,%%xmm9 			\n\t"\
			"movaps	0x0a0(%%rax),%%xmm5	\n\t							addpd	%%xmm12,%%xmm11			\n\t"\
			"movaps	0x0c0(%%rax),%%xmm6	\n\t							mulpd	 0x30(%%rsi),%%xmm9 		\n\t"\
			"addpd	%%xmm7,%%xmm3			\n\t						movaps	%%xmm9 ,-0x280(%%rdi)	\n\t"\
			"addpd	%%xmm0,%%xmm4			\n\t						movaps	%%xmm13,%%xmm15			\n\t"\
			"subpd	%%xmm6,%%xmm5			\n\t						subpd	%%xmm12,%%xmm13			\n\t"\
			"movaps	%%xmm5,-0x200(%%rcx)	\n\t						mulpd	 0x40(%%rsi),%%xmm12		\n\t"\
			"addpd	%%xmm6,%%xmm6			\n\t						mulpd	 0x70(%%rsi),%%xmm13		\n\t"\
			"movaps	(%%rax),%%xmm0			\n\t						mulpd	 0x10(%%rsi),%%xmm15		\n\t"\
			"addpd	%%xmm6,%%xmm5			\n\t						addpd	%%xmm11,%%xmm10			\n\t"\
			"/********************************************/\n\t			movaps	%%xmm14,%%xmm9 			\n\t"\
			"/*               Real Parts:                */\n\t			subpd	%%xmm11,%%xmm14			\n\t"\
			"/********************************************/\n\t			mulpd	 0x80(%%rsi),%%xmm14		\n\t"\
			"subq	$0x10,%%rcx				\n\t						mulpd	 0x50(%%rsi),%%xmm11		\n\t"\
			"subpd	%%xmm2,%%xmm1			\n\t						addpd	%%xmm9 ,%%xmm10			\n\t"\
			"subpd	%%xmm2,%%xmm5			\n\t						mulpd	 0x20(%%rsi),%%xmm9 		\n\t"\
			"movaps	%%xmm1,%%xmm6			\n\t						addpd	%%xmm10,%%xmm8 			\n\t"\
			"subpd	%%xmm2,%%xmm3			\n\t						mulpd	 0x90(%%rsi),%%xmm10		\n\t"\
			"movaps	%%xmm1,%%xmm7			\n\t						movaps	%%xmm8 ,(%%rdi)			\n\t"\
			"mulpd	     (%%rbx),%%xmm1		\n\t						addpd	%%xmm8 ,%%xmm10			\n\t"\
			"movaps	%%xmm1,-0x400(%%rcx)	\n\t						addpd	%%xmm9 ,%%xmm15			\n\t"\
			"subpd	%%xmm2,%%xmm4			\n\t						addpd	0x180(%%rdi),%%xmm9 	\n\t"\
			"mulpd	-0x10(%%rbx),%%xmm2		\n\t						addpd	%%xmm11,%%xmm12			\n\t"\
			"addpd	%%xmm5,%%xmm6			\n\t						addpd	-0x280(%%rdi),%%xmm11	\n\t"\
			"subpd	%%xmm3,%%xmm7			\n\t						addpd	%%xmm14,%%xmm13			\n\t"\
			"mulpd	 0x60(%%rbx),%%xmm7		\n\t						addpd	-0x100(%%rdi),%%xmm14	\n\t"\
			"movaps	%%xmm7,-0x100(%%rcx)	\n\t						movaps	%%xmm10,%%xmm8 			\n\t"\
			"movaps	%%xmm3,%%xmm1			\n\t						subpd	%%xmm9 ,%%xmm10			\n\t"\
			"addpd	%%xmm4,%%xmm3			\n\t						addpd	%%xmm8 ,%%xmm9 			\n\t"\
			"mulpd	 0x30(%%rbx),%%xmm1		\n\t						subpd	%%xmm15,%%xmm10			\n\t"\
			"movaps	%%xmm1,-0x280(%%rcx)	\n\t						addpd	%%xmm8 ,%%xmm15			\n\t"\
			"movaps	%%xmm5,%%xmm7			\n\t						subpd	%%xmm14,%%xmm9 			\n\t"\
			"subpd	%%xmm4,%%xmm5			\n\t						subpd	%%xmm11,%%xmm10			\n\t"\
			"mulpd	 0x40(%%rbx),%%xmm4		\n\t						movaps	%%xmm9 ,0x180(%%rdi)	\n\t"\
			"mulpd	 0x70(%%rbx),%%xmm5		\n\t						addpd	%%xmm8 ,%%xmm11			\n\t"\
			"mulpd	 0x10(%%rbx),%%xmm7		\n\t						subpd	%%xmm13,%%xmm15			\n\t"\
			"addpd	%%xmm3,%%xmm2			\n\t						subpd	%%xmm12,%%xmm10			\n\t"\
			"movaps	%%xmm6,%%xmm1			\n\t						movaps	%%xmm15,0x200(%%rdi)	\n\t"\
			"subpd	%%xmm3,%%xmm6			\n\t						movaps	%%xmm10,-0x280(%%rdi)	\n\t"\
			"mulpd	 0x80(%%rbx),%%xmm6		\n\t						addpd	%%xmm8 ,%%xmm12			\n\t"\
			"mulpd	 0x50(%%rbx),%%xmm3		\n\t						addpd	%%xmm14,%%xmm11			\n\t"\
			"addpd	%%xmm1,%%xmm2			\n\t						addpd	%%xmm13,%%xmm12			\n\t"\
			"mulpd	 0x20(%%rbx),%%xmm1		\n\t						movaps	%%xmm11,-0x100(%%rdi)	\n\t"\
			"addpd	%%xmm2,%%xmm0			\n\t						movaps	%%xmm12,0x080(%%rdi)	\n\t"\
			"mulpd	 0x90(%%rbx),%%xmm2		\n\t						/********************************************/\n\t"\
			"movaps	%%xmm0,(%%rcx)			\n\t						/*          Imaginary Parts:                */\n\t"\
			"addpd	%%xmm0,%%xmm2			\n\t						/********************************************/\n\t"\
			"addpd	%%xmm1,%%xmm7			\n\t						addq	$0x10,%%rdx				\n\t"\
			"addpd	-0x400(%%rcx),%%xmm1	\n\t						movaps	0x020(%%rdx),%%xmm9 	\n\t"\
			"addpd	%%xmm3,%%xmm4			\n\t						movaps	0x140(%%rdx),%%xmm13	\n\t"\
			"addpd	-0x280(%%rcx),%%xmm3	\n\t						movaps	0x040(%%rdx),%%xmm10	\n\t"\
			"addpd	%%xmm6,%%xmm5			\n\t						movaps	0x120(%%rdx),%%xmm14	\n\t"\
			"addpd	-0x100(%%rcx),%%xmm6	\n\t						movaps	0x060(%%rdx),%%xmm11	\n\t"\
			"movaps	%%xmm2,%%xmm0			\n\t						movaps	0x100(%%rdx),%%xmm15	\n\t"\
			"subpd	%%xmm1,%%xmm2			\n\t						movaps	0x080(%%rdx),%%xmm12	\n\t"\
			"addpd	%%xmm0,%%xmm1			\n\t						movaps	0x0e0(%%rdx),%%xmm8 	\n\t"\
			"subpd	%%xmm7,%%xmm2			\n\t						subpd	%%xmm13,%%xmm9 			\n\t"\
			"addpd	%%xmm0,%%xmm7			\n\t						subpd	%%xmm14,%%xmm10			\n\t"\
			"subpd	%%xmm6,%%xmm1			\n\t						subpd	%%xmm15,%%xmm11			\n\t"\
			"subpd	%%xmm3,%%xmm2			\n\t						subpd	%%xmm8 ,%%xmm12			\n\t"\
			"movaps	%%xmm1,-0x400(%%rcx)	\n\t						movaps	%%xmm9 ,-0x180(%%rdi)	\n\t"\
			"addpd	%%xmm0,%%xmm3			\n\t						movaps	%%xmm10,0x280(%%rdi)	\n\t"\
			"subpd	%%xmm5,%%xmm7			\n\t						movaps	%%xmm11,0x100(%%rdi)	\n\t"\
			"subpd	%%xmm4,%%xmm2			\n\t						movaps	%%xmm12,-0x080(%%rdi)	\n\t"\
			"movaps	%%xmm7,-0x380(%%rcx)	\n\t						addpd	%%xmm13,%%xmm13			\n\t"\
			"movaps	%%xmm2,-0x280(%%rcx)	\n\t						addpd	%%xmm14,%%xmm14			\n\t"\
			"addpd	%%xmm0,%%xmm4			\n\t						mulpd	(%%r8),%%xmm15			\n\t"\
			"addpd	%%xmm6,%%xmm3			\n\t						mulpd	(%%r8),%%xmm8 			\n\t"\
			"addpd	%%xmm5,%%xmm4			\n\t						addpd	%%xmm13,%%xmm9 			\n\t"\
			"movaps	%%xmm3,-0x100(%%rcx)	\n\t						addpd	%%xmm14,%%xmm10			\n\t"\
			"movaps	%%xmm4,0x080(%%rcx)	\n\t							movaps	0x0a0(%%rdx),%%xmm13	\n\t"\
			"/********************************************/\n\t			movaps	0x0c0(%%rdx),%%xmm14	\n\t"\
			"/*          Imaginary Parts:                */\n\t			addpd	%%xmm15,%%xmm11			\n\t"\
			"/********************************************/\n\t			addpd	%%xmm8 ,%%xmm12			\n\t"\
			"addq	$0x10,%%rax				\n\t						subpd	%%xmm14,%%xmm13			\n\t"\
			"movaps	0x020(%%rax),%%xmm1	\n\t							movaps	%%xmm13,-0x200(%%rdi)	\n\t"\
			"movaps	0x140(%%rax),%%xmm5	\n\t							addpd	%%xmm14,%%xmm14			\n\t"\
			"movaps	0x040(%%rax),%%xmm2	\n\t							movaps	(%%rdx),%%xmm8 			\n\t"\
			"movaps	0x120(%%rax),%%xmm6	\n\t							addpd	%%xmm14,%%xmm13			\n\t"\
			"movaps	0x060(%%rax),%%xmm3	\n\t							addq	$0x10,%%rdi				\n\t"\
			"movaps	0x100(%%rax),%%xmm7	\n\t							subpd	%%xmm10,%%xmm9 			\n\t"\
			"movaps	0x080(%%rax),%%xmm4	\n\t							subpd	%%xmm10,%%xmm13			\n\t"\
			"movaps	0x0e0(%%rax),%%xmm0	\n\t							movaps	%%xmm9 ,%%xmm14			\n\t"\
			"subpd	%%xmm5,%%xmm1			\n\t						subpd	%%xmm10,%%xmm11			\n\t"\
			"subpd	%%xmm6,%%xmm2			\n\t						movaps	%%xmm9 ,%%xmm15			\n\t"\
			"subpd	%%xmm7,%%xmm3			\n\t						mulpd	     (%%rsi),%%xmm9 		\n\t"\
			"subpd	%%xmm0,%%xmm4			\n\t						movaps	%%xmm9 ,0x180(%%rdi)	\n\t"\
			"movaps	%%xmm1,-0x180(%%rcx)	\n\t						subpd	%%xmm10,%%xmm12			\n\t"\
			"movaps	%%xmm2,-0x300(%%rcx)	\n\t						mulpd	-0x10(%%rsi),%%xmm10		\n\t"\
			"movaps	%%xmm3,0x100(%%rcx)	\n\t							addpd	%%xmm13,%%xmm14			\n\t"\
			"movaps	%%xmm4,-0x080(%%rcx)	\n\t						subpd	%%xmm11,%%xmm15			\n\t"\
			"addpd	%%xmm5,%%xmm5			\n\t						mulpd	 0x60(%%rsi),%%xmm15		\n\t"\
			"addpd	%%xmm6,%%xmm6			\n\t						movaps	%%xmm15,-0x100(%%rdi)	\n\t"\
			"mulpd	(%%r8),%%xmm7			\n\t						movaps	%%xmm11,%%xmm9 			\n\t"\
			"mulpd	(%%r8),%%xmm0			\n\t						addpd	%%xmm12,%%xmm11			\n\t"\
			"addpd	%%xmm5,%%xmm1			\n\t						mulpd	 0x30(%%rsi),%%xmm9 		\n\t"\
			"addpd	%%xmm6,%%xmm2			\n\t						movaps	%%xmm9 ,-0x280(%%rdi)	\n\t"\
			"movaps	0x0a0(%%rax),%%xmm5	\n\t							movaps	%%xmm13,%%xmm15			\n\t"\
			"movaps	0x0c0(%%rax),%%xmm6	\n\t							subpd	%%xmm12,%%xmm13			\n\t"\
			"addpd	%%xmm7,%%xmm3			\n\t						mulpd	 0x40(%%rsi),%%xmm12		\n\t"\
			"addpd	%%xmm0,%%xmm4			\n\t						mulpd	 0x70(%%rsi),%%xmm13		\n\t"\
			"subpd	%%xmm6,%%xmm5			\n\t						mulpd	 0x10(%%rsi),%%xmm15		\n\t"\
			"movaps	%%xmm5,-0x200(%%rcx)	\n\t						addpd	%%xmm11,%%xmm10			\n\t"\
			"addpd	%%xmm6,%%xmm6			\n\t						movaps	%%xmm14,%%xmm9 			\n\t"\
			"movaps	(%%rax),%%xmm0			\n\t						subpd	%%xmm11,%%xmm14			\n\t"\
			"addpd	%%xmm6,%%xmm5			\n\t						mulpd	 0x80(%%rsi),%%xmm14		\n\t"\
			"addq	$0x10,%%rcx				\n\t						mulpd	 0x50(%%rsi),%%xmm11		\n\t"\
			"subpd	%%xmm2,%%xmm1			\n\t						addpd	%%xmm9 ,%%xmm10			\n\t"\
			"subpd	%%xmm2,%%xmm5			\n\t						mulpd	 0x20(%%rsi),%%xmm9 		\n\t"\
			"movaps	%%xmm1,%%xmm6			\n\t						addpd	%%xmm10,%%xmm8 			\n\t"\
			"subpd	%%xmm2,%%xmm3			\n\t						mulpd	 0x90(%%rsi),%%xmm10		\n\t"\
			"movaps	%%xmm1,%%xmm7			\n\t						movaps	%%xmm8 ,(%%rdi)			\n\t"\
			"mulpd	     (%%rbx),%%xmm1		\n\t						addpd	%%xmm8 ,%%xmm10			\n\t"\
			"movaps	%%xmm1,-0x400(%%rcx)	\n\t						addpd	%%xmm9 ,%%xmm15			\n\t"\
			"subpd	%%xmm2,%%xmm4			\n\t						addpd	0x180(%%rdi),%%xmm9 	\n\t"\
			"mulpd	-0x10(%%rbx),%%xmm2		\n\t						addpd	%%xmm11,%%xmm12			\n\t"\
			"addpd	%%xmm5,%%xmm6			\n\t						addpd	-0x280(%%rdi),%%xmm11	\n\t"\
			"subpd	%%xmm3,%%xmm7			\n\t						addpd	%%xmm14,%%xmm13			\n\t"\
			"mulpd	 0x60(%%rbx),%%xmm7		\n\t						addpd	-0x100(%%rdi),%%xmm14	\n\t"\
			"movaps	%%xmm7,-0x100(%%rcx)	\n\t						movaps	%%xmm10,%%xmm8 			\n\t"\
			"movaps	%%xmm3,%%xmm1			\n\t						subpd	%%xmm9 ,%%xmm10			\n\t"\
			"addpd	%%xmm4,%%xmm3			\n\t						addpd	%%xmm8 ,%%xmm9 			\n\t"\
			"mulpd	 0x30(%%rbx),%%xmm1		\n\t						subpd	%%xmm15,%%xmm10			\n\t"\
			"movaps	%%xmm1,-0x280(%%rcx)	\n\t						addpd	%%xmm8 ,%%xmm15			\n\t"\
			"movaps	%%xmm5,%%xmm7			\n\t						subpd	%%xmm14,%%xmm9 			\n\t"\
			"subpd	%%xmm4,%%xmm5			\n\t						subpd	%%xmm11,%%xmm10			\n\t"\
			"mulpd	 0x40(%%rbx),%%xmm4		\n\t						movaps	%%xmm9 ,0x180(%%rdi)	\n\t"\
			"mulpd	 0x70(%%rbx),%%xmm5		\n\t						addpd	%%xmm8 ,%%xmm11			\n\t"\
			"mulpd	 0x10(%%rbx),%%xmm7		\n\t						subpd	%%xmm13,%%xmm15			\n\t"\
			"addpd	%%xmm3,%%xmm2			\n\t						subpd	%%xmm12,%%xmm10			\n\t"\
			"movaps	%%xmm6,%%xmm1			\n\t						movaps	%%xmm15,0x200(%%rdi)	\n\t"\
			"subpd	%%xmm3,%%xmm6			\n\t						movaps	%%xmm10,-0x280(%%rdi)	\n\t"\
			"mulpd	 0x80(%%rbx),%%xmm6		\n\t						addpd	%%xmm8 ,%%xmm12			\n\t"\
			"mulpd	 0x50(%%rbx),%%xmm3		\n\t						addpd	%%xmm14,%%xmm11			\n\t"\
			"addpd	%%xmm1,%%xmm2			\n\t						addpd	%%xmm13,%%xmm12			\n\t"\
			"mulpd	 0x20(%%rbx),%%xmm1		\n\t						movaps	%%xmm11,-0x100(%%rdi)	\n\t"\
			"addpd	%%xmm2,%%xmm0			\n\t						movaps	%%xmm12,0x080(%%rdi)	\n\t"\
			"mulpd	 0x90(%%rbx),%%xmm2		\n\t						/********************************************/\n\t"\
			"movaps	%%xmm0,(%%rcx)			\n\t						/*        Here are the 5 sine terms:        */\n\t"\
			"addpd	%%xmm0,%%xmm2			\n\t						/********************************************/\n\t"\
			"addpd	%%xmm1,%%xmm7			\n\t						subq	$0x10,%%rdx				\n\t"\
			"addpd	-0x400(%%rcx),%%xmm1	\n\t						addq	$0xa0,%%rsi				\n\t"\
			"addpd	%%xmm3,%%xmm4			\n\t						/********************************************/\n\t"\
			"addpd	-0x280(%%rcx),%%xmm3	\n\t						/*               Real Parts:                */\n\t"\
			"addpd	%%xmm6,%%xmm5			\n\t						/********************************************/\n\t"\
			"addpd	-0x100(%%rcx),%%xmm6	\n\t						movaps	-0x180(%%rdi),%%xmm9 	\n\t"\
			"movaps	%%xmm2,%%xmm0			\n\t						movaps	0x280(%%rdi),%%xmm10	\n\t"\
			"subpd	%%xmm1,%%xmm2			\n\t						movaps	0x100(%%rdi),%%xmm11	\n\t"\
			"addpd	%%xmm0,%%xmm1			\n\t						movaps	-0x080(%%rdi),%%xmm12	\n\t"\
			"subpd	%%xmm7,%%xmm2			\n\t						movaps	-0x200(%%rdi),%%xmm13	\n\t"\
			"addpd	%%xmm0,%%xmm7			\n\t						addpd	%%xmm10,%%xmm9 			\n\t"\
			"subpd	%%xmm6,%%xmm1			\n\t						addpd	%%xmm10,%%xmm13			\n\t"\
			"subpd	%%xmm3,%%xmm2			\n\t						movaps	%%xmm9 ,%%xmm14			\n\t"\
			"movaps	%%xmm1,-0x400(%%rcx)	\n\t						addpd	%%xmm10,%%xmm11			\n\t"\
			"addpd	%%xmm0,%%xmm3			\n\t						movaps	%%xmm9 ,%%xmm15			\n\t"\
			"subpd	%%xmm5,%%xmm7			\n\t						mulpd	     (%%rsi),%%xmm9 		\n\t"\
			"subpd	%%xmm4,%%xmm2			\n\t						movaps	%%xmm9 ,-0x200(%%rdi)	\n\t"\
			"movaps	%%xmm7,-0x380(%%rcx)	\n\t						addpd	%%xmm10,%%xmm12			\n\t"\
			"movaps	%%xmm2,-0x280(%%rcx)	\n\t						mulpd	-0xb0(%%rsi),%%xmm10		\n\t"\
			"addpd	%%xmm0,%%xmm4			\n\t						addpd	%%xmm13,%%xmm14			\n\t"\
			"addpd	%%xmm6,%%xmm3			\n\t						subpd	%%xmm11,%%xmm15			\n\t"\
			"addpd	%%xmm5,%%xmm4			\n\t						mulpd	 0x60(%%rsi),%%xmm15		\n\t"\
			"movaps	%%xmm3,-0x100(%%rcx)	\n\t						movaps	%%xmm15,0x100(%%rdi)	\n\t"\
			"movaps	%%xmm4,0x080(%%rcx)	\n\t							movaps	%%xmm11,%%xmm9 			\n\t"\
			"/********************************************/\n\t			addpd	%%xmm12,%%xmm11			\n\t"\
			"/*        Here are the 5 sine terms:        */\n\t			mulpd	 0x30(%%rsi),%%xmm9 		\n\t"\
			"/********************************************/\n\t			movaps	%%xmm9 ,-0x080(%%rdi)	\n\t"\
			"subq	$0x10,%%rax				\n\t						movaps	%%xmm13,%%xmm15			\n\t"\
			"addq	$0xa0,%%rbx				\n\t						subpd	%%xmm12,%%xmm13			\n\t"\
			"/********************************************/\n\t			mulpd	 0x40(%%rsi),%%xmm12		\n\t"\
			"/*               Real Parts:                */\n\t			mulpd	 0x70(%%rsi),%%xmm13		\n\t"\
			"/********************************************/\n\t			mulpd	 0x10(%%rsi),%%xmm15		\n\t"\
			"movaps	-0x180(%%rcx),%%xmm1	\n\t						subpd	%%xmm11,%%xmm10			\n\t"\
			"movaps	-0x300(%%rcx),%%xmm2	\n\t						movaps	%%xmm14,%%xmm9 			\n\t"\
			"movaps	0x100(%%rcx),%%xmm3	\n\t							subpd	%%xmm11,%%xmm14			\n\t"\
			"movaps	-0x080(%%rcx),%%xmm4	\n\t						mulpd	 0x80(%%rsi),%%xmm14		\n\t"\
			"movaps	-0x200(%%rcx),%%xmm5	\n\t						mulpd	 0x50(%%rsi),%%xmm11		\n\t"\
			"addpd	%%xmm2,%%xmm1			\n\t						subpd	%%xmm9 ,%%xmm10			\n\t"\
			"addpd	%%xmm2,%%xmm5			\n\t						mulpd	 0x20(%%rsi),%%xmm9 		\n\t"\
			"movaps	%%xmm1,%%xmm6			\n\t						mulpd	 0x90(%%rsi),%%xmm10		\n\t"\
			"addpd	%%xmm2,%%xmm3			\n\t						addpd	%%xmm9 ,%%xmm15			\n\t"\
			"movaps	%%xmm1,%%xmm7			\n\t						addpd	-0x200(%%rdi),%%xmm9 	\n\t"\
			"mulpd	     (%%rbx),%%xmm1		\n\t						addpd	%%xmm11,%%xmm12			\n\t"\
			"movaps	%%xmm1,-0x200(%%rcx)	\n\t						addpd	-0x080(%%rdi),%%xmm11	\n\t"\
			"addpd	%%xmm2,%%xmm4			\n\t						addpd	%%xmm14,%%xmm13			\n\t"\
			"mulpd	-0xb0(%%rbx),%%xmm2		\n\t						addpd	0x100(%%rdi),%%xmm14	\n\t"\
			"addpd	%%xmm5,%%xmm6			\n\t						xorpd	%%xmm8 ,%%xmm8 			\n\t"\
			"subpd	%%xmm3,%%xmm7			\n\t						subpd	%%xmm10,%%xmm8 			\n\t"\
			"mulpd	 0x60(%%rbx),%%xmm7		\n\t						addpd	%%xmm9 ,%%xmm8 			\n\t"\
			"movaps	%%xmm7,0x100(%%rcx)	\n\t							addpd	%%xmm10,%%xmm9 			\n\t"\
			"movaps	%%xmm3,%%xmm1			\n\t						addpd	%%xmm15,%%xmm8 			\n\t"\
			"addpd	%%xmm4,%%xmm3			\n\t						addpd	%%xmm10,%%xmm15			\n\t"\
			"mulpd	 0x30(%%rbx),%%xmm1		\n\t						subpd	%%xmm14,%%xmm9 			\n\t"\
			"movaps	%%xmm1,-0x080(%%rcx)	\n\t						addpd	%%xmm11,%%xmm8 			\n\t"\
			"movaps	%%xmm5,%%xmm7			\n\t						addpd	%%xmm10,%%xmm11			\n\t"\
			"subpd	%%xmm4,%%xmm5			\n\t						subpd	%%xmm13,%%xmm15			\n\t"\
			"mulpd	 0x40(%%rbx),%%xmm4		\n\t						addpd	%%xmm12,%%xmm8 			\n\t"\
			"mulpd	 0x70(%%rbx),%%xmm5		\n\t						addpd	%%xmm10,%%xmm12			\n\t"\
			"mulpd	 0x10(%%rbx),%%xmm7		\n\t						addpd	%%xmm14,%%xmm11			\n\t"\
			"subpd	%%xmm3,%%xmm2			\n\t						addpd	%%xmm13,%%xmm12			\n\t"\
			"movaps	%%xmm6,%%xmm1			\n\t						movaps	%%xmm9 ,%%xmm10			\n\t"\
			"subpd	%%xmm3,%%xmm6			\n\t						addpd	0x180(%%rdi),%%xmm9 	\n\t"\
			"mulpd	 0x80(%%rbx),%%xmm6		\n\t						addpd	%%xmm10,%%xmm10			\n\t"\
			"mulpd	 0x50(%%rbx),%%xmm3		\n\t						movaps	%%xmm9 ,0x180(%%rdi)	\n\t"\
			"subpd	%%xmm1,%%xmm2			\n\t						subpd	%%xmm10,%%xmm9 			\n\t"\
			"mulpd	 0x20(%%rbx),%%xmm1		\n\t						movaps	%%xmm9 ,-0x180(%%rdi)	\n\t"\
			"mulpd	 0x90(%%rbx),%%xmm2		\n\t						movaps	%%xmm8 ,%%xmm13			\n\t"\
			"addpd	%%xmm1,%%xmm7			\n\t						addpd	-0x280(%%rdi),%%xmm8 	\n\t"\
			"addpd	-0x200(%%rcx),%%xmm1	\n\t						mulpd	(%%r8),%%xmm13			\n\t"\
			"addpd	%%xmm3,%%xmm4			\n\t						movaps	%%xmm8 ,-0x280(%%rdi)	\n\t"\
			"addpd	-0x080(%%rcx),%%xmm3	\n\t						subpd	%%xmm13,%%xmm8 			\n\t"\
			"addpd	%%xmm6,%%xmm5			\n\t						movaps	%%xmm8 ,0x280(%%rdi)	\n\t"\
			"addpd	0x100(%%rcx),%%xmm6	\n\t							movaps	%%xmm11,%%xmm14			\n\t"\
			"xorpd	%%xmm0,%%xmm0			\n\t						addpd	-0x100(%%rdi),%%xmm11	\n\t"\
			"subpd	%%xmm2,%%xmm0			\n\t						mulpd	(%%r8),%%xmm14			\n\t"\
			"addpd	%%xmm1,%%xmm0			\n\t						movaps	%%xmm11,-0x100(%%rdi)	\n\t"\
			"addpd	%%xmm2,%%xmm1			\n\t						subpd	%%xmm14,%%xmm11			\n\t"\
			"addpd	%%xmm7,%%xmm0			\n\t						movaps	%%xmm11,0x100(%%rdi)	\n\t"\
			"addpd	%%xmm2,%%xmm7			\n\t						movaps	%%xmm12,%%xmm10			\n\t"\
			"subpd	%%xmm6,%%xmm1			\n\t						addpd	0x080(%%rdi),%%xmm12	\n\t"\
			"addpd	%%xmm3,%%xmm0			\n\t						mulpd	(%%r8),%%xmm10			\n\t"\
			"addpd	%%xmm2,%%xmm3			\n\t						movaps	%%xmm12,0x080(%%rdi)	\n\t"\
			"subpd	%%xmm5,%%xmm7			\n\t						subpd	%%xmm10,%%xmm12			\n\t"\
			"addpd	%%xmm4,%%xmm0			\n\t						movaps	%%xmm12,-0x080(%%rdi)	\n\t"\
			"addpd	%%xmm2,%%xmm4			\n\t						movaps	%%xmm15,%%xmm13			\n\t"\
			"addpd	%%xmm6,%%xmm3			\n\t						addpd	0x200(%%rdi),%%xmm15	\n\t"\
			"addpd	%%xmm5,%%xmm4			\n\t						mulpd	(%%r8),%%xmm13			\n\t"\
			"movaps	%%xmm1,%%xmm2			\n\t						movaps	%%xmm15,0x200(%%rdi)	\n\t"\
			"addpd	-0x400(%%rcx),%%xmm1	\n\t						subpd	%%xmm13,%%xmm15			\n\t"\
			"mulpd	(%%r8),%%xmm2			\n\t						movaps	%%xmm15,-0x200(%%rdi)	\n\t"\
			"movaps	%%xmm1,-0x400(%%rcx)	\n\t						/********************************************/\n\t"\
			"subpd	%%xmm2,%%xmm1			\n\t						/*          Imaginary Parts:                */\n\t"\
			"movaps	%%xmm1,-0x180(%%rcx)	\n\t						/********************************************/\n\t"\
			"movaps	%%xmm0,%%xmm5			\n\t						subq	$0x10,%%rdi				\n\t"\
			"addpd	-0x280(%%rcx),%%xmm0	\n\t						movaps	-0x180(%%rdi),%%xmm9 	\n\t"\
			"mulpd	(%%r8),%%xmm5			\n\t						movaps	0x280(%%rdi),%%xmm10	\n\t"\
			"movaps	%%xmm0,-0x280(%%rcx)	\n\t						movaps	0x100(%%rdi),%%xmm11	\n\t"\
			"subpd	%%xmm5,%%xmm0			\n\t						movaps	-0x080(%%rdi),%%xmm12	\n\t"\
			"movaps	%%xmm0,-0x300(%%rcx)	\n\t						movaps	-0x200(%%rdi),%%xmm13	\n\t"\
			"movaps	%%xmm3,%%xmm6			\n\t						addpd	%%xmm10,%%xmm9 			\n\t"\
			"addpd	-0x100(%%rcx),%%xmm3	\n\t						addpd	%%xmm10,%%xmm13			\n\t"\
			"mulpd	(%%r8),%%xmm6			\n\t						movaps	%%xmm9 ,%%xmm14			\n\t"\
			"movaps	%%xmm3,-0x100(%%rcx)	\n\t						addpd	%%xmm10,%%xmm11			\n\t"\
			"subpd	%%xmm6,%%xmm3			\n\t						movaps	%%xmm9 ,%%xmm15			\n\t"\
			"movaps	%%xmm3,0x100(%%rcx)	\n\t							mulpd	     (%%rsi),%%xmm9 		\n\t"\
			"movaps	%%xmm4,%%xmm2			\n\t						movaps	%%xmm9 ,-0x200(%%rdi)	\n\t"\
			"addpd	0x080(%%rcx),%%xmm4	\n\t							addpd	%%xmm10,%%xmm12			\n\t"\
			"addpd	%%xmm2,%%xmm2			\n\t						mulpd	-0xb0(%%rsi),%%xmm10		\n\t"\
			"movaps	%%xmm4,0x080(%%rcx)	\n\t							addpd	%%xmm13,%%xmm14			\n\t"\
			"subpd	%%xmm2,%%xmm4			\n\t						subpd	%%xmm11,%%xmm15			\n\t"\
			"movaps	%%xmm4,-0x080(%%rcx)	\n\t						mulpd	 0x60(%%rsi),%%xmm15		\n\t"\
			"movaps	%%xmm7,%%xmm5			\n\t						movaps	%%xmm15,0x100(%%rdi)	\n\t"\
			"addpd	-0x380(%%rcx),%%xmm7	\n\t						movaps	%%xmm11,%%xmm9 			\n\t"\
			"mulpd	(%%r8),%%xmm5			\n\t						addpd	%%xmm12,%%xmm11			\n\t"\
			"movaps	%%xmm7,-0x380(%%rcx)	\n\t						mulpd	 0x30(%%rsi),%%xmm9 		\n\t"\
			"subpd	%%xmm5,%%xmm7			\n\t						movaps	%%xmm9 ,-0x080(%%rdi)	\n\t"\
			"movaps	%%xmm7,-0x200(%%rcx)	\n\t						movaps	%%xmm13,%%xmm15			\n\t"\
			"/********************************************/\n\t			subpd	%%xmm12,%%xmm13			\n\t"\
			"/*          Imaginary Parts:                */\n\t			mulpd	 0x40(%%rsi),%%xmm12		\n\t"\
			"/********************************************/\n\t			mulpd	 0x70(%%rsi),%%xmm13		\n\t"\
			"subq	$0x10,%%rcx				\n\t						mulpd	 0x10(%%rsi),%%xmm15		\n\t"\
			"movaps	-0x180(%%rcx),%%xmm1	\n\t						subpd	%%xmm11,%%xmm10			\n\t"\
			"movaps	-0x300(%%rcx),%%xmm2	\n\t						movaps	%%xmm14,%%xmm9 			\n\t"\
			"movaps	0x100(%%rcx),%%xmm3	\n\t							subpd	%%xmm11,%%xmm14			\n\t"\
			"movaps	-0x080(%%rcx),%%xmm4	\n\t						mulpd	 0x80(%%rsi),%%xmm14		\n\t"\
			"movaps	-0x200(%%rcx),%%xmm5	\n\t						mulpd	 0x50(%%rsi),%%xmm11		\n\t"\
			"addpd	%%xmm2,%%xmm1			\n\t						subpd	%%xmm9 ,%%xmm10			\n\t"\
			"addpd	%%xmm2,%%xmm5			\n\t						mulpd	 0x20(%%rsi),%%xmm9 		\n\t"\
			"movaps	%%xmm1,%%xmm6			\n\t						mulpd	 0x90(%%rsi),%%xmm10		\n\t"\
			"addpd	%%xmm2,%%xmm3			\n\t						addpd	%%xmm9 ,%%xmm15			\n\t"\
			"movaps	%%xmm1,%%xmm7			\n\t						addpd	-0x200(%%rdi),%%xmm9 	\n\t"\
			"mulpd	     (%%rbx),%%xmm1		\n\t						addpd	%%xmm11,%%xmm12			\n\t"\
			"movaps	%%xmm1,-0x200(%%rcx)	\n\t						addpd	-0x080(%%rdi),%%xmm11	\n\t"\
			"addpd	%%xmm2,%%xmm4			\n\t						addpd	%%xmm14,%%xmm13			\n\t"\
			"mulpd	-0xb0(%%rbx),%%xmm2		\n\t						addpd	0x100(%%rdi),%%xmm14	\n\t"\
			"addpd	%%xmm5,%%xmm6			\n\t						xorpd	%%xmm8 ,%%xmm8 			\n\t"\
			"subpd	%%xmm3,%%xmm7			\n\t						subpd	%%xmm10,%%xmm8 			\n\t"\
			"mulpd	 0x60(%%rbx),%%xmm7		\n\t						addpd	%%xmm9 ,%%xmm8 			\n\t"\
			"movaps	%%xmm7,0x100(%%rcx)	\n\t							addpd	%%xmm10,%%xmm9 			\n\t"\
			"movaps	%%xmm3,%%xmm1			\n\t						addpd	%%xmm15,%%xmm8 			\n\t"\
			"addpd	%%xmm4,%%xmm3			\n\t						addpd	%%xmm10,%%xmm15			\n\t"\
			"mulpd	 0x30(%%rbx),%%xmm1		\n\t						subpd	%%xmm14,%%xmm9 			\n\t"\
			"movaps	%%xmm1,-0x080(%%rcx)	\n\t						addpd	%%xmm11,%%xmm8 			\n\t"\
			"movaps	%%xmm5,%%xmm7			\n\t						addpd	%%xmm10,%%xmm11			\n\t"\
			"subpd	%%xmm4,%%xmm5			\n\t						subpd	%%xmm13,%%xmm15			\n\t"\
			"mulpd	 0x40(%%rbx),%%xmm4		\n\t						addpd	%%xmm12,%%xmm8 			\n\t"\
			"mulpd	 0x70(%%rbx),%%xmm5		\n\t						addpd	%%xmm10,%%xmm12			\n\t"\
			"mulpd	 0x10(%%rbx),%%xmm7		\n\t						addpd	%%xmm14,%%xmm11			\n\t"\
			"subpd	%%xmm3,%%xmm2			\n\t						addpd	%%xmm13,%%xmm12			\n\t"\
			"movaps	%%xmm6,%%xmm1			\n\t						movaps	%%xmm9 ,%%xmm10			\n\t"\
			"subpd	%%xmm3,%%xmm6			\n\t						addpd	0x180(%%rdi),%%xmm9 	\n\t"\
			"mulpd	 0x80(%%rbx),%%xmm6		\n\t						addpd	%%xmm10,%%xmm10			\n\t"\
			"mulpd	 0x50(%%rbx),%%xmm3		\n\t						movaps	%%xmm9 ,-0x180(%%rdi)	\n\t"\
			"subpd	%%xmm1,%%xmm2			\n\t						subpd	%%xmm10,%%xmm9 			\n\t"\
			"mulpd	 0x20(%%rbx),%%xmm1		\n\t						movaps	%%xmm9 ,0x180(%%rdi)	\n\t"\
			"mulpd	 0x90(%%rbx),%%xmm2		\n\t						movaps	%%xmm8 ,%%xmm13			\n\t"\
			"addpd	%%xmm1,%%xmm7			\n\t						addpd	-0x280(%%rdi),%%xmm8 	\n\t"\
			"addpd	-0x200(%%rcx),%%xmm1	\n\t						mulpd	(%%r8),%%xmm13			\n\t"\
			"addpd	%%xmm3,%%xmm4			\n\t						movaps	%%xmm8 ,0x280(%%rdi)	\n\t"\
			"addpd	-0x080(%%rcx),%%xmm3	\n\t						subpd	%%xmm13,%%xmm8 			\n\t"\
			"addpd	%%xmm6,%%xmm5			\n\t						movaps	%%xmm8 ,-0x280(%%rdi)	\n\t"\
			"addpd	0x100(%%rcx),%%xmm6	\n\t							movaps	%%xmm11,%%xmm14			\n\t"\
			"xorpd	%%xmm0,%%xmm0			\n\t						addpd	-0x100(%%rdi),%%xmm11	\n\t"\
			"subpd	%%xmm2,%%xmm0			\n\t						mulpd	(%%r8),%%xmm14			\n\t"\
			"addpd	%%xmm1,%%xmm0			\n\t						movaps	%%xmm11,0x100(%%rdi)	\n\t"\
			"addpd	%%xmm2,%%xmm1			\n\t						subpd	%%xmm14,%%xmm11			\n\t"\
			"addpd	%%xmm7,%%xmm0			\n\t						movaps	%%xmm11,-0x100(%%rdi)	\n\t"\
			"addpd	%%xmm2,%%xmm7			\n\t						movaps	%%xmm12,%%xmm10			\n\t"\
			"subpd	%%xmm6,%%xmm1			\n\t						addpd	0x080(%%rdi),%%xmm12	\n\t"\
			"addpd	%%xmm3,%%xmm0			\n\t						mulpd	(%%r8),%%xmm10			\n\t"\
			"addpd	%%xmm2,%%xmm3			\n\t						movaps	%%xmm12,-0x080(%%rdi)	\n\t"\
			"subpd	%%xmm5,%%xmm7			\n\t						subpd	%%xmm10,%%xmm12			\n\t"\
			"addpd	%%xmm4,%%xmm0			\n\t						movaps	%%xmm12,0x080(%%rdi)	\n\t"\
			"addpd	%%xmm2,%%xmm4			\n\t						movaps	%%xmm15,%%xmm13			\n\t"\
			"addpd	%%xmm6,%%xmm3			\n\t						addpd	0x200(%%rdi),%%xmm15	\n\t"\
			"addpd	%%xmm5,%%xmm4			\n\t						mulpd	(%%r8),%%xmm13			\n\t"\
			"movaps	%%xmm1,%%xmm2			\n\t						movaps	%%xmm15,-0x200(%%rdi)	\n\t"\
			"addpd	-0x400(%%rcx),%%xmm1	\n\t						subpd	%%xmm13,%%xmm15			\n\t"\
			"mulpd	(%%r8),%%xmm2			\n\t						movaps	%%xmm15,0x200(%%rdi)	\n\t"\
			"movaps	%%xmm1,-0x180(%%rcx)	\n\t"\
			"subpd	%%xmm2,%%xmm1			\n\t"\
			"movaps	%%xmm1,-0x400(%%rcx)	\n\t"\
			"movaps	%%xmm0,%%xmm5			\n\t"\
			"addpd	-0x280(%%rcx),%%xmm0	\n\t"\
			"mulpd	(%%r8),%%xmm5			\n\t"\
			"movaps	%%xmm0,-0x300(%%rcx)	\n\t"\
			"subpd	%%xmm5,%%xmm0			\n\t"\
			"movaps	%%xmm0,-0x280(%%rcx)	\n\t"\
			"movaps	%%xmm3,%%xmm6			\n\t"\
			"addpd	-0x100(%%rcx),%%xmm3	\n\t"\
			"mulpd	(%%r8),%%xmm6			\n\t"\
			"movaps	%%xmm3,0x100(%%rcx)	\n\t"\
			"subpd	%%xmm6,%%xmm3			\n\t"\
			"movaps	%%xmm3,-0x100(%%rcx)	\n\t"\
			"movaps	%%xmm4,%%xmm2			\n\t"\
			"addpd	0x080(%%rcx),%%xmm4	\n\t"\
			"mulpd	(%%r8),%%xmm2			\n\t"\
			"movaps	%%xmm4,-0x080(%%rcx)	\n\t"\
			"subpd	%%xmm2,%%xmm4			\n\t"\
			"movaps	%%xmm4,0x080(%%rcx)	\n\t"\
			"movaps	%%xmm7,%%xmm5			\n\t"\
			"addpd	-0x380(%%rcx),%%xmm7	\n\t"\
			"mulpd	(%%r8),%%xmm5			\n\t"\
			"movaps	%%xmm7,-0x200(%%rcx)	\n\t"\
			"subpd	%%xmm5,%%xmm7			\n\t"\
			"movaps	%%xmm7,-0x380(%%rcx)	\n\t"\
			"\n\t"\
			:					/* outputs: none */\
			: [__add] "m" (Xadd)	/* All inputs from memory addresses here */\
			 ,[__p01] "m" (Xp01)\
			 ,[__p02] "m" (Xp02)\
			 ,[__p03] "m" (Xp03)\
			 ,[__p04] "m" (Xp04)\
			 ,[__t00r] "m" (Xt00r)\
			 ,[__cc] "m" (Xcc)\
			 ,[__out] "m" (Xout)\
			: "cc","memory","rax","rbx","rcx","rdx","rdi","rsi","r8","xmm0","xmm1","xmm2","xmm3","xmm4","xmm5","xmm6","xmm7","xmm8","xmm9","xmm10","xmm11","xmm12","xmm13","xmm14","xmm15"		/* Clobbered registers */\
		);\
	}

#endif	/* radix44_ditN_cy_dif1_gcc_h_included */
