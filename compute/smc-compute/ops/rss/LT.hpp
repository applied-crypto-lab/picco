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

#ifndef _LT_HPP_
#define _LT_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "MSB.hpp"

// evaluates the boolean expression (a <? b) and returns a bit (not the larger/smaller of the two)
template <typename T> // a priv/ b priv -> all for int
void doOperation_LT(T **result, T **a, T **b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;
    T **diff = new T *[numShares];
    for (size_t i = 0; i < numShares; i++) {
        diff[i] = new T[size];
    }

    for (size_t s = 0; s < numShares; s++)
        for (size_t i = 0; i < size; i++)
            diff[s][i] = a[s][i] - b[s][i]; // compinting the difference of a and b

    Rss_MSB(result, diff, size, ring_size, nodeNet, ss);

    for (size_t i = 0; i < numShares; i++) {
        delete[] diff[i];
    }

    delete[] diff;
}

template <typename T> // a pub/ b priv
void doOperation_LT(T **result, int *a, T **b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;
    T **diff = new T *[numShares];
    for (size_t i = 0; i < numShares; i++) {
        diff[i] = new T[size];
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, 1);

    for (size_t s = 0; s < numShares; s++)
        for (size_t i = 0; i < size; i++)
            diff[s][i] = T(a[s]) * ai[s] - b[s][i]; // compinting the difference of a and b

    Rss_MSB(result, diff, size, ring_size, nodeNet, ss);

    for (size_t i = 0; i < numShares; i++) {
        delete[] diff[i];
    }

    delete[] diff;
    delete[] ai;
}

template <typename T> // a priv , b pub
void doOperation_LT(T **result, T **a, int *b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;
    T **diff = new T *[numShares];
    for (size_t i = 0; i < numShares; i++) {
        diff[i] = new T[size];
    }
    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, 1);

    for (size_t s = 0; s < numShares; s++)
        for (size_t i = 0; i < size; i++)
            diff[s][i] = a[s][i] - T(b[i]) * ai[s]; // compinting the difference of a and b

    Rss_MSB(result, diff, size, ring_size, nodeNet, ss);

    for (size_t i = 0; i < numShares; i++) {
        delete[] diff[i];
    }

    delete[] diff;
    delete[] ai;
}

#endif // _LT_HPP_