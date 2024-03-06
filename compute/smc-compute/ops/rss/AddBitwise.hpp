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
#ifndef _ADDBITWISE_HPP_
#define _ADDBITWISE_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "Mult.hpp"
#include <cmath>


template <typename T>
void Rss_BitAdd(T **res, T **a, T **b, uint ring_size, uint size, NodeNetwork *nodeNet, replicatedSecretShare<T> *ss) {

    uint numShares = ss->getNumShares();

    T **d = new T *[2 * numShares];
    for (size_t i = 0; i < 2 * numShares; i++) {
        d[i] = new T[size];
        memset(d[i], 0, sizeof(T) * size);
    }

    Rss_Mult_Bitwise(res, a, b, size, ring_size, nodeNet, ss);

    for (size_t i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            d[s][i] = a[s][i] ^ b[s][i];
            // d[1][i] = a[1][i] ^ b[1][i];

            d[numShares + s][i] = res[s][i];
            // d[3][i] = res[1][i];
        }
    }
    Rss_CircleOpL(d, ring_size, size, nodeNet); // original

    for (size_t i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            res[s][i] = (a[s][i] ^ b[s][i]) ^ (d[numShares + s][i] << T(1));
    }
    for (size_t i = 0; i < 2 * numShares; i++) {
        delete[] d[i];
    }
    delete[] d;
}


#endif
