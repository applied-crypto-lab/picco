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

#ifndef _PRIVINDEX_HPP_
#define _PRIVINDEX_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "DotProduct.hpp"

// All arrays use [size][numShares] format: array[i][s] = share s of element i
// For uint8_t bit-packed arrays in AllOr: buf[byte_index][s] = share s at byte_index
//
// input array is bits shared over Z2 (packed into a single T), this is generated from edaBit
// array [size][numShares]
// result [size*(2^k)][numShares] (interpreted as (size) blocks of dimension (2^k), which is the k-ary OR of all bits and their complements)
// we maintain the original code from the Shamir implementation as a reference for each operation (in RSS)
// reminder, when operating in Z2 in RSS, +/- is equivalent to bitwise XOR (^)
template <typename T>
void AllOr(T **array, int k, T **result, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint numShares = ss->getNumShares();
    vector<T> ai(numShares, 0);
    ss->sparsify_public(ai, 1);

    if (k == 1) {
        for (int i = 0; i < size; i++) {
            for (size_t s = 0; s < numShares; s++) {
                result[2 * i][s] = GET_BIT(array[i][s], T(0));
                result[2 * i + 1][s] = (T(1) & ai[s]) ^ GET_BIT(array[i][s], T(0));
            }
        }
        return;
    }

    // this rounds up to the nearest (whole) multiple of 8
    uint num_bits = (1 << k) * size; // exact number of bits in the output
    uint num_uints = (num_bits + 7) >> 3;

    // All uint8_t arrays in [num_uints][numShares] format
    uint8_t **buff = new uint8_t *[num_uints];
    uint8_t **u1 = new uint8_t *[num_uints];
    uint8_t **v1 = new uint8_t *[num_uints];
    uint8_t **add_b = new uint8_t *[num_uints];
    uint8_t **mul_b = new uint8_t *[num_uints];
    for (size_t i = 0; i < num_uints; i++) {
        buff[i] = new uint8_t[numShares];
        memset(buff[i], 0, sizeof(uint8_t) * numShares);
        u1[i] = new uint8_t[numShares];
        memset(u1[i], 0, sizeof(uint8_t) * numShares);
        v1[i] = new uint8_t[numShares];
        memset(v1[i], 0, sizeof(uint8_t) * numShares);
        add_b[i] = new uint8_t[numShares];
        memset(add_b[i], 0, sizeof(uint8_t) * numShares);
        mul_b[i] = new uint8_t[numShares];
        memset(mul_b[i], 0, sizeof(uint8_t) * numShares);
    }
    int round = 0;
    int nS = k - 1;
    int sizeLen = 2;
    while (nS > 1) {
        round++;
        nS /= 2;
        sizeLen *= 2;
    }

    vector<int> sizeArray(sizeLen, 0);
    int len = 1;
    sizeArray[0] = k; // divide and get unit size
    for (int x = 0; x <= round; x++) {
        for (int i = len - 1; i >= 0; i--) {
            int val = sizeArray[i];
            sizeArray[i * 2] = val / 2;
            sizeArray[i * 2 + 1] = val / 2;
            if (val & 0x01)
                sizeArray[i * 2 + 1]++;
        }
        len *= 2;
    }

    // switching these to unsigned, since there's no reason for them to be signed
    uint oPos = 0;
    uint iPos = 0;
    uint8_t bit_idx_0, bit_idx_1, bit_idx_2, bit_idx_3; // will always be between 0 and 7 (inclusive)
    uint u8_idx_0, u8_idx_1, u8_idx_2, u8_idx_3;
    uint8_t tmp; // will always store a single bit
    for (size_t s = 0; s < numShares; s++) {
        // resetting counters for each share (the values will be set in the last iteration and will be subsequently used)
        oPos = 0;
        iPos = 0;
        for (int n = 0; n < size; n++) {
            iPos = 0;
            for (int i = 0; i < sizeLen; i += 2) {
                if (sizeArray[i] != 0 && sizeArray[i + 1] != 0) {
                    // computing all the indices once per iteration
                    bit_idx_0 = uint8_t((oPos) & 7); // equivalent to (oPos) % 8
                    u8_idx_0 = (oPos) >> 3;          // equivalent to floor((oPos)/2^3)

                    bit_idx_1 = uint8_t((oPos + 1) & 7);
                    u8_idx_1 = (oPos + 1) >> 3;

                    bit_idx_2 = uint8_t((oPos + 2) & 7);
                    u8_idx_2 = (oPos + 2) >> 3;

                    bit_idx_3 = uint8_t((oPos + 3) & 7);
                    u8_idx_3 = (oPos + 3) >> 3;

                    u1[u8_idx_0][s] = SET_BIT(u1[u8_idx_0][s], bit_idx_0, uint8_t(GET_BIT(array[n][s], T(iPos))));
                    //  mpz_set(u1[oPos], array[n][iPos]);

                    tmp = (T(1) & ai[s]) ^ GET_BIT(array[n][s], T(iPos));
                    u1[u8_idx_1][s] = SET_BIT(u1[u8_idx_1][s], bit_idx_1, tmp);
                    // ss->modSub(u1[oPos + 1], 1, array[n][iPos]);

                    u1[u8_idx_2][s] = SET_BIT(u1[u8_idx_2][s], bit_idx_2, GET_BIT(u1[u8_idx_0][s], bit_idx_0));
                    // mpz_set(u1[oPos + 2], u1[oPos]);

                    u1[u8_idx_3][s] = SET_BIT(u1[u8_idx_3][s], bit_idx_3, GET_BIT(u1[u8_idx_1][s], bit_idx_1));
                    // mpz_set(u1[oPos + 3], u1[oPos + 1]);

                    v1[u8_idx_0][s] = SET_BIT(v1[u8_idx_0][s], bit_idx_0, uint8_t(GET_BIT(array[n][s], T(iPos + 1))));
                    //  mpz_set(v1[oPos], array[n][iPos + 1]);

                    v1[u8_idx_1][s] = SET_BIT(v1[u8_idx_1][s], bit_idx_1, GET_BIT(v1[u8_idx_0][s], bit_idx_0));
                    // mpz_set(v1[oPos + 1], v1[oPos]);

                    tmp = (T(1) & ai[s]) ^ GET_BIT(array[n][s], T(iPos + 1));
                    v1[u8_idx_2][s] = SET_BIT(v1[u8_idx_2][s], bit_idx_2, tmp);
                    // ss->modSub(v1[oPos + 2], 1, array[n][iPos + 1]);

                    v1[u8_idx_3][s] = SET_BIT(v1[u8_idx_3][s], bit_idx_3, GET_BIT(v1[u8_idx_2][s], bit_idx_2));
                    // mpz_set(v1[oPos + 3], v1[oPos + 2]);
                    iPos += 2;
                    oPos += 4;
                } else
                    iPos += 1;
            }
        }
    }
    // oPos is the total number of bits (for the ENTIRE BATCH)
    // we need to convert this value to the total number of uint8_t's we're interested in
    uint oPos_num_uints = (oPos + 7) >> 3;

    // ss->modAdd(add_b, u1, v1, oPos);
    for (size_t i = 0; i < oPos_num_uints; i++) {
        for (size_t s = 0; s < numShares; s++) {
            add_b[i][s] = u1[i][s] ^ v1[i][s];
        }
    }

    // Mult(mul_b, u1, v1, oPos, threadID, nodeNet, ss);
    Mult_Byte(mul_b, u1, v1, oPos_num_uints, nodeNet, ss);

    // ss->modSub(u1, add_b, mul_b, oPos);
    for (size_t i = 0; i < oPos_num_uints; i++) {
        for (size_t s = 0; s < numShares; s++) {
            u1[i][s] = add_b[i][s] ^ mul_b[i][s];
        }
    }

    uint oPos2 = 0;
    for (size_t s = 0; s < numShares; s++) {
        // resetting counters for each share
        oPos2 = 0;
        oPos = 0;
        for (int n = 0; n < size; n++) {
            iPos = 0;
            for (int i = 0; i < sizeLen; i += 2) {
                if (sizeArray[i] != 0 && sizeArray[i + 1] != 0) {
                    for (int x = 0; x < 4; x++) {
                        bit_idx_0 = uint8_t((oPos) & 7);
                        u8_idx_0 = (oPos) >> 3;

                        bit_idx_2 = uint8_t((oPos2) & 7);
                        u8_idx_2 = (oPos2) >> 3;

                        buff[u8_idx_2][s] = SET_BIT(buff[u8_idx_2][s], bit_idx_2, GET_BIT(u1[u8_idx_0][s], bit_idx_0));
                        // mpz_set(buff[oPos2], u1[oPos]);
                        oPos++;
                        oPos2++;
                    }
                    iPos += 2;
                } else {

                    bit_idx_2 = uint8_t((oPos2) & 7);
                    u8_idx_2 = (oPos2) >> 3;

                    bit_idx_3 = uint8_t((oPos2 + 1) & 7);
                    u8_idx_3 = (oPos2 + 1) >> 3;

                    buff[u8_idx_2][s] = SET_BIT(buff[u8_idx_2][s], bit_idx_2, uint8_t(GET_BIT(array[n][s], T(iPos))));
                    // mpz_set(buff[oPos2], array[n][iPos]);

                    tmp = (T(1) & ai[s]) ^ GET_BIT(array[n][s], T(iPos));
                    buff[u8_idx_3][s] = SET_BIT(buff[u8_idx_3][s], bit_idx_3, tmp);
                    // ss->modSub(buff[oPos2 + 1], const1, array[n][iPos]);

                    oPos2 += 2;
                    iPos++;
                }
            }
        }
    }

    // if block size is 0, then recover to original data
    for (int n = 0; n < sizeLen; n += 2) {
        if (sizeArray[n] == 0 || sizeArray[n + 1] == 0)
            sizeArray[n / 2] = 2;
        else
            sizeArray[n / 2] = sizeArray[n] * sizeArray[n + 1] * 4;
    }
    sizeLen /= 2;
    int uL, vL;
    // other Round
    for (int x = 0; x < round; x++) {
        // sanitizing at start of round (everything is now in buff)
        for (size_t i = 0; i < num_uints; i++) {
            memset(u1[i], 0, sizeof(uint8_t) * numShares);
            memset(v1[i], 0, sizeof(uint8_t) * numShares);
            memset(mul_b[i], 0, sizeof(uint8_t) * numShares);
            memset(add_b[i], 0, sizeof(uint8_t) * numShares);
        }

        for (size_t s = 0; s < numShares; s++) {
            oPos = 0;
            iPos = 0;
            for (int n = 0; n < size; n++) {
                for (int i = 0; i < sizeLen; i += 2) {
                    uL = sizeArray[i];
                    vL = sizeArray[i + 1];
                    for (int v = 0; v < vL; v++)
                        for (int u = 0; u < uL; u++) {
                            bit_idx_0 = uint8_t((oPos) & 7);
                            u8_idx_0 = (oPos) >> 3;

                            bit_idx_2 = uint8_t((iPos + u) & 7);
                            u8_idx_2 = (iPos + u) >> 3;

                            bit_idx_3 = uint8_t((iPos + uL + v) & 7);
                            u8_idx_3 = (iPos + uL + v) >> 3;

                            u1[u8_idx_0][s] = SET_BIT(u1[u8_idx_0][s], bit_idx_0, GET_BIT(buff[u8_idx_2][s], bit_idx_2));
                            // mpz_set(u1[oPos], buff[iPos + u]);
                            v1[u8_idx_0][s] = SET_BIT(v1[u8_idx_0][s], bit_idx_0, GET_BIT(buff[u8_idx_3][s], bit_idx_3));
                            // mpz_set(v1[oPos], buff[iPos + uL + v]);

                            oPos++;
                        }
                    iPos += uL + vL;
                }
            }
        }
        oPos_num_uints = (oPos + 7) >> 3;

        // ss->modAdd(add_b, u1, v1, oPos);
        for (size_t i = 0; i < oPos_num_uints; i++) {
            for (size_t s = 0; s < numShares; s++) {
                add_b[i][s] = u1[i][s] ^ v1[i][s];
            }
        }

        // Mult(mul_b, u1, v1, oPos, threadID, nodeNet, ss);
        Mult_Byte(mul_b, u1, v1, oPos_num_uints, nodeNet, ss);

        // ss->modSub(buff, add_b, mul_b, oPos);
        for (size_t i = 0; i < oPos_num_uints; i++) {
            for (size_t s = 0; s < numShares; s++) {
                buff[i][s] = add_b[i][s] ^ mul_b[i][s];
            }
        }

        sizeLen /= 2;
        for (int n = 0; n < sizeLen; n++)
            sizeArray[n] = sizeArray[n * 2] * sizeArray[n * 2 + 1];
    }

    // oPos at this point should be equal to 2^k
    oPos /= size;
    for (int x = 0; x < size; x++) {
        for (uint i = 0; i < oPos; i++) {
            bit_idx_0 = uint8_t((x * oPos + i) & 7);
            u8_idx_0 = (x * oPos + i) >> 3;
            for (size_t s = 0; s < numShares; s++) {
                result[x * oPos + i][s] = GET_BIT(buff[u8_idx_0][s], bit_idx_0);
            }
        }
    }

    for (size_t i = 0; i < num_uints; i++) {
        delete[] buff[i];
        delete[] u1[i];
        delete[] v1[i];
        delete[] add_b[i];
        delete[] mul_b[i];
    }
    delete[] buff;
    delete[] u1;
    delete[] v1;
    delete[] add_b;
    delete[] mul_b;
}

template <typename T>
void doOperation_PrivIndex_Write(T **index, T **array, int *values, int dim, int size, T *out_cond, T **priv_cond, int counter, int threadID, int type, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    // sparsify outputs in [numShares][size] format (internal convention)
    T **val_internal = new T *[numShares];
    for (size_t s = 0; s < numShares; s++) {
        val_internal[s] = new T[size];
        memset(val_internal[s], 0, sizeof(T) * size);
    }
    ss->sparsify(val_internal, values, size);

    // Convert to [size][numShares] format for Write
    T **val = new T *[size];
    for (int i = 0; i < size; i++) {
        val[i] = new T[numShares];
        for (size_t s = 0; s < numShares; s++) {
            val[i][s] = val_internal[s][i];
        }
    }

    doOperation_PrivIndex_Write(index, array, val, dim, size, out_cond, priv_cond, counter, threadID, 0, nodeNet, ss);

    for (size_t s = 0; s < numShares; s++) {
        delete[] val_internal[s];
    }
    delete[] val_internal;
    for (int i = 0; i < size; i++) {
        delete[] val[i];
    }
    delete[] val;
}

// All arrays use interface format [size][numShares] where array[i][s] is share s of element i
// array : [dim][numShares] - the array to index into
// index : [size][numShares] - the indices to read
// result : [size][numShares] - the output
// dim = m in protocol specification
// this implementation of privIndex_read assumes the array contains integers shared over Z2k
//
// SIMPLIFIED IMPLEMENTATION:
// Instead of using AllOr which operates in Z2, we:
// 1. Generate random r and its bits using edaBit
// 2. Convert each bit from Z2 to Z2k using B2A
// 3. Compute equality indicators directly in Z2k:
//    indicator[i] = PRODUCT_{j} ((binary(i)_j == 0) ? (1 - b_j) : b_j)
// 4. Use the indicators in dot product
template <typename T>
void doOperation_PrivIndex_Read(T **index, T **array, T **result, int m, int size, int threadID, int type, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;
    uint logm = ceil(log2(m));
    int dtype_offset = (type == 0) ? 1 : 4;

    assertm((ring_size >= logm), "the ring size must be greater than logm");

    uint pow_logm = (1 << logm);

    // All arrays in [size][numShares] format
    T **edaBit_r = new T *[size];
    T **edaBit_b_2 = new T *[size];
    T **sum = new T *[size];

    T *c = new T[size];

    for (int i = 0; i < size; i++) {
        edaBit_r[i] = new T[numShares];
        memset(edaBit_r[i], 0, sizeof(T) * numShares);
        edaBit_b_2[i] = new T[numShares];
        memset(edaBit_b_2[i], 0, sizeof(T) * numShares);
        sum[i] = new T[numShares];
        memset(sum[i], 0, sizeof(T) * numShares);
    }

    edaBit(edaBit_r, edaBit_b_2, logm, size, ring_size, nodeNet, ss);

    // Extract individual bits from edaBit_b_2 and convert to Z2k
    // bits_z2k[bit_index][element_index] in [logm * size][numShares] format
    T **bits_z2 = new T *[logm * size];
    T **bits_z2k = new T *[logm * size];
    for (size_t i = 0; i < logm * size; i++) {
        bits_z2[i] = new T[numShares];
        bits_z2k[i] = new T[numShares];
        memset(bits_z2[i], 0, sizeof(T) * numShares);
        memset(bits_z2k[i], 0, sizeof(T) * numShares);
    }

    // Extract each bit from the packed edaBit_b_2
    for (int i = 0; i < size; i++) {
        for (uint b = 0; b < logm; b++) {
            for (size_t s = 0; s < numShares; s++) {
                bits_z2[b * size + i][s] = GET_BIT(edaBit_b_2[i][s], T(b));
            }
        }
    }

    // Convert all bits from Z2 to Z2k using B2A
    Rss_B2A(bits_z2k, bits_z2, logm * size, ring_size, nodeNet, ss);

    // Clean up bits_z2
    for (size_t i = 0; i < logm * size; i++) {
        delete[] bits_z2[i];
    }
    delete[] bits_z2;

    // Compute equality indicators in Z2k
    // indicators[indicator_index][element_index] in [pow_logm * size][numShares] format
    // indicator[i] = PRODUCT_{j} ((binary(i)_j == 0) ? (1 - b_j) : b_j)
    T **indicators = new T *[pow_logm * size];
    for (size_t i = 0; i < pow_logm * size; i++) {
        indicators[i] = new T[numShares];
        memset(indicators[i], 0, sizeof(T) * numShares);
    }

    // We need to compute the product of logm terms for each indicator
    // First, compute the complement of each bit: (1 - b_j)
    T **bits_complement = new T *[logm * size];
    for (size_t i = 0; i < logm * size; i++) {
        bits_complement[i] = new T[numShares];
    }

    // Sparsify 1 for subtraction
    vector<T> ai(numShares, 0);
    ss->sparsify_public(ai, 1);

    for (size_t i = 0; i < logm * size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            bits_complement[i][s] = ai[s] - bits_z2k[i][s];
        }
    }

    // For each indicator position i (0 to pow_logm-1), compute the product
    // We'll do this iteratively using multiplication
    // Start with indicator = 1, then multiply by the appropriate term for each bit

    // Initialize indicators with the first bit's contribution
    for (int elem = 0; elem < size; elem++) {
        for (size_t ind = 0; ind < pow_logm; ind++) {
            // First bit (bit 0)
            int bit0 = (ind >> 0) & 1;
            if (bit0 == 0) {
                // Use complement: (1 - b_0)
                for (size_t s = 0; s < numShares; s++) {
                    indicators[ind * size + elem][s] = bits_complement[0 * size + elem][s];
                }
            } else {
                // Use bit: b_0
                for (size_t s = 0; s < numShares; s++) {
                    indicators[ind * size + elem][s] = bits_z2k[0 * size + elem][s];
                }
            }
        }
    }

    // Multiply in remaining bits
    T **temp_mult = new T *[pow_logm * size];
    for (size_t i = 0; i < pow_logm * size; i++) {
        temp_mult[i] = new T[numShares];
    }

    for (uint b = 1; b < logm; b++) {
        // Prepare the terms to multiply
        for (int elem = 0; elem < size; elem++) {
            for (size_t ind = 0; ind < pow_logm; ind++) {
                int bit_val = (ind >> b) & 1;
                if (bit_val == 0) {
                    // Use complement
                    for (size_t s = 0; s < numShares; s++) {
                        temp_mult[ind * size + elem][s] = bits_complement[b * size + elem][s];
                    }
                } else {
                    // Use bit
                    for (size_t s = 0; s < numShares; s++) {
                        temp_mult[ind * size + elem][s] = bits_z2k[b * size + elem][s];
                    }
                }
            }
        }
        // Multiply indicators by temp_mult
        Mult(indicators, indicators, temp_mult, pow_logm * size, nodeNet, ss);
    }

    // Clean up temp arrays
    for (size_t i = 0; i < pow_logm * size; i++) {
        delete[] temp_mult[i];
    }
    delete[] temp_mult;

    for (size_t i = 0; i < logm * size; i++) {
        delete[] bits_complement[i];
        delete[] bits_z2k[i];
    }
    delete[] bits_complement;
    delete[] bits_z2k;

    // [index] + [r]
    for (int i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            sum[i][s] = index[i][s] + edaBit_r[i][s];
        }
    }

    Open(c, sum, size, -1, nodeNet, ss);

    // obtaining the lower (log m) bits: c mod 2^{log m}
    for (int i = 0; i < size; i++) {
        c[i] = c[i] & ss->SHIFT[logm];
    }

    // A_buff and B_buff in [m * dtype_offset * size][numShares] format
    T **A_buff = new T *[m * dtype_offset * size];
    T **B_buff = new T *[m * dtype_offset * size];
    for (int i = 0; i < m * dtype_offset * size; i++) {
        A_buff[i] = new T[numShares];
        memset(A_buff[i], 0, sizeof(T) * numShares);
        B_buff[i] = new T[numShares];
        memset(B_buff[i], 0, sizeof(T) * numShares);
    }

    int nb;
    int idx = 0;
    // Layout: group by (element, component) so each component gets its own dot product batch
    // Batch (i * dtype_offset + k) has m entries: one per array position j
    for (int i = 0; i < size; i++) {
        for (int k = 0; k < dtype_offset; k++) {
            for (int j = 0; j < m; j++) {
                nb = ((int)c[i] - j) % (int)pow_logm;
                if (nb < 0) nb += pow_logm;

                for (size_t s = 0; s < numShares; s++) {
                    A_buff[idx][s] = array[j * dtype_offset + k][s];
                    B_buff[idx][s] = indicators[nb * size + i][s];
                }
                idx++;
            }
        }
    }

    // dot product: m terms per batch, size*dtype_offset batches
    DotProduct(result, A_buff, B_buff, m, size * dtype_offset, nodeNet, ss);

    delete[] c;

    for (int i = 0; i < m * dtype_offset * size; i++) {
        delete[] A_buff[i];
        delete[] B_buff[i];
    }
    delete[] A_buff;
    delete[] B_buff;

    for (size_t i = 0; i < pow_logm * size; i++) {
        delete[] indicators[i];
    }
    delete[] indicators;

    for (int i = 0; i < size; i++) {
        delete[] edaBit_r[i];
        delete[] edaBit_b_2[i];
        delete[] sum[i];
    }
    delete[] edaBit_r;
    delete[] edaBit_b_2;
    delete[] sum;
}

template <typename T>
void doOperation_PrivIndex_int(T *index, T **array, T *result, int dim, int type, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint numShares = ss->getNumShares();

    // Convert single index/result to [1][numShares] format
    T **index_tmp = new T *[1];
    T **result_tmp = new T *[1];
    index_tmp[0] = new T[numShares];
    result_tmp[0] = new T[numShares];

    for (size_t s = 0; s < numShares; s++) {
        index_tmp[0][s] = index[s];
        result_tmp[0][s] = 0;
    }

    doOperation_PrivIndex_Read(index_tmp, array, result_tmp, dim, 1, threadID, 0, nodeNet, ss);

    for (size_t s = 0; s < numShares; s++) {
        result[s] = result_tmp[0][s];
    }

    delete[] index_tmp[0];
    delete[] result_tmp[0];
    delete[] index_tmp;
    delete[] result_tmp;
}

template <typename T>
void doOperation_PrivIndex_float(T *index, T ***array, T **result, int dim, int type, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    // Flatten float array from [dim][4][numShares] to [dim*4][numShares]
    T **flat_array = new T *[dim * 4];
    for (int j = 0; j < dim; j++) {
        for (int k = 0; k < 4; k++) {
            flat_array[j * 4 + k] = new T[numShares];
            for (size_t s = 0; s < numShares; s++) {
                flat_array[j * 4 + k][s] = array[j][k][s];
            }
        }
    }

    // Wrap index and result for batch-1 call
    T **index_tmp = new T *[1];
    index_tmp[0] = new T[numShares];
    for (size_t s = 0; s < numShares; s++) {
        index_tmp[0][s] = index[s];
    }

    // result_tmp needs [1 * 4][numShares] = [4][numShares] for dtype_offset=4
    T **result_tmp = new T *[4];
    for (int k = 0; k < 4; k++) {
        result_tmp[k] = new T[numShares];
        memset(result_tmp[k], 0, sizeof(T) * numShares);
    }

    doOperation_PrivIndex_Read(index_tmp, flat_array, result_tmp, dim, 1, threadID, 1, nodeNet, ss);

    // Copy results back: result[component][share]
    for (int k = 0; k < 4; k++) {
        for (size_t s = 0; s < numShares; s++) {
            result[k][s] = result_tmp[k][s];
        }
    }

    for (int i = 0; i < dim * 4; i++) delete[] flat_array[i];
    delete[] flat_array;
    delete[] index_tmp[0];
    delete[] index_tmp;
    for (int k = 0; k < 4; k++) delete[] result_tmp[k];
    delete[] result_tmp;
}

template <typename T>
void doOperation_PrivIndex_int_arr(T *index, T ***array, T *result, int dim1, int dim2, int type, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    int total = dim1 * dim2;

    // doOperation_PrivIndex_Read expects all arrays in [element][numShares] format
    T **array_tmp = new T *[total];
    T **index_tmp = new T *[1];
    T **result_tmp = new T *[1];

    index_tmp[0] = new T[numShares];
    result_tmp[0] = new T[numShares];
    for (size_t s = 0; s < numShares; s++) {
        index_tmp[0][s] = index[s];
        result_tmp[0][s] = 0;
    }

    // Flatten 2D array from [row][col][share] to [element][share]
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            int idx = i * dim2 + j;
            array_tmp[idx] = new T[numShares];
            for (size_t s = 0; s < numShares; s++) {
                array_tmp[idx][s] = array[i][j][s];
            }
        }
    }

    doOperation_PrivIndex_Read(index_tmp, array_tmp, result_tmp, total, 1, threadID, 0, nodeNet, ss);

    for (size_t s = 0; s < numShares; s++) {
        result[s] = result_tmp[0][s];
    }

    for (int i = 0; i < total; i++) {
        delete[] array_tmp[i];
    }
    delete[] index_tmp[0];
    delete[] result_tmp[0];
    delete[] array_tmp;
    delete[] index_tmp;
    delete[] result_tmp;
}

template <typename T>
void doOperation_PrivIndex_float_arr(T *index, T ****array, T **result, int dim1, int dim2, int type, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    int total = dim1 * dim2;

    // Flatten 2D float array from [dim1][dim2][4][numShares] to [total*4][numShares]
    T **flat_array = new T *[total * 4];
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            int base = (i * dim2 + j) * 4;
            for (int k = 0; k < 4; k++) {
                flat_array[base + k] = new T[numShares];
                for (size_t s = 0; s < numShares; s++) {
                    flat_array[base + k][s] = array[i][j][k][s];
                }
            }
        }
    }

    // Wrap index for batch-1 call
    T **index_tmp = new T *[1];
    index_tmp[0] = new T[numShares];
    for (size_t s = 0; s < numShares; s++) {
        index_tmp[0][s] = index[s];
    }

    // result_tmp: [4][numShares] for the 4 float components
    T **result_tmp = new T *[4];
    for (int k = 0; k < 4; k++) {
        result_tmp[k] = new T[numShares];
        memset(result_tmp[k], 0, sizeof(T) * numShares);
    }

    doOperation_PrivIndex_Read(index_tmp, flat_array, result_tmp, total, 1, threadID, 1, nodeNet, ss);

    // Copy results back
    for (int k = 0; k < 4; k++) {
        for (size_t s = 0; s < numShares; s++) {
            result[k][s] = result_tmp[k][s];
        }
    }

    for (int i = 0; i < total * 4; i++) delete[] flat_array[i];
    delete[] flat_array;
    delete[] index_tmp[0];
    delete[] index_tmp;
    for (int k = 0; k < 4; k++) delete[] result_tmp[k];
    delete[] result_tmp;
}

// All arrays in [size][numShares] format:
// array : [dim][numShares] - the array being written to
// index : [size][numShares] - the indices to write at
// value : [size][numShares] - the values to write
// this algorithm can theoretically be done in a more efficient manner involving prefixes/trees/etc
// the array can be updated up to size times, down to 0 times (depending on indexes)
// array_i will equal the value where its last bit (computed in AllOR) is 1
// currently array updates are performed sequentially
template <typename T>
void doOperation_PrivIndex_Write(T **index, T **array, T **value, int m, int size, T *out_cond, T **priv_cond, int counter, int threadID, int type, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;
    uint logm = ceil(log2(m));
    int dtype_offset = (type == 0) ? 1 : 4;

    vector<T> ai(numShares, 0);
    ss->sparsify_public(ai, 1);

    assertm((ring_size >= logm), "the ring size must be greater than logm");

    // All arrays in [size][numShares] format
    T **edaBit_r = new T *[size];
    T **edaBit_b_2 = new T *[size];
    T **sum = new T *[size];

    T *c = new T[size];

    for (int i = 0; i < size; i++) {
        edaBit_r[i] = new T[numShares];
        edaBit_b_2[i] = new T[numShares];
        sum[i] = new T[numShares];
    }

    edaBit(edaBit_r, edaBit_b_2, logm, size, ring_size, nodeNet, ss);

    uint pow_logm = (1 << logm);
    uint num_bits = pow_logm * size;
    // ao_res in [num_bits][numShares] format
    T **ao_res = new T *[num_bits];
    for (size_t i = 0; i < num_bits; i++)
        ao_res[i] = new T[numShares];

    AllOr(edaBit_b_2, logm, ao_res, size, -1, nodeNet, ss);

    // computing the complement of the output of AllOr
    for (size_t i = 0; i < num_bits; i++) {
        for (size_t s = 0; s < numShares; s++) {
            ao_res[i][s] = (T(1) & ai[s]) ^ ao_res[i][s];
        }
    }

    // converting the complemented output of AllOr to Z2k
    Rss_B2A(ao_res, ao_res, num_bits, ring_size, nodeNet, ss);

    // [index] + [r]
    for (int i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            sum[i][s] = (index[i][s] + edaBit_r[i][s]);
        }
    }

    Open(c, sum, size, -1, nodeNet, ss);

    // obtaining the lower (log m) bits  c mod 2^{log m}
    for (int i = 0; i < size; i++) {
        c[i] = c[i] & ss->SHIFT[logm];
    }

    // A_buff, B_buff in [m * dtype_offset][numShares] format
    T **A_buff = new T *[m * dtype_offset];
    T **B_buff = new T *[m * dtype_offset];
    for (int j = 0; j < m * dtype_offset; j++) {
        A_buff[j] = new T[numShares];
        B_buff[j] = new T[numShares];
    }

    // temp buffer for multiplication result
    T **temp = new T *[m * dtype_offset];
    for (int j = 0; j < m * dtype_offset; j++) {
        temp[j] = new T[numShares];
    }

    int nb;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < m; j++) {
            nb = ((int)c[i] - j) % (int)pow_logm;
            if (nb < 0) nb += pow_logm;

            for (int k = 0; k < dtype_offset; k++) {
                int elem = j * dtype_offset + k;
                for (size_t s = 0; s < numShares; s++) {
                    A_buff[elem][s] = array[j * dtype_offset + k][s] - value[i * dtype_offset + k][s];
                    B_buff[elem][s] = ao_res[i * pow_logm + nb][s];
                }
            }
        }
        // array[j] = array[j] - indicator[j] * (array[j] - value[i])
        // Multiply into temp, then subtract from array
        Mult(temp, A_buff, B_buff, m * dtype_offset, nodeNet, ss);
        for (int j = 0; j < m * dtype_offset; j++) {
            for (size_t s = 0; s < numShares; s++) {
                array[j][s] -= temp[j][s];
            }
        }
    }

    for (int j = 0; j < m * dtype_offset; j++) {
        delete[] temp[j];
    }
    delete[] temp;

    delete[] c;

    for (int j = 0; j < m * dtype_offset; j++) {
        delete[] A_buff[j];
        delete[] B_buff[j];
    }

    delete[] A_buff;
    delete[] B_buff;

    for (size_t i = 0; i < num_bits; i++) {
        delete[] ao_res[i];
    }
    delete[] ao_res;

    for (int i = 0; i < size; i++) {
        delete[] edaBit_r[i];
        delete[] edaBit_b_2[i];
        delete[] sum[i];
    }

    delete[] edaBit_r;
    delete[] edaBit_b_2;
    delete[] sum;
}

template <typename T>
void doOperation_PrivIndex_Write_2d(T **index, T ***array, int *values, int dim1, int dim2, int size, T *out_cond, T **priv_cond, int counter, int threadID, int type, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    // sparsify outputs in [numShares][size] format (internal convention)
    T **val_internal = new T *[numShares];
    for (size_t s = 0; s < numShares; s++) {
        val_internal[s] = new T[size];
        memset(val_internal[s], 0, sizeof(T) * size);
    }
    ss->sparsify(val_internal, values, size);

    // Convert to [size][numShares] format for Write
    T **val = new T *[size];
    for (int i = 0; i < size; i++) {
        val[i] = new T[numShares];
        for (size_t s = 0; s < numShares; s++) {
            val[i][s] = val_internal[s][i];
        }
    }

    doOperation_PrivIndex_Write_2d(index, array, val, dim1, dim2, size, out_cond, priv_cond, counter, threadID, 0, nodeNet, ss);

    for (size_t s = 0; s < numShares; s++) {
        delete[] val_internal[s];
    }
    delete[] val_internal;
    for (int i = 0; i < size; i++) {
        delete[] val[i];
    }
    delete[] val;
}

template <typename T>
void doOperation_PrivIndex_Write_2d(T **index, T ***array, T **values, int dim1, int dim2, int size, T *out_cond, T **priv_cond, int counter, int threadID, int type, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    int total = dim1 * dim2;

    // Flatten 2D array from [row][col][share] to [element][share]
    T **array_tmp = new T *[total];
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            int idx = i * dim2 + j;
            array_tmp[idx] = new T[numShares];
            for (size_t s = 0; s < numShares; s++) {
                array_tmp[idx][s] = array[i][j][s];
            }
        }
    }

    doOperation_PrivIndex_Write(index, array_tmp, values, total, size, out_cond, priv_cond, counter, threadID, 0, nodeNet, ss);

    // Copy updated array back from [element][share] to [row][col][share]
    for (int i = 0; i < dim1; i++) {
        for (int j = 0; j < dim2; j++) {
            int idx = i * dim2 + j;
            for (size_t s = 0; s < numShares; s++) {
                array[i][j][s] = array_tmp[idx][s];
            }
        }
    }

    for (int i = 0; i < total; i++) {
        delete[] array_tmp[i];
    }
    delete[] array_tmp;
}

#endif // _PRIVINDEX_HPP_
