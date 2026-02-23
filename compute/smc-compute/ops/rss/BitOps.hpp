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

#ifndef _BITOPS_HPP_
#define _BITOPS_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "Mult.hpp"

// ============================================================================
// Single-bit implementations only (matching original Shamir behavior)
// These work correctly for 1-bit inputs only.
// For multi-bit inputs, results will be incorrect.
// ============================================================================

// Logical AND: result = A * B (works for single bits)
template <typename T>
void LogicalAnd(T **A, T **B, T **result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    Mult(result, A, B, size, threadID, net, ss);
}

// Logical OR: result = A + B - A*B (works for single bits)
template <typename T>
void LogicalOr(T **A, T **B, T **result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    T **C = new T*[size];
    for (int i = 0; i < size; i++) {
        C[i] = new T[numShares];
    }

    // C = A * B
    Mult(C, A, B, size, threadID, net, ss);

    // result = A + B - C
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            result[i][s] = A[i][s] + B[i][s] - C[i][s];
        }
    }

    for (int i = 0; i < size; i++) {
        delete[] C[i];
    }
    delete[] C;
}

// Bitwise AND with public value
template <typename T>
void BitAnd(T **A, int *B, T **result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    ss->modMul(result, A, B, size);
}

// Bitwise AND: result = A * B (works for single bits)
template <typename T>
void BitAnd(T **A, T **B, T **result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    Mult(result, A, B, size, threadID, net, ss);
}

// Bitwise OR with public value
template <typename T>
void BitOr(T **A, int *B, T **result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);

    for (int i = 0; i < size; i++) {
        ss->sparsify_public(ai, T(B[i]));
        for (uint s = 0; s < numShares; s++) {
            result[i][s] = A[i][s] + ai[s] - A[i][s] * T(B[i]);
        }
    }
    delete[] ai;
}

// Bitwise OR: result = A + B - A*B (works for single bits)
template <typename T>
void BitOr(T **A, T **B, T **result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    T **C = new T*[size];
    for (int i = 0; i < size; i++) {
        C[i] = new T[numShares];
    }

    Mult(C, A, B, size, threadID, net, ss);

    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            result[i][s] = A[i][s] + B[i][s] - C[i][s];
        }
    }

    for (int i = 0; i < size; i++) {
        delete[] C[i];
    }
    delete[] C;
}

// Bitwise XOR with public value
template <typename T>
void BitXor(T **A, int *B, T **result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    // XOR with public: just XOR one share
    for (int i = 0; i < size; i++) {
        result[i][0] = A[i][0] ^ T(B[i]);
        for (uint s = 1; s < numShares; s++) {
            result[i][s] = A[i][s];
        }
    }
}

// Bitwise XOR: result = A + B - 2*A*B (works for single bits only)
template <typename T>
void BitXor(T **A, T **B, T **result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    T **C = new T*[size];
    for (int i = 0; i < size; i++) {
        C[i] = new T[numShares];
    }

    Mult(C, A, B, size, threadID, net, ss);

    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            result[i][s] = A[i][s] + B[i][s] - T(2) * C[i][s];
        }
    }

    for (int i = 0; i < size; i++) {
        delete[] C[i];
    }
    delete[] C;
}

// ============================================================================
// Scalar (1D array) wrappers - for single element operations
// These wrap 1D arrays (T*) into 2D format and call the main functions
// ============================================================================

template <typename T>
void BitAnd(T *a, T *b, T *result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    BitAnd(&a, &b, &result, alen, blen, resultlen, size, threadID, net, ss);
}

template <typename T>
void BitOr(T *a, T *b, T *result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    BitOr(&a, &b, &result, alen, blen, resultlen, size, threadID, net, ss);
}

template <typename T>
void BitXor(T *a, T *b, T *result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    BitXor(&a, &b, &result, alen, blen, resultlen, size, threadID, net, ss);
}

template <typename T>
void LogicalAnd(T *a, T *b, T *result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    LogicalAnd(&a, &b, &result, alen, blen, resultlen, size, threadID, net, ss);
}

template <typename T>
void LogicalOr(T *a, T *b, T *result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    LogicalOr(&a, &b, &result, alen, blen, resultlen, size, threadID, net, ss);
}

#endif // _BITOPS_HPP_
