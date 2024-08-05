#include "Mult.hpp"

void Rss_Mult_7pc_test(priv_int_t **c, priv_int_t **a, priv_int_t **b, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<priv_int_t> *ss) {
    std::cout << "Rss_Mult_7pc_test" << std::endl;
    std::cout << "size  " << size << std::endl;
    std::cout << "ring_size  " << ring_size << std::endl;

    struct timeval start;
    struct timeval end;
    unsigned long timer = 0;

    uint bytes = (ring_size + 7) >> 3;
    uint i, p_prime, T_index;
    uint numShares = ss->getNumShares();
    uint numParties = ss->getPeers();
    uint threshold = ss->getThreshold();
    static int pid = ss->getID();

    priv_int_t *v = new priv_int_t[size];
    uint8_t prg_ctrs[20] = {3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 4, 4, 4, 4, 4, 3, 4, 4, 3};

    priv_int_t **recv_buf = new priv_int_t *[threshold];
    for (i = 0; i < threshold; i++) {
        recv_buf[i] = new priv_int_t[size];
        memset(recv_buf[i], 0, sizeof(priv_int_t) * size);
    }

    uint8_t **buffer = new uint8_t *[numShares];
    for (i = 0; i < numShares; i++) {
        buffer[i] = new uint8_t[prg_ctrs[i] * bytes * size];
        ss->prg_getrandom(i, bytes, prg_ctrs[i] * size, buffer[i]);
    }
    priv_int_t z = priv_int_t(0);
    uint tracker;
    gettimeofday(&start, NULL);
    for (i = 0; i < size; i++) {
        v[i] =
            a[0][i] * (b[0][i] + b[1][i] + b[2][i] + b[3][i] + b[4][i] + b[5][i] + b[6][i] + b[7][i] + b[8][i] + b[9][i] + b[10][i] + b[11][i] + b[12][i] + b[13][i] + b[14][i] + b[15][i] + b[16][i] + b[17][i] + b[18][i] + b[19][i]) +
            a[1][i] * (b[0][i] + b[1][i] + b[2][i] + b[3][i] + b[4][i] + b[5][i] + b[6][i] + b[7][i] + b[8][i] + b[9][i] + b[10][i] + b[11][i] + b[12][i] + b[13][i] + b[14][i] + b[15][i] + b[16][i] + b[17][i] + b[18][i] + b[19][i]) +
            a[2][i] * (b[0][i] + b[1][i] + b[2][i] + b[3][i] + b[4][i] + b[5][i] + b[6][i] + b[7][i] + b[8][i] + b[9][i] + b[10][i] + b[11][i] + b[12][i] + b[13][i] + b[14][i] + b[15][i] + b[16][i] + b[17][i] + b[18][i] + b[19][i]) +
            a[3][i] * (b[0][i] + b[1][i] + b[2][i] + b[3][i] + b[4][i] + b[5][i] + b[6][i] + b[7][i] + b[8][i] + b[9][i] + b[10][i] + b[11][i] + b[12][i] + b[13][i] + b[14][i] + b[15][i] + b[16][i] + b[17][i] + b[18][i] + b[19][i]) +
            a[4][i] * (b[0][i] + b[1][i] + b[2][i] + b[3][i] + b[10][i] + b[11][i] + b[12][i] + b[13][i] + b[15][i]) +
            a[5][i] * (b[0][i] + b[1][i] + b[2][i] + b[3][i] + b[4][i] + b[5][i] + b[6][i] + b[7][i] + b[8][i] + b[9][i] + b[10][i] + b[11][i] + b[12][i] + b[13][i] + b[14][i] + b[15][i] + b[16][i] + b[17][i] + b[18][i] + b[19][i]) +
            a[6][i] * (b[2][i] + b[5][i] + b[7][i] + b[9][i] + b[11][i] + b[13][i]) +
            a[7][i] * (b[0][i] + b[4][i] + b[5][i] + b[6][i] + b[10][i] + b[11][i] + b[12][i]) +
            a[8][i] * (b[0][i] + b[4][i] + b[5][i] + b[10][i] + b[11][i] + b[16][i]) +
            a[9][i] * (b[1][i] + b[4][i] + b[7][i] + b[8][i] + b[10][i] + b[13][i]) +
            a[10][i] * (b[0][i] + b[1][i] + b[2][i] + b[3][i] + b[4][i] + b[5][i] + b[9][i]) +
            a[11][i] * (b[0][i] + b[1][i] + b[4][i] + b[6][i] + b[7][i] + b[8][i]) +
            a[12][i] * (b[0][i] + b[1][i] + b[2][i] + b[4][i] + b[5][i] + b[7][i]) +
            a[13][i] * (b[0][i] + b[4][i] + b[5][i] + b[6][i]) +
            a[14][i] * (b[2][i] + b[4][i] + b[5][i]) +
            a[15][i] * (b[1][i] + b[4][i]) +
            a[16][i] * (b[0][i] + b[1][i] + b[2][i] + b[3][i] + b[15][i]) +
            a[17][i] * (b[0][i] + b[1][i] + b[2][i]) +
            a[18][i] * (b[1][i] + b[8][i]) +
            a[19][i] * (b[0][i] + b[5][i] + b[6][i]);
    // }
    // for (int s = 0; s < numShares; s++) {
    //     // sanitizing after the product is computed, so we can reuse the buffer
    //     memset(c[s], 0, sizeof(priv_int_t) * size);
    // }
    // for (i = 0; i < size; i++) {

        // printf("finished calculating v\n");
        for (p_prime = 1; p_prime < numParties + 1; p_prime++) {
            // printf("\n");
            for (T_index = 0; T_index < numShares; T_index++) {
                tracker = 0;
                if ((p_prime != (pid)) and (!(p_prime_in_T_7(p_prime, ss->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T_7(p_prime, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[T_index][i] += z;
                    tracker += 1;
                } else if (
                    (p_prime == pid) and (!(chi_p_prime_in_T_7(pid, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[T_index][i] += z;
                    v[i] -= z;

                    tracker += 1;
                }
            }
        }
    }
    gettimeofday(&end, NULL); // stop timer here
    timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    printf("[7pc local] [%.3lf ms]\n", (double)(timer * 0.001));

    gettimeofday(&start, NULL);
    // communication
    // nodeNet.SendAndGetDataFromPeer_Mult(v, recv_buf, size, ring_size);
    nodeNet.SendAndGetDataFromPeer(v, recv_buf, size, ring_size, ss->general_map);

    gettimeofday(&end, NULL); // stop timer here
    timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    printf("[7pc send recv] [%.3lf ms]\n", (double)(timer * 0.001));

    // ss->prg_getrandom(0, bytes, size, buffer);
    for (i = 0; i < size; i++) {
        c[19][i] = c[19][i] + recv_buf[0][i];
        c[16][i] = c[16][i] + recv_buf[1][i];
        c[10][i] = c[10][i] + recv_buf[2][i];

        c[0][i] = c[0][i] + v[i];
    }

    for (i = 0; i < threshold; i++) {
        delete[] recv_buf[i];
    }

    for (i = 0; i < numShares; i++) {
        delete[] buffer[i];
    }

    // free
    delete[] v;
    delete[] buffer;
    delete[] recv_buf;
}