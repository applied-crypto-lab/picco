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
// Uses interface format [size][numShares] where array[i][s] is share s of element i
template <typename T>
void Rss_Mult_Sparse_3pc(T **c, T **a, T **b_hat, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint bytes = (ring_size + 7) >> 3;
    uint i;
    T *v = new T[size];
    memset(v, 0, sizeof(T) * size);

    int id = ss->getID();
    int n = ss->getPeers();
    vector<vector<int>> mul_map = ss->generate_MultSparse_map(n, id);
    uint8_t *buffer = new uint8_t[bytes * size];

    // Temporary buffer for share 0 values (needed because c[i][0] is not contiguous)
    T *c_share0 = new T[size];
    memset(c_share0, 0, sizeof(T) * size);

    ss->prg_getrandom(1, bytes, size, buffer); // all parties do this, even if not needed (for consistency in future prg invocations)

    if (id != 1) {
        for (i = 0; i < size; i++) {
            memcpy(&c_share0[i], buffer + i * bytes, bytes);
            // Interface format: a[i][0], a[i][1] are shares 0,1 of element i
            v[i] = a[i][0] * b_hat[i][0] + a[i][0] * b_hat[i][1] + a[i][1] * b_hat[i][0];
            v[i] -= c_share0[i];
        }
    } else {
        for (i = 0; i < size; i++) {
            memcpy(&c_share0[i], buffer + i * bytes, bytes);
        }
    }

    // Buffer for receiving share 1 values
    T *c1_recv = new T[size];
    memset(c1_recv, 0, sizeof(T) * size);
    nodeNet.SendAndGetDataFromPeer(v, c1_recv, size, ring_size, mul_map);

    ss->prg_getrandom(0, bytes, size, buffer); // all parties do this, even if not needed (for consistency in future prg invocations)

    switch (id) {
    case 1:
        for (i = 0; i < size; i++) {
            memcpy(&c[i][0], buffer + i * bytes, bytes); // uses a different prg from earlier
            c[i][1] = c1_recv[i];
        }
        break;
    case 2:
        for (i = 0; i < size; i++) {
            c[i][1] = c1_recv[i] + c_share0[i]; // u' + u'' (in this order)
            c[i][0] = v[i];
        }
        break;
    case 3:
        for (i = 0; i < size; i++) {
            c[i][1] = c_share0[i]; // c_share0[i] stores [u]^1 at this point
            memcpy(&c[i][0], buffer + i * bytes, bytes);
            c[i][0] = c[i][0] + v[i]; // u' + G_3
        }
        break;
    default:
        break;
    }

    // free
    delete[] v;
    delete[] c_share0;
    delete[] c1_recv;
    delete[] buffer;
}

// (5,2): \hat{T}  = {1,2}
// Interface format: [size][numShares] where array[i][s] is share s of element i
template <typename T>
void Rss_Mult_Sparse_5pc(T **c, T **a, T **b_hat, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    vector<int> T_hat = {1, 2};

    uint bytes = (ring_size + 7) >> 3;
    uint i;
    uint numShares = ss->getNumShares();
    uint numParties = ss->getPeers();
    uint threshold = ss->getThreshold();
    int pid = ss->getID();
    vector<vector<int>> mul_map = ss->generate_MultSparse_map(numParties, pid);
    T *v = new T[size];
    memset(v, 0, sizeof(T) * size);

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
    }

    // Sanitize destination in interface format [size][numShares]
    for (i = 0; i < size; i++) {
        memset(c[i], 0, sizeof(T) * numShares);
    }

    T z = T(0);
    uint tracker;
    // only p \nin T_star computes v
    if (!ss->pid_in_T(pid, T_hat)) {
        for (i = 0; i < size; i++) {
            // Fixed: use a[i][s] instead of a[s][i]
            v[i] = a[i][0] * (b_hat[i][0] + b_hat[i][1] + b_hat[i][2] + b_hat[i][3] + b_hat[i][4] + b_hat[i][5]) +
                   a[i][1] * (b_hat[i][0] + b_hat[i][1] + b_hat[i][2] + b_hat[i][3] + b_hat[i][4] + b_hat[i][5]) +
                   a[i][2] * (b_hat[i][1] + b_hat[i][3]) +
                   a[i][3] * (b_hat[i][0] + b_hat[i][2]) +
                   a[i][4] * (b_hat[i][0] + b_hat[i][1]) +
                   a[i][5] * (b_hat[i][0] + b_hat[i][4]);
        }
    }

    for (uint p_prime = 1; p_prime < numParties + 1; p_prime++) {
        if (!ss->pid_in_T(p_prime, T_hat)) {
            for (uint T_index = 0; T_index < numShares; T_index++) {
                tracker = 0;
                if (((int)p_prime != (pid)) and
                    (!(ss->pid_in_T(p_prime, ss->T_map_mpc[T_index]))) and
                    (!(ss->chi_pid_is_T(p_prime, ss->T_map_mpc[T_index])))) {
                    for (i = 0; i < size; i++) {
                        memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                        c[i][T_index] += z;  // Fixed: [element][share]
                    }
                    tracker += 1;
                } else if (((int)p_prime == pid) and (!(ss->chi_pid_is_T(pid, ss->T_map_mpc[T_index])))) {
                    for (i = 0; i < size; i++) {
                        memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                        c[i][T_index] += z;  // Fixed: [element][share]
                        v[i] -= z;
                    }
                    tracker += 1;
                }
            }
        }
    }

    // communication
    nodeNet.SendAndGetDataFromPeer(v, recv_buf, size, ring_size, mul_map);
    if (mul_map[1][0] > 0) {
        for (size_t i = 0; i < size; i++)
            c[i][3] = c[i][3] + recv_buf[0][i];  // Fixed: [element][share]
    }
    if (mul_map[1][1] > 0) {
        for (size_t i = 0; i < size; i++)
            c[i][5] = c[i][5] + recv_buf[1][i];  // Fixed: [element][share]
    }
    // if myid \notin T_hat
    if (!ss->pid_in_T(pid, T_hat)) {
        for (i = 0; i < size; i++) {
            c[i][0] = c[i][0] + v[i];  // Fixed: [element][share]
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

// Interface format: [size][numShares] where array[i][s] is share s of element i
template <typename T>
void Rss_Mult_Sparse_7pc(T **c, T **a, T **b, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    vector<int> T_hat = {1, 2, 3};

    uint bytes = (ring_size + 7) >> 3;
    uint i;
    uint numShares = ss->getNumShares();
    uint numParties = ss->getPeers();
    uint threshold = ss->getThreshold();
    int pid = ss->getID();
    vector<vector<int>> mul_map = ss->generate_MultSparse_map(numParties, pid);

    T *v = new T[size];
    memset(v, 0, sizeof(T) * size);
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
    }

    // Sanitize destination in interface format [size][numShares]
    for (i = 0; i < size; i++) {
        memset(c[i], 0, sizeof(T) * numShares);
    }

    T z = T(0);
    uint tracker;
    // only p \nin T_star computes v

    if (!ss->pid_in_T(pid, T_hat)) {
        for (i = 0; i < size; i++) {
            // Fixed: use a[i][s] instead of a[s][i]
            v[i] =
                a[i][0] * (b[i][0] + b[i][1] + b[i][2] + b[i][3] + b[i][4] + b[i][5] + b[i][6] + b[i][7] + b[i][8] + b[i][9] + b[i][10] + b[i][11] + b[i][12] + b[i][13] + b[i][14] + b[i][15] + b[i][16] + b[i][17] + b[i][18] + b[i][19]) +
                a[i][1] * (b[i][0] + b[i][1] + b[i][2] + b[i][3] + b[i][4] + b[i][5] + b[i][6] + b[i][7] + b[i][8] + b[i][9] + b[i][10] + b[i][11] + b[i][12] + b[i][13] + b[i][14] + b[i][15] + b[i][16] + b[i][17] + b[i][18] + b[i][19]) +
                a[i][2] * (b[i][0] + b[i][1] + b[i][2] + b[i][3] + b[i][4] + b[i][5] + b[i][6] + b[i][7] + b[i][8] + b[i][9] + b[i][10] + b[i][11] + b[i][12] + b[i][13] + b[i][14] + b[i][15] + b[i][16] + b[i][17] + b[i][18] + b[i][19]) +
                a[i][3] * (b[i][0] + b[i][1] + b[i][2] + b[i][3] + b[i][4] + b[i][5] + b[i][6] + b[i][7] + b[i][8] + b[i][9] + b[i][10] + b[i][11] + b[i][12] + b[i][13] + b[i][14] + b[i][15] + b[i][16] + b[i][17] + b[i][18] + b[i][19]) +
                a[i][4] * (b[i][0] + b[i][1] + b[i][2] + b[i][3] + b[i][10] + b[i][11] + b[i][12] + b[i][13] + b[i][15]) +
                a[i][5] * (b[i][0] + b[i][1] + b[i][2] + b[i][3] + b[i][4] + b[i][5] + b[i][6] + b[i][7] + b[i][8] + b[i][9] + b[i][10] + b[i][11] + b[i][12] + b[i][13] + b[i][14] + b[i][15] + b[i][16] + b[i][17] + b[i][18] + b[i][19]) +
                a[i][6] * (b[i][2] + b[i][5] + b[i][7] + b[i][9] + b[i][11] + b[i][13]) +
                a[i][7] * (b[i][0] + b[i][4] + b[i][5] + b[i][6] + b[i][10] + b[i][11] + b[i][12]) +
                a[i][8] * (b[i][0] + b[i][4] + b[i][5] + b[i][10] + b[i][11] + b[i][16]) +
                a[i][9] * (b[i][1] + b[i][4] + b[i][7] + b[i][8] + b[i][10] + b[i][13]) +
                a[i][10] * (b[i][0] + b[i][1] + b[i][2] + b[i][3] + b[i][4] + b[i][5] + b[i][9]) +
                a[i][11] * (b[i][0] + b[i][1] + b[i][4] + b[i][6] + b[i][7] + b[i][8]) +
                a[i][12] * (b[i][0] + b[i][1] + b[i][2] + b[i][4] + b[i][5] + b[i][7]) +
                a[i][13] * (b[i][0] + b[i][4] + b[i][5] + b[i][6]) +
                a[i][14] * (b[i][2] + b[i][4] + b[i][5]) +
                a[i][15] * (b[i][1] + b[i][4]) +
                a[i][16] * (b[i][0] + b[i][1] + b[i][2] + b[i][3] + b[i][15]) +
                a[i][17] * (b[i][0] + b[i][1] + b[i][2]) +
                a[i][18] * (b[i][1] + b[i][8]) +
                a[i][19] * (b[i][0] + b[i][5] + b[i][6]);
        }
    }

    for (uint p_prime = 1; p_prime < numParties + 1; p_prime++) {
        if (!ss->pid_in_T(p_prime, T_hat)) {
            for (uint T_index = 0; T_index < numShares; T_index++) {
                tracker = 0;
                if (((int)p_prime != (pid)) and (!(ss->pid_in_T(p_prime, ss->T_map_mpc[T_index]))) and (!(ss->chi_pid_is_T(p_prime, ss->T_map_mpc[T_index])))) {
                    for (i = 0; i < size; i++) {
                        memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                        c[i][T_index] += z;  // Fixed: [element][share]
                    }
                    tracker += 1;
                } else if (
                    ((int)p_prime == pid) and
                    (!(ss->chi_pid_is_T(pid, ss->T_map_mpc[T_index])))) {
                    for (i = 0; i < size; i++) {
                        memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                        c[i][T_index] += z;  // Fixed: [element][share]
                        v[i] -= z;
                    }
                    tracker += 1;
                }
            }
        }
    }
    // communication
    nodeNet.SendAndGetDataFromPeer(v, recv_buf, size, ring_size, mul_map);

    int idx = 0;
    if (mul_map[1][0] > 0) {
        idx = 10;
        for (size_t i = 0; i < size; i++)
            c[i][idx] += recv_buf[0][i];  // Fixed: [element][share]
    }

    if (mul_map[1][1] > 0) {
        idx = 16;
        for (size_t i = 0; i < size; i++)
            c[i][idx] += recv_buf[1][i];  // Fixed: [element][share]
    }

    if (mul_map[1][2] > 0) {
        idx = 19;
        for (size_t i = 0; i < size; i++)
            c[i][idx] += recv_buf[2][i];  // Fixed: [element][share]
    }
    // if myid \notin T_hat
    if (!ss->pid_in_T(pid, T_hat)) {
        for (i = 0; i < size; i++) {
            c[i][0] += v[i];  // Fixed: [element][share]
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
void Mult_Sparse(T **C, T **A, T **B, int size, NodeNetwork net, replicatedSecretShare<T> *ss) {
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