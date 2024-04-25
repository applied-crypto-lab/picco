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
#ifndef _MSB_HPP_
#define _MSB_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "BitLT.hpp"
#include "EdaBit.hpp"
#include "RandBit.hpp"

template <typename T>
void Rss_MSB(T **res, T **a, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    assertm((ring_size == ss->ring_size ) , "checking ring_size argument == ss->ring_size");

    uint numShares = ss->getNumShares();
    uint i; // used for loops

    // only need to generate a single random bit per private value
    T **b = new T *[numShares];
    T **sum = new T *[numShares];
    T **u_2 = new T *[numShares];
    T **edaBit_r = new T *[numShares];
    T **edaBit_b_2 = new T *[numShares];
    T **rprime = new T *[numShares];

    T *c = new T[size];
    T *e = new T[size];

    for (i = 0; i < numShares; i++) {
        b[i] = new T[size];
        edaBit_r[i] = new T[size];
        edaBit_b_2[i] = new T[size];
        sum[i] = new T[size];
        u_2[i] = new T[size];

        rprime[i] = new T[size];
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, 1);

    // stays the same
    Rss_RandBit(b, size, ring_size, nodeNet, ss);

    // generating a full-sized edaBit
    edaBit(edaBit_r, edaBit_b_2, ring_size, size, ring_size, nodeNet, ss);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            rprime[s][i] = edaBit_r[s][i] - (GET_BIT(edaBit_b_2[s][i], T(ring_size - 1)) << T(ring_size - 1));
            // rprime[1][i] = edaBit_r[1][i] - (GET_BIT(edaBit_b_2[1][i], T(ring_size - 1)) << T(ring_size - 1));
            // combining w the next loop
            // combining w the previous loop
            sum[s][i] = (a[s][i] + edaBit_r[s][i]);
            // sum[1][i] = (a[1][i] + edaBit_r[1][i]);
            // sum[0][i] = (a[0][i] + edaBit_r[0][i]) << T(1);
            // sum[1][i] = (a[1][i] + edaBit_r[1][i]) << T(1);
        }
    }

    Open(c, sum, size, -1, nodeNet, ss);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            edaBit_b_2[s][i] = edaBit_b_2[s][i] & ss->SHIFT[ring_size - 1];

        // used for alternate solution
        // prevents us from using it directly later on
        // edaBit_b_2[1][i] = edaBit_b_2[1][i] & ss->SHIFT[ring_size - 1];

        // definitely needed
        c[i] = c[i] & ss->SHIFT[ring_size - 1];
        // c[i] = c[i] >> T(1);
    }

    // Rss_Open_Bitwise(r_2_open, edaBit_b_2, size, ring_size, nodeNet, ss);
    // // this part is still correct
    // however, the edaBit_b_2 shares do get modified
    // which may not be desierable
    Rss_BitLT(u_2, c, edaBit_b_2, size, ring_size, nodeNet, ss);

    for (i = 0; i < size; ++i) {
        // cant do this because we modify edaBit_b_2 earlier
        // for (size_t s = 0; s < numShares; s++)
        for (size_t s = 0; s < numShares; s++)
            sum[s][i] = a[s][i] - c[i] * ai[s] + rprime[s][i] - (u_2[s][i] << T(ring_size - 1)) + (b[s][i] << T(ring_size - 1));
        // sum[1][i] = a[1][i] - c[i] * a2 + rprime[1][i] - (u_2[1][i] << T(ring_size - 1)) + (b[1][i] << T(ring_size - 1));
    }
    // opening sum
    Open(e, sum, size, -1, nodeNet, ss);

    T e_bit;
    for (i = 0; i < size; ++i) {
        e_bit = GET_BIT(e[i], ring_size - 1); // getting the (k-1)th bit
        for (size_t s = 0; s < numShares; s++) {
            res[s][i] = e_bit * ai[s] + b[s][i] - (e_bit * b[s][i] << T(1));
        }
        // res[1][i] = e_bit * a2 + b[1][i] - (e_bit * b[1][i] << T(1));
    }

    // cleanup
    delete[] ai;
    delete[] c;
    delete[] e;

    for (i = 0; i < numShares; i++) {
        delete[] edaBit_r[i];
        delete[] edaBit_b_2[i];
        delete[] b[i];
        delete[] sum[i];
        delete[] u_2[i];
        delete[] rprime[i];
    }
    delete[] edaBit_r;
    delete[] edaBit_b_2;
    delete[] b;
    delete[] sum;
    delete[] u_2;
    delete[] rprime;
}

#endif // _MSB_HPP_