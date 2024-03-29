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
#include "FLLTZ.h"

// Source: Aliasgari et al., "Secure Computation on Floating Point Numbers," 2013
// Based on Protocol FLLT, page 9
void doOperation_FLLTZ(mpz_t **A1, mpz_t **B1, mpz_t *result, int K, int L, int size, int threadID, NodeNetwork net,  SecretShare *ss) {

    /***********************************************************************/
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
    /***********************************************************************/
    mpz_t const1, const2;
    mpz_t *a = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *c = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *d = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b_p = (mpz_t *)malloc(sizeof(mpz_t) * size);

    mpz_t *temp1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp3 = (mpz_t *)malloc(sizeof(mpz_t) * size);

    mpz_init_set_ui(const1, 1);
    mpz_init_set_ui(const2, 2);

    for (int i = 0; i < size; i++) {
        mpz_init(a[i]);
        mpz_init(b[i]);
        mpz_init(b1[i]);
        mpz_init(b2[i]);
        mpz_init(c[i]);
        mpz_init(d[i]);
        mpz_init(b_p[i]);

        mpz_init(temp1[i]);
        mpz_init(temp2[i]);
        mpz_init(temp3[i]);
    }
    // gettimeofday(&tv1, NULL);
    // L = 9;
    // K = 32;
    //  line 1
    ss->modSub(temp1, A[1], B[1], size);
    doOperation_LTZ(a, temp1, L, size, threadID, net, ss);
    // line 2
    ss->modSub(temp1, A[1], B[1], size);
    doOperation_EQZ(temp1, c, L, size, threadID, net, ss);
    // line 3
    ss->modMul(temp1, A[3], const2, size);
    ss->modSub(temp2, const1, temp1, size);
    Mult(temp3, temp2, A[0], size, threadID, net, ss);

    ss->modMul(temp1, B[3], const2, size);
    ss->modSub(temp2, const1, temp1, size);
    Mult(temp1, temp2, B[0], size, threadID, net, ss);

    ss->modSub(temp1, temp3, temp1, size);
    doOperation_LTZ(d, temp1, K + 1, size, threadID, net, ss);

    // line 4
    Mult(temp1, c, d, size, threadID, net, ss);
    ss->modSub(temp2, const1, c, size);
    Mult(temp3, temp2, a, size, threadID, net, ss);
    ss->modAdd(b1, temp1, temp3, size);
    // line 5
    Mult(temp1, c, d, size, threadID, net, ss);
    ss->modSub(temp2, const1, c, size);
    ss->modSub(temp3, const1, a, size);
    Mult(b2, temp2, temp3, size, threadID, net, ss);
    ss->modAdd(b2, b2, temp1, size);
    // line 6
    // b_p1
    ss->modSub(temp1, const1, B[2], size);
    ss->modSub(temp2, const1, B[3], size);
    Mult(temp3, temp1, temp2, size, threadID, net, ss);
    Mult(temp3, temp3, A[2], size, threadID, net, ss);
    ss->modAdd(b, b, temp3, size);
    // b_p2
    ss->modSub(temp1, const1, A[2], size);
    Mult(temp2, temp1, B[2], size, threadID, net, ss);
    Mult(temp2, temp2, A[3], size, threadID, net, ss);
    ss->modAdd(b, b, temp2, size);
    // b_p
    /* compute (1-[z1]) * (1-[z2]) */
    ss->modSub(temp1, const1, B[2], size);
    ss->modSub(temp3, const1, A[2], size);
    Mult(b_p, temp1, temp3, size, threadID, net, ss);
    /* compute [s1] * (1 - [s2])  */
    ss->modSub(temp1, const1, B[3], size);
    Mult(temp2, temp1, A[3], size, threadID, net, ss);
    /* compute (1-[s1]) *(1-[s2]) * b+ */
    ss->modSub(temp1, const1, A[3], size);
    ss->modSub(temp3, const1, B[3], size);
    Mult(temp1, temp1, temp3, size, threadID, net, ss);
    Mult(temp3, temp1, b1, size, threadID, net, ss);
    /*  compute [s1] * [s2] * b2 */
    Mult(temp1, A[3], B[3], size, threadID, net, ss);
    Mult(temp1, temp1, b2, size, threadID, net, ss);
    ss->modAdd(temp1, temp1, temp2, size);
    ss->modAdd(temp1, temp1, temp3, size);
    Mult(temp3, temp1, b_p, size, threadID, net, ss);
    // add b_p1, b_p2, and b_p
    ss->modAdd(b, b, temp3, size);

    for (int i = 0; i < size; i++)
        mpz_set(result[i], b[i]);

    // free the memory
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

    for (int i = 0; i < size; i++) {
        mpz_clear(a[i]);
        mpz_clear(b[i]);
        mpz_clear(b1[i]);
        mpz_clear(b2[i]);
        mpz_clear(c[i]);
        mpz_clear(d[i]);
        mpz_clear(b_p[i]);
        mpz_clear(temp1[i]);
        mpz_clear(temp2[i]);
        mpz_clear(temp3[i]);
    }

    free(a);
    free(b);
    free(b1);
    free(b2);
    free(c);
    free(d);
    free(b_p);
    free(temp1);
    free(temp2);
    free(temp3);
    mpz_clear(const1);
    mpz_clear(const2);
}
