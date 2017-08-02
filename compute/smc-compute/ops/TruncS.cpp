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
#include "TruncS.h"


TruncS::TruncS(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare* s,  mpz_t coeficients[]) {
    
	Mul = new Mult(nodeNet,nodeID, s);
	Lt = new LTZ(nodeNet, poly, nodeID, s, coeficients);
	Bt = new B2U(nodeNet, poly, nodeID, s, coeficients);
	Pre = new PrefixMultiplication(nodeNet, poly, nodeID, s, coeficients);
	In = new Inv(nodeNet, poly, nodeID, s, coeficients);
	Rand = new Random(nodeNet, poly, nodeID, s);
    
	net = nodeNet;
	id = nodeID;
	ss = s;
    
}

TruncS::~TruncS() {
	// TODO Auto-generated destructor stub
}

void TruncS::doOperation(mpz_t* result, mpz_t* A, int K, mpz_t* M, int size, int threadID){
    	int peers = ss->getPeers();
	mpz_t** X = (mpz_t**)malloc(sizeof(mpz_t*) * (K+1));
	mpz_t** R = (mpz_t**)malloc(sizeof(mpz_t*) * (K+2));
	mpz_t** resultShares = (mpz_t**)malloc(sizeof(mpz_t*) * peers);
	
	mpz_t* pow2K = (mpz_t*)malloc(sizeof(mpz_t) * K);
	mpz_t* R1 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* R2 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* T1 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* C = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* CC = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* temp = (mpz_t*)malloc(sizeof(mpz_t) * size);
	
	mpz_t const1, const2, constL, pow2L; 
	mpz_init_set_ui(const1, 1); 
	mpz_init_set_ui(const2, 2); 
	mpz_init_set_ui(constL, K); 
	mpz_init(pow2L); 
	ss->modPow(pow2L, const2, constL); 
	for(int i = 0; i < K; i++){
		mpz_init_set_ui(pow2K[i], 1); 
	}
	for(int i = 0; i < K+1; i++){
		X[i]= (mpz_t*)malloc(sizeof(mpz_t) * size);
		for(int j = 0; j < size; j++)
			mpz_init(X[i][j]);
	}

	for(int i = 0; i < K+2; i++){
		R[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
		for(int j = 0; j < size; j++)
			mpz_init(R[i][j]);
	}

	for(int i = 0; i < peers;  i++){
		resultShares[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
		for(int j = 0; j < size; j++)
			mpz_init(resultShares[i][j]);
	}

	for(int i = 0; i < size; i++){
		mpz_init(R1[i]);
		mpz_init(R2[i]);
		mpz_init(T1[i]);
		mpz_init(C[i]); 
		mpz_init(CC[i]);
		mpz_init(temp[i]);
	}
	//do the computation
 
	Bt->doOperation(M, K, X, size, threadID);
	Rand->PRandM(K, K, size, R, threadID);
    
	for(int i = 0; i < K; i++){
		if(i != 0)
			ss->modMul(pow2K[i], pow2K[i-1], const2);
		ss->modSub(temp, const1, X[i], size);
		Mul->doOperation(temp, temp, R[i], size, threadID);
		ss->modMul(temp, temp, pow2K[i], size);
		ss->modAdd(R1, R1, temp, size);

		Mul->doOperation(temp, X[i], R[i], size, threadID);
		ss->modMul(temp, temp, pow2K[i], size);
		ss->modAdd(R2, R2, temp, size);
	}

	Rand->PRandInt(K, K, size, temp, threadID);
	ss->modMul(temp, temp, pow2L, size); 
	ss->modAdd(R1, R1, temp, size);

	ss->copy(R2, temp, size);
	ss->modAdd(temp, temp, R1, size);
	ss->modAdd(temp, A, temp, size);
	net.broadcastToPeers(temp, size, resultShares, threadID);
	ss->reconstructSecret(T1, resultShares, size, true);

	for(int i = 0; i < K-1; i++){
		ss->modSub(temp, X[i], X[i+1], size);
		for(int j = 0; j < size; j++)
			mpz_mod(C[j], T1[j], pow2K[i+1]); 
		ss->modMul(temp, temp, C, size);
		ss->modAdd(CC, CC, temp, size);
	}
	//line 9
	ss->modSub(temp, CC, R2, size);
	Lt->doOperation(T1, temp, K, size, threadID);
	//line 10
	ss->modSub(result, CC, R2, size);
	Mul->doOperation(temp, T1, X[K], size, threadID);
	ss->modAdd(result, temp, result, size);
	In->doOperation(X[K], temp, size, threadID);
	ss->modSub(result, A, result, size);
	Mul->doOperation(result, result, temp, size, threadID);
	
	//free the memory 
	mpz_clear(const1); 
	mpz_clear(const2); 
	mpz_clear(constL);
	mpz_clear(pow2L);  
	
	for(int i = 0; i < K; i++)
		mpz_clear(pow2K[i]); 
	free(pow2K); 

	for(int i = 0; i < K+1; i++){
		for(int j = 0; j < size; j++)
			mpz_clear(X[i][j]);
		free(X[i]); 
	}
	free(X); 

	for(int i = 0; i < K+2; i++){
		for(int j = 0; j < size; j++)
			mpz_clear(R[i][j]);
		free(R[i]); 
	}
	free(R); 

	for(int i = 0; i < peers;  i++){
		for(int j = 0; j < size; j++)
			mpz_clear(resultShares[i][j]);
		free(resultShares[i]); 
	}
	free(resultShares); 

	for(int i = 0; i < size; i++){
		mpz_clear(R1[i]);
		mpz_clear(R2[i]);
		mpz_clear(T1[i]);
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
}
