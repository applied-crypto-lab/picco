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
/*
Formula: 
1) [a] ← LT([p1], [p2], k);
2) [c] ← EQ([p1], [p2], k);
3) [d] ← LT((1 − 2[s1])[v1], (1 − 2[s2])[v2], l + 1);
4) [b+] ← [c][d] + (1 − [c])[a];
5) [b−] ← [c][d] + (1 − [c])(1 − [a]);
6) [b] ← [z1](1 − [z2])(1 − [s2]) + (1 − [z1])[z2][s1] + (1 −
[z1])(1 − [z2])([s1](1 − [s2]) + (1 − [s1])(1 − [s2])[b+] +
[s1][s2][b−]);
7) return [b];

Update:
1. line 1, line 2
2. 

*/
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


// priv, pub // Find what K and L is? 
// sign -> 1 bit, expo -> 8 bit, 23
// paper has K->l and L->K
// L -> bit-len of the expo -> 9 -> 
// K -> bit-len of the man -> 32 -> 
void doOperation_FLLTZ(mpz_t **A1, float *B1, mpz_t *result, int K, int L, int size, int threadID, NodeNetwork net,  SecretShare *ss) {


    // convert somewhere her then store it to B go over the size then get the four values 
    

    /***********************************************************************/
    // first we need to convert b1 in to 4 field for exp, ... 
    // a will be the same 
    mpz_t **A = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    int **B = (int **)malloc(sizeof(int *) * 4);
    for (int i = 0; i < 4; i++) {
        A[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        B[i] = (int *)malloc(sizeof(int) * size); // do allocate size for it
        for (int j = 0; j < size; j++) { // B would be an array of 4 int unsiggned 
            mpz_init_set(A[i][j], A1[j][i]); // the values get flipped here
            // mpz_init_set(B[i][j], B1[j][i]);  // write the values to the B from B1
        }
    }

    // convert here from B1 and write it to 4 elements of array 
    // Allocate array for 4 elements
    long long *elements = new long long[4];
    // for (int i = 0; i < 4; ++i) {
    //     elements[i] = new long long[4]; 
    // }
    
    // Convert B1 to elements
    for (int i = 0; i < size; ++i) {
        convertFloat(B1[i], K, L, &elements);
        for (int j = 0; j < 4; ++j) {
            B[j][i] = (int)elements[j]; // cause all the exp in on the same row for all the cols 
        }
    }
    
    // // set elements to B
    // for (int i = 0; i < size; ++i) {
    //     // B[i] = new mpz_t[4];
    //     for (int j = 0; j < 4; ++j) {
    //         // mpz_init(B[i][j]);
    //         // mpz_set_si(B[i][j], elements[i][j]);
    //         B[i][j] = elements[i];
    //     }
    // }

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
    // ss->modSub(temp1, A[1], B[1], size); // mpzt, priv_int(whatever extracted 4 values)
    // doOperation_LTZ(a, temp1, L, size, threadID, net, ss);


    // int * 
    doOperation_LT(a, A[1], B[1], L, L, L, size, threadID, net, ss);
    // line 2
    // ss->modSub(temp1, A[1], B[1], size);
    // doOperation_EQZ(temp1, c, L, size, threadID, net, ss);

    doOperation_EQZ(c, A[1], B[1], L, L, L, size, threadID, net, ss);
    // line 3
    // stay the same
    ss->modMul(temp1, A[3], const2, size); // mul a*2 
    ss->modSub(temp2, const1, temp1, size); // temp1
    Mult(temp3, temp2, A[0], size, threadID, net, ss); // res in temp3

    // changes to the line below, same compu but different cause b has ints now
    // ss->modMul(temp1, B[3], const2, size);
    // ss->modSub(temp2, const1, temp1, size);
    // Mult(temp1, temp2, B[0], size, threadID, net, ss);

    // ss->modSub(temp1, temp3, temp1, size);
    // doOperation_LTZ(d, temp1, K + 1, size, threadID, net, ss); // temp1 -> A

// Note: 
//     create a tmp array and loop to give only the elements, the type of the tmp should be int and it should store the elements of B[3]
//     just one tmp would be enough. then do the op 
    
    int *temp_arr = (int *)malloc(sizeof(int) * size); // b[4][size]
    for (int i = 0; i < size; i++) {
        temp_arr[i] = (1-2*B[3][i])*B[0][i]; // b[0]->man, B[3]->sign, -> case 1: (1-0)*MAN=>MAN OR case 2: (1-2)*MAN=>-MAN
    } 
    doOperation_LT(d, temp1, temp_arr, K + 1, K + 1, K + 1, size, threadID, net, ss); // K + 1 -> BECAUSE this compares the man values, +1 is needed cause the man can be neg or pos, if neg it would have one more bit 

    // the lines below stay the same 
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

    // these changes cause now B has a different type (priv_int *)
    // b_p1
    // ss->modSub(temp1, const1, B[2], size);
    // ss->modSub(temp2, const1, B[3], size);
    // Mult(temp3, temp1, temp2, size, threadID, net, ss);
    Mult(temp3, temp3, A[2], size, threadID, net, ss); // changed to ss->modMul

    mpz_t *temp_arr1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (size_t i = 0; i < size; i++) {
        mpz_init(temp_arr1[i]);
    }
    for (size_t i = 0; i < size; i++) {
        mpz_set_ui(temp_arr1[i], (1-B[2][i])*(1-B[3][i])); // takes (1-Exp * 1-sign) 
    }
    ss->modMul(temp3, temp_arr1, A[2], size);
    ss->modAdd(b, b, temp3, size);
    // b_p2
    ss->modSub(temp1, const1, A[2], size);
    // Mult(temp2, temp1, B[2], size, threadID, net, ss);
    ss->modMul(temp2, temp1, B[2], size);
    Mult(temp2, temp2, A[3], size, threadID, net, ss);
    ss->modAdd(b, b, temp2, size);
    // b_p
    /* compute (1-[z1]) * (1-[z2]) */
    // ss->modSub(temp1, const1, B[2], size);
    ss->modSub(temp3, const1, A[2], size);
    mpz_t *temp_arr2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (size_t i = 0; i < size; i++) {
        mpz_init(temp_arr2[i]);
    }
    for (size_t i = 0; i < size; i++) {
        mpz_set_ui(temp_arr2[i], 1-B[2][i]);
    }
    ss->modMul(b_p, temp_arr2, temp3, size); // changed too from MUlt
    /* compute [s1] * (1 - [s2])  */
    // ss->modSub(temp1, const1, B[3], size); // temp1 is directly computed on line below, 
    mpz_t *temp_arr3 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (size_t i = 0; i < size; i++) {
        mpz_init(temp_arr3[i]);
    }
    for (size_t i = 0; i < size; i++) {
        mpz_set_ui(temp_arr3[i], 1-B[3][i]);
    }
    ss->modMul(temp2, temp_arr3, A[3], size);
    /* compute (1-[s1]) *(1-[s2]) * b+ */
    ss->modSub(temp1, const1, A[3], size);
    // ss->modSub(temp3, const1, B[3], size);
    // Mult(temp1, temp1, temp3, size, threadID, net, ss);
    for (int i = 0; i < size; i++) {
        temp_arr[i] = 1-B[3][i];
    }
    ss->modMul(temp1, temp1, temp_arr, size);
    Mult(temp3, temp1, b1, size, threadID, net, ss);
    /*  compute [s1] * [s2] * b2 */
    // Mult(temp1, A[3], B[3], size, threadID, net, ss);
    ss->modMul(temp1, A[3], B[3], size); // when pub is used, no need to get anything, we can do local computation
    Mult(temp1, temp1, b2, size, threadID, net, ss);
    ss->modAdd(temp1, temp1, temp2, size);
    ss->modAdd(temp1, temp1, temp3, size);
    Mult(temp3, temp1, b_p, size, threadID, net, ss);
    // add b_p1, b_p2, and b_p
    ss->modAdd(b, b, temp3, size);

    for (int i = 0; i < size; i++)
        mpz_set(result[i], b[i]);

    // free the memory 
    // B should be freed based on its new type not the mpz type 
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < size; j++) {
            mpz_clear(A[i][j]);
            // mpz_clear(B[i][j]);
        }
        free(A[i]);
        free(B[i]);
    }
    free(A);
    free(B);

    free(temp_arr);

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
        mpz_clear(temp_arr1[i]);
        mpz_clear(temp_arr2[i]);
        mpz_clear(temp_arr3[i]);
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



void doOperation_FLLTZ(float *A1, mpz_t **B1, mpz_t *result, int K, int L, int size, int threadID, NodeNetwork net,  SecretShare *ss) {

    /***********************************************************************/
    int **A = (int **)malloc(sizeof(int *) * 4);
    mpz_t **B = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    for (int i = 0; i < 4; i++) {
        A[i] = (int *)malloc(sizeof(int) * size);
        B[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++) {
            mpz_init_set(B[i][j], B1[j][i]);
        }
    }

    // Convert here from B1 and write it to 4 elements of array 
    long long *elements = new long long[4];
    for (int i = 0; i < size; ++i) {
        convertFloat(A1[i], K, L, &elements);
        for (int j = 0; j < 4; ++j) {
            A[j][i] = (int)elements[j]; // cause all the exp in on the same row for all the cols 
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

    // line 1
    doOperation_LT(a, A[1], B[1], L, L, L, size, threadID, net, ss);
    // line 2
    doOperation_EQZ(c, B[1], A[1], L, L, L, size, threadID, net, ss);
    // line 3
    int *temp_arr = (int *)malloc(sizeof(int) * size); // b[4][size]
    for (int i = 0; i < size; i++) {
        temp_arr[i] = (1-2*A[3][i])*A[0][i]; // A[0]->man, A[3]->sign, -> case 1: (1-0)*MAN=>MAN OR case 2: (1-2)*MAN=>-MAN
    }

    ss->modMul(temp1, B[3], const2, size);
    ss->modSub(temp2, const1, temp1, size);
    Mult(temp1, temp2, B[0], size, threadID, net, ss);

    doOperation_LT(d, temp1, temp_arr, K + 1, K + 1, K + 1, size, threadID, net, ss); 

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

    mpz_t *temp_arr1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (size_t i = 0; i < size; i++) {
        mpz_init(temp_arr1[i]);
    }
    for (size_t i = 0; i < size; i++) {
        mpz_set_ui(temp_arr1[i], (1-A[2][i])*(1-A[3][i])); // takes (1-Exp * 1-sign) 
    }

    ss->modAdd(b, b, temp3, size);
    // b_p2
    Mult(temp2, temp1, B[2], size, threadID, net, ss);
    ss->modAdd(b, b, temp2, size);


    // b_p
    /* compute (1-[z1]) * (1-[z2]) */
    ss->modSub(temp1, const1, B[2], size);
    mpz_t *temp_arr2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (size_t i = 0; i < size; i++) {
        mpz_init(temp_arr2[i]);
    }
    for (size_t i = 0; i < size; i++) {
        mpz_set_ui(temp_arr2[i], 1-A[2][i]);
    }
    ss->modMul(b_p, temp_arr2, temp3, size);
    
    /* compute [s1] * (1 - [s2])  */
    ss->modSub(temp1, const1, B[3], size);
    /* compute (1-[s1]) *(1-[s2]) * b+ */
    mpz_t *temp_arr3 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (size_t i = 0; i < size; i++) {
        mpz_init(temp_arr3[i]);
    }
    for (size_t i = 0; i < size; i++) {
        mpz_set_ui(temp_arr3[i], 1-A[3][i]);
    }
    ss->modMul(temp1, temp_arr3, A[3], size);

    for (int i = 0; i < size; i++) {
        temp_arr[i] = 1-A[3][i];
    }
    ss->modSub(temp3, const1, B[3], size);
    ss->modMul(temp1, temp3, temp_arr, size);
    Mult(temp3, temp1, b1, size, threadID, net, ss);
    /*  compute [s1] * [s2] * b2 */
    ss->modMul(temp1, B[3], A[3], size);
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
            mpz_clear(B[i][j]);
        }
        free(A[i]);
        free(B[i]);
    }
    free(A);
    free(B);

    free(temp_arr);

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

// New
// void doOperation_FLLTZ(mpz_t **A1, int **B1, mpz_t *result, int K, int L, int size, int threadID, NodeNetwork net,  SecretShare *ss) {
    
//     std::cout << "priv, pub" << std::endl;
//     /***********************************************************************/
//     mpz_t **A = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
//     int **B = (int **)malloc(sizeof(int *) * 4);
//     for (int i = 0; i < 4; i++) {
//         A[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
//         B[i] = (int *)malloc(sizeof(int) * size); // do allocate size for it
//         for (int j = 0; j < size; j++) { // B would be an array of 4 int unsiggned 
//             mpz_init_set(A[i][j], A1[j][i]); // the values get flipped here
//             B[i][j] = B1[j][i];
//         }
//     }

//     /***********************************************************************/
//     mpz_t const1, const2;
//     mpz_t *a = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *b = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *b1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *b2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *c = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *d = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *b_p = (mpz_t *)malloc(sizeof(mpz_t) * size);

//     mpz_t *temp1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *temp2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *temp3 = (mpz_t *)malloc(sizeof(mpz_t) * size);

//     mpz_init_set_ui(const1, 1);
//     mpz_init_set_ui(const2, 2);

//     for (int i = 0; i < size; i++) {
//         mpz_init(a[i]);
//         mpz_init(b[i]);
//         mpz_init(b1[i]);
//         mpz_init(b2[i]);
//         mpz_init(c[i]);
//         mpz_init(d[i]);
//         mpz_init(b_p[i]);

//         mpz_init(temp1[i]);
//         mpz_init(temp2[i]);
//         mpz_init(temp3[i]);
//     }
//     // gettimeofday(&tv1, NULL);
//     // L = 9;
//     // K = 32;

//     //  line 1
//     doOperation_LT(a, A[1], B[1], L, L, L, size, threadID, net, ss);

//     // line 2
//     doOperation_EQZ(c, A[1], B[1], L, L, L, size, threadID, net, ss);

//     // line 3
//     ss->modMul(temp1, A[3], const2, size); 
//     ss->modSub(temp2, const1, temp1, size); 
//     Mult(temp3, temp2, A[0], size, threadID, net, ss); 
//     int *temp_arr = (int *)malloc(sizeof(int) * size); // b[4][size]
//     for (int i = 0; i < size; i++) {
//         temp_arr[i] = (1-2*B[3][i])*B[0][i]; // b[0]->man, B[3]->sign, -> case 1: (1-0)*MAN=>MAN OR case 2: (1-2)*MAN=>-MAN
//     } 
//     doOperation_LT(d, temp1, temp_arr, K + 1, K + 1, K + 1, size, threadID, net, ss); // K + 1 -> BECAUSE this compares the man values, +1 is needed cause the man can be neg or pos, if neg it would have one more bit 
    
//     // line 4
//     Mult(temp1, c, d, size, threadID, net, ss);
//     ss->modSub(temp2, const1, c, size);
//     Mult(temp3, temp2, a, size, threadID, net, ss);
//     ss->modAdd(b1, temp1, temp3, size);

//     // line 5
//     Mult(temp1, c, d, size, threadID, net, ss);
//     ss->modSub(temp2, const1, c, size);
//     ss->modSub(temp3, const1, a, size);
//     Mult(b2, temp2, temp3, size, threadID, net, ss);
//     ss->modAdd(b2, b2, temp1, size);

//     // line 6
//     // b_p1
//     Mult(temp3, temp3, A[2], size, threadID, net, ss);
//     mpz_t *temp_arr1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     for (size_t i = 0; i < size; i++) {
//         mpz_init(temp_arr1[i]);
//     }
//     for (size_t i = 0; i < size; i++) {
//         mpz_set_ui(temp_arr1[i], (1-B[2][i])*(1-B[3][i])); // takes (1-Exp * 1-sign) 
//     }
//     ss->modMul(temp3, temp_arr1, A[2], size);
//     ss->modAdd(b, b, temp3, size);

//     // b_p2
//     ss->modSub(temp1, const1, A[2], size);
//     ss->modMul(temp2, temp1, B[2], size);
//     Mult(temp2, temp2, A[3], size, threadID, net, ss);
//     ss->modAdd(b, b, temp2, size);

//     // b_p
//     /* compute (1-[z1]) * (1-[z2]) */
//     ss->modSub(temp3, const1, A[2], size);
//     mpz_t *temp_arr2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     for (size_t i = 0; i < size; i++) {
//         mpz_init(temp_arr2[i]);
//     }
//     for (size_t i = 0; i < size; i++) {
//         mpz_set_ui(temp_arr2[i], 1-B[2][i]);
//     }
//     ss->modMul(b_p, temp_arr2, temp3, size);

//     /* compute [s1] * (1 - [s2])  */
//     mpz_t *temp_arr3 = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     for (size_t i = 0; i < size; i++) {
//         mpz_init(temp_arr3[i]);
//     }
//     for (size_t i = 0; i < size; i++) {
//         mpz_set_ui(temp_arr3[i], 1-B[3][i]);
//     }
//     ss->modMul(temp2, temp_arr3, A[3], size);

//     /* compute (1-[s1]) *(1-[s2]) * b+ */
//     ss->modSub(temp1, const1, A[3], size);
//     for (int i = 0; i < size; i++) {
//         temp_arr[i] = 1-B[3][i];
//     }
//     ss->modMul(temp1, temp1, temp_arr, size);
//     Mult(temp3, temp1, b1, size, threadID, net, ss);

//     /*  compute [s1] * [s2] * b2 */
//     ss->modMul(temp1, A[3], B[3], size);
//     Mult(temp1, temp1, b2, size, threadID, net, ss);
//     ss->modAdd(temp1, temp1, temp2, size);
//     ss->modAdd(temp1, temp1, temp3, size);
//     Mult(temp3, temp1, b_p, size, threadID, net, ss);

//     // add b_p1, b_p2, and b_p
//     ss->modAdd(b, b, temp3, size);

//     for (int i = 0; i < size; i++)
//         mpz_set(result[i], b[i]);

//     // free the memory 
//     for (int i = 0; i < 4; i++) {
//         for (int j = 0; j < size; j++) {
//             mpz_clear(A[i][j]);
//         }
//         free(A[i]);
//         free(B[i]);
//     }
//     free(A);
//     free(B);

//     free(temp_arr);

//     for (int i = 0; i < size; i++) {
//         mpz_clear(a[i]);
//         mpz_clear(b[i]);
//         mpz_clear(b1[i]);
//         mpz_clear(b2[i]);
//         mpz_clear(c[i]);
//         mpz_clear(d[i]);
//         mpz_clear(b_p[i]);
//         mpz_clear(temp1[i]);
//         mpz_clear(temp2[i]);
//         mpz_clear(temp3[i]);
//         mpz_clear(temp_arr1[i]);
//         mpz_clear(temp_arr2[i]);
//         mpz_clear(temp_arr3[i]);
//     }

//     free(a);
//     free(b);
//     free(b1);
//     free(b2);
//     free(c);
//     free(d);
//     free(b_p);
//     free(temp1);
//     free(temp2);
//     free(temp3);
//     mpz_clear(const1);
//     mpz_clear(const2);
// }



// void doOperation_FLLTZ(int **A1, mpz_t **B1, mpz_t *result, int K, int L, int size, int threadID, NodeNetwork net,  SecretShare *ss) {

//     std::cout << "pub, priv" << std::endl;


//     /***********************************************************************/
//     int **A = (int **)malloc(sizeof(int *) * 4);
//     mpz_t **B = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
//     for (int i = 0; i < 4; i++) {
//         A[i] = (int *)malloc(sizeof(int) * size);
//         B[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
//         for (int j = 0; j < size; j++) {
//             A[i][j] = A1[j][i];
//             mpz_init_set(B[i][j], B1[j][i]);
//         }
//     }

//     /***********************************************************************/
//     mpz_t const1, const2;
//     mpz_t *a = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *b = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *b1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *b2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *c = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *d = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *b_p = (mpz_t *)malloc(sizeof(mpz_t) * size);

//     mpz_t *temp1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *temp2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *temp3 = (mpz_t *)malloc(sizeof(mpz_t) * size);

//     mpz_init_set_ui(const1, 1);
//     mpz_init_set_ui(const2, 2);

//     for (int i = 0; i < size; i++) {
//         mpz_init(a[i]);
//         mpz_init(b[i]);
//         mpz_init(b1[i]);
//         mpz_init(b2[i]);
//         mpz_init(c[i]);
//         mpz_init(d[i]);
//         mpz_init(b_p[i]);

//         mpz_init(temp1[i]);
//         mpz_init(temp2[i]);
//         mpz_init(temp3[i]);
//     }
//     // gettimeofday(&tv1, NULL);
//     // L = 9;
//     // K = 32;

//     //  line 1
//     /* compute [a] ← LT([p1], [p2], k) */
//     ss->modSub(temp1, A[1], B[1], size);
//     doOperation_LTZ(a, temp1, L, size, threadID, net, ss);

//     // line 2
//     /* compute [c] ← EQ([p1], [p2], k) */
//     ss->modSub(temp1, A[1], B[1], size);
//     doOperation_EQZ(temp1, c, L, size, threadID, net, ss);

//     // line 3
//     /* compute [d] ← LT((1 − 2[s1])[v1], (1 − 2[s2])[v2], l(K here) + 1) */
//     int *temp_arr = (int *)malloc(sizeof(int) * size); // b[4][size]
//     for (int i = 0; i < size; i++) {
//         temp_arr[i] = (1-2*A[3][i])*A[0][i]; 
//     } 
//     ss->modMul(temp1, B[3], const2, size);
//     ss->modSub(temp2, const1, temp1, size);
//     Mult(temp1, temp2, B[0], size, threadID, net, ss);
//     doOperation_LT(d, temp1, temp_arr, K + 1, K + 1, K + 1, size, threadID, net, ss);

//     // line 4
//     /* compute [b+] ← [c][d] + (1 − [c])[a] */
//     Mult(temp1, c, d, size, threadID, net, ss);
//     ss->modSub(temp2, const1, c, size);
//     Mult(temp3, temp2, a, size, threadID, net, ss);
//     ss->modAdd(b1, temp1, temp3, size);

//     // line 5
//     /* compute [b−] ← [c][d] + (1 − [c])(1 − [a]) */
//     Mult(temp1, c, d, size, threadID, net, ss);
//     ss->modSub(temp2, const1, c, size);
//     ss->modSub(temp3, const1, a, size);
//     Mult(b2, temp2, temp3, size, threadID, net, ss);
//     ss->modAdd(b2, b2, temp1, size);

//     // line 6
//     // b_p1
//     ss->modSub(temp1, const1, B[2], size);
//     ss->modSub(temp2, const1, B[3], size);
//     Mult(temp3, temp1, temp2, size, threadID, net, ss);
//     ss->modMul(temp3, temp3, A[2], size);
//     ss->modAdd(b, b, temp3, size);

//     // b_p2
//     int *temp_arr1 = (int *)malloc(sizeof(int) * size);
//     for (size_t i = 0; i < size; i++) {
//         temp_arr1[i] = (1-A[2][i]); // takes (1-Exp * 1-sign) 
//     }
//     ss->modMul(temp2, B[2], temp_arr1, size);
//     ss->modMul(temp2, temp2, A[3], size);
//     ss->modAdd(b, b, temp2, size);

//     // b_p
//     /* compute (1-[z1]) * (1-[z2]) */
//     ss->modSub(temp1, const1, B[2], size);
//     ss->modMul(b_p, temp1, temp_arr1, size);
    
//     /* compute [s1] * (1 - [s2])  */
//     ss->modSub(temp1, const1, B[3], size);
//     ss->modMul(temp2, temp1, A[3], size);
    
//     /* compute (1-[s1]) *(1-[s2]) * b+ */
//     for (size_t i = 0; i < size; i++) {
//         temp_arr1[i] = (1-A[3][i]); // takes (1-Exp * 1-sign) 
//     }
//     ss->modSub(temp3, const1, B[3], size);
//     ss->modMul(temp1, temp3, temp_arr1, size);
//     Mult(temp3, temp1, b1, size, threadID, net, ss); 
    
//     /*  compute [s1] * [s2] * b2 */
//     ss->modMul(temp1, B[3], A[3], size);
//     Mult(temp1, temp1, b2, size, threadID, net, ss);
//     ss->modAdd(temp1, temp1, temp2, size);
//     ss->modAdd(temp1, temp1, temp3, size);
//     Mult(temp3, temp1, b_p, size, threadID, net, ss);
    
//     // add b_p1, b_p2, and b_p
//     ss->modAdd(b, b, temp3, size);

//     for (int i = 0; i < size; i++)
//         mpz_set(result[i], b[i]);

//     // free the memory
//     for (int i = 0; i < 4; i++) {
//         for (int j = 0; j < size; j++) {
//             mpz_clear(B[i][j]);
//         }
//         free(A[i]);
//         free(B[i]);
//     }
//     free(A);
//     free(B);

//     for (int i = 0; i < size; i++) {
//         mpz_clear(a[i]);
//         mpz_clear(b[i]);
//         mpz_clear(b1[i]);
//         mpz_clear(b2[i]);
//         mpz_clear(c[i]);
//         mpz_clear(d[i]);
//         mpz_clear(b_p[i]);
//         mpz_clear(temp1[i]);
//         mpz_clear(temp2[i]);
//         mpz_clear(temp3[i]);
//     }

//     free(a);
//     free(b);
//     free(b1);
//     free(b2);
//     free(c);
//     free(d);
//     free(b_p);
//     free(temp1);
//     free(temp2);
//     free(temp3);
//     mpz_clear(const1);
//     mpz_clear(const2);
// }
