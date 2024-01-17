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

#include "SecretShare.h"
#include "../shared.h"
#include <cmath>
#include <fstream>
#include <gmp.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <math.h>
using namespace std;

// these will be eventually read from the config files;
int numOfComputeNodes;
int numOfInputNodes;
int numOfOutputNodes;
int party;
int bits;
int threshold;

mpz_t modulus_shamir; // Global modulus_shamir variable
int technique; // Global technique variable 

std::ifstream var_list;

SecretShare *ss;

void loadConfig();
void produceOutputs(std::ifstream[], std::ofstream[], std::string, std::string, int, int, int);
void produceInputs(std::ifstream[], std::ofstream[], std::string, std::string, int, int, int, int, int);
void openInputOutputFiles(std::string, std::string, std::ifstream *, std::ofstream *, int);
void readVarList(std::ifstream &, std::ifstream[], std::ofstream[], int);
void writeToOutputFile(std::ofstream &, std::string, std::string, int, int, int);
void convertFloat(float value, int K, int L, long long **elements);

int main(int argc, char **argv) {

    if (argc != 6) {
        fprintf(stderr, "Incorrect input parameters:\n");
        fprintf(stderr, "Usage: picco-utility -I/O <input/output party ID> <input/output filename> <utility-config> <share/result>\n");
        exit(1);
    }

    // set the mode: 0 - input, 1 - output
    /**************************************************/
    int mode;
    if (!strcmp(argv[1], "-I") || !strcmp(argv[1], "-i"))
        mode = 0;
    if (!strcmp(argv[1], "-O") || !strcmp(argv[1], "-o"))
        mode = 1;
    /*************************************************/
    // read the input/ouptut party ID
    party = atoi(argv[2]);
    /************************************************/
    // open var_list and set values for parameters
    var_list.open(argv[4], std::ifstream::in);
    if (!var_list) {
        std::cout << "Variable list cannot be opened...\n";
        std::exit(1);
    }

    mpz_init(modulus_shamir);
    loadConfig();
    int numOfInput, numOfOutput;
    if (mode == 0) {
        numOfInput = numOfInputNodes;
        numOfOutput = numOfComputeNodes;
    } else {
        numOfInput = numOfComputeNodes;
        numOfOutput = numOfOutputNodes;
    }

    std::ifstream inputFiles[numOfInput];
    std::ofstream outputFiles[numOfOutput];

    // this will go where we determine which technique we're using
    // ss = new ShamirSS(numOfComputeNodes, threshold, modulus_shamir);
    // ss = new RSS<uint64_t>(numOfComputeNodes, threshold, 64);
    // return 0;
    // testing polymorphism
    // mp z_t field_test;
    // mp z_init(field_test);
    // ss->getFieldSize(field_test);
    // gmp_printf("field_test %Zd\n", field_test);
    // return 0;

    /******************************************************/
    // open all input and output files
    std::string file(argv[3]);
    std::string output(argv[5]);
    stringstream s;
    openInputOutputFiles(file, output, inputFiles, outputFiles, mode);
    /******************************************************/
    // read and analyze variable list
    readVarList(var_list, inputFiles, outputFiles, mode);
    /******************************************************/
    // close all files
    var_list.close();
    for (int i = 0; i < numOfInput; i++)
        inputFiles[i].close();
    for (int i = 0; i < numOfOutput; i++)
        outputFiles[i].close();
}

void readVarList(std::ifstream &var_list, std::ifstream inputFiles[], std::ofstream outputFiles[], int mode) {
    std::string line;
    std::vector<std::string> tokens;
    std::vector<std::string> temp;
    int secrecy, size, ID, sig_len, exp_len;
    std::string name, type;
    // read each line from var_list
    if (mode == 0) {
        while (std::getline(var_list, line)) {
            temp = splitfunc(line.c_str(), ":");
            if (!temp[0].compare("I")) {
                tokens = splitfunc(temp[1].c_str(), ",");
                secrecy = atoi(tokens[0].c_str());
                name = tokens[1];
                type = tokens[2];
                ID = atoi(tokens[3].c_str());
                size = atoi(tokens[4].c_str());

                // process only the lines with their ID = party
                if (ID == party) {
                    if ((!type.compare("int") && tokens.size() == 6) || (!type.compare("float") && tokens.size() == 7)) {
                        if (!type.compare("int"))
                            produceInputs(inputFiles, outputFiles, name, type, 0, size, secrecy, atoi(tokens[5].c_str()), -1);
                        else
                            produceInputs(inputFiles, outputFiles, name, type, 0, size, secrecy, atoi(tokens[5].c_str()), atoi(tokens[6].c_str()));
                    } else if ((!type.compare("int") && tokens.size() == 7) || (!type.compare("float") && tokens.size() == 8)) {
                        if (!type.compare("int"))
                            produceInputs(inputFiles, outputFiles, name, type, size, atoi(tokens[5].c_str()), secrecy, atoi(tokens[6].c_str()), -1);
                        else
                            produceInputs(inputFiles, outputFiles, name, type, size, atoi(tokens[5].c_str()), secrecy, atoi(tokens[6].c_str()), atoi(tokens[7].c_str()));
                    }
                }
            }
        }
    } else {
        while (std::getline(var_list, line)) {
            temp = splitfunc(line.c_str(), ":");
            if (!temp[0].compare("O")) {
                tokens = splitfunc(temp[1].c_str(), ",");
                secrecy = atoi(tokens[0].c_str());
                name = tokens[1];
                type = tokens[2];
                ID = atoi(tokens[3].c_str());
                size = atoi(tokens[4].c_str());
                // process only the lines with their ID == party
                if (ID == party) {
                    if ((!type.compare("int") && tokens.size() == 6) || (!type.compare("float") && tokens.size() == 7))
                        produceOutputs(inputFiles, outputFiles, name, type, 0, size, secrecy);
                    else if ((!type.compare("int") && tokens.size() == 7) || (!type.compare("float") && tokens.size() == 8))
                        produceOutputs(inputFiles, outputFiles, name, type, size, atoi(tokens[5].c_str()), secrecy);
                }
            }
        }
    }
}

void openInputOutputFiles(std::string file, std::string output, std::ifstream inputFiles[], std::ofstream outputFiles[], int mode) {
    if (mode == 0) {
        inputFiles[0].open(file.c_str(), std::ifstream::in);
        if (!inputFiles[0]) {
            std::cout << "input file could not be opened" << std::endl;
            std::exit(1);
        }
        for (int i = 1; i <= numOfComputeNodes; i++) {
            stringstream s;
            s << i;
            std::string outfile = output + s.str();
            outputFiles[i - 1].open(outfile.c_str(), std::ofstream::out);
            if (!outputFiles[i - 1]) {
                std::cout << "input file could not be opened" << std::endl;
                std::exit(1);
            }
        }
    } else {
        for (int i = 1; i <= numOfComputeNodes; i++) {
            stringstream s;
            s << i;
            std::string infile = file + s.str(); // file||ID
            inputFiles[i - 1].open(infile.c_str(), std::ifstream::in);
            if (!inputFiles[i - 1]) {
                std::cout << "input file could not be opened" << std::endl;
                std::exit(1);
            }
        }
        outputFiles[0].open(output.c_str(), std::ofstream::out);
        if (!outputFiles[0]) {
            std::cout << "input file could not be opened" << std::endl;
            std::exit(1);
        }
    }
}

void writeToOutputFile(std::ofstream &outputFile, std::string value, std::string token, int secrecy, int index, int size) {
    if (secrecy == 1) {
        if (index != size - 1)
            outputFile << value + ",";
        else
            outputFile << value + "\n";
    } else {
        if (index != size - 1)
            outputFile << token + ",";
        else
            outputFile << token + "\n";
    }
}

/*
The function produceOutputs() read the data from inputFiles, processes it based on the specified 
data type and secrecy level, and writes the results to the outputFiles. This function generates 
output files based on the specified input files, variable information, data type, secrecy level, 
and dimensions. The generated outputs can be either public (non-secret) or private (secret-shared) 
computations, depending on the parameters provided. Important variables used in this function are: 

    1. element; Represents an individual element during the computation.
    2. **shares; Represents an array of shares used in secure multi-party computation.
    3. *result; Represents the result of the secure multi-party computation.
*/

void produceOutputs(std::ifstream inputFiles[], std::ofstream outputFiles[], std::string name, std::string type, int size1, int size2, int secrecy) {
    std::string line;
    std::string value;
    std::vector<std::string> tokens;
    std::vector<std::string> temp;
    long long element = 0;
    int dim = (size1 == 0) ? 1 : size1;

    // works for both one or two dimensional arrays
    if (!type.compare("int")) {
        std::vector<std::vector<std::string>> shares;
        // Set the size for shares[x][x] to numOfComputeNodes and for shares[x] to size2
        shares.resize(numOfComputeNodes, std::vector<std::string>(size2)); 
        // Set the size for result to size2
        std::vector<long long> result(size2);
        for (int i = 0; i < dim; i++) {
            // extract the shares
            for (int k = 0; k < numOfComputeNodes; k++) {
                std::getline(inputFiles[k], line);
                tokens = splitfunc(line.c_str(), ",");
                if (secrecy == 1)
                    for (int j = 0; j < tokens.size(); j++)
                        shares[k][j] = tokens[j];
                        /* 
                        mpz_set_str is used to convert a string (tokens[j]) to an 
                            arbitrary precision integer (shares[k][j]), taking into 
                            account the base (10 in this case).
                        
                        In the second implementation the vector token of type string is
                            directly set to the vector of shares[k]. To do this the 
                            fucntion push_back() is used to make sure there isn't any 
                            undefined behavior cause of the sizes of both vectors. 
                            
                        */
            }
            if (secrecy == 1)
                result = ss->reconstructSecret(shares, size2);

            for (int j = 0; j < tokens.size(); j++) {
                // for single variable or one-dimension
                if (size1 == 0 && j == 0)
                    outputFiles[0] << name + "=";
                // for two-dimension
                else if (size1 != 0 && j == 0) {
                    stringstream s;
                    s << i;
                    outputFiles[0] << name + "[" + s.str() + "]=";
                }
                if (secrecy == 1) {
                    // deal with negative results
                    ss->flipNegative(result[j]);
                    value = std::to_string(result[j]);  // The reconstructSecret returns long long so we need to convert it to str before outputting
                }
                writeToOutputFile(outputFiles[0], value, tokens[j], secrecy, j, tokens.size());
            }
        }
    }
    // public float
    else if (!type.compare("float") && secrecy == 2) {
        for (int i = 0; i < dim; i++) {
            for (int k = 0; k < numOfComputeNodes; k++) {
                std::getline(inputFiles[k], line);
                tokens = splitfunc(line.c_str(), ",");
            }
            for (int j = 0; j < tokens.size(); j++) {
                if (size1 == 0 && j == 0)
                    outputFiles[0] << name + "=";
                else if (size1 != 0 && j == 0) {
                    stringstream s1, s2;
                    s1 << i;
                    s2 << j;
                    outputFiles[0] << name + "[" + s1.str() + "][" + s2.str() + "]=";
                }
                writeToOutputFile(outputFiles[0], "", tokens[j], 2, j, tokens.size());
            }
        }
    }
    // private float
    else if (!type.compare("float") && secrecy == 1) {
        std::vector<std::vector<std::string>> shares;
        shares.resize(numOfComputeNodes, std::vector<std::string>(4));  // Resize the vector of vectors
        std::vector<long long> result(4);

        for (int i = 0; i < dim; i++) {
            for (int j = 0; j < size2; j++) {
                for (int k = 0; k < numOfComputeNodes; k++) {
                    std::getline(inputFiles[k], line);
                    tokens = splitfunc(line.c_str(), ",");
                    for (int l = 0; l < 4; l++)
                        //mpz_set_str(shares[k][l], tokens[l].c_str(), 10);
                        shares[k][l] = tokens[l];
                }
                result = ss->reconstructSecret(shares, 4);
                for (int k = 0; k < 4; k++) {
                    if (k == 1) {
                        // deal with negative results
                        ss->flipNegative(result[1]);
                    }
                }
                // p (result[1]): Exponent part.
                // v (result[0]): Mantissa.
                // z (result[2]): Indicator for special cases.
                // s (result[3]): Sign indicator.
                if (result[2] != 0){
                    element = result[0] * pow(2, result[1]);
                    if (result[3] == 1)
                        element = -element;
                }
                if (j == 0) {
                    if (size1 == 0)
                        outputFiles[0] << name + "=";
                    else {
                        stringstream s1;
                        s1 << i;
                        outputFiles[0] << name + "[" + s1.str() + "]=";
                    }
                }
                std::ostringstream ss;
                ss << element;
                writeToOutputFile(outputFiles[0], ss.str(), "", 1, j, size2);
            }
        }
    } else {
        std::cout << "Wrong type has been detected";
        std::exit(1);
    }
}


/*
The function produceInputs() read the data from inputFiles, processes it based on the specified 
data type and secrecy level, and writes the results to the outputFiles. The data can be either 
integers or floating-point numbers. The process includes generating shares of the data using 
(ss->getShares()). Important variables used in this function are: 

1. long long element; Represents an individual element of the input data -> this gets passed to getShares().
2. long long *elements; Represents an array of elements used when processing floating-point numbers -> this 
   gets passed to convertFloat(). 
3. std::vector<std::string> shares(numOfComputeNodes); Represents the shares obtained through 
   secret sharing for a given element -> this stores the shares returned from getShares() fucntion.     
*/

void produceInputs(std::ifstream inputFiles[], std::ofstream outputFiles[], std::string name, std::string type, int size1, int size2, int secrecy, int len_sig, int len_exp) {
    std::string line;
    std::vector<std::string> tokens;
    std::vector<std::string> temp;
    long long element;
    std::vector<std::string> shares(numOfComputeNodes);
    int dim = (size1 == 0) ? 1 : size1;
    
    // works for both one or two dimensional arrays
    if (!type.compare("int")) {
        for (int i = 0; i < dim; i++) {
            std::getline(inputFiles[0], line);
            temp = splitfunc(line.c_str(), "=");
            tokens = splitfunc(temp[1].c_str(), ",");

            for (int j = 0; j < tokens.size(); j++) {
                // The str tokens[j] is converted to long long, using base 10. 
                // (nullptr in here is not relevant to our computatuon, this version of stoll 
                // to make sure the conversion uses base 10.)
                element = std::stoll(tokens[j], nullptr, BASE); 
                if (secrecy == 1)
                    shares = ss->getShares(element);
                for (int k = 0; k < numOfComputeNodes; k++) {
                    if (j == 0)
                        outputFiles[k] << name + "=";
                    writeToOutputFile(outputFiles[k], shares[k], tokens[j], secrecy, j, tokens.size());
                }
            }
        }
    } else if (!type.compare("float") && secrecy == 2) {
        for (int i = 0; i < dim; i++) {
            std::getline(inputFiles[0], line);
            temp = splitfunc(line.c_str(), "=");
            tokens = splitfunc(temp[1].c_str(), ",");

            for (int j = 0; j < tokens.size(); j++) {
                for (int k = 0; k < numOfComputeNodes; k++) {
                    if (j == 0)
                        outputFiles[k] << name + "=";
                    writeToOutputFile(outputFiles[k], "", tokens[j], 2, j, tokens.size());
                }
            }
        }

    } else if (!type.compare("float") && secrecy == 1) {
        for (int i = 0; i < dim; i++) {
            std::getline(inputFiles[0], line);
            temp = splitfunc(line.c_str(), "=");
            tokens = splitfunc(temp[1].c_str(), ",");

            for (int j = 0; j < tokens.size(); j++) {
                long long *elements = new long long[4];
                convertFloat((float)atof(tokens[j].c_str()), len_sig, len_exp, &elements);

                for (int m = 0; m < 4; m++) {
                    shares = ss->getShares(elements[m]);
                    for (int k = 0; k < numOfComputeNodes; k++) {
                        if (m == 0)
                            outputFiles[k] << name + "=";
                        writeToOutputFile(outputFiles[k], shares[k], "", 1, m, 4);
                    }
                }
                // Elements array was dynamically allocate memory using new -> free it once done
                delete[] elements; 
            }
        }
    } else {
        std::cout << "Wrong type has been detected";
        std::exit(1);
    }
    // No need to clear the shares vector
}


/*
loadConfig reads the files from var_list file stream. Then it extracts
the data and store it to the appropriate variables as follow: 
1. technique = technique_var
2. bits = bits
3. numOfComputeNodes = peers
4. threshold = threshold
5. numOfInputNodes = inputs
6. numOfOutputNodes = outputs
7. modulus_shamir = modulus_shamir (Conditional - set only if shamir is used)
*/
void loadConfig() {
    std::string line;
    std::vector<std::string> tokens;
    int results[7];
    
    for (int i = 0; i < 7; i++) {
        std::getline(var_list, line);
        tokens = splitfunc(line.c_str(), ":");

        if (i == 0) {
            technique = atoi(tokens[1].c_str());
        } else {
            // Based on the technique used read the last element (shamir, rss))
            if (i == 6 && technique == SHAMIR_SS) {
                mpz_init(modulus_shamir);
                mpz_set_str(modulus_shamir, tokens[1].c_str(), 10);
                // gmp_printf("%Zd\n", modulus_shamir);
            } else {
                results[i] = atoi(tokens[1].c_str());
            }
        }
    }
    bits = results[1];
    numOfComputeNodes = results[2];
    threshold = results[3];
    numOfInputNodes = results[4];
    numOfOutputNodes = results[5];
    
    if (technique == SHAMIR_SS) {
        ss = new ShamirSS(numOfComputeNodes, threshold, modulus_shamir);
    } else if (technique == REPLICATED_SS) {
        if (bits <= 8) { // Bits less than or equal to 8 
            ss = new RSS<uint8_t>(numOfComputeNodes, threshold, bits); //the last argument is supposed to be the ring size, which is stored in the bits field in utility_config
        } else if (bits >= 9 && bits <= 16) { // Between 9 and 16 inclusive 
            ss = new RSS<uint16_t>(numOfComputeNodes, threshold, bits); 
        } else if (bits >= 17 && bits <= 32) { // Between 17 and 32 inclusive 
            ss = new RSS<uint32_t>(numOfComputeNodes, threshold, bits); 
        } else if (bits >= 33 && bits <= 64){ // Between 33 and 64 inclusive 
            ss = new RSS<uint64_t>(numOfComputeNodes, threshold, bits); 
        }
    }
}

/**
 * This fucntion converts floating-point number to a set of integer components based on 
 * the specified parameters K and L. The function does some bit manipulation on the floating-point 
 * representation of the input value to extract the sign bit (s), exponent (e), and mantissa (significand). 
 * 
 * In particular, each floating point number is represented as a 4-tuple (v, p, s, z) where v is
 * an l-bit significand, p is a k-bit exponent, and s and z are sign and
 * zero bits, respectively (2013 CCS paper). 
 * 
 * The integers that will be stored in elements are as follows: 
 * 1. Significand part (significand)
 * 2. Exponent (p)
 * 3. Flag indicating zero (z)
 * 4. Sign (s)
*/
void convertFloat(float value, int K, int L, long long **elements) {
    unsigned int *newptr = (unsigned int *)&value;
    int s = *newptr >> 31;
    int e = *newptr & 0x7f800000;
    e >>= 23;
    int m = 0;
    m = *newptr & 0x007fffff;

    int z;
    long v, p, k;
    long long significand=0, one=1, two=2, tmp=0, tmpm=0;

    if (e == 0 && m == 0) {
        s = 0;
        z = 1;
        significand = 0;
        p = 0;
    } else {
        z = 0;
        if (L < 8) {
            k = (1 << L) - 1; // Raise two to the power of L using shifting and subtract 1, then store it to k
            if (e - 127 - K + 1 > k) {
                p = k;
                significand = one << K; // Raise one to the power of K and store it to significand
                significand = significand - 1; // Sub 1 
            } else if (e - 127 - K + 1 < -k) {
                p = -k;
                significand = 1; // Set the value of significand to 1
            } else {
                p = e - 127 - K + 1;
                m = m + (1 << 23);
                tmpm = m; // Set the value of tmpm to m
                if (K < 24) {
                    tmp = pow(two, (24 - K)); // Raise two to the power of (24 - K) using shifting and store it to tmp
                    significand = tmpm / tmp; // Perform division of tmpm to tmp and store it to significand
                } else {
                    significand = tmpm << (K - 24); // Raise tmpm to the power of (K - 24) and store it to significand
                }
            }
        } else {
            p = e - 127 - K + 1;
            m = m + (1 << 23);
            tmpm = m; // Set the value of tmpm to m
            if (K < 24) {
                tmp = pow(two, (24 - K)); // Raise two to the power of (24 - K) using shifting and store it to tmp
                significand = tmpm / tmp; // Perform division of tmpm to tmp and store it to significand
            } else {
                significand = tmpm; // Set significand to tmpm
                significand = significand << (K - 24); // Raise significand to the power of (K - 24) and store it to significand
            }
        }
    }

    // Set the significand, p, z, and s value directly to the long long array of elements.
    (*elements)[0] = significand;
    (*elements)[1] = p;
    (*elements)[2] = z;
    (*elements)[3] = s;
}

