/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame
   ** Department of Computer Science and Engineering, University of Buffalo (SUNY)

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
#include "SecretShare.h"
#include "openssl/bio.h"
#include "unistd.h"
#include <cmath>
#include <netinet/in.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <string>

// Constructors
SMC_Utils::SMC_Utils(int id, std::string runtime_config, std::string privatekey_filename, int numOfInputPeers, int numOfOutputPeers, std::string *IO_files, int numOfPeers, int threshold, int bits, std::string mod, int num_threads) {
    // test_type(15);

    std::cout << "SMC_Utils constructor\n";
    mpz_t modulus;
    mpz_init(modulus);
    mpz_set_str(modulus, mod.c_str(), BASE_10);
    nodeConfig = new NodeConfiguration(id, runtime_config, bits);
    peers = numOfPeers;
    std::cout << "Creating the NodeNetwork\n";

    NodeNetwork *nodeNet = new NodeNetwork(nodeConfig, privatekey_filename, num_threads);
    net = *nodeNet;

    std::cout << "Creating SecretShare\n";

    clientConnect();
    receivePolynomials(privatekey_filename);

    ss = new SecretShare(numOfPeers, threshold, modulus, id, num_threads, net.getPRGseeds(), polynomials);

    // initialize input and output streams
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
    // if (peers == 3) {
    //     ss->getCoef(id);
    //     ss->Seed((nodeNet->key_0), (nodeNet->key_1));
    // }
    // setCoef();
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
    ss->ss_input(id, var, type, inputStreams);
}

// for float variable I/O
void SMC_Utils::smc_input(int id, float *var, std::string type, int threadID) {
    ss->ss_input(id, var, type, inputStreams);
}

void SMC_Utils::smc_input(int id, mpz_t **var, std::string type, int threadID) {
    ss->ss_input(id, var, type, inputStreams);
}

// one-dimensional int array I/O
void SMC_Utils::smc_input(int id, mpz_t *var, int size, std::string type, int threadID) {
    ss->ss_input(id, var, size, type, inputStreams);
}

void SMC_Utils::smc_input(int id, int *var, int size, std::string type, int threadID) {
    ss->ss_input(id, var, size, type, inputStreams);
}

// one-dimensional float array I/O
void SMC_Utils::smc_input(int id, mpz_t **var, int size, std::string type, int threadID) {
    ss->ss_input(id, var, size, type, inputStreams);
}

void SMC_Utils::smc_input(int id, float *var, int size, std::string type, int threadID) {
    ss->ss_input(id, var, size, type, inputStreams);
}

void SMC_Utils::smc_output(int id, int *var, std::string type, int threadID) {
    ss->ss_output(id, var, type, outputStreams);
}

void SMC_Utils::smc_output(int id, mpz_t *var, std::string type, int threadID) {
    ss->ss_output(id, var, type, outputStreams);
}

void SMC_Utils::smc_output(int id, float *var, std::string type, int threadID) {
    ss->ss_output(id, var, type, outputStreams);
}

void SMC_Utils::smc_output(int id, mpz_t **var, std::string type, int threadID) {
    ss->ss_output(id, var, type, outputStreams);
}

void SMC_Utils::smc_output(int id, mpz_t *var, int size, std::string type, int threadID) {
    ss->ss_output(id, var, size, type, outputStreams);
}

void SMC_Utils::smc_output(int id, int *var, int size, std::string type, int threadID) {
    ss->ss_output(id, var, size, type, outputStreams);
}

void SMC_Utils::smc_output(int id, mpz_t **var, int size, std::string type, int threadID) {
    ss->ss_output(id, var, type, outputStreams);
}

void SMC_Utils::smc_output(int id, float *var, int size, std::string type, int threadID) {
    ss->ss_output(id, var, size, type, outputStreams);
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
    // mpz_t *sub = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // for (int i = 0; i < size; ++i)
    //     mpz_init(sub[i]);
    // int len = smc_compute_len(alen, blen);
    // ss->modSub(sub, a, b, size);
    // Lt->doOperation(result, sub, len, size, threadID);
    // ss_batch_free_operator(&sub, size);
}

void SMC_Utils::smc_lt(mpz_t **a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    ss_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "<0", threadID, net, id, ss);
}

void SMC_Utils::smc_gt(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(b), MPZ_CAST(a), blen, alen, resultlen, 1, threadID, net, id, ss);
    // mpz_t sub;
    // mpz_init(sub);
    // ss->modSub(sub, b, a);

    // mpz_t *results = (mpz_t *)malloc(sizeof(mpz_t));
    // mpz_t *subs = (mpz_t *)malloc(sizeof(mpz_t));
    // mpz_init_set(subs[0], sub);
    // mpz_init(results[0]);

    // /********************************************/
    // int len = smc_compute_len(alen, blen);
    // /********************************************/
    // Lt->doOperation(results, subs, len, 1, threadID);
    // mpz_set(result, results[0]);

    // // free the memory
    // mpz_clear(sub);
    // ss_batch_free_operator(&subs, 1);
    // ss_batch_free_operator(&results, 1);
}

void SMC_Utils::smc_gt(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), (int *)&b, MPZ_CAST(a), blen, alen, resultlen, 1, threadID, net, id, ss);
    // mpz_t bs;
    // mpz_init_set_si(bs, b);
    // smc_gt(a, bs, result, alen, blen, resultlen, type, threadID);
    // mpz_clear(bs);
}

void SMC_Utils::smc_gt(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(b), (int *)&a, blen, alen, resultlen, 1, threadID, net, id, ss);
    // mpz_t as;
    // mpz_init_set_si(as, a);
    // smc_gt(as, b, result, alen, blen, resultlen, type, threadID);
    // mpz_clear(as);
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
    // switching order of parameters for LT will compute GT

    // mpz_t *sub = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // for (int i = 0; i < size; ++i)
    //     mpz_init(sub[i]);
    // int len = smc_compute_len(alen, blen);
    // ss->modSub(sub, b, a, size);
    // Lt->doOperation(result, sub, len, size, threadID);
    // ss_batch_free_operator(&sub, size);
}

void SMC_Utils::smc_gt(mpz_t **a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    ss_batch_fop_comparison(result, b, a, resultlen, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID, net, id, ss);
}

void SMC_Utils::smc_leq(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    // Lt->doOperation_LEQ(result, a, b, alen, blen, resultlen, threadID);
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(b), MPZ_CAST(a), blen, alen, resultlen, 1, threadID, net, id, ss);
    ss->modSub(result, 1, result);

    //     mpz_t sub;
    //     mpz_init(sub);
    //     ss->modSub(sub, b, a);

    //     mpz_t *results = (mpz_t *)malloc(sizeof(mpz_t));
    //     mpz_t *subs = (mpz_t *)malloc(sizeof(mpz_t));
    //     mpz_init_set(subs[0], sub);
    //     mpz_init(results[0]);

    //     int len = smc_compute_len(alen, blen);
    //     Lt->doOperation(results, subs, len, 1, threadID);
    //     mpz_set(result, results[0]);
    //     ss->modSub(result, 1, result);

    //     // free the memory
    //     mpz_clear(sub);
    //     ss_batch_free_operator(&results, 1);
    //     ss_batch_free_operator(&subs, 1);
}

void SMC_Utils::smc_leq(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), (int *)&b, MPZ_CAST(a), blen, alen, resultlen, 1, threadID, net, id, ss);
    ss->modSub(result, 1, result);

    // mpz_t bs;
    // mpz_init_set_si(bs, b);
    // smc_leq(a, bs, result, alen, blen, resultlen, type, threadID);
    // mpz_clear(bs);
}

void SMC_Utils::smc_leq(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(b), (int *)&a, blen, alen, resultlen, 1, threadID, net, id, ss);
    ss->modSub(result, 1, result);

    // mpz_t as;
    // mpz_init_set_si(as, a);
    // smc_leq(as, b, result, alen, blen, resultlen, type, threadID);
    // mpz_clear(as);
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
    // Lt->doOperation_LEQ(result, a, b, alen, blen, resultlen, size, threadID);
    doOperation_LT(result, b, a, blen, alen, resultlen, size, threadID, net, id, ss);
    ss->modSub(result, 1, result, size);
    // mpz_t *sub = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // for (int i = 0; i < size; i++)
    //     mpz_init(sub[i]);
    // ss->modSub(sub, b, a, size);
    // int len = smc_compute_len(alen, blen);
    // Lt->doOperation(result, sub, len, size, threadID);
    // ss->modSub(result, 1, result, size);
    // ss_batch_free_operator(&sub, size);
}

void SMC_Utils::smc_leq(mpz_t **a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t *result, int resultlen, int size, std::string type, int threadID) {
    ss_batch_fop_comparison(result, b, a, resultlen, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID, net, id, ss);
    ss->modSub(result, 1, result, size);
}

void SMC_Utils::smc_geq(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(a), MPZ_CAST(b), alen, blen, resultlen, 1, threadID, net, id, ss);
    ss->modSub(result, 1, result);
    // mpz_t sub;
    // mpz_init(sub);
    // ss->modSub(sub, a, b);

    // mpz_t *results = (mpz_t *)malloc(sizeof(mpz_t));
    // mpz_t *subs = (mpz_t *)malloc(sizeof(mpz_t));
    // mpz_init_set(subs[0], sub);
    // mpz_init(results[0]);

    // int len = smc_compute_len(alen, blen);
    // Lt->doOperation(results, subs, len, 1, threadID);
    // mpz_set(result, results[0]);
    // ss->modSub(result, 1, result);

    // mpz_clear(sub);
    // ss_batch_free_operator(&results, 1);
    // ss_batch_free_operator(&subs, 1);
}

void SMC_Utils::smc_geq(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), MPZ_CAST(a), (int *)&b, alen, blen, resultlen, 1, threadID, net, id, ss);
    ss->modSub(result, 1, result);
    // mpz_t bs;
    // mpz_init_set_si(bs, b);
    // smc_geq(a, bs, result, alen, blen, resultlen, type, threadID);
    // mpz_clear(bs);
}

void SMC_Utils::smc_geq(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID) {
    doOperation_LT(MPZ_CAST(result), (int *)&a, MPZ_CAST(b), alen, blen, resultlen, 1, threadID, net, id, ss);
    ss->modSub(result, 1, result);
    // mpz_t as;
    // mpz_init_set_si(as, a);
    // smc_geq(as, b, result, alen, blen, resultlen, type, threadID);
    // mpz_clear(as);
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

// im assuming that result has some data stored in it already, otherwise it would just be over-written when an operiations was called
// void SMC_Utils::ss_batch_BOP_int(mpz_t *result, mpz_t *a, mpz_t *b, int resultlen, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int size, std::string op, std::string type, int threadID), net, id, ss {
//     mpz_t *result_org = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     for (int i = 0; i < size; i++)
//         ss_init_set(result_org[i], result[i]);
//     if (op == "*") {
//         smc_mult(a, b, alen, blen, result, resultlen, size, type, threadID);
//     } else if (op == "-") {
//         smc_sub(a, b, alen, blen, result, resultlen, size, type, threadID);
//     } else if (op == "+") {
//         smc_add(a, b, alen, blen, result, resultlen, size, type, threadID);
//     } else if (op == "==") {
//         smc_eqeq(a, b, alen, blen, result, resultlen, size, type, threadID);
//     } else if (op == "!=") {
//         smc_neq(a, b, alen, blen, result, resultlen, size, type, threadID);
//     } else if (op == ">") {
//         smc_gt(a, b, alen, blen, result, resultlen, size, type, threadID);
//     } else if (op == ">=") {
//         smc_geq(a, b, alen, blen, result, resultlen, size, type, threadID);
//     } else if (op == "<") {
//         smc_lt(a, b, alen, blen, result, resultlen, size, type, threadID);
//     } else if (op == "<=") {
//         smc_leq(a, b, alen, blen, result, resultlen, size, type, threadID);
//     } else if (op == "/") {
//         smc_div(a, b, alen, blen, result, resultlen, size, type, threadID);
//     } else if (op == "/P") {
//         doOperation_IntDiv_Pub(result, a, b, resultlen, size, threadID, net, id, ss);
//     } else if (op == "=") {
//         smc_set(a, result, alen, resultlen, size, type, threadID);
//     } else if (op == ">>") {
//         smc_shr(a, b, alen, blen, result, resultlen, size, type, threadID);
//     } else if (op == "<<") {
//         smc_shl(a, b, alen, blen, result, resultlen, size, type, threadID);
//     } else {
//         std::cout << "Unrecognized op: " << op << "\n";
//     }

//     ss_batch_handle_priv_cond(result, result_org, out_cond, priv_cond, counter, size, threadID, net, id, ss);
//     ss_batch_free_operator(&result_org, size);
// }

// void SMC_Utils::ss_batch_BOP_float_arithmetic(mpz_t **result, mpz_t **a, mpz_t **b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int size, std::string op, std::string type, int threadID), net, id, ss {
//     mpz_t **result_org = (mpz_t **)malloc(sizeof(mpz_t *) * size);
//     for (int i = 0; i < size; i++) {
//         result_org[i] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
//         for (int j = 0; j < 4; j++)
//             mpz_init_set(result_org[i][j], result[i][j]);
//     }

//     if (op == "*") {
//         smc_mult(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
//     } else if (op == "-") {
//         smc_sub(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
//     } else if (op == "+") {
//         smc_add(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
//     } else if (op == "/") {
//         smc_div(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
//     } else if (op == "/P") {
//         doOperation_FLDiv_Pub(a, b, result, alen_sig, size, threadID, net, id, ss);
//         ss_process_results(result, resultlen_sig, resultlen_exp, alen_sig, alen_exp, size, threadID, net, id, ss);
//     } else if (op == "=") {
//         smc_set(a, result, alen_sig, alen_exp, resultlen_sig, resultlen_exp, size, type, threadID);
//     }

//     mpz_t *result_unify = (mpz_t *)malloc(sizeof(mpz_t) * 4 * size);
//     mpz_t *result_org_unify = (mpz_t *)malloc(sizeof(mpz_t) * 4 * size);

//     for (int i = 0; i < size; i++) {
//         for (int j = 0; j < 4; j++) {
//             mpz_init_set(result_unify[4 * i + j], result[i][j]);
//             mpz_init_set(result_org_unify[4 * i + j], result_org[i][j]);
//         }
//     }

//     ss_batch_handle_priv_cond(result_unify, result_org_unify, out_cond, priv_cond, counter, 4 * size, threadID, net, id, ss);

//     for (int i = 0; i < size; i++)
//         for (int j = 0; j < 4; j++)
//             mpz_set(result[i][j], result_unify[4 * i + j]);

//     ss_batch_free_operator(&result_unify, 4 * size);
//     ss_batch_free_operator(&result_org_unify, 4 * size);
// }

// void SMC_Utils::ss_batch_BOP_float_comparison(mpz_t *result, mpz_t **a, mpz_t **b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID), net, id, ss {

//     mpz_t *result_org = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     mpz_t *result_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);

//     for (int i = 0; i < size; i++) {
//         mpz_init_set(result_org[i], result[index_array[3 * i + 2]]);
//         mpz_init(result_tmp[i]);
//     }

//     if (op == "==") {
//         smc_eqeq(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result_tmp, resultlen_sig, size, type, threadID);
//     } else if (op == "!=") {
//         smc_neq(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result_tmp, resultlen_sig, size, type, threadID);
//     } else if (op == ">") {
//         smc_gt(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result_tmp, resultlen_sig, size, type, threadID);
//     } else if (op == ">=") {
//         smc_geq(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result_tmp, resultlen_sig, size, type, threadID);
//     } else if (op == "<") {
//         smc_lt(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result_tmp, resultlen_sig, size, type, threadID);
//     } else if (op == "<=") {
//         smc_leq(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result_tmp, resultlen_sig, size, type, threadID);
//     }

//     ss_batch_handle_priv_cond(result_tmp, result_org, out_cond, priv_cond, counter, size, threadID, net, id, ss);

//     for (int i = 0; i < size; ++i)
//         mpz_set(result[index_array[3 * i + 2]], result_tmp[i]);

//     ss_batch_free_operator(&result_org, size);
//     ss_batch_free_operator(&result_tmp, size);
// }

/************************************ INTEGER BATCH ****************************************/
void SMC_Utils::smc_batch(mpz_t *a, mpz_t *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t *a_tmp, *b_tmp, *result_tmp;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    // ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    // for (int i = 0; i < size; ++i)
    //     mpz_set(result[index_array[3 * i + 2]], result_tmp[i]);

    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&b_tmp, size);
    // ss_batch_free_operator(&result_tmp, size);
}

// used to compute 1-priv_cond in a batch stmt
void SMC_Utils::smc_batch(int a, mpz_t *b, mpz_t *result, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, int threadID) {
    ss_batch(a, b, result, out_cond, priv_cond, counter, index_array, size, op, threadID, net, id, ss);

    // mpz_t *a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // mpz_t *out_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);

    // for (int i = 0; i < size; i++) {
    //     mpz_init_set_ui(a_tmp[i], a);
    //     mpz_init(out_tmp[i]);
    // }

    // ss->modSub(result, a_tmp, b, size);

    // if (out_cond != NULL) {
    //     for (int i = 0; i < size; i++)
    //         mpz_set(out_tmp[i], out_cond);
    //     Mult(result, result, out_tmp, size, threadID, net, id, ss);
    // }

    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&out_tmp, size);
}

void SMC_Utils::smc_batch(mpz_t *a, mpz_t *b, mpz_t *result, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, int threadID) {
    ss_batch(a, b, result, out_cond, priv_cond, counter, index_array, size, op, threadID, net, id, ss);
    // if (counter == size)
    //     ss->modSub(result, a, b, size);
    // else {
    //     mpz_t *tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    //     for (int i = 0; i < size; i++)
    //         mpz_init_set(tmp[i], a[i / (size / counter)]);
    //     ss->modSub(result, tmp, b, size);
    //     ss_batch_free_operator(&tmp, size);
    // }
}

// first param: int array
// second param: int array
// third param: one-dim private int array
void SMC_Utils::smc_batch(int *a, int *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t *a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // mpz_t *b_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // for (int i = 0; i < size; i++) {
    //     mpz_init_set_si(a_tmp[i], a[i]);
    //     mpz_init_set_si(b_tmp[i], b[i]);
    // }

    // smc_batch(a_tmp, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);

    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&b_tmp, size);
}

// first param: int array
// second param: int array
// third param: two-dim private int array
void SMC_Utils::smc_batch(int *a, int *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    //     mpz_t *a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    //     mpz_t *b_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);

    //     for (int i = 0; i < size; i++) {
    //         mpz_init_set_si(a_tmp[i], a[i]);
    //         mpz_init_set_si(b_tmp[i], b[i]);
    //     }

    //     smc_batch(a_tmp, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    //     ss_batch_free_operator(&a_tmp, size);
    //     ss_batch_free_operator(&b_tmp, size);
}

// first param: int array
// second param: one-dim private int array
// third param: one-dim private int array
void SMC_Utils::smc_batch(int *a, mpz_t *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t *a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // for (int i = 0; i < size; i++)
    //     mpz_init_set_si(a_tmp[i], a[i]);
    // smc_batch(a_tmp, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&a_tmp, size);
}

// first param: int array
// second param: one-dim private int array
// third param: two-dim private int array
void SMC_Utils::smc_batch(int *a, mpz_t *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t *a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // for (int i = 0; i < size; i++)
    //     mpz_init_set_si(a_tmp[i], a[i]);
    // smc_batch(a_tmp, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&a_tmp, size);
}

// first param: one-dim private int array
// second param: int array
// third param: one-dim private int array
void SMC_Utils::smc_batch(mpz_t *a, int *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t *b_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // for (int i = 0; i < size; i++)
    //     mpz_init_set_si(b_tmp[i], b[i]);
    // op = (op == "/") ? "/P" : op;
    // smc_batch(a, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&b_tmp, size);
}

// first param: one-dim private int array
// second param: int array
// third param: two-dim private int array
void SMC_Utils::smc_batch(mpz_t *a, int *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t *b_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // for (int i = 0; i < size; i++)
    //     mpz_init_set_si(b_tmp[i], b[i]);
    // op = (op == "/") ? "/P" : op;
    // smc_batch(a, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&b_tmp, size);
}

// first param: integer array
// second param: two-dim private int
// assignment param: one-dim private int
void SMC_Utils::smc_batch(int *a, mpz_t **b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t *a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // for (int i = 0; i < size; i++)
    //     mpz_init_set_si(a_tmp[i], a[i]);
    // smc_batch(a_tmp, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&a_tmp, size);
}

// first param: integer array
// second param: two-dim private int
// assignment param: two-dim private int
void SMC_Utils::smc_batch(int *a, mpz_t **b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t *a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // for (int i = 0; i < size; i++)
    //     mpz_init_set_si(a_tmp[i], a[i]);
    // smc_batch(a_tmp, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&a_tmp, size);
}

// first param: two-dim private int
// second param: integer array
// assignment param: one-dim private int
void SMC_Utils::smc_batch(mpz_t **a, int *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t *b_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // for (int i = 0; i < size; i++)
    //     mpz_init_set_si(b_tmp[i], b[i]);
    // op = (op == "/") ? "/P" : op;
    // smc_batch(a, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&b_tmp, size);
}

// first param: two-dim private int
// second param: integer array
// assignment param: two-dim private int
void SMC_Utils::smc_batch(mpz_t **a, int *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t *b_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // for (int i = 0; i < size; i++)
    //     mpz_init_set_si(b_tmp[i], b[i]);
    // op = (op == "/") ? "/P" : op;
    // smc_batch(a, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // // free the memory
    // ss_batch_free_operator(&b_tmp, size);
}

// first param: one-dim private int
// second param: two-dim private int
// assignment param: two-dim private int
void SMC_Utils::smc_batch(mpz_t *a, mpz_t **b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // int dim1, dim2;
    // mpz_t *a_tmp, *b_tmp, *result_tmp;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    // ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    // if (resultdim != 0) {
    //     for (int i = 0; i < size; ++i) {
    //         dim1 = index_array[3 * i + 2] / resultdim;
    //         dim2 = index_array[3 * i + 2] % resultdim;
    //         mpz_set(result[dim1][dim2], result_tmp[i]);
    //     }
    // }

    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&b_tmp, size);
    // ss_batch_free_operator(&result_tmp, size);
}

// first param: two-dim private int
// second param: one-dim private int
// assignment param: two-dim private int
void SMC_Utils::smc_batch(mpz_t **a, mpz_t *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // int dim1, dim2;
    // mpz_t *a_tmp, *b_tmp, *result_tmp;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    // ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    // if (resultdim != 0) {
    //     for (int i = 0; i < size; ++i) {
    //         dim1 = index_array[3 * i + 2] / resultdim;
    //         dim2 = index_array[3 * i + 2] % resultdim;
    //         mpz_set(result[dim1][dim2], result_tmp[i]);
    //     }
    // }

    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&b_tmp, size);
    // ss_batch_free_operator(&result_tmp, size);
}

// first param: two-dim private int
// second param: two-dim private int
// assignment param: one-dim private int
void SMC_Utils::smc_batch(mpz_t **a, mpz_t **b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    // if (op == "@") {
    //     mpz_t **result_tmp = (mpz_t **)malloc(sizeof(mpz_t *));
    //     result_tmp[0] = (mpz_t *)malloc(sizeof(mpz_t) * resultdim);
    //     for (int i = 0; i < resultdim; i++)
    //         mpz_init_set(result_tmp[0][i], result[i]);
    //     smc_batch(a, b, result_tmp, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    //     for (int i = 0; i < resultdim; i++)
    //         mpz_set(result[i], result_tmp[0][i]);
    //     ss_batch_free_operator(&(result_tmp[0]), resultdim);
    //     free(result_tmp);
    //     return;
    // }
    // mpz_t *a_tmp, *b_tmp, *result_tmp;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    // ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    // for (int i = 0; i < size; i++)
    //     mpz_set(result[index_array[3 * i + 2]], result_tmp[i]);

    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&b_tmp, size);
    // ss_batch_free_operator(&result_tmp, size);
}

// first param: one-dim private int
// second param: one-dim private int
// assignment param: two-dim private int
void SMC_Utils::smc_batch(mpz_t *a, mpz_t *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    // int dim1, dim2;
    // mpz_t *a_tmp, *b_tmp, *result_tmp;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    // ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    // if (resultdim != 0) {
    //     for (int i = 0; i < size; ++i) {
    //         dim1 = index_array[3 * i + 2] / resultdim;
    //         dim2 = index_array[3 * i + 2] % resultdim;
    //         mpz_set(result[dim1][dim2], result_tmp[i]);
    //     }
    // }

    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&b_tmp, size);
    // ss_batch_free_operator(&result_tmp, size);
}
// first param: one-dim private int
// second param: two-dim private int
// assignment param: one-dim private int
void SMC_Utils::smc_batch(mpz_t *a, mpz_t **b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t *a_tmp, *b_tmp, *result_tmp;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    // ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    // for (int i = 0; i < size; i++)
    //     mpz_set(result[index_array[3 * i + 2]], result_tmp[i]);

    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&b_tmp, size);
    // ss_batch_free_operator(&result_tmp, size);
}

// first param: two-dim private int
// second param: one-dim private int
// assignment param: one-dim private int
void SMC_Utils::smc_batch(mpz_t **a, mpz_t *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t *a_tmp, *b_tmp, *result_tmp;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    // ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    // for (int i = 0; i < size; i++)
    //     mpz_set(result[index_array[3 * i + 2]], result_tmp[i]);

    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&b_tmp, size);
    // ss_batch_free_operator(&result_tmp, size);
}

void SMC_Utils::smc_batch_dot(mpz_t **a, mpz_t **b, int size, int array_size, int *index_array, mpz_t *result, int threadID) {
    ss_batch_dot(a, b, size, array_size, index_array, result, threadID, net, id, ss);

    // int a_dim = 0, b_dim = 0;
    // mpz_t **a_tmp = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    // mpz_t **b_tmp = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    // for (int i = 0; i < size; i++) {
    //     a_tmp[i] = (mpz_t *)malloc(sizeof(mpz_t) * array_size);
    //     b_tmp[i] = (mpz_t *)malloc(sizeof(mpz_t) * array_size);
    //     a_dim = index_array[3 * i];
    //     b_dim = index_array[3 * i + 1];
    //     for (int j = 0; j < array_size; j++) {
    //         mpz_init_set(a_tmp[i][j], a[a_dim][j]);
    //         mpz_init_set(b_tmp[i][j], b[b_dim][j]);
    //     }
    // }

    // smc_dot(a_tmp, b_tmp, size, array_size, result, "int", threadID);

    // // free the memory
    // for (int i = 0; i < size; i++) {
    //     for (int j = 0; j < array_size; j++) {
    //         mpz_clear(a_tmp[i][j]);
    //         mpz_clear(b_tmp[i][j]);
    //     }
    //     free(a_tmp[i]);
    //     free(b_tmp[i]);
    // }
    // free(a_tmp);
    // free(b_tmp);
}

// first param: two-dim private int
// second param: two-dim private int
// assignment param: two-dim private int

void SMC_Utils::smc_batch(mpz_t **a, mpz_t **b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // int dim1, dim2;
    // mpz_t *a_tmp, *b_tmp, *result_tmp, *result_org;
    // if (op == "@") {
    //     result_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    //     result_org = (mpz_t *)malloc(sizeof(mpz_t) * size);
    //     for (int i = 0; i < size; i++) {
    //         mpz_init(result_tmp[i]);
    //         mpz_init(result_org[i]);
    //         if (resultdim != 0) {
    //             dim1 = index_array[3 * i + 2] / resultdim;
    //             dim2 = index_array[3 * i + 2] % resultdim;
    //             mpz_set(result_org[i], result[dim1][dim2]);
    //         }
    //     }
    //     smc_batch_dot(a, b, size, adim, index_array, result_tmp, threadID);
    //     ss_batch_handle_priv_cond(result_tmp, result_org, out_cond, priv_cond, counter, size, threadID, net, id, ss);
    // } else {
    //     ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    //     ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    //     ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);
    //     ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);
    // }

    // if (resultdim != 0) {
    //     for (int i = 0; i < size; ++i) {
    //         dim1 = index_array[3 * i + 2] / resultdim;
    //         dim2 = index_array[3 * i + 2] % resultdim;
    //         mpz_set(result[dim1][dim2], result_tmp[i]);
    //     }
    // }

    // if (op != "@") {
    //     ss_batch_free_operator(&a_tmp, size);
    //     ss_batch_free_operator(&b_tmp, size);
    //     ss_batch_free_operator(&result_tmp, size);
    // } else {
    //     ss_batch_free_operator(&result_org, size);
    //     ss_batch_free_operator(&result_tmp, size);
    // }
}

/*********************************************** FLOAT BATCH ****************************************************/
// public + private one-dimension float - arithmetic
void SMC_Utils::smc_batch(float *a, mpz_t **b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **atmp;
    // ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    // smc_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&atmp, size);
}

void SMC_Utils::smc_batch(mpz_t **a, float *b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **btmp;
    // ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    // op = (op == "/") ? "/P" : op;
    // smc_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&btmp, size);
}

void SMC_Utils::smc_batch(float *a, mpz_t **b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **atmp;
    // ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    // smc_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&atmp, size);
}

void SMC_Utils::smc_batch(mpz_t **a, float *b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **btmp;
    // ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    // op = (op == "/") ? "/P" : op;
    // smc_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&btmp, size);
}
// public to private assignments
void SMC_Utils::smc_batch(float *a, float *b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // // mainly used for assignments, which means b will not be necessary
    // mpz_t **atmp;
    // ss_batch_convert_to_private_float(a, &atmp, resultlen_sig, resultlen_exp, size, ss);
    // smc_batch(atmp, atmp, result, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&atmp, size);
}

void SMC_Utils::smc_batch(int *a, int *b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // float *a1 = (float *)malloc(sizeof(float) * size);
    // for (int i = 0; i < size; i++)
    //     a1[i] = a[i];
    // mpz_t **atmp;
    // ss_batch_convert_to_private_float(a1, &atmp, resultlen_sig, resultlen_exp, size, ss);
    // smc_batch(atmp, atmp, result, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&atmp, size);
    // free(a1);
}

void SMC_Utils::smc_batch(float *a, float *b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **atmp;
    // ss_batch_convert_to_private_float(a, &atmp, resultlen_sig, resultlen_exp, size, ss);
    // smc_batch(atmp, atmp, result, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&atmp, size);
}

void SMC_Utils::smc_batch(int *a, int *b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // float *a1 = (float *)malloc(sizeof(float) * size);
    // for (int i = 0; i < size; i++)
    //     a1[i] = a[i];
    // mpz_t **atmp;
    // ss_batch_convert_to_private_float(a1, &atmp, resultlen_sig, resultlen_exp, size, ss);
    // smc_batch(atmp, atmp, result, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&atmp, size);
    // free(a1);
}

// public + private two-dimension float - arithmetic
void SMC_Utils::smc_batch(float *a, mpz_t ***b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **atmp;
    // ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    // smc_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&atmp, size);
}

void SMC_Utils::smc_batch(mpz_t ***a, float *b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **btmp;
    // ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    // op = (op == "/") ? "/P" : op;
    // smc_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&btmp, size);
}

void SMC_Utils::smc_batch(float *a, mpz_t ***b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **atmp;
    // ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    // smc_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&atmp, size);
}

void SMC_Utils::smc_batch(mpz_t ***a, float *b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **btmp;
    // ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    // op = (op == "/") ? "/P" : op;
    // smc_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&btmp, size);
}

// public + private one-dimension float - comparison
void SMC_Utils::smc_batch(float *a, mpz_t **b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **atmp;
    // ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    // smc_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&atmp, size);
}

void SMC_Utils::smc_batch(mpz_t **a, float *b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **btmp;
    // ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    // smc_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&btmp, size);
}

// public + private two-dimension float - comparison
void SMC_Utils::smc_batch(float *a, mpz_t ***b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **atmp;
    // ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    // smc_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&atmp, size);
}

void SMC_Utils::smc_batch(mpz_t ***a, float *b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **btmp;
    // ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    // smc_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
    // ss_batch_free_operator(&btmp, size);
}

void SMC_Utils::smc_batch(mpz_t **a, mpz_t **b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **a_tmp, **b_tmp, **result_tmp;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    // ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    // for (int i = 0; i < size; ++i)
    //     for (int j = 0; j < 4; j++)
    //         mpz_set(result[index_array[3 * i + 2]][j], result_tmp[i][j]);

    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&b_tmp, size);
    // ss_batch_free_operator(&result_tmp, size);
}

// first parameter: one-dim float
// second parameter: two-dim float
// assignment parameter: two-dim float

void SMC_Utils::smc_batch(mpz_t **a, mpz_t ***b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **a_tmp, **b_tmp, **result_tmp;
    // int dim1, dim2;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    // ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    // for (int i = 0; i < size; ++i) {
    //     for (int j = 0; j < 4; j++) {
    //         dim1 = index_array[3 * i + 2] / resultdim;
    //         dim2 = index_array[3 * i + 2] % resultdim;
    //         mpz_set(result[dim1][dim2][j], result_tmp[i][j]);
    //     }
    // }

    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&b_tmp, size);
    // ss_batch_free_operator(&result_tmp, size);
}

// first parameter: two-dim float
// second parameter: one-dim float
// assignment parameter: two-dim float

void SMC_Utils::smc_batch(mpz_t ***a, mpz_t **b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **a_tmp, **b_tmp, **result_tmp;
    // int dim1, dim2;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    // ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    // for (int i = 0; i < size; ++i) {
    //     for (int j = 0; j < 4; j++) {
    //         dim1 = index_array[3 * i + 2] / resultdim;
    //         dim2 = index_array[3 * i + 2] % resultdim;
    //         mpz_set(result[dim1][dim2][j], result_tmp[i][j]);
    //     }
    // }

    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&b_tmp, size);
    // ss_batch_free_operator(&result_tmp, size);
}

// first parameter: two-dim float
// second parameter: two-dim float
// assignment parameter: one-dim float

void SMC_Utils::smc_batch(mpz_t ***a, mpz_t ***b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **a_tmp, **b_tmp, **result_tmp;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    // ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    // for (int i = 0; i < size; ++i)
    //     for (int j = 0; j < 4; j++)
    //         mpz_set(result[index_array[3 * i + 2]][j], result_tmp[i][j]);

    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&b_tmp, size);
    // ss_batch_free_operator(&result_tmp, size);
}

// first parameter: two-dim float
// second parameter: one-dim float
// assignment parameter: one-dim float

void SMC_Utils::smc_batch(mpz_t ***a, mpz_t **b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **a_tmp, **b_tmp, **result_tmp;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    // ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    // for (int i = 0; i < size; ++i)
    //     for (int j = 0; j < 4; j++)
    //         mpz_set(result[index_array[3 * i + 2]][j], result_tmp[i][j]);

    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&b_tmp, size);
    // ss_batch_free_operator(&result_tmp, size);
}

// first parameter: one-dim float
// second parameter: two-dim float
// assignment parameter: one-dim float

void SMC_Utils::smc_batch(mpz_t **a, mpz_t ***b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **a_tmp, **b_tmp, **result_tmp;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    // ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    // for (int i = 0; i < size; ++i)
    //     for (int j = 0; j < 4; j++)
    //         mpz_set(result[index_array[3 * i + 2]][j], result_tmp[i][j]);

    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&b_tmp, size);
    // ss_batch_free_operator(&result_tmp, size);
}

// first parameter: one-dim float
// second parameter: one-dim float
// assignment parameter: two-dim float

void SMC_Utils::smc_batch(mpz_t **a, mpz_t **b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **a_tmp, **b_tmp, **result_tmp;
    // int dim1, dim2;

    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    // ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    // for (int i = 0; i < size; ++i) {
    //     for (int j = 0; j < 4; j++) {
    //         dim1 = index_array[3 * i + 2] / resultdim;
    //         dim2 = index_array[3 * i + 2] % resultdim;
    //         mpz_set(result[dim1][dim2][j], result_tmp[i][j]);
    //     }
    // }

    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&b_tmp, size);
    // ss_batch_free_operator(&result_tmp, size);
}

// two dimension float general computation
void SMC_Utils::smc_batch(mpz_t ***a, mpz_t ***b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // int dim1, dim2;
    // mpz_t **a_tmp, **b_tmp, **result_tmp;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    // ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    // for (int i = 0; i < size; ++i) {
    //     for (int j = 0; j < 4; j++) {
    //         dim1 = index_array[3 * i + 2] / resultdim;
    //         dim2 = index_array[3 * i + 2] % resultdim;
    //         mpz_set(result[dim1][dim2][j], result_tmp[i][j]);
    //     }
    // }

    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&b_tmp, size);
    // ss_batch_free_operator(&result_tmp, size);
}

// two-dimensional float comparison
void SMC_Utils::smc_batch(mpz_t ***a, mpz_t ***b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **a_tmp, **b_tmp;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);

    // ss_batch_BOP_float_comparison(result, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&b_tmp, size);
}

// one-dimensional float and two-dimensional float comparison
void SMC_Utils::smc_batch(mpz_t **a, mpz_t ***b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **a_tmp, **b_tmp;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);

    // ss_batch_BOP_float_comparison(result, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&b_tmp, size);
}

// two-dimensional float and one-dimensional float comparison
void SMC_Utils::smc_batch(mpz_t ***a, mpz_t **b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    //     mpz_t **a_tmp, **b_tmp;
    //     ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    //     ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);

    //     ss_batch_BOP_float_comparison(result, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    //     ss_batch_free_operator(&a_tmp, size);
    //     ss_batch_free_operator(&b_tmp, size);
}

// one-dimensional float and one-dimensional float comparison
void SMC_Utils::smc_batch(mpz_t **a, mpz_t **b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID) {
    ss_batch(a, b, result, alen_sig, alen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // mpz_t **a_tmp, **b_tmp;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);

    // ss_batch_BOP_float_comparison(result, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&b_tmp, size);
}

/* conversion from public integer to private float*/
void SMC_Utils::smc_batch_int2fl(int *a, mpz_t **result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    //  a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // for (int i = 0; i < size; i++)
    //     mpz_init_set_si(a_tmp[i], a[i]);
    // smc_batch_int2fl(a_tmp, result, size, resultdim, 32, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID);
    // ss_batch_free_operator(&a_tmp, size);
}

/* conversion from private integer to private float */
// one-dimensional int array to one-dimensional float array
void SMC_Utils::smc_batch_int2fl(mpz_t *a, mpz_t **result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    // mpz_t **result_tmp = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    // mpz_t *a_tmp;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // for (int i = 0; i < size; i++) {
    //     result_tmp[i] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    //     for (int j = 0; j < 4; j++)
    //         mpz_init(result_tmp[i][j]);
    // }
    // doOperation_Int2FL(a_tmp, result_tmp, alen, resultlen_sig, size, threadID, net, id, ss);
    // /* consider the private condition */
    // mpz_t *result_new = (mpz_t *)malloc(sizeof(mpz_t) * 4 * size);
    // mpz_t *result_org = (mpz_t *)malloc(sizeof(mpz_t) * 4 * size);

    // for (int i = 0; i < size; i++) {
    //     for (int j = 0; j < 4; j++) {
    //         mpz_init_set(result_new[4 * i + j], result_tmp[i][j]);
    //         mpz_init_set(result_org[4 * i + j], result[index_array[3 * i + 2]][j]);
    //     }
    // }

    // ss_batch_handle_priv_cond(result_new, result_org, out_cond, priv_cond, counter, 4 * size, threadID, net, id, ss);
    // for (int i = 0; i < size; i++)
    //     for (int j = 0; j < 4; j++)
    //         mpz_set(result[index_array[3 * i + 2]][j], result_new[4 * i + j]);

    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&result_new, 4 * size);
    // ss_batch_free_operator(&result_org, 4 * size);
    // ss_batch_free_operator(&result_tmp, size);
}

void SMC_Utils::smc_batch_int2fl(int *a, mpz_t ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    // mpz_t *a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // for (int i = 0; i < size; i++)
    //     mpz_init_set_si(a_tmp[i], a[i]);
    // smc_batch_int2fl(a_tmp, result, size, resultdim, 32, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID);
    // ss_batch_free_operator(&a_tmp, size);
}

// one-dimensional int array to two-dimensional float array
void SMC_Utils::smc_batch_int2fl(mpz_t *a, mpz_t ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    // mpz_t **result_tmp;
    // int *result_index_array = (int *)malloc(sizeof(int) * size);
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    // for (int i = 0; i < size; i++) {
    //     result_index_array[i] = index_array[3 * i + 2];
    //     index_array[3 * i + 2] = i;
    // }
    // /* call one-dimensional int array + one-dimensional float array */
    // smc_batch_int2fl(a, result_tmp, adim, size, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID);
    // /* set the results back */
    // for (int i = 0; i < size; i++) {
    //     for (int j = 0; j < 4; j++) {
    //         int dim1 = result_index_array[i] / resultdim;
    //         int dim2 = result_index_array[i] % resultdim;
    //         mpz_set(result[dim1][dim2][j], result_tmp[i][j]);
    //     }
    // }
    // free(result_index_array);
    // ss_batch_free_operator(&result_tmp, size);
}

// two-dimensional int array to one-dimensional float array
void SMC_Utils::smc_batch_int2fl(mpz_t **a, mpz_t **result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    // mpz_t *a_tmp;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // for (int i = 0; i < size; i++)
    //     index_array[3 * i] = i;
    // smc_batch_int2fl(a_tmp, result, size, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID);
    // ss_batch_free_operator(&a_tmp, size);
}

// two-dimensional int array to two-dimensional float array
void SMC_Utils::smc_batch_int2fl(mpz_t **a, mpz_t ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2fl(a, result, adim, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    // mpz_t **result_tmp;
    // mpz_t *a_tmp;
    // int *result_index_array = (int *)malloc(sizeof(int) * size);
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    // for (int i = 0; i < size; i++) {
    //     result_index_array[i] = index_array[3 * i + 2];
    //     index_array[3 * i] = i;
    //     index_array[3 * i + 2] = i;
    // }

    // /* call one-dimensional int array + one-dimensional float array	*/
    // smc_batch_int2fl(a_tmp, result_tmp, size, size, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID);

    // /* set the results back */
    // for (int i = 0; i < size; i++) {
    //     for (int j = 0; j < 4; j++) {
    //         int dim1 = result_index_array[i] / resultdim;
    //         int dim2 = result_index_array[i] % resultdim;
    //         mpz_set(result[dim1][dim2][j], result_tmp[i][j]);
    //     }
    // }
    // free(result_index_array);
    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&result_tmp, size);
}

/*FL2INT*/
void SMC_Utils::smc_batch_fl2int(float *a, mpz_t *result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    // mpz_t **a_tmp = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    // for (int i = 0; i < size; i++) {
    //     a_tmp[i] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    //     for (int j = 0; j < 4; j++)
    //         mpz_init(a_tmp[i][j]);
    //     convertFloat(a[i], 32, 9, &a_tmp[i]);
    // }
    // smc_batch_fl2int(a_tmp, result, size, resultdim, 32, 9, blen, out_cond, priv_cond, counter, index_array, size, threadID);
    // ss_batch_free_operator(&a_tmp, size);
}

void SMC_Utils::smc_batch_fl2int(float *a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    // mpz_t **a_tmp = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    // for (int i = 0; i < size; i++) {
    //     a_tmp[i] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    //     for (int j = 0; j < 4; j++)
    //         mpz_init(a_tmp[i][j]);
    //     convertFloat(a[i], 32, 9, &a_tmp[i]);
    // }
    // smc_batch_fl2int(a_tmp, result, size, resultdim, 32, 9, blen, out_cond, priv_cond, counter, index_array, size, threadID);
    // ss_batch_free_operator(&a_tmp, size);
}

void SMC_Utils::smc_batch_fl2int(mpz_t **a, mpz_t *result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    // mpz_t *result_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // mpz_t **a_tmp;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // for (int i = 0; i < size; i++)
    //     mpz_init(result_tmp[i]);

    // doOperation_FL2Int(a_tmp, result_tmp, alen_sig, alen_exp, blen, size, threadID, net, id, ss);

    // /* consider private conditions */
    // mpz_t *result_org = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // for (int i = 0; i < size; i++)
    //     mpz_init_set(result_org[i], result[index_array[3 * i + 2]]);
    // ss_batch_handle_priv_cond(result_tmp, result_org, out_cond, priv_cond, counter, size, threadID, net, id, ss);
    // for (int i = 0; i < size; i++)
    //     mpz_set(result[index_array[3 * i + 2]], result_tmp[i]);

    // ss_batch_free_operator(&result_tmp, size);
    // ss_batch_free_operator(&result_org, size);
    // ss_batch_free_operator(&a_tmp, size);
}

void SMC_Utils::smc_batch_fl2int(mpz_t **a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    // int dim1, dim2;
    // mpz_t *result_tmp;
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);
    // int *result_index_array = (int *)malloc(sizeof(int) * size);
    // for (int i = 0; i < size; i++) {
    //     result_index_array[i] = index_array[3 * i + 2];
    //     index_array[3 * i + 2] = i;
    // }
    // smc_batch_fl2int(a, result_tmp, adim, size, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID);
    // for (int i = 0; i < size; i++) {
    //     dim1 = result_index_array[i] / resultdim;
    //     dim2 = result_index_array[i] % resultdim;
    //     mpz_set(result[dim1][dim2], result_tmp[i]);
    // }
    // free(result_index_array);
    // ss_batch_free_operator(&result_tmp, size);
}

void SMC_Utils::smc_batch_fl2int(mpz_t ***a, mpz_t *result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    // mpz_t **a_tmp;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // for (int i = 0; i < size; i++)
    //     index_array[3 * i] = i;
    // smc_batch_fl2int(a_tmp, result, size, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID);
    // ss_batch_free_operator(&a_tmp, size);
}

void SMC_Utils::smc_batch_fl2int(mpz_t ***a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2int(a, result, adim, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    // int dim1, dim2;
    // mpz_t *result_tmp;
    // mpz_t **a_tmp;
    // int *result_index_array = (int *)malloc(sizeof(int) * size);

    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    // for (int i = 0; i < size; i++) {
    //     result_index_array[i] = index_array[3 * i + 2];
    //     index_array[3 * i] = i;
    //     index_array[3 * i + 2] = i;
    // }

    // smc_batch_fl2int(a_tmp, result_tmp, size, size, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID);

    // for (int i = 0; i < size; i++) {
    //     dim1 = result_index_array[i] / resultdim;
    //     dim2 = result_index_array[i] % resultdim;
    //     mpz_set(result[dim1][dim2], result_tmp[i]);
    // }

    // free(result_index_array);
    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&result_tmp, size);
}
// INT2INT
void SMC_Utils::smc_batch_int2int(int *a, mpz_t *result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
    
    // mpz_t *a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // for (int i = 0; i < size; i++)
    //     mpz_init_set_si(a_tmp[i], a[i]);
    // smc_batch_int2int(a_tmp, result, size, resultdim, 32, blen, out_cond, priv_cond, counter, index_array, size, threadID);
    // ss_batch_free_operator(&a_tmp, size);
}

void SMC_Utils::smc_batch_int2int(int *a, mpz_t **result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    // mpz_t *a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // for (int i = 0; i < size; i++)
    //     mpz_init_set_si(a_tmp[i], a[i]);
    // smc_batch_int2int(a_tmp, result, size, resultdim, 32, blen, out_cond, priv_cond, counter, index_array, size, threadID);
    // ss_batch_free_operator(&a_tmp, size);
}

void SMC_Utils::smc_batch_int2int(mpz_t *a, mpz_t *result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    // mpz_t *result_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // mpz_t *result_org = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // for (int i = 0; i < size; i++) {
    //     mpz_init_set(result_tmp[i], a[index_array[3 * i]]);
    //     mpz_init_set(result_org[i], result[index_array[3 * i + 2]]);
    // }
    // ss_batch_handle_priv_cond(result_tmp, result_org, out_cond, priv_cond, counter, size, threadID, net, id, ss);
    // for (int i = 0; i < size; i++)
    //     mpz_set(result[index_array[3 * i + 2]], result_tmp[i]);
    // ss_batch_free_operator(&result_tmp, size);
    // ss_batch_free_operator(&result_org, size);
}

void SMC_Utils::smc_batch_int2int(mpz_t **a, mpz_t *result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

//     mpz_t *a_tmp;
//     ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
//     for (int i = 0; i < size; i++)
//         index_array[3 * i] = i;
//     smc_batch_int2int(a_tmp, result, size, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID);
//     ss_batch_free_operator(&a_tmp, size);
}

void SMC_Utils::smc_batch_int2int(mpz_t *a, mpz_t **result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    // mpz_t *result_tmp;
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);
    // int *result_index_array = (int *)malloc(sizeof(int) * size);
    // for (int i = 0; i < size; i++) {
    //     result_index_array[i] = index_array[3 * i + 2];
    //     index_array[3 * i + 2] = i;
    // }
    // smc_batch_int2int(a, result_tmp, adim, size, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID);
    // for (int i = 0; i < size; i++) {
    //     int dim1 = result_index_array[i] / resultdim;
    //     int dim2 = result_index_array[i] % resultdim;
    //     mpz_set(result[dim1][dim2], result_tmp[i]);
    // }
    // free(result_index_array);
    // ss_batch_free_operator(&result_tmp, size);
}

void SMC_Utils::smc_batch_int2int(mpz_t **a, mpz_t **result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_int2int(a, result, adim, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    // mpz_t *result_tmp, *a_tmp;
    // int *result_index_array = (int *)malloc(sizeof(int) * size);
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    // for (int i = 0; i < size; i++) {
    //     result_index_array[i] = index_array[3 * i + 2];
    //     index_array[3 * i] = i;
    //     index_array[3 * i + 2] = i;
    // }

    // smc_batch_int2int(a_tmp, result_tmp, size, size, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID);

    // for (int i = 0; i < size; i++) {
    //     int dim1 = result_index_array[i] / resultdim;
    //     int dim2 = result_index_array[i] % resultdim;
    //     mpz_set(result[dim1][dim2], result_tmp[i]);
    // }

    // free(result_index_array);
    // ss_batch_free_operator(&a_tmp, size);
    // ss_batch_free_operator(&result_tmp, size);
}

// FL2FL
void SMC_Utils::smc_batch_fl2fl(float *a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    // mpz_t **a_tmp = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    // for (int i = 0; i < size; i++) {
    //     a_tmp[i] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    //     for (int j = 0; j < 4; j++)
    //         mpz_init(a_tmp[i][j]);
    //     convertFloat(a[i], 32, 9, &a_tmp[i]);
    // }
    // smc_batch_fl2fl(a_tmp, result, size, resultdim, 32, 9, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID);
    // ss_batch_free_operator(&a_tmp, size);
}

void SMC_Utils::smc_batch_fl2fl(float *a, mpz_t ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    // mpz_t **a_tmp = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    // for (int i = 0; i < size; i++) {
    //     a_tmp[i] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    //     for (int j = 0; j < 4; j++)
    //         mpz_init(a_tmp[i][j]);
    //     convertFloat(a[i], 32, 9, &a_tmp[i]);
    // }
    // smc_batch_fl2fl(a_tmp, result, size, resultdim, 32, 9, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID);
    // ss_batch_free_operator(&a_tmp, size);
}

void SMC_Utils::smc_batch_fl2fl(mpz_t **a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    // // extract inputs
    // mpz_t **result_tmp = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    // mpz_t **a_tmp = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    // for (int i = 0; i < 4; i++) {
    //     a_tmp[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
    //     result_tmp[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
    //     for (int j = 0; j < size; j++) {
    //         mpz_init_set(a_tmp[i][j], a[index_array[3 * j]][i]);
    //         mpz_init(result_tmp[i][j]);
    //     }
    // }

    // // start computation
    // if (alen_sig >= blen_sig)
    //     doOperation_Trunc(result_tmp[0], a_tmp[0], alen_sig, alen_sig - blen_sig, size, threadID, net, id, ss);
    // else {
    //     mpz_t tmp, two;
    //     mpz_init_set_ui(two, 2);
    //     mpz_init(tmp);
    //     ss->modPow(tmp, two, blen_sig - alen_sig);
    //     ss->modMul(result_tmp[0], a_tmp[0], tmp, size);
    //     mpz_clear(tmp);
    //     mpz_clear(two);
    // }

    // ss->modAdd(result_tmp[1], a_tmp[1], alen_sig - blen_sig, size);
    // ss->copy(a_tmp[2], result_tmp[2], size);
    // ss->copy(a_tmp[3], result_tmp[3], size);

    // /* consider private conditions */
    // mpz_t *result_new = (mpz_t *)malloc(sizeof(mpz_t) * 4 * size);
    // mpz_t *result_org = (mpz_t *)malloc(sizeof(mpz_t) * 4 * size);
    // for (int i = 0; i < size; i++) {
    //     for (int j = 0; j < 4; j++) {
    //         mpz_init_set(result_new[4 * i + j], result_tmp[j][i]);
    //         mpz_init_set(result_org[4 * i + j], result[index_array[3 * i + 2]][j]);
    //     }
    // }
    // ss_batch_handle_priv_cond(result_new, result_org, out_cond, priv_cond, counter, 4 * size, threadID, net, id, ss);
    // for (int i = 0; i < size; i++)
    //     for (int j = 0; j < 4; j++)
    //         mpz_set(result[index_array[3 * i + 2]][j], result_new[4 * i + j]);

    // // free memory
    // for (int i = 0; i < 4; i++) {
    //     for (int j = 0; j < size; j++) {
    //         mpz_clear(a_tmp[i][j]);
    //         mpz_clear(result_tmp[i][j]);
    //     }
    //     free(a_tmp[i]);
    //     free(result_tmp[i]);
    // }
    // free(a_tmp);
    // free(result_tmp);

    // ss_batch_free_operator(&result_new, 4 * size);
    // ss_batch_free_operator(&result_org, 4 * size);
}

void SMC_Utils::smc_batch_fl2fl(mpz_t **a, mpz_t ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    // mpz_t **result_tmp;
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);
    // int *result_index_array = (int *)malloc(sizeof(int) * size);

    // for (int i = 0; i < size; i++) {
    //     result_index_array[i] = index_array[3 * i + 2];
    //     index_array[3 * i + 2] = i;
    // }

    // smc_batch_fl2fl(a, result_tmp, adim, size, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID);

    // for (int i = 0; i < size; i++) {
    //     for (int j = 0; j < 4; j++) {
    //         int dim1 = result_index_array[i] / resultdim;
    //         int dim2 = result_index_array[i] % resultdim;
    //         mpz_set(result[dim1][dim2][j], result_tmp[i][j]);
    //     }
    // }
    // free(result_index_array);
    // ss_batch_free_operator(&result_tmp, size);
}

void SMC_Utils::smc_batch_fl2fl(mpz_t ***a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    // mpz_t **a_tmp;
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // for (int i = 0; i < size; i++)
    //     index_array[3 * i] = i;
    // smc_batch_fl2fl(a_tmp, result, size, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID);
    // ss_batch_free_operator(&a_tmp, size);
}

void SMC_Utils::smc_batch_fl2fl(mpz_t ***a, mpz_t ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID) {
    ss_batch_fl2fl(a, result, adim, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    // mpz_t **result_tmp;
    // mpz_t **a_tmp;
    // int *result_index_array = (int *)malloc(sizeof(int) * size);
    // ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    // ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    // for (int i = 0; i < size; i++) {
    //     result_index_array[i] = index_array[3 * i + 2];
    //     index_array[3 * i + 2] = i;
    //     index_array[3 * i] = i;
    // }

    // smc_batch_fl2fl(a_tmp, result_tmp, size, size, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID);

    // for (int i = 0; i < size; i++) {
    //     for (int j = 0; j < 4; j++) {
    //         int dim1 = result_index_array[i] / resultdim;
    //         int dim2 = result_index_array[i] % resultdim;
    //         mpz_set(result[dim1][dim2][j], result_tmp[i][j]);
    //     }
    // }

    // free(result_index_array);
    // ss_batch_free_operator(&result_tmp, size);
    // ss_batch_free_operator(&a_tmp, size);
}

/* Clienct Connection and Data Passing */
void SMC_Utils::clientConnect() {
    int sockfd, portno;
    socklen_t clilen;
    struct sockaddr_in server_addr, cli_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        fprintf(stderr, "ERROR, opening socket\n");
    bzero((char *)&server_addr, sizeof(server_addr));
    portno = nodeConfig->getPort() + 100;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        fprintf(stderr, "ERROR, on binding\n");
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
        fprintf(stderr, "ERROR, on accept\n");
    printf("Client connected\n");
}

void SMC_Utils::receivePolynomials(std::string privatekey_filename) {
    FILE *prikeyfp = fopen(privatekey_filename.c_str(), "r");
    if (prikeyfp == NULL)
        printf("File Open %s error\n", privatekey_filename.c_str());
    RSA *priRkey = PEM_read_RSAPrivateKey(prikeyfp, NULL, NULL, NULL);
    if (priRkey == NULL)
        printf("Read Private Key for RSA Error\n");
    char *buffer = (char *)malloc(RSA_size(priRkey));
    int n = read(newsockfd, buffer, RSA_size(priRkey));
    if (n < 0)
        printf("ERROR reading from socket \n");
    char *decrypt = (char *)malloc(n);
    memset(decrypt, 0x00, n);
    int dec_len = RSA_private_decrypt(n, (unsigned char *)buffer, (unsigned char *)decrypt, priRkey, RSA_PKCS1_OAEP_PADDING);
    if (dec_len < 1)
        printf("RSA private decrypt error\n");
    if (dec_len < 1) {
        printf("RSA private decrypt error\n");
    }

    int keysize = 0;
    int coefsize = 0;
    int mpz_t_size = 0;
    memcpy(&keysize, decrypt, sizeof(int));
    memcpy(&coefsize, decrypt + sizeof(int), sizeof(int));
    memcpy(&mpz_t_size, decrypt + sizeof(int) * 2, sizeof(int));
    mpz_t *Keys = (mpz_t *)malloc(sizeof(mpz_t) * keysize);
    for (int k = 0; k < keysize; k++)
        mpz_init(Keys[k]);
    int *Coefficients = (int *)malloc(sizeof(int) * coefsize);
    int position = 0;
    for (int i = 0; i < keysize; i++) {
        // char strkey[mpz_t_size + 1] = {0}; // this needed to be added, otherwise some garbage was appearing at the end

        char strkey[mpz_t_size + 1]; // clang doesnt like the above version
        memset(strkey, 0x00, mpz_t_size + 1);

        // strkey[0] = '\0';
        memcpy(strkey, decrypt + sizeof(int) * 3 + position, mpz_t_size);
        // printf("%s\n", strkey);
        // std::string Strkey (strkey);
        // std::cout<<Strkey<<std::endl;
        // for (int c = 0; strkey[c] != '\0'; c++)
        // 	printf("%c", strkey[c]);
        // printf("\n");
        mpz_set_str(Keys[i], strkey, BASE_10);
        position += mpz_t_size;
    }
    for (size_t i = 0; i < keysize; i++) {
        // gmp_printf("Key[%i]: %Zu\n", i, Keys[i]);
    }

    memcpy(Coefficients, decrypt + sizeof(int) * 3 + mpz_t_size * keysize, sizeof(int) * coefsize);
    free(buffer);
    free(decrypt);
    // printf("coefsize = %i \t keysize = %i, \t mpz_t_size = %i\n", coefsize, mpz_t_size);
    // for (size_t i = 0; i < coefsize; i++) {
    //     printf("Coefficients[%i] = %i\n", i, Coefficients[i]);
    // }

    for (int i = 0; i < keysize; i++) {
        char strkey[mpz_t_size + 1];
        memset(strkey, 0x00, mpz_t_size + 1);
        // strkey[0] = 0;
        mpz_get_str(strkey, BASE_10, Keys[i]);
        std::string Strkey = strkey;
        std::vector<int> temp;
        for (int k = 0; k < coefsize / keysize; k++) {
            temp.push_back(Coefficients[i * coefsize / keysize + k]);
        }
        // std::cout<<"temp.size = "<<temp.size()<<std::endl;
        // std::cout << Strkey << " temp = ";
        // for (size_t j = 0; j < temp.size(); j++) {
        //     std::cout << temp.at(j) << ", ";
        // }
        // std::cout << std::endl;

        polynomials.insert(std::pair<std::string, std::vector<int>>(Strkey, temp));
    }
    // printf("POLYNOMIAL SIZE = %i\n",polysize);
    printf("Polynomials received... \n");

    // for (auto &[key, value] : polynomials) {
    //     std::cout << key << ", ";
    //     for (size_t j = 0; j < value.size(); j++) {
    //         std::cout << value.at(j) << ", ";
    //     }
    //     std::cout << std::endl;
    // }
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

    int K = alen;
    int M = ceil(log2(K));

    printf("Testing PRandInt for (K = %i, M = %i), size = %i)\n", K, M, size);
    PRandInt(K, M, size, a, threadID, ss);
    // for (int i = 0; i < size; i++) {
    //     gmp_printf("a[%i] %Zu\n", i, a[i]);
    // }
    Open(a, b, size, threadID, net, ss);
    for (int i = 0; i < size; i++) {
        gmp_printf("result[%i] %Zu\n", i, b[i]);
    }

    printf("Testing PRandBit for (K = %i, M = %i), size = %i)\n", K, M, size);
    PRandBit(size, a, threadID, net, id, ss);
    // for (int i = 0; i < size; i++) {
    //     gmp_printf("a[%i] %Zu\n", i, a[i]);
    // }
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
    mpz_t *res_check = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; ++i) {
        mpz_init(res[i]);
        mpz_init(res_check[i]);
    }

    PRandM(K, size, V, threadID, net, id, ss); // generating r', r'_k-1,...,r'_0

    printf("Testing PRandM for (K = %i, M = %i), size = %i)\n", K, M, size);

    // for (int i = 0; i < Kp1 - 1; i++) {
    //     Open(V[i], res, size, threadID, net, ss);
    //     for (int j = 0; j < size; j++) {
    //         gmp_printf("%Zu ", res[j]);
    //     }
    //     printf("\n");
    // }
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
        mpz_clear(res_check[i]);
    }
    free(res_check);
    free(res);
}

// proof of concept type-specifc implementations

// void SMC_Utils::test_type(int x) {
//     priv_int value;
//     ss_init_set_si(value, x);
//     gmp_printf("value %Zd\n", value);
//     // printf("value %lu\n", value);

//     ss_clear(value);
//     // printf("value %lu\n", value);
//     gmp_printf("value %Zd\n", value); // value is cleared at this point, returns garbage

//     uint size = 3;
//     // priv_int *val_arr = (priv_int *)malloc(sizeof(priv_int) * size);
//     priv_int *val_arr = new priv_int[size];
//     ss_init_set_si(val_arr[0], x);
//     ss_init_set_si(val_arr[1], x + 3);
//     ss_init_set_si(val_arr[2], x + 16);
//     for (size_t i = 0; i < size; i++) {
//         // printf("val_arr[%lu] %lu\n", i, val_arr[i]);
//         gmp_printf("val_arr[%i] %Zd\n", i, val_arr[i]);
//     }
//     ss_free_arr(val_arr, size);
//     for (size_t i = 0; i < size; i++) {
//         // printf("val_arr[%lu] %lu\n", i, val_arr[i]);
//         gmp_printf("val_arr[%i] %Zd\n", i, val_arr[i]);
//     }
// }

// void ss_init_set_si(unsigned long &x, int x_val) {
//     x = x_val;
// }

// // for ul's, dont need to do anything for within-scope single variables, so just return
// void ss_clear(unsigned long &x) {}

// // // frees space oucupied by x

// void ss_free_arr(unsigned long *op, int size) {
//     delete[] op;
// }
