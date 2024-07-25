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

#ifndef _INTDIV_HPP_
#define _INTDIV_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "AppRcr.hpp"

template <typename T>
void doOperation_IntDiv_Pub(T *result, T *a, int b, int bitlength, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // will not be implemeneted
}

template <typename T>
void doOperation_IntDiv_Pub(T **result, T **a, int *b, int bitlength, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // main protocol
}

template <typename T>
void doOperation_IntDiv(T *result, T *a, T *b, int bitlength, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // will not be implemeneted
}

template <typename T>
void doOperation_IntDiv(T **result, T **a, T **b, int bitlength, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // main protocol
    // bitlength is computed by the compiler as the max of (alen, blen) ?
    int theta = ceil(log2(double(bitlength) / 3.5));
    int lambda = 8; // lambda adjusts the resolution of y in the algorithm, which reduces the error in the algorithm
    // compute lambda dynamically? it cannot be larger than bitlength
    // this may be what is referrred to as "incresaing the resolution of y" that is stated in the paper
    T alpha = (1 << bitlength);
    uint ring_size = ss->ring_size;

    static uint numShares = ss->getNumShares();

    T **A_buff = new T *[numShares];
    T **B_buff = new T *[numShares];
    T **C_buff = new T *[numShares];
    T **atmp = new T *[numShares];
    T **btmp = new T *[numShares];
    T **sign = new T *[numShares];
    for (size_t s = 0; s < numShares; s++) {
        A_buff[s] = new T[3 * size];
        B_buff[s] = new T[3 * size];
        C_buff[s] = new T[3 * size];

        atmp[s] = new T[size];
        memset(atmp[s], 0, sizeof(T) * size);
        btmp[s] = new T[size];
        memset(btmp[s], 0, sizeof(T) * size);
        sign[s] = new T[size];
        memset(sign[s], 0, sizeof(T) * size);
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, T(1));

    // Need to perform absolute value of a,b here if the protocol has issues with negative valus
    // compute the final sign of the result by XORing the MSBs of a and b
    // and take the absolute value of a and b
    // note, this will elimiate the need to copmpute the absolute value
    // (and subsequent re-application of the sign) inside of Norm

    for (size_t s = 0; s < numShares; s++) {
        memcpy(B_buff[s], a[s], sizeof(T) * size);
        memcpy(B_buff[s] + size, b[s], sizeof(T) * size);
    }

    Rss_MSB(A_buff, B_buff, 2 * size, ring_size, net, ss);

    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            A_buff[s][i] = ai[s] * 1 - 2 * A_buff[s][i];
            A_buff[s][i + size] = ai[s] * 1 - 2 * A_buff[s][i + size];
        }

        memcpy(A_buff[s] + 2 * size, A_buff[s], sizeof(T) * size);
        // B_buff already contains a and b, which was used for the MSB calculation
        memcpy(B_buff[s] + 2 * size, A_buff[s] + size, sizeof(T) * size);
    }

    Mult(C_buff, A_buff, B_buff, 3 * size, threadID, net, ss);

    for (size_t s = 0; s < numShares; s++) {
        memcpy(atmp[s], C_buff[s], sizeof(T) * size);
        memcpy(btmp[s], C_buff[s] + size, sizeof(T) * size);
        memcpy(sign[s], C_buff[s] + 2 * size, sizeof(T) * size);
    }

    doOperation_IntAppRcr(B_buff, btmp, bitlength, size, ring_size, threadID, net, ss); // B_buff contains w

    // computing y = atmp*w and x = btmp*w in parallel, in this order
    for (size_t s = 0; s < numShares; s++) {
        memcpy(A_buff[s], atmp[s], sizeof(T) * size);
        memcpy(A_buff[s] + size, btmp[s], sizeof(T) * size);   // atmp, btmp
        memcpy(B_buff[s] + size, B_buff[s], sizeof(T) * size); // w, w

        // clearing destination
        memset(C_buff[s], 0, sizeof(T) * 3 * size);
    }
    Mult(C_buff, A_buff, B_buff, 2 * size, threadID, net, ss);

    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            C_buff[s][i + size] = (ai[s] * alpha) - C_buff[s][i + size];
        }
    }

    // both need to be truncated by ell bits
    // doOperation_Trunc(C_buff, C_buff, bitlength, bitlength , 2 * size, threadID, net, ss);
    doOperation_Trunc(C_buff, C_buff, bitlength, bitlength - lambda, size, threadID, net, ss);

    for (int th = 0; th < theta - 1; th++) {

        for (size_t s = 0; s < numShares; s++) {

            memcpy(A_buff[s], C_buff[s], sizeof(T) * size);               // y
            memcpy(A_buff[s] + size, C_buff[s] + size, sizeof(T) * size); // x
            memcpy(B_buff[s] + size, C_buff[s] + size, sizeof(T) * size); // x

            for (size_t s = 0; s < numShares; s++) {
                for (size_t i = 0; i < size; i++) {
                    B_buff[s][i] = (ai[s] * alpha) + C_buff[s][i + size]; // alpha + x
                }
            }
        }
        for (size_t s = 0; s < numShares; s++) {
            memset(C_buff[s], 0, sizeof(T) * 2 * size); // sanitizing after we already inserted everything into the respective buffers
        }
        // computing y*(alpha + x) and  x*x, in this order
        Mult(C_buff, A_buff, B_buff, 2 * size, threadID, net, ss);

        doOperation_Trunc(C_buff, C_buff, bitlength, bitlength, 2 * size, threadID, net, ss);
    }

    for (size_t s = 0; s < numShares; s++) {
        memcpy(A_buff[s], C_buff[s], sizeof(T) * size);

        for (size_t i = 0; i < size; i++) {
            B_buff[s][i] = (ai[s] * alpha) + C_buff[s][i + size]; // alpha + x
        }

        memset(C_buff[s], 0, sizeof(T) * size);
    }

    Mult(C_buff, B_buff, A_buff, size, threadID, net, ss);

    doOperation_Trunc(A_buff, C_buff, bitlength, bitlength + lambda, size, threadID, net, ss);

    // correction (?) that is present in shamir
    for (size_t s = 0; s < numShares; s++) {
        memset(C_buff[s], 0, sizeof(T) * size);
    }
    Mult(C_buff, A_buff, btmp, size, threadID, net, ss);

    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            C_buff[s][i] = atmp[s][i] - C_buff[s][i];
        }
    }

    Rss_MSB(C_buff, C_buff, size, ring_size, net, ss);

    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            A_buff[s][i] = A_buff[s][i] + ai[s] * 1 - 2 * C_buff[s][i];
        }
        memset(C_buff[s], 0, sizeof(T) * size);
    }

    Mult(C_buff, A_buff, btmp, size, threadID, net, ss);

    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            C_buff[s][i] = atmp[s][i] - C_buff[s][i];
        }
    }

    Rss_MSB(C_buff, C_buff, size, ring_size, net, ss);

    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            A_buff[s][i] = A_buff[s][i] - C_buff[s][i];
        }
        memset(result[s], 0, sizeof(T) * size);
    }

    Mult(result, sign, A_buff, size, threadID, net, ss);

    // cleanup
    for (size_t i = 0; i < numShares; i++) {
        delete[] atmp[i];
        delete[] btmp[i];
        delete[] sign[i];

        delete[] A_buff[i];
        delete[] B_buff[i];
        delete[] C_buff[i];
    }
    delete[] atmp;
    delete[] btmp;
    delete[] sign;

    delete[] A_buff;
    delete[] B_buff;
    delete[] C_buff;

    delete[] ai;
}

template <typename T>
void doOperation_IntDiv(T *result, int a, T *b, int bitlength, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // division by a private divisor
    // single division of a public value by a private divisior (will not be implemented)
    // will not be implemeneted
}

template <typename T>
void doOperation_IntDiv(T **result, int *a, T **b, int bitlength, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // division by a private divisor
    // will call main intDiv
    // Need to call sparsify
}

#endif // _INTDIV_HPP_