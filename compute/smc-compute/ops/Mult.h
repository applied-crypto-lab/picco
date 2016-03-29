/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame

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
#ifndef MULT_H_
#define MULT_H_

#include "Operation.h"
class Mult : public Operation{
public:
	Mult(NodeNetwork nodeNet, int nodeID, SecretShare *s); 
	virtual ~Mult();
	void doOperation(mpz_t* C, mpz_t* A, mpz_t* B, int size, int threadID); 
};

#endif /* MULT_H_ */
