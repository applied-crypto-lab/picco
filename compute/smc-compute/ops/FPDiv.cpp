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
#include "FPDiv.h"


FPDiv::FPDiv(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare *s, mpz_t coeficients[]) {
    
	Mul = new Mult(nodeNet, nodeID, s);
	App = new AppRcr(nodeNet,poly,nodeID,s,coeficients);
	//need to use 
	T = new TruncPr(nodeNet,poly,nodeID,s,coeficients);
	net = nodeNet;
	id = nodeID;
    	ss = s;
    
}

FPDiv::~FPDiv() {
	// TODO Auto-generated destructor stub
}

void FPDiv::doOperation(mpz_t* result, mpz_t* a, mpz_t* b, int k, int f, int size, int threadID){
	
	int peers = ss->getPeers(); 
	//Set theta
	double t = k/3.5;
	int theta = ceil(log2(t));
	
	//Set alpha
	mpz_t alpha, const2;
	mpz_init_set_ui(const2, 2); 
	mpz_init(alpha); 
	mpz_pow_ui(alpha, const2, 2*f);
	
	/**************************************************/
	/*mpz_t** resultShares = (mpz_t**)malloc(sizeof(mpz_t*) * peers); 
	mpz_t* tmpResults = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	for(int i = 0; i < size; i++)
		mpz_init(tmpResults[i]); 
	
	for(int i = 0; i < peers; i++)
	{
		resultShares[i] = (mpz_t*)malloc(sizeof(mpz_t) * size); 
		for(int j = 0; j < size; j++)
			mpz_init(resultShares[i][j]); 
	}

	/**************************************************/
	/*********************************/
	/*printf("line 1 of FPDiv...\n"); 
	gmp_printf("theta: %d, alpha: %Zd\n", theta, alpha); 
	/*********************************/
	/***********************************/
	/*printf("test for correctness of inputs...\n"); 
	
  	net.broadcastToPeers(a, size, resultShares);
        ss->reconstructSecret(tmpResults, resultShares, size, true);
	for(int i = 0; i < size; i++)
		gmp_printf("%Zd ", tmpResults[i]); 
	printf("\n"); 
	
  	net.broadcastToPeers(b, size, resultShares);
        ss->reconstructSecret(tmpResults, resultShares, size, true);
	for(int i = 0; i < size; i++)
		gmp_printf("%Zd ", tmpResults[i]); 
	printf("\n"); 

	/**********************************/
	mpz_t* w = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* x = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* y = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* temp = (mpz_t*)malloc(sizeof(mpz_t) * size);

	// do initialization     
	for(int i=0; i<size; ++i){
		mpz_init(w[i]);
		mpz_init(x[i]);
		mpz_init(y[i]);
		mpz_init(temp[i]);
	}
    
	App->doOperation(w, b, k, f, size, threadID);
	/************************************/
	/*printf("Result of App...\n"); 
	net.broadcastToPeers(w, size, resultShares); 
	ss->reconstructSecret(tmpResults, resultShares, size, true); 
	for(int i = 0; i < size; i++)
		gmp_printf("%Zd ", tmpResults[i]); 
	printf("\n"); 
	
	printf("a: "); 
	net.broadcastToPeers(a, size, resultShares); 
	ss->reconstructSecret(tmpResults, resultShares, size, true); 
	for(int i = 0; i < size; i++)
		gmp_printf("%Zd ", tmpResults[i]); 
	printf("\n"); 
	
	printf("b: "); 
	net.broadcastToPeers(b, size, resultShares); 
	ss->reconstructSecret(tmpResults, resultShares, size, true); 
	for(int i = 0; i < size; i++)
		gmp_printf("%Zd ", tmpResults[i]); 
	printf("\n"); 
	/************************************/
	Mul->doOperation(x, b, w, size, threadID);
	Mul->doOperation(y, a, w, size, threadID);
	
	for(int i=0; i<size; i++)
		ss->modSub(x[i], alpha, x[i]);
	
	/*
 	printf("line 3 x: "); 
	net.broadcastToPeers(x, size, resultShares); 
	ss->reconstructSecret(tmpResults, resultShares, size, true); 
	for(int i = 0; i < size; i++)
		gmp_printf("%Zd ", tmpResults[i]); 
	printf("\n"); 
	
	printf("line 4 y: "); 
	net.broadcastToPeers(y, size, resultShares); 
	ss->reconstructSecret(tmpResults, resultShares, size, true); 
	for(int i = 0; i < size; i++)
		gmp_printf("%Zd ", tmpResults[i]); 
	printf("\n"); 
	*/
	
	T->doOperation(y, y, 2*k, f, size, threadID);
	
	/*
	printf("line 5 y: "); 
	net.broadcastToPeers(y, size, resultShares); 
	ss->reconstructSecret(tmpResults, resultShares, size, true); 
	for(int i = 0; i < size; i++)
		gmp_printf("%Zd ", tmpResults[i]); 
	printf("\n"); 
	*/

	for(int i=0; i<theta-1; i++){
		//printf("round %d: \n", i); 
		for(int j=0; j<size; j++)
			ss->modAdd(temp[j], alpha, x[j]);
		Mul->doOperation(y, y, temp, size, threadID);
		/*
		printf("line 7 y: "); 
		net.broadcastToPeers(y, size, resultShares); 
		ss->reconstructSecret(tmpResults, resultShares, size, true); 
		for(int i = 0; i < size; i++)
			gmp_printf("%Zd ", tmpResults[i]); 
		printf("\n"); 
		*/
		Mul->doOperation(x, x, x, size, threadID);
		/*
		printf("line 8 x: "); 
		net.broadcastToPeers(x, size, resultShares); 
		ss->reconstructSecret(tmpResults, resultShares, size, true); 
		for(int i = 0; i < size; i++)
			gmp_printf("%Zd ", tmpResults[i]); 
		printf("\n"); 
		*/
		T->doOperation(y, y, 2*k, 2*f, size, threadID);
		/*
		printf("line 9 y: "); 
		net.broadcastToPeers(y, size, resultShares); 
		ss->reconstructSecret(tmpResults, resultShares, size, true); 
		for(int i = 0; i < size; i++)
			gmp_printf("%Zd ", tmpResults[i]); 
		printf("\n"); 
		*/
		T->doOperation(x, x, 2*k, 2*f, size, threadID);
	
		/*
		printf("line 10 x: "); 
		net.broadcastToPeers(x, size, resultShares); 
		ss->reconstructSecret(tmpResults, resultShares, size, true); 
		for(int i = 0; i < size; i++)
			gmp_printf("%Zd ", tmpResults[i]); 
		printf("\n"); 
		*/
	}

	for(int i=0; i<size; i++)
		ss->modAdd(x[i],alpha,x[i]);
	
	Mul->doOperation(y, y, x, size, threadID);
	/*
	printf("line 11 y: "); 
	net.broadcastToPeers(y, size, resultShares); 
	ss->reconstructSecret(tmpResults, resultShares, size, true); 
	for(int i = 0; i < size; i++)
		gmp_printf("%Zd ", tmpResults[i]); 
	printf("\n"); 
	*/
	T->doOperation(result, y, 2*k, 2*f, size, threadID);
	/*
	printf("line 12 y:\n"); 
	net.broadcastToPeers(result, size, resultShares); 
	ss->reconstructSecret(tmpResults, resultShares, size, true); 
	for(int i = 0; i < size; i++)
		gmp_printf("%Zd ", tmpResults[i]); 
	printf("\n\n"); 
	*/
	// free the memory
	mpz_clear(const2); 
	mpz_clear(alpha); 
	for(int i=0; i<size; ++i){
		mpz_clear(w[i]);
		mpz_clear(x[i]);
		mpz_clear(y[i]);
		mpz_clear(temp[i]);
	}
	free(w); 
	free(x); 
	free(y); 
	free(temp); 
}

