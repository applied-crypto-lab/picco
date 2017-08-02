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
#include "Norm.h"

Norm::Norm() {
	// TODO Auto-generated constructor stub

}

Norm::Norm(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare *s, mpz_t coefficients[]) {
	Mul = new Mult(nodeNet, nodeID, s);
	Lt = new LTZ(nodeNet,poly,nodeID,s,coefficients);
	Bt = new BitDec(nodeNet,poly,nodeID,s,coefficients);
	Pre = new PreOr(nodeNet,poly,nodeID,s,coefficients);
    
	net = nodeNet;
	id = nodeID;
	ss = s;
    
}

Norm::~Norm() {
	// TODO Auto-generated destructor stub
}

// ndeds more tests
void Norm::doOperation(mpz_t* c, mpz_t* vp, mpz_t* b, int k, int f, int size, int threadID){
	mpz_t one, two, temp;
	mpz_init_set_ui(one,1);
	mpz_init_set_ui(two,2);
	mpz_init(temp);
	
	int peers = ss->getPeers(); 
	//initialization	
	mpz_t* s = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* x = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* v = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t** xb = (mpz_t**)malloc(sizeof(mpz_t*) * (k+1));
        mpz_t** xb1 = (mpz_t**)malloc(sizeof(mpz_t*) * (k+1));

	
        for(int i=0; i<size; ++i){
                mpz_init(s[i]);
                mpz_init(x[i]);
                mpz_init(v[i]);
        }

        for(int i=0; i<k+1; ++i){
                xb[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
                xb1[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
                for(int j=0; j<size; ++j){
                        mpz_init(xb[i][j]);
                        mpz_init(xb1[i][j]);
                }
        }
	//start computation 
	Lt->doOperation(s, b, k, size, threadID);
	ss->modMul(s, s, two, size); 
	ss->modSub(s, one, s, size); 
	Mul->doOperation(x, s, b, size, threadID);
	Bt->doOperation(xb, x, k, k, size, threadID);
	for(int i=0; i<k; ++i)
		for(int j=0; j<size; ++j)
			mpz_set(xb1[k-i-1][j],xb[i][j]);
	Pre->doOperation(xb1, xb1, k, size, threadID);
	for(int i = 0; i < k; i++)
		for(int j = 0; j < size; j++)
			mpz_set(xb[i][j], xb1[k-i-1][j]); 
	for(int i = 0; i < k-1; i++)
		ss->modSub(xb1[i], xb[i], xb[i+1], size);
    	for(int i = 0; i < size; i++)
        	mpz_set(xb1[k-1][i], xb[k-1][i]);
    
    	//summation here
    	for(int i=0; i<k; ++i){
        	for(int j=0; j<size; ++j){
            		mpz_set_ui(temp, (k-i-1));
            		ss->modPow(temp, two, temp);
            		ss->modMul(temp, temp, xb1[i][j]);
            		ss->modAdd(v[j], v[j], temp);
        	}
    	}

    	Mul->doOperation(c, x, v, size, threadID);
    	Mul->doOperation(vp, s, v, size, threadID);
	
    	//free the memory
	for(int i=0; i<size; ++i){
		mpz_clear(s[i]);
		mpz_clear(x[i]);
		mpz_clear(v[i]);
	}
	free(s);
	free(x);
	free(v);
	
	for(int i=0; i<k+1; ++i){
		for(int j=0; j<size; ++j){
			mpz_clear(xb[i][j]);
			mpz_clear(xb1[i][j]); 
		}
		free(xb[i]); 
		free(xb1[i]); 
	}
	free(xb); 
	free(xb1);
	mpz_clear(one); 
	mpz_clear(two); 
	mpz_clear(temp);  
}
