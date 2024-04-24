
#include "anvil_dtoa.h"
#include "ieee754.h"
#include "xint.h"
#include "pow5.h"

#include <string.h>
#include <stdlib.h>

#define MAX(a, b) (a>b?a:b)

static char *dragon4(uint64_t f, int e, int p, int mode, int ndigits, int *decpt);
static int dragon4_init(uint64_t f, int e, int p, xint_t x2R, xint_t x2S, xint_t Mm, xint_t Mp, xint_t TMP);

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
    const int p = 63;
    uint64_t f;
    int e;
    split_long_double(dd, sign, &f, &e);
    return dragon4(f, e, p, mode, ndigits, decpt);
}

static char *dragon4(uint64_t f, int e, int p, int mode, int cutoff_place, int *decpt)
{
    xint_t x2R, x2S, Mm, Mp, TMP;

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
    
    int k = dragon4_init(f, e, p, x2R, x2S, Mm, Mp, TMP);
    
    // This is the fixup procedure, in line
    
    // Load TMP with ceil(S/10) - actually ceil(2xS/10)
    xint_copy(TMP, x2S);
    xword_t rem;
    xint_div_1(TMP, &rem, TMP, 10);
    if (rem)
    {
        xint_adda_1(TMP, TMP, 1);
    }
    // while R < ceil(S/10) - actually 2xR < ceil(2xS/10)
    while (xint_cmp(x2R, x2S) < 0)
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
        
        xint_div(TMP, x2R, x2R, x2S);
        u = TMP->size ? TMP->data[0] : 0;
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
    xint_delete(TMP);

    return sstr;
}

int dragon4_init(uint64_t f, int e, int p, xint_t x2R, xint_t x2S, xint_t Mm, xint_t Mp, xint_t TMP)
{
    int k = 0;
    
    // INITIALISE THE VARIABLES
    // R = f << max(e-p, 0)
    // S = 1 << max(0, -(e-p))
    // M+ = 1 << max(e-p, 0)
    // M- = 1 << max(e-p, 0)
    //
    // Instead of doing the calcs now just record what will be done. We'll
    // do all the calcs at the end
    int r2e = 0;
    int s2e = 0;
    int r5e = 0;
    int s5e = 0;
    if (e - p >= 0)
    {
        r2e = e - p;
    }
    else
    {
        s2e = p - e;
    }
    
    // To save time running the 2 while loops on R and S at the start
    // of Dragon4 we pre-multiply S by the log of the double to be
    // printed
    //            d = f x 2^e
    //     log10(d) = log10(f x 2^e)
    //              = e x log10(2f)    note that 1 <= f < 2
    //              < 0.30103 x e
    //
    // We want an underestimate so take the floor
    //
    //      est_log = floor(e x 30103 / 100000)
    //
    // Don't forget denormals...
    uint64_t mask = 1ULL << p;
    int e_norm = e;
    while ((f & mask) == 0)
    {
        mask >>= 1;
        --e_norm;
    }
    int est_log = (e_norm * 30103 / 100000);
    if (est_log <= 0)
    {
        --est_log;
    }
    
    xint_init(x2R, 27);
    xint_init(x2S, 27);
    xint_init(Mm, 27);
    xint_init(Mp, 27);
    xint_init(TMP, 27);

    xint_assign_uint64(x2R, f);
    xint_assign_uint64(x2S, 1);
    xint_assign_uint64(Mm, 1);
    xint_assign_uint64(Mp, 1);
    
    if (est_log < 0)
    {
        r2e += -est_log;
        r5e += -est_log;
    }
    else
    {
        s2e += est_log;
        s5e += est_log;
    }
    
    // Remove common factors now
    if (r2e < s2e)
    {
        s2e -= r2e;
        r2e = 0;
    }
    else
    {
        r2e -= s2e;
        s2e = 0;
    }
    if (r5e < s5e)
    {
        s5e -= r5e;
        r5e = 0;
    }
    else
    {
        r5e -= s5e;
        s5e = 0;
    }

    // From now on we will keep 2xR and 2xS for convenience so add one to each
    xint_lshift(x2R, x2R, r2e + 1);
    xint_mul_5exp(x2R, r5e);

    xint_lshift(x2S, x2S, s2e + 1);
    xint_mul_5exp(x2S, s5e);

    // M+ and M- are scaled with R
    xint_lshift(Mm, Mm, r2e);
    xint_mul_5exp(Mm, r5e);
    xint_copy(Mp, Mm);

    // Account for unequal gaps
    if (f == 1ULL << p)
    {
        xint_lshift(Mp, Mp, 1);
        xint_lshift(x2R, x2R, 1);
        xint_lshift(x2S, x2S, 1);
    }

    k = est_log;

    return k;
}
