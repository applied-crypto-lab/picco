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
#ifndef _FL2INT_HPP_
#define _FL2INT_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "FLRound.hpp"
#include "MSB.hpp"
#include "Mult.hpp"
#include "Open.hpp"
#include "Pow2.hpp"
#include "TruncS.hpp"

// Enable debug output for FL2Int
#define FL2INT_DEBUG 0

// ============================================================================
// FL2Int - Convert floating point to integer for RSS
// Source: Aliasgari et al., "Secure Computation on Floating Point Numbers," 2013
// Protocol FL2Int, page 9
//
// Optimized implementation with 7 multiplication rounds (down from 10):
// Round 1: bc, (gamma-1-p')*(1-c), c*p, (1-c)*a, (1-z)*a, (1-c)*b
// Round 2: line5_step2, pow2_input, line7_part1, sign_za
// Round 3: m = line5_step2*a
// [Mod2MS + Pow2 can run]
// Round 4: line7_part2 = one_minus_cb*u
// [TruncS]
// Round 5: c*(t-v') for line 10
// [Mod2MS]
// Round 6: bc*(u'-v'), sign_za*pow2p (batched)
// Round 7: temp1*v' -> g
//
// Input: values1[4][size][numShares] = (v, p, z, s) float representation
// Output: results[size][numShares] = integer value (truncated toward zero)
// Parameters: L = mantissa bits (q), K = exponent bits (k), gamma = output integer bits (ℓ)
// ============================================================================

template <typename T>
void doOperation_FL2Int(T ***values1, T **results, int L, int K, int gamma, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, T(1));

    // Allocate rounded float storage
    T ***valuesP = new T**[4];
    T **mode = new T*[size];

    for (int c = 0; c < 4; c++) {
        valuesP[c] = new T*[size];
        for (int i = 0; i < size; i++) {
            valuesP[c][i] = new T[numShares];
            memset(valuesP[c][i], 0, sizeof(T) * numShares);
        }
    }

    for (int i = 0; i < size; i++) {
        mode[i] = new T[numShares];
        memset(mode[i], 0, sizeof(T) * numShares);  // mode = 0 for truncation toward zero
    }

    // Allocate working arrays
    T **v = new T*[size];       // mantissa
    T **p = new T*[size];       // exponent
    T **z = new T*[size];       // zero flag
    T **s = new T*[size];       // sign
    T **a_flag = new T*[size];  // a = LTZ(p' - (gamma - 1))
    T **b_flag = new T*[size];  // b = LTZ((gamma - L - 1) - p')
    T **c_flag = new T*[size];  // c = LTZ(p')
    T **m = new T*[size];       // shift amount for Mod2MS
    T **u = new T*[size];       // result of Mod2MS
    T **pow2m = new T*[size];   // 2^m from Mod2MS
    T **t = new T*[size];       // truncation result
    T **m_prime = new T*[size]; // m' for second Mod2MS
    T **u_prime = new T*[size]; // u' from second Mod2MS
    T **pow2m_prime = new T*[size]; // 2^{m'}
    T **pow2p = new T*[size];   // 2^{p'}
    T **bc = new T*[size];      // b * c

    // Precomputed values for optimization
    T **neg_p = new T*[size];       // -c*p = |p| when c=1
    T **one_minus_ca = new T*[size]; // (1-c)*a
    T **one_minus_za = new T*[size]; // (1-z)*a
    T **one_minus_cb = new T*[size]; // (1-c)*b
    T **pow2_input = new T*[size];   // (1-c)*a*p for Pow2
    T **sign_za = new T*[size];      // (1-2s)*(1-z)*a
    T **line7_part1 = new T*[size];  // (bc+1-b)*v

    // Mult buffer - max 6 parallel multiplications
    T **mult_buf1 = new T*[6 * size];
    T **mult_buf2 = new T*[6 * size];
    T **mult_out = new T*[6 * size];

    for (int i = 0; i < size; i++) {
        v[i] = new T[numShares];
        p[i] = new T[numShares];
        z[i] = new T[numShares];
        s[i] = new T[numShares];
        a_flag[i] = new T[numShares];
        b_flag[i] = new T[numShares];
        c_flag[i] = new T[numShares];
        m[i] = new T[numShares];
        u[i] = new T[numShares];
        pow2m[i] = new T[numShares];
        t[i] = new T[numShares];
        m_prime[i] = new T[numShares];
        u_prime[i] = new T[numShares];
        pow2m_prime[i] = new T[numShares];
        pow2p[i] = new T[numShares];
        bc[i] = new T[numShares];
        neg_p[i] = new T[numShares];
        one_minus_ca[i] = new T[numShares];
        one_minus_za[i] = new T[numShares];
        one_minus_cb[i] = new T[numShares];
        pow2_input[i] = new T[numShares];
        sign_za[i] = new T[numShares];
        line7_part1[i] = new T[numShares];

        memset(v[i], 0, sizeof(T) * numShares);
        memset(p[i], 0, sizeof(T) * numShares);
        memset(z[i], 0, sizeof(T) * numShares);
        memset(s[i], 0, sizeof(T) * numShares);
        memset(a_flag[i], 0, sizeof(T) * numShares);
        memset(b_flag[i], 0, sizeof(T) * numShares);
        memset(c_flag[i], 0, sizeof(T) * numShares);
        memset(m[i], 0, sizeof(T) * numShares);
        memset(u[i], 0, sizeof(T) * numShares);
        memset(pow2m[i], 0, sizeof(T) * numShares);
        memset(t[i], 0, sizeof(T) * numShares);
        memset(m_prime[i], 0, sizeof(T) * numShares);
        memset(u_prime[i], 0, sizeof(T) * numShares);
        memset(pow2m_prime[i], 0, sizeof(T) * numShares);
        memset(pow2p[i], 0, sizeof(T) * numShares);
        memset(bc[i], 0, sizeof(T) * numShares);
        memset(neg_p[i], 0, sizeof(T) * numShares);
        memset(one_minus_ca[i], 0, sizeof(T) * numShares);
        memset(one_minus_za[i], 0, sizeof(T) * numShares);
        memset(one_minus_cb[i], 0, sizeof(T) * numShares);
        memset(pow2_input[i], 0, sizeof(T) * numShares);
        memset(sign_za[i], 0, sizeof(T) * numShares);
        memset(line7_part1[i], 0, sizeof(T) * numShares);
    }

    for (int i = 0; i < 6 * size; i++) {
        mult_buf1[i] = new T[numShares];
        mult_buf2[i] = new T[numShares];
        mult_out[i] = new T[numShares];
        memset(mult_buf1[i], 0, sizeof(T) * numShares);
        memset(mult_buf2[i], 0, sizeof(T) * numShares);
        memset(mult_out[i], 0, sizeof(T) * numShares);
    }

    // ========================================================================
    // Line 1: [ã'] ← FLRound([ã], (q, k), 0)
    // Round toward zero (truncation)
    // ========================================================================
    doOperation_FLRound(values1, valuesP, mode, L, K, size, threadID, net, ss);

    // Copy rounded float components
    for (int i = 0; i < size; i++) {
        for (uint sh = 0; sh < numShares; sh++) {
            v[i][sh] = valuesP[0][i][sh];
            p[i][sh] = valuesP[1][i][sh];
            z[i][sh] = valuesP[2][i][sh];
            s[i][sh] = valuesP[3][i][sh];
        }
    }

#if FL2INT_DEBUG
    // Debug: Print input float components after FLRound
    // IMPORTANT: Open must be called by ALL parties, not just Party 1
    T *debug_v = new T[size];
    T *debug_p = new T[size];
    T *debug_z = new T[size];
    T *debug_s = new T[size];
    Open(debug_v, v, size, threadID, net, ss);
    Open(debug_p, p, size, threadID, net, ss);
    Open(debug_z, z, size, threadID, net, ss);
    Open(debug_s, s, size, threadID, net, ss);
    if (ss->getID() == 1) {
        printf("\n=== FL2INT DEBUG (Party 1) ===\n");
        printf("Parameters: L(mantissa)=%d, K(exponent)=%d, gamma(int_bits)=%d\n", L, K, gamma);
        for (int i = 0; i < size; i++) {
            // Handle signed exponent - if MSB is set, it's negative
            long long p_signed = (long long)debug_p[i];
            if (debug_p[i] > (T(1) << (ring_size - 1))) {
                // Negative value in two's complement
                p_signed = (long long)debug_p[i] - ((long long)1 << ring_size);
            }
            printf("Input[%d] after FLRound: v=%lld, p=%lld, z=%lld, s=%lld\n",
                   i, (long long)debug_v[i], p_signed, (long long)debug_z[i], (long long)debug_s[i]);
            // Compute expected value
            double expected = (double)debug_v[i] * pow(2.0, p_signed);
            if (debug_s[i]) expected = -expected;
            if (debug_z[i]) expected = 0;
            printf("  -> Expected float value: %f\n", expected);
        }
    }
    delete[] debug_v;
    delete[] debug_p;
    delete[] debug_z;
    delete[] debug_s;
#endif

    // ========================================================================
    // Lines 2-4: Compute flags a, b, c using batched MSB
    // ========================================================================
    T **msb_input = new T*[3 * size];
    T **msb_output = new T*[3 * size];
    for (int i = 0; i < 3 * size; i++) {
        msb_input[i] = new T[numShares];
        msb_output[i] = new T[numShares];
        memset(msb_input[i], 0, sizeof(T) * numShares);
        memset(msb_output[i], 0, sizeof(T) * numShares);
    }

    for (int i = 0; i < size; i++) {
        for (uint sh = 0; sh < numShares; sh++) {
            msb_input[i][sh] = p[i][sh] - T(gamma - 1) * ai[sh];           // a = LTZ(p - (gamma-1))
            msb_input[i + size][sh] = T(gamma - L - 1) * ai[sh] - p[i][sh]; // b = LTZ((gamma-L-1) - p)
            msb_input[i + 2 * size][sh] = p[i][sh];                         // c = LTZ(p)
        }
    }

    Rss_MSB(msb_output, msb_input, 3 * size, ring_size, net, ss);

    for (int i = 0; i < size; i++) {
        for (uint sh = 0; sh < numShares; sh++) {
            a_flag[i][sh] = msb_output[i][sh];
            b_flag[i][sh] = msb_output[i + size][sh];
            c_flag[i][sh] = msb_output[i + 2 * size][sh];
        }
    }

    for (int i = 0; i < 3 * size; i++) {
        delete[] msb_input[i];
        delete[] msb_output[i];
    }
    delete[] msb_input;
    delete[] msb_output;

#if FL2INT_DEBUG
    // Debug: Print flags a, b, c
    if (ss->getID() == 1) {
        T *debug_a = new T[size];
        T *debug_b = new T[size];
        T *debug_c = new T[size];
        Open(debug_a, a_flag, size, threadID, net, ss);
        Open(debug_b, b_flag, size, threadID, net, ss);
        Open(debug_c, c_flag, size, threadID, net, ss);
        printf("\nFlags after MSB:\n");
        for (int i = 0; i < size; i++) {
            printf("  [%d] a=%lld (p<gamma-1=%d), b=%lld (p>gamma-L-1=%d), c=%lld (p<0)\n",
                   i, (long long)debug_a[i], gamma-1,
                   (long long)debug_b[i], gamma-L-1,
                   (long long)debug_c[i]);
        }
        delete[] debug_a;
        delete[] debug_b;
        delete[] debug_c;
    }
#endif

    // ========================================================================
    // MULT ROUND 1 (6 batched): Precompute many products
    // 0: b*c -> bc
    // 1: (gamma-1-p')*(1-c) -> line5_step1
    // 2: c*p -> neg_p (will negate later)
    // 3: (1-c)*a -> one_minus_ca
    // 4: (1-z)*a -> one_minus_za
    // 5: (1-c)*b -> one_minus_cb
    // ========================================================================
    for (int i = 0; i < size; i++) {
        for (uint sh = 0; sh < numShares; sh++) {
            // b * c
            mult_buf1[i][sh] = b_flag[i][sh];
            mult_buf2[i][sh] = c_flag[i][sh];
            // (gamma-1-p') * (1-c)
            mult_buf1[i + size][sh] = T(gamma - 1) * ai[sh] - p[i][sh];
            mult_buf2[i + size][sh] = ai[sh] - c_flag[i][sh];
            // c * p
            mult_buf1[i + 2*size][sh] = c_flag[i][sh];
            mult_buf2[i + 2*size][sh] = p[i][sh];
            // (1-c) * a
            mult_buf1[i + 3*size][sh] = ai[sh] - c_flag[i][sh];
            mult_buf2[i + 3*size][sh] = a_flag[i][sh];
            // (1-z) * a
            mult_buf1[i + 4*size][sh] = ai[sh] - z[i][sh];
            mult_buf2[i + 4*size][sh] = a_flag[i][sh];
            // (1-c) * b
            mult_buf1[i + 5*size][sh] = ai[sh] - c_flag[i][sh];
            mult_buf2[i + 5*size][sh] = b_flag[i][sh];
        }
    }

    Mult(mult_out, mult_buf1, mult_buf2, 6 * size, threadID, net, ss);

    // Temp storage for line5_step1
    T **line5_step1 = new T*[size];
    for (int i = 0; i < size; i++) {
        line5_step1[i] = new T[numShares];
    }

    for (int i = 0; i < size; i++) {
        for (uint sh = 0; sh < numShares; sh++) {
            bc[i][sh] = mult_out[i][sh];
            line5_step1[i][sh] = mult_out[i + size][sh];
            neg_p[i][sh] = T(0) - mult_out[i + 2*size][sh];  // neg_p = -c*p
            one_minus_ca[i][sh] = mult_out[i + 3*size][sh];
            one_minus_za[i][sh] = mult_out[i + 4*size][sh];
            one_minus_cb[i][sh] = mult_out[i + 5*size][sh];
        }
    }

#if FL2INT_DEBUG
    // Debug: Print neg_p and bc
    if (ss->getID() == 1) {
        T *debug_negp = new T[size];
        T *debug_bc = new T[size];
        Open(debug_negp, neg_p, size, threadID, net, ss);
        Open(debug_bc, bc, size, threadID, net, ss);
        printf("\nAfter Round 1:\n");
        for (int i = 0; i < size; i++) {
            printf("  [%d] neg_p=%lld (should be |p| when c=1), bc=%lld\n",
                   i, (long long)debug_negp[i], (long long)debug_bc[i]);
        }
        delete[] debug_negp;
        delete[] debug_bc;
    }
#endif

    // ========================================================================
    // MULT ROUND 2 (4 batched):
    // 0: line5_step1 * b -> line5_step2
    // 1: one_minus_ca * p -> pow2_input (for line 14)
    // 2: (bc + 1 - b) * v -> line7_part1
    // 3: (1-2s) * one_minus_za -> sign_za (for lines 15-16)
    // ========================================================================
    for (int i = 0; i < size; i++) {
        for (uint sh = 0; sh < numShares; sh++) {
            // line5_step1 * b
            mult_buf1[i][sh] = line5_step1[i][sh];
            mult_buf2[i][sh] = b_flag[i][sh];
            // one_minus_ca * p
            mult_buf1[i + size][sh] = one_minus_ca[i][sh];
            mult_buf2[i + size][sh] = p[i][sh];
            // (bc + 1 - b) * v
            mult_buf1[i + 2*size][sh] = bc[i][sh] + ai[sh] - b_flag[i][sh];
            mult_buf2[i + 2*size][sh] = v[i][sh];
            // (1 - 2s) * one_minus_za
            mult_buf1[i + 3*size][sh] = ai[sh] - T(2) * s[i][sh];
            mult_buf2[i + 3*size][sh] = one_minus_za[i][sh];
        }
    }

    Mult(mult_out, mult_buf1, mult_buf2, 4 * size, threadID, net, ss);

    // Temp storage for line5_step2
    T **line5_step2 = new T*[size];
    for (int i = 0; i < size; i++) {
        line5_step2[i] = new T[numShares];
    }

    for (int i = 0; i < size; i++) {
        for (uint sh = 0; sh < numShares; sh++) {
            line5_step2[i][sh] = mult_out[i][sh];
            pow2_input[i][sh] = mult_out[i + size][sh];
            line7_part1[i][sh] = mult_out[i + 2*size][sh];
            sign_za[i][sh] = mult_out[i + 3*size][sh];
        }
    }

#if FL2INT_DEBUG
    // Debug: Print pow2_input = (1-c)*a*p
    if (ss->getID() == 1) {
        T *debug_pow2in = new T[size];
        Open(debug_pow2in, pow2_input, size, threadID, net, ss);
        printf("\nAfter Round 2:\n");
        for (int i = 0; i < size; i++) {
            printf("  [%d] pow2_input=(1-c)*a*p=%lld (for Pow2, should be 0 when c=1)\n",
                   i, (long long)debug_pow2in[i]);
        }
        delete[] debug_pow2in;
    }
#endif

    // ========================================================================
    // MULT ROUND 3: Complete line 5
    // m = line5_step2 * a = (gamma-1-p')*(1-c)*b*a
    // ========================================================================
    Mult(m, line5_step2, a_flag, size, threadID, net, ss);

    // Cleanup temp arrays
    for (int i = 0; i < size; i++) {
        delete[] line5_step1[i];
        delete[] line5_step2[i];
    }
    delete[] line5_step1;
    delete[] line5_step2;

    // ========================================================================
    // Line 6: Mod2MS and start Pow2 (pow2_input is ready)
    // ========================================================================
    Rss_Mod2MS(u, v, m, pow2m, L, size, ring_size, net, ss);
    doOperation_Pow2(pow2p, pow2_input, gamma - 1, size, threadID, net, ss);

#if FL2INT_DEBUG
    // Debug: Print pow2p = 2^pow2_input
    if (ss->getID() == 1) {
        T *debug_pow2p = new T[size];
        T *debug_m = new T[size];
        Open(debug_pow2p, pow2p, size, threadID, net, ss);
        Open(debug_m, m, size, threadID, net, ss);
        printf("\nAfter Pow2 and Mod2MS:\n");
        for (int i = 0; i < size; i++) {
            printf("  [%d] pow2p=2^pow2_input=%lld, m=%lld\n",
                   i, (long long)debug_pow2p[i], (long long)debug_m[i]);
        }
        delete[] debug_pow2p;
        delete[] debug_m;
    }
#endif

    // ========================================================================
    // MULT ROUND 4: Complete line 7
    // line7_part2 = one_minus_cb * u = (1-c)*b*u
    // ========================================================================
    Mult(mult_out, one_minus_cb, u, size, threadID, net, ss);

    // Line 7: v' = line7_part1 + line7_part2
    for (int i = 0; i < size; i++) {
        for (uint sh = 0; sh < numShares; sh++) {
            v[i][sh] = line7_part1[i][sh] + mult_out[i][sh];
        }
    }

    // ========================================================================
    // Line 9: TruncS (neg_p already computed in round 1)
    // ========================================================================
    doOperation_TruncS(t, v, L, neg_p, size, threadID, net, ss);

#if FL2INT_DEBUG
    // Debug: Print TruncS result
    if (ss->getID() == 1) {
        T *debug_t = new T[size];
        T *debug_v_before = new T[size];
        Open(debug_t, t, size, threadID, net, ss);
        Open(debug_v_before, v, size, threadID, net, ss);
        printf("\nAfter TruncS:\n");
        for (int i = 0; i < size; i++) {
            printf("  [%d] t=TruncS(v,L,neg_p)=%lld, v_before_line10=%lld\n",
                   i, (long long)debug_t[i], (long long)debug_v_before[i]);
        }
        delete[] debug_t;
        delete[] debug_v_before;
    }
#endif

    // ========================================================================
    // MULT ROUND 5: Line 10 optimized
    // v' = c*t + (1-c)*v' = v' + c*(t - v')
    // ========================================================================
    for (int i = 0; i < size; i++) {
        for (uint sh = 0; sh < numShares; sh++) {
            mult_buf1[i][sh] = c_flag[i][sh];
            mult_buf2[i][sh] = t[i][sh] - v[i][sh];
        }
    }
    Mult(mult_out, mult_buf1, mult_buf2, size, threadID, net, ss);

    for (int i = 0; i < size; i++) {
        for (uint sh = 0; sh < numShares; sh++) {
            v[i][sh] = v[i][sh] + mult_out[i][sh];
        }
    }

#if FL2INT_DEBUG
    // Debug: Print v' after line 10
    if (ss->getID() == 1) {
        T *debug_v10 = new T[size];
        Open(debug_v10, v, size, threadID, net, ss);
        printf("\nAfter Line 10 (v' = c*t + (1-c)*v'):\n");
        for (int i = 0; i < size; i++) {
            printf("  [%d] v'=%lld\n", i, (long long)debug_v10[i]);
        }
        delete[] debug_v10;
    }
#endif

    // ========================================================================
    // Line 11: m' = bc * (gamma - 1) (local computation)
    // ========================================================================
    for (int i = 0; i < size; i++) {
        for (uint sh = 0; sh < numShares; sh++) {
            m_prime[i][sh] = bc[i][sh] * T(gamma - 1);
        }
    }

    // ========================================================================
    // Line 12: Second Mod2MS
    // ========================================================================
    Rss_Mod2MS(u_prime, v, m_prime, pow2m_prime, L, size, ring_size, net, ss);

    // ========================================================================
    // MULT ROUND 6 (2 batched):
    // 0: bc * (u' - v') for line 13 (v' = v' + bc*(u'-v'))
    // 1: sign_za * pow2p for lines 15-16
    // ========================================================================
    for (int i = 0; i < size; i++) {
        for (uint sh = 0; sh < numShares; sh++) {
            // bc * (u' - v')
            mult_buf1[i][sh] = bc[i][sh];
            mult_buf2[i][sh] = u_prime[i][sh] - v[i][sh];
            // sign_za * pow2p
            mult_buf1[i + size][sh] = sign_za[i][sh];
            mult_buf2[i + size][sh] = pow2p[i][sh];
        }
    }

    Mult(mult_out, mult_buf1, mult_buf2, 2 * size, threadID, net, ss);

    // Line 13: v' = v' + bc*(u' - v')
    for (int i = 0; i < size; i++) {
        for (uint sh = 0; sh < numShares; sh++) {
            v[i][sh] = v[i][sh] + mult_out[i][sh];
        }
    }

    // Store sign_za * pow2p for round 7
    T **temp_r6 = new T*[size];
    for (int i = 0; i < size; i++) {
        temp_r6[i] = new T[numShares];
        for (uint sh = 0; sh < numShares; sh++) {
            temp_r6[i][sh] = mult_out[i + size][sh];
        }
    }

    // ========================================================================
    // MULT ROUND 7: Final result
    // g = temp_r6 * v' = (1-2s)*(1-z)*a*pow2p*v'
    // ========================================================================
    Mult(results, temp_r6, v, size, threadID, net, ss);

#if FL2INT_DEBUG
    // Debug: Print final v' and result
    if (ss->getID() == 1) {
        T *debug_vfinal = new T[size];
        T *debug_result = new T[size];
        T *debug_temp_r6 = new T[size];
        Open(debug_vfinal, v, size, threadID, net, ss);
        Open(debug_result, results, size, threadID, net, ss);
        Open(debug_temp_r6, temp_r6, size, threadID, net, ss);
        printf("\nFinal computation:\n");
        for (int i = 0; i < size; i++) {
            // Handle signed result
            long long result_signed = (long long)debug_result[i];
            if (debug_result[i] > (T(1) << (ring_size - 1))) {
                result_signed = (long long)debug_result[i] - ((long long)1 << ring_size);
            }
            printf("  [%d] v'_final=%lld, temp_r6=(1-2s)*(1-z)*a*pow2p=%lld\n",
                   i, (long long)debug_vfinal[i], (long long)debug_temp_r6[i]);
            printf("  [%d] RESULT g = temp_r6 * v' = %lld\n", i, result_signed);
        }
        printf("\n=== END FL2INT DEBUG ===\n\n");
        delete[] debug_vfinal;
        delete[] debug_result;
        delete[] debug_temp_r6;
    }
#endif

    // Cleanup temp_r6
    for (int i = 0; i < size; i++) {
        delete[] temp_r6[i];
    }
    delete[] temp_r6;

    // ========================================================================
    // Cleanup
    // ========================================================================
    delete[] ai;

    for (int c = 0; c < 4; c++) {
        for (int i = 0; i < size; i++) {
            delete[] valuesP[c][i];
        }
        delete[] valuesP[c];
    }
    delete[] valuesP;

    for (int i = 0; i < size; i++) {
        delete[] mode[i];
        delete[] v[i];
        delete[] p[i];
        delete[] z[i];
        delete[] s[i];
        delete[] a_flag[i];
        delete[] b_flag[i];
        delete[] c_flag[i];
        delete[] m[i];
        delete[] u[i];
        delete[] pow2m[i];
        delete[] t[i];
        delete[] m_prime[i];
        delete[] u_prime[i];
        delete[] pow2m_prime[i];
        delete[] pow2p[i];
        delete[] bc[i];
        delete[] neg_p[i];
        delete[] one_minus_ca[i];
        delete[] one_minus_za[i];
        delete[] one_minus_cb[i];
        delete[] pow2_input[i];
        delete[] sign_za[i];
        delete[] line7_part1[i];
    }
    delete[] mode;
    delete[] v;
    delete[] p;
    delete[] z;
    delete[] s;
    delete[] a_flag;
    delete[] b_flag;
    delete[] c_flag;
    delete[] m;
    delete[] u;
    delete[] pow2m;
    delete[] t;
    delete[] m_prime;
    delete[] u_prime;
    delete[] pow2m_prime;
    delete[] pow2p;
    delete[] bc;
    delete[] neg_p;
    delete[] one_minus_ca;
    delete[] one_minus_za;
    delete[] one_minus_cb;
    delete[] pow2_input;
    delete[] sign_za;
    delete[] line7_part1;

    for (int i = 0; i < 6 * size; i++) {
        delete[] mult_buf1[i];
        delete[] mult_buf2[i];
        delete[] mult_out[i];
    }
    delete[] mult_buf1;
    delete[] mult_buf2;
    delete[] mult_out;
}

#endif // _FL2INT_HPP_
