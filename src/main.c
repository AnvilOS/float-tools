
#include "dragon4.h"
#include "xint.h"
#include "ieee754.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

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
        printf("Mismatch between dtoa mode 0 and Dragon4\n");
        printf("%.30e\n", d);
        printf("0x%016llx\n", double_to_hex(d));
        printf("%s %d\n", pstr1, decpt1);
        printf("%s %d\n", pstr2, decpt2);
        printf("NG\n\n");
        return -1;
    }
//    printf("0x%016llx\n", double_to_hex(d));
//    printf("%s %d\n", pstr1, decpt1);
//    printf("%s %d\n", pstr2, decpt2);
//    printf("OK\n");
    return 0;
}

int run_all_test_cases(int (*f)(double))
{
    const struct file_list { char *name; int type; } file_list[] =
    {
        { "3rd_party/1e23_problem", 1 },
        NULL,
    };
    int err_cnt = 0;
    
    const struct file_list *curr_file = &file_list[0];
    while (curr_file->name)
    {
        printf("Processing file %s\n", curr_file->name);
        FILE *fin = fopen(curr_file->name, "r");
        if (fin == NULL)
        {
            printf("Cannot process %s\n", curr_file->name);
            continue;
        }
        while (1)
        {
            char line[10000];
            char num_str[10000];
            uint64_t nn;
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
            int res;
            if (curr_file->type == 1)
            {
                uint32_t u1;
                uint32_t u2;
                res = sscanf(line, "%s %x %x", num_str, &u1, &u2);
                if (res == -1)
                {
                    continue;
                }
                nn = (uint64_t)u1 << 32 | u2;
            }
            else if (curr_file->type == 2)
            {
                uint32_t x4;
                union { uint32_t bits; float fp; } x8;
                char num_str[1000];
                if (fgets(line, 999, fin) == NULL)
                {
                    break;
                }
                res = sscanf(line, "%x %x %llx %s", &x4, &x8.bits, &nn, num_str);
                if (res == -1)
                {
                    continue;
                }
            }
            else
            {
                continue;
            }
            if ((nn & 0x7ff0000000000000ULL) == 0x7ff0000000000000ULL)
            {
                continue;
            }
            if (f(hex_to_double(nn)) == -1)
            {
                ++err_cnt;
            }
        }
        printf("%d errors\n", err_cnt);
        ++curr_file;
    }
    printf("*** %d errors ***\n", err_cnt);
    return 0;
}
