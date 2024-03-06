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

#ifndef SMC_UTILS_H_
#define SMC_UTILS_H_

#include "bit_utils.hpp"
#include "openssl/bio.h"
#include "unistd.h"
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <math.h>
#include <netinet/in.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <sstream>
#include <string>

#if __SHAMIR__
#include "shamir/ShamirHeaders.h"
#include "shamir/ShamirOps.h"
typedef mpz_t priv_int;
#define MPZ_CAST(X) (mpz_t *)(X)
#endif
#if __RSS__
#include "rss/RSS_types.hpp" // header generated dynamically by the compiler according to the ring size
#define MPZ_CAST(X) (priv_int *)(X) // check how to up-cast from 1D to 2D

#include "rss/RSSHeaders.hpp"
#include "rss/RSSOps.hpp"
#include <type_traits>
#endif

class SMC_Utils {
public:
    // Constructors
    SMC_Utils(int id, std::string runtime_config, std::string privatekey_filename, int numOfInputPeers, int numOfOutputPeers, std::string *IO_files, int numOfPeers, int threshold, int bits, std::string mod, std::vector<int> &seed_map, int num_threads);
    // Share a secret between
    int smc_open(priv_int var, int threadID);
    float smc_open(priv_int *var, int threadID);
   #if _RSS_ 
    void smc_open(priv_int result, priv_int *var, int size, int threadID);
#endif
    // Methods for input and output
    // for integer variable;
    void smc_input(int id, int *var, std::string type, int threadID);
    void smc_input(int id, priv_int *var, std::string type, int threadID);
    void smc_output(int id, int *var, std::string type, int threadID);
    void smc_output(int id, priv_int *var, std::string type, int thread);

    // for float variable;
    void smc_input(int id, float *var, std::string type, int threadID);
    void smc_input(int id, priv_int **var, std::string type, int threadID);
    void smc_output(int id, float *var, std::string type, int threadID);
    void smc_output(int id, priv_int **var, std::string type, int threadID);

    // for one dimensional integer;
    void smc_input(int id, int *var, int size, std::string type, int threadID);
    void smc_input(int id, priv_int *var, int size, std::string type, int threadID);
    void smc_output(int id, int *var, int size, std::string type, int threadID);
    void smc_output(int id, priv_int *var, int size, std::string type, int threadID);
    // for one-dimensional float
    void smc_input(int id, float *var, int size, std::string type, int threadID);
    void smc_input(int id, priv_int **var, int size, std::string type, int threadID);
    void smc_output(int id, float *var, int size, std::string type, int threadID);
    void smc_output(int id, priv_int **var, int size, std::string type, int threadID);

    /******************************
     * All the different operators
     ******************************/
    // Helper function for floating point operations
    void smc_single_fop_arithmetic(priv_int *result, priv_int *a, priv_int *b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, std::string op, int threadID);
    // void smc_single_fop_comparison(priv_int result, priv_int *a, priv_int *b, int resultlen, int alen_sig, int alen_exp, int blen_sig, int blen_exp, std::string op, int threadID);

    void smc_batch_fop_arithmetic(priv_int **result, priv_int **a, priv_int **b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID);
    // void smc_batch_fop_comparison(priv_int *result, priv_int **a, priv_int **b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID);

    /************************************ Addition *****************************************/
    /********* singular operations *******/
    // 1) private int + private int
    void smc_add(priv_int a, priv_int b, priv_int result, int alen, int blen, int clen, std::string type, int threadID);
    // 2) private float + private float
    void smc_add(priv_int *a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID);
    // 3) private int + public int
    void smc_add(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 4) public int + private int
    void smc_add(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 5) private float + public float
    void smc_add(priv_int *a, float b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID);
    // 6) private float + public float
    void smc_add(float a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID);
    /************ batch operations *********/
    void smc_add(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_add(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_add(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_add(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID);
    void smc_add(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID);
    void smc_add(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID);

    void smc_sum(priv_int *a, priv_int result, int size);

    /************************************* Subtraction *************************************/
    /********** singular operations ***********/
    // 1) private int - private int
    void smc_sub(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 2) private float - private float
    void smc_sub(priv_int *a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID);
    // 3) private int - public int
    void smc_sub(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 4) public int - private int
    void smc_sub(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 5) private float - public float
    void smc_sub(priv_int *a, float b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID);
    // 6) public float - private float
    void smc_sub(float a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID);
    /************ batch operations *********/
    void smc_sub(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_sub(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_sub(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_sub(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID);
    void smc_sub(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID);
    void smc_sub(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID);
    /************************************** Multiplication ***************************************/
    /************* singular operations *****************/
    // 1) private int * private int
    void smc_mult(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 2) private int * public int
    void smc_mult(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 3) public int * private int
    void smc_mult(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 4) private float * private float
    void smc_mult(priv_int *a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID);
    // 5) public float * private float
    void smc_mult(float a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID);
    // 6) private float * public float
    void smc_mult(priv_int *a, float b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID);
    // 7) private float * private int (only used when private float is updated in the body of private condition)
    void smc_mult(priv_int *a, priv_int b, priv_int *result, int alen_sig, int alen_exp, int blen, int resultlen_sig, int resultlen_exp, std::string type, int threadID); // only used in the priv condition
    /************ batch operations *********/
    void smc_mult(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_mult(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_mult(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_mult(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID);
    void smc_mult(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID);
    void smc_mult(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID);
    /*****************************************  Division ******************************************/
    /******************* singular operations *****************/
    // 1) private int / private int
    void smc_div(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 2) private int / public int
    void smc_div(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 3) public int / private int
    void smc_div(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 4) private float / private float
    void smc_div(priv_int *a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID);
    // 5) private float / public float
    void smc_div(priv_int *a, float b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID);
    // 6) public float / private float
    void smc_div(float a, priv_int *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID);

    /************ batch operations *********/
    void smc_div(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_div(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_div(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_div(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID);
    void smc_div(priv_int **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID);
    void smc_div(float *a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID);

    /************************** Comparisons ***********************************************/
    /************************** singular operations ************************/
    // 1) private int < private int
    void smc_lt(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 2) private int < public int
    void smc_lt(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 3) public int < private int
    void smc_lt(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 4) private float < private float
    void smc_lt(priv_int *a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID);
    // 5) private float < public float
    void smc_lt(priv_int *a, float b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID);
    // 6) public float < private float
    void smc_lt(float a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID);

    /************ batch operations *********/
    void smc_lt(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_lt(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID);
    /************************* singular operations ************************/
    // 1) private int > private int
    void smc_gt(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 2) private int > public int
    void smc_gt(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 3) public int > private int
    void smc_gt(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 4) private float > private float
    void smc_gt(priv_int *a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID);
    // 5) private float > public float
    void smc_gt(priv_int *a, float b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID);
    // 6) public float > private float
    void smc_gt(float a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID);

    /************ batch operations *********/
    void smc_gt(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_gt(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID);
    /************************* singular operations ***********************/
    // 1) private int <= private int
    void smc_leq(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 2) private int <= public int
    void smc_leq(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 3) public int <= private int
    void smc_leq(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 4) private float <= private float
    void smc_leq(priv_int *a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID);
    // 5) private float <= public float
    void smc_leq(priv_int *a, float b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID);
    // 6) public float <= private float
    void smc_leq(float a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID);

    /************ batch operations *********/
    void smc_leq(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_leq(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID);

    /************************* singular operations **********************/
    // 1) private int >= private int
    void smc_geq(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 2) private int >= public int
    void smc_geq(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 3) public int >= private int
    void smc_geq(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 4) private float >= private float
    void smc_geq(priv_int *a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID);
    // 5) private float >= public float
    void smc_geq(priv_int *a, float b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID);
    // 6) public float >= private float
    void smc_geq(float a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID);

    /************ batch operations *********/
    void smc_geq(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_geq(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID);

    /*********************** singular operations *************************/
    // 1) private int == private int
    void smc_eqeq(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 2) private int == public int
    void smc_eqeq(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 3) public int == private int
    void smc_eqeq(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 4) private float == private float
    void smc_eqeq(priv_int *a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID);
    // 5) private float == public float
    void smc_eqeq(priv_int *a, float b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID);
    // 6) public float == private float
    void smc_eqeq(float a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID);

    /************ batch operations *********/
    void smc_eqeq(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_eqeq(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID);

    void smc_eqeq(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_eqeq(int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);

    /************************* singular operations **************************/
    // 1) private int != private int
    void smc_neq(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 2) private int != public int
    void smc_neq(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 3) public int != private int
    void smc_neq(int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    // 4) private float != private float
    void smc_neq(priv_int *a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID);
    // 5) private float != public float
    void smc_neq(priv_int *a, float b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID);
    // 6) public float != private float
    void smc_neq(float a, priv_int *b, priv_int result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID);

    /************ batch operations *********/
    void smc_neq(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_neq(priv_int **a, priv_int **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int *result, int resultlen, int size, std::string type, int threadID);

    // Logical operators
    void smc_land(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    void smc_land(priv_int *a, priv_int *b, int size, priv_int *result, std::string type, int threadID);
    void smc_xor(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    void smc_xor(priv_int *a, priv_int *b, int size, priv_int *result, std::string type, int threadID);
    void smc_lor(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    void smc_lor(priv_int *a, priv_int *b, int size, priv_int *result, std::string type, int threadID);

    // Shift operators
    void smc_shl(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    void smc_shl(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    void smc_shr(priv_int a, priv_int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);
    void smc_shr(priv_int a, int b, priv_int result, int alen, int blen, int resultlen, std::string type, int threadID);

    void smc_shl(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_shl(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_shr(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);
    void smc_shr(priv_int *a, int *b, int alen, int blen, priv_int *result, int resultlen, int size, std::string type, int threadID);

    // for floating point
    void smc_set(float a, priv_int *result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID);
    void smc_set(priv_int *a, priv_int *result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID);
    void smc_set(priv_int **a, priv_int **result, int alen_sig, int alne_exp, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID);
    // for integer set
    void smc_set(priv_int a, priv_int result, int alen, int resultlen, std::string type, int threadID);
    void smc_set(priv_int *a, priv_int *result, int alen, int resultlen, int size, std::string type, int threadID);
    void smc_set(int a, priv_int result, int alen, int resultlen, std::string type, int threadID);
    // Dot Product
    void smc_dot(priv_int *a, priv_int *b, int size, priv_int result, int threadID);
    void smc_dot(priv_int **a, priv_int **b, int batch_size, int array_size, priv_int *result, std::string type, int threadID);
    void smc_batch_dot(priv_int **a, priv_int **b, int batch_size, int array_size, int *index_array, priv_int *result, int threadID);

    void smc_priv_eval(priv_int a, priv_int b, priv_int cond, int threadID);
    void smc_priv_eval(priv_int *a, priv_int *b, priv_int cond, int threadID);

    /******************** singular privindex *******************/
    // one-dimension private integer read
    void smc_privindex_read(priv_int index, priv_int *array, priv_int result, int dim, std::string type, int threadID);
    // two-dimension private integer read
    void smc_privindex_read(priv_int index, priv_int **array, priv_int result, int dim1, int dim2, std::string type, int threadID);
    // one-dimension private float read
    void smc_privindex_read(priv_int index, priv_int **array, priv_int *result, int dim, std::string type, int threadID);
    // two-dimension private float read
    void smc_privindex_read(priv_int index, priv_int ***array, priv_int *result, int dim1, int dim2, std::string type, int threadID);

    // one-dimension private integer write
    void smc_privindex_write(priv_int index, priv_int *array, int len_sig, int len_exp, int result, int dim, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID);
    void smc_privindex_write(priv_int index, priv_int **array, int len_sig, int len_exp, int result, int dim1, int dim2, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID);
    void smc_privindex_write(priv_int index, priv_int *array, int len_sig, int len_exp, priv_int result, int dim, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID);

    // two-dimension private integer write
    void smc_privindex_write(priv_int index, priv_int **array, int len_sig, int len_exp, priv_int result, int dim1, int dim2, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID);
    // one-dimension private float write
    void smc_privindex_write(priv_int index, priv_int **array, int len_sig, int len_exp, priv_int *result, int dim, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID);
    void smc_privindex_write(priv_int index, priv_int **array, int len_sig, int len_exp, float result, int dim, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID);
    // two-dimension private float write
    void smc_privindex_write(priv_int index, priv_int ***array, int len_sig, int len_exp, priv_int *result, int dim1, int dim2, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID);
    void smc_privindex_write(priv_int index, priv_int ***array, int len_sig, int len_exp, float result, int dim1, int dim2, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID);

    /***************** batch privindex **********************/
    // one-dimension private integer read
    void smc_privindex_read(priv_int *index, priv_int *array, priv_int *result, int dim, int batch_size, std::string type, int threadID);
    // two-dimension private integer read
    void smc_privindex_read(priv_int *index, priv_int **array, priv_int *result, int dim1, int dim2, int batch_size, std::string type, int threadID);
    // one-dimension private float read
    void smc_privindex_read(priv_int *index, priv_int **array, priv_int **result, int dim, int batch_size, std::string type, int threadID);
    // two-dimension private float read
    void smc_privindex_read(priv_int *index, priv_int ***array, priv_int **result, int dim1, int dim2, int batch_size, std::string type, int threadID);

    // one-dimension private integer write
    void smc_privindex_write(priv_int *index, priv_int *array, int len_sig, int len_exp, int *result, int dim, int batch_size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID);
    void smc_privindex_write(priv_int *index, priv_int *array, int len_sig, int len_exp, priv_int *result, int dim, int batch_size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID);
    // two-dimension private integer write
    void smc_privindex_write(priv_int *index, priv_int **array, int len_sig, int len_exp, int *result, int dim1, int dim2, int batch_size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID);
    void smc_privindex_write(priv_int *index, priv_int **array, int len_sig, int len_exp, priv_int *result, int dim1, int dim2, int batch_size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID);
    // one-dimension private float write
    void smc_privindex_write(priv_int *index, priv_int **array, int len_sig, int len_exp, float *result, int dim, int batch_size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID);
    void smc_privindex_write(priv_int *index, priv_int **array, int len_sig, int len_exp, priv_int **result, int dim, int batch_size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID);
    // two-dimension private float write
    void smc_privindex_write(priv_int *index, priv_int ***array, int len_sig, int len_exp, float *result, int dim1, int dim2, int batch_size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID);
    void smc_privindex_write(priv_int *index, priv_int ***array, int len_sig, int len_exp, priv_int **result, int dim1, int dim2, int batch_size, priv_int out_cond, priv_int *priv_cond, int counter, std::string type, int threadID);

    /********************** pointer interfaces (for int) **************************************/
#if __SHAMIR__

    priv_ptr smc_new_ptr(int level, int type);
    priv_ptr *smc_new_ptr(int level, int type, int num);

    void smc_set_int_ptr(priv_ptr ptr, priv_int *var_loc, std::string type, int threadID);
    void smc_set_float_ptr(priv_ptr ptr, priv_int **var_loc, std::string type, int threadID);
    void smc_set_struct_ptr(priv_ptr ptr, void *var_loc, std::string type, int threadID);
    void smc_set_ptr(priv_ptr ptr, int var_loc, std::string type, int threadID);
    void smc_set_ptr(priv_ptr ptr, priv_ptr *ptr_loc, std::string type, int threadID);
    void smc_set_ptr(priv_ptr assign_ptr, priv_ptr right_ptr, std::string type, int threadID);

    void smc_update_int_ptr(priv_ptr ptr, priv_int *var_loc, priv_int private_tag, int index, int threadID);
    void smc_update_float_ptr(priv_ptr ptr, priv_int **var_loc, priv_int private_tag, int index, int threadID);
    void smc_update_struct_ptr(priv_ptr ptr, void *var_loc, priv_int private_tag, int index, int threadID);

    void smc_update_ptr(priv_ptr ptr, priv_ptr *ptr_loc, priv_int private_tag, int index, int threadID);
    void smc_update_ptr(priv_ptr assign_ptr, priv_ptr right_ptr, priv_int private_tag, int index, int threadID);

    void smc_add_int_ptr(priv_ptr, priv_int *var_loc, priv_int private_tag, int threadID);
    void smc_add_float_ptr(priv_ptr, priv_int **var_loc, priv_int private_tag, int threadID);
    void smc_add_struct_ptr(priv_ptr, void *var_loc, priv_int private_tag, int threadID);
    void smc_add_ptr(priv_ptr, priv_ptr *ptr_loc, priv_int private_tag, int threadID);

    void smc_shrink_ptr(priv_ptr, int current_index, int parent_index, int threadID);

    void smc_dereference_read_ptr(priv_ptr ptr, priv_int var_result, int num_of_dereferences, priv_int priv_cond, std::string type, int threadID);
    void smc_dereference_read_ptr(priv_ptr ptr, priv_int *var_result, int num_of_dereferences, priv_int priv_cond, std::string type, int threadID);
    void smc_dereference_read_ptr(priv_ptr ptr, priv_ptr ptr_result, int num_of_dereferences, priv_int priv_cond, std::string type, int threadID);

    void smc_dereference_write_int_ptr(priv_ptr ptr, priv_int *var_value, int num_of_dereferences, priv_int private_cond, std::string type, int threadID);
    void smc_dereference_write_float_ptr(priv_ptr ptr, priv_int **var_value, int num_of_dereferences, priv_int private_cond, std::string type, int threadID);
    void smc_dereference_write_struct_ptr(priv_ptr ptr, void *var_value, int num_of_dereferences, priv_int private_cond, std::string type, int threadID);

    void smc_dereference_write_ptr(priv_ptr ptr, priv_int var_value, int num_of_dereferences, priv_int private_cond, std::string type, int threadID);
    void smc_dereference_write_ptr(priv_ptr ptr, priv_int *var_value, int num_of_dereferences, priv_int private_cond, std::string type, int threadID);
    void smc_dereference_write_ptr(priv_ptr ptr, priv_ptr ptr_value, int num_of_dereferences, priv_int private_cond, std::string type, int threadID);
    void smc_dereference_write_ptr(priv_ptr ptr, priv_ptr *ptr_value, int num_of_dereferences, priv_int private_cond, std::string type, int threadID);

    void smc_clear_ptr(priv_ptr *ptr);
    void smc_free_ptr(priv_ptr *ptr);
    void smc_free_ptr(priv_ptr **arrays, int num);

#endif

    /************************************************/
    void smc_int2fl(priv_int value, priv_int *result, int gamma, int K, int L, int threadID);
    void smc_int2fl(int value, priv_int *result, int gamma, int K, int L, int threadID);
    void smc_int2int(priv_int value, priv_int result, int gamma1, int gamma2, int threadID);
    void smc_int2int(int value, priv_int result, int gamma1, int gamma2, int threadID);
    void smc_fl2int(priv_int *value, priv_int result, int K, int L, int gamma, int threadID);
    void smc_fl2int(float value, priv_int result, int K, int L, int gamma, int threadID);
    void smc_fl2fl(priv_int *value, priv_int *result, int K1, int L1, int K2, int L2, int threadID);
    void smc_fl2fl(float value, priv_int *result, int K1, int L1, int K2, int L2, int threadID);

    // Batch Operation Support
    // void smc_convert_operator(priv_int **result, priv_int **op, int *index_array, int dim, int size, int flag);
    // void smc_convert_operator(priv_int **result, priv_int *op, int *index_array, int dim, int size, int flag);
    // void smc_convert_operator(priv_int ***result, priv_int **op, int *index_array, int dim, int size, int flag);
    // void smc_convert_operator(priv_int ***result, priv_int ***op, int *index_array, int dim, int size, int flag);

    void smc_single_convert_to_private_float(float a, priv_int **priv_a, int len_sig, int len_exp);
    void smc_batch_convert_to_private_float(float *a, priv_int ***priv_a, int len_sig, int len_exp, int size);
    // void ss_batch_free_operator(priv_int **op, int size);
    // void ss_batch_free_operator(priv_int ***op, int size);

    // int
    void smc_batch(priv_int *a, priv_int *b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int **a, priv_int **b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int *a, priv_int **b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int **a, priv_int *b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int **a, priv_int **b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int *a, priv_int *b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int **a, priv_int *b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int *a, priv_int **b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);

    // float
    void smc_batch(priv_int **a, priv_int **b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int ***a, priv_int ***b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int **a, priv_int ***b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int ***a, priv_int **b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int ***a, priv_int ***b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int **a, priv_int **b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int **a, priv_int ***b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int ***a, priv_int **b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);

    // private float comparisons
    void smc_batch(priv_int ***a, priv_int ***b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int **a, priv_int ***b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int ***a, priv_int **b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int **a, priv_int **b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);

    // operations between public and private values;
    // public + private one-dimension int
    void smc_batch(int *a, int *b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(int *a, int *b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);

    void smc_batch(int *a, priv_int *b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int *a, int *b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);

    void smc_batch(priv_int *a, int *b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(int *a, priv_int *b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);

    void smc_batch(int *a, priv_int **b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int **a, int *b, priv_int *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);

    void smc_batch(int *a, priv_int **b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int **a, int *b, priv_int **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);

    void smc_batch(int a, priv_int *b, priv_int *result, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, int threadID);
    void smc_batch(priv_int *a, priv_int *b, priv_int *result, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, int threadID);

    // public + private one-dimension float - arithmetic
    void smc_batch(float *a, float *b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(int *a, int *b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);

    void smc_batch(float *a, float *b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(int *a, int *b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);

    void smc_batch(float *a, priv_int **b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int **a, float *b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(float *a, priv_int **b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int **a, float *b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);

    // public + private two-dimension float - arithmetic
    void smc_batch(float *a, priv_int ***b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int ***a, float *b, priv_int ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(float *a, priv_int ***b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int ***a, float *b, priv_int **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);

    // public + private one-dimension float - comparison
    void smc_batch(float *a, priv_int **b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int **a, float *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);

    // public + private two-dimension float - comparison
    void smc_batch(float *a, priv_int ***b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);
    void smc_batch(priv_int ***a, float *b, priv_int *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);

    void smc_batch_handle_priv_cond(priv_int *result, priv_int *result_org, priv_int out_cond, priv_int *priv_cond, int counter, int size, int threadID);
    void smc_batch_BOP_int(priv_int *result, priv_int *a, priv_int *b, int resultlen, int alen, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int size, std::string op, std::string type, int threadID);
    void smc_batch_BOP_float_arithmetic(priv_int **result, priv_int **a, priv_int **b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int size, std::string op, std::string type, int threadID);
    void smc_batch_BOP_float_comparison(priv_int *result, priv_int **a, priv_int **b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID);

    void smc_batch_int2fl(int *a, priv_int **result, int adim, int resultdim, int alen, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);
    void smc_batch_int2fl(int *a, priv_int ***result, int adim, int resultdim, int alen, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);
    void smc_batch_int2fl(priv_int *a, priv_int **result, int adim, int resultdim, int alen, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);
    void smc_batch_int2fl(priv_int *a, priv_int ***result, int adim, int resultdim, int alen, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);
    void smc_batch_int2fl(priv_int **a, priv_int **result, int adim, int resultdim, int alen, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);
    void smc_batch_int2fl(priv_int **a, priv_int ***result, int adim, int resultdim, int alen, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);

    void smc_batch_fl2int(float *a, priv_int *result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);
    void smc_batch_fl2int(float *a, priv_int **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);
    void smc_batch_fl2int(priv_int **a, priv_int *result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);
    void smc_batch_fl2int(priv_int **a, priv_int **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);
    void smc_batch_fl2int(priv_int ***a, priv_int *result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);
    void smc_batch_fl2int(priv_int ***a, priv_int **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);

    void smc_batch_int2int(int *a, priv_int *result, int adim, int resultdim, int alen, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);
    void smc_batch_int2int(int *a, priv_int **result, int adim, int resultdim, int alen, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);
    void smc_batch_int2int(priv_int *a, priv_int *result, int adim, int resultdim, int alen, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);
    void smc_batch_int2int(priv_int **a, priv_int *result, int adim, int resultdim, int alen, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);
    void smc_batch_int2int(priv_int *a, priv_int **result, int adim, int resultdim, int alen, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);
    void smc_batch_int2int(priv_int **a, priv_int **result, int adim, int resultdim, int alen, int blen, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);

    void smc_batch_fl2fl(float *a, priv_int **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);
    void smc_batch_fl2fl(float *a, priv_int ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);
    void smc_batch_fl2fl(priv_int **a, priv_int **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);
    void smc_batch_fl2fl(priv_int **a, priv_int ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);
    void smc_batch_fl2fl(priv_int ***a, priv_int **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);
    void smc_batch_fl2fl(priv_int ***a, priv_int ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, priv_int out_cond, priv_int *priv_cond, int counter, int *index_array, int size, int threadID);

    // void smc_compute_len(int alen, int blen, int *len);
    void smc_process_results(priv_int **result, int resultlen_sig, int resultlen_exp, int len_sig, int len_exp, int size, int threadID);

    // void test_type(int);

    void smc_test_op(priv_int *a, priv_int *b, int alen, int blen, priv_int *result, int resultlen, int size, int threadID);

    std::map<std::string, std::vector<int>> shamir_seeds_coefs; // mapping of keys (str) to polynomial coefficients
    std::map<std::vector<int>, uint8_t *> rss_share_seeds;

    int id;

    double time_diff(struct timeval *, struct timeval *);

    void seedSetup(std::vector<int> &seed_map, int peers, int threshold);

    std::vector<int> extract_share_WITH_ACCESS(int binary_rep, int peers, int id);
    std::vector<int> extract_share_WITHOUT_ACCESS(int binary_rep, int peers, int id);
    void getCombinations(std::vector<int> &elements, int reqLen, std::vector<int> &pos, int depth, int margin, std::vector<std::vector<int>> &result);
    std::vector<int> generateCoefficients(std::vector<int> T_set, int threshold);

#if __RSS__
    uint getNumShares();
#endif

private:
#if __SHAMIR__
    SecretShare *ss;
#endif
#if __RSS__
    replicatedSecretShare<std::remove_pointer_t<priv_int>> *ss;
#endif

    FILE *inputFile;
    NodeConfiguration *nodeConfig;
    NodeNetwork net;
    std::ifstream *inputStreams;
    std::ofstream *outputStreams;
};

#endif /* SMC_UTILS_H_ */
