
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
#ifndef _APPRCR_HPP_
#define _APPRCR_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "Mult.hpp"
#include "Norm.hpp"
#include "Trunc.hpp"

template <typename T>
void doOperation_IntAppRcr(T **w, T **b, int bitlength, int size, uint ring_size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    assertm(ring_size > 2 * bitlength, "The ring size must be at least 2*bitlength");

    static uint numShares = ss->getNumShares();
    // long double alpha = (2.9142 * (long double)(1 << bitlength)); // check this
    T alpha = T(2.9142 * (1 << bitlength)); // check this

    T **c = new T *[numShares];
    T **v = new T *[numShares];
    T **d = new T *[numShares];
    for (size_t i = 0; i < numShares; i++) {
        c[i] = new T[size];
        memset(c[i], 0, sizeof(T) * size);
        v[i] = new T[size];
        memset(v[i], 0, sizeof(T) * size);
        d[i] = new T[size];
        memset(d[i], 0, sizeof(T) * size);
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, T(1));

    // check that v is well-formed when b is negative
    doOperation_Norm(c, v, b, bitlength, size, ring_size, threadID, net, ss);

    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            d[s][i] = (ai[s] * alpha) - 2 * c[s][i];
        }
    }

    Mult(w, d, v, size, threadID, net, ss);

    // check if we need to modify anything for bitlength < k?
    // the "K" argument is unused by RSS
    doOperation_Trunc(w, w, bitlength, bitlength, size, threadID, net, ss);

    for (size_t i = 0; i < numShares; i++) {
        delete[] c[i];
        delete[] d[i];
        delete[] v[i];
    }

    delete[] c;
    delete[] d;
    delete[] v;
    delete[] ai;
}

#endif // _APPRCR_HPP_