/*
  PICCO: A General Purpose Compiler for Private Distributed Computation
  ** Copyright (C) from 2013 PICCO Team
  ** Department of Computer Science and Engineering, University of Notre Dame
  ** Department of Computer Science and Engineering, University at Buffalo (SUNY)

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
#include "Trunc.h"
#include <cstdio>
#include <iostream>

void doOperation_Trunc(mpz_t *result, mpz_t *input, int K, int M, int size, int threadID, NodeNetwork net, SecretShare *ss) {

    mpz_t *temp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++) {
        mpz_init(temp[i]);
    }

    mpz_t const2, power, const2M, constInv2M;
    // initialization
    mpz_init(const2M);
    mpz_init(constInv2M);
    mpz_init_set_ui(const2, 2);
    mpz_init_set_ui(power, M);
    ss->modPow(const2M, const2, power);
    ss->modInv(constInv2M, const2M);

    // start computation
    if (M > 1) {
        doOperation_Mod2M(temp, input, K, M, size, threadID, net, ss);
    } else {
        doOperation_Mod2(temp, input, K, size, threadID, net, ss);
    }

    ss->modSub(temp, input, temp, size);
    ss->modMul(result, temp, constInv2M, size);

    // free memory
    mpz_clear(const2);
    mpz_clear(power);
    mpz_clear(const2M);
    mpz_clear(constInv2M);

    for (int i = 0; i < size; i++) {
        mpz_clear(temp[i]);
    }
    free(temp);
}


void doOperation_Trunc(mpz_t *result, mpz_t *shares, int K, int *M, int size, int threadID, NodeNetwork net, SecretShare *ss) {
    int same = 1;
    for (int i = 1; i < size; i++)
        if (M[i] != M[0])
            same = 0;
    if (same) {
        if (M[0] >= K) {
            for (int i = 0; i < size; i++) {
                mpz_set_ui(result[i], 0); // set to zero when the number of bits we are shifting by is greater than K (the bitlength of K)
            }
        } else {
            doOperation_Trunc(result, shares, K, M[0], size, threadID, net, ss);
        }

    } else {
        // this is a very rare edge case, where each piece of data in shares1 is shifted by a different number of bits (stored in M)
        // we then do everything sequentially
        // mpz_t *shares = (mpz_t *)malloc(sizeof(mpz_t) * size);
        mpz_t const2, power, const2M, constInv2M;
        // initialization
        mpz_init(const2M);
        mpz_init(constInv2M);
        mpz_init_set_ui(const2, 2);
        ss->modPow(const2M, const2, power);

        for (int i = 0; i < size; i++) {
            if (M[i] >= K) {
                mpz_set_ui(result[i], 0); // set to zero when the number of bits we are shifting by is greater than K (the bitlength of K)
            } else {
                mpz_init_set_ui(power, M[i]);
                ss->modInv(constInv2M, const2M);
                // initialization
                // mpz_init_set(shares[i], shares1[i]);
                // start computation
                doOperation_Mod2M((mpz_t *)&result[i], (mpz_t *)&shares[i], K, M[i], size, threadID, net, ss);
                if (M[i] > 1) {
                    doOperation_Mod2M((mpz_t *)&result[i], (mpz_t *)&shares[i], K, M[i], size, threadID, net, ss);
                } else {
                    // special case, significantly cheaper than mod2m
                    doOperation_Mod2((mpz_t *)&result[i], (mpz_t *)&shares[i], K, size, threadID, net, ss);
                }

                ss->modSub(result[i], shares[i], result[i]);
                ss->modMul(result[i], result[i], constInv2M);
            }
        }

        // free memory
        // for (int i = 0; i < size; i++)
        //     mpz_clear(shares[i]);
        // free(shares);

        mpz_clear(const2);
        mpz_clear(power);
        mpz_clear(const2M);
        mpz_clear(constInv2M);
    }
}
