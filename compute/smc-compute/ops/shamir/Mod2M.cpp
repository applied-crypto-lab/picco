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
#include "Mod2M.h"


// Source: Catrina and de Hoogh, "Improved Primites for Secure Multiparty Integer Computation," 2010
// Protocol 3.2 page 7
void doOperation_Mod2M(mpz_t *result, mpz_t *input, int K, int M, int size, int threadID, NodeNetwork net,  SecretShare *ss) {
    mpz_t **R = (mpz_t **)malloc(sizeof(mpz_t *) * (M + 2));
    mpz_t *U = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *C = (mpz_t *)malloc(sizeof(mpz_t) * size);

    // initialization
    mpz_t const2, constM, constK1, pow2M, pow2K1;
    mpz_init_set_ui(const2, 2);
    mpz_init_set_ui(constM, M);
    mpz_init_set_ui(constK1, K - 1);
    mpz_init(pow2M);
    mpz_init(pow2K1);
    for (int i = 0; i < M + 2; i++) {
        R[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++)
            mpz_init(R[i][j]);
    }

    for (int i = 0; i < size; i++) {
        mpz_init(U[i]);
        mpz_init(C[i]);
    }

    ss->modPow(pow2M, const2, constM);
    ss->modPow(pow2K1, const2, constK1);

    // start comutation.
    PRandInt(K, M, size, C, threadID, ss);
    ss->modMul(C, C, pow2M, size);
    PRandM(M, size, R, threadID, net, ss);
    ss->modAdd(C, C, input, size);
    ss->modAdd(C, C, R[M], size);
    ss->modAdd(C, C, pow2K1, size);
    
    Open(C, C, size, threadID, net, ss);

    ss->mod(C, C, pow2M, size);
    doOperation_BitLTC(C, R, U, M, size, threadID, net, ss);
    ss->modMul(U, U, pow2M, size);
    ss->modAdd(result, C, U, size);
    ss->modSub(result, result, R[M], size);

    // free the memory
    for (int i = 0; i < M + 2; i++) {
        for (int j = 0; j < size; j++)
            mpz_clear(R[i][j]);
        free(R[i]);
    }
    free(R);

    for (int i = 0; i < size; i++) {
        mpz_clear(U[i]);
        mpz_clear(C[i]);
    }
    free(U);
    free(C);

    mpz_clear(const2);
    mpz_clear(constK1);
    mpz_clear(constM);
    mpz_clear(pow2M);
    mpz_clear(pow2K1);
}
