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
#ifndef _INT2FL_HPP_
#define _INT2FL_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "B2A.hpp"
#include "BitDec.hpp"
#include "LTEQ.hpp"
#include "Mult.hpp"
#include "PreOR.hpp"
#include "Trunc.hpp"

// Source: Aliasgari et al., "Secure Computation on Floating Point Numbers," 2013
// Protocol Int2FL, page 9
// Converts a private integer to a private float
// Input: values[size][numShares] - the integers to convert
// Output: results1[4][size][numShares] - float representation [v, p, z, s]
//   v = significand (mantissa)
//   p = exponent
//   z = zero flag
//   s = sign bit
// Parameters:
//   gamma = bit length of input integer (ℓ in paper notation)
//   K = mantissa bit length (q in paper notation)
template <typename T>
void doOperation_Int2FL(T **values, T ***results1, int gamma, int K, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;
    uint i, j, s;

    // Line 1: λ = γ - 1
    int lambda = gamma - 1;

    // Allocate temporary arrays - all use [element][share] format
    T **A = new T *[size];           // absolute value (reuses input)
    T **sign = new T *[size];        // sign bit (s = a < 0)
    T **zero = new T *[size];        // zero flag (z = a == 0)
    T **temp1 = new T *[size];
    T **exponent = new T *[size];    // exponent p

    for (i = 0; i < (uint)size; i++) {
        A[i] = new T[numShares];
        sign[i] = new T[numShares];
        zero[i] = new T[numShares];
        temp1[i] = new T[numShares];
        exponent[i] = new T[numShares];
        memset(exponent[i], 0, sizeof(T) * numShares);
    }

    // Bit decomposition result - stored as packed bits per element
    T **S_packed = new T *[size];
    for (i = 0; i < (uint)size; i++) {
        S_packed[i] = new T[numShares];
        memset(S_packed[i], 0, sizeof(T) * numShares);
    }

    // Initialize ai for public value embedding
    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, 1);

    // Copy input to A
    for (i = 0; i < (uint)size; i++) {
        for (s = 0; s < numShares; s++) {
            A[i][s] = values[i][s];
        }
    }

    // Allocate zero array for comparison
    T **zero_val = new T *[size];
    for (i = 0; i < (uint)size; i++) {
        zero_val[i] = new T[numShares];
        memset(zero_val[i], 0, sizeof(T) * numShares);
    }

    // Line 2: (s, z) = LTEQ(a, 0) - compute sign and zero in one operation
    // LTEQ(a, 0) gives: cLT = (a < 0) = sign, cEQ = (a == 0) = zero
    doOperation_LTEQ(A, zero_val, sign, zero, ring_size, size, net, ss);

    // Cleanup zero_val
    for (i = 0; i < (uint)size; i++) {
        delete[] zero_val[i];
    }
    delete[] zero_val;

    // Line 4: a = (1 - 2*s) * a = absolute value
    for (i = 0; i < (uint)size; i++) {
        for (s = 0; s < numShares; s++) {
            temp1[i][s] = ai[s] - T(2) * sign[i][s];  // 1 - 2*s
        }
    }
    Mult(A, temp1, A, size, threadID, net, ss);

    // Line 5: BitDec(a, λ, λ) - bit decomposition
    Rss_BitDec(S_packed, A, lambda, size, ring_size, net, ss);

    // Mask to keep only the first lambda bits
    T mask = (T(1) << lambda) - 1;
    for (i = 0; i < (uint)size; i++) {
        for (s = 0; s < numShares; s++) {
            S_packed[i][s] &= mask;
        }
    }

    // Line 6: PreOR([a_{λ-1}], ..., [a_0]) -> [b_0], ..., [b_{λ-1}]
    T **S_preor = new T *[size];
    for (i = 0; i < (uint)size; i++) {
        S_preor[i] = new T[numShares];
        for (s = 0; s < numShares; s++) {
            S_preor[i][s] = S_packed[i][s];
        }
    }
    Rss_PreOR(S_preor, S_preor, size, lambda, net, ss);

    // Extract individual bits for B2A conversion
    // S_bits[j * size + i][s] = bit j of element i, share s
    T **S_bits = new T *[lambda * size];
    for (i = 0; i < (uint)(lambda * size); i++) {
        S_bits[i] = new T[numShares];
        memset(S_bits[i], 0, sizeof(T) * numShares);
    }

    for (i = 0; i < (uint)size; i++) {
        for (j = 0; j < (uint)lambda; j++) {
            for (s = 0; s < numShares; s++) {
                S_bits[j * size + i][s] = (S_preor[i][s] >> j) & T(1);
            }
        }
    }

    // Convert bits from Z2 to Zp arithmetic shares
    Rss_B2A(S_bits, S_bits, lambda * size, ring_size, net, ss);

    // Line 7: v = a * (1 + Σᵢ 2^i(1 - b_i)) = a * (2^λ - Σᵢ 2^i * b_i)
    // Compute weighted_sum = Σᵢ 2^(λ-1-i) * b_i and sum_b = Σᵢ b_i
    // Note: We use 2^(λ-1-j) instead of 2^j because RSS PreOR outputs bits
    // in the opposite order from Shamir (which reverses bits before PreOR).
    // This maps b_j at position j to weight 2^(λ-1-j), matching the paper's formula.
    T **weighted_sum = new T *[size];
    T **sum_b = new T *[size];
    for (i = 0; i < (uint)size; i++) {
        weighted_sum[i] = new T[numShares];
        sum_b[i] = new T[numShares];
        memset(weighted_sum[i], 0, sizeof(T) * numShares);
        memset(sum_b[i], 0, sizeof(T) * numShares);
    }

    for (j = 0; j < (uint)lambda; j++) {
        T pow2j = T(1) << (lambda - 1 - j);  // 2^(λ-1-j)
        for (i = 0; i < (uint)size; i++) {
            for (s = 0; s < numShares; s++) {
                weighted_sum[i][s] += pow2j * S_bits[j * size + i][s];
                sum_b[i][s] += S_bits[j * size + i][s];
            }
        }
    }

    // Compute multiplier = 2^λ - weighted_sum
    T pow2lambda = T(1) << lambda;
    T **multiplier = new T *[size];
    for (i = 0; i < (uint)size; i++) {
        multiplier[i] = new T[numShares];
        for (s = 0; s < numShares; s++) {
            multiplier[i][s] = pow2lambda * ai[s] - weighted_sum[i][s];
        }
    }

    // v = a * multiplier (normalizes mantissa)
    Mult(A, A, multiplier, size, threadID, net, ss);

    // Line 7-8: Adjust mantissa for target precision
    // if (γ - 1) > K then v = Trunc(v, γ - 1, γ - K - 1)
    // else v = 2^(K - γ + 1) * v
    if (lambda > K) {
        doOperation_Trunc(results1[0], A, lambda, lambda - K, size, threadID, net, ss);
    } else {
        T scale = T(1) << (K - lambda);
        for (i = 0; i < (uint)size; i++) {
            for (s = 0; s < numShares; s++) {
                results1[0][i][s] = A[i][s] * scale;
            }
        }
    }

    // Line 9: p = (Σᵢ b_i - K)(1 - z)
    // Combined step 8 and 10 from original protocol:
    // Original: p = Σb_i - λ, then p = (p + λ - K)(1 - z) = (Σb_i - K)(1 - z)
    for (i = 0; i < (uint)size; i++) {
        for (s = 0; s < numShares; s++) {
            exponent[i][s] = sum_b[i][s] - T(K) * ai[s];  // Σb_i - K
            temp1[i][s] = ai[s] - zero[i][s];  // 1 - z
        }
    }
    Mult(results1[1], exponent, temp1, size, threadID, net, ss);

    // Copy zero flag and sign to result
    for (i = 0; i < (uint)size; i++) {
        for (s = 0; s < numShares; s++) {
            results1[2][i][s] = zero[i][s];
            results1[3][i][s] = sign[i][s];
        }
    }

    // Cleanup
    delete[] ai;

    for (i = 0; i < (uint)size; i++) {
        delete[] A[i];
        delete[] sign[i];
        delete[] zero[i];
        delete[] temp1[i];
        delete[] exponent[i];
        delete[] S_packed[i];
        delete[] S_preor[i];
        delete[] weighted_sum[i];
        delete[] sum_b[i];
        delete[] multiplier[i];
    }
    delete[] A;
    delete[] sign;
    delete[] zero;
    delete[] temp1;
    delete[] exponent;
    delete[] S_packed;
    delete[] S_preor;
    delete[] weighted_sum;
    delete[] sum_b;
    delete[] multiplier;

    for (i = 0; i < (uint)(lambda * size); i++) {
        delete[] S_bits[i];
    }
    delete[] S_bits;
}

#endif // _INT2FL_HPP_
