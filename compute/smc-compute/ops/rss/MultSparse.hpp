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

#ifndef _MULTSPARSE_HPP_
#define _MULTSPARSE_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"

using std::vector;

/* 
This header contains implementations of the functionality MultSparse([a], [b_hat]), where the shares of [b_hat] are "sparese"
Definition: A secret is denoted as "sparsely distributed" iff all but one of the shares are zero.
e.g. in (3,1), [b_hat] is considered sparse if b_hat_{2} = b_hat_{3} = 0, and b_hat_{1} is some value in the ring R

marked shares for all computational setups:
(3,1): \hat{T}  = {1}
(5,2): \hat{T}  = {1,2}
(7,3): \hat{T}  = {1,2,3}
*/

// \hat{T}  = {1}
template <typename T>
void Rss_Mult_Sparse_3pc(T **c, T **a, T **b_hat, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // uint bytes = (nodeNet.RING[ring_size] + 7) >> 3;
    uint bytes = (ring_size + 7) >> 3;
    int i;
    T *v = new T[size];
    int id = ss->getID();
    int n = ss->getPeers();
    vector<vector<int>> mul_map = ss->generate_MultSparse_map(n, id);


    uint8_t *buffer = new uint8_t[bytes * size];
    ss->prg_getrandom(1, bytes, size, buffer);

    for (i = 0; i < size; i++) {
        memcpy(c[0] + i, buffer + i * bytes, bytes);
        v[i] = a[0][i] * b_hat[0][i] + a[0][i] * b_hat[1][i] + a[1][i] * b_hat[0][i] - c[0][i];
    }

    // map is different from regular multiplication
    // For \hat{T} = 1:
    // p1 only receives from p2, doesnt send
    // p3 only sends to p2, doesnt receive
    // p2 sends to p1 and receives from p3
    nodeNet.SendAndGetDataFromPeer(v, c[1], size, ring_size, mul_map);
    ss->prg_getrandom(0, bytes, size, buffer);

    for (i = 0; i < size; i++) {
        c[1][i] = c[1][i] + c[0][i];
        memcpy(c[0] + i, buffer + i * bytes, bytes);
        c[0][i] = c[0][i] + v[i];
    }

    // free
    delete[] v;
    delete[] buffer;
}


#endif // _MULTSPARSE_HPP_