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
#include "FLMult.h"

// Source: Aliasgari et al., "Secure Computation on Floating Point Numbers," 2013
// Protocol FLMult, page 6
/*A[0](B[0]) contains a value V*/
/*A[1](B[1]) contains a power P*/
/*A[2](B[2]) contains a zero bit Z*/
/*A[3](B[3]) contains a sign bit S*/
void doOperation_FLMult(mpz_t **A2, mpz_t **B1, mpz_t **result1, int K, int size, int threadID, NodeNetwork net,  SecretShare *ss) {

    mpz_t **A = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    mpz_t **B = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    mpz_t **result = (mpz_t **)malloc(sizeof(mpz_t *) * 4);

    for (int i = 0; i < 4; i++) {
        A[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        B[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        result[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++) {
            mpz_init_set(A[i][j], A2[j][i]);
            mpz_init_set(B[i][j], B1[j][i]);
            mpz_init_set(result[i][j], result1[j][i]);
        }
    }

    mpz_t const1, const2, constL, constP2L;
    mpz_t *V = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *A1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp4 = (mpz_t *)malloc(sizeof(mpz_t) * size * 3);
    mpz_t *temp5 = (mpz_t *)malloc(sizeof(mpz_t) * size * 3);

    for (int i = 0; i < size; i++) {
        mpz_init(V[i]);
        mpz_init(A1[i]);
        mpz_init(temp1[i]);
        mpz_init(temp2[i]);
    }

    mpz_init_set_ui(const1, 1);
    mpz_init_set_ui(const2, 2);
    mpz_init_set_ui(constL, K);
    mpz_init(constP2L);
    ss->modPow(constP2L, const2, constL);

    for (int i = 0; i < 3 * size; i++) {
        mpz_init(temp4[i]);
        mpz_init(temp5[i]);
    }

    /*squeeze A[0], A[2], A[3] into the vector temp4*/
    /*squeeze B[0], B[2], B[3] into the vector temp5*/
    for (int i = 0; i < 3 * size; i++) {
        if (i < size) {
            mpz_set(temp4[i], A[0][i]);
            mpz_set(temp5[i], B[0][i]);
        } else if (i >= size && i < 2 * size) {
            mpz_set(temp4[i], A[2][i - size]);
            mpz_set(temp5[i], B[2][i - size]);
        } else {
            mpz_set(temp4[i], A[3][i - size * 2]);
            mpz_set(temp5[i], B[3][i - size * 2]);
        }
    }

    Mult(temp5, temp4, temp5, size * 3, threadID, net, ss);
    for (int i = 0; i < 3 * size; i++)
        mpz_clear(temp4[i]);
    free(temp4);

    /*computes the zero bit Z*/
    for (int i = 0; i < size; i++)
        mpz_set(temp1[i], temp5[i + size]);
    ss->modAdd(temp2, A[2], B[2], size);
    ss->modSub(result[2], temp2, temp1, size);

    /*computes the sign bit S using A[3] and B[3]*/
    for (int i = 0; i < size; i++)
        mpz_set(temp1[i], temp5[i + size * 2]);
    ss->modAdd(temp2, A[3], B[3], size);
    ss->modMul(temp1, temp1, const2, size);
    ss->modSub(result[3], temp2, temp1, size);

    /*compute v1 * v2*/
    for (int i = 0; i < size; i++)
        mpz_set(V[i], temp5[i]);
    doOperation_Trunc(V, V, 2 * K, K - 1, size, threadID, net, ss);
    ss->modSub(temp1, V, constP2L, size);
    doOperation_LTZ(A1, temp1, K + 1, size, threadID, net, ss);

    Mult(temp1, V, A1, size, threadID, net, ss);
    ss->modMul(temp2, temp1, const2, size);

    ss->modSub(temp1, V, temp1, size);
    ss->modAdd(temp1, temp1, temp2, size);
    doOperation_Trunc(result[0], temp1, K + 1, 1, size, threadID, net, ss);

    /*computes the power*/
    ss->modSub(temp1, const1, result[2], size);
    ss->modSub(temp2, constL, A1, size);
    ss->modAdd(temp2, temp2, B[1], size);
    ss->modAdd(temp2, temp2, A[1], size);
    Mult(result[1], temp2, temp1, size, threadID, net, ss);

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < size; j++)
            mpz_set(result1[j][i], result[i][j]);

    // free the memory;
    mpz_clear(const1);
    mpz_clear(const2);
    mpz_clear(constL);
    mpz_clear(constP2L);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < size; j++) {
            mpz_clear(A[i][j]);
            mpz_clear(B[i][j]);
            mpz_clear(result[i][j]);
        }
        free(A[i]);
        free(B[i]);
        free(result[i]);
    }
    free(A);
    free(B);
    free(result);

    for (int i = 0; i < size; i++) {
        mpz_clear(V[i]);
        mpz_clear(A1[i]);
        mpz_clear(temp1[i]);
        mpz_clear(temp2[i]);
    }
    free(V);
    free(A1);
    free(temp1);
    free(temp2);

    for (int i = 0; i < 3 * size; i++)
        mpz_clear(temp5[i]);
    free(temp5);
}
