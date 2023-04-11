/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame
   ** Department of Computer Science and Engineering, University of Buffalo (SUNY)

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

#include "SecretShare.h"
#include "stdint.h"
#include <vector>
#include <math.h>
#include <iostream>
#include <cstdlib>
#include <gmp.h>
#include <cstring>

SecretShare::SecretShare(int p, int t, mpz_t mod){
	peers = p;
	threshold = t;
	mpz_init(fieldSize);
	mpz_set(fieldSize, mod);
	computeSharingMatrix();
	computeLagrangeWeight();
	gmp_randinit_mt(rstate); 
}

int SecretShare::getPeers(){
	return peers; 
}

void SecretShare::getFieldSize(mpz_t field){
	mpz_set(field, fieldSize); 
}

int SecretShare::getThreshold(){
	return threshold; 
}

/* the function creates n=peers shares of the second argument */
void SecretShare::getShares(mpz_t* shares, mpz_t secret) {

	mpz_t coefficient;
	mpz_init(coefficient);
	mpz_t temp;
	mpz_init(temp);
	mpz_set_ui(temp, 0);
	mpz_t random;
	mpz_init(random);
    
	for (int i = 0; i < peers; i++)
	  mpz_set_ui(shares[i], 0);
	
	if (mpz_cmp_si(secret, 0) < 0)
	  mpz_mod(secret, secret, fieldSize);

	for (int degree = 0; degree < threshold+1; degree++) {
	  // set the free coefficient to the secret; otherwise generate a
	  // random field element
	  if (degree == 0)
	    mpz_set(coefficient, secret);
	  else {
	    mpz_urandomm(coefficient, rstate, fieldSize);

	    //the coefficient of the highest degree has to be non-zer to
	    // guarantee that this is a polynomial of degree t
	    if(degree == threshold && mpz_sgn(coefficient) == 0)
	      mpz_add_ui(coefficient, coefficient, 1); 
	  }

	  // add the contribution of the current coefficient to each share
	  for (int peer = 0; peer < peers; peer++){
	    modMul(temp, sharingMatrix[peer][degree], coefficient);
	    modAdd(shares[peer],shares[peer], temp);
	  }
	}
}

/* 
 * this function is similar to the above, but generates shares for a number of
 *  secrets specified by the last argument 
 */
void SecretShare::getShares(mpz_t** shares, mpz_t* secrets, int size) {
  
	mpz_t coefficient;
	mpz_init(coefficient);
	mpz_t temp;
	mpz_init_set_ui(temp, 0);
	int degree, peer;
	
	for (int i = 0; i < size; i++) {
	  for(degree = 0; degree < threshold+1; degree++) {
	    // generate coefficients one at a time
	    if(degree == 0)
	      mpz_set(coefficient,secrets[i]);
	    else {
	      mpz_urandomm(coefficient, rstate, fieldSize);

	      // the most significant coefficient must be non-zero
	      if(degree == threshold && mpz_sgn(coefficient) == 0)
		mpz_add_ui(coefficient, coefficient, 1); 
	    }

	    // incorporate the coefficient into the shares
	    for (peer = 0; peer < peers; peer++) {
	      modMul(temp, sharingMatrix[peer][degree], coefficient);
	      modAdd(shares[peer][i],shares[peer][i], temp);
	    }
	  }
	}
	mpz_clear(coefficient); 
	mpz_clear(temp); 
}

void SecretShare::modMul(mpz_t result, mpz_t x, mpz_t y){
	mpz_mul(result,x,y);
	mpz_mod(result,result,fieldSize);

}

void SecretShare::modMul(mpz_t* result, mpz_t* x, mpz_t* y, int size){
	for(int i = 0; i < size; i++)
		modMul(result[i],x[i],y[i]);
}

void SecretShare::modMul(mpz_t result, mpz_t x, long y){
	mpz_mul_si(result,x,y);
	mpz_mod(result,result,fieldSize);
}

void SecretShare::modMul(mpz_t* result, mpz_t* x, long y, int size){
	for(int i=0; i<size; ++i){
		modMul(result[i],x[i],y);
	}
}

void SecretShare::modMul(mpz_t* result, mpz_t* x, mpz_t y, int size){
	for(int i=0; i<size; ++i){
		modMul(result[i],x[i],y);
	}
}

void SecretShare::modAdd(mpz_t result, mpz_t x, mpz_t y){
	mpz_add(result,x,y);
	mpz_mod(result,result,fieldSize);
}

void SecretShare::modAdd(mpz_t* result, mpz_t* x, mpz_t* y, int size){
	for(int i = 0; i < size; i++)
		modAdd(result[i],x[i],y[i]);
}

void SecretShare::modAdd(mpz_t result, mpz_t x, long y){
	mpz_t y1;
	mpz_init_set_si(y1, y);
	mpz_add(result, x, y1);
	mpz_mod(result, result, fieldSize);
	mpz_clear(y1); 
}

void SecretShare::modAdd(mpz_t* result, mpz_t*x, long y, int size){
	for(int i=0; i<size; ++i)
		modAdd(result[i], x[i], y);
}

void SecretShare::modAdd(mpz_t* result, mpz_t* x, mpz_t y, int size){
	for(int i = 0; i < size; i++)
		modAdd(result[i], x[i], y); 
}

void SecretShare::modSub(mpz_t result, mpz_t x, mpz_t y){
	mpz_sub(result,x,y);
	mpz_mod(result,result,fieldSize);
}

void SecretShare::modSub(mpz_t* result, mpz_t* x, mpz_t* y, int size){
	for(int i = 0; i < size; i++)
		modSub(result[i],x[i],y[i]);
}

void SecretShare::modSub(mpz_t result, mpz_t x, long y){
	mpz_t y1; 
	mpz_init_set_si(y1, y); 
	mpz_sub(result, x, y1);
	mpz_mod(result, result, fieldSize);
	mpz_clear(y1); 
}

void SecretShare::modSub(mpz_t result, long x, mpz_t y){
	mpz_t x1; 
	mpz_init_set_si(x1, x); 
	mpz_sub(result, x1, y);
	mpz_mod(result, result, fieldSize);
	mpz_clear(x1); 
}

void SecretShare::modSub(mpz_t* result, mpz_t* x, long y, int size){
	for(int i=0; i<size; ++i)
		modSub(result[i],x[i],y);
}

void SecretShare::modSub(mpz_t* result, long x, mpz_t* y, int size){
	for(int i=0; i<size; ++i)
		modSub(result[i],x,y[i]);
}

void SecretShare::modSub(mpz_t* result, mpz_t* x, mpz_t y, int size){
	for(int i=0; i<size; ++i)
		modSub(result[i],x[i],y);
}

void SecretShare::modSub(mpz_t* result, mpz_t x, mpz_t* y, int size){
	for(int i=0; i<size; ++i)
		modSub(result[i],x,y[i]);
}

void SecretShare::modPow(mpz_t result, mpz_t base, mpz_t exponent){
	mpz_powm(result,base,exponent,fieldSize);
}

void SecretShare::modPow(mpz_t* result, mpz_t* base, mpz_t* exponent, int size){
	for(int i = 0; i < size; i++)
		mpz_powm(result[i],base[i],exponent[i],fieldSize);
}

void SecretShare::modPow(mpz_t result, mpz_t base, long exponent){
	mpz_t value; 
	mpz_init_set_si(value, exponent); 
	modAdd(value, value, (long)0); 
	mpz_powm(result, base, value, fieldSize);
	mpz_clear(value); 
}

void SecretShare::modPow(mpz_t* result, mpz_t* base, long exponent, int size){
	for(int i=0; i<size; ++i)
		modPow(result[i],base[i],exponent);
}

void SecretShare::modInv(mpz_t result, mpz_t value){
	mpz_t temp;
	mpz_init(temp);
	mpz_sub_ui(temp, fieldSize, 2);
	modPow(result, value, temp);
	mpz_clear(temp); 
}

void SecretShare::modInv(mpz_t* result, mpz_t* values, int size){
	for(int i = 0; i < size; i++)
		modInv(result[i],values[i]);
}


void SecretShare::modSqrt(mpz_t result, mpz_t x){
	mpz_t temp;
	mpz_init(temp);
	mpz_add_ui(temp, fieldSize, 1);
	mpz_div_ui(temp, temp, 4);
	modPow(result, x, temp);
	mpz_clear(temp); 
}

void SecretShare::modSqrt(mpz_t* result, mpz_t* x, int size){
	mpz_t* power = (mpz_t*)malloc(sizeof(mpz_t) * size);
	for(int i = 0; i < size; i++){
		mpz_init(power[i]);
		mpz_add_ui(power[i], fieldSize, 1);
		mpz_div_ui(power[i], power[i], 4);
	}
	modPow(result, x, power, size);
	for(int i = 0; i < size; i++)
		mpz_clear(power[i]); 
}

void SecretShare::modSum(mpz_t result, mpz_t* x, int size){
	mpz_set_ui(result,0);
	for(int i=0; i<size; ++i){
		mpz_add(result,result,x[i]);
	}
	mpz_mod(result,result,fieldSize);
}

void SecretShare::mod(mpz_t* result, mpz_t* a, mpz_t* m, int size){
	mpz_t tmp; 
	mpz_init(tmp); 
	for(int i=0; i<size; ++i){
		mpz_init_set_ui(tmp, 0); 
		mpz_add(tmp, a[i], m[i]); 
		if(mpz_cmp(tmp, fieldSize) > 0)
			mpz_sub(result[i], tmp, fieldSize);
		else{
			mpz_mod(result[i],a[i],m[i]);
			mpz_mod(result[i],result[i],fieldSize);
		}
	}
}

void SecretShare::mod(mpz_t* result, mpz_t* a, mpz_t m, int size){
        mpz_t tmp;
        mpz_init(tmp);
        for(int i=0; i<size; ++i){
                mpz_init_set_ui(tmp, 0);
                mpz_add(tmp, a[i], m);
                if(mpz_cmp(tmp, fieldSize) > 0)
                        mpz_sub(result[i], tmp, fieldSize);
                else{
                        mpz_mod(result[i],a[i],m);
                        mpz_mod(result[i],result[i],fieldSize);
                }
        }
}

void SecretShare::copy(mpz_t* src, mpz_t* des, int size)
{
	for(int i = 0; i < size; i++)
		mpz_set(des[i], src[i]);
}

/* 
 * this function initializes a two-dimensional array called sharingMatrix. 
 * sharingMatrix is used during computation of shares, i.e., evaluating a 
 * polynomial at different points. It basically stores different powers of 
 * points on which a polynomial will need to be evaluated.
 */
void SecretShare::computeSharingMatrix(){

	mpz_t t1, t2; 
	mpz_init(t1); 
	mpz_init(t2);
	int i;
	
	sharingMatrix = (mpz_t**)malloc(sizeof(mpz_t*) * peers);
	for (i = 0; i < peers; i++)
	  sharingMatrix[i] = (mpz_t*)malloc(sizeof(mpz_t) * peers);
	for (i = 0; i < peers; i++)
	  for (int j = 0; j < peers; j++)
	    mpz_init(sharingMatrix[i][j]);
	
	for (int i = 0; i < peers; i++){
	  for(int j = 0; j < 2*threshold+1; j++){
	    mpz_set_ui(t1,i+1);
	    mpz_set_ui(t2,j);
	    modPow(sharingMatrix[i][j], t1, t2);
	  }
	}
	mpz_clear(t1); 
	mpz_clear(t2);
}

/* 
 * this function pre-computed Largrange coefficients for reconstructing a 
 * secret from n=peer shares, which corresponds to evaluating the polynomial
 * at point 0.
 */
void SecretShare::computeLagrangeWeight(){
	mpz_t nom, denom, t1, t2, temp;
	mpz_init(nom);
	mpz_init(denom);
	mpz_init(t1);
	mpz_init(t2);
	mpz_init(temp);

	lagrangeWeight = (mpz_t*)malloc(sizeof(mpz_t) * peers);
	
	for(int i = 0; i < peers; i++)
		mpz_init(lagrangeWeight[i]);
	
	for(int peer = 0; peer < peers; peer++){
		int point = peer+1;
		mpz_set_ui(nom,1);
		mpz_set_ui(denom,1);

		for(int l = 0; l < peers; l++){
			if(l != peer){
				mpz_set_ui(t1, l+1);
				modMul(nom, nom, t1);
				mpz_set_ui(t2, point);
				modSub(temp, t1, t2);
				modMul(denom, denom, temp);
			}
		}
		modInv(temp, denom);
		modMul(lagrangeWeight[peer], nom, temp);
	}

	mpz_clear(nom); 
	mpz_clear(denom); 
	mpz_clear(t1); 
	mpz_clear(t2); 
	mpz_clear(temp); 
}

/* reconstruction of a secret from n=peers shares */
void SecretShare::reconstructSecret(mpz_t result, mpz_t* y, bool isMultiply){
	mpz_t temp;
	mpz_init(temp);
	mpz_set_ui(result, 0); 
	for(int peer = 0; peer < peers; peer++){
		modMul(temp, y[peer], lagrangeWeight[peer]);
		modAdd(result,result, temp);
	}
	mpz_clear(temp); 
}

/* reconstruction of a number of secrets from n=peers shares each */
void SecretShare::reconstructSecret(mpz_t* result, mpz_t** y, int size, bool isMultiply){
	mpz_t temp;
	mpz_init(temp);
	for(int i = 0; i < size; i++)
		mpz_set_ui(result[i], 0);
	for(int i = 0; i < size; i++){
		for(int peer = 0; peer < peers; peer++){
			modMul(temp, y[peer][i], lagrangeWeight[peer]);
			modAdd(result[i],result[i], temp);
		}
	}
	mpz_clear(temp); 
}

void SecretShare::Seed(unsigned char * key_0,unsigned char * key_1){
 
	gmp_randinit_mt(rstate_0); //m1
	gmp_randinit_mt(rstate_1); //p1	

	mpz_t seed;
	mpz_init(seed);

	mpz_import(seed, 16, 1, sizeof(key_0[0]), 0, 0, key_0);
	gmp_randseed(rstate_0, seed);

	mpz_import(seed, 16, 1, sizeof(key_1[0]), 0, 0, key_1);
	gmp_randseed(rstate_1, seed);
	
	seeded=1;
	mpz_clear(seed);
}

void SecretShare::getShares2(mpz_t* temp, mpz_t* rand, mpz_t** data, int size){

//	printf("threshold is %d and seed %d and myiid %d\n",threshold, seeded, myiid);

	mpz_t coefficient;
	mpz_init(coefficient);
	for(int i=0;i<size;i++)
	{
		mpz_urandomm(rand[i], rstate_1, fieldSize);
	}
	for(int i = 0; i < size; i++){
		mpz_sub(coefficient,rand[i],temp[i]);
		mpz_mul(coefficient,coefficient,id_p1_inv);
		mpz_mul_ui(data[myid-1][i],coefficient,myid);	//for id
		mpz_add(data[myid-1][i],data[myid-1][i],temp[i]);
		mpz_mod(data[myid-1][i],data[myid-1][i],fieldSize);
		mpz_mul_ui(data[id_m1-1][i],coefficient,id_m1);	//for id-1
		mpz_add(data[id_m1-1][i],data[id_m1-1][i],temp[i]);
		mpz_mod(data[id_m1-1][i],data[id_m1-1][i],fieldSize);
	}
	mpz_clear(coefficient);
	for(int i=0;i<size;i++)
	{
		mpz_urandomm(temp[i], rstate_0, fieldSize);
	}
}

void SecretShare::checkSeed(){
 	if(seeded==0)
	{
		printf("not seeded\n");
	}
	else
	{
		printf("seeded\n");
	}
}

void SecretShare::getCoef(int id){
	myid=id;
	id_p1 = (myid+1)%(peers+1);
	if(id_p1 == 0)
		id_p1 = 1;

	id_m1 = (myid-1)%(peers+1);
	if(id_m1 == 0)
		id_m1 = peers;

	mpz_init(id_p1_inv);
	mpz_set_ui(id_p1_inv,id_p1);
	mpz_invert(id_p1_inv,id_p1_inv,fieldSize);
}

