/*  
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame

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

#include "Headers.h"
#include "NodeConfiguration.h"
#include "NodeNetwork.h"
#include "ops/Operation.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>

class SMC_Utils {
public:
	//Constructors
	SMC_Utils(int id, std::string runtime_config, std::string privatekey_filename, int numOfInputPeers, int numOfOutputPeers, std::string* IO_files, int numOfPeers, int threshold, int bits, std::string mod, int num_threads);
	//Share a secret between	
	int smc_open(mpz_t var, int threadID);
	float smc_open(mpz_t* var, int threadID); 
	//Methods for input and output
	//for integer variable;
	void smc_input(int id, int* var, std::string type, int threadID);
	void smc_input(int id, mpz_t* var, std::string type, int  threadID);
	void smc_output(int id, int* var, std::string type, int threadID);
	void smc_output(int id, mpz_t* var, std::string type, int thread);
	void convertFloat(float value, int K, int L, mpz_t** elements); 
	void convertDouble(double value, int K, int L, mpz_t** elements); 

	//for float variable;
	void smc_input(int id, float* var, std::string type, int threadID);
	void smc_input(int id, mpz_t** var, std::string type, int threadID);
	void smc_output(int id, float* var, std::string type, int threadID);
	void smc_output(int id, mpz_t** var, std::string type, int threadID);
	
	//for one dimensional integer;  
	void smc_input(int id, int* var, int size, std::string type, int threadID);
	void smc_input(int id, mpz_t* var, int size, std::string type, int threadID);
	void smc_output(int id, int* var, int size, std::string type, int threadID);
	void smc_output(int id, mpz_t* var, int size, std::string type, int threadID);
	//for one-dimensional float
	void smc_input(int id, float* var, int size, std::string type, int threadID);  
	void smc_input(int id, mpz_t** var, int size, std::string type, int threadID);  
	void smc_output(int id, float* var, int size, std::string type, int threadID);  
	void smc_output(int id, mpz_t** var, int size, std::string type, int threadID);  
	
    /******************************
	* All the different operators 
	******************************/
	//Helper function for floating point operations
	void smc_single_fop_arithmetic(mpz_t* result, mpz_t* a, mpz_t* b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, std::string op, int threadID); 
	void smc_single_fop_comparison(mpz_t result, mpz_t* a, mpz_t* b, int resultlen, int alen_sig, int alen_exp, int blen_sig, int blen_exp, std::string op, int threadID); 
	
	void smc_batch_fop_arithmetic(mpz_t** result, mpz_t** a, mpz_t** b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID); 
	void smc_batch_fop_comparison(mpz_t* result, mpz_t** a, mpz_t** b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID); 

	/************************************ Addition *****************************************/	
	/********* singular operations *******/
	//1) private int + private int
	void smc_add(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int clen, std::string type, int threadID);
	//2) private float + private float
	void smc_add(mpz_t* a, mpz_t* b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID); 
	//3) private int + public int
	void smc_add(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//4) public int + private int
	void smc_add(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//5) private float + public float
	void smc_add(mpz_t* a, float b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID);
	//6) private float + public float
	void smc_add(float a, mpz_t* b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID);
	/************ batch operations *********/
	void smc_add(int* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	void smc_add(mpz_t* a, int* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	void smc_add(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	void smc_add(mpz_t** a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int  resultlen_sig, int resultlen_exp, int size, std::string type, int threadID); 
	void smc_add(float* a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int  resultlen_sig, int resultlen_exp, int size, std::string type, int threadID); 
	void smc_add(mpz_t** a, float* b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int  resultlen_sig, int resultlen_exp, int size, std::string type, int threadID); 
    	
	void smc_sum(mpz_t* a, mpz_t result, int size);

	/************************************* Subtraction *************************************/
	/********** singular operations ***********/
	//1) private int - private int 
	void smc_sub(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//2) private float - private float
	void smc_sub(mpz_t* a, mpz_t* b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID); 
	//3) private int - public int
	void smc_sub(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//4) public int - private int
	void smc_sub(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//5) private float - public float
	void smc_sub(mpz_t* a, float b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID);
	//6) public float - private float
	void smc_sub(float a, mpz_t* b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID);
	/************ batch operations *********/
	void smc_sub(int* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	void smc_sub(mpz_t* a, int* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	void smc_sub(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	void smc_sub(mpz_t** a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int  resultlen_sig, int resultlen_exp, int size, std::string type, int threadID); 
	void smc_sub(float* a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int  resultlen_sig, int resultlen_exp, int size, std::string type, int threadID); 
	void smc_sub(mpz_t** a, float* b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int  resultlen_sig, int resultlen_exp, int size, std::string type, int threadID); 
	/************************************** Multiplication ***************************************/
	/************* singular operations *****************/
	//1) private int * private int
	void smc_mult(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//2) private int * public int
	void smc_mult(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//3) public int * private int
	void smc_mult(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID); 
	//4) private float * private float
	void smc_mult(mpz_t* a, mpz_t* b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID); 
	//5) public float * private float
	void smc_mult(float a, mpz_t* b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID); 
	//6) private float * public float
	void smc_mult(mpz_t* a, float b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID); 
	//7) private float * private int (only used when private float is updated in the body of private condition)
	void smc_mult(mpz_t* a, mpz_t b, mpz_t* result, int alen_sig, int alen_exp, int blen, int resultlen_sig, int resultlen_exp, std::string type, int threadID); //only used in the priv condition
	/************ batch operations *********/
	void smc_mult(mpz_t* a, int* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	void smc_mult(int* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	void smc_mult(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	void smc_mult(mpz_t** a, float* b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int  resultlen_sig, int resultlen_exp, int size, std::string type, int threadID); 
	void smc_mult(float* a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int  resultlen_sig, int resultlen_exp, int size, std::string type, int threadID); 
	void smc_mult(mpz_t** a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int  resultlen_sig, int resultlen_exp, int size, std::string type, int threadID); 
	/*****************************************  Division ******************************************/
	/******************* singular operations *****************/
	//1) private int / private int
	void smc_div(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//2) private int / public int
	void smc_div(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//3) public int / private int 
	void smc_div(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//4) private float / private float
	void smc_div(mpz_t* a, mpz_t* b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID); 
	//5) private float / public float
	void smc_div(mpz_t* a, float b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID); 
	//6) public float / private float
	void smc_div(float a, mpz_t* b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID); 
	
	/************ batch operations *********/
	void smc_div(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	void smc_div(mpz_t* a, int* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	void smc_div(int* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	void smc_div(mpz_t** a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int  resultlen_sig, int resultlen_exp, int size, std::string type, int threadID); 
	void smc_div(mpz_t** a, float* b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int  resultlen_sig, int resultlen_exp, int size, std::string type, int threadID); 
	void smc_div(float* a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int  resultlen_sig, int resultlen_exp, int size, std::string type, int threadID); 

 	
	/************************** Comparisons ***********************************************/
	/************************** singular operations ************************/
	//1) private int < private int
	void smc_lt(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//2) private int < public int
	void smc_lt(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//3) public int < private int
	void smc_lt(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//4) private float < private float
	void smc_lt(mpz_t* a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID);
	//5) private float < public float
	void smc_lt(mpz_t* a, float b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID);
	//6) public float < private float
	void smc_lt(float a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID);
 
	/************ batch operations *********/
	void smc_lt(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	void smc_lt(mpz_t** a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	/************************* singular operations ************************/
	//1) private int > private int
	void smc_gt(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//2) private int > public int
	void smc_gt(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//3) public int > private int  
	void smc_gt(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//4) private float > private float
	void smc_gt(mpz_t* a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID); 
	//5) private float > public float
	void smc_gt(mpz_t* a, float b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID); 
	//6) public float > private float
	void smc_gt(float a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID); 
	
	/************ batch operations *********/
	void smc_gt(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	void smc_gt(mpz_t** a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	/************************* singular operations ***********************/	
	//1) private int <= private int
	void smc_leq(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//2) private int <= public int
	void smc_leq(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//3) public int <= private int
	void smc_leq(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//4) private float <= private float
	void smc_leq(mpz_t* a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID); 
	//5) private float <= public float
	void smc_leq(mpz_t* a, float b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID); 
	//6) public float <= private float
	void smc_leq(float a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID); 

	/************ batch operations *********/
	void smc_leq(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	void smc_leq(mpz_t** a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t* result, int resultlen, int size, std::string type, int threadID); 

	/************************* singular operations **********************/
	//1) private int >= private int 
	void smc_geq(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//2) private int >= public int
	void smc_geq(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//3) public int >= private int
	void smc_geq(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//4) private float >= private float	
	void smc_geq(mpz_t* a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID); 
	//5) private float >= public float	
	void smc_geq(mpz_t* a, float b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID); 
	//6) public float >= private float	
	void smc_geq(float a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID); 
	
	/************ batch operations *********/
	void smc_geq(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	void smc_geq(mpz_t** a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t* result, int resultlen, int size, std::string type, int threadID); 

	/*********************** singular operations *************************/
	//1) private int == private int
	void smc_eqeq(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//2) private int == public int
	void smc_eqeq(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//3) public int == private int
	void smc_eqeq(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//4) private float == private float
	void smc_eqeq(mpz_t* a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID); 
	//5) private float == public float
	void smc_eqeq(mpz_t* a, float b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID); 
	//6) public float == private float
	void smc_eqeq(float a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID); 
	
	/************ batch operations *********/
	void smc_eqeq(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	void smc_eqeq(mpz_t** a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t* result, int resultlen, int size, std::string type, int threadID); 

	/************************* singular operations **************************/
	//1) private int != private int
	void smc_neq(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//2) private int != public int
	void smc_neq(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//3) public int != private int
	void smc_neq(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	//4) private float != private float
	void smc_neq(mpz_t* a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID); 
	//5) private float != public float
	void smc_neq(mpz_t* a, float b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID); 
	//6) public float != private float
	void smc_neq(float a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID); 
	
	/************ batch operations *********/
	void smc_neq(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	void smc_neq(mpz_t** a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	 
	//Logical operators
	void smc_land(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	void smc_land(mpz_t* a, mpz_t* b, int size, mpz_t* result, std::string type, int threadID);
	void smc_xor(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	void smc_xor(mpz_t* a, mpz_t* b, int size, mpz_t* result, std::string type, int threadID);
	void smc_lor(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID);
	void smc_lor(mpz_t* a, mpz_t* b, int size, mpz_t* result, std::string type, int threadID);

	//Shift operators
	void smc_shl(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID); 
	void smc_shl(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID); 
	void smc_shr(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID); 
	void smc_shr(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID); 

	
	void smc_shl(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	void smc_shl(mpz_t* a, int* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	void smc_shr(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	void smc_shr(mpz_t* a, int* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID); 
	
	//for floating point
	void smc_set(float a, mpz_t* result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID); 
	void smc_set(mpz_t* a, mpz_t* result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID); 
	void smc_set(mpz_t** a, mpz_t** result, int alen_sig, int alne_exp, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID); 
	//for integer set
	void smc_set(mpz_t a, mpz_t result, int alen, int resultlen, std::string type, int threadID); 
	void smc_set(mpz_t* a, mpz_t* result, int alen, int resultlen, int size, std::string type, int threadID); 
	void smc_set(int a, mpz_t result, int alen, int resultlen, std::string type, int threadID);
	//Dot Product
	void smc_dot(mpz_t* a, mpz_t* b, int size, mpz_t result, int threadID);
	void smc_dot(mpz_t** a, mpz_t**b, int batch_size, int array_size, mpz_t* result, std::string type, int threadID);	
	void smc_batch_dot(mpz_t** a, mpz_t** b, int batch_size, int array_size, int *index_array, mpz_t* result, int threadID); 
   
	void smc_priv_eval(mpz_t a, mpz_t b, mpz_t cond, int threadID); 
	void smc_priv_eval(mpz_t* a, mpz_t* b, mpz_t cond, int threadID); 
 
    /******************** singular privindex *******************/
    //one-dimension private integer read
    void smc_privindex_read(mpz_t index, mpz_t* array, mpz_t result, int dim, std::string type, int threadID);
    //two-dimension private integer read
    void smc_privindex_read(mpz_t index, mpz_t** array, mpz_t result, int dim1, int dim2, std::string type, int threadID);
    //one-dimension private float read
    void smc_privindex_read(mpz_t index, mpz_t** array, mpz_t* result, int dim, std::string type, int threadID);
    //two-dimension private float read
    void smc_privindex_read(mpz_t index, mpz_t*** array, mpz_t* result, int dim1, int dim2, std::string type, int threadID);
    
    //one-dimension private integer write
    void smc_privindex_write(mpz_t index, mpz_t* array, int len_sig, int len_exp, int result, int dim, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID);
    void smc_privindex_write(mpz_t index, mpz_t** array, int len_sig, int len_exp, int result, int dim1, int dim2, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID);
    void smc_privindex_write(mpz_t index, mpz_t* array, int len_sig, int len_exp, mpz_t result, int dim, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID);
    
    //two-dimension private integer write
    void smc_privindex_write(mpz_t index, mpz_t** array, int len_sig, int len_exp, mpz_t result, int dim1, int dim2, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID);
    //one-dimension private float write
    void smc_privindex_write(mpz_t index, mpz_t** array, int len_sig, int len_exp, mpz_t* result, int dim, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID);
    void smc_privindex_write(mpz_t index, mpz_t** array, int len_sig, int len_exp, float result, int dim, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID);
    //two-dimension private float write
    void smc_privindex_write(mpz_t index, mpz_t*** array, int len_sig, int len_exp, mpz_t* result, int dim1, int dim2, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID);
    void smc_privindex_write(mpz_t index, mpz_t*** array, int len_sig, int len_exp, float result, int dim1, int dim2, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID);
    
    /***************** batch privindex **********************/
    //one-dimension private integer read
    void smc_privindex_read(mpz_t* index, mpz_t* array, mpz_t* result, int dim, int batch_size, std::string type, int threadID);
    //two-dimension private integer read
    void smc_privindex_read(mpz_t* index, mpz_t** array, mpz_t* result, int dim1, int dim2, int batch_size, std::string type, int threadID);
    //one-dimension private float read
    void smc_privindex_read(mpz_t* index, mpz_t** array, mpz_t** result, int dim, int batch_size, std::string type, int threadID);
    //two-dimension private float read
    void smc_privindex_read(mpz_t* index, mpz_t*** array, mpz_t** result, int dim1, int dim2, int batch_size, std::string type, int threadID);
    
    //one-dimension private integer write
    void smc_privindex_write(mpz_t* index, mpz_t* array, int len_sig, int len_exp, int* result, int dim, int batch_size, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID);
    void smc_privindex_write(mpz_t* index, mpz_t* array, int len_sig, int len_exp, mpz_t* result, int dim, int batch_size, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID);
    //two-dimension private integer write
    void smc_privindex_write(mpz_t* index, mpz_t** array, int len_sig, int len_exp, int* result, int dim1, int dim2, int batch_size, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID);
    void smc_privindex_write(mpz_t* index, mpz_t** array, int len_sig, int len_exp, mpz_t* result, int dim1, int dim2, int batch_size, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID);
    //one-dimension private float write
    void smc_privindex_write(mpz_t* index, mpz_t** array, int len_sig, int len_exp, float* result, int dim, int batch_size, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID);
    void smc_privindex_write(mpz_t* index, mpz_t** array, int len_sig, int len_exp, mpz_t** result, int dim, int batch_size, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID);
    //two-dimension private float write
    void smc_privindex_write(mpz_t* index, mpz_t*** array, int len_sig, int len_exp, float* result, int dim1, int dim2, int batch_size, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID);
    void smc_privindex_write(mpz_t* index, mpz_t*** array, int len_sig, int len_exp, mpz_t** result, int dim1, int dim2, int batch_size, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID);
   
    /********************** pointer interfaces (for int) **************************************/
   
    priv_ptr smc_new_ptr(int level, int type); 
    priv_ptr* smc_new_ptr(int level, int type, int num);
 
    void smc_set_int_ptr(priv_ptr ptr, mpz_t* var_loc, std::string type, int threadID); 
    void smc_set_float_ptr(priv_ptr ptr, mpz_t** var_loc, std::string type, int threadID); 
    void smc_set_struct_ptr(priv_ptr ptr, void* var_loc, std::string type, int threadID);
    void smc_set_ptr(priv_ptr ptr, int var_loc, std::string type, int threadID); 
    void smc_set_ptr(priv_ptr ptr, priv_ptr* ptr_loc, std::string type, int threadID); 
    void smc_set_ptr(priv_ptr assign_ptr, priv_ptr right_ptr, std::string type, int threadID); 
    
    void smc_update_int_ptr(priv_ptr ptr, mpz_t* var_loc, mpz_t private_tag, int index, int threadID); 
    void smc_update_float_ptr(priv_ptr ptr, mpz_t** var_loc, mpz_t private_tag, int index, int threadID); 
    void smc_update_struct_ptr(priv_ptr ptr, void* var_loc, mpz_t private_tag, int index, int threadID); 

    void smc_update_ptr(priv_ptr ptr, priv_ptr* ptr_loc, mpz_t private_tag, int index, int threadID); 
    void smc_update_ptr(priv_ptr assign_ptr, priv_ptr right_ptr, mpz_t private_tag, int index, int threadID); 

    void smc_add_int_ptr(priv_ptr, mpz_t* var_loc, mpz_t private_tag, int threadID); 
    void smc_add_float_ptr(priv_ptr, mpz_t** var_loc, mpz_t private_tag, int threadID); 
    void smc_add_struct_ptr(priv_ptr, void* var_loc, mpz_t private_tag, int threadID); 
    void smc_add_ptr(priv_ptr, priv_ptr* ptr_loc, mpz_t private_tag, int threadID); 
    
    void smc_shrink_ptr(priv_ptr, int current_index, int parent_index, int threadID); 
    
    void smc_dereference_read_ptr(priv_ptr ptr, mpz_t var_result, int num_of_dereferences, mpz_t priv_cond, std::string type, int threadID);
    void smc_dereference_read_ptr(priv_ptr ptr, mpz_t* var_result, int num_of_dereferences, mpz_t priv_cond, std::string type, int threadID);
    void smc_dereference_read_ptr(priv_ptr ptr, priv_ptr ptr_result, int num_of_dereferences, mpz_t priv_cond, std::string type, int threadID);
    
    void smc_dereference_write_int_ptr(priv_ptr ptr, mpz_t* var_value, int num_of_dereferences, mpz_t private_cond, std::string type, int threadID); 
    void smc_dereference_write_float_ptr(priv_ptr ptr, mpz_t** var_value, int num_of_dereferences, mpz_t private_cond, std::string type, int threadID); 
    void smc_dereference_write_struct_ptr(priv_ptr ptr, void* var_value, int num_of_dereferences, mpz_t private_cond, std::string type, int threadID); 

    void smc_dereference_write_ptr(priv_ptr ptr, mpz_t var_value, int num_of_dereferences, mpz_t private_cond, std::string type, int threadID);  
    void smc_dereference_write_ptr(priv_ptr ptr, mpz_t* var_value, int num_of_dereferences, mpz_t private_cond, std::string type, int threadID);  
    void smc_dereference_write_ptr(priv_ptr ptr, priv_ptr ptr_value, int num_of_dereferences, mpz_t private_cond, std::string type, int threadID);  
    void smc_dereference_write_ptr(priv_ptr ptr, priv_ptr* ptr_value, int num_of_dereferences, mpz_t private_cond, std::string type, int threadID);  
    
    void smc_clear_ptr(priv_ptr* ptr); 
    void smc_free_ptr(priv_ptr* ptr); 
    void smc_free_ptr(priv_ptr** arrays, int num); 

        /************************************************/
	void smc_int2fl(mpz_t value, mpz_t* result, int gamma, int K, int L, int threadID); 	
	void smc_int2fl(int value, mpz_t* result, int gamma, int K, int L, int threadID); 
	void smc_int2int(mpz_t value, mpz_t result, int gamma1, int gamma2, int threadID);
	void smc_int2int(int value, mpz_t result, int gamma1, int gamma2, int threadID);  
	void smc_fl2int(mpz_t* value, mpz_t result, int K, int L, int gamma, int threadID); 
	void smc_fl2int(float value, mpz_t result, int K, int L, int gamma, int threadID); 
	void smc_fl2fl(mpz_t* value, mpz_t* result, int K1, int L1, int K2, int L2, int threadID); 
	void smc_fl2fl(float value, mpz_t* result, int K1, int L1, int K2, int L2, int threadID); 

	//Batch Operation Support
	void smc_convert_operator(mpz_t** result, mpz_t** op, int* index_array, int dim, int size, int flag);
	void smc_convert_operator(mpz_t** result, mpz_t* op, int* index_array, int dim, int size, int flag); 
	void smc_convert_operator(mpz_t*** result, mpz_t** op, int* index_array, int dim, int size, int flag); 
	void smc_convert_operator(mpz_t*** result, mpz_t*** op, int* index_array, int dim, int size, int flag); 
	
	void smc_single_convert_to_private_float(float a, mpz_t** priv_a, int len_sig, int len_exp); 
	void smc_batch_convert_to_private_float(float* a, mpz_t*** priv_a, int len_sig, int len_exp, int size); 
	void smc_batch_free_operator(mpz_t** op, int size);
	void smc_batch_free_operator(mpz_t*** op, int size);

    //int
	void smc_batch(mpz_t* a, mpz_t* b, mpz_t* result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
	void smc_batch(mpz_t** a, mpz_t** b, mpz_t** result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
	void smc_batch(mpz_t* a, mpz_t** b, mpz_t** result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
	void smc_batch(mpz_t** a, mpz_t* b, mpz_t** result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
	void smc_batch(mpz_t** a, mpz_t** b, mpz_t* result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
        void smc_batch(mpz_t* a, mpz_t* b, mpz_t** result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
        void smc_batch(mpz_t** a, mpz_t* b, mpz_t* result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
        void smc_batch(mpz_t* a, mpz_t** b, mpz_t* result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
    
    //float
	void smc_batch(mpz_t** a, mpz_t** b, mpz_t** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
	void smc_batch(mpz_t*** a, mpz_t*** b, mpz_t*** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
	void smc_batch(mpz_t** a, mpz_t*** b, mpz_t*** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID); 	
	void smc_batch(mpz_t*** a, mpz_t** b, mpz_t*** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
	void smc_batch(mpz_t*** a, mpz_t*** b, mpz_t** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
        void smc_batch(mpz_t** a, mpz_t** b, mpz_t*** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
	void smc_batch(mpz_t** a, mpz_t*** b, mpz_t** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
	void smc_batch(mpz_t*** a, mpz_t** b, mpz_t** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
		
	//private float comparisons
	void smc_batch(mpz_t*** a, mpz_t*** b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
	void smc_batch(mpz_t** a, mpz_t*** b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
	void smc_batch(mpz_t*** a, mpz_t** b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
	void smc_batch(mpz_t** a, mpz_t** b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
	
	
	//operations between public and private values; 
	//public + private one-dimension int
    	void smc_batch(int* a, int* b, mpz_t* result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
	void smc_batch(int* a, int* b, mpz_t** result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID); 
    	
	void smc_batch(int* a, mpz_t* b, mpz_t* result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
	void smc_batch(mpz_t* a, int* b, mpz_t* result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
	
	void smc_batch(mpz_t* a, int* b, mpz_t** result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
	void smc_batch(int* a, mpz_t* b, mpz_t** result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
	
    	void smc_batch(int* a, mpz_t** b, mpz_t* result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
    	void smc_batch(mpz_t** a, int* b, mpz_t* result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
	
    	void smc_batch(int* a, mpz_t** b, mpz_t** result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
    	void smc_batch(mpz_t** a, int* b, mpz_t** result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID);
	
	void smc_batch(int a, mpz_t* b, mpz_t* result, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, int threadID); 
	void smc_batch(mpz_t* a, mpz_t* b, mpz_t* result, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, int threadID); 
		
	//public + private one-dimension float - arithmetic
	void smc_batch(float* a, float* b, mpz_t*** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID); 	
	void smc_batch(int* a, int* b, mpz_t*** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID); 	
	
	void smc_batch(float* a, float* b, mpz_t** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID); 	
	void smc_batch(int* a, int* b, mpz_t** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID); 	
	
	void smc_batch(float* a, mpz_t** b, mpz_t*** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID); 	
	void smc_batch(mpz_t** a, float* b, mpz_t*** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID); 	
	void smc_batch(float* a, mpz_t** b, mpz_t** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID); 	
	void smc_batch(mpz_t** a, float* b, mpz_t** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID); 	
	
	//public + private two-dimension float - arithmetic
	void smc_batch(float* a, mpz_t*** b, mpz_t*** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID); 	
	void smc_batch(mpz_t*** a, float* b, mpz_t*** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID); 	
	void smc_batch(float* a, mpz_t*** b, mpz_t** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID); 	
	void smc_batch(mpz_t*** a, float* b, mpz_t** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID); 	
	
	//public + private one-dimension float - comparison
	void smc_batch(float* a, mpz_t** b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID); 	
	void smc_batch(mpz_t** a, float* b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID); 	
	
	//public + private two-dimension float - comparison
	void smc_batch(float* a, mpz_t*** b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID); 	
	void smc_batch(mpz_t*** a, float* b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID); 	

	void smc_batch_handle_priv_cond(mpz_t* result, mpz_t* result_org, mpz_t out_cond, mpz_t *priv_cond, int counter, int size, int threadID); 
	void smc_batch_BOP_int(mpz_t* result, mpz_t* a, mpz_t* b, int resultlen, int alen, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int size, std::string op, std::string type, int threadID); 
	void smc_batch_BOP_float_arithmetic(mpz_t** result, mpz_t** a, mpz_t** b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int size, std::string op, std::string type, int threadID); 
	void smc_batch_BOP_float_comparison(mpz_t* result, mpz_t** a, mpz_t** b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID); 

	void smc_batch_int2fl(int* a, mpz_t** result, int adim, int resultdim, int alen, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID); 
        void smc_batch_int2fl(int* a, mpz_t*** result, int adim, int resultdim, int alen, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID);
	void smc_batch_int2fl(mpz_t* a, mpz_t** result, int adim, int resultdim, int alen, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID); 
	void smc_batch_int2fl(mpz_t* a, mpz_t*** result, int adim, int resultdim, int alen, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID); 
	void smc_batch_int2fl(mpz_t** a, mpz_t** result, int adim, int resultdim, int alen, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID); 
	void smc_batch_int2fl(mpz_t** a, mpz_t*** result, int adim, int resultdim, int alen, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID); 

 	void smc_batch_fl2int(float* a, mpz_t* result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID);
        void smc_batch_fl2int(float* a, mpz_t** result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID);
        void smc_batch_fl2int(mpz_t** a, mpz_t* result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID);
        void smc_batch_fl2int(mpz_t** a, mpz_t** result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID); 
        void smc_batch_fl2int(mpz_t*** a, mpz_t* result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID); 
        void smc_batch_fl2int(mpz_t*** a, mpz_t** result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID);

	void smc_batch_int2int(int* a, mpz_t* result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID);
        void smc_batch_int2int(int* a, mpz_t** result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID);
        void smc_batch_int2int(mpz_t* a, mpz_t* result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID);
        void smc_batch_int2int(mpz_t** a, mpz_t* result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID);
        void smc_batch_int2int(mpz_t* a, mpz_t** result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID);
        void smc_batch_int2int(mpz_t** a, mpz_t** result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID);	

	
	 void smc_batch_fl2fl(float* a, mpz_t** result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID);
        void smc_batch_fl2fl(float* a, mpz_t*** result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID);
        void smc_batch_fl2fl(mpz_t** a, mpz_t** result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID);
        void smc_batch_fl2fl(mpz_t** a, mpz_t*** result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID);
        void smc_batch_fl2fl(mpz_t*** a, mpz_t** result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID);
        void smc_batch_fl2fl(mpz_t*** a, mpz_t*** result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID);

	void smc_compute_len(int alen, int blen, int* len); 
	void smc_process_operands(mpz_t** a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int* len_sig, int* len_exp, int size); 
	void smc_process_results(mpz_t** result, int resultlen_sig, int resultlen_exp, int len_sig, int len_exp, int size, int threadID); 
	
	std::map<std::string, std::vector<int> > polynomials; //temporarily public
	mpz_t coef[9]; //temporarily public
	int id; //temporarily public;

	double time_diff(struct timeval *, struct timeval *);
	std::vector<std::string> splitfunc(const char* str, const char* delim);
private:
	SecretShare *ss;
	FILE* inputFile; 
	NodeConfiguration* nodeConfig;
    	NodeNetwork nNet;	
	std::ifstream* inputStreams;
        std::ofstream* outputStreams; 
	int base;

	Mult *Mul;
	LTZ *Lt;
	EQZ *Eq;
	DotProduct *DProd;
	BitOps *BOps;
	Trunc *T; 
	TruncS *Ts; 
	FLAdd *Fladd; 
	FLMult *Flmult; 
	FLLTZ *Flltz;
	FLEQZ *Fleqz;  
	FLDiv *Fldiv;
	FPDiv *Fpdiv;  
	PrivIndex *PI;
	PrivPtr *PP; 
	Int2FL *I2F;  
	FL2Int *F2I; 
	IntDiv *Idiv; 
	Pow2* P; 	
	//Handle client connections and polynomail stuff
	void clientConnect();
	void receivePolynomials(std::string privatekey_filename);
	void setCoef();
	int peers;
	int newsockfd;
};

#endif /* SMC_UTILS_H_ */
