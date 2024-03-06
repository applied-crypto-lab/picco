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
#include "FLEQZ.h"

// FLEQZ::FLEQZ(NodeNetwork nodeNet, std::map<std::string, std::vector<int>> poly, int nodeID, SecretShare *s) {
//     // Mul = new Mult(nodeNet, s);
//     net = nodeNet;
//     id = nodeID;
//     ss = s;
// }

// FLEQZ::~FLEQZ() {
//     // TODO Auto-generated destructor stub
// }

// Source: Aliasgari et al., "Secure Computation on Floating Point Numbers," 2013
// Based on Protocol FLLT, page 9
void doOperation_FLEQZ(mpz_t **A1, mpz_t **B1, mpz_t *result, int K, int L, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {

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
    mpz_init_set_ui(const1, 1);
    mpz_init_set_ui(const2, 2);

    mpz_t *b = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b5 = (mpz_t *)malloc(sizeof(mpz_t) * size);

    mpz_t *temp1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp2 = (mpz_t *)malloc(sizeof(mpz_t) * size);

    for (int i = 0; i < size; i++) {
        mpz_init(b[i]);
        mpz_init(b1[i]);
        mpz_init(b2[i]);
        mpz_init(b5[i]);

        mpz_init(temp1[i]);
        mpz_init(temp2[i]);
    }
    // compute b1 = v_a == v_b
    ss->modSub(temp1, A[0], B[0], size);
    doOperation_EQZ(temp1, b1, K, size, threadID, net, id, ss);
    // compute b2 = p_a == p_b
    ss->modSub(temp1, A[1], B[1], size);
    doOperation_EQZ(temp1, b2, L, size, threadID, net, id, ss);

    // compute b3 = 1 - XOR(s_a, s_b) and b5 = AND(z_a, z_b)
    ss->modAdd(temp1, A[3], B[3], size);
    Mult(temp2, A[3], B[3], size, threadID, net, ss);
    ss->modMul(temp2, temp2, const2, size);
    ss->modSub(temp1, temp1, temp2, size);
    ss->modSub(temp2, const1, temp1, size); // temp2 = b3
    Mult(b5, A[2], B[2], size, threadID, net, ss);

    // compute b4 = AND(b1, b2, b3)
    Mult(temp1, b1, b2, size, threadID, net, ss);
    Mult(temp2, temp1, temp2, size, threadID, net, ss);

    // compute b = OR(b4, b5)
    ss->modAdd(temp1, temp2, b5, size);
    Mult(temp2, temp2, b5, size, threadID, net, ss);
    ss->modSub(b, temp1, temp2, size);

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
    /***********************************************************************/
    for (int i = 0; i < size; i++) {
        mpz_clear(b[i]);
        mpz_clear(b1[i]);
        mpz_clear(b2[i]);
        mpz_clear(b5[i]);
        mpz_clear(temp1[i]);
        mpz_clear(temp2[i]);
    }

    free(b);
    free(b1);
    free(b2);
    free(b5);
    free(temp1);
    free(temp2);
}
