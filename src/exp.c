/*
 * Copyright Supranational LLC
 * Licensed under the Apache License, Version 2.0, see LICENSE for details.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "vect.h"
#include "fields.h"

static void reciprocal_fp(vec384 out, const vec384 inp)
{
    vec768 temp;

    ct_inverse_mod_383(temp, inp, BLS12_381_P);
    redc_mont_384(out, temp, BLS12_381_P, p0);
    mul_mont_384(out, out, BLS12_381_RR, BLS12_381_P, p0);
}

void blst_fp_inverse(vec384 out, const vec384 inp)
{   reciprocal_fp(out, inp);   }

/*
 * |out| = |inp|^|pow|, small footprint, public exponent
 */
static void exp_mont_384(vec384 out, const vec384 inp, const byte *pow,
                         size_t pow_bits, const vec384 p, limb_t n0)
{
#if 1
    vec384 ret;

    vec_copy(ret, inp, sizeof(ret));  /* ret = inp^1 */
    --pow_bits; /* most significant bit is set, skip over */
    while (pow_bits--) {
        sqr_mont_384(ret, ret, p, n0);
        if (is_bit_set(pow, pow_bits))
            mul_mont_384(ret, ret, inp, p, n0);
    }
    vec_copy(out, ret, sizeof(ret));  /* out = ret */
#else
    unsigned int i;
    vec384 sqr;

    vec_copy(sqr, inp, sizeof(sqr));
    for (i = 0; !is_bit_set(pow, i++);)
        sqr_mont_384(sqr, sqr, sqr, p, n0);
    vec_copy(out, sqr, sizeof(sqr));
    for (; i < pow_bits; i++) {
        sqr_mont_384(sqr, sqr, sqr, p, n0);
        if (is_bit_set(pow, i))
            mul_mont_384(out, out, sqr, p, n0);
    }
#endif
}

#ifdef __OPTIMIZE_SIZE__
static void recip_sqrt_fp_3mod4(vec384 out, const vec384 inp)
{
    static const byte BLS_12_381_P_minus_3_div_4[] = {
        TO_BYTES(0xee7fbfffffffeaaa), TO_BYTES(0x07aaffffac54ffff),
        TO_BYTES(0xd9cc34a83dac3d89), TO_BYTES(0xd91dd2e13ce144af),
        TO_BYTES(0x92c6e9ed90d2eb35), TO_BYTES(0x0680447a8e5ff9a6)
    };

    exp_mont_384(out, inp, BLS_12_381_P_minus_3_div_4, 379, BLS12_381_P, p0);
}
#else
# if 1
/*
 * "383"-bit variant omits full reductions at the ends of squarings,
 * which results in up to ~15% improvement. [One can improve further
 * by omitting full reductions even after multiplications and
 * performing final reduction at the very end of the chain.]
 */
static inline void sqr_n_mul_fp(vec384 out, const vec384 a, size_t count,
                                const vec384 b)
{   sqr_n_mul_mont_383(out, a, count, BLS12_381_P, p0, b);   }
# else
static void sqr_n_mul_fp(vec384 out, const vec384 a, size_t count,
                         const vec384 b)
{
    while(count--) {
        sqr_fp(out, a);
        a = out;
    }
    mul_fp(out, out, b);
}
# endif

# define sqr(ret,a)		sqr_fp(ret,a)
# define mul(ret,a,b)		mul_fp(ret,a,b)
# define sqr_n_mul(ret,a,n,b)	sqr_n_mul_fp(ret,a,n,b)

# include "sqrt-addchain.h"
static void recip_sqrt_fp_3mod4(vec384 out, const vec384 inp)
{
    RECIP_SQRT_MOD_BLS12_381_P(out, inp, vec384);
}
# undef RECIP_SQRT_MOD_BLS12_381_P

# undef sqr_n_mul
# undef sqr
# undef mul
#endif

static bool_t recip_sqrt_fp(vec384 out, const vec384 inp)
{
    vec384 t0, t1;
    bool_t ret;

    recip_sqrt_fp_3mod4(t0, inp);

    mul_fp(t1, t0, inp);
    sqr_fp(t1, t1);
    ret = vec_is_equal(t1, inp, sizeof(t1));
    vec_copy(out, t0, sizeof(t0));

    return ret;
}

static bool_t sqrt_fp(vec384 out, const vec384 inp)
{
    vec384 t0, t1;
    bool_t ret;

    recip_sqrt_fp_3mod4(t0, inp);

    mul_fp(t0, t0, inp);
    sqr_fp(t1, t0);
    ret = vec_is_equal(t1, inp, sizeof(t1));
    vec_copy(out, t0, sizeof(t0));

    return ret;
}

int blst_fp_sqrt(vec384 out, const vec384 inp)
{   return (int)sqrt_fp(out, inp);   }
