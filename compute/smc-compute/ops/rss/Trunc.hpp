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
#include "B2A.hpp"
#include "BitLT.hpp"
#include "EdaBit.hpp"

// Single-element wrapper for truncation
template <typename T>
void doOperation_Trunc(T *result, T *input, int K, int m, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // Single-element wrapper - converts to batch format and calls the main function
    // Note: size is expected to be 1 for this wrapper
    uint numShares = ss->getNumShares();

    // Create temporary 2D arrays for batch interface [size][numShares] where size=1
    T **result_2d = new T*[1];
    T **input_2d = new T*[1];
    result_2d[0] = new T[numShares];
    input_2d[0] = new T[numShares];

    // Copy input to batch format
    for (uint s = 0; s < numShares; s++) {
        input_2d[0][s] = input[s];
        result_2d[0][s] = 0;
    }

    // Call batch version with size=1
    doOperation_Trunc(result_2d, input_2d, K, m, 1, threadID, nodeNet, ss);

    // Copy result back
    for (uint s = 0; s < numShares; s++) {
        result[s] = result_2d[0][s];
    }

    // Cleanup
    delete[] result_2d[0];
    delete[] input_2d[0];
    delete[] result_2d;
    delete[] input_2d;
}

// trunation of all data by a single M
// this protocol requires that the bitlength of the input is at least one bit shorter than the ring size, i.e. MSB(input) = 0
// All arrays use interface format [size][numShares]
template <typename T>
void doOperation_Trunc(T **result, T **input, int K, int m, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;

    // shift by 0 is identity
    if (m == 0) {
        for (int i = 0; i < size; i++)
            for (uint s = 0; s < numShares; s++)
                result[i][s] = input[i][s];
        return;
    }

    // All arrays use interface format [size][numShares]
    T **edaBit_r = new T *[size];
    T **sum = new T *[size];
    T **r_hat = new T *[size];
    T **b = new T *[size];
    T **b_km1 = new T *[size];
    T **b_2 = new T *[size];
    T **u_2 = new T *[size];

    T *c = new T[size];
    memset(c, 0, sizeof(T) * size);
    T *c_prime = new T[size];
    memset(c_prime, 0, sizeof(T) * size);

    for (int i = 0; i < size; i++) {
        edaBit_r[i] = new T[numShares];
        memset(edaBit_r[i], 0, sizeof(T) * numShares);
        r_hat[i] = new T[numShares];
        memset(r_hat[i], 0, sizeof(T) * numShares);

        b_km1[i] = new T[numShares];
        memset(b_km1[i], 0, sizeof(T) * numShares);
        b_2[i] = new T[numShares];
        memset(b_2[i], 0, sizeof(T) * numShares);
        sum[i] = new T[numShares];
        memset(sum[i], 0, sizeof(T) * numShares);

        b[i] = new T[numShares];
        memset(b[i], 0, sizeof(T) * numShares);
        u_2[i] = new T[numShares];
        memset(u_2[i], 0, sizeof(T) * numShares);
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, 1);

    edaBit_Trunc(edaBit_r, r_hat, b_2, b_km1, m, size, ring_size, nodeNet, ss);

    // computing the sum of input and edabit_r
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            sum[i][s] = (input[i][s] + edaBit_r[i][s]);
        }
    }

    Open(c, sum, size, threadID, nodeNet, ss);

    // (c / 2^m) mod 2^(k-m-1)
    for (int i = 0; i < size; i++) {
        c_prime[i] = (c[i] >> T(m)) & ss->SHIFT[ring_size - m - 1];
    }

    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            b[i][s] = b_km1[i][s] + ((c[i] * ai[s]) >> T(ring_size - 1)) - 2 * ((c[i]) >> T(ring_size - 1)) * b_km1[i][s];
            r_hat[i][s] = r_hat[i][s] - (b_km1[i][s] << T(ring_size - 1 - m));
            result[i][s] = (c_prime[i] * ai[s]) - r_hat[i][s] + (b[i][s] << T(ring_size - m - 1));
        }
    }

    // the following block of code converts a probabilistic trunation to deterministic (exact)
    // in order to preserve security (see Li et al., "Efficient 3PC for Binary Circuits with Application to Maliciously-Secure DNN Inference", USENIX 2023)
    Rss_BitLT(u_2, c, b_2, size, m, nodeNet, ss);
    Rss_B2A(u_2, u_2, size, ring_size, nodeNet, ss);
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            result[i][s] = (result[i][s] - u_2[i][s]);
        }
    }

    delete[] ai;
    delete[] c;
    delete[] c_prime;

    for (int i = 0; i < size; i++) {
        delete[] edaBit_r[i];
        delete[] sum[i];
        delete[] b[i];
        delete[] r_hat[i];
        delete[] b_km1[i];
        delete[] b_2[i];
        delete[] u_2[i];
    }
    delete[] u_2;
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
