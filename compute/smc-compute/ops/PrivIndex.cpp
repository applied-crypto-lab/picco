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
#include "PrivIndex.h"


PrivIndex::PrivIndex(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare *s, mpz_t coefficients[]) {
    
	Mul = new Mult(nodeNet, nodeID, s); 
	Bd = new BitDec(nodeNet, poly, nodeID, s, coefficients);
    	Eq = new EQZ(nodeNet, poly, nodeID, s, coefficients);
	net = nodeNet;
	id = nodeID;
	ss = s;
}

PrivIndex::~PrivIndex() {
	// TODO Auto-generated destructor stub
}

void PrivIndex::compute_private_conditions(mpz_t* private_conditions, mpz_t out_cond, mpz_t* priv_cond, int counter, int size)
{
        if(out_cond != NULL && counter == -1 && priv_cond == NULL){
                for(int i = 0; i < size; i++)
                        mpz_set(private_conditions[i], out_cond);
	}
        else if(out_cond == NULL && counter != -1 && priv_cond != NULL){
                for(int i = 0; i < size; i++)
                        if(counter != size)
                                mpz_set(private_conditions[i], priv_cond[i/(size/counter)]);
                        else
                                mpz_set(private_conditions[i], priv_cond[i]);
        }else if(out_cond == NULL && priv_cond == NULL){
		for(int i = 0; i < size; i++)
			mpz_set_ui(private_conditions[i], 1); 
	}
}

void PrivIndex::doOperationRead(mpz_t* index, mpz_t* array, mpz_t* result, int dim, int size, int threadID, int type){
	int K = ceil(log2(dim));
	int m = (type == 0) ? 1 : 4;
	
	mpz_t** U = (mpz_t**)malloc(sizeof(mpz_t*) * (K+1));
	mpz_t** U1 = (mpz_t**)malloc(sizeof(mpz_t*) * size);
	mpz_t* temp1 = (mpz_t*)malloc(sizeof(mpz_t) * m * size * dim);
	mpz_t* temp2 = (mpz_t*)malloc(sizeof(mpz_t) * m * size * dim);
	mpz_t* temp3 = (mpz_t*)malloc(sizeof(mpz_t) * m * size * dim);
	mpz_t const1;
	mpz_init_set_ui(const1, 1);
	int** bitArray = (int**)malloc(sizeof(int*) * dim);
	int ind, val;
	
	//initialization
	for(int i = 0; i < dim; i++)
		bitArray[i] = (int*)malloc(sizeof(int) * K);
	for(int i = 0; i < m*size*dim; i++){
		mpz_init(temp1[i]);
		mpz_init(temp2[i]);
		mpz_init(temp3[i]);
	}
	for(int i = 0; i < K+1; i++){
		U[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
		for(int j = 0; j < size; j++)
			mpz_init(U[i][j]);
	}
	for(int i = 0; i < size; i++){
		U1[i] = (mpz_t*)malloc(sizeof(mpz_t) * (K+1));
		for(int j = 0; j < K+1; j++)
			mpz_init(U1[i][j]);
	}
	for(int i = 0; i < m * size; i++)
		mpz_init_set_ui(result[i], 0); 
	
	//start computation
	for(int i = 0; i < dim; i++)
		binarySplit(i, bitArray[i], K);
	Bd->doOperation(U, index, K, K, size, threadID);
	for(int i = 0; i < size; i++)
		for(int j = 0; j < K+1; j++)
			mpz_set(U1[i][j], U[j][i]);
    
	for(int i = 0; i < size; i++)
		for(int j = 0; j < dim; j++)
			mpz_set(temp1[i*dim+j], const1);
    
	for(int i = 0; i < K; i++){
		for(int j = 0; j < size; j++){
			for(int k = 0; k < dim; k++){
				if(bitArray[k][i] == 0)
					ss->modSub(temp2[j*dim+k], const1, U1[j][i]);
				else
					mpz_set(temp2[j*dim+k], U1[j][i]);
			}
		}
		Mul->doOperation(temp1, temp1, temp2, size*dim, threadID);
	}
	
	for(int i = 0; i < size; i++){
		for(int j = 0; j < dim; j++){
			for(int k = 0; k < m; k++){
				mpz_set(temp2[i*dim*m+j*m+k], array[j*m+k]);
				mpz_set(temp3[i*dim*m+j*m+k], temp1[i*dim+j]);
			}
		}
	}
    	
	Mul->doOperation(temp3, temp2, temp3, m*size*dim, threadID);
	
	for(int i = 0; i < size; i++)
		for(int j = 0; j < m; j++)
			for(int k = 0; k < dim; k++)
				ss->modAdd(result[i*m+j], result[i*m+j], temp3[i*m*dim+k*m+j]);
	//free the memory
	for(int i = 0; i < dim; i++)
		free(bitArray[i]); 
	free(bitArray); 
	for(int i = 0; i < m*size*dim; i++)
	{
		mpz_clear(temp1[i]); 
		mpz_clear(temp2[i]);
		mpz_clear(temp3[i]); 
	}
	free(temp1); 
	free(temp2); 
	free(temp3); 
	for(int i = 0; i < K+1; i++){
		for(int j = 0; j < size; j++)
			mpz_clear(U[i][j]);
		free(U[i]); 
	}
 	free(U); 
	for(int i = 0; i < size; i++){
		for(int j = 0; j < K+1; j++)
			mpz_clear(U1[i][j]);
		free(U1[i]);  
	}
	free(U1); 
	mpz_clear(const1); 
}

void PrivIndex::doOperationWrite(mpz_t* index, mpz_t* array, mpz_t* value, int dim, int size, mpz_t out_cond, mpz_t* priv_cond, int counter, int threadID, int type){
        int K = ceil(log2(dim));
	int m = (type == 0) ? 1 : 4; 
        mpz_t** U = (mpz_t**)malloc(sizeof(mpz_t*) * (K+1));
        mpz_t** U1 = (mpz_t**)malloc(sizeof(mpz_t*) * size);
        mpz_t* temp1 = (mpz_t*)malloc(sizeof(mpz_t) * m * size * dim);
        mpz_t* temp2 = (mpz_t*)malloc(sizeof(mpz_t) * m * size * dim);
        mpz_t* temp3 = (mpz_t*)malloc(sizeof(mpz_t) * m * size * dim);
        mpz_t* temp4 = (mpz_t*)malloc(sizeof(mpz_t) * m * dim); 
        mpz_t* temp5 = (mpz_t*)malloc(sizeof(mpz_t) * m * dim); 
	mpz_t const1;
        mpz_init_set_ui(const1, 1);
	
	//initialization
        int** bitArray = (int**)malloc(sizeof(int*) * dim);
        int ind, val;
        for(int i = 0; i < dim; i++)
                bitArray[i] = (int*)malloc(sizeof(int) * K);
        for(int i = 0; i < m*size*dim; i++){
                mpz_init(temp1[i]);
                mpz_init(temp2[i]);
                mpz_init(temp3[i]);
        }
	for(int i = 0; i < m*dim; i++){
		mpz_init(temp4[i]); 
		mpz_init(temp5[i]); 
	}
        for(int i = 0; i < K+1; i++){
                U[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
                for(int j = 0; j < size; j++)
                        mpz_init(U[i][j]);
        }
        for(int i = 0; i < size; i++){
                U1[i] = (mpz_t*)malloc(sizeof(mpz_t) * (K+1));
                for(int j = 0; j < K+1; j++)
                	mpz_init(U1[i][j]);
        }
	
	//start computation
        compute_private_conditions(temp3, out_cond, priv_cond, counter, size);
        for(int i = 0; i < dim; i++)
                binarySplit(i, bitArray[i], K);
        Bd->doOperation(U, index, K, K, size, threadID);
        for(int i = 0; i < size; i++)
                for(int j = 0; j < K+1; j++)
                        mpz_set(U1[i][j], U[j][i]);
        for(int i = 0; i < size; i++){
                for(int j = 0; j < dim; j++){
                        mpz_set(temp1[i*dim+j], const1);
                        mpz_set(temp2[i*dim+j], temp3[i]);
                }
	}
        for(int i = 0; i < K; i++){
                for(int j = 0; j < size; j++){
                        for(int k = 0; k < dim; k++){
                                if(bitArray[k][i] == 0)
                                        ss->modSub(temp3[j*dim+k], const1, U1[j][i]);
                                else
                                        mpz_set(temp3[j*dim+k], U1[j][i]);
			}
		}
                Mul->doOperation(temp1, temp1, temp3, size*dim, threadID);
        }
        Mul->doOperation(temp1, temp1, temp2, size*dim, threadID);
        for(int i = 0; i < size; i++){
                for(int j = 0; j < dim; j++){
                        for(int k = 0; k < m; k++){
                                mpz_set(temp2[i*dim*m+j*m+k], value[i*m+k]);
                                mpz_set(temp3[i*dim*m+j*m+k], temp1[i*dim+j]);
                        }
		}
	}
        Mul->doOperation(temp2, temp2, temp3, m*size*dim, threadID);
	for(int i = 0; i < size; i++){
		for(int j = 0; j < dim; j++){
			for(int k = 0; k < m; k++){
				ss->modAdd(temp4[j*m+k], temp4[j*m+k], temp2[i*dim*m+j*m+k]); 
				ss->modAdd(temp5[j*m+k], temp5[j*m+k], temp1[i*dim+j]);   
			}
		}
	}
	ss->modSub(temp5, const1, temp5, m*dim); 
	Mul->doOperation(temp5, temp5, array, m*dim, threadID); 
	ss->modAdd(array, temp4, temp5, m*dim); 
	
	//free memory
        for(int i = 0; i < dim; i++)
                free(bitArray[i]);
        free(bitArray);
	for(int i = 0; i < m*size*dim; i++){
                mpz_clear(temp1[i]);
                mpz_clear(temp2[i]);
                mpz_clear(temp3[i]);
        }
	for(int i = 0; i < m*dim; i++){
		mpz_clear(temp4[i]); 
		mpz_clear(temp5[i]); 
	}
        free(temp1);
        free(temp2);
        free(temp3);
        free(temp4);
        free(temp5); 
	for(int i = 0; i < K+1; i++){
                for(int j = 0; j < size; j++)
                        mpz_clear(U[i][j]);
                free(U[i]);
        }
        free(U);

        for(int i = 0; i < size; i++){
                for(int j = 0; j < K+1; j++)
                        mpz_clear(U1[i][j]);
                free(U1[i]);
        }
        free(U1);
        mpz_clear(const1);
}

