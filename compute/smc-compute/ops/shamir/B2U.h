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

#include "Mult.h"
#include "Open.h"
#include "Operation.h"
#include "Pow2.h"
#include "PreOr.h"
#include "Random.h"

#ifndef B2U_SHAMIR_H_
#define B2U_SHAMIR_H_

void doOperation_B2U(mpz_t *A, int L, mpz_t **result, int size, int threadID, NodeNetwork net,  SecretShare *ss);

#endif /* B2U_SHAMIR_H_ */
