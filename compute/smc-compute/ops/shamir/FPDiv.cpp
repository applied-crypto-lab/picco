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
#include "FPDiv.h"

FPDiv::FPDiv(NodeNetwork nodeNet, std::map<std::string, std::vector<int>> poly, int nodeID, SecretShare *s) {

    // Mul = new Mult(nodeNet, nodeID, s);
    App = new AppRcr(nodeNet, poly, nodeID, s);
    // need to use
    T = new TruncPr(nodeNet, poly, nodeID, s);
    net = nodeNet;
    id = nodeID;
    ss = s;
}

FPDiv::~FPDiv() {
    // TODO Auto-generated destructor stub
}

void FPDiv::doOperation(mpz_t *result, mpz_t *a, mpz_t *b, int k, int f, int size, int threadID) {

    // Set theta
    double t = k / 3.5;
    int theta = ceil(log2(t));

    // Set alpha
    mpz_t alpha, const2;
    mpz_init_set_ui(const2, 2);
    mpz_init(alpha);
    mpz_pow_ui(alpha, const2, 2 * f);

    mpz_t *w = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *x = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *y = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *temp = (mpz_t *)malloc(sizeof(mpz_t) * size);

    // do initialization
    for (int i = 0; i < size; ++i) {
        mpz_init(w[i]);
        mpz_init(x[i]);
        mpz_init(y[i]);
        mpz_init(temp[i]);
    }

    App->doOperation(w, b, k, f, size, threadID);
  
    Mult(x, b, w, size, threadID, net, id, ss);
    Mult(y, a, w, size, threadID, net, id, ss);

    for (int i = 0; i < size; i++)
        ss->modSub(x[i], alpha, x[i]);

    T->doOperation(y, y, 2 * k, f, size, threadID);


    for (int i = 0; i < theta - 1; i++) {
        // printf("round %d: \n", i);
        for (int j = 0; j < size; j++)
            ss->modAdd(temp[j], alpha, x[j]);
        Mult(y, y, temp, size, threadID, net, id, ss);
        Mult(x, x, x, size, threadID, net, id, ss);
        T->doOperation(y, y, 2 * k, 2 * f, size, threadID);
        T->doOperation(x, x, 2 * k, 2 * f, size, threadID);

    }

    for (int i = 0; i < size; i++)
        ss->modAdd(x[i], alpha, x[i]);

    Mult(y, y, x, size, threadID, net, id, ss);
    T->doOperation(result, y, 2 * k, 2 * f, size, threadID);
    mpz_clear(const2);
    mpz_clear(alpha);
    for (int i = 0; i < size; ++i) {
        mpz_clear(w[i]);
        mpz_clear(x[i]);
        mpz_clear(y[i]);
        mpz_clear(temp[i]);
    }
    free(w);
    free(x);
    free(y);
    free(temp);
}
