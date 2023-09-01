/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame
   ** Department of Computer Science and Engineering, University of Buffalo (SUNY)

   PICCO is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   PICCO is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with PICCO. If not, see <http://www.gnu.org/licenses/>.
*/

#include "ShamirUtil.h"

int modulo(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

long long nChoosek(long long n, long long k) {
    if (k > n)
        return 0;
    if (k * 2 > n)
        k = n - k;
    if (k == 0)
        return 1;

    int result = n;
    for (int i = 2; i <= k; ++i) {
        result *= (n - i + 1);
        result /= i;
    }
    return result;
}


/* General utility functions */
void smc_batch_free_operator(mpz_t **op, int size) {
    for (int i = 0; i < size; i++)
        mpz_clear((*op)[i]);
    free(*op);
}

void smc_batch_free_operator(mpz_t ***op, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 4; j++)
            mpz_clear((*op)[i][j]);
        free((*op)[i]);
    }
    free(*op);
}

// used for comparisons
// unknown as to why the original version stores at pointer
int smc_compute_len(int alen, int blen) {
    return alen >= blen ? alen : blen;
}

void convertFloat(float value, int K, int L, mpz_t **elements) {
    unsigned int *newptr = (unsigned int *)&value;
    int s = *newptr >> 31;
    int e = *newptr & 0x7f800000;
    e >>= 23;
    int m = 0;
    m = *newptr & 0x007fffff;

    int z;
    long v, p, k;
    mpz_t significant, one, two, tmp, tmpm;
    mpz_init(significant);
    mpz_init(tmp);
    mpz_init_set_ui(one, 1);
    mpz_init_set_ui(two, 2);
    mpz_init(tmpm);

    if (e == 0 && m == 0) {
        s = 0;
        z = 1;
        // v = 0;
        mpz_set_ui(significant, 0);
        p = 0;
    } else {
        z = 0;
        if (L < 8) {
            k = (1 << L) - 1;
            /*check for overflow*/
            if (e - 127 - K + 1 > k) {
                p = k;
                mpz_mul_2exp(significant, one, K);
                mpz_sub_ui(significant, significant, 1);
                // v = (1 << K) - 1;
                /*check for underflow*/
            } else if (e - 127 - K + 1 < -k) {
                p = -k;
                mpz_set_ui(significant, 1);
                // v = 1;
            } else {
                p = e - 127 - K + 1;
                m = m + (1 << 23);
                mpz_set_si(tmpm, m);
                if (K < 24) {
                    mpz_pow_ui(tmp, two, 24 - K);
                    mpz_div(significant, tmpm, tmp);
                    // v = (m >> (24 - K));
                } else {
                    mpz_mul_2exp(significant, tmpm, K - 24);
                    // v = m << (K - 24);
                }
            }
        } else {
            p = e - 127 - K + 1;
            m = m + (1 << 23);
            mpz_set_si(tmpm, m);
            if (K < 24) {
                mpz_pow_ui(tmp, two, 24 - K);
                mpz_div(significant, tmpm, tmp);
                // v = (m >> (24 - K));
            } else {
                mpz_set(significant, tmpm);
                mpz_mul_2exp(significant, significant, K - 24);
                // v = m;
                // v = v << (K - 24);
            }
        }
    }

    // assignments;
    mpz_set((*elements)[0], significant);
    mpz_set_si((*elements)[1], p);
    mpz_set_si((*elements)[2], z);
    mpz_set_si((*elements)[3], s);

    // clear the memory
    mpz_clear(one);
    mpz_clear(two);
    mpz_clear(tmp);
    mpz_clear(tmpm);
    mpz_clear(significant);
}

void convertDouble(double value, int K, int L, mpz_t **elements) {
    unsigned long *newptr = (unsigned long *)&value;
    int s = (int)(*newptr >> 63);
    unsigned long temp = (1 << 11);
    temp--;
    temp = temp << 52;
    temp = (*newptr & temp);
    temp = temp >> 52;
    int e = (int)temp;
    unsigned long m = 0;
    temp = 1;
    temp = temp << 52;
    temp--;
    m = (*newptr & temp);

    int z;
    long v, p, k;

    mpz_t significant, tmp, tmpm, one, two;
    mpz_init(significant);
    mpz_init(tmp);
    mpz_init(tmpm);
    mpz_init_set_ui(one, 1);
    mpz_init_set_ui(two, 2);

    if (e == 0 && m == 0) {
        s = 0;
        z = 1;
        // v = 0;
        mpz_set_ui(significant, 0);
        p = 0;
    } else {
        z = 0;
        if (L < 11) {
            k = (1 << L);
            k--;
            /*check for overflow*/
            if (e - 1023 - K + 1 > k) {
                p = k;
                mpz_mul_2exp(significant, one, K);
                mpz_sub_ui(significant, significant, 1);
                // v = (1 << K);
                // v--;
                /*check for underflow*/
            } else if (e - 1023 - K + 1 < -k) {
                p = -k;
                mpz_set_ui(significant, 1);
                // v = 1;
            } else {
                p = e - 1023 - K + 1;
                k = 1;
                k = k << 52;
                m = m + k;
                mpz_set_si(tmpm, m);

                if (K < 53) {
                    mpz_pow_ui(tmp, two, 53 - K);
                    mpz_div(significant, tmpm, tmp);
                    // v = (m >> (53 - K));
                } else {
                    mpz_mul_2exp(significant, tmpm, K - 53);
                    // v = m << (K - 53);
                }
            }
        } else {
            p = e - 1023 - K + 1;
            k = 1;
            k = k << 52;
            m = m + k;
            mpz_set_ui(tmpm, m);

            if (K < 53) {
                mpz_pow_ui(tmp, two, 53 - K);
                mpz_div(significant, tmpm, tmp);
                // v = (m >> (53 - K));
            } else {
                mpz_mul_2exp(significant, tmpm, K - 53);
                // v = m << (K - 53);
            }
        }
    }

    mpz_set((*elements)[0], significant);
    mpz_set_si((*elements)[1], p);
    mpz_set_si((*elements)[2], z);
    mpz_set_si((*elements)[3], s);

    // free the memory
    mpz_clear(one);
    mpz_clear(two);
    mpz_clear(tmp);
    mpz_clear(tmpm);
    mpz_clear(significant);

    //(*elements)[0] = v;
    //(*elements)[1] = p;
    //(*elements)[2] = z;
    //(*elements)[3] = s;
}