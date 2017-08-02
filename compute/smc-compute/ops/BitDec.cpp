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

#include "BitDec.h"


BitDec::BitDec(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare *s, mpz_t coeficients[]) {
    
	Add = new AddBitwise(nodeNet, poly, nodeID, s, coeficients);
	Rand = new Random(nodeNet, poly, nodeID, s);
    
	net = nodeNet;
	id = nodeID;
	ss = s;
}

BitDec::~BitDec() {
	// TODO Auto-generated destructor stub
}

void BitDec::doOperation(mpz_t** S, mpz_t* A, int K, int M, int size, int threadID){
	int peers = ss->getPeers(); 
	mpz_t** R = (mpz_t**)malloc(sizeof(mpz_t*) * (M+2));
	mpz_t* R1 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t** resultShares = (mpz_t**)malloc(sizeof(mpz_t*) * peers);
	mpz_t** B = (mpz_t**)malloc(sizeof(mpz_t*) * size);
	mpz_t** BB = (mpz_t**)malloc(sizeof(mpz_t*) * M);
	mpz_t* temp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	
	mpz_t constK, const2, constM, constS;
	mpz_t pow2K, pow2M, pow2S; 	
	//initialization
	mpz_init_set_ui(constK, K);
	mpz_init_set_ui(const2, 2);
	mpz_init_set_ui(constM, M); 
	mpz_init_set_ui(constS, K+48); //changed from 55 to 48 
	mpz_init(pow2K); 
	mpz_init(pow2M); 
	mpz_init(pow2S); 

	for(int i = 0; i < M+2; i++){
		R[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
		for(int j = 0; j < size; j++)
			mpz_init(R[i][j]);
	}

	for(int i = 0; i < M; i++){
		BB[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
		for(int j = 0; j < size; j++)
			mpz_init(BB[i][j]);
	}

	for(int i = 0; i < size; i++){
	        B[i] = (mpz_t*)malloc(sizeof(mpz_t) * M);
		for(int j = 0; j < M; j++)
                     mpz_init(B[i][j]);

	}
	for(int i = 0; i < peers; i++){
		resultShares[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
		for(int j = 0; j < size; j++)
			mpz_init(resultShares[i][j]);
	}

	for(int i = 0; i < size; i++){
		mpz_init(R1[i]);
		mpz_init(temp[i]); 
	}
	ss->modPow(pow2K, const2, constK); 
	ss->modPow(pow2M, const2, constM); 
	ss->modPow(pow2S, const2, constS); 
	
	//start computation
	Rand->PRandInt(K, M, size, R1, threadID); 
	ss->modMul(R1, R1, pow2M, size);
	Rand->PRandM(K, M, size, R, threadID);
	ss->modAdd(temp, A, pow2K, size);
	ss->modAdd(temp, temp, pow2S, size); 
	ss->modSub(temp, temp, R[M], size);
	ss->modSub(temp, temp, R1, size); 
	net.broadcastToPeers(temp, size, resultShares, threadID);
	ss->reconstructSecret(temp, resultShares, size, true);
	
	for(int i = 0; i < size; i++)
		binarySplit(temp[i], B[i], M);

	for(int i = 0; i < M; i++)
		for(int j = 0; j < size; j++)
			mpz_set(BB[i][j], B[j][i]);

	/************ free memory of B *************/
	for(int i = 0; i < size; i++){
                for(int j = 0; j < M; j++)
                        mpz_clear(B[i][j]);
                free(B[i]);
        }
        free(B);
	/*******************************************/
	
	Add->doOperation(S, BB, R, M, size, threadID);
	
	//free the memory 
	for(int i = 0; i < M+2; i++){
		for(int j = 0; j < size; j++)
			mpz_clear(R[i][j]);
		free(R[i]); 
	}
	free(R); 

	for(int i = 0; i < M; i++){
		for(int j = 0; j < size; j++)
			mpz_clear(BB[i][j]);
		free(BB[i]); 
	}
	free(BB); 
	
	for(int i = 0; i < peers; i++){
		for(int j = 0; j < size; j++)
			mpz_clear(resultShares[i][j]);
		free(resultShares[i]); 
	}
	free(resultShares); 

	for(int i = 0; i < size; i++){
		mpz_clear(temp[i]);
		mpz_clear(R1[i]);  
	}
	free(R1); 
	free(temp); 

	mpz_clear(constK); 
	mpz_clear(const2);  
	mpz_clear(constM); 
	mpz_clear(constS); 
	mpz_clear(pow2K); 
	mpz_clear(pow2M);
	mpz_clear(pow2S); 
}

