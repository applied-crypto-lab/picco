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

#include "FL2Int.h"

FL2Int::FL2Int() {
	// TODO Auto-generated constructor stub

}

FL2Int::FL2Int(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare *s, mpz_t coefficients[]) {
    
	Mul = new Mult(nodeNet, nodeID, s);
	Lt = new LTZ(nodeNet, poly, nodeID, s, coefficients);
	Flround = new FLRound(nodeNet, poly, nodeID, s, coefficients); 
	Mod2ms = new Mod2MS(nodeNet, poly, nodeID, s, coefficients);
	P = new Pow2(nodeNet, poly, nodeID, s, coefficients);
	I = new Inv(nodeNet, poly, nodeID, s, coefficients);     
	net = nodeNet;
	id = nodeID;
	ss = s;
}

FL2Int::~FL2Int() {
	// TODO Auto-generated destructor stub
}

void FL2Int::doOperation(mpz_t** values1, mpz_t* results, int L, int K, int gamma, int size, int threadID){
	
	mpz_t** values = (mpz_t**)malloc(sizeof(mpz_t*) * 4); 
	mpz_t** valuesP = (mpz_t**)malloc(sizeof(mpz_t) * 4); 
	for(int i = 0; i < 4; i++){
		values[i] = (mpz_t*)malloc(sizeof(mpz_t) * size); 
		valuesP[i] = (mpz_t*)malloc(sizeof(mpz_t) * size); 
		for(int j = 0; j < size; j++){
			mpz_init_set(values[i][j], values1[j][i]); 
			mpz_init(valuesP[i][j]); 
		}
	}
	mpz_t* modes = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* temp1 = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* temp2 = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* a = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* b = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* c = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* u = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* pow2 = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	
	for(int i = 0; i < size; i++){
		mpz_init_set_ui(modes[i], 0); 
		mpz_init(temp1[i]); 
		mpz_init(temp2[i]); 
		mpz_init(a[i]); 
		mpz_init(b[i]); 
		mpz_init(c[i]); 
		mpz_init(u[i]); 
		mpz_init(pow2[i]); 
	}

	/*****************************************************************/
	/*mpz_t** resultShares = (mpz_t**)malloc(sizeof(mpz_t*)*3);
	for(int i=0; i<3; ++i){
                resultShares[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
                for(int j=0; j<size; ++j)
                        mpz_init(resultShares[i][j]);
        }
	/*****************************************************************/
	//line 1
	Flround->doOperation(values, valuesP, modes, L, K, size, threadID);
	/********************************************/
	/*for(int i = 0; i < 4; i++){
		    net.broadcastToPeers(valuesP[i], size, resultShares, threadID);
        	    ss->reconstructSecret(temp1, resultShares, size, true);
		    for(int j = 0; j < size; j++)
			gmp_printf("%Zd ", temp1[j]); 
		    printf("\n"); 
	}
	/********************************************/ 
	//line 2
	ss->modSub(temp1, valuesP[1], gamma-1, size); 
	Lt->doOperation(a, temp1, K, size, threadID); 
	//line 3
	ss->modSub(temp1, gamma-L-1, valuesP[1], size); 
	Lt->doOperation(b, temp1, K, size, threadID); 

	//line 4
	Lt->doOperation(c, valuesP[1], K, size, threadID); 
	
	//line 5
	ss->modSub(temp1, gamma-1, valuesP[1], size); 
	ss->modSub(temp2, 1, c, size);
	Mul->doOperation(temp1, temp1, temp2, size, threadID); 
	Mul->doOperation(temp1, temp1, b, size, threadID); 
	Mul->doOperation(temp2, temp1, a, size, threadID); 

	//line 6
	Mod2ms->doOperation(u, valuesP[0], temp2, temp1, L, size, threadID); 
	//line 7
	Mul->doOperation(temp1, b, c, size, threadID); 
	ss->modAdd(temp1, temp1, 1, size);
	ss->modSub(temp1, temp1, b, size); 
	Mul->doOperation(temp1, temp1, valuesP[0], size, threadID); 

	ss->modSub(temp2, 1, c, size); 
	Mul->doOperation(temp2, temp2, u, size, threadID); 
	Mul->doOperation(temp2, temp2, b, size, threadID); 
	ss->modAdd(valuesP[0], temp1, temp2, size); 
	//line 8 
	Mul->doOperation(temp1, c, valuesP[1], size, threadID);
	ss->modSub(temp1, (long)0, temp1, size); 
 	P->doOperation(pow2, temp1, L, size, threadID); 
			
	//line 9 
	I->doOperation(pow2, pow2, size, threadID); 
		
	//line 10
	Mul->doOperation(temp1, c, pow2, size, threadID); 
	ss->modAdd(temp1, temp1, 1, size); 
	ss->modSub(temp1, temp1, c, size); 
	Mul->doOperation(valuesP[0], valuesP[0], temp1, size, threadID); 
	// line 11 
	Mul->doOperation(temp1, b, c, size, threadID); 
	ss->modMul(temp1, temp1, gamma-1, size); 
	Mod2ms->doOperation(u, valuesP[0], temp1, temp2, L, size, threadID); 
	// line 12
	Mul->doOperation(temp1, b, c, size, threadID); 
	Mul->doOperation(temp1, temp1, u, size, threadID); 
	Mul->doOperation(temp2, b, c, size, threadID); 
	ss->modSub(temp2, 1, temp2, size); 
	Mul->doOperation(temp2, temp2, valuesP[0], size, threadID); 
	ss->modAdd(valuesP[0], temp2, temp1, size); 
	// line 13
	ss->modSub(temp1, 1, c, size); 
	Mul->doOperation(temp1, temp1, a, size, threadID); 
	Mul->doOperation(temp1, temp1, valuesP[1], size, threadID); 
	P->doOperation(pow2, temp1, gamma-1, size, threadID); 
	//line 14
	ss->modSub(temp1, 1, valuesP[2], size); 
	ss->modMul(temp2, valuesP[3], 2, size); 
	ss->modSub(temp2, 1, temp2, size); 
	Mul->doOperation(temp1, temp1, temp2, size, threadID); 
	Mul->doOperation(results, temp1, pow2, size, threadID); 
	Mul->doOperation(results, results, a, size, threadID); 
	Mul->doOperation(results, results, valuesP[0], size, threadID); 
	//free memory
	for(int i = 0; i < size; i++){
		mpz_clear(modes[i]); 
		mpz_clear(temp1[i]); 
		mpz_clear(temp2[i]); 
		mpz_clear(a[i]); 
		mpz_clear(b[i]); 
		mpz_clear(c[i]); 
		mpz_clear(u[i]); 
		mpz_clear(pow2[i]); 
	}
	free(modes); 
	free(temp1); 
	free(temp2); 
	free(a); 
	free(b); 
	free(c); 
	free(u); 
	free(pow2); 
	
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < size; j++)
		{
			mpz_clear(values[i][j]);
			mpz_clear(valuesP[i][j]);
		}
		free(values[i]); 
		free(valuesP[i]);  
	}

	free(values); 
	free(valuesP); 

}
