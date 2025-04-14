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

#ifndef _RNTE_HPP_
#define _RNTE_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "B2A.hpp"
#include "BitDec.hpp"
#include "BitLT.hpp"
#include "EQZ.hpp"
#include "EdaBit.hpp"
#include "Mult.hpp"
#include <cstdint>

// this protocol requires that the bitlength of the input is at least one bit shorter than the ring size, i.e. MSB(input) = 0
// This function as additional functionality in that it returns two values, result and result_prime
// result: input >> m
// result_prime: input >> (m-2), where the LSB is set if and only if any
// of the lower (m-2) bits of the input are set
// this is referred to as the "sticky bit" (in floating-point parlance)
template <typename T>
void doOperation_Trunc_RNTE(T **result, T **result_prime, T **input, int K, int m, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;
    int m2 = m - 2;

    T **edaBit_r = new T *[numShares];
    T **sum = new T *[numShares];
    T **r_hat = new T *[numShares];
    T **r_hat_hat = new T *[numShares];
    T **b = new T *[numShares];
    T **b_km1 = new T *[numShares];
    T **b_2 = new T *[numShares];
    T **u_2 = new T *[numShares];
    T **v_2 = new T *[numShares];
    T **w = new T *[numShares];

    T **b2a_buff = new T *[numShares];

    T *c = new T[size];
    memset(c, 0, sizeof(T) * size);
    T *c_prime = new T[size];
    memset(c_prime, 0, sizeof(T) * size);
    T *d_prime = new T[size];
    memset(d_prime, 0, sizeof(T) * size);

    for (size_t i = 0; i < numShares; i++) {
        edaBit_r[i] = new T[size];
        memset(edaBit_r[i], 0, sizeof(T) * size);
        r_hat[i] = new T[size];
        memset(r_hat[i], 0, sizeof(T) * size);

        r_hat_hat[i] = new T[size];
        memset(r_hat_hat[i], 0, sizeof(T) * size);

        b_km1[i] = new T[size];
        memset(b_km1[i], 0, sizeof(T) * size);
        b_2[i] = new T[size];
        memset(b_2[i], 0, sizeof(T) * size);
        sum[i] = new T[size];

        b[i] = new T[size];
        memset(b[i], 0, sizeof(T) * size);
        u_2[i] = new T[size];
        memset(u_2[i], 0, sizeof(T) * size);

        v_2[i] = new T[size];
        memset(v_2[i], 0, sizeof(T) * size);
        w[i] = new T[size];
        memset(w[i], 0, sizeof(T) * size);

        b2a_buff[i] = new T[4 * size];
        memset(b2a_buff[i], 0, sizeof(T) * 4 * size);
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, 1);

    edaBit_RNTE(edaBit_r, r_hat, r_hat_hat, b_2, b_km1, m, size, ring_size, nodeNet, ss);

    // computing the sum of input and edabit_r
    // unmodified from Trunc
    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            sum[s][i] = (input[s][i] + edaBit_r[s][i]);
        }
    }

    Open(c, sum, size, threadID, nodeNet, ss);

    // (c / 2^m) mod 2^(k-m-1)
    // (d / 2^m) mod 2^(k-m-3)
    for (size_t i = 0; i < size; i++) {
        c_prime[i] = (c[i] >> T(m)) & ss->SHIFT[ring_size - m - 1];
        d_prime[i] = (c[i] >> T(m2)) & ss->SHIFT[ring_size - m2 - 1];
    }

    for (size_t i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            b[s][i] = b_km1[s][i] + ((c[i] * ai[s]) >> T(ring_size - 1)) - 2 * ((c[i]) >> T(ring_size - 1)) * b_km1[s][i];

            r_hat[s][i] = r_hat[s][i] - (b_km1[s][i] << T(ring_size - 1 - m));
            result[s][i] = (c_prime[i] * ai[s]) - r_hat[s][i] + (b[s][i] << T(ring_size - m - 1));

            r_hat_hat[s][i] = r_hat_hat[s][i] - (b_km1[s][i] << T(ring_size - 1 - m2));
            result_prime[s][i] = (d_prime[i] * ai[s]) - r_hat_hat[s][i] + (b[s][i] << T(ring_size - m2 - 1));
        }
    }

    // the following block of code converts a probabilistic trunation to deterministic (exact)
    // in order to preserve security (see Li et al., "Efficient 3PC for Binary Circuits with Application to Maliciously-Secure DNN Inference", USENIX 2023)
    Rss_BitLT(u_2, c, b_2, size, m, nodeNet, ss);
    Rss_BitLT(v_2, c, b_2, size, m2, nodeNet, ss);

    // we only need the LSB of the computed value
    // need 0th bit of [b_2]
    // v_2 is already a bit in Z_2
    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            // computing (c_0 ^ b_0) ^ v (in Z2), and storing it in the last (size) elements
            r_hat[s][i] = (c[i] & (ai[s] * T(-1))) ^ b_2[s][i]; // reusing r_hat
            // we only care about the LSB, which is why we AND the result from the previous line with 1
            b2a_buff[s][3 * size + i] = (r_hat[s][i] & T(1)) ^ v_2[s][i];
        }
    }

    // computing [w], a bit
    Rss_k_OR_L(b2a_buff, r_hat, size, m2, nodeNet, ss); // checking if any of the m-2 lower bits of the input are set
    // stores "w" in the first (size) elements

    for (size_t s = 0; s < numShares; s++) {
        memcpy(b2a_buff[s] + size, u_2[s], sizeof(T) * size);     // bit, [u]_1
        memcpy(b2a_buff[s] + 2 * size, v_2[s], sizeof(T) * size); // bit, [v]_1
    }

    Rss_B2A(b2a_buff, b2a_buff, 4 * size, ring_size, nodeNet, ss);

    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            result[s][i] = result[s][i] - b2a_buff[s][size + i]; // result of normal truncation

            // result of truncation by two fewer bits, and
            // setting the LSB of result_prime (the "sticky bit")
            // iff any of the lower (m-2) bits of the input are set
            result_prime[s][i] = result_prime[s][i] - b2a_buff[s][2 * size + i] + b2a_buff[s][i] - b2a_buff[s][3 * size + i];
            // result_prime[s][i] = result_prime[s][i] - b2a_buff[s][2 * size + i] ;
        }
    }

    delete[] ai;
    delete[] c;
    delete[] c_prime;
    delete[] d_prime;

    for (size_t i = 0; i < numShares; i++) {
        delete[] edaBit_r[i];
        delete[] sum[i];
        delete[] b[i];
        delete[] r_hat[i];
        delete[] r_hat_hat[i];
        delete[] b_km1[i];
        delete[] b_2[i];
        delete[] u_2[i];
        delete[] v_2[i];
        delete[] w[i];
        delete[] b2a_buff[i];
    }
    delete[] b2a_buff;
    delete[] u_2;
    delete[] v_2;
    delete[] w;
    delete[] edaBit_r;
    delete[] b;
    delete[] sum;
    delete[] r_hat;
    delete[] r_hat_hat;
    delete[] b_km1;
    delete[] b_2;
}

template <typename T>
void RNTE(T **result, T **input, int K, int m, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;
    uint num_uints = (size + 7) >> 3;

    T **a_pp = new T *[numShares];
    T **a_bits = new T *[numShares];

    uint8_t **A_buff = new uint8_t *[numShares];
    uint8_t **B_buff = new uint8_t *[numShares];
    uint8_t **C_buff = new uint8_t *[numShares];
    for (size_t s = 0; s < numShares; s++) {
        A_buff[s] = new uint8_t[num_uints];
        memset(A_buff[s], 0, sizeof(uint8_t) * num_uints);
        B_buff[s] = new uint8_t[num_uints];
        memset(B_buff[s], 0, sizeof(uint8_t) * num_uints);
        C_buff[s] = new uint8_t[num_uints];
        memset(C_buff[s], 0, sizeof(uint8_t) * num_uints);

        a_pp[s] = new T[size];
        memset(a_pp[s], 0, sizeof(T) * size);
        a_bits[s] = new T[size];
        memset(a_bits[s], 0, sizeof(T) * size);

        memset(result[s], 0, sizeof(T) * size);
    }

    doOperation_Trunc_RNTE(result, a_pp, input, K, m, size, threadID, nodeNet, ss);

    Rss_BitDec(a_bits, a_pp, 3, size, ring_size, nodeNet, ss);

    uint byte_idx; // byte index of buffer vars, only increments every 8 bits
    uint bit_idx;  // bit position in buffer (0 to 7 then loops around)
    for (size_t s = 0; s < numShares; s++) {
        for (uint i = 0; i < size; i++) {
            bit_idx = i & 7;
            byte_idx = i >> 3;
            A_buff[s][byte_idx] = SET_BIT(A_buff[s][byte_idx], uint8_t(bit_idx), GET_BIT(uint8_t(a_bits[s][i]), uint8_t(0)));
            B_buff[s][byte_idx] = SET_BIT(B_buff[s][byte_idx], uint8_t(bit_idx), GET_BIT(uint8_t(a_bits[s][i]), uint8_t(2)));
        }
    }

    Mult_Byte(C_buff, A_buff, B_buff, num_uints, nodeNet, ss);

    for (size_t s = 0; s < numShares; s++) {
        for (uint i = 0; i < num_uints; i++) {
            A_buff[s][byte_idx] = B_buff[s][i] ^ A_buff[s][i] ^ C_buff[s][i];
        }

        memset(B_buff[s], 0, sizeof(uint8_t) * num_uints);
        memset(C_buff[s], 0, sizeof(uint8_t) * num_uints);
    }

    for (size_t s = 0; s < numShares; s++) {
        for (uint i = 0; i < size; i++) {
            bit_idx = i & 7;
            byte_idx = i >> 3;
            B_buff[s][byte_idx] = SET_BIT(B_buff[s][byte_idx], uint8_t(bit_idx), GET_BIT(uint8_t(a_bits[s][i]), uint8_t(1)));
        }
    }

    Mult_Byte(C_buff, A_buff, B_buff, num_uints, nodeNet, ss);

    for (size_t s = 0; s < numShares; s++) {
        memset(a_pp[s], 0, sizeof(T) * size); // using a_pp as a buffer for B2A
    }

    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            bit_idx = i & 7;
            byte_idx = i >> 3;
            a_pp[s][i] = GET_BIT(C_buff[s][byte_idx], uint8_t(bit_idx));
        }
    }

    Rss_B2A(a_pp, a_pp, size, ring_size, nodeNet, ss);

    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            result[s][i] += a_pp[s][i];
        }
    }

    for (size_t i = 0; i < numShares; i++) {
        delete[] A_buff[i];
        delete[] B_buff[i];
        delete[] C_buff[i];
        delete[] a_pp[i];
        delete[] a_bits[i];
    }
    delete[] A_buff;
    delete[] B_buff;
    delete[] C_buff;
    delete[] a_bits;
    delete[] a_pp;
}

#endif // _TRUNCRNTE_HPP_
