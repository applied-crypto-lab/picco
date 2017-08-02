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
#include "AppRcr.h"


AppRcr::AppRcr(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare *s, mpz_t coeficients[]) {
	
	Mul = new Mult(nodeNet, nodeID, s);
	T = new TruncPr(nodeNet,poly,nodeID,s,coeficients);
	No = new Norm(nodeNet,poly,nodeID,s,coeficients);
    	ss = s;
	net = nodeNet; 
	id = nodeID; 
}

AppRcr::~AppRcr() {
	// TODO Auto-generated destructor stub
}

void AppRcr::doOperation(mpz_t* w, mpz_t* b, int k, int f, int size, int threadID){
	mpz_t one, two, temp, alpha;
	mpz_init_set_ui(one, 1); 
	mpz_init_set_ui(two, 2);
	mpz_init(temp);
	mpz_init(alpha);

	int peers = ss->getPeers(); 
	
	mpf_t num1, num2; 
	mpf_init(num1); 
	mpf_init(num2); 

	mpz_t* c = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* v = (mpz_t*)malloc(sizeof(mpz_t) * size);

	for(int i=0; i<size; ++i){
		mpz_init(c[i]);
		mpz_init(v[i]);
	}
	//compute alpha 
	mpf_set_d(num1, 2.9142); 
	mpz_pow_ui(alpha, two, k);
	mpf_set_z(num2, alpha); 
	mpf_mul(num2, num2, num1); 
	mpz_set_f(temp, num2); 
	ss->modMul(alpha, temp, one);
	//finish the rest of computation
	No->doOperation(c, v, b, k, f, size,  threadID);
	ss->modMul(c, c, two, size); 
	ss->modSub(c, alpha, c, size); 
	Mul->doOperation(c, c, v, size, threadID);
	T->doOperation(w, c, 2*k, 2*(k-f), size, threadID);

	//free the memory
	for(int i=0; i<size; ++i){
		mpz_clear(c[i]);
		mpz_clear(v[i]);
	}
	free(c); 
	free(v); 

	mpz_clear(alpha);
	mpz_clear(one);  
	mpz_clear(two); 
	mpz_clear(temp); 
	mpf_clear(num1); 
	mpf_clear(num2);  
}
