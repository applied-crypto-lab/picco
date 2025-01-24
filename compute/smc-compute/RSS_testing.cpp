// this file is to be used for development testing of RSS operations. If you
// have created a new file in the ops/rss directory, you will need to add the
// new files name to the rss/RSSheaders.hpp file. Otherwise, the code will not
// successfully compile.

#include "RSS_types.hpp"
#include "SMC_Utils.h"

void SMC_Utils::smc_test_rss(int threadID, int batch_size) {
    // "batch_size"  corresponds to how many operations are
    // we performing in a single protocol invocation.

    uint numShares = ss->getNumShares();           // the number of shares each party maintains
    uint totalNumShares = ss->getTotalNumShares(); // the total number of shares a secret is split into
    uint ring_size = ss->ring_size;                // the ring size of the computation ("k" in the literature)
    printf("ring_size : %u\n", ring_size);
    printf("size : %u\n", batch_size);
    printf("----\n\n");

    // this is the setup process for producing secret shares. You can ignore
    // the key initialization, as it's not particularly relevant
    __m128i *key_prg;
    uint8_t key_raw[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    key_prg = offline_prg_keyschedule(key_raw);
    uint8_t k1[] = {0x31, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    uint8_t k2[] = {0xa1, 0x34, 0x6f, 0x67, 0x10, 0x1b, 0x13, 0xa3, 0x56, 0x45, 0x90, 0xb2, 0x13, 0xe3, 0x23, 0x24};

    // this defines the ordering in which shares of a secret are stored by each
    // party this is critical for consistency across the parties for certain
    // operations Consider the following example for 3 parties: a secret x is
    // split into shares x_1, x_2, x_3, such that x = x_1 + x_2 + x_3. The
    // parties store their shares in the following orders:
    //
    // party_1: [x_2, x_3] (share_mapping {1,2})
    // party_2: [x_3, x_1] (share_mapping {2,0})
    // party_3: [x_1, x_2] (share_mapping {0,1})
    //
    // For more than 3 parties, it is a bit more complicated, but you should
    // just use the ones defined below of 5 and 7 parties

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

    // initalizing the variable which store ALL of the shares that are generated (see below)
    priv_int_t **Data1 = new priv_int_t *[totalNumShares];
    priv_int_t **Data2 = new priv_int_t *[totalNumShares];
    for (int i = 0; i < totalNumShares; i++) {
        Data1[i] = new priv_int_t[batch_size];
        memset(Data1[i], 0, sizeof(priv_int_t) * batch_size);
        Data2[i] = new priv_int_t[batch_size];
        memset(Data2[i], 0, sizeof(priv_int_t) * batch_size);
    }

    // To secret share a private value using RSS, we generate (totalNumShares - 1)
    // pseudorandom values. The "last" share (stored in position [totalNumShares-1]) is then
    // computed as the secret value, minus all the pseudorandomly generated
    // shares.
    //
    // Recalling our 3-party example, we have a secret x that we wish to secret share. We:
    //   1. pseudorandomly generate the shares x_1 and x_2 by calling prg_aes_ni()
    //   2. compute x_3 as follows:
    //
    //          x_3 = x - x_1 - x_2
    //
    // Thus satisfying our requirement that x = x_1 + x_2 + x_3

    priv_int_t fixed_secret = 1; // an example, fixed secret value
    for (int i = 0; i < batch_size; i++) {
        // generating the random shares by calling our PRG
        for (size_t j = 0; j < totalNumShares - 1; j++) {
            prg_aes_ni(Data1[j] + i, k1, key_prg);
            prg_aes_ni(Data2[j] + i, k2, key_prg);
        }

        // Setting the "secret value". You can modify these lines in order to
        // control the secret inputs you're using for testing.

        Data1[totalNumShares - 1][i] = i; // The secret values in Data1 are the integers 0, 1, ..., b_size - 1

        Data2[totalNumShares - 1][i] = fixed_secret; // all (b_size) shares of Data2 are that of the fixed_secret
        // Though all shares in different [i] positions are unique, they will all reconstruct to fixed_secret.

        for (size_t j = 0; j < totalNumShares - 1; j++) {
            // computing the "final" share
            Data1[totalNumShares - 1][i] -= Data1[j][i];
            Data2[totalNumShares - 1][i] -= Data2[j][i];
        }
    }

    // Private array initalization
    // for RSS, priv_int is typedef (in RSS_types.hpp) as either a 32- or 64-bit POINTER
    // so these variables are actually two-dimensional arrays
    // The dimensions of RSS variables are
    //
    // var[numShares][batch_size]
    //
    // This is so we are guaranteed to have large contiguous blocks of memory, which improves 
    // performance.

    priv_int *in_1 = new priv_int[ss->getNumShares()];
    priv_int *in_2 = new priv_int[ss->getNumShares()];
    priv_int *out_1 = new priv_int[ss->getNumShares()];
    priv_int *out_2 = new priv_int[ss->getNumShares()];

    priv_int res_1 = new priv_int_t[batch_size];
    priv_int res_2 = new priv_int_t[batch_size];

    for (int i = 0; i < ss->getNumShares(); i++) {
        out_1[i] = new priv_int_t[batch_size];
        memset(out_1[i], 0, sizeof(priv_int_t) * batch_size);
        out_2[i] = new priv_int_t[batch_size];
        memset(out_2[i], 0, sizeof(priv_int_t) * batch_size);
    }

    // assigning the shares to the correct locations, according to the share mapping we defined previously
    for (size_t i = 0; i < numShares; i++) {
        in_1[i] = Data1[share_mapping[id - 1][i]];
        in_2[i] = Data2[share_mapping[id - 1][i]];
    }

    // Let's first verify that our shares (stored in in_1 and in_2) reconstruct to what we expect:
    // in_1 should reconstruct to 0, 1, ..., b_size-1
    // in_2 should reconstruct to 1, 1, ..., 1
    Open(res_1, in_1, batch_size, -1, net, ss);
    Open(res_2, in_2, batch_size, -1, net, ss);

    for (priv_int_t i = 0; i < batch_size; i++) {
        if (!(res_1[i] == i)) {
            printf("in_1 did not reconstruct properly\n");
            printf("(expect)   [%u]: %u\n", i, i);
            printf("(actual)   [%u]: %u\n", i, res_2[i]);
        }
        if (!(res_2[i] == fixed_secret)) {

            printf("in_2 did not reconstruct properly\n");
            printf("(expect)   [%u]: %u\n", i, fixed_secret);
            printf("(actual)   [%u]: %u\n", i, res_2[i]);
        }
    }

    // For illustrative purposes, suppose we want to test the protocol EQZ for
    // correctness. The expected behavior for EQZ is that it returns 1 if the
    // input (in_1 in this example) is zero, and 0 otherwise (in_1 > 0). The
    // implementation will be performing (b_size) many operations in parallel.
    // The secret-shared result is stored in out_1.

    doOperation_EQZ(in_1, out_1, ring_size, batch_size, -1, net, ss);

    // Once we have obtained the output of the protocol, let's reconstruct the
    // result to verify correctness of our implementation. We also open the
    // secret input (in_1), since we need to know what input was supplied to
    // the function.
    Open(res_1, out_1, batch_size, -1, net, ss);
    Open(res_2, in_1, batch_size, -1, net, ss);

    for (size_t i = 0; i < batch_size; i++) {
        // checking if the result of eqz (res_1) is what we expected, which we
        // do by checking if res_2 (reconstruction of in_1) is, in fact, equal
        // to zero. If this check fails, then we have an error somewhere in our
        // protocol.
        priv_int_t check = (res_2[i] == 0);
        if (!(res_1[i] == check)) {
            printf("EQZ ERROR\n");
            // for certain operations, it may be useful to display the
            // individual bits of our values, which can be done by calling:
            //
            // print_binary(result[i], ring_size);
            //
            printf("(input)    [%lu]: %u\n", i, res_2[i]);
            printf("(expect)   [%lu]: %u\n", i, res_1[i]);
            printf("(actual)   [%lu]: %u\n", i, check);
        }
    }

    // memory cleanup
    // every variable initialized with new MUST be explicitly freed
    delete[] res_1;
    delete[] res_2;

    for (size_t i = 0; i < totalNumShares; i++) {
        delete[] Data1[i];
        delete[] Data2[i];
    }

    delete[] Data1;
    delete[] Data2;

    for (size_t i = 0; i < numShares; i++) {
        delete[] out_1[i];
        delete[] out_2[i];
    }
    delete[] in_1;
    delete[] in_2;
    delete[] out_1;
    delete[] out_2;
}

////////////////////////////////////////////////
// PRG-related functions, do not modify
////////////////////////////////////////////////

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
