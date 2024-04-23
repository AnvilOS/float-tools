
#ifndef _ANVIL_DTOA_H
#define _ANVIL_DTOA_H

char *anvil_dtoa(double dd, int mode, int ndigits, int *decpt, int *sign, char **rve);
char *anvil_ldtoa(long double dd, int mode, int ndigits, int *decpt, int *sign, char **rve);

#endif // _ANVIL_DTOA_H
