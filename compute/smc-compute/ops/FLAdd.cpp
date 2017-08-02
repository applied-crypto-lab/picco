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
#include "FLAdd.h"


FLAdd::FLAdd(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare* s, mpz_t coeficients[]) {
    
	Mul = new Mult(nodeNet, nodeID, s);
	Lt = new LTZ(nodeNet, poly, nodeID, s, coeficients);
	T = new Trunc(nodeNet, poly, nodeID, s, coeficients);
	Eq = new EQZ(nodeNet, poly, nodeID, s, coeficients);
	Bd = new BitDec(nodeNet, poly, nodeID, s, coeficients); 
	Preor = new PreOr(nodeNet, poly, nodeID, s, coeficients); 
	P2 = new Pow2(nodeNet, poly, nodeID, s, coeficients); 
	In = new Inv(nodeNet, poly, nodeID, s, coeficients); 

	net = nodeNet;
	id = nodeID;
	ss = s; 
	
}

FLAdd::~FLAdd() {
	// TODO Auto-generated destructor stub
}
/**************************/
//notice that in this implementation K is the same as "L" specified in the paper and vice versa. 
/**************************/

/*A[0](B[0]) contains a value V*/
/*A[1](B[1]) contains a power P*/
/*A[2](B[2]) contains a zero bit Z*/
/*A[3](B[3]) contains a sign bit S*/
void FLAdd::doOperation(mpz_t** A2, mpz_t** B1, mpz_t** result1, int K, int L, int size, int threadID){
	mpz_t** A = (mpz_t**)malloc(sizeof(mpz_t*)  * 4); 
	mpz_t** B = (mpz_t**)malloc(sizeof(mpz_t*)  * 4); 
	mpz_t** result = (mpz_t**)malloc(sizeof(mpz_t*)  * 4); 
	
	for(int i = 0; i < 4; i++){
		A[i] = (mpz_t*)malloc(sizeof(mpz_t) * size); 
		B[i] = (mpz_t*)malloc(sizeof(mpz_t) * size); 
		result[i] = (mpz_t*)malloc(sizeof(mpz_t) * size); 
		for(int j = 0; j < size; j++){
			mpz_init_set(A[i][j], A2[j][i]); 
			mpz_init_set(B[i][j], B1[j][i]); 
			mpz_init_set(result[i][j], result1[j][i]); 
		}
	}


	mpz_t const1, const2, constK, constP2K;
	mpz_t* X = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* E = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* Z = (mpz_t*)malloc(sizeof(mpz_t) * size);

	mpz_t* AA = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* temp1 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* temp2 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* temp3 = (mpz_t*)malloc(sizeof(mpz_t) * size);

	mpz_t* Pmax = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* Pmin = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* Vmin = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* V3 = (mpz_t*)malloc(sizeof(mpz_t) * size);


	mpz_t* av1 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* av2 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* cv1 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* cv2 = (mpz_t*)malloc(sizeof(mpz_t) * size);

	mpz_t* Vmax = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* Y = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* V = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* S3 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* P = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* P0 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* P1 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* pow2 = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* S = (mpz_t*)malloc(sizeof(mpz_t) * size);
	
	mpz_t** H = (mpz_t**)malloc(sizeof(mpz_t*) * (K+3));
	mpz_t* pow2K = (mpz_t*)malloc(sizeof(mpz_t) * (K+2));  
	
	//initialization
	for(int i = 0; i < K+3; i++)
	{
		H[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
		for(int j = 0; j < size; j++)
			mpz_init(H[i][j]);
	}

	for(int i = 0; i < size; i++){
		mpz_init(X[i]);
		mpz_init(E[i]);
		mpz_init(Z[i]);
		mpz_init(AA[i]);

		mpz_init(Pmax[i]);
		mpz_init(Pmin[i]);
		mpz_init(Vmin[i]);
		mpz_init(Vmax[i]);
		mpz_init(V3[i]);

		mpz_init(av1[i]);
		mpz_init(av2[i]);
		mpz_init(cv1[i]);
		mpz_init(cv2[i]);

		mpz_init(P[i]);
		mpz_init(P0[i]);
		mpz_init(P1[i]);

		mpz_init(pow2[i]);
		mpz_init(S[i]);
		mpz_init(S3[i]);

		mpz_init(temp1[i]);
		mpz_init(temp2[i]);
		mpz_init(temp3[i]);

		mpz_init(Y[i]);
		mpz_init(V[i]);

	}
	
	mpz_init_set_ui(const1, 1); 
	mpz_init_set_ui(const2, 2); 
	mpz_init_set_ui(constK, K); 
	mpz_init_set_ui(constP2K, pow(2, K)); 

	for(int i = 0; i < K+2; i++){
		mpz_init(pow2K[i]); 
		ss->modPow(pow2K[i], const2, i);
	}

	//start computation
	//long X = LTZ(ss->modSub(A[1], B[1]), 31);
	ss->modSub(temp1, A[1], B[1], size);
	Lt->doOperation(X, temp1, L, size, threadID);
	

	//long E = EQZ(ss->modSub(A[1], B[1]), 31);
	ss->modSub(temp1, A[1], B[1], size);
	Eq->doOperation(temp1, E, L, size, threadID);

	//long AA = LTZ(ss->modSub(A[0], B[0]), 31);
	ss->modSub(temp1, A[0], B[0], size);
	Lt->doOperation(AA, temp1, K, size, threadID);
	
	/*compute the higher and smaller power out of two*/
	Mul->doOperation(temp3, X, B[1], size, threadID);
	ss->modSub(temp1, const1, X, size);
	Mul->doOperation(temp2, temp1, A[1], size, threadID);
	ss->modAdd(Pmax, temp3, temp2, size);

	ss->modSub(temp1, B[1], temp3, size);
	ss->modAdd(Pmin, Pmin, temp1, size);
	ss->modSub(Pmin, Pmin, temp2, size);
	ss->modAdd(Pmin, Pmin, A[1], size);

	/*compute the larger and smaller value out of two*/
	Mul->doOperation(av1, X, A[0], size, threadID);
	Mul->doOperation(av2, X, B[0], size, threadID);
	Mul->doOperation(cv1, AA, A[0], size, threadID);
	Mul->doOperation(cv2, AA, B[0], size, threadID);
	//compute Vmax_p1
	ss->modSub(temp1, A[0], av1, size);
	ss->modAdd(temp1, temp1, av2, size);
	ss->modSub(temp2, const1, E, size);
	Mul->doOperation(temp3, temp2, temp1, size, threadID);
	ss->modAdd(Vmax, Vmax, temp3, size); 
	//compute Vmax_p2
	ss->modSub(temp1, A[0], cv1, size);
	ss->modAdd(temp2, temp1, cv2, size);
	Mul->doOperation(temp3, E, temp2, size, threadID);
	//compute Vmax
	ss->modAdd(Vmax, Vmax, temp3, size);

	//compute Vmin_p1
	ss->modSub(temp1, B[0], av2, size);
	ss->modAdd(temp1, temp1, av1, size);
	ss->modSub(temp2, const1, E, size);
	Mul->doOperation(temp3, temp2, temp1, size, threadID);
	ss->modAdd(Vmin, Vmin, temp3, size); 
	//compute Vmin_p2
	ss->modSub(temp1, B[0], cv2, size);
	ss->modAdd(temp2, temp1, cv1, size);
	Mul->doOperation(temp3, E, temp2, size, threadID);
	ss->modAdd(Vmin, Vmin, temp3, size);
	
	//line 8 
	ss->modAdd(temp1, A[3], B[3], size);
	Mul->doOperation(temp2, A[3], B[3], size, threadID);
	ss->modMul(temp2, temp2, const2, size);
	ss->modSub(S3, temp1, temp2, size);

	//line 9 
	ss->modSub(temp1, Pmax, Pmin, size);
	ss->modSub(temp2, constK, temp1, size);
	Lt->doOperation(Y, temp2, L, size, threadID);
	
	//line 10 
	ss->modSub(temp1, const1, Y, size);
	ss->modSub(temp2, Pmax, Pmin, size);
	Mul->doOperation(temp3, temp1, temp2, size, threadID);
	P2->doOperation(pow2, temp3, K+1, size, threadID);
	
	//line 11
	ss->modSub(temp1, Vmax, S3, size);
	ss->modMul(temp1, temp1, const2, size);
	ss->modAdd(V3, temp1, const1, size);
	
	//line 12
	Mul->doOperation(temp1, Vmax, pow2, size, threadID);
	ss->modMul(temp2, S3, const2, size);
	ss->modSub(temp2, const1, temp2, size);
	Mul->doOperation(temp3, temp2, Vmin, size, threadID);
	ss->modAdd(temp3, temp3, temp1, size);
	
	//line 13 and 14 
	Mul->doOperation(temp1, Y, V3, size, threadID);
	ss->modSub(temp2, const1, Y, size);
	Mul->doOperation(temp3, temp2, temp3, size, threadID);
	ss->modAdd(temp3, temp1, temp3, size);
	ss->modMul(temp3, temp3, constP2K, size);
	In->doOperation(pow2, temp1, size, threadID);
	Mul->doOperation(temp2, temp3, temp1, size, threadID);
	T->doOperation(V, temp2, 2*K+1, K-1, size, threadID);
	
	//line 15 and 16
	Bd->doOperation(H, V, K+2, K+2, size, threadID);
	mpz_t tmp; 
	mpz_init(tmp); 

	for(int i = 0; i < K+2 && i <= K+1-i; i++)
		for(int j = 0; j < size; j++){
			mpz_set(tmp, H[i][j]); 
			mpz_set(H[i][j], H[K+1-i][j]); 
			mpz_set(H[K+1-i][j], tmp); 
			//ss->copy(U[i], H[K+1-i], size);
		}

	Preor->doOperation(H, H, K+2, size, threadID);
	
	//line 17
	for(int i = 0; i < size; i++)
		ss->modAdd(temp1[i], const2, constK);  
	for(int i = 0; i < size; i++)
		mpz_set_ui(temp2[i], 0);
	for(int i = 0; i < K+2; i++)
		ss->modAdd(temp2, temp2, H[i], size);
	ss->modSub(P0, temp1, temp2, size);

	//line 18
	for(int i = 0; i < K+2; i++){
		for(int j = 0; j < size; j++)
			mpz_set(temp1[j], pow2K[i]); 
		ss->modSub(temp2, const1, H[i], size);
		ss->modMul(temp3, temp1, temp2, size);
		ss->modAdd(P1, P1, temp3, size);
	}
	ss->modAdd(P1, P1, const1, size);
	
	//line 19 
	Mul->doOperation(temp1, P1, V, size, threadID);
	T->doOperation(V, temp1, K+2, 2, size, threadID);

	//line 20 
	ss->modSub(temp1, Pmax, P0, size);
	ss->modAdd(P, temp1, const1, size);
	ss->modSub(P, P, Y, size);

	//line 21 and 22 
	Mul->doOperation(temp1, B[2], A[0], size, threadID);
	Mul->doOperation(temp2, A[2], B[0], size, threadID);
	ss->modAdd(temp1, temp1, temp2, size);
	ss->modSub(temp2, const1, B[2], size);
	ss->modSub(temp3, const1, A[2], size);
	Mul->doOperation(temp2, temp2, temp3, size, threadID);
	Mul->doOperation(temp2, temp2, V, size, threadID);
	ss->modAdd(V, temp2, temp1, size);
	Eq->doOperation(V, Z, K, size, threadID);
	//line 23
	
	ss->modSub(temp1, const1, B[2], size);
	ss->modSub(temp2, const1, A[2], size);
	Mul->doOperation(temp3, temp1, temp2, size, threadID);
	Mul->doOperation(temp3, P, temp3, size, threadID);
	
	Mul->doOperation(temp1, B[2], A[1], size, threadID);
	Mul->doOperation(temp2, A[2], B[1], size, threadID);
	ss->modAdd(temp2, temp1, temp2, size);
	
	ss->modSub(temp1, const1, Z, size);
	ss->modAdd(temp2, temp3, temp2, size);
	Mul->doOperation(P, temp1, temp2, size, threadID);
	//line 24
	ss->modSub(temp1, const1, X, size);
	Mul->doOperation(temp1, temp1, A[3], size, threadID);
	Mul->doOperation(temp2, X, B[3], size, threadID);
	ss->modAdd(temp2, temp2, temp1, size);
	ss->modSub(temp1, const1, E, size);
	Mul->doOperation(temp3, temp1, temp2, size, threadID);	
	ss->modAdd(S, S, temp3, size); 
	
	ss->modSub(temp1, const1, AA, size);
	Mul->doOperation(temp1, temp1, A[3], size, threadID);
	Mul->doOperation(temp2, AA, B[3], size, threadID);
	ss->modAdd(temp2, temp2, temp1, size);
	Mul->doOperation(temp3, E, temp2, size, threadID);
	ss->modAdd(S, S, temp3, size);
	// line 25
	ss->modSub(temp1, const1, B[2], size);
	ss->modSub(temp2, const1, A[2], size);
	Mul->doOperation(temp1, temp1, temp2, size, threadID);
	Mul->doOperation(S, S, temp1, size, threadID);
		
	ss->modSub(temp1, const1, A[2], size);
	Mul->doOperation(temp2, temp1, B[2], size, threadID);
	Mul->doOperation(temp3, A[3], temp2, size, threadID);
	ss->modAdd(S, S, temp3, size); 

	ss->modSub(temp1, const1, B[2], size);
	Mul->doOperation(temp2, temp1, A[2], size, threadID);
	Mul->doOperation(temp3, A[3], temp2, size, threadID);
	ss->modAdd(S, S, temp3, size); 


	ss->copy(V, result[0], size);
	ss->copy(P, result[1], size);
	ss->copy(Z, result[2], size);
	ss->copy(S, result[3], size);

	
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < size; j++)
			mpz_set(result1[j][i], result[i][j]); 
	
	
	//free the memory
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < size; j++){
			mpz_clear(A[i][j]); 
			mpz_clear(B[i][j]); 
			mpz_clear(result[i][j]); 
		}
		free(A[i]); 
		free(B[i]); 
		free(result[i]); 
	}
	free(A); 
	free(B); 
	free(result); 


	for(int i = 0; i < K+3; i++)
	{
		for(int j = 0; j < size; j++)
			mpz_clear(H[i][j]);
		free(H[i]); 
	}
	free(H); 

	for(int i = 0; i < size; i++){
		mpz_clear(X[i]);
		mpz_clear(E[i]);
		mpz_clear(Z[i]);
		mpz_clear(AA[i]);

		mpz_clear(Pmax[i]);
		mpz_clear(Pmin[i]);
		mpz_clear(Vmin[i]);
		mpz_clear(Vmax[i]);
		mpz_clear(V3[i]);

		mpz_clear(av1[i]);
		mpz_clear(av2[i]);
		mpz_clear(cv1[i]);
		mpz_clear(cv2[i]);

		mpz_clear(P[i]);
		mpz_clear(P0[i]);
		mpz_clear(P1[i]);

		mpz_clear(pow2[i]);
		mpz_clear(S[i]);

		mpz_clear(S3[i]);

		mpz_clear(temp1[i]);
		mpz_clear(temp2[i]);
		mpz_clear(temp3[i]);
		mpz_clear(Y[i]);
		mpz_clear(V[i]);
	}
	
	free(X);
	free(E);
	free(Z);
	free(AA);

	free(Pmax);
	free(Pmin);
	free(Vmin);
	free(Vmax);
	free(V3);

	free(av1);
	free(av2);
	free(cv1);
	free(cv2);

	free(P);
	free(P0);
	free(P1);
	free(pow2);
	free(S);
	free(S3);

	free(temp1);
	free(temp2);
	free(temp3);
	free(Y);
	free(V);

	mpz_clear(const1); 
	mpz_clear(const2); 
	mpz_clear(constK); 
	mpz_clear(constP2K); 
	
	for(int i = 0; i < K+2; i++)
		mpz_clear(pow2K[i]); 
	free(pow2K); 
}
