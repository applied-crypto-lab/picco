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

#include "Operation.h"
#include "Random.h"
#include "Pow2.h"
#include "Mult.h"
#include "PreOr.h"

#ifndef B2U_H_
#define B2U_H_

class B2U : public Operation{
public:
	B2U(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare* s, mpz_t coeficients[]);
	virtual ~B2U();
	void doOperation(mpz_t* A, int L, mpz_t** result, int size, int threadID);
private:
	Random *Rand; 
	Pow2 *Pw2;
	Mult *Mul;
	PreOr *Por;
};

#endif /* B2U_H_ */
