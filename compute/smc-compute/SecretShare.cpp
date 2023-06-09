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
#include <cstdlib>
#include <cstring>
#include <gmp.h>
#include <iostream>
#include <math.h>
#include <vector>

/*
 * the constructor receives:
 * p - the number of computational parties or peers,
 * t - secret sharing threshold,
 * mod - modulus of the field/computation,
 * id - the ID of the current party, and
 * keys - an array of 16-byte keys of size threshold, which are used as PRG seeds in optimized multiplication.
 */
SecretShare::SecretShare(unsigned int p, unsigned int t, mpz_t mod, unsigned int id, unsigned char *keys[KEYSIZE]) {

    // keys are properly passed using the getter function, so they are now private members of NodeNetwork
    peers = p;
    threshold = t;
    myid = id;
    
    mpz_init(fieldSize);
    mpz_set(fieldSize, mod);
    gmp_printf("fieldSize %Zu\n", fieldSize);
    unsigned int i;
    // initialize arrays of indices
    sendToIDs = (unsigned int *)malloc(sizeof(unsigned int) * threshold);
    recvFromIDs = (unsigned int *)malloc(sizeof(unsigned int) * threshold);
    multIndices = new uint[peers];

    // IDs range between 1 and n=peers
    for (i = 0; i < threshold; i++) {
        sendToIDs[i] = ((myid + i) % peers) + 1;
        if ((threshold - i) >= myid)
            recvFromIDs[i] = myid + peers - threshold + i;
        else
            recvFromIDs[i] = myid - threshold + i;
    }

    // printf("original\n");
    // for (int i = 0; i < threshold; i++) {
    //     printf("sendToIDs[%i]    %u\n", i, sendToIDs[i]);
    //     printf("recvFromIDs[%i]  %u\n", i, recvFromIDs[i]);
    // }
    // computeSharingMatrix();
    // computeLagrangeWeights();

    // corrected versionm
    // printf("test %i\n", (1 - 1 - 1) % 5 + 1);
    // printf("-1 % 5 %i\n", -1 % 5);
    // for (int j = 1; j < threshold+1; j++)
    // {
    //     printf("exp send[%i]  %i\n",j - 1, int(int(myid) + (j) - (1)) % int(peers) + 1);
    //     printf("exp recv[%i]  %i\n",j - 1, int(int(myid) - (j) - (1)) % int(peers) + 1);
    // }
    
    // for (int i = 0; i < threshold; i++) {
    //     sendToIDs  [i]= modulo((int(myid) + int(i+1) - int(1)), int(peers)) + int(1);
    //     recvFromIDs[i]= modulo((int(myid) - int(i+1) - int(1)), int(peers)) + int(1);
    // }
    // printf("corrected\n");
    for (int i = 0; i < threshold; i++) {
        printf("sendToIDs[%i]    %u\n", i, sendToIDs[i]);
    }
    for (int i = 0; i < threshold; i++) {
        printf("recvFromIDs[%i]  %u\n", i, recvFromIDs[i]);
    }


    // printf("peers: %u\n", peers);
    for (int i = 0; i < threshold; i++) {
        multIndices[i] = sendToIDs[i];
        multIndices[threshold + i + 1] = recvFromIDs[i];
    }
    multIndices[threshold] = id;


    // for (int i = 0; i < 2 * threshold; i++) {
    //     print_hexa(keys[i], KEYSIZE);
    // }
    // for (int i = 0; i <peers; i++) {
    //     printf("multIndices[%i]    %u\n", i, multIndices[i]);

    // }



    computeSharingMatrix();
    computeLagrangeWeights(); 
    gmp_randinit_mt(rstate);
    // for (i = 0; i < (threshold+1); i++) {
    // 	for (int j = 0; j < (threshold+1); j++) {
    // 		gmp_printf("%Zu,", lagrangeWeightsMult[i][j]);
    // 	}
    // printf("\n");
    // }

    // initialize PRGs
    mpz_t seed;
    mpz_init(seed);
    rstatesMult = (gmp_randstate_t *)malloc(sizeof(gmp_randstate_t) * (2 * threshold));
    for (i = 0; i < 2 * threshold; i++) {
        gmp_randinit_mt(rstatesMult[i]);
        mpz_import(seed, KEYSIZE, 1, sizeof(keys[i][0]), 0, 0, keys[i]);
        // gmp_printf("seed[%i]: %Zu\n",i, seed); //lower digits of seed different between parties?
        gmp_randseed(rstatesMult[i], seed);

    }
}

unsigned int SecretShare::getPeers() {
    return peers;
}

void SecretShare::getFieldSize(mpz_t field) {
    mpz_set(field, fieldSize);
}

unsigned int SecretShare::getThreshold() {
    return threshold;
}

unsigned int *SecretShare::getSendToIDs() {
    return sendToIDs;
}

unsigned int *SecretShare::getRecvFromIDs() {
    return recvFromIDs;
}

/* the function creates n=peers shares of the second argument */
void SecretShare::getShares(mpz_t *shares, mpz_t secret) {

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

    for (int degree = 0; degree < threshold + 1; degree++) {
        // set the free coefficient to the secret; otherwise generate a
        // random field element
        if (degree == 0)
            mpz_set(coefficient, secret);
        else {
            mpz_urandomm(coefficient, rstate, fieldSize);

            // the coefficient of the highest degree has to be non-zer to
            //  guarantee that this is a polynomial of degree t
            if (degree == threshold && mpz_sgn(coefficient) == 0)
                mpz_add_ui(coefficient, coefficient, 1);
        }

        // add the contribution of the current coefficient to each share
        for (int peer = 0; peer < peers; peer++) {
            modMul(temp, sharingMatrix[peer][degree], coefficient);
            modAdd(shares[peer], shares[peer], temp);
        }
    }
}

/*
 * this function is similar to the above, but generates shares for a number of
 *  secrets specified by the last argument
 */
void SecretShare::getShares(mpz_t **shares, mpz_t *secrets, int size) {

    mpz_t coefficient;
    mpz_init(coefficient);
    mpz_t temp;
    mpz_init_set_ui(temp, 0);
    int degree, peer;

    for (int i = 0; i < size; i++) {
        for (degree = 0; degree < threshold + 1; degree++) {
            // generate coefficients one at a time
            if (degree == 0)
                mpz_set(coefficient, secrets[i]);
            else {
                mpz_urandomm(coefficient, rstate, fieldSize);

                // the most significant coefficient must be non-zero
                if (degree == threshold && mpz_sgn(coefficient) == 0)
                    mpz_add_ui(coefficient, coefficient, 1);
            }

            // incorporate the coefficient into the shares
            for (peer = 0; peer < peers; peer++) {
                modMul(temp, sharingMatrix[peer][degree], coefficient);
                modAdd(shares[peer][i], shares[peer][i], temp);
            }
        }
    }
    mpz_clear(coefficient);
    mpz_clear(temp);
}

void SecretShare::modMul(mpz_t result, mpz_t x, mpz_t y) {
    mpz_mul(result, x, y);
    mpz_mod(result, result, fieldSize);
}

void SecretShare::modMul(mpz_t *result, mpz_t *x, mpz_t *y, int size) {
    for (int i = 0; i < size; i++)
        modMul(result[i], x[i], y[i]);
}

void SecretShare::modMul(mpz_t result, mpz_t x, long y) {
    mpz_mul_si(result, x, y);
    mpz_mod(result, result, fieldSize);
}

void SecretShare::modMul(mpz_t *result, mpz_t *x, long y, int size) {
    for (int i = 0; i < size; ++i) {
        modMul(result[i], x[i], y);
    }
}

void SecretShare::modMul(mpz_t *result, mpz_t *x, mpz_t y, int size) {
    for (int i = 0; i < size; ++i) {
        modMul(result[i], x[i], y);
    }
}

void SecretShare::modAdd(mpz_t result, mpz_t x, mpz_t y) {
    mpz_add(result, x, y);
    mpz_mod(result, result, fieldSize);
}

void SecretShare::modAdd(mpz_t *result, mpz_t *x, mpz_t *y, int size) {
    for (int i = 0; i < size; i++)
        modAdd(result[i], x[i], y[i]);
}

void SecretShare::modAdd(mpz_t result, mpz_t x, long y) {
    mpz_t y1;
    mpz_init_set_si(y1, y);
    mpz_add(result, x, y1);
    mpz_mod(result, result, fieldSize);
    mpz_clear(y1);
}

void SecretShare::modAdd(mpz_t *result, mpz_t *x, long y, int size) {
    for (int i = 0; i < size; ++i)
        modAdd(result[i], x[i], y);
}

void SecretShare::modAdd(mpz_t *result, mpz_t *x, mpz_t y, int size) {
    for (int i = 0; i < size; i++)
        modAdd(result[i], x[i], y);
}

void SecretShare::modSub(mpz_t result, mpz_t x, mpz_t y) {
    mpz_sub(result, x, y);
    mpz_mod(result, result, fieldSize);
}

void SecretShare::modSub(mpz_t *result, mpz_t *x, mpz_t *y, int size) {
    for (int i = 0; i < size; i++)
        modSub(result[i], x[i], y[i]);
}

void SecretShare::modSub(mpz_t result, mpz_t x, long y) {
    mpz_t y1;
    mpz_init_set_si(y1, y);
    mpz_sub(result, x, y1);
    mpz_mod(result, result, fieldSize);
    mpz_clear(y1);
}

void SecretShare::modSub(mpz_t result, long x, mpz_t y) {
    mpz_t x1;
    mpz_init_set_si(x1, x);
    mpz_sub(result, x1, y);
    mpz_mod(result, result, fieldSize);
    mpz_clear(x1);
}

void SecretShare::modSub(mpz_t *result, mpz_t *x, long y, int size) {
    for (int i = 0; i < size; ++i)
        modSub(result[i], x[i], y);
}

void SecretShare::modSub(mpz_t *result, long x, mpz_t *y, int size) {
    for (int i = 0; i < size; ++i)
        modSub(result[i], x, y[i]);
}

void SecretShare::modSub(mpz_t *result, mpz_t *x, mpz_t y, int size) {
    for (int i = 0; i < size; ++i)
        modSub(result[i], x[i], y);
}

void SecretShare::modSub(mpz_t *result, mpz_t x, mpz_t *y, int size) {
    for (int i = 0; i < size; ++i)
        modSub(result[i], x, y[i]);
}

void SecretShare::modPow(mpz_t result, mpz_t base, mpz_t exponent) {
    mpz_powm(result, base, exponent, fieldSize);
}

void SecretShare::modPow(mpz_t *result, mpz_t *base, mpz_t *exponent, int size) {
    for (int i = 0; i < size; i++)
        mpz_powm(result[i], base[i], exponent[i], fieldSize);
}

void SecretShare::modPow(mpz_t result, mpz_t base, long exponent) {
    mpz_t value;
    mpz_init_set_si(value, exponent);
    modAdd(value, value, (long)0);
    mpz_powm(result, base, value, fieldSize);
    mpz_clear(value);
}

void SecretShare::modPow(mpz_t *result, mpz_t *base, long exponent, int size) {
    for (int i = 0; i < size; ++i)
        modPow(result[i], base[i], exponent);
}

void SecretShare::modInv(mpz_t result, mpz_t value) {
    mpz_t temp;
    mpz_init(temp);
    mpz_sub_ui(temp, fieldSize, 2);
    modPow(result, value, temp);
    mpz_clear(temp);
}

void SecretShare::modInv(mpz_t *result, mpz_t *values, int size) {
    for (int i = 0; i < size; i++)
        modInv(result[i], values[i]);
}

void SecretShare::modSqrt(mpz_t result, mpz_t x) {
    mpz_t temp;
    mpz_init(temp);
    mpz_add_ui(temp, fieldSize, 1);
    mpz_div_ui(temp, temp, 4);
    modPow(result, x, temp);
    mpz_clear(temp);
}

void SecretShare::modSqrt(mpz_t *result, mpz_t *x, int size) {
    mpz_t *power = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++) {
        mpz_init(power[i]);
        mpz_add_ui(power[i], fieldSize, 1);
        mpz_div_ui(power[i], power[i], 4);
    }
    modPow(result, x, power, size);
    for (int i = 0; i < size; i++)
        mpz_clear(power[i]);
}

void SecretShare::modSum(mpz_t result, mpz_t *x, int size) {
    mpz_set_ui(result, 0);
    for (int i = 0; i < size; ++i) {
        mpz_add(result, result, x[i]);
    }
    mpz_mod(result, result, fieldSize);
}

void SecretShare::mod(mpz_t *result, mpz_t *a, mpz_t *m, int size) {
    mpz_t tmp;
    mpz_init(tmp);
    for (int i = 0; i < size; ++i) {
        mpz_init_set_ui(tmp, 0);
        mpz_add(tmp, a[i], m[i]);
        if (mpz_cmp(tmp, fieldSize) > 0)
            mpz_sub(result[i], tmp, fieldSize);
        else {
            mpz_mod(result[i], a[i], m[i]);
            mpz_mod(result[i], result[i], fieldSize);
        }
    }
}

void SecretShare::mod(mpz_t *result, mpz_t *a, mpz_t m, int size) {
    mpz_t tmp;
    mpz_init(tmp);
    for (int i = 0; i < size; ++i) {
        mpz_init_set_ui(tmp, 0);
        mpz_add(tmp, a[i], m);
        if (mpz_cmp(tmp, fieldSize) > 0)
            mpz_sub(result[i], tmp, fieldSize);
        else {
            mpz_mod(result[i], a[i], m);
            mpz_mod(result[i], result[i], fieldSize);
        }
    }
}

void SecretShare::copy(mpz_t *src, mpz_t *des, int size) {
    for (int i = 0; i < size; i++)
        mpz_set(des[i], src[i]);
}

/*
 * this function initializes a two-dimensional array called sharingMatrix.
 * sharingMatrix is used during computation of shares, i.e., evaluating a
 * polynomial at different points. It basically stores different powers of
 * points on which a polynomial will need to be evaluated.
 */
void SecretShare::computeSharingMatrix() {

    mpz_t t1, t2;
    mpz_init(t1);
    mpz_init(t2);
    int i;

    sharingMatrix = (mpz_t **)malloc(sizeof(mpz_t *) * peers);
    for (i = 0; i < peers; i++)
        sharingMatrix[i] = (mpz_t *)malloc(sizeof(mpz_t) * peers);
    for (i = 0; i < peers; i++)
        for (int j = 0; j < peers; j++)
            mpz_init(sharingMatrix[i][j]);

    for (int i = 0; i < peers; i++) {
        for (int j = 0; j < 2 * threshold + 1; j++) {
            mpz_set_ui(t1, i + 1);
            mpz_set_ui(t2, j);
            modPow(sharingMatrix[i][j], t1, t2);
        }
    }
    mpz_clear(t1);
    mpz_clear(t2);
}

/*
 * this function pre-computed Largrange coefficients for:
 * 1) reconstructing a secret from n=peers shares, which corresponds to evaluating the polynomial at point 0;
 * 2) reconstructing a secret from threshold+1 shares using points at indices stored in recvFromIDs (used by Open); and
 * 3) evaluating a polynomial on threshold+1 points at indices stored in sendToIDs as well as myid, where the polynomial is encoded using threshold+1 values at indices stored in recvFromIDs and point 0.
 */
void SecretShare::computeLagrangeWeights() {
    mpz_t nom, denom, t1, t2, temp;
    mpz_init(nom);
    mpz_init(denom);
    mpz_init(t1);
    mpz_init(t2);
    mpz_init(temp);
    unsigned int i, j, l;

    // first set of coefficients
    lagrangeWeightsAll = (mpz_t *)malloc(sizeof(mpz_t) * peers);

    for (i = 0; i < peers; i++)
        mpz_init(lagrangeWeightsAll[i]);

    // refer to https://en.wikipedia.org/wiki/Lagrange_polynomial
    // at loop iteration i compute ell_i(0)
    for (i = 0; i < peers; i++) {
        mpz_set_ui(nom, 1);
        mpz_set_ui(denom, 1);
        mpz_set_ui(t2, i + 1);
        for (l = 0; l < peers; l++) {
            if (l != i) {
                mpz_set_ui(t1, l + 1);
                modMul(nom, nom, t1); // nominator is multiplied by x_l=l+1
                modSub(temp, t1, t2); // denominator is muliplied by x_l-x_i=l-i
                modMul(denom, denom, temp);
            }
        }
        modInv(temp, denom);
        modMul(lagrangeWeightsAll[i], nom, temp);
        // gmp_printf("lagrangeWeightsAll[%i]: %Zu\n",i, lagrangeWeightsAll[i]);
    }

    // second set
    lagrangeWeightsThreshold = (mpz_t *)malloc(sizeof(mpz_t) * (threshold + 1));

    for (i = 0; i < threshold + 1; i++)
        mpz_init(lagrangeWeightsThreshold[i]);

    for (i = 0; i < threshold + 1; i++) {
        mpz_set_ui(nom, 1);
        mpz_set_ui(denom, 1);
        if (i == threshold)
            mpz_set_ui(t2, myid);
        else
            mpz_set_ui(t2, recvFromIDs[i]);
        // gmp_printf("\n t2[%i] %Zu , ",i, t2);
        for (l = 0; l < threshold + 1; l++) {
            if (l != i) {
                if (l == threshold)
                    mpz_set_ui(t1, myid);
                else
                    mpz_set_ui(t1, recvFromIDs[l]);
                // gmp_printf("t1[%i] %Zu, ",l, t1);
                modMul(nom, nom, t1);
                modSub(temp, t1, t2);
                modMul(denom, denom, temp);
            }
        }
        // gmp_printf("\n nom %Zu , ",nom);
        // gmp_printf("\n denom %Zu , ",denom);

        modInv(temp, denom);
        modMul(lagrangeWeightsThreshold[i], nom, temp);
        gmp_printf("lagrangeWeightsThreshold[%i]: %Zu, %Zu\n",i, t2, lagrangeWeightsThreshold[i]);
    }

    // third set of coefficients
    lagrangeWeightsMult = (mpz_t **)malloc(sizeof(mpz_t *) * (threshold + 1));

    for (i = 0; i < threshold + 1; i++) {
        lagrangeWeightsMult[i] = (mpz_t *)malloc(sizeof(mpz_t) * (threshold + 1));

        for (j = 0; j < threshold + 1; j++)
            mpz_init(lagrangeWeightsMult[i][j]);
    }

    // weights at row i (i.e., in largrangeWeightsMult[i]) correspond to polynomial evaluation on point sendToIDs[i] and row threshold corresponds to evaluation on point myid
    // the denominator is independent of the point and needs to be computed only once
    // column j corresponds to the coefficient on point recvFromIDs[j] in the interpolation set; the point at position j=threshold is 0
    for (j = 0; j < threshold + 1; j++) {
        mpz_set_ui(denom, 1);
        if (j == threshold)
            mpz_set_ui(t2, 0);
        else
            mpz_set_ui(t2, recvFromIDs[j]);

        // computing denominator
        for (l = 0; l < threshold + 1; l++) {
            if (l != j) {
                if (l == threshold)
                    mpz_set_ui(t1, 0);
                else
                    mpz_set_ui(t1, recvFromIDs[l]);
                modSub(temp, t1, t2);
                modMul(denom, denom, temp);
            }
        }
        modInv(temp, denom);
        mpz_set(denom, temp);

        // computing point-dependent nominators
        for (i = 0; i < threshold + 1; i++) {
            mpz_set_ui(nom, 1);
            if (i == threshold)
                mpz_set_ui(t2, myid);
            else
                mpz_set_ui(t2, sendToIDs[i]);

            for (l = 0; l < threshold + 1; l++) {
                if (l != j) {
                    if (l == threshold)
                        mpz_set_ui(t1, 0);
                    else
                        mpz_set_ui(t1, recvFromIDs[l]);
                    modSub(temp, t1, t2);
                    modMul(nom, nom, temp);
                }
            }
            modMul(lagrangeWeightsMult[i][j], nom, denom);
            // gmp_printf("lagrangeWeightsMult[%i][%i]: %Zu\n",i,j, lagrangeWeightsMult[i][j]);
        }
    }

    mpz_clear(nom);
    mpz_clear(denom);
    mpz_clear(t1);
    mpz_clear(t2);
    mpz_clear(temp);
}

/* reconstruction of a secret from n=peers shares */
void SecretShare::reconstructSecret(mpz_t result, mpz_t *y) {
    mpz_t temp;
    mpz_init(temp);
    mpz_set_ui(result, 0);
    for (int peer = 0; peer < peers; peer++) {
        modMul(temp, y[peer], lagrangeWeightsAll[peer]);
        modAdd(result, result, temp);
    }
    mpz_clear(temp);
}

/* reconstruction of a number of secrets from n=peers shares each */
void SecretShare::reconstructSecret(mpz_t *result, mpz_t **y, int size) {
    mpz_t temp;
    mpz_init(temp);
    for (int i = 0; i < size; i++)
        mpz_set_ui(result[i], 0);
    for (int i = 0; i < size; i++) {
        for (int peer = 0; peer < peers; peer++) {
            modMul(temp, y[peer][i], lagrangeWeightsAll[peer]);
            modAdd(result[i], result[i], temp);
        }
    }
    mpz_clear(temp);
}

void SecretShare::reconstructSecretMult(mpz_t *result, mpz_t **y, int size) {
    mpz_t temp;
    mpz_init(temp);
    for (int i = 0; i < size; i++)
        mpz_set_ui(result[i], 0);
    for (int i = 0; i < size; i++) {
        for (int peer = 0; peer < peers; peer++) {
            modMul(temp, y[peer][i], lagrangeWeightsAll[multIndices[peer]-1]);
            modAdd(result[i], result[i], temp);
        }
    }
    mpz_clear(temp);
}
/*
 * reconstruction of a number of secrets from threshold+1 shares each.
 * the shares are expected to correspond to points at indices stored in recvFromIDs as well as myid.
 */
void SecretShare::reconstructSecretFromMin(mpz_t *result, mpz_t **y, unsigned int size) {
    mpz_t temp;
    mpz_init(temp);
    unsigned i, j;

    for (i = 0; i < size; i++)
        mpz_set_ui(result[i], 0);

    for (i = 0; i < size; i++) {
        for (j = 0; j < threshold + 1; j++) {
            // gmp_printf("LWT[%i], y[%i][%i] :  (%Zu, %Zu) \n",j, j, i,lagrangeWeightsThreshold[j], y[j][i]);
            modMul(temp, y[j][i], lagrangeWeightsThreshold[j]);
            modAdd(result[i], result[i], temp);
        }
    }
    mpz_clear(temp);
}
void SecretShare::reconstructSecretFromMin_test(mpz_t *result, mpz_t **y, unsigned int size) {
    mpz_t temp;
    mpz_init(temp);
    unsigned i, j;

    for (i = 0; i < size; i++)
        mpz_set_ui(result[i], 0);
    uint index = 0;
    for (i = 0; i < size; i++) {
        for (j = 0; j < threshold + 1; j++) {
             if (j == threshold)
                index = myid - 1;
            else
                index = recvFromIDs[j] - 1;
            printf("index %i\n",index); 
            modMul(temp, y[j][i], lagrangeWeightsAll[index]);
            modAdd(result[i], result[i], temp);
        }
    }
    mpz_clear(temp);
}

/*
 * for each set of threshold+1 input shares, evaluate a polynomial represented by them on another set of threshold+1 points.
 * input is of size (threshold+1)*size, i.e., a row stores size shares from a single party.
 * ouput result is also of size (threshold+1)*size, a row stores size shares for a single party (parties in sendToIDs and myid).
 */
void SecretShare::getSharesMul(mpz_t **result, mpz_t **input, unsigned int size) {
    mpz_t temp;
    mpz_init(temp);
    unsigned i, j, k;

    for (i = 0; i < threshold + 1; i++)
        for (j = 0; j < size; j++)
            mpz_set_ui(result[i][j], 0);

    for (i = 0; i < size; i++) {
        // for each secret, evaluate the polynomial on threshold+1 points
        for (j = 0; j < threshold + 1; j++)
            for (k = 0; k < threshold + 1; k++) {
                modMul(temp, input[j][i], lagrangeWeightsMult[k][j]);
                modAdd(result[k][i], result[k][i], temp);
            }
    }
    mpz_clear(temp);
}

void SecretShare::Seed(unsigned char *key_0, unsigned char *key_1) {

    gmp_randinit_mt(rstate_0); // m1
    gmp_randinit_mt(rstate_1); // p1

    mpz_t seed;
    mpz_init(seed);

    mpz_import(seed, 16, 1, sizeof(key_0[0]), 0, 0, key_0);
    gmp_randseed(rstate_0, seed);

    mpz_import(seed, 16, 1, sizeof(key_1[0]), 0, 0, key_1);
    gmp_randseed(rstate_1, seed);

    seeded = 1;
    mpz_clear(seed);
}

void SecretShare::getShares2(mpz_t *temp, mpz_t *rand, mpz_t **data, int size) {

    //	printf("threshold is %d and seed %d and myiid %d\n",threshold, seeded, myiid);

    mpz_t coefficient;
    mpz_init(coefficient);
    for (int i = 0; i < size; i++) {
        mpz_urandomm(rand[i], rstate_1, fieldSize); // step 2
    }

    for (int i = 0; i < size; i++) {
        mpz_sub(coefficient, rand[i], temp[i]);
        mpz_mul(coefficient, coefficient, id_p1_inv);
        mpz_mul_ui(data[myid - 1][i], coefficient, myid); // for id
        mpz_add(data[myid - 1][i], data[myid - 1][i], temp[i]);
        mpz_mod(data[myid - 1][i], data[myid - 1][i], fieldSize);
        mpz_mul_ui(data[id_m1 - 1][i], coefficient, id_m1); // for id-1
        mpz_add(data[id_m1 - 1][i], data[id_m1 - 1][i], temp[i]);
        mpz_mod(data[id_m1 - 1][i], data[id_m1 - 1][i], fieldSize);
    }
    mpz_clear(coefficient);
    for (int i = 0; i < size; i++) {
        mpz_urandomm(temp[i], rstate_0, fieldSize); // step 5, the "or" condition
    }
}

// start_ind dictates which half of the array to take from
// if 0, the first half. if t, the second half
void SecretShare::PRG(mpz_t **output, uint size, uint start_ind) {
    for (int i = 0; i < threshold; i++) {
        // printf("state = %i\n",i + start_ind);
        // gmp_printf("rstatesMult[%i]: %Zu\n",i, rstatesMult[i]);

        for (int j = 0; j < size; j++) {
            mpz_urandomm(output[i][j], rstatesMult[i + start_ind], fieldSize);
        }
    }
}
void SecretShare::checkSeed() {
    if (seeded == 0) {
        printf("not seeded\n");
    } else {
        printf("seeded\n");
    }
}

void SecretShare::getCoef(int id) {
    myid = id;
    id_p1 = (myid + 1) % (peers + 1);
    if (id_p1 == 0)
        id_p1 = 1;

    id_m1 = (myid - 1) % (peers + 1);
    if (id_m1 == 0)
        id_m1 = peers;

    mpz_init(id_p1_inv);
    mpz_set_ui(id_p1_inv, id_p1);
    mpz_invert(id_p1_inv, id_p1_inv, fieldSize);
    // printf("id_p1: %u\n", id_p1);
    // gmp_printf("id_p1_inv: %Zu\n", id_p1_inv);
}

int modulo(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}