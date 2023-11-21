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

#include "EQZ.h"

void doOperation_EQZ(mpz_t *shares, mpz_t *result, int K, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    int peers = ss->getPeers();
    int m = ceil(log2(K));
    int m_idx = ss->getCoefIndex(K);

    // printf("K = %i, m = %i, m_idx = %i\n", K, m, m_idx);
    mpz_t *r_pp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *r_pp_2 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *bitK = (mpz_t *)malloc(sizeof(mpz_t) * K);
    mpz_t *bitm = (mpz_t *)malloc(sizeof(mpz_t) * m);
    mpz_t **resultShares = (mpz_t **)malloc(sizeof(mpz_t *) * peers);
    mpz_t *C = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *c = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *c_test = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *sum = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t temp1, temp2, const1, const2, constK, constK_m1, constm, const2K_m1, const2K, const2m;

    // initialization
    mpz_init(temp1);
    mpz_init(temp2);
    mpz_init_set_ui(const1, 1);
    mpz_init_set_ui(const2, 2);
    mpz_init_set_ui(constK, K);
    mpz_init_set_ui(constK_m1, K - 1);
    mpz_init_set_ui(constm, m);
    mpz_init(const2K);
    mpz_init(const2K_m1);
    mpz_init(const2m);

    ss->modPow(const2K, const2, constK);
    ss->modPow(const2K_m1, const2, constK_m1); // New
    ss->modPow(const2m, const2, constm);

    for (int i = 0; i < K; i++)
        mpz_init(bitK[i]);
    for (int i = 0; i < m; i++)
        mpz_init(bitm[i]);

    for (int i = 0; i < size; ++i) {
        mpz_init(C[i]);
        mpz_init(c[i]);
        mpz_init(c_test[i]);
        mpz_init(r_pp[i]);
        mpz_init(r_pp_2[i]);
        mpz_init(sum[i]);
    }

    for (int i = 0; i < peers; ++i) {
        resultShares[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; ++j)
            mpz_init(resultShares[i][j]);
    }

    /**************** EQZ (PART 1): LINE 1-3 ******************/
    mpz_t **V = (mpz_t **)malloc(sizeof(mpz_t *) * (K + 1));
    for (int i = 0; i < K + 1; ++i) {
        V[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; ++j)
            mpz_init(V[i][j]);
    }
    mpz_t **U = (mpz_t **)malloc(sizeof(mpz_t *) * (m + 1));
    for (int i = 0; i < m + 1; ++i) {
        U[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; ++j)
            mpz_init(U[i][j]);
    }

    // testing
    // Open(shares, c_test, size, threadID, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     gmp_printf("input[%i]  = %Zd \n", i, c_test[i]);
    // }

    PRandM(K, size, V, threadID, net, id, ss); // generating r', r'_k-1,...,r'_0
    PRandInt(K, K, size, r_pp, threadID, ss);  // generating r''

    ss->modAdd(C, shares, V[K], size);     // [r'] + [a]
    ss->modMul(r_pp, r_pp, const2K, size); // 2^k*[r'']
    // The below line is in the original protocol but somehow causes results to be incorrect
    // ss->modAdd(C, C, const2K_m1, size);    // ([r'] + [a]) + 2^{k-1}
    ss->modAdd(C, C, r_pp, size); // (2^{k-1} + [r'] + [a]) + 2^k*[r'']

    Open(C, c, size, threadID, net, ss);

    for (int i = 0; i < size; i++) {
        binarySplit(c[i], bitK, K);              // Line 3 of EQZ
        for (int j = 0; j < K; j++) {            // Line 4 of EQZ
            ss->modAdd(temp1, bitK[j], V[j][i]); // [c_i]+[r_i]
            ss->modMul(temp2, bitK[j], V[j][i]); // [c_i]*[r_i]
            ss->modMul(temp2, temp2, const2);    // ([c_i]*[r_i]) * 2
            ss->modSub(temp1, temp1, temp2);     // ([c_i]+[r_i]) - (2*[c_i]*[r_i])
            ss->modAdd(sum[i], sum[i], temp1);   // [sum_i] = temp1 + sum_i-1 + ... + sum_0
        }
    }

    // Open(sum, c_test, size, threadID, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     gmp_printf("sum[%i] = %Zd \n", i, c_test[i]);
    // }

    /**************** EQZ (PART 2): LINE 1-5 of KOrCL ******************/
    PRandM(m, size, U, threadID, net, id, ss); // generating r', r'_m-1,...,r'_0
    PRandInt(K, m, size, r_pp, threadID, ss);  // generating r'' (NEEDS TO BE DIFFERENT THAN r_pp, WAS GETTING INCORRECT RESULTS WHEN RE-USING r_pp????)

    ss->modAdd(C, sum, U[m], size); // reusing C
    ss->modMul(r_pp, r_pp, const2m, size);
    ss->modAdd(C, C, r_pp, size);
    Open(C, c, size, threadID, net, ss); // Line 2 of EQZ

    for (int i = 0; i < size; i++) {
        binarySplit(c[i], bitm, m);
        mpz_set_ui(sum[i], 0);
        for (int j = 0; j < m; j++) {
            ss->modAdd(temp1, bitm[j], U[j][i]);
            ss->modMul(temp2, bitm[j], U[j][i]);
            ss->modMul(temp2, const2, temp2);
            ss->modSub(temp1, temp1, temp2);
            ss->modAdd(sum[i], sum[i], temp1);
        }
    }

    // adding 1 to the sum of all the bits, as prescribed by symmetric func. evaluation
    ss->modAdd(sum, sum, const1, size);

    /************ EQZ (PART 3): evaluate symmetric function  *************/
    mpz_t **T = (mpz_t **)malloc(sizeof(mpz_t *) * m);
    for (int i = 0; i < m; ++i) {
        T[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; ++j) {
            mpz_init(T[i][j]);
        }
    }

    for (int i = 0; i < m; i++) {
        ss->copy(sum, T[i], size); // original
    }

    // works as expected
    doOperation_PrefixMult(T, T, m, size, threadID, net, id, ss);

    // the sum of all the bits
    // symmetric function evaluation works: f(1) = 0, f(2,3,...,m) = 1
    // something wrong above with the input summation
    for (int i = 0; i < size; i++) {
        mpz_set(temp2, ss->coef[m_idx][m]);
        mpz_set_ui(result[i], 0);
        ss->modAdd(result[i], result[i], temp2);
        for (int j = 0; j < m; j++) {
            mpz_set(temp2, ss->coef[m_idx][m - j - 1]);
            ss->modMul(temp1, T[j][i], temp2);
            ss->modAdd(result[i], result[i], temp1);
        }
    }

    ss->modSub(result, const1, result, size); // Line 5 of EQZ
    // used for testing
    // Open(result, c_test, size, threadID, net, ss); // Line 2 of EQZ
    // for (size_t i = 0; i < size; i++) {
    //     gmp_printf("result[%i]  = %Zd \n", i, c_test[i]);
    // }

    /*Free the memory*/
    for (int i = 0; i < K + 1; ++i) {
        for (int j = 0; j < size; ++j)
            mpz_clear(V[i][j]);
        free(V[i]);
    }
    free(V);
    for (int i = 0; i < m + 1; ++i) {
        for (int j = 0; j < size; ++j)
            mpz_clear(U[i][j]);
        free(U[i]);
    }
    free(U);

    mpz_clear(temp1);
    mpz_clear(temp2);
    mpz_clear(const2);
    mpz_clear(const2K);
    mpz_clear(const2m);
    mpz_clear(constK);
    mpz_clear(const1);
    mpz_clear(constK_m1);
    mpz_clear(const2K_m1);

    for (int i = 0; i < size; ++i) {
        mpz_clear(C[i]);
        mpz_clear(c[i]);
        mpz_clear(c_test[i]);
        mpz_clear(sum[i]);
        mpz_clear(r_pp[i]);
        mpz_clear(r_pp_2[i]);
    }
    free(C);
    free(c);
    free(c_test);
    free(sum);
    free(r_pp);
    free(r_pp_2);

    for (int i = 0; i < K; i++)
        mpz_clear(bitK[i]);
    free(bitK);

    for (int i = 0; i < m; i++)
        mpz_clear(bitm[i]);
    free(bitm);

    for (int i = 0; i < peers; ++i) {
        for (int j = 0; j < size; ++j)
            mpz_clear(resultShares[i][j]);
        free(resultShares[i]);
    }
    free(resultShares);

    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < size; ++j) {
            mpz_clear(T[i][j]);
        }
        free(T[i]);
    }
    free(T);
}

void doOperation_EQZ(mpz_t *result, mpz_t *a, mpz_t *b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *sub = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; ++i)
        mpz_init(sub[i]);
    int len = smc_compute_len(alen, blen);
    ss->modSub(sub, a, b, size);
    doOperation_EQZ(sub, result, len, size, threadID, net, id, ss);
    ss_batch_free_operator(&sub, size);
}
void doOperation_EQZ(mpz_t *result, mpz_t *a, int *b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *sub = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; ++i)
        mpz_init(sub[i]);
    int len = smc_compute_len(alen, blen);
    ss->modSub(sub, a, b, size);
    doOperation_EQZ(sub, result, len, size, threadID, net, id, ss);
    ss_batch_free_operator(&sub, size);
}

