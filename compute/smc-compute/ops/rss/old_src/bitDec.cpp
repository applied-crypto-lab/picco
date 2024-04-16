#include "bitDec.h"

// IMPORTANT !!!!!!!!!
// the result is returned as bitwise shares packed into a single Lint (res)
void Rss_BitDec(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint i, j, k, index; // used for loops
    uint numShares = nodeNet->getNumShares();

    // first size*ring_size random bits used for first part of share conversion
    // 2nd half used for 2tok'

    Lint **edaBit_r = new Lint *[numShares];
    Lint **edaBit_b_2 = new Lint *[numShares];
    Lint **sum = new Lint *[numShares];

    Lint *c = new Lint[size];

    for (i = 0; i < numShares; i++) {
        sum[i] = new Lint[size];

        edaBit_r[i] = new Lint[size];
        edaBit_b_2[i] = new Lint[size];
    }

    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
    if (pid == 1) {
        ai[0] = 1; // party 1's share 1
    } else if (pid == 3) {
        ai[1] = 1; // party 3's share 2
    }

    // only need ring_size bit-length values for both parts of computation
    // first protection and b2a
    edaBit(edaBit_r, edaBit_b_2, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            sum[s][i] = (a[s][i] - edaBit_r[s][i]);
        }
    }

    Rss_Open(c, sum, size, ring_size, nodeNet);

    Rss_BitAdd(res, c, edaBit_b_2, ring_size, size, nodeNet);

    delete[] ai;
    delete[] c;

    for (i = 0; i < numShares; i++) {
        delete[] edaBit_r[i];
        delete[] edaBit_b_2[i];
        delete[] sum[i];
    }
    delete[] edaBit_r;
    delete[] edaBit_b_2;
    delete[] sum;
}