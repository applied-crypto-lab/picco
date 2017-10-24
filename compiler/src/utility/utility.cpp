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

#include "../seed/SecretShare.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <gmp.h>
#include <string.h>
using namespace std; 
// these will be eventually read from the config files;
int numOfComputeNodes;
int numOfInputNodes;
int numOfOutputNodes;
int party; 
int bits;
int threshold;


std::ifstream var_list;

SecretShare* ss;

void loadConfig(mpz_t);
void produceOutputs(std::ifstream[], std::ofstream[], std::string, std::string, int, int, int); 
void produceInputs(std::ifstream[], std::ofstream[], std::string, std::string, int, int, int, int, int); 
void openInputOutputFiles(std::string, std::string, std::ifstream*, std::ofstream*, int); 
void readVarList(std::ifstream&, std::ifstream[], std::ofstream[], int); 
void writeToOutputFile(std::ofstream&, std::string, std::string, int, int, int); 
void convertFloat(float value, int K, int L, mpz_t** elements); 
std::vector<std::string> splitfunc(const char* str, const char* delim);

int main(int argc, char** argv){
    
     if(argc != 6){
	fprintf(stderr,"Incorrect input parameters:\n");
	fprintf(stderr,"Usage: picco-utility -I/O <input/output party ID> <input/output filename> <utility-config> <share/result>\n");
	exit(1);
     }

    //set the mode: 0 - input, 1 - output
    /**************************************************/
    int mode; 
    if(!strcmp(argv[1], "-I") || !strcmp(argv[1], "-i"))
	mode = 0; 
    if(!strcmp(argv[1], "-O") || !strcmp(argv[1], "-o"))
	mode = 1; 
    /*************************************************/
    //read the input/ouptut party ID
    party = atoi(argv[2]); 
    /************************************************/
    //open var_list and set values for parameters
    var_list.open(argv[4], std::ifstream::in);
    if(!var_list){
        std::cout << "Variable list cannot be opened...\n";
        std::exit(1);
    }
    mpz_t modulus;
    mpz_init(modulus);
    loadConfig(modulus);
    int numOfInput, numOfOutput; 
    if(mode == 0)
    {
   	numOfInput = numOfInputNodes; 
    	numOfOutput = numOfComputeNodes; 
    } 
    else
    {
	numOfInput = numOfComputeNodes; 
	numOfOutput = numOfOutputNodes; 
    }
     
    std::ifstream inputFiles[numOfInput];
    std::ofstream outputFiles[numOfOutput];
    
    ss = new SecretShare(numOfComputeNodes, threshold, modulus);
    /******************************************************/
    //open all input and output files
    std::string file(argv[3]);
    std::string output(argv[5]); 
    stringstream s;
    openInputOutputFiles(file, output, inputFiles, outputFiles, mode);  
    /******************************************************/
    //read and analyze variable list
    readVarList(var_list, inputFiles, outputFiles, mode);  
    /******************************************************/
    // close all files
    var_list.close();
    for(int i = 0; i < numOfInput; i++)
        inputFiles[i].close();
    for(int i = 0; i < numOfOutput; i++)
        outputFiles[i].close();
}

void readVarList(std::ifstream& var_list, std::ifstream inputFiles[], std::ofstream outputFiles[], int mode)
{  
    std::string line;
    std::vector<std::string> tokens;
    std::vector<std::string> temp; 
    int secrecy, size, ID, sig_len, exp_len; 
    std::string name, type; 
    // read each line from var_list 
    if(mode == 0)
    {
    	while(std::getline(var_list, line)){
		temp = splitfunc(line.c_str(), ":");
		if(!temp[0].compare("I"))
		{
			tokens = splitfunc(temp[1].c_str(), ",");
        		secrecy = atoi(tokens[0].c_str()); 
			name = tokens[1];
        		type = tokens[2];
        		ID = atoi(tokens[3].c_str());
        		size = atoi(tokens[4].c_str());
			
			//process only the lines with their ID = party
			if(ID == party)
			{
        			if((!type.compare("int") && tokens.size() == 6) || (!type.compare("float") && tokens.size() == 7)){
					if(!type.compare("int"))		
            					produceInputs(inputFiles, outputFiles, name, type, 0, size, secrecy, atoi(tokens[5].c_str()), -1);
					else
						produceInputs(inputFiles, outputFiles, name, type, 0, size, secrecy, atoi(tokens[5].c_str()), atoi(tokens[6].c_str())); 
        			}
				else if((!type.compare("int") && tokens.size() == 7) || (!type.compare("float") && tokens.size() == 8)){
					if(!type.compare("int"))
						produceInputs(inputFiles, outputFiles, name, type, size, atoi(tokens[5].c_str()), secrecy, atoi(tokens[6].c_str()), -1); 
					else
            					produceInputs(inputFiles, outputFiles, name, type, size, atoi(tokens[5].c_str()), secrecy, atoi(tokens[6].c_str()), atoi(tokens[7].c_str()));

				}
			}
    		}
	}
    }
    else 
    {
	  while(std::getline(var_list, line)){
		temp = splitfunc(line.c_str(), ":");
          	if(!temp[0].compare("O"))
                {
			tokens = splitfunc(temp[1].c_str(), ",");
                        secrecy = atoi(tokens[0].c_str()); 
			name = tokens[1];
                        type = tokens[2];
                        ID = atoi(tokens[3].c_str());
                        size = atoi(tokens[4].c_str());
			//process only the lines with their ID == party
			if(ID == party)
			{
				if((!type.compare("int") && tokens.size() == 6) || (!type.compare("float") && tokens.size() == 7))
                                	produceOutputs(inputFiles, outputFiles, name, type, 0, size, secrecy);
                        	else if((!type.compare("int") && tokens.size() == 7) || (!type.compare("float") && tokens.size() == 8))
                                	produceOutputs(inputFiles, outputFiles, name, type, size, atoi(tokens[5].c_str()), secrecy);
			}
                }
	  }
    }
}

void openInputOutputFiles(std::string file, std::string output, std::ifstream inputFiles[], std::ofstream outputFiles[], int mode)
{  
    if(mode == 0)
    {
    	inputFiles[0].open(file.c_str(), std::ifstream::in);
    	if(!inputFiles[0]){
        	 std::cout << "input file could not be opened"<< std::endl;
         	std::exit(1);
    	}
    	for(int i = 1; i <= numOfComputeNodes; i++)
    	{
		stringstream s; 
		s << i; 
		std::string outfile = output + s.str(); 
		outputFiles[i-1].open(outfile.c_str(), std::ofstream::out);
        	if(!outputFiles[i-1]){
            		std::cout << "input file could not be opened" << std::endl;
            		std::exit(1);
        	}
	}
    }
    else
    {
	for(int i = 1; i <= numOfComputeNodes; i++)
        {
                stringstream s;
                s << i;
                std::string infile = file + s.str(); //file||ID
                inputFiles[i-1].open(infile.c_str(), std::ifstream::in);
                if(!inputFiles[i-1]){
                	std::cout << "input file could not be opened" << std::endl;
                	std::exit(1);
		}
        }
    	outputFiles[0].open(output.c_str(), std::ofstream::out);
    	if(!outputFiles[0]){
        	 std::cout << "input file could not be opened" << std::endl;
         	 std::exit(1);
    	}

    }
}

void writeToOutputFile(std::ofstream& outputFile, std::string value, std::string token, int secrecy, int index, int size) 
{			
	if(secrecy == 1)
	{
		if(index != size-1)
       			outputFile << value+",";
       		else
       			outputFile << value+"\n";
	}
	else
	{
       		if(index != size-1)
        		outputFile << token+",";
               	else
               		outputFile << token+"\n";
	}
}

void produceOutputs(std::ifstream inputFiles[], std::ofstream outputFiles[], std::string name, std::string type, int size1, int size2, int secrecy)
{
    std::string line;
    std::string value;
    std::vector<std::string> tokens;
    std::vector<std::string> temp;
    mpz_t element;
    int base = 10;
    int dim = (size1 == 0) ? 1 : size1; 
    // initialization
    mpz_init(element);
    //works for both one or two dimensional arrays
   if(!type.compare("int"))
    {
	mpz_t** shares = (mpz_t**)malloc(sizeof(mpz_t*) * numOfComputeNodes); 
	mpz_t* result = (mpz_t*)malloc(sizeof(mpz_t) * size2); 
	for(int i = 0; i < numOfComputeNodes; i++)
	{
		shares[i] = (mpz_t*)malloc(sizeof(mpz_t) * size2); 
		for(int j = 0; j < size2; j++)
			mpz_init(shares[i][j]); 
	}
	for(int i = 0; i < size2; i++)
		mpz_init(result[i]); 
	
	for(int i = 0; i < dim; i++)
	{
		//extract the shares 
		for(int k = 0; k < numOfComputeNodes; k++)
		{
			std::getline(inputFiles[k], line);
			tokens = splitfunc(line.c_str(), ",");
			if(secrecy == 1)
				for(int j = 0; j < tokens.size(); j++)
					mpz_set_str(shares[k][j], tokens[j].c_str(), 10); 
		}
		if(secrecy == 1)
	       		ss->reconstructSecret(result, shares, size2, false);
        	for(int j = 0; j < tokens.size(); j++)
        	{
			//for single variable or one-dimension
                	if(size1 == 0 && j == 0)
                    		outputFiles[0] << name+"=";
			//for two-dimension
			else if(size1 != 0 && j == 0)
			{
				stringstream s; 
				s << i; 
				outputFiles[0] << name+"["+s.str()+"]="; 
			}
			if(secrecy == 1){
				   //deal with negative results
				   mpz_t tmp, field;
        			   mpz_init(tmp); mpz_init(field);
        			   ss->getFieldSize(field);
        			   mpz_mul_ui(tmp, result[j], 2);
        			   if(mpz_cmp(tmp, field) > 0)
                			mpz_sub(result[j], result[j], field);
                		   value = mpz_get_str(NULL, base, result[j]);
			}
			writeToOutputFile(outputFiles[0], value, tokens[j], secrecy, j, tokens.size()); 
		}
      }
     //clear the memory 
     for(int i = 0; i < numOfComputeNodes; i++)
     {
            for(int j = 0; j < size2; j++)
                    mpz_clear(shares[i][j]);
	    free(shares[i]); 
     }
     free(shares); 
     for(int i = 0; i < size2; i++)
            mpz_clear(result[i]);
     free(result); 

    }
    //public float
    else if(!type.compare("float") && secrecy == 2)
    {

	for(int i = 0; i < dim; i++)
	{
		for(int k = 0; k < numOfComputeNodes; k++)
                {
			std::getline(inputFiles[k], line);
			tokens = splitfunc(line.c_str(), ",");
		}
        	for(int j = 0; j < tokens.size(); j++)
        	{
                	if(size1 == 0 && j == 0)
                    		outputFiles[0] << name+"=";
			else if(size1 != 0 && j == 0)
			{
				stringstream s1, s2; 
				s1 << i; s2 << j; 
				outputFiles[0] << name+"["+s1.str()+"]["+s2.str()+"]="; 
			}
			writeToOutputFile(outputFiles[0], "", tokens[j], 2, j, tokens.size()); 
		}
            }
    }
    //private float
    else if(!type.compare("float") && secrecy == 1)
    {
	mpz_t** shares = (mpz_t**)malloc(sizeof(mpz_t*) * numOfComputeNodes); 
	mpz_t* result = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
	for(int i = 0; i < numOfComputeNodes; i++)
	{
		shares[i] = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
		for(int j = 0; j < 4; j++)
			mpz_init(shares[i][j]); 
	}
	for(int i = 0; i < 4; i++)
		mpz_init(result[i]);
	for(int i = 0; i < dim; i++)
       	{
		for(int j = 0; j < size2; j++)
		{
			for(int k = 0; k < numOfComputeNodes; k++)
			{
				std::getline(inputFiles[k], line);
				tokens = splitfunc(line.c_str(), ",");
				for(int l = 0; l < 4; l++)
					mpz_set_str(shares[k][l], tokens[l].c_str(), 10); 
			}
	       		ss->reconstructSecret(result, shares, 4, false);
			for(int k = 0; k < 4; k++){
                		if(k == 1){
                        		mpz_t tmp, field;
                        		mpz_init(tmp); mpz_init(field);
                        		ss->getFieldSize(field);
                        		mpz_mul_ui(tmp, result[1], 2);
                        		if(mpz_cmp(tmp, field) > 0)
                                		mpz_sub(result[1], result[1], field);
                		}
        		}
        		double v = mpz_get_d(result[0]);
        		double p = mpz_get_d(result[1]);
        		double z = mpz_get_d(result[2]);
        		double s = mpz_get_d(result[3]);
        		double element = 0; 
			if(z == 1) element = 0; 
        		else{
                		element = v * pow(2, p);
                		if(s == 1) element = -element; 
                	}
			if(j == 0){
               			if(size1 == 0)
               				outputFiles[0] << name+"=";
				else{
					stringstream s1; 
					s1 << i;  
					outputFiles[0] << name+"["+s1.str()+"]="; 
				}
			}
			std::ostringstream ss;
    			ss << element;
			writeToOutputFile(outputFiles[0], ss.str(), "", 1, j, size2); 
        	}
	}
	//free the memory
        for(int i = 0; i < numOfComputeNodes; i++)
        {
                for(int j = 0; j < 4; j++)
                        mpz_clear(shares[i][j]);
		free(shares[i]); 
        }
	free(shares); 
        for(int i = 0; i < 4; i++)
                mpz_clear(result[i]);
	free(result); 
    }
    else
    {
        std::cout<<"Wrong type has been detected";
        std::exit(1);
    }
}

void produceInputs(std::ifstream inputFiles[], std::ofstream outputFiles[], std::string name, std::string type, int size1, int size2, int secrecy, int len_sig, int len_exp)
{
    std::string line;
    std::string share;
    std::vector<std::string> tokens;
    std::vector<std::string> temp;
    mpz_t element;
    mpz_t* shares = (mpz_t*)malloc(sizeof(mpz_t) * numOfComputeNodes);
    int base = 10;
    // initialization
    mpz_init(element);
    for(int i = 0; i < numOfComputeNodes; i++)
        mpz_init(shares[i]);
    int dim = (size1 == 0) ? 1 : size1; 
    //works for both one or two dimensional arrays
    if(!type.compare("int"))
    {
	for(int i = 0; i < dim; i++)
	{
		std::getline(inputFiles[0], line);
		temp = splitfunc(line.c_str(), "=");
		tokens = splitfunc(temp[1].c_str(), ",");
	
        	for(int j = 0; j < tokens.size(); j++)
        	{
            		mpz_set_str(element, tokens[j].c_str(), 10);
			if(secrecy == 1)
            			ss->getShares(shares, element);
            		for(int k = 0; k < numOfComputeNodes; k++)
            		{
                		if(j == 0)
                    			outputFiles[k] << name+"=";
				if(secrecy == 1)
					share = mpz_get_str(NULL, base, shares[k]); 
				
				writeToOutputFile(outputFiles[k], share, tokens[j], secrecy, j, tokens.size()); 
			}
            	}
        }
    }
    else if(!type.compare("float") && secrecy == 2)
    {
	 for(int i = 0; i < dim; i++)
        {
                std::getline(inputFiles[0], line);
		temp = splitfunc(line.c_str(), "=");
		tokens = splitfunc(temp[1].c_str(), ",");
		
                for(int j = 0; j < tokens.size(); j++)
                {
                        for(int k = 0; k < numOfComputeNodes; k++)
                        {
                                if(j == 0)
                                        outputFiles[k] << name+"=";
				writeToOutputFile(outputFiles[k], "", tokens[j], 2, j, tokens.size()); 
                        }
               }
        }

    }
    else if(!type.compare("float") && secrecy == 1)
    {
	for(int i = 0; i < dim; i++)
       	{
		std::getline(inputFiles[0], line);
		temp = splitfunc(line.c_str(), "=");
		tokens = splitfunc(temp[1].c_str(), ",");
		
		for(int j = 0; j < tokens.size(); j++)
		{
       			
			mpz_t* elements = (mpz_t*)malloc(sizeof(mpz_t) * 4); 
			for(int k = 0; k < 4; k++)
				mpz_init(elements[k]); 
			
			convertFloat((float)atof(tokens[j].c_str()), len_sig, len_exp, &elements); 
			
			for(int m = 0; m < 4; m++)
       			{
              			ss->getShares(shares, elements[m]);
               			for(int k = 0; k < numOfComputeNodes; k++)
               			{
              				if(m == 0)
                      				outputFiles[k] << name+"=";
                 	 		share = mpz_get_str(NULL, base, shares[k]);
					writeToOutputFile(outputFiles[k], share, "", 1, m, 4); 
				}
               		}
			
			for(int k = 0; k < 4; k++)
				mpz_clear(elements[k]); 
			free(elements); 
		}
        }
    }
    else
    {
        std::cout<<"Wrong type has been detected";
        std::exit(1);
    }
	
    //free the memory
    mpz_clear(element);
    for(int i = 0; i < numOfComputeNodes; i++)
        mpz_clear(shares[i]);
    free(shares); 
}

void loadConfig(mpz_t mod)
{
    std::string line;
    std::vector<std::string> tokens;
    int results[6]; 
    for(int i = 0; i < 6; i++) 
    {
        std::getline(var_list, line);
	tokens = splitfunc(line.c_str(), ":");
        if(i == 1) 
	{ 
 		mpz_set_str(mod, tokens[1].c_str(), 10);
		//gmp_printf("%Zd\n", mod);	
	}
	else
		results[i] = atoi(tokens[1].c_str());
    }
    bits = results[0];
    numOfComputeNodes = results[2]; 
    threshold = results[3]; 
    numOfInputNodes = results[4]; 
    numOfOutputNodes = results[5]; 
}


void convertFloat(float value, int K, int L, mpz_t** elements)
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
                mpz_set_ui(significant, 0);
                p = 0;
        }else{
                z = 0;
                if(L < 8){
                        k = (1 << L) - 1;
                        if(e - 127 - K + 1 > k){
                                p = k;
                                mpz_mul_2exp(significant, one, K);
                                mpz_sub_ui(significant, significant, 1);
                        }else if(e - 127 -K + 1 < -k){
                                p = -k;
                                mpz_set_ui(significant, 1);
                        }else{
                                p = e - 127 - K + 1;
                                m = m + (1 << 23);
                                mpz_set_si(tmpm, m);
                                if(K < 24){
                                        mpz_pow_ui(tmp, two, 24-K);
                                        mpz_div(significant, tmpm, tmp);
                                }else{
                                        mpz_mul_2exp(significant, tmpm, K-24);
                                }
                        }
         }else{
                        p = e - 127 - K + 1;
                        m = m + (1 << 23);
                        mpz_set_si(tmpm, m);
                        if(K < 24){
                                mpz_pow_ui(tmp, two, 24-K);
                                mpz_div(significant, tmpm, tmp);
                        }else{
                                mpz_set(significant, tmpm);
                                mpz_mul_2exp(significant, significant, K-24);
                        }
              }
        }

        mpz_set((*elements)[0], significant);
        mpz_set_si((*elements)[1], p);
        mpz_set_si((*elements)[2], z);
        mpz_set_si((*elements)[3], s);

        mpz_clear(one);
        mpz_clear(two);
        mpz_clear(tmp);
        mpz_clear(tmpm);
        mpz_clear(significant);
}


std::vector<std::string> splitfunc(const char* str, const char* delim)
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
