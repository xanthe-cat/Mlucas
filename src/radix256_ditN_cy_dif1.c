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

#include "Mlucas.h"
#include "radix256.h"

#define RADIX 256	// Use #define rather than const int to ensure it's really a compile-time const in the C sense

#define EPS 1e-10

#define USE_SCALAR_DFT_MACRO	0

// Mersenne-mod takes a binary-toggle LOACC; must give a numerical value for Fermat-mod:
#if defined(HIACC) && defined(LOACC)
	#error Only one of LOACC and HIACC may be defined!
#endif
#if !defined(HIACC) && !defined(LOACC)
  #if OS_BITS == 64
	#define LOACC	1	// Default is suitable for F29 work @ FFT length 30M
	#warning LOACC = 1
  #else
	#define HIACC	1	// 32-bit mode only supports the older HIACC carry macros
  #endif
#endif
#if defined(HIACC) && defined(USE_AVX512)
	#error Currently only LOACC carry-mode supported in AVX-512 builds!
#endif
#if defined(LOACC) && (OS_BITS == 32)
	#error 32-bit mode only supports the older HIACC carry macros!
#endif

#ifndef PFETCH_DIST
  #ifdef USE_AVX512
	#define PFETCH_DIST	64	// Feb 2017: Test on KNL point to this as best
  #elif defined(USE_AVX)
	#define PFETCH_DIST	32	// This seems to work best on my Haswell, even though 64 bytes seems more logical in AVX mode
  #else
	#define PFETCH_DIST	32
  #endif
#endif

// SIMD+SSE2 code only available for GCC build:
#if defined(USE_SSE2) && defined(COMPILER_TYPE_GCC)

	#include "sse2_macro.h"

  // For Mersenne-mod need (16 [SSE2] or 64 [AVX]) + (4 [HIACC] or 40 [LOACC]) added slots for half_arr lookup tables.
  // Max = (40 [SSE2]; 128 [AVX]),
  // For Fermat-mod we use RADIX*4 = 1024 [note there is no Fermat-mod LOACC option for this power-of-2 DFT] more
  // slots in AVX mode for the compact negacyclic-roots chained-multiply scheme. Add larger of the 2 numbers -
  // 1024 for AVX, 40 for SSE2 - to (half_arr_offset256 + RADIX) to get SIMD value of radix256_creals_in_local_store:
  #ifdef USE_AVX512
	const int half_arr_offset256 = 0x628;	// 0x40 = 2*(RADIX/8) fewer cy-slots than in AVX mode
	const int radix256_creals_in_local_store = 0xb28;
  #elif defined(USE_AVX)
	const int half_arr_offset256 = 0x668;	// + 5*RADIX = 0xb68; Used for thread local-storage-integrity checking
	const int radix256_creals_in_local_store = 0xb68;	// (half_arr_offset256 + 5*RADIX) and round up to nearest multiple of 8
  #else
	const int half_arr_offset256 = 0x6e8;	// + RADIX = 0x8e8; Used for thread local-storage-integrity checking
	const int radix256_creals_in_local_store = 0x910;	// (half_arr_offset256 + RADIX) + 0x28 and round up to nearest multiple of 8
  #endif

  #ifdef USE_AVX
	const uint64 radix256_avx_negadwt_consts[RADIX] = {	// 8 entries per line ==> RADIX/8 lines:
		0x3FF0000000000000ull,0x3FEFFFD8858E8A92ull,0x3FEFFF62169B92DBull,0x3FEFFE9CB44B51A1ull,0x3FEFFD886084CD0Dull,0x3FEFFC251DF1D3F8ull,0x3FEFFA72EFFEF75Dull,0x3FEFF871DADB81DFull,
		0x3FEFF621E3796D7Eull,0x3FEFF3830F8D575Cull,0x3FEFF095658E71ADull,0x3FEFED58ECB673C4ull,0x3FEFE9CDAD01883Aull,0x3FEFE5F3AF2E3940ull,0x3FEFE1CAFCBD5B09ull,0x3FEFDD539FF1F456ull,
		0x3FEFD88DA3D12526ull,0x3FEFD37914220B84ull,0x3FEFCE15FD6DA67Bull,0x3FEFC8646CFEB721ull,0x3FEFC26470E19FD3ull,0x3FEFBC1617E44186ull,0x3FEFB5797195D741ull,0x3FEFAE8E8E46CFBBull,
		0x3FEFA7557F08A517ull,0x3FEF9FCE55ADB2C8ull,0x3FEF97F924C9099Bull,0x3FEF8FD5FFAE41DBull,0x3FEF8764FA714BA9ull,0x3FEF7EA629E63D6Eull,0x3FEF7599A3A12077ull,0x3FEF6C3F7DF5BBB7ull,
		0x3FEF6297CFF75CB0ull,0x3FEF58A2B1789E84ull,0x3FEF4E603B0B2F2Dull,0x3FEF43D085FF92DDull,0x3FEF38F3AC64E589ull,0x3FEF2DC9C9089A9Dull,0x3FEF2252F7763ADAull,0x3FEF168F53F7205Dull,
		0x3FEF0A7EFB9230D7ull,0x3FEEFE220C0B95ECull,0x3FEEF178A3E473C2ull,0x3FEEE482E25A9DBCull,0x3FEED740E7684963ull,0x3FEEC9B2D3C3BF84ull,0x3FEEBBD8C8DF0B74ull,0x3FEEADB2E8E7A88Eull,
		0x3FEE9F4156C62DDAull,0x3FEE9084361DF7F2ull,0x3FEE817BAB4CD10Dull,0x3FEE7227DB6A9744ull,0x3FEE6288EC48E112ull,0x3FEE529F04729FFCull,0x3FEE426A4B2BC17Eull,0x3FEE31EAE870CE25ull,
		0x3FEE212104F686E5ull,0x3FEE100CCA2980ACull,0x3FEDFEAE622DBE2Bull,0x3FEDED05F7DE47DAull,0x3FEDDB13B6CCC23Cull,0x3FEDC8D7CB410260ull,0x3FEDB6526238A09Bull,0x3FEDA383A9668988ull,
		0x3FED906BCF328D46ull,0x3FED7D0B02B8ECF9ull,0x3FED696173C9E68Bull,0x3FED556F52E93EB1ull,0x3FED4134D14DC93Aull,0x3FED2CB220E0EF9Full,0x3FED17E7743E35DCull,0x3FED02D4FEB2BD92ull,
		0x3FECED7AF43CC773ull,0x3FECD7D9898B32F6ull,0x3FECC1F0F3FCFC5Cull,0x3FECABC169A0B900ull,0x3FEC954B213411F5ull,0x3FEC7E8E52233CF3ull,0x3FEC678B3488739Bull,0x3FEC5042012B6907ull,
		0x3FEC38B2F180BDB1ull,0x3FEC20DE3FA971B0ull,0x3FEC08C426725549ull,0x3FEBF064E15377DDull,0x3FEBD7C0AC6F952Aull,0x3FEBBED7C49380EAull,0x3FEBA5AA673590D2ull,0x3FEB8C38D27504E9ull,
		0x3FEB728345196E3Eull,0x3FEB5889FE921405ull,0x3FEB3E4D3EF55712ull,0x3FEB23CD470013B4ull,0x3FEB090A58150200ull,0x3FEAEE04B43C1474ull,0x3FEAD2BC9E21D511ull,0x3FEAB7325916C0D4ull,
		0x3FEA9B66290EA1A3ull,0x3FEA7F58529FE69Dull,0x3FEA63091B02FAE2ull,0x3FEA4678C8119AC8ull,0x3FEA29A7A0462782ull,0x3FEA0C95EABAF937ull,0x3FE9EF43EF29AF94ull,0x3FE9D1B1F5EA80D5ull,
		0x3FE9B3E047F38741ull,0x3FE995CF2ED80D22ull,0x3FE9777EF4C7D742ull,0x3FE958EFE48E6DD7ull,0x3FE93A22499263FBull,0x3FE91B166FD49DA2ull,0x3FE8FBCCA3EF940Dull,0x3FE8DC45331698CCull,
		0x3FE8BC806B151741ull,0x3FE89C7E9A4DD4AAull,0x3FE87C400FBA2EBFull,0x3FE85BC51AE958CCull,0x3FE83B0E0BFF976Eull,0x3FE81A1B33B57ACCull,0x3FE7F8ECE3571771ull,0x3FE7D7836CC33DB2ull,
		0x3FE7B5DF226AAFAFull,0x3FE79400574F55E5ull,0x3FE771E75F037261ull,0x3FE74F948DA8D28Dull,0x3FE72D0837EFFF96ull,0x3FE70A42B3176D7Aull,0x3FE6E74454EAA8AFull,0x3FE6C40D73C18275ull,
		0x3FE6A09E667F3BCDull,0x3FE67CF78491AF10ull,0x3FE6591925F0783Dull,0x3FE63503A31C1BE9ull,0x3FE610B7551D2CDFull,0x3FE5EC3495837074ull,0x3FE5C77BBE65018Cull,0x3FE5A28D2A5D7250ull,
		0x3FE57D69348CECA0ull,0x3FE5581038975137ull,0x3FE5328292A35596ull,0x3FE50CC09F59A09Bull,0x3FE4E6CABBE3E5E9ull,0x3FE4C0A145EC0004ull,0x3FE49A449B9B0939ull,0x3FE473B51B987347ull,
		0x3FE44CF325091DD6ull,0x3FE425FF178E6BB1ull,0x3FE3FED9534556D4ull,0x3FE3D78238C58344ull,0x3FE3AFFA292050B9ull,0x3FE3884185DFEB22ull,0x3FE36058B10659F3ull,0x3FE338400D0C8E57ull,
		0x3FE30FF7FCE17035ull,0x3FE2E780E3E8EA17ull,0x3FE2BEDB25FAF3EAull,0x3FE2960727629CA8ull,0x3FE26D054CDD12DFull,0x3FE243D5FB98AC1Full,0x3FE21A799933EB59ull,0x3FE1F0F08BBC861Bull,
		0x3FE1C73B39AE68C8ull,0x3FE19D5A09F2B9B8ull,0x3FE1734D63DEDB49ull,0x3FE14915AF336CEBull,0x3FE11EB3541B4B23ull,0x3FE0F426BB2A8E7Eull,0x3FE0C9704D5D898Full,0x3FE09E907417C5E1ull,
		0x3FE073879922FFEEull,0x3FE0485626AE221Aull,0x3FE01CFC874C3EB7ull,0x3FDFE2F64BE71210ull,0x3FDF8BA4DBF89ABAull,0x3FDF3405963FD067ull,0x3FDEDC1952EF78D6ull,0x3FDE83E0EAF85114ull,
		0x3FDE2B5D3806F63Bull,0x3FDDD28F1481CC58ull,0x3FDD79775B86E389ull,0x3FDD2016E8E9DB5Bull,0x3FDCC66E9931C45Eull,0x3FDC6C7F4997000Bull,0x3FDC1249D8011EE7ull,0x3FDBB7CF2304BD01ull,
		0x3FDB5D1009E15CC0ull,0x3FDB020D6C7F4009ull,0x3FDAA6C82B6D3FCAull,0x3FDA4B4127DEA1E5ull,0x3FD9EF7943A8ED8Aull,0x3FD993716141BDFFull,0x3FD9372A63BC93D7ull,0x3FD8DAA52EC8A4B0ull,
		0x3FD87DE2A6AEA963ull,0x3FD820E3B04EAAC4ull,0x3FD7C3A9311DCCE7ull,0x3FD766340F2418F6ull,0x3FD7088530FA459Full,0x3FD6AA9D7DC77E17ull,0x3FD64C7DDD3F27C6ull,0x3FD5EE27379EA693ull,
		0x3FD58F9A75AB1FDDull,0x3FD530D880AF3C24ull,0x3FD4D1E24278E76Aull,0x3FD472B8A5571054ull,0x3FD4135C94176601ull,0x3FD3B3CEFA0414B7ull,0x3FD35410C2E18152ull,0x3FD2F422DAEC0387ull,
		0x3FD294062ED59F06ull,0x3FD233BBABC3BB71ull,0x3FD1D3443F4CDB3Eull,0x3FD172A0D7765177ull,0x3FD111D262B1F677ull,0x3FD0B0D9CFDBDB90ull,0x3FD04FB80E37FDAEull,0x3FCFDCDC1ADFEDF9ull,
		0x3FCF19F97B215F1Bull,0x3FCE56CA1E101A1Bull,0x3FCD934FE5454311ull,0x3FCCCF8CB312B286ull,0x3FCC0B826A7E4F63ull,0x3FCB4732EF3D6722ull,0x3FCA82A025B00451ull,0x3FC9BDCBF2DC4366ull,
		0x3FC8F8B83C69A60Bull,0x3FC83366E89C64C6ull,0x3FC76DD9DE50BF31ull,0x3FC6A81304F64AB2ull,0x3FC5E214448B3FC6ull,0x3FC51BDF8597C5F2ull,0x3FC45576B1293E5Aull,0x3FC38EDBB0CD8D14ull,
		0x3FC2C8106E8E613Aull,0x3FC20116D4EC7BCFull,0x3FC139F0CEDAF577ull,0x3FC072A047BA831Dull,0x3FBF564E56A9730Eull,0x3FBDC70ECBAE9FC9ull,0x3FBC3785C79EC2D5ull,0x3FBAA7B724495C03ull,
		0x3FB917A6BC29B42Cull,0x3FB787586A5D5B21ull,0x3FB5F6D00A9AA419ull,0x3FB4661179272096ull,0x3FB2D52092CE19F6ull,0x3FB1440134D709B3ull,0x3FAF656E79F820E0ull,0x3FAC428D12C0D7E3ull,
		0x3FA91F65F10DD814ull,0x3FA5FC00D290CD43ull,0x3FA2D865759455CDull,0x3F9F693731D1CF01ull,0x3F992155F7A3667Eull,0x3F92D936BBE30EFDull,0x3F8921D1FCDEC784ull,0x3F7921F0FE670071ull
	};
  #endif

#elif defined(USE_SSE2)

	#error SIMD build only supported for GCC-compatible compiler under *nix/macOS!

#endif	/* USE_SSE2 */

#ifdef USE_PTHREAD

	// Use non-pooled simple spawn/rejoin thread-team model
	#include "threadpool.h"

	struct cy_thread_data_t{
	// int data - if needed, pad to yield an even number of these:
		int iter;
		int tid;
		int ndivr;

		int khi;
		int i;
		int jstart;
		int jhi;
		int col;
		int co2;
		int co3;
		int sw;
		int nwt;

	// double data:
		double maxerr;
		double scale;

	// pointer data:
		double *arrdat;			/* Main data array */
		double *wt0;
		double *wt1;
	#ifdef LOACC
		double *wts_mult, *inv_mult;
	#endif
		int *si;
		struct complex *rn0;
		struct complex *rn1;
	#ifdef USE_SSE2
		vec_dbl *r00;
		vec_dbl *half_arr;
	#else
		double *r00;
		double *half_arr;
	#endif
		uint64*sm_ptr;
		uint32 bjmodnini;
		int bjmodn0;
	// For large radix0 use thread-local arrays for DWT indices/carries - only caveat is these must be SIMD-aligned:
	// Since GCC uses __BIGGEST_ALIGNMENT__ = 16 on x86, which is too small to be useful for avx data,
	// we are forced to resort to fugly hackage - add pad slots to a garbage-named struct-internal array along with
	// a pointer-to-be-inited-at-runtime, when we set ptr to the lowest-index array element having the desired alginment:
		double *cy_r,*cy_i;
	#ifdef USE_AVX512
		double cy_dat[2*RADIX+8] __attribute__ ((__aligned__(8)));
	#else
		double cy_dat[2*RADIX+4] __attribute__ ((__aligned__(8)));	// Enforce min-alignment of 8 bytes in 32-bit builds.
	#endif
	};

#endif

/***************/

int radix256_ditN_cy_dif1(double a[], int n, int nwt, int nwt_bits, double wt0[], double wt1[], int si[], struct complex rn0[], struct complex rn1[], double base[], double baseinv[], int iter, double *fracmax, uint64 p)
{
/*
!...Acronym: DWT = Discrete Weighted Transform, DIT = Decimation In Time, DIF = Decimation In Frequency
!
!...Performs a final radix-256 complex DIT pass, an inverse DWT weighting, a carry propagation,
!   a forward DWT weighting, and an initial radix-256 complex DIF pass on the data in the length-N real vector A.
!
!   Data enter and are returned in the A-array.
!
!   See the documentation in mers_mod_square and radix16_dif_pass for further details on the array
!   storage scheme, and radix16_ditN_cy_dif1 for details on the reduced-length weights array scheme.
*/
#if !defined(MULTITHREAD) || defined(USE_SSE2)
	#include "radix256_twiddles.h"
#endif
	const char func[] = "radix256_ditN_cy_dif1";
	static int poff[RADIX>>2];
#if USE_SCALAR_DFT_MACRO
	static int dft_offsets_lo[16], dft_offsets_hi[16];
#endif
#if !USE_SCALAR_DFT_MACRO || !defined(USE_SSE2)	// SIMD build uses these for radix-256 DFTs; Scalar-double build uses these for carry-macro loops
	static int po_br[16];
#endif
	// FMA-based DFT needs the tangent:
#ifdef USE_AVX2
	static double tan = 0.41421356237309504879;
#elif defined(USE_FMA)
	static double tan = 0.41421356237309504879,
	//	c1_1,s1_1,c1_2,s1_2,c1_3,s1_3,c1_4,s1_4,c1_5,s1_5,c1_6,s1_6,c1_7,s1_7,c1_8,s1_8,c1_9,s1_9,c1_10,s1_10,c1_11,s1_11,c1_12,s1_12,c1_13,s1_13,c1_14,s1_14,c1_15,s1_15,c1_1_c, c1_1i2,c1_2i2,	see code below for why this 1st set not used
		c2_1,s2_1,c2_2,s2_2,c2_3,s2_3,c2_4,s2_4,c2_5,s2_5,c2_6,s2_6,c2_7,s2_7,c2_8,s2_8,c2_9,s2_9,c2_10,s2_10,c2_11,s2_11,c2_12,s2_12,c2_13,s2_13,c2_14,s2_14,c2_15,s2_15,c2_1_c, c2_1i2,c2_2i2,
		c3_1,s3_1,c3_2,s3_2,c3_3,s3_3,c3_4,s3_4,c3_5,s3_5,c3_6,s3_6,c3_7,s3_7,c3_8,s3_8,c3_9,s3_9,c3_10,s3_10,c3_11,s3_11,c3_12,s3_12,c3_13,s3_13,c3_14,s3_14,c3_15,s3_15,c3_1_c, c3_1i2,c3_2i2,
		c4_1,s4_1,c4_2,s4_2,c4_3,s4_3,c4_4,s4_4,c4_5,s4_5,c4_6,s4_6,c4_7,s4_7,c4_8,s4_8,c4_9,s4_9,c4_10,s4_10,c4_11,s4_11,c4_12,s4_12,c4_13,s4_13,c4_14,s4_14,c4_15,s4_15,c4_1_c, c4_1i2,c4_2i2,
		c5_1,s5_1,c5_2,s5_2,c5_3,s5_3,c5_4,s5_4,c5_5,s5_5,c5_6,s5_6,c5_7,s5_7,c5_8,s5_8,c5_9,s5_9,c5_10,s5_10,c5_11,s5_11,c5_12,s5_12,c5_13,s5_13,c5_14,s5_14,c5_15,s5_15,c5_1_c, c5_1i2,c5_2i2,
		c6_1,s6_1,c6_2,s6_2,c6_3,s6_3,c6_4,s6_4,c6_5,s6_5,c6_6,s6_6,c6_7,s6_7,c6_8,s6_8,c6_9,s6_9,c6_10,s6_10,c6_11,s6_11,c6_12,s6_12,c6_13,s6_13,c6_14,s6_14,c6_15,s6_15,c6_1_c, c6_1i2,c6_2i2,
		c7_1,s7_1,c7_2,s7_2,c7_3,s7_3,c7_4,s7_4,c7_5,s7_5,c7_6,s7_6,c7_7,s7_7,c7_8,s7_8,c7_9,s7_9,c7_10,s7_10,c7_11,s7_11,c7_12,s7_12,c7_13,s7_13,c7_14,s7_14,c7_15,s7_15,c7_1_c, c7_1i2,c7_2i2,
		c8_1,s8_1,c8_2,s8_2,c8_3,s8_3,c8_4,s8_4,c8_5,s8_5,c8_6,s8_6,c8_7,s8_7,c8_8,s8_8,c8_9,s8_9,c8_10,s8_10,c8_11,s8_11,c8_12,s8_12,c8_13,s8_13,c8_14,s8_14,c8_15,s8_15,c8_1_c, c8_1i2,c8_2i2,
		c9_1,s9_1,c9_2,s9_2,c9_3,s9_3,c9_4,s9_4,c9_5,s9_5,c9_6,s9_6,c9_7,s9_7,c9_8,s9_8,c9_9,s9_9,c9_10,s9_10,c9_11,s9_11,c9_12,s9_12,c9_13,s9_13,c9_14,s9_14,c9_15,s9_15,c9_1_c, c9_1i2,c9_2i2,
		ca_1,sa_1,ca_2,sa_2,ca_3,sa_3,ca_4,sa_4,ca_5,sa_5,ca_6,sa_6,ca_7,sa_7,ca_8,sa_8,ca_9,sa_9,ca_10,sa_10,ca_11,sa_11,ca_12,sa_12,ca_13,sa_13,ca_14,sa_14,ca_15,sa_15,ca_1_c, ca_1i2,ca_2i2,
		cb_1,sb_1,cb_2,sb_2,cb_3,sb_3,cb_4,sb_4,cb_5,sb_5,cb_6,sb_6,cb_7,sb_7,cb_8,sb_8,cb_9,sb_9,cb_10,sb_10,cb_11,sb_11,cb_12,sb_12,cb_13,sb_13,cb_14,sb_14,cb_15,sb_15,cb_1_c, cb_1i2,cb_2i2,
		cc_1,sc_1,cc_2,sc_2,cc_3,sc_3,cc_4,sc_4,cc_5,sc_5,cc_6,sc_6,cc_7,sc_7,cc_8,sc_8,cc_9,sc_9,cc_10,sc_10,cc_11,sc_11,cc_12,sc_12,cc_13,sc_13,cc_14,sc_14,cc_15,sc_15,cc_1_c, cc_1i2,cc_2i2,
		cd_1,sd_1,cd_2,sd_2,cd_3,sd_3,cd_4,sd_4,cd_5,sd_5,cd_6,sd_6,cd_7,sd_7,cd_8,sd_8,cd_9,sd_9,cd_10,sd_10,cd_11,sd_11,cd_12,sd_12,cd_13,sd_13,cd_14,sd_14,cd_15,sd_15,cd_1_c, cd_1i2,cd_2i2,
		ce_1,se_1,ce_2,se_2,ce_3,se_3,ce_4,se_4,ce_5,se_5,ce_6,se_6,ce_7,se_7,ce_8,se_8,ce_9,se_9,ce_10,se_10,ce_11,se_11,ce_12,se_12,ce_13,se_13,ce_14,se_14,ce_15,se_15,ce_1_c, ce_1i2,ce_2i2,
		cf_1,sf_1,cf_2,sf_2,cf_3,sf_3,cf_4,sf_4,cf_5,sf_5,cf_6,sf_6,cf_7,sf_7,cf_8,sf_8,cf_9,sf_9,cf_10,sf_10,cf_11,sf_11,cf_12,sf_12,cf_13,sf_13,cf_14,sf_14,cf_15,sf_15,cf_1_c, cf_1i2,cf_2i2;
#endif
	const int pfetch_dist = PFETCH_DIST;
	const int stride = (int)RE_IM_STRIDE << 1;	// main-array loop stride = 2*RE_IM_STRIDE
#ifdef USE_SSE2
	const int sz_vd = sizeof(vec_dbl);
	// lg(sizeof(vec_dbl)):
  #ifdef USE_AVX512
	const int l2_sz_vd = 6;
  #elif defined(USE_AVX)
	const int l2_sz_vd = 5;
  #else
	const int l2_sz_vd = 4;
  #endif
#else
	const int sz_vd = sizeof(double);
#endif
	const int sz_vd_m1 = sz_vd-1;
  #ifdef USE_AVX512
	const int jhi_wrap_mers = 15;
	const int jhi_wrap_ferm = 15;
  #else
	const int jhi_wrap_mers =  7;
	const int jhi_wrap_ferm = 15;	// For right-angle transform need *complex* elements for wraparound, so jhi needs to be twice as large
  #endif
	int NDIVR,i,j,j1,j2,jt,jp,jstart,jhi,full_pass,k,khi,l,ntmp,outer,nbytes;
	int k1,k2;
	int col,co2,co3;
  #ifdef USE_AVX512
	double t0,t1,t2,t3;
   #ifdef CARRY_16_WAY
	static struct uint32x16 *n_minus_sil,*n_minus_silp1,*sinwt,*sinwtm1;
   #else
	static struct uint32x8  *n_minus_sil,*n_minus_silp1,*sinwt,*sinwtm1;
   #endif
  #elif defined(USE_AVX)
	static struct uint32x4 *n_minus_sil,*n_minus_silp1,*sinwt,*sinwtm1;
  #else
	int n_minus_sil,n_minus_silp1,sinwt,sinwtm1;
	double wtl,wtlp1,wtn,wtnm1;	/* Mersenne-mod weights stuff */
  #endif
  #ifdef LOACC
	static double wts_mult[2], inv_mult[2];	// Const wts-multiplier and 2*(its multiplicative inverse)
  #endif
	double rt,it, wt_re,wt_im, wi_re,wi_im;	// Fermat-mod weights stuff, used in both scalar and AVX mode
	static uint32 bjmodnini;
	static uint64 psave = 0;
	static uint32 bw,sw,nm1,
		p01,p02,p03,p04,p05,p06,p07,p08,p09,p0a,p0b,p0c,p0d,p0e,p0f,
		p10,p20,p30,p40,p50,p60,p70,p80,p90,pa0,pb0,pc0,pd0,pe0,pf0, nsave = 0;
	static double radix_inv, n2inv;
	double scale, dtmp, maxerr = 0.0;
	// Local storage: We must use an array here because scalars have no guarantees about relative address offsets
	// [and even if those are contiguous-as-hoped-for, they may run in reverse]; Make array type (struct complex)
	// to allow us to use the same offset-indexing as in the original radix-32 in-place DFT macros:
	struct complex t[RADIX], *tptr;
	double *addr,*addi;
	int *itmp,*itm2;	// Pointer into the bjmodn array
	int err;
	static int first_entry=TRUE;

	int n_div_nwt;

#ifdef USE_SSE2

	int idx_offset,idx_incr;
	static int cslots_in_local_store;
	static vec_dbl *sc_arr = 0x0, *sc_ptr;
	static uint64 *sm_ptr, *sign_mask, *sse_bw, *sse_sw, *sse_nm1;
	uint64 tmp64;

  #ifdef MULTITHREAD
	static vec_dbl *__r0;	// Base address for discrete per-thread local stores
  #else
	double *add0, *add1, *add2, *add3;	// Latter 3 also used for carries
   #if !USE_SCALAR_DFT_MACRO
	double *add4, *add5, *add6, *add7, *add8, *add9, *adda, *addb, *addc, *addd, *adde, *addf;
   #endif
  #endif	// MULTITHREAD

	static int *bjmodn;	// Alloc mem for this along with other 	SIMD stuff
	const double crnd = 3.0*0x4000000*0x2000000;
	struct complex *ctmp;	// Hybrid AVX-DFT/SSE2-carry scheme used for Mersenne-mod needs a 2-word-double pointer
	// Uint64 bitmaps for alternate "rounded the other way" copies of sqrt2,isrt2. Default round-to-nearest versions
	// (SQRT2, ISRT2) end in ...3BCD. Since we round these down as ...3BCC90... --> ..3BCC, append _dn to varnames:
	const uint64 sqrt2_dn = 0x3FF6A09E667F3BCCull, isrt2_dn = 0x3FE6A09E667F3BCCull;
	static vec_dbl *max_err, *sse2_rnd, *half_arr, *two,*one,*sqrt2,*isrt2, *cc0,*ss0,
		// ptrs to 16 sets of twiddles shared by the 2nd-half DIF and DIT DFT macros:
		*twid0,*twid1,*twid2,*twid3,*twid4,*twid5,*twid6,*twid7,*twid8,*twid9,*twida,*twidb,*twidc,*twidd,*twide,*twidf,
		*r00,*r01,*r02,*r03,*r04,*r05,*r06,*r07,*r08,*r09,*r0a,*r0b,*r0c,*r0d,*r0e,*r0f,
		*r10,*r11,*r12,*r13,*r14,*r15,*r16,*r17,*r18,*r19,*r1a,*r1b,*r1c,*r1d,*r1e,*r1f,
		*r20,*r21,*r22,*r23,*r24,*r25,*r26,*r27,*r28,*r29,*r2a,*r2b,*r2c,*r2d,*r2e,*r2f,
		*r30,*r31,*r32,*r33,*r34,*r35,*r36,*r37,*r38,*r39,*r3a,*r3b,*r3c,*r3d,*r3e,*r3f,
		*r40,*r41,*r42,*r43,*r44,*r45,*r46,*r47,*r48,*r49,*r4a,*r4b,*r4c,*r4d,*r4e,*r4f,
		*r50,*r51,*r52,*r53,*r54,*r55,*r56,*r57,*r58,*r59,*r5a,*r5b,*r5c,*r5d,*r5e,*r5f,
		*r60,*r61,*r62,*r63,*r64,*r65,*r66,*r67,*r68,*r69,*r6a,*r6b,*r6c,*r6d,*r6e,*r6f,
		*r70,*r71,*r72,*r73,*r74,*r75,*r76,*r77,*r78,*r79,*r7a,*r7b,*r7c,*r7d,*r7e,*r7f,
		*r80,*r81,*r82,*r83,*r84,*r85,*r86,*r87,*r88,*r89,*r8a,*r8b,*r8c,*r8d,*r8e,*r8f,
		*r90,*r91,*r92,*r93,*r94,*r95,*r96,*r97,*r98,*r99,*r9a,*r9b,*r9c,*r9d,*r9e,*r9f,
		*ra0,*ra1,*ra2,*ra3,*ra4,*ra5,*ra6,*ra7,*ra8,*ra9,*raa,*rab,*rac,*rad,*rae,*raf,
		*rb0,*rb1,*rb2,*rb3,*rb4,*rb5,*rb6,*rb7,*rb8,*rb9,*rba,*rbb,*rbc,*rbd,*rbe,*rbf,
		*rc0,*rc1,*rc2,*rc3,*rc4,*rc5,*rc6,*rc7,*rc8,*rc9,*rca,*rcb,*rcc,*rcd,*rce,*rcf,
		*rd0,*rd1,*rd2,*rd3,*rd4,*rd5,*rd6,*rd7,*rd8,*rd9,*rda,*rdb,*rdc,*rdd,*rde,*rdf,
		*re0,*re1,*re2,*re3,*re4,*re5,*re6,*re7,*re8,*re9,*rea,*reb,*rec,*red,*ree,*ref,
		*rf0,*rf1,*rf2,*rf3,*rf4,*rf5,*rf6,*rf7,*rf8,*rf9,*rfa,*rfb,*rfc,*rfd,*rfe,*rff,
		*s1p00,*s1p01,*s1p02,*s1p03,*s1p04,*s1p05,*s1p06,*s1p07,*s1p08,*s1p09,*s1p0a,*s1p0b,*s1p0c,*s1p0d,*s1p0e,*s1p0f,
		*s1p10,*s1p11,*s1p12,*s1p13,*s1p14,*s1p15,*s1p16,*s1p17,*s1p18,*s1p19,*s1p1a,*s1p1b,*s1p1c,*s1p1d,*s1p1e,*s1p1f,
		*s1p20,*s1p21,*s1p22,*s1p23,*s1p24,*s1p25,*s1p26,*s1p27,*s1p28,*s1p29,*s1p2a,*s1p2b,*s1p2c,*s1p2d,*s1p2e,*s1p2f,
		*s1p30,*s1p31,*s1p32,*s1p33,*s1p34,*s1p35,*s1p36,*s1p37,*s1p38,*s1p39,*s1p3a,*s1p3b,*s1p3c,*s1p3d,*s1p3e,*s1p3f,
		*s1p40,*s1p41,*s1p42,*s1p43,*s1p44,*s1p45,*s1p46,*s1p47,*s1p48,*s1p49,*s1p4a,*s1p4b,*s1p4c,*s1p4d,*s1p4e,*s1p4f,
		*s1p50,*s1p51,*s1p52,*s1p53,*s1p54,*s1p55,*s1p56,*s1p57,*s1p58,*s1p59,*s1p5a,*s1p5b,*s1p5c,*s1p5d,*s1p5e,*s1p5f,
		*s1p60,*s1p61,*s1p62,*s1p63,*s1p64,*s1p65,*s1p66,*s1p67,*s1p68,*s1p69,*s1p6a,*s1p6b,*s1p6c,*s1p6d,*s1p6e,*s1p6f,
		*s1p70,*s1p71,*s1p72,*s1p73,*s1p74,*s1p75,*s1p76,*s1p77,*s1p78,*s1p79,*s1p7a,*s1p7b,*s1p7c,*s1p7d,*s1p7e,*s1p7f,
		*s1p80,*s1p81,*s1p82,*s1p83,*s1p84,*s1p85,*s1p86,*s1p87,*s1p88,*s1p89,*s1p8a,*s1p8b,*s1p8c,*s1p8d,*s1p8e,*s1p8f,
		*s1p90,*s1p91,*s1p92,*s1p93,*s1p94,*s1p95,*s1p96,*s1p97,*s1p98,*s1p99,*s1p9a,*s1p9b,*s1p9c,*s1p9d,*s1p9e,*s1p9f,
		*s1pa0,*s1pa1,*s1pa2,*s1pa3,*s1pa4,*s1pa5,*s1pa6,*s1pa7,*s1pa8,*s1pa9,*s1paa,*s1pab,*s1pac,*s1pad,*s1pae,*s1paf,
		*s1pb0,*s1pb1,*s1pb2,*s1pb3,*s1pb4,*s1pb5,*s1pb6,*s1pb7,*s1pb8,*s1pb9,*s1pba,*s1pbb,*s1pbc,*s1pbd,*s1pbe,*s1pbf,
		*s1pc0,*s1pc1,*s1pc2,*s1pc3,*s1pc4,*s1pc5,*s1pc6,*s1pc7,*s1pc8,*s1pc9,*s1pca,*s1pcb,*s1pcc,*s1pcd,*s1pce,*s1pcf,
		*s1pd0,*s1pd1,*s1pd2,*s1pd3,*s1pd4,*s1pd5,*s1pd6,*s1pd7,*s1pd8,*s1pd9,*s1pda,*s1pdb,*s1pdc,*s1pdd,*s1pde,*s1pdf,
		*s1pe0,*s1pe1,*s1pe2,*s1pe3,*s1pe4,*s1pe5,*s1pe6,*s1pe7,*s1pe8,*s1pe9,*s1pea,*s1peb,*s1pec,*s1ped,*s1pee,*s1pef,
		*s1pf0,*s1pf1,*s1pf2,*s1pf3,*s1pf4,*s1pf5,*s1pf6,*s1pf7,*s1pf8,*s1pf9,*s1pfa,*s1pfb,*s1pfc,*s1pfd,*s1pfe,*s1pff,
		*cy_r,*cy_i;	// Need RADIX slots for sse2 carries, RADIX/2 for avx
  #ifdef USE_AVX
	static vec_dbl *base_negacyclic_root;
  #endif

	vec_dbl *tmp,*tm0,*tm1,*tm2;	// Non-static utility ptrs

#endif	// USE_SSE2

#ifdef MULTITHREAD

	static struct cy_thread_data_t *tdat = 0x0;
	// Threadpool-based dispatch stuff:
	static int main_work_units = 0, pool_work_units = 0;
	static struct threadpool *tpool = 0x0;
	static int task_is_blocking = TRUE;
	static thread_control_t thread_control = {0,0,0};
	// First 3 subfields same for all threads, 4th provides thread-specifc data, will be inited at thread dispatch:
	static task_control_t   task_control = {NULL, (void*)cy256_process_chunk, NULL, 0x0};

#elif !defined(USE_SSE2)

	// Vars needed in scalar mode only:
	const  double one_half[3] = {1.0, 0.5, 0.25};	/* Needed for small-weights-tables scheme */
	int m,m2;
	double wt,wtinv,wtA,wtB,wtC;	/* Mersenne-mod weights stuff */
  #if PFETCH
	int prefetch_offset;
  #endif
	int bjmodn[RADIX];
	double temp,frac,
		cy_r[RADIX],cy_i[RADIX];

#endif

/*...stuff for the multithreaded implementation is here:	*/
	static uint32 CY_THREADS,pini;
	int ithread,j_jhi;
	uint32 ptr_prod;
	static int *_i, *_jstart = 0x0, *_jhi = 0x0, *_col = 0x0, *_co2 = 0x0, *_co3 = 0x0;
	static int *_bjmodnini = 0x0, *_bjmodn[RADIX];
	static double *_cy_r[RADIX],*_cy_i[RADIX];
	if(!_jhi) {
		_cy_r[0] = 0x0;	// First of these used as an "already inited consts?" sentinel, must init = 0x0 at same time do so for non-array static ptrs
	}

	// Init these to get rid of GCC "may be used uninitialized in this function" warnings:
	col=co2=co3=-1;

/*...change NDIVR and n_div_wt to non-static to work around a gcc compiler bug. */
	NDIVR   = n/RADIX;
	n_div_nwt = NDIVR >> nwt_bits;

	if((n_div_nwt << nwt_bits) != NDIVR)
	{
		sprintf(cbuf,"FATAL: iter = %10d; NWT_BITS does not divide N/RADIX in %s.\n",iter,func);
		if(INTERACT)fprintf(stderr,"%s",cbuf);
		fp = mlucas_fopen(   OFILE,"a");
		fq = mlucas_fopen(STATFILE,"a");
		fprintf(fp,"%s",cbuf);
		fprintf(fq,"%s",cbuf);
		fclose(fp);	fp = 0x0;
		fclose(fq);	fq = 0x0;
		err=ERR_CARRY;
		return(err);
	}

	if(p != psave || n != nsave) {	/* Exponent or array length change triggers re-init */
		first_entry=TRUE;
		/* To-do: Support #thread change here! */
	}

/*...initialize things upon first entry: */

	if(first_entry)
	{
		psave = p;	nsave = n;
		radix_inv = qfdbl(qf_rational_quotient((int64)1, (int64)RADIX));
		n2inv     = qfdbl(qf_rational_quotient((int64)1, (int64)(n/2)));

		bw    = p%n;	/* Number of bigwords in the Crandall/Fagin mixed-radix representation = (Mersenne exponent) mod (vector length).	*/
		sw    = n - bw;	/* Number of smallwords.	*/

		nm1   = n-1;

	#ifdef LOACC

	  #ifdef USE_AVX512
	   #ifdef CARRY_16_WAY
		i = 16;
	   #else
		i = 8;
	   #endif
	  #elif defined(USE_AVX)	// AVX LOACC: Make CARRY_8_WAY default here:
		i = 8;
	  #elif defined(USE_SSE2)	// AVX and SSE2 modes use 4-way carry macros
		i = 4;
	  #else	// Scalar-double mode:
		i = 1;
	  #endif

		// For n a power of 2 don't need to worry about 32-bit integer overflow in the sw*NDIVR term,
		// but for non-power-of-2 n we must cast-to-uint64 to avoid such overflows fubaring the result:
		struct qfloat qt,qn;
		qt = i64_to_q(i*(uint64)sw*NDIVR % n);
		qn = i64_to_q((int64) n);
		qt = qfdiv(qt, qn);		// x = (sw*NDIVR (mod n))/n
		qt = qfmul(qt, QLN2);	// x*ln(2)...
		qt = qfexp(qt);			// ...and get 2^x via exp[x*ln(2)].
		wts_mult[0] = qfdbl(qt);		// a = 2^(x/n), with x = sw
		inv_mult[0] = qfdbl(qfinv(qt));	// Double-based inversion (1.0 / wts_mult_a[0]) often gets LSB wrong
		ASSERT(HERE,fabs(wts_mult[0]*inv_mult[0] - 1.0) < EPS, "wts_mults fail accuracy check!");
		//curr have w, 2/w, separate-mul-by-1-or-0.5 gives [w,w/2] and [1/w,2/w] for i = 0,1, resp:
		wts_mult[1] = 0.5*wts_mult[0];
		inv_mult[1] = 2.0*inv_mult[0];
		ASSERT(HERE,fabs(wts_mult[1]*inv_mult[1] - 1.0) < EPS, "wts_mults fail accuracy check!");

	#endif

	#if !defined(USE_SSE2) && defined(USE_FMA)
		// Precompute the FMA-modified twiddles for the 2nd-pass radix-16 DFTs:

		#define FMA_TWIDDLE_FIDDLE(\
		 __c8, __s8, __c4, __s4, __cC, __sC, __c2, __s2, __cA, __sA, __c6, __s6, __cE, __sE, __c1, __s1, __c9, __s9, __c5, __s5, __cD, __sD, __c3, __s3, __cB, __sB, __c7, __s7, __cF, __sF,	__c,\
		__cc1,__ss1,__cc2,__ss2,__cc3,__ss3,__cc4,__ss4,__cc5,__ss5,__cc6,__ss6,__cc7,__ss7,__cc8,__ss8,__cc9,__ss9,__ccA,__ssA,__ccB,__ssB,__ccC,__ssC,__ccD,__ssD,__ccE,__ssE,__ccF,__ssF,	__c1_c,__c1i2,__c2i2)\
		{\
			/* FMA-based version replaces sine terms with tangents; tan = s/c assumed const-defined in calling routine: */\
			__ss1 = __s1/__c1;\
			__ss2 = __s2/__c2;\
			__ss3 = __s3/__c3;\
			__ss4 = __s4/__c4;\
			__ss5 = __s5/__c5;\
			__ss6 = __s6/__c6;\
			__ss7 = __s7/__c7;\
			__ss8 = __s8/__c8;\
			__ss9 = __s9/__c9;\
			__ssA = __sA/__cA;\
			__ssB = __sB/__cB;\
			__ssC = __sC/__cC;\
			__ssD = __sD/__cD;\
			__ssE = __sE/__cE;\
			__ssF = __sF/__cF;\
			/* Cosine terms defined like so: */\
			__c1_c = __c1*__c;	/* In FMA code, this takes the place of c */\
			__c1i2 = __c1*ISRT2;\
			__c2i2 = __c2*ISRT2;\
			__cc1 = __c1;\
			__cc2 = __c2;\
			__cc3 = __c3/__c1;\
			__cc4 = __c4;\
			__cc5 = __c5/__c1;\
			__cc6 = __c6/__c2;\
			__cc7 = __c7/__c3;\
			__cc8 = __c8;\
			__cc9 = __c9/__c1;\
			__ccA = __cA/__c2;\
			__ccB = __cB/__c3;\
			__ccC = __cC/__c4;\
			__ccD = __cD/__c5;\
			__ccE = __cE/__c6;\
			__ccF = __cF/__c7;\
			/* In addition to the 30 derived multipliers, we need the unmodified __c1,2,4,8, for a total of 34. */\
		}

	/* This is reason we can't use FMA-based DFT for first set of non-unity twiddles - the leading [0+i] twiddle leads to div-by-0 in our tangent-based FMA-DFT scheme:
		FMA_TWIDDLE_FIDDLE(
			0,1, ISRT2,ISRT2, -ISRT2,ISRT2, c16,s16, -s16,c16, s16,c16, -c16,s16, c32_1,s32_1, -s32_1,c32_1, s32_3,c32_3, -c32_3,s32_3, c32_3,s32_3, -s32_3,c32_3, s32_1,c32_1, -c32_1,s32_1	,c16,
			c1_1,s1_1,c1_2,s1_2,c1_3,s1_3,c1_4,s1_4,c1_5,s1_5,c1_6,s1_6,c1_7,s1_7,c1_8,s1_8,c1_9,s1_9,c1_10,s1_10,c1_11,s1_11,c1_12,s1_12,c1_13,s1_13,c1_14,s1_14,c1_15,s1_15,	c1_1_c,c1_1i2,c1_2i2)
	*/
		FMA_TWIDDLE_FIDDLE(
			ISRT2,ISRT2, c16,s16, s16,c16, c32_1,s32_1, s32_3,c32_3, c32_3,s32_3, s32_1,c32_1, c64_1,s64_1, s64_7,c64_7, c64_5,s64_5, s64_3,c64_3, c64_3,s64_3, s64_5,c64_5, c64_7,s64_7, s64_1,c64_1	,c16,
			c2_1,s2_1,c2_2,s2_2,c2_3,s2_3,c2_4,s2_4,c2_5,s2_5,c2_6,s2_6,c2_7,s2_7,c2_8,s2_8,c2_9,s2_9,c2_10,s2_10,c2_11,s2_11,c2_12,s2_12,c2_13,s2_13,c2_14,s2_14,c2_15,s2_15,	c2_1_c,c2_1i2,c2_2i2)
		FMA_TWIDDLE_FIDDLE(
			-ISRT2,ISRT2, s16,c16, -c16,-s16, c32_3,s32_3, -c32_1,s32_1, -s32_1,c32_1, -s32_3,-c32_3, c64_3,s64_3, -c64_5,s64_5, s64_1,c64_1, -c64_7,-s64_7, s64_7,c64_7, -c64_1,-s64_1, -s64_5,c64_5, -s64_3,-c64_3	,c16,
			c3_1,s3_1,c3_2,s3_2,c3_3,s3_3,c3_4,s3_4,c3_5,s3_5,c3_6,s3_6,c3_7,s3_7,c3_8,s3_8,c3_9,s3_9,c3_10,s3_10,c3_11,s3_11,c3_12,s3_12,c3_13,s3_13,c3_14,s3_14,c3_15,s3_15,	c3_1_c,c3_1i2,c3_2i2)
		FMA_TWIDDLE_FIDDLE(
			c16,s16, c32_1,s32_1, c32_3,s32_3, c64_1,s64_1, c64_5,s64_5, c64_3,s64_3, c64_7,s64_7, c128_1,s128_1, c128_9,s128_9, c128_5,s128_5, c128_d,s128_d, c128_3,s128_3, c128_b,s128_b, c128_7,s128_7, c128_f,s128_f	,c16,
			c4_1,s4_1,c4_2,s4_2,c4_3,s4_3,c4_4,s4_4,c4_5,s4_5,c4_6,s4_6,c4_7,s4_7,c4_8,s4_8,c4_9,s4_9,c4_10,s4_10,c4_11,s4_11,c4_12,s4_12,c4_13,s4_13,c4_14,s4_14,c4_15,s4_15,	c4_1_c,c4_1i2,c4_2i2)
		FMA_TWIDDLE_FIDDLE(
			-s16,c16, s32_3,c32_3, -c32_1,s32_1, c64_5,s64_5, -c64_7,s64_7, s64_1,c64_1, -c64_3,-s64_3, c128_5,s128_5, -s128_d,c128_d, s128_7,c128_7, -c128_1,-s128_1, c128_f,s128_f, -c128_9,s128_9, -s128_3,c128_3, -c128_b,-s128_b	,c16,
			c5_1,s5_1,c5_2,s5_2,c5_3,s5_3,c5_4,s5_4,c5_5,s5_5,c5_6,s5_6,c5_7,s5_7,c5_8,s5_8,c5_9,s5_9,c5_10,s5_10,c5_11,s5_11,c5_12,s5_12,c5_13,s5_13,c5_14,s5_14,c5_15,s5_15,	c5_1_c,c5_1i2,c5_2i2)
		FMA_TWIDDLE_FIDDLE(
			s16,c16, c32_3,s32_3, -s32_1,c32_1, c64_3,s64_3, s64_1,c64_1, s64_7,c64_7, -s64_5,c64_5, c128_3,s128_3, s128_5,c128_5, c128_f,s128_f, -s128_7,c128_7, c128_9,s128_9, -s128_1,c128_1, s128_b,c128_b, -s128_d,c128_d	,c16,
			c6_1,s6_1,c6_2,s6_2,c6_3,s6_3,c6_4,s6_4,c6_5,s6_5,c6_6,s6_6,c6_7,s6_7,c6_8,s6_8,c6_9,s6_9,c6_10,s6_10,c6_11,s6_11,c6_12,s6_12,c6_13,s6_13,c6_14,s6_14,c6_15,s6_15,	c6_1_c,c6_1i2,c6_2i2)
		FMA_TWIDDLE_FIDDLE(
			-c16,s16, s32_1,c32_1, -s32_3,-c32_3, c64_7,s64_7, -c64_3,-s64_3, -s64_5,c64_5, s64_1,-c64_1, c128_7,s128_7, -c128_1,s128_1, -s128_3,c128_3, -s128_5,-c128_5, s128_b,c128_b, -c128_d,-s128_d, -c128_f,s128_f, s128_9,-c128_9	,c16,
			c7_1,s7_1,c7_2,s7_2,c7_3,s7_3,c7_4,s7_4,c7_5,s7_5,c7_6,s7_6,c7_7,s7_7,c7_8,s7_8,c7_9,s7_9,c7_10,s7_10,c7_11,s7_11,c7_12,s7_12,c7_13,s7_13,c7_14,s7_14,c7_15,s7_15,	c7_1_c,c7_1i2,c7_2i2)
		FMA_TWIDDLE_FIDDLE(
			c32_1,s32_1, c64_1,s64_1, c64_3,s64_3, c128_1,s128_1, c128_5,s128_5, c128_3,s128_3, c128_7,s128_7, c256_01,s256_01, c256_09,s256_09, c256_05,s256_05, c256_0d,s256_0d, c256_03,s256_03, c256_0b,s256_0b, c256_07,s256_07, c256_0f,s256_0f	,c16,
			c8_1,s8_1,c8_2,s8_2,c8_3,s8_3,c8_4,s8_4,c8_5,s8_5,c8_6,s8_6,c8_7,s8_7,c8_8,s8_8,c8_9,s8_9,c8_10,s8_10,c8_11,s8_11,c8_12,s8_12,c8_13,s8_13,c8_14,s8_14,c8_15,s8_15,	c8_1_c,c8_1i2,c8_2i2)
		FMA_TWIDDLE_FIDDLE(
			-s32_1,c32_1, s64_7,c64_7, -c64_5,s64_5, c128_9,s128_9, -s128_d,c128_d, s128_5,c128_5, -c128_1,s128_1, c256_09,s256_09, -s256_11,c256_11, s256_13,c256_13, -c256_0b,s256_0b, c256_1b,s256_1b, -c256_1d,s256_1d, s256_01,c256_01, -c256_07,-s256_07	,c16,
			c9_1,s9_1,c9_2,s9_2,c9_3,s9_3,c9_4,s9_4,c9_5,s9_5,c9_6,s9_6,c9_7,s9_7,c9_8,s9_8,c9_9,s9_9,c9_10,s9_10,c9_11,s9_11,c9_12,s9_12,c9_13,s9_13,c9_14,s9_14,c9_15,s9_15,	c9_1_c,c9_1i2,c9_2i2)
		FMA_TWIDDLE_FIDDLE(
			s32_3,c32_3, c64_5,s64_5, s64_1,c64_1, c128_5,s128_5, s128_7,c128_7, c128_f,s128_f, -s128_3,c128_3, c256_05,s256_05, s256_13,c256_13, c256_19,s256_19, -s256_01,c256_01, c256_0f,s256_0f, s256_09,c256_09, s256_1d,c256_1d, -s256_0b,c256_0b	,c16,
			ca_1,sa_1,ca_2,sa_2,ca_3,sa_3,ca_4,sa_4,ca_5,sa_5,ca_6,sa_6,ca_7,sa_7,ca_8,sa_8,ca_9,sa_9,ca_10,sa_10,ca_11,sa_11,ca_12,sa_12,ca_13,sa_13,ca_14,sa_14,ca_15,sa_15,	ca_1_c,ca_1i2,ca_2i2)
		FMA_TWIDDLE_FIDDLE(
			-c32_3,s32_3, s64_3,c64_3, -c64_7,-s64_7, c128_d,s128_d, -c128_1,-s128_1, -s128_7,c128_7, -s128_5,-c128_5, c256_0d,s256_0d, -c256_0b,s256_0b, -s256_01,c256_01, -s256_17,-c256_17, s256_19,c256_19, -c256_0f,-s256_0f, -s256_1b,c256_1b, s256_03,-c256_03	,c16,
			cb_1,sb_1,cb_2,sb_2,cb_3,sb_3,cb_4,sb_4,cb_5,sb_5,cb_6,sb_6,cb_7,sb_7,cb_8,sb_8,cb_9,sb_9,cb_10,sb_10,cb_11,sb_11,cb_12,sb_12,cb_13,sb_13,cb_14,sb_14,cb_15,sb_15,	cb_1_c,cb_1i2,cb_2i2)
		FMA_TWIDDLE_FIDDLE(
			c32_3,s32_3, c64_3,s64_3, s64_7,c64_7, c128_3,s128_3, c128_f,s128_f, c128_9,s128_9, s128_b,c128_b, c256_03,s256_03, c256_1b,s256_1b, c256_0f,s256_0f, s256_19,c256_19, c256_09,s256_09, s256_1f,c256_1f, c256_15,s256_15, s256_13,c256_13	,c16,
			cc_1,sc_1,cc_2,sc_2,cc_3,sc_3,cc_4,sc_4,cc_5,sc_5,cc_6,sc_6,cc_7,sc_7,cc_8,sc_8,cc_9,sc_9,cc_10,sc_10,cc_11,sc_11,cc_12,sc_12,cc_13,sc_13,cc_14,sc_14,cc_15,sc_15,	cc_1_c,cc_1i2,cc_2i2)
		FMA_TWIDDLE_FIDDLE(
			-s32_3,c32_3, s64_5,c64_5, -c64_1,-s64_1, c128_b,s128_b, -c128_9,s128_9, -s128_1,c128_1, -c128_d,-s128_d, c256_0b,s256_0b, -c256_1d,s256_1d, s256_09,c256_09, -c256_0f,-s256_0f, s256_1f,c256_1f, -c256_07,s256_07, -s256_0d,c256_0d, -s256_1b,-c256_1b	,c16,
			cd_1,sd_1,cd_2,sd_2,cd_3,sd_3,cd_4,sd_4,cd_5,sd_5,cd_6,sd_6,cd_7,sd_7,cd_8,sd_8,cd_9,sd_9,cd_10,sd_10,cd_11,sd_11,cd_12,sd_12,cd_13,sd_13,cd_14,sd_14,cd_15,sd_15,	cd_1_c,cd_1i2,cd_2i2)
		FMA_TWIDDLE_FIDDLE(
			s32_1,c32_1, c64_7,s64_7, -s64_5,c64_5, c128_7,s128_7, -s128_3,c128_3, s128_b,c128_b, -c128_f,s128_f, c256_07,s256_07, s256_01,c256_01, s256_1d,c256_1d, -s256_1b,c256_1b, c256_15,s256_15, -s256_0d,c256_0d, s256_0f,c256_0f, -c256_17,s256_17	,c16,
			ce_1,se_1,ce_2,se_2,ce_3,se_3,ce_4,se_4,ce_5,se_5,ce_6,se_6,ce_7,se_7,ce_8,se_8,ce_9,se_9,ce_10,se_10,ce_11,se_11,ce_12,se_12,ce_13,se_13,ce_14,se_14,ce_15,se_15,	ce_1_c,ce_1i2,ce_2i2)
		FMA_TWIDDLE_FIDDLE(
			-c32_1,s32_1, s64_1,c64_1, -s64_3,-c64_3, c128_f,s128_f, -c128_b,-s128_b, -s128_d,c128_d, s128_9,-c128_9, c256_0f,s256_0f, -c256_07,-s256_07, -s256_0b,c256_0b, s256_03,-c256_03, s256_13,c256_13, -s256_1b,-c256_1b, -c256_17,s256_17, c256_1f,-s256_1f	,c16,
			cf_1,sf_1,cf_2,sf_2,cf_3,sf_3,cf_4,sf_4,cf_5,sf_5,cf_6,sf_6,cf_7,sf_7,cf_8,sf_8,cf_9,sf_9,cf_10,sf_10,cf_11,sf_11,cf_12,sf_12,cf_13,sf_13,cf_14,sf_14,cf_15,sf_15,	cf_1_c,cf_1i2,cf_2i2)

	#endif

	#ifdef MULTITHREAD

		/* #Chunks ||ized in carry step is ideally a power of 2, so use the largest
		power of 2 that is <= the value of the global NTHREADS (but still <= MAX_THREADS):
		*/
		if(isPow2(NTHREADS))
			CY_THREADS = NTHREADS;
		else
		{
			i = leadz32(NTHREADS);
			CY_THREADS = (((uint32)NTHREADS << i) & 0x80000000) >> i;
		}

		if(CY_THREADS > MAX_THREADS)
		{
		//	CY_THREADS = MAX_THREADS;
			fprintf(stderr,"WARN: CY_THREADS = %d exceeds number of cores = %d\n", CY_THREADS, MAX_THREADS);
		}
		if(!isPow2(CY_THREADS))		{ WARN(HERE, "CY_THREADS not a power of 2!", "", 1); return(ERR_ASSERT); }
		if(CY_THREADS > 1)
		{
			if(NDIVR    %CY_THREADS != 0) { WARN(HERE, "NDIVR    %CY_THREADS != 0", "", 1); return(ERR_ASSERT); }
			if(n_div_nwt%CY_THREADS != 0) { WARN(HERE, "n_div_nwt%CY_THREADS != 0", "", 1); return(ERR_ASSERT); }
		}

	  #ifdef USE_PTHREAD
		if(tdat == 0x0) {
			j = (uint32)sizeof(struct cy_thread_data_t);
			tdat = (struct cy_thread_data_t *)calloc(CY_THREADS, sizeof(struct cy_thread_data_t));
	
			// MacOS does weird things with threading (e.g. Idle" main thread burning 100% of 1 CPU)
			// so on that platform try to be clever and interleave main-thread and threadpool-work processing
			#if 0//def OS_TYPE_MACOSX
	
				if(CY_THREADS > 1) {
					main_work_units = CY_THREADS/2;
					pool_work_units = CY_THREADS - main_work_units;
					ASSERT(HERE, 0x0 != (tpool = threadpool_init(pool_work_units, MAX_THREADS, pool_work_units, &thread_control)), "threadpool_init failed!");
					printf("radix%d_ditN_cy_dif1: Init threadpool of %d threads\n", RADIX, pool_work_units);
				} else {
					main_work_units = 1;
					printf("radix%d_ditN_cy_dif1: CY_THREADS = 1: Using main execution thread, no threadpool needed.\n", RADIX);
				}
	
			#else
	
				pool_work_units = CY_THREADS;
				ASSERT(HERE, 0x0 != (tpool = threadpool_init(CY_THREADS, MAX_THREADS, CY_THREADS, &thread_control)), "threadpool_init failed!");
	
			#endif
	
			fprintf(stderr,"Using %d threads in carry step\n", CY_THREADS);
		}
	  #endif

	#else
		CY_THREADS = 1;
	#endif

	#ifdef USE_PTHREAD
		/* Populate the elements of the thread-specific data structs which don't change after init: */
		for(ithread = 0; ithread < CY_THREADS; ithread++)
		{
		// int data:
			tdat[ithread].tid = ithread;
			tdat[ithread].ndivr = NDIVR;

			tdat[ithread].sw  = sw;
			tdat[ithread].nwt = nwt;

		// pointer data:
			tdat[ithread].arrdat = a;			/* Main data array */
			tdat[ithread].wt0 = wt0;
			tdat[ithread].wt1 = wt1;
		#ifdef LOACC
			tdat[ithread].wts_mult = wts_mult;
			tdat[ithread].inv_mult = inv_mult;
		#endif
			tdat[ithread].si  = si;
			tdat[ithread].rn0 = rn0;
			tdat[ithread].rn1 = rn1;

		// This array pointer must be set based on vec_dbl-sized alignment at runtime for each thread:
			for(l = 0; l < RE_IM_STRIDE; l++) {
				if( ((long)&tdat[ithread].cy_dat[l] & sz_vd_m1) == 0 ) {
					tdat[ithread].cy_r = &tdat[ithread].cy_dat[l];
					tdat[ithread].cy_i = tdat[ithread].cy_r + RADIX;
				//	fprintf(stderr,"%d-byte-align cy_dat array at element[%d]\n",sz_vd,l);
					break;
				}
			}
			ASSERT(HERE, l < RE_IM_STRIDE, "Failed to align cy_dat array!");
		}
	#endif

	#ifdef USE_SSE2

		ASSERT(HERE, ((long)wt0    & 0x3f) == 0, "wt0[]  not 64-byte aligned!");
		ASSERT(HERE, ((long)wt1    & 0x3f) == 0, "wt1[]  not 64-byte aligned!");

		// Use vector-double type size (16 bytes for SSE2, 32 for AVX) to alloc a block of local storage
		// consisting of 256 vec_dbl and ([8 if SSE2, 16 if AVX] + RADIX/2) uint64 element slots per thread
		cslots_in_local_store = radix256_creals_in_local_store + (20+RADIX/2)/2;	// Just add enough int64 space for both cases, plus some
		sc_arr = ALLOC_VEC_DBL(sc_arr, cslots_in_local_store*CY_THREADS);	if(!sc_arr){ sprintf(cbuf, "FATAL: unable to allocate sc_arr!.\n"); fprintf(stderr,"%s", cbuf);	ASSERT(HERE, 0,cbuf); }
		sc_ptr = ALIGN_VEC_DBL(sc_arr);
		ASSERT(HERE, ((long)sc_ptr & 0x3f) == 0, "sc_ptr not 64-byte aligned!");
		sm_ptr = (uint64*)(sc_ptr + radix256_creals_in_local_store);
		ASSERT(HERE, ((long)sm_ptr & 0x3f) == 0, "sm_ptr not 64-byte aligned!");

	  #ifdef USE_PTHREAD
		__r0 = sc_ptr;
	  #endif
		tmp = sc_ptr;			tm2 = tmp + 0x100;
		r00 = tmp + 0x00;		r80 = tm2 + 0x00;
		r01 = tmp + 0x02;		r81 = tm2 + 0x02;
		r02 = tmp + 0x04;		r82 = tm2 + 0x04;
		r03 = tmp + 0x06;		r83 = tm2 + 0x06;
		r04 = tmp + 0x08;		r84 = tm2 + 0x08;
		r05 = tmp + 0x0a;		r85 = tm2 + 0x0a;
		r06 = tmp + 0x0c;		r86 = tm2 + 0x0c;
		r07 = tmp + 0x0e;		r87 = tm2 + 0x0e;
		r08 = tmp + 0x10;		r88 = tm2 + 0x10;
		r09 = tmp + 0x12;		r89 = tm2 + 0x12;
		r0a = tmp + 0x14;		r8a = tm2 + 0x14;
		r0b = tmp + 0x16;		r8b = tm2 + 0x16;
		r0c = tmp + 0x18;		r8c = tm2 + 0x18;
		r0d = tmp + 0x1a;		r8d = tm2 + 0x1a;
		r0e = tmp + 0x1c;		r8e = tm2 + 0x1c;
		r0f = tmp + 0x1e;		r8f = tm2 + 0x1e;
		r10 = tmp + 0x20;		r90 = tm2 + 0x20;
		r11 = tmp + 0x22;		r91 = tm2 + 0x22;
		r12 = tmp + 0x24;		r92 = tm2 + 0x24;
		r13 = tmp + 0x26;		r93 = tm2 + 0x26;
		r14 = tmp + 0x28;		r94 = tm2 + 0x28;
		r15 = tmp + 0x2a;		r95 = tm2 + 0x2a;
		r16 = tmp + 0x2c;		r96 = tm2 + 0x2c;
		r17 = tmp + 0x2e;		r97 = tm2 + 0x2e;
		r18 = tmp + 0x30;		r98 = tm2 + 0x30;
		r19 = tmp + 0x32;		r99 = tm2 + 0x32;
		r1a = tmp + 0x34;		r9a = tm2 + 0x34;
		r1b = tmp + 0x36;		r9b = tm2 + 0x36;
		r1c = tmp + 0x38;		r9c = tm2 + 0x38;
		r1d = tmp + 0x3a;		r9d = tm2 + 0x3a;
		r1e = tmp + 0x3c;		r9e = tm2 + 0x3c;
		r1f = tmp + 0x3e;		r9f = tm2 + 0x3e;
		r20 = tmp + 0x40;		ra0 = tm2 + 0x40;
		r21 = tmp + 0x42;		ra1 = tm2 + 0x42;
		r22 = tmp + 0x44;		ra2 = tm2 + 0x44;
		r23 = tmp + 0x46;		ra3 = tm2 + 0x46;
		r24 = tmp + 0x48;		ra4 = tm2 + 0x48;
		r25 = tmp + 0x4a;		ra5 = tm2 + 0x4a;
		r26 = tmp + 0x4c;		ra6 = tm2 + 0x4c;
		r27 = tmp + 0x4e;		ra7 = tm2 + 0x4e;
		r28 = tmp + 0x50;		ra8 = tm2 + 0x50;
		r29 = tmp + 0x52;		ra9 = tm2 + 0x52;
		r2a = tmp + 0x54;		raa = tm2 + 0x54;
		r2b = tmp + 0x56;		rab = tm2 + 0x56;
		r2c = tmp + 0x58;		rac = tm2 + 0x58;
		r2d = tmp + 0x5a;		rad = tm2 + 0x5a;
		r2e = tmp + 0x5c;		rae = tm2 + 0x5c;
		r2f = tmp + 0x5e;		raf = tm2 + 0x5e;
		r30 = tmp + 0x60;		rb0 = tm2 + 0x60;
		r31 = tmp + 0x62;		rb1 = tm2 + 0x62;
		r32 = tmp + 0x64;		rb2 = tm2 + 0x64;
		r33 = tmp + 0x66;		rb3 = tm2 + 0x66;
		r34 = tmp + 0x68;		rb4 = tm2 + 0x68;
		r35 = tmp + 0x6a;		rb5 = tm2 + 0x6a;
		r36 = tmp + 0x6c;		rb6 = tm2 + 0x6c;
		r37 = tmp + 0x6e;		rb7 = tm2 + 0x6e;
		r38 = tmp + 0x70;		rb8 = tm2 + 0x70;
		r39 = tmp + 0x72;		rb9 = tm2 + 0x72;
		r3a = tmp + 0x74;		rba = tm2 + 0x74;
		r3b = tmp + 0x76;		rbb = tm2 + 0x76;
		r3c = tmp + 0x78;		rbc = tm2 + 0x78;
		r3d = tmp + 0x7a;		rbd = tm2 + 0x7a;
		r3e = tmp + 0x7c;		rbe = tm2 + 0x7c;
		r3f = tmp + 0x7e;		rbf = tm2 + 0x7e;
		r40 = tmp + 0x80;		rc0 = tm2 + 0x80;
		r41 = tmp + 0x82;		rc1 = tm2 + 0x82;
		r42 = tmp + 0x84;		rc2 = tm2 + 0x84;
		r43 = tmp + 0x86;		rc3 = tm2 + 0x86;
		r44 = tmp + 0x88;		rc4 = tm2 + 0x88;
		r45 = tmp + 0x8a;		rc5 = tm2 + 0x8a;
		r46 = tmp + 0x8c;		rc6 = tm2 + 0x8c;
		r47 = tmp + 0x8e;		rc7 = tm2 + 0x8e;
		r48 = tmp + 0x90;		rc8 = tm2 + 0x90;
		r49 = tmp + 0x92;		rc9 = tm2 + 0x92;
		r4a = tmp + 0x94;		rca = tm2 + 0x94;
		r4b = tmp + 0x96;		rcb = tm2 + 0x96;
		r4c = tmp + 0x98;		rcc = tm2 + 0x98;
		r4d = tmp + 0x9a;		rcd = tm2 + 0x9a;
		r4e = tmp + 0x9c;		rce = tm2 + 0x9c;
		r4f = tmp + 0x9e;		rcf = tm2 + 0x9e;
		r50 = tmp + 0xa0;		rd0 = tm2 + 0xa0;
		r51 = tmp + 0xa2;		rd1 = tm2 + 0xa2;
		r52 = tmp + 0xa4;		rd2 = tm2 + 0xa4;
		r53 = tmp + 0xa6;		rd3 = tm2 + 0xa6;
		r54 = tmp + 0xa8;		rd4 = tm2 + 0xa8;
		r55 = tmp + 0xaa;		rd5 = tm2 + 0xaa;
		r56 = tmp + 0xac;		rd6 = tm2 + 0xac;
		r57 = tmp + 0xae;		rd7 = tm2 + 0xae;
		r58 = tmp + 0xb0;		rd8 = tm2 + 0xb0;
		r59 = tmp + 0xb2;		rd9 = tm2 + 0xb2;
		r5a = tmp + 0xb4;		rda = tm2 + 0xb4;
		r5b = tmp + 0xb6;		rdb = tm2 + 0xb6;
		r5c = tmp + 0xb8;		rdc = tm2 + 0xb8;
		r5d = tmp + 0xba;		rdd = tm2 + 0xba;
		r5e = tmp + 0xbc;		rde = tm2 + 0xbc;
		r5f = tmp + 0xbe;		rdf = tm2 + 0xbe;
		r60 = tmp + 0xc0;		re0 = tm2 + 0xc0;
		r61 = tmp + 0xc2;		re1 = tm2 + 0xc2;
		r62 = tmp + 0xc4;		re2 = tm2 + 0xc4;
		r63 = tmp + 0xc6;		re3 = tm2 + 0xc6;
		r64 = tmp + 0xc8;		re4 = tm2 + 0xc8;
		r65 = tmp + 0xca;		re5 = tm2 + 0xca;
		r66 = tmp + 0xcc;		re6 = tm2 + 0xcc;
		r67 = tmp + 0xce;		re7 = tm2 + 0xce;
		r68 = tmp + 0xd0;		re8 = tm2 + 0xd0;
		r69 = tmp + 0xd2;		re9 = tm2 + 0xd2;
		r6a = tmp + 0xd4;		rea = tm2 + 0xd4;
		r6b = tmp + 0xd6;		reb = tm2 + 0xd6;
		r6c = tmp + 0xd8;		rec = tm2 + 0xd8;
		r6d = tmp + 0xda;		red = tm2 + 0xda;
		r6e = tmp + 0xdc;		ree = tm2 + 0xdc;
		r6f = tmp + 0xde;		ref = tm2 + 0xde;
		r70 = tmp + 0xe0;		rf0 = tm2 + 0xe0;
		r71 = tmp + 0xe2;		rf1 = tm2 + 0xe2;
		r72 = tmp + 0xe4;		rf2 = tm2 + 0xe4;
		r73 = tmp + 0xe6;		rf3 = tm2 + 0xe6;
		r74 = tmp + 0xe8;		rf4 = tm2 + 0xe8;
		r75 = tmp + 0xea;		rf5 = tm2 + 0xea;
		r76 = tmp + 0xec;		rf6 = tm2 + 0xec;
		r77 = tmp + 0xee;		rf7 = tm2 + 0xee;
		r78 = tmp + 0xf0;		rf8 = tm2 + 0xf0;
		r79 = tmp + 0xf2;		rf9 = tm2 + 0xf2;
		r7a = tmp + 0xf4;		rfa = tm2 + 0xf4;
		r7b = tmp + 0xf6;		rfb = tm2 + 0xf6;
		r7c = tmp + 0xf8;		rfc = tm2 + 0xf8;
		r7d = tmp + 0xfa;		rfd = tm2 + 0xfa;
		r7e = tmp + 0xfc;		rfe = tm2 + 0xfc;
		r7f = tmp + 0xfe;		rff = tm2 + 0xfe;
		tmp += 0x200;			tm2 += 0x200;
		s1p00 = tmp + 0x00;		s1p80 = tm2 + 0x00;
		s1p01 = tmp + 0x02;		s1p81 = tm2 + 0x02;
		s1p02 = tmp + 0x04;		s1p82 = tm2 + 0x04;
		s1p03 = tmp + 0x06;		s1p83 = tm2 + 0x06;
		s1p04 = tmp + 0x08;		s1p84 = tm2 + 0x08;
		s1p05 = tmp + 0x0a;		s1p85 = tm2 + 0x0a;
		s1p06 = tmp + 0x0c;		s1p86 = tm2 + 0x0c;
		s1p07 = tmp + 0x0e;		s1p87 = tm2 + 0x0e;
		s1p08 = tmp + 0x10;		s1p88 = tm2 + 0x10;
		s1p09 = tmp + 0x12;		s1p89 = tm2 + 0x12;
		s1p0a = tmp + 0x14;		s1p8a = tm2 + 0x14;
		s1p0b = tmp + 0x16;		s1p8b = tm2 + 0x16;
		s1p0c = tmp + 0x18;		s1p8c = tm2 + 0x18;
		s1p0d = tmp + 0x1a;		s1p8d = tm2 + 0x1a;
		s1p0e = tmp + 0x1c;		s1p8e = tm2 + 0x1c;
		s1p0f = tmp + 0x1e;		s1p8f = tm2 + 0x1e;
		s1p10 = tmp + 0x20;		s1p90 = tm2 + 0x20;
		s1p11 = tmp + 0x22;		s1p91 = tm2 + 0x22;
		s1p12 = tmp + 0x24;		s1p92 = tm2 + 0x24;
		s1p13 = tmp + 0x26;		s1p93 = tm2 + 0x26;
		s1p14 = tmp + 0x28;		s1p94 = tm2 + 0x28;
		s1p15 = tmp + 0x2a;		s1p95 = tm2 + 0x2a;
		s1p16 = tmp + 0x2c;		s1p96 = tm2 + 0x2c;
		s1p17 = tmp + 0x2e;		s1p97 = tm2 + 0x2e;
		s1p18 = tmp + 0x30;		s1p98 = tm2 + 0x30;
		s1p19 = tmp + 0x32;		s1p99 = tm2 + 0x32;
		s1p1a = tmp + 0x34;		s1p9a = tm2 + 0x34;
		s1p1b = tmp + 0x36;		s1p9b = tm2 + 0x36;
		s1p1c = tmp + 0x38;		s1p9c = tm2 + 0x38;
		s1p1d = tmp + 0x3a;		s1p9d = tm2 + 0x3a;
		s1p1e = tmp + 0x3c;		s1p9e = tm2 + 0x3c;
		s1p1f = tmp + 0x3e;		s1p9f = tm2 + 0x3e;
		s1p20 = tmp + 0x40;		s1pa0 = tm2 + 0x40;
		s1p21 = tmp + 0x42;		s1pa1 = tm2 + 0x42;
		s1p22 = tmp + 0x44;		s1pa2 = tm2 + 0x44;
		s1p23 = tmp + 0x46;		s1pa3 = tm2 + 0x46;
		s1p24 = tmp + 0x48;		s1pa4 = tm2 + 0x48;
		s1p25 = tmp + 0x4a;		s1pa5 = tm2 + 0x4a;
		s1p26 = tmp + 0x4c;		s1pa6 = tm2 + 0x4c;
		s1p27 = tmp + 0x4e;		s1pa7 = tm2 + 0x4e;
		s1p28 = tmp + 0x50;		s1pa8 = tm2 + 0x50;
		s1p29 = tmp + 0x52;		s1pa9 = tm2 + 0x52;
		s1p2a = tmp + 0x54;		s1paa = tm2 + 0x54;
		s1p2b = tmp + 0x56;		s1pab = tm2 + 0x56;
		s1p2c = tmp + 0x58;		s1pac = tm2 + 0x58;
		s1p2d = tmp + 0x5a;		s1pad = tm2 + 0x5a;
		s1p2e = tmp + 0x5c;		s1pae = tm2 + 0x5c;
		s1p2f = tmp + 0x5e;		s1paf = tm2 + 0x5e;
		s1p30 = tmp + 0x60;		s1pb0 = tm2 + 0x60;
		s1p31 = tmp + 0x62;		s1pb1 = tm2 + 0x62;
		s1p32 = tmp + 0x64;		s1pb2 = tm2 + 0x64;
		s1p33 = tmp + 0x66;		s1pb3 = tm2 + 0x66;
		s1p34 = tmp + 0x68;		s1pb4 = tm2 + 0x68;
		s1p35 = tmp + 0x6a;		s1pb5 = tm2 + 0x6a;
		s1p36 = tmp + 0x6c;		s1pb6 = tm2 + 0x6c;
		s1p37 = tmp + 0x6e;		s1pb7 = tm2 + 0x6e;
		s1p38 = tmp + 0x70;		s1pb8 = tm2 + 0x70;
		s1p39 = tmp + 0x72;		s1pb9 = tm2 + 0x72;
		s1p3a = tmp + 0x74;		s1pba = tm2 + 0x74;
		s1p3b = tmp + 0x76;		s1pbb = tm2 + 0x76;
		s1p3c = tmp + 0x78;		s1pbc = tm2 + 0x78;
		s1p3d = tmp + 0x7a;		s1pbd = tm2 + 0x7a;
		s1p3e = tmp + 0x7c;		s1pbe = tm2 + 0x7c;
		s1p3f = tmp + 0x7e;		s1pbf = tm2 + 0x7e;
		s1p40 = tmp + 0x80;		s1pc0 = tm2 + 0x80;
		s1p41 = tmp + 0x82;		s1pc1 = tm2 + 0x82;
		s1p42 = tmp + 0x84;		s1pc2 = tm2 + 0x84;
		s1p43 = tmp + 0x86;		s1pc3 = tm2 + 0x86;
		s1p44 = tmp + 0x88;		s1pc4 = tm2 + 0x88;
		s1p45 = tmp + 0x8a;		s1pc5 = tm2 + 0x8a;
		s1p46 = tmp + 0x8c;		s1pc6 = tm2 + 0x8c;
		s1p47 = tmp + 0x8e;		s1pc7 = tm2 + 0x8e;
		s1p48 = tmp + 0x90;		s1pc8 = tm2 + 0x90;
		s1p49 = tmp + 0x92;		s1pc9 = tm2 + 0x92;
		s1p4a = tmp + 0x94;		s1pca = tm2 + 0x94;
		s1p4b = tmp + 0x96;		s1pcb = tm2 + 0x96;
		s1p4c = tmp + 0x98;		s1pcc = tm2 + 0x98;
		s1p4d = tmp + 0x9a;		s1pcd = tm2 + 0x9a;
		s1p4e = tmp + 0x9c;		s1pce = tm2 + 0x9c;
		s1p4f = tmp + 0x9e;		s1pcf = tm2 + 0x9e;
		s1p50 = tmp + 0xa0;		s1pd0 = tm2 + 0xa0;
		s1p51 = tmp + 0xa2;		s1pd1 = tm2 + 0xa2;
		s1p52 = tmp + 0xa4;		s1pd2 = tm2 + 0xa4;
		s1p53 = tmp + 0xa6;		s1pd3 = tm2 + 0xa6;
		s1p54 = tmp + 0xa8;		s1pd4 = tm2 + 0xa8;
		s1p55 = tmp + 0xaa;		s1pd5 = tm2 + 0xaa;
		s1p56 = tmp + 0xac;		s1pd6 = tm2 + 0xac;
		s1p57 = tmp + 0xae;		s1pd7 = tm2 + 0xae;
		s1p58 = tmp + 0xb0;		s1pd8 = tm2 + 0xb0;
		s1p59 = tmp + 0xb2;		s1pd9 = tm2 + 0xb2;
		s1p5a = tmp + 0xb4;		s1pda = tm2 + 0xb4;
		s1p5b = tmp + 0xb6;		s1pdb = tm2 + 0xb6;
		s1p5c = tmp + 0xb8;		s1pdc = tm2 + 0xb8;
		s1p5d = tmp + 0xba;		s1pdd = tm2 + 0xba;
		s1p5e = tmp + 0xbc;		s1pde = tm2 + 0xbc;
		s1p5f = tmp + 0xbe;		s1pdf = tm2 + 0xbe;
		s1p60 = tmp + 0xc0;		s1pe0 = tm2 + 0xc0;
		s1p61 = tmp + 0xc2;		s1pe1 = tm2 + 0xc2;
		s1p62 = tmp + 0xc4;		s1pe2 = tm2 + 0xc4;
		s1p63 = tmp + 0xc6;		s1pe3 = tm2 + 0xc6;
		s1p64 = tmp + 0xc8;		s1pe4 = tm2 + 0xc8;
		s1p65 = tmp + 0xca;		s1pe5 = tm2 + 0xca;
		s1p66 = tmp + 0xcc;		s1pe6 = tm2 + 0xcc;
		s1p67 = tmp + 0xce;		s1pe7 = tm2 + 0xce;
		s1p68 = tmp + 0xd0;		s1pe8 = tm2 + 0xd0;
		s1p69 = tmp + 0xd2;		s1pe9 = tm2 + 0xd2;
		s1p6a = tmp + 0xd4;		s1pea = tm2 + 0xd4;
		s1p6b = tmp + 0xd6;		s1peb = tm2 + 0xd6;
		s1p6c = tmp + 0xd8;		s1pec = tm2 + 0xd8;
		s1p6d = tmp + 0xda;		s1ped = tm2 + 0xda;
		s1p6e = tmp + 0xdc;		s1pee = tm2 + 0xdc;
		s1p6f = tmp + 0xde;		s1pef = tm2 + 0xde;
		s1p70 = tmp + 0xe0;		s1pf0 = tm2 + 0xe0;
		s1p71 = tmp + 0xe2;		s1pf1 = tm2 + 0xe2;
		s1p72 = tmp + 0xe4;		s1pf2 = tm2 + 0xe4;
		s1p73 = tmp + 0xe6;		s1pf3 = tm2 + 0xe6;
		s1p74 = tmp + 0xe8;		s1pf4 = tm2 + 0xe8;
		s1p75 = tmp + 0xea;		s1pf5 = tm2 + 0xea;
		s1p76 = tmp + 0xec;		s1pf6 = tm2 + 0xec;
		s1p77 = tmp + 0xee;		s1pf7 = tm2 + 0xee;
		s1p78 = tmp + 0xf0;		s1pf8 = tm2 + 0xf0;
		s1p79 = tmp + 0xf2;		s1pf9 = tm2 + 0xf2;
		s1p7a = tmp + 0xf4;		s1pfa = tm2 + 0xf4;
		s1p7b = tmp + 0xf6;		s1pfb = tm2 + 0xf6;
		s1p7c = tmp + 0xf8;		s1pfc = tm2 + 0xf8;
		s1p7d = tmp + 0xfa;		s1pfd = tm2 + 0xfa;
		s1p7e = tmp + 0xfc;		s1pfe = tm2 + 0xfc;
		s1p7f = tmp + 0xfe;		s1pff = tm2 + 0xfe;
		tmp += 0x200;
		two    = tmp + 0;	// AVX+ versions of various DFT macros assume consts 2.0,1.0,isrt2 laid out thusly
		one    = tmp + 1;
		sqrt2  = tmp + 2;
		isrt2  = tmp + 3;	// Radix-16 DFT macros assume [isrt2,cc0,ss0] contiguous in memory
		cc0    = tmp + 4;
		ss0    = tmp + 5;
		tmp += 0x6;
		// ptrs to 16 sets (2*15 = 30 vec_dbl data each) of non-unity twiddles shared by the 2nd-half DIF and DIT DFT macros:
		twid0  = tmp + 0x00;
		twid1  = tmp + 0x1e;
		twid2  = tmp + 0x3c;
		twid3  = tmp + 0x5a;
		twid4  = tmp + 0x78;
		twid5  = tmp + 0x96;
		twid6  = tmp + 0xb4;
		twid7  = tmp + 0xd2;
		twid8  = tmp + 0xf0;
		twid9  = tmp + 0x10e;
		twida  = tmp + 0x12c;
		twidb  = tmp + 0x14a;
		twidc  = tmp + 0x168;
		twidd  = tmp + 0x186;
		twide  = tmp + 0x1a4;
		twidf  = tmp + 0x1c2;
		tmp += 0x1e0;	// += 16*30 = 16*0x1e => sc_ptr + 0x5e6
	  #ifdef USE_AVX512
		cy_r = tmp;	cy_i = tmp+0x20;	tmp += 2*0x20;	// RADIX/8 vec_dbl slots for each of cy_r and cy_i carry sub-arrays
		max_err = tmp + 0x00;
		sse2_rnd= tmp + 0x01;
		half_arr= tmp + 0x02;
	  #elif defined(USE_AVX)
		cy_r = tmp;	cy_i = tmp+0x40;	tmp += 2*0x40;	// RADIX/4 vec_dbl slots for each of cy_r and cy_i carry sub-arrays
		max_err = tmp + 0x00;
		sse2_rnd= tmp + 0x01;	// 0x666 +2 = 0x668 = 1640 vec_dbl
		// This is where the value of half_arr_offset comes from
		half_arr= tmp + 0x02;	/* This table needs 68 vec_dbl for Mersenne-mod, and 3.5*RADIX[avx] | RADIX[sse2] for Fermat-mod */
	  #else
		cy_r = tmp;	cy_i = tmp+0x80;	tmp += 2*0x80;	// RADIX/2 vec_dbl slots for each of cy_r and cy_i carry sub-arrays
		max_err = tmp + 0x00;
		sse2_rnd= tmp + 0x01;	// 0x6e6 +2 = 0x6e8 = 1768 complex
		// This is where the value of half_arr_offset comes from
		half_arr= tmp + 0x02;	/* This table needs 20 x 16 bytes for Mersenne-mod, 2 for Fermat-mod */
	  #endif

		ASSERT(HERE, half_arr_offset256 == (uint32)(half_arr-sc_ptr), "half_arr_offset mismatches actual!");
		ASSERT(HERE, (radix256_creals_in_local_store << l2_sz_vd) >= ((long)half_arr - (long)r00) + (20 << l2_sz_vd), "radix256_creals_in_local_store checksum failed!");

		/* These remain fixed: */
		VEC_DBL_INIT(two  , 2.0  );	VEC_DBL_INIT(one  , 1.0  );
		// 2 unnamed slots for alternate "rounded the other way" copies of sqrt2,isrt2:
	  #if 1
		dtmp = *(double *)&sqrt2_dn;	VEC_DBL_INIT(sqrt2, dtmp);
		dtmp = *(double *)&isrt2_dn;	VEC_DBL_INIT(isrt2, dtmp);
	  #else
		VEC_DBL_INIT(sqrt2, SQRT2);	VEC_DBL_INIT(isrt2, ISRT2);
	  #endif
		VEC_DBL_INIT(cc0  ,  c16);	VEC_DBL_INIT(ss0  ,  s16);
		/* SSE2 math = 53-mantissa-bit IEEE double-float: */
	  #ifdef USE_AVX512	// In AVX-512 mode, use VRNDSCALEPD for rounding and hijack this vector-data slot for the 4 base/baseinv-consts
		sse2_rnd->d0 = base[0]; sse2_rnd->d1 = baseinv[1]; sse2_rnd->d2 = wts_mult[1]; sse2_rnd->d3 = inv_mult[0];
	  #else
		VEC_DBL_INIT(sse2_rnd, crnd);
	  #endif

		// ptrs to 16 sets (30 vec_dbl data each) of non-unity twiddles shared by the 2nd-half DIF and DIT DFT macros.
		// Since we copied the init-blocks here from the code below in which the twiddle-sets appear in BR order, init same way:

		// Use loop-based init for codecompactness and extensibility to larger pow2 radices:
	  #ifdef USE_AVX2
		// The first 2 sets (twid0/8) are processed in non-FMA fashion by both DIF/DIT macros, so init in non-FMA fashion:
		for(l = 0; l < 2; l++) {
	  #else
		for(l = 0; l < 16; l++) {
	  #endif
			j = reverse(l,16)<<1;	// twid0-offsets are processed in BR16 order
			tmp = twid0 + (j<<4)-j;	// Twid-offsets are multiples of 30 vec_dbl
			addr = DFT256_TWIDDLES[l]; addi = addr+1;	// Pointer to required row of 2-D twiddles array
			VEC_DBL_INIT(tmp,*(addr+0x00)); tmp++;	VEC_DBL_INIT(tmp,*(addi+0x00)); tmp++;
			VEC_DBL_INIT(tmp,*(addr+0x02)); tmp++;	VEC_DBL_INIT(tmp,*(addi+0x02)); tmp++;
			VEC_DBL_INIT(tmp,*(addr+0x04)); tmp++;	VEC_DBL_INIT(tmp,*(addi+0x04)); tmp++;
			VEC_DBL_INIT(tmp,*(addr+0x06)); tmp++;	VEC_DBL_INIT(tmp,*(addi+0x06)); tmp++;
			VEC_DBL_INIT(tmp,*(addr+0x08)); tmp++;	VEC_DBL_INIT(tmp,*(addi+0x08)); tmp++;
			VEC_DBL_INIT(tmp,*(addr+0x0a)); tmp++;	VEC_DBL_INIT(tmp,*(addi+0x0a)); tmp++;
			VEC_DBL_INIT(tmp,*(addr+0x0c)); tmp++;	VEC_DBL_INIT(tmp,*(addi+0x0c)); tmp++;
			VEC_DBL_INIT(tmp,*(addr+0x0e)); tmp++;	VEC_DBL_INIT(tmp,*(addi+0x0e)); tmp++;
			VEC_DBL_INIT(tmp,*(addr+0x10)); tmp++;	VEC_DBL_INIT(tmp,*(addi+0x10)); tmp++;
			VEC_DBL_INIT(tmp,*(addr+0x12)); tmp++;	VEC_DBL_INIT(tmp,*(addi+0x12)); tmp++;
			VEC_DBL_INIT(tmp,*(addr+0x14)); tmp++;	VEC_DBL_INIT(tmp,*(addi+0x14)); tmp++;
			VEC_DBL_INIT(tmp,*(addr+0x16)); tmp++;	VEC_DBL_INIT(tmp,*(addi+0x16)); tmp++;
			VEC_DBL_INIT(tmp,*(addr+0x18)); tmp++;	VEC_DBL_INIT(tmp,*(addi+0x18)); tmp++;
			VEC_DBL_INIT(tmp,*(addr+0x1a)); tmp++;	VEC_DBL_INIT(tmp,*(addi+0x1a)); tmp++;
			VEC_DBL_INIT(tmp,*(addr+0x1c)); tmp++;	VEC_DBL_INIT(tmp,*(addi+0x1c)); tmp++;
		}

		// The remaining 14 sets are inited differently depending on whether SIMD+FMA is used:
	  #ifdef USE_AVX2

		// Precompute the FMA-modified twiddles for the 2nd-pass radix-16 DFTs:
		#ifdef USE_FMA
			#error USE_FMA flag not supported in SIMD mode - to use FMA under AVX2/FMA3, define *only* USE_AVX2!
		#endif

		#include "radix16_dif_dit_pass_asm.h"	// Need this for FMA_TWIDDLE_FIDDLE macro

		// Init the vec_dbl const 1.0:
		for(l = 2; l < 16; l++) {
			j = reverse(l,16)<<1;	// twid0-offsets are processed in BR16 order
			tmp = twid0 + (j<<4)-j;	// Twid-offsets are multiples of 30 vec_dbl
			addr = DFT256_TWIDDLES[l]; addi = addr+1;	// Pointer to required row of 2-D twiddles array
			FMA_TWIDDLE_FIDDLE(
				*(addr+0x00),*(addi+0x00), *(addr+0x02),*(addi+0x02), *(addr+0x04),*(addi+0x04), *(addr+0x06),*(addi+0x06), *(addr+0x08),*(addi+0x08), *(addr+0x0a),*(addi+0x0a), *(addr+0x0c),*(addi+0x0c), *(addr+0x0e),*(addi+0x0e), *(addr+0x10),*(addi+0x10), *(addr+0x12),*(addi+0x12), *(addr+0x14),*(addi+0x14), *(addr+0x16),*(addi+0x16), *(addr+0x18),*(addi+0x18), *(addr+0x1a),*(addi+0x1a), *(addr+0x1c),*(addi+0x1c),
				c16,tan,
				tmp
			)
		}

	#endif

		// Propagate the above consts to the remaining threads:
		nbytes = (long)cy_r - (long)two;	// #bytes in 1st of above block of consts
		tmp = two;
		tm2 = tmp + cslots_in_local_store;
		for(ithread = 1; ithread < CY_THREADS; ++ithread) {
			memcpy(tm2, tmp, nbytes);
			tmp = tm2;		tm2 += cslots_in_local_store;
		}
		nbytes = sz_vd;	// sse2_rnd is a solo (in the SIMD-vector) datum
		tmp = sse2_rnd;
		tm2 = tmp + cslots_in_local_store;
		for(ithread = 1; ithread < CY_THREADS; ++ithread) {
			memcpy(tm2, tmp, nbytes);
			tmp = tm2;		tm2 += cslots_in_local_store;
		}

		/* SSE2 version of the one_half array - we have a 2-bit lookup, low bit is from the low word of the carry pair,
		high bit from the high, i.e. based on this lookup index [listed with LSB at right], we have:

			index	half_lo	half_hi
			00		1.0		1.0
			01		.50		1.0
			10		1.0		.50
			11		.50		.50

		The inverse-weights computation uses a similar table, but with all entries multiplied by .50:

			index2	half_lo	half_hi
			00		.50		.50
			01		.25		.50
			10		.50		.25
			11		.25		.25

		We do similarly for the base[] and baseinv[] table lookups - each of these get 4 further slots in half_arr.
		We also allocate a further 4 16-byte slots [uninitialized] for storage of the wtl,wtn,wtlp1,wtnm1 locals.

		In 4-way SIMD (AVX) mode, we expand this from 2^2 2-vector table entries to 2^4 4-vector entries.
		*/
		tmp = half_arr;

		if(TRANSFORM_TYPE == RIGHT_ANGLE)
		{
			/* In Fermat-mod mode, use first 2 SIMD-sized slots for base and 1/base: */
			VEC_DBL_INIT(tmp, base   [0]);	++tmp;
			VEC_DBL_INIT(tmp, baseinv[0]);	++tmp;
			/* [+2] slot is for [scale,scale] */

			// Propagate the above consts to the remaining threads:
			nbytes = 2 << l2_sz_vd;
			tmp = half_arr;
			tm2 = tmp + cslots_in_local_store;
			for(ithread = 1; ithread < CY_THREADS; ++ithread) {
				memcpy(tm2, tmp, nbytes);
				tmp = tm2;		tm2 += cslots_in_local_store;
			}

		#ifdef USE_AVX

			base_negacyclic_root = half_arr + RADIX;

			/*
			The pattern of the negacyclic-DWT-weights ("nDWTs") applied to the RADIX complex outputs of the final-DIT-pass is like so:
			The nDWTs multiplying each set of RADIX DIT DFT outputs are simply the product of a single complex-root "base multiplier" rbase
			(separately computed for each batch of DFT outputs), which "base root" multiplies the (0 - RADIX-1)st [4*RADIX]th roots of unity,
			i.e.
				 rbase * (j*I*Pi/2)/RADIX, for j = 0, ..., RADIX-1 .

			See the radix28 version of this routine for additional details.
			*/
			#if 0
			// Simple qfloat-based loop to crank out the roots which populate the radix256_avx_negadwt_consts table:
				struct qfloat qc,qs,qx,qy,qt,qn,qmul;
				qt = qfdiv(QPIHALF, i64_to_q((int64)RADIX));	// Pi/2/RADIX
				qc = qfcos(qt);	qs = qfsin(qt);
				qx = QONE;		qy = QZRO;
				for(j = 0; j < RADIX; j++) {
					printf("j = %3u: cos = 0x%16llX\n",j,qfdbl_as_uint64(qx));
					// Up-multiply the complex exponential:
					qn = qfmul(qx, qc); qt = qfmul(qy, qs); qmul = qfsub(qn, qt);	// Store qxnew in qmul for now.
					qn = qfmul(qx, qs); qt = qfmul(qy, qc); qy   = qfadd(qn, qt); qx = qmul;
				}
				exit(0);
			#endif

		  #ifdef USE_AVX512	// 8-way-double analog of AVX inits below:

			tmp = base_negacyclic_root + 2*RADIX;	// First 2*RADIX slots reserved for RADIX/8 copies of the Re/Im parts of the 8 base multipliers
			tm2 = tmp + RADIX/4 - 1;
			// First elt-pair needs special handling - have the 1.0 in avx_negadwt_consts[0] but the sine term buggers things
			tmp->d0 = 1.0;	(tmp+1)->d0 = 0.0;
			tmp64 = radix256_avx_negadwt_consts[1];	tmp->d1 = tm2->d7 = *(double *)&tmp64;
			tmp64 = radix256_avx_negadwt_consts[2];	tmp->d2 = tm2->d6 = *(double *)&tmp64;
			tmp64 = radix256_avx_negadwt_consts[3];	tmp->d3 = tm2->d5 = *(double *)&tmp64;
			tmp64 = radix256_avx_negadwt_consts[4];	tmp->d4 = tm2->d4 = *(double *)&tmp64;
			tmp64 = radix256_avx_negadwt_consts[5];	tmp->d5 = tm2->d3 = *(double *)&tmp64;
			tmp64 = radix256_avx_negadwt_consts[6];	tmp->d6 = tm2->d2 = *(double *)&tmp64;
			tmp64 = radix256_avx_negadwt_consts[7];	tmp->d7 = tm2->d1 = *(double *)&tmp64;	tmp += 2;
			for(j = 8; j < RADIX; j += 8) {
				tmp64 = radix256_avx_negadwt_consts[j+0];	tmp->d0 = tm2->d0 = *(double *)&tmp64;	tm2 -= 2;
				tmp64 = radix256_avx_negadwt_consts[j+1];	tmp->d1 = tm2->d7 = *(double *)&tmp64;
				tmp64 = radix256_avx_negadwt_consts[j+2];	tmp->d2 = tm2->d6 = *(double *)&tmp64;
				tmp64 = radix256_avx_negadwt_consts[j+3];	tmp->d3 = tm2->d5 = *(double *)&tmp64;
				tmp64 = radix256_avx_negadwt_consts[j+4];	tmp->d4 = tm2->d4 = *(double *)&tmp64;
				tmp64 = radix256_avx_negadwt_consts[j+5];	tmp->d5 = tm2->d3 = *(double *)&tmp64;
				tmp64 = radix256_avx_negadwt_consts[j+6];	tmp->d6 = tm2->d2 = *(double *)&tmp64;
				tmp64 = radix256_avx_negadwt_consts[j+7];	tmp->d7 = tm2->d1 = *(double *)&tmp64;	tmp += 2;
			}
			tmp = base_negacyclic_root + RADIX*2;	// reset to point to start of above block
	
		  #else

			tmp = base_negacyclic_root + RADIX*2;	// First 2*RADIX slots reserved for RADIX/4 copies of the Re/Im parts of the 4 base multipliers
			tm2 = tmp + RADIX/2 - 1;
			// First elt-pair needs special handling - have the 1.0 in avx_negadwt_consts[0] but the sine term buggers things
			tmp->d0 = 1.0;	(tmp+1)->d0 = 0.0;
			tmp64 = radix256_avx_negadwt_consts[1];	tmp->d1 = tm2->d3 = *(double *)&tmp64;	/* cos(  1*I*Pi/(2*RADIX)) = sin((RADIX-  1)*I*Pi/(2*RADIX)) */
			tmp64 = radix256_avx_negadwt_consts[2];	tmp->d2 = tm2->d2 = *(double *)&tmp64;	/* cos(  2*I*Pi/(2*RADIX)) = sin((RADIX-  2)*I*Pi/(2*RADIX)) */
			tmp64 = radix256_avx_negadwt_consts[3];	tmp->d3 = tm2->d1 = *(double *)&tmp64;	/* cos(  3*I*Pi/(2*RADIX)) = sin((RADIX-  3)*I*Pi/(2*RADIX)) */	tmp += 2;
			for(j = 4; j < RADIX; j += 4) {
				tmp64 = radix256_avx_negadwt_consts[j+0];	tmp->d0 = tm2->d0 = *(double *)&tmp64;	tm2 -= 2;
				tmp64 = radix256_avx_negadwt_consts[j+1];	tmp->d1 = tm2->d3 = *(double *)&tmp64;
				tmp64 = radix256_avx_negadwt_consts[j+2];	tmp->d2 = tm2->d2 = *(double *)&tmp64;
				tmp64 = radix256_avx_negadwt_consts[j+3];	tmp->d3 = tm2->d1 = *(double *)&tmp64;	tmp += 2;
			}

			tmp = base_negacyclic_root + RADIX*2;	// reset to point to start of above block

		  #endif

			nbytes = RADIX << 4;	// RADIX*sz_vd/2 [AVX] or RADIX*sz_vd/4 [AVX-512]; same #bytes in either case

			// Propagate the above consts to the remaining threads:
			tm2 = tmp + cslots_in_local_store;
			for(ithread = 1; ithread < CY_THREADS; ++ithread) {
				memcpy(tm2, tmp, nbytes);
				tmp = tm2;		tm2 += cslots_in_local_store;
			}

		#endif	// AVX?
		}
		else
		{
		#ifdef USE_AVX512
			// Each lookup-category in the 'mini-tables' used in AVX mode balloons from 16x32-bytes to 64x64-bytes,
			// so switch to an opmask-based scheme which starts with e.g. a broadcast constant and onditional doubling.
			// Here are the needed consts and opmasks:
			// [1] Fwd-wt multipliers: Init = 0.50 x 8, anytime AVX-style lookup into 1st table below would have bit = 0, double the corr. datum
			// [2] Inv-wt multipliers: Init = 0.25 x 8, anytime AVX-style lookup into 2nd table below would have bit = 0, double the corr. datum
			// [3] Fwd-base mults: Init = base[0] x 8, anytime AVX-style lookup into 3rd table below would have bit = 1, double the corr. datum
			// [4] Inv-base mults: Init = binv[1] x 8, anytime AVX-style lookup into 4th table below would have bit = 0, double the corr. datum
			// [5] [LOACC] Init = wts_mult[1] x 8, anytime AVX-style lookup into 5th table below would have bit = 0, double the corr. datum
			// [6] [LOACC] Init = inv_mult[0] x 8, anytime AVX-style lookup into 6th table below would have bit = 1, double the corr. datum
			nbytes = 0;
		#elif defined(USE_AVX)
			/* Forward-weight multipliers: */
			tmp->d0 = 1.0;	tmp->d1 = 1.0;	tmp->d2 = 1.0;	tmp->d3 = 1.0;	++tmp;
			tmp->d0 = .50;	tmp->d1 = 1.0;	tmp->d2 = 1.0;	tmp->d3 = 1.0;	++tmp;
			tmp->d0 = 1.0;	tmp->d1 = .50;	tmp->d2 = 1.0;	tmp->d3 = 1.0;	++tmp;
			tmp->d0 = .50;	tmp->d1 = .50;	tmp->d2 = 1.0;	tmp->d3 = 1.0;	++tmp;
			tmp->d0 = 1.0;	tmp->d1 = 1.0;	tmp->d2 = .50;	tmp->d3 = 1.0;	++tmp;
			tmp->d0 = .50;	tmp->d1 = 1.0;	tmp->d2 = .50;	tmp->d3 = 1.0;	++tmp;
			tmp->d0 = 1.0;	tmp->d1 = .50;	tmp->d2 = .50;	tmp->d3 = 1.0;	++tmp;
			tmp->d0 = .50;	tmp->d1 = .50;	tmp->d2 = .50;	tmp->d3 = 1.0;	++tmp;
			tmp->d0 = 1.0;	tmp->d1 = 1.0;	tmp->d2 = 1.0;	tmp->d3 = .50;	++tmp;
			tmp->d0 = .50;	tmp->d1 = 1.0;	tmp->d2 = 1.0;	tmp->d3 = .50;	++tmp;
			tmp->d0 = 1.0;	tmp->d1 = .50;	tmp->d2 = 1.0;	tmp->d3 = .50;	++tmp;
			tmp->d0 = .50;	tmp->d1 = .50;	tmp->d2 = 1.0;	tmp->d3 = .50;	++tmp;
			tmp->d0 = 1.0;	tmp->d1 = 1.0;	tmp->d2 = .50;	tmp->d3 = .50;	++tmp;
			tmp->d0 = .50;	tmp->d1 = 1.0;	tmp->d2 = .50;	tmp->d3 = .50;	++tmp;
			tmp->d0 = 1.0;	tmp->d1 = .50;	tmp->d2 = .50;	tmp->d3 = .50;	++tmp;
			tmp->d0 = .50;	tmp->d1 = .50;	tmp->d2 = .50;	tmp->d3 = .50;	++tmp;
			/* Inverse-weight multipliers (only needed for mersenne-mod): */
			tmp->d0 = .50;	tmp->d1 = .50;	tmp->d2 = .50;	tmp->d3 = .50;	++tmp;
			tmp->d0 = .25;	tmp->d1 = .50;	tmp->d2 = .50;	tmp->d3 = .50;	++tmp;
			tmp->d0 = .50;	tmp->d1 = .25;	tmp->d2 = .50;	tmp->d3 = .50;	++tmp;
			tmp->d0 = .25;	tmp->d1 = .25;	tmp->d2 = .50;	tmp->d3 = .50;	++tmp;
			tmp->d0 = .50;	tmp->d1 = .50;	tmp->d2 = .25;	tmp->d3 = .50;	++tmp;
			tmp->d0 = .25;	tmp->d1 = .50;	tmp->d2 = .25;	tmp->d3 = .50;	++tmp;
			tmp->d0 = .50;	tmp->d1 = .25;	tmp->d2 = .25;	tmp->d3 = .50;	++tmp;
			tmp->d0 = .25;	tmp->d1 = .25;	tmp->d2 = .25;	tmp->d3 = .50;	++tmp;
			tmp->d0 = .50;	tmp->d1 = .50;	tmp->d2 = .50;	tmp->d3 = .25;	++tmp;
			tmp->d0 = .25;	tmp->d1 = .50;	tmp->d2 = .50;	tmp->d3 = .25;	++tmp;
			tmp->d0 = .50;	tmp->d1 = .25;	tmp->d2 = .50;	tmp->d3 = .25;	++tmp;
			tmp->d0 = .25;	tmp->d1 = .25;	tmp->d2 = .50;	tmp->d3 = .25;	++tmp;
			tmp->d0 = .50;	tmp->d1 = .50;	tmp->d2 = .25;	tmp->d3 = .25;	++tmp;
			tmp->d0 = .25;	tmp->d1 = .50;	tmp->d2 = .25;	tmp->d3 = .25;	++tmp;
			tmp->d0 = .50;	tmp->d1 = .25;	tmp->d2 = .25;	tmp->d3 = .25;	++tmp;
			tmp->d0 = .25;	tmp->d1 = .25;	tmp->d2 = .25;	tmp->d3 = .25;	++tmp;
			/* Forward-base[] multipliers: */
			tmp->d0 = base   [0];	tmp->d1 = base   [0];	tmp->d2 = base   [0];	tmp->d3 = base   [0];	++tmp;
			tmp->d0 = base   [1];	tmp->d1 = base   [0];	tmp->d2 = base   [0];	tmp->d3 = base   [0];	++tmp;
			tmp->d0 = base   [0];	tmp->d1 = base   [1];	tmp->d2 = base   [0];	tmp->d3 = base   [0];	++tmp;
			tmp->d0 = base   [1];	tmp->d1 = base   [1];	tmp->d2 = base   [0];	tmp->d3 = base   [0];	++tmp;
			tmp->d0 = base   [0];	tmp->d1 = base   [0];	tmp->d2 = base   [1];	tmp->d3 = base   [0];	++tmp;
			tmp->d0 = base   [1];	tmp->d1 = base   [0];	tmp->d2 = base   [1];	tmp->d3 = base   [0];	++tmp;
			tmp->d0 = base   [0];	tmp->d1 = base   [1];	tmp->d2 = base   [1];	tmp->d3 = base   [0];	++tmp;
			tmp->d0 = base   [1];	tmp->d1 = base   [1];	tmp->d2 = base   [1];	tmp->d3 = base   [0];	++tmp;
			tmp->d0 = base   [0];	tmp->d1 = base   [0];	tmp->d2 = base   [0];	tmp->d3 = base   [1];	++tmp;
			tmp->d0 = base   [1];	tmp->d1 = base   [0];	tmp->d2 = base   [0];	tmp->d3 = base   [1];	++tmp;
			tmp->d0 = base   [0];	tmp->d1 = base   [1];	tmp->d2 = base   [0];	tmp->d3 = base   [1];	++tmp;
			tmp->d0 = base   [1];	tmp->d1 = base   [1];	tmp->d2 = base   [0];	tmp->d3 = base   [1];	++tmp;
			tmp->d0 = base   [0];	tmp->d1 = base   [0];	tmp->d2 = base   [1];	tmp->d3 = base   [1];	++tmp;
			tmp->d0 = base   [1];	tmp->d1 = base   [0];	tmp->d2 = base   [1];	tmp->d3 = base   [1];	++tmp;
			tmp->d0 = base   [0];	tmp->d1 = base   [1];	tmp->d2 = base   [1];	tmp->d3 = base   [1];	++tmp;
			tmp->d0 = base   [1];	tmp->d1 = base   [1];	tmp->d2 = base   [1];	tmp->d3 = base   [1];	++tmp;
			/* Inverse-base[] multipliers: */
			tmp->d0 = baseinv[0];	tmp->d1 = baseinv[0];	tmp->d2 = baseinv[0];	tmp->d3 = baseinv[0];	++tmp;
			tmp->d0 = baseinv[1];	tmp->d1 = baseinv[0];	tmp->d2 = baseinv[0];	tmp->d3 = baseinv[0];	++tmp;
			tmp->d0 = baseinv[0];	tmp->d1 = baseinv[1];	tmp->d2 = baseinv[0];	tmp->d3 = baseinv[0];	++tmp;
			tmp->d0 = baseinv[1];	tmp->d1 = baseinv[1];	tmp->d2 = baseinv[0];	tmp->d3 = baseinv[0];	++tmp;
			tmp->d0 = baseinv[0];	tmp->d1 = baseinv[0];	tmp->d2 = baseinv[1];	tmp->d3 = baseinv[0];	++tmp;
			tmp->d0 = baseinv[1];	tmp->d1 = baseinv[0];	tmp->d2 = baseinv[1];	tmp->d3 = baseinv[0];	++tmp;
			tmp->d0 = baseinv[0];	tmp->d1 = baseinv[1];	tmp->d2 = baseinv[1];	tmp->d3 = baseinv[0];	++tmp;
			tmp->d0 = baseinv[1];	tmp->d1 = baseinv[1];	tmp->d2 = baseinv[1];	tmp->d3 = baseinv[0];	++tmp;
			tmp->d0 = baseinv[0];	tmp->d1 = baseinv[0];	tmp->d2 = baseinv[0];	tmp->d3 = baseinv[1];	++tmp;
			tmp->d0 = baseinv[1];	tmp->d1 = baseinv[0];	tmp->d2 = baseinv[0];	tmp->d3 = baseinv[1];	++tmp;
			tmp->d0 = baseinv[0];	tmp->d1 = baseinv[1];	tmp->d2 = baseinv[0];	tmp->d3 = baseinv[1];	++tmp;
			tmp->d0 = baseinv[1];	tmp->d1 = baseinv[1];	tmp->d2 = baseinv[0];	tmp->d3 = baseinv[1];	++tmp;
			tmp->d0 = baseinv[0];	tmp->d1 = baseinv[0];	tmp->d2 = baseinv[1];	tmp->d3 = baseinv[1];	++tmp;
			tmp->d0 = baseinv[1];	tmp->d1 = baseinv[0];	tmp->d2 = baseinv[1];	tmp->d3 = baseinv[1];	++tmp;
			tmp->d0 = baseinv[0];	tmp->d1 = baseinv[1];	tmp->d2 = baseinv[1];	tmp->d3 = baseinv[1];	++tmp;
			tmp->d0 = baseinv[1];	tmp->d1 = baseinv[1];	tmp->d2 = baseinv[1];	tmp->d3 = baseinv[1];	++tmp;
			// In LOACC mode, put wts_mult and their inverses in the first 32 slots below in place of the 1/2-stuff:
		  #ifdef LOACC
			/* wts_mult:*/
			tmp->d0 = wts_mult[0];	tmp->d1 = wts_mult[0];	tmp->d2 = wts_mult[0];	tmp->d3 = wts_mult[0];	++tmp;
			tmp->d0 = wts_mult[1];	tmp->d1 = wts_mult[0];	tmp->d2 = wts_mult[0];	tmp->d3 = wts_mult[0];	++tmp;
			tmp->d0 = wts_mult[0];	tmp->d1 = wts_mult[1];	tmp->d2 = wts_mult[0];	tmp->d3 = wts_mult[0];	++tmp;
			tmp->d0 = wts_mult[1];	tmp->d1 = wts_mult[1];	tmp->d2 = wts_mult[0];	tmp->d3 = wts_mult[0];	++tmp;
			tmp->d0 = wts_mult[0];	tmp->d1 = wts_mult[0];	tmp->d2 = wts_mult[1];	tmp->d3 = wts_mult[0];	++tmp;
			tmp->d0 = wts_mult[1];	tmp->d1 = wts_mult[0];	tmp->d2 = wts_mult[1];	tmp->d3 = wts_mult[0];	++tmp;
			tmp->d0 = wts_mult[0];	tmp->d1 = wts_mult[1];	tmp->d2 = wts_mult[1];	tmp->d3 = wts_mult[0];	++tmp;
			tmp->d0 = wts_mult[1];	tmp->d1 = wts_mult[1];	tmp->d2 = wts_mult[1];	tmp->d3 = wts_mult[0];	++tmp;
			tmp->d0 = wts_mult[0];	tmp->d1 = wts_mult[0];	tmp->d2 = wts_mult[0];	tmp->d3 = wts_mult[1];	++tmp;
			tmp->d0 = wts_mult[1];	tmp->d1 = wts_mult[0];	tmp->d2 = wts_mult[0];	tmp->d3 = wts_mult[1];	++tmp;
			tmp->d0 = wts_mult[0];	tmp->d1 = wts_mult[1];	tmp->d2 = wts_mult[0];	tmp->d3 = wts_mult[1];	++tmp;
			tmp->d0 = wts_mult[1];	tmp->d1 = wts_mult[1];	tmp->d2 = wts_mult[0];	tmp->d3 = wts_mult[1];	++tmp;
			tmp->d0 = wts_mult[0];	tmp->d1 = wts_mult[0];	tmp->d2 = wts_mult[1];	tmp->d3 = wts_mult[1];	++tmp;
			tmp->d0 = wts_mult[1];	tmp->d1 = wts_mult[0];	tmp->d2 = wts_mult[1];	tmp->d3 = wts_mult[1];	++tmp;
			tmp->d0 = wts_mult[0];	tmp->d1 = wts_mult[1];	tmp->d2 = wts_mult[1];	tmp->d3 = wts_mult[1];	++tmp;
			tmp->d0 = wts_mult[1];	tmp->d1 = wts_mult[1];	tmp->d2 = wts_mult[1];	tmp->d3 = wts_mult[1];	++tmp;
			/* inv_mult: */
			tmp->d0 = inv_mult[0];	tmp->d1 = inv_mult[0];	tmp->d2 = inv_mult[0];	tmp->d3 = inv_mult[0];	++tmp;
			tmp->d0 = inv_mult[1];	tmp->d1 = inv_mult[0];	tmp->d2 = inv_mult[0];	tmp->d3 = inv_mult[0];	++tmp;
			tmp->d0 = inv_mult[0];	tmp->d1 = inv_mult[1];	tmp->d2 = inv_mult[0];	tmp->d3 = inv_mult[0];	++tmp;
			tmp->d0 = inv_mult[1];	tmp->d1 = inv_mult[1];	tmp->d2 = inv_mult[0];	tmp->d3 = inv_mult[0];	++tmp;
			tmp->d0 = inv_mult[0];	tmp->d1 = inv_mult[0];	tmp->d2 = inv_mult[1];	tmp->d3 = inv_mult[0];	++tmp;
			tmp->d0 = inv_mult[1];	tmp->d1 = inv_mult[0];	tmp->d2 = inv_mult[1];	tmp->d3 = inv_mult[0];	++tmp;
			tmp->d0 = inv_mult[0];	tmp->d1 = inv_mult[1];	tmp->d2 = inv_mult[1];	tmp->d3 = inv_mult[0];	++tmp;
			tmp->d0 = inv_mult[1];	tmp->d1 = inv_mult[1];	tmp->d2 = inv_mult[1];	tmp->d3 = inv_mult[0];	++tmp;
			tmp->d0 = inv_mult[0];	tmp->d1 = inv_mult[0];	tmp->d2 = inv_mult[0];	tmp->d3 = inv_mult[1];	++tmp;
			tmp->d0 = inv_mult[1];	tmp->d1 = inv_mult[0];	tmp->d2 = inv_mult[0];	tmp->d3 = inv_mult[1];	++tmp;
			tmp->d0 = inv_mult[0];	tmp->d1 = inv_mult[1];	tmp->d2 = inv_mult[0];	tmp->d3 = inv_mult[1];	++tmp;
			tmp->d0 = inv_mult[1];	tmp->d1 = inv_mult[1];	tmp->d2 = inv_mult[0];	tmp->d3 = inv_mult[1];	++tmp;
			tmp->d0 = inv_mult[0];	tmp->d1 = inv_mult[0];	tmp->d2 = inv_mult[1];	tmp->d3 = inv_mult[1];	++tmp;
			tmp->d0 = inv_mult[1];	tmp->d1 = inv_mult[0];	tmp->d2 = inv_mult[1];	tmp->d3 = inv_mult[1];	++tmp;
			tmp->d0 = inv_mult[0];	tmp->d1 = inv_mult[1];	tmp->d2 = inv_mult[1];	tmp->d3 = inv_mult[1];	++tmp;
			tmp->d0 = inv_mult[1];	tmp->d1 = inv_mult[1];	tmp->d2 = inv_mult[1];	tmp->d3 = inv_mult[1];	++tmp;
			nbytes = 96 << l2_sz_vd;
		  #else
			nbytes = 64 << l2_sz_vd;
		  #endif

		#else	// USE_SSE2

			ctmp = (struct complex *)tmp;
			/* Forward-weight multipliers: */
			ctmp->re = 1.0;	ctmp->im = 1.0;	++ctmp;
			ctmp->re = .50;	ctmp->im = 1.0;	++ctmp;
			ctmp->re = 1.0;	ctmp->im = .50;	++ctmp;
			ctmp->re = .50;	ctmp->im = .50;	++ctmp;
			/* Inverse-weight multipliers (only needed for mersenne-mod): */
			ctmp->re = .50;	ctmp->im = .50;	++ctmp;
			ctmp->re = .25;	ctmp->im = .50;	++ctmp;
			ctmp->re = .50;	ctmp->im = .25;	++ctmp;
			ctmp->re = .25;	ctmp->im = .25;	++ctmp;
			/* Forward-base[] multipliers: */
			ctmp->re = base   [0];	ctmp->im = base   [0];	++ctmp;
			ctmp->re = base   [1];	ctmp->im = base   [0];	++ctmp;
			ctmp->re = base   [0];	ctmp->im = base   [1];	++ctmp;
			ctmp->re = base   [1];	ctmp->im = base   [1];	++ctmp;
			/* Inverse-base[] multipliers: */
			ctmp->re = baseinv[0];	ctmp->im = baseinv[0];	++ctmp;
			ctmp->re = baseinv[1];	ctmp->im = baseinv[0];	++ctmp;
			ctmp->re = baseinv[0];	ctmp->im = baseinv[1];	++ctmp;
			ctmp->re = baseinv[1];	ctmp->im = baseinv[1];	++ctmp;
			// In LOACC mode, put wts_mult and their inverses in the first 8 slots below in place of the 1/2-stuff:
		  #ifdef LOACC
			/* wts_mult:*/
			ctmp->re = wts_mult[0];	ctmp->im = wts_mult[0];	++ctmp;
			ctmp->re = wts_mult[1];	ctmp->im = wts_mult[0];	++ctmp;
			ctmp->re = wts_mult[0];	ctmp->im = wts_mult[1];	++ctmp;
			ctmp->re = wts_mult[1];	ctmp->im = wts_mult[1];	++ctmp;
			/* inv_mult:*/
			ctmp->re = inv_mult[0];	ctmp->im = inv_mult[0];	++ctmp;
			ctmp->re = inv_mult[1];	ctmp->im = inv_mult[0];	++ctmp;
			ctmp->re = inv_mult[0];	ctmp->im = inv_mult[1];	++ctmp;
			ctmp->re = inv_mult[1];	ctmp->im = inv_mult[1];	++ctmp;
			nbytes = 24 << l2_sz_vd;
		  #else
			nbytes = 16 << l2_sz_vd;
		  #endif

		#endif

			// Propagate the above consts to the remaining threads:
			tmp = half_arr;
			tm2 = tmp + cslots_in_local_store;
			for(ithread = 1; ithread < CY_THREADS; ++ithread) {
				memcpy(tm2, tmp, nbytes);
				tmp = tm2;		tm2 += cslots_in_local_store;
			}

		}	// TRANSFORM_TYPE toggle

		/* Floating-point sign mask used for FABS on packed doubles: */
		sign_mask = sm_ptr;
		for(i = 0; i < RE_IM_STRIDE; ++i) {
			*(sign_mask+i) = (uint64)0x7FFFFFFFFFFFFFFFull;
		}
		// Set up the SIMD-tupled-32-bit-int SSE constants used by the carry macros:
		sse_bw  = sm_ptr + RE_IM_STRIDE;	// (#doubles in a SIMD complex) x 32-bits = RE_IM_STRIDE x 64-bits
		tmp64 = (uint64)bw;
		tmp64 = tmp64 + (tmp64 << 32);
		for(i = 0; i < RE_IM_STRIDE; ++i) {
			*(sse_bw+i) = tmp64;
		}

		sse_sw  = sse_bw + RE_IM_STRIDE;
		tmp64 = (uint64)sw;
		tmp64 = tmp64 + (tmp64 << 32);
		for(i = 0; i < RE_IM_STRIDE; ++i) {
			*(sse_sw+i) = tmp64;
		}

		sse_nm1 = sse_sw + RE_IM_STRIDE;
		tmp64 = (uint64)nm1;
		tmp64 = tmp64 + (tmp64 << 32);
		for(i = 0; i < RE_IM_STRIDE; ++i) {
			*(sse_nm1 +i) = tmp64;
		}

		nbytes = 4 << l2_sz_vd;

	  #ifdef USE_AVX512
	   #ifdef CARRY_16_WAY
		n_minus_sil   = (struct uint32x16*)sse_nm1 + 1;
		n_minus_silp1 = (struct uint32x16*)sse_nm1 + 2;
		sinwt         = (struct uint32x16*)sse_nm1 + 3;
		sinwtm1       = (struct uint32x16*)sse_nm1 + 4;
		nbytes += 256;
	   #else
		n_minus_sil   = (struct uint32x8 *)sse_nm1 + 1;
		n_minus_silp1 = (struct uint32x8 *)sse_nm1 + 2;
		sinwt         = (struct uint32x8 *)sse_nm1 + 3;
		sinwtm1       = (struct uint32x8 *)sse_nm1 + 4;
		nbytes += 128;
	   #endif
	  #elif defined(USE_AVX)
		n_minus_sil   = (struct uint32x4 *)sse_nm1 + 1;
		n_minus_silp1 = (struct uint32x4 *)sse_nm1 + 2;
		sinwt         = (struct uint32x4 *)sse_nm1 + 3;
		sinwtm1       = (struct uint32x4 *)sse_nm1 + 4;
		nbytes += 64;
	  #endif

		// Propagate the above consts to the remaining threads:
		tmp = (vec_dbl *)sm_ptr;
		tm2 = tmp + cslots_in_local_store;
		for(ithread = 1; ithread < CY_THREADS; ++ithread) {
			memcpy(tm2, tmp, nbytes);
			tmp = tm2;		tm2 += cslots_in_local_store;
		}

		// sse_nm1, sinwtm1 are both uint64-pointers, i.e. RE_IM_STRIDE such data per vec_dbl-sized slot:
	  #ifdef USE_AVX
		bjmodn = (int*)(sinwtm1 + RE_IM_STRIDE);
	  #else
		bjmodn = (int*)(sse_nm1 + RE_IM_STRIDE);
	  #endif

	#endif	// USE_SSE2

		/*   constant index offsets for load/stores are here.	*/
		pini = NDIVR/CY_THREADS;
		p01 = NDIVR;
		p02 = p01 + p01;
		p03 = p02 + p01;
		p04 = p03 + p01;
		p05 = p04 + p01;
		p06 = p05 + p01;
		p07 = p06 + p01;
		p08 = p07 + p01;
		p09 = p08 + p01;
		p0a = p09 + p01;
		p0b = p0a + p01;
		p0c = p0b + p01;
		p0d = p0c + p01;
		p0e = p0d + p01;
		p0f = p0e + p01;
		p10 = p08 + p08;
		p20 = p10 + p10;
		p30 = p20 + p10;
		p40 = p30 + p10;
		p50 = p40 + p10;
		p60 = p50 + p10;
		p70 = p60 + p10;
		p80 = p70 + p10;
		p90 = p80 + p10;
		pa0 = p90 + p10;
		pb0 = pa0 + p10;
		pc0 = pb0 + p10;
		pd0 = pc0 + p10;
		pe0 = pd0 + p10;
		pf0 = pe0 + p10;

		p01 = p01 + ( (p01 >> DAT_BITS) << PAD_BITS );
		p02 = p02 + ( (p02 >> DAT_BITS) << PAD_BITS );
		p03 = p03 + ( (p03 >> DAT_BITS) << PAD_BITS );
		p04 = p04 + ( (p04 >> DAT_BITS) << PAD_BITS );
		p05 = p05 + ( (p05 >> DAT_BITS) << PAD_BITS );
		p06 = p06 + ( (p06 >> DAT_BITS) << PAD_BITS );
		p07 = p07 + ( (p07 >> DAT_BITS) << PAD_BITS );
		p08 = p08 + ( (p08 >> DAT_BITS) << PAD_BITS );
		p09 = p09 + ( (p09 >> DAT_BITS) << PAD_BITS );
		p0a = p0a + ( (p0a >> DAT_BITS) << PAD_BITS );
		p0b = p0b + ( (p0b >> DAT_BITS) << PAD_BITS );
		p0c = p0c + ( (p0c >> DAT_BITS) << PAD_BITS );
		p0d = p0d + ( (p0d >> DAT_BITS) << PAD_BITS );
		p0e = p0e + ( (p0e >> DAT_BITS) << PAD_BITS );
		p0f = p0f + ( (p0f >> DAT_BITS) << PAD_BITS );
		p10 = p10 + ( (p10 >> DAT_BITS) << PAD_BITS );
		p20 = p20 + ( (p20 >> DAT_BITS) << PAD_BITS );
		p30 = p30 + ( (p30 >> DAT_BITS) << PAD_BITS );
		p40 = p40 + ( (p40 >> DAT_BITS) << PAD_BITS );
		p50 = p50 + ( (p50 >> DAT_BITS) << PAD_BITS );
		p60 = p60 + ( (p60 >> DAT_BITS) << PAD_BITS );
		p70 = p70 + ( (p70 >> DAT_BITS) << PAD_BITS );
		p80 = p80 + ( (p80 >> DAT_BITS) << PAD_BITS );
		p90 = p90 + ( (p90 >> DAT_BITS) << PAD_BITS );
		pa0 = pa0 + ( (pa0 >> DAT_BITS) << PAD_BITS );
		pb0 = pb0 + ( (pb0 >> DAT_BITS) << PAD_BITS );
		pc0 = pc0 + ( (pc0 >> DAT_BITS) << PAD_BITS );
		pd0 = pd0 + ( (pd0 >> DAT_BITS) << PAD_BITS );
		pe0 = pe0 + ( (pe0 >> DAT_BITS) << PAD_BITS );
		pf0 = pf0 + ( (pf0 >> DAT_BITS) << PAD_BITS );

		poff[     0] =   0; poff[     1] =     p04; poff[     2] =     p08; poff[     3] =     p0c;
		poff[0x04+0] = p10; poff[0x04+1] = p10+p04; poff[0x04+2] = p10+p08; poff[0x04+3] = p10+p0c;
		poff[0x08+0] = p20; poff[0x08+1] = p20+p04; poff[0x08+2] = p20+p08; poff[0x08+3] = p20+p0c;
		poff[0x0c+0] = p30; poff[0x0c+1] = p30+p04; poff[0x0c+2] = p30+p08; poff[0x0c+3] = p30+p0c;
		poff[0x10+0] = p40; poff[0x10+1] = p40+p04; poff[0x10+2] = p40+p08; poff[0x10+3] = p40+p0c;
		poff[0x14+0] = p50; poff[0x14+1] = p50+p04; poff[0x14+2] = p50+p08; poff[0x14+3] = p50+p0c;
		poff[0x18+0] = p60; poff[0x18+1] = p60+p04; poff[0x18+2] = p60+p08; poff[0x18+3] = p60+p0c;
		poff[0x1c+0] = p70; poff[0x1c+1] = p70+p04; poff[0x1c+2] = p70+p08; poff[0x1c+3] = p70+p0c;
		poff[0x20+0] = p80; poff[0x20+1] = p80+p04; poff[0x20+2] = p80+p08; poff[0x20+3] = p80+p0c;
		poff[0x24+0] = p90; poff[0x24+1] = p90+p04; poff[0x24+2] = p90+p08; poff[0x24+3] = p90+p0c;
		poff[0x28+0] = pa0; poff[0x28+1] = pa0+p04; poff[0x28+2] = pa0+p08; poff[0x28+3] = pa0+p0c;
		poff[0x2c+0] = pb0; poff[0x2c+1] = pb0+p04; poff[0x2c+2] = pb0+p08; poff[0x2c+3] = pb0+p0c;
		poff[0x30+0] = pc0; poff[0x30+1] = pc0+p04; poff[0x30+2] = pc0+p08; poff[0x30+3] = pc0+p0c;
		poff[0x34+0] = pd0; poff[0x34+1] = pd0+p04; poff[0x34+2] = pd0+p08; poff[0x34+3] = pd0+p0c;
		poff[0x38+0] = pe0; poff[0x38+1] = pe0+p04; poff[0x38+2] = pe0+p08; poff[0x38+3] = pe0+p0c;
		poff[0x3c+0] = pf0; poff[0x3c+1] = pf0+p04; poff[0x3c+2] = pf0+p08; poff[0x3c+3] = pf0+p0c;

	#if USE_SCALAR_DFT_MACRO
		// Set array offsets for in/outputs - low parts in fisrt 16 slots, high parts in next 16:
		dft_offsets_lo[0x0] = 0  ;	dft_offsets_hi[0x0] = 0  ;
		dft_offsets_lo[0x1] = p01;	dft_offsets_hi[0x1] = p10;
		dft_offsets_lo[0x2] = p02;	dft_offsets_hi[0x2] = p20;
		dft_offsets_lo[0x3] = p03;	dft_offsets_hi[0x3] = p30;
		dft_offsets_lo[0x4] = p04;	dft_offsets_hi[0x4] = p40;
		dft_offsets_lo[0x5] = p05;	dft_offsets_hi[0x5] = p50;
		dft_offsets_lo[0x6] = p06;	dft_offsets_hi[0x6] = p60;
		dft_offsets_lo[0x7] = p07;	dft_offsets_hi[0x7] = p70;
		dft_offsets_lo[0x8] = p08;	dft_offsets_hi[0x8] = p80;
		dft_offsets_lo[0x9] = p09;	dft_offsets_hi[0x9] = p90;
		dft_offsets_lo[0xa] = p0a;	dft_offsets_hi[0xa] = pa0;
		dft_offsets_lo[0xb] = p0b;	dft_offsets_hi[0xb] = pb0;
		dft_offsets_lo[0xc] = p0c;	dft_offsets_hi[0xc] = pc0;
		dft_offsets_lo[0xd] = p0d;	dft_offsets_hi[0xd] = pd0;
		dft_offsets_lo[0xe] = p0e;	dft_offsets_hi[0xe] = pe0;
		dft_offsets_lo[0xf] = p0f;	dft_offsets_hi[0xf] = pf0;
	#endif
	#if !USE_SCALAR_DFT_MACRO || !defined(USE_SSE2)	// SIMD build uses these for radix-256 DFTs; Scalar-double build uses these for carry-macro loops
		po_br[0x0] =   0; po_br[0x1] = p08; po_br[0x2] = p04; po_br[0x3] = p0c;
		po_br[0x4] = p02; po_br[0x5] = p0a; po_br[0x6] = p06; po_br[0x7] = p0e;
		po_br[0x8] = p01; po_br[0x9] = p09; po_br[0xa] = p05; po_br[0xb] = p0d;
		po_br[0xc] = p03; po_br[0xd] = p0b; po_br[0xe] = p07; po_br[0xf] = p0f;
	#endif

		if(_cy_r[0])	/* If it's a new exponent of a range test, need to deallocate these. */
		{
			free((void *)_i     ); _i      = 0x0;
			for(i = 0; i < RADIX; i++) {
				free((void *)_bjmodn[i]); _bjmodn[i] = 0x0;
				free((void *)  _cy_r[i]);   _cy_r[i] = 0x0;
				free((void *)  _cy_i[i]);   _cy_i[i] = 0x0;
			}
			free((void *)_jstart ); _jstart  = 0x0;
			free((void *)_jhi    ); _jhi     = 0x0;
			free((void *)_col   ); _col    = 0x0;
			free((void *)_co2   ); _co2    = 0x0;
			free((void *)_co3   ); _co3    = 0x0;
			free((void *)_bjmodnini); _bjmodnini = 0x0;
		}

		ptr_prod = (uint32)0;	/* Store bitmask for allocatable-array ptrs here, check vs 0 after all alloc calls finish */
		j = CY_THREADS*sizeof(int);
		_i       	= (int *)malloc(j);	ptr_prod += (uint32)(_i== 0x0);
		for(i = 0; i < RADIX; i++) {
			_bjmodn[i]	= (int *)malloc(j);	ptr_prod += (uint32)(_bjmodn[i]== 0x0);
		}
		_jstart  	= (int *)malloc(j);	ptr_prod += (uint32)(_jstart  == 0x0);
		_jhi     	= (int *)malloc(j);	ptr_prod += (uint32)(_jhi     == 0x0);
		_col     	= (int *)malloc(j);	ptr_prod += (uint32)(_col     == 0x0);
		_co2     	= (int *)malloc(j);	ptr_prod += (uint32)(_co2     == 0x0);
		_co3     	= (int *)malloc(j);	ptr_prod += (uint32)(_co3     == 0x0);

		j = CY_THREADS*sizeof(double);
		for(i = 0; i < RADIX; i++) {
			_cy_r[i]	= (double *)malloc(j);	ptr_prod += (uint32)(_cy_r[i]== 0x0);
			_cy_i[i]	= (double *)malloc(j);	ptr_prod += (uint32)(_cy_i[i]== 0x0);
		}

		ASSERT(HERE, ptr_prod == 0, "FATAL: unable to allocate one or more auxiliary arrays!");

		if(MODULUS_TYPE == MODULUS_TYPE_MERSENNE)
		{
			/* Create (THREADS + 1) copies of _bjmodnini and use the extra (uppermost) one to store the "master" increment,
			i.e. the one that n2/radix-separated FFT outputs need:
			*/
			_bjmodnini = (int *)malloc((CY_THREADS + 1)*sizeof(int));	if(!_bjmodnini){ sprintf(cbuf,"FATAL: unable to allocate array _bjmodnini.\n"); fprintf(stderr,"%s", cbuf);	ASSERT(HERE, 0,cbuf); }
			_bjmodnini[0] = 0;
			_bjmodnini[1] = 0;
			for(j=0; j < NDIVR/CY_THREADS; j++)
			{
				_bjmodnini[1] -= sw; _bjmodnini[1] = _bjmodnini[1] + ( (-(int)((uint32)_bjmodnini[1] >> 31)) & n);
			}
			if(CY_THREADS > 1)
			{
				for(ithread = 2; ithread <= CY_THREADS; ithread++)
				{
					_bjmodnini[ithread] = _bjmodnini[ithread-1] + _bjmodnini[1] - n; _bjmodnini[ithread] = _bjmodnini[ithread] + ( (-(int)((uint32)_bjmodnini[ithread] >> 31)) & n);
				}
			}
			/* Check upper element against scalar value, as precomputed in single-thread mode: */
			bjmodnini=0;
			for(j=0; j < NDIVR; j++)
			{
				bjmodnini -= sw; bjmodnini = bjmodnini + ( (-(int)((uint32)bjmodnini >> 31)) & n);
			}
			ASSERT(HERE, _bjmodnini[CY_THREADS] == bjmodnini,"_bjmodnini[CY_THREADS] != bjmodnini");
			if(CY_THREADS > 1)
			{
				for(ithread = 1; ithread < CY_THREADS; ithread++)
				{
					_i[ithread] = ((uint32)(sw - _bjmodnini[ithread]) >> 31);
				}
			}

			// Include 0-thread here ... bjmodn terms all 0 for that, but need jhi computed for all threads:
			j = _bjmodnini[CY_THREADS];
			for(ithread = 0; ithread < CY_THREADS; ithread++)
			{
				_bjmodn[0][ithread] = _bjmodnini[ithread];
				for(i = 1; i < RADIX; i++) {
					MOD_ADD32(_bjmodn[i-1][ithread], j, n, _bjmodn[i][ithread]);
				}
			}
		}

	#ifdef USE_PTHREAD
		/* Populate the elements of the thread-specific data structs which don't change after init: */
		for(ithread = 0; ithread < CY_THREADS; ithread++)
		{
		  // For pow2 FFT lengths, bjmodn only needed for mers-mod:
		  if(MODULUS_TYPE == MODULUS_TYPE_MERSENNE)
		  {
			tdat[ithread].bjmodnini = _bjmodnini[CY_THREADS];
			tdat[ithread].bjmodn0 = _bjmodnini[ithread];
		  }
		#ifdef USE_SSE2
			tdat[ithread].r00      = __r0 + ithread*cslots_in_local_store;
			tdat[ithread].half_arr = (long)tdat[ithread].r00 + ((long)half_arr - (long)r00);
		#else	// In scalar mode use these 2 ptrs to pass the base & baseinv arrays:
			tdat[ithread].r00      = (double *)base;
			tdat[ithread].half_arr = (double *)baseinv;
		#endif	// USE_SSE2
		}
	#endif

		first_entry=FALSE;
	}	/* endif(first_entry) */

/*...The radix-256 final DIT pass is here.	*/

	/* init carries	*/
	for(ithread = 0; ithread < CY_THREADS; ithread++)
	{
		for(i = 0; i < RADIX; i++) {
			_cy_r[i][ithread] = 0;
			_cy_i[i][ithread] = 0;
		}
	}
	/* If an LL test, init the subtract-2: */
	if(MODULUS_TYPE == MODULUS_TYPE_MERSENNE && TEST_TYPE == TEST_TYPE_PRIMALITY)
	{
		_cy_r[0][0] = -2;
	}

	*fracmax=0;	/* init max. fractional error	*/
	full_pass = 1;	/* set = 1 for normal carry pass, = 0 for wrapper pass	*/
	scale = n2inv;	/* init inverse-weight scale factor  (set = 2/n for normal carry pass, = 1 for wrapper pass)	*/

for(outer=0; outer <= 1; outer++)
{
	if(MODULUS_TYPE == MODULUS_TYPE_MERSENNE)
	{
		/*
		Moved this inside the outer-loop, so on cleanup pass can use it to reset _col,_co2,_co3 starting values,
		then simply overwrite it with 1 prior to starting the k-loop.
		*/
		_i[0] = 1;		/* Pointer to the BASE and BASEINV arrays. lowest-order digit is always a bigword (_i[0] = 1).	*/

		khi = n_div_nwt/CY_THREADS;
		for(ithread = 0; ithread < CY_THREADS; ithread++)
		{
			_jstart[ithread] = ithread*NDIVR/CY_THREADS;
			if(!full_pass)
				_jhi[ithread] = _jstart[ithread] + jhi_wrap_mers;	/* Cleanup loop assumes carryins propagate at most 4 words up. */
			else
				_jhi[ithread] = _jstart[ithread] + nwt-1;

			_col[ithread] = ithread*(khi*RADIX);			/* col gets incremented by RADIX_VEC[0] on every pass through the k-loop */
			_co2[ithread] = (n>>nwt_bits)-1+RADIX - _col[ithread];	/* co2 gets decremented by RADIX_VEC[0] on every pass through the k-loop */
			_co3[ithread] = _co2[ithread]-RADIX;			/* At the start of each new j-loop, co3=co2-RADIX_VEC[0]	*/
		}
	}
	else
	{
		khi = 1;
		for(ithread = 0; ithread < CY_THREADS; ithread++)
		{
			_jstart[ithread] = ithread*NDIVR/CY_THREADS;
			/*
			For right-angle transform need *complex* elements for wraparound, so jhi needs to be twice as large
			*/
			if(!full_pass)
				_jhi[ithread] = _jstart[ithread] + jhi_wrap_ferm;	/* Cleanup loop assumes carryins propagate at most 4 words up. */
			else
				_jhi[ithread] = _jstart[ithread] + n_div_nwt/CY_THREADS;
		}
	}

#ifdef USE_SSE2

	tmp = max_err;	VEC_DBL_INIT(tmp, 0.0);
	tm2 = tmp + cslots_in_local_store;
	for(ithread = 1; ithread < CY_THREADS; ++ithread) {
		memcpy(tm2, tmp, sz_vd);
		tmp = tm2;		tm2 += cslots_in_local_store;
	}

#endif	// USE_PTHREAD

	/* Move this cleanup-pass-specific khi setting here, since need regular-pass khi value for above inits: */
	if(!full_pass)
	{
		khi = 1;
	}

#ifdef USE_PTHREAD
	/* Populate the thread-specific data structs - use the invariant terms as memchecks: */
	for(ithread = 0; ithread < CY_THREADS; ithread++)
	{
		tdat[ithread].iter = iter;
	// int data:
		ASSERT(HERE, tdat[ithread].tid == ithread, "thread-local memcheck fail!");
		ASSERT(HERE, tdat[ithread].ndivr == NDIVR, "thread-local memcheck fail!");

		tdat[ithread].khi    = khi;
		tdat[ithread].i      = _i[ithread];	/* Pointer to the BASE and BASEINV arrays.	*/
		tdat[ithread].jstart = _jstart[ithread];
		tdat[ithread].jhi    = _jhi[ithread];

		tdat[ithread].col = _col[ithread];
		tdat[ithread].co2 = _co2[ithread];
		tdat[ithread].co3 = _co3[ithread];
		ASSERT(HERE, tdat[ithread].sw  == sw, "thread-local memcheck fail!");
		ASSERT(HERE, tdat[ithread].nwt == nwt, "thread-local memcheck fail!");

	// double data:
		tdat[ithread].maxerr = 0.0;
		tdat[ithread].scale = scale;

	// pointer data:
		ASSERT(HERE, tdat[ithread].arrdat == a, "thread-local memcheck fail!");			/* Main data array */
		ASSERT(HERE, tdat[ithread].wt0 == wt0, "thread-local memcheck fail!");
		ASSERT(HERE, tdat[ithread].wt1 == wt1, "thread-local memcheck fail!");
		ASSERT(HERE, tdat[ithread].si  == si, "thread-local memcheck fail!");
		ASSERT(HERE, tdat[ithread].rn0 == rn0, "thread-local memcheck fail!");
		ASSERT(HERE, tdat[ithread].rn1 == rn1, "thread-local memcheck fail!");
	#ifdef USE_SSE2
		ASSERT(HERE, tdat[ithread].r00 == __r0 + ithread*cslots_in_local_store, "thread-local memcheck fail!");
		tmp = tdat[ithread].r00;
		ASSERT(HERE, ((tmp + 0x400)->d0 == 2.0 && (tmp + 0x400)->d1 == 2.0), "thread-local memcheck failed!");
		tmp = tdat[ithread].half_arr;
	  #ifdef USE_AVX512	// In AVX-512 mode, use VRNDSCALEPD for rounding and hijack this vector-data slot for the 4 base/baseinv-consts
		ASSERT(HERE, ((tmp-1)->d0 == base[0] && (tmp-1)->d1 == baseinv[1] && (tmp-1)->d2 == wts_mult[1] && (tmp-1)->d3 == inv_mult[0]), "thread-local memcheck failed!");
	  #else
		ASSERT(HERE, ((tmp-1)->d0 == crnd && (tmp-1)->d1 == crnd), "thread-local memcheck failed!");
	  #endif
	#endif

		if(MODULUS_TYPE == MODULUS_TYPE_MERSENNE)
		{
		#ifdef USE_AVX512
			/* No-Op */
		#elif defined(USE_AVX)
			// Grab some elt of base-data [offset by, say, +32] and mpy by its inverse [+16 further]
			dtmp = (tmp+40)->d0 * (tmp+56)->d0;	ASSERT(HERE, fabs(dtmp - 1.0) < EPS, "thread-local memcheck failed!");
			dtmp = (tmp+40)->d1 * (tmp+56)->d1;	ASSERT(HERE, fabs(dtmp - 1.0) < EPS, "thread-local memcheck failed!");
		#elif defined(USE_SSE2)
			dtmp = (tmp+10)->d0 * (tmp+14)->d0;	ASSERT(HERE, fabs(dtmp - 1.0) < EPS, "thread-local memcheck failed!");
			dtmp = (tmp+10)->d1 * (tmp+14)->d1;	ASSERT(HERE, fabs(dtmp - 1.0) < EPS, "thread-local memcheck failed!");
		#endif
			/* init carries	*/
			for(i = 0; i < RADIX; i++) {
				tdat[ithread].cy_r[i] = _cy_r[i][ithread];
			}
		}
		else	/* Fermat-mod uses "double helix" carry scheme - 2 separate sets of real/imaginary carries for right-angle transform, plus "twisted" wraparound step. */
		{
		#ifdef USE_AVX512
			/* No-Op */
		#elif defined(USE_SSE2)
			// This is slightly different for power-of-2 DFTs: Here, scale is in the +2 slot, base & baseinv remain fixed in 0,+1 slots:
			dtmp = tmp->d0 * (tmp+1)->d0;	ASSERT(HERE, fabs(dtmp - 1.0) < EPS, "thread-local memcheck failed!");
			dtmp = tmp->d1 * (tmp+1)->d1;	ASSERT(HERE, fabs(dtmp - 1.0) < EPS, "thread-local memcheck failed!");
		#endif
			// scale gets set immediately prior to calling carry macro, hence no use checking it here.
			/* init carries	*/
			for(i = 0; i < RADIX; i++) {
				tdat[ithread].cy_r[i] = _cy_r[i][ithread];
				tdat[ithread].cy_i[i] = _cy_i[i][ithread];
			}
		}
	}
#endif

#ifdef USE_PTHREAD

	// If also using main thread to do work units, that task-dispatch occurs after all the threadpool-task launches:
	for(ithread = 0; ithread < pool_work_units; ithread++)
	{
		task_control.data = (void*)(&tdat[ithread]);
		threadpool_add_task(tpool, &task_control, task_is_blocking);

#else

	for(ithread = 0; ithread < CY_THREADS; ithread++)
	{
		if(full_pass) maxerr = 0.0;	// Wraparound-carry pass has maxerr ~0, keep that from overwriting full-pass value
	#ifdef USE_SSE2
	//	VEC_DBL_INIT(max_err, 0.0);	*** must do this in conjunction with thread-local-data-copy
	#endif

		i      = _i[ithread];	/* Pointer to the BASE and BASEINV arrays.	*/
		jstart = _jstart[ithread];
		jhi    = _jhi[ithread];

		if(MODULUS_TYPE == MODULUS_TYPE_MERSENNE)
		{
			col = _col[ithread];
			co2 = _co2[ithread];
			co3 = _co3[ithread];

			for(l = 0; l < RADIX; l++) {
				bjmodn[l] = _bjmodn[l][ithread];
			}
			/* init carries	*/
		#ifdef USE_AVX512
			tmp = cy_r;
			for(l = 0; l < RADIX; l += 8, ++tmp) {
				tmp->d0 = _cy_r[l  ][ithread];
				tmp->d1 = _cy_r[l+1][ithread];
				tmp->d2 = _cy_r[l+2][ithread];
				tmp->d3 = _cy_r[l+3][ithread];
				tmp->d4 = _cy_r[l+4][ithread];
				tmp->d5 = _cy_r[l+5][ithread];
				tmp->d6 = _cy_r[l+6][ithread];
				tmp->d7 = _cy_r[l+7][ithread];
			}
		#elif defined(USE_AVX)	// AVX and AVX2 both use 256-bit registers
			tmp = cy_r;
			for(l = 0; l < RADIX; l += 4, ++tmp) {
				tmp->d0 = _cy_r[l  ][ithread];
				tmp->d1 = _cy_r[l+1][ithread];
				tmp->d2 = _cy_r[l+2][ithread];
				tmp->d3 = _cy_r[l+3][ithread];
			}
		#elif defined(USE_SSE2)
			tmp = cy_r;
			for(l = 0; l < RADIX; l += 2, ++tmp) {
				tmp->d0 = _cy_r[l  ][ithread];
				tmp->d1 = _cy_r[l+1][ithread];
			}
		#else
			for(l = 0; l < RADIX; l++) {
				cy_r[l] = _cy_r[l][ithread];
			}
		#endif
		}
		else	/* Fermat-mod uses "double helix" carry scheme - 2 separate sets of real/imaginary carries for right-angle transform, plus "twisted" wraparound step. */
		{
			/* init carries	*/
		#ifdef USE_AVX512
			tmp = cy_r;	tm2 = cy_i;
			for(l = 0; l < RADIX; l += 8, ++tmp, ++tm2) {
				tmp->d0 = _cy_r[l  ][ithread];		tm2->d0 = _cy_i[l  ][ithread];
				tmp->d1 = _cy_r[l+1][ithread];		tm2->d1 = _cy_i[l+1][ithread];
				tmp->d2 = _cy_r[l+2][ithread];		tm2->d2 = _cy_i[l+2][ithread];
				tmp->d3 = _cy_r[l+3][ithread];		tm2->d3 = _cy_i[l+3][ithread];
				tmp->d4 = _cy_r[l+4][ithread];		tm2->d4 = _cy_i[l+4][ithread];
				tmp->d5 = _cy_r[l+5][ithread];		tm2->d5 = _cy_i[l+5][ithread];
				tmp->d6 = _cy_r[l+6][ithread];		tm2->d6 = _cy_i[l+6][ithread];
				tmp->d7 = _cy_r[l+7][ithread];		tm2->d7 = _cy_i[l+7][ithread];
			}
		#elif defined(USE_AVX)	// AVX and AVX2 both use 256-bit registers
			tmp = cy_r;	tm2 = cy_i;
			for(l = 0; l < RADIX; l += 4, ++tmp, ++tm2) {
				tmp->d0 = _cy_r[l  ][ithread];		tm2->d0 = _cy_i[l  ][ithread];
				tmp->d1 = _cy_r[l+1][ithread];		tm2->d1 = _cy_i[l+1][ithread];
				tmp->d2 = _cy_r[l+2][ithread];		tm2->d2 = _cy_i[l+2][ithread];
				tmp->d3 = _cy_r[l+3][ithread];		tm2->d3 = _cy_i[l+3][ithread];
			}
		#elif defined(USE_SSE2)
			// Carry pattern for Fermat-mod in SSE2 mode is kinda funky:
			tmp = cy_r;
			for(l = 0; l < RADIX; l++, ++tmp) {
				// This relies on the cy_R,i sections of the SIMD data being contiguous, i.e.
				// step-thru the cy_r data via the tmp-pointer takes us seamlessly into the cy_i:
				tmp->d0 = _cy_r[l][ithread];		tmp->d1 = _cy_i[l][ithread];
			}
		#else
			for(l = 0; l < RADIX; l++) {
				cy_r[l] = _cy_r[l][ithread];		cy_i[l] = _cy_i[l][ithread];
			}
		#endif
		}

		/********************************************************************************/
		/* This main loop is same for un-and-multithreaded, so stick into a header file */
		/* (can't use a macro because of the #if-enclosed stuff).                       */
		/********************************************************************************/
		#include "radix256_main_carry_loop.h"

		/* At end of each thread-processed work chunk, dump the
		carryouts into their non-thread-private array slots:
		*/
		if(MODULUS_TYPE == MODULUS_TYPE_MERSENNE)
		{
		#ifdef USE_AVX512
			tmp = cy_r;
			for(l = 0; l < RADIX; l += 8, ++tmp) {
				_cy_r[l  ][ithread] = tmp->d0;
				_cy_r[l+1][ithread] = tmp->d1;
				_cy_r[l+2][ithread] = tmp->d2;
				_cy_r[l+3][ithread] = tmp->d3;
				_cy_r[l+4][ithread] = tmp->d4;
				_cy_r[l+5][ithread] = tmp->d5;
				_cy_r[l+6][ithread] = tmp->d6;
				_cy_r[l+7][ithread] = tmp->d7;
			}
			if(full_pass) {
				t0 = MAX(max_err->d0,max_err->d1);
				t1 = MAX(max_err->d2,max_err->d3);
				t2 = MAX(max_err->d4,max_err->d5);
				t3 = MAX(max_err->d6,max_err->d7);
				maxerr = MAX( MAX(t0,t1), MAX(t2,t3) );
			}
		#elif defined(USE_AVX)	// AVX and AVX2 both use 256-bit registers
			tmp = cy_r;
			for(l = 0; l < RADIX; l += 4, ++tmp) {
				_cy_r[l  ][ithread] = tmp->d0;
				_cy_r[l+1][ithread] = tmp->d1;
				_cy_r[l+2][ithread] = tmp->d2;
				_cy_r[l+3][ithread] = tmp->d3;
			}
			if(full_pass) maxerr = MAX( MAX(max_err->d0,max_err->d1) , MAX(max_err->d2,max_err->d3) );
		#elif defined(USE_SSE2)
			tmp = cy_r;
			for(l = 0; l < RADIX; l += 2, ++tmp) {
				_cy_r[l  ][ithread] = tmp->d0;
				_cy_r[l+1][ithread] = tmp->d1;
			}
			if(full_pass) maxerr = MAX(max_err->d0,max_err->d1);
		#else
			for(l = 0; l < RADIX; l++) {
				_cy_r[l][ithread] = cy_r[l];
			}
		#endif
		}
		else
		{
		#ifdef USE_AVX512
			tmp = cy_r;	tm2 = cy_i;
			for(l = 0; l < RADIX; l += 8, ++tmp, ++tm2) {
				_cy_r[l  ][ithread] = tmp->d0;		_cy_i[l  ][ithread] = tm2->d0;
				_cy_r[l+1][ithread] = tmp->d1;		_cy_i[l+1][ithread] = tm2->d1;
				_cy_r[l+2][ithread] = tmp->d2;		_cy_i[l+2][ithread] = tm2->d2;
				_cy_r[l+3][ithread] = tmp->d3;		_cy_i[l+3][ithread] = tm2->d3;
				_cy_r[l+4][ithread] = tmp->d4;		_cy_i[l+4][ithread] = tm2->d4;
				_cy_r[l+5][ithread] = tmp->d5;		_cy_i[l+5][ithread] = tm2->d5;
				_cy_r[l+6][ithread] = tmp->d6;		_cy_i[l+6][ithread] = tm2->d6;
				_cy_r[l+7][ithread] = tmp->d7;		_cy_i[l+7][ithread] = tm2->d7;
			}
			if(full_pass) {
				t0 = MAX(max_err->d0,max_err->d1);
				t1 = MAX(max_err->d2,max_err->d3);
				t2 = MAX(max_err->d4,max_err->d5);
				t3 = MAX(max_err->d6,max_err->d7);
				maxerr = MAX( MAX(t0,t1), MAX(t2,t3) );
			}
		#elif defined(USE_AVX)	// AVX and AVX2 both use 256-bit registers
			tmp = cy_r;	tm2 = cy_i;
			for(l = 0; l < RADIX; l += 4, ++tmp, ++tm2) {
				_cy_r[l  ][ithread] = tmp->d0;		_cy_i[l  ][ithread] = tm2->d0;
				_cy_r[l+1][ithread] = tmp->d1;		_cy_i[l+1][ithread] = tm2->d1;
				_cy_r[l+2][ithread] = tmp->d2;		_cy_i[l+2][ithread] = tm2->d2;
				_cy_r[l+3][ithread] = tmp->d3;		_cy_i[l+3][ithread] = tm2->d3;
			}
			if(full_pass) maxerr = MAX( MAX(max_err->d0,max_err->d1) , MAX(max_err->d2,max_err->d3) );
		#elif defined(USE_SSE2)
			// Carry pattern for Fermat-mod in SSE2 mode is kinda funky:
			tmp = cy_r;
			for(l = 0; l < RADIX; l++, ++tmp) {
				// This relies on the cy_R,i sections of the SIMD data being contiguous, i.e.
				// step-thru the cy_r data via the tmp-pointer takes us seamlessly into the cy_i:
				_cy_r[l][ithread] = tmp->d0;		_cy_i[l][ithread] = tmp->d1;
			}
			if(full_pass) maxerr = MAX(max_err->d0,max_err->d1);
		#else
			for(l = 0; l < RADIX; l++) {
				_cy_r[l][ithread] = cy_r[l];		_cy_i[l][ithread] = cy_i[l];
			}
		#endif
		}

  #endif	// #ifdef USE_PTHREAD

	}	/******* END OF PARALLEL FOR-LOOP ********/

#ifdef USE_PTHREAD	// End of threadpool-based dispatch: Add a small wait-loop to ensure all threads complete

  #if 0//def OS_TYPE_MACOSX

	/*** Main execution thread executes remaining chunks in serial fashion (but in || with the pool threads): ***/
	for(j = 0; j < main_work_units; ++j)
	{
	//	printf("adding main task %d\n",j + pool_work_units);
		ASSERT(HERE, 0x0 == cy256_process_chunk( (void*)(&tdat[j + pool_work_units]) ), "Main-thread task failure!");
	}

  #endif

	struct timespec ns_time;	// We want a sleep interval of 0.1 mSec here...
	ns_time.tv_sec  =      0;	// (time_t)seconds - Don't use this because under OS X it's of type __darwin_time_t, which is long rather than double as under most linux distros
	ns_time.tv_nsec = 100000;	// (long)nanoseconds - Get our desired 0.1 mSec as 10^5 nSec here

	while(tpool && tpool->free_tasks_queue.num_tasks != pool_work_units) {
		ASSERT(HERE, 0 == nanosleep(&ns_time, 0x0), "nanosleep fail!");
	}
//	printf("%s end  ; #tasks = %d, #free_tasks = %d\n",func, tpool->tasks_queue.num_tasks, tpool->free_tasks_queue.num_tasks);

	/* Copy the thread-specific output carry data back to shared memory: */
	for(ithread = 0; ithread < CY_THREADS; ithread++)
	{
		if(maxerr < tdat[ithread].maxerr) {
			maxerr = tdat[ithread].maxerr;
		}

		if(MODULUS_TYPE == MODULUS_TYPE_MERSENNE)
		{
			for(l = 0; l < RADIX; l++) {
				_cy_r[l][ithread] = tdat[ithread].cy_r[l];
			}
		}
		else
		{
			for(l = 0; l < RADIX; l++) {
				_cy_r[l][ithread] = tdat[ithread].cy_r[l];
				_cy_i[l][ithread] = tdat[ithread].cy_i[l];
			}
		}
	}
#endif

	if(full_pass) {
	//	printf("Iter = %d, maxerr = %20.15f\n",iter,maxerr);
	} else {
		break;
	}

	/*   Wraparound carry cleanup loop is here:

	The cleanup carries from the end of each length-N/RADIX set of contiguous data into the begining of the next
	can all be neatly processed as follows:

	(1) Invert the forward DIF FFT of the first block of RADIX complex elements in A and unweight;
	(2) Propagate cleanup carries among the real and imaginary parts of the RADIX outputs of (1);
	(3) Reweight and perform a forward DIF FFT on the result of (2);
	(4) If any of the exit carries from (2) are nonzero, advance to the next RADIX elements and repeat (1-4).
	*/
	if(MODULUS_TYPE == MODULUS_TYPE_MERSENNE)
	{
		for(l = 0; l < RADIX; l++) {
			t[l].re = _cy_r[l][CY_THREADS - 1];
		}
		for(ithread = CY_THREADS - 1; ithread > 0; ithread--)
		{
			for(l = 0; l < RADIX; l++) {
				_cy_r[l][ithread] = _cy_r[l][ithread-1];
			}
		}
		_cy_r[0][0] =+t[RADIX-1].re;	/* ...The wraparound carry is here: */
		for(l = 1; l < RADIX; l++) {
			_cy_r[l][0] = t[l-1].re;
		}
	}
	else
	{
		j = CY_THREADS - 1;
		for(l = 0; l < RADIX; l++) {
			t[l].re = _cy_r[l][j];		t[l].im = _cy_i[l][j];
		}
		for(ithread = CY_THREADS - 1; ithread > 0; ithread--)
		{
			for(l = 0; l < RADIX; l++) {
				_cy_r[l][ithread] = _cy_r[l][ithread-1];	_cy_i[l][ithread] = _cy_i[l][ithread-1];
			}
		}
		_cy_r[0][0] =-t[RADIX-1].im;	_cy_i[0][0] =+t[RADIX-1].re;	/* ...The 2 Mo"bius carries are here: */
		for(l = 1; l < RADIX; l++) {
			_cy_r[l][0] = t[l-1].re;	_cy_i[l][0] = t[l-1].im;
		}
	}

	full_pass = 0;
	scale = 1;

	/*
	For right-angle transform need *complex* elements for wraparound, so jhi needs to be twice as large
	*/
	if((MODULUS_TYPE == MODULUS_TYPE_GENFFTMUL) || (TRANSFORM_TYPE == RIGHT_ANGLE))
		j_jhi = jhi_wrap_ferm;
	else
		j_jhi = jhi_wrap_mers;

	for(ithread = 0; ithread < CY_THREADS; ithread++)
	{
		for(j = ithread*pini; j <= ithread*pini + j_jhi; j++)
		{
			// Generate padded version of j, since prepadding pini is thread-count unsafe:
			j1 = j + ( (j >> DAT_BITS) << PAD_BITS );
			for(ntmp = 0; ntmp < RADIX>>2; ntmp++) {
				jt = j1 + poff[ntmp];
				a[jt    ] *= radix_inv;
				a[jt+p01] *= radix_inv;
				a[jt+p02] *= radix_inv;
				a[jt+p03] *= radix_inv;
			}
		}
	}
}	/* endfor(outer) */

	dtmp = 0;
	for(ithread = 0; ithread < CY_THREADS; ithread++)
	{
		for(l = 0; l < RADIX; l++) {
			dtmp += fabs(_cy_r[l][ithread]) + fabs(_cy_i[l][ithread]);
		}
		*fracmax = maxerr;
	}
	if(dtmp != 0.0)
	{
		sprintf(cbuf,"FATAL: iter = %10d; nonzero exit carry in %s - input wordsize may be too small.\n",iter,func);
		if(INTERACT)fprintf(stderr,"%s",cbuf);
		fp = mlucas_fopen(   OFILE,"a");
		fq = mlucas_fopen(STATFILE,"a");
		fprintf(fp,"%s",cbuf);
		fprintf(fq,"%s",cbuf);
		fclose(fp);	fp = 0x0;
		fclose(fq);	fq = 0x0;
		err=ERR_CARRY;
		return(err);
	}
	return(0);
}

/***************/

void radix256_dif_pass1(double a[], int n)
{
/*
!...Acronym: DIF = Decimation In Frequency
!
!...Subroutine to perform an initial radix-256 complex DIF FFT pass on the data in the length-N real vector A.
!
!   The data are stored in a 1-D zero-offset array, with 2^PAD_BITS 8-byte padding elements inserted
!   between every block of 2^DAT_BITS contiguous data. The array padding is to prevent data being accessed
!   in strides that are large powers of two and thus to minimize cache thrashing
!   (in cache-based microprocessor architectures) or bank conflicts (in supercomputers.)
!
!   See the documentation in radix64_dif_pass for further details on storage, indexing and DFT decomposition.
*/
	int j,j1,j2,jt,jp;
	static int NDIVR,p01,p02,p03,p04,p05,p06,p07,p08,p09,p0a,p0b,p0c,p0d,p0e,p0f,p10,p20,p30,p40,p50,p60,p70,p80,p90,pa0,pb0,pc0,pd0,pe0,pf0	, first_entry=TRUE;
	static int i_offsets_lo[16], i_offsets_hi[16];

	if(!first_entry && (n >> 8) != NDIVR)	/* New runlength?	*/
	{
		first_entry=TRUE;
	}

/*...initialize things upon first entry	*/

	if(first_entry)
	{
		first_entry=FALSE;
		NDIVR = n >> 8;

		p01 = NDIVR;
		p02 = p01 + p01;
		p03 = p02 + p01;
		p04 = p03 + p01;
		p05 = p04 + p01;
		p06 = p05 + p01;
		p07 = p06 + p01;
		p08 = p07 + p01;
		p09 = p08 + p01;
		p0a = p09 + p01;
		p0b = p0a + p01;
		p0c = p0b + p01;
		p0d = p0c + p01;
		p0e = p0d + p01;
		p0f = p0e + p01;
		p10 = p08 + p08;
		p20 = p10 + p10;
		p30 = p20 + p10;
		p40 = p30 + p10;
		p50 = p40 + p10;
		p60 = p50 + p10;
		p70 = p60 + p10;
		p80 = p70 + p10;
		p90 = p80 + p10;
		pa0 = p90 + p10;
		pb0 = pa0 + p10;
		pc0 = pb0 + p10;
		pd0 = pc0 + p10;
		pe0 = pd0 + p10;
		pf0 = pe0 + p10;

		p01 = p01 + ( (p01 >> DAT_BITS) << PAD_BITS );
		p02 = p02 + ( (p02 >> DAT_BITS) << PAD_BITS );
		p03 = p03 + ( (p03 >> DAT_BITS) << PAD_BITS );
		p04 = p04 + ( (p04 >> DAT_BITS) << PAD_BITS );
		p05 = p05 + ( (p05 >> DAT_BITS) << PAD_BITS );
		p06 = p06 + ( (p06 >> DAT_BITS) << PAD_BITS );
		p07 = p07 + ( (p07 >> DAT_BITS) << PAD_BITS );
		p08 = p08 + ( (p08 >> DAT_BITS) << PAD_BITS );
		p09 = p09 + ( (p09 >> DAT_BITS) << PAD_BITS );
		p0a = p0a + ( (p0a >> DAT_BITS) << PAD_BITS );
		p0b = p0b + ( (p0b >> DAT_BITS) << PAD_BITS );
		p0c = p0c + ( (p0c >> DAT_BITS) << PAD_BITS );
		p0d = p0d + ( (p0d >> DAT_BITS) << PAD_BITS );
		p0e = p0e + ( (p0e >> DAT_BITS) << PAD_BITS );
		p0f = p0f + ( (p0f >> DAT_BITS) << PAD_BITS );
		p10 = p10 + ( (p10 >> DAT_BITS) << PAD_BITS );
		p20 = p20 + ( (p20 >> DAT_BITS) << PAD_BITS );
		p30 = p30 + ( (p30 >> DAT_BITS) << PAD_BITS );
		p40 = p40 + ( (p40 >> DAT_BITS) << PAD_BITS );
		p50 = p50 + ( (p50 >> DAT_BITS) << PAD_BITS );
		p60 = p60 + ( (p60 >> DAT_BITS) << PAD_BITS );
		p70 = p70 + ( (p70 >> DAT_BITS) << PAD_BITS );
		p80 = p80 + ( (p80 >> DAT_BITS) << PAD_BITS );
		p90 = p90 + ( (p90 >> DAT_BITS) << PAD_BITS );
		pa0 = pa0 + ( (pa0 >> DAT_BITS) << PAD_BITS );
		pb0 = pb0 + ( (pb0 >> DAT_BITS) << PAD_BITS );
		pc0 = pc0 + ( (pc0 >> DAT_BITS) << PAD_BITS );
		pd0 = pd0 + ( (pd0 >> DAT_BITS) << PAD_BITS );
		pe0 = pe0 + ( (pe0 >> DAT_BITS) << PAD_BITS );
		pf0 = pf0 + ( (pf0 >> DAT_BITS) << PAD_BITS );

		// Set array offsets for in/outputs - low parts in fisrt 16 slots, high parts in next 16:
		i_offsets_lo[0x0] = 0  ;	i_offsets_hi[0x0] = 0  ;
		i_offsets_lo[0x1] = p01;	i_offsets_hi[0x1] = p10;
		i_offsets_lo[0x2] = p02;	i_offsets_hi[0x2] = p20;
		i_offsets_lo[0x3] = p03;	i_offsets_hi[0x3] = p30;
		i_offsets_lo[0x4] = p04;	i_offsets_hi[0x4] = p40;
		i_offsets_lo[0x5] = p05;	i_offsets_hi[0x5] = p50;
		i_offsets_lo[0x6] = p06;	i_offsets_hi[0x6] = p60;
		i_offsets_lo[0x7] = p07;	i_offsets_hi[0x7] = p70;
		i_offsets_lo[0x8] = p08;	i_offsets_hi[0x8] = p80;
		i_offsets_lo[0x9] = p09;	i_offsets_hi[0x9] = p90;
		i_offsets_lo[0xa] = p0a;	i_offsets_hi[0xa] = pa0;
		i_offsets_lo[0xb] = p0b;	i_offsets_hi[0xb] = pb0;
		i_offsets_lo[0xc] = p0c;	i_offsets_hi[0xc] = pc0;
		i_offsets_lo[0xd] = p0d;	i_offsets_hi[0xd] = pd0;
		i_offsets_lo[0xe] = p0e;	i_offsets_hi[0xe] = pe0;
		i_offsets_lo[0xf] = p0f;	i_offsets_hi[0xf] = pf0;
	}

/*...The radix-256 pass is here.	*/

	for(j = 0; j < NDIVR; j += 2)
	{
	#ifdef USE_AVX512
		j1 = (j & mask03) + br16[j&15];
	#elif defined(USE_AVX)
		j1 = (j & mask02) + br8[j&7];
	#elif defined(USE_SSE2)
		j1 = (j & mask01) + br4[j&3];
	#else
		j1 = j;
	#endif
		j1 =j1 + ( (j1>> DAT_BITS) << PAD_BITS );	/* padded-array fetch index is here */
		j2 = j1+RE_IM_STRIDE;
		RADIX_256_DIF(
			(a+j1),RE_IM_STRIDE,i_offsets_lo,i_offsets_hi,
			(a+j1),RE_IM_STRIDE,i_offsets_lo,(uint32)0,i_offsets_hi
		);
	}
}

/**************/

void radix256_dit_pass1(double a[], int n)
{
/*
!...Acronym: DIT = Decimation In Time
!
!...Subroutine to perform an initial radix-256 complex inverse DIT FFT pass on the data in the length-N real vector A.
!
!   See the documentation in radix256_dif_pass for further details on storage, indexing and DFT decomposition.
*/
	int j,j1,j2,jt,jp;
	static int NDIVR,p01,p02,p03,p04,p05,p06,p07,p08,p09,p0a,p0b,p0c,p0d,p0e,p0f,p10,p20,p30,p40,p50,p60,p70,p80,p90,pa0,pb0,pc0,pd0,pe0,pf0	, first_entry=TRUE;
	static int i_offsets_lo[16], i_offsets_hi[16];

	if(!first_entry && (n >> 8) != NDIVR)	/* New runlength?	*/
	{
		first_entry=TRUE;
	}

/*...initialize things upon first entry	*/

	if(first_entry)
	{
		first_entry=FALSE;
		NDIVR = n >> 8;

		p01 = NDIVR;
		p02 = p01 + p01;
		p03 = p02 + p01;
		p04 = p03 + p01;
		p05 = p04 + p01;
		p06 = p05 + p01;
		p07 = p06 + p01;
		p08 = p07 + p01;
		p09 = p08 + p01;
		p0a = p09 + p01;
		p0b = p0a + p01;
		p0c = p0b + p01;
		p0d = p0c + p01;
		p0e = p0d + p01;
		p0f = p0e + p01;
		p10 = p08 + p08;
		p20 = p10 + p10;
		p30 = p20 + p10;
		p40 = p30 + p10;
		p50 = p40 + p10;
		p60 = p50 + p10;
		p70 = p60 + p10;
		p80 = p70 + p10;
		p90 = p80 + p10;
		pa0 = p90 + p10;
		pb0 = pa0 + p10;
		pc0 = pb0 + p10;
		pd0 = pc0 + p10;
		pe0 = pd0 + p10;
		pf0 = pe0 + p10;

		p01 = p01 + ( (p01 >> DAT_BITS) << PAD_BITS );
		p02 = p02 + ( (p02 >> DAT_BITS) << PAD_BITS );
		p03 = p03 + ( (p03 >> DAT_BITS) << PAD_BITS );
		p04 = p04 + ( (p04 >> DAT_BITS) << PAD_BITS );
		p05 = p05 + ( (p05 >> DAT_BITS) << PAD_BITS );
		p06 = p06 + ( (p06 >> DAT_BITS) << PAD_BITS );
		p07 = p07 + ( (p07 >> DAT_BITS) << PAD_BITS );
		p08 = p08 + ( (p08 >> DAT_BITS) << PAD_BITS );
		p09 = p09 + ( (p09 >> DAT_BITS) << PAD_BITS );
		p0a = p0a + ( (p0a >> DAT_BITS) << PAD_BITS );
		p0b = p0b + ( (p0b >> DAT_BITS) << PAD_BITS );
		p0c = p0c + ( (p0c >> DAT_BITS) << PAD_BITS );
		p0d = p0d + ( (p0d >> DAT_BITS) << PAD_BITS );
		p0e = p0e + ( (p0e >> DAT_BITS) << PAD_BITS );
		p0f = p0f + ( (p0f >> DAT_BITS) << PAD_BITS );
		p10 = p10 + ( (p10 >> DAT_BITS) << PAD_BITS );
		p20 = p20 + ( (p20 >> DAT_BITS) << PAD_BITS );
		p30 = p30 + ( (p30 >> DAT_BITS) << PAD_BITS );
		p40 = p40 + ( (p40 >> DAT_BITS) << PAD_BITS );
		p50 = p50 + ( (p50 >> DAT_BITS) << PAD_BITS );
		p60 = p60 + ( (p60 >> DAT_BITS) << PAD_BITS );
		p70 = p70 + ( (p70 >> DAT_BITS) << PAD_BITS );
		p80 = p80 + ( (p80 >> DAT_BITS) << PAD_BITS );
		p90 = p90 + ( (p90 >> DAT_BITS) << PAD_BITS );
		pa0 = pa0 + ( (pa0 >> DAT_BITS) << PAD_BITS );
		pb0 = pb0 + ( (pb0 >> DAT_BITS) << PAD_BITS );
		pc0 = pc0 + ( (pc0 >> DAT_BITS) << PAD_BITS );
		pd0 = pd0 + ( (pd0 >> DAT_BITS) << PAD_BITS );
		pe0 = pe0 + ( (pe0 >> DAT_BITS) << PAD_BITS );
		pf0 = pf0 + ( (pf0 >> DAT_BITS) << PAD_BITS );

		// Set array offsets for in/outputs - low parts in fisrt 16 slots, high parts in next 16:
		i_offsets_lo[0x0] = 0  ;	i_offsets_hi[0x0] = 0  ;
		i_offsets_lo[0x1] = p01;	i_offsets_hi[0x1] = p10;
		i_offsets_lo[0x2] = p02;	i_offsets_hi[0x2] = p20;
		i_offsets_lo[0x3] = p03;	i_offsets_hi[0x3] = p30;
		i_offsets_lo[0x4] = p04;	i_offsets_hi[0x4] = p40;
		i_offsets_lo[0x5] = p05;	i_offsets_hi[0x5] = p50;
		i_offsets_lo[0x6] = p06;	i_offsets_hi[0x6] = p60;
		i_offsets_lo[0x7] = p07;	i_offsets_hi[0x7] = p70;
		i_offsets_lo[0x8] = p08;	i_offsets_hi[0x8] = p80;
		i_offsets_lo[0x9] = p09;	i_offsets_hi[0x9] = p90;
		i_offsets_lo[0xa] = p0a;	i_offsets_hi[0xa] = pa0;
		i_offsets_lo[0xb] = p0b;	i_offsets_hi[0xb] = pb0;
		i_offsets_lo[0xc] = p0c;	i_offsets_hi[0xc] = pc0;
		i_offsets_lo[0xd] = p0d;	i_offsets_hi[0xd] = pd0;
		i_offsets_lo[0xe] = p0e;	i_offsets_hi[0xe] = pe0;
		i_offsets_lo[0xf] = p0f;	i_offsets_hi[0xf] = pf0;
	}

/*...The radix-256 pass is here.	*/

	for(j = 0; j < NDIVR; j += 2)
	{
	#ifdef USE_AVX512
		j1 = (j & mask03) + br16[j&15];
	#elif defined(USE_AVX)
		j1 = (j & mask02) + br8[j&7];
	#elif defined(USE_SSE2)
		j1 = (j & mask01) + br4[j&3];
	#else
		j1 = j;
	#endif
		j1 =j1 + ( (j1>> DAT_BITS) << PAD_BITS );	/* padded-array fetch index is here */
		j2 = j1+RE_IM_STRIDE;
		RADIX_256_DIT(
			(a+j1),RE_IM_STRIDE,i_offsets_lo,(uint32)0,i_offsets_hi,
			(a+j1),RE_IM_STRIDE,i_offsets_lo,i_offsets_hi
		);
	}
}

/******************** Multithreaded function body - NO STATIC VARS BELOW THIS POINT!: ***************************/

#ifdef USE_PTHREAD

	#ifndef COMPILER_TYPE_GCC
		#error pthreaded carry code requires GCC build!
	#endif

	void*
	cy256_process_chunk(void*targ)	// Thread-arg pointer *must* be cast to void and specialized inside the function
	{
		struct cy_thread_data_t* thread_arg = targ;	// Move to top because scalar-mode carry pointers taken directly from it
		double *addr,*addi;
		struct complex *tptr;
		const int pfetch_dist = PFETCH_DIST;
		const int stride = (int)RE_IM_STRIDE << 1;	// main-array loop stride = 2*RE_IM_STRIDE
		uint32 p01,p02,p03,p04,p05,p06,p07,p08,p09,p0a,p0b,p0c,p0d,p0e,p0f,p10,p20,p30,p40,p50,p60,p70,p80,p90,pa0,pb0,pc0,pd0,pe0,pf0;
		int poff[RADIX>>2];
	#if USE_SCALAR_DFT_MACRO
		int dft_offsets_lo[16], dft_offsets_hi[16];
	#endif
	#if !USE_SCALAR_DFT_MACRO || !defined(USE_SSE2)	// SIMD build uses these for radix-256 DFTs; Scalar-double build uses these for carry-macro loops
		int po_br[16];
	#endif
		int j,j1,j2,jt,jp,k,l;
		double wtl,wtlp1,wtn,wtnm1;	/* Mersenne-mod weights stuff */
	#ifdef USE_AVX512
		double t0,t1,t2,t3;
	  #ifdef CARRY_16_WAY
		struct uint32x16 *n_minus_sil,*n_minus_silp1,*sinwt,*sinwtm1;
	  #else
		struct uint32x8  *n_minus_sil,*n_minus_silp1,*sinwt,*sinwtm1;
	  #endif
	#elif defined(USE_AVX)
		struct uint32x4 *n_minus_sil,*n_minus_silp1,*sinwt,*sinwtm1;
	#else
		int n_minus_sil,n_minus_silp1,sinwt,sinwtm1;
	#endif
		double rt,it, wt_re,wt_im, wi_re,wi_im;	// Fermat-mod weights stuff, used in both scalar and AVX mode
		int k1,k2;

	#ifdef USE_SSE2

		const double crnd = 3.0*0x4000000*0x2000000;
		double *add0, *add1, *add2, *add3;	// Latter 3 also used for carries
	   #if !USE_SCALAR_DFT_MACRO
		double *add4, *add5, *add6, *add7, *add8, *add9, *adda, *addb, *addc, *addd, *adde, *addf;
	   #endif
		int *bjmodn;	// Alloc mem for this along with other 	SIMD stuff
		vec_dbl *tmp,*tm0,*tm1,*tm2;	// utility ptrs
		int *itmp,*itm2;			// Pointer into the bjmodn array
		struct complex *ctmp;	// Hybrid AVX-DFT/SSE2-carry scheme used for Mersenne-mod needs a 2-word-double pointer
		vec_dbl *max_err, *sse2_rnd, *half_arr, *two,*one,*sqrt2,*isrt2, *cc0,*ss0,
			// ptrs to 16 sets of non-unity twiddles shared by the 2nd-half DIF and DIT DFT macros:
			*twid0,*twid1,*twid2,*twid3,*twid4,*twid5,*twid6,*twid7,*twid8,*twid9,*twida,*twidb,*twidc,*twidd,*twide,*twidf,
			*r00,*r01,*r02,*r03,*r04,*r05,*r06,*r07,*r08,*r09,*r0a,*r0b,*r0c,*r0d,*r0e,*r0f,
			*r10,*r11,*r12,*r13,*r14,*r15,*r16,*r17,*r18,*r19,*r1a,*r1b,*r1c,*r1d,*r1e,*r1f,
			*r20,*r21,*r22,*r23,*r24,*r25,*r26,*r27,*r28,*r29,*r2a,*r2b,*r2c,*r2d,*r2e,*r2f,
			*r30,*r31,*r32,*r33,*r34,*r35,*r36,*r37,*r38,*r39,*r3a,*r3b,*r3c,*r3d,*r3e,*r3f,
			*r40,*r41,*r42,*r43,*r44,*r45,*r46,*r47,*r48,*r49,*r4a,*r4b,*r4c,*r4d,*r4e,*r4f,
			*r50,*r51,*r52,*r53,*r54,*r55,*r56,*r57,*r58,*r59,*r5a,*r5b,*r5c,*r5d,*r5e,*r5f,
			*r60,*r61,*r62,*r63,*r64,*r65,*r66,*r67,*r68,*r69,*r6a,*r6b,*r6c,*r6d,*r6e,*r6f,
			*r70,*r71,*r72,*r73,*r74,*r75,*r76,*r77,*r78,*r79,*r7a,*r7b,*r7c,*r7d,*r7e,*r7f,
			*r80,*r81,*r82,*r83,*r84,*r85,*r86,*r87,*r88,*r89,*r8a,*r8b,*r8c,*r8d,*r8e,*r8f,
			*r90,*r91,*r92,*r93,*r94,*r95,*r96,*r97,*r98,*r99,*r9a,*r9b,*r9c,*r9d,*r9e,*r9f,
			*ra0,*ra1,*ra2,*ra3,*ra4,*ra5,*ra6,*ra7,*ra8,*ra9,*raa,*rab,*rac,*rad,*rae,*raf,
			*rb0,*rb1,*rb2,*rb3,*rb4,*rb5,*rb6,*rb7,*rb8,*rb9,*rba,*rbb,*rbc,*rbd,*rbe,*rbf,
			*rc0,*rc1,*rc2,*rc3,*rc4,*rc5,*rc6,*rc7,*rc8,*rc9,*rca,*rcb,*rcc,*rcd,*rce,*rcf,
			*rd0,*rd1,*rd2,*rd3,*rd4,*rd5,*rd6,*rd7,*rd8,*rd9,*rda,*rdb,*rdc,*rdd,*rde,*rdf,
			*re0,*re1,*re2,*re3,*re4,*re5,*re6,*re7,*re8,*re9,*rea,*reb,*rec,*red,*ree,*ref,
			*rf0,*rf1,*rf2,*rf3,*rf4,*rf5,*rf6,*rf7,*rf8,*rf9,*rfa,*rfb,*rfc,*rfd,*rfe,*rff,
			*s1p00,*s1p01,*s1p02,*s1p03,*s1p04,*s1p05,*s1p06,*s1p07,*s1p08,*s1p09,*s1p0a,*s1p0b,*s1p0c,*s1p0d,*s1p0e,*s1p0f,
			*s1p10,*s1p11,*s1p12,*s1p13,*s1p14,*s1p15,*s1p16,*s1p17,*s1p18,*s1p19,*s1p1a,*s1p1b,*s1p1c,*s1p1d,*s1p1e,*s1p1f,
			*s1p20,*s1p21,*s1p22,*s1p23,*s1p24,*s1p25,*s1p26,*s1p27,*s1p28,*s1p29,*s1p2a,*s1p2b,*s1p2c,*s1p2d,*s1p2e,*s1p2f,
			*s1p30,*s1p31,*s1p32,*s1p33,*s1p34,*s1p35,*s1p36,*s1p37,*s1p38,*s1p39,*s1p3a,*s1p3b,*s1p3c,*s1p3d,*s1p3e,*s1p3f,
			*s1p40,*s1p41,*s1p42,*s1p43,*s1p44,*s1p45,*s1p46,*s1p47,*s1p48,*s1p49,*s1p4a,*s1p4b,*s1p4c,*s1p4d,*s1p4e,*s1p4f,
			*s1p50,*s1p51,*s1p52,*s1p53,*s1p54,*s1p55,*s1p56,*s1p57,*s1p58,*s1p59,*s1p5a,*s1p5b,*s1p5c,*s1p5d,*s1p5e,*s1p5f,
			*s1p60,*s1p61,*s1p62,*s1p63,*s1p64,*s1p65,*s1p66,*s1p67,*s1p68,*s1p69,*s1p6a,*s1p6b,*s1p6c,*s1p6d,*s1p6e,*s1p6f,
			*s1p70,*s1p71,*s1p72,*s1p73,*s1p74,*s1p75,*s1p76,*s1p77,*s1p78,*s1p79,*s1p7a,*s1p7b,*s1p7c,*s1p7d,*s1p7e,*s1p7f,
			*s1p80,*s1p81,*s1p82,*s1p83,*s1p84,*s1p85,*s1p86,*s1p87,*s1p88,*s1p89,*s1p8a,*s1p8b,*s1p8c,*s1p8d,*s1p8e,*s1p8f,
			*s1p90,*s1p91,*s1p92,*s1p93,*s1p94,*s1p95,*s1p96,*s1p97,*s1p98,*s1p99,*s1p9a,*s1p9b,*s1p9c,*s1p9d,*s1p9e,*s1p9f,
			*s1pa0,*s1pa1,*s1pa2,*s1pa3,*s1pa4,*s1pa5,*s1pa6,*s1pa7,*s1pa8,*s1pa9,*s1paa,*s1pab,*s1pac,*s1pad,*s1pae,*s1paf,
			*s1pb0,*s1pb1,*s1pb2,*s1pb3,*s1pb4,*s1pb5,*s1pb6,*s1pb7,*s1pb8,*s1pb9,*s1pba,*s1pbb,*s1pbc,*s1pbd,*s1pbe,*s1pbf,
			*s1pc0,*s1pc1,*s1pc2,*s1pc3,*s1pc4,*s1pc5,*s1pc6,*s1pc7,*s1pc8,*s1pc9,*s1pca,*s1pcb,*s1pcc,*s1pcd,*s1pce,*s1pcf,
			*s1pd0,*s1pd1,*s1pd2,*s1pd3,*s1pd4,*s1pd5,*s1pd6,*s1pd7,*s1pd8,*s1pd9,*s1pda,*s1pdb,*s1pdc,*s1pdd,*s1pde,*s1pdf,
			*s1pe0,*s1pe1,*s1pe2,*s1pe3,*s1pe4,*s1pe5,*s1pe6,*s1pe7,*s1pe8,*s1pe9,*s1pea,*s1peb,*s1pec,*s1ped,*s1pee,*s1pef,
			*s1pf0,*s1pf1,*s1pf2,*s1pf3,*s1pf4,*s1pf5,*s1pf6,*s1pf7,*s1pf8,*s1pf9,*s1pfa,*s1pfb,*s1pfc,*s1pfd,*s1pfe,*s1pff,
			*cy_r,*cy_i;	// Need RADIX slots for sse2 carries, RADIX/2 for avx
	  #ifdef USE_AVX
		vec_dbl *base_negacyclic_root;
	  #endif

		/* These are used in conjunction with the langth-odd_radix arrays in the USE_SCALAR_CARRY #define below;
		In SSE2 mode store doubled versions of these data in the scratch storage accessed via the half_arr pointer: */
		int idx_offset, idx_incr;
		double dtmp;
		uint64 *sign_mask, *sse_bw, *sse_sw, *sse_nm1;

	#else

		double *base, *baseinv;
		const  double one_half[3] = {1.0, 0.5, 0.25};	/* Needed for small-weights-tables scheme */
		int m,m2,ntmp;
		double wt,wtinv,wtA,wtB,wtC;	/* Mersenne-mod weights stuff */
		int bjmodn[RADIX];	// Thread only carries a base datum here, must alloc a local array for remaining values
		double *cy_r = thread_arg->cy_r,*cy_i = thread_arg->cy_i, temp,frac;
		// Local storage: We must use an array here because scalars have no guarantees about relative address offsets
		// [and even if those are contiguous-as-hoped-for, they may run in reverse]; Make array type (struct complex)
		// to allow us to use the same offset-indexing as in the original radix-32 in-place DFT macros:
		struct complex t[RADIX];
		#include "radix256_twiddles.h"
		int *itmp;	// Pointer into the bjmodn array

	#endif

	// int data:
		int thr_id = thread_arg->tid;
		int iter = thread_arg->iter;
		int NDIVR = thread_arg->ndivr;
		int n = NDIVR*RADIX, nm1 = n-1;
		int khi    = thread_arg->khi;
		int i      = thread_arg->i;	/* Pointer to the BASE and BASEINV arrays.	*/
		int jstart = thread_arg->jstart;
		int jhi    = thread_arg->jhi;
		int col = thread_arg->col;
		int co2 = thread_arg->co2;
		int co3 = thread_arg->co3;
		int sw  = thread_arg->sw, bw = n - sw;
		int nwt = thread_arg->nwt;

	// double data:
		double maxerr = thread_arg->maxerr;
		double scale = thread_arg->scale;	int full_pass = scale < 0.5;
	// pointer data:
		double *a = thread_arg->arrdat;
		double *wt0 = thread_arg->wt0;
		double *wt1 = thread_arg->wt1;
	#ifdef LOACC
		double *wts_mult = thread_arg->wts_mult;	// Const Intra-block wts-multiplier...
		double *inv_mult = thread_arg->inv_mult;	// ...and 2*(its multiplicative inverse).
		ASSERT(HERE,fabs(wts_mult[0]*inv_mult[0] - 1.0) < EPS, "wts_mults fail accuracy check!");
		ASSERT(HERE,fabs(wts_mult[1]*inv_mult[1] - 1.0) < EPS, "wts_mults fail accuracy check!");
	#endif
		int *si = thread_arg->si;
		struct complex *rn0 = thread_arg->rn0;
		struct complex *rn1 = thread_arg->rn1;

		/*   constant index offsets for load/stores are here.	*/
		p01 = NDIVR;
		p02 = p01 + p01;
		p03 = p02 + p01;
		p04 = p03 + p01;
		p05 = p04 + p01;
		p06 = p05 + p01;
		p07 = p06 + p01;
		p08 = p07 + p01;
		p09 = p08 + p01;
		p0a = p09 + p01;
		p0b = p0a + p01;
		p0c = p0b + p01;
		p0d = p0c + p01;
		p0e = p0d + p01;
		p0f = p0e + p01;
		p10 = p08 + p08;
		p20 = p10 + p10;
		p30 = p20 + p10;
		p40 = p30 + p10;
		p50 = p40 + p10;
		p60 = p50 + p10;
		p70 = p60 + p10;
		p80 = p70 + p10;
		p90 = p80 + p10;
		pa0 = p90 + p10;
		pb0 = pa0 + p10;
		pc0 = pb0 + p10;
		pd0 = pc0 + p10;
		pe0 = pd0 + p10;
		pf0 = pe0 + p10;

		p01 = p01 + ( (p01 >> DAT_BITS) << PAD_BITS );
		p02 = p02 + ( (p02 >> DAT_BITS) << PAD_BITS );
		p03 = p03 + ( (p03 >> DAT_BITS) << PAD_BITS );
		p04 = p04 + ( (p04 >> DAT_BITS) << PAD_BITS );
		p05 = p05 + ( (p05 >> DAT_BITS) << PAD_BITS );
		p06 = p06 + ( (p06 >> DAT_BITS) << PAD_BITS );
		p07 = p07 + ( (p07 >> DAT_BITS) << PAD_BITS );
		p08 = p08 + ( (p08 >> DAT_BITS) << PAD_BITS );
		p09 = p09 + ( (p09 >> DAT_BITS) << PAD_BITS );
		p0a = p0a + ( (p0a >> DAT_BITS) << PAD_BITS );
		p0b = p0b + ( (p0b >> DAT_BITS) << PAD_BITS );
		p0c = p0c + ( (p0c >> DAT_BITS) << PAD_BITS );
		p0d = p0d + ( (p0d >> DAT_BITS) << PAD_BITS );
		p0e = p0e + ( (p0e >> DAT_BITS) << PAD_BITS );
		p0f = p0f + ( (p0f >> DAT_BITS) << PAD_BITS );
		p10 = p10 + ( (p10 >> DAT_BITS) << PAD_BITS );
		p20 = p20 + ( (p20 >> DAT_BITS) << PAD_BITS );
		p30 = p30 + ( (p30 >> DAT_BITS) << PAD_BITS );
		p40 = p40 + ( (p40 >> DAT_BITS) << PAD_BITS );
		p50 = p50 + ( (p50 >> DAT_BITS) << PAD_BITS );
		p60 = p60 + ( (p60 >> DAT_BITS) << PAD_BITS );
		p70 = p70 + ( (p70 >> DAT_BITS) << PAD_BITS );
		p80 = p80 + ( (p80 >> DAT_BITS) << PAD_BITS );
		p90 = p90 + ( (p90 >> DAT_BITS) << PAD_BITS );
		pa0 = pa0 + ( (pa0 >> DAT_BITS) << PAD_BITS );
		pb0 = pb0 + ( (pb0 >> DAT_BITS) << PAD_BITS );
		pc0 = pc0 + ( (pc0 >> DAT_BITS) << PAD_BITS );
		pd0 = pd0 + ( (pd0 >> DAT_BITS) << PAD_BITS );
		pe0 = pe0 + ( (pe0 >> DAT_BITS) << PAD_BITS );
		pf0 = pf0 + ( (pf0 >> DAT_BITS) << PAD_BITS );

		poff[     0] =   0; poff[     1] =     p04; poff[     2] =     p08; poff[     3] =     p0c;
		poff[0x04+0] = p10; poff[0x04+1] = p10+p04; poff[0x04+2] = p10+p08; poff[0x04+3] = p10+p0c;
		poff[0x08+0] = p20; poff[0x08+1] = p20+p04; poff[0x08+2] = p20+p08; poff[0x08+3] = p20+p0c;
		poff[0x0c+0] = p30; poff[0x0c+1] = p30+p04; poff[0x0c+2] = p30+p08; poff[0x0c+3] = p30+p0c;
		poff[0x10+0] = p40; poff[0x10+1] = p40+p04; poff[0x10+2] = p40+p08; poff[0x10+3] = p40+p0c;
		poff[0x14+0] = p50; poff[0x14+1] = p50+p04; poff[0x14+2] = p50+p08; poff[0x14+3] = p50+p0c;
		poff[0x18+0] = p60; poff[0x18+1] = p60+p04; poff[0x18+2] = p60+p08; poff[0x18+3] = p60+p0c;
		poff[0x1c+0] = p70; poff[0x1c+1] = p70+p04; poff[0x1c+2] = p70+p08; poff[0x1c+3] = p70+p0c;
		poff[0x20+0] = p80; poff[0x20+1] = p80+p04; poff[0x20+2] = p80+p08; poff[0x20+3] = p80+p0c;
		poff[0x24+0] = p90; poff[0x24+1] = p90+p04; poff[0x24+2] = p90+p08; poff[0x24+3] = p90+p0c;
		poff[0x28+0] = pa0; poff[0x28+1] = pa0+p04; poff[0x28+2] = pa0+p08; poff[0x28+3] = pa0+p0c;
		poff[0x2c+0] = pb0; poff[0x2c+1] = pb0+p04; poff[0x2c+2] = pb0+p08; poff[0x2c+3] = pb0+p0c;
		poff[0x30+0] = pc0; poff[0x30+1] = pc0+p04; poff[0x30+2] = pc0+p08; poff[0x30+3] = pc0+p0c;
		poff[0x34+0] = pd0; poff[0x34+1] = pd0+p04; poff[0x34+2] = pd0+p08; poff[0x34+3] = pd0+p0c;
		poff[0x38+0] = pe0; poff[0x38+1] = pe0+p04; poff[0x38+2] = pe0+p08; poff[0x38+3] = pe0+p0c;
		poff[0x3c+0] = pf0; poff[0x3c+1] = pf0+p04; poff[0x3c+2] = pf0+p08; poff[0x3c+3] = pf0+p0c;

	#if USE_SCALAR_DFT_MACRO
		// Set array offsets for in/outputs - low parts in first 16 slots, high parts in next 16:
		dft_offsets_lo[0x0] = 0  ;	dft_offsets_hi[0x0] = 0  ;
		dft_offsets_lo[0x1] = p01;	dft_offsets_hi[0x1] = p10;
		dft_offsets_lo[0x2] = p02;	dft_offsets_hi[0x2] = p20;
		dft_offsets_lo[0x3] = p03;	dft_offsets_hi[0x3] = p30;
		dft_offsets_lo[0x4] = p04;	dft_offsets_hi[0x4] = p40;
		dft_offsets_lo[0x5] = p05;	dft_offsets_hi[0x5] = p50;
		dft_offsets_lo[0x6] = p06;	dft_offsets_hi[0x6] = p60;
		dft_offsets_lo[0x7] = p07;	dft_offsets_hi[0x7] = p70;
		dft_offsets_lo[0x8] = p08;	dft_offsets_hi[0x8] = p80;
		dft_offsets_lo[0x9] = p09;	dft_offsets_hi[0x9] = p90;
		dft_offsets_lo[0xa] = p0a;	dft_offsets_hi[0xa] = pa0;
		dft_offsets_lo[0xb] = p0b;	dft_offsets_hi[0xb] = pb0;
		dft_offsets_lo[0xc] = p0c;	dft_offsets_hi[0xc] = pc0;
		dft_offsets_lo[0xd] = p0d;	dft_offsets_hi[0xd] = pd0;
		dft_offsets_lo[0xe] = p0e;	dft_offsets_hi[0xe] = pe0;
		dft_offsets_lo[0xf] = p0f;	dft_offsets_hi[0xf] = pf0;
	#endif
	#if !USE_SCALAR_DFT_MACRO || !defined(USE_SSE2)	// SIMD build uses these for radix-256 DFTs; Scalar-double build uses these for carry-macro loops
		po_br[0x0] =   0; po_br[0x1] = p08; po_br[0x2] = p04; po_br[0x3] = p0c;
		po_br[0x4] = p02; po_br[0x5] = p0a; po_br[0x6] = p06; po_br[0x7] = p0e;
		po_br[0x8] = p01; po_br[0x9] = p09; po_br[0xa] = p05; po_br[0xb] = p0d;
		po_br[0xc] = p03; po_br[0xd] = p0b; po_br[0xe] = p07; po_br[0xf] = p0f;
	#endif

	#ifdef USE_SSE2
		r00 = thread_arg->r00;	// declared above
		tmp = r00;				tm2 = tmp + 0x100;
		r00 = tmp + 0x00;		r80 = tm2 + 0x00;
		r01 = tmp + 0x02;
		r02 = tmp + 0x04;
		r08 = tmp + 0x10;
		r10 = tmp + 0x20;		r90 = tm2 + 0x20;
		r20 = tmp + 0x40;		ra0 = tm2 + 0x40;
		r30 = tmp + 0x60;		rb0 = tm2 + 0x60;
		r40 = tmp + 0x80;		rc0 = tm2 + 0x80;
		r50 = tmp + 0xa0;		rd0 = tm2 + 0xa0;
		r60 = tmp + 0xc0;		re0 = tm2 + 0xc0;
		r70 = tmp + 0xe0;		rf0 = tm2 + 0xe0;
		tmp += 0x200;
		s1p00 = tmp + 0x00;		s1p08 = tmp + 0x10;
		tmp += 0x200;
		two    = tmp + 0;	// AVX+ versions of various DFT macros assume consts 2.0,1.0,isrt2 laid out thusly
		one    = tmp + 1;
		sqrt2  = tmp + 2;
		isrt2  = tmp + 3;	// Radix-16 DFT macros assume [isrt2,cc0,ss0] contiguous in memory
		cc0    = tmp + 4;
		ss0    = tmp + 5;
		tmp += 0x6;
		// ptrs to 16 sets (2*15 = 30 vec_dbl data each) of non-unity twiddles shared by the 2nd-half DIF and DIT DFT macros:
		twid0  = tmp + 0x00;
		twid1  = tmp + 0x1e;
		twid2  = tmp + 0x3c;
		twid3  = tmp + 0x5a;
		twid4  = tmp + 0x78;
		twid5  = tmp + 0x96;
		twid6  = tmp + 0xb4;
		twid7  = tmp + 0xd2;
		twid8  = tmp + 0xf0;
		twid9  = tmp + 0x10e;
		twida  = tmp + 0x12c;
		twidb  = tmp + 0x14a;
		twidc  = tmp + 0x168;
		twidd  = tmp + 0x186;
		twide  = tmp + 0x1a4;
		twidf  = tmp + 0x1c2;
		tmp += 0x1e0;	// += 16*30 = 16*0x1e
	  #ifdef USE_AVX512
		cy_r = tmp;	cy_i = tmp+0x20;	tmp += 2*0x20;	// RADIX/8 vec_dbl slots for each of cy_r and cy_i carry sub-arrays
		max_err = tmp + 0x00;
		sse2_rnd= tmp + 0x01;
		half_arr= tmp + 0x02;
		base_negacyclic_root = half_arr + RADIX;	// Only used for Fermat-mod
	  #elif defined(USE_AVX)
		cy_r = tmp;	cy_i = tmp+0x40;	tmp += 2*0x40;	// RADIX/4 vec_dbl slots for each of cy_r and cy_i carry sub-arrays
		max_err = tmp + 0x00;
		sse2_rnd= tmp + 0x01;	// 0x645 +2 = 0x647 = 1607 vec_dbl
		// This is where the value of half_arr_offset comes from
		half_arr= tmp + 0x02;	/* This table needs 68 vec_dbl for Mersenne-mod, and 3.5*RADIX[avx] | RADIX[sse2] for Fermat-mod */
		base_negacyclic_root = half_arr + RADIX;	// Only used for Fermat-mod
	  #else
		cy_r = tmp;	cy_i = tmp+0x80;	tmp += 2*0x80;	// RADIX/2 vec_dbl slots for each of cy_r and cy_i carry sub-arrays
		max_err = tmp + 0x00;
		sse2_rnd= tmp + 0x01;	// 0x6c5 +2 = 0x6c7 = 1735 complex
		// This is where the value of half_arr_offset comes from
		half_arr= tmp + 0x02;	/* This table needs 20 x 16 bytes for Mersenne-mod, 2 for Fermat-mod */
	  #endif

		ASSERT(HERE, (r00 == thread_arg->r00), "thread-local memcheck failed!");
		ASSERT(HERE, (half_arr == thread_arg->half_arr), "thread-local memcheck failed!");
		ASSERT(HERE, (two->d0 == 2.0 && two->d1 == 2.0), "thread-local memcheck failed!");
	//	ASSERT(HERE, (isrt2->d0 == ISRT2 && isrt2->d1 == ISRT2), "thread-local memcheck failed!");	Disable to allow alternate "rounded down" variant of isrt2,sqrt2
	  #ifndef USE_AVX512	// In AVX-512 mode, use VRNDSCALEPD for rounding and hijack this vector-data slot for the 4 base/baseinv-consts:
		ASSERT(HERE, (sse2_rnd->d0 == crnd && sse2_rnd->d1 == crnd), "thread-local memcheck failed!");
	  #endif
	
		if(MODULUS_TYPE == MODULUS_TYPE_MERSENNE)
		{
			tmp = half_arr;
		#ifdef USE_AVX512
			/* No-Op */
		#elif defined(USE_AVX)
			// Grab some elt of base-data [offset by, say, +32] and mpy by its inverse [+16 further]
			dtmp = (tmp+40)->d0 * (tmp+56)->d0;	ASSERT(HERE, fabs(dtmp - 1.0) < EPS, "thread-local memcheck failed!");
			dtmp = (tmp+40)->d1 * (tmp+56)->d1;	ASSERT(HERE, fabs(dtmp - 1.0) < EPS, "thread-local memcheck failed!");
		#else	// SSE2:
			dtmp = (tmp+10)->d0 * (tmp+14)->d0;	ASSERT(HERE, fabs(dtmp - 1.0) < EPS, "thread-local memcheck failed!");
			dtmp = (tmp+10)->d1 * (tmp+14)->d1;	ASSERT(HERE, fabs(dtmp - 1.0) < EPS, "thread-local memcheck failed!");
		#endif
		} else {
		#ifdef USE_AVX512
			/* No-Op */
		#else
			dtmp = (half_arr)->d0 * (half_arr+1)->d0;	ASSERT(HERE, fabs(dtmp - 1.0) < EPS, "thread-local memcheck failed!");
			dtmp = (half_arr)->d1 * (half_arr+1)->d1;	ASSERT(HERE, fabs(dtmp - 1.0) < EPS, "thread-local memcheck failed!");
		#endif
		}

		VEC_DBL_INIT(max_err, 0.0);

		sign_mask = (uint64*)(r00 + radix256_creals_in_local_store);
		sse_bw  = sign_mask + RE_IM_STRIDE;	// (#doubles in a SIMD complex) x 32-bits = RE_IM_STRIDE x 64-bits
		sse_sw  = sse_bw    + RE_IM_STRIDE;
		sse_nm1 = sse_sw    + RE_IM_STRIDE;

	  #ifdef USE_AVX512
	   #ifdef CARRY_16_WAY
		n_minus_sil   = (struct uint32x16*)sse_nm1 + 1;
		n_minus_silp1 = (struct uint32x16*)sse_nm1 + 2;
		sinwt         = (struct uint32x16*)sse_nm1 + 3;
		sinwtm1       = (struct uint32x16*)sse_nm1 + 4;
	   #else
		n_minus_sil   = (struct uint32x8 *)sse_nm1 + 1;
		n_minus_silp1 = (struct uint32x8 *)sse_nm1 + 2;
		sinwt         = (struct uint32x8 *)sse_nm1 + 3;
		sinwtm1       = (struct uint32x8 *)sse_nm1 + 4;
	   #endif
	  #elif defined(USE_AVX)
		n_minus_sil   = (struct uint32x4 *)sse_nm1 + 1;
		n_minus_silp1 = (struct uint32x4 *)sse_nm1 + 2;
		sinwt         = (struct uint32x4 *)sse_nm1 + 3;
		sinwtm1       = (struct uint32x4 *)sse_nm1 + 4;
	  #endif
	  #ifdef USE_AVX
		bjmodn = (int*)(sinwtm1 + RE_IM_STRIDE);
	  #else
		bjmodn = (int*)(sse_nm1 + RE_IM_STRIDE);
	  #endif

	#else

		// In scalar mode use these 2 ptrs to pass the base & baseinv arrays:
		base    = (double *)thread_arg->r00     ;
		baseinv = (double *)thread_arg->half_arr;

	#endif	// USE_SSE2 ?

		if(MODULUS_TYPE == MODULUS_TYPE_MERSENNE)
		{
			/* Init DWT-indices: */
			uint32 bjmodnini = thread_arg->bjmodnini;
			bjmodn[0] = thread_arg->bjmodn0;
			for(l = 1; l < RADIX; l++) {	// must use e.g. l for loop idx here as i is used for dwt indexing
				MOD_ADD32(bjmodn[l-1], bjmodnini, n, bjmodn[l]);
			}

			/* init carries	*/
			addr = thread_arg->cy_r;
		#ifdef USE_AVX512
			tmp = cy_r;
			for(l = 0; l < RADIX; l += 8, ++tmp) {
				tmp->d0 = *(addr+l  );
				tmp->d1 = *(addr+l+1);
				tmp->d2 = *(addr+l+2);
				tmp->d3 = *(addr+l+3);
				tmp->d4 = *(addr+l+4);
				tmp->d5 = *(addr+l+5);
				tmp->d6 = *(addr+l+6);
				tmp->d7 = *(addr+l+7);
			}
		#elif defined(USE_AVX)
			tmp = cy_r;
			for(l = 0; l < RADIX; l += 4, ++tmp) {
				tmp->d0 = *(addr+l  );
				tmp->d1 = *(addr+l+1);
				tmp->d2 = *(addr+l+2);
				tmp->d3 = *(addr+l+3);
			}
		#elif defined(USE_SSE2)
			tmp = cy_r;
			for(l = 0; l < RADIX; l += 2, ++tmp) {
				tmp->d0 = *(addr+l  );
				tmp->d1 = *(addr+l+1);
			}
		#elif 0	// No_op in scalar case, since carry pattern matches that of thread data
			for(l = 0; l < RADIX; l++) {
				cy_r[l] = *(addr+l);
			}
		#endif
		}
		else	/* Fermat-mod uses "double helix" carry scheme - 2 separate sets of real/imaginary carries for right-angle transform, plus "twisted" wraparound step. */
		{
			/* init carries	*/
			addr = thread_arg->cy_r;	addi = thread_arg->cy_i;
		#ifdef USE_AVX512
			tmp = cy_r;	tm2 = cy_i;
			for(l = 0; l < RADIX; l += 8, ++tmp, ++tm2) {
				tmp->d0 = *(addr+l  );		tm2->d0 = *(addi+l  );
				tmp->d1 = *(addr+l+1);		tm2->d1 = *(addi+l+1);
				tmp->d2 = *(addr+l+2);		tm2->d2 = *(addi+l+2);
				tmp->d3 = *(addr+l+3);		tm2->d3 = *(addi+l+3);
				tmp->d4 = *(addr+l+4);		tm2->d4 = *(addi+l+4);
				tmp->d5 = *(addr+l+5);		tm2->d5 = *(addi+l+5);
				tmp->d6 = *(addr+l+6);		tm2->d6 = *(addi+l+6);
				tmp->d7 = *(addr+l+7);		tm2->d7 = *(addi+l+7);
			}
		#elif defined(USE_AVX)
			tmp = cy_r;	tm2 = cy_i;
			for(l = 0; l < RADIX; l += 4, ++tmp, ++tm2) {
				tmp->d0 = *(addr+l  );		tm2->d0 = *(addi+l  );
				tmp->d1 = *(addr+l+1);		tm2->d1 = *(addi+l+1);
				tmp->d2 = *(addr+l+2);		tm2->d2 = *(addi+l+2);
				tmp->d3 = *(addr+l+3);		tm2->d3 = *(addi+l+3);
			}
		#elif defined(USE_SSE2)
			// Carry pattern for Fermat-mod in SSE2 mode is kinda funky:
			tmp = cy_r;
			for(l = 0; l < RADIX; l++, ++tmp) {
				// This relies on the cy_R,i sections of the SIMD data being contiguous, i.e.
				// step-thru the cy_r data via the tmp-pointer takes us seamlessly into the cy_i:
				tmp->d0 = *(addr+l  );		tmp->d1 = *(addi+l  );
			}
		#elif 0	// No_op in scalar case, since carry pattern matches that of thread data
			for(l = 0; l < RADIX; l++) {
				cy_r[l] = *(addr+l);		cy_i[l] = *(addi+l);
			}
		#endif
		}

		/********************************************************************************/
		/* This main loop is same for un-and-multithreaded, so stick into a header file */
		/* (can't use a macro because of the #if-enclosed stuff).                       */
		/********************************************************************************/
		#include "radix256_main_carry_loop.h"

		/* At end of each thread-processed work chunk, dump the
		carryouts into their non-thread-private array slots:
		*/
		if(MODULUS_TYPE == MODULUS_TYPE_MERSENNE)
		{
			addr = thread_arg->cy_r;
		#ifdef USE_AVX512
			tmp = cy_r;
			for(l = 0; l < RADIX; l += 8, ++tmp) {
				*(addr+l  ) = tmp->d0;
				*(addr+l+1) = tmp->d1;
				*(addr+l+2) = tmp->d2;
				*(addr+l+3) = tmp->d3;
				*(addr+l+4) = tmp->d4;
				*(addr+l+5) = tmp->d5;
				*(addr+l+6) = tmp->d6;
				*(addr+l+7) = tmp->d7;
			}
			t0 = MAX(max_err->d0,max_err->d1);
			t1 = MAX(max_err->d2,max_err->d3);
			t2 = MAX(max_err->d4,max_err->d5);
			t3 = MAX(max_err->d6,max_err->d7);
			maxerr = MAX( MAX(t0,t1), MAX(t2,t3) );
		#elif defined(USE_AVX)
			tmp = cy_r;
			for(l = 0; l < RADIX; l += 4, ++tmp) {
				*(addr+l  ) = tmp->d0;
				*(addr+l+1) = tmp->d1;
				*(addr+l+2) = tmp->d2;
				*(addr+l+3) = tmp->d3;
			}
			maxerr = MAX( MAX(max_err->d0,max_err->d1) , MAX(max_err->d2,max_err->d3) );
		#elif defined(USE_SSE2)
			tmp = cy_r;
			for(l = 0; l < RADIX; l += 2, ++tmp) {
				*(addr+l  ) = tmp->d0;
				*(addr+l+1) = tmp->d1;
			}
			maxerr = MAX(max_err->d0,max_err->d1);
		#elif 0	// No_op in scalar case, since carry pattern matches that of thread data
			for(l = 0; l < RADIX; l++) {
				*(addr+l) = cy_r[l];
			}
		#endif
		}
		else
		{
			addr = thread_arg->cy_r;	addi = thread_arg->cy_i;
		#ifdef USE_AVX512
			tmp = cy_r;	tm2 = cy_i;
			for(l = 0; l < RADIX; l += 8, ++tmp, ++tm2) {
				*(addr+l  ) = tmp->d0;		*(addi+l  ) = tm2->d0;
				*(addr+l+1) = tmp->d1;		*(addi+l+1) = tm2->d1;
				*(addr+l+2) = tmp->d2;		*(addi+l+2) = tm2->d2;
				*(addr+l+3) = tmp->d3;		*(addi+l+3) = tm2->d3;
				*(addr+l+4) = tmp->d4;		*(addi+l+4) = tm2->d4;
				*(addr+l+5) = tmp->d5;		*(addi+l+5) = tm2->d5;
				*(addr+l+6) = tmp->d6;		*(addi+l+6) = tm2->d6;
				*(addr+l+7) = tmp->d7;		*(addi+l+7) = tm2->d7;
			}
			t0 = MAX(max_err->d0,max_err->d1);
			t1 = MAX(max_err->d2,max_err->d3);
			t2 = MAX(max_err->d4,max_err->d5);
			t3 = MAX(max_err->d6,max_err->d7);
			maxerr = MAX( MAX(t0,t1), MAX(t2,t3) );
		#elif defined(USE_AVX)
			tmp = cy_r;	tm2 = cy_i;
			for(l = 0; l < RADIX; l += 4, ++tmp, ++tm2) {
				*(addr+l  ) = tmp->d0;		*(addi+l  ) = tm2->d0;
				*(addr+l+1) = tmp->d1;		*(addi+l+1) = tm2->d1;
				*(addr+l+2) = tmp->d2;		*(addi+l+2) = tm2->d2;
				*(addr+l+3) = tmp->d3;		*(addi+l+3) = tm2->d3;
			}
			maxerr = MAX( MAX(max_err->d0,max_err->d1) , MAX(max_err->d2,max_err->d3) );
		#elif defined(USE_SSE2)
			// Carry pattern for Fermat-mod in SSE2 mode is kinda funky:
			tmp = cy_r;
			for(l = 0; l < RADIX; l++, ++tmp) {
				// This relies on the cy_R,i sections of the SIMD data being contiguous, i.e.
				// step-thru the cy_r data via the tmp-pointer takes us seamlessly into the cy_i:
				*(addr+l  ) = tmp->d0;		*(addi+l  ) = tmp->d1;
			}
			maxerr = MAX(max_err->d0,max_err->d1);
		#elif 0	// No_op in scalar case, since carry pattern matches that of thread data
			for(l = 0; l < RADIX; l++) {
				*(addr+l) = cy_r[l];		*(addi+l) = cy_i[l];
			}
		#endif
		}

		/* Since will lose separate maxerr values when threads are merged, save them after each pass. */
		if(thread_arg->maxerr < maxerr)
		{
			thread_arg->maxerr = maxerr;
		}
		return 0x0;
	}
#endif

#undef RADIX
#undef PFETCH_DIST
