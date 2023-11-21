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
#include "CarryBit.h"

void doOperation_CarryBit(mpz_t *D11, mpz_t *D12, mpz_t *D21, mpz_t *D22, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *temp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init(temp[i]);
    Mult(D21, D21, D11, size, threadID, net, id, ss);
    Mult(temp, D21, D12, size, threadID, net, id, ss);
    ss->modAdd(D22, D22, temp, size);

    for (int i = 0; i < size; i++)
        mpz_clear(temp[i]);
    free(temp);
}
