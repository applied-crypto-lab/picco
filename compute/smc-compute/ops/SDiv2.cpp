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
#include "SDiv.h"

SDiv::SDiv(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare *s, mpz_t coeficients[], int K1) {
    
	Mul = new Mult(nodeNet, nodeID, s);
	Mod = new Mod2M(nodeNet, poly, nodeID, s, coeficients);
	Truncpr= new TruncPr(nodeNet, poly, nodeID, s, coeficients);
    
	net = nodeNet;
	id = nodeID;
	ss = s;
	K = K1;
}

SDiv::~SDiv() {
	// TODO Auto-generated destructor stub
}

void SDiv::doOperation(mpz_t* Y, mpz_t* A, mpz_t* B, int size){
	int peers = ss->getPeers(); 
	mpz_t const2, constK1;
	mpz_t* X = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* const2K1 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* temp1 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* temp2 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* temp3 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	
	// initialization	
	mpz_init_set_ui(const2, 2); 
	mpz_init_set_ui(constK1, K+1); 
	
	for(int i = 0; i < size; i++){
		mpz_init(const2K1[i]);
		mpz_init(temp1[i]);
		mpz_init(temp2[i]);
		mpz_init(temp3[i]);
		mpz_init(X[i]);
	}
	// start computation
	ss->modPow(const2K1[0], const2, constK1);
	int sita = ceil(log2(K));
	ss->copy(A, Y, size); 
	ss->copy(B, X, size);

    /***** test purpose ********/
       mpz_t* temp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
       for(int i = 0; i < size; i++)
           mpz_init(temp[i]); 
       mpz_t** shares = (mpz_t**)malloc(sizeof(mpz_t*) * peers); 
       for(int i = 0; i < peers; i++){
           shares[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
           for(int j = 0; j < size; j++){
                mpz_init(shares[i][j]);
          }
    }




	for(int i = 0; i < size; i++)
		mpz_set(const2K1[i], const2K1[0]);
	
	for(int i = 1; i <= sita-1; i++){
		ss->modSub(temp2, const2K1, X, size);
		/*********/
		/*net.broadcastToPeers(temp2, size, shares); 
		ss->reconstructSecret(temp, shares, size, true); 
		for(int i = 0; i < size; i++)
			gmp_printf("temp2 : %Zd\n", temp[i]); */
		/********/
		Mul->doOperation(temp3, Y, temp2, size);
		Truncpr->doOperation(Y, temp3, 2*K+1, K, size);
		ss->modSub(temp2, const2K1, X, size);
		Mul->doOperation(temp3, X, temp2, size);
		Truncpr->doOperation(X, temp3, 2*K+1, K, size);
	}
	ss->modSub(temp2, const2K1, X, size);
	Mul->doOperation(temp3, Y, temp2, size);
	Truncpr->doOperation(Y, temp3, 2*K+1, K, size); 
	//free the memory
	mpz_clear(const2); 
	mpz_clear(constK1); 
	
	for(int i = 0; i < size; i++){
		mpz_clear(const2K1[i]);
		mpz_clear(temp1[i]);
		mpz_clear(temp2[i]);
		mpz_clear(temp3[i]);
		mpz_clear(X[i]);
	}

	free(const2K1); 
	free(temp1); 
	free(temp2); 
	free(temp3); 
	free(X); 
}
