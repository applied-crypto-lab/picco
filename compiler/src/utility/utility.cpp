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

#include "../../../common/shared.h"
#include "SecretShare.h"
#include <cmath>
#include <fstream>
#include <gmp.h>
#include <iostream>
#include <math.h>
#include <sstream>
#include <string.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <unordered_set>

#ifdef _WIN32
    #include <direct.h>
#else
    #include <sys/stat.h>
#endif

using namespace std;

// these will be eventually read from the config files;
int numOfComputeNodes;
int numOfInputNodes;
int numOfOutputNodes;
int party;
int bits;
int threshold;

mpz_t modulus_shamir; // Global modulus_shamir variable
int technique;        // Global technique variable

std::ifstream var_list;

SecretShare *ss;

void loadConfig();
void produceOutputs(std::ifstream[], std::ofstream[], std::string, std::string, int, int, int);
void produceInputs(std::ifstream[], std::ofstream[], std::string, std::string, int, int, int, int, int);
void openInputOutputFiles(std::string, std::string, std::ifstream *, std::ofstream *, int);
void readVarList(std::ifstream &, std::ifstream[], std::ofstream[], int);
void writeToOutputFile(std::ofstream &, std::string, std::string, int, int, int);
void convertFloat(float value, int K, int L, long long **elements);
bool createDirectory(const std::string& path);
void pathCreator(const std::string& file_name);

int main(int argc, char **argv) {
    try {
        if (argc != 6) {
            fprintf(stderr, "Incorrect input parameters:\n");
            fprintf(stderr, "Usage: picco-utility -I/O <input/output party ID> <input/output filename> <utility-config> <share/result>\n");
            std::exit(1);
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
        // Checks is the utility-conf opens or not 
        if (!var_list) {
            throw std::runtime_error("Variable list file " + std::string(argv[4]) + " cannot be opened. Check your file path!");
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
        try {
            pathCreator(argv[5]);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
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

    } catch (const std::exception &e) {
        std::cerr << "Error: [utility.cpp, main] " << e.what() << "\nExiting...\n";
        exit(1);
    }
}

void readVarList(std::ifstream &var_list, std::ifstream inputFiles[], std::ofstream outputFiles[], int mode) {
    try {
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
    } catch (const std::exception& e) {
        std::cerr << "[utility.cpp, readVarList] " << e.what() << std::endl;
        exit(1);
    }
}

void openInputOutputFiles(std::string file, std::string output, std::ifstream inputFiles[], std::ofstream outputFiles[], int mode) {
    if (mode == 0) {
        inputFiles[0].open(file.c_str(), std::ifstream::in);
        if (!inputFiles[0]) {
            std::cout << "input file " << file << " could not be opened" << std::endl;
            std::exit(1);
        }
        for (int i = 1; i <= numOfComputeNodes; i++) {
            stringstream s;
            s << i;
            std::string outfile = output + s.str();
            outputFiles[i - 1].open(outfile.c_str(), std::ofstream::out);
            if (!outputFiles[i - 1]) {
                std::cout << "output file " << outfile << " could not be opened" << std::endl;
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
                std::cout << "input file " << infile << " could not be opened" << std::endl;
                std::exit(1);
            }
        }
        outputFiles[0].open(output.c_str(), std::ofstream::out);
        if (!outputFiles[0]) {
            std::cout << "output file " << output << " could not be opened" << std::endl;
            std::exit(1);
        }
    }
}

void writeToOutputFile(std::ofstream &outputFile, std::string value, std::string token, int secrecy, int index, int size) {
    try {
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
    } catch (const std::exception& e) {
        std::cerr << "[utility.cpp, writeToOutputFile] " << e.what() << std::endl;
        exit(1); 
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
    try {
        std::string line;
        std::string value;
        std::vector<std::string> tokens;
        std::vector<std::string> temp;
        double element = 0;
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
                    if (tokens.empty()) {
                        throw std::runtime_error("[Int] Tokens vector is not set correctly. Line# " + std::to_string(k) + " inside [inputFiles]");
                    }
                    if (secrecy == 1)
                        for (int j = 0; j < tokens.size(); j++)
                            shares[k][j] = tokens[j];
                }
                if (secrecy == 1) {
                    try {
                        result = ss->reconstructSecret(shares, size2);
                    } catch (const std::runtime_error &e) {
                        throw std::runtime_error("[reconstructSecret, int] Error in reconstructing secret: " + std::string(e.what()));
                    }
                }

                for (int j = 0; j < tokens.size(); j++) {
                    // for single variable or one-dimension
                    if (size1 == 0 && j == 0)
                        outputFiles[0] << name + "=";
                    // for two-dimension
                    else if (size1 != 0 && j == 0) {
                        std::stringstream s;
                        s << i;
                        outputFiles[0] << name + "[" + s.str() + "]=";
                    }
                    if (secrecy == 1) {
                        // deal with negative results
                        ss->flipNegative(result[j]);
                        value = std::to_string(result[j]); // The reconstructSecret returns long long so we need to convert it to str before outputting
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
                    if (tokens.empty()) {
                        throw std::runtime_error("[Public Float] Tokens vector is not set correctly. Line# " + std::to_string(k) + " inside [inputFiles]");
                    }
                }
                for (int j = 0; j < tokens.size(); j++) {
                    if (size1 == 0 && j == 0)
                        outputFiles[0] << name + "=";
                    else if (size1 != 0 && j == 0) {
                        std::stringstream s1, s2;
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
            shares.resize(numOfComputeNodes, std::vector<std::string>(4)); // Resize the vector of vectors
            std::vector<long long> result(4);

            for (int i = 0; i < dim; i++) {
                for (int j = 0; j < size2; j++) {
                    for (int k = 0; k < numOfComputeNodes; k++) {
                        std::getline(inputFiles[k], line);
                        tokens = splitfunc(line.c_str(), ",");
                        if (tokens.empty()) {
                            throw std::runtime_error("[Private Float] Tokens vector is not set correctly. Line# " + std::to_string(k) + " inside [inputFiles]");
                        }
                        for (int l = 0; l < 4; l++)
                            shares[k][l] = tokens[l];
                    }
                    if (secrecy == 1) {
                        try {
                            result = ss->reconstructSecret(shares, 4);
                        } catch (const std::runtime_error &e) {
                            throw std::runtime_error("[reconstructSecret, private float] Error in reconstructing secret for [private float]: " + std::string(e.what()));
                        }
                    }
                    for (int k = 0; k < 4; k++) {
                        if (k == 1) {
                            // deal with negative results
                            ss->flipNegative(result[1]);
                        }
                    }
                    // p (result[1]): Exponent part.
                    // v (result[0]): Mantissa.
                    // z (result[2]): Indicator for special cases. if set, val=0
                    // s (result[3]): Sign indicator.
                    if (result[2] != 1) {
                        element = result[0] * pow(2, result[1]);
                        if (result[3] == 1)
                            element = -element;
                    }
                    if (j == 0) {
                        if (size1 == 0)
                            outputFiles[0] << name + "=";
                        else {
                            std::stringstream s1;
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
            throw std::runtime_error("Wrong type has been detected");
        }
    } catch (const std::runtime_error &e) {
        std::cerr << "[utility.cpp, produceOutputs] " << e.what() << "\nExiting...\n";
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
        try {
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
                tokens = splitfunc(temp[1].c_str(), ","); // not stripping whitespace from the values we are reading
                if (tokens.empty()) {
                    throw std::runtime_error("[Int] Tokens vector is not set correctly. Line# " + std::to_string(0) + " inside [inputFiles]");
                }
                for (int j = 0; j < tokens.size(); j++) {
                    // The str tokens[j] is converted to long long, using base 10.
                    // (nullptr in here is not relevant to our computation, this version of stoll
                    // to make sure the conversion uses base 10.)
                    try {
                        element = std::stoll(tokens[j], nullptr, BASE);
                    } catch (const std::invalid_argument& e) {
                        throw std::runtime_error("[Int] Error converting element to long long: " + std::string(e.what()));
                    } catch (const std::out_of_range& e) {
                        throw std::runtime_error("[Int] Error converting element to long long (out of range): " + std::string(e.what()));
                    }
                    if (secrecy == 1) {
                        try {
                            shares = ss->getShares(element);
                        } catch (const std::runtime_error& e) {
                            throw std::runtime_error("[getShares, Int] Error in getting shares: " + std::string(e.what()));
                        }
                    }
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
                if (tokens.empty()) {
                    throw std::runtime_error("[Private Float] Tokens vector is not set correctly. Line# " + std::to_string(0) + " inside [inputFiles]");
                }
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
                if (tokens.empty()) {
                    throw std::runtime_error("[Public Float] Tokens vector is not set correctly. Line# " + std::to_string(0) + " inside [inputFiles]");
                }
                for (int j = 0; j < tokens.size(); j++) {
                    long long *elements = new long long[4];
                    convertFloat((float)atof(tokens[j].c_str()), len_sig, len_exp, &elements);

                    for (int m = 0; m < 4; m++) {
                        try {
                            shares = ss->getShares(elements[m]);
                        } catch (const std::runtime_error& e) {
                            throw std::runtime_error("[getShares, Public Float] Error in getting shares: " + std::string(e.what()));
                        }
                        for (int k = 0; k < numOfComputeNodes; k++) {
                            if (m == 0)
                                outputFiles[k] << name + "=";
                            writeToOutputFile(outputFiles[k], shares[k], "", 1, m, 4);
                        }
                    }
                    // Elements array was dynamically allocated memory using new -> free it once done
                    delete[] elements;
                }
            }
        } else {
            throw std::runtime_error("Wrong type has been detected");
        }
        // No need to clear the shares vector
    } catch (const std::runtime_error &e) {
        std::cerr << "[utility.cpp, produceInputs] " << e.what() << "\nExiting...\n";
        std::exit(1);
    }
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
    std::unordered_set<std::string> variableSet; // Store the name of variable to check for duplicate variables
    std::unordered_set<std::string> validVariables = {"technique", "bits", "peers", "threshold", "inputs", "outputs"}; // Valid variable names
    int results[6]; // 6 params for both shamir and RSS
    int peer_value = 1;
    bool techniqueEncountered = false; // Flag to track if 'technique' has been encountered
    std::string::size_type pos; // The optional variable sent to stoi to check malformed input
    int integer_value_token; // Variable to help check tokens

    for (int i = 0; i < 6; i++) {
        std::getline(var_list, line);
        tokens = splitfunc(line.c_str(), ":");

        // Check if the token is an int or not before conversions 
        /* Logic Used: The pos parameter in the std::stoi function call returns the index within the input string where parsing stopped. 
        It's an optional output parameter used to indicate the position where the conversion stopped due to encountering a non-numeric character. 
        - If the entire string can be converted to a valid integer without encountering any non-numeric characters, pos will be set to tokens[1].size(). 
        This indicates that the entire string has been successfully parsed.
        - If parsing stops before reaching the end of the string due to encountering a non-numeric character, pos will be set to the index of that character.
        - This was used since the catch was not throwing an error if the given value was a number following with a character. This is due to stoi!
        */
        try {
            integer_value_token = std::stoi(tokens[1], &pos);
            if (pos != tokens[1].size()) {
                throw std::runtime_error("[loadConfig, Reading var_list] Invalid value for " + tokens[0] + ": '" + tokens[1] + "'. Conversion from string to integer failed.");
            }
        } catch (const std::invalid_argument& e) {
            throw std::runtime_error("[loadConfig, Reading var_list] Invalid value for " + tokens[0] + ": '" + tokens[1] + "'. Conversion from string to integer failed.");
        }

        // Validate line format // If either key/value is missing or empty
        if (tokens.size() != 2 || tokens[0].empty() || tokens[1].empty()) {
            throw std::runtime_error("[loadConfig, Reading var_list] Invalid configuration format: each line must be in the form 'var:value'");
        }

        // Check if variable name is misspelled
        if (validVariables.find(tokens[0]) == validVariables.end()) {
            throw std::runtime_error("[loadConfig, Reading var_list] Unknown variable found: " + tokens[0]);
        }
        
        // Check for duplicate variable
        /* Logic used: 
            - insert each variable name token[0] to the set
            - Use .second to check if the insertion took place or not
            - If false -> means the name was already in the set 
            - Throw an exception saying a duplicated value was encountered 
        */
        if (!variableSet.insert(tokens[0]).second) {
            throw std::runtime_error("[loadConfig, Reading var_list] Duplicate variable found: " + tokens[0] + "\n");
        }

        // Check to make sure the first value is always technique
        if (!techniqueEncountered) {
            if (tokens[0] != "technique") {
                throw std::runtime_error("[loadConfig, Reading var_list] 'technique' must be the first variable encountered.");
            }
            techniqueEncountered = true; // Set the flag to true after encountering 'technique'
        }

        // Check if the technique is shamir or rss
        if (tokens[0] == "technique") {
            if (integer_value_token != 2 && integer_value_token != 1) {
                throw std::runtime_error("[loadConfig, Reading var_list] Invalid value for technique: '" + tokens[1] + "'. Must use single integer '2' for SHAMIR_SS or '1' for REPLICATED_SS!\n");
            }
        }
        
        // Check if values are all positive and non-zero
        if (tokens[0] == "bits" || tokens[0] == "peers" || tokens[0] == "threshold" || tokens[0] == "inputs" || tokens[0] == "outputs") {
            if (integer_value_token <= 0) {
                throw std::runtime_error("[loadConfig, Reading var_list] Invalid value for " + tokens[0] + ": must be a non-zero and positive integer\n");
            }
        }

        // Check if peers is set correctly
        if (tokens[0] == "peers") {
            if (integer_value_token % 2 == 0) {
                throw std::runtime_error("[loadConfig, Reading var_list] Invalid value for peers: must be an odd integer and equal to 2 * threshold + 1\n");
            } else {
                peer_value = integer_value_token;
            }
        }

        // Check if threshold is set correctly
        if (tokens[0] == "threshold") {
            if (integer_value_token != ((peer_value-1) / 2)) {
                throw std::runtime_error("[loadConfig, Reading var_list] Invalid value for threshold: must be consistant to this formula 'peers=2*threshold+1' The given peer is: " +  std::to_string(peer_value) + ". The threshold should be set accordingly!");
            } 
        }
        // Assign integerValue to results[i]
        results[i] = integer_value_token;
    }

    // Based on the technique used read the last element (shamir, rss))
    technique = results[0];
    bits = results[1];
    numOfComputeNodes = results[2];
    threshold = results[3];
    numOfInputNodes = results[4];
    numOfOutputNodes = results[5];

    // only if technique is SHAMIR, read the next line to get the modulus
    // this was causing a problem with utility for RSS in produceInputs
    if (technique == SHAMIR_SS) {
        // if RSS, this starts to read lines with info about inputs, hence why it's only called if technique=shamir
        std::getline(var_list, line); 
        tokens = splitfunc(line.c_str(), ":");
        if (tokens.size() != 2 || tokens[0] != "modulus" || tokens[1].empty()) {
            throw std::runtime_error("[loadConfig, Reading var_list] Invalid configuration format: missing or invalid modulus variable/value");
        }
        std::stringstream ss(tokens[1]);
        signed long long int mod_value;
        ss >> mod_value;
        if (mod_value <= 0) {
            throw std::runtime_error("[loadConfig, Reading var_list] Modulus should be a positive integer!");
        }
        mpz_init(modulus_shamir);
        mpz_set_str(modulus_shamir, tokens[1].c_str(), 10);
    }

    if (technique == SHAMIR_SS) {
        ss = new ShamirSS(numOfComputeNodes, threshold, modulus_shamir);
    } else if (technique == REPLICATED_SS) {
        if (bits <= 8) {                                               // Bits less than or equal to 8
            ss = new RSS<uint8_t>(numOfComputeNodes, threshold, bits); // the last argument is supposed to be the ring size, which is stored in the bits field in utility_config
        } else if (bits >= 9 && bits <= 16) {                          // Between 9 and 16 inclusive
            ss = new RSS<uint16_t>(numOfComputeNodes, threshold, bits);
        } else if (bits >= 17 && bits <= 32) { // Between 17 and 32 inclusive
            ss = new RSS<uint32_t>(numOfComputeNodes, threshold, bits);
        } else if (bits >= 33 && bits <= 64) { // Between 33 and 64 inclusive
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
    try {
        unsigned int *newptr = (unsigned int *)&value;
        int s = *newptr >> 31;
        int e = *newptr & 0x7f800000;
        e >>= 23;
        int m = 0;
        m = *newptr & 0x007fffff;

        int z;
        long v, p, k;
        long long significand = 0, one = 1, two = 2, tmp = 0, tmpm = 0;

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
                    significand = one << K;        // Raise one to the power of K and store it to significand
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
                    significand = tmpm;                    // Set significand to tmpm
                    significand = significand << (K - 24); // Raise significand to the power of (K - 24) and store it to significand
                }
            }
        }

        // printf("sig  %lli\n", significand);
        // printf("p    %li\n", p);
        // printf("z    %i\n", z);
        // printf("sgn  %i\n", s);
        // Set the significand, p, z, and s value directly to the long long array of elements.
        (*elements)[0] = significand;
        (*elements)[1] = p;
        (*elements)[2] = z;
        (*elements)[3] = s;
    } catch (const std::exception &e) {
        std::cerr << "[utility.cpp, convertFlost] " << e.what() << "\nExiting...\n";
        std::exit(1);
    }
}


bool createDirectory(const std::string& path) {
#ifdef _WIN32
    return _mkdir(path.c_str()) == 0;
#else
    return mkdir(path.c_str(), 0777) == 0;
#endif
}

// Check and create the directory 
void pathCreator(const std::string& file_name) {
    try {
        size_t path_separator_pos = file_name.find('/');
        while (path_separator_pos != std::string::npos) {
            std::string directory = file_name.substr(0, path_separator_pos);
            createDirectory(directory);
            path_separator_pos = file_name.find('/', path_separator_pos + 1);
        }
    } catch (const std::exception& e) {
        std::cerr << "[utility.cpp, pathCreator] creating the paths for " << file_name << e.what() << "\nExiting...\n";
        std::exit(1);
    }
}
