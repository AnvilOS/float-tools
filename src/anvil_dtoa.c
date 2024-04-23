
#include "anvil_dtoa.h"
#include "ieee754.h"
#include "xint.h"

#include <string.h>
#include <stdlib.h>

#define MAX(a, b) (a>b?a:b)

static char *dragon4(uint64_t f, int e, int p, int mode, int ndigits, int *decpt);

char *anvil_dtoa(double dd, int mode, int ndigits, int *decpt, int *sign, char **rve)
{
    const int p = 52;
    uint64_t f;
    int e;
    split_double(dd, sign, &f, &e);
    return dragon4(f, e, p, mode, ndigits, decpt);
}

char *anvil_ldtoa(long double dd, int mode, int ndigits, int *decpt, int *sign, char **rve)
{
    return NULL;
}

static char *dragon4(uint64_t f, int e, int p, int mode, int cutoff_place, int *decpt)
{
    xint_t x2R = XINT_INIT_VAL;
    xint_t x2S = XINT_INIT_VAL;
    xint_t Mm = XINT_INIT_VAL;
    xint_t Mp = XINT_INIT_VAL;
    xint_t U = XINT_INIT_VAL;
    xint_t TMP = XINT_INIT_VAL;

    int round_up = 0;
    int even = 0;
    
    if ((f % 2) == 0)
    {
        if (mode == 0)
        {
            even = 1;
        }
    }
    
    if (f == 0)
    {
        *decpt = 1;
        return strdup("0");
    }
    xint_assign_uint64(x2R, f);
    xint_assign_uint64(x2S, 1);
    xint_assign_uint64(Mm, 1);
    xint_assign_uint64(Mp, 1);
    
    int k = 0;
    
    xint_lshift(x2R, x2R, MAX(e-p, 0));
    xint_lshift(x2S, x2S, MAX(0, -(e-p)));
    xint_lshift(Mm, Mm, MAX(e-p, 0));
    xint_copy(Mp, Mm);
    
    //    fixup(R, S, Mp, Mm, &k, &f, p, mode, &place, &round_up, even);
    if (f == 0x10000000000000ULL)
    {
        xint_lshift(Mp, Mp, 1);
        xint_lshift(x2R, x2R, 1);
        xint_lshift(x2S, x2S, 1);
    }
    k = 0;

    // From now on we will keep 2xr and 2xS for convenience
    xint_lshift(x2S, x2S, 1);
    xint_lshift(x2R, x2R, 1);

    // Load TMP with ceil(S/10) - actually ceil(2xS/10)
    xint_copy(TMP, x2S);
    xword_t rem;
    xint_div_1(TMP, &rem, TMP, 10);
    if (rem)
    {
        xint_adda_1(TMP, TMP, 1);
    }
    // while R < ceil(S/10) - actually 2xR < ceil(2xS/10)
    while (xint_cmp(x2R, TMP) < 0)
    {
        k = k - 1;
        xint_mul_1(x2R, x2R, 10);
        xint_mul_1(Mm, Mm, 10);
        xint_mul_1(Mp, Mp, 10);
    }

    // This time TMP will be 2xR + M+
    while (1)
    {
        xint_adda(TMP, x2R, Mp);
        // while 2xR + M+ >= 2S
        while (round_up || even ? xint_cmp(TMP, x2S) >= 0 : xint_cmp(TMP, x2S) > 0)
        {
            xint_mul_1(x2S, x2S, 10);
            k = k + 1;
        }
        // Adjust Mp and Mm
        switch (mode)
        {
            case 0:
            case 1:
                cutoff_place = k;
                break;

            default:
                break;
        }

        // We need to do this again in case M+ changed
        xint_adda(TMP, x2R, Mp);
        // if 2xR + M+ >= 2S
        if (!(round_up || even ? xint_cmp(TMP, x2S) >= 0 : xint_cmp(TMP, x2S) > 0))
        {
            break;
        }
    }

    int u;
    int low = 0;
    int high = 0;
    
    // XXX : calculate how many chars are needed
    
    char *sstr = malloc(20);
    char *pstr = sstr;
    
    while (1)
    {
        k = k - 1;

        xint_mul_1(x2R, x2R, 10);
        xint_div(U, x2R, x2R, x2S);

        u = U->size ? U->data[0] : 0;
        u += '0';

        xint_mul_1(Mm, Mm, 10);
        xint_mul_1(Mp, Mp, 10);
        
        // low = 2R < Mm
        low = round_up || even ? xint_cmp(x2R, Mm) <= 0 : xint_cmp(x2R, Mm) < 0;
        
        xint_adda(TMP, x2R, Mp);
        // Compare 2R with 2S - Mp (actually compare 2xR + M+ with 2xS
        if (round_up || even)
        {
            high = xint_cmp(TMP, x2S) >= 0;
        }
        else
        {
            high = xint_cmp(TMP, x2S) > 0;
        }
        if (low || high || k == cutoff_place)
        {
            break;
        }
        *pstr++ = u;
    }
    
    if (low && !high)
    {
        *pstr++ = u;
    }
    else if (!low && high)
    {
        *pstr++ = u + 1;
    }
    else
    {
        // Compare 2xR with S - in our case 4xR with 2xS
        xint_lshift(x2R, x2R, 1);
        int cmp = xint_cmp(x2R, x2S);
        if (cmp < 0)
        {
            *pstr++ = u;
        }
        else if (cmp > 0)
        {
            *pstr++ = u + 1;
        }
        else
        {
            if (u & 1)
            {
                *pstr++ = u + 1;
            }
            else
            {
                *pstr++ = u;
            }
        }
    }
    *pstr = 0;
    --pstr;
    // Remove trailing zeroes
    while (pstr > sstr)
    {
        if (*pstr != '0')
        {
            break;
        }
        ++k;
        *pstr = 0;
        --pstr;
    }
  
    *decpt = k + (int)strlen(sstr);

    xint_delete(x2R);
    xint_delete(x2S);
    xint_delete(Mm);
    xint_delete(Mp);
    xint_delete(U);
    xint_delete(TMP);

    return sstr;
}
