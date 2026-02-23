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

inline bool chi_p_prime_in_T(int p_prime, std::vector<int> &T_map, uint n) {
    return (((((p_prime + 1 - 1) % n + 1) == (uint)T_map[0]) and (((p_prime + 2 - 1) % n + 1) == (uint)T_map[1])) or
            ((((p_prime + 1 - 1) % n + 1) == (uint)T_map[1]) and (((p_prime + 2 - 1) % n + 1) == (uint)T_map[0])));
}

inline bool p_prime_in_T(int p_prime, std::vector<int> &T_map) {
    return (p_prime == T_map[0] or p_prime == T_map[1]);
}

inline bool chi_p_prime_in_T_7(int p_prime, std::vector<int> &T_map, uint n) {

    int chi_0 = (p_prime + 1 - 1) % n + 1;
    int chi_1 = (p_prime + 2 - 1) % n + 1;
    int chi_2 = (p_prime + 3 - 1) % n + 1;

    return ((chi_0 == T_map[0] or chi_0 == T_map[1] or chi_0 == T_map[2]) and (chi_1 == T_map[0] or chi_1 == T_map[1] or chi_1 == T_map[2]) and
            (chi_2 == T_map[0] or chi_2 == T_map[1] or chi_2 == T_map[2]));
}

inline bool p_prime_in_T_7(int p_prime, std::vector<int> &T_map) {
    return (p_prime == T_map[0] or p_prime == T_map[1] or p_prime == T_map[2]);
}

// Arrays use interface format [size][numShares] where:
//   a[i][0], a[i][1] are share 0 and share 1 of element i
template <typename T>
void Rss_Mult_Bitwise_3pc(T **c, T **a, T **b, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint bytes = (ring_size + 7) >> 3;
    uint i;
    T *v = new T[size];
    T *prg0_vals = new T[size];  // PRG(0) values for masking
    T *c_share1 = new T[size];   // buffer for receiving

    uint8_t *buffer = new uint8_t[bytes * size];

    // FIX: The communication pattern is P1←P2, P2←P3, P3←P1
    // PRG relationships: P1.PRG(1)=P2.PRG(0), P2.PRG(1)=P3.PRG(0), P3.PRG(1)=P1.PRG(0)
    // For sender's mask to match receiver's unmask:
    //   - Each party masks with PRG(0), receiver unmasks with PRG(1)
    ss->prg_getrandom(0, bytes, size, buffer);

    for (i = 0; i < size; i++) {
        memcpy(&prg0_vals[i], buffer + i * bytes, bytes);
        // v = local_and XOR PRG(0)
        v[i] = ((a[i][0] & b[i][0]) ^ (a[i][0] & b[i][1]) ^ (a[i][1] & b[i][0])) ^ prg0_vals[i];
    }

    nodeNet.SendAndGetDataFromPeer(v, c_share1, size, ring_size, ss->general_map);

    // Get PRG(1) for unmasking (sender's PRG(0) = our PRG(1))
    ss->prg_getrandom(1, bytes, size, buffer);

    for (i = 0; i < size; i++) {
        // c[1] = recv XOR PRG(1) (sender's PRG(0) = our PRG(1), so this unmasks correctly)
        T prg1_val;
        memcpy(&prg1_val, buffer + i * bytes, bytes);
        c[i][1] = c_share1[i] ^ prg1_val;
        // c[0] = v XOR PRG(0) = (local XOR PRG(0)) XOR PRG(0) = local
        c[i][0] = prg0_vals[i] ^ v[i];
    }

    delete[] prg0_vals;

    // free
    delete[] v;
    delete[] c_share1;
    delete[] buffer;
}

//  For party 1, a[0,1]=a_2,3; b[0,1]=b_2,3;  c[0,1] = c_2,3;
//  For party 2, a[0,1]=a_3,1; b[0,1]=b_3,1;  c[0,1] = c_3,1;
//  For party 3, a[0,1]=a_1,2; b[0,1]=b_1,2;  c[0,1] = c_1,2;
// Arrays use interface format [size][numShares] where:
//   a[i][0], a[i][1] are share 0 and share 1 of element i
template <typename T>
void Rss_Mult_3pc(T **c, T **a, T **b, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint bytes = (ring_size + 7) >> 3;
    uint i;

    T *v = new T[size];
    T *c_share0 = new T[size];  // temporary for share 0 values
    T *c_share1 = new T[size];  // buffer for receiving share 1

    uint8_t *buffer = new uint8_t[bytes * size];
    ss->prg_getrandom(1, bytes, size, buffer);

    for (i = 0; i < size; i++) {
        // Copy random value to c_share0[i] (was c[0][i] in old format)
        memcpy(&c_share0[i], buffer + i * bytes, bytes);
        // Compute v using interface format: a[i][0], a[i][1], b[i][0], b[i][1]
        v[i] = a[i][0] * b[i][0] + a[i][0] * b[i][1] + a[i][1] * b[i][0] - c_share0[i];
    }

    // Send v, receive into c_share1
    nodeNet.SendAndGetDataFromPeer(v, c_share1, size, ring_size, ss->general_map);
    ss->prg_getrandom(0, bytes, size, buffer);

    for (i = 0; i < size; i++) {
        // c[i][1] = received + c_share0
        c[i][1] = c_share1[i] + c_share0[i];
        // Get new random for c[i][0]
        memcpy(&c_share0[i], buffer + i * bytes, bytes);
        c[i][0] = c_share0[i] + v[i];
    }

    // free
    delete[] v;
    delete[] c_share0;
    delete[] c_share1;
    delete[] buffer;
}

//  For party 1, a[0,1]=a_2,3; b[0,1]=b_2,3;  c[0,1] = c_2,3;
//  For party 2, a[0,1]=a_3,1; b[0,1]=b_3,1;  c[0,1] = c_3,1;
//  For party 3, a[0,1]=a_1,2; b[0,1]=b_1,2;  c[0,1] = c_1,2;
// Arrays use interface format [size][numShares] where:
//   a[i][0], a[i][1] are share 0 and share 1 of element i
//   b[b_index][0], b[b_index][1] are the fixed shares being multiplied with all elements
template <typename T>
void Rss_Mult_fixed_b_3pc(T **c, T **a, T **b, uint b_index, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint bytes = (ring_size + 7) >> 3;
    uint i;

    T *v = new T[size];
    T *c_share0 = new T[size];
    T *c_share1 = new T[size];

    uint8_t *buffer = new uint8_t[bytes * size];
    ss->prg_getrandom(1, bytes, size, buffer);

    for (i = 0; i < size; i++) {
        memcpy(&c_share0[i], buffer + i * bytes, bytes);
        // Use interface format: a[i][0], a[i][1] for element i, b[b_index][0], b[b_index][1] for fixed b
        v[i] = a[i][0] * b[b_index][0] + a[i][0] * b[b_index][1] + a[i][1] * b[b_index][0] - c_share0[i];
    }
    // communication
    nodeNet.SendAndGetDataFromPeer(v, c_share1, size, ring_size, ss->general_map);
    ss->prg_getrandom(0, bytes, size, buffer);

    for (i = 0; i < size; i++) {
        c[i][1] = c_share1[i] + c_share0[i];
        memcpy(&c_share0[i], buffer + i * bytes, bytes);
        c[i][0] = c_share0[i] + v[i];
    }

    // free
    delete[] v;
    delete[] c_share0;
    delete[] c_share1;
    delete[] buffer;
}

// Arrays use interface format [size][numShares] where:
//   a[i][0], a[i][1] are share 0 and share 1 of element i
template <typename T>
void Rss_Mult_Byte_3pc(uint8_t **c, uint8_t **a, uint8_t **b, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // size == how many bytes we're multiplying
    // REVERT: Try original pattern to see if it works for byte operations
    uint i = 0;
    uint8_t *v = new uint8_t[size];
    uint8_t *c_share0 = new uint8_t[size];
    uint8_t *c_share1 = new uint8_t[size];

    ss->prg_getrandom(1, 1, size, c_share0);

    for (i = 0; i < size; i++) {
        uint8_t temp = ((a[i][0] & (b[i][0] ^ b[i][1])) ^ (a[i][1] & b[i][0]));
        v[i] = temp ^ c_share0[i];
    }

    nodeNet.SendAndGetDataFromPeer_bit(v, c_share1, size, ss->general_map);
    for (i = 0; i < size; i++) {
        c[i][1] = c_share1[i] ^ c_share0[i];
    }
    ss->prg_getrandom(0, 1, size, c_share0);
    for (i = 0; i < size; i++) {
        c[i][0] = c_share0[i] ^ v[i];
    }

    delete[] v;
    delete[] c_share0;
    delete[] c_share1;
}

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

    int pid = ss->getID();

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

// Interface format: [size][numShares] where array[i][s] = share s of element i
template <typename T>
void Rss_Mult_Bitwise_5pc(T **c, T **a, T **b, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint bytes = (ring_size + 7) >> 3;
    uint i;
    uint numShares = ss->getNumShares();
    uint numParties = ss->getPeers();
    uint threshold = ss->getThreshold();
    uint pid = (uint)ss->getID();

    T *v = new T[size];
    memset(v, 0, sizeof(T) * size);
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

    // Sanitize output in interface format [size][numShares]
    for (i = 0; i < size; i++) {
        memset(c[i], 0, sizeof(T) * numShares);
    }

    T z = 0;
    uint trackers[6] = {0, 0, 0, 0, 0, 0};
    for (i = 0; i < size; i++) {
        // Fixed: use a[i][s] instead of a[s][i]
        v[i] = (a[i][0] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][3] ^ b[i][4] ^ b[i][5])) ^
               (a[i][1] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][3] ^ b[i][4] ^ b[i][5])) ^
               (a[i][2] & (b[i][1] ^ b[i][3])) ^
               (a[i][3] & (b[i][0] ^ b[i][2])) ^
               (a[i][4] & (b[i][0] ^ b[i][1])) ^
               (a[i][5] & (b[i][0] ^ b[i][4]));

        for (uint p_prime = 1; p_prime < numParties + 1; p_prime++) {
            for (uint T_index = 0; T_index < numShares; T_index++) {
                z = T(0);

                if ((p_prime != (pid)) and (!(p_prime_in_T(p_prime, ss->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T(p_prime, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (trackers[T_index]) * bytes, bytes);
                    c[i][T_index] = c[i][T_index] ^ z;  // Fixed: [element][share]
                    trackers[T_index] += 1;
                } else if ((p_prime == pid) and (!(chi_p_prime_in_T(pid, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (trackers[T_index]) * bytes, bytes);
                    c[i][T_index] = c[i][T_index] ^ z;  // Fixed: [element][share]
                    v[i] = v[i] ^ z;
                    trackers[T_index] += 1;
                }
            }
        }
    }

    // communication
    nodeNet.SendAndGetDataFromPeer(v, recv_buf, size, ring_size, ss->general_map);

    for (i = 0; i < size; i++) {
        c[i][3] = c[i][3] ^ recv_buf[1][i];  // Fixed: [element][share]
        c[i][5] = c[i][5] ^ recv_buf[0][i];  // Fixed: [element][share]
        c[i][0] = c[i][0] ^ v[i];            // Fixed: [element][share]
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
    uint numShares = ss->getNumShares();
    uint numParties = ss->getPeers();
    uint threshold = ss->getThreshold();
    uint pid = (uint)ss->getID();

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
        // Fixed indexing: arrays are [element][share] not [share][element]
        v[i] = a[i][0] * (b[i][0] + b[i][1] + b[i][2] + b[i][3] + b[i][4] + b[i][5]) +
               a[i][1] * (b[i][0] + b[i][1] + b[i][2] + b[i][3] + b[i][4] + b[i][5]) +
               a[i][2] * (b[i][1] + b[i][3]) +
               a[i][3] * (b[i][0] + b[i][2]) +
               a[i][4] * (b[i][0] + b[i][1]) +
               a[i][5] * (b[i][0] + b[i][4]);
    }
    for (i = 0; i < size; i++) {
        // sanitizing after the product is computed, so we can reuse the buffer
        memset(c[i], 0, sizeof(T) * numShares);
    }
    // printf("finished calculating v\n");
    for (uint p_prime = 1; p_prime < numParties + 1; p_prime++) {
        // printf("\n");
        for (uint T_index = 0; T_index < numShares; T_index++) {
            tracker = 0;
            if ((p_prime != (pid)) and (!(p_prime_in_T(p_prime, ss->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T(p_prime, ss->T_map_mpc[T_index], numParties)))) {
                for (i = 0; i < size; i++) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[i][T_index] += z;  // Fixed: [element][share]
                }
                tracker += 1;
            } else if (
                (p_prime == pid) and (!(chi_p_prime_in_T(pid, ss->T_map_mpc[T_index], numParties)))) {
                for (i = 0; i < size; i++) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[i][T_index] += z;  // Fixed: [element][share]
                    v[i] -= z;
                }
                tracker += 1;
            }
        }
    }
    // printf("sending now\n");

    // communication
    nodeNet.SendAndGetDataFromPeer(v, recv_buf, size, ring_size, ss->general_map);
    // nodeNet.SendAndGetDataFromPeer_Mult(v, recv_buf, size, ring_size);
    // ss->prg_getrandom(0, bytes, size, buffer);
    for (i = 0; i < size; i++) {
        c[i][3] = c[i][3] + recv_buf[1][i];  // Fixed: [element][share]
        c[i][5] = c[i][5] + recv_buf[0][i];  // Fixed: [element][share]

        c[i][0] = c[i][0] + v[i];  // Fixed: [element][share]
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

// Interface format: [size][numShares] where array[i][s] = share s of element i
// b[b_index][s] is the fixed element being multiplied with all elements of a
template <typename T>
void Rss_Mult_fixed_b_5pc(T **c, T **a, T **b, uint b_index, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint bytes = (ring_size + 7) >> 3;
    uint i, p_prime, T_index;
    uint numShares = ss->getNumShares();
    uint numParties = ss->getPeers();
    uint threshold = ss->getThreshold();
    int pid = ss->getID();

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
    T z = 0;
    uint tracker;
    for (i = 0; i < size; i++) {
        // Fixed: use a[i][s] for element i, b[b_index][s] for fixed element
        v[i] = a[i][0] * (b[b_index][0] + b[b_index][1] + b[b_index][2] + b[b_index][3] + b[b_index][4] + b[b_index][5]) +
               a[i][1] * (b[b_index][0] + b[b_index][1] + b[b_index][2] + b[b_index][3] + b[b_index][4] + b[b_index][5]) +
               a[i][2] * (b[b_index][1] + b[b_index][3]) +
               a[i][3] * (b[b_index][0] + b[b_index][2]) +
               a[i][4] * (b[b_index][0] + b[b_index][1]) +
               a[i][5] * (b[b_index][0] + b[b_index][4]);
    }

    // Sanitize output in interface format [size][numShares]
    for (i = 0; i < size; i++) {
        memset(c[i], 0, sizeof(T) * numShares);
    }

    for (i = 0; i < size; i++) {
        for (p_prime = 1; p_prime < numParties + 1; p_prime++) {
            for (T_index = 0; T_index < numShares; T_index++) {
                tracker = 0;
                if ((p_prime != (pid)) and (!(p_prime_in_T(p_prime, ss->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T(p_prime, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[i][T_index] += z;  // Fixed: [element][share]
                    tracker += 1;
                } else if (
                    (p_prime == pid) and (!(chi_p_prime_in_T(pid, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[i][T_index] += z;  // Fixed: [element][share]
                    v[i] -= z;
                    tracker += 1;
                }
            }
        }
    }

    // communication
    nodeNet.SendAndGetDataFromPeer(v, recv_buf, size, ring_size, ss->general_map);
    for (i = 0; i < size; i++) {
        c[i][3] = c[i][3] + recv_buf[1][i];  // Fixed: [element][share]
        c[i][5] = c[i][5] + recv_buf[0][i];  // Fixed: [element][share]
        c[i][0] = c[i][0] + v[i];            // Fixed: [element][share]
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

// Interface format: [size][numShares] where array[i][s] = share s of element i
template <typename T>
void Rss_Mult_Byte_5pc(uint8_t **c, uint8_t **a, uint8_t **b, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint i = 0;
    uint numShares = ss->getNumShares();
    uint numParties = ss->getPeers();
    uint threshold = ss->getThreshold();
    uint pid = (uint)ss->getID();

    uint8_t *v = new uint8_t[size];
    memset(v, 0, sizeof(uint8_t) * size);
    uint8_t prg_ctrs[6] = {2, 3, 3, 2, 3, 2};

    uint8_t **recv_buf = new uint8_t *[threshold];
    for (i = 0; i < threshold; i++) {
        recv_buf[i] = new uint8_t[size];
        memset(recv_buf[i], 0, sizeof(uint8_t) * size);
    }

    uint8_t **buffer = new uint8_t *[numShares];
    for (i = 0; i < numShares; i++) {
        buffer[i] = new uint8_t[prg_ctrs[i] * size];
        ss->prg_getrandom(i, 1, prg_ctrs[i] * size, buffer[i]);
    }
    uint trackers[6] = {0, 0, 0, 0, 0, 0};

    for (i = 0; i < size; i++) {
        // Fixed: use a[i][s] instead of a[s][i]
        v[i] = (a[i][0] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][3] ^ b[i][4] ^ b[i][5])) ^
               (a[i][1] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][3] ^ b[i][4] ^ b[i][5])) ^
               (a[i][2] & (b[i][1] ^ b[i][3])) ^
               (a[i][3] & (b[i][0] ^ b[i][2])) ^
               (a[i][4] & (b[i][0] ^ b[i][1])) ^
               (a[i][5] & (b[i][0] ^ b[i][4]));
    }

    // Sanitize output in interface format [size][numShares]
    for (i = 0; i < size; i++) {
        memset(c[i], 0, sizeof(uint8_t) * numShares);
    }

    for (i = 0; i < size; i++) {
        for (uint p_prime = 1; p_prime < numParties + 1; p_prime++) {
            for (uint T_index = 0; T_index < numShares; T_index++) {
                if ((p_prime != (pid)) and (!(p_prime_in_T(p_prime, ss->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T(p_prime, ss->T_map_mpc[T_index], numParties)))) {
                    c[i][T_index] = c[i][T_index] ^ buffer[T_index][trackers[T_index]];  // Fixed: [element][share]
                    trackers[T_index] += 1;
                } else if ((p_prime == pid) and (!(chi_p_prime_in_T(pid, ss->T_map_mpc[T_index], numParties)))) {
                    c[i][T_index] = c[i][T_index] ^ buffer[T_index][trackers[T_index]];  // Fixed: [element][share]
                    v[i] = v[i] ^ buffer[T_index][trackers[T_index]];
                    trackers[T_index] += 1;
                }
            }
        }
    }
    nodeNet.SendAndGetDataFromPeer_bit(v, recv_buf, size, ss->general_map);

    for (i = 0; i < size; i++) {
        c[i][3] = c[i][3] ^ recv_buf[1][i];  // Fixed: [element][share]
        c[i][5] = c[i][5] ^ recv_buf[0][i];  // Fixed: [element][share]
        c[i][0] = c[i][0] ^ v[i];            // Fixed: [element][share]
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
void Rss_Mult_7pc(T **c, T **a, T **b, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    // struct timeval start;
    // struct timeval end;
    // unsigned long timer = 0;

    uint bytes = (ring_size + 7) >> 3;
    uint i;
    uint numShares = ss->getNumShares();
    uint numParties = ss->getPeers();
    uint threshold = ss->getThreshold();
    uint pid = (uint)ss->getID();

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
    // gettimeofday(&start, NULL);
    // Fixed indexing: arrays are [element][share] not [share][element]
    for (i = 0; i < size; i++) {
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

    for (i = 0; i < size; i++) {
        memset(c[i], 0, sizeof(T) * numShares);
    }

    // printf("finished calculating v\n");
    for (uint p_prime = 1; p_prime < numParties + 1; p_prime++) {
        // printf("\n");
        for (uint T_index = 0; T_index < numShares; T_index++) {
            tracker = 0;
            if ((p_prime != (pid)) and (!(p_prime_in_T_7(p_prime, ss->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T_7(p_prime, ss->T_map_mpc[T_index], numParties)))) {
                for (i = 0; i < size; i++) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[i][T_index] += z;  // Fixed: [element][share]
                }
                tracker += 1;
            } else if (
                (p_prime == pid) and (!(chi_p_prime_in_T_7(pid, ss->T_map_mpc[T_index], numParties)))) {
                for (i = 0; i < size; i++) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[i][T_index] += z;  // Fixed: [element][share]
                    v[i] -= z;
                }
                tracker += 1;
            }
        }
    }

    // gettimeofday(&end, NULL); // stop timer here
    // timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    // printf("[7pc local 2] [%.3lf ms]\n", (double)(timer * 0.001));

    // gettimeofday(&start, NULL);
    // communication
    // nodeNet.SendAndGetDataFromPeer_Mult(v, recv_buf, size, ring_size);
    nodeNet.SendAndGetDataFromPeer(v, recv_buf, size, ring_size, ss->general_map);

    // gettimeofday(&end, NULL); // stop timer here
    // timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    // printf("[7pc send recv] [%.3lf ms]\n", (double)(timer * 0.001));

    // ss->prg_getrandom(0, bytes, size, buffer);
    for (i = 0; i < size; i++) {
        c[i][19] = c[i][19] + recv_buf[0][i];  // Fixed: [element][share]
        c[i][16] = c[i][16] + recv_buf[1][i];  // Fixed: [element][share]
        c[i][10] = c[i][10] + recv_buf[2][i];  // Fixed: [element][share]

        c[i][0] = c[i][0] + v[i];  // Fixed: [element][share]
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
    int pid = ss->getID();

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
    // Fixed indexing: arrays are [element][share] not [share][element]
    for (i = 0; i < size; i++) {
        v[i] =
            a[i][0] * (b[b_index][0] + b[b_index][1] + b[b_index][2] + b[b_index][3] + b[b_index][4] + b[b_index][5] + b[b_index][6] + b[b_index][7] + b[b_index][8] + b[b_index][9] + b[b_index][10] + b[b_index][11] + b[b_index][12] + b[b_index][13] + b[b_index][14] + b[b_index][15] + b[b_index][16] + b[b_index][17] + b[b_index][18] + b[b_index][19]) +
            a[i][1] * (b[b_index][0] + b[b_index][1] + b[b_index][2] + b[b_index][3] + b[b_index][4] + b[b_index][5] + b[b_index][6] + b[b_index][7] + b[b_index][8] + b[b_index][9] + b[b_index][10] + b[b_index][11] + b[b_index][12] + b[b_index][13] + b[b_index][14] + b[b_index][15] + b[b_index][16] + b[b_index][17] + b[b_index][18] + b[b_index][19]) +
            a[i][2] * (b[b_index][0] + b[b_index][1] + b[b_index][2] + b[b_index][3] + b[b_index][4] + b[b_index][5] + b[b_index][6] + b[b_index][7] + b[b_index][8] + b[b_index][9] + b[b_index][10] + b[b_index][11] + b[b_index][12] + b[b_index][13] + b[b_index][14] + b[b_index][15] + b[b_index][16] + b[b_index][17] + b[b_index][18] + b[b_index][19]) +
            a[i][3] * (b[b_index][0] + b[b_index][1] + b[b_index][2] + b[b_index][3] + b[b_index][4] + b[b_index][5] + b[b_index][6] + b[b_index][7] + b[b_index][8] + b[b_index][9] + b[b_index][10] + b[b_index][11] + b[b_index][12] + b[b_index][13] + b[b_index][14] + b[b_index][15] + b[b_index][16] + b[b_index][17] + b[b_index][18] + b[b_index][19]) +
            a[i][4] * (b[b_index][0] + b[b_index][1] + b[b_index][2] + b[b_index][3] + b[b_index][10] + b[b_index][11] + b[b_index][12] + b[b_index][13] + b[b_index][15]) +
            a[i][5] * (b[b_index][0] + b[b_index][1] + b[b_index][2] + b[b_index][3] + b[b_index][4] + b[b_index][5] + b[b_index][6] + b[b_index][7] + b[b_index][8] + b[b_index][9] + b[b_index][10] + b[b_index][11] + b[b_index][12] + b[b_index][13] + b[b_index][14] + b[b_index][15] + b[b_index][16] + b[b_index][17] + b[b_index][18] + b[b_index][19]) +
            a[i][6] * (b[b_index][2] + b[b_index][5] + b[b_index][7] + b[b_index][9] + b[b_index][11] + b[b_index][13]) +
            a[i][7] * (b[b_index][0] + b[b_index][4] + b[b_index][5] + b[b_index][6] + b[b_index][10] + b[b_index][11] + b[b_index][12]) +
            a[i][8] * (b[b_index][0] + b[b_index][4] + b[b_index][5] + b[b_index][10] + b[b_index][11] + b[b_index][16]) +
            a[i][9] * (b[b_index][1] + b[b_index][4] + b[b_index][7] + b[b_index][8] + b[b_index][10] + b[b_index][13]) +
            a[i][10] * (b[b_index][0] + b[b_index][1] + b[b_index][2] + b[b_index][3] + b[b_index][4] + b[b_index][5] + b[b_index][9]) +
            a[i][11] * (b[b_index][0] + b[b_index][1] + b[b_index][4] + b[b_index][6] + b[b_index][7] + b[b_index][8]) +
            a[i][12] * (b[b_index][0] + b[b_index][1] + b[b_index][2] + b[b_index][4] + b[b_index][5] + b[b_index][7]) +
            a[i][13] * (b[b_index][0] + b[b_index][4] + b[b_index][5] + b[b_index][6]) +
            a[i][14] * (b[b_index][2] + b[b_index][4] + b[b_index][5]) +
            a[i][15] * (b[b_index][1] + b[b_index][4]) +
            a[i][16] * (b[b_index][0] + b[b_index][1] + b[b_index][2] + b[b_index][3] + b[b_index][15]) +
            a[i][17] * (b[b_index][0] + b[b_index][1] + b[b_index][2]) +
            a[i][18] * (b[b_index][1] + b[b_index][8]) +
            a[i][19] * (b[b_index][0] + b[b_index][5] + b[b_index][6]);
    }
    for (i = 0; i < size; i++) {
        // sanitizing after the product is computed, so we can reuse the buffer
        memset(c[i], 0, sizeof(T) * numShares);
    }
    for (i = 0; i < size; i++) {
        // printf("finished calculating v\n");
        for (p_prime = 1; p_prime < numParties + 1; p_prime++) {
            // printf("\n");
            for (T_index = 0; T_index < numShares; T_index++) {
                tracker = 0;
                if ((p_prime != (pid)) and (!(p_prime_in_T_7(p_prime, ss->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T_7(p_prime, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[i][T_index] += z;  // Fixed: [element][share]
                    tracker += 1;
                } else if (
                    (p_prime == pid) and (!(chi_p_prime_in_T_7(pid, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[i][T_index] += z;  // Fixed: [element][share]
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
        c[i][19] = c[i][19] + recv_buf[0][i];  // Fixed: [element][share]
        c[i][16] = c[i][16] + recv_buf[1][i];  // Fixed: [element][share]
        c[i][10] = c[i][10] + recv_buf[2][i];  // Fixed: [element][share]

        c[i][0] = c[i][0] + v[i];  // Fixed: [element][share]
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
    uint numShares = ss->getNumShares();
    uint numParties = ss->getPeers();
    uint threshold = ss->getThreshold();
    uint pid = (uint)ss->getID();

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
    // Fixed indexing: arrays are [element][share] not [share][element]
    for (i = 0; i < size; i++) {
        v[i] =
            (a[i][0] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][3] ^ b[i][4] ^ b[i][5] ^ b[i][6] ^ b[i][7] ^ b[i][8] ^ b[i][9] ^ b[i][10] ^ b[i][11] ^ b[i][12] ^ b[i][13] ^ b[i][14] ^ b[i][15] ^ b[i][16] ^ b[i][17] ^ b[i][18] ^ b[i][19])) ^
            (a[i][1] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][3] ^ b[i][4] ^ b[i][5] ^ b[i][6] ^ b[i][7] ^ b[i][8] ^ b[i][9] ^ b[i][10] ^ b[i][11] ^ b[i][12] ^ b[i][13] ^ b[i][14] ^ b[i][15] ^ b[i][16] ^ b[i][17] ^ b[i][18] ^ b[i][19])) ^
            (a[i][2] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][3] ^ b[i][4] ^ b[i][5] ^ b[i][6] ^ b[i][7] ^ b[i][8] ^ b[i][9] ^ b[i][10] ^ b[i][11] ^ b[i][12] ^ b[i][13] ^ b[i][14] ^ b[i][15] ^ b[i][16] ^ b[i][17] ^ b[i][18] ^ b[i][19])) ^
            (a[i][3] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][3] ^ b[i][4] ^ b[i][5] ^ b[i][6] ^ b[i][7] ^ b[i][8] ^ b[i][9] ^ b[i][10] ^ b[i][11] ^ b[i][12] ^ b[i][13] ^ b[i][14] ^ b[i][15] ^ b[i][16] ^ b[i][17] ^ b[i][18] ^ b[i][19])) ^
            (a[i][4] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][3] ^ b[i][10] ^ b[i][11] ^ b[i][12] ^ b[i][13] ^ b[i][15])) ^
            (a[i][5] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][3] ^ b[i][4] ^ b[i][5] ^ b[i][6] ^ b[i][7] ^ b[i][8] ^ b[i][9] ^ b[i][10] ^ b[i][11] ^ b[i][12] ^ b[i][13] ^ b[i][14] ^ b[i][15] ^ b[i][16] ^ b[i][17] ^ b[i][18] ^ b[i][19])) ^
            (a[i][6] & (b[i][2] ^ b[i][5] ^ b[i][7] ^ b[i][9] ^ b[i][11] ^ b[i][13])) ^
            (a[i][7] & (b[i][0] ^ b[i][4] ^ b[i][5] ^ b[i][6] ^ b[i][10] ^ b[i][11] ^ b[i][12])) ^
            (a[i][8] & (b[i][0] ^ b[i][4] ^ b[i][5] ^ b[i][10] ^ b[i][11] ^ b[i][16])) ^
            (a[i][9] & (b[i][1] ^ b[i][4] ^ b[i][7] ^ b[i][8] ^ b[i][10] ^ b[i][13])) ^
            (a[i][10] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][3] ^ b[i][4] ^ b[i][5] ^ b[i][9])) ^
            (a[i][11] & (b[i][0] ^ b[i][1] ^ b[i][4] ^ b[i][6] ^ b[i][7] ^ b[i][8])) ^
            (a[i][12] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][4] ^ b[i][5] ^ b[i][7])) ^
            (a[i][13] & (b[i][0] ^ b[i][4] ^ b[i][5] ^ b[i][6])) ^
            (a[i][14] & (b[i][2] ^ b[i][4] ^ b[i][5])) ^
            (a[i][15] & (b[i][1] ^ b[i][4])) ^
            (a[i][16] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][3] ^ b[i][15])) ^
            (a[i][17] & (b[i][0] ^ b[i][1] ^ b[i][2])) ^
            (a[i][18] & (b[i][1] ^ b[i][8])) ^
            (a[i][19] & (b[i][0] ^ b[i][5] ^ b[i][6]));
    }
    for (i = 0; i < size; i++) {
        // sanitizing after the product is computed, so we can reuse the buffer
        memset(c[i], 0, sizeof(T) * numShares);
    }
    for (i = 0; i < size; i++) {
        // printf("finished calculating v\n");
        for (uint p_prime = 1; p_prime < numParties + 1; p_prime++) {
            // printf("\n");
            for (uint T_index = 0; T_index < numShares; T_index++) {
                tracker = 0;
                if ((p_prime != (pid)) and (!(p_prime_in_T_7(p_prime, ss->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T_7(p_prime, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[i][T_index] ^= z;  // Fixed: [element][share]
                    tracker += 1;
                } else if (
                    (p_prime == pid) and (!(chi_p_prime_in_T_7(pid, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[i][T_index] ^= z;  // Fixed: [element][share]
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
        c[i][19] = c[i][19] ^ recv_buf[0][i];  // Fixed: [element][share]
        c[i][16] = c[i][16] ^ recv_buf[1][i];  // Fixed: [element][share]
        c[i][10] = c[i][10] ^ recv_buf[2][i];  // Fixed: [element][share]

        c[i][0] = c[i][0] ^ v[i];  // Fixed: [element][share]
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
    uint numShares = ss->getNumShares();
    uint numParties = ss->getPeers();
    uint threshold = ss->getThreshold();
    uint pid = (uint)ss->getID();

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

    // Fixed indexing: arrays are [element][share] not [share][element]
    for (i = 0; i < size; i++) {
        v[i] =
            (a[i][0] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][3] ^ b[i][4] ^ b[i][5] ^ b[i][6] ^ b[i][7] ^ b[i][8] ^ b[i][9] ^ b[i][10] ^ b[i][11] ^ b[i][12] ^ b[i][13] ^ b[i][14] ^ b[i][15] ^ b[i][16] ^ b[i][17] ^ b[i][18] ^ b[i][19])) ^
            (a[i][1] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][3] ^ b[i][4] ^ b[i][5] ^ b[i][6] ^ b[i][7] ^ b[i][8] ^ b[i][9] ^ b[i][10] ^ b[i][11] ^ b[i][12] ^ b[i][13] ^ b[i][14] ^ b[i][15] ^ b[i][16] ^ b[i][17] ^ b[i][18] ^ b[i][19])) ^
            (a[i][2] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][3] ^ b[i][4] ^ b[i][5] ^ b[i][6] ^ b[i][7] ^ b[i][8] ^ b[i][9] ^ b[i][10] ^ b[i][11] ^ b[i][12] ^ b[i][13] ^ b[i][14] ^ b[i][15] ^ b[i][16] ^ b[i][17] ^ b[i][18] ^ b[i][19])) ^
            (a[i][3] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][3] ^ b[i][4] ^ b[i][5] ^ b[i][6] ^ b[i][7] ^ b[i][8] ^ b[i][9] ^ b[i][10] ^ b[i][11] ^ b[i][12] ^ b[i][13] ^ b[i][14] ^ b[i][15] ^ b[i][16] ^ b[i][17] ^ b[i][18] ^ b[i][19])) ^
            (a[i][4] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][3] ^ b[i][10] ^ b[i][11] ^ b[i][12] ^ b[i][13] ^ b[i][15])) ^
            (a[i][5] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][3] ^ b[i][4] ^ b[i][5] ^ b[i][6] ^ b[i][7] ^ b[i][8] ^ b[i][9] ^ b[i][10] ^ b[i][11] ^ b[i][12] ^ b[i][13] ^ b[i][14] ^ b[i][15] ^ b[i][16] ^ b[i][17] ^ b[i][18] ^ b[i][19])) ^
            (a[i][6] & (b[i][2] ^ b[i][5] ^ b[i][7] ^ b[i][9] ^ b[i][11] ^ b[i][13])) ^
            (a[i][7] & (b[i][0] ^ b[i][4] ^ b[i][5] ^ b[i][6] ^ b[i][10] ^ b[i][11] ^ b[i][12])) ^
            (a[i][8] & (b[i][0] ^ b[i][4] ^ b[i][5] ^ b[i][10] ^ b[i][11] ^ b[i][16])) ^
            (a[i][9] & (b[i][1] ^ b[i][4] ^ b[i][7] ^ b[i][8] ^ b[i][10] ^ b[i][13])) ^
            (a[i][10] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][3] ^ b[i][4] ^ b[i][5] ^ b[i][9])) ^
            (a[i][11] & (b[i][0] ^ b[i][1] ^ b[i][4] ^ b[i][6] ^ b[i][7] ^ b[i][8])) ^
            (a[i][12] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][4] ^ b[i][5] ^ b[i][7])) ^
            (a[i][13] & (b[i][0] ^ b[i][4] ^ b[i][5] ^ b[i][6])) ^
            (a[i][14] & (b[i][2] ^ b[i][4] ^ b[i][5])) ^
            (a[i][15] & (b[i][1] ^ b[i][4])) ^
            (a[i][16] & (b[i][0] ^ b[i][1] ^ b[i][2] ^ b[i][3] ^ b[i][15])) ^
            (a[i][17] & (b[i][0] ^ b[i][1] ^ b[i][2])) ^
            (a[i][18] & (b[i][1] ^ b[i][8])) ^
            (a[i][19] & (b[i][0] ^ b[i][5] ^ b[i][6]));
    }
    for (i = 0; i < size; i++) {
        // sanitizing after the product is computed, so we can reuse the buffer
        memset(c[i], 0, sizeof(uint8_t) * numShares);
    }
    for (i = 0; i < size; i++) {
        // printf("finished calculating v\n");
        for (uint p_prime = 1; p_prime < numParties + 1; p_prime++) {
            // printf("\n");
            for (uint T_index = 0; T_index < numShares; T_index++) {
                tracker = 0;
                if ((p_prime != (pid)) and (!(p_prime_in_T_7(p_prime, ss->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T_7(p_prime, ss->T_map_mpc[T_index], numParties)))) {
                    c[i][T_index] ^= buffer[T_index][trackers[T_index]];  // Fixed: [element][share]
                    tracker += 1;
                } else if (
                    (p_prime == pid) and (!(chi_p_prime_in_T_7(pid, ss->T_map_mpc[T_index], numParties)))) {
                    c[i][T_index] = c[i][T_index] ^ buffer[T_index][trackers[T_index]];  // Fixed: [element][share]

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
        c[i][19] = c[i][19] ^ recv_buf[0][i];  // Fixed: [element][share]
        c[i][16] = c[i][16] ^ recv_buf[1][i];  // Fixed: [element][share]
        c[i][10] = c[i][10] ^ recv_buf[2][i];  // Fixed: [element][share]

        c[i][0] = c[i][0] ^ v[i];  // Fixed: [element][share]
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
