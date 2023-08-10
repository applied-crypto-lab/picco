#include "bitAdd.h"

// need way to tell which parties' bitwise shares we're adding
// e.g. 1st add p_1 and p_2 to get p'
// then add p' and p_3
void Rss_nBitAdd(Lint **res, Lint **r_bitwise, uint ring_size, uint size, NodeNetwork *nodeNet) {

    uint i, j;
    uint numShares = nodeNet->getNumShares();
    uint threshold = nodeNet->getThreshold();
    uint rounds = ceil(log2(threshold + 1));
    // printf("rounds: %u\n", rounds);
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
            Rss_BitAdd(res, a, b, ring_size, size, nodeNet);
        }
        // else {
        //     // we only need to copy r_bitwise into b
        //     for (size_t s = 0; s < numShares; s++) {
        //         memcpy(a[s], res[s], size * sizeof(Lint));
        //         memcpy(b[s], r_bitwise[s] + 2 * size, size * sizeof(Lint));
        //     }
        //     Rss_BitAdd(res, a, b, ring_size, size, nodeNet);
        // }
    }

    for (i = 0; i < numShares; i++) {
        delete[] a[i];
        delete[] b[i];
    }
    delete[] a;
    delete[] b;
}

// alternative BitAdd implementation when both a and b are secret shared
// used in edaBit
void Rss_BitAdd(Lint **res, Lint **a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet) {

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

    // Lint *res_check = new Lint[size];
    // memset(res_check, 0, sizeof(Lint) * size);

    // Rss_Open_Bitwise(res_check, res, size, ring_size, nodeNet);
    // for (int i = 0; i < size; i++) {
    //     printf("Mult_bitwise res_check[%i]  : %llu\n", i, res_check[i]);
    //     print_binary(res_check[i], ring_size);
    // }
    // printf("\n");

    // delete[] res_check;

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            d[s][i] = a[s][i] ^ b[s][i];
            // d[1][i] = a[1][i] ^ b[1][i];

            d[numShares + s][i] = res[s][i];
            // d[3][i] = res[1][i];
        }
    }

    // Rss_CircleOpL(d, ring_size, size, nodeNet);

    // gettimeofday(&start, NULL); //start timer here

    // Rss_CircleOpL_Lint(d, ring_size, size, nodeNet); // new version w Lints
    Rss_CircleOpL(d, ring_size, size, nodeNet); // original

    // gettimeofday(&end, NULL); //stop timer here
    // timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    // printf("Runtime for Rss_CircleOpL with data size %d = %.6lf ms\n", size, (double)(timer * 0.001));

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            res[s][i] = (a[s][i] ^ b[s][i]) ^ (d[numShares + s][i] << Lint(1));
    }

    for (i = 0; i < 2 * numShares; i++) {
        delete[] d[i];
    }
    delete[] d;
}

// a is public, b is bitwise-shared
// res will be a bitwise shared output
void Rss_BitAdd(Lint **res, Lint *a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet) {

    Lint i;
    int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();

    Lint **d = new Lint *[2 * numShares];
    for (i = 0; i < 2 * numShares; i++) {
        d[i] = new Lint[size];
        memset(d[i], 0, sizeof(Lint) * size);
    }

    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;

    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
    if (pid == 1) {
        ai[0] = -1; // party 1's share 1
    } else if (pid == 3) {
        ai[1] = -1; // party 3's share 2
    }

    // Lint a1, a2;
    // switch (pid) {
    // case 1:
    //     a1 = -1;
    //     a2 = 0;
    //     break;
    // case 2:
    //     a1 = 0;
    //     a2 = 0;
    //     break;
    // case 3:
    //     a1 = 0;
    //     a2 = -1;
    //     break;
    // }
    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            d[s][i] = (a[i] & ai[s]) ^ b[s][i];
            // d[1][i] = (a[i] & a2) ^ b[1][i];

            d[numShares + s][i] = (a[i] & b[s][i]);
            // d[3][i] = (a[i] & b[1][i]);
        }
    }

    // gettimeofday(&start, NULL); //start timer here
    // Rss_CircleOpL_Lint(d, ring_size, size, nodeNet);
    Rss_CircleOpL(d, ring_size, size, nodeNet);
    // gettimeofday(&end, NULL); //stop timer here
    // timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    // printf("Runtime for Rss_CircleOpL with data size %d = %.6lf ms\n", size, (double)(timer * 0.001));

    // we only need the values in the g position (indices 2 and 3)

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            res[s][i] = ((a[i] & ai[s]) ^ b[s][i]) ^ (d[numShares + s][i] << 1);
        // res[1][i] = ((a[i] & a2) ^ b[1][i]) ^ (d[3][i] << 1);
    }

    for (i = 0; i < 2 * numShares; i++) {
        delete[] d[i];
    }
    delete[] ai;
    delete[] d;
}

void Rss_CircleOpL(Lint **d, uint r_size, uint size, NodeNetwork *nodeNet) {

    Lint i, j, l, y, z, op_r; // used for loops
    uint numShares = nodeNet->getNumShares();

    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;

    // struct timeval start2;
    // struct timeval end2;
    // unsigned long timer2;

    if (r_size > 1) {

        // just three nested for-loops
        // r_size <=> k in algorithm

        uint num = ((r_size + 7) >> 3) * size;
        uint n_uints = ((r_size + 7) >> 3);
        uint t_index;
        Lint mask2, mask1m8, mask2m8;
        // Lint mask1p, mask2p;
        uint r_size_2 = pow(2, ceil(log2(r_size))); // rounding up to next power of two
        uint rounds = ceil(log2(r_size_2));

        uint **index_array = new uint *[2];
        Lint **buffer = new Lint *[2 * numShares];
        // we need at most (r_size + 7)/8 bytes to store bits from the buffer
        uint8_t **a = new uint8_t *[numShares];
        uint8_t **b = new uint8_t *[numShares];
        uint8_t **u = new uint8_t *[numShares];
        for (i = 0; i < 2; i++) {

            index_array[i] = new uint[r_size_2];
            memset(index_array[i], 0, sizeof(uint) * r_size_2);
        }

        for (i = 0; i < numShares; i++) {
            buffer[i] = new Lint[size];
            buffer[numShares + i] = new Lint[size];
            memset(buffer[i], 0, sizeof(Lint) * size);
            memset(buffer[numShares + i], 0, sizeof(Lint) * size);
            // memsets are actually needed here since are ORing
            a[i] = new uint8_t[num];
            memset(a[i], 0, sizeof(uint8_t) * num);
            b[i] = new uint8_t[num];
            memset(b[i], 0, sizeof(uint8_t) * num);
            u[i] = new uint8_t[num];
            memset(u[i], 0, sizeof(uint8_t) * num);
        }

        for (i = 1; i <= rounds; i++) {

            // gettimeofday(&start, NULL); //start timer here

            op_r = 0; // number of operations in a round
            // equivalent to the new_ring_size in MSB

            // gettimeofday(&start2, NULL); //start timer here

            for (j = 1; j <= ceil(r_size_2 / pow(2, i)); j++) {
                y = uint(pow(2, i - 1) + j * pow(2, i)) % r_size_2;
                for (z = 1; z <= (pow(2, i - 1)); z++) {
                    // checking we have a valid set of indices to add to our set
                    if ((((y % r_size_2)) <= r_size) && (((y + z) % (r_size_2 + 1)) <= r_size)) {
                        // printf("y : %u\n", y);
                        // printf("y+z : %u\n", y+z);
                        index_array[0][op_r] = (y % r_size_2) - 1;
                        index_array[1][op_r] = ((y + z) % (r_size_2 + 1)) - 1;
                        op_r++;
                    }
                }
            }

            // gettimeofday(&end2, NULL); //stop timer here
            // timer2 = 1000000 * (end2.tv_sec - start2.tv_sec) + end2.tv_usec - start2.tv_usec;
            // printf("Runtime for index_array with data size %d = %.6lf ms\n", size, (double)(timer2 * 0.001));

            // updating parameters for optimization
            n_uints = ((2 * op_r + 7) >> 3);
            num = ((2 * op_r + 7) >> 3) * size;

            // extracting terms into buffer
            CarryBuffer2(buffer, d, index_array, size, op_r, numShares);

            // gettimeofday(&start2, NULL); //start timer here
            // Splitting the buffer into bytes

            // THIS DOESNT WORK
            // DO NOT TRY
            // memcpy(a[0], buffer[0], size*n_uints);
            // memcpy(a[1], buffer[1], size*n_uints);
            // memcpy(b[0], buffer[2], size*n_uints);
            // memcpy(b[1], buffer[3], size*n_uints);

            for (j = 0; j < size; ++j) {
                for (size_t s = 0; s < numShares; s++) {
                    memcpy(a[s] + j * n_uints, buffer[s] + j, n_uints);
                    // memcpy(a[1] + j * n_uints, buffer[1] + j, n_uints);
                    memcpy(b[s] + j * n_uints, buffer[numShares + s] + j, n_uints);
                    // memcpy(b[1] + j * n_uints, buffer[3] + j, n_uints);
                }
            }

            // uint8_t *res_check = new uint8_t[num];
            // memset(res_check, 0, sizeof(uint8_t) * num);

            // Rss_Open_Byte(res_check, a, num, r_size, nodeNet);
            // for (int ii = 0; ii < num; ii++) {
            //     printf("a[%i]  : %llu\n", ii, res_check[ii]);
            //     print_binary(res_check[ii], 8);
            // }
            // Rss_Open_Byte(res_check, b, num, r_size, nodeNet);
            // for (int ii = 0; ii < num; ii++) {
            //     printf("b[%i]  : %llu\n", ii, res_check[ii]);
            //     print_binary(res_check[ii], 8);
            // }

            // printf("\n");

            // gettimeofday(&end2, NULL); //stop timer here
            // timer2 = 1000000 * (end2.tv_sec - start2.tv_sec) + end2.tv_usec - start2.tv_usec;
            // printf("Runtime for memcpy with data size %d = %.6lf ms\n", size, (double)(timer2 * 0.001));

            // gettimeofday(&start2, NULL); //start timer here
            // printf("\n");
            // for (int ii = 0; ii < num; ii++) {
            //     for (uint s = 0; s < numShares; s++) {
            //         printf("a[%i][%i]  : %u \t", s, ii, a[s][ii]);
            //         print_binary(a[s][ii], 8);
            //     }
            // }
            // printf("\n");
            // for (int ii = 0; ii < num; ii++) {
            //     for (uint s = 0; s < numShares; s++) {
            //         printf("b[%i][%i]  : %u \t", s, ii, b[s][ii]);
            //         print_binary(b[s][ii], 8);
            //     }

            //     // for (size_t s = 0; s < numShares; s++)
            //     // {
            //     // printf("b[%i][%i]  : %llu\n", s, ii, b[s][ii]);
            //     // print_binary(b[s][ii], 8);

            //     // }
            // }

            // bitwise multiplication
            Rss_Mult_Byte(u, a, b, num, nodeNet);
            // Rss_Open_Byte(res_check, u, num, r_size, nodeNet);

            // for (int ii = 0; ii < num; ii++) {
            //     printf("Mult_byte u[%i]  : %llu\n", ii, res_check[ii]);
            //     print_binary(res_check[i], 8);
            // }
            // printf("\n");

            // delete[] res_check;

            // gettimeofday(&end2, NULL); //stop timer here
            // timer2 = 1000000 * (end2.tv_sec - start2.tv_sec) + end2.tv_usec - start2.tv_usec;
            // printf("Runtime for Rss_Mult_Byte with data size %d = %.6lf ms\n", size, (double)(timer2 * 0.001));

            // gettimeofday(&start2, NULL); //start timer here

            for (l = 0; l < size; ++l) {
                for (j = 0; j < op_r; ++j) {
                    // loop constants
                    t_index = (j >> 2) + (l * n_uints);
                    mask2 = index_array[1][j];
                    mask1m8 = (2 * j) & 7; // "&7" = %8, used for leftover bits
                    mask2m8 = (2 * j + 1) & 7;

                    for (size_t s = 0; s < numShares; s++) {
                        d[s][l] = SET_BIT(d[s][l], mask2, GET_BIT(u[s][t_index], mask1m8));
                        // d[1][l] = SET_BIT(d[1][l], mask2, GET_BIT(u[1][t_index], mask1m8));

                        // simplified from needing two separate loops
                        d[numShares + s][l] = SET_BIT(d[numShares + s][l], mask2, (GET_BIT(u[s][t_index], mask2m8) ^ GET_BIT(d[numShares + s][l], mask2)));
                        // d[3][l] = SET_BIT(d[3][l], mask2, (GET_BIT(u[1][t_index], mask2m8) ^ GET_BIT(d[3][l], mask2)));
                    }
                }
            }
            // only doing one round for testing

            // break;

            // gettimeofday(&end2, NULL); //stop timer here
            // timer2 = 1000000 * (end2.tv_sec - start2.tv_sec) + end2.tv_usec - start2.tv_usec;
            // printf("Runtime for rearranging/circleOp with data size %d = %.6lf ms\n", size, (double)(timer2 * 0.001));

            // gettimeofday(&end, NULL); //stop timer here
            // timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
            // printf("Runtime for round %i with data size %d = %.6lf ms\n\n", i, size, (double)(timer * 0.001));
        }

        for (i = 0; i < numShares; i++) {
            delete[] buffer[i];
            delete[] buffer[numShares + i];
            delete[] a[i];
            delete[] b[i];
            delete[] u[i];
        }
        for (i = 0; i < 2; i++) {
            delete[] index_array[i];
        }

        delete[] a;
        delete[] b;
        delete[] u;
        delete[] index_array;
        delete[] buffer;
    }
}

// NOT UPDATED
void Rss_CircleOpL_Lint(Lint **d, uint r_size, uint size, NodeNetwork *nodeNet) {

    Lint i, j, l, y, z, op_r; // used for loops
    struct timeval start;
    struct timeval end;
    unsigned long timer;
    uint numShares = nodeNet->getNumShares();

    if (r_size > 1) {

        Lint mask2, mask1m8, mask2m8;
        uint r_size_2 = pow(2, ceil(log2(r_size))); // rounding up to next power of two
        uint rounds = ceil(log2(r_size_2));

        uint **index_array = new uint *[2];
        Lint **a_prime = new Lint *[2];
        Lint **b_prime = new Lint *[2];
        Lint **u_prime = new Lint *[2];

        for (i = 0; i < 2; i++) {
            index_array[i] = new uint[r_size_2];
            a_prime[i] = new Lint[size];
            b_prime[i] = new Lint[size];
            u_prime[i] = new Lint[size];
        }

        for (i = 1; i <= rounds; i++) {
            gettimeofday(&start, NULL); // start timer here

            op_r = 0; // number of operations in a round
            // equivalent to the new_ring_size in MSB

            for (j = 1; j <= ceil(r_size_2 / pow(2, i)); j++) {

                y = uint(pow(2, i - 1) + j * pow(2, i)) % r_size_2;

                for (z = 1; z <= (pow(2, i - 1)); z++) {
                    // checking we have a valid set of indices to add to our set
                    if ((((y % r_size_2)) <= r_size) && (((y + z) % (r_size_2 + 1)) <= r_size)) {
                        // printf("y : %u\n", y);
                        // printf("y+z : %u\n", y+z);
                        index_array[0][op_r] = (y % r_size_2) - 1;
                        index_array[1][op_r] = ((y + z) % (r_size_2 + 1)) - 1;
                        op_r++;
                    }
                }
            }

            // extracting terms into a_prime and b_prime directly
            CarryBuffer_Lint(a_prime, b_prime, d, index_array, size, op_r, numShares);

            // bitwise multiplication
            Rss_Mult_Bitwise(u_prime, a_prime, b_prime, size, r_size, nodeNet);

            // printf("adding g2j+1\n");
            for (l = 0; l < size; ++l) {
                for (j = 0; j < op_r; ++j) {

                    // loop constants
                    // DO NOT REMOVE mask2
                    // putting it directly in next  operations
                    // of SET BIT causes it to FAIL
                    // DONT ASK WHY

                    mask2 = index_array[1][j]; // CHECK
                    mask1m8 = (2 * j);         // "&7" = %8, used for leftover bits
                    mask2m8 = (2 * j + 1);

                    d[0][l] = SET_BIT(d[0][l], mask2, GET_BIT(u_prime[0][l], mask1m8));
                    d[1][l] = SET_BIT(d[1][l], mask2, GET_BIT(u_prime[1][l], mask1m8));

                    d[2][l] = SET_BIT(d[2][l], mask2, (GET_BIT(u_prime[0][l], mask2m8) ^ GET_BIT(d[2][l], mask2)));
                    d[3][l] = SET_BIT(d[3][l], mask2, (GET_BIT(u_prime[1][l], mask2m8) ^ GET_BIT(d[3][l], mask2)));
                }
            }
            gettimeofday(&end, NULL); // stop timer here
            timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
            // printf("Runtime for round with data size %d = %.6lf ms\n", size, (double)(timer * 0.001));
        }
        for (i = 0; i < 2; i++) {
            delete[] a_prime[i];
            delete[] b_prime[i];
            delete[] u_prime[i];
            delete[] index_array[i];
        }
        delete[] a_prime;
        delete[] b_prime;
        delete[] u_prime;
        delete[] index_array;
    }
}

void CarryBuffer2(Lint **buffer, Lint **d, uint **index_array, uint size, uint k, uint numShares) {
    // prepares input u for multiplication
    // extracts p2i, p2i-1, and g2i
    // buffer and d are the same size (4 x size)

    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;

    Lint i, j, mask1, mask2, mask1p, mask2p;

    // gettimeofday(&start, NULL); //start timer here

    for (i = 0; i < size; i++) {
        for (j = 0; j < k; j++) {

            // used to set the bits in the correct positions in buffer
            mask1 = 2 * j;
            mask2 = 2 * j + 1;

            // used to get the correct bits from d
            mask1p = index_array[0][j];
            mask2p = index_array[1][j];

            for (Lint s = 0; s < numShares; s++) {
                buffer[s][i] = SET_BIT(buffer[s][i], mask1, GET_BIT(d[s][i], mask1p));
                buffer[s][i] = SET_BIT(buffer[s][i], mask2, GET_BIT(d[numShares + s][i], mask1p));
                buffer[numShares + s][i] = SET_BIT(buffer[numShares + s][i], mask1, GET_BIT(d[s][i], mask2p));
                buffer[numShares + s][i] = SET_BIT(buffer[numShares + s][i], mask2, GET_BIT(d[s][i], mask2p));
            }

            // buffer[0][i] = SET_BIT(buffer[0][i], mask1, GET_BIT(d[0][i], mask1p));
            // buffer[1][i] = SET_BIT(buffer[1][i], mask1, GET_BIT(d[1][i], mask1p));

            // buffer[0][i] = SET_BIT(buffer[0][i], mask2, GET_BIT(d[2][i], mask1p));
            // buffer[1][i] = SET_BIT(buffer[1][i], mask2, GET_BIT(d[3][i], mask1p));

            // buffer[2][i] = SET_BIT(buffer[2][i], mask1, GET_BIT(d[0][i], mask2p));
            // buffer[3][i] = SET_BIT(buffer[3][i], mask1, GET_BIT(d[1][i], mask2p));

            // buffer[2][i] = SET_BIT(buffer[2][i], mask2, GET_BIT(d[0][i], mask2p));
            // buffer[3][i] = SET_BIT(buffer[3][i], mask2, GET_BIT(d[1][i], mask2p));
        }
    }

    // gettimeofday(&end, NULL); //stop timer here
    // timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    // printf("Runtime for CarryBuffer2 with data size %d = %.6lf ms\n", size, (double)(timer * 0.001));
}

void CarryBuffer_Lint(Lint **a_prime, Lint **b_prime, Lint **d, uint **index_array, uint size, uint k, uint numShares) {

    Lint i, j, mask1, mask2, mask1p, mask2p;

    for (i = 0; i < size; i++) {
        for (j = 0; j < k; j++) {

            // used to set the bits in the correct positions in buffer
            mask1 = 2 * j;
            mask2 = 2 * j + 1;

            // used to get the correct bits from d
            mask1p = index_array[0][j];
            mask2p = index_array[1][j];

            for (Lint s = 0; s < numShares; s++) {
                a_prime[s][i] = SET_BIT(a_prime[s][i], mask1, GET_BIT(d[s][i], mask1p));
                a_prime[s][i] = SET_BIT(a_prime[s][i], mask2, GET_BIT(d[numShares + s][i], mask1p));
                b_prime[s][i] = SET_BIT(b_prime[s][i], mask1, GET_BIT(d[s][i], mask2p));
                b_prime[s][i] = SET_BIT(b_prime[s][i], mask2, GET_BIT(d[s][i], mask2p));
            }

            // a_prime[0][i] = SET_BIT(a_prime[0][i], mask1, GET_BIT(d[0][i], mask1p));
            // a_prime[1][i] = SET_BIT(a_prime[1][i], mask1, GET_BIT(d[1][i], mask1p));
            // a_prime[0][i] = SET_BIT(a_prime[0][i], mask2, GET_BIT(d[2][i], mask1p));
            // a_prime[1][i] = SET_BIT(a_prime[1][i], mask2, GET_BIT(d[3][i], mask1p));

            // b_prime[0][i] = SET_BIT(b_prime[0][i], mask1, GET_BIT(d[0][i], mask2p));
            // b_prime[1][i] = SET_BIT(b_prime[1][i], mask1, GET_BIT(d[1][i], mask2p));
            // b_prime[0][i] = SET_BIT(b_prime[0][i], mask2, GET_BIT(d[0][i], mask2p));
            // b_prime[1][i] = SET_BIT(b_prime[1][i], mask2, GET_BIT(d[1][i], mask2p));
        }
    }
}


// need way to tell which parties' bitwise shares we're adding
// e.g. 1st add p_1 and p_2 to get p'
// then add p' and p_3
void Rss_nBitAdd_5pc(Lint **res, Lint **r_bitwise, uint ring_size, uint size, NodeNetwork *nodeNet) {
    uint s;

    uint i, j;
    uint numShares = nodeNet->getNumShares();
    uint threshold = nodeNet->getThreshold();
    uint rounds = ceil(log2(threshold + 1));
    // printf("rounds: %u\n", rounds);
    Lint **a = new Lint *[numShares];
    Lint **b = new Lint *[numShares];
    Lint **c = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        a[i] = new Lint[size];
        memset(a[i], 0, sizeof(Lint) * size);
        b[i] = new Lint[size];
        memset(b[i], 0, sizeof(Lint) * size);
        c[i] = new Lint[size];
        memset(c[i], 0, sizeof(Lint) * size);
    }
    // Lint *res_check = new Lint[size];
    // memset(res_check, 0, sizeof(Lint) * size);

    for (j = 0; j < rounds; j++) {

        // if this is the first iteration, we copy r_bitwise into a and b
        if (j == 0) {
            // copy p_1 and p_2 into a, b respectively
            // printf("mc1\n");
            for (s = 0; s < numShares; s++) {
                memcpy(a[s], r_bitwise[s] + 0 * size, size * sizeof(Lint));
                memcpy(b[s], r_bitwise[s] + 1 * size, size * sizeof(Lint));
            }
            Rss_BitAdd_5pc(res, a, b, ring_size, size, nodeNet);

        } else {
            // break;
            // for (i = 0; i < numShares; i++) {

            //     memset(b[i], 0, sizeof(Lint) * size);
            // }
            for (s = 0; s < numShares; s++) {
                memcpy(a[s], res[s], size * sizeof(Lint));
                memcpy(b[s], r_bitwise[s] + 2 * size, size * sizeof(Lint));
            }
            // printf("ba2\n");
            Rss_BitAdd_5pc(res, a, b, ring_size, size, nodeNet);

        }
    }
    // delete[] res_check;

    for (i = 0; i < numShares; i++) {
        delete[] a[i];
        delete[] c[i];
        delete[] b[i];
    }
    delete[] a;
    delete[] c;
    delete[] b;
}

// alternative BitAdd implementation when both a and b are secret shared
// used in edaBit
void Rss_BitAdd_5pc(Lint **res, Lint **a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet) {
    uint s;
    Lint i;
    // int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();

    Lint **d = new Lint *[2 * numShares];
    for (i = 0; i < 2 * numShares; i++) {
        d[i] = new Lint[size];
        memset(d[i], 0, sizeof(Lint) * size);
    }

    Rss_Mult_Bitwise_5pc(res, a, b, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (s = 0; s < numShares; s++) {
            d[s][i] = a[s][i] ^ b[s][i];
            // d[1][i] = a[1][i] ^ b[1][i];

            d[numShares + s][i] = res[s][i];
            // d[3][i] = res[1][i];
        }
    }

    // Rss_Open_Bitwise_5pc(res_check, c_test, size, ring_size, nodeNet);
    // for (int i = 0; i < size; i++) {
    //     printf("Mult_bw res_check[%i]  : %llu\n", i, res_check[i]);
    //     print_binary(res_check[i], ring_size);
    // }

    Rss_CircleOpL_mp_test(d, ring_size, size, nodeNet); // original
    // Rss_CircleOpL_5pc(d, ring_size, size, nodeNet); // original

    // Rss_CircleOpL_Lint_5pc(d, ring_size, size, nodeNet); // original

    for (i = 0; i < size; i++) {
        for (s = 0; s < numShares; s++) {
            res[s][i] = (a[s][i] ^ b[s][i]) ^ (d[numShares + s][i] << Lint(1));
        }
    }

    for (i = 0; i < 2 * numShares; i++) {
        delete[] d[i];
    }
    delete[] d;
}

// a is public, b is bitwise-shared
// res will be a bitwise shared output
void Rss_BitAdd_5pc(Lint **res, Lint *a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet) {
    uint s;

    Lint i;
    int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();

    Lint **d = new Lint *[2 * numShares];
    for (i = 0; i < 2 * numShares; i++) {
        d[i] = new Lint[size];
        memset(d[i], 0, sizeof(Lint) * size);
    }

    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;

    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
    if (pid == 1) {
        ai[0] = -1;
    } else if (pid == 4) {
        ai[5] = -1; // party 4's share 6
    } else if (pid == 5) {
        ai[3] = -1; // parthy 5's share 4
    }

    // Lint a1, a2;
    // switch (pid) {
    // case 1:
    //     a1 = -1;
    //     a2 = 0;
    //     break;
    // case 2:
    //     a1 = 0;
    //     a2 = 0;
    //     break;
    // case 3:
    //     a1 = 0;
    //     a2 = -1;
    //     break;
    // }
    for (i = 0; i < size; i++) {
        for (s = 0; s < numShares; s++) {
            d[s][i] = (a[i] & ai[s]) ^ b[s][i];
            // d[1][i] = (a[i] & a2) ^ b[1][i];

            d[numShares + s][i] = (a[i] & b[s][i]);
            // d[3][i] = (a[i] & b[1][i]);
        }
    }

    // gettimeofday(&start, NULL); //start timer here
    // Rss_CircleOpL_Lint(d, ring_size, size, map, nodeNet);
    Rss_CircleOpL_mp_test(d, ring_size, size, nodeNet);
    // gettimeofday(&end, NULL); //stop timer here
    // timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    // printf("Runtime for Rss_CircleOpL with data size %d = %.6lf ms\n", size, (double)(timer * 0.001));

    // we only need the values in the g position (indices 2 and 3)

    for (i = 0; i < size; i++) {
        for (s = 0; s < numShares; s++)
            res[s][i] = ((a[i] & ai[s]) ^ b[s][i]) ^ (d[numShares + s][i] << 1);
        // res[1][i] = ((a[i] & a2) ^ b[1][i]) ^ (d[3][i] << 1);
    }

    for (i = 0; i < 2 * numShares; i++) {
        delete[] d[i];
    }
    delete[] d;
    delete[] ai;
}

void Rss_CircleOpL_5pc(Lint **d, uint r_size, uint size, NodeNetwork *nodeNet) {
    uint s;

    Lint i, j, l, y, z, op_r; // used for loops
    uint numShares = nodeNet->getNumShares();

    // printf("numShares : %u \n", numShares);
    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;

    // struct timeval start2;
    // struct timeval end2;
    // unsigned long timer2;

    if (r_size > 1) {

        // just three nested for-loops
        // r_size <=> k in algorithm

        uint num = ((r_size + 7) >> 3) * size;
        uint n_uints = ((r_size + 7) >> 3);
        uint t_index;
        Lint mask2, mask1m8, mask2m8;
        // Lint mask1p, mask2p;
        uint r_size_2 = pow(2, ceil(log2(r_size))); // rounding up to next power of two
        uint rounds = ceil(log2(r_size_2));

        uint **index_array = new uint *[2];
        Lint **buffer = new Lint *[2 * numShares];
        // we need at most (r_size + 7)/8 bytes to store bits from the buffer
        uint8_t **a = new uint8_t *[numShares];
        uint8_t **b = new uint8_t *[numShares];
        uint8_t **u = new uint8_t *[numShares];

        for (i = 0; i < 2; i++) {
            index_array[i] = new uint[r_size_2];
            memset(index_array[i], 0, sizeof(uint) * r_size_2);
        }
        for (i = 0; i < 2 * numShares; i++) {
            buffer[i] = new Lint[size];
            memset(buffer[i], 0, sizeof(Lint) * size);
        }

        for (i = 0; i < numShares; i++) {
            // memsets are actually needed here since are ORing
            a[i] = new uint8_t[num];
            memset(a[i], 0, sizeof(uint8_t) * num);
            b[i] = new uint8_t[num];
            memset(b[i], 0, sizeof(uint8_t) * num);
            u[i] = new uint8_t[num];
            memset(u[i], 0, sizeof(uint8_t) * num);
        }

        for (uint r = 1; r <= rounds; r++) {
            // gettimeofday(&start, NULL); //start timer here

            op_r = 0; // number of operations in a round
            // equivalent to the new_ring_size in MSB

            // gettimeofday(&start2, NULL); //start timer here

            for (j = 1; j <= ceil(r_size_2 / pow(2, r)); j++) {

                y = uint(pow(2, r - 1) + j * pow(2, r)) % r_size_2;

                for (z = 1; z <= (pow(2, r - 1)); z++) {
                    // checking we have a valid set of indices to add to our set
                    if ((((y % r_size_2)) <= r_size) && (((y + z) % (r_size_2 + 1)) <= r_size)) {
                        // printf("y : %u\n", y);
                        // printf("y+z : %u\n", y+z);
                        index_array[0][op_r] = (y % r_size_2) - 1;
                        index_array[1][op_r] = ((y + z) % (r_size_2 + 1)) - 1;
                        op_r++;
                    }
                }
            }

            // gettimeofday(&end2, NULL); //stop timer here
            // timer2 = 1000000 * (end2.tv_sec - start2.tv_sec) + end2.tv_usec - start2.tv_usec;
            // printf("Runtime for index_array with data size %d = %.6lf ms\n", size, (double)(timer2 * 0.001));

            // updating parameters for optimization
            n_uints = ((2 * op_r + 7) >> 3);
            num = ((2 * op_r + 7) >> 3) * size;

            // extracting terms into buffer
            CarryBuffer2(buffer, d, index_array, size, op_r, numShares);

            // gettimeofday(&start2, NULL); //start timer here
            // Splitting the buffer into bytes

            // THIS DOESNT WORK
            // DO NOT TRY
            // memcpy(a[0], buffer[0], size*n_uints);
            // memcpy(a[1], buffer[1], size*n_uints);
            // memcpy(b[0], buffer[2], size*n_uints);
            // memcpy(b[1], buffer[3], size*n_uints);

            for (j = 0; j < size; ++j) {
                for (s = 0; s < numShares; s++) {
                    memcpy(a[s] + j * n_uints, buffer[s] + j, n_uints);
                    // memcpy(a[1] + j * n_uints, buffer[1] + j, n_uints);
                    memcpy(b[s] + j * n_uints, buffer[numShares + s] + j, n_uints);
                    // memcpy(b[1] + j * n_uints, buffer[3] + j, n_uints);
                }
            }

            // gettimeofday(&end2, NULL); //stop timer here
            // timer2 = 1000000 * (end2.tv_sec - start2.tv_sec) + end2.tv_usec - start2.tv_usec;
            // printf("Runtime for memcpy with data size %d = %.6lf ms\n", size, (double)(timer2 * 0.001));

            // gettimeofday(&start2, NULL); //start timer here

            // bitwise multiplication
            Rss_Mult_Byte_5pc(u, a, b, num, nodeNet);
            // gettimeofday(&end2, NULL); //stop timer here
            // timer2 = 1000000 * (end2.tv_sec - start2.tv_sec) + end2.tv_usec - start2.tv_usec;
            // printf("Runtime for Rss_Mult_Byte with data size %d = %.6lf ms\n", size, (double)(timer2 * 0.001));

            // gettimeofday(&start2, NULL); //start timer here

            for (l = 0; l < size; ++l) {
                for (j = 0; j < op_r; ++j) {
                    // loop constants
                    t_index = (j >> 2) + (l * n_uints);
                    mask2 = index_array[1][j];
                    mask1m8 = (2 * j) & 7; // "&7" = %8, used for leftover bits
                    mask2m8 = (2 * j + 1) & 7;

                    for (s = 0; s < numShares; s++) {
                        d[s][l] = SET_BIT(d[s][l], mask2, GET_BIT(u[s][t_index], mask1m8));
                        // d[1][l] = SET_BIT(d[1][l], mask2, GET_BIT(u[1][t_index], mask1m8));

                        // simplified from needing two separate loops
                        d[numShares + s][l] = SET_BIT(d[numShares + s][l], mask2, (GET_BIT(u[s][t_index], mask2m8) ^ GET_BIT(d[numShares + s][l], mask2)));
                        // d[3][l] = SET_BIT(d[3][l], mask2, (GET_BIT(u[1][t_index], mask2m8) ^ GET_BIT(d[3][l], mask2)));
                    }
                }
            }

            // gettimeofday(&end2, NULL); //stop timer here
            // timer2 = 1000000 * (end2.tv_sec - start2.tv_sec) + end2.tv_usec - start2.tv_usec;
            // printf("Runtime for rearranging/circleOp with data size %d = %.6lf ms\n", size, (double)(timer2 * 0.001));

            // gettimeofday(&end, NULL); //stop timer here
            // timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
            // printf("Runtime for round %i with data size %d = %.6lf ms\n\n", i, size, (double)(timer * 0.001));
        }

        for (i = 0; i < numShares; i++) {
            delete[] a[i];
            delete[] b[i];
            delete[] u[i];
        }
        for (i = 0; i < 2 * numShares; i++) {
            delete[] buffer[i];
        }

        for (i = 0; i < 2; i++) {
            delete[] index_array[i];
        }
        delete[] a;
        delete[] b;
        delete[] u;
        delete[] index_array;
        delete[] buffer;
    }
}

void Rss_CircleOpL_mp_test(Lint **d, uint r_size, uint size, NodeNetwork *nodeNet) {
    uint s;

    Lint i, j, l, y, z, op_r; // used for loops
    uint numShares = nodeNet->getNumShares();
    // printf("numShares : %u \n", numShares);

    // printf("CIRCLE OP START -------\n");
    // struct timeval start2;
    // struct timeval end2;
    // unsigned long timer2;

    if (r_size > 1) {

        // just three nested for-loops
        // r_size <=> k in algorithm

        uint num = ((r_size + 7) >> 3) * size;
        uint n_uints = ((r_size + 7) >> 3);
        uint t_index;
        Lint mask2, mask1m8, mask2m8;
        // Lint mask1p, mask2p;
        uint r_size_2 = pow(2, ceil(log2(r_size))); // rounding up to next power of two
        uint rounds = ceil(log2(r_size_2));

        uint **index_array = new uint *[2];

        Lint **buffer = new Lint *[2 * numShares];
        // we need at most (r_size + 7)/8 bytes to store bits from the buffer
        uint8_t **a = new uint8_t *[numShares];
        uint8_t **b = new uint8_t *[numShares];
        uint8_t **u = new uint8_t *[numShares];

        for (i = 0; i < 2; i++) {
            index_array[i] = new uint[r_size_2];
            memset(index_array[i], 0, sizeof(uint) * r_size_2);
        }
        for (i = 0; i < 2 * numShares; i++) {
            buffer[i] = new Lint[size];
            memset(buffer[i], 0, sizeof(Lint) * size);
        }
        for (i = 0; i < numShares; i++) {
            // buffer[2 * i + 1] = new Lint[size];
            // memset(buffer[2 * i + 1], 0, sizeof(Lint) * size);
            // memsets are actually needed here since are ORing
            a[i] = new uint8_t[num];
            memset(a[i], 0, sizeof(uint8_t) * num);
            b[i] = new uint8_t[num];
            memset(b[i], 0, sizeof(uint8_t) * num);
            u[i] = new uint8_t[num];
            memset(u[i], 0, sizeof(uint8_t) * num);
        }

        for (uint r = 1; r <= rounds; r++) {
            for (int ii = 0; ii < numShares; ii++) {
                memset(a[ii], 0, sizeof(uint8_t) * num);
                memset(b[ii], 0, sizeof(uint8_t) * num);
                memset(u[ii], 0, sizeof(uint8_t) * num);
            }
            for (int ii = 0; ii < 2 * numShares; ii++) {
                memset(buffer[ii], 0, sizeof(Lint) * size);
            }

          

            op_r = 0; // number of operations in a round
            // equivalent to the new_ring_size in MSB

            // gettimeofday(&start2, NULL); //start timer here

            for (j = 1; j <= ceil(r_size_2 / pow(2, r)); j++) {

                y = uint(pow(2, r - 1) + j * pow(2, r)) % r_size_2;

                for (z = 1; z <= (pow(2, r - 1)); z++) {
                    // checking we have a valid set of indices to add to our set
                    if ((((y % r_size_2)) <= r_size) && (((y + z) % (r_size_2 + 1)) <= r_size)) {
                        // printf("y : %u\n", y);
                        // printf("y+z : %u\n", y+z);
                        index_array[0][op_r] = (y % r_size_2) - 1;
                        index_array[1][op_r] = ((y + z) % (r_size_2 + 1)) - 1;
                        op_r++;
                    }
                }
            }


            // updating parameters for optimization
            n_uints = ((2 * op_r + 7) >> 3);
            num = ((2 * op_r + 7) >> 3) * size;



            // extracting terms into buffer
            CarryBuffer2(buffer, d, index_array, size, op_r, numShares);

            // for (j = 0; j < size; ++j) {
            //     for (uint s = 0; s <  numShares; s++) {
            //         printf("buffer[%i][%i]  : %llu -- ", s, j, buffer[s][j]);
            //         print_binary(buffer[s][j], r_size);
            //     }
            //     printf("\n");
            //     for (uint s = 0; s < numShares; s++) {
            //         printf("buffer[%i][%i]  : %llu -- ", numShares + s, j, buffer[numShares + s][j]);
            //         print_binary(buffer[numShares + s][j], r_size);
            //     }
            // }

            // gettimeofday(&start2, NULL); //start timer here
            // Splitting the buffer into bytes

            for (j = 0; j < size; ++j) {
                for (s = 0; s < numShares; s++) {
                    // printf("s: %llu\n", s);
                    // printf("numShares + s: %llu\n", numShares + s);
                    memcpy(a[s] + j * n_uints, buffer[s] + j, n_uints);
                    memcpy(b[s] + j * n_uints, buffer[numShares + s] + j, n_uints);
                }

                //     memcpy(a[0] + j * n_uints, buffer[0] + j, n_uints);
                //     memcpy(a[1] + j * n_uints, buffer[1] + j, n_uints);
                //     memcpy(a[2] + j * n_uints, buffer[2] + j, n_uints);
                //     memcpy(a[3] + j * n_uints, buffer[3] + j, n_uints);
                //     memcpy(a[4] + j * n_uints, buffer[4] + j, n_uints);
                //     memcpy(a[5] + j * n_uints, buffer[5] + j, n_uints);

                //     memcpy(b[0] + j * n_uints, buffer[6] + j, n_uints);
                //     memcpy(b[1] + j * n_uints, buffer[7] + j, n_uints);
                //     memcpy(b[2] + j * n_uints, buffer[8] + j, n_uints);
                //     memcpy(b[3] + j * n_uints, buffer[9] + j, n_uints);
                //     memcpy(b[4] + j * n_uints, buffer[10] + j, n_uints);
                //     memcpy(b[5] + j * n_uints, buffer[11] + j, n_uints);
            }

            Rss_Mult_Byte_5pc(u, a, b, num, nodeNet);

            // gettimeofday(&end2, NULL); //stop timer here
            // timer2 = 1000000 * (end2.tv_sec - start2.tv_sec) + end2.tv_usec - start2.tv_usec;
            // printf("Runtime for Rss_Mult_Byte with data size %d = %.6lf ms\n", size, (double)(timer2 * 0.001));

            // gettimeofday(&start2, NULL); //start timer here

            for (l = 0; l < size; ++l) {
                for (j = 0; j < op_r; ++j) {
                    // loop constants
                    t_index = (j >> 2) + (l * n_uints);
                    mask2 = index_array[1][j];
                    mask1m8 = (2 * j) & 7; // "&7" = %8, used for leftover bits
                    mask2m8 = (2 * j + 1) & 7;

                    // printf("j: %u, ", j);
                    // printf("t_index: %u, ", t_index);
                    // printf("mask2: %u, ", mask2);
                    // printf("mask1m8: %u, ", mask1m8);
                    // printf("mask2m8: %u\n", mask2m8);
                    // for (s = 0; s < numShares; s++) {
                    //     d[s][l] = SET_BIT(d[s][l], mask2, GET_BIT(u[s][t_index], mask1m8));

                    //     d[numShares + s][l] = SET_BIT(d[numShares + s][l], mask2, (GET_BIT(u[s][t_index], mask2m8) ^ GET_BIT(d[numShares + s][l], mask2)));
                    // }
                    for (s = 0; s < numShares; s++) {
                        d[s][l] = SET_BIT(d[s][l], mask2, GET_BIT(u[s][t_index], mask1m8));
                        // d[1][l] = SET_BIT(d[1][l], mask2, GET_BIT(u[1][t_index], mask1m8));

                        // simplified from needing two separate loops
                        d[numShares + s][l] = SET_BIT(d[numShares + s][l], mask2, (GET_BIT(u[s][t_index], mask2m8) ^ GET_BIT(d[numShares + s][l], mask2)));
                        // d[3][l] = SET_BIT(d[3][l], mask2, (GET_BIT(u[1][t_index], mask2m8) ^ GET_BIT(d[3][l], mask2)));
                    }
                }
            }

            // only doing one round for testing
            // gettimeofday(&end2, NULL); //stop timer here
            // timer2 = 1000000 * (end2.tv_sec - start2.tv_sec) + end2.tv_usec - start2.tv_usec;
            // printf("Runtime for rearranging/circleOp with data size %d = %.6lf ms\n", size, (double)(timer2 * 0.001));

            // gettimeofday(&end, NULL); //stop timer here
            // timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
            // printf("Runtime for round %i with data size %d = %.6lf ms\n\n", i, size, (double)(timer * 0.001));
        }
        for (i = 0; i < 2 * numShares; i++) {
            delete[] buffer[i];
        }
        for (i = 0; i < numShares; i++) {
            delete[] a[i];
            delete[] b[i];
            delete[] u[i];
        }
        for (i = 0; i < 2; i++) {
            delete[] index_array[i];
        }

        delete[] a;
        delete[] b;
        delete[] u;
        delete[] index_array;
        delete[] buffer;
    }
}

// void CarryBuffer2_test(Lint **buffer, Lint **d, uint **index_array, uint size, uint k, uint numShares) {
//     // prepares input u for multiplication
//     // extracts p2i, p2i-1, and g2i
//     // buffer and d are the same size (4 x size)

//     // struct timeval start;
//     // struct timeval end;
//     // unsigned long timer;

//     Lint i, j, mask1, mask2, mask1p, mask2p;
//     // uint s;

//     // gettimeofday(&start, NULL); //start timer here

//     for (i = 0; i < size; i++) {
//         for (j = 0; j < k; j++) {

//             // used to set the bits in the correct positions in buffer
//             mask1 = Lint(2) * j;
//             mask2 = Lint(2) * j + Lint(1);

//             // used to get the correct bits from d
//             mask1p = Lint(index_array[0][j]);
//             mask2p = Lint(index_array[1][j]);
//             // for (Lint s = 0; s < numShares; s++) {
//             //     buffer[s][i] = SET_BIT(buffer[s][i], mask1, GET_BIT(d[s][i], mask1p));
//             //     buffer[s][i] = SET_BIT(buffer[s][i], mask2, GET_BIT(d[numShares + s][i], mask1p));
//             //     buffer[numShares + s][i] = SET_BIT(buffer[numShares + s][i], mask1, GET_BIT(d[s][i], mask2p));
//             //     buffer[numShares + s][i] = SET_BIT(buffer[numShares + s][i], mask2, GET_BIT(d[s][i], mask2p));
//             // }

//             buffer[0][i] = SET_BIT(buffer[0][i], mask1, GET_BIT(d[0][i], mask1p));
//             buffer[1][i] = SET_BIT(buffer[1][i], mask1, GET_BIT(d[1][i], mask1p));
//             buffer[2][i] = SET_BIT(buffer[2][i], mask1, GET_BIT(d[2][i], mask1p));
//             buffer[3][i] = SET_BIT(buffer[3][i], mask1, GET_BIT(d[3][i], mask1p));
//             buffer[4][i] = SET_BIT(buffer[4][i], mask1, GET_BIT(d[4][i], mask1p));
//             buffer[5][i] = SET_BIT(buffer[5][i], mask1, GET_BIT(d[5][i], mask1p));

//             buffer[0][i] = SET_BIT(buffer[0][i], mask2, GET_BIT(d[6][i], mask1p));
//             buffer[1][i] = SET_BIT(buffer[1][i], mask2, GET_BIT(d[7][i], mask1p));
//             buffer[2][i] = SET_BIT(buffer[2][i], mask2, GET_BIT(d[8][i], mask1p));
//             buffer[3][i] = SET_BIT(buffer[3][i], mask2, GET_BIT(d[9][i], mask1p));
//             buffer[4][i] = SET_BIT(buffer[4][i], mask2, GET_BIT(d[10][i], mask1p));
//             buffer[5][i] = SET_BIT(buffer[5][i], mask2, GET_BIT(d[11][i], mask1p));

//             buffer[6][i] = SET_BIT(buffer[6][i], mask1, GET_BIT(d[0][i], mask2p));
//             buffer[7][i] = SET_BIT(buffer[7][i], mask1, GET_BIT(d[1][i], mask2p));
//             buffer[8][i] = SET_BIT(buffer[8][i], mask1, GET_BIT(d[2][i], mask2p));
//             buffer[9][i] = SET_BIT(buffer[9][i], mask1, GET_BIT(d[3][i], mask2p));
//             buffer[10][i] = SET_BIT(buffer[10][i], mask1, GET_BIT(d[4][i], mask2p));
//             buffer[11][i] = SET_BIT(buffer[11][i], mask1, GET_BIT(d[5][i], mask2p));

//             buffer[6][i] = SET_BIT(buffer[6][i], mask2, GET_BIT(d[0][i], mask2p));
//             buffer[7][i] = SET_BIT(buffer[7][i], mask2, GET_BIT(d[1][i], mask2p));
//             buffer[8][i] = SET_BIT(buffer[8][i], mask2, GET_BIT(d[2][i], mask2p));
//             buffer[9][i] = SET_BIT(buffer[9][i], mask2, GET_BIT(d[3][i], mask2p));
//             buffer[10][i] = SET_BIT(buffer[10][i], mask2, GET_BIT(d[4][i], mask2p));
//             buffer[11][i] = SET_BIT(buffer[11][i], mask2, GET_BIT(d[5][i], mask2p));

//             // buffer[2][i] = SET_BIT(buffer[2][i], mask1, GET_BIT(d[0][i], mask2p));
//             // buffer[3][i] = SET_BIT(buffer[3][i], mask1, GET_BIT(d[1][i], mask2p));
//             // buffer[2][i] = SET_BIT(buffer[2][i], mask2, GET_BIT(d[0][i], mask2p));
//             // buffer[3][i] = SET_BIT(buffer[3][i], mask2, GET_BIT(d[1][i], mask2p));
//         }
//     }

//     // gettimeofday(&end, NULL); //stop timer here
//     // timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
//     // printf("Runtime for CarryBuffer2 with data size %d = %.6lf ms\n", size, (double)(timer * 0.001));
// }

void Rss_CircleOpL_Lint_5pc(Lint **d, uint r_size, uint size, NodeNetwork *nodeNet) {

    uint s;

    Lint i, j, l, y, z, op_r; // used for loops
    struct timeval start;
    struct timeval end;
    unsigned long timer;
    uint numShares = nodeNet->getNumShares();

    if (r_size > 1) {

        Lint mask2, mask1m8, mask2m8;
        uint r_size_2 = pow(2, ceil(log2(r_size))); // rounding up to next power of two
        uint rounds = ceil(log2(r_size_2));

        uint **index_array = new uint *[2];
        Lint **a_prime = new Lint *[numShares];
        Lint **b_prime = new Lint *[numShares];
        Lint **u_prime = new Lint *[numShares];

        for (i = 0; i < 2; i++) {
            index_array[i] = new uint[r_size_2];
        }
        for (i = 0; i < numShares; i++) {
            a_prime[i] = new Lint[size];
            b_prime[i] = new Lint[size];
            u_prime[i] = new Lint[size];
        }

        for (i = 1; i <= rounds; i++) {
            // gettimeofday(&start, NULL); // start timer here

            op_r = 0; // number of operations in a round
            // equivalent to the new_ring_size in MSB

            for (j = 1; j <= ceil(r_size_2 / pow(2, i)); j++) {

                y = uint(pow(2, i - 1) + j * pow(2, i)) % r_size_2;

                for (z = 1; z <= (pow(2, i - 1)); z++) {
                    // checking we have a valid set of indices to add to our set
                    if ((((y % r_size_2)) <= r_size) && (((y + z) % (r_size_2 + 1)) <= r_size)) {
                        // printf("y : %u\n", y);
                        // printf("y+z : %u\n", y+z);
                        index_array[0][op_r] = (y % r_size_2) - 1;
                        index_array[1][op_r] = ((y + z) % (r_size_2 + 1)) - 1;
                        op_r++;
                    }
                }
            }

            // extracting terms into a_prime and b_prime directly
            CarryBuffer_Lint(a_prime, b_prime, d, index_array, size, op_r, numShares);

            // bitwise multiplication
            Rss_Mult_Bitwise_5pc(u_prime, a_prime, b_prime, size, r_size, nodeNet);

            // printf("adding g2j+1\n");
            for (l = 0; l < size; ++l) {
                for (j = 0; j < op_r; ++j) {

                    // loop constants
                    // DO NOT REMOVE mask2
                    // putting it directly in next  operations
                    // of SET BIT causes it to FAIL
                    // DONT ASK WHY

                    mask2 = index_array[1][j]; // CHECK
                    mask1m8 = (2 * j);         // "&7" = %8, used for leftover bits
                    mask2m8 = (2 * j + 1);

                    for (s = 0; s < numShares; s++) {
                        d[s][l] = SET_BIT(d[s][l], mask2, GET_BIT(u_prime[s][l], mask1m8));
                        d[numShares + s][l] = SET_BIT(d[numShares + s][l], mask2, (GET_BIT(u_prime[s][l], mask2m8) ^ GET_BIT(d[numShares + s][l], mask2)));
                    }

                    // d[0][l] = SET_BIT(d[0][l], mask2, GET_BIT(u_prime[0][l], mask1m8));
                    // d[1][l] = SET_BIT(d[1][l], mask2, GET_BIT(u_prime[1][l], mask1m8));

                    // d[2][l] = SET_BIT(d[2][l], mask2, (GET_BIT(u_prime[0][l], mask2m8) ^ GET_BIT(d[2][l], mask2)));
                    // d[3][l] = SET_BIT(d[3][l], mask2, (GET_BIT(u_prime[1][l], mask2m8) ^ GET_BIT(d[3][l], mask2)));
                }
            }
            // gettimeofday(&end, NULL); // stop timer here
            timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
            // printf("Runtime for round with data size %d = %.6lf ms\n", size, (double)(timer * 0.001));
        }
        for (i = 0; i < 2; i++) {
            delete[] a_prime[i];
            delete[] b_prime[i];
            delete[] u_prime[i];
            delete[] index_array[i];
        }
        delete[] a_prime;
        delete[] b_prime;
        delete[] u_prime;
        delete[] index_array;
    }
}



// need way to tell which parties' bitwise shares we're adding
// e.g. 1st add p_1 and p_2 to get p'
// then add p' and p_3
void Rss_nBitAdd_7pc(Lint **res, Lint **r_bitwise, uint ring_size, uint size, NodeNetwork *nodeNet) {
    uint s;

    uint i, j;
    uint numShares = nodeNet->getNumShares();
    uint threshold = nodeNet->getThreshold();
    uint rounds = ceil(log2(threshold + 1));
    // printf("rounds: %u\n", rounds);
    Lint **a = new Lint *[numShares];
    Lint **b = new Lint *[numShares];
    Lint **c = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        a[i] = new Lint[2 * size];
        memset(a[i], 0, sizeof(Lint)* 2 * size);
        b[i] = new Lint[2 * size];
        memset(b[i], 0, sizeof(Lint)* 2 * size);
        c[i] = new Lint[2 * size];
        memset(c[i], 0, sizeof(Lint)* 2 * size);
    }
    // Lint *res_check = new Lint[size];
    // memset(res_check, 0, sizeof(Lint) * size);

    for (j = 0; j < rounds; j++) {

        // if this is the first iteration, we copy r_bitwise into a and b
        if (j == 0) {
            // copy p_1 and p_2 into a, b respectively
            // printf("mc1\n");
            for (s = 0; s < numShares; s++) {
                memcpy(a[s], r_bitwise[s] + 0 * size, 2 * size * sizeof(Lint));
                memcpy(b[s], r_bitwise[s] + 2 * size, 2 * size * sizeof(Lint));
            }
            Rss_BitAdd_7pc(c, a, b, ring_size, 2*size, nodeNet);
        } else {
            for (s = 0; s < numShares; s++) {
                memcpy(a[s], c[s], size * sizeof(Lint));
                memcpy(b[s], c[s] + 1 * size, size * sizeof(Lint));
            }
            Rss_BitAdd_7pc(res, a, b, ring_size, size, nodeNet);

        }
    }
    // delete[] res_check;

    for (i = 0; i < numShares; i++) {
        delete[] a[i];
        delete[] c[i];
        delete[] b[i];
    }
    delete[] a;
    delete[] c;
    delete[] b;
}

// alternative BitAdd implementation when both a and b are secret shared
// used in edaBit
void Rss_BitAdd_7pc(Lint **res, Lint **a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet) {
    uint s;
    Lint i;
    // int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();

    Lint **d = new Lint *[2 * numShares];
    for (i = 0; i < 2 * numShares; i++) {
        d[i] = new Lint[size];
        memset(d[i], 0, sizeof(Lint) * size);
    }

    Rss_Mult_Bitwise_7pc(res, a, b, size, ring_size, nodeNet);

    for (i = 0; i < size; i++) {
        for (s = 0; s < numShares; s++) {
            d[s][i] = a[s][i] ^ b[s][i];
            // d[1][i] = a[1][i] ^ b[1][i];

            d[numShares + s][i] = res[s][i];
            // d[3][i] = res[1][i];
        }
    }

    Rss_CircleOpL_7pc_test(d, ring_size, size, nodeNet); // original

    // Rss_CircleOpL_Lint_7pc(d, ring_size, size, nodeNet); // original

    for (i = 0; i < size; i++) {
        for (s = 0; s < numShares; s++) {
            res[s][i] = (a[s][i] ^ b[s][i]) ^ (d[numShares + s][i] << Lint(1));
        }
    }

    for (i = 0; i < 2 * numShares; i++) {
        delete[] d[i];
    }
    delete[] d;
}

// a is public, b is bitwise-shared
// res will be a bitwise shared output
void Rss_BitAdd_7pc(Lint **res, Lint *a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet) {
    uint s;

    Lint i;
    int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();

    Lint **d = new Lint *[2 * numShares];
    for (i = 0; i < 2 * numShares; i++) {
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

    for (i = 0; i < size; i++) {
        for (s = 0; s < numShares; s++) {
            d[s][i] = (a[i] & ai[s]) ^ b[s][i];

            d[numShares + s][i] = (a[i] & b[s][i]);
            // d[3][i] = (a[i] & b[1][i]);
        }
    }

    Rss_CircleOpL_7pc_test(d, ring_size, size, nodeNet);

    for (i = 0; i < size; i++) {
        for (s = 0; s < numShares; s++)
            res[s][i] = ((a[i] & ai[s]) ^ b[s][i]) ^ (d[numShares + s][i] << 1);
        // res[1][i] = ((a[i] & a2) ^ b[1][i]) ^ (d[3][i] << 1);
    }

    for (i = 0; i < 2 * numShares; i++) {
        delete[] d[i];
    }
    delete[] d;
    delete[] ai;
}

void Rss_CircleOpL_7pc(Lint **d, uint r_size, uint size, NodeNetwork *nodeNet) {
    uint s;

    Lint i, j, l, y, z, op_r; // used for loops
    uint numShares = nodeNet->getNumShares();

    if (r_size > 1) {

        // just three nested for-loops
        // r_size <=> k in algorithm

        uint num = ((r_size + 7) >> 3) * size;
        uint n_uints = ((r_size + 7) >> 3);
        uint t_index;
        Lint mask2, mask1m8, mask2m8;
        // Lint mask1p, mask2p;
        uint r_size_2 = pow(2, ceil(log2(r_size))); // rounding up to next power of two
        uint rounds = ceil(log2(r_size_2));

        uint **index_array = new uint *[2];
        Lint **buffer = new Lint *[2 * numShares];
        // we need at most (r_size + 7)/8 bytes to store bits from the buffer
        uint8_t **a = new uint8_t *[numShares];
        uint8_t **b = new uint8_t *[numShares];
        uint8_t **u = new uint8_t *[numShares];

        for (i = 0; i < 2; i++) {
            index_array[i] = new uint[r_size_2];
            memset(index_array[i], 0, sizeof(uint) * r_size_2);
        }
        for (i = 0; i < 2 * numShares; i++) {
            buffer[i] = new Lint[size];
            memset(buffer[i], 0, sizeof(Lint) * size);
        }

        for (i = 0; i < numShares; i++) {
            // memsets are actually needed here since are ORing
            a[i] = new uint8_t[num];
            memset(a[i], 0, sizeof(uint8_t) * num);
            b[i] = new uint8_t[num];
            memset(b[i], 0, sizeof(uint8_t) * num);
            u[i] = new uint8_t[num];
            memset(u[i], 0, sizeof(uint8_t) * num);
        }

        for (uint r = 1; r <= rounds; r++) {
            // gettimeofday(&start, NULL); //start timer here

            op_r = 0; // number of operations in a round
            // equivalent to the new_ring_size in MSB

            // gettimeofday(&start2, NULL); //start timer here

            for (j = 1; j <= ceil(r_size_2 / pow(2, r)); j++) {

                y = uint(pow(2, r - 1) + j * pow(2, r)) % r_size_2;

                for (z = 1; z <= (pow(2, r - 1)); z++) {
                    // checking we have a valid set of indices to add to our set
                    if ((((y % r_size_2)) <= r_size) && (((y + z) % (r_size_2 + 1)) <= r_size)) {
                        // printf("y : %u\n", y);
                        // printf("y+z : %u\n", y+z);
                        index_array[0][op_r] = (y % r_size_2) - 1;
                        index_array[1][op_r] = ((y + z) % (r_size_2 + 1)) - 1;
                        op_r++;
                    }
                }
            }

            // gettimeofday(&end2, NULL); //stop timer here
            // timer2 = 1000000 * (end2.tv_sec - start2.tv_sec) + end2.tv_usec - start2.tv_usec;
            // printf("Runtime for index_array with data size %d = %.6lf ms\n", size, (double)(timer2 * 0.001));

            // updating parameters for optimization
            n_uints = ((2 * op_r + 7) >> 3);
            num = ((2 * op_r + 7) >> 3) * size;

            // extracting terms into buffer
            CarryBuffer2(buffer, d, index_array, size, op_r, numShares);

            for (j = 0; j < size; ++j) {
                for (s = 0; s < numShares; s++) {
                    memcpy(a[s] + j * n_uints, buffer[s] + j, n_uints);
                    // memcpy(a[1] + j * n_uints, buffer[1] + j, n_uints);
                    memcpy(b[s] + j * n_uints, buffer[numShares + s] + j, n_uints);
                    // memcpy(b[1] + j * n_uints, buffer[3] + j, n_uints);
                }
            }

            // bitwise multiplication
            Rss_Mult_Byte_7pc(u, a, b, num, nodeNet);

            for (l = 0; l < size; ++l) {
                for (j = 0; j < op_r; ++j) {
                    // loop constants
                    t_index = (j >> 2) + (l * n_uints);
                    mask2 = index_array[1][j];
                    mask1m8 = (2 * j) & 7; // "&7" = %8, used for leftover bits
                    mask2m8 = (2 * j + 1) & 7;

                    for (s = 0; s < numShares; s++) {
                        d[s][l] = SET_BIT(d[s][l], mask2, GET_BIT(u[s][t_index], mask1m8));
                        // d[1][l] = SET_BIT(d[1][l], mask2, GET_BIT(u[1][t_index], mask1m8));

                        // simplified from needing two separate loops
                        d[numShares + s][l] = SET_BIT(d[numShares + s][l], mask2, (GET_BIT(u[s][t_index], mask2m8) ^ GET_BIT(d[numShares + s][l], mask2)));
                        // d[3][l] = SET_BIT(d[3][l], mask2, (GET_BIT(u[1][t_index], mask2m8) ^ GET_BIT(d[3][l], mask2)));
                    }
                }
            }
        }

        for (i = 0; i < numShares; i++) {
            delete[] a[i];
            delete[] b[i];
            delete[] u[i];
        }
        for (i = 0; i < 2 * numShares; i++) {
            delete[] buffer[i];
        }

        for (i = 0; i < 2; i++) {
            delete[] index_array[i];
        }
        delete[] a;
        delete[] b;
        delete[] u;
        delete[] index_array;
        delete[] buffer;
    }
}

void Rss_CircleOpL_7pc_test(Lint **d, uint r_size, uint size, NodeNetwork *nodeNet) {
    uint s;

    Lint i, j, l, y, z, op_r; // used for loops
    uint numShares = nodeNet->getNumShares();
    // printf("numShares : %u \n", numShares);

    // printf("CIRCLE OP START -------\n");
    // struct timeval start2;
    // struct timeval end2;
    // unsigned long timer2;

    if (r_size > 1) {

        // just three nested for-loops
        // r_size <=> k in algorithm

        uint num = ((r_size + 7) >> 3) * size;
        uint n_uints = ((r_size + 7) >> 3);
        uint t_index;
        Lint mask2, mask1m8, mask2m8;
        // Lint mask1p, mask2p;
        uint r_size_2 = pow(2, ceil(log2(r_size))); // rounding up to next power of two
        uint rounds = ceil(log2(r_size_2));

        uint **index_array = new uint *[2];

        Lint **buffer = new Lint *[2 * numShares];
        // we need at most (r_size + 7)/8 bytes to store bits from the buffer
        uint8_t **a = new uint8_t *[numShares];
        uint8_t **b = new uint8_t *[numShares];
        uint8_t **u = new uint8_t *[numShares];

        for (i = 0; i < 2; i++) {
            index_array[i] = new uint[r_size_2];
            memset(index_array[i], 0, sizeof(uint) * r_size_2);
        }
        for (i = 0; i < 2 * numShares; i++) {
            buffer[i] = new Lint[size];
            memset(buffer[i], 0, sizeof(Lint) * size);
        }
        for (i = 0; i < numShares; i++) {
            // buffer[2 * i + 1] = new Lint[size];
            // memset(buffer[2 * i + 1], 0, sizeof(Lint) * size);
            // memsets are actually needed here since are ORing
            a[i] = new uint8_t[num];
            memset(a[i], 0, sizeof(uint8_t) * num);
            b[i] = new uint8_t[num];
            memset(b[i], 0, sizeof(uint8_t) * num);
            u[i] = new uint8_t[num];
            memset(u[i], 0, sizeof(uint8_t) * num);
        }

        for (uint r = 1; r <= rounds; r++) {
            for (int ii = 0; ii < numShares; ii++) {
                memset(a[ii], 0, sizeof(uint8_t) * num);
                memset(b[ii], 0, sizeof(uint8_t) * num);
                memset(u[ii], 0, sizeof(uint8_t) * num);
            }
            for (int ii = 0; ii < 2 * numShares; ii++) {
                memset(buffer[ii], 0, sizeof(Lint) * size);
            }

            // printf("ROUND %i\n-----------------------------\n", r);

            // printf("\n");
            // for (int ii = 0; ii < num; ii++) {
            //     for (uint s = 0; s < numShares; s++) {
            //         printf("a[%i][%i]  : %u \t", s, ii, a[s][ii]);
            //         print_binary(a[s][ii], 8);
            //     }
            // }
            // printf("\n");
            // for (int ii = 0; ii < num; ii++) {
            //     for (uint s = 0; s < numShares; s++) {
            //         printf("b[%i][%i]  : %u \t", s, ii, b[s][ii]);
            //         print_binary(b[s][ii], 8);
            //     }

            // }
            // gettimeofday(&start, NULL); //start timer here

            op_r = 0; // number of operations in a round
            // equivalent to the new_ring_size in MSB

            // gettimeofday(&start2, NULL); //start timer here

            for (j = 1; j <= ceil(r_size_2 / pow(2, r)); j++) {

                y = uint(pow(2, r - 1) + j * pow(2, r)) % r_size_2;

                for (z = 1; z <= (pow(2, r - 1)); z++) {
                    // checking we have a valid set of indices to add to our set
                    if ((((y % r_size_2)) <= r_size) && (((y + z) % (r_size_2 + 1)) <= r_size)) {
                        // printf("y : %u\n", y);
                        // printf("y+z : %u\n", y+z);
                        index_array[0][op_r] = (y % r_size_2) - 1;
                        index_array[1][op_r] = ((y + z) % (r_size_2 + 1)) - 1;
                        op_r++;
                    }
                }
            }

            // updating parameters for optimization
            n_uints = ((2 * op_r + 7) >> 3);
            num = ((2 * op_r + 7) >> 3) * size;

            // extracting terms into buffer
            CarryBuffer2(buffer, d, index_array, size, op_r, numShares);

            for (j = 0; j < size; ++j) {
                for (s = 0; s < numShares; s++) {
                    // printf("s: %llu\n", s);
                    // printf("numShares + s: %llu\n", numShares + s);
                    memcpy(a[s] + j * n_uints, buffer[s] + j, n_uints);
                    memcpy(b[s] + j * n_uints, buffer[numShares + s] + j, n_uints);
                }
            }

            Rss_Mult_Byte_7pc(u, a, b, num, nodeNet);

            // gettimeofday(&end2, NULL); //stop timer here
            // timer2 = 1000000 * (end2.tv_sec - start2.tv_sec) + end2.tv_usec - start2.tv_usec;
            // printf("Runtime for Rss_Mult_Byte with data size %d = %.6lf ms\n", size, (double)(timer2 * 0.001));

            // gettimeofday(&start2, NULL); //start timer here

            for (l = 0; l < size; ++l) {
                for (j = 0; j < op_r; ++j) {
                    // loop constants
                    t_index = (j >> 2) + (l * n_uints);
                    mask2 = index_array[1][j];
                    mask1m8 = (2 * j) & 7; // "&7" = %8, used for leftover bits
                    mask2m8 = (2 * j + 1) & 7;

                    // printf("j: %u, ", j);
                    // printf("t_index: %u, ", t_index);
                    // printf("mask2: %u, ", mask2);
                    // printf("mask1m8: %u, ", mask1m8);
                    // printf("mask2m8: %u\n", mask2m8);
                    // for (s = 0; s < numShares; s++) {
                    //     d[s][l] = SET_BIT(d[s][l], mask2, GET_BIT(u[s][t_index], mask1m8));

                    //     d[numShares + s][l] = SET_BIT(d[numShares + s][l], mask2, (GET_BIT(u[s][t_index], mask2m8) ^ GET_BIT(d[numShares + s][l], mask2)));
                    // }
                    for (s = 0; s < numShares; s++) {
                        d[s][l] = SET_BIT(d[s][l], mask2, GET_BIT(u[s][t_index], mask1m8));
                        // d[1][l] = SET_BIT(d[1][l], mask2, GET_BIT(u[1][t_index], mask1m8));

                        // simplified from needing two separate loops
                        d[numShares + s][l] = SET_BIT(d[numShares + s][l], mask2, (GET_BIT(u[s][t_index], mask2m8) ^ GET_BIT(d[numShares + s][l], mask2)));
                        // d[3][l] = SET_BIT(d[3][l], mask2, (GET_BIT(u[1][t_index], mask2m8) ^ GET_BIT(d[3][l], mask2)));
                    }
                }
            }

            // only doing one round for testing
            // gettimeofday(&end2, NULL); //stop timer here
            // timer2 = 1000000 * (end2.tv_sec - start2.tv_sec) + end2.tv_usec - start2.tv_usec;
            // printf("Runtime for rearranging/circleOp with data size %d = %.6lf ms\n", size, (double)(timer2 * 0.001));

            // gettimeofday(&end, NULL); //stop timer here
            // timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
            // printf("Runtime for round %i with data size %d = %.6lf ms\n\n", i, size, (double)(timer * 0.001));
        }
        for (i = 0; i < 2 * numShares; i++) {
            delete[] buffer[i];
        }
        for (i = 0; i < numShares; i++) {
            delete[] a[i];
            delete[] b[i];
            delete[] u[i];
        }
        for (i = 0; i < 2; i++) {
            delete[] index_array[i];
        }

        delete[] a;
        delete[] b;
        delete[] u;
        delete[] index_array;
        delete[] buffer;
    }
}
