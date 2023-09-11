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

#ifndef ADDBITWISE_SHAMIR_H_
#define ADDBITWISE_SHAMIR_H_

#include "Operation.h"
#include "PreOpL.h"

// class AddBitwise : public Operation {
// public:
//     AddBitwise(NodeNetwork nodeNet, std::map<std::string, std::vector<int>> poly, int nodeID, SecretShare *s);
//     virtual ~AddBitwise();
//     void doOperation(mpz_t **S, mpz_t **A, mpz_t **B, int K, int size, int threadID);

// private:
//     // PreOpL *Pre;
// };

void AddBitwise(mpz_t **S, mpz_t **A, mpz_t **B, int K, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) ;

#endif /* ADDBITWISE_SHAMIR_H_ */