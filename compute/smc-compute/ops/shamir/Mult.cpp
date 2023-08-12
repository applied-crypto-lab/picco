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
   but WITHOUT ANY WARRANTY; without even the impled warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with PICCO. If not, see <http://www.gnu.org/licenses/>.
*/
#include "Mult.h"

// Mult::Mult(NodeNetwork nodeNet, int nodeID, SecretShare *s) {
//     net = nodeNet;
//     id = nodeID;
//     ss = s;
// }

// Mult::~Mult() {
//     // TODO Auto-generated destructor stub
// }

// This protocol performs the optimized multiplication proposed by Blanton, Kang, and Yuan (ACNS 2020)
void Mult(mpz_t *C, mpz_t *A, mpz_t *B, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    int peers = ss->getPeers();
    // bool vers = true;
    if (peers == 3) {
        // printf("Using optimized multiplication...\n");
        int id_p1;
        int id_m1;
        mpz_t *rand_id = (mpz_t *)malloc(sizeof(mpz_t) * size);
        mpz_t *temp = (mpz_t *)malloc(sizeof(mpz_t) * size);
        mpz_t **data = (mpz_t **)malloc(sizeof(mpz_t *) * peers);
        mpz_t **buffer = (mpz_t **)malloc(sizeof(mpz_t *) * peers);

        for (int i = 0; i < peers; i++) {
            data[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
            buffer[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        }
        // initialziation
        for (int i = 0; i < peers; i++) {
            for (int j = 0; j < size; j++) {
                mpz_init(data[i][j]);
                mpz_init(buffer[i][j]);
            }
        }
        for (int i = 0; i < size; i++) {
            mpz_init(temp[i]);
            mpz_init(rand_id[i]);
        }
        // start computation
        ss->modMul(temp, A, B, size); // step 1

        id_p1 = (id + 1) % (peers + 1);
        if (id_p1 == 0)
            id_p1 = 1;

        id_m1 = (id - 1) % (peers + 1);
        if (id_m1 == 0)
            id_m1 = peers;

        // secret stored in temp
        // create polynomial, stored in data
        ss->getShares2(temp, rand_id, data, size);

        net.multicastToPeers_Mul(data, size, threadID); // step 4 and step 5

        for (int i = 0; i < size; i++) {
            mpz_set(data[id_m1 - 1][i], temp[i]);
        }

        ss->reconstructSecret(C, data, size); // step 5
        // free memory
        for (int i = 0; i < peers; i++) {
            for (int j = 0; j < size; j++) {
                mpz_clear(data[i][j]);
                mpz_clear(buffer[i][j]);
            }
            free(data[i]);
            free(buffer[i]);
        }
        free(data);
        free(buffer);
        for (int i = 0; i < size; i++) {
            mpz_clear(temp[i]);
            mpz_clear(rand_id[i]);
        }
        free(temp);
        free(rand_id);
    } else {
        // printf("Optimized multiplication\n");
        uint threshold = ss->getThreshold();
        int id_p1;
        int id_m1;

        mpz_t *temp = (mpz_t *)malloc(sizeof(mpz_t) * size);
        mpz_t **rand_buff = (mpz_t **)malloc(sizeof(mpz_t *) * (threshold + 1));

        mpz_t **buffer = (mpz_t **)malloc(sizeof(mpz_t *) * (peers));
        mpz_t **data = (mpz_t **)malloc(sizeof(mpz_t *) * peers);

        for (int i = 0; i < (threshold + 1); i++) {
            rand_buff[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        }
        for (int i = 0; i < peers; i++) {
            buffer[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
            data[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        }
        // initialziation
        for (int i = 0; i < peers; i++) {
            for (int j = 0; j < size; j++) {
                mpz_init(data[i][j]);
                mpz_init(buffer[i][j]);
            }
        }

        for (int i = 0; i < (threshold + 1); i++) {
            for (int j = 0; j < size; j++) {
                mpz_init(rand_buff[i][j]);
            }
        }
        for (int i = 0; i < size; i++) {
            mpz_init(temp[i]);
        }
        // start computation
        ss->modMul(temp, A, B, size); // step 1
        ss->PRG(rand_buff, size, 0);  // step 2
        for (int i = 0; i < size; i++) {
            // putting the [c]_p into last position of rand_buff
            mpz_set(rand_buff[threshold][i], temp[i]);
        }
        ss->getSharesMul(buffer, rand_buff, size); // step 3 and 4

        // step 4? do we use the send/recv IDs defined in SecretShare?
        // sending contents of buffer[0,...,t], recieving into buffer[t+1,...,n]
        net.multicastToPeers_Mul_v2(ss->getSendToIDs(), ss->getRecvFromIDs(), buffer, size, threadID);

        ss->PRG(buffer, size, threshold); // step 5, reusing buffer

        ss->reconstructSecretMult(C, buffer, size); // step 5

        // free memory
        for (int i = 0; i < (threshold + 1); i++) {
            for (int j = 0; j < size; j++) {
                mpz_clear(rand_buff[i][j]);
            }
            free(rand_buff[i]);
        }
        for (int i = 0; i < peers; i++) {
            for (int j = 0; j < size; j++) {
                mpz_clear(buffer[i][j]);
                mpz_clear(data[i][j]);
            }
            free(buffer[i]);
            free(data[i]);
        }
        free(rand_buff);
        free(buffer);
        free(data);
        for (int i = 0; i < size; i++) {
            mpz_clear(temp[i]);
        }
        free(temp);

    }
}
