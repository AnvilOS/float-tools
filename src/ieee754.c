
#include "ieee754.h"

double hex_to_double(uint64_t bits)
{
    union double_bits d;
    d.bits = bits;
    return d.dbl;
}

double hex_to_float(uint32_t bits)
{
    union float_bits f;
    f.bits = bits;
    return f.flt;
}

uint64_t double_to_hex(double d)
{
    union double_bits db;
    db.dbl = d;
    return db.bits;
}

void split_double(double dd, int *sign, uint64_t *f, int *e)
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

void split_long_double(long double dd, int *sign, uint64_t *f, int *e)
{
    union long_double_bits value;
    value.dbl = dd;
    *sign = (value.bits[1] >> 15) & 1;
    *e = (value.bits[1] & 0x7fff) - 16383;
    *f = value.bits[0];
    if (*e == -16383)
    {
        ++*e;
    }
}
