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
    int theta = ceil(log2(double(bitlength) / 3.5));
    int lambda = 8; //??? what is this from???
    // compute lambda dynamically? it cannot be larger than bitlength
    // this may be what is referrred to as "incresaing the resolution of y" that is stated in the paper
    T alpha = (1 << bitlength);
    uint ring_size = ss->ring_size;

    static uint numShares = ss->getNumShares();

    T *res_check = new T[3 * size];

    T **A_buff = new T *[numShares];
    T **B_buff = new T *[numShares];
    T **C_buff = new T *[numShares];
    for (size_t s = 0; s < numShares; s++) {
        A_buff[s] = new T[3 * size];
        B_buff[s] = new T[3 * size];
        C_buff[s] = new T[3 * size];
    }

    T **atmp = new T *[numShares];
    T **btmp = new T *[numShares];
    T **x = new T *[numShares];
    T **y = new T *[numShares];
    T **w = new T *[numShares];
    T **c = new T *[numShares];
    T **lt = new T *[numShares];
    T **temp0 = new T *[numShares];
    T **sign = new T *[numShares];
    for (size_t i = 0; i < numShares; i++) {
        lt[i] = new T[2 * size];
        memset(lt[i], 0, sizeof(T) * 2 * size);
        atmp[i] = new T[size];
        memset(atmp[i], 0, sizeof(T) * size);
        btmp[i] = new T[size];
        memset(btmp[i], 0, sizeof(T) * size);
        x[i] = new T[size];
        memset(x[i], 0, sizeof(T) * size);
        y[i] = new T[size];
        memset(y[i], 0, sizeof(T) * size);
        temp0[i] = new T[size];
        memset(temp0[i], 0, sizeof(T) * size);
        c[i] = new T[size];
        memset(c[i], 0, sizeof(T) * size);
        w[i] = new T[size];
        memset(w[i], 0, sizeof(T) * size);
        sign[i] = new T[size];
        memset(sign[i], 0, sizeof(T) * size);
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
    }

    for (size_t s = 0; s < numShares; s++) {
        memcpy(A_buff[s] + 2 * size, A_buff[s], sizeof(T) * size);
        // B_buff already contains a and b, which was used for the MSB calculation
        // memcpy(B_buff[s], a[s], sizeof(T) * size);
        // memcpy(B_buff[s] + size, b[s], sizeof(T) * size);
        memcpy(B_buff[s] + 2 * size, A_buff[s] + size, sizeof(T) * size);
    }

    Mult(C_buff, A_buff, B_buff, 3 * size, threadID, net, ss);

    for (size_t s = 0; s < numShares; s++) {
        memcpy(atmp[s], C_buff[s], sizeof(T) * size);
        memcpy(btmp[s], C_buff[s] + size, sizeof(T) * size);
        memcpy(sign[s], C_buff[s] + 2 * size, sizeof(T) * size);
    }

    /*     Mult(c, a, temp0, size, threadID, net, ss);
        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                atmp[s][i] = a[s][i] - 2 * c[s][i];
            }
        }
        for (size_t s = 0; s < numShares; s++) {
            memcpy(temp0[s], temp0[s] + size, sizeof(T) * size);
        }
        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                temp2[s][i] = ai[s] * 1 - 2 * temp0[s][i];
            }
        }
        Mult(sign, temp3, temp2, size, threadID, net, ss);
        Mult(temp4, b, temp0, size, threadID, net, ss);
        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                btmp[s][i] = b[s][i] - 2 * temp4[s][i];
                // temp1[s][i] = b[s][i] - temp4[s][i];
                // btmp[s][i] = temp1[s][i] - temp4[s][i];
            }
        }
     */
    doOperation_IntAppRcr(w, btmp, bitlength, size, ring_size, threadID, net, ss);

    // computing y = atmp*w and x = btmp*w in parallel, in this order
    for (size_t s = 0; s < numShares; s++) {
        memcpy(A_buff[s], atmp[s], sizeof(T) * size);
        memcpy(A_buff[s] + size, btmp[s], sizeof(T) * size);
        memcpy(B_buff[s], w[s], sizeof(T) * size);
        memcpy(B_buff[s] + size, w[s], sizeof(T) * size);

        // clearing destination
        memset(C_buff[s], 0, sizeof(T) * 3 * size);
    }
    Mult(C_buff, A_buff, B_buff, 2 * size, threadID, net, ss);

    // performing x = (b * w) and y = (a * w) in a batch, in that order
    // Mult(y, atmp, w, size, threadID, net, ss);
    // Mult(x, btmp, w, size, threadID, net, ss);

    Open(res_check, atmp, size, threadID, net, ss);
    for (size_t i = 0; i < size; i++) {
        printf("[atmp]   [%lu]: %u\t", i, res_check[i]);
        print_binary(res_check[i], 2 * bitlength);
    }
    printf("\n");
    Open(res_check, btmp, size, threadID, net, ss);
    for (size_t i = 0; i < size; i++) {
        printf("[btmp]   [%lu]: %u\t", i, res_check[i]);
        print_binary(res_check[i], 2 * bitlength);
    }
    printf("\n");
    Open(res_check, sign, size, threadID, net, ss);
    for (size_t i = 0; i < size; i++) {
        printf("[sign]   [%lu]: %u\t", i, res_check[i]);
        print_binary(res_check[i], 2 * bitlength);
    }
    // fine up to here
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

        // for (size_t s = 0; s < numShares; s++) {
        //     for (size_t i = 0; i < size; i++) {
        //         temp1[s][i] = (ai[s] * alpha) + x[s][i]; // alpha + x
        //     }
        // }

        // Mult(temp2, y, temp1, size, threadID, net, ss);
        // for (size_t s = 0; s < numShares; s++) {
        //     memset(temp1[s], 0, sizeof(T) * size);
        // }
        // Mult(temp1, x, x, size, threadID, net, ss);

        // doOperation_Trunc(x, temp1, bitlength, bitlength, size, threadID, net, ss);
        // doOperation_Trunc(y, temp2, bitlength, bitlength, size, threadID, net, ss);
    }
    for (size_t s = 0; s < numShares; s++) {
        memcpy(y[s], C_buff[s], sizeof(T) * size);
    }

    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            x[s][i] = (ai[s] * alpha) + C_buff[s][i + size]; // alpha + x
        }
    }
    for (size_t s = 0; s < numShares; s++) {
        memset(C_buff[s], 0, sizeof(T)  * size);
    }
    Mult(C_buff, x, y, size, threadID, net, ss);
    doOperation_Trunc(result, C_buff, bitlength, bitlength + lambda, size, threadID, net, ss);

    // for (size_t s = 0; s < numShares; s++) {
    //     for (size_t i = 0; i < size; i++) {
    //         x[s][i] = (ai[s] * alpha) + x[s][i]; // alpha + x
    //     }
    // }
    // Mult(temp1, x, y, size, threadID, net, ss);
    // doOperation_Trunc(result, temp1, bitlength, bitlength + lambda, size, threadID, net, ss);

    // correction (?) that is present in shamir
    for (size_t s = 0; s < numShares; s++) {
        memcpy(c[s], result[s], sizeof(T) * size);
    }
    for (size_t s = 0; s < numShares; s++) {
        memset(temp0[s], 0, sizeof(T) * size);
        // memset(temp2[s], 0, sizeof(T) * size);
        // memset(temp3[s], 0, sizeof(T) * size);
        // memset(temp4[s], 0, sizeof(T) * size);
    }
    Mult(temp0, c, btmp, size, threadID, net, ss);

    // Open(res_check, temp0, size, threadID, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     printf("[mult1]   [%lu]: %u\t", i, res_check[i]);
    //     print_binary(res_check[i], ring_size);
    // }

    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            temp0[s][i] = atmp[s][i] - temp0[s][i];
        }
    }

    // Open(res_check, temp2, size, threadID, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     printf("[a - mult]   [%lu]: %u\t", i, res_check[i]);
    //     print_binary(res_check[i], ring_size);
    // }

    Rss_MSB(temp0, temp0, size, ring_size, net, ss);

    // Open(res_check, temp3, size, threadID, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     printf("[msb of prior]   [%lu]: %u\t", i, res_check[i]);
    //     print_binary(res_check[i], ring_size);
    // }

    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            // temp3[s][i] = ai[s] * 1 - 2 * temp3[s][i];
            c[s][i] = c[s][i] + ai[s] * 1 - 2 * temp0[s][i];
        }
    }
    for (size_t s = 0; s < numShares; s++) {
        memset(temp0[s], 0, sizeof(T) * size);
        // memset(temp2[s], 0, sizeof(T) * size);
        // memset(temp3[s], 0, sizeof(T) * size);
        // memset(temp4[s], 0, sizeof(T) * size);
    }

    // Open(res_check, c, size, threadID, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     printf("[mid adjust]   [%lu]: %u\t", i, res_check[i]);
    //     print_binary(res_check[i], ring_size);
    // }

    Mult(temp0, c, btmp, size, threadID, net, ss);

    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            temp0[s][i] = atmp[s][i] - temp0[s][i];
        }
    }

    // Rss_MSB(temp3, temp2, size, ring_size, net, ss);
    Rss_MSB(temp0, temp0, size, ring_size, net, ss);

    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            c[s][i] = c[s][i] - temp0[s][i];
        }
    }

    for (size_t s = 0; s < numShares; s++) {
        memset(result[s], 0, sizeof(T) * size);
    }
    Mult(result, sign, c, size, threadID, net, ss);

    // Open(res_check, temp4, size, threadID, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     printf("[post sign mult]   [%lu]: %u\t", i, res_check[i]);
    //     print_binary(res_check[i], ring_size);
    // }

    // for (size_t s = 0; s < numShares; s++) {
    //     memcpy(result[s], temp1[s], sizeof(T) * size);
    // }

    /*     for (int th = 0; th < theta - 1; th++) {
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
     */

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