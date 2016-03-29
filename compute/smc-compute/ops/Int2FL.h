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
#ifndef INT2FL_H_
#define INT2FL_H_

#include "Operation.h"
#include "Mult.h"
#include "LTZ.h"
#include "EQZ.h"
#include "PreOr.h"
#include "BitDec.h"
#include "Trunc.h"

class Int2FL: public Operation {
public:
	Int2FL();
	Int2FL(NodeNetwork nodeNet, std::map<long, std::vector<int> > poly, int nodeID, SecretShare *s, mpz_t coeficients[]);
	virtual ~Int2FL();
	void doOperation(mpz_t* values, mpz_t** results, int gamma, int K, int size, int threadID);
    
private:
	Mult *Mul;
	LTZ *Lt;
	PreOr *Pre;
	BitDec *Bd;
	Trunc * T; 
	EQZ *Eq; 
};

#endif /* INT2FL_H */
