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

// Core EQZ function - uses [element][share] format: data[i][s] = share s of element i
template <typename T>
void doOperation_EQZ(T **shares, T **result, int K, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint ring_size = ss->ring_size;
    uint numShares = ss->getNumShares();
    uint i, s; // used for loops

    // Allocate in [element][share] format: array[size][numShares]
    T **sum = new T *[size];
    T **edaBit_r = new T *[size];
    T **edaBit_b_2 = new T *[size];
    T **rprime = new T *[size];
    for (i = 0; i < (uint)size; i++) {
        edaBit_r[i] = new T[numShares];
        edaBit_b_2[i] = new T[numShares];
        sum[i] = new T[numShares];
        rprime[i] = new T[numShares];
    }
    T *res_check = new T[size];

    T *c = new T[size];

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, -1);

    edaBit(edaBit_r, edaBit_b_2, ring_size, size, ring_size, nodeNet, ss);

    // Format: data[i][s] = share s of element i
    for (i = 0; i < (uint)size; i++) {
        for (s = 0; s < numShares; s++) {
            rprime[i][s] = edaBit_r[i][s];
            sum[i][s] = (shares[i][s] + rprime[i][s]);
        }
    }

    Open(c, sum, size, -1, nodeNet, ss);

    for (i = 0; i < (uint)size; i++) {
        for (s = 0; s < numShares; s++) {
            rprime[i][s] = (c[i] & ai[s]) ^ edaBit_b_2[i][s]; // computing XOR in Z2
        }
    }

    Rss_k_OR_L(result, rprime, size, ring_size, nodeNet, ss);

    for (i = 0; i < (uint)size; i++) {
        for (s = 0; s < numShares; s++) {
            result[i][s] = (T(1) & ai[s]) ^ result[i][s]; // 1 - result in Z2
        }
    }

    Rss_B2A(result, result, size, ring_size, nodeNet, ss);

    delete[] c;
    delete[] ai;
    delete[] res_check;

    for (i = 0; i < (uint)size; i++) {
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

// Rss_k_OR_L - uses [element][share] format: data[i][s] = share s of element i
template <typename T>
void Rss_k_OR_L(T **res, T **r, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    uint i, s, originial_num, num, r_size, n_uints, new_r_size;

    // Remove leading bits - r[i][s] = share s of element i
    for (i = 0; i < size; i++) {
        for (s = 0; s < numShares; s++) {
            r[i][s] = r[i][s] & ss->SHIFT[ring_size];
        }
    }

    // Allocate temp_res in [element][share] format
    T **temp_res = new T *[size];
    for (i = 0; i < size; i++) {
        temp_res[i] = new T[numShares];
    }

    r_size = ring_size;

    originial_num = (((r_size >> 1) + 7) >> 3) * size;

    // Mult_Byte expects [element][share] format for size*n_uints elements
    uint8_t **a = new uint8_t *[originial_num];
    uint8_t **b = new uint8_t *[originial_num];
    uint8_t **u = new uint8_t *[originial_num];

    for (i = 0; i < originial_num; i++) {
        a[i] = new uint8_t[numShares];
        memset(a[i], 0, sizeof(uint8_t) * numShares);
        b[i] = new uint8_t[numShares];
        memset(b[i], 0, sizeof(uint8_t) * numShares);
        u[i] = new uint8_t[numShares];
        memset(u[i], 0, sizeof(uint8_t) * numShares);
    }
    T temp1, temp2, msk;

    while (r_size > 1) {

        new_r_size = r_size >> 1;

        n_uints = ((new_r_size + 7) >> 3);
        num = ((new_r_size + 7) >> 3) * size;

        msk = (T(1) << new_r_size) - T(1);

        // Pack data: a[i*n_uints + byte][s] and b[i*n_uints + byte][s]
        for (i = 0; i < size; ++i) {
            for (s = 0; s < numShares; s++) {
                temp1 = r[i][s] & msk;
                temp2 = (r[i][s] & (msk << new_r_size)) >> new_r_size;

                for (uint k = 0; k < n_uints; k++) {
                    a[i * n_uints + k][s] = ((uint8_t*)&temp1)[k];
                    b[i * n_uints + k][s] = ((uint8_t*)&temp2)[k];
                }
            }
        }

        Mult_Byte(u, a, b, num, nodeNet, ss);

        // Compute OR: u = a ^ b ^ u
        for (i = 0; i < num; ++i) {
            for (s = 0; s < numShares; s++) {
                u[i][s] = a[i][s] ^ b[i][s] ^ u[i][s];
            }
        }

        // Unpack results
        for (i = 0; i < size; ++i) {
            for (s = 0; s < numShares; s++) {
                temp_res[i][s] = 0;
                for (uint k = 0; k < n_uints; k++) {
                    temp_res[i][s] |= ((T)u[i * n_uints + k][s]) << (8 * k);
                }
            }
        }

        // Handle odd bit
        if ((r_size & 1)) {
            for (i = 0; i < size; ++i) {
                for (s = 0; s < numShares; s++) {
                    temp_res[i][s] = SET_BIT(temp_res[i][s], T(new_r_size), GET_BIT(r[i][s], T(r_size - 1)));
                }
            }
            new_r_size += 1;
        }

        for (i = 0; i < size; ++i) {
            for (s = 0; s < numShares; s++) {
                r[i][s] = temp_res[i][s] & ss->SHIFT[new_r_size];
            }
        }

        r_size = new_r_size;
    }

    for (i = 0; i < size; ++i) {
        for (s = 0; s < numShares; s++) {
            res[i][s] = r[i][s];
        }
    }

    for (i = 0; i < originial_num; i++) {
        delete[] a[i];
        delete[] b[i];
        delete[] u[i];
    }
    for (i = 0; i < size; i++) {
        delete[] temp_res[i];
    }

    delete[] a;
    delete[] b;
    delete[] u;
    delete[] temp_res;
}

// Compare two private values: a == b
// Uses [element][share] format: data[i][s] = share s of element i
template <typename T>
void doOperation_EQZ(T **result, T **a, T **b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // Compute diff = a - b, then check if diff == 0
    uint numShares = ss->getNumShares();

    T **diff = new T *[size];
    for (int i = 0; i < size; i++) {
        diff[i] = new T[numShares];
        for (uint s = 0; s < numShares; s++) {
            diff[i][s] = a[i][s] - b[i][s];
        }
    }

    // Call core EQZ to check if diff == 0
    doOperation_EQZ(diff, result, alen, size, threadID, nodeNet, ss);

    for (int i = 0; i < size; i++) {
        delete[] diff[i];
    }
    delete[] diff;
}
// Compare private with public: a == b (public)
// Uses [element][share] format: data[i][s] = share s of element i
template <typename T>
void doOperation_EQZ(T **result, T **a, int *b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // Compute diff = a - b (public), then check if diff == 0
    uint numShares = ss->getNumShares();

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, -1);

    T **diff = new T *[size];
    for (int i = 0; i < size; i++) {
        diff[i] = new T[numShares];
        for (uint s = 0; s < numShares; s++) {
            diff[i][s] = a[i][s] - (T(b[i]) & ai[s]);
        }
    }

    // Call core EQZ to check if diff == 0
    doOperation_EQZ(diff, result, alen, size, threadID, nodeNet, ss);

    for (int i = 0; i < size; i++) {
        delete[] diff[i];
    }
    delete[] diff;
    delete[] ai;
}
template <typename T>
void doOperation_EQZ_bit(T **result, T **a, T **b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_EQZ_bit(T **result, T **a, int *b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
}

// Scalar versions (T*) for RSS - wrap scalars in arrays and call the batch version
template <typename T>
void doOperation_EQZ(T *result, T *a, T *b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // Wrap scalar pointers in array format for the batch function
    T **a_arr = new T*[size];
    T **b_arr = new T*[size];
    T **result_arr = new T*[size];
    for (int i = 0; i < size; i++) {
        a_arr[i] = a + i * ss->getNumShares();
        b_arr[i] = b + i * ss->getNumShares();
        result_arr[i] = result + i * ss->getNumShares();
    }

    // Call the batch version
    doOperation_EQZ(result_arr, a_arr, b_arr, alen, blen, resultlen, size, threadID, nodeNet, ss);

    delete[] a_arr;
    delete[] b_arr;
    delete[] result_arr;
}
template <typename T>
void doOperation_EQZ(T *result, T *a, int *b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // Wrap scalar pointers in array format for the batch function
    T **a_arr = new T*[size];
    T **result_arr = new T*[size];
    for (int i = 0; i < size; i++) {
        a_arr[i] = a + i * ss->getNumShares();
        result_arr[i] = result + i * ss->getNumShares();
    }

    // Call the batch version
    doOperation_EQZ(result_arr, a_arr, b, alen, blen, resultlen, size, threadID, nodeNet, ss);

    delete[] a_arr;
    delete[] result_arr;
}
template <typename T>
void doOperation_EQZ_bit(T *result, T *a, T *b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // Stub - not implemented
}
template <typename T>
void doOperation_EQZ_bit(T *result, T *a, int *b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // Stub - not implemented
}

#endif // _EQZ_HPP_
