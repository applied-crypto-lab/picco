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

// Source: Aliasgari et al., "Secure Computation on Floating Point Numbers," 2013
// Based on Protocol FLLT, page 9
/* 
Formula:
    b1 = (v_a == v_b)
    b2 = (p_a == p_b)
    b3 = 1 - XOR(s_a, s_b) 
    b4 = AND(b1, b2, b3)
    b5 = AND(z_a, z_b)
    b = OR(b4, b5)

Updated:
    compute b1, b2
    Round 1 of Mult - b3, b6=AND(b1, b2)
    Round 2 of Mult - b4=And(b3, b6), b5
    Round 3 of Mult - b
*/
// void doOperation_FLEQZ(mpz_t **A1, mpz_t **B1, mpz_t *result, int K, int L, int size, int threadID, NodeNetwork net,  SecretShare *ss) {

//     /***********************************************************************/
//     mpz_t **A = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
//     mpz_t **B = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
//     for (int i = 0; i < 4; i++) {
//         A[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
//         B[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
//         for (int j = 0; j < size; j++) {
//             mpz_init_set(A[i][j], A1[j][i]);
//             mpz_init_set(B[i][j], B1[j][i]);
//         }
//     }

//     /***********************************************************************/
//     mpz_t const1, const2;
//     mpz_init_set_ui(const1, 1);
//     mpz_init_set_ui(const2, 2);

//     mpz_t *b = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *b1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *b2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *b5 = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *b6 = (mpz_t *)malloc(sizeof(mpz_t) * size);

//     mpz_t *temp1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *temp2 = (mpz_t *)malloc(sizeof(mpz_t) * size);

//     for (int i = 0; i < size; i++) {
//         mpz_init(b[i]);
//         mpz_init(b1[i]);
//         mpz_init(b2[i]);
//         mpz_init(b5[i]);
//         mpz_init(b6[i]);

//         mpz_init(temp1[i]);
//         mpz_init(temp2[i]);
//     }

//     mpz_t *first_array = (mpz_t *)malloc(sizeof(mpz_t) * size * 2); 
//     mpz_t *second_array = (mpz_t *)malloc(sizeof(mpz_t) * size * 2);

//     // compute b1 = v_a == v_b
//     ss->modSub(temp1, A[0], B[0], size);
//     doOperation_EQZ(temp1, b1, K, size, threadID, net, ss);
//     // compute b2 = p_a == p_b
//     ss->modSub(temp1, A[1], B[1], size);
//     doOperation_EQZ(temp1, b2, L, size, threadID, net, ss);

//     // compute b3 = 1 - XOR(s_a, s_b) and b5 = AND(z_a, z_b)
//     ss->modAdd(temp1, A[3], B[3], size);
//     // Mult(temp2, A[3], B[3], size, threadID, net, ss); // 1
//     ss->copy(A[3], first_array, size);
//     ss->copy(B[3], second_array, size);

//     // compute b4 = AND(b1, b2, b3)
//     // b6 = Mult(temp1, b1, b2, size, threadID, net, ss); // 1
//     ss->copy(b1, &first_array[size], size);
//     ss->copy(b2, &second_array[size], size);

//     // Round 1 of Mult - b3, b6=AND(b1, b2) (temp2 = b3 = first_array), (b6 = second_array)
//     Mult(first_array, first_array, second_array, 2*size, threadID, net, ss);
//     ss->copy(b6, &first_array[size], size); // copy the first part of b4 to b6

//     ss->modMul(temp2, first_array, const2, size); // first_array used instead of temp2
//     ss->modSub(temp1, temp1, temp2, size);
//     ss->modSub(temp2, const1, temp1, size); // temp2 = b3

//     // this is b5 done in round 2
//     // Mult(b5, A[2], B[2], size, threadID, net, ss); // 2
//     ss->copy(A[3], first_array, size);
//     ss->copy(B[3], second_array, size);

//     // this is b4=And(b3, b6) from above
//     // Mult(temp2, temp1, temp2, size, threadID, net, ss);
//     ss->copy(b6, &first_array[size], size); // we use b6 instead of temp1 to calculate the second half
//     ss->copy(temp2, &second_array[size], size);

//     // Mult for Round 2 - b4=And(b3, b6), b5
//     Mult(first_array, first_array, second_array, 2*size, threadID, net, ss);

//     // compute b = OR(b4, b5)
//     // Mult for Round 2 - b
//     ss->modAdd(temp1, &first_array[size], first_array, size); // first_array used instead of b5, &first_array[size] instead of temp3
//     Mult(temp2, temp2, b5, size, threadID, net, ss); 

//     ss->modSub(b, temp1, temp2, size);

//     for (int i = 0; i < size; i++)
//         mpz_set(result[i], b[i]);

//     // free the memory
//     for (int i = 0; i < 4; i++) {
//         for (int j = 0; j < size; j++) {
//             mpz_clear(A[i][j]);
//             mpz_clear(B[i][j]);
//         }
//         free(A[i]);
//         free(B[i]);
//     }
//     free(A);
//     free(B);
//     /***********************************************************************/
//     for (int i = 0; i < size; i++) {
//         mpz_clear(b[i]);
//         mpz_clear(b1[i]);
//         mpz_clear(b2[i]);
//         mpz_clear(b5[i]);
//         mpz_clear(temp1[i]);
//         mpz_clear(temp2[i]);
//     }

//     free(b);
//     free(b1);
//     free(b2);
//     free(b5);
//     free(temp1);
//     free(temp2);
// }

void doOperation_FLEQZ(mpz_t **A1, mpz_t **B1, mpz_t *result, int K, int L, int size, int threadID, NodeNetwork net,  SecretShare *ss) {

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
    doOperation_EQZ(temp1, b1, K, size, threadID, net, ss);
    // compute b2 = p_a == p_b
    ss->modSub(temp1, A[1], B[1], size);
    doOperation_EQZ(temp1, b2, L, size, threadID, net, ss);

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



// PRIVATE AND PUBLIC
void doOperation_FLEQZ(mpz_t **A1, int **B1, mpz_t *result, int K, int L, int size, int threadID, NodeNetwork net,  SecretShare *ss) {


    /***********************************************************************/
    mpz_t **A = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    int **B = (int **)malloc(sizeof(int *) * 4);
    for (int i = 0; i < 4; i++) {
        A[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        B[i] = (int *)malloc(sizeof(int) * size);
        for (int j = 0; j < size; j++) {
            mpz_init_set(A[i][j], A1[j][i]);
            B[i][j] = B1[j][i];
        }
    }

    // long long *elements = new long long[4];
    // for (int i = 0; i < size; ++i) {
    //     convertFloat(B1[i], K, L, &elements);
    //     for (int j = 0; j < 4; ++j) {
    //         B[j][i] = (int)elements[j]; 
    //     }
    // }

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
    std::cout << "Start" << std::endl;
    ss->modSub(temp1, A[0], B[0], size); // B[0] -> now an int // -> no change cause line 114 will be called from secrectshare.h
    std::cout << temp1 << std::endl;
    std::cout << A[0] << std::endl;
    std::cout << B[0] << std::endl;
    doOperation_EQZ(temp1, b1, K, size, threadID, net, ss); 
    std::cout << temp1 << std::endl;
    std::cout << b1 << std::endl;
    std::cout << "End" << std::endl;

    // compute b2 = p_a == p_b
    ss->modSub(temp1, A[1], B[1], size);
    doOperation_EQZ(temp1, b2, L, size, threadID, net, ss);

    // compute b3 = 1 - XOR(s_a, s_b) and b5 = AND(z_a, z_b)
    ss->modAdd(temp1, A[3], B[3], size); // line 102 

    // Mult(temp2, A[3], B[3], size, threadID, net, ss); // modMult
    ss->modMul(temp2, A[3], B[3], size); // line 93

    ss->modMul(temp2, temp2, const2, size);
    ss->modSub(temp1, temp1, temp2, size);
    ss->modSub(temp2, const1, temp1, size); // temp2 = b3

    // Mult(b5, A[2], B[2], size, threadID, net, ss); // change to modMult
    ss->modMul(b5, A[2], B[2], size); // line 93

    // compute b4 = AND(b1, b2, b3) // no change cause all b1, b2, b3 are pri no pub involved like the B
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


// Implemented the same as above with flipping the multiplication order while calling modMul functions 
void doOperation_FLEQZ(int **A1, mpz_t **B1, mpz_t *result, int K, int L, int size, int threadID, NodeNetwork net,  SecretShare *ss) {

    /***********************************************************************/
    int **A = (int **)malloc(sizeof(int *) * 4);
    mpz_t **B = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    for (int i = 0; i < 4; i++) {
        A[i] = (int *)malloc(sizeof(int) * size);
        B[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++) {
            A[i][j] = A1[j][i];
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

    // compute b2 = p_a == p_b
    ss->modSub(temp1, A[1], B[1], size);
    doOperation_EQZ(temp1, b2, L, size, threadID, net, ss);

    // compute b3 = 1 - XOR(s_a, s_b) and b5 = AND(z_a, z_b)
    ss->modAdd(temp1, B[3], A[3], size); // line 102 

    ss->modMul(temp2, B[3], A[3], size); // line 93

    ss->modMul(temp2, temp2, const2, size);
    ss->modSub(temp1, temp1, temp2, size);
    ss->modSub(temp2, const1, temp1, size); // temp2 = b3

    ss->modMul(b5, B[2], A[2], size); // line 93

    // compute b4 = AND(b1, b2, b3) // no change cause all b1, b2, b3 are pri no pub involved like the B
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