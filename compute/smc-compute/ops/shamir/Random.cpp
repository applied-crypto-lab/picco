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

#include "Random.h"

// Source: Catrina and de Hoogh, "Improved Primites for Secure Multiparty Integer Computation," 2010
// Protocols 2.1 and 2.2, pages 4-5

// generating a secret shared random integer
void PRandInt(int K, int M, int size, mpz_t *result, int threadID, SecretShare *ss) {
    // why is this set this way?
    int bits = 48 + K - M;
    // sanitizing destination (so the results variable can be reused in something like EQZ)
    for (int i = 0; i < size; i++) {
        mpz_set_ui(result[i], 0);
    }
    //  generateRandValue checks if threadID is -1 and calls appropriate version
    ss->generateRandValue(bits, size, result, threadID);
}

void PRandBit(int size, mpz_t *results, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    int peers = ss->getPeers();
    mpz_t **resultShares = (mpz_t **)malloc(sizeof(mpz_t *) * peers);
    mpz_t *u = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *v = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t const1, inv2;

    // initialization
    mpz_init_set_ui(const1, 1);
    mpz_init_set_ui(inv2, 2);
    ss->modInv(inv2, inv2);

    for (int i = 0; i < peers; i++) {
        resultShares[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++)
            mpz_init(resultShares[i][j]);
    }

    for (int i = 0; i < size; i++) {
        mpz_init(u[i]);
        mpz_init(v[i]);
    }
    /***********************************************************/
    mpz_t field;
    mpz_init(field);
    ss->getFieldSize(field);
    // PRandFld (generating a uniformly random field element [r])
    // need to add PRZS functionality s.t. [c]_i = [a]_i*[b]_i + [0]_i, then open [c]
    ss->generateRandValue(field, size, u, threadID);
    // MulPub (squaring [r])
    ss->modMul(v, u, u, size);
    net.broadcastToPeers(v, size, resultShares, threadID);
    ss->reconstructSecret(v, resultShares, size);
    // v <- u^(-(q + 1)/4) mod q (q is field size)
    ss->modSqrt(v, v, size);
    ss->modInv(v, v, size);
    // [b] <- (v*[r] + 1)*2^-1 (mod q)
    ss->modMul(results, v, u, size);
    ss->modAdd(results, results, const1, size);
    ss->modMul(results, results, inv2, size);

    // free the memory
    mpz_clear(inv2);
    mpz_clear(const1);
    for (int i = 0; i < peers; i++) {
        for (int j = 0; j < size; j++)
            mpz_clear(resultShares[i][j]);
        free(resultShares[i]);
    }
    free(resultShares);

    for (int i = 0; i < size; i++) {
        mpz_clear(u[i]);
        mpz_clear(v[i]);
    }
    free(u);
    free(v);
}

// Does NOT follow PRandM specification exactly - does not generate [r''] since it can be performed separately and hence optimized
// produces a secret shared random value [r'] and its individual bits [b_{m-1}],...,[b_0]
// results is organized as follows:
// result[M+1][size]
// result[0][size],...,result[M-1][size] contains the individual bits
// result[M][size] contains the random value itself
void PRandM(int M, int size, mpz_t **result, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *tempResult = (mpz_t *)malloc(sizeof(mpz_t) * size * M);
    mpz_t *temp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    unsigned long pow = 1;
    unsigned long i, j;

    // do initialization
    for (i = 0; i < size * M; i++) {
        mpz_init(tempResult[i]);
    }
    for (i = 0; i < size; i++) {
        mpz_init(temp[i]);
    }
    // generating size*M random bits, storing in tempResult
    PRandBit(size * M, tempResult, threadID, net, id, ss);
    for (i = 0; i < size; i++) {
        // using result[M] as accumulator for summation
        mpz_set(result[M][i], tempResult[i]);
        // storing result[0] where it actually is going to go
        mpz_set(result[0][i], tempResult[i]);
    }

    // computing [r'] <- \sum_{i=0}^{m-1} 2^i [b_i]
    // result[M] accumulates the sum
    for (i = 1; i < M; i++) {
        pow = pow << 1;
        for (j = 0; j < size; j++) {
            mpz_set(result[i][j], tempResult[i * size + j]);
            mpz_mul_ui(temp[j], result[i][j], pow);
        }
        ss->modAdd(result[M], result[M], temp, size);
    }

    // free the memory
    for (i = 0; i < size * M; i++)
        mpz_clear(tempResult[i]);
    free(tempResult);

    for (i = 0; i < size; i++)
        mpz_clear(temp[i]);
    free(temp);
}
