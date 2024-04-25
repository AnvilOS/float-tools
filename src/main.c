
#include "dragon4.h"
#include "xint.h"
#include "ieee754.h"
#include "anvil_dtoa.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <fenv.h>
#include <math.h>
#include <time.h>

#include "gdtoa.h"

int cmp_dragon4_w_dtoa(double d);
int cmp_anvil_w_dtoa(double d);
int run_all_test_cases(int (*f)(double));

int main(int argc, const char * argv[])
{
    static double time_consumed = 0;
    clock_t start, end;
    start = clock();
    
    run_all_test_cases(cmp_anvil_w_dtoa);
    
    end = clock();
    time_consumed += (double)(end - start) / CLOCKS_PER_SEC;
    printf("Time=%f\n", time_consumed);
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
        printf("%.30e ", d);
        printf("0x%016llx\n", double_to_hex(d));
        printf("%s %d\n", pstr1, decpt1);
        printf("%s %d\n", pstr2, decpt2);
        printf("NG\n\n");
        return -1;
    }
    return 0;
}

int cmp_anvil_w_dtoa(double d)
{
    uint64_t f;
    int e;
    int sign;
    split_double(d, &sign, &f, &e);
    int decpt1;
    char *pstr1 = anvil_dtoa(d, 0, 0, &decpt1, &sign, NULL);
    int decpt2;
    char *pstr2 = dtoa(d, 0, 0, &decpt2, &sign, NULL);
    if ((decpt1 != decpt2) || strcmp(pstr1, pstr2))
    {
        printf("Mismatch between dtoa mode 0 and Dragon4\n");
        printf("%.30e ", d);
        printf("0x%016llx\n", double_to_hex(d));
        printf("%s %d\n", pstr1, decpt1);
        printf("%s %d\n", pstr2, decpt2);
        printf("NG\n\n");
        return -1;
    }
    free(pstr1);
    return 0;
}

int run_all_test_cases(int (*f)(double))
{
    const struct file_list { char *name; int type; } file_list[] =
    {
        { "3rd_party/1e23_problem", 1 },
        NULL,
    };
    int file_err = 0;
    int file_cnt = 0;
    int total_err = 0;
    int total_cnt = 0;

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
                res = sscanf(line, "%x %x %llx %s", &x4, &x8.bits, &nn, num_str);
                if (res == -1)
                {
                    continue;
                }
            }
            else
            {
                printf("Bad file format\n");
                continue;
            }
            if ((nn & 0x7ff0000000000000ULL) == 0x7ff0000000000000ULL)
            {
                continue;
            }
            if (f(hex_to_double(nn)) == -1)
            {
                ++file_err;
                ++total_err;
            }
            ++file_cnt;
            ++total_cnt;
        }
        printf("%d/%d errors in file\n", file_err, file_cnt);
        file_err = 0;
        file_cnt = 0;
        ++curr_file;
    }
    printf("*** %d/%d errors ***\n", total_err, total_cnt);
    return 0;
}
