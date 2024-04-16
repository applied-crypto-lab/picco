#include "MSB.h"

void new_Rss_MSB(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet) {
    int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();
    uint i; // used for loops

    // only need to generate a single random bit per private value
    Lint **b = new Lint *[numShares];
    Lint **sum = new Lint *[numShares];
    Lint **u_2 = new Lint *[numShares];
    Lint **edaBit_r = new Lint *[numShares];
    Lint **edaBit_b_2 = new Lint *[numShares];
    Lint **rprime = new Lint *[numShares];

    Lint *c = new Lint[size];
    Lint *e = new Lint[size];

    // used for testing correctness
    // Lint *res_check = new Lint[size];
    // Lint *r_2_open = new Lint[size];
    // Lint *u_2_open = new Lint[size];

    for (i = 0; i < numShares; i++) {
        b[i] = new Lint[size];
        edaBit_r[i] = new Lint[size];
        edaBit_b_2[i] = new Lint[size];
        sum[i] = new Lint[size];
        u_2[i] = new Lint[size];

        rprime[i] = new Lint[size];
    }

    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
    if (pid == 1) {
        ai[0] = 1; // party 1's share 1
    } else if (pid == 3) {
        ai[1] = 1; // party 3's share 2
    }

    // stays the same
    Rss_RandBit(b, size, ring_size, nodeNet);

    // edaBit(edaBit_r, edaBit_b_2, size, ring_size, ring_size - 1, nodeNet);
    // need to generate full edabit for final implementation
    edaBit(edaBit_r, edaBit_b_2, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            rprime[s][i] = edaBit_r[s][i] - (GET_BIT(edaBit_b_2[s][i], Lint(ring_size - 1)) << Lint(ring_size - 1));
            // rprime[1][i] = edaBit_r[1][i] - (GET_BIT(edaBit_b_2[1][i], Lint(ring_size - 1)) << Lint(ring_size - 1));
            // combining w the next loop
            // combining w the previous loop
            sum[s][i] = (a[s][i] + edaBit_r[s][i]);
            // sum[1][i] = (a[1][i] + edaBit_r[1][i]);
            // sum[0][i] = (a[0][i] + edaBit_r[0][i]) << Lint(1);
            // sum[1][i] = (a[1][i] + edaBit_r[1][i]) << Lint(1);
        }
    }

    Rss_Open(c, sum, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            edaBit_b_2[s][i] = edaBit_b_2[s][i] & nodeNet->SHIFT[ring_size - 1];

        // used for alternate solution
        // prevents us from using it directly later on
        // edaBit_b_2[1][i] = edaBit_b_2[1][i] & nodeNet->SHIFT[ring_size - 1];

        // definitely needed
        c[i] = c[i] & nodeNet->SHIFT[ring_size - 1];
        // c[i] = c[i] >> Lint(1);
    }

    // Rss_Open_Bitwise(r_2_open, edaBit_b_2, size, ring_size, nodeNet);
    // // this part is still correct
    // however, the edaBit_b_2 shares do get modified
    // which may not be desierable
    Rss_BitLT(u_2, c, edaBit_b_2, ring_size, size, nodeNet);

    for (i = 0; i < size; ++i) {
        // cant do this because we modify edaBit_b_2 earlier
        // for (size_t s = 0; s < numShares; s++)
        for (size_t s = 0; s < numShares; s++)
            sum[s][i] = a[s][i] - c[i] * ai[s] + rprime[s][i] - (u_2[s][i] << Lint(ring_size - 1)) + (b[s][i] << Lint(ring_size - 1));
        // sum[1][i] = a[1][i] - c[i] * a2 + rprime[1][i] - (u_2[1][i] << Lint(ring_size - 1)) + (b[1][i] << Lint(ring_size - 1));
    }
    // opening sum
    Rss_Open(e, sum, size, ring_size, nodeNet);

    Lint e_bit;
    for (i = 0; i < size; ++i) {
        e_bit = GET_BIT(e[i], ring_size - 1); // getting the (k-1)th bit
        for (size_t s = 0; s < numShares; s++) {
            res[s][i] = e_bit * ai[s] + b[s][i] - (e_bit * b[s][i] << Lint(1));
        }
        // res[1][i] = e_bit * a2 + b[1][i] - (e_bit * b[1][i] << Lint(1));
    }

    // cleanup
    delete[] ai;
    delete[] c;
    delete[] e;
    // delete[] res_check;
    // delete[] r_2_open;
    // delete[] u_2_open;

    for (i = 0; i < numShares; i++) {
        delete[] edaBit_r[i];
        delete[] edaBit_b_2[i];
        delete[] b[i];
        delete[] sum[i];
        delete[] u_2[i];
        delete[] rprime[i];
    }
    delete[] edaBit_r;
    delete[] edaBit_b_2;
    delete[] b;
    delete[] sum;
    delete[] u_2;
    delete[] rprime;
}

void new_Rss_MSB_time(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet, unsigned long &timer) {

    struct timeval start;
    struct timeval end;
    unsigned long op_timer = 0;
    uint numShares = nodeNet->getNumShares();

    int pid = nodeNet->getID();
    uint i; // used for loops

    // only need to generate a single random bit per private value
    Lint **b = new Lint *[numShares];
    Lint **sum = new Lint *[numShares];
    Lint **u_2 = new Lint *[numShares];
    Lint **edaBit_r = new Lint *[numShares];
    Lint **edaBit_b_2 = new Lint *[numShares];
    Lint **rprime = new Lint *[numShares];

    Lint *c = new Lint[size];
    Lint *e = new Lint[size];

    // used for testing correctness
    Lint *res_check = new Lint[size];
    Lint *r_2_open = new Lint[size];
    Lint *u_2_open = new Lint[size];

    for (i = 0; i < numShares; i++) {
        b[i] = new Lint[size];
        edaBit_r[i] = new Lint[size];
        edaBit_b_2[i] = new Lint[size];
        sum[i] = new Lint[size];
        u_2[i] = new Lint[size];

        rprime[i] = new Lint[size];
    }

    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
    // if (pid == 1) {
    //     ai[0] = 1;
    // } else if (pid == numShares) {
    //     ai[numShares - 1] = 1;
    // }

    if (pid == 1) {
        ai[0] = 1; // party 1's share 1
    } else if (pid == 3) {
        ai[1] = 1; // party 3's share 2
    }
    // Lint a1 = 0;
    // Lint a2 = 0;
    // switch (pid) {
    // case 1:
    //     a1 = 1;
    //     a2 = 0;
    //     break;
    // case 2:
    //     a1 = 0;
    //     a2 = 0;
    //     break;
    // case 3:
    //     a1 = 0;
    //     a2 = 1;
    //     break;
    // }

    gettimeofday(&start, NULL); // start timer here
    // stays the same
    Rss_RandBit(b, size, ring_size, nodeNet);
    gettimeofday(&end, NULL); // stop timer here
    op_timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    timer += 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    // printf("----> rb time = %.6lf\n", (double)(op_timer * 1e-6));
    // edaBit(edaBit_r, edaBit_b_2, size, ring_size, ring_size - 1, nodeNet);
    // need to generate full edabit for final implementation
    gettimeofday(&start, NULL); // start timer here

    edaBit(edaBit_r, edaBit_b_2, size, ring_size, nodeNet);
    gettimeofday(&end, NULL); // stop timer here
    op_timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    timer += 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;

    // printf("----> edaBit MSB time = %.6lf\n", (double)(op_timer * 1e-6));

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            rprime[s][i] = edaBit_r[s][i] - (GET_BIT(edaBit_b_2[s][i], Lint(ring_size - 1)) << Lint(ring_size - 1));
            sum[s][i] = (a[s][i] + edaBit_r[s][i]);
        }
        // rprime[1][i] = edaBit_r[1][i] - (GET_BIT(edaBit_b_2[1][i], Lint(ring_size - 1)) << Lint(ring_size - 1));
        // combining w the next loop
        // combining w the previous loop
        // sum[1][i] = (a[1][i] + edaBit_r[1][i]);
        // sum[0][i] = (a[0][i] + edaBit_r[0][i]) << Lint(1);
        // sum[1][i] = (a[1][i] + edaBit_r[1][i]) << Lint(1);
    }

    Rss_Open(c, sum, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            edaBit_b_2[s][i] = edaBit_b_2[s][i] & nodeNet->SHIFT[ring_size - 1];

        // used for alternate solution
        // prevents us from using it directly later on
        // edaBit_b_2[0][i] = edaBit_b_2[0][i] & nodeNet->SHIFT[ring_size - 1];
        // edaBit_b_2[1][i] = edaBit_b_2[1][i] & nodeNet->SHIFT[ring_size - 1];

        // definitely needed
        c[i] = c[i] & nodeNet->SHIFT[ring_size - 1];
        // c[i] = c[i] >> Lint(1);
    }

    // Rss_Open_Bitwise(r_2_open, edaBit_b_2, size, ring_size, nodeNet);
    // // this part is still correct
    // however, the edaBit_b_2 shares do get modified
    // which may not be desierable
    Rss_BitLT(u_2, c, edaBit_b_2, ring_size, size, nodeNet);

    // Rss_Open_Bitwise(u_2_open, u_2, size, ring_size, nodeNet);

    // for (int i = 0; i < size; i++) {

    //     res_check[i] = (c[i] < r_2_open[i]);
    //     if (!(u_2_open[i] == res_check[i])) {
    //         // printf("[%i] c < r_2 : %u   --- expected: %u\n", i, u_2_open[i], res_check[i]);
    //         // printf("c = %u --- edaBit_b_2 = %u\n", c[i], r_2_open[i]);
    //         // printf("BitLT ERROR at %d \n", i);
    //     }
    // }

    for (i = 0; i < size; ++i) {
        // cant do this because we modify edaBit_b_2 earlier

        for (size_t s = 0; s < numShares; s++)
            sum[s][i] = a[s][i] - c[i] * ai[s] + rprime[s][i] - (u_2[s][i] << Lint(ring_size - 1)) + (b[s][i] << Lint(ring_size - 1));
        // sum[1][i] = a[1][i] - c[i] * a2 + rprime[1][i] - (u_2[1][i] << Lint(ring_size - 1)) + (b[1][i] << Lint(ring_size - 1));

        // sum[0][i] = a[0][i] - c[i] * a1 + edaBit_r[0][i] - (u_2[0][i] << Lint(ring_size - 1)) + (b[0][i] << Lint(ring_size - 1));
        // sum[1][i] = a[1][i] - c[i] * a2 + edaBit_r[1][i] - (u_2[1][i] << Lint(ring_size - 1)) + (b[1][i] << Lint(ring_size - 1));
    }
    // opening sum
    Rss_Open(e, sum, size, ring_size, nodeNet);

    Lint e_bit;
    for (i = 0; i < size; ++i) {
        e_bit = GET_BIT(e[i], ring_size - 1); // getting the (k-1)th bit
        for (size_t s = 0; s < numShares; s++)
            res[s][i] = e_bit * ai[s] + b[s][i] - (e_bit * b[s][i] << Lint(1));
        // res[1][i] = e_bit * a2 + b[1][i] - (e_bit * b[1][i] << Lint(1));
    }

    // cleanup
    delete[] ai;
    delete[] c;
    delete[] e;
    delete[] res_check;
    delete[] r_2_open;
    delete[] u_2_open;

    for (i = 0; i < numShares; i++) {
        delete[] edaBit_r[i];
        delete[] edaBit_b_2[i];
        delete[] b[i];
        delete[] sum[i];
        delete[] u_2[i];
        delete[] rprime[i];
    }
    delete[] edaBit_r;
    delete[] edaBit_b_2;
    delete[] b;
    delete[] sum;
    delete[] u_2;
    delete[] rprime;
}

void test_new_Rss_MSB(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet) {
    int pid = nodeNet->getID();
    uint i; // used for loops
    uint numShares = nodeNet->getNumShares();

    // only need to generate a single random bit per private value
    Lint **b = new Lint *[numShares];

    Lint **sum = new Lint *[numShares];
    Lint **sum2 = new Lint *[numShares];

    Lint **u_2 = new Lint *[numShares];
    Lint **u_2_2 = new Lint *[numShares];

    Lint **edaBit_r = new Lint *[numShares];
    Lint **edaBit_b_2 = new Lint *[numShares];
    Lint **edaBit_b_2_2 = new Lint *[numShares];

    Lint **rprime = new Lint *[numShares];
    Lint **r = new Lint *[numShares];

    Lint *c = new Lint[size];
    Lint *c2 = new Lint[size];
    Lint *e = new Lint[size];
    Lint *e2 = new Lint[size];

    // used for testing correctness
    Lint *res_check = new Lint[size];
    Lint *res_check_2 = new Lint[size];
    Lint *r_2_open = new Lint[size];
    Lint *u_2_open = new Lint[size];
    Lint *r_2_open_2 = new Lint[size];
    Lint *u_2_open_2 = new Lint[size];

    for (i = 0; i < numShares; i++) {
        b[i] = new Lint[2 * size];
        edaBit_r[i] = new Lint[size];

        edaBit_b_2[i] = new Lint[size];
        edaBit_b_2_2[i] = new Lint[size];

        sum[i] = new Lint[size];
        sum2[i] = new Lint[size];

        u_2[i] = new Lint[size];
        u_2_2[i] = new Lint[size];

        rprime[i] = new Lint[size];
        r[i] = new Lint[size];
    }

    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
    // if (pid == 1) {
    //     ai[0] = 1;
    // } else if (pid == numShares) {
    //     ai[numShares - 1] = 1;
    // }
    uint numParties = nodeNet->getPeers();
    if (numParties == 3) {
        if (pid == 1) {
            ai[0] = 1; // party 1's share 1
        } else if (pid == 3) {
            ai[1] = 1; // party 3's share 2
        }
    } else if (numParties == 5) {
        if (pid == 1) {
            ai[0] = 1;
        } else if (pid == 4) {
            ai[5] = 1; // party 4's share 6
        } else if (pid == 5) {
            ai[3] = 1; // parthy 5's share 4
        }
    }

    Lint a1 = 0;
    Lint a2 = 0;
    switch (pid) {
    case 1:
        a1 = 1;
        a2 = 0;
        break;
    case 2:
        a1 = 0;
        a2 = 0;
        break;
    case 3:
        a1 = 0;
        a2 = 1;
        break;
    }
    // stays the same
    Rss_RandBit(b, 2 * size, ring_size, nodeNet);

    edaBit(edaBit_r, edaBit_b_2, size, ring_size, ring_size - 1, nodeNet);
    // need to generate full edabit for final implementation
    // edaBit(edaBit_r, edaBit_b_2, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        r[0][i] = edaBit_r[0][i] + (GET_BIT(b[0][i], Lint(ring_size - 1)) << Lint(ring_size - 1));
        r[1][i] = edaBit_r[1][i] + (GET_BIT(b[1][i], Lint(ring_size - 1)) << Lint(ring_size - 1));
        // combining w the next loop
        // }

        // sum = 2(a _ )
        // for (i = 0; i < size; i++) {
        // combining w the previous loop
        // sum[0][i] = (a[0][i] + edaBit_r[0][i]);
        // sum[1][i] = (a[1][i] + edaBit_r[1][i]);
        sum[0][i] = (a[0][i] + edaBit_r[0][i]);
        sum[1][i] = (a[1][i] + edaBit_r[1][i]);

        sum2[0][i] = (a[0][i] + r[0][i]);
        sum2[1][i] = (a[1][i] + r[1][i]);
    }

    Rss_Open(c, sum, size, ring_size, nodeNet);
    Rss_Open(c2, sum2, size, ring_size, nodeNet);

    // c = c/2
    for (i = 0; i < size; i++) {

        // used for alternate solution
        // prevents us from using it directly later on
        // edaBit_b_2_2[0][i] = edaBit_b_2[0][i] & nodeNet->SHIFT[ring_size - 1];
        // edaBit_b_2_2[1][i] = edaBit_b_2[1][i] & nodeNet->SHIFT[ring_size - 1];

        edaBit_b_2_2[0][i] = edaBit_b_2[0][i];
        edaBit_b_2_2[1][i] = edaBit_b_2[1][i];
        // definitely needed
        c[i] = c[i] & nodeNet->SHIFT[ring_size - 1];
        c2[i] = c2[i] & nodeNet->SHIFT[ring_size - 1];

        // c2[i] = c2[i] ;
        // c[i] = c[i] ;
    }

    // printf("hi2\n");
    Rss_Open_Bitwise(r_2_open, edaBit_b_2, size, ring_size, nodeNet);
    Rss_Open_Bitwise(r_2_open_2, edaBit_b_2_2, size, ring_size, nodeNet);
    // // this part is still correct
    // however, the edaBit_b_2 shares do get modified
    // which may not be desierable
    Rss_BitLT(u_2, c, edaBit_b_2, ring_size, size, nodeNet);
    Rss_BitLT(u_2_2, c2, edaBit_b_2_2, ring_size, size, nodeNet);

    Rss_Open_Bitwise(u_2_open, u_2, size, ring_size, nodeNet);
    Rss_Open_Bitwise(u_2_open_2, u_2_2, size, ring_size, nodeNet);

    // for (int i = 0; i < size; i++) {

    //     res_check[i] = (c[i] < r_2_open[i]);
    //     res_check_2[i] = (c2[i] < r_2_open_2[i]);
    //     if (!(u_2_open[i] == res_check[i]) && !(u_2_open_2[i] == res_check_2[i])) {
    //         printf("[%i] c < r_2 : %u   --- expected: %u\n", i, u_2_open[i], res_check[i]);
    //         printf("[%i] c2 < r_2_2 : %u   --- expected: %u\n", i, u_2_open_2[i], res_check_2[i]);
    //         printf("c = %u --- edaBit_b_2 = %u\n", c[i], r_2_open[i]);
    //         printf("c2 = %u --- edaBit_b_2_2 = %u\n", c2[i], r_2_open_2[i]);
    //         printf("BitLT ERROR at %u \n", i);
    //     }
    // }

    // printf("hi3\n");
    for (i = 0; i < size; ++i) {
        // cant do this because we modify edaBit_b_2 earlier
        // sum[0][i] = a[0][i] - c[i] * a1 + (edaBit_r[0][i] - (GET_BIT(edaBit_b_2[0][i], Lint(ring_size - 1)) << Lint(ring_size - 1))) - (u_2[0][i] << Lint(ring_size - 1)) + (b[0][i] << Lint(ring_size - 1));
        // sum[1][i] = a[1][i] - c[i] * a2 + (edaBit_r[1][i] - (GET_BIT(edaBit_b_2[1][i], Lint(ring_size - 1)) << Lint(ring_size - 1))) - (u_2[1][i] << Lint(ring_size - 1)) + (b[1][i] << Lint(ring_size - 1));

        sum2[0][i] = a[0][i] - c2[i] * a1 + edaBit_r[0][i] - (u_2_2[0][i] << Lint(ring_size - 1)) + (b[0][size + i] << Lint(ring_size - 1));
        sum2[1][i] = a[1][i] - c2[i] * a2 + edaBit_r[1][i] - (u_2_2[1][i] << Lint(ring_size - 1)) + (b[1][size + i] << Lint(ring_size - 1));

        sum[0][i] = a[0][i] - c[i] * a1 + edaBit_r[0][i] - (u_2[0][i] << Lint(ring_size - 1)) + (b[0][size + i] << Lint(ring_size - 1));
        sum[1][i] = a[1][i] - c[i] * a2 + edaBit_r[1][i] - (u_2[1][i] << Lint(ring_size - 1)) + (b[1][size + i] << Lint(ring_size - 1));
    }
    // opening sum
    Rss_Open(e, sum, size, ring_size, nodeNet);
    Rss_Open(e2, sum2, size, ring_size, nodeNet);
    // for (int i = 0; i < size; i++) {
    //     if (!(GET_BIT(e[i], ring_size - 1) == GET_BIT(e2[i], ring_size - 1))) {
    //         printf("[%i] e = %u --- e2 = %u\n", i, GET_BIT(e[i], ring_size - 1), GET_BIT(e2[i], ring_size - 1));
    //     }
    // }

    Lint e_bit;
    Lint e_bit2;
    for (i = 0; i < size; ++i) {
        e_bit = GET_BIT(e[i], ring_size - 1); // getting the (k-1)th bit
        res[0][i] = e_bit * a1 + b[0][size + i] - (e_bit * b[0][size + i] << Lint(1));
        res[1][i] = e_bit * a2 + b[1][size + i] - (e_bit * b[1][size + i] << Lint(1));

        // res[0][i] = e_bit2 * a1 + b[0][i] - (e_bit2 * b[0][i] << Lint(1));
        // res[1][i] = e_bit2 * a2 + b[1][i] - (e_bit2 * b[1][i] << Lint(1));
    }

    Rss_Open(res_check, res, size, ring_size, nodeNet);

    for (i = 0; i < size; ++i) {
        e_bit2 = GET_BIT(e2[i], ring_size - 1); // getting the (k-1)th bit
        // res[0][i] = e_bit * a1 + b[0][i] - (e_bit * b[0][i] << Lint(1));
        // res[1][i] = e_bit * a2 + b[1][i] - (e_bit * b[1][i] << Lint(1));

        res[0][i] = e_bit2 * a1 + b[0][size + i] - (e_bit2 * b[0][size + i] << Lint(1));
        res[1][i] = e_bit2 * a2 + b[1][size + i] - (e_bit2 * b[1][size + i] << Lint(1));
    }
    Rss_Open(res_check_2, res, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        if (!(res_check_2[i] == res_check[i])) {
            // printf("actual:  : %llu\t", res_check[i]);
            // printf("expected  : %llu\n", res_check_2[i]);
        }
    }

    // cleanup
    delete[] ai;
    delete[] c2;
    delete[] e2;

    delete[] c;
    delete[] e;

    delete[] res_check;
    delete[] res_check_2;

    delete[] r_2_open;
    delete[] r_2_open_2;
    delete[] u_2_open;
    delete[] u_2_open_2;

    for (i = 0; i < numShares; i++) {
        delete[] edaBit_r[i];
        delete[] edaBit_b_2[i];
        delete[] edaBit_b_2_2[i];
        delete[] b[i];
        delete[] sum[i];
        delete[] sum2[i];
        delete[] u_2[i];
        delete[] u_2_2[i];
        delete[] rprime[i];
        delete[] r[i];
    }
    delete[] edaBit_r;
    delete[] edaBit_b_2;
    delete[] edaBit_b_2_2;
    delete[] b;

    delete[] sum;
    delete[] sum2;

    delete[] u_2;
    delete[] u_2_2;

    delete[] rprime;
    delete[] r;
}

void Rss_MSB(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint i, j, k, index; // used for loops
    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;
    uint numShares = nodeNet->getNumShares();
    uint n_rand_bits = size * (ring_size + 1);

    Lint **r_shares = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        r_shares[i] = new Lint[n_rand_bits];
    }

    Lint **b = new Lint *[numShares];
    Lint **r = new Lint *[numShares];
    Lint **sum = new Lint *[numShares];
    Lint **rprime = new Lint *[numShares];
    Lint **r_2 = new Lint *[numShares];
    Lint **u_2 = new Lint *[numShares];

    Lint *c = new Lint[size];
    memset(c, 0, sizeof(Lint) * size);
    Lint *e = new Lint[size];
    memset(e, 0, sizeof(Lint) * size);

    // Lint *res_check = new Lint[size];
    // Lint *r_2_open = new Lint[size];
    // Lint *u_2_open = new Lint[size];

    for (i = 0; i < numShares; i++) {
        b[i] = new Lint[size];
        r[i] = new Lint[size];
        sum[i] = new Lint[size];
        rprime[i] = new Lint[size];
        memset(rprime[i], 0, sizeof(Lint) * size);
        r_2[i] = new Lint[size];
        memset(r_2[i], 0, sizeof(Lint) * size);
        u_2[i] = new Lint[size];
        memset(u_2[i], 0, sizeof(Lint) * size);
    }

    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
    if (pid == 1) {
        ai[0] = 1; // party 1's share 1
    } else if (pid == 3) {
        ai[1] = 1; // party 3's share 2
    }
    // Lint a1 = 0;
    // Lint a2 = 0;
    // switch (pid) {
    // case 1:
    //     a1 = 1;
    //     a2 = 0;
    //     break;
    // case 2:
    //     a1 = 0;
    //     a2 = 0;
    //     break;
    // case 3:
    //     a1 = 0;
    //     a2 = 1;
    //     break;
    // }

    // offline component start
    Rss_RandBit(r_shares, n_rand_bits, ring_size, nodeNet);
    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            b[s][i] = r_shares[s][size * ring_size + i];
        // b[1][i] = r_shares[1][size * ring_size + i];
    }

    for (j = 0; j < size; j++) {
        for (k = 0; k < ring_size - 1; k++) {
            // this is for step 3
            index = j * ring_size + k;
            for (size_t s = 0; s < numShares; s++)
                rprime[s][j] = rprime[s][j] + (r_shares[s][index] << Lint(k));
            // rprime[1][j] = rprime[1][j] + (r_shares[1][index] << Lint(k));
        }
        index = j * ring_size + k;
        for (size_t s = 0; s < numShares; s++)
            r[s][j] = (rprime[s][j] + ((r_shares[s][index]) << Lint(k)));
        // r[1][j] = (rprime[1][j] + ((r_shares[1][index]) << Lint(k)));
    }

    // for ( i = 0; i < size; i++)
    // {
    //     printf("rprime[0][%i]: %llu\n",i, rprime[0][i]);
    //     print_binary(rprime[0][i], 8*sizeof(Lint));
    //     printf("r[0][%i]: %llu\n",i, r[0][i]);
    //     print_binary(r[0][i], 8*sizeof(Lint));

    //     printf("rprime[1][%i]: %llu\n",i, rprime[1][i]);
    //     print_binary(rprime[1][i], 8*sizeof(Lint));
    //     printf("r[1][%i]: %llu\n",i, r[1][i]);
    //     print_binary(r[1][i], 8*sizeof(Lint));
    // }

    for (j = 0; j < size; j++) {
        for (k = 0; k < ring_size - 1; k++) {
            index = j * ring_size + k;

            for (size_t s = 0; s < numShares; s++)
                r_2[s][j] = Lint(SET_BIT(r_2[s][j], Lint(k), GET_BIT(r_shares[s][index], Lint(0))));
            // r_2[1][j] = Lint(SET_BIT(r_2[1][j], Lint(k), GET_BIT(r_shares[1][index], Lint(0))));
        }
    }
    // offline component ends
    // step 2
    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            sum[s][i] = (a[s][i] + r[s][i]); // & nodeNet->SHIFT[1] ;
        // sum[1][i] = (a[1][i] + r[1][i]); // & nodeNet->SHIFT[1] ;
    }

    // for (size_t i = 0; i < size; i++)
    // {
    //     printf("r[0][%i] : %llu\n", i, r[0][i]);
    //     print_binary(r[0][i], 32);
    //     printf("r[1][%i] : %llu\n", i, r[1][i]);
    //     print_binary(r[0][i], 32);
    // }

    Rss_Open(c, sum, size, ring_size, nodeNet);

    // step 3 -- getting the k-1 lowest bits of c[i]
    for (i = 0; i < size; i++) {
        c[i] = c[i] & nodeNet->SHIFT[ring_size - 1];
    }

    // Rss_Open_Bitwise(r_2_open, r_2, size, ring_size, nodeNet);
    // calculating c <? r_2, where r_2 is bitwise shared
    // gettimeofday(&start,NULL); //start timer here
    Rss_BitLT(u_2, c, r_2, ring_size, size, nodeNet);
    // gettimeofday(&end,NULL);//stop timer here
    // timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
    // printf("runtime for BitLT with data size %d = %ld us\n", size, timer);

    // Rss_Open_Bitwise(u_2_open, u_2, size, ring_size, nodeNet);

    // for (i = 0; i < size; ++i) {
    //     sum[0][i] = a[0][i] - c[i] * a1 + rprime[0][i] - (u_2[0][i] << Lint(ring_size - 1)) + (b[0][i] << Lint(ring_size - 1));
    //     sum[1][i] = a[1][i] - c[i] * a2 + rprime[1][i] - (u_2[1][i] << Lint(ring_size - 1)) + (b[1][i] << Lint(ring_size - 1));
    // }

    for (i = 0; i < size; ++i) {
        // cant do this because we modify edaBit_b_2 earlier
        // for (size_t s = 0; s < numShares; s++)
        for (size_t s = 0; s < numShares; s++)
            sum[s][i] = a[s][i] - c[i] * ai[s] + rprime[s][i] - (u_2[s][i] << Lint(ring_size - 1)) + (b[s][i] << Lint(ring_size - 1));
        // sum[1][i] = a[1][i] - c[i] * a2 + rprime[1][i] - (u_2[1][i] << Lint(ring_size - 1)) + (b[1][i] << Lint(ring_size - 1));
    }
    // opening sum
    Rss_Open(e, sum, size, ring_size, nodeNet);

    Lint e_bit;
    for (i = 0; i < size; ++i) {
        e_bit = GET_BIT(e[i], ring_size - 1); // getting the (k-1)th bit
        for (size_t s = 0; s < numShares; s++)
            res[s][i] = e_bit * ai[s] + b[s][i] - (e_bit * b[s][i] << Lint(1));
        // res[1][i] = e_bit * a2 + b[1][i] - (e_bit * b[1][i] << Lint(1));
    }

    // cleanup
    delete[] c;
    delete[] e;
    // delete[] res_check;
    // delete[] r_2_open;
    // delete[] u_2_open;

    for (i = 0; i < numShares; i++) {
        delete[] r_shares[i];
        delete[] b[i];
        delete[] r[i];
        delete[] sum[i];
        delete[] rprime[i];
        delete[] r_2[i];
        delete[] u_2[i];
    }
    delete[] r_shares;
    delete[] b;
    delete[] r;
    delete[] ai;
    delete[] sum;
    delete[] rprime;
    delete[] r_2;
    delete[] u_2;
}


void new_Rss_MSB_5pc(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet) {
    int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();
    uint i; // used for loops

    // only need to generate a single random bit per private value
    Lint **b = new Lint *[numShares];
    Lint **sum = new Lint *[numShares];
    Lint **u_2 = new Lint *[numShares];
    Lint **edaBit_r = new Lint *[numShares];
    Lint **edaBit_b_2 = new Lint *[numShares];
    Lint **rprime = new Lint *[numShares];

    Lint *c = new Lint[size];
    Lint *e = new Lint[size];

    // used for testing correctness
    Lint *res_check = new Lint[size];
    Lint *r_2_open = new Lint[size];
    Lint *u_2_open = new Lint[size];

    for (i = 0; i < numShares; i++) {
        b[i] = new Lint[size];
        edaBit_r[i] = new Lint[size];
        edaBit_b_2[i] = new Lint[size];
        sum[i] = new Lint[size];
        u_2[i] = new Lint[size];

        rprime[i] = new Lint[size];
    }

    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
    // if (pid == 1) {
    //     ai[0] = 1;
    // } else if (pid == numShares) {
    //     ai[numShares - 1] = 1;
    // }
    if (pid == 1) {
        ai[0] = 1;
    } else if (pid == 4) {
        ai[5] = 1; // party 4's share 6
    } else if (pid == 5) {
        ai[3] = 1; // parthy 5's share 4
    }

    // Lint a1 = 0;
    // Lint a2 = 0;
    // switch (pid) {
    // case 1:
    //     a1 = 1;
    //     a2 = 0;
    //     break;
    // case 2:
    //     a1 = 0;
    //     a2 = 0;
    //     break;
    // case 3:
    //     a1 = 0;
    //     a2 = 1;
    //     break;
    // }
    // stays the same
    Rss_RandBit_5pc(b, size, ring_size, nodeNet);

    // Rss_edaBit_5pc(edaBit_r, edaBit_b_2, size, ring_size, ring_size - 1, nodeNet);
    // need to generate full edabit for final implementation
    Rss_edaBit_5pc(edaBit_r, edaBit_b_2, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            rprime[s][i] = edaBit_r[s][i] - (GET_BIT(edaBit_b_2[s][i], Lint(ring_size - 1)) << Lint(ring_size - 1));
            // rprime[1][i] = edaBit_r[1][i] - (GET_BIT(edaBit_b_2[1][i], Lint(ring_size - 1)) << Lint(ring_size - 1));
            // combining w the next loop
            // combining w the previous loop
            sum[s][i] = (a[s][i] + edaBit_r[s][i]);
            // sum[1][i] = (a[1][i] + edaBit_r[1][i]);
            // sum[0][i] = (a[0][i] + edaBit_r[0][i]) << Lint(1);
            // sum[1][i] = (a[1][i] + edaBit_r[1][i]) << Lint(1);
        }
    }

    Rss_Open_5pc(c, sum, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            edaBit_b_2[s][i] = edaBit_b_2[s][i] & nodeNet->SHIFT[ring_size - 1];

        // used for alternate solution
        // prevents us from using it directly later on
        // edaBit_b_2[1][i] = edaBit_b_2[1][i] & nodeNet->SHIFT[ring_size - 1];

        // definitely needed
        c[i] = c[i] & nodeNet->SHIFT[ring_size - 1];
        // c[i] = c[i] >> Lint(1);
    }

    // Rss_Open_Bitwise(r_2_open, edaBit_b_2, size, map, ring_size, nodeNet);
    // // this part is still correct
    // however, the edaBit_b_2 shares do get modified
    // which may not be desierable
    Rss_BitLT_5pc(u_2, c, edaBit_b_2, ring_size, size, nodeNet);

    for (i = 0; i < size; ++i) {
        // cant do this because we modify edaBit_b_2 earlier
        // for (size_t s = 0; s < numShares; s++)
        for (size_t s = 0; s < numShares; s++)
            sum[s][i] = a[s][i] - c[i] * ai[s] + rprime[s][i] - (u_2[s][i] << Lint(ring_size - 1)) + (b[s][i] << Lint(ring_size - 1));
        // sum[1][i] = a[1][i] - c[i] * a2 + rprime[1][i] - (u_2[1][i] << Lint(ring_size - 1)) + (b[1][i] << Lint(ring_size - 1));
    }
    // opening sum
    Rss_Open_5pc(e, sum, size, ring_size, nodeNet);

    Lint e_bit;
    for (i = 0; i < size; ++i) {
        e_bit = GET_BIT(e[i], ring_size - 1); // getting the (k-1)th bit
        for (size_t s = 0; s < numShares; s++) {
            res[s][i] = e_bit * ai[s] + b[s][i] - (e_bit * b[s][i] << Lint(1));
        }
        // res[1][i] = e_bit * a2 + b[1][i] - (e_bit * b[1][i] << Lint(1));
    }

    // cleanup
    delete[] c;
    delete[] ai;
    delete[] e;
    delete[] res_check;
    delete[] r_2_open;
    delete[] u_2_open;

    for (i = 0; i < numShares; i++) {
        delete[] edaBit_r[i];
        delete[] edaBit_b_2[i];
        delete[] b[i];
        delete[] sum[i];
        delete[] u_2[i];
        delete[] rprime[i];
    }
    delete[] edaBit_r;
    delete[] edaBit_b_2;
    delete[] b;
    delete[] sum;
    delete[] u_2;
    delete[] rprime;
}

void new_Rss_MSB_time_5pc(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet, unsigned long &timer) {

    struct timeval start;
    struct timeval end;
    unsigned long op_timer = 0;
    uint numShares = nodeNet->getNumShares();

    int pid = nodeNet->getID();
    uint i; // used for loops

    // only need to generate a single random bit per private value
    Lint **b = new Lint *[numShares];
    Lint **sum = new Lint *[numShares];
    Lint **u_2 = new Lint *[numShares];
    Lint **edaBit_r = new Lint *[numShares];
    Lint **edaBit_b_2 = new Lint *[numShares];
    Lint **rprime = new Lint *[numShares];

    Lint *c = new Lint[size];
    Lint *e = new Lint[size];

    // used for testing correctness
    Lint *res_check = new Lint[size];
    Lint *r_2_open = new Lint[size];
    Lint *u_2_open = new Lint[size];

    for (i = 0; i < numShares; i++) {
        b[i] = new Lint[size];
        edaBit_r[i] = new Lint[size];
        edaBit_b_2[i] = new Lint[size];
        sum[i] = new Lint[size];
        u_2[i] = new Lint[size];

        rprime[i] = new Lint[size];
    }

    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
    // if (pid == 1) {
    //     ai[0] = 1;
    // } else if (pid == numShares) {
    //     ai[numShares - 1] = 1;
    // }

    if (pid == 1) {
        ai[0] = 1;
    } else if (pid == 4) {
        ai[5] = 1; // party 4's share 6
    } else if (pid == 5) {
        ai[3] = 1; // parthy 5's share 4
    }
    // Lint a1 = 0;
    // Lint a2 = 0;
    // switch (pid) {
    // case 1:
    //     a1 = 1;
    //     a2 = 0;
    //     break;
    // case 2:
    //     a1 = 0;
    //     a2 = 0;
    //     break;
    // case 3:
    //     a1 = 0;
    //     a2 = 1;
    //     break;
    // }

    gettimeofday(&start, NULL); // start timer here
    // stays the same
    Rss_RandBit_5pc(b, size, ring_size, nodeNet);
    gettimeofday(&end, NULL); // stop timer here
    op_timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    timer += 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    // printf("----> rb time = %.6lf\n", (double)(op_timer * 1e-6));
    // Rss_edaBit_5pc(edaBit_r, edaBit_b_2, size, ring_size, ring_size - 1, nodeNet);
    // need to generate full edabit for final implementation
    gettimeofday(&start, NULL); // start timer here

    Rss_edaBit_5pc(edaBit_r, edaBit_b_2, size, ring_size, nodeNet);
    gettimeofday(&end, NULL); // stop timer here
    op_timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    timer += 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;

    // printf("----> edaBit MSB time = %.6lf\n", (double)(op_timer * 1e-6));

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            rprime[s][i] = edaBit_r[s][i] - (GET_BIT(edaBit_b_2[s][i], Lint(ring_size - 1)) << Lint(ring_size - 1));
            sum[s][i] = (a[s][i] + edaBit_r[s][i]);
        }
        // rprime[1][i] = edaBit_r[1][i] - (GET_BIT(edaBit_b_2[1][i], Lint(ring_size - 1)) << Lint(ring_size - 1));
        // combining w the next loop
        // combining w the previous loop
        // sum[1][i] = (a[1][i] + edaBit_r[1][i]);
        // sum[0][i] = (a[0][i] + edaBit_r[0][i]) << Lint(1);
        // sum[1][i] = (a[1][i] + edaBit_r[1][i]) << Lint(1);
    }

    Rss_Open_5pc(c, sum, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            edaBit_b_2[s][i] = edaBit_b_2[s][i] & nodeNet->SHIFT[ring_size - 1];

        // used for alternate solution
        // prevents us from using it directly later on
        // edaBit_b_2[0][i] = edaBit_b_2[0][i] & nodeNet->SHIFT[ring_size - 1];
        // edaBit_b_2[1][i] = edaBit_b_2[1][i] & nodeNet->SHIFT[ring_size - 1];

        // definitely needed
        c[i] = c[i] & nodeNet->SHIFT[ring_size - 1];
        // c[i] = c[i] >> Lint(1);
    }

    // Rss_Open_Bitwise(r_2_open, edaBit_b_2, size, map, ring_size, nodeNet);
    // // this part is still correct
    // however, the edaBit_b_2 shares do get modified
    // which may not be desierable
    Rss_BitLT_5pc(u_2, c, edaBit_b_2, ring_size, size, nodeNet);

    // Rss_Open_Bitwise(u_2_open, u_2, size, map, ring_size, nodeNet);

    // for (int i = 0; i < size; i++) {

    //     res_check[i] = (c[i] < r_2_open[i]);
    //     if (!(u_2_open[i] == res_check[i])) {
    //         // printf("[%i] c < r_2 : %u   --- expected: %u\n", i, u_2_open[i], res_check[i]);
    //         // printf("c = %u --- edaBit_b_2 = %u\n", c[i], r_2_open[i]);
    //         // printf("BitLT ERROR at %d \n", i);
    //     }
    // }

    for (i = 0; i < size; ++i) {
        // cant do this because we modify edaBit_b_2 earlier

        for (size_t s = 0; s < numShares; s++)
            sum[s][i] = a[s][i] - c[i] * ai[s] + rprime[s][i] - (u_2[s][i] << Lint(ring_size - 1)) + (b[s][i] << Lint(ring_size - 1));
        // sum[1][i] = a[1][i] - c[i] * a2 + rprime[1][i] - (u_2[1][i] << Lint(ring_size - 1)) + (b[1][i] << Lint(ring_size - 1));

        // sum[0][i] = a[0][i] - c[i] * a1 + edaBit_r[0][i] - (u_2[0][i] << Lint(ring_size - 1)) + (b[0][i] << Lint(ring_size - 1));
        // sum[1][i] = a[1][i] - c[i] * a2 + edaBit_r[1][i] - (u_2[1][i] << Lint(ring_size - 1)) + (b[1][i] << Lint(ring_size - 1));
    }
    // opening sum
    Rss_Open_5pc(e, sum, size, ring_size, nodeNet);

    Lint e_bit;
    for (i = 0; i < size; ++i) {
        e_bit = GET_BIT(e[i], ring_size - 1); // getting the (k-1)th bit
        for (size_t s = 0; s < numShares; s++)
            res[s][i] = e_bit * ai[s] + b[s][i] - (e_bit * b[s][i] << Lint(1));
        // res[1][i] = e_bit * a2 + b[1][i] - (e_bit * b[1][i] << Lint(1));
    }

    // cleanup
    delete[] ai;
    delete[] c;
    delete[] e;
    delete[] res_check;
    delete[] r_2_open;
    delete[] u_2_open;

    for (i = 0; i < numShares; i++) {
        delete[] edaBit_r[i];
        delete[] edaBit_b_2[i];
        delete[] b[i];
        delete[] sum[i];
        delete[] u_2[i];
        delete[] rprime[i];
    }
    delete[] edaBit_r;
    delete[] edaBit_b_2;
    delete[] b;
    delete[] sum;
    delete[] u_2;
    delete[] rprime;
}

void Rss_MSB_5pc(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint i, j, k, index; // used for loops
    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;
    uint numShares = nodeNet->getNumShares();
    uint n_rand_bits = size * (ring_size + 1);

    Lint **r_shares = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        r_shares[i] = new Lint[n_rand_bits];
    }

    Lint **b = new Lint *[numShares];
    Lint **r = new Lint *[numShares];
    Lint **sum = new Lint *[numShares];
    Lint **rprime = new Lint *[numShares];
    Lint **r_2 = new Lint *[numShares];
    Lint **u_2 = new Lint *[numShares];

    Lint *c = new Lint[size];
    memset(c, 0, sizeof(Lint) * size);
    Lint *e = new Lint[size];
    memset(e, 0, sizeof(Lint) * size);

    // Lint *res_check = new Lint[size];
    // Lint *r_2_open = new Lint[size];
    // Lint *u_2_open = new Lint[size];

    for (i = 0; i < numShares; i++) {
        b[i] = new Lint[size];
        r[i] = new Lint[size];
        sum[i] = new Lint[size];
        rprime[i] = new Lint[size];
        memset(rprime[i], 0, sizeof(Lint) * size);
        r_2[i] = new Lint[size];
        memset(r_2[i], 0, sizeof(Lint) * size);
        u_2[i] = new Lint[size];
        memset(u_2[i], 0, sizeof(Lint) * size);
    }

    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
    if (pid == 1) {
        ai[0] = 1;
    } else if (pid == 4) {
        ai[5] = 1; // party 4's share 6
    } else if (pid == 5) {
        ai[3] = 1; // parthy 5's share 4
    }
    // Lint a1 = 0;
    // Lint a2 = 0;
    // switch (pid) {
    // case 1:
    //     a1 = 1;
    //     a2 = 0;
    //     break;
    // case 2:
    //     a1 = 0;
    //     a2 = 0;
    //     break;
    // case 3:
    //     a1 = 0;
    //     a2 = 1;
    //     break;
    // }
    // printf("hi\n");
    // offline component start
    Rss_RandBit_5pc(r_shares, n_rand_bits, ring_size, nodeNet);
    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            b[s][i] = r_shares[s][size * ring_size + i];
        // b[1][i] = r_shares[1][size * ring_size + i];
    }
    // printf("Rss_RandBit_mp\n");

    for (j = 0; j < size; j++) {
        for (k = 0; k < ring_size - 1; k++) {
            // this is for step 3
            index = j * ring_size + k;
            for (size_t s = 0; s < numShares; s++)
                rprime[s][j] = rprime[s][j] + (r_shares[s][index] << Lint(k));
            // rprime[1][j] = rprime[1][j] + (r_shares[1][index] << Lint(k));
        }
        index = j * ring_size + k;
        for (size_t s = 0; s < numShares; s++)
            r[s][j] = (rprime[s][j] + ((r_shares[s][index]) << Lint(k)));
        // r[1][j] = (rprime[1][j] + ((r_shares[1][index]) << Lint(k)));
    }

    // for ( i = 0; i < size; i++)
    // {
    //     printf("rprime[0][%i]: %llu\n",i, rprime[0][i]);
    //     print_binary(rprime[0][i], 8*sizeof(Lint));
    //     printf("r[0][%i]: %llu\n",i, r[0][i]);
    //     print_binary(r[0][i], 8*sizeof(Lint));

    //     printf("rprime[1][%i]: %llu\n",i, rprime[1][i]);
    //     print_binary(rprime[1][i], 8*sizeof(Lint));
    //     printf("r[1][%i]: %llu\n",i, r[1][i]);
    //     print_binary(r[1][i], 8*sizeof(Lint));
    // }

    for (j = 0; j < size; j++) {
        for (k = 0; k < ring_size - 1; k++) {
            index = j * ring_size + k;

            for (size_t s = 0; s < numShares; s++)
                r_2[s][j] = Lint(SET_BIT(r_2[s][j], Lint(k), GET_BIT(r_shares[s][index], Lint(0))));
            // r_2[1][j] = Lint(SET_BIT(r_2[1][j], Lint(k), GET_BIT(r_shares[1][index], Lint(0))));
        }
    }
    // offline component ends
    // step 2
    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            sum[s][i] = (a[s][i] + r[s][i]); // & nodeNet->SHIFT[1] ;
        // sum[1][i] = (a[1][i] + r[1][i]); // & nodeNet->SHIFT[1] ;
    }

    // for (size_t i = 0; i < size; i++)
    // {
    //     printf("r[0][%i] : %llu\n", i, r[0][i]);
    //     print_binary(r[0][i], 32);
    //     printf("r[1][%i] : %llu\n", i, r[1][i]);
    //     print_binary(r[0][i], 32);
    // }

    Rss_Open_5pc(c, sum, size, ring_size, nodeNet);
    // printf("Rss_Open_mp\n");

    // step 3 -- getting the k-1 lowest bits of c[i]
    for (i = 0; i < size; i++) {
        c[i] = c[i] & nodeNet->SHIFT[ring_size - 1];
    }

    // Rss_Open_Bitwise(r_2_open, r_2, size, map, ring_size, nodeNet);
    // calculating c <? r_2, where r_2 is bitwise shared
    // gettimeofday(&start,NULL); //start timer here
    Rss_BitLT_5pc(u_2, c, r_2, ring_size, size, nodeNet);
    // printf("Rss_BitLT_mp\n");

    // gettimeofday(&end,NULL);//stop timer here
    // timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
    // printf("runtime for BitLT with data size %d = %ld us\n", size, timer);

    // Rss_Open_Bitwise(u_2_open, u_2, size, map, ring_size, nodeNet);

    // for (i = 0; i < size; ++i) {
    //     sum[0][i] = a[0][i] - c[i] * a1 + rprime[0][i] - (u_2[0][i] << Lint(ring_size - 1)) + (b[0][i] << Lint(ring_size - 1));
    //     sum[1][i] = a[1][i] - c[i] * a2 + rprime[1][i] - (u_2[1][i] << Lint(ring_size - 1)) + (b[1][i] << Lint(ring_size - 1));
    // }

    for (i = 0; i < size; ++i) {
        // cant do this because we modify edaBit_b_2 earlier
        // for (size_t s = 0; s < numShares; s++)
        for (size_t s = 0; s < numShares; s++)
            sum[s][i] = a[s][i] - c[i] * ai[s] + rprime[s][i] - (u_2[s][i] << Lint(ring_size - 1)) + (b[s][i] << Lint(ring_size - 1));
        // sum[1][i] = a[1][i] - c[i] * a2 + rprime[1][i] - (u_2[1][i] << Lint(ring_size - 1)) + (b[1][i] << Lint(ring_size - 1));
    }
    // opening sum
    Rss_Open_5pc(e, sum, size, ring_size, nodeNet);

    Lint e_bit;
    for (i = 0; i < size; ++i) {
        e_bit = GET_BIT(e[i], ring_size - 1); // getting the (k-1)th bit
        for (size_t s = 0; s < numShares; s++)
            res[s][i] = e_bit * ai[s] + b[s][i] - (e_bit * b[s][i] << Lint(1));
        // res[1][i] = e_bit * a2 + b[1][i] - (e_bit * b[1][i] << Lint(1));
    }

    // cleanup
    delete[] ai;
    delete[] c;
    delete[] e;
    // delete[] res_check;
    // delete[] r_2_open;
    // delete[] u_2_open;

    for (i = 0; i < numShares; i++) {
        delete[] r_shares[i];
        delete[] b[i];
        delete[] r[i];
        delete[] sum[i];
        delete[] rprime[i];
        delete[] r_2[i];
        delete[] u_2[i];
    }
    delete[] r_shares;
    delete[] b;
    delete[] r;
    delete[] sum;
    delete[] rprime;
    delete[] r_2;
    delete[] u_2;
}


void new_Rss_MSB_7pc(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet) {
    int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();
    uint i; // used for loops

    // only need to generate a single random bit per private value
    Lint **b = new Lint *[numShares];
    Lint **sum = new Lint *[numShares];
    Lint **u_2 = new Lint *[numShares];
    Lint **edaBit_r = new Lint *[numShares];
    Lint **edaBit_b_2 = new Lint *[numShares];
    Lint **rprime = new Lint *[numShares];

    Lint *c = new Lint[size];
    Lint *e = new Lint[size];

    // used for testing correctness
    Lint *res_check = new Lint[size];
    Lint *r_2_open = new Lint[size];
    Lint *u_2_open = new Lint[size];

    for (i = 0; i < numShares; i++) {
        b[i] = new Lint[size];
        edaBit_r[i] = new Lint[size];
        edaBit_b_2[i] = new Lint[size];
        sum[i] = new Lint[size];
        u_2[i] = new Lint[size];

        rprime[i] = new Lint[size];
    }

    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
    if (pid == 1) {
        ai[0] = 1;
    } else if (pid == 5) {
        ai[19] = 1; // party 4's share 6
    } else if (pid == 6) {
        ai[16] = 1; // parthy 5's share 4
    } else if (pid == 7) {
        ai[10] = 1; // parthy 5's share 4
    } 

    Rss_RandBit_7pc(b, size, ring_size, nodeNet);
    Rss_edaBit_7pc(edaBit_r, edaBit_b_2, size, ring_size, nodeNet);

    // need to generate full edabit for final implementation
    // Rss_edaBit_7pc(edaBit_r, edaBit_b_2, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            rprime[s][i] = edaBit_r[s][i] - (GET_BIT(edaBit_b_2[s][i], Lint(ring_size - 1)) << Lint(ring_size - 1));
            sum[s][i] = (a[s][i] + edaBit_r[s][i]);
        }
    }

    Rss_Open_7pc(c, sum, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            edaBit_b_2[s][i] = edaBit_b_2[s][i] & nodeNet->SHIFT[ring_size - 1];
        c[i] = c[i] & nodeNet->SHIFT[ring_size - 1];
    }

    Rss_BitLT_7pc(u_2, c, edaBit_b_2, ring_size, size, nodeNet);

    for (i = 0; i < size; ++i) {
        // cant do this because we modify edaBit_b_2 earlier
        // for (size_t s = 0; s < numShares; s++)
        for (size_t s = 0; s < numShares; s++)
            sum[s][i] = a[s][i] - c[i] * ai[s] + rprime[s][i] - (u_2[s][i] << Lint(ring_size - 1)) + (b[s][i] << Lint(ring_size - 1));
        // sum[1][i] = a[1][i] - c[i] * a2 + rprime[1][i] - (u_2[1][i] << Lint(ring_size - 1)) + (b[1][i] << Lint(ring_size - 1));
    }
    // opening sum
    Rss_Open_7pc(e, sum, size, ring_size, nodeNet);

    Lint e_bit;
    for (i = 0; i < size; ++i) {
        e_bit = GET_BIT(e[i], ring_size - 1); // getting the (k-1)th bit
        for (size_t s = 0; s < numShares; s++) {
            res[s][i] = e_bit * ai[s] + b[s][i] - (e_bit * b[s][i] << Lint(1));
        }
        // res[1][i] = e_bit * a2 + b[1][i] - (e_bit * b[1][i] << Lint(1));
    }

    // cleanup
    delete[] c;
    delete[] ai;
    delete[] e;
    delete[] res_check;
    delete[] r_2_open;
    delete[] u_2_open;

    for (i = 0; i < numShares; i++) {
        delete[] edaBit_r[i];
        delete[] edaBit_b_2[i];
        delete[] b[i];
        delete[] sum[i];
        delete[] u_2[i];
        delete[] rprime[i];
    }
    delete[] edaBit_r;
    delete[] edaBit_b_2;
    delete[] b;
    delete[] sum;
    delete[] u_2;
    delete[] rprime;
}


void Rss_MSB_7pc(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint i, j, k, index; // used for loops
    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;
    uint numShares = nodeNet->getNumShares();
    uint n_rand_bits = size * (ring_size + 1);

    Lint **r_shares = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        r_shares[i] = new Lint[n_rand_bits];
    }

    Lint **b = new Lint *[numShares];
    Lint **r = new Lint *[numShares];
    Lint **sum = new Lint *[numShares];
    Lint **rprime = new Lint *[numShares];
    Lint **r_2 = new Lint *[numShares];
    Lint **u_2 = new Lint *[numShares];

    Lint *c = new Lint[size];
    memset(c, 0, sizeof(Lint) * size);
    Lint *e = new Lint[size];
    memset(e, 0, sizeof(Lint) * size);

    // Lint *res_check = new Lint[size];
    // Lint *r_2_open = new Lint[size];
    // Lint *u_2_open = new Lint[size];

    for (i = 0; i < numShares; i++) {
        b[i] = new Lint[size];
        r[i] = new Lint[size];
        sum[i] = new Lint[size];
        rprime[i] = new Lint[size];
        memset(rprime[i], 0, sizeof(Lint) * size);
        r_2[i] = new Lint[size];
        memset(r_2[i], 0, sizeof(Lint) * size);
        u_2[i] = new Lint[size];
        memset(u_2[i], 0, sizeof(Lint) * size);
    }

    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
    if (pid == 1) {
        ai[0] = 1;
    } else if (pid == 5) {
        ai[19] = 1; // party 4's share 6
    } else if (pid == 6) {
        ai[16] = 1; // parthy 5's share 4
    } else if (pid == 7) {
        ai[10] = 1; // parthy 5's share 4
    } 
   
    // printf("Rss_RandBit_7pc start\n");
   
    Rss_RandBit_7pc(r_shares, n_rand_bits, ring_size, nodeNet);
    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            b[s][i] = r_shares[s][size * ring_size + i];
        // b[1][i] = r_shares[1][size * ring_size + i];
    }
    // printf("Rss_RandBit_7pc end\n");

    for (j = 0; j < size; j++) {
        for (k = 0; k < ring_size - 1; k++) {
            // this is for step 3
            index = j * ring_size + k;
            for (size_t s = 0; s < numShares; s++)
                rprime[s][j] = rprime[s][j] + (r_shares[s][index] << Lint(k));
            // rprime[1][j] = rprime[1][j] + (r_shares[1][index] << Lint(k));
        }
        index = j * ring_size + k;
        for (size_t s = 0; s < numShares; s++)
            r[s][j] = (rprime[s][j] + ((r_shares[s][index]) << Lint(k)));
        // r[1][j] = (rprime[1][j] + ((r_shares[1][index]) << Lint(k)));
    }



    for (j = 0; j < size; j++) {
        for (k = 0; k < ring_size - 1; k++) {
            index = j * ring_size + k;

            for (size_t s = 0; s < numShares; s++)
                r_2[s][j] = Lint(SET_BIT(r_2[s][j], Lint(k), GET_BIT(r_shares[s][index], Lint(0))));
            // r_2[1][j] = Lint(SET_BIT(r_2[1][j], Lint(k), GET_BIT(r_shares[1][index], Lint(0))));
        }
    }
    // offline component ends
    // step 2
    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            sum[s][i] = (a[s][i] + r[s][i]); // & nodeNet->SHIFT[1] ;
        // sum[1][i] = (a[1][i] + r[1][i]); // & nodeNet->SHIFT[1] ;
    }

    // printf("Rss_Open_7pc\n");
    Rss_Open_7pc(c, sum, size, ring_size, nodeNet);
    // printf("Rss_Open_7pc_end\n");

    // step 3 -- getting the k-1 lowest bits of c[i]
    for (i = 0; i < size; i++) {
        c[i] = c[i] & nodeNet->SHIFT[ring_size - 1];
    }
    // printf("Rss_BitLT_7pc\n");

    Rss_BitLT_7pc(u_2, c, r_2, ring_size, size, nodeNet);
 
    for (i = 0; i < size; ++i) {
        // cant do this because we modify edaBit_b_2 earlier
        // for (size_t s = 0; s < numShares; s++)
        for (size_t s = 0; s < numShares; s++)
            sum[s][i] = a[s][i] - c[i] * ai[s] + rprime[s][i] - (u_2[s][i] << Lint(ring_size - 1)) + (b[s][i] << Lint(ring_size - 1));
        // sum[1][i] = a[1][i] - c[i] * a2 + rprime[1][i] - (u_2[1][i] << Lint(ring_size - 1)) + (b[1][i] << Lint(ring_size - 1));
    }
    // opening sum
    // printf("Rss_Open_7pc\n");

    Rss_Open_7pc(e, sum, size, ring_size, nodeNet);
    // printf("Rss_Open_7pc_end\n");

    Lint e_bit;
    for (i = 0; i < size; ++i) {
        e_bit = GET_BIT(e[i], ring_size - 1); // getting the (k-1)th bit
        for (size_t s = 0; s < numShares; s++)
            res[s][i] = e_bit * ai[s] + b[s][i] - (e_bit * b[s][i] << Lint(1));
        // res[1][i] = e_bit * a2 + b[1][i] - (e_bit * b[1][i] << Lint(1));
    }

    // cleanup
    delete[] ai;
    delete[] c;
    delete[] e;
    // delete[] res_check;
    // delete[] r_2_open;
    // delete[] u_2_open;

    for (i = 0; i < numShares; i++) {
        delete[] r_shares[i];
        delete[] b[i];
        delete[] r[i];
        delete[] sum[i];
        delete[] rprime[i];
        delete[] r_2[i];
        delete[] u_2[i];
    }
    delete[] r_shares;
    delete[] b;
    delete[] r;
    delete[] sum;
    delete[] rprime;
    delete[] r_2;
    delete[] u_2;
}
