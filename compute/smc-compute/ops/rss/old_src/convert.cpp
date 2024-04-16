#include "convert.h"

// converts shares of a_k to shares of a_k'
// k --> ring_size
// k' --> ring_size_prime
void new_Rss_Convert(Lint **res, Lint **a, uint size, uint ring_size, uint ring_size_prime, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint i, j, k, index; // used for loops
    uint numShares = nodeNet->getNumShares();

    uint n_rand_bits = size * ring_size;
    // first size*ring_size random bits used for first part of share conversion
    // 2nd half used for 2tok'

    Lint **r_shares = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        r_shares[i] = new Lint[n_rand_bits];
    }

    Lint **edaBit_r = new Lint *[numShares];
    Lint **edaBit_b_2 = new Lint *[numShares];
    Lint **r_2 = new Lint *[numShares];
    Lint **sum = new Lint *[numShares];
    Lint **a_2 = new Lint *[numShares];

    // will hold the k shares of a in (k_prime)
    Lint **a_k_prime = new Lint *[numShares];
    Lint *c = new Lint[size];

    Lint *res_check = new Lint[size];

    for (i = 0; i < numShares; i++) {
        r_2[i] = new Lint[size];
        a_2[i] = new Lint[size];
        sum[i] = new Lint[size];
        a_k_prime[i] = new Lint[n_rand_bits];

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

    // used for B2A component
    Rss_RandBit(r_shares, n_rand_bits, ring_size_prime, nodeNet);

    // only need ring_size bit-length values for both parts of computation
    // first protection and b2a
    edaBit(edaBit_r, edaBit_b_2, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            sum[s][i] = (a[s][i] - edaBit_r[s][i]);
        }
        // sum[1][i] = (a[1][i] - edaBit_r[1][i]);
    }

    // Rss_Open(c, sum, size, ring_size_prime, nodeNet);
    Rss_Open(c, sum, size, ring_size, nodeNet);

    Rss_BitAdd(a_2, c, edaBit_b_2, ring_size, size, nodeNet);
    // Rss_BitAdd(res, c, edaBit_b_2, ring_size, size, nodeNet);

    // 2tok' component (B2A)
    // a_2 is k-bits long

    for (j = 0; j < size; j++) {
        for (k = 0; k < ring_size; k++) {
            index = j * ring_size + k;
            for (size_t s = 0; s < numShares; s++) {
                r_2[s][j] = Lint(SET_BIT(r_2[s][j], Lint(k), GET_BIT(r_shares[s][index], Lint(0))));
            }
            // r_2[0][j] = Lint(SET_BIT(r_2[0][j], Lint(k), GET_BIT(r_shares[0][index], Lint(0))));
            // r_2[1][j] = Lint(SET_BIT(r_2[1][j], Lint(k), GET_BIT(r_shares[1][index], Lint(0))));
        }
    }

    // 2tok' component
    // a_2 is k-bits long

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            sum[s][i] = (a_2[s][i] ^ r_2[s][i]);
        // sum[1][i] = (a_2[1][i] ^ r_2[1][i]);
    }

    // should this be bitwise-open? (like Open_Byte)
    // and what should the ring_size be?
    // memset(c, 0, sizeof(Lint) * size); // clearing c
    Rss_Open_Bitwise(c, sum, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (j = 0; j < ring_size; j++) {
            index = i * ring_size + j;
            for (size_t s = 0; s < numShares; s++) {
                a_k_prime[s][index] = ai[s] * GET_BIT(c[i], Lint(j)) + r_shares[s][index] - 2 * GET_BIT(c[i], Lint(j)) * r_shares[s][index];
            }
            // a_k_prime[1][index] = a2 * GET_BIT(c[i], Lint(j)) + r_shares[1][index] - 2 * GET_BIT(c[i], Lint(j)) * r_shares[1][index];
        }
    }

    // sanitizing res
    for (i = 0; i < numShares; i++) {
        memset(res[i], 0, sizeof(Lint) * size);
    }
    // constructing the final shares of a
    for (j = 0; j < size; j++) {
        // res[0][j] = 0;
        // res[1][j] = 0;

        for (k = 0; k < ring_size; k++) {
            // this is for step 3
            index = j * ring_size + k;
            for (size_t s = 0; s < numShares; s++) {
                res[s][j] += (a_k_prime[s][index] << Lint(k));
            }
            // res[1][j] += (a_k_prime[1][index] << Lint(k));
        }
    }

    delete[] ai;
    delete[] c;
    delete[] res_check;

    for (i = 0; i < numShares; i++) {
        delete[] r_shares[i];
        delete[] edaBit_r[i];
        delete[] edaBit_b_2[i];
        delete[] sum[i];
        delete[] r_2[i];
        delete[] a_2[i];
        delete[] a_k_prime[i];
    }
    delete[] r_shares;
    delete[] edaBit_r;
    delete[] edaBit_b_2;
    delete[] sum;
    delete[] r_2;
    delete[] a_2;
    delete[] a_k_prime;
}

// Not updating because it's outdated
void Rss_Convert(Lint **res, Lint **a, uint size, uint ring_size, uint ring_size_prime, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();
    uint i, j, k, index; // used for loops

    uint n_rand_bits = 2 * size * (ring_size);
    // first size*ring_size random bits used for first part of share conversion
    // 2nd half used for 2tok'
    Lint a1 = 0, a2 = 0;
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

    Lint **r_shares = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        r_shares[i] = new Lint[n_rand_bits];
    }
    // generating shares in k' since we will be using them at a later point in the computation
    Rss_RandBit(r_shares, n_rand_bits, ring_size_prime, nodeNet);
    // Rss_RandBit(r_shares, n_rand_bits, ring_size, nodeNet);
    Lint **r = new Lint *[numShares];
    Lint **r_2 = new Lint *[numShares];
    Lint **sum = new Lint *[numShares];
    Lint **a_2 = new Lint *[numShares];

    // will hold the k shares of a in (k_prime)
    Lint **a_k_prime = new Lint *[numShares];

    Lint *c = new Lint[size];
    memset(c, 0, sizeof(Lint) * size);

    Lint *res_check = new Lint[size];
    // memset(res_check, 0, sizeof(Lint) * size);

    for (i = 0; i < numShares; i++) {
        r[i] = new Lint[size];
        memset(r[i], 0, sizeof(Lint) * size);
        r_2[i] = new Lint[size];
        a_2[i] = new Lint[size];
        sum[i] = new Lint[size];
        a_k_prime[i] = new Lint[(n_rand_bits >> 1)];
    }

    for (j = 0; j < size; j++) {
        for (k = 0; k < ring_size; k++) {
            index = j * ring_size + k;
            r[0][j] = r[0][j] + (r_shares[0][index] << Lint(k));
            r[1][j] = r[1][j] + (r_shares[1][index] << Lint(k));
        }
    }
    for (j = 0; j < size; j++) {
        for (k = 0; k < ring_size; k++) {
            index = j * ring_size + k;

            r_2[0][j] = Lint(SET_BIT(r_2[0][j], Lint(k), GET_BIT(r_shares[0][index], Lint(0))));
            r_2[1][j] = Lint(SET_BIT(r_2[1][j], Lint(k), GET_BIT(r_shares[1][index], Lint(0))));
        }
    }

    for (i = 0; i < size; i++) {
        sum[0][i] = (a[0][i] - r[0][i]);
        sum[1][i] = (a[1][i] - r[1][i]);
    }

    // Rss_Open(c, sum, size, ring_size_prime, nodeNet);
    Rss_Open(c, sum, size, ring_size, nodeNet);

    Rss_BitAdd(a_2, c, r_2, ring_size, size, nodeNet);

    // Rss_Open_Bitwise(res_check, a_2, size, ring_size, nodeNet);
    // for (i = 0; i < size; i++) {
    // 	// printf("res_check[%i] : %llu\n", i, res_check[i] & nodeNet->SHIFT[ring_size]);
    // }

    // FIX the r_2
    // resetting r_2 with fresh random bits
    for (i = 0; i < numShares; i++) {
        memset(r_2[i], 0, sizeof(Lint) * size);
    }

    for (j = 0; j < size; j++) {
        for (k = 0; k < ring_size; k++) {
            index = j * ring_size + k;

            r_2[0][j] = Lint(SET_BIT(r_2[0][j], Lint(k), GET_BIT(r_shares[0][size * ring_size + index], Lint(0))));
            r_2[1][j] = Lint(SET_BIT(r_2[1][j], Lint(k), GET_BIT(r_shares[1][size * ring_size + index], Lint(0))));
        }
    }

    // 2tok' component
    // a_2 is k-bits long

    for (i = 0; i < size; i++) {
        sum[0][i] = (a_2[0][i] ^ r_2[0][i]);
        sum[1][i] = (a_2[1][i] ^ r_2[1][i]);
    }

    // should this be bitwise-open? (like Open_Byte)
    // and what should the ring_size be?
    memset(c, 0, sizeof(Lint) * size); // clearing c
    Rss_Open_Bitwise(c, sum, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (j = 0; j < ring_size; j++) {
            index = i * ring_size + j;

            a_k_prime[0][index] = a1 * GET_BIT(c[i], Lint(j)) + r_shares[0][size * ring_size + index] - 2 * GET_BIT(c[i], Lint(j)) * r_shares[0][size * ring_size + index];
            a_k_prime[1][index] = a2 * GET_BIT(c[i], Lint(j)) + r_shares[1][size * ring_size + index] - 2 * GET_BIT(c[i], Lint(j)) * r_shares[1][size * ring_size + index];
        }
    }

    // constructing the final shares of a
    for (j = 0; j < size; j++) {

        res[0][j] = 0, res[1][j] = 0;
        // using temps so we don't have to worry about res being cleared1

        for (k = 0; k < ring_size; k++) {
            // this is for step 3
            index = j * ring_size + k;

            res[0][j] += (a_k_prime[0][index] << Lint(k));
            res[1][j] += (a_k_prime[1][index] << Lint(k));
        }
    }

    delete[] c;
    delete[] res_check;

    for (i = 0; i < numShares; i++) {
        delete[] r_shares[i];
        delete[] r[i];
        delete[] sum[i];
        delete[] r_2[i];
        delete[] a_2[i];
        delete[] a_k_prime[i];
    }
    delete[] r_shares;
    delete[] r;
    delete[] sum;
    delete[] r_2;
    delete[] a_2;
    delete[] a_k_prime;
}


// converts shares of a_k to shares of a_k'
// k --> ring_size
// k' --> ring_size_prime
void new_Rss_Convert_5pc(Lint **res, Lint **a, uint size, uint ring_size, uint ring_size_prime, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint i, j, k, index; // used for loops
    uint numShares = nodeNet->getNumShares();

    uint n_rand_bits = size * ring_size;
    // first size*ring_size random bits used for first part of share conversion
    // 2nd half used for 2tok'

    Lint **r_shares = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        r_shares[i] = new Lint[n_rand_bits];
    }
    Lint **edaBit_r = new Lint *[numShares];
    Lint **edaBit_b_2 = new Lint *[numShares];
    Lint **r_2 = new Lint *[numShares];
    Lint **sum = new Lint *[numShares];
    Lint **a_2 = new Lint *[numShares];

    // will hold the k shares of a in (k_prime)
    Lint **a_k_prime = new Lint *[numShares];
    Lint *c = new Lint[size];

    Lint *res_check = new Lint[size];

    for (i = 0; i < numShares; i++) {
        r_2[i] = new Lint[size];
        a_2[i] = new Lint[size];
        sum[i] = new Lint[size];
        a_k_prime[i] = new Lint[n_rand_bits];

        edaBit_r[i] = new Lint[size];
        edaBit_b_2[i] = new Lint[size];
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

    // used for B2A component
    Rss_RandBit_5pc(r_shares, n_rand_bits, ring_size_prime, nodeNet);

    // only need ring_size bit-length values for both parts of computation
    // first protection and b2a
    Rss_edaBit_5pc(edaBit_r, edaBit_b_2, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            sum[s][i] = (a[s][i] - edaBit_r[s][i]);
        }
        // sum[1][i] = (a[1][i] - edaBit_r[1][i]);
    }

    // Rss_Open_5pc(c, sum, size, ring_size_prime, nodeNet);
    Rss_Open_5pc(c, sum, size, ring_size, nodeNet);

    Rss_BitAdd_5pc(a_2, c, edaBit_b_2, ring_size, size, nodeNet);
    // Rss_BitAdd_5pc(res, c, edaBit_b_2, ring_size, size, nodeNet);

    // 2tok' component (B2A)
    // a_2 is k-bits long

    for (j = 0; j < size; j++) {
        for (k = 0; k < ring_size; k++) {
            index = j * ring_size + k;
            for (size_t s = 0; s < numShares; s++) {
                r_2[s][j] = Lint(SET_BIT(r_2[s][j], Lint(k), GET_BIT(r_shares[s][index], Lint(0))));
            }
            // r_2[0][j] = Lint(SET_BIT(r_2[0][j], Lint(k), GET_BIT(r_shares[0][index], Lint(0))));
            // r_2[1][j] = Lint(SET_BIT(r_2[1][j], Lint(k), GET_BIT(r_shares[1][index], Lint(0))));
        }
    }

    // 2tok' component
    // a_2 is k-bits long

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            sum[s][i] = (a_2[s][i] ^ r_2[s][i]);
        // sum[1][i] = (a_2[1][i] ^ r_2[1][i]);
    }

    // should this be bitwise-open? (like Open_Byte)
    // and what should the ring_size be?
    // memset(c, 0, sizeof(Lint) * size); // clearing c
    Rss_Open_Bitwise_5pc(c, sum, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (j = 0; j < ring_size; j++) {
            index = i * ring_size + j;
            for (size_t s = 0; s < numShares; s++) {
                a_k_prime[s][index] = ai[s] * GET_BIT(c[i], Lint(j)) + r_shares[s][index] - 2 * GET_BIT(c[i], Lint(j)) * r_shares[s][index];
            }
            // a_k_prime[1][index] = a2 * GET_BIT(c[i], Lint(j)) + r_shares[1][index] - 2 * GET_BIT(c[i], Lint(j)) * r_shares[1][index];
        }
    }

    // sanitizing res
    for (i = 0; i < numShares; i++) {
        memset(res[i], 0, sizeof(Lint) * size);
    }
    // constructing the final shares of a
    for (j = 0; j < size; j++) {
        // res[0][j] = 0;
        // res[1][j] = 0;

        for (k = 0; k < ring_size; k++) {
            // this is for step 3
            index = j * ring_size + k;
            for (size_t s = 0; s < numShares; s++) {
                res[s][j] += (a_k_prime[s][index] << Lint(k));
            }
            // res[1][j] += (a_k_prime[1][index] << Lint(k));
        }
    }

    delete[] ai;
    delete[] c;
    delete[] res_check;

    for (i = 0; i < numShares; i++) {
        delete[] r_shares[i];
        delete[] edaBit_r[i];
        delete[] edaBit_b_2[i];
        delete[] sum[i];
        delete[] r_2[i];
        delete[] a_2[i];
        delete[] a_k_prime[i];
    }
    delete[] r_shares;
    delete[] edaBit_r;
    delete[] edaBit_b_2;
    delete[] sum;
    delete[] r_2;
    delete[] a_2;
    delete[] a_k_prime;
}

// Not updating because it's outdated
void Rss_Convert_5pc(Lint **res, Lint **a, uint size, uint ring_size, uint ring_size_prime, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();
    uint i, j, k, index; // used for loops

    uint n_rand_bits = 2 * size * (ring_size);

    Lint **r_shares = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        r_shares[i] = new Lint[n_rand_bits];
    }
    Lint **r = new Lint *[numShares];
    Lint **r_2 = new Lint *[numShares];
    Lint **sum = new Lint *[numShares];
    Lint **a_2 = new Lint *[numShares];

    // will hold the k shares of a in (k_prime)
    Lint **a_k_prime = new Lint *[numShares];

    Lint *c = new Lint[size];
    memset(c, 0, sizeof(Lint) * size);

    Lint *res_check = new Lint[size];
    // memset(res_check, 0, sizeof(Lint) * size);

    for (i = 0; i < numShares; i++) {
        r[i] = new Lint[size];
        memset(r[i], 0, sizeof(Lint) * size);
        r_2[i] = new Lint[size];
        a_2[i] = new Lint[size];
        sum[i] = new Lint[size];
        a_k_prime[i] = new Lint[(n_rand_bits >> 1)];
    }

    // first size*ring_size random bits used for first part of share conversion
    // 2nd half used for 2tok'
    Lint a1 = 0, a2 = 0;
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

    // generating shares in k' since we will be using them at a later point in the computation
    Rss_RandBit_5pc(r_shares, n_rand_bits, ring_size_prime, nodeNet);
    // Rss_RandBit_5pc(r_shares, n_rand_bits, ring_size, nodeNet);

    for (j = 0; j < size; j++) {
        for (k = 0; k < ring_size; k++) {
            index = j * ring_size + k;
            r[0][j] = r[0][j] + (r_shares[0][index] << Lint(k));
            r[1][j] = r[1][j] + (r_shares[1][index] << Lint(k));
        }
    }
    for (j = 0; j < size; j++) {
        for (k = 0; k < ring_size; k++) {
            index = j * ring_size + k;

            r_2[0][j] = Lint(SET_BIT(r_2[0][j], Lint(k), GET_BIT(r_shares[0][index], Lint(0))));
            r_2[1][j] = Lint(SET_BIT(r_2[1][j], Lint(k), GET_BIT(r_shares[1][index], Lint(0))));
        }
    }

    for (i = 0; i < size; i++) {
        sum[0][i] = (a[0][i] - r[0][i]);
        sum[1][i] = (a[1][i] - r[1][i]);
    }

    // Rss_Open_5pc(c, sum, size, ring_size_prime, nodeNet);
    Rss_Open_5pc(c, sum, size, ring_size, nodeNet);

    Rss_BitAdd_5pc(a_2, c, r_2, ring_size, size, nodeNet);

    // Rss_Open_Bitwise_5pc(res_check, a_2, size, ring_size, nodeNet);
    // for (i = 0; i < size; i++) {
    // 	// printf("res_check[%i] : %llu\n", i, res_check[i] & nodeNet->SHIFT[ring_size]);
    // }

    // FIX the r_2
    // resetting r_2 with fresh random bits
    for (i = 0; i < numShares; i++) {
        memset(r_2[i], 0, sizeof(Lint) * size);
    }

    for (j = 0; j < size; j++) {
        for (k = 0; k < ring_size; k++) {
            index = j * ring_size + k;

            r_2[0][j] = Lint(SET_BIT(r_2[0][j], Lint(k), GET_BIT(r_shares[0][size * ring_size + index], Lint(0))));
            r_2[1][j] = Lint(SET_BIT(r_2[1][j], Lint(k), GET_BIT(r_shares[1][size * ring_size + index], Lint(0))));
        }
    }

    // 2tok' component
    // a_2 is k-bits long

    for (i = 0; i < size; i++) {
        sum[0][i] = (a_2[0][i] ^ r_2[0][i]);
        sum[1][i] = (a_2[1][i] ^ r_2[1][i]);
    }

    // should this be bitwise-open? (like Open_Byte)
    // and what should the ring_size be?
    memset(c, 0, sizeof(Lint) * size); // clearing c
    Rss_Open_Bitwise_5pc(c, sum, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (j = 0; j < ring_size; j++) {
            index = i * ring_size + j;

            a_k_prime[0][index] = a1 * GET_BIT(c[i], Lint(j)) + r_shares[0][size * ring_size + index] - 2 * GET_BIT(c[i], Lint(j)) * r_shares[0][size * ring_size + index];
            a_k_prime[1][index] = a2 * GET_BIT(c[i], Lint(j)) + r_shares[1][size * ring_size + index] - 2 * GET_BIT(c[i], Lint(j)) * r_shares[1][size * ring_size + index];
        }
    }

    // constructing the final shares of a
    for (j = 0; j < size; j++) {

        res[0][j] = 0, res[1][j] = 0;
        // using temps so we don't have to worry about res being cleared1

        for (k = 0; k < ring_size; k++) {
            // this is for step 3
            index = j * ring_size + k;

            res[0][j] += (a_k_prime[0][index] << Lint(k));
            res[1][j] += (a_k_prime[1][index] << Lint(k));
        }
    }

    delete[] c;
    delete[] res_check;

    for (i = 0; i < numShares; i++) {
        delete[] r_shares[i];
        delete[] r[i];
        delete[] sum[i];
        delete[] r_2[i];
        delete[] a_2[i];
        delete[] a_k_prime[i];
    }
    delete[] r_shares;
    delete[] r;
    delete[] sum;
    delete[] r_2;
    delete[] a_2;
    delete[] a_k_prime;
}