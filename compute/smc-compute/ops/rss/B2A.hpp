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
// Both input (a) and output (res) use interface format [size][numShares]
template <typename T>
void Rss_B2A(T **res, T **a, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    assertm((ring_size == ss->ring_size ) , "checking ring_size argument == ss->ring_size" );

    //  int n = ss->getPeers();
    int threshold = ss->getThreshold();
    int numParties = ss->getPeers();
    int id = ss->getID();
    uint numShares = ss->getNumShares();
    // passing 0 will always give us index of the nonzero share:
    // n = 3 -> {1}
    // n = 5 -> {1,2}
    // n = 7 -> {1,2,3}
    const int T_star_index = ss->generateT_star_index(0);
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
    // result uses interface format [threshold][size][numShares] as expected by Rss_Input_p_star
    T ***result = new T **[threshold];
    for (size_t s = 0; s < (size_t)threshold; s++) {
        result[s] = new T *[size];
        for (size_t i = 0; i < size; i++) {
            result[s][i] = new T[numShares];
            memset(result[s][i], 0, sizeof(T) * numShares); // sanitizing destination
        }
    }

    // a_sparse and w use interface format [size][numShares] for Mult_Sparse
    T **a_sparse = new T *[size];
    T **w = new T *[size];
    for (size_t i = 0; i < size; i++) {
        a_sparse[i] = new T[numShares];
        w[i] = new T[numShares];
        memset(a_sparse[i], 0, sizeof(T) * numShares); // sanitizing destination
        memset(w[i], 0, sizeof(T) * numShares);        // sanitizing destination
    }

    // only t participants need to compute the XOR of a subset of their shares of [a]
    // mapping \xi predefined
    // All parties need to sparsify their share for the multiplication
    // sparsify_from_interface: input a is in interface format [size][numShares], output a_sparse also in interface format
    // We need to extract T_star_index share from each element and sparsify it
    T *a_T_star = new T[size];
    memset(a_T_star, 0, sizeof(T) * size);  // Initialize to zero
    // Only extract if party has access (T_star_index >= 0)
    if (T_star_index >= 0) {
        for (size_t i = 0; i < size; i++) {
            a_T_star[i] = a[i][T_star_index];
        }
    }

    // Temporary array for sparsify output in internal format [numShares][size]
    T **a_sparse_internal = new T *[numShares];
    for (size_t s = 0; s < numShares; s++) {
        a_sparse_internal[s] = new T[size];
        memset(a_sparse_internal[s], 0, sizeof(T) * size);
    }
    ss->sparsify(a_sparse_internal, a_T_star, size);
    // Convert a_sparse_internal [numShares][size] to a_sparse [size][numShares]
    for (size_t i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            a_sparse[i][s] = a_sparse_internal[s][i];
        }
    }
    delete[] a_T_star;
    // Clean up a_sparse_internal
    for (size_t s = 0; s < numShares; s++) {
        delete[] a_sparse_internal[s];
    }
    delete[] a_sparse_internal;

    if (id < threshold + 1) {
        // std::cout << id << " is an input party" << std::endl;
        std::vector<int> xi_map = ss->generateXi_map();
        // std::cout << "xi_map : " << xi_map << std::endl;
        T *xors = new T[size];
        memset(xors, 0, sizeof(T) * size);
        for (auto idx : xi_map) {
            // a is in interface format [size][numShares]
            for (size_t i = 0; i < size; i++) {
                xors[i] ^= a[i][idx];
            }
        }

        Rss_Input_p_star(result, xors, input_parties, size, ring_size, nodeNet, ss);

        delete[] xors; // not needed anymore
    } else {
        // std::cout << id << " is NOT an input party" << std::endl;
        Rss_Input_p_star(result, static_cast<T *>(nullptr), input_parties, size, ring_size, nodeNet, ss);
    }

    // for Mult, we cant store the output in one of the variables we use for sparsify, since we need the original values to compute the XOR (after mult)
    // result[0] is now in interface format [size][numShares], can use directly with Mult_Sparse

    switch (numParties) {
    case 3:
        Mult_Sparse(w, result[0], a_sparse, size, nodeNet, ss);

        // w, a_sparse, and result[0] are all in interface format [size][numShares]
        // res is also in interface format [size][numShares]
        for (size_t i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                res[i][s] = result[0][i][s] + a_sparse[i][s] - 2 * w[i][s]; // XOR
            }
        }
        break;
    case 5: {
        // result[0] and result[1] are in interface format [size][numShares]
        Mult_Sparse(w, result[0], a_sparse, size, nodeNet, ss);

        // w and a_sparse are in interface format [size][numShares]
        for (size_t i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                w[i][s] = result[0][i][s] + a_sparse[i][s] - 2 * w[i][s]; // XOR
            }
        }

        // reusing a_sparse
        Mult(a_sparse, result[1], w, size, nodeNet, ss);

        for (size_t i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                res[i][s] = result[1][i][s] + w[i][s] - 2 * a_sparse[i][s]; // XOR
            }
        }
        break;
    }
    case 7: {
        // result[0], result[1], result[2] are in interface format [size][numShares]
        // pack result[0], result[1] into A (2*size)
        // pack x_sparse , result[2] into B_hat (2*size)

        // Use interface format [2*size][numShares]
        T **A_buff = new T *[2 * size];
        T **B_buff = new T *[2 * size];
        T **C_buff = new T *[2 * size];
        for (size_t i = 0; i < 2 * size; i++) {
            A_buff[i] = new T[numShares];
            B_buff[i] = new T[numShares];
            C_buff[i] = new T[numShares];
            memset(C_buff[i], 0, sizeof(T) * numShares);
        }
        // Pack in interface format - result arrays are already in interface format
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                A_buff[i][s] = result[0][i][s];
                A_buff[size + i][s] = result[1][i][s];
                B_buff[i][s] = a_sparse[i][s];
                B_buff[size + i][s] = result[2][i][s];
            }
        }
        // this can theoretically be done with a Mult_and_MultSparse special function
        Mult(C_buff, A_buff, B_buff, 2*size, nodeNet, ss);

        for (size_t i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                w[i][s] = A_buff[i][s] + B_buff[i][s] - 2 * C_buff[i][s]; // XOR
                a_sparse[i][s] = A_buff[size + i][s] + B_buff[size + i][s] - 2 * C_buff[size + i][s]; // XOR, reusing a_sparse
            }
        }
        // reusing half of C_buff (not needed anymore)
        Mult(C_buff, a_sparse, w, size, nodeNet, ss);
        for (size_t i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                res[i][s] = a_sparse[i][s] + w[i][s] - 2 * C_buff[i][s]; // XOR
            }
        }

        for (size_t i = 0; i < 2 * size; i++) {
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

    // cleanup - result is [threshold][size][numShares]
    for (size_t s = 0; s < (size_t)threshold; s++) {
        for (size_t i = 0; i < size; i++) {
            delete[] result[s][i];
        }
        delete[] result[s];
    }
    delete[] result;

    // w and a_sparse are in interface format [size][numShares]
    for (size_t i = 0; i < size; i++) {
        delete[] w[i];
        delete[] a_sparse[i];
    }
    delete[] w;
    delete[] a_sparse;
}
