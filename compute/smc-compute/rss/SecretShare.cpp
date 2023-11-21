#ifndef RSS_SECRETSHARE_TPP
#define RSS_SECRETSHARE_TPP

#ifndef RSS_SECRETSHARE_H
#error __FILE__ should only be included from RSS_SecretShare.h.
#endif 


#define KE2(NK, OK, RND)                           \
    NK = OK;                                       \
    NK = _mm_xor_si128(NK, _mm_slli_si128(NK, 4)); \
    NK = _mm_xor_si128(NK, _mm_slli_si128(NK, 4)); \
    NK = _mm_xor_si128(NK, _mm_slli_si128(NK, 4)); \
    NK = _mm_xor_si128(NK, _mm_shuffle_epi32(_mm_aeskeygenassist_si128(OK, RND), 0xff));

template <typename T>
RSS_SecretShare<T>::RSS_SecretShare(NodeNetwork *nodeNet, uint _id, uint n, uint t, uint ring_size) {
    id = _id;
    numParties = n;
    threshold = t;
    numShares = nCk(numParties - 1, threshold);  // shares PER PARTY
    totalNumShares = nCk(numParties, threshold); // total shares
    nNet = nodeNet;                              // just stores a reference to nodeNet, only used for prg_setup


    SHIFT = new T[sizeof(T) * 8];
    ODD = new T[ring_size + 2];
    EVEN = new T[ring_size + 2];

    // i think this is "21" for the minion nn?
    for (T i = 0; i <= sizeof(T) * 8 - 1; i++) {
        SHIFT[i] = (T(1) << T(i)) - T(1); // mod 2^i

        // this is needed to handle "undefined behavior" of << when we want
        // to shift by more than the size of the type (in bits)
        if (i == sizeof(T) * 8) {
            SHIFT[i] = -1;
        }
    }

    T temp = 0;
    for (T i = 0; i <= 8 * sizeof(T); i++) {
        temp = temp | T((i % 2 == 0));
        temp = temp << 1;
    }
    for (T i = 0; i < ring_size + 1; i++) {
        EVEN[i] = (temp >> 1) & SHIFT[i];
        ODD[i] = (temp)&SHIFT[i];
    }

    general_map = new int *[2];
    for (size_t i = 0; i < 2; i++) {
        general_map[i] = new int[threshold];
    }
    if (numParties == 3) {

        prg_setup_3();
        general_map[0][0] = ((id + 2 - 1) % numParties + 1);
        general_map[1][0] = ((id + 1 - 1) % numParties + 1);
    } else {

        open_map_mpc = new int *[2];
        eda_map_mpc = new int *[2];
        T_map_mpc = new int *[numShares];
        for (size_t i = 0; i < 2; i++) {
            open_map_mpc[i] = new int[threshold];
            eda_map_mpc[i] = new int[threshold];
        }
        for (size_t i = 0; i < numShares; i++) {
            T_map_mpc[i] = new int[threshold];
        }

        if (numParties == 5) {

            printf("prg_setup_5\n");
            prg_setup_5();

            open_map_mpc[0][0] = (id + 1 - 1) % numParties + 1;
            open_map_mpc[0][1] = (id + 4 - 1) % numParties + 1;

            open_map_mpc[1][0] = (id + 4 - 1) % numParties + 1;
            open_map_mpc[1][1] = (id + 1 - 1) % numParties + 1;
            // send
            general_map[0][0] = ((id + 3 - 1) % numParties + 1);
            general_map[0][1] = ((id + 4 - 1) % numParties + 1);

            // recv
            general_map[1][0] = ((id + 2 - 1) % numParties + 1);
            general_map[1][1] = ((id + 1 - 1) % numParties + 1);

            // used for multiplication
            T_map_mpc[0][0] = ((id + 1 - 1) % numParties + 1);
            T_map_mpc[0][1] = ((id + 2 - 1) % numParties + 1);

            T_map_mpc[1][0] = ((id + 1 - 1) % numParties + 1);
            T_map_mpc[1][1] = ((id + 3 - 1) % numParties + 1);

            T_map_mpc[2][0] = ((id + 1 - 1) % numParties + 1);
            T_map_mpc[2][1] = ((id + 4 - 1) % numParties + 1);

            T_map_mpc[3][0] = ((id + 2 - 1) % numParties + 1);
            T_map_mpc[3][1] = ((id + 3 - 1) % numParties + 1);

            T_map_mpc[4][0] = ((id + 2 - 1) % numParties + 1);
            T_map_mpc[4][1] = ((id + 4 - 1) % numParties + 1);

            T_map_mpc[5][0] = ((id + 3 - 1) % numParties + 1);
            T_map_mpc[5][1] = ((id + 4 - 1) % numParties + 1);

            switch (id) {
            case 1:
                eda_map_mpc[0][0] = 5;
                eda_map_mpc[0][1] = 4;
                eda_map_mpc[1][0] = 2;
                eda_map_mpc[1][1] = 3;
                // eda_5pc_comm_ctr = 2;
                break;
            case 2:
                eda_map_mpc[0][0] = 1;
                eda_map_mpc[0][1] = 5;
                eda_map_mpc[1][0] = 3;
                eda_map_mpc[1][1] = -1;
                // eda_5pc_comm_ctr = 2;
                break;
            case 3:
                eda_map_mpc[0][0] = 2;
                eda_map_mpc[0][1] = 1;
                eda_map_mpc[1][0] = -1;
                eda_map_mpc[1][1] = -1;
                // eda_5pc_comm_ctr = 2;
                break;
            case 4:
                eda_map_mpc[0][0] = -1;
                eda_map_mpc[0][1] = -1;
                eda_map_mpc[1][0] = -1;
                eda_map_mpc[1][1] = 1;
                // eda_5pc_comm_ctr = 0;
                break;
            case 5:
                eda_map_mpc[0][0] = -1;
                eda_map_mpc[0][1] = -1;
                eda_map_mpc[1][0] = 1;
                eda_map_mpc[1][1] = 2;
                // eda_5pc_comm_ctr = 0;
                break;
            }

        } else if (numParties == 7) {
            printf("prg_setup_mp_7\n");
            prg_setup_mp_7();

            open_map_mpc[0][0] = (id + 1 - 1) % numParties + 1;
            open_map_mpc[0][1] = (id + 2 - 1) % numParties + 1;
            open_map_mpc[0][2] = (id + 3 - 1) % numParties + 1;

            open_map_mpc[1][0] = (id + 6 - 1) % numParties + 1;
            open_map_mpc[1][1] = (id + 5 - 1) % numParties + 1;
            open_map_mpc[1][2] = (id + 4 - 1) % numParties + 1;

            // send
            general_map[0][0] = ((id + 4 - 1) % numParties + 1);
            general_map[0][1] = ((id + 5 - 1) % numParties + 1);
            general_map[0][2] = ((id + 6 - 1) % numParties + 1);

            // recv
            general_map[1][0] = ((id + 3 - 1) % numParties + 1);
            general_map[1][1] = ((id + 2 - 1) % numParties + 1);
            general_map[1][2] = ((id + 1 - 1) % numParties + 1);

            T_map_mpc[0][0] = ((id + 1 - 1) % numParties + 1);
            T_map_mpc[0][1] = ((id + 2 - 1) % numParties + 1);
            T_map_mpc[0][2] = ((id + 3 - 1) % numParties + 1);

            T_map_mpc[1][0] = ((id + 1 - 1) % numParties + 1);
            T_map_mpc[1][1] = ((id + 2 - 1) % numParties + 1);
            T_map_mpc[1][2] = ((id + 4 - 1) % numParties + 1);

            T_map_mpc[2][0] = ((id + 1 - 1) % numParties + 1);
            T_map_mpc[2][1] = ((id + 2 - 1) % numParties + 1);
            T_map_mpc[2][2] = ((id + 5 - 1) % numParties + 1);

            T_map_mpc[3][0] = ((id + 1 - 1) % numParties + 1);
            T_map_mpc[3][1] = ((id + 2 - 1) % numParties + 1);
            T_map_mpc[3][2] = ((id + 6 - 1) % numParties + 1);

            T_map_mpc[4][0] = ((id + 1 - 1) % numParties + 1);
            T_map_mpc[4][1] = ((id + 3 - 1) % numParties + 1);
            T_map_mpc[4][2] = ((id + 4 - 1) % numParties + 1);

            T_map_mpc[5][0] = ((id + 1 - 1) % numParties + 1);
            T_map_mpc[5][1] = ((id + 3 - 1) % numParties + 1);
            T_map_mpc[5][2] = ((id + 5 - 1) % numParties + 1);

            T_map_mpc[6][0] = ((id + 1 - 1) % numParties + 1);
            T_map_mpc[6][1] = ((id + 3 - 1) % numParties + 1);
            T_map_mpc[6][2] = ((id + 6 - 1) % numParties + 1);

            T_map_mpc[7][0] = ((id + 1 - 1) % numParties + 1);
            T_map_mpc[7][1] = ((id + 4 - 1) % numParties + 1);
            T_map_mpc[7][2] = ((id + 5 - 1) % numParties + 1);

            T_map_mpc[8][0] = ((id + 1 - 1) % numParties + 1);
            T_map_mpc[8][1] = ((id + 4 - 1) % numParties + 1);
            T_map_mpc[8][2] = ((id + 6 - 1) % numParties + 1);

            T_map_mpc[9][0] = ((id + 1 - 1) % numParties + 1);
            T_map_mpc[9][1] = ((id + 5 - 1) % numParties + 1);
            T_map_mpc[9][2] = ((id + 6 - 1) % numParties + 1);

            T_map_mpc[10][0] = ((id + 2 - 1) % numParties + 1);
            T_map_mpc[10][1] = ((id + 3 - 1) % numParties + 1);
            T_map_mpc[10][2] = ((id + 4 - 1) % numParties + 1);

            T_map_mpc[11][0] = ((id + 2 - 1) % numParties + 1);
            T_map_mpc[11][1] = ((id + 3 - 1) % numParties + 1);
            T_map_mpc[11][2] = ((id + 5 - 1) % numParties + 1);

            T_map_mpc[12][0] = ((id + 2 - 1) % numParties + 1);
            T_map_mpc[12][1] = ((id + 3 - 1) % numParties + 1);
            T_map_mpc[12][2] = ((id + 6 - 1) % numParties + 1);

            T_map_mpc[13][0] = ((id + 2 - 1) % numParties + 1);
            T_map_mpc[13][1] = ((id + 4 - 1) % numParties + 1);
            T_map_mpc[13][2] = ((id + 5 - 1) % numParties + 1);

            T_map_mpc[14][0] = ((id + 2 - 1) % numParties + 1);
            T_map_mpc[14][1] = ((id + 4 - 1) % numParties + 1);
            T_map_mpc[14][2] = ((id + 6 - 1) % numParties + 1);

            T_map_mpc[15][0] = ((id + 2 - 1) % numParties + 1);
            T_map_mpc[15][1] = ((id + 5 - 1) % numParties + 1);
            T_map_mpc[15][2] = ((id + 6 - 1) % numParties + 1);

            T_map_mpc[16][0] = ((id + 3 - 1) % numParties + 1);
            T_map_mpc[16][1] = ((id + 4 - 1) % numParties + 1);
            T_map_mpc[16][2] = ((id + 5 - 1) % numParties + 1);

            T_map_mpc[17][0] = ((id + 3 - 1) % numParties + 1);
            T_map_mpc[17][1] = ((id + 4 - 1) % numParties + 1);
            T_map_mpc[17][2] = ((id + 6 - 1) % numParties + 1);

            T_map_mpc[18][0] = ((id + 3 - 1) % numParties + 1);
            T_map_mpc[18][1] = ((id + 5 - 1) % numParties + 1);
            T_map_mpc[18][2] = ((id + 6 - 1) % numParties + 1);

            T_map_mpc[19][0] = ((id + 4 - 1) % numParties + 1);
            T_map_mpc[19][1] = ((id + 5 - 1) % numParties + 1);
            T_map_mpc[19][2] = ((id + 6 - 1) % numParties + 1);

            switch (id) {
            case 1:
                eda_map_mpc[0][0] = 7;
                eda_map_mpc[0][1] = 6;
                eda_map_mpc[0][2] = 5;
                eda_map_mpc[1][0] = 2;
                eda_map_mpc[1][1] = 3;
                eda_map_mpc[1][2] = 4;
                // eda_7pc_comm_ctr = 3;
                break;
            case 2:
                eda_map_mpc[0][0] = 1;
                eda_map_mpc[0][1] = 7;
                eda_map_mpc[0][2] = 6;
                eda_map_mpc[1][0] = 3;
                eda_map_mpc[1][1] = 4;
                eda_map_mpc[1][2] = -1;
                // eda_7pc_comm_ctr = 3;
                break;
            case 3:
                eda_map_mpc[0][0] = 2;
                eda_map_mpc[0][1] = 1;
                eda_map_mpc[0][2] = 7;
                eda_map_mpc[1][0] = 4;
                eda_map_mpc[1][1] = -1;
                eda_map_mpc[1][2] = -1;
                // eda_7pc_comm_ctr = 3;
                break;
            case 4:
                eda_map_mpc[0][0] = 3;
                eda_map_mpc[0][1] = 2;
                eda_map_mpc[0][2] = 1;
                eda_map_mpc[1][0] = -1;
                eda_map_mpc[1][1] = -1;
                eda_map_mpc[1][2] = -1;
                // eda_7pc_comm_ctr = 3;
                break;
            case 5:
                eda_map_mpc[0][0] = -1;
                eda_map_mpc[0][1] = -1;
                eda_map_mpc[0][2] = -1;
                eda_map_mpc[1][0] = -1;
                eda_map_mpc[1][1] = -1;
                eda_map_mpc[1][2] = 1;
                // eda_7pc_comm_ctr = 0;
                break;
            case 6:
                eda_map_mpc[0][0] = -1;
                eda_map_mpc[0][1] = -1;
                eda_map_mpc[0][2] = -1;
                eda_map_mpc[1][0] = -1;
                eda_map_mpc[1][1] = 1;
                eda_map_mpc[1][2] = 2;
                // eda_7pc_comm_ctr = 0;
                break;
            case 7:
                eda_map_mpc[0][0] = -1;
                eda_map_mpc[0][1] = -1;
                eda_map_mpc[0][2] = -1;
                eda_map_mpc[1][0] = 1;
                eda_map_mpc[1][1] = 2;
                eda_map_mpc[1][2] = 3;
                // eda_7pc_comm_ctr = 0;
                break;
            }
        } else {
            printf("ERROR: numParties are not 3, 5, or 7\n");
        }
    }
    // init_index_array
}

template <typename T>
__m128i *RSS_SecretShare<T>::prg_keyschedule(uint8_t *src) {
    __m128i *r = (__m128i *)malloc(11 * sizeof(__m128i));

    r[0] = _mm_load_si128((__m128i *)src);

    KE2(r[1], r[0], 0x01)
    KE2(r[2], r[1], 0x02)
    KE2(r[3], r[2], 0x04)
    KE2(r[4], r[3], 0x08)
    KE2(r[5], r[4], 0x10)
    KE2(r[6], r[5], 0x20)
    KE2(r[7], r[6], 0x40)
    KE2(r[8], r[7], 0x80)
    KE2(r[9], r[8], 0x1b)
    KE2(r[10], r[9], 0x36)

    return r;
}
template <typename T>
void RSS_SecretShare<T>::prg_aes(uint8_t *dest, uint8_t *src, __m128i *ri) {
    __m128i rr, mr;
    __m128i *r = ri;

    rr = _mm_loadu_si128((__m128i *)src);
    mr = rr;

    mr = _mm_xor_si128(mr, r[0]);

    mr = _mm_aesenc_si128(mr, r[1]);
    mr = _mm_aesenc_si128(mr, r[2]);
    mr = _mm_aesenc_si128(mr, r[3]);
    mr = _mm_aesenc_si128(mr, r[4]);
    mr = _mm_aesenc_si128(mr, r[5]);
    mr = _mm_aesenc_si128(mr, r[6]);
    mr = _mm_aesenc_si128(mr, r[7]);
    mr = _mm_aesenc_si128(mr, r[8]);
    mr = _mm_aesenc_si128(mr, r[9]);
    mr = _mm_aesenclast_si128(mr, r[10]);
    mr = _mm_xor_si128(mr, rr);
    _mm_storeu_si128((__m128i *)dest, mr);
}

template <typename T>
void RSS_SecretShare<T>::prg_setup_3() {
    // need to create numShares+1 keys, random containers, etc
    uint numKeys = numShares + 1;
    random_container = new uint8_t *[numKeys];
    for (int i = 0; i < numKeys; i++) {
        random_container[i] = new uint8_t[16];
        memset(random_container[i], 0, sizeof(uint8_t) * 16);
    }

    prg_key = new __m128i *[numKeys];

    uint8_t tempKey_A[16];
    uint8_t tempKey_B[16];
    uint8_t tempKey_C[16]; // USED FOR OFFLINE, INDEPENDENT GENERATION

    uint8_t **tempKey = new uint8_t *[numKeys];
    for (int i = 0; i < numKeys; i++) {
        tempKey[i] = new uint8_t[16];
    }

    uint8_t RandomData[64];
    FILE *fp = fopen("/dev/urandom", "r");
    // reading 48 bits to generate 2 keys
    // last 16 are to seed private key
    // increaseing to 48 so each party has a private key
    // that will be used for offline prg_aes_ni (non-interactive)
    if (fread(RandomData, 1, 64, fp) != 64) {
        fprintf(stderr, "Could not read random bytes.");
        exit(1);
    }
    fclose(fp);

    memcpy(random_container[0], RandomData, 16);
    memcpy(tempKey_A, RandomData + 16, 16);
    memcpy(tempKey_C, RandomData + 32, 16);
    memcpy(random_container[2], RandomData + 48, 16);

    // sending to i + 2 mod n
    // receiving from i + 1 mod n
    // this depends on the threshold?
    // but we end up with two keys - one we generate and send, and one we receive
    int map[2];
    switch (id) {
    case 1:
        map[0] = 3;
        map[1] = 2;
        break;
    case 2:
        map[0] = 1;
        map[1] = 3;
        break;
    case 3:
        map[0] = 2;
        map[1] = 1;
        break;
    }

    nNet->sendDataToPeer(map[0], 32, RandomData);
    nNet->getDataFromPeer(map[1], 32, RandomData);

    memcpy(random_container[1], RandomData, 16);
    memcpy(tempKey_B, RandomData + 16, 16);

    prg_key[0] = prg_keyschedule(tempKey_A);
    prg_key[1] = prg_keyschedule(tempKey_B);
    prg_key[2] = prg_keyschedule(tempKey_C);

    uint8_t res[16] = {};
    for (size_t i = 0; i < numKeys; i++) {
        prg_aes(res, random_container[i], prg_key[i]);
        memcpy(random_container[i], res, 16);
    }

    P_container = new int[numKeys];
    memset(P_container, 0, sizeof(int) * numKeys);
    container_size = 16;
    printf("prg setup\n");

    for (int i = 0; i < numKeys; i++) {
        delete[] tempKey[i];
    }
    delete[] tempKey;
}
template <typename T>
void RSS_SecretShare<T>::prg_setup_5() {
    // need to create numShares+1 keys, random containers, etc
    uint numKeys = numShares + 1;
    random_container = new uint8_t *[numKeys];
    for (int i = 0; i < numKeys; i++) {
        random_container[i] = new uint8_t[16];
        memset(random_container[i], 0, sizeof(uint8_t) * 16);
    }
    prg_key = new __m128i *[numKeys];

    uint8_t **tempKey = new uint8_t *[numKeys];
    for (int i = 0; i < numKeys; i++) {
        tempKey[i] = new uint8_t[16];
        memset(tempKey[i], 0, sizeof(uint8_t) * 16);
    }

    // printf("num random bytes needed = %u\n",4 * 16 + 32);
    uint8_t RandomData[4 * 16 + 32];
    uint8_t RandomData_recv[32];
    memset(RandomData_recv, 0, sizeof(uint8_t) * 32);

    FILE *fp = fopen("/dev/urandom", "r");
    // reading 48 bits to generate 2 keys
    // last 16 are to seed private key
    // increaseing to 48 so each party has a private key
    // that will be used for offline prg_aes_ni (non-interactive)
    if (fread(RandomData, 1, 4 * 16 + 32, fp) != 4 * 16 + 32) {
        fprintf(stderr, "Could not read random bytes.");
        exit(1);
    }
    fclose(fp);

    memcpy(random_container[2], RandomData, 16);
    memcpy(tempKey[2], RandomData + 16, 16);

    memcpy(random_container[3], RandomData + 32, 16);
    memcpy(tempKey[3], RandomData + 48, 16);

    // for offline
    memcpy(random_container[numKeys - 1], RandomData + 64, 16);
    memcpy(tempKey[numKeys - 1], RandomData + 80, 16);

    int map[2][2];
    // star topology then ring
    map[0][0] = ((id + 2 - 1) % numParties + numParties) % numParties + 1; // send s_3
    map[0][1] = ((id + 1 - 1) % numParties + numParties) % numParties + 1; // send s_3
    map[1][0] = ((id - 2 - 1) % numParties + numParties) % numParties + 1; // recv s_5
    map[1][1] = ((id - 1 - 1) % numParties + numParties) % numParties + 1; // recv s_2

    nNet->sendDataToPeer(map[0][0], 32, RandomData);
    nNet->getDataFromPeer(map[1][0], 32, RandomData_recv);

    memcpy(random_container[4], RandomData_recv, 16);
    memcpy(tempKey[4], RandomData_recv + 16, 16);

    nNet->sendDataToPeer(map[1][0], 32, RandomData);
    nNet->getDataFromPeer(map[0][0], 32, RandomData_recv);

    memcpy(random_container[1], RandomData_recv, 16);
    memcpy(tempKey[1], RandomData_recv + 16, 16);

    nNet->sendDataToPeer(map[0][1], 32, RandomData + 32);
    nNet->getDataFromPeer(map[1][1], 32, RandomData_recv);

    memcpy(random_container[0], RandomData_recv, 16);
    memcpy(tempKey[0], RandomData_recv + 16, 16);
    // memset(RandomData_recv, 0, sizeof(uint8_t) * 32);

    nNet->sendDataToPeer(map[1][1], 32, RandomData + 32);
    nNet->getDataFromPeer(map[0][1], 32, RandomData_recv);

    memcpy(random_container[5], RandomData_recv, 16);
    memcpy(tempKey[5], RandomData_recv + 16, 16);
    // memset(RandomData_recv, 0, sizeof(uint8_t) * 32);

    for (size_t i = 0; i < numKeys; i++) {
        prg_key[i] = prg_keyschedule(tempKey[i]);
    }

    uint8_t res[16] = {};
    for (size_t i = 0; i < numKeys; i++) {
        prg_aes(res, random_container[i], prg_key[i]);
        memcpy(random_container[i], res, 16);
    }

    P_container = new int[numKeys];
    memset(P_container, 0, sizeof(int) * numKeys);
    container_size = 16;
    printf("prg setup\n");

    for (int i = 0; i < numKeys; i++) {
        delete[] tempKey[i];
    }
    delete[] tempKey;
}

template <typename T>
void RSS_SecretShare<T>::prg_setup_mp_7() {
    // need to create numShares+1 keys, random containers, etc
    uint numKeys = numShares + 1;
    random_container = new uint8_t *[numKeys];
    for (int i = 0; i < numKeys; i++) {
        random_container[i] = new uint8_t[16];
        memset(random_container[i], 0, sizeof(uint8_t) * 16);
    }
    // printf("numKeys : %i \n", numKeys);
    // printf("numShares : %i \n", numShares);

    prg_key = new __m128i *[numKeys];

    uint8_t **tempKey = new uint8_t *[numKeys];
    for (int i = 0; i < numKeys; i++) {
        tempKey[i] = new uint8_t[16];
        memset(tempKey[i], 0, sizeof(uint8_t) * 16);
    }

    // printf("num random bytes needed = %u\n",4 * 16 + 32);
    // 32 bytes needed per key since we need to fill rand_container AND tempkey
    uint8_t RandomData[5 * (2 * 16) + (2 * 16)];
    uint8_t RandomData_recv[32];
    memset(RandomData_recv, 0, sizeof(uint8_t) * 32);

    FILE *fp = fopen("/dev/urandom", "r");
    if (fread(RandomData, 1, (5 * (2 * 16) + (2 * 16)), fp) != (5 * (2 * 16) + (2 * 16))) {
        fprintf(stderr, "Could not read random bytes.");
        exit(1);
    }
    fclose(fp);

    // each party generates 5 keys, s.t. 5*7 = 35 total keys are generated
    for (size_t i = 0; i < 4; i++) {
        memcpy(random_container[i], RandomData + 2 * i * 16, 16);
        memcpy(tempKey[i], RandomData + (2 * i + 1) * 16, 16);
    }

    memcpy(random_container[5], RandomData + 4 * (2 * 16), 16);
    memcpy(tempKey[5], RandomData + 4 * (2 * 16) + 16, 16);

    // for offline
    memcpy(random_container[numKeys - 1], RandomData + 5 * (2 * 16), 16);
    memcpy(tempKey[numKeys - 1], RandomData + 5 * (2 * 16) + 16, 16);

    int map[5][2][3];

    map[0][0][0] = ((id + 4 - 1) % numParties + numParties) % numParties + 1; // send s1
    map[0][0][1] = ((id + 5 - 1) % numParties + numParties) % numParties + 1; // send s1
    map[0][0][2] = ((id + 6 - 1) % numParties + numParties) % numParties + 1; // send s1
    map[0][1][0] = ((id + 3 - 1) % numParties + numParties) % numParties + 1; // recv s20
    map[0][1][1] = ((id + 2 - 1) % numParties + numParties) % numParties + 1; // recv s17
    map[0][1][2] = ((id + 1 - 1) % numParties + numParties) % numParties + 1; // recv s11

    map[1][0][0] = ((id + 3 - 1) % numParties + numParties) % numParties + 1; // send s2
    map[1][0][1] = ((id + 5 - 1) % numParties + numParties) % numParties + 1; // send s2
    map[1][0][2] = ((id + 6 - 1) % numParties + numParties) % numParties + 1; // send s2
    map[1][1][0] = ((id + 4 - 1) % numParties + numParties) % numParties + 1; // recv s10
    map[1][1][1] = ((id + 2 - 1) % numParties + numParties) % numParties + 1; // recv s18
    map[1][1][2] = ((id + 1 - 1) % numParties + numParties) % numParties + 1; // recv s12

    map[2][0][0] = ((id + 3 - 1) % numParties + numParties) % numParties + 1; // send s3
    map[2][0][1] = ((id + 4 - 1) % numParties + numParties) % numParties + 1; // send s3
    map[2][0][2] = ((id + 6 - 1) % numParties + numParties) % numParties + 1; // send s3
    map[2][1][0] = ((id + 4 - 1) % numParties + numParties) % numParties + 1; // recv s16
    map[2][1][1] = ((id + 3 - 1) % numParties + numParties) % numParties + 1; // recv s8
    map[2][1][2] = ((id + 1 - 1) % numParties + numParties) % numParties + 1; // recv s13

    map[3][0][0] = ((id + 3 - 1) % numParties + numParties) % numParties + 1; // send s4
    map[3][0][1] = ((id + 4 - 1) % numParties + numParties) % numParties + 1; // send s4
    map[3][0][2] = ((id + 5 - 1) % numParties + numParties) % numParties + 1; // send s4
    map[3][1][0] = ((id + 4 - 1) % numParties + numParties) % numParties + 1; // recv s19
    map[3][1][1] = ((id + 3 - 1) % numParties + numParties) % numParties + 1; // recv s14
    map[3][1][2] = ((id + 2 - 1) % numParties + numParties) % numParties + 1; // recv s5

    map[4][0][0] = ((id + 2 - 1) % numParties + numParties) % numParties + 1; // send s6
    map[4][0][1] = ((id + 4 - 1) % numParties + numParties) % numParties + 1; // send s6
    map[4][0][2] = ((id + 6 - 1) % numParties + numParties) % numParties + 1; // send s6
    map[4][1][0] = ((id + 5 - 1) % numParties + numParties) % numParties + 1; // recv s7
    map[4][1][1] = ((id + 3 - 1) % numParties + numParties) % numParties + 1; // recv s9
    map[4][1][2] = ((id + 1 - 1) % numParties + numParties) % numParties + 1; // recv s15

    uint8_t recived_order[5][3] = {
        {19, 17, 10},
        {9, 17, 11},
        {15, 7, 12},
        {18, 13, 4},
        {6, 8, 14},
    };
    for (size_t i = 0; i < 4; i++) {
        for (size_t j = 0; j < 3; j++) {
            nNet->sendDataToPeer(map[i][0][j], 32, RandomData + 32 * i);
            nNet->getDataFromPeer(map[i][1][j], 32, RandomData_recv);

            memcpy(random_container[recived_order[i][j]], RandomData_recv, 16);
            memcpy(tempKey[recived_order[i][j]], RandomData_recv + 16, 16);
        }
    }

    // doing last one separately since we had to skip s5
    for (size_t j = 0; j < 3; j++) {
        nNet->sendDataToPeer(map[4][0][j], 32, RandomData + 32 * 4);
        nNet->getDataFromPeer(map[4][1][j], 32, RandomData_recv);

        memcpy(random_container[recived_order[4][j]], RandomData_recv, 16);
        memcpy(tempKey[recived_order[4][j]], RandomData_recv + 16, 16);
    }

    for (size_t i = 0; i < numKeys; i++) {
        prg_key[i] = prg_keyschedule(tempKey[i]);
    }

    uint8_t res[16] = {};
    for (size_t i = 0; i < numKeys; i++) {
        prg_aes(res, random_container[i], prg_key[i]);
        memcpy(random_container[i], res, 16);
    }

    P_container = new int[numKeys];
    memset(P_container, 0, sizeof(int) * numKeys);
    container_size = 16;
    printf("prg setup\n");

    for (int i = 0; i < numKeys; i++) {
        delete[] tempKey[i];
    }
    delete[] tempKey;
}

template <typename T>
void RSS_SecretShare<T>::prg_getrandom(int keyID, uint size, uint length, uint8_t *dest) {
    // we assume container_size is 16, so all *container_size are replaced as <<4
    // this size means how many random bytes we need
    // uint8_t *buffer = new uint8_t [size];
    // its always size * length
    // printf("curent P is %d \n",P_container[keyID]);
    uint rounds = ((size * length - container_size + P_container[keyID]) + 15) >> 4;
    if (rounds == 0) {
        memcpy(dest, random_container[keyID] + P_container[keyID], size * length);
        P_container[keyID] = P_container[keyID] + size * length;
    } else {
        memcpy(dest, random_container[keyID] + P_container[keyID], container_size - P_container[keyID]);
        if (rounds >= 2) {
            prg_aes(dest + (container_size - P_container[keyID]), random_container[keyID], prg_key[keyID]);
            for (int i = 1; i < rounds - 1; i++) {
                // segfault in this loop for "large" size
                // printf("i : %u\n", i);
                prg_aes(dest + (container_size - P_container[keyID]) + (i << 4), dest + (container_size - P_container[keyID]) + ((i - 1) << 4), prg_key[keyID]);
            }
            prg_aes(random_container[keyID], dest + (container_size - P_container[keyID]) + ((rounds - 2) << 4), prg_key[keyID]);
            memcpy(dest + container_size - P_container[keyID] + ((rounds - 1) << 4), random_container[keyID], size * length - ((rounds - 1) << 4) - container_size + P_container[keyID]);
            P_container[keyID] = size * length - ((rounds - 1) << 4) - container_size + P_container[keyID];
        } else {
            prg_aes(random_container[keyID], random_container[keyID], prg_key[keyID]);
            memcpy(dest + container_size - P_container[keyID], random_container[keyID], size * length - container_size + P_container[keyID]);
            P_container[keyID] = size * length - container_size + P_container[keyID];
        }
    }

    // delete [] buffer;
}

template <typename T>
void RSS_SecretShare<T>::prg_getrandom(uint size, uint length, uint8_t *dest) {
    // uses party's own key not shared with others to locally generate shares
    // we assume container_size is 16, so all *container_size are replaced as <<4
    // this size means how many random bytes we need
    // when this functions is called we use the party's private key stored in the LAST position [numKeys - 1] (or just numShares since numShares = numKeys + 1)
    uint keyID = numShares;
    uint rounds = ((size * length - container_size + P_container[keyID]) + 15) >> 4;
    if (rounds == 0) {
        memcpy(dest, random_container[keyID] + P_container[keyID], size * length);
        P_container[keyID] = P_container[keyID] + size * length;
    } else {
        memcpy(dest, random_container[keyID] + P_container[keyID], container_size - P_container[keyID]);
        if (rounds >= 2) {
            prg_aes(dest + (container_size - P_container[keyID]), random_container[keyID], prg_key[keyID]);
            for (int i = 1; i < rounds - 1; i++) {
                prg_aes(dest + (container_size - P_container[keyID]) + (i << 4), dest + (container_size - P_container[keyID]) + ((i - 1) << 4), prg_key[keyID]);
            }
            prg_aes(random_container[keyID], dest + (container_size - P_container[keyID]) + ((rounds - 2) << 4), prg_key[keyID]);
            memcpy(dest + container_size - P_container[keyID] + ((rounds - 1) << 4), random_container[keyID], size * length - ((rounds - 1) << 4) - container_size + P_container[keyID]);
            P_container[keyID] = size * length - ((rounds - 1) << 4) - container_size + P_container[keyID];
        } else {
            prg_aes(random_container[keyID], random_container[keyID], prg_key[keyID]);
            memcpy(dest + container_size - P_container[keyID], random_container[keyID], size * length - container_size + P_container[keyID]);
            P_container[keyID] = size * length - container_size + P_container[keyID];
        }
    }

    // delete [] buffer;
}

template <typename T>
uint RSS_SecretShare<T>::getNumParties() {
    return numParties;
}

template <typename T>
uint RSS_SecretShare<T>::getThreshold() {
    return threshold;
}

template <typename T>
uint RSS_SecretShare<T>::getNumShares() {
    return numShares;
}

template <typename T>
uint RSS_SecretShare<T>::getTotalNumShares() {
    return totalNumShares;
}

template <typename T>
uint RSS_SecretShare<T>::nCk(uint n, uint k) {

    if (k > n) {
        printf("Error: n must be >= k\n");
        return -1;
    } else {
        uint res = 1;
        if (k > n - k) {
            k = n - k;
        }
        for (uint i = 0; i < k; ++i) {
            res *= ((uint)n - i);
            res /= (i + (uint)1);
        }
        return res;
    }
}

// destructor
template <typename T>
RSS_SecretShare<T>::~RSS_SecretShare() {
    for (int i = 0; i < numShares + 1; i++) {
        delete[] random_container[i];
        free(prg_key[i]);
    }

    for (size_t i = 0; i < 2; i++) {
        delete[] general_map[i];
    }
    delete[] general_map;

    if (numParties == 5 or numParties == 7) {
    for (size_t i = 0; i < 2; i++) {
        delete[] open_map_mpc[i];
        delete[] eda_map_mpc[i];
    }
        for (size_t i = 0; i < numShares; i++) {
            delete[] T_map_mpc[i];
        }
        delete[] open_map_mpc;
        delete[] eda_map_mpc;
        delete[] T_map_mpc;
    }

    delete[] random_container;
    delete[] prg_key;
    delete[] P_container;

    delete[] SHIFT;
    delete[] ODD;
    delete[] EVEN;
}

#endif