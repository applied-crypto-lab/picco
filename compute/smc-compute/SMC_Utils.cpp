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
#include "bit_utils.hpp"
#include <functional>
#include <iomanip>
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;
using std::vector;

// Constructors
SMC_Utils::SMC_Utils(int _id, std::string runtime_config, std::string privatekey_filename, int numOfInputPeers, int numOfOutputPeers, std::string *IO_files, int numOfPeers, int threshold, int bits, std::string mod, std::vector<int> &seed_map, int num_threads) {
    id = _id;
#if __DEPLOYMENT__
    printf("DEPLOYMENT MODE\n");
#else
    printf("BENCHMARK MODE\n");
#endif

    std::cout << "SMC_Utils constructor\n";
#if __SHAMIR__
    priv_int modulus;
    ss_init(modulus);
    mpz_set_str(modulus, mod.c_str(), BASE_10);
#endif

    nodeConfig = new NodeConfiguration(id, runtime_config, bits, (numOfPeers - 1));

    std::cout << "Creating the NodeNetwork\n";
    NodeNetwork *nodeNet = new NodeNetwork(nodeConfig, privatekey_filename, num_threads);
    net = *nodeNet; // dereferencing, net is "copy initialized"

    seedSetup(seed_map, numOfPeers, threshold);

    std::cout << "Creating SecretShare\n";
#if __SHAMIR__
    if (num_threads > 1) {
        net.launchManager(); // launching thread manager here as to not conflict with seed setup, only done for Shamir since RSS doesn't support multithreading
        printf("Technique: SHAMIR\n");
    }
    ss = new SecretShare(numOfPeers, threshold, modulus, id, num_threads, net.getPRGseeds(), shamir_seeds_coefs);
#endif
#if __RSS__

    if (bits > 8 * sizeof(priv_int_t)) {
        std::cerr << "ring size cannot be larger than the bitlength of priv_int_t\nExiting...\n";

        std::exit(1);
    }
    printf("Technique: RSS\n");
    ss = new replicatedSecretShare<std::remove_pointer_t<priv_int>>(id, numOfPeers, threshold, bits, rss_share_seeds);

#endif

// initialize input and output streams (deployment mode only)
#if __DEPLOYMENT__
// #if __SHAMIR__
    try {
        inputStreams = new std::ifstream[numOfInputPeers];
        outputStreams = new std::ofstream[numOfOutputPeers];
        for (int i = 0; i < numOfInputPeers; i++) {
            inputStreams[i].open(IO_files[i].c_str(), std::ifstream::in);
            if (!inputStreams[i]) {
                std::cout << "Input files from input party " + std::to_string(i + 1) + " could not be opened\n";
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
                std::cout << "Output files from output party " + std::to_string(i + 1) + " could not be opened\n";
                std::exit(1);
            }
        }
    } catch (const std::runtime_error &ex) {
        // capturing error message from original throw
        std::string error(ex.what());
        // appending to new throw, then re-throwing
        throw std::runtime_error("[SMC_Utils, constructor] " + error);
    }
#endif
}

/* Specific SMC Utility Functions */
// open a single integer value
int SMC_Utils::smc_open(priv_int var, int threadID) {
    return Open_int(var, threadID, net, ss);
}

// open a single floating-point value
float SMC_Utils::smc_open(priv_int *var, int threadID) {
    return Open_float(var, threadID, net, ss);
}

#if __SHAMIR__
void SMC_Utils::smc_open(priv_int *result, priv_int *input, int size, int threadID) {
    Open(result, input, size, threadID, net, ss);
}
#endif

#if __RSS__
void SMC_Utils::smc_open(priv_int result, priv_int *var, int size, int threadID) {
    Open(result, var, size, threadID, net, ss);
}
#endif

// for integer variable I/O
// why is this a pointer?
void SMC_Utils::smc_input(int id, int *var, std::string type, int threadID) {
    try {
        ss->ss_input(id, var, type, inputStreams);
    } catch (const std::runtime_error &ex) {
        std::cerr << "[smc_input] " << ex.what() << "\nExiting...\n";
        exit(1);
    }
}

// why is this a pointer?
void SMC_Utils::smc_input(int id, priv_int *var, std::string type, int threadID) {
#if __DEPLOYMENT__
    try {
        ss->ss_input(id, var, type, inputStreams);
    } catch (const std::runtime_error &ex) {
        std::cerr << "[smc_input] " << ex.what() << "\nExiting...\n";
        exit(1);
    }

#else
    ss->ss_input(var, type);
#endif
}

// for float variable I/O
void SMC_Utils::smc_input(int id, float *var, std::string type, int threadID) {
    try {
        ss->ss_input(id, var, type, inputStreams);
    } catch (const std::runtime_error &ex) {
        std::cerr << "[smc_input] " << ex.what() << "\nExiting...\n";
        exit(1);
    }
}

// input private float
void SMC_Utils::smc_input(int id, priv_int **var, std::string type, int threadID) {
#if __DEPLOYMENT__
    try {
        ss->ss_input(id, var, type, inputStreams);
    } catch (const std::runtime_error &ex) {
        std::cerr << "[smc_input] " << ex.what() << "\nExiting...\n";
        exit(1);
    }
#else
    ss->ss_input(var, type);
#endif
}

// one-dimensional int array I/O
void SMC_Utils::smc_input(int id, priv_int *var, int size, std::string type, int threadID) {
#if __DEPLOYMENT__
    try {
        ss->ss_input(id, var, size, type, inputStreams);
    } catch (const std::runtime_error &ex) {
        std::cerr << "[smc_input] " << ex.what() << "\nExiting...\n";
        exit(1);
    }
#else
    ss->ss_input(var, size, type);
#endif
}

void SMC_Utils::smc_input(int id, int *var, int size, std::string type, int threadID) {
    try {
        ss->ss_input(id, var, size, type, inputStreams);
    } catch (const std::runtime_error &ex) {
        std::cerr << "[smc_input] " << ex.what() << "\nExiting...\n";
        exit(1);
    }
}

// one-dimensional float array I/O
void SMC_Utils::smc_input(int id, priv_int **var, int size, std::string type, int threadID) {
#if __DEPLOYMENT__
    try {
        ss->ss_input(id, var, size, type, inputStreams);
    } catch (const std::runtime_error &ex) {
        std::cerr << "[smc_input] " << ex.what() << "\nExiting...\n";
        exit(1);
    }
#else
    ss->ss_input(var, size, type);
#endif
}

#if __SHAMIR__

void SMC_Utils::smc_add_ui(priv_int rop, priv_int op1, uint op2) {

    ss->modAdd(rop, op1, op2);
}
void SMC_Utils::smc_sub_ui(priv_int rop, priv_int op1, uint op2) {

    ss->modSub(rop, op1, op2);
}
#endif
#if __RSS__

void SMC_Utils::smc_add_ui(priv_int *rop, priv_int *op1, uint op2) {

    ss->modAdd(rop, op1, op2, 1);
}
void SMC_Utils::smc_sub_ui(priv_int *rop, priv_int *op1, uint op2) {

    ss->modSub(rop, op1, op2, 1);
}
#endif

void SMC_Utils::smc_input(int id, float *var, int size, std::string type, int threadID) {
    try {
        ss->ss_input(id, var, size, type, inputStreams);
    } catch (const std::runtime_error &ex) {
        std::cerr << "[smc_input] " << ex.what() << "\nExiting...\n";
        exit(1);
    }
}

void SMC_Utils::smc_output(int id, int *var, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, type, outputStreams);
#endif // otherwise, do nothing
}

void SMC_Utils::smc_output(int id, priv_int *var, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, type, outputStreams);
#endif
}

void SMC_Utils::smc_output(int id, float *var, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, type, outputStreams);
#endif
}

void SMC_Utils::smc_output(int id, priv_int **var, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, type, outputStreams);
#endif
}

void SMC_Utils::smc_output(int id, priv_int *var, int size, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, size, type, outputStreams);
#endif
}

void SMC_Utils::smc_output(int id, int *var, int size, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, size, type, outputStreams);
#endif
}

void SMC_Utils::smc_output(int id, priv_int **var, int size, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, size, type, outputStreams);
#endif
}

void SMC_Utils::smc_output(int id, float *var, int size, std::string type, int threadID) {
#if __DEPLOYMENT__
    ss->ss_output(id, var, size, type, outputStreams);
#endif
}

/* SMC Addition */
void SMC_Utils::smc_add(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modAdd(result, a, b);
}

void SMC_Utils::smc_add(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modAdd(result, a, b);
}

void SMC_Utils::smc_add(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modAdd(result, b, a);
}

void SMC_Utils::smc_add(priv_int *a, float b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    priv_int *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    smc_add(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID);
    ss_batch_free_operator(&btmp, 4);
}

void SMC_Utils::smc_add(float a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    priv_int *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    smc_add(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, type, threadID);
    ss_batch_free_operator(&atmp, 4);
}

void SMC_Utils::smc_add(priv_int *a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    ss_single_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, "+", threadID, net, ss);
}

// batch version of smc_add
void SMC_Utils::smc_add(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss->modAdd(result, b, a, size);
}

void SMC_Utils::smc_add(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss->modAdd(result, a, b, size);
}

void SMC_Utils::smc_add(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss->modAdd(result, a, b, size);
}

void SMC_Utils::smc_add(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    priv_int **btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    smc_add(a, btmp, alen_sig, alen_exp, alen_sig, alen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
    ss_batch_free_operator(&btmp, size);
}

void SMC_Utils::smc_add(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    priv_int **atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    smc_add(atmp, b, blen_sig, blen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
    ss_batch_free_operator(&atmp, size);
}

void SMC_Utils::smc_add(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    ss_batch_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "+", threadID, net, ss);
}

void SMC_Utils::smc_set(priv_int *a, priv_int *result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    ss_set(a, result, alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID, net, ss);
}

void SMC_Utils::smc_set(priv_int **a, priv_int **result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    for (int i = 0; i < size; i++)
        ss_set(a[i], result[i], alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID, net, ss);
}

void SMC_Utils::smc_set(float *a, priv_int **result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, int size,std::string type, int threadID) {
    for (int i = 0; i < size; i++)
        ss_set(a[i], result[i], alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID, net, ss);
}

void SMC_Utils::smc_set(float a, priv_int *result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    ss_set(a, result, alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID, net, ss);
}

void SMC_Utils::smc_set(priv_int a, priv_int result, int alen, int resultlen, std::string type, int threadID) {
    ss_set(a, result, alen, resultlen, type, threadID, net, ss);
}

void SMC_Utils::smc_set(priv_int *a, priv_int *result, int alen, int resultlen, int size, std::string type, int threadID) {

#if __SHAMIR__
    for (int i = 0; i < size; i++)
        ss_set(a[i], result[i], alen, resultlen, type, threadID, net, ss);
#endif

#if __RSS__
    ss_set(a, result, alen, resultlen, size, type, threadID, net, ss);
#endif
}

// this routine should implement in a way that result = a + share[0]
void SMC_Utils::smc_set(int a, priv_int result, int alen, int resultlen, std::string type, int threadID) {
    ss_set(a, result, alen, resultlen, type, threadID, net, ss);
}

void SMC_Utils::smc_set(int *a, priv_int *result, int alen, int resultlen, int size, std::string type, int threadID) {
    for (size_t i = 0; i < size; i++) {
        ss_set(a[i], result[i], alen, resultlen, type, threadID, net, ss);
    }
}

void SMC_Utils::smc_priv_eval(priv_int a, priv_int b, priv_int cond, int threadID) {
    ss_priv_eval(a, b, cond, threadID, net, ss);
}

// floating point
void SMC_Utils::smc_priv_eval(priv_int *a, priv_int *b, priv_int cond, int threadID) {
    ss_priv_eval(a, b, cond, threadID, net, ss);
}

/* SMC Subtraction */
void SMC_Utils::smc_sub(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modSub(result, a, b);
}

void SMC_Utils::smc_sub(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modSub(result, a, b);
}

void SMC_Utils::smc_sub(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modSub(result, a, b);
}

void SMC_Utils::smc_sub(priv_int *a, float b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    priv_int *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    smc_sub(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID);
    ss_batch_free_operator(&btmp, 4);
}

void SMC_Utils::smc_sub(float a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    priv_int *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    smc_sub(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, type, threadID);
    ss_batch_free_operator(&atmp, 4);
}

void SMC_Utils::smc_sub(priv_int *a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    ss_sub(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, type, threadID, net, ss);
}

// batch operations of subtraction
void SMC_Utils::smc_sub(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss->modSub(result, a, b, size);
}

void SMC_Utils::smc_sub(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss->modSub(result, a, b, size);
}

void SMC_Utils::smc_sub(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss->modSub(result, a, b, size);
}

void SMC_Utils::smc_sub(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    ss_sub(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID, net, ss);
}

void SMC_Utils::smc_sub(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    ss_sub(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID, net, ss);
}

void SMC_Utils::smc_sub(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    ss_sub(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID, net, ss);
}
#if __SHAMIR__
/* SMC Multiplication */
void SMC_Utils::smc_mult(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    // double check this works (passing reference to essentiall up-cast to priv_int*)
    // cant use & for some reason, wont compile
    // replacing priv_int* with & works on
    Mult(MPZ_CAST(result), MPZ_CAST(a), MPZ_CAST(b), 1, threadID, net, ss);
}
#endif
/******************************************************/
void SMC_Utils::smc_mult(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modMul(result, a, b);
}

void SMC_Utils::smc_mult(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modMul(result, b, a);
}

void SMC_Utils::smc_mult(priv_int *a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    ss_single_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, "*", threadID, net, ss);
}

void SMC_Utils::smc_mult(float a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    priv_int *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    smc_mult(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, type, threadID);
    ss_batch_free_operator(&atmp, 4);
}

void SMC_Utils::smc_mult(priv_int *a, float b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    priv_int *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    smc_mult(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID);
    ss_batch_free_operator(&btmp, 4);
}

#if __SHAMIR__
// private float * private int (needs more examinations)
void SMC_Utils::smc_mult(priv_int *a, priv_int b, priv_int *result, int alen_sig, int alen_exp, int blen, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    priv_int b1, one;
    ss_init(b1);
    ss_init_set_ui(one, 1);
    // start computation
    ss->modSub(b1, one, b);
    smc_lor(a[2], b1, result[2], 1, 1, 1, type, threadID);
    // free the memory
    ss_clear(b1);
    ss_clear(one);
}
#endif

void SMC_Utils::smc_mult(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss->modMul(result, b, a, size);
}

void SMC_Utils::smc_mult(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss->modMul(result, a, b, size);
}

void SMC_Utils::smc_mult(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    Mult(result, a, b, size, threadID, net, ss);
}

void SMC_Utils::smc_mult(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    priv_int **btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    smc_mult(a, btmp, alen_sig, alen_exp, alen_sig, alen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
    ss_batch_free_operator(&btmp, size);
}

void SMC_Utils::smc_mult(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    priv_int **atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    smc_mult(atmp, b, blen_sig, blen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
    ss_batch_free_operator(&atmp, size);
}

void SMC_Utils::smc_mult(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    ss_batch_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "*", threadID, net, ss);
}

/* SMC Integer Division*/
#if __SHAMIR__
void SMC_Utils::smc_div(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_IntDiv(MPZ_CAST(result), MPZ_CAST(a), MPZ_CAST(b), resultlen, 1, threadID, net, ss);
    // doOperation_IntDiv(result, a, b, resultlen, threadID, net, ss);
}

void SMC_Utils::smc_div(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_IntDiv_Pub(MPZ_CAST(result), MPZ_CAST(a), (int *)&b, resultlen, 1, threadID, net, ss);
}

void SMC_Utils::smc_div(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_IntDiv(MPZ_CAST(result), (int *)&a, MPZ_CAST(b), resultlen, 1, threadID, net, ss);
}
#endif

void SMC_Utils::smc_div(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_IntDiv(result, a, b, resultlen, size, threadID, net, ss);
}

void SMC_Utils::smc_div(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_IntDiv(result, a, b, resultlen, size, threadID, net, ss);
}

void SMC_Utils::smc_div(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_IntDiv_Pub(result, a, b, resultlen, size, threadID, net, ss);
}

/* SMC Floating-point division */
void SMC_Utils::smc_div(priv_int *a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    ss_single_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, "/", threadID, net, ss);
}

void SMC_Utils::smc_div(priv_int *a, float b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    priv_int **results = (priv_int **)malloc(sizeof(priv_int *));
    priv_int **as = (priv_int **)malloc(sizeof(priv_int *));
    priv_int **bs;

    as[0] = (priv_int *)malloc(sizeof(priv_int) * 4);
    results[0] = (priv_int *)malloc(sizeof(priv_int) * 4);
    ss_batch_convert_to_private_float(&b, &bs, alen_sig, alen_exp, 1, ss);

    for (int i = 0; i < 4; i++) {
        ss_init(results[0][i]);
        ss_init_set(as[0][i], a[i]);
    }

    doOperation_FLDiv_Pub(as, bs, results, alen_sig, 1, threadID, net, ss);
    ss_process_results(results, resultlen_sig, resultlen_exp, alen_sig, alen_exp, 1, threadID, net, ss);

    for (int i = 0; i < 4; i++)
        ss_set(result[i], results[0][i]);

    // free the memory
    ss_batch_free_operator(&as, 1);
    ss_batch_free_operator(&bs, 1);
    ss_batch_free_operator(&results, 1);
}

void SMC_Utils::smc_div(float a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID) {
    priv_int *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    smc_div(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, type, threadID);
    ss_batch_free_operator(&atmp, 4);
}

void SMC_Utils::smc_div(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    ss_batch_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "/", threadID, net, ss);
}

void SMC_Utils::smc_div(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    priv_int **btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    doOperation_FLDiv_Pub(a, btmp, result, alen_sig, size, threadID, net, ss);
    ss_process_results(result, resultlen_sig, resultlen_exp, alen_sig, alen_exp, size, threadID, net, ss);
    ss_batch_free_operator(&btmp, size);
}

void SMC_Utils::smc_div(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID) {
    priv_int **atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    smc_div(atmp, b, blen_sig, blen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
    ss_batch_free_operator(&atmp, size);
}

/* All Comparisons */
#if __SHAMIR__
// Int
// 1) private int < private int
void SMC_Utils::smc_lt(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(a), MPZ_CAST(b), alen, blen, resultlen, 1, threadID, net, ss);
}
// 2) private int < public int
void SMC_Utils::smc_lt(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(a), (int *)&b, alen, blen, resultlen, 1, threadID, net, ss);
}
// 3) public int < private int
void SMC_Utils::smc_lt(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), (int *)&a, MPZ_CAST(b), alen, blen, resultlen, 1, threadID, net, ss);
}
#endif

// Float
// 4) private float < private float
void SMC_Utils::smc_lt(priv_int *a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    ss_single_fop_comparison(result, a, b, resultlen, alen_sig, alen_exp, blen_sig, blen_exp, "<0", threadID, net, ss);
}
// take b turn to array of 4 ints, using convertfloat same like the flltz and tq. 
// float can be larger than priv below -> but the size can be different for float, so we can use
// same as a or float -> so if the float is larger than a then it can change the size of the private a -> so we keeep the size same for both a and pub b
// 5) private float < public float -> (a, pb)
void SMC_Utils::smc_lt(priv_int *a, float b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    ss_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, alen_sig, alen_exp, "<0", threadID, net, ss);
    ss_batch_free_operator(&btmp, 4);
  
    // New_A_Code 
    // int *btmp = new int[4]; // add memory for btmp
    // long long *elements = new long long[4];
    // convertFloat(b, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     btmp[j] = (int)elements[j]; 
    // }

    // ss_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, "<0", threadID, net, ss);

    // delete[] btmp;
    // delete[] elements;
}
// 6) public float < private float -> (pa, b) -> Flip the order (b, pa) and the result (modSub)
void SMC_Utils::smc_lt(float a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    ss_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "<0", threadID, net, ss);
    ss_batch_free_operator(&atmp, 4);

    // New_A_Code
    // int *atmp = new int[4]; // add memory for atmp
    // long long *elements = new long long[4];
    // convertFloat(a, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     atmp[j] = (int)elements[j]; 
    // }

    // ss_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, "<0", threadID, net, ss);
    
    // delete[] atmp;
    // delete[] elements;
}

    /************ batch operations *********/
// 1) private *int = private *int < private *int
void SMC_Utils::smc_lt(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
}
// 2) private *int = private *float < private *float
void SMC_Utils::smc_lt(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "<0", threadID, net, ss);
}

// New_A_code
// int 
// 3) private *int = public *int < private *int
void SMC_Utils::smc_lt(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
}
// 4) private *int = private *int < public *int
void SMC_Utils::smc_lt(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
}

// float
// 5) private *int = public *float < private *float
void SMC_Utils::smc_lt(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "<0", threadID, net, ss);
}
// 6) private *int = private *float < public *float
void SMC_Utils::smc_lt(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, b, a, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "<0", threadID, net, ss);
}

#if __SHAMIR__
// 1) private int > private int
void SMC_Utils::smc_gt(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(b), MPZ_CAST(a), blen, alen, resultlen, 1, threadID, net, ss);
}
// 2) private int > public int
void SMC_Utils::smc_gt(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), (int *)&b, MPZ_CAST(a), blen, alen, resultlen, 1, threadID, net, ss);
}
// 3) public int > private int
void SMC_Utils::smc_gt(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(b), (int *)&a, blen, alen, resultlen, 1, threadID, net, ss);
}
#endif
// 4) private float > private float
void SMC_Utils::smc_gt(priv_int *a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    ss_single_fop_comparison(result, b, a, resultlen, blen_sig, blen_exp, alen_sig, alen_exp, "<0", threadID, net, ss);
}

// 5) private float > public float

void SMC_Utils::smc_gt(priv_int *a, float b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    ss_single_fop_comparison(result, btmp, a, resultlen, alen_sig, alen_exp, alen_sig, alen_exp, "<0", threadID, net, ss);
    ss_batch_free_operator(&btmp, 4);
    
    // New_A_Code
    // New version that uses the new protocol between private and public 
    // int *btmp = new int[4]; 

    // long long *elements = new long long[4];
    // convertFloat(b, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     btmp[j] = (int)elements[j]; 
    // }

    // ss_single_fop_comparison(result, btmp, a, resultlen, alen_sig, alen_exp, "<0", threadID, net, ss);

    // delete[] btmp;
    // delete[] elements;
}

// 6) public float > private float
void SMC_Utils::smc_gt(float a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    ss_single_fop_comparison(result, b, atmp, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "<0", threadID, net, ss);
    ss_batch_free_operator(&atmp, 4);

    // New_A_Code
    // New version that uses the new protocol between private and public 
    // int *atmp = new int[4];

    // long long *elements = new long long[4];
    // convertFloat(a, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     atmp[j] = (int)elements[j]; 
    // }
    
    // ss_single_fop_comparison(result, b, atmp, resultlen, blen_sig, blen_exp, "<0", threadID, net, ss);

    // delete[] atmp;
    // delete[] elements;
}

    /************ batch operations *********/
// 1) private *int = private *int > private *int
void SMC_Utils::smc_gt(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, b, a, blen, alen, resultlen, size, threadID, net, ss);
}
// 2) private *int = private *float > private *float
void SMC_Utils::smc_gt(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss_batch_fop_comparison(result, b, a, resultlen, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID, net, ss);
}

// New_A_code
// int 
// 3) private *int = public *int > private *int
void SMC_Utils::smc_gt(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, b, a, blen, alen, resultlen, size, threadID, net, ss);
}
// 4) private *int = private *int > public *int
void SMC_Utils::smc_gt(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, b, a, blen, alen, resultlen, size, threadID, net, ss);
}

// float 
// 5) private *int = public *float > private *float
void SMC_Utils::smc_gt(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, b, a, resultlen, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID, net, ss);
}
// 6) private *int = private *float > public *float
void SMC_Utils::smc_gt(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, b, a, resultlen, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID, net, ss);
}

#if __SHAMIR__
// 1) private int <= private int
void SMC_Utils::smc_leq(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(b), MPZ_CAST(a), blen, alen, resultlen, 1, threadID, net, ss);
    ss->modSub(result, 1, result);
}
// 2) private int <= public int
void SMC_Utils::smc_leq(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), (int *)&b, MPZ_CAST(a), blen, alen, resultlen, 1, threadID, net, ss);
    ss->modSub(result, 1, result);
}
// 3) public int <= private int
void SMC_Utils::smc_leq(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(b), (int *)&a, blen, alen, resultlen, 1, threadID, net, ss);
    ss->modSub(result, 1, result);
}
#endif
// 4) private float <= private float
void SMC_Utils::smc_leq(priv_int *a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    ss_single_fop_comparison(result, b, a, resultlen, blen_sig, blen_exp, alen_sig, alen_exp, "<0", threadID, net, ss);
    ss->modSub(result, 1, result);
}
// 5) private float <= public float
void SMC_Utils::smc_leq(priv_int *a, float b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    ss_single_fop_comparison(result, btmp, a, resultlen, alen_sig, alen_exp, alen_sig, alen_exp, "<0", threadID, net, ss);
    ss->modSub(result, 1, result);
    ss_batch_free_operator(&btmp, 4);

    // New_A_Code 
    // int *btmp = new int[4]; 
    // long long *elements = new long long[4];
    // convertFloat(b, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     btmp[j] = (int)elements[j]; 
    // }

    // ss_single_fop_comparison(result, btmp, a, resultlen, alen_sig, alen_exp, "<0", threadID, net, ss);
    // ss->modSub(result, 1, result);

    // delete[] btmp;
    // delete[] elements;
}

// 6) public float <= private float
void SMC_Utils::smc_leq(float a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    ss_single_fop_comparison(result, b, atmp, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "<0", threadID, net, ss);
    ss->modSub(result, 1, result);
    ss_batch_free_operator(&atmp, 4);

    // New_A_Code 
    // int *atmp = new int[4];

    // long long *elements = new long long[4];
    // convertFloat(a, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     atmp[j] = (int)elements[j]; 
    // }

    // ss_single_fop_comparison(result, b, atmp, resultlen, blen_sig, blen_exp, "<0", threadID, net, ss);
    // ss->modSub(result, 1, result);

    // delete[] atmp;
    // delete[] elements;
}

    /************ batch operations *********/
// 1) private *int = private *int <= private *int
void SMC_Utils::smc_leq(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, b, a, blen, alen, resultlen, size, threadID, net, ss);
    ss->modSub(result, 1, result, size);
}
// 2) private *int = private *float <= private *float
void SMC_Utils::smc_leq(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss_batch_fop_comparison(result, b, a, resultlen, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID, net, ss);
    ss->modSub(result, 1, result, size);
}

// New_A_code
// int 
// 3) private *int = public *int <= private *int
void SMC_Utils::smc_leq(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, b, a, blen, alen, resultlen, size, threadID, net, ss);
    ss->modSub(result, 1, result, size);
}
// 4) private *int = private *int <= public *int
void SMC_Utils::smc_leq(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, b, a, blen, alen, resultlen, size, threadID, net, ss);
    ss->modSub(result, 1, result, size);
}
// float 
// 5) private *int = public *float <= private *float
void SMC_Utils::smc_leq(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, b, a, resultlen, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID, net, ss);
    // ss->modSub(result, 1, result, size);
}
// 6) private *int = private *float <= public *float
void SMC_Utils::smc_leq(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, b, a, resultlen, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID, net, ss);
    // ss->modSub(result, 1, result, size);
}

#if __SHAMIR__
// 1) private int >= private int
void SMC_Utils::smc_geq(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(a), MPZ_CAST(b), alen, blen, resultlen, 1, threadID, net, ss);
    ss->modSub(result, 1, result);
}
// 2) private int >= public int
void SMC_Utils::smc_geq(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(a), (int *)&b, alen, blen, resultlen, 1, threadID, net, ss);
    ss->modSub(result, 1, result);
}
// 3) public int >= private int
void SMC_Utils::smc_geq(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), (int *)&a, MPZ_CAST(b), alen, blen, resultlen, 1, threadID, net, ss);
    ss->modSub(result, 1, result);
}
#endif
// 4) private float >= private float
void SMC_Utils::smc_geq(priv_int *a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    ss_single_fop_comparison(result, a, b, resultlen, alen_sig, alen_exp, blen_sig, blen_exp, "<0", threadID, net, ss);
    ss->modSub(result, 1, result);
}
// 5) private float >= public float
void SMC_Utils::smc_geq(priv_int *a, float b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    ss_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, blen_sig, blen_exp, "<0", threadID, net, ss);
    ss->modSub(result, 1, result);
    ss_batch_free_operator(&btmp, 4);

    // New_A_code
    // int *btmp = new int[4]; 
    // long long *elements = new long long[4];
    // convertFloat(b, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     btmp[j] = (int)elements[j]; 
    // }

    // ss_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, "<0", threadID, net, ss);
    // ss->modSub(result, 1, result); 

    // delete[] btmp;
    // delete[] elements;
}
// 6) public float >= private float
void SMC_Utils::smc_geq(float a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    ss_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "<0", threadID, net, ss);
    ss->modSub(result, 1, result);
    ss_batch_free_operator(&atmp, 4);

    // New_A_code
    // int *atmp = new int[4];
    // long long *elements = new long long[4];
    // convertFloat(a, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     atmp[j] = (int)elements[j]; 
    // }

    // ss_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, "<0", threadID, net, ss);
    // ss->modSub(result, 1, result);
    
    // delete[] atmp;
    // delete[] elements;
}

    /************ batch operations *********/
// 1) private *int = private *int >= private *int
void SMC_Utils::smc_geq(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
    ss->modSub(result, 1, result, size);
}
// 2) private *int = private *float >= private *float
void SMC_Utils::smc_geq(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "<0", threadID, net, ss);
    ss->modSub(result, 1, result, size);
}

// New_A_code 
// int
// 3) private *int = public *int >= private *int
void SMC_Utils::smc_geq(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
    ss->modSub(result, 1, result, size);
}
// 4) private *int = private *int >= public *int
void SMC_Utils::smc_geq(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_LT(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
    ss->modSub(result, 1, result, size);
}
// float
// 5) private *int = public *float >= private *float
void SMC_Utils::smc_geq(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "<0", threadID, net, ss);
    // ss->modSub(result, 1, result, size);
}
// 6) private *int = private *float >= public *float
void SMC_Utils::smc_geq(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "<0", threadID, net, ss);
    // ss->modSub(result, 1, result, size);
}

#if __SHAMIR__
// Equality and Inequality
// 1) private int == private int
void SMC_Utils::smc_eqeq(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_EQZ(MPZ_CAST(result), MPZ_CAST(a), MPZ_CAST(b), alen, blen, resultlen, 1, threadID, net, ss);
}
// 2) private int == public int
void SMC_Utils::smc_eqeq(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_EQZ(MPZ_CAST(result), MPZ_CAST(a), (int *)&b, alen, blen, resultlen, 1, threadID, net, ss);
}
// 3) public int == private int
void SMC_Utils::smc_eqeq(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_EQZ(MPZ_CAST(result), MPZ_CAST(b), (int *)&a, blen, alen, resultlen, 1, threadID, net, ss);
}
#endif
// 4) private float == private float
void SMC_Utils::smc_eqeq(priv_int *a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    ss_single_fop_comparison(result, a, b, resultlen, alen_sig, alen_exp, blen_sig, blen_exp, "==", threadID, net, ss);
}
// 5) private float == public float
void SMC_Utils::smc_eqeq(priv_int *a, float b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    ss_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, alen_sig, alen_exp, "==", threadID, net, ss);
    ss_batch_free_operator(&btmp, 4);

    // New_A_Code
    // int *btmp = new int[4]; 
    // long long *elements = new long long[4];
    // convertFloat(b, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     btmp[j] = (int)elements[j]; 
    // }

    // ss_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, "==", threadID, net, ss);
    
    // delete[] btmp;
    // delete[] elements;
}
// 6) public float == private float
void SMC_Utils::smc_eqeq(float a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    ss_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "==", threadID, net, ss);
    ss_batch_free_operator(&atmp, 4);

    // New_A_code
    // int *atmp = new int[4];
    // long long *elements = new long long[4];
    // convertFloat(a, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     atmp[j] = (int)elements[j]; 
    // }

    // ss_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, "==", threadID, net, ss);
   
    // delete[] atmp;
    // delete[] elements;
}

    /************ batch operations *********/
// 1) private *int == private *int
void SMC_Utils::smc_eqeq(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_EQZ(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
}
// 3) private *int == public *int
void SMC_Utils::smc_eqeq(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_EQZ(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
}
// 4) public *int == private *int
void SMC_Utils::smc_eqeq(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_EQZ(result, b, a, alen, blen, resultlen, size, threadID, net, ss);
}
// 2) private *float == private *float
void SMC_Utils::smc_eqeq(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "==", threadID, net, ss);
}
// New_A_code
// 5) public *float == private **float
void SMC_Utils::smc_eqeq(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "==", threadID, net, ss);
}
// 6) private **float == public *float
void SMC_Utils::smc_eqeq(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "==", threadID, net, ss);
}

#if __SHAMIR__
// 1) private int != private int
void SMC_Utils::smc_neq(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_EQZ(MPZ_CAST(result), MPZ_CAST(a), MPZ_CAST(b), alen, blen, resultlen, 1, threadID, net, ss);
    ss->modSub(result, 1, result);
}
// 2) private int != public int
void SMC_Utils::smc_neq(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_EQZ(MPZ_CAST(result), MPZ_CAST(a), (int *)&b, alen, blen, resultlen, 1, threadID, net, ss);
    ss->modSub(result, 1, result);
}
// 3) public int != private int
void SMC_Utils::smc_neq(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_EQZ(MPZ_CAST(result), MPZ_CAST(b), (int *)&a, blen, alen, resultlen, 1, threadID, net, ss);
    ss->modSub(result, 1, result);
}
#endif
// 4) private float != private float
void SMC_Utils::smc_neq(priv_int *a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    ss_single_fop_comparison(result, a, b, resultlen, alen_sig, alen_exp, blen_sig, blen_exp, "==", threadID, net, ss);
    ss->modSub(result, 1, result);
}
// 6) public float != private float
void SMC_Utils::smc_neq(float a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *atmp;
    ss_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp, ss);
    ss_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "==", threadID, net, ss);
    ss->modSub(result, 1, result);
    ss_batch_free_operator(&atmp, 4);

    // New_A_code
    // int *atmp = new int[4];
    // long long *elements = new long long[4];
    // convertFloat(a, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     atmp[j] = (int)elements[j]; 
    // }
    // ss_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, "==", threadID, net, ss);    
    // ss->modSub(result, 1, result);
    // delete[] atmp;
    // delete[] elements;
}
// 5) private float != public float
void SMC_Utils::smc_neq(priv_int *a, float b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID) {
    priv_int *btmp;
    ss_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp, ss);
    ss_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, alen_sig, alen_exp, "==", threadID, net, ss);
    ss->modSub(result, 1, result);
    ss_batch_free_operator(&btmp, 4);

    // New_A_code
    // int *btmp = new int[4]; 
    // long long *elements = new long long[4];
    // convertFloat(b, alen_sig, alen_exp, &elements);
    // for (int j = 0; j < 4; ++j) {
    //     btmp[j] = (int)elements[j]; 
    // }
    // ss_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, "==", threadID, net, ss);
    // ss->modSub(result, 1, result);
    // delete[] btmp;
    // delete[] elements;
}

    /************ batch operations *********/
// 1) private *int != private *int
void SMC_Utils::smc_neq(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_EQZ(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
    ss->modSub(result, 1, result, size);
}
// 2) private *float != private *float
void SMC_Utils::smc_neq(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "==", threadID, net, ss);
    ss->modSub(result, 1, result, size);
}

// New_A_code 
// int
// 3) private *int != public *int
void SMC_Utils::smc_neq(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_EQZ(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
    ss->modSub(result, 1, result, size);
}
// 4) public *int != private *int
void SMC_Utils::smc_neq(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // doOperation_EQZ(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
    // ss->modSub(result, 1, result, size);
}

// New_A_code
// float
// 5) private *float != public *float
void SMC_Utils::smc_neq(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "==", threadID, net, ss);
    // ss->modSub(result, 1, result, size);
}
// 6) public *float != private *float
void SMC_Utils::smc_neq(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    // ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "==", threadID, net, ss);
    // ss->modSub(result, 1, result, size);
}

// batch logical operations
// 2) private float && private float 
void SMC_Utils::smc_land(priv_int *a, priv_int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    LogicalAnd(a, b, result, alen, blen, resultlen, size, threadID, net, ss);
}

void SMC_Utils::smc_lor(priv_int *a, priv_int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    LogicalOr(a, b, result, alen, blen, resultlen, size, threadID, net, ss);
}

// batch logical operations
void SMC_Utils::smc_lnot(priv_int *a, int size, priv_int *result, int alen, int resultlen, std::string type, int threadID) {
    ss->modSub(result, 1, a, size);
}

void SMC_Utils::smc_band(priv_int *a, priv_int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    // ensuring the first argument always has the longer bitlength
    if (blen > alen) {
        BitAnd(a, b, result, alen, blen, resultlen, size, threadID, net, ss);
    } else {
        BitAnd(a, b, result, alen, blen, resultlen, size, threadID, net, ss);
    }
}
void SMC_Utils::smc_band(int *a, priv_int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    BitAnd(b, a, result, blen, alen, resultlen, size, threadID, net, ss);
}
void SMC_Utils::smc_band(priv_int *a, int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    BitAnd(a, b, result, alen, blen, resultlen, size, threadID, net, ss);
}
void SMC_Utils::smc_bxor(priv_int *a, priv_int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    // ensuring the first argument always has the longer bitlength
    if (blen > alen) {
        BitXor(b, a, result, blen, alen, resultlen, size, threadID, net, ss);
    } else {

        BitXor(a, b, result, alen, blen, resultlen, size, threadID, net, ss);
    }
}
void SMC_Utils::smc_bxor(int *a, priv_int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    BitXor(b, a, result, blen, alen, resultlen, size, threadID, net, ss);
}
void SMC_Utils::smc_bxor(priv_int *a, int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    BitXor(a, b, result, alen, blen, resultlen, size, threadID, net, ss);
}

void SMC_Utils::smc_bor(priv_int *a, priv_int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    if (blen > alen) {
        BitOr(a, b, result, alen, blen, resultlen, size, threadID, net, ss);
    } else {
        BitOr(a, b, result, alen, blen, resultlen, size, threadID, net, ss);
    }
}

void SMC_Utils::smc_bor(int *a, priv_int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    BitOr(b, a, result, blen, alen, resultlen, size, threadID, net, ss);
}
void SMC_Utils::smc_bor(priv_int *a, int *b, int size, priv_int *result, int alen, int blen, int resultlen, std::string type, int threadID) {
    BitOr(a, b, result, alen, blen, resultlen, size, threadID, net, ss);
}

#if __SHAMIR__

// bitwise operations
void SMC_Utils::smc_band(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    if (blen > alen) {
        BitAnd(MPZ_CAST(a), MPZ_CAST(b), MPZ_CAST(result), alen, blen, resultlen, 1, threadID, net, ss);
    } else {
        BitAnd(MPZ_CAST(a), MPZ_CAST(b), MPZ_CAST(result), alen, blen, resultlen, 1, threadID, net, ss);
    }
}

void SMC_Utils::smc_bxor(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    if (blen > alen) {
        BitXor(MPZ_CAST(a), MPZ_CAST(b), MPZ_CAST(result), alen, blen, resultlen, 1, threadID, net, ss);
    } else {
        BitXor(MPZ_CAST(a), MPZ_CAST(b), MPZ_CAST(result), alen, blen, resultlen, 1, threadID, net, ss);
    }
}

void SMC_Utils::smc_bor(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    if (blen > alen) {
        BitOr(MPZ_CAST(a), MPZ_CAST(b), MPZ_CAST(result), alen, blen, resultlen, 1, threadID, net, ss);
    } else {
        BitOr(MPZ_CAST(a), MPZ_CAST(b), MPZ_CAST(result), alen, blen, resultlen, 1, threadID, net, ss);
    }
}

// logical Operations
// 1) private int && private int 
void SMC_Utils::smc_land(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    LogicalAnd(MPZ_CAST(a), MPZ_CAST(b), MPZ_CAST(result), alen, blen, resultlen, 1, threadID, net, ss);
}

void SMC_Utils::smc_lnot(priv_int a, priv_int result, int alen, int resultlen, std::string type, int threadID) {
    ss->modSub(result, 1, a);
}

void SMC_Utils::smc_lor(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    LogicalOr(MPZ_CAST(a), MPZ_CAST(b), MPZ_CAST(result), alen, blen, resultlen, 1, threadID, net, ss);
}

void SMC_Utils::smc_shr(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    smc_shr(MPZ_CAST(a), MPZ_CAST(b), alen, blen, MPZ_CAST(result), resultlen, 1, type, threadID);
}

void SMC_Utils::smc_shr(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    smc_shr(MPZ_CAST(a), (int *)&b, alen, blen, MPZ_CAST(result), resultlen, 1, type, threadID);
}
#endif

void SMC_Utils::smc_shr(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    if (blen == -1) { // public b
        // check that m is !> k
        int *b_tmp = (int *)malloc(sizeof(int) * size);
        for (int i = 0; i < size; i++)
            b_tmp[i] = ss_get_si(b[i]);
        doOperation_Trunc(result, a, alen, b_tmp, size, threadID, net, ss);
        free(b_tmp);
    } else
        doOperation_TruncS(result, a, alen, b, size, threadID, net, ss);
}

void SMC_Utils::smc_shr(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    doOperation_Trunc(result, a, alen, b, size, threadID, net, ss);
}

#if __SHAMIR__
// ANB: does the compiler actually ever produce code that calls this?
void SMC_Utils::smc_shl(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    if (blen == -1) {           // b is public, but stored in an priv_int
        ss->modPow2(result, b); // problem for RSS, since we have an incompatible type ()
        ss->modMul(result, a, result);
    } else {
        doOperation_Pow2(MPZ_CAST(result), MPZ_CAST(b), blen, 1, threadID, net, ss);
        Mult(MPZ_CAST(result), MPZ_CAST(result), MPZ_CAST(a), 1, threadID, net, ss);
    }
}
#endif

void SMC_Utils::smc_shl(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID) {
    ss->modPow2(result, b);
    ss->modMul(result, a, result);
}

void SMC_Utils::smc_shl(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    if (blen == -1) {
        // b is PUBLIC, but stored in type priv_int
        ss->modPow2(result, b, size);
        ss->modMul(result, a, result, size);
    } else {
        doOperation_Pow2(result, b, blen, size, threadID, net, ss);
        Mult(result, result, a, size, threadID, net, ss);
    }
}

void SMC_Utils::smc_shl(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID) {
    ss->modPow2(result, b, size);
    ss->modMul(result, a, result, size);
}

// Dot Product
void SMC_Utils::smc_dot(priv_int *a, priv_int *b, int size, priv_int result, int threadID) {
    doOperation_DotProduct(a, b, result, size, threadID, net, ss);
}

void SMC_Utils::smc_dot(priv_int **a, priv_int **b, int size, int array_size, priv_int *result, std::string type, int threadID) {
    doOperation_DotProduct(a, b, result, size, array_size, threadID, net, ss);
}

// one-dimension private integer singular read
void SMC_Utils::smc_privindex_read(priv_int index, priv_int *array, priv_int result, int dim, std::string type, int threadID) {
    doOperation_PrivIndex_int(index, array, result, dim, 0, threadID, net, ss);
}

// two-dimension private integer singular read
void SMC_Utils::smc_privindex_read(priv_int index, priv_int **array, priv_int result, int dim1, int dim2, std::string type, int threadID) {
    doOperation_PrivIndex_int_arr(index, array, result, dim1, dim2, 0, threadID, net, ss);
}

// one-dimension private float singular read
void SMC_Utils::smc_privindex_read(priv_int index, priv_int **array, priv_int *result, int dim, std::string type, int threadID) {
    doOperation_PrivIndex_float(index, array, result, dim, 1, threadID, net, ss);
}

// two-dimension private float singular read
void SMC_Utils::smc_privindex_read(priv_int index, priv_int ***array, priv_int *result, int dim1, int dim2, std::string type, int threadID) {
    doOperation_PrivIndex_float_arr(index, array, result, dim1, dim2, 0, threadID, net, ss);
}

#if __SHAMIR__
// one-dimension private integer singular write
void SMC_Utils::smc_privindex_write(priv_int index, priv_int *array, int len_sig, int len_exp, int value, int dim, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    doOperation_PrivIndex_Write(MPZ_CAST(index), array, &value, dim, 1, out_cond, priv_cond, counter, threadID, 0, net, ss);
}

void SMC_Utils::smc_privindex_write(priv_int index, priv_int *array, int len_sig, int len_exp, priv_int value, int dim, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {

    doOperation_PrivIndex_Write(MPZ_CAST(index), array, MPZ_CAST(value), dim, 1, out_cond, priv_cond, counter, threadID, 0, net, ss);
}

// two-dimension private integer singular write
void SMC_Utils::smc_privindex_write(priv_int index, priv_int **array, int len_sig, int len_exp, int value, int dim1, int dim2, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {

    doOperation_PrivIndex_Write_2d(MPZ_CAST(index), array, (int *)&value, dim1, dim2, 1, out_cond, priv_cond, counter, threadID, 0, net, ss);
}

void SMC_Utils::smc_privindex_write(priv_int index, priv_int **array, int len_sig, int len_exp, priv_int value, int dim1, int dim2, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    doOperation_PrivIndex_Write_2d(MPZ_CAST(index), array, MPZ_CAST(value), dim1, dim2, 1, out_cond, priv_cond, counter, threadID, 0, net, ss);
}
#endif

// one-dimension private float singular write
void SMC_Utils::smc_privindex_write(priv_int index, priv_int **array, int len_sig, int len_exp, float value, int dim, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(index, array, len_sig, len_exp, value, dim, out_cond, priv_cond, counter, type, threadID, net, ss);
}

void SMC_Utils::smc_privindex_write(priv_int index, priv_int **array, int len_sig, int len_exp, priv_int *value, int dim, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(index, array, len_sig, len_exp, value, dim, out_cond, priv_cond, counter, type, threadID, net, ss);
}

// two-dimension private float singular write
void SMC_Utils::smc_privindex_write(priv_int index, priv_int ***array, int len_sig, int len_exp, float value, int dim1, int dim2, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(index, array, len_sig, len_exp, value, dim1, dim2, out_cond, priv_cond, counter, type, threadID, net, ss);
}

void SMC_Utils::smc_privindex_write(priv_int index, priv_int ***array, int len_sig, int len_exp, priv_int *value, int dim1, int dim2, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(index, array, len_sig, len_exp, value, dim1, dim2, out_cond, priv_cond, counter, type, threadID, net, ss);
}

// one-dimension private integer batch read
void SMC_Utils::smc_privindex_read(priv_int *indices, priv_int *arrays, priv_int *results, int dim, int size, std::string type, int threadID) {
    doOperation_PrivIndex_Read(indices, arrays, results, dim, size, threadID, 0, net, ss);
}

// two-dimension private integer batch read
void SMC_Utils::smc_privindex_read(priv_int *indices, priv_int **array, priv_int *results, int dim1, int dim2, int size, std::string type, int threadID) {
    ss_privindex_read(indices, array, results, dim1, dim2, size, type, threadID, net, ss);
}

// one-dimension private float batch read
void SMC_Utils::smc_privindex_read(priv_int *indices, priv_int **array, priv_int **results, int dim, int size, std::string type, int threadID) {
    ss_privindex_read(indices, array, results, dim, size, type, threadID, net, ss);
}

// two-dimension private float batch read
void SMC_Utils::smc_privindex_read(priv_int *indices, priv_int ***array, priv_int **results, int dim1, int dim2, int size, std::string type, int threadID) {
    ss_privindex_read(indices, array, results, dim1, dim2, size, type, threadID, net, ss);
}

// one-dimension private integer batch write
void SMC_Utils::smc_privindex_write(priv_int *indices, priv_int *array, int len_sig, int len_exp, int *values, int dim, int size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    doOperation_PrivIndex_Write(indices, array, values, dim, size, out_cond, priv_cond, counter, threadID, 0, net, ss);
}

void SMC_Utils::smc_privindex_write(priv_int *indices, priv_int *array, int len_sig, int len_exp, priv_int *values, int dim, int size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    doOperation_PrivIndex_Write(indices, array, values, dim, size, out_cond, priv_cond, counter, threadID, 0, net, ss);
}

void SMC_Utils::smc_privindex_write(priv_int *indices, priv_int **array, int len_sig, int len_exp, int *values, int dim1, int dim2, int size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(indices, array, len_sig, len_exp, values, dim1, dim2, size, out_cond, priv_cond, counter, type, threadID, net, ss);
}

// two-dimension private integer batch write
void SMC_Utils::smc_privindex_write(priv_int *indices, priv_int **array, int len_sig, int len_exp, priv_int *values, int dim1, int dim2, int size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(indices, array, len_sig, len_exp, values, dim1, dim2, size, out_cond, priv_cond, counter, type, threadID, net, ss);
}

// one-dimension private float batch write
void SMC_Utils::smc_privindex_write(priv_int *indices, priv_int **array, int len_sig, int len_exp, float *values, int dim, int size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(indices, array, len_sig, len_exp, values, dim, size, out_cond, priv_cond, counter, type, threadID, net, ss);
}
void SMC_Utils::smc_privindex_write(priv_int *indices, priv_int **array, int len_sig, int len_exp, priv_int **values, int dim, int size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(indices, array, len_sig, len_exp, values, dim, size, out_cond, priv_cond, counter, type, threadID, net, ss);
}

// two-dimension private float batch write
void SMC_Utils::smc_privindex_write(priv_int *indices, priv_int ***array, int len_sig, int len_exp, float *values, int dim1, int dim2, int size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(indices, array, len_sig, len_exp, values, dim1, dim2, size, out_cond, priv_cond, counter, type, threadID, net, ss);
}
void SMC_Utils::smc_privindex_write(priv_int *indices, priv_int ***array, int len_sig, int len_exp, priv_int **values, int dim1, int dim2, int size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID) {
    ss_privindex_write(indices, array, len_sig, len_exp, values, dim1, dim2, size, out_cond, priv_cond, counter, type, threadID, net, ss);
    ;
}

#if __SHAMIR__

// private pointer arithmetic only supported by shamirSS currently

priv_ptr SMC_Utils::smc_new_ptr(int level, int type) {
    return create_ptr(level, type);
}

priv_ptr *SMC_Utils::smc_new_ptr(int level, int type, int num) {
    return create_ptr(level, type, num);
}

/*************/
void SMC_Utils::smc_set_int_ptr(priv_ptr ptr, priv_int *var_loc, std::string type, int threadID) {
    set_ptr(ptr, var_loc, NULL, NULL, NULL, threadID);
}

void SMC_Utils::smc_set_float_ptr(priv_ptr ptr, priv_int **var_loc, std::string type, int threadID) {
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
void SMC_Utils::smc_update_int_ptr(priv_ptr ptr, priv_int *var_loc, priv_int private_tag, int index, int threadID) {
    update_ptr(ptr, var_loc, NULL, NULL, NULL, private_tag, index, threadID, net, ss);
}

void SMC_Utils::smc_update_float_ptr(priv_ptr ptr, priv_int **var_loc, priv_int private_tag, int index, int threadID) {
    update_ptr(ptr, NULL, var_loc, NULL, NULL, private_tag, index, threadID, net, ss);
}

void SMC_Utils::smc_update_struct_ptr(priv_ptr ptr, void *var_loc, priv_int private_tag, int index, int threadID) {
    update_ptr(ptr, NULL, NULL, var_loc, NULL, private_tag, index, threadID, net, ss);
}

void SMC_Utils::smc_update_ptr(priv_ptr ptr, priv_ptr *ptr_loc, priv_int private_tag, int index, int threadID) {
    update_ptr(ptr, NULL, NULL, NULL, ptr_loc, private_tag, index, threadID, net, ss);
}

void SMC_Utils::smc_update_ptr(priv_ptr assign_ptr, priv_ptr right_ptr, priv_int private_tag, int index, int threadID) {
    update_ptr(assign_ptr, right_ptr, private_tag, index, threadID, net, ss);
}

void SMC_Utils::smc_add_int_ptr(priv_ptr ptr, priv_int *var_loc, priv_int private_tag, int threadID) {
    add_ptr(ptr, var_loc, NULL, NULL, NULL, private_tag, threadID);
}

void SMC_Utils::smc_add_float_ptr(priv_ptr ptr, priv_int **var_loc, priv_int private_tag, int threadID) {
    add_ptr(ptr, NULL, var_loc, NULL, NULL, private_tag, threadID);
}

void SMC_Utils::smc_add_struct_ptr(priv_ptr ptr, void *var_loc, priv_int private_tag, int threadID) {
    add_ptr(ptr, NULL, NULL, var_loc, NULL, private_tag, threadID);
}

void SMC_Utils::smc_add_ptr(priv_ptr ptr, priv_ptr *ptr_loc, priv_int private_tag, int threadID) {
    add_ptr(ptr, NULL, NULL, NULL, ptr_loc, private_tag, threadID);
}

/*******************/
void SMC_Utils::smc_shrink_ptr(priv_ptr ptr, int current_index, int parent_index, int threadID) {
    shrink_ptr(ptr, current_index, parent_index, threadID);
}
/********************/
void SMC_Utils::smc_dereference_read_ptr(priv_ptr ptr, priv_int result, int num_of_dereferences, priv_int priv_cond, std::string type, int threadID) {
    dereference_ptr_read_var(ptr, result, num_of_dereferences, threadID, net, ss);
}

void SMC_Utils::smc_dereference_read_ptr(priv_ptr ptr, priv_int *result, int num_of_dereferences, priv_int priv_cond, std::string type, int threadID) {
    dereference_ptr_read_var(ptr, result, num_of_dereferences, threadID, net, ss);
}

void SMC_Utils::smc_dereference_read_ptr(priv_ptr ptr, priv_ptr result, int num_of_dereferences, priv_int priv_cond, std::string type, int threadID) {
    dereference_ptr_read_ptr(ptr, result, num_of_dereferences, priv_cond, threadID, net, ss);
}

void SMC_Utils::smc_dereference_write_ptr(priv_ptr ptr, priv_int value, int num_of_dereferences, priv_int private_cond, std::string type, int threadID) {
    dereference_ptr_write_var(ptr, value, num_of_dereferences, private_cond, threadID, net, ss);
}

void SMC_Utils::smc_dereference_write_int_ptr(priv_ptr ptr, priv_int *value, int num_of_dereferences, priv_int private_cond, std::string type, int threadID) {
    dereference_ptr_write(ptr, value, NULL, NULL, NULL, num_of_dereferences, private_cond, threadID, net, ss);
}

void SMC_Utils::smc_dereference_write_ptr(priv_ptr ptr, priv_int *value, int num_of_dereferences, priv_int private_cond, std::string type, int threadID) {
    dereference_ptr_write_var(ptr, value, num_of_dereferences, private_cond, threadID, net, ss);
}

void SMC_Utils::smc_dereference_write_float_ptr(priv_ptr ptr, priv_int **value, int num_of_dereferences, priv_int private_cond, std::string type, int threadID) {
    dereference_ptr_write(ptr, NULL, value, NULL, NULL, num_of_dereferences, private_cond, threadID, net, ss);
}

void SMC_Utils::smc_dereference_write_struct_ptr(priv_ptr ptr, void *value, int num_of_dereferences, priv_int private_cond, std::string type, int threadID) {
    dereference_ptr_write(ptr, NULL, NULL, value, NULL, num_of_dereferences, private_cond, threadID, net, ss);
}

void SMC_Utils::smc_dereference_write_ptr(priv_ptr ptr, priv_ptr *value, int num_of_dereferences, priv_int private_cond, std::string type, int threadID) {
    dereference_ptr_write(ptr, NULL, NULL, NULL, value, num_of_dereferences, private_cond, threadID, net, ss);
}

void SMC_Utils::smc_dereference_write_ptr(priv_ptr ptr, priv_ptr value, int num_of_dereferences, priv_int private_cond, std::string type, int threadID) {
    dereference_ptr_write_ptr(ptr, value, num_of_dereferences, private_cond, threadID, net, ss);
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

#endif

// private float = public int 
void SMC_Utils::smc_int2fl(int value, priv_int *result, int gamma, int K, int L, int threadID) {
    ss_int2fl(value, result, gamma, K, L, threadID, net, ss);
}

// private float = private int
void SMC_Utils::smc_int2fl(priv_int value, priv_int *result, int gamma, int K, int L, int threadID) {
    ss_int2fl(value, result, gamma, K, L, threadID, net, ss);
}

// private int = public int 
void SMC_Utils::smc_int2int(int value, priv_int result, int gamma1, int gamma2, int threadID) {
    ss_int2int(value, result, gamma1, gamma2, threadID, net, ss);
}

// private int = private int 
void SMC_Utils::smc_int2int(priv_int value, priv_int result, int gamma1, int gamma2, int threadID) {
    ss_int2int(value, result, gamma1, gamma2, threadID, net, ss);
}

// private int = public float
void SMC_Utils::smc_fl2int(float value, priv_int result, int K, int L, int gamma, int threadID) {
    ss_fl2int(value, result, K, L, gamma, threadID, net, ss);
}

// private int = private float 
void SMC_Utils::smc_fl2int(priv_int *value, priv_int result, int K, int L, int gamma, int threadID) {
    ss_fl2int(value, result, K, L, gamma, threadID, net, ss);
}

// Public float casted to private float
void SMC_Utils::smc_fl2fl(float value, priv_int *result, int K1, int L1, int K2, int L2, int threadID) {
    ss_fl2fl(value, result, K1, L1, K2, L2, threadID, net, ss);
}

// private float = public float 
void SMC_Utils::smc_fl2fl(priv_int *value, priv_int *result, int K1, int L1, int K2, int L2, int threadID) {
    ss_fl2fl(value, result, K1, L1, K2, L2, threadID, net, ss);
}

// private float* = private int* -> array
void SMC_Utils::smc_int2fl(priv_int *value, priv_int **result, int size, int gamma, int K, int L, int threadID) {
    ss_int2fl(value, result, size, gamma, K, L, threadID, net, ss);
}

// private float* = public int* -> array
void SMC_Utils::smc_int2fl(int *value, priv_int **result, int size, int gamma, int K, int L, int threadID) {
    ss_int2fl(value, result, size, gamma, K, L, threadID, net, ss);
}

// private int* = private int* -> array
void SMC_Utils::smc_int2int(priv_int *value, priv_int *result, int size, int gamma1, int gamma2, int threadID) {
    ss_int2int(value, result, size, gamma1, gamma2, threadID, net, ss);
}

// private int* = public int* -> array
void SMC_Utils::smc_int2int(int *value, priv_int *result, int size, int gamma1, int gamma2, int threadID) {
    ss_int2int(value, result, size, gamma1, gamma2, threadID, net, ss);
}

// private int* = private float* -> array
void SMC_Utils::smc_fl2int(priv_int **value, priv_int *result, int size, int K, int L, int gamma, int threadID) {
    ss_fl2int(value, result, size, K, L, gamma, threadID, net, ss);
}

// private int* = public float* -> array
void SMC_Utils::smc_fl2int(float *value, priv_int *result, int size, int K, int L, int gamma, int threadID) {
    ss_fl2int(value, result, size,  K, L, gamma, threadID, net, ss);
}

// private float* = public float* -> array
void SMC_Utils::smc_fl2fl(priv_int **value, priv_int **result, int size, int K1, int L1, int K2, int L2, int threadID) {
    ss_fl2fl(value, result, size, K1, L1, K2, L2, threadID, net, ss);
}

// Public float* casted to private float*
void SMC_Utils::smc_fl2fl(float *value, priv_int **result, int size, int K1, int L1, int K2, int L2, int threadID) {
    ss_fl2fl(value, result, size, K1, L1, K2, L2, threadID, net, ss);
}

/************************************ INTEGER BATCH ****************************************/
void SMC_Utils::smc_batch(priv_int *a, priv_int *b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// used to compute 1-priv_cond in a batch stmt
void SMC_Utils::smc_batch(int a, priv_int *b, priv_int *result, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, int threadID) {
    ss_batch(a, b, result, out_cond, priv_cond, counter, index_array, size, op, threadID, net, ss);
}

void SMC_Utils::smc_batch(priv_int *a, priv_int *b, priv_int *result, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, int threadID) {
    ss_batch(a, b, result, out_cond, priv_cond, counter, index_array, size, op, threadID, net, ss);
}

// first param: int array
// second param: int array
// third param: one-dim private int array
void SMC_Utils::smc_batch(int *a, int *b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: int array
// second param: int array
// third param: two-dim private int array
void SMC_Utils::smc_batch(int *a, int *b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: int array
// second param: one-dim private int array
// third param: one-dim private int array
void SMC_Utils::smc_batch(int *a, priv_int *b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: int array
// second param: one-dim private int array
// third param: two-dim private int array
void SMC_Utils::smc_batch(int *a, priv_int *b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: one-dim private int array
// second param: int array
// third param: one-dim private int array
void SMC_Utils::smc_batch(priv_int *a, int *b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: one-dim private int array
// second param: int array
// third param: two-dim private int array
void SMC_Utils::smc_batch(priv_int *a, int *b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: integer array
// second param: two-dim private int
// assignment param: one-dim private int
void SMC_Utils::smc_batch(int *a, priv_int **b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: integer array
// second param: two-dim private int
// assignment param: two-dim private int
void SMC_Utils::smc_batch(int *a, priv_int **b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: two-dim private int
// second param: integer array
// assignment param: one-dim private int
void SMC_Utils::smc_batch(priv_int **a, int *b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: two-dim private int
// second param: integer array
// assignment param: two-dim private int
void SMC_Utils::smc_batch(priv_int **a, int *b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: one-dim private int
// second param: two-dim private int
// assignment param: two-dim private int
void SMC_Utils::smc_batch(priv_int *a, priv_int **b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: two-dim private int
// second param: one-dim private int
// assignment param: two-dim private int
void SMC_Utils::smc_batch(priv_int **a, priv_int *b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: two-dim private int
// second param: two-dim private int
// assignment param: one-dim private int
void SMC_Utils::smc_batch(priv_int **a, priv_int **b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: one-dim private int
// second param: one-dim private int
// assignment param: two-dim private int
void SMC_Utils::smc_batch(priv_int *a, priv_int *b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}
// first param: one-dim private int
// second param: two-dim private int
// assignment param: one-dim private int
void SMC_Utils::smc_batch(priv_int *a, priv_int **b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first param: two-dim private int
// second param: one-dim private int
// assignment param: one-dim private int
void SMC_Utils::smc_batch(priv_int **a, priv_int *b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch_dot(priv_int **a, priv_int **b, int size, int array_size, int *index_array, priv_int *result, int threadID) {
    ss_batch_dot(a, b, size, array_size, index_array, result, threadID, net, ss);
}

// first param: two-dim private int
// second param: two-dim private int
// assignment param: two-dim private int

void SMC_Utils::smc_batch(priv_int **a, priv_int **b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

/*********************************************** FLOAT BATCH ****************************************************/
// public + private one-dimension float - arithmetic
void SMC_Utils::smc_batch(float *a, priv_int **b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(priv_int **a, float *b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(float *a, priv_int **b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(priv_int **a, float *b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// public to private assignments
void SMC_Utils::smc_batch(float *a, float *b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(int *a, int *b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(float *a, float *b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(int *a, int *b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// public + private two-dimension float - arithmetic
void SMC_Utils::smc_batch(float *a, priv_int ***b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(priv_int ***a, float *b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(float *a, priv_int ***b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(priv_int ***a, float *b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// public + private one-dimension float - comparison
void SMC_Utils::smc_batch(float *a, priv_int **b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(priv_int **a, float *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// public + private two-dimension float - comparison
void SMC_Utils::smc_batch(float *a, priv_int ***b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(priv_int ***a, float *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

void SMC_Utils::smc_batch(priv_int **a, priv_int **b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first parameter: one-dim float
// second parameter: two-dim float
// assignment parameter: two-dim float

void SMC_Utils::smc_batch(priv_int **a, priv_int ***b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first parameter: two-dim float
// second parameter: one-dim float
// assignment parameter: two-dim float

void SMC_Utils::smc_batch(priv_int ***a, priv_int **b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first parameter: two-dim float
// second parameter: two-dim float
// assignment parameter: one-dim float

void SMC_Utils::smc_batch(priv_int ***a, priv_int ***b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first parameter: two-dim float
// second parameter: one-dim float
// assignment parameter: one-dim float

void SMC_Utils::smc_batch(priv_int ***a, priv_int **b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first parameter: one-dim float
// second parameter: two-dim float
// assignment parameter: one-dim float

void SMC_Utils::smc_batch(priv_int **a, priv_int ***b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// first parameter: one-dim float
// second parameter: one-dim float
// assignment parameter: two-dim float

void SMC_Utils::smc_batch(priv_int **a, priv_int **b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// two dimension float general computation
void SMC_Utils::smc_batch(priv_int ***a, priv_int ***b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// two-dimensional float comparison
void SMC_Utils::smc_batch(priv_int ***a, priv_int ***b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// one-dimensional float and two-dimensional float comparison
void SMC_Utils::smc_batch(priv_int **a, priv_int ***b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// two-dimensional float and one-dimensional float comparison
void SMC_Utils::smc_batch(priv_int ***a, priv_int **b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

// one-dimensional float and one-dimensional float comparison
void SMC_Utils::smc_batch(priv_int **a, priv_int **b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
}

/* conversion from public integer to private float*/
void SMC_Utils::smc_batch_int2fl(int *a, priv_int **result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

/* conversion from private integer to private float */
// one-dimensional int array to one-dimensional float array
void SMC_Utils::smc_batch_int2fl(priv_int *a, priv_int **result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_int2fl(int *a, priv_int ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

// one-dimensional int array to two-dimensional float array
void SMC_Utils::smc_batch_int2fl(priv_int *a, priv_int ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

// two-dimensional int array to one-dimensional float array
void SMC_Utils::smc_batch_int2fl(priv_int **a, priv_int **result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

// two-dimensional int array to two-dimensional float array
void SMC_Utils::smc_batch_int2fl(priv_int **a, priv_int ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

/*FL2INT*/
void SMC_Utils::smc_batch_fl2int(float *a, priv_int *result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_fl2int(float *a, priv_int **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_fl2int(priv_int **a, priv_int *result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_fl2int(priv_int **a, priv_int **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_fl2int(priv_int ***a, priv_int *result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_fl2int(priv_int ***a, priv_int **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}
// INT2INT
void SMC_Utils::smc_batch_int2int(int *a, priv_int *result, int adim, int resultdim, int alen, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_int2int(int *a, priv_int **result, int adim, int resultdim, int alen, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_int2int(priv_int *a, priv_int *result, int adim, int resultdim, int alen, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_int2int(priv_int **a, priv_int *result, int adim, int resultdim, int alen, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_int2int(priv_int *a, priv_int **result, int adim, int resultdim, int alen, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_int2int(priv_int **a, priv_int **result, int adim, int resultdim, int alen, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

// FL2FL
void SMC_Utils::smc_batch_fl2fl(float *a, priv_int **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_fl2fl(float *a, priv_int ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_fl2fl(priv_int **a, priv_int **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_fl2fl(priv_int **a, priv_int ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_fl2fl(priv_int ***a, priv_int **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
}

void SMC_Utils::smc_batch_fl2fl(priv_int ***a, priv_int ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
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

std::string uint8PtrToString(const uint8_t *data, size_t length) {
    return std::string(data, data + length);
}

// seed_map - contains binary encodings of access sets T 
// (as defined in Baccarini et al., "Multi-Party Replicated Secret Sharing over a Ring with Applications to Privacy-Preserving Machine Learning," 2023)
// 2*KEYSIZE - 16 bytes for
void SMC_Utils::seedSetup(std::vector<int> &seed_map, int peers, int threshold) {
    // this works now, since we are copying the data to a string, which is persistent in scope
    uint8_t RandomData_send[2 * KEYSIZE];
    uint8_t RandomData_recv[2 * KEYSIZE];
    printf("seedSetup\n");
#if __SHAMIR__
    std::vector<int> coefficients;
    // typedef int var_type; // this is here because I don't feel like refactoring shamirSS to use uints
#endif
#if __RSS__
    // typedef int var_type; // this is here because I don't feel like refactoring shamirSS to use uints
    // typedef uint var_type;
#endif

    std::vector<int> T_recv, send_map, recv_map; // declaring once to be reused
    std::vector<int> recv_map_original;
    // its fine to reuse vectors when inserting into the final mapping
    FILE *fp = fopen("/dev/urandom", "r"); // used for pulling good randomness for seeds
    try {
        for (auto &seed : seed_map) {

            send_map = extract_share_WITH_ACCESS(seed, peers, id);
            recv_map_original = extract_share_WITHOUT_ACCESS(seed, peers, id);       // equivalent to T_mine in the current iteration
            recv_map = extract_share_WITHOUT_ACCESS_new(seed, peers, id, threshold); // equivalent to T_mine in the current iteration

            if (fread(RandomData_send, 1, (2 * KEYSIZE), fp) != (2 * KEYSIZE))
                throw std::runtime_error("error reading random bytes from /dev/urandom. Which OS are you using?");

            assert(send_map.size() == recv_map.size());
            for (size_t i = 0; i < send_map.size(); i++) {
                // printf("sending to %i\n", send_map[i]);
                net.sendDataToPeer(static_cast<int>(send_map[i]), 2 * KEYSIZE, RandomData_send);
                // print_hexa_2(RandomData_send, 2 * KEYSIZE);
                // printf("recv from %i\n", recv_map[i]);
                net.getDataFromPeer(static_cast<int>(recv_map[i]), 2 * KEYSIZE, RandomData_recv);
                // print_hexa_2(RandomData_recv, 2 * KEYSIZE);

                // generating the share id T corresponding to the key I just recieved
                T_recv = extract_share_WITHOUT_ACCESS(seed, peers, recv_map[i]);
                std::vector<int> T_recv_test = extract_share_WITHOUT_ACCESS_new(seed, peers, recv_map[i], threshold);
                sort(T_recv.begin(), T_recv.end());
                sort(T_recv_test.begin(), T_recv_test.end());
#if __SHAMIR__
                coefficients = generateCoefficients(T_recv, threshold);
                shamir_seeds_coefs.insert(std::pair<std::string, std::vector<int>>(reinterpret_cast<char *>(RandomData_recv), coefficients));

#endif
#if __RSS__
                // cout << "inserting (original)" << T_recv << endl;
                // cout << "inserting (new)     " << T_recv_test << endl; // SHOULDNT BE USED
                // print_hexa_2(RandomData_recv, 2 * KEYSIZE);
                std::string result = uint8PtrToString(RandomData_recv, 2 * KEYSIZE);
                // rss_share_seeds.insert(std::pair<std::vector<int>, uint8_t *>(T_recv, RandomData_recv));
                rss_share_seeds.insert(std::pair<std::vector<int>, std::string>(T_recv, result));

#endif
            }
            // printf("\n");
            sort(recv_map.begin(), recv_map.end());                   // sorting now that we're done using it to know the order which we're recieving shares
            sort(recv_map_original.begin(), recv_map_original.end()); // sorting now that we're done using it to know the order which we're recieving shares
#if __SHAMIR__
            coefficients = generateCoefficients(recv_map, threshold);
            shamir_seeds_coefs.insert(std::pair<std::string, std::vector<int>>(reinterpret_cast<char *>(RandomData_send), coefficients));

#endif
#if __RSS__
            // cout << "inserting (recv_map_org)" << recv_map_original << endl;
            // cout << "inserting (recv_map_new)" << recv_map << endl;
            // print_hexa_2(RandomData_send, 2 * KEYSIZE);
            // rss_share_seeds.insert(std::pair<std::vector<int>, uint8_t *>(recv_map, RandomData_send));
            std::string result = uint8PtrToString(RandomData_send, 2 * KEYSIZE);
            rss_share_seeds.insert(std::pair<std::vector<int>, std::string>(recv_map_original, result));

            // printf("\n");
#endif
        }
        fclose(fp);
    } catch (const std::runtime_error &ex) {
        std::cerr << "[seedSetup] " << ex.what() << "\n";
        exit(1);
    }

    // used for testing
    // int ctr = 0;
    // for (auto &[key, value] : rss_share_seeds) {
    //     std::cout << key << endl;
    //     ctr += 1;
    //     // print_hexa_2(value, 2*KEYSIZE);
    // }
    // std::cout << "num keys = " << ctr << std::endl;
}

// binary_rep = bianry encoding of share T generated from seed_map
// e.g. 0001111
// produces either the set of parties with access (set bits, 1s) or without access (unset bits, 0s)
// the LSB is IGNORED, since that corresponds to the party itself, and is ALWAYS SET
// id - my PID (1,...,n)
// result - UNsorted vector of party ID's with or without access
// ANB: w.r.t. sending keys for seeds, DONT CHANGE THE ORDER (yet)
// this explicitly informs us the exact order which shares will be sent/received from
std::vector<int> SMC_Utils::extract_share_WITH_ACCESS(int binary_rep, int peers, int id) {
    // iterate through bits from right to left
    std::vector<int> result;
    for (int i = 1; i < peers; i++) { // scans through remaining n-1 (LSB always set)
        if (GET_BIT(binary_rep, i)) {
            result.push_back(((id + i - 1) % peers + 1));
        }
    }
    return result;
}
// older version, gives incorrect results for some binary representations for >=7 parties
// replaced by "naive" approach in "new" function
std::vector<int> SMC_Utils::extract_share_WITHOUT_ACCESS(int binary_rep, int peers, int id) {
    // iterate through bits from left to right
    std::vector<int> result;
    for (int i = peers - 1; i > 0; i--) { // scans through remaining n-1 (LSB always set)
        if (!GET_BIT(binary_rep, i)) {
            result.push_back(((id + i - 1) % peers + 1));
        }
    }
    return result;
}

std::vector<int> SMC_Utils::extract_share_WITHOUT_ACCESS_new(int binary_rep, int peers, int id, int threshold) {
    // first getting the sendIDs
    int tmp = 0;
    int idx = 0;
    std::vector<int> result(threshold);
    for (int peer = 1; peer <= peers; peer++) {
        // std::cout << "peer: " << peer << std::endl;
        idx = 0;
        for (int i = 1; i < peers; i++) { // scans through remaining n-1 (LSB always set)
            // std::cout << "i : " << i << std::endl;
            tmp = ((peer + i - 1) % peers + 1);
            if (GET_BIT(binary_rep, i)) {
                if (tmp == id) {
                    // std::cout << "idx : " << idx << std::endl;
                    // std::cout << "peer: " << peer << std::endl;
                    result.at(idx) = peer;
                    // std::cout << "result: " << result << std::endl;
                    break;
                } else {
                    idx += 1;
                }
            }
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

// std::vector<int> generateCoefficients(std::vector<int> T_set, int threshold);

#if __RSS__
uint SMC_Utils::getNumShares() {
    return ss->getNumShares();
}
using std::cout;
using std::endl;
using std::vector;

void SMC_Utils::offline_prg(uint8_t *dest, uint8_t *src, __m128i *ri) { // ri used to be void, replaced with __m128i* to compile
    __m128i orr, mr;
    __m128i *r = ri;

    orr = _mm_load_si128((__m128i *)src);
    mr = orr;

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
    mr = _mm_xor_si128(mr, orr);
    _mm_storeu_si128((__m128i *)dest, mr);
}

__m128i *SMC_Utils::offline_prg_keyschedule(uint8_t *src) {
    // correctness must be checked here (modified from original just to compile, used to not have cast (__m128i*))
    // __m128i *r = static_cast<__m128i *>(malloc(11 * sizeof(__m128i)));
    __m128i *r = new __m128i[11]; // alternate
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

void SMC_Utils::prg_aes_ni(priv_int_t *destination, uint8_t *seed, __m128i *key) {
    uint8_t res[16] = {};
    offline_prg(res, seed, key);
    memset(seed, 0, 16);
    memset(destination, 0, sizeof(priv_int_t));
    memcpy(seed, res, sizeof(priv_int_t));        // cipher becomes new seed or key
    memcpy(destination, res, sizeof(priv_int_t)); // cipher becomes new seed or key
}

void SMC_Utils::smc_rss_benchmark(string operation, int size, int num_iterations) {
    struct timeval start;
    struct timeval end;
    uint numShares = ss->getNumShares();
    unsigned long timer;
    uint numParties = ss->getPeers();

    uint ring_size = ss->ring_size;

    uint bytes = (ss->ring_size + 7) >> 3;
    printf("bytes : %u\n", bytes);
    printf("ring_size : %u\n", ring_size);
    printf("size : %u\n", size);
    printf("8*sizeof(Lint) = %lu\n", 8 * sizeof(priv_int_t));
    printf("sizeof(Lint) = %lu\n", sizeof(priv_int_t));
    priv_int *a = new priv_int[numShares];
    priv_int *b = new priv_int[numShares];
    priv_int *c = new priv_int[numShares];

    for (int i = 0; i < numShares; i++) {
        a[i] = new priv_int_t[size];
        memset(a[i], 0, sizeof(priv_int_t) * size);
        b[i] = new priv_int_t[size];
        memset(b[i], 0, sizeof(priv_int_t) * size);
        c[i] = new priv_int_t[size];
        memset(a[i], 0, sizeof(priv_int_t) * size);
    }

    std::cout << "START" << std::endl;
    gettimeofday(&start, NULL); // start timer here
    if (operation == "b2a") {
        for (size_t j = 0; j < num_iterations; j++) {
            Rss_B2A(c, a, size, ring_size, net, ss);
        }

    } else if (operation == "mult") {

        for (size_t j = 0; j < num_iterations; j++) {
            Mult(c, a, b, size, net, ss);
            // Rss_Mult_7pc_test(c, a, b, size, ring_size, net, ss);
            // Rss_B2A(c, a, b, size, ring_size, net, ss);
        }
    } else if (operation == "fl_mul") {

    } else if (operation == "fl_add") {

    } else if (operation == "fl_div") {

    } else if (operation == "fl_cmp") {
    }

    else {
        std::cerr << "ERROR: unknown operation " << operation << ", exiting..." << endl;
        exit(1);
    }
    std::cout << "END" << std::endl;
    // std::cout<<numBytesSent<<std::endl;

    gettimeofday(&end, NULL); // stop timer here
    timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    printf("[%s_%spc] [%u, %i, %u] [%.6lf ms,  %.6lf ms/size,  %lu bytes] \n", operation.c_str(), std::to_string(numParties).c_str(), ring_size, size, num_iterations, (double)(timer * 0.001) / num_iterations, (double)(timer * 0.001 / size) / num_iterations, 
    0 / num_iterations);

    for (size_t i = 0; i < numShares; i++) {
        delete[] a[i];
        delete[] b[i];
        delete[] c[i];
    }
    delete[] a;
    delete[] b;
    delete[] c;
}

void SMC_Utils::smc_test_rss(priv_int *A, int *B, int size, int threadID) {
    size = 5; //  testing only so I dont have to keep opening rss_main.cpp
    // uint bitlength = 20;

    uint numShares = ss->getNumShares();
    uint totalNumShares = ss->getTotalNumShares();
    uint ring_size = ss->ring_size;
    uint bytes = (ss->ring_size + 7) >> 3;
    printf("bytes : %u\n", bytes);
    printf("ring_size : %u\n", ring_size);
    printf("size : %u\n", size);
    printf("----\n\n");

    __m128i *key_prg;
    uint8_t key_raw[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
    key_prg = offline_prg_keyschedule(key_raw);
    uint8_t k1[] = {0x31, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
    uint8_t k2[] = {0xa1, 0x34, 0x6f, 0x67, 0x10, 0x1b, 0x13, 0xa3, 0x56, 0x45, 0x90, 0xb2, 0x13, 0xe3, 0x23, 0x24};

    std::vector<std::vector<int>> share_mapping;
    int numPeers = ss->getPeers();
    switch (numPeers) {
    case 3:
        share_mapping = {
            {1, 2}, // p1
            {2, 0}, // p2
            {0, 1}, // p3
        };
        break;
    case 5:
        share_mapping = {
            {4, 5, 6, 7, 8, 9}, // p1
            {7, 8, 1, 9, 2, 3}, // p2
            {9, 2, 5, 3, 6, 0}, // p3
            {3, 6, 8, 0, 1, 4}, // p4
            {0, 1, 2, 4, 5, 7}  // p5
        };
        break;
    case 7:
        share_mapping = {
            {15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34}, // p1
            {25, 26, 27, 5, 28, 29, 6, 30, 7, 8, 31, 32, 9, 33, 10, 11, 34, 12, 13, 14},      // p2
            {31, 32, 9, 19, 33, 10, 20, 11, 21, 1, 34, 12, 22, 13, 23, 2, 14, 24, 3, 4},      // p3
            {34, 12, 22, 28, 13, 23, 29, 2, 6, 16, 14, 24, 30, 3, 7, 17, 4, 8, 18, 0},        // p4
            {14, 24, 30, 33, 3, 7, 10, 17, 20, 26, 4, 8, 11, 18, 21, 27, 0, 1, 5, 15},        // p5
            {4, 8, 11, 13, 18, 21, 23, 27, 29, 32, 0, 1, 2, 5, 6, 9, 15, 16, 19, 25},         // p6
            {0, 1, 2, 3, 5, 6, 7, 9, 10, 12, 15, 16, 17, 19, 20, 22, 25, 26, 28, 31}          // p7
        };
        break;
    default:
        fprintf(stderr, "ERROR (rss_setup): invalid number of parties, only n = {3, 5, 7} is supported for RSS \n");
        exit(1);
    }
    uint8_t **Data1_byte = new uint8_t *[totalNumShares];
    uint8_t **Data2_byte = new uint8_t *[totalNumShares];

    priv_int_t **Data1 = new priv_int_t *[totalNumShares];
    priv_int_t **Data2 = new priv_int_t *[totalNumShares];
    for (int i = 0; i < totalNumShares; i++) {
        Data1[i] = new priv_int_t[size];
        memset(Data1[i], 0, sizeof(priv_int_t) * size);
        Data2[i] = new priv_int_t[size];
        memset(Data2[i], 0, sizeof(priv_int_t) * size);

        Data1_byte[i] = new uint8_t[size];
        memset(Data1_byte[i], 0, sizeof(uint8_t) * size);
        Data2_byte[i] = new uint8_t[size];
        memset(Data2_byte[i], 0, sizeof(uint8_t) * size);
    }
    uint bitlength = 12;

    for (int i = 0; i < size; i++) {
        for (size_t j = 0; j < totalNumShares - 1; j++) {
            prg_aes_ni(Data1[j] + i, k1, key_prg);
            // Data1[j][i] = GET_BIT(Data1[j][i], priv_int_t(0));
            prg_aes_ni(Data2[j] + i, k2, key_prg);
            // Data2[j][i] = GET_BIT(Data2[j][i], priv_int_t(0));
        }
        // Data1[totalNumShares - 1][i] = ( (-1) * i ) & ss->SHIFT[bitlength];
        Data1[totalNumShares - 1][i] = 10 * (i + 1) + 1;
        // Data1[totalNumShares - 1][i] = i;
        Data1_byte[totalNumShares - 1][i] = i;
        // Data1[totalNumShares - 1][i] = ((-1) * i);
        // Data1[totalNumShares - 1][i] = 6 + i;
        // Data2[totalNumShares - 1][i] = priv_int_t(-1) >> 1;
        // Data2[totalNumShares - 1][i] = 1 + i;
        Data2[totalNumShares - 1][i] = 2;
        Data2_byte[totalNumShares - 1][i] = i;
        for (size_t j = 0; j < totalNumShares - 1; j++) {
            Data1[totalNumShares - 1][i] -= Data1[j][i];
            Data2[totalNumShares - 1][i] -= Data2[j][i];
            // Data2[totalNumShares - 1][i] ^= Data2[j][i];
            // Data1[totalNumShares - 1][i] ^= GET_BIT(Data1[j][i], priv_int_t(0)); // only want a single bit
            // Data2[totalNumShares - 1][i] ^= GET_BIT(Data2[j][i], priv_int_t(0)); // only want a single bit
        }
    }

    // for (size_t i = 0; i < size; i++) {
    //     for (size_t s = 0; s < totalNumShares; s++) {
    //         printf("Data1[%lu][%lu]: %u \n", i, s, Data1[s][i]);
    //         printf("Data2[%lu][%lu]: %u \n", i, s, Data2[s][i]);
    //     }
    //     printf("\n");
    // }

    // expected RSS initialization

    priv_int result = new priv_int_t[size];
    memset(result, 0, sizeof(priv_int_t) * size);

    priv_int result_2 = new priv_int_t[size];
    memset(result_2, 0, sizeof(priv_int_t) * size);

    priv_int result_3 = new priv_int_t[size];
    memset(result_3, 0, sizeof(priv_int_t) * size);

    uint8_t *result_byte = new uint8_t[size];
    memset(result_byte, 0, sizeof(uint8_t) * size);

    uint8_t *result_byte_2 = new uint8_t[size];
    memset(result_byte_2, 0, sizeof(uint8_t) * size);
    uint8_t *result_byte_3 = new uint8_t[size];
    memset(result_byte_3, 0, sizeof(uint8_t) * size);

    priv_int *B_sparse = new priv_int[ss->getNumShares()];
    priv_int *a = new priv_int[ss->getNumShares()];
    priv_int *b = new priv_int[ss->getNumShares()];
    uint8_t **a_byte = new uint8_t *[ss->getNumShares()];
    uint8_t **b_byte = new uint8_t *[ss->getNumShares()];

    uint8_t **C_byte = new uint8_t *[ss->getNumShares()];
    priv_int *C = new priv_int[ss->getNumShares()];
    priv_int *D = new priv_int[ss->getNumShares()];
    priv_int *A_bit = new priv_int[ss->getNumShares()];
    priv_int *B_bit = new priv_int[ss->getNumShares()];
    for (int i = 0; i < ss->getNumShares(); i++) {
        B_sparse[i] = new priv_int_t[size];
        memset(B_sparse[i], 0, sizeof(priv_int_t) * size);
        C[i] = new priv_int_t[size];
        memset(C[i], 0, sizeof(priv_int_t) * size);
        D[i] = new priv_int_t[size];
        memset(D[i], 0, sizeof(priv_int_t) * size);

        C_byte[i] = new uint8_t[size];
        memset(C[i], 0, sizeof(uint8_t) * size);
    }

    for (size_t i = 0; i < numShares; i++) {
        a[i] = Data1[share_mapping[id - 1][i]];
        b[i] = Data2[share_mapping[id - 1][i]];
        a_byte[i] = Data1_byte[share_mapping[id - 1][i]];
        b_byte[i] = Data2_byte[share_mapping[id - 1][i]];
    }

    printf("Intdiv\n");
    doOperation_IntDiv(C, a, b, bitlength, size, -1, net, ss);

    Open(result, C, size, -1, net, ss);
    Open(result_2, a, size, -1, net, ss);
    Open(result_3, b, size, -1, net, ss);

    printf("\n");
    for (size_t i = 0; i < size; i++) {
        // printf("(input a)     [%lu]: %i\n", i, (int)result_2[i]);
        // // print_binary((result_2[i]), ring_size);

        // printf("(input b)     [%lu]: %i\n", i, (int)result_3[i]);
        // // print_binary((result_3[i]), ring_size);

        printf("(a / b)     [%lu]: %i / %i = %i\t", i, (int)result_2[i], (int)result_3[i], (int)result[i]);
        printf("(off by) %i\n", (int)result[i] - (int)result_2[i] / (int)result_3[i]);
        // print_binary(result[i], ring_size);
        // printf("\n");
    }

    // uint k = 3;
    // uint num_bits = (1 << k) * size; // exact number of bits in the output
    // uint num_uints = (num_bits + 7) >> 3;
    // std::cout << "k : " << k << std::endl;
    // std::cout << "num_bits : " << num_bits << std::endl;
    // std::cout << "num_uints : " << num_uints << std::endl;

    // priv_int_t **ao_res = new priv_int_t *[numShares];
    // for (size_t i = 0; i < numShares; i++)
    //     ao_res[i] = new priv_int_t[num_bits];

    // AllOr(b, k, ao_res, size, -1, net, ss);

    // priv_int_t *res_8 = new priv_int_t[num_bits];
    // memset(res_8, 0, sizeof(priv_int_t) * num_bits);

    // Open_Bitwise(res_8, ao_res, num_bits, -1, net, ss);
    // Open_Bitwise(result, b, size, -1, net, ss);

    // for (size_t i = 0; i < size; i++) {
    //     printf("(open) b   [%lu]: %u\t", i, result[i]);
    //     print_binary(result[i], ring_size);
    // }

    // for (size_t i = 0; i < num_bits; i++) {
    //     printf("(open) ao_res   [%lu]: %u\t", i, res_8[i]);
    //     print_binary(res_8[i], 8);
    //     if (((i + 1) % (1 << k)) == 0) {
    //         printf("\n");
    //     }
    // }

    // for (size_t i = 0; i < numShares; i++) {
    //     delete[] ao_res[i];
    // }
    // delete[] ao_res;

    // printf("multbyte\n");
    // Mult_Byte(C_byte, a_byte, b_byte, size, net, ss);
    // Open_Byte(result_byte, C_byte, size, -1, net, ss);
    // Open_Byte(result_byte_2, a_byte, size, -1, net, ss);
    // Open_Byte(result_byte_3, b_byte, size, -1, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     if (!(result[i] == (result_3[i] & result_2[i]))) {
    //         printf("(open) [a*b] (z2)  [%lu]: %u\n", i, result[i]);
    //         printf("expected %u\n", result_3[i] & result_2[i]);
    //         print_binary(result[i], ring_size);
    //     }
    // }

    // // Mult_Bitwise(C, a, b, size, net, ss);
    // // Open_Bitwise(result, C, size, -1, net, ss);
    // // Open_Bitwise(result_2, a, size, -1, net, ss);
    // // Open_Bitwise(result_3, b, size, -1, net, ss);
    // // for (size_t i = 0; i < size; i++) {
    // //     if (!(result[i] == (result_3[i] & result_2[i]))) {
    // //         printf("(open) [a*b] (z2)  [%lu]: %u\n", i, result[i]);
    // //         printf("expected %u\n", result_3[i] & result_2[i]);
    // //         print_binary(result[i], ring_size);
    // //     }
    // // }
    // // return;

    // for (size_t i = 0; i < numShares; i++) {
    //     A_bit[i] = Data1[share_mapping[id - 1][i]];
    //     B_bit[i] = Data2[share_mapping[id - 1][i]];
    // }
    // Open(result_2, a, size, -1, net, ss);
    // Open(result_3, b, size, -1, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     // printf("(open) c   [%lu]: %u\n", i, result[i]);
    //     printf("(open) a   [%lu]: %u\n", i, result_2[i]);
    //     printf("(open) b   [%lu]: %u\n", i, result_3[i]);
    // }

    // int m = 10;
    // doOperation_Trunc(C, b, ring_size, m, size, -1, net, ss);
    // Open(result_2, C, size, -1, net, ss);
    // Open(result, b, size, -1, net, ss);
    // // printf("\n");
    // for (size_t i = 0; i < size; i++) {
    //     if (!(result_2[i] == (result[i] >> priv_int_t(m)))) {
    //         printf("trunc  ERROR\n");
    //         printf("(open)  input     [%lu]: %u\n", i, result[i]);
    //         print_binary(result[i], ring_size);
    //         printf("(open)  trunc res [%lu]: %u\n", i, result_2[i]);
    //         print_binary(result_2[i], ring_size);
    //         printf("(open)  expected  [%lu]: %u\n", i, result[i] >> priv_int_t(m));
    //         print_binary(result[i] >> priv_int_t(m), ring_size);
    //     }
    // }

    // printf("\n");
    // // Rss_BitDec(C, a, size, ring_size, net, ss);
    // doOperation_EQZ(a, C, ring_size, size, -1, net, ss);
    // Open(result, C, size, -1, net, ss);
    // Open(result_2, a, size, -1, net, ss);
    // // Open(result_3, b, size, -1, net, ss);

    // for (size_t i = 0; i < size; i++) {
    //     printf("(open) c   [%lu]: %u\n", i, result[i]);
    //     // print_binary(result[i], ring_size);
    //     printf("(open) a   [%lu]: %u\n", i, result_2[i]);
    //     // print_binary(result_2[i], ring_size);
    //     if (!(result[i] == (result_2[i] == 0))) {
    //         printf("EQZ ERROR\n");

    //         // printf("(open ) c   [%lu]: %u\n", i, result[i]);
    //         // printf("(open ) a   [%lu]: %u\n", i, result_2[i]);
    //         // printf("(open ) b   [%lu]: %u\n", i, result_3[i]);
    //     }
    // }

    // uint m = ceil(log2(ring_size)); // rounding up to the nearest integer

    // Rss_BitDec(C, a, m, size, ring_size, net, ss);
    // Open_Bitwise(result, C, size, m, -1, net, ss);
    // Open(result_2, a, size, -1, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     printf("(open) a   [%lu]: %u\n", i, result_2[i]);
    //     if (!(result[i] == (result_2[i]))) {
    //         printf("BitDec ERROR\n");
    //         printf("(open) c   [%lu]: %u\t", i, result[i]);
    //         print_binary(result[i], ring_size);
    //         printf("(open) a   [%lu]: %u\t", i, result_2[i]);
    //         print_binary(result_2[i], ring_size);
    //     }
    // }

    // doOperation_LT(C, b, a, ring_size, ring_size, ring_size, size, -1, net, ss);
    // Open(result, C, size, -1, net, ss);
    // Open(result_2, a, size, -1, net, ss);
    // Open(result_3, b, size, -1, net, ss);

    // for (size_t i = 0; i < size; i++) {
    //     // printf("(open) c   [%lu]: %u\n", i, result[i]);
    //     // printf("(open) a   [%lu]: %u\n", i, result_2[i]);
    //     // printf("(open) b   [%lu]: %u\n", i, result_3[i]);
    //     if (!(result[i] == ((int)result_2[i] > (int)result_3[i]))) {
    //         printf("LT ERROR\n");

    //         printf("(open ) c   [%lu]: %u\n", i, result[i]);
    //         printf("(open ) a   [%lu]: %u\n", i, result_2[i]);
    //         printf("(open ) b   [%lu]: %u\n", i, result_3[i]);
    //     }
    // }

    // Rss_RandBit(C, size, ring_size, net, ss);
    // Open(result, C, size, -1, net, ss);
    // int ctr = 0;
    // for (size_t i = 0; i < size; i++) {
    //     // printf("(RandBit, open, Z_2k) r   [%lu]: %u\n", i, result[i]);
    //     if (result[i] == 0)
    //         ctr += 1;
    // }
    // std::cout << "percentage of 0's: " << 100 * float(ctr) / float(size) << "  percentage of 1's : " << 100 * float(size - ctr) / float(size) << std::endl;

    // Rss_MSB(C, a, size, ring_size, net, ss);
    // Open(result, C, size, -1, net, ss);
    // Open(result_2, a, size, -1, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     priv_int_t check = GET_BIT(result_2[i], ring_size - 1);
    //     if (!(result[i] == check)) {
    //         printf("MSB ERROR\n");
    //     }
    // }

    // edaBit(C, D, 8, size, ring_size, net, ss);
    // Open(result, C, size, -1, net, ss);
    // Open_Bitwise(result_2, D, size, -1, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     if (result[i] != result_2[i]) {
    //         printf("edabit ERROR\n");

    //         printf("(open, Z_2k) r   [%lu]: %u\t", i, result[i]);
    //         print_binary(result[i], ring_size);
    //         printf("(open, Z_2) bits [%lu]: %u\t", i, result_2[i]);
    //         print_binary(result_2[i], ring_size);
    //     }
    // }

    // edaBit(C, D, ring_size, size, ring_size, net, ss);
    // Open(result, C, size, -1, net, ss);
    // Open_Bitwise(result_2, D, size, -1, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     if (result[i] != result_2[i]) {
    //         printf("edabit ERROR\n");
    //         printf("(open, Z_2k) r   [%lu]: %u\t", i, result[i]);
    //         print_binary(result[i], ring_size);
    //         printf("(open, Z_2) bits [%lu]: %u\t", i, result_2[i]);
    //         print_binary(result_2[i], ring_size);
    //     }
    // }
    // for (size_t s = 0; s < numShares; s++) {
    //     for (size_t i = 0; i < size; i++) {
    //         printf("A_bit[%lu][%lu]: %u \n", i, s, A_bit[s][i]);
    //         printf("B_bit[%lu][%lu]: %u \n", s,i, B_bit[s][i]);
    //     }
    //     printf("\n");
    // }

    // Open_Bitwise(result, A_bit, size, -1, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     printf("(open, Z_2) A_bit [%lu]: %u\n", i, result[i]);
    // }

    // printf("\n");
    // Rss_B2A(C, A_bit, ring_size, size, net, ss);

    // smc_open(result, C, size, -1);
    // for (size_t i = 0; i < size; i++) {
    //     printf("(open, Z_k) A_bit [%lu]: %u\n", i, result[i]);
    // }
    // printf("\n");

    // Open_Bitwise(result, B_bit, size, -1, net, ss);
    // for (size_t i = 0; i < size; i++) {
    //     printf("(open, Z_2) B_bit [%lu]: %u\n", i, result[i]);
    // }

    // Rss_B2A(C, B_bit, ring_size, size, net, ss);
    // printf("\n");

    // smc_open(result, C, size, -1);
    // for (size_t i = 0; i < size; i++) {
    //     printf("(open, Z_k) B_bit [%lu]: %u\n", i, result[i]);
    // }

    // ss->sparsify(B_sparse, B, size);

    // for (size_t i = 0; i < size; i++) {
    //     for (size_t s = 0; s < numShares; s++) {
    //         printf("B_sparse[%lu][%lu]: %u \n", i, s, B_sparse[s][i]);
    //     }
    // }
    // smc_open(result, B_sparse, size, -1);
    // for (size_t i = 0; i < size; i++) {
    //     printf("(open) B_sparse [%lu]: %u\n", i, result[i]);
    // }
    // Mult_Sparse(C, B_sparse, B_sparse, size, net, ss);
    // Mult(C, A, B_sparse, size, net, ss);

    // for (size_t i = 0; i < size; i++) {
    //     for (size_t s = 0; s < numShares; s++) {
    //         printf("C[%lu][%lu]: %u \n", i, s, C[s][i]);
    //     }
    //     printf("\n");
    // }

    // smc_open(result, C, size, -1);
    // printf("\n");
    // for (size_t i = 0; i < size; i++) {
    //     printf("(open) C [%lu]: %u\n", i, result[i]);
    // }
    // printf("\n");

    //    Rss_Mult_Sparse(C, A, B_sparse, size, net, ss);
    //     // Mult(C, A, B_sparse, size, net, ss);
    //     smc_open(result, C, size, -1);
    //     printf("\n");
    //     for (size_t i = 0; i < size; i++) {
    //         printf("(open) C [%lu]: %u\n", i, result[i]);
    //     }
    //     printf("\n");

    // vector<int> input_parties = {1 };
    // vector<int> input_parties = {0};
    // uint numInputParties = input_parties.size();

    // priv_int_t ***res = new priv_int_t **[numInputParties];
    // for (size_t j = 0; j < numInputParties; j++) {
    //     res[j] = new priv_int_t *[numShares];
    //     for (size_t s = 0; s < numShares; s++) {
    //         res[j][s] = new priv_int_t[size];
    //         memset(res[j][s], 0, sizeof(priv_int_t) * size);
    //     }
    // }

    // cant use pid_in_T because that is only for fixed vector sizes:
    // n = 3 -> len(T) = 1
    // n = 5 -> len(T) = 2
    // n = 7 -> len(T) = 3

    // if (std::find(input_parties.begin(), input_parties.end(), id) != input_parties.end()) {
    //     std::cout << id << " is an input party" << std::endl;
    //     Rss_Input_p_star(res, reinterpret_cast<priv_int_t *>(B), input_parties, size, ring_size, net, ss);
    // } else {
    //     std::cout << id << " is NOT an input party" << std::endl;
    //     // priv_int placeholder = nullptr;
    //     Rss_Input_p_star(res, static_cast<priv_int>(nullptr), input_parties, size, ring_size, net, ss);
    // }

    // for (size_t i = 0; i < numInputParties; i++) {
    //     smc_open(result, res[i], size, -1);
    //     for (size_t j = 0; j < size; j++) {
    //         printf("(open) party %i's input  [%lu] %u\n", input_parties[i], j, result[j]);
    //     }
    //     // printf("\n");
    // }

    // std::vector<std::vector<int>> send_recv_map = ss->generateInputSendRecvMap(input_parties);
    // printf("---\nsend_recv_map\n");
    // for (auto var : send_recv_map) {
    //     std::cout << var << std::endl;
    // }

    for (size_t i = 0; i < totalNumShares; i++) {
        delete[] Data1[i];
        delete[] Data2[i];
    }

    delete[] Data1;
    delete[] Data2;
    delete[] A_bit;
    delete[] B_bit;

    for (size_t i = 0; i < numShares; i++) {
        delete[] C[i];
        delete[] D[i];
        delete[] B_sparse[i];
    }
    delete[] C;
    delete[] D;
    delete[] B_sparse;
    delete[] result;
    delete[] result_2;

    // for (size_t i = 0; i < numInputParties; i++) {
    //     for (size_t s = 0; s < numShares; s++) {
    //         delete[] res[i][s];
    //     }
    //     delete[] res[i];
    // }
    // delete[] res;
}
#endif

