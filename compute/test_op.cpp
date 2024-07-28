/* File generated from [sample-programs/non-thread/float-test.c] by PICCO Sat Jul 20 17:15:38 2024
 */

#include <limits.h>

#include <float.h>

// extern void *memcpy(void*,const void*,unsigned int);

// # 1 "ort.onoff.defs"

extern "C" int ort_initialize(int *, char ***);
extern "C" void ort_finalize(int);

#include "smc-compute/SMC_Utils.h"

#include <gmp.h>

SMC_Utils *__s;

std::vector<int> seed_map = {3};

int __original_main(int _argc_ignored, char **_argv_ignored) {

    priv_int _picco_tmp1;
    ss_init(_picco_tmp1);

    priv_int *_picco_ftmp1 = (priv_int *)malloc(sizeof(priv_int) * 4);
    for (int _picco_j = 0; _picco_j < 4; _picco_j++)
        ss_init(_picco_ftmp1[_picco_j]);
    priv_int *_picco_ftmp2 = (priv_int *)malloc(sizeof(priv_int) * 4);
    for (int _picco_j = 0; _picco_j < 4; _picco_j++)
        ss_init(_picco_ftmp2[_picco_j]);
    void *_picco_temp_;
    priv_int **A;
    A = (priv_int **)malloc(sizeof(priv_int *) * (3));
    for (int _picco_i = 0; _picco_i < 3; _picco_i++) {
        A[_picco_i] = (priv_int *)malloc(sizeof(priv_int) * (4));
        for (int _picco_j = 0; _picco_j < 4; _picco_j++)
            ss_init(A[_picco_i][_picco_j]);
    }

    __s->smc_input(1, A, 3, "float", -1);

    __s->smc_open(A[0], -1);

    __s->smc_open(A[1], -1);

    // std::cout << "a0 =" << __s->smc_open(A[0], -1) << std::endl;
    // std::cout << "a1 =" << __s->smc_open(A[1], -1) << std::endl;
    __s->smc_add(A[0], A[1], _picco_ftmp1, 32, 9, 32, 9, 32, 9, "float", -1);
    __s->smc_set(_picco_ftmp1, A[2], 32, 9, 32, 9, "float", -1);

    std::cout << "a0+ a1 =" << __s->smc_open(A[2], -1) << std::endl;

    for (int _picco_i = 0; _picco_i < 3; _picco_i++) {
        for (int _picco_j = 0; _picco_j < 4; _picco_j++)
            ss_clear(A[_picco_i][_picco_j]);
        free(A[_picco_i]);
    }
    free(A);
    ss_clear(_picco_tmp1);
    for (int _picco_j = 0; _picco_j < 4; _picco_j++)
        ss_clear(_picco_ftmp1[_picco_j]);
    free(_picco_ftmp1);
    for (int _picco_j = 0; _picco_j < 4; _picco_j++)
        ss_clear(_picco_ftmp2[_picco_j]);
    free(_picco_ftmp2);
    return (0);
}

/* smc-compiler generated main() */
int main(int argc, char **argv) {

    if (argc < 8) {
        fprintf(stderr, "Incorrect input parameters\n");
        fprintf(stderr, "Usage: <id> <runtime-config> <privatekey-filename> <number-of-input-parties> <number-of-output-parties> <input-share> <output>\n");
        exit(1);
    }

    std::string IO_files[atoi(argv[4]) + atoi(argv[5])];
    for (int i = 0; i < argc - 6; i++)
        IO_files[i] = argv[6 + i];

    __s = new SMC_Utils(atoi(argv[1]), argv[2], argv[3], atoi(argv[4]), atoi(argv[5]), IO_files, 3, 1, 116, "41538374868278621028243970633760839", seed_map, 1);

    struct timeval tv1;
    struct timeval tv2;
    int _xval = 0;

    gettimeofday(&tv1, NULL);

    _xval = (int)__original_main(argc, argv);
    gettimeofday(&tv2, NULL);
    std::cout << "Time: " << __s->time_diff(&tv1, &tv2) << " seconds " << std::endl;
    return (_xval);
}
