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

#include "EQZ.h"

EQZ::EQZ(NodeNetwork nodeNet, std::map<std::string, std::vector<int>> poly, int NodeID, SecretShare *s) {

    PreMul = new PrefixMultiplication(nodeNet, poly, NodeID, s);
    Rand = new Random(nodeNet, poly, NodeID, s);

    net = nodeNet;
    id = NodeID;
    ss = s;

    setCoef();
}

void EQZ::setCoef() {
    // printf("setCoef\n");
    // combined code for m=5 (uses coef5) and m=8 (uses coef)
    // memory for coef (of size 9) and coef5 (of size 6) was assumed to be allocated elsewhere
    mpz_t temp1, temp2, zero;
    mpz_init(temp1);
    mpz_init(temp2);
    mpz_init_set_ui(zero, 0);

    for (int i = 0; i < 9; i++)
        mpz_init(coef[i]);

    mpz_set(coef[8], zero);

    mpz_set_ui(temp1, 40320);
    mpz_set_ui(temp2, 109584);
    ss->modInv(temp1, temp1);
    mpz_set(coef[7], temp1);
    ss->modMul(coef[7], coef[7], temp2);

    mpz_set_ui(temp2, 118124);
    mpz_set(coef[6], temp1);
    ss->modMul(coef[6], coef[6], temp2);
    ss->modSub(coef[6], zero, coef[6]);

    mpz_set_ui(temp2, 67284);
    mpz_set(coef[5], temp1);
    ss->modMul(coef[5], coef[5], temp2);

    mpz_set_ui(temp2, 22449);
    mpz_set(coef[4], temp1);
    ss->modMul(coef[4], coef[4], temp2);
    ss->modSub(coef[4], zero, coef[4]);

    mpz_set_ui(temp2, 4536);
    mpz_set(coef[3], temp1);
    ss->modMul(coef[3], coef[3], temp2);

    mpz_set_ui(temp2, 546);
    mpz_set(coef[2], temp1);
    ss->modMul(coef[2], coef[2], temp2);
    ss->modSub(coef[2], zero, coef[2]);

    mpz_set_ui(temp2, 36);
    mpz_set(coef[1], temp1);
    ss->modMul(coef[1], coef[1], temp2);

    mpz_set(coef[0], temp1);
    ss->modSub(coef[0], zero, coef[0]);

    // code for coef5
    for (int i = 0; i < 6; i++)
        mpz_init(coef5[i]);
    // the polynomial is f(x) = (x^5 - 20x^4 + 155x^3 - 580x^2 + 1044x - 600)*(120^-1)

    mpz_set_ui(temp1, 120);
    ss->modInv(temp1, temp1);

    mpz_set_ui(temp2, 600);
    mpz_set(coef5[5], temp1);
    ss->modMul(coef5[5], coef5[5], temp2);
    ss->modSub(coef5[5], zero, coef5[5]);

    mpz_set_ui(temp2, 1044);
    mpz_set(coef5[4], temp1);
    ss->modMul(coef5[4], coef5[4], temp2);

    mpz_set_ui(temp2, 580);
    mpz_set(coef5[3], temp1);
    ss->modMul(coef5[3], coef5[3], temp2);
    ss->modSub(coef5[3], zero, coef5[3]);

    mpz_set_ui(temp2, 155);
    mpz_set(coef5[2], temp1);
    ss->modMul(coef5[2], coef5[2], temp2);

    mpz_set_ui(temp2, 20);
    mpz_set(coef5[1], temp1);
    ss->modMul(coef5[1], coef5[1], temp2);
    ss->modSub(coef5[1], zero, coef5[1]);

    mpz_set(coef5[0], temp1);

    mpz_clear(zero);
    mpz_clear(temp1);
    mpz_clear(temp2);
}

EQZ::~EQZ() {
    // TODO Auto-generated destructor stub
}

// Source: Catrina and de Hoogh, "Improved Primites for Secure Multiparty Integer Computation," 2010
// Protocol 3.7, page 9
// Uses Symmetric Function subprotocol from Damgard et al., "Unconditionally Secure Constant-Rounds Multi-party
// Computation for Equality, Comparison, Bits and Exponentiation", 2006
// This is the only protocol that used the "coefficients" parameter, and hence why it was relegated to an EQZ class member (created/filled in constructor)
void EQZ::doOperation(mpz_t *shares, mpz_t *result, int K, int size, int threadID) {
    // for (size_t i = 0; i < size; i++) {
    //     gmp_printf("shares[%i]  = %Zd \n", i, shares[i]);
    // }
    int peers = ss->getPeers();
    // int m;
    // if (ceil(log2(K)) <= 5) {
    //     m = 5;
    // } else {
    //     m = 8;
    // }

    // to be commented in once ss->coef is verified to be correct
    // MAKE SURE TO COMMENT ABOVE OUT
    int m = ceil(log2(K));
    int m_idx = ss->getCoefIndex(K);

    printf("K = %i, m = %i, m_idx = %i\n", K, m, m_idx);
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
    Open(shares, c_test, size, threadID, net, ss);
    for (size_t i = 0; i < size; i++) {
        gmp_printf("input[%i]  = %Zd \n", i, c_test[i]);
    }

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

    Open(sum, c_test, size, threadID, net, ss);
    for (size_t i = 0; i < size; i++) {
        gmp_printf("sum[%i] = %Zd \n", i, c_test[i]);
    }

    /**************** EQZ (PART 2): LINE 1-5 of KOrCL ******************/
    PRandM(m, size, U, threadID, net, id, ss);  // generating r', r'_m-1,...,r'_0
    PRandInt(K, m, size, r_pp, threadID, ss); // generating r'' (NEEDS TO BE DIFFERENT THAN r_pp, WAS GETTING INCORRECT RESULTS WHEN RE-USING r_pp????)

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
    PreMul->doOperation(T, T, m, size, threadID);

    // the sum of all the bits
    // symmetric function evaluation works: f(1) = 0, f(2,3,...,m) = 1
    // something wrong above with the input summation
    for (int i = 0; i < size; i++) {
        mpz_set(temp2, ss->coef[m_idx][m]);
        mpz_set_ui(result[i], 0);
        ss->modAdd(result[i], result[i], temp2);
        for (int j = 0; j < m; j++) {
            // gmp_printf("coef[%i][%i] %Zd\n",m_idx, m - j - 1,ss->coef[m_idx][m - j - 1]);
            mpz_set(temp2, ss->coef[m_idx][m - j - 1]);
            ss->modMul(temp1, T[j][i], temp2);
            ss->modAdd(result[i], result[i], temp1);
        }
    }

    ss->modSub(result, const1, result, size); // Line 5 of EQZ
    // used for testing
    Open(result, c_test, size, threadID, net, ss); // Line 2 of EQZ
    for (size_t i = 0; i < size; i++) {
        gmp_printf("result[%i]  = %Zd \n", i, c_test[i]);
    }

    // replace with ss->coef[m_idx][...] below
    // if (m == 8) {
    //     for (int i = 0; i < size; i++) {
    //         mpz_set(temp2, coef[m]);
    //         mpz_set_ui(result[i], 0);
    //         ss->modAdd(result[i], result[i], temp2);
    //         for (int j = 0; j < m; j++) {
    //             mpz_set(temp2, coef[m - j - 1]);
    //             ss->modMul(temp1, T[j][i], temp2);
    //             ss->modAdd(result[i], result[i], temp1);
    //         }
    //         ss->modSub(result[i], const1, result[i]); // Line 5 of EQZ
    //     }

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

void EQZ::doOperation_EQZ(mpz_t *result, mpz_t *a, mpz_t *b, int alen, int blen, int resultlen, int size, int threadID) {
    mpz_t *sub = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; ++i)
        mpz_init(sub[i]);
    int len = smc_compute_len(alen, blen);
    ss->modSub(sub, a, b, size);
    doOperation(sub, result, len, size, threadID);
    smc_batch_free_operator(&sub, size);
}

void EQZ::doOperation_EQZ(mpz_t result, mpz_t a, mpz_t b, int alen, int blen, int resultlen, int threadID) {
    mpz_t sub;
    mpz_init(sub);
    ss->modSub(sub, a, b);
    mpz_t *results = (mpz_t *)malloc(sizeof(mpz_t));
    mpz_t *subs = (mpz_t *)malloc(sizeof(mpz_t));
    mpz_init_set(subs[0], sub);
    mpz_init(results[0]);

    int len = smc_compute_len(alen, blen);
    doOperation(subs, results, len, 1, threadID);
    mpz_set(result, results[0]);

    mpz_clear(sub);
    smc_batch_free_operator(&results, 1);
    smc_batch_free_operator(&subs, 1);
}

void EQZ::doOperation_EQZ(mpz_t result, mpz_t a, int b, int alen, int blen, int resultlen, int threadID) {

    mpz_t sub;
    mpz_t b_tmp;

    mpz_init_set_si(b_tmp, b);
    mpz_init(sub);
    ss->modSub(sub, a, b_tmp);
    mpz_t *results = (mpz_t *)malloc(sizeof(mpz_t));
    mpz_t *subs = (mpz_t *)malloc(sizeof(mpz_t));

    mpz_init_set(subs[0], sub);
    mpz_init(results[0]);

    int len = smc_compute_len(alen, blen);
    doOperation(results, subs, len, 1, threadID);
    mpz_set(result, results[0]);

    // free the memory
    mpz_clear(sub);
    mpz_clear(b_tmp);

    smc_batch_free_operator(&subs, 1);
    smc_batch_free_operator(&results, 1);
}
