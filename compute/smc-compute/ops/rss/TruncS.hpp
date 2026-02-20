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

#ifndef _TRUNCS_HPP_
#define _TRUNCS_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "LT.hpp"
#include "Mult.hpp"
#include "Pow2.hpp"
#include "Trunc.hpp"

// ============================================================================
// TruncS - Truncation by secret amount
// Computes result = A >> m = floor(A / 2^m) where m is secret
//
// This is a modification of Dalskov et al.'s private truncation protocol:
// 1. Compute b = LT(m, bitlength) - check if m < bitlength
// 2. Compute 2^(bitlength - m)
// 3. Multiply A * 2^(bitlength - m)
// 4. Truncate by bitlength bits (public truncation)
// 5. If m >= bitlength (b=0), result is 0; otherwise result is the truncation
// ============================================================================

template <typename T>
void doOperation_TruncS(T **result, T **A, int bitlength, T **m, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, T(1));

    // Allocate arrays with correct dimensions [size][numShares]
    T **b = new T *[size];
    T **diff = new T *[size];
    T **pow2_diff = new T *[size];
    T **temp = new T *[size];
    for (int i = 0; i < size; i++) {
        b[i] = new T[numShares];
        diff[i] = new T[numShares];
        pow2_diff[i] = new T[numShares];
        temp[i] = new T[numShares];
        memset(b[i], 0, sizeof(T) * numShares);
        memset(diff[i], 0, sizeof(T) * numShares);
        memset(pow2_diff[i], 0, sizeof(T) * numShares);
        memset(temp[i], 0, sizeof(T) * numShares);
    }

    // Create bitlength array for LT interface
    T **bitlen_arr = new T *[size];
    for (int i = 0; i < size; i++) {
        bitlen_arr[i] = new T[numShares];
        for (uint sh = 0; sh < numShares; sh++) {
            bitlen_arr[i][sh] = T(bitlength) * ai[sh];
        }
    }

    // Step 1: b = LT(m, bitlength) = 1 if m < bitlength
    // Using the comparison: LT(m, bitlength)
    doOperation_LT(b, m, bitlen_arr, ring_size, ring_size, 1, size, threadID, nodeNet, ss);

    // Step 2: Compute diff = bitlength - m
    for (int i = 0; i < size; i++) {
        for (uint sh = 0; sh < numShares; sh++) {
            diff[i][sh] = T(bitlength) * ai[sh] - m[i][sh];
        }
    }

    // Step 3: Compute 2^(bitlength - m)
    doOperation_Pow2(pow2_diff, diff, bitlength, size, threadID, nodeNet, ss);

    // Step 4: Compute A * 2^(bitlength - m)
    Mult(temp, A, pow2_diff, size, threadID, nodeNet, ss);

    // Step 5: Truncate by bitlength bits (public truncation)
    // This gives floor(A * 2^(bitlength - m) / 2^bitlength) = floor(A / 2^m)
    doOperation_Trunc(temp, temp, 2 * bitlength, bitlength, size, threadID, nodeNet, ss);

    // Step 6: If b = 0 (m >= bitlength), result is 0
    // Otherwise, result is temp
    Mult(result, b, temp, size, threadID, nodeNet, ss);

    // Cleanup
    delete[] ai;

    for (int i = 0; i < size; i++) {
        delete[] b[i];
        delete[] diff[i];
        delete[] pow2_diff[i];
        delete[] temp[i];
        delete[] bitlen_arr[i];
    }
    delete[] b;
    delete[] diff;
    delete[] pow2_diff;
    delete[] temp;
    delete[] bitlen_arr;
}

// ============================================================================
// Scalar wrappers for RSS (following Mult.hpp pattern)
// ============================================================================

template <typename T>
void doOperation_TruncS(priv_int &result, priv_int &A, int bitlength, priv_int &m, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    doOperation_TruncS(&result, &A, bitlength, &m, size, threadID, nodeNet, ss);
}

template <typename T>
void doOperation_TruncS(priv_int &result, priv_int &A, int bitlength, priv_int &m, int size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    doOperation_TruncS(&result, &A, bitlength, &m, size, nodeNet, ss);
}

#endif // _TRUNCS_HPP_
