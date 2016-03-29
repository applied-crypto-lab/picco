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
#ifndef INTDIV_H_
#define INTDIV_H_

#include "Operation.h"
#include "Mult.h"
#include "EQZ.h"
#include "LTZ.h"
#include "IntAppRcr.h"
#include "TruncPr.h"

class IntDiv: public Operation {
public:
	IntDiv(NodeNetwork nodeNet, std::map<long, std::vector<int> > poly, int nodeID, SecretShare* s, mpz_t coefficients[]);
	virtual ~IntDiv();
	void doOperation(mpz_t* result, mpz_t* a, mpz_t* b, int k, int size, int threadID);
	void doOperationPub(mpz_t* result, mpz_t* a, mpz_t* b, int k, int size, int threadID); 
private:
	Mult *Mul;
	IntAppRcr *App;
	TruncPr *T;
	EQZ *Eq; 
	LTZ *Lt; 
};

#endif /* INTDIV_H_ */
