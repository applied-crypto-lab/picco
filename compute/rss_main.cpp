/* File generated from [sample-programs/non-thread/rss_test.c] by PICCO Thu Aug 15 10:33:27 2024
 */

#include <limits.h>

#include <float.h>

extern "C" int ort_initialize(int *, char ***);
extern "C" void ort_finalize(int);

#include "smc-compute/SMC_Utils.h"

#include <gmp.h>

SMC_Utils *__s;
std::vector<int> seed_map;

/* smc-compiler generated main() */
int main(int argc, char **argv) {

    if (argc < 7) {
        fprintf(stderr, "Incorrect input parameters\n");
        fprintf(stderr, "Usage: <id> <runtime-config> <privatekey-filename> <num_parties> <ring_size> <batch_size>\n");
        exit(1);
    }

    int threshold;
    int numParties = atoi(argv[4]);
    int ring_size = atoi(argv[5]);
    int batch_size = atoi(argv[6]);

    switch (numParties) {
    case 3:
        seed_map = {3};
        threshold = 1;
        break;
    case 5:
        seed_map = {7, 11};
        threshold = 2;
        break;
    case 7:
        seed_map = {15, 23, 27, 29, 43};
        threshold = 3;
        break;
    default:
        std::cerr << "wrong number of parties!" << std::endl;
        exit(1);
        break;
    }

    __s = new SMC_Utils(atoi(argv[1]), argv[2], argv[3], 0, 0, NULL, numParties, threshold, ring_size, "", seed_map, 1);

    struct timeval tv1;
    struct timeval tv2;
    int _xval = 0;

    gettimeofday(&tv1, NULL);
    __s->smc_test_rss(-1, batch_size);
    gettimeofday(&tv2, NULL);
    std::cout << "Time: " << __s->time_diff(&tv1, &tv2) << " seconds " << std::endl;
    return (_xval);
}
