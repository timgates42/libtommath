/* LibTomMath, multiple-precision integer library -- Tom St Denis */
/* SPDX-License-Identifier: Unlicense */

#ifndef TOMMATH_PRIVATE_H_
#define TOMMATH_PRIVATE_H_

#include "tommath.h"
#include "tommath_class.h"
#include <limits.h>

/*
 * Private symbols
 * ---------------
 *
 * On Unix symbols can be marked as hidden if libtommath is compiled
 * as a shared object. By default, symbols are visible.
 * On Win32 a .def file must be used to specify the exported symbols.
 */
#if defined(__GNUC__) && __GNUC__ >= 4 && !defined(_WIN32) && !defined(__CYGWIN__)
#   define MP_PRIVATE __attribute__ ((visibility ("hidden")))
#else
#   define MP_PRIVATE
#endif

/* Hardening libtommath
 * --------------------
 *
 * By default memory is zeroed before calling
 * MP_FREE to avoid leaking data. This is good
 * practice in cryptographical applications.
 *
 * Note however that memory allocators used
 * in cryptographical applications can often
 * be configured by itself to clear memory,
 * rendering the clearing in tommath unnecessary.
 * See for example https://github.com/GrapheneOS/hardened_malloc
 * and the option CONFIG_ZERO_ON_FREE.
 *
 * Furthermore there are applications which
 * value performance more and want this
 * feature to be disabled. For such applications
 * define MP_NO_ZERO_ON_FREE during compilation.
 */
#ifdef MP_NO_ZERO_ON_FREE
#  define MP_FREE_BUF(mem, size)   MP_FREE((mem), (size))
#  define MP_FREE_DIGS(mem, digits) MP_FREE((mem), sizeof (mp_digit) * (size_t)(digits))
#else
#  define MP_FREE_BUF(mem, size)                        \
do {                                                    \
   size_t fs_ = (size);                                 \
   void* fm_ = (mem);                                   \
   if (fm_ != NULL) {                                   \
      s_mp_zero_buf(fm_, fs_);                          \
      MP_FREE(fm_, fs_);                                \
   }                                                    \
} while (0)
#  define MP_FREE_DIGS(mem, digits)                     \
do {                                                    \
   int fd_ = (digits);                                  \
   mp_digit* fm_ = (mem);                               \
   if (fm_ != NULL) {                                   \
      s_mp_zero_digs(fm_, fd_);                         \
      MP_FREE(fm_, sizeof (mp_digit) * (size_t)fd_);    \
   }                                                    \
} while (0)
#endif

/* Tunable cutoffs
 * ---------------
 *
 *  - In the default settings, a cutoff X can be modified at runtime
 *    by adjusting the corresponding X_CUTOFF variable.
 *
 *  - Tunability of the library can be disabled at compile time
 *    by defining the MP_FIXED_CUTOFFS macro.
 *
 *  - There is an additional file tommath_cutoffs.h, which defines
 *    the default cutoffs. These can be adjusted manually or by the
 *    autotuner.
 *
 */

#ifdef MP_FIXED_CUTOFFS
#  include "tommath_cutoffs.h"
#  define MP_MUL_KARATSUBA_CUTOFF MP_DEFAULT_MUL_KARATSUBA_CUTOFF
#  define MP_SQR_KARATSUBA_CUTOFF MP_DEFAULT_SQR_KARATSUBA_CUTOFF
#  define MP_MUL_TOOM_CUTOFF      MP_DEFAULT_MUL_TOOM_CUTOFF
#  define MP_SQR_TOOM_CUTOFF      MP_DEFAULT_SQR_TOOM_CUTOFF
#endif

/* define heap macros */
#ifndef MP_MALLOC
/* default to libc stuff */
#   include <stdlib.h>
#   define MP_MALLOC(size)                   malloc(size)
#   define MP_REALLOC(mem, oldsize, newsize) realloc((mem), (newsize))
#   define MP_CALLOC(nmemb, size)            calloc((nmemb), (size))
#   define MP_FREE(mem, size)                free(mem)
#else
/* prototypes for our heap functions */
extern void *MP_MALLOC(size_t size);
extern void *MP_REALLOC(void *mem, size_t oldsize, size_t newsize);
extern void *MP_CALLOC(size_t nmemb, size_t size);
extern void MP_FREE(void *mem, size_t size);
#endif

/* feature detection macro */
#ifdef _MSC_VER
/* Prevent false positive: not enough arguments for function-like macro invocation */
#pragma warning(disable: 4003)
#endif
#define MP_STRINGIZE(x)  MP__STRINGIZE(x)
#define MP__STRINGIZE(x) ""#x""
#define MP_HAS(x)        (sizeof(MP_STRINGIZE(x##_C)) == 1u)

#define MP_MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MP_MAX(x, y) (((x) > (y)) ? (x) : (y))

#define MP_TOUPPER(c) ((((c) >= 'a') && ((c) <= 'z')) ? (((c) + 'A') - 'a') : (c))

#define MP_EXCH(t, a, b) do { t _c = a; a = b; b = _c; } while (0)

/* Static assertion */
#define MP_STATIC_ASSERT(msg, cond) typedef char mp_static_assert_##msg[(cond) ? 1 : -1];

#define MP_SIZEOF_BITS(type)    ((size_t)CHAR_BIT * sizeof(type))

#define MP_MAX_COMBA            (int)(1uL << (MP_SIZEOF_BITS(mp_word) - (2u * (size_t)MP_DIGIT_BIT)))
#define MP_WARRAY               (int)(1uL << ((MP_SIZEOF_BITS(mp_word) - (2u * (size_t)MP_DIGIT_BIT)) + 1u))

#if defined(MP_16BIT)
typedef uint32_t mp_word;
#elif defined(MP_64BIT)
typedef unsigned long mp_word __attribute__((mode(TI)));
#else
typedef uint64_t mp_word;
#endif

MP_STATIC_ASSERT(correct_word_size, sizeof(mp_word) == (2u * sizeof(mp_digit)))

/* default precision */
#ifndef MP_PREC
#   ifndef MP_LOW_MEM
#      define MP_PREC 32        /* default digits of precision */
#   else
#      define MP_PREC 8         /* default digits of precision */
#   endif
#endif

/* Minimum number of available digits in mp_int, MP_PREC >= MP_MIN_PREC
 * - Must be at least 3 for s_mp_div_school.
 * - Must be large enough such that uint64_t can be stored in mp_int without growing
 */
#define MP_MIN_PREC MP_MAX(3, (((int)MP_SIZEOF_BITS(long long) + MP_DIGIT_BIT) - 1) / MP_DIGIT_BIT)
MP_STATIC_ASSERT(prec_geq_min_prec, MP_PREC >= MP_MIN_PREC)

/* random number source */
extern MP_PRIVATE mp_err(*s_mp_rand_source)(void *out, size_t size);

/* lowlevel functions, do not call! */
MP_PRIVATE bool s_mp_get_bit(const mp_int *a, int b);
MP_PRIVATE mp_err s_mp_add(const mp_int *a, const mp_int *b, mp_int *c) MP_WUR;
MP_PRIVATE mp_err s_mp_sub(const mp_int *a, const mp_int *b, mp_int *c) MP_WUR;
MP_PRIVATE mp_err s_mp_mul_comba(const mp_int *a, const mp_int *b, mp_int *c, int digs) MP_WUR;
MP_PRIVATE mp_err s_mp_mul(const mp_int *a, const mp_int *b, mp_int *c, int digs) MP_WUR;
MP_PRIVATE mp_err s_mp_mul_high_comba(const mp_int *a, const mp_int *b, mp_int *c, int digs) MP_WUR;
MP_PRIVATE mp_err s_mp_mul_high(const mp_int *a, const mp_int *b, mp_int *c, int digs) MP_WUR;
MP_PRIVATE mp_err s_mp_sqr_comba(const mp_int *a, mp_int *b) MP_WUR;
MP_PRIVATE mp_err s_mp_sqr(const mp_int *a, mp_int *b) MP_WUR;
MP_PRIVATE mp_err s_mp_mul_balance(const mp_int *a, const mp_int *b, mp_int *c) MP_WUR;
MP_PRIVATE mp_err s_mp_mul_karatsuba(const mp_int *a, const mp_int *b, mp_int *c) MP_WUR;
MP_PRIVATE mp_err s_mp_mul_toom(const mp_int *a, const mp_int *b, mp_int *c) MP_WUR;
MP_PRIVATE mp_err s_mp_sqr_karatsuba(const mp_int *a, mp_int *b) MP_WUR;
MP_PRIVATE mp_err s_mp_sqr_toom(const mp_int *a, mp_int *b) MP_WUR;
MP_PRIVATE mp_err s_mp_invmod_odd(const mp_int *a, const mp_int *b, mp_int *c) MP_WUR;
MP_PRIVATE mp_err s_mp_invmod(const mp_int *a, const mp_int *b, mp_int *c) MP_WUR;
MP_PRIVATE mp_err s_mp_montgomery_reduce_comba(mp_int *x, const mp_int *n, mp_digit rho) MP_WUR;
MP_PRIVATE mp_err s_mp_exptmod_fast(const mp_int *G, const mp_int *X, const mp_int *P, mp_int *Y, int redmode) MP_WUR;
MP_PRIVATE mp_err s_mp_exptmod(const mp_int *G, const mp_int *X, const mp_int *P, mp_int *Y, int redmode) MP_WUR;
MP_PRIVATE mp_err s_mp_rand_platform(void *p, size_t n) MP_WUR;
MP_PRIVATE mp_err s_mp_prime_is_divisible(const mp_int *a, bool *result);
MP_PRIVATE mp_digit s_mp_log_d(mp_digit base, mp_digit n);
MP_PRIVATE mp_err s_mp_log(const mp_int *a, uint32_t base, uint32_t *c);
MP_PRIVATE uint32_t s_mp_log_pow2(const mp_int *a, uint32_t base);
MP_PRIVATE mp_err s_mp_div_recursive(const mp_int *a, const mp_int *b, mp_int *q, mp_int *r);
MP_PRIVATE mp_err s_mp_div_school(const mp_int *a, const mp_int *b, mp_int *c, mp_int *d);
MP_PRIVATE mp_err s_mp_div_small(const mp_int *a, const mp_int *b, mp_int *c, mp_int *d);
MP_PRIVATE void s_mp_zero_buf(void *mem, size_t size);
MP_PRIVATE void s_mp_zero_digs(mp_digit *d, int digits);
MP_PRIVATE void s_mp_copy_digs(mp_digit *d, const mp_digit *s, int digits);

/* TODO: jenkins prng is not thread safe as of now */
MP_PRIVATE mp_err s_mp_rand_jenkins(void *p, size_t n) MP_WUR;
MP_PRIVATE void s_mp_rand_jenkins_init(uint64_t seed);

#define MP_RMAP_REVERSE_SIZE 80u
extern MP_PRIVATE const char s_mp_rmap[];
extern MP_PRIVATE const uint8_t s_mp_rmap_reverse[];
extern MP_PRIVATE const mp_digit s_mp_prime_tab[];

/* number of primes */
#define MP_PRIME_TAB_SIZE 256

#define MP_GET_ENDIANNESS(x) \
   do{\
      int16_t n = 0x1;                                          \
      char *p = (char *)&n;                                     \
      x = (p[0] == '\x01') ? MP_LITTLE_ENDIAN : MP_BIG_ENDIAN;  \
   } while (0)

/* code-generating macros */
#define MP_SET_UNSIGNED(name, type)                                                    \
    void name(mp_int * a, type b)                                                      \
    {                                                                                  \
        int i = 0;                                                                     \
        while (b != 0u) {                                                              \
            a->dp[i++] = ((mp_digit)b & MP_MASK);                                      \
            if (MP_SIZEOF_BITS(type) <= MP_DIGIT_BIT) { break; }                       \
            b >>= ((MP_SIZEOF_BITS(type) <= MP_DIGIT_BIT) ? 0 : MP_DIGIT_BIT);         \
        }                                                                              \
        a->used = i;                                                                   \
        a->sign = MP_ZPOS;                                                             \
        s_mp_zero_digs(a->dp + a->used, a->alloc - a->used);                         \
    }

#define MP_SET_SIGNED(name, uname, type, utype)          \
    void name(mp_int * a, type b)                        \
    {                                                    \
        uname(a, (b < 0) ? -(utype)b : (utype)b);        \
        if (b < 0) { a->sign = MP_NEG; }                 \
    }

#define MP_INIT_INT(name , set, type)                    \
    mp_err name(mp_int * a, type b)                      \
    {                                                    \
        mp_err err;                                      \
        if ((err = mp_init(a)) != MP_OKAY) {             \
            return err;                                  \
        }                                                \
        set(a, b);                                       \
        return MP_OKAY;                                  \
    }

#define MP_GET_MAG(name, type)                                                         \
    type name(const mp_int* a)                                                         \
    {                                                                                  \
        int i = MP_MIN(a->used, (int)((MP_SIZEOF_BITS(type) + MP_DIGIT_BIT - 1) / MP_DIGIT_BIT)); \
        type res = 0u;                                                                 \
        while (i --> 0) {                                                              \
            res <<= ((MP_SIZEOF_BITS(type) <= MP_DIGIT_BIT) ? 0 : MP_DIGIT_BIT);       \
            res |= (type)a->dp[i];                                                     \
            if (MP_SIZEOF_BITS(type) <= MP_DIGIT_BIT) { break; }                       \
        }                                                                              \
        return res;                                                                    \
    }

#define MP_GET_SIGNED(name, mag, type, utype)                 \
    type name(const mp_int* a)                                \
    {                                                         \
        utype res = mag(a);                                   \
        return (a->sign == MP_NEG) ? (type)-res : (type)res;  \
    }

#endif
