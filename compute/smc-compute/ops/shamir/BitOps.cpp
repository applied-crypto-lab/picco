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
#include "BitOps.h"

void LogicalAnd(mpz_t *A, mpz_t *B, mpz_t *result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, SecretShare *ss) {
    Mult(result, A, B, size, threadID, net, ss);
}

void LogicalOr(mpz_t *A, mpz_t *B, mpz_t *result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, SecretShare *ss) {
    mpz_t *C = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; ++i)
        mpz_init(C[i]);
    // (a+b) - ab
    Mult(C, A, B, size, threadID, net, ss);
    ss->modAdd(result, A, B, size);
    ss->modSub(result, result, C, size);
    // free the memory
    for (int i = 0; i < size; ++i)
        mpz_clear(C[i]);
    free(C);
}

// bitwise AND
void BitAnd(mpz_t *A, mpz_t *B, mpz_t *result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, SecretShare *ss) {
    int M = std::max(alen, blen);
    int M_min = std::min(alen, blen);
    // int diff = abs(alen - blen); // the amount of upper bits of the longer value which will just be copied into the result
    mpz_t two, pow_two;
    mpz_init_set_ui(two, 2);
    mpz_init(pow_two);

    mpz_t *C = (mpz_t *)malloc(sizeof(mpz_t) * M * size);
    for (int i = 0; i < M * size; ++i)
        mpz_init(C[i]);
    mpz_t **S = (mpz_t **)malloc(sizeof(mpz_t *) * (M + 1));
    for (int i = 0; i < (M + 1); i++) {
        S[i] = (mpz_t *)malloc(sizeof(mpz_t) * 2 * size);
        for (int j = 0; j < 2 * size; j++)
            mpz_init(S[i][j]);
    }
    mpz_t *buffer = (mpz_t *)malloc(sizeof(mpz_t) * 2 * size);
    mpz_t *mul_buff_A = (mpz_t *)malloc(sizeof(mpz_t) * M_min * size);
    mpz_t *mul_buff_B = (mpz_t *)malloc(sizeof(mpz_t) * M_min * size);
    mpz_t *tmp = (mpz_t *)malloc(sizeof(mpz_t) * M_min * size);
    for (int i = 0; i < 2 * size; i++)
        mpz_init(buffer[i]);

    for (int i = 0; i < M_min * size; i++) {
        mpz_init(mul_buff_A[i]);
        mpz_init(mul_buff_B[i]);
        mpz_init(tmp[i]);
    }
    // moving into buffer
    for (int i = 0; i < size; i++)
        mpz_set(buffer[i], A[i]);
    for (int i = 0; i < size; i++)
        mpz_set(buffer[size + i], B[i]);

    // doing a full bit decompostion on the inputs (cant be done on M_min, since we need ALL the bits for reassembly)
    doOperation_bitDec(S, buffer, M, M, 2 * size, threadID, net, ss);

    // moving all the "shared" bits into buffers
    // M_min = min(alen, blen)
    for (size_t i = 0; i < M_min; i++) {
        for (size_t j = 0; j < size; j++) {
            mpz_set(mul_buff_A[i * size + j], S[i][j]);
            mpz_set(mul_buff_B[i * size + j], S[i][size + j]);
        }
    }
    int offset = (alen > blen) ? 0 : 1;

    // a*b (only "operation" needed for bitAND)
    Mult(C, mul_buff_A, mul_buff_B, M_min * size, threadID, net, ss);

    // reassembly
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < size; ++j) {
            ss->modPow(pow_two, two, i);
            ss->modMul(pow_two, pow_two, C[i * size + j]);
            ss->modAdd(result[j], result[j], pow_two);
        }
    }

    // free the memory
    for (int i = 0; i < size; ++i)
        mpz_clear(buffer[i]);
    for (int i = 0; i < M * size; ++i) {
        mpz_clear(C[i]);
        mpz_clear(mul_buff_A[i]);
        mpz_clear(mul_buff_B[i]);
        mpz_clear(tmp[i]);
    }
    free(buffer);
    free(C);
    free(mul_buff_A);
    free(mul_buff_B);
    free(tmp);
    mpz_clear(two);
    mpz_clear(pow_two);

    // freeing
    for (int i = 0; i < (M + 1); i++) {
        for (int j = 0; j < 2 * size; j++)
            mpz_clear(S[i][j]);
        free(S[i]);
    }
    free(S);
}

// bitwise OR, virtually identical to XOR without multiplying the intermediary product by "2"
void BitOr(mpz_t *A, mpz_t *B, mpz_t *result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, SecretShare *ss) {
    int M = std::max(alen, blen);
    int M_min = std::min(alen, blen);
    // int diff = abs(alen - blen); // the amount of upper bits of the longer value which will just be copied into the result
    mpz_t two, pow_two;
    mpz_init_set_ui(two, 2);
    mpz_init(pow_two);

    mpz_t *C = (mpz_t *)malloc(sizeof(mpz_t) * M * size);
    for (int i = 0; i < M * size; ++i)
        mpz_init(C[i]);
    mpz_t **S = (mpz_t **)malloc(sizeof(mpz_t *) * (M + 1));
    for (int i = 0; i < (M + 1); i++) {
        S[i] = (mpz_t *)malloc(sizeof(mpz_t) * 2 * size);
        for (int j = 0; j < 2 * size; j++)
            mpz_init(S[i][j]);
    }
    mpz_t *buffer = (mpz_t *)malloc(sizeof(mpz_t) * 2 * size);
    mpz_t *mul_buff_A = (mpz_t *)malloc(sizeof(mpz_t) * M_min * size);
    mpz_t *mul_buff_B = (mpz_t *)malloc(sizeof(mpz_t) * M_min * size);
    mpz_t *tmp = (mpz_t *)malloc(sizeof(mpz_t) * M_min * size);
    for (int i = 0; i < 2 * size; i++)
        mpz_init(buffer[i]);

    for (int i = 0; i < M_min * size; i++) {
        mpz_init(mul_buff_A[i]);
        mpz_init(mul_buff_B[i]);
        mpz_init(tmp[i]);
    }
    // moving into buffer
    for (int i = 0; i < size; i++)
        mpz_set(buffer[i], A[i]);
    for (int i = 0; i < size; i++)
        mpz_set(buffer[size + i], B[i]);

    // doing a full bit decompostion on the inputs (cant be done on M_min, since we need ALL the bits for reassembly)
    doOperation_bitDec(S, buffer, M, M, 2 * size, threadID, net, ss);

    // moving all the "shared" bits into buffers
    // M_min = min(alen, blen)
    for (size_t i = 0; i < M_min; i++) {
        for (size_t j = 0; j < size; j++) {
            mpz_set(mul_buff_A[i * size + j], S[i][j]);
            mpz_set(mul_buff_B[i * size + j], S[i][size + j]);
        }
    }
    int offset = (alen > blen) ? 0 : 1;

    // a + b - a*b
    Mult(C, mul_buff_A, mul_buff_B, M_min * size, threadID, net, ss);
    // ss->modMul(C, C, 2, M_min * size);
    ss->modAdd(tmp, mul_buff_A, mul_buff_B, M_min * size);
    ss->modSub(C, tmp, C, M_min * size);

    if (alen != blen) {
        int offset = (alen > blen) ? 0 : 1;
        for (size_t i = M_min; i < M; i++)
            for (size_t j = 0; j < size; j++)
                mpz_set(C[i * size + j], S[i][offset * size + j]);
    }


    // reassembly
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < size; ++j) {
            ss->modPow(pow_two, two, i);
            ss->modMul(pow_two, pow_two, C[i * size + j]);
            ss->modAdd(result[j], result[j], pow_two);
        }
    }

    // free the memory
    for (int i = 0; i < size; ++i)
        mpz_clear(buffer[i]);
    for (int i = 0; i < M * size; ++i) {
        mpz_clear(C[i]);
        mpz_clear(mul_buff_A[i]);
        mpz_clear(mul_buff_B[i]);
        mpz_clear(tmp[i]);
    }
    free(buffer);
    free(C);
    free(mul_buff_A);
    free(mul_buff_B);
    free(tmp);
    mpz_clear(two);
    mpz_clear(pow_two);

    // freeing
    for (int i = 0; i < (M + 1); i++) {
        for (int j = 0; j < 2 * size; j++)
            mpz_clear(S[i][j]);
        free(S[i]);
    }
    free(S);
}

// bitwise XOR
// work in progress, need to test once compiler/ is consistent with compute/
void BitXor(mpz_t *A, mpz_t *B, mpz_t *result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, SecretShare *ss) {
    int M = std::max(alen, blen);
    int M_min = std::min(alen, blen);
    // int diff = abs(alen - blen); // the amount of upper bits of the longer value which will just be copied into the result
    mpz_t two, pow_two;
    mpz_init_set_ui(two, 2);
    mpz_init(pow_two);

    mpz_t *C = (mpz_t *)malloc(sizeof(mpz_t) * M * size);
    for (int i = 0; i < M * size; ++i)
        mpz_init(C[i]);
    mpz_t **S = (mpz_t **)malloc(sizeof(mpz_t *) * (M + 1));
    for (int i = 0; i < (M + 1); i++) {
        S[i] = (mpz_t *)malloc(sizeof(mpz_t) * 2 * size);
        for (int j = 0; j < 2 * size; j++)
            mpz_init(S[i][j]);
    }
    mpz_t *buffer = (mpz_t *)malloc(sizeof(mpz_t) * 2 * size);
    mpz_t *mul_buff_A = (mpz_t *)malloc(sizeof(mpz_t) * M_min * size);
    mpz_t *mul_buff_B = (mpz_t *)malloc(sizeof(mpz_t) * M_min * size);
    mpz_t *tmp = (mpz_t *)malloc(sizeof(mpz_t) * M_min * size);
    for (int i = 0; i < 2 * size; i++)
        mpz_init(buffer[i]);

    for (int i = 0; i < M_min * size; i++) {
        mpz_init(mul_buff_A[i]);
        mpz_init(mul_buff_B[i]);
        mpz_init(tmp[i]);
    }
    // moving into buffer
    for (int i = 0; i < size; i++)
        mpz_set(buffer[i], A[i]);
    for (int i = 0; i < size; i++)
        mpz_set(buffer[size + i], B[i]);

    // doing a full bit decompostion on the inputs (cant be done on M_min, since we need ALL the bits for reassembly)
    doOperation_bitDec(S, buffer, M, M, 2 * size, threadID, net, ss);

    // moving all the "shared" bits into buffers
    // M_min = min(alen, blen)
    for (size_t i = 0; i < M_min; i++) {
        for (size_t j = 0; j < size; j++) {
            mpz_set(mul_buff_A[i * size + j], S[i][j]);
            mpz_set(mul_buff_B[i * size + j], S[i][size + j]);
        }
    }
    int offset = (alen > blen) ? 0 : 1;

    // a + b - 2(a*b)
    Mult(C, mul_buff_A, mul_buff_B, M_min * size, threadID, net, ss);
    ss->modMul(C, C, 2, M_min * size);
    ss->modAdd(tmp, mul_buff_A, mul_buff_B, M_min * size);
    ss->modSub(C, tmp, C, M_min * size);

    if (alen != blen) {
        int offset = (alen > blen) ? 0 : 1;
        for (size_t i = M_min; i < M; i++)
            for (size_t j = 0; j < size; j++)
                mpz_set(C[i * size + j], S[i][offset * size + j]);
    }

    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < size; ++j) {
            ss->modPow(pow_two, two, i);
            ss->modMul(pow_two, pow_two, C[i * size + j]);
            ss->modAdd(result[j], result[j], pow_two);
        }
    }

    // free the memory
    for (int i = 0; i < size; ++i)
        mpz_clear(buffer[i]);
    for (int i = 0; i < M * size; ++i) {
        mpz_clear(C[i]);
        mpz_clear(mul_buff_A[i]);
        mpz_clear(mul_buff_B[i]);
        mpz_clear(tmp[i]);
    }
    free(buffer);
    free(C);
    free(mul_buff_A);
    free(mul_buff_B);
    free(tmp);
    mpz_clear(two);
    mpz_clear(pow_two);

    // freeing
    for (int i = 0; i < (M + 1); i++) {
        for (int j = 0; j < 2 * size; j++)
            mpz_clear(S[i][j]);
        free(S[i]);
    }
    free(S);
}
