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
   along with PICCO. If not, see <http://www.gnu.org/licenses/>.i
*/
#ifndef PREOPL_H_
#define PREOPL_H_

#include "Operation.h"
#include "Mult.h"
#include "CarryBit.h"

class PreOpL: public Operation {
public:
	PreOpL(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare* s, mpz_t coefficients[]);
	virtual ~PreOpL();
	void doOperation(mpz_t** C, mpz_t** D1, mpz_t** D2, int K, int size, int threadID);
private:
	Mult *M;
	CarryBit *Carry;
};

#endif /* PREOPL_H_ */
