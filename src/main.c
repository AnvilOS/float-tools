
#include <stdio.h>

#include "dragon4.h"
#include "xint.h"

union double_bits
{
    double dbl;
    uint64_t bits;
};

static void split_double(double dd, int *sign, uint64_t *f, int *e);

int main(int argc, const char * argv[])
{
    uint64_t f;
    int e;
    int sign;
    
    split_double(123.456, &sign, &f, &e);
    dragon4(e, f, 52, 0, 3);
    
    return 0;
}

static void split_double(double dd, int *sign, uint64_t *f, int *e)
{
    union double_bits value;
    value.dbl = dd;
    *sign = value.bits >> 63;
    *e = ((value.bits >> 52) & 0x7ff) - 1023;
    *f = value.bits & 0xfffffffffffff;
    if (*e == -1023)
    {
        ++*e;
    }
    else
    {
        *f |= 0x10000000000000;
    }
}
