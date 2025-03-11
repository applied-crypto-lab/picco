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

#include "SecretShare.h"
#include "NodeNetwork.h"

using std::cout;
using std::endl;
gmp_randstate_t *SecretShare::rstatesMult;
gmp_randstate_t **SecretShare::rstates_thread_mult;
int *SecretShare::rand_isFirst_thread_mult = NULL;

gmp_randstate_t *SecretShare::rstates;
gmp_randstate_t **SecretShare::rstates_thread;
int *SecretShare::rand_isFirst_thread = NULL;

pthread_mutex_t SecretShare::mutex;
/*
 * the constructor receives:
 * p - the number of computational parties or peers,
 * t - secret sharing threshold,
 * mod - modulus of the field/computation,
 * id - the ID of the current party, and
 * keys - an array of 16-byte keys of size threshold, which are used as PRG seeds in optimized multiplication.
 */
SecretShare::SecretShare(unsigned int p, unsigned int t, mpz_t mod, unsigned int id, unsigned int _numThreads, unsigned char *keys[KEYSIZE], std::map<std::string, vector<int>> _polynomials) {

    // keys are properly passed using the getter function, so they are now private members of NodeNetwork
    peers = p;
    threshold = t;
    myID = id;

    polynomials = _polynomials; // doesnt even really need to be a map
    numThreads = _numThreads;
    // for (size_t i = 0; i < 2 * threshold; i++) {
    //     print_hexa(keys[i], KEYSIZE);
    // }
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
        sendToIDs[i] = ((myID + i) % peers) + 1;
        if ((threshold - i) >= myID)
            recvFromIDs[i] = myID + peers - threshold + i;
        else
            recvFromIDs[i] = myID - threshold + i;
    }

    mult_keys = new unsigned char *[2 * threshold];
    for (size_t i = 0; i < 2 * threshold; i++) {
        mult_keys[i] = new unsigned char[KEYSIZE];
        memcpy(mult_keys[i], keys[i], KEYSIZE);
    }
    // mult_keys = keys;

    for (int i = 0; i < threshold; i++) {
        multIndices[i] = sendToIDs[i];
        multIndices[threshold + i + 1] = recvFromIDs[i];
    }
    multIndices[threshold] = id;

    computeSharingMatrix();
    computeLagrangeWeights();

    randInit(keys);

    initCoef();
}

void SecretShare::randInit(unsigned char *keys[KEYSIZE]) {

    gmp_randinit_default(rstate);

    mpz_t seed;
    mpz_init(seed);
    unsigned char temp_key[KEYSIZE];
    gmp_randinit_default(rstate_mine);
    if (!RAND_bytes(temp_key, KEYSIZE))
        printf("Key, iv generation error\n");
    mpz_import(seed, KEYSIZE, 1, sizeof(temp_key[0]), 0, 0, temp_key);
    gmp_randseed(rstate_mine, seed);

    // used for mult, could probably be moved into randInit but we'd need to pass keys along

    rstatesMult = (gmp_randstate_t *)malloc(sizeof(gmp_randstate_t) * (2 * threshold));
    for (int i = 0; i < 2 * threshold; i++) {
        gmp_randinit_default(rstatesMult[i]);
        mpz_import(seed, KEYSIZE, 1, sizeof(keys[i][0]), 0, 0, keys[i]);
        gmp_randseed(rstatesMult[i], seed);
    }

    // initialize PRGs for (non-)interactive random generation
    pthread_mutex_init(&mutex, NULL);
    rand_isFirst_thread = (int *)malloc(sizeof(int) * numThreads);
    rand_isFirst_thread_mult = (int *)malloc(sizeof(int) * numThreads);
    for (int i = 0; i < numThreads; i++) {
        rand_isFirst_thread[i] = 0;
        rand_isFirst_thread_mult[i] = 0;
    }

    rstates_thread = (gmp_randstate_t **)malloc(sizeof(gmp_randstate_t *) * polynomials.size());
    for (int i = 0; i < polynomials.size(); i++) {
        rstates_thread[i] = (gmp_randstate_t *)malloc(sizeof(gmp_randstate_t) * numThreads);
    }

    rstates_thread_mult = (gmp_randstate_t **)malloc(sizeof(gmp_randstate_t *) * (2 * threshold));
    for (int i = 0; i < 2 * threshold; i++) {
        rstates_thread_mult[i] = (gmp_randstate_t *)malloc(sizeof(gmp_randstate_t) * numThreads);
    }

    std::map<std::string, vector<int>>::iterator it;
    mpz_t *temp_keys = (mpz_t *)malloc(sizeof(mpz_t) * polynomials.size());
    int k = 0;
    for (it = polynomials.begin(); it != polynomials.end(); it++) {
        mpz_init(temp_keys[k]);
        mpz_set_str(temp_keys[k], ((*it).first).c_str(), BASE_10);
        k++;
    }
    rstates = (gmp_randstate_t *)malloc(sizeof(gmp_randstate_t) * polynomials.size());
    for (int i = 0; i < polynomials.size(); i++) {
        gmp_randinit_default(rstates[i]);
        gmp_randseed(rstates[i], temp_keys[i]);
    }

    // computing polynomial evaluation on points once
    int polysize = polynomials.size();
    mpz_t zero;
    mpz_init(zero);
    mpz_set_ui(zero, 0);

    poly_evaluation = (mpz_t *)malloc(sizeof(mpz_t) * polysize);
    mpz_t *temp1 = (mpz_t *)malloc(sizeof(mpz_t) * polysize);
    mpz_t *temp2 = (mpz_t *)malloc(sizeof(mpz_t) * polysize);

    for (int i = 0; i < polysize; i++) {
        mpz_init(poly_evaluation[i]);
        mpz_init(temp1[i]);
        mpz_init(temp2[i]);
    }
    vector<long> polyOutput;
    vector<long> denominator;

    for (it = polynomials.begin(); it != polynomials.end(); it++) {
        vector<int> polys = (*it).second;
        denominator.push_back(polys[polys.size() - 1]);
        polyOutput.push_back(computePolynomials((*it).second, myID));
    }

    for (int m = 0; m < polysize; m++) {
        mpz_set_si(temp2[m], denominator[m]);
        mpz_set_si(poly_evaluation[m], polyOutput[m]);
        if (denominator[m] < 0)
            modAdd(temp2[m], zero, temp2[m]);
        if (polyOutput[m] < 0)
            modAdd(poly_evaluation[m], zero, poly_evaluation[m]);
        modInv(temp1[m], temp2[m]);
        modMul(poly_evaluation[m], poly_evaluation[m], temp1[m]);
    }

    for (int i = 0; i < polysize; i++) {
        mpz_clear(temp1[i]);
        mpz_clear(temp2[i]);
    }
    free(temp1);
    free(temp2);
    mpz_clear(zero);
    mpz_clear(seed);
}

void SecretShare::randInit_thread_mult(int threadID) {
    // std::cout << "RAND INIT THREAD MULT threadID = " << threadID << std::endl;
    if (threadID == -1) {
        return;
    }
    if (rand_isFirst_thread_mult[threadID] == 0) {
        for (size_t i = 0; i < 2 * threshold; i++) {
            print_hexa(mult_keys[i], KEYSIZE);
        }
        mpz_t seed;
        mpz_init(seed);
        for (int threadID = 0; threadID < numThreads; threadID++) {
            for (int i = 0; i < 2 * threshold; i++) {
                gmp_randinit_default(rstates_thread_mult[i][threadID]);
                mpz_import(seed, KEYSIZE, 1, sizeof(mult_keys[i][0]), 0, 0, mult_keys[i]);
                gmp_randseed(rstates_thread_mult[i][threadID], seed);
            }
        }
    }
}

void SecretShare::randInit_thread(int threadID) {
    if (threadID == -1) {
        return;
    }
    if (rand_isFirst_thread[threadID] == 0) {
        std::map<std::string, vector<int>>::iterator it;
        mpz_t *temp_keys = (mpz_t *)malloc(sizeof(mpz_t) * polynomials.size());
        int k = 0;
        for (it = polynomials.begin(); it != polynomials.end(); it++) {
            mpz_init(temp_keys[k]);
            mpz_set_str(temp_keys[k], ((*it).first).c_str(), BASE_10);
            k++;
        }
        for (int i = 0; i < polynomials.size(); i++) {
            gmp_randinit_default(rstates_thread[i][threadID]);
            gmp_randseed(rstates_thread[i][threadID], temp_keys[i]);
        }
    }
}

// initializes coefficients for symmteric function evaluation (used in EQZ)
// starts with m = 2, goes up to COEFF_BOUND-1 sets of coefs
// each row is of size (m+1), which corresponds to
// the number of coef's for a degree m poly
// example: legend-- [index][size]
// coef[0][3] (m = 2)
// coef[1][4] (m = 4)
// coef[2][5] (m = 5)
// ...
// coef[COEFF_BOUND - 1][COEFF_BOUND + 2 + 1] (m = COEFF_BOUND + 2 + 1)
void SecretShare::initCoef() {
    coef = (mpz_t **)malloc(sizeof(mpz_t *) * COEFF_BOUND);
    for (int i = 0; i < COEFF_BOUND; ++i) {
        // printf("dim coef[%i][%i]\n", i, COEFF_OFFSET + i + 1);
        coef[i] = (mpz_t *)malloc(sizeof(mpz_t) * (COEFF_OFFSET + i + 1)); // 2 is the offset of where we start, 1 is for poly degree
        for (int j = 0; j < (COEFF_OFFSET + i + 1); ++j)
            mpz_init(coef[i][j]);
    }
    mpz_t temp1, temp2, zero;
    mpz_init(temp1);
    mpz_init(temp2);
    mpz_init_set_ui(zero, 0);

    for (size_t m = 0; m < COEFF_BOUND; m++) {
        uint inv_term;
        vector<int> ret_coef = generateCoef(COEFF_OFFSET + m, inv_term);
        // std::cout << " m + COEFF_OFFSET =  " << m + COEFF_OFFSET << "     ";
        // for (size_t i = 0; i < ret_coef.size(); i++) {
        //     std::cout << ", " << ret_coef.at(i);
        // }
        // std::cout << " --- inv " << inv_term << std::endl;
        // printf("\n");
        // setting the divisor
        mpz_set_ui(temp1, inv_term);
        modInv(temp1, temp1);

        for (int i = 0; i < ret_coef.size(); i++) {
            uint tmp = abs(ret_coef.at(i)); // making sure this is positive, will deal with negatives below
            mpz_set_ui(temp2, tmp);
            mpz_set(coef[m][i], temp1);
            modMul(coef[m][i], coef[m][i], temp2);
            // if the coefficient is negative, flip the sign
            if (ret_coef.at(i) < 0) {
                modSub(coef[m][i], zero, coef[m][i]);
            }
            // gmp_printf("(%i, %Zd) ", ret_coef.at(i), coef[m][i]);

            // gmp_printf("");
        }
        // printf("\n");
    }
    mpz_clear(zero);
    mpz_clear(temp1);
    mpz_clear(temp2);
}

// returns the appropriate set of coefs to use based on the input K value
// thows errors if K is too large (resolve by increaseing COEFF_BOUND), or K <= 1 which means m < 0 which is problematic
int SecretShare::getCoefIndex(int K) {
    try {
        if (ceil(log2(K)) > COEFF_BOUND) {
            throw std::runtime_error("K is too large. Increase COEFF_BOUND and recompile. ");
        }
        if (K <= 1) {
            throw std::runtime_error("K < 2, meaning m < 1. Something went very wrong in configuration.");
        }

    } catch (std::exception &e) {
        std::cout << "An exception was caught: " << e.what() << "\n";
        throw std::exception();
    }
    return ceil(log2(K)) - COEFF_OFFSET;
}

vector<int> generateCoef(int m, uint &inv_term) {
    vector<vector<int>> polys;
    vector<int> denoms;
    vector<int> result(m + 1, 0); // used as accumulator for later

    for (int i = 2; i <= m + 1; i++) {
        vector<int> poly_accum{1};
        int denom = 1;
        for (int j = 1; j <= m + 1; j++) {
            if (i != j) {
                vector<int> poly_tmp{1, (-1) * j};
                poly_accum = multiply_poly(poly_accum, poly_tmp);
                denom *= ((-1) * j + i);
            }
        }
        for (int j = 0; j < poly_accum.size(); j++) {
            poly_accum.at(j) *= sign(denom);
        }
        denom *= sign(denom); // ensuring all denoms are positive
        polys.push_back(poly_accum);
        denoms.push_back(denom);
    }
    inv_term = std::accumulate(denoms.begin(), denoms.end(), 1, lcm);
    int constant;
    for (size_t i = 0; i < polys.size(); i++) {
        constant = inv_term / denoms.at(i);
        for (int j = 0; j < polys.at(i).size(); j++) {
            polys.at(i).at(j) *= constant;
        }
        for (int j = 0; j < result.size(); j++) {
            result.at(j) += polys.at(i).at(j); // element-wise addition
        }
    }

    return result;
}

vector<int> multiply_poly(vector<int> A, vector<int> B) {
    int m = A.size();
    int n = B.size();
    vector<int> prod(m + n - 1, 0);
    // Multiply two polynomials term by term
    // Take ever term of first polynomial
    for (int i = 0; i < m; i++) {
        // Multiply the current term of first polynomial
        // with every term of second polynomial.
        for (int j = 0; j < n; j++)
            prod[i + j] += A[i] * B[j];
    }
    return prod;
}

uint gcd(uint a, uint b) {
    for (;;) {
        if (a == 0)
            return b;
        b %= a;
        if (b == 0)
            return a;
        a %= b;
    }
}

uint lcm(uint a, uint b) {
    uint temp = gcd(a, b);

    return temp ? (a / temp * b) : 0;
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

void SecretShare::modMul(mpz_t *result, mpz_t *x, int *y, int size) {
    mpz_t *ytmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++) {
        mpz_init_set_si(ytmp[i], y[i]);
        modMul(result[i], ytmp[i], x[i]);
    }
    for (int i = 0; i < size; i++)
        mpz_clear(ytmp[i]);
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

void SecretShare::modSub(mpz_t *result, mpz_t *x, int *y, int size) {
    mpz_t *ytmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set_si(ytmp[i], y[i]);
    modSub(result, x, ytmp, size);
    for (int i = 0; i < size; i++)
        mpz_clear(ytmp[i]);
}
void SecretShare::modSub(mpz_t *result, int *x, mpz_t *y, int size) {
    mpz_t *xtmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set_si(xtmp[i], x[i]);
    modSub(result, xtmp, y, size);
    for (int i = 0; i < size; i++)
        mpz_clear(xtmp[i]);
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

void SecretShare::modPow2(mpz_t result, mpz_t exponent) {
    mpz_t value, base;
    mpz_init_set_ui(base, 2);
    mpz_init_set(value, exponent);
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
 * 3) evaluating a polynomial on threshold+1 points at indices stored in sendToIDs as well as myID, where the polynomial is encoded using threshold+1 values at indices stored in recvFromIDs and point 0.
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
            mpz_set_ui(t2, myID);
        else
            mpz_set_ui(t2, recvFromIDs[i]);
        for (l = 0; l < threshold + 1; l++) {
            if (l != i) {
                if (l == threshold)
                    mpz_set_ui(t1, myID);
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

    // weights at row i (i.e., in largrangeWeightsMult[i]) correspond to polynomial evaluation on point sendToIDs[i] and row threshold corresponds to evaluation on point myID
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
                mpz_set_ui(t2, myID);
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
    // sanitizing destionation
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
 * the shares are expected to correspond to points at indices stored in recvFromIDs as well as myID.
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
                index = myID - 1;
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
 * ouput result is also of size (threshold+1)*size, a row stores size shares for a single party (parties in sendToIDs and myID).
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
        mpz_mul_ui(data[myID - 1][i], coefficient, myID); // for id
        mpz_add(data[myID - 1][i], data[myID - 1][i], temp[i]);
        mpz_mod(data[myID - 1][i], data[myID - 1][i], fieldSize);
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
void SecretShare::PRG_thread(mpz_t **output, uint size, uint start_ind, int threadID) {
    if (threadID == -1) {
        PRG(output, size, start_ind);
    } else {
        if (rand_isFirst_thread_mult[threadID] == 0) {
            randInit_thread_mult(threadID);
            rand_isFirst_thread_mult[threadID] = 1;
        }
        for (int i = 0; i < threshold; i++) {
            for (int j = 0; j < size; j++) {
                mpz_urandomm(output[i][j], rstates_thread_mult[i + start_ind][threadID], fieldSize);
            }
        }
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

void SecretShare::getCoef(int id) {
    myID = id;
    id_p1 = (myID + 1) % (peers + 1);
    if (id_p1 == 0)
        id_p1 = 1;

    id_m1 = (myID - 1) % (peers + 1);
    if (id_m1 == 0)
        id_m1 = peers;

    mpz_init(id_p1_inv);
    mpz_set_ui(id_p1_inv, id_p1);
    mpz_invert(id_p1_inv, id_p1_inv, fieldSize);
}

// random generation functions, was previously in Random.cpp

int SecretShare::computePolynomials(vector<int> polys, int point) {
    int result = 0;
    for (unsigned int i = 0; i < polys.size(); i++)
        result += pow(point, (polys.size() - 1 - i)) * polys[i];
    return result;
}

void SecretShare::generateRandValue(int bits, int size, mpz_t *results) {
    mpz_t rand, temp;
    int polysize = polynomials.size();
    mpz_init(rand);
    mpz_init(temp);

    /************* Generate the random values ******************/
    for (int i = 0; i < size; i++) {
        for (int m = 0; m < polysize; m++) {
            // Generate a uniformly distributed random integer in the range 0 to 2*bits-1, inclusive.
            mpz_urandomb(rand, rstates[m], bits);

            // mpz_div_ui(rand, rand, combinations);
            modMul(temp, rand, poly_evaluation[m]);
            modAdd(results[i], results[i], temp);
        }
    }
    mpz_clear(rand);
    mpz_clear(temp);
}

void SecretShare::generateRandValue(int bits, int size, mpz_t *results, int threadID) {
    if (threadID == -1) {
        generateRandValue(bits, size, results);
        return;
    }
    mpz_t rand, temp;
    int polysize = polynomials.size();
    mpz_init(rand);
    mpz_init(temp);

    if (rand_isFirst_thread[threadID] == 0) {
        randInit_thread(threadID);
        rand_isFirst_thread[threadID] = 1;
    }

    /************* Generate the random values ******************/
    for (int i = 0; i < size; i++) {
        for (int m = 0; m < polysize; m++) {
            // getNextRandValue(m, bits, polynomials, rand, threadID);
            mpz_urandomb(rand, rstates_thread[m][threadID], bits);

            // mpz_div_ui(rand, rand, combinations);
            modMul(temp, rand, poly_evaluation[m]);
            modAdd(results[i], results[i], temp);
        }
    }
    mpz_clear(rand);
    mpz_clear(temp);
}

void SecretShare::generateRandValue(mpz_t mod, int size, mpz_t *results) {
    mpz_t rand, temp;
    int polysize = polynomials.size();
    mpz_init(rand);
    mpz_init(temp);
    /************* Generate the random values ******************/
    for (int i = 0; i < size; i++) {
        for (int m = 0; m < polysize; m++) {
            // Generate a uniform random integer in the range 0 to mod-1, inclusive
            mpz_urandomm(rand, rstates[m], mod);
            // mpz_div_ui(rand, rand, combinations);not needed
            modMul(temp, rand, poly_evaluation[m]);
            modAdd(results[i], results[i], temp);
        }
    }
    mpz_clear(rand);
    mpz_clear(temp);
}

void SecretShare::generateRandValue(mpz_t mod, int size, mpz_t *results, int threadID) {
    if (threadID == -1) {
        generateRandValue(mod, size, results);
        return;
    }
    mpz_t rand, temp;
    int polysize = polynomials.size();
    mpz_init(rand);
    mpz_init(temp);

    if (rand_isFirst_thread[threadID] == 0) {
        randInit_thread(threadID);
        rand_isFirst_thread[threadID] = 1;
    }

    // evaluation done in rand_init
    /************* Generate the random values ******************/
    for (int i = 0; i < size; i++) {
        for (int m = 0; m < polysize; m++) {
            // getNextRandValue(m, mod, polynomials, rand, threadID);
            mpz_urandomm(rand, rstates_thread[m][threadID], mod);
            // mpz_div_ui(rand, rand, combinations);
            modMul(temp, rand, poly_evaluation[m]);
            modAdd(results[i], results[i], temp);
        }
    }
    mpz_clear(rand);
    mpz_clear(temp);
    /* When multiple compute nodes attempt to access polynomials at the same time, there may exisit a conflict that results in the memory problem */
    /*for(unsigned int i = 0; i < 2 ; i++){
        polys = polynomials.find(oldseed[i])->second;
        polynomials.erase(oldseed[i]);
        polynomials.insert(std::pair<uint64_t, vector<int> >(newseed[i], polys));
    }*/
}

void SecretShare::PRZS(mpz_t mod, int size, mpz_t *results) {
    mpz_t temp, temp2;
    mpz_init(temp);
    mpz_init(temp2);

    int polysize = polynomials.size();
    // mpz_init(rand);
    mpz_t *rand = (mpz_t *)malloc(sizeof(mpz_t) * threshold);
    for (int i = 0; i < threshold; i++) {
        mpz_init(rand[i]);
    }
    int temp_int;
    /************* Generate the random values ******************/
    for (int i = 0; i < size; i++) {
        for (int m = 0; m < polysize; m++) {
            mpz_set_ui(temp, 0);
            // Generate t random values from the PRG/PRF
            for (int j = 0; j < threshold; j++) {
                mpz_urandomm(rand[j], rstates[m], mod);
                temp_int = pow(myID, j + 1);
                // printf("%i^%i = %i\n", myID, j+1, temp_int);
                modMul(temp2, rand[j], temp_int); // i^j * rand[j]
                modAdd(temp, temp, temp2);        // sum_{j=1}^{t} (i^j * rand[j])
            }
            modMul(temp, temp, poly_evaluation[m]); // (sum_{j=1}^{t} i^j * rand[j]) * f(m)
            modAdd(results[i], results[i], temp);
        }
    }
    for (int i = 0; i < threshold; i++) {
        mpz_clear(rand[i]);
    }
    free(rand);
    mpz_clear(temp);
    mpz_clear(temp2);
}

// to do:
// convert inputs from const char* to std::strings, no reason to use C functions here.
// check correctness
// different expected size based on delim "=" or ","
// if "=", check if result.size != 2
// if ",", check if result.size != expected_size
// throw error, catch in calling function, throw again and pass up the call stack (prepending function name in the process to know exactly where )
// depracated, to be removed
std::vector<std::string> SecretShare::splitfunc(const char *str, const char *delim) {
    char *saveptr;
    char *token = strtok_r((char *)str, delim, &saveptr);
    std::vector<std::string> result;
    while (token != NULL) {
        result.push_back(token);
        token = strtok_r(NULL, delim, &saveptr);
    }
    return result;
}

bool SecretShare::is_int(const std::string &str) {
    return str.find_first_not_of("0123456789") == std::string::npos;
}
bool SecretShare::is_float(const std::string &str) {
    return str.find_first_not_of(".0123456789") == std::string::npos;
}

// modern, robust C++ version of the above function
// expected_size is an OPTIONAL ARGUMENT (default 0)
std::vector<std::string> SecretShare::split(const std::string s, const std::string delimiter, int expected_size) {
    try {
        if (s.empty())
            throw std::runtime_error("Empty string passed to split.");
        size_t pos_start = 0, pos_end, delim_len = delimiter.length();
        std::string token;
        std::vector<std::string> result;
        while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
            token = s.substr(pos_start, pos_end - pos_start);
            pos_start = pos_end + delim_len;
            result.push_back(token);
        }
        result.push_back(s.substr(pos_start));
        if (delimiter == "=") { // should only be two values in the output, the variable name (result[0]) and a string of shares "s_0,s_1,...,s_n" (result[1])
            if (result.size() != 2)
                throw std::runtime_error("Encountered an unexpected number of entries than expected, or string is not well-formed.\nInput provided:\n" + s);
        } else if (delimiter == ",") {
            for (auto &var : result)
                var = std::regex_replace(var, std::regex("^ +| +$|( ) +"), "$1"); // stripping leading/trailing whitespace from string

            // should only be `expected_size` number of entries in result.size(): <"s_0", "s_1", ..., "s_n">
            if (expected_size <= 0)
                throw std::runtime_error("Attempting to split a string with either an unknown expected_size, or expected_size <= 0");
            if ((result.size() != expected_size) || (((result.size() == (expected_size))) && result.back().empty())) {
                int offset = result.back().empty() ? 1 : 0; // used for accurate error reporting when the last element is empty, but the comma is present
                throw std::runtime_error("Encountered an unexpected number of shares than expected.\nInput provided:\n" + s + "\nExpected " + std::to_string(expected_size) + " value(s), but found " + std::to_string(result.size() - offset) + " value(s)");
            }
        } else
            std::cout << "Delimter other than  \'=\' or \',\' was passed. I'm not checking the expected length!" << std::endl;
        return result;
    } catch (const std::runtime_error &ex) {
        //  can throw from interior function, as long as we catch elsewhere
        std::string error(ex.what()); // capturing the message from the exception to concatenate below
        throw std::runtime_error("[splitfunc] " + error);
    }
}

// input public int
void SecretShare::ss_input(int id, int *var, std::string type, std::ifstream *inputStreams) {
    try {
        std::string line;
        std::vector<std::string> tokens;
        std::getline(inputStreams[id - 1], line);
        tokens = split(line, "=");
        tokens = split(tokens[1], ",", 1); // done for correctness testing for single inputs  (non-array only)
        if (!is_int(tokens.front()))
            throw std::runtime_error("Non-integer input provided: " + tokens.front());
        // cout << "ss_input, public int :" << tokens.front() << endl;
        *var = stoi(tokens.front()); // discards any whitespace, non-numerical characters automatically
    } catch (const std::runtime_error &ex) {
        // capturing error message from original throw
        std::string error(ex.what());
        // appending to new throw, then re-throwing
        throw std::runtime_error("[ss_input, public int] stream from party " + std::to_string(id) + ": " + error);
    }
}

// input private int
void SecretShare::ss_input(int id, mpz_t *var, std::string type, std::ifstream *inputStreams) {
    try {
        std::string line;
        std::vector<std::string> tokens;
        std::getline(inputStreams[id - 1], line);
        tokens = split(line, "=");
        tokens = split(tokens[1], ",", 1); // done for correctness testing for single inputs  (non-array only)
        if (!is_int(tokens.front()))
            throw std::runtime_error("Non-integer input provided: " + tokens.front());
        // cout << "ss_input, private int :" << tokens.front() << endl;
        mpz_set_str(*var, tokens.front().c_str(), BASE_10);
    } catch (const std::runtime_error &ex) {
        // capturing error message from original throw
        std::string error(ex.what());
        // appending to new throw, then re-throwing
        throw std::runtime_error("[ss_input, private int] stream from party " + std::to_string(id) + ": " + error);
    }
}

// input public float
void SecretShare::ss_input(int id, float *var, std::string type, std::ifstream *inputStreams) {
    try {
        std::string line;
        std::vector<std::string> tokens;
        std::getline(inputStreams[id - 1], line);
        tokens = split(line, "=");
        tokens = split(tokens[1], ",", 1); // done for correctness testing for single inputs (non-array only)
        if (!is_float(tokens.front()))
            throw std::runtime_error("Non-float input provided: " + tokens.front());
        // cout << "ss_input, public float :" << tokens.front() << endl;
        *var = stof(tokens.front()); // discards any whitespace, non-numerical characters automatically
    } catch (const std::runtime_error &ex) {
        // capturing error message from original throw
        std::string error(ex.what());
        // appending to new throw, then re-throwing
        throw std::runtime_error("[ss_input, public float] stream from party " + std::to_string(id) + ": " + error);
    }
}

// input private float
void SecretShare::ss_input(int id, mpz_t **var, std::string type, std::ifstream *inputStreams) {
    try {
        std::string line;
        std::vector<std::string> tokens;
        std::vector<std::string> temp;
        std::getline(inputStreams[id - 1], line);
        tokens = split(line, "=");
        temp = split(tokens[1], ",", 4);
        // for (auto v : temp)
        //     cout << "ss_input, private float :" << v << endl;
        for (int i = 0; i < 4; i++) {
            if (!is_int(temp[i]))
                throw std::runtime_error("Non-integer input provided: " + temp[i]);
            mpz_set_str((*var)[i], temp[i].c_str(), BASE_10);
        }
    } catch (const std::runtime_error &ex) {
        // capturing error message from original throw
        std::string error(ex.what());
        // appending to new throw, then re-throwing
        throw std::runtime_error("[ss_input, private float] stream from party " + std::to_string(id) + ": " + error);
    }
}

// one-dimensional PRIVATE int array I/O
void SecretShare::ss_input(int id, mpz_t *var, int size, std::string type, std::ifstream *inputStreams) {
    try {
        std::string line;
        std::vector<std::string> tokens;
        std::vector<std::string> temp;
        std::getline(inputStreams[id - 1], line);
        tokens = split(line, "=");
        temp = split(tokens[1], ",", size);
        for (int i = 0; i < size; i++) {
            if (!is_int(temp[i]))
                throw std::runtime_error("Non-integer input provided: " + temp[i]);
            mpz_set_str(var[i], temp[i].c_str(), BASE_10);
        }

    } catch (const std::runtime_error &ex) {
        // capturing error message from original throw
        std::string error(ex.what());
        // appending to new throw, then re-throwing
        throw std::runtime_error("[ss_input, private int array] stream from party " + std::to_string(id) + ": " + error);
    }
}

// one-dimensional PUBLIC int array I/O
void SecretShare::ss_input(int id, int *var, int size, std::string type, std::ifstream *inputStreams) {
    try {
        std::string line;
        std::vector<std::string> tokens;
        std::vector<std::string> temp;
        std::getline(inputStreams[id - 1], line);
        tokens = split(line, "=");
        temp = split(tokens[1], ",", size);
        for (int i = 0; i < size; i++) {
            if (!is_int(temp[i]))
                throw std::runtime_error("Non-integer input provided: " + temp[i]);
            var[i] = atoi(temp[i].c_str());
        }

    } catch (const std::runtime_error &ex) {
        // capturing error message from original throw
        std::string error(ex.what());
        // appending to new throw, then re-throwing
        throw std::runtime_error("[ss_input, public int array] stream from party " + std::to_string(id) + ": " + error);
    }
}

// one-dimensional PRIVATE float array I/O
// the difference between this function and the other array inputs is that it can't directly check if there are "size" things to split
// it just calls getline() size times
// and if it encounters something that it doesnt expect (array of size 4), it will throw an error
void SecretShare::ss_input(int id, mpz_t **var, int size, std::string type, std::ifstream *inputStreams) {
    try {
        std::string line;
        std::vector<std::string> tokens;
        std::vector<std::string> temp;
        for (int i = 0; i < size; i++) {
            std::getline(inputStreams[id - 1], line);
            tokens = split(line, "=");
            temp = split(tokens[1], ",", 4);
            for (int j = 0; j < 4; j++) {
                if (!is_int(temp[j]))
                    throw std::runtime_error("Non-integer input provided: " + temp[j]);
                mpz_set_str(var[i][j], temp[j].c_str(), BASE_10);
            }
        }
    } catch (const std::runtime_error &ex) {
        // capturing error message from original throw
        std::string error(ex.what());
        // appending to new throw, then re-throwing
        throw std::runtime_error("[ss_input, private float array] stream from party " + std::to_string(id) + ": " + error);
    }
}

// one-dimensional PUBLIC float array I/O
void SecretShare::ss_input(int id, float *var, int size, std::string type, std::ifstream *inputStreams) {
    try {
        std::string line;
        std::vector<std::string> tokens;
        std::vector<std::string> temp;
        std::getline(inputStreams[id - 1], line);
        tokens = split(line, "=");
        temp = split(tokens[1], ",", size);
        for (int i = 0; i < size; i++) {
            if (!is_float(temp[i]))
                throw std::runtime_error("Non-float input provided: " + temp[i]);
            var[i] = atof(temp[i].c_str());
        }

    } catch (const std::runtime_error &ex) {
        // capturing error message from original throw
        std::string error(ex.what());
        // appending to new throw, then re-throwing
        throw std::runtime_error("[ss_input, public float array] stream from party " + std::to_string(id) + ": " + error);
    }
}

// input randomly generated private int
void SecretShare::ss_input(mpz_t *var, std::string type) {
    mpz_urandomm(*var, rstate_mine, fieldSize);
}

// input randomly generated private float
void SecretShare::ss_input(mpz_t **var, std::string type) {
    for (int i = 0; i < 4; i++)
        mpz_urandomm((*var)[i], rstate_mine, fieldSize);
}

// input randomly generated private int arr
void SecretShare::ss_input(mpz_t *var, int size, std::string type) {
    for (int i = 0; i < size; i++) {
        mpz_urandomm(var[i], rstate_mine, fieldSize);
    }
}

// input randomly generated private float arr
void SecretShare::ss_input(mpz_t **var, int size, std::string type) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 4; j++)
            mpz_urandomm(var[i][j], rstate_mine, fieldSize);
    }
}

void SecretShare::ss_output(int id, int *var, std::string type, std::ofstream *outputStreams) {
    std::stringstream s;
    s << *var;
    outputStreams[id - 1] << s.str() + "\n";
    outputStreams[id - 1].flush();
}

void SecretShare::ss_output(int id, mpz_t *var, std::string type, std::ofstream *outputStreams) {
    std::string value;
    value = mpz_get_str(NULL, BASE_10, *var);
    outputStreams[id - 1] << value + "\n";
    outputStreams[id - 1].flush();
}

void SecretShare::ss_output(int id, float *var, std::string type, std::ofstream *outputStreams) {
    std::stringstream s;

    s << *var;
    outputStreams[id - 1] << s.str() + "\n";
    outputStreams[id - 1].flush();
}

void SecretShare::ss_output(int id, mpz_t **var, std::string type, std::ofstream *outputStreams) {
    std::string value;
    for (int i = 0; i < 4; i++) {
        value = mpz_get_str(NULL, BASE_10, (*var)[i]);
        if (i != 3)
            outputStreams[id - 1] << value + ",";
        else
            outputStreams[id - 1] << value + "\n";
        outputStreams[id - 1].flush();
    }
}

void SecretShare::ss_output(int id, mpz_t *var, int size, std::string type, std::ofstream *outputStreams) {
    std::string value;
    for (int i = 0; i < size; i++) {
        value = mpz_get_str(NULL, BASE_10, var[i]);
        if (i != size - 1)
            outputStreams[id - 1] << value + ",";
        else
            outputStreams[id - 1] << value + "\n";
        outputStreams[id - 1].flush();
    }
}

void SecretShare::ss_output(int id, int *var, int size, std::string type, std::ofstream *outputStreams) {
    for (int i = 0; i < size; i++) {
        std::stringstream s;
        s << var[i];
        if (i != size - 1)
            outputStreams[id - 1] << s.str() + ",";
        else
            outputStreams[id - 1] << s.str() + "\n";
        outputStreams[id - 1].flush();
    }
}

void SecretShare::ss_output(int id, mpz_t **var, int size, std::string type, std::ofstream *outputStreams) {
    std::string value;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 4; j++) {
            value = mpz_get_str(NULL, BASE_10, var[i][j]);
            if (j != 3)
                outputStreams[id - 1] << value + ",";
            else
                outputStreams[id - 1] << value + "\n";
            outputStreams[id - 1].flush();
        }
    }
}

void SecretShare::ss_output(int id, float *var, int size, std::string type, std::ofstream *outputStreams) {
    for (int i = 0; i < size; i++) {
        std::stringstream s;
        s << var[i];
        if (i != size - 1)
            outputStreams[id - 1] << s.str() + ",";
        else
            outputStreams[id - 1] << s.str() + "\n";
        outputStreams[id - 1].flush();
    }
}

// void ss_clear(mpz_t &x) {
//     mpz_clear(x);
// }
// void ss_set_str(mpz_t x, const char *str, int base) {
//     mpz_set_str(x, str, base);
// }
// char *ss_get_str(char *str, int base, const mpz_t op) {

//     return mpz_get_str(str, base, op);
// }

// void ss_free_arr(mpz_t *op, int size) {
//     for (int i = 0; i < size; i++)
//         mpz_clear(op[i]);
//     delete[] op;
// }

// void ss_init_set_si(mpz_t &x, int x_val) {
//     mpz_init_set_si(x, x_val);
// }
