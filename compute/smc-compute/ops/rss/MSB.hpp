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
    T **b = new T *[size];
    T **sum = new T *[size];
    T **u_2 = new T *[size];
    T **rprime = new T *[size];

    // edaBit uses interface format [size][numShares]
    T **edaBit_r = new T *[size];
    T **edaBit_b_2 = new T *[size];

    T *c = new T[size];
    T *e = new T[size];

    for (i = 0; i < size; i++) {
        b[i] = new T[numShares];
        sum[i] = new T[numShares];
        u_2[i] = new T[numShares];
        rprime[i] = new T[numShares];
        edaBit_r[i] = new T[numShares];
        edaBit_b_2[i] = new T[numShares];
        memset(edaBit_r[i], 0, sizeof(T) * numShares);
        memset(edaBit_b_2[i], 0, sizeof(T) * numShares);
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, 1);

    // stays the same
    Rss_RandBit(b, size, ring_size, nodeNet, ss);

    // generating a full-sized edaBit
    edaBit(edaBit_r, edaBit_b_2, ring_size, size, ring_size, nodeNet, ss);

    // edaBit outputs are already in interface format [size][numShares]
    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            rprime[i][s] = edaBit_r[i][s] - (GET_BIT(edaBit_b_2[i][s], T(ring_size - 1)) << T(ring_size - 1));
            // combining w the next loop
            // combining w the previous loop
            sum[i][s] = (a[i][s] + edaBit_r[i][s]);
        }
    }

    Open(c, sum, size, -1, nodeNet, ss);

    // edaBit_b_2 is already in interface format [size][numShares], just mask the values
    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            edaBit_b_2[i][s] = edaBit_b_2[i][s] & ss->SHIFT[ring_size - 1];
        }
        // definitely needed
        c[i] = c[i] & ss->SHIFT[ring_size - 1];
    }

    Rss_BitLT(u_2, c, edaBit_b_2, size, ring_size, nodeNet, ss);

    for (i = 0; i < size; ++i) {
        for (size_t s = 0; s < numShares; s++)
            sum[i][s] = a[i][s] - c[i] * ai[s] + rprime[i][s] - (u_2[i][s] << T(ring_size - 1)) + (b[i][s] << T(ring_size - 1));
    }
    // opening sum
    Open(e, sum, size, -1, nodeNet, ss);

    T e_bit;
    for (i = 0; i < size; ++i) {
        e_bit = GET_BIT(e[i], T(ring_size - 1)); // getting the (k-1)th bit
        for (size_t s = 0; s < numShares; s++) {
            res[i][s] = e_bit * ai[s] + b[i][s] - (e_bit * b[i][s] << T(1));
        }
    }

    // cleanup
    delete[] ai;
    delete[] c;
    delete[] e;

    for (i = 0; i < size; i++) {
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

// not implemented yet
// unsure if needed 
template <typename T>
void Rss_lth_bit(T **res, T **a, uint bitlength, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    assertm((ring_size == ss->ring_size ) , "checking ring_size argument == ss->ring_size");

    uint numShares = ss->getNumShares();
    uint i; // used for loops

    // only need to generate a single random bit per private value
    T **b = new T *[size];
    T **sum = new T *[size];
    T **u_2 = new T *[size];
    T **edaBit_r = new T *[size];
    T **edaBit_b_2 = new T *[size];
    T **rprime = new T *[size];

    T *c = new T[size];
    T *e = new T[size];

    for (i = 0; i < size; i++) {
        b[i] = new T[numShares];
        edaBit_r[i] = new T[numShares];
        edaBit_b_2[i] = new T[numShares];
        sum[i] = new T[numShares];
        u_2[i] = new T[numShares];

        rprime[i] = new T[numShares];
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
            rprime[i][s] = edaBit_r[i][s] - (GET_BIT(edaBit_b_2[i][s], T(ring_size - 1)) << T(ring_size - 1));
            // combining w the next loop
            // combining w the previous loop
            sum[i][s] = (a[i][s] + edaBit_r[i][s]);
        }
    }

    Open(c, sum, size, -1, nodeNet, ss);

    // Create temporary edaBit_b_2_interface for BitLT (which expects [size][numShares])
    T **edaBit_b_2_interface = new T *[size];
    for (i = 0; i < size; i++) {
        edaBit_b_2_interface[i] = new T[numShares];
        for (size_t s = 0; s < numShares; s++) {
            edaBit_b_2_interface[i][s] = edaBit_b_2[s][i] & ss->SHIFT[ring_size - 1];
        }
        // definitely needed
        c[i] = c[i] & ss->SHIFT[ring_size - 1];
    }

    Rss_BitLT(u_2, c, edaBit_b_2_interface, size, ring_size, nodeNet, ss);

    for (i = 0; i < size; ++i) {
        for (size_t s = 0; s < numShares; s++)
            sum[i][s] = a[i][s] - c[i] * ai[s] + rprime[i][s] - (u_2[i][s] << T(ring_size - 1)) + (b[i][s] << T(ring_size - 1));
    }
    // opening sum
    Open(e, sum, size, -1, nodeNet, ss);

    T e_bit;
    for (i = 0; i < size; ++i) {
        e_bit = GET_BIT(e[i], ring_size - 1); // getting the (k-1)th bit
        for (size_t s = 0; s < numShares; s++) {
            res[i][s] = e_bit * ai[s] + b[i][s] - (e_bit * b[i][s] << T(1));
        }
    }

    // cleanup
    delete[] ai;
    delete[] c;
    delete[] e;

    for (i = 0; i < size; i++) {
        delete[] edaBit_b_2_interface[i];
        delete[] b[i];
        delete[] sum[i];
        delete[] u_2[i];
        delete[] rprime[i];
    }
    for (size_t s = 0; s < numShares; s++) {
        delete[] edaBit_r[s];
        delete[] edaBit_b_2[s];
    }
    delete[] edaBit_r;
    delete[] edaBit_b_2;
    delete[] edaBit_b_2_interface;
    delete[] b;
    delete[] sum;
    delete[] u_2;
    delete[] rprime;
}
#endif // _MSB_HPP_