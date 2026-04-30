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
#ifndef _POW2_HPP_
#define _POW2_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "BitDec.hpp"
#include "Mult.hpp"

// All arrays use interface format: [size][numShares] where array[i][s] is share s of element i

template <typename T>
void doOperation_Pow2(T **result, T **a, int L, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;
    uint m = ceil(log2(L + 1)); // +1 so the value L itself is representable (not just 0..L-1)
    uint numPows = size * m;

    // Allocate a_bits in [size][numShares] format for BitDec
    T **a_bits = new T *[size];
    for (int i = 0; i < size; i++) {
        a_bits[i] = new T[numShares];
        memset(a_bits[i], 0, sizeof(T) * numShares);
    }

    // prods uses [numPows][numShares] format - consistent with interface format
    T **prods = new T *[numPows];
    for (size_t i = 0; i < numPows; i++) {
        prods[i] = new T[numShares];
        memset(prods[i], 0, sizeof(T) * numShares);
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, T(1));

    vector<T> pows(m, T(0));
    // computing all of the 2^2^j powers once
    for (size_t j = 0; j < m; j++) {
        pows[j] = T(1) << (T(1) << T(j));
    }

    // BitDec expects [size][numShares] format - both input (a) and output (a_bits)
    Rss_BitDec(a_bits, a, m, size, ring_size, nodeNet, ss);
    // a_bits[i][s] now contains the packed bits for element i, share s

    // extracting all the individual bits of a_bits and storing in prods
    // prods[j * size + i][s] = bit j of a_bits[i][s]
    for (size_t j = 0; j < m; j++) {
        for (int i = 0; i < size; i++) {
            size_t p_idx = j * size + i;
            for (size_t s = 0; s < numShares; s++) {
                prods[p_idx][s] = GET_BIT(a_bits[i][s], T(j));
            }
        }
    }

    // B2A expects [size][numShares] format - prods is already in this format
    Rss_B2A(prods, prods, numPows, ring_size, nodeNet, ss);

    // Apply transformation: prods[idx][s] = pows[j] * prods[idx][s] + ai[s] - prods[idx][s]
    for (size_t j = 0; j < m; j++) {
        for (int i = 0; i < size; i++) {
            size_t p_idx = j * size + i;
            for (size_t s = 0; s < numShares; s++) {
                prods[p_idx][s] = pows[j] * prods[p_idx][s] + ai[s] - prods[p_idx][s];
            }
        }
    }

    uint new_m;
    uint max_numPows = numPows; // save original numPows for buffer allocation

    // performing the multiplication as a tree
    while (m > 1) {
        new_m = m >> 1;         // m/2 (rounded down)
        numPows = size * new_m; // updating numPows

        // Mult expects [size][numShares] format
        // A = prods[0..numPows-1], B = prods[numPows..2*numPows-1]
        // Store result in prods[0..numPows-1]
        Mult(prods, prods, prods + numPows, numPows, nodeNet, ss);

        // if there was an odd number of inputs, move it into the new_m'th position
        if (m & 1) { // if m is ODD
            // Move prods[2*numPows..2*numPows+size-1] to prods[numPows..numPows+size-1]
            for (int i = 0; i < size; i++) {
                for (size_t s = 0; s < numShares; s++) {
                    prods[numPows + i][s] = prods[2 * numPows + i][s];
                }
            }
            new_m += 1;
        }
        m = new_m;
    }

    // Copy result from prods[0..size-1] to result[0..size-1]
    // Both are in [size][numShares] format
    for (int i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            result[i][s] = prods[i][s];
        }
    }

    // Clean up
    delete[] ai;
    for (int i = 0; i < size; i++) {
        delete[] a_bits[i];
    }
    delete[] a_bits;

    for (size_t i = 0; i < max_numPows; i++) {
        delete[] prods[i];
    }
    delete[] prods;
}

// ============================================================================
// Scalar wrappers for RSS (following Mult.hpp pattern)
// ============================================================================

template <typename T>
void doOperation_Pow2(priv_int &result, priv_int &a, int L, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    doOperation_Pow2(&result, &a, L, size, threadID, nodeNet, ss);
}

template <typename T>
void doOperation_Pow2(priv_int &result, priv_int &a, int L, int size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    doOperation_Pow2(&result, &a, L, size, nodeNet, ss);
}

#endif // _POW2_HPP_
