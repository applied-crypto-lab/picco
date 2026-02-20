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

// Internal function - generates one random bit share (no replication)
// This is the core RandBit protocol without statistical security amplification
template <typename T>
void Rss_RandBit_Internal(T **res, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    //  int n = ss->getPeers();
    int threshold = ss->getThreshold();
    int numParties = ss->getPeers();
    int id = ss->getID();
    uint numShares = ss->getNumShares();
    // passing 0 will always give us index of the nonzero share (as long as id is entitled to it):
    // n = 3 -> {1}
    // n = 5 -> {1,2}
    // n = 7 -> {1,2,3}
    const int T_star_index = ss->generateT_star_index(0);
    // std::vector<std::vector<int>> send_recv_map = ss->generateB2A_map();

    // first t parties are the "input parties"
    // (3,1): p1
    // (5,2): p1, p2
    // (7,3): p1, p2, p3
    std::vector<int> input_parties(threshold);
    std::iota(input_parties.begin(), input_parties.end(), 1);

    T ***result = new T **[threshold];
    for (int s = 0; s < threshold; s++) {
        result[s] = new T *[size];
        for (uint i = 0; i < size; i++) {
            result[s][i] = new T[numShares];
            memset(result[s][i], 0, sizeof(T) * numShares); // sanitizing destination
        }
    }

    T **a_sparse = new T *[size];
    T **w = new T *[size];
    for (uint i = 0; i < size; i++) {
        a_sparse[i] = new T[numShares];
        w[i] = new T[numShares];
        memset(a_sparse[i], 0, sizeof(T) * numShares); // sanitizing destination
        memset(w[i], 0, sizeof(T) * numShares);        // sanitizing destination
    }

    // only t participants need to compute the XOR of a subset of their shares of [a]
    // mapping \xi predefined
    if (id < threshold + 1) {
        uint8_t *buffer = new uint8_t[size];
        ss->prg_getrandom(1, size, buffer); // generating (size) bytes into buffer
        T *bits = new T[size];
        memset(bits, 0, sizeof(T) * size);
        for (uint i = 0; i < size; i++) {
            bits[i] = GET_LSB(buffer[i]); // check that this is correct/secure
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
        for (uint i = 0; i < size; i++) {
            a_sparse[i][T_star_index] = GET_LSB(buffer[i]); // check that this is correct/secure
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
        for (uint i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                res[i][s] = result[0][i][s] + a_sparse[i][s] - 2 * w[i][s]; // XOR
            }
        }
        break;
    case 5:
        Mult_Sparse(w, result[0], a_sparse, size, nodeNet, ss);
        // Interface format [size][numShares]: w[i][s], result[0][i][s], a_sparse[i][s]
        for (uint i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                w[i][s] = result[0][i][s] + a_sparse[i][s] - 2 * w[i][s]; // XOR
            }
        }

        Mult(a_sparse, result[1], w, size, nodeNet, ss);
        for (uint i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                res[i][s] = result[1][i][s] + w[i][s] - 2 * a_sparse[i][s]; // XOR
            }
        }

        break;
    case 7: {
        // pack result[0], result[1] into A (2*size)
        // pack a_sparse, result[2] into B (2*size)
        // Use interface format [2*size][numShares]

        T **A_buff = new T *[2 * size];
        T **B_buff = new T *[2 * size];
        T **C_buff = new T *[2 * size];
        for (uint i = 0; i < 2 * size; i++) {
            A_buff[i] = new T[numShares];
            B_buff[i] = new T[numShares];
            C_buff[i] = new T[numShares];
            memset(C_buff[i], 0, sizeof(T) * numShares);
        }
        // Pack in interface format - result arrays are already in interface format [size][numShares]
        for (uint i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                A_buff[i][s] = result[0][i][s];
                A_buff[size + i][s] = result[1][i][s];
                B_buff[i][s] = a_sparse[i][s];
                B_buff[size + i][s] = result[2][i][s];
            }
        }
        // this can theoretically be done with a Mult_and_MultSparse special function
        Mult(C_buff, A_buff, B_buff, 2 * size, nodeNet, ss);

        for (uint i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                w[i][s] = A_buff[i][s] + B_buff[i][s] - 2 * C_buff[i][s]; // XOR
                a_sparse[i][s] = A_buff[size + i][s] + B_buff[size + i][s] - 2 * C_buff[size + i][s]; // XOR, reusing a_sparse
            }
        }
        // reusing half of C_buff (not needed anymore)
        Mult(C_buff, a_sparse, w, size, nodeNet, ss);
        for (uint i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                res[i][s] = a_sparse[i][s] + w[i][s] - 2 * C_buff[i][s]; // XOR
            }
        }

        for (uint i = 0; i < 2 * size; i++) {
            delete[] A_buff[i];
            delete[] B_buff[i];
            delete[] C_buff[i];
        }
        delete[] A_buff;
        delete[] B_buff;
        delete[] C_buff;
        break;
    }
    default:
        break;
    }

    // cleanup
    for (int s = 0; s < threshold; s++) {
        for (uint i = 0; i < size; i++) {
            delete[] result[s][i];
        }
        delete[] result[s];
    }
    delete[] result;

    for (uint i = 0; i < size; i++) {
        delete[] w[i];
        delete[] a_sparse[i];
    }
    delete[] w;
    delete[] a_sparse;
}

// Public interface - simple wrapper around the internal RandBit function
// Note: 3x replication was removed as it's not needed for RSS security.
// The actual security concerns are in comparison/truncation operations.
template <typename T>
void Rss_RandBit(T **res, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    Rss_RandBit_Internal(res, size, ring_size, nodeNet, ss);
}

#endif // _RANDBIT_HPP_