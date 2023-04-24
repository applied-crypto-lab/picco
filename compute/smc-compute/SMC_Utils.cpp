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
#include <string>
#include <cmath>
#include <openssl/rsa.h> 
#include <openssl/rand.h> 
#include <openssl/pem.h>
#include "openssl/bio.h"
#include <netinet/in.h> 
#include "unistd.h" 

//Constructors
SMC_Utils::SMC_Utils(int id, std::string runtime_config, std::string privatekey_filename, int numOfInputPeers, int numOfOutputPeers, std::string *IO_files, int numOfPeers, int threshold, int bits, std::string mod, int num_threads){ 
	base = 10;
	std::cout << "SMC_Utils constructor\n";
	mpz_t modulus;
	mpz_init(modulus);
	mpz_set_str(modulus, mod.c_str(), 10);
	nodeConfig = new NodeConfiguration(id, runtime_config, bits);
	peers = numOfPeers; 
	std::cout << "Creating the NodeNetwork\n";

	NodeNetwork* nodeNet = new NodeNetwork(nodeConfig, privatekey_filename, num_threads);    
	nNet = *nodeNet;

	std::cout << "Creating SecretShare\n";
	ss = new SecretShare(numOfPeers, threshold, modulus, id, nNet.getPRGseeds());

	clientConnect();
	receivePolynomials(privatekey_filename);

	//initialize input and output streams
	inputStreams = new std::ifstream[numOfInputPeers]; 
	outputStreams = new std::ofstream[numOfOutputPeers]; 
	for(int i = 0; i < numOfInputPeers; i++)
	{
		inputStreams[i].open(IO_files[i].c_str(), std::ifstream::in);
		if(!inputStreams[i]){
			std::cout << "Input files could not be opened\n";
			std::exit(1);
		}
	}
	
	for(int i = 0; i < numOfOutputPeers; i++)
	{ 
		std::stringstream c;
		c << id;
        	std::string s = c.str();
		IO_files[numOfInputPeers+i] = IO_files[numOfInputPeers+i] + s;
		outputStreams[i].open(IO_files[numOfInputPeers+i].c_str(), std::ofstream::out); 
		if(!outputStreams[i])
		{
			std::cout<<"Output files could not be opened\n"; 
			std::exit(1); 
		}
	}
    if (peers == 3) {
		ss->getCoef(id);
		ss->Seed((nodeNet->key_0),(nodeNet->key_1));
	}
	setCoef(); 
	Mul = new Mult(*nodeNet, id, ss);
	Lt = new LTZ(*nodeNet, polynomials, id, ss, coef);	
	Eq = new EQZ(*nodeNet, polynomials, id, ss, coef);
	T = new Trunc(*nodeNet, polynomials, id, ss, coef); 
	Ts = new TruncS(*nodeNet, polynomials, id, ss, coef); 
	P = new Pow2(*nodeNet, polynomials, id, ss, coef); 
	BOps = new BitOps(*nodeNet, polynomials, id, ss, coef);  
	DProd = new DotProduct(*nodeNet, polynomials, id, ss, coef); 
	PI = new PrivIndex(*nodeNet, polynomials, id, ss, coef); 
	PP = new PrivPtr(*nodeNet, id, ss); 
	Idiv = new IntDiv(*nodeNet, polynomials, id, ss, coef);
	I2F = new Int2FL(*nodeNet, polynomials, id, ss, coef); 
 	F2I = new FL2Int(*nodeNet, polynomials, id, ss, coef); 
	Fladd = new FLAdd(*nodeNet, polynomials, id, ss, coef); 
	Flmult = new FLMult(*nodeNet, polynomials, id, ss, coef); 
	Fldiv = new FLDiv(*nodeNet, polynomials, id,  ss, coef); 
	Flltz = new FLLTZ(*nodeNet, polynomials, id, ss, coef); 		
	Fleqz = new FLEQZ(*nodeNet, polynomials, id, ss, coef); 		
	Fpdiv = new FPDiv(*nodeNet, polynomials, id, ss, coef); 	
	//for test purposes
}	

/* Specific SMC Utility Functions */
int SMC_Utils::smc_open(mpz_t var, int threadID){
	mpz_t* data = (mpz_t*)malloc(sizeof(mpz_t) * 1); 
	mpz_t* results = (mpz_t*)malloc(sizeof(mpz_t) * 1); 
	mpz_t** buffer = (mpz_t**)malloc(sizeof(mpz_t*) * peers); 
	for(int i=0; i<peers; ++i){
		buffer[i] = (mpz_t*)malloc(sizeof(mpz_t)); 
		mpz_init(buffer[i][0]);
	}
	mpz_init(data[0]); 
	mpz_init(results[0]); 
	mpz_set(data[0], var);  
	nNet.broadcastToPeers(data, 1, buffer, threadID);
	ss->reconstructSecret(results,buffer, 1);
	mpz_t tmp, field; 
	mpz_init(tmp); 
	mpz_init(field); 
	ss->getFieldSize(field); 
	mpz_mul_ui(tmp, results[0], 2); 
	if(mpz_cmp(tmp, field) > 0)
		mpz_sub(results[0], results[0], field); 
	//gmp_printf("%Zd ", results[0]); 
	int result = mpz_get_si(results[0]); 
	
	//free the memory 
	mpz_clear(data[0]); 
	mpz_clear(results[0]); 
	free(data); 
	free(results); 
	for(int i = 0; i < peers; i++)
	{
		mpz_clear(buffer[i][0]); 
		free(buffer[i]); 
	} 
	free(buffer);
 
	return result; 
}

/********************************************************/

float SMC_Utils::smc_open(mpz_t* var, int threadID){
        mpz_t* data = (mpz_t*)malloc(sizeof(mpz_t) * 4);
        mpz_t* results = (mpz_t*)malloc(sizeof(mpz_t) * 4);
        mpz_t** buffer = (mpz_t**)malloc(sizeof(mpz_t*) * peers);
        for(int i=0; i<peers; ++i){
                buffer[i] = (mpz_t*)malloc(sizeof(mpz_t) * 4);
		for(int j = 0; j < 4; j++)
                	mpz_init(buffer[i][j]);
        }

	for(int i = 0; i < 4; i++)
	{
        	mpz_init(data[i]);
        	mpz_init(results[i]);
        	mpz_set(data[i], var[i]);
	}
        nNet.broadcastToPeers(data, 4, buffer, threadID);
        ss->reconstructSecret(results,buffer, 4);
	
	for(int i = 0; i < 4; i++){
		if(i == 1){
		 	mpz_t tmp, field;
       			mpz_init(tmp);
       		 	mpz_init(field);
        		ss->getFieldSize(field);
        		mpz_mul_ui(tmp, results[1], 2);
        		if(mpz_cmp(tmp, field) > 0)
                		mpz_sub(results[1], results[1], field);
		}
	}
        double v = mpz_get_d(results[0]);
	double p = mpz_get_d(results[1]); 
	double z = mpz_get_d(results[2]); 
	double s = mpz_get_d(results[3]);
	double result = 0; 
	
	//free the memory 
        for(int i = 0; i < 4; i++)
	{
		mpz_clear(data[i]);
        	mpz_clear(results[i]);
	}
	free(data); 
	free(results); 
	
        for(int i = 0; i < peers; i++)
        {
                for(int j = 0; j < 4; j++)
			mpz_clear(buffer[i][j]);
                free(buffer[i]);
        }
        free(buffer);
	
	//return the result 
	if(z == 1){
		//printf("%d\n", 0); 	
		return 0; 
	}
	else{
		result = v * pow(2, p); 
		if(s == 1){
			//printf("%f\n", -result); 
			return -result; 
		}
		else{
			//printf("%f\n", result);
			return result; 
		}
	}
	
}


//for integer variable I/O
void SMC_Utils::smc_input(int id, int* var, std::string type, int threadID){
	std::string line;
	std::vector<std::string> tokens;
	std::getline(inputStreams[id-1], line);
	tokens = splitfunc(line.c_str(), "=");
	*var = atoi(tokens[1].c_str());
}

void SMC_Utils::smc_input(int id, mpz_t* var, std::string type, int threadID){
	std::string line;
	std::vector<std::string> tokens;
	std::getline(inputStreams[id-1], line);
	tokens = splitfunc(line.c_str(), "=");
	mpz_set_str(*var, tokens[1].c_str(), base);
}

void SMC_Utils::smc_output(int id, int* var, std::string type, int threadID){
	std::string value;
	std::stringstream s; 
	s << *var;  
	outputStreams[id-1] << s.str() + "\n";
	outputStreams[id-1].flush();  
}

void SMC_Utils::smc_output(int id, mpz_t* var, std::string type, int threadID){
	//smc_open(*var, threadID); 
	std::string value; 
        value = mpz_get_str(NULL, base, *var);
	outputStreams[id-1] << value + "\n";
	outputStreams[id-1].flush();  
}
//for float variable I/O 
void SMC_Utils::smc_input(int id, float *var, std::string type, int threadID){
        std::string line;
        std::vector<std::string> tokens;
        std::getline(inputStreams[id-1], line);
	tokens = splitfunc(line.c_str(), "=");
        *var = atof(tokens[1].c_str());
}

void SMC_Utils::smc_input(int id, mpz_t** var, std::string type, int threadID){
        std::string line;
        std::vector<std::string> temp;
        std::vector<std::string> tokens;
        std::getline(inputStreams[id-1], line);
	temp = splitfunc(line.c_str(), "=");
	tokens = splitfunc(temp[1].c_str(), ",");
        for(int i=0; i<4; i++)
           mpz_set_str((*var)[i], tokens[i].c_str(), base);
}

void SMC_Utils::smc_output(int id, float *var, std::string type, int threadID){
        std::string value;
        std::stringstream s;
	
	s << *var; 
	outputStreams[id-1] << s.str() + "\n";
	outputStreams[id-1].flush(); 
}

void SMC_Utils::smc_output(int id, mpz_t** var, std::string type, int threadID){
        std::string value; 
	//smc_open(*var, threadID); 
	for(int i=0; i<4; i++)
	{
        	value = mpz_get_str(NULL, base, (*var)[i]);
		if(i != 3)
        		outputStreams[id-1] << value+",";
		else
			outputStreams[id-1] << value+"\n"; 
		outputStreams[id-1].flush(); 
	}
}

//one-dimensional int array I/O 
void SMC_Utils::smc_input(int id, mpz_t* var, int size, std::string type, int threadID){
	std:: string line; 
	std::vector<std::string> tokens; 
	std::vector<std::string> temp; 
	std::getline(inputStreams[id-1], line); 
	temp = splitfunc(line.c_str(), "=");
	tokens = splitfunc(temp[1].c_str(), ",");
	for(int i=0; i<size; i++)
		mpz_set_str(var[i], tokens[i].c_str(), base);
}

void SMC_Utils::smc_output(int id, mpz_t* var, int size, std::string type, int threadID){
	std::string value; 
       	for(int i = 0; i < size; i++)
       	{
               	value = mpz_get_str(NULL, base, var[i]);
               	//smc_open(var[i], threadID);
		if(i != size-1)
                   	outputStreams[id-1] << value+",";
               	else
                   	outputStreams[id-1] << value+"\n";
		outputStreams[id-1].flush(); 
        }

}

void SMC_Utils::smc_input(int id, int* var, int size, std::string type, int threadID){
	std:: string line; 
	std::vector<std::string> tokens; 
	std::vector<std::string> temp; 
	std::getline(inputStreams[id-1], line); 
	temp = splitfunc(line.c_str(), "=");
	tokens = splitfunc(temp[1].c_str(), ",");
	for(int i=0; i<size; i++)
		var[i] = atoi(tokens[i].c_str()); 
}

void SMC_Utils::smc_output(int id, int* var, int size, std::string type, int threadID){
        std::string value;
	for(int i = 0; i < size; i++)
	{
		std::stringstream s; 
		s << var[i]; 
		if(i != size-1)
			outputStreams[id-1] << s.str()+","; 
		else
        		outputStreams[id-1] << s.str()+"\n";
		outputStreams[id-1].flush(); 
	}
}

//one-dimensional float array I/O
void SMC_Utils::smc_input(int id, mpz_t** var, int size, std::string type, int threadID){
	std:: string line; 
	std::vector<std::string> tokens; 
	std::vector<std::string> temp; 
	for(int i = 0; i < size; i++)
	{
		std::getline(inputStreams[id-1], line); 
		temp = splitfunc(line.c_str(), "=");
		tokens = splitfunc(temp[1].c_str(), ",");
		for(int j=0; j<4; j++)
			mpz_set_str(var[i][j], tokens[j].c_str(), base);
	}
}
        

void SMC_Utils::smc_output(int id, mpz_t** var, int size, std::string type, int threadID){
	std::string value;
        for(int i = 0; i < size; i++)
        {
                //smc_open(var[i], threadID); 
		for(int j = 0; j < 4; j++)
		{
			value = mpz_get_str(NULL, base, var[i][j]);
                	if(j != 3)
                        	outputStreams[id-1] << value+",";
                	else
                        	outputStreams[id-1] << value+"\n";
			outputStreams[id-1].flush(); 
		}
        }
}

void SMC_Utils::smc_input(int id, float* var, int size, std::string type, int threadID)
{
	std:: string line;
        std::vector<std::string> tokens;
        std::vector<std::string> temp;
        std::getline(inputStreams[id-1], line);
	temp = splitfunc(line.c_str(), "=");
	tokens = splitfunc(temp[1].c_str(), ",");
        for(int i=0; i<size; i++)
                var[i] = atof(tokens[i].c_str());
	
}

void SMC_Utils::smc_output(int id, float* var, int size, std::string type, int threadID)
{
	std::string value;
        for(int i = 0; i < size; i++)
        {
		std::stringstream s; 
		s << var[i]; 
		if(i != size-1)
                        outputStreams[id-1] << s.str()+",";
                else
                        outputStreams[id-1] << s.str()+"\n";
		outputStreams[id-1].flush(); 
        }
}

/* SMC Addition */
void SMC_Utils::smc_add(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	ss->modAdd(result,a,b);
}

void SMC_Utils::smc_add(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t btmp; 
	mpz_init_set_si(btmp, b); 
	smc_add(a, btmp, result, alen, blen, resultlen, type, threadID); 
	mpz_clear(btmp);  
}

void SMC_Utils::smc_add(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t atmp; 
	mpz_init_set_si(atmp, a); 
	smc_add(atmp, b, result, alen, blen, resultlen, type, threadID); 
	mpz_clear(atmp); 
}

void SMC_Utils::smc_add(mpz_t* a, float b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID)
{
	mpz_t* btmp; 
	smc_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp); 	
	smc_add(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID);
	smc_batch_free_operator(&btmp, 4); 
}

void SMC_Utils::smc_add(float a, mpz_t* b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID)
{
	mpz_t* atmp; 
	smc_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp); 	
	smc_add(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, type, threadID); 	
	smc_batch_free_operator(&atmp, 4); 
}

void SMC_Utils::smc_add(mpz_t* a, mpz_t* b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID)
{
	smc_single_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, "+", threadID); 	
}

//batch version of smc_add
void SMC_Utils::smc_add(int* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
	mpz_t* atmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	for(int i = 0; i < size; i++)
		mpz_init_set_si(atmp[i], a[i]); 
	smc_add(atmp, b, alen, blen, result, resultlen, size, type, threadID); 
	smc_batch_free_operator(&atmp, size); 
}

void SMC_Utils::smc_add(mpz_t* a, int* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
	mpz_t* btmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	for(int i = 0; i < size; i++)
		mpz_init_set_si(btmp[i], b[i]); 
	smc_add(a, btmp, alen, blen, result, resultlen, size, type, threadID); 
	smc_batch_free_operator(&btmp, size); 
}

void SMC_Utils::smc_add(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
	ss->modAdd(result, a, b, size); 	
}

void SMC_Utils::smc_add(mpz_t** a, float* b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int resultlen_sig, int resultlen_exp,  int size, std::string type, int threadID)
{
	mpz_t** btmp; 
	smc_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size);
	smc_add(a, btmp, alen_sig, alen_exp, alen_sig, alen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID); 
	smc_batch_free_operator(&btmp, size); 	
}

void SMC_Utils::smc_add(float* a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int resultlen_sig, int resultlen_exp,  int size, std::string type, int threadID)
{
	mpz_t** atmp; 
	smc_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size);
	smc_add(atmp, b, blen_sig, blen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID); 
	smc_batch_free_operator(&atmp, size); 	
}

void SMC_Utils::smc_add(mpz_t** a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int resultlen_sig, int resultlen_exp,  int size, std::string type, int threadID)
{
	smc_batch_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "+", threadID); 
}


void SMC_Utils::smc_set(mpz_t* a, mpz_t* result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID){
	mpz_t** results = (mpz_t**)malloc(sizeof(mpz_t*)); 
	results[0] = (mpz_t*)malloc(sizeof(mpz_t) * 4); 	
	for(int i = 0; i < 4; i++)
		mpz_init_set(results[0][i], a[i]); 
	smc_process_results(results, resultlen_sig, resultlen_exp, alen_sig, alen_exp, 1, threadID); 	
	for(int i = 0; i < 4; i++)
		mpz_set(result[i], results[0][i]);
	smc_batch_free_operator(&results, 1); 	
}

void SMC_Utils::smc_set(mpz_t** a, mpz_t** result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID){
	for(int i = 0; i < size; i++)
		smc_set(a[i], result[i], alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID); 
}

void SMC_Utils::smc_set(float a, mpz_t* result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID){
	convertFloat(a, resultlen_sig, resultlen_exp, &result);
	for(int i = 0; i < 4; i++)
		ss->modAdd(result[i], result[i], (long)0); 
}

void SMC_Utils::smc_set(mpz_t a, mpz_t result, int alen, int resultlen, std::string type, int threadID){
	mpz_init_set(result, a); 
}

void SMC_Utils::smc_set(mpz_t* a, mpz_t* result, int alen, int resultlen, int size, std::string type, int threadID){
	for(int i = 0; i < size; i++)
		smc_set(a[i], result[i], alen, resultlen, type, threadID); 
}

// this routine should implement in a way that result = a + share[0]
void SMC_Utils::smc_set(int a, mpz_t result, int alen, int resultlen, std::string type, int threadID){
	mpz_t value; 
	mpz_init_set_si(value, a);
	mpz_set_ui(result, 0);  
	ss->modAdd(result, result, value);
	mpz_clear(value);  
}

void SMC_Utils::smc_priv_eval(mpz_t a, mpz_t b, mpz_t cond, int threadID){
	 mpz_t* results = (mpz_t*)malloc(sizeof(mpz_t));
         mpz_t* op1 = (mpz_t*)malloc(sizeof(mpz_t));
         mpz_t* op2 = (mpz_t*)malloc(sizeof(mpz_t));
         mpz_init(op1[0]);
         mpz_init_set(op2[0], cond);
         mpz_init(results[0]);
	
	 ss->modSub(op1[0], a, b); 
         Mul->doOperation(results, op1, op2, 1, threadID);
         ss->modAdd(a, results[0], b);
	 
	 smc_batch_free_operator(&op1, 1);
         smc_batch_free_operator(&op2, 1);
         smc_batch_free_operator(&results, 1);
}

void SMC_Utils::smc_priv_eval(mpz_t* a, mpz_t* b, mpz_t cond, int threadID){
	 mpz_t* results = (mpz_t*)malloc(sizeof(mpz_t) * 4);
         mpz_t* op1 = (mpz_t*)malloc(sizeof(mpz_t) * 4);
         mpz_t* op2 = (mpz_t*)malloc(sizeof(mpz_t) * 4);
	 for(int i = 0; i < 4; i++){
	 	mpz_init(op1[i]); 
		ss->modSub(op1[i], a[i], b[i]); 
		mpz_init_set(op2[i], cond); 
		mpz_init(results[i]); 
	 }
	
         Mul->doOperation(results, op1, op2, 4, threadID);
	 ss->modAdd(a, results, b, 4);
         
         smc_batch_free_operator(&op1, 4);
         smc_batch_free_operator(&op2, 4);
         smc_batch_free_operator(&results, 4);
}

/* SMC Subtraction */
void SMC_Utils::smc_sub(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	ss->modSub(result,a,b);
}

void SMC_Utils::smc_sub(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t btmp;
	mpz_init_set_si(btmp, b); 
	smc_sub(a, btmp, result, alen, blen, resultlen, type, threadID); 	
	mpz_clear(btmp); 	
}

void SMC_Utils::smc_sub(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t atmp;
	mpz_init_set_si(atmp, a); 
	smc_sub(atmp, b, result, alen, blen, resultlen, type, threadID); 	
	mpz_clear(atmp); 	
}

void SMC_Utils::smc_sub(mpz_t* a, float b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID)
{
        mpz_t* btmp;
	smc_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp); 	
	smc_sub(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID); 
	smc_batch_free_operator(&btmp, 4); 	
}

void SMC_Utils::smc_sub(float a, mpz_t* b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID)
{
        mpz_t* atmp; 
	smc_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp); 	
	smc_sub(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, type, threadID);
	smc_batch_free_operator(&atmp, 4); 
}

void SMC_Utils::smc_sub(mpz_t* a, mpz_t* b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID)
{
	//initialization
	mpz_t* b1 = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
	for(int i = 0; i < 4; i++)
		mpz_init_set(b1[i], b[i]);
	mpz_t one; 
	mpz_init_set_ui(one, 1); 

	//computation
	ss->modSub(b1[3], one, b[3]); 
	smc_single_fop_arithmetic(result, a, b1, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, "+", threadID); 
	
	//free the memory
	smc_batch_free_operator(&b1, 4); 
	mpz_clear(one); 
}

//batch operations of subtraction 
void SMC_Utils::smc_sub(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
	ss->modSub(result, a, b, size); 	
}

void SMC_Utils::smc_sub(int* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
	mpz_t* atmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	for(int i = 0; i < size; i++)
		mpz_init_set_si(atmp[i], a[i]); 
	smc_sub(atmp, b, alen, blen, result, resultlen, size, type, threadID); 
	smc_batch_free_operator(&atmp, size); 
}

void SMC_Utils::smc_sub(mpz_t* a, int* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
	mpz_t* btmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	for(int i = 0; i < size; i++)
		mpz_init_set_si(btmp[i], b[i]); 
	smc_sub(a, btmp, alen, blen, result, resultlen, size, type, threadID); 
	smc_batch_free_operator(&btmp, size); 
}


void SMC_Utils::smc_sub(mpz_t** a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int resultlen_sig, int resultlen_exp,  int size, std::string type, int threadID)
{
	mpz_t** b1 = (mpz_t**)malloc(sizeof(mpz_t*) * size);
        for(int i = 0; i < size; i++)
        {
                b1[i] = (mpz_t*)malloc(sizeof(mpz_t) * 4);
                for(int j = 0; j < 4; j++)
                        mpz_init_set(b1[i][j], b[i][j]);
        }
        for(int i = 0; i < size; i++)
                ss->modSub(b1[i][3], (long)1, b1[i][3]);
        
	smc_batch_fop_arithmetic(result, a, b1, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "+", threadID);
	//free the memory	
	smc_batch_free_operator(&b1, size); 
}

void SMC_Utils::smc_sub(float* a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int resultlen_sig, int resultlen_exp,  int size, std::string type, int threadID)
{
	mpz_t** atmp; 
	smc_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size);
	smc_sub(atmp, b, blen_sig, blen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID); 
	smc_batch_free_operator(&atmp, size); 	
}

void SMC_Utils::smc_sub(mpz_t** a, float* b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int resultlen_sig, int resultlen_exp,  int size, std::string type, int threadID)
{
	mpz_t** btmp; 
	smc_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size);
	smc_sub(a, btmp, alen_sig, alen_exp, alen_sig, alen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID); 
	smc_batch_free_operator(&btmp, size); 	
}

/* SMC Multiplication */
void SMC_Utils::smc_mult(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
		mpz_t* results = (mpz_t*)malloc(sizeof(mpz_t)); 
		mpz_t* op1 = (mpz_t*)malloc(sizeof(mpz_t)); 
		mpz_t* op2 = (mpz_t*)malloc(sizeof(mpz_t));
		 
		mpz_init_set(op1[0], a); 
		mpz_init_set(op2[0], b); 
		mpz_init(results[0]); 
		
		Mul->doOperation(results, op1, op2, 1, threadID);
		mpz_set(result, results[0]); 
		
		//free the memory
		smc_batch_free_operator(&op1, 1); 
		smc_batch_free_operator(&op2, 1); 
		smc_batch_free_operator(&results, 1); 
}

/******************************************************/
void SMC_Utils::smc_mult(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t btmp; 
	mpz_init(btmp); 
	ss->modAdd(btmp, btmp, b);
	ss->modMul(result, a, btmp);  
	//smc_mult(a, btmp, result, alen, blen, resultlen, type, threadID); 
	mpz_clear(btmp); 
}

void SMC_Utils::smc_mult(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t atmp; 
	mpz_init(atmp); 
	ss->modAdd(atmp, atmp, a); 
	ss->modMul(result, atmp, b); 
	//smc_mult(atmp, b, result, alen, blen, resultlen, type, threadID); 
	mpz_clear(atmp); 
}

void SMC_Utils::smc_mult(mpz_t* a, mpz_t*b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID){
	smc_single_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, "*", threadID); 
}

void SMC_Utils::smc_mult(float a, mpz_t* b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID){
        mpz_t* atmp;
	smc_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp); 
	smc_mult(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, type, threadID); 
	smc_batch_free_operator(&atmp, 4); 	
}

void SMC_Utils::smc_mult(mpz_t* a, float b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID){
        mpz_t* btmp;
       	smc_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp); 
	smc_mult(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID);  
	smc_batch_free_operator(&btmp, 4); 
}

//private float * private int (needs more examinations)
void SMC_Utils::smc_mult(mpz_t* a, mpz_t b, mpz_t* result, int alen_sig, int alen_exp, int blen, int resultlen_sig, int resultlen_exp, std::string type, int threadID){
	mpz_t b1, one; 
	mpz_init(b1); 
	mpz_init_set_ui(one, 1);
	//start computation 
	ss->modSub(b1, one, b); 
	smc_lor(a[2], b1, result[2], 1, 1, 1, type, threadID); 
	//free the memory
	mpz_clear(b1);
	mpz_clear(one); 
}

void SMC_Utils::smc_mult(int* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
	mpz_t* atmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	for(int i = 0; i < size; i++){
		mpz_init_set_si(atmp[i], a[i]);
		ss->modMul(result[i], atmp[i], b[i]); 
	} 
	smc_batch_free_operator(&atmp, size); 
}

void SMC_Utils::smc_mult(mpz_t* a, int* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
	mpz_t* btmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	for(int i = 0; i < size; i++){
		mpz_init_set_si(btmp[i], b[i]); 
		ss->modMul(result[i], a[i], btmp[i]); 
	}
	//smc_mult(a, btmp, alen, blen, result, resultlen, size, type, threadID); 
	smc_batch_free_operator(&btmp, size); 
}

void SMC_Utils::smc_mult(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
	Mul->doOperation(result, a, b, size, threadID); 	
}

void SMC_Utils::smc_mult(mpz_t** a, float* b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int resultlen_sig, int resultlen_exp,  int size, std::string type, int threadID)
{
	mpz_t** btmp; 
	smc_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size);
	smc_mult(a, btmp, alen_sig, alen_exp, alen_sig, alen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID); 
	smc_batch_free_operator(&btmp, size); 	
}

void SMC_Utils::smc_mult(float* a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int resultlen_sig, int resultlen_exp,  int size, std::string type, int threadID)
{
	mpz_t** atmp; 
	smc_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size);
	smc_mult(atmp, b, blen_sig, blen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID); 
	smc_batch_free_operator(&atmp, size); 	
}

void SMC_Utils::smc_mult(mpz_t** a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int resultlen_sig, int resultlen_exp,  int size, std::string type, int threadID)
{
	smc_batch_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "*", threadID); 
}

/* SMC Division*/
void SMC_Utils::smc_div(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t* results = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t* op1 = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t* op2 = (mpz_t*)malloc(sizeof(mpz_t)); 
	
	mpz_init_set(op1[0], a); 
	mpz_init_set(op2[0], b); 
	mpz_init(results[0]); 
	
	//alen and blen could be negative when a and b are coverted from public values 
	Idiv->doOperation(results, op1, op2, resultlen, 1, threadID); 
	mpz_set(result, results[0]); 
	
	//free the memory
	smc_batch_free_operator(&op1, 1); 
	smc_batch_free_operator(&op2, 1); 
	smc_batch_free_operator(&results, 1); 
}

void SMC_Utils::smc_div(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t* results = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t* op1 = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t* op2 = (mpz_t*)malloc(sizeof(mpz_t)); 
	
	mpz_init_set(op1[0], a); 
	mpz_init_set_si(op2[0], b); 
	mpz_init(results[0]); 
	
	Idiv->doOperationPub(results, op1, op2, resultlen, 1, threadID); 
	mpz_set(result, results[0]); 
	
	//free the memory
	smc_batch_free_operator(&op1, 1); 
	smc_batch_free_operator(&op2, 1); 
	smc_batch_free_operator(&results, 1); 
}

void SMC_Utils::smc_div(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t zero, atmp;  
	mpz_init_set_ui(zero, 0); 
	mpz_init_set_si(atmp, a); 
	ss->modAdd(atmp, atmp, zero);
	smc_div(atmp, b, result, alen, blen, resultlen, type, threadID);  
	
	//free the memory
	mpz_clear(zero); 
	mpz_clear(atmp); 
}

void SMC_Utils::smc_div(mpz_t* a, mpz_t* b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID)
{
	smc_single_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, "/", threadID); 	
}

void SMC_Utils::smc_div(mpz_t* a, float b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID)
{
	mpz_t** results = (mpz_t**)malloc(sizeof(mpz_t*)); 
	mpz_t** as = (mpz_t**)malloc(sizeof(mpz_t*)); 
	mpz_t** bs; 
	
	as[0] = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
	results[0] = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
	smc_batch_convert_to_private_float(&b, &bs, alen_sig, alen_exp, 1); 
	
	for(int i = 0; i < 4; i++)
	{
		mpz_init(results[0][i]); 
		mpz_init_set(as[0][i], a[i]); 
	}
	
	Fldiv->doOperationPub(as, bs, results, alen_sig, 1, threadID); 
	smc_process_results(results, resultlen_sig, resultlen_exp, alen_sig, alen_exp, 1, threadID); 	
	
	for(int i = 0; i < 4; i++) 
		mpz_set(result[i], results[0][i]); 
	
	//free the memory
	smc_batch_free_operator(&as, 1); 
	smc_batch_free_operator(&bs, 1); 
	smc_batch_free_operator(&results, 1); 
}
 
void SMC_Utils::smc_div(float a, mpz_t* b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID)
{
        mpz_t* atmp;
        smc_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp); 
	smc_div(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, type, threadID);
	smc_batch_free_operator(&atmp, 4);         
}

//batch operations of division 
void SMC_Utils::smc_div(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
	Idiv->doOperation(result, a, b, resultlen, size, threadID); 
}

void SMC_Utils::smc_div(mpz_t** a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int resultlen_sig, int resultlen_exp,  int size, std::string type, int threadID)
{
	smc_batch_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "/", threadID);  
}

void SMC_Utils::smc_div(int* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID){
	mpz_t* atmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	for(int i = 0; i < size; i++)
		mpz_init_set_si(atmp[i], a[i]); 
	smc_div(atmp, b, alen, blen, result, resultlen, size, type, threadID); 
	smc_batch_free_operator(&atmp, size); 
}

void SMC_Utils::smc_div(mpz_t* a, int* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID){
	mpz_t* b_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	for(int i = 0; i < size; i++)
		mpz_init_set_si(b_tmp[i], b[i]); 
	Idiv->doOperationPub(result, a, b_tmp, resultlen, size, threadID); 
	smc_batch_free_operator(&b_tmp, size); 
}

void SMC_Utils::smc_div(mpz_t** a, float* b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int resultlen_sig, int resultlen_exp,  int size, std::string type, int threadID)
{
        mpz_t** btmp; 
	smc_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size); 	
	Fldiv->doOperationPub(a, btmp, result, alen_sig, size, threadID); 
	smc_process_results(result, resultlen_sig, resultlen_exp, alen_sig, alen_exp, size, threadID); 	
	smc_batch_free_operator(&btmp, size); 
}

void SMC_Utils::smc_div(float* a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t** result, int resultlen_sig, int resultlen_exp,  int size, std::string type, int threadID)
{
	mpz_t** atmp; 
	smc_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size);
	smc_div(atmp, b, blen_sig, blen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID); 
	smc_batch_free_operator(&atmp, size); 	
}
void SMC_Utils::smc_compute_len(int alen, int blen, int* len)
{
	*len = alen >= blen ? alen : blen; 
}

void SMC_Utils::smc_process_operands(mpz_t** a1, mpz_t** b1, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int* len_sig, int* len_exp, int size)
{
	mpz_t** a = (mpz_t**)malloc(sizeof(mpz_t*) * 4); 
	mpz_t** b = (mpz_t**)malloc(sizeof(mpz_t*) * 4); 
	
	for(int i = 0; i < 4; i++){
		a[i] = (mpz_t*)malloc(sizeof(mpz_t) * size); 
		b[i] = (mpz_t*)malloc(sizeof(mpz_t) * size); 
		for(int j = 0; j < size; j++){
			mpz_init_set(a[i][j], a1[j][i]); 
			mpz_init_set(b[i][j], b1[j][i]); 
		}
	}
	
	int sig = alen_sig >= blen_sig ? alen_sig : blen_sig; 
	int exp = alen_exp >= blen_exp ? alen_exp : blen_exp; 
	
	int diff = (int)abs(alen_sig - blen_sig);
	
	mpz_t pow; 
	mpz_init(pow); 
	mpz_ui_pow_ui(pow, 2, exp); 
	
	//compute the log(pow+diff) which will be the final value for *len_exp
	if(mpz_cmp_ui(pow, diff) >= 0) 
		exp++; 
	else
		exp = ceil(log(diff))+1; 
	
	*len_sig = sig; 
	*len_exp = exp;
 
	//convert both opeators a and b to ensure their significands and exponents will be in the correct range (sig, exp)
	if(alen_sig < sig){
		mpz_ui_pow_ui(pow, 2, sig-alen_sig); 	
		ss->modMul(a[0], a[0], pow, size); 
		ss->modSub(a[1], a[1], diff, size); 		
	}

	if(blen_sig < sig){
		mpz_ui_pow_ui(pow, 2, sig-blen_sig); 
		ss->modMul(b[0], b[0], pow, size); 
		ss->modSub(b[1], b[1], diff, size); 
	}

	//copy the result back to a1 and b1 
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < size; j++){
			mpz_set(a1[j][i], a[i][j]); 
			mpz_set(b1[j][i], b[i][j]); 
		}

	//free the memory
	mpz_clear(pow); 
	
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < size; j++){
			mpz_clear(a[i][j]); 
			mpz_clear(b[i][j]); 
		}
		free(a[i]); 
		free(b[i]); 
	}
	free(a); 
	free(b); 

	return; 	 
}

void SMC_Utils::smc_process_results(mpz_t** result1, int resultlen_sig, int resultlen_exp, int len_sig, int len_exp, int size, int threadID)
{
	mpz_t** result = (mpz_t**)malloc(sizeof(mpz_t*) * 4); 
	for(int i = 0; i < 4; i++){
		result[i] = (mpz_t*)malloc(sizeof(mpz_t) * size); 
		for(int j = 0; j < size; j++)
			mpz_init_set(result[i][j], result1[j][i]);  
	}

	//truncate results by 2^{len_sig-resultlen_sig}
	if(resultlen_sig < len_sig){
		T->doOperation(result[0], result[0], len_sig, resultlen_sig, size, threadID); 	
		ss->modAdd(result[1], result[1], len_sig-resultlen_sig, size); 
	}

	//multiply results by 2^{resultlen_sig-len_sig}
	else if(resultlen_sig > len_sig){
		mpz_t pow; 
		mpz_init(pow); 
		mpz_ui_pow_ui(pow, 2, resultlen_sig-len_sig);
		ss->modMul(result[0], result[0], pow, size); 
		ss->modSub(result[1], result[1], resultlen_sig-len_sig, size);  
		mpz_clear(pow); 
	}

	//copy the result back to result1 	
	for(int i = 0; i < 4; i++)
		for(int j = 0; j < size; j++)
			mpz_set(result1[j][i], result[i][j]); 
	
	//free the memory
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < size; j++)
			mpz_clear(result[i][j]); 
		free(result[i]); 
	}
	free(result); 
}

void SMC_Utils::smc_single_fop_arithmetic(mpz_t* result, mpz_t* a, mpz_t* b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, std::string op, int threadID){
	
	mpz_t** results = (mpz_t**)malloc(sizeof(mpz_t*)); 
	mpz_t** as = (mpz_t**)malloc(sizeof(mpz_t*)); 
	mpz_t** bs = (mpz_t**)malloc(sizeof(mpz_t*)); 
	
	results[0] = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
	as[0] = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
	bs[0] = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
	
	for(int i = 0; i < 4; i++)
	{
		mpz_init(results[0][i]);
		mpz_init_set(as[0][i], a[i]); 
		mpz_init_set(bs[0][i], b[i]); 
	}
	
	/*********************************************************/
	int len_sig = 0, len_exp = 0; 
	smc_process_operands(as, bs, alen_sig, alen_exp, blen_sig, blen_exp, &len_sig, &len_exp, 1); 
	
	/*********************************************************/		
	
	if(!strcmp(op.c_str(), "*"))
		Flmult->doOperation(as, bs, results, len_sig, 1, threadID); 
	else if(!strcmp(op.c_str(), "+"))
		Fladd->doOperation(as, bs, results, len_sig, len_exp, 1, threadID); 
	else if(!strcmp(op.c_str(), "/"))
		Fldiv->doOperation(as, bs, results, len_sig, 1, threadID); 
	
	//convert the result with size len_sig and len_exp to resultlen_sig and resultlen_exp 
	smc_process_results(results, resultlen_sig, resultlen_exp, len_sig, len_exp, 1, threadID); 	
	for(int i = 0; i < 4; i++) 
		mpz_set(result[i], results[0][i]); 
	//free the memory
	smc_batch_free_operator(&results, 1); 
	smc_batch_free_operator(&as, 1); 
	smc_batch_free_operator(&bs, 1); 
}

void SMC_Utils::smc_single_fop_comparison(mpz_t result, mpz_t* a, mpz_t* b, int resultlen, int alen_sig, int alen_exp, int blen_sig, int blen_exp, std::string op, int threadID){
	
	mpz_t* results = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t** as = (mpz_t**)malloc(sizeof(mpz_t*)); 
	mpz_t** bs = (mpz_t**)malloc(sizeof(mpz_t*)); 

	as[0] = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
	bs[0] = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
	
	for(int i = 0; i < 4; i++)
	{
		mpz_init_set(as[0][i], a[i]); 
		mpz_init_set(bs[0][i], b[i]); 
	}

	mpz_init(results[0]); 

	/***********************************************/
	int len_sig = 0, len_exp = 0; 
	smc_process_operands(as, bs, alen_sig, alen_exp, blen_sig, blen_exp, &len_sig, &len_exp, 1); 
	/***********************************************/	
	
	if(!strcmp(op.c_str(), "<0"))
		Flltz->doOperation(as, bs, results, len_sig, len_exp, 1, threadID); 
	else if(!strcmp(op.c_str(), "=="))
		Fleqz->doOperation(as, bs, results, len_sig, len_exp, 1, threadID); 
	
	mpz_set(result, results[0]); 
	
	//free the memory
	smc_batch_free_operator(&results, 1);
	smc_batch_free_operator(&as, 1); 
	smc_batch_free_operator(&bs, 1); 
}

void SMC_Utils::smc_batch_fop_arithmetic(mpz_t** result, mpz_t** a, mpz_t** b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID){

	int len_sig = 0, len_exp = 0; 
	smc_process_operands(a, b, alen_sig, alen_exp, blen_sig, blen_exp, &len_sig, &len_exp, size); 
	if(!strcmp(op.c_str(), "*"))
		Flmult->doOperation(a, b, result, len_sig, size, threadID); 
	else if(!strcmp(op.c_str(), "+"))
		Fladd->doOperation(a, b, result, len_sig, len_exp, size, threadID); 
	else if(!strcmp(op.c_str(), "/"))
		Fldiv->doOperation(a, b, result, len_sig, size, threadID); 
	smc_process_results(result, resultlen_sig, resultlen_exp, len_sig, len_exp, size, threadID); 
}

void SMC_Utils::smc_batch_fop_comparison(mpz_t* result, mpz_t** a, mpz_t** b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID){

	int len_sig = 0, len_exp = 0; 
	smc_process_operands(a, b, alen_sig, alen_exp, blen_sig, blen_exp, &len_sig, &len_exp, size); 
	if(!strcmp(op.c_str(), "<0"))
		Flltz->doOperation(a, b, result, len_sig, len_exp, size, threadID);
	else if(!strcmp(op.c_str(), "=="))
		Fleqz->doOperation(a, b, result, len_sig, len_exp, size, threadID);
}

/* All Comparisons */
void SMC_Utils::smc_lt(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t sub;
	mpz_init(sub);
	ss->modSub(sub,a,b);
	mpz_t* results = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t* subs = (mpz_t*)malloc(sizeof(mpz_t));

	mpz_init_set(subs[0], sub); 
	mpz_init(results[0]); 
	
	int len = 0; 
	smc_compute_len(alen, blen, &len);
	Lt->doOperation(results, subs, len, 1, threadID);
	mpz_set(result, results[0]);
	
	//free the memory
	mpz_clear(sub); 
	smc_batch_free_operator(&subs, 1); 
	smc_batch_free_operator(&results, 1); 
}

void SMC_Utils::smc_lt(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t bs; 
	mpz_init_set_si(bs, b); 
	smc_lt(a, bs, result, alen, blen, resultlen, "int", threadID); 
	mpz_clear(bs); 
}

void SMC_Utils::smc_lt(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t as; 
	mpz_init_set_si(as, a); 
	smc_lt(as, b, result, alen, blen, resultlen, "int", threadID); 
	mpz_clear(as); 
}

void SMC_Utils::smc_lt(mpz_t* a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID)
{
	smc_single_fop_comparison(result, a, b, resultlen, alen_sig, alen_exp, blen_sig, blen_exp, "<0", threadID); 
}

void SMC_Utils::smc_lt(mpz_t* a, float b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID)
{
	mpz_t* btmp; 
	smc_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp); 
	smc_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, alen_sig, alen_exp, "<0", threadID);
	smc_batch_free_operator(&btmp, 4); 
}

void SMC_Utils::smc_lt(float a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID)
{
        mpz_t* atmp; 
	smc_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp); 
        smc_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "<0", threadID);
	smc_batch_free_operator(&atmp, 4); 
}

//batch operations of comparisons 
void SMC_Utils::smc_lt(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
	mpz_t* sub = (mpz_t*)malloc(sizeof(mpz_t) * size);
	for(int i=0; i<size; ++i)
		mpz_init(sub[i]);
	int len = 0; 
	smc_compute_len(alen, blen, &len); 
	ss->modSub(sub, a, b, size);
	Lt->doOperation(result, sub, len, size, threadID);
	smc_batch_free_operator(&sub, size); 
}

void SMC_Utils::smc_lt(mpz_t** a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
	smc_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "<0", threadID);  
}

void SMC_Utils::smc_gt(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t sub;
	mpz_init(sub);
	ss->modSub(sub,b,a);

	mpz_t* results = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t* subs = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_init_set(subs[0], sub); 
	mpz_init(results[0]); 

	/********************************************/
	int len = 0; 
	smc_compute_len(alen, blen, &len); 
	/********************************************/
	Lt->doOperation(results, subs, len, 1, threadID);	
	mpz_set(result, results[0]); 

	//free the memory	
	mpz_clear(sub); 
	smc_batch_free_operator(&subs, 1); 
	smc_batch_free_operator(&results, 1); 
}

void SMC_Utils::smc_gt(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t bs; 
	mpz_init_set_si(bs, b); 
	smc_gt(a, bs, result, alen, blen, resultlen, type, threadID); 
	mpz_clear(bs); 
}

void SMC_Utils::smc_gt(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t as; 
	mpz_init_set_si(as, a); 
	smc_gt(as, b, result, alen, blen, resultlen, type, threadID); 
	mpz_clear(as); 
}

void SMC_Utils::smc_gt(mpz_t* a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID)
{
	smc_single_fop_comparison(result, b, a, resultlen, blen_sig, blen_exp, alen_sig, alen_exp, "<0", threadID); 
}

void SMC_Utils::smc_gt(float a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID)
{
        mpz_t* atmp;
	smc_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp); 
        smc_single_fop_comparison(result, b, atmp, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "<0", threadID);
	smc_batch_free_operator(&atmp, 4); 
}

void SMC_Utils::smc_gt(mpz_t* a, float b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID)
{
        mpz_t* btmp;
        smc_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp); 
        smc_single_fop_comparison(result, btmp, a, resultlen, alen_sig, alen_exp, alen_sig, alen_exp, "<0", threadID);
	smc_batch_free_operator(&btmp, 4); 
}

//batch operations of gt
void SMC_Utils::smc_gt(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
 	mpz_t* sub = (mpz_t*)malloc(sizeof(mpz_t) * size);
        for(int i = 0; i < size; ++i)
                 mpz_init(sub[i]);
	int len =  0; 
	smc_compute_len(alen, blen, &len); 
        ss->modSub(sub, b, a, size);
        Lt->doOperation(result, sub, len, size, threadID);
	smc_batch_free_operator(&sub, size); 
}

void SMC_Utils::smc_gt(mpz_t** a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
	smc_batch_fop_comparison(result, b, a, resultlen, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID); 
}


void SMC_Utils::smc_leq(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t sub;
	mpz_init(sub);
	ss->modSub(sub,b,a);

	mpz_t* results = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t* subs = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_init_set(subs[0], sub); 
	mpz_init(results[0]); 
	
	int len = 0; 
	smc_compute_len(alen, blen, &len); 
	Lt->doOperation(results, subs, len, 1, threadID);
	mpz_set(result, results[0]); 
	ss->modSub(result, 1, result);

	//free the memory
	mpz_clear(sub); 
	smc_batch_free_operator(&results, 1); 
	smc_batch_free_operator(&subs, 1); 
}

void SMC_Utils::smc_leq(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t bs; 
	mpz_init_set_si(bs, b); 
	smc_leq(a, bs, result, alen, blen, resultlen, type, threadID); 
	mpz_clear(bs); 
}

void SMC_Utils::smc_leq(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t as; 
	mpz_init_set_si(as, a); 
	smc_leq(as, b, result, alen, blen, resultlen, type, threadID); 
	mpz_clear(as); 
}

void SMC_Utils::smc_leq(mpz_t* a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID)
{
	smc_single_fop_comparison(result, b, a, resultlen, blen_sig, blen_exp, alen_sig, alen_exp, "<0", threadID); 
	ss->modSub(result, 1, result); 
}

void SMC_Utils::smc_leq(float a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID)
{
  	mpz_t* atmp; 
        smc_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp); 
	smc_single_fop_comparison(result, b, atmp, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "<0", threadID); 
	ss->modSub(result, 1, result);
	smc_batch_free_operator(&atmp, 4); 	
}

void SMC_Utils::smc_leq(mpz_t* a, float b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID)
{
        mpz_t* btmp; 
	smc_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp); 
        smc_single_fop_comparison(result, btmp, a, resultlen, alen_sig, alen_exp, alen_sig, alen_exp, "<0", threadID); 
        ss->modSub(result, 1, result);  
	smc_batch_free_operator(&btmp, 4);         
}

//batch operations of leq 
void SMC_Utils::smc_leq(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
	  mpz_t* sub = (mpz_t*)malloc(sizeof(mpz_t) * size);
          for(int i = 0; i < size; i++)
                   mpz_init(sub[i]);
          ss->modSub(sub, b, a, size);
	  int len = 0; 
	  smc_compute_len(alen, blen, &len); 
          Lt->doOperation(result, sub, len, size, threadID);
          ss->modSub(result, 1, result, size);
	  smc_batch_free_operator(&sub, size); 
}

void SMC_Utils::smc_leq(mpz_t** a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
	smc_batch_fop_comparison(result, b, a, resultlen, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID);
        ss->modSub(result, 1, result, size);
}

void SMC_Utils::smc_geq(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t sub;
	mpz_init(sub);
	ss->modSub(sub, a, b);

	mpz_t* results = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t* subs = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_init_set(subs[0], sub); 
	mpz_init(results[0]); 

	int len = 0; 
	smc_compute_len(alen, blen, &len); 
	Lt->doOperation(results, subs, len, 1, threadID);
	mpz_set(result, results[0]); 
	ss->modSub(result, 1, result);

	mpz_clear(sub); 
	smc_batch_free_operator(&results, 1); 
	smc_batch_free_operator(&subs, 1); 
	
}

void SMC_Utils::smc_geq(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t bs; 
	mpz_init_set_si(bs, b); 
	smc_geq(a, bs, result, alen, blen, resultlen, type, threadID); 
	mpz_clear(bs);
}

void SMC_Utils::smc_geq(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t as; 
	mpz_init_set_si(as, a); 
	smc_geq(as, b, result, alen, blen, resultlen, type, threadID); 
	mpz_clear(as);
}

void SMC_Utils::smc_geq(mpz_t* a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID)
{
	smc_single_fop_comparison(result, a, b, resultlen, alen_sig, alen_exp, blen_sig, blen_exp, "<0", threadID); 
	ss->modSub(result, 1, result); 
}

void SMC_Utils::smc_geq(float a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID)
{
        mpz_t* atmp;
	smc_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp);  
	smc_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "<0", threadID); 
	ss->modSub(result, 1, result);
	smc_batch_free_operator(&atmp, 4); 	
}

void SMC_Utils::smc_geq(mpz_t* a, float b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID)
{
        mpz_t* btmp;
	smc_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp); 
        smc_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, blen_sig, blen_exp, "<0", threadID);
        ss->modSub(result, 1, result);
	smc_batch_free_operator(&btmp, 4); 
}

//batch operations of geq 
void SMC_Utils::smc_geq(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
	  mpz_t* sub = (mpz_t*)malloc(sizeof(mpz_t) * size);
          for(int i = 0; i < size; i++)
                  mpz_init(sub[i]);
          ss->modSub(sub, a, b, size);
	  
	  int len = 0; 
	  smc_compute_len(alen, blen, &len); 
          Lt->doOperation(result, sub, len, size, threadID);
          ss->modSub(result, 1, result, size);
	  smc_batch_free_operator(&sub, size); 
}

void SMC_Utils::smc_geq(mpz_t** a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
	smc_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "<0", threadID); 
	ss->modSub(result, 1, result, size); 
}

// Equality and Inequality
void SMC_Utils::smc_eqeq(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t sub;
	mpz_init(sub);
	ss->modSub(sub, a, b);
	mpz_t* results = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t* subs = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_init_set(subs[0], sub); 
	mpz_init(results[0]); 
	
	int len = 0; 
	smc_compute_len(alen, blen, &len); 
	Eq->doOperation(subs, results, len, 1, threadID);
	mpz_set(result, results[0]);
	
	mpz_clear(sub); 
	smc_batch_free_operator(&results, 1); 
	smc_batch_free_operator(&subs, 1); 
}

void SMC_Utils::smc_eqeq(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t bs; 
	mpz_init_set_si(bs, b); 
	smc_eqeq(a, bs, result, alen, blen, resultlen, type, threadID); 
	mpz_clear(bs); 
}

void SMC_Utils::smc_eqeq(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t as; 
	mpz_init_set_si(as, a); 
	smc_eqeq(as, b, result, alen, blen, resultlen, type, threadID); 
	mpz_clear(as); 
}

void SMC_Utils::smc_eqeq(mpz_t* a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID)
{
	smc_single_fop_comparison(result, a, b, resultlen, alen_sig, alen_exp, blen_sig, blen_exp, "==", threadID); 
}

void SMC_Utils::smc_eqeq(mpz_t* a, float b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID)
{
	mpz_t* btmp; 
        smc_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp); 
	smc_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, alen_sig, alen_exp, "==", threadID); 
	smc_batch_free_operator(&btmp, 4); 
}

void SMC_Utils::smc_eqeq(float a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID)
{
        mpz_t* atmp; 
	smc_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp); 
	smc_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "==", threadID); 
	smc_batch_free_operator(&atmp, 4); 
}

//batch operations of eqeq 
void SMC_Utils::smc_eqeq(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
	mpz_t* sub = (mpz_t*)malloc(sizeof(mpz_t) * size);
        for(int i=0; i<size; ++i)
                  mpz_init(sub[i]);
	int len = 0; 
	smc_compute_len(alen, blen, &len); 
        ss->modSub(sub, a, b, size);
        Eq->doOperation(sub, result, len, size, threadID);
	smc_batch_free_operator(&sub, size); 
}

void SMC_Utils::smc_eqeq(mpz_t** a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
	smc_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "==", threadID); 
}

void SMC_Utils::smc_neq(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t sub;
	mpz_init(sub);
	
	ss->modSub(sub,a,b);
	mpz_t* results = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t* subs = (mpz_t*)malloc(sizeof(mpz_t));
 
	mpz_init_set(subs[0], sub); 
	mpz_init(results[0]); 

	int len = 0; 
	smc_compute_len(alen, blen, &len); 	
	Eq->doOperation(subs, results, len, 1, threadID);
	mpz_set(result, results[0]);
	ss->modSub(result, 1, result);
	
	mpz_clear(sub); 
	smc_batch_free_operator(&results, 1); 
	smc_batch_free_operator(&subs, 1); 	
}

void SMC_Utils::smc_neq(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t bs; 
	mpz_init_set_si(bs, b); 
	smc_neq(a, bs, result, alen, blen, resultlen, type, threadID); 
	mpz_clear(bs); 
}

void SMC_Utils::smc_neq(int a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t as; 
	mpz_init_set_si(as, a); 
	smc_neq(as, b, result, alen, blen, resultlen, type, threadID); 
	mpz_clear(as); 
}

void SMC_Utils::smc_neq(mpz_t* a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID)
{
	smc_single_fop_comparison(result, a, b, resultlen, alen_sig, alen_exp, blen_sig, blen_exp, "==", threadID); 
	ss->modSub(result, 1, result); 
}

void SMC_Utils::smc_neq(float a, mpz_t* b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID)
{
        mpz_t* atmp; 
	smc_single_convert_to_private_float(a, &atmp, blen_sig, blen_exp); 
	smc_single_fop_comparison(result, atmp, b, resultlen, blen_sig, blen_exp, blen_sig, blen_exp, "==", threadID); 
	ss->modSub(result, 1, result); 
	smc_batch_free_operator(&atmp, 4); 	
}

void SMC_Utils::smc_neq(mpz_t* a, float b, mpz_t result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen, std::string type, int threadID)
{
        mpz_t* btmp;
	smc_single_convert_to_private_float(b, &btmp, alen_sig, alen_exp); 
        smc_single_fop_comparison(result, a, btmp, resultlen, alen_sig, alen_exp, alen_sig, alen_exp, "==", threadID); 
        ss->modSub(result, 1, result);  
	smc_batch_free_operator(&btmp, 4);         
}

//batch operations of neq 
void SMC_Utils::smc_neq(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
    	mpz_t* sub = (mpz_t*)malloc(sizeof(mpz_t) * size);
        for(int i=0; i<size; ++i)
                  mpz_init(sub[i]);
	int len = 0; 
	smc_compute_len(alen, blen, &len); 
        ss->modSub(sub, a, b, size);
        Eq->doOperation(sub, result, len, size, threadID);
        ss->modSub(result, 1, result, size);
	smc_batch_free_operator(&sub, size);         
}

void SMC_Utils::smc_neq(mpz_t** a, mpz_t** b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t* result, int resultlen, int size, std::string type, int threadID)
{
	smc_batch_fop_comparison(result, a, b, resultlen, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "==", threadID); 
	ss->modSub(result, 1, result, size); 
}

// Bitwise Operations
void SMC_Utils::smc_land(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t* as = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t* bs = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t* results = (mpz_t*)malloc(sizeof(mpz_t)); 
	
	mpz_init_set(as[0], a); 
	mpz_init_set(bs[0], b); 
	mpz_init(results[0]); 
	BOps->BitAnd(as, bs, results, 1, threadID);
	mpz_set(result, results[0]); 
	
	mpz_clear(as[0]); 
	mpz_clear(bs[0]); 
	mpz_clear(results[0]); 
	free(as); 
	free(bs); 
	free(results); 
}

void SMC_Utils::smc_land(mpz_t* a, mpz_t* b, int size, mpz_t* result, std::string type, int threadID){
	BOps->BitAnd(a, b, result, size, threadID);
}

void SMC_Utils::smc_xor(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t* as = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t* bs = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t* results = (mpz_t*)malloc(sizeof(mpz_t)); 
	
	mpz_init_set(as[0], a); 
	mpz_init_set(bs[0], b); 
	mpz_init(results[0]); 
	BOps->BitXor(as, bs, results, 1, threadID);
	mpz_set(result, results[0]); 
	
	//free the memory	
	mpz_clear(as[0]); 
	mpz_clear(bs[0]); 
	mpz_clear(results[0]); 
	free(as); 
	free(bs); 
	free(results); 
}

void SMC_Utils::smc_xor(mpz_t* a, mpz_t* b, int size, mpz_t* result, std::string type, int threadID){
	BOps->BitXor(a, b, result, size, threadID);
}

void SMC_Utils::smc_lor(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t* as = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t* bs = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t* results = (mpz_t*)malloc(sizeof(mpz_t)); 
	
	mpz_init_set(as[0], a); 
	mpz_init_set(bs[0], b); 
	mpz_init(results[0]); 
	BOps->BitOr(as, bs, results, 1, threadID);
	mpz_set(result, results[0]);  
	
	//free the memory
	mpz_clear(as[0]); 
	mpz_clear(bs[0]); 
	mpz_clear(results[0]); 

	free(as); 
	free(bs); 
	free(results); 
}

void SMC_Utils::smc_lor(mpz_t* a, mpz_t* b, int size, mpz_t* result, std::string type, int threadID){
	BOps->BitOr(a, b, result, size, threadID);
}


void SMC_Utils::smc_shr(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t* a_tmp = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t* b_tmp = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t* result_tmp = (mpz_t*)malloc(sizeof(mpz_t)); 

	mpz_init_set(a_tmp[0], a); 
	mpz_init_set(b_tmp[0], b); 
	mpz_init(result_tmp[0]); 

	smc_shr(a_tmp, b_tmp, alen, blen, result_tmp, resultlen, 1, type, threadID); 
	mpz_set(result, result_tmp[0]); 
	
	smc_batch_free_operator(&a_tmp, 1); 
	smc_batch_free_operator(&b_tmp, 1); 
	smc_batch_free_operator(&result_tmp, 1); 
	
}

void SMC_Utils::smc_shr(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t* a_tmp = (mpz_t*)malloc(sizeof(mpz_t));
        mpz_t* result_tmp = (mpz_t*)malloc(sizeof(mpz_t));
	int* b_tmp = (int*)malloc(sizeof(int)); 
        mpz_init_set(a_tmp[0], a);
        mpz_init(result_tmp[0]);
	b_tmp[0] = b; 

        smc_shr(a_tmp, b_tmp, alen, blen, result_tmp, resultlen, 1, type, threadID); 
	mpz_set(result, result_tmp[0]);

        free(b_tmp); 
	smc_batch_free_operator(&a_tmp, 1);
        smc_batch_free_operator(&result_tmp, 1);
}

void SMC_Utils::smc_shr(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID){
        if(blen == -1){
		int* b_tmp = (int*)malloc(sizeof(int) * size); 
		for(int i = 0; i < size; i++)
			b_tmp[i] = mpz_get_si(b[i]); 
		smc_shr(a, b_tmp, alen, blen, result, resultlen, size, type, threadID); 
		free(b_tmp); 
	}else
		Ts->doOperation(result, a, alen, b, size, threadID);
}

void SMC_Utils::smc_shr(mpz_t* a, int* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID){
	int same = 1; 
	for(int i = 1; i < size; i++)
		if(b[i] != b[0]) 
			same = 0; 
	if(same)	
		T->doOperation(result, a, alen, b[0], size, threadID);
	else{
		//for now, we perform sequential executions 
		for(int i = 0; i < size; i++)
			smc_shr(a[i], b[i], result[i], alen, blen, resultlen, type, threadID); 	
	}
}

void SMC_Utils::smc_shl(mpz_t a, mpz_t b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t* a_tmp = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t* b_tmp = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t* result_tmp = (mpz_t*)malloc(sizeof(mpz_t)); 

	mpz_init_set(a_tmp[0], a); 
	mpz_init_set(b_tmp[0], b); 
	mpz_init(result_tmp[0]);

	smc_shl(a_tmp, b_tmp, alen, blen, result_tmp, resultlen, 1, type, threadID); 
	mpz_set(result, result_tmp[0]);  

	smc_batch_free_operator(&a_tmp, 1); 
	smc_batch_free_operator(&b_tmp, 1); 
	smc_batch_free_operator(&result_tmp, 1); 		
}

void SMC_Utils::smc_shl(mpz_t a, int b, mpz_t result, int alen, int blen, int resultlen, std::string type, int threadID){
	mpz_t const2; 
	mpz_init_set_ui(const2, 2); 
	ss->modPow(result, const2, b);
	ss->modMul(result, a, result); 
	mpz_clear(const2); 
}

void SMC_Utils::smc_shl(mpz_t* a, mpz_t* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID){
	if(blen == -1){
		int* b_tmp = (int*)malloc(sizeof(int)*size); 
		for(int i = 0; i < size; i++)
                        b_tmp[i] = mpz_get_si(b[i]);
                smc_shl(a, b_tmp, alen, blen, result, resultlen, size, type, threadID);
                free(b_tmp);

	}else{
		P->doOperation(result, b, blen, size, threadID);
        	Mul->doOperation(result, result, a, size, threadID);
	}
}

void SMC_Utils::smc_shl(mpz_t* a, int* b, int alen, int blen, mpz_t* result, int resultlen, int size, std::string type, int threadID){
        mpz_t const2;
        mpz_init_set_ui(const2, 2);
	for(int i = 0; i < size; i++){
		ss->modPow(result[i], const2, b[i]);
        	ss->modMul(result[i], a[i], result[i]);
	}
        mpz_clear(const2);
}


// Dot Product
void SMC_Utils::smc_dot(mpz_t* a, mpz_t* b, int size, mpz_t result, int threadID){
	DProd->doOperation(a, b, result, size, threadID);
}

void SMC_Utils::smc_dot(mpz_t** a, mpz_t** b, int size, int array_size, mpz_t* result, std::string type, int threadID){
	DProd->doOperation(a, b, result, size, array_size, threadID);
}


//one-dimension private integer singular read
void SMC_Utils::smc_privindex_read(mpz_t index, mpz_t* array, mpz_t result, int dim, std::string type, int threadID)
{
	mpz_t* index_tmp = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_t* result_tmp = (mpz_t*)malloc(sizeof(mpz_t)); 
	mpz_init_set(index_tmp[0], index);
	mpz_init(result_tmp[0]);
	
	PI->doOperationRead(index_tmp, array, result_tmp, dim, 1, threadID, 0);
	mpz_set(result, result_tmp[0]);

	smc_batch_free_operator(&index_tmp, 1); 
	smc_batch_free_operator(&result_tmp, 1);  
}

//two-dimension private integer singular read
void SMC_Utils::smc_privindex_read(mpz_t index, mpz_t** array, mpz_t result, int dim1, int dim2, std::string type, int threadID)
{
    
	mpz_t* index_tmp = (mpz_t*)malloc(sizeof(mpz_t));
	mpz_t* array_tmp = (mpz_t*)malloc(sizeof(mpz_t) * dim1 * dim2);
	mpz_t* result_tmp = (mpz_t*)malloc(sizeof(mpz_t));
	
	mpz_init_set(index_tmp[0], index);
	mpz_init(result_tmp[0]);
	for(int i = 0; i < dim1; i++)
        	for(int j = 0; j < dim2; j++)
			mpz_init_set(array_tmp[i*dim2+j], array[i][j]);
    
	PI->doOperationRead(index_tmp, array_tmp, result_tmp, dim1 * dim2, 1, threadID, 0);
	mpz_set(result, result_tmp[0]);
    	
	smc_batch_free_operator(&index_tmp, 1); 
	smc_batch_free_operator(&result_tmp, 1); 
	smc_batch_free_operator(&array_tmp, dim1*dim2); 
}

//one-dimension private float singular read
void SMC_Utils::smc_privindex_read(mpz_t index, mpz_t** array, mpz_t* result, int dim, std::string type, int threadID)
{
    	mpz_t* index_tmp = (mpz_t*)malloc(sizeof(mpz_t));
	mpz_t* array_tmp = (mpz_t*)malloc(sizeof(mpz_t) * 4 * dim);
	mpz_init_set(index_tmp[0], index);
	for(int i = 0; i < dim; i++)
        	for(int j = 0; j < 4; j++)
            		mpz_init_set(array_tmp[4*i+j], array[i][j]);
    
	PI->doOperationRead(index_tmp, array_tmp, result, dim, 1, threadID, 1);

 	smc_batch_free_operator(&index_tmp, 1);
        smc_batch_free_operator(&array_tmp, 4*dim);
}

//two-dimension private float singular read
void SMC_Utils::smc_privindex_read(mpz_t index, mpz_t*** array, mpz_t* result, int dim1, int dim2, std::string type, int threadID)
{
    	mpz_t* index_tmp = (mpz_t*)malloc(sizeof(mpz_t));
	mpz_t* array_tmp = (mpz_t*)malloc(sizeof(mpz_t) * 4 * dim1 * dim2);
	mpz_init_set(index_tmp[0], index);
	for(int i = 0; i < dim1; i++)
		for(int j = 0; j < dim2; j++)
            		for(int k = 0; k < 4; k++)
                		mpz_init_set(array_tmp[4*(i*dim2+j)+k], array[i][j][k]);
	
	PI->doOperationRead(index_tmp, array_tmp, result, dim1*dim2, 1, threadID, 1);
    
	smc_batch_free_operator(&index_tmp, 1);
        smc_batch_free_operator(&array_tmp, 4*dim1*dim2);
}

//one-dimension private integer singular write
void SMC_Utils::smc_privindex_write(mpz_t index, mpz_t* array, int len_sig, int len_exp, int value, int dim, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID)
{
	mpz_t val; 
	mpz_init_set_si(val, value); 
	smc_privindex_write(index, array, len_sig, len_exp, val, dim, out_cond, priv_cond, counter, type, threadID); 
	mpz_clear(val); 
}

void SMC_Utils::smc_privindex_write(mpz_t index, mpz_t* array, int len_sig, int len_exp, mpz_t value, int dim, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID)
{
    	mpz_t* index_tmp = (mpz_t*)malloc(sizeof(mpz_t));
	mpz_t* value_tmp = (mpz_t*)malloc(sizeof(mpz_t));
	mpz_init_set(index_tmp[0], index);
	mpz_init_set(value_tmp[0], value);
    
	PI->doOperationWrite(index_tmp, array, value_tmp, dim, 1, out_cond, priv_cond, counter, threadID, 0);

 	smc_batch_free_operator(&index_tmp, 1);
        smc_batch_free_operator(&value_tmp, 1);
}

//two-dimension private integer singular write
void SMC_Utils::smc_privindex_write(mpz_t index, mpz_t** array, int len_sig, int len_exp, int value, int dim1, int dim2, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID)
{
	mpz_t val; 
	mpz_init_set_si(val, value); 
	smc_privindex_write(index, array, len_sig, len_exp, val, dim1, dim2, out_cond, priv_cond, counter, type, threadID); 
	mpz_clear(val); 
}

void SMC_Utils::smc_privindex_write(mpz_t index, mpz_t** array, int len_sig, int len_exp, mpz_t value, int dim1, int dim2, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID)
{
    	mpz_t* index_tmp = (mpz_t*)malloc(sizeof(mpz_t));
	mpz_t* array_tmp = (mpz_t*)malloc(sizeof(mpz_t) * dim1 * dim2);
	mpz_t* value_tmp = (mpz_t*)malloc(sizeof(mpz_t));
	mpz_init_set(index_tmp[0], index);
    	mpz_init_set(value_tmp[0], value);
	
	for(int i = 0; i < dim1; i++)
        	for(int j = 0; j < dim2; j++)
            		mpz_init_set(array_tmp[i*dim2+j], array[i][j]);
    
	PI->doOperationWrite(index_tmp, array_tmp, value_tmp, dim1*dim2, 1, out_cond, priv_cond, counter, threadID, 0);
    
	for(int i = 0; i < dim1; i++)
        	for(int j = 0; j < dim2; j++)
            		mpz_set(array[i][j], array_tmp[i*dim2+j]);

	smc_batch_free_operator(&index_tmp, 1);
        smc_batch_free_operator(&value_tmp, 1);
        smc_batch_free_operator(&array_tmp, dim1*dim2);
	
}

//one-dimension private float singular write
void SMC_Utils::smc_privindex_write(mpz_t index, mpz_t** array, int len_sig, int len_exp, float value, int dim, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID)
{
	mpz_t* val = (mpz_t*)malloc(sizeof(mpz_t) * 4);
        for(int i = 0; i < 4; i++)
                mpz_init(val[i]);
        convertFloat(value, len_sig, len_exp, &val);
	smc_privindex_write(index, array, len_sig, len_exp, val, dim, out_cond, priv_cond, counter, type, threadID); 
	smc_batch_free_operator(&val, 4); 	
}

void SMC_Utils::smc_privindex_write(mpz_t index, mpz_t** array, int len_sig, int len_exp, mpz_t* value, int dim, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID)
{
    	mpz_t* index_tmp = (mpz_t*)malloc(sizeof(mpz_t));
	mpz_t* array_tmp = (mpz_t*)malloc(sizeof(mpz_t) * 4 * dim); 
	
	mpz_init_set(index_tmp[0], index);
	for(int i = 0; i < dim; i++)
		for(int j = 0; j < 4; j++)
			mpz_init_set(array_tmp[4*i+j], array[i][j]); 
    
	PI->doOperationWrite(index_tmp, array_tmp, value, dim, 1, out_cond, priv_cond, counter, threadID, 1);
	
	for(int i = 0; i < dim; i++)
		for(int j = 0; j < 4; j++)
			mpz_set(array[i][j], array_tmp[4*i+j]); 

	smc_batch_free_operator(&index_tmp, 1);
        smc_batch_free_operator(&array_tmp, 4*dim);    	
}

//two-dimension private float singular write
void SMC_Utils::smc_privindex_write(mpz_t index, mpz_t*** array, int len_sig, int len_exp, float value, int dim1, int dim2, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID)
{
	mpz_t* val = (mpz_t*)malloc(sizeof(mpz_t) * 4);
        for(int i = 0; i < 4; i++)
                mpz_init(val[i]);
        convertFloat(value, len_sig, len_exp, &val);
	smc_privindex_write(index, array, len_sig, len_exp, val, dim1, dim2, out_cond, priv_cond, counter, type, threadID);
        smc_batch_free_operator(&val, 4);

}

void SMC_Utils::smc_privindex_write(mpz_t index, mpz_t*** array, int len_sig, int len_exp, mpz_t* value, int dim1, int dim2, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID)
{
    mpz_t* index_tmp = (mpz_t*)malloc(sizeof(mpz_t));
    mpz_t* array_tmp = (mpz_t*)malloc(sizeof(mpz_t) * 4 * dim1 * dim2);
    
    mpz_init_set(index_tmp[0], index);
    for(int i = 0; i < dim1; i++)
	for(int j = 0; j < dim2; j++)
            for(int k = 0; k < 4; k++)
                mpz_init_set(array_tmp[4*(i*dim2+j)+k], array[i][j][k]);
    
    PI->doOperationWrite(index_tmp, array_tmp, value, dim1*dim2, 1, out_cond, priv_cond, counter, threadID, 1);

    for(int i = 0; i < dim1; i++)
        for(int j = 0; j < dim2; j++)
            for(int k = 0; k < 4; k++)
                mpz_set(array[i][j][k], array_tmp[4*(i*dim2+j)+k]);
    
    smc_batch_free_operator(&index_tmp, 1);
    smc_batch_free_operator(&array_tmp, 4*dim1*dim2);
}

//one-dimension private integer batch read
void SMC_Utils::smc_privindex_read(mpz_t* indices, mpz_t* arrays, mpz_t* results, int dim, int size, std::string type, int threadID)
{
    PI->doOperationRead(indices, arrays, results, dim, size, threadID, 0);
}

//two-dimension private integer batch read
void SMC_Utils::smc_privindex_read(mpz_t* indices, mpz_t** array, mpz_t* results, int dim1, int dim2, int size, std::string type, int threadID)
{
	mpz_t* array_tmp = (mpz_t*)malloc(sizeof(mpz_t) * dim1 * dim2);
	for(int i = 0; i < dim1; i++)
        	for(int j = 0; j < dim2; j++)
            		mpz_init_set(array_tmp[i*dim2+j], array[i][j]);
	PI->doOperationRead(indices, array_tmp, results, dim1*dim2, size, threadID, 0);
	smc_batch_free_operator(&array_tmp, dim1*dim2);     
}

//one-dimension private float batch read
void SMC_Utils::smc_privindex_read(mpz_t* indices, mpz_t** array, mpz_t** results, int dim, int size, std::string type, int threadID)
{
    	mpz_t* array_tmp = (mpz_t*)malloc(sizeof(mpz_t) * 4 * dim);
	mpz_t* result_tmp = (mpz_t*)malloc(sizeof(mpz_t) * 4 * size); 
	
        for(int i = 0; i < dim; i++)
                for(int j = 0; j < 4; j++)
                        mpz_init_set(array_tmp[4*i+j], array[i][j]);
	for(int i = 0; i < 4 * size; i++)
		mpz_init(result_tmp[i]); 

	PI->doOperationRead(indices, array_tmp, result_tmp, dim, size, threadID, 1);
	
	for(int i = 0; i < size; i++)
		for(int j = 0; j < 4; j++)
			mpz_set(results[i][j], result_tmp[4*i+j]); 

	smc_batch_free_operator(&result_tmp, 4*size); 
	smc_batch_free_operator(&array_tmp, 4*dim); 
}

//two-dimension private float batch read
void SMC_Utils::smc_privindex_read(mpz_t* indices, mpz_t*** array, mpz_t** results, int dim1, int dim2, int size, std::string type, int threadID)
{
    mpz_t* array_tmp = (mpz_t*)malloc(sizeof(mpz_t) * 4 * dim1 * dim2);
    mpz_t* result_tmp = (mpz_t*)malloc(sizeof(mpz_t) * 4 * size);

    for(int i = 0; i < dim1; i++)
        for(int j = 0; j < dim2; j++)
            for(int k = 0; k < 4; k++)
                mpz_init_set(array_tmp[4*(i*dim2+j)+k], array[i][j][k]);
    for(int i = 0; i < 4 * size; i++)
        mpz_init(result_tmp[i]);
 
    PI->doOperationRead(indices, array_tmp, result_tmp, dim1*dim2, size, threadID, 1);
    
    for(int i = 0; i < size; i++)
        for(int j = 0; j < 4; j++)
             mpz_set(results[i][j], result_tmp[4*i+j]);
    
    smc_batch_free_operator(&result_tmp, 4*size);  
    smc_batch_free_operator(&array_tmp, 4*dim1*dim2);  
}

//one-dimension private integer batch write
void SMC_Utils::smc_privindex_write(mpz_t* indices, mpz_t* array, int len_sig, int len_exp, int* values, int dim, int size, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID)
{
	mpz_t* val = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	for(int i = 0; i < size; i++)
		mpz_init_set_si(val[i], values[i]); 
	smc_privindex_write(indices, array, len_sig, len_exp, val, dim, size, out_cond, priv_cond, counter, type, threadID); 
	smc_batch_free_operator(&val, size); 
}

void SMC_Utils::smc_privindex_write(mpz_t* indices, mpz_t* array, int len_sig, int len_exp, mpz_t* values, int dim, int size, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID)
{
    PI->doOperationWrite(indices, array, values, dim, size, out_cond, priv_cond, counter, threadID, 0);
}

void SMC_Utils::smc_privindex_write(mpz_t* indices, mpz_t** array, int len_sig, int len_exp, int* values, int dim1, int dim2, int size, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID){
   	mpz_t* val = (mpz_t*)malloc(sizeof(mpz_t) * size);
        for(int i = 0; i < size; i++)
                mpz_init_set_si(val[i], values[i]);
        smc_privindex_write(indices, array, len_sig, len_exp, val, dim1, dim2, size, out_cond, priv_cond, counter, type, threadID);
        smc_batch_free_operator(&val, size);
}

//two-dimension private integer batch write
void SMC_Utils::smc_privindex_write(mpz_t* indices, mpz_t** array, int len_sig, int len_exp, mpz_t* values, int dim1, int dim2, int size, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID)
{
    mpz_t* array_tmp = (mpz_t*)malloc(sizeof(mpz_t) * dim1 * dim2);
    for(int i = 0; i < dim1; i++)
        for(int j = 0; j < dim2; j++)
            mpz_init_set(array_tmp[i*dim2+j], array[i][j]);
    
    PI->doOperationWrite(indices, array_tmp, values, dim1*dim2, size, out_cond, priv_cond, counter, threadID, 0);
    
    for(int i = 0; i < dim1; i++)
        for(int j = 0; j < dim2; j++)
            mpz_set(array[i][j], array_tmp[i*dim2+j]);
   
    smc_batch_free_operator(&array_tmp, dim1*dim2);     
}

//one-dimension private float batch write
void SMC_Utils::smc_privindex_write(mpz_t* indices, mpz_t** array, int len_sig, int len_exp, float* values, int dim, int size, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID)
{
	mpz_t** val = (mpz_t**)malloc(sizeof(mpz_t*) * size);
	for(int i = 0; i < size; i++){
		val[i] = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
		for(int j = 0; j < 4; j++)
			mpz_init(val[i][j]); 
        	convertFloat(values[i], len_sig, len_exp, &(val[i]));
	}
	smc_privindex_write(indices, array, len_sig, len_exp, val, dim, size, out_cond, priv_cond, counter, type, threadID); 
	smc_batch_free_operator(&val, size); 

}
void SMC_Utils::smc_privindex_write(mpz_t* indices, mpz_t** array, int len_sig, int len_exp, mpz_t** values, int dim, int size, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID)
{
    	mpz_t* array_tmp = (mpz_t*)malloc(sizeof(mpz_t) * 4 * dim);
    	mpz_t* value_tmp = (mpz_t*)malloc(sizeof(mpz_t) * 4 * size); 
	
	for(int i = 0; i < dim; i++)
        	for(int j = 0; j < 4; j++)
            		mpz_init_set(array_tmp[i*4+j], array[i][j]);	

	for(int i = 0; i < size; i++)
		for(int j = 0; j < 4; j++)
			mpz_init_set(value_tmp[i*4+j], values[i][j]); 

	PI->doOperationWrite(indices, array_tmp, value_tmp, dim, size, out_cond, priv_cond, counter, threadID, 1);
	
	for(int i = 0; i < dim; i++)
		for(int j = 0; j < 4; j++)
			mpz_set(array[i][j], array_tmp[4*i+j]); 
	
	smc_batch_free_operator(&array_tmp, 4*dim); 
	smc_batch_free_operator(&value_tmp, 4*size); 
}

//two-dimension private float batch write
void SMC_Utils::smc_privindex_write(mpz_t* indices, mpz_t*** array, int len_sig, int len_exp, float* values, int dim1, int dim2, int size, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID)
{
	mpz_t** val = (mpz_t**)malloc(sizeof(mpz_t*) * size);
        for(int i = 0; i < size; i++){
                val[i] = (mpz_t*)malloc(sizeof(mpz_t) * 4);
                for(int j = 0; j < 4; j++)
                        mpz_init(val[i][j]);
                convertFloat(values[i], len_sig, len_exp, &(val[i]));
        }
        smc_privindex_write(indices, array, len_sig, len_exp, val, dim1, dim2, size, out_cond, priv_cond, counter, type, threadID);
        smc_batch_free_operator(&val, size);

}
void SMC_Utils::smc_privindex_write(mpz_t* indices, mpz_t*** array, int len_sig, int len_exp, mpz_t** values, int dim1, int dim2, int size, mpz_t out_cond, mpz_t* priv_cond, int counter, std::string type, int threadID)
{
    mpz_t* array_tmp = (mpz_t*)malloc(sizeof(mpz_t) * 4 * dim1 * dim2);
    mpz_t* value_tmp = (mpz_t*)malloc(sizeof(mpz_t) * 4 * size); 

    for(int i = 0; i < dim1; i++)
	for(int j = 0; j < dim2; j++)
            for(int k = 0; k < 4; k++)
                mpz_init_set(array_tmp[4*(i*dim2+j)+k], array[i][j][k]);
	
    for(int i = 0; i < size; i++)
	for(int j = 0; j < 4; j++)
		mpz_init_set(value_tmp[4*i+j], values[i][j]); 
    
    PI->doOperationWrite(indices, array_tmp, value_tmp, dim1*dim2, size, out_cond, priv_cond, counter, threadID, 1);
    
    for(int i = 0; i < dim1; i++)
        for(int j = 0; j < dim2; j++)
            for(int k = 0; k < 4; k++)
                mpz_set(array[i][j][k], array_tmp[4*(i*dim2+j)+k]);

    smc_batch_free_operator(&array_tmp, 4*dim1*dim2); 
    smc_batch_free_operator(&value_tmp, 4*size); 
}

priv_ptr SMC_Utils::smc_new_ptr(int level, int type)
{
	return PP->create_ptr(level, type); 
}

priv_ptr* SMC_Utils::smc_new_ptr(int level, int type, int num)
{
	return PP->create_ptr(level, type, num); 
}

/*************/
void SMC_Utils::smc_set_int_ptr(priv_ptr ptr, mpz_t* var_loc, std::string type, int threadID)
{
	PP->set_ptr(ptr, var_loc, NULL, NULL, NULL, threadID); 
}

void SMC_Utils::smc_set_float_ptr(priv_ptr ptr, mpz_t** var_loc, std::string type, int threadID)
{
	PP->set_ptr(ptr, NULL, var_loc, NULL, NULL, threadID); 
}

void SMC_Utils::smc_set_struct_ptr(priv_ptr ptr, void* var_loc, std::string type, int threadID)
{
	PP->set_ptr(ptr, NULL, NULL, var_loc, NULL, threadID); 
}

void SMC_Utils::smc_set_ptr(priv_ptr assign_ptr, priv_ptr* ptr_loc, std::string type, int threadID)
{
	PP->set_ptr(assign_ptr, NULL, NULL, NULL, ptr_loc, threadID); 
}

void SMC_Utils::smc_set_ptr(priv_ptr assign_ptr, priv_ptr right_ptr, std::string type, int threadID)
{
	PP->set_ptr(assign_ptr, right_ptr, threadID); 
}

void SMC_Utils::smc_set_ptr(priv_ptr ptr, int var_loc, std::string type, int threadID)
{

}
/****************/
void SMC_Utils::smc_update_int_ptr(priv_ptr ptr, mpz_t* var_loc, mpz_t private_tag, int index, int threadID) 
{
	PP->update_ptr(ptr, var_loc, NULL, NULL, NULL, private_tag, index, threadID); 
}

void SMC_Utils::smc_update_float_ptr(priv_ptr ptr, mpz_t** var_loc, mpz_t private_tag, int index, int threadID) 
{
	PP->update_ptr(ptr, NULL, var_loc, NULL, NULL, private_tag, index, threadID); 
}

void SMC_Utils::smc_update_struct_ptr(priv_ptr ptr, void* var_loc, mpz_t private_tag, int index, int threadID) 
{
	PP->update_ptr(ptr, NULL, NULL, var_loc, NULL, private_tag, index, threadID); 
}

void SMC_Utils::smc_update_ptr(priv_ptr ptr, priv_ptr* ptr_loc, mpz_t private_tag, int index, int threadID) 
{
	PP->update_ptr(ptr, NULL, NULL, NULL, ptr_loc, private_tag, index, threadID); 
}

void SMC_Utils::smc_update_ptr(priv_ptr assign_ptr, priv_ptr right_ptr, mpz_t private_tag, int index, int threadID)
{
	PP->update_ptr(assign_ptr, right_ptr, private_tag, index, threadID); 
}

void SMC_Utils::smc_add_int_ptr(priv_ptr ptr, mpz_t* var_loc, mpz_t private_tag, int threadID) 
{
	PP->add_ptr(ptr, var_loc, NULL, NULL, NULL, private_tag, threadID); 
}

void SMC_Utils::smc_add_float_ptr(priv_ptr ptr, mpz_t** var_loc, mpz_t private_tag, int threadID) 
{
	PP->add_ptr(ptr, NULL, var_loc, NULL, NULL, private_tag, threadID); 
}

void SMC_Utils::smc_add_struct_ptr(priv_ptr ptr, void* var_loc, mpz_t private_tag, int threadID) 
{
	PP->add_ptr(ptr, NULL, NULL, var_loc, NULL, private_tag, threadID); 
}

void SMC_Utils::smc_add_ptr(priv_ptr ptr, priv_ptr* ptr_loc, mpz_t private_tag, int threadID) 
{
	PP->add_ptr(ptr, NULL, NULL, NULL, ptr_loc, private_tag, threadID); 
}

/*******************/
void SMC_Utils::smc_shrink_ptr(priv_ptr ptr, int current_index, int parent_index, int threadID)
{
	PP->shrink_ptr(ptr, current_index, parent_index, threadID); 
}
/********************/
void SMC_Utils::smc_dereference_read_ptr(priv_ptr ptr, mpz_t result, int num_of_dereferences, mpz_t priv_cond, std::string type, int threadID)
{
	PP->dereference_ptr_read_var(ptr, result, num_of_dereferences, threadID); 
}

void SMC_Utils::smc_dereference_read_ptr(priv_ptr ptr, mpz_t* result, int num_of_dereferences, mpz_t priv_cond, std::string type, int threadID)
{
	PP->dereference_ptr_read_var(ptr, result, num_of_dereferences, threadID); 
}

void SMC_Utils::smc_dereference_read_ptr(priv_ptr ptr, priv_ptr result, int num_of_dereferences, mpz_t priv_cond, std::string type, int threadID)
{
	PP->dereference_ptr_read_ptr(ptr, result, num_of_dereferences, priv_cond, threadID); 
}

void SMC_Utils::smc_dereference_write_ptr(priv_ptr ptr, mpz_t value, int num_of_dereferences, mpz_t private_cond, std::string type, int threadID)
{
	PP->dereference_ptr_write_var(ptr, value, num_of_dereferences, private_cond, threadID); 
}

void SMC_Utils::smc_dereference_write_int_ptr(priv_ptr ptr, mpz_t* value, int num_of_dereferences, mpz_t private_cond, std::string type, int threadID)
{
	PP->dereference_ptr_write(ptr, value, NULL, NULL, NULL, num_of_dereferences, private_cond, threadID); 
}

void SMC_Utils::smc_dereference_write_ptr(priv_ptr ptr, mpz_t* value, int num_of_dereferences, mpz_t private_cond, std::string type, int threadID)
{
	PP->dereference_ptr_write_var(ptr, value, num_of_dereferences, private_cond, threadID); 
}

void SMC_Utils::smc_dereference_write_float_ptr(priv_ptr ptr, mpz_t** value, int num_of_dereferences, mpz_t private_cond, std::string type, int threadID)
{
	PP->dereference_ptr_write(ptr, NULL, value, NULL, NULL, num_of_dereferences, private_cond, threadID); 
}

void SMC_Utils::smc_dereference_write_struct_ptr(priv_ptr ptr, void* value, int num_of_dereferences, mpz_t private_cond, std::string type, int threadID)
{
	PP->dereference_ptr_write(ptr, NULL, NULL, value, NULL, num_of_dereferences, private_cond, threadID); 
}

void SMC_Utils::smc_dereference_write_ptr(priv_ptr ptr, priv_ptr* value, int num_of_dereferences, mpz_t private_cond, std::string type, int threadID)
{
	PP->dereference_ptr_write(ptr, NULL, NULL, NULL, value, num_of_dereferences, private_cond, threadID); 
}

void SMC_Utils::smc_dereference_write_ptr(priv_ptr ptr, priv_ptr value, int num_of_dereferences, mpz_t private_cond, std::string type, int threadID)
{
	PP->dereference_ptr_write_ptr(ptr, value, num_of_dereferences, private_cond, threadID); 
}
/*******************************************/
void SMC_Utils::smc_clear_ptr(priv_ptr* ptr)
{
	PP->clear_ptr(ptr); 
}
void SMC_Utils::smc_free_ptr(priv_ptr* ptr)
{
	PP->destroy_ptr(ptr); 
}

void SMC_Utils::smc_free_ptr(priv_ptr** ptrs, int num)
{
	PP->destroy_ptr(ptrs, num); 
}

void SMC_Utils::smc_int2fl(int value, mpz_t* result, int gamma, int K, int L, int threadID){
	mpz_t val; 
	mpz_init_set_si(val, value); 
	smc_int2fl(val, result, 32, K, L, threadID); 
	mpz_clear(val); 
}

void SMC_Utils::smc_int2fl(mpz_t value, mpz_t* result, int gamma, int K, int L, int threadID)
{
	mpz_t** results = (mpz_t**)malloc(sizeof(mpz_t*)); 
	mpz_t* values = (mpz_t*)malloc(sizeof(mpz_t));
	results[0] = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
	
	for(int i = 0; i < 4; i++)
		mpz_init(results[0][i]); 
	mpz_init_set(values[0], value); 
	
	I2F->doOperation(values, results, gamma, K, 1, threadID); 	
	
	for(int i = 0; i < 4; i++)
		mpz_set(result[i], results[0][i]); 
	
	smc_batch_free_operator(&values, 1); 
	smc_batch_free_operator(&results, 1); 
}

void SMC_Utils::smc_int2int(int value, mpz_t result, int gamma1, int gamma2, int threadID){
	mpz_set_si(result, value); 
}

void SMC_Utils::smc_int2int(mpz_t value, mpz_t result, int gamma1, int gamma2, int threadID){
	mpz_set(result, value); 
}

void SMC_Utils::smc_fl2int(float value, mpz_t result, int K, int L, int gamma, int threadID){
        mpz_t* val = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
	for(int i = 0; i < 4; i++)
		mpz_init(val[i]); 
	convertFloat(value, 32, 9, &val);
	smc_fl2int(val, result, 32, 9, gamma, threadID); 
	for(int i = 0; i < 4; i++)
		mpz_clear(val[i]); 
	free(val); 
}

void SMC_Utils::smc_fl2int(mpz_t* value, mpz_t result, int K, int L, int gamma, int threadID){
 	mpz_t** values = (mpz_t**)malloc(sizeof(mpz_t*));
        mpz_t* results = (mpz_t*)malloc(sizeof(mpz_t));
        values[0] = (mpz_t*)malloc(sizeof(mpz_t) * 4);

        for(int i = 0; i < 4; i++)
        	mpz_init_set(values[0][i], value[i]);
	mpz_init(results[0]); 
        F2I->doOperation(values, results, K, L, gamma, 1, threadID);
        mpz_set(result, results[0]);
        smc_batch_free_operator(&values, 1);
        smc_batch_free_operator(&results, 1);

}

void SMC_Utils::smc_fl2fl(float value, mpz_t* result, int K1, int L1, int K2, int L2, int threadID){	
	mpz_t* val = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
	for(int i = 0; i < 4; i++)
		mpz_init(val[i]); 
	convertFloat(value, 32, 9, &val); 
	smc_fl2fl(val, result, 32, 9, K2, L2, threadID); 
	for(int i = 0; i < 4; i++)
		mpz_clear(val[i]); 
	free(val); 
}

void SMC_Utils::smc_fl2fl(mpz_t* value, mpz_t* result, int K1, int L1, int K2, int L2, int threadID){	
	if(K1 >= K2)	
		T->doOperation(result, value, K1, K1-K2, 1, threadID); 
	else{
		mpz_t tmp, two; 
		mpz_init_set_ui(two, 2); 
		mpz_init(tmp); 
		ss->modPow(tmp, two, K2-K1); 
		ss->modMul(result[0], value[0], tmp); 
		mpz_clear(tmp); 
		mpz_clear(two);  
	}
	ss->modAdd(result[1], value[1], K1-K2); 
	mpz_set(result[2], value[2]); 
        mpz_set(result[3], value[3]); 			
}


void SMC_Utils::smc_batch_handle_priv_cond(mpz_t* result, mpz_t* result_org, mpz_t out_cond, mpz_t *priv_cond, int counter, int size, int threadID)
{
        mpz_t* tmp = (mpz_t*)malloc(sizeof(mpz_t) * size);
        for(int i=0; i<size; ++i)
                        mpz_init(tmp[i]);
	if(out_cond == NULL && counter == -1 && priv_cond == NULL)
	{
	}
	else if(out_cond != NULL && counter == -1 && priv_cond == NULL)
	{
		for(int i = 0; i < size; i++)
			mpz_set(tmp[i], out_cond);
		ss->modSub(result, result, result_org, size); 
		Mul->doOperation(result, result, tmp, size, threadID); 
		ss->modAdd(result, result, result_org, size); 
		/*
		ss->modSub(tmp3, 1, tmp, size); 
                Mul->doOperation(tmp1, result, tmp, size);
                Mul->doOperation(tmp2, result_org, tmp3, size);
                ss->modAdd(result, tmp1, tmp2, size);
		*/
	}
	else if(out_cond == NULL && counter != -1 && priv_cond != NULL)
	{
		for(int i = 0; i < size; i++)
			if(counter != size)
				mpz_set(tmp[i], priv_cond[i/(size/counter)]); 
			else
				mpz_set(tmp[i], priv_cond[i]);
		ss->modSub(result, result, result_org, size); 
		Mul->doOperation(result, result, tmp, size, threadID); 
		ss->modAdd(result, result, result_org, size); 
		/* 
		ss->modSub(tmp3, 1, tmp, size);
                Mul->doOperation(tmp1, result, tmp, size);
                Mul->doOperation(tmp2, result_org, tmp3, size);
                ss->modAdd(result, tmp1, tmp2, size);
		*/
	}
       for(int i=0; i<size; ++i)
              mpz_clear(tmp[i]);
       free(tmp); 
}

void SMC_Utils::smc_batch_BOP_int(mpz_t* result, mpz_t* a, mpz_t* b, int resultlen, int alen, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int size, std::string op, std::string type, int threadID)
{	
	mpz_t* result_org = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	for(int i = 0; i < size; i++)
		mpz_init_set(result_org[i], result[i]); 
	if (op == "*"){
		smc_mult(a, b, alen, blen, result, resultlen, size, type, threadID);
	} else if (op == "-"){
		smc_sub(a, b, alen, blen, result, resultlen, size, type, threadID);
	} else if (op == "+"){
		smc_add(a, b, alen, blen, result, resultlen, size, type, threadID);
	} else if (op == "=="){
		smc_eqeq(a, b, alen, blen, result, resultlen, size, type, threadID);
	} else if (op == "!="){
		smc_neq(a, b, alen, blen, result, resultlen, size, type, threadID);
	} else if (op == ">"){
		smc_gt(a, b, alen, blen, result, resultlen, size, type, threadID); 
	} else if (op == ">="){
		smc_geq(a, b, alen, blen, result, resultlen, size, type, threadID);
	} else if (op == "<"){
		smc_lt(a, b, alen, blen, result, resultlen, size, type, threadID);
	} else if (op == "<="){
		smc_leq(a, b, alen, blen, result, resultlen, size, type, threadID);
	} else if (op == "/"){
		smc_div(a, b, alen, blen, result, resultlen, size, type, threadID);
	} else if (op == "/P"){
		Idiv->doOperationPub(result, a, b, resultlen, size, threadID); 
	} else if (op == "="){
		smc_set(a, result, alen, resultlen, size, type, threadID); 
	} else if (op == ">>"){
		smc_shr(a, b, alen, blen, result, resultlen, size, type, threadID); 	
	} else if(op == "<<"){
		smc_shl(a, b, alen, blen, result, resultlen, size, type, threadID); 
	}
	else{
		std::cout << "Unrecognized op: " << op << "\n";
	}
	
	smc_batch_handle_priv_cond(result, result_org, out_cond, priv_cond, counter, size, threadID); 
	smc_batch_free_operator(&result_org, size); 
}

void SMC_Utils::smc_batch_BOP_float_arithmetic(mpz_t** result, mpz_t** a, mpz_t** b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int size, std::string op, std::string type, int threadID)
{
	mpz_t** result_org = (mpz_t**)malloc(sizeof(mpz_t*) * size); 
	for(int i = 0; i < size; i++){
		result_org[i] = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
		for(int j = 0; j < 4; j++)
			mpz_init_set(result_org[i][j], result[i][j]); 
	}
	
	if (op == "*"){
		smc_mult(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
	} else if (op == "-"){
		smc_sub(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
	} else if (op == "+"){
		smc_add(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
	} else if (op == "/"){
		smc_div(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
	} else if (op == "/P"){
		Fldiv->doOperationPub(a, b, result, alen_sig, size, threadID); 
		smc_process_results(result, resultlen_sig, resultlen_exp, alen_sig, alen_exp, size, threadID); 	
	} else if (op == "="){
		smc_set(a, result, alen_sig, alen_exp, resultlen_sig, resultlen_exp, size, type, threadID); 
	} 

	mpz_t* result_unify = (mpz_t*)malloc(sizeof(mpz_t) * 4 * size); 
	mpz_t* result_org_unify = (mpz_t*)malloc(sizeof(mpz_t) * 4 * size);
	
	for(int i = 0; i < size; i++){
		for(int j = 0; j < 4; j++){
			mpz_init_set(result_unify[4*i+j], result[i][j]); 
			mpz_init_set(result_org_unify[4*i+j], result_org[i][j]); 		
		}
			
	}	
	
	smc_batch_handle_priv_cond(result_unify, result_org_unify, out_cond, priv_cond, counter, 4 * size, threadID); 
	
	for(int i = 0; i < size; i++)
		for(int j = 0; j < 4; j++)
			mpz_set(result[i][j], result_unify[4*i+j]); 			
	
	smc_batch_free_operator(&result_unify, 4 * size); 
	smc_batch_free_operator(&result_org_unify, 4 * size); 
}

void SMC_Utils::smc_batch_BOP_float_comparison(mpz_t* result, mpz_t** a, mpz_t** b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID)
{

	mpz_t* result_org = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* result_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 

	for(int i = 0; i < size; i++){	
        	mpz_init_set(result_org[i], result[index_array[3*i+2]]); 
		mpz_init(result_tmp[i]); 
	}
	
	if (op == "=="){
		smc_eqeq(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result_tmp, resultlen_sig, size, type, threadID);
	} else if (op == "!="){
		smc_neq(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result_tmp, resultlen_sig, size, type, threadID);
	} else if (op == ">"){
		smc_gt(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result_tmp, resultlen_sig, size, type, threadID); 
	} else if (op == ">="){
		smc_geq(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result_tmp, resultlen_sig, size, type, threadID);
	} else if (op == "<"){
		smc_lt(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result_tmp, resultlen_sig, size, type, threadID);
	} else if (op == "<="){
		smc_leq(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result_tmp, resultlen_sig, size, type, threadID);
	}
		
	smc_batch_handle_priv_cond(result_tmp, result_org, out_cond, priv_cond, counter, size, threadID); 

	for(int i = 0; i < size; ++i)
                  mpz_set(result[index_array[3*i+2]], result_tmp[i]);
	
	smc_batch_free_operator(&result_org, size); 	
	smc_batch_free_operator(&result_tmp, size); 	
}

void SMC_Utils::smc_convert_operator(mpz_t** result, mpz_t** op, int* index_array, int dim, int size, int flag){
      *result = (mpz_t*)malloc(sizeof(mpz_t) * size); 
      for(int i = 0; i < size; i++)
	mpz_init((*result)[i]);  
      int dim1, dim2; 
      for(int i = 0; i < size; i++){
		if(flag == 1){
                	if(dim != 0){
                        	dim1 = index_array[3*i]/dim;
                        	dim2 = index_array[3*i]%dim;
                        	mpz_set((*result)[i], op[dim1][dim2]);
                	}
                	else
                        	mpz_set((*result)[i], op[0][index_array[3*i]]);
		}
		else if(flag == 2){
                	if(dim != 0 && dim != -1){
                        	dim1 = index_array[3*i+1]/dim;
                        	dim2 = index_array[3*i+1]%dim;
                        	mpz_set((*result)[i], op[dim1][dim2]);
                	}
                	else if(dim == 0)
                        	mpz_set((*result)[i], op[0][index_array[3*i+1]]);
		}
		else{
                	if(dim != 0){
                        	dim1 = index_array[3*i+2]/dim;
                        	dim2 = index_array[3*i+2]%dim;
                        	mpz_set((*result)[i], op[dim1][dim2]);
                	}
		}
     }
}

//convert op to corresponding one-dimensional array result
void SMC_Utils::smc_convert_operator(mpz_t** result, mpz_t* op, int* index_array, int dim, int size, int flag)
{
	mpz_t** ops = (mpz_t**)malloc(sizeof(mpz_t*)); 
	*ops = op; 
	smc_convert_operator(result, ops, index_array, dim, size, flag); 
	free(ops); 
}

void SMC_Utils::smc_convert_operator(mpz_t*** result, mpz_t** op, int* index_array, int dim, int size, int flag){
	mpz_t*** ops = NULL; 
	if(op != NULL){
		ops = (mpz_t***)malloc(sizeof(mpz_t**)); 
		 *ops = op; 
	}
	smc_convert_operator(result, ops, index_array, dim, size, flag); 
	if(op != NULL)
		free(ops); 
}

void SMC_Utils::smc_convert_operator(mpz_t*** result, mpz_t*** op, int* index_array, int dim, int size, int flag)
{
	*result =(mpz_t**)malloc(sizeof(mpz_t*) * size);
        int dim1, dim2;
        for(int i = 0; i < size; i++){
                (*result)[i] = (mpz_t*)malloc(sizeof(mpz_t) * 4);
                for(int j = 0; j < 4; j++)
                        mpz_init((*result)[i][j]);
                for(int j = 0; j < 4; j++){
                        if(flag == 1){
				if(dim != 0){
                                	dim1 = index_array[3*i]/dim;
                                	dim2 = index_array[3*i]%dim;
                                	mpz_set((*result)[i][j], op[dim1][dim2][j]);
                        	}
                        	else
                                	mpz_set((*result)[i][j], op[0][i][j]);
			}
			else if(flag == 2){
                        	if(op != NULL && dim != -1){
                                	if(dim != 0){
                                        	dim1 = index_array[3*i+1]/dim;
                                        	dim2 = index_array[3*i+1]%dim;
                                        	mpz_set((*result)[i][j], op[dim1][dim2][j]);
                                	}
                                	else
                                        	mpz_set((*result)[i][j], op[0][i][j]);
				}
                        }
			else{
                        	if(dim != 0){
                                	dim1 = index_array[3*i+2]/dim;
                                	dim2 = index_array[3*i+2]%dim;
                                	mpz_set((*result)[i][j], op[dim1][dim2][j]);
                        	}
			}
                }
        }
}

void SMC_Utils::smc_batch_free_operator(mpz_t** op, int size)
{
	for(int i = 0; i < size; i++)
		mpz_clear((*op)[i]); 
	free(*op); 
}

void SMC_Utils::smc_batch_free_operator(mpz_t*** op, int size)
{
	for(int i = 0; i < size; i++)
	{
		for(int j = 0; j < 4; j++)
			mpz_clear((*op)[i][j]); 
		free((*op)[i]); 
	}
	free(*op); 
}

/************************************ INTEGER BATCH ****************************************/
void SMC_Utils::smc_batch(mpz_t* a, mpz_t* b, mpz_t* result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	mpz_t *a_tmp, *b_tmp, *result_tmp; 
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
	smc_convert_operator(&b_tmp, b, index_array, bdim, size, 2); 
	smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3); 
	
	smc_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID);
		
	for(int i = 0; i < size; ++i)
		mpz_set(result[index_array[3*i+2]], result_tmp[i]);
	
	smc_batch_free_operator(&a_tmp, size); 
	smc_batch_free_operator(&b_tmp, size); 
	smc_batch_free_operator(&result_tmp, size); 
}

// used to compute 1-priv_cond in a batch stmt
void SMC_Utils::smc_batch(int a, mpz_t* b, mpz_t* result, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, int threadID){
	mpz_t* a_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size);
	mpz_t* out_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size);
  
	for(int i = 0; i < size; i++){
		mpz_init_set_ui(a_tmp[i], a); 
		mpz_init(out_tmp[i]); 
	}

	ss->modSub(result, a_tmp, b, size); 	

	if(out_cond != NULL){
		for(int i = 0; i < size; i++)
			mpz_set(out_tmp[i], out_cond); 
		Mul->doOperation(result, result, out_tmp, size, threadID); 	
	}
	
	smc_batch_free_operator(&a_tmp, size); 
	smc_batch_free_operator(&out_tmp, size); 
}

void SMC_Utils::smc_batch(mpz_t* a, mpz_t* b, mpz_t* result, mpz_t out_cond, mpz_t* priv_cond, int counter, int *index_array, int size, std::string op, int threadID)
{
	if(counter == size)
		ss->modSub(result, a, b, size);  
	else{
		mpz_t* tmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
		for(int i = 0; i < size; i++)
			mpz_init_set(tmp[i], a[i/(size/counter)]);
		ss->modSub(result, tmp, b, size);  
		smc_batch_free_operator(&tmp, size); 
	}
}

//first param: int array 
//second param: int array
//third param: one-dim private int array 
void SMC_Utils::smc_batch(int* a, int* b, mpz_t* result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	
	mpz_t* a_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* b_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	for(int i = 0; i < size; i++){
		mpz_init_set_si(a_tmp[i], a[i]); 
		mpz_init_set_si(b_tmp[i], b[i]); 
	}

	smc_batch(a_tmp, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);  
	
	smc_batch_free_operator(&a_tmp, size); 
	smc_batch_free_operator(&b_tmp, size); 
}

//first param: int array 
//second param: int array
//third param: two-dim private int array 
void SMC_Utils::smc_batch(int* a, int* b, mpz_t** result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	
	mpz_t* a_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* b_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 

	for(int i = 0; i < size; i++){
		mpz_init_set_si(a_tmp[i], a[i]); 
		mpz_init_set_si(b_tmp[i], b[i]); 
	}

	smc_batch(a_tmp, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);  
	smc_batch_free_operator(&a_tmp, size); 
	smc_batch_free_operator(&b_tmp, size); 
}

//first param: int array 
//second param: one-dim private int array
//third param: one-dim private int array 
void SMC_Utils::smc_batch(int* a, mpz_t* b, mpz_t* result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	
	mpz_t* a_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	for(int i = 0; i < size; i++)
		mpz_init_set_si(a_tmp[i], a[i]); 
	smc_batch(a_tmp, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);  
	smc_batch_free_operator(&a_tmp, size); 
}

//first param: int array 
//second param: one-dim private int array
//third param: two-dim private int array 
void SMC_Utils::smc_batch(int* a, mpz_t* b, mpz_t** result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	
	mpz_t* a_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	for(int i = 0; i < size; i++)
		mpz_init_set_si(a_tmp[i], a[i]); 
	smc_batch(a_tmp, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);  
	smc_batch_free_operator(&a_tmp, size); 
}

//first param: one-dim private int array
//second param: int array
//third param: one-dim private int array
void SMC_Utils::smc_batch(mpz_t* a, int *b, mpz_t* result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	
	mpz_t* b_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	for(int i = 0; i < size; i++)
		mpz_init_set_si(b_tmp[i], b[i]);
	op = (op == "/") ? "/P" : op; 
	smc_batch(a, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	smc_batch_free_operator(&b_tmp, size); 
}

//first param: one-dim private int array
//second param: int array
//third param: two-dim private int array
void SMC_Utils::smc_batch(mpz_t* a, int *b, mpz_t** result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	mpz_t* b_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	for(int i = 0; i < size; i++)
		mpz_init_set_si(b_tmp[i], b[i]);
	op = (op == "/") ? "/P" : op; 
	smc_batch(a, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	smc_batch_free_operator(&b_tmp, size); 
}

//first param: integer array
//second param: two-dim private int
//assignment param: one-dim private int
void SMC_Utils::smc_batch(int *a, mpz_t** b, mpz_t* result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	mpz_t* a_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	for(int i = 0; i < size; i++)
		mpz_init_set_si(a_tmp[i], a[i]);
	smc_batch(a_tmp, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
	smc_batch_free_operator(&a_tmp, size); 	
}

//first param: integer array
//second param: two-dim private int
//assignment param: two-dim private int
void SMC_Utils::smc_batch(int *a, mpz_t** b, mpz_t** result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	mpz_t* a_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	for(int i = 0; i < size; i++)
		mpz_init_set_si(a_tmp[i], a[i]);
	smc_batch(a_tmp, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
	smc_batch_free_operator(&a_tmp, size); 	
}

//first param: two-dim private int 
//second param: integer array
//assignment param: one-dim private int
void SMC_Utils::smc_batch(mpz_t** a, int* b, mpz_t* result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	mpz_t* b_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	for(int i = 0; i < size; i++)
		mpz_init_set_si(b_tmp[i], b[i]);
	op = (op == "/") ? "/P" : op; 
	smc_batch(a, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
	smc_batch_free_operator(&b_tmp, size); 
}

//first param: two-dim private int 
//second param: integer array
//assignment param: two-dim private int
void SMC_Utils::smc_batch(mpz_t** a, int* b, mpz_t** result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	
	mpz_t* b_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	for(int i = 0; i < size; i++)
		mpz_init_set_si(b_tmp[i], b[i]);
	op = (op == "/") ? "/P" : op; 
	smc_batch(a, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID);
	//free the memory 	
	smc_batch_free_operator(&b_tmp, size); 
}

//first param: one-dim private int
//second param: two-dim private int
//assignment param: two-dim private int
void SMC_Utils::smc_batch(mpz_t* a, mpz_t** b, mpz_t** result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	int dim1, dim2; 
	mpz_t *a_tmp, *b_tmp, *result_tmp; 
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
	smc_convert_operator(&b_tmp, b, index_array, bdim, size, 2); 
	smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3); 

	smc_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID);

	if(resultdim != 0){
                for(int i = 0; i < size; ++i){
                        dim1 = index_array[3*i+2]/resultdim;
                        dim2 = index_array[3*i+2]%resultdim;
                        mpz_set(result[dim1][dim2], result_tmp[i]);
                }
        }

	smc_batch_free_operator(&a_tmp, size);
	smc_batch_free_operator(&b_tmp, size); 
	smc_batch_free_operator(&result_tmp, size);  
}

//first param: two-dim private int
//second param: one-dim private int
//assignment param: two-dim private int
void SMC_Utils::smc_batch(mpz_t** a, mpz_t* b, mpz_t** result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	int dim1, dim2; 
	mpz_t *a_tmp, *b_tmp, *result_tmp; 
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
	smc_convert_operator(&b_tmp, b, index_array, bdim, size, 2); 
	smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3); 
	
	smc_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID);

	if(resultdim != 0){
                for(int i = 0; i < size; ++i){
                        dim1 = index_array[3*i+2]/resultdim;
                        dim2 = index_array[3*i+2]%resultdim;
                        mpz_set(result[dim1][dim2], result_tmp[i]);
                }
	}
	
	smc_batch_free_operator(&a_tmp, size);
	smc_batch_free_operator(&b_tmp, size); 
	smc_batch_free_operator(&result_tmp, size);  
}

//first param: two-dim private int
//second param: two-dim private int
//assignment param: one-dim private int
void SMC_Utils::smc_batch(mpz_t** a, mpz_t** b, mpz_t* result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	if(op == "@"){
		mpz_t** result_tmp = (mpz_t**)malloc(sizeof(mpz_t*)); 
		result_tmp[0] = (mpz_t*)malloc(sizeof(mpz_t) * resultdim);
		for(int i = 0; i < resultdim; i++)
			mpz_init_set(result_tmp[0][i], result[i]);
		smc_batch(a, b, result_tmp, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
		for(int i = 0; i < resultdim; i++)
			mpz_set(result[i], result_tmp[0][i]); 
		smc_batch_free_operator(&(result_tmp[0]), resultdim); 
		free(result_tmp); 
		return; 
	}
	mpz_t *a_tmp, *b_tmp, *result_tmp; 
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
	smc_convert_operator(&b_tmp, b, index_array, bdim, size, 2);  
	smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);
	
	smc_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID);
	
	for(int i = 0; i < size; i++)	
		mpz_set(result[index_array[3*i+2]], result_tmp[i]);
	
	smc_batch_free_operator(&a_tmp, size);
	smc_batch_free_operator(&b_tmp, size); 
	smc_batch_free_operator(&result_tmp, size);  	
}

//first param: one-dim private int
//second param: one-dim private int
//assignment param: two-dim private int
void SMC_Utils::smc_batch(mpz_t* a, mpz_t* b, mpz_t** result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
    int dim1, dim2; 
    mpz_t *a_tmp, *b_tmp, *result_tmp; 
    smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
    smc_convert_operator(&b_tmp, b, index_array, bdim, size, 2); 
    smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3); 

    smc_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID);

    if(resultdim != 0){
                for(int i = 0; i < size; ++i){
                        dim1 = index_array[3*i+2]/resultdim;
                        dim2 = index_array[3*i+2]%resultdim;
                        mpz_set(result[dim1][dim2], result_tmp[i]);
                }
    }
	
    smc_batch_free_operator(&a_tmp, size); 
    smc_batch_free_operator(&b_tmp, size);
    smc_batch_free_operator(&result_tmp, size);  
	
}
//first param: one-dim private int
//second param: two-dim private int
//assignment param: one-dim private int
void SMC_Utils::smc_batch(mpz_t* a, mpz_t** b, mpz_t* result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
    
    mpz_t *a_tmp, *b_tmp, *result_tmp; 
    smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
    smc_convert_operator(&b_tmp, b, index_array, bdim, size, 2); 
    smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3); 

    smc_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID);
    
    for(int i = 0; i < size; i++)
		mpz_set(result[index_array[3*i+2]], result_tmp[i]);

    smc_batch_free_operator(&a_tmp, size); 
    smc_batch_free_operator(&b_tmp, size); 	
    smc_batch_free_operator(&result_tmp, size); 
}

//first param: two-dim private int
//second param: one-dim private int
//assignment param: one-dim private int
void SMC_Utils::smc_batch(mpz_t** a, mpz_t* b, mpz_t* result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
    
    mpz_t *a_tmp, *b_tmp, *result_tmp; 
    smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
    smc_convert_operator(&b_tmp, b, index_array, bdim, size, 2); 
    smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3); 
    
    smc_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID);
    
    for(int i = 0; i < size; i++)
		mpz_set(result[index_array[3*i+2]], result_tmp[i]);

    smc_batch_free_operator(&a_tmp, size); 	
    smc_batch_free_operator(&b_tmp, size); 
    smc_batch_free_operator(&result_tmp, size); 
}

void SMC_Utils::smc_batch_dot(mpz_t** a, mpz_t** b, int size, int array_size, int* index_array, mpz_t* result, int threadID)
{
	int a_dim = 0, b_dim = 0; 
	mpz_t **a_tmp = (mpz_t**)malloc(sizeof(mpz_t*) * size); 
	mpz_t **b_tmp = (mpz_t**)malloc(sizeof(mpz_t*) * size); 
	for(int i = 0 ; i < size; i++){
		   a_tmp[i] = (mpz_t*)malloc(sizeof(mpz_t) * array_size); 
		   b_tmp[i] = (mpz_t*)malloc(sizeof(mpz_t) * array_size); 
                   a_dim = index_array[3*i]; 
                   b_dim = index_array[3*i+1]; 
	           for(int j = 0; j < array_size; j++)
		   {
			mpz_init_set(a_tmp[i][j],a[a_dim][j]);
			mpz_init_set(b_tmp[i][j],b[b_dim][j]); 
		   }
	}

	smc_dot(a_tmp, b_tmp, size, array_size, result, "int", threadID); 
	
	//free the memory
	for(int i = 0; i < size; i++)
	{
		for(int j = 0; j < array_size; j++)
		{
			mpz_clear(a_tmp[i][j]); 
			mpz_clear(b_tmp[i][j]); 
		}
		free(a_tmp[i]); 
		free(b_tmp[i]); 
	}
	free(a_tmp); 
	free(b_tmp); 
	
}

//first param: two-dim private int
//second param: two-dim private int
//assignment param: two-dim private int

void SMC_Utils::smc_batch(mpz_t** a, mpz_t** b, mpz_t** result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	int dim1, dim2; 
	mpz_t *a_tmp, *b_tmp, *result_tmp, *result_org; 	
	if(op == "@"){
		result_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size);
		result_org = (mpz_t*)malloc(sizeof(mpz_t) * size);
		for(int i = 0; i < size; i++){
			mpz_init(result_tmp[i]);
			mpz_init(result_org[i]);  
			if(resultdim != 0){
				dim1 = index_array[3*i+2]/resultdim;
				dim2 = index_array[3*i+2]%resultdim; 
				mpz_set(result_org[i], result[dim1][dim2]);
			}
		}
		smc_batch_dot(a, b, size, adim, index_array, result_tmp, threadID); 
		smc_batch_handle_priv_cond(result_tmp, result_org, out_cond, priv_cond, counter, size, threadID); 
	}
	else{
    		smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
    		smc_convert_operator(&b_tmp, b, index_array, bdim, size, 2); 
    		smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3); 
    		smc_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID);
	}
	
	if(resultdim != 0){
		for(int i = 0; i < size; ++i){
			dim1 = index_array[3*i+2]/resultdim; 
			dim2 = index_array[3*i+2]%resultdim; 
			mpz_set(result[dim1][dim2], result_tmp[i]);
		}
	}

	if(op != "@"){
		smc_batch_free_operator(&a_tmp, size); 
		smc_batch_free_operator(&b_tmp, size); 
		smc_batch_free_operator(&result_tmp, size);
	}else{
		smc_batch_free_operator(&result_org, size); 
		smc_batch_free_operator(&result_tmp, size); 
	}
}

void SMC_Utils::smc_single_convert_to_private_float(float a, mpz_t** priv_a, int len_sig, int len_exp){
	mpz_t* elements = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
	*priv_a = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
	for(int i = 0; i < 4; i++){
		mpz_init((*priv_a)[i]);
		mpz_init(elements[i]); 
	}  
	convertFloat(a, len_sig, len_exp, &elements); 
	for(int i = 0; i < 4; i++){
		mpz_set((*priv_a)[i], elements[i]); 
		//convert the negative values to the positives in the filed
		ss->modAdd((*priv_a)[i], (*priv_a)[i], (long)0); 
	}
	//free the memory
	smc_batch_free_operator(&elements, 4); 
}

void SMC_Utils::smc_batch_convert_to_private_float(float* a, mpz_t*** priv_a, int len_sig, int len_exp, int size){
	mpz_t* elements = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
	*priv_a = (mpz_t**)malloc(sizeof(mpz_t*) * size); 
	
	for(int i = 0; i < 4; i++)
		mpz_init(elements[i]); 

	for(int i = 0; i < size; i++){
		(*priv_a)[i] = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
		for(int j = 0; j < 4; j++)
			mpz_init((*priv_a)[i][j]);  
	}

	for(int i = 0; i < size; i++){
		convertFloat(a[i], len_sig, len_exp, &elements); 
		for(int j = 0; j < 4; j++){
			mpz_set((*priv_a)[i][j], elements[j]); 
			ss->modAdd((*priv_a)[i][j], (*priv_a)[i][j], (long)0); 
		}
	}
	//free the memory
	smc_batch_free_operator(&elements, 4); 	
}


/*********************************************** FLOAT BATCH ****************************************************/
//public + private one-dimension float - arithmetic
void SMC_Utils::smc_batch(float* a, mpz_t** b, mpz_t*** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID)
{
        mpz_t** atmp; 
	smc_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size); 
	smc_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	smc_batch_free_operator(&atmp, size); 	
}

        
void SMC_Utils::smc_batch(mpz_t** a, float* b, mpz_t*** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID)
{
        mpz_t** btmp; 
	smc_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size); 
	op = (op == "/") ? "/P" : op; 
	smc_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	smc_batch_free_operator(&btmp, size); 	
}

void SMC_Utils::smc_batch(float* a, mpz_t** b, mpz_t** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID)
{
        mpz_t** atmp; 
	smc_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size); 
	smc_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	smc_batch_free_operator(&atmp, size); 	
}

void SMC_Utils::smc_batch(mpz_t** a, float* b, mpz_t** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID)
{
        mpz_t** btmp; 
	smc_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size); 
	op = (op == "/") ? "/P" : op; 
	smc_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	smc_batch_free_operator(&btmp, size); 	
}
//public to private assignments
void SMC_Utils::smc_batch(float* a, float* b, mpz_t*** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID)
{
	//mainly used for assignments, which means b will not be necessary
	mpz_t** atmp; 
	smc_batch_convert_to_private_float(a, &atmp, resultlen_sig, resultlen_exp, size); 
	smc_batch(atmp, atmp, result, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	smc_batch_free_operator(&atmp, size); 	
} 


void SMC_Utils::smc_batch(int* a, int* b, mpz_t*** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID)
{
	float* a1 = (float*)malloc(sizeof(float) * size); 
	for(int i = 0; i < size; i++)
		a1[i] = a[i]; 
	mpz_t** atmp; 
	smc_batch_convert_to_private_float(a1, &atmp, resultlen_sig, resultlen_exp, size); 
	smc_batch(atmp, atmp, result, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	smc_batch_free_operator(&atmp, size); 	
	free(a1); 
}
 
void SMC_Utils::smc_batch(float* a, float* b, mpz_t** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID)
{
	mpz_t** atmp; 
	smc_batch_convert_to_private_float(a, &atmp, resultlen_sig, resultlen_exp, size); 
	smc_batch(atmp, atmp, result, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	smc_batch_free_operator(&atmp, size); 	
} 


void SMC_Utils::smc_batch(int* a, int* b, mpz_t** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID)
{
	float* a1 = (float*)malloc(sizeof(float) * size); 
	for(int i = 0; i < size; i++)
		a1[i] = a[i]; 
	mpz_t** atmp; 
	smc_batch_convert_to_private_float(a1, &atmp, resultlen_sig, resultlen_exp, size); 
	smc_batch(atmp, atmp, result, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	smc_batch_free_operator(&atmp, size); 	
	free(a1); 
}

//public + private two-dimension float - arithmetic
void SMC_Utils::smc_batch(float* a, mpz_t*** b, mpz_t*** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID)
{
        mpz_t** atmp; 
	smc_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size); 
	smc_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	smc_batch_free_operator(&atmp, size); 	
}

void SMC_Utils::smc_batch(mpz_t*** a, float* b, mpz_t*** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID)
{
        mpz_t** btmp; 
	smc_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size); 
	op = (op == "/") ? "/P" : op; 
	smc_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	smc_batch_free_operator(&btmp, size); 	
}

void SMC_Utils::smc_batch(float* a, mpz_t*** b, mpz_t** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID)
{
        mpz_t** atmp; 
	smc_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size); 
	smc_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	smc_batch_free_operator(&atmp, size); 	
}

void SMC_Utils::smc_batch(mpz_t*** a, float* b, mpz_t** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID)
{
        mpz_t** btmp; 
	smc_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size); 
	op = (op == "/") ? "/P" : op; 
	smc_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	smc_batch_free_operator(&btmp, size); 	
}

//public + private one-dimension float - comparison
void SMC_Utils::smc_batch(float* a, mpz_t** b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID)
{
        mpz_t** atmp; 
	smc_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size); 
	smc_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	smc_batch_free_operator(&atmp, size); 	
}

void SMC_Utils::smc_batch(mpz_t** a, float* b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID)
{
        mpz_t** btmp; 
	smc_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size); 
	smc_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	smc_batch_free_operator(&btmp, size); 	
}

//public + private two-dimension float - comparison
void SMC_Utils::smc_batch(float* a, mpz_t*** b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID)
{
        mpz_t** atmp; 
	smc_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size); 
	smc_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	smc_batch_free_operator(&atmp, size); 	
}

void SMC_Utils::smc_batch(mpz_t*** a, float* b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID)
{
        mpz_t** btmp; 
	smc_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size); 
	smc_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	smc_batch_free_operator(&btmp, size); 	
}

void SMC_Utils::smc_batch(mpz_t** a, mpz_t** b, mpz_t** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
		
	mpz_t **a_tmp, **b_tmp, **result_tmp; 
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
	smc_convert_operator(&b_tmp, b, index_array, bdim, size, 2); 
	smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3); 
	
	smc_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID); 
	
	for(int i = 0; i < size; ++i)
		for(int j = 0; j < 4; j++)
			mpz_set(result[index_array[3*i+2]][j], result_tmp[i][j]);

	smc_batch_free_operator(&a_tmp, size); 
	smc_batch_free_operator(&b_tmp, size); 
	smc_batch_free_operator(&result_tmp, size); 
}

// first parameter: one-dim float
// second parameter: two-dim float
// assignment parameter: two-dim float

void SMC_Utils::smc_batch(mpz_t** a, mpz_t*** b, mpz_t*** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	mpz_t **a_tmp, **b_tmp, **result_tmp;
	int dim1, dim2;
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1);   
	smc_convert_operator(&b_tmp, b, index_array, bdim, size, 2); 
	smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3); 
	
	smc_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID); 

        for(int i = 0; i < size; ++i){
               for(int j = 0; j < 4; j++){
                          dim1 = index_array[3*i+2]/resultdim;
                          dim2 = index_array[3*i+2]%resultdim;
                          mpz_set(result[dim1][dim2][j],result_tmp[i][j]);
               }
        }

	smc_batch_free_operator(&a_tmp, size); 
	smc_batch_free_operator(&b_tmp, size); 
	smc_batch_free_operator(&result_tmp, size);    	 
}

// first parameter: two-dim float
// second parameter: one-dim float
// assignment parameter: two-dim float

void SMC_Utils::smc_batch(mpz_t*** a, mpz_t** b, mpz_t*** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	mpz_t **a_tmp, **b_tmp, **result_tmp; 
	int dim1, dim2; 
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
	smc_convert_operator(&b_tmp, b, index_array, bdim, size, 2); 
	smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3); 
	
	smc_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID); 
	
        for(int i = 0; i < size; ++i){
               for(int j = 0; j < 4; j++){
                          dim1 = index_array[3*i+2]/resultdim;
                          dim2 = index_array[3*i+2]%resultdim;
                          mpz_set(result[dim1][dim2][j],result_tmp[i][j]);
               }
        }

	smc_batch_free_operator(&a_tmp, size); 
	smc_batch_free_operator(&b_tmp, size); 
	smc_batch_free_operator(&result_tmp, size); 
}

// first parameter: two-dim float
// second parameter: two-dim float
// assignment parameter: one-dim float

void SMC_Utils::smc_batch(mpz_t*** a, mpz_t*** b, mpz_t** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	mpz_t **a_tmp, **b_tmp, **result_tmp;  	
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
	smc_convert_operator(&b_tmp, b, index_array, bdim, size, 2); 
	smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3); 
	
	smc_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID); 
	
	for(int i = 0; i < size; ++i)
               for(int j = 0; j < 4; j++)
                    mpz_set(result[index_array[3*i+2]][j], result_tmp[i][j]);

	smc_batch_free_operator(&a_tmp, size); 
	smc_batch_free_operator(&b_tmp, size); 
	smc_batch_free_operator(&result_tmp, size); 
}


// first parameter: two-dim float
// second parameter: one-dim float
// assignment parameter: one-dim float

void SMC_Utils::smc_batch(mpz_t*** a, mpz_t** b, mpz_t** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	mpz_t **a_tmp, **b_tmp, **result_tmp; 
	smc_convert_operator(&a_tmp, a, index_array, adim,  size, 1); 
	smc_convert_operator(&b_tmp, b, index_array, bdim, size, 2); 
	smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3); 
	
	smc_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID); 

	for(int i = 0; i < size; ++i)
               for(int j = 0; j < 4; j++)
                    mpz_set(result[index_array[3*i+2]][j], result_tmp[i][j]);
    
	smc_batch_free_operator(&a_tmp, size); 
	smc_batch_free_operator(&b_tmp, size); 
	smc_batch_free_operator(&result_tmp, size); 
}

// first parameter: one-dim float
// second parameter: two-dim float
// assignment parameter: one-dim float

void SMC_Utils::smc_batch(mpz_t** a, mpz_t*** b, mpz_t** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
    
	mpz_t **a_tmp, **b_tmp, **result_tmp;
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1);  
	smc_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
	smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3); 

	smc_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID); 

	for(int i = 0; i < size; ++i)
               for(int j = 0; j < 4; j++)
                    mpz_set(result[index_array[3*i+2]][j], result_tmp[i][j]);
	
	smc_batch_free_operator(&a_tmp, size); 	
	smc_batch_free_operator(&b_tmp, size); 
	smc_batch_free_operator(&result_tmp, size); 
}


// first parameter: one-dim float
// second parameter: one-dim float
// assignment parameter: two-dim float

void SMC_Utils::smc_batch(mpz_t** a, mpz_t** b, mpz_t*** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
    
	mpz_t **a_tmp, **b_tmp, **result_tmp;
	int dim1, dim2;  
	
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
	smc_convert_operator(&b_tmp, b, index_array, bdim, size, 2); 
	smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);
	
	smc_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID);

	for(int i = 0; i < size; ++i){
               for(int j = 0; j < 4; j++){
                          dim1 = index_array[3*i+2]/resultdim;
                          dim2 = index_array[3*i+2]%resultdim;
                          mpz_set(result[dim1][dim2][j], result_tmp[i][j]);
               }
        }

	smc_batch_free_operator(&a_tmp, size); 
	smc_batch_free_operator(&b_tmp, size); 
	smc_batch_free_operator(&result_tmp, size);  
}

// two dimension float general computation
void SMC_Utils::smc_batch(mpz_t*** a, mpz_t*** b, mpz_t*** result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	
	int dim1, dim2; 
	mpz_t **a_tmp, **b_tmp, **result_tmp; 
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
	smc_convert_operator(&b_tmp, b, index_array, bdim, size, 2); 
	smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3); 
	
	smc_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID);
	
	for(int i = 0; i < size; ++i){
		for(int j = 0; j < 4; j++){
			dim1 = index_array[3*i+2]/resultdim; 
			dim2 = index_array[3*i+2]%resultdim; 
			mpz_set(result[dim1][dim2][j],result_tmp[i][j]);
		}
	}
	
	smc_batch_free_operator(&a_tmp, size); 
	smc_batch_free_operator(&b_tmp, size); 
	smc_batch_free_operator(&result_tmp, size); 
}

//two-dimensional float comparison
void SMC_Utils::smc_batch(mpz_t*** a, mpz_t*** b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	mpz_t **a_tmp, **b_tmp; 
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
	smc_convert_operator(&b_tmp, b, index_array, bdim, size, 2); 
	
	smc_batch_BOP_float_comparison(result, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	smc_batch_free_operator(&a_tmp, size); 
	smc_batch_free_operator(&b_tmp, size); 
}

//one-dimensional float and two-dimensional float comparison
void SMC_Utils::smc_batch(mpz_t** a, mpz_t*** b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	mpz_t **a_tmp, **b_tmp; 
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
	smc_convert_operator(&b_tmp, b, index_array, bdim, size, 2); 

	smc_batch_BOP_float_comparison(result, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	smc_batch_free_operator(&a_tmp, size); 
	smc_batch_free_operator(&b_tmp, size); 	
}


//two-dimensional float and one-dimensional float comparison
void SMC_Utils::smc_batch(mpz_t*** a, mpz_t** b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){
	mpz_t **a_tmp, **b_tmp; 
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
	smc_convert_operator(&b_tmp, b, index_array, bdim, size, 2); 
	
	smc_batch_BOP_float_comparison(result, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	smc_batch_free_operator(&a_tmp, size); 
	smc_batch_free_operator(&b_tmp, size); 	
}

//one-dimensional float and one-dimensional float comparison
void SMC_Utils::smc_batch(mpz_t** a, mpz_t** b, mpz_t* result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, std::string op, std::string type, int threadID){

	mpz_t **a_tmp, **b_tmp; 
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
	smc_convert_operator(&b_tmp, b, index_array, bdim, size, 2); 
	
	smc_batch_BOP_float_comparison(result, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, op, type, threadID); 
	
	smc_batch_free_operator(&a_tmp, size); 
	smc_batch_free_operator(&b_tmp, size); 
}



/* conversion from public integer to private float*/
void SMC_Utils::smc_batch_int2fl(int* a, mpz_t** result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID)
{
	mpz_t* a_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	for(int i = 0; i < size; i++)
		mpz_init_set_si(a_tmp[i], a[i]); 
	smc_batch_int2fl(a_tmp, result, size, resultdim, 32, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID); 
	smc_batch_free_operator(&a_tmp, size); 
}

/* conversion from private integer to private float */
//one-dimensional int array to one-dimensional float array
void SMC_Utils::smc_batch_int2fl(mpz_t* a, mpz_t** result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID)
{
	mpz_t** result_tmp = (mpz_t**)malloc(sizeof(mpz_t*) * size); 
	mpz_t* a_tmp;
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1);
	for(int i = 0; i < size; i++){
		result_tmp[i] = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
		for(int j = 0; j < 4; j++)
			mpz_init(result_tmp[i][j]); 
	}
	I2F->doOperation(a_tmp, result_tmp, alen, resultlen_sig, size, threadID); 	
	/* consider the private condition */
	mpz_t* result_new = (mpz_t*)malloc(sizeof(mpz_t) * 4 * size); 
	mpz_t* result_org = (mpz_t*)malloc(sizeof(mpz_t) * 4 * size); 
	
	for(int i = 0; i < size; i++){
		for(int j = 0; j < 4; j++){
			mpz_init_set(result_new[4*i+j], result_tmp[i][j]); 
			mpz_init_set(result_org[4*i+j], result[index_array[3*i+2]][j]); 		
		}
	}

	smc_batch_handle_priv_cond(result_new, result_org, out_cond, priv_cond, counter, 4 * size, threadID); 
	for(int i = 0; i < size; i++)
		for(int j = 0; j < 4; j++)
			mpz_set(result[index_array[3*i+2]][j], result_new[4*i+j]); 	
	
	smc_batch_free_operator(&a_tmp, size); 
	smc_batch_free_operator(&result_new, 4 * size); 
	smc_batch_free_operator(&result_org, 4 * size); 
	smc_batch_free_operator(&result_tmp, size); 
}

void SMC_Utils::smc_batch_int2fl(int* a, mpz_t*** result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID)
{
	mpz_t* a_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size);
        for(int i = 0; i < size; i++)
                mpz_init_set_si(a_tmp[i], a[i]);
        smc_batch_int2fl(a_tmp, result, size, resultdim, 32, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID);
	smc_batch_free_operator(&a_tmp, size); 
}

//one-dimensional int array to two-dimensional float array
void SMC_Utils::smc_batch_int2fl(mpz_t* a, mpz_t*** result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID)
{

	mpz_t** result_tmp;	
	int* result_index_array = (int*)malloc(sizeof(int) * size); 
	smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);
	
	for(int i = 0; i < size; i++)
	{
		result_index_array[i] = index_array[3*i+2]; 
		index_array[3*i+2] = i; 
	}
	/* call one-dimensional int array + one-dimensional float array */
	smc_batch_int2fl(a, result_tmp, adim, size, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID); 
	/* set the results back */
	for(int i = 0; i < size; i++){
		for(int j = 0; j < 4; j++)
		{
			int dim1 = result_index_array[i]/resultdim; 
			int dim2 = result_index_array[i]%resultdim; 
			mpz_set(result[dim1][dim2][j], result_tmp[i][j]); 
		}
	}
	free(result_index_array);
	smc_batch_free_operator(&result_tmp, size);  
}

//two-dimensional int array to one-dimensional float array
void SMC_Utils::smc_batch_int2fl(mpz_t** a, mpz_t** result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID)
{
	mpz_t* a_tmp; 
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
	for(int i = 0; i < size; i++)
		index_array[3*i] = i;
	smc_batch_int2fl(a_tmp, result, size, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID); 
	smc_batch_free_operator(&a_tmp, size); 
}

//two-dimensional int array to two-dimensional float array
void SMC_Utils::smc_batch_int2fl(mpz_t** a, mpz_t*** result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID)
{
	mpz_t** result_tmp; 
	mpz_t* a_tmp; 
	int* result_index_array = (int*)malloc(sizeof(int) * size); 
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1);
	smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3); 
 
	for(int i = 0; i < size; i++)
	{
		result_index_array[i] = index_array[3*i+2]; 
		index_array[3*i] = i; 
		index_array[3*i+2] = i; 
	}
	
	/* call one-dimensional int array + one-dimensional float array	*/
	smc_batch_int2fl(a_tmp, result_tmp, size, size, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID); 

	/* set the results back */
	for(int i = 0; i < size; i++){
		for(int j = 0; j < 4; j++)
		{
			int dim1 = result_index_array[i]/resultdim; 
			int dim2 = result_index_array[i]%resultdim; 
			mpz_set(result[dim1][dim2][j], result_tmp[i][j]); 
		}
	}
	free(result_index_array); 
	smc_batch_free_operator(&a_tmp, size); 
	smc_batch_free_operator(&result_tmp, size); 
}

/*FL2INT*/
void SMC_Utils::smc_batch_fl2int(float* a, mpz_t* result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID){
	mpz_t** a_tmp = (mpz_t**)malloc(sizeof(mpz_t*) * size);
	for(int i = 0; i < size; i++){
		a_tmp[i] = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
		for(int j = 0; j < 4; j++)
			mpz_init(a_tmp[i][j]); 
		convertFloat(a[i], 32, 9, &a_tmp[i]);
	}
        smc_batch_fl2int(a_tmp, result, size, resultdim, 32, 9, blen, out_cond, priv_cond, counter, index_array, size, threadID);
        smc_batch_free_operator(&a_tmp, size);
}
        
void SMC_Utils::smc_batch_fl2int(float* a, mpz_t** result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID){
     	mpz_t** a_tmp = (mpz_t**)malloc(sizeof(mpz_t*) * size);
        for(int i = 0; i < size; i++){
                a_tmp[i] = (mpz_t*)malloc(sizeof(mpz_t) * 4);
		for(int j = 0; j < 4; j++)
			mpz_init(a_tmp[i][j]); 
                convertFloat(a[i], 32, 9, &a_tmp[i]);
        }
        smc_batch_fl2int(a_tmp, result, size, resultdim, 32, 9, blen, out_cond, priv_cond, counter, index_array, size, threadID);
        smc_batch_free_operator(&a_tmp, size);
}

void SMC_Utils::smc_batch_fl2int(mpz_t** a, mpz_t* result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID){
  	mpz_t* result_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size);
        mpz_t** a_tmp;
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 		
        for(int i = 0; i < size; i++)
		mpz_init(result_tmp[i]); 
	
	F2I->doOperation(a_tmp, result_tmp, alen_sig, alen_exp, blen, size, threadID);
        
	/* consider private conditions */
        mpz_t* result_org = (mpz_t*)malloc(sizeof(mpz_t) * size); 
        for(int i = 0; i < size; i++)
                mpz_init_set(result_org[i], result[index_array[3*i+2]]);            
        smc_batch_handle_priv_cond(result_tmp, result_org, out_cond, priv_cond, counter, size, threadID); 
        for(int i = 0; i < size; i++)
                mpz_set(result[index_array[3*i+2]], result_tmp[i]);         
        
        smc_batch_free_operator(&result_tmp, size); 
        smc_batch_free_operator(&result_org, size); 
        smc_batch_free_operator(&a_tmp, size); 
}

void SMC_Utils::smc_batch_fl2int(mpz_t** a, mpz_t** result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID){
	int dim1, dim2;
        mpz_t* result_tmp;
	smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3); 
        int* result_index_array = (int*)malloc(sizeof(int) * size);
        for(int i = 0; i < size; i++)
        {
              result_index_array[i] = index_array[3*i+2];
              index_array[3*i+2] = i;
        }
        smc_batch_fl2int(a, result_tmp, adim, size, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID);
        for(int i = 0; i < size; i++){
              dim1 = result_index_array[i]/resultdim;
              dim2 = result_index_array[i]%resultdim;
              mpz_set(result[dim1][dim2], result_tmp[i]);
        }
        free(result_index_array);
        smc_batch_free_operator(&result_tmp, size);
}

void SMC_Utils::smc_batch_fl2int(mpz_t*** a, mpz_t* result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID){
        mpz_t** a_tmp;
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
        for(int i = 0; i < size; i++)
            index_array[3*i] = i;
        smc_batch_fl2int(a_tmp, result, size, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID);
        smc_batch_free_operator(&a_tmp, size);
}

void SMC_Utils::smc_batch_fl2int(mpz_t*** a, mpz_t** result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID){
	int dim1, dim2;
        mpz_t* result_tmp;
        mpz_t** a_tmp;
        int* result_index_array = (int*)malloc(sizeof(int) * size);
       	
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
	smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3); 

	for(int i = 0; i < size; i++)
        {
            result_index_array[i] = index_array[3*i+2];
            index_array[3*i] = i;
            index_array[3*i+2] = i;
        }

        smc_batch_fl2int(a_tmp, result_tmp, size, size, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID);

        for(int i = 0; i < size; i++){
              dim1 = result_index_array[i]/resultdim; 
              dim2 = result_index_array[i]%resultdim; 
              mpz_set(result[dim1][dim2], result_tmp[i]); 
	}

        free(result_index_array); 
        smc_batch_free_operator(&a_tmp, size); 
        smc_batch_free_operator(&result_tmp, size); 
}
//INT2INT
void SMC_Utils::smc_batch_int2int(int* a, mpz_t* result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID){
	mpz_t* a_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size);
        for(int i = 0; i < size; i++)
                mpz_init_set_si(a_tmp[i], a[i]);
        smc_batch_int2int(a_tmp, result, size, resultdim, 32, blen, out_cond, priv_cond, counter, index_array, size, threadID);
        smc_batch_free_operator(&a_tmp, size);
}

void SMC_Utils::smc_batch_int2int(int* a, mpz_t** result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID){
	mpz_t* a_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size);
        for(int i = 0; i < size; i++)
                mpz_init_set_si(a_tmp[i], a[i]);
        smc_batch_int2int(a_tmp, result, size, resultdim, 32, blen, out_cond, priv_cond, counter, index_array, size, threadID);
        smc_batch_free_operator(&a_tmp, size);
}

void SMC_Utils::smc_batch_int2int(mpz_t* a, mpz_t* result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID){
	mpz_t* result_tmp = (mpz_t*)malloc(sizeof(mpz_t) * size);
        mpz_t* result_org = (mpz_t*)malloc(sizeof(mpz_t) * size);
        for(int i = 0; i < size; i++){
                mpz_init_set(result_tmp[i], a[index_array[3*i]]);
                mpz_init_set(result_org[i], result[index_array[3*i+2]]);
	}
        smc_batch_handle_priv_cond(result_tmp, result_org, out_cond, priv_cond, counter, size, threadID);
        for(int i = 0; i < size; i++)
                mpz_set(result[index_array[3*i+2]], result_tmp[i]);
        smc_batch_free_operator(&result_tmp, size);
        smc_batch_free_operator(&result_org, size);
}

void SMC_Utils::smc_batch_int2int(mpz_t** a, mpz_t* result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID){
        mpz_t* a_tmp;
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1);  
        for(int i = 0; i < size; i++)
		index_array[3*i] = i; 
        smc_batch_int2int(a_tmp, result, size, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID);
        smc_batch_free_operator(&a_tmp, size);
}

void SMC_Utils::smc_batch_int2int(mpz_t* a, mpz_t** result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID){
	mpz_t* result_tmp;
        smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3); 
	int* result_index_array = (int*)malloc(sizeof(int) * size);
        for(int i = 0; i < size; i++){
                result_index_array[i] = index_array[3*i+2];
                index_array[3*i+2] = i;
        }
        smc_batch_int2int(a, result_tmp, adim, size, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID); 
        for(int i = 0; i < size; i++){
                int dim1 = result_index_array[i]/resultdim; 
                int dim2 = result_index_array[i]%resultdim; 
                mpz_set(result[dim1][dim2], result_tmp[i]); 
	}
        free(result_index_array);
        smc_batch_free_operator(&result_tmp, size);  
}

void SMC_Utils::smc_batch_int2int(mpz_t** a, mpz_t** result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID){
        mpz_t *result_tmp, *a_tmp; 
        int* result_index_array = (int*)malloc(sizeof(int) * size);
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
	smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3); 
	
        for(int i = 0; i < size; i++)
        {
                result_index_array[i] = index_array[3*i+2];
                index_array[3*i] = i;
                index_array[3*i+2] = i;
        }
	
        smc_batch_int2int(a_tmp, result_tmp, size, size, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID);
	
	for(int i = 0; i < size; i++){
              int dim1 = result_index_array[i]/resultdim;
              int dim2 = result_index_array[i]%resultdim;
              mpz_set(result[dim1][dim2], result_tmp[i]);
	}
	
        free(result_index_array);
        smc_batch_free_operator(&a_tmp, size);
        smc_batch_free_operator(&result_tmp, size);
}

//FL2FL
void SMC_Utils::smc_batch_fl2fl(float* a, mpz_t** result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID){
	mpz_t** a_tmp = (mpz_t**)malloc(sizeof(mpz_t*) * size);
        for(int i = 0; i < size; i++){
                a_tmp[i] = (mpz_t*)malloc(sizeof(mpz_t) * 4);
                for(int j = 0; j < 4; j++)
                        mpz_init(a_tmp[i][j]);
                convertFloat(a[i], 32, 9, &a_tmp[i]);
        }
        smc_batch_fl2fl(a_tmp, result, size, resultdim, 32, 9, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID);
        smc_batch_free_operator(&a_tmp, size);
}

void SMC_Utils::smc_batch_fl2fl(float* a, mpz_t*** result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID){
	mpz_t** a_tmp = (mpz_t**)malloc(sizeof(mpz_t*) * size);
        for(int i = 0; i < size; i++){
                a_tmp[i] = (mpz_t*)malloc(sizeof(mpz_t) * 4);
                for(int j = 0; j < 4; j++)
                        mpz_init(a_tmp[i][j]);
                convertFloat(a[i], 32, 9, &a_tmp[i]);
        }
        smc_batch_fl2fl(a_tmp, result, size, resultdim, 32, 9, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID);
        smc_batch_free_operator(&a_tmp, size);
}

void SMC_Utils::smc_batch_fl2fl(mpz_t** a, mpz_t** result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID){
	
	//extract inputs
	mpz_t** result_tmp = (mpz_t**)malloc(sizeof(mpz_t*) * 4);
        mpz_t** a_tmp = (mpz_t**)malloc(sizeof(mpz_t*) * 4);
        for(int i = 0; i < 4; i++){
                a_tmp[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
		result_tmp[i] = (mpz_t*)malloc(sizeof(mpz_t) * size); 
                for(int j = 0; j < size; j++){
                        mpz_init_set(a_tmp[i][j], a[index_array[3*j]][i]);
			mpz_init(result_tmp[i][j]); 
		}
        }
	
	//start computation
	if(alen_sig >= blen_sig)
                T->doOperation(result_tmp[0], a_tmp[0], alen_sig, alen_sig-blen_sig, size, threadID);
        else{
                mpz_t tmp, two;
                mpz_init_set_ui(two, 2);
                mpz_init(tmp);
                ss->modPow(tmp, two, blen_sig-alen_sig);
                ss->modMul(result_tmp[0], a_tmp[0], tmp, size);
                mpz_clear(tmp);
                mpz_clear(two);
        }

        ss->modAdd(result_tmp[1], a_tmp[1], alen_sig-blen_sig, size);
        ss->copy(a_tmp[2], result_tmp[2], size); 
        ss->copy(a_tmp[3], result_tmp[3], size);
	
	/* consider private conditions */
	mpz_t* result_new = (mpz_t*)malloc(sizeof(mpz_t) * 4 * size);
        mpz_t* result_org = (mpz_t*)malloc(sizeof(mpz_t) * 4 * size);
        for(int i = 0; i < size; i++){
                for(int j = 0; j < 4; j++){
                        mpz_init_set(result_new[4*i+j], result_tmp[j][i]);
                        mpz_init_set(result_org[4*i+j], result[index_array[3*i+2]][j]);
                }
        }
        smc_batch_handle_priv_cond(result_new, result_org, out_cond, priv_cond, counter, 4 * size, threadID);
        for(int i = 0; i < size; i++)
                for(int j = 0; j < 4; j++)
                        mpz_set(result[index_array[3*i+2]][j], result_new[4*i+j]);

	//free memory
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < size; j++){
			mpz_clear(a_tmp[i][j]); 
			mpz_clear(result_tmp[i][j]); 
		}
		free(a_tmp[i]); 
		free(result_tmp[i]); 
	}
	free(a_tmp); 
	free(result_tmp);
 
	smc_batch_free_operator(&result_new, 4*size); 
	smc_batch_free_operator(&result_org, 4*size); 
}

void SMC_Utils::smc_batch_fl2fl(mpz_t** a, mpz_t*** result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID){
	mpz_t** result_tmp;
	smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3); 
        int* result_index_array = (int*)malloc(sizeof(int) * size);

        for(int i = 0; i < size; i++){
                result_index_array[i] = index_array[3*i+2];
                index_array[3*i+2] = i;
        }

        smc_batch_fl2fl(a, result_tmp, adim, size, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID);
        
	for(int i = 0; i < size; i++){
                for(int j = 0; j < 4; j++){
                        int dim1 = result_index_array[i]/resultdim;
                        int dim2 = result_index_array[i]%resultdim;
                        mpz_set(result[dim1][dim2][j], result_tmp[i][j]);
                }
	}
        free(result_index_array);
        smc_batch_free_operator(&result_tmp, size);
}

void SMC_Utils::smc_batch_fl2fl(mpz_t*** a, mpz_t** result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID){
        mpz_t** a_tmp;
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
        for(int i = 0; i < size; i++)
                index_array[3*i] = i;
        smc_batch_fl2fl(a_tmp, result, size, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID);
        smc_batch_free_operator(&a_tmp, size);
}

void SMC_Utils::smc_batch_fl2fl(mpz_t*** a, mpz_t*** result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t* priv_cond, int counter, int* index_array, int size, int threadID){
	mpz_t** result_tmp;
        mpz_t** a_tmp;
	int* result_index_array = (int*)malloc(sizeof(int) * size);
	smc_convert_operator(&a_tmp, a, index_array, adim, size, 1); 
	smc_convert_operator(&result_tmp, result, index_array, resultdim, size, 3); 

        for(int i = 0; i < size; i++){
                result_index_array[i] = index_array[3*i+2];
                index_array[3*i+2] = i;
		index_array[3*i] = i; 
        }
	
	smc_batch_fl2fl(a_tmp, result_tmp, size, size, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID);

        for(int i = 0; i < size; i++){
                for(int j = 0; j < 4; j++){
                        int dim1 = result_index_array[i]/resultdim;
                        int dim2 = result_index_array[i]%resultdim;
                        mpz_set(result[dim1][dim2][j], result_tmp[i][j]);
                }
	}
	
        free(result_index_array);
        smc_batch_free_operator(&result_tmp, size);
	smc_batch_free_operator(&a_tmp, size); 
}


/* Clienct Connection and Data Passing */
void SMC_Utils::clientConnect(){
        int sockfd, portno;
        socklen_t clilen;
        struct sockaddr_in server_addr, cli_addr;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd < 0)
                fprintf(stderr, "ERROR, opening socket\n");
        bzero((char*) &server_addr, sizeof(server_addr));
        portno = nodeConfig->getPort()+ 100;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(portno);
        if(bind(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0)
                        fprintf(stderr, "ERROR, on binding\n");
        listen(sockfd, 5);
        clilen = sizeof(cli_addr);
        newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &clilen);
        if(newsockfd < 0)
                fprintf(stderr, "ERROR, on accept\n");
	printf("Client connected\n");
}

void SMC_Utils::receivePolynomials(std::string privatekey_filename){ 
	FILE *prikeyfp = fopen(privatekey_filename.c_str(), "r");
	if( prikeyfp == NULL ) printf("File Open %s error\n", privatekey_filename.c_str());
	RSA *priRkey = PEM_read_RSAPrivateKey(prikeyfp, NULL, NULL, NULL);
	if( priRkey == NULL) printf("Read Private Key for RSA Error\n"); 
	char *buffer = (char*)malloc(RSA_size(priRkey));
	int n = read(newsockfd, buffer, RSA_size(priRkey));
	if (n < 0) printf("ERROR reading from socket \n");
	char *decrypt = (char*)malloc(n);
	memset(decrypt, 0x00, n); 
	int dec_len = RSA_private_decrypt(n, (unsigned char*)buffer, (unsigned char*)decrypt, priRkey, RSA_PKCS1_OAEP_PADDING);
	if(dec_len < 1) printf("RSA private decrypt error\n");
	if(dec_len < 1)
	{
		printf("RSA private decrypt error\n");
	} 

	int keysize = 0; 
	int coefsize = 0;
	int mpz_t_size = 0; 
	memcpy(&keysize, decrypt, sizeof(int)); 
	memcpy(&coefsize, decrypt+sizeof(int), sizeof(int)); 
	memcpy(&mpz_t_size, decrypt+sizeof(int)*2, sizeof(int));
	mpz_t* Keys = (mpz_t*)malloc(sizeof(mpz_t) * keysize); 
    	for(int k = 0; k < keysize; k++)
    		mpz_init(Keys[k]);
	int* Coefficients = (int*)malloc(sizeof(int) * coefsize); 
	int position = 0; 
	for(int i = 0; i < keysize; i++){ 
	  char strkey[mpz_t_size+1];
	  strkey[0] = 0; 
	  memcpy(strkey, decrypt+sizeof(int)*3+position, mpz_t_size); 
	  mpz_set_str(Keys[i], strkey, 10);		
	  position += mpz_t_size; 
	}
	memcpy(Coefficients, decrypt+sizeof(int)*3+mpz_t_size*keysize, sizeof(int)*coefsize); 
	free(buffer); 
	free(decrypt); 

	for(int i = 0; i < keysize; i++){
	  char strkey[mpz_t_size+1];
	  strkey[0] = 0; 
	  mpz_get_str(strkey, 10, Keys[i]); 
	  std::string Strkey = strkey; 
	  std::vector<int> temp;
	  for(int k = 0; k < coefsize/keysize; k++){
	    temp.push_back(Coefficients[i * coefsize/keysize + k]);
	  }
	  polynomials.insert(std::pair<std::string, std::vector<int> >(Strkey, temp));
	}
	//printf("Polynomials received... \n"); 
}

void SMC_Utils::setCoef(){
	mpz_t temp1, temp2, zero;
	mpz_init(temp1);
	mpz_init(temp2);
	mpz_init_set_ui(zero,0);

	for(int i=0; i<9; i++)
		mpz_init(coef[i]);

	mpz_set(coef[8], zero);

	mpz_set_ui(temp1, 40320);
	mpz_set_ui(temp2, 109584);
	ss->modInv(temp1, temp1);
	mpz_set(coef[7], temp1);
	ss->modMul(coef[7], coef[7], temp2);

	mpz_set_ui(temp2, 118124);
	mpz_set(coef[6], temp1);
	ss->modMul(coef[6], coef[6], temp2);
	ss->modSub(coef[6], zero, coef[6]);

	mpz_set_ui(temp2, 67284);
	mpz_set(coef[5], temp1);
	ss->modMul(coef[5], coef[5], temp2);

	mpz_set_ui(temp2, 22449);
	mpz_set(coef[4], temp1);
	ss->modMul(coef[4], coef[4], temp2);
	ss->modSub(coef[4], zero, coef[4]);

	mpz_set_ui(temp2, 4536);
	mpz_set(coef[3], temp1);
	ss->modMul(coef[3], coef[3], temp2);

	mpz_set_ui(temp2, 546);
	mpz_set(coef[2], temp1);
	ss->modMul(coef[2], coef[2], temp2);
	ss->modSub(coef[2], zero, coef[2]);

	mpz_set_ui(temp2, 36);
	mpz_set(coef[1], temp1);
	ss->modMul(coef[1], coef[1], temp2);
	
	mpz_set(coef[0], temp1);
	ss->modSub(coef[0], zero, coef[0]);
	
	mpz_clear(zero); 
	mpz_clear(temp1); 
	mpz_clear(temp2); 
}

void SMC_Utils::convertFloat(float value, int K, int L, mpz_t** elements)
{
        unsigned int* newptr = (unsigned int*)&value;
        int s = *newptr >> 31;
        int e = *newptr & 0x7f800000;
        e >>= 23;
        int m = 0;
        m = *newptr & 0x007fffff;

	
        int z;
        long v, p, k;
	mpz_t significant, one, two, tmp, tmpm; 
	mpz_init(significant); 
	mpz_init(tmp); 
	mpz_init_set_ui(one, 1); 
	mpz_init_set_ui(two, 2); 
	mpz_init(tmpm); 

        if(e == 0 && m == 0){
                s = 0;
                z = 1;
                //v = 0;
                mpz_set_ui(significant, 0); 
                p = 0;
        }else{
                z = 0;
                if(L < 8){
                        k = (1 << L) - 1;
/*check for overflow*/
                        if(e - 127 - K + 1 > k){
                                p = k;
				mpz_mul_2exp(significant, one, K); 
				mpz_sub_ui(significant, significant, 1); 
                                //v = (1 << K) - 1;
/*check for underflow*/
                        }else if(e - 127 -K + 1 < -k){
                                p = -k;
				mpz_set_ui(significant, 1); 
                                //v = 1;
                        }else{
                                p = e - 127 - K + 1;
                                m = m + (1 << 23);
				mpz_set_si(tmpm, m); 
                                if(K < 24){
					mpz_pow_ui(tmp, two, 24-K); 
					mpz_div(significant, tmpm, tmp); 
                                        //v = (m >> (24 - K));
                                }else{
                                        mpz_mul_2exp(significant, tmpm, K-24); 
					//v = m << (K - 24);
                                }
                        }
                }else{
                        p = e - 127 - K + 1;
                        m = m + (1 << 23);
			mpz_set_si(tmpm, m); 
                        if(K < 24){
				mpz_pow_ui(tmp, two, 24-K); 
				mpz_div(significant, tmpm, tmp); 
                                //v = (m >> (24 - K));
                        }else{
                        	mpz_set(significant, tmpm); 
				mpz_mul_2exp(significant, significant, K-24); 
				//v = m;
			        //v = v << (K - 24);
                        }
              }
        }

	//assignments;
	mpz_set((*elements)[0], significant); 
	mpz_set_si((*elements)[1], p); 
	mpz_set_si((*elements)[2], z); 
	mpz_set_si((*elements)[3], s); 

	//clear the memory
	mpz_clear(one); 
	mpz_clear(two); 
	mpz_clear(tmp); 
	mpz_clear(tmpm); 
	mpz_clear(significant); 
}

void SMC_Utils::convertDouble(double value, int K, int L, mpz_t** elements)
{
        unsigned long* newptr = (unsigned long*)&value;
        int s = (int)(*newptr >> 63);
        unsigned long temp = (1 << 11);
        temp--;
        temp = temp << 52;
        temp = (*newptr & temp);
        temp = temp >> 52;
        int e = (int) temp;
        unsigned long m = 0;
        temp = 1;
        temp = temp << 52;
        temp--;
        m = (*newptr & temp);

        int z;
        long v, p, k;
	
	mpz_t significant, tmp, tmpm, one, two; 
	mpz_init(significant); 
	mpz_init(tmp); 
	mpz_init(tmpm); 
	mpz_init_set_ui(one, 1); 
	mpz_init_set_ui(two, 2); 

        if(e == 0 && m == 0){
                s = 0;
                z = 1;
                //v = 0;
                mpz_set_ui(significant, 0); 
                p = 0;
        }else{
                z = 0;
                if(L < 11){
                        k = (1 << L);
                        k--;
/*check for overflow*/
                        if(e - 1023 - K + 1 > k){
                                p = k;
                                mpz_mul_2exp(significant, one, K); 
				mpz_sub_ui(significant, significant, 1); 
				//v = (1 << K);
                                //v--;
/*check for underflow*/
                        }else if(e - 1023 - K + 1 < -k){
                                p = -k;
                                mpz_set_ui(significant, 1); 
				//v = 1;
                        }else{
                                p = e - 1023 - K + 1;
                                k = 1;
                                k = k << 52;
                                m = m + k;
				mpz_set_si(tmpm, m); 
				
                                if(K < 53){
					mpz_pow_ui(tmp, two, 53-K); 
                                        mpz_div(significant, tmpm, tmp); 
					//v = (m >> (53 - K));
                     		}else{
                                        mpz_mul_2exp(significant, tmpm, K-53); 
					//v = m << (K - 53);
                                }
                        }
                }else{
                        p = e - 1023 - K + 1;
                        k = 1;
                        k = k << 52;
                        m = m + k;
			mpz_set_ui(tmpm, m); 
		
                        if(K < 53){
				mpz_pow_ui(tmp, two, 53-K); 
				mpz_div(significant, tmpm, tmp); 
                                //v = (m >> (53 - K));
                        }else{
                                mpz_mul_2exp(significant, tmpm, K-53); 
				//v = m << (K - 53);
                        }
                }
        }
	
	mpz_set((*elements)[0], significant); 
	mpz_set_si((*elements)[1], p); 
	mpz_set_si((*elements)[2], z); 
	mpz_set_si((*elements)[3], s); 
	
	//free the memory
	mpz_clear(one); 
	mpz_clear(two); 
	mpz_clear(tmp); 
	mpz_clear(tmpm); 
	mpz_clear(significant); 
	
	//(*elements)[0] = v; 
	//(*elements)[1] = p; 
	//(*elements)[2] = z; 
	//(*elements)[3] = s; 
}

double SMC_Utils::time_diff(struct timeval *t1, struct timeval *t2){
	double elapsed;

	if(t1->tv_usec > t2->tv_usec){
		t2->tv_usec += 1000000;
		t2->tv_sec--;
	}

	elapsed = (t2->tv_sec-t1->tv_sec) + (t2->tv_usec - t1->tv_usec)/1000000.0;

	return elapsed;
}

std::vector<std::string> SMC_Utils::splitfunc(const char* str, const char* delim)
{
    char* saveptr;
    char* token = strtok_r((char*)str, delim, &saveptr);
    std::vector<std::string> result;
    while(token != NULL)
    {
        result.push_back(token);
        token = strtok_r(NULL,delim,&saveptr);
    }
    return result;
}
	
