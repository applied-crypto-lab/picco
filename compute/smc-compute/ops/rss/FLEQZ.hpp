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

/*
 * Float Equality (FLEQZ) - Checks if two floating-point numbers are equal
 *
 * Source: Aliasgari et al., "Secure Computation on Floating Point Numbers," 2013
 *
 * Float representation: [mantissa, exponent, zero_flag, sign]
 *   - A[0][i][s] = mantissa (v)
 *   - A[1][i][s] = exponent (p)
 *   - A[2][i][s] = zero flag (z)
 *   - A[3][i][s] = sign (s)
 *
 * Algorithm:
 *   b1 = EQZ(v_a - v_b)           // mantissas are equal
 *   b2 = EQZ(p_a - p_b)           // exponents are equal
 *   b3 = 1 - XOR(s_a, s_b)        // signs are equal (XNOR)
 *        where XOR(a,b) = a + b - 2*a*b, so XNOR = 1 - a - b + 2*a*b
 *   b4 = AND(b1, b2, b3)          // all non-zero components match
 *   b5 = AND(z_a, z_b)            // both are zero
 *   result = OR(b4, b5)           // equal if components match OR both zero
 *
 * Optimized multiplication rounds:
 *   Round 1: s_a * s_b, b1 * b2   (2 mults in parallel)
 *   Round 2: (b1*b2) * b3, z_a * z_b   (2 mults in parallel)
 *   Round 3: b4 * b5              (1 mult for OR: a+b-a*b)
 */

#ifndef _FLEQZ_HPP_
#define _FLEQZ_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "EQZ.hpp"
#include "Mult.hpp"

// Float equality: A == B
// Interface format: A[component][element][share], result[element][share]
// Components: 0=mantissa, 1=exponent, 2=zero_flag, 3=sign
template <typename T>
void doOperation_FLEQZ(T ***A, T ***B, T **result, int K, int L, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    uint i, s;

    // Allocate arrays in [size][numShares] format
    T **b1 = new T *[size];       // mantissas equal
    T **b2 = new T *[size];       // exponents equal
    T **b3 = new T *[size];       // signs equal (XNOR)
    T **b4 = new T *[size];       // all match (b1 AND b2 AND b3)
    T **b5 = new T *[size];       // both zero (z_a AND z_b)
    T **b6 = new T *[size];       // intermediate: b1 * b2
    T **sa_sb = new T *[size];    // s_a * s_b

    T **diff_v = new T *[size];   // v_a - v_b (for EQZ)
    T **diff_p = new T *[size];   // p_a - p_b (for EQZ)

    // Buffers for parallel multiplications - max 2*size for batching
    T **mult_buffer1 = new T *[2 * size];
    T **mult_buffer2 = new T *[2 * size];
    T **mult_result = new T *[2 * size];

    // Memory allocation
    for (i = 0; i < (uint)size; i++) {
        b1[i] = new T[numShares];
        memset(b1[i], 0, sizeof(T) * numShares);
        b2[i] = new T[numShares];
        memset(b2[i], 0, sizeof(T) * numShares);
        b3[i] = new T[numShares];
        memset(b3[i], 0, sizeof(T) * numShares);
        b4[i] = new T[numShares];
        memset(b4[i], 0, sizeof(T) * numShares);
        b5[i] = new T[numShares];
        memset(b5[i], 0, sizeof(T) * numShares);
        b6[i] = new T[numShares];
        memset(b6[i], 0, sizeof(T) * numShares);
        sa_sb[i] = new T[numShares];
        memset(sa_sb[i], 0, sizeof(T) * numShares);
        diff_v[i] = new T[numShares];
        memset(diff_v[i], 0, sizeof(T) * numShares);
        diff_p[i] = new T[numShares];
        memset(diff_p[i], 0, sizeof(T) * numShares);
    }

    for (i = 0; i < 2 * (uint)size; i++) {
        mult_buffer1[i] = new T[numShares];
        memset(mult_buffer1[i], 0, sizeof(T) * numShares);
        mult_buffer2[i] = new T[numShares];
        memset(mult_buffer2[i], 0, sizeof(T) * numShares);
        mult_result[i] = new T[numShares];
        memset(mult_result[i], 0, sizeof(T) * numShares);
    }

    // Public constant 1 in sparse representation
    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, 1);

    // Step 0: Compute differences for EQZ
    for (i = 0; i < (uint)size; i++) {
        for (s = 0; s < numShares; s++) {
            diff_v[i][s] = A[0][i][s] - B[0][i][s];
            diff_p[i][s] = A[1][i][s] - B[1][i][s];
        }
    }

    // Compute b1 = EQZ(v_a - v_b) and b2 = EQZ(p_a - p_b)
    doOperation_EQZ(diff_v, b1, K, size, threadID, net, ss);
    doOperation_EQZ(diff_p, b2, L, size, threadID, net, ss);

    // Round 1: Compute s_a * s_b and b1 * b2 in parallel
    for (i = 0; i < (uint)size; i++) {
        for (s = 0; s < numShares; s++) {
            mult_buffer1[i][s] = A[3][i][s];              // s_a
            mult_buffer2[i][s] = B[3][i][s];              // s_b

            mult_buffer1[i + size][s] = b1[i][s];         // b1
            mult_buffer2[i + size][s] = b2[i][s];         // b2
        }
    }

    Mult(mult_result, mult_buffer1, mult_buffer2, 2 * size, threadID, net, ss);

    // Extract results and compute b3 = 1 - XOR(s_a, s_b) = 1 - s_a - s_b + 2*s_a*s_b
    for (i = 0; i < (uint)size; i++) {
        for (s = 0; s < numShares; s++) {
            sa_sb[i][s] = mult_result[i][s];              // s_a * s_b
            b6[i][s] = mult_result[i + size][s];          // b1 * b2

            // b3 = 1 - s_a - s_b + 2*s_a*s_b
            b3[i][s] = ai[s] - A[3][i][s] - B[3][i][s] + T(2) * sa_sb[i][s];
        }
    }

    // Round 2: Compute (b1*b2) * b3 = b4 and z_a * z_b = b5 in parallel
    for (i = 0; i < (uint)size; i++) {
        for (s = 0; s < numShares; s++) {
            mult_buffer1[i][s] = b6[i][s];                // b1 * b2
            mult_buffer2[i][s] = b3[i][s];                // b3

            mult_buffer1[i + size][s] = A[2][i][s];       // z_a
            mult_buffer2[i + size][s] = B[2][i][s];       // z_b
        }
    }

    Mult(mult_result, mult_buffer1, mult_buffer2, 2 * size, threadID, net, ss);

    // Extract results: b4 = (b1*b2)*b3, b5 = z_a * z_b
    for (i = 0; i < (uint)size; i++) {
        for (s = 0; s < numShares; s++) {
            b4[i][s] = mult_result[i][s];                 // b4 = b1 * b2 * b3
            b5[i][s] = mult_result[i + size][s];          // b5 = z_a * z_b
        }
    }

    // Round 3: Compute OR(b4, b5) = b4 + b5 - b4*b5
    Mult(mult_result, b4, b5, size, threadID, net, ss);

    // Final result: result = b4 + b5 - b4*b5
    for (i = 0; i < (uint)size; i++) {
        for (s = 0; s < numShares; s++) {
            result[i][s] = b4[i][s] + b5[i][s] - mult_result[i][s];
        }
    }

    // Cleanup
    delete[] ai;

    for (i = 0; i < (uint)size; i++) {
        delete[] b1[i];
        delete[] b2[i];
        delete[] b3[i];
        delete[] b4[i];
        delete[] b5[i];
        delete[] b6[i];
        delete[] sa_sb[i];
        delete[] diff_v[i];
        delete[] diff_p[i];
    }
    delete[] b1;
    delete[] b2;
    delete[] b3;
    delete[] b4;
    delete[] b5;
    delete[] b6;
    delete[] sa_sb;
    delete[] diff_v;
    delete[] diff_p;

    for (i = 0; i < 2 * (uint)size; i++) {
        delete[] mult_buffer1[i];
        delete[] mult_buffer2[i];
        delete[] mult_result[i];
    }
    delete[] mult_buffer1;
    delete[] mult_buffer2;
    delete[] mult_result;
}

#endif // _FLEQZ_HPP_
