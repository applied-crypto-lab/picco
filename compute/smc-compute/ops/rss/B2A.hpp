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

// [a] is bitwise-shared
template <typename T>
void Rss_B2A(T **res, T **a, uint ring_size, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    //  int n = ss->getPeers();
    int threshold = ss->getThreshold();
    int id = ss->getID();
    uint numShares = ss->getNumShares();
    std::vector<std::vector<int>> send_recv_map = ss->generateB2A_map();

    // this map is written such that the workload is distributed as evenly as possible across the input parties (perfectly even distribution is mathematically impossible)
    std::vector<std::vector<int>> xi_map = ss->generateB2A_map();

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

        // Rss_Input_p_star(result, computed_xors, input_parties, send_recv_map, size, ring_size, nodeNet, ss);
    } else {

        // Rss_Input_p_star(result, NULL, input_parties, send_recv_map, size, ring_size, nodeNet, ss);
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