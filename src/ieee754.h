
#ifndef IEEE754_H
#define IEEE754_H

#include <stdio.h>

struct _Anvil_float
{
    uint64_t mant;
    int exp;
    int mant_bits;
    int exp_bits;
    int neg;
};

union float_bits
{
    float flt;
    uint32_t bits;
};

union double_bits
{
    double dbl;
    uint64_t bits;
};

union long_double_bits
{
    long double dbl;
    uint64_t bits[2];
};

double hex_to_float(uint32_t bits);
double hex_to_double(uint64_t bits);
uint64_t double_to_hex(double d);
void split_double(double dd, int *sign, uint64_t *f, int *e);
void split_long_double(long double dd, int *sign, uint64_t *f, int *e);

#endif // IEEE754_H
