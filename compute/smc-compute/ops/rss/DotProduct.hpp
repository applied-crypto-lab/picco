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

#ifndef _DOTPRODUCT_HPP_
#define _DOTPRODUCT_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"

template <typename T>

void doOperation_DotProduct(T **a, T **b, T *result, int array_size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {}

template <typename T>
void doOperation_DotProduct(T ***a, T ***b, T **result, int batch_size, int array_size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {}

// a : [array_size * size] (blocks of array_size)
// b : [array_size * size] (blocks of array_size)
// c : [size]
template <typename T>
void Rss_DotProd_3pc(T **c, T **a, T **b, uint array_size, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // uint bytes = (nodeNet.RING[ring_size] + 7) >> 3;
    uint bytes = (ring_size + 7) >> 3;
    int i;

    T *v = new T[size];
    memset(v, 0, sizeof(T) * (size));

    uint8_t *buffer = new uint8_t[bytes * size];
    ss->prg_getrandom(1, bytes, size, buffer);

    uint idx;
    for (i = 0; i < size; i++) {
        memcpy(c[0] + i, buffer + i * bytes, bytes);
        for (size_t j = 0; j < array_size; j++) {
            idx = i * array_size + j;
            v[i] += a[0][idx] * b[0][idx] + a[0][idx] * b[1][idx] + a[1][idx] * b[0][idx];
        }
        v[i] -= c[0][i];
    }

    nodeNet.SendAndGetDataFromPeer(v, c[1], size, ring_size, ss->general_map);
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

template <typename T>
void Rss_DotProd_5pc(T **c, T **a, T **b, uint array_size, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint bytes = (ring_size + 7) >> 3;
    uint i;
    int p_prime, T_index;
    static uint numShares = ss->getNumShares();
    static uint numParties = ss->getPeers();
    static uint threshold = ss->getThreshold();
    static int pid = ss->getID();

    T *v = new T[size];
    memset(v, 0, sizeof(T) * (size));

    uint8_t prg_ctrs[6] = {2, 3, 3, 2, 3, 2};

    T **recv_buf = new T *[threshold];
    for (i = 0; i < threshold; i++) {
        recv_buf[i] = new T[size];
        memset(recv_buf[i], 0, sizeof(T) * size);
    }

    uint8_t **buffer = new uint8_t *[numShares];
    for (i = 0; i < numShares; i++) {
        buffer[i] = new uint8_t[prg_ctrs[i] * bytes * size];
        ss->prg_getrandom(i, bytes, prg_ctrs[i] * size, buffer[i]);

        // sanitizing destination (just in case)
        memset(c[i], 0, sizeof(T) * size);
    }
    T z = T(0);
    uint tracker;
    for (i = 0; i < size; i++) {
        v[i] = a[0][i] * (b[0][i] + b[1][i] + b[2][i] + b[3][i] + b[4][i] + b[5][i]) +
               a[1][i] * (b[0][i] + b[1][i] + b[2][i] + b[3][i] + b[4][i] + b[5][i]) +
               a[2][i] * (b[1][i] + b[3][i]) +
               a[3][i] * (b[0][i] + b[2][i]) +
               a[4][i] * (b[0][i] + b[1][i]) +
               a[5][i] * (b[0][i] + b[4][i]);
        // printf("finished calculating v\n");
        for (p_prime = 1; p_prime < numParties + 1; p_prime++) {
            // printf("\n");
            for (T_index = 0; T_index < numShares; T_index++) {
                tracker = 0;
                if ((p_prime != (pid)) and (!(p_prime_in_T(p_prime, ss->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T(p_prime, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[T_index][i] += z;
                    tracker += 1;
                } else if (
                    (p_prime == pid) and (!(chi_p_prime_in_T(pid, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[T_index][i] += z;
                    v[i] -= z;

                    tracker += 1;
                }
            }
        }
    }
    // printf("sending now\n");

    // communication
    nodeNet.SendAndGetDataFromPeer(v, recv_buf, size, ring_size, ss->general_map);
    // nodeNet.SendAndGetDataFromPeer_Mult(v, recv_buf, size, ring_size);
    // ss->prg_getrandom(0, bytes, size, buffer);
    for (i = 0; i < size; i++) {
        c[3][i] = c[3][i] + recv_buf[1][i];
        c[5][i] = c[5][i] + recv_buf[0][i];

        c[0][i] = c[0][i] + v[i];
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
void Rss_DotProd_7pc(T **c, T **a, T **b, uint array_size, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint bytes = (ring_size + 7) >> 3;
    uint i;
    int p_prime, T_index;
    static uint numShares = ss->getNumShares();
    static uint numParties = ss->getPeers();
    static uint threshold = ss->getThreshold();
    static int pid = ss->getID();

    T *v = new T[size];
    memset(v, 0, sizeof(T) * (size));

    uint8_t prg_ctrs[20] = {3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 4, 4, 4, 4, 4, 3, 4, 4, 3};

    T **recv_buf = new T *[threshold];
    for (i = 0; i < threshold; i++) {
        recv_buf[i] = new T[size];
        memset(recv_buf[i], 0, sizeof(T) * size);
    }

    uint8_t **buffer = new uint8_t *[numShares];
    for (i = 0; i < numShares; i++) {
        buffer[i] = new uint8_t[prg_ctrs[i] * bytes * size];
        ss->prg_getrandom(i, bytes, prg_ctrs[i] * size, buffer[i]);

        // sanitizing destination (just in case)
        // printf("case )\n");
        memset(c[i], 0, sizeof(T) * size);
    }
    T z = T(0);
    uint tracker;
    uint idx;
    for (i = 0; i < size; i++) {
        for (size_t j = 0; j < array_size; j++) {
            idx = i * array_size + j;
            v[i] +=
                a[0][idx] * (b[0][idx] + b[1][idx] + b[2][idx] + b[3][idx] + b[4][idx] + b[5][idx] + b[6][idx] + b[7][idx] + b[8][idx] + b[9][idx] + b[10][idx] + b[11][idx] + b[12][idx] + b[13][idx] + b[14][idx] + b[15][idx] + b[16][idx] + b[17][idx] + b[18][idx] + b[19][idx]) +
                a[1][idx] * (b[0][idx] + b[1][idx] + b[2][idx] + b[3][idx] + b[4][idx] + b[5][idx] + b[6][idx] + b[7][idx] + b[8][idx] + b[9][idx] + b[10][idx] + b[11][idx] + b[12][idx] + b[13][idx] + b[14][idx] + b[15][idx] + b[16][idx] + b[17][idx] + b[18][idx] + b[19][idx]) +
                a[2][idx] * (b[0][idx] + b[1][idx] + b[2][idx] + b[3][idx] + b[4][idx] + b[5][idx] + b[6][idx] + b[7][idx] + b[8][idx] + b[9][idx] + b[10][idx] + b[11][idx] + b[12][idx] + b[13][idx] + b[14][idx] + b[15][idx] + b[16][idx] + b[17][idx] + b[18][idx] + b[19][idx]) +
                a[3][idx] * (b[0][idx] + b[1][idx] + b[2][idx] + b[3][idx] + b[4][idx] + b[5][idx] + b[6][idx] + b[7][idx] + b[8][idx] + b[9][idx] + b[10][idx] + b[11][idx] + b[12][idx] + b[13][idx] + b[14][idx] + b[15][idx] + b[16][idx] + b[17][idx] + b[18][idx] + b[19][idx]) +
                a[4][idx] * (b[0][idx] + b[1][idx] + b[2][idx] + b[3][idx] + b[10][idx] + b[11][idx] + b[12][idx] + b[13][idx] + b[15][idx]) +
                a[5][idx] * (b[0][idx] + b[1][idx] + b[2][idx] + b[3][idx] + b[4][idx] + b[5][idx] + b[6][idx] + b[7][idx] + b[8][idx] + b[9][idx] + b[10][idx] + b[11][idx] + b[12][idx] + b[13][idx] + b[14][idx] + b[15][idx] + b[16][idx] + b[17][idx] + b[18][idx] + b[19][idx]) +
                a[6][idx] * (b[2][idx] + b[5][idx] + b[7][idx] + b[9][idx] + b[11][idx] + b[13][idx]) +
                a[7][idx] * (b[0][idx] + b[4][idx] + b[5][idx] + b[6][idx] + b[10][idx] + b[11][idx] + b[12][idx]) +
                a[8][idx] * (b[0][idx] + b[4][idx] + b[5][idx] + b[10][idx] + b[11][idx] + b[16][idx]) +
                a[9][idx] * (b[1][idx] + b[4][idx] + b[7][idx] + b[8][idx] + b[10][idx] + b[13][idx]) +
                a[10][idx] * (b[0][idx] + b[1][idx] + b[2][idx] + b[3][idx] + b[4][idx] + b[5][idx] + b[9][idx]) +
                a[11][idx] * (b[0][idx] + b[1][idx] + b[4][idx] + b[6][idx] + b[7][idx] + b[8][idx]) +
                a[12][idx] * (b[0][idx] + b[1][idx] + b[2][idx] + b[4][idx] + b[5][idx] + b[7][idx]) +
                a[13][idx] * (b[0][idx] + b[4][idx] + b[5][idx] + b[6][idx]) +
                a[14][idx] * (b[2][idx] + b[4][idx] + b[5][idx]) +
                a[15][idx] * (b[1][idx] + b[4][idx]) +
                a[16][idx] * (b[0][idx] + b[1][idx] + b[2][idx] + b[3][idx] + b[15][idx]) +
                a[17][idx] * (b[0][idx] + b[1][idx] + b[2][idx]) +
                a[18][idx] * (b[1][idx] + b[8][idx]) +
                a[19][idx] * (b[0][idx] + b[5][idx] + b[6][idx]);
        }
        // printf("finished calculating v\n");
        for (p_prime = 1; p_prime < numParties + 1; p_prime++) {
            // printf("\n");
            for (T_index = 0; T_index < numShares; T_index++) {
                tracker = 0;
                if ((p_prime != (pid)) and (!(p_prime_in_T_7(p_prime, ss->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T_7(p_prime, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[T_index][i] += z;
                    tracker += 1;
                } else if (
                    (p_prime == pid) and (!(chi_p_prime_in_T_7(pid, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[T_index][i] += z;
                    v[i] -= z;

                    tracker += 1;
                }
            }
        }
    }
    // communication
    // nodeNet.SendAndGetDataFromPeer_Mult(v, recv_buf, size, ring_size);
    nodeNet.SendAndGetDataFromPeer(v, recv_buf, size, ring_size, ss->general_map);

    // ss->prg_getrandom(0, bytes, size, buffer);
    for (i = 0; i < size; i++) {
        c[19][i] = c[19][i] + recv_buf[0][i];
        c[16][i] = c[16][i] + recv_buf[1][i];
        c[10][i] = c[10][i] + recv_buf[2][i];

        c[0][i] = c[0][i] + v[i];
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
void DotProduct(T **C, T **A, T **B, uint array_size, uint size, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // from here, we defer to the 3-, 5-, or 7-party implementations
    try {
        int peers = ss->getPeers();
        switch (peers) {
        case 3:
            Rss_DotProd_3pc(C, A, B, array_size, size, ss->ring_size, net, ss);
            break;
        case 5:
            Rss_DotProd_5pc(C, A, B, array_size, size, ss->ring_size, net, ss);
            break;
        case 7:
            Rss_DotProd_7pc(C, A, B, array_size, size, ss->ring_size, net, ss);
            break;
        default:
            throw std::runtime_error("invalid number of parties");
        }
    } catch (const std::runtime_error &ex) {
        std::string error(ex.what());
        throw std::runtime_error("[Mult] " + error);
    }
}

#endif // _DOTPRODUCT_HPP_
