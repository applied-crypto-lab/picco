
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
#include "Open.hpp"

template <typename T>
void doOperation_Norm(T **c, T **v, T **b, int bitlength, int size, uint ring_size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {

    uint numShares = ss->getNumShares();

    T **x_bits = new T *[size];
    T **prod = new T *[size];
    for (size_t i = 0; i < (size_t)size; i++) {
        x_bits[i] = new T[numShares];
        memset(x_bits[i], 0, sizeof(T) * numShares);

        prod[i] = new T[numShares];
        memset(prod[i], 0, sizeof(T) * numShares);

        // Initialize v[i] to zero - critical for += accumulation later
        memset(v[i], 0, sizeof(T) * numShares);
    }

    T **z = new T *[size * bitlength];
    T **z_res = new T *[size * bitlength];
    for (size_t i = 0; i < (size_t)(size * bitlength); i++) {
        z_res[i] = new T[numShares];
        memset(z_res[i], 0, sizeof(T) * numShares);
        z[i] = new T[numShares];
        memset(z[i], 0, sizeof(T) * numShares);
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, T(1));

    Rss_BitDec(x_bits, b, bitlength, size, ring_size, net, ss);

    // sanitizing?
    for (size_t i = 0; i < (size_t)size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            x_bits[i][s] = x_bits[i][s] & ss->SHIFT[bitlength];
        }
    }

    // santiziing since we're dealing with bits
    for (size_t i = 0; i < (size_t)size; i++) {
        memset(prod[i], 0, sizeof(T) * numShares);
    }

    // commputing the "reverse" prefixOR
    //  i.e., a_8, a_8 | a_7, a_8 | a_7 | a_6, ... (subscripts denote the bit position)
    // reusing prod
    Rss_PreOR(prod, x_bits, size, bitlength, net, ss);

    for (size_t j = 0; j < (size_t)(bitlength - 1); j++) {
        for (size_t i = 0; i < (size_t)size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                z[j * size + i][s] = GET_BIT(prod[i][s], T(j)) ^ GET_BIT(prod[i][s], T(j + 1));
            }
        }
    }
    for (size_t i = 0; i < (size_t)size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            z[(bitlength - 1) * size + i][s] = GET_BIT(prod[i][s], T(bitlength - 1));
        }
    }

    // reusing z
    Rss_B2A(z_res, z, bitlength * size, ring_size, net, ss);

    for (size_t j = 0; j < (size_t)bitlength; j++) {
        for (size_t i = 0; i < (size_t)size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                v[i][s] += (T(1) << T(bitlength - j - 1)) * z_res[j * size + i][s];
            }
        }
    }

    // performing (x*v) and (b_msb * v) in a batch, in that order
    // Mult(C_buff, A_buff, B_buff, size, threadID, net, ss);
    Mult(c, b, v, size, threadID, net, ss);

    delete[] ai;
    for (size_t i = 0; i < (size_t)size; i++) {
        delete[] prod[i];
        delete[] x_bits[i];
    }
    for (size_t i = 0; i < (size_t)(size * bitlength); i++) {
        delete[] z[i];
        delete[] z_res[i];
    }
    delete[] prod;
    delete[] x_bits;
    delete[] z;
    delete[] z_res;
}
#endif // _NORM_HPP_