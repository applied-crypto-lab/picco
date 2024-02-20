#include "edaBit.h"

void Rss_edaBit(Lint **r, Lint **b_2, uint size, uint ring_size, NodeNetwork *nodeNet) {

    // int pid = nodeNet->getID();
    uint numParties = nodeNet->getNumParties();
    // printf("numParties : %u\n",numParties);
    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;

    uint i;
    // need to multiply by the number of parties in the computation
    uint new_size = numParties * size;

    Lint **r_bitwise = new Lint *[2];
    for (i = 0; i < 2; i++) {
        r_bitwise[i] = new Lint[new_size];
        memset(r_bitwise[i], 0, sizeof(Lint) * new_size);

        // ensuring destinations are sanitized
        memset(r[i], 0, sizeof(Lint) * size);
        memset(b_2[i], 0, sizeof(Lint) * size);
    }

    Rss_GenerateRandomShares(r, r_bitwise, ring_size, size, nodeNet);

    Rss_nBitAdd(b_2, r_bitwise, ring_size, size, nodeNet);

    for (i = 0; i < 2; i++) {
        delete[] r_bitwise[i];
    }
    delete[] r_bitwise;
}

void Rss_GenerateRandomShares(Lint **res, Lint **res_bitwise, uint ring_size, uint size, NodeNetwork *nodeNet) {
    // printf("start\n");
    int pid = nodeNet->getID();
    uint i, j;
    uint bytes = (ring_size + 7) >> 3;
    // printf("bytes : %u \n", bytes);
    uint p_index = pid - 1;
    uint numParties = nodeNet->getNumParties();
    // printf("numParties : %u \n", numParties);

    // Lint temp0, temp1;

    // used since we have effectively double the number of values
    // since we need to represent both arithmetic and binary shares
    uint new_size = 2 * size; // DO NOT CHANGE, IT IS IRRELEVANT

    // generate a single random value, which happens to already be the sum of random bits *2^j
    // [shares (0,1)][party (0,1,2)][new_size (2*size)]
    Lint ***r_values = new Lint **[2];
    for (i = 0; i < 2; i++) {
        r_values[i] = new Lint *[numParties];
        for (j = 0; j < numParties; j++) {
            r_values[i][j] = new Lint[new_size];
            memset(r_values[i][j], 0, sizeof(Lint) * new_size);
        }
    }

    int gamma[2];
    switch (pid) {
    case 1:
        gamma[0] = 1;
        gamma[1] = 2;
        break;
    case 2:
        gamma[0] = 2;
        gamma[1] = 0;
        break;
    case 3:
        gamma[0] = 0;
        gamma[1] = 1;
        break;
    }

    Lint *r_bits = new Lint[size];
    memset(r_bits, 0, sizeof(Lint) * size);

    uint8_t *buffer = new uint8_t[bytes * new_size];
    // memset(buffer, 0, sizeof(uint8_t) * bytes * new_size);

    // each party generating a unique random value
    // printf("prg1\n");
    nodeNet->prg_getrandom(bytes, size, buffer);
    // printf("mc1\n");
    memcpy(r_bits, buffer, size * bytes);

    // printf("prg2\n");
    // SEGFAULT HERE FOR VERY LARGE VALUES
    nodeNet->prg_getrandom(1, bytes, new_size, buffer);

    // store arithmetic and bitwise representation sequentially
    // calculating p_i's own individual shares

    memcpy(r_values[1][p_index], buffer, size * bytes);
    memcpy(r_values[1][p_index] + size, buffer + size * bytes, size * bytes);

    for (i = 0; i < size; i++) {

        r_values[0][p_index][1 * i] = r_bits[i] - r_values[1][p_index][1 * i];
        r_values[0][p_index][size + i] = r_bits[i] ^ r_values[1][p_index][size + i];


        // r_values[0][p_index][1 * i] = Lint(5) - r_values[1][p_index][1 * i];
        // r_values[0][p_index][size + i] = Lint(5) ^ r_values[1][p_index][size + i];



    }

    // need to generate more random shares so that binary and arithmetic representations are different
    nodeNet->prg_getrandom(0, bytes, new_size, buffer);
    memcpy(r_values[0][gamma[1]], buffer, size * bytes);
    memcpy(r_values[0][gamma[1]] + size, buffer + size * bytes, size * bytes);

    //  sending r_values[0][p_index], receiving r_values[1][gamma[0]],
    nodeNet->SendAndGetDataFromPeer(r_values[0][p_index], r_values[1][gamma[0]], new_size, ring_size);

    for (i = 0; i < numParties - 1; i++) {
        // for (i = 0; i < numParties; i++) {
        memcpy(res_bitwise[0] + i * size, r_values[0][i] + (size), size * sizeof(Lint));
        memcpy(res_bitwise[1] + i * size, r_values[1][i] + (size), size * sizeof(Lint));
    }

    for (i = 0; i < size; i++) {
        // this is so we only have two parties generating shares
        for (j = 0; j < numParties - 1; j++) {
            // adding all the parties arithmetic shares together
            res[0][i] += r_values[0][j][1 * i];
            res[1][i] += r_values[1][j][1 * i];
        }
    }

    for (i = 0; i < 2; i++) {
        for (j = 0; j < numParties; j++) {
            delete[] r_values[i][j];
        }
        delete[] r_values[i];
    }
    delete[] r_values;
    delete[] buffer;
    delete[] r_bits;
}

void Rss_edaBit(Lint **r, Lint **b_2, uint size, uint ring_size, uint bit_length, NodeNetwork *nodeNet) {

    // int pid = nodeNet->getID();
    uint numParties = nodeNet->getNumParties();
    // printf("numParties : %u\n",numParties);

    uint i;
    // need to multiply by the number of parties in the computation
    uint new_size = numParties * size;

    Lint **r_bitwise = new Lint *[2];
    for (i = 0; i < 2; i++) {
        r_bitwise[i] = new Lint[new_size];
        memset(r_bitwise[i], 0, sizeof(Lint) * new_size);

        // ensuring destinations are sanitized
        memset(r[i], 0, sizeof(Lint) * size);
        memset(b_2[i], 0, sizeof(Lint) * size);
    }

    Rss_GenerateRandomShares(r, r_bitwise, ring_size, bit_length, size, nodeNet);

    Rss_nBitAdd(b_2, r_bitwise, ring_size, size, nodeNet);

    for (i = 0; i < 2; i++) {
        delete[] r_bitwise[i];
    }
    delete[] r_bitwise;
}

void Rss_GenerateRandomShares(Lint **res, Lint **res_bitwise, uint ring_size, uint bit_length, uint size, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint i, j;
    uint bytes = (ring_size + 7) >> 3;
    // printf("bytes : %u \n", bytes);
    uint p_index = pid - 1;
    uint numParties = nodeNet->getNumParties();
    // Lint temp0, temp1;

    // used since we have effectively double the number of values
    // since we need to represent both arithmetic and binary shares
    uint new_size = 2 * size;

    // generate a single random value, which happens to already be the sum of random bits *2^j
    // [shares (0,1)][party (0,1,2)][new_size (2*size)]
    Lint ***r_values = new Lint **[2];
    for (i = 0; i < 2; i++) {
        r_values[i] = new Lint *[numParties];
        for (j = 0; j < numParties; j++) {
            r_values[i][j] = new Lint[new_size];
            memset(r_values[i][j], 0, sizeof(Lint) * new_size);
        }
    }

    int gamma[2];
    switch (pid) {
    case 1:
        gamma[0] = 1;
        gamma[1] = 2;
        break;
    case 2:
        gamma[0] = 2;
        gamma[1] = 0;
        break;
    case 3:
        gamma[0] = 0;
        gamma[1] = 1;
        break;
    }

    Lint *r_bits = new Lint[size];
    memset(r_bits, 0, sizeof(Lint) * size);

    uint8_t *buffer = new uint8_t[bytes * new_size];
    // each party generating a unique random value
    nodeNet->prg_getrandom(bytes, size, buffer);
    for (i = 0; i < size; i++) {
        memcpy(r_bits + i, buffer + i * bytes, bytes);
        // is this what we need to do to ensure we have a shorter value?
        // or do we need to do something at the end of the computation

        // r_bits[i] = -1; // USED FOR TESTING
        r_bits[i] = r_bits[i] & nodeNet->SHIFT[bit_length];
    }

    nodeNet->prg_getrandom(1, bytes, new_size, buffer);

    // store arithmetic and bitwise representation sequentially
    // calculating p_i's own individual shares
    for (i = 0; i < size; i++) {
        memcpy(r_values[1][p_index] + 2 * i, buffer + (2 * i) * bytes, bytes);
        memcpy(r_values[1][p_index] + 2 * i + 1, buffer + (2 * i + 1) * bytes, bytes);
        // r_values[0][p_index][2 * i] = r_bits[i] ;
        r_values[0][p_index][2 * i] = r_bits[i] - r_values[1][p_index][2 * i];
        r_values[0][p_index][2 * i + 1] = r_bits[i] ^ r_values[1][p_index][2 * i + 1];

    }

    // need to generate more random shares so that binary and arithmetic representations are different
    nodeNet->prg_getrandom(0, bytes, new_size, buffer);
    for (i = 0; i < size; i++) {
        memcpy(r_values[0][gamma[1]] + (2 * i), buffer + (2 * i) * bytes, bytes);
        memcpy(r_values[0][gamma[1]] + (2 * i + 1), buffer + (2 * i + 1) * bytes, bytes);
    }

    //  sending r_values[0][p_index], receiving r_values[1][gamma[0]],
    nodeNet->SendAndGetDataFromPeer(r_values[0][p_index], r_values[1][gamma[0]], new_size, ring_size);

    for (i = 0; i < size; i++) {
        for (j = 0; j < numParties; j++) {
            // adding all the parties arithmetic shares together
            // memcpy(res[0] + (3 * i + j), r_values[0][j] + (2 * i), sizeof(Lint));
            // memcpy(res[1] + (3 * i + j), r_values[1][j] + (2 * i), sizeof(Lint));
            res[0][i] += r_values[0][j][2 * i];
            res[1][i] += r_values[1][j][2 * i];

            memcpy(res_bitwise[0] + (numParties * i + j), r_values[0][j] + (2 * i + 1), sizeof(Lint));
            memcpy(res_bitwise[1] + (numParties * i + j), r_values[1][j] + (2 * i + 1), sizeof(Lint));
        }
    }

    for (i = 0; i < 2; i++) {
        for (j = 0; j < numParties; j++) {
            delete[] r_values[i][j];
        }
        delete[] r_values[i];
    }
    delete[] r_values;
    delete[] buffer;
    delete[] r_bits;
}

// void Rss_edaBit_trunc(Lint **r, Lint **r_prime, Lint **b_2, uint size, uint ring_size, uint m, NodeNetwork *nodeNet) {
void Rss_edaBit_trunc(Lint **r, Lint **r_prime, Lint **r_km1, uint size, uint ring_size, uint m, NodeNetwork *nodeNet) {

    // int pid = nodeNet->getID();
    uint numParties = nodeNet->getNumParties();

    uint i;

    Lint **r_bitwise = new Lint *[2];
    Lint **carry = new Lint *[2];
    Lint **b_2 = new Lint *[2];
    uint new_size = numParties * size;
    uint b2a_size = 3 * size;

    for (i = 0; i < 2; i++) {
        r_bitwise[i] = new Lint[new_size];
        memset(r_bitwise[i], 0, sizeof(Lint) * new_size);
        // carry will hold both kth and m-1th bits, in succession
        carry[i] = new Lint[b2a_size];
        memset(carry[i], 0, sizeof(Lint) * b2a_size);

        b_2[i] = new Lint[size];
        memset(b_2[i], 0, sizeof(Lint) * size);

        // ensuring destinations are sanitized
        memset(r[i], 0, sizeof(Lint) * size);
        memset(r_prime[i], 0, sizeof(Lint) * size);
        memset(r_km1[i], 0, sizeof(Lint) * size);
    }

    Rss_GenerateRandomShares_trunc(r, r_prime, r_bitwise, ring_size, m, size, nodeNet);

    Rss_nBitAdd_trunc(b_2, carry, r_bitwise, ring_size, m, size, nodeNet);

    Rss_b2a(carry, carry, ring_size, b2a_size, nodeNet);

    memcpy(r_km1[0], carry[0] + 2 * (size), size * sizeof(Lint));
    memcpy(r_km1[1], carry[1] + 2 * (size), size * sizeof(Lint));

    // adding m-1 and subtracting k carries
    for (size_t i = 0; i < size; i++) {

        r_prime[0][i] = r_prime[0][i] + carry[0][i] - ((carry[0][size + i]) << Lint(ring_size - m));
        r_prime[1][i] = r_prime[1][i] + carry[1][i] - ((carry[1][size + i]) << Lint(ring_size - m));
    }

    for (i = 0; i < 2; i++) {
        delete[] r_bitwise[i];
        delete[] carry[i];
        delete[] b_2[i];
    }
    delete[] r_bitwise;
    delete[] carry;
    delete[] b_2;
}

void Rss_edaBit_trunc_test(Lint **r, Lint **r_prime, Lint **r_km1, uint size, uint ring_size, uint m, NodeNetwork *nodeNet) {

    // int pid = nodeNet->getID();
    uint numParties = nodeNet->getNumParties();

    uint i;

    Lint **r_bitwise = new Lint *[2];
    Lint **carry = new Lint *[2];
    Lint **b_2 = new Lint *[2];
    uint new_size = numParties * size;
    uint b2a_size = 3 * size;

    for (i = 0; i < 2; i++) {
        r_bitwise[i] = new Lint[new_size];
        memset(r_bitwise[i], 0, sizeof(Lint) * new_size);
        // carry will hold both kth and m-1th bits, in succession
        carry[i] = new Lint[b2a_size];
        memset(carry[i], 0, sizeof(Lint) * b2a_size);

        b_2[i] = new Lint[size];
        memset(b_2[i], 0, sizeof(Lint) * size);

        // ensuring destinations are sanitized
        memset(r[i], 0, sizeof(Lint) * size);
        memset(r_prime[i], 0, sizeof(Lint) * size);
        memset(r_km1[i], 0, sizeof(Lint) * size);
    }

    Rss_GenerateRandomShares_trunc(r, r_prime, r_bitwise, ring_size, m, size, nodeNet);

    Rss_nBitAdd_trunc(b_2, carry, r_bitwise, ring_size, m, size, nodeNet);

    Rss_b2a(carry, carry, ring_size, b2a_size, nodeNet);

    memcpy(r_km1[0], carry[0] + 2 * (size), size * sizeof(Lint));
    memcpy(r_km1[1], carry[1] + 2 * (size), size * sizeof(Lint));

    // adding m-1 and subtracting k carries
    for (size_t i = 0; i < size; i++) {

        // r_prime[0][i] = r_prime[0][i] + carry[0][i] - ((carry[0][size + i]) << Lint(ring_size - m));
        // r_prime[1][i] = r_prime[1][i] + carry[1][i] - ((carry[1][size + i]) << Lint(ring_size - m));

        r_prime[0][i] = r_prime[0][i] + carry[0][i];
        r_prime[1][i] = r_prime[1][i] + carry[1][i];
    }

    for (i = 0; i < 2; i++) {
        delete[] r_bitwise[i];
        delete[] carry[i];
        delete[] b_2[i];
    }
    delete[] r_bitwise;
    delete[] carry;
    delete[] b_2;
}

void Rss_GenerateRandomShares_trunc(Lint **res, Lint **res_prime, Lint **res_bitwise, uint ring_size, uint m, uint size, NodeNetwork *nodeNet) {

    int pid = nodeNet->getID();
    uint i, j;
    uint bytes = (ring_size + 7) >> 3;
    // printf("bytes : %u \n", bytes);
    uint p_index = pid - 1;
    uint numParties = nodeNet->getNumParties();
    // printf("numParties : %u \n", numParties);

    // Lint temp0, temp1;

    // used since we have effectively double the number of values
    // since we need to represent both arithmetic and binary shares
    uint new_size = 3 * size;

    // generate a single random value, which happens to already be the sum of random bits *2^j
    // [shares (0,1)][party (0,1,2)][new_size (2*size)]
    Lint ***r_values = new Lint **[2];
    for (i = 0; i < 2; i++) {
        r_values[i] = new Lint *[numParties];
        for (j = 0; j < numParties; j++) {
            r_values[i][j] = new Lint[new_size];
            memset(r_values[i][j], 0, sizeof(Lint) * new_size);
        }
    }

    int gamma[2];
    switch (pid) {
    case 1:
        gamma[0] = 1;
        gamma[1] = 2;
        break;
    case 2:
        gamma[0] = 2;
        gamma[1] = 0;
        break;
    case 3:
        gamma[0] = 0;
        gamma[1] = 1;
        break;
    }

    Lint *r_bits = new Lint[size];
    memset(r_bits, 0, sizeof(Lint) * size);
    Lint *r_prime = new Lint[size];
    memset(r_prime, 0, sizeof(Lint) * size);

    uint8_t *buffer = new uint8_t[bytes * new_size];
    // each party generating a unique random value
    nodeNet->prg_getrandom(bytes, size, buffer);

    memcpy(r_bits, buffer, size * bytes);

    for (i = 0; i < size; i++) {
        r_bits[i] = r_bits[i] & nodeNet->SHIFT[ring_size];
        r_prime[i] = (r_bits[i] >> Lint(m));
    }

    nodeNet->prg_getrandom(1, bytes, new_size, buffer);

    // store arithmetic and bitwise representation sequentially
    // calculating p_i's own individual shares

    memcpy(r_values[1][p_index], buffer, size * bytes);
    memcpy(r_values[1][p_index] + size, buffer + size * bytes, size * bytes);
    memcpy(r_values[1][p_index] + 2 * size, buffer + 2 * size * bytes, size * bytes);

    for (i = 0; i < size; i++) {
        r_values[0][p_index][1 * i] = r_bits[i] - r_values[1][p_index][1 * i];
        r_values[0][p_index][size + i] = r_bits[i] ^ r_values[1][p_index][size + i];
        r_values[0][p_index][2 * size + i] = r_prime[i] - r_values[1][p_index][2 * size + i];
        // r_values[0][p_index][2*size + i] = r_bits[i] - r_values[1][p_index][2*size + i];
    }

    // need to generate more random shares so that binary and arithmetic representations are different
    nodeNet->prg_getrandom(0, bytes, new_size, buffer);
    memcpy(r_values[0][gamma[1]], buffer, size * bytes);
    memcpy(r_values[0][gamma[1]] + size, buffer + size * bytes, size * bytes);
    memcpy(r_values[0][gamma[1]] + 2 * size, buffer + 2 * size * bytes, size * bytes);

    //  sending r_values[0][p_index], receiving r_values[1][gamma[0]],
    nodeNet->SendAndGetDataFromPeer(r_values[0][p_index], r_values[1][gamma[0]], new_size, ring_size);

    for (i = 0; i < numParties - 1; i++) {
        memcpy(res_bitwise[0] + i * size, r_values[0][i] + (size), size * sizeof(Lint));
        memcpy(res_bitwise[1] + i * size, r_values[1][i] + (size), size * sizeof(Lint));
    }

    for (i = 0; i < size; i++) {
        // this is so we only have two parties generating shares
        for (j = 0; j < numParties - 1; j++) {

            // adding all the parties arithmetic shares together
            res[0][i] += r_values[0][j][1 * i];
            res[1][i] += r_values[1][j][1 * i];

            res_prime[0][i] += r_values[0][j][2 * size + i];
            res_prime[1][i] += r_values[1][j][2 * size + i];
        }
    }

    for (i = 0; i < 2; i++) {
        for (j = 0; j < numParties; j++) {
            delete[] r_values[i][j];
        }
        delete[] r_values[i];
    }
    delete[] r_values;
    delete[] buffer;
    delete[] r_bits;
    delete[] r_prime;
}


void Rss_edaBit_5pc(Lint **r, Lint **b_2, uint size, uint ring_size, NodeNetwork *nodeNet) {

    // int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();
    uint threshold = nodeNet->getThreshold();
    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;

    uint i;
    // this is the number of shares we need to add (t+1)
    uint new_size = (threshold + 1) * size;
    // printf("new_size: %llu\n", new_size);

    Lint **r_bitwise = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        r_bitwise[i] = new Lint[new_size];
        memset(r_bitwise[i], 0, sizeof(Lint) * new_size);

        // ensuring destinations are sanitized
        memset(r[i], 0, sizeof(Lint) * size);
        memset(b_2[i], 0, sizeof(Lint) * size);
    }

    Rss_GenerateRandomShares_5pc(r, r_bitwise, ring_size, size, nodeNet);

    Rss_nBitAdd_5pc(b_2, r_bitwise, ring_size, size, nodeNet);

    for (i = 0; i < numShares; i++) {
        delete[] r_bitwise[i];
    }
    delete[] r_bitwise;
}

void Rss_GenerateRandomShares_5pc(Lint **res, Lint **res_bitwise, uint ring_size, uint size, NodeNetwork *nodeNet) {
    // printf("start\n");
    int pid = nodeNet->getID();
    uint i, j;
    uint bytes = (ring_size + 7) >> 3;
    uint new_size = 2 * size; // DO NOT CHANGE, IT IS IRRELEVANT for n>3
    // printf("bytes : %u \n", bytes);
    uint p_index = pid - 1;
    uint numParties = nodeNet->getNumParties();
    uint numShares = nodeNet->getNumShares();
    uint threshold = nodeNet->getThreshold();
    // printf("threshold : %u \n", threshold);
    // printf("numParties : %u \n", numParties);

    Lint **recvbuf = new Lint *[threshold];
    for (i = 0; i < threshold; i++) {
        recvbuf[i] = new Lint[new_size];
        memset(recvbuf[i], 0, sizeof(Lint) * new_size);
    }

    // used since we have effectively double the number of values
    // since we need to represent both arithmetic and binary shares
    // printf("new_size : %u \n", new_size);

    // generate a single random value, which happens to already be the sum of random bits *2^j
    // [shares (0,1)][party (0,1,2)][new_size (2*size)]
    Lint ***r_values = new Lint **[numShares];
    for (i = 0; i < numShares; i++) {
        r_values[i] = new Lint *[(threshold + 1)];
        for (j = 0; j < (threshold + 1); j++) {
            r_values[i][j] = new Lint[new_size];
            memset(r_values[i][j], 0, sizeof(Lint) * new_size); // NECESSARY FOR n>3
        }
    }

    Lint *r_bits = new Lint[size];
    memset(r_bits, 0, sizeof(Lint) * size);

    uint8_t *r_buffer = new uint8_t[bytes * size];

    uint8_t **buffer = new uint8_t *[numShares];
    for (uint s = 0; s < numShares; s++) {
        buffer[s] = new uint8_t[(threshold + 1) * bytes * new_size]; // we may not use all of these random bytes, but
        nodeNet->prg_getrandom(s, bytes, (threshold + 1) * new_size, buffer[s]);
    }

    int *index_map = new int[threshold+1];

    uint reset_value;
    switch (pid) {
    case 1:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = 3;
        index_map[2] = 5;
        break;
    case 2:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = -1;
        index_map[2] = 3;
        break;
    case 3:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = -1;
        index_map[2] = -1;
        break;
    case 4:
        reset_value = 1;
        index_map[0] = 5;
        index_map[1] = -1;
        index_map[2] = -1;
        break;
    case 5:
        reset_value = 0;
        index_map[0] = 3;
        index_map[1] = 5;
        index_map[2] = -1;
        break;
    }

    bool prg_bools[4][6] = {
        {1, 1, 0, 1, 0, 0},
        {1, 0, 1, 0, 1, 0},
        {0, 1, 1, 0, 0, 0},
        {0, 0, 0, 0, 1, 1},
    };

    if (pid < threshold + 2) {
        // p1, p2, p3, choosing random values

        nodeNet->prg_getrandom(bytes, size, r_buffer);
        // memcpy(r_bits, r_buffer, size * bytes);

        for (i = 0; i < size; i++) {
            memcpy(r_bits + i, r_buffer + i * bytes, bytes);

            // printf("hi1\n");
            for (size_t s = 1; s < numShares; s++) {
                // memcpy(r_values[s][p_index], buffer[s], new_size * bytes);
                memcpy(r_values[s][p_index] + 2 * i, buffer[s] + (2 * i) * bytes, bytes);
                memcpy(r_values[s][p_index] + 2 * i + 1, buffer[s] + (2 * i + 1) * bytes, bytes);
            }

            // r_values[0][p_index][2 * i] = Lint(5 + i);
            // r_values[0][p_index][2 * i + 1] = Lint(5 + i);
            r_values[0][p_index][2 * i] = r_bits[i];
            r_values[0][p_index][2 * i + 1] = r_bits[i];

            for (size_t s = 1; s < numShares; s++) {
                r_values[0][p_index][2 * i] -= r_values[s][p_index][2 * i];
                r_values[0][p_index][2 * i + 1] ^= r_values[s][p_index][2 * i + 1];
            }
        }
    }

    // num parties - 1

    // printf("indices\n");
    // int n = numParties - 1;
    // printf("n: %llu\n", n);

    for (size_t i = 0; i < size; i++) {

        int index = ((pid - 2) % (numParties - 1) + (numParties - 1)) % (numParties - 1);
        for (size_t t = 0; t < threshold + 1; t++) {
            if (p_index != t) {
                // int index = (((pid - 2 - t) % ((numParties - 1))) + ((numParties - 1)) )  % ((numParties - 1) );

                // printf("%i : bool_index = %i\n", t, index);
                // loop through num_shares
                // if we're supposed to generate, then memcpy from buffer[j]
                for (size_t s = 0; s < numShares; s++) {
                    if (prg_bools[index][s]) {
                        // printf("copying: %llu\n", copying);
                        memcpy(r_values[s][t] + (2 * i), buffer[s] + (2 * i) * bytes, bytes);
                        memcpy(r_values[s][t] + (2 * i + 1), buffer[s] + (2 * i + 1) * bytes, bytes);
                    }
                }
                // printf("subtracting\(numParties - 1)");
                index = ((index - 1) % (numParties - 1) + (numParties - 1)) % (numParties - 1);
            }
        }
    }

    nodeNet->SendAndGetDataFromPeer_Eda(r_values[0][p_index], recvbuf, new_size, ring_size);

    // extracting from buffer
    for (size_t i = 0; i < size; i++) {
        j = reset_value;
        for (size_t t = 0; t < threshold + 1; t++) {

            if (index_map[t] > 0) {
                // printf("%u : putting %llu from recvbuff[%i] in %u\n", t, recvbuf[j][2 * i], j, index_map[t]);
                // printf("%u : putting %llu from recvbuff[%i] in %u\n", t, recvbuf[j][2 * i + 1], j, index_map[t]);
                memcpy(r_values[index_map[t]][t] + 2 * i, recvbuf[j] + 2 * i, sizeof(Lint));
                memcpy(r_values[index_map[t]][t] + 2 * i + 1, recvbuf[j] + 2 * i + 1, sizeof(Lint));
                j++;
            }
        }
    }

    for (size_t i = 0; i < size; i++) {
        for (uint p = 0; p < threshold + 1; p++) {
            for (size_t s = 0; s < numShares; s++) {
                res_bitwise[s][p * (size) + i] = r_values[s][p][2 * i + 1];
            }
        }
    }

    for (i = 0; i < size; i++) {
        // this is so we only have t+1 parties generating shares
        for (j = 0; j < threshold + 1; j++) {
            // adding all the parties arithmetic shares together
            for (size_t s = 0; s < numShares; s++) {
                res[s][i] += r_values[s][j][2 * i];
                // res[1][i] += r_values[1][j][1 * i];
            }
        }
    }

    delete[] index_map;
    for (i = 0; i < threshold; i++) {
        delete[] recvbuf[i];
    }

    delete[] recvbuf;

    for (i = 0; i < numShares; i++) {
        for (j = 0; j < (threshold + 1); j++) {
            delete[] r_values[i][j];
        }
        delete[] r_values[i];
    }
    delete[] r_values;
    for (i = 0; i < numShares; i++) {
        delete[] buffer[i];
    }
    delete[] buffer;
    delete[] r_buffer;
    delete[] r_bits;
}

void Rss_edaBit_trunc_5pc(Lint **r, Lint **r_prime, Lint **r_km1, uint size, uint ring_size, uint m, NodeNetwork *nodeNet) {

    // int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();
    uint threshold = nodeNet->getThreshold();
    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;

    uint i;
    // this is the number of shares we need to add (t+1)
    uint new_size = (threshold + 1) * size;
    // printf("new_size: %llu\n", new_size);

    uint b2a_size = 3 * size;

    Lint **carry = new Lint *[numShares];
    Lint **b_2 = new Lint *[numShares];
    Lint **r_bitwise = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        r_bitwise[i] = new Lint[new_size];
        memset(r_bitwise[i], 0, sizeof(Lint) * new_size);
        // carry will hold both kth and m-1th bits, in succession
        carry[i] = new Lint[b2a_size];
        memset(carry[i], 0, sizeof(Lint) * b2a_size);

        b_2[i] = new Lint[size];
        memset(b_2[i], 0, sizeof(Lint) * size);

        // ensuring destinations are sanitized
        memset(r[i], 0, sizeof(Lint) * size);
        memset(r_prime[i], 0, sizeof(Lint) * size);
        memset(r_km1[i], 0, sizeof(Lint) * size);
    }

    Rss_GenerateRandomShares_trunc_5pc(r, r_prime, r_bitwise, ring_size, m, size, nodeNet);

    Rss_nBitAdd_trunc_5pc(b_2, carry, r_bitwise, ring_size, m, size, nodeNet);

    Rss_b2a_5pc(carry, carry, ring_size, b2a_size, nodeNet);
    for (size_t s = 0; s < numShares; s++) {
        memcpy(r_km1[s], carry[s] + 2 * (size), size * sizeof(Lint));
    }

    // adding m-1 and subtracting k carries
    for (size_t i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {

            r_prime[s][i] = r_prime[s][i] + carry[s][i] - ((carry[s][size + i]) << Lint(ring_size - m));
        }
    }

    for (i = 0; i < numShares; i++) {
        delete[] r_bitwise[i];
        delete[] carry[i];
        delete[] b_2[i];
    }
    delete[] r_bitwise;
    delete[] carry;
    delete[] b_2;
}

void Rss_GenerateRandomShares_trunc_5pc(Lint **res, Lint **res_prime, Lint **res_bitwise, uint ring_size, uint m, uint size, NodeNetwork *nodeNet) {
    // printf("start\n");
    int pid = nodeNet->getID();
    uint i, j;
    uint bytes = (ring_size + 7) >> 3;
    uint new_size = 3 * size; // DO NOT CHANGE, IT IS IRRELEVANT for n>3
    // printf("bytes : %u \n", bytes);
    uint p_index = pid - 1;
    uint numParties = nodeNet->getNumParties();
    uint numShares = nodeNet->getNumShares();
    uint threshold = nodeNet->getThreshold();
    // printf("threshold : %u \n", threshold);
    // printf("numParties : %u \n", numParties);

    int *index_map = new int[3];

    uint reset_value;
    switch (pid) {
    case 1:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = 3;
        index_map[2] = 5;
        break;
    case 2:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = -1;
        index_map[2] = 3;
        break;
    case 3:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = -1;
        index_map[2] = -1;
        break;
    case 4:
        reset_value = 1;
        index_map[0] = 5;
        index_map[1] = -1;
        index_map[2] = -1;
        break;
    case 5:
        reset_value = 0;
        index_map[0] = 3;
        index_map[1] = 5;
        index_map[2] = -1;
        break;
    }

    bool prg_bools[4][6] = {
        {1, 1, 0, 1, 0, 0},
        {1, 0, 1, 0, 1, 0},
        {0, 1, 1, 0, 0, 0},
        {0, 0, 0, 0, 1, 1},
    };

    Lint **recvbuf = new Lint *[threshold];
    for (i = 0; i < threshold; i++) {
        recvbuf[i] = new Lint[new_size];
        memset(recvbuf[i], 0, sizeof(Lint) * new_size);
    }

    // used since we have effectively double the number of values
    // since we need to represent both arithmetic and binary shares
    // printf("new_size : %u \n", new_size);

    // generate a single random value, which happens to already be the sum of random bits *2^j
    // [shares (0,1)][party (0,1,2)][new_size (2*size)]
    Lint ***r_values = new Lint **[numShares];
    for (i = 0; i < numShares; i++) {
        r_values[i] = new Lint *[(threshold + 1)];
        for (j = 0; j < (threshold + 1); j++) {
            r_values[i][j] = new Lint[new_size];
            memset(r_values[i][j], 0, sizeof(Lint) * new_size); // NECESSARY FOR n>3
        }
    }

    Lint *r_bits = new Lint[size];
    memset(r_bits, 0, sizeof(Lint) * size);
    Lint *r_prime = new Lint[size];
    memset(r_prime, 0, sizeof(Lint) * size);

    uint8_t *r_buffer = new uint8_t[bytes * size];

    uint8_t **buffer = new uint8_t *[numShares];
    for (uint s = 0; s < numShares; s++) {
        buffer[s] = new uint8_t[(threshold + 1) * bytes * new_size]; // we may not use all of these random bytes, but
        nodeNet->prg_getrandom(s, bytes, (threshold + 1) * new_size, buffer[s]);
    }

    if (pid < 4) {
        // p1, p2, p3, choosing random values

        nodeNet->prg_getrandom(bytes, size, r_buffer);
        // memcpy(r_bits, r_buffer, size * bytes);

        for (i = 0; i < size; i++) {
            memcpy(r_bits + i, r_buffer + i * bytes, bytes);

            r_bits[i] = r_bits[i] & nodeNet->SHIFT[ring_size];
            r_prime[i] = (r_bits[i] >> Lint(m));

            // printf("hi1\n");
            for (size_t s = 1; s < numShares; s++) {
                // memcpy(r_values[s][p_index], buffer[s], new_size * bytes);
                memcpy(r_values[s][p_index] + 2 * i, buffer[s] + (2 * i) * bytes, bytes);
                memcpy(r_values[s][p_index] + 2 * i + 1, buffer[s] + (2 * i + 1) * bytes, bytes);
                memcpy(r_values[s][p_index] + 2 * i + 2, buffer[s] + (2 * i + 2) * bytes, bytes);
            }

            // r_values[0][p_index][2 * i] = Lint(5 + i);
            // r_values[0][p_index][2 * i + 1] = Lint(5 + i);
            r_values[0][p_index][2 * i] = r_bits[i];
            r_values[0][p_index][2 * i + 1] = r_bits[i];
            r_values[0][p_index][2 * i + 2] = r_bits[i];

            for (size_t s = 1; s < numShares; s++) {
                r_values[0][p_index][2 * i] -= r_values[s][p_index][2 * i];
                r_values[0][p_index][2 * i + 1] ^= r_values[s][p_index][2 * i + 1];
                r_values[0][p_index][2 * i + 2] -= r_values[s][p_index][2 * i + 2];
            }
        }
    }



    for (size_t i = 0; i < size; i++) {

        int index = ((pid - 2) % (numParties - 1) + (numParties - 1)) % (numParties - 1);
        for (size_t t = 0; t < threshold + 1; t++) {
            if (p_index != t) {
                // int index = (((pid - 2 - t) % ((numParties - 1))) + ((numParties - 1)) )  % ((numParties - 1) );

                // printf("%i : bool_index = %i\n", t, index);
                // loop through num_shares
                // if we're supposed to generate, then memcpy from buffer[j]
                for (size_t s = 0; s < numShares; s++) {
                    if (prg_bools[index][s]) {
                        // printf("copying: %llu\n", copying);
                        memcpy(r_values[s][t] + (2 * i), buffer[s] + (2 * i) * bytes, bytes);
                        memcpy(r_values[s][t] + (2 * i + 1), buffer[s] + (2 * i + 1) * bytes, bytes);
                        memcpy(r_values[s][t] + (2 * i + 2), buffer[s] + (2 * i + 2) * bytes, bytes);
                    }
                }
                // printf("subtracting\(numParties - 1)");
                index = ((index - 1) % (numParties - 1) + (numParties - 1)) % (numParties - 1);
            }
        }
    }

    nodeNet->SendAndGetDataFromPeer_Eda(r_values[0][p_index], recvbuf, new_size, ring_size);

    // extracting from buffer
    for (size_t i = 0; i < size; i++) {
        j = reset_value;
        for (size_t t = 0; t < threshold + 1; t++) {

            if (index_map[t] > 0) {
                // printf("%u : putting %llu from recvbuff[%i] in %u\n", t, recvbuf[j][2 * i], j, index_map[t]);
                // printf("%u : putting %llu from recvbuff[%i] in %u\n", t, recvbuf[j][2 * i + 1], j, index_map[t]);
                memcpy(r_values[index_map[t]][t] + 2 * i, recvbuf[j] + 2 * i, sizeof(Lint));
                memcpy(r_values[index_map[t]][t] + 2 * i + 1, recvbuf[j] + 2 * i + 1, sizeof(Lint));
                memcpy(r_values[index_map[t]][t] + 2 * i + 2, recvbuf[j] + 2 * i + 2, sizeof(Lint));
                j++;
            }
        }
    }

    for (size_t i = 0; i < size; i++) {
        for (uint p = 0; p < threshold + 1; p++) {
            for (size_t s = 0; s < numShares; s++) {
                res_bitwise[s][p * (size) + i] = r_values[s][p][2 * i + 1];
            }
        }
    }

    for (i = 0; i < size; i++) {
        // this is so we only have t+1 parties generating shares
        for (j = 0; j < threshold + 1; j++) {
            // adding all the parties arithmetic shares together
            for (size_t s = 0; s < numShares; s++) {
                res[s][i] += r_values[s][j][2 * i];

                res_prime[s][i] += r_values[s][j][2 * i + 2];
                // res[1][i] += r_values[1][j][1 * i];
            }
        }
    }

    for (i = 0; i < threshold; i++) {
        delete[] recvbuf[i];
    }

    delete[] recvbuf;
    delete[] index_map;

    for (i = 0; i < numShares; i++) {
        for (j = 0; j < (threshold + 1); j++) {
            delete[] r_values[i][j];
        }
        delete[] r_values[i];
    }
    for (i = 0; i < numShares; i++) {
        delete[] buffer[i];
    }
    delete[] r_values;
    delete[] buffer;
    delete[] r_buffer;
    delete[] r_prime;
    delete[] r_bits;
}


void Rss_edaBit_7pc(Lint **r, Lint **b_2, uint size, uint ring_size, NodeNetwork *nodeNet) {

    // int pid = nodeNet->getID();
    uint numShares = nodeNet->getNumShares();
    uint threshold = nodeNet->getThreshold();
    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;

    uint i;
    // this is the number of shares we need to add (t+1)
    uint new_size = (threshold + 1) * size;
    // printf("new_size: %llu\n", new_size);

    Lint **r_bitwise = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        r_bitwise[i] = new Lint[new_size];
        memset(r_bitwise[i], 0, sizeof(Lint) * new_size);

        // ensuring destinations are sanitized
        memset(r[i], 0, sizeof(Lint) * size);
        memset(b_2[i], 0, sizeof(Lint) * size);
    }

    Rss_GenerateRandomShares_7pc(r, r_bitwise, ring_size, size, nodeNet);

    Rss_nBitAdd_7pc(b_2, r_bitwise, ring_size, size, nodeNet);

    for (i = 0; i < numShares; i++) {
        delete[] r_bitwise[i];
    }
    delete[] r_bitwise;
}

void Rss_GenerateRandomShares_7pc(Lint **res, Lint **res_bitwise, uint ring_size, uint size, NodeNetwork *nodeNet) {
    // printf("start\n");
    int pid = nodeNet->getID();
    uint i, j;
    uint bytes = (ring_size + 7) >> 3;
    uint new_size = 2 * size; // DO NOT CHANGE, IT IS IRRELEVANT for n>3
    // printf("bytes : %u \n", bytes);
    uint p_index = pid - 1;
    uint numParties = nodeNet->getNumParties();
    uint numShares = nodeNet->getNumShares();
    uint threshold = nodeNet->getThreshold();
    // printf("threshold : %u \n", threshold);
    // printf("numParties : %u \n", numParties);

    Lint **recvbuf = new Lint *[threshold];
    for (i = 0; i < threshold; i++) {
        recvbuf[i] = new Lint[new_size];
        memset(recvbuf[i], 0, sizeof(Lint) * new_size);
    }

    // used since we have effectively double the number of values
    // since we need to represent both arithmetic and binary shares
    // printf("new_size : %u \n", new_size);

    // generate a single random value, which happens to already be the sum of random bits *2^j
    // [shares (0,1)][party (0,1,2)][new_size (2*size)]
    Lint ***r_values = new Lint **[numShares];
    for (i = 0; i < numShares; i++) {
        r_values[i] = new Lint *[(threshold + 1)];
        for (j = 0; j < (threshold + 1); j++) {
            r_values[i][j] = new Lint[new_size];
            memset(r_values[i][j], 0, sizeof(Lint) * new_size); // NECESSARY FOR n>3
        }
    }

    Lint *r_bits = new Lint[size];
    memset(r_bits, 0, sizeof(Lint) * size);

    uint8_t *r_buffer = new uint8_t[bytes * size];

    uint8_t **buffer = new uint8_t *[numShares];
    for (uint s = 0; s < numShares; s++) {
        buffer[s] = new uint8_t[(threshold + 1) * bytes * new_size]; // we may not use all of these random bytes, but
        nodeNet->prg_getrandom(s, bytes, (threshold + 1) * new_size, buffer[s]);
    }

    int *index_map = new int[threshold + 1];

    uint reset_value;
    switch (pid) {
    case 1:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = 10;
        index_map[2] = 16;
        index_map[3] = 19;
        break;
    case 2:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = -1;
        index_map[2] = 10;
        index_map[3] = 16;
        break;
    case 3:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = -1;
        index_map[2] = -1;
        index_map[3] = 10;
        break;
    case 4:
        reset_value = 0;
        index_map[0] = -1;
        index_map[1] = -1;
        index_map[2] = -1;
        index_map[3] = -1;
        break;
    case 5:
        reset_value = 2;
        index_map[0] = 19;
        index_map[1] = -1;
        index_map[2] = -1;
        index_map[3] = -1;
        break;
    case 6:
        reset_value = 1;
        index_map[0] = 16;
        index_map[1] = 19;
        index_map[2] = -1;
        index_map[3] = -1;
        break;
    case 7:
        reset_value = 0;
        index_map[0] = 10;
        index_map[1] = 16;
        index_map[2] = 19;
        index_map[3] = -1;
        break;
    }

    bool prg_bools[6][20] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1},
        {0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0},
        {1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0},
        {1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0},
        {1, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 0}};

    if (pid < threshold + 2) {
        // p1, p2, p3, p4, choosing random values

        nodeNet->prg_getrandom(bytes, size, r_buffer);
        // memcpy(r_bits, r_buffer, size * bytes);

        for (i = 0; i < size; i++) {
            memcpy(r_bits + i, r_buffer + i * bytes, bytes);

            // printf("hi1\n");
            for (size_t s = 1; s < numShares; s++) {
                // memcpy(r_values[s][p_index], buffer[s], new_size * bytes);
                memcpy(r_values[s][p_index] + 2 * i, buffer[s] + (2 * i) * bytes, bytes);
                memcpy(r_values[s][p_index] + 2 * i + 1, buffer[s] + (2 * i + 1) * bytes, bytes);
            }

            // r_values[0][p_index][2 * i] = Lint(5 + i);
            // r_values[0][p_index][2 * i + 1] = Lint(5 + i);
            r_values[0][p_index][2 * i] = r_bits[i];
            r_values[0][p_index][2 * i + 1] = r_bits[i];

            for (size_t s = 1; s < numShares; s++) {
                r_values[0][p_index][2 * i] -= r_values[s][p_index][2 * i];
                r_values[0][p_index][2 * i + 1] ^= r_values[s][p_index][2 * i + 1];
            }
        }
    }

    for (size_t i = 0; i < size; i++) {

        int index = ((pid - 2) % (numParties - 1) + (numParties - 1)) % (numParties - 1);
        for (size_t t = 0; t < threshold + 1; t++) {
            if (p_index != t) {
                // int index = (((pid - 2 - t) % ((numParties - 1))) + ((numParties - 1)) )  % ((numParties - 1) );

                // printf("%i : bool_index = %i\n", t, index);
                // loop through num_shares
                // if we're supposed to generate, then memcpy from buffer[j]
                for (size_t s = 0; s < numShares; s++) {
                    if (prg_bools[index][s]) {
                        // printf("copying: %llu\n", copying);
                        memcpy(r_values[s][t] + (2 * i), buffer[s] + (2 * i) * bytes, bytes);
                        memcpy(r_values[s][t] + (2 * i + 1), buffer[s] + (2 * i + 1) * bytes, bytes);
                    }
                }
                // printf("subtracting\(numParties - 1)");
                index = ((index - 1) % (numParties - 1) + (numParties - 1)) % (numParties - 1);
            }
        }
    }

    nodeNet->SendAndGetDataFromPeer_Eda(r_values[0][p_index], recvbuf, new_size, ring_size);

    // extracting from buffer
    for (size_t i = 0; i < size; i++) {
        j = reset_value;
        for (size_t t = 0; t < threshold + 1; t++) {

            if (index_map[t] > 0) {
                // printf("%u : putting %llu from recvbuff[%i] in %u\n", t, recvbuf[j][2 * i], j, index_map[t]);
                // printf("%u : putting %llu from recvbuff[%i] in %u\n", t, recvbuf[j][2 * i + 1], j, index_map[t]);
                memcpy(r_values[index_map[t]][t] + 2 * i, recvbuf[j] + 2 * i, sizeof(Lint));
                memcpy(r_values[index_map[t]][t] + 2 * i + 1, recvbuf[j] + 2 * i + 1, sizeof(Lint));
                j++;
            }
        }
    }

    for (size_t i = 0; i < size; i++) {
        for (uint p = 0; p < threshold + 1; p++) {
            for (size_t s = 0; s < numShares; s++) {
                res_bitwise[s][p * (size) + i] = r_values[s][p][2 * i + 1];
            }
        }
    }

    for (i = 0; i < size; i++) {
        // this is so we only have t+1 parties generating shares
        for (j = 0; j < threshold + 1; j++) {
            // adding all the parties arithmetic shares together
            for (size_t s = 0; s < numShares; s++) {
                res[s][i] += r_values[s][j][2 * i];
                // res[1][i] += r_values[1][j][1 * i];
            }
        }
    }

    delete[] index_map;
    for (i = 0; i < threshold; i++) {
        delete[] recvbuf[i];
    }

    delete[] recvbuf;

    for (i = 0; i < numShares; i++) {
        for (j = 0; j < (threshold + 1); j++) {
            delete[] r_values[i][j];
        }
        delete[] r_values[i];
    }
    delete[] r_values;
    for (i = 0; i < numShares; i++) {
        delete[] buffer[i];
    }
    delete[] buffer;
    delete[] r_buffer;
    delete[] r_bits;
}
