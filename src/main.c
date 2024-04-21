
#include <stdio.h>

#include "dragon4.h"
#include "xint.h"
#include "ieee754.h"

int main(int argc, const char * argv[])
{
    uint64_t f;
    int e;
    int sign;
    printf("%.60e\n", hex_to_double(0x44b52d02c7e14af6));
    split_double(hex_to_double(0x44b52d02c7e14af6), &sign, &f, &e);
    dragon4(e, f, 52, 0, 3);
    return 0;
}

