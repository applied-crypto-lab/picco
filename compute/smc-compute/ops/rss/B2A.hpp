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
#include "Input.hpp"
#include "Mult.hpp"
#include "MultSparse.hpp"
#include <cassert>
#include <cmath>
#include <numeric>
#include <string>
/*
3pc xi map:
p1 : [{2}, {3}]

5pc xi map:
p1 :  [{2,3}, {2,4}, {2,5}, {3,4}, {3,5}]
p2 :  [{1,3}, {1,4}, {1,5}, {4,5}]

7pc xi map:
p1 : [(2, 3, 4), (2, 3, 5), (2, 3, 6), (2, 3, 7), (2, 4, 6), (2, 4, 7), (2, 5, 6), (2, 5, 7), (2, 6, 7), (4, 6, 7), (3, 5, 7)]
p2 : [(3, 4, 5), (3, 4, 6), (3, 4, 7), (1, 3, 4), (3, 5, 6), (1, 3, 5), (3, 6, 7), (1, 3, 6), (1, 3, 7), (1, 5, 7), (1, 6, 7)]
p3 : [(4, 5, 6), (4, 5, 7), (1, 4, 5), (2, 4, 5), (1, 4, 6), (1, 4, 7), (1, 2, 4), (5, 6, 7), (1, 5, 6), (1, 2, 5), (1, 2, 6), (1, 2, 7)]
 */
// [a] is a secret bit shared in Z_2 (stored in a T)
template <typename T>
void Rss_B2A(T **res, T **a, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    assertm((ring_size == ss->ring_size ) , "checking ring_size argument == ss->ring_size" );

    //  int n = ss->getPeers();
    static int threshold = ss->getThreshold();
    static int numParties = ss->getPeers();
    static int id = ss->getID();
    static uint numShares = ss->getNumShares();
    // passing 0 will always give us index of the nonzero share:
    // n = 3 -> {1}
    // n = 5 -> {1,2}
    // n = 7 -> {1,2,3}
    static const int T_star_index = ss->generateT_star_index(0);
    // std::cout << "T_star_index = "<<T_star_index << std::endl;
    // std::vector<std::vector<int>> send_recv_map = ss->generateB2A_map();

    // this map is written such that the workload is distributed as evenly as possible across the input parties (perfectly even distribution is mathematically impossible)
    // std::vector<std::vector<int>> xi_map = ;

    // first t parties are the "input parties"
    // (3,1): p1
    // (5,2): p1, p2
    // (7,3): p1, p2, p3
    std::vector<int> input_parties(threshold);
    std::iota(input_parties.begin(), input_parties.end(), 1);
    // std::cout << "input_parties = "<< input_parties << std::endl;
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
        // std::cout << id << " is an input party" << std::endl;
        std::vector<int> xi_map = ss->generateXi_map();
        // std::cout << "xi_map : " << xi_map << std::endl;
        T *xors = new T[size];
        memset(xors, 0, sizeof(T) * size);
        for (auto idx : xi_map) {
            // doing it this way since each share array of a is a contiguous block
            // can just swap loop order later on if performance is better?
            // this is better, since all of a[idx] can be loaded into the cache
            for (size_t i = 0; i < size; i++) {
                xors[i] ^= a[idx][i];
            }
        }

        Rss_Input_p_star(result, xors, input_parties, size, ring_size, nodeNet, ss);

        delete[] xors; // not needed anymore
    } else {
        // std::cout << id << " is NOT an input party" << std::endl;
        // performing sparsiy "while" parties <= t compute their xors
        // this means i (id) have access to the last share of a
        // call sparsify on the share at index (T_star_index)
        ss->sparsify(a_sparse, a[T_star_index], size);
        Rss_Input_p_star(result, static_cast<T *>(nullptr), input_parties, size, ring_size, nodeNet, ss);
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

        // reusing a_sparse
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
        Mult(C_buff, A_buff, B_buff, 2*size, nodeNet, ss);

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

