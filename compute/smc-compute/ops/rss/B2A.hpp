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

#ifndef _B2A_HPP_
#define _B2A_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "Input.hpp"
#include <cmath>
#include <numeric>
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
// [a] is bitwise-shared
template <typename T>
void Rss_B2A(T **res, T **a, uint ring_size, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    //  int n = ss->getPeers();
    int threshold = ss->getThreshold();
    int id = ss->getID();
    uint numShares = ss->getNumShares();
    std::vector<std::vector<int>> send_recv_map = ss->generateB2A_map();
    std::vector<std::vector<int>> xi_map = ss->generateXi_map();

    // this map is written such that the workload is distributed as evenly as possible across the input parties (perfectly even distribution is mathematically impossible)
    // std::vector<std::vector<int>> xi_map = ;

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

    // only t participants need to compute the XOR of a subset of their shares of [a]
    // mapping \xi predefined
    if (id < threshold + 1) {

        // Rss_Input_p_star(result, computed_xors, input_parties, size, ring_size, nodeNet, ss);
    } else {

        // Rss_Input_p_star(result, static_cast<priv_int>(nullptr), input_parties, size, ring_size, nodeNet, ss);
    }

    // these maps need to be revised (taken originally from edabit, where we needed t+1 input parties)
    // for B2A, only need t input parties

    // cleanup
    for (size_t i = 0; i < numShares; i++) {
        for (size_t j = 0; j < threshold; j++) {
            delete[] result[i][j];
        }
        delete[] result[i];
    }
    delete[] result;
}

#endif // _B2A_HPP_