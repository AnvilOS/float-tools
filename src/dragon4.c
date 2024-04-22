
#include "dragon4.h"

#include "xint.h"
#include "xint_io.h"

#include <stdio.h>
#include <string.h>

#define MAX(a, b) (a>b?a:b)

int fixup(xint_t R, xint_t S , xint_t Mp, xint_t Mm, int *k, uint64_t *f, int p, int mode, int *place);

char sstr[1000];

int round_up = 0;

char *dragon4(int e, uint64_t f, int p, int mode, int place, int *pk)
{
    if (f & 1)
    {
        round_up = 1;
    }
    if (f == 0)
    {
        *pk = 1;
        return strdup("0");
    }
    xint_t R = XINT_INIT_VAL;
    xint_t S = XINT_INIT_VAL;
    xint_t Mm = XINT_INIT_VAL;
    xint_t Mp = XINT_INIT_VAL;
    xint_t T2S = XINT_INIT_VAL;
    xint_t T2R = XINT_INIT_VAL;

    xint_assign_uint64(R, f);
    xint_assign_uint64(S, 1);
    xint_assign_uint64(Mm, 1);
    xint_assign_uint64(Mp, 1);

    int k = 0;
    
    xint_lshift(R, R, MAX(e-p, 0));
    xint_lshift(S, S, MAX(0, -(e-p)));
    xint_lshift(Mm, Mm, MAX(e-p, 0));
    xint_copy(Mp, Mm);

    fixup(R, S, Mp, Mm, &k, &f, p, mode, &place);
    
    xint_t U = XINT_INIT_VAL;
    int u;
    int low = 0;
    int high = 0;
    
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
        low = xint_cmp(T2R, Mm) < 0;
        
        xint_lshift(T2S, S, 1);
        xint_adda(T2R, T2R, Mp);
        // Compare 2R with 2S - Mp
        if (round_up)
        {
            high = xint_cmp(T2R, T2S) >= 0;
        }
        else
        {
            high = xint_cmp(T2R, T2S) > 0;
        }
        if (low || high || k == place)
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
        // Scale R up by 2
        xint_lshift(R, R, 1);
        int cmp = xint_cmp(R, S);
        if (cmp <= 0 || (cmp == 0 && round_up))
        {
            *pstr++ = u;
        }
        else
        {
            *pstr++ = u + 1;
        }
    }

    *pstr = 0;
    *pk = k + (int)strlen(sstr);
    return sstr;
}

int fixup(xint_t R, xint_t S , xint_t Mp, xint_t Mm, int *k, uint64_t *f, int p, int mode, int *place)
{
    if (*f == (1 << (p - 1)))
    {
        xint_lshift(Mp, Mp, 1);
        xint_lshift(R, R, 1);
        xint_lshift(S, S, 1);
    }
    *k = 0;

    xint_t ceil_s_div_10 = XINT_INIT_VAL;
    xint_copy(ceil_s_div_10, S);
    xword_t rem;
    xint_div_1(ceil_s_div_10, &rem, ceil_s_div_10, 10);
    if (rem)
    {
        xint_adda_1(ceil_s_div_10, ceil_s_div_10, 1);
    }
    while (round_up ? xint_cmp(R, ceil_s_div_10) <= 0 : xint_cmp(R, ceil_s_div_10) < 0)
    {
        *k = *k - 1;
        xint_mul_1(R, R, 10);
        xint_mul_1(Mm, Mm, 10);
        xint_mul_1(Mp, Mp, 10);
    }
    
    while (1)
    {
        xint_t tmp = XINT_INIT_VAL;
        xint_mul_1(tmp, R, 2);
        xint_adda(tmp, tmp, Mp);
        xint_lshift(S, S, 1);
        while (xint_cmp(tmp, S) >= 0)
        {
            xint_mul_1(S, S, 10);
            *k = *k + 1;
        }
        xint_rshift(S, S, 1);
        // Adjust Mp and Mm
        switch (mode)
        {
            default:
                *place = *k;
        }
        xint_mul_1(tmp, R, 2);
        xint_adda(tmp, tmp, Mp);
        xint_lshift(S, S, 1);
        if (!(xint_cmp(tmp, S) >= 0))
        {
            xint_rshift(S, S, 1);
            break;
        }
        xint_rshift(S, S, 1);
    }
    return 0;
}
