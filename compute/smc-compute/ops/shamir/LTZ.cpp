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

#include "LTZ.h"

// Source: Catrina and de Hoogh, "Improved Primitives for Secure Multiparty Integer Computation," 2010
// Protocol 3.6, page 9
// LTZ(x) returns 1 if x < 0, else 0
// For K-bit signed x in range [-2^{K-1}, 2^{K-1}):
//   1. Shift to unsigned: y = x + 2^{K-1} (now y in [0, 2^K))
//   2. Extract MSB: msb = Trunc(y, K, K-1) = floor(y / 2^{K-1})
//      - If x < 0: y < 2^{K-1}, so msb = 0
//      - If x >= 0: y >= 2^{K-1}, so msb = 1
//   3. Return 1 - msb
void doOperation_LTZ(mpz_t *result, mpz_t *shares, int K, int size, int threadID, NodeNetwork net,  SecretShare *ss) {
    mpz_t const1, const2, power, const2K_m1;
    mpz_init_set_ui(const1, 1);
    mpz_init_set_ui(const2, 2);
    mpz_init_set_ui(power, K - 1);
    mpz_init(const2K_m1);
    ss->modPow(const2K_m1, const2, power);  // 2^{K-1}

    // Allocate temp array for shifted values
    mpz_t *shifted = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++) {
        mpz_init(shifted[i]);
    }

    // Step 1: Shift to unsigned range: y = x + 2^{K-1}
    ss->modAdd(shifted, shares, const2K_m1, size);

    // Step 2: Extract MSB via truncation: msb = Trunc(y, K, K-1)
    doOperation_Trunc(result, shifted, K, K - 1, size, threadID, net, ss);

    // Step 3: Return 1 - msb (so negative gives 1, non-negative gives 0)
    ss->modSub(result, const1, result, size);

    // Cleanup
    for (int i = 0; i < size; i++) {
        mpz_clear(shifted[i]);
    }
    free(shifted);
    mpz_clear(const1);
    mpz_clear(const2);
    mpz_clear(power);
    mpz_clear(const2K_m1);
}

// computes  a <? b or a >? b (if args are reversed)
void doOperation_LT(mpz_t *result, mpz_t *a, mpz_t *b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net,  SecretShare *ss) {
    mpz_t *sub = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; ++i)
        mpz_init(sub[i]);
    int len = smc_compute_len(alen, blen);
    ss->modSub(sub, a, b, size);
    doOperation_LTZ(result, sub, len, size, threadID, net, ss);
    ss_batch_free_operator(&sub, size);
}

void doOperation_LT(mpz_t *result, int *a, mpz_t *b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net,  SecretShare *ss) {
    mpz_t *sub = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; ++i)
        mpz_init(sub[i]);
    int len = smc_compute_len(alen, blen);
    ss->modSub(sub, a, b, size);
    doOperation_LTZ(result, sub, len, size, threadID, net, ss);
    ss_batch_free_operator(&sub, size);
}

void doOperation_LT(mpz_t *result, mpz_t *a, int *b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net,  SecretShare *ss) {
    mpz_t *sub = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; ++i)
        mpz_init(sub[i]);
    int len = smc_compute_len(alen, blen);
    ss->modSub(sub, a, b, size);
    doOperation_LTZ(result, sub, len, size, threadID, net, ss);
    ss_batch_free_operator(&sub, size);
}

// //helper functions that interface with SMC_utils
// // reversing the parameters computes GT
// void doOperation_LT(mpz_t result, mpz_t a, mpz_t b, int alen, int blen, int resultlen, int threadID, NodeNetwork net,  SecretShare *ss) {
//     mpz_t sub;
//     mpz_init(sub);
//     ss->modSub(sub, a, b);
//     mpz_t *results = (mpz_t *)malloc(sizeof(mpz_t));
//     mpz_t *subs = (mpz_t *)malloc(sizeof(mpz_t));

//     mpz_init_set(subs[0], sub);
//     mpz_init(results[0]);

//     int len = smc_compute_len(alen, blen);
//     doOperation_LTZ(results, subs, len, 1, threadID, net, ss);
//     mpz_set(result, results[0]);

//     // free the memory
//     mpz_clear(sub);
//     ss_batch_free_operator(&subs, 1);
//     ss_batch_free_operator(&results, 1);
// }

// void doOperation_LT(mpz_t result, mpz_t a, int b, int alen, int blen, int resultlen, int threadID, NodeNetwork net,  SecretShare *ss) {
//     mpz_t sub;
//     mpz_t b_tmp;

//     mpz_init_set_si(b_tmp, b);
//     mpz_init(sub);
//     ss->modSub(sub, a, b_tmp);
//     mpz_t *results = (mpz_t *)malloc(sizeof(mpz_t));
//     mpz_t *subs = (mpz_t *)malloc(sizeof(mpz_t));

//     mpz_init_set(subs[0], sub);
//     mpz_init(results[0]);

//     int len = smc_compute_len(alen, blen);
//     doOperation_LTZ(results, subs, len, 1, threadID, net, ss);
//     mpz_set(result, results[0]);

//     // free the memory
//     mpz_clear(sub);
//     mpz_clear(b_tmp);

//     ss_batch_free_operator(&subs, 1);
//     ss_batch_free_operator(&results, 1);
// }

// void doOperation_LT(mpz_t result, int a, mpz_t b, int alen, int blen, int resultlen, int threadID, NodeNetwork net,  SecretShare *ss) {
//     mpz_t a_tmp;
//     mpz_t sub;
//     mpz_init_set_si(a_tmp, a);
//     mpz_init(sub);
//     ss->modSub(sub, a_tmp, b);
//     mpz_t *results = (mpz_t *)malloc(sizeof(mpz_t));
//     mpz_t *subs = (mpz_t *)malloc(sizeof(mpz_t));

//     mpz_init_set(subs[0], sub);
//     mpz_init(results[0]);

//     int len = smc_compute_len(alen, blen);
//     doOperation_LTZ(results, subs, len, 1, threadID, net, ss);
//     mpz_set(result, results[0]);

//     // free the memory
//     ss_batch_free_operator(&subs, 1);
//     ss_batch_free_operator(&results, 1);
//     mpz_clear(sub);
//     mpz_clear(a_tmp);
// }

