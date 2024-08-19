
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

    static uint numShares = ss->getNumShares();

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
    for (size_t i = 0; i < numShares; i++) {
        b_msb[i] = new T[size];
        x[i] = new T[size];

        x_bits[i] = new T[size];
        memset(x_bits[i], 0, sizeof(T) * size);

        prod[i] = new T[size];
        memset(prod[i], 0, sizeof(T) * size);

        z_res[i] = new T[size * bitlength];
        memset(z_res[i], 0, sizeof(T) * bitlength * size);
        z[i] = new T[size * bitlength];
        memset(z[i], 0, sizeof(T) * bitlength * size);
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, T(1));

    Rss_BitDec(x_bits, b, bitlength, size, ring_size, net, ss);

    // sanitizing?
    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            x_bits[s][i] = x_bits[s][i] & ss->SHIFT[bitlength];
        }
    }

    // santiziing since we're dealing with bits
    for (size_t s = 0; s < numShares; s++) {
        memset(prod[s], 0, sizeof(T) * size);
    }

    // commputing the "reverse" prefixOR
    //  i.e., a_8, a_8 | a_7, a_8 | a_7 | a_6, ... (subscripts denote the bit position)
    // reusing prod
    Rss_PreOR(prod, x_bits, size, bitlength, net, ss);

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

    // reusing z
    Rss_B2A(z_res, z, bitlength * size, ring_size, net, ss);

    for (size_t s = 0; s < numShares; s++) {
        for (size_t j = 0; j < bitlength; j++) {
            for (size_t i = 0; i < size; i++) {
                v[s][i] += (T(1) << T(bitlength - j - 1)) * z_res[s][j * size + i];
            }
        }
    }

    // performing (x*v) and (b_msb * v) in a batch, in that order
    // Mult(C_buff, A_buff, B_buff, size, threadID, net, ss);
    Mult(c, b, v, size, threadID, net, ss);

    delete[] ai;
    for (size_t i = 0; i < numShares; i++) {
        delete[] b_bits[i];
        delete[] prod[i];
        delete[] x[i];
        delete[] x_bits[i];
        delete[] z[i];
        delete[] z_res[i];
    }
    delete[] b_bits;
    delete[] x;
    delete[] prod;
    delete[] x_bits;
    delete[] z;
    delete[] z_res;
}
#endif // _NORM_HPP_