/*
 *      lame_intrin.h include file
 *
 *      Copyright (c) 2006 Gabriel Bouvigne
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#ifndef LAME_INTRIN_H
#define LAME_INTRIN_H

#if !defined(LAME_HAVE_SSE_INTRINSICS)
# if defined(HAVE_XMMINTRIN_H) || defined(__SSE__) || defined(_M_X64) || defined(_M_IX86_FP)
#  define LAME_HAVE_SSE_INTRINSICS 1
# else
#  define LAME_HAVE_SSE_INTRINSICS 0
# endif
#endif

#if !defined(LAME_HAVE_AARCH64_NEON_INTRINSICS)
# if defined(__aarch64__)
#  define LAME_HAVE_AARCH64_NEON_INTRINSICS 1
# else
#  define LAME_HAVE_AARCH64_NEON_INTRINSICS 0
# endif
#endif

#if !defined(LAME_HAVE_WASM_SIMD_INTRINSICS)
# if defined(__wasm_simd128__)
#  define LAME_HAVE_WASM_SIMD_INTRINSICS 1
# else
#  define LAME_HAVE_WASM_SIMD_INTRINSICS 0
# endif
#endif

#if LAME_HAVE_SSE_INTRINSICS
void
init_xrpow_core_sse(gr_info * const cod_info, FLOAT xrpow[576], int upper, FLOAT * sum);

void
fht_SSE2(FLOAT* , int);
#endif

#if LAME_HAVE_AARCH64_NEON_INTRINSICS
void
init_xrpow_core_neon(gr_info * const cod_info, FLOAT xrpow[576], int upper, FLOAT * sum);
#endif

#if LAME_HAVE_WASM_SIMD_INTRINSICS
void
init_xrpow_core_wasm(gr_info * const cod_info, FLOAT xrpow[576], int upper, FLOAT * sum);
#endif

#endif
