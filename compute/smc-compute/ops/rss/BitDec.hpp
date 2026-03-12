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
#include "AddBitwise.hpp"
#include "B2A.hpp"
#include "EdaBit.hpp"
#include "Open.hpp"

// IMPORTANT !!!!!!!!!
// the result is returned as bitwise shares packed into a single Lint (res)
// would additionally need to call B2A (After unpacking the shares) to get results over Z2k
// not implementing the functionality explicitly, since theres only a few floating point protocols where it's used
// better to do the allocations/B2A there
// All arrays use interface format [size][numShares]
template <typename T>
void Rss_BitDec(T **res, T **a, uint bitlength, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint numShares = ss->getNumShares();

    T *c = new T[size];

    // edaBit uses interface format [size][numShares]
    T **edaBit_r = new T *[size];
    T **edaBit_b_2 = new T *[size];

    for (size_t i = 0; i < size; i++) {
        edaBit_r[i] = new T[numShares];
        edaBit_b_2[i] = new T[numShares];
        memset(edaBit_r[i], 0, sizeof(T) * numShares);
        memset(edaBit_b_2[i], 0, sizeof(T) * numShares);
    }

    // only need ring_size bit-length values for both parts of computation
    // first protection and b2a
    edaBit(edaBit_r, edaBit_b_2, bitlength, size, ring_size, nodeNet, ss);

    // Compute sum = a - edaBit_r and store in interface format [size][numShares] for Open
    // Note: both a and edaBit_r are now in interface format [size][numShares]
    T **sum = new T *[size];
    for (size_t i = 0; i < size; i++) {
        sum[i] = new T[numShares];
        for (size_t s = 0; s < numShares; s++) {
            sum[i][s] = (a[i][s] - edaBit_r[i][s]);
        }
    }

    Open(c, sum, size, -1, nodeNet, ss);

    // Free sum
    for (size_t i = 0; i < size; i++) {
        delete[] sum[i];
    }
    delete[] sum;

    // edaBit_b_2 is already in interface format [size][numShares], use directly
    Rss_BitAdd(res, c, edaBit_b_2, size, ring_size, nodeNet, ss);

    delete[] c;

    for (size_t i = 0; i < size; i++) {
        delete[] edaBit_r[i];
        delete[] edaBit_b_2[i];
    }
    delete[] edaBit_r;
    delete[] edaBit_b_2;
}

