
#include "anvil_dtoa.h"
#include "ieee754.h"
#include "xint.h"

#include <string.h>
#include <stdlib.h>

#define MAX(a, b) (a>b?a:b)

char *anvil_dtoa(double dd, int mode, int ndigits, int *decpt, int *sign, char **rve)
{
    const int p = 52;
    uint64_t f;
    int e;
    
    split_double(dd, sign, &f, &e);
    
    xint_t R = XINT_INIT_VAL;
    xint_t S = XINT_INIT_VAL;
    xint_t Mm = XINT_INIT_VAL;
    xint_t Mp = XINT_INIT_VAL;
    xint_t T2S = XINT_INIT_VAL;
    xint_t T2R = XINT_INIT_VAL;
    
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
    xint_assign_uint64(R, f);
    xint_assign_uint64(S, 1);
    xint_assign_uint64(Mm, 1);
    xint_assign_uint64(Mp, 1);
    
    int k = 0;
    
    xint_lshift(R, R, MAX(e-p, 0));
    xint_lshift(S, S, MAX(0, -(e-p)));
    xint_lshift(Mm, Mm, MAX(e-p, 0));
    xint_copy(Mp, Mm);
    
    //    fixup(R, S, Mp, Mm, &k, &f, p, mode, &place, &round_up, even);
    {
        xint_t TMP = XINT_INIT_VAL;
        
        if (f == 0x10000000000000ULL)
        {
            xint_lshift(Mp, Mp, 1);
            xint_lshift(R, R, 1);
            xint_lshift(S, S, 1);
        }
        k = 0;
        
        // Load TMP with ceil(S/10)
        xint_copy(TMP, S);
        xword_t rem;
        xint_div_1(TMP, &rem, TMP, 10);
        if (rem)
        {
            xint_adda_1(TMP, TMP, 1);
        }
        while (xint_cmp(R, TMP) < 0)
        {
            k = k - 1;
            xint_mul_1(R, R, 10);
            xint_mul_1(Mm, Mm, 10);
            xint_mul_1(Mp, Mp, 10);
        }
        
        // This time TMP will be 2R + M+ and we will load 2S into S
        while (1)
        {
            xint_mul_1(TMP, R, 2);
            xint_adda(TMP, TMP, Mp);
            xint_lshift(S, S, 1);
            while (round_up || even ? xint_cmp(TMP, S) >= 0 : xint_cmp(TMP, S) > 0)
            {
                xint_mul_1(S, S, 10);
                k = k + 1;
            }
            xint_rshift(S, S, 1);
            // Adjust Mp and Mm
            switch (mode)
            {
                default:
                    *decpt = k;
            }
            xint_mul_1(TMP, R, 2);
            xint_adda(TMP, TMP, Mp);
            xint_lshift(S, S, 1);
            if (!(round_up || even ? xint_cmp(TMP, S) >= 0 : xint_cmp(TMP, S) > 0))
            {
                xint_rshift(S, S, 1);
                break;
            }
            xint_rshift(S, S, 1);
        }
        xint_delete(TMP);
        //return 0;
    }

    xint_t U = XINT_INIT_VAL;
    int u;
    int low = 0;
    int high = 0;
    
    // XXX : calculate how many chars are needed
    
    char *sstr = malloc(20);
    char *pstr = sstr;
    
    while (1)
    {
        k = k - 1;

        xint_mul_1(R, R, 10);
        xint_div(U, R, R, S);
        
        u = U->size ? U->data[0] : 0;
        u += '0';

        xint_mul_1(Mm, Mm, 10);
        xint_mul_1(Mp, Mp, 10);
        
        // low = 2R < Mm
        xint_lshift(T2R, R, 1);
        low = round_up || even ? xint_cmp(T2R, Mm) <= 0 : xint_cmp(T2R, Mm) < 0;
        
        xint_lshift(T2S, S, 1);
        xint_adda(T2R, T2R, Mp);
        // Compare 2R with 2S - Mp

        if (round_up || even)
        {
            high = xint_cmp(T2R, T2S) >= 0;
        }
        else
        {
            high = xint_cmp(T2R, T2S) > 0;
        }
        if (low || high /*|| k == ndigits*/)
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
        xint_lshift(R, R, 1);
        int cmp = xint_cmp(R, S);
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

    xint_delete(R);
    xint_delete(S);
    xint_delete(Mm);
    xint_delete(Mp);
    xint_delete(T2S);
    xint_delete(T2R);

    return sstr;

    return NULL;
}
