#include "randBit.h"

void Rss_RandBit(Lint **b, uint size, uint ring_size, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint i;
    uint bytes = (ring_size + 9) >> 3;
    // printf("bytes : %llu\n", bytes );
    uint numShares = nodeNet->getNumShares();

    Lint **u = new Lint *[numShares];
    Lint **a = new Lint *[numShares];
    Lint **d = new Lint *[numShares];

    for (i = 0; i < numShares; i++) {
        u[i] = new Lint[size];
        a[i] = new Lint[size];
        d[i] = new Lint[size];
    }
    Lint *e = new Lint[size];
    Lint *c = new Lint[size];
    uint8_t *buffer = new uint8_t[bytes * size];

    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
    if (pid == 1) {
        ai[0] = 1;
    } else if (pid == 3) {
        ai[numShares - 1] = 1;
    }

    nodeNet->prg_getrandom(0, bytes, size, buffer);
    for (i = 0; i < size; i++) {
        memcpy(u[0] + i, buffer + i * bytes, bytes);
    }
    nodeNet->prg_getrandom(1, bytes, size, buffer);
    for (i = 0; i < size; i++) {
        memcpy(u[1] + i, buffer + i * bytes, bytes);
    }

    for (i = 0; i < size; i++) {
        // ensuring [a] is odd
        for (size_t s = 0; s < numShares; s++)
            a[s][i] = (u[s][i] << Lint(1)) + ai[s];
        // a[1][i] = (u[1][i] << Lint(1)) + a2;
    }
    // squaring a
    Rss_MultPub(e, a, a, size, ring_size + 2, nodeNet); // ringsize+2
    rss_sqrt_inv(c, e, size, ring_size + 2);

    // effectively combines the two loops into one, eliminates d variable
    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            b[s][i] = (c[i] * a[s][i] + ai[s]) >> Lint(1);
        // b[1][i] = (c[i] * a[1][i] + a2) >> (1);
    }

    // freeing up
    delete[] c;
    delete[] buffer;
    delete[] e;
    for (i = 0; i < numShares; i++) {
        delete[] d[i];
        delete[] a[i];
        delete[] u[i];
    }
    delete[] d;
    delete[] a;
    delete[] ai;
    delete[] u;
}


void rss_sqrt_inv(Lint *c, Lint *e, uint size, uint ring_size) {

    Lint c1, c2, temp, d_;
    uint i, j;

    for (i = 0; i < size; i++) {
        c1 = Lint(1);
        c2 = Lint(1);
        d_ = Lint(4); // 100 - the first mask

        for (j = 2; j < ring_size - 1; j++) {
            temp = e[i] - (c1) * (c1);
            if (temp != Lint(0)) {
                // get the jth+1 bit of temp, place it in jth position, and add to c1
                c1 += (temp & (d_ << Lint(1))) >> Lint(1);
            }

            temp = Lint(1) - c1 * c2;
            // get the jth bit of temp and add it to c2
            c2 += temp & d_;
            d_ = d_ << Lint(1);
        }
        // last round for the inv portion
        temp = Lint(1) - c1 * c2;
        c[i] = c2 + (temp & d_);
    }
}


void Rss_RandBit_5pc(Lint **b, uint size, uint ring_size, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint i;
    uint bytes = (ring_size + 9) >> 3;
    // printf("bytes : %llu\n", bytes );
    uint numShares = nodeNet->getNumShares();

    Lint **u = new Lint *[numShares];
    Lint **a = new Lint *[numShares];
    Lint **d = new Lint *[numShares];

    for (i = 0; i < numShares; i++) {
        u[i] = new Lint[size];
        a[i] = new Lint[size];
        d[i] = new Lint[size];
    }
    Lint *e = new Lint[size];
    Lint *c = new Lint[size];
    uint8_t *buffer = new uint8_t[bytes * size];

    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
    if (pid == 1) {
        ai[0] = 1;
    } else if (pid == 4) {
        ai[5] = 1; // party 4's share 6
    } else if (pid == 5) {
        ai[3] = 1; // parthy 5's share 4
    } 

    for (size_t j = 0; j < numShares; j++) {
        nodeNet->prg_getrandom(j, bytes, size, buffer);
        for (i = 0; i < size; i++) {
            memcpy(u[j] + i, buffer + i * bytes, bytes);
        }
    }


    for (i = 0; i < size; i++) {
        // ensuring [a] is odd
        for (size_t s = 0; s < numShares; s++)
            a[s][i] = (u[s][i] << Lint(1)) + ai[s];
        // a[1][i] = (u[1][i] << Lint(1)) + a2;
    }

    // squaring a
    Rss_MultPub_5pc(e, a, a, size, ring_size + 2, nodeNet); // ringsize+2
    rss_sqrt_inv(c, e, size, ring_size + 2);

    // effectively combines the two loops into one, eliminates d variable
    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            b[s][i] = (c[i] * a[s][i] + ai[s]) >> Lint(1);
        // b[1][i] = (c[i] * a[1][i] + a2) >> (1);
    }

    // freeing up
    delete[] c;
    delete[] buffer;
    delete[] e;
    for (i = 0; i < numShares; i++) {
        delete[] d[i];
        delete[] a[i];
        delete[] u[i];
    }
    delete[] ai;
    delete[] d;
    delete[] a;
    delete[] u;
}


void Rss_RandBit_7pc(Lint **b, uint size, uint ring_size, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint i;
    uint bytes = (ring_size + 9) >> 3;
    // printf("bytes : %u\n", bytes );
    // printf("size : %u\n", size );
    uint numShares = nodeNet->getNumShares();

    Lint **u = new Lint *[numShares];
    Lint **a = new Lint *[numShares];
    Lint **d = new Lint *[numShares];

    for (i = 0; i < numShares; i++) {
        u[i] = new Lint[size];
        a[i] = new Lint[size];
        d[i] = new Lint[size];
    }
    Lint *e = new Lint[size];
    Lint *c = new Lint[size];
    uint8_t *buffer = new uint8_t[bytes * size];

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

    for (size_t j = 0; j < numShares; j++) {
        nodeNet->prg_getrandom(j, bytes, size, buffer);
        for (i = 0; i < size; i++) {
            memcpy(u[j] + i, buffer + i * bytes, bytes);
        }
    }


    for (i = 0; i < size; i++) {
        // ensuring [a] is odd
        for (size_t s = 0; s < numShares; s++)
            a[s][i] = (u[s][i] << Lint(1)) + ai[s];
        // a[1][i] = (u[1][i] << Lint(1)) + a2;
    }

    // printf("Rss_MultPub_7pc\n");
    // squaring a
    Rss_MultPub_7pc(e, a, a, size, ring_size + 2, nodeNet); // ringsize+2
    // printf("Rss_MultPub_7pc end\n");
    rss_sqrt_inv(c, e, size, ring_size + 2);

    // effectively combines the two loops into one, eliminates d variable
    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            b[s][i] = (c[i] * a[s][i] + ai[s]) >> Lint(1);
        // b[1][i] = (c[i] * a[1][i] + a2) >> (1);
    }

    // freeing up
    delete[] c;
    delete[] buffer;
    delete[] e;
    for (i = 0; i < numShares; i++) {
        delete[] d[i];
        delete[] a[i];
        delete[] u[i];
    }
    delete[] ai;
    delete[] d;
    delete[] a;
    delete[] u;
}
