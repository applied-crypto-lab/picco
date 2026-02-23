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
    T **diff = new T *[size];
    for (int i = 0; i < size; i++) {
        diff[i] = new T[numShares];
    }

    for (int i = 0; i < size; i++)
        for (uint s = 0; s < numShares; s++)
            diff[i][s] = a[i][s] - b[i][s]; // computing the difference of a and b

    Rss_MSB(result, diff, size, ring_size, nodeNet, ss);

    for (int i = 0; i < size; i++) {
        delete[] diff[i];
    }

    delete[] diff;
}

template <typename T> // a pub/ b priv
void doOperation_LT(T **result, int *a, T **b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;
    T **diff = new T *[size];
    for (int i = 0; i < size; i++) {
        diff[i] = new T[numShares];
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, 1);

    for (int i = 0; i < size; i++)
        for (uint s = 0; s < numShares; s++)
            diff[i][s] = T(a[i]) * ai[s] - b[i][s]; // computing the difference of a and b

    Rss_MSB(result, diff, size, ring_size, nodeNet, ss);

    for (int i = 0; i < size; i++) {
        delete[] diff[i];
    }

    delete[] diff;
    delete[] ai;
}

template <typename T> // a priv , b pub
void doOperation_LT(T **result, T **a, int *b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;
    T **diff = new T *[size];
    for (int i = 0; i < size; i++) {
        diff[i] = new T[numShares];
    }
    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, 1);

    for (int i = 0; i < size; i++)
        for (uint s = 0; s < numShares; s++)
            diff[i][s] = a[i][s] - T(b[i]) * ai[s]; // computing the difference of a and b

    Rss_MSB(result, diff, size, ring_size, nodeNet, ss);

    for (int i = 0; i < size; i++) {
        delete[] diff[i];
    }

    delete[] diff;
    delete[] ai;
}

// Scalar wrapper functions for RSS - converts priv_int& to priv_int**
template <typename T>
void doOperation_LT(priv_int &result, priv_int &a, priv_int &b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    doOperation_LT(&result, &a, &b, alen, blen, resultlen, size, threadID, nodeNet, ss);
}

template <typename T>
void doOperation_LT(priv_int &result, priv_int &a, int *b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    doOperation_LT(&result, &a, b, alen, blen, resultlen, size, threadID, nodeNet, ss);
}

template <typename T>
void doOperation_LT(priv_int &result, int *a, priv_int &b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    doOperation_LT(&result, a, &b, alen, blen, resultlen, size, threadID, nodeNet, ss);
}

#endif // _LT_HPP_