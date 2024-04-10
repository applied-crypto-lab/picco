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

#ifndef _RANDBIT_HPP_
#define _RANDBIT_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "Input.hpp"
#include "Mult.hpp"
#include "MultSparse.hpp"
#include "bit_utils.hpp"
#include <cmath>
#include <numeric>

template <typename T>
void Rss_RandBit(T **res, uint ring_size, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    //  int n = ss->getPeers();
    int threshold = ss->getThreshold();
    int numParties = ss->getPeers();
    int id = ss->getID();
    uint numShares = ss->getNumShares();
    // passing 0 will always give us index of the nonzero share (as long as id is entitled to it):
    // n = 3 -> {1}
    // n = 5 -> {1,2}
    // n = 7 -> {1,2,3}
    static const int T_star_index = ss->generateT_star_index(0);
    // std::vector<std::vector<int>> send_recv_map = ss->generateB2A_map();

    // first t parties are the "input parties"
    // (3,1): p1
    // (5,2): p1, p2
    // (7,3): p1, p2, p3
    std::vector<int> input_parties(threshold);
    std::iota(input_parties.begin(), input_parties.end(), 1);

    T ***result = new T **[threshold];
    for (size_t s = 0; s < threshold; s++) {
        result[s] = new T *[numShares];
        for (size_t i = 0; i < numShares; i++) {
            result[s][i] = new T[size];
            memset(result[s][i], 0, sizeof(T) * size); // sanitizing destination
        }
    }

    T **a_sparse = new T *[numShares];
    T **w = new T *[numShares];
    for (size_t s = 0; s < numShares; s++) {
        a_sparse[s] = new T[size];
        w[s] = new T[size];
        memset(a_sparse[s], 0, sizeof(T) * size); // sanitizing destination
        memset(w[s], 0, sizeof(T) * size);        // sanitizing destination
    }

    // only t participants need to compute the XOR of a subset of their shares of [a]
    // mapping \xi predefined
    if (id < threshold + 1) {
        uint8_t *buffer = new uint8_t[size];
        ss->prg_getrandom(1, size, buffer);
        T *bits = new T[size];
        memset(bits, 0, sizeof(T) * size);
        for (size_t i = 0; i < size; i++) {
            bits[i] = GET_LSB(buffer); // check that this is correct/secure
        }
        Rss_Input_p_star(result, bits, input_parties, size, ring_size, nodeNet, ss);

        delete[] buffer; // not needed anymore
        delete[] bits;   // not needed anymore
    } else {
        // this means i (id) and the other t+1 parties are responsible for generating the random bit b_{\hat{T}}
        uint8_t *buffer = new uint8_t[size];
        ss->prg_getrandom(T_star_index, 1, size, buffer);
        T *bits = new T[size];
        memset(bits, 0, sizeof(T) * size);
        for (size_t i = 0; i < size; i++) {
            a_sparse[T_star_index][i] = GET_LSB(buffer); // check that this is correct/secure
        }

        // don't need to call sparsify here
        // ss->sparsify(a_sparse, a[T_star_index], size);
        Rss_Input_p_star(result, static_cast<T *>(nullptr), input_parties, size, ring_size, nodeNet, ss);

        delete[] buffer; // not needed anymore
    }

    // for Mult, we cant store the output in one of the variables we use for sparsify, since we need the original values to compute the XOR (after mult)
    switch (numParties) {
    case 3:
        Mult_Sparse(w, result[0], a_sparse, size, nodeNet, ss);
        for (uint s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                res[s][i] = result[0][s][i] + a_sparse[s][i] - 2 * w[s][i]; // XOR
            }
        }
        break;
    case 5:
        Mult_Sparse(w, result[0], a_sparse, size, nodeNet, ss);
        for (uint s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                w[s][i] = result[0][s][i] + a_sparse[s][i] - 2 * w[s][i]; // XOR
            }
        }

        Mult(a_sparse, result[1], w, size, nodeNet, ss);
        for (uint s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                res[s][i] = result[1][s][i] + w[s][i] - 2 * a_sparse[s][i]; // XOR
            }
        }

        break;
    case 7: {
        // pack result[0], result[1] into A (2*size)
        // pack x_sparse , result[2] into B_hat (2*size)

        T **A_buff = new T *[numShares];
        T **B_buff = new T *[numShares];
        T **C_buff = new T *[numShares];
        for (size_t s = 0; s < numShares; s++) {
            A_buff[s] = new T[2 * size];
            B_buff[s] = new T[2 * size];
            C_buff[s] = new T[2 * size];
            memcpy(A_buff[s], result[0][s], sizeof(T) * size);
            memcpy(A_buff[s] + size, result[1][s], sizeof(T) * size);
            memcpy(B_buff[s], a_sparse[s], sizeof(T) * size);
            memcpy(B_buff[s] + size, result[2][s], sizeof(T) * size);
            // result[0]*a_sparse
            // result[1]*result[2]
            memset(C_buff[s], 0, sizeof(T) * 2 * size); // sanitizing destination
        }
        // this can theoretically be done with a Mult_and_MultSparse special function
        Mult(C_buff, A_buff, B_buff, size, nodeNet, ss);

        for (uint s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                w[s][i] = A_buff[s][i] + B_buff[s][i] - 2 * C_buff[s][i]; // XOR
            }
            for (size_t i = 0; i < size; i++) {
                a_sparse[s][i] = A_buff[s][size + i] + B_buff[s][size + i] - 2 * C_buff[s][size + i]; // XOR, reusing a_sparse
            }
        }
        // reusing half of C_buff (not needed anymore)
        Mult(C_buff, a_sparse, w, size, nodeNet, ss);
        for (uint s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                res[s][i] = a_sparse[s][i] + w[s][i] - 2 * C_buff[s][i]; // XOR
            }
        }

        for (size_t i = 0; i < numShares; i++) {
            delete[] A_buff[i];
            delete[] B_buff[i];
            delete[] C_buff[i];
        }
        delete[] A_buff;
        delete[] B_buff;
        delete[] C_buff;
        // run MultSparse
        break;
    }
    default:
        break;
    }

    // cleanup
    for (size_t s = 0; s < threshold; s++) {
        for (size_t i = 0; i < numShares; i++) {
            delete[] result[s][i];
        }
        delete[] result[s];
    }
    delete[] result;

    for (size_t i = 0; i < numShares; i++) {
        delete[] w[i];
        delete[] a_sparse[i];
    }
    delete[] w;
    delete[] a_sparse;
}

#endif // _B2A_HPP_