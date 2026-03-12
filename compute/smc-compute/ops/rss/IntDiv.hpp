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
#include "Open.hpp"

// Single-element wrapper: private a / public b
template <typename T>
void doOperation_IntDiv_Pub(T *result, T *a, int b, int bitlength, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **result_2d = new T *[1];
    T **a_2d = new T *[1];
    result_2d[0] = new T[numShares];
    a_2d[0] = new T[numShares];
    int b_arr[1] = {b};

    for (uint s = 0; s < numShares; s++) {
        a_2d[0][s] = a[s];
        result_2d[0][s] = 0;
    }

    doOperation_IntDiv_Pub(result_2d, a_2d, b_arr, bitlength, 1, threadID, net, ss);

    for (uint s = 0; s < numShares; s++)
        result[s] = result_2d[0][s];

    delete[] result_2d[0];
    delete[] a_2d[0];
    delete[] result_2d;
    delete[] a_2d;
}

// Single-element wrapper with int* b (called from SMC_Utils with size=1)
template <typename T>
void doOperation_IntDiv_Pub(T *result, T *a, int *b, int bitlength, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    doOperation_IntDiv_Pub(result, a, b[0], bitlength, threadID, net, ss);
}

// Main batch protocol: private a[i] / public b[i]
// Algorithm: compute sign of a, take |a|, multiply by floor(2^(k+lambda)/|b|),
// truncate, then apply correction step.
template <typename T>
void doOperation_IntDiv_Pub(T **result, T **a, int *b, int bitlength, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    int lambda = DIVISION_ACCURACY;
    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, T(1));

    // Allocate working buffers [size][numShares]
    T **lt = new T *[size];
    T **sign_a = new T *[size];
    T **a_tmp = new T *[size];
    T **temp = new T *[size];
    T **c = new T *[size];
    int *b_abs = new int[size];

    for (int i = 0; i < size; i++) {
        lt[i] = new T[numShares];
        sign_a[i] = new T[numShares];
        a_tmp[i] = new T[numShares];
        temp[i] = new T[numShares];
        c[i] = new T[numShares];
        memset(lt[i], 0, sizeof(T) * numShares);
        memset(sign_a[i], 0, sizeof(T) * numShares);
        memset(a_tmp[i], 0, sizeof(T) * numShares);
        memset(temp[i], 0, sizeof(T) * numShares);
        memset(c[i], 0, sizeof(T) * numShares);
    }

    // Step 1: Compute sign of a using MSB
    // MSB gives 1 for negative, 0 for positive
    Rss_MSB(lt, a, size, ring_size, net, ss);

    // sign_a = 1 - 2*MSB(a) (gives +1 for positive, -1 for negative)
    for (int i = 0; i < size; i++)
        for (uint s = 0; s < numShares; s++)
            sign_a[i][s] = ai[s] - 2 * lt[i][s];

    // Step 2: Compute |a| = sign_a * a
    Mult(a_tmp, sign_a, a, size, threadID, net, ss);

    // Step 3: Handle sign of public b (locally)
    // Combined sign = sign_a * sign_b
    // We can just flip sign_a if b is negative, since b is public
    for (int i = 0; i < size; i++) {
        if (b[i] < 0) {
            b_abs[i] = -b[i];
            // Negate sign_a for this element
            for (uint s = 0; s < numShares; s++)
                sign_a[i][s] = -sign_a[i][s];
        } else {
            b_abs[i] = b[i];
        }
    }

    // Step 4: Multiply by public reciprocal
    // temp[i] = floor(2^(k+lambda) / |b[i]|) * |a[i]|
    // This is a public-private multiplication
    for (int i = 0; i < size; i++) {
        // Compute floor(2^(k+lambda) / |b[i]|) as a plain integer
        // Use T(1) to handle large bitlengths
        T recip = (T(1) << (bitlength + lambda)) / T(b_abs[i]);
        for (uint s = 0; s < numShares; s++)
            temp[i][s] = a_tmp[i][s] * recip;
    }

    // Step 5: Truncate by (k + lambda) bits to get approximate quotient
    doOperation_Trunc(c, temp, 2 * bitlength + lambda, bitlength + lambda, size, threadID, net, ss);

    // Step 6: Correction step
    // Compute c * |b| and compare with |a| to correct off-by-one errors
    // temp = c * |b|
    for (int i = 0; i < size; i++)
        for (uint s = 0; s < numShares; s++)
            temp[i][s] = c[i][s] * T(b_abs[i]);

    // temp = |a| - c * |b| (remainder)
    for (int i = 0; i < size; i++)
        for (uint s = 0; s < numShares; s++)
            temp[i][s] = a_tmp[i][s] - temp[i][s];

    // If remainder < 0 (MSB=1), quotient is too large by 1
    // temp = c*b - a - 1; if this < 0, quotient needs +1
    // Actually, we check: remainder - 1 >= 0, meaning remainder >= 1, so we should add 1...
    // Following the Shamir approach more directly:
    // temp = c*|b| - |a| - 1
    for (int i = 0; i < size; i++)
        for (uint s = 0; s < numShares; s++)
            temp[i][s] = -temp[i][s] - ai[s]; // = c*|b| - |a| - 1

    // lt = MSB(temp) = 1 if (c*|b| - |a| - 1) < 0, meaning c*|b| <= |a|, meaning we need +1
    Rss_MSB(lt, temp, size, ring_size, net, ss);

    // c = c + lt (add 1 if remainder was non-negative, i.e., quotient was too small)
    for (int i = 0; i < size; i++)
        for (uint s = 0; s < numShares; s++)
            c[i][s] = c[i][s] + lt[i][s];

    // Second correction: check if (c+1)*|b| > |a|+1, i.e., we overshot
    for (int i = 0; i < size; i++)
        for (uint s = 0; s < numShares; s++)
            temp[i][s] = c[i][s] * T(b_abs[i]) - a_tmp[i][s] - ai[s];

    Rss_MSB(lt, temp, size, ring_size, net, ss);

    // If temp >= 0 (MSB=0), we overshot, subtract 1
    // result_unsigned = c - (1 - MSB) = c - 1 + MSB
    for (int i = 0; i < size; i++)
        for (uint s = 0; s < numShares; s++)
            c[i][s] = c[i][s] - ai[s] + lt[i][s];

    // Step 7: Apply sign
    Mult(result, sign_a, c, size, threadID, net, ss);

    // Cleanup
    for (int i = 0; i < size; i++) {
        delete[] lt[i];
        delete[] sign_a[i];
        delete[] a_tmp[i];
        delete[] temp[i];
        delete[] c[i];
    }
    delete[] lt;
    delete[] sign_a;
    delete[] a_tmp;
    delete[] temp;
    delete[] c;
    delete[] b_abs;
    delete[] ai;
}

template <typename T>
void doOperation_IntDiv(T *result, T *a, T *b, int bitlength, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // Single-element wrapper - converts to batch format and calls the main function
    // Note: size is expected to be 1 for this wrapper
    uint numShares = ss->getNumShares();

    // Create temporary 2D arrays for batch interface [size][numShares] where size=1
    T **result_2d = new T*[1];
    T **a_2d = new T*[1];
    T **b_2d = new T*[1];
    result_2d[0] = new T[numShares];
    a_2d[0] = new T[numShares];
    b_2d[0] = new T[numShares];

    // Copy input to batch format
    for (uint s = 0; s < numShares; s++) {
        a_2d[0][s] = a[s];
        b_2d[0][s] = b[s];
        result_2d[0][s] = 0;
    }

    // Call batch version with size=1
    doOperation_IntDiv(result_2d, a_2d, b_2d, bitlength, 1, threadID, net, ss);

    // Copy result back
    for (uint s = 0; s < numShares; s++) {
        result[s] = result_2d[0][s];
    }

    // Cleanup
    delete[] result_2d[0];
    delete[] a_2d[0];
    delete[] b_2d[0];
    delete[] result_2d;
    delete[] a_2d;
    delete[] b_2d;
}

// Wrapper: public a / private b
template <typename T>
void doOperation_IntDiv(T *result, int *a, T *b, int bitlength, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **result_2d = new T *[size];
    T **b_2d = new T *[size];
    int *a_arr = new int[size];
    for (int i = 0; i < size; i++) {
        result_2d[i] = new T[numShares];
        b_2d[i] = new T[numShares];
        memset(result_2d[i], 0, sizeof(T) * numShares);
        for (uint s = 0; s < numShares; s++)
            b_2d[i][s] = b[s]; // single element replicated
        a_arr[i] = a[i];
    }
    doOperation_IntDiv(result_2d, a_arr, b_2d, bitlength, size, threadID, net, ss);
    for (uint s = 0; s < numShares; s++)
        result[s] = result_2d[0][s];
    for (int i = 0; i < size; i++) {
        delete[] result_2d[i];
        delete[] b_2d[i];
    }
    delete[] result_2d;
    delete[] b_2d;
    delete[] a_arr;
}

template <typename T>
void doOperation_IntDiv(T **result, T **a, T **b, int bitlength, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // main protocol
    // NOTE: Interface format is [size][numShares] where array[i][s] is share s of element i
    // bitlength is computed by the compiler as the max of (alen, blen) ?
    int theta = ceil(log2(double(bitlength) / 3.5));
    // lambda adjusts the resolution of y in the algorithm, reducing error
    // defined in common/shared.h as DIVISION_ACCURACY
    int lambda = DIVISION_ACCURACY;
    // Use T(1) to avoid 32-bit overflow when bitlength >= 32
    T alpha = (T(1) << bitlength);
    uint ring_size = ss->ring_size;

    uint numShares = ss->getNumShares();

    // All buffers use interface format [size][numShares]
    T **A_buff = new T *[3 * size];
    T **B_buff = new T *[3 * size];
    T **C_buff = new T *[3 * size];
    T **atmp = new T *[size];
    T **btmp = new T *[size];
    T **sign = new T *[size];
    for (int i = 0; i < 3 * size; i++) {
        A_buff[i] = new T[numShares];
        B_buff[i] = new T[numShares];
        C_buff[i] = new T[numShares];
        memset(A_buff[i], 0, sizeof(T) * numShares);
        memset(B_buff[i], 0, sizeof(T) * numShares);
        memset(C_buff[i], 0, sizeof(T) * numShares);
    }
    for (int i = 0; i < size; i++) {
        atmp[i] = new T[numShares];
        memset(atmp[i], 0, sizeof(T) * numShares);
        btmp[i] = new T[numShares];
        memset(btmp[i], 0, sizeof(T) * numShares);
        sign[i] = new T[numShares];
        memset(sign[i], 0, sizeof(T) * numShares);
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, T(1));

    // Need to perform absolute value of a,b here if the protocol has issues with negative valus
    // compute the final sign of the result by XORing the MSBs of a and b
    // and take the absolute value of a and b
    // note, this will elimiate the need to copmpute the absolute value
    // (and subsequent re-application of the sign) inside of Norm

    // Copy a and b into B_buff (interface format [size][numShares])
    // B_buff[0..size-1] = a, B_buff[size..2*size-1] = b
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            B_buff[i][s] = a[i][s];
            B_buff[i + size][s] = b[i][s];
        }
    }

    Rss_MSB(A_buff, B_buff, 2 * size, ring_size, net, ss);

    // Compute sign = 1 - 2*MSB for a and b
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            A_buff[i][s] = ai[s] * 1 - 2 * A_buff[i][s];           // sign of a
            A_buff[i + size][s] = ai[s] * 1 - 2 * A_buff[i + size][s]; // sign of b
        }
    }

    // Copy for multiplication: A_buff[2*size..] = sign_a, B_buff[2*size..] = sign_b
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            A_buff[i + 2 * size][s] = A_buff[i][s];      // sign_a
            B_buff[i + 2 * size][s] = A_buff[i + size][s]; // sign_b
        }
    }

    // Mult computes: C_buff[0..size-1] = sign_a * a = |a|
    //                C_buff[size..2*size-1] = sign_b * b = |b|
    //                C_buff[2*size..3*size-1] = sign_a * sign_b = final sign
    Mult(C_buff, A_buff, B_buff, 3 * size, threadID, net, ss);

    // Copy results to atmp, btmp, sign
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            atmp[i][s] = C_buff[i][s];           // |a|
            btmp[i][s] = C_buff[i + size][s];    // |b|
            sign[i][s] = C_buff[i + 2 * size][s]; // final sign
        }
    }

    doOperation_IntAppRcr(B_buff, btmp, bitlength, size, ring_size, threadID, net, ss); // B_buff contains w

    // computing y = atmp*w and x = btmp*w in parallel, in this order
    // A_buff[0..size-1] = atmp, A_buff[size..2*size-1] = btmp
    // B_buff[0..size-1] = w (from AppRcr), B_buff[size..2*size-1] = w (copy)
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            A_buff[i][s] = atmp[i][s];
            A_buff[i + size][s] = btmp[i][s];
            B_buff[i + size][s] = B_buff[i][s]; // copy w
        }
    }

    // Clear C_buff before multiplication
    for (int i = 0; i < 3 * size; i++) {
        memset(C_buff[i], 0, sizeof(T) * numShares);
    }

    // C_buff[0..size-1] = atmp * w = y
    // C_buff[size..2*size-1] = btmp * w = x
    Mult(C_buff, A_buff, B_buff, 2 * size, threadID, net, ss);

    // Compute alpha - x (stored in C_buff[size..2*size-1])
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            C_buff[i + size][s] = (ai[s] * alpha) - C_buff[i + size][s];
        }
    }

    // both need to be truncated by ell bits
    // doOperation_Trunc(C_buff, C_buff, bitlength, bitlength , 2 * size, threadID, net, ss);
    doOperation_Trunc(C_buff, C_buff, bitlength, bitlength - lambda, size, threadID, net, ss);

    for (int th = 0; th < theta - 1; th++) {
        // Newton iteration: y_new = y * (alpha + x), x_new = x * x
        // A_buff[0..size-1] = y, A_buff[size..2*size-1] = x
        // B_buff[0..size-1] = alpha + x, B_buff[size..2*size-1] = x
        for (int i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                A_buff[i][s] = C_buff[i][s];           // y
                A_buff[i + size][s] = C_buff[i + size][s]; // x
                B_buff[i][s] = (ai[s] * alpha) + C_buff[i + size][s]; // alpha + x
                B_buff[i + size][s] = C_buff[i + size][s]; // x
            }
        }

        // Clear C_buff before multiplication
        for (int i = 0; i < 2 * size; i++) {
            memset(C_buff[i], 0, sizeof(T) * numShares);
        }

        // computing y*(alpha + x) and x*x, in this order
        Mult(C_buff, A_buff, B_buff, 2 * size, threadID, net, ss);

        doOperation_Trunc(C_buff, C_buff, bitlength, bitlength, 2 * size, threadID, net, ss);
    }

    // Final Newton iteration: result = y * (alpha + x)
    // A_buff[0..size-1] = y, B_buff[0..size-1] = alpha + x
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            A_buff[i][s] = C_buff[i][s]; // y
            B_buff[i][s] = (ai[s] * alpha) + C_buff[i + size][s]; // alpha + x
        }
    }

    // Clear C_buff before multiplication
    for (int i = 0; i < size; i++) {
        memset(C_buff[i], 0, sizeof(T) * numShares);
    }

    Mult(C_buff, B_buff, A_buff, size, threadID, net, ss);

    doOperation_Trunc(A_buff, C_buff, bitlength, bitlength + lambda, size, threadID, net, ss);

    // correction (?) that is present in shamir
    // Clear C_buff before multiplication
    for (int i = 0; i < size; i++) {
        memset(C_buff[i], 0, sizeof(T) * numShares);
    }
    Mult(C_buff, A_buff, btmp, size, threadID, net, ss);

    // C_buff = atmp - A_buff * btmp
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            C_buff[i][s] = atmp[i][s] - C_buff[i][s];
        }
    }

    Rss_MSB(C_buff, C_buff, size, ring_size, net, ss);

    // A_buff = A_buff + 1 - 2*MSB
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            A_buff[i][s] = A_buff[i][s] + ai[s] * 1 - 2 * C_buff[i][s];
        }
    }

    // Clear C_buff before multiplication
    for (int i = 0; i < size; i++) {
        memset(C_buff[i], 0, sizeof(T) * numShares);
    }

    Mult(C_buff, A_buff, btmp, size, threadID, net, ss);

    // C_buff = atmp - A_buff * btmp
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            C_buff[i][s] = atmp[i][s] - C_buff[i][s];
        }
    }

    Rss_MSB(C_buff, C_buff, size, ring_size, net, ss);

    // A_buff = A_buff - MSB
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            A_buff[i][s] = A_buff[i][s] - C_buff[i][s];
        }
    }

    // Clear result before final multiplication
    for (int i = 0; i < size; i++) {
        memset(result[i], 0, sizeof(T) * numShares);
    }

    // result = sign * A_buff
    Mult(result, sign, A_buff, size, threadID, net, ss);

    // cleanup
    for (int i = 0; i < size; i++) {
        delete[] atmp[i];
        delete[] btmp[i];
        delete[] sign[i];
    }
    for (int i = 0; i < 3 * size; i++) {
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
    uint numShares = ss->getNumShares();
    T **result_2d = new T *[1];
    T **a_2d = new T *[1];
    T **b_2d = new T *[1];
    result_2d[0] = new T[numShares];
    a_2d[0] = new T[numShares];
    b_2d[0] = new T[numShares];
    memset(result_2d[0], 0, sizeof(T) * numShares);
    memset(a_2d[0], 0, sizeof(T) * numShares);
    ss->sparsify_public(a_2d[0], T(a));
    for (uint s = 0; s < numShares; s++)
        b_2d[0][s] = b[s];

    doOperation_IntDiv(result_2d, a_2d, b_2d, bitlength, 1, threadID, net, ss);

    for (uint s = 0; s < numShares; s++)
        result[s] = result_2d[0][s];

    delete[] result_2d[0];
    delete[] a_2d[0];
    delete[] b_2d[0];
    delete[] result_2d;
    delete[] a_2d;
    delete[] b_2d;
}

template <typename T>
void doOperation_IntDiv(T **result, int *a, T **b, int bitlength, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **a_2d = new T *[size];
    for (int i = 0; i < size; i++) {
        a_2d[i] = new T[numShares];
        memset(a_2d[i], 0, sizeof(T) * numShares);
        ss->sparsify_public(a_2d[i], T(a[i]));
    }
    doOperation_IntDiv(result, a_2d, b, bitlength, size, threadID, net, ss);
    for (int i = 0; i < size; i++)
        delete[] a_2d[i];
    delete[] a_2d;
}

#endif // _INTDIV_HPP_