
#include "ieee754.h"

union double_bits
{
    double dbl;
    uint64_t bits;
};

double hex_to_double(uint64_t bits)
{
    union double_bits d;
    d.bits = bits;
    return d.dbl;
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
