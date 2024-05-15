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

// input array is bits shared over Z2 (packed into a single T), this is generated from edaBit
// array [numShares][size]
// result [numShares][size*(2^k)] (interpreted as (size) blocks of dimension (2^k), which is the k-ary OR of all bits and their complements)
// we maintain the original code from the Shamir implementaiton as a reference for each operation (in RSS)
// reminder, when operating in Z2 in RSS, +/- is equivalent to bitwise XOR (^)
template <typename T>
void AllOr(T **array, int k, T **result, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    static uint numShares = ss->getNumShares();
    // uint ring_size = ss->ring_size;
    // T *ai = new T[numShares];
    // memset(ai, 0, sizeof(T) * numShares);
    vector<T> ai(numShares, 0);
    ss->sparsify_public(ai, 1);

    if (k == 1) {
        for (size_t s = 0; s < numShares; s++) {
            for (int i = 0; i < size; i++) {
                result[s][2 * i] = GET_BIT(array[s][i], T(0));
                result[s][2 * i + 1] = (T(1) & ai[s]) ^ GET_BIT(array[s][i], T(0));
            }
        }
        return;
    }

    // this rounds up to the nearest (whole) multiple of 8
    uint num_bits = (1 << k) * size; // exact number of bits in the output
    uint num_uints = (num_bits + 7) >> 3;

    uint8_t **buff = new uint8_t *[numShares];
    uint8_t **u1 = new uint8_t *[numShares];
    uint8_t **v1 = new uint8_t *[numShares];
    uint8_t **add_b = new uint8_t *[numShares];
    uint8_t **mul_b = new uint8_t *[numShares];
    for (size_t s = 0; s < numShares; s++) {
        buff[s] = new uint8_t[num_uints];
        memset(buff[s], 0, sizeof(uint8_t) * num_uints);
        u1[s] = new uint8_t[num_uints];
        memset(u1[s], 0, sizeof(uint8_t) * num_uints);
        v1[s] = new uint8_t[num_uints];
        memset(v1[s], 0, sizeof(uint8_t) * num_uints);
        add_b[s] = new uint8_t[num_uints];
        memset(add_b[s], 0, sizeof(uint8_t) * num_uints);
        mul_b[s] = new uint8_t[num_uints];
        memset(mul_b[s], 0, sizeof(uint8_t) * num_uints);
    }
    int round = 0;
    int nS = k - 1;
    int sizeLen = 2;
    while (nS > 1) {
        round++;
        nS /= 2;
        sizeLen *= 2;
    }

    // // making sure everything matches what python version generated
    // std::cout << "round : " << round << std::endl;
    // std::cout << "nS : " << nS << std::endl;
    // std::cout << "sizeLen : " << sizeLen << std::endl;

    vector<int> sizeArray(sizeLen, 0);
    //  int *sizeArray = (int *)malloc(sizeof(int) * sizeLen);
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
        // resetting counters for each share (the values will be set in the last iteraiton and will be subsequently used)
        oPos = 0;
        iPos = 0;
        for (uint n = 0; n < size; n++) {
            iPos = 0;
            for (uint i = 0; i < sizeLen; i += 2) {
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

                    u1[s][u8_idx_0] = SET_BIT(u1[s][u8_idx_0], bit_idx_0, uint8_t(GET_BIT(array[s][n], T(iPos))));
                    //  mpz_set(u1[oPos], array[n][iPos]);

                    tmp = (T(1) & ai[s]) ^ GET_BIT(array[s][n], T(iPos));
                    u1[s][u8_idx_1] = SET_BIT(u1[s][u8_idx_1], bit_idx_1, tmp);
                    // ss->modSub(u1[oPos + 1], 1, array[n][iPos]);

                    u1[s][u8_idx_2] = SET_BIT(u1[s][u8_idx_2], bit_idx_2, GET_BIT(u1[s][u8_idx_0], bit_idx_0));
                    // mpz_set(u1[oPos + 2], u1[oPos]);

                    u1[s][u8_idx_3] = SET_BIT(u1[s][u8_idx_3], bit_idx_3, GET_BIT(u1[s][u8_idx_1], bit_idx_1));
                    // mpz_set(u1[oPos + 3], u1[oPos + 1]);

                    v1[s][u8_idx_0] = SET_BIT(v1[s][u8_idx_0], bit_idx_0, uint8_t(GET_BIT(array[s][n], T(iPos + 1))));
                    //  mpz_set(v1[oPos], array[n][iPos + 1]);

                    v1[s][u8_idx_1] = SET_BIT(v1[s][u8_idx_1], bit_idx_1, GET_BIT(v1[s][u8_idx_0], bit_idx_0));
                    // mpz_set(v1[oPos + 1], v1[oPos]);

                    tmp = (T(1) & ai[s]) ^ GET_BIT(array[s][n], T(iPos + 1));
                    v1[s][u8_idx_2] = SET_BIT(v1[s][u8_idx_2], bit_idx_2, tmp);
                    // ss->modSub(v1[oPos + 2], 1, array[n][iPos + 1]);

                    v1[s][u8_idx_3] = SET_BIT(v1[s][u8_idx_3], bit_idx_3, GET_BIT(v1[s][u8_idx_2], bit_idx_2));
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
    // uint oPos_num_uints = (oPos >> 3);
    uint oPos_num_uints = (oPos + 7) >> 3;

    // ss->modAdd(add_b, u1, v1, oPos);
    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < oPos_num_uints; i++) {
            add_b[s][i] = u1[s][i] ^ v1[s][i];
        }
    }

    // Mult(mul_b, u1, v1, oPos, threadID, nodeNet, ss);
    Mult_Byte(mul_b, u1, v1, oPos_num_uints, nodeNet, ss);

    // ss->modSub(u1, add_b, mul_b, oPos);
    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < oPos_num_uints; i++) {
            u1[s][i] = add_b[s][i] ^ mul_b[s][i];
        }
    }

    uint oPos2 = 0;
    for (size_t s = 0; s < numShares; s++) {
        // resetting counters for each share
        oPos2 = 0;
        oPos = 0;
        for (uint n = 0; n < size; n++) {
            iPos = 0;
            for (uint i = 0; i < sizeLen; i += 2) {
                if (sizeArray[i] != 0 && sizeArray[i + 1] != 0) {
                    for (int x = 0; x < 4; x++) {
                        bit_idx_0 = uint8_t((oPos) & 7);
                        u8_idx_0 = (oPos) >> 3;

                        bit_idx_2 = uint8_t((oPos2) & 7);
                        u8_idx_2 = (oPos2) >> 3;

                        buff[s][u8_idx_2] = SET_BIT(buff[s][u8_idx_2], bit_idx_2, GET_BIT(u1[s][u8_idx_0], bit_idx_0));
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

                    buff[s][u8_idx_2] = SET_BIT(buff[s][u8_idx_2], bit_idx_2, uint8_t(GET_BIT(array[s][n], T(iPos))));
                    // mpz_set(buff[oPos2], array[n][iPos]);

                    tmp = (T(1) & ai[s]) ^ GET_BIT(array[s][n], T(iPos));
                    buff[s][u8_idx_3] = SET_BIT(buff[s][u8_idx_3], bit_idx_3, tmp);
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
        for (size_t s = 0; s < numShares; s++) {
            memset(u1[s], 0, sizeof(uint8_t) * num_uints);
            memset(v1[s], 0, sizeof(uint8_t) * num_uints);
            memset(mul_b[s], 0, sizeof(uint8_t) * num_uints);
            memset(add_b[s], 0, sizeof(uint8_t) * num_uints);
        }

        for (size_t s = 0; s < numShares; s++) {
            oPos = 0;
            iPos = 0;
            for (uint n = 0; n < size; n++) {
                for (uint i = 0; i < sizeLen; i += 2) {
                    uL = sizeArray[i];
                    vL = sizeArray[i + 1];
                    for (uint v = 0; v < vL; v++)
                        for (uint u = 0; u < uL; u++) {
                            bit_idx_0 = uint8_t((oPos) & 7);
                            u8_idx_0 = (oPos) >> 3;

                            bit_idx_2 = uint8_t((iPos + u) & 7);
                            u8_idx_2 = (iPos + u) >> 3;

                            bit_idx_3 = uint8_t((iPos + uL + v) & 7);
                            u8_idx_3 = (iPos + uL + v) >> 3;

                            u1[s][u8_idx_0] = SET_BIT(u1[s][u8_idx_0], bit_idx_0, GET_BIT(buff[s][u8_idx_2], bit_idx_2));
                            // mpz_set(u1[oPos], buff[iPos + u]);
                            v1[s][u8_idx_0] = SET_BIT(v1[s][u8_idx_0], bit_idx_0, GET_BIT(buff[s][u8_idx_3], bit_idx_3));
                            // mpz_set(v1[oPos], buff[iPos + uL + v]);

                            oPos++;
                        }
                    iPos += uL + vL;
                }
            }
        }
        // oPos_num_uints = (oPos >> 3);
        oPos_num_uints = (oPos + 7) >> 3;

        // ss->modAdd(add_b, u1, v1, oPos);
        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < oPos_num_uints; i++) {
                add_b[s][i] = u1[s][i] ^ v1[s][i];
            }
        }

        // Mult(mul_b, u1, v1, oPos, threadID, nodeNet, ss);
        Mult_Byte(mul_b, u1, v1, oPos_num_uints, nodeNet, ss);

        // ss->modSub(buff, add_b, mul_b, oPos);
        for (size_t s = 0; s < numShares; s++) {
            for (size_t i = 0; i < oPos_num_uints; i++) {
                buff[s][i] = add_b[s][i] ^ mul_b[s][i];
            }
        }

        sizeLen /= 2;
        for (int n = 0; n < sizeLen; n++)
            sizeArray[n] = sizeArray[n * 2] * sizeArray[n * 2 + 1];
    }
    // /*

    // oPos at this point should be equal to 2^k
    oPos /= size;
    // std::cout << "oPos : "<< oPos << std::endl;
    for (size_t s = 0; s < numShares; s++) {
        for (int x = 0; x < size; x++) {
            for (int i = 0; i < oPos; i++) {
                bit_idx_0 = uint8_t((x * oPos + i) & 7);
                u8_idx_0 = (x * oPos + i) >> 3;
                // std::cout << "bit_idx_0 : " <<+bit_idx_0<< std::endl;
                // std::cout << "u8_idx_0  : " <<u8_idx_0<< std::endl;
                // std::cout << "res_idx   : " << i * size + x<< std::endl;
                // check these indices
                result[s][x * oPos + i] = GET_BIT(buff[s][u8_idx_0], bit_idx_0);
                // result[s][x * oPos + i] = GET_BIT(buff[s][u8_idx_0], bit_idx_0);
                // mpz_set(result[x][i], buff[x * oPos + i]);
            }
        }
    }
    // */

    // delete[] ai;

    for (size_t s = 0; s < numShares; s++) {
        delete[] buff[s];
        delete[] u1[s];
        delete[] v1[s];
        delete[] add_b[s];
        delete[] mul_b[s];
    }
    delete[] buff;
    delete[] u1;
    delete[] v1;
    delete[] add_b;
    delete[] mul_b;
}

template <typename T>
void doOperation_PrivIndex_Write(T **index, T **array, int *values, int dim, int size, T *out_cond, T **priv_cond, int counter, int threadID, int type, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
}

// array : [numShares][dim]
// index : [numShares][size]
// dim = m in protocol specification
// this implementation of privIndex_read assumes the array contains integers shared over Z2k
// an alternate implementation of privIndex_read and AllOr is required if we want to assume the array contains bits shared over Z2
// the final result would be a single bit in Z2 (which can be converted to Z2k if whatever calling protocol requires it)
template <typename T>
void doOperation_PrivIndex_Read(T **index, T **array, T **result, int m, int size, int threadID, int type, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    static uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;
    uint logm = ceil(log2(m));
    int dtype_offset = (type == 0) ? 1 : 4;

    vector<T> ai(numShares, 0);
    ss->sparsify_public(ai, 1);

    assertm((ring_size >= logm), "the ring size must be greater than logm");

    T **edaBit_r = new T *[numShares];
    T **edaBit_b_2 = new T *[numShares];
    T **sum = new T *[numShares];

    T *c = new T[size];

    for (size_t i = 0; i < numShares; i++) {
        sum[i] = new T[size];

        edaBit_r[i] = new T[size];
        edaBit_b_2[i] = new T[size];
    }

    edaBit(edaBit_r, edaBit_b_2, logm, size, ring_size, nodeNet, ss);

    uint pow_logm = (1 << logm);
    uint num_bits = (1 << logm) * size; // exact number of bits in the output
    T **ao_res = new T *[numShares];
    for (size_t i = 0; i < numShares; i++)
        ao_res[i] = new T[num_bits];

    AllOr(edaBit_b_2, logm, ao_res, size, -1, nodeNet, ss);

    // computing the complement of the output of AllOr
    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < num_bits; i++) {
            ao_res[s][i] = (T(1) & ai[s]) ^ ao_res[s][i];
        }
    }

    // converting the complemented output of AllOr to Z2k
    Rss_B2A(ao_res, ao_res, num_bits, ring_size, nodeNet, ss);

    // [index] + [r]
    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            sum[s][i] = (index[s][i] + edaBit_r[s][i]);
        }
    }

    Open(c, sum, size, -1, nodeNet, ss);

    // obtatining the lower (log m) bits  c mod 2^{log m}
    for (size_t i = 0; i < size; i++) {
        c[i] = c[i] & ss->SHIFT[logm];
    }

    T **A_buff = new T *[numShares];
    T **B_buff = new T *[numShares];
    for (size_t s = 0; s < numShares; s++) {
        A_buff[s] = new T[m * dtype_offset * size];
        B_buff[s] = new T[m * dtype_offset * size];
    }

    int nb;
    int idx = 0;
    for (size_t s = 0; s < numShares; s++) {
        for (int i = 0; i < size; i++) {
            // A_buff will contain (size) copies of array
            memcpy(A_buff[s] + i * dtype_offset * m, array[s], sizeof(T) * dtype_offset * m);

            for (int j = 0; j < m; j++) {
                nb = (c[i] - j) % pow_logm;
                // printf("nb %d C1 %d j %d pow_logm %d \n", nb, C1, j, pow_logm);
                // B_buff[s][i * m + nb] = ao_res[s][i * m + nb];

                // not supporting floating point (for now)
                for (int k = 0; k < dtype_offset; k++) {
                    B_buff[s][idx] = ao_res[s][i * m + nb];
                    idx += 1;
                }
            }
        }
    }
    // call dot product, not mult
    DotProduct(result, A_buff, B_buff, m, dtype_offset * size, nodeNet, ss);

    delete[] c;

    for (size_t i = 0; i < numShares; i++) {

        delete[] A_buff[i];
        delete[] B_buff[i];

        delete[] ao_res[i];
        delete[] edaBit_r[i];
        delete[] edaBit_b_2[i];
        delete[] sum[i];
    }

    delete[] A_buff;
    delete[] B_buff;

    delete[] edaBit_r;
    delete[] edaBit_b_2;
    delete[] sum;
    delete[] ao_res;
}

template <typename T>
void compute_private_conditions(T **private_conditions, T *out_cond, T **priv_cond, int counter, int size) {
}
template <typename T>
void doOperation_PrivIndex_int(T *index, T **array, T *result, int dim, int type, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_float(T *index, T ***array, T **result, int dim, int type, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_int_arr(T *index, T ***array, T *result, int dim1, int dim2, int type, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_float_arr(T *index, T ****array, T **result, int dim1, int dim2, int type, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_Write(T **index, T **array, T **value, int m, int size, T *out_cond, T **priv_cond, int counter, int threadID, int type, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    static uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;
    uint logm = ceil(log2(m));
    int dtype_offset = (type == 0) ? 1 : 4;

    vector<T> ai(numShares, 0);
    ss->sparsify_public(ai, 1);

    assertm((ring_size >= logm), "the ring size must be greater than logm");

    T **edaBit_r = new T *[numShares];
    T **edaBit_b_2 = new T *[numShares];
    T **sum = new T *[numShares];

    T *c = new T[size];

    for (size_t i = 0; i < numShares; i++) {
        sum[i] = new T[size];

        edaBit_r[i] = new T[size];
        edaBit_b_2[i] = new T[size];
    }

    edaBit(edaBit_r, edaBit_b_2, logm, size, ring_size, nodeNet, ss);

    uint pow_logm = (1 << logm);
    uint num_bits = (1 << logm) * size; // exact number of bits in the output
    T **ao_res = new T *[numShares];
    for (size_t i = 0; i < numShares; i++)
        ao_res[i] = new T[num_bits];

    AllOr(edaBit_b_2, logm, ao_res, size, -1, nodeNet, ss);

    // computing the complement of the output of AllOr
    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < num_bits; i++) {
            ao_res[s][i] = (T(1) & ai[s]) ^ ao_res[s][i];
        }
    }

    // converting the complemented output of AllOr to Z2k
    Rss_B2A(ao_res, ao_res, num_bits, ring_size, nodeNet, ss);

    // [index] + [r]
    for (size_t s = 0; s < numShares; s++) {
        for (size_t i = 0; i < size; i++) {
            sum[s][i] = (index[s][i] + edaBit_r[s][i]);
        }
    }

    Open(c, sum, size, -1, nodeNet, ss);

    // obtatining the lower (log m) bits  c mod 2^{log m}
    for (size_t i = 0; i < size; i++) {
        c[i] = c[i] & ss->SHIFT[logm];
    }

    T **A_buff = new T *[numShares];
    T **B_buff = new T *[numShares];
    T **C_buff = new T *[numShares];
    for (size_t s = 0; s < numShares; s++) {
        A_buff[s] = new T[m * dtype_offset * size];
        B_buff[s] = new T[m * dtype_offset * size];
        C_buff[s] = new T[m * dtype_offset * size];
    }

    int nb;
    int idx = 0;
    for (int i = 0; i < size; i++) {
        idx = 0;
        for (size_t s = 0; s < numShares; s++) {
            // A_buff will contain (size) copies of array
            // memcpy(A_buff[s] + i * dtype_offset * m, array[s], sizeof(T) * dtype_offset * m);

            for (int j = 0; j < m; j++) {

                nb = (c[i] - j) % pow_logm;
                // printf("nb %d C1 %d j %d pow_logm %d \n", nb, C1, j, pow_logm);
                // B_buff[s][i * m + nb] = ao_res[s][i * m + nb];

                // not supporting floating point (for now)
                for (int k = 0; k < dtype_offset; k++) {
                    A_buff[s][idx] = array[s][j * dtype_offset + k] - value[s][i * dtype_offset + k];
                    B_buff[s][idx] = ao_res[s][i * m + nb];
                    idx += 1;
                }
            }
        }
        // updating array
        Mult(array, A_buff, B_buff, m * dtype_offset, nodeNet, ss);
    }

    delete[] c;

    for (size_t i = 0; i < numShares; i++) {

        delete[] A_buff[i];
        delete[] B_buff[i];
        delete[] C_buff[i];

        delete[] ao_res[i];
        delete[] edaBit_r[i];
        delete[] edaBit_b_2[i];
        delete[] sum[i];
    }

    delete[] A_buff;
    delete[] B_buff;
    delete[] C_buff;

    delete[] edaBit_r;
    delete[] edaBit_b_2;
    delete[] sum;
    delete[] ao_res;
}
template <typename T>
void doOperation_PrivIndex_Write_2d(T **index, T ***array, int *values, int dim1, int dim2, int size, T *out_cond, T **priv_cond, int counter, int threadID, int type, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_Write_2d(T **index, T ***array, T **values, int dim1, int dim2, int size, T *out_cond, T **priv_cond, int counter, int threadID, int type, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
}
#endif // _PRIVINDEX_HPP_