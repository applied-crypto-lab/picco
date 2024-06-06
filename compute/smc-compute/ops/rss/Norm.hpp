
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
#ifndef _NORM_HPP_
#define _NORM_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "BitDec.hpp"
#include "LT.hpp"
#include "MSB.hpp"
#include "Mult.hpp"
#include "PreOR.hpp"

template <typename T>
void doOperation_Norm(T **c, T **v, T **b, int bitlength, int size, uint ring_size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {

    uint numShares = ss->getNumShares();

    // std::cout << "numShares  = " << numShares << std::endl;
    // std::cout << "ring_size = " << ring_size << std::endl;
    // std::cout << "bitlen = " << bitlength << std::endl;
    // std::cout << "size = " << size << std::endl;

    T **b_bits = new T *[numShares];
    for (size_t i = 0; i < numShares; i++) {
        b_bits[i] = new T[size];
        memset(b_bits[i], 0, sizeof(T) * size);
    }

    T **b_msb = new T *[numShares];
    T **x = new T *[numShares];
    T **x_bits = new T *[numShares];
    T **prod = new T *[numShares];
    T **z = new T *[numShares];
    T **z_res = new T *[numShares];
    T **vp = new T *[numShares];
    for (size_t i = 0; i < numShares; i++) {
        b_msb[i] = new T[size];
        x[i] = new T[size];

        x_bits[i] = new T[size];
        memset(x_bits[i], 0, sizeof(T) * size);

        prod[i] = new T[size];
        memset(prod[i], 0, sizeof(T) * size);

        vp[i] = new T[size];
        memset(vp[i], 0, sizeof(T) * size);

        z_res[i] = new T[size * bitlength];
        memset(z_res[i], 0, sizeof(T) * bitlength * size);
        z[i] = new T[size * bitlength];
        memset(z[i], 0, sizeof(T) * bitlength * size);
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, T(1));

    Rss_MSB(b_msb, b, size, ring_size, net, ss);

    Mult(prod, b, b_msb, size, threadID, net, ss);
    // computing the absolute value of b

    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            x[s][i] = b[s][i] - 2 * prod[s][i];
        }
    }

    // T *result = new T[size];
    // memset(result, 0, sizeof(T) * size);

    // Open(result, b_msb, size, threadID, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     printf("[b_msb]   [%lu]: %u\n", i, result[i]);
    //     print_binary(result[i], ring_size);
    // }

    // Open(result, x, size, threadID, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     printf("[abs b]   [%lu]: %u\n", i, result[i]);
    //     print_binary(result[i], ring_size);
    // }

    // printf("bitdec\n");
    Rss_BitDec(x_bits, x, bitlength, size, ring_size, net, ss);

    // sanitizing?
    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            x_bits[s][i] = x_bits[s][i] & ss->SHIFT[bitlength];
        }
    }

    // memset(result, 0, sizeof(T) * size);
    // Open_Bitwise(result, x_bits, size, threadID, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     printf("[bitDec]   [%lu]: %u\n", i, result[i]);
    //     print_binary(result[i], ring_size);
    // }

    // santiziing since we're dealing with bits
    for (size_t s = 0; s < numShares; s++) {
        memset(prod[s], 0, sizeof(T) * size);
    }

    // printf("PreOR\n");
    // commputing the "reverse" prefixOR
    //  i.e., a_8, a_8 | a_7, a_8 | a_7 | a_6, ... (subscripts denote the bit position)
    // reusing prod
    Rss_PreOR(prod, x_bits, size, bitlength, net, ss);

    // // sanitizing?
    // for (size_t s = 0; s < numShares; s++) {
    //     for (size_t i = 0; i < size; i++) {
    //         prod[s][i] = prod[s][i] & ss->SHIFT[bitlength];
    //     }
    // }

    // memset(result, 0, sizeof(T) * size);
    // Open_Bitwise(result, prod, size, threadID, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     printf("[preOR]  prod [%lu]: %u\n", i, result[i]);
    //     print_binary(result[i], ring_size);
    // }

    for (size_t s = 0; s < numShares; s++) {
        for (size_t j = 0; j < bitlength - 1; j++) {
            for (size_t i = 0; i < size; i++) {
                z[s][j * size + i] = GET_BIT(prod[s][i], T(j)) ^ GET_BIT(prod[s][i], T(j + 1));
            }
        }
        for (size_t i = 0; i < size; i++) {
            z[s][(bitlength - 1) * size + i] = GET_BIT(prod[s][i], T(bitlength - 1));
        }
    }

    // printf("B2A\n");
    // reusing z
    Rss_B2A(z_res, z, bitlength * size, ring_size, net, ss);

    // T *result2 = new T[bitlength * size];
    // memset(result2, 0, sizeof(T) * bitlength*size);
    // Open_Bitwise(result, z, bitlength*size, threadID, net, ss);
    // for (size_t i = 0; i < bitlength*size; i++) {
    //     printf("[z2k]   [%lu]: %u\n", i, result[i]);
    //     print_binary(result[i], ring_size);
    // }

    for (size_t s = 0; s < numShares; s++) {
        for (size_t j = 0; j < bitlength; j++) {
            for (size_t i = 0; i < size; i++) {
                vp[s][i] += (1 << (bitlength - j - 1)) * z_res[s][j * size + i];
            }
        }
    }

    // memset(result, 0, sizeof(T) * size);
    // Open(result, vp, size, threadID, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     printf("[v_prime]   [%lu]: %u\n", i, result[i]);
    //     print_binary(result[i], ring_size);
    // }

    T **A_buff = new T *[numShares];
    T **B_buff = new T *[numShares];
    T **C_buff = new T *[numShares];
    for (size_t s = 0; s < numShares; s++) {
        A_buff[s] = new T[2 * size];
        B_buff[s] = new T[2 * size];
        C_buff[s] = new T[2 * size];

        memcpy(A_buff[s], x[s], sizeof(T) * size);
        memcpy(A_buff[s] + size, b_msb[s], sizeof(T) * size);

        memcpy(B_buff[s], vp[s], sizeof(T) * size);
        memcpy(B_buff[s] + size, vp[s], sizeof(T) * size);

        memset(C_buff[s], 0, sizeof(T) * 2 * size); // sanitizing destination
    }

    // performing (x*v) and (b_msb * v) in a batch, in that order
    Mult(C_buff, A_buff, B_buff, 2 * size, threadID, net, ss);

    // reverting the absolute value (?)
    // ******************************************************************************
    // WE MAY NEED TO CHOP OFF THE LEADING ring_size - bitlength bits
    // in order to preserve correctness for truncaiton in subsequent truncation
    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            v[s][i] = vp[s][i] - 2 * C_buff[s][size + i];
        }
    }
    // ******************************************************************************

    for (size_t s = 0; s < numShares; s++) {
        memcpy(c[s], C_buff[s], sizeof(T) * size);
    }

    // printf("\n");
    // memset(result, 0, sizeof(T) * size);
    // Open(result, c, size, threadID, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     printf("[c]   [%lu]: %u\n", i, result[i]);
    //     print_binary(result[i], ring_size);
    // }

    // printf("\n");
    // memset(result, 0, sizeof(T) * size);
    // Open(result, v, size, threadID, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     printf("[v]   [%lu]: %u\n", i, result[i]);
    //     print_binary(result[i], ring_size);
    // }

    delete[] ai;
    for (size_t i = 0; i < numShares; i++) {
        delete[] b_bits[i];
        delete[] prod[i];
        delete[] x[i];
        delete[] x_bits[i];
        delete[] z[i];
        delete[] z_res[i];
        delete[] vp[i];

        delete[] A_buff[i];
        delete[] B_buff[i];
        delete[] C_buff[i];
    }
    delete[] b_bits;
    delete[] x;
    delete[] prod;
    delete[] x_bits;
    delete[] z;
    delete[] z_res;

    delete[] A_buff;
    delete[] B_buff;
    delete[] C_buff;
}
#endif // _NORM_HPP_