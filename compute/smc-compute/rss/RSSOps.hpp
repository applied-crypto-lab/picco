/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2024 PICCO Team
   ** Department of Computer Science and Engineering, University at Buffalo (SUNY)

   PICCO is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   PICCO is distributed in the hope that it will be useful,
   but WITHOUT *ANY WARRANTY{
      return;
   } without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with PICCO. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef _RSSOPS_HPP_
#define _RSSOPS_HPP_

#include "NodeNetwork.h"
#include "RepSecretShare.hpp"
#include <fstream>
#include <iostream>
#include <math.h>
#include <sstream>
#include <string>

template <typename T>
void ss_init_set(T *x, T *x_val) {
    return;
}

template <typename T>
void ss_init(T *x) {
    return;
}

template <typename T, typename U>
void ss_init(T x, U size) {
    x = (T)malloc(sizeof(std::remove_pointer_t<T>) * (size));
}

template <typename T>
void ss_set(T *x, T *x_val) {
    return;
}

template <typename T>
void ss_init_set_ui(T x, uint x_val) {
    return;
}

template <typename T>
int ss_get_si(T *x) {
    return 0;
}

template <typename T>
void ss_clear(T x) {
    free(x);
}

template <typename T, typename U>
void ss_clear(T *x, U size) {
    return;
}

// template <typename T>
// void ss_add_ui(T **rop, T **op1, uint op2) {
//     return;
// }

// template <typename T>
// void ss_sub_ui(T **rop, T **op1, uint op2) {
//     return;
// }

template <typename T>
void ss_process_operands(T ***a1, T ***b1, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int *len_sig, int *len_exp, int size, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_process_results(T ***result1, int resultlen_sig, int resultlen_exp, int len_sig, int len_exp, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_single_convert_to_private_float(float a, T ***priv_a, int len_sig, int len_exp, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_convert_to_private_float(float *a, T ****priv_a, int len_sig, int len_exp, int size, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_convert_operator(T ***result, T ***op, int *index_array, int dim, int size, int flag) {
    return;
}

template <typename T>
void ss_convert_operator(T ***result, T **op, int *index_array, int dim, int size, int flag) {
    return;
}

template <typename T>
void ss_convert_operator(T ****result, T ***op, int *index_array, int dim, int size, int flag) {
    return;
}

template <typename T>
void ss_convert_operator(T ****result, T ****op, int *index_array, int dim, int size, int flag) {
    return;
}

template <typename T>
void ss_single_fop_comparison(T *result, T **a, T **b, int resultlen, int alen_sig, int alen_exp, int blen_sig, int blen_exp, std::string op, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_single_fop_arithmetic(T **result, T **a, T **b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, std::string op, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_fop_comparison(T **result, float *a, T ***b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}
template <typename T>
void ss_batch_fop_comparison(T **result, T ***a, float *b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_fop_comparison(T **result, T ***a, T ***b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_fop_arithmetic(T ***result, T ***a, T ***b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_set(T **a, T **result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_set(T ***a, T ***result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_set(float a, T **result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_set(T *a, T *result, int alen, int resultlen, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_set(T *a, T *result, int alen, int resultlen, int size, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    for (size_t s = 0; s < ss->numShares; s++) {
        memcpy(result[s], a[s], sizeof(T) * size);
    }
}

template <typename T>
void ss_set(T **a, T **result, int alen, int resultlen, int size, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_set(int a, T *result, int alen, int resultlen, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_priv_eval(T *a, T *b, T *cond, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_priv_eval(T **a, T **b, T *cond, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_sub(T **a, T **b, T **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_sub(int *a, T **b, int alen, int blen, T **result, int resultlen, int size, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_sub(T **a, int *b, int alen, int blen, T **result, int resultlen, int size, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_sub(T ***a, T ***b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, T ***result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_sub(float *a, T ***b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, T ***result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_sub(T ***a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, T ***result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_privindex_write(T *index, T ***array, int len_sig, int len_exp, float value, int dim, T *out_cond, T **priv_cond, int counter, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_privindex_write(T *index, T ***array, int len_sig, int len_exp, T **value, int dim, T *out_cond, T **priv_cond, int counter, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_privindex_write(T *index, T ****array, int len_sig, int len_exp, float value, int dim1, int dim2, T *out_cond, T **priv_cond, int counter, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_privindex_write(T *index, T ****array, int len_sig, int len_exp, T **value, int dim1, int dim2, T *out_cond, T **priv_cond, int counter, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_privindex_read(T **indices, T ***array, T **results, int dim1, int dim2, int size, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_privindex_read(T **indices, T ***array, T ***results, int dim, int size, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_privindex_read(T **indices, T ****array, T ***results, int dim1, int dim2, int size, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T> //
void ss_privindex_write(T **indices, T **array, int len_sig, int len_exp, int *values, int dim, int size, T *out_cond, T **priv_cond, int counter, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T> //
void ss_privindex_write(T **indices, T **array, int len_sig, int len_exp, T **values, int dim, int size, T *out_cond, T **priv_cond, int counter, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_privindex_write(T **indices, T ***array, int len_sig, int len_exp, int *values, int dim1, int dim2, int size, T *out_cond, T **priv_cond, int counter, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_privindex_write(T **indices, T ***array, int len_sig, int len_exp, T **values, int dim1, int dim2, int size, T *out_cond, T **priv_cond, int counter, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_privindex_write(T **indices, T ***array, int len_sig, int len_exp, float *values, int dim, int size, T *out_cond, T **priv_cond, int counter, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_privindex_write(T **indices, T ***array, int len_sig, int len_exp, T ***values, int dim, int size, T *out_cond, T **priv_cond, int counter, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_privindex_write(T **indices, T ****array, int len_sig, int len_exp, float *values, int dim1, int dim2, int size, T *out_cond, T **priv_cond, int counter, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_privindex_write(T **indices, T ****array, int len_sig, int len_exp, T ***values, int dim1, int dim2, int size, T *out_cond, T **priv_cond, int counter, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

// template <typename T>
// void ss_int2fl(int value, T **result, int gamma, int K, int L, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
//     return;
// }

template <typename T>
void ss_int2fl(int *value, T ***result, int size, int gamma, int K, int L, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_int2fl(T **value, T ***result, int size, int gamma, int K, int L, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_int2int(int *value, T **result, int size, int gamma1, int gamma2, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_int2int(T **value, T **result, int size, int gamma1, int gamma2, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_fl2int(float *value, T **result, int size, int K, int L, int gamma, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_fl2int(T ***value, T **result, int size, int K, int L, int gamma, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_fl2fl(float *value, T ***result, int size, int K1, int L1, int K2, int L2, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_fl2fl(T ***value, T ***result,int size,  int K1, int L1, int K2, int L2, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_handle_priv_cond(T **result, T **result_org, T *out_cond, T **priv_cond, int counter, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_BOP_float_arithmetic(T ***result, T ***a, T ***b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, T *out_cond, T **priv_cond, int counter, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_BOP_int(T **result, T **a, T **b, int resultlen, int alen, int blen, T *out_cond, T **priv_cond, int counter, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_shr(T **a, T **b, int alen, int blen, T **result, int resultlen, int size, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_shl(T **a, T **b, int alen, int blen, T **result, int resultlen, int size, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_BOP_float_comparison(T **result, T ***a, T ***b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T **a, T **b, T **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(int a, T **b, T **result, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T **a, T **b, T **result, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(int *a, int *b, T **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(int *a, int *b, T ***result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(int *a, T **b, T **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(int *a, T **b, T ***result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T **a, int *b, T **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T **a, int *b, T ***result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(int *a, T ***b, T **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(int *a, T ***b, T ***result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ***a, int *b, T **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ***a, int *b, T ***result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T **a, T ***b, T ***result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ***a, T **b, T ***result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ***a, T ***b, T **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T **a, T **b, T ***result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T **a, T ***b, T **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ***a, T **b, T **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_dot(T ***a, T ***b, int size, int array_size, int *index_array, T **result, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ***a, T ***b, T ***result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(float *a, T ***b, T ****result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ***a, float *b, T ****result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(float *a, T ***b, T ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ***a, float *b, T ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(float *a, float *b, T ****result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(int *a, int *b, T ****result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(float *a, float *b, T ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(int *a, int *b, T ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(float *a, T ****b, T ****result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ****a, float *b, T ****result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(float *a, T ****b, T ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ****a, float *b, T ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(float *a, T ***b, T **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ***a, float *b, T **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(float *a, T ****b, T **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ****a, float *b, T **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ***a, T ***b, T ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ***a, T ****b, T ****result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ****a, T ***b, T ****result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ****a, T ****b, T ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ****a, T ***b, T ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ***a, T ****b, T ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ***a, T ***b, T ****result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ****a, T ****b, T ****result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ****a, T ****b, T **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ***a, T ****b, T **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ****a, T ***b, T **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch(T ***a, T ***b, T **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_int2fl(int *a, T ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_int2fl(T **a, T ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_int2fl(int *a, T ****result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_int2fl(T **a, T ****result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_int2fl(T ***a, T ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_int2fl(T ***a, T ****result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_fl2int(float *a, T **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_fl2int(float *a, T ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_fl2int(T ***a, T **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_fl2int(T ***a, T ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_fl2int(T ****a, T **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_fl2int(T ****a, T ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_int2int(int *a, T **result, int adim, int resultdim, int alen, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_int2int(int *a, T ***result, int adim, int resultdim, int alen, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_int2int(T **a, T **result, int adim, int resultdim, int alen, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_int2int(T ***a, T **result, int adim, int resultdim, int alen, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_int2int(T **a, T ***result, int adim, int resultdim, int alen, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_int2int(T ***a, T ***result, int adim, int resultdim, int alen, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_fl2fl(float *a, T ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_fl2fl(float *a, T ****result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_fl2fl(T ***a, T ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_fl2fl(T ***a, T ****result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_fl2fl(T ****a, T ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_batch_fl2fl(T ****a, T ****result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

#endif
