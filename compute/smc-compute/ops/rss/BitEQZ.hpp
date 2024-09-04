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
#include "EQZ.hpp"

// inputs v_array and v_fixed are packed with individual bits of secrets a and b shared in Z_2
// v_fixed is a single value (still stored in a 2D array for consistency), which we are comparing to all of the values in v_array
// NOTE: output is a SINGLE BIT shared in Z_2, but stored in a full-sized priv_int T
template <typename T>
void BitEQZ_fixed(T **output, T **v_fixed, T **v_array, int size, int ring_size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    static uint numShares = ss->getNumShares();
    T **xor_res = new T *[numShares];
    for (size_t i = 0; i < numShares; i++) {
        xor_res[i] = new T[size];
    }
    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, -1);

    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            xor_res[s][i] = v_array[s][i] ^ v_fixed[s][0]; 
        }
    }
    Rss_k_OR_L(output, xor_res, size, ring_size, nodeNet, ss);

    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            output[s][i] = (T(1) & ai[s]) ^ output[s][i]; 
        }
    }

    delete[] ai;
    for (size_t i = 0; i < numShares; i++) {
        delete[] xor_res[i];
    }
    delete[] xor_res;
}

