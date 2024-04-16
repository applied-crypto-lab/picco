#include "EQZ.h"

void Rss_EQZ(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();
    uint i; // used for loops

    Lint **sum = new Lint *[numShares];
    Lint **edaBit_r = new Lint *[numShares];
    Lint **edaBit_b_2 = new Lint *[numShares];
    Lint **rprime = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        edaBit_r[i] = new Lint[size];
        edaBit_b_2[i] = new Lint[size];
        sum[i] = new Lint[size];
        rprime[i] = new Lint[size];


    }
    Lint *res_check = new Lint[size];

    Lint *c = new Lint[size];

    Lint *ai = new Lint[numShares];

    memset(ai, 0, sizeof(Lint) * numShares);
    if (pid == 1) {
        ai[0] = -1; // party 1's share 1
    } else if (pid == 3) {
        ai[1] = -1; // party 3's share 2
    }

    edaBit(edaBit_r, edaBit_b_2, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            // rprime[s][i] = edaBit_r[s][i] - (GET_BIT(edaBit_b_2[s][i], Lint(ring_size - 1)) << Lint(ring_size)); //original version from new primitives
            rprime[s][i] = edaBit_r[s][i] ;
            sum[s][i] = (a[s][i] + rprime[s][i]);
        }
    }
    Rss_Open(c, sum, size, ring_size, nodeNet);
    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            rprime[s][i] = (c[i] & ai[s]) ^ edaBit_b_2[s][i];
        }
    }

    

    // Rss_Open_Bitwise(res_check, rprime, size, ring_size, nodeNet);
    // for (uint i = 0; i < size; i++) {
    //     printf("(no mask ) res_check[%u]: %llu\n", i, res_check[i]);
    // }

    // for (i = 0; i < size; i++) {
    //     for (size_t s = 0; s < numShares; s++) {
    //         rprime[s][i] = rprime[s][i] & nodeNet->SHIFT[ring_size];
    //     }
    // }
    // Rss_Open_Bitwise(res_check, rprime, size, ring_size, nodeNet);

    // for (uint i = 0; i < size; i++) {
    //     printf("(  mask  ) res_check[%u]: %llu\n", i, res_check[i]);
    // }


    Rss_k_OR_L(res, rprime, size, ring_size, nodeNet);
    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            res[s][i] = 1 ^ res[s][i]; // CHECK THIS
        }
    }
    Rss_b2a(res, res, ring_size, size, nodeNet);

    delete[] c;
    delete[] ai;
    delete[] res_check;

    for (i = 0; i < numShares; i++) {
        delete[] edaBit_r[i];
        delete[] edaBit_b_2[i];
        delete[] sum[i];
        delete[] rprime[i];
    }
    delete[] edaBit_r;
    delete[] edaBit_b_2;
    delete[] sum;
    delete[] rprime;
}

void Rss_k_OR_L(Lint **res, Lint **r, uint size, uint ring_size, NodeNetwork *nodeNet) {
    uint numShares = nodeNet->getNumShares();
    uint i, s, originial_num, num, r_size, n_uints, new_r_size;
    for (i = 0; i < size; i++) {
        // removing any leading bits that could interfere with future masking
        // this is fine since shares are bitwise and won't affect the final result
        for (s = 0; s < numShares; s++) {
            r[s][i] = r[s][i] & nodeNet->SHIFT[ring_size];
        }
    }

    Lint **temp_res = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        temp_res[i] = new Lint[size];
    }

    r_size = ring_size;

    originial_num = (((r_size >> 1) + 7) >> 3) * size;

    uint8_t **a = new uint8_t *[numShares];
    uint8_t **b = new uint8_t *[numShares];
    uint8_t **u = new uint8_t *[numShares];

    for (i = 0; i < numShares; i++) {
        a[i] = new uint8_t[originial_num];
        memset(a[i], 0, sizeof(uint8_t) * originial_num);
        b[i] = new uint8_t[originial_num];
        memset(b[i], 0, sizeof(uint8_t) * originial_num);
        u[i] = new uint8_t[originial_num];
        memset(u[i], 0, sizeof(uint8_t) * originial_num);
    }
    Lint temp1, temp2, msk;

    while (r_size > 1) {

        new_r_size = r_size >> 1; // dividing by two because we only need half as many uuint8_t's in the first (and subsequent rounds)

        n_uints = ((new_r_size + 7) >> 3);
        num = ((new_r_size + 7) >> 3) * size;

        msk = (Lint(1) << new_r_size) - Lint(1);

        for (i = 0; i < size; ++i) {
            for (s = 0; s < numShares; s++) {
                temp1 = r[s][i] & msk;
                temp2 = (r[s][i] & (msk << new_r_size)) >> new_r_size;

                memcpy(a[s] + i * n_uints, &temp1, n_uints);
                memcpy(b[s] + i * n_uints, &temp2, n_uints);
            }
        }
        
        Rss_Mult_Byte(u, a, b, num, nodeNet);

        for (i = 0; i < num; ++i) {
            for (s = 0; s < numShares; s++) {
                u[s][i] = a[s][i] ^ b[s][i] ^ u[s][i];
            }
        }

        for (i = 0; i < size; ++i) {
            for (s = 0; s < numShares; s++) {
                memcpy(temp_res[s] + i, u[s] + i * n_uints, n_uints);
                temp_res[s][i] = temp_res[s][i] ;
            }
        }

        // need to move the unused bit forward
        if ((r_size & 1)) {
            for (i = 0; i < size; ++i) {
                for (s = 0; s < numShares; s++) {
                    temp_res[s][i] = SET_BIT(temp_res[s][i], Lint(new_r_size), GET_BIT(r[s][i], Lint(r_size - 1)));
                }
            }
            new_r_size += 1;
        }

        for (i = 0; i < size; ++i) {
            for (s = 0; s < numShares; s++) {
                r[s][i] = temp_res[s][i] & nodeNet->SHIFT[new_r_size]; // santizing leading > new_ring_size bits for next round
            }
        }

        r_size = new_r_size;
    }

    for (i = 0; i < size; ++i) {
        for (s = 0; s < numShares; s++) {
            res[s][i] = r[s][i];
        }
    }
    for (i = 0; i < numShares; i++) {

        delete[] a[i];
        delete[] b[i];
        delete[] u[i];
        delete[] temp_res[i];
    }

    delete[] a;
    delete[] b;
    delete[] u;
    delete[] temp_res;
}


void Rss_EQZ_5pc(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();
    uint i; // used for loops

    Lint **sum = new Lint *[numShares];
    Lint **edaBit_r = new Lint *[numShares];
    Lint **edaBit_b_2 = new Lint *[numShares];
    Lint **rprime = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        edaBit_r[i] = new Lint[size];
        edaBit_b_2[i] = new Lint[size];
        sum[i] = new Lint[size];
        rprime[i] = new Lint[size];
    }
    Lint *res_check = new Lint[size];

    Lint *c = new Lint[size];

    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
    if (pid == 1) {
        ai[0] = 1;
    } else if (pid == 4) {
        ai[5] = 1; // party 4's share 6
    } else if (pid == 5) {
        ai[3] = 1; // parthy 5's share 4
    }
    

    Rss_edaBit_5pc(edaBit_r, edaBit_b_2, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            rprime[s][i] = edaBit_r[s][i] - (GET_BIT(edaBit_b_2[s][i], Lint(ring_size - 1)) << Lint(ring_size));
            sum[s][i] = (a[s][i] + rprime[s][i]);
        }
    }
    Rss_Open_5pc(c, sum, size, ring_size, nodeNet);
    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            rprime[s][i] = (c[i] & ai[s]) ^ edaBit_b_2[s][i];
        }
    }

    Rss_k_OR_L_5pc(res, rprime, size, ring_size, nodeNet);
    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            res[s][i] = 1 ^ res[s][i]; // CHECK THIS
        }
    }
    Rss_b2a_5pc(res, res, ring_size, size, nodeNet);

    delete[] c;
    delete[] ai;
    delete[] res_check;

    for (i = 0; i < numShares; i++) {
        delete[] edaBit_r[i];
        delete[] edaBit_b_2[i];
        delete[] sum[i];
        delete[] rprime[i];
    }
    delete[] edaBit_r;
    delete[] edaBit_b_2;
    delete[] sum;
    delete[] rprime;
}

void Rss_k_OR_L_5pc(Lint **res, Lint **r, uint size, uint ring_size, NodeNetwork *nodeNet) {
    uint numShares = nodeNet->getNumShares();
    uint i, s, originial_num, num, r_size, n_uints, new_r_size;
    for (i = 0; i < size; i++) {
        // removing any leading bits that could interfere with future masking
        // this is fine since shares are bitwise and won't affect the final result
        for (s = 0; s < numShares; s++) {
            r[s][i] = r[s][i] & nodeNet->SHIFT[ring_size];
        }
    }

    Lint **temp_res = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        temp_res[i] = new Lint[size];
    }

    r_size = ring_size;

    originial_num = (((r_size >> 1) + 7) >> 3) * size;

    uint8_t **a = new uint8_t *[numShares];
    uint8_t **b = new uint8_t *[numShares];
    uint8_t **u = new uint8_t *[numShares];

    for (i = 0; i < numShares; i++) {
        a[i] = new uint8_t[originial_num];
        memset(a[i], 0, sizeof(uint8_t) * originial_num);
        b[i] = new uint8_t[originial_num];
        memset(b[i], 0, sizeof(uint8_t) * originial_num);
        u[i] = new uint8_t[originial_num];
        memset(u[i], 0, sizeof(uint8_t) * originial_num);
    }
    Lint temp1, temp2, msk;

    while (r_size > 1) {

        new_r_size = r_size >> 1; // dividing by two because we only need half as many uuint8_t's in the first (and subsequent rounds)

        n_uints = ((new_r_size + 7) >> 3);
        num = ((new_r_size + 7) >> 3) * size;

        msk = (Lint(1) << new_r_size) - Lint(1);

        for (i = 0; i < size; ++i) {
            for (s = 0; s < numShares; s++) {
                temp1 = r[s][i] & msk;
                temp2 = (r[s][i] & (msk << new_r_size)) >> new_r_size;

                memcpy(a[s] + i * n_uints, &temp1, n_uints);
                memcpy(b[s] + i * n_uints, &temp2, n_uints);
            }
        }
        
        Rss_Mult_Byte_5pc(u, a, b, num, nodeNet);

        for (i = 0; i < num; ++i) {
            for (s = 0; s < numShares; s++) {
                u[s][i] = a[s][i] ^ b[s][i] ^ u[s][i];
            }
        }

        for (i = 0; i < size; ++i) {
            for (s = 0; s < numShares; s++) {
                memcpy(temp_res[s] + i, u[s] + i * n_uints, n_uints);
                temp_res[s][i] = temp_res[s][i] ;
            }
        }

        // need to move the unused bit forward
        if ((r_size & 1)) {
            for (i = 0; i < size; ++i) {
                for (s = 0; s < numShares; s++) {
                    temp_res[s][i] = SET_BIT(temp_res[s][i], Lint(new_r_size), GET_BIT(r[s][i], Lint(r_size - 1)));
                }
            }
            new_r_size += 1;
        }

        for (i = 0; i < size; ++i) {
            for (s = 0; s < numShares; s++) {
                r[s][i] = temp_res[s][i] & nodeNet->SHIFT[new_r_size]; // santizing leading > new_ring_size bits for next round
            }
        }

        r_size = new_r_size;
    }

    for (i = 0; i < size; ++i) {
        for (s = 0; s < numShares; s++) {
            res[s][i] = r[s][i];
        }
    }
    for (i = 0; i < numShares; i++) {

        delete[] a[i];
        delete[] b[i];
        delete[] u[i];
        delete[] temp_res[i];
    }

    delete[] a;
    delete[] b;
    delete[] u;
    delete[] temp_res;
}
