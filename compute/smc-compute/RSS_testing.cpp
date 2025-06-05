#include "RSS_types.hpp"
#include "SMC_Utils.h"
#include "ops/rss/FLLT.hpp"
#include <cfloat>

void convertFloat(float value, int K, int L, long long **elements) {
    unsigned int *newptr = (unsigned int *)&value;
    int s = *newptr >> 31; // Extract the sign bit
    int e = *newptr & 0x7f800000; // Extract the exponent
    e >>= 23;
    int m = 0;
    m = *newptr & 0x007fffff; // Extract the significand (mantissa)

    int z;
    long v, p, k;
    long long significand = 0, one = 1, two = 2, tmp = 0, tmpm = 0;

    if (e == 0 && m == 0) {
        s = 0;
        z = 1;
        significand = 0;
        p = 0;
    } else {
        z = 0;
        if (L < 8) {
            k = (1 << L) - 1; // Raise two to the power of L using shifting and subtract 1, then store it to k
            if (e - 127 - K + 1 > k) {
                p = k;
                significand = one << K;        // Raise one to the power of K and store it to significand
                significand = significand - 1; // Sub 1
            } else if (e - 127 - K + 1 < -k) {
                p = -k;
                significand = 1; // Set the value of significand to 1
            } else {
                p = e - 127 - K + 1;
                m = m + (1 << 23);
                tmpm = m; // Set the value of tmpm to m
                if (K < 24) {
                    try {
                        tmp = pow(two, (24 - K)); // Raise two to the power of (24 - K) using shifting and store it to tmp
                    } catch(const std::exception& e) {
                        throw std::runtime_error("An exception occurred during pow operation: " + std::string(e.what()));
                    }
                    if (tmp == 0) // Division by zero check
                        throw std::runtime_error("Division by zero: overflow in significand calculation");
                    significand = tmpm / tmp; // Perform division of tmpm to tmp and store it to significand
                } else {
                    significand = tmpm << (K - 24); // Raise tmpm to the power of (K - 24) and store it to significand
                }
            }
        } else {
            p = e - 127 - K + 1;
            m = m + (1 << 23);
            tmpm = m; // Set the value of tmpm to m
            if (K < 24) {
                try {
                    tmp = pow(two, (24 - K)); // Raise two to the power of (24 - K) using shifting and store it to tmp
                } catch(const std::exception& e) {
                    throw std::runtime_error("An exception occurred during pow operation: " + std::string(e.what()));
                }
                if (tmp == 0) // Division by zero check
                    throw std::runtime_error("Division by zero: overflow in significand calculation");
                significand = tmpm / tmp; // Perform division of tmpm to tmp and store it to significand
            } else {
                significand = tmpm;                    // Set significand to tmpm
                significand = significand << (K - 24); // Raise significand to the power of (K - 24) and store it to significand
            }
        }
    }

    printf("DEBUG convertFloat: float=%f, e=%d, K=%d, L=%d, p=%ld\n", value, e, K, L, p);

    // printf("sig  %lli\n", significand);
    // printf("p    %li\n", p);
    // printf("z    %i\n", z);
    // printf("sgn  %i\n", s);
    // Set the significand, p, z, and s value directly to the long long array of elements.
    (*elements)[0] = significand;
    (*elements)[1] = p;
    (*elements)[2] = z;
    (*elements)[3] = s;

}

void SMC_Utils::smc_test_rss(int threadID, int batch_size) {
    uint numShares = ss->getNumShares();
    uint totalNumShares = ss->getTotalNumShares();
    uint ring_size = ss->ring_size;

    // Test input floats
    // These arrays include only the (num1, num2) pairs for which FLLT failed:
    float numbers_1[14] = {
        1.45f,
        1.45f,
        -2.0f,
        -9999.0f,
        -1e-6f,
        1.45f,
        -1.0f,
        -0.0f,
        -340282346638528859811704183484516925440.0f, // -FLT_MAX as double-precision
        -INFINITY,
        340282346638528859811704183484516925440.0f,  // FLT_MAX as double-precision
        -340282346638528859811704183484516925440.0f, // -FLT_MAX as double-precision
        -2.0f,
        -1.0f
    };

    float numbers_2[14] = {
        1.10f,
        1.05f,
        2.0f,
        -10000.0f,
        -2e-6f,
        1.10f,
        1.0f,
        0.0f,
        340282346638528859811704183484516925440.0f,  // FLT_MAX as double-precision
        INFINITY,
        -340282346638528859811704183484516925440.0f, // -FLT_MAX as double-precision
        340282346638528859811704183484516925440.0f,  // FLT_MAX as double-precision
        2.0f,
        -1.0f
    };

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
        long long* ptr1 = elements_1;
        long long* ptr2 = elements_2;
        convertFloat(numbers_1[i], 32, 8, &ptr1);
        convertFloat(numbers_2[i], 32, 8, &ptr2);
        // === [START] Print the original float and its 4-part converted representation ===
        std::cout << "Number 1: " << numbers_1[i] << " -> Converted (mantissa, exponent, zero_flag, sign): ";
        for (int j = 0; j < 4; j++) std::cout << elements_1[j] << " ";
        std::cout << std::endl;

        std::cout << "Number 2: " << numbers_2[i] << " -> Converted (mantissa, exponent, zero_flag, sign): ";
        for (int j = 0; j < 4; j++) std::cout << elements_2[j] << " ";
        std::cout << std::endl;
        // === [END] Print section ===

        // Store the converted values into the in_1 and in_2 arrays
        for (uint s = 0; s < numShares; ++s) {
            for (int j = 0; j < 4; j++) {
                // If j == 1 (exponent), reinterpret the signed int as unsigned
                if (j == 1) {
                    in_1[j][s][i] = static_cast<priv_int_t>(static_cast<int32_t>(elements_1[j]));
                    in_2[j][s][i] = static_cast<priv_int_t>(static_cast<int32_t>(elements_2[j]));
                } else {
                    in_1[j][s][i] = static_cast<priv_int_t>(elements_1[j]);
                    in_2[j][s][i] = static_cast<priv_int_t>(elements_2[j]);
                }
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
        if (corrected != expected) {
            printf("####### Test Failure at index %d ###\n", i);
            printf("Test failed for index %d: Expected %d, got %lld\n", i, expected, corrected);
            printf("Input 1: %f, Input 2: %f\n", numbers_1[i], numbers_2[i]);
            printf("Converted 1: (%lld, %lld, %lld, %lld)\n", in_1[0][0][i], in_1[1][0][i], in_1[2][0][i], in_1[3][0][i]);
            printf("Converted 2: (%lld, %lld, %lld, %lld)\n", in_2[0][0][i], in_2[1][0][i], in_2[2][0][i], in_2[3][0][i]);
            printf("Batch size: %d, Ring size: %d, Thread ID: %d\n", batch_size, ring_size, threadID);
            printf("###### FLLT Test Failed #######\n");
        } else {
            printf("Test passed for index %d: Expected %d, got %lld\n", i, expected, corrected);
        } 
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
