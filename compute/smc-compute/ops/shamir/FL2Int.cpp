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

#include "FL2Int.h"

// Source: Aliasgari et al., "Secure Computation on Floating Point Numbers," 2013
// Protocol FL2Int, page 9
void doOperation_FL2Int(mpz_t **values1, mpz_t *results, int L, int K, int gamma, int size, int threadID, NodeNetwork net,  SecretShare *ss) {

    mpz_t **values = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    mpz_t **valuesP = (mpz_t **)malloc(sizeof(mpz_t) * 4);
    for (int i = 0; i < 4; i++) {
        values[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        valuesP[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++) {
            mpz_init_set(values[i][j], values1[j][i]);
            mpz_init(valuesP[i][j]);
        }
    }
    mpz_t *modes = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *a = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *c = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *u = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *pow2 = (mpz_t *)malloc(sizeof(mpz_t) * size);

    for (int i = 0; i < size; i++) {
        mpz_init_set_ui(modes[i], 0);
        mpz_init(temp1[i]);
        mpz_init(temp2[i]);
        mpz_init(a[i]);
        mpz_init(b[i]);
        mpz_init(c[i]);
        mpz_init(u[i]);
        mpz_init(pow2[i]);
    }

    doOperation_FLRound(values, valuesP, modes, L, K, size, threadID, net, ss);
    // line 2
    ss->modSub(temp1, valuesP[1], gamma - 1, size);
    doOperation_LTZ(a, temp1, K, size, threadID, net, ss);
    // line 3
    ss->modSub(temp1, gamma - L - 1, valuesP[1], size);
    doOperation_LTZ(b, temp1, K, size, threadID, net, ss);

    // line 4
    doOperation_LTZ(c, valuesP[1], K, size, threadID, net, ss);

    // line 5
    ss->modSub(temp1, gamma - 1, valuesP[1], size);
    ss->modSub(temp2, 1, c, size);
    Mult(temp1, temp1, temp2, size, threadID, net, ss);
    Mult(temp1, temp1, b, size, threadID, net, ss);
    Mult(temp2, temp1, a, size, threadID, net, ss);

    // line 6
    doOperation_Mod2MS(u, valuesP[0], temp2, temp1, L, size, threadID, net, ss);
    // line 7
    Mult(temp1, b, c, size, threadID, net, ss);
    ss->modAdd(temp1, temp1, 1, size);
    ss->modSub(temp1, temp1, b, size);
    Mult(temp1, temp1, valuesP[0], size, threadID, net, ss);

    ss->modSub(temp2, 1, c, size);
    Mult(temp2, temp2, u, size, threadID, net, ss);
    Mult(temp2, temp2, b, size, threadID, net, ss);
    ss->modAdd(valuesP[0], temp1, temp2, size);
    // line 8
    Mult(temp1, c, valuesP[1], size, threadID, net, ss);
    ss->modSub(temp1, (long)0, temp1, size);
    doOperation_Pow2(pow2, temp1, L, size, threadID, net, ss);

    // line 9
    doOperation_Inv(pow2, pow2, size, threadID, net, ss);

    // line 10
    Mult(temp1, c, pow2, size, threadID, net, ss);
    ss->modAdd(temp1, temp1, 1, size);
    ss->modSub(temp1, temp1, c, size);
    Mult(valuesP[0], valuesP[0], temp1, size, threadID, net, ss);
    // line 11
    Mult(temp1, b, c, size, threadID, net, ss);
    ss->modMul(temp1, temp1, gamma - 1, size);
    doOperation_Mod2MS(u, valuesP[0], temp1, temp2, L, size, threadID, net, ss);
    // line 12
    Mult(temp1, b, c, size, threadID, net, ss);
    Mult(temp1, temp1, u, size, threadID, net, ss);
    Mult(temp2, b, c, size, threadID, net, ss);
    ss->modSub(temp2, 1, temp2, size);
    Mult(temp2, temp2, valuesP[0], size, threadID, net, ss);
    ss->modAdd(valuesP[0], temp2, temp1, size);
    // line 13
    ss->modSub(temp1, 1, c, size);
    Mult(temp1, temp1, a, size, threadID, net, ss);
    Mult(temp1, temp1, valuesP[1], size, threadID, net, ss);
    doOperation_Pow2(pow2, temp1, gamma - 1, size, threadID, net, ss);
    // line 14
    ss->modSub(temp1, 1, valuesP[2], size);
    ss->modMul(temp2, valuesP[3], 2, size);
    ss->modSub(temp2, 1, temp2, size);
    Mult(temp1, temp1, temp2, size, threadID, net, ss);
    Mult(results, temp1, pow2, size, threadID, net, ss);
    Mult(results, results, a, size, threadID, net, ss);
    Mult(results, results, valuesP[0], size, threadID, net, ss);
    // free memory
    for (int i = 0; i < size; i++) {
        mpz_clear(modes[i]);
        mpz_clear(temp1[i]);
        mpz_clear(temp2[i]);
        mpz_clear(a[i]);
        mpz_clear(b[i]);
        mpz_clear(c[i]);
        mpz_clear(u[i]);
        mpz_clear(pow2[i]);
    }
    free(modes);
    free(temp1);
    free(temp2);
    free(a);
    free(b);
    free(c);
    free(u);
    free(pow2);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < size; j++) {
            mpz_clear(values[i][j]);
            mpz_clear(valuesP[i][j]);
        }
        free(values[i]);
        free(valuesP[i]);
    }

    free(values);
    free(valuesP);
}
