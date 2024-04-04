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
#include "Mult.hpp"
#include "Open.hpp"

using std::vector;

/*
This header contains implementations of the functionality MultSparse([a], [b_hat]), where the shares of [b_hat] are "sparse"
Definition: A secret is denoted as "sparsely distributed" iff all but one of the shares are zero.
e.g. in (3,1), [b_hat] is considered sparse if b_hat_{2} = b_hat_{3} = 0, and b_hat_{1} is some value in the ring R

marked shares for all computational setups:
(3,1): \hat{T}  = {1}
(5,2): \hat{T}  = {1,2}
(7,3): \hat{T}  = {1,2,3}
*/

// \hat{T}  = {1}
// this is an implementation of the second half of the optimized B2A protocol (Algorithm 6) from Blanton et al's 2023 paper "Secure and Accurate Summation of Many Floating-Point Numbers"
// in the paper, the authors assume \hat{T} = {3}
template <typename T>
void Rss_Mult_Sparse_3pc(T **c, T **a, T **b_hat, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // uint bytes = (nodeNet.RING[ring_size] + 7) >> 3;
    uint bytes = (ring_size + 7) >> 3;
    int i;
    T *v = new T[size];
    memset(v, 0, sizeof(T) * size);

    int id = ss->getID();
    int n = ss->getPeers();
    vector<vector<int>> mul_map = ss->generate_MultSparse_map(n, id);
    uint8_t *buffer = new uint8_t[bytes * size];

    ss->prg_getrandom(1, bytes, size, buffer); // all parties do this, even if not needed (for consistency in future prg invocations)

    if (id != 1) {
        for (i = 0; i < size; i++) {
            memcpy(c[0] + i, buffer + i * bytes, bytes);
            v[i] = a[0][i] * b_hat[0][i] + a[0][i] * b_hat[1][i] + a[1][i] * b_hat[0][i];
            v[i] -= c[0][i];
        }
    } else {
        for (i = 0; i < size; i++) {
            memcpy(c[0] + i, buffer + i * bytes, bytes); // this is the same thing as the earlier memcpy, except inserted here as to not have a
            // c[0][i] = c[0][i];
        }
    }
    nodeNet.SendAndGetDataFromPeer(v, c[1], size, ring_size, mul_map);
    ss->prg_getrandom(0, bytes, size, buffer); // all parties do this, even if not needed (for consistency in future prg invocations)

    switch (id) {
    case 1:
        for (i = 0; i < size; i++) {
            memcpy(c[0] + i, buffer + i * bytes, bytes); // uses a different prg from earlier (which p1 doesnt touch regardless)
            // c[0][i] = c[0][i];
        }
        break;
    case 2:
        for (i = 0; i < size; i++) {
            c[1][i] = c[1][i] + c[0][i]; // u' + u'' (in this order)
            c[0][i] = v[i];              //
        }
        break;

    case 3:
        for (i = 0; i < size; i++) {
            c[1][i] = c[0][i]; // c[0] stores [u]^1 at this point
            memcpy(c[0] + i, buffer + i * bytes, bytes);
            c[0][i] = c[0][i] + v[i]; // u' + G_3
        }
        break;
    default:
        break;
    }

    // free
    delete[] v;
    delete[] buffer;
}

//(5,2): \hat{T}  = {1,2}
template <typename T>
void Rss_Mult_Sparse_5pc(T **c, T **a, T **b_hat, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    vector<int> T_hat = {1, 2};

    uint bytes = (ring_size + 7) >> 3;
    uint i, p_prime, T_index;
    uint numShares = ss->getNumShares();
    uint numParties = ss->getPeers();
    uint threshold = ss->getThreshold();
    int pid = nodeNet.getID();
    vector<vector<int>> mul_map = ss->generate_MultSparse_map(numParties, pid);
    T *v = new T[size];

    std::vector<uint8_t> prg_ctrs(6);
    switch (pid) {
    case 1:
        prg_ctrs = {2, 2, 2, 1, 1, 0};
        break;
    case 2:
        prg_ctrs = {1, 1, 2, 0, 2, 1};
        break;
    case 3:
        prg_ctrs = {0, 2, 2, 1, 2, 2};
        break;
    case 4:
        prg_ctrs = {1, 2, 1, 2, 2, 2};
        break;
    case 5:
        prg_ctrs = {2, 2, 2, 2, 2, 1};
        break;
    }

    T **recv_buf = new T *[threshold];
    for (i = 0; i < threshold; i++) {
        recv_buf[i] = new T[size];
        memset(recv_buf[i], 0, sizeof(T) * size);
    }

    uint8_t **buffer = new uint8_t *[numShares];
    for (i = 0; i < numShares; i++) {
        buffer[i] = new uint8_t[prg_ctrs[i] * bytes * size];
        if (prg_ctrs[i] > 0) {
            ss->prg_getrandom(i, bytes, prg_ctrs[i] * size, buffer[i]);
        }

        // sanitizing destination (just in case)
        memset(c[i], 0, sizeof(T) * size);
    }

    T z = T(0);
    uint tracker;
    // only p \nin T_star computes v
    if (!ss->pid_in_T(pid, T_hat)) {
        // std::cout << pid << " not in " << T_hat << std::endl;

        for (i = 0; i < size; i++) {
            v[i] = a[0][i] * (b_hat[0][i] + b_hat[1][i] + b_hat[2][i] + b_hat[3][i] + b_hat[4][i] + b_hat[5][i]) +
                   a[1][i] * (b_hat[0][i] + b_hat[1][i] + b_hat[2][i] + b_hat[3][i] + b_hat[4][i] + b_hat[5][i]) +
                   a[2][i] * (b_hat[1][i] + b_hat[3][i]) +
                   a[3][i] * (b_hat[0][i] + b_hat[2][i]) +
                   a[4][i] * (b_hat[0][i] + b_hat[1][i]) +
                   a[5][i] * (b_hat[0][i] + b_hat[4][i]);
        }
    }

    // printf("finished calculating v\n");
    for (p_prime = 1; p_prime < numParties + 1; p_prime++) {
        if (!ss->pid_in_T(p_prime, T_hat)) {
            for (T_index = 0; T_index < numShares; T_index++) {
                tracker = 0;
                if ((p_prime != (pid)) and
                    (!(ss->pid_in_T(p_prime, ss->T_map_mpc[T_index]))) and
                    (!(ss->chi_pid_is_T(p_prime, ss->T_map_mpc[T_index])))) {
                    for (i = 0; i < size; i++) {
                        memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                        c[T_index][i] += z;
                    }
                    tracker += 1;
                } else if ((p_prime == pid) and (!(ss->chi_pid_is_T(pid, ss->T_map_mpc[T_index])))) {
                    for (i = 0; i < size; i++) {
                        memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                        c[T_index][i] += z;
                        v[i] -= z;
                    }
                    tracker += 1;
                }
            }
        }
    }
    // printf("sending now\n");

    // communication
    nodeNet.SendAndGetDataFromPeer(v, recv_buf, size, ring_size, mul_map);
    if (mul_map[1][0] > 0) {
        // printf("FIRST IF\n");
        // std::cout << "inserting data recived from " << mul_map[1][0] << " into index " << 0 << " and adding to share index " << 5 << std::endl;
        for (size_t i = 0; i < size; i++)
            c[3][i] = c[3][i] + recv_buf[0][i]; // received from id + 2 (mul_map[1][0])
    }
    if (mul_map[1][1] > 0) {
        // printf("SECOND IF\n");
        // std::cout << "inserting data recived from " << mul_map[1][1] << " into index " << 1 << " and adding to share index " << 3 << std::endl;
        for (size_t i = 0; i < size; i++)
            c[5][i] = c[5][i] + recv_buf[1][i]; // received from id + 1 (mul_map[1][1])
    }
    // if myid \notin T_hat
    if (!ss->pid_in_T(pid, T_hat)) {
        // std::cout << pid << " not in " << T_hat << std::endl;
        for (i = 0; i < size; i++) {
            // need to check here that i actually received something from another party
            // worth separating these loops, first checking if mul_map > 0 so it's checked exactly twice
            c[0][i] = c[0][i] + v[i]; // can be done before S/R
        }
    }
    for (i = 0; i < threshold; i++) {
        delete[] recv_buf[i];
    }

    for (i = 0; i < numShares; i++) {
        delete[] buffer[i];
    }

    // free
    delete[] v;
    delete[] buffer;
    delete[] recv_buf;
}

template <typename T>
void Rss_Mult_Sparse_7pc(T **c, T **a, T **b, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    vector<int> T_hat = {1, 2, 3};

    uint bytes = (ring_size + 7) >> 3;
    uint i, p_prime, T_index;
    uint numShares = ss->getNumShares();
    uint numParties = ss->getPeers();
    uint threshold = ss->getThreshold();
    int pid = nodeNet.getID();
    vector<vector<int>> mul_map = ss->generate_MultSparse_map(numParties, pid);
    // printf("mul_map:\n");
    // for (auto m : mul_map) {
    //     std::cout << m << std::endl;
    // }

    T *v = new T[size];
    std::vector<uint8_t> prg_ctrs(20);
    switch (pid) {
    case 1:
        prg_ctrs = {3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 0};
        break;
    case 2:
        prg_ctrs = {2, 2, 2, 3, 2, 2, 3, 2, 3, 3, 1, 1, 2, 1, 2, 2, 0, 2, 2, 1};
        break;
    case 3:
        prg_ctrs = {1, 1, 2, 2, 1, 2, 2, 2, 2, 3, 0, 2, 2, 2, 2, 3, 1, 2, 3, 2};
        break;
    case 4:
        prg_ctrs = {0, 2, 2, 2, 2, 2, 2, 3, 3, 3, 1, 2, 2, 3, 3, 3, 2, 3, 3, 3};
        break;
    case 5:
        prg_ctrs = {1, 2, 2, 1, 3, 3, 2, 3, 2, 2, 2, 3, 2, 3, 2, 2, 3, 3, 3, 3};
        break;
    case 6:
        prg_ctrs = {2, 3, 2, 2, 3, 2, 2, 2, 2, 1, 3, 3, 3, 3, 3, 2, 3, 3, 2, 2};
        break;
    case 7:
        prg_ctrs = {3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 3, 3, 3, 2, 2, 2, 2, 2, 2, 1};
        break;
    }

    T **recv_buf = new T *[threshold];
    for (i = 0; i < threshold; i++) {
        recv_buf[i] = new T[size];
        memset(recv_buf[i], 0, sizeof(T) * size);
    }

    uint8_t **buffer = new uint8_t *[numShares];
    for (i = 0; i < numShares; i++) {
        buffer[i] = new uint8_t[prg_ctrs[i] * bytes * size];
        if (prg_ctrs[i] > 0) {
            ss->prg_getrandom(i, bytes, prg_ctrs[i] * size, buffer[i]);
        }

        // sanitizing destination (just in case)
        // printf("case )\n");
        memset(c[i], 0, sizeof(T) * size);
    }
    T z = T(0);
    uint tracker;
    // only p \nin T_star computes v

    if (!ss->pid_in_T(pid, T_hat)) {
        for (i = 0; i < size; i++) {
            v[i] =
                a[0][i] * (b[0][i] + b[1][i] + b[2][i] + b[3][i] + b[4][i] + b[5][i] + b[6][i] + b[7][i] + b[8][i] + b[9][i] + b[10][i] + b[11][i] + b[12][i] + b[13][i] + b[14][i] + b[15][i] + b[16][i] + b[17][i] + b[18][i] + b[19][i]) +
                a[1][i] * (b[0][i] + b[1][i] + b[2][i] + b[3][i] + b[4][i] + b[5][i] + b[6][i] + b[7][i] + b[8][i] + b[9][i] + b[10][i] + b[11][i] + b[12][i] + b[13][i] + b[14][i] + b[15][i] + b[16][i] + b[17][i] + b[18][i] + b[19][i]) +
                a[2][i] * (b[0][i] + b[1][i] + b[2][i] + b[3][i] + b[4][i] + b[5][i] + b[6][i] + b[7][i] + b[8][i] + b[9][i] + b[10][i] + b[11][i] + b[12][i] + b[13][i] + b[14][i] + b[15][i] + b[16][i] + b[17][i] + b[18][i] + b[19][i]) +
                a[3][i] * (b[0][i] + b[1][i] + b[2][i] + b[3][i] + b[4][i] + b[5][i] + b[6][i] + b[7][i] + b[8][i] + b[9][i] + b[10][i] + b[11][i] + b[12][i] + b[13][i] + b[14][i] + b[15][i] + b[16][i] + b[17][i] + b[18][i] + b[19][i]) +
                a[4][i] * (b[0][i] + b[1][i] + b[2][i] + b[3][i] + b[10][i] + b[11][i] + b[12][i] + b[13][i] + b[15][i]) +
                a[5][i] * (b[0][i] + b[1][i] + b[2][i] + b[3][i] + b[4][i] + b[5][i] + b[6][i] + b[7][i] + b[8][i] + b[9][i] + b[10][i] + b[11][i] + b[12][i] + b[13][i] + b[14][i] + b[15][i] + b[16][i] + b[17][i] + b[18][i] + b[19][i]) +
                a[6][i] * (b[2][i] + b[5][i] + b[7][i] + b[9][i] + b[11][i] + b[13][i]) +
                a[7][i] * (b[0][i] + b[4][i] + b[5][i] + b[6][i] + b[10][i] + b[11][i] + b[12][i]) +
                a[8][i] * (b[0][i] + b[4][i] + b[5][i] + b[10][i] + b[11][i] + b[16][i]) +
                a[9][i] * (b[1][i] + b[4][i] + b[7][i] + b[8][i] + b[10][i] + b[13][i]) +
                a[10][i] * (b[0][i] + b[1][i] + b[2][i] + b[3][i] + b[4][i] + b[5][i] + b[9][i]) +
                a[11][i] * (b[0][i] + b[1][i] + b[4][i] + b[6][i] + b[7][i] + b[8][i]) +
                a[12][i] * (b[0][i] + b[1][i] + b[2][i] + b[4][i] + b[5][i] + b[7][i]) +
                a[13][i] * (b[0][i] + b[4][i] + b[5][i] + b[6][i]) +
                a[14][i] * (b[2][i] + b[4][i] + b[5][i]) +
                a[15][i] * (b[1][i] + b[4][i]) +
                a[16][i] * (b[0][i] + b[1][i] + b[2][i] + b[3][i] + b[15][i]) +
                a[17][i] * (b[0][i] + b[1][i] + b[2][i]) +
                a[18][i] * (b[1][i] + b[8][i]) +
                a[19][i] * (b[0][i] + b[5][i] + b[6][i]);
        }
    }

    // printf("finished calculating v\n");
    for (p_prime = 1; p_prime < numParties + 1; p_prime++) {
        if (!ss->pid_in_T(p_prime, T_hat)) {

            // printf("\n");
            for (T_index = 0; T_index < numShares; T_index++) {
                tracker = 0;
                if ((p_prime != (pid)) and (!(ss->pid_in_T(p_prime, ss->T_map_mpc[T_index]))) and (!(ss->chi_pid_is_T(p_prime, ss->T_map_mpc[T_index])))) {
                    for (i = 0; i < size; i++) {
                        memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                        c[T_index][i] += z;
                    }
                    tracker += 1;
                } else if (
                    (p_prime == pid) and
                    (!(ss->chi_pid_is_T(pid, ss->T_map_mpc[T_index])))) {
                    for (i = 0; i < size; i++) {

                        memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                        c[T_index][i] += z;
                        v[i] -= z;
                    }
                    tracker += 1;
                }
            }
        }
    }
    // communication
    // nodeNet.SendAndGetDataFromPeer_Mult(v, recv_buf, size, ring_size);
    nodeNet.SendAndGetDataFromPeer(v, recv_buf, size, ring_size, mul_map);

    int idx = 0;
    if (mul_map[1][0] > 0) {
        idx = 10;
        // printf("FIRST IF\n");
        // std::cout << "inserting data recived from " << mul_map[1][0] << " into index " << 0 << " and adding to share index " << idx << std::endl;
        for (size_t i = 0; i < size; i++)
            c[idx][i] += recv_buf[0][i]; // received from id + 1 (mul_map[1][2])
    }

    if (mul_map[1][1] > 0) {
        idx = 16;
        // printf("SECOND IF\n");
        // std::cout << "inserting data recived from " << mul_map[1][1] << " into index " << 1 << " and adding to share index " << idx << std::endl;
        for (size_t i = 0; i < size; i++)
            c[idx][i] += recv_buf[1][i]; // received from id + 2 (mul_map[1][1])
    }

    if (mul_map[1][2] > 0) {
        idx = 19;
        // printf("THIRD IF\n");
        // std::cout << "inserting data recived from " << mul_map[1][2] << " into index " << 2 << " and adding to share index " << idx << std::endl;
        for (size_t i = 0; i < size; i++)
            c[idx][i] += recv_buf[2][i]; // received from id + 3 (mul_map[1][0])
    }
    // if myid \notin T_hat
    if (!ss->pid_in_T(pid, T_hat)) {
        for (i = 0; i < size; i++) {
            // need to check here that i actually received something from another party
            // worth separating these loops, first checking if mul_map > 0 so it's checked exactly twice
            c[0][i] += v[i]; // can be done before S/R
        }
    }

    for (i = 0; i < threshold; i++) {
        delete[] recv_buf[i];
    }

    for (i = 0; i < numShares; i++) {
        delete[] buffer[i];
    }

    // free
    delete[] v;
    delete[] buffer;
    delete[] recv_buf;
}

// non-threaded versions (since RSS is single threaded )
template <typename T>
void Rss_Mult_Sparse(T **C, T **A, T **B, int size, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // from here, we defer to the 3-, 5-, or 7-party implementations
    try {
        int peers = ss->getPeers();
        switch (peers) {
        case 3:
            Rss_Mult_Sparse_3pc(C, A, B, size, ss->ring_size, net, ss);
            break;
        case 5:
            Rss_Mult_Sparse_5pc(C, A, B, size, ss->ring_size, net, ss);
            break;
        case 7:
            Rss_Mult_Sparse_7pc(C, A, B, size, ss->ring_size, net, ss);
            break;
        default:
            throw std::runtime_error("invalid number of parties");
        }
    } catch (const std::runtime_error &ex) {
        std::string error(ex.what());
        throw std::runtime_error("[Mult] " + error);
    }
}

#endif // _MULTSPARSE_HPP_