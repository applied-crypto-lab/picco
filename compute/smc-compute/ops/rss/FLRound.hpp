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
#ifndef _FLROUND_HPP_
#define _FLROUND_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "EQZ.hpp"
#include "MSB.hpp"
#include "Mult.hpp"
#include "Open.hpp"
#include "Pow2.hpp"
#include "PreOR.hpp"
#include "RandBit.hpp"
#include "Trunc.hpp"

// ============================================================================
// Helper: PRandM - Generate random M-bit value with individual bits
// Source: Catrina and de Hoogh, "Improved Primitives for Secure Multiparty
//         Integer Computation," 2010
// Output format:
//   result[0..M-1][size][numShares] - individual bits
//   result[M][size][numShares] - the M-bit random value r' = sum_{i=0}^{M-1} 2^i * b_i
// ============================================================================
template <typename T>
void Rss_PRandM(T ***result, int M, int size, uint ring_size, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    // Generate M*size random bits
    T **bits = new T*[M * size];
    for (int i = 0; i < M * size; i++) {
        bits[i] = new T[numShares];
        memset(bits[i], 0, sizeof(T) * numShares);
    }

    // Generate random bits using RandBit
    Rss_RandBit(bits, M * size, ring_size, net, ss);

    // Organize bits into result array
    // result[bit_idx][elem_idx][share_idx]
    for (int b = 0; b < M; b++) {
        for (int i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                result[b][i][s] = bits[b * size + i][s];
            }
        }
    }

    // Compute r' = sum_{i=0}^{M-1} 2^i * b_i
    // Initialize result[M] to bit 0
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            result[M][i][s] = result[0][i][s];
        }
    }

    // Add remaining bits with appropriate powers of 2
    T pow2 = T(2);
    for (int b = 1; b < M; b++) {
        for (int i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                result[M][i][s] += pow2 * result[b][i][s];
            }
        }
        pow2 <<= 1;
    }

    // Cleanup
    for (int i = 0; i < M * size; i++) {
        delete[] bits[i];
    }
    delete[] bits;
}

// ============================================================================
// Helper: B2U - Binary to Unary conversion
// Source: Aliasgari et al., "Secure Computation on Floating Point Numbers," 2013
// Protocol B2U, page 4
// Input: A[size][numShares] - L-bit secret values (representing values 0 to L-1)
// Output: result[0..L-1][size][numShares] where result[k] = 1 if k < A
//         result[L][size][numShares] = 2^A
// Example: if A = 3, then result[0]=1, result[1]=1, result[2]=1, result[3]=0, ...
// ============================================================================
template <typename T>
void Rss_B2U(T **A, int L, T ***result, int size, uint ring_size, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, T(1));

    // Allocate pow2A for 2^A
    T **pow2A = new T*[size];
    for (int i = 0; i < size; i++) {
        pow2A[i] = new T[numShares];
        memset(pow2A[i], 0, sizeof(T) * numShares);
    }

    // Allocate temp for comparisons
    T **temp = new T*[size];
    T **msb_result = new T*[size];
    for (int i = 0; i < size; i++) {
        temp[i] = new T[numShares];
        msb_result[i] = new T[numShares];
        memset(temp[i], 0, sizeof(T) * numShares);
        memset(msb_result[i], 0, sizeof(T) * numShares);
    }

    // Step 1: Compute 2^A
    doOperation_Pow2(pow2A, A, L, size, -1, net, ss);

    // Step 2: For each k from 0 to L-1, compute result[k] = (k < A)
    // result[k] = LTZ(k - A) = MSB(k - A)
    // When k < A: k - A < 0, MSB = 1, result = 1
    // When k >= A: k - A >= 0, MSB = 0, result = 0
    for (int k = 0; k < L; k++) {
        // Compute temp = k - A
        for (int i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                temp[i][s] = T(k) * ai[s] - A[i][s];
            }
        }
        // Compute MSB(temp) = LTZ(k - A) = 1 if k < A
        Rss_MSB(msb_result, temp, size, ring_size, net, ss);
        // Copy to result[k]
        for (int i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                result[k][i][s] = msb_result[i][s];
            }
        }
    }

    // result[L] = 2^A
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            result[L][i][s] = pow2A[i][s];
        }
    }

    // Cleanup
    delete[] ai;

    for (int i = 0; i < size; i++) {
        delete[] pow2A[i];
        delete[] temp[i];
        delete[] msb_result[i];
    }
    delete[] pow2A;
    delete[] temp;
    delete[] msb_result;
}

// ============================================================================
// Helper: Mod2MS - Modular reduction with secret modulus
// Computes result = A mod 2^M where M is a secret value
// Also outputs powM = 2^M
// ============================================================================
template <typename T>
void Rss_Mod2MS(T **result, T **A, T **M, T **powM, int L, int size, uint ring_size, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, T(1));

    // Allocate X for B2U result: X[0..L-1] = unary, X[L] = 2^M
    T ***X = new T**[L + 1];
    for (int b = 0; b <= L; b++) {
        X[b] = new T*[size];
        for (int i = 0; i < size; i++) {
            X[b][i] = new T[numShares];
            memset(X[b][i], 0, sizeof(T) * numShares);
        }
    }

    // Allocate R for PRandM: R[0..L-1] = bits, R[L] = random L-bit value
    T ***R = new T**[L + 1];
    for (int b = 0; b <= L; b++) {
        R[b] = new T*[size];
        for (int i = 0; i < size; i++) {
            R[b][i] = new T[numShares];
            memset(R[b][i], 0, sizeof(T) * numShares);
        }
    }

    // Temp arrays
    T **R1 = new T*[size];
    T **R2 = new T*[size];
    T **S = new T*[size];
    T **T1 = new T*[size];
    T **CC = new T*[size];
    T **temp = new T*[size];
    T *temp_open = new T[size];  // Flat array for Open results
    for (int i = 0; i < size; i++) {
        R1[i] = new T[numShares];
        R2[i] = new T[numShares];
        S[i] = new T[numShares];
        T1[i] = new T[numShares];
        CC[i] = new T[numShares];
        temp[i] = new T[numShares];
        memset(R1[i], 0, sizeof(T) * numShares);
        memset(R2[i], 0, sizeof(T) * numShares);
        memset(S[i], 0, sizeof(T) * numShares);
        memset(T1[i], 0, sizeof(T) * numShares);
        memset(CC[i], 0, sizeof(T) * numShares);
        memset(temp[i], 0, sizeof(T) * numShares);
    }
    memset(temp_open, 0, sizeof(T) * size);

    T pow2L = T(1) << L;

    // Step 1: B2U(M) to get unary representation and 2^M
    Rss_B2U(M, L, X, size, ring_size, net, ss);

    // Copy 2^M to powM
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            powM[i][s] = X[L][i][s];
        }
    }

    // Step 2: Generate PRandM
    Rss_PRandM(R, L, size, ring_size, net, ss);

    // Step 3: Compute R1 and R2
    // With B2U(M): X[k] = 1 if k < M, X[k] = 0 if k >= M
    // R1 = sum_{k=0}^{L-1} 2^k * (1 - X[k]) * R[k]  (random part where k >= M, i.e., upper bits)
    // R2 = sum_{k=0}^{L-1} 2^k * X[k] * R[k]        (random part where k < M, i.e., lower bits)
    T pow2k = T(1);
    for (int k = 0; k < L; k++) {
        // temp = (1 - X[k]) * R[k]
        for (int i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                temp[i][s] = ai[s] - X[k][i][s];
            }
        }
        Mult(temp, temp, R[k], size, net, ss);
        for (int i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                R1[i][s] += pow2k * temp[i][s];
            }
        }

        // temp = X[k] * R[k]
        Mult(temp, X[k], R[k], size, net, ss);
        for (int i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                R2[i][s] += pow2k * temp[i][s];
            }
        }

        pow2k <<= 1;
    }

    // Step 4: Add random multiple of 2^L to R1 for security
    // Generate random S and add S * 2^L to R1
    T **S_bits = new T*[size];
    for (int i = 0; i < size; i++) {
        S_bits[i] = new T[numShares];
        memset(S_bits[i], 0, sizeof(T) * numShares);
    }
    Rss_RandBit(S_bits, size, ring_size, net, ss);
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            R1[i][s] += pow2L * S_bits[i][s];
        }
    }

    // Step 5: Open T1 = A + R1 + R2
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            temp[i][s] = A[i][s] + R1[i][s] + R2[i][s];
        }
    }
    Open(temp_open, temp, size, -1, net, ss);

    // Step 6: Compute CC = sum of (X[k] - X[k+1]) * (T1 mod 2^{k+1})
    // This extracts A mod 2^M from the opened value
    for (int k = 0; k < L - 1; k++) {
        T mask = (T(1) << (k + 1)) - 1;
        for (int i = 0; i < size; i++) {
            T c_val = temp_open[i] & mask;  // T1 mod 2^{k+1} (public)
            for (uint s = 0; s < numShares; s++) {
                temp[i][s] = (X[k][i][s] - X[k + 1][i][s]) * c_val;
            }
        }
        for (int i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                CC[i][s] += temp[i][s];
            }
        }
    }

    // Step 7: result = CC - R2
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            result[i][s] = CC[i][s] - R2[i][s];
        }
    }

    // Step 8: Handle wrap-around using LTZ
    // If result < 0 (i.e., CC < R2), add 2^M
    // temp = CC - R2, check if negative
    Rss_MSB(temp, result, size, ring_size, net, ss);
    // result = result + temp * powM
    Mult(temp, temp, powM, size, net, ss);
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            result[i][s] += temp[i][s];
        }
    }

    // Cleanup
    delete[] ai;
    for (int i = 0; i < size; i++) {
        delete[] S_bits[i];
    }
    delete[] S_bits;

    for (int b = 0; b <= L; b++) {
        for (int i = 0; i < size; i++) {
            delete[] X[b][i];
            delete[] R[b][i];
        }
        delete[] X[b];
        delete[] R[b];
    }
    delete[] X;
    delete[] R;

    for (int i = 0; i < size; i++) {
        delete[] R1[i];
        delete[] R2[i];
        delete[] S[i];
        delete[] T1[i];
        delete[] CC[i];
        delete[] temp[i];
    }
    delete[] R1;
    delete[] R2;
    delete[] S;
    delete[] T1;
    delete[] CC;
    delete[] temp;
    delete[] temp_open;
}

// ============================================================================
// FLRound - Round a floating point number
// Source: Aliasgari et al., "Secure Computation on Floating Point Numbers," 2013
// Protocol FLRound, page 8
//
// Input: A_in[4][size][numShares] = (v, p, z, s) - float representation
//        mode[size][numShares] - rounding mode (0 = toward zero, 1 = away from zero)
// Output: result[4][size][numShares] = rounded float (v', p', z', s')
// Parameters: L = mantissa bits (q in paper), K = exponent bits (k in paper)
//
// Optimization: Steps 1-2 are batched into a single MSB call.
// ============================================================================
template <typename T>
void doOperation_FLRound(T ***A_in, T ***result, T **mode, int L, int K, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, T(1));

    T constPower2L = T(1) << L;
    T constPower2L1 = T(1) << (L - 1);

    // Work directly with input (no FLAdd - protocol doesn't add 0.5)
    T ***A = A_in;

    // Allocate temporary arrays
    T **a_flag = new T*[size];      // a = LTZ(p)
    T **b_flag = new T*[size];      // b = LT(p, -L+1)
    T **c_flag = new T*[size];      // c = EQZ(v2)
    T **d_flag = new T*[size];      // d = EQ(v, 2^L)
    T **V = new T*[size];           // working mantissa
    T **V2 = new T*[size];          // v mod 2^m
    T **powM = new T*[size];        // 2^m from Mod2MS
    T **temp1 = new T*[size];
    T **temp2 = new T*[size];
    T **temp3 = new T*[size];

    for (int i = 0; i < size; i++) {
        a_flag[i] = new T[numShares];
        b_flag[i] = new T[numShares];
        c_flag[i] = new T[numShares];
        d_flag[i] = new T[numShares];
        V[i] = new T[numShares];
        V2[i] = new T[numShares];
        powM[i] = new T[numShares];
        temp1[i] = new T[numShares];
        temp2[i] = new T[numShares];
        temp3[i] = new T[numShares];
        memset(a_flag[i], 0, sizeof(T) * numShares);
        memset(b_flag[i], 0, sizeof(T) * numShares);
        memset(c_flag[i], 0, sizeof(T) * numShares);
        memset(d_flag[i], 0, sizeof(T) * numShares);
        memset(V[i], 0, sizeof(T) * numShares);
        memset(V2[i], 0, sizeof(T) * numShares);
        memset(powM[i], 0, sizeof(T) * numShares);
        memset(temp1[i], 0, sizeof(T) * numShares);
        memset(temp2[i], 0, sizeof(T) * numShares);
        memset(temp3[i], 0, sizeof(T) * numShares);
    }

    // Line 1 (batched): Compute a = LTZ(p) and b = LT(p, -L+1) in one MSB call
    // a = LTZ(p) = MSB(p)
    // b = LT(p, -L+1) = LTZ(p + L - 1) = MSB(p + L - 1)
    T **msb_input = new T*[2 * size];
    T **msb_output = new T*[2 * size];
    for (int i = 0; i < 2 * size; i++) {
        msb_input[i] = new T[numShares];
        msb_output[i] = new T[numShares];
        memset(msb_input[i], 0, sizeof(T) * numShares);
        memset(msb_output[i], 0, sizeof(T) * numShares);
    }

    // First half: p (for a = LTZ(p))
    // Second half: p + L - 1 (for b = LT(p, -L+1))
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            msb_input[i][s] = A[1][i][s];                           // p
            msb_input[i + size][s] = A[1][i][s] + T(L - 1) * ai[s]; // p + L - 1
        }
    }

    // Single batched MSB call
    Rss_MSB(msb_output, msb_input, 2 * size, ring_size, net, ss);

    // Extract results: a_flag from first half, b_flag from second half
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            a_flag[i][s] = msb_output[i][s];
            b_flag[i][s] = msb_output[i + size][s];
        }
    }

    // Cleanup batched arrays
    for (int i = 0; i < 2 * size; i++) {
        delete[] msb_input[i];
        delete[] msb_output[i];
    }
    delete[] msb_input;
    delete[] msb_output;

    // Line 2: Compute m = -a*(1-b)*p (shift amount for Mod2MS)
    // When a=1 (p<0) and b=0 (p >= -L+1): m = -p = |p|
    // Otherwise: m = 0
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            temp1[i][s] = ai[s] - b_flag[i][s];  // 1 - b
        }
    }
    Mult(temp1, a_flag, temp1, size, net, ss);      // a * (1 - b)
    Mult(temp1, temp1, A[1], size, net, ss);        // a * (1 - b) * p
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            temp1[i][s] = T(0) - temp1[i][s];       // m = -a*(1-b)*p
        }
    }

    // Mod2MS: V2 = v mod 2^m, powM = 2^m
    Rss_Mod2MS(V2, A[0], temp1, powM, L, size, ring_size, net, ss);

    // Line 3: c = EQZ(V2)
    doOperation_EQZ(V2, c_flag, L, size, threadID, net, ss);

    // Line 4: V = v - V2 + (1-c) * powM * mode
    // For mode 0 (toward zero): just subtract V2 (floor the mantissa)
    // For mode 1 (away from zero): add powM to round up (ceil the mantissa)
    // Note: The sign is handled separately, so we always operate on magnitude.
    //       Mode 0 floors the magnitude (toward zero), mode 1 ceils (away from zero).

    // (1-c) * powM * mode
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            temp2[i][s] = ai[s] - c_flag[i][s];  // 1 - c
        }
    }
    Mult(temp2, temp2, powM, size, net, ss);
    Mult(temp2, temp2, mode, size, net, ss);

    // V = v - V2 + temp2
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            V[i][s] = A[0][i][s] - V2[i][s] + temp2[i][s];
        }
    }

    // Line 5: d = EQ(V, 2^L) = EQZ(V - 2^L)
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            temp1[i][s] = V[i][s] - constPower2L * ai[s];
        }
    }
    doOperation_EQZ(temp1, d_flag, L + 1, size, threadID, net, ss);

    // Line 6: V = d * 2^{L-1} + (1-d) * V
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            temp1[i][s] = ai[s] - d_flag[i][s];  // 1 - d
        }
    }
    Mult(temp1, temp1, V, size, net, ss);  // (1-d) * V
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            V[i][s] = d_flag[i][s] * constPower2L1 + temp1[i][s];
        }
    }

    // Line 7: V = a * ((1-b)*V + b*(mode - s)) + (1-a) * v
    // First: (1-b)*V
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            temp1[i][s] = ai[s] - b_flag[i][s];  // 1 - b
        }
    }
    Mult(temp1, temp1, V, size, net, ss);  // (1-b)*V

    // Second: b*(mode - s)
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            temp2[i][s] = mode[i][s] - A[3][i][s];  // mode - s
        }
    }
    Mult(temp2, b_flag, temp2, size, net, ss);  // b*(mode - s)

    // (1-b)*V + b*(mode - s)
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            temp1[i][s] = temp1[i][s] + temp2[i][s];
        }
    }

    // a * ((1-b)*V + b*(mode - s))
    Mult(temp1, a_flag, temp1, size, net, ss);

    // (1-a) * v
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            temp2[i][s] = ai[s] - a_flag[i][s];  // 1 - a
        }
    }
    Mult(temp2, temp2, A[0], size, net, ss);

    // V = a*(...) + (1-a)*v
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            V[i][s] = temp1[i][s] + temp2[i][s];
        }
    }

    // Line 8: s' = (1 - b*mode) * s
    Mult(temp1, b_flag, mode, size, net, ss);  // b * mode
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            temp1[i][s] = ai[s] - temp1[i][s];  // 1 - b*mode
        }
    }
    Mult(result[3], temp1, A[3], size, net, ss);

    // Line 9: z' = OR(EQZ(V), z) = EQZ(V) + z - EQZ(V)*z
    doOperation_EQZ(V, temp1, L, size, threadID, net, ss);
    Mult(temp2, temp1, A[2], size, net, ss);  // EQZ(V) * z
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            result[2][i][s] = temp1[i][s] + A[2][i][s] - temp2[i][s];
        }
    }

    // Line 10: v' = V * (1 - z')
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            temp1[i][s] = ai[s] - result[2][i][s];  // 1 - z'
        }
    }
    Mult(result[0], V, temp1, size, net, ss);

    // Line 11: p' = (p + d*a*(1-b)) * (1 - z')
    // First: d*a*(1-b)
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            temp1[i][s] = ai[s] - b_flag[i][s];  // 1 - b
        }
    }
    Mult(temp1, d_flag, temp1, size, net, ss);  // d*(1-b)
    Mult(temp1, temp1, a_flag, size, net, ss);  // d*a*(1-b)

    // p + d*a*(1-b)
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            temp1[i][s] = A[1][i][s] + temp1[i][s];
        }
    }

    // (p + d*a*(1-b)) * (1 - z')
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            temp2[i][s] = ai[s] - result[2][i][s];  // 1 - z'
        }
    }
    Mult(result[1], temp1, temp2, size, net, ss);

    // Cleanup
    delete[] ai;

    // Note: A is just a pointer to A_in, no need to free it

    for (int i = 0; i < size; i++) {
        delete[] a_flag[i];
        delete[] b_flag[i];
        delete[] c_flag[i];
        delete[] d_flag[i];
        delete[] V[i];
        delete[] V2[i];
        delete[] powM[i];
        delete[] temp1[i];
        delete[] temp2[i];
        delete[] temp3[i];
    }
    delete[] a_flag;
    delete[] b_flag;
    delete[] c_flag;
    delete[] d_flag;
    delete[] V;
    delete[] V2;
    delete[] powM;
    delete[] temp1;
    delete[] temp2;
    delete[] temp3;
}

#endif // _FLROUND_HPP_
