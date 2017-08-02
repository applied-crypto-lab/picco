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
#include "PreOr.h"

PreOr::PreOr(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare* s, mpz_t coefficients[]) {
	M2 = new Mod2(nodeNet, poly, nodeID, s);
	PreMul = new PrefixMultiplication(nodeNet, poly, nodeID, s, coefficients);
	net = nodeNet;
	id = nodeID;
	ss = s;
}

PreOr::~PreOr() {}

//results[K][size]
//C[K][size]
//K = Number of things in a single prefix operations
//size = Number of prefix operations to do in parallel 
void PreOr::doOperation(mpz_t** result, mpz_t** C, int K, int size, int threadID){
	mpz_t** b = (mpz_t**)malloc(sizeof(mpz_t*) * K);
	mpz_t* c = (mpz_t*)malloc(sizeof(mpz_t) * K * size);
	//initialization
	for(int i=0; i<K; ++i){
		b[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
		for(int j=0; j<size; ++j)
			mpz_init(b[i][j]);
	}
	for(int i=0; i<K*size; ++i)
		mpz_init(c[i]);
	for(int i=0; i<K; ++i){
		for(int j=0; j<size; ++j){ 
			ss->modAdd(b[i][j],C[i][j],1);
		}
	}
	PreMul->doOperation(b, b, K, size, threadID);
	for(int i=0; i<size; ++i){
		mpz_set(result[0][i],C[0][i]);
	}
	for(int i=0; i<K; ++i){
		for(int j=0; j<size; ++j){
			mpz_set(c[i*size+j],b[i][j]);
		}
	}
	M2->doOperation(c, c, K, K*size, threadID);
	for(int i=1; i<K; i++)
		for(int j=0; j<size; ++j)
			ss->modSub(result[i][j], 1, c[i*size+j]);
	//free the memory
	for(int i=0; i<K; ++i){
		for(int j=0; j<size; ++j)
			mpz_clear(b[i][j]);
		free(b[i]); 
	}
	for(int i=0; i<K*size; ++i)
		mpz_clear(c[i]);
	free(b);
	free(c);
}


