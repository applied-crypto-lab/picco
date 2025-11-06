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
#include "FLDiv.h"

// Source: Aliasgari et al., "Secure Computation on Floating Point Numbers," 2013
// Protocol FLDiv with public divisor, page 6
// New 
// I combined the two mult to one the other two were dependent
/*
1) [v] ← SDiv([v1], [v2] + [z2], l);
2) [b] ← LT([v], 2l, l + 1);
3) [v] ← Trunc(2[b][v] + (1 − [b])[v], l + 1, 1);
4) [p] ← (1 − [z1])([p1] − [p2] − l + 1 − [b]);
5) [z] ← [z1];
6) [s] ← XOR([s1], [s2]);
7) [Error] ← [z2];
8) return (〈[v], [p], [z], [s]〉, [Error]);
*/
// void doOperation_FLDiv(mpz_t **A1, mpz_t **B1, mpz_t **result1, int K, int size, int threadID, NodeNetwork net,  SecretShare *ss) {
//     // int peers = ss->getPeers();
//     mpz_t beta, const1, const2, constK; // change K to k-1 for naming
//     /***********************************************************************/
//     mpz_t **A = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
//     mpz_t **B = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
//     mpz_t **result = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
//     for (int i = 0; i < 4; i++) {
//         A[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
//         B[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
//         result[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);

//         for (int j = 0; j < size; j++) {
//             mpz_init_set(A[i][j], A1[j][i]);
//             mpz_init_set(B[i][j], B1[j][i]);
//             mpz_init_set(result[i][j], result1[j][i]);
//         }
//     }
//     /***********************************************************************/
//     mpz_t *b = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *Y = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *temp1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *temp2 = (mpz_t *)malloc(sizeof(mpz_t) * size);

//     // initialization
//     mpz_init(beta);
//     mpz_init_set_ui(const1, 1);
//     mpz_init_set_ui(const2, 2);
//     mpz_init_set_ui(constK, K-1); // changed to k-1 

//     for (int i = 0; i < size; i++) {
//         mpz_init(b[i]);
//         mpz_init(temp1[i]);
//         mpz_init(temp2[i]);
//         mpz_init(Y[i]);
//     }
//     ss->modPow(beta, const2, constK);

//     ss->modAdd(temp1, B[0], B[2], size);
//     ss->copy(A[0], Y, size);
//     doOperation_SDiv(Y, Y, temp1, K, size, threadID, net, ss);
//     // line 2
//     ss->modSub(temp1, Y, beta, size);
//     doOperation_LTZ(b, temp1, K + 1, size, threadID, net, ss);
//     // line 3
//     ss->modSub(temp1, const1, b, size);


//     mpz_t *first_array = (mpz_t *)malloc(sizeof(mpz_t) * size * 3); 
//     mpz_t *second_array = (mpz_t *)malloc(sizeof(mpz_t) * size * 3);
//     // Mult(temp1, temp1, Y, size, threadID, net, ss);
//     // Mult(temp2, b, Y, size, threadID, net, ss);
//     ss->copy(temp1, first_array, size);
//     ss->copy(Y, second_array, size);

//     // line 4
//     ss->modSub(&first_array[size], const1, A[2], size);
//     ss->modSub(temp2, A[1], B[1], size);
//     ss->modSub(temp2, temp2, constK, size); // this is k-1  instead of doing the next line
//     // ss->modAdd(temp2, temp2, const1, size); // add by 1 goes away
//     ss->modSub(temp2, temp2, b, size);

//     // Mult(temp1, temp1, temp2, size, threadID, net, ss);

//     // Do the last mult here round 1
//     Mult(first_array, first_array, second_array, 2*size, threadID, net, ss);
//     ss->modMul(temp2, &first_array[size], const2, size);
//     ss->modAdd(temp1, temp2, first_array, size);
//     doOperation_Trunc(result[0], temp1, K + 1, 1, size, threadID, net, ss);

    
//     // line 5
//     ss->copy(temp1, result[1], size);
//     ss->copy(A[2], result[2], size);
//     // line 6
//     Mult(temp1, A[3], B[3], size, threadID, net, ss);
//     ss->modMul(temp1, temp1, const2, size);
//     ss->modAdd(temp2, A[3], B[3], size);
//     ss->modSub(result[3], temp2, temp1, size);

//     for (int i = 0; i < size; i++)
//         for (int j = 0; j < 4; j++)
//             mpz_set(result1[i][j], result[j][i]);

//     // free the memory
//     for (int i = 0; i < 4; i++) {
//         for (int j = 0; j < size; j++) {
//             mpz_clear(A[i][j]);
//             mpz_clear(B[i][j]);
//             mpz_clear(result[i][j]);
//         }
//         free(A[i]);
//         free(B[i]);
//         free(result[i]);
//     }
//     free(A);
//     free(B);
//     free(result);

//     mpz_clear(beta);
//     mpz_clear(const1);
//     mpz_clear(const2);
//     mpz_clear(constK);

//     for (int i = 0; i < size; i++) {
//         mpz_clear(b[i]);
//         mpz_clear(temp1[i]);
//         mpz_clear(temp2[i]);
//         mpz_clear(Y[i]);
//     }
//     free(b);
//     free(temp1);
//     free(temp2);
//     free(Y);
// }

void doOperation_FLDiv_Pub(mpz_t **A1, mpz_t **B1, mpz_t **result1, int K, int size, int threadID, NodeNetwork net,  SecretShare *ss) {

    /***********************************************************************/
    mpz_t **A = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    mpz_t **B = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    mpz_t **result = (mpz_t **)malloc(sizeof(mpz_t *) * 4);

    for (int i = 0; i < 4; i++) {
        A[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        B[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        result[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);

        for (int j = 0; j < size; j++) {
            mpz_init_set(A[i][j], A1[j][i]);
            mpz_init_set(B[i][j], B1[j][i]);
            mpz_init_set(result[i][j], result1[j][i]);
        }
    }
    /***********************************************************************/
    // int peers = ss->getPeers();
    mpz_t beta, const1, const2, constK, const2K;
    mpz_t *b = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *Y = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp2 = (mpz_t *)malloc(sizeof(mpz_t) * size);

    // initialization
    mpz_init(beta);
    mpz_init(const2K);
    mpz_init_set_ui(const1, 1);
    mpz_init_set_ui(const2, 2);
    mpz_init_set_ui(constK, K);

    for (int i = 0; i < size; i++) {
        mpz_init(b[i]);
        mpz_init(temp1[i]);
        mpz_init(temp2[i]);
        mpz_init(Y[i]);
    }

    /***************************/
    ss->modPow(beta, const2, constK);
    ss->modPow(const2K, const2, 2 * K);

    ss->modAdd(temp1, B[0], B[2], size);
    ss->copy(A[0], Y, size);

    for (int i = 0; i < size; i++) {
        mpz_div(temp1[i], const2K, temp1[i]);
        ss->modMul(Y[i], Y[i], temp1[i]);
    }
    doOperation_Trunc(Y, Y, 2 * K, K, size, threadID, net, ss);
    // line 2
    ss->modSub(temp1, Y, beta, size);
    doOperation_LTZ(b, temp1, K + 1, size, threadID, net, ss);
    // line 3
    ss->modSub(temp1, const1, b, size);
    Mult(temp1, temp1, Y, size, threadID, net, ss);
    Mult(temp2, b, Y, size, threadID, net, ss);
    ss->modMul(temp2, temp2, const2, size);
    ss->modAdd(temp1, temp2, temp1, size);
    doOperation_Trunc(result[0], temp1, K + 1, 1, size, threadID, net, ss);
    // line 4
    ss->modSub(temp1, const1, A[2], size);
    ss->modSub(temp2, A[1], B[1], size);
    ss->modSub(temp2, temp2, constK, size);
    ss->modAdd(temp2, temp2, const1, size);
    ss->modSub(temp2, temp2, b, size);
    Mult(temp1, temp1, temp2, size, threadID, net, ss);
    // line 5
    ss->copy(temp1, result[1], size);
    ss->copy(A[2], result[2], size);
    // line 6
    Mult(temp1, A[3], B[3], size, threadID, net, ss);
    ss->modMul(temp1, temp1, const2, size);
    ss->modAdd(temp2, A[3], B[3], size);
    ss->modSub(result[3], temp2, temp1, size);

    // copy the result
    for (int i = 0; i < size; i++)
        for (int j = 0; j < 4; j++)
            mpz_set(result1[i][j], result[j][i]);
    // free the memory
    mpz_clear(beta);
    mpz_clear(const2K);
    mpz_clear(const1);
    mpz_clear(const2);
    mpz_clear(constK);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < size; j++) {
            mpz_clear(A[i][j]);
            mpz_clear(B[i][j]);
            mpz_clear(result[i][j]);
        }
        free(A[i]);
        free(B[i]);
        free(result[i]);
    }

    free(A);
    free(B);
    free(result);

    for (int i = 0; i < size; i++) {
        mpz_clear(b[i]);
        mpz_clear(temp1[i]);
        mpz_clear(temp2[i]);
        mpz_clear(Y[i]);
    }

    free(b);
    free(temp1);
    free(temp2);
    free(Y);
}

// Source: Aliasgari et al., "Secure Computation on Floating Point Numbers," 2013
// Protocol FLDiv, page 6
void doOperation_FLDiv(mpz_t **A1, mpz_t **B1, mpz_t **result1, int K, int size, int threadID, NodeNetwork net,  SecretShare *ss) {
    // int peers = ss->getPeers();
    mpz_t beta, const1, const2, constK;
    /***********************************************************************/
    mpz_t **A = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    mpz_t **B = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    mpz_t **result = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    for (int i = 0; i < 4; i++) {
        A[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        B[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        result[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);

        for (int j = 0; j < size; j++) {
            mpz_init_set(A[i][j], A1[j][i]);
            mpz_init_set(B[i][j], B1[j][i]);
            mpz_init_set(result[i][j], result1[j][i]);
        }
    }
    /***********************************************************************/
    mpz_t *b = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *Y = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp2 = (mpz_t *)malloc(sizeof(mpz_t) * size);

    // initialization
    mpz_init(beta);
    mpz_init_set_ui(const1, 1);
    mpz_init_set_ui(const2, 2);
    mpz_init_set_ui(constK, K);

    for (int i = 0; i < size; i++) {
        mpz_init(b[i]);
        mpz_init(temp1[i]);
        mpz_init(temp2[i]);
        mpz_init(Y[i]);
    }
    ss->modPow(beta, const2, constK);

    ss->modAdd(temp1, B[0], B[2], size);
    ss->copy(A[0], Y, size);
    doOperation_SDiv(Y, Y, temp1, K, size, threadID, net, ss);
    // line 2
    ss->modSub(temp1, Y, beta, size);
    doOperation_LTZ(b, temp1, K + 1, size, threadID, net, ss);
    // line 3
    ss->modSub(temp1, const1, b, size);
    Mult(temp1, temp1, Y, size, threadID, net, ss);
    Mult(temp2, b, Y, size, threadID, net, ss);
    ss->modMul(temp2, temp2, const2, size);
    ss->modAdd(temp1, temp2, temp1, size);
    doOperation_Trunc(result[0], temp1, K + 1, 1, size, threadID, net, ss);
    // line 4
    ss->modSub(temp1, const1, A[2], size);
    ss->modSub(temp2, A[1], B[1], size);
    ss->modSub(temp2, temp2, constK, size);
    ss->modAdd(temp2, temp2, const1, size);
    ss->modSub(temp2, temp2, b, size);
    Mult(temp1, temp1, temp2, size, threadID, net, ss);
    // line 5
    ss->copy(temp1, result[1], size);
    ss->copy(A[2], result[2], size);
    // line 6
    Mult(temp1, A[3], B[3], size, threadID, net, ss);
    ss->modMul(temp1, temp1, const2, size);
    ss->modAdd(temp2, A[3], B[3], size);
    ss->modSub(result[3], temp2, temp1, size);

    for (int i = 0; i < size; i++)
        for (int j = 0; j < 4; j++)
            mpz_set(result1[i][j], result[j][i]);

    // free the memory
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < size; j++) {
            mpz_clear(A[i][j]);
            mpz_clear(B[i][j]);
            mpz_clear(result[i][j]);
        }
        free(A[i]);
        free(B[i]);
        free(result[i]);
    }
    free(A);
    free(B);
    free(result);

    mpz_clear(beta);
    mpz_clear(const1);
    mpz_clear(const2);
    mpz_clear(constK);

    for (int i = 0; i < size; i++) {
        mpz_clear(b[i]);
        mpz_clear(temp1[i]);
        mpz_clear(temp2[i]);
        mpz_clear(Y[i]);
    }
    free(b);
    free(temp1);
    free(temp2);
    free(Y);
}
