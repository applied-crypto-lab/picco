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

#ifndef OPEN_H_
#define OPEN_H_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"

template <typename T>
int Open_int(T *var, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *s) {
    return 0;
}

template <typename T>
float Open_float(T **var, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    return 0.0;
}

template <typename T>
void Rss_Open_3pc(T *res, T **a, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    int i;
    nodeNet.SendAndGetDataFromPeer(a[1], res, int(size), ring_size, ss->general_map);
    for (i = 0; i < size; i++) {
        res[i] = a[0][i] + a[1][i] + res[i];
        res[i] = res[i] & ss->SHIFT[ring_size];
    }
}

template <typename T>
void Rss_Open_Byte_3pc(uint8_t *res, uint8_t **a, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    int i;
    nodeNet.SendAndGetDataFromPeer_bit(a[1], res, int(size), ss->general_map);
    for (i = 0; i < size; i++) {
        res[i] = a[0][i] ^ a[1][i] ^ res[i];
    }
}

template <typename T>
void Rss_Open_Bitwise_3pc(T *res, T **a, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    int i;
    nodeNet.SendAndGetDataFromPeer(a[1], res, size, ring_size, ss->general_map);
    for (i = 0; i < size; i++) {
        res[i] = a[0][i] ^ a[1][i] ^ res[i];
        res[i] = res[i] & ss->SHIFT[ring_size];
    }
}

template <typename T>
void Rss_Open_5pc(T *res, T **a, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint threshold = ss->getThreshold();
    int i;
    T **recvbuf = new T *[threshold];
    T **sendbuf = new T *[threshold];
    for (i = 0; i < threshold; i++) {
        recvbuf[i] = new T[size];
        memset(recvbuf[i], 0, sizeof(T) * size);
        sendbuf[i] = new T[size];
        memset(sendbuf[i], 0, sizeof(T) * size);
    }
    for (i = 0; i < size; i++) {
        sendbuf[0][i] = a[0][i] + a[1][i];
        sendbuf[1][i] = a[4][i] + a[5][i];
    }

    nodeNet.SendAndGetDataFromPeer(sendbuf, recvbuf, size, ring_size, ss->open_map_mpc);
    for (i = 0; i < size; i++) {
        res[i] = a[0][i] + a[1][i] + a[2][i] + a[3][i] + a[4][i] + a[5][i] + recvbuf[0][i] + recvbuf[1][i];
        res[i] = res[i] & ss->SHIFT[ring_size];
    }
    for (i = 0; i < threshold; i++) {
        delete[] recvbuf[i];
        delete[] sendbuf[i];
    }
    delete[] recvbuf;
    delete[] sendbuf;
}

template <typename T>
void Rss_Open_Byte_5pc(uint8_t *res, uint8_t **a, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint threshold = ss->getThreshold();
    int i;
    uint8_t **recvbuf = new uint8_t *[threshold];
    uint8_t **sendbuf = new uint8_t *[threshold];
    for (i = 0; i < threshold; i++) {
        recvbuf[i] = new uint8_t[size];
        sendbuf[i] = new uint8_t[size];
        memset(recvbuf[i], 0, sizeof(uint8_t) * size);
        memset(sendbuf[i], 0, sizeof(uint8_t) * size);
    }

    for (i = 0; i < size; i++) {
        sendbuf[0][i] = a[0][i] ^ a[1][i];
        sendbuf[1][i] = a[4][i] ^ a[5][i];
    }

    nodeNet.SendAndGetDataFromPeer_bit(sendbuf, recvbuf, size, ss->open_map_mpc);
    for (i = 0; i < size; i++) {
        res[i] = a[0][i] ^ a[1][i] ^ a[2][i] ^ a[3][i] ^ a[4][i] ^ a[5][i] ^ recvbuf[0][i] ^ recvbuf[1][i];
    }
    for (i = 0; i < threshold; i++) {
        delete[] recvbuf[i];
        delete[] sendbuf[i];
    }
    delete[] recvbuf;
    delete[] sendbuf;
}

template <typename T>
void Rss_Open_Bitwise_5pc(T *res, T **a, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint threshold = ss->getThreshold();
    int i;
    T **recvbuf = new T *[threshold];
    T **sendbuf = new T *[threshold];
    for (i = 0; i < threshold; i++) {
        recvbuf[i] = new T[size];
        memset(recvbuf[i], 0, sizeof(T) * size);
        sendbuf[i] = new T[size];
        memset(sendbuf[i], 0, sizeof(T) * size);
    }
    for (i = 0; i < size; i++) {
        sendbuf[0][i] = a[0][i] ^ a[1][i];
        sendbuf[1][i] = a[4][i] ^ a[5][i];
    }

    nodeNet.SendAndGetDataFromPeer(sendbuf, recvbuf, size, ring_size, ss->open_map_mpc);
    for (i = 0; i < size; i++) {
        res[i] = a[0][i] ^ a[1][i] ^ a[2][i] ^ a[3][i] ^ a[4][i] ^ a[5][i] ^ recvbuf[0][i] ^ recvbuf[1][i];
        res[i] = res[i] & ss->SHIFT[ring_size];
    }
    for (i = 0; i < threshold; i++) {
        delete[] recvbuf[i];
        delete[] sendbuf[i];
    }
    delete[] recvbuf;
    delete[] sendbuf;
}

template <typename T>
void Rss_Open_7pc(T *res, T **a, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint threshold = ss->getThreshold();
    uint numShares = ss->getNumShares();
    int i;
    T **recvbuf = new T *[threshold];
    T **sendbuf = new T *[threshold];
    for (i = 0; i < threshold; i++) {
        recvbuf[i] = new T[size];
        memset(recvbuf[i], 0, sizeof(T) * size);
        sendbuf[i] = new T[size];
        memset(sendbuf[i], 0, sizeof(T) * size);
    }
    for (i = 0; i < size; i++) {
        sendbuf[0][i] = a[9][i] + a[8][i] + a[6][i] + a[3][i] + a[4][i];
        sendbuf[1][i] = a[3][i] + a[15][i] + a[14][i] + a[12][i] + a[11][i];
        sendbuf[2][i] = a[0][i] + a[12][i] + a[11][i] + a[10][i] + a[16][i];
    }

    nodeNet.SendAndGetDataFromPeer(sendbuf, recvbuf, size, ring_size, ss->open_map_mpc);
    for (i = 0; i < size; i++) {
        res[i] = 0;
        for (uint j = 0; j < numShares; j++) {
            res[i] += a[j][i];
        }
        res[i] += recvbuf[0][i] + recvbuf[1][i] + recvbuf[2][i];
        res[i] = res[i] & ss->SHIFT[ring_size];
    }
    for (i = 0; i < threshold; i++) {
        delete[] recvbuf[i];
        delete[] sendbuf[i];
    }
    delete[] recvbuf;
    delete[] sendbuf;
}

template <typename T>
void Rss_Open_Bitwise_7pc(T *res, T **a, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint threshold = ss->getThreshold();
    uint numShares = ss->getNumShares();
    int i;
    T **recvbuf = new T *[threshold];
    T **sendbuf = new T *[threshold];
    for (i = 0; i < threshold; i++) {
        recvbuf[i] = new T[size];
        memset(recvbuf[i], 0, sizeof(T) * size);
        sendbuf[i] = new T[size];
        memset(sendbuf[i], 0, sizeof(T) * size);
    }
    for (i = 0; i < size; i++) {
        sendbuf[0][i] = a[9][i] ^ a[8][i] ^ a[6][i] ^ a[3][i] ^ a[4][i];
        sendbuf[1][i] = a[3][i] ^ a[15][i] ^ a[14][i] ^ a[12][i] ^ a[11][i];
        sendbuf[2][i] = a[0][i] ^ a[12][i] ^ a[11][i] ^ a[10][i] ^ a[16][i];
    }

    nodeNet.SendAndGetDataFromPeer(sendbuf, recvbuf, size, ring_size, ss->open_map_mpc);
    for (i = 0; i < size; i++) {
        res[i] = 0;
        for (uint j = 0; j < numShares; j++) {
            res[i] ^= a[j][i];
        }
        res[i] ^= recvbuf[0][i] ^ recvbuf[1][i] ^ recvbuf[2][i];
        res[i] = res[i] & ss->SHIFT[ring_size];
    }
    for (i = 0; i < threshold; i++) {
        delete[] recvbuf[i];
        delete[] sendbuf[i];
    }
    delete[] recvbuf;
    delete[] sendbuf;
}

template <typename T>
void Rss_Open_Byte_7pc(uint8_t *res, uint8_t **a, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint threshold = ss->getThreshold();
    uint numShares = ss->getNumShares();
    int i;
    uint8_t **recvbuf = new uint8_t *[threshold];
    uint8_t **sendbuf = new uint8_t *[threshold];
    for (i = 0; i < threshold; i++) {
        recvbuf[i] = new uint8_t[size];
        sendbuf[i] = new uint8_t[size];
        memset(recvbuf[i], 0, sizeof(uint8_t) * size);
        memset(sendbuf[i], 0, sizeof(uint8_t) * size);
    }
    for (i = 0; i < size; i++) {
        sendbuf[0][i] = a[9][i] ^ a[8][i] ^ a[6][i] ^ a[3][i] ^ a[4][i];
        sendbuf[1][i] = a[3][i] ^ a[15][i] ^ a[14][i] ^ a[12][i] ^ a[11][i];
        sendbuf[2][i] = a[0][i] ^ a[12][i] ^ a[11][i] ^ a[10][i] ^ a[16][i];
    }

    nodeNet.SendAndGetDataFromPeer_bit(sendbuf, recvbuf, size, ss->open_map_mpc);
    for (i = 0; i < size; i++) {
        res[i] = 0;
        for (uint j = 0; j < numShares; j++) {
            res[i] ^= a[j][i];
        }
        res[i] ^= recvbuf[0][i] ^ recvbuf[1][i] ^ recvbuf[2][i];
    }
    for (i = 0; i < threshold; i++) {
        delete[] recvbuf[i];
        delete[] sendbuf[i];
    }
    delete[] recvbuf;
    delete[] sendbuf;
}

template <typename T>
void Open(T *result, T **shares, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    try {
        int peers = ss->getPeers();
        switch (peers) {
        case 3:
            Rss_Open_3pc(result, shares, size, ss->ring_size, nodeNet, ss);
            break;
        case 5:
            Rss_Open_5pc(result, shares, size, ss->ring_size, nodeNet, ss);
            break;
        case 7:
            Rss_Open_7pc(result, shares, size, ss->ring_size, nodeNet, ss);
            break;
        default:
            throw std::runtime_error("invalid number of parties");
        }
    } catch (const std::runtime_error &ex) {
        std::string error(ex.what());
        throw std::runtime_error("[Open] " + error);
    }
}

template <typename T>
void Open_Bitwise(T *result, T **shares, int size, uint ring_size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    try {
        int peers = ss->getPeers();
        switch (peers) {
        case 3:
            Rss_Open_Bitwise_3pc(result, shares, size, ring_size, nodeNet, ss);
            break;
        case 5:
            Rss_Open_Bitwise_5pc(result, shares, size, ring_size, nodeNet, ss);
            break;
        case 7:
            Rss_Open_Bitwise_7pc(result, shares, size, ring_size, nodeNet, ss);
            break;
        default:
            throw std::runtime_error("invalid number of parties");
        }
    } catch (const std::runtime_error &ex) {
        std::string error(ex.what());
        throw std::runtime_error("[Open_Bitwise] " + error);
    }
}

template <typename T>
void Open_Bitwise(T *result, T **shares, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    try {
        int peers = ss->getPeers();
        switch (peers) {
        case 3:
            Rss_Open_Bitwise_3pc(result, shares, size, ss->ring_size, nodeNet, ss);
            break;
        case 5:
            Rss_Open_Bitwise_5pc(result, shares, size, ss->ring_size, nodeNet, ss);
            break;
        case 7:
            Rss_Open_Bitwise_7pc(result, shares, size, ss->ring_size, nodeNet, ss);
            break;
        default:
            throw std::runtime_error("invalid number of parties");
        }
    } catch (const std::runtime_error &ex) {
        std::string error(ex.what());
        throw std::runtime_error("[Open_Bitwise] " + error);
    }
}


template <typename T>
void Open_Byte(uint8_t *result, uint8_t **shares, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    try {
        int peers = ss->getPeers();
        switch (peers) {
        case 3:
            Rss_Open_Byte_3pc(result, shares, size, nodeNet, ss);
            break;
        case 5:
            Rss_Open_Byte_5pc(result, shares, size, nodeNet, ss);
            break;
        case 7:
            Rss_Open_Byte_7pc(result, shares, size, nodeNet, ss);
            break;
        default:
            throw std::runtime_error("invalid number of parties");
        }
    } catch (const std::runtime_error &ex) {
        std::string error(ex.what());
        throw std::runtime_error("[Open_Bitwise] " + error);
    }
}

#endif
