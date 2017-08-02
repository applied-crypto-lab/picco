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

#ifndef SECRETSHARE_H_
#define SECRETSHARE_H_

#include "SecretShare.h"
#include "stdint.h"
#include <vector>
#include <math.h>
#include <iostream>
#include <cstdlib>
#include <gmp.h>

class SecretShare{

public:
	SecretShare(int, int, mpz_t);

	//Set the number of peers
	void setPeers(int p);
	int getPeers(); 
	int getThreshold(); 
	void getFieldSize(mpz_t); 
	//Set the privacy threshold
	void setThreshold(int t);

	//Divide a secret into n shares
	void getShares(mpz_t*, mpz_t);
	void getShares(mpz_t**, mpz_t*, int);

	void computeLagrangeWeight();
	void computeSharingMatrix();

	//Reconstruct a secret from shares
	void reconstructSecret(mpz_t, mpz_t*, bool);
	void reconstructSecret(mpz_t*, mpz_t**, int, bool);

	//Modular Multiplication
	void modMul(mpz_t, mpz_t, mpz_t);
	void modMul(mpz_t*, mpz_t*, mpz_t*, int);
	void modMul(mpz_t, mpz_t, long);
	void modMul(mpz_t*, mpz_t*, long, int);
	void modMul(mpz_t*, mpz_t*, mpz_t, int);

	//Modular Addition
	void modAdd(mpz_t, mpz_t, mpz_t);
	void modAdd(mpz_t*, mpz_t*, mpz_t*, int);
	void modAdd(mpz_t, mpz_t, long);
	void modAdd(mpz_t*, mpz_t*, long, int);
	void modAdd(mpz_t*, mpz_t*, mpz_t, int); 

	//Modular Subtraction
	void modSub(mpz_t, mpz_t, mpz_t);
	void modSub(mpz_t*, mpz_t*, mpz_t*, int);
	void modSub(mpz_t, mpz_t, long);
	void modSub(mpz_t, long, mpz_t);
	void modSub(mpz_t*, mpz_t*, long, int);
	void modSub(mpz_t*, long, mpz_t*, int);
	void modSub(mpz_t*, mpz_t*, mpz_t, int); 
	void modSub(mpz_t*, mpz_t, mpz_t*, int); 

	//Modular Exponentiation
	void modPow(mpz_t, mpz_t, mpz_t);
	void modPow(mpz_t*, mpz_t*, mpz_t*, int);
	void modPow(mpz_t, mpz_t, long);
	void modPow(mpz_t*, mpz_t*, long, int);

	//Modular Inverse
	void modInv(mpz_t, mpz_t);
	void modInv(mpz_t*, mpz_t*, int);

	//Modular Square root
	void modSqrt(mpz_t, mpz_t);
	void modSqrt(mpz_t*, mpz_t*, int);

	//Miscellaneous Functions
	void modSum(mpz_t, mpz_t*, int);
	//void getPrime(mpz_t prime, int bits);
	void copy(mpz_t* src, mpz_t* des, int size);
	void mod(mpz_t* result, mpz_t* a, mpz_t* m, int size);
	void mod(mpz_t* result, mpz_t* a, mpz_t m, int size); 

private:
	mpz_t fieldSize;
	int threshold;
	int peers;
	std::vector<long> coefficients;
	mpz_t* lagrangeWeight;
	mpz_t** sharingMatrix;
	int bits; 
	gmp_randstate_t rstate; 
};
#endif

