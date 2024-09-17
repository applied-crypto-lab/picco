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
#ifndef SHAMIROPS_H_
#define SHAMIROPS_H_

#include "ShamirHeaders.h"
#include <fstream>
#include <iostream>
#include <math.h>
#include <sstream>
#include <string>

void ss_init_set(mpz_t x, mpz_t x_val);
void ss_init_set_ui(mpz_t x, unsigned long x_val);
void ss_init(mpz_t x);
void ss_set(mpz_t x, mpz_t x_val);
void ss_set_ui(mpz_t x, unsigned long x_val);
void ss_clear(mpz_t x);
// void ss_add_ui(mpz_t rop, mpz_t op1, uint op2);
// void ss_sub_ui(mpz_t rop, mpz_t op1, uint op2);
long int ss_get_si(mpz_t x);


void ss_process_operands(mpz_t **a1, mpz_t **b1, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int *len_sig, int *len_exp, int size, SecretShare *ss);
void ss_process_results(mpz_t **result1, int resultlen_sig, int resultlen_exp, int len_sig, int len_exp, int size, int threadID, NodeNetwork net,  SecretShare *ss);

void ss_single_convert_to_private_float(float a, mpz_t **priv_a, int len_sig, int len_exp, SecretShare *ss);
void ss_batch_convert_to_private_float(float *a, mpz_t ***priv_a, int len_sig, int len_exp, int size, SecretShare *ss);

void ss_convert_operator(mpz_t **result, mpz_t **op, int *index_array, int dim, int size, int flag);
void ss_convert_operator(mpz_t **result, mpz_t *op, int *index_array, int dim, int size, int flag);
void ss_convert_operator(mpz_t ***result, mpz_t **op, int *index_array, int dim, int size, int flag);
void ss_convert_operator(mpz_t ***result, mpz_t ***op, int *index_array, int dim, int size, int flag);

void ss_single_fop_comparison(mpz_t result, mpz_t *a, mpz_t *b, int resultlen, int alen_sig, int alen_exp, int blen_sig, int blen_exp, std::string op, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_single_fop_arithmetic(mpz_t *result, mpz_t *a, mpz_t *b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, std::string op, int threadID, NodeNetwork net,  SecretShare *ss);

void ss_batch_fop_comparison(mpz_t *result, mpz_t **a, mpz_t **b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_fop_comparison(mpz_t *result, float *a, mpz_t **b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_fop_comparison(mpz_t *result, mpz_t **a, float *b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_fop_arithmetic(mpz_t **result, mpz_t **a, mpz_t **b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID, NodeNetwork net,  SecretShare *ss);

void ss_set(mpz_t *a, mpz_t *result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_set(mpz_t **a, mpz_t **result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_set(float a, mpz_t *result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_set(mpz_t a, mpz_t result, int alen, int resultlen, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_set(mpz_t *a, mpz_t *result, int alen, int resultlen, int size, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_set(int a, mpz_t result, int alen, int resultlen, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);

void ss_set(mpz_t *a, mpz_t *result, int alen, int resultlen, int size, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);

void ss_priv_eval(mpz_t a, mpz_t b, mpz_t cond, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_priv_eval(mpz_t *a, mpz_t *b, mpz_t cond, int threadID, NodeNetwork net,  SecretShare *ss);

void ss_sub(mpz_t *a, mpz_t *b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);

void ss_sub(int *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_sub(mpz_t *a, int *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_sub(mpz_t **a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_sub(float *a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_sub(mpz_t **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);

void ss_privindex_write(mpz_t index, mpz_t **array, int len_sig, int len_exp, float value, int dim, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_privindex_write(mpz_t index, mpz_t **array, int len_sig, int len_exp, mpz_t *value, int dim, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_privindex_write(mpz_t index, mpz_t ***array, int len_sig, int len_exp, float value, int dim1, int dim2, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_privindex_write(mpz_t index, mpz_t ***array, int len_sig, int len_exp, mpz_t *value, int dim1, int dim2, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);

void ss_privindex_write(mpz_t index, mpz_t **array, int len_sig, int len_exp, mpz_t *value, int dim, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);

void ss_privindex_read(mpz_t *indices, mpz_t **array, mpz_t *results, int dim1, int dim2, int size, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_privindex_read(mpz_t *indices, mpz_t **array, mpz_t **results, int dim, int size, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_privindex_read(mpz_t *indices, mpz_t ***array, mpz_t **results, int dim1, int dim2, int size, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
// void ss_privindex_write(mpz_t *indices, mpz_t *array, int len_sig, int len_exp, int *values, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
// void ss_privindex_write(mpz_t *indices, mpz_t *array, int len_sig, int len_exp, mpz_t *values, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_privindex_write(mpz_t *indices, mpz_t **array, int len_sig, int len_exp, int *values, int dim1, int dim2, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_privindex_write(mpz_t *indices, mpz_t **array, int len_sig, int len_exp, mpz_t *values, int dim1, int dim2, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_privindex_write(mpz_t *indices, mpz_t **array, int len_sig, int len_exp, float *values, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_privindex_write(mpz_t *indices, mpz_t **array, int len_sig, int len_exp, mpz_t **values, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_privindex_write(mpz_t *indices, mpz_t ***array, int len_sig, int len_exp, float *values, int dim1, int dim2, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_privindex_write(mpz_t *indices, mpz_t ***array, int len_sig, int len_exp, mpz_t **values, int dim1, int dim2, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);

void ss_int2fl(int value, mpz_t *result, int gamma, int K, int L, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_int2fl(int *value, mpz_t **result, int size, int gamma, int K, int L, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_int2fl(mpz_t value, mpz_t *result, int gamma, int K, int L, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_int2fl(mpz_t *value, mpz_t **result, int size, int gamma, int K, int L, int threadID, NodeNetwork net,  SecretShare *ss);


void ss_int2int(int value, mpz_t result, int gamma1, int gamma2, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_int2int(int *value, mpz_t *result, int size, int gamma1, int gamma2, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_int2int(mpz_t value, mpz_t result, int gamma1, int gamma2, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_int2int(mpz_t *value, mpz_t *result, int size, int gamma1, int gamma2, int threadID, NodeNetwork net,  SecretShare *ss);


void ss_fl2int(float value, mpz_t result, int K, int L, int gamma, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_fl2int(float *value, mpz_t *result, int size, int K, int L, int gamma, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_fl2int(mpz_t *value, mpz_t result, int K, int L, int gamma, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_fl2int(mpz_t **value, mpz_t *result, int size, int K, int L, int gamma, int threadID, NodeNetwork net,  SecretShare *ss);


void ss_fl2fl(float value, mpz_t *result, int K1, int L1, int K2, int L2, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_fl2fl(float *value, mpz_t **result, int size, int K1, int L1, int K2, int L2, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_fl2fl(mpz_t *value, mpz_t *result, int K1, int L1, int K2, int L2, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_fl2fl(mpz_t **value, mpz_t **result, int size, int K1, int L1, int K2, int L2, int threadID, NodeNetwork net,  SecretShare *ss);


void ss_batch_handle_priv_cond(mpz_t *result, mpz_t *result_org, mpz_t out_cond, mpz_t *priv_cond, int counter, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_BOP_float_arithmetic(mpz_t **result, mpz_t **a, mpz_t **b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);

void ss_batch_BOP_int(mpz_t *result, mpz_t *a, mpz_t *b, int resultlen, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);

void ss_shr(mpz_t *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_shl(mpz_t *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);

void ss_batch_BOP_float_comparison(mpz_t *result, mpz_t **a, mpz_t **b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss);

void ss_batch(mpz_t *a, mpz_t *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(int a, mpz_t *b, mpz_t *result, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t *a, mpz_t *b, mpz_t *result, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(int *a, int *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(int *a, int *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(int *a, mpz_t *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(int *a, mpz_t *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t *a, int *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t *a, int *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(int *a, mpz_t **b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(int *a, mpz_t **b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t **a, int *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t **a, int *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t *a, mpz_t **b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t **a, mpz_t *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t **a, mpz_t **b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t *a, mpz_t *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t *a, mpz_t **b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t **a, mpz_t *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch_dot(mpz_t **a, mpz_t **b, int size, int array_size, int *index_array, mpz_t *result, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch(mpz_t **a, mpz_t **b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(float *a, mpz_t **b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t **a, float *b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(float *a, mpz_t **b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t **a, float *b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(float *a, float *b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(int *a, int *b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(float *a, float *b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(int *a, int *b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(float *a, mpz_t ***b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t ***a, float *b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(float *a, mpz_t ***b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t ***a, float *b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(float *a, mpz_t **b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t **a, float *b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(float *a, mpz_t ***b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t ***a, float *b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t **a, mpz_t **b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t **a, mpz_t ***b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t ***a, mpz_t **b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t ***a, mpz_t ***b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t ***a, mpz_t **b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t **a, mpz_t ***b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t **a, mpz_t **b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t ***a, mpz_t ***b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t ***a, mpz_t ***b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t **a, mpz_t ***b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t ***a, mpz_t **b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch(mpz_t **a, mpz_t **b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net,  SecretShare *ss, int *index_array_flags);
void ss_batch_int2fl(int *a, mpz_t **result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_int2fl(mpz_t *a, mpz_t **result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_int2fl(int *a, mpz_t ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_int2fl(mpz_t *a, mpz_t ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_int2fl(mpz_t **a, mpz_t **result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_int2fl(mpz_t **a, mpz_t ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_fl2int(float *a, mpz_t *result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_fl2int(float *a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_fl2int(mpz_t **a, mpz_t *result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_fl2int(mpz_t **a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_fl2int(mpz_t ***a, mpz_t *result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_fl2int(mpz_t ***a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_int2int(int *a, mpz_t *result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_int2int(int *a, mpz_t **result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_int2int(mpz_t *a, mpz_t *result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_int2int(mpz_t **a, mpz_t *result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_int2int(mpz_t *a, mpz_t **result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_int2int(mpz_t **a, mpz_t **result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_fl2fl(float *a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_fl2fl(float *a, mpz_t ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_fl2fl(mpz_t **a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_fl2fl(mpz_t **a, mpz_t ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_fl2fl(mpz_t ***a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void ss_batch_fl2fl(mpz_t ***a, mpz_t ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net,  SecretShare *ss);

#endif
