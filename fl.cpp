/*
From F(n+m) = F(n)*F(m-1)+F(n+1)*F(m) 

deduce that

F(2n) = 2 * F(n) * F(n+1) - F(n) ^2
F(2n+1) = F(n-1) ^2 + F(n) ^2


and this can be used in a binary left-to-right "double-and-add" addition chain.

New little improvement

By setting n = s + 2^k and using Lucas sequences

#1 compute F(s+1) and F(s) as above,

#2 set L(s) = 2 * F(s+1) - F(s)

#3 repeat k time this simplified iteration

F(2n) = F(n) * L(n)
L(2n) = L(n) ^2 - 2 * (-1)^n

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include <gmp.h>

int main(int argc, char **argv)
{
	bool cycle = false;
	bool number = true;
	unsigned long numbers[20];
	unsigned count = 0;
	for (unsigned i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-t"))	// clock
		{
			cycle = true;
			continue;
		}
		if (!strcmp(argv[i], "-n"))	// number
		{
			number = true;
			continue;
		}
		if (!strcmp(argv[i], "-v"))	// verbose
		{
			number = true;
			cycle = true;
			continue;
		}
		if (!strcmp(argv[i], "-s"))	// silent
		{
			number = false;
			cycle = false;
			continue;
		}
		if (isdigit(*argv[i]) && count < 20) {
			numbers[count++] = strtoull(argv[i], 0, 0);
			continue;
		}
		printf("%s: run fibonacci sequence ad nauseam\n", argv[0]);
		printf("-t : enable display elapsed time in milliseconds\n");
		printf("-n : enable display the number (might be huge !!!)\n");
		printf("-v : display all\n");
		printf("-s : silent\n");
		printf("\n");
		printf("Examples\n");
		printf("\n");
		printf("%s 17 18 19\n", argv[0]);
		printf("%s -s -t 9 10 11 99 100 101 999 1000 1001\n", argv[0]);
		printf("\n");
		printf("This is based on libgmp and will crash the way GMP does when internal allocation fails\n");
		printf("Better buy zillions of petabytes of memory !!!!\n");
		exit(1);
	}

	for (unsigned i = 0; i < count; i++) {
		unsigned long nn = numbers[i];
		fflush(stdout);
		fflush(stderr);

		struct timespec t0, t1;
		clock_gettime(CLOCK_MONOTONIC, &t0);

		mpz_t f1;
		mpz_init(f1);
		switch (nn) {
		case 0:
			mpz_set_ui(f1, 0);
			break;
		case 1:
			mpz_set_ui(f1, 1);
			break;
		case 2:
			mpz_set_ui(f1, 1);
			break;
		case 3:
			mpz_set_ui(f1, 2);
			break;
		default:
			{
				mpz_t f0, t, l, s;
				mpz_init(f0);
				mpz_init(l);
				mpz_init(s);
				mpz_init(t);

// -----------------------------------------
// compute ss such as nn = ss * 2^k
// -----------------------------------------
				unsigned k = __builtin_ctzll(nn);
				unsigned long ss = nn >> k;
				unsigned bit = 63 - __builtin_clzll(ss);
				mpz_set_ui(f0, 0);	// f(0)
				mpz_set_ui(f1, 1);	// f(1)

// -----------------------------------------
// compute F[s] and F[s-1]
// -----------------------------------------

				while (bit--) {
					// double
					mpz_mul(t, f0, f0);	// t = (f0)^2
					mpz_add(s, f0, f1);
					mpz_mul(s, s, s);	// s = (f0+f1)^2
					mpz_mul(f0, f1, f1);
					mpz_add(f0, f0, t);	// f0 = (f1)^2 + (f0)^2
					mpz_sub(f1, s, t);	// f1 = (f0+f1)^2 - (f0)^2 
					if ((ss >> bit) & 1) {
						// add
						mpz_add(t, f1, f0);
						mpz_set(f0, f1);
						mpz_set(f1, t);	// (f0, f1) = (f1, f1 + f0)
					}
				}
				mpz_clear(s);

				if (k != 0) {
// -----------------------------------------
// compute F[s+1] 
// -----------------------------------------
					mpz_add(t, f1, f0);
// -----------------------------------------
// compute L(s) = 2*F[s+1] - F[s]
// -----------------------------------------
					mpz_add(l, t, t);
					mpz_sub(l, l, f1);	// l = 2 * f1 - f0

					mpz_clear(t);
					mpz_clear(f0);

					bool subtract_once = true;
					while (k-- > 1) {
// -----------------------------------------
// compute F[2s] 
// compute l[2s] 
// -----------------------------------------
						mpz_mul(f1, f1, l);	// f1 = f1 * l
						mpz_mul(l, l, l);

						if (subtract_once) {
							mpz_add_ui(l, l, 2);	// l = (l)^2 -2
							subtract_once = false;
						} else {
							mpz_sub_ui(l, l, 2);	// l = (l)^2 -2
						}

					}
					mpz_mul(f1, f1, l);	// f1 = f1 * l
				} else {
					mpz_clear(t);
					mpz_clear(f0);
				}

				mpz_clear(l);
			}
		}
		clock_gettime(CLOCK_MONOTONIC, &t1);
		if (cycle) {
			unsigned long bits = mpz_sizeinbase(f1, 2);
			unsigned long diff = t1.tv_sec - t0.tv_sec;	// seconds
			diff *= 1e9;	// into nsecs
			diff += t1.tv_nsec;
			diff -= t0.tv_nsec;
			double fdiff = (double)diff;
			fdiff /= 1e6;	// into msecs
			printf("F(%12ld) --> %12.3f msecs (%12lu bits)\n", nn, fdiff, bits);
		}
		if (number) {
			gmp_printf("F(%ld) = %Zd\n", nn, f1);
		}
		mpz_clear(f1);
	}
	return 0;
}
