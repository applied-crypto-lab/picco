/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame
   ** Department of Computer Science and Engineering, University at Buffalo (SUNY)

   PICCO is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   PICCO is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with PICCO. If not, see <http://www.gnu.org/licenses/>.
*/
#include "SMC_Utils.h"

// Constructors
SMC_Utils::SMC_Utils(int _id, std::string runtime_config, std::string privatekey_filename, int numOfInputPeers, int numOfOutputPeers, std::string *IO_files, int numOfPeers, int threshold, int bits, std::string mod, std::vector<int> &seed_map, int num_threads) {
    id = _id;
#if __DEPLOYMENT__
    printf("DEPLOYMENT MODE\n");
#else
    printf("BENCHMARK MODE\n");
#endif

    std::cout << "SMC_Utils constructor\n";
    mpz_t modulus;
    mpz_init(modulus);
    mpz_set_str(modulus, mod.c_str(), BASE_10);
    nodeConfig = new NodeConfiguration(id, runtime_config, bits);

    std::cout << "Creating the NodeNetwork\n";
    NodeNetwork *nodeNet = new NodeNetwork(nodeConfig, privatekey_filename, num_threads);
    net = *nodeNet; // dereferencing, net is "copy initialized"
    seedSetup(seed_map, numOfPeers, threshold);

    // only starting the manager if the program we're running is multithreaded
    // actually doesnt work in the current state, because the way multicast/broadcast is written
    // even if we're running a single-threaded program, it still sends the mode/threadID/some other BS that interferes
    // need to launch manager regardless
    // if (num_threads > 1) {
    net.launchManager(); // launching thread manager here as to not conflict with seed setup
    // }

    std::cout << "Creating SecretShare\n";
    ss = new SecretShare(numOfPeers, threshold, modulus, id, num_threads, net.getPRGseeds(), shamir_seeds_coefs);

// initialize input and output streams (deployment mode only)
#if __DEPLOYMENT__
    inputStreams = new std::ifstream[numOfInputPeers];
    outputStreams = new std::ofstream[numOfOutputPeers];
    for (int i = 0; i < numOfInputPeers; i++) {
        inputStreams[i].open(IO_files[i].c_str(), std::ifstream::in);
        if (!inputStreams[i]) {
            std::cout << "Input files could not be opened\n";
            std::exit(1);
        }
    }

    for (int i = 0; i < numOfOutputPeers; i++) {
        std::stringstream c;
        c << id;
        std::string s = c.str();
        IO_files[numOfInputPeers + i] = IO_files[numOfInputPeers + i] + s;
        outputStreams[i].open(IO_files[numOfInputPeers + i].c_str(), std::ofstream::out);
        if (!outputStreams[i]) {
            std::cout << "Output files could not be opened\n";
            std::exit(1);
        }
    }
#endif
}

/* Specific SMC Utility Functions */
// open a single integer value
int SMC_Utils::smc_open(mpz_t var, int threadID) {
    return Open_int(var, threadID, net, ss);
}

// open a single floating-point value
float SMC_Utils::smc_open(mpz_t *var, int threadID) {
    return Open_float(var, threadID, net, ss);
}

// for integer variable I/O
void SMC_Utils::smc_input(int id, int *var, std::string type, int threadID) {
    ss->ss_input(id, var, type, inputStreams);
}

void SMC_Utils::smc_input(int id, mpz_t *var, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_input(id, var, type, inputStreams);
#else
    ss->ss_input(var, type);
#endif
}

// for float variable I/O
void SMC_Utils::smc_input(int id, float *var, std::string type, int threadID) {
    ss->ss_input(id, var, type, inputStreams);
}

void SMC_Utils::smc_input(int id, mpz_t **var, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_input(id, var, type, inputStreams);
#else
    ss->ss_input(var, type);
#endif
}

// one-dimensional int array I/O
void SMC_Utils::smc_input(int id, mpz_t *var, int size, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_input(id, var, size, type, inputStreams);
#else
    ss->ss_input(var, size, type);
#endif
}

void SMC_Utils::smc_input(int id, int *var, int size, std::string type, int threadID) {
    ss->ss_input(id, var, size, type, inputStreams);
}

// one-dimensional float array I/O
void SMC_Utils::smc_input(int id, mpz_t **var, int size, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_input(id, var, size, type, inputStreams);
#else
    ss->ss_input(var, size, type);
#endif
}

void SMC_Utils::smc_input(int id, float *var, int size, std::string type, int threadID) {
    ss->ss_input(id, var, size, type, inputStreams);
}

void SMC_Utils::smc_output(int id, int *var, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, type, outputStreams);
#endif // otherwise, do nothing
}

void SMC_Utils::smc_output(int id, mpz_t *var, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, type, outputStreams);
#endif
}

void SMC_Utils::smc_output(int id, float *var, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, type, outputStreams);
#endif
}

void SMC_Utils::smc_output(int id, mpz_t **var, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, type, outputStreams);
#endif
}

void SMC_Utils::smc_output(int id, mpz_t *var, int size, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, size, type, outputStreams);
#endif
}

void SMC_Utils::smc_output(int id, int *var, int size, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, size, type, outputStreams);
#endif
}

void SMC_Utils::smc_output(int id, mpz_t **var, int size, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, type, outputStreams);
#endif
}

void SMC_Utils::smc_output(int id, float *var, int size, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, size, type, outputStreams);
#endif
}

/* SMC Addition */
void SMC_Utils::smc_add(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modAdd(result, a, b);
}

void SMC_Utils::smc_add(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modAdd(result, a, b);
}

void SMC_Utils::smc_add(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modAdd(result, b, a);
}

void SMC_Utils::smc_add(mpz_t *a, float b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    mpz_t *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    smc_add(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID);
    ss_batch_free_operator(&btmp, 4);
}

void SMC_Utils::smc_add(float a, mpz_t *b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    mpz_t *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    smc_add(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, type, threadID);
    ss_batch_free_operator(&atmp, 4);
}

void SMC_Utils::smc_add(mpz_t *a, mpz_t *b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    ss_single_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, "+", threadID, net, id, ss);
}

// batch version of smc_add
void SMC_Utils::smc_add(int *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    ss->modAdd(result, b, a, size);
}

void SMC_Utils::smc_add(mpz_t *a, int *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    ss->modAdd(result, a, b, size);
}

void SMC_Utils::smc_add(mpz_t *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    ss->modAdd(result, a, b, size);
}

void SMC_Utils::smc_add(mpz_t **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    mpz_t **btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    smc_add(a, btmp, alen_sig, alen_exp, alen_sig, alen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
    ss_batch_free_operator(&btmp, size);
}

void SMC_Utils::smc_add(float *a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    mpz_t **atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    smc_add(atmp, b, blen_sig, blen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
    ss_batch_free_operator(&atmp, size);
}

void SMC_Utils::smc_add(mpz_t **a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    ss_batch_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "+", threadID, net, id, ss);
}

void SMC_Utils::smc_set(mpz_t *a, mpz_t *result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    ss_set(a, result, alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID, net, id, ss);
}

void SMC_Utils::smc_set(mpz_t **a, mpz_t **result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    for (int i = 0; i < size; i++)
        ss_set(a[i], result[i], alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID, net, id, ss);
}

void SMC_Utils::smc_set(float a, mpz_t *result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    ss_set(a, result, alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID, net, id, ss);
}

void SMC_Utils::smc_set(mpz_t a, mpz_t result, int alen, int resultlen, std::string type, int threadID) {
    ss_set(a, result, alen, resultlen, type, threadID, net, id, ss);
}

void SMC_Utils::smc_set(mpz_t *a, mpz_t *result, int alen, int resultlen, int size, std::string type, int threadID) {
    for (int i = 0; i < size; i++)
        ss_set(a[i], result[i], alen, resultlen, type, threadID, net, id, ss);
}

// this routine should implement in a way that result = a + share[0]
void SMC_Utils::smc_set(int a, mpz_t result, int alen, int resultlen, std::string type, int threadID) {
    ss_set(a, result, alen, resultlen, type, threadID, net, id, ss);
}

void SMC_Utils::smc_priv_eval(mpz_t a, mpz_t b, mpz_t cond, int threadID) {
    ss_priv_eval(a, b, cond, threadID, net, id, ss);
}

// floating point
void SMC_Utils::smc_priv_eval(mpz_t *a, mpz_t *b, mpz_t cond, int threadID) {
    ss_priv_eval(a, b, cond, threadID, net, id, ss);
}

/* SMC Subtraction */
void SMC_Utils::smc_sub(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modSub(result, a, b);
}

void SMC_Utils::smc_sub(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modSub(result, a, b);
}

void SMC_Utils::smc_sub(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modSub(result, a, b);
}

void SMC_Utils::smc_sub(mpz_t *a, float b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    mpz_t *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    smc_sub(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID);
    ss_batch_free_operator(&btmp, 4);
}

void SMC_Utils::smc_sub(float a, mpz_t *b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    mpz_t *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    smc_sub(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, type, threadID);
    ss_batch_free_operator(&atmp, 4);
}

void SMC_Utils::smc_sub(mpz_t *a, mpz_t *b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    ss_sub(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, type, threadID, net, id, ss);
}

// batch operations of subtraction
void SMC_Utils::smc_sub(mpz_t *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    ss->modSub(result, a, b, size);
}

void SMC_Utils::smc_sub(int *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    ss->modSub(result, a, b, size);
}

void SMC_Utils::smc_sub(mpz_t *a, int *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    ss->modSub(result, a, b, size);
}

void SMC_Utils::smc_sub(mpz_t **a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    ss_sub(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID, net, id, ss);
}

void SMC_Utils::smc_sub(float *a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    ss_sub(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID, net, id, ss);
}

void SMC_Utils::smc_sub(mpz_t **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    ss_sub(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID, net, id, ss);
}

/* SMC Multiplication */
void SMC_Utils::smc_mult(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    // double check this works (passing reference to essentiall up-cast to mpz_t*)
    // cant use & for some reason, wont compile
    // replacing mpz_t* with & works on
    Mult(MPZ_CAST(result), MPZ_CAST(a), MPZ_CAST(b), 1, threadID, net, id, ss);
}

/******************************************************/
void SMC_Utils::smc_mult(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modMul(result, a, b);
}

void SMC_Utils::smc_mult(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modMul(result, b, a);
}

void SMC_Utils::smc_mult(mpz_t *a, mpz_t *b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    ss_single_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, "*", threadID, net, id, ss);
}

void SMC_Utils::smc_mult(float a, mpz_t *b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    mpz_t *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    smc_mult(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, type, threadID);
    ss_batch_free_operator(&atmp, 4);
}

void SMC_Utils::smc_mult(mpz_t *a, float b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    mpz_t *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    smc_mult(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID);
    ss_batch_free_operator(&btmp, 4);
}

// private float * private int (needs more examinations)
void SMC_Utils::smc_mult(mpz_t *a, mpz_t b, mpz_t *result, int alen_sig, int alen_exp, int blen, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    mpz_t b1, one;
    mpz_init(b1);
    mpz_init_set_ui(one, 1);
    // start computation
    ss->modSub(b1, one, b);
    smc_lor(a[2], b1, result[2], 1, 1, 1, type, threadID);
    // free the memory
    mpz_clear(b1);
    mpz_clear(one);
}

void SMC_Utils::smc_mult(int *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    mpz_t *atmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++) {
        mpz_init_set_si(atmp[i], a[i]);
        ss->modMul(result[i], atmp[i], b[i]);
    }
    ss_batch_free_operator(&atmp, size);
}

void SMC_Utils::smc_mult(mpz_t *a, int *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    mpz_t *btmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++) {
        mpz_init_set_si(btmp[i], b[i]);
        ss->modMul(result[i], a[i], btmp[i]);
    }
    smc_mult(a, btmp, alen, blen, result, resultlen, size, type, threadID);
    ss_batch_free_operator(&btmp, size);
}

void SMC_Utils::smc_mult(mpz_t *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    Mult(result, a, b, size, threadID, net, id, ss);
}

void SMC_Utils::smc_mult(mpz_t **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    mpz_t **btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    smc_mult(a, btmp, alen_sig, alen_exp, alen_sig, alen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
    ss_batch_free_operator(&btmp, size);
}

void SMC_Utils::smc_mult(float *a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    mpz_t **atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    smc_mult(atmp, b, blen_sig, blen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
    ss_batch_free_operator(&atmp, size);
}

void SMC_Utils::smc_mult(mpz_t **a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    ss_batch_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "*", threadID, net, id, ss);
}

/* SMC Integer Division*/
void SMC_Utils::smc_div(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_IntDiv(MPZ_CAST(result), MPZ_CAST(a), MPZ_CAST(b), resultlen, 1, threadID, net, id, ss);
    // doOperation_IntDiv(result, a, b, resultlen, threadID, net, id, ss);
}

void SMC_Utils::smc_div(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_IntDiv_Pub(MPZ_CAST(result), MPZ_CAST(a), (int *)&b, resultlen, 1, threadID, net, id, ss);
}

void SMC_Utils::smc_div(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_IntDiv(MPZ_CAST(result), (int *)&a, MPZ_CAST(b), resultlen, 1, threadID, net, id, ss);
}

void SMC_Utils::smc_div(mpz_t *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_IntDiv(result, a, b, resultlen, size, threadID, net, id, ss);
}

void SMC_Utils::smc_div(int *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_IntDiv(result, a, b, resultlen, size, threadID, net, id, ss);
}

void SMC_Utils::smc_div(mpz_t *a, int *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_IntDiv_Pub(result, a, b, resultlen, size, threadID, net, id, ss);
}

/* SMC Floating-point division */
void SMC_Utils::smc_div(mpz_t *a, mpz_t *b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    ss_single_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, "/", threadID, net, id, ss);
}

void SMC_Utils::smc_div(mpz_t *a, float b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    mpz_t **results = (mpz_t **)malloc(sizeof(mpz_t *));
    mpz_t **as = (mpz_t **)malloc(sizeof(mpz_t *));
    mpz_t **bs;

    as[0] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    results[0] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    ss_batch_convert_to_private_float(&b, &bs, alen_sig, alen_exp, 1, ss);

    for (int i = 0; i < 4; i++) {
        mpz_init(results[0][i]);
        mpz_init_set(as[0][i], a[i]);
    }

    doOperation_FLDiv_Pub(as, bs, results, alen_sig, 1, threadID, net, id, ss);
    ss_process_results(results, resultlen_sig, resultlen_exp, alen_sig, alen_exp, 1, threadID, net, id, ss);

    for (int i = 0; i < 4; i++)
        mpz_set(result[i], results[0][i]);

    // free the memory
    ss_batch_free_operator(&as, 1);
    ss_batch_free_operator(&bs, 1);
    ss_batch_free_operator(&results, 1);
}

void SMC_Utils::smc_div(float a, mpz_t *b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    mpz_t *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    smc_div(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, type, threadID);
    ss_batch_free_operator(&atmp, 4);
}

void SMC_Utils::smc_div(mpz_t **a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    ss_batch_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "/", threadID, net, id, ss);
}

void SMC_Utils::smc_div(mpz_t **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    mpz_t **btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    doOperation_FLDiv_Pub(a, btmp, result, alen_sig, size, threadID, net, id, ss);
    ss_process_results(result, resultlen_sig, resultlen_exp, alen_sig, alen_exp, size, threadID, net, id, ss);
    ss_batch_free_operator(&btmp, size);
}

void SMC_Utils::smc_div(float *a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    mpz_t **atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    smc_div(atmp, b, blen_sig, blen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
    ss_batch_free_operator(&atmp, size);
}

/* All Comparisons */
void SMC_Utils::smc_lt(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(a), MPZ_CAST(b), alen, blen, resultlen, 1, threadID, net, id, ss);
}

void SMC_Utils::smc_lt(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(a), (int *)&b, alen, blen, resultlen, 1, threadID, net, id, ss);
}

void SMC_Utils::smc_lt(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), (int *)&a, MPZ_CAST(b), alen, blen, resultlen, 1, threadID, net, id, ss);
}

void SMC_Utils::smc_lt(mpz_t *a, mpz_t *b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    ss_single_fop_comparison(result, a, b, resultlen, alen_sig, alen_exp, blen_sig, blen_exp, "<0", threadID, net, id, ss);
}

void SMC_Utils::smc_lt(mpz_t *a, float b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    mpz_t *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    ss_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, alen_sig, alen_exp, "<0", threadID, net, id, ss);
    ss_batch_free_operator(&btmp, 4);
}

void SMC_Utils::smc_lt(float a, mpz_t *b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    mpz_t *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    ss_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "<0", threadID, net, id, ss);
    ss_batch_free_operator(&atmp, 4);
}

// batch operations of comparisons
void SMC_Utils::smc_lt(mpz_t *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, a, b, alen, blen, resultlen, size, threadID, net, id, ss);
}

void SMC_Utils::smc_lt(mpz_t **a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "<0", threadID, net, id, ss);
}

void SMC_Utils::smc_gt(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(b), MPZ_CAST(a), blen, alen, resultlen, 1, threadID, net, id, ss);
}

void SMC_Utils::smc_gt(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), (int *)&b, MPZ_CAST(a), blen, alen, resultlen, 1, threadID, net, id, ss);
}

void SMC_Utils::smc_gt(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(b), (int *)&a, blen, alen, resultlen, 1, threadID, net, id, ss);
}

void SMC_Utils::smc_gt(mpz_t *a, mpz_t *b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    ss_single_fop_comparison(result, b, a, resultlen, blen_sig, blen_exp, alen_sig, alen_exp, "<0", threadID, net, id, ss);
}

void SMC_Utils::smc_gt(float a, mpz_t *b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    mpz_t *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    ss_single_fop_comparison(result, b, atmp, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "<0", threadID, net, id, ss);
    ss_batch_free_operator(&atmp, 4);
}

void SMC_Utils::smc_gt(mpz_t *a, float b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    mpz_t *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    ss_single_fop_comparison(result, btmp, a, resultlen, alen_sig, alen_exp, alen_sig, alen_exp, "<0", threadID, net, id, ss);
    ss_batch_free_operator(&btmp, 4);
}

// batch operations of gt
void SMC_Utils::smc_gt(mpz_t *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, b, a, blen, alen, resultlen, size, threadID, net, id, ss);
}

void SMC_Utils::smc_gt(mpz_t **a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    ss_batch_fop_comparison(result, b, a, resultlen, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID, net, id, ss);
}

void SMC_Utils::smc_leq(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(b), MPZ_CAST(a), blen, alen, resultlen, 1, threadID, net, id, ss);
    ss->modSub(result, 1, result);
}

void SMC_Utils::smc_leq(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), (int *)&b, MPZ_CAST(a), blen, alen, resultlen, 1, threadID, net, id, ss);
    ss->modSub(result, 1, result);
}

void SMC_Utils::smc_leq(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(b), (int *)&a, blen, alen, resultlen, 1, threadID, net, id, ss);
    ss->modSub(result, 1, result);
}

void SMC_Utils::smc_leq(mpz_t *a, mpz_t *b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    ss_single_fop_comparison(result, b, a, resultlen, blen_sig, blen_exp, alen_sig, alen_exp, "<0", threadID, net, id, ss);
    ss->modSub(result, 1, result);
}

void SMC_Utils::smc_leq(float a, mpz_t *b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    mpz_t *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    ss_single_fop_comparison(result, b, atmp, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "<0", threadID, net, id, ss);
    ss->modSub(result, 1, result);
    ss_batch_free_operator(&atmp, 4);
}

void SMC_Utils::smc_leq(mpz_t *a, float b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    mpz_t *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    ss_single_fop_comparison(result, btmp, a, resultlen, alen_sig, alen_exp, alen_sig, alen_exp, "<0", threadID, net, id, ss);
    ss->modSub(result, 1, result);
    ss_batch_free_operator(&btmp, 4);
}

// batch operations of leq
void SMC_Utils::smc_leq(mpz_t *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, b, a, blen, alen, resultlen, size, threadID, net, id, ss);
    ss->modSub(result, 1, result, size);
}

void SMC_Utils::smc_leq(mpz_t **a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    ss_batch_fop_comparison(result, b, a, resultlen, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID, net, id, ss);
    ss->modSub(result, 1, result, size);
}

void SMC_Utils::smc_geq(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(a), MPZ_CAST(b), alen, blen, resultlen, 1, threadID, net, id, ss);
    ss->modSub(result, 1, result);
}

void SMC_Utils::smc_geq(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(a), (int *)&b, alen, blen, resultlen, 1, threadID, net, id, ss);
    ss->modSub(result, 1, result);
}

void SMC_Utils::smc_geq(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), (int *)&a, MPZ_CAST(b), alen, blen, resultlen, 1, threadID, net, id, ss);
    ss->modSub(result, 1, result);
}

void SMC_Utils::smc_geq(mpz_t *a, mpz_t *b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    ss_single_fop_comparison(result, a, b, resultlen, alen_sig, alen_exp, blen_sig, blen_exp, "<0", threadID, net, id, ss);
    ss->modSub(result, 1, result);
}

void SMC_Utils::smc_geq(float a, mpz_t *b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    mpz_t *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    ss_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "<0", threadID, net, id, ss);
    ss->modSub(result, 1, result);
    ss_batch_free_operator(&atmp, 4);
}

void SMC_Utils::smc_geq(mpz_t *a, float b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    mpz_t *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    ss_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, blen_sig, blen_exp, "<0", threadID, net, id, ss);
    ss->modSub(result, 1, result);
    ss_batch_free_operator(&btmp, 4);
}

// batch operations of geq
void SMC_Utils::smc_geq(mpz_t *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, a, b, alen, blen, resultlen, size, threadID, net, id, ss);
    ss->modSub(result, 1, result, size);
}

void SMC_Utils::smc_geq(mpz_t **a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "<0", threadID, net, id, ss);
    ss->modSub(result, 1, result, size);
}

// Equality and Inequality
void SMC_Utils::smc_eqeq(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_EQZ(MPZ_CAST(result), MPZ_CAST(a), MPZ_CAST(b), alen, blen, resultlen, 1, threadID, net, id, ss);
}

void SMC_Utils::smc_eqeq(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_EQZ(MPZ_CAST(result), MPZ_CAST(a), (int *)&b, alen, blen, resultlen, 1, threadID, net, id, ss);
}

void SMC_Utils::smc_eqeq(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_EQZ(MPZ_CAST(result), MPZ_CAST(b), (int *)&a, blen, alen, resultlen, 1, threadID, net, id, ss);
}

void SMC_Utils::smc_eqeq(mpz_t *a, mpz_t *b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    ss_single_fop_comparison(result, a, b, resultlen, alen_sig, alen_exp, blen_sig, blen_exp, "==", threadID, net, id, ss);
}

void SMC_Utils::smc_eqeq(mpz_t *a, float b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    mpz_t *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    ss_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, alen_sig, alen_exp, "==", threadID, net, id, ss);
    ss_batch_free_operator(&btmp, 4);
}

void SMC_Utils::smc_eqeq(float a, mpz_t *b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    mpz_t *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    ss_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "==", threadID, net, id, ss);
    ss_batch_free_operator(&atmp, 4);
}

// batch operations of eqeq
void SMC_Utils::smc_eqeq(mpz_t *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_EQZ(result, a, b, alen, blen, resultlen, size, threadID, net, id, ss);
}

void SMC_Utils::smc_eqeq(mpz_t **a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "==", threadID, net, id, ss);
}

void SMC_Utils::smc_neq(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_EQZ(MPZ_CAST(result), MPZ_CAST(a), MPZ_CAST(b), alen, blen, resultlen, 1, threadID, net, id, ss);
    ss->modSub(result, 1, result);
}

void SMC_Utils::smc_neq(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_EQZ(MPZ_CAST(result), MPZ_CAST(a), (int *)&b, alen, blen, resultlen, 1, threadID, net, id, ss);
    ss->modSub(result, 1, result);
}

void SMC_Utils::smc_neq(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_EQZ(MPZ_CAST(result), MPZ_CAST(b), (int *)&a, blen, alen, resultlen, 1, threadID, net, id, ss);
    ss->modSub(result, 1, result);
}

void SMC_Utils::smc_neq(mpz_t *a, mpz_t *b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    ss_single_fop_comparison(result, a, b, resultlen, alen_sig, alen_exp, blen_sig, blen_exp, "==", threadID, net, id, ss);
    ss->modSub(result, 1, result);
}

void SMC_Utils::smc_neq(float a, mpz_t *b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    mpz_t *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    ss_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "==", threadID, net, id, ss);
    ss->modSub(result, 1, result);
    ss_batch_free_operator(&atmp, 4);
}

void SMC_Utils::smc_neq(mpz_t *a, float b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    mpz_t *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    ss_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, alen_sig, alen_exp, "==", threadID, net, id, ss);
    ss->modSub(result, 1, result);
    ss_batch_free_operator(&btmp, 4);
}

// batch operations of neq
void SMC_Utils::smc_neq(mpz_t *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_EQZ(result, a, b, alen, blen, resultlen, size, threadID, net, id, ss);
    ss->modSub(result, 1, result, size);
}

void SMC_Utils::smc_neq(mpz_t **a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "==", threadID, net, id, ss);
    ss->modSub(result, 1, result, size);
}

// Bitwise Operations
void SMC_Utils::smc_land(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    BitAnd(MPZ_CAST(a), MPZ_CAST(b), MPZ_CAST(result), 1, threadID, net, id, ss);
}

void SMC_Utils::smc_land(mpz_t *a, mpz_t *b, int size, mpz_t *result, std::string type, int threadID) {
    BitAnd(a, b, result, size, threadID, net, id, ss);
}

void SMC_Utils::smc_xor(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    BitXor(MPZ_CAST(a), MPZ_CAST(b), MPZ_CAST(result), 1, threadID, net, id, ss);
}

void SMC_Utils::smc_xor(mpz_t *a, mpz_t *b, int size, mpz_t *result, std::string type, int threadID) {
    BitXor(a, b, result, size, threadID, net, id, ss);
}

void SMC_Utils::smc_lor(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    BitOr(MPZ_CAST(a), MPZ_CAST(b), MPZ_CAST(result), 1, threadID, net, id, ss);
}

void SMC_Utils::smc_lor(mpz_t *a, mpz_t *b, int size, mpz_t *result, std::string type, int threadID) {
    BitOr(a, b, result, size, threadID, net, id, ss);
}

void SMC_Utils::smc_shr(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    smc_shr(MPZ_CAST(a), MPZ_CAST(b), alen, blen, MPZ_CAST(result), resultlen, 1, type, threadID);
}

void SMC_Utils::smc_shr(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    smc_shr(MPZ_CAST(a), (int *)&b, alen, blen, MPZ_CAST(result), resultlen, 1, type, threadID);
}

void SMC_Utils::smc_shr(mpz_t *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    if (blen == -1) {
        // doOperation_Trunc(result, a, alen, b[0], size, threadID, net, id, ss);

        int *b_tmp = (int *)malloc(sizeof(int) * size);
        for (int i = 0; i < size; i++)
            b_tmp[i] = mpz_get_si(b[i]);
        doOperation_Trunc(result, a, alen, b_tmp, size, threadID, net, id, ss);
        // smc_shr(a, b_tmp, alen, blen, result, resultlen, size, type, threadID);
        free(b_tmp);
    } else
        doOperation_TruncS(result, a, alen, b, size, threadID, net, id, ss);
}

void SMC_Utils::smc_shr(mpz_t *a, int *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_Trunc(result, a, alen, b, size, threadID, net, id, ss);
}

void SMC_Utils::smc_shl(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    if (blen == -1) {

        ss->modPow2(result, b);
        ss->modMul(result, a, result);
    } else {
        doOperation_Pow2(MPZ_CAST(result), MPZ_CAST(b), blen, 1, threadID, net, id, ss);
        Mult(MPZ_CAST(result), MPZ_CAST(result), MPZ_CAST(a), 1, threadID, net, id, ss);
    }
}

void SMC_Utils::smc_shl(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modPow2(result, b);
    ss->modMul(result, a, result);
}

void SMC_Utils::smc_shl(mpz_t *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    if (blen == -1) {
        // b is PUBLIC, but stored in type mpz_t
        ss->modPow2(result, b, size);
        ss->modMul(result, a, result, size);
    } else {
        doOperation_Pow2(result, b, blen, size, threadID, net, id, ss);
        Mult(result, result, a, size, threadID, net, id, ss);
    }
}

void SMC_Utils::smc_shl(mpz_t *a, int *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    ss->modPow2(result, b, size);
    ss->modMul(result, a, result, size);
}

// Dot Product
void SMC_Utils::smc_dot(mpz_t *a, mpz_t *b, int size, mpz_t result, int threadID) {
    doOperation_DotProduct(a, b, result, size, threadID, net, id, ss);
}

void SMC_Utils::smc_dot(mpz_t **a, mpz_t **b, int size, int array_size, mpz_t *result, std::string type, int threadID) {
    doOperation_DotProduct(a, b, result, size, array_size, threadID, net, id, ss);
}

// one-dimension private integer singular read
void SMC_Utils::smc_privindex_read(mpz_t index, mpz_t *array, mpz_t result, int dim, std::string type, int threadID) {
    doOperation_PrivIndex_int(index, array, result, dim, 0, threadID, net, id, ss);
}

// two-dimension private integer singular read
void SMC_Utils::smc_privindex_read(mpz_t index, mpz_t **array, mpz_t result, int dim1, int dim2, std::string type, int threadID) {
    doOperation_PrivIndex_int_arr(index, array, result, dim1, dim2, 0, threadID, net, id, ss);
}

// one-dimension private float singular read
void SMC_Utils::smc_privindex_read(mpz_t index, mpz_t **array, mpz_t *result, int dim, std::string type, int threadID) {
    doOperation_PrivIndex_float(index, array, result, dim, 1, threadID, net, id, ss);
}

// two-dimension private float singular read
void SMC_Utils::smc_privindex_read(mpz_t index, mpz_t ***array, mpz_t *result, int dim1, int dim2, std::string type, int threadID) {
    doOperation_PrivIndex_float_arr(index, array, result, dim1, dim2, 0, threadID, net, id, ss);
}

// one-dimension private integer singular write
void SMC_Utils::smc_privindex_write(mpz_t index, mpz_t *array, int len_sig, int len_exp, int value, int dim, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID) {
    doOperation_PrivIndex_Write((mpz_t *)index, array, &value, dim, 1, out_cond, priv_cond, counter, threadID, 0, net, id, ss);
}

void SMC_Utils::smc_privindex_write(mpz_t index, mpz_t *array, int len_sig, int len_exp, mpz_t value, int dim, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID) {

    doOperation_PrivIndex_Write((mpz_t *)index, array, (mpz_t *)value, dim, 1, out_cond, priv_cond, counter, threadID, 0, net, id, ss);
}

// two-dimension private integer singular write
void SMC_Utils::smc_privindex_write(mpz_t index, mpz_t **array, int len_sig, int len_exp, int value, int dim1, int dim2, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID) {

    doOperation_PrivIndex_Write_2d(MPZ_CAST(index), array, (int *)&value, dim1, dim2, 1, out_cond, priv_cond, counter, threadID, 0, net, id, ss);
}

void SMC_Utils::smc_privindex_write(mpz_t index, mpz_t **array, int len_sig, int len_exp, mpz_t value, int dim1, int dim2, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID) {
    doOperation_PrivIndex_Write_2d(MPZ_CAST(index), array, MPZ_CAST(value), dim1, dim2, 1, out_cond, priv_cond, counter, threadID, 0, net, id, ss);
}

// one-dimension private float singular write
void SMC_Utils::smc_privindex_write(mpz_t index, mpz_t **array, int len_sig, int len_exp, float value, int dim, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(index, array, len_sig, len_exp, value, dim, out_cond, priv_cond, counter, type, threadID, net, id, ss);
}

void SMC_Utils::smc_privindex_write(mpz_t index, mpz_t **array, int len_sig, int len_exp, mpz_t *value, int dim, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(index, array, len_sig, len_exp, value, dim, out_cond, priv_cond, counter, type, threadID, net, id, ss);
}

// two-dimension private float singular write
void SMC_Utils::smc_privindex_write(mpz_t index, mpz_t ***array, int len_sig, int len_exp, float value, int dim1, int dim2, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(index, array, len_sig, len_exp, value, dim1, dim2, out_cond, priv_cond, counter, type, threadID, net, id, ss);
}

void SMC_Utils::smc_privindex_write(mpz_t index, mpz_t ***array, int len_sig, int len_exp, mpz_t *value, int dim1, int dim2, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(index, array, len_sig, len_exp, value, dim1, dim2, out_cond, priv_cond, counter, type, threadID, net, id, ss);
}

// one-dimension private integer batch read
void SMC_Utils::smc_privindex_read(mpz_t *indices, mpz_t *arrays, mpz_t *results, int dim, int size, std::string type, int threadID) {
    doOperation_PrivIndex_Read(indices, arrays, results, dim, size, threadID, 0, net, id, ss);
}

// two-dimension private integer batch read
void SMC_Utils::smc_privindex_read(mpz_t *indices, mpz_t **array, mpz_t *results, int dim1, int dim2, int size, std::string type, int threadID) {
    ss_privindex_read(indices, array, results, dim1, dim2, size, type, threadID, net, id, ss);
}

// one-dimension private float batch read
void SMC_Utils::smc_privindex_read(mpz_t *indices, mpz_t **array, mpz_t **results, int dim, int size, std::string type, int threadID) {
    ss_privindex_read(indices, array, results, dim, size, type, threadID, net, id, ss);
}

// two-dimension private float batch read
void SMC_Utils::smc_privindex_read(mpz_t *indices, mpz_t ***array, mpz_t **results, int dim1, int dim2, int size, std::string type, int threadID) {
    ss_privindex_read(indices, array, results, dim1, dim2, size, type, threadID, net, id, ss);
}

// one-dimension private integer batch write
void SMC_Utils::smc_privindex_write(mpz_t *indices, mpz_t *array, int len_sig, int len_exp, int *values, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID) {
    doOperation_PrivIndex_Write(indices, array, values, dim, size, out_cond, priv_cond, counter, threadID, 0, net, id, ss);
}

void SMC_Utils::smc_privindex_write(mpz_t *indices, mpz_t *array, int len_sig, int len_exp, mpz_t *values, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID) {
    doOperation_PrivIndex_Write(indices, array, values, dim, size, out_cond, priv_cond, counter, threadID, 0, net, id, ss);
}

void SMC_Utils::smc_privindex_write(mpz_t *indices, mpz_t **array, int len_sig, int len_exp, int *values, int dim1, int dim2, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(indices, array, len_sig, len_exp, values, dim1, dim2, size, out_cond, priv_cond, counter, type, threadID, net, id, ss);
}

// two-dimension private integer batch write
void SMC_Utils::smc_privindex_write(mpz_t *indices, mpz_t **array, int len_sig, int len_exp, mpz_t *values, int dim1, int dim2, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(indices, array, len_sig, len_exp, values, dim1, dim2, size, out_cond, priv_cond, counter, type, threadID, net, id, ss);
}

// one-dimension private float batch write
void SMC_Utils::smc_privindex_write(mpz_t *indices, mpz_t **array, int len_sig, int len_exp, float *values, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(indices, array, len_sig, len_exp, values, dim, size, out_cond, priv_cond, counter, type, threadID, net, id, ss);
}
void SMC_Utils::smc_privindex_write(mpz_t *indices, mpz_t **array, int len_sig, int len_exp, mpz_t **values, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(indices, array, len_sig, len_exp, values, dim, size, out_cond, priv_cond, counter, type, threadID, net, id, ss);
}

// two-dimension private float batch write
void SMC_Utils::smc_privindex_write(mpz_t *indices, mpz_t ***array, int len_sig, int len_exp, float *values, int dim1, int dim2, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(indices, array, len_sig, len_exp, values, dim1, dim2, size, out_cond, priv_cond, counter, type, threadID, net, id, ss);
}
void SMC_Utils::smc_privindex_write(mpz_t *indices, mpz_t ***array, int len_sig, int len_exp, mpz_t **values, int dim1, int dim2, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(indices, array, len_sig, len_exp, values, dim1, dim2, size, out_cond, priv_cond, counter, type, threadID, net, id, ss);
    ;
}

priv_ptr SMC_Utils::smc_new_ptr(int level, int type) {
    return create_ptr(level, type);
}

priv_ptr *SMC_Utils::smc_new_ptr(int level, int type, int num) {
    return create_ptr(level, type, num);
}

/*************/
void SMC_Utils::smc_set_int_ptr(priv_ptr ptr, mpz_t *var_loc, std::string type, int threadID) {
    set_ptr(ptr, var_loc, NULL, NULL, NULL, threadID);
}

void SMC_Utils::smc_set_float_ptr(priv_ptr ptr, mpz_t **var_loc, std::string type, int threadID) {
    set_ptr(ptr, NULL, var_loc, NULL, NULL, threadID);
}

void SMC_Utils::smc_set_struct_ptr(priv_ptr ptr, void *var_loc, std::string type, int threadID) {
    set_ptr(ptr, NULL, NULL, var_loc, NULL, threadID);
}

void SMC_Utils::smc_set_ptr(priv_ptr assign_ptr, priv_ptr *ptr_loc, std::string type, int threadID) {
    set_ptr(assign_ptr, NULL, NULL, NULL, ptr_loc, threadID);
}

void SMC_Utils::smc_set_ptr(priv_ptr assign_ptr, priv_ptr right_ptr, std::string type, int threadID) {
    set_ptr(assign_ptr, right_ptr, threadID);
}

void SMC_Utils::smc_set_ptr(priv_ptr ptr, int var_loc, std::string type, int threadID) {
}
/****************/
void SMC_Utils::smc_update_int_ptr(priv_ptr ptr, mpz_t *var_loc, mpz_t private_tag, int index, int threadID) {
    update_ptr(ptr, var_loc, NULL, NULL, NULL, private_tag, index, threadID, net, id, ss);
}

void SMC_Utils::smc_update_float_ptr(priv_ptr ptr, mpz_t **var_loc, mpz_t private_tag, int index, int threadID) {
    update_ptr(ptr, NULL, var_loc, NULL, NULL, private_tag, index, threadID, net, id, ss);
}

void SMC_Utils::smc_update_struct_ptr(priv_ptr ptr, void *var_loc, mpz_t private_tag, int index, int threadID) {
    update_ptr(ptr, NULL, NULL, var_loc, NULL, private_tag, index, threadID, net, id, ss);
}

void SMC_Utils::smc_update_ptr(priv_ptr ptr, priv_ptr *ptr_loc, mpz_t private_tag, int index, int threadID) {
    update_ptr(ptr, NULL, NULL, NULL, ptr_loc, private_tag, index, threadID, net, id, ss);
}

void SMC_Utils::smc_update_ptr(priv_ptr assign_ptr, priv_ptr right_ptr, mpz_t private_tag, int index, int threadID) {
    update_ptr(assign_ptr, right_ptr, private_tag, index, threadID, net, id, ss);
}

void SMC_Utils::smc_add_int_ptr(priv_ptr ptr, mpz_t *var_loc, mpz_t private_tag, int threadID) {
    add_ptr(ptr, var_loc, NULL, NULL, NULL, private_tag, threadID);
}

void SMC_Utils::smc_add_float_ptr(priv_ptr ptr, mpz_t **var_loc, mpz_t private_tag, int threadID) {
    add_ptr(ptr, NULL, var_loc, NULL, NULL, private_tag, threadID);
}

void SMC_Utils::smc_add_struct_ptr(priv_ptr ptr, void *var_loc, mpz_t private_tag, int threadID) {
    add_ptr(ptr, NULL, NULL, var_loc, NULL, private_tag, threadID);
}

void SMC_Utils::smc_add_ptr(priv_ptr ptr, priv_ptr *ptr_loc, mpz_t private_tag, int threadID) {
    add_ptr(ptr, NULL, NULL, NULL, ptr_loc, private_tag, threadID);
}

/*******************/
void SMC_Utils::smc_shrink_ptr(priv_ptr ptr, int current_index, int parent_index, int threadID) {
    shrink_ptr(ptr, current_index, parent_index, threadID);
}
/********************/
void SMC_Utils::smc_dereference_read_ptr(priv_ptr ptr, mpz_t result, int num_of_dereferences, mpz_t priv_cond, std::string type, int threadID) {
    dereference_ptr_read_var(ptr, result, num_of_dereferences, threadID, net, id, ss);
}

void SMC_Utils::smc_dereference_read_ptr(priv_ptr ptr, mpz_t *result, int num_of_dereferences, mpz_t priv_cond, std::string type, int threadID) {
    dereference_ptr_read_var(ptr, result, num_of_dereferences, threadID, net, id, ss);
}

void SMC_Utils::smc_dereference_read_ptr(priv_ptr ptr, priv_ptr result, int num_of_dereferences, mpz_t priv_cond, std::string type, int threadID) {
    dereference_ptr_read_ptr(ptr, result, num_of_dereferences, priv_cond, threadID, net, id, ss);
}

void SMC_Utils::smc_dereference_write_ptr(priv_ptr ptr, mpz_t value, int num_of_dereferences, mpz_t private_cond, std::string type, int threadID) {
    dereference_ptr_write_var(ptr, value, num_of_dereferences, private_cond, threadID, net, id, ss);
}

void SMC_Utils::smc_dereference_write_int_ptr(priv_ptr ptr, mpz_t *value, int num_of_dereferences, mpz_t private_cond, std::string type, int threadID) {
    dereference_ptr_write(ptr, value, NULL, NULL, NULL, num_of_dereferences, private_cond, threadID, net, id, ss);
}

void SMC_Utils::smc_dereference_write_ptr(priv_ptr ptr, mpz_t *value, int num_of_dereferences, mpz_t private_cond, std::string type, int threadID) {
    dereference_ptr_write_var(ptr, value, num_of_dereferences, private_cond, threadID, net, id, ss);
}

void SMC_Utils::smc_dereference_write_float_ptr(priv_ptr ptr, mpz_t **value, int num_of_dereferences, mpz_t private_cond, std::string type, int threadID) {
    dereference_ptr_write(ptr, NULL, value, NULL, NULL, num_of_dereferences, private_cond, threadID, net, id, ss);
}

void SMC_Utils::smc_dereference_write_struct_ptr(priv_ptr ptr, void *value, int num_of_dereferences, mpz_t private_cond, std::string type, int threadID) {
    dereference_ptr_write(ptr, NULL, NULL, value, NULL, num_of_dereferences, private_cond, threadID, net, id, ss);
}

void SMC_Utils::smc_dereference_write_ptr(priv_ptr ptr, priv_ptr *value, int num_of_dereferences, mpz_t private_cond, std::string type, int threadID) {
    dereference_ptr_write(ptr, NULL, NULL, NULL, value, num_of_dereferences, private_cond, threadID, net, id, ss);
}

void SMC_Utils::smc_dereference_write_ptr(priv_ptr ptr, priv_ptr value, int num_of_dereferences, mpz_t private_cond, std::string type, int threadID) {
    dereference_ptr_write_ptr(ptr, value, num_of_dereferences, private_cond, threadID, net, id, ss);
}
/*******************************************/
void SMC_Utils::smc_clear_ptr(priv_ptr *ptr) {
    clear_ptr(ptr);
}
void SMC_Utils::smc_free_ptr(priv_ptr *ptr) {
    destroy_ptr(ptr);
}

void SMC_Utils::smc_free_ptr(priv_ptr **ptrs, int num) {
    destroy_ptr(ptrs, num);
}

void SMC_Utils::smc_int2fl(int value, mpz_t *result, int gamma, int K, int L, int threadID) {
    ss_int2fl(value, result, gamma, K, L, threadID, net, id, ss);
}

void SMC_Utils::smc_int2fl(mpz_t value, mpz_t *result, int gamma, int K, int L, int threadID) {
    ss_int2fl(value, result, gamma, K, L, threadID, net, id, ss);
}

void SMC_Utils::smc_int2int(int value, mpz_t result, int gamma1, int gamma2, int threadID) {
    ss_int2int(value, result, gamma1, gamma2, threadID, net, id, ss);
}

void SMC_Utils::smc_int2int(mpz_t value, mpz_t result, int gamma1, int gamma2, int threadID) {
    ss_int2int(value, result, gamma1, gamma2, threadID, net, id, ss);
}

void SMC_Utils::smc_fl2int(float value, mpz_t result, int K, int L, int gamma, int threadID) {
    ss_fl2int(value, result, K, L, gamma, threadID, net, id, ss);
}

void SMC_Utils::smc_fl2int(mpz_t *value, mpz_t result, int K, int L, int gamma, int threadID) {
    ss_fl2int(value, result, K, L, gamma, threadID, net, id, ss);
}

void SMC_Utils::smc_fl2fl(float value, mpz_t *result, int K1, int L1, int K2, int L2, int threadID) {
    ss_fl2fl(value, result, K1, L1, K2, L2, threadID, net, id, ss);
}

void SMC_Utils::smc_fl2fl(mpz_t *value, mpz_t *result, int K1, int L1, int K2, int L2, int threadID) {
    ss_fl2fl(value, result, K1, L1, K2, L2, threadID, net, id, ss);
}

/************************************ INTEGER BATCH ****************************************/
void SMC_Utils::smc_batch(mpz_t *a, mpz_t *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// used to compute 1-priv_cond in a batch stmt
void SMC_Utils::smc_batch(int a, mpz_t *b, mpz_t *result, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, int threadID) {
    ss_batch(a, b, result, out_cond, priv_cond, counter, index_array, size, op, threadID, net, id, ss);
}

void SMC_Utils::smc_batch(mpz_t *a, mpz_t *b, mpz_t *result, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, int threadID) {
    ss_batch(a, b, result, out_cond, priv_cond, counter, index_array, size, op, threadID, net, id, ss);
}

// first param: int array
// second param: int array
// third param: one-dim private int array
void SMC_Utils::smc_batch(int *a, int *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// first param: int array
// second param: int array
// third param: two-dim private int array
void SMC_Utils::smc_batch(int *a, int *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// first param: int array
// second param: one-dim private int array
// third param: one-dim private int array
void SMC_Utils::smc_batch(int *a, mpz_t *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// first param: int array
// second param: one-dim private int array
// third param: two-dim private int array
void SMC_Utils::smc_batch(int *a, mpz_t *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// first param: one-dim private int array
// second param: int array
// third param: one-dim private int array
void SMC_Utils::smc_batch(mpz_t *a, int *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// first param: one-dim private int array
// second param: int array
// third param: two-dim private int array
void SMC_Utils::smc_batch(mpz_t *a, int *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// first param: integer array
// second param: two-dim private int
// assignment param: one-dim private int
void SMC_Utils::smc_batch(int *a, mpz_t **b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// first param: integer array
// second param: two-dim private int
// assignment param: two-dim private int
void SMC_Utils::smc_batch(int *a, mpz_t **b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// first param: two-dim private int
// second param: integer array
// assignment param: one-dim private int
void SMC_Utils::smc_batch(mpz_t **a, int *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// first param: two-dim private int
// second param: integer array
// assignment param: two-dim private int
void SMC_Utils::smc_batch(mpz_t **a, int *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// first param: one-dim private int
// second param: two-dim private int
// assignment param: two-dim private int
void SMC_Utils::smc_batch(mpz_t *a, mpz_t **b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// first param: two-dim private int
// second param: one-dim private int
// assignment param: two-dim private int
void SMC_Utils::smc_batch(mpz_t **a, mpz_t *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// first param: two-dim private int
// second param: two-dim private int
// assignment param: one-dim private int
void SMC_Utils::smc_batch(mpz_t **a, mpz_t **b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// first param: one-dim private int
// second param: one-dim private int
// assignment param: two-dim private int
void SMC_Utils::smc_batch(mpz_t *a, mpz_t *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}
// first param: one-dim private int
// second param: two-dim private int
// assignment param: one-dim private int
void SMC_Utils::smc_batch(mpz_t *a, mpz_t **b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// first param: two-dim private int
// second param: one-dim private int
// assignment param: one-dim private int
void SMC_Utils::smc_batch(mpz_t **a, mpz_t *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

void SMC_Utils::smc_batch_dot(mpz_t **a, mpz_t **b, int size, int array_size, int *index_array, mpz_t *result, int threadID) {
    ss_batch_dot(a, b, size, array_size, index_array, result, threadID, net, id, ss);
}

// first param: two-dim private int
// second param: two-dim private int
// assignment param: two-dim private int

void SMC_Utils::smc_batch(mpz_t **a, mpz_t **b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

/*********************************************** FLOAT BATCH ****************************************************/
// public + private one-dimension float - arithmetic
void SMC_Utils::smc_batch(float *a, mpz_t **b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

void SMC_Utils::smc_batch(mpz_t **a, float *b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

void SMC_Utils::smc_batch(float *a, mpz_t **b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

void SMC_Utils::smc_batch(mpz_t **a, float *b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// public to private assignments
void SMC_Utils::smc_batch(float *a, float *b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

void SMC_Utils::smc_batch(int *a, int *b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

void SMC_Utils::smc_batch(float *a, float *b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

void SMC_Utils::smc_batch(int *a, int *b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// public + private two-dimension float - arithmetic
void SMC_Utils::smc_batch(float *a, mpz_t ***b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

void SMC_Utils::smc_batch(mpz_t ***a, float *b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

void SMC_Utils::smc_batch(float *a, mpz_t ***b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

void SMC_Utils::smc_batch(mpz_t ***a, float *b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// public + private one-dimension float - comparison
void SMC_Utils::smc_batch(float *a, mpz_t **b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

void SMC_Utils::smc_batch(mpz_t **a, float *b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// public + private two-dimension float - comparison
void SMC_Utils::smc_batch(float *a, mpz_t ***b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

void SMC_Utils::smc_batch(mpz_t ***a, float *b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

void SMC_Utils::smc_batch(mpz_t **a, mpz_t **b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// first parameter: one-dim float
// second parameter: two-dim float
// assignment parameter: two-dim float

void SMC_Utils::smc_batch(mpz_t **a, mpz_t ***b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// first parameter: two-dim float
// second parameter: one-dim float
// assignment parameter: two-dim float

void SMC_Utils::smc_batch(mpz_t ***a, mpz_t **b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// first parameter: two-dim float
// second parameter: two-dim float
// assignment parameter: one-dim float

void SMC_Utils::smc_batch(mpz_t ***a, mpz_t ***b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// first parameter: two-dim float
// second parameter: one-dim float
// assignment parameter: one-dim float

void SMC_Utils::smc_batch(mpz_t ***a, mpz_t **b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// first parameter: one-dim float
// second parameter: two-dim float
// assignment parameter: one-dim float

void SMC_Utils::smc_batch(mpz_t **a, mpz_t ***b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// first parameter: one-dim float
// second parameter: one-dim float
// assignment parameter: two-dim float

void SMC_Utils::smc_batch(mpz_t **a, mpz_t **b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// two dimension float general computation
void SMC_Utils::smc_batch(mpz_t ***a, mpz_t ***b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// two-dimensional float comparison
void SMC_Utils::smc_batch(mpz_t ***a, mpz_t ***b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// one-dimensional float and two-dimensional float comparison
void SMC_Utils::smc_batch(mpz_t **a, mpz_t ***b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// two-dimensional float and one-dimensional float comparison
void SMC_Utils::smc_batch(mpz_t ***a, mpz_t **b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

// one-dimensional float and one-dimensional float comparison
void SMC_Utils::smc_batch(mpz_t **a, mpz_t **b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
}

/* conversion from public integer to private float*/
void SMC_Utils::smc_batch_int2fl(int *a, mpz_t **result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

/* conversion from private integer to private float */
// one-dimensional int array to one-dimensional float array
void SMC_Utils::smc_batch_int2fl(mpz_t *a, mpz_t **result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

void SMC_Utils::smc_batch_int2fl(int *a, mpz_t ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

// one-dimensional int array to two-dimensional float array
void SMC_Utils::smc_batch_int2fl(mpz_t *a, mpz_t ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

// two-dimensional int array to one-dimensional float array
void SMC_Utils::smc_batch_int2fl(mpz_t **a, mpz_t **result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

// two-dimensional int array to two-dimensional float array
void SMC_Utils::smc_batch_int2fl(mpz_t **a, mpz_t ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

/*FL2INT*/
void SMC_Utils::smc_batch_fl2int(float *a, mpz_t *result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

void SMC_Utils::smc_batch_fl2int(float *a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

void SMC_Utils::smc_batch_fl2int(mpz_t **a, mpz_t *result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

void SMC_Utils::smc_batch_fl2int(mpz_t **a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

void SMC_Utils::smc_batch_fl2int(mpz_t ***a, mpz_t *result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

void SMC_Utils::smc_batch_fl2int(mpz_t ***a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}
// INT2INT
void SMC_Utils::smc_batch_int2int(int *a, mpz_t *result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

void SMC_Utils::smc_batch_int2int(int *a, mpz_t **result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

void SMC_Utils::smc_batch_int2int(mpz_t *a, mpz_t *result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

void SMC_Utils::smc_batch_int2int(mpz_t **a, mpz_t *result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

void SMC_Utils::smc_batch_int2int(mpz_t *a, mpz_t **result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

void SMC_Utils::smc_batch_int2int(mpz_t **a, mpz_t **result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

// FL2FL
void SMC_Utils::smc_batch_fl2fl(float *a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

void SMC_Utils::smc_batch_fl2fl(float *a, mpz_t ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

void SMC_Utils::smc_batch_fl2fl(mpz_t **a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

void SMC_Utils::smc_batch_fl2fl(mpz_t **a, mpz_t ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

void SMC_Utils::smc_batch_fl2fl(mpz_t ***a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

void SMC_Utils::smc_batch_fl2fl(mpz_t ***a, mpz_t ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
}

double SMC_Utils::time_diff(struct timeval *t1, struct timeval *t2) {
    double elapsed;
    if (t1->tv_usec > t2->tv_usec) {
        t2->tv_usec += 1000000;
        t2->tv_sec--;
    }
    elapsed = (t2->tv_sec - t1->tv_sec) + (t2->tv_usec - t1->tv_usec) / 1000000.0;
    return elapsed;
}

std::vector<std::string> SMC_Utils::splitfunc(const char *str, const char *delim) {
    char *saveptr;
    char *token = strtok_r((char *)str, delim, &saveptr);
    std::vector<std::string> result;
    while (token != NULL) {
        result.push_back(token);
        token = strtok_r(NULL, delim, &saveptr);
    }
    return result;
}

void SMC_Utils::smc_test_op(mpz_t *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, int threadID) {
#if __DEPLOYMENT__
    printf("DEPLOYMENT MODE\n");
#else
    printf("BENCHMARK MODE\n");
#endif

    int K = alen;
    int M = ceil(log2(K));

    mpz_t field;
    mpz_init(field);
    ss->getFieldSize(field);
    printf("Testing PRandInt for (K = %i, M = %i), size = %i)\n", K, M, size);
    PRandInt(K, M, size, a, threadID, ss);
    for (int i = 0; i < size; i++) {
        gmp_printf("a[%i] %Zu\n", i, a[i]);
    }
    Open(a, b, size, threadID, net, ss);
    for (int i = 0; i < size; i++) {
        gmp_printf("result[%i] %Zu\n", i, b[i]);
    }

    printf("Testing PRandBit for (K = %i, M = %i), size = %i)\n", K, M, size);
    PRandBit(size, a, threadID, net, id, ss);
    for (int i = 0; i < size; i++) {
        gmp_printf("a[%i] %Zu\n", i, a[i]);
    }
    Open(a, a, size, threadID, net, ss);
    for (int i = 0; i < size; i++) {
        gmp_printf("result[%i] %Zu\n", i, a[i]);
    }

    K = 32;
    int Kp1 = K + 1;
    M = ceil(log2(K));
    mpz_t **V = (mpz_t **)malloc(sizeof(mpz_t *) * (Kp1));
    for (int i = 0; i < Kp1; ++i) {
        V[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; ++j)
            mpz_init(V[i][j]);
    }

    mpz_t *res = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *a_test = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *res_check = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; ++i) {
        mpz_init(res[i]);
        mpz_init(a_test[i]);
        mpz_init(res_check[i]);
    }
    // // gmp_printf("Testing PRZS for (mod = %Zu), size = %i)\n", field, size);
    // ss->PRZS(field, size, a_test);
    // // printf("PRZS end\n");
    // Open_from_all(a_test, res, size, threadID, net, ss);
    // for (int i = 0; i < size; i++) {
    //     gmp_printf("result[%i] %Zu\n", i, res[i]);
    // }

    PRandM(K, size, V, threadID, net, id, ss); // generating r', r'_k-1,...,r'_0

    printf("Testing PRandM for (K = %i, M = %i), size = %i)\n", K, M, size);

    for (int i = 0; i < Kp1 - 1; i++) {
        Open(V[i], res, size, threadID, net, ss);
        for (int j = 0; j < size; j++) {
            gmp_printf("%Zu ", res[j]);
        }
        printf("\n");
    }
    unsigned long pow = 1;

    // Open(V[0], res, size, threadID, net, ss);
    for (int j = 0; j < size; j++) {
        mpz_set(res_check[j], V[0][j]); // setting the first bit of accumulator to b_0
    }
    mpz_t temp;
    mpz_init(temp);

    for (int i = 1; i < Kp1 - 1; i++) {
        pow = pow << 1;
        // Open(V[i], res, size, threadID, net, ss);
        for (int j = 0; j < size; j++) {
            mpz_mul_ui(temp, V[i][j], pow);
            ss->modAdd(res_check[j], res_check[j], temp);
        }
    }

    Open(V[Kp1 - 1], res, size, threadID, net, ss);
    for (int j = 0; j < size; j++) {
        gmp_printf("result[%i] %Zu\n", j, res[j]);
    }

    Open(res_check, res, size, threadID, net, ss);
    for (int j = 0; j < size; j++) {
        gmp_printf("res_check[%i] %Zu\n", j, res[j]);
    }

    for (int i = 0; i < Kp1; ++i) {
        for (int j = 0; j < size; ++j)
            mpz_clear(V[i][j]);
        free(V[i]);
    }

    for (int i = 0; i < size; ++i) {
        mpz_clear(res[i]);
        mpz_clear(a_test[i]);
        mpz_clear(res_check[i]);
    }
    mpz_clear(field);

    free(res_check);
    free(res);
    free(a_test);
}

// seed_map - contains binary encodings of access sets T (as defined in Baccarini et al., "Multi-Party Replicated Secret Sharing over a Ring with Applications to Privacy-Preserving Machine Learning," 2023)
// 2*KEYSIZE - 16 bytes for
void SMC_Utils::seedSetup(vector<int> &seed_map, int peers, int threshold) {
    uint8_t RandomData_send[2 * KEYSIZE];
    uint8_t RandomData_recv[2 * KEYSIZE];
    memset(RandomData_recv, 0, sizeof(uint8_t) * 2 * KEYSIZE);
    vector<int> T_recv, send_map, recv_map; // declaring once to be reused
#if __SHAMIR__
    vector<int> coefficients;
#endif
    // its fine to reuse vectors when inserting into the final mapping
    FILE *fp = fopen("/dev/urandom", "r"); // used for pulling good randomness for seeds
    try {
        for (auto &seed : seed_map) {
            send_map = extract_share_WITH_ACCESS(seed, peers, id);
            recv_map = extract_share_WITHOUT_ACCESS(seed, peers, id); // equivalent to T_mine in the current iteration

            // ANB: is 32 bytes "enough" of a seed for Shamir SS?
            // Previously in PRSS.cpp, the 'seeds' (denoted by "keys[i]") were generated as follows:
            //      mpz_urandomm(keys[i], gmpRandState, modulus);
            // keys[] is what is ultimately used to seed the shamir PRG (in shamir/SecretShare.cpp):
            //      gmp_randseed(rstates[i], keys[i]);
            // My take - I think it should be fine with the 32 bytes of "good" randomness
            if (fread(RandomData_send, 1, (2 * KEYSIZE), fp) != (2 * KEYSIZE))
                throw std::runtime_error("error reading random bytes from /dev/urandom");

            assert(send_map.size() == recv_map.size());
            for (size_t i = 0; i < send_map.size(); i++) {
                // printf("send %i\n", send_map[i]);
                net.sendDataToPeer(send_map[i], 2 * KEYSIZE, RandomData_send);
                // printf("recv %i\n", recv_map[i]);
                net.getDataFromPeer(recv_map[i], 2 * KEYSIZE, RandomData_recv);

                // generating the share id T corresponding to the key I just recieved
                T_recv = extract_share_WITHOUT_ACCESS(seed, peers, recv_map[i]);
                sort(T_recv.begin(), T_recv.end());
#if __SHAMIR__
                coefficients = generateCoefficients(T_recv, threshold);
                shamir_seeds_coefs.insert(std::pair<std::string, std::vector<int>>(reinterpret_cast<char *>(RandomData_recv), coefficients));

#endif
#if __RSS__
                rss_share_seeds.insert(std::pair<std::vector<int>, uint8_t *>(T_recv, RandomData_recv));

#endif
            }
            sort(recv_map.begin(), recv_map.end()); // sorting now that we're done using it to know the order which we're recieving shares
#if __SHAMIR__
            coefficients = generateCoefficients(recv_map, threshold);
            shamir_seeds_coefs.insert(std::pair<std::string, std::vector<int>>(reinterpret_cast<char *>(RandomData_send), coefficients));

#endif
#if __RSS__
            rss_share_seeds.insert(std::pair<std::vector<int>, uint8_t *>(recv_map, RandomData_send));
#endif
        }
        fclose(fp);
    } catch (const std::runtime_error &ex) {
        std::cerr << "[seedSetup] " << ex.what() << "\n";
        exit(1);
    }
}

// binary_rep = bianry encoding of share T generated from seed_map
// e.g. 0001111
// produces either the set of parties with access (set bits, 1s) or without access (unset bits, 0s)
// the LSB is IGNORED, since that corresponds to the party itself, and is ALWAYS SET
// id - my PID (1,...,n)
// result - UNsorted vector of party ID's with or without access
// ANB: w.r.t. sending keys for seeds, DONT CHANGE THE ORDER (yet)
// this explicitly informs us the exact order which shares will be sent/received from
vector<int> SMC_Utils::extract_share_WITH_ACCESS(int binary_rep, int peers, int id) {
    // iterate through bits from right to left
    vector<int> result;
    for (int i = 1; i < peers; i++) { // scans through remaining n-1 (LSB always set)
        if (GET_BIT(binary_rep, i)) {
            result.push_back(((id + i - 1) % peers + 1));
        }
    }
    return result;
}
vector<int> SMC_Utils::extract_share_WITHOUT_ACCESS(int binary_rep, int peers, int id) {
    // iterate through bits from left to right
    vector<int> result;
    for (int i = peers - 1; i > 0; i--) { // scans through remaining n-1 (LSB always set)
        if (!GET_BIT(binary_rep, i)) {
            result.push_back(((id + i - 1) % peers + 1));
        }
    }
    return result;
}

// taken from PRSS in seed/
void SMC_Utils::getCombinations(std::vector<int> &elements, int reqLen, std::vector<int> &pos, int depth, int margin, std::vector<std::vector<int>> &result) {
    if (depth >= reqLen) {
        std::vector<int> temp;
        for (unsigned int i = 0; i < pos.size(); i++)
            temp.push_back(elements.at(pos.at(i)));
        result.push_back(temp);
        return;
    }
    if ((elements.size() - margin) < (unsigned int)(reqLen - depth))
        return;
    for (unsigned int i = margin; i < elements.size(); i++) {
        pos.at(depth) = i;
        getCombinations(elements, reqLen, pos, depth + 1, i + 1, result);
    }
    return;
}

// used for Shamir secret sharing only, not applicable for RSS
// generates the polynomials described in  section 3.1 of Cramer et al. TCC'05 paper
// one polynomial for maximum unqualified set (T_set)
// T_set is generated from the current seed and the party id of who I'm recieving a key from (or myself if it's the key I generated)
// threshold is the degree of the polynomial
std::vector<int> SMC_Utils::generateCoefficients(std::vector<int> T_set, int threshold) {
    std::vector<int> coefficients = {1}; // the first coefficient is always 1

    transform(T_set.begin(), T_set.end(), T_set.begin(), std::bind(std::multiplies<int>(), std::placeholders::_1, -1)); // negating every element in T_set (only within the scope of this function since it's passed by value

    for (int j = 1; j <= threshold; j++) {
        std::vector<int> pos(j);
        std::vector<std::vector<int>> result;
        getCombinations(T_set, j, pos, 0, 0, result);
        int coef = 0;
        for (unsigned int m = 0; m < result.size(); m++) {
            std::vector<int> temp = result.at(m);
            int tmp = 1;
            for (unsigned int n = 0; n < temp.size(); n++) {
                tmp *= temp.at(n);
            }
            coef += tmp;
        }
        coefficients.push_back(coef);
    }
    return coefficients;
}