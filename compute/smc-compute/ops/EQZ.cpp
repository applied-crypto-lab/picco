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


#include "EQZ.h"


EQZ::EQZ(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int NodeID, SecretShare *s, mpz_t coefficients[]) {
    
	PreMul = new PrefixMultiplication(nodeNet,poly,NodeID,s,coefficients);
	Rand = new Random(nodeNet, poly, NodeID, s);
    
	net = nodeNet;
	id = NodeID;
	ss = s;

	for (int i = 0; i < 9; i++){ //Not optimal, pass this thing by pointer somehow
		mpz_init(coef[i]);
		mpz_set(coef[i],coefficients[i]);
	}
}

EQZ::~EQZ() {
	// TODO Auto-generated destructor stub
}

void EQZ::doOperation(mpz_t* shares, mpz_t* result, int K, int size, int threadID){
	int peers = ss->getPeers(); 
	int m = 8; //works for 256 bit field size
	mpz_t* S = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* bitK = (mpz_t*)malloc(sizeof(mpz_t) * K); 
	mpz_t* bitm = (mpz_t*)malloc(sizeof(mpz_t) * m); 
	mpz_t** resultShares = (mpz_t**)malloc(sizeof(mpz_t*)*peers);
	mpz_t* C = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* c = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* sum = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t temp1, temp2, const1, const2, constK, constm, const2K, const2m;
	
	//initialization
	mpz_init(temp1);
	mpz_init(temp2);
	mpz_init_set_ui(const1, 1);
	mpz_init_set_ui(const2, 2);
	mpz_init_set_ui(constK, K); 
	mpz_init_set_ui(constm, m); 
	mpz_init(const2K); 
	mpz_init(const2m); 

	for(int i = 0; i < K; i++)
		mpz_init(bitK[i]); 
	for(int i = 0; i < m; i++)
		mpz_init(bitm[i]); 

	for(int i=0; i<size; ++i){
		mpz_init(C[i]);
		mpz_init(c[i]);
		mpz_init(S[i]); 
		mpz_init(sum[i]);
	}

	for(int i=0; i<peers; ++i){
		resultShares[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
		for(int j=0; j<size; ++j)
			mpz_init(resultShares[i][j]);
	}

	/**************** EQZ (PART 1): LINE 1-3 ******************/
	mpz_t** V = (mpz_t**)malloc(sizeof(mpz_t*) * (K+2));
	for(int i=0; i<K+2; ++i){
                V[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
                for(int j=0; j<size; ++j)
                        mpz_init(V[i][j]);
        }

	Rand->PRandM(K, K, size, V, threadID);
        ss->modAdd(C, shares, V[K], size);
	Rand->PRandInt(K, K, size, S, threadID);
	ss->modPow(const2K, const2, constK);
 	ss->modMul(S, S, const2K, size); 
	ss->modAdd(C, C, S, size);
	net.broadcastToPeers(C, size, resultShares, threadID);
	ss->reconstructSecret(c, resultShares, size, true);
	for(int i = 0; i < size; i++){
		binarySplit(c[i], bitK, K);
		for(int j = 0; j < K; j++){
			ss->modAdd(temp1, bitK[j], V[j][i]);
			ss->modMul(temp2, bitK[j], V[j][i]);
			ss->modMul(temp2, temp2, const2);
			ss->modSub(temp1, temp1, temp2);
			ss->modAdd(sum[i], sum[i], temp1);
		}
	}
	for(int i=0; i<K+2; ++i){
                for(int j=0; j<size; ++j)
                        mpz_clear(V[i][j]);
                free(V[i]);
        }
        free(V);
	/**************** EQZ (PART 2): LINE 1-5 of KOrCL ******************/
	mpz_t** U = (mpz_t**)malloc(sizeof(mpz_t*) * (m+2));
	for(int i = 0; i < m+2; i++){
                        U[i] = (mpz_t*)malloc(sizeof(mpz_t));
                        mpz_init(U[i][0]);
        }
	Rand->PRandM(K, m, 1, U, threadID);
	Rand->PRandInt(K, m, 1, S, threadID);
	ss->modPow(const2m, const2, constm);
 	ss->modMul(S[0], S[0], const2m);
	for(int i = 0; i < size; i++)
		ss->modAdd(C[i], C[i], S[0]);
	for(int i = 0; i < size; i++)
		ss->modAdd(C[i], U[m][0], sum[i]);
	net.broadcastToPeers(C, size, resultShares, threadID);
	ss->reconstructSecret(c,resultShares, size,true);
	for(int i = 0; i < size; i++){
		binarySplit(c[i], bitm, m);
		mpz_set_ui(sum[i], 0); 
		for(int j = 0; j < m; j++){
			ss->modAdd(temp1,bitm[j],U[j][0]);
			ss->modMul(temp2,bitm[j],U[j][0]);
			ss->modMul(temp2,const2,temp2);
			ss->modSub(temp1,temp1,temp2);
			ss->modAdd(sum[i],sum[i],temp1);
		}
	}

	for(int i = 0; i < m+2; i++){
                        mpz_clear(U[i][0]);
                        free(U[i]);
        }
        free(U);
	/************ EQZ (PART 3): evaluate symmetric function  *************/
	mpz_t** T = (mpz_t**)malloc(sizeof(mpz_t*) * m);
	for(int i=0; i<m; ++i){
                        T[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
                        for(int j=0; j<size; ++j){
                                mpz_init(T[i][j]);
                        }
        }
        for(int i = 0; i < m; i++)
                ss->copy(sum, T[i], size);
	PreMul->doOperation(T, T, m, size, threadID);
	for(int i = 0; i < size; i++){
		mpz_set(temp2, coef[m]);
		mpz_set_ui(result[i], 0); 
		ss->modAdd(result[i],result[i], temp2);
		for(int j = 0; j < m; j++){
			mpz_set(temp2,coef[m-j-1]);
			ss->modMul(temp1,T[j][i],temp2);
			ss->modAdd(result[i], result[i], temp1);
		}
		ss->modSub(result[i],const1,result[i]);
	}

	/*Free the memory*/
	mpz_clear(temp1);
	mpz_clear(temp2);
	mpz_clear(const2); 
	mpz_clear(const2K); 
	mpz_clear(const2m); 
	mpz_clear(constK); 
	mpz_clear(const1); 
	
	for(int i=0; i<size; ++i){
		mpz_clear(C[i]);
		mpz_clear(c[i]);
		mpz_clear(sum[i]);
		mpz_clear(S[i]);
	}
	free(C); 
	free(c); 
	free(sum); 
	free(S); 

	for(int i = 0; i < K; i++)
		mpz_clear(bitK[i]); 
	free(bitK); 
	
	for(int i = 0; i < m; i++)
		mpz_clear(bitm[i]); 
	free(bitm); 

	for(int i=0; i<peers; ++i){
		for(int j=0; j<size; ++j)
			mpz_clear(resultShares[i][j]);
		free(resultShares[i]); 
	}
	free(resultShares); 
	
	for(int i=0; i<m; ++i){
		for(int j=0; j<size; ++j){
			mpz_clear(T[i][j]);
		}
		free(T[i]); 
	}
	free(T); 
}
