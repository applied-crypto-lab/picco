#include "bitAddTrunc.h"

void Rss_nBitAdd_trunc(Lint **res, Lint **carry, Lint **r_bitwise, uint ring_size, uint m, uint size, NodeNetwork *nodeNet) {

    uint i, j;
    uint numShares = nodeNet->getNumShares();
    uint threshold = nodeNet->getThreshold();
    uint rounds = ceil(log2(threshold + 1));

    Lint **a = new Lint *[numShares];

    Lint **b = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        a[i] = new Lint[size];
        memset(a[i], 0, sizeof(Lint) * size);

        b[i] = new Lint[size];
        memset(b[i], 0, sizeof(Lint) * size);
    }

    // // always will be 2 rounds for 3pc
    for (j = 0; j < rounds; j++) {

        // if this is the first iteration, we copy r_bitwise into a and b
        if (j == 0) {
            // copy p_1 and p_2 into a, b respectively

            // memcpy(a[0], r_bitwise[0], size * sizeof(Lint));
            // memcpy(a[1], r_bitwise[1], size * sizeof(Lint));

            // memcpy(b[0], r_bitwise[0] + size, size * sizeof(Lint));
            // memcpy(b[1], r_bitwise[1] + size, size * sizeof(Lint));
            for (size_t s = 0; s < numShares; s++) {
                memcpy(a[s], r_bitwise[s], size * sizeof(Lint));
                memcpy(b[s], r_bitwise[s] + size, size * sizeof(Lint));
            }
            Rss_BitAdd_trunc(res, carry, a, b, ring_size, m, size, nodeNet);
        }
    }

    for (i = 0; i < numShares; i++) {
        delete[] a[i];
        delete[] b[i];
    }
    delete[] a;
    delete[] b;
}

void Rss_BitAdd_trunc(Lint **res, Lint **carry, Lint **a, Lint **b, uint ring_size, uint m, uint size, NodeNetwork *nodeNet) {

    Lint i;
    // int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();

    Lint **d = new Lint *[2 * numShares];
    for (i = 0; i < 2 * numShares; i++) {
        d[i] = new Lint[size];
        memset(d[i], 0, sizeof(Lint) * size);
    }
    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;

    // gettimeofday(&start, NULL); //start timer here
    Rss_Mult_Bitwise(res, a, b, size, ring_size, nodeNet);
    // gettimeofday(&end, NULL); //stop timer here
    // timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    // printf("Runtime for Rss_Mult_Bitwise with data size %d = %.6lf ms\n", size, (double)(timer * 0.001));

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            d[s][i] = a[s][i] ^ b[s][i];
            // d[1][i] = a[1][i] ^ b[1][i];

            d[numShares + s][i] = res[s][i];
            // d[3][i] = res[1][i];
        };
    }

    // Rss_CircleOpL(d, ring_size, size, nodeNet);

    // gettimeofday(&start, NULL); //start timer here

    // Rss_CircleOpL_Lint(d, ring_size, size, nodeNet); // new version w Lints
    Rss_CircleOpL(d, ring_size, size, nodeNet); // original

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            res[s][i] = (a[s][i] ^ b[s][i]) ^ (d[numShares + s][i] << 1);
            carry[s][i] = GET_BIT(d[numShares + s][i], Lint(m - 1));
            carry[s][size + i] = GET_BIT(d[numShares + s][i], Lint(ring_size - 1));
            carry[s][2 * size + i] = GET_BIT(res[s][i], Lint(ring_size - 1));
        }

        // res[0][i] = (a[0][i] ^ b[0][i]) ^ (d[2][i] << 1);
        // res[1][i] = (a[1][i] ^ b[1][i]) ^ (d[3][i] << 1);

        // // m-1th carry bit
        // carry[0][i] = GET_BIT(d[2][i], Lint(m - 1));
        // carry[1][i] = GET_BIT(d[3][i], Lint(m - 1));

        // // kth carry bit
        // carry[0][size + i] = GET_BIT(d[2][i], Lint(ring_size - 1));
        // carry[1][size + i] = GET_BIT(d[3][i], Lint(ring_size - 1));

        // // getting MSB of res = a + b
        // carry[0][2 * size + i] = GET_BIT(res[0][i], Lint(ring_size - 1));
        // carry[1][2 * size + i] = GET_BIT(res[1][i], Lint(ring_size - 1));
    }

    for (i = 0; i < 2 * numShares; i++) {
        delete[] d[i];
    }
    delete[] d;
}
void Rss_nBitAdd_trunc_5pc(Lint **res, Lint **carry, Lint **r_bitwise, uint ring_size, uint m, uint size, NodeNetwork *nodeNet) {

    uint i, j;
    uint numShares = nodeNet->getNumShares();
    uint threshold = nodeNet->getThreshold();
    uint rounds = ceil(log2(threshold + 1));

    Lint **a = new Lint *[numShares];

    Lint **b = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        a[i] = new Lint[size];
        memset(a[i], 0, sizeof(Lint) * size);

        b[i] = new Lint[size];
        memset(b[i], 0, sizeof(Lint) * size);
    }

    for (j = 0; j < rounds; j++) {

        // if this is the first iteration, we copy r_bitwise into a and b
        if (j == 0) {
            // copy p_1 and p_2 into a, b respectively

            for (size_t s = 0; s < numShares; s++) {
                memcpy(a[s], r_bitwise[s], size * sizeof(Lint));
                memcpy(b[s], r_bitwise[s] + size, size * sizeof(Lint));
            }
            Rss_BitAdd_trunc_5pc(res, carry, a, b, ring_size, size, m, nodeNet);
        } else {
            // we only need to copy r_bitwise into b
            for (size_t s = 0; s < numShares; s++) {
                memcpy(a[s], res[s], size * sizeof(Lint));
                memcpy(b[s], r_bitwise[s] + 2 * size, size * sizeof(Lint));
            }
            Rss_BitAdd_trunc_5pc(res, carry, a, b, ring_size, size, m, nodeNet);
        }
    }

    for (i = 0; i < numShares; i++) {
        delete[] a[i];
        delete[] b[i];
    }
    delete[] a;
    delete[] b;
}

void Rss_BitAdd_trunc_5pc(Lint **res, Lint **carry, Lint **a, Lint **b, uint ring_size, uint m, uint size, NodeNetwork *nodeNet) {

    Lint i;
    // int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();

    Lint **d = new Lint *[2 * numShares];
    for (i = 0; i < 2 * numShares; i++) {
        d[i] = new Lint[size];
        memset(d[i], 0, sizeof(Lint) * size);
    }
    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;

    // gettimeofday(&start, NULL); //start timer here
    Rss_Mult_Bitwise_5pc(res, a, b, size, ring_size, nodeNet);
    // gettimeofday(&end, NULL); //stop timer here
    // timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    // printf("Runtime for Rss_Mult_Bitwise with data size %d = %.6lf ms\n", size, (double)(timer * 0.001));

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            d[s][i] = a[s][i] ^ b[s][i];
            // d[1][i] = a[1][i] ^ b[1][i];

            d[numShares + s][i] = res[s][i];
            // d[3][i] = res[1][i];
        };
    }

    // Rss_CircleOpL(d, ring_size, size, map, nodeNet);

    // gettimeofday(&start, NULL); //start timer here

    // Rss_CircleOpL_Lint(d, ring_size, size, map, nodeNet); // new version w Lints
    Rss_CircleOpL_mp_test(d, ring_size, size, nodeNet); // original

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            res[s][i] = (a[s][i] ^ b[s][i]) ^ (d[numShares + s][i] << 1);
            carry[s][i] = GET_BIT(d[numShares + s][i], Lint(m - 1));
            carry[s][size + i] = GET_BIT(d[numShares + s][i], Lint(ring_size - 1));
            
            carry[s][2 * size + i] = GET_BIT(res[s][i], Lint(ring_size - 1));
        }

        // res[0][i] = (a[0][i] ^ b[0][i]) ^ (d[2][i] << 1);
        // res[1][i] = (a[1][i] ^ b[1][i]) ^ (d[3][i] << 1);

        // // m-1th carry bit
        // carry[0][i] = GET_BIT(d[2][i], Lint(m - 1));
        // carry[1][i] = GET_BIT(d[3][i], Lint(m - 1));

        // // k-1th carry bit
        // carry[0][size + i] = GET_BIT(d[2][i], Lint(ring_size - 1));
        // carry[1][size + i] = GET_BIT(d[3][i], Lint(ring_size - 1));

        // // getting MSB of res = a + b
        // carry[0][2 * size + i] = GET_BIT(res[0][i], Lint(ring_size - 1));
        // carry[1][2 * size + i] = GET_BIT(res[1][i], Lint(ring_size - 1));
    }

    for (i = 0; i < 2 * numShares; i++) {
        delete[] d[i];
    }
    delete[] d;
}