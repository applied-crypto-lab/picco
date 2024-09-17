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

// first input is private, second is public
void BitAnd(mpz_t *A, int *B, mpz_t *result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, SecretShare *ss) {
    if ((alen == 1) and (blen == 1)) {
        ss->modMul(result, A, B, size);
    } else {
        mpz_t two, pow_two;
        mpz_init_set_ui(two, 2);
        mpz_init(pow_two);
        int M = std::min(alen, blen);

        // will store the product of A's bits and B's bits
        mpz_t *tmp = (mpz_t *)malloc(sizeof(mpz_t) * M * size);
        for (int i = 0; i < M * size; i++) {
            mpz_init(tmp[i]);
        }
        if (alen > 1) {
            // we only need to bitDecompose A if alen > 1
            mpz_t **S = (mpz_t **)malloc(sizeof(mpz_t *) * (M + 1));
            for (int i = 0; i < (M + 1); i++) {
                S[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
                for (int j = 0; j < size; j++)
                    mpz_init(S[i][j]);
            }
            doOperation_bitDec(S, A, M, M, size, threadID, net, ss);
            for (size_t i = 0; i < M; i++)
                for (size_t j = 0; j < size; j++)
                    mpz_set(tmp[i * size + j], S[i][j]);
            for (int i = 0; i < (M + 1); i++) {
                for (int j = 0; j < size; j++)
                    mpz_clear(S[i][j]);
                free(S[i]);
            }
            free(S);
        } else {
        }

        // reassembly
        for (int i = 0; i < M; ++i) {
            ss->modPow(pow_two, two, i);
            for (int j = 0; j < size; ++j) {
                ss->modMul(pow_two, pow_two, tmp[i * size + j]);
                ss->modAdd(result[j], result[j], pow_two);
            }
        }
    }
}

// bitwise AND
// we guarantee (in SMC_utils) that the first argument is always the one with the longer bitlength (alen > blen).
// If in the calling function blen > alen, we reverse the order of the inputs
void BitAnd(mpz_t *A, mpz_t *B, mpz_t *result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, SecretShare *ss) {
    if ((alen == 1) and (blen == 1)) {
        Mult(result, A, B, size, threadID, net, ss);
    } else {

        int M = std::min(alen, blen);
        int sz_offset = (alen > 1 and blen > 1) ? 2 : 1;

        mpz_t two, pow_two;
        mpz_init_set_ui(two, 2);
        mpz_init(pow_two);

        mpz_t *accum = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int i = 0; i < size; ++i)
            mpz_init(accum[i]);

        mpz_t **S = (mpz_t **)malloc(sizeof(mpz_t *) * (M + 1));
        for (int i = 0; i < (M + 1); i++) {
            S[i] = (mpz_t *)malloc(sizeof(mpz_t) * sz_offset * size);
            for (int j = 0; j < sz_offset * size; j++)
                mpz_init(S[i][j]);
        }

        mpz_t *buffer = (mpz_t *)malloc(sizeof(mpz_t) * sz_offset * size);
        for (int i = 0; i < sz_offset * size; i++)
            mpz_init(buffer[i]);

        mpz_t *C = (mpz_t *)malloc(sizeof(mpz_t) * M * size);
        mpz_t *mul_buff_A = (mpz_t *)malloc(sizeof(mpz_t) * M * size);
        mpz_t *mul_buff_B = (mpz_t *)malloc(sizeof(mpz_t) * M * size);
        // this stays the same regardless of whether a or b is a single bit
        for (int i = 0; i < M * size; i++) {
            mpz_init(C[i]);
            mpz_init(mul_buff_A[i]);
            mpz_init(mul_buff_B[i]);
        }

        // moving into buffer
        // A will always be in the first position of the buffer
        // if alen = 1 (and blen > 1, which is only reachable in this block), then we only need to bit-decompose B
        if (alen > 1) {
            for (int i = 0; i < size; i++)
                mpz_set(buffer[i], A[i]);
        }
        if (blen > 1) {
            int offset = (alen == 1) ? 0 : 1;
            for (int i = 0; i < size; i++)
                mpz_set(buffer[offset * size + i], B[i]);
        }

        doOperation_bitDec(S, buffer, M, M, sz_offset * size, threadID, net, ss);

        if (alen > 1) {
            for (size_t i = 0; i < M; i++)
                for (size_t j = 0; j < size; j++)
                    mpz_set(mul_buff_A[i * size + j], S[i][j]);
        } else {
            // we didn't bit-decompose A, and can just move A into the buffer directly
            for (size_t j = 0; j < size; j++)
                mpz_set(mul_buff_A[j], A[j]);
        }

        if (blen > 1) {
            int offset = (alen == 1) ? 0 : 1;
            for (size_t i = 0; i < M; i++)
                for (size_t j = 0; j < size; j++)
                    mpz_set(mul_buff_B[i * size + j], S[i][offset * size + j]);
        } else {
            // we didn't bit-decompose B, and can just move B into the buffer directly
            for (size_t j = 0; j < size; j++)
                mpz_set(mul_buff_B[j], B[j]);
        }

        // a*b (only "operation" needed for bitAND)
        Mult(mul_buff_A, mul_buff_A, mul_buff_B, M * size, threadID, net, ss);

        // reassembly
        for (int i = 0; i < M; ++i) {
            ss->modPow(pow_two, two, i);
            for (int j = 0; j < size; ++j) {
                ss->modMul(pow_two, pow_two, mul_buff_A[i * size + j]);
                ss->modAdd(result[j], result[j], pow_two);
            }
        }

        // free the memory
        for (int i = 0; i < sz_offset * size; ++i)
            mpz_clear(buffer[i]);
        for (int i = 0; i < M * size; ++i) {
            mpz_clear(C[i]);
            mpz_clear(mul_buff_A[i]);
            mpz_clear(mul_buff_B[i]);
        }
        free(buffer);
        free(C);
        free(mul_buff_A);
        free(mul_buff_B);
        mpz_clear(two);
        mpz_clear(pow_two);

        // freeing
        for (int i = 0; i < (M + 1); i++) {
            for (int j = 0; j < sz_offset * size; j++)
                mpz_clear(S[i][j]);
            free(S[i]);
        }
        free(S);
    }
}

void BitOr(mpz_t *A, int *B, mpz_t *result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, SecretShare *ss) {
}

// bitwise OR, virtually identical to XOR without multiplying the intermediary product by "2"
void BitOr(mpz_t *A, mpz_t *B, mpz_t *result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, SecretShare *ss) {
    if ((alen == 1) and (blen == 1)) {
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
    } else {

        int M = std::min(alen, blen);
        int sz_offset = (alen > 1 and blen > 1) ? 2 : 1;

        mpz_t two, pow_two;
        mpz_init_set_ui(two, 2);
        mpz_init(pow_two);

        mpz_t *accum = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int i = 0; i < size; ++i)
            mpz_init(accum[i]);

        mpz_t **S = (mpz_t **)malloc(sizeof(mpz_t *) * (M + 1));
        for (int i = 0; i < (M + 1); i++) {
            S[i] = (mpz_t *)malloc(sizeof(mpz_t) * sz_offset * size);
            for (int j = 0; j < sz_offset * size; j++)
                mpz_init(S[i][j]);
        }
        mpz_t *buffer = (mpz_t *)malloc(sizeof(mpz_t) * sz_offset * size);
        for (int i = 0; i < sz_offset * size; i++)
            mpz_init(buffer[i]);

        mpz_t *C = (mpz_t *)malloc(sizeof(mpz_t) * M * size);
        mpz_t *tmp = (mpz_t *)malloc(sizeof(mpz_t) * M * size);
        mpz_t *mul_buff_A = (mpz_t *)malloc(sizeof(mpz_t) * M * size);
        mpz_t *mul_buff_B = (mpz_t *)malloc(sizeof(mpz_t) * M * size);
        // this stays the same regardless of whether a or b is a single bit
        for (int i = 0; i < M * size; i++) {
            mpz_init(C[i]);
            mpz_init(tmp[i]);
            mpz_init(mul_buff_A[i]);
            mpz_init(mul_buff_B[i]);
        }

        // moving into buffer
        // A will always be in the first position of the buffer
        // if alen = 1 (and blen > 1, which is only reachable in this block), then we only need to bit-decompose B
        if (alen > 1) {
            for (int i = 0; i < size; i++)
                mpz_set(buffer[i], A[i]);
        }
        if (blen > 1) {
            int offset = (alen == 1) ? 0 : 1;
            for (int i = 0; i < size; i++)
                mpz_set(buffer[offset * size + i], B[i]);
        }

        doOperation_bitDec(S, buffer, M, M, sz_offset * size, threadID, net, ss);

        if (alen > 1) {
            for (size_t i = 0; i < M; i++)
                for (size_t j = 0; j < size; j++)
                    mpz_set(mul_buff_A[i * size + j], S[i][j]);
        } else {
            // we didn't bit-decompose A, and can just move A into the buffer directly
            for (size_t j = 0; j < size; j++)
                mpz_set(mul_buff_A[j], A[j]);
        }

        if (blen > 1) {
            int offset = (alen == 1) ? 0 : 1;
            for (size_t i = 0; i < M; i++)
                for (size_t j = 0; j < size; j++)
                    mpz_set(mul_buff_B[i * size + j], S[i][offset * size + j]);
        } else {
            // we didn't bit-decompose B, and can just move B into the buffer directly
            for (size_t j = 0; j < size; j++)
                mpz_set(mul_buff_B[j], B[j]);
        }

        // a + b - a*b
        Mult(mul_buff_A, mul_buff_A, mul_buff_B, M * size, threadID, net, ss);
        ss->modAdd(tmp, mul_buff_A, mul_buff_B, M * size);
        ss->modSub(C, tmp, C, M * size);

        // reassembly
        for (int i = 0; i < M; ++i) {
            ss->modPow(pow_two, two, i);
            for (int j = 0; j < size; ++j) {
                ss->modMul(pow_two, pow_two, mul_buff_A[i * size + j]);
                ss->modAdd(result[j], result[j], pow_two);
            }
        }

        // this is only needed for XOR and OR
        if (alen > blen) {
            for (int i = 0; i < M; ++i) {
                ss->modPow(pow_two, two, i);
                for (int j = 0; j < size; ++j) {
                    ss->modMul(pow_two, pow_two, S[i][j]);
                    ss->modAdd(accum[j], accum[j], pow_two);
                }
            }
            ss->modSub(accum, A, accum, size);
        }
        ss->modAdd(result, result, accum, size);

        // free the memory
        for (int i = 0; i < sz_offset * size; ++i)
            mpz_clear(buffer[i]);
        for (int i = 0; i < M * size; ++i) {
            mpz_clear(C[i]);
            mpz_clear(tmp[i]);
            mpz_clear(mul_buff_A[i]);
            mpz_clear(mul_buff_B[i]);
        }
        free(buffer);
        free(C);
        free(tmp);
        free(mul_buff_A);
        free(mul_buff_B);
        mpz_clear(two);
        mpz_clear(pow_two);

        // freeing
        for (int i = 0; i < (M + 1); i++) {
            for (int j = 0; j < sz_offset * size; j++)
                mpz_clear(S[i][j]);
            free(S[i]);
        }
        free(S);
    }
}

// bitwise XOR
// work in progress, need to test once compiler/ is consistent with compute/
void BitXor(mpz_t *A, mpz_t *B, mpz_t *result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, SecretShare *ss) {

    if ((alen == 1) and (blen == 1)) {
        mpz_t *C = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int i = 0; i < size; ++i)
            mpz_init(C[i]);
        // (a+b) - 2*ab
        Mult(C, A, B, size, threadID, net, ss);
        ss->modMul(C, C, 2, size);
        ss->modAdd(result, A, B, size);
        ss->modSub(result, result, C, size);
        // free the memory
        for (int i = 0; i < size; ++i)
            mpz_clear(C[i]);
        free(C);
    } else {
        int M = std::min(alen, blen);
        int sz_offset = (alen > 1 and blen > 1) ? 2 : 1;

        mpz_t two, pow_two;
        mpz_init_set_ui(two, 2);
        mpz_init(pow_two);

        mpz_t *accum = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int i = 0; i < size; ++i)
            mpz_init(accum[i]);

        mpz_t **S = (mpz_t **)malloc(sizeof(mpz_t *) * (M + 1));
        for (int i = 0; i < (M + 1); i++) {
            S[i] = (mpz_t *)malloc(sizeof(mpz_t) * sz_offset * size);
            for (int j = 0; j < sz_offset * size; j++)
                mpz_init(S[i][j]);
        }
        mpz_t *buffer = (mpz_t *)malloc(sizeof(mpz_t) * sz_offset * size);
        for (int i = 0; i < sz_offset * size; i++)
            mpz_init(buffer[i]);

        mpz_t *C = (mpz_t *)malloc(sizeof(mpz_t) * M * size);
        mpz_t *tmp = (mpz_t *)malloc(sizeof(mpz_t) * M * size);
        mpz_t *mul_buff_A = (mpz_t *)malloc(sizeof(mpz_t) * M * size);
        mpz_t *mul_buff_B = (mpz_t *)malloc(sizeof(mpz_t) * M * size);
        // this stays the same regardless of whether a or b is a single bit
        for (int i = 0; i < M * size; i++) {
            mpz_init(C[i]);
            mpz_init(tmp[i]);
            mpz_init(mul_buff_A[i]);
            mpz_init(mul_buff_B[i]);
        }

        // moving into buffer
        // A will always be in the first position of the buffer
        // if alen = 1 (and blen > 1, which is only reachable in this block), then we only need to bit-decompose B
        if (alen > 1) {
            for (int i = 0; i < size; i++)
                mpz_set(buffer[i], A[i]);
        }
        if (blen > 1) {
            int offset = (alen == 1) ? 0 : 1;
            for (int i = 0; i < size; i++)
                mpz_set(buffer[offset * size + i], B[i]);
        }

        doOperation_bitDec(S, buffer, M, M, sz_offset * size, threadID, net, ss);

        if (alen > 1) {
            for (size_t i = 0; i < M; i++)
                for (size_t j = 0; j < size; j++)
                    mpz_set(mul_buff_A[i * size + j], S[i][j]);
        } else {
            // we didn't bit-decompose A, and can just move A into the buffer directly
            for (size_t j = 0; j < size; j++)
                mpz_set(mul_buff_A[j], A[j]);
        }

        if (blen > 1) {
            int offset = (alen == 1) ? 0 : 1;
            for (size_t i = 0; i < M; i++)
                for (size_t j = 0; j < size; j++)
                    mpz_set(mul_buff_B[i * size + j], S[i][offset * size + j]);
        } else {
            // we didn't bit-decompose B, and can just move B into the buffer directly
            for (size_t j = 0; j < size; j++)
                mpz_set(mul_buff_B[j], B[j]);
        }

        // a + b - a*b
        Mult(mul_buff_A, mul_buff_A, mul_buff_B, M * size, threadID, net, ss);
        ss->modMul(C, C, 2, M * size);
        ss->modAdd(tmp, mul_buff_A, mul_buff_B, M * size);
        ss->modSub(C, tmp, C, M * size);

        // reassembly
        for (int i = 0; i < M; ++i) {
            ss->modPow(pow_two, two, i);
            for (int j = 0; j < size; ++j) {
                ss->modMul(pow_two, pow_two, mul_buff_A[i * size + j]);
                ss->modAdd(result[j], result[j], pow_two);
            }
        }

        // this is only needed for XOR and OR
        if (alen > blen) {
            for (int i = 0; i < M; ++i) {
                ss->modPow(pow_two, two, i);
                for (int j = 0; j < size; ++j) {
                    ss->modMul(pow_two, pow_two, S[i][j]);
                    ss->modAdd(accum[j], accum[j], pow_two);
                }
            }
            ss->modSub(accum, A, accum, size);
        }
        ss->modAdd(result, result, accum, size);

        // free the memory
        for (int i = 0; i < sz_offset * size; ++i)
            mpz_clear(buffer[i]);
        for (int i = 0; i < M * size; ++i) {
            mpz_clear(C[i]);
            mpz_clear(tmp[i]);
            mpz_clear(mul_buff_A[i]);
            mpz_clear(mul_buff_B[i]);
        }
        free(buffer);
        free(C);
        free(tmp);
        free(mul_buff_A);
        free(mul_buff_B);
        mpz_clear(two);
        mpz_clear(pow_two);
        for (int i = 0; i < size; ++i)
            mpz_clear(accum[i]);
        free(accum);

        // freeing
        for (int i = 0; i < (M + 1); i++) {
            for (int j = 0; j < sz_offset * size; j++)
                mpz_clear(S[i][j]);
            free(S[i]);
        }
        free(S);
    }
}

void BitXor(mpz_t *A, int *B, mpz_t *result, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, SecretShare *ss) {
}
