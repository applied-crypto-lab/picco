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
#ifndef _PREOR_HPP_
#define _PREOR_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "Mult.hpp"
#include "Open.hpp"
#include "bit_utils.hpp"

template <typename T>
void CarryBufferPreOR(T **buffer, T **a, uint **index_array, uint size, uint k, uint numShares) ;



// performs a "reverse" parallel prefix, i.e. a_8, a_8 | a_7, a_8 | a_7 | a_6, ...
// directly on bits
// there isn't a functionality that requires the "normal" ordered parallel prefix, so we don't implement it here
// we should probably have it compute the "normal" ordered parallel prefix, but have the bits of the input reversed
// follows similar logic to that in BitAdd implementation
// although, there isn't a protocol which directly uses it
// All arrays use interface format [size][numShares]
template <typename T>
void Rss_PreOR(  T **result, T **input, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint numShares = ss->getNumShares();

    for (size_t i = 0; i < size; i++) {
        memcpy(result[i], input[i], sizeof(T) * numShares);
    }
    
    T i, j, l, y, z, op_r; // used for loops
    uint r_size = ring_size;
    uint idx_1, idx_2;
    if (r_size > 1) {

        uint num = ((r_size + 7) >> 3) * size;
        uint initial_num = num;  // Save initial allocation size for cleanup
        uint n_uints = ((r_size + 7) >> 3);
        uint t_index;
        T mask1, mask2, mask1m8;
        uint r_size_2 = pow(2, ceil(log2(r_size))); // rounding up to next power of two
        uint rounds = ceil(log2(r_size_2));

        uint **index_array = new uint *[2];
        // buffer uses [2*size][numShares] - two buffers back to back
        T **buffer = new T *[2 * size];

        // we need at most (r_size + 7)/8 bytes to store bits from the buffer
        // Interface format: a[i][s] = share s of byte i
        uint8_t **a = new uint8_t *[num];
        uint8_t **b = new uint8_t *[num];
        uint8_t **u = new uint8_t *[num];

        for (i = 0; i < 2; i++) {
            index_array[i] = new uint[r_size_2];
            memset(index_array[i], 0, sizeof(uint) * r_size_2);
        }

        for (i = 0; i < 2 * size; i++) {
            buffer[i] = new T[numShares];
            memset(buffer[i], 0, sizeof(T) * numShares);
        }

        for (i = 0; i < num; i++) {
            // memsets are actually needed here since are ORing
            a[i] = new uint8_t[numShares];
            memset(a[i], 0, sizeof(uint8_t) * numShares);
            b[i] = new uint8_t[numShares];
            memset(b[i], 0, sizeof(uint8_t) * numShares);
            u[i] = new uint8_t[numShares];
            memset(u[i], 0, sizeof(uint8_t) * numShares);
        }

        for (i = 1; i <= rounds; i++) {
            // printf("\n -- round %u --\n", i);
            op_r = 0; // number of operations in a round

            // for (j = 1; j <= ceil(r_size_2 / pow(2, i)); j++) {
            for (j = ceil(r_size_2 / pow(2, i)); j >= 1; j--) {

                y = uint(pow(2, i - 1) + j * pow(2, i)) % r_size_2;

                for (z = (pow(2, i - 1)); z >= 1; z--) {
                    // for (z = 1; z <= (pow(2, i - 1)); z++) {

                    idx_1 = (y % r_size_2);
                    idx_2 = (y + z) % (r_size_2 + 1);

                    if ((idx_1 <= r_size) && (idx_2 <= r_size)) {
                        index_array[0][op_r] = (idx_1 - 1) * (-1) + (r_size - 1); // used for reverse pp
                        index_array[1][op_r] = (idx_2 - 1) * (-1) + (r_size - 1); // used for reverse pp
                        op_r++;
                    }
                }
            }

            // updating parameters for optimization
            n_uints = ((op_r + 7) >> 3);
            num = ((op_r + 7) >> 3) * size;

            // extracting terms into buffer
            CarryBufferPreOR(buffer, result, index_array, size, op_r, numShares);

            // Copy from buffer [size][numShares] to a, b in interface format [num][numShares]
            // Each byte position gets its share values
            for (j = 0; j < size; ++j) {
                for (size_t byte_idx = 0; byte_idx < n_uints; byte_idx++) {
                    size_t flat_idx = j * n_uints + byte_idx;
                    for (size_t s = 0; s < numShares; s++) {
                        a[flat_idx][s] = ((uint8_t*)&buffer[j][s])[byte_idx];
                        b[flat_idx][s] = ((uint8_t*)&buffer[size + j][s])[byte_idx];
                    }
                }
            }
            // break;
            // bitwise multiplication
            Mult_Byte(u, a, b, num, nodeNet, ss);

            T temp;
            for (l = 0; l < size; ++l) {
                for (j = 0; j < op_r; ++j) {
                    // loop constants
                    // t_index = (j >> 2) + (l * n_uints);
                    t_index = (j >> 3) + (l * n_uints);
                    mask1 = index_array[0][j];
                    mask2 = index_array[1][j];
                    // mask1m8 = (2 * j) & 7; // "&7" = %8, used for leftover bits
                    mask1m8 = j & 7; // "&7" = %8, used for leftover bits
                    // printf("j %u:  (mask1, mask2) -- t_index, mask1m8 = (%u, %u) \t %u, %u \n", j ,mask1, mask2, t_index, mask1m8);

                    for (size_t s = 0; s < numShares; s++) {
                        // u is now in interface format [num][numShares], so u[t_index][s]
                        temp = GET_BIT(result[l][s], mask1) ^ GET_BIT(result[l][s], mask2) ^ GET_BIT(static_cast<T>(u[t_index][s]), mask1m8);

                        // simplified from needing two separate loops
                        result[l][s] = SET_BIT(result[l][s], mask2, temp);
                    }
                }
            }
        }

        for (i = 0; i < 2 * size; i++) {
            delete[] buffer[i];
        }
        for (i = 0; i < initial_num; i++) {
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
void CarryBufferPreOR(T **buffer, T **a, uint **index_array, uint size, uint k, uint numShares) {
    T i, j, mask1p, mask2p;

    for (i = 0; i < size; i++) {
        for (j = 0; j < k; j++) {
            // used to get the correct bits from a
            mask1p = index_array[0][j];
            mask2p = index_array[1][j];
            // printf("i,j %u, %u:  (mask1, mask2) = (%u, %u) \n", i,j ,mask1p, mask2p);

            for (T s = 0; s < numShares; s++) {
                buffer[i][s] = SET_BIT(buffer[i][s], j, GET_BIT(a[i][s], mask1p));
                buffer[size + i][s] = SET_BIT(buffer[size + i][s], j, GET_BIT(a[i][s], mask2p));
            }
        }
    }
}
#endif // _PREOR_HPP_
