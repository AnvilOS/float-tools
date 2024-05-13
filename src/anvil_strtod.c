
#include <stdlib.h>
#include <ctype.h>
#include <strings.h>

#include "ieee754.h"
#include "xint.h"
#include "pow5.h"

static const int n = 53;
static const int p = 64;

static const uint64_t two_to_n = 1LL << n;                // 2^53
static const uint64_t two_to_n_minus_1 = 1LL << (n-1);

static const int MAX_DIG = 19;

enum pfp_res parse_fp_num(const char *restrict nptr, char **restrict endptr, struct _Anvil_float *estimate_s);
static int algoritm_r(uint64_t significand, const char *pnum, int total_dig, int e, struct _Anvil_float *z0);
static int prev_float(struct _Anvil_float *z);
static int next_float(struct _Anvil_float *z);

#define LONGEST_DOUBLE long double

static const LONGEST_DOUBLE pos_exp[] =
{
    1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7,
    1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15,
    1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22, 1e23,
    1e24, 1e25, 1e26, 1e27,
};

static const LONGEST_DOUBLE bin_exp[] =
{
    1e16L, 1e32L, 1e64L, 1e128L, 1e256L, 1e512L, 1e1024L, 1e2048L, 1e4096L
};

static const double dbl_pos_exp[] =
{
    1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7,
    1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15,
    1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22,
};

enum pfp_res
{
    PFP_ERROR,
    PFP_ZERO,
    PFP_INF,
    PFP_NAN,
    PFP_DEC_EXACT,
    PFP_DEC,
};

double _Anvil_strtod(const char *restrict nptr, char **restrict endptr)
{
    struct _Anvil_float result;
    result.mant_bits = 53;
    result.exp_bits = 10;
    result.exp = 0;
    enum pfp_res pfp_res = parse_fp_num(nptr, endptr, &result);
    
    union double_bits ret;
    switch (pfp_res)
    {
        case PFP_ERROR: return 0.0;
        case PFP_ZERO: return result.neg ? -0.0 : 0.0;
        case PFP_INF:  ret.bits = result.neg ? 0xfff0000000000000 : 0x7ff0000000000000; return ret.dbl;
        case PFP_NAN:  ret.bits = result.neg ? 0xfff0000000000001 : 0x7ff0000000000001; return ret.dbl;
        case PFP_DEC_EXACT:
            if (result.exp >= 0)
            {
                return result.neg ? -(result.mant * dbl_pos_exp[result.exp]) : result.mant * dbl_pos_exp[result.exp];
            }
            else if (result.exp < 0)
            {
                return result.neg ? -(result.mant / dbl_pos_exp[-result.exp]) : result.mant / dbl_pos_exp[-result.exp];
            }
        case PFP_DEC:
            ret.bits = result.mant & 0xfffffffffffff;
            ret.bits |= (uint64_t)result.exp << 52;
            return result.neg ? -ret.dbl : ret.dbl;
    }
}

float _Anvil_strtof(const char *restrict nptr, char **restrict endptr)
{
    struct _Anvil_float result;
    result.mant_bits = 24;
    result.exp_bits = 7;
    result.exp = 0;
    enum pfp_res pfp_res = parse_fp_num(nptr, endptr, &result);
    
    union double_bits ret;
    switch (pfp_res)
    {
        case PFP_ERROR: return 0.0;
        case PFP_ZERO: return result.neg ? -0.0 : 0.0;
        case PFP_INF:  ret.bits = result.neg ? 0xfff0000000000000 : 0x7ff0000000000000; return ret.dbl;
        case PFP_NAN:  ret.bits = result.neg ? 0xfff0000000000001 : 0x7ff0000000000001; return ret.dbl;
        case PFP_DEC_EXACT:
            if (result.exp >= 0)
            {
                return result.neg ? -(result.mant * dbl_pos_exp[result.exp]) : result.mant * dbl_pos_exp[result.exp];
            }
            else if (result.exp < 0)
            {
                return result.neg ? -(result.mant / dbl_pos_exp[-result.exp]) : result.mant / dbl_pos_exp[-result.exp];
            }
        case PFP_DEC:
            ret.bits = result.mant;
            ret.bits |= (uint64_t)result.exp << 23;
            return result.neg ? -ret.dbl : ret.dbl;
    }
}

enum pfp_res parse_fp_num(const char *restrict nptr, char **restrict endptr, struct _Anvil_float *result)
{
    const char *str = nptr;
    int base;

    // Skip the whitespace
    while (isspace(*str))
    {
        ++str;
    }

    // Look for the sign next
    switch (*str)
    {
        case '-':
            ++str;
            result->neg = 1;
            break;
        case '+':
            ++str;
            /* Fall through */
        default:
            result->neg = 0;
            break;
    }

    switch (*str)
    {
        case 'I':
        case 'i':
            if (!strcasecmp(str, "inf") || !strcasecmp(str, "infinity"))
            {
                return PFP_INF;
            }
            return PFP_ERROR;
            
        case 'N':
        case 'n':
            if (!strncasecmp(str, "nan", 3))
            {
                if (str[3] == '\0')
                {
                    return PFP_NAN;
                }
                else if (str[3] == '(')
                {
                    return PFP_NAN;
                }
            }
            return PFP_ERROR;

        default:
            break;
    }
    
    // The base can be 10 or 16
    base = 10;
    if (*str == '0')
    {
        if (*(str+1) == 'x' || *(str+1) == 'X')
        {
            base = 16;
            str += 2;;
        }
        // Todo: What if the next char isn't a hex digit
    }

    uint64_t significand = 0;
    int exponent = 0;
    int nz_b = 0;
    int ndig_b = 0;
    int nz_a = 0;
    int ndig_a = 0;
    
    // Skip spaces
    while (*str == '0')
    {
        ++nz_b;
        ++str;
    }

    const char *pnum = str;

    // Gather up the digits before the radix
    while (*str >= '0' && *str <= '9')
    {
        if (ndig_b < MAX_DIG)
        {
            significand = significand * base + (*str - '0');
        }
        ++ndig_b;
        ++str;
    }
    
    // Did we stop because of the radix character ?
    // XXX: this should check for the radix char
    if (*str == '.')
    {
        ++str;
        if (ndig_b == 0)
        {
            // Count up more leading zeros
            while (*str == '0')
            {
                ++nz_a;
                ++str;
            }
            pnum = str;
        }
        while (*str >= '0' && *str <= '9')
        {
            if (ndig_b + ndig_a < MAX_DIG)
            {
                significand = significand * base + (*str - '0');
            }
            ++ndig_a;
            ++str;
        }
    }

    if (*str == 'e' || *str == 'E')
    {
        int exp_neg = 0;
        ++str;
        // Look for the sign first
        switch (*str)
        {
            case '-':
                ++str;
                exp_neg = 1;
                break;
            case '+':
                ++str;
                // Fall through
            default:
                exp_neg = 0;
                break;
        }

        while (*str >= '0' && *str <= '9')
        {
            if (exponent < 50000)
            {
                exponent = exponent * base + (*str - '0');
            }
            ++str;
        }
        if (exp_neg)
        {
            exponent = -exponent;
        }
    }

    // The full exponent is the exponent that would need to be applied to the
    // entire significand to create our number. We don't use it yet because
    // we are currently dealing with a signicand that has potentially been
    // truncated to MAX_DIG digits. Let's save it though
    int full_exponent = exponent - nz_a - ndig_a;
    
    // XXX: adjust these cut-off points to account for digits in the significand
    if (significand == 0)
    {
        return PFP_ZERO;
    }
    if (full_exponent + ndig_b + ndig_a - 1 > 308)
    {
        return PFP_INF;
    }
    if (full_exponent + ndig_b + ndig_a - 1 < -324)
    {
        return PFP_ZERO;
    }

    // Here's the exponent for the truncated significand
    exponent = exponent - nz_a - ndig_a;
    if (ndig_b + ndig_a > MAX_DIG)
    {
        // Not all the digits fitted in the significand so account for
        // the discarded number
        exponent = exponent + (ndig_b + ndig_a - MAX_DIG);
    }
    
    // In some cases the caller can calculate the closest fp number
    if ((significand < two_to_n) && (abs(exponent) <= 22))
    {
        result->mant = significand;
        result->exp = exponent;
        return PFP_DEC_EXACT;
    }

    // Let's make an estimate using the first MAX_DIG digits
    LONGEST_DOUBLE estimate = significand;
    if (exponent > 0)
    {
        if (exponent < sizeof(pos_exp)/sizeof(pos_exp[0]))
        {
            estimate *= pos_exp[exponent];
        }
        else
        {
            int mask = exponent;
            estimate *= pos_exp[(mask & 0xf)];
            mask >>= 4;
            int ndx = 0;
            while (mask)
            {
                if (mask & 1)
                {
                    estimate *= bin_exp[ndx];
                }
                ++ndx;
                mask >>= 1;
            }
        }
    }
    else if (exponent < 0)
    {
        if (exponent < sizeof(pos_exp)/sizeof(pos_exp[0]))
        {
            estimate /= pos_exp[exponent];
        }
        else
        {
            int mask = -exponent;
            estimate /= pos_exp[(mask & 0xf)];
            mask >>= 4;
            int ndx = 0;
            while (mask)
            {
                if (mask & 1)
                {
                    estimate /= bin_exp[ndx];
                }
                ++ndx;
                mask >>= 1;
            }
        }
    }
    
    long_double_to_struct(result, estimate);

    algoritm_r(significand, pnum, ndig_b + ndig_a, full_exponent, result);

    return PFP_DEC;
}

int algoritm_r(uint64_t significand, const char *pnum, int total_dig, int e, struct _Anvil_float *z0)
{
    xint_t X;
    xint_t Y;
    xint_t TEMP;
    xint_t f;
    
    int k_offs = (1 << z0->exp_bits) - 1 + z0->mant_bits - 1;

    int k = z0->exp;
    if (k == 0)
    {
        ++k;
    }
    k -= k_offs;
    
    xint_init(f, 30);
    xint_init(X, 30);
    xint_init(Y, 30);
    xint_init(TEMP, 30);

    if (total_dig <= MAX_DIG)
    {
        xint_assign_uint64(f, significand);
    }
    else
    {
        const char *str = pnum;
        while ((*str >= '0' && *str <= '9') || *str == '.')
        {
            if (*str != '.')
            {
                xint_mul_1_add_1(f, f, 10, *str - '0');
            }
            ++str;
        }
    }
    
    if (z0->mant == 0 && z0->exp == 0)
    {
        next_float(z0);
    }
    
    while (1)
    {
        uint64_t m = z0->mant;
        int k = z0->exp;
        
        if (k == 0)
        {
            ++k;
        }
        k -= k_offs;

        // Find x and y such that
        // x / y = (f * 10^e) / (m * 2^k)
        //       = f * 5^e * 2^e / m / 2^k
        //       = f * 5^e * 2^(e-k)  / m
        
        // If e and/or e-k is negative we will move that term to the
        // bottom of the ratio and negate it.
        
        xint_copy(X, f);
        xint_assign_uint64(Y, m);

        if (e > 0)
        {
            xint_mul_5exp(X, e);
        }
        else
        {
            xint_mul_5exp(Y, -e);
        }
        
        if (e > k)
        {
            xint_lshift(X, X, e-k);
        }
        else
        {
            xint_lshift(Y, Y, k-e);
        }

        // E = m * (x-y) / y
        // If E < 1/2
        // y/2 < m * (x-y)
        // y < 2 * m * (x-y)
        //
        // Let D = X - Y
        int D_sign = xint_suba(TEMP, X, Y);
        // Now TEMP is D

        // D2 = 2 * m * abs(d)
        xint_mul_2(TEMP, TEMP, 2 * m);
        // Now TEMP is D2

        // Compare D2 with y
        int cmp_d2_y = xint_cmp(TEMP, Y);
        
        if (cmp_d2_y < 0)
        {
            xint_mul_1(TEMP, TEMP, 2);
            int cmp_2d2_y = xint_cmp(TEMP, Y);
            // D2 < y
            if ((k != -1074) && (m == two_to_n_minus_1) && (D_sign < 0) && (cmp_2d2_y > 0))
            {
                if (prev_float(z0) == -1)
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        else if (cmp_d2_y == 0)
        {
            // If m is even
            if ((m % 2) == 0)
            {
                if ((m == two_to_n_minus_1) && (D_sign < 0))
                {
                    if (prev_float(z0) == -1)
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
            else
            {
                if (D_sign < 0)
                {
                    prev_float(z0);
                    break;
                }
                if (D_sign > 0)
                {
                    next_float(z0);
                    break;
                }
            }
        }
        else
        {
            if (D_sign < 0)
            {
                if (prev_float(z0) == -1)
                {
                    break;
                }
            }
            if (D_sign > 0)
            {
                if (next_float(z0) == -1)
                {
                    break;
                }
            }
        }
    }
    
    xint_delete(f);
    xint_delete(X);
    xint_delete(Y);
    xint_delete(TEMP);

    return 0;
}

int prev_float(struct _Anvil_float *z)
{
    if (z->exp == 0)
    {
        // Sub-normal
        if (z->mant)
        {
            --z->mant;
        }
        if (z->mant == 0)
        {
            return -1;
        }
    }
    else if (z->exp == 0x7ff)
    {
        // NaN
        z->mant = (1ULL << z->mant_bits) - 1;
        z->exp = 0x7fe;
    }
    else
    {
        if (z->mant == 1ULL << 52)
        {
            if (z->exp == 1)
            {
                z->mant = (1ULL << 52) - 1;
                z->exp = 0;
            }
            else
            {
                z->mant = (1ULL << z->mant_bits) - 1;
                --z->exp;
            }
        }
        else
        {
            --z->mant;
        }
    }
    return 0;
}

int next_float(struct _Anvil_float *z)
{
    if (z->exp == 0)
    {
        // Sub-normal
        if (z->mant < (1ULL << (z->mant_bits - 1)) - 1)
        {
            ++z->mant;
        }
        else
        {
            z->exp = 1;
            z->mant = 1ULL << (z->mant_bits - 1);
        }
    }
    else if (z->exp == 0x7ff)
    {
        // NaN
        return -1;
    }
    else
    {
        ++z->mant;
        if (z->mant == 1ULL << z->mant_bits)
        {
            z->mant = 1ULL << (z->mant_bits - 1);
            ++z->exp;
        }
    }
    return 0;
}
