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

#include "../shared.h"
#include "stdint.h"
#include <charconv>
#include <cstdlib>
#include <gmp.h>
#include <inttypes.h>
#include <iostream>
#include <math.h>
#include <openssl/rand.h>
#include <sstream>
#include <stdint.h> //for int8_t
#include <stdio.h>
#include <string.h> //for memcmp
#include <tmmintrin.h>
#include <unistd.h>
#include <vector>
#include <wmmintrin.h> //for intrinsics for AES-NI

#define KE2(NK, OK, RND)                           \
    NK = OK;                                       \
    NK = _mm_xor_si128(NK, _mm_slli_si128(NK, 4)); \
    NK = _mm_xor_si128(NK, _mm_slli_si128(NK, 4)); \
    NK = _mm_xor_si128(NK, _mm_slli_si128(NK, 4)); \
    NK = _mm_xor_si128(NK, _mm_shuffle_epi32(_mm_aeskeygenassist_si128(OK, RND), 0xff));
// extern "C" {
// #include "aes_ni.h"
// }

// #define KEYSIZE 16
// #define BASE 10

std::vector<std::string> splitfunc(const char *str, const char *delim);

class SecretShare {
public:
    // SecretShare() {};
    virtual ~SecretShare(){};
    virtual void getFieldSize(mpz_t){};


    // these are the only two virtual functions that need to be implemented in the child classes
    // currently only implemented in RSS
    // ShamirSS needs implementations (modification of existing functions)
    virtual std::vector<std::string> getShares(long long ) =0;
    virtual std::vector<long long> reconstructSecret(std::vector<std::vector<std::string>> , int) =0;
    virtual void flipNegative(long long&) = 0;

    // virtual void getShares(mpz_t *, mpz_t){};
    // virtual void getShares(mpz_t **, mpz_t *, int){};
    // virtual void reconstructSecret(mpz_t, mpz_t *){};
    // virtual void reconstructSecret(mpz_t *, mpz_t **, int){};

    // virtual void flipNegative(mpz_t *, mpz_t **, int){};

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

    std::vector<std::string> getShares(long long );
    std::vector<long long> reconstructSecret(std::vector<std::vector<std::string>> , int);

    void flipNegative(long long&);


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
    RSS(int, int, int); // n, t, and ring_size
    ~RSS();

    // depracated, to be removed
    void getShares(T *, T);
    void reconstructSecret(T *, T **, int);
    void flipNegative(long long&);

    // new string-based version
    std::vector<std::string> getShares(long long input);
    std::vector<long long> reconstructSecret(std::vector<std::vector<std::string>> input, int size);

    // AES prg functions
    void offline_prg(uint8_t *dest, uint8_t *src, __m128i *ri);
    __m128i *offline_prg_keyschedule(uint8_t *src);
    void prg_aes_ni(T *destination, uint8_t *seed, __m128i *key);

    // used for computing number of shares
    int nCk(int, int);

private:
    // note on SHIFT:
    // It is the modulus 2^k, computed once in constructor.
    // Differs from original RSS implementation (single value vs array of size k), since multiple different masks were needed in several protocols
    // Here, we call it based on a single ring_size for all inputs (by design)
    T SHIFT;

    int totalNumShares; // total number of shares
    int numShares;      // shares PER PARTY
    int peers;          // total number of parties (n)
    int threshold;
    int ring_size;
    std::vector<std::vector<int>> share_mapping;
    __m128i *key_prg;  // aes keyschedule
    uint8_t *key_raw;  // used to generate keyschedule
    uint8_t *key_seed; // first seed key, overwritten in every subsequent call to prg
};

template <typename T>
RSS<T>::RSS(int _peers, int _threshold, int _ring_size) : peers(_peers), threshold(_threshold), ring_size(_ring_size) {
    totalNumShares = nCk(peers, threshold);
    numShares = nCk(peers - 1, threshold);
    key_raw = new uint8_t[KEYSIZE];
    key_seed = new uint8_t[KEYSIZE];

    if (!RAND_bytes(key_raw, KEYSIZE) && !RAND_bytes(key_seed, KEYSIZE)) {
        fprintf(stderr, "ERROR (rss_setup): key generation, RAND_bytes()\n");
        exit(0);
    }

    key_prg = offline_prg_keyschedule(key_raw);
    // for (size_t i = 0; i < 100; i++)
    // {
    //      T test_input;
    // prg_aes_ni(&test_input, key_seed, key_prg);
    // std::cout<<i<<": "<<test_input<<std::endl;
    // }

    SHIFT = (T(1) << T(ring_size)) - T(1);
    // accomadates for undefined behavior where we left shift more than sizeof(T)
    if (ring_size == sizeof(T) * 8) {
        SHIFT = -1;
    }

    // static share mappings, MUST be consistent with RSS protocols in smc-compute
    switch (peers) {
    case 3:
        share_mapping = {
            {1, 2}, // p1
            {2, 0}, // p2
            {0, 1}, // p3
        };
        break;
    case 5:
        share_mapping = {
            {4, 5, 6, 7, 8, 9}, // p1
            {7, 8, 1, 9, 2, 3}, // p2
            {9, 2, 5, 3, 6, 0}, // p3
            {3, 6, 8, 0, 1, 4}, // p4
            {0, 1, 2, 4, 5, 7}  // p5
        };
        break;
    case 7:
        share_mapping = {
            {15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34}, // p1
            {25, 26, 27, 5, 28, 29, 6, 30, 7, 8, 31, 32, 9, 33, 10, 11, 34, 12, 13, 14},      // p2
            {31, 32, 9, 19, 33, 10, 20, 11, 21, 1, 34, 12, 22, 13, 23, 2, 14, 24, 3, 4},      // p3
            {34, 12, 22, 28, 13, 23, 29, 2, 6, 16, 14, 24, 30, 3, 7, 17, 4, 8, 18, 0},        // p4
            {14, 24, 30, 33, 3, 7, 10, 17, 20, 26, 4, 8, 11, 18, 21, 27, 0, 1, 5, 15},        // p5
            {4, 8, 11, 13, 18, 21, 23, 27, 29, 32, 0, 1, 2, 5, 6, 9, 15, 16, 19, 25},         // p6
            {0, 1, 2, 3, 5, 6, 7, 9, 10, 12, 15, 16, 17, 19, 20, 22, 25, 26, 28, 31}          // p7
        };
        break;
    default:
        fprintf(stderr, "ERROR (rss_setup): invalid number of parties, only n = {3, 5, 7} is supported for RSS \n");
        exit(0);
    }
}

template <typename T>
RSS<T>::~RSS() {
    delete[] key_prg;
    delete[] key_raw;
    delete[] key_seed;
}

// new implementations that use strings instead of technique-dependent types
// input: string of a single input
// return: n-dimensional vector of strings (one per party). Each string contains numShares = (n-1)Ct shares
// the representation we use for RSS share strings is A=(x_1;x_2;...;x_numShares), using ";" as to not interfere with existing parsing
// vectors of inputs would then be  A=x_1;...;x_num,y_1;...;y_num,z_1;...;z_num
template <typename T>
std::vector<std::string> RSS<T>::getShares(long long input) {
    // if (input.empty()) {
    //     fprintf(stderr, "ERROR (getShares): empty input string, are your inputs formatted correctly?\n");
    //     exit(0);
    // }

    // storing input into largest type just in case
    // long long extracted_input = stoi(input);
    std::vector<std::string> result;

    // potential alternate version that stores in T (unsigned) instead of integer (signed), need to verify (do not remove)
    // T extracted_input = 0;
    // // used to extract input into template without writing multiple versions based on ring_size (stoi, stol, stoll, ...)
    // std::from_chars(input.data(), input.data() + input.size(), extracted_input);
    T *splitInput = new T[totalNumShares];
    memset(splitInput, 0, sizeof(T) * totalNumShares);
    for (size_t i = 0; i < totalNumShares - 1; i++) {
        prg_aes_ni(&splitInput[i], key_seed, key_prg);
        splitInput[i] = splitInput[i] & SHIFT; // making sure any bits beyond k are zeroed
    }
    splitInput[totalNumShares - 1] = (input & ((T(1) << T(ring_size)) - T(1)));

    // total = nCt
    // computing x_total = x - x_1 - x_2 - ... - x_{total-1}
    for (size_t i = 0; i < totalNumShares - 1; i++) {
        splitInput[totalNumShares - 1] -= splitInput[i];
    }
    splitInput[totalNumShares - 1] = splitInput[totalNumShares - 1] & SHIFT;

    // packing shares into strings
    for (size_t i = 0; i < peers; i++) {
        std::vector<T> result_T;
        // extracting party i's shares from splitInput
        // numShares == share_mapping[i].size()
        for (size_t j = 0; j < numShares; j++) {
            result_T.push_back(splitInput[share_mapping[i][j]]);
        }
        // std::stringstream joined_stream; // alternate approach using stringstream
        // joined_stream << "(";
        std::string share_str = "";
        for (auto &value : result_T) {
            share_str.append(std::to_string(value) + ";");
            // joined_stream << value << ",";
        }
        if (share_str.empty()) {
            fprintf(stderr, "ERROR (getShares): share_str, something went very wrong in share generation\n");
            exit(0);
        }

        // std::string share_str = joined_stream.str();
        share_str.pop_back(); // deleting trailing ","
        // share_str.append(")"); // inserting closing paren

        result.push_back(share_str); // adding party i's shares to result
    }

    delete[] splitInput;

    return result;
}
// input: n*size vector of strings of shares in the format (x_1;...),(y_1;...),(z_1;...)
// return: size vector of strings of reconstructed secrets
template <typename T>
std::vector<long long> RSS<T>::reconstructSecret(std::vector<std::vector<std::string>> input, int size) {
    std::vector<long long> result;
    std::vector<std::string> share_strs;
    std::vector<std::vector<T>> extracted(size, std::vector<T>(totalNumShares, 0)); // where we store the extracted shares are stored

    // for (auto &peer : input) { // peer: vector of participant (peer)'s shares of length (size)
    // for (auto &peer_strs : peer) {                      // peer_strs: string of shares of a secret
    // p: 0->n
    // i: 0->size
    // j: 0->totalNumShares
    for (size_t p = 0; p < input.size(); p++) { // peer.size() = size
        auto peer = input[p];
        for (size_t i = 0; i < peer.size(); i++) {        // peer.size() = size
            share_strs = splitfunc(peer[i].c_str(), ";"); // vector of length numShares, split by ";"
            // for (auto &share : share_strs) {                // share: string of a single share
            for (size_t j = 0; j < share_strs.size(); j++) {
                T extracted_input = 0;
                std::from_chars(share_strs[j].data(), share_strs[j].data() + share_strs[j].size(), extracted_input);
                // putting party p's jth share of secret i into its right place (potentially overwriting the EXACT same share that is already there)
                // could theoretically do it directly in the above line
                extracted[i][share_mapping[p][j]] = extracted_input;
            }
        }
    }
    // iterating through all values
    for (auto &shares : extracted) {
        T accumulator = 0;
        // summing all shares together
        for (auto &s : shares) {
            accumulator += s;
        }
        accumulator = accumulator & SHIFT; // masking 
        // **** ANB: what is the protocol if the result is negative? 
        result.push_back((long long)(accumulator));
    }
    return result;
}

// check this works
template <typename T>
void RSS<T>::flipNegative(long long &x) {

}


template <typename T>
void RSS<T>::offline_prg(uint8_t *dest, uint8_t *src, __m128i *ri) { // ri used to be void, replaced with __m128i* to compile
    __m128i orr, mr;
    __m128i *r = ri;

    orr = _mm_load_si128((__m128i *)src);
    mr = orr;

    mr = _mm_xor_si128(mr, r[0]);
    mr = _mm_aesenc_si128(mr, r[1]);
    mr = _mm_aesenc_si128(mr, r[2]);
    mr = _mm_aesenc_si128(mr, r[3]);
    mr = _mm_aesenc_si128(mr, r[4]);
    mr = _mm_aesenc_si128(mr, r[5]);
    mr = _mm_aesenc_si128(mr, r[6]);
    mr = _mm_aesenc_si128(mr, r[7]);
    mr = _mm_aesenc_si128(mr, r[8]);
    mr = _mm_aesenc_si128(mr, r[9]);
    mr = _mm_aesenclast_si128(mr, r[10]);
    mr = _mm_xor_si128(mr, orr);
    _mm_storeu_si128((__m128i *)dest, mr);
}

template <typename T>
__m128i *RSS<T>::offline_prg_keyschedule(uint8_t *src) {
    // correctness must be checked here (modified from original just to compile, used to not have cast (__m128i*))
    // __m128i *r = static_cast<__m128i *>(malloc(11 * sizeof(__m128i)));
    __m128i *r = new __m128i[11]; // alternate
    r[0] = _mm_load_si128((__m128i *)src);

    KE2(r[1], r[0], 0x01)
    KE2(r[2], r[1], 0x02)
    KE2(r[3], r[2], 0x04)
    KE2(r[4], r[3], 0x08)
    KE2(r[5], r[4], 0x10)
    KE2(r[6], r[5], 0x20)
    KE2(r[7], r[6], 0x40)
    KE2(r[8], r[7], 0x80)
    KE2(r[9], r[8], 0x1b)
    KE2(r[10], r[9], 0x36)
    return r;
}

template <typename T>
void RSS<T>::prg_aes_ni(T *destination, uint8_t *seed, __m128i *key) {
    uint8_t res[16] = {};
    offline_prg(res, seed, key);
    memset(seed, 0, 16);
    memset(destination, 0, sizeof(T));
    memcpy(seed, res, sizeof(T));        // cipher becomes new seed or key
    memcpy(destination, res, sizeof(T)); // cipher becomes new seed or key
}

template <typename T>
int RSS<T>::nCk(int n, int k) {
    if (k > n) {
        fprintf(stderr, "ERROR (nCk): n must be >= k\n");
        exit(0);
        return -1;
    } else {
        int res = 1;
        if (k > n - k) {
            k = n - k;
        }
        for (int i = 0; i < k; ++i) {
            res *= ((int)n - i);
            res /= (i + (int)1);
        }
        return res;
    }
}

#endif
