/****************************************************************

The author of this software is David M. Gay.

Copyright (C) 1998 by Lucent Technologies
All Rights Reserved

Permission to use, copy, modify, and distribute this software and
its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the name of Lucent or any of its entities
not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.

LUCENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
IN NO EVENT SHALL LUCENT OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.

****************************************************************/

/* Please send bug reports to David M. Gay (dmg at acm dot org,
 * with " at " changed at "@" and " dot " changed to ".").	*/

#include "gdtoaimp.h"

#ifdef USE_LOCALE
#include "locale.h"
#endif

 int
#ifdef KR_headers
gethex(sp, fpi, exp, bp, sign MTa)
	CONST char **sp; CONST FPI *fpi; Long *exp; Bigint **bp; int sign; MTk
#else
gethex( CONST char **sp, CONST FPI *fpi, Long *exp, Bigint **bp, int sign MTd)
#endif
{
	Bigint *b;
	CONST unsigned char *decpt, *s0, *s, *s1;
	Long e, e1;
	ULong L, lostbits, *x;
	int big, esign, havedig, irv, j, k, n, n0, nb, nbits, up, zret;
#ifdef IEEE_Arith
	int check_denorm = 0;
#endif
#ifdef USE_LOCALE
	int i;
#ifdef NO_LOCALE_CACHE
	const unsigned char *decimalpoint = (unsigned char*)localeconv()->decimal_point;
#else
	const unsigned char *decimalpoint;
	static unsigned char *decimalpoint_cache;
	if (!(s0 = decimalpoint_cache)) {
		s0 = (unsigned char*)localeconv()->decimal_point;
		if ((decimalpoint_cache = (char*)MALLOC(strlen(s0) + 1))) {
			strcpy(decimalpoint_cache, s0);
			s0 = decimalpoint_cache;
			}
		}
	decimalpoint = s0;
#endif
#endif

	/**** if (!hexdig['0']) hexdig_init_D2A(); ****/
	*bp = 0;
	havedig = 0;
	s0 = *(CONST unsigned char **)sp + 2;
	while(s0[havedig] == '0')
		havedig++;
	s0 += havedig;
	s = s0;
	decpt = 0;
	zret = 0;
	e = 0;
	if (hexdig[*s])
		havedig++;
	else {
		zret = 1;
#ifdef USE_LOCALE
		for(i = 0; decimalpoint[i]; ++i) {
			if (s[i] != decimalpoint[i])
				goto pcheck;
			}
		decpt = s += i;
#else
		if (*s != '.')
			goto pcheck;
		decpt = ++s;
#endif
		if (!hexdig[*s])
			goto pcheck;
		while(*s == '0')
			s++;
		if (hexdig[*s])
			zret = 0;
		havedig = 1;
		s0 = s;
		}
	while(hexdig[*s])
		s++;
#ifdef USE_LOCALE
	if (*s == *decimalpoint && !decpt) {
		for(i = 1; decimalpoint[i]; ++i) {
			if (s[i] != decimalpoint[i])
				goto pcheck;
			}
		decpt = s += i;
#else
	if (*s == '.' && !decpt) {
		decpt = ++s;
#endif
		while(hexdig[*s])
			s++;
		}/*}*/
	if (decpt)
		e = -(((Long)(s-decpt)) << 2);
 pcheck:
	s1 = s;
	big = esign = 0;
	switch(*s) {
	  case 'p':
	  case 'P':
		switch(*++s) {
		  case '-':
			esign = 1;
			/* no break */
		  case '+':
			s++;
		  }
		if ((n = hexdig[*s]) == 0 || n > 0x19) {
			s = s1;
			break;
			}
		e1 = n - 0x10;
		while((n = hexdig[*++s]) !=0 && n <= 0x19) {
			if (e1 & 0xf8000000)
				big = 1;
			e1 = 10*e1 + n - 0x10;
			}
		if (esign)
			e1 = -e1;
		e += e1;
	  }
	*sp = (char*)s;
	if (!havedig)
		*sp = (char*)s0 - 1;
	if (zret)
		return STRTOG_Zero;
	if (big) {
		if (esign) {
			switch(fpi->rounding) {
			  case FPI_Round_up:
				if (sign)
					break;
				goto ret_tiny;
			  case FPI_Round_down:
				if (!sign)
					break;
				goto ret_tiny;
			  }
			goto retz;
 ret_tiny:
			b = Balloc(0 MTa);
			b->wds = 1;
			b->x[0] = 1;
			goto dret;
			}
		switch(fpi->rounding) {
		  case FPI_Round_near:
			goto ovfl1;
		  case FPI_Round_up:
			if (!sign)
				goto ovfl1;
			goto ret_big;
		  case FPI_Round_down:
			if (sign)
				goto ovfl1;
		  }
 ret_big:
		nbits = fpi->nbits;
		n0 = n = nbits >> kshift;
		if (nbits & kmask)
			++n;
		for(j = n, k = 0; j >>= 1; ++k);
		*bp = b = Balloc(k MTa);
		b->wds = n;
		for(j = 0; j < n0; ++j)
			b->x[j] = ALL_ON;
		if (n > n0)
			b->x[j] = ALL_ON >> (ULbits - (nbits & kmask));
		*exp = fpi->emax;
		return STRTOG_Normal | STRTOG_Inexlo;
		}
	n = s1 - s0 - 1;
	for(k = 0; n > (1 << (kshift-2)) - 1; n >>= 1)
		k++;
	b = Balloc(k MTa);
	x = b->x;
	n = 0;
	L = 0;
#ifdef USE_LOCALE
	for(i = 0; decimalpoint[i+1]; ++i);
#endif
	while(s1 > s0 && s1[-1] == '0') {
		--s1;
		e += 4;
		}
	while(s1 > s0) {
#ifdef USE_LOCALE
		if (*--s1 == decimalpoint[i]) {
			s1 -= i;
			continue;
			}
#else
		if (*--s1 == '.')
			continue;
#endif
		if (n == ULbits) {
			*x++ = L;
			L = 0;
			n = 0;
			}
		L |= (hexdig[*s1] & 0x0f) << n;
		n += 4;
		}
	*x++ = L;
	b->wds = n = x - b->x;
	nb = ULbits*n - hi0bits(L);
	nbits = fpi->nbits;
	lostbits = 0;
	x = b->x;
	if (nb > nbits) {
		n = nb - nbits;
		if (any_on(b,n)) {
			lostbits = 1;
			k = n - 1;
			if (x[k>>kshift] & 1 << (k & kmask)) {
				lostbits = 2;
				if (k > 0 && any_on(b,k))
					lostbits = 3;
				}
			}
		rshift(b, n);
		e += n;
		}
	else if (nb < nbits) {
		n = nbits - nb;
		b = lshift(b, n MTa);
		e -= n;
		x = b->x;
		}
	if (e > fpi->emax) {
 ovfl:
		Bfree(b MTa);
 ovfl1:
#ifndef NO_ERRNO
		errno = ERANGE;
#endif
		switch (fpi->rounding) {
		  case FPI_Round_zero:
			goto ret_big;
		  case FPI_Round_down:
			if (!sign)
				goto ret_big;
			break;
		  case FPI_Round_up:
			if (sign)
				goto ret_big;
		  }
		return STRTOG_Infinite | STRTOG_Overflow | STRTOG_Inexhi;
		}
	irv = STRTOG_Normal;
	if (e < fpi->emin) {
		irv = STRTOG_Denormal;
		n = fpi->emin - e;
		if (n >= nbits) {
			switch (fpi->rounding) {
			  case FPI_Round_near:
				if (n == nbits && (n < 2 || lostbits || any_on(b,n-1)))
					goto one_bit;
				break;
			  case FPI_Round_up:
				if (!sign)
					goto one_bit;
				break;
			  case FPI_Round_down:
				if (sign) {
 one_bit:
					x[0] = b->wds = 1;
 dret:
					*bp = b;
					*exp = fpi->emin;
#ifndef NO_ERRNO
					errno = ERANGE;
#endif
					return STRTOG_Denormal | STRTOG_Inexhi
						| STRTOG_Underflow;
					}
			  }
			Bfree(b MTa);
 retz:
#ifndef NO_ERRNO
			errno = ERANGE;
#endif
			return STRTOG_Zero | STRTOG_Inexlo | STRTOG_Underflow;
			}
		k = n - 1;
#ifdef IEEE_Arith
		if (!k) {
			switch(fpi->rounding) {
			  case FPI_Round_near:
				if (((b->x[0] & 3) == 3) || (lostbits && (b->x[0] & 1))) {
					multadd(b, 1, 1 MTa);
 emin_check:
					if (b->x[1] == (1 << (Exp_shift + 1))) {
						rshift(b,1);
						e = fpi->emin;
						goto normal;
						}
					}
				break;
			  case FPI_Round_up:
				if (!sign && (lostbits || (b->x[0] & 1))) {
 incr_denorm:
					multadd(b, 1, 2 MTa);
					check_denorm = 1;
					lostbits = 0;
					goto emin_check;
					}
				break;
			  case FPI_Round_down:
				if (sign && (lostbits || (b->x[0] & 1)))
					goto incr_denorm;
				break;
			  }
			}
#endif
		if (lostbits)
			lostbits = 1;
		else if (k > 0)
			lostbits = any_on(b,k);
#ifdef IEEE_Arith
		else if (check_denorm)
			goto no_lostbits;
#endif
		if (x[k>>kshift] & 1 << (k & kmask))
			lostbits |= 2;
#ifdef IEEE_Arith
 no_lostbits:
#endif
		nbits -= n;
		rshift(b,n);
		e = fpi->emin;
		}
	if (lostbits) {
		up = 0;
		switch(fpi->rounding) {
		  case FPI_Round_zero:
			break;
		  case FPI_Round_near:
			if (lostbits & 2
			 && (lostbits | x[0]) & 1)
				up = 1;
			break;
		  case FPI_Round_up:
			up = 1 - sign;
			break;
		  case FPI_Round_down:
			up = sign;
		  }
		if (up) {
			k = b->wds;
			b = increment(b MTa);
			x = b->x;
			if (irv == STRTOG_Denormal) {
				if (nbits == fpi->nbits - 1
				 && x[nbits >> kshift] & 1 << (nbits & kmask))
					irv =  STRTOG_Normal;
				}
			else if (b->wds > k
			 || ((n = nbits & kmask) !=0
			      && hi0bits(x[k-1]) < 32-n)) {
				rshift(b,1);
				if (++e > fpi->emax)
					goto ovfl;
				}
			irv |= STRTOG_Inexhi;
			}
		else
			irv |= STRTOG_Inexlo;
		}
#ifdef IEEE_Arith
 normal:
#endif
	*bp = b;
	*exp = e;
	return irv;
	}
