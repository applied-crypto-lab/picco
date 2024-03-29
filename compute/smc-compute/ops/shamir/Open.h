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

#ifndef OPEN_SHAMIR_H_
#define OPEN_SHAMIR_H_

#include "Operation.h"

void Open(mpz_t *shares, mpz_t *result, int size, int threadID, NodeNetwork nodeNet, SecretShare *ss);
int Open_int(mpz_t var, int threadID, NodeNetwork nodeNet, SecretShare *ss);
float Open_float(mpz_t *var, int threadID, NodeNetwork nodeNet, SecretShare *ss);
void Open_from_all(mpz_t *shares, mpz_t *result, int size, int threadID, NodeNetwork nodeNet, SecretShare *ss);

void Open_print(mpz_t *shares, std::string name, int size, int threadID, NodeNetwork nodeNet, SecretShare *ss);

#endif
