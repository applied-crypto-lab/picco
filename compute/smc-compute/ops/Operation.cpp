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

#include "Operation.h"

Operation::Operation() {
    }

Operation::~Operation() {}

void Operation::clearBuffer(mpz_t** buffer, int firstDimSize, int secondDimSize){
	for(int i = 0; i < firstDimSize; i++)
			for(int j = 0; j < secondDimSize; j++)
				mpz_set_ui(buffer[i][j], 0);
}

void Operation::clearBuffer(mpz_t* buffer, int firstDimSize){
	for(int i = 0; i < firstDimSize; i++)
			mpz_set_ui(buffer[i], 0);
}

void Operation::binarySplit(mpz_t v1, mpz_t* result, int K){
	mpz_t rem;
	mpz_init(rem);
	mpz_t v; 
	mpz_init_set(v, v1); 
	int i = 0;
	do{
		mpz_mod_ui(rem, v, 2);
		mpz_set(result[i], rem);
		mpz_div_ui(v, v, 2);
		i++;
	}while((mpz_cmp_ui(v, 0) > 0) && i < K);
	if(i < K){
		for(int j = i; j < K; j++){
			mpz_set_ui(rem, 0);
			mpz_set(result[j], rem);
		}
	}
	mpz_clear(rem); 
}

void Operation::binarySplit(int v, int* result, int K){
	int rem;
	int i = 0;
	do{
		rem = v % 2; 
		result[i] = rem; 	
		v = v/2; 
		i++;
	}while(v > 0);

	if(i < K)
		for(int j = i; j < K; j++)
			result[j] = 0; 
}


double Operation::time_diff(
struct timeval *t1, struct timeval *t2){
	double elapsed;

	if(t1->tv_usec > t2->tv_usec){
		t2->tv_usec += 1000000;
		t2->tv_sec--;
	}

	elapsed = (t2->tv_sec-t1->tv_sec) + (t2->tv_usec - t1->tv_usec)/1000000.0;

	return(elapsed);
}
