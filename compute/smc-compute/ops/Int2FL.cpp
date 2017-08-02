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
#include "Int2FL.h"

Int2FL::Int2FL() {
	// TODO Auto-generated constructor stub

}

Int2FL::Int2FL(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare *s, mpz_t coeficients[]) {
    
	Mul = new Mult(nodeNet,nodeID,s);
	Lt = new LTZ(nodeNet,poly,nodeID,s,coeficients);
	Eq = new EQZ(nodeNet,poly,nodeID,s,coeficients); 
	Bd = new BitDec(nodeNet,poly,nodeID,s,coeficients);
	Pre = new PreOr(nodeNet,poly,nodeID,s,coeficients);
	T = new Trunc(nodeNet,poly,nodeID,s,coeficients);     
	net = nodeNet;
	id = nodeID;
	ss = s;
}

Int2FL::~Int2FL() {
	// TODO Auto-generated destructor stub
}

void Int2FL::doOperation(mpz_t* values, mpz_t** results1, int gamma, int K, int size, int threadID){
	
	mpz_t** results = (mpz_t**)malloc(sizeof(mpz_t*) * 4); 
	for(int i = 0; i < 4; i++){
		results[i] = (mpz_t*)malloc(sizeof(mpz_t) * size); 
		for(int j = 0; j < size; j++)
			mpz_init_set(results[i][j], results1[j][i]); 
	}
	mpz_t const1, const2, constK, tmp; 
	
	mpz_t* temp1 = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* temp2 = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* A = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* P = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t** S = (mpz_t**)malloc(sizeof(mpz_t*) * gamma); 
	mpz_t* pow2K = (mpz_t*)malloc(sizeof(mpz_t) * (gamma-1)); 
	
	mpz_init_set_ui(const1, 1); 
	mpz_init_set_ui(const2, 2); 
	mpz_init_set_ui(constK, K);
	mpz_init(tmp);  
	for(int i = 0; i < size; i++)
	{
		mpz_init(temp1[i]); 
		mpz_init(temp2[i]); 
		mpz_init_set(A[i], values[i]); 
		mpz_init(P[i]); 
	}
		
	for(int i = 0; i < gamma; i++)
	{
		S[i] = (mpz_t*)malloc(sizeof(mpz_t) * size); 
		for(int j = 0; j < size; j++)
			mpz_init(S[i][j]);
	}
	
	for(int i = 0; i < gamma-1; i++)
	{
		mpz_init(pow2K[i]); 
		ss->modPow(pow2K[i], const2, i); 
	}
	int lambda = gamma-1;
	//line 2
	Lt->doOperation(results[3], A, gamma, size, threadID); 
	//line 3
	Eq->doOperation(A, results[2], gamma, size, threadID); 
	//line 4
	ss->modMul(temp1, results[3], const2, size); 
	ss->modSub(temp1, const1, temp1, size); 
	Mul->doOperation(A, temp1, A, size, threadID); 
	//line 5 and 6
	Bd->doOperation(S, A, lambda, lambda, size, threadID); 
	for(int i = 0; i < lambda && i <= lambda-i-1; i++)
		for(int j = 0; j < size; j++){
			mpz_set(tmp, S[i][j]); 
			mpz_set(S[i][j], S[lambda-i-1][j]); 
			mpz_set(S[lambda-i-1][j], tmp); 
		} 
	Pre->doOperation(S, S, lambda, size, threadID); 
	//line 7
	for(int i = 0; i < lambda; i++)
	{
		for(int j = 0; j < size; j++)
			mpz_set(temp1[j], pow2K[i]); 
		ss->modSub(temp2, const1, S[i], size);
		ss->modMul(temp2, temp1, temp2, size); 
		ss->modAdd(P, P, temp2, size);  
	}
	ss->modAdd(P, P, const1, size); 
	Mul->doOperation(A, A, P, size, threadID); 
	//line 8 and 11
	for(int i = 0; i < lambda; i++)
		ss->modAdd(results[1], results[1], S[i], size);
	ss->modSub(results[1], results[1], constK, size);
	ss->modSub(temp1, const1, results[2], size);   
	Mul->doOperation(results[1], results[1], temp1, size, threadID); 
	//line 9 and 10
	if(lambda > K)
		T->doOperation(results[0], A, lambda, lambda-K, size, threadID); 
	else
	{
		ss->modPow(temp1[0], const2, K-lambda); 
		for(int i = 1; i < size; i++)
			mpz_set(temp1[i], temp1[0]); 
		ss->modMul(results[0], A, temp1, size); 
	}
	
	//copy the result 
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < size; j++)
			mpz_set(results1[j][i], results[i][j]); 

	//free the memory
	mpz_clear(const1); 
	mpz_clear(const2); 
	mpz_clear(constK);
	mpz_clear(tmp);  
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < size; j++)
			mpz_clear(results[i][j]); 
		free(results[i]); 
	}
	free(results); 

	for(int i = 0; i < size; i++)
	{
		mpz_clear(temp1[i]); 
		mpz_clear(temp2[i]); 
		mpz_clear(A[i]); 
		mpz_clear(P[i]); 
	}
	free(temp1); 
	free(temp2); 
	free(A); 
	free(P); 
	
	for(int i = 0; i < gamma; i++)
	{
		for(int j = 0; j < size; j++)
			mpz_clear(S[i][j]);
		free(S[i]); 
	}
	free(S); 

	for(int i = 0; i < gamma-1; i++)
		mpz_clear(pow2K[i]); 
	free(pow2K); 
}
