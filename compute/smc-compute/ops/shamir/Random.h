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

#ifndef RANDOM_SHAMIR_H_
#define RANDOM_SHAMIR_H_

#include "Operation.h"

class Random : public Operation {
public:
    Random(NodeNetwork nodeNet, std::map<std::string, std::vector<int>> poly, int nodeID, SecretShare *s);
    virtual ~Random();
    int computePolynomials(std::vector<int> polys, int point);
    void generateRandValue(int nodeID, int bits, int size, mpz_t *results);
    void generateRandValue(int nodeID, int bits, int size, mpz_t *results, int threadID);
    void generateRandValue(int nodeID, mpz_t mod, int size, mpz_t *results);
    void generateRandValue(int nodeID, mpz_t mod, int size, mpz_t *results, int threadID);
  
    static void getNextRandValue(int id, int bits, std::map<std::string, std::vector<int>> poly, mpz_t value);
    void getNextRandValue(int id, int bits, std::map<std::string, std::vector<int>> poly, mpz_t value, int threadID);
    static void getNextRandValue(int id, mpz_t mod, std::map<std::string, std::vector<int>> poly, mpz_t value);
    void getNextRandValue(int id, mpz_t mod, std::map<std::string, std::vector<int>> poly, mpz_t value, int threadID);


  void PRandM(int K, int M, int size, mpz_t **result);
    void PRandM(int K, int M, int size, mpz_t **result, int threadID);
    void PRandM_two(int K, int M_1, int M_2, int size, mpz_t **result_1, mpz_t **result_2, int threadID);
    void PRandM_two(int K, int M_1, int M_2, int size, mpz_t **result_1, mpz_t **result_2);
    void PRandBit(int size, mpz_t *results);
    void PRandBit(int size, mpz_t *results, int threadID);
    void PRandInt(int K, int M, int size, mpz_t *result);
    void PRandInt(int K, int M, int size, mpz_t *result, int threadID);
private:
    // int numOfThreads;
    // static pthread_mutex_t mutex;
};


#endif /* OPERATION_SHAMIR_H_ */
