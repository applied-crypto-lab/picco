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
#include "bit_utils.hpp"
#include <cstdint>

// input:  a_i [numShares][size] of bits shared in Z_2, stored in a T
// output: out [numShares][1] a bit in Z_2 which is the k-ary OR of all the bits a_i (a_0 | a_1 | ... | a_size)
template <typename T>
void OR_ALL(T **output, T **a_i, int size, int ring_size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    static uint numShares = ss->getNumShares();
    uint num_uints = ((size + 15) >> 4);

    uint8_t **a = new uint8_t *[numShares];
    uint8_t **b = new uint8_t *[numShares];
    uint8_t **u = new uint8_t *[numShares];

    for (size_t i = 0; i < numShares; i++) {
        a[i] = new uint8_t[num_uints];
        memset(a[i], 0, sizeof(uint8_t) * num_uints);
        b[i] = new uint8_t[num_uints];
        memset(b[i], 0, sizeof(uint8_t) * num_uints);
        u[i] = new uint8_t[num_uints];
        memset(u[i], 0, sizeof(uint8_t) * num_uints);
    }
    int tmp_size = size;
    int new_tmp_size;

    uint8_t idx;
    uint8_t idx2;
    uint8_t bpos_in;
    uint8_t bpos_out;
    // performing the first packing
    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            idx = i >> 3;
            bpos_in = i % 8;
            u[s][idx] = SET_BIT(u[s][idx], bpos_in, uint8_t(GET_BIT(a_i[s][i], 0)));
        }
    }

    while (tmp_size > 1) {
        new_tmp_size = tmp_size >> 1; // rounds down
        num_uints = ((new_tmp_size + 7) >> 3);

        process_bytes(a, b, u, tmp_size, ss);
        Mult_Byte(u, a, b, num_uints, nodeNet, ss);

        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < num_uints; i++) {
                u[s][i] = a[s][i] ^ b[s][i] ^ u[s][i]; // OR in Z_2
            }
        }

        if (tmp_size & 1) {
            idx = tmp_size >> 3;
            idx2 = new_tmp_size >> 3;
            bpos_in = (tmp_size - 1) % 8;
            bpos_out = new_tmp_size % 8;

            for (size_t s = 0; s < numShares; s++) {
                u[s][idx2] = SET_BIT(u[s][idx2], bpos_out, GET_BIT(u[s][idx], bpos_in));
            }
            new_tmp_size += 1;
        }

        for (size_t i = 0; i < numShares; i++) {
            memset(a[i], 0, sizeof(uint8_t) * num_uints);
            memset(b[i], 0, sizeof(uint8_t) * num_uints);
        }

        tmp_size = new_tmp_size;
    }

    for (size_t i = 0; i < numShares; i++) {
        delete[] a[i];
        delete[] b[i];
        delete[] u[i];
    }

    delete[] a;
    delete[] b;
    delete[] u;
}

template <typename T>
void process_bytes(uint8_t **a, uint8_t **b, uint8_t **in_buff, int size, replicatedSecretShare<T> *ss) {

    static uint numShares = ss->getNumShares();

    int even_size = size - (size % 2);
    int num_bytes = even_size / 16;
    int remainder = even_size % 16;

    // we have enough data to memcpy
    if (num_bytes) {
        for (size_t s = 0; s < numShares; s++) {
            memcpy(a[s], in_buff[s], num_bytes);
            memcpy(b[s], in_buff[s] + num_bytes, num_bytes);
        }
    }

    // there are < 16 leftover bits that must be moved/split
    uint8_t idx_ab = num_bytes; 
    // static, since at this point it will always be in the "last " byte position
    uint8_t j, idx_in;
    if (remainder) {
        for (size_t s = 0; s < numShares; s++) {
            j = 0;
            idx_in = 2 * (num_bytes);
            for (uint8_t i = 0; i < (remainder >> 1); i++) {
                // idx_in will always be fixed for a
                a[s][idx_ab] = SET_BIT(a[s][idx_ab], i, GET_BIT(in_buff[s][idx_in], i));
            }
            for (uint8_t i = (remainder >> 1); i < remainder; i++) {
                // idx_in may move to the second of the leftover bytes, which is why we need to recompute it each iteration
                idx_in = 2 * (num_bytes) + (i / 8);
                b[s][idx_ab] = SET_BIT(b[s][idx_ab], j, GET_BIT(in_buff[s][idx_in], uint8_t(i % 8)));
                j++;
            }
        }
    }
}
