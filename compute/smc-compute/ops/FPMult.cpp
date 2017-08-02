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
#include "FPMult.h"

FPMult::FPMult(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare* s, mpz_t coeficients[]) {
	T = new Trunc(nodeNet,poly,nodeID,s,coeficients);
	net = nodeNet;
	id = nodeID;
	ss = s;
}

FPMult::~FPMult() {
	// TODO Auto-generated destructor stub
}

void FPMult::doOperation(mpz_t* C, mpz_t* A, mpz_t* B, int K, int F, int size){
	mpz_t* temp = (mpz_t*)malloc(sizeof(mpz_t) * size);
	for(int i = 0; i < size; i++)
		mpz_init(temp[i]);
	ss->modMul(temp, A, B, size);
	T->doOperation(C, temp, 2*K, F, size);
	for(int i = 0; i < size; i++)
		mpz_clear(temp[i]); 
	free(temp); 
}
