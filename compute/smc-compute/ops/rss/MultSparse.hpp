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
#include "Open.hpp"

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
    T *res = new T[size];
    memset(v, 0, sizeof(T) * size);
    memset(res, 0, sizeof(T) * size);

    int id = ss->getID();
    int n = ss->getPeers();

    printf("id : %i\n", id);
    printf("n : %i\n", n);

    vector<vector<int>> mul_map = ss->generate_MultSparse_map(n, id);
    std::cout << "mul_map :" << mul_map << std::endl;
    uint8_t *buffer = new uint8_t[bytes * size];

    ss->prg_getrandom(1, bytes, size, buffer);
    for (i = 0; i < size; i++) {
        // printf("c[0][%i] : %u\n", i, c[0][i]);
        if (id != 1) {
            memcpy(c[0] + i, buffer + i * bytes, bytes);
            // printf("if1\n");
            v[i] = a[0][i] * b_hat[0][i] + a[0][i] * b_hat[1][i] + a[1][i] * b_hat[0][i];
            printf("v[%i] : %u\n", i, v[i]);
            printf("prg1 c[0][%i] : %u\n", i, c[0][i]);

            v[i] -= c[0][i];
            printf("v[%i] - c[0] : %u\n", i, v[i]);
        }
    }
    // p1: c[0] contains G_3 at this point
    // p2: c[0] contains G_1 at this point
    // p3: c[0] contains G_2 at this point

    // map is different from regular multiplication
    // For \hat{T} = 1:
    // p1 only receives from p2, doesnt send
    // p3 only sends to p2, doesnt receive
    // p2 sends to p1 and receives from p3
    // nodeNet.SendAndGetDataFromPeer(v, c[1], size, ring_size, ss->general_map);
    nodeNet.SendAndGetDataFromPeer(v, c[1], size, ring_size, mul_map);

    // p1: c[1] contains u_3 at this point (received from p2)
    // p2: c[1] contains w_1 at this point (recieved from p3)
    // p3: c[1] contains 0 at this point (nothing)

    // p1 : G_2
    // p2 : G_3
    // p3 : G_1
    // for (int i = 0; i < size; i++) {
    //     printf("v[%i] : %u\n", i, v[i]);
    // }

    // for (int i = 0; i < size; i++) {
    //     printf("c[0][%i] : %u\n", i, c[0][i]);
    //     printf("c[1][%i] : %u\n", i, c[1][i]);
    // }
    // printf("\n");
    //  c[1] STORES THE RECEIVED VALUE
    ss->prg_getrandom(0, bytes, size, buffer);

    for (i = 0; i < size; i++) {
        if (id == 1) {
            c[1][i] = c[1][i]; // do nothing
        } else if (id == 2) {
            c[1][i] = c[1][i] + c[0][i]; // u' + u'' (in this order)
            c[0][i] = v[i]; // u' + u'' (in this order)
        } else if (id == 3) {
            c[1][i] = c[0][i]; // c[0] stores [u]^1 at this point
        }

        if (id == 3 or id == 1) {

            memcpy(c[0] + i, buffer + i * bytes, bytes);
            printf("prg2 c[0][%i] : %u\n", i, c[0][i]);
            c[0][i] = c[0][i] + v[i];
        }

        // if (id == 1) {
        //     // c[1][i] = c[0][i]; // only p3 does this, since they never recieved anything from into c[1]
        // } else if (id == 2) {
        //     // p1 computes c_3 = u_3 + v_3 (v_3 = G_3)
        //     // p2 computes c_1 = u_1 + w_1 (u_1 = G_1)
        //     c[1][i] = c[1][i] + c[0][i]; // c[1] is received
        // } else if (id == 3) {
        //     c[1][i] = c[1][i];
        // }
        // if (id != 1) {

        //     memcpy(c[0] + i, buffer + i * bytes, bytes);

        //     // p1: c_3 = v_3 + G_3
        //     // p2: c_1 = w_1 + G_1
        //     // p3: c_2 = v_2 + G_2

        //     // p1: c[0] contains G_2 at this point
        //     // p2: c[0] contains G_3 at this point
        //     // p3: c[0] contains G_1 at this point

        //     c[0][i] = c[0][i] + v[i]; // v[i] --> v_2, w_1, u_3
        // }
    }

    for (int i = 0; i < size; i++) {
        printf("c[0][%i] : %u\n", i, c[0][i]);
        printf("c[1][%i] : %u\n", i, c[1][i]);
    };

    printf("---end\n");

    Rss_Open_3pc(res, c, size, ring_size, nodeNet, ss);
    for (size_t i = 0; i < size; i++) {
        printf("res [%lu]: %u\n", i, res[i]);
    }

    // free
    delete[] v;
    delete[] res;
    delete[] buffer;
}

#endif // _MULTSPARSE_HPP_