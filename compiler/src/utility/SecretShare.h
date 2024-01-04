/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame
   ** Department of Computer Science and Engineering, University at Buffalo (SUNY)

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

#include "stdint.h"
#include <cstdlib>
#include <gmp.h>
#include <iostream>
#include <math.h>
#include <vector>

class SecretShare {
public:
    // SecretShare() {};
    virtual ~SecretShare(){};
    virtual void getFieldSize(mpz_t){};
    virtual void getShares(mpz_t *, mpz_t){};
    virtual void getShares(mpz_t **, mpz_t *, int){};
    virtual void reconstructSecret(mpz_t, mpz_t *){};
    virtual void reconstructSecret(mpz_t *, mpz_t **, int){};
};

class ShamirSS : public SecretShare {
public:
    ShamirSS(int, int, mpz_t);
    ~ShamirSS(){};

    // Set the number of peers
    void setPeers(int p);
    int getPeers();
    int getThreshold();
    void getFieldSize(mpz_t);
    // Set the privacy threshold
    void setThreshold(int t);

    // Divide a secret into n shares
    void getShares(mpz_t *, mpz_t);
    void getShares(mpz_t **, mpz_t *, int);

    void computeLagrangeWeight();
    void computeSharingMatrix();

    // Reconstruct a secret from shares
    void reconstructSecret(mpz_t, mpz_t *);
    void reconstructSecret(mpz_t *, mpz_t **, int);

    // Modular Multiplication
    void modMul(mpz_t, mpz_t, mpz_t);
    void modMul(mpz_t *, mpz_t *, mpz_t *, int);
    void modMul(mpz_t, mpz_t, long);
    void modMul(mpz_t *, mpz_t *, long, int);
    void modMul(mpz_t *, mpz_t *, mpz_t, int);

    // Modular Addition
    void modAdd(mpz_t, mpz_t, mpz_t);
    void modAdd(mpz_t *, mpz_t *, mpz_t *, int);
    void modAdd(mpz_t, mpz_t, long);
    void modAdd(mpz_t *, mpz_t *, long, int);
    void modAdd(mpz_t *, mpz_t *, mpz_t, int);

    // Modular Subtraction
    void modSub(mpz_t, mpz_t, mpz_t);
    void modSub(mpz_t *, mpz_t *, mpz_t *, int);
    void modSub(mpz_t, mpz_t, long);
    void modSub(mpz_t, long, mpz_t);
    void modSub(mpz_t *, mpz_t *, long, int);
    void modSub(mpz_t *, long, mpz_t *, int);
    void modSub(mpz_t *, mpz_t *, mpz_t, int);
    void modSub(mpz_t *, mpz_t, mpz_t *, int);

    // Modular Exponentiation
    void modPow(mpz_t, mpz_t, mpz_t);
    void modPow(mpz_t *, mpz_t *, mpz_t *, int);
    void modPow(mpz_t, mpz_t, long);
    void modPow(mpz_t *, mpz_t *, long, int);

    // Modular Inverse
    void modInv(mpz_t, mpz_t);
    void modInv(mpz_t *, mpz_t *, int);

    // Modular Square root
    void modSqrt(mpz_t, mpz_t);
    void modSqrt(mpz_t *, mpz_t *, int);

    // Miscellaneous Functions
    void modSum(mpz_t, mpz_t *, int);
    // void getPrime(mpz_t prime, int bits);
    void copy(mpz_t *src, mpz_t *des, int size);
    void mod(mpz_t *result, mpz_t *a, mpz_t *m, int size);
    void mod(mpz_t *result, mpz_t *a, mpz_t m, int size);

private:
    mpz_t fieldSize;
    int threshold;
    int peers;
    std::vector<long> coefficients;
    mpz_t *lagrangeWeight;
    mpz_t **sharingMatrix;
    int bits;
    gmp_randstate_t rstate;
};

// templated subclass, will be fully implemented below because of T's
template <typename T>
class RSS : public SecretShare {
public:
    RSS(int, int); // only needs n and t, the ring size is derived from sizeof(T)
    ~RSS(){};

    void getShares(T *, T);
    void reconstructSecret(T*, T **, int);
};

template <typename T>
void RSS<T>::getShares(T* result, T y) {
    // mpz_t temp;
    // mpz_init(temp);
    // mpz_set_ui(result, 0);
    // for (int peer = 0; peer < peers; peer++) {
    //     modMul(temp, y[peer], lagrangeWeight[peer]);
    //     modAdd(result, result, temp);
    // }
    // mpz_clear(temp);
}
template <typename T>
void RSS<T>::reconstructSecret(T *result, T **y, int size) {
    // mpz_t temp;
    // mpz_init(temp);
    // for (int i = 0; i < size; i++)
    //     mpz_set_ui(result[i], 0);
    // for (int i = 0; i < size; i++) {
    //     for (int peer = 0; peer < peers; peer++) {
    //         modMul(temp, y[peer][i], lagrangeWeight[peer]);
    //         modAdd(result[i], result[i], temp);
    //     }
    // }
    // mpz_clear(temp);
}



#endif
