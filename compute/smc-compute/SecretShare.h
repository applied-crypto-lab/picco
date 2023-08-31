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

#ifndef SECRETSHARE_H_
#define SECRETSHARE_H_

#include "NodeNetwork.h"
#include "SecretShare.h"
#include "stdint.h"
#include <cstdlib>
#include <gmp.h>
#include <iostream>
#include <math.h>
#include <vector>

class SecretShare {

public:
    SecretShare(unsigned int, unsigned int, mpz_t, unsigned int, unsigned char *[KEYSIZE],std::map<std::string, std::vector<int>>);

    unsigned int getPeers();
    unsigned int getThreshold();
    void getFieldSize(mpz_t);

    unsigned int *getSendToIDs();
    unsigned int *getRecvFromIDs();

    void print_poly();

    // Create n shares of a secret or multiple secrets
    void getShares(mpz_t *, mpz_t);
    void getShares(mpz_t **, mpz_t *, int);

    void computeLagrangeWeights();
    void computeSharingMatrix();

    // Reconstruct a secret from n shares
    void reconstructSecret(mpz_t, mpz_t *);
    void reconstructSecret(mpz_t *, mpz_t **, int);
    // Reconstruct a secret from the minimum (threshold+1) number of shares
    void reconstructSecretFromMin(mpz_t *, mpz_t **, unsigned int);
    void reconstructSecretMult(mpz_t *result, mpz_t **y, int size);
    void reconstructSecretFromMin_test(mpz_t *, mpz_t **, unsigned int);

    // Evaluate a polynomial represented by threshold+1 shares on another threshold+1 points
    void getSharesMul(mpz_t **, mpz_t **, unsigned int);

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
    void modAdd(mpz_t *, mpz_t *, long *, int);
    void modAdd(mpz_t *, mpz_t *, int *, int);

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
    void modPow2(mpz_t, int);
    void modPow2(mpz_t *result, int *exponent, int size);
    void modPow2(mpz_t *result, mpz_t *exponent, int size);

    // Modular Inverse
    void modInv(mpz_t, mpz_t);
    void modInv(mpz_t *, mpz_t *, int);

    // Modular Square root
    void modSqrt(mpz_t, mpz_t);
    void modSqrt(mpz_t *, mpz_t *, int);

    // Miscellaneous Functions
    void modSum(mpz_t, mpz_t *, int);
    void copy(mpz_t *src, mpz_t *des, int size);
    void mod(mpz_t *result, mpz_t *a, mpz_t *m, int size);
    void mod(mpz_t *result, mpz_t *a, mpz_t m, int size);

    // computation for 3P multiplication, to be removed
    void getShares2(mpz_t *temp, mpz_t *rand, mpz_t **data, int size);
    void Seed(unsigned char *key_0, unsigned char *key_1);
    void checkSeed();
    void getCoef(int id);

    void PRG(mpz_t **output, uint size, uint start_ind);

    std::map<std::string, std::vector<int>> polynomials; // public for easier access in Random, but polynomials are only accessed inside of generateRandomValue?


private:
    mpz_t fieldSize;
    unsigned int threshold;
    unsigned int peers;
    unsigned int myid;

    std::vector<long> coefficients;
    // coeffiicents for polynomial reconstruction on point 0 from all shares
    mpz_t *lagrangeWeightsAll;
    // coefficients for polynomial reconstruction on point 0 from threshold+1 shares at points with indices in recvFromIDs
    mpz_t *lagrangeWeightsThreshold;
    // coefficients for polynonial evaluation on threshold points at indices stored in sendToIDs, where the polynomial is stored as threshold+1 values at indices in recvFromIDs and point 0
    // size is threshold*(threshold+1)
    mpz_t **lagrangeWeightsMult;
    mpz_t **sharingMatrix;
    //    int bits;
    gmp_randstate_t rstate;

    // peers to whom a share or shares will be sent, numbered consequently
    // from myid (myid+1, ..., myid+t)
    unsigned int *sendToIDs;
    // peers to receive shares from or generate via PRGs, numbered from myid
    // in the decreasing order (myid-t, ..., myid-1) ***this is ultimately INCREASING order
    // e.g. for id = 3, recvFromIDs[0] = 4, recvFromIDs[1] = 5
    unsigned int *recvFromIDs;

    uint *multIndices;

    // additional data structures for multiplication
    gmp_randstate_t *rstatesMult;

    // for 3-party multiplication
    gmp_randstate_t rstate_0;
    gmp_randstate_t rstate_1;
    int seeded;
    int id_p1;
    int id_m1;
    mpz_t id_p1_inv;
};

// substitute for % operator to (properly) handle negative numbers
int modulo(int a, int b);

void smc_batch_free_operator(mpz_t **op, int size);
void smc_batch_free_operator(mpz_t ***op, int size);
int smc_compute_len(int alen, int blen);

void convertFloat(float value, int K, int L, mpz_t **elements);
void convertDouble(double value, int K, int L, mpz_t **elements);

#endif
