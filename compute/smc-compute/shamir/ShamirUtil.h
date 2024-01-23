/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2023 PICCO Team
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

#ifndef SHAMIR_UTIL_H_
#define SHAMIR_UTIL_H_

#include "stdint.h"
#include <cstdlib>
#include <gmp.h>
#include <iostream>
#include <math.h>
#include <vector>

int modulo(int a, int b);
long long nChoosek(long long n, long long k);


void ss_batch_free_operator(mpz_t **op, int size);
void ss_batch_free_operator(mpz_t ***op, int size);
int smc_compute_len(int alen, int blen);

void convertFloat(float value, int K, int L, mpz_t **elements);
void convertDouble(double value, int K, int L, mpz_t **elements);

#endif
