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
#include "TruncPr.h"


TruncPr::~TruncPr() {}

TruncPr::TruncPr(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare* s, mpz_t coeficients[]){
    
	net = nodeNet;
	id = nodeID;
	ss = s;
	Rand = new Random(nodeNet, poly, nodeID, s);
	
}

void TruncPr::doOperation(mpz_t* result, mpz_t* shares, int K, int M, int size, int threadID){
		int peers = ss->getPeers();
		mpz_t** R = (mpz_t**)malloc(sizeof(mpz_t*) * (M+2));
		mpz_t** resultShares = (mpz_t**)malloc(sizeof(mpz_t*) * (peers));
		mpz_t* C = (mpz_t*)malloc(sizeof(mpz_t) * size);
		mpz_t* temp = (mpz_t*)malloc(sizeof(mpz_t) * size);
		
		mpz_t const2, constM, constK1, pow2M, inv2M, pow2K1;
		mpz_init_set_ui(const2, 2);
		mpz_init_set_ui(constM, M);
		mpz_init_set_ui(constK1, K-1);
		mpz_init(pow2M); 
		mpz_init(inv2M); 
		mpz_init(pow2K1);  
		ss->modPow(pow2M, const2, constM);
		ss->modPow(pow2K1, const2, constK1);
		ss->modInv(inv2M, pow2M); 
		
		//initialization
		for(int i = 0; i < M+2; i++){
				R[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
				for(int j = 0; j < size; j++)
					mpz_init(R[i][j]);
		}
			
		for(int i = 0; i <peers; i++){
				resultShares[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
				for(int j = 0; j < size; j++)
					mpz_init(resultShares[i][j]);
		}
    	
 
		for(int i = 0; i < size; i++){
			mpz_init(C[i]);
			mpz_init(temp[i]);
		}
    		
		// start comutation.
		/**************/
		Rand->PRandInt(K, M, size, temp, threadID); 
		Rand->PRandM(K, M, size, R, threadID); 
		
		ss->modMul(temp, temp, pow2M, size); 
		ss->modAdd(C, C, temp, size); 
		ss->modAdd(C, C, shares, size); 
		ss->modAdd(C, C, R[M], size);
		ss->modAdd(C, C, pow2K1, size);
		
		net.broadcastToPeers(C, size, resultShares, threadID);
		ss->reconstructSecret(C, resultShares, size, true);

		for(int i = 0; i < size; i++){
			mpz_mod(C[i], C[i], pow2M);
			mpz_set_ui(temp[i], 0); 
		}
		ss->modAdd(temp, temp, shares, size);
		ss->modSub(temp, temp, C, size);
		ss->modAdd(temp, temp, R[M], size);
		ss->modMul(temp, temp, inv2M, size);
		
		for(int i = 0; i < size; i++)
			mpz_set(result[i], temp[i]); 
		
		//free the memory
		mpz_clear(const2);
		mpz_clear(constM);
		mpz_clear(constK1); 
		mpz_clear(pow2M);
		mpz_clear(pow2K1);
		mpz_clear(inv2M); 
	
		for(int i = 0; i < M+2; i++){
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
			mpz_clear(C[i]);
			mpz_clear(temp[i]);
		}
		free(C); 
		free(temp); 
}

