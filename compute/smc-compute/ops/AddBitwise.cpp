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

#include "AddBitwise.h"

AddBitwise::AddBitwise(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare* s, mpz_t coeficients[]) {
	Pre = new PreOpL(nodeNet, poly, nodeID, s, coeficients);
	ss = s;
}

AddBitwise::~AddBitwise() {
	// TODO Auto-generated destructor stub
}

// Notice that S has a format [K+1][size] while A and B has formats [K][size].
void AddBitwise::doOperation(mpz_t** S, mpz_t** A, mpz_t** B, int K, int size, int threadID){
	mpz_t* temp1 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* temp2 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t** C = (mpz_t**)malloc(sizeof(mpz_t*) * K);
	mpz_t** D1 = (mpz_t**)malloc(sizeof(mpz_t*) * K);
	mpz_t** D2 = (mpz_t**)malloc(sizeof(mpz_t*) * K);
	mpz_t const2;
	
	//initialization
	mpz_init_set_ui(const2, 2); 
	for(int i = 0; i < K; i++){
		D1[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
		D2[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
		C[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);

		for(int j = 0; j < size; j++){
			mpz_init(D1[i][j]);
			mpz_init(D2[i][j]);
			mpz_init(C[i][j]);
		}
	}

	for(int i = 0; i < size; i++){
		mpz_init(temp1[i]);
		mpz_init(temp2[i]);
	}

	//start computation
	for(int i = 0; i < K; i++){
		ss->modAdd(temp1, A[i], B[i], size);
		ss->modMul(D2[i], A[i], B[i], size);
		ss->modMul(temp2, D2[i], const2, size);
		ss->modSub(D1[i], temp1, temp2, size);
	}

	Pre->doOperation(C, D1, D2, K, size, threadID);
	ss->modAdd(temp1, A[0], B[0], size);
	ss->modMul(temp2, C[0], const2, size);
	ss->modSub(S[0], temp1, temp2, size);
	for(int i = 1; i <  K; i++){
		ss->modAdd(temp1, A[i], B[i], size);
		ss->modAdd(temp1, temp1, C[i-1], size);
		ss->modMul(temp2, C[i], const2, size);
		ss->modSub(S[i], temp1, temp2, size);
	}
	
	ss->copy(C[K-1], S[K], size);

	//free the memory	
	for(int i = 0; i < K; i++){
		for(int j = 0; j < size; j++){
			mpz_clear(D1[i][j]);
			mpz_clear(D2[i][j]);
			mpz_clear(C[i][j]);
		}
		free(D1[i]); 
		free(D2[i]); 
		free(C[i]); 
	}
	free(D1); 
	free(D2); 
	free(C); 

	for(int i = 0; i < size; i++){
		mpz_clear(temp1[i]);
		mpz_clear(temp2[i]);
	}
	free(temp1); 
	free(temp2); 
	mpz_clear(const2); 
}
