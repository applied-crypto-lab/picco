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
#ifndef FL2INT_H_
#define INT2FL_H_

#include "Operation.h"
#include "Mult.h"
#include "LTZ.h"
#include "FLRound.h"
#include "Mod2MS.h"
#include "Pow2.h"
#include "Inv.h"


class FL2Int: public Operation {
public:
	FL2Int();
	FL2Int(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare *s, mpz_t coeficients[]);
	virtual ~FL2Int();
	void doOperation(mpz_t** values, mpz_t* results, int K, int L, int gamma, int size, int threadID);
    
private:
	Mult *Mul;
	LTZ *Lt;
	FLRound *Flround; 
	Mod2MS *Mod2ms; 
	Pow2 *P; 
	Inv *I; 
};

#endif /* INT2FL_H */
