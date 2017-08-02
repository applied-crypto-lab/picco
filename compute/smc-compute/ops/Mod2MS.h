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
#include "B2U.h"
#include "Inv.h"
#include "Mult.h"
#include "LTZ.h"
#include "Random.h"
#include "Operation.h"

#ifndef MOD2MS_H_
#define MOD2MS_H_

class Mod2MS : public Operation{
public:
	Mod2MS(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare* s, mpz_t coeficients[]);
	virtual ~Mod2MS();
	void doOperation(mpz_t* result, mpz_t* A, mpz_t* M, mpz_t* InvM, int L, int size, int threadID);
private:
	B2U *B2u;
	Inv *Iv;
	Mult *Mul;
	LTZ *Ltz;
	Random *Rand; 
};

#endif /* MOD2M1_H_ */
