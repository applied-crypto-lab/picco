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
#ifndef _BITLT_HPP_
#define _BITLT_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "Mult.hpp" 
// returns a binary sharing res, where res = a ?< b (we pack 2 shares in a single bit
// the function expects a and b to be arrays of binary values
// a is a single value
// b is two single values of packed shares

template <typename T>
void Rss_CarryOut(T **res, T *a, T **b, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss);

template <typename T>
void OptimalBuffer(T **buffer, T **d, uint size, uint r_size, replicatedSecretShare<T> *ss);

template <typename T>
void Rss_CarryOutAux(T **res, T **d, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss);

template <typename T>
void Rss_BitLT(T **res, T *a, T **b, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // commenting this line out since we should still be able to use this function for smaller ring sizes than what is used for the computation as a whole, since this is on bits
    // assertm((ring_size == ss->ring_size), "checking ring_size argument == ss->ring_size");
    uint numShares = ss->getNumShares();

    uint i, j; // used for loops

    T **b_prime = new T *[numShares];
    for (i = 0; i < numShares; i++) {
        b_prime[i] = new T[size];
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, -1);

    for (j = 0; j < size; j++) {
        // step 1  -- flipping all the bits of bk
        // if switch value is zero, do nothing
        for (size_t s = 0; s < numShares; s++) {
            b_prime[s][j] = (b[s][j] ^ ai[s]);
        }
    }
    Rss_CarryOut(res, a, b_prime, size, ring_size, nodeNet, ss);

    // flipping on share of output
    for (j = 0; j < size; ++j) {
        for (size_t s = 0; s < numShares; s++) {
            res[s][j] = GET_BIT(res[s][j], T(0)) ^ GET_BIT(ai[s], T(0));
        }
        // res[1][j] = GET_BIT(res[1][j], 0) ^ GET_BIT(b2, 0);
    }

    for (i = 0; i < numShares; i++) {
        delete[] b_prime[i];
    }
    delete[] b_prime;
    delete[] ai;
}

template <typename T>
void Rss_CarryOut(T **res, T *a, T **b, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    // assertm((ring_size == ss->ring_size), "checking ring_size argument == ss->ring_size");

    uint i;
    uint numShares = ss->getNumShares();

    // d is shared
    // printf("inits\n");
    T **d = new T *[numShares * 2];
    for (i = 0; i < numShares * 2; i++) {
        d[i] = new T[size];
        memset(d[i], 0, sizeof(T) * size);
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, -1);

    // initial step
    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            d[s][i] = (a[i] & ai[s]) ^ b[s][i];
            d[numShares + s][i] = (a[i] & b[s][i]);
        }
        for (size_t s = 0; s < numShares; s++) {
            d[numShares + s][i] = SET_BIT(d[numShares + s][i], T(0), GET_BIT(d[numShares + s][i], T(0)) ^ (GET_BIT(d[s][i], T(0))));
        }
    }

    Rss_CarryOutAux(res, d, size, ring_size, nodeNet, ss);

    for (i = 0; i < numShares * 2; i++) {
        delete[] d[i];
    }
    delete[] d;
    delete[] ai;

    // res is the actual return value of carry out, being [g] secret shared
}

template <typename T>
void OptimalBuffer(T **buffer, T **d, uint size, uint r_size, replicatedSecretShare<T> *ss) {
    // prepares input u for multiplication
    // buffer and d are the same size (4 x size)
    // reduction to a single loop, making complexity O(size), rather than O(size * r_size)
    static uint numShares = ss->getNumShares();

    T i, even, odd;
    // doing this once
    even = ss->EVEN[r_size];
    odd = ss->ODD[r_size];

    for (i = 0; i < size; ++i) {
        for (size_t s = 0; s < numShares; s++) {
            buffer[s][i] = buffer[s][i] | (d[s][i] & even);
            buffer[s][i] = buffer[s][i] | ((d[s + numShares][i] & even) << T(1));

            buffer[s + numShares][i] = buffer[s + numShares][i] | ((d[s][i] & odd) >> T(1));
            buffer[s + numShares][i] = buffer[s + numShares][i] | (d[s][i] & odd);
        }
    }
}

// a non-recursive solution to minimize memory consumption
template <typename T>
void Rss_CarryOutAux(T **res, T **d, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // assertm((ring_size == ss->ring_size), "checking ring_size argument == ss->ring_size");

    static uint numShares = ss->getNumShares();
    uint i, j, originial_num, num, n_uints, r_size, new_r_size, t_index;
    // uint8_t utemp1, utemp2;
    T mask2, mask1m8, mask2m8;
    r_size = ring_size;
    originial_num = ((r_size + 7) >> 3) * size;

    T **buffer = new T *[numShares * 2];
    for (i = 0; i < numShares * 2; i++) {
        buffer[i] = new T[size];
        memset(buffer[i], 0, sizeof(T) * size);
    }

    uint8_t **a = new uint8_t *[numShares];
    uint8_t **b = new uint8_t *[numShares];
    uint8_t **u = new uint8_t *[numShares];

    //  need to do memsets every iteration
    for (i = 0; i < numShares; i++) {
        // memsets are actually needed here since are ORing
        // tried w/o memsets - still working?
        a[i] = new uint8_t[originial_num];
        memset(a[i], 0, sizeof(uint8_t) * originial_num);
        b[i] = new uint8_t[originial_num];
        memset(b[i], 0, sizeof(uint8_t) * originial_num);
        u[i] = new uint8_t[originial_num];
        // memset(u[i], 0, sizeof(uint8_t) * originial_num);
    }

    while (r_size > 1) {
        num = ((r_size + 7) >> 3) * size;
        n_uints = ((r_size + 7) >> 3);
        // rounding down here (by default)
        new_r_size = r_size >> 1; // (r_size / 2)

        OptimalBuffer(buffer, d, size, r_size, ss);

        // Splitting the buffer into bytes
        for (i = 0; i < size; ++i) {
            for (size_t s = 0; s < numShares; s++) {
                memcpy(a[s] + i * n_uints, buffer[s] + i, n_uints);
                memcpy(b[s] + i * n_uints, buffer[numShares + s] + i, n_uints);
            }
        }

        Mult_Byte(u, a, b, num, nodeNet, ss);

        // clearing the buffer
        for (int i = 0; i < numShares * 2; i++) {
            memset(buffer[i], 0, sizeof(T) * size);
        }

        for (i = 0; i < size; ++i) {
            for (j = 0; j < new_r_size; ++j) {
                // loop constants
                t_index = (j >> 2) + (i * n_uints);
                mask2 = (2 * j + 1); 
                mask2m8 = (2 * j + 1) & 7;
                mask1m8 = (2 * j) & 7; // "&7" = %8, used for leftover bits

                for (size_t s = 0; s < numShares; s++) {
                    buffer[s][i] = SET_BIT(buffer[s][i], T(j), GET_BIT(T(u[s][t_index]), mask1m8));
                    buffer[numShares + s][i] = SET_BIT(buffer[numShares + s][i], T(j), (GET_BIT(T(u[s][t_index]), mask2m8) ^ GET_BIT(d[numShares + s][i], mask2)));
                }
            }
        }

        // checking if we have an unused (p,g) pair
        // if yes, move it to new_r_size's position
        // and update the new_r_size by 1
        if (r_size & 1) {
            for (i = 0; i < size; ++i) {
                for (j = 0; j < 2 * numShares; ++j) {
                    // getting the unused p (or g) from d and
                    // moving it to new_r_size + 1
                    buffer[j][i] = SET_BIT(buffer[j][i], T(new_r_size), GET_BIT(d[j][i], T(r_size - 1)));
                }
            }
            new_r_size += 1;
        }
        // copying buffer back to d for next round
        for (i = 0; i < size; i++) {
            for (j = 0; j < 2 * numShares; j++) {
                memcpy(d[j] + i, buffer[j] + i, sizeof(T));
            }
        }

        // updating r_size for next round
        r_size = new_r_size;

        // sanitizing at end of round
        for (i = 0; i < 2 * numShares; i++) {
            memset(buffer[i], 0, sizeof(T) * size);
        }
    }

    // base case of recursive implementation
    // Is this wrong? why is it only two shares?
    for (size_t s = 0; s < numShares; s++) {
        for (i = 0; i < size; ++i) {
            res[s][i] = SET_BIT(res[s][i], T(0), GET_BIT(d[numShares + s][i], T(0)));
            // res[1][i] = SET_BIT(res[1][i], 0, GET_BIT(d[3][i], 0));
        }
    }

    for (i = 0; i < numShares; i++) {

        delete[] a[i];
        delete[] b[i];
        delete[] u[i];
    }

    delete[] a;
    delete[] b;
    delete[] u;

    for (i = 0; i < numShares * 2; i++) {
        delete[] buffer[i];
    }
    delete[] buffer;
}
#endif // _BITLT_HPP_
