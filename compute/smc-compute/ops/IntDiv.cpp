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
#include "IntDiv.h"
#include <math.h>
IntDiv::IntDiv(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare *s, mpz_t coefficients[]) {
    
	Mul = new Mult(nodeNet, nodeID, s);
	App = new IntAppRcr(nodeNet,poly,nodeID,s,coefficients);
	//need to use 
	T = new TruncPr(nodeNet,poly,nodeID,s,coefficients);
	Eq = new EQZ(nodeNet,poly,nodeID,s,coefficients); 
	Lt = new LTZ(nodeNet,poly,nodeID,s,coefficients); 
	net = nodeNet;
	id = nodeID;
    	ss = s;
    
}

IntDiv::~IntDiv() {
	// TODO Auto-generated destructor stub
}

void IntDiv::doOperationPub(mpz_t* result, mpz_t* a, mpz_t* b, int k, int size, int threadID)
{
	int lambda = 8; 
	int peers = ss->getPeers(); 
	mpz_t const0, const1, const2, constk;
	mpz_init_set_ui(const0, 0);  
	mpz_init_set_ui(const1, 1);  
	mpz_init_set_ui(const2, 2);  
	mpz_init(constk); 
	mpz_pow_ui(constk, const2, k+lambda);
	
	mpz_t* denom = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* temp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* c = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* sign = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* lt = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* a_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	int* b_tmp = (int*)malloc(sizeof(int) * size); 	
	
	for(int i = 0; i < size; i++)
	{
		mpz_init(denom[i]); 
		mpz_init(temp[i]); 	
		mpz_init(c[i]); 
		mpz_init(sign[i]); 
		mpz_init(a_tmp[i]); 
		mpz_init(lt[i]); 
		b_tmp[i] = mpz_get_si(b[i]); 
		
	}

	/***************************************************/
	//compute the sign of a
	Lt->doOperation(lt, a, k, size, threadID); 
	ss->modSub(sign, const1, lt, size); 
	ss->modSub(sign, sign, lt, size); 
	//make a to be a positive value
	Mul->doOperation(c, a, lt, size, threadID); 
	ss->modSub(temp, a, c, size); 
	ss->modSub(temp, temp, c, size); 
	ss->copy(temp, a_tmp, size); 		
	for(int i = 0; i < size; i++)
		ss->modAdd(temp[i], const0, 1-2*(b_tmp[i]<0)); 
        //make b to be a positive value
        for(int i = 0; i < size; i++)
		b_tmp[i] = b_tmp[i] * (1-2*(b_tmp[i]<0)); 	
	ss->modMul(sign, sign, temp, size); 	
	/***************************************************/
	for(int i = 0; i < size; i++)
	{
		mpz_set_si(denom[i], b_tmp[i]); 
		mpz_div_ui(temp[i], constk, b_tmp[i]); 
	}

	ss->modMul(temp, temp, a_tmp, size); 
	T->doOperation(result, temp, 2*k+lambda, k+lambda, size, threadID);
	
 	ss->copy(result, c, size); 
        ss->modMul(temp, c, denom, size); 
	ss->modSub(temp, temp, a_tmp, size); 
	ss->modSub(temp, temp, const1, size); 
        Lt->doOperation(lt, temp, k, size, threadID); 
	ss->modAdd(lt, lt, c, size); 
	ss->modSub(result, lt, const1, size); 
	Mul->doOperation(result, result, sign, size, threadID); 
	
	//free the memory
	for(int i = 0; i < size; i++)
	{
		mpz_clear(denom[i]); 
		mpz_clear(temp[i]);
		mpz_clear(c[i]); 
		mpz_clear(sign[i]); 
		mpz_clear(lt[i]);  
		mpz_clear(a_tmp[i]);  
	}

	free(denom); 
	free(temp); 
	free(c); 
	free(sign); 
	free(lt); 
	free(a_tmp); 
	free(b_tmp); 

	mpz_clear(const0);
	mpz_clear(const1); 
	mpz_clear(const2);  
	mpz_clear(constk); 
	
}

void IntDiv::doOperation(mpz_t* result, mpz_t* a, mpz_t* b, int k, int size, int threadID){
	int peers = ss->getPeers(); 
	//Set theta
	double t = k/3.5;
	int theta = ceil(log2(t));
	int lambda = 6; 	
	//Set alpha
	mpz_t alpha, const1, const2, inv2;
	mpz_init_set_ui(const1, 1); 
	mpz_init_set_ui(const2, 2); 
	mpz_init(alpha);
	mpz_init(inv2);  
	mpz_pow_ui(alpha, const2, k); 
	ss->modInv(inv2, const2); 

	/*****************************************************/
	mpz_t* w = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* x = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* y = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* c = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* a_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* b_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* temp = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* lt = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* sign = (mpz_t*)malloc(sizeof(mpz_t) * size);
 	mpz_t* temp1 = (mpz_t*)malloc(sizeof(mpz_t) * 2 * size); 
 
	// do initialization     
	for(int i=0; i<size; ++i){
		mpz_init(w[i]);
		mpz_init(x[i]);
		mpz_init(y[i]);
		mpz_init(temp[i]);
		mpz_init(c[i]); 
		mpz_init(lt[i]);
		mpz_init(sign[i]);
		mpz_init(a_tmp[i]);
		mpz_init(b_tmp[i]);
		mpz_init(temp1[i]); 
		mpz_init(temp1[size+i]);  
		
	}

	/***********************************************/
	for(int i = 0; i < size; i++)
	{
		mpz_set(temp1[i], a[i]); 
		mpz_set(temp1[size+i], b[i]); 
	}

	Lt->doOperation(temp1, temp1, k, 2*size, threadID); 
	
	//compute the sign of a
	for(int i  = 0; i < size; i++)
		mpz_set(lt[i], temp1[i]); 
	ss->modSub(sign, const1, lt, size); 
	ss->modSub(sign, sign, lt, size); 
	
	//make a to be a positive value 
	Mul->doOperation(c, a, lt, size, threadID); 
	ss->modSub(temp, a, c, size); 
	ss->modSub(temp, temp, c, size); 
	ss->copy(temp, a_tmp, size); 		
	
	//compute the sign of b and the overall sign
	for(int i = 0; i < size; i++)
		mpz_set(lt[i], temp1[size+i]); 
	ss->modSub(temp, const1, lt, size); 
	ss->modSub(temp, temp, lt, size); 
	Mul->doOperation(sign, sign, temp, size, threadID); 	
	
	//make b to be a positive value
	Mul->doOperation(c, b, lt, size, threadID); 
	ss->modSub(temp, b, c, size); 
	ss->modSub(temp, temp, c, size); 
	ss->copy(temp, b_tmp, size); 	
	
	/***********************************************/
	App->doOperation(w, temp, k, size, threadID);
	Mul->doOperation(x, b_tmp, w, size, threadID);
	Mul->doOperation(y, a_tmp, w, size, threadID);
	ss->modSub(x, alpha, x, size);
	T->doOperation(y, y, 2*k, k-lambda, size, threadID);
	
	for(int i=0; i<theta-1; i++){
		ss->modAdd(temp, x, alpha, size);
		Mul->doOperation(y, y, temp, size, threadID);
		Mul->doOperation(x, x, x, size, threadID);
		T->doOperation(y, y, 2*k+lambda, k, size, threadID);
		T->doOperation(x, x, 2*k, k, size, threadID);
	}
	ss->modAdd(x, x, alpha, size);
	Mul->doOperation(y, y, x, size, threadID);
	T->doOperation(result, y, 2*k+lambda, k+lambda, size, threadID);
	/******************** VERSION 1 ***************************/
	/**********************************************************/
	ss->copy(result, c, size); 
	Mul->doOperation(temp, c, b_tmp, size, threadID);
	ss->modSub(temp, a_tmp, temp, size);  
	Lt->doOperation(lt, temp, k, size, threadID);
	ss->modMul(temp, lt, const2, size); 
	ss->modSub(temp, const1, temp, size); // d
	ss->modAdd(c, c, temp, size); 
	Mul->doOperation(temp, c, b_tmp, size, threadID); 
	ss->modSub(temp, a_tmp, temp, size); 
	Lt->doOperation(lt, temp, k, size, threadID); 
	ss->modMul(temp, lt, const2, size); 
	ss->modSub(temp, const1, temp, size); //d 
	ss->modSub(temp, const1, temp, size); 
	ss->modMul(temp, temp, inv2, size); 
	ss->modSub(c, c, temp, size); 
	ss->copy(c, result, size);
	Mul->doOperation(result, result, sign, size, threadID); 
	// free the memory
	mpz_clear(const1);
	mpz_clear(const2); 
	mpz_clear(inv2);  
	mpz_clear(alpha);
	for(int i=0; i<size; ++i){
		mpz_clear(w[i]);
		mpz_clear(x[i]);
		mpz_clear(y[i]);
		mpz_clear(temp[i]);
		mpz_clear(c[i]); 
		mpz_clear(sign[i]); 
		mpz_clear(lt[i]);
		mpz_clear(a_tmp[i]); 
		mpz_clear(b_tmp[i]); 
		mpz_clear(temp1[i]); 
		mpz_clear(temp1[size+i]); 
	}
	free(w); 
	free(x); 
	free(y); 
	free(temp); 
	free(temp1); 
	free(c);
	free(sign); 
	free(lt); 
	free(a_tmp); 
	free(b_tmp); 
}

