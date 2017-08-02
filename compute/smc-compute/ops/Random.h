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


#ifndef RANDOM_H_
#define RANDOM_H_

#include "Operation.h"

class Random : public Operation{
public:
	Random(NodeNetwork nodeNet, std::map<std::string, std::vector<int> > poly, int nodeID, SecretShare *s);
	virtual ~Random();
	int computePolynomials(std::vector<int> polys, int point);
	long long nChoosek(long long n, long long k); 
	void generateRandValue(int nodeID, int bits, int size, mpz_t* results);
	void generateRandValue(int nodeID, int bits, int size, mpz_t* results, int threadID);
	void generateRandValue(int nodeID, mpz_t mod, int size, mpz_t* results);
	void generateRandValue(int nodeID, mpz_t mod, int size, mpz_t* results, int threadID); 
	void PRandM(int K, int M, int size, mpz_t** result);
	void PRandM(int K, int M, int size, mpz_t** result, int threadID);
	void PRandBit(int size, mpz_t* results);
	void PRandBit(int size, mpz_t* results, int threadID);
	void PRandInt(int K, int M, int size, mpz_t* result); 
	void PRandInt(int K, int M, int size, mpz_t* result, int threadID); 
	static gmp_randstate_t* rstates;
	static gmp_randstate_t** rstates_thread; 
	static int isFirst; 
	static int isInitialized; 
	static int* isFirst_thread; 
	static void getNextRandValue(int id, int bits, std::map<std::string, std::vector<int> > poly, mpz_t value);  
	void getNextRandValue(int id, int bits, std::map<std::string, std::vector<int> > poly, mpz_t value, int threadID); 
	static void getNextRandValue(int id, mpz_t mod, std::map<std::string, std::vector<int> > poly, mpz_t value);  
	void getNextRandValue(int id, mpz_t mod, std::map<std::string, std::vector<int> > poly, mpz_t value, int threadID); 
private:
	int numOfThreads;
	static pthread_mutex_t mutex;  
};

#endif /* OPERATION_H_ */
