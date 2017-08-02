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

#ifndef CARRYBIT_H_
#define CARRYBIT_H_

#include "Mult.h"
#include "Operation.h"

class CarryBit : public Operation{
public:
	CarryBit(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, int field, mpz_t coeficients[]);
	virtual ~CarryBit();
	void doOperation(mpz_t* D1, mpz_t* D2, mpz_t* D3, mpz_t* D4, int size, int threadID);
private:
	Mult *M;
};

#endif /* CARRYBIT_H_ */
