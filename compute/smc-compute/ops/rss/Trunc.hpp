/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2024 PICCO Team
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
#ifndef _TRUNC_HPP_
#define _TRUNC_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "EdaBit.hpp"

// trunation of all data by a single M
template <typename T>
void doOperation_Trunc(T **result, T **input, int K, int m, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;

    T **edaBit_r = new T *[numShares];
    T **sum = new T *[numShares];
    T **r_hat = new T *[numShares];
    T **b = new T *[numShares];
    T **b_km1 = new T *[numShares];
    T **b_2 = new T *[numShares];

    T *c = new T[size];
    memset(c, 0, sizeof(T) * size);
    T *c_prime = new T[size];
    memset(c_prime, 0, sizeof(T) * size);

    for (size_t i = 0; i < numShares; i++) {
        edaBit_r[i] = new T[size];
        r_hat[i] = new T[size];

        b_km1[i] = new T[size];
        memset(b_km1[i], 0, sizeof(T) * size);
        b_2[i] = new T[size];
        memset(b_2[i], 0, sizeof(T) * size);
        sum[i] = new T[size];

        b[i] = new T[size];
        memset(b[i], 0, sizeof(T) * size);
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, 1);

    edaBit_Trunc(edaBit_r, r_hat, b_2, b_km1, m, size, ring_size, nodeNet, ss);

    T *res_check_1 = new T[size];
    memset(res_check_1, 0, sizeof(T) * size);

    T *res_check_2 = new T[size];
    memset(res_check_2, 0, sizeof(T) * size);

    T *res_check_3 = new T[size];
    memset(res_check_3, 0, sizeof(T) * size);

    T *res_check_4 = new T[size];
    memset(res_check_4, 0, sizeof(T) * size);

    Open(res_check_1, edaBit_r, size, -1, nodeNet, ss);
    Open(res_check_2, r_hat, size, -1, nodeNet, ss);
    Open_Bitwise(res_check_3, b_2, size, -1, nodeNet, ss); // b_2 shares are in Z2
    Open(res_check_4, b_km1, size, -1, nodeNet, ss);

    for (size_t i = 0; i < size; i++) {
        printf("(open)  r      [%lu]: %u\n", i, res_check_1[i]);
        print_binary(res_check_1[i], ring_size);
        printf("(open)  b_2    [%lu]: %u\n", i, res_check_3[i]);
        print_binary(res_check_3[i], ring_size);
        printf("(open)  b_km1  [%lu]: %u\n", i, res_check_4[i]);
        print_binary(res_check_4[i], ring_size);
        printf("(open)  r_hat  [%lu]: %u\n", i, res_check_2[i]);
        print_binary(res_check_2[i], ring_size);
    }

    // computing the sum of input and edabit_r
    for (size_t i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            sum[s][i] = (input[s][i] + edaBit_r[s][i]);
        }
    }

    Open(c, sum, size, threadID, nodeNet, ss);

    for (size_t i = 0; i < size; i++) {
        // (c / 2^m) mod 2^(k-m-1)
        c_prime[i] = (c[i] >> T(m)) & ss->SHIFT[ring_size - m - 1];
        for (size_t s = 0; s < numShares; s++) {
            b[s][i] = b_km1[s][i] + ((c[i] * ai[s]) >> T(ring_size - 1)) - 2 * ((c[i]) >> T(ring_size - 1)) * b_km1[s][i];
            r_hat[s][i] = r_hat[s][i] - (b_km1[s][i] << T(ring_size - 1 - m));
            result[s][i] = (c_prime[i] * ai[s]) - r_hat[s][i] + (b[s][i] << T(ring_size - m - 1));
        }
    }

    delete[] ai;
    delete[] c;
    delete[] c_prime;

    for (size_t i = 0; i < numShares; i++) {
        delete[] edaBit_r[i];
        delete[] sum[i];
        delete[] b[i];
        delete[] r_hat[i];
        delete[] b_km1[i];
        delete[] b_2[i];
    }
    delete[] edaBit_r;
    delete[] b;
    delete[] sum;
    delete[] r_hat;
    delete[] b_km1;
    delete[] b_2;
}

// trunation of all data by DIFFERENT m's
template <typename T>
void doOperation_Trunc(T **result, T **input, int K, int *m, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
}

#endif // _TRUNC_HPP_