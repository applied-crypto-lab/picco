#include "SMC_Utils.h"
#include <chrono>

#if __RSS__
#include "rss/RSS_types.hpp"
#include "ops/rss/FLDiv.hpp"
#include "ops/rss/FLMult.hpp"
#include "ops/rss/FL2Int.hpp"
#include "ops/rss/Int2FL.hpp"
#include "ops/rss/Open.hpp"
#include "ops/rss/RandBit.hpp"
#include "ops/rss/LT.hpp"

void SMC_Utils::smc_test_rss(int threadID, int batch_size) {
    fprintf(stderr, "[DEBUG] smc_test_rss entered for FLDiv testing\n");
    fflush(stderr);

    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;
    fprintf(stderr, "[DEBUG] numShares=%u, ring_size=%u\n", numShares, ring_size);
    fflush(stderr);

    // ============= RANDBIT TEST (3x REPLICATION) =============
    printf("\n===== RANDBIT TEST (3x Replication) =====\n");
    fflush(stdout);

    int randbit_size = 20;  // Generate 20 random bits
    printf("Generating %d random bits using 3x replication...\n", randbit_size);
    fflush(stdout);

    // Allocate arrays in [size][numShares] format
    priv_int_t **randbit_result = new priv_int_t*[randbit_size];
    for (int i = 0; i < randbit_size; i++) {
        randbit_result[i] = new priv_int_t[numShares]();
    }

    // Time the RandBit operation
    auto randbit_start = std::chrono::high_resolution_clock::now();
    Rss_RandBit(randbit_result, randbit_size, ring_size, net, ss);
    auto randbit_end = std::chrono::high_resolution_clock::now();
    auto randbit_time = std::chrono::duration_cast<std::chrono::milliseconds>(randbit_end - randbit_start).count();

    printf("RandBit (3x replication) completed in %ld ms\n\n", randbit_time);
    fflush(stdout);

    // Open results
    priv_int_t *randbit_output = new priv_int_t[randbit_size];
    Open(randbit_output, randbit_result, randbit_size, -1, net, ss);

    printf("Random bits generated:\n  ");
    int randbit_valid = 0;
    int count_zeros = 0;
    int count_ones = 0;
    for (int i = 0; i < randbit_size; i++) {
        priv_int_t bit = randbit_output[i];
        printf("%lld ", (long long)bit);
        if (bit == 0) {
            count_zeros++;
            randbit_valid++;
        } else if (bit == 1) {
            count_ones++;
            randbit_valid++;
        }
        if ((i + 1) % 10 == 0) printf("\n  ");
    }
    printf("\n\n");

    printf("Validation: %d/%d bits are valid (0 or 1)\n", randbit_valid, randbit_size);
    printf("Distribution: %d zeros, %d ones\n", count_zeros, count_ones);
    if (randbit_valid == randbit_size) {
        printf("RandBit test: PASSED\n");
    } else {
        printf("RandBit test: FAILED (some bits are not 0 or 1)\n");
    }
    fflush(stdout);

    // Cleanup
    for (int i = 0; i < randbit_size; i++) {
        delete[] randbit_result[i];
    }
    delete[] randbit_result;
    delete[] randbit_output;

    printf("\n===== RANDBIT TEST COMPLETE =====\n\n");
    fflush(stdout);

    // ============= RANDBIT PERFORMANCE COMPARISON =============
    printf("\n===== RANDBIT PERFORMANCE COMPARISON =====\n");
    printf("Comparing 3x replication vs single RandBit (internal)\n\n");
    fflush(stdout);

    int perf_size = 100;  // Generate 100 random bits for timing

    // Allocate arrays
    priv_int_t **perf_result = new priv_int_t*[perf_size];
    for (int i = 0; i < perf_size; i++) {
        perf_result[i] = new priv_int_t[numShares]();
    }

    // Time single RandBit (internal) - 3 calls to simulate what 3x replication does
    auto single_start = std::chrono::high_resolution_clock::now();
    Rss_RandBit_Internal(perf_result, perf_size, ring_size, net, ss);
    auto single_end = std::chrono::high_resolution_clock::now();
    auto single_time = std::chrono::duration_cast<std::chrono::microseconds>(single_end - single_start).count();

    // Reset result
    for (int i = 0; i < perf_size; i++) {
        memset(perf_result[i], 0, sizeof(priv_int_t) * numShares);
    }

    // Time 3x replication RandBit
    auto triple_start = std::chrono::high_resolution_clock::now();
    Rss_RandBit(perf_result, perf_size, ring_size, net, ss);
    auto triple_end = std::chrono::high_resolution_clock::now();
    auto triple_time = std::chrono::duration_cast<std::chrono::microseconds>(triple_end - triple_start).count();

    printf("Single RandBit (internal): %ld µs for %d bits (%.2f µs/bit)\n",
           single_time, perf_size, (double)single_time / perf_size);
    printf("3x Replication RandBit:    %ld µs for %d bits (%.2f µs/bit)\n",
           triple_time, perf_size, (double)triple_time / perf_size);
    printf("Overhead ratio: %.2fx\n", (double)triple_time / single_time);
    printf("\nNote: 3x replication provides statistical security without requiring 64-bit rings\n");
    printf("for all operations. The overhead is localized to RandBit-dependent operations.\n");
    fflush(stdout);

    // Cleanup
    for (int i = 0; i < perf_size; i++) {
        delete[] perf_result[i];
    }
    delete[] perf_result;

    printf("\n===== RANDBIT PERFORMANCE COMPARISON COMPLETE =====\n\n");
    fflush(stdout);

    // ============= LT (LESS THAN) TEST =============
    // This tests that LT works correctly with 3x replication RandBit
    // LT uses MSB which uses RandBit internally
    printf("\n===== LT (Less Than) TEST =====\n");
    printf("Testing LT operation (uses MSB which uses RandBit internally)\n");
    fflush(stdout);

    int lt_size = 8;
    // Test cases: pairs of values to compare
    int lt_a_vals[] = {10, 5, 100, 0, -5, 7, 1000, 50};
    int lt_b_vals[] = {5, 10, 100, 1, -10, 7, 999, 100};
    int lt_expected[] = {0, 1, 0, 1, 0, 0, 0, 1};  // 1 if a < b, 0 otherwise

    printf("Test cases (a < b):\n");
    for (int i = 0; i < lt_size; i++) {
        printf("  Test %d: %d < %d = %d (expected)\n", i, lt_a_vals[i], lt_b_vals[i], lt_expected[i]);
    }
    printf("\n");
    fflush(stdout);

    // Allocate arrays in [size][numShares] format
    priv_int_t **lt_a = new priv_int_t*[lt_size];
    priv_int_t **lt_b = new priv_int_t*[lt_size];
    priv_int_t **lt_result = new priv_int_t*[lt_size];

    for (int i = 0; i < lt_size; i++) {
        lt_a[i] = new priv_int_t[numShares]();
        lt_b[i] = new priv_int_t[numShares]();
        lt_result[i] = new priv_int_t[numShares]();
        ss->sparsify_public(lt_a[i], lt_a_vals[i]);
        ss->sparsify_public(lt_b[i], lt_b_vals[i]);
    }

    // Time the LT operation
    auto lt_start = std::chrono::high_resolution_clock::now();
    doOperation_LT(lt_result, lt_a, lt_b, 32, 32, 1, lt_size, threadID, net, ss);
    auto lt_end = std::chrono::high_resolution_clock::now();
    auto lt_time = std::chrono::duration_cast<std::chrono::milliseconds>(lt_end - lt_start).count();

    printf("LT operation completed in %ld ms\n\n", lt_time);
    fflush(stdout);

    // Open results
    priv_int_t *lt_output = new priv_int_t[lt_size];
    Open(lt_output, lt_result, lt_size, -1, net, ss);

    printf("Results:\n");
    int lt_passed = 0;
    for (int i = 0; i < lt_size; i++) {
        int actual = (int)lt_output[i];
        bool test_passed = (actual == lt_expected[i]);
        printf("  Test %d: %d < %d = %d (expected %d) - %s\n",
               i, lt_a_vals[i], lt_b_vals[i], actual, lt_expected[i],
               test_passed ? "PASSED" : "FAILED");
        if (test_passed) lt_passed++;
    }

    printf("\nLT test: %d/%d passed\n", lt_passed, lt_size);
    if (lt_passed == lt_size) {
        printf("LT test: PASSED (3x replication RandBit works correctly with MSB)\n");
    } else {
        printf("LT test: FAILED\n");
    }
    fflush(stdout);

    // Cleanup
    for (int i = 0; i < lt_size; i++) {
        delete[] lt_a[i];
        delete[] lt_b[i];
        delete[] lt_result[i];
    }
    delete[] lt_a;
    delete[] lt_b;
    delete[] lt_result;
    delete[] lt_output;

    printf("\n===== LT TEST COMPLETE =====\n\n");
    fflush(stdout);

    // Skip float tests if ring_size < 64 (they require larger rings)
    if (ring_size < 64) {
        printf("\n===== SKIPPING FLOAT TESTS (ring_size=%u < 64) =====\n", ring_size);
        printf("Float operations require 64-bit rings. Run with ring_size=64 to test.\n\n");
        fflush(stdout);
        return;
    }

    // ============= BATCH FLOAT DIVISION (FLDiv) TEST =============
    printf("\n===== BATCH FLOAT DIVISION (FLDiv) TEST =====\n");
    fflush(stdout);

    // Test cases with actual float values
    int fldiv_K = 23;  // IEEE 754 single precision mantissa bits
    int fldiv_size = 5;

    // Test values: simple divisions
    float test_a[] = {8.0f, 10.0f, 12.0f, 6.0f, 15.0f};
    float test_b[] = {2.0f, 5.0f, 3.0f, 4.0f, 3.0f};
    float expected[] = {4.0f, 2.0f, 4.0f, 1.5f, 5.0f};

    printf("Test cases:\n");
    for (int i = 0; i < fldiv_size; i++) {
        printf("  Test %d: %.2f / %.2f = %.2f (expected)\n", i, test_a[i], test_b[i], expected[i]);
    }
    printf("\n");
    fflush(stdout);

    // Allocate float arrays: [component][size][numShares]
    // component 0 = mantissa, 1 = exponent, 2 = zero flag, 3 = sign
    priv_int_t ***fldiv_a = new priv_int_t**[4];
    priv_int_t ***fldiv_b = new priv_int_t**[4];
    priv_int_t ***fldiv_result = new priv_int_t**[4];
    priv_int_t **fldiv_error = new priv_int_t*[fldiv_size];

    for (int comp = 0; comp < 4; comp++) {
        fldiv_a[comp] = new priv_int_t*[fldiv_size];
        fldiv_b[comp] = new priv_int_t*[fldiv_size];
        fldiv_result[comp] = new priv_int_t*[fldiv_size];
        for (int i = 0; i < fldiv_size; i++) {
            fldiv_a[comp][i] = new priv_int_t[numShares]();
            fldiv_b[comp][i] = new priv_int_t[numShares]();
            fldiv_result[comp][i] = new priv_int_t[numShares]();
        }
    }

    for (int i = 0; i < fldiv_size; i++) {
        fldiv_error[i] = new priv_int_t[numShares]();
    }

    fprintf(stderr, "[DEBUG] Converting float values to private format...\n");
    fflush(stderr);

    // Convert each float value to IEEE 754 components manually
    for (int i = 0; i < fldiv_size; i++) {
        // Decompose test_a[i]
        uint32_t a_bits;
        memcpy(&a_bits, &test_a[i], sizeof(float));
        uint32_t a_sign = (a_bits >> 31) & 1;
        int32_t a_exp = ((a_bits >> 23) & 0xFF) - 127;  // Unbiased exponent
        uint32_t a_mantissa = (a_bits & 0x7FFFFF) | 0x800000;  // Add implicit 1

        // Decompose test_b[i]
        uint32_t b_bits;
        memcpy(&b_bits, &test_b[i], sizeof(float));
        uint32_t b_sign = (b_bits >> 31) & 1;
        int32_t b_exp = ((b_bits >> 23) & 0xFF) - 127;
        uint32_t b_mantissa = (b_bits & 0x7FFFFF) | 0x800000;

        fprintf(stderr, "[DEBUG] Test %d: %.2f -> m=%u e=%d z=0 s=%u\n", i, test_a[i],
                a_mantissa, a_exp, a_sign);
        fprintf(stderr, "[DEBUG] Test %d: %.2f -> m=%u e=%d z=0 s=%u\n", i, test_b[i],
                b_mantissa, b_exp, b_sign);

        // Use sparsify_public to properly embed public test values into shares
        // This correctly handles the RSS share structure based on party ID
        ss->sparsify_public(fldiv_a[0][i], (int)a_mantissa);
        ss->sparsify_public(fldiv_a[1][i], (int)a_exp);
        ss->sparsify_public(fldiv_a[2][i], 0);  // zero flag
        ss->sparsify_public(fldiv_a[3][i], (int)a_sign);

        ss->sparsify_public(fldiv_b[0][i], (int)b_mantissa);
        ss->sparsify_public(fldiv_b[1][i], (int)b_exp);
        ss->sparsify_public(fldiv_b[2][i], 0);
        ss->sparsify_public(fldiv_b[3][i], (int)b_sign);
    }

    fprintf(stderr, "\n[DEBUG] ========== CALLING FLDIV ==========\n");
    fprintf(stderr, "[DEBUG] Calling doOperation_FLDiv with K=%d, size=%d\n", fldiv_K, fldiv_size);
    fflush(stderr);

    printf("Executing FLDiv (K=%d)...\n", fldiv_K);
    fflush(stdout);

    doOperation_FLDiv(fldiv_a, fldiv_b, fldiv_result, fldiv_error, fldiv_K, fldiv_size, threadID, net, ss);

    fprintf(stderr, "[DEBUG] doOperation_FLDiv completed\n");
    fflush(stderr);
    printf("FLDiv execution completed.\n\n");
    fflush(stdout);

    // Open results
    priv_int_t *fldiv_m_output = new priv_int_t[fldiv_size];
    priv_int_t *fldiv_e_output = new priv_int_t[fldiv_size];
    priv_int_t *fldiv_z_output = new priv_int_t[fldiv_size];
    priv_int_t *fldiv_s_output = new priv_int_t[fldiv_size];
    priv_int_t *fldiv_err_output = new priv_int_t[fldiv_size];

    Open(fldiv_m_output, fldiv_result[0], fldiv_size, -1, net, ss);
    Open(fldiv_e_output, fldiv_result[1], fldiv_size, -1, net, ss);
    Open(fldiv_z_output, fldiv_result[2], fldiv_size, -1, net, ss);
    Open(fldiv_s_output, fldiv_result[3], fldiv_size, -1, net, ss);
    Open(fldiv_err_output, fldiv_error, fldiv_size, -1, net, ss);

    fprintf(stderr, "\n[DEBUG] ========== FLOAT DIVISION RESULTS ==========\n");
    printf("\n===== RESULTS =====\n");

    int passed = 0;
    for (int i = 0; i < fldiv_size; i++) {
        uint64_t actual_m = (uint64_t)fldiv_m_output[i];
        int64_t actual_e = (int64_t)fldiv_e_output[i];
        uint64_t actual_z = (uint64_t)fldiv_z_output[i];
        uint64_t actual_s = (uint64_t)fldiv_s_output[i];
        uint64_t err = (uint64_t)fldiv_err_output[i];

        // Convert result back to float for comparison
        // float = mantissa * 2^(exponent - (K-1))
        double actual_float = 0.0;
        if (actual_z == 0 && actual_m > 0) {
            double mantissa_normalized = (double)actual_m / (1ULL << (fldiv_K - 1));
            actual_float = mantissa_normalized * pow(2.0, (double)actual_e);
            if (actual_s != 0) actual_float = -actual_float;
        }

        double expected_float = (double)expected[i];
        double error_pct = fabs((actual_float - expected_float) / expected_float) * 100.0;
        bool test_passed = (error_pct < 5.0);  // Allow 5% error

        fprintf(stderr, "[DEBUG] Test %d: %.2f / %.2f = %.6f (expected %.2f)\n",
                i, test_a[i], test_b[i], actual_float, expected[i]);
        fprintf(stderr, "[DEBUG]   Raw result: m=%lu e=%ld z=%lu s=%lu err=%lu\n",
                actual_m, actual_e, actual_z, actual_s, err);
        fprintf(stderr, "[DEBUG]   Error: %.2f%% - %s\n\n",
                error_pct, test_passed ? "PASSED" : "FAILED");

        printf("Test %d: %.2f / %.2f = %.6f (expected %.2f, error %.2f%%) - %s\n",
               i, test_a[i], test_b[i], actual_float, expected[i], error_pct,
               test_passed ? "PASSED" : "FAILED");

        if (test_passed) passed++;
    }

    fprintf(stderr, "[DEBUG] ========================================\n\n");
    printf("\nFloat Division: %d/%d tests passed\n", passed, fldiv_size);
    fflush(stdout);

    // Cleanup
    for (int comp = 0; comp < 4; comp++) {
        for (int i = 0; i < fldiv_size; i++) {
            delete[] fldiv_a[comp][i];
            delete[] fldiv_b[comp][i];
            delete[] fldiv_result[comp][i];
        }
        delete[] fldiv_a[comp];
        delete[] fldiv_b[comp];
        delete[] fldiv_result[comp];
    }
    delete[] fldiv_a;
    delete[] fldiv_b;
    delete[] fldiv_result;

    for (int i = 0; i < fldiv_size; i++) {
        delete[] fldiv_error[i];
    }
    delete[] fldiv_error;

    delete[] fldiv_m_output;
    delete[] fldiv_e_output;
    delete[] fldiv_z_output;
    delete[] fldiv_s_output;
    delete[] fldiv_err_output;

    printf("\n===== FLDiv TEST COMPLETE =====\n");
    fflush(stdout);

    // ============= MDiv vs SDiv COMPARISON TEST =============
    printf("\n===== MDiv vs SDiv COMPARISON TEST =====\n");
    fflush(stdout);

    // Test MDiv (Goldschmidt) vs SDiv (Newton-Raphson) for mantissa division
    int mdiv_K = 24;     // mantissa bit length
    int mdiv_lambda = 2; // precision parameter for MDiv (reduced for 32-bit ring)
    int mdiv_size = 4;

    // Test values: mantissa pairs in range [2^{K-1}, 2^K)
    // These represent normalized mantissas
    priv_int_t mdiv_m1_vals[] = {12582912, 14680064, 8388608, 16777215};  // 0.75, 0.875, 0.5, ~1.0 in normalized form
    priv_int_t mdiv_m2_vals[] = {8388608, 8388608, 16777215, 8388608};    // 0.5, 0.5, ~1.0, 0.5
    double mdiv_expected[] = {1.5, 1.75, 0.5, 2.0};  // m1/m2

    printf("Test cases (mantissa division m1/m2):\n");
    for (int i = 0; i < mdiv_size; i++) {
        printf("  Test %d: %lld / %lld = %.2f (expected ratio)\n", i,
               (long long)mdiv_m1_vals[i], (long long)mdiv_m2_vals[i], mdiv_expected[i]);
    }
    printf("\n");
    fflush(stdout);

    // Allocate arrays in [size][numShares] format
    priv_int_t **mdiv_m1 = new priv_int_t*[mdiv_size];
    priv_int_t **mdiv_m2 = new priv_int_t*[mdiv_size];
    priv_int_t **mdiv_result = new priv_int_t*[mdiv_size];
    priv_int_t **sdiv_result = new priv_int_t*[mdiv_size];

    for (int i = 0; i < mdiv_size; i++) {
        mdiv_m1[i] = new priv_int_t[numShares]();
        mdiv_m2[i] = new priv_int_t[numShares]();
        mdiv_result[i] = new priv_int_t[numShares]();
        sdiv_result[i] = new priv_int_t[numShares]();

        ss->sparsify_public(mdiv_m1[i], mdiv_m1_vals[i]);
        ss->sparsify_public(mdiv_m2[i], mdiv_m2_vals[i]);
    }

    printf("Testing SDiv (Newton-Raphson)...\n");
    fflush(stdout);
    auto sdiv_start = std::chrono::high_resolution_clock::now();
    doOperation_SDiv(sdiv_result, mdiv_m1, mdiv_m2, mdiv_K, mdiv_size, threadID, net, ss);
    auto sdiv_end = std::chrono::high_resolution_clock::now();
    auto sdiv_time = std::chrono::duration_cast<std::chrono::milliseconds>(sdiv_end - sdiv_start).count();
    printf("SDiv completed in %ld ms\n\n", sdiv_time);

    printf("Testing MDiv (Goldschmidt)...\n");
    fflush(stdout);
    auto mdiv_start = std::chrono::high_resolution_clock::now();
    doOperation_MDiv(mdiv_m1, mdiv_m2, mdiv_result, mdiv_lambda, mdiv_K, mdiv_size, threadID, net, ss);
    auto mdiv_end = std::chrono::high_resolution_clock::now();
    auto mdiv_time = std::chrono::duration_cast<std::chrono::milliseconds>(mdiv_end - mdiv_start).count();
    printf("MDiv completed in %ld ms\n\n", mdiv_time);

    // Open results
    priv_int_t *sdiv_output = new priv_int_t[mdiv_size];
    priv_int_t *mdiv_output = new priv_int_t[mdiv_size];
    Open(sdiv_output, sdiv_result, mdiv_size, -1, net, ss);
    Open(mdiv_output, mdiv_result, mdiv_size, -1, net, ss);

    printf("\n===== MDiv vs SDiv RESULTS =====\n");
    printf("%-8s %-15s %-15s %-15s %-15s %-10s\n",
           "Test", "SDiv Result", "MDiv Result", "Difference", "Expected*2^K", "Status");
    printf("--------------------------------------------------------------------------------\n");

    int mdiv_passed = 0;
    for (int i = 0; i < mdiv_size; i++) {
        priv_int_t expected_val = (priv_int_t)(mdiv_expected[i] * (1ULL << mdiv_K));
        priv_int_t sdiv_val = sdiv_output[i];
        priv_int_t mdiv_val = mdiv_output[i];
        long long diff = (long long)(sdiv_val - mdiv_val);

        // Check if MDiv result is within 1% of SDiv result
        double error_pct = 0;
        if (sdiv_val != 0) {
            error_pct = fabs((double)diff / (double)sdiv_val) * 100.0;
        }
        bool test_passed = (error_pct < 1.0);

        printf("%-8d %-15lld %-15lld %-15lld %-15lld %s\n",
               i, (long long)sdiv_val, (long long)mdiv_val, diff,
               (long long)expected_val, test_passed ? "PASS" : "FAIL");

        if (test_passed) mdiv_passed++;
    }

    printf("\nMDiv vs SDiv: %d/%d tests passed\n", mdiv_passed, mdiv_size);
    printf("SDiv time: %ld ms, MDiv time: %ld ms\n", sdiv_time, mdiv_time);
    fflush(stdout);

    // Cleanup
    for (int i = 0; i < mdiv_size; i++) {
        delete[] mdiv_m1[i];
        delete[] mdiv_m2[i];
        delete[] mdiv_result[i];
        delete[] sdiv_result[i];
    }
    delete[] mdiv_m1;
    delete[] mdiv_m2;
    delete[] mdiv_result;
    delete[] sdiv_result;
    delete[] sdiv_output;
    delete[] mdiv_output;

    printf("\n===== MDiv vs SDiv TEST COMPLETE =====\n");
    fflush(stdout);

    // ============= FLOAT TO INTEGER (FL2Int) TEST =============
    printf("\n===== FLOAT TO INTEGER (FL2Int) TEST =====\n");
    fflush(stdout);

    // Test cases: floats to convert to integers
    // FL2Int truncates toward zero
    int fl2int_L = 24;  // mantissa bits
    int fl2int_K = 8;   // exponent bits
    int fl2int_gamma = 32;  // output integer bits
    int fl2int_size = 5;

    float fl2int_inputs[] = {42.0f, 3.75f, -7.5f, 0.25f, 100.0f};
    int64_t fl2int_expected[] = {42, 3, -7, 0, 100};

    printf("Test cases:\n");
    for (int i = 0; i < fl2int_size; i++) {
        printf("  Test %d: FL2Int(%.2f) = %ld (expected)\n", i, fl2int_inputs[i], fl2int_expected[i]);
    }
    printf("\n");
    fflush(stdout);

    // Allocate float arrays: [component][size][numShares]
    priv_int_t ***fl2int_float = new priv_int_t**[4];
    priv_int_t **fl2int_result = new priv_int_t*[fl2int_size];

    for (int comp = 0; comp < 4; comp++) {
        fl2int_float[comp] = new priv_int_t*[fl2int_size];
        for (int i = 0; i < fl2int_size; i++) {
            fl2int_float[comp][i] = new priv_int_t[numShares]();
        }
    }

    for (int i = 0; i < fl2int_size; i++) {
        fl2int_result[i] = new priv_int_t[numShares]();
    }

    fprintf(stderr, "[DEBUG] Converting float values to private format for FL2Int...\n");
    fflush(stderr);

    // Convert each float value to PICCO representation
    // PICCO uses: value = v * 2^p where v is in [2^{L-1}, 2^L)
    // The exponent formula from picco-utility is: p = e_biased - 127 - L + 1
    // This differs from IEEE where value = (1.mantissa) * 2^(e_biased - 127)
    for (int i = 0; i < fl2int_size; i++) {
        float val = fl2int_inputs[i];
        uint32_t bits;
        memcpy(&bits, &val, sizeof(float));
        uint32_t sign = (bits >> 31) & 1;
        int32_t e_biased = (bits >> 23) & 0xFF;  // biased exponent (0-255)
        uint32_t mantissa_raw = bits & 0x7FFFFF;  // 23-bit mantissa without implicit 1

        int32_t exp;
        uint32_t mantissa;
        int zero_flag = 0;

        if (e_biased == 0 && mantissa_raw == 0) {
            // Zero
            zero_flag = 1;
            mantissa = 0;
            exp = 0;
            sign = 0;
        } else if (e_biased == 0) {
            // Denormalized number (not handling for now)
            zero_flag = 1;
            mantissa = 0;
            exp = 0;
        } else {
            // Normalized number
            // PICCO formula: p = e_biased - 127 - L + 1 (same as picco-utility convertFloat)
            exp = e_biased - 127 - fl2int_L + 1;
            // Add implicit leading 1 to get 24-bit mantissa in [2^23, 2^24)
            mantissa = mantissa_raw | 0x800000;
        }

        fprintf(stderr, "[DEBUG] FL2Int Test %d: %.2f -> v=%u, p=%d, z=%d, s=%u\n",
                i, val, mantissa, exp, zero_flag, sign);
        fprintf(stderr, "[DEBUG]   Verification: v * 2^p = %u * 2^%d = %.6f\n",
                mantissa, exp, (double)mantissa * pow(2.0, exp));

        // Use sparsify_public to properly embed public test values into shares
        // This correctly handles the RSS share structure based on party ID
        ss->sparsify_public(fl2int_float[0][i], (priv_int_t)mantissa);  // v: mantissa
        ss->sparsify_public(fl2int_float[1][i], (priv_int_t)exp);       // p: exponent (can be negative)
        ss->sparsify_public(fl2int_float[2][i], zero_flag);             // z: zero flag
        ss->sparsify_public(fl2int_float[3][i], (priv_int_t)sign);      // s: sign
    }

    fprintf(stderr, "\n[DEBUG] ========== CALLING FL2Int ==========\n");
    fprintf(stderr, "[DEBUG] Calling doOperation_FL2Int with L=%d, K=%d, gamma=%d, size=%d\n",
            fl2int_L, fl2int_K, fl2int_gamma, fl2int_size);
    fflush(stderr);


    printf("Executing FL2Int (L=%d, K=%d, gamma=%d)...\n", fl2int_L, fl2int_K, fl2int_gamma);
    fflush(stdout);

    doOperation_FL2Int(fl2int_float, fl2int_result, fl2int_L, fl2int_K, fl2int_gamma,
                       fl2int_size, threadID, net, ss);

    fprintf(stderr, "[DEBUG] doOperation_FL2Int completed\n");
    fflush(stderr);
    printf("FL2Int execution completed.\n\n");
    fflush(stdout);

    // Open results
    priv_int_t *fl2int_output = new priv_int_t[fl2int_size];
    Open(fl2int_output, fl2int_result, fl2int_size, -1, net, ss);

    fprintf(stderr, "\n[DEBUG] ========== FL2INT RESULTS ==========\n");
    printf("\n===== RESULTS =====\n");

    int fl2int_passed = 0;
    for (int i = 0; i < fl2int_size; i++) {
        int64_t actual = (int64_t)fl2int_output[i];

        // Handle signed interpretation for 64-bit ring
        if (ring_size == 64 && actual > ((int64_t)1 << 62)) {
            actual = actual - ((int64_t)1 << 63) - ((int64_t)1 << 63);  // Two's complement
        }

        int64_t expected_val = fl2int_expected[i];
        bool test_passed = (actual == expected_val);

        fprintf(stderr, "[DEBUG] FL2Int Test %d: FL2Int(%.2f) = %ld (expected %ld) - %s\n",
                i, fl2int_inputs[i], actual, expected_val, test_passed ? "PASSED" : "FAILED");

        printf("Test %d: FL2Int(%.2f) = %ld (expected %ld) - %s\n",
               i, fl2int_inputs[i], actual, expected_val, test_passed ? "PASSED" : "FAILED");

        if (test_passed) fl2int_passed++;
    }

    fprintf(stderr, "[DEBUG] ========================================\n\n");
    printf("\nFL2Int: %d/%d tests passed\n", fl2int_passed, fl2int_size);
    fflush(stdout);

    // Cleanup
    for (int comp = 0; comp < 4; comp++) {
        for (int i = 0; i < fl2int_size; i++) {
            delete[] fl2int_float[comp][i];
        }
        delete[] fl2int_float[comp];
    }
    delete[] fl2int_float;

    for (int i = 0; i < fl2int_size; i++) {
        delete[] fl2int_result[i];
    }
    delete[] fl2int_result;
    delete[] fl2int_output;

    printf("\n===== FL2Int TEST COMPLETE =====\n");
    fflush(stdout);

    // ============= FLMULT MODULUS REQUIREMENTS TEST =============
    printf("\n===== FLMULT MODULUS REQUIREMENTS TEST =====\n");
    printf("Testing FLMult ring size requirements (2q+3 formula)\n\n");
    fflush(stdout);

    // Modulus requirement for FLMult: ring_size >= 2*K + 1 (RNTE constraint)
    // The formula "2q+3" comes from:
    // - 2q for the product of two q-bit mantissas
    // - +2 for the implicit bits (one per operand)
    // - +1 for RNTE headroom
    // So for K=q+1 (mantissa with implicit bit): ring_size >= 2K + 1

    printf("Ring size requirements for FLMult:\n");
    printf("  K (mantissa bits)  Required ring_size  Current ring (%u bits)\n", ring_size);
    printf("  ---------------------------------------------------------------\n");

    int test_K_values[] = {24, 28, 31, 32, 33, 52};
    int num_K_values = 6;

    for (int k = 0; k < num_K_values; k++) {
        int K = test_K_values[k];
        uint required = 2 * K + 1;  // RNTE requires input < ring_size
        const char* status = (ring_size >= required) ? "SUPPORTED" : "NEED LARGER RING";
        printf("  K = %-3d            >= %-3u bits           %s\n", K, required, status);
    }
    printf("\n");

    // Test with K that works for current ring_size
    // Find maximum K that works: 2K + 1 <= ring_size => K <= (ring_size - 1) / 2
    int max_K = (ring_size - 1) / 2;
    printf("Maximum supported K for %u-bit ring: %d\n\n", ring_size, max_K);

    // User-requested test: mantissa=32, exp=9, K=33
    // This requires ring_size >= 2*33 + 1 = 67 bits
    int user_K = 33;
    uint user_required = 2 * user_K + 1;

    printf("User-requested test: mantissa=32, exponent=9, K=%d\n", user_K);
    printf("  Required ring_size: >= %u bits\n", user_required);
    printf("  Current ring_size:  %u bits\n", ring_size);

    if (ring_size >= user_required) {
        printf("  Status: SUPPORTED - Running test...\n\n");

        // Run FLMult test with user parameters
        int flmult_K = user_K;
        int flmult_size = 3;

        // Test values: 1.5 * 2.0 = 3.0, 2.5 * 1.5 = 3.75, 3.0 * 0.5 = 1.5
        float flmult_a[] = {1.5f, 2.5f, 3.0f};
        float flmult_b[] = {2.0f, 1.5f, 0.5f};
        float flmult_expected[] = {3.0f, 3.75f, 1.5f};

        printf("Test cases:\n");
        for (int i = 0; i < flmult_size; i++) {
            printf("  Test %d: %.2f * %.2f = %.2f (expected)\n", i, flmult_a[i], flmult_b[i], flmult_expected[i]);
        }
        printf("\n");
        fflush(stdout);

        // Allocate float arrays: [component][size][numShares]
        priv_int_t ***flmult_a_arr = new priv_int_t**[4];
        priv_int_t ***flmult_b_arr = new priv_int_t**[4];
        priv_int_t ***flmult_result = new priv_int_t**[4];

        for (int comp = 0; comp < 4; comp++) {
            flmult_a_arr[comp] = new priv_int_t*[flmult_size];
            flmult_b_arr[comp] = new priv_int_t*[flmult_size];
            flmult_result[comp] = new priv_int_t*[flmult_size];
            for (int i = 0; i < flmult_size; i++) {
                flmult_a_arr[comp][i] = new priv_int_t[numShares]();
                flmult_b_arr[comp][i] = new priv_int_t[numShares]();
                flmult_result[comp][i] = new priv_int_t[numShares]();
            }
        }

        // Convert each float value to custom format components
        // For K=33, we use 32 stored bits + 1 implicit bit
        for (int i = 0; i < flmult_size; i++) {
            // Decompose flmult_a[i]
            uint32_t a_bits;
            memcpy(&a_bits, &flmult_a[i], sizeof(float));
            uint32_t a_sign = (a_bits >> 31) & 1;
            int32_t a_exp = ((a_bits >> 23) & 0xFF) - 127;
            // Scale mantissa to K bits (add implicit 1, shift to K-1 bits)
            uint64_t a_mantissa = ((uint64_t)(a_bits & 0x7FFFFF) | 0x800000) << (flmult_K - 24);

            // Decompose flmult_b[i]
            uint32_t b_bits;
            memcpy(&b_bits, &flmult_b[i], sizeof(float));
            uint32_t b_sign = (b_bits >> 31) & 1;
            int32_t b_exp = ((b_bits >> 23) & 0xFF) - 127;
            uint64_t b_mantissa = ((uint64_t)(b_bits & 0x7FFFFF) | 0x800000) << (flmult_K - 24);

            fprintf(stderr, "[DEBUG] FLMult Test %d: %.2f -> m=%lu e=%d z=0 s=%u\n",
                    i, flmult_a[i], a_mantissa, a_exp, a_sign);
            fprintf(stderr, "[DEBUG] FLMult Test %d: %.2f -> m=%lu e=%d z=0 s=%u\n",
                    i, flmult_b[i], b_mantissa, b_exp, b_sign);

            ss->sparsify_public(flmult_a_arr[0][i], (priv_int_t)a_mantissa);
            ss->sparsify_public(flmult_a_arr[1][i], (int)a_exp);
            ss->sparsify_public(flmult_a_arr[2][i], 0);
            ss->sparsify_public(flmult_a_arr[3][i], (int)a_sign);

            ss->sparsify_public(flmult_b_arr[0][i], (priv_int_t)b_mantissa);
            ss->sparsify_public(flmult_b_arr[1][i], (int)b_exp);
            ss->sparsify_public(flmult_b_arr[2][i], 0);
            ss->sparsify_public(flmult_b_arr[3][i], (int)b_sign);
        }

        printf("Executing FLMult (K=%d)...\n", flmult_K);
        fflush(stdout);

        auto flmult_start = std::chrono::high_resolution_clock::now();
        doOperation_FLMult(flmult_a_arr, flmult_b_arr, flmult_result, flmult_K, flmult_size, threadID, net, ss);
        auto flmult_end = std::chrono::high_resolution_clock::now();
        auto flmult_time = std::chrono::duration_cast<std::chrono::milliseconds>(flmult_end - flmult_start).count();

        printf("FLMult completed in %ld ms\n\n", flmult_time);
        fflush(stdout);

        // Open results
        priv_int_t *flmult_m_output = new priv_int_t[flmult_size];
        priv_int_t *flmult_e_output = new priv_int_t[flmult_size];
        priv_int_t *flmult_z_output = new priv_int_t[flmult_size];
        priv_int_t *flmult_s_output = new priv_int_t[flmult_size];

        Open(flmult_m_output, flmult_result[0], flmult_size, -1, net, ss);
        Open(flmult_e_output, flmult_result[1], flmult_size, -1, net, ss);
        Open(flmult_z_output, flmult_result[2], flmult_size, -1, net, ss);
        Open(flmult_s_output, flmult_result[3], flmult_size, -1, net, ss);

        printf("Results:\n");
        int flmult_passed = 0;
        for (int i = 0; i < flmult_size; i++) {
            uint64_t actual_m = (uint64_t)flmult_m_output[i];
            int64_t actual_e = (int64_t)flmult_e_output[i];
            uint64_t actual_z = (uint64_t)flmult_z_output[i];
            uint64_t actual_s = (uint64_t)flmult_s_output[i];

            // Handle signed exponent
            if (ring_size == 64 && actual_e > ((int64_t)1 << 62)) {
                actual_e = actual_e - ((int64_t)1 << 63) - ((int64_t)1 << 63);
            }

            // Convert result back to float for comparison
            double actual_float = 0.0;
            if (actual_z == 0 && actual_m > 0) {
                double mantissa_normalized = (double)actual_m / (1ULL << (flmult_K - 1));
                actual_float = mantissa_normalized * pow(2.0, (double)actual_e);
                if (actual_s != 0) actual_float = -actual_float;
            }

            double expected_float = (double)flmult_expected[i];
            double error_pct = (expected_float != 0) ? fabs((actual_float - expected_float) / expected_float) * 100.0 : 0;
            bool test_passed = (error_pct < 5.0);

            printf("  Test %d: %.2f * %.2f = %.6f (expected %.2f, error %.2f%%) - %s\n",
                   i, flmult_a[i], flmult_b[i], actual_float, flmult_expected[i], error_pct,
                   test_passed ? "PASSED" : "FAILED");

            if (test_passed) flmult_passed++;
        }

        printf("\nFLMult (K=%d): %d/%d tests passed\n", flmult_K, flmult_passed, flmult_size);
        fflush(stdout);

        // Cleanup
        for (int comp = 0; comp < 4; comp++) {
            for (int i = 0; i < flmult_size; i++) {
                delete[] flmult_a_arr[comp][i];
                delete[] flmult_b_arr[comp][i];
                delete[] flmult_result[comp][i];
            }
            delete[] flmult_a_arr[comp];
            delete[] flmult_b_arr[comp];
            delete[] flmult_result[comp];
        }
        delete[] flmult_a_arr;
        delete[] flmult_b_arr;
        delete[] flmult_result;
        delete[] flmult_m_output;
        delete[] flmult_e_output;
        delete[] flmult_z_output;
        delete[] flmult_s_output;
    } else {
        printf("  Status: SKIPPED - Ring size too small\n");
        printf("  Note: User-requested K=%d requires %u-bit ring (128-bit not yet implemented)\n\n", user_K, user_required);
    }

    // Run FLMult with a K value that works for current ring_size
    if (max_K >= 24) {
        int working_K = 24;  // Use IEEE single precision mantissa
        printf("\nFLMult test with K=%d (compatible with %u-bit ring):\n", working_K, ring_size);
        fflush(stdout);

        int flmult_size = 3;
        float flmult_a[] = {1.5f, 2.5f, 3.0f};
        float flmult_b[] = {2.0f, 1.5f, 0.5f};
        float flmult_expected[] = {3.0f, 3.75f, 1.5f};

        priv_int_t ***flmult_a_arr = new priv_int_t**[4];
        priv_int_t ***flmult_b_arr = new priv_int_t**[4];
        priv_int_t ***flmult_result = new priv_int_t**[4];

        for (int comp = 0; comp < 4; comp++) {
            flmult_a_arr[comp] = new priv_int_t*[flmult_size];
            flmult_b_arr[comp] = new priv_int_t*[flmult_size];
            flmult_result[comp] = new priv_int_t*[flmult_size];
            for (int i = 0; i < flmult_size; i++) {
                flmult_a_arr[comp][i] = new priv_int_t[numShares]();
                flmult_b_arr[comp][i] = new priv_int_t[numShares]();
                flmult_result[comp][i] = new priv_int_t[numShares]();
            }
        }

        for (int i = 0; i < flmult_size; i++) {
            uint32_t a_bits;
            memcpy(&a_bits, &flmult_a[i], sizeof(float));
            uint32_t a_sign = (a_bits >> 31) & 1;
            int32_t a_exp = ((a_bits >> 23) & 0xFF) - 127;
            uint32_t a_mantissa = (a_bits & 0x7FFFFF) | 0x800000;

            uint32_t b_bits;
            memcpy(&b_bits, &flmult_b[i], sizeof(float));
            uint32_t b_sign = (b_bits >> 31) & 1;
            int32_t b_exp = ((b_bits >> 23) & 0xFF) - 127;
            uint32_t b_mantissa = (b_bits & 0x7FFFFF) | 0x800000;

            ss->sparsify_public(flmult_a_arr[0][i], (int)a_mantissa);
            ss->sparsify_public(flmult_a_arr[1][i], (int)a_exp);
            ss->sparsify_public(flmult_a_arr[2][i], 0);
            ss->sparsify_public(flmult_a_arr[3][i], (int)a_sign);

            ss->sparsify_public(flmult_b_arr[0][i], (int)b_mantissa);
            ss->sparsify_public(flmult_b_arr[1][i], (int)b_exp);
            ss->sparsify_public(flmult_b_arr[2][i], 0);
            ss->sparsify_public(flmult_b_arr[3][i], (int)b_sign);
        }

        printf("Executing FLMult (K=%d)...\n", working_K);
        fflush(stdout);

        doOperation_FLMult(flmult_a_arr, flmult_b_arr, flmult_result, working_K, flmult_size, threadID, net, ss);

        printf("FLMult completed.\n");

        priv_int_t *flmult_m_output = new priv_int_t[flmult_size];
        priv_int_t *flmult_e_output = new priv_int_t[flmult_size];
        priv_int_t *flmult_z_output = new priv_int_t[flmult_size];
        priv_int_t *flmult_s_output = new priv_int_t[flmult_size];

        Open(flmult_m_output, flmult_result[0], flmult_size, -1, net, ss);
        Open(flmult_e_output, flmult_result[1], flmult_size, -1, net, ss);
        Open(flmult_z_output, flmult_result[2], flmult_size, -1, net, ss);
        Open(flmult_s_output, flmult_result[3], flmult_size, -1, net, ss);

        printf("\nResults:\n");
        int flmult_passed = 0;
        for (int i = 0; i < flmult_size; i++) {
            uint64_t actual_m = (uint64_t)flmult_m_output[i];
            int64_t actual_e = (int64_t)flmult_e_output[i];
            uint64_t actual_z = (uint64_t)flmult_z_output[i];
            uint64_t actual_s = (uint64_t)flmult_s_output[i];

            if (ring_size == 64 && actual_e > ((int64_t)1 << 62)) {
                actual_e = actual_e - ((int64_t)1 << 63) - ((int64_t)1 << 63);
            }

            double actual_float = 0.0;
            if (actual_z == 0 && actual_m > 0) {
                double mantissa_normalized = (double)actual_m / (1ULL << (working_K - 1));
                actual_float = mantissa_normalized * pow(2.0, (double)actual_e);
                if (actual_s != 0) actual_float = -actual_float;
            }

            double expected_float = (double)flmult_expected[i];
            double error_pct = (expected_float != 0) ? fabs((actual_float - expected_float) / expected_float) * 100.0 : 0;
            bool test_passed = (error_pct < 5.0);

            printf("  Test %d: %.2f * %.2f = %.6f (expected %.2f, error %.2f%%) - %s\n",
                   i, flmult_a[i], flmult_b[i], actual_float, flmult_expected[i], error_pct,
                   test_passed ? "PASSED" : "FAILED");

            if (test_passed) flmult_passed++;
        }

        printf("\nFLMult (K=%d): %d/%d tests passed\n", working_K, flmult_passed, flmult_size);
        fflush(stdout);

        for (int comp = 0; comp < 4; comp++) {
            for (int i = 0; i < flmult_size; i++) {
                delete[] flmult_a_arr[comp][i];
                delete[] flmult_b_arr[comp][i];
                delete[] flmult_result[comp][i];
            }
            delete[] flmult_a_arr[comp];
            delete[] flmult_b_arr[comp];
            delete[] flmult_result[comp];
        }
        delete[] flmult_a_arr;
        delete[] flmult_b_arr;
        delete[] flmult_result;
        delete[] flmult_m_output;
        delete[] flmult_e_output;
        delete[] flmult_z_output;
        delete[] flmult_s_output;
    }

    printf("\n===== FLMULT MODULUS TEST COMPLETE =====\n");
    fflush(stdout);

    // ============= INTEGER TO FLOAT (Int2FL) TEST =============
    printf("\n===== INTEGER TO FLOAT (Int2FL) TEST =====\n");
    fflush(stdout);

    // Test cases: integers to convert to floats
    int int2fl_gamma = 32;  // input integer bits
    int int2fl_K = 24;      // mantissa bits
    int int2fl_size = 5;

    int64_t int2fl_inputs[] = {42, 100, -25, 0, 7};
    float int2fl_expected[] = {42.0f, 100.0f, -25.0f, 0.0f, 7.0f};

    printf("Test cases:\n");
    for (int i = 0; i < int2fl_size; i++) {
        printf("  Test %d: Int2FL(%ld) = %.2f (expected)\n", i, int2fl_inputs[i], int2fl_expected[i]);
    }
    printf("\n");
    fflush(stdout);

    // Allocate integer input: [size][numShares]
    priv_int_t **int2fl_input = new priv_int_t*[int2fl_size];
    // Allocate float output: [component][size][numShares]
    priv_int_t ***int2fl_result = new priv_int_t**[4];

    for (int i = 0; i < int2fl_size; i++) {
        int2fl_input[i] = new priv_int_t[numShares]();
    }

    for (int comp = 0; comp < 4; comp++) {
        int2fl_result[comp] = new priv_int_t*[int2fl_size];
        for (int i = 0; i < int2fl_size; i++) {
            int2fl_result[comp][i] = new priv_int_t[numShares]();
        }
    }

    fprintf(stderr, "[DEBUG] Setting up integer inputs for Int2FL...\n");
    fflush(stderr);

    // Set up integer inputs using sparsify_public
    for (int i = 0; i < int2fl_size; i++) {
        ss->sparsify_public(int2fl_input[i], (int)int2fl_inputs[i]);
        fprintf(stderr, "[DEBUG] Int2FL input %d: %ld\n", i, int2fl_inputs[i]);
    }

    fprintf(stderr, "\n[DEBUG] ========== CALLING Int2FL ==========\n");
    fprintf(stderr, "[DEBUG] Calling doOperation_Int2FL with gamma=%d, K=%d, size=%d\n",
            int2fl_gamma, int2fl_K, int2fl_size);
    fflush(stderr);

    printf("Executing Int2FL (gamma=%d, K=%d)...\n", int2fl_gamma, int2fl_K);
    fflush(stdout);

    doOperation_Int2FL(int2fl_input, int2fl_result, int2fl_gamma, int2fl_K,
                       int2fl_size, threadID, net, ss);

    fprintf(stderr, "[DEBUG] doOperation_Int2FL completed\n");
    fflush(stderr);
    printf("Int2FL execution completed.\n\n");
    fflush(stdout);

    // Open results
    priv_int_t *int2fl_m_output = new priv_int_t[int2fl_size];
    priv_int_t *int2fl_e_output = new priv_int_t[int2fl_size];
    priv_int_t *int2fl_z_output = new priv_int_t[int2fl_size];
    priv_int_t *int2fl_s_output = new priv_int_t[int2fl_size];

    Open(int2fl_m_output, int2fl_result[0], int2fl_size, -1, net, ss);
    Open(int2fl_e_output, int2fl_result[1], int2fl_size, -1, net, ss);
    Open(int2fl_z_output, int2fl_result[2], int2fl_size, -1, net, ss);
    Open(int2fl_s_output, int2fl_result[3], int2fl_size, -1, net, ss);

    fprintf(stderr, "\n[DEBUG] ========== INT2FL RESULTS ==========\n");
    printf("\n===== RESULTS =====\n");

    int int2fl_passed = 0;
    for (int i = 0; i < int2fl_size; i++) {
        uint64_t actual_m = (uint64_t)int2fl_m_output[i];
        int64_t actual_e = (int64_t)int2fl_e_output[i];
        uint64_t actual_z = (uint64_t)int2fl_z_output[i];
        uint64_t actual_s = (uint64_t)int2fl_s_output[i];

        // Handle signed exponent
        if (ring_size == 64 && actual_e > ((int64_t)1 << 62)) {
            actual_e = actual_e - ((int64_t)1 << 63) - ((int64_t)1 << 63);
        }

        // Convert result back to float for comparison
        // PICCO representation: value = v * 2^p (NOT v / 2^{K-1} * 2^p)
        // where v is in [2^{K-1}, 2^K) and p is computed by Int2FL as (Σb_i - K)(1-z)
        double actual_float = 0.0;
        if (actual_z == 0 && actual_m > 0) {
            actual_float = (double)actual_m * pow(2.0, (double)actual_e);
            if (actual_s != 0) actual_float = -actual_float;
        }

        double expected_float = (double)int2fl_expected[i];
        double error_pct = (expected_float != 0) ? fabs((actual_float - expected_float) / expected_float) * 100.0 : (actual_float == 0 ? 0 : 100);
        bool test_passed = (error_pct < 1.0);  // Allow 1% error (should be exact for integers)

        fprintf(stderr, "[DEBUG] Int2FL Test %d: Int2FL(%ld) = %.6f (expected %.2f)\n",
                i, int2fl_inputs[i], actual_float, int2fl_expected[i]);
        fprintf(stderr, "[DEBUG]   Raw result: m=%lu e=%ld z=%lu s=%lu\n",
                actual_m, actual_e, actual_z, actual_s);
        fprintf(stderr, "[DEBUG]   Error: %.2f%% - %s\n\n",
                error_pct, test_passed ? "PASSED" : "FAILED");

        printf("Test %d: Int2FL(%ld) = %.6f (expected %.2f, error %.2f%%) - %s\n",
               i, int2fl_inputs[i], actual_float, int2fl_expected[i], error_pct,
               test_passed ? "PASSED" : "FAILED");

        if (test_passed) int2fl_passed++;
    }

    fprintf(stderr, "[DEBUG] ========================================\n\n");
    printf("\nInt2FL: %d/%d tests passed\n", int2fl_passed, int2fl_size);
    fflush(stdout);

    // Cleanup
    for (int i = 0; i < int2fl_size; i++) {
        delete[] int2fl_input[i];
    }
    delete[] int2fl_input;

    for (int comp = 0; comp < 4; comp++) {
        for (int i = 0; i < int2fl_size; i++) {
            delete[] int2fl_result[comp][i];
        }
        delete[] int2fl_result[comp];
    }
    delete[] int2fl_result;

    delete[] int2fl_m_output;
    delete[] int2fl_e_output;
    delete[] int2fl_z_output;
    delete[] int2fl_s_output;

    printf("\n===== Int2FL TEST COMPLETE =====\n");
    fflush(stdout);
}
#endif // __RSS__
