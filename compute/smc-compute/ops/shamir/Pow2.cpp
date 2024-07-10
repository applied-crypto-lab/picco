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
#include "Pow2.h"

// Source: Aliasgari et al., "Secure Computation on Floating Point Numbers," 2013
// Protocol Pow2, page 4
void doOperation_Pow2(mpz_t *result, mpz_t *A, int L, int size, int threadID, NodeNetwork net, SecretShare *ss) {
    int M = ceil(log2(L));
    mpz_t **S = (mpz_t **)malloc(sizeof(mpz_t *) * (M + 1));
    mpz_t *temp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t const1, const2, constI, pow2M;
    // initialization
    mpz_init_set_ui(const1, 1);
    mpz_init_set_ui(const2, 2);
    mpz_init(constI);
    mpz_init(pow2M);
    for (int i = 0; i < M + 1; i++) {
        S[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++)
            mpz_init(S[i][j]);
    }
    for (int i = 0; i < size; i++)
        mpz_init(temp[i]);
    // start computation
    doOperation_bitDec(S, A, M, M, size, threadID, net, ss);
    for (int i = 0; i < M; i++) {
        mpz_set_ui(constI, pow(2, i));
        ss->modPow(pow2M, const2, constI);
        ss->modMul(temp, S[i], pow2M, size);
        ss->modSub(S[i], temp, S[i], size);
        ss->modAdd(S[i], S[i], const1, size);
    }
    doOperation_PrefixMult(S, S, M, size, threadID, net, ss);
    ss->copy(S[M - 1], result, size);
    // free the memory
    for (int i = 0; i < M + 1; i++) {
        for (int j = 0; j < size; j++)
            mpz_clear(S[i][j]);
        free(S[i]);
    }
    free(S);
    for (int i = 0; i < size; i++)
        mpz_clear(temp[i]);
    free(temp);
    mpz_clear(const1);
    mpz_clear(const2);
    mpz_clear(constI);
    mpz_clear(pow2M);
}
