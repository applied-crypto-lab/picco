#ifndef OPEN_H_
#define OPEN_H_

#include "../../RSS_SecretShare.h"
#include "../../NodeNetwork.h"

template <typename T>
void Rss_Open_3pc(T *res, T **a, uint size, uint ring_size, NodeNetwork *nodeNet, RSS_SecretShare<T> *ss) {
    uint threshold = ss->getThreshold();
    int i;
    nodeNet->SendAndGetDataFromPeer(a[1], res, size, ring_size, ss->general_map, threshold);
    for (i = 0; i < size; i++) {
        res[i] = a[0][i] + a[1][i] + res[i];
        res[i] = res[i] & ss->SHIFT[ring_size];
    }
}

template <typename T>
void Rss_Open_Byte_3pc(uint8_t *res, uint8_t **a, uint size, uint ring_size, NodeNetwork *nodeNet, RSS_SecretShare<T> *ss) {
    uint threshold = ss->getThreshold();
    int i;
    nodeNet->SendAndGetDataFromPeer_bit(a[1], res, size, ss->general_map, threshold);
    for (i = 0; i < size; i++) {
        res[i] = a[0][i] ^ a[1][i] ^ res[i];
    }
}

template <typename T>
void Rss_Open_Bitwise_3pc(T *res, T **a, uint size, uint ring_size, NodeNetwork *nodeNet, RSS_SecretShare<T> *ss) {
    uint threshold = ss->getThreshold();
    int i;
    nodeNet->SendAndGetDataFromPeer(a[1], res, size, ring_size, ss->general_map, threshold);
    for (i = 0; i < size; i++) {
        res[i] = a[0][i] ^ a[1][i] ^ res[i];
        res[i] = res[i] & ss->SHIFT[ring_size];
    }
}

template <typename T>
void Rss_Open_5pc(T *res, T **a, uint size, uint ring_size, NodeNetwork *nodeNet, RSS_SecretShare<T> *ss) {
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

    nodeNet->SendAndGetDataFromPeer(sendbuf, recvbuf, size, ring_size, ss->open_map_mpc, threshold);
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
void Rss_Open_Byte_5pc(uint8_t *res, uint8_t **a, uint size, uint ring_size, NodeNetwork *nodeNet, RSS_SecretShare<T> *ss) {

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

    nodeNet->SendAndGetDataFromPeer_bit(sendbuf, recvbuf, size, ss->open_map_mpc, threshold);
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
void Rss_Open_Bitwise_5pc(T *res, T **a, uint size, uint ring_size, NodeNetwork *nodeNet, RSS_SecretShare<T> *ss) {
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

    nodeNet->SendAndGetDataFromPeer(sendbuf, recvbuf, size, ring_size, ss->open_map_mpc , threshold);
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
void Rss_Open_7pc(T *res, T **a, uint size, uint ring_size, NodeNetwork *nodeNet, RSS_SecretShare<T> *ss) {
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
        sendbuf[0][i] = a[0][i] + a[1][i] + a[2][i] + a[3][i] + a[4][i];
        sendbuf[1][i] = a[10][i] + a[11][i] + a[12][i] + a[13][i] + a[14][i];
        sendbuf[2][i] = a[1][i] + a[5][i] + a[16][i] + a[17][i] + a[18][i];
    }

    nodeNet->SendAndGetDataFromPeer(sendbuf, recvbuf, size, ring_size, ss->open_map_mpc , threshold);
    for (i = 0; i < size; i++) {
        res[i] = a[0][i] + a[1][i] + a[2][i] + a[3][i] + a[4][i] + a[5][i] + a[6][i] + a[7][i] + a[8][i] + a[9][i] + a[10][i] + a[11][i] + a[12][i] + a[13][i] + a[14][i] + a[15][i] + a[16][i] + a[17][i] + a[18][i] + a[19][i];

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


#endif
