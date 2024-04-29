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
void doOperation_Pow2(T **result, T **a, int L, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;
    int m = ceil(log2(L));

    T **a_bits = new T *[numShares];
    T **prods = new T *[numShares];

    for (size_t i = 0; i < numShares; i++) {
        prods[i] = new T[size * m];
        memset(prods[i], 0, sizeof(T) * size * m);
        a_bits[i] = new T[size];
        memset(a_bits[i], 0, sizeof(T) * size);
    }

    // we have to bit decompose all of a's bits (can't do a subset, right?)
    Rss_BitDec(a_bits, a, size, ring_size, net, ss);
    // a_bits consists of all the bits of [a] packed into a single value (as it normaly is)

    T bit;
    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            for (size_t j = 0; j < m; j++) {
                bit = GET_BIT(a_bits[s][i], T(j));
                prods[s][i * m + j] = (T(1) << (T(1) << T(i))) * bit + T(1) - bit;
            }
        }
    }

    for (size_t i = 0; i < numShares; i++) {
        delete[] a_bits[i];
        delete[] prods[i];
    }
    delete[] a_bits;
    delete[] prods;
}

#endif // _POW2_HPP_