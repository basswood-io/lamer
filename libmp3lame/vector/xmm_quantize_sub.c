/*
 * MP3 quantization, intrinsics functions
 *
 *      Copyright (c) 2005-2006 Gabriel Bouvigne
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.     See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "lame.h"
#include "machine.h"
#include "encoder.h"
#include "util.h"
#include "lame_intrin.h"

#if LAME_HAVE_SSE_INTRINSICS || LAME_HAVE_AARCH64_NEON_INTRINSICS
#define TRI_SIZE (5-1)  /* 1024 =  4**5 */
static const FLOAT costab[TRI_SIZE * 2] = {
    9.238795325112867e-01, 3.826834323650898e-01,
    9.951847266721969e-01, 9.801714032956060e-02,
    9.996988186962042e-01, 2.454122852291229e-02,
    9.999811752826011e-01, 6.135884649154475e-03
};
#endif

#if LAME_HAVE_SSE_INTRINSICS
#include <xmmintrin.h>

typedef union {
    int32_t _i_32[4]; /* unions are initialized by its first member */
    float   _float[4];
    __m128  _m128;
} vecfloat_union;

/* make sure functions with SSE instructions maintain their own properly aligned stack */
#if defined (__GNUC__) && ((__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 2)))
#define SSE_FUNCTION __attribute__((force_align_arg_pointer))
#else
#define SSE_FUNCTION
#endif


SSE_FUNCTION void
init_xrpow_core_sse(gr_info * const cod_info, FLOAT xrpow[576], int max_nz, FLOAT * sum)
{
    int     i;
    float   tmp_max = 0;
    float   tmp_sum = 0;
    int     upper = max_nz + 1;
    int     upper4 = (upper / 4) * 4;

    const vecfloat_union fabs_mask = {{ 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF }};
    const __m128 vec_fabs_mask = _mm_loadu_ps(&fabs_mask._float[0]);
    vecfloat_union vec_xrpow_max;
    vecfloat_union vec_sum;
    vecfloat_union vec_tmp;

    _mm_prefetch((char const *) cod_info->xr, _MM_HINT_T0);
    _mm_prefetch((char const *) xrpow, _MM_HINT_T0);

    vec_xrpow_max._m128 = _mm_set_ps1(0);
    vec_sum._m128 = _mm_set_ps1(0);

    for (i = 0; i < upper4; i += 4) {
        vec_tmp._m128 = _mm_loadu_ps(&(cod_info->xr[i])); /* load */
        vec_tmp._m128 = _mm_and_ps(vec_tmp._m128, vec_fabs_mask); /* fabs */
        vec_sum._m128 = _mm_add_ps(vec_sum._m128, vec_tmp._m128);
        vec_tmp._m128 = _mm_sqrt_ps(_mm_mul_ps(vec_tmp._m128, _mm_sqrt_ps(vec_tmp._m128)));
        vec_xrpow_max._m128 = _mm_max_ps(vec_xrpow_max._m128, vec_tmp._m128); /* retrieve max */
        _mm_storeu_ps(&(xrpow[i]), vec_tmp._m128); /* store into xrpow[] */
    }
    tmp_sum = vec_sum._float[0] + vec_sum._float[1] + vec_sum._float[2] + vec_sum._float[3];
    {
        float ma = vec_xrpow_max._float[0] > vec_xrpow_max._float[1]
                ? vec_xrpow_max._float[0] : vec_xrpow_max._float[1];
        float mb = vec_xrpow_max._float[2] > vec_xrpow_max._float[3]
                ? vec_xrpow_max._float[2] : vec_xrpow_max._float[3];
        tmp_max = ma > mb ? ma : mb;
    }
    for (i = upper4; i < upper; ++i) {
        float const tmp = fabs(cod_info->xr[i]);
        float const xp = sqrt(tmp * sqrt(tmp));
        tmp_sum += tmp;
        xrpow[i] = xp;
        if (xp > tmp_max)
            tmp_max = xp;
    }
    cod_info->xrpow_max = tmp_max;
    *sum = tmp_sum;
}


SSE_FUNCTION static void
store4(__m128 v, float* f0, float* f1, float* f2, float* f3)
{
    vecfloat_union r;
    r._m128 = v;
    *f0 = r._float[0];
    *f1 = r._float[1];
    *f2 = r._float[2];
    *f3 = r._float[3];
}


SSE_FUNCTION void
fht_SSE2(FLOAT * fz, int n)
{
    const FLOAT *tri = costab;
    int     k4;
    FLOAT  *fi, *gi;
    FLOAT const *fn;

    n <<= 1;            /* convert the half-block length to BLKSIZE */
    fn = fz + n;
    k4 = 4;
    do {
        FLOAT   s1, c1;
        int     i, k1, k2, k3, kx;
        kx = k4 >> 1;
        k1 = k4;
        k2 = k4 << 1;
        k3 = k2 + k1;
        k4 = k2 << 1;
        fi = fz;
        gi = fi + kx;
        do {
            FLOAT   f0, f1, f2, f3;
            f1 = fi[0] - fi[k1];
            f0 = fi[0] + fi[k1];
            f3 = fi[k2] - fi[k3];
            f2 = fi[k2] + fi[k3];
            fi[k2] = f0 - f2;
            fi[0] = f0 + f2;
            fi[k3] = f1 - f3;
            fi[k1] = f1 + f3;
            f1 = gi[0] - gi[k1];
            f0 = gi[0] + gi[k1];
            f3 = SQRT2 * gi[k3];
            f2 = SQRT2 * gi[k2];
            gi[k2] = f0 - f2;
            gi[0] = f0 + f2;
            gi[k3] = f1 - f3;
            gi[k1] = f1 + f3;
            gi += k4;
            fi += k4;
        } while (fi < fn);
        c1 = tri[0];
        s1 = tri[1];
        for (i = 1; i < kx; i++) {
            __m128 v_s2;
            __m128 v_c2;
            __m128 v_c1;
            __m128 v_s1;
            FLOAT   c2, s2, s1_2 = s1+s1;
            c2 = 1 - s1_2 * s1;
            s2 = s1_2 * c1;
            fi = fz + i;
            gi = fz + k1 - i;
            v_c1 = _mm_set_ps1(c1);
            v_s1 = _mm_set_ps1(s1);
            v_c2 = _mm_set_ps1(c2);
            v_s2 = _mm_set_ps1(s2);
            {
                static const vecfloat_union sign_mask = {{0x80000000,0,0,0}};
                v_c1 = _mm_xor_ps(sign_mask._m128, v_c1); /* v_c1 := {-c1, +c1, +c1, +c1} */
            }
            {
                static const vecfloat_union sign_mask = {{0,0x80000000,0,0}};
                v_s1 = _mm_xor_ps(sign_mask._m128, v_s1); /* v_s1 := {+s1, -s1, +s1, +s1} */
            }
            {
                static const vecfloat_union sign_mask = {{0,0,0x80000000,0x80000000}};
                v_c2 = _mm_xor_ps(sign_mask._m128, v_c2); /* v_c2 := {+c2, +c2, -c2, -c2} */
            }
            do {
                __m128 p, q, r;

                q = _mm_setr_ps(fi[k1], fi[k3], gi[k1], gi[k3]); /* Q := {fi_k1,fi_k3,gi_k1,gi_k3}*/
                p = _mm_mul_ps(v_s2, q);                         /* P := s2 * Q */
                q = _mm_mul_ps(v_c2, q);                         /* Q := c2 * Q */
                q = _mm_shuffle_ps(q, q, _MM_SHUFFLE(1,0,3,2));  /* Q := {-c2*gi_k1,-c2*gi_k3,c2*fi_k1,c2*fi_k3} */
                p = _mm_add_ps(p, q);
                
                r = _mm_setr_ps(gi[0], gi[k2], fi[0], fi[k2]);   /* R := {gi_0,gi_k2,fi_0,fi_k2} */
                q = _mm_sub_ps(r, p);                            /* Q := {gi_0-p0,gi_k2-p1,fi_0-p2,fi_k2-p3} */
                r = _mm_add_ps(r, p);                            /* R := {gi_0+p0,gi_k2+p1,fi_0+p2,fi_k2+p3} */
                p = _mm_shuffle_ps(q, r, _MM_SHUFFLE(2,0,2,0));  /* P := {q0,q2,r0,r2} */
                p = _mm_shuffle_ps(p, p, _MM_SHUFFLE(3,1,2,0));  /* P := {q0,r0,q2,r2} */
                q = _mm_shuffle_ps(q, r, _MM_SHUFFLE(3,1,3,1));  /* Q := {q1,q3,r1,r3} */
                r = _mm_mul_ps(v_c1, q);
                q = _mm_mul_ps(v_s1, q);
                q = _mm_shuffle_ps(q, q, _MM_SHUFFLE(0,1,2,3));  /* Q := {q3,q2,q1,q0} */
                q = _mm_add_ps(q, r);

                store4(_mm_sub_ps(p, q), &gi[k3], &gi[k2], &fi[k3], &fi[k2]);
                store4(_mm_add_ps(p, q), &gi[k1], &gi[ 0], &fi[k1], &fi[ 0]);

                gi += k4;
                fi += k4;
            } while (fi < fn);
            c2 = c1;
            c1 = c2 * tri[0] - s1 * tri[1];
            s1 = c2 * tri[1] + s1 * tri[0];
        }
        tri += 2;
    } while (k4 < n);
}

#endif	/* LAME_HAVE_SSE_INTRINSICS */


#if LAME_HAVE_AARCH64_NEON_INTRINSICS

#include <arm_neon.h>

void
init_xrpow_core_neon(gr_info * const cod_info, FLOAT xrpow[576], int max_nz, FLOAT * sum)
{
    int     i;
    float   tmp_max;
    float   tmp_sum;
    int     upper = max_nz + 1;
    int     upper4 = (upper / 4) * 4;
    float32x4_t vec_sum = vdupq_n_f32(0.0f);
    float32x4_t vec_xrpow_max = vdupq_n_f32(0.0f);

    for (i = 0; i < upper4; i += 4) {
        float32x4_t vec_tmp = vld1q_f32(&cod_info->xr[i]);
        vec_tmp = vabsq_f32(vec_tmp);
        vec_sum = vaddq_f32(vec_sum, vec_tmp);
        vec_tmp = vsqrtq_f32(vmulq_f32(vec_tmp, vsqrtq_f32(vec_tmp)));
        vec_xrpow_max = vmaxq_f32(vec_xrpow_max, vec_tmp);
        vst1q_f32(&xrpow[i], vec_tmp);
    }

    tmp_sum = vaddvq_f32(vec_sum);
    tmp_max = vmaxvq_f32(vec_xrpow_max);
    for (i = upper4; i < upper; ++i) {
        float const tmp = fabs(cod_info->xr[i]);
        float const xp = sqrt(tmp * sqrt(tmp));
        tmp_sum += tmp;
        xrpow[i] = xp;
        if (xp > tmp_max)
            tmp_max = xp;
    }
    cod_info->xrpow_max = tmp_max;
    *sum = tmp_sum;
}

static float32x4_t
neon_setr4(float a, float b, float c, float d)
{
    float32x4_t v = vdupq_n_f32(0.0f);
    v = vsetq_lane_f32(a, v, 0);
    v = vsetq_lane_f32(b, v, 1);
    v = vsetq_lane_f32(c, v, 2);
    return vsetq_lane_f32(d, v, 3);
}

static float32x4_t
neon_reverse4(float32x4_t v)
{
    return vcombine_f32(vrev64_f32(vget_high_f32(v)), vrev64_f32(vget_low_f32(v)));
}

static void
store4_neon(float32x4_t v, float* f0, float* f1, float* f2, float* f3)
{
    *f0 = vgetq_lane_f32(v, 0);
    *f1 = vgetq_lane_f32(v, 1);
    *f2 = vgetq_lane_f32(v, 2);
    *f3 = vgetq_lane_f32(v, 3);
}

void
fht_neon(FLOAT * fz, int n)
{
    const FLOAT *tri = costab;
    int     k4;
    FLOAT  *fi, *gi;
    FLOAT const *fn;

    n <<= 1;            /* convert the half-block length to BLKSIZE */
    fn = fz + n;
    k4 = 4;
    do {
        FLOAT   s1, c1;
        int     i, k1, k2, k3, kx;
        kx = k4 >> 1;
        k1 = k4;
        k2 = k4 << 1;
        k3 = k2 + k1;
        k4 = k2 << 1;
        fi = fz;
        gi = fi + kx;
        do {
            FLOAT   f0, f1, f2, f3;
            f1 = fi[0] - fi[k1];
            f0 = fi[0] + fi[k1];
            f3 = fi[k2] - fi[k3];
            f2 = fi[k2] + fi[k3];
            fi[k2] = f0 - f2;
            fi[0] = f0 + f2;
            fi[k3] = f1 - f3;
            fi[k1] = f1 + f3;
            f1 = gi[0] - gi[k1];
            f0 = gi[0] + gi[k1];
            f3 = SQRT2 * gi[k3];
            f2 = SQRT2 * gi[k2];
            gi[k2] = f0 - f2;
            gi[0] = f0 + f2;
            gi[k3] = f1 - f3;
            gi[k1] = f1 + f3;
            gi += k4;
            fi += k4;
        } while (fi < fn);
        c1 = tri[0];
        s1 = tri[1];
        for (i = 1; i < kx; i++) {
            FLOAT   c2, s2, s1_2 = s1 + s1;
            float32x4_t v_c1;
            float32x4_t v_s1;
            float32x4_t v_c2;
            float32x4_t v_s2;
            c2 = 1 - s1_2 * s1;
            s2 = s1_2 * c1;
            fi = fz + i;
            gi = fz + k1 - i;
            v_c1 = neon_setr4(-c1, c1, c1, c1);
            v_s1 = neon_setr4(s1, -s1, s1, s1);
            v_c2 = neon_setr4(c2, c2, -c2, -c2);
            v_s2 = vdupq_n_f32(s2);
            do {
                float32x4_t p, q, r;

                q = neon_setr4(fi[k1], fi[k3], gi[k1], gi[k3]);
                p = vmulq_f32(v_s2, q);
                q = vmulq_f32(v_c2, q);
                q = vextq_f32(q, q, 2);
                p = vaddq_f32(p, q);

                r = neon_setr4(gi[0], gi[k2], fi[0], fi[k2]);
                q = vsubq_f32(r, p);
                r = vaddq_f32(r, p);
                p = neon_setr4(vgetq_lane_f32(q, 0), vgetq_lane_f32(r, 0),
                               vgetq_lane_f32(q, 2), vgetq_lane_f32(r, 2));
                q = neon_setr4(vgetq_lane_f32(q, 1), vgetq_lane_f32(q, 3),
                               vgetq_lane_f32(r, 1), vgetq_lane_f32(r, 3));
                r = vmulq_f32(v_c1, q);
                q = vmulq_f32(v_s1, q);
                q = neon_reverse4(q);
                q = vaddq_f32(q, r);

                store4_neon(vsubq_f32(p, q), &gi[k3], &gi[k2], &fi[k3], &fi[k2]);
                store4_neon(vaddq_f32(p, q), &gi[k1], &gi[0], &fi[k1], &fi[0]);

                gi += k4;
                fi += k4;
            } while (fi < fn);
            c2 = c1;
            c1 = c2 * tri[0] - s1 * tri[1];
            s1 = c2 * tri[1] + s1 * tri[0];
        }
        tri += 2;
    } while (k4 < n);
}

#endif /* LAME_HAVE_AARCH64_NEON_INTRINSICS */


#if LAME_HAVE_WASM_SIMD_INTRINSICS

#include <wasm_simd128.h>

void
init_xrpow_core_wasm(gr_info * const cod_info, FLOAT xrpow[576], int max_nz, FLOAT * sum)
{
    int     i;
    float   tmp_max;
    float   tmp_sum;
    int     upper = max_nz + 1;
    int     upper4 = (upper / 4) * 4;
    v128_t const fabs_mask = wasm_i32x4_splat(0x7fffffff);
    v128_t vec_sum = wasm_f32x4_splat(0.0f);
    v128_t vec_xrpow_max = wasm_f32x4_splat(0.0f);

    for (i = 0; i < upper4; i += 4) {
        v128_t vec_tmp = wasm_v128_load(&cod_info->xr[i]);
        vec_tmp = wasm_v128_and(vec_tmp, fabs_mask);
        vec_sum = wasm_f32x4_add(vec_sum, vec_tmp);
        vec_tmp = wasm_f32x4_sqrt(wasm_f32x4_mul(vec_tmp, wasm_f32x4_sqrt(vec_tmp)));
        vec_xrpow_max = wasm_f32x4_max(vec_xrpow_max, vec_tmp);
        wasm_v128_store(&xrpow[i], vec_tmp);
    }

    tmp_sum = wasm_f32x4_extract_lane(vec_sum, 0)
            + wasm_f32x4_extract_lane(vec_sum, 1)
            + wasm_f32x4_extract_lane(vec_sum, 2)
            + wasm_f32x4_extract_lane(vec_sum, 3);
    {
        float const ma = Max(wasm_f32x4_extract_lane(vec_xrpow_max, 0),
                             wasm_f32x4_extract_lane(vec_xrpow_max, 1));
        float const mb = Max(wasm_f32x4_extract_lane(vec_xrpow_max, 2),
                             wasm_f32x4_extract_lane(vec_xrpow_max, 3));
        tmp_max = Max(ma, mb);
    }
    for (i = upper4; i < upper; ++i) {
        float const tmp = fabs(cod_info->xr[i]);
        float const xp = sqrt(tmp * sqrt(tmp));
        tmp_sum += tmp;
        xrpow[i] = xp;
        if (xp > tmp_max)
            tmp_max = xp;
    }
    cod_info->xrpow_max = tmp_max;
    *sum = tmp_sum;
}

#endif /* LAME_HAVE_WASM_SIMD_INTRINSICS */
