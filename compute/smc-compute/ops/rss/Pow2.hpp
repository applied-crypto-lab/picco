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

template <typename T>
void doOperation_Pow2(T **result, T **a, int L, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;
    uint m = ceil(log2(L)); // rounding up to the nearest integer
    uint numPows = size * m;

    T **a_bits = new T *[numShares];
    T **prods = new T *[numShares];
    for (size_t i = 0; i < numShares; i++) {
        prods[i] = new T[numPows];
        memset(prods[i], 0, sizeof(T) * numPows);
        a_bits[i] = new T[size];
        memset(a_bits[i], 0, sizeof(T) * size);
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, T(1));


    vector<T> pows(m, T(0));
    // computing all of the 2^2^j powers once
    for (size_t j = 0; j < m; j++) {
        pows[j] = T(1) << (T(1) << T(j));
    }

    // we have to bit decompose all of a's bits (can't do a subset, right?)
    // need to check if we can do bitDec on the m LSB's of a (need to add new argument to BitDec)
    Rss_BitDec(a_bits, a, m, size, ring_size, nodeNet, ss);
    // a_bits consists of all the bits of [a] packed into a single value (as it normaly is)

    // extracting all the individual bits of a_bits
    for (size_t s = 0; s < numShares; s++) {
        for (size_t j = 0; j < m; j++) {
            for (size_t i = 0; i < size; i++) {
                prods[s][j * size + i] = GET_BIT(a_bits[s][i], T(j));
            }
        }
    }
    // reusing prods
    Rss_B2A(prods, prods, numPows, ring_size, nodeNet, ss);

    size_t p_idx;
    for (size_t s = 0; s < numShares; s++) {
        for (size_t j = 0; j < m; j++) {
            for (size_t i = 0; i < size; i++) {
                p_idx = j * size + i;
                prods[s][p_idx] = pows[j] * prods[s][p_idx] + ai[s] - prods[s][p_idx];
            }
        }
    }
    uint new_m;

    // allocating buffers and moving prods into the buffers
    T **A_buff = new T *[numShares];
    T **B_buff = new T *[numShares];
    T **C_buff = new T *[numShares];
    for (size_t s = 0; s < numShares; s++) {
        A_buff[s] = new T[numPows >> 1]; // only need half of numPows to start
        B_buff[s] = new T[numPows >> 1];
        C_buff[s] = new T[numPows >> 1];
    }

    // performing the multiplication as a tree
    while (m > 1) {
        new_m = m >> 1;         // m/2 (rounded down)
        numPows = size * new_m; // updating numPows

        for (size_t s = 0; s < numShares; s++) {
            memcpy(A_buff[s], prods[s], sizeof(T) * numPows);
            memcpy(B_buff[s], prods[s] + numPows, sizeof(T) * numPows);
        }
        // performing mult, stores the result in prods
        Mult(prods, A_buff, B_buff, numPows, nodeNet, ss);

        // if there was an odd number of inputs, move it into the new_m'th position in the buffers for the next round
        if (m & 1) { // if m is ODD
            // printf("m is ODD\n");
            for (size_t s = 0; s < numShares; s++) {
                memcpy(prods[s] + numPows, prods[s] + 2 * numPows, sizeof(T) * size);
            }

            new_m += 1;
        }
        m = new_m;
    }

    // moving the result of the tree-mult into res
    for (size_t s = 0; s < numShares; s++) {
        memcpy(result[s], prods[s], sizeof(T) * size);
    }

    delete[] ai;
    for (size_t i = 0; i < numShares; i++) {
        delete[] A_buff[i];
        delete[] B_buff[i];
        delete[] a_bits[i];
        delete[] prods[i];
    }
    delete[] A_buff;
    delete[] B_buff;
    delete[] a_bits;
    delete[] prods;
}

#endif // _POW2_HPP_