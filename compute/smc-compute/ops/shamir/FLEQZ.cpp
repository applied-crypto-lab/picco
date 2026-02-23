/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame
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
#include "FLEQZ.h"

/*
 * Float Equality (FLEQZ) - Checks if two floating-point numbers are equal
 *
 * Source: Aliasgari et al., "Secure Computation on Floating Point Numbers," 2013
 *
 * Float representation: [mantissa, exponent, zero_flag, sign]
 *   - A[0] = mantissa (v)
 *   - A[1] = exponent (p)
 *   - A[2] = zero flag (z)
 *   - A[3] = sign (s)
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
 * Optimized multiplication rounds (for private-private comparison):
 *   Round 1: s_a * s_b, b1 * b2   (2 mults in parallel)
 *   Round 2: (b1*b2) * b3, z_a * z_b   (2 mults in parallel)
 *   Round 3: b4 * b5              (1 mult for OR: a+b-a*b)
 */

// Private-Private comparison: A == B (both private floats)
void doOperation_FLEQZ(mpz_t **A1, mpz_t **B1, mpz_t *result, int K, int L, int size, int threadID, NodeNetwork net, SecretShare *ss) {

    // Convert input format: A1[element][component] -> A[component][element]
    mpz_t **A = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    mpz_t **B = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    for (int i = 0; i < 4; i++) {
        A[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        B[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++) {
            mpz_init_set(A[i][j], A1[j][i]);
            mpz_init_set(B[i][j], B1[j][i]);
        }
    }

    mpz_t const1, const2;
    mpz_init_set_ui(const1, 1);
    mpz_init_set_ui(const2, 2);

    mpz_t *b1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b3 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b4 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b5 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b6 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *sa_sb = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *diff = (mpz_t *)malloc(sizeof(mpz_t) * size);

    // Buffers for batched multiplications - max 2*size
    mpz_t *mult_buffer1 = (mpz_t *)malloc(sizeof(mpz_t) * size * 2);
    mpz_t *mult_buffer2 = (mpz_t *)malloc(sizeof(mpz_t) * size * 2);
    mpz_t *mult_result = (mpz_t *)malloc(sizeof(mpz_t) * size * 2);

    for (int i = 0; i < size; i++) {
        mpz_init(b1[i]);
        mpz_init(b2[i]);
        mpz_init(b3[i]);
        mpz_init(b4[i]);
        mpz_init(b5[i]);
        mpz_init(b6[i]);
        mpz_init(sa_sb[i]);
        mpz_init(diff[i]);
    }

    for (int i = 0; i < 2 * size; i++) {
        mpz_init(mult_buffer1[i]);
        mpz_init(mult_buffer2[i]);
        mpz_init(mult_result[i]);
    }

    // Step 0: Compute EQZ for mantissa and exponent
    // b1 = EQZ(v_a - v_b)
    ss->modSub(diff, A[0], B[0], size);
    doOperation_EQZ(diff, b1, K, size, threadID, net, ss);

    // b2 = EQZ(p_a - p_b)
    ss->modSub(diff, A[1], B[1], size);
    doOperation_EQZ(diff, b2, L, size, threadID, net, ss);

    // Round 1: Compute s_a * s_b and b1 * b2 in parallel
    for (int i = 0; i < size; i++) {
        mpz_set(mult_buffer1[i], A[3][i]);           // s_a
        mpz_set(mult_buffer2[i], B[3][i]);           // s_b
        mpz_set(mult_buffer1[i + size], b1[i]);      // b1
        mpz_set(mult_buffer2[i + size], b2[i]);      // b2
    }

    Mult(mult_result, mult_buffer1, mult_buffer2, 2 * size, threadID, net, ss);

    // Extract results and compute b3 = 1 - XOR(s_a, s_b) = 1 - s_a - s_b + 2*s_a*s_b
    for (int i = 0; i < size; i++) {
        mpz_set(sa_sb[i], mult_result[i]);           // s_a * s_b
        mpz_set(b6[i], mult_result[i + size]);       // b1 * b2

        // b3 = 1 - s_a - s_b + 2*s_a*s_b
        mpz_set(b3[i], const1);
        ss->modSub(b3[i], b3[i], A[3][i]);
        ss->modSub(b3[i], b3[i], B[3][i]);
        mpz_t temp;
        mpz_init(temp);
        ss->modMul(temp, sa_sb[i], const2);
        ss->modAdd(b3[i], b3[i], temp);
        mpz_clear(temp);
    }

    // Round 2: Compute (b1*b2) * b3 = b4 and z_a * z_b = b5 in parallel
    for (int i = 0; i < size; i++) {
        mpz_set(mult_buffer1[i], b6[i]);             // b1 * b2
        mpz_set(mult_buffer2[i], b3[i]);             // b3
        mpz_set(mult_buffer1[i + size], A[2][i]);    // z_a
        mpz_set(mult_buffer2[i + size], B[2][i]);    // z_b
    }

    Mult(mult_result, mult_buffer1, mult_buffer2, 2 * size, threadID, net, ss);

    // Extract results: b4 = (b1*b2)*b3, b5 = z_a * z_b
    for (int i = 0; i < size; i++) {
        mpz_set(b4[i], mult_result[i]);              // b4 = b1 * b2 * b3
        mpz_set(b5[i], mult_result[i + size]);       // b5 = z_a * z_b
    }

    // Round 3: Compute OR(b4, b5) = b4 + b5 - b4*b5
    Mult(mult_result, b4, b5, size, threadID, net, ss);

    // Final result: result = b4 + b5 - b4*b5
    for (int i = 0; i < size; i++) {
        ss->modAdd(result[i], b4[i], b5[i]);
        ss->modSub(result[i], result[i], mult_result[i]);
    }

    // Free memory
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < size; j++) {
            mpz_clear(A[i][j]);
            mpz_clear(B[i][j]);
        }
        free(A[i]);
        free(B[i]);
    }
    free(A);
    free(B);

    mpz_clear(const1);
    mpz_clear(const2);

    for (int i = 0; i < size; i++) {
        mpz_clear(b1[i]);
        mpz_clear(b2[i]);
        mpz_clear(b3[i]);
        mpz_clear(b4[i]);
        mpz_clear(b5[i]);
        mpz_clear(b6[i]);
        mpz_clear(sa_sb[i]);
        mpz_clear(diff[i]);
    }

    for (int i = 0; i < 2 * size; i++) {
        mpz_clear(mult_buffer1[i]);
        mpz_clear(mult_buffer2[i]);
        mpz_clear(mult_result[i]);
    }

    free(b1);
    free(b2);
    free(b3);
    free(b4);
    free(b5);
    free(b6);
    free(sa_sb);
    free(diff);
    free(mult_buffer1);
    free(mult_buffer2);
    free(mult_result);
}

// Private-Public comparison: A (private) == B (public int)
// When B is public, multiplications with B become modMul (no communication needed)
void doOperation_FLEQZ(mpz_t **A1, int **B1, mpz_t *result, int K, int L, int size, int threadID, NodeNetwork net, SecretShare *ss) {

    // Convert input format
    mpz_t **A = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    int **B = (int **)malloc(sizeof(int *) * 4);
    for (int i = 0; i < 4; i++) {
        A[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        B[i] = (int *)malloc(sizeof(int) * size);
        for (int j = 0; j < size; j++) {
            mpz_init_set(A[i][j], A1[j][i]);
            B[i][j] = B1[j][i];
        }
    }

    mpz_t const1, const2;
    mpz_init_set_ui(const1, 1);
    mpz_init_set_ui(const2, 2);

    mpz_t *b1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b3 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b4 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b5 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *diff = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp = (mpz_t *)malloc(sizeof(mpz_t) * size);

    for (int i = 0; i < size; i++) {
        mpz_init(b1[i]);
        mpz_init(b2[i]);
        mpz_init(b3[i]);
        mpz_init(b4[i]);
        mpz_init(b5[i]);
        mpz_init(diff[i]);
        mpz_init(temp[i]);
    }

    // Step 0: Compute EQZ
    // b1 = EQZ(v_a - v_b)
    ss->modSub(diff, A[0], B[0], size);
    doOperation_EQZ(diff, b1, K, size, threadID, net, ss);

    // b2 = EQZ(p_a - p_b)
    ss->modSub(diff, A[1], B[1], size);
    doOperation_EQZ(diff, b2, L, size, threadID, net, ss);

    // Compute b3 = 1 - XOR(s_a, s_b) where s_b is public
    // Since s_b is public, s_a * s_b is just modMul (no communication)
    ss->modAdd(temp, A[3], B[3], size);         // s_a + s_b
    ss->modMul(b3, A[3], B[3], size);           // s_a * s_b (public mul)
    ss->modMul(b3, b3, const2, size);           // 2 * s_a * s_b
    ss->modSub(temp, temp, b3, size);           // s_a + s_b - 2*s_a*s_b = XOR
    ss->modSub(b3, const1, temp, size);         // 1 - XOR = b3

    // Compute b5 = z_a * z_b (public mul)
    ss->modMul(b5, A[2], B[2], size);

    // Round 1: b1 * b2
    Mult(temp, b1, b2, size, threadID, net, ss);

    // Round 2: (b1*b2) * b3
    Mult(b4, temp, b3, size, threadID, net, ss);

    // Round 3: OR(b4, b5) = b4 + b5 - b4*b5
    Mult(temp, b4, b5, size, threadID, net, ss);
    for (int i = 0; i < size; i++) {
        ss->modAdd(result[i], b4[i], b5[i]);
        ss->modSub(result[i], result[i], temp[i]);
    }

    // Free memory
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < size; j++) {
            mpz_clear(A[i][j]);
        }
        free(A[i]);
        free(B[i]);
    }
    free(A);
    free(B);

    mpz_clear(const1);
    mpz_clear(const2);

    for (int i = 0; i < size; i++) {
        mpz_clear(b1[i]);
        mpz_clear(b2[i]);
        mpz_clear(b3[i]);
        mpz_clear(b4[i]);
        mpz_clear(b5[i]);
        mpz_clear(diff[i]);
        mpz_clear(temp[i]);
    }

    free(b1);
    free(b2);
    free(b3);
    free(b4);
    free(b5);
    free(diff);
    free(temp);
}

// Public-Private comparison: A (public int) == B (private)
// When A is public, multiplications with A become modMul (no communication needed)
void doOperation_FLEQZ(int **A1, mpz_t **B1, mpz_t *result, int K, int L, int size, int threadID, NodeNetwork net, SecretShare *ss) {

    // Convert input format
    int **A = (int **)malloc(sizeof(int *) * 4);
    mpz_t **B = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    for (int i = 0; i < 4; i++) {
        A[i] = (int *)malloc(sizeof(int) * size);
        B[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++) {
            A[i][j] = A1[j][i];
            mpz_init_set(B[i][j], B1[j][i]);
        }
    }

    mpz_t const1, const2;
    mpz_init_set_ui(const1, 1);
    mpz_init_set_ui(const2, 2);

    mpz_t *b1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b3 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b4 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b5 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *diff = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp = (mpz_t *)malloc(sizeof(mpz_t) * size);

    for (int i = 0; i < size; i++) {
        mpz_init(b1[i]);
        mpz_init(b2[i]);
        mpz_init(b3[i]);
        mpz_init(b4[i]);
        mpz_init(b5[i]);
        mpz_init(diff[i]);
        mpz_init(temp[i]);
    }

    // Step 0: Compute EQZ
    // b1 = EQZ(v_a - v_b)
    ss->modSub(diff, A[0], B[0], size);
    doOperation_EQZ(diff, b1, K, size, threadID, net, ss);

    // b2 = EQZ(p_a - p_b)
    ss->modSub(diff, A[1], B[1], size);
    doOperation_EQZ(diff, b2, L, size, threadID, net, ss);

    // Compute b3 = 1 - XOR(s_a, s_b) where s_a is public
    // Since s_a is public, s_a * s_b is just modMul (no communication)
    ss->modAdd(temp, B[3], A[3], size);         // s_a + s_b
    ss->modMul(b3, B[3], A[3], size);           // s_a * s_b (public mul)
    ss->modMul(b3, b3, const2, size);           // 2 * s_a * s_b
    ss->modSub(temp, temp, b3, size);           // s_a + s_b - 2*s_a*s_b = XOR
    ss->modSub(b3, const1, temp, size);         // 1 - XOR = b3

    // Compute b5 = z_a * z_b (public mul)
    ss->modMul(b5, B[2], A[2], size);

    // Round 1: b1 * b2
    Mult(temp, b1, b2, size, threadID, net, ss);

    // Round 2: (b1*b2) * b3
    Mult(b4, temp, b3, size, threadID, net, ss);

    // Round 3: OR(b4, b5) = b4 + b5 - b4*b5
    Mult(temp, b4, b5, size, threadID, net, ss);
    for (int i = 0; i < size; i++) {
        ss->modAdd(result[i], b4[i], b5[i]);
        ss->modSub(result[i], result[i], temp[i]);
    }

    // Free memory
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < size; j++) {
            mpz_clear(B[i][j]);
        }
        free(A[i]);
        free(B[i]);
    }
    free(A);
    free(B);

    mpz_clear(const1);
    mpz_clear(const2);

    for (int i = 0; i < size; i++) {
        mpz_clear(b1[i]);
        mpz_clear(b2[i]);
        mpz_clear(b3[i]);
        mpz_clear(b4[i]);
        mpz_clear(b5[i]);
        mpz_clear(diff[i]);
        mpz_clear(temp[i]);
    }

    free(b1);
    free(b2);
    free(b3);
    free(b4);
    free(b5);
    free(diff);
    free(temp);
}
