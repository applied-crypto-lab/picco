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

// Source: Catrina and de Hoogh, "Improved Primites for Secure Multiparty Integer Computation," 2010
// Protocol 3.6, page 9
void doOperation_LTZ(mpz_t *result, mpz_t *shares, int K, int size, int threadID, NodeNetwork net,  SecretShare *ss) {
    mpz_t const0;
    mpz_init_set_ui(const0, 0);
    doOperation_Trunc(result, shares, K, K - 1, size, threadID, net, ss);
    ss->modSub(result, const0, result, size);
    mpz_clear(const0);
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

