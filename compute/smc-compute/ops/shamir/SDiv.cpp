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
#include "SDiv.h"


void doOperation_SDiv(mpz_t *Y, mpz_t *A, mpz_t *B, int K, int size, int threadID, NodeNetwork net,  SecretShare *ss) {
    mpz_t const2, constK1, const2K1;
    mpz_t *X1 = (mpz_t *)malloc(sizeof(mpz_t) * 2 * size);
    mpz_t *XY = (mpz_t *)malloc(sizeof(mpz_t) * 2 * size);
    mpz_t *temp = (mpz_t *)malloc(sizeof(mpz_t) * 2 * size);
    mpz_t *temp1 = (mpz_t *)malloc(sizeof(mpz_t) * size);

    // initialization
    mpz_init_set_ui(const2, 2);
    mpz_init_set_ui(constK1, K + 1);
    mpz_init(const2K1);
    ss->modPow(const2K1, const2, constK1);

    for (int i = 0; i < size; i++)
        mpz_init(temp1[i]);

    for (int i = 0; i < 2 * size; i++) {
        mpz_init(temp[i]);
        mpz_init(X1[i]);
        mpz_init(XY[i]);
    }

    // start computation
    int sita = ceil(log2(K));
    ss->copy(A, Y, size);

    for (int i = 0; i < size; i++) {
        mpz_set(X1[i], B[i]);
        mpz_set(X1[size + i], B[i]);
        mpz_set(XY[i], Y[i]);
        mpz_set(XY[size + i], B[i]);
    }

    for (int i = 1; i <= sita - 1; i++) {
        ss->modSub(temp, const2K1, X1, 2 * size);
        Mult(temp, XY, temp, 2 * size, threadID, net, ss);
        doOperation_TruncPr(XY, temp, 2 * K + 1, K, 2 * size, threadID, net, ss);
        for (int j = 0; j < size; j++) {
            mpz_set(X1[j], XY[size + j]);
            mpz_set(X1[size + j], XY[size + j]);
        }
    }

    for (int i = 0; i < size; i++) {
        mpz_set(temp1[i], XY[size + i]);
        mpz_set(Y[i], XY[i]);
    }

    ss->modSub(temp1, const2K1, temp1, size);
    Mult(temp1, Y, temp1, size, threadID, net, ss);
    doOperation_TruncPr(Y, temp1, 2 * K + 1, K, size, threadID, net, ss);

    // free the memory
    mpz_clear(const2);
    mpz_clear(constK1);
    mpz_clear(const2K1);

    for (int i = 0; i < size; i++)
        mpz_clear(temp1[i]);

    for (int i = 0; i < 2 * size; i++) {
        mpz_clear(temp[i]);
        mpz_clear(X1[i]);
        mpz_clear(XY[i]);
    }

    free(temp);
    free(temp1);
    free(X1);
    free(XY);
}

// from the SDiv2 file, unsure what the fundamental difference with the above implementation beyond the multithreading argument
void doOperation_SDiv_2(mpz_t *Y, mpz_t *A, mpz_t *B, int K, int size, NodeNetwork net,  SecretShare *ss) {
    int peers = ss->getPeers();
    mpz_t const2, constK1;
    mpz_t *X = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *const2K1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp3 = (mpz_t *)malloc(sizeof(mpz_t) * size);

    // initialization
    mpz_init_set_ui(const2, 2);
    mpz_init_set_ui(constK1, K + 1);

    for (int i = 0; i < size; i++) {
        mpz_init(const2K1[i]);
        mpz_init(temp1[i]);
        mpz_init(temp2[i]);
        mpz_init(temp3[i]);
        mpz_init(X[i]);
    }
    // start computation
    ss->modPow(const2K1[0], const2, constK1);
    int sita = ceil(log2(K));
    ss->copy(A, Y, size);
    ss->copy(B, X, size);

    /***** test purpose ********/
    mpz_t *temp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init(temp[i]);
    mpz_t **shares = (mpz_t **)malloc(sizeof(mpz_t *) * peers);
    for (int i = 0; i < peers; i++) {
        shares[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++) {
            mpz_init(shares[i][j]);
        }
    }

    for (int i = 0; i < size; i++)
        mpz_set(const2K1[i], const2K1[0]);

    for (int i = 1; i <= sita - 1; i++) {
        ss->modSub(temp2, const2K1, X, size);
        /*********/
        /*net.broadcastToPeers(temp2, size, shares);
        ss->reconstructSecret(temp, shares, size);
        for(int i = 0; i < size; i++)
            gmp_printf("temp2 : %Zd\n", temp[i]); */
        /********/
        Mult(temp3, Y, temp2, size,-1, net, ss);
        doOperation_TruncPr(Y, temp3, 2 * K + 1, K, size, -1, net, ss);
        ss->modSub(temp2, const2K1, X, size);
        Mult(temp3, X, temp2, size,-1, net, ss);
        doOperation_TruncPr(X, temp3, 2 * K + 1, K, size, -1, net, ss);
    }
    ss->modSub(temp2, const2K1, X, size);
    Mult(temp3, Y, temp2, size,-1, net, ss);
    doOperation_TruncPr(Y, temp3, 2 * K + 1, K, size, -1, net, ss);
    // free the memory
    mpz_clear(const2);
    mpz_clear(constK1);

    for (int i = 0; i < size; i++) {
        mpz_clear(const2K1[i]);
        mpz_clear(temp1[i]);
        mpz_clear(temp2[i]);
        mpz_clear(temp3[i]);
        mpz_clear(X[i]);
    }

    free(const2K1);
    free(temp1);
    free(temp2);
    free(temp3);
    free(X);
}
