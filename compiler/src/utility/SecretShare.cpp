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

std::vector<std::string> splitfunc(const char *str, const char *delim) {
    char *saveptr;
    char *token = strtok_r((char *)str, delim, &saveptr);
    std::vector<std::string> result;
    while (token != NULL) {
        result.push_back(token);
        token = strtok_r(NULL, delim, &saveptr);
    }
    return result;
}

ShamirSS::ShamirSS(int p, int t, mpz_t mod) {

    peers = p;
    threshold = t;
    mpz_init(fieldSize);
    mpz_set(fieldSize, mod);
    computeSharingMatrix();
    computeLagrangeWeight();

    // reading random bytes to seed
    uint8_t random_seed[KEYSIZE];
    FILE *fp = fopen("/dev/urandom", "r");
    if (fread(random_seed, 1, KEYSIZE, fp) != (KEYSIZE)) {
        fprintf(stderr, "Could not read random bytes.");
        exit(1);
    }
    fclose(fp);

    mpz_t seed;
    mpz_init(seed);
    mpz_import(seed, KEYSIZE, 1, sizeof(random_seed), 0, 0, random_seed);
    gmp_randinit_default(rstate); // default = mersenne twister
    gmp_randseed(rstate, seed); // (seed with randomness from OS)
    mpz_clear(seed);
}

void ShamirSS::setPeers(int p) {
    peers = p;
}

int ShamirSS::getPeers() {
    return peers;
}
void ShamirSS::getFieldSize(mpz_t field) {
    mpz_set(field, fieldSize);
}
void ShamirSS::setThreshold(int t) {
    threshold = t;
}
int ShamirSS::getThreshold() {
    return threshold;
}

// responsible for actually producing shares of a secret
void ShamirSS::getShares(mpz_t *shares, mpz_t secret) {

    // srand(time(NULL));
    mpz_t coefficient;
    mpz_init(coefficient);
    mpz_t temp;
    mpz_init(temp);
    mpz_set_ui(temp, 0);

    for (int i = 0; i < peers; i++)
        mpz_set_ui(shares[i], 0);
    if (mpz_cmp_si(secret, 0) < 0) {
        mpz_mod(secret, secret, fieldSize);
    }
    for (int degree = 0; degree < threshold + 1; degree++) {
        if (degree == 0)
            mpz_set(coefficient, secret);
        else {
            mpz_urandomm(coefficient, rstate, fieldSize);
            if (degree == threshold && mpz_sgn(coefficient) == 0)
                mpz_add_ui(coefficient, coefficient, 1);
        }
        for (int peer = 0; peer < peers; peer++) {
            modMul(temp, sharingMatrix[peer][degree], coefficient);
            modAdd(shares[peer], shares[peer], temp);
        }
    }
}

void ShamirSS::getShares(mpz_t **shares, mpz_t *secrets, int size) {
    mpz_t coefficient;
    mpz_init(coefficient);
    mpz_t temp;
    mpz_init_set_ui(temp, 0);
    int peer;
    for (int i = 0; i < size; i++) {
        for (int degree = 0; degree < threshold + 1; degree++) {
            if (degree == 0)
                mpz_set(coefficient, secrets[i]);
            else {
                mpz_urandomm(coefficient, rstate, fieldSize);
                if (degree == threshold && mpz_sgn(coefficient) == 0)
                    mpz_add_ui(coefficient, coefficient, 1);
            }

            for (int peer = 0; peer < peers; peer++) {
                modMul(temp, sharingMatrix[peer][degree], coefficient);
                modAdd(shares[peer][i], shares[peer][i], temp);
            }
        }
    }
    mpz_clear(coefficient);
    mpz_clear(temp);
}

void ShamirSS::modMul(mpz_t result, mpz_t x, mpz_t y) {
    mpz_mul(result, x, y);
    mpz_mod(result, result, fieldSize);
}

void ShamirSS::modMul(mpz_t *result, mpz_t *x, mpz_t *y, int size) {
    for (int i = 0; i < size; i++)
        modMul(result[i], x[i], y[i]);
}

void ShamirSS::modMul(mpz_t result, mpz_t x, long y) {
    mpz_mul_si(result, x, y);
    mpz_mod(result, result, fieldSize);
}

void ShamirSS::modMul(mpz_t *result, mpz_t *x, long y, int size) {
    for (int i = 0; i < size; ++i) {
        modMul(result[i], x[i], y);
    }
}

void ShamirSS::modMul(mpz_t *result, mpz_t *x, mpz_t y, int size) {
    for (int i = 0; i < size; ++i) {
        modMul(result[i], x[i], y);
    }
}

void ShamirSS::modAdd(mpz_t result, mpz_t x, mpz_t y) {
    mpz_add(result, x, y);
    mpz_mod(result, result, fieldSize);
}

void ShamirSS::modAdd(mpz_t *result, mpz_t *x, mpz_t *y, int size) {
    for (int i = 0; i < size; i++)
        modAdd(result[i], x[i], y[i]);
}

void ShamirSS::modAdd(mpz_t result, mpz_t x, long y) {
    mpz_t y1;
    mpz_init_set_si(y1, y);
    mpz_add(result, x, y1);
    mpz_mod(result, result, fieldSize);
    mpz_clear(y1);
}

void ShamirSS::modAdd(mpz_t *result, mpz_t *x, long y, int size) {
    for (int i = 0; i < size; ++i)
        modAdd(result[i], x[i], y);
}

void ShamirSS::modAdd(mpz_t *result, mpz_t *x, mpz_t y, int size) {
    for (int i = 0; i < size; i++)
        modAdd(result[i], x[i], y);
}

void ShamirSS::modSub(mpz_t result, mpz_t x, mpz_t y) {
    mpz_sub(result, x, y);
    mpz_mod(result, result, fieldSize);
}

void ShamirSS::modSub(mpz_t *result, mpz_t *x, mpz_t *y, int size) {
    for (int i = 0; i < size; i++)
        modSub(result[i], x[i], y[i]);
}

void ShamirSS::modSub(mpz_t result, mpz_t x, long y) {
    mpz_t y1;
    mpz_init_set_si(y1, y);
    mpz_sub(result, x, y1);
    mpz_mod(result, result, fieldSize);
    mpz_clear(y1);
}

void ShamirSS::modSub(mpz_t result, long x, mpz_t y) {
    mpz_t x1;
    mpz_init_set_si(x1, x);
    mpz_sub(result, x1, y);
    mpz_mod(result, result, fieldSize);
    mpz_clear(x1);
}

void ShamirSS::modSub(mpz_t *result, mpz_t *x, long y, int size) {
    for (int i = 0; i < size; ++i)
        modSub(result[i], x[i], y);
}

void ShamirSS::modSub(mpz_t *result, long x, mpz_t *y, int size) {
    for (int i = 0; i < size; ++i)
        modSub(result[i], x, y[i]);
}

void ShamirSS::modSub(mpz_t *result, mpz_t *x, mpz_t y, int size) {
    for (int i = 0; i < size; ++i)
        modSub(result[i], x[i], y);
}

void ShamirSS::modSub(mpz_t *result, mpz_t x, mpz_t *y, int size) {
    for (int i = 0; i < size; ++i)
        modSub(result[i], x, y[i]);
}

void ShamirSS::modPow(mpz_t result, mpz_t base, mpz_t exponent) {
    mpz_powm(result, base, exponent, fieldSize);
}

void ShamirSS::modPow(mpz_t *result, mpz_t *base, mpz_t *exponent, int size) {
    for (int i = 0; i < size; i++)
        mpz_powm(result[i], base[i], exponent[i], fieldSize);
}

void ShamirSS::modPow(mpz_t result, mpz_t base, long exponent) {
    mpz_t value;
    mpz_init_set_si(value, exponent);
    modAdd(value, value, (long)0);
    mpz_powm(result, base, value, fieldSize);
    mpz_clear(value);
}

void ShamirSS::modPow(mpz_t *result, mpz_t *base, long exponent, int size) {
    for (int i = 0; i < size; ++i)
        modPow(result[i], base[i], exponent);
}

void ShamirSS::modInv(mpz_t result, mpz_t value) {
    mpz_t temp;
    mpz_init(temp);
    mpz_sub_ui(temp, fieldSize, 2);
    modPow(result, value, temp);
    mpz_clear(temp);
}

void ShamirSS::modInv(mpz_t *result, mpz_t *values, int size) {
    for (int i = 0; i < size; i++)
        modInv(result[i], values[i]);
}

void ShamirSS::modSqrt(mpz_t result, mpz_t x) {
    mpz_t temp;
    mpz_init(temp);
    mpz_add_ui(temp, fieldSize, 1);
    mpz_div_ui(temp, temp, 4);
    modPow(result, x, temp);
    mpz_clear(temp);
}

void ShamirSS::modSqrt(mpz_t *result, mpz_t *x, int size) {
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

void ShamirSS::modSum(mpz_t result, mpz_t *x, int size) {
    mpz_set_ui(result, 0);
    for (int i = 0; i < size; ++i) {
        mpz_add(result, result, x[i]);
    }
    mpz_mod(result, result, fieldSize);
}

void ShamirSS::mod(mpz_t *result, mpz_t *a, mpz_t *m, int size) {
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

void ShamirSS::mod(mpz_t *result, mpz_t *a, mpz_t m, int size) {
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

void ShamirSS::copy(mpz_t *src, mpz_t *des, int size) {
    for (int i = 0; i < size; i++)
        mpz_set(des[i], src[i]);
}
void ShamirSS::computeSharingMatrix() {
    // initialize the shairngMatrix
    mpz_t t1, t2;
    mpz_init(t1);
    mpz_init(t2);
    sharingMatrix = (mpz_t **)malloc(sizeof(mpz_t *) * peers);
    for (int i = 0; i < peers; i++)
        sharingMatrix[i] = (mpz_t *)malloc(sizeof(mpz_t) * peers);
    for (int i = 0; i < peers; i++)
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

void ShamirSS::computeLagrangeWeight() {
    mpz_t nom, denom, t1, t2, temp;
    mpz_init(nom);
    mpz_init(denom);
    mpz_init(t1);
    mpz_init(t2);
    mpz_init(temp);

    lagrangeWeight = (mpz_t *)malloc(sizeof(mpz_t) * peers);

    for (int i = 0; i < peers; i++)
        mpz_init(lagrangeWeight[i]);

    for (int peer = 0; peer < peers; peer++) {
        int point = peer + 1;
        mpz_set_ui(nom, 1);
        mpz_set_ui(denom, 1);

        for (int l = 0; l < peers; l++) {
            if (l != peer) {
                mpz_set_ui(t1, l + 1);
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

void ShamirSS::reconstructSecret(mpz_t result, mpz_t *y) {
    mpz_t temp;
    mpz_init(temp);
    mpz_set_ui(result, 0);
    for (int peer = 0; peer < peers; peer++) {
        modMul(temp, y[peer], lagrangeWeight[peer]);
        modAdd(result, result, temp);
    }
    mpz_clear(temp);
}

void ShamirSS::reconstructSecret(mpz_t *result, mpz_t **y, int size) {
    mpz_t temp;
    mpz_init(temp);
    for (int i = 0; i < size; i++)
        mpz_set_ui(result[i], 0);
    for (int i = 0; i < size; i++) {
        for (int peer = 0; peer < peers; peer++) {
            modMul(temp, y[peer][i], lagrangeWeight[peer]);
            modAdd(result[i], result[i], temp);
        }
    }
    mpz_clear(temp);
}

// these need to be implemented (just copy from above methods, making changes to take/return ints/strings  )
std::vector<std::string> ShamirSS::getShares(long long secret) {

    // Convert long long to mpz_t
    mpz_t mpzSecret;
    mpz_init_set_si(mpzSecret, secret);

    // srand(time(NULL));
    mpz_t coefficient;
    mpz_init(coefficient);
    mpz_t temp;
    mpz_init(temp);
    mpz_set_ui(temp, 0);

    // Initialize shares inside the code to minimize changes
    mpz_t shares[peers];
    for (int i = 0; i < peers; i++)
        mpz_init(shares[i]);

    if (mpz_cmp_si(mpzSecret, 0) < 0) {
        mpz_mod(mpzSecret, mpzSecret, fieldSize);
    }

    for (int degree = 0; degree < threshold + 1; degree++) {
        if (degree == 0)
            mpz_set(coefficient, mpzSecret);
        else {
            mpz_urandomm(coefficient, rstate, fieldSize);
            if (degree == threshold && mpz_sgn(coefficient) == 0)
                mpz_add_ui(coefficient, coefficient, 1);
        }
        for (int peer = 0; peer < peers; peer++) {
            modMul(temp, sharingMatrix[peer][degree], coefficient);
            modAdd(shares[peer], shares[peer], temp);
        }
    }

    // Initialize result vector
    std::vector<std::string> result;

    // Convert shares to vector of strings
    for (int i = 0; i < peers; i++) {
        result.push_back(mpz_get_str(nullptr, BASE, shares[i]));
    }

    // Clear the mpz_t elements and return the vector of result
    for (int i = 0; i < peers; i++) {
        mpz_clear(shares[i]);
    }
    mpz_clear(coefficient);
    mpz_clear(temp);
    mpz_clear(mpzSecret);
    return result;
}


// The reconstructSecret fucntion that is used for float reconstruction
double ShamirSS::floatreconstructSecret(std::vector<std::vector<std::string>> y, int size) {

    // Declare mpz_result as an array of mpz_t
    mpz_t mpz_result[size];
    for (int i = 0; i < size; i++) {
        mpz_init(mpz_result[i]);
    }

    mpz_t temp;
    mpz_init(temp);

    for (int i = 0; i < size; i++) {
        for (int peer = 0; peer < peers; peer++) {

            // Convert string to mpz_t before using y - then use one by one
            mpz_t mpzY;
            mpz_init_set_str(mpzY, y[peer][i].c_str(), BASE);

            modMul(temp, mpzY, lagrangeWeight[peer]);
            modAdd(mpz_result[i], mpz_result[i], temp);

            mpz_clear(mpzY); // Clear mpzY before the next iteration
        }
    }

    // Do the flipNegative part here 
    for (int k = 0; k < 4; k++) {
        if (k == 1) {
            // deal with negative results
            mpz_t tmp;
            mpz_init(tmp);
            mpz_mul_ui(tmp, mpz_result[1], 2);
            if (mpz_cmp(tmp, fieldSize) > 0)
                mpz_sub(mpz_result[1], mpz_result[1], fieldSize);
        }
    }

    // Get the float form of the number
    double v = mpz_get_d(mpz_result[0]);
    double p = mpz_get_d(mpz_result[1]);
    double z = mpz_get_d(mpz_result[2]);
    double s = mpz_get_d(mpz_result[3]);
    double element = 0;
    if (z == 1)
        element = 0;
    else {
        element = v * pow(2, p);
        if (s == 1)
            element = -element;
    }

    return element;
}



// The reconstructSecret fucntion that is used for int reconstruction
std::vector<long long> ShamirSS::reconstructSecret(std::vector<std::vector<std::string>> y, int size) {

    // Declare result as an array of mpz_t
    std::vector<long long> result;
    result.resize(size);

    // Declare mpz_result as an array of mpz_t
    mpz_t mpz_result[size];
    for (int i = 0; i < size; i++) {
        mpz_init(mpz_result[i]);
    }

    mpz_t temp;
    mpz_init(temp);

    for (int i = 0; i < size; i++) {
        for (int peer = 0; peer < peers; peer++) {

            // Convert string to mpz_t before using y - then use one by one
            mpz_t mpzY;
            mpz_init_set_str(mpzY, y[peer][i].c_str(), BASE);

            modMul(temp, mpzY, lagrangeWeight[peer]);
            modAdd(mpz_result[i], mpz_result[i], temp);

            mpz_clear(mpzY); // Clear mpzY before the next iteration
        }
    }

    // Convert mpz_result to long long and store in result vector
    for (int i = 0; i < size; i++) {

        // result[i] = mpz_get_d(mpz_result[i]); // Retrieve the value
        // mpz_clear(mpz_result[i]);              // Clear the memeory

        // Initialize temporary variable
        mpz_t tmp;
        mpz_init(tmp);

        // Multiply result[j] by 2
        mpz_mul_ui(tmp, mpz_result[i], 2);

        // Check if tmp is greater than fieldSize
        if (mpz_cmp(tmp, fieldSize) > 0) { 
            // If so, subtract fieldSize from result[j]
            mpz_sub(mpz_result[i], mpz_result[i], fieldSize);
        }
        result[i] = mpz_get_si(mpz_result[i]); // Retrieve the value

        // Clear the memeory
        mpz_clear(mpz_result[i]);             
        mpz_clear(tmp);
    }
    mpz_clear(temp);
    return result;
}

// check this works
void ShamirSS::flipNegative(long long &x) {

    // Init the variables
    mpz_t tmp, tmp_x;
    mpz_init(tmp);
    mpz_init(tmp_x);

    mpz_set_si(tmp_x, x);
    mpz_mul_ui(tmp, tmp_x, 2);
    if (mpz_cmp(tmp, fieldSize) > 0)
        mpz_sub(tmp_x, tmp_x, fieldSize);
    x = mpz_get_si(tmp_x);

    // Clear the memory
    mpz_clear(tmp);
    mpz_clear(tmp_x);
}
