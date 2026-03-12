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
// Interface format: r, b_2 are [size][numShares] where array[i][s] is share s of element i
template <typename T>
void edaBit(T **r, T **b_2, uint bitlength, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    assertm((ring_size == ss->ring_size), "checking ring_size argument == ss->ring_size");
    if (bitlength > ring_size) {
        throw std::runtime_error("the bitlength cannot be larger than the ring_size; bitlength = " + std::to_string(bitlength) + ", ring_size = " + std::to_string(ring_size));
    }
    int threshold = ss->getThreshold();
    int numParties = ss->getPeers();
    int id = ss->getID();
    uint numShares = ss->getNumShares();
    uint bytes = (ring_size + 7) >> 3;

    uint total_size = 2 * size; // for shares in Z_2k and Z_2
    std::vector<int> input_parties(threshold + 1);
    std::iota(input_parties.begin(), input_parties.end(), 1);
    int numInputParties = input_parties.size();

    // Internal result array uses [threshold+1][total_size][numShares] format to match Input.hpp
    // result[p_star_index][i][s] = share s of element i from input party p_star_index
    T ***result = new T **[threshold + 1];
    for (size_t p = 0; p < (size_t)(threshold + 1); p++) {
        result[p] = new T *[total_size];
        for (size_t i = 0; i < total_size; i++) {
            result[p][i] = new T[numShares];
            memset(result[p][i], 0, sizeof(T) * numShares); // sanitizing destination
        }
    }
    // Sanitize output arrays (interface format [size][numShares])
    for (size_t i = 0; i < size; i++) {
        memset(r[i], 0, sizeof(T) * numShares);
        memset(b_2[i], 0, sizeof(T) * numShares);
    }

    if (id <= threshold + 1) {
        uint8_t *buffer = new uint8_t[size * bytes];
        ss->prg_getrandom(bytes, size, buffer);
        T *r_val = new T[size];
        memset(r_val, 0, sizeof(T) * size);

        if (bitlength == ring_size) {
            for (size_t i = 0; i < size; i++) {
                memcpy(r_val + i, buffer + i * bytes, bytes);
                r_val[i] = r_val[i] & ss->SHIFT[ring_size];
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
        Rss_Input_edaBit(result, static_cast<T *>(nullptr), input_parties, size, ring_size, nodeNet, ss);
    }

    // first (size) elements of result are the shares over Z_2k
    // summing all the random values shared in Z_2k
    // Convert from internal format result[in][i][s] to interface format r[i][s]
    for (size_t in = 0; in < (size_t)numInputParties; in++) {
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                r[i][s] += result[in][i][s];
            }
        }
    }

    switch (numParties) {
    case 3: {
        // Internal buffers for BitAdd use interface format [size][numShares]
        T **A_buff = new T *[size];
        T **B_buff = new T *[size];
        for (size_t i = 0; i < size; i++) {
            A_buff[i] = new T[numShares];
            B_buff[i] = new T[numShares];
            // Convert from internal format result[in][i][s] to interface format [i][s]
            for (size_t s = 0; s < numShares; s++) {
                A_buff[i][s] = result[0][size + i][s];
                B_buff[i][s] = result[1][size + i][s];
            }
        }

        Rss_BitAdd(b_2, A_buff, B_buff, bitlength, bitlength, size, ring_size, nodeNet, ss);

        for (size_t i = 0; i < size; i++) {
            delete[] A_buff[i];
            delete[] B_buff[i];
        }
        delete[] A_buff;
        delete[] B_buff;
        break;
    }
    case 5: {
        // Internal buffers for BitAdd use interface format [size][numShares]
        T **A_buff = new T *[size];
        T **B_buff = new T *[size];
        T **temp = new T *[size];
        for (size_t i = 0; i < size; i++) {
            A_buff[i] = new T[numShares];
            B_buff[i] = new T[numShares];
            temp[i] = new T[numShares];
            memset(temp[i], 0, sizeof(T) * numShares);
            // Convert from internal format result[in][i][s] to interface format [i][s]
            for (size_t s = 0; s < numShares; s++) {
                A_buff[i][s] = result[0][size + i][s];
                B_buff[i][s] = result[1][size + i][s];
            }
        }
        Rss_BitAdd(temp, A_buff, B_buff, bitlength, bitlength, size, ring_size, nodeNet, ss);
        uint reslen = std::min(bitlength + 1, ring_size);                                     // not needed here, since we're just interested in the lengths of the inputs

        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                A_buff[i][s] = result[2][size + i][s];
            }
        }

        Rss_BitAdd(b_2, temp, A_buff, reslen, bitlength, size, ring_size, nodeNet, ss);

        for (size_t i = 0; i < size; i++) {
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
        // Internal buffers for BitAdd use interface format [size][numShares]
        T **A_buff = new T *[2 * size];
        T **B_buff = new T *[2 * size];
        T **C_buff = new T *[2 * size];
        for (size_t i = 0; i < 2 * size; i++) {
            A_buff[i] = new T[numShares];
            B_buff[i] = new T[numShares];
            C_buff[i] = new T[numShares];
            memset(C_buff[i], 0, sizeof(T) * numShares); // sanitizing destination
        }
        // Convert from internal format result[in][i][s] to interface format [i][s]
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                A_buff[i][s] = result[0][size + i][s];
                A_buff[size + i][s] = result[1][size + i][s];
                B_buff[i][s] = result[2][size + i][s];
                B_buff[size + i][s] = result[3][size + i][s];
            }
        }
        // this can theoretically be done with a Mult_and_MultSparse special function
        Rss_BitAdd(C_buff, A_buff, B_buff, bitlength, bitlength, 2 * size, ring_size, nodeNet, ss);
        uint reslen = std::min(bitlength + 1, ring_size); // not needed here, since we're just interested in the lengths of the inputs

        // Reuse A_buff and B_buff with first half of C_buff
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                A_buff[i][s] = C_buff[i][s];
                B_buff[i][s] = C_buff[size + i][s];
            }
        }

        Rss_BitAdd(b_2, A_buff, B_buff, reslen, reslen, size, ring_size, nodeNet, ss);

        for (size_t i = 0; i < 2 * size; i++) {
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
        // buffer and res use interface format [numCarry * size][numShares]
        T **buffer = new T *[numCarry * size];
        T **res = new T *[numCarry * size];
        for (size_t i = 0; i < numCarry * size; i++) {
            buffer[i] = new T[numShares];
            res[i] = new T[numShares];
            memset(buffer[i], 0, sizeof(T) * numShares); // sanitizing destination
            memset(res[i], 0, sizeof(T) * numShares);    // sanitizing destination
        }
        // Extract carry bits from b_2 (interface format [size][numShares])
        for (size_t m = 0; m < numCarry; m++) {
            for (size_t i = 0; i < size; i++) {
                for (size_t s = 0; s < numShares; s++) {
                    buffer[m * size + i][s] = GET_BIT(b_2[i][s], T(bitlength + m)); // carries start at the (bitlength) position
                }
            }
        }
        Rss_B2A(res, buffer, numCarry * size, ring_size, nodeNet, ss);
        for (size_t i = 0; i < numCarry * size; i++) {
            memset(buffer[i], 0, sizeof(T) * numShares); // sanitizing destination
        }

        // Accumulate carry corrections into buffer[0..size-1]
        for (size_t m = 0; m < numCarry; m++) {
            for (size_t i = 0; i < size; i++) {
                for (size_t s = 0; s < numShares; s++) {
                    buffer[i][s] += res[m * size + i][s] * (T(1) << (T(m))); // check
                }
            }
        }

        // Apply carry corrections to r and mask b_2
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                r[i][s] -= buffer[i][s] * (T(1) << (T(bitlength))); // check

                // removing any bits > (bitlength-1), since they are irrelevant may cause interference
                b_2[i][s] = b_2[i][s] & ss->SHIFT[bitlength];
            }
        }
        // reusing buffer for summation, since it's no longer needed

        for (size_t i = 0; i < numCarry * size; i++) {
            delete[] buffer[i];
            delete[] res[i];
        }
        delete[] buffer;
        delete[] res;
    }

    for (size_t p = 0; p < (size_t)(threshold + 1); p++) {
        for (size_t i = 0; i < total_size; i++) {
            delete[] result[p][i];
        }
        delete[] result[p];
    }
    delete[] result;
}

// m = number of bits being truncated
// r -  full size share (z2k)
// r_hat - k-1-m bit share (z2k)
// b_2 - individual bits of r (z2), used for deterministic truncation
// b_km1 - MSB of r, shares over z2k
// Interface format: r, r_hat, b_2, b_km1 are [size][numShares] where array[i][s] is share s of element i
template <typename T>
void edaBit_Trunc(T **r, T **r_hat, T **b_2, T **b_km1, uint m, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    assertm((ring_size == ss->ring_size), "checking ring_size argument == ss->ring_size");

    int threshold = ss->getThreshold();
    int numParties = ss->getPeers();
    int id = ss->getID();
    uint numShares = ss->getNumShares();
    uint bytes = (ring_size + 7) >> 3;

    uint total_size = 3 * size; // for shares in 2*Z_2k and Z_2
    std::vector<int> input_parties(threshold + 1);
    std::iota(input_parties.begin(), input_parties.end(), 1);
    int numInputParties = input_parties.size();

    // Internal result array uses [threshold+1][total_size][numShares] format to match Input.hpp
    // result[p_star_index][i][s] = share s of element i from input party p_star_index
    T ***result = new T **[threshold + 1];
    for (size_t p = 0; p < (size_t)(threshold + 1); p++) {
        result[p] = new T *[total_size];
        for (size_t i = 0; i < total_size; i++) {
            result[p][i] = new T[numShares];
            memset(result[p][i], 0, sizeof(T) * numShares); // sanitizing destination
        }
    }

    // Sanitize output arrays (interface format [size][numShares])
    for (size_t i = 0; i < size; i++) {
        memset(r[i], 0, sizeof(T) * numShares);
        memset(b_2[i], 0, sizeof(T) * numShares);
        memset(r_hat[i], 0, sizeof(T) * numShares);
        memset(b_km1[i], 0, sizeof(T) * numShares);
    }

    if (id <= threshold + 1) {
        uint8_t *buffer = new uint8_t[size * bytes];
        ss->prg_getrandom(bytes, size, buffer);
        T *r_val = new T[size];
        memset(r_val, 0, sizeof(T) * size);
        for (size_t i = 0; i < size; i++) {
            memcpy(r_val + i, buffer + i * bytes, bytes);
            r_val[i] = r_val[i] & ss->SHIFT[ring_size];
        }

        Rss_Input_edaBit_Trunc(result, r_val, input_parties, m, size, ring_size, nodeNet, ss);

        delete[] buffer; // not needed anymore
        delete[] r_val;  // not needed anymore
    } else {
        Rss_Input_edaBit_Trunc(result, static_cast<T *>(nullptr), input_parties, m, size, ring_size, nodeNet, ss);
    }

    // first (size) elements of result are the shares over Z_2k
    // summing all the random values shared in Z_2k
    // Convert from internal format result[in][i][s] to interface format [i][s]
    for (size_t in = 0; in < (size_t)numInputParties; in++) {
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                r[i][s] += result[in][i][s];
                r_hat[i][s] += result[in][2 * size + i][s];
            }
        }
    }

    switch (numParties) {
    case 3: {
        // Internal buffers use interface format [size][numShares]
        T **A_buff = new T *[size];
        T **B_buff = new T *[size];
        T **temp_carry = new T *[3 * size]; // 3*size for k-1 carry, mth carry, and b_k-1
        for (size_t i = 0; i < size; i++) {
            A_buff[i] = new T[numShares];
            B_buff[i] = new T[numShares];
            // Convert from internal format result[in][i][s] to interface format [i][s]
            for (size_t s = 0; s < numShares; s++) {
                A_buff[i][s] = result[0][size + i][s];
                B_buff[i][s] = result[1][size + i][s];
            }
        }
        for (size_t i = 0; i < 3 * size; i++) {
            temp_carry[i] = new T[numShares];
            memset(temp_carry[i], 0, sizeof(T) * numShares);
        }

        Rss_BitAdd_Trunc(b_2, temp_carry, A_buff, B_buff, ring_size, ring_size, m, 0, size, ring_size, nodeNet, ss);
        // Extract MSB from b_2 (interface format [size][numShares])
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                temp_carry[2 * size + i][s] = GET_BIT(b_2[i][s], T(ring_size - 1));
            }
        }
        Rss_B2A(temp_carry, temp_carry, 3 * size, ring_size, nodeNet, ss);

        // getting msb (that we just converted from z2 to z2k) and moving it to b_km1
        // can we theoretically use an assignment operator?
        // potential problem is that this would lead to a memory leak, would need to free b_2's memory (allocated inside trunc) first, and then make sure we dont free it again at the end of trunc
        // this is viable, since edaBit_trunc should only be called inside truncation
        // leaving for now since we know it's correct
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                b_km1[i][s] = temp_carry[2 * size + i][s];
            }
        }

        // this line is missing subtracting the left-shifted MSB
        // adding m-1 and subtracting k carries
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                r_hat[i][s] += temp_carry[i][s] - ((temp_carry[size + i][s]) << T(ring_size - m));
                // NOTE WE DO NOT SUBTRACT b_k-1 * 2^{k-m-1} HERE, WE DO IT INSIDE TRUNCATION
                // r_hat[i][s] -= b_km1[i][s] << T(ring_size - m -1);
            }
        }

        for (size_t i = 0; i < size; i++) {
            delete[] A_buff[i];
            delete[] B_buff[i];
        }
        for (size_t i = 0; i < 3 * size; i++) {
            delete[] temp_carry[i];
        }
        delete[] A_buff;
        delete[] B_buff;
        delete[] temp_carry;
        break;
    }
    case 5: {
        // Internal buffers use interface format [size][numShares]
        T **A_buff = new T *[size];
        T **B_buff = new T *[size];
        T **temp = new T *[size];
        T **temp_carry = new T *[5 * size]; // 2 carries from first bitAdd, 2 carries from second bitAdd, and MSB of result
        for (size_t i = 0; i < size; i++) {
            A_buff[i] = new T[numShares];
            B_buff[i] = new T[numShares];
            temp[i] = new T[numShares];
            memset(temp[i], 0, sizeof(T) * numShares);
            // Convert from internal format result[in][i][s] to interface format [i][s]
            for (size_t s = 0; s < numShares; s++) {
                A_buff[i][s] = result[0][size + i][s];
                B_buff[i][s] = result[1][size + i][s];
            }
        }
        for (size_t i = 0; i < 5 * size; i++) {
            temp_carry[i] = new T[numShares];
            memset(temp_carry[i], 0, sizeof(T) * numShares);
        }
        Rss_BitAdd_Trunc(temp, temp_carry, A_buff, B_buff, ring_size, ring_size, m, 0, size, ring_size, nodeNet, ss);

        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                A_buff[i][s] = result[2][size + i][s];
            }
        }

        Rss_BitAdd_Trunc(b_2, temp_carry, temp, A_buff, ring_size, ring_size, m, 2 * size, size, ring_size, nodeNet, ss);

        // Extract MSB from b_2
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                temp_carry[4 * size + i][s] = GET_BIT(b_2[i][s], T(ring_size - 1));
            }
        }

        Rss_B2A(temp_carry, temp_carry, 5 * size, ring_size, nodeNet, ss);

        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                b_km1[i][s] = temp_carry[4 * size + i][s];
            }
        }

        // adding m-1 and subtracting k carries
        // check if this is correct
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                // NOTE WE DO NOT SUBTRACT b_k-1 * 2^{k-m-1} HERE, WE DO IT INSIDE TRUNCATION
                r_hat[i][s] += temp_carry[i][s] - ((temp_carry[size + i][s]) << T(ring_size - m));
                r_hat[i][s] += temp_carry[2 * size + i][s] - ((temp_carry[3 * size + i][s]) << T(ring_size - m));
            }
        }

        for (size_t i = 0; i < size; i++) {
            delete[] temp[i];
            delete[] A_buff[i];
            delete[] B_buff[i];
        }
        for (size_t i = 0; i < 5 * size; i++) {
            delete[] temp_carry[i];
        }
        delete[] temp_carry;
        delete[] temp;
        delete[] A_buff;
        delete[] B_buff;
        break;
    }
    case 7: {
        // Internal buffers use interface format [size][numShares]
        T **A_buff = new T *[2 * size];
        T **B_buff = new T *[2 * size];
        T **C_buff = new T *[2 * size];
        T **temp_carry = new T *[7 * size]; // 7 for 2 carries per bitAdd (3), plus one for MSB of res
        for (size_t i = 0; i < 2 * size; i++) {
            A_buff[i] = new T[numShares];
            B_buff[i] = new T[numShares];
            C_buff[i] = new T[numShares];
            memset(C_buff[i], 0, sizeof(T) * numShares);
        }
        for (size_t i = 0; i < 7 * size; i++) {
            temp_carry[i] = new T[numShares];
            memset(temp_carry[i], 0, sizeof(T) * numShares);
        }
        // Convert from internal format result[in][i][s] to interface format [i][s]
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                A_buff[i][s] = result[0][size + i][s];
                A_buff[size + i][s] = result[1][size + i][s];
                B_buff[i][s] = result[2][size + i][s];
                B_buff[size + i][s] = result[3][size + i][s];
            }
        }

        // this can theoretically be done with a Mult_and_MultSparse special function
        Rss_BitAdd_Trunc(C_buff, temp_carry, A_buff, B_buff, ring_size, ring_size, m, 0, 2 * size, ring_size, nodeNet, ss);

        // Reuse A_buff and B_buff with first half of C_buff
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                A_buff[i][s] = C_buff[i][s];
                B_buff[i][s] = C_buff[size + i][s];
            }
        }

        Rss_BitAdd_Trunc(b_2, temp_carry, A_buff, B_buff, ring_size, ring_size, m, 4 * size, size, ring_size, nodeNet, ss);
        /*
        NOTICE BEFORE PROCEEDING

        THE ORDER OF THE FIRST GROUPS CARRY BITS INSERTED INTO THE CARRY BUFFER FOR 7 PARTIES ARE INCONSISTENT

        the data is ordered as follows:

        carry_buffer[7*size][numShares]:
        0 (m-1, first half of BA 1)
        1 (m-1, second half of BA 1)
        2 (k-1, first half of BA 1)
        3 (k-1, second half of BA 1)
        4 (m-1, BA 2)
        5 (k-1, BA 2)

        therfore, to remove the carry bits, we need to be careful when computing the summation
         */

        // Extract MSB from b_2
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                temp_carry[6 * size + i][s] = GET_BIT(b_2[i][s], T(ring_size - 1));
            }
        }

        Rss_B2A(temp_carry, temp_carry, 7 * size, ring_size, nodeNet, ss);

        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                b_km1[i][s] = temp_carry[6 * size + i][s];
            }
        }

        // check if this is correct
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                // NOTE WE DO NOT SUBTRACT b_k-1 * 2^{k-m-1} HERE, WE DO IT INSIDE TRUNCATION
                r_hat[i][s] += temp_carry[i][s] - ((temp_carry[2 * size + i][s]) << T(ring_size - m));
                r_hat[i][s] += temp_carry[size + i][s] - ((temp_carry[3 * size + i][s]) << T(ring_size - m));
                r_hat[i][s] += temp_carry[4 * size + i][s] - ((temp_carry[5 * size + i][s]) << T(ring_size - m));
            }
        }

        for (size_t i = 0; i < 2 * size; i++) {
            delete[] A_buff[i];
            delete[] B_buff[i];
            delete[] C_buff[i];
        }
        for (size_t i = 0; i < 7 * size; i++) {
            delete[] temp_carry[i];
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

    for (size_t p = 0; p < (size_t)(threshold + 1); p++) {
        for (size_t i = 0; i < total_size; i++) {
            delete[] result[p][i];
        }
        delete[] result[p];
    }
    delete[] result;
}

// m = number of bits being truncated
// r -  full size share (z2k)
// r_hat - k-1-m bit share (z2k)
// r_hat_hat - k-1-m-2 bit share (z2k)
// b_2 - individual bits of r (z2), used for deterministic truncation
// b_km1 - MSB of r, shares over z2k
// Interface format: r, r_hat, r_hat_hat, b_2, b_km1 are [size][numShares] where array[i][s] is share s of element i
template <typename T>
void edaBit_RNTE(T **r, T **r_hat, T **r_hat_hat, T **b_2, T **b_km1, uint m, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    assertm((ring_size == ss->ring_size), "checking ring_size argument == ss->ring_size");

    int threshold = ss->getThreshold();
    int numParties = ss->getPeers();
    int id = ss->getID();
    uint numShares = ss->getNumShares();
    int m2 = m - 2;
    uint bytes = (ring_size + 7) >> 3;

    uint total_size = 4 * size; // for shares in 3*Z_2k and Z_2
    std::vector<int> input_parties(threshold + 1);
    std::iota(input_parties.begin(), input_parties.end(), 1);
    int numInputParties = input_parties.size();

    // Internal result array uses [threshold+1][total_size][numShares] format to match Input.hpp
    // result[p_star_index][i][s] = share s of element i from input party p_star_index
    T ***result = new T **[threshold + 1];
    for (size_t p = 0; p < (size_t)(threshold + 1); p++) {
        result[p] = new T *[total_size];
        for (size_t i = 0; i < total_size; i++) {
            result[p][i] = new T[numShares];
            memset(result[p][i], 0, sizeof(T) * numShares); // sanitizing destination
        }
    }

    // Sanitize output arrays (interface format [size][numShares])
    for (size_t i = 0; i < size; i++) {
        memset(r[i], 0, sizeof(T) * numShares);
        memset(b_2[i], 0, sizeof(T) * numShares);
        memset(r_hat[i], 0, sizeof(T) * numShares);
        memset(r_hat_hat[i], 0, sizeof(T) * numShares);
        memset(b_km1[i], 0, sizeof(T) * numShares);
    }

    if (id <= threshold + 1) {
        uint8_t *buffer = new uint8_t[size * bytes];
        ss->prg_getrandom(bytes, size, buffer);
        T *r_val = new T[size];
        memset(r_val, 0, sizeof(T) * size);
        for (size_t i = 0; i < size; i++) {
            memcpy(r_val + i, buffer + i * bytes, bytes);
            r_val[i] = r_val[i] & ss->SHIFT[ring_size];
        }

        Rss_Input_edaBit_RNTE(result, r_val, input_parties, m, size, ring_size, nodeNet, ss);

        delete[] buffer; // not needed anymore
        delete[] r_val;  // not needed anymore
    } else {
        Rss_Input_edaBit_RNTE(result, static_cast<T *>(nullptr), input_parties, m, size, ring_size, nodeNet, ss);
    }

    // first (size) elements of result are the shares over Z_2k
    // summing all the random values shared in Z_2k
    // Convert from internal format result[in][i][s] to interface format [i][s]
    for (size_t in = 0; in < (size_t)numInputParties; in++) {
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                r[i][s] += result[in][i][s];
                r_hat[i][s] += result[in][2 * size + i][s];
                r_hat_hat[i][s] += result[in][3 * size + i][s];
            }
        }
    }

    switch (numParties) {
    case 3: {
        // Internal buffers use interface format [size][numShares]
        T **A_buff = new T *[size];
        T **B_buff = new T *[size];
        T **temp_carry = new T *[4 * size]; // 4*size for k-1 carry, mth carry, m-2 carry, and b_k-1
        for (size_t i = 0; i < size; i++) {
            A_buff[i] = new T[numShares];
            B_buff[i] = new T[numShares];
            // Convert from internal format result[in][i][s] to interface format [i][s]
            for (size_t s = 0; s < numShares; s++) {
                A_buff[i][s] = result[0][size + i][s];
                B_buff[i][s] = result[1][size + i][s];
            }
        }
        for (size_t i = 0; i < 4 * size; i++) {
            temp_carry[i] = new T[numShares];
            memset(temp_carry[i], 0, sizeof(T) * numShares);
        }

        Rss_BitAdd_RNTE(b_2, temp_carry, A_buff, B_buff, ring_size, ring_size, m, 0, size, ring_size, nodeNet, ss);
        // Extract MSB from b_2 (interface format [size][numShares])
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                temp_carry[3 * size + i][s] = GET_BIT(b_2[i][s], T(ring_size - 1));
            }
        }
        Rss_B2A(temp_carry, temp_carry, 4 * size, ring_size, nodeNet, ss);

        // getting msb (that we just converted from z2 to z2k) and moving it to b_km1
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                b_km1[i][s] = temp_carry[3 * size + i][s];
            }
        }

        // this line is missing subtracting the left-shifted MSB
        // adding m-1 and subtracting k carries
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                // NOTE WE DO NOT SUBTRACT b_k-1 * 2^{k-m-1} HERE, WE DO IT INSIDE TRUNCATION
                // computing cr_m - cr_k-1
                r_hat[i][s] += temp_carry[i][s] - ((temp_carry[2 * size + i][s]) << T(ring_size - m));
                // computing cr_m-2 - cr_k-1
                r_hat_hat[i][s] += temp_carry[size + i][s] - ((temp_carry[2 * size + i][s]) << T(ring_size - m2));
            }
        }

        for (size_t i = 0; i < size; i++) {
            delete[] A_buff[i];
            delete[] B_buff[i];
        }
        for (size_t i = 0; i < 4 * size; i++) {
            delete[] temp_carry[i];
        }
        delete[] A_buff;
        delete[] B_buff;
        delete[] temp_carry;
        break;
    }
    case 5: {
        // Internal buffers use interface format [size][numShares]
        T **A_buff = new T *[size];
        T **B_buff = new T *[size];
        T **temp = new T *[size];
        T **temp_carry = new T *[7 * size]; // 3 carries from first bitAdd, 3 from second, plus MSB
        for (size_t i = 0; i < size; i++) {
            A_buff[i] = new T[numShares];
            B_buff[i] = new T[numShares];
            temp[i] = new T[numShares];
            memset(temp[i], 0, sizeof(T) * numShares);
            // Convert from internal format result[in][i][s] to interface format [i][s]
            for (size_t s = 0; s < numShares; s++) {
                A_buff[i][s] = result[0][size + i][s];
                B_buff[i][s] = result[1][size + i][s];
            }
        }
        for (size_t i = 0; i < 7 * size; i++) {
            temp_carry[i] = new T[numShares];
            memset(temp_carry[i], 0, sizeof(T) * numShares);
        }
        Rss_BitAdd_RNTE(temp, temp_carry, A_buff, B_buff, ring_size, ring_size, m, 0, size, ring_size, nodeNet, ss);

        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                A_buff[i][s] = result[2][size + i][s];
            }
        }

        Rss_BitAdd_RNTE(b_2, temp_carry, temp, A_buff, ring_size, ring_size, m, 3 * size, size, ring_size, nodeNet, ss);

        // Extract MSB from b_2
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                temp_carry[6 * size + i][s] = GET_BIT(b_2[i][s], T(ring_size - 1));
            }
        }

        Rss_B2A(temp_carry, temp_carry, 7 * size, ring_size, nodeNet, ss);

        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                b_km1[i][s] = temp_carry[6 * size + i][s];
            }
        }

        // adding m-1 and subtracting k carries
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                // NOTE WE DO NOT SUBTRACT b_k-1 * 2^{k-m-1} HERE, WE DO IT INSIDE TRUNCATION
                r_hat[i][s] += temp_carry[i][s] - ((temp_carry[2 * size + i][s]) << T(ring_size - m));
                r_hat[i][s] += temp_carry[3 * size + i][s] - ((temp_carry[5 * size + i][s]) << T(ring_size - m));

                r_hat_hat[i][s] += temp_carry[size + i][s] - ((temp_carry[2 * size + i][s]) << T(ring_size - m2));
                r_hat_hat[i][s] += temp_carry[4 * size + i][s] - ((temp_carry[5 * size + i][s]) << T(ring_size - m2));
            }
        }

        for (size_t i = 0; i < size; i++) {
            delete[] temp[i];
            delete[] A_buff[i];
            delete[] B_buff[i];
        }
        for (size_t i = 0; i < 7 * size; i++) {
            delete[] temp_carry[i];
        }
        delete[] temp_carry;
        delete[] temp;
        delete[] A_buff;
        delete[] B_buff;
        break;
    }
    case 7: {
        // Internal buffers use interface format [size][numShares]
        T **A_buff = new T *[2 * size];
        T **B_buff = new T *[2 * size];
        T **C_buff = new T *[2 * size];
        // 10*size: 6*size for first BitAdd (2*size elements × 3 carry types),
        //          3*size for second BitAdd (size elements × 3 carry types),
        //          1*size for MSB of b_2
        T **temp_carry = new T *[10 * size];
        for (size_t i = 0; i < 2 * size; i++) {
            A_buff[i] = new T[numShares];
            B_buff[i] = new T[numShares];
            C_buff[i] = new T[numShares];
            memset(C_buff[i], 0, sizeof(T) * numShares);
        }
        for (size_t i = 0; i < 10 * size; i++) {
            temp_carry[i] = new T[numShares];
            memset(temp_carry[i], 0, sizeof(T) * numShares);
        }
        // Convert from internal format result[in][i][s] to interface format [i][s]
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                A_buff[i][s] = result[0][size + i][s];
                A_buff[size + i][s] = result[1][size + i][s];
                B_buff[i][s] = result[2][size + i][s];
                B_buff[size + i][s] = result[3][size + i][s];
            }
        }

        // this can theoretically be done with a Mult_and_MultSparse special function
        Rss_BitAdd_RNTE(C_buff, temp_carry, A_buff, B_buff, ring_size, ring_size, m, 0, 2 * size, ring_size, nodeNet, ss);

        // Reuse A_buff and B_buff with first half of C_buff
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                A_buff[i][s] = C_buff[i][s];
                B_buff[i][s] = C_buff[size + i][s];
            }
        }

        Rss_BitAdd_RNTE(b_2, temp_carry, A_buff, B_buff, ring_size, ring_size, m, 6 * size, size, ring_size, nodeNet, ss);
        /*
        NOTICE: Carry buffer layout for 7 parties

        carry_buffer[10*size][numShares]
        0 (m-1, first half of BA 1)
        1 (m-1, second half of BA 1)
        2 (m-3, first half of BA 1)
        3 (m-3, second half of BA 1)
        4 (k-1, first half of BA 1)
        5 (k-1, second half of BA 1)
        6 (m-1, BA 2)
        7 (m-3, BA 2)
        8 (k-1, BA 2)
        9 (MSB of b_2, i.e., b_km1)

        The r_hat correction uses carries from indices 0,1,4,5,6,8
        The r_hat_hat correction uses carries from indices 2,3,4,5,7,8
         */

        // Extract MSB from b_2 - use index 9*size to avoid overwriting k-1 carries from BA2
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                temp_carry[9 * size + i][s] = GET_BIT(b_2[i][s], T(ring_size - 1));
            }
        }

        Rss_B2A(temp_carry, temp_carry, 10 * size, ring_size, nodeNet, ss);

        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                b_km1[i][s] = temp_carry[9 * size + i][s];
            }
        }

        // check if this is correct
        for (size_t i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                // NOTE WE DO NOT SUBTRACT b_k-1 * 2^{k-m-1} HERE, WE DO IT INSIDE TRUNCATION
                r_hat[i][s] += temp_carry[0 * size + i][s] - ((temp_carry[4 * size + i][s]) << T(ring_size - m));
                r_hat[i][s] += temp_carry[1 * size + i][s] - ((temp_carry[5 * size + i][s]) << T(ring_size - m));
                r_hat[i][s] += temp_carry[6 * size + i][s] - ((temp_carry[8 * size + i][s]) << T(ring_size - m));

                r_hat_hat[i][s] += temp_carry[2 * size + i][s] - ((temp_carry[4 * size + i][s]) << T(ring_size - m2));
                r_hat_hat[i][s] += temp_carry[3 * size + i][s] - ((temp_carry[5 * size + i][s]) << T(ring_size - m2));
                r_hat_hat[i][s] += temp_carry[7 * size + i][s] - ((temp_carry[8 * size + i][s]) << T(ring_size - m2));
            }
        }

        for (size_t i = 0; i < 2 * size; i++) {
            delete[] A_buff[i];
            delete[] B_buff[i];
            delete[] C_buff[i];
        }
        for (size_t i = 0; i < 10 * size; i++) {
            delete[] temp_carry[i];
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

    for (size_t p = 0; p < (size_t)(threshold + 1); p++) {
        for (size_t i = 0; i < total_size; i++) {
            delete[] result[p][i];
        }
        delete[] result[p];
    }
    delete[] result;
}
#endif // _EDABIT_HPP_
