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
// stores the result in input
// follows similar logic to that in BitAdd implementation
template <typename T>
void Rss_PreOR(T **result, T **input, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    T i, j, l, y, z, op_r; // used for loops
    static uint numShares = ss->getNumShares();
    uint r_size = ring_size;
    uint idx_1, idx_2;
    if (r_size > 1) {

        uint num = ((r_size + 7) >> 3) * size;
        uint n_uints = ((r_size + 7) >> 3);
        uint t_index;
        T mask1, mask2, mask1m8;
        uint r_size_2 = pow(2, ceil(log2(r_size))); // rounding up to next power of two
        uint rounds = ceil(log2(r_size_2));

        uint **index_array = new uint *[2];
        T **buffer = new T *[2 * numShares];

        // we need at most (r_size + 7)/8 bytes to store bits from the buffer
        uint8_t **a = new uint8_t *[numShares];
        uint8_t **b = new uint8_t *[numShares];
        uint8_t **u = new uint8_t *[numShares];

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
            CarryBufferPreOR(buffer, input, index_array, size, op_r, numShares);

            for (j = 0; j < size; ++j) {
                for (size_t s = 0; s < numShares; s++) {
                    memcpy(a[s] + j * n_uints, buffer[s] + j, n_uints);
                    // memcpy(a[1] + j * n_uints, buffer[1] + j, n_uints);
                    memcpy(b[s] + j * n_uints, buffer[numShares + s] + j, n_uints);
                    // memcpy(b[1] + j * n_uints, buffer[3] + j, n_uints);
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
                        // input[s][l] = SET_BIT(input[s][l], mask2, GET_BIT(u[s][t_index], mask1m8));
                        // input[1][l] = SET_BIT(input[1][l], mask2, GET_BIT(u[1][t_index], mask1m8));

                        temp = GET_BIT(input[s][l], mask1) ^ GET_BIT(input[s][l], mask2) ^ GET_BIT(static_cast<T>(u[s][t_index]), mask1m8);

                        // simplified from needing two separate loops
                        result[s][l] = SET_BIT(input[s][l], mask2, temp);
                        // input[3][l] = SET_BIT(input[3][l], mask2, (GET_BIT(u[1][t_index], mask2m8) ^ GET_BIT(input[3][l], mask2)));
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
void CarryBufferPreOR(T **buffer, T **a, uint **index_array, uint size, uint k, uint numShares) {
    T i, j, mask1p, mask2p;

    for (i = 0; i < size; i++) {
        for (j = 0; j < k; j++) {
            // used to get the correct bits from a
            mask1p = index_array[0][j];
            mask2p = index_array[1][j];
            // printf("i,j %u, %u:  (mask1, mask2) = (%u, %u) \n", i,j ,mask1p, mask2p);

            for (T s = 0; s < numShares; s++) {
                buffer[s][i] = SET_BIT(buffer[s][i], j, GET_BIT(a[s][i], mask1p));
                buffer[numShares + s][i] = SET_BIT(buffer[numShares + s][i], j, GET_BIT(a[s][i], mask2p));
            }
        }
    }
}
#endif // _PREOR_HPP_
