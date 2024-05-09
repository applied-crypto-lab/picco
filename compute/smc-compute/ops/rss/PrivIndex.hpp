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

#ifndef _PRIVINDEX_HPP_
#define _PRIVINDEX_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"

// input array is bits shared over Z2 (packed into a single T)
// array [numShares][size]
// result [numShares][size*(2^k)] (interpreted as 2^k blocks of dimension "size")
template <typename T>
void AllOr(T **array, int k, T **result, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {

    static uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;
    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, 1);


    if (k == 1) {
        for (size_t s = 0; s < numShares; s++) {
            for (int i = 0; i < size; i++) {
                result[s][i] = GET_BIT(array[s][i], 0);
                result[s][size + i] = (T(1) & ai[s]) ^ GET_BIT(array[s][i], 0);
            }
        }
        return;
    }

    int b_size = 1 << (k + 1);
    int total_size = b_size * size;

    //  mpz_t *buff = (mpz_t *)malloc(sizeof(mpz_t) * b_size * batch_size);
    //  mpz_t *u1 = (mpz_t *)malloc(sizeof(mpz_t) * b_size * batch_size);
    //  mpz_t *v1 = (mpz_t *)malloc(sizeof(mpz_t) * b_size * batch_size);
    //  mpz_t *add_b = (mpz_t *)malloc(sizeof(mpz_t) * b_size * batch_size);
    //  mpz_t *mul_b = (mpz_t *)malloc(sizeof(mpz_t) * b_size * batch_size);
    //  for (int i = 0; i < b_size * batch_size; i++) {
    //      mpz_init(buff[i]);
    //      mpz_init(u1[i]);
    //      mpz_init(v1[i]);
    //      mpz_init(add_b[i]);
    //      mpz_init(mul_b[i]);
    //  }
    T **buff = new T *[numShares];
    T **u1 = new T *[numShares];
    T **v1 = new T *[numShares];
    T **add_b = new T *[numShares];
    T **mul_b = new T *[numShares];
    for (size_t i = 0; i < numShares; i++) {
        buff[i] = new T[total_size];
        memset(buff[i], 0, sizeof(T) * total_size);
        u1[i] = new T[total_size];
        memset(u1[i], 0, sizeof(T) * total_size);
        v1[i] = new T[total_size];
        memset(v1[i], 0, sizeof(T) * total_size);
        add_b[i] = new T[total_size];
        memset(add_b[i], 0, sizeof(T) * total_size);
        mul_b[i] = new T[total_size];
        memset(mul_b[i], 0, sizeof(T) * total_size);
    }

    int round = 0;
    int nS = k - 1;
    int sizeLen = 2;
    while (nS > 1) {
        round++;
        nS /= 2;
        sizeLen *= 2;
    }
    vector<int> sizeArray(sizeLen);
    //  int *sizeArray = (int *)malloc(sizeof(int) * sizeLen);
    int len = 1;
    sizeArray[0] = k; // divide and get unit size
    for (int x = 0; x <= round; x++) {
        for (int i = len - 1; i >= 0; i--) {
            int val = sizeArray[i];
            sizeArray[i * 2] = val / 2;
            sizeArray[i * 2 + 1] = val / 2;
            if (val & 0x01)
                sizeArray[i * 2 + 1]++;
        }
        len *= 2;
    }

    int oPos = 0;
    int iPos = 0;

    for (size_t s = 0; s < numShares; s++) {
        for (int n = 0; n < size; n++) {
            iPos = 0;
            for (int i = 0; i < sizeLen; i += 2) {
                if (sizeArray[i] != 0 && sizeArray[i + 1] != 0) {
                    u1[s][oPos] = GET_BIT(array[s][n], iPos);
                    //  mpz_set(u1[oPos], array[n][iPos]);
                    u1[s][oPos + 1] = (T(1) & ai[s]) ^  GET_BIT(array[s][n], iPos);
                    // ss->modSub(u1[oPos + 1], 1, array[n][iPos]);
                    mpz_set(u1[oPos + 2], u1[oPos]);
                    mpz_set(u1[oPos + 3], u1[oPos + 1]);
                    v1[s][oPos] = GET_BIT(array[s][n], iPos + 1);
                    //  mpz_set(v1[oPos], array[n][iPos + 1]);
                    mpz_set(v1[oPos + 1], v1[oPos]);
                    v1[s][oPos + 1] = (T(1) & ai[s]) ^  GET_BIT(array[s][n], iPos + 1);
                    // ss->modSub(v1[oPos + 2], 1, array[n][iPos + 1]);
                    mpz_set(v1[oPos + 3], v1[oPos + 2]);
                    iPos += 2;
                    oPos += 4;
                } else
                    iPos++;
            }
        }
    }
    ss->modAdd(add_b, u1, v1, oPos);
    // 1 rou, net, ssnd
    Mult(mul_b, u1, v1, oPos, threadID, net, ss);
    ss->modSub(u1, add_b, mul_b, oPos);

    for (size_t i = 0; i < numShares; i++) {
        delete[] buff[i];
        delete[] u1[i];
        delete[] v1[i];
        delete[] add_b[i];
        delete[] mul_b[i];
    }
    delete[] buff;
    delete[] u1;
    delete[] v1;
    delete[] add_b;
    delete[] mul_b;
    delete[] ai;

}

template <typename T>
void doOperation_PrivIndex_Write(T **index, T **array, int *values, int dim, int size, T *out_cond, T **priv_cond, int counter, int threadID, int type, NodeNetwork net, replicatedSecretShare<T> *ss) {
}

// array : [numShares][size * dim]
// index : [numShares][size]
template <typename T>
void doOperation_PrivIndex_Read(T **index, T **array, T **result, int dim, int size, int threadID, int type, NodeNetwork net, replicatedSecretShare<T> *ss) {
}

template <typename T>
void compute_private_conditions(T **private_conditions, T *out_cond, T **priv_cond, int counter, int size) {
}
template <typename T>
void doOperation_PrivIndex_int(T *index, T **array, T *result, int dim, int type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_float(T *index, T ***array, T **result, int dim, int type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_int_arr(T *index, T ***array, T *result, int dim1, int dim2, int type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_float_arr(T *index, T ****array, T **result, int dim1, int dim2, int type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_Write(T **index, T **array, T **value, int dim, int size, T *out_cond, T **priv_cond, int counter, int threadID, int type, NodeNetwork net, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_Write_2d(T **index, T ***array, int *values, int dim1, int dim2, int size, T *out_cond, T **priv_cond, int counter, int threadID, int type, NodeNetwork net, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_Write_2d(T **index, T ***array, T **values, int dim1, int dim2, int size, T *out_cond, T **priv_cond, int counter, int threadID, int type, NodeNetwork net, replicatedSecretShare<T> *ss) {
}
#endif // _PRIVINDEX_HPP_