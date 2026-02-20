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
#ifndef _SDIV_HPP_
#define _SDIV_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "Mult.hpp"
#include "Trunc.hpp"

// SDiv: Computes Y = A / B using Newton-Raphson iteration
// Based on the Shamir implementation in SDiv.cpp
// All arrays use interface format [size][numShares]
// K is the bit length of the operands
template <typename T>
void doOperation_SDiv(T **Y, T **A, T **B, int K, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    // Number of Newton-Raphson iterations
    int theta = (int)ceil(log2((double)K));

    // 2^(K+1)
    T const2K1 = T(1) << (K + 1);

    // Allocate arrays in [size][numShares] format
    T **X = new T *[size];
    T **temp = new T *[2 * size];
    T **XY = new T *[2 * size];

    for (int i = 0; i < size; i++) {
        X[i] = new T[numShares];
        memset(X[i], 0, sizeof(T) * numShares);
    }

    for (int i = 0; i < 2 * size; i++) {
        temp[i] = new T[numShares];
        memset(temp[i], 0, sizeof(T) * numShares);
        XY[i] = new T[numShares];
        memset(XY[i], 0, sizeof(T) * numShares);
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, T(1));

    // Initialize: Y = A, X = B
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            Y[i][s] = A[i][s];
            X[i][s] = B[i][s];
            // Set up XY for batched multiplication: XY[0..size-1] = Y, XY[size..2*size-1] = X
            XY[i][s] = Y[i][s];
            XY[size + i][s] = X[i][s];
        }
    }

    // Newton-Raphson iterations
    for (int iter = 1; iter <= theta - 1; iter++) {
        // Compute temp = 2^(K+1) - X for both Y and X multiplications
        for (int i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                temp[i][s] = ai[s] * const2K1 - XY[size + i][s];  // For Y * (2^(K+1) - X)
                temp[size + i][s] = temp[i][s];                    // Same for X * (2^(K+1) - X)
            }
        }

        // Batch multiply: XY[0..size-1] = Y * temp, XY[size..2*size-1] = X * temp
        Mult(XY, XY, temp, 2 * size, threadID, net, ss);

        // Truncate both results
        doOperation_Trunc(XY, XY, 2 * K + 1, K, 2 * size, threadID, net, ss);

        // Update X from the second half of XY
        for (int i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                X[i][s] = XY[size + i][s];
            }
        }
    }

    // Final iteration: Y = Y * (2^(K+1) - X)
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            temp[i][s] = ai[s] * const2K1 - XY[size + i][s];
        }
    }

    Mult(Y, XY, temp, size, threadID, net, ss);
    doOperation_Trunc(Y, Y, 2 * K + 1, K, size, threadID, net, ss);

    // Cleanup
    for (int i = 0; i < size; i++) {
        delete[] X[i];
    }
    for (int i = 0; i < 2 * size; i++) {
        delete[] temp[i];
        delete[] XY[i];
    }
    delete[] X;
    delete[] temp;
    delete[] XY;
    delete[] ai;
}

#endif // _SDIV_HPP_
