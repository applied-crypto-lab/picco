#include "b2a.h"

// this implementaiton of b2a assumes uses a single secret bit (shared in Z_2) is stored in a Lint
// this is directly compatible with the new b2a implementation
void Rss_b2a(Lint **res, Lint **a, uint ring_size, uint size, NodeNetwork *nodeNet) {
    int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();
    uint i;
    Lint **b = new Lint *[numShares];
    Lint **b_2 = new Lint *[numShares];
    Lint **sum = new Lint *[numShares];

    Lint *c = new Lint[size];

    for (i = 0; i < numShares; i++) {
        b[i] = new Lint[size];
        b_2[i] = new Lint[size];
        sum[i] = new Lint[size];
    }
    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
    if (pid == 1) {
        ai[0] = 1; // party 1's share 1
    } else if (pid == 3) {
        ai[1] = 1; // party 3's share 2
    }

    Rss_RandBit(b, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            b_2[s][i] = GET_BIT(b[s][i], Lint(0));
            // b_2[1][i] = GET_BIT(b[1][i], Lint(0));

            sum[s][i] = (a[s][i] ^ b_2[s][i]);
            // sum[1][i] = (a[1][i] ^ b_2[1][i]);
        }
    }

    Rss_Open_Bitwise(c, sum, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            res[s][i] = ai[s] * c[i] + b[s][i] - 2 * c[i] * b[s][i];
        }
        // res[1][i] = a2 * c[i] + b[1][i] - 2 * c[i] * b[1][i];
    }

    delete[] c;
    delete[] ai;

    for (i = 0; i < numShares; i++) {
        delete[] b[i];
        delete[] b_2[i];
        delete[] sum[i];
    }
    delete[] b;
    delete[] b_2;
    delete[] sum;
}

void Rss_b2a_opt(Lint **res, Lint **a, uint size, uint ring_size, uint bitlength, NodeNetwork *nodeNet) {
    int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();
    uint i;
    Lint **b = new Lint *[numShares];
    Lint **b_2 = new Lint *[numShares];
    Lint **sum = new Lint *[numShares];

    Lint *c = new Lint[size];

    for (i = 0; i < numShares; i++) {
        b[i] = new Lint[size];
        b_2[i] = new Lint[size];
        sum[i] = new Lint[size];
    }
    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
    if (pid == 1) {
        ai[0] = 1; // party 1's share 1
    } else if (pid == 3) {
        ai[1] = 1; // party 3's share 2
    }

    Rss_RandBit(b, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            b_2[s][i] = GET_BIT(b[s][i], Lint(0));
            // b_2[1][i] = GET_BIT(b[1][i], Lint(0));

            sum[s][i] = (a[s][i] ^ b_2[s][i]);
            // sum[1][i] = (a[1][i] ^ b_2[1][i]);
        }
    }

    Rss_Open_Bitwise(c, sum, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            res[s][i] = ai[s] * c[i] + b[s][i] - 2 * c[i] * b[s][i];
        }
        // res[1][i] = a2 * c[i] + b[1][i] - 2 * c[i] * b[1][i];
    }

    delete[] c;
    delete[] ai;

    for (i = 0; i < numShares; i++) {
        delete[] b[i];
        delete[] b_2[i];
        delete[] sum[i];
    }
    delete[] b;
    delete[] b_2;
    delete[] sum;
}

void Rss_b2a_5pc(Lint **res, Lint **a, uint ring_size, uint size, NodeNetwork *nodeNet) {
    int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();
    uint i;
    Lint **b = new Lint *[numShares];
    Lint **b_2 = new Lint *[numShares];
    Lint **sum = new Lint *[numShares];

    Lint *c = new Lint[size];

    for (i = 0; i < numShares; i++) {
        b[i] = new Lint[size];
        b_2[i] = new Lint[size];
        sum[i] = new Lint[size];
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
    Rss_RandBit_5pc(b, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            b_2[s][i] = GET_BIT(b[s][i], Lint(0));
            // b_2[1][i] = GET_BIT(b[1][i], Lint(0));

            sum[s][i] = (a[s][i] ^ b_2[s][i]);
            // sum[1][i] = (a[1][i] ^ b_2[1][i]);
        }
    }

    Rss_Open_Bitwise_5pc(c, sum, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            res[s][i] = ai[s] * c[i] + b[s][i] - 2 * c[i] * b[s][i];
        }
        // res[1][i] = a2 * c[i] + b[1][i] - 2 * c[i] * b[1][i];
    }

    delete[] c;
    delete[] ai;

    for (i = 0; i < numShares; i++) {
        delete[] b[i];
        delete[] b_2[i];
        delete[] sum[i];
    }
    delete[] b;
    delete[] b_2;
    delete[] sum;
}