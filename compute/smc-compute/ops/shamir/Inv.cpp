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

#include "Inv.h"

// where is this protocol from??????
// referenced as a building block in Aliasgari et al., 2013
void doOperation_Inv(mpz_t *shares, mpz_t *results, int size, int threadID, NodeNetwork net,  SecretShare *ss) {
    int peers = ss->getPeers();

    mpz_t *R = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t **buffer = (mpz_t **)malloc(sizeof(mpz_t *) * peers);
    mpz_t *temp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // initialziation
    for (int i = 0; i < peers; i++) {
        buffer[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++)
            mpz_init(buffer[i][j]);
    }

    for (int i = 0; i < size; i++) {
        mpz_init(R[i]);
        mpz_init(temp[i]);
    }

    // start computation
    mpz_t field; //
    mpz_init(field);
    ss->getFieldSize(field);
    ss->generateRandValue(field, size, R, threadID); //

    // Open_print(shares, "Inv shares ", size, threadID, net, ss);
    // Open_print(R, "Inv R ", size, threadID, net, ss);

    ss->modMul(temp, shares, R, size);
    // net.broadcastToPeers(temp, size, buffer, threadID);
    // ss->reconstructSecret(results, buffer, size);
    // Open(temp, results, size, threadID, net, ss); // why does this not work?
    Open_from_all(temp, results, size, threadID, net, ss); // only works when we reconstruct from all
    // Open_print(results, "Inv R*shares ", size, threadID, net, ss);

    ss->modInv(results, results, size);
    ss->modMul(results, R, results, size);

    // memory free
    for (int i = 0; i < peers; i++) {
        for (int j = 0; j < size; j++)
            mpz_clear(buffer[i][j]);
        free(buffer[i]);
    }
    free(buffer);

    for (int i = 0; i < size; i++) {
        mpz_clear(R[i]);
        mpz_clear(temp[i]);
    }
    free(R);
    free(temp);
}
