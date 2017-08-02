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
#include "PrefixMultiplication.h"

PrefixMultiplication::PrefixMultiplication(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare *s, mpz_t coeficients[]){
    
    	net = nodeNet;
	polynomials = poly;
	id = nodeID;
	ss = s;
	for (int i = 0; i < 9; i++){ //Not optimal, pass this thing by pointer somehow
		mpz_init(coef[i]);
		mpz_set(coef[i],coeficients[i]);
	}


	Rand = new Random(nodeNet, poly, nodeID, s);
}

PrefixMultiplication::~PrefixMultiplication() {}

void PrefixMultiplication::doOperation(mpz_t** B, mpz_t** result, int length, int size, int threadID){
	int peers = ss->getPeers();
	mpz_t* R = (mpz_t*)malloc(sizeof(mpz_t) * length);
	mpz_t* S = (mpz_t*)malloc(sizeof(mpz_t) * length);
	mpz_t* V = (mpz_t*)malloc(sizeof(mpz_t) * length);
	mpz_t* W = (mpz_t*)malloc(sizeof(mpz_t) * length);
	mpz_t* U = (mpz_t*)malloc(sizeof(mpz_t) * length);
	mpz_t* temp = (mpz_t*)malloc(sizeof(mpz_t) * length);
	mpz_t* temp1 = (mpz_t*)malloc(sizeof(mpz_t) * size);

	mpz_t** buffer1 = (mpz_t**)malloc(sizeof(mpz_t*) * peers);
	mpz_t** buffer2 = (mpz_t**)malloc(sizeof(mpz_t*) * peers);

	for(int i = 0; i < length; i++){
		mpz_init(R[i]);
		mpz_init(S[i]);
		mpz_init(V[i]);
		mpz_init(W[i]);
		mpz_init(U[i]);
		mpz_init(temp[i]);
	}

	for(int i = 0; i < size; i++)
		mpz_init(temp1[i]);

	for(int i = 0; i < peers; i++){
		buffer1[i] = (mpz_t*)malloc(sizeof(mpz_t) * length);
		buffer2[i] = (mpz_t*)malloc(sizeof(mpz_t) * length);
		for(int j = 0; j < length; j++){
			mpz_init(buffer1[i][j]);
			mpz_init(buffer2[i][j]);
		}
	}

	//Rand->generateRandValue(id, ss->getBits(), length, R, threadID);
	//Rand->generateRandValue(id, ss->getBits(), length, S, threadID);
	mpz_t field; //
	mpz_init(field); 
	ss->getFieldSize(field);
	Rand->generateRandValue(id, field, length, R, threadID); //
	Rand->generateRandValue(id, field, length, S, threadID); //


	ss->modMul(temp, R, S, length);
	net.broadcastToPeers(temp, length, buffer1, threadID);
	ss->reconstructSecret(U, buffer1, length, true);
	clearBuffer(buffer1, peers, length);
	for(int i = 0; i < length-1; i++)
		ss->modMul(V[i], R[i+1], S[i]);

	ss->getShares(buffer1, V, length);
	net.multicastToPeers(buffer1, buffer2, length, threadID);
	ss->reconstructSecret(V, buffer2, length, true);
	/************ free memory ********************/
	for(int i = 0; i < peers; i++){
                for(int j = 0; j < length; j++){
                        mpz_clear(buffer1[i][j]);
                        mpz_clear(buffer2[i][j]);
                }
                free(buffer1[i]);
                free(buffer2[i]);
        }
        free(buffer1);
        free(buffer2);
	/*********************************************/
        mpz_t** buffer3 = (mpz_t**)malloc(sizeof(mpz_t*) * peers);
	for(int i = 0; i < peers; i++){
                buffer3[i] = (mpz_t*)malloc(sizeof(mpz_t) * length * size);
                for(int j = 0; j < length * size; j++)
                        mpz_init(buffer3[i][j]);
        }
        mpz_t* results = (mpz_t*)malloc(sizeof(mpz_t) * length * size);
	for(int i = 0; i < length * size; i++)
              mpz_init(results[i]);

		
	mpz_set(W[0], R[0]);
	for(int i = 1; i < length; i++){
		mpz_t temp;
		mpz_init(temp);
		ss->modInv(temp, U[i-1]);
		ss->modMul(W[i], V[i-1], temp);
	}
	
	ss->modInv(temp, U, length);
	ss->modMul(S, S, temp, length);
	
	for(int i = 0; i < length; i++)
		for(int m = 0; m < size; m++)
			ss->modMul(results[i*size+m],B[i][m],W[i]);
    
	net.broadcastToPeers(results, size*length, buffer3, threadID);
	ss->reconstructSecret(results, buffer3, size * length, true);

	for(int i = 0; i < size; i++)
		mpz_set(temp1[i], results[i]); 
	ss->copy(B[0], result[0], size);
	for(int i = 1; i < length; i++){
		for(int m = 0; m < size; m++){
			ss->modMul(temp1[m], temp1[m], results[i*size+m]);
			ss->modMul(result[i][m], S[i], temp1[m]);
		}
	}

	for(int i = 0; i < length; i++){
		mpz_clear(R[i]);
		mpz_clear(S[i]);
		mpz_clear(V[i]);
		mpz_clear(W[i]);
		mpz_clear(U[i]);
		mpz_clear(temp[i]);
	}
	free(R); 
	free(S); 
	free(V); 
	free(W); 
	free(U); 
	free(temp); 

	for(int i = 0; i < length * size; i++)
		mpz_clear(results[i]);
	free(results); 
	for(int i = 0; i < size; i++)
		mpz_clear(temp1[i]);
	free(temp1); 
	
	for(int i = 0; i < peers; i++){
		for(int j = 0; j < length * size; j++)
			mpz_clear(buffer3[i][j]);
		free(buffer3[i]); 
	}
	free(buffer3); 
}


