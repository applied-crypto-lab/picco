/* File generated from [sample-programs/non-thread/test-op.c] by PICCO Tue Feb 27 11:39:38 2024
 */

#include <iostream>
#include <limits.h>

#include <float.h>

// extern void *memcpy(void*,const void*,unsigned int);

// # 1 "ort.onoff.defs"

extern "C" int ort_initialize(int *, char ***);
extern "C" void ort_finalize(int);

#include "smc-compute/SMC_Utils.h"

// #include <gmp.h>

SMC_Utils *__s;

std::vector<int> seed_map;

int __original_main(int _argc_ignored, char **_argv_ignored) {

    int K = 5;
    std::cout << "numShares : " << __s->getNumShares() << std::endl;

    // expected RSS initialization
    priv_int *A = new priv_int[__s->getNumShares()];
    for (int _picco_i = 0; _picco_i < __s->getNumShares(); _picco_i++) {
        A[_picco_i] = new std::remove_pointer_t<priv_int>[K];
        memset(A[_picco_i], 0, sizeof(std::remove_pointer_t<priv_int>) * K);
    }
    // current
    //    priv_int* A;
    //       A = (priv_int*)malloc(sizeof(priv_int) * (K));
    //       for (int _picco_i = 0; _picco_i < K; _picco_i++)
    //             ss_init(A[_picco_i]);
    priv_int A_open = new std::remove_pointer_t<priv_int>[K];
    memset(A_open, 0, sizeof(std::remove_pointer_t<priv_int>) * K);

    printf("sizeof(priv_int)   = %lu\n", sizeof(std::remove_pointer_t<priv_int>));
    printf("sizeof(priv_int)*8 = %lu\n", 8 * sizeof(std::remove_pointer_t<priv_int>));

    int B[K];

    // printf("smc_input1\n");
    // __s->smc_input(1, A, K, "int", -1);
    // // printf("smc_input2\n");
    // __s->smc_input(1, B, K, "int", -1);

    // __s->smc_test_rss(A, B, K, -1);

    // expected
    for (size_t _picco_i = 0; _picco_i < __s->getNumShares(); _picco_i++) {
        delete[] A[_picco_i];
    }
    delete[] A;
    delete[] A_open;

    /* // current
          for (int _picco_i = 0; _picco_i < K; _picco_i++)
              ss_clear(A[_picco_i]);
        free(A);
         */
    return (0);
}

/* smc-compiler generated main() */
int main(int argc, char **argv) {

    // added new argument for numParties
    if (argc < 7) {
        fprintf(stderr, "Incorrect input parameters\n");
        fprintf(stderr, "Usage: <id> <runtime-config> <privatekey-filename> <numParties> <exp_name> <ring_size> <batch_size> <num_iterations>\n");
        exit(1);
    }

    int numParties = atoi(argv[4]);
    std::string exp_name = argv[5];
    uint ring_size = atoi(argv[6]);
    int batch_size = atoi(argv[7]);
    int num_iterations = atoi(argv[8]);

    // seed_map is computed by picco, leaving here for testing purposes (single main file )
    switch (numParties) {
    case 3:
        seed_map = {3};
        break;
    case 5:
        seed_map = {7, 11};
        break;
    case 7:
        seed_map = {15, 23, 27, 29, 43};
        break;
    default:
        std::cerr<<"wrong number of parties!"<<endl;
        exit(1);
        break;
    }

    printf("numParties : %i\n",numParties);
    printf("threshold : %i\n", numParties / 2);
    __s = new SMC_Utils(atoi(argv[1]), argv[2], argv[3], 0, 0, NULL, numParties, numParties / 2, ring_size, "", seed_map, 1);

    struct timeval tv1;
    struct timeval tv2;
    int _xval = 0;

    // gettimeofday(&tv1, NULL);
    __s->smc_rss_benchmark(exp_name, batch_size, num_iterations);
    // _xval = (int)__original_main(argc, argv);
    // gettimeofday(&tv2, NULL);

    // std::cout << "Time: " << __s->time_diff(&tv1, &tv2) << " seconds" << std::endl; // in seconds
    return (0);
}
