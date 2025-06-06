#include "FLLTZ.h"
#include <stdio.h>

int main() {
    // Example test values
    float a[3] = {1.45f, -2.0f, -1e-6f};
    float b[3] = {1.1f, 2.0f, -2e-6f};

    int size = 3, K = 32, L = 8;
    // (Adapt if you want to test all 14 pairs.)

    // Allocate A1 and B1: arrays of [size][4]
    float* A1 = a;
    float* B1 = b;
    // Output: secret-shared result, usually mpz_t *result
    mpz_t* result = (mpz_t*)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; ++i) mpz_init(result[i]);

    // Assuming NodeNetwork net, SecretShare *ss are initialized by your SMC runtime
    int threadID = 0;
    NodeNetwork net;
    SecretShare *ss = GET_SS(); // however your repo provides this

    // Run FLLTZ
    doOperation_FLLTZ(A1, B1, result, K, L, size, threadID, net, ss);

    // Reveal and print result
    for (int i = 0; i < size; ++i) {
        gmp_printf("Result for %.8f < %.8f: %Zd\n", a[i], b[i], result[i]);
    }
    for (int i = 0; i < size; ++i) mpz_clear(result[i]);
    free(result);
    return 0;
}
