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
#ifndef BITDEC_SHAMIR_H_
#define BITDEC_SHAMIR_H_

#include "AddBitwise.h"
#include "Open.h"
#include "Operation.h"
#include "Random.h"

void doOperation_bitDec(mpz_t **S, mpz_t *A, int K, int M, int size, int threadID, NodeNetwork net, int id, SecretShare *ss);

#endif /* BITDEC_SHAMIR_H_ */
