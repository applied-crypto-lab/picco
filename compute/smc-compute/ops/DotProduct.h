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
#ifndef DOTPRODUCT_H_
#define DOTPRODUCT_H_

#include "Mult.h"
#include "Operation.h"

class DotProduct : public Operation{
public:
	DotProduct(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare *s, mpz_t coeficients[]);
	virtual ~DotProduct();
	void doOperation(mpz_t* a, mpz_t* b, mpz_t result, int size, int threadID);
	void doOperation(mpz_t** a, mpz_t** b, mpz_t* result, int num_vec, int size_vec, int threadID); 
private:
	Mult *Mul;
};

#endif /*DOTPRODUCT_H*/

