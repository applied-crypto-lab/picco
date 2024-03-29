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
#include "PrefixMultiplication.h"

// Source: Catrina and de Hoogh, "Improved Primites for Secure Multiparty Integer Computation," 2010
// Protocol 4.2 page 11
void doOperation_PrefixMult(mpz_t **input, mpz_t **result, int length_k, int size, int threadID, NodeNetwork net,  SecretShare *ss) {

    int peers = ss->getPeers();
    mpz_t *R = (mpz_t *)malloc(sizeof(mpz_t) * length_k * size);
    mpz_t *S = (mpz_t *)malloc(sizeof(mpz_t) * length_k * size);

    mpz_t *R_buff = (mpz_t *)malloc(sizeof(mpz_t) * length_k * size);
    mpz_t *S_buff = (mpz_t *)malloc(sizeof(mpz_t) * length_k * size);

    mpz_t *przs = (mpz_t *)malloc(sizeof(mpz_t) * length_k * size);

    mpz_t *U = (mpz_t *)malloc(sizeof(mpz_t) * length_k * size);
    mpz_t *V = (mpz_t *)malloc(sizeof(mpz_t) * length_k * size);

    // these are needed for Step 5 multiplication
    mpz_t **buffer1 = (mpz_t **)malloc(sizeof(mpz_t *) * peers);
    mpz_t **buffer2 = (mpz_t **)malloc(sizeof(mpz_t *) * peers);

    for (int i = 0; i < length_k * size; i++) {
        mpz_init(R[i]);
        mpz_init(S[i]);

        mpz_init(R_buff[i]);
        mpz_init(S_buff[i]);

        mpz_init(przs[i]);

        mpz_init(V[i]);
        mpz_init(U[i]);
    }

    for (int i = 0; i < peers; i++) {
        buffer1[i] = (mpz_t *)malloc(sizeof(mpz_t) * length_k * size);
        buffer2[i] = (mpz_t *)malloc(sizeof(mpz_t) * length_k * size);
        for (int j = 0; j < length_k * size; j++) {
            mpz_init(buffer1[i][j]);
            mpz_init(buffer2[i][j]);
        }
    }

    mpz_t field;
    mpz_init(field);
    ss->getFieldSize(field);
    ss->generateRandValue(field, length_k * size, R, threadID);
    ss->generateRandValue(field, length_k * size, S, threadID);
    ss->PRZS(field, size, przs);

    // step 4, MulPub (can't be replaced with Open)
    ss->modMul(U, R, S, length_k * size);
    ss->modAdd(U, U, przs, length_k * size); // adding pseudorandom shares of zero

    net.broadcastToPeers(U, length_k * size, buffer1, threadID);
    ss->reconstructSecret(U, buffer1, length_k * size);
    clearBuffer(buffer1, peers, length_k * size);

    // computing all the inverses of u (used in steps 7 and 8, only needs to be done once)
    ss->modInv(U, U, length_k * size);

    // step 5, multiplication
    // moving R,S values into correct locations
    // (length_k - 1) * size total multiplications

    int r_idx, s_idx;
    for (int i = 0; i < length_k - 1; i++) {
        for (int j = 0; j < size; j++) {
            r_idx = (i + 1) * size + j;
            s_idx = i * size + j;
            mpz_set(R_buff[s_idx], R[r_idx]);
            mpz_set(S_buff[s_idx], S[s_idx]);
        }
    }
    Mult(V, R_buff, S_buff, (length_k - 1) * size, threadID, net, ss);

    // mpz_set(W[0], R[0]); // not needed since we are using R in place of U
    for (int i = 1; i < length_k; i++) {
        for (int j = 0; j < size; j++) {
            // step 7
            ss->modMul(R[i * size + j], V[(i - 1) * size + j], U[(i - 1) * size + j]);
        }
    }

    // step 8 [z_i] <- [s_i] * (u_{i-1}^{-1} mod q)
    ss->modMul(S, S, U, length_k * size);

    // step 9, MulPub (can't be replaced with Open)
    for (int i = 0; i < length_k; i++) {
        for (int j = 0; j < size; j++) {
            // computing m_i <- [a_i] * [w_i]
            ss->modMul(U[i * size + j], input[i][j], R[i * size + j]);
        }
    }
    net.broadcastToPeers(U, length_k * size, buffer1, threadID);
    ss->reconstructSecret(U, buffer1, length_k * size);
    // end step 9

    for (int i = 0; i < size; i++)
        // reusing R, setting all the m_1's to first (size) elements of R
        mpz_set(R[i], U[i]);

    ss->copy(input[0], result[0], size); // all the a_0's are stored in input[0]
    for (int j = 0; j < size; j++) {
        for (int i = 1; i < length_k; i++) {
            // computing prod(m_i, i = 1, j)
            // int s_index = j * length_k + i;
            // printf("i = %i, j =%i, s_index = %i\n",i,j, s_index);
            ss->modMul(R[j], R[j], U[i * size + j]);
            // computing [p_j] = [z_j] * prod(m_i, i = 1, j)
            ss->modMul(result[i][j], S[i * size + j], R[j]);
        }
    }

    for (int i = 0; i < peers; i++) {
        for (int j = 0; j < length_k * size; j++) {
            mpz_clear(buffer1[i][j]);
            mpz_clear(buffer2[i][j]);
        }
        free(buffer1[i]);
        free(buffer2[i]);
    }
    free(buffer1);
    free(buffer2);

    for (int i = 0; i < length_k * size; i++) {
        mpz_clear(R[i]);
        mpz_clear(S[i]);
        mpz_clear(R_buff[i]);
        mpz_clear(S_buff[i]);
        mpz_clear(przs[i]);
        mpz_clear(V[i]);
        mpz_clear(U[i]);
    }
    free(R);
    free(S);
    free(R_buff);
    free(S_buff);
    free(przs);
    free(V);
    free(U);
}
