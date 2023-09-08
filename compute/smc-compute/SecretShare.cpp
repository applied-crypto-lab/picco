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
SecretShare::SecretShare(unsigned int p, unsigned int t, mpz_t mod, unsigned int id, unsigned int _numThreads, unsigned char *keys[KEYSIZE], std::map<std::string, std::vector<int>> _polynomials) {

    // keys are properly passed using the getter function, so they are now private members of NodeNetwork
    peers = p;
    threshold = t;
    myid = id;

    polynomials = _polynomials;
    numThreads = _numThreads;

    mpz_init(fieldSize);
    mpz_set(fieldSize, mod);
    // gmp_printf("fieldSize %Zu\n", fieldSize);
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

    for (int i = 0; i < threshold; i++) {
        multIndices[i] = sendToIDs[i];
        multIndices[threshold + i + 1] = recvFromIDs[i];
    }
    multIndices[threshold] = id;

    computeSharingMatrix();
    computeLagrangeWeights();
    gmp_randinit_mt(rstate);

    // initialize PRGs
    mpz_t seed;
    mpz_init(seed);
    rstatesMult = (gmp_randstate_t *)malloc(sizeof(gmp_randstate_t) * (2 * threshold));
    for (i = 0; i < 2 * threshold; i++) {
        gmp_randinit_mt(rstatesMult[i]);
        mpz_import(seed, KEYSIZE, 1, sizeof(keys[i][0]), 0, 0, keys[i]);
        gmp_randseed(rstatesMult[i], seed);
    }
    pthread_mutex_init(&mutex, NULL);
    rand_isFirst_thread = (int *)malloc(sizeof(int) * numThreads);
    for (int i = 0; i < numThreads; i++) {
        rand_isFirst_thread[i] = 0;
    }
    rstates_thread = (gmp_randstate_t **)malloc(sizeof(gmp_randstate_t *) * polynomials.size());
    for (int i = 0; i < polynomials.size(); i++) {
        rstates_thread[i] = (gmp_randstate_t *)malloc(sizeof(gmp_randstate_t) * numThreads);
    }
    std::map<std::string, std::vector<int>>::iterator it;
    mpz_t *temp_keys = (mpz_t *)malloc(sizeof(mpz_t) * polynomials.size());
    int k = 0;
    for (it = polynomials.begin(); it != polynomials.end(); it++) {
        mpz_init(temp_keys[k]);
        mpz_set_str(temp_keys[k], ((*it).first).c_str(), 10);
        k++;
    }
    rstates = (gmp_randstate_t *)malloc(sizeof(gmp_randstate_t) * polynomials.size());
    for (int i = 0; i < polynomials.size(); i++) {
        gmp_randinit_default(rstates[i]);
        gmp_randseed(rstates[i], temp_keys[i]);
    }


    // }
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

void SecretShare::print_poly() {
    // for (auto &[key, value] : polynomials) {
    //     std::cout << key << ", ";
    //     for (size_t j = 0; j < value.size(); j++) {
    //         std::cout << value.at(j) << ", ";
    //     }
    //     std::cout << std::endl;
    // }
   return; 
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

void SecretShare::modAdd(mpz_t *result, mpz_t *x, long *y, int size) {
    mpz_t *ytmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set_si(ytmp[i], y[i]);
    modAdd(result, x, ytmp, size);
    for (int i = 0; i < size; i++)
        mpz_clear(ytmp[i]);
}

void SecretShare::modAdd(mpz_t *result, mpz_t *x, int *y, int size) {
    mpz_t *ytmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set_si(ytmp[i], y[i]);
    modAdd(result, x, ytmp, size);
    for (int i = 0; i < size; i++)
        mpz_clear(ytmp[i]);
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

void SecretShare::modPow2(mpz_t result, int exponent) {
    mpz_t value, base;
    mpz_init_set_ui(base, 2);
    mpz_init_set_si(value, exponent);
    // modAdd(value, value, (long)0); // assuming this just performs modular reduction, replaced with line below
    mpz_mod(value, value, fieldSize);
    mpz_powm(result, base, value, fieldSize);
    mpz_clear(value);
    mpz_clear(base);
}

void SecretShare::modPow2(mpz_t *result, int *exponent, int size) {
    // for (int i = 0; i < size; ++i)
    //     modPow(result[i], base[i], exponent);
    mpz_t value, base;
    mpz_init_set_ui(base, 2);

    for (int i = 0; i < size; ++i) {
        mpz_init_set_si(value, exponent[i]);
        mpz_mod(value, value, fieldSize);
        mpz_powm(result[i], base, value, fieldSize);
    }
    mpz_clear(value);
    mpz_clear(base);
}

void SecretShare::modPow2(mpz_t *result, mpz_t *exponent, int size) {
    // for (int i = 0; i < size; ++i)
    //     modPow(result[i], base[i], exponent);
    mpz_t value, base;
    mpz_init_set_ui(base, 2);

    for (int i = 0; i < size; ++i) {
        mpz_init_set(value, exponent[i]);
        mpz_mod(value, value, fieldSize);
        mpz_powm(result[i], base, value, fieldSize);
    }
    mpz_clear(value);
    mpz_clear(base);
}

void SecretShare::modPow(mpz_t *result, mpz_t *base, mpz_t *exponent, int size) {
    for (int i = 0; i < size; i++)
        mpz_powm(result[i], base[i], exponent[i], fieldSize);
}

void SecretShare::modPow(mpz_t result, mpz_t base, long exponent) {
    mpz_t value;
    mpz_init_set_si(value, exponent);
    // modAdd(value, value, (long)0);
    mpz_mod(value, value, fieldSize);
    mpz_powm(result, base, value, fieldSize);
    mpz_clear(value);
}

void SecretShare::modPow(mpz_t *result, mpz_t *base, long exponent, int size) {
    // for (int i = 0; i < size; ++i)
    //     modPow(result[i], base[i], exponent);
    mpz_t value;
    mpz_init_set_si(value, exponent);
    mpz_mod(value, value, fieldSize);
    for (int i = 0; i < size; ++i) {
        mpz_powm(result[i], base[i], value, fieldSize);
    }
    mpz_clear(value);
}

void SecretShare::modInv(mpz_t result, mpz_t value) {
    mpz_t temp;
    mpz_init(temp);
    mpz_sub_ui(temp, fieldSize, 2);
    modPow(result, value, temp);
    mpz_clear(temp);
}

void SecretShare::modInv(mpz_t *result, mpz_t *values, int size) {

    mpz_t temp;
    mpz_init(temp);
    mpz_sub_ui(temp, fieldSize, 2);
    for (int i = 0; i < size; i++)
        modPow(result[i], values[i], temp);
    // modInv(result[i], values[i]); // highly inefficient
    mpz_clear(temp);
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
        for (l = 0; l < threshold + 1; l++) {
            if (l != i) {
                if (l == threshold)
                    mpz_set_ui(t1, myid);
                else
                    mpz_set_ui(t1, recvFromIDs[l]);
                modMul(nom, nom, t1);
                modSub(temp, t1, t2);
                modMul(denom, denom, temp);
            }
        }

        modInv(temp, denom);
        modMul(lagrangeWeightsThreshold[i], nom, temp);
        // gmp_printf("lagrangeWeightsThreshold[%i]: %Zu, %Zu\n", i, t2, lagrangeWeightsThreshold[i]);
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
            modMul(temp, y[peer][i], lagrangeWeightsAll[multIndices[peer] - 1]);
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
            printf("index %i\n", index);
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
}

// random generation functions, was previously in Random.cpp

int SecretShare::computePolynomials(std::vector<int> polys, int point) {
    int result = 0;
    for (unsigned int i = 0; i < polys.size(); i++)
        result += pow(point, (polys.size() - 1 - i)) * polys[i];
    return result;
}

void SecretShare::generateRandValue(int nodeID, int bits, int size, mpz_t *results) {
    std::map<std::string, std::vector<int>>::iterator it;
    mpz_t zero, rand, temp;
    int polysize = polynomials.size();
    mpz_t *temp1 = (mpz_t *)malloc(sizeof(mpz_t) * polysize);
    mpz_t *temp2 = (mpz_t *)malloc(sizeof(mpz_t) * polysize);
    mpz_t *temp3 = (mpz_t *)malloc(sizeof(mpz_t) * polysize);
    mpz_init(zero);
    mpz_init(rand);
    mpz_init(temp);
    mpz_set_ui(zero, 0);
    for (int i = 0; i < polysize; i++) {
        mpz_init(temp1[i]);
        mpz_init(temp2[i]);
        mpz_init(temp3[i]);
    }

    int index = 0;
    std::vector<long> polyOutput;
    std::vector<long> denominator;
    long long combinations = nChoosek(getPeers(), getThreshold());

    // if (rand_isFirst == 0)
        // getNextRandValue(0, 0, polynomials, NULL);

    /*************** Evaluate the polynomials on points ******************/
    for (it = polynomials.begin(); it != polynomials.end(); it++) {
        std::vector<int> polys = (*it).second;
        denominator.push_back(polys[polys.size() - 1]);
        polyOutput.push_back(computePolynomials((*it).second, nodeID));
    }

    for (int m = 0; m < polysize; m++) {
        mpz_set_si(temp2[m], denominator[m]);
        mpz_set_si(temp3[m], polyOutput[m]);
        if (denominator[m] < 0)
            modAdd(temp2[m], zero, temp2[m]);
        if (polyOutput[m] < 0)
            modAdd(temp3[m], zero, temp3[m]);
        modInv(temp1[m], temp2[m]);
        modMul(temp3[m], temp3[m], temp1[m]);
    }

    /************* Generate the random values ******************/
    for (int i = 0; i < size; i++) {
        for (int m = 0; m < polysize; m++) {
            getNextRandValue(m, bits, polynomials, rand);
            mpz_div_ui(rand, rand, combinations);
            modMul(temp, rand, temp3[m]);
            modAdd(results[i], results[i], temp);
        }
    }
    /*********** Free the memory **************/
    for (int i = 0; i < polysize; i++) {
        mpz_clear(temp1[i]);
        mpz_clear(temp2[i]);
        mpz_clear(temp3[i]);
    }
    free(temp1);
    free(temp2);
    free(temp3);

    mpz_clear(zero);
    mpz_clear(rand);
}

void SecretShare::generateRandValue(int nodeID, int bits, int size, mpz_t *results, int threadID) {
    if (threadID == -1) {
        generateRandValue(nodeID, bits, size, results);
        return;
    }
    std::map<std::string, std::vector<int>>::iterator it;
    mpz_t zero, rand, temp;
    int polysize = polynomials.size();
    mpz_t *temp1 = (mpz_t *)malloc(sizeof(mpz_t) * polysize);
    mpz_t *temp2 = (mpz_t *)malloc(sizeof(mpz_t) * polysize);
    mpz_t *temp3 = (mpz_t *)malloc(sizeof(mpz_t) * polysize);
    mpz_init(zero);
    mpz_init(rand);
    mpz_init(temp);
    mpz_set_ui(zero, 0);
    for (int i = 0; i < polysize; i++) {
        mpz_init(temp1[i]);
        mpz_init(temp2[i]);
        mpz_init(temp3[i]);
    }

    int index = 0;
    std::vector<long> polyOutput;
    std::vector<long> denominator;
    long long combinations = nChoosek(getPeers(), getThreshold());

    if (rand_isFirst_thread[threadID] == 0)
        getNextRandValue(0, 0, polynomials, NULL, threadID);

    /*************** Evaluate the polynomials on points ******************/
    for (it = polynomials.begin(); it != polynomials.end(); it++) {
        std::vector<int> polys = (*it).second;
        denominator.push_back(polys[polys.size() - 1]);
        polyOutput.push_back(computePolynomials((*it).second, nodeID));
    }

    for (int m = 0; m < polysize; m++) {
        mpz_set_si(temp2[m], denominator[m]);
        mpz_set_si(temp3[m], polyOutput[m]);
        if (denominator[m] < 0)
            modAdd(temp2[m], zero, temp2[m]);
        if (polyOutput[m] < 0)
            modAdd(temp3[m], zero, temp3[m]);
        modInv(temp1[m], temp2[m]);
        modMul(temp3[m], temp3[m], temp1[m]);
    }

    /************* Generate the random values ******************/
    for (int i = 0; i < size; i++) {
        for (int m = 0; m < polysize; m++) {
            getNextRandValue(m, bits, polynomials, rand, threadID);
            mpz_div_ui(rand, rand, combinations);
            modMul(temp, rand, temp3[m]);
            modAdd(results[i], results[i], temp);
        }
    }
    /*********** Free the memory **************/
    for (int i = 0; i < polysize; i++) {
        mpz_clear(temp1[i]);
        mpz_clear(temp2[i]);
        mpz_clear(temp3[i]);
    }
    free(temp1);
    free(temp2);
    free(temp3);

    mpz_clear(zero);
    mpz_clear(rand);
    /* When multiple compute nodes attempt to access polynomials at the same time, there may exisit a conflict that results in the memory problem */
    /*for(unsigned int i = 0; i < 2 ; i++){
        polys = polynomials.find(oldseed[i])->second;
        polynomials.erase(oldseed[i]);
        polynomials.insert(std::pair<uint64_t, std::vector<int> >(newseed[i], polys));
    }*/
}

void SecretShare::generateRandValue(int nodeID, mpz_t mod, int size, mpz_t *results) {
    std::map<std::string, std::vector<int>>::iterator it;
    mpz_t zero, rand, temp;
    int polysize = polynomials.size();
    mpz_t *temp1 = (mpz_t *)malloc(sizeof(mpz_t) * polysize);
    mpz_t *temp2 = (mpz_t *)malloc(sizeof(mpz_t) * polysize);
    mpz_t *temp3 = (mpz_t *)malloc(sizeof(mpz_t) * polysize);
    mpz_init(zero);
    mpz_init(rand);
    mpz_init(temp);
    mpz_set_ui(zero, 0);
    for (int i = 0; i < polysize; i++) {
        mpz_init(temp1[i]);
        mpz_init(temp2[i]);
        mpz_init(temp3[i]);
    }

    int index = 0;
    std::vector<long> polyOutput;
    std::vector<long> denominator;
    long long combinations = nChoosek(getPeers(), getThreshold());

    // if (rand_isFirst == 0)
        // getNextRandValue(0, 0, polynomials, NULL);

    /*************** Evaluate the polynomials on points ******************/
    for (it = polynomials.begin(); it != polynomials.end(); it++) {
        std::vector<int> polys = (*it).second;
        denominator.push_back(polys[polys.size() - 1]);
        polyOutput.push_back(computePolynomials((*it).second, nodeID));
    }

    for (int m = 0; m < polysize; m++) {
        mpz_set_si(temp2[m], denominator[m]);
        mpz_set_si(temp3[m], polyOutput[m]);
        if (denominator[m] < 0)
            modAdd(temp2[m], zero, temp2[m]);
        if (polyOutput[m] < 0)
            modAdd(temp3[m], zero, temp3[m]);
        modInv(temp1[m], temp2[m]);
        modMul(temp3[m], temp3[m], temp1[m]);
    }

    /************* Generate the random values ******************/
    for (int i = 0; i < size; i++) {
        for (int m = 0; m < polysize; m++) {
            getNextRandValue(m, mod, polynomials, rand);
            mpz_div_ui(rand, rand, combinations);
            modMul(temp, rand, temp3[m]);
            modAdd(results[i], results[i], temp);
        }
    }
    /*********** Free the memory **************/
    for (int i = 0; i < polysize; i++) {
        mpz_clear(temp1[i]);
        mpz_clear(temp2[i]);
        mpz_clear(temp3[i]);
    }
    free(temp1);
    free(temp2);
    free(temp3);

    mpz_clear(zero);
    mpz_clear(rand);
}

void SecretShare::generateRandValue(int nodeID, mpz_t mod, int size, mpz_t *results, int threadID) {
    if (threadID == -1) {
        generateRandValue(nodeID, mod, size, results);
        return;
    }
    std::map<std::string, std::vector<int>>::iterator it;
    mpz_t zero, rand, temp;
    int polysize = polynomials.size();
    mpz_t *temp1 = (mpz_t *)malloc(sizeof(mpz_t) * polysize);
    mpz_t *temp2 = (mpz_t *)malloc(sizeof(mpz_t) * polysize);
    mpz_t *temp3 = (mpz_t *)malloc(sizeof(mpz_t) * polysize);
    mpz_init(zero);
    mpz_init(rand);
    mpz_init(temp);
    mpz_set_ui(zero, 0);
    for (int i = 0; i < polysize; i++) {
        mpz_init(temp1[i]);
        mpz_init(temp2[i]);
        mpz_init(temp3[i]);
    }

    int index = 0;
    std::vector<long> polyOutput;
    std::vector<long> denominator;
    long long combinations = nChoosek(getPeers(), getThreshold());

    if (rand_isFirst_thread[threadID] == 0)
        getNextRandValue(0, 0, polynomials, NULL, threadID);

    /*************** Evaluate the polynomials on points ******************/
    for (it = polynomials.begin(); it != polynomials.end(); it++) {
        std::vector<int> polys = (*it).second;
        denominator.push_back(polys[polys.size() - 1]);
        polyOutput.push_back(computePolynomials((*it).second, nodeID));
    }

    for (int m = 0; m < polysize; m++) {
        mpz_set_si(temp2[m], denominator[m]);
        mpz_set_si(temp3[m], polyOutput[m]);
        if (denominator[m] < 0)
            modAdd(temp2[m], zero, temp2[m]);
        if (polyOutput[m] < 0)
            modAdd(temp3[m], zero, temp3[m]);
        modInv(temp1[m], temp2[m]);
        modMul(temp3[m], temp3[m], temp1[m]);
    }

    /************* Generate the random values ******************/
    for (int i = 0; i < size; i++) {
        for (int m = 0; m < polysize; m++) {
            getNextRandValue(m, mod, polynomials, rand, threadID);
            mpz_div_ui(rand, rand, combinations);
            modMul(temp, rand, temp3[m]);
            modAdd(results[i], results[i], temp);
        }
    }
    /*********** Free the memory **************/
    for (int i = 0; i < polysize; i++) {
        mpz_clear(temp1[i]);
        mpz_clear(temp2[i]);
        mpz_clear(temp3[i]);
    }
    free(temp1);
    free(temp2);
    free(temp3);

    mpz_clear(zero);
    mpz_clear(rand);
    /* When multiple compute nodes attempt to access polynomials at the same time, there may exisit a conflict that results in the memory problem */
    /*for(unsigned int i = 0; i < 2 ; i++){
        polys = polynomials.find(oldseed[i])->second;
        polynomials.erase(oldseed[i]);
        polynomials.insert(std::pair<uint64_t, std::vector<int> >(newseed[i], polys));
    }*/
}

void SecretShare::getNextRandValue(int id, int bits, std::map<std::string, std::vector<int>> polynomials, mpz_t value) {
    // if (rand_isFirst == 0) {
    //     std::map<std::string, std::vector<int>>::iterator it;
    //     mpz_t *keys = (mpz_t *)malloc(sizeof(mpz_t) * polynomials.size());
    //     int k = 0;
    //     for (it = polynomials.begin(); it != polynomials.end(); it++) {
    //         mpz_init(keys[k]);
    //         mpz_set_str(keys[k], ((*it).first).c_str(), 10);
    //         k++;
    //     }
    //     rstates = (gmp_randstate_t *)malloc(sizeof(gmp_randstate_t) * polynomials.size());
    //     for (int i = 0; i < polynomials.size(); i++) {
    //         gmp_randinit_default(rstates[i]);
    //         gmp_randseed(rstates[i], keys[i]);
    //     }
    //     rand_isFirst = 1;
    // } else
        mpz_urandomb(value, rstates[id], bits);
}

void SecretShare::getNextRandValue(int id, int bits, std::map<std::string, std::vector<int>> polynomials, mpz_t value, int threadID) {
    if (threadID == -1) {
        getNextRandValue(id, bits, polynomials, value);
        return;
    }
    if (rand_isFirst_thread[threadID] == 0) {
        std::map<std::string, std::vector<int>>::iterator it;
        mpz_t *keys = (mpz_t *)malloc(sizeof(mpz_t) * polynomials.size());
        int k = 0;
        for (it = polynomials.begin(); it != polynomials.end(); it++) {
            mpz_init(keys[k]);
            mpz_set_str(keys[k], ((*it).first).c_str(), 10);
            k++;
        }
        for (int i = 0; i < polynomials.size(); i++) {
            gmp_randinit_default(rstates_thread[i][threadID]);
            gmp_randseed(rstates_thread[i][threadID], keys[i]);
        }
        rand_isFirst_thread[threadID] = 1;
    } else
        mpz_urandomb(value, rstates_thread[id][threadID], bits);
}

void SecretShare::getNextRandValue(int id, mpz_t mod, std::map<std::string, std::vector<int>> polynomials, mpz_t value) {
    // if (rand_isFirst == 0) {
    //     std::map<std::string, std::vector<int>>::iterator it;
    //     mpz_t *keys = (mpz_t *)malloc(sizeof(mpz_t) * polynomials.size());
    //     int k = 0;
    //     for (it = polynomials.begin(); it != polynomials.end(); it++) {
    //         mpz_init(keys[k]);
    //         mpz_set_str(keys[k], ((*it).first).c_str(), 10);
    //         k++;
    //     }
    //     rstates = (gmp_randstate_t *)malloc(sizeof(gmp_randstate_t) * polynomials.size());
    //     for (int i = 0; i < polynomials.size(); i++) {
    //         gmp_randinit_default(rstates[i]);
    //         gmp_randseed(rstates[i], keys[i]);
    //     }
    //     rand_isFirst = 1;
    // } else
        mpz_urandomm(value, rstates[id], mod);
}

void SecretShare::getNextRandValue(int id, mpz_t mod, std::map<std::string, std::vector<int>> polynomials, mpz_t value, int threadID) {
    if (threadID == -1) {
        getNextRandValue(id, mod, polynomials, value);
        return;
    }
    if (rand_isFirst_thread[threadID] == 0) {
        std::map<std::string, std::vector<int>>::iterator it;
        mpz_t *keys = (mpz_t *)malloc(sizeof(mpz_t) * polynomials.size());
        int k = 0;
        for (it = polynomials.begin(); it != polynomials.end(); it++) {
            mpz_init(keys[k]);
            mpz_set_str(keys[k], ((*it).first).c_str(), 10);
            k++;
        }
        for (int i = 0; i < polynomials.size(); i++) {
            gmp_randinit_default(rstates_thread[i][threadID]);
            gmp_randseed(rstates_thread[i][threadID], keys[i]);
        }
        rand_isFirst_thread[threadID] = 1;
    } else
        mpz_urandomm(value, rstates_thread[id][threadID], mod);
}
