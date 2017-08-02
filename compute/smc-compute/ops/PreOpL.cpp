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
#include "PreOpL.h"


PreOpL::PreOpL(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare* s, mpz_t coefficients[]) {
	M = new Mult(nodeNet, nodeID, s);
	net = nodeNet;
	id = nodeID;
	ss = s;

}

PreOpL::~PreOpL() {
	// TODO Auto-generated destructor stub
}

//D1 contains the propagation bit and D2 contains the generation bit
void PreOpL::doOperation(mpz_t** C, mpz_t** D1, mpz_t** D2, int K, int size, int threadID){
	mpz_t* temp1 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* temp2 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	//initialization
	for(int i = 0; i < size; i++){
		mpz_init(temp1[i]);
		mpz_init(temp2[i]);
	}
	//start computation
	ss->copy(D2[0], C[0], size);
	for(int i = 1; i < K; i++){
		M->doOperation(temp1, D1[i], C[i-1], size, threadID);
		M->doOperation(temp2, temp1, D2[i], size, threadID);
		ss->modAdd(C[i], temp1, D2[i], size);
		ss->modSub(C[i], C[i], temp2, size);
	}
	//free memory
	for(int i = 0; i < size; i++){
		mpz_clear(temp1[i]);
		mpz_clear(temp2[i]);
	}
	free(temp1); 
	free(temp2); 
}
