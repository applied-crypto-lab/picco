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

B2U::B2U(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare* s, mpz_t coeficients[]) {
	Pw2 = new Pow2(nodeNet, poly, nodeID, s, coeficients);
	Mul = new Mult(nodeNet, nodeID, s);
	Por = new PreOr(nodeNet, poly, nodeID, s, coeficients);
	Rand = new Random(nodeNet, poly, nodeID, s);
    
	net = nodeNet;
	id = nodeID;
	ss = s;
}

B2U::~B2U() {}

void B2U::doOperation(mpz_t* A, int L, mpz_t** result, int size, int threadID){
	int peers = ss->getPeers(); 
	mpz_t* pow2A = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* C = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* temp = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* temp1 = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t** R = (mpz_t**)malloc(sizeof(mpz_t*) * (L+2));
	mpz_t** c = (mpz_t**)malloc(sizeof(mpz_t*) * size);
	mpz_t** resultShares = (mpz_t**)malloc(sizeof(mpz_t*) * peers);
    	mpz_t constL, const1, const2, pow2L; 

	//initialization
	mpz_init_set_ui(const1, 1);
	mpz_init_set_ui(const2, 2); 
	mpz_init_set_ui(constL, L);
	mpz_init(pow2L);  
	for(int i = 0; i < size; i++){
		mpz_init(pow2A[i]);
		mpz_init(C[i]);
		mpz_init(temp[i]);
		mpz_init(temp1[i]); 
	}

	for(int i = 0; i < L+2; i++){
		R[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
		for(int j = 0; j < size; j++)
			mpz_init(R[i][j]);
	}
	for(int i = 0; i < peers; i++){
		resultShares[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
		for(int j = 0; j < size; j++)
			mpz_init(resultShares[i][j]);
	}

	for(int i = 0; i < size; i++){
		c[i] = (mpz_t*)malloc(sizeof(mpz_t) * L);
		for(int j = 0; j < L; j++)
			mpz_init(c[i][j]);
	}

    	ss->modPow(pow2L, const2, constL); 
	
	//start computation.
	Pw2->doOperation(pow2A, A, L, size, threadID);
	Rand->PRandM(L, L, size, R, threadID);
	Rand->PRandInt(L, L, size, C, threadID); 
	ss->modMul(C, C, pow2L, size); 
	ss->modAdd(C, C, pow2A, size);
	ss->modAdd(C, C, R[L], size);
	net.broadcastToPeers(C, size, resultShares, threadID);
	ss->reconstructSecret(C, resultShares, size, true);
	for(int i = 0; i < size; i++)
		binarySplit(C[i], c[i], L);

	for(int i = 0; i < L; i++){
		for(int j = 0; j < size; j++)
			mpz_set(temp[j], c[j][i]); 
		ss->modAdd(temp1, temp, R[i], size);
		Mul->doOperation(temp, temp, R[i], size, threadID);
		ss->modMul(temp, temp, const2, size);
		ss->modSub(temp1, temp1, temp, size);
		ss->copy(temp1, R[i], size); 
	}
	Por->doOperation(R, R, L, size, threadID);
	for(int i = 0; i < L; i++)
		ss->modSub(result[i], const1, R[i], size);
	ss->copy(pow2A, result[L], size);
	
	// free the memory
	mpz_clear(constL); 
	mpz_clear(const1); 
	mpz_clear(const2); 
	mpz_clear(pow2L); 
	for(int i = 0; i < size; i++){
		mpz_clear(pow2A[i]);
		mpz_clear(C[i]);
		mpz_clear(temp[i]);
		mpz_clear(temp1[i]); 
	}
	free(pow2A); 
	free(C); 
	free(temp); 
	free(temp1); 

	for(int i = 0; i < L+2; i++){
		for(int j = 0; j < size; j++)
			mpz_clear(R[i][j]);
		free(R[i]); 
	}
	free(R);
	for(int i = 0; i < peers; i++){
		for(int j = 0; j < size; j++)
			mpz_clear(resultShares[i][j]);
		free(resultShares[i]); 
	}
	free(resultShares); 
	for(int i = 0; i < size; i++){
		for(int j = 0; j < L; j++)
			mpz_clear(c[i][j]);
		free(c[i]); 
	}
	free(c); 
}

