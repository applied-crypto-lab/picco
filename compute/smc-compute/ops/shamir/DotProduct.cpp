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
#include "DotProduct.h"

void doOperation_DotProduct(mpz_t *a, mpz_t *b, mpz_t result, int array_size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    int peers = ss->getPeers();
    mpz_t **shares = (mpz_t **)malloc(sizeof(mpz_t *) * peers);
    mpz_t **buffer = (mpz_t **)malloc(sizeof(mpz_t *) * peers);
    mpz_t *data = (mpz_t *)malloc(sizeof(mpz_t));
    mpz_t tmp;

    // initialization
    for (int i = 0; i < peers; i++) {
        shares[i] = (mpz_t *)malloc(sizeof(mpz_t) * 1);
        buffer[i] = (mpz_t *)malloc(sizeof(mpz_t) * 1);
        for (int j = 0; j < 1; j++) {
            mpz_init(shares[i][j]);
            mpz_init(buffer[i][j]);
        }
    }
    mpz_init(tmp);
    mpz_init(data[0]);

    // do computation
    for (int i = 0; i < array_size; i++) {
        mpz_mul(tmp, a[i], b[i]);
        mpz_add(data[0], data[0], tmp);
    }

    ss->getShares(shares, data, 1);
    net.multicastToPeers(shares, buffer, 1, threadID);
    ss->reconstructSecret(data, buffer, 1);
    mpz_set(result, data[0]);

    // free the memory
    for (int i = 0; i < peers; i++) {
        for (int j = 0; j < 1; j++) {
            mpz_clear(shares[i][j]);
            mpz_clear(buffer[i][j]);
        }
        free(shares[i]);
        free(buffer[i]);
    }

    free(shares);
    free(buffer);
    mpz_clear(tmp);
    mpz_clear(data[0]);
    free(data);
}
void doOperation_DotProduct(mpz_t **a, mpz_t **b, mpz_t *result, int batch_size, int array_size, int threadID, NodeNetwork net, int id, SecretShare *ss) {

    int peers = ss->getPeers();
    mpz_t **shares = (mpz_t **)malloc(sizeof(mpz_t *) * peers);
    mpz_t **buffer = (mpz_t **)malloc(sizeof(mpz_t *) * peers);
    mpz_t tmp;

    for (int i = 0; i < peers; i++) {
        shares[i] = (mpz_t *)malloc(sizeof(mpz_t) * batch_size);
        buffer[i] = (mpz_t *)malloc(sizeof(mpz_t) * batch_size);
        for (int j = 0; j < batch_size; j++) {
            mpz_init(shares[i][j]);
            mpz_init(buffer[i][j]);
        }
    }
    mpz_init(tmp);

    for (int i = 0; i < batch_size; i++) {
        for (int j = 0; j < array_size; j++) {
            mpz_mul(tmp, a[i][j], b[i][j]);
            mpz_add(result[i], result[i], tmp);
        }
    }

    ss->getShares(shares, result, batch_size);
    net.multicastToPeers(shares, buffer, batch_size, threadID);
    ss->reconstructSecret(result, buffer, batch_size);

    // free the memory
    for (int i = 0; i < peers; i++) {
        for (int j = 0; j < batch_size; j++) {
            mpz_clear(shares[i][j]);
            mpz_clear(buffer[i][j]);
        }
        free(shares[i]);
        free(buffer[i]);
    }
    free(shares);
    free(buffer);
    mpz_clear(tmp);
}
