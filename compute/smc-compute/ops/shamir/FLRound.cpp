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
#include "FLRound.h"

// Source: Aliasgari et al., "Secure Computation on Floating Point Numbers," 2013
// Protocol FLRound, page 8
//
// Input: A2[4][size] = (v, p, z, s) - float representation
//        mode[size] - rounding mode (0 = toward zero/truncation, 1 = away from zero)
// Output: result[4][size] = rounded float (v', p', z', s')
// Parameters: L = mantissa bits (q in paper), K = exponent bits (k in paper)
//
// This implementation mirrors the RSS FLRound algorithm (no 0.5 addition).
// For FL2Int with mode=0, this provides truncation toward zero.
void doOperation_FLRound(mpz_t **A2, mpz_t **result, mpz_t *mode, int L, int K, int size, int threadID, NodeNetwork net,  SecretShare *ss) {

    mpz_t constPower2L, constPower2L1, const2, constL, constL1;
    mpz_init(constPower2L);
    mpz_init(constPower2L1);
    mpz_init_set_ui(const2, 2);
    mpz_init_set_ui(constL, L);
    mpz_init_set_ui(constL1, L - 1);
    ss->modPow(constPower2L, const2, constL);
    ss->modPow(constPower2L1, const2, constL1);

    // Allocate temporary arrays (Shamir uses [size] arrays of mpz_t)
    mpz_t *a_flag = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b_flag = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *c_flag = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *d_flag = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *V = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *V2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *powM = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *m = (mpz_t *)malloc(sizeof(mpz_t) * size);

    for (int i = 0; i < size; i++) {
        mpz_init(a_flag[i]);
        mpz_init(b_flag[i]);
        mpz_init(c_flag[i]);
        mpz_init(d_flag[i]);
        mpz_init(V[i]);
        mpz_init(V2[i]);
        mpz_init(powM[i]);
        mpz_init(temp1[i]);
        mpz_init(temp2[i]);
        mpz_init(m[i]);
    }

    // Line 1: a = LTZ(p) - check if exponent is negative
    doOperation_LTZ(a_flag, A2[1], K, size, threadID, net, ss);

    // Line 1 continued: b = LT(p, -L+1) = LTZ(p + L - 1)
    // If b=1, the value is very small (< 2^{-L+1}), should round to 0 or ±1
    ss->modAdd(temp1, A2[1], L - 1, size);
    doOperation_LTZ(b_flag, temp1, K, size, threadID, net, ss);

    // Line 2: Compute m = -a*(1-b)*p (shift amount for Mod2MS)
    // When a=1 (p<0) and b=0 (p >= -L+1): m = -p = |p|
    // Otherwise: m = 0
    ss->modSub(temp1, 1, b_flag, size);               // 1 - b
    Mult(temp1, temp1, a_flag, size, threadID, net, ss);  // a * (1 - b)
    Mult(temp1, temp1, A2[1], size, threadID, net, ss);   // a * (1 - b) * p
    ss->modSub(m, (long)0, temp1, size);              // m = -a*(1-b)*p

    // Mod2MS: V2 = v mod 2^m, powM = 2^m
    doOperation_Mod2MS(V2, A2[0], m, powM, L, size, threadID, net, ss);

    // Line 3: c = EQZ(V2) - check if fractional part is zero
    doOperation_EQZ(V2, c_flag, L, size, threadID, net, ss);

    // Line 4: V = v - V2 + (1-c) * powM * mode
    // For mode 0 (toward zero): just subtract V2 (floor the mantissa)
    // For mode 1 (away from zero): add powM to round up (ceil the mantissa)
    ss->modSub(temp2, 1, c_flag, size);               // 1 - c
    Mult(temp1, temp2, powM, size, threadID, net, ss);    // (1-c) * powM
    Mult(temp1, temp1, mode, size, threadID, net, ss);    // (1-c) * powM * mode
    ss->modSub(temp2, A2[0], V2, size);               // v - V2
    ss->modAdd(V, temp2, temp1, size);                // v - V2 + (1-c)*powM*mode

    // Line 5: d = EQ(V, 2^L) = EQZ(V - 2^L) - check for mantissa overflow
    ss->modSub(temp1, V, constPower2L, size);
    doOperation_EQZ(temp1, d_flag, L + 1, size, threadID, net, ss);

    // Line 6: V = d * 2^{L-1} + (1-d) * V
    // If overflow occurred, normalize mantissa to 2^{L-1}
    ss->modSub(temp1, 1, d_flag, size);               // 1 - d
    Mult(temp1, temp1, V, size, threadID, net, ss);       // (1-d) * V
    ss->modMul(temp2, d_flag, constPower2L1, size);       // d * 2^{L-1}
    ss->modAdd(V, temp2, temp1, size);                // d * 2^{L-1} + (1-d) * V

    // Line 7: V = a * ((1-b)*V + b*(mode - s)) + (1-a) * v
    // First: (1-b)*V
    ss->modSub(temp1, 1, b_flag, size);               // 1 - b
    Mult(temp1, temp1, V, size, threadID, net, ss);       // (1-b)*V

    // Second: b*(mode - s)
    ss->modSub(temp2, mode, A2[3], size);             // mode - s
    Mult(temp2, b_flag, temp2, size, threadID, net, ss);  // b*(mode - s)

    // (1-b)*V + b*(mode - s)
    ss->modAdd(temp1, temp1, temp2, size);

    // a * ((1-b)*V + b*(mode - s))
    Mult(temp1, a_flag, temp1, size, threadID, net, ss);

    // (1-a) * v
    ss->modSub(temp2, 1, a_flag, size);               // 1 - a
    Mult(temp2, temp2, A2[0], size, threadID, net, ss);   // (1-a) * v

    // V = a*(...) + (1-a)*v
    ss->modAdd(V, temp1, temp2, size);

    // Line 8: s' = (1 - b*mode) * s
    Mult(temp1, b_flag, mode, size, threadID, net, ss);   // b * mode
    ss->modSub(temp1, 1, temp1, size);                // 1 - b*mode
    Mult(result[3], temp1, A2[3], size, threadID, net, ss);

    // Line 9: z' = OR(EQZ(V), z) = EQZ(V) + z - EQZ(V)*z
    doOperation_EQZ(V, temp1, L, size, threadID, net, ss);
    Mult(temp2, temp1, A2[2], size, threadID, net, ss);   // EQZ(V) * z
    ss->modAdd(temp1, temp1, A2[2], size);            // EQZ(V) + z
    ss->modSub(result[2], temp1, temp2, size);        // EQZ(V) + z - EQZ(V)*z

    // Line 10: v' = V * (1 - z')
    ss->modSub(temp1, 1, result[2], size);
    Mult(result[0], V, temp1, size, threadID, net, ss);

    // Line 11: p' = (p + d*a*(1-b)) * (1 - z')
    // First: d*a*(1-b)
    ss->modSub(temp1, 1, b_flag, size);               // 1 - b
    Mult(temp1, d_flag, temp1, size, threadID, net, ss);  // d*(1-b)
    Mult(temp1, temp1, a_flag, size, threadID, net, ss);  // d*a*(1-b)

    // p + d*a*(1-b)
    ss->modAdd(temp1, A2[1], temp1, size);

    // (p + d*a*(1-b)) * (1 - z')
    ss->modSub(temp2, 1, result[2], size);            // 1 - z'
    Mult(result[1], temp1, temp2, size, threadID, net, ss);

    // Free memory
    mpz_clear(constPower2L);
    mpz_clear(constPower2L1);
    mpz_clear(const2);
    mpz_clear(constL);
    mpz_clear(constL1);

    for (int i = 0; i < size; i++) {
        mpz_clear(a_flag[i]);
        mpz_clear(b_flag[i]);
        mpz_clear(c_flag[i]);
        mpz_clear(d_flag[i]);
        mpz_clear(V[i]);
        mpz_clear(V2[i]);
        mpz_clear(powM[i]);
        mpz_clear(temp1[i]);
        mpz_clear(temp2[i]);
        mpz_clear(m[i]);
    }

    free(a_flag);
    free(b_flag);
    free(c_flag);
    free(d_flag);
    free(V);
    free(V2);
    free(powM);
    free(temp1);
    free(temp2);
    free(m);
}
