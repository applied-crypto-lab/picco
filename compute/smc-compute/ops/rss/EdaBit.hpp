/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2024 PICCO Team
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

#ifndef _EDABIT_HPP_
#define _EDABIT_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "AddBitwise.hpp"
#include "B2A.hpp"
#include "Open.hpp"
#include <algorithm>
#include <cstring>
#include <numeric>

// bitlength -> bitlength of the output
// updated, generic version
template <typename T>
void Rss_edaBit(T **r, T **b_2, uint bitlength, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    if (bitlength > ring_size) {
        throw std::runtime_error("the bitlength cannot be larger than the ring_size; bitlength = " + std::to_string(bitlength) + ", ring_size = " + std::to_string(ring_size));
    }
    static int threshold = ss->getThreshold();
    static int numParties = ss->getPeers();
    static int id = ss->getID();
    static uint numShares = ss->getNumShares();
    uint bytes = (ring_size + 7) >> 3;

    uint total_size = 2 * size; // for shares in Z_2k and Z_2
    std::vector<int> input_parties(threshold + 1);
    std::iota(input_parties.begin(), input_parties.end(), 1);
    int numInputParties = input_parties.size();

    // std::cout << "numParties : " << numParties << std::endl;
    // std::cout << "threshold : " << threshold << std::endl;
    // std::cout << "id : " << id << std::endl;
    // std::cout << "numShares : " << numShares << std::endl;
    // std::cout << "numInputParties : " << numInputParties << std::endl;
    // std::cout << "bytes : " << bytes << std::endl;

    T ***result = new T **[threshold + 1];
    for (size_t s = 0; s < threshold + 1; s++) {
        result[s] = new T *[numShares];
        for (size_t i = 0; i < numShares; i++) {
            result[s][i] = new T[total_size];
            memset(result[s][i], 0, sizeof(T) * total_size); // sanitizing destination
        }
    }
    for (size_t s = 0; s < numShares; s++) {
        // ensuring destinations are sanitized
        memset(r[s], 0, sizeof(T) * size);
        memset(b_2[s], 0, sizeof(T) * size);
    }

    if (id <= threshold + 1) {
        // std::cout << "id " << id << " is an input party" << std::endl;
        uint8_t *buffer = new uint8_t[size*bytes];
        ss->prg_getrandom(bytes, size, buffer);
        T *r_val = new T[size];
        memset(r_val, 0, sizeof(T) * size);
        for (size_t i = 0; i < size; i++) {
            memcpy(r_val + i, buffer + i * bytes, bytes);
        }
        Rss_Input_edaBit(result, r_val, input_parties, size, ring_size, nodeNet, ss);

        delete[] buffer; // not needed anymore
        delete[] r_val;  // not needed anymore
    } else {
        // std::cout << "id " << id << " is NOT an input party" << std::endl;
        Rss_Input_edaBit(result, static_cast<T *>(nullptr), input_parties, size, ring_size, nodeNet, ss);
    }
 return;

    // printf("input_eda_done\n");

    // first (size) elements of result are the shares over Z_2k
    // summing all the random values shared in Z_2k
    for (size_t in = 0; in < numInputParties; in++) {
        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                r[s][i] += result[in][s][i];
            }
        }
    }

    switch (numParties) {
    case 3: {
        T **A_buff = new T *[numShares];
        T **B_buff = new T *[numShares];
        for (size_t s = 0; s < numShares; s++) {
            A_buff[s] = new T[size];
            B_buff[s] = new T[size];
            memcpy(A_buff[s], result[0][s] + size, sizeof(T) * size);
            memcpy(B_buff[s], result[1][s] + size, sizeof(T) * size);
        }

        Rss_BitAdd(b_2, A_buff, B_buff, bitlength, bitlength, ring_size, size, nodeNet, ss);

        for (size_t i = 0; i < numShares; i++) {
            delete[] A_buff[i];
            delete[] B_buff[i];
        }
        delete[] A_buff;
        delete[] B_buff;
        break;
    }
    case 5: {
        T **A_buff = new T *[numShares];
        T **B_buff = new T *[numShares];
        T **temp = new T *[numShares];
        for (size_t s = 0; s < numShares; s++) {
            A_buff[s] = new T[size];
            B_buff[s] = new T[size];
            memcpy(A_buff[s], result[0][s] + size, sizeof(T) * size);
            memcpy(B_buff[s], result[1][s] + size, sizeof(T) * size);

            temp[s] = new T[size];
            memset(temp[s], 0, sizeof(T) * size);
        }
        Rss_BitAdd(temp, result[0], result[1], bitlength, bitlength, ring_size, size, nodeNet, ss); // WRONG ARGS
        uint reslen = std::min(bitlength + 1, ring_size);                                           // not needed here, since we're just interested in the lengths of the inputs

        for (size_t s = 0; s < numShares; s++) {
            memcpy(A_buff[s], result[2][s] + size, sizeof(T) * size);
        }

        Rss_BitAdd(b_2, temp, A_buff, reslen, bitlength, ring_size, size, nodeNet, ss);

        for (size_t i = 0; i < numShares; i++) {
            delete[] temp[i];
            delete[] A_buff[i];
            delete[] B_buff[i];
        }
        delete[] temp;
        delete[] A_buff;
        delete[] B_buff;
        break;
    }
    case 7: {
        T **A_buff = new T *[numShares];
        T **B_buff = new T *[numShares];
        T **C_buff = new T *[numShares];
        for (size_t s = 0; s < numShares; s++) {
            A_buff[s] = new T[2 * size];
            // memset(A_buff[s], 0, sizeof(T) * 2 * size); // sanitizing destination
            B_buff[s] = new T[2 * size];
            // memset(B_buff[s], 0, sizeof(T) * 2 * size); // sanitizing destination
            C_buff[s] = new T[2 * size];
            memset(C_buff[s], 0, sizeof(T) * 2 * size); // sanitizing destination

            memcpy(A_buff[s], result[0][s] + size, sizeof(T) * size);
            memcpy(A_buff[s] + size, result[1][s] + size, sizeof(T) * size);
            memcpy(B_buff[s], result[2][s] + size, sizeof(T) * size);
            memcpy(B_buff[s] + size, result[3][s] + size, sizeof(T) * size);
        }
        // this can theoretically be done with a Mult_and_MultSparse special function
        Rss_BitAdd(C_buff, A_buff, B_buff, bitlength, bitlength, ring_size, 2 * size, nodeNet, ss);
        uint reslen = std::min(bitlength + 1, ring_size); // not needed here, since we're just interested in the lengths of the inputs
        for (size_t s = 0; s < numShares; s++) {
            // memset(A_buff[s], 0, sizeof(T) *  size); // sanitizing destination
            // memset(B_buff[s], 0, sizeof(T) *  size); // sanitizing destination

            memcpy(A_buff[s], C_buff[s], sizeof(T) * size);
            memcpy(B_buff[s], C_buff[s] + size, sizeof(T) * size);
        }

        Rss_BitAdd(b_2, A_buff, B_buff, reslen, reslen, ring_size, size, nodeNet, ss);

        for (size_t i = 0; i < numShares; i++) {
            delete[] A_buff[i];
            delete[] B_buff[i];
            delete[] C_buff[i];
        }
        delete[] A_buff;
        delete[] B_buff;
        delete[] C_buff;
        break;
    }
    default:
        break;
    }

    if (bitlength < ring_size) {
        // at this point, we know there are carries which need to be removed
        // however, any carries BEYOND k (ring_size) bits do NOT need to be converted
        // therefore, we are only interested in the smaller of the following two values:
        // the exact number of carries introduced by BitAdd : ceil(log2(t+1))
        // the difference between k and the bitlength
        uint numCarry = std::min(uint(ceil(log2(threshold))), ring_size - bitlength);

        T **buffer = new T *[numShares];
        T **res = new T *[numShares];
        for (size_t s = 0; s < numShares; s++) {
            buffer[s] = new T[numCarry * size];
            res[s] = new T[numCarry * size];
            memset(buffer[s], 0, sizeof(T) * 2 * size); // sanitizing destination
            memset(res[s], 0, sizeof(T) * 2 * size);    // sanitizing destination
        }
        for (size_t s = 0; s < numShares; s++) {
            for (size_t m = 0; m < numCarry; m++) {
                for (size_t i = 0; i < size; i++) {
                    buffer[s][m * size + i] = GET_BIT(b_2[s][i], T(bitlength + m)); // carries start at the (bitlength) position
                }
            }
        }
        Rss_B2A(res, buffer, ring_size, numCarry * size, nodeNet, ss);
        for (size_t s = 0; s < numShares; s++) {
            memset(buffer[s], 0, sizeof(T) * 2 * size); // sanitizing destination
        }

        for (size_t s = 0; s < numShares; s++) {
            for (size_t m = 0; m < numCarry; m++) {
                for (size_t i = 0; i < size; i++) {
                    buffer[s][i] += res[s][m * size + i] * (T(1) << (T(m))); // check
                }
            }
        }

        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                r[s][i] -= buffer[s][i] * (T(1) << (T(bitlength))); // check
            }
        }
        // reusing buffer for summation, since it's no longer needed

        for (size_t i = 0; i < numShares; i++) {
            delete[] buffer[i];
            delete[] res[i];
        }
        delete[] buffer;
        delete[] res;
    }

    for (size_t s = 0; s < threshold + 1; s++) {
        for (size_t i = 0; i < numShares; i++) {
            delete[] result[s][i];
        }
        delete[] result[s];
    }
    delete[] result;
}

template <typename T>
void Rss_edaBit(T **r, T **b_2, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    // int pid = ss->getID();
    uint numParties = ss->getPeers();
    // printf("numParties : %u\n",numParties);
    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;

    uint i;
    // need to multiply by the number of parties in the computation
    uint new_size = numParties * size;

    T **r_bitwise = new T *[2];
    for (i = 0; i < 2; i++) {
        r_bitwise[i] = new T[new_size];
        memset(r_bitwise[i], 0, sizeof(T) * new_size);

        // ensuring destinations are sanitized
        memset(r[i], 0, sizeof(T) * size);
        memset(b_2[i], 0, sizeof(T) * size);
    }

    Rss_GenerateRandomShares(r, r_bitwise, ring_size, size, nodeNet);

    Rss_nBitAdd(b_2, r_bitwise, ring_size, size, nodeNet);

    for (i = 0; i < 2; i++) {
        delete[] r_bitwise[i];
    }
    delete[] r_bitwise;
}

template <typename T>
void Rss_GenerateRandomShares(T **res, T **res_bitwise, uint ring_size, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // printf("start\n");
    int pid = ss->getID();
    uint i, j;
    uint bytes = (ring_size + 7) >> 3;
    // printf("bytes : %u \n", bytes);
    uint p_index = pid - 1;
    uint numParties = ss->getPeers();
    // printf("numParties : %u \n", numParties);

    // T temp0, temp1;

    // used since we have effectively double the number of values
    // since we need to represent both arithmetic and binary shares
    uint new_size = 2 * size; // DO NOT CHANGE, IT IS IRRELEVANT

    // generate a single random value, which happens to already be the sum of random bits *2^j
    // [shares (0,1)][party (0,1,2)][new_size (2*size)]
    T ***r_values = new T **[2];
    for (i = 0; i < 2; i++) {
        r_values[i] = new T *[numParties];
        for (j = 0; j < numParties; j++) {
            r_values[i][j] = new T[new_size];
            memset(r_values[i][j], 0, sizeof(T) * new_size);
        }
    }

    int gamma[2];
    switch (pid) {
    case 1:
        gamma[0] = 1;
        gamma[1] = 2;
        break;
    case 2:
        gamma[0] = 2;
        gamma[1] = 0;
        break;
    case 3:
        gamma[0] = 0;
        gamma[1] = 1;
        break;
    }

    T *r_bits = new T[size];
    memset(r_bits, 0, sizeof(T) * size);

    uint8_t *buffer = new uint8_t[bytes * new_size];
    // memset(buffer, 0, sizeof(uint8_t) * bytes * new_size);

    // each party generating a unique random value
    // printf("prg1\n");
    ss->prg_getrandom(bytes, size, buffer);
    // printf("mc1\n");
    memcpy(r_bits, buffer, size * bytes);

    // printf("prg2\n");
    // SEGFAULT HERE FOR VERY LARGE VALUES
    ss->prg_getrandom(1, bytes, new_size, buffer);

    // store arithmetic and bitwise representation sequentially
    // calculating p_i's own individual shares

    memcpy(r_values[1][p_index], buffer, size * bytes);
    memcpy(r_values[1][p_index] + size, buffer + size * bytes, size * bytes);

    for (i = 0; i < size; i++) {

        r_values[0][p_index][1 * i] = r_bits[i] - r_values[1][p_index][1 * i];
        r_values[0][p_index][size + i] = r_bits[i] ^ r_values[1][p_index][size + i];

        // r_values[0][p_index][1 * i] = T(5) - r_values[1][p_index][1 * i];
        // r_values[0][p_index][size + i] = T(5) ^ r_values[1][p_index][size + i];
    }

    // need to generate more random shares so that binary and arithmetic representations are different
    ss->prg_getrandom(0, bytes, new_size, buffer);
    memcpy(r_values[0][gamma[1]], buffer, size * bytes);
    memcpy(r_values[0][gamma[1]] + size, buffer + size * bytes, size * bytes);

    //  sending r_values[0][p_index], receiving r_values[1][gamma[0]],
    nodeNet.SendAndGetDataFromPeer(r_values[0][p_index], r_values[1][gamma[0]], new_size, ring_size);

    for (i = 0; i < numParties - 1; i++) {
        // for (i = 0; i < numParties; i++) {
        memcpy(res_bitwise[0] + i * size, r_values[0][i] + (size), size * sizeof(T));
        memcpy(res_bitwise[1] + i * size, r_values[1][i] + (size), size * sizeof(T));
    }

    for (i = 0; i < size; i++) {
        // this is so we only have two parties generating shares
        for (j = 0; j < numParties - 1; j++) {
            // adding all the parties arithmetic shares together
            res[0][i] += r_values[0][j][1 * i];
            res[1][i] += r_values[1][j][1 * i];
        }
    }

    for (i = 0; i < 2; i++) {
        for (j = 0; j < numParties; j++) {
            delete[] r_values[i][j];
        }
        delete[] r_values[i];
    }
    delete[] r_values;
    delete[] buffer;
    delete[] r_bits;
}

// template <typename T>
// void Rss_edaBit(T **r, T **b_2, uint size, uint ring_size, uint bit_length, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

//     // int pid = ss->getID();
//     uint numParties = ss->getPeers();
//     // printf("numParties : %u\n",numParties);

//     uint i;
//     // need to multiply by the number of parties in the computation
//     uint new_size = numParties * size;

//     T **r_bitwise = new T *[2];
//     for (i = 0; i < 2; i++) {
//         r_bitwise[i] = new T[new_size];
//         memset(r_bitwise[i], 0, sizeof(T) * new_size);

//         // ensuring destinations are sanitized
//         memset(r[i], 0, sizeof(T) * size);
//         memset(b_2[i], 0, sizeof(T) * size);
//     }

//     Rss_GenerateRandomShares(r, r_bitwise, ring_size, bit_length, size, nodeNet);

//     Rss_nBitAdd(b_2, r_bitwise, ring_size, size, nodeNet);

//     for (i = 0; i < 2; i++) {
//         delete[] r_bitwise[i];
//     }
//     delete[] r_bitwise;
// }

template <typename T>
void Rss_GenerateRandomShares(T **res, T **res_bitwise, uint ring_size, uint bit_length, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    int pid = ss->getID();
    uint i, j;
    uint bytes = (ring_size + 7) >> 3;
    // printf("bytes : %u \n", bytes);
    uint p_index = pid - 1;
    uint numParties = ss->getPeers();
    // T temp0, temp1;

    // used since we have effectively double the number of values
    // since we need to represent both arithmetic and binary shares
    uint new_size = 2 * size;

    // generate a single random value, which happens to already be the sum of random bits *2^j
    // [shares (0,1)][party (0,1,2)][new_size (2*size)]
    T ***r_values = new T **[2];
    for (i = 0; i < 2; i++) {
        r_values[i] = new T *[numParties];
        for (j = 0; j < numParties; j++) {
            r_values[i][j] = new T[new_size];
            memset(r_values[i][j], 0, sizeof(T) * new_size);
        }
    }

    int gamma[2];
    switch (pid) {
    case 1:
        gamma[0] = 1;
        gamma[1] = 2;
        break;
    case 2:
        gamma[0] = 2;
        gamma[1] = 0;
        break;
    case 3:
        gamma[0] = 0;
        gamma[1] = 1;
        break;
    }

    T *r_bits = new T[size];
    memset(r_bits, 0, sizeof(T) * size);

    uint8_t *buffer = new uint8_t[bytes * new_size];
    // each party generating a unique random value
    ss->prg_getrandom(bytes, size, buffer);
    for (i = 0; i < size; i++) {
        memcpy(r_bits + i, buffer + i * bytes, bytes);
        // is this what we need to do to ensure we have a shorter value?
        // or do we need to do something at the end of the computation

        // r_bits[i] = -1; // USED FOR TESTING
        r_bits[i] = r_bits[i] & ss->SHIFT[bit_length];
    }

    ss->prg_getrandom(1, bytes, new_size, buffer);

    // store arithmetic and bitwise representation sequentially
    // calculating p_i's own individual shares
    for (i = 0; i < size; i++) {
        memcpy(r_values[1][p_index] + 2 * i, buffer + (2 * i) * bytes, bytes);
        memcpy(r_values[1][p_index] + 2 * i + 1, buffer + (2 * i + 1) * bytes, bytes);
        // r_values[0][p_index][2 * i] = r_bits[i] ;
        r_values[0][p_index][2 * i] = r_bits[i] - r_values[1][p_index][2 * i];
        r_values[0][p_index][2 * i + 1] = r_bits[i] ^ r_values[1][p_index][2 * i + 1];
    }

    // need to generate more random shares so that binary and arithmetic representations are different
    ss->prg_getrandom(0, bytes, new_size, buffer);
    for (i = 0; i < size; i++) {
        memcpy(r_values[0][gamma[1]] + (2 * i), buffer + (2 * i) * bytes, bytes);
        memcpy(r_values[0][gamma[1]] + (2 * i + 1), buffer + (2 * i + 1) * bytes, bytes);
    }

    //  sending r_values[0][p_index], receiving r_values[1][gamma[0]],
    nodeNet.SendAndGetDataFromPeer(r_values[0][p_index], r_values[1][gamma[0]], new_size, ring_size);

    for (i = 0; i < size; i++) {
        for (j = 0; j < numParties; j++) {
            // adding all the parties arithmetic shares together
            // memcpy(res[0] + (3 * i + j), r_values[0][j] + (2 * i), sizeof(T));
            // memcpy(res[1] + (3 * i + j), r_values[1][j] + (2 * i), sizeof(T));
            res[0][i] += r_values[0][j][2 * i];
            res[1][i] += r_values[1][j][2 * i];

            memcpy(res_bitwise[0] + (numParties * i + j), r_values[0][j] + (2 * i + 1), sizeof(T));
            memcpy(res_bitwise[1] + (numParties * i + j), r_values[1][j] + (2 * i + 1), sizeof(T));
        }
    }

    for (i = 0; i < 2; i++) {
        for (j = 0; j < numParties; j++) {
            delete[] r_values[i][j];
        }
        delete[] r_values[i];
    }
    delete[] r_values;
    delete[] buffer;
    delete[] r_bits;
}

template <typename T>
void Rss_edaBit_trunc(T **r, T **r_prime, T **r_km1, uint size, uint ring_size, uint m, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    // int pid = ss->getID();
    uint numParties = ss->getPeers();

    uint i;

    T **r_bitwise = new T *[2];
    T **carry = new T *[2];
    T **b_2 = new T *[2];
    uint new_size = numParties * size;
    uint b2a_size = 3 * size;

    for (i = 0; i < 2; i++) {
        r_bitwise[i] = new T[new_size];
        memset(r_bitwise[i], 0, sizeof(T) * new_size);
        // carry will hold both kth and m-1th bits, in succession
        carry[i] = new T[b2a_size];
        memset(carry[i], 0, sizeof(T) * b2a_size);

        b_2[i] = new T[size];
        memset(b_2[i], 0, sizeof(T) * size);

        // ensuring destinations are sanitized
        memset(r[i], 0, sizeof(T) * size);
        memset(r_prime[i], 0, sizeof(T) * size);
        memset(r_km1[i], 0, sizeof(T) * size);
    }

    Rss_GenerateRandomShares_trunc(r, r_prime, r_bitwise, ring_size, m, size, nodeNet);

    Rss_nBitAdd_trunc(b_2, carry, r_bitwise, ring_size, m, size, nodeNet);

    Rss_b2a(carry, carry, ring_size, b2a_size, nodeNet);

    memcpy(r_km1[0], carry[0] + 2 * (size), size * sizeof(T));
    memcpy(r_km1[1], carry[1] + 2 * (size), size * sizeof(T));

    // adding m-1 and subtracting k carries
    for (size_t i = 0; i < size; i++) {

        r_prime[0][i] = r_prime[0][i] + carry[0][i] - ((carry[0][size + i]) << T(ring_size - m));
        r_prime[1][i] = r_prime[1][i] + carry[1][i] - ((carry[1][size + i]) << T(ring_size - m));
    }

    for (i = 0; i < 2; i++) {
        delete[] r_bitwise[i];
        delete[] carry[i];
        delete[] b_2[i];
    }
    delete[] r_bitwise;
    delete[] carry;
    delete[] b_2;
}

template <typename T>
void Rss_edaBit_trunc_test(T **r, T **r_prime, T **r_km1, uint size, uint ring_size, uint m, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    // int pid = ss->getID();
    uint numParties = ss->getPeers();

    uint i;

    T **r_bitwise = new T *[2];
    T **carry = new T *[2];
    T **b_2 = new T *[2];
    uint new_size = numParties * size;
    uint b2a_size = 3 * size;

    for (i = 0; i < 2; i++) {
        r_bitwise[i] = new T[new_size];
        memset(r_bitwise[i], 0, sizeof(T) * new_size);
        // carry will hold both kth and m-1th bits, in succession
        carry[i] = new T[b2a_size];
        memset(carry[i], 0, sizeof(T) * b2a_size);

        b_2[i] = new T[size];
        memset(b_2[i], 0, sizeof(T) * size);

        // ensuring destinations are sanitized
        memset(r[i], 0, sizeof(T) * size);
        memset(r_prime[i], 0, sizeof(T) * size);
        memset(r_km1[i], 0, sizeof(T) * size);
    }

    Rss_GenerateRandomShares_trunc(r, r_prime, r_bitwise, ring_size, m, size, nodeNet);

    Rss_nBitAdd_trunc(b_2, carry, r_bitwise, ring_size, m, size, nodeNet);

    Rss_b2a(carry, carry, ring_size, b2a_size, nodeNet);

    memcpy(r_km1[0], carry[0] + 2 * (size), size * sizeof(T));
    memcpy(r_km1[1], carry[1] + 2 * (size), size * sizeof(T));

    // adding m-1 and subtracting k carries
    for (size_t i = 0; i < size; i++) {

        // r_prime[0][i] = r_prime[0][i] + carry[0][i] - ((carry[0][size + i]) << T(ring_size - m));
        // r_prime[1][i] = r_prime[1][i] + carry[1][i] - ((carry[1][size + i]) << T(ring_size - m));

        r_prime[0][i] = r_prime[0][i] + carry[0][i];
        r_prime[1][i] = r_prime[1][i] + carry[1][i];
    }

    for (i = 0; i < 2; i++) {
        delete[] r_bitwise[i];
        delete[] carry[i];
        delete[] b_2[i];
    }
    delete[] r_bitwise;
    delete[] carry;
    delete[] b_2;
}

template <typename T>
void Rss_GenerateRandomShares_trunc(T **res, T **res_prime, T **res_bitwise, uint ring_size, uint m, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    int pid = ss->getID();
    uint i, j;
    uint bytes = (ring_size + 7) >> 3;
    // printf("bytes : %u \n", bytes);
    uint p_index = pid - 1;
    uint numParties = ss->getPeers();
    // printf("numParties : %u \n", numParties);

    // T temp0, temp1;

    // used since we have effectively double the number of values
    // since we need to represent both arithmetic and binary shares
    uint new_size = 3 * size;

    // generate a single random value, which happens to already be the sum of random bits *2^j
    // [shares (0,1)][party (0,1,2)][new_size (2*size)]
    T ***r_values = new T **[2];
    for (i = 0; i < 2; i++) {
        r_values[i] = new T *[numParties];
        for (j = 0; j < numParties; j++) {
            r_values[i][j] = new T[new_size];
            memset(r_values[i][j], 0, sizeof(T) * new_size);
        }
    }

    int gamma[2];
    switch (pid) {
    case 1:
        gamma[0] = 1;
        gamma[1] = 2;
        break;
    case 2:
        gamma[0] = 2;
        gamma[1] = 0;
        break;
    case 3:
        gamma[0] = 0;
        gamma[1] = 1;
        break;
    }

    T *r_bits = new T[size];
    memset(r_bits, 0, sizeof(T) * size);
    T *r_prime = new T[size];
    memset(r_prime, 0, sizeof(T) * size);

    uint8_t *buffer = new uint8_t[bytes * new_size];
    // each party generating a unique random value
    ss->prg_getrandom(bytes, size, buffer);

    memcpy(r_bits, buffer, size * bytes);

    for (i = 0; i < size; i++) {
        r_bits[i] = r_bits[i] & ss->SHIFT[ring_size];
        r_prime[i] = (r_bits[i] >> T(m));
    }

    ss->prg_getrandom(1, bytes, new_size, buffer);

    // store arithmetic and bitwise representation sequentially
    // calculating p_i's own individual shares

    memcpy(r_values[1][p_index], buffer, size * bytes);
    memcpy(r_values[1][p_index] + size, buffer + size * bytes, size * bytes);
    memcpy(r_values[1][p_index] + 2 * size, buffer + 2 * size * bytes, size * bytes);

    for (i = 0; i < size; i++) {
        r_values[0][p_index][1 * i] = r_bits[i] - r_values[1][p_index][1 * i];
        r_values[0][p_index][size + i] = r_bits[i] ^ r_values[1][p_index][size + i];
        r_values[0][p_index][2 * size + i] = r_prime[i] - r_values[1][p_index][2 * size + i];
        // r_values[0][p_index][2*size + i] = r_bits[i] - r_values[1][p_index][2*size + i];
    }

    // need to generate more random shares so that binary and arithmetic representations are different
    ss->prg_getrandom(0, bytes, new_size, buffer);
    memcpy(r_values[0][gamma[1]], buffer, size * bytes);
    memcpy(r_values[0][gamma[1]] + size, buffer + size * bytes, size * bytes);
    memcpy(r_values[0][gamma[1]] + 2 * size, buffer + 2 * size * bytes, size * bytes);

    //  sending r_values[0][p_index], receiving r_values[1][gamma[0]],
    nodeNet.SendAndGetDataFromPeer(r_values[0][p_index], r_values[1][gamma[0]], new_size, ring_size);

    for (i = 0; i < numParties - 1; i++) {
        memcpy(res_bitwise[0] + i * size, r_values[0][i] + (size), size * sizeof(T));
        memcpy(res_bitwise[1] + i * size, r_values[1][i] + (size), size * sizeof(T));
    }

    for (i = 0; i < size; i++) {
        // this is so we only have two parties generating shares
        for (j = 0; j < numParties - 1; j++) {

            // adding all the parties arithmetic shares together
            res[0][i] += r_values[0][j][1 * i];
            res[1][i] += r_values[1][j][1 * i];

            res_prime[0][i] += r_values[0][j][2 * size + i];
            res_prime[1][i] += r_values[1][j][2 * size + i];
        }
    }

    for (i = 0; i < 2; i++) {
        for (j = 0; j < numParties; j++) {
            delete[] r_values[i][j];
        }
        delete[] r_values[i];
    }
    delete[] r_values;
    delete[] buffer;
    delete[] r_bits;
    delete[] r_prime;
}

template <typename T>
void Rss_edaBit_5pc(T **r, T **b_2, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    // int pid = ss->getID();
    uint numShares = ss->getNumShares();
    uint threshold = ss->getThreshold();
    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;

    uint i;
    // this is the number of shares we need to add (t+1)
    uint new_size = (threshold + 1) * size;
    // printf("new_size: %llu\n", new_size);

    T **r_bitwise = new T *[numShares];
    for (i = 0; i < numShares; i++) {
        r_bitwise[i] = new T[new_size];
        memset(r_bitwise[i], 0, sizeof(T) * new_size);

        // ensuring destinations are sanitized
        memset(r[i], 0, sizeof(T) * size);
        memset(b_2[i], 0, sizeof(T) * size);
    }

    Rss_GenerateRandomShares_5pc(r, r_bitwise, ring_size, size, nodeNet);

    Rss_nBitAdd_5pc(b_2, r_bitwise, ring_size, size, nodeNet);

    for (i = 0; i < numShares; i++) {
        delete[] r_bitwise[i];
    }
    delete[] r_bitwise;
}

template <typename T>
void Rss_GenerateRandomShares_5pc(T **res, T **res_bitwise, uint ring_size, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // printf("start\n");
    int pid = ss->getID();
    uint i, j;
    uint bytes = (ring_size + 7) >> 3;
    uint new_size = 2 * size; // DO NOT CHANGE, IT IS IRRELEVANT for n>3
    // printf("bytes : %u \n", bytes);
    uint p_index = pid - 1;
    uint numParties = ss->getPeers();
    uint numShares = ss->getNumShares();
    uint threshold = ss->getThreshold();
    // printf("threshold : %u \n", threshold);
    // printf("numParties : %u \n", numParties);

    T **recvbuf = new T *[threshold];
    for (i = 0; i < threshold; i++) {
        recvbuf[i] = new T[new_size];
        memset(recvbuf[i], 0, sizeof(T) * new_size);
    }

    // used since we have effectively double the number of values
    // since we need to represent both arithmetic and binary shares
    // printf("new_size : %u \n", new_size);

    // generate a single random value, which happens to already be the sum of random bits *2^j
    // [shares (0,1)][party (0,1,2)][new_size (2*size)]
    T ***r_values = new T **[numShares];
    for (i = 0; i < numShares; i++) {
        r_values[i] = new T *[(threshold + 1)];
        for (j = 0; j < (threshold + 1); j++) {
            r_values[i][j] = new T[new_size];
            memset(r_values[i][j], 0, sizeof(T) * new_size); // NECESSARY FOR n>3
        }
    }

    T *r_bits = new T[size];
    memset(r_bits, 0, sizeof(T) * size);

    uint8_t *r_buffer = new uint8_t[bytes * size];

    uint8_t **buffer = new uint8_t *[numShares];
    for (uint s = 0; s < numShares; s++) {
        buffer[s] = new uint8_t[(threshold + 1) * bytes * new_size]; // we may not use all of these random bytes, but
        ss->prg_getrandom(s, bytes, (threshold + 1) * new_size, buffer[s]);
    }

    int *index_map = new int[threshold + 1];

    uint reset_value;
    switch (pid) {
    case 1:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = 3;
        index_map[2] = 5;
        break;
    case 2:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = -1;
        index_map[2] = 3;
        break;
    case 3:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = -1;
        index_map[2] = -1;
        break;
    case 4:
        reset_value = 1;
        index_map[0] = 5;
        index_map[1] = -1;
        index_map[2] = -1;
        break;
    case 5:
        reset_value = 0;
        index_map[0] = 3;
        index_map[1] = 5;
        index_map[2] = -1;
        break;
    }

    bool prg_bools[4][6] = {
        {1, 1, 0, 1, 0, 0},
        {1, 0, 1, 0, 1, 0},
        {0, 1, 1, 0, 0, 0},
        {0, 0, 0, 0, 1, 1},
    };

    if (pid < threshold + 2) {
        // p1, p2, p3, choosing random values

        ss->prg_getrandom(bytes, size, r_buffer);
        // memcpy(r_bits, r_buffer, size * bytes);

        for (i = 0; i < size; i++) {
            memcpy(r_bits + i, r_buffer + i * bytes, bytes);

            // printf("hi1\n");
            for (size_t s = 1; s < numShares; s++) {
                // memcpy(r_values[s][p_index], buffer[s], new_size * bytes);
                memcpy(r_values[s][p_index] + 2 * i, buffer[s] + (2 * i) * bytes, bytes);
                memcpy(r_values[s][p_index] + 2 * i + 1, buffer[s] + (2 * i + 1) * bytes, bytes);
            }

            // r_values[0][p_index][2 * i] = T(5 + i);
            // r_values[0][p_index][2 * i + 1] = T(5 + i);
            r_values[0][p_index][2 * i] = r_bits[i];
            r_values[0][p_index][2 * i + 1] = r_bits[i];

            for (size_t s = 1; s < numShares; s++) {
                r_values[0][p_index][2 * i] -= r_values[s][p_index][2 * i];
                r_values[0][p_index][2 * i + 1] ^= r_values[s][p_index][2 * i + 1];
            }
        }
    }

    // num parties - 1

    // printf("indices\n");
    // int n = numParties - 1;
    // printf("n: %llu\n", n);

    for (size_t i = 0; i < size; i++) {

        int index = ((pid - 2) % (numParties - 1) + (numParties - 1)) % (numParties - 1);
        for (size_t t = 0; t < threshold + 1; t++) {
            if (p_index != t) {
                // int index = (((pid - 2 - t) % ((numParties - 1))) + ((numParties - 1)) )  % ((numParties - 1) );

                // printf("%i : bool_index = %i\n", t, index);
                // loop through num_shares
                // if we're supposed to generate, then memcpy from buffer[j]
                for (size_t s = 0; s < numShares; s++) {
                    if (prg_bools[index][s]) {
                        // printf("copying: %llu\n", copying);
                        memcpy(r_values[s][t] + (2 * i), buffer[s] + (2 * i) * bytes, bytes);
                        memcpy(r_values[s][t] + (2 * i + 1), buffer[s] + (2 * i + 1) * bytes, bytes);
                    }
                }
                // printf("subtracting\(numParties - 1)");
                index = ((index - 1) % (numParties - 1) + (numParties - 1)) % (numParties - 1);
            }
        }
    }

    nodeNet.SendAndGetDataFromPeer(r_values[0][p_index], recvbuf, new_size, ring_size, ss->eda_map_mpc);

    // extracting from buffer
    for (size_t i = 0; i < size; i++) {
        j = reset_value;
        for (size_t t = 0; t < threshold + 1; t++) {

            if (index_map[t] > 0) {
                // printf("%u : putting %llu from recvbuff[%i] in %u\n", t, recvbuf[j][2 * i], j, index_map[t]);
                // printf("%u : putting %llu from recvbuff[%i] in %u\n", t, recvbuf[j][2 * i + 1], j, index_map[t]);
                memcpy(r_values[index_map[t]][t] + 2 * i, recvbuf[j] + 2 * i, sizeof(T));
                memcpy(r_values[index_map[t]][t] + 2 * i + 1, recvbuf[j] + 2 * i + 1, sizeof(T));
                j++;
            }
        }
    }

    for (size_t i = 0; i < size; i++) {
        for (uint p = 0; p < threshold + 1; p++) {
            for (size_t s = 0; s < numShares; s++) {
                res_bitwise[s][p * (size) + i] = r_values[s][p][2 * i + 1];
            }
        }
    }

    for (i = 0; i < size; i++) {
        // this is so we only have t+1 parties generating shares
        for (j = 0; j < threshold + 1; j++) {
            // adding all the parties arithmetic shares together
            for (size_t s = 0; s < numShares; s++) {
                res[s][i] += r_values[s][j][2 * i];
                // res[1][i] += r_values[1][j][1 * i];
            }
        }
    }

    delete[] index_map;
    for (i = 0; i < threshold; i++) {
        delete[] recvbuf[i];
    }

    delete[] recvbuf;

    for (i = 0; i < numShares; i++) {
        for (j = 0; j < (threshold + 1); j++) {
            delete[] r_values[i][j];
        }
        delete[] r_values[i];
    }
    delete[] r_values;
    for (i = 0; i < numShares; i++) {
        delete[] buffer[i];
    }
    delete[] buffer;
    delete[] r_buffer;
    delete[] r_bits;
}

template <typename T>
void Rss_edaBit_trunc_5pc(T **r, T **r_prime, T **r_km1, uint size, uint ring_size, uint m, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    // int pid = ss->getID();
    uint numShares = ss->getNumShares();
    uint threshold = ss->getThreshold();
    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;

    uint i;
    // this is the number of shares we need to add (t+1)
    uint new_size = (threshold + 1) * size;
    // printf("new_size: %llu\n", new_size);

    uint b2a_size = 3 * size;

    T **carry = new T *[numShares];
    T **b_2 = new T *[numShares];
    T **r_bitwise = new T *[numShares];
    for (i = 0; i < numShares; i++) {
        r_bitwise[i] = new T[new_size];
        memset(r_bitwise[i], 0, sizeof(T) * new_size);
        // carry will hold both kth and m-1th bits, in succession
        carry[i] = new T[b2a_size];
        memset(carry[i], 0, sizeof(T) * b2a_size);

        b_2[i] = new T[size];
        memset(b_2[i], 0, sizeof(T) * size);

        // ensuring destinations are sanitized
        memset(r[i], 0, sizeof(T) * size);
        memset(r_prime[i], 0, sizeof(T) * size);
        memset(r_km1[i], 0, sizeof(T) * size);
    }

    Rss_GenerateRandomShares_trunc_5pc(r, r_prime, r_bitwise, ring_size, m, size, nodeNet);

    Rss_nBitAdd_trunc_5pc(b_2, carry, r_bitwise, ring_size, m, size, nodeNet);

    Rss_b2a_5pc(carry, carry, ring_size, b2a_size, nodeNet);
    for (size_t s = 0; s < numShares; s++) {
        memcpy(r_km1[s], carry[s] + 2 * (size), size * sizeof(T));
    }

    // adding m-1 and subtracting k carries
    for (size_t i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {

            r_prime[s][i] = r_prime[s][i] + carry[s][i] - ((carry[s][size + i]) << T(ring_size - m));
        }
    }

    for (i = 0; i < numShares; i++) {
        delete[] r_bitwise[i];
        delete[] carry[i];
        delete[] b_2[i];
    }
    delete[] r_bitwise;
    delete[] carry;
    delete[] b_2;
}

template <typename T>
void Rss_GenerateRandomShares_trunc_5pc(T **res, T **res_prime, T **res_bitwise, uint ring_size, uint m, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // printf("start\n");
    int pid = ss->getID();
    uint i, j;
    uint bytes = (ring_size + 7) >> 3;
    uint new_size = 3 * size; // DO NOT CHANGE, IT IS IRRELEVANT for n>3
    // printf("bytes : %u \n", bytes);
    uint p_index = pid - 1;
    uint numParties = ss->getPeers();
    uint numShares = ss->getNumShares();
    uint threshold = ss->getThreshold();
    // printf("threshold : %u \n", threshold);
    // printf("numParties : %u \n", numParties);

    int *index_map = new int[3];

    uint reset_value;
    switch (pid) {
    case 1:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = 3;
        index_map[2] = 5;
        break;
    case 2:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = -1;
        index_map[2] = 3;
        break;
    case 3:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = -1;
        index_map[2] = -1;
        break;
    case 4:
        reset_value = 1;
        index_map[0] = 5;
        index_map[1] = -1;
        index_map[2] = -1;
        break;
    case 5:
        reset_value = 0;
        index_map[0] = 3;
        index_map[1] = 5;
        index_map[2] = -1;
        break;
    }

    bool prg_bools[4][6] = {
        {1, 1, 0, 1, 0, 0},
        {1, 0, 1, 0, 1, 0},
        {0, 1, 1, 0, 0, 0},
        {0, 0, 0, 0, 1, 1},
    };

    T **recvbuf = new T *[threshold];
    for (i = 0; i < threshold; i++) {
        recvbuf[i] = new T[new_size];
        memset(recvbuf[i], 0, sizeof(T) * new_size);
    }

    // used since we have effectively double the number of values
    // since we need to represent both arithmetic and binary shares
    // printf("new_size : %u \n", new_size);

    // generate a single random value, which happens to already be the sum of random bits *2^j
    // [shares (0,1)][party (0,1,2)][new_size (2*size)]
    T ***r_values = new T **[numShares];
    for (i = 0; i < numShares; i++) {
        r_values[i] = new T *[(threshold + 1)];
        for (j = 0; j < (threshold + 1); j++) {
            r_values[i][j] = new T[new_size];
            memset(r_values[i][j], 0, sizeof(T) * new_size); // NECESSARY FOR n>3
        }
    }

    T *r_bits = new T[size];
    memset(r_bits, 0, sizeof(T) * size);
    T *r_prime = new T[size];
    memset(r_prime, 0, sizeof(T) * size);

    uint8_t *r_buffer = new uint8_t[bytes * size];

    uint8_t **buffer = new uint8_t *[numShares];
    for (uint s = 0; s < numShares; s++) {
        buffer[s] = new uint8_t[(threshold + 1) * bytes * new_size]; // we may not use all of these random bytes, but
        ss->prg_getrandom(s, bytes, (threshold + 1) * new_size, buffer[s]);
    }

    if (pid < 4) {
        // p1, p2, p3, choosing random values

        ss->prg_getrandom(bytes, size, r_buffer);
        // memcpy(r_bits, r_buffer, size * bytes);

        for (i = 0; i < size; i++) {
            memcpy(r_bits + i, r_buffer + i * bytes, bytes);

            r_bits[i] = r_bits[i] & ss->SHIFT[ring_size];
            r_prime[i] = (r_bits[i] >> T(m));

            // printf("hi1\n");
            for (size_t s = 1; s < numShares; s++) {
                // memcpy(r_values[s][p_index], buffer[s], new_size * bytes);
                memcpy(r_values[s][p_index] + 2 * i, buffer[s] + (2 * i) * bytes, bytes);
                memcpy(r_values[s][p_index] + 2 * i + 1, buffer[s] + (2 * i + 1) * bytes, bytes);
                memcpy(r_values[s][p_index] + 2 * i + 2, buffer[s] + (2 * i + 2) * bytes, bytes);
            }

            // r_values[0][p_index][2 * i] = T(5 + i);
            // r_values[0][p_index][2 * i + 1] = T(5 + i);
            r_values[0][p_index][2 * i] = r_bits[i];
            r_values[0][p_index][2 * i + 1] = r_bits[i];
            r_values[0][p_index][2 * i + 2] = r_bits[i];

            for (size_t s = 1; s < numShares; s++) {
                r_values[0][p_index][2 * i] -= r_values[s][p_index][2 * i];
                r_values[0][p_index][2 * i + 1] ^= r_values[s][p_index][2 * i + 1];
                r_values[0][p_index][2 * i + 2] -= r_values[s][p_index][2 * i + 2];
            }
        }
    }

    for (size_t i = 0; i < size; i++) {

        int index = ((pid - 2) % (numParties - 1) + (numParties - 1)) % (numParties - 1);
        for (size_t t = 0; t < threshold + 1; t++) {
            if (p_index != t) {
                // int index = (((pid - 2 - t) % ((numParties - 1))) + ((numParties - 1)) )  % ((numParties - 1) );

                // printf("%i : bool_index = %i\n", t, index);
                // loop through num_shares
                // if we're supposed to generate, then memcpy from buffer[j]
                for (size_t s = 0; s < numShares; s++) {
                    if (prg_bools[index][s]) {
                        // printf("copying: %llu\n", copying);
                        memcpy(r_values[s][t] + (2 * i), buffer[s] + (2 * i) * bytes, bytes);
                        memcpy(r_values[s][t] + (2 * i + 1), buffer[s] + (2 * i + 1) * bytes, bytes);
                        memcpy(r_values[s][t] + (2 * i + 2), buffer[s] + (2 * i + 2) * bytes, bytes);
                    }
                }
                // printf("subtracting\(numParties - 1)");
                index = ((index - 1) % (numParties - 1) + (numParties - 1)) % (numParties - 1);
            }
        }
    }

    nodeNet.SendAndGetDataFromPeer(r_values[0][p_index], recvbuf, new_size, ring_size, ss->eda_map_mpc);

    // extracting from buffer
    for (size_t i = 0; i < size; i++) {
        j = reset_value;
        for (size_t t = 0; t < threshold + 1; t++) {

            if (index_map[t] > 0) {
                // printf("%u : putting %llu from recvbuff[%i] in %u\n", t, recvbuf[j][2 * i], j, index_map[t]);
                // printf("%u : putting %llu from recvbuff[%i] in %u\n", t, recvbuf[j][2 * i + 1], j, index_map[t]);
                memcpy(r_values[index_map[t]][t] + 2 * i, recvbuf[j] + 2 * i, sizeof(T));
                memcpy(r_values[index_map[t]][t] + 2 * i + 1, recvbuf[j] + 2 * i + 1, sizeof(T));
                memcpy(r_values[index_map[t]][t] + 2 * i + 2, recvbuf[j] + 2 * i + 2, sizeof(T));
                j++;
            }
        }
    }

    for (size_t i = 0; i < size; i++) {
        for (uint p = 0; p < threshold + 1; p++) {
            for (size_t s = 0; s < numShares; s++) {
                res_bitwise[s][p * (size) + i] = r_values[s][p][2 * i + 1];
            }
        }
    }

    for (i = 0; i < size; i++) {
        // this is so we only have t+1 parties generating shares
        for (j = 0; j < threshold + 1; j++) {
            // adding all the parties arithmetic shares together
            for (size_t s = 0; s < numShares; s++) {
                res[s][i] += r_values[s][j][2 * i];

                res_prime[s][i] += r_values[s][j][2 * i + 2];
                // res[1][i] += r_values[1][j][1 * i];
            }
        }
    }

    for (i = 0; i < threshold; i++) {
        delete[] recvbuf[i];
    }

    delete[] recvbuf;
    delete[] index_map;

    for (i = 0; i < numShares; i++) {
        for (j = 0; j < (threshold + 1); j++) {
            delete[] r_values[i][j];
        }
        delete[] r_values[i];
    }
    for (i = 0; i < numShares; i++) {
        delete[] buffer[i];
    }
    delete[] r_values;
    delete[] buffer;
    delete[] r_buffer;
    delete[] r_prime;
    delete[] r_bits;
}

template <typename T>
void Rss_edaBit_7pc(T **r, T **b_2, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    // int pid = ss->getID();
    uint numShares = ss->getNumShares();
    uint threshold = ss->getThreshold();
    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;

    uint i;
    // this is the number of shares we need to add (t+1)
    uint new_size = (threshold + 1) * size;
    // printf("new_size: %llu\n", new_size);

    T **r_bitwise = new T *[numShares];
    for (i = 0; i < numShares; i++) {
        r_bitwise[i] = new T[new_size];
        memset(r_bitwise[i], 0, sizeof(T) * new_size);

        // ensuring destinations are sanitized
        memset(r[i], 0, sizeof(T) * size);
        memset(b_2[i], 0, sizeof(T) * size);
    }

    Rss_GenerateRandomShares_7pc(r, r_bitwise, ring_size, size, nodeNet);

    Rss_nBitAdd_7pc(b_2, r_bitwise, ring_size, size, nodeNet);

    for (i = 0; i < numShares; i++) {
        delete[] r_bitwise[i];
    }
    delete[] r_bitwise;
}

template <typename T>
void Rss_GenerateRandomShares_7pc(T **res, T **res_bitwise, uint ring_size, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // printf("start\n");
    int pid = ss->getID();
    uint i, j;
    uint bytes = (ring_size + 7) >> 3;
    uint new_size = 2 * size; // DO NOT CHANGE, IT IS IRRELEVANT for n>3
    // printf("bytes : %u \n", bytes);
    uint p_index = pid - 1;
    uint numParties = ss->getPeers();
    uint numShares = ss->getNumShares();
    uint threshold = ss->getThreshold();
    // printf("threshold : %u \n", threshold);
    // printf("numParties : %u \n", numParties);

    T **recvbuf = new T *[threshold];
    for (i = 0; i < threshold; i++) {
        recvbuf[i] = new T[new_size];
        memset(recvbuf[i], 0, sizeof(T) * new_size);
    }

    // used since we have effectively double the number of values
    // since we need to represent both arithmetic and binary shares
    // printf("new_size : %u \n", new_size);

    // generate a single random value, which happens to already be the sum of random bits *2^j
    // [shares (0,1)][party (0,1,2)][new_size (2*size)]
    T ***r_values = new T **[numShares];
    for (i = 0; i < numShares; i++) {
        r_values[i] = new T *[(threshold + 1)];
        for (j = 0; j < (threshold + 1); j++) {
            r_values[i][j] = new T[new_size];
            memset(r_values[i][j], 0, sizeof(T) * new_size); // NECESSARY FOR n>3
        }
    }

    T *r_bits = new T[size];
    memset(r_bits, 0, sizeof(T) * size);

    uint8_t *r_buffer = new uint8_t[bytes * size];

    uint8_t **buffer = new uint8_t *[numShares];
    for (uint s = 0; s < numShares; s++) {
        buffer[s] = new uint8_t[(threshold + 1) * bytes * new_size]; // we may not use all of these random bytes, but
        ss->prg_getrandom(s, bytes, (threshold + 1) * new_size, buffer[s]);
    }

    int *index_map = new int[threshold + 1];

    uint reset_value;
    switch (pid) {
    case 1:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = 10;
        index_map[2] = 16;
        index_map[3] = 19;
        break;
    case 2:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = -1;
        index_map[2] = 10;
        index_map[3] = 16;
        break;
    case 3:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = -1;
        index_map[2] = -1;
        index_map[3] = 10;
        break;
    case 4:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = -1;
        index_map[2] = -1;
        index_map[3] = -1;
        break;
    case 5:
        reset_value = 2;
        index_map[0] = 19;
        index_map[1] = -1;
        index_map[2] = -1;
        index_map[3] = -1;
        break;
    case 6:
        reset_value = 1;
        index_map[0] = 16;
        index_map[1] = 19;
        index_map[2] = -1;
        index_map[3] = -1;
        break;
    case 7:
        reset_value = 0;
        index_map[0] = 10;
        index_map[1] = 16;
        index_map[2] = 19;
        index_map[3] = -1;
        break;
    }

    bool prg_bools[6][20] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
        {0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0},
        {1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0},
        {1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0},
        {1, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0}};

    if (pid < threshold + 2) {
        // p1, p2, p3, p4, choosing random values

        ss->prg_getrandom(bytes, size, r_buffer);
        // memcpy(r_bits, r_buffer, size * bytes);

        for (i = 0; i < size; i++) {
            memcpy(r_bits + i, r_buffer + i * bytes, bytes);

            // printf("hi1\n");
            for (size_t s = 1; s < numShares; s++) {
                // memcpy(r_values[s][p_index], buffer[s], new_size * bytes);
                memcpy(r_values[s][p_index] + 2 * i, buffer[s] + (2 * i) * bytes, bytes);
                memcpy(r_values[s][p_index] + 2 * i + 1, buffer[s] + (2 * i + 1) * bytes, bytes);
            }

            // r_values[0][p_index][2 * i] = T(5 + i);
            // r_values[0][p_index][2 * i + 1] = T(5 + i);
            r_values[0][p_index][2 * i] = r_bits[i];
            r_values[0][p_index][2 * i + 1] = r_bits[i];

            for (size_t s = 1; s < numShares; s++) {
                r_values[0][p_index][2 * i] -= r_values[s][p_index][2 * i];
                r_values[0][p_index][2 * i + 1] ^= r_values[s][p_index][2 * i + 1];
            }
        }
    }

    for (size_t i = 0; i < size; i++) {

        int index = ((pid - 2) % (numParties - 1) + (numParties - 1)) % (numParties - 1);
        for (size_t t = 0; t < threshold + 1; t++) {
            if (p_index != t) {
                // int index = (((pid - 2 - t) % ((numParties - 1))) + ((numParties - 1)) )  % ((numParties - 1) );

                // printf("%i : bool_index = %i\n", t, index);
                // loop through num_shares
                // if we're supposed to generate, then memcpy from buffer[j]
                for (size_t s = 0; s < numShares; s++) {
                    if (prg_bools[index][s]) {
                        // printf("copying: %llu\n", copying);
                        memcpy(r_values[s][t] + (2 * i), buffer[s] + (2 * i) * bytes, bytes);
                        memcpy(r_values[s][t] + (2 * i + 1), buffer[s] + (2 * i + 1) * bytes, bytes);
                    }
                }
                // printf("subtracting\(numParties - 1)");
                index = ((index - 1) % (numParties - 1) + (numParties - 1)) % (numParties - 1);
            }
        }
    }

    nodeNet.SendAndGetDataFromPeer(r_values[0][p_index], recvbuf, new_size, ring_size, ss->eda_map_mpc);

    // extracting from buffer
    for (size_t i = 0; i < size; i++) {
        j = reset_value;
        for (size_t t = 0; t < threshold + 1; t++) {

            if (index_map[t] > 0) {
                // printf("%u : putting %llu from recvbuff[%i] in %u\n", t, recvbuf[j][2 * i], j, index_map[t]);
                // printf("%u : putting %llu from recvbuff[%i] in %u\n", t, recvbuf[j][2 * i + 1], j, index_map[t]);
                memcpy(r_values[index_map[t]][t] + 2 * i, recvbuf[j] + 2 * i, sizeof(T));
                memcpy(r_values[index_map[t]][t] + 2 * i + 1, recvbuf[j] + 2 * i + 1, sizeof(T));
                j++;
            }
        }
    }

    for (size_t i = 0; i < size; i++) {
        for (uint p = 0; p < threshold + 1; p++) {
            for (size_t s = 0; s < numShares; s++) {
                res_bitwise[s][p * (size) + i] = r_values[s][p][2 * i + 1];
            }
        }
    }

    for (i = 0; i < size; i++) {
        // this is so we only have t+1 parties generating shares
        for (j = 0; j < threshold + 1; j++) {
            // adding all the parties arithmetic shares together
            for (size_t s = 0; s < numShares; s++) {
                res[s][i] += r_values[s][j][2 * i];
                // res[1][i] += r_values[1][j][1 * i];
            }
        }
    }

    delete[] index_map;
    for (i = 0; i < threshold; i++) {
        delete[] recvbuf[i];
    }

    delete[] recvbuf;

    for (i = 0; i < numShares; i++) {
        for (j = 0; j < (threshold + 1); j++) {
            delete[] r_values[i][j];
        }
        delete[] r_values[i];
    }
    delete[] r_values;
    for (i = 0; i < numShares; i++) {
        delete[] buffer[i];
    }
    delete[] buffer;
    delete[] r_buffer;
    delete[] r_bits;
}

#endif // _EDABIT_HPP_