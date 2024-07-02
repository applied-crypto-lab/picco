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

#include "Open.h"

// Reconstructs shares into result using t+1 shares
// Can be used where the output is stored in the input's location
void Open(mpz_t *shares, mpz_t *result, int size, int threadID, NodeNetwork net, SecretShare *ss) {

    uint threshold = ss->getThreshold();
    mpz_t **buffer = (mpz_t **)malloc(sizeof(mpz_t *) * (threshold + 1));
    for (int i = 0; i < (threshold + 1); i++) {
        buffer[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++)
            mpz_init(buffer[i][j]);
    }

    net.multicastToPeers_Open(ss->getSendToIDs(), ss->getRecvFromIDs(), shares, buffer, size, threadID);

    for (int i = 0; i < size; i++) {
        // putting my share into last position of buff as to match lagrangeWeightsThreshold[i]
        mpz_set(buffer[threshold][i], shares[i]);
    }
    ss->reconstructSecretFromMin(result, buffer, size);

    // free memory
    for (int i = 0; i < (threshold + 1); i++) {
        for (int j = 0; j < size; j++)
            mpz_clear(buffer[i][j]);
        free(buffer[i]);
    }
    free(buffer);
}

void Open_from_all(mpz_t *shares, mpz_t *result, int size, int threadID, NodeNetwork net, SecretShare *ss) {
    uint peers = ss->getPeers();

    mpz_t **buffer = (mpz_t **)malloc(sizeof(mpz_t *) * (peers));
    for (int i = 0; i < (peers); i++) {
        buffer[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++)
            mpz_init(buffer[i][j]);
    }
    net.broadcastToPeers(shares, size, buffer, threadID);
    ss->reconstructSecret(result, buffer, size);
    // freeing
    for (int i = 0; i < (peers); i++) {
        for (int j = 0; j < size; j++)
            mpz_clear(buffer[i][j]);
        free(buffer[i]);
    }
    free(buffer);
}

int Open_int(mpz_t var, int threadID, NodeNetwork net, SecretShare *ss) {

    mpz_t *data = (mpz_t *)malloc(sizeof(mpz_t) * 1);
    mpz_t *results = (mpz_t *)malloc(sizeof(mpz_t) * 1);
    mpz_init(data[0]);
    mpz_init(results[0]);
    mpz_set(data[0], var);

    Open(data, results, 1, threadID, net, ss);

    mpz_t tmp, field;
    mpz_init(tmp);
    mpz_init(field);
    ss->getFieldSize(field);
    mpz_mul_ui(tmp, results[0], 2);
    if (mpz_cmp(tmp, field) > 0)
        mpz_sub(results[0], results[0], field);
    // gmp_printf("%Zd ", results[0]);
    int result = mpz_get_si(results[0]);

    mpz_clear(tmp);
    mpz_clear(field);
    mpz_clear(data[0]);
    mpz_clear(results[0]);
    free(data);
    free(results);
    return result;
}

float Open_float(mpz_t *var, int threadID, NodeNetwork net, SecretShare *ss) {

    mpz_t *data = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    mpz_t *results = (mpz_t *)malloc(sizeof(mpz_t) * 4);

    for (int i = 0; i < 4; i++) {
        mpz_init(data[i]);
        mpz_init(results[i]);
        mpz_set(data[i], var[i]);
    }
    Open(data, results, 4, threadID, net, ss);

    mpz_t tmp, field;
    mpz_init(tmp);
    mpz_init(field);
    ss->getFieldSize(field);
    mpz_mul_ui(tmp, results[1], 2); // if larger than half of the space, convert to negative value
    if (mpz_cmp(tmp, field) > 0)
        mpz_sub(results[1], results[1], field);

    double v = mpz_get_d(results[0]);
    double p = mpz_get_d(results[1]);
    double z = mpz_get_d(results[2]);
    double s = mpz_get_d(results[3]);
    // std::cout << "v : " << v << std::endl;
    // std::cout << "p : " << p << std::endl;
    // std::cout << "z : " << z << std::endl;
    // std::cout << "s : " << s << std::endl;

    double result = 0;

    // free the memory
    mpz_clear(field);
    mpz_clear(tmp);
    for (int i = 0; i < 4; i++) {
        mpz_clear(data[i]);
        mpz_clear(results[i]);
    }
    free(data);
    free(results);

    // return the result
    if (z == 1) {
        return 0;
    } else {
        result = v * pow(2, p);
        if (s == 1) {
            return -result;
        } else {
            return result;
        }
    }

    return result;
}

void Open_print(mpz_t *shares, std::string name, int size, int threadID, NodeNetwork net, SecretShare *ss) {
    mpz_t *res = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++) {
        mpz_init(res[i]);
    }
    Open(shares, res, size, threadID, net, ss);
    for (size_t i = 0; i < size; i++) {
        std::cout << name << "[" << i << "] : ";
        gmp_printf("%Zu\n", res[i]);
    }
    for (int i = 0; i < size; ++i) {
        mpz_clear(res[i]);
    }
    free(res);
}
