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

#ifndef _EQZ_HPP_
#define _EQZ_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "B2A.hpp"
#include "EdaBit.hpp"
#include "Mult.hpp"
#include "Open.hpp"

template <typename T>
void Rss_k_OR_L(T **res, T **r, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss);

template <typename T>
void doOperation_EQZ(T **shares, T **result, int K, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint ring_size = ss->ring_size;
    static uint numShares = ss->getNumShares();
    uint i; // used for loops

    T **sum = new T *[numShares];
    T **edaBit_r = new T *[numShares];
    T **edaBit_b_2 = new T *[numShares];
    T **rprime = new T *[numShares];
    for (i = 0; i < numShares; i++) {
        edaBit_r[i] = new T[size];
        edaBit_b_2[i] = new T[size];
        sum[i] = new T[size];
        rprime[i] = new T[size];
    }
    T *res_check = new T[size];

    T *c = new T[size];

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, -1);

    edaBit(edaBit_r, edaBit_b_2, ring_size, size, ring_size, nodeNet, ss);

    for (size_t s = 0; s < numShares; s++) {
        for (i = 0; i < size; i++) {
            // rprime[s][i] = edaBit_r[s][i] - (GET_BIT(edaBit_b_2[s][i], T(ring_size - 1)) << T(ring_size)); //original version from new primitives
            rprime[s][i] = edaBit_r[s][i];
            sum[s][i] = (shares[s][i] + rprime[s][i]);
        }
    }

    Open(c, sum, size, -1, nodeNet, ss);

    for (size_t s = 0; s < numShares; s++) {
        for (i = 0; i < size; i++) {
            rprime[s][i] = (c[i] & ai[s]) ^ edaBit_b_2[s][i]; // computing XOR in Z2
        }
    }

    Rss_k_OR_L(result, rprime, size, ring_size, nodeNet, ss);

    for (size_t s = 0; s < numShares; s++) {
        for (i = 0; i < size; i++) {
            result[s][i] = (T(1) & ai[s]) ^ result[s][i]; // CHECK THIS (equivalent to computing 1 - result, but in Z2)
        }
    }

    Rss_B2A(result, result, size, ring_size, nodeNet, ss);

    delete[] c;
    delete[] ai;
    delete[] res_check;

    for (i = 0; i < numShares; i++) {
        delete[] edaBit_r[i];
        delete[] edaBit_b_2[i];
        delete[] sum[i];
        delete[] rprime[i];
    }
    delete[] edaBit_r;
    delete[] edaBit_b_2;
    delete[] sum;
    delete[] rprime;
}

template <typename T>
void Rss_k_OR_L(T **res, T **r, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    static uint numShares = ss->getNumShares();
    uint i, s, originial_num, num, r_size, n_uints, new_r_size;
    for (i = 0; i < size; i++) {
        // removing any leading bits that could interfere with future masking
        // this is fine since shares are bitwise and won't affect the final result
        for (s = 0; s < numShares; s++) {
            r[s][i] = r[s][i] & ss->SHIFT[ring_size];
        }
    }

    T **temp_res = new T *[numShares];
    for (i = 0; i < numShares; i++) {
        temp_res[i] = new T[size];
    }

    r_size = ring_size;

    originial_num = (((r_size >> 1) + 7) >> 3) * size;

    uint8_t **a = new uint8_t *[numShares];
    uint8_t **b = new uint8_t *[numShares];
    uint8_t **u = new uint8_t *[numShares];

    for (i = 0; i < numShares; i++) {
        a[i] = new uint8_t[originial_num];
        memset(a[i], 0, sizeof(uint8_t) * originial_num);
        b[i] = new uint8_t[originial_num];
        memset(b[i], 0, sizeof(uint8_t) * originial_num);
        u[i] = new uint8_t[originial_num];
        memset(u[i], 0, sizeof(uint8_t) * originial_num);
    }
    T temp1, temp2, msk;

    while (r_size > 1) {

        new_r_size = r_size >> 1; // dividing by two because we only need half as many uuint8_t's in the first (and subsequent rounds)

        n_uints = ((new_r_size + 7) >> 3);
        num = ((new_r_size + 7) >> 3) * size;

        msk = (T(1) << new_r_size) - T(1);

        for (i = 0; i < size; ++i) {
            for (s = 0; s < numShares; s++) {
                temp1 = r[s][i] & msk;
                temp2 = (r[s][i] & (msk << new_r_size)) >> new_r_size;

                memcpy(a[s] + i * n_uints, &temp1, n_uints);
                memcpy(b[s] + i * n_uints, &temp2, n_uints);
            }
        }

        Mult_Byte(u, a, b, num, nodeNet, ss);

        for (s = 0; s < numShares; s++) {
            for (i = 0; i < num; ++i) {
                u[s][i] = a[s][i] ^ b[s][i] ^ u[s][i];
            }
        }

        for (s = 0; s < numShares; s++) {
            for (i = 0; i < size; ++i) {
                memcpy(temp_res[s] + i, u[s] + i * n_uints, n_uints);
                temp_res[s][i] = temp_res[s][i];
            }
        }

        // need to move the unused bit forward
        if ((r_size & 1)) {
            for (i = 0; i < size; ++i) {
                for (s = 0; s < numShares; s++) {
                    temp_res[s][i] = SET_BIT(temp_res[s][i], T(new_r_size), GET_BIT(r[s][i], T(r_size - 1)));
                }
            }
            new_r_size += 1;
        }

        for (i = 0; i < size; ++i) {
            for (s = 0; s < numShares; s++) {
                r[s][i] = temp_res[s][i] & ss->SHIFT[new_r_size]; // santizing leading > new_ring_size bits for next round
            }
        }

        r_size = new_r_size;
    }

    for (i = 0; i < size; ++i) {
        for (s = 0; s < numShares; s++) {
            res[s][i] = r[s][i];
        }
    }
    for (i = 0; i < numShares; i++) {

        delete[] a[i];
        delete[] b[i];
        delete[] u[i];
        delete[] temp_res[i];
    }

    delete[] a;
    delete[] b;
    delete[] u;
    delete[] temp_res;
}

template <typename T>
void doOperation_EQZ(T **result, T **a, T **b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_EQZ(T **result, T **a, int *b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_EQZ_bit(T **result, T **a, T **b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_EQZ_bit(T **result, T **a, int *b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
}

#endif // _EQZ_HPP_
