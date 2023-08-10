/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame
   ** Department of Computer Science and Engineering, University of Buffalo (SUNY)

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
#include "FLRound.h"

FLRound::FLRound(NodeNetwork nodeNet, std::map<std::string, std::vector<int>> poly, int nodeID, SecretShare *s) {

    Lt = new LTZ(nodeNet, poly, nodeID, s);
    Mod = new Mod2M(nodeNet, poly, nodeID, s);
    Eq = new EQZ(nodeNet, poly, nodeID, s);
    // Mul = new Mult(nodeNet, nodeID, s);
    Md2m = new Mod2MS(nodeNet, poly, nodeID, s);
    Fladd = new FLAdd(nodeNet, poly, nodeID, s);

    net = nodeNet;
    id = nodeID;
    ss = s;
}

FLRound::~FLRound() {}

// Source: Aliasgari et al., "Secure Computation on Floating Point Numbers," 2013
// Protocol FLRound, page 8
void FLRound::doOperation(mpz_t **A2, mpz_t **result, mpz_t *mode, int L, int K, int size, int threadID) {

    mpz_t constPower2L, constPower2L1, const2, constL, constL1;
    mpz_init(constPower2L);
    mpz_init(constPower2L1);
    mpz_init_set_ui(const2, 2);
    mpz_init_set_ui(constL, L);
    mpz_init_set_ui(constL1, L - 1);
    ss->modPow(constPower2L, const2, constL);
    ss->modPow(constPower2L1, const2, constL1);

    mpz_t *a = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *c = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *d = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *V2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *V = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *powM = (mpz_t *)malloc(sizeof(mpz_t) * size);

    for (int i = 0; i < size; i++) {
        mpz_init(a[i]);
        mpz_init(b[i]);
        mpz_init(c[i]);
        mpz_init(d[i]);
        mpz_init(V[i]);
        mpz_init(V2[i]);
        mpz_init(temp1[i]);
        mpz_init(temp2[i]);
        mpz_init(powM[i]);
    }

    mpz_t **A1 = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    mpz_t **constOneHalf = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    mpz_t **A = (mpz_t **)malloc(sizeof(mpz_t *) * 4);

    for (int i = 0; i < size; i++) {
        A1[i] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
        constOneHalf[i] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
        for (int j = 0; j < 4; j++)
            mpz_init_set(A1[i][j], A2[j][i]);
        mpz_init_set(constOneHalf[i][0], constPower2L1);
        mpz_init_set_si(constOneHalf[i][1], -L);
        mpz_init_set_ui(constOneHalf[i][2], 0);
        mpz_init_set_ui(constOneHalf[i][3], 0);
    }

    // line 0
    Fladd->doOperation(A1, constOneHalf, A1, L, K, size, threadID);
    for (int i = 0; i < 4; i++) {
        A[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++)
            mpz_init_set(A[i][j], A1[j][i]);
    }
    // free memory for A1 and constOneHalf
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 4; j++) {
            mpz_clear(constOneHalf[i][j]);
            mpz_clear(A1[i][j]);
        }
        free(constOneHalf[i]);
        free(A1[i]);
    }
    free(constOneHalf);
    free(A1);
    // line 1
    Lt->doOperation(a, A[1], K, size, threadID);

    // line 2
    ss->modSub(temp1, A[1], 1, size);
    ss->modAdd(temp1, temp1, L, size);
    Lt->doOperation(b, temp1, K, size, threadID);
    // line 3
    ss->modSub(temp1, 1, b, size);
    Mult(temp1, temp1, a, size, threadID, net, id, ss);
    Mult(temp1, temp1, A[1], size, threadID, net, id, ss);
    ss->modSub(temp1, (long)0, temp1, size);
    Md2m->doOperation(V2, A[0], temp1, powM, L, size, threadID);
    // line 4
    Eq->doOperation(V2, c, L, size, threadID);

    // line 5
    ss->modAdd(temp1, mode, A[3], size);
    Mult(temp2, mode, A[3], size, threadID, net, id, ss);
    ss->modMul(temp2, temp2, 2, size);
    ss->modSub(temp1, temp1, temp2, size);
    ss->modSub(temp2, 1, c, size);
    Mult(temp1, temp1, temp2, size, threadID, net, id, ss);
    Mult(temp1, temp1, powM, size, threadID, net, id, ss);
    ss->modSub(temp2, A[0], V2, size);
    ss->modAdd(V, temp2, temp1, size);

    // line 6
    ss->modSub(temp1, constPower2L, V, size);
    Eq->doOperation(temp1, d, L + 1, size, threadID);

    // line 7
    ss->modMul(temp1, d, constPower2L1, size);
    ss->modSub(temp2, 1, d, size);
    Mult(temp2, temp2, V, size, threadID, net, id, ss);
    ss->modAdd(V, temp2, temp1, size);

    // line 8
    ss->modSub(temp1, 1, b, size);
    Mult(temp1, temp1, V, size, threadID, net, id, ss);
    ss->modSub(temp2, mode, A[3], size);
    Mult(temp2, temp2, b, size, threadID, net, id, ss);
    ss->modAdd(temp1, temp1, temp2, size);
    Mult(temp1, temp1, a, size, threadID, net, id, ss);
    ss->modSub(temp2, 1, a, size);
    Mult(temp2, temp2, A[0], size, threadID, net, id, ss);
    ss->modAdd(V, temp1, temp2, size);

    // line 9
    Mult(temp1, mode, b, size, threadID, net, id, ss);
    ss->modSub(temp1, 1, temp1, size);
    Mult(result[3], A[3], temp1, size, threadID, net, id, ss);

    // line 10
    Eq->doOperation(V, temp2, L, size, threadID);
    Mult(temp1, temp2, A[2], size, threadID, net, id, ss);
    ss->modAdd(temp2, temp2, A[2], size);
    ss->modSub(result[2], temp2, temp1, size);

    // line 11
    ss->modSub(temp1, 1, result[2], size);
    Mult(result[0], V, temp1, size, threadID, net, id, ss);

    // line 12
    ss->modSub(temp1, 1, b, size);
    Mult(temp1, temp1, a, size, threadID, net, id, ss);
    Mult(temp1, temp1, d, size, threadID, net, id, ss);
    ss->modAdd(temp1, temp1, A[1], size);
    ss->modSub(temp2, 1, result[2], size);
    Mult(result[1], temp1, temp2, size, threadID, net, id, ss);

    // free memory for temporary arrays
    mpz_clear(constPower2L);
    mpz_clear(constPower2L1);
    mpz_clear(const2);
    mpz_clear(constL);
    mpz_clear(constL1);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < size; j++)
            mpz_clear(A[i][j]);
        free(A[i]);
    }
    free(A);

    for (int i = 0; i < size; i++) {
        mpz_clear(a[i]);
        mpz_clear(b[i]);
        mpz_clear(c[i]);
        mpz_clear(d[i]);
        mpz_clear(V[i]);
        mpz_clear(V2[i]);
        mpz_clear(temp1[i]);
        mpz_clear(temp2[i]);
        mpz_clear(powM[i]);
    }

    free(a);
    free(b);
    free(c);
    free(d);
    free(V);
    free(V2);
    free(temp1);
    free(temp2);
    free(powM);
}
