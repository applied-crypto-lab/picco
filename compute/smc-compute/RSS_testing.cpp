#include "RSS_types.hpp"
#include "SMC_Utils.h"
#include "ops/rss/FLLT.hpp"
#include "../../compiler/src/utility/utility.h"

void SMC_Utils::smc_test_rss(int threadID, int batch_size) {
    uint numShares = ss->getNumShares();
    uint totalNumShares = ss->getTotalNumShares();
    uint ring_size = ss->ring_size;
    int id = ss->getPartyNum(); // Party index (1-based)
    NodeNetwork &net = *network;

    // Test input floats
    float numbers_1[10] = {1.45, 3.67, 2.34, 5.89, 0.76, 8.12, 9.67, 3.21, 4.56, 7.89};
    float numbers_2[10] = {1.10, 4.67, 5.34, 2.89, 1.76, 8.12, 9.67, 3.21, 4.56, 7.89};

    // Allocate input arrays: [4][numShares][batch_size]
    priv_int_t ***in_1 = new priv_int_t **[4];
    priv_int_t ***in_2 = new priv_int_t **[4];
    for (int k = 0; k < 4; ++k) {
        in_1[k] = new priv_int_t *[numShares];
        in_2[k] = new priv_int_t *[numShares];
        for (uint s = 0; s < numShares; ++s) {
            in_1[k][s] = new priv_int_t[batch_size]();
            in_2[k][s] = new priv_int_t[batch_size]();
        }
    }

    // Convert floats to 4-tuple and fill the secret shares
    for (int i = 0; i < batch_size; ++i) {
        long long elements_1[4], elements_2[4];
        convertFloat(numbers_1[i], 32, 8, elements_1);
        convertFloat(numbers_2[i], 32, 8, elements_2);
        for (int k = 0; k < 4; ++k) {
            for (uint s = 0; s < numShares; ++s) {
                in_1[k][s][i] = elements_1[k];
                in_2[k][s][i] = elements_2[k];
            }
        }
    }

    // Output array: [numShares][batch_size]
    priv_int_t **out_1 = new priv_int_t *[numShares];
    for (uint s = 0; s < numShares; ++s)
        out_1[s] = new priv_int_t[batch_size]();

    printf("FLLT Started running...\n");
    FLLT(in_1, in_2, out_1, batch_size, ring_size, threadID, net, ss);
    printf("FLLT Finished.\n");

    // Reveal and print
    printf("\n====== FLLT Final Output ======\n");
    priv_int_t *output_vals = new priv_int_t[batch_size];
    Open(output_vals, out_1, batch_size, -1, net, ss);

    for (int i = 0; i < batch_size; ++i) {
        int64_t corrected = (output_vals[i] < (1ULL << 63)) ? output_vals[i] : (int64_t)(output_vals[i] - (1ULL << 64));
        int expected = (numbers_1[i] < numbers_2[i]) ? 1 : 0;
        printf("[i=%d] num1=%.6f num2=%.6f => FLLT result = %ld (expected: %d)\n",
               i, numbers_1[i], numbers_2[i], corrected, expected);
    }

    delete[] output_vals;
    for (int k = 0; k < 4; ++k) {
        for (uint s = 0; s < numShares; ++s) {
            delete[] in_1[k][s];
            delete[] in_2[k][s];
        }
        delete[] in_1[k];
        delete[] in_2[k];
    }
    delete[] in_1;
    delete[] in_2;
    for (uint s = 0; s < numShares; ++s)
        delete[] out_1[s];
    delete[] out_1;
}
