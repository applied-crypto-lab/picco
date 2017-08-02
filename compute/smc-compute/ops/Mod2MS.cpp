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
#include "Mod2MS.h"

Mod2MS::Mod2MS(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare* s, mpz_t coeficients[]) {
	B2u = new B2U(nodeNet,poly,nodeID, s, coeficients);
	Iv = new Inv(nodeNet, poly, nodeID, s, coeficients);
	Mul = new Mult(nodeNet, nodeID, s);
	Ltz = new LTZ(nodeNet, poly, nodeID, s, coeficients);
	Rand = new Random(nodeNet, poly, nodeID, s);
    
	net = nodeNet;
	id = nodeID;
	ss = s;
}

Mod2MS::~Mod2MS() {}

void Mod2MS::doOperation(mpz_t* result, mpz_t* A, mpz_t* M, mpz_t* powM, int L, int size, int threadID){
	int peers = ss->getPeers(); 
	mpz_t** X = (mpz_t**)malloc(sizeof(mpz_t*) * (L+1));
	mpz_t** R = (mpz_t**)malloc(sizeof(mpz_t*) * (L+2));
	mpz_t** resultShares = (mpz_t**)malloc(sizeof(mpz_t*) * peers);
	
	mpz_t* pow2K = (mpz_t*)malloc(sizeof(mpz_t) * L); 
	mpz_t* R1 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* R2 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* T1 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* S  = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* C = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* CC = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* temp = (mpz_t*)malloc(sizeof(mpz_t) * size);
	
	//initialization
	for(int i = 0; i < L; i++)
		mpz_init_set_ui(pow2K[i], 1); 
	for(int i = 0; i < L+1; i++){
		X[i]= (mpz_t*)malloc(sizeof(mpz_t) * size);
		for(int j = 0; j < size; j++)
			mpz_init(X[i][j]);
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
		mpz_init(R1[i]);
		mpz_init(R2[i]);
		mpz_init(S[i]); 
		mpz_init(T1[i]);
		mpz_init(C[i]); 
		mpz_init(CC[i]);
		mpz_init(temp[i]);
	}
   	
	//do the computation
	mpz_t pow2L, const2, constL; 
        mpz_init(pow2L); 
	mpz_init_set_ui(const2, 2); 
	mpz_init_set_ui(constL, L); 
	ss->modPow(pow2L, const2, constL); 
	B2u->doOperation(M, L, X, size, threadID);
	Rand->PRandM(L, L, size, R, threadID);

	for(int i = 0; i < L; i++){
		if(i != 0)
			ss->modMul(pow2K[i], pow2K[i-1], 2);
		ss->modSub(temp, 1, X[i], size);
		Mul->doOperation(temp, temp, R[i], size, threadID);
		ss->modMul(temp, temp, pow2K[i], size);
		ss->modAdd(R1, R1, temp, size);
		
		Mul->doOperation(temp, X[i], R[i], size, threadID);
		ss->modMul(temp, temp, pow2K[i], size);
		ss->modAdd(R2, R2, temp, size);
	}
   
  	Rand->PRandInt(L, L, size, S, threadID);
        ss->modMul(S, S, pow2L, size);
        ss->modAdd(R1, R1, S, size);
	 
	ss->copy(R2, temp, size);
	ss->modAdd(temp, temp, R1, size);
	ss->modAdd(temp, A, temp, size);
	net.broadcastToPeers(temp, size, resultShares, threadID);
	ss->reconstructSecret(T1, resultShares, size, true);
	for(int i = 0; i < L-1; i++){
		ss->modSub(temp, X[i], X[i+1], size);
		for(int j = 0; j < size; j++)
			mpz_mod(C[j], T1[j], pow2K[i+1]); 
		ss->modMul(temp, temp, C, size);
		ss->modAdd(CC, CC, temp, size);
	}
    	//line 9
	ss->modSub(temp, CC, R2, size);
	Ltz->doOperation(T1, temp, L, size, threadID);
	//line 10
	ss->modSub(result, CC, R2, size);
	Mul->doOperation(temp, T1, X[L], size, threadID);
	ss->modAdd(result, temp, result, size);
	ss->copy(X[L], powM, size);
	
	//free the memory
	for(int i = 0; i < L; i++)
		mpz_clear(pow2K[i]); 
	free(pow2K); 
	for(int i = 0; i < L+1; i++){
		for(int j = 0; j < size; j++)
			mpz_clear(X[i][j]);
		free(X[i]); 
	}
	free(X); 

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
		mpz_clear(R1[i]);
		mpz_clear(R2[i]);
		mpz_clear(T1[i]);
		mpz_clear(S[i]); 
		mpz_clear(C[i]); 
		mpz_clear(CC[i]);
		mpz_clear(temp[i]);
	}
	free(R1); 
	free(R2); 
	free(T1);
	free(C);  
	free(CC); 
	free(temp); 
	free(S); 

	mpz_clear(pow2L); 
	mpz_clear(const2); 
	mpz_clear(constL);  
}
