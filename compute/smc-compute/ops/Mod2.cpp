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

#include "Mod2.h"

Mod2::Mod2(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare *s) {
    
	Rand = new Random(nodeNet, poly, nodeID, s);
	net = nodeNet;
	id = nodeID;
	polynomials = poly;
	ss = s;

}

Mod2::~Mod2() {
	// TODO Auto-generated destructor stub
}

void Mod2::doOperation(mpz_t* A, mpz_t* result, int K, int size, int threadID){
	int peers = ss->getPeers();
	mpz_t** R = (mpz_t**)malloc(sizeof(mpz_t*) * 2);
	mpz_t** shares = (mpz_t**)malloc(sizeof(mpz_t*) * peers);
	mpz_t* C = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* S = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* Bit = (mpz_t*)malloc(sizeof(mpz_t));
	mpz_t const2, constK1, const2K1; 	
	/*initialization*/
	for(int i = 0; i < peers; i++){
		shares[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
		for(int j = 0; j < size; j++)
			mpz_init(shares[i][j]);
	}

	for(int i = 0; i < 2; i++){
                R[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
                for(int j = 0; j < size; j++)
                        mpz_init(R[i][j]);
        }

	for(int i = 0; i < size; i++){
		mpz_init(C[i]);
		mpz_init(S[i]); 
	}

	mpz_init_set_ui(const2, 2); 
    	mpz_init_set_ui(constK1, K-1);
 	mpz_init(const2K1); 	
	ss->modPow(const2K1, const2, constK1); 
	
	mpz_init(Bit[0]);
	Rand->PRandM(K, 1, size, R, threadID);
	Rand->PRandInt(K, 1, size, S, threadID);
	ss->modAdd(C, A, R[0], size);
	ss->modMul(S, S, const2, size); 
	if(K > 1)
		ss->modAdd(C, C, const2K1, size); 
	ss->modAdd(C, C, S, size); 
	net.broadcastToPeers(C, size, shares, threadID);
	ss->reconstructSecret(C, shares, size, true);
	for(int i = 0; i < size; i++){
		binarySplit(C[i], Bit, 1);
		mpz_set(C[i], Bit[0]);
	}
	ss->modMul(result, C, R[0], size);
	ss->modMul(result, result, const2, size);
	ss->modSub(result, R[0], result, size);
	ss->modAdd(result, result, C, size);
	
	// free the memory
	mpz_clear(constK1); 
	mpz_clear(const2); 
	mpz_clear(const2K1); 
	mpz_clear(Bit[0]); 

	for(int i = 0; i < peers; i++){
		for(int j = 0; j < size; j++)
			mpz_clear(shares[i][j]);
		free(shares[i]); 
	}
	for(int i = 0; i < 2; i++){
                for(int j = 0; j < size; j++)
                        mpz_clear(R[i][j]);
                free(R[i]);
        }
	free(R); 
	free(shares); 

	for(int i = 0; i < size; i++){
		mpz_clear(C[i]);
		mpz_clear(S[i]); 
	}

	free(S); 
	free(C); 
	free(Bit);  
}
