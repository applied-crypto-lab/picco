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
#ifndef PRIVINDEX_SHAMIR_H_
#define PRIVINDEX_SHAMIR_H_

#include "BitDec.h"
#include "EQZ.h"
#include "Mult.h"
#include "Open.h"
#include "Operation.h"
#include "Random.h"
#include <sys/time.h>

class PrivIndex : public Operation {

public:
    PrivIndex(NodeNetwork nodeNet, std::map<std::string, std::vector<int>> poly, int nodeID, SecretShare *s);
    virtual ~PrivIndex();
    void compute_private_conditions(mpz_t *, mpz_t, mpz_t *, int, int);
    void doOperationRead(mpz_t *index, mpz_t *array, mpz_t *result, int dim, int size, int threadID, int type);
    void doOperationWrite(mpz_t *index, mpz_t *array, mpz_t *value, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, int threadID, int type);
    void AllOr(mpz_t **array, int begin, int size, mpz_t **result, int batch_size, int threadID);
    double time_diff(struct timeval *, struct timeval *);

    void doOperation_int(mpz_t index, mpz_t *array, mpz_t result, int dim, int type, int threadID);
    void doOperation_int_arr(mpz_t index, mpz_t **array, mpz_t result, int dim1, int dim2, int type, int threadID);
    void doOperation_float_arr(mpz_t index, mpz_t ***array, mpz_t *result, int dim1, int dim2, int type, int threadID);
    void doOperation_float(mpz_t index, mpz_t **array, mpz_t *result, int dim, int type, int threadID);

    void doOperationWrite(mpz_t *index, mpz_t *array, int *value, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, int threadID, int type);

    void doOperationWrite_2d(mpz_t *index, mpz_t **array, int *values, int dim1, int dim2, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, int threadID, int type);
    void doOperationWrite_2d(mpz_t *index, mpz_t **array, mpz_t *values, int dim1, int dim2, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, int threadID, int type);


    // void doOperationWrite_int(mpz_t *index, mpz_t *array, int value, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, int threadID, int type);

    // void doOperationWrite_int_arr(mpz_t *index, mpz_t **array, mpz_t *value, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, int threadID, int type);

    // void doOperationWrite_float(mpz_t *index, mpz_t *array, mpz_t *value, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, int threadID, int type);
    // void doOperationWrite_float_arr(mpz_t *index, mpz_t ***array, mpz_t *value, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, int threadID, int type);



private:
    // Mult *Mul;
    BitDec *Bd;
    // EQZ *Eq;
    Random *Rand;
};

#endif /* PRIVINDEX_SHAMIR_H_ */
