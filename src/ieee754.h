
#ifndef IEEE754_H
#define IEEE754_H

#include <stdio.h>

void split_double(double dd, int *sign, uint64_t *f, int *e);
double hex_to_double(uint64_t);

#endif // IEEE754_H
