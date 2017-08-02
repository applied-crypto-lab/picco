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

#include "BitLTC.h"

BitLTC::BitLTC(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare *s, mpz_t coeficients[]){
    
	PreMul = new PrefixMultiplication(nodeNet, poly, nodeID, s, coeficients);
	Mod = new Mod2(nodeNet, poly, nodeID, s);
    	ss = s;
	net = nodeNet;
	polynomials = poly;
	id = nodeID;
	for (int i = 0; i < 9; i++){ //Not optimal, pass this thing by pointer somehow
		mpz_init(coef[i]);
		mpz_set(coef[i],coeficients[i]);
	}
}

BitLTC::~BitLTC() {
	// TODO Auto-generated destructor stub
}

void BitLTC::doOperation(mpz_t* A, mpz_t** b, mpz_t* result, int K, int size, int threadID){
	mpz_t** d = (mpz_t**)malloc(sizeof(mpz_t*) * K);
	mpz_t** a = (mpz_t**)malloc(sizeof(mpz_t*) * size);
	mpz_t* temp = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* temp1 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* temp2 = (mpz_t*)malloc(sizeof(mpz_t) * K); 	
	mpz_t tmp, const1, const2;
	
	//initialization
	mpz_init(tmp); 
	mpz_init_set_ui(const1, 1); 
	mpz_init_set_ui(const2, 2); 
	for(int i = 0; i < K; i++){
		d[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
		for(int j = 0; j < size; j++)
			mpz_init(d[i][j]);
		mpz_init(temp2[i]); 
	}
	for(int i = 0; i < size; i++){
		a[i] = (mpz_t*)malloc(sizeof(mpz_t) * K);
		for(int j = 0; j < K; j++)
			mpz_init(a[i][j]);
	}

	for(int i = 0; i < size; i++){
		mpz_init(temp[i]);
		mpz_init(temp1[i]); 
	}

	//start computation
	for(int m = 0; m < size; m++)
		binarySplit(A[m], a[m], K);
	for(int i = 0; i < K; i++){
		for(int j = 0; j < size; j++)
			mpz_set(temp1[j], a[j][i]); 
		ss->modMul(temp, temp1, b[i], size);
		ss->modMul(temp, temp, const2, size);
		ss->modSub(temp, b[i], temp, size);
		ss->modAdd(temp, temp1, temp, size);
		ss->modAdd(temp, temp, const1, size);
		for(int j = 0; j < size; j++)
			mpz_set(d[K-1-i][j], temp[j]);
	}
	
	PreMul->doOperation(d, d, K, size, threadID);
		
	for(int m = 0; m < size; m++){
		mpz_set_ui(temp[m], 0);
		for(int i = 0; i < K; i++)
                        mpz_set(temp2[i], d[K-1-i][m]);
                for(int i = 0; i < K-1; i++)
                        ss->modSub(temp2[i], temp2[i], temp2[i+1]);
                ss->modSub(temp2[K-1], temp2[K-1], const1);
		for(int i = 0; i < K; i++){
			ss->modSub(tmp, const1, a[m][i]);
			ss->modMul(tmp, temp2[i], tmp);
			ss->modAdd(tmp, tmp, temp[m]);
			mpz_set(temp[m], tmp);
		}
	}

	Mod->doOperation(temp, result, K, size, threadID);
	
	//free the memory
	for(int i = 0; i < K; i++){
		for(int j = 0; j < size; j++)
			mpz_clear(d[i][j]);
		mpz_clear(temp2[i]); 
		free(d[i]); 
	}
	free(temp2); 
	free(d); 

	for(int i = 0; i < size; i++){
		for(int j = 0; j < K; j++)
			mpz_clear(a[i][j]);
		free(a[i]); 
	}
	free(a); 

	for(int i = 0; i < size; i++){
		mpz_clear(temp[i]);
		mpz_clear(temp1[i]); 
	}
	free(temp);
	free(temp1);  
	
	mpz_clear(const1); 
	mpz_clear(const2); 
	mpz_clear(tmp);  
}
