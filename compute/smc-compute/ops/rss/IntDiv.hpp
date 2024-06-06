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
}

template <typename T>
void doOperation_IntDiv_Pub(T **result, T **a, int *b, int bitlength, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
}

template <typename T>
void doOperation_IntDiv_Pub(T **result, T **a, T **b, int bitlength, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
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
    double t = bitlength / 3.5;
    int theta = ceil(log2(t));
    // int lambda = 6; //??? what is this from???
    T alpha = (1 << bitlength);
    std::cout << "alpha  = " << alpha <<std::endl;
    std::cout << "theta  = " << theta <<std::endl;
    std::cout << "bitlen = " << bitlength <<std::endl;

    uint ring_size = ss->ring_size;
    std::cout << "ring   = " << ring_size <<std::endl;

    static uint numShares = ss->getNumShares();

    T **x = new T *[numShares];
    T **y = new T *[numShares];
    T **w = new T *[numShares];
    for (size_t i = 0; i < numShares; i++) {
        x[i] = new T[size];
        memset(x[i], 0, sizeof(T) * size);
        y[i] = new T[size];
        memset(y[i], 0, sizeof(T) * size);
        w[i] = new T[size];
        memset(w[i], 0, sizeof(T) * size);
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, T(1));

    // Need to perform absolute value of a,b here if the protocol has issues with negative valus
    // compute the final sign of the result by XORing the MSBs of a and b
    // and take the absolute value of a and b
    // note, this will elimiate the need to copmpute the absolute value
    // (and subsequent re-application of the sign) inside of Norm

    /*

    (insert absolute value computaiton here)

    */
    T *res_check = new T[2*size];

    doOperation_IntAppRcr(w, b, bitlength, size, ring_size, threadID, net, ss);

    Open(res_check, w, size, threadID, net, ss);
    for (size_t i = 0; i < size; i++) {
        printf("[w]   [%lu]: %u\n", i, res_check[i]);
        print_binary(res_check[i], ring_size);
    }


    T **A_buff = new T *[numShares];
    T **B_buff = new T *[numShares];
    T **C_buff = new T *[numShares];
    for (size_t s = 0; s < numShares; s++) {
        A_buff[s] = new T[2 * size];
        B_buff[s] = new T[2 * size];
        C_buff[s] = new T[2 * size];

        memcpy(A_buff[s], b[s], sizeof(T) * size);
        memcpy(A_buff[s] + size, a[s], sizeof(T) * size);

        memcpy(B_buff[s], w[s], sizeof(T) * size);
        memcpy(B_buff[s] + size, w[s], sizeof(T) * size);

        memset(C_buff[s], 0, sizeof(T) * 2 * size); // sanitizing
    }

    // performing x = (b * w) and y = (a * w) in a batch, in that order
    Mult(C_buff, A_buff, B_buff, 2 * size, threadID, net, ss);

    Open(res_check, C_buff, 2*size, threadID, net, ss);
    for (size_t i = 0; i < size; i++) {
        printf("[b * w]   [%lu]: %u\n", i, res_check[i]);
        print_binary(res_check[i], 2*bitlength);
    }

    for (size_t i = size; i < 2*size; i++) {
        printf("[a * w]   [%lu]: %u\n", i, res_check[i]);
        print_binary(res_check[i], 2*bitlength);
    }
    // both need to be truncated by ell bits
    doOperation_Trunc(C_buff, C_buff, bitlength, bitlength,  size, threadID, net, ss);

    // extracting  trunc(a*w) into y
    // NOT NEEDED, SINCE WE ARE USING C_buff DIRECTLY
    // for (size_t s = 0; s < numShares; s++) {
    //     memcpy(y[s], C_buff[s] + size, sizeof(T) * size);
    // }

    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            C_buff[s][i] = (ai[s] * alpha) - C_buff[s][i]; // alpha - trunc([b][w])
        }
    }

    for (int th = 0; th < theta - 1; th++) {
        for (size_t s = 0; s < numShares; s++) {

            memcpy(A_buff[s], C_buff[s], sizeof(T) * size);               // x
            memcpy(A_buff[s] + size, C_buff[s] + size, sizeof(T) * size); // y
            memcpy(B_buff[s], C_buff[s], sizeof(T) * size);               // x

            for (size_t s = 0; s < numShares; s++) {
                for (size_t i = 0; i < size; i++) {
                    B_buff[s][i + size] = (ai[s] * alpha) + C_buff[s][i]; // alpha + x
                }
            }
        }
        for (size_t s = 0; s < numShares; s++) {
            memset(C_buff[s], 0, sizeof(T) * 2 * size); // sanitizing after we already inserted everything into the respective buffers
        }
        // computing x*x and y*(alpha + x), in this order
        Mult(C_buff, A_buff, B_buff, 2 * size, threadID, net, ss);
        doOperation_Trunc(C_buff, C_buff, bitlength, bitlength, 2 * size, threadID, net, ss);
    }

    for (size_t s = 0; s < numShares; s++) {
        memcpy(y[s], C_buff[s] + size, sizeof(T) * size);
    }

    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            x[s][i] = (ai[s] * alpha) + C_buff[s][i]; // alpha + x
        }
    }

    Mult(C_buff, x, y, size, threadID, net, ss);
    doOperation_Trunc(result, C_buff, bitlength, bitlength, size, threadID, net, ss);

    // cleanup
    for (size_t i = 0; i < numShares; i++) {
        delete[] x[i];
        delete[] y[i];
        delete[] w[i];

        delete[] A_buff[i];
        delete[] B_buff[i];
        delete[] C_buff[i];
    }
    delete[] x;
    delete[] y;
    delete[] w;

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