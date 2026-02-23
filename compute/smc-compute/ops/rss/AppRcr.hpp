
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
#pragma once

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "Mult.hpp"
#include "Norm.hpp"
#include "Trunc.hpp"

template <typename T>
void doOperation_IntAppRcr(T **w, T **b, int bitlength, int size, uint ring_size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // assertm(ring_size > 2 * bitlength, "The ring size must be at least 2*bitlength");
    // NOTE: Interface format is [size][numShares] where array[i][s] is share s of element i

    uint numShares = ss->getNumShares();
    // Use T(1) << bitlength to avoid 32-bit integer overflow
    T alpha = T((double)2.9142 * (double)(T(1) << bitlength));

    // All arrays use interface format [size][numShares]
    T **c = new T *[size];
    T **v = new T *[size];
    T **d = new T *[size];
    for (int i = 0; i < size; i++) {
        c[i] = new T[numShares];
        memset(c[i], 0, sizeof(T) * numShares);
        v[i] = new T[numShares];
        memset(v[i], 0, sizeof(T) * numShares);
        d[i] = new T[numShares];
        memset(d[i], 0, sizeof(T) * numShares);
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, T(1));

    doOperation_Norm(c, v, b, bitlength, size, ring_size, threadID, net, ss);

    // d = alpha - 2*c
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            d[i][s] = (ai[s] * alpha) - T(2) * c[i][s];
        }
    }

    // w = d * v
    Mult(w, d, v, size, threadID, net, ss);

    doOperation_Trunc(w, w, bitlength, bitlength, size, threadID, net, ss);

    for (int i = 0; i < size; i++) {
        delete[] c[i];
        delete[] d[i];
        delete[] v[i];
    }

    delete[] c;
    delete[] d;
    delete[] v;
    delete[] ai;
}

