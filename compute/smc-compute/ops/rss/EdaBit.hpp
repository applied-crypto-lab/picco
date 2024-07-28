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
#include <iostream>
#include <numeric>

// bitlength -> bitlength of the output
// updated, generic version
template <typename T>
void edaBit(T **r, T **b_2, uint bitlength, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    assertm((ring_size == ss->ring_size), "checking ring_size argument == ss->ring_size");
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
        uint8_t *buffer = new uint8_t[size * bytes];
        ss->prg_getrandom(bytes, size, buffer);
        T *r_val = new T[size];
        memset(r_val, 0, sizeof(T) * size);

        if (bitlength == ring_size) {
            for (size_t i = 0; i < size; i++) {
                memcpy(r_val + i, buffer + i * bytes, bytes);
            }
        } else if (bitlength < ring_size) {
            for (size_t i = 0; i < size; i++) {
                memcpy(r_val + i, buffer + i * bytes, bytes);
                r_val[i] = r_val[i] & ss->SHIFT[bitlength];
            }
        } else {
            std::cerr << "bitlength cannot exceede the ring size: " << bitlength << " > " << ring_size << endl;
        }

        Rss_Input_edaBit(result, r_val, input_parties, size, ring_size, nodeNet, ss);

        delete[] buffer; // not needed anymore
        delete[] r_val;  // not needed anymore
    } else {
        // std::cout << "id " << id << " is NOT an input party" << std::endl;
        Rss_Input_edaBit(result, static_cast<T *>(nullptr), input_parties, size, ring_size, nodeNet, ss);
    }

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

        Rss_BitAdd(b_2, A_buff, B_buff, bitlength, bitlength, size, ring_size, nodeNet, ss);

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
        Rss_BitAdd(temp, A_buff, B_buff, bitlength, bitlength, size, ring_size, nodeNet, ss); // WRONG ARGS
        uint reslen = std::min(bitlength + 1, ring_size);                                     // not needed here, since we're just interested in the lengths of the inputs

        for (size_t s = 0; s < numShares; s++) {
            memcpy(A_buff[s], result[2][s] + size, sizeof(T) * size);
        }

        Rss_BitAdd(b_2, temp, A_buff, reslen, bitlength, size, ring_size, nodeNet, ss);

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
        Rss_BitAdd(C_buff, A_buff, B_buff, bitlength, bitlength, 2 * size, ring_size, nodeNet, ss);
        uint reslen = std::min(bitlength + 1, ring_size); // not needed here, since we're just interested in the lengths of the inputs
        for (size_t s = 0; s < numShares; s++) {
            // memset(A_buff[s], 0, sizeof(T) *  size); // sanitizing destination
            // memset(B_buff[s], 0, sizeof(T) *  size); // sanitizing destination

            memcpy(A_buff[s], C_buff[s], sizeof(T) * size);
            memcpy(B_buff[s], C_buff[s] + size, sizeof(T) * size);
        }

        Rss_BitAdd(b_2, A_buff, B_buff, reslen, reslen, size, ring_size, nodeNet, ss);

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
        uint numCarry = std::min(uint(ceil(log2(threshold + 1))), ring_size - bitlength);
        // cout << "numCarry : " << numCarry << endl;
        T **buffer = new T *[numShares];
        T **res = new T *[numShares];
        for (size_t s = 0; s < numShares; s++) {
            buffer[s] = new T[numCarry * size];
            res[s] = new T[numCarry * size];
            memset(buffer[s], 0, sizeof(T) * numCarry * size); // sanitizing destination
            memset(res[s], 0, sizeof(T) * numCarry * size);    // sanitizing destination
        }
        for (size_t s = 0; s < numShares; s++) {
            for (size_t m = 0; m < numCarry; m++) {
                for (size_t i = 0; i < size; i++) {
                    buffer[s][m * size + i] = GET_BIT(b_2[s][i], T(bitlength + m)); // carries start at the (bitlength) position
                }
            }
        }
        Rss_B2A(res, buffer, numCarry * size, ring_size, nodeNet, ss);
        for (size_t s = 0; s < numShares; s++) {
            memset(buffer[s], 0, sizeof(T) * numCarry * size); // sanitizing destination
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

                // removing any bits > (bitlength-1), since they are irrelevant may cause interference
                b_2[s][i] = b_2[s][i] & ss->SHIFT[bitlength];
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

// m = number of bits being truncated
// r -  full size share (z2k)
// r_hat - k-1-m bit share (z2k)
// b_2 - individual bits of r (z2), used for deterministic truncation
// b_km1 - MSB of r, shares over z2k
template <typename T>
void edaBit_Trunc(T **r, T **r_hat, T **b_2, T **b_km1, uint m, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    assertm((ring_size == ss->ring_size), "checking ring_size argument == ss->ring_size");

    static int threshold = ss->getThreshold();
    static int numParties = ss->getPeers();
    static int id = ss->getID();
    static uint numShares = ss->getNumShares();
    uint bytes = (ring_size + 7) >> 3;

    uint total_size = 3 * size; // for shares in 2*Z_2k and Z_2
    std::vector<int> input_parties(threshold + 1);
    std::iota(input_parties.begin(), input_parties.end(), 1);
    int numInputParties = input_parties.size();

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
        memset(r_hat[s], 0, sizeof(T) * size);
        memset(b_km1[s], 0, sizeof(T) * size);
    }

    if (id <= threshold + 1) {
        uint8_t *buffer = new uint8_t[size * bytes];
        ss->prg_getrandom(bytes, size, buffer);
        T *r_val = new T[size];
        memset(r_val, 0, sizeof(T) * size);
        for (size_t i = 0; i < size; i++) {
            memcpy(r_val + i, buffer + i * bytes, bytes);
        }

        Rss_Input_edaBit_Trunc(result, r_val, input_parties, m, size, ring_size, nodeNet, ss);

        delete[] buffer; // not needed anymore
        delete[] r_val;  // not needed anymore
    } else {
        // std::cout << "id " << id << " is NOT an input party" << std::endl;
        Rss_Input_edaBit_Trunc(result, static_cast<T *>(nullptr), input_parties, m, size, ring_size, nodeNet, ss);
    }

    // first (size) elements of result are the shares over Z_2k
    // summing all the random values shared in Z_2k
    for (size_t in = 0; in < numInputParties; in++) {
        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                r[s][i] += result[in][s][i];
                r_hat[s][i] += result[in][s][2 * size + i];
            }
        }
    }

    switch (numParties) {
    case 3: {
        T **A_buff = new T *[numShares];
        T **B_buff = new T *[numShares];
        T **temp_carry = new T *[numShares];
        for (size_t s = 0; s < numShares; s++) {
            A_buff[s] = new T[size];
            B_buff[s] = new T[size];
            memcpy(A_buff[s], result[0][s] + size, sizeof(T) * size);
            memcpy(B_buff[s], result[1][s] + size, sizeof(T) * size);
            temp_carry[s] = new T[size * 3]; // 3*size for k-1 carry, mth carry, and b_k-1
            memset(temp_carry[s], 0, sizeof(T) * size * 3);
        }

        Rss_BitAdd_Trunc(b_2, temp_carry, A_buff, B_buff, ring_size, ring_size, m, 0, size, ring_size, nodeNet, ss);
        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                temp_carry[s][2 * size + i] = GET_BIT(b_2[s][i], T(ring_size - 1));
            }
        }
        Rss_B2A(temp_carry, temp_carry, 3 * size, ring_size, nodeNet, ss);

        // getting msb (that we just converted from z2 to z2k) and moving it to b_km1
        // can we theoretically use an assignment operator?
        // potential problem is that this would lead to a memory leak, would need to free b_2's memory (allocated inside trunc) first, and then make sure we dont free it again at the end of trunc
        // this is viable, since edaBit_trunc should only be called inside truncation
        // leaving for now since we know it's correct
        for (size_t s = 0; s < numShares; s++) {
            memcpy(b_km1[s], temp_carry[s] + 2 * (size), size * sizeof(T));
        }

        // this line is missing subtracting the left-shifted MSB
        // adding m-1 and subtracting k carries
        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                r_hat[s][i] += temp_carry[s][i] - ((temp_carry[s][size + i]) << T(ring_size - m));
                // NOTE WE DO NOT SUBTRACT b_k-1 * 2^{k-m-1} HERE, WE DO IT INSIDE TRUNCATION
                // r_hat[s][i] -= b_km1[s][i] << T(ring_size - m -1);
            }
        }

        for (size_t i = 0; i < numShares; i++) {
            delete[] A_buff[i];
            delete[] B_buff[i];
            delete[] temp_carry[i];
        }
        delete[] A_buff;
        delete[] B_buff;
        delete[] temp_carry;
        break;
    }
    case 5: {
        T **A_buff = new T *[numShares];
        T **B_buff = new T *[numShares];
        T **temp = new T *[numShares];
        T **temp_carry = new T *[numShares];
        for (size_t s = 0; s < numShares; s++) {
            A_buff[s] = new T[size];
            B_buff[s] = new T[size];
            memcpy(A_buff[s], result[0][s] + size, sizeof(T) * size);
            memcpy(B_buff[s], result[1][s] + size, sizeof(T) * size);

            temp_carry[s] = new T[size * 5]; // 2 carries from first bitAdd, 2 carries from second bitAdd, and MSB of result
            memset(temp_carry[s], 0, sizeof(T) * size * 5);
            temp[s] = new T[size];
            memset(temp[s], 0, sizeof(T) * size);
        }
        Rss_BitAdd_Trunc(temp, temp_carry, A_buff, B_buff, ring_size, ring_size, m, 0, size, ring_size, nodeNet, ss);

        for (size_t s = 0; s < numShares; s++) {
            memcpy(A_buff[s], result[2][s] + size, sizeof(T) * size);
        }

        Rss_BitAdd_Trunc(b_2, temp_carry, temp, A_buff, ring_size, ring_size, m, 2 * size, size, ring_size, nodeNet, ss);

        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                temp_carry[s][4 * size + i] = GET_BIT(b_2[s][i], T(ring_size - 1));
            }
        }

        Rss_B2A(temp_carry, temp_carry, 5 * size, ring_size, nodeNet, ss);

        for (size_t s = 0; s < numShares; s++) {
            memcpy(b_km1[s], temp_carry[s] + 4 * (size), size * sizeof(T));
        }

        // adding m-1 and subtracting k carries
        // check if this is correct
        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                // NOTE WE DO NOT SUBTRACT b_k-1 * 2^{k-m-1} HERE, WE DO IT INSIDE TRUNCATION
                r_hat[s][i] += temp_carry[s][i] - ((temp_carry[s][size + i]) << T(ring_size - m));
                r_hat[s][i] += temp_carry[s][2 * size + i] - ((temp_carry[s][3 * size + i]) << T(ring_size - m));
            }
        }

        for (size_t i = 0; i < numShares; i++) {
            delete[] temp_carry[i];
            delete[] temp[i];
            delete[] A_buff[i];
            delete[] B_buff[i];
        }
        delete[] temp_carry;
        delete[] temp;
        delete[] A_buff;
        delete[] B_buff;
        break;
    }
    case 7: {
        T **A_buff = new T *[numShares];
        T **B_buff = new T *[numShares];
        T **C_buff = new T *[numShares];
        T **temp_carry = new T *[numShares];
        for (size_t s = 0; s < numShares; s++) {
            A_buff[s] = new T[2 * size];
            B_buff[s] = new T[2 * size];

            C_buff[s] = new T[2 * size];
            memset(C_buff[s], 0, sizeof(T) * 2 * size);
            temp_carry[s] = new T[size * 7]; // 7 for 2 carries per bitAdd (3), plus one for MSB of res
            memset(temp_carry[s], 0, sizeof(T) * size * 7);

            memcpy(A_buff[s], result[0][s] + size, sizeof(T) * size);
            memcpy(A_buff[s] + size, result[1][s] + size, sizeof(T) * size);
            memcpy(B_buff[s], result[2][s] + size, sizeof(T) * size);
            memcpy(B_buff[s] + size, result[3][s] + size, sizeof(T) * size);
        }

        // this can theoretically be done with a Mult_and_MultSparse special function
        Rss_BitAdd_Trunc(C_buff, temp_carry, A_buff, B_buff, ring_size, ring_size, m, 0, 2 * size, ring_size, nodeNet, ss);

        for (size_t s = 0; s < numShares; s++) {
            memcpy(A_buff[s], C_buff[s], sizeof(T) * size);
            memcpy(B_buff[s], C_buff[s] + size, sizeof(T) * size);
        }

        Rss_BitAdd_Trunc(b_2, temp_carry, A_buff, B_buff, ring_size, ring_size, m, 4 * size, size, ring_size, nodeNet, ss);
        /*
        NOTICE BEFORE PROCEEDING

        THE ORDER OF THE FIRST GROUPS CARRY BITS INSERTED INTO THE CARRY BUFFER FOR 7 PARTIES ARE INCONSISTENT

        the data is ordered as follows:

        carry_buffer[numShares][7*size]:
        0 (m-1, first half of BA 1)
        1 (m-1, second half of BA 1)
        2 (k-1, first half of BA 1)
        3 (k-1, second half of BA 1)
        4 (m-1, BA 2)
        5 (k-1, BA 2)

        therfore, to remove the carry bits, we need to be careful when computing the summation
         */

        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                temp_carry[s][6 * size + i] = GET_BIT(b_2[s][i], T(ring_size - 1));
            }
        }

        Rss_B2A(temp_carry, temp_carry, 7 * size, ring_size, nodeNet, ss);

        for (size_t s = 0; s < numShares; s++) {
            memcpy(b_km1[s], temp_carry[s] + 6 * (size), size * sizeof(T));
        }

        // check if this is correct
        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                // NOTE WE DO NOT SUBTRACT b_k-1 * 2^{k-m-1} HERE, WE DO IT INSIDE TRUNCATION
                r_hat[s][i] += temp_carry[s][i] - ((temp_carry[s][2 * size + i]) << T(ring_size - m));
                r_hat[s][i] += temp_carry[s][size + i] - ((temp_carry[s][3 * size + i]) << T(ring_size - m));
                r_hat[s][i] += temp_carry[s][4 * size + i] - ((temp_carry[s][5 * size + i]) << T(ring_size - m));
            }
        }

        for (size_t i = 0; i < numShares; i++) {
            delete[] A_buff[i];
            delete[] B_buff[i];
            delete[] temp_carry[i];
            delete[] C_buff[i];
        }
        delete[] A_buff;
        delete[] temp_carry;
        delete[] B_buff;
        delete[] C_buff;
        break;
    }
    default:
        break;
    }

    for (size_t s = 0; s < threshold + 1; s++) {
        for (size_t i = 0; i < numShares; i++) {
            delete[] result[s][i];
        }
        delete[] result[s];
    }
    delete[] result;
}

// m = number of bits being truncated
// r -  full size share (z2k)
// r_hat - k-1-m bit share (z2k)
// r_hat_hat - k-1-m-2 bit share (z2k)
// b_2 - individual bits of r (z2), used for deterministic truncation
// b_km1 - MSB of r, shares over z2k
template <typename T>
void edaBit_RNTE(T **r, T **r_hat, T **r_hat_hat, T **b_2, T **b_km1, uint m, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    assertm((ring_size == ss->ring_size), "checking ring_size argument == ss->ring_size");

    static int threshold = ss->getThreshold();
    static int numParties = ss->getPeers();
    static int id = ss->getID();
    static uint numShares = ss->getNumShares();
    uint bytes = (ring_size + 7) >> 3;

    uint total_size = 4 * size; // for shares in 3*Z_2k and Z_2
    std::vector<int> input_parties(threshold + 1);
    std::iota(input_parties.begin(), input_parties.end(), 1);
    int numInputParties = input_parties.size();

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
        memset(r_hat[s], 0, sizeof(T) * size);
        memset(r_hat_hat[s], 0, sizeof(T) * size);
        memset(b_km1[s], 0, sizeof(T) * size);
    }

    if (id <= threshold + 1) {
        uint8_t *buffer = new uint8_t[size * bytes];
        ss->prg_getrandom(bytes, size, buffer);
        T *r_val = new T[size];
        memset(r_val, 0, sizeof(T) * size);
        for (size_t i = 0; i < size; i++) {
            memcpy(r_val + i, buffer + i * bytes, bytes);
        }

        Rss_Input_edaBit_RNTE(result, r_val, input_parties, m, size, ring_size, nodeNet, ss);

        delete[] buffer; // not needed anymore
        delete[] r_val;  // not needed anymore
    } else {
        // std::cout << "id " << id << " is NOT an input party" << std::endl;
        Rss_Input_edaBit_RNTE(result, static_cast<T *>(nullptr), input_parties, m, size, ring_size, nodeNet, ss);
    }

    // first (size) elements of result are the shares over Z_2k
    // summing all the random values shared in Z_2k
    for (size_t in = 0; in < numInputParties; in++) {
        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                r[s][i] += result[in][s][i];
                r_hat[s][i] += result[in][s][2 * size + i];
                r_hat_hat[s][i] += result[in][s][3 * size + i];
            }
        }
    }

    switch (numParties) {
    case 3: {
        T **A_buff = new T *[numShares];
        T **B_buff = new T *[numShares];
        T **temp_carry = new T *[numShares];
        for (size_t s = 0; s < numShares; s++) {
            A_buff[s] = new T[size];
            B_buff[s] = new T[size];
            memcpy(A_buff[s], result[0][s] + size, sizeof(T) * size);
            memcpy(B_buff[s], result[1][s] + size, sizeof(T) * size);
            temp_carry[s] = new T[size * 4]; // 3*size for k-1 carry, mth carry, and b_k-1
            memset(temp_carry[s], 0, sizeof(T) * size * 4);
        }

        Rss_BitAdd_RNTE(b_2, temp_carry, A_buff, B_buff, ring_size, ring_size, m, 0, size, ring_size, nodeNet, ss);
        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                temp_carry[s][3 * size + i] = GET_BIT(b_2[s][i], T(ring_size - 1));
            }
        }
        Rss_B2A(temp_carry, temp_carry, 4 * size, ring_size, nodeNet, ss);

        // getting msb (that we just converted from z2 to z2k) and moving it to b_km1
        // can we theoretically use an assignment operator?
        // potential problem is that this would lead to a memory leak, would need to free b_2's memory (allocated inside trunc) first, and then make sure we dont free it again at the end of trunc
        // this is viable, since edaBit_trunc should only be called inside truncation
        // leaving for now since we know it's correct
        for (size_t s = 0; s < numShares; s++) {
            memcpy(b_km1[s], temp_carry[s] + 3 * (size), size * sizeof(T));
        }

        // this line is missing subtracting the left-shifted MSB
        // adding m-1 and subtracting k carries
        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                // NOTE WE DO NOT SUBTRACT b_k-1 * 2^{k-m-1} HERE, WE DO IT INSIDE TRUNCATION
                // compiting cr_m - cr_k-1
                r_hat[s][i] += temp_carry[s][i] - ((temp_carry[s][2 * size + i]) << T(ring_size - m));
                // compiting cr_m-2 - cr_k-1
                r_hat_hat[s][i] += temp_carry[s][size + i] - ((temp_carry[s][2 * size + i]) << T(ring_size - m - 2));
            }
        }

        for (size_t i = 0; i < numShares; i++) {
            delete[] A_buff[i];
            delete[] B_buff[i];
            delete[] temp_carry[i];
        }
        delete[] A_buff;
        delete[] B_buff;
        delete[] temp_carry;
        break;
    }
    case 5: {
        T **A_buff = new T *[numShares];
        T **B_buff = new T *[numShares];
        T **temp = new T *[numShares];
        T **temp_carry = new T *[numShares];
        for (size_t s = 0; s < numShares; s++) {
            A_buff[s] = new T[size];
            B_buff[s] = new T[size];
            memcpy(A_buff[s], result[0][s] + size, sizeof(T) * size);
            memcpy(B_buff[s], result[1][s] + size, sizeof(T) * size);

            temp_carry[s] = new T[size * 7]; // 2 carries from first bitAdd, 2 carries from second bitAdd, and MSB of result
            memset(temp_carry[s], 0, sizeof(T) * size * 7);
            temp[s] = new T[size];
            memset(temp[s], 0, sizeof(T) * size);
        }
        Rss_BitAdd_RNTE(temp, temp_carry, A_buff, B_buff, ring_size, ring_size, m, 0, size, ring_size, nodeNet, ss);

        for (size_t s = 0; s < numShares; s++) {
            memcpy(A_buff[s], result[2][s] + size, sizeof(T) * size);
        }

        Rss_BitAdd_RNTE(b_2, temp_carry, temp, A_buff, ring_size, ring_size, m, 3 * size, size, ring_size, nodeNet, ss);

        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                temp_carry[s][6 * size + i] = GET_BIT(b_2[s][i], T(ring_size - 1));
            }
        }

        Rss_B2A(temp_carry, temp_carry, 7 * size, ring_size, nodeNet, ss);

        for (size_t s = 0; s < numShares; s++) {
            memcpy(b_km1[s], temp_carry[s] + 6 * (size), size * sizeof(T));
        }

        // adding m-1 and subtracting k carries
        // check if this is correct
        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                // NOTE WE DO NOT SUBTRACT b_k-1 * 2^{k-m-1} HERE, WE DO IT INSIDE TRUNCATION
                r_hat[s][i] += temp_carry[s][i] - ((temp_carry[s][2 * size + i]) << T(ring_size - m));
                r_hat[s][i] += temp_carry[s][3 * size + i] - ((temp_carry[s][5 * size + i]) << T(ring_size - m));

                r_hat_hat[s][i] += temp_carry[s][size * i] - ((temp_carry[s][2 * size + i]) << T(ring_size - m - 2));
                r_hat_hat[s][i] += temp_carry[s][4 * size + i] - ((temp_carry[s][5 * size + i]) << T(ring_size - m - 2));
            }
        }

        for (size_t i = 0; i < numShares; i++) {
            delete[] temp_carry[i];
            delete[] temp[i];
            delete[] A_buff[i];
            delete[] B_buff[i];
        }
        delete[] temp_carry;
        delete[] temp;
        delete[] A_buff;
        delete[] B_buff;
        break;
    }
    case 7: {
        T **A_buff = new T *[numShares];
        T **B_buff = new T *[numShares];
        T **C_buff = new T *[numShares];
        T **temp_carry = new T *[numShares];
        for (size_t s = 0; s < numShares; s++) {
            A_buff[s] = new T[2 * size];
            B_buff[s] = new T[2 * size];

            C_buff[s] = new T[2 * size];
            memset(C_buff[s], 0, sizeof(T) * 2 * size);
            temp_carry[s] = new T[size * 9]; // 9 for 3 carries per bitAdd (2), plus one for MSB of res
            memset(temp_carry[s], 0, sizeof(T) * size * 9);

            memcpy(A_buff[s], result[0][s] + size, sizeof(T) * size);
            memcpy(A_buff[s] + size, result[1][s] + size, sizeof(T) * size);
            memcpy(B_buff[s], result[2][s] + size, sizeof(T) * size);
            memcpy(B_buff[s] + size, result[3][s] + size, sizeof(T) * size);
        }

        // this can theoretically be done with a Mult_and_MultSparse special function
        Rss_BitAdd_RNTE(C_buff, temp_carry, A_buff, B_buff, ring_size, ring_size, m, 0, 2 * size, ring_size, nodeNet, ss);

        for (size_t s = 0; s < numShares; s++) {
            memcpy(A_buff[s], C_buff[s], sizeof(T) * size);
            memcpy(B_buff[s], C_buff[s] + size, sizeof(T) * size);
        }

        Rss_BitAdd_RNTE(b_2, temp_carry, A_buff, B_buff, ring_size, ring_size, m, 6 * size, size, ring_size, nodeNet, ss);
        /*
        NOTICE BEFORE PROCEEDING

        THE ORDER OF THE FIRST GROUPS CARRY BITS INSERTED INTO THE CARRY BUFFER FOR 7 PARTIES ARE INCONSISTENT

        the data is ordered as follows:

        carry_buffer[numShares][9*size]
        0 (m-1, first half of BA 1)
        1 (m-1, second half of BA 1)
        2 (m-3, first half of BA 1)
        3 (m-3, second half of BA 1)
        4 (k-1, first half of BA 1)
        5 (k-1, second half of BA 1)
        6 (m-1, BA 2)
        7 (m-3, BA 2)
        8 (k-1, BA 2)

        therfore, to remove the carry bits, we need to be careful when computing the summation
         */

        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                temp_carry[s][8 * size + i] = GET_BIT(b_2[s][i], T(ring_size - 1));
            }
        }

        Rss_B2A(temp_carry, temp_carry, 9 * size, ring_size, nodeNet, ss);

        for (size_t s = 0; s < numShares; s++) {
            memcpy(b_km1[s], temp_carry[s] + 8 * (size), size * sizeof(T));
        }

        // check if this is correct
        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < size; i++) {
                // NOTE WE DO NOT SUBTRACT b_k-1 * 2^{k-m-1} HERE, WE DO IT INSIDE TRUNCATION
                r_hat[s][i] += temp_carry[s][0 * size + i] - ((temp_carry[s][4 * size + i]) << T(ring_size - m));
                r_hat[s][i] += temp_carry[s][1 * size + i] - ((temp_carry[s][5 * size + i]) << T(ring_size - m));
                r_hat[s][i] += temp_carry[s][6 * size + i] - ((temp_carry[s][8 * size + i]) << T(ring_size - m));

                r_hat_hat[s][i] += temp_carry[s][2 * size + i] - ((temp_carry[s][4 * size + i]) << T(ring_size - m - 2));
                r_hat_hat[s][i] += temp_carry[s][3 * size + i] - ((temp_carry[s][5 * size + i]) << T(ring_size - m - 2));
                r_hat_hat[s][i] += temp_carry[s][7 * size + i] - ((temp_carry[s][8 * size + i]) << T(ring_size - m - 2));
            }
        }

        for (size_t i = 0; i < numShares; i++) {
            delete[] A_buff[i];
            delete[] B_buff[i];
            delete[] temp_carry[i];
            delete[] C_buff[i];
        }
        delete[] A_buff;
        delete[] temp_carry;
        delete[] B_buff;
        delete[] C_buff;
        break;
    }
    default:
        break;
    }

    for (size_t s = 0; s < threshold + 1; s++) {
        for (size_t i = 0; i < numShares; i++) {
            delete[] result[s][i];
        }
        delete[] result[s];
    }
    delete[] result;
}
#endif // _EDABIT_HPP_r
