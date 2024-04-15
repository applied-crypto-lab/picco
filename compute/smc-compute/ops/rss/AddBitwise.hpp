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
#include <cstdint>

template <typename T>
void CarryBuffer(T **buffer, T **d, uint **index_array, uint size, uint k, uint numShares);

// a,b are private and bitwise secret-shared
// we would never supply any alen/blen >ring_size, so we don't need to check here, right?
// alen,blen should ALWAYS be <= k
// any bits BEYOND the k-1th would be discarded anyways
template <typename T>
void Rss_BitAdd(T **res, T **a, T **b, uint alen, uint blen, uint ring_size, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    // std::cout << "(bitAdd) sizeof(T) = " << sizeof(T) << std::endl;
    // std::cout << "(bitAdd) size = " << size << std::endl;
    // std::cout << "(bitAdd) ring_size = " << ring_size << std::endl;
    uint inlen = std::max(alen, blen);

    // uint reslen = std::min(std::max(alen,blen) + 1, ring_size); // not needed here, since we're just interested in the lengths of the inputs
    // the resultlen WILL be max(alen,blen) + 1

    static uint numShares = ss->getNumShares();

    T **d = new T *[2 * numShares];
    for (size_t i = 0; i < 2 * numShares; i++) {
        d[i] = new T[size];
        memset(d[i], 0, sizeof(T) * size);
    }

    Mult_Bitwise(res, a, b, size, nodeNet, ss);
    for (size_t i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            d[s][i] = a[s][i] ^ b[s][i];
            d[numShares + s][i] = res[s][i];
        }
    }
    Rss_CircleOpL(d, inlen, size, nodeNet, ss);

    for (size_t i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            res[s][i] = (a[s][i] ^ b[s][i]) ^ (d[numShares + s][i] << T(1));
    }
    for (size_t i = 0; i < 2 * numShares; i++) {
        delete[] d[i];
    }
    delete[] d;
}

// a is public, b is private and bitwise secret-shared
// used in share conversion (potentially not needed anymore)
template <typename T>
void Rss_BitAdd(T **res, T *a, T **b, uint ring_size, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint numShares = ss->getNumShares();
    T **d = new T *[2 * numShares];
    for (size_t i = 0; i < 2 * numShares; i++) {
        d[i] = new T[size];
        memset(d[i], 0, sizeof(T) * size);
    }
    T *ai = new T[numShares];
    ss->generateMap(ai, -1);

    for (size_t i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            d[s][i] = (a[i] & ai[s]) ^ b[s][i];
            d[numShares + s][i] = (a[i] & b[s][i]);
        }
    }
    Rss_CircleOpL(d, ring_size, size, nodeNet);

    for (size_t i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            res[s][i] = ((a[i] & ai[s]) ^ b[s][i]) ^ (d[numShares + s][i] << T(1));
    }
    for (size_t i = 0; i < 2 * numShares; i++) {
        delete[] d[i];
    }
    delete[] d;
}

template <typename T>
void Rss_CircleOpL(T **d, uint r_size, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    T i, j, l, y, z, op_r; // used for loops
    uint numShares = ss->getNumShares();

    if (r_size > 1) {
        // just three nested for-loops
        // r_size <=> k in algorithm
        uint num = ((r_size + 7) >> 3) * size;
        uint n_uints = ((r_size + 7) >> 3);
        uint t_index;
        T mask2, mask1m8, mask2m8;
        uint r_size_2 = pow(2, ceil(log2(r_size))); // rounding up to next power of two
        uint rounds = ceil(log2(r_size_2));

        T **buffer = new T *[2 * numShares];
        // we need at most (r_size + 7)/8 bytes to store bits from the buffer
        uint8_t **a = new uint8_t *[numShares];
        uint8_t **b = new uint8_t *[numShares];
        uint8_t **u = new uint8_t *[numShares];

        uint **index_array = new uint *[2];
        for (i = 0; i < 2; i++) {
            index_array[i] = new uint[r_size_2];
            memset(index_array[i], 0, sizeof(uint) * r_size_2);
        }

        for (i = 0; i < numShares; i++) {
            buffer[i] = new T[size];
            buffer[numShares + i] = new T[size];
            memset(buffer[i], 0, sizeof(T) * size);
            memset(buffer[numShares + i], 0, sizeof(T) * size);
            // memsets are actually needed here since are ORing
            a[i] = new uint8_t[num];
            memset(a[i], 0, sizeof(uint8_t) * num);
            b[i] = new uint8_t[num];
            memset(b[i], 0, sizeof(uint8_t) * num);
            u[i] = new uint8_t[num];
            memset(u[i], 0, sizeof(uint8_t) * num);
        }

        for (i = 1; i <= rounds; i++) {

            op_r = 0; // number of operations in a round
            // equivalent to the new_ring_size in MSB
            for (j = 1; j <= ceil(r_size_2 / pow(2, i)); j++) {
                y = uint(pow(2, i - 1) + j * pow(2, i)) % r_size_2;
                for (z = 1; z <= (pow(2, i - 1)); z++) {
                    // checking we have a valid set of indices to add to our set
                    if ((((y % r_size_2)) <= r_size) && (((y + z) % (r_size_2 + 1)) <= r_size)) {
                        index_array[0][op_r] = (y % r_size_2) - 1;
                        index_array[1][op_r] = ((y + z) % (r_size_2 + 1)) - 1;
                        op_r++;
                    }
                }
            }

            // updating parameters for optimization
            n_uints = ((2 * op_r + 7) >> 3);
            num = ((2 * op_r + 7) >> 3) * size;

            // extracting terms into buffer
            CarryBuffer(buffer, d, index_array, size, op_r, numShares);

            // THIS DOESNT WORK (blocks are not contiguous)
            // DO NOT TRY
            // memcpy(a[0], buffer[0], size*n_uints); ...

            // Splitting the buffer into bytes
            for (j = 0; j < size; ++j) {
                for (size_t s = 0; s < numShares; s++) {
                    memcpy(a[s] + j * n_uints, buffer[s] + j, n_uints);
                    // memcpy(a[1] + j * n_uints, buffer[1] + j, n_uints);
                    memcpy(b[s] + j * n_uints, buffer[numShares + s] + j, n_uints);
                    // memcpy(b[1] + j * n_uints, buffer[3] + j, n_uints);
                }
            }

            // bitwise multiplication
            Mult_Byte(u, a, b, num, nodeNet, ss);

            for (l = 0; l < size; ++l) {
                for (j = 0; j < op_r; ++j) {
                    // loop constants
                    t_index = (j >> 2) + (l * n_uints);
                    mask2 = index_array[1][j];
                    mask1m8 = (2 * j) & 7; // "&7" = %8, used for leftover bits
                    mask2m8 = (2 * j + 1) & 7;

                    for (size_t s = 0; s < numShares; s++) {
                        d[s][l] = SET_BIT(d[s][l], mask2, GET_BIT(static_cast<T>(u[s][t_index]), mask1m8));

                        // simplified from needing two separate loops
                        d[numShares + s][l] = SET_BIT(d[numShares + s][l], mask2, (GET_BIT(static_cast<T>(u[s][t_index]), mask2m8) ^ GET_BIT(d[numShares + s][l], mask2)));
                    }
                }
            }
        }

        for (i = 0; i < numShares; i++) {
            delete[] buffer[i];
            delete[] buffer[numShares + i];
            delete[] a[i];
            delete[] b[i];
            delete[] u[i];
        }
        for (i = 0; i < 2; i++) {
            delete[] index_array[i];
        }

        delete[] a;
        delete[] b;
        delete[] u;
        delete[] index_array;
        delete[] buffer;
    }
}

template <typename T>
void CarryBuffer(T **buffer, T **d, uint **index_array, uint size, uint k, uint numShares) {
    // prepares input u for multiplication
    // extracts p2i, p2i-1, and g2i
    // buffer and d are the same size (4 x size)
    T i, j, mask1, mask2, mask1p, mask2p;

    for (i = 0; i < size; i++) {
        for (j = 0; j < k; j++) {

            // used to set the bits in the correct positions in buffer
            mask1 = 2 * j;
            mask2 = 2 * j + 1;

            // used to get the correct bits from d
            mask1p = index_array[0][j];
            mask2p = index_array[1][j];

            for (T s = 0; s < numShares; s++) {
                buffer[s][i] = SET_BIT(buffer[s][i], mask1, GET_BIT(d[s][i], mask1p));
                buffer[s][i] = SET_BIT(buffer[s][i], mask2, GET_BIT(d[numShares + s][i], mask1p));
                buffer[numShares + s][i] = SET_BIT(buffer[numShares + s][i], mask1, GET_BIT(d[s][i], mask2p));
                buffer[numShares + s][i] = SET_BIT(buffer[numShares + s][i], mask2, GET_BIT(d[s][i], mask2p));
            }
        }
    }
}

#endif
