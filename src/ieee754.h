
#ifndef IEEE754_H
#define IEEE754_H

#include <stdio.h>

double hex_to_double(uint64_t);
uint64_t double_to_hex(double d);
void split_double(double dd, int *sign, uint64_t *f, int *e);

#endif // IEEE754_H
