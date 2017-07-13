/*******************************************************************************
*                                                                              *
*   (C) 1997-2017 by Ernst W. Mayer.                                           *
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

// This main loop is same for un-and-multithreaded, so stick into a header file
// (can't use a macro because of the #if-enclosed stuff).

for(k=1; k <= khi; k++)	/* Do n/(radix(1)*nwt) outer loop executions...	*/
{
	/* In SIMD mode, data are arranged in [re_0,...,re_n-1,im_0,...,im_n-1] groups, not the usual [re_0,im_0],...,[re_n-1,im_n-1] pairs.
	Thus we can still increment the j-index as if stepping through the residue array-of-doubles in strides of 2,
	but to point to the proper real datum, we need to index-map e.g. [0,1,2,3] ==> [0,2,1,3] in 2-way SIMD mode.
	(But only ever need to explicitly do this in debug mode).
	*/
	for(j = jstart; j < jhi; j += stride)
	{
		j1 =  j;
		j1 = j1 + ( (j1 >> DAT_BITS) << PAD_BITS );	/* padded-array fetch index is here */
		j2 = j1 + RE_IM_STRIDE;

	/*...The radix-64 DIT pass is here:	*/

#ifdef USE_SSE2

  #if USE_SCALAR_DFT_MACRO

	SSE2_RADIX_64_DIT( FALSE, thr_id,
		(a+j1),dft_offsets,
		// Intermediates base pointer:
		r00,
		// Output pointer: Base ptr of 8 pairs of vec_dbl local-mem:
		s1p00, c_offsets
	);

  #else

	/* NOTE: THE SIMD MACRO HERE RETURNS OUTPUTS 1-7 INDEX-REVERSED W.R.TO THE SCALAR ANALOG -- THIS REVERSAL IS
	REFLECTED IN THE INPUT-ARGS TO THE ENSUING DFTS-WITH-TWIDDLES, I.E. INSTEAD OF THOSE BEING ORDERED 0,4,2,6,1,5,3,7,
	WE HAVE INPUT ORDERING 0,4,6,2,7,3,5,1: */
	tmp = r00;
	for(l = 0; l < 8; l++) {
		add0 = &a[j1] + poff[l+l];	// poff[2*l] = p00,08,...,38
		add1 = add0+p01; add2 = add0+p02; add3 = add0+p03; add4 = add0+p04; add5 = add0+p05; add6 = add0+p06; add7 = add0+p07;
	  #ifdef USE_AVX2
		SSE2_RADIX8_DIT_0TWIDDLE(add0,add1,add2,add3,add4,add5,add6,add7, tmp, isrt2,two)
	  #else
		SSE2_RADIX8_DIT_0TWIDDLE(add0,add1,add2,add3,add4,add5,add6,add7, tmp, isrt2)
	  #endif
		tmp += 16;
	}

	/*...and now do eight radix-8 subtransforms w/internal twiddles - cf. radix64_dit_pass1 for details.
	Note: 1st of the 15 sincos args in each call to SSE2_RADIX8_DIT_TWIDDLE_OOP is the basic isrt2 needed for
	radix-8. This is a workaround of GCC's 30-arg limit for inline ASM macros, which proves a royal pain here.
	...and another kludge for the 30-arg limit: put a copy of (vec_dbl)2.0 into the first of each s1p*r outputs:
	*/
	VEC_DBL_INIT(s1p01,2.0);
	VEC_DBL_INIT(s1p02,2.0);
	VEC_DBL_INIT(s1p03,2.0);
	VEC_DBL_INIT(s1p04,2.0);
	VEC_DBL_INIT(s1p05,2.0);
	VEC_DBL_INIT(s1p06,2.0);
	VEC_DBL_INIT(s1p07,2.0);

	/* NOTE: THE ABOVE 0-TWIDDLES SIMD MACRO HERE RETURNS OUTPUTS INDEX-REVERSED W.R.TO THE SCALAR ANALOG -- THIS REVERSAL IS
	REFLECTED IN THE INPUT-ARGS TO THE ENSUING DFTS-WITH-TWIDDLES, I.E. INSTEAD OF THOSE BEING ORDERED 0,4,2,6,1,5,3,7,
	WE HAVE INPUT ORDERING 0,4,6,2,7,3,5,1:
	*/
	// Block 0: jt = j1; jp = j2, All unity twiddles:
  #ifdef USE_AVX2
	SSE2_RADIX8_DIT_0TWIDDLE_OOP(	// This outputs o[07654321], so reverse o-index order of latter 7 outputs
		r00,r10,r20,r30,r40,r50,r60,r70,
		s1p00,s1p38,s1p30,s1p28,s1p20,s1p18,s1p10,s1p08, isrt2,two
	);
  #else
	SSE2_RADIX8_DIT_0TWIDDLE_OOP(	// This outputs o[07654321], so reverse o-index order of latter 7 outputs
		r00,r10,r20,r30,r40,r50,r60,r70,
		s1p00,s1p38,s1p30,s1p28,s1p20,s1p18,s1p10,s1p08, isrt2
	);
  #endif
	// The with-twiddles macros have distinct impls for AVX and AVX2, but same 'prototype'
	// (No room for added 'two' arg since we are already maxed out at 30 args, and feed the 2.0 via the above kludge):
	// Block 4: jt = j1 + p04;	jp = j2 + p04;
	SSE2_RADIX8_DIT_TWIDDLE_OOP(
		r08,r18,r28,r38,r48,r58,r68,r78,
		s1p04,s1p24,s1p14,s1p34,s1p0c,s1p2c,s1p1c,s1p3c,
		/* c1,s1	c2,s2		c3,s3		c4,s4		c5,s5	c6,s6	c7,s7 (in terms of roots-naming in this macro) */
		ss0,cc0, isrt2,isrt2, nisrt2,isrt2, cc4,ss4, nss4,cc4, ss4,cc4, ncc4,ss4
	);
	// Block 2: jt = j1 + p02;	jp = j2 + p02;
	SSE2_RADIX8_DIT_TWIDDLE_OOP(
		r0C,r1C,r2C,r3C,r4C,r5C,r6C,r7C,	// 2/6 swap ==> r*4 / r*C swap
		s1p02,s1p22,s1p12,s1p32,s1p0a,s1p2a,s1p1a,s1p3a,
		isrt2,isrt2, cc4,ss4, ss4,cc4, cc2,ss2, ss6,cc6, cc6,ss6, ss2,cc2
	);
	// Block 6: jt = j1 + p06;	jp = j2 + p06;
	SSE2_RADIX8_DIT_TWIDDLE_OOP(
		r04,r14,r24,r34,r44,r54,r64,r74,	// 2/6 swap ==> r*4 / r*C swap
		s1p06,s1p26,s1p16,s1p36,s1p0e,s1p2e,s1p1e,s1p3e,
		nisrt2,isrt2, ss4,cc4, ncc4,nss4, cc6,ss6, ncc2,ss2, nss2,cc2, nss6,ncc6
	);
	// Block 1: jt = j1 + p01;	jp = j2 + p01;
	SSE2_RADIX8_DIT_TWIDDLE_OOP(
		r0E,r1E,r2E,r3E,r4E,r5E,r6E,r7E,	// 1/7 swap ==> r*2 / r*E swap
		s1p01,s1p21,s1p11,s1p31,s1p09,s1p29,s1p19,s1p39,
		cc4,ss4, cc2,ss2, cc6,ss6, cc1,ss1, cc5,ss5, cc3,ss3, cc7,ss7
	);
	// Block 5: jt = j1 + p05;	jp = j2 + p05;
	SSE2_RADIX8_DIT_TWIDDLE_OOP(
		r06,r16,r26,r36,r46,r56,r66,r76,
		s1p05,s1p25,s1p15,s1p35,s1p0d,s1p2d,s1p1d,s1p3d,
		nss4,cc4, ss6,cc6, ncc2,ss2, cc5,ss5, ncc7,ss7, ss1,cc1, ncc3,nss3
	);
	// Block 3: jt = j1 + p03;	jp = j2 + p03;
	SSE2_RADIX8_DIT_TWIDDLE_OOP(
		r0A,r1A,r2A,r3A,r4A,r5A,r6A,r7A,	// 3/5 swap ==> r*6 / r*A swap
		s1p03,s1p23,s1p13,s1p33,s1p0b,s1p2b,s1p1b,s1p3b,
		ss4,cc4, cc6,ss6, nss2,cc2, cc3,ss3, ss1,cc1, ss7,cc7, nss5,cc5
	);
	// Block 7: jt = j1 + p07;	jp = j2 + p07;
	SSE2_RADIX8_DIT_TWIDDLE_OOP(
		r02,r12,r22,r32,r42,r52,r62,r72,	// 1/7 swap ==> r*2 / r*E swap
		s1p07,s1p27,s1p17,s1p37,s1p0f,s1p2f,s1p1f,s1p3f,
		ncc4,ss4, ss2,cc2, nss6,ncc6, cc7,ss7, ncc3,nss3, nss5,cc5, ss1,ncc1
	);

  #endif

#else	// USE_SSE2 = False, or non-64-bit-GCC:

/* Gather the needed data (64 64-bit complex, i.e. 128 64-bit reals) and do 8 twiddleless length-8 subtransforms: */

	// Gather the needed data and do 8 twiddleless length-8 subtransforms, with p-offsets in-order:
	tptr = t;
	for(l = 0; l < 8; l++) {
		k2 = poff[l+l];	// poffs[2*l] = p00,08,10,...,30,38
		jt = j1 + k2; jp = j2 + k2;
		RADIX_08_DIT_OOP(
			a[jt],a[jp],a[jt+p01],a[jp+p01],a[jt+p02],a[jp+p02],a[jt+p03],a[jp+p03],a[jt+p04],a[jp+p04],a[jt+p05],a[jp+p05],a[jt+p06],a[jp+p06],a[jt+p07],a[jp+p07],
			tptr->re,tptr->im,(tptr+1)->re,(tptr+1)->im,(tptr+2)->re,(tptr+2)->im,(tptr+3)->re,(tptr+3)->im,(tptr+4)->re,(tptr+4)->im,(tptr+5)->re,(tptr+5)->im,(tptr+6)->re,(tptr+6)->im,(tptr+7)->re,(tptr+7)->im
		);
		tptr += 8;
	}

/*...and now do eight radix-8 subtransforms w/internal twiddles - cf. radix64_dit_pass1 for details: */

	/* Block 0: */ tptr = t;	jt = j1;	jp = j2;
	/* 0-index block has all-unity twiddles: Remember, the twiddleless DIT also bit-reverses its outputs, so a_p* terms appear in-order here: */
	RADIX_08_DIT_OOP(
		tptr->re,tptr->im,(tptr+0x08)->re,(tptr+0x08)->im,(tptr+0x10)->re,(tptr+0x10)->im,(tptr+0x18)->re,(tptr+0x18)->im,(tptr+0x20)->re,(tptr+0x20)->im,(tptr+0x28)->re,(tptr+0x28)->im,(tptr+0x30)->re,(tptr+0x30)->im,(tptr+0x38)->re,(tptr+0x38)->im,
		a[jt],a[jp],a[jt+p08],a[jp+p08],a[jt+p10],a[jp+p10],a[jt+p18],a[jp+p18],a[jt+p20],a[jp+p20],a[jt+p28],a[jp+p28],a[jt+p30],a[jp+p30],a[jt+p38],a[jp+p38]
	);

	// Remaining 7 sets of macro calls done in loop:
	for(l = 1; l < 8; l++) {
		tptr = t + reverse(l,8);
		k2 = po_br[l];	// po_br[] = p[04261537]
		jt = j1 + k2; jp = j2 + k2;
		addr = DFT64_TWIDDLES[l-1]; addi = addr+1;	// Pointer to required row of 2-D twiddles array
		RADIX_08_DIT_TWIDDLE_OOP(
			tptr->re,tptr->im,(tptr+0x08)->re,(tptr+0x08)->im,(tptr+0x10)->re,(tptr+0x10)->im,(tptr+0x18)->re,(tptr+0x18)->im,(tptr+0x20)->re,(tptr+0x20)->im,(tptr+0x28)->re,(tptr+0x28)->im,(tptr+0x30)->re,(tptr+0x30)->im,(tptr+0x38)->re,(tptr+0x38)->im,
			a[jt],a[jp],a[jt+p20],a[jp+p20],a[jt+p10],a[jp+p10],a[jt+p30],a[jp+p30],a[jt+p08],a[jp+p08],a[jt+p28],a[jp+p28],a[jt+p18],a[jp+p18],a[jt+p38],a[jp+p38],
			*(addr+0x00),*(addi+0x00), *(addr+0x02),*(addi+0x02), *(addr+0x04),*(addi+0x04), *(addr+0x06),*(addi+0x06), *(addr+0x08),*(addi+0x08), *(addr+0x0a),*(addi+0x0a), *(addr+0x0c),*(addi+0x0c)
		);
	}

#endif	// USE_SSE2 ?

/*...Now do the carries. Since the outputs would
normally be getting dispatched to [radix] separate blocks of the A-array, we need [radix] separate carries.	*/

/************ See the radix16_ditN_cy_dif1 routine for details on how the SSE2 carry stuff works **********/
	if(MODULUS_TYPE == MODULUS_TYPE_MERSENNE)
	{
	#ifdef USE_AVX
		// For AVX512-and-beyond we support only the fast Mers-carry macros.
		add1 = &wt1[col  ];
		add2 = &wt1[co2-1];
		add3 = &wt1[co3-1];
		/* ptr to local storage for the doubled wtl,wtn terms: */
	  #ifdef USE_AVX512
		tmp = half_arr +  64;	// No lookup-tables used in avx-512; instead use opmasked conditional-doubling;
								// 1st 64 slots hold outputs of wtsinit call. Only half of said slots used in 8-way-init mode.
	  #else
		tmp = half_arr + 128;	// 1st 64 slots are basic-4 LUTs, next 32 are the additional 2 LOACC LUTs, next 32 hold outputs of wtsinit call
	  #endif
		l= j & (nwt-1);						// These rcol wts-terms are for individual-double-broadcast-to-full-vector-width,
		n_minus_sil  ->d0 = n-si[l  ];		tmp->d0 = wt0[    l  ];	// hence the mixing of fwd/inv wts, which is normally taboo.
		n_minus_silp1->d0 = n-si[l+1];		tmp->d1 = wt0[nwt-l  ]*scale;
		sinwt        ->d0 = si[nwt-l  ];	tmp->d2 = wt0[    l+1];
		sinwtm1      ->d0 = si[nwt-l-1];	tmp->d3 = wt0[nwt-l-1]*scale;

		l= (j+2) & (nwt-1);					++tmp;	/* Get ready for next 4 weights-related doubles... */
		n_minus_sil  ->d1 = n-si[l  ];		tmp->d0 = wt0[    l  ];
		n_minus_silp1->d1 = n-si[l+1];		tmp->d1 = wt0[nwt-l  ]*scale;
		sinwt        ->d1 = si[nwt-l  ];	tmp->d2 = wt0[    l+1];
		sinwtm1      ->d1 = si[nwt-l-1];	tmp->d3 = wt0[nwt-l-1]*scale;

		l= (j+4) & (nwt-1);					++tmp;
		n_minus_sil  ->d2 = n-si[l  ];		tmp->d0 = wt0[    l  ];
		n_minus_silp1->d2 = n-si[l+1];		tmp->d1 = wt0[nwt-l  ]*scale;
		sinwt        ->d2 = si[nwt-l  ];	tmp->d2 = wt0[    l+1];
		sinwtm1      ->d2 = si[nwt-l-1];	tmp->d3 = wt0[nwt-l-1]*scale;

		l= (j+6) & (nwt-1);					++tmp;
		n_minus_sil  ->d3 = n-si[l  ];		tmp->d0 = wt0[    l  ];
		n_minus_silp1->d3 = n-si[l+1];		tmp->d1 = wt0[nwt-l  ]*scale;
		sinwt        ->d3 = si[nwt-l  ];	tmp->d2 = wt0[    l+1];
		sinwtm1      ->d3 = si[nwt-l-1];	tmp->d3 = wt0[nwt-l-1]*scale;
	  #ifdef USE_AVX512
		l= (j+8) & (nwt-1);					tmp -= 3;	// Reset to same tmp-startval as above, now copy data into d4-7 slots of vec_dbl
		n_minus_sil  ->d4 = n-si[l  ];		tmp->d4 = wt0[    l  ];
		n_minus_silp1->d4 = n-si[l+1];		tmp->d5 = wt0[nwt-l  ]*scale;
		sinwt        ->d4 = si[nwt-l  ];	tmp->d6 = wt0[    l+1];
		sinwtm1      ->d4 = si[nwt-l-1];	tmp->d7 = wt0[nwt-l-1]*scale;

		l= (j+10) & (nwt-1);				++tmp;
		n_minus_sil  ->d5 = n-si[l  ];		tmp->d4 = wt0[    l  ];
		n_minus_silp1->d5 = n-si[l+1];		tmp->d5 = wt0[nwt-l  ]*scale;
		sinwt        ->d5 = si[nwt-l  ];	tmp->d6 = wt0[    l+1];
		sinwtm1      ->d5 = si[nwt-l-1];	tmp->d7 = wt0[nwt-l-1]*scale;

		l= (j+12) & (nwt-1);				++tmp;
		n_minus_sil  ->d6 = n-si[l  ];		tmp->d4 = wt0[    l  ];
		n_minus_silp1->d6 = n-si[l+1];		tmp->d5 = wt0[nwt-l  ]*scale;
		sinwt        ->d6 = si[nwt-l  ];	tmp->d6 = wt0[    l+1];
		sinwtm1      ->d6 = si[nwt-l-1];	tmp->d7 = wt0[nwt-l-1]*scale;

		l= (j+14) & (nwt-1);				++tmp;
		n_minus_sil  ->d7 = n-si[l  ];		tmp->d4 = wt0[    l  ];
		n_minus_silp1->d7 = n-si[l+1];		tmp->d5 = wt0[nwt-l  ]*scale;
		sinwt        ->d7 = si[nwt-l  ];	tmp->d6 = wt0[    l+1];
		sinwtm1      ->d7 = si[nwt-l-1];	tmp->d7 = wt0[nwt-l-1]*scale;
	  #endif

	  #ifdef LOACC

		// Since use wt1-array in the wtsinit macro, need to fiddle this here:
		co2 = co3;	// For all data but the first set in each j-block, co2=co3. Thus, after the first block of data is done
					// (and only then: for all subsequent blocks it's superfluous), this assignment decrements co2 by radix(1).
		i = (!j);
	   #ifdef CARRY_16_WAY
		const uint32 nloop = RADIX>>4;
		AVX_cmplx_carry_fast_pow2_wtsinit_X16(add1,add2,add3, bjmodn, half_arr,sign_mask, n_minus_sil,n_minus_silp1,sinwt,sinwtm1, sse_bw,sse_nm1)
	   #else
		const uint32 nloop = RADIX>>3;
		AVX_cmplx_carry_fast_pow2_wtsinit_X8 (add1,add2,add3, bjmodn, half_arr,sign_mask, n_minus_sil,n_minus_silp1,sinwt,sinwtm1, sse_bw,sse_nm1)
	   #endif
		tmp = s1p00; tm1 = cy_r; tm2 = cy_r+1; itmp = bjmodn; itm2 = bjmodn+4;
		for(l = 0; l < nloop; l++) {
			// Each AVX carry macro call also processes 8 prefetches of main-array data
			add0 = a + j1 + pfetch_dist + poff[l+l];
		#ifdef USE_AVX512
		  #ifdef CARRY_16_WAY
			AVX_cmplx_carry_fast_pow2_errcheck_X16(tmp, tm1,     itmp, half_arr,i,sign_mask,sse_bw,sse_nm1,sse_sw, add0,p01,p02,p03,p04);
			tmp += 32; tm1 += 2;           itmp += 16; i = 0;
		  #else
			AVX_cmplx_carry_fast_pow2_errcheck_X8 (tmp, tm1,     itmp, half_arr,i,sign_mask,sse_bw,sse_nm1,sse_sw, add0,p01,p02,p03,p04);
			tmp += 16; tm1 += 1;           itmp += 8; i = 0;
		  #endif
		#else
			AVX_cmplx_carry_fast_pow2_errcheck_X8(tmp, tm1,tm2, itmp,itm2, half_arr,i,sign_mask,sse_bw,sse_nm1,sse_sw, add0,p01,p02,p03,p04);
			tmp += 16; tm1 += 2; tm2 += 2; itmp += 8; itm2 += 8; i = 0;
		#endif
		}

	  #else	// USE_AVX: Hi-accuracy 4-way carry is the default:

		/* In AVX mode advance carry-ptrs just 1 for each vector-carry-macro call: */
		tm1 = s1p00; tmp = cy_r; itmp = bjmodn;
		i = (!j);
		for(l = 0; l < RADIX>>2; l++) {
			// Each AVX carry macro call also processes 4 prefetches of main-array data
			add0 = a + j1 + pfetch_dist + poff[l];
			AVX_cmplx_carry_norm_pow2_errcheck_X4(tm1,add1,add2,add3,tmp,itmp,half_arr,i,n_minus_silp1,n_minus_sil,sign_mask,sinwt,sinwtm1,sse_bw,sse_nm1,sse_sw, add0,p01,p02,p03);
			tm1 += 8; tmp++; itmp += 4; i = 0;
		}

		co2 = co3;	// For all data but the first set in each j-block, co2=co3. Thus, after the first block of data is done
					// (and only then: for all subsequent blocks it's superfluous), this assignment decrements co2 by radix(1).

	  #endif	// LOACC ?

		i =((uint32)(sw - bjmodn[0]) >> 31);	/* get ready for the next set...	*/

	#elif defined(USE_SSE2)

	  #ifdef LOACC

		uint32 k0,k1,k2,k3, ii,loop,nwtml, co2save = co2;

		i = (!j);	// Need this to force 0-wod to be bigword
		tm1 = s1p00; tmp = cy_r; tm2 = cy_r+0x01; itmp = bjmodn;
		// Beyond chain length 8, the chained-weights scheme becomes too inaccurate, so re-init seed-wts every 8th pass:
		for(loop = 0; loop < RADIX>>2; loop += 4)
		{
			ii = loop << 2;	// Reflects 4 independent carry chains being done in each SSE2_cmplx_carry_fast_pow2_errcheck call
			/*** wt_re,wi_re,wt_im,wi_im inits. Cf. radix16_main_carry_loop.h for scalar-macro prototyping of this: ***/
			l = j & (nwt-1);	nwtml = nwt-l;
			n_minus_sil   = n-si[l  ];
			n_minus_silp1 = n-si[l+1];
			sinwt   = si[nwtml  ];
			sinwtm1 = si[nwtml-1];
			wtl     = wt0[    l  ];
			wtn     = wt0[nwtml  ]*scale;
			wtlp1   = wt0[    l+1];
			wtnm1   = wt0[nwtml-1]*scale;
	
			co2 = co2save;	// Need this for all wts-inits beynd the initial set, due to the co2 = co3 preceding the (j+2) data
			ctmp = (struct complex *)half_arr + 24;	// ptr to local storage for the doubled wtl,wtn terms:
			// (j)-data occupy the 8 xmm-sized slots above the 16 used by fixed auxiliary-data, and overwrite these inits:
			ctmp->re = ctmp->im = wtl;		ctmp += 2;
			ctmp->re = ctmp->im = wtn;		ctmp += 2;
			ctmp->re = ctmp->im = wtlp1;	ctmp += 2;
			ctmp->re = ctmp->im = wtnm1;
	
			l = (j+2) & (nwt-1);	nwtml = nwt-l;;
			k0 = n-si[l  ];
			k1 = n-si[l+1];
			k2 = si[nwtml  ];
			k3 = si[nwtml-1];
			wtl     = wt0[    l  ];
			wtn     = wt0[nwtml  ]*scale;
			wtlp1   = wt0[    l+1];
			wtnm1   = wt0[nwtml-1]*scale;
	
			ctmp = (struct complex *)half_arr + 32;	// (j+2) data start at ctmp + 8
			ctmp->re = ctmp->im = wtl;		ctmp += 2;
			ctmp->re = ctmp->im = wtn;		ctmp += 2;
			ctmp->re = ctmp->im = wtlp1;	ctmp += 2;
			ctmp->re = ctmp->im = wtnm1;
	
			add1 = &wt1[col  +ii];	/* Don't use add0 here, to avoid need to reload main-array address */
			add2 = &wt1[co2-1-ii];
			add3 = &wt1[co3-1-ii];
	
			// Since use wt1-array in the wtsinit macro, need to fiddle this here:
			co2 = co3;	// For all data but the first set in each j-block, co2=co3. Thus, after the first block of data is done
						// (and only then: for all subsequent blocks it's superfluous), this assignment decrements co2 by radix(1).
			// *But*: since the init macro does an on-the-fly version of this between j,j+2 portions, external code co2=co3 must come *after* both ctmp-data octets are inited.
			add0 = (double*)(bjmodn+ii);
			SSE2_cmplx_carry_fast_pow2_wtsinit(add1,add2,add3, add0, half_arr,sign_mask, n_minus_sil,n_minus_silp1,sinwt,sinwtm1, k0,k1,k2,k3, sse_bw,sse_nm1)

			for(l = loop; l < loop+4; l++) {
				// Each SSE2 LOACC carry macro call also processes 4 prefetches of main-array data:
				add0 = a + j1 + pfetch_dist + poff[l];	// poff[] = p0,4,8,...
				SSE2_cmplx_carry_fast_pow2_errcheck(tm1,tmp,tm2,itmp,half_arr,i,sign_mask,sse_bw,sse_nm1,sse_sw, add0,p01,p02,p03);
				tm1 += 8; tmp += 2; tm2 += 2; itmp += 4; i = 0;
			}
		}

	  #else	// Hi-accuracy is the default:

		l= j & (nwt-1);			/* We want (S*J mod N) - SI(L) for all 32 carries, so precompute	*/
		n_minus_sil   = n-si[l  ];		/* N - SI(L) and for each J, find N - (B*J mod N) - SI(L)		*/
		n_minus_silp1 = n-si[l+1];		/* For the inverse weight, want (S*(N - J) mod N) - SI(NWT - L) =	*/
		sinwt   = si[nwt-l  ];		/*	= N - (S*J mod N) - SI(NWT - L) = (B*J mod N) - SI(NWT - L).	*/
		sinwtm1 = si[nwt-l-1];

		wtl     =wt0[    l  ];
		wtn     =wt0[nwt-l  ]*scale;	/* Include 1/(n/2) scale factor of inverse transform here...	*/
		wtlp1   =wt0[    l+1];
		wtnm1   =wt0[nwt-l-1]*scale;	/* ...and here.	*/

		ctmp = (struct complex *)half_arr + 16;	/* ptr to local storage for the doubled wtl,wtn terms: */
		ctmp->re = ctmp->im = wtl;		++ctmp;
		ctmp->re = ctmp->im = wtn;		++ctmp;
		ctmp->re = ctmp->im = wtlp1;	++ctmp;
		ctmp->re = ctmp->im = wtnm1;

		add1 = &wt1[col  ];
		add2 = &wt1[co2-1];
		add3 = &wt1[co3-1];

		tm1 = s1p00; tmp = cy_r; tm2 = cy_r+0x01; itmp = bjmodn;
		i = (!j);
		for(l = 0; l < RADIX>>2; l++) {
			// Each SSE2 carry macro call also processes 2 prefetches of main-array data
			add0 = a + j1 + pfetch_dist + poff[l];	// poff[] = p0,4,8,...
			add0 += (-(l&0x1)) & p02;	// Base-addr incr by extra p2 on odd-index passes
			SSE2_cmplx_carry_norm_pow2_errcheck1_2B(tm1,add1,add2,add3,tmp,tm2,itmp,half_arr,i,n_minus_silp1,n_minus_sil,sign_mask,sinwt,sinwtm1,sse_bw,sse_nm1,sse_sw, add0,p01);
			tm1 += 8; tmp += 2; tm2 += 2; itmp += 4; i = 0;
		}

		l= (j+2) & (nwt-1);			/* We want (S*J mod N) - SI(L) for all 16 carries, so precompute	*/
		n_minus_sil   = n-si[l  ];		/* N - SI(L) and for each J, find N - (B*J mod N) - SI(L)		*/
		n_minus_silp1 = n-si[l+1];		/* For the inverse weight, want (S*(N - J) mod N) - SI(NWT - L) =	*/
		sinwt   = si[nwt-l  ];		/*	= N - (S*J mod N) - SI(NWT - L) = (B*J mod N) - SI(NWT - L).	*/
		sinwtm1 = si[nwt-l-1];

		wtl     =wt0[    l  ];
		wtn     =wt0[nwt-l  ]*scale;	/* Include 1/(n/2) scale factor of inverse transform here...	*/
		wtlp1   =wt0[    l+1];
		wtnm1   =wt0[nwt-l-1]*scale;	/* ...and here.	*/

		ctmp = (struct complex *)half_arr + 16;	/* ptr to local storage for the doubled wtl,wtn terms: */
		ctmp->re = ctmp->im = wtl;		++ctmp;
		ctmp->re = ctmp->im = wtn;		++ctmp;
		ctmp->re = ctmp->im = wtlp1;	++ctmp;
		ctmp->re = ctmp->im = wtnm1;

		co2 = co3;	// For all data but the first set in each j-block, co2=co3. Thus, after the first block of data is done
					// (and only then: for all subsequent blocks it's superfluous), this assignment decrements co2 by radix(1).
		add1 = &wt1[col  ];
		add2 = &wt1[co2-1];

		tm1 = s1p00; tmp = cy_r; tm2 = cy_r+0x01; itmp = bjmodn;
		for(l = 0; l < RADIX>>2; l++) {
			// Each SSE2 carry macro call also processes 2 prefetches of main-array data
			add0 = a + j1 + pfetch_dist + poff[l];	// poff[] = p0,4,8,...
			add0 += (-(l&0x1)) & p02;	// Base-addr incr by extra p2 on odd-index passes
			SSE2_cmplx_carry_norm_pow2_errcheck2_2B(tm1,add1,add2,     tmp,tm2,itmp,half_arr,  n_minus_silp1,n_minus_sil,sign_mask,sinwt,sinwtm1,sse_bw,sse_nm1,sse_sw, add0,p02,p03);
			tm1 += 8; tmp += 2; tm2 += 2; itmp += 4;
		}

	  #endif // HIACC or LOACC?

		i =((uint32)(sw - bjmodn[0]) >> 31);	/* get ready for the next set...	*/

	#else	// Scalar-double mode:

		l= j & (nwt-1);			/* We want (S*J mod N) - SI(L) for all 32 carries, so precompute	*/
		n_minus_sil   = n-si[l  ];		/* N - SI(L) and for each J, find N - (B*J mod N) - SI(L)		*/
		n_minus_silp1 = n-si[l+1];		/* For the inverse weight, want (S*(N - J) mod N) - SI(NWT - L) =	*/
		sinwt   = si[nwt-l  ];		/*	= N - (S*J mod N) - SI(NWT - L) = (B*J mod N) - SI(NWT - L).	*/
		sinwtm1 = si[nwt-l-1];

		wtl     =wt0[    l  ];
		wtn     =wt0[nwt-l  ]*scale;	/* Include 1/(n/2) scale factor of inverse transform here...	*/
		wtlp1   =wt0[    l+1];
		wtnm1   =wt0[nwt-l-1]*scale;	/* ...and here.	*/

	  #ifdef LOACC

		/*...set0 is slightly different from others; divide work into blocks of 4 macro calls, 1st set of which gets pulled out of loop: */
		l = 0; addr = cy_r; itmp = bjmodn;
	   cmplx_carry_norm_pow2_errcheck0(a[j1    ],a[j2    ],*addr,*itmp,0); ++l; ++addr; ++itmp;
		cmplx_carry_fast_pow2_errcheck(a[j1+p01],a[j2+p01],*addr,*itmp,l); ++l; ++addr; ++itmp;
		cmplx_carry_fast_pow2_errcheck(a[j1+p02],a[j2+p02],*addr,*itmp,l); ++l; ++addr; ++itmp;
		cmplx_carry_fast_pow2_errcheck(a[j1+p03],a[j2+p03],*addr,*itmp,l); ++l; ++addr; ++itmp;
		// Remaining quartets of macro calls done in loop:
		for(ntmp = 1; ntmp < RADIX>>2; ntmp++) {
			jt = j1 + poff[ntmp]; jp = j2 + poff[ntmp];	// poff[] = p04,08,...
			// Re-init weights every 4th macro invocatin to keep errors under control:
			cmplx_carry_norm_pow2_errcheck0(a[jt    ],a[jp    ],*addr,*itmp,l); ++l; ++addr; ++itmp;
			cmplx_carry_fast_pow2_errcheck (a[jt+p01],a[jp+p01],*addr,*itmp,l); ++l; ++addr; ++itmp;
			cmplx_carry_fast_pow2_errcheck (a[jt+p02],a[jp+p02],*addr,*itmp,l); ++l; ++addr; ++itmp;
			cmplx_carry_fast_pow2_errcheck (a[jt+p03],a[jp+p03],*addr,*itmp,l); ++l; ++addr; ++itmp;
		}

	  #else	// Hi-accuracy is the default:

		/*...set0 is slightly different from others; divide work into blocks of 4 macro calls, 1st set of which gets pulled out of loop: */
		l = 0; addr = cy_r; itmp = bjmodn;
	   cmplx_carry_norm_pow2_errcheck0(a[j1    ],a[j2    ],*addr,*itmp,0); ++l; ++addr; ++itmp;
		cmplx_carry_norm_pow2_errcheck(a[j1+p01],a[j2+p01],*addr,*itmp,l); ++l; ++addr; ++itmp;
		cmplx_carry_norm_pow2_errcheck(a[j1+p02],a[j2+p02],*addr,*itmp,l); ++l; ++addr; ++itmp;
		cmplx_carry_norm_pow2_errcheck(a[j1+p03],a[j2+p03],*addr,*itmp,l); ++l; ++addr; ++itmp;
		// Remaining quartets of macro calls done in loop:
		for(ntmp = 1; ntmp < RADIX>>2; ntmp++) {
			jt = j1 + poff[ntmp]; jp = j2 + poff[ntmp];	// poff[] = p04,08,...
			cmplx_carry_norm_pow2_errcheck(a[jt    ],a[jp    ],*addr,*itmp,l); ++l; ++addr; ++itmp;
			cmplx_carry_norm_pow2_errcheck(a[jt+p01],a[jp+p01],*addr,*itmp,l); ++l; ++addr; ++itmp;
			cmplx_carry_norm_pow2_errcheck(a[jt+p02],a[jp+p02],*addr,*itmp,l); ++l; ++addr; ++itmp;
			cmplx_carry_norm_pow2_errcheck(a[jt+p03],a[jp+p03],*addr,*itmp,l); ++l; ++addr; ++itmp;
		}

	  #endif

		i =((uint32)(sw - bjmodn[0]) >> 31);	/* get ready for the next set...	*/
		co2=co3;	/* For all data but the first set in each j-block, co2=co3. Thus, after the first block of data is done
				 and only then: for all subsequent blocks it's superfluous), this assignment decrements co2 by radix(1).	*/

	#endif	// USE_AVX?
	}
	else	/* Fermat-mod carry in SIMD mode */
	{
	#ifdef USE_AVX512

		// For a description of the data movement in AVX mode, see radix28_ditN_cy_dif1.
		tmp = half_arr+2;	// In Fermat-mod mode, use first 2 vector slots above half_arr for base and 1/base
		VEC_DBL_INIT(tmp, scale);
		/* Get the needed Nth root of -1: */
		add1 = (double *)&rn0[0];
		add2 = (double *)&rn1[0];

		idx_offset = j;
		idx_incr = NDIVR;

		tmp = base_negacyclic_root;	tm2 = tmp+1;

		// Hi-accuracy version needs 2 copies of each base root, one for each invocation of the SSE2_fermat_carry_norm_pow2 carry macri:
		l = (j >> 1);	k1=(l & NRTM1);	k2=(l >> NRT_BITS);
		dtmp=rn0[k1].re;			wt_im=rn0[k1].im;
		rt  =rn1[k2].re;			it   =rn1[k2].im;
		wt_re =dtmp*rt-wt_im*it;	wt_im =dtmp*it+wt_im*rt;
		for(i = 0; i < (RADIX << 1); i += 16) {
			VEC_DBL_INIT(tmp+ i,wt_re);	VEC_DBL_INIT(tm2+ i,wt_im);
		}
		tmp += 2;	tm2 += 2;
		l += 1;	k1=(l & NRTM1);	k2=(l >> NRT_BITS);
		dtmp=rn0[k1].re;			wt_im=rn0[k1].im;
		rt  =rn1[k2].re;			it   =rn1[k2].im;
		wt_re =dtmp*rt-wt_im*it;	wt_im =dtmp*it+wt_im*rt;
		for(i = 0; i < (RADIX << 1); i += 16) {
			VEC_DBL_INIT(tmp+ i,wt_re);	VEC_DBL_INIT(tm2+ i,wt_im);
		}
		tmp += 2;	tm2 += 2;
		l += 1;	k1=(l & NRTM1);	k2=(l >> NRT_BITS);
		dtmp=rn0[k1].re;			wt_im=rn0[k1].im;
		rt  =rn1[k2].re;			it   =rn1[k2].im;
		wt_re =dtmp*rt-wt_im*it;	wt_im =dtmp*it+wt_im*rt;
		for(i = 0; i < (RADIX << 1); i += 16) {
			VEC_DBL_INIT(tmp+ i,wt_re);	VEC_DBL_INIT(tm2+ i,wt_im);
		}
		tmp += 2;	tm2 += 2;
		l += 1;	k1=(l & NRTM1);	k2=(l >> NRT_BITS);
		dtmp=rn0[k1].re;			wt_im=rn0[k1].im;
		rt  =rn1[k2].re;			it   =rn1[k2].im;
		wt_re =dtmp*rt-wt_im*it;	wt_im =dtmp*it+wt_im*rt;
		for(i = 0; i < (RADIX << 1); i += 16) {
			VEC_DBL_INIT(tmp+ i,wt_re);	VEC_DBL_INIT(tm2+ i,wt_im);
		}
		tmp += 2;	tm2 += 2;
		l += 1;	k1=(l & NRTM1);	k2=(l >> NRT_BITS);
		dtmp=rn0[k1].re;			wt_im=rn0[k1].im;
		rt  =rn1[k2].re;			it   =rn1[k2].im;
		wt_re =dtmp*rt-wt_im*it;	wt_im =dtmp*it+wt_im*rt;
		for(i = 0; i < (RADIX << 1); i += 16) {
			VEC_DBL_INIT(tmp+ i,wt_re);	VEC_DBL_INIT(tm2+ i,wt_im);
		}
		tmp += 2;	tm2 += 2;
		l += 1;	k1=(l & NRTM1);	k2=(l >> NRT_BITS);
		dtmp=rn0[k1].re;			wt_im=rn0[k1].im;
		rt  =rn1[k2].re;			it   =rn1[k2].im;
		wt_re =dtmp*rt-wt_im*it;	wt_im =dtmp*it+wt_im*rt;
		for(i = 0; i < (RADIX << 1); i += 16) {
			VEC_DBL_INIT(tmp+ i,wt_re);	VEC_DBL_INIT(tm2+ i,wt_im);
		}
		tmp += 2;	tm2 += 2;
		l += 1;	k1=(l & NRTM1);	k2=(l >> NRT_BITS);
		dtmp=rn0[k1].re;			wt_im=rn0[k1].im;
		rt  =rn1[k2].re;			it   =rn1[k2].im;
		wt_re =dtmp*rt-wt_im*it;	wt_im =dtmp*it+wt_im*rt;
		for(i = 0; i < (RADIX << 1); i += 16) {
			VEC_DBL_INIT(tmp+ i,wt_re);	VEC_DBL_INIT(tm2+ i,wt_im);
		}
		tmp += 2;	tm2 += 2;
		l += 1;	k1=(l & NRTM1);	k2=(l >> NRT_BITS);
		dtmp=rn0[k1].re;			wt_im=rn0[k1].im;
		rt  =rn1[k2].re;			it   =rn1[k2].im;
		wt_re =dtmp*rt-wt_im*it;	wt_im =dtmp*it+wt_im*rt;
		for(i = 0; i < (RADIX << 1); i += 16) {
			VEC_DBL_INIT(tmp+ i,wt_re);	VEC_DBL_INIT(tm2+ i,wt_im);
		}

		// AVX-custom 8-way carry macro - each contains 8 of the [RADIX] stride-n/RADIX-separated carries
		// (processed independently in parallel), and steps through sequential-data indicies j,j+2,j+4,j+6,j+8,j+10,j+12,j+14:
		tm0 = s1p00; tmp = base_negacyclic_root; tm1 = cy_r; tm2 = cy_i; l = 0x2000;
		for(i = 0; i < RADIX>>3; i++) {	// RADIX/8 loop passes
			// Each AVX carry macro call also processes 8 prefetches of main-array data
			add0 = a + j1 + pfetch_dist + poff[2*i];	// Can't use tm2 as prefetch base-ptr for pow2 Fermat-mod carry-macro since that's used for cy_i
			SSE2_fermat_carry_norm_pow2_errcheck_X8(tm0,tmp,l,tm1,tm2,half_arr,sign_mask, add0,p01,p02,p03,p04);
			tm0 += 16; tm1++; tm2++; tmp += 16; l -= 0x380;
		}

	#elif defined(USE_AVX)

		// For a description of the data movement for Fermat-mod carries in SSE2 mode, see radix16_ditN_cy_dif1.c.
		// For a description of the data movement in AVX mode, see radix28_ditN_cy_dif1.

		tmp = half_arr+2;
		VEC_DBL_INIT(tmp, scale);
		/* Get the needed Nth root of -1: */
		add1 = (double *)&rn0[0];
		add2 = (double *)&rn1[0];

		idx_offset = j;
		idx_incr = NDIVR;

		tmp = base_negacyclic_root;	tm2 = tmp+1;

		// Hi-accuracy version needs 8 copies of each base root:
		l = (j >> 1);	k1=(l & NRTM1);	k2=(l >> NRT_BITS);
		dtmp=rn0[k1].re;			wt_im=rn0[k1].im;
		rt  =rn1[k2].re;			it   =rn1[k2].im;
		wt_re =dtmp*rt-wt_im*it;	wt_im =dtmp*it+wt_im*rt;
		for(i = 0; i < (RADIX << 1); i += 8) {
			VEC_DBL_INIT(tmp+ i,wt_re);	VEC_DBL_INIT(tm2+ i,wt_im);
		}
		tmp += 2;	tm2 += 2;
		l += 1;	k1=(l & NRTM1);	k2=(l >> NRT_BITS);
		dtmp=rn0[k1].re;			wt_im=rn0[k1].im;
		rt  =rn1[k2].re;			it   =rn1[k2].im;
		wt_re =dtmp*rt-wt_im*it;	wt_im =dtmp*it+wt_im*rt;
		for(i = 0; i < (RADIX << 1); i += 8) {
			VEC_DBL_INIT(tmp+ i,wt_re);	VEC_DBL_INIT(tm2+ i,wt_im);
		}
		tmp += 2;	tm2 += 2;
		l += 1;	k1=(l & NRTM1);	k2=(l >> NRT_BITS);
		dtmp=rn0[k1].re;			wt_im=rn0[k1].im;
		rt  =rn1[k2].re;			it   =rn1[k2].im;
		wt_re =dtmp*rt-wt_im*it;	wt_im =dtmp*it+wt_im*rt;
		for(i = 0; i < (RADIX << 1); i += 8) {
			VEC_DBL_INIT(tmp+ i,wt_re);	VEC_DBL_INIT(tm2+ i,wt_im);
		}
		tmp += 2;	tm2 += 2;
		l += 1;	k1=(l & NRTM1);	k2=(l >> NRT_BITS);
		dtmp=rn0[k1].re;			wt_im=rn0[k1].im;
		rt  =rn1[k2].re;			it   =rn1[k2].im;
		wt_re =dtmp*rt-wt_im*it;	wt_im =dtmp*it+wt_im*rt;
		for(i = 0; i < (RADIX << 1); i += 8) {
			VEC_DBL_INIT(tmp+ i,wt_re);	VEC_DBL_INIT(tm2+ i,wt_im);
		}

		// AVX-custom 4-way carry macro - each contains 4 of the RADIX stride-n/RADIX-separated carries
		// (processed independently in parallel), and steps through sequential-data indicies j,j+2,j+4,j+6:

	// The starting value of the literal pointer offsets following 'tmp' in these macro calls = RADIX*2*sizeof(vec_dbl) = 2^12
	// which is the byte offset between the 'active' negacyclic weights [pointed to by base_negacyclic_root] and the
	// precomputed multipliers in the HIACC-wrapped section of the SIMD data initializations. Each 0x100-byte quartet of base roots
	// uses the same 0x40-byte up-multiplier, so the literal offsets advance (+0x100-0x40) = -0xc0 bytes between macro calls:
		tm0 = s1p00; tmp = base_negacyclic_root; tm1 = cy_r; tm2 = cy_i; l = 0x1000;
		for(i = 0; i < RADIX>>2; i++) {	// RADIX/4 loop passes
			// Each AVX carry macro call also processes 4 prefetches of main-array data
			add0 = a + j1 + pfetch_dist + poff[i];	// Can't use tm2 as prefetch base-ptr for pow2 Fermat-mod carry-macro since that's used for cy_i
			SSE2_fermat_carry_norm_pow2_errcheck_X4(tm0,tmp,l,tm1,tm2,half_arr,sign_mask, add0,p01,p02,p03);
			tm0 += 8; tm1++; tm2++; tmp += 8; l -= 0xc0;
		}

	#elif defined(USE_SSE2)

		// For a description of the data movement for Fermat-mod carries in SSE2 mode, see radix16_ditN_cy_dif1.c.
		// For a description of the data movement in AVX mode, see radix28_ditN_cy_dif1.

		tmp = half_arr+2;
		VEC_DBL_INIT(tmp, scale);
		/* Get the needed Nth root of -1: */
		add1 = (double *)&rn0[0];
		add2 = (double *)&rn1[0];

		idx_offset = j;
		idx_incr = NDIVR;

		tm1 = s1p00; tmp = cy_r;	/* <*** Again rely on contiguity of cy_r,i here ***/
	  #if (OS_BITS == 32)
		for(l = 0; l < RADIX; l++) {	// RADIX loop passes
			// Each SSE2 carry macro call also processes 1 prefetch of main-array data
			tm2 = a + j1 + pfetch_dist + poff[l>>2];	// poff[] = p0,4,8,...
			tm2 += (-(l&0x10)) & p02;
			tm2 += (-(l&0x01)) & p01;
			SSE2_fermat_carry_norm_pow2_errcheck   (tm1,tmp,NRT_BITS,NRTM1,idx_offset,idx_incr,half_arr,sign_mask,add1,add2, tm2);
			tm1 += 2; tmp++;
		}
	  #else	// 64-bit SSE2
		for(l = 0; l < RADIX>>1; l++) {	// RADIX/2 loop passes
			// Each SSE2 carry macro call also processes 2 prefetches of main-array data
			tm2 = a + j1 + pfetch_dist + poff[l>>1];	// poff[] = p0,4,8,...; (tm1-cy_r) acts as a linear loop index running from 0,...,RADIX-1 here.
			tm2 += (-(l&0x1)) & p02;	// Base-addr incr by extra p2 on odd-index passes
			SSE2_fermat_carry_norm_pow2_errcheck_X2(tm1,tmp,NRT_BITS,NRTM1,idx_offset,idx_incr,half_arr,sign_mask,add1,add2, tm2,p01);
			tm1 += 4; tmp += 2;
		}
	  #endif

	#else	// Scalar-double mode:

		// Can't use l as loop index here, since it gets used in the Fermat-mod carry macro (as are k1,k2);
		ntmp = 0; addr = cy_r; addi = cy_i;
		for(m = 0; m < RADIX>>2; m++) {
			jt = j1 + poff[m]; jp = j2 + poff[m];	// poff[] = p04,08,...
			fermat_carry_norm_pow2_errcheck(a[jt    ],a[jp    ],*addr,*addi,ntmp,NRTM1,NRT_BITS);	ntmp += NDIVR; ++addr; ++addi;
			fermat_carry_norm_pow2_errcheck(a[jt+p01],a[jp+p01],*addr,*addi,ntmp,NRTM1,NRT_BITS);	ntmp += NDIVR; ++addr; ++addi;
			fermat_carry_norm_pow2_errcheck(a[jt+p02],a[jp+p02],*addr,*addi,ntmp,NRTM1,NRT_BITS);	ntmp += NDIVR; ++addr; ++addi;
			fermat_carry_norm_pow2_errcheck(a[jt+p03],a[jp+p03],*addr,*addi,ntmp,NRTM1,NRT_BITS);	ntmp += NDIVR; ++addr; ++addi;
		}

	#endif	/* #ifdef USE_SSE2 */

	}	/* if(MODULUS_TYPE == ...) */

/*...The radix-64 DIF pass is here:	*/

#ifdef USE_SSE2

/*...and now do 16 radix-16 subtransforms, including the internal twiddle factors - we use the same positive-power
roots as in the DIF here, just fiddle with signs within the macro to effect the conjugate-multiplies. Twiddles occur
in the same order here as DIF, but the in-and-output-index offsets are BRed: j1 + p[0,8,4,c,2,a,6,e,1,9,5,d,3,b,7,f].
*/
/* Gather the needed data (64 64-bit complex, i.e. 128 64-bit reals) and do 8 twiddleless length-8 subtransforms: */

  #if USE_SCALAR_DFT_MACRO

	SSE2_RADIX_64_DIF( FALSE, thr_id,
		0,
		// Input pointer: Base ptr of 8 local-mem:
		s1p00, c_offsets,	// If want to use this, need a valid i_offsets array/pointer here!
		// Intermediates base pointer:
		r00,
		// Outputs: Base address plus index offset lo/hi-half arrays:
		(a+j1),dft_offsets
	);

  #else

   #ifdef USE_AVX512
	#define OFF1	4*0x100
	#define OFF2	4*0x200
	#define OFF3	4*0x300
	#define OFF4	4*0x400
	#define OFF5	4*0x500
	#define OFF6	4*0x600
	#define OFF7	4*0x700
   #elif defined(USE_AVX)
	#define OFF1	2*0x100
	#define OFF2	2*0x200
	#define OFF3	2*0x300
	#define OFF4	2*0x400
	#define OFF5	2*0x500
	#define OFF6	2*0x600
	#define OFF7	2*0x700
   #else
	#define OFF1	0x100
	#define OFF2	0x200
	#define OFF3	0x300
	#define OFF4	0x400
	#define OFF5	0x500
	#define OFF6	0x600
	#define OFF7	0x700
   #endif

   // Unlike for DIT, reducing obj-code size here by wrapping SSE2_RADIX8_DIF_0TWIDDLE in a loop hurt performance on SSE2:
   #if 1	// Jan 2016: Loop version sign. faster on KNL under AVX-512, make that the default moving forward

	// Outout (s-pointer) offsets are normal BRed - note r's in this routine are separated as 1*index-stride, not 2* as for others:
	vec_dbl *out0, *out1, *out2, *out3, *out4, *out5, *out6, *out7;
	out0 = r00; out1 = out0+8; out2 = out0+4; out3 = out0+12; out4 = out0+2; out5 = out0+10; out6 = out0+6; out7 = out0+14;
	for(l = 0; l < 8; l++) {
		k1 = reverse(l,8)<<1;
		tmp = s1p00 + k1;
	  #ifdef USE_AVX2
		SSE2_RADIX8_DIF_0TWIDDLE(
			tmp,OFF1,OFF2,OFF3,OFF4,OFF5,OFF6,OFF7,
			out0,out1,out2,out3,out4,out5,out6,out7, isrt2,two
		);
	  #else
		SSE2_RADIX8_DIF_0TWIDDLE(
			tmp,OFF1,OFF2,OFF3,OFF4,OFF5,OFF6,OFF7,
			out0,out1,out2,out3,out4,out5,out6,out7, isrt2
		);
	  #endif
		out0 += 16; out1 += 16; out2 += 16; out3 += 16; out4 += 16; out5 += 16; out6 += 16; out7 += 16;
	}

   #else

	  #ifdef USE_AVX2
		SSE2_RADIX8_DIF_0TWIDDLE(s1p00,OFF1,OFF2,OFF3,OFF4,OFF5,OFF6,OFF7, r00,r08,r04,r0C,r02,r0A,r06,r0E, isrt2,two);
		SSE2_RADIX8_DIF_0TWIDDLE(s1p04,OFF1,OFF2,OFF3,OFF4,OFF5,OFF6,OFF7, r10,r18,r14,r1C,r12,r1A,r16,r1E, isrt2,two);
		SSE2_RADIX8_DIF_0TWIDDLE(s1p02,OFF1,OFF2,OFF3,OFF4,OFF5,OFF6,OFF7, r20,r28,r24,r2C,r22,r2A,r26,r2E, isrt2,two);
		SSE2_RADIX8_DIF_0TWIDDLE(s1p06,OFF1,OFF2,OFF3,OFF4,OFF5,OFF6,OFF7, r30,r38,r34,r3C,r32,r3A,r36,r3E, isrt2,two);
		SSE2_RADIX8_DIF_0TWIDDLE(s1p01,OFF1,OFF2,OFF3,OFF4,OFF5,OFF6,OFF7, r40,r48,r44,r4C,r42,r4A,r46,r4E, isrt2,two);
		SSE2_RADIX8_DIF_0TWIDDLE(s1p05,OFF1,OFF2,OFF3,OFF4,OFF5,OFF6,OFF7, r50,r58,r54,r5C,r52,r5A,r56,r5E, isrt2,two);
		SSE2_RADIX8_DIF_0TWIDDLE(s1p03,OFF1,OFF2,OFF3,OFF4,OFF5,OFF6,OFF7, r60,r68,r64,r6C,r62,r6A,r66,r6E, isrt2,two);
		SSE2_RADIX8_DIF_0TWIDDLE(s1p07,OFF1,OFF2,OFF3,OFF4,OFF5,OFF6,OFF7, r70,r78,r74,r7C,r72,r7A,r76,r7E, isrt2,two);
	  #else
		//...Block 0: jt = j1;	jp = j2;
	// Relative to RADIX_08_DIF_OOP, this SIMD macro produces outputs in BR order [04261537], so swap r-pointer pairs 2/8,6/C to handle that:
		SSE2_RADIX8_DIF_0TWIDDLE(s1p00,OFF1,OFF2,OFF3,OFF4,OFF5,OFF6,OFF7, r00,r08,r04,r0C,r02,r0A,r06,r0E, isrt2);
		//...Block 1: jt = j1 + p04;	jp = j2 + p04;
		SSE2_RADIX8_DIF_0TWIDDLE(s1p04,OFF1,OFF2,OFF3,OFF4,OFF5,OFF6,OFF7, r10,r18,r14,r1C,r12,r1A,r16,r1E, isrt2);
		//...Block 2: jt = j1 + p02;	jp = j2 + p02;
		SSE2_RADIX8_DIF_0TWIDDLE(s1p02,OFF1,OFF2,OFF3,OFF4,OFF5,OFF6,OFF7, r20,r28,r24,r2C,r22,r2A,r26,r2E, isrt2);
		//...Block 3: jt = j1 + p06;	jp = j2 + p06;
		SSE2_RADIX8_DIF_0TWIDDLE(s1p06,OFF1,OFF2,OFF3,OFF4,OFF5,OFF6,OFF7, r30,r38,r34,r3C,r32,r3A,r36,r3E, isrt2);
		//...Block 4: jt = j1 + p01;	jp = j2 + p01;
		SSE2_RADIX8_DIF_0TWIDDLE(s1p01,OFF1,OFF2,OFF3,OFF4,OFF5,OFF6,OFF7, r40,r48,r44,r4C,r42,r4A,r46,r4E, isrt2);
		//...Block 5: jt = j1 + p05;	jp = j2 + p05;
		SSE2_RADIX8_DIF_0TWIDDLE(s1p05,OFF1,OFF2,OFF3,OFF4,OFF5,OFF6,OFF7, r50,r58,r54,r5C,r52,r5A,r56,r5E, isrt2);
		//...Block 6: jt = j1 + p03;	jp = j2 + p03;
		SSE2_RADIX8_DIF_0TWIDDLE(s1p03,OFF1,OFF2,OFF3,OFF4,OFF5,OFF6,OFF7, r60,r68,r64,r6C,r62,r6A,r66,r6E, isrt2);
		//...Block 7: jt = j1 + p07;	jp = j2 + p07;
		SSE2_RADIX8_DIF_0TWIDDLE(s1p07,OFF1,OFF2,OFF3,OFF4,OFF5,OFF6,OFF7, r70,r78,r74,r7C,r72,r7A,r76,r7E, isrt2);
	  #endif

    #endif	// 0?

/*...and now do eight radix-8 subtransforms w/internal twiddles - cf. radix64_dif_pass1 for details.
Note: 1st of the 15 sincos args in each call to SSE2_RADIX8_DIT_TWIDDLE_OOP is the basic isrt2 needed for
radix-8. This is a workaround of GCC's 30-arg limit for inline ASM macros, which proves a royal pain here.
*/
	/* Block 0: */
	add0 = &a[j1]      ; add1 = add0+p01; add2 = add0+p02; add3 = add0+p03; add4 = add0+p04; add5 = add0+p05; add6 = add0+p06; add7 = add0+p07;
	/* 0-index block has all-unity twiddles: Remember, the twiddleless DIF bit-reverses both its in-and-outputs,
	so swap index-offset pairs 1/4 and 3/6 in t*-inputs and a-outputs: */
  #ifdef USE_AVX2
	SSE2_RADIX8_DIF_0TWIDDLE(
		r00, OFF4,OFF2,OFF6,OFF1,OFF5,OFF3,OFF7,
		add0,add1,add2,add3,add4,add5,add6,add7, isrt2,two
	);
  #else
	SSE2_RADIX8_DIF_0TWIDDLE(
		r00, OFF4,OFF2,OFF6,OFF1,OFF5,OFF3,OFF7,
		add0,add1,add2,add3,add4,add5,add6,add7, isrt2
	);
  #endif
	/* Block 4: */
	add0 = &a[j1] + p08; add1 = add0+p01; add2 = add0+p02; add3 = add0+p03; add4 = add0+p04; add5 = add0+p05; add6 = add0+p06; add7 = add0+p07;
//...and another kludge for the 30-arg limit: put copies of (vec_dbl)2.0,SQRT2 into the first 2 of each set of outputs.
	VEC_DBL_INIT((vec_dbl *)add0,2.0);	VEC_DBL_INIT((vec_dbl *)add1,SQRT2);
	SSE2_RADIX8_DIF_TWIDDLE_OOP(
		r08,r48,r28,r68,r18,r58,r38,r78,
		add0,add1,add2,add3,add4,add5,add6,add7,
		ss0,cc0, isrt2,isrt2, nisrt2,isrt2, cc4,ss4, nss4,cc4, ss4,cc4, ncc4,ss4
	);
	/* Block 2: */
	add0 = &a[j1] + p10; add1 = add0+p01; add2 = add0+p02; add3 = add0+p03; add4 = add0+p04; add5 = add0+p05; add6 = add0+p06; add7 = add0+p07;
	VEC_DBL_INIT((vec_dbl *)add0,2.0);	VEC_DBL_INIT((vec_dbl *)add1,SQRT2);
	SSE2_RADIX8_DIF_TWIDDLE_OOP(
		r04,r44,r24,r64,r14,r54,r34,r74,
		add0,add1,add2,add3,add4,add5,add6,add7,
		isrt2,isrt2, cc4,ss4, ss4,cc4, cc2,ss2, ss6,cc6, cc6,ss6, ss2,cc2
	);
	/* Block 6: */
	add0 = &a[j1] + p18; add1 = add0+p01; add2 = add0+p02; add3 = add0+p03; add4 = add0+p04; add5 = add0+p05; add6 = add0+p06; add7 = add0+p07;
	VEC_DBL_INIT((vec_dbl *)add0,2.0);	VEC_DBL_INIT((vec_dbl *)add1,SQRT2);
	SSE2_RADIX8_DIF_TWIDDLE_OOP(
		r0C,r4C,r2C,r6C,r1C,r5C,r3C,r7C,
		add0,add1,add2,add3,add4,add5,add6,add7,
		nisrt2,isrt2, ss4,cc4, ncc4,nss4, cc6,ss6, ncc2,ss2 ,nss2,cc2, nss6,ncc6
	);
	/* Block 1: */
	add0 = &a[j1] + p20; add1 = add0+p01; add2 = add0+p02; add3 = add0+p03; add4 = add0+p04; add5 = add0+p05; add6 = add0+p06; add7 = add0+p07;
	VEC_DBL_INIT((vec_dbl *)add0,2.0);	VEC_DBL_INIT((vec_dbl *)add1,SQRT2);
	SSE2_RADIX8_DIF_TWIDDLE_OOP(
		r02,r42,r22,r62,r12,r52,r32,r72,
		add0,add1,add2,add3,add4,add5,add6,add7,
		cc4,ss4, cc2,ss2, cc6,ss6, cc1,ss1, cc5,ss5, cc3,ss3, cc7,ss7
	);
	/* Block 5: */
	add0 = &a[j1] + p28; add1 = add0+p01; add2 = add0+p02; add3 = add0+p03; add4 = add0+p04; add5 = add0+p05; add6 = add0+p06; add7 = add0+p07;
	VEC_DBL_INIT((vec_dbl *)add0,2.0);	VEC_DBL_INIT((vec_dbl *)add1,SQRT2);
	SSE2_RADIX8_DIF_TWIDDLE_OOP(
		r0A,r4A,r2A,r6A,r1A,r5A,r3A,r7A,
		add0,add1,add2,add3,add4,add5,add6,add7,
		nss4,cc4, ss6,cc6, ncc2,ss2, cc5,ss5, ncc7,ss7, ss1,cc1, ncc3,nss3
	);
	/* Block 3: */
	add0 = &a[j1] + p30; add1 = add0+p01; add2 = add0+p02; add3 = add0+p03; add4 = add0+p04; add5 = add0+p05; add6 = add0+p06; add7 = add0+p07;
	VEC_DBL_INIT((vec_dbl *)add0,2.0);	VEC_DBL_INIT((vec_dbl *)add1,SQRT2);
	SSE2_RADIX8_DIF_TWIDDLE_OOP(
		r06,r46,r26,r66,r16,r56,r36,r76,
		add0,add1,add2,add3,add4,add5,add6,add7,
		ss4,cc4, cc6,ss6, nss2,cc2, cc3,ss3, ss1,cc1, ss7,cc7, nss5,cc5
	);
	/* Block 7: */
	add0 = &a[j1] + p38; add1 = add0+p01; add2 = add0+p02; add3 = add0+p03; add4 = add0+p04; add5 = add0+p05; add6 = add0+p06; add7 = add0+p07;
	VEC_DBL_INIT((vec_dbl *)add0,2.0);	VEC_DBL_INIT((vec_dbl *)add1,SQRT2);
	SSE2_RADIX8_DIF_TWIDDLE_OOP(
		r0E,r4E,r2E,r6E,r1E,r5E,r3E,r7E,
		add0,add1,add2,add3,add4,add5,add6,add7,
		ncc4,ss4, ss2,cc2, nss6,ncc6, cc7,ss7, ncc3,nss3, nss5,cc5, ss1,ncc1
	);

	#undef OFF1
	#undef OFF2
	#undef OFF3
	#undef OFF4
	#undef OFF5
	#undef OFF6
	#undef OFF7

  #endif	// USE_SCALAR_DFT_MACRO?

#else	// USE_SSE2 = False:

/* Gather the needed data (64 64-bit complex, i.e. 128 64-bit reals) and do 8 twiddleless length-8 subtransforms: */

	tptr = t;
	for(l = 0; l < 8; l++) {
		k2 = po_br[l];	// po_br[] = p[04261537]
		jt = j1 + k2; jp = j2 + k2;
		RADIX_08_DIF_OOP(
			a[jt],a[jp],a[jt+p08],a[jp+p08],a[jt+p10],a[jp+p10],a[jt+p18],a[jp+p18],a[jt+p20],a[jp+p20],a[jt+p28],a[jp+p28],a[jt+p30],a[jp+p30],a[jt+p38],a[jp+p38],
			tptr->re,tptr->im,(tptr+1)->re,(tptr+1)->im,(tptr+2)->re,(tptr+2)->im,(tptr+3)->re,(tptr+3)->im,(tptr+4)->re,(tptr+4)->im,(tptr+5)->re,(tptr+5)->im,(tptr+6)->re,(tptr+6)->im,(tptr+7)->re,(tptr+7)->im
		);
		tptr += 8;
	}

/*...and now do eight radix-8 subtransforms w/internal twiddles - cf. radix64_dif_pass1 for details: */

	/* Block 0: */ tptr = t;	jt = j1;	jp = j2;
	/* 0-index block has all-unity twiddles: Remember, the twiddleless DIF bit-reverses both its in-and-outputs,
	so swap index-offset pairs 1/4 and 3/6 in t*-inputs and a-outputs: */
	RADIX_08_DIF_OOP(
		tptr->re,tptr->im,(tptr+0x20)->re,(tptr+0x20)->im,(tptr+0x10)->re,(tptr+0x10)->im,(tptr+0x30)->re,(tptr+0x30)->im,(tptr+0x08)->re,(tptr+0x08)->im,(tptr+0x28)->re,(tptr+0x28)->im,(tptr+0x18)->re,(tptr+0x18)->im,(tptr+0x38)->re,(tptr+0x38)->im,
		a[jt],a[jp],a[jt+p04],a[jp+p04],a[jt+p02],a[jp+p02],a[jt+p06],a[jp+p06],a[jt+p01],a[jp+p01],a[jt+p05],a[jp+p05],a[jt+p03],a[jp+p03],a[jt+p07],a[jp+p07]
	);

	// Remaining 7 sets of macro calls done in loop:
	for(l = 1; l < 8; l++) {
		tptr = t + reverse(l,8);
		k2 = poff[l+l];	// poffs[2*l] = p00,08,10,...,30,38
		jt = j1 + k2; jp = j2 + k2;
		addr = DFT64_TWIDDLES[l-1]; addi = addr+1;	// Pointer to required row of 2-D twiddles array
		RADIX_08_DIF_TWIDDLE_OOP(
			tptr->re,tptr->im,(tptr+0x08)->re,(tptr+0x08)->im,(tptr+0x10)->re,(tptr+0x10)->im,(tptr+0x18)->re,(tptr+0x18)->im,(tptr+0x20)->re,(tptr+0x20)->im,(tptr+0x28)->re,(tptr+0x28)->im,(tptr+0x30)->re,(tptr+0x30)->im,(tptr+0x38)->re,(tptr+0x38)->im,
			a[jt],a[jp],a[jt+p01],a[jp+p01],a[jt+p02],a[jp+p02],a[jt+p03],a[jp+p03],a[jt+p04],a[jp+p04],a[jt+p05],a[jp+p05],a[jt+p06],a[jp+p06],a[jt+p07],a[jp+p07],
			*(addr+0x00),*(addi+0x00), *(addr+0x02),*(addi+0x02), *(addr+0x04),*(addi+0x04), *(addr+0x06),*(addi+0x06), *(addr+0x08),*(addi+0x08), *(addr+0x0a),*(addi+0x0a), *(addr+0x0c),*(addi+0x0c)
		);
	}

#endif	/* #ifdef USE_SSE2 */

	}	/* end for(j=_jstart[ithread]; j < _jhi[ithread]; j += 2) */

	if(MODULUS_TYPE == MODULUS_TYPE_MERSENNE)
	{
		jstart += nwt;
		jhi    += nwt;

		col += RADIX;
		co3 -= RADIX;
	}
}	/* end for(k=1; k <= khi; k++) */

