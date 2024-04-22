
#include "dragon4.h"
#include "xint.h"
#include "ieee754.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

char *dtoa(double dd, int mode, int ndigits, int *decpt, int *sign, char **rve);

int cmp_dragon4_w_dtoa(double d);
int run_all_test_cases(int (*f)(double));

int main(int argc, const char * argv[])
{
    run_all_test_cases(cmp_dragon4_w_dtoa);
    return 0;
}

int cmp_dragon4_w_dtoa(double d)
{
    uint64_t f;
    int e;
    int sign;
    split_double(d, &sign, &f, &e);
    int decpt1;
    char *pstr1 = dragon4(e, f, 52, 0, 3, &decpt1);
    int decpt2;
    char *pstr2 = dtoa(d, 0, 0, &decpt2, &sign, NULL);
    if ((decpt1 != decpt2) || strcmp(pstr1, pstr2))
    {
        printf("0x%016llx\n", double_to_hex(d));
        printf("%s %d\n", pstr1, decpt1);
        printf("%s %d\n", pstr2, decpt2);
        printf("NG\n");
        return -1;
    }
    return 0;
}

int run_all_test_cases(int (*f)(double))
{
    FILE *fin = fopen("3rd_party/testnos3", "r");
    int err_cnt = 0;
    while (1)
    {
        char line[10000];
        char num_str[10000];
        uint32_t u1;
        uint32_t u2;
        if (fgets(line, 999, fin) == NULL)
        {
            break;
        }
        const char *p = line;
        while (isspace(*p))
        {
            ++p;
        }
        if (*p == '#')
        {
            continue;
        }
        int res = sscanf(line, "%s %x %x", num_str, &u1, &u2);
        if (res == -1)
        {
            continue;
        }
        uint64_t nn = (uint64_t)u1 << 32 | u2;
        if (f(hex_to_double(nn)) == -1)
        {
            ++err_cnt;
        }
    }
    printf("*** %d errors ***\n", err_cnt);
    return 0;
}
