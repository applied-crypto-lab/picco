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
#include "Open.hpp"
#include <cmath>
#include <cstdint>

template <typename T>
void CarryBuffer(T **buffer, T **d, uint **index_array, uint size, uint k, uint numShares);

// a,b are private and bitwise secret-shared
// we would never supply any alen/blen >ring_size, so we don't need to check here, right?
// alen,blen should ALWAYS be <= k
// any bits BEYOND the k-1th would be discarded anyways
// Interface format: a, b, res are [size][numShares] where array[i][s] is share s of element i
template <typename T>
void Rss_BitAdd(T **res, T **a, T **b, uint alen, uint blen, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    assertm((ring_size == ss->ring_size), "checking ring_size argument == ss->ring_size");

    uint inlen = std::max(alen, blen);

    uint numShares = ss->getNumShares();

    // Internal d array uses [2*numShares][size] for carry lookahead algorithm
    T **d = new T *[2 * numShares];
    for (size_t i = 0; i < 2 * numShares; i++) {
        d[i] = new T[size];
        memset(d[i], 0, sizeof(T) * size);
    }

    // Mult_Bitwise expects [size][numShares] format
    Mult_Bitwise(res, a, b, size, nodeNet, ss);

    // Convert from interface format [size][numShares] to internal format [2*numShares][size]
    for (size_t i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            d[s][i] = a[i][s] ^ b[i][s];
            d[numShares + s][i] = res[i][s];
        }
    }
    Rss_CircleOpL(d, size, inlen, nodeNet, ss);

    // Convert from internal format back to interface format [size][numShares]
    for (size_t i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            res[i][s] = (a[i][s] ^ b[i][s]) ^ (d[numShares + s][i] << T(1));
    }

    for (size_t i = 0; i < 2 * numShares; i++) {
        delete[] d[i];
    }
    delete[] d;
}

// a is public, b is private and bitwise secret-shared
// used in share conversion (potentially not needed anymore)
// Interface format: b, res are [size][numShares] where array[i][s] is share s of element i
template <typename T>
void Rss_BitAdd(T **res, T *a, T **b, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint numShares = ss->getNumShares();
    // Internal d array uses [2*numShares][size] for carry lookahead algorithm
    T **d = new T *[2 * numShares];
    for (size_t i = 0; i < 2 * numShares; i++) {
        d[i] = new T[size];
        memset(d[i], 0, sizeof(T) * size);
    }
    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, -1);

    // Convert from interface format [size][numShares] to internal format [2*numShares][size]
    for (size_t i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            d[s][i] = (a[i] & ai[s]) ^ b[i][s];
            d[numShares + s][i] = (a[i] & b[i][s]);
        }
    }

    Rss_CircleOpL(d, size, ring_size, nodeNet, ss);

    // Convert from internal format back to interface format [size][numShares]
    for (size_t i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            res[i][s] = ((a[i] & ai[s]) ^ b[i][s]) ^ (d[numShares + s][i] << T(1));
    }
    for (size_t i = 0; i < 2 * numShares; i++) {
        delete[] d[i];
    }
    delete[] d;
}

template <typename T>
void Rss_CircleOpL(T **d, uint size, uint r_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

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
        }

        // Byte arrays use interface format [num][numShares] where:
        //   a[byteIndex][shareIndex] is share shareIndex of byte byteIndex
        uint8_t **a = nullptr;
        uint8_t **b = nullptr;
        uint8_t **u = nullptr;

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

            // Allocate byte arrays in interface format [num][numShares]
            a = new uint8_t *[num];
            b = new uint8_t *[num];
            u = new uint8_t *[num];
            for (size_t idx = 0; idx < num; idx++) {
                a[idx] = new uint8_t[numShares];
                b[idx] = new uint8_t[numShares];
                u[idx] = new uint8_t[numShares];
                memset(a[idx], 0, sizeof(uint8_t) * numShares);
                memset(b[idx], 0, sizeof(uint8_t) * numShares);
                memset(u[idx], 0, sizeof(uint8_t) * numShares);
            }

            // extracting terms into buffer
            CarryBuffer(buffer, d, index_array, size, op_r, numShares);

            // Splitting the buffer into bytes - convert to interface format [num][numShares]
            for (j = 0; j < size; ++j) {
                for (size_t byte_offset = 0; byte_offset < n_uints; byte_offset++) {
                    size_t byte_idx = j * n_uints + byte_offset;
                    for (size_t s = 0; s < numShares; s++) {
                        // Extract byte from buffer[s][j] at position byte_offset
                        a[byte_idx][s] = (buffer[s][j] >> (byte_offset * 8)) & 0xFF;
                        b[byte_idx][s] = (buffer[numShares + s][j] >> (byte_offset * 8)) & 0xFF;
                    }
                }
            }

            // bitwise multiplication - expects [num][numShares] format
            Mult_Byte(u, a, b, num, nodeNet, ss);

            for (l = 0; l < size; ++l) {
                for (j = 0; j < op_r; ++j) {
                    // loop constants
                    t_index = (j >> 2) + (l * n_uints);
                    mask2 = index_array[1][j];
                    mask1m8 = (2 * j) & 7; // "&7" = %8, used for leftover bits
                    mask2m8 = (2 * j + 1) & 7;

                    for (size_t s = 0; s < numShares; s++) {
                        // Access u in interface format [t_index][s]
                        d[s][l] = SET_BIT(d[s][l], mask2, GET_BIT(static_cast<T>(u[t_index][s]), mask1m8));

                        // simplified from needing two separate loops
                        d[numShares + s][l] = SET_BIT(d[numShares + s][l], mask2, (GET_BIT(static_cast<T>(u[t_index][s]), mask2m8) ^ GET_BIT(d[numShares + s][l], mask2)));
                    }
                }
            }

            // Free byte arrays for this round
            for (size_t idx = 0; idx < num; idx++) {
                delete[] a[idx];
                delete[] b[idx];
                delete[] u[idx];
            }
            delete[] a;
            delete[] b;
            delete[] u;
        }

        for (i = 0; i < numShares; i++) {
            delete[] buffer[i];
            delete[] buffer[numShares + i];
        }
        for (i = 0; i < 2; i++) {
            delete[] index_array[i];
        }

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
                // For carry lookahead: P_combined = P[i] AND P[j], G_combined = G[j] XOR (P[j] AND G[i])
                // a[2k] = P[i], a[2k+1] = G[i]
                // b[2k] = P[j], b[2k+1] = P[j] (to compute P[j] AND G[i])
                // Then u[2k] = P[i] AND P[j], u[2k+1] = G[i] AND P[j]
                buffer[s][i] = SET_BIT(buffer[s][i], mask1, GET_BIT(d[s][i], mask1p));           // a[2j] = P[i]
                buffer[s][i] = SET_BIT(buffer[s][i], mask2, GET_BIT(d[numShares + s][i], mask1p)); // a[2j+1] = G[i]
                buffer[numShares + s][i] = SET_BIT(buffer[numShares + s][i], mask1, GET_BIT(d[s][i], mask2p)); // b[2j] = P[j]
                // FIX: b[2j+1] should be P[j], not G[j], so we can compute P[j] AND G[i]
                buffer[numShares + s][i] = SET_BIT(buffer[numShares + s][i], mask2, GET_BIT(d[s][i], mask2p)); // b[2j+1] = P[j]
            }
        }
    }
}

// a,b are private and bitwise secret-shared
// we would never supply any alen/blen >ring_size, so we don't need to check here, right?
// alen,blen should ALWAYS be <= k
// any bits BEYOND the k-1th would be discarded anyways
// Interface format: a, b, res, res_carry are [size][numShares] where array[i][s] is share s of element i
template <typename T>
void Rss_BitAdd_Trunc(T **res, T **res_carry, T **a, T **b, uint alen, uint blen, uint m, uint carry_offeset, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    assertm((ring_size == ss->ring_size), "checking ring_size argument == ss->ring_size");

    uint inlen = std::max(alen, blen);

    uint numShares = ss->getNumShares();

    // Internal d array uses [2*numShares][size] for carry lookahead algorithm
    T **d = new T *[2 * numShares];
    for (size_t i = 0; i < 2 * numShares; i++) {
        d[i] = new T[size];
        memset(d[i], 0, sizeof(T) * size);
    }

    // Mult_Bitwise expects [size][numShares] format
    Mult_Bitwise(res, a, b, size, nodeNet, ss);
    // Convert from interface format [size][numShares] to internal format [2*numShares][size]
    for (size_t i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            d[s][i] = a[i][s] ^ b[i][s];
            d[numShares + s][i] = res[i][s];
        }
    }
    Rss_CircleOpL(d, size, inlen, nodeNet, ss);

    // Convert from internal format back to interface format [size][numShares]
    // only getting the carrys for bitAddTrunc - the MSB of res will be extracted later
    for (size_t i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            res[i][s] = (a[i][s] ^ b[i][s]) ^ (d[numShares + s][i] << T(1));
            // shouldn't this be (m) instead of (m-1)?
            res_carry[carry_offeset + i][s] = GET_BIT(d[numShares + s][i], T(m - 1));
            res_carry[carry_offeset + size + i][s] = GET_BIT(d[numShares + s][i], T(ring_size - 1));
        }
    }

    for (size_t i = 0; i < 2 * numShares; i++) {
        delete[] d[i];
    }
    delete[] d;
}

// Interface format: a, b, res, res_carry are [size][numShares] where array[i][s] is share s of element i
template <typename T>
void Rss_BitAdd_RNTE(T **res, T **res_carry, T **a, T **b, uint alen, uint blen, uint m, uint carry_offeset, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    assertm((ring_size == ss->ring_size), "checking ring_size argument == ss->ring_size");

    uint inlen = std::max(alen, blen);

    uint numShares = ss->getNumShares();

    // Internal d array uses [2*numShares][size] for carry lookahead algorithm
    T **d = new T *[2 * numShares];
    for (size_t i = 0; i < 2 * numShares; i++) {
        d[i] = new T[size];
        memset(d[i], 0, sizeof(T) * size);
    }

    // Mult_Bitwise expects [size][numShares] format
    Mult_Bitwise(res, a, b, size, nodeNet, ss);
    // Convert from interface format [size][numShares] to internal format [2*numShares][size]
    for (size_t i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            d[s][i] = a[i][s] ^ b[i][s];
            d[numShares + s][i] = res[i][s];
        }
    }
    Rss_CircleOpL(d, size, inlen, nodeNet, ss);

    // Convert from internal format back to interface format [size][numShares]
    // only getting the carrys for bitAddTrunc - the MSB of res will be extracted later
    for (size_t i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            res[i][s] = (a[i][s] ^ b[i][s]) ^ (d[numShares + s][i] << T(1));
            // shouldn't this be (m) instead of (m-1)?
            res_carry[carry_offeset + i][s] = GET_BIT(d[numShares + s][i], T(m - 1));
            res_carry[carry_offeset + size + i][s] = GET_BIT(d[numShares + s][i], T(m - 3));
            res_carry[carry_offeset + 2 * size + i][s] = GET_BIT(d[numShares + s][i], T(ring_size - 1));
        }
    }

    for (size_t i = 0; i < 2 * numShares; i++) {
        delete[] d[i];
    }
    delete[] d;
}
