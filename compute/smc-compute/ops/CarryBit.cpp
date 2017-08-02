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
#include "CarryBit.h"

CarryBit::CarryBit(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare *s, mpz_t coeficients[]) {
    
	M = new Mult(nodeNet, nodeID, s);
    	ss = s;
	net = nodeNet;
	id = nodeID;
}

CarryBit::~CarryBit() {
	// TODO Auto-generated destructor stub
}

void CarryBit::doOperation(mpz_t* D11, mpz_t* D12, mpz_t* D21, mpz_t* D22, int size, int threadID)
{
	mpz_t* temp = (mpz_t*)malloc(sizeof(mpz_t) * size);
	for(int i = 0; i < size; i++)
		mpz_init(temp[i]);
	M->doOperation(D21, D21, D11, size, threadID);
	M->doOperation(temp, D21, D12, size, threadID);
	ss->modAdd(D22, D22, temp, size);
	
	for(int i = 0; i < size; i++)
		mpz_clear(temp[i]);
	free(temp); 
}
