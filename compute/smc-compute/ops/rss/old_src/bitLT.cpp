#include "bitLT.h"

// returns a binary sharing res, where res = a ?< b (we pack 2 shares in a single bit
// the function expects a and b to be arrays of binary values
// a is a single value
// b is two single values of packed shares
void Rss_BitLT(Lint **res, Lint *a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint i, j; // used for loops

    Lint **b_prime = new Lint *[2];
    for (i = 0; i < 2; i++) {
        b_prime[i] = new Lint[size];
    }

    Lint b1, b2;
    switch (pid) {
    case 1:
        b1 = -1;
        b2 = 0;
        break;
    case 2:
        b1 = 0;
        b2 = 0;
        break;
    case 3:
        b1 = 0;
        b2 = -1;
        break;
    }
    for (j = 0; j < size; j++) {
        // step 1  -- flipping all the bits of bk
        // if switch value is zero, do nothing
        b_prime[0][j] = (b[0][j] ^ b1);
        b_prime[1][j] = (b[1][j] ^ b2);
    }

    Rss_CarryOut(res, a, b_prime, ring_size, size, nodeNet);

    // flipping on share of output
    for (j = 0; j < size; ++j) {
        res[0][j] = GET_BIT(res[0][j], 0) ^ GET_BIT(b1, 0);
        res[1][j] = GET_BIT(res[1][j], 0) ^ GET_BIT(b2, 0);
    }

    for (i = 0; i < 2; i++) {
        delete[] b_prime[i];
    }
    delete[] b_prime;
}

void Rss_CarryOut(Lint **res, Lint *a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet) {

    uint i;
    int pid = nodeNet->getID();

    // d is shared
    Lint **d = new Lint *[4];
    for (i = 0; i < 4; i++) {
        d[i] = new Lint[size];
        memset(d[i], 0, sizeof(Lint) * size);
    }

    Lint a1, a2;
    switch (pid) {
    case 1:
        a1 = -1;
        a2 = 0;
        break;
    case 2:
        a1 = 0;
        a2 = 0;
        break;
    case 3:
        a1 = 0;
        a2 = -1;
        break;
    }

    // initial step
    for (i = 0; i < size; i++) {
        // shares of p
        // changed from a + b - 2ab
        d[0][i] = (a[i] & a1) ^ b[0][i];
        d[1][i] = (a[i] & a2) ^ b[1][i];

        d[2][i] = (a[i] & b[0][i]);
        d[3][i] = (a[i] & b[1][i]);

        d[2][i] = SET_BIT(d[2][i], Lint(0), GET_BIT(d[2][i], Lint(0)) ^ (GET_BIT(d[0][i], Lint(0))));
        d[3][i] = SET_BIT(d[3][i], Lint(0), GET_BIT(d[3][i], Lint(0)) ^ (GET_BIT(d[1][i], Lint(0))));
    }

    // Rss_CarryOutAux(res, d, ring_size, size, nodeNet);
    new_Rss_CarryOutAux(res, d, ring_size, size, nodeNet);
    // Rss_CarryOutAux(res, d, ring_size, size, nodeNet);

    for (i = 0; i < 4; i++) {
        delete[] d[i];
    }
    delete[] d;
    // res is the actual return value of carry out, being [g] secret shared
}

void Rss_CarryOutAux(Lint **res, Lint **d, uint r_size, uint size, NodeNetwork *nodeNet) {
    uint i, j; // used for loops

    if (r_size > 1) {
        Lint **buffer = new Lint *[4];
        for (i = 0; i < 4; i++) {
            buffer[i] = new Lint[size];
            memset(buffer[i], 0, sizeof(Lint) * size);
        }
        // (r_size + 7) >> 3 calculates the ceiling
        uint num = ((r_size + 7) >> 3) * size;
        uint n_uints = ((r_size + 7) >> 3);
        // rounding down here (by default)
        uint new_r_size = r_size >> 1; // (r_size / 2)
        int t_index;
        // uint8_t utemp1, utemp2;
        Lint mask2, mask1m8, mask2m8;

        uint8_t **a = new uint8_t *[2];
        uint8_t **b = new uint8_t *[2];
        uint8_t **u = new uint8_t *[2];

        for (i = 0; i < 2; i++) {
            // memsets are actually needed here since are ORing
            a[i] = new uint8_t[num];
            memset(a[i], 0, sizeof(uint8_t) * num);
            b[i] = new uint8_t[num];
            memset(b[i], 0, sizeof(uint8_t) * num);
            u[i] = new uint8_t[num];
            memset(u[i], 0, sizeof(uint8_t) * num);
        }
        // preparing the buffer
        // CarryBuffer(buffer, d, size, new_r_size);
        OptimalBuffer(buffer, d, size, r_size, nodeNet);

        // Splitting the buffer into bytes
        for (i = 0; i < size; ++i) {
            memcpy(a[0] + i * n_uints, buffer[0] + i, n_uints);
            memcpy(a[1] + i * n_uints, buffer[1] + i, n_uints);
            memcpy(b[0] + i * n_uints, buffer[2] + i, n_uints);
            memcpy(b[1] + i * n_uints, buffer[3] + i, n_uints);
        }

        Rss_Mult_Byte(u, a, b, num, nodeNet);

        // clearing the buffer
        for (int i = 0; i < 4; i++) {
            memset(buffer[i], 0, sizeof(Lint) * size);
        }

        // printf("adding g2j+1\n");
        for (i = 0; i < size; ++i) {

            for (j = 0; j < new_r_size; ++j) {

                // loop constants
                t_index = (j >> 2) + (i * n_uints);
                mask2 = (2 * j + 1);
                mask2m8 = (2 * j + 1) & 7;
                mask1m8 = (2 * j) & 7; // "&7" = %8, used for leftover bits

                buffer[0][i] = SET_BIT(buffer[0][i], j, GET_BIT(u[0][t_index], mask1m8));
                buffer[1][i] = SET_BIT(buffer[1][i], j, GET_BIT(u[1][t_index], mask1m8));

                buffer[2][i] = SET_BIT(buffer[2][i], j, (GET_BIT(u[0][t_index], mask2m8) ^ GET_BIT(d[2][i], mask2)));
                buffer[3][i] = SET_BIT(buffer[3][i], j, (GET_BIT(u[1][t_index], mask2m8) ^ GET_BIT(d[3][i], mask2)));
            }
        }

        // checking if we have an unused (p,g) pair
        // if yes, move it to new_r_size's position
        // and update the new_r_size by 1
        if ((r_size & 1)) {
            // printf("moving unused\n");
            for (i = 0; i < size; ++i) {
                for (j = 0; j < 4; ++j) {
                    // getting the unused p (or g) from d and
                    // moving it to new_r_size + 1
                    buffer[j][i] = SET_BIT(buffer[j][i], Lint(new_r_size), GET_BIT(d[j][i], Lint(r_size - 1)));
                }
            }
            new_r_size += 1;
        }
        // recursive call
        Rss_CarryOutAux(res, buffer, new_r_size, size, nodeNet);

        for (i = 0; i < 2; i++) {

            delete[] a[i];
            delete[] b[i];
            delete[] u[i];
        }

        delete[] a;
        delete[] b;
        delete[] u;

        for (i = 0; i < 4; i++) {
            delete[] buffer[i];
        }
        delete[] buffer;

    } else {
        // hit the base case
        // this is what we're effectively returning
        for (i = 0; i < size; ++i) {
            res[0][i] = SET_BIT(res[0][i], 0, GET_BIT(d[2][i], 0));
            res[1][i] = SET_BIT(res[1][i], 0, GET_BIT(d[3][i], 0));
        }
    }
}

void CarryBuffer(Lint **buffer, Lint **d, uint size, uint r_size) {
    // prepares input u for multiplication
    // extracts p2i, p2i-1, and g2i
    // buffer and d are the same size (4 x size)

    Lint i, j, mask1, mask2;
    for (i = 0; i < size; ++i) {

        for (j = 0; j < r_size; ++j) {

            mask1 = Lint(2) * j;
            mask2 = Lint(2) * j + Lint(1);
            // getting the even bits of d
            buffer[0][i] = SET_BIT(buffer[0][i], mask1, GET_BIT(d[0][i], mask1));
            buffer[1][i] = SET_BIT(buffer[1][i], mask1, GET_BIT(d[1][i], mask1));
            buffer[0][i] = SET_BIT(buffer[0][i], mask2, GET_BIT(d[2][i], mask1));
            buffer[1][i] = SET_BIT(buffer[1][i], mask2, GET_BIT(d[3][i], mask1));

            // getting the odd bits of d
            buffer[2][i] = SET_BIT(buffer[2][i], mask1, GET_BIT(d[0][i], mask2));
            buffer[3][i] = SET_BIT(buffer[3][i], mask1, GET_BIT(d[1][i], mask2));
            buffer[2][i] = SET_BIT(buffer[2][i], mask2, GET_BIT(d[0][i], mask2));
            buffer[3][i] = SET_BIT(buffer[3][i], mask2, GET_BIT(d[1][i], mask2));
        }
    }
}

void OptimalBuffer(Lint **buffer, Lint **d, uint size, uint r_size, NodeNetwork *nodeNet) {
    // prepares input u for multiplication
    // buffer and d are the same size (4 x size)
    // reduction to a single loop, making complexity O(size), rather than O(size * r_size)

    Lint i, even, odd;
    // doing this once
    even = nodeNet->EVEN[r_size];
    odd = nodeNet->ODD[r_size];

    for (i = 0; i < size; ++i) {

        buffer[0][i] = buffer[0][i] | (d[0][i] & even);
        buffer[1][i] = buffer[1][i] | (d[1][i] & even);
        buffer[0][i] = buffer[0][i] | ((d[2][i] & even) << Lint(1));
        buffer[1][i] = buffer[1][i] | ((d[3][i] & even) << Lint(1));

        buffer[2][i] = buffer[2][i] | ((d[0][i] & odd) >> Lint(1));
        buffer[3][i] = buffer[3][i] | ((d[1][i] & odd) >> Lint(1));
        buffer[2][i] = buffer[2][i] | (d[0][i] & odd);
        buffer[3][i] = buffer[3][i] | (d[1][i] & odd);
    }
}

// a non-recursive solution to minimize memory consumption
void new_Rss_CarryOutAux(Lint **res, Lint **d, uint ring_size, uint size, NodeNetwork *nodeNet) {
    uint i, j, originial_num, num, n_uints, r_size, new_r_size, t_index;
    // uint8_t utemp1, utemp2;
    Lint mask2, mask1m8, mask2m8;
    r_size = ring_size;
    originial_num = ((r_size + 7) >> 3) * size;

    Lint **buffer = new Lint *[4];
    for (i = 0; i < 4; i++) {
        buffer[i] = new Lint[size];
        memset(buffer[i], 0, sizeof(Lint) * size);
    }

    uint8_t **a = new uint8_t *[2];
    uint8_t **b = new uint8_t *[2];
    uint8_t **u = new uint8_t *[2];

    //  need to do memsets every iteration
    for (i = 0; i < 2; i++) {
        // memsets are actually needed here since are ORing
        // tried w/o memsets - still working?
        a[i] = new uint8_t[originial_num];
        memset(a[i], 0, sizeof(uint8_t) * originial_num);
        b[i] = new uint8_t[originial_num];
        memset(b[i], 0, sizeof(uint8_t) * originial_num);
        u[i] = new uint8_t[originial_num];
        // memset(u[i], 0, sizeof(uint8_t) * originial_num);
    }

    while (r_size > 1) {
        num = ((r_size + 7) >> 3) * size;
        n_uints = ((r_size + 7) >> 3);
        // rounding down here (by default)
        new_r_size = r_size >> 1; // (r_size / 2)

        // preparing the buffer
        // CarryBuffer(buffer, d, size, new_r_size);
        // OptimalBuffer(buffer2, d, size, r_size, nodeNet);
        OptimalBuffer(buffer, d, size, r_size, nodeNet);

        // Splitting the buffer into bytes
        for (i = 0; i < size; ++i) {
            memcpy(a[0] + i * n_uints, buffer[0] + i, n_uints);
            memcpy(a[1] + i * n_uints, buffer[1] + i, n_uints);
            memcpy(b[0] + i * n_uints, buffer[2] + i, n_uints);
            memcpy(b[1] + i * n_uints, buffer[3] + i, n_uints);
        }

        Rss_Mult_Byte(u, a, b, num, nodeNet);

        // clearing the buffer
        for (int i = 0; i < 4; i++) {
            memset(buffer[i], 0, sizeof(Lint) * size);
        }

        // printf("adding g2j+1\n");
        for (i = 0; i < size; ++i) {
            for (j = 0; j < new_r_size; ++j) {
                // loop constants
                t_index = (j >> 2) + (i * n_uints);
                mask2 = (2 * j + 1);
                mask2m8 = (2 * j + 1) & 7;
                mask1m8 = (2 * j) & 7; // "&7" = %8, used for leftover bits

                buffer[0][i] = SET_BIT(buffer[0][i], j, GET_BIT(u[0][t_index], mask1m8));
                buffer[1][i] = SET_BIT(buffer[1][i], j, GET_BIT(u[1][t_index], mask1m8));

                buffer[2][i] = SET_BIT(buffer[2][i], j, (GET_BIT(u[0][t_index], mask2m8) ^ GET_BIT(d[2][i], mask2)));
                buffer[3][i] = SET_BIT(buffer[3][i], j, (GET_BIT(u[1][t_index], mask2m8) ^ GET_BIT(d[3][i], mask2)));
            }
        }

        // checking if we have an unused (p,g) pair
        // if yes, move it to new_r_size's position
        // and update the new_r_size by 1
        if ((r_size & 1)) {
            // printf("moving unused\n");
            for (i = 0; i < size; ++i) {
                for (j = 0; j < 4; ++j) {
                    // getting the unused p (or g) from d and
                    // moving it to new_r_size + 1
                    buffer[j][i] = SET_BIT(buffer[j][i], Lint(new_r_size), GET_BIT(d[j][i], Lint(r_size - 1)));
                }
            }
            new_r_size += 1;
        }
        // copying buffer back to d for next round
        for (i = 0; i < size; i++) {
            for (j = 0; j < 4; j++) {
                memcpy(d[j] + i, buffer[j] + i, sizeof(Lint));
            }
        }

        // updating r_size for next round
        r_size = new_r_size;

        // sanitizing at end of round
        for (i = 0; i < 4; i++) {
            memset(buffer[i], 0, sizeof(Lint) * size);
        }
        // for (i = 0; i < 2; i++) {
        //     memset(a[i], 0, sizeof(uint8_t) * originial_num);
        //     memset(b[i], 0, sizeof(uint8_t) * originial_num);
        //     memset(u[i], 0, sizeof(uint8_t) * originial_num);
        // }
    }

    // base case of recursive implementation
    for (i = 0; i < size; ++i) {
        res[0][i] = SET_BIT(res[0][i], 0, GET_BIT(d[2][i], 0));
        res[1][i] = SET_BIT(res[1][i], 0, GET_BIT(d[3][i], 0));
    }

    for (i = 0; i < 2; i++) {

        delete[] a[i];
        delete[] b[i];
        delete[] u[i];
    }

    delete[] a;
    delete[] b;
    delete[] u;

    for (i = 0; i < 4; i++) {
        delete[] buffer[i];
    }
    delete[] buffer;
}
// function is used for checking correctness of BitLT
void CircleOp(Lint **res, Lint *p, Lint *g, uint size, uint r_size) {
    int i, j;
    uint new_r_size = r_size >> 1; // (r_size / 2)

    for (i = 0; i < size; ++i) {

        for (j = 0; j < r_size; ++j) {

            uint b1 = GET_BIT(p[i], 2 * j + 1) & GET_BIT(p[i], 2 * j);
            uint b2 = (GET_BIT(p[i], 2 * j + 1) & GET_BIT(g[i], 2 * j)) ^ GET_BIT(g[i], 2 * j + 1);
            // printf("b1 = %u ; b2 = %u\n", b1, b2);
            res[0][i] = SET_BIT(res[0][i], j, b1);
            res[1][i] = SET_BIT(res[1][i], j, b2);
        }
    }

    if ((r_size & 1)) {
        for (i = 0; i < size; ++i) {

            uint b1 = GET_BIT(p[i], r_size - 1);
            uint b2 = GET_BIT(g[i], r_size - 1);

            res[0][i] = SET_BIT(res[0][i], new_r_size, b1);
            res[1][i] = SET_BIT(res[1][i], new_r_size, b2);
        }
    }
}

//  non-recursive solution using Lints and not bytes, and with pext commands
void new_Rss_CarryOutAux_Lint(Lint **res, Lint **d, uint ring_size, uint size, NodeNetwork *nodeNet) {
    uint i, j, originial_num, num, n_uints, r_size, new_r_size, t_index;
    // uint8_t utemp1, utemp2;
    Lint mask2, mask1m8, mask2m8;
    r_size = ring_size;
    originial_num = ((r_size + 7) >> 3) * size;

    Lint **buffer = new Lint *[4];
    for (i = 0; i < 4; i++) {
        buffer[i] = new Lint[size];
        memset(buffer[i], 0, sizeof(Lint) * size);
    }

    uint8_t **a = new uint8_t *[2];
    uint8_t **b = new uint8_t *[2];
    uint8_t **u = new uint8_t *[2];

    // may need to do memsets every iteration
    for (i = 0; i < 2; i++) {
        // memsets are actually needed here since are ORing
        a[i] = new uint8_t[originial_num];
        memset(a[i], 0, sizeof(uint8_t) * originial_num);
        b[i] = new uint8_t[originial_num];
        memset(b[i], 0, sizeof(uint8_t) * originial_num);
        u[i] = new uint8_t[originial_num];
        memset(u[i], 0, sizeof(uint8_t) * originial_num);
    }

    while (r_size > 1) {
        num = ((r_size + 7) >> 3) * size;
        n_uints = ((r_size + 7) >> 3);
        // rounding down here (by default)
        new_r_size = r_size >> 1; // (r_size / 2)

        // preparing the buffer
        // CarryBuffer(buffer, d, size, new_r_size);
        // OptimalBuffer(buffer2, d, size, r_size, nodeNet);
        OptimalBuffer(buffer, d, size, r_size, nodeNet);

        // Splitting the buffer into bytes
        for (i = 0; i < size; ++i) {
            memcpy(a[0] + i * n_uints, buffer[0] + i, n_uints);
            memcpy(a[1] + i * n_uints, buffer[1] + i, n_uints);
            memcpy(b[0] + i * n_uints, buffer[2] + i, n_uints);
            memcpy(b[1] + i * n_uints, buffer[3] + i, n_uints);
        }

        Rss_Mult_Byte(u, a, b, num, nodeNet);

        // clearing the buffer
        for (int i = 0; i < 4; i++) {
            memset(buffer[i], 0, sizeof(Lint) * size);
        }

        // printf("adding g2j+1\n");
        for (i = 0; i < size; ++i) {
            for (j = 0; j < new_r_size; ++j) {
                // loop constants
                t_index = (j >> 2) + (i * n_uints);
                mask2 = (2 * j + 1);
                mask2m8 = (2 * j + 1) & 7;
                mask1m8 = (2 * j) & 7; // "&7" = %8, used for leftover bits

                buffer[0][i] = SET_BIT(buffer[0][i], j, GET_BIT(u[0][t_index], mask1m8));
                buffer[1][i] = SET_BIT(buffer[1][i], j, GET_BIT(u[1][t_index], mask1m8));

                buffer[2][i] = SET_BIT(buffer[2][i], j, (GET_BIT(u[0][t_index], mask2m8) ^ GET_BIT(d[2][i], mask2)));
                buffer[3][i] = SET_BIT(buffer[3][i], j, (GET_BIT(u[1][t_index], mask2m8) ^ GET_BIT(d[3][i], mask2)));
            }
        }

        // checking if we have an unused (p,g) pair
        // if yes, move it to new_r_size's position
        // and update the new_r_size by 1
        if ((r_size & 1)) {
            // printf("moving unused\n");
            for (i = 0; i < size; ++i) {
                for (j = 0; j < 4; ++j) {
                    // getting the unused p (or g) from d and
                    // moving it to new_r_size + 1
                    buffer[j][i] = SET_BIT(buffer[j][i], Lint(new_r_size), GET_BIT(d[j][i], Lint(r_size - 1)));
                }
            }
            new_r_size += 1;
        }
        // copying buffer back to d for next round
        for (i = 0; i < size; i++) {
            for (j = 0; j < 4; j++) {
                memcpy(d[j] + i, buffer[j] + i, sizeof(Lint));
            }
        }

        // updating r_size for next round
        r_size = new_r_size;

        // sanitizing at end of round
        for (i = 0; i < 4; i++) {
            memset(buffer[i], 0, sizeof(Lint) * size);
        }
        // for (i = 0; i < 2; i++) {
        //     memset(a[i], 0, sizeof(uint8_t) * originial_num);
        //     memset(b[i], 0, sizeof(uint8_t) * originial_num);
        //     memset(u[i], 0, sizeof(uint8_t) * originial_num);
        // }
    }

    // base case of recursive implementation
    for (i = 0; i < size; ++i) {
        res[0][i] = SET_BIT(res[0][i], 0, GET_BIT(d[2][i], 0));
        res[1][i] = SET_BIT(res[1][i], 0, GET_BIT(d[3][i], 0));
    }

    for (i = 0; i < 2; i++) {

        delete[] a[i];
        delete[] b[i];
        delete[] u[i];
    }

    delete[] a;
    delete[] b;
    delete[] u;

    for (i = 0; i < 4; i++) {
        delete[] buffer[i];
    }
    delete[] buffer;
}


// returns a binary sharing res, where res = a ?< b (we pack 2 shares in a single bit
// the function expects a and b to be arrays of binary values
// a is a single value
// b is two single values of packed shares
void Rss_BitLT_5pc(Lint **res, Lint *a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet) {
    uint numShares = nodeNet->getNumShares();
    // printf("Rss_BitLT_mp\n");

    int pid = nodeNet->getID();
    uint i, j; // used for loops

    Lint **b_prime = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        b_prime[i] = new Lint[size];
    }



    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
        if (pid == 1) {
            ai[0] = -1;
        } else if (pid == 4) {
            ai[5] = -1; // party 4's share 6
        } else if (pid == 5) {
            ai[3] = -1; // parthy 5's share 4
        }

    for (j = 0; j < size; j++) {
        // step 1  -- flipping all the bits of bk
        // if switch value is zero, do nothing
        for (size_t s = 0; s < numShares; s++) {
            // printf("s: %llu\n", s);
            b_prime[s][j] = (b[s][j] ^ ai[s]);
        }
    }
    // printf("Rss_CarryOut_mp\n");

    Rss_CarryOut_5pc(res, a, b_prime, ring_size, size, nodeNet);

    // flipping on share of output
    for (j = 0; j < size; ++j) {

        for (size_t s = 0; s < numShares; s++) {
            res[s][j] = GET_BIT(res[s][j], 0) ^ GET_BIT(ai[s], 0);
        }

        // res[1][j] = GET_BIT(res[1][j], 0) ^ GET_BIT(b2, 0);
    }

    for (i = 0; i < numShares; i++) {
        delete[] b_prime[i];
    }
    delete[] b_prime;
    delete[] ai;
}

void Rss_CarryOut_5pc(Lint **res, Lint *a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet) {

    uint i;
    int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();

    // d is shared
    // printf("inits\n");
    Lint **d = new Lint *[numShares * 2];
    for (i = 0; i < numShares * 2; i++) {
        d[i] = new Lint[size];
        memset(d[i], 0, sizeof(Lint) * size);
    }

    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
        if (pid == 1) {
            ai[0] = -1;
        } else if (pid == 4) {
            ai[5] = -1; // party 4's share 6
        } else if (pid == 5) {
            ai[3] = -1; // parthy 5's share 4
    }

    // printf("initial step\n");

    // initial step
    for (i = 0; i < size; i++) {
        // printf("i: %llu\n", i);

        for (size_t s = 0; s < numShares; s++) {
            // printf("s_0: %llu\n", s);
            d[s][i] = (a[i] & ai[s]) ^ b[s][i];
            // d[1][i] = (a[i] & a2) ^ b[1][i];
            // printf("s_1: %llu\n", s);

            d[numShares + s][i] = (a[i] & b[s][i]);
        }
        for (size_t s = 0; s < numShares; s++) {
            // printf("s: %llu\n", s);
            d[numShares + s][i] = SET_BIT(d[numShares + s][i], Lint(0), GET_BIT(d[numShares + s][i], Lint(0)) ^ (GET_BIT(d[s][i], Lint(0))));

            // d[3][i] = (a[i] & b[1][i]);
        }

        // shares of p
        // changed from a + b - 2ab
        // d[0][i] = (a[i] & a1) ^ b[0][i];
        // d[1][i] = (a[i] & a2) ^ b[1][i];

        // d[2][i] = (a[i] & b[0][i]);
        // d[3][i] = (a[i] & b[1][i]);

        // d[2][i] = SET_BIT(d[2][i], Lint(0), GET_BIT(d[2][i], Lint(0)) ^ (GET_BIT(d[0][i], Lint(0))));
        // d[3][i] = SET_BIT(d[3][i], Lint(0), GET_BIT(d[3][i], Lint(0)) ^ (GET_BIT(d[1][i], Lint(0))));
    }
    // printf("new_Rss_CarryOutAux_mp\n");

    // Rss_CarryOutAux(res, d, ring_size, size, map, nodeNet);
    new_Rss_CarryOutAux_5pc(res, d, ring_size, size, nodeNet);
    // Rss_CarryOutAux(res, d, ring_size, size, map, nodeNet);

    for (i = 0; i < numShares * 2; i++) {
        delete[] d[i];
    }
    delete[] d;
    delete[] ai;

    // res is the actual return value of carry out, being [g] secret shared
}

void OptimalBuffer_5pc(Lint **buffer, Lint **d, uint size, uint r_size, NodeNetwork *nodeNet) {
    // prepares input u for multiplication
    // buffer and d are the same size (4 x size)
    // reduction to a single loop, making complexity O(size), rather than O(size * r_size)
    uint numShares = nodeNet->getNumShares();

    Lint i, even, odd;
    // doing this once
    even = nodeNet->EVEN[r_size];
    odd = nodeNet->ODD[r_size];

    for (i = 0; i < size; ++i) {
        for (size_t s = 0; s < numShares; s++) {
            buffer[s][i] = buffer[s][i] | (d[s][i] & even);
            buffer[s][i] = buffer[s][i] | ((d[s + numShares][i] & even) << Lint(1));

            buffer[s + numShares][i] = buffer[s + numShares][i] | ((d[s][i] & odd) >> Lint(1));
            buffer[s + numShares][i] = buffer[s + numShares][i] | (d[s][i] & odd);
        }

        // buffer[0][i] = buffer[0][i] | (d[0][i] & even);
        // buffer[1][i] = buffer[1][i] | (d[1][i] & even);
        // buffer[0][i] = buffer[0][i] | ((d[2][i] & even) << Lint(1));
        // buffer[1][i] = buffer[1][i] | ((d[3][i] & even) << Lint(1));

        // buffer[2][i] = buffer[2][i] | ((d[0][i] & odd) >> Lint(1));
        // buffer[3][i] = buffer[3][i] | ((d[1][i] & odd) >> Lint(1));
        // buffer[2][i] = buffer[2][i] | (d[0][i] & odd);
        // buffer[3][i] = buffer[3][i] | (d[1][i] & odd);

        // buffer[0][i] = buffer[0][i] & nodeNet->SHIFT[r_size];
        // buffer[1][i] = buffer[1][i] & nodeNet->SHIFT[r_size];
        // buffer[2][i] = buffer[2][i] & nodeNet->SHIFT[r_size];
        // buffer[3][i] = buffer[3][i] & nodeNet->SHIFT[r_size];
    }
}

// a non-recursive solution to minimize memory consumption
void new_Rss_CarryOutAux_5pc(Lint **res, Lint **d, uint ring_size, uint size, NodeNetwork *nodeNet) {
    uint numShares = nodeNet->getNumShares();
    uint i, j, originial_num, num, n_uints, r_size, new_r_size, t_index;
    // uint8_t utemp1, utemp2;
    Lint mask2, mask1m8, mask2m8;
    r_size = ring_size;
    originial_num = ((r_size + 7) >> 3) * size;

    Lint **buffer = new Lint *[numShares * 2];
    for (i = 0; i < numShares * 2; i++) {
        buffer[i] = new Lint[size];
        memset(buffer[i], 0, sizeof(Lint) * size);
    }

    uint8_t **a = new uint8_t *[numShares];
    uint8_t **b = new uint8_t *[numShares];
    uint8_t **u = new uint8_t *[numShares];

    //  need to do memsets every iteration
    for (i = 0; i < numShares; i++) {
        // memsets are actually needed here since are ORing
        // tried w/o memsets - still working?
        a[i] = new uint8_t[originial_num];
        memset(a[i], 0, sizeof(uint8_t) * originial_num);
        b[i] = new uint8_t[originial_num];
        memset(b[i], 0, sizeof(uint8_t) * originial_num);
        u[i] = new uint8_t[originial_num];
        // memset(u[i], 0, sizeof(uint8_t) * originial_num);
    }

    while (r_size > 1) {
        num = ((r_size + 7) >> 3) * size;
        n_uints = ((r_size + 7) >> 3);
        // rounding down here (by default)
        new_r_size = r_size >> 1; // (r_size / 2)

        // preparing the buffer
        // CarryBuffer(buffer, d, size, new_r_size);
        // OptimalBuffer(buffer2, d, size, r_size, nodeNet);
        OptimalBuffer_5pc(buffer, d, size, r_size, nodeNet);

        // Splitting the buffer into bytes
        for (i = 0; i < size; ++i) {
            for (size_t s = 0; s < numShares; s++) {
                memcpy(a[s] + i * n_uints, buffer[s] + i, n_uints);
                memcpy(b[s] + i * n_uints, buffer[numShares + s] + i, n_uints);
            }
        }

        Rss_Mult_Byte_5pc(u, a, b, num, nodeNet);

        // clearing the buffer
        for (int i = 0; i < numShares * 2; i++) {
            memset(buffer[i], 0, sizeof(Lint) * size);
        }

        // printf("adding g2j+1\n");
        for (i = 0; i < size; ++i) {
            for (j = 0; j < new_r_size; ++j) {
                // loop constants
                t_index = (j >> 2) + (i * n_uints);
                mask2 = (2 * j + 1);
                mask2m8 = (2 * j + 1) & 7;
                mask1m8 = (2 * j) & 7; // "&7" = %8, used for leftover bits

                for (size_t s = 0; s < numShares; s++) {

                    buffer[s][i] = SET_BIT(buffer[s][i], j, GET_BIT(u[s][t_index], mask1m8));
                    // buffer[s][i] = SET_BIT(buffer[s][i], j, GET_BIT(u[s][t_index], mask1m8));

                    buffer[numShares + s][i] = SET_BIT(buffer[numShares + s][i], j, (GET_BIT(u[s][t_index], mask2m8) ^ GET_BIT(d[numShares + s][i], mask2)));
                    // buffer[numShares + s][i] = SET_BIT(buffer[numShares + s][i], j, (GET_BIT(u[s][t_index], mask2m8) ^ GET_BIT(d[numShares + s][i], mask2)));
                }
            }
        }

        // checking if we have an unused (p,g) pair
        // if yes, move it to new_r_size's position
        // and update the new_r_size by 1
        if ((r_size & 1)) {
            // printf("moving unused\n");
            for (i = 0; i < size; ++i) {
                for (j = 0; j < 2 * numShares; ++j) {
                    // getting the unused p (or g) from d and
                    // moving it to new_r_size + 1
                    buffer[j][i] = SET_BIT(buffer[j][i], Lint(new_r_size), GET_BIT(d[j][i], Lint(r_size - 1)));
                }
            }
            new_r_size += 1;
        }
        // copying buffer back to d for next round
        for (i = 0; i < size; i++) {
            for (j = 0; j < 2 * numShares; j++) {
                memcpy(d[j] + i, buffer[j] + i, sizeof(Lint));
            }
        }

        // updating r_size for next round
        r_size = new_r_size;

        // sanitizing at end of round
        for (i = 0; i < 2 * numShares; i++) {
            memset(buffer[i], 0, sizeof(Lint) * size);
        }
        // for (i = 0; i < 2; i++) {
        //     memset(a[i], 0, sizeof(uint8_t) * originial_num);
        //     memset(b[i], 0, sizeof(uint8_t) * originial_num);
        //     memset(u[i], 0, sizeof(uint8_t) * originial_num);
        // }
    }

    // base case of recursive implementation
    for (i = 0; i < size; ++i) {
        res[0][i] = SET_BIT(res[0][i], 0, GET_BIT(d[2][i], 0));
        res[1][i] = SET_BIT(res[1][i], 0, GET_BIT(d[3][i], 0));
    }

    for (i = 0; i < numShares; i++) {

        delete[] a[i];
        delete[] b[i];
        delete[] u[i];
    }

    delete[] a;
    delete[] b;
    delete[] u;

    for (i = 0; i < numShares * 2; i++) {
        delete[] buffer[i];
    }
    delete[] buffer;
}


// returns a binary sharing res, where res = a ?< b (we pack 2 shares in a single bit
// the function expects a and b to be arrays of binary values
// a is a single value
// b is two single values of packed shares
void Rss_BitLT_7pc(Lint **res, Lint *a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet) {
    uint numShares = nodeNet->getNumShares();
    // printf("Rss_BitLT_7pc\n");

    int pid = nodeNet->getID();
    uint i, j; // used for loops

    Lint **b_prime = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        b_prime[i] = new Lint[size];
    }



    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
    if (pid == 1) {
        ai[0] = -1;
    } else if (pid == 5) {
        ai[19] = -1; // party 4's share 6
    } else if (pid == 6) {
        ai[16] = -1; // parthy 5's share 4
    } else if (pid == 7) {
        ai[10] = -1; // parthy 5's share 4
    } 
    

    for (j = 0; j < size; j++) {
        // step 1  -- flipping all the bits of bk
        // if switch value is zero, do nothing
        for (size_t s = 0; s < numShares; s++) {
            // printf("s: %llu\n", s);
            b_prime[s][j] = (b[s][j] ^ ai[s]);
        }
    }
    // printf("Rss_CarryOut_7pc\n");

    Rss_CarryOut_7pc(res, a, b_prime, ring_size, size, nodeNet);

    // flipping on share of output
    for (j = 0; j < size; ++j) {

        for (size_t s = 0; s < numShares; s++) {
            res[s][j] = GET_BIT(res[s][j], 0) ^ GET_BIT(ai[s], 0);
        }

        // res[1][j] = GET_BIT(res[1][j], 0) ^ GET_BIT(b2, 0);
    }

    for (i = 0; i < numShares; i++) {
        delete[] b_prime[i];
    }
    delete[] b_prime;
    delete[] ai;
}

void Rss_CarryOut_7pc(Lint **res, Lint *a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet) {

    uint i;
    int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();

    // d is shared
    // printf("inits\n");
    Lint **d = new Lint *[numShares * 2];
    for (i = 0; i < numShares * 2; i++) {
        d[i] = new Lint[size];
        memset(d[i], 0, sizeof(Lint) * size);
    }

    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
    if (pid == 1) {
        ai[0] = -1;
    } else if (pid == 5) {
        ai[19] = -1; // party 4's share 6
    } else if (pid == 6) {
        ai[16] = -1; // parthy 5's share 4
    } else if (pid == 7) {
        ai[10] = -1; // parthy 5's share 4
    } 
    

    // printf("initial step\n");

    // initial step
    for (i = 0; i < size; i++) {
        // printf("i: %llu\n", i);

        for (size_t s = 0; s < numShares; s++) {
            // printf("s_0: %llu\n", s);
            d[s][i] = (a[i] & ai[s]) ^ b[s][i];
            // d[1][i] = (a[i] & a2) ^ b[1][i];
            // printf("s_1: %llu\n", s);

            d[numShares + s][i] = (a[i] & b[s][i]);
        }
        for (size_t s = 0; s < numShares; s++) {
            // printf("s: %llu\n", s);
            d[numShares + s][i] = SET_BIT(d[numShares + s][i], Lint(0), GET_BIT(d[numShares + s][i], Lint(0)) ^ (GET_BIT(d[s][i], Lint(0))));

            // d[3][i] = (a[i] & b[1][i]);
        }

        // shares of p
        // changed from a + b - 2ab
        // d[0][i] = (a[i] & a1) ^ b[0][i];
        // d[1][i] = (a[i] & a2) ^ b[1][i];

        // d[2][i] = (a[i] & b[0][i]);
        // d[3][i] = (a[i] & b[1][i]);

        // d[2][i] = SET_BIT(d[2][i], Lint(0), GET_BIT(d[2][i], Lint(0)) ^ (GET_BIT(d[0][i], Lint(0))));
        // d[3][i] = SET_BIT(d[3][i], Lint(0), GET_BIT(d[3][i], Lint(0)) ^ (GET_BIT(d[1][i], Lint(0))));
    }
    // printf("new_Rss_CarryOutAux_7pc\n");

    // Rss_CarryOutAux(res, d, ring_size, size, map, nodeNet);
    new_Rss_CarryOutAux_7pc(res, d, ring_size, size, nodeNet);
    // Rss_CarryOutAux(res, d, ring_size, size, map, nodeNet);

    for (i = 0; i < numShares * 2; i++) {
        delete[] d[i];
    }
    delete[] d;
    delete[] ai;

    // res is the actual return value of carry out, being [g] secret shared
}

void OptimalBuffer_7pc(Lint **buffer, Lint **d, uint size, uint r_size, NodeNetwork *nodeNet) {
    // prepares input u for multiplication
    // buffer and d are the same size (4 x size)
    // reduction to a single loop, making complexity O(size), rather than O(size * r_size)
    uint numShares = nodeNet->getNumShares();

    Lint i, even, odd;
    // doing this once
    even = nodeNet->EVEN[r_size];
    odd = nodeNet->ODD[r_size];

    for (i = 0; i < size; ++i) {
        for (size_t s = 0; s < numShares; s++) {
            buffer[s][i] = buffer[s][i] | (d[s][i] & even);
            buffer[s][i] = buffer[s][i] | ((d[s + numShares][i] & even) << Lint(1));

            buffer[s + numShares][i] = buffer[s + numShares][i] | ((d[s][i] & odd) >> Lint(1));
            buffer[s + numShares][i] = buffer[s + numShares][i] | (d[s][i] & odd);
        }

        // buffer[0][i] = buffer[0][i] | (d[0][i] & even);
        // buffer[1][i] = buffer[1][i] | (d[1][i] & even);
        // buffer[0][i] = buffer[0][i] | ((d[2][i] & even) << Lint(1));
        // buffer[1][i] = buffer[1][i] | ((d[3][i] & even) << Lint(1));

        // buffer[2][i] = buffer[2][i] | ((d[0][i] & odd) >> Lint(1));
        // buffer[3][i] = buffer[3][i] | ((d[1][i] & odd) >> Lint(1));
        // buffer[2][i] = buffer[2][i] | (d[0][i] & odd);
        // buffer[3][i] = buffer[3][i] | (d[1][i] & odd);

        // buffer[0][i] = buffer[0][i] & nodeNet->SHIFT[r_size];
        // buffer[1][i] = buffer[1][i] & nodeNet->SHIFT[r_size];
        // buffer[2][i] = buffer[2][i] & nodeNet->SHIFT[r_size];
        // buffer[3][i] = buffer[3][i] & nodeNet->SHIFT[r_size];
    }
}

// a non-recursive solution to minimize memory consumption
void new_Rss_CarryOutAux_7pc(Lint **res, Lint **d, uint ring_size, uint size, NodeNetwork *nodeNet) {
    uint numShares = nodeNet->getNumShares();
    uint i, j, originial_num, num, n_uints, r_size, new_r_size, t_index;
    // uint8_t utemp1, utemp2;
    Lint mask2, mask1m8, mask2m8;
    r_size = ring_size;
    originial_num = ((r_size + 7) >> 3) * size;

    Lint **buffer = new Lint *[numShares * 2];
    for (i = 0; i < numShares * 2; i++) {
        buffer[i] = new Lint[size];
        memset(buffer[i], 0, sizeof(Lint) * size);
    }

    uint8_t **a = new uint8_t *[numShares];
    uint8_t **b = new uint8_t *[numShares];
    uint8_t **u = new uint8_t *[numShares];

    //  need to do memsets every iteration
    for (i = 0; i < numShares; i++) {
        // memsets are actually needed here since are ORing
        // tried w/o memsets - still working?
        a[i] = new uint8_t[originial_num];
        memset(a[i], 0, sizeof(uint8_t) * originial_num);
        b[i] = new uint8_t[originial_num];
        memset(b[i], 0, sizeof(uint8_t) * originial_num);
        u[i] = new uint8_t[originial_num];
        // memset(u[i], 0, sizeof(uint8_t) * originial_num);
    }

    while (r_size > 1) {
        num = ((r_size + 7) >> 3) * size;
        n_uints = ((r_size + 7) >> 3);
        // rounding down here (by default)
        new_r_size = r_size >> 1; // (r_size / 2)

        // preparing the buffer
        // CarryBuffer(buffer, d, size, new_r_size);
        // OptimalBuffer(buffer2, d, size, r_size, nodeNet);
        OptimalBuffer_7pc(buffer, d, size, r_size, nodeNet);

        // Splitting the buffer into bytes
        for (i = 0; i < size; ++i) {
            for (size_t s = 0; s < numShares; s++) {
                memcpy(a[s] + i * n_uints, buffer[s] + i, n_uints);
                memcpy(b[s] + i * n_uints, buffer[numShares + s] + i, n_uints);
            }
        }

        Rss_Mult_Byte_7pc(u, a, b, num, nodeNet);

        // clearing the buffer
        for (int i = 0; i < numShares * 2; i++) {
            memset(buffer[i], 0, sizeof(Lint) * size);
        }

        // printf("adding g2j+1\n");
        for (i = 0; i < size; ++i) {
            for (j = 0; j < new_r_size; ++j) {
                // loop constants
                t_index = (j >> 2) + (i * n_uints);
                mask2 = (2 * j + 1);
                mask2m8 = (2 * j + 1) & 7;
                mask1m8 = (2 * j) & 7; // "&7" = %8, used for leftover bits

                for (size_t s = 0; s < numShares; s++) {

                    buffer[s][i] = SET_BIT(buffer[s][i], j, GET_BIT(u[s][t_index], mask1m8));
                    // buffer[s][i] = SET_BIT(buffer[s][i], j, GET_BIT(u[s][t_index], mask1m8));

                    buffer[numShares + s][i] = SET_BIT(buffer[numShares + s][i], j, (GET_BIT(u[s][t_index], mask2m8) ^ GET_BIT(d[numShares + s][i], mask2)));
                    // buffer[numShares + s][i] = SET_BIT(buffer[numShares + s][i], j, (GET_BIT(u[s][t_index], mask2m8) ^ GET_BIT(d[numShares + s][i], mask2)));
                }
            }
        }

        // checking if we have an unused (p,g) pair
        // if yes, move it to new_r_size's position
        // and update the new_r_size by 1
        if ((r_size & 1)) {
            // printf("moving unused\n");
            for (i = 0; i < size; ++i) {
                for (j = 0; j < 2 * numShares; ++j) {
                    // getting the unused p (or g) from d and
                    // moving it to new_r_size + 1
                    buffer[j][i] = SET_BIT(buffer[j][i], Lint(new_r_size), GET_BIT(d[j][i], Lint(r_size - 1)));
                }
            }
            new_r_size += 1;
        }
        // copying buffer back to d for next round
        for (i = 0; i < size; i++) {
            for (j = 0; j < 2 * numShares; j++) {
                memcpy(d[j] + i, buffer[j] + i, sizeof(Lint));
            }
        }

        // updating r_size for next round
        r_size = new_r_size;

        // sanitizing at end of round
        for (i = 0; i < 2 * numShares; i++) {
            memset(buffer[i], 0, sizeof(Lint) * size);
        }
    }

    // base case of recursive implementation
    for (i = 0; i < size; ++i) {
        res[0][i] = SET_BIT(res[0][i], 0, GET_BIT(d[2][i], 0));
        res[1][i] = SET_BIT(res[1][i], 0, GET_BIT(d[3][i], 0));
    }

    for (i = 0; i < numShares; i++) {

        delete[] a[i];
        delete[] b[i];
        delete[] u[i];
    }

    delete[] a;
    delete[] b;
    delete[] u;

    for (i = 0; i < numShares * 2; i++) {
        delete[] buffer[i];
    }
    delete[] buffer;
}
