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
#ifndef INTAPPRCR_H_
#define INTAPPRCR_H_

#include "Norm.h"
#include "Mult.h"
#include "TruncPr.h"
#include "Operation.h"

class IntAppRcr:public Operation{
public:
	IntAppRcr(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare *s, mpz_t coeficients[]);
	virtual ~IntAppRcr();
	void doOperation(mpz_t* w, mpz_t* b, int k, int size, int threadID);
private:
	Mult *Mul;
	Norm *No;
	TruncPr *T;
};

#endif /* INTAPPRCR_H_ */
