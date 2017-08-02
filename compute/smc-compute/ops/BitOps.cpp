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
#include "BitOps.h"

BitOps::BitOps(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int NodeID, SecretShare *s, mpz_t coeficients[]){
	Mul = new Mult(nodeNet, NodeID, s);
	net = nodeNet;
	id = NodeID;
	ss = s;
} 		

BitOps::~BitOps(){}

void BitOps::BitAnd(mpz_t* A, mpz_t* B, mpz_t* result, int size, int threadID){
	Mul->doOperation(result, A, B, size, threadID);
}

void BitOps::BitOr(mpz_t* A, mpz_t* B, mpz_t* result, int size, int threadID){
	mpz_t* C = (mpz_t*)malloc(sizeof(mpz_t) * size);
	for(int i=0; i<size; ++i)
		mpz_init(C[i]);
	// (a+b) - ab
	Mul->doOperation(C, A, B, size, threadID);
	ss->modAdd(result, A, B, size);
	ss->modSub(result, result, C, size);
	//free the memory	
	for(int i=0; i<size; ++i)
		mpz_clear(C[i]);
	free(C); 
}

void BitOps::BitXor(mpz_t* A, mpz_t* B, mpz_t* result, int size, int threadID){
	mpz_t* C = (mpz_t*)malloc(sizeof(mpz_t) * size);
	for(int i=0; i<size; ++i)
		mpz_init(C[i]);
	//(a+b) - 2ab
	Mul->doOperation(C, A, B, size, threadID);
	ss->modMul(C, C, 2, size);
	ss->modAdd(result, A, B, size);
	ss->modSub(result, result, C, size);
	//free the memory
	for(int i=0; i<size; ++i)
		mpz_clear(C[i]);
	free(C); 	
}

