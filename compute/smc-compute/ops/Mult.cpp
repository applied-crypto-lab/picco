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
#include "Mult.h"

Mult::Mult(NodeNetwork nodeNet, int nodeID, SecretShare *s) {
    net = nodeNet;
    id = nodeID;
    ss = s;
}

Mult::~Mult() {
    // TODO Auto-generated destructor stub
}

// This protocol performs integer multiplication as defined by Gennaro, Rabin, and Rabin (PODC 1998)
// In the 3-party setting, the program switches to the optimized multiplication proposed by Blanton, Kang, and Yuan (ACNS 2020)
void Mult::doOperation(mpz_t *C, mpz_t *A, mpz_t *B, int size, int threadID) {
    int peers = ss->getPeers();
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
        uint threshold = ss->getThreshold();
        int id_p1;
        int id_m1;

        mpz_t *rand_id = (mpz_t *)malloc(sizeof(mpz_t) * size);
        mpz_t *temp = (mpz_t *)malloc(sizeof(mpz_t) * size);
        mpz_t **buffer = (mpz_t **)malloc(sizeof(mpz_t *) * (threshold + 1));
        mpz_t **rand_buff = (mpz_t **)malloc(sizeof(mpz_t *) * (threshold + 1));

        mpz_t **data = (mpz_t **)malloc(sizeof(mpz_t *) * peers);

        for (int i = 0; i < (threshold + 1); i++) {
            buffer[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
            rand_buff[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        }
        for (int i = 0; i < peers; i++) {
            data[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        }
        // initialziation
        for (int i = 0; i < peers; i++) {
            for (int j = 0; j < size; j++) {
                mpz_init(data[i][j]);
            }
        }


        for (int i = 0; i < (threshold + 1); i++) {
            for (int j = 0; j < size; j++) {
                // mpz_init(data[i][j]);
                mpz_init(buffer[i][j]);
                mpz_init(rand_buff[i][j]);
            }
        }
        for (int i = 0; i < size; i++) {
            mpz_init(temp[i]);
            mpz_init(rand_id[i]);
        }
        // start computation
        ss->modMul(temp, A, B, size); // step 1

        ss->PRG(rand_buff, size, 0); // step 2
        for (int i = 0; i < size; i++) {
            // putting the [c]_p into last position of rand_buff
            // does this matter where [c]_p goes relative to getSharesMul?
            mpz_set(rand_buff[threshold][i], temp[i]); 
        }

        ss->getSharesMul(buffer, rand_buff, size); // step 3

        ss->PRG(rand_buff, size, threshold); // step 5, reusing rand_buffer

        // id_p1 = (id + 1) % (peers + 1);
        // if (id_p1 == 0)
        //     id_p1 = 1;

        // id_m1 = (id - 1) % (peers + 1);
        // if (id_m1 == 0)
        //     id_m1 = peers;

        // // secret stored in temp
        // // create polynomial, stored in data
        // ss->getShares2(temp, rand_id, data, size);

        // net.multicastToPeers_Mul(data, size, threadID); // step 4 and step 5

        // for (int i = 0; i < size; i++) {
        //     mpz_set(data[id_m1 - 1][i], temp[i]);
        // }

        ss->reconstructSecret(C, data, size); // step 5
        // free memory
        for (int i = 0; i < (threshold+1); i++) {
            for (int j = 0; j < size; j++) {
                mpz_clear(rand_buff[i][j]);
                mpz_clear(buffer[i][j]);
            }
            free(rand_buff[i]);
            free(buffer[i]);
        }
        for (int i = 0; i < peers; i++) {
            for (int j = 0; j < size; j++) {
                mpz_clear(data[i][j]);
            }
            free(data[i]);
        }
        free(rand_buff);
        free(buffer);
        free(data);
        for (int i = 0; i < size; i++) {
            mpz_clear(temp[i]);
            mpz_clear(rand_id[i]);
        }
        free(temp);
        free(rand_id);

        // GRR legacy multiplication
        // mpz_t *temp = (mpz_t *)malloc(sizeof(mpz_t) * size);
        // mpz_t **data = (mpz_t **)malloc(sizeof(mpz_t *) * peers);
        // mpz_t **buffer = (mpz_t **)malloc(sizeof(mpz_t *) * peers);

        // for (int i = 0; i < peers; i++) {
        //     data[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        //     buffer[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        // }
        // // initialziation
        // for (int i = 0; i < peers; i++) {
        //     for (int j = 0; j < size; j++) {
        //         mpz_init(data[i][j]);
        //         mpz_init(buffer[i][j]);
        //     }
        // }
        // for (int i = 0; i < size; i++)
        //     mpz_init(temp[i]);
        // // start computation
        // ss->modMul(temp, A, B, size);
        // ss->getShares(data, temp, size);
        // net.multicastToPeers(data, buffer, size, threadID);
        // ss->reconstructSecret(C, buffer, size);
        // // free memory
        // for (int i = 0; i < peers; i++) {
        //     for (int j = 0; j < size; j++) {
        //         mpz_clear(data[i][j]);
        //         mpz_clear(buffer[i][j]);
        //     }
        //     free(data[i]);
        //     free(buffer[i]);
        // }
        // free(data);
        // free(buffer);
        // for (int i = 0; i < size; i++)
        //     mpz_clear(temp[i]);
        // free(temp);
    }
}
