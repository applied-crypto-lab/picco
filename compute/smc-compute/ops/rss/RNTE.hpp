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

// Interface format: [size][numShares] where array[i][s] is share s of element i
// This protocol requires that the bitlength of the input is at least one bit shorter than the ring size
// This function returns two values:
// result: input >> m
// result_prime: input >> (m-2), where the LSB is set iff any of the lower (m-2) bits are set (sticky bit)
template <typename T>
void doOperation_Trunc_RNTE(T **result, T **result_prime, T **input, int K, int m, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;
    int m2 = m - 2;

    // Allocate in interface format [size][numShares]
    T **edaBit_r = new T *[size];
    T **sum = new T *[size];
    T **r_hat = new T *[size];
    T **r_hat_hat = new T *[size];
    T **b = new T *[size];
    T **b_km1 = new T *[size];
    T **b_2 = new T *[size];
    T **u_2 = new T *[size];
    T **v_2 = new T *[size];
    T **b2a_buff = new T *[4 * size];

    T *c = new T[size];
    memset(c, 0, sizeof(T) * size);
    T *c_prime = new T[size];
    memset(c_prime, 0, sizeof(T) * size);
    T *d_prime = new T[size];
    memset(d_prime, 0, sizeof(T) * size);

    for (int i = 0; i < size; i++) {
        edaBit_r[i] = new T[numShares];
        memset(edaBit_r[i], 0, sizeof(T) * numShares);
        r_hat[i] = new T[numShares];
        memset(r_hat[i], 0, sizeof(T) * numShares);
        r_hat_hat[i] = new T[numShares];
        memset(r_hat_hat[i], 0, sizeof(T) * numShares);
        b_km1[i] = new T[numShares];
        memset(b_km1[i], 0, sizeof(T) * numShares);
        b_2[i] = new T[numShares];
        memset(b_2[i], 0, sizeof(T) * numShares);
        sum[i] = new T[numShares];
        memset(sum[i], 0, sizeof(T) * numShares);
        b[i] = new T[numShares];
        memset(b[i], 0, sizeof(T) * numShares);
        u_2[i] = new T[numShares];
        memset(u_2[i], 0, sizeof(T) * numShares);
        v_2[i] = new T[numShares];
        memset(v_2[i], 0, sizeof(T) * numShares);
    }
    for (int i = 0; i < 4 * size; i++) {
        b2a_buff[i] = new T[numShares];
        memset(b2a_buff[i], 0, sizeof(T) * numShares);
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, 1);

    edaBit_RNTE(edaBit_r, r_hat, r_hat_hat, b_2, b_km1, m, size, ring_size, nodeNet, ss);

    // computing the sum of input and edabit_r
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            sum[i][s] = (input[i][s] + edaBit_r[i][s]);
        }
    }

    Open(c, sum, size, threadID, nodeNet, ss);

    // (c / 2^m) mod 2^(k-m-1)
    // (d / 2^m) mod 2^(k-m-3)
    for (int i = 0; i < size; i++) {
        c_prime[i] = (c[i] >> T(m)) & ss->SHIFT[ring_size - m - 1];
        d_prime[i] = (c[i] >> T(m2)) & ss->SHIFT[ring_size - m2 - 1];
    }

    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            b[i][s] = b_km1[i][s] + ((c[i] * ai[s]) >> T(ring_size - 1)) - 2 * ((c[i]) >> T(ring_size - 1)) * b_km1[i][s];

            r_hat[i][s] = r_hat[i][s] - (b_km1[i][s] << T(ring_size - 1 - m));
            result[i][s] = (c_prime[i] * ai[s]) - r_hat[i][s] + (b[i][s] << T(ring_size - m - 1));

            r_hat_hat[i][s] = r_hat_hat[i][s] - (b_km1[i][s] << T(ring_size - 1 - m2));
            result_prime[i][s] = (d_prime[i] * ai[s]) - r_hat_hat[i][s] + (b[i][s] << T(ring_size - m2 - 1));
        }
    }

    // Convert probabilistic truncation to deterministic
    Rss_BitLT(u_2, c, b_2, size, m, nodeNet, ss);
    Rss_BitLT(v_2, c, b_2, size, m2, nodeNet, ss);

    // We only need the LSB of the computed value
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            // computing (c_0 ^ b_0) ^ v (in Z2)
            r_hat[i][s] = (c[i] & (ai[s] * T(-1))) ^ b_2[i][s];
            // store in last size elements of b2a_buff
            b2a_buff[3 * size + i][s] = (r_hat[i][s] & T(1)) ^ v_2[i][s];
        }
    }

    // computing [w], a bit - checking if any of the m-2 lower bits of the input are set
    Rss_k_OR_L(b2a_buff, r_hat, size, m2, nodeNet, ss);

    // Copy u_2 and v_2 into b2a_buff
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            b2a_buff[size + i][s] = u_2[i][s];
            b2a_buff[2 * size + i][s] = v_2[i][s];
        }
    }

    Rss_B2A(b2a_buff, b2a_buff, 4 * size, ring_size, nodeNet, ss);

    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            result[i][s] = result[i][s] - b2a_buff[size + i][s];

            // result of truncation by two fewer bits, and
            // setting the LSB of result_prime (the "sticky bit")
            result_prime[i][s] = result_prime[i][s] - b2a_buff[2 * size + i][s] + b2a_buff[i][s] - b2a_buff[3 * size + i][s];
        }
    }

    // Cleanup
    delete[] ai;
    delete[] c;
    delete[] c_prime;
    delete[] d_prime;

    for (int i = 0; i < size; i++) {
        delete[] edaBit_r[i];
        delete[] sum[i];
        delete[] b[i];
        delete[] r_hat[i];
        delete[] r_hat_hat[i];
        delete[] b_km1[i];
        delete[] b_2[i];
        delete[] u_2[i];
        delete[] v_2[i];
    }
    for (int i = 0; i < 4 * size; i++) {
        delete[] b2a_buff[i];
    }
    delete[] b2a_buff;
    delete[] u_2;
    delete[] v_2;
    delete[] edaBit_r;
    delete[] b;
    delete[] sum;
    delete[] r_hat;
    delete[] r_hat_hat;
    delete[] b_km1;
    delete[] b_2;
}

// Interface format: [size][numShares] where array[i][s] is share s of element i
template <typename T>
void RNTE(T **result, T **input, int K, int m, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;
    uint num_uints = (size + 7) >> 3;

    // Allocate in interface format [size][numShares]
    T **a_pp = new T *[size];
    T **a_bits = new T *[size];

    uint8_t **A_buff = new uint8_t *[num_uints];
    uint8_t **B_buff = new uint8_t *[num_uints];
    uint8_t **C_buff = new uint8_t *[num_uints];

    for (int i = 0; i < size; i++) {
        a_pp[i] = new T[numShares];
        memset(a_pp[i], 0, sizeof(T) * numShares);
        a_bits[i] = new T[numShares];
        memset(a_bits[i], 0, sizeof(T) * numShares);
        memset(result[i], 0, sizeof(T) * numShares);
    }

    for (uint i = 0; i < num_uints; i++) {
        A_buff[i] = new uint8_t[numShares];
        memset(A_buff[i], 0, sizeof(uint8_t) * numShares);
        B_buff[i] = new uint8_t[numShares];
        memset(B_buff[i], 0, sizeof(uint8_t) * numShares);
        C_buff[i] = new uint8_t[numShares];
        memset(C_buff[i], 0, sizeof(uint8_t) * numShares);
    }

    doOperation_Trunc_RNTE(result, a_pp, input, K, m, size, threadID, nodeNet, ss);

    Rss_BitDec(a_bits, a_pp, 3, size, ring_size, nodeNet, ss);

    uint byte_idx;
    uint bit_idx;
    for (int i = 0; i < size; i++) {
        bit_idx = i & 7;
        byte_idx = i >> 3;
        for (uint s = 0; s < numShares; s++) {
            A_buff[byte_idx][s] = SET_BIT(A_buff[byte_idx][s], uint8_t(bit_idx), GET_BIT(uint8_t(a_bits[i][s]), uint8_t(0)));
            B_buff[byte_idx][s] = SET_BIT(B_buff[byte_idx][s], uint8_t(bit_idx), GET_BIT(uint8_t(a_bits[i][s]), uint8_t(2)));
        }
    }

    Mult_Byte(C_buff, A_buff, B_buff, num_uints, nodeNet, ss);

    for (uint i = 0; i < num_uints; i++) {
        for (uint s = 0; s < numShares; s++) {
            A_buff[i][s] = B_buff[i][s] ^ A_buff[i][s] ^ C_buff[i][s];
        }
    }

    for (uint i = 0; i < num_uints; i++) {
        for (uint s = 0; s < numShares; s++) {
            B_buff[i][s] = 0;
            C_buff[i][s] = 0;
        }
    }

    for (int i = 0; i < size; i++) {
        bit_idx = i & 7;
        byte_idx = i >> 3;
        for (uint s = 0; s < numShares; s++) {
            B_buff[byte_idx][s] = SET_BIT(B_buff[byte_idx][s], uint8_t(bit_idx), GET_BIT(uint8_t(a_bits[i][s]), uint8_t(1)));
        }
    }

    Mult_Byte(C_buff, A_buff, B_buff, num_uints, nodeNet, ss);

    // Using a_pp as a buffer for B2A
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            a_pp[i][s] = 0;
        }
    }

    for (int i = 0; i < size; i++) {
        bit_idx = i & 7;
        byte_idx = i >> 3;
        for (uint s = 0; s < numShares; s++) {
            a_pp[i][s] = GET_BIT(C_buff[byte_idx][s], uint8_t(bit_idx));
        }
    }

    Rss_B2A(a_pp, a_pp, size, ring_size, nodeNet, ss);

    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            result[i][s] += a_pp[i][s];
        }
    }

    // Cleanup
    for (int i = 0; i < size; i++) {
        delete[] a_pp[i];
        delete[] a_bits[i];
    }
    for (uint i = 0; i < num_uints; i++) {
        delete[] A_buff[i];
        delete[] B_buff[i];
        delete[] C_buff[i];
    }
    delete[] A_buff;
    delete[] B_buff;
    delete[] C_buff;
    delete[] a_bits;
    delete[] a_pp;
}

#endif // _RNTE_HPP_
