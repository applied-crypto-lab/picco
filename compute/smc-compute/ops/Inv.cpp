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

#include "Inv.h"


Inv::Inv(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare* s, mpz_t coeficients[]){
    
	Rand = new Random(nodeNet, poly, nodeID, s);
	net = nodeNet;
	id = nodeID;
	ss = s;
    
}

Inv::~Inv() {
	// TODO Auto-generated destructor stub
}

void Inv::doOperation(mpz_t* shares, mpz_t* results, int size, int threadID){
	int peers = ss->getPeers(); 
	mpz_t* R = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t** buffer = (mpz_t**)malloc(sizeof(mpz_t*) * peers);
	mpz_t* temp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	//initialziation
	for(int i = 0; i < peers; i++){
		buffer[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
		for(int j = 0; j < size; j++)
			mpz_init(buffer[i][j]);
	}

	for(int i = 0; i < size; i++){
		mpz_init(R[i]);
		mpz_init(temp[i]); 	
	}

	//start computation
	//Rand->generateRandValue(id, ss->getBits(), size, R, threadID);
	mpz_t field; //
	mpz_init(field); 
	ss->getFieldSize(field);
	Rand->generateRandValue(id, field, size, R, threadID); //
	ss->modMul(temp, shares, R, size);
	net.broadcastToPeers(temp, size, buffer, threadID);
	ss->reconstructSecret(results, buffer, size, true);
	ss->modInv(results, results, size);
	ss->modMul(results, R, results, size);
	
	//memory free
	for(int i = 0; i < peers; i++){
		for(int j = 0; j < size; j++)
			mpz_clear(buffer[i][j]);
		free(buffer[i]); 
	}
	free(buffer); 

	for(int i = 0; i < size; i++){
		mpz_clear(R[i]);
		mpz_clear(temp[i]); 	
	}
	free(R); 
	free(temp); 
}

