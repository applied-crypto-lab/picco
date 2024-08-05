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

#ifndef RSS_MULT_H_
#define RSS_MULT_H_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include <sys/time.h>
// ANB, 1/30/2024
// this is written in such a way that it checks both orderings of T_map.
// we are now sorting the T_map in the SecertShare constructor
// so we no longer really need to do this, provided we sort calculate what T_map[i] is compared against

inline bool chi_p_prime_in_T(int &p_prime, std::vector<int> &T_map, uint &n) {
    return (((((p_prime + 1 - 1) % n + 1) == T_map[0]) and (((p_prime + 2 - 1) % n + 1) == T_map[1])) or
            ((((p_prime + 1 - 1) % n + 1) == T_map[1]) and (((p_prime + 2 - 1) % n + 1) == T_map[0])));
}

inline bool p_prime_in_T(int &p_prime, std::vector<int> &T_map) {
    return (p_prime == T_map[0] or p_prime == T_map[1]);
}

inline bool chi_p_prime_in_T_7(int &p_prime, std::vector<int> &T_map, uint &n) {

    int chi_0 = (p_prime + 1 - 1) % n + 1;
    int chi_1 = (p_prime + 2 - 1) % n + 1;
    int chi_2 = (p_prime + 3 - 1) % n + 1;

    return ((chi_0 == T_map[0] or chi_0 == T_map[1] or chi_0 == T_map[2]) and (chi_1 == T_map[0] or chi_1 == T_map[1] or chi_1 == T_map[2]) and
            (chi_2 == T_map[0] or chi_2 == T_map[1] or chi_2 == T_map[2]));
}

inline bool p_prime_in_T_7(int& p_prime, std::vector<int> &T_map) {
    return (p_prime == T_map[0] or p_prime == T_map[1] or p_prime == T_map[2]);
}

template <typename T>
void Rss_Mult_Bitwise_3pc(T **c, T **a, T **b, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    // uint bytes = (nodeNet.RING[ring_size] + 7) >> 3;
    uint bytes = (ring_size + 7) >> 3;
    int i;
    T *v = new T[size];

    uint8_t *buffer = new uint8_t[bytes * size];
    ss->prg_getrandom(1, bytes, size, buffer);

    for (i = 0; i < size; i++) {
        memcpy(c[0] + i, buffer + i * bytes, bytes);
        v[i] = ((a[0][i] & b[0][i]) ^ (a[0][i] & b[1][i]) ^ (a[1][i] & b[0][i])) ^ c[0][i];
    }
    nodeNet.SendAndGetDataFromPeer(v, c[1], size, ring_size, ss->general_map);
    ss->prg_getrandom(0, bytes, size, buffer);

    for (i = 0; i < size; i++) {
        c[1][i] = c[1][i] ^ c[0][i];
        memcpy(c[0] + i, buffer + i * bytes, bytes);
        c[0][i] = c[0][i] ^ v[i];
    }
    // free
    delete[] v;
    delete[] buffer;
}

//  For party 1, a[0,1]=a_2,3; b[0,1]=b_2,3;  c[0,1] = c_2,3;
//  For party 2, a[0,1]=a_3,1; b[0,1]=b_3,1;  c[0,1] = c_3,1;
//  For party 3, a[0,1]=a_1,2; b[0,1]=b_1,2;  c[0,1] = c_1,2;
template <typename T>
void Rss_Mult_3pc(T **c, T **a, T **b, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // uint bytes = (nodeNet.RING[ring_size] + 7) >> 3;
    uint bytes = (ring_size + 7) >> 3;
    int i;

    T *v = new T[size];

    uint8_t *buffer = new uint8_t[bytes * size];
    ss->prg_getrandom(1, bytes, size, buffer);

    for (i = 0; i < size; i++) {
        // original
        memcpy(c[0] + i, buffer + i * bytes, bytes);
        v[i] = a[0][i] * b[0][i] + a[0][i] * b[1][i] + a[1][i] * b[0][i] - c[0][i];

        // alternate version that allows for buffers to be reused for inputs/outputs
        // v[i] = a[0][i] * b[0][i] + a[0][i] * b[1][i] + a[1][i] * b[0][i];
        // memcpy(c[0] + i, buffer + i * bytes, bytes);
        // v[i] -= c[0][i];
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

//  For party 1, a[0,1]=a_2,3; b[0,1]=b_2,3;  c[0,1] = c_2,3;
//  For party 2, a[0,1]=a_3,1; b[0,1]=b_3,1;  c[0,1] = c_3,1;
//  For party 3, a[0,1]=a_1,2; b[0,1]=b_1,2;  c[0,1] = c_1,2;
template <typename T>
void Rss_Mult_fixed_b_3pc(T **c, T **a, T **b, uint b_index, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // uint bytes = (nodeNet.RING[ring_size] + 7) >> 3;
    uint bytes = (ring_size + 7) >> 3;
    int i;

    T *v = new T[size];

    T **c_placeholder = new T *[2];
    for (i = 0; i < 2; i++) {
        c_placeholder[i] = new T[size];
        memset(c_placeholder[i], 0, sizeof(T) * size);
    }

    uint8_t *buffer = new uint8_t[bytes * size];
    ss->prg_getrandom(1, bytes, size, buffer);
    // memcpy(c[0], buffer, size*bytes);

    for (i = 0; i < size; i++) {
        memcpy(c_placeholder[0] + i, buffer + i * bytes, bytes);
        v[i] = a[0][i] * b[0][b_index] + a[0][i] * b[1][b_index] + a[1][i] * b[0][b_index] - c_placeholder[0][i];
    }
    // communication
    nodeNet.SendAndGetDataFromPeer(v, c_placeholder[1], size, ring_size, ss->general_map);
    ss->prg_getrandom(0, bytes, size, buffer);

    for (i = 0; i < size; i++) {
        c[1][i] = c_placeholder[1][i] + c_placeholder[0][i];
        // ss->prg_getrandom(0, bytes, c[0]+i);
        memcpy(c_placeholder[0] + i, buffer + i * bytes, bytes);
        c[0][i] = c_placeholder[0][i] + v[i];
    }
    for (i = 0; i < 2; i++) {
        delete[] c_placeholder[i];
    }

    // free
    delete[] v;
    delete[] c_placeholder;
    delete[] buffer;
}

template <typename T>
void Rss_Mult_Byte_3pc(uint8_t **c, uint8_t **a, uint8_t **b, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // size == how many bytes we're multiplying
    // uint bytes = (size+8-1)>>3;  //number of bytes need to be send/recv
    // uint bytes = size;  //number of bytes need to be send/recv
    // printf("size: %llu\n", size);

    int i = 0;
    uint8_t *v = new uint8_t[size];
    ss->prg_getrandom(1, 1, size, c[0]); //<-- COMMENT OUT FOR TESTING
    // for(i = 0; i < 2; i++) memset(c[i], 0, sizeof(uint8_t)*size); //<-- COMMENT IN FOR TESTING

    for (i = 0; i < size; i++) { // do operations byte by byte
        uint8_t temp = ((a[0][i] & (b[0][i] ^ b[1][i])) ^ (a[1][i] & b[0][i]));
        v[i] = temp ^ c[0][i];
        // v[i] = ((a[0][i] & b[0][i]) ^ (a[0][i] & b[1][i]) ^ (a[1][i] & b[0][i])) ^ c[0][i]; // original, do not modify
    }

    // communication
    nodeNet.SendAndGetDataFromPeer_bit(v, c[1], size, ss->general_map);
    for (i = 0; i < size; i++) {
        c[1][i] = c[1][i] ^ c[0][i];
    }
    ss->prg_getrandom(0, 1, size, c[0]); //<-- COMMENT OUT FOR TESTING
    for (i = 0; i < size; i++) {
        c[0][i] = c[0][i] ^ v[i];
    }

    // free
    delete[] v;
}

//  For party 1, a[0,1]=a_2,3; b[0,1]=b_2,3;  c[0,1] = c_2,3;
//  For party 2, a[0,1]=a_3,1; b[0,1]=b_3,1;  c[0,1] = c_3,1;
//  For party 3, a[0,1]=a_1,2; b[0,1]=b_1,2;  c[0,1] = c_1,2;
// template <typename T>
// void Rss_MultPub_3pc(T *c, T **a, T **b, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
//     int i; // used for loops

//     // uint bytes = (nodeNet.RING[ring_size] +7) >> 3;
//     uint bytes = (ring_size + 7) >> 3;

//     T **sendbuf = new T *[3];
//     T **recvbuf = new T *[3];
//     for (i = 0; i < 3; i++) {
//         sendbuf[i] = new T[size];
//         memset(sendbuf[i], 0, sizeof(T) * size);
//         recvbuf[i] = new T[size];
//         memset(recvbuf[i], 0, sizeof(T) * size);
//     }

//         static int pid = ss->getID();

//     T *v = new T[size];
//     T *v_a = new T[size];

//     T opa = 0;
//     T opb = 0;
//     switch (pid) {
//     case 1:
//         opa = 1;
//         opb = 1;
//         break;
//     case 2:
//         opa = -1;
//         opb = 1;
//         break;
//     case 3:
//         opa = -1;
//         opb = -1;
//         break;
//     }

//     uint8_t *buffer = new uint8_t[bytes * size];
//     ss->prg_getrandom(0, bytes, size, buffer);
//     for (i = 0; i < size; i++) {
//         memcpy(v_a + i, buffer + i * bytes, bytes);
//     }
//     ss->prg_getrandom(1, bytes, size, buffer);
//     for (i = 0; i < size; i++) {
//         memcpy(c + i, buffer + i * bytes, bytes);
//     }

//     for (i = 0; i < size; i++) {
//         v[i] = a[0][i] * b[0][i] + a[0][i] * b[1][i] + a[1][i] * b[0][i];
//         c[i] = v[i] + opb * c[i] + opa * v_a[i];
//     }

//     // communication
//     // move data into buf
//     for (i = 1; i <= 3; i++) {
//         if (i == pid)
//             continue;
//         memcpy(sendbuf[i - 1], c, sizeof(T) * size);
//     }

//     nodeNet.multicastToPeers(sendbuf, recvbuf, size, ring_size);

//     memcpy(v_a, recvbuf[(((nodeNet.getID()) + 2 - 1) % 3 + 1) - 1], sizeof(T) * size);
//     memcpy(v, recvbuf[(((nodeNet.getID()) + 1 - 1) % 3 + 1) - 1], sizeof(T) * size);

//     for (i = 0; i < size; i++) {
//         // mask here
//         c[i] = c[i] + v_a[i] + v[i];
//         c[i] = c[i] & ss->SHIFT[ring_size];
//     }

//     // free
//     delete[] v;
//     delete[] v_a;
//     delete[] buffer;
//     for (i = 0; i < 3; i++) {
//         delete[] sendbuf[i];
//         delete[] recvbuf[i];
//     }
//     delete[] sendbuf;
//     delete[] recvbuf;
// }

template <typename T>
void Rss_MultPub_3pc(T *c, T **a, T **b, uint size, uint ring_size, uint bitlength, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    int i; // used for loops

    // uint bytes = (nodeNet.RING[ring_size] +7) >> 3;
    uint bytes = (ring_size + 7) >> 3;

    T **sendbuf = new T *[3];
    T **recvbuf = new T *[3];
    for (i = 0; i < 3; i++) {
        sendbuf[i] = new T[size];
        memset(sendbuf[i], 0, sizeof(T) * size);
        recvbuf[i] = new T[size];
        memset(recvbuf[i], 0, sizeof(T) * size);
    }

    static int pid = ss->getID();

    T *v = new T[size];
    T *v_a = new T[size];

    T opa = 0;
    T opb = 0;
    switch (pid) {
    case 1:
        opa = 1;
        opb = 1;
        break;
    case 2:
        opa = -1;
        opb = 1;
        break;
    case 3:
        opa = -1;
        opb = -1;
        break;
    }

    uint8_t *buffer = new uint8_t[bytes * size];
    ss->prg_getrandom(0, bytes, size, buffer);
    for (i = 0; i < size; i++) {
        memcpy(v_a + i, buffer + i * bytes, bytes);
    }
    ss->prg_getrandom(1, bytes, size, buffer);
    for (i = 0; i < size; i++) {
        memcpy(c + i, buffer + i * bytes, bytes);
    }

    for (i = 0; i < size; i++) {
        v[i] = a[0][i] * b[0][i] + a[0][i] * b[1][i] + a[1][i] * b[0][i];
        c[i] = v[i] + opb * c[i] + opa * v_a[i];
    }

    // communication
    // move data into buf
    for (i = 1; i <= 3; i++) {
        if (i == pid)
            continue;
        memcpy(sendbuf[i - 1], c, sizeof(T) * size);
    }

    nodeNet.multicastToPeers(sendbuf, recvbuf, size, bitlength);

    memcpy(v_a, recvbuf[(((nodeNet.getID()) + 2 - 1) % 3 + 1) - 1], sizeof(T) * size);
    memcpy(v, recvbuf[(((nodeNet.getID()) + 1 - 1) % 3 + 1) - 1], sizeof(T) * size);

    for (i = 0; i < size; i++) {
        // mask here
        c[i] = c[i] + v_a[i] + v[i];
        c[i] = c[i] & ss->SHIFT[bitlength];
    }

    // free
    delete[] v;
    delete[] v_a;
    delete[] buffer;
    for (i = 0; i < 3; i++) {
        delete[] sendbuf[i];
        delete[] recvbuf[i];
    }
    delete[] sendbuf;
    delete[] recvbuf;
}

template <typename T>
void Rss_Mult_Bitwise_5pc(T **c, T **a, T **b, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint bytes = (ring_size + 7) >> 3;
    uint i;
    int p_prime, T_index;
    uint numShares = ss->getNumShares();
    uint numParties = ss->getPeers();
    uint threshold = ss->getThreshold();
    static int pid = ss->getID();

    T *v = new T[size];
    memset(v, 0, sizeof(T) * size);
    uint8_t prg_ctrs[6] = {2, 3, 3, 2, 3, 2};
    // uint8_t prg_ctrs[6] = {1, 1, 1, 1, 1, 1}; // FOR TESTING

    T **recv_buf = new T *[threshold];
    for (i = 0; i < threshold; i++) {
        recv_buf[i] = new T[size];
        memset(recv_buf[i], 0, sizeof(T) * size);
    }
    // printf("prg start\n");

    uint8_t **buffer = new uint8_t *[numShares];
    for (i = 0; i < numShares; i++) {
        buffer[i] = new uint8_t[prg_ctrs[i] * bytes * size];
        ss->prg_getrandom(i, bytes, prg_ctrs[i] * size, buffer[i]);
        memset(c[i], 0, sizeof(T) * size);
    }
    T z = 0;
    uint trackers[6] = {0, 0, 0, 0, 0, 0};
    // printf("-- calculating v\n");
    // uint tracker;
    for (i = 0; i < size; i++) {
        v[i] = (a[0][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[3][i] ^ b[4][i] ^ b[5][i])) ^ (a[1][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[3][i] ^ b[4][i] ^ b[5][i])) ^ (a[2][i] & (b[1][i] ^ b[3][i])) ^ (a[3][i] & (b[0][i] ^ b[2][i])) ^ (a[4][i] & (b[0][i] ^ b[1][i])) ^ (a[5][i] & (b[0][i] ^ b[4][i]));
        // printf("\n------------v[i]: %llu\t", v[i]  & ss->SHIFT[ring_size] );

        // printf("finished calculating v\n");
        for (p_prime = 1; p_prime < numParties + 1; p_prime++) {
            // printf("\n");
            for (T_index = 0; T_index < numShares; T_index++) {
                // tracker = 0;
                z = T(0);

                if ((p_prime != (pid)) and (!(p_prime_in_T(p_prime, ss->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T(p_prime, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (trackers[T_index]) * bytes, bytes);
                    c[T_index][i] = c[T_index][i] ^ z;
                    trackers[T_index] += 1;
                } else if ((p_prime == pid) and (!(chi_p_prime_in_T(pid, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (trackers[T_index]) * bytes, bytes);
                    c[T_index][i] = c[T_index][i] ^ z;
                    v[i] = v[i] ^ z;
                    trackers[T_index] += 1;
                }
            }
        }
    }
    // printf("-- sending v\n");

    // communication
    nodeNet.SendAndGetDataFromPeer(v, recv_buf, size, ring_size, ss->general_map);

    // nodeNet.SendAndGetDataFromPeer_Mult(v, recv_buf, size, ring_size);
    // ss->prg_getrandom(0, bytes, size, buffer);
    for (i = 0; i < size; i++) {
        c[3][i] = c[3][i] ^ recv_buf[1][i];
        c[5][i] = c[5][i] ^ recv_buf[0][i];
        c[0][i] = c[0][i] ^ v[i];
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
void Rss_Mult_5pc(T **c, T **a, T **b, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint bytes = (ring_size + 7) >> 3;
    uint i;
    int p_prime, T_index;
    uint numShares = ss->getNumShares();
    uint numParties = ss->getPeers();
    uint threshold = ss->getThreshold();
    static int pid = ss->getID();

    T *v = new T[size];
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
    }
    for (int s = 0; s < numShares; s++) {
        // sanitizing after the product is computed, so we can reuse the buffer
        memset(c[s], 0, sizeof(T) * size);
    }
    for (i = 0; i < size; i++) {
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
void Rss_Mult_fixed_b_5pc(T **c, T **a, T **b, uint b_index, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint bytes = (ring_size + 7) >> 3;
    uint i, p_prime, T_index;
    uint numShares = ss->getNumShares();
    uint numParties = ss->getPeers();
    uint threshold = ss->getThreshold();
    static int pid = ss->getID();

    T *v = new T[size];
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

        // memset(c[i], 0, sizeof(T) * size);
    }
    T z = 0;
    uint tracker;
    for (i = 0; i < size; i++) {
        v[i] = a[0][i] * (b[0][b_index] + b[1][b_index] + b[2][b_index] + b[3][b_index] + b[4][b_index] + b[5][b_index]) +
               a[1][i] * (b[0][b_index] + b[1][b_index] + b[2][b_index] + b[3][b_index] + b[4][b_index] + b[5][b_index]) +
               a[2][i] * (b[1][b_index] + b[3][b_index]) +
               a[3][i] * (b[0][b_index] + b[2][b_index]) +
               a[4][i] * (b[0][b_index] + b[1][b_index]) +
               a[5][i] * (b[0][b_index] + b[4][b_index]);
    }
    for (int s = 0; s < numShares; s++) {
        // sanitizing after the product is computed, so we can reuse the buffer
        memset(c[s], 0, sizeof(T) * size);
    }
    for (i = 0; i < size; i++) {
        for (p_prime = 1; p_prime < numParties + 1; p_prime++) {
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

    // communication
    nodeNet.SendAndGetDataFromPeer(v, recv_buf, size, ring_size, ss->general_map);
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
void Rss_Mult_Byte_5pc(uint8_t **c, uint8_t **a, uint8_t **b, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint i = 0;
    int p_prime, T_index;
    uint numShares = ss->getNumShares();
    uint numParties = ss->getPeers();
    uint threshold = ss->getThreshold();
    static int pid = ss->getID();

    // printf("size: %u\n", size);

    uint8_t *v = new uint8_t[size];
    memset(v, 0, sizeof(uint8_t) * size);
    uint8_t prg_ctrs[6] = {2, 3, 3, 2, 3, 2};
    // uint8_t prg_ctrs[6] = {1, 1, 1, 1, 1, 1}; // FOR TESTING

    uint8_t **recv_buf = new uint8_t *[threshold];
    for (i = 0; i < threshold; i++) {
        recv_buf[i] = new uint8_t[size];
        memset(recv_buf[i], 0, sizeof(uint8_t) * size);
    }
    // printf("prg start\n");

    uint8_t **buffer = new uint8_t *[numShares];
    for (i = 0; i < numShares; i++) {
        buffer[i] = new uint8_t[prg_ctrs[i] * size];
        ss->prg_getrandom(i, 1, prg_ctrs[i] * size, buffer[i]);
        // memset(c[i], 0, sizeof(uint8_t) * size);
    }
    // uint tracker;
    uint trackers[6] = {0, 0, 0, 0, 0, 0};

    for (i = 0; i < size; i++) {

        v[i] = (a[0][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[3][i] ^ b[4][i] ^ b[5][i])) ^
               (a[1][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[3][i] ^ b[4][i] ^ b[5][i])) ^
               (a[2][i] & (b[1][i] ^ b[3][i])) ^
               (a[3][i] & (b[0][i] ^ b[2][i])) ^
               (a[4][i] & (b[0][i] ^ b[1][i])) ^
               (a[5][i] & (b[0][i] ^ b[4][i]));
    }
    for (int s = 0; s < numShares; s++) {
        // sanitizing after the product is computed, so we can reuse the buffer
        memset(c[s], 0, sizeof(uint8_t) * size);
    }
    for (i = 0; i < size; i++) {
        for (p_prime = 1; p_prime < numParties + 1; p_prime++) {
            for (T_index = 0; T_index < numShares; T_index++) {
                if ((p_prime != (pid)) and (!(p_prime_in_T(p_prime, ss->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T(p_prime, ss->T_map_mpc[T_index], numParties)))) {
                    c[T_index][i] = c[T_index][i] ^ buffer[T_index][trackers[T_index]];
                    trackers[T_index] += 1;
                } else if ((p_prime == pid) and (!(chi_p_prime_in_T(pid, ss->T_map_mpc[T_index], numParties)))) {
                    c[T_index][i] = c[T_index][i] ^ buffer[T_index][trackers[T_index]];
                    v[i] = v[i] ^ buffer[T_index][trackers[T_index]];
                    trackers[T_index] += 1;
                }
            }
        }
    }
    nodeNet.SendAndGetDataFromPeer_bit(v, recv_buf, size, ss->general_map);

    // nodeNet.SendAndGetDataFromPeer_bit_Mult(v, recv_buf, size);
    for (i = 0; i < size; i++) {
        c[3][i] = c[3][i] ^ recv_buf[1][i];
        c[5][i] = c[5][i] ^ recv_buf[0][i];

        c[0][i] = c[0][i] ^ v[i];
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

// template <typename T>
// void Rss_MultPub_5pc(T *c, T **a, T **b, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

//     uint bytes = (ring_size + 7) >> 3;
//     uint i, j;
//     uint numShares = ss->getNumShares();
//     uint numParties = ss->getPeers();
//     // uint threshold = ss->getThreshold();
//         static int pid = ss->getID();

//     T **send_buf = new T *[numParties];
//     T **recv_buf = new T *[numParties];
//     for (i = 0; i < numParties; i++) {
//         send_buf[i] = new T[size];
//         memset(send_buf[i], 0, sizeof(T) * size);
//         recv_buf[i] = new T[size];
//         memset(recv_buf[i], 0, sizeof(T) * size);
//     }
//     T *v = new T[size];

//     // move to outside function
//     T *ops;
//     uint *prg_ctrs;
//     int num_ops = 0;
//     switch (pid) {
//     case 1:
//         num_ops = 6;
//         prg_ctrs = new uint[numShares]{1, 1, 1, 1, 1, 1};
//         // ops = new T[num_ops]{(1), (1), (1), (1), (1), (1)};
//         ops = new T[num_ops];
//         ops[0] = T(1);
//         ops[1] = T(1);
//         ops[2] = T(1);
//         ops[3] = T(1);
//         ops[4] = T(1);
//         ops[5] = T(1);
//         break;
//     case 2:
//         num_ops = 6;
//         prg_ctrs = new uint[numShares]{1, 1, 1, 1, 1, 1};
//         // ops = new T[num_ops]{(1), (1), (1), (1), (1), (1)};
//         ops = new T[num_ops];
//         ops[0] = T(1);
//         ops[1] = T(1);
//         ops[2] = T(1);
//         ops[3] = T(1);
//         ops[4] = T(1);
//         ops[5] = T(1);
//         break;
//     case 3:
//         num_ops = 7;
//         prg_ctrs = new uint[numShares]{2, 1, 1, 1, 1, 1};
//         // ops = new T[num_ops]{(-1), (-1), (1), (1), (1), (1), (1)};
//         ops = new T[num_ops];
//         ops[0] = T(-1);
//         ops[1] = T(-1);
//         ops[2] = T(1);
//         ops[3] = T(1);
//         ops[4] = T(1);
//         ops[5] = T(1);
//         ops[6] = T(1);
//         break;
//     case 4:
//         num_ops = 9;
//         prg_ctrs = new uint[numShares]{2, 2, 2, 1, 1, 1};
//         // ops = new T[num_ops]{(-1), (-1), (-1), (-1), (-1), (-1), (1), (1), (1)};
//         ops = new T[num_ops];
//         ops[0] = T(-1);
//         ops[1] = T(-1);
//         ops[2] = T(-1);
//         ops[3] = T(-1);
//         ops[4] = T(-1);
//         ops[5] = T(-1);
//         ops[6] = T(1);
//         ops[7] = T(1);
//         ops[8] = T(1);

//         break;
//     case 5:
//         num_ops = 12;
//         prg_ctrs = new uint[numShares]{2, 2, 2, 2, 2, 2};
//         ops = new T[num_ops];
//         // ops = new T[num_ops]{(-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1), (-1)};
//         ops[0] = T(-1);
//         ops[1] = T(-1);
//         ops[2] = T(-1);
//         ops[3] = T(-1);
//         ops[4] = T(-1);
//         ops[5] = T(-1);
//         ops[6] = T(-1);
//         ops[7] = T(-1);
//         ops[8] = T(-1);
//         ops[9] = T(-1);
//         ops[10] = T(-1);
//         ops[11] = T(-1);
//         break;
//     }

//     uint8_t **buffer = new uint8_t *[numShares];
//     T **v_a = new T *[numShares];
//     for (i = 0; i < numShares; i++) {
//         buffer[i] = new uint8_t[bytes * size];
//         v_a[i] = new T[size];
//         ss->prg_getrandom(i, bytes, size, buffer[i]);
//         memcpy(v_a[i], buffer[i], bytes * size);
//     }

//     // T z = 0;
//     uint sign_index = 0;

//     for (size_t i = 0; i < size; i++) {
//         c[i] = a[0][i] * (b[0][i] + b[1][i] + b[2][i] + b[3][i] + b[4][i] + b[5][i]) +
//                a[1][i] * (b[0][i] + b[1][i] + b[2][i] + b[3][i] + b[4][i] + b[5][i]) +
//                a[2][i] * (b[1][i] + b[3][i]) +
//                a[3][i] * (b[0][i] + b[2][i]) +
//                a[4][i] * (b[0][i] + b[1][i]) +
//                a[5][i] * (b[0][i] + b[4][i]);

//         sign_index = 0;
//         for (size_t j = 0; j < numShares; j++) {
//             for (size_t v_index = 0; v_index < prg_ctrs[j]; v_index++) {
//                 c[i] = c[i] + ops[sign_index] * v_a[j][i];
//                 sign_index++;
//             }
//             // printf("sign_index: %llu\n", sign_index);
//         }
//     }

//     // move data into buf
//     for (i = 1; i <= numParties; i++) {
//         if (i == pid) {
//             continue;
//         }
//         memcpy(send_buf[i - 1], c, sizeof(T) * size);
//     }

//     nodeNet.multicastToPeers(send_buf, recv_buf, size, ring_size);

//     for (i = 0; i < size; i++) {
//         for (j = 0; j < numParties; j++) {
//             c[i] = c[i] + recv_buf[j][i]; // we can just add up all received messages, including the one from itself (which is zero from earlier)
//         }
//         c[i] = c[i] & ss->SHIFT[ring_size];
//     }

//     for (i = 0; i < numParties; i++) {
//         delete[] send_buf[i];
//         delete[] recv_buf[i];
//     }
//     delete[] send_buf;
//     delete[] recv_buf;

//     for (i = 0; i < numShares; i++) {
//         delete[] buffer[i];
//         delete[] v_a[i];
//     }
//     delete[] v_a;

//     // free
//     delete[] v;
//     delete[] buffer;
//     delete[] ops;
//     delete[] prg_ctrs;
//     // delete[] recv_buf
// }

template <typename T>
void Rss_Mult_7pc(T **c, T **a, T **b, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    struct timeval start;
    struct timeval end;
    unsigned long timer = 0;

    uint bytes = (ring_size + 7) >> 3;
    uint i;
    int p_prime, T_index;
    static uint numShares = ss->getNumShares();
    static uint numParties = ss->getPeers();
    static uint threshold = ss->getThreshold();
    static int pid = ss->getID();

    // std::cout << "numShares  " << numShares << std::endl;
    // std::cout << "numParties  " << numParties << std::endl;
    // std::cout << "threshold  " << threshold << std::endl;
    // std::cout << "pid  " << pid << std::endl;

    T *v = new T[size];
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
    }
    T z = T(0);
    uint tracker;
    gettimeofday(&start, NULL);
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
    // gettimeofday(&end, NULL); // stop timer here
    // timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    // printf("[7pc local 1] [%.3lf ms]\n", (double)(timer * 0.001));

    // // for (int s = 0; s < numShares; s++) {
    // //     // sanitizing after the product is computed, so we can reuse the buffer
    // //     memset(c[s], 0, sizeof(priv_int_t) * size);
    // // }
    // gettimeofday(&start, NULL);
    for (i = 0; i < size; i++) {

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
    gettimeofday(&end, NULL); // stop timer here
    timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    printf("[7pc local 2] [%.3lf ms]\n", (double)(timer * 0.001));
    gettimeofday(&start, NULL);
    // communication
    // nodeNet.SendAndGetDataFromPeer_Mult(v, recv_buf, size, ring_size);
    nodeNet.SendAndGetDataFromPeer(v, recv_buf, size, ring_size, ss->general_map);

    gettimeofday(&end, NULL); // stop timer here
    timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    printf("[7pc send recv] [%.3lf ms]\n", (double)(timer * 0.001));

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

template <typename T>
void Rss_Mult_fixed_b_7pc(T **c, T **a, T **b, uint b_index, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint bytes = (ring_size + 7) >> 3;
    uint i, p_prime, T_index;
    uint numShares = ss->getNumShares();
    uint numParties = ss->getPeers();
    uint threshold = ss->getThreshold();
    static int pid = ss->getID();

    T *v = new T[size];
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
        // memset(c[i], 0, sizeof(T) * size);
    }
    T z = T(0);
    uint tracker;
    for (i = 0; i < size; i++) {
        v[i] =
            a[0][i] * (b[0][b_index] + b[1][b_index] + b[2][b_index] + b[3][b_index] + b[4][b_index] + b[5][b_index] + b[6][b_index] + b[7][b_index] + b[8][b_index] + b[9][b_index] + b[10][b_index] + b[11][b_index] + b[12][b_index] + b[13][b_index] + b[14][b_index] + b[15][b_index] + b[16][b_index] + b[17][b_index] + b[18][b_index] + b[19][b_index]) +
            a[1][i] * (b[0][b_index] + b[1][b_index] + b[2][b_index] + b[3][b_index] + b[4][b_index] + b[5][b_index] + b[6][b_index] + b[7][b_index] + b[8][b_index] + b[9][b_index] + b[10][b_index] + b[11][b_index] + b[12][b_index] + b[13][b_index] + b[14][b_index] + b[15][b_index] + b[16][b_index] + b[17][b_index] + b[18][b_index] + b[19][b_index]) +
            a[2][i] * (b[0][b_index] + b[1][b_index] + b[2][b_index] + b[3][b_index] + b[4][b_index] + b[5][b_index] + b[6][b_index] + b[7][b_index] + b[8][b_index] + b[9][b_index] + b[10][b_index] + b[11][b_index] + b[12][b_index] + b[13][b_index] + b[14][b_index] + b[15][b_index] + b[16][b_index] + b[17][b_index] + b[18][b_index] + b[19][b_index]) +
            a[3][i] * (b[0][b_index] + b[1][b_index] + b[2][b_index] + b[3][b_index] + b[4][b_index] + b[5][b_index] + b[6][b_index] + b[7][b_index] + b[8][b_index] + b[9][b_index] + b[10][b_index] + b[11][b_index] + b[12][b_index] + b[13][b_index] + b[14][b_index] + b[15][b_index] + b[16][b_index] + b[17][b_index] + b[18][b_index] + b[19][b_index]) +
            a[4][i] * (b[0][b_index] + b[1][b_index] + b[2][b_index] + b[3][b_index] + b[10][b_index] + b[11][b_index] + b[12][b_index] + b[13][b_index] + b[15][b_index]) +
            a[5][i] * (b[0][b_index] + b[1][b_index] + b[2][b_index] + b[3][b_index] + b[4][b_index] + b[5][b_index] + b[6][b_index] + b[7][b_index] + b[8][b_index] + b[9][b_index] + b[10][b_index] + b[11][b_index] + b[12][b_index] + b[13][b_index] + b[14][b_index] + b[15][b_index] + b[16][b_index] + b[17][b_index] + b[18][b_index] + b[19][b_index]) +
            a[6][i] * (b[2][b_index] + b[5][b_index] + b[7][b_index] + b[9][b_index] + b[11][b_index] + b[13][b_index]) +
            a[7][i] * (b[0][b_index] + b[4][b_index] + b[5][b_index] + b[6][b_index] + b[10][b_index] + b[11][b_index] + b[12][b_index]) +
            a[8][i] * (b[0][b_index] + b[4][b_index] + b[5][b_index] + b[10][b_index] + b[11][b_index] + b[16][b_index]) +
            a[9][i] * (b[1][b_index] + b[4][b_index] + b[7][b_index] + b[8][b_index] + b[10][b_index] + b[13][b_index]) +
            a[10][i] * (b[0][b_index] + b[1][b_index] + b[2][b_index] + b[3][b_index] + b[4][b_index] + b[5][b_index] + b[9][b_index]) +
            a[11][i] * (b[0][b_index] + b[1][b_index] + b[4][b_index] + b[6][b_index] + b[7][b_index] + b[8][b_index]) +
            a[12][i] * (b[0][b_index] + b[1][b_index] + b[2][b_index] + b[4][b_index] + b[5][b_index] + b[7][b_index]) +
            a[13][i] * (b[0][b_index] + b[4][b_index] + b[5][b_index] + b[6][b_index]) +
            a[14][i] * (b[2][b_index] + b[4][b_index] + b[5][b_index]) +
            a[15][i] * (b[1][b_index] + b[4][b_index]) +
            a[16][i] * (b[0][b_index] + b[1][b_index] + b[2][b_index] + b[3][b_index] + b[15][b_index]) +
            a[17][i] * (b[0][b_index] + b[1][b_index] + b[2][b_index]) +
            a[18][i] * (b[1][b_index] + b[8][b_index]) +
            a[19][i] * (b[0][b_index] + b[5][b_index] + b[6][b_index]);
    }
    for (int s = 0; s < numShares; s++) {
        // sanitizing after the product is computed, so we can reuse the buffer
        memset(c[s], 0, sizeof(T) * size);
    }
    for (i = 0; i < size; i++) {
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

template <typename T>
void Rss_Mult_Bitwise_7pc(T **c, T **a, T **b, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint bytes = (ring_size + 7) >> 3;
    uint i;
    int p_prime, T_index;
    uint numShares = ss->getNumShares();
    uint numParties = ss->getPeers();
    uint threshold = ss->getThreshold();
    static int pid = ss->getID();

    T *v = new T[size];
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
        // memset(c[i], 0, sizeof(T) * size);
    }
    T z = T(0);
    uint tracker;
    for (i = 0; i < size; i++) {
        v[i] =
            (a[0][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[3][i] ^ b[4][i] ^ b[5][i] ^ b[6][i] ^ b[7][i] ^ b[8][i] ^ b[9][i] ^ b[10][i] ^ b[11][i] ^ b[12][i] ^ b[13][i] ^ b[14][i] ^ b[15][i] ^ b[16][i] ^ b[17][i] ^ b[18][i] ^ b[19][i])) ^
            (a[1][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[3][i] ^ b[4][i] ^ b[5][i] ^ b[6][i] ^ b[7][i] ^ b[8][i] ^ b[9][i] ^ b[10][i] ^ b[11][i] ^ b[12][i] ^ b[13][i] ^ b[14][i] ^ b[15][i] ^ b[16][i] ^ b[17][i] ^ b[18][i] ^ b[19][i])) ^
            (a[2][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[3][i] ^ b[4][i] ^ b[5][i] ^ b[6][i] ^ b[7][i] ^ b[8][i] ^ b[9][i] ^ b[10][i] ^ b[11][i] ^ b[12][i] ^ b[13][i] ^ b[14][i] ^ b[15][i] ^ b[16][i] ^ b[17][i] ^ b[18][i] ^ b[19][i])) ^
            (a[3][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[3][i] ^ b[4][i] ^ b[5][i] ^ b[6][i] ^ b[7][i] ^ b[8][i] ^ b[9][i] ^ b[10][i] ^ b[11][i] ^ b[12][i] ^ b[13][i] ^ b[14][i] ^ b[15][i] ^ b[16][i] ^ b[17][i] ^ b[18][i] ^ b[19][i])) ^
            (a[4][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[3][i] ^ b[10][i] ^ b[11][i] ^ b[12][i] ^ b[13][i] ^ b[15][i])) ^
            (a[5][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[3][i] ^ b[4][i] ^ b[5][i] ^ b[6][i] ^ b[7][i] ^ b[8][i] ^ b[9][i] ^ b[10][i] ^ b[11][i] ^ b[12][i] ^ b[13][i] ^ b[14][i] ^ b[15][i] ^ b[16][i] ^ b[17][i] ^ b[18][i] ^ b[19][i])) ^
            (a[6][i] & (b[2][i] ^ b[5][i] ^ b[7][i] ^ b[9][i] ^ b[11][i] ^ b[13][i])) ^
            (a[7][i] & (b[0][i] ^ b[4][i] ^ b[5][i] ^ b[6][i] ^ b[10][i] ^ b[11][i] ^ b[12][i])) ^
            (a[8][i] & (b[0][i] ^ b[4][i] ^ b[5][i] ^ b[10][i] ^ b[11][i] ^ b[16][i])) ^
            (a[9][i] & (b[1][i] ^ b[4][i] ^ b[7][i] ^ b[8][i] ^ b[10][i] ^ b[13][i])) ^
            (a[10][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[3][i] ^ b[4][i] ^ b[5][i] ^ b[9][i])) ^
            (a[11][i] & (b[0][i] ^ b[1][i] ^ b[4][i] ^ b[6][i] ^ b[7][i] ^ b[8][i])) ^
            (a[12][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[4][i] ^ b[5][i] ^ b[7][i])) ^
            (a[13][i] & (b[0][i] ^ b[4][i] ^ b[5][i] ^ b[6][i])) ^
            (a[14][i] & (b[2][i] ^ b[4][i] ^ b[5][i])) ^
            (a[15][i] & (b[1][i] ^ b[4][i])) ^
            (a[16][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[3][i] ^ b[15][i])) ^
            (a[17][i] & (b[0][i] ^ b[1][i] ^ b[2][i])) ^
            (a[18][i] & (b[1][i] ^ b[8][i])) ^
            (a[19][i] & (b[0][i] ^ b[5][i] ^ b[6][i]));
    }
    for (int s = 0; s < numShares; s++) {
        // sanitizing after the product is computed, so we can reuse the buffer
        memset(c[s], 0, sizeof(T) * size);
    }
    for (i = 0; i < size; i++) {
        // printf("finished calculating v\n");
        for (p_prime = 1; p_prime < numParties + 1; p_prime++) {
            // printf("\n");
            for (T_index = 0; T_index < numShares; T_index++) {
                tracker = 0;
                if ((p_prime != (pid)) and (!(p_prime_in_T_7(p_prime, ss->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T_7(p_prime, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[T_index][i] ^= z;
                    tracker += 1;
                } else if (
                    (p_prime == pid) and (!(chi_p_prime_in_T_7(pid, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[T_index][i] ^= z;
                    v[i] ^= z;

                    tracker += 1;
                }
            }
        }
    }
    // communication
    nodeNet.SendAndGetDataFromPeer(v, recv_buf, size, ring_size, ss->general_map);

    // nodeNet.SendAndGetDataFromPeer_Mult(v, recv_buf, size, ring_size);
    // ss->prg_getrandom(0, bytes, size, buffer);
    for (i = 0; i < size; i++) {
        c[19][i] = c[19][i] ^ recv_buf[0][i];
        c[16][i] = c[16][i] ^ recv_buf[1][i];
        c[10][i] = c[10][i] ^ recv_buf[2][i];

        c[0][i] = c[0][i] ^ v[i];
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
void Rss_Mult_Byte_7pc(uint8_t **c, uint8_t **a, uint8_t **b, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint i;
    int p_prime, T_index;
    uint numShares = ss->getNumShares();
    uint numParties = ss->getPeers();
    uint threshold = ss->getThreshold();
    static int pid = ss->getID();

    uint8_t *v = new uint8_t[size];
    uint8_t prg_ctrs[20] = {3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 4, 4, 4, 4, 4, 3, 4, 4, 3};

    uint8_t **recv_buf = new uint8_t *[threshold];
    for (i = 0; i < threshold; i++) {
        recv_buf[i] = new uint8_t[size];
        memset(recv_buf[i], 0, sizeof(uint8_t) * size);
    }

    uint8_t **buffer = new uint8_t *[numShares];
    for (i = 0; i < numShares; i++) {
        buffer[i] = new uint8_t[prg_ctrs[i] * size];
        ss->prg_getrandom(i, 1, prg_ctrs[i] * size, buffer[i]);
        // sanitizing destination (just in case)
        // printf("case )\n");
        // memset(c[i], 0, sizeof(uint8_t) * size);
    }
    uint tracker = 0;
    uint trackers[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    for (i = 0; i < size; i++) {
        v[i] =
            (a[0][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[3][i] ^ b[4][i] ^ b[5][i] ^ b[6][i] ^ b[7][i] ^ b[8][i] ^ b[9][i] ^ b[10][i] ^ b[11][i] ^ b[12][i] ^ b[13][i] ^ b[14][i] ^ b[15][i] ^ b[16][i] ^ b[17][i] ^ b[18][i] ^ b[19][i])) ^
            (a[1][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[3][i] ^ b[4][i] ^ b[5][i] ^ b[6][i] ^ b[7][i] ^ b[8][i] ^ b[9][i] ^ b[10][i] ^ b[11][i] ^ b[12][i] ^ b[13][i] ^ b[14][i] ^ b[15][i] ^ b[16][i] ^ b[17][i] ^ b[18][i] ^ b[19][i])) ^
            (a[2][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[3][i] ^ b[4][i] ^ b[5][i] ^ b[6][i] ^ b[7][i] ^ b[8][i] ^ b[9][i] ^ b[10][i] ^ b[11][i] ^ b[12][i] ^ b[13][i] ^ b[14][i] ^ b[15][i] ^ b[16][i] ^ b[17][i] ^ b[18][i] ^ b[19][i])) ^
            (a[3][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[3][i] ^ b[4][i] ^ b[5][i] ^ b[6][i] ^ b[7][i] ^ b[8][i] ^ b[9][i] ^ b[10][i] ^ b[11][i] ^ b[12][i] ^ b[13][i] ^ b[14][i] ^ b[15][i] ^ b[16][i] ^ b[17][i] ^ b[18][i] ^ b[19][i])) ^
            (a[4][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[3][i] ^ b[10][i] ^ b[11][i] ^ b[12][i] ^ b[13][i] ^ b[15][i])) ^
            (a[5][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[3][i] ^ b[4][i] ^ b[5][i] ^ b[6][i] ^ b[7][i] ^ b[8][i] ^ b[9][i] ^ b[10][i] ^ b[11][i] ^ b[12][i] ^ b[13][i] ^ b[14][i] ^ b[15][i] ^ b[16][i] ^ b[17][i] ^ b[18][i] ^ b[19][i])) ^
            (a[6][i] & (b[2][i] ^ b[5][i] ^ b[7][i] ^ b[9][i] ^ b[11][i] ^ b[13][i])) ^
            (a[7][i] & (b[0][i] ^ b[4][i] ^ b[5][i] ^ b[6][i] ^ b[10][i] ^ b[11][i] ^ b[12][i])) ^
            (a[8][i] & (b[0][i] ^ b[4][i] ^ b[5][i] ^ b[10][i] ^ b[11][i] ^ b[16][i])) ^
            (a[9][i] & (b[1][i] ^ b[4][i] ^ b[7][i] ^ b[8][i] ^ b[10][i] ^ b[13][i])) ^
            (a[10][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[3][i] ^ b[4][i] ^ b[5][i] ^ b[9][i])) ^
            (a[11][i] & (b[0][i] ^ b[1][i] ^ b[4][i] ^ b[6][i] ^ b[7][i] ^ b[8][i])) ^
            (a[12][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[4][i] ^ b[5][i] ^ b[7][i])) ^
            (a[13][i] & (b[0][i] ^ b[4][i] ^ b[5][i] ^ b[6][i])) ^
            (a[14][i] & (b[2][i] ^ b[4][i] ^ b[5][i])) ^
            (a[15][i] & (b[1][i] ^ b[4][i])) ^
            (a[16][i] & (b[0][i] ^ b[1][i] ^ b[2][i] ^ b[3][i] ^ b[15][i])) ^
            (a[17][i] & (b[0][i] ^ b[1][i] ^ b[2][i])) ^
            (a[18][i] & (b[1][i] ^ b[8][i])) ^
            (a[19][i] & (b[0][i] ^ b[5][i] ^ b[6][i]));
    }
    for (int s = 0; s < numShares; s++) {
        // sanitizing after the product is computed, so we can reuse the buffer
        memset(c[s], 0, sizeof(uint8_t) * size);
    }
    for (i = 0; i < size; i++) {
        // printf("finished calculating v\n");
        for (p_prime = 1; p_prime < numParties + 1; p_prime++) {
            // printf("\n");
            for (T_index = 0; T_index < numShares; T_index++) {
                tracker = 0;
                if ((p_prime != (pid)) and (!(p_prime_in_T_7(p_prime, ss->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T_7(p_prime, ss->T_map_mpc[T_index], numParties)))) {
                    c[T_index][i] ^= buffer[T_index][trackers[T_index]];
                    tracker += 1;
                } else if (
                    (p_prime == pid) and (!(chi_p_prime_in_T_7(pid, ss->T_map_mpc[T_index], numParties)))) {
                    c[T_index][i] = c[T_index][i] ^ buffer[T_index][trackers[T_index]];

                    v[i] ^= buffer[T_index][trackers[T_index]];

                    tracker += 1;
                }
            }
        }
    }
    // communication
    nodeNet.SendAndGetDataFromPeer_bit(v, recv_buf, size, ss->general_map);

    // nodeNet.SendAndGetDataFromPeer_bit_Mult(v, recv_buf, size);
    for (i = 0; i < size; i++) {
        c[19][i] = c[19][i] ^ recv_buf[0][i];
        c[16][i] = c[16][i] ^ recv_buf[1][i];
        c[10][i] = c[10][i] ^ recv_buf[2][i];

        c[0][i] = c[0][i] ^ v[i];
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

// only need one version that has threadID parameter, since thats the only version that gets called inside SMC_utils
template <typename T>
void Mult(T **C, T **A, T **B, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // from here, we defer to the 3-, 5-, or 7-party implementations
    try {
        int peers = ss->getPeers();
        switch (peers) {
        case 3:
            Rss_Mult_3pc(C, A, B, size, ss->ring_size, net, ss);
            break;
        case 5:
            Rss_Mult_5pc(C, A, B, size, ss->ring_size, net, ss);
            break;
        case 7:
            Rss_Mult_7pc(C, A, B, size, ss->ring_size, net, ss);
            break;
        default:
            throw std::runtime_error("invalid number of parties");
        }
    } catch (const std::runtime_error &ex) {
        std::string error(ex.what());
        throw std::runtime_error("[Mult] " + error);
    }
}

// non-threaded versions (since RSS is single threaded )
template <typename T>
void Mult(T **C, T **A, T **B, int size, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // from here, we defer to the 3-, 5-, or 7-party implementations
    try {
        int peers = ss->getPeers();
        switch (peers) {
        case 3:
            Rss_Mult_3pc(C, A, B, size, ss->ring_size, net, ss);
            break;
        case 5:
            Rss_Mult_5pc(C, A, B, size, ss->ring_size, net, ss);
            break;
        case 7:
            Rss_Mult_7pc(C, A, B, size, ss->ring_size, net, ss);
            break;
        default:
            throw std::runtime_error("invalid number of parties");
        }
    } catch (const std::runtime_error &ex) {
        std::string error(ex.what());
        throw std::runtime_error("[Mult] " + error);
    }
}

template <typename T>
void Mult_Bitwise(T **C, T **A, T **B, int size, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // from here, we defer to the 3-, 5-, or 7-party implementations
    try {
        int peers = ss->getPeers();
        switch (peers) {
        case 3:
            Rss_Mult_Bitwise_3pc(C, A, B, size, ss->ring_size, net, ss);
            break;
        case 5:
            Rss_Mult_Bitwise_5pc(C, A, B, size, ss->ring_size, net, ss);
            break;
        case 7:
            Rss_Mult_Bitwise_7pc(C, A, B, size, ss->ring_size, net, ss);
            break;
        default:
            throw std::runtime_error("invalid number of parties");
        }
    } catch (const std::runtime_error &ex) {
        std::string error(ex.what());
        throw std::runtime_error("[Mult] " + error);
    }
}

template <typename T>
void Mult_Byte(uint8_t **C, uint8_t **A, uint8_t **B, int size, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // from here, we defer to the 3-, 5-, or 7-party implementations
    try {
        int peers = ss->getPeers();
        switch (peers) {
        case 3:
            Rss_Mult_Byte_3pc(C, A, B, size, net, ss);
            break;
        case 5:
            Rss_Mult_Byte_5pc(C, A, B, size, net, ss);
            break;
        case 7:
            Rss_Mult_Byte_7pc(C, A, B, size, net, ss);
            break;
        default:
            throw std::runtime_error("invalid number of parties");
        }
    } catch (const std::runtime_error &ex) {
        std::string error(ex.what());
        throw std::runtime_error("[Mult] " + error);
    }
}

#endif