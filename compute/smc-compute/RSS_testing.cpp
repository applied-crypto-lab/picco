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
#include "Mult.hpp"
#include "SMC_Utils.h"

#if __RSS__

using std::cout;
using std::endl;
using std::string;
using std::vector;

uint SMC_Utils::getNumShares() {
    return ss->getNumShares();
}

void SMC_Utils::offline_prg(uint8_t *dest, uint8_t *src, __m128i *ri) { // ri used to be void, replaced with __m128i* to compile
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

__m128i *SMC_Utils::offline_prg_keyschedule(uint8_t *src) {
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

void SMC_Utils::prg_aes_ni(priv_int_t *destination, uint8_t *seed, __m128i *key) {
    uint8_t res[16] = {};
    offline_prg(res, seed, key);
    memset(seed, 0, 16);
    memset(destination, 0, sizeof(priv_int_t));
    memcpy(seed, res, sizeof(priv_int_t));        // cipher becomes new seed or key
    memcpy(destination, res, sizeof(priv_int_t)); // cipher becomes new seed or key
}

void SMC_Utils::smc_rss_benchmark(string operation, int size, int num_iterations) {
    struct timeval start;
    struct timeval end;
    uint numShares = ss->getNumShares();
    unsigned long timer;
    uint numParties = ss->getPeers();

    uint ring_size = ss->ring_size;

    uint bytes = (ss->ring_size + 7) >> 3;
    printf("bytes : %u\n", bytes);
    printf("ring_size : %u\n", ring_size);
    printf("size : %u\n", size);
    printf("8*sizeof(Lint) = %lu\n", 8 * sizeof(priv_int_t));
    printf("sizeof(Lint) = %lu\n", sizeof(priv_int_t));
    priv_int *a = new priv_int[numShares];
    priv_int *b = new priv_int[numShares];
    priv_int *c = new priv_int[numShares];

    for (int i = 0; i < numShares; i++) {
        a[i] = new priv_int_t[size];
        memset(a[i], 0, sizeof(priv_int_t) * size);
        b[i] = new priv_int_t[size];
        memset(b[i], 0, sizeof(priv_int_t) * size);
        c[i] = new priv_int_t[size];
        memset(a[i], 0, sizeof(priv_int_t) * size);
    }

    std::cout << "START" << std::endl;
    gettimeofday(&start, NULL); // start timer here
    if (operation == "b2a") {
        for (size_t j = 0; j < num_iterations; j++) {
            Rss_B2A(c, a, size, ring_size, net, ss);
        }

    } else if (operation == "mult") {

        for (size_t j = 0; j < num_iterations; j++) {
            Mult(c, a, b, size, net, ss);
            // Rss_Mult_7pc_test(c, a, b, size, ring_size, net, ss);
            // Rss_B2A(c, a, b, size, ring_size, net, ss);
        }
    } else if (operation == "fl_mul") {

    } else if (operation == "fl_add") {

    } else if (operation == "fl_div") {

    } else if (operation == "fl_cmp") {
    }

    else {
        std::cerr << "ERROR: unknown operation " << operation << ", exiting..." << endl;
        exit(1);
    }
    std::cout << "END" << std::endl;
    // std::cout<<numBytesSent<<std::endl;

    gettimeofday(&end, NULL); // stop timer here
    timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    printf("[%s_%spc] [%u, %i, %u] [%.6lf ms,  %.6lf ms/size,  %lu bytes] \n", operation.c_str(), std::to_string(numParties).c_str(), ring_size, size, num_iterations, (double)(timer * 0.001) / num_iterations, (double)(timer * 0.001 / size) / num_iterations, 0 / num_iterations);

    for (size_t i = 0; i < numShares; i++) {
        delete[] a[i];
        delete[] b[i];
        delete[] c[i];
    }
    delete[] a;
    delete[] b;
    delete[] c;
}

void SMC_Utils::smc_test_rss(int threadID) {
    int size = pow(10, 3); //  testing only so I dont have to keep opening rss_main.cpp
    // uint bitlength = 20;

    uint numShares = ss->getNumShares();
    uint totalNumShares = ss->getTotalNumShares();
    uint ring_size = ss->ring_size;
    uint bytes = (ss->ring_size + 7) >> 3;
    printf("bytes : %u\n", bytes);
    printf("ring_size : %u\n", ring_size);
    printf("size : %u\n", size);
    printf("----\n\n");

    __m128i *key_prg;
    uint8_t key_raw[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    key_prg = offline_prg_keyschedule(key_raw);
    uint8_t k1[] = {0x31, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    uint8_t k2[] = {0xa1, 0x34, 0x6f, 0x67, 0x10, 0x1b, 0x13, 0xa3, 0x56, 0x45, 0x90, 0xb2, 0x13, 0xe3, 0x23, 0x24};

    std::vector<std::vector<int>> share_mapping;
    int numPeers = ss->getPeers();
    switch (numPeers) {
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
        exit(1);
    }
    uint8_t **Data1_byte = new uint8_t *[totalNumShares];
    uint8_t **Data2_byte = new uint8_t *[totalNumShares];

    priv_int_t **Data1 = new priv_int_t *[totalNumShares];
    priv_int_t **Data2 = new priv_int_t *[totalNumShares];
    for (int i = 0; i < totalNumShares; i++) {
        Data1[i] = new priv_int_t[size];
        memset(Data1[i], 0, sizeof(priv_int_t) * size);
        Data2[i] = new priv_int_t[size];
        memset(Data2[i], 0, sizeof(priv_int_t) * size);

        Data1_byte[i] = new uint8_t[size];
        memset(Data1_byte[i], 0, sizeof(uint8_t) * size);
        Data2_byte[i] = new uint8_t[size];
        memset(Data2_byte[i], 0, sizeof(uint8_t) * size);
    }
    uint bitlength = 12;

    for (int i = 0; i < size; i++) {
        for (size_t j = 0; j < totalNumShares - 1; j++) {
            prg_aes_ni(Data1[j] + i, k1, key_prg);
            // Data1[j][i] = GET_BIT(Data1[j][i], priv_int_t(0));
            prg_aes_ni(Data2[j] + i, k2, key_prg);
            // Data2[j][i] = GET_BIT(Data2[j][i], priv_int_t(0));
        }
        // Data1[totalNumShares - 1][i] = ( (-1) * i ) & ss->SHIFT[bitlength];
        // Data1[totalNumShares - 1][i] = 10 * (i + 1) + 1;
        Data1[totalNumShares - 1][i] = i;
        // Data1_byte[totalNumShares - 1][i] = i;
        // Data1[totalNumShares - 1][i] = ((-1) * i);
        // Data1[totalNumShares - 1][i] = 6 + i;
        // Data2[totalNumShares - 1][i] = priv_int_t(-1) >> 1;
        // Data2[totalNumShares - 1][i] = 1 + i;
        // Data1[totalNumShares - 1][i] = rand() % 2;
        Data2[totalNumShares - 1][i] = 1;
        // Data2_byte[totalNumShares - 1][i] = i;
        for (size_t j = 0; j < totalNumShares - 1; j++) {
            // Data1[totalNumShares - 1][i] -= Data1[j][i];
            // Data2[totalNumShares - 1][i] -= Data2[j][i];
            Data1[totalNumShares - 1][i] ^= Data1[j][i];
            Data2[totalNumShares - 1][i] ^= Data2[j][i];
            // Data1[totalNumShares - 1][i] ^= Data1[j][i] & 1;
            // Data2[totalNumShares - 1][i] ^= Data2[j][i] & 1;

            // Data1[totalNumShares - 1][i] ^= GET_BIT(Data1[j][i], priv_int_t(0)); // only want a single bit
            // Data2[totalNumShares - 1][i] ^= GET_BIT(Data2[j][i], priv_int_t(0)); // only want a single bit
        }
    }

    priv_int result = new priv_int_t[size];
    memset(result, 0, sizeof(priv_int_t) * size);

    priv_int result_2 = new priv_int_t[size];
    memset(result_2, 0, sizeof(priv_int_t) * size);

    priv_int result_3 = new priv_int_t[size];
    memset(result_3, 0, sizeof(priv_int_t) * size);

    uint8_t *result_byte = new uint8_t[size];
    memset(result_byte, 0, sizeof(uint8_t) * size);

    uint8_t *result_byte_2 = new uint8_t[size];
    memset(result_byte_2, 0, sizeof(uint8_t) * size);
    uint8_t *result_byte_3 = new uint8_t[size];
    memset(result_byte_3, 0, sizeof(uint8_t) * size);

    priv_int *B_sparse = new priv_int[ss->getNumShares()];
    priv_int *a = new priv_int[ss->getNumShares()];
    priv_int *b = new priv_int[ss->getNumShares()];
    uint8_t **a_byte = new uint8_t *[ss->getNumShares()];
    uint8_t **b_byte = new uint8_t *[ss->getNumShares()];

    uint8_t **c_byte = new uint8_t *[ss->getNumShares()];
    priv_int *c = new priv_int[ss->getNumShares()];
    priv_int *d = new priv_int[ss->getNumShares()];
    priv_int *A_bit = new priv_int[ss->getNumShares()];
    priv_int *B_bit = new priv_int[ss->getNumShares()];

    priv_int *a_buffer = new priv_int[ss->getNumShares()];
    priv_int *b_buffer = new priv_int[ss->getNumShares()];

    for (int i = 0; i < ss->getNumShares(); i++) {
        B_sparse[i] = new priv_int_t[size];
        memset(B_sparse[i], 0, sizeof(priv_int_t) * size);
        c[i] = new priv_int_t[size];
        memset(c[i], 0, sizeof(priv_int_t) * size);
        d[i] = new priv_int_t[size];
        memset(d[i], 0, sizeof(priv_int_t) * size);

        c_byte[i] = new uint8_t[size];
        memset(c_byte[i], 0, sizeof(uint8_t) * size);

        a_buffer[i] = new priv_int_t[2 * size];
        memset(a_buffer[i], 0, sizeof(priv_int_t) * 2 * size);
        b_buffer[i] = new priv_int_t[2 * size];
        memset(b_buffer[i], 0, sizeof(priv_int_t) * 2 * size);
    }

    for (size_t i = 0; i < numShares; i++) {
        a[i] = Data1[share_mapping[id - 1][i]];
        b[i] = Data2[share_mapping[id - 1][i]];
        a_byte[i] = Data1_byte[share_mapping[id - 1][i]];
        b_byte[i] = Data2_byte[share_mapping[id - 1][i]];
    }

    // printf("Intdiv\n");
    // doOperation_IntDiv(C, a, b, bitlength, size, -1, net, ss);

    // algorithm 9, precise algorithm

    struct timeval start;
    struct timeval end;
    unsigned long timer;

    int num_iterations = 1;

    std::cout << "START precise, unsorted" << std::endl;
    gettimeofday(&start, NULL); // start timer here
    for (size_t j = 0; j < num_iterations; j++) {
        BitEQ_fixed(c, b, a, size, ring_size, -1, net, ss);

        ss->modSub(a, a, 1, size);
        Mult_Byte(c_byte, a_byte, b_byte, size, net, ss);

        Rss_B2A(c, a, size, ring_size, net, ss);
        ss->modAdd(a, a, c, size);
        // Mult(c, a, b, size, net, ss);
        OR_ALL(c, b, size, ring_size, -1, net, ss);
    }
    std::cout << "END precise, unsorted" << std::endl;
    gettimeofday(&end, NULL); // stop timer here

    timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    printf("[%u, %i, %u] [%.6lf ms] \n", ring_size, size, num_iterations, (double)(timer * 0.001) / num_iterations);

    std::cout << "START precise, sorted" << std::endl;
    gettimeofday(&start, NULL); // start timer here
    for (size_t j = 0; j < num_iterations; j++) {

        // 5
        doOperation_EQZ(a, b, ring_size, 1, -1, net, ss);

        //7
        Mult_Byte(c_byte, a_byte, b_byte, size, net, ss);
        Rss_B2A(c, a, size, ring_size, net, ss);
        ss->modAdd(a, a, c, size);

        // 11
        ss->modAdd(a, a, c, size - 1);

        // 14
        ss->modSub(a, a, 1, size);
        Mult_Byte(c_byte, a_byte, b_byte, size, net, ss);

        // 15
        Rss_B2A(c, a, size, ring_size, net, ss);

        // 16, 17
        ss->modSub(a, a, 1, size);
        ss->modSub(a, a, 1, size);
        Mult(a_buffer, a_buffer, b_buffer, 2 * size, net, ss); // tow mults in parallel
        ss->modAdd(a, a, c, size);
        ss->modAdd(a, a, c, size);

        // 18, mult 1
        Mult(a, a, b, size, net, ss); 

        // 18, mult 2
        Mult(a, a, b, size, net, ss);

        // 19
        Rss_B2A(c, a, size, ring_size, net, ss);

        // 20, 21
        ss->modSub(a, a, c, size);
        ss->modSub(a, a, c, size);

        // 20,21
        ss->modAdd(a, a, c, size);
        ss->modAdd(a, a, c, size);

        // 20,21
        Mult(a_buffer, a_buffer, b_buffer, 2 * size, net, ss); // tow mults in parallel
    }
    std::cout << "END precise, sorted" << std::endl;
    gettimeofday(&end, NULL); // stop timer here

    timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    printf("[%u, %i, %u] [%.6lf ms] \n", ring_size, size, num_iterations, (double)(timer * 0.001) / num_iterations);

    // Open_Bitwise_print(a, "a", size, -1, net, ss);
    // Open_Bitwise_print(b, "b", size, -1, net, ss);
    // Open_Bitwise_print(c, "c", size, -1, net, ss);

    // Open_Bitwise(result, a, size, -1, net, ss);
    // Open_Bitwise(result, b, size, -1, net, ss);
    // Open_print(a, "res", size, -1, net, ss);
    // Open(result, C, size, -1, net, ss);
    // Open(result_2, a, size, -1, net, ss);
    // Open(result_3, b, size, -1, net, ss);

    // printf("\n");
    // for (size_t i = 0; i < size; i++) {
    // printf("(input a)     [%lu]: %i\n", i, (int)result_2[i]);
    // // print_binary((result_2[i]), ring_size);

    // printf("(input b)     [%lu]: %i\n", i, (int)result_3[i]);
    // // print_binary((result_3[i]), ring_size);

    // printf("(a / b)     [%lu]: %i / %i = %i\t", i, (int)result_2[i], (int)result_3[i], (int)result[i]);
    // printf("(off by) %i\n", (int)result[i] - (int)result_2[i] / (int)result_3[i]);
    // print_binary(result[i], ring_size);
    // printf("\n");
    // }

    // uint k = 3;
    // uint num_bits = (1 << k) * size; // exact number of bits in the output
    // uint num_uints = (num_bits + 7) >> 3;
    // std::cout << "k : " << k << std::endl;
    // std::cout << "num_bits : " << num_bits << std::endl;
    // std::cout << "num_uints : " << num_uints << std::endl;

    // priv_int_t **ao_res = new priv_int_t *[numShares];
    // for (size_t i = 0; i < numShares; i++)
    //     ao_res[i] = new priv_int_t[num_bits];

    // AllOr(b, k, ao_res, size, -1, net, ss);

    // priv_int_t *res_8 = new priv_int_t[num_bits];
    // memset(res_8, 0, sizeof(priv_int_t) * num_bits);

    // Open_Bitwise(res_8, ao_res, num_bits, -1, net, ss);
    // Open_Bitwise(result, b, size, -1, net, ss);

    // for (size_t i = 0; i < size; i++) {
    //     printf("(open) b   [%lu]: %u\t", i, result[i]);
    //     print_binary(result[i], ring_size);
    // }

    // for (size_t i = 0; i < num_bits; i++) {
    //     printf("(open) ao_res   [%lu]: %u\t", i, res_8[i]);
    //     print_binary(res_8[i], 8);
    //     if (((i + 1) % (1 << k)) == 0) {
    //         printf("\n");
    //     }
    // }

    // for (size_t i = 0; i < numShares; i++) {
    //     delete[] ao_res[i];
    // }
    // delete[] ao_res;

    // printf("multbyte\n");
    // Mult_Byte(C_byte, a_byte, b_byte, size, net, ss);
    // Open_Byte(result_byte, C_byte, size, -1, net, ss);
    // Open_Byte(result_byte_2, a_byte, size, -1, net, ss);
    // Open_Byte(result_byte_3, b_byte, size, -1, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     if (!(result[i] == (result_3[i] & result_2[i]))) {
    //         printf("(open) [a*b] (z2)  [%lu]: %u\n", i, result[i]);
    //         printf("expected %u\n", result_3[i] & result_2[i]);
    //         print_binary(result[i], ring_size);
    //     }
    // }

    // // Mult_Bitwise(C, a, b, size, net, ss);
    // // Open_Bitwise(result, C, size, -1, net, ss);
    // // Open_Bitwise(result_2, a, size, -1, net, ss);
    // // Open_Bitwise(result_3, b, size, -1, net, ss);
    // // for (size_t i = 0; i < size; i++) {
    // //     if (!(result[i] == (result_3[i] & result_2[i]))) {
    // //         printf("(open) [a*b] (z2)  [%lu]: %u\n", i, result[i]);
    // //         printf("expected %u\n", result_3[i] & result_2[i]);
    // //         print_binary(result[i], ring_size);
    // //     }
    // // }
    // // return;

    // for (size_t i = 0; i < numShares; i++) {
    //     A_bit[i] = Data1[share_mapping[id - 1][i]];
    //     B_bit[i] = Data2[share_mapping[id - 1][i]];
    // }
    // Open(result_2, a, size, -1, net, ss);
    // Open(result_3, b, size, -1, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     // printf("(open) c   [%lu]: %u\n", i, result[i]);
    //     printf("(open) a   [%lu]: %u\n", i, result_2[i]);
    //     printf("(open) b   [%lu]: %u\n", i, result_3[i]);
    // }

    // int m = 10;
    // doOperation_Trunc(C, b, ring_size, m, size, -1, net, ss);
    // Open(result_2, C, size, -1, net, ss);
    // Open(result, b, size, -1, net, ss);
    // // printf("\n");
    // for (size_t i = 0; i < size; i++) {
    //     if (!(result_2[i] == (result[i] >> priv_int_t(m)))) {
    //         printf("trunc  ERROR\n");
    //         printf("(open)  input     [%lu]: %u\n", i, result[i]);
    //         print_binary(result[i], ring_size);
    //         printf("(open)  trunc res [%lu]: %u\n", i, result_2[i]);
    //         print_binary(result_2[i], ring_size);
    //         printf("(open)  expected  [%lu]: %u\n", i, result[i] >> priv_int_t(m));
    //         print_binary(result[i] >> priv_int_t(m), ring_size);
    //     }
    // }

    // printf("\n");
    // // Rss_BitDec(C, a, size, ring_size, net, ss);
    // doOperation_EQZ(a, C, ring_size, size, -1, net, ss);
    // Open(result, C, size, -1, net, ss);
    // Open(result_2, a, size, -1, net, ss);
    // // Open(result_3, b, size, -1, net, ss);

    // for (size_t i = 0; i < size; i++) {
    //     printf("(open) c   [%lu]: %u\n", i, result[i]);
    //     // print_binary(result[i], ring_size);
    //     printf("(open) a   [%lu]: %u\n", i, result_2[i]);
    //     // print_binary(result_2[i], ring_size);
    //     if (!(result[i] == (result_2[i] == 0))) {
    //         printf("EQZ ERROR\n");

    //         // printf("(open ) c   [%lu]: %u\n", i, result[i]);
    //         // printf("(open ) a   [%lu]: %u\n", i, result_2[i]);
    //         // printf("(open ) b   [%lu]: %u\n", i, result_3[i]);
    //     }
    // }

    // uint m = ceil(log2(ring_size)); // rounding up to the nearest integer

    // Rss_BitDec(C, a, m, size, ring_size, net, ss);
    // Open_Bitwise(result, C, size, m, -1, net, ss);
    // Open(result_2, a, size, -1, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     printf("(open) a   [%lu]: %u\n", i, result_2[i]);
    //     if (!(result[i] == (result_2[i]))) {
    //         printf("BitDec ERROR\n");
    //         printf("(open) c   [%lu]: %u\t", i, result[i]);
    //         print_binary(result[i], ring_size);
    //         printf("(open) a   [%lu]: %u\t", i, result_2[i]);
    //         print_binary(result_2[i], ring_size);
    //     }
    // }

    // doOperation_LT(C, b, a, ring_size, ring_size, ring_size, size, -1, net, ss);
    // Open(result, C, size, -1, net, ss);
    // Open(result_2, a, size, -1, net, ss);
    // Open(result_3, b, size, -1, net, ss);

    // for (size_t i = 0; i < size; i++) {
    //     // printf("(open) c   [%lu]: %u\n", i, result[i]);
    //     // printf("(open) a   [%lu]: %u\n", i, result_2[i]);
    //     // printf("(open) b   [%lu]: %u\n", i, result_3[i]);
    //     if (!(result[i] == ((int)result_2[i] > (int)result_3[i]))) {
    //         printf("LT ERROR\n");

    //         printf("(open ) c   [%lu]: %u\n", i, result[i]);
    //         printf("(open ) a   [%lu]: %u\n", i, result_2[i]);
    //         printf("(open ) b   [%lu]: %u\n", i, result_3[i]);
    //     }
    // }

    // Rss_RandBit(C, size, ring_size, net, ss);
    // Open(result, C, size, -1, net, ss);
    // int ctr = 0;
    // for (size_t i = 0; i < size; i++) {
    //     // printf("(RandBit, open, Z_2k) r   [%lu]: %u\n", i, result[i]);
    //     if (result[i] == 0)
    //         ctr += 1;
    // }
    // std::cout << "percentage of 0's: " << 100 * float(ctr) / float(size) << "  percentage of 1's : " << 100 * float(size - ctr) / float(size) << std::endl;

    // Rss_MSB(C, a, size, ring_size, net, ss);
    // Open(result, C, size, -1, net, ss);
    // Open(result_2, a, size, -1, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     priv_int_t check = GET_BIT(result_2[i], ring_size - 1);
    //     if (!(result[i] == check)) {
    //         printf("MSB ERROR\n");
    //     }
    // }

    // edaBit(C, D, 8, size, ring_size, net, ss);
    // Open(result, C, size, -1, net, ss);
    // Open_Bitwise(result_2, D, size, -1, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     if (result[i] != result_2[i]) {
    //         printf("edabit ERROR\n");

    //         printf("(open, Z_2k) r   [%lu]: %u\t", i, result[i]);
    //         print_binary(result[i], ring_size);
    //         printf("(open, Z_2) bits [%lu]: %u\t", i, result_2[i]);
    //         print_binary(result_2[i], ring_size);
    //     }
    // }

    // edaBit(C, D, ring_size, size, ring_size, net, ss);
    // Open(result, C, size, -1, net, ss);
    // Open_Bitwise(result_2, D, size, -1, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     if (result[i] != result_2[i]) {
    //         printf("edabit ERROR\n");
    //         printf("(open, Z_2k) r   [%lu]: %u\t", i, result[i]);
    //         print_binary(result[i], ring_size);
    //         printf("(open, Z_2) bits [%lu]: %u\t", i, result_2[i]);
    //         print_binary(result_2[i], ring_size);
    //     }
    // }
    // for (size_t s = 0; s < numShares; s++) {
    //     for (size_t i = 0; i < size; i++) {
    //         printf("A_bit[%lu][%lu]: %u \n", i, s, A_bit[s][i]);
    //         printf("B_bit[%lu][%lu]: %u \n", s,i, B_bit[s][i]);
    //     }
    //     printf("\n");
    // }

    // Open_Bitwise(result, A_bit, size, -1, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     printf("(open, Z_2) A_bit [%lu]: %u\n", i, result[i]);
    // }

    // printf("\n");
    // Rss_B2A(C, A_bit, ring_size, size, net, ss);

    // smc_open(result, C, size, -1);
    // for (size_t i = 0; i < size; i++) {
    //     printf("(open, Z_k) A_bit [%lu]: %u\n", i, result[i]);
    // }
    // printf("\n");

    // Open_Bitwise(result, B_bit, size, -1, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     printf("(open, Z_2) B_bit [%lu]: %u\n", i, result[i]);
    // }

    // Rss_B2A(C, B_bit, ring_size, size, net, ss);
    // printf("\n");

    // smc_open(result, C, size, -1);
    // for (size_t i = 0; i < size; i++) {
    //     printf("(open, Z_k) B_bit [%lu]: %u\n", i, result[i]);
    // }

    // ss->sparsify(B_sparse, B, size);

    // for (size_t i = 0; i < size; i++) {
    //     for (size_t s = 0; s < numShares; s++) {
    //         printf("B_sparse[%lu][%lu]: %u \n", i, s, B_sparse[s][i]);
    //     }
    // }
    // smc_open(result, B_sparse, size, -1);
    // for (size_t i = 0; i < size; i++) {
    //     printf("(open) B_sparse [%lu]: %u\n", i, result[i]);
    // }
    // Mult_Sparse(C, B_sparse, B_sparse, size, net, ss);
    // Mult(C, A, B_sparse, size, net, ss);

    // for (size_t i = 0; i < size; i++) {
    //     for (size_t s = 0; s < numShares; s++) {
    //         printf("C[%lu][%lu]: %u \n", i, s, C[s][i]);
    //     }
    //     printf("\n");
    // }

    // smc_open(result, C, size, -1);
    // printf("\n");
    // for (size_t i = 0; i < size; i++) {
    //     printf("(open) C [%lu]: %u\n", i, result[i]);
    // }
    // printf("\n");

    //    Rss_Mult_Sparse(C, A, B_sparse, size, net, ss);
    //     // Mult(C, A, B_sparse, size, net, ss);
    //     smc_open(result, C, size, -1);
    //     printf("\n");
    //     for (size_t i = 0; i < size; i++) {
    //         printf("(open) C [%lu]: %u\n", i, result[i]);
    //     }
    //     printf("\n");

    // vector<int> input_parties = {1 };
    // vector<int> input_parties = {0};
    // uint numInputParties = input_parties.size();

    // priv_int_t ***res = new priv_int_t **[numInputParties];
    // for (size_t j = 0; j < numInputParties; j++) {
    //     res[j] = new priv_int_t *[numShares];
    //     for (size_t s = 0; s < numShares; s++) {
    //         res[j][s] = new priv_int_t[size];
    //         memset(res[j][s], 0, sizeof(priv_int_t) * size);
    //     }
    // }

    // cant use pid_in_T because that is only for fixed vector sizes:
    // n = 3 -> len(T) = 1
    // n = 5 -> len(T) = 2
    // n = 7 -> len(T) = 3

    // if (std::find(input_parties.begin(), input_parties.end(), id) != input_parties.end()) {
    //     std::cout << id << " is an input party" << std::endl;
    //     Rss_Input_p_star(res, reinterpret_cast<priv_int_t *>(B), input_parties, size, ring_size, net, ss);
    // } else {
    //     std::cout << id << " is NOT an input party" << std::endl;
    //     // priv_int placeholder = nullptr;
    //     Rss_Input_p_star(res, static_cast<priv_int>(nullptr), input_parties, size, ring_size, net, ss);
    // }

    // for (size_t i = 0; i < numInputParties; i++) {
    //     smc_open(result, res[i], size, -1);
    //     for (size_t j = 0; j < size; j++) {
    //         printf("(open) party %i's input  [%lu] %u\n", input_parties[i], j, result[j]);
    //     }
    //     // printf("\n");
    // }

    // std::vector<std::vector<int>> send_recv_map = ss->generateInputSendRecvMap(input_parties);
    // printf("---\nsend_recv_map\n");
    // for (auto var : send_recv_map) {
    //     std::cout << var << std::endl;
    // }

    for (size_t i = 0; i < totalNumShares; i++) {
        delete[] Data1[i];
        delete[] Data2[i];
    }

    delete[] Data1;
    delete[] Data2;
    delete[] A_bit;
    delete[] B_bit;

    for (size_t i = 0; i < numShares; i++) {
        delete[] c[i];
        delete[] d[i];
        delete[] B_sparse[i];
    }
    delete[] c;
    delete[] d;
    delete[] B_sparse;
    delete[] result;
    delete[] result_2;

    // for (size_t i = 0; i < numInputParties; i++) {
    //     for (size_t s = 0; s < numShares; s++) {
    //         delete[] res[i][s];
    //     }
    //     delete[] res[i];
    // }
    // delete[] res;
}
#endif

void SMC_Utils::smc_test_op(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, int threadID) {
#if __DEPLOYMENT__
    printf("DEPLOYMENT MODE\n");
#else
    printf("BENCHMARK MODE\n");
#endif

#if __SHAMIR__

    priv_int *res = (priv_int *)malloc(sizeof(priv_int) * size);
    priv_int *a_test = (priv_int *)malloc(sizeof(priv_int) * size);
    priv_int *res_check = (priv_int *)malloc(sizeof(priv_int) * size);
    for (int i = 0; i < size; ++i) {
        ss_init(res[i]);
        ss_init(a_test[i]);
        ss_init(res_check[i]);
    }

    int K = 10;
    int M = K - 1;

    priv_int field;
    ss_init(field);
    ss->getFieldSize(field);
    printf("Testing PRandInt for (K = %i, M = %i), size = %i)\n", K, M, size);
    PRandInt(K, M, size, a_test, threadID, ss);

    priv_int **S = (priv_int **)malloc(sizeof(priv_int *) * (M + 1));

    M = 32;
    for (int i = 0; i < M + 1; i++) {
        S[i] = (priv_int *)malloc(sizeof(priv_int) * size);
        for (int j = 0; j < size; j++)
            ss_init(S[i][j]);
    }
    Open_print(MPZ_CAST(a[2]), "a[2]", size, threadID, net, ss);

    doOperation_bitDec(S, MPZ_CAST(a[2]), M, M, size, threadID, net, ss); // problem here
    for (int i = 0; i < M + 1; i++) {
        Open_print(S[i], "S[0]", size, threadID, net, ss);
    }

    for (int i = 0; i < M + 1; i++) {
        for (int j = 0; j < size; j++)
            ss_clear(S[i][j]);
        free(S[i]);
    }
    free(S);
    // printf("Testing PRandBit for (K = %i, M = %i), size = %i)\n", K, M, size);
    // PRandBit(size, a, threadID, net, ss);
    // for (int i = 0; i < size; i++) {
    //     gmp_printf("a[%i] %Zu\n", i, a[i]);
    // }
    // Open(a, a, size, threadID, net, ss);
    // for (int i = 0; i < size; i++) {
    //     gmp_printf("result[%i] %Zu\n", i, a[i]);
    // }

    // K = 32;
    // int Kp1 = K + 1;
    // M = ceil(log2(K));
    // priv_int **V = (priv_int **)malloc(sizeof(priv_int *) * (Kp1));
    // for (int i = 0; i < Kp1; ++i) {
    //     V[i] = (priv_int *)malloc(sizeof(priv_int) * size);
    //     for (int j = 0; j < size; ++j)
    //         ss_init(V[i][j]);
    // }

    // // gmp_printf("Testing PRZS for (mod = %Zu), size = %i)\n", field, size);
    // ss->PRZS(field, size, a_test);
    // // printf("PRZS end\n");
    // Open_from_all(a_test, res, size, threadID, net, ss);
    // for (int i = 0; i < size; i++) {
    //     gmp_printf("result[%i] %Zu\n", i, res[i]);
    // }

    // PRandM(K, size, V, threadID, net, ss); // generating r', r'_k-1,...,r'_0

    // printf("Testing PRandM for (K = %i, M = %i), size = %i)\n", K, M, size);

    // for (int i = 0; i < Kp1 - 1; i++) {
    //     Open(V[i], res, size, threadID, net, ss);
    //     for (int j = 0; j < size; j++) {
    //         gmp_printf("%Zu ", res[j]);
    //     }
    //     printf("\n");
    // }
    // unsigned long pow = 1;

    // // Open(V[0], res, size, threadID, net, ss);
    // for (int j = 0; j < size; j++) {
    //     ss_set(res_check[j], V[0][j]); // setting the first bit of accumulator to b_0
    // }
    // priv_int temp;
    // ss_init(temp);

    // for (int i = 1; i < Kp1 - 1; i++) {
    //     pow = pow << 1;
    //     // Open(V[i], res, size, threadID, net, ss);
    //     for (int j = 0; j < size; j++) {
    //         mpz_mul_ui(temp, V[i][j], pow);
    //         ss->modAdd(res_check[j], res_check[j], temp);
    //     }
    // }

    // Open(V[Kp1 - 1], res, size, threadID, net, ss);
    // for (int j = 0; j < size; j++) {
    //     gmp_printf("result[%i] %Zu\n", j, res[j]);
    // }

    // Open(res_check, res, size, threadID, net, ss);
    // for (int j = 0; j < size; j++) {
    //     gmp_printf("res_check[%i] %Zu\n", j, res[j]);
    // }

    // for (int i = 0; i < Kp1; ++i) {
    //     for (int j = 0; j < size; ++j)
    //         ss_clear(V[i][j]);
    //     free(V[i]);
    // }

    for (int i = 0; i < size; ++i) {
        ss_clear(res[i]);
        ss_clear(a_test[i]);
        ss_clear(res_check[i]);
    }
    ss_clear(field);

    free(res_check);
    free(res);
    free(a_test);

#endif
}
