/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2023 PICCO Team
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

#ifndef _REPSECRETSHARE_HPP_
#define _REPSECRETSHARE_HPP_

#include "../bit_utils.hpp"
#include "stdint.h"
#include <charconv>
#include <cmath>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <vector>

#ifdef __arm64__
#include "../../../common/sse2neon.h" //for intrinsics (translated for ARM processors, e.g., Apple Silicon)
#else
#include <wmmintrin.h> //for intrinsics for AES-NI
#endif

#define CONTAINER_SIZE 16
#define KEYSIZE 16

using std::cout;
using std::endl;
using std::vector;

#define KE2(NK, OK, RND)                           \
    NK = OK;                                       \
    NK = _mm_xor_si128(NK, _mm_slli_si128(NK, 4)); \
    NK = _mm_xor_si128(NK, _mm_slli_si128(NK, 4)); \
    NK = _mm_xor_si128(NK, _mm_slli_si128(NK, 4)); \
    NK = _mm_xor_si128(NK, _mm_shuffle_epi32(_mm_aeskeygenassist_si128(OK, RND), 0xff));

// the RSS replicatedSecretShare class is implemented entirely in this header file, due to it being templated (for 32- and 64- bit computation)

// arithmetic mod the number of parties, values are in the ring 1,...,n
// supports "negative" values, i.e. if you enter id-1, it will appropriately compute the "left" id in the ring
template <typename T>
T mod_n(T a, T n) {
    return ((a - T(1)) % n + n) % n + T(1);
}

inline void print_hexa_2(uint8_t *message, int message_length) {
    for (int i = 0; i < message_length; i++) {
        printf("%02x", message[i]);
    }
    printf(";\n");
}
template <typename T>
std::ostream &operator<<(std::ostream &out, const std::vector<T> &v) {
    out << "[";
    size_t last = v.size() - 1;
    for (size_t i = 0; i < v.size(); ++i) {
        out << v[i];
        if (i != last)
            out << ", ";
    }
    out << "]";
    return out;
}

template <typename T>
class replicatedSecretShare {

public:
    replicatedSecretShare(int _id, int _n, int _t, uint ring_size, std::map<std::vector<int>, std::string>);
    ~replicatedSecretShare();
    uint getNumShares();
    uint getTotalNumShares();
    int getPeers();
    int getThreshold();
    int getID();

    uint nCk(uint n, uint k);

    __m128i *prg_keyschedule(uint8_t *src);
    void prg_aes(uint8_t *, uint8_t *, __m128i *);
    void prg_setup(std::map<std::vector<int>, std::string>);
    void prg_getrandom(int keyID, uint size, uint length, uint8_t *dest);
    void prg_getrandom(uint size, uint length, uint8_t *dest);

    std::vector<std::string> split(const std::string s, const std::string delimiter, int expected_size = 0); // =0 used for optional argument passsing of expected_size

    // I/O functions
    void ss_input(int id, int *var, std::string type, std::ifstream *inputStreams);
    void ss_input(int id, T **var, std::string type, std::ifstream *inputStreams);
    void ss_input(int id, float *var, std::string type, std::ifstream *inputStreams);
    void ss_input(int id, T ***var, std::string type, std::ifstream *inputStreams);

    void ss_input(int id, T **var, int size, std::string type, std::ifstream *inputStreams);
    void ss_input(int id, int *var, int size, std::string type, std::ifstream *inputStreams);
    void ss_input(int id, T ***var, int size, std::string type, std::ifstream *inputStreams);
    void ss_input(int id, float *var, int size, std::string type, std::ifstream *inputStreams);
    void ss_input(T *var, std::string type);
    void ss_input(T **var, std::string type);
    void ss_input(T **var, int size, std::string type);
    void ss_input(T ***var, int size, std::string type);

    void ss_output(int id, int *var, std::string type, std::ofstream *outputStreams);
    void ss_output(int id, T **var, std::string type, std::ofstream *outputStreams);
    void ss_output(int id, float *var, std::string type, std::ofstream *outputStreams);
    void ss_output(int id, T ***var, std::string type, std::ofstream *outputStreams);

    void ss_output(int id, T **var, int size, std::string type, std::ofstream *outputStreams);
    void ss_output(int id, int *var, int size, std::string type, std::ofstream *outputStreams);
    void ss_output(int id, T ***var, int size, std::string type, std::ofstream *outputStreams);
    void ss_output(int id, float *var, int size, std::string type, std::ofstream *outputStreams);

    void modMul(T *result, T *x, T y);
    void modMul(T *result, T *x, T *y);
    void modMul(T *result, T *x, int y);
    void modMul(T **result, T **x, T *y, int size);
    void modMul(T *result, T *x, long y);
    void modMul(T **result, T **x, long y, int size);
    void modMul(T **result, T **x, T y, int size);
    void modMul(T **result, T **x, int *y, int size);

    void modAdd(T *result, T *x, T *y);
    void modAdd(T **result, T **x, T **y, int size);
    void modAdd(T *result, T *x, long y);
    void modAdd(T **result, T **x, long y, int size);
    void modAdd(T **result, T **x, T *y, int size);
    void modAdd(T **result, T **x, long *y, int size);
    void modAdd(T **result, T **x, int *y, int size);

    void modSub(T *result, T *x, T *y);
    void modSub(T **result, T **x, T **y, int size);
    void modSub(T *result, T *x, long y);
    void modSub(T *result, long x, T *y);
    void modSub(T **result, T **x, long y, int size);
    void modSub(T **result, long x, T **y, int size);
    void modSub(T **result, T **x, T *y, int size);
    void modSub(T **result, T *x, T **y, int size);
    void modSub(T **result, T **x, int *y, int size);
    void modSub(T **result, int *x, T **y, int size);

    void modPow2(T &result, int exponent);
    void modPow2(T &result, T exponent);
    void modPow2(T *result, int *exponent, int size);
    void modPow2(T *result, T *exponent, int size);

    // temporary functions, need to be dealt with later (just to get smc-utils to compile)
    //  these are undefined in the context of RSS, because the dimensionality doesn't match
    //  in shamir, you can store a public value in an mpz_t (one to one)
    //  however, you can't do the same for RSS. Storing a public int in a T* doesn't work (one to arr)
    void modPow2(T *result, T *exponent);
    void modPow2(T *result, int exponent);
    void modPow2(T **result, T **exponent, int size);
    void modPow2(T **result, int *exponent, int size);
    void modMul(T **result, T **x, T **y, int size);

    void modPow(T result, T base, T exponent);
    void modPow(T *result, T *base, T *exponent, int size);
    void modPow(T result, T base, long exponent);
    void modPow(T *result, T *base, long exponent, int size);

    void generatePublicCoef(T *result, int value);

    std::vector<int> generateT_star(int p_star);
    int generateT_star_index(int p_star);
    void sparsify(T **result, int *x, int size);
    void sparsify(T **result, T *x, int size);
    void sparsify_public(T *result, int x);
    void sparsify_public(std::vector<T> &result, int x);

    std::vector<std::vector<int>> generateB2A_map();
    std::vector<int> generateXi_map();

    std::vector<std::vector<int>> generate_MultSparse_map(int n, int id);

    inline bool pid_in_T(int &pid, std::vector<int> &T_map);
    inline bool chi_pid_is_T(int& pid, std::vector<int> &T_map);
    std::vector<std::vector<int>> generateInputSendRecvMap(std::vector<int> &input_parties);


    T *SHIFT;
    T *ODD;
    T *EVEN;
    uint ring_size; // ring_size

    std::vector<std::vector<int>> general_map;
    std::vector<std::vector<int>> open_map_mpc;
    std::vector<std::vector<int>> eda_map_mpc;
    std::vector<std::vector<int>> T_map_mpc;
    std::vector<T> const_map; // this is used for multiplying a private value by a public constant, OR adding a public constant to a private value

private:
    int id;              // node ID
    int n;               // n
    int t;               // t
    uint numShares;      // (n-1) choose t
    uint totalNumShares; // n choose t
    uint8_t **random_container;
    int *P_container;
    __m128i **prg_key;
};

/* method definitions */
template <typename T>
replicatedSecretShare<T>::replicatedSecretShare(int _id, int _n, int _t, uint _ring_size, std::map<std::vector<int>, std::string> rss_share_seeds) {
    id = _id;
    n = _n;
    t = _t;
    ring_size = _ring_size;
    numShares = nCk(n - 1, t);  // shares PER PARTY
    totalNumShares = nCk(n, t); // total shares

    // allocating the const_map dynamically after numShares is calculated
    const_map.resize(numShares, 0);

    SHIFT = new T[sizeof(T) * 8 + 1];
    ODD = new T[ring_size + 2];
    EVEN = new T[ring_size + 2];

    // i think this is "21" for the minion nn?
    for (T i = 0; i <= sizeof(T) * 8; i++) {
        SHIFT[i] = (T(1) << T(i)) - T(1); // mod 2^i

        // this is needed to handle "undefined behavior" of << when we want
        // to shift by more than the size of the type (in bits)
        if (i == sizeof(T) * 8) {
            SHIFT[i] = -1;
        }
    }

    T temp = 0;
    for (T i = 0; i <= 8 * sizeof(T); i++) {
        temp = temp | T((i % 2 == 0));
        temp = temp << 1;
    }
    for (T i = 0; i < ring_size + 1; i++) {
        EVEN[i] = (temp >> T(1)) & SHIFT[i];
        ODD[i] = temp & SHIFT[i];
    }

    switch (n) {
    case 3:

        general_map = {
            {mod_n(id + 2, n)},
            {mod_n(id + 1, n)},
        };
        // p1: s2, s3
        // p2: s3, s1
        // p3: s1, s2
        T_map_mpc = {
            {mod_n(id + 1, n)},
            {mod_n(id - 1, n)},
        };
        for (auto &var : T_map_mpc) { // using reference so vector gets modified
            sort(var.begin(), var.end());
        }
        // we want s2 = 0 (for parties 1 and 3)
        if (id == 1) {
            const_map[0] = 1; // party 1's share 1
        } else if (id == 3) {
            const_map[1] = 1; // party 3's share 2
        }

        break;
    case 5:
        open_map_mpc = {
            {mod_n(id + 1, n), mod_n(id + 4, n)},
            {mod_n(id + 4, n), mod_n(id + 1, n)},
        };
        general_map = {
            {mod_n(id + 3, n), mod_n(id + 4, n)}, // send
            {mod_n(id + 2, n), mod_n(id + 1, n)}, // recv
        };

        T_map_mpc = {
            {mod_n(id + 1, n), mod_n(id + 2, n)}, // becomes T_star in Input()
            {mod_n(id + 1, n), mod_n(id + 3, n)},
            {mod_n(id + 1, n), mod_n(id + 4, n)},
            {mod_n(id + 2, n), mod_n(id + 3, n)},
            {mod_n(id + 2, n), mod_n(id + 4, n)},
            {mod_n(id + 3, n), mod_n(id + 4, n)},
        };

        // sorting the T_map
        // we can sort the T_map without fear of messing up the order of shares
        // since these partially function as the ID for each share
        // shares will always be in the correct positions regardless
        for (auto &var : T_map_mpc) {
            sort(var.begin(), var.end());
        }

        // used to indicate which parties generate which random shares, and if/who they recieve shares from
        eda_map_mpc = {
            // if my id < 4, compute as normal, otherwise set to -1
            {((id < 4) ? mod_n(id - 1, n) : -1),
             ((id < 4) ? mod_n(id - 2, n) : -1)},
            // if the COMPUTED VALUE is < 4 , compute as normal, otherwise set to -1
            {(mod_n(id + 1, n) < 4 ? mod_n(id + 1, n) : -1),
             (mod_n(id + 2, n) < 4 ? mod_n(id + 2, n) : -1)},
        };

        if (id == 1) {
            const_map[0] = 1;
        } else if (id == 4) {
            const_map[5] = 1; // party 4's share 6
        } else if (id == 5) {
            const_map[3] = 1; // party 5's share 4
        }

        break;
    case 7:
        open_map_mpc = {
            {mod_n(id + 1, n), mod_n(id + 2, n), mod_n(id + 3, n)},
            {mod_n(id - 1, n), mod_n(id - 2, n), mod_n(id - 3, n)},
        };

        general_map = {
            {mod_n(id + 4, n), mod_n(id + 5, n), mod_n(id + 6, n)}, // send
            {mod_n(id + 3, n), mod_n(id + 2, n), mod_n(id + 1, n)}, // recv
        };

        T_map_mpc = {
            {mod_n(id + 1, n), mod_n(id + 2, n), mod_n(id + 3, n)},
            {mod_n(id + 1, n), mod_n(id + 2, n), mod_n(id + 4, n)},
            {mod_n(id + 1, n), mod_n(id + 2, n), mod_n(id + 5, n)},
            {mod_n(id + 1, n), mod_n(id + 2, n), mod_n(id + 6, n)},
            {mod_n(id + 1, n), mod_n(id + 3, n), mod_n(id + 4, n)},
            {mod_n(id + 1, n), mod_n(id + 3, n), mod_n(id + 5, n)},
            {mod_n(id + 1, n), mod_n(id + 3, n), mod_n(id + 6, n)},
            {mod_n(id + 1, n), mod_n(id + 4, n), mod_n(id + 5, n)},
            {mod_n(id + 1, n), mod_n(id + 4, n), mod_n(id + 6, n)},
            {mod_n(id + 1, n), mod_n(id + 5, n), mod_n(id + 6, n)},
            {mod_n(id + 2, n), mod_n(id + 3, n), mod_n(id + 4, n)},
            {mod_n(id + 2, n), mod_n(id + 3, n), mod_n(id + 5, n)},
            {mod_n(id + 2, n), mod_n(id + 3, n), mod_n(id + 6, n)},
            {mod_n(id + 2, n), mod_n(id + 4, n), mod_n(id + 5, n)},
            {mod_n(id + 2, n), mod_n(id + 4, n), mod_n(id + 6, n)},
            {mod_n(id + 2, n), mod_n(id + 5, n), mod_n(id + 6, n)},
            {mod_n(id + 3, n), mod_n(id + 4, n), mod_n(id + 5, n)},
            {mod_n(id + 3, n), mod_n(id + 4, n), mod_n(id + 6, n)},
            {mod_n(id + 3, n), mod_n(id + 5, n), mod_n(id + 6, n)},
            {mod_n(id + 4, n), mod_n(id + 5, n), mod_n(id + 6, n)},
        };

        // sorting each share in T_map
        for (auto &var : T_map_mpc) {
            sort(var.begin(), var.end());
        }

        if (id == 1) {
            const_map[0] = 1; // party 1's share 1
        } else if (id == 5) {
            const_map[19] = 1; // party 5's share
        } else if (id == 6) {
            const_map[16] = 1; // party 6's share
        } else if (id == 7) {
            const_map[10] = 1; // party 7's share
        }

        eda_map_mpc = {
            // if my id < 5, compute as normal, otherwise set to -1
            {((id < 5) ? mod_n(id - 1, n) : -1),
             ((id < 5) ? mod_n(id - 2, n) : -1),
             ((id < 5) ? mod_n(id - 3, n) : -1)},
            // if the COMPUTED VALUE is < 5 , compute as normal, otherwise set to -1
            {(mod_n(id + 1, n) < 5 ? mod_n(id + 1, n) : -1),
             (mod_n(id + 2, n) < 5 ? mod_n(id + 2, n) : -1),
             (mod_n(id + 3, n) < 5 ? mod_n(id + 3, n) : -1)},
        };
        break;
    default:
        std::cerr << "ERROR: an invalid number of parties was provided. RSS only supports n = {3,5,7}\n";
        exit(1);
    }
    printf("prg_setup\n");
    prg_setup(rss_share_seeds);
}

// the keys in rss_share_seeds are ALREADY SORTED
// we sorted T_map_mpc in the replicatedSecretShare constructor (above)
// now we just need to access rss_share_seeds[T_map_mpc.at(i)] to get the corresponding value (32 random bytes, the "value")
template <typename T>
void replicatedSecretShare<T>::prg_setup(std::map<std::vector<int>, std::string> rss_share_seeds) {
    // for (auto &[key, value] : rss_share_seeds) {
    //     std::cout << key << " has value " << value << endl;
    //     // print_hexa_2(value, 2*KEYSIZE);
    // }

    // need to create numShares+1 keys, random containers, etc
    uint numKeys = numShares + 1;
    random_container = new uint8_t *[numKeys];
    // this doesn't need to be created with "new"
    // since we call new inside prg_keyschedule, and return the memory created inside it to prg_key
    prg_key = new __m128i *[numKeys];
    for (int i = 0; i < numKeys; i++) {
        random_container[i] = new uint8_t[KEYSIZE];
        memset(random_container[i], 0, sizeof(uint8_t) * KEYSIZE);
    }
    // initalizing P_container
    P_container = new int[numKeys];
    memset(P_container, 0, sizeof(int) * numKeys);

    // Reading random bytes for my own private key (not shared with others)
    uint8_t RandomData[2 * KEYSIZE];
    try {
        FILE *fp = fopen("/dev/urandom", "r");
        if (fread(RandomData, 1, 2 * KEYSIZE, fp) != 2 * KEYSIZE) {
            throw std::runtime_error("error reading random bytes from /dev/urandom. Which OS are you using?");
        }
        fclose(fp);
    } catch (const std::runtime_error &ex) {
        std::cerr << "[generateInputSendRecvMap] " << ex.what() << "\n";
        exit(1);
    }

    // my own PRG key is always stored LAST (at index [numKeys - 1])
    memcpy(random_container[numKeys - 1], RandomData, KEYSIZE);
    prg_key[numKeys - 1] = prg_keyschedule(RandomData + KEYSIZE);

    for (size_t i = 0; i < T_map_mpc.size(); i++) {
        // getting value stored at rss_share_seeds[T_map_mpc[i]]
        std::string str = rss_share_seeds[T_map_mpc.at(i)];
        uint8_t key[2 * KEYSIZE];
        std::copy(str.begin(), str.end(), key);

        // std::cout << T_map_mpc.at(i) << " => ";
        // print_hexa_2(key, 2*KEYSIZE);

        memcpy(random_container[i], key, KEYSIZE);
        prg_key[i] = prg_keyschedule(key + KEYSIZE); // should be able to do this
    }
    uint8_t res[KEYSIZE] = {};
    for (size_t i = 0; i < numKeys; i++) {
        prg_aes(res, random_container[i], prg_key[i]);
        memcpy(random_container[i], res, KEYSIZE);
    }

    printf("PRG setup complete\n");
}

template <typename T>
__m128i *replicatedSecretShare<T>::prg_keyschedule(uint8_t *src) {
    // __m128i *r = (__m128i *)malloc(11 * sizeof(__m128i));
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
void replicatedSecretShare<T>::prg_aes(uint8_t *dest, uint8_t *src, __m128i *ri) {
    __m128i rr, mr;
    __m128i *r = ri;

    rr = _mm_loadu_si128((__m128i *)src);
    mr = rr;

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
    mr = _mm_xor_si128(mr, rr);

    _mm_storeu_si128((__m128i *)dest, mr);
}

template <typename T>
void replicatedSecretShare<T>::prg_getrandom(int keyID, uint size, uint length, uint8_t *dest) {
    // we assume CONTAINER_SIZE is 16, so all *CONTAINER_SIZE are replaced as <<4
    // this size means how many random bytes we need
    // uint8_t *buffer = new uint8_t [size];
    // its always size * length
    // printf("curent P is %d \n",P_container[keyID]);
    uint rounds = ((size * length - CONTAINER_SIZE + P_container[keyID]) + 15) >> 4;
    if (rounds == 0) {
        memcpy(dest, random_container[keyID] + P_container[keyID], size * length);
        P_container[keyID] = P_container[keyID] + size * length;
    } else {
        memcpy(dest, random_container[keyID] + P_container[keyID], CONTAINER_SIZE - P_container[keyID]);
        if (rounds >= 2) {
            prg_aes(dest + (CONTAINER_SIZE - P_container[keyID]), random_container[keyID], prg_key[keyID]);
            for (int i = 1; i < rounds - 1; i++) {
                // segfault in this loop for "large" size
                // printf("i : %u\n", i);
                prg_aes(dest + (CONTAINER_SIZE - P_container[keyID]) + (i << 4), dest + (CONTAINER_SIZE - P_container[keyID]) + ((i - 1) << 4), prg_key[keyID]);
            }
            prg_aes(random_container[keyID], dest + (CONTAINER_SIZE - P_container[keyID]) + ((rounds - 2) << 4), prg_key[keyID]);
            memcpy(dest + CONTAINER_SIZE - P_container[keyID] + ((rounds - 1) << 4), random_container[keyID], size * length - ((rounds - 1) << 4) - CONTAINER_SIZE + P_container[keyID]);
            P_container[keyID] = size * length - ((rounds - 1) << 4) - CONTAINER_SIZE + P_container[keyID];
        } else {
            prg_aes(random_container[keyID], random_container[keyID], prg_key[keyID]);
            memcpy(dest + CONTAINER_SIZE - P_container[keyID], random_container[keyID], size * length - CONTAINER_SIZE + P_container[keyID]);
            P_container[keyID] = size * length - CONTAINER_SIZE + P_container[keyID];
        }
    }
}

template <typename T>
void replicatedSecretShare<T>::prg_getrandom(uint size, uint length, uint8_t *dest) {
    // uses party's OWN KEY not shared with others to locally generate shares
    // we assume CONTAINER_SIZE is 16, so all *CONTAINER_SIZE are replaced as <<4
    // this size means how many random bytes we need
    // when this functions is called we use the party's private key stored in the LAST position [numKeys - 1] (or just numShares since numShares = numKeys + 1)
    uint keyID = numShares;
    uint rounds = ((size * length - CONTAINER_SIZE + P_container[keyID]) + 15) >> 4;
    if (rounds == 0) {
        memcpy(dest, random_container[keyID] + P_container[keyID], size * length);
        P_container[keyID] = P_container[keyID] + size * length;
    } else {
        memcpy(dest, random_container[keyID] + P_container[keyID], CONTAINER_SIZE - P_container[keyID]);
        if (rounds >= 2) {
            prg_aes(dest + (CONTAINER_SIZE - P_container[keyID]), random_container[keyID], prg_key[keyID]);
            for (int i = 1; i < rounds - 1; i++) {
                prg_aes(dest + (CONTAINER_SIZE - P_container[keyID]) + (i << 4), dest + (CONTAINER_SIZE - P_container[keyID]) + ((i - 1) << 4), prg_key[keyID]);
            }
            prg_aes(random_container[keyID], dest + (CONTAINER_SIZE - P_container[keyID]) + ((rounds - 2) << 4), prg_key[keyID]);
            memcpy(dest + CONTAINER_SIZE - P_container[keyID] + ((rounds - 1) << 4), random_container[keyID], size * length - ((rounds - 1) << 4) - CONTAINER_SIZE + P_container[keyID]);
            P_container[keyID] = size * length - ((rounds - 1) << 4) - CONTAINER_SIZE + P_container[keyID];
        } else {
            prg_aes(random_container[keyID], random_container[keyID], prg_key[keyID]);
            memcpy(dest + CONTAINER_SIZE - P_container[keyID], random_container[keyID], size * length - CONTAINER_SIZE + P_container[keyID]);
            P_container[keyID] = size * length - CONTAINER_SIZE + P_container[keyID];
        }
    }
}

template <typename T>
int replicatedSecretShare<T>::getPeers() {
    return n;
}

template <typename T>
int replicatedSecretShare<T>::getThreshold() {
    return t;
}

template <typename T>
int replicatedSecretShare<T>::getID() {
    return id;
}

// template <typename T>
// uint replicatedSecretShare<T>::getNumShares() {
//     return numShares;
// }

// template <typename T>
// uint replicatedSecretShare<T>::getTotalNumShares() {
//     return totalNumShares;
// }

// modern, robust C++ version of the above function
// expected_size is an OPTIONAL ARGUMENT (default 0)
template <typename T>
std::vector<std::string> replicatedSecretShare<T>::split(const std::string s, const std::string delimiter, int expected_size) {
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
        if (delimiter == "=") { // should only be two values in the output, the variable name (result[0]) and a string of shares "s_00;s_01;...;s_0numShares,s_10;s_11;...;s_1numShares" (result[1])
            if (result.size() != 2)
                throw std::runtime_error("Encountered an unexpected number of entries than expected, or string is not well-formed.\nInput provided:\n" + s);
        } else if ((delimiter == ",") or (delimiter == ";")) {
            for (auto &var : result)
                var = std::regex_replace(var, std::regex("^ +| +$|( ) +"), "$1"); // stripping leading/trailing whitespace from string
            //(,) should only be `expected_size` number of entries in result.size(): <"s_00;s_01;...;s_0numShares", "s_10;s_11;...;s_1numShares", ...>
            //(;) should only be `expected_size` number of entries in result.size(): <"s_0", "s_1", ..., "s_n">
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
template <typename T>
void replicatedSecretShare<T>::ss_input(int id, int *var, std::string type, std::ifstream *inputStreams) {
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
template <typename T>
void replicatedSecretShare<T>::ss_input(int id, T **var, std::string type, std::ifstream *inputStreams) {
    try {
        std::string line;
        std::vector<std::string> tokens;
        std::getline(inputStreams[id - 1], line);
        tokens = split(line, "=");
        tokens = split(tokens[1], ",", numShares);
        for (int i = 0; i < numShares; i++) {
            if (!is_int(tokens[i]))
                throw std::runtime_error("Non-integer input provided: " + tokens[i]);
            std::from_chars(tokens[i].data(), tokens[i].data() + tokens[i].size(), (*var)[i]);
        }

    } catch (const std::runtime_error &ex) {
        // capturing error message from original throw
        std::string error(ex.what());
        // appending to new throw, then re-throwing
        throw std::runtime_error("[ss_input, private int] stream from party " + std::to_string(id) + ": " + error);
    }
}

// input public float
template <typename T>
void replicatedSecretShare<T>::ss_input(int id, float *var, std::string type, std::ifstream *inputStreams) {
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
template <typename T>
void replicatedSecretShare<T>::ss_input(int id, T ***var, std::string type, std::ifstream *inputStreams) {
    try {
        std::string line;
        std::vector<std::string> tokens;
        std::vector<std::string> temp;
        std::getline(inputStreams[id - 1], line);
        tokens = split(line, "=");
        tokens = split(tokens[1], ",", 4); // reusing tokens
        for (int i = 0; i < 4; i++) {
            temp = split(tokens[i], ";", numShares);
            for (int j = 0; j < numShares; j++) {
                if (!is_int(temp[j]))
                    throw std::runtime_error("Non-integer input provided: " + temp[j]);
                std::from_chars(temp[j].data(), temp[j].data() + temp[j].size(), (*var)[j][i]);
            }
        }
    } catch (const std::runtime_error &ex) {
        // capturing error message from original throw
        std::string error(ex.what());
        // appending to new throw, then re-throwing
        throw std::runtime_error("[ss_input, private float] stream from party " + std::to_string(id) + ": " + error);
    }
}

// one-dimensional PRIVATE int array I/O
template <typename T>
void replicatedSecretShare<T>::ss_input(int id, T **var, int size, std::string type, std::ifstream *inputStreams) {
    try {
        std::string line;
        std::vector<std::string> tokens;
        std::vector<std::string> temp;
        std::getline(inputStreams[id - 1], line);
        tokens = split(line, "=");
        tokens = split(tokens[1], ",", size); // reusing tokens
        for (int i = 0; i < size; i++) {
            temp = split(tokens[i], ";", numShares);
            for (int j = 0; j < numShares; j++) {
                if (!is_int(temp[j]))
                    throw std::runtime_error("Non-integer input provided: " + temp[j]);
                std::from_chars(temp[j].data(), temp[j].data() + temp[j].size(), var[j][i]);
            }
        }
    } catch (const std::runtime_error &ex) {
        // capturing error message from original throw
        std::string error(ex.what());
        // appending to new throw, then re-throwing
        throw std::runtime_error("[ss_input, private int arr] stream from party " + std::to_string(id) + ": " + error);
    }
}

// 1-D PUBLIC int
template <typename T>
void replicatedSecretShare<T>::ss_input(int id, int *var, int size, std::string type, std::ifstream *inputStreams) {
    try {
        std::string line;
        std::vector<std::string> tokens;
        std::getline(inputStreams[id - 1], line);
        tokens = split(line, "=");
        tokens = split(tokens[1], ",", size);
        for (int i = 0; i < size; i++) {
            if (!is_int(tokens[i]))
                throw std::runtime_error("Non-integer input provided: " + tokens[i]);
            var[i] = atoi(tokens[i].c_str());
        }

    } catch (const std::runtime_error &ex) {
        // capturing error message from original throw
        std::string error(ex.what());
        // appending to new throw, then re-throwing
        throw std::runtime_error("[ss_input, public int array] stream from party " + std::to_string(id) + ": " + error);
    }
}

// one-dimensional PRIVATE float array I/O
template <typename T>
void replicatedSecretShare<T>::ss_input(int id, T ***var, int size, std::string type, std::ifstream *inputStreams) {
    try {
        std::string line;
        std::vector<std::string> tokens;
        std::vector<std::string> temp;

        for (int i = 0; i < size; i++) { // reading size lines
            std::getline(inputStreams[id - 1], line);
            tokens = split(line, "=");
            tokens = split(tokens[1], ",", 4); // reusing tokens, each line will contain 4 sets of shares
            for (int j = 0; j < 4; j++) {
                temp = split(tokens[j], ";", numShares);
                for (int k = 0; k < numShares; k++) { // each set will contain numShares shares
                    if (!is_int(temp[k]))
                        throw std::runtime_error("Non-integer input provided: " + temp[k]);
                    std::from_chars(temp[k].data(), temp[k].data() + temp[k].size(), var[k][i][j]);
                }
            }
        }
    } catch (const std::runtime_error &ex) {
        // capturing error message from original throw
        std::string error(ex.what());
        // appending to new throw, then re-throwing
        throw std::runtime_error("[ss_input, private float array] stream from party " + std::to_string(id) + ": " + error);
    }
}

// 1-D PUBLIC float
template <typename T>
void replicatedSecretShare<T>::ss_input(int id, float *var, int size, std::string type, std::ifstream *inputStreams) {
    try {
        std::string line;
        std::vector<std::string> tokens;
        std::getline(inputStreams[id - 1], line);
        tokens = split(line, "=");
        tokens = split(tokens[1], ",", size);
        for (int i = 0; i < size; i++) {
            if (!is_float(tokens[i]))
                throw std::runtime_error("Non-float input provided: " + tokens[i]);
            var[i] = atof(tokens[i].c_str());
        }

    } catch (const std::runtime_error &ex) {
        // capturing error message from original throw
        std::string error(ex.what());
        // appending to new throw, then re-throwing
        throw std::runtime_error("[ss_input, public float array] stream from party " + std::to_string(id) + ": " + error);
    }
}

// input randomly generated private int
template <typename T>
void replicatedSecretShare<T>::ss_input(T *var, std::string type) {
    uint bytes = (ring_size + 7) >> 3;
    uint8_t *buffer = new uint8_t[bytes * numShares];
    prg_getrandom(bytes, numShares, buffer);
    memcpy(var, buffer, bytes * numShares); // check
}

// input randomly generated private float
template <typename T>
void replicatedSecretShare<T>::ss_input(T **var, std::string type) {
    uint bytes = (ring_size + 7) >> 3;
    uint8_t *buffer = new uint8_t[bytes * 4 * numShares];
    prg_getrandom(bytes, 4 * numShares, buffer);
    for (int i = 0; i < numShares; i++)
        memcpy(var[i], buffer + i * 4 * bytes, 4 * bytes); // check

    // this memcpy cant be done in a single line, i..e
    // memcpy(var, buffer, bytes*numShares);
    // reason: each "row" of var is contiguous, but the ENTIRE 2D array is NOT CONTIGUOUS
}

// input randomly generated private int arr
template <typename T>
void replicatedSecretShare<T>::ss_input(T **var, int size, std::string type) {
    uint bytes = (ring_size + 7) >> 3;
    uint8_t *buffer = new uint8_t[bytes * size * numShares];
    prg_getrandom(bytes, size * numShares, buffer);
    for (int i = 0; i < numShares; i++)
        memcpy(var[i], buffer + i * size * bytes, size * bytes); // check
}

// input randomly generated private float arr
template <typename T>
void replicatedSecretShare<T>::ss_input(T ***var, int size, std::string type) {
    uint bytes = (ring_size + 7) >> 3;
    uint8_t *buffer = new uint8_t[bytes * size * 4 * numShares];
    prg_getrandom(bytes, size * 4 * numShares, buffer);
    for (int i = 0; i < numShares; i++)
        for (int j = 0; j < size; j++)
            // memcpy(var[i][j], buffer + i * size * 4 * bytes + j * 4 * bytes, 4 * bytes); // check
            memcpy(var[i][j], buffer + (i * size + j) * 4 * bytes, 4 * bytes); // "simpler" version
}

template <typename T>
void replicatedSecretShare<T>::ss_output(int id, int *var, std::string type, std::ofstream *outputStreams) {
    std::stringstream s;
    s << *var;
    outputStreams[id - 1] << s.str() + "\n";
    outputStreams[id - 1].flush();
}

template <typename T>
void replicatedSecretShare<T>::ss_output(int id, T **var, std::string type, std::ofstream *outputStreams) {
    // for (int i = 0; i < numShares; i++) {
    //     if (i != (numShares - 1))
    //         outputStreams[id - 1] << std::to_string(var[i]) + ";";
    //     else
    //         outputStreams[id - 1] << std::to_string(var[i]) + "\n";
    //     outputStreams[id - 1].flush();
    // }
}

template <typename T>
void replicatedSecretShare<T>::ss_output(int id, float *var, std::string type, std::ofstream *outputStreams) {
    std::stringstream s;
    s << *var;
    outputStreams[id - 1] << s.str() + "\n";
    outputStreams[id - 1].flush();
}

template <typename T>
void replicatedSecretShare<T>::ss_output(int id, T ***var, std::string type, std::ofstream *outputStreams) {
    // for (int i = 0; i < 4; i++) {
    //     for (int j = 0; j < numShares; j++) {
    //         if (j != (numShares - 1))
    //             outputStreams[id - 1] << std::to_string(var[j][i]) + ";";
    //         else {
    //             if (i != 3)
    //                 outputStreams[id - 1] << std::to_string(var[j][i]) + ",";
    //             else
    //                 outputStreams[id - 1] << std::to_string(var[j][i]) + "\n";
    //         }
    //         outputStreams[id - 1].flush();
    //     }
    // }
}

template <typename T>
void replicatedSecretShare<T>::ss_output(int id, T **var, int size, std::string type, std::ofstream *outputStreams) {
    // for (int j = 0; j < numShares; j++) {
    //     for (int i = 0; i < size; i++) {
    //         if (j != (numShares - 1))
    //             outputStreams[id - 1] << std::to_string(var[j][i]) + ";";
    //         else {
    //             if (i != (size - 1))
    //                 outputStreams[id - 1] << std::to_string(var[j][i]) + ",";
    //             else
    //                 outputStreams[id - 1] << std::to_string(var[j][i]) + "\n";
    //         }
    //         outputStreams[id - 1].flush();
    //     }
    // }
}

template <typename T>
void replicatedSecretShare<T>::ss_output(int id, int *var, int size, std::string type, std::ofstream *outputStreams) {
    // std::string value;
    // for (int i = 0; i < size; i++) {
    //     std::stringstream s;
    //     s << var[i];
    //     if (i != (size - 1))
    //         outputStreams[id - 1] << s.str() + ",";
    //     else
    //         outputStreams[id - 1] << s.str() + "\n";
    //     outputStreams[id - 1].flush();
    // }
}

template <typename T>
void replicatedSecretShare<T>::ss_output(int id, T ***var, int size, std::string type, std::ofstream *outputStreams) {
    // for (int i = 0; i < size; i++) {              // total number of floats
    //     for (int j = 0; j < 4; j++) {             // always will be 4 PER LINE
    //         for (int k = 0; k < numShares; k++) { // numShares
    //             if (k != (numShares - 1))
    //                 outputStreams[id - 1] << std::to_string(var[k][i][j]) + ";";
    //             else {
    //                 if (j != 3)
    //                     outputStreams[id - 1] << std::to_string(var[k][i][j]) + ",";
    //                 else
    //                     outputStreams[id - 1] << std::to_string(var[k][i][j]) + "\n";
    //             }
    //             outputStreams[id - 1].flush();
    //         }
    //     }
    // }
}

template <typename T>
void replicatedSecretShare<T>::ss_output(int id, float *var, int size, std::string type, std::ofstream *outputStreams) {
    // std::string value;
    // for (int i = 0; i < size; i++) {
    //     std::stringstream s;
    //     s << var[i];
    //     if (i != (size - 1))
    //         outputStreams[id - 1] << s.str() + ",";
    //     else
    //         outputStreams[id - 1] << s.str() + "\n";
    //     outputStreams[id - 1].flush();
    // }
}

// x is private, but y is PUBLIC constant (but stored in T)?
template <typename T>
void replicatedSecretShare<T>::modMul(T *result, T *x, T y) {
    for (size_t i = 0; i < numShares; i++) {
        result[i] = x[i] * y;
    }
}

// we
template <typename T>
void replicatedSecretShare<T>::modMul(T *result, T *x, T *y) {
    for (size_t i = 0; i < numShares; i++) {
        result[i] = x[i] * y[0];
    }
}

template <typename T>
void replicatedSecretShare<T>::modMul(T **result, T **x, T *y, int size) {
    for (size_t i = 0; i < numShares; i++) {
        for (size_t j = 0; j < size; j++)
            result[i][j] = x[i][j] * y[j];
    }
}

template <typename T>
void replicatedSecretShare<T>::modMul(T *result, T *x, long y) {
    for (size_t i = 0; i < numShares; i++) {
        result[i] = x[i] * (T)y;
    }
}

template <typename T>
void replicatedSecretShare<T>::modMul(T *result, T *x, int y) {
    for (size_t i = 0; i < numShares; i++) {
        result[i] = x[i] * (T)y;
    }
}

template <typename T>
void replicatedSecretShare<T>::modMul(T **result, T **x, long y, int size) {
    for (size_t i = 0; i < numShares; i++) {
        for (size_t j = 0; j < size; j++)
            result[i][j] = x[i][j] * (T)y;
    }
}

template <typename T>
void replicatedSecretShare<T>::modMul(T **result, T **x, T y, int size) {
    for (size_t i = 0; i < numShares; i++) {
        for (size_t j = 0; j < size; j++)
            result[i][j] = x[i][j] * y;
    }
}

// double check, do we have to do anything if y is negative??
template <typename T>
void replicatedSecretShare<T>::modMul(T **result, T **x, int *y, int size) {
    for (size_t i = 0; i < numShares; i++) {
        for (size_t j = 0; j < size; j++)
            result[i][j] = x[i][j] * (T)y[j];
    }
}

// adding two private vals
template <typename T>
void replicatedSecretShare<T>::modAdd(T *result, T *x, T *y) {
    for (size_t i = 0; i < numShares; i++)
        result[i] = x[i] + y[i];
}

template <typename T>
void replicatedSecretShare<T>::modAdd(T **result, T **x, T **y, int size) {
    for (size_t i = 0; i < numShares; i++) {
        for (size_t j = 0; j < size; j++)
            result[i][j] = x[i][j] + y[i][j];
    }
}

template <typename T>
void replicatedSecretShare<T>::modAdd(T *result, T *x, long y) {
    for (size_t i = 0; i < numShares; i++)
        result[i] = x[i] + T(y) * const_map[i];
}

template <typename T>
void replicatedSecretShare<T>::modAdd(T **result, T **x, long y, int size) {
    for (size_t i = 0; i < numShares; i++) {
        for (size_t j = 0; j < size; j++)
            result[i][j] = x[i][j] + T(y) * const_map[i];
    }
}

// adding one secret y to every x[j]
template <typename T>
void replicatedSecretShare<T>::modAdd(T **result, T **x, T *y, int size) {
    for (size_t i = 0; i < numShares; i++) {
        for (size_t j = 0; j < size; j++)
            result[i][j] = x[i][j] + y[i];
    }
}

template <typename T>
void replicatedSecretShare<T>::modAdd(T **result, T **x, long *y, int size) {
    for (size_t i = 0; i < numShares; i++) {
        for (size_t j = 0; j < size; j++)
            result[i][j] = x[i][j] + T(y[j]) * const_map[i];
    }
}

template <typename T>
void replicatedSecretShare<T>::modAdd(T **result, T **x, int *y, int size) {
    for (size_t i = 0; i < numShares; i++) {
        for (size_t j = 0; j < size; j++)
            result[i][j] = x[i][j] + T(y[j]) * const_map[i];
    }
}

template <typename T>
void replicatedSecretShare<T>::modSub(T *result, T *x, T *y) {
    for (size_t i = 0; i < numShares; i++)
        result[i] = x[i] - y[i];
}

template <typename T>
void replicatedSecretShare<T>::modSub(T **result, T **x, T **y, int size) {
    for (size_t i = 0; i < numShares; i++) {
        for (size_t j = 0; j < size; j++)
            result[i][j] = x[i][j] - y[i][j];
    }
}

// check
template <typename T>
void replicatedSecretShare<T>::modSub(T *result, T *x, long y) {
    for (size_t i = 0; i < numShares; i++)
        result[i] = x[i] - T(y) * const_map[i];
}

template <typename T>
void replicatedSecretShare<T>::modSub(T *result, long x, T *y) {
    for (size_t i = 0; i < numShares; i++)
        result[i] = T(x) * const_map[i] - y[i];
}

template <typename T>
void replicatedSecretShare<T>::modSub(T **result, T **x, long y, int size) {
    for (size_t i = 0; i < numShares; i++) {
        for (size_t j = 0; j < size; j++)
            result[i][j] = x[i][j] - T(y) * const_map[i];
    }
}

template <typename T>
void replicatedSecretShare<T>::modSub(T **result, long x, T **y, int size) {
    for (size_t i = 0; i < numShares; i++)
        for (size_t j = 0; j < size; j++)
            result[i][j] = T(x) * const_map[i] - y[i][j];
}

template <typename T>
void replicatedSecretShare<T>::modSub(T **result, T **x, T *y, int size) {
    for (int i = 0; i < size; ++i)
        modSub(result[i], x[i], y);
}

template <typename T>
void replicatedSecretShare<T>::modSub(T **result, T *x, T **y, int size) {
    for (int i = 0; i < size; ++i)
        modSub(result[i], x, y[i]);
}

template <typename T>
void replicatedSecretShare<T>::modSub(T **result, T **x, int *y, int size) {
    for (size_t i = 0; i < numShares; i++)
        for (size_t j = 0; j < size; j++)
            result[i][j] = x[i][j] - T(y[j]) * const_map[i];
}
template <typename T>
void replicatedSecretShare<T>::modSub(T **result, int *x, T **y, int size) {
    for (size_t i = 0; i < numShares; i++)
        for (size_t j = 0; j < size; j++)
            result[i][j] = T(x[j]) * const_map[i] - y[i][j];
}

// this is DIFFERENT from the shamir version, since we need to store 2^exponent in result
template <typename T>
void replicatedSecretShare<T>::modPow2(T &result, int exponent) {
    result = 1 << T(exponent);
}

template <typename T>
void replicatedSecretShare<T>::modPow2(T &result, T exponent) {
    result = 1 << exponent;
}

template <typename T>
void replicatedSecretShare<T>::modPow2(T *result, T *exponent, int size) {
    for (int i = 0; i < size; ++i) {
        result[i] = T(1) << exponent[i];
    }
}

// these may not be needed? not called in SMC_utils, are most likely used inside shamir protocols
template <typename T>
void replicatedSecretShare<T>::modPow(T result, T base, T exponent) {
}

template <typename T>
void replicatedSecretShare<T>::modPow(T *result, T *base, T *exponent, int size) {
}

template <typename T>
void replicatedSecretShare<T>::modPow(T result, T base, long exponent) {
}

template <typename T>
void replicatedSecretShare<T>::modPow(T *result, T *base, long exponent, int size) {
}

// these are to be dealt with later, problems explained above
template <typename T>
void replicatedSecretShare<T>::modPow2(T *result, T *exponent) {
}

template <typename T>
void replicatedSecretShare<T>::modPow2(T *result, int exponent) {
}
template <typename T>
void replicatedSecretShare<T>::modPow2(T **result, T **exponent, int size) {
}
template <typename T>
void replicatedSecretShare<T>::modPow2(T **result, int *exponent, int size) {
}
template <typename T>
void replicatedSecretShare<T>::modMul(T **result, T **x, T **y, int size) {
}

template <typename T>
uint replicatedSecretShare<T>::getNumShares() {
    return numShares;
}

template <typename T>
uint replicatedSecretShare<T>::getTotalNumShares() {
    return totalNumShares;
}

template <typename T>
std::vector<int> replicatedSecretShare<T>::generateT_star(int p_star) {
    switch (n) {
    case 3:
        return {mod_n(p_star + 1, n)};
    case 5:
        return {mod_n(p_star + 1, n), mod_n(p_star + 2, n)};
    case 7:
        return {mod_n(p_star + 1, n), mod_n(p_star + 2, n), mod_n(p_star + 3, n)};
    default:
        std::cerr << "ERROR: an invalid number of parties was provided. RSS only supports n = {3,5,7}\n";
        exit(1);
    }
}

template <typename T>
int replicatedSecretShare<T>::generateT_star_index(int p_star) {
    try {

        std::vector<int> tmp;
        switch (n) {
        case 3:
            // T_{p + 1}
            // no sorting necessary
            tmp = {mod_n(p_star + 1, n)};
            break;
        case 5:
            // T_{p + 1, p + 2}
            tmp = {mod_n(p_star + 1, n), mod_n(p_star + 2, n)};
            // sorting for consistency
            sort(tmp.begin(), tmp.end());
            break;
        case 7:
            // T_{p + 1, p + 2, p + 3}
            tmp = {mod_n(p_star + 1, n), mod_n(p_star + 2, n), mod_n(p_star + 3, n)};
            sort(tmp.begin(), tmp.end());
            break;
        default:
            std::cerr << "ERROR: an invalid number of parties was provided. RSS only supports n = {3,5,7}\n";
            exit(1);
        }
        // cout << "tmp : " << tmp << endl;
        for (size_t i = 0; i < T_map_mpc.size(); i++) {
            if (tmp == T_map_mpc.at(i)) {
                return i;
            }
        }
        // cout << "index not found (party " << id << " does not have access), returning -1" << endl;
        return -1;
        // throw std::runtime_error("index not found, there's a logic error somewhere");
    } catch (const std::runtime_error &ex) {
        std::cerr << "[generateT_star_index] " << ex.what() << "\n";
        exit(1);
    }

    // should never get here
    // return -1;
}
/*
* this function is used to generate a mapping used when adding a public value to secret shares
* e.g. for 3 parties, if [a] is secret shared and b is public, and we want to compute [a] + b, each party performs the following:

in: value = 1
p1:
a_2 + result[0]*b (result[0] = 1)
a_3 + result[1]*b (result[1] = 0)
p2:
a_3 + result[0]*b (result[0] = 0)
a_1 + result[1]*b (result[1] = 0)
p3:
a_1 + result[0]*b (result[0] = 0)
a_2 + result[1]*b (result[1] = 1)

 */
template <typename T>
void replicatedSecretShare<T>::generatePublicCoef(T *result, int value) {
    // ensuring destination is sanitized
    memset(result, 0, sizeof(T) * numShares);

    switch (n) {
    case 3:
        switch (id) {
        case 1:
            result[0] = T(value); // party 1's share 1
            break;
        case 3:
            result[1] = T(value); // party 1's share 1
            break;
        default:
            break;
        }
        break;
    case 5:
        switch (id) {
        case 1:
            result[0] = T(value); // party 1's share 1
            break;
        case 4:
            result[5] = T(value); // party 1's share 1
            break;
        case 5:
            result[3] = T(value); // party 1's share 1
            break;
        default:
            break;
        }
        break;
    case 7:
        switch (id) {
        case 1:
            result[0] = T(value); // party 1's share 1
            break;
        case 5:
            result[19] = T(value); // party 1's share 1
            break;
        case 6:
            result[16] = T(value); // party 1's share 1
            break;
        case 7:
            result[10] = T(value); // party 1's share 1
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

// template <typename T>
// std::vector<std::vector<int>> replicatedSecretShare<T>::generateB2A_map() {
//     switch (n) {
//     case 3:
//         return {
//             {((id < 4) ? mod_n(id - 1, n) : -1)},
//             {(mod_n(id + 1, n) < 4 ? mod_n(id + 1, n) : -1)}};
//     case 5:
//         return {
//             // if my id < 4, compute as normal, otherwise set to -1
//             {((id < 4) ? mod_n(id - 1, n) : -1),
//              ((id < 4) ? mod_n(id - 2, n) : -1)},
//             // if the COMPUTED VALUE is < 4 , compute as normal, otherwise set to -1
//             {(mod_n(id + 1, n) < 4 ? mod_n(id + 1, n) : -1),
//              (mod_n(id + 2, n) < 4 ? mod_n(id + 2, n) : -1)},
//         };

//     case 7:
//         return {
//             // if my id < 5, compute as normal, otherwise set to -1
//             {((id < 5) ? mod_n(id - 1, n) : -1),
//              ((id < 5) ? mod_n(id - 2, n) : -1),
//              ((id < 5) ? mod_n(id - 3, n) : -1)},
//             // if the COMPUTED VALUE is < 5 , compute as normal, otherwise set to -1
//             {(mod_n(id + 1, n) < 5 ? mod_n(id + 1, n) : -1),
//              (mod_n(id + 2, n) < 5 ? mod_n(id + 2, n) : -1),
//              (mod_n(id + 3, n) < 5 ? mod_n(id + 3, n) : -1)},
//         };
//     default:
//         break;
//     }
// }

// this map is used in the updated B2A protocol
// only parties < t+1 use this
// returns the indices of the shares that party i needs to compute the local XOR of the input to B2A
// this is the most optimal implementation
template <typename T>
std::vector<int> replicatedSecretShare<T>::generateXi_map() {
    switch (n) {
    case 3:
        switch (id) {
        case 1:
            // p1 : [{2}, {3}]
            return {0, 1};
        default:
            std::cerr << "ERROR: a party other than 1 called generateXi_map\n";
            exit(1);
        }
    case 5:
        switch (id) {
        case 1:
            // p1 :  [{2,3}, {2,4}, {2,5}, {3,4}, {3,5}]
            return {0, 1, 2, 3, 4};
        case 2:
            // p2 :  [{1,3}, {1,4}, {1,5}, {4,5}]
            return {2, 3, 4, 5};
        default:
            std::cerr << "ERROR: a party other than 1,2 called generateXi_map\n";
            exit(1);
        }
    case 7:
        switch (id) {
        case 1:
            // p1 : [(2, 3, 4), (2, 3, 5), (2, 3, 6), (2, 3, 7), (2, 4, 6), (2, 4, 7), (2, 5, 6), (2, 5, 7), (2, 6, 7), (4, 6, 7), (3, 5, 7)]
            return {0, 1, 2, 3, 5, 6, 7, 8, 9, 14, 18};
        case 2:
            // p2 : [(3, 4, 5), (3, 4, 6), (3, 4, 7), (1, 3, 4), (3, 5, 6), (1, 3, 5), (3, 6, 7), (1, 3, 6), (1, 3, 7), (1, 5, 7), (1, 6, 7)]
            return {0, 1, 2, 3, 4, 6, 7, 8, 9, 18, 19};
        case 3:
            // p3 : [(4, 5, 6), (4, 5, 7), (1, 4, 5), (2, 4, 5), (1, 4, 6), (1, 4, 7), (1, 2, 4), (5, 6, 7), (1, 5, 6), (1, 2, 5), (1, 2, 6), (1, 2, 7)]
            return {0, 1, 2, 3, 5, 7, 9, 10, 11, 15, 18, 19};
        default:
            std::cerr << "ERROR: a party other than 1,2,3 called generateXi_map\n";
            exit(1);
        }
    default:
        std::cerr << "ERROR: an invalid number of parties was provided. RSS only supports n = {3,5,7}\n";
        exit(1);
    }
}

// this mapping must match whatever share is determined to be "special" (the nonzero share of \hat{b} in MultSparse)
template <typename T>
std::vector<std::vector<int>> replicatedSecretShare<T>::generate_MultSparse_map(int _n, int _id) {
    switch (_n) {
    case 3:
        return {
            // if my id > 1, compute as normal, otherwise set to -1
            {((_id > 1) ? mod_n(_id - 1, _n) : -1)},
            //  ((id < 4) ? mod_n(id - 2, n) : -1)},
            // if the COMPUTED VALUE is > 1 , compute as normal, otherwise set to -1
            {(mod_n(_id + 1, _n) > 1 ? mod_n(_id + 1, _n) : -1)},
        };

    case 5:
        return {
            // if my id > 2, compute as normal, otherwise set to -1
            {((_id > 2) ? mod_n(_id - 1, _n) : -1),
             ((_id > 2) ? mod_n(_id - 2, _n) : -1)},
            // if the COMPUTED VALUE is > 2 , compute as normal, otherwise set to -1
            {(mod_n(_id + 1, _n) > 2 ? mod_n(_id + 1, _n) : -1),
             (mod_n(_id + 2, _n) > 2 ? mod_n(_id + 2, _n) : -1)},
        };

    case 7:
        return {
            // if my id > 3, compute as normal, otherwise set to -1
            {((_id > 3) ? mod_n(_id - 1, _n) : -1),
             ((_id > 3) ? mod_n(_id - 2, _n) : -1),
             ((_id > 3) ? mod_n(_id - 3, _n) : -1)},
            // if the COMPUTED VALUE is > 3 , compute as normal, otherwise set to -1
            {(mod_n(_id + 1, _n) > 3 ? mod_n(_id + 1, _n) : -1),
             (mod_n(_id + 2, _n) > 3 ? mod_n(_id + 2, _n) : -1),
             (mod_n(_id + 3, _n) > 3 ? mod_n(_id + 3, _n) : -1)},
        };

    default:
        return {}; // should throw an error
    }
}

template <typename T>
uint replicatedSecretShare<T>::nCk(uint n, uint k) {
    if (k > n) {
        printf("Error: n must be >= k\n");
        return -1;
    } else {
        uint res = 1;
        if (k > n - k) {
            k = n - k;
        }
        for (uint i = 0; i < k; ++i) {
            res *= ((uint)n - i);
            res /= (i + (uint)1);
        }
        return res;
    }
}

// destructor
template <typename T>
replicatedSecretShare<T>::~replicatedSecretShare() {
    for (int i = 0; i < numShares + 1; i++) {
        delete[] random_container[i];
        delete[] prg_key[i];
    }

    delete[] random_container;
    delete[] prg_key;
    delete[] P_container;
    delete[] SHIFT;
    delete[] ODD;
    delete[] EVEN;
}

template <typename T>
void ss_batch_free_operator(T ***op, int size) {
}

template <typename T>
void ss_batch_free_operator(T ****op, int size) {
}

// takes an array of public values and creates sparse
// only useful for testing correctness
// in actual protocols where we deal with sparse shares, already secret-shared values would be sparsify'd, and not all parties would have access to the nonzero share
template <typename T>
void replicatedSecretShare<T>::sparsify(T **result, int *x, int size) {
    // using this function for convenience, the argument here is fixed to 'n' since we fix T_hat to always be the lexicographically first share
    // using n as an argument is a shortcut to guarantee we get the indeces of the following T_hat's
    // e.g.,
    // n = 3 -> T_hat_{1}
    // n = 5 -> T_hat_{1,2}
    // n = 7 -> T_hat_{1,2,3}
    static const int idx = generateT_star_index(n); // only needs to be done once ever
    // cout << "idx : " << idx << endl;
    if (idx >= 0) {
        for (size_t j = 0; j < size; j++)
            result[idx][j] = T(x[j]);
    }
}
template <typename T>
void replicatedSecretShare<T>::sparsify(T **result, T *x, int size) {
    // using this function for convenience, the argument here is fixed to 'n' since we fix T_hat to always be the lexicographically first share
    // using n as an argument is a shortcut to guarantee we get the indeces of the following T_hat's
    // e.g.,
    // n = 3 -> T_hat_{1}
    // n = 5 -> T_hat_{1,2}
    // n = 7 -> T_hat_{1,2,3}
    static const int idx = generateT_star_index(n); // only needs to be done once, ever
    // cout << "idx : " << idx << endl;
    if (idx >= 0) {
        for (size_t j = 0; j < size; j++)
            result[idx][j] = x[j];
    }
}

template <typename T>
void replicatedSecretShare<T>::sparsify_public(std::vector<T> &result, int x) {
    static const int idx = generateT_star_index(1); // will always be party 1's "first" share, other parties (with access) are set accordingly
    if (idx >= 0) {
        result[idx] = x;
    }
}

// the destination result is allocated to be of dimension [numShares]
template <typename T>
void replicatedSecretShare<T>::sparsify_public(T *result, int x) {
    static const int idx = generateT_star_index(1); // will always be party 1's "first" share, other parties (with access) are set accordingly
    if (idx >= 0) {
        result[idx] = x;
    }
}
// general-use "is pid in share T?"
template <typename T>
inline bool replicatedSecretShare<T>::pid_in_T(int &pid, std::vector<int> &T_map) {
    switch (n) {
    case 3:
        return (pid == T_map[0]);
    case 5:
        return (pid == T_map[0] or pid == T_map[1]);
    case 7:
        return (pid == T_map[0] or pid == T_map[1] or pid == T_map[2]);
    default:
        throw std::runtime_error("Invalid number of parties");
        return false;
    }
}

// checks if \chi(p) == T?
template <typename T>
inline bool replicatedSecretShare<T>::chi_pid_is_T(int &pid, std::vector<int> &T_map) {
    switch (n) {
    case 5: {
        return ((mod_n(pid + 1, n) == T_map[0]) and (mod_n(pid + 2, n) == T_map[1])) or
               ((mod_n(pid + 1, n) == T_map[1]) and (mod_n(pid + 2, n) == T_map[0]));
    }
    case 7: {
        int chi_0 = mod_n(pid + 1, n);
        int chi_1 = mod_n(pid + 2, n);
        int chi_2 = mod_n(pid + 3, n);

        return ((chi_0 == T_map[0] or chi_0 == T_map[1] or chi_0 == T_map[2]) and (chi_1 == T_map[0] or chi_1 == T_map[1] or chi_1 == T_map[2]) and
                (chi_2 == T_map[0] or chi_2 == T_map[1] or chi_2 == T_map[2]));
    }
    default:
        return false;
    }
}

/*
* generates the send-recv mapping for Input_p_star
* guarantees there's no "deadlocks", meaning parties will never encounter a scenario where two (or more) parties are waiting to receive from another party in the same round
* An example "deadlock" would be;:
    p2 sends to [ 1,  3] (in order)
    p3 sends to [ 1,  2] (in order)
    p1 receives from [ (2, 3), -1 ] (same round)
    p2 receives from [-1,  3]
    p3 receives from [-1,  2]
* This configuration is problematic has p2 and p3 sending to p1 in the same round
* The optimal setup would be as follows:
    p2 sends to [ 3,  1] (in order)
    p3 sends to [ 1,  2] (in order)
    p1 receives from [ 3,  2]
    p2 receives from [-1,  3]
    p3 receives from [ 2, -1]
* note, the following case CAN occur (and is NOT a deadlock)
    p2 sends to [1, 3] (in order)
    p1 receives from [2, -1]
    p3 receives from [-1, 2]
* Since p3 is not receiving in the "first round", they immediately jump to the "second round" and start waiting to receive from p2
* This is NOT considered a deadlock, because it does not incur any additional communication cost
*/
template <typename T>
std::vector<std::vector<int>> replicatedSecretShare<T>::generateInputSendRecvMap(std::vector<int> &input_parties) {
    try {

        if (input_parties.size() > n) {
            throw std::runtime_error("Cannot have more than n input parties");
        }

        uint numInputParties = input_parties.size();
        uint dim = std::max(numInputParties, uint(t));
        // uint dim = (numInputParties);
        // std::cout << "dim : " << dim << std::endl;
        std::vector<std::vector<int>> send_recv_map(2, vector<int>(dim, -1));
        // for (auto var : send_recv_map) {
        //     std::cout << var << std::endl;
        // }
        // alternate S-R mapping implementation using pairs instead of 2d vectors
        // DO NOT DELETE, may use in future (replace 2d vectors with pairs for better readability)
        // std::pair<std::vector<int>, std::vector<int>> s_r_pair(vector<int>(numInputParties, -1), vector<int>(numInputParties, -1));
        // std::cout<<s_r_pair.first<<endl;
        // std::cout<<s_r_pair.second<<endl;
        std::vector<int> T_star;

        for (auto p_star : input_parties) {
            if (p_star > n || p_star < 1) {
                throw std::runtime_error("Non-existent input party supplied :  " + std::to_string(p_star));
            }
            T_star = generateT_star(p_star);
            if (!pid_in_T(id, T_star)) {
                // id IS entitled to share T_star, so continue
                int last_pid = T_star.back();
                std::vector<int> send_ids = generateT_star(last_pid);
                // std::cout << send_ids << std::endl;
                // id is the one responsible for sending shares, and therefore needs to populate the sendIDs in the send_recv_map
                if (id == p_star) {
                    send_recv_map[0] = send_ids;
                } else {
                    // otherwise, i am a party entitled to receive this share, and need to set one of my recv_id's according to my position in send_ids
                    // looping through the send_ids of p_star
                    for (size_t i = 0; i < send_ids.size(); i++) {
                        if (id == send_ids[i]) {
                            send_recv_map[1][i] = p_star;
                        }
                    }
                }
            }
        }
        // deleting placeholder -1's
        // this is necessary for the edge case where the number of input parties is LESS than the threshold
        // e.g., if p1 is an input party, then T_star = {2,3} (which 4 and 5 are entitled to)
        // the algorithm originally produced this mapping
        // 1 sends to [4, 5]
        // 4 receives from [1]
        // 5 receives from [-1]
        // this is obviously incorrect, since 5 never gets the share it was entitled to
        // the first solution we attempted is to initalize send_recv_map based on max(numInputParties, threshold)
        // the logic here is that an input party will need to communicate the computed share to exactly (threshold) number of parties
        // but this introduces a problem in send/recv, since in send/recv, the data p5 receives from p1 will be written out-of-bounds of the recv_buffer (which is allocated based on the number of input parties, as to not waste memory)
        // The below solution just removes the "-1" placeholders that were introduced above to ensure the recv indices were placed appropriately
        // The complecity is linear in std::max(numInputParties, uint(t)), which is very fast
        send_recv_map[1].erase(std::remove(send_recv_map[1].begin(), send_recv_map[1].end(), -1), send_recv_map[1].end());
        send_recv_map[0].erase(std::remove(send_recv_map[0].begin(), send_recv_map[0].end(), -1), send_recv_map[0].end());
        return send_recv_map;
    } catch (const std::runtime_error &ex) {
        std::cerr << "[generateInputSendRecvMap] " << ex.what() << "\n";
        exit(1);
    }
}

#endif
