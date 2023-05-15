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

#include "Open.h"

void Open_Shamir(mpz_t *shares, mpz_t *result, int size, int threadID, NodeNetwork nodeNet, int nodeID, SecretShare *s) {
    uint threshold = s->getThreshold();
    int peers = s->getPeers();

    mpz_t **resultShares = (mpz_t **)malloc(sizeof(mpz_t *) * (threshold + 1));
    for (int i = 0; i < (threshold + 1); i++) {
        resultShares[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++)
            mpz_init(resultShares[i][j]);
    }

    nodeNet.broadcastToPeers(shares, size, resultShares, threadID);
    s->reconstructSecretFromMin(result, resultShares, size);

    // freeing
    for (int i = 0; i < (threshold + 1); i++) {
        for (int j = 0; j < size; j++)
            mpz_clear(resultShares[i][j]);
        free(resultShares[i]);
    }
    free(resultShares);
}
