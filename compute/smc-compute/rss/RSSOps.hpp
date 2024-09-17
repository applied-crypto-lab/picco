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

template <typename T>
void ss_add_ui(T *rop, T *op1, uint op2) {
    return;
}

template <typename T>
void ss_sub_ui(T *rop, T *op1, uint op2) {
    return;
}

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
void ss_convert_operator(T ***result, T ***op, int *index_array, int dim, int extra_dim, int extra_dimsize, int flag) {
    *result = new T**[size];
    for (int i = 0; i < size; ++i) {
        (*result)[i] = new T*[extra_dim];
        for (int j = 0; j < extra_dim; ++j) {
            (*result)[i][j] = new T;
        }
    }
    int dim1, dim2, dim3;
    for (int j = 0; j < extra_dim; ++j) {
        for (int i = 0; i < size; ++i) {
            if (flag == 1) {
                if (dim != 0) {
                    dim1 = index_array[3 * i] / dim;
                    dim2 = index_array[3 * i] % dim;
                    dim3 = j;  // Accessing the extra dimension
                    *(*result)[i][j] = op[dim1][dim2][dim3];
                } else {
                    *(*result)[i][j] = op[0][index_array[3 * i]][j];
                }
            } else if (flag == 2) {
                if (dim != 0 && dim != -1) {
                    dim1 = index_array[3 * i + 1] / dim;
                    dim2 = index_array[3 * i + 1] % dim;
                    dim3 = j;  // Accessing the extra dimension
                    *(*result)[i][j] = op[dim1][dim2][dim3];
                } else if (dim == 0) {
                    *(*result)[i][j] = op[0][index_array[3 * i + 1]][j];
                }
            } else {
                if (dim != 0) {
                    dim1 = index_array[3 * i + 2] / dim;
                    dim2 = index_array[3 * i + 2] % dim;
                    dim3 = j;  // Accessing the extra dimension
                    *(*result)[i][j] = op[dim1][dim2][dim3];
                }
            }
        }
    }
    return;
}

template <typename T>
void ss_convert_operator(T ***result, T **op, int *index_array, int dim, int extra_dim, int size, int flag) {
    T ***ops = new T**[1];
    **ops = op;
    ss_convert_operator(result, ops, index_array, dim, extra_dim, size, flag);
    for (int i = 0; i < extra_dim; i++) {
            free(ops[i]);
        }
    free(ops);
    return;
}

template <typename T>
void ss_convert_operator(T ****result, T ***op, int *index_array, int dim, int extra_dim, int size, int flag) {
    T ***ops = NULL;
    if (op != NULL) {
        ops = new T**[1];
        **ops = op;
    }
    ss_convert_operator(result, ops, index_array, dim, extra_dim, size, flag);
    if (op != NULL) {
        for (int i = 0; i < extra_dim; i++) {
            free(ops[i]);
        }
        free(ops);
    }
    return;
}

template <typename T>
void ss_convert_operator(T ****result, T ****op, int *index_array, int dim, int size, int flag) {
    ***result = new T***[extra_dim][size][4]; 
    int dim1, dim2;
    for (int k = 0; k < extra_dim; ++k) {
        for (int i = 0; i < size; ++i) {
            (*result)[i] = new T*[4]; 
            for (int j = 0; j < 4; ++j) {
                (*result)[i][j] = new T[extra_dim];
                if (flag == 1) {
                    if (dim != 0) {
                        dim1 = index_array[3 * i] / dim;
                        dim2 = index_array[3 * i] % dim;
                        (*result)[k][i][j] = op[dim1][dim2][j];
                    } else {
                        (*result)[k][i][j] = op[0][i][j];
                    }
                } else if (flag == 2) {
                    if (op != nullptr && dim != -1) {
                        if (dim != 0) {
                            dim1 = index_array[3 * i + 1] / dim;
                            dim2 = index_array[3 * i + 1] % dim;
                            (*result)[k][i][j] = op[dim1][dim2][j];
                        } else {
                            (*result)[k][i][j] = op[0][i][j];
                        }
                    }
                } else {
                    if (dim != 0) {
                        dim1 = index_array[3 * i + 2] / dim;
                        dim2 = index_array[3 * i + 2] % dim;
                        (*result)[k][i][j] = op[dim1][dim2][j];
                    }
                }
            }
        }
    }
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

template <typename T>
void ss_int2fl(int value, T **result, int gamma, int K, int L, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_int2fl(T *value, T **result, int gamma, int K, int L, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_int2int(int value, T *result, int gamma1, int gamma2, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_int2int(T *value, T *result, int gamma1, int gamma2, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_fl2int(float value, T *result, int K, int L, int gamma, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_fl2int(T **value, T *result, int K, int L, int gamma, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_fl2fl(float value, T **result, int K1, int L1, int K2, int L2, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

template <typename T>
void ss_fl2fl(T **value, T **result, int K1, int L1, int K2, int L2, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
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
void ss_batch_BOP_int(T **result, T **a, T **b, int resultlen, int alen, int blen, T *out_cond, T **priv_cond, int counter, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss, int extra_dim) {
        
    // Removing any whitespace from the operand (was causing issues for assignment, or "=")
    op.erase(std::remove(op.begin(), op.end(), ' '), op.end());

    T ***result_org = new T**[extra_dim];
    for (int i = 0; i < extra_dim; i++) {
        result_org[i] = new T[size];
        for (int j = 0; j < size; j++) {
            ss_init_set(result_org[i][j], result[i][j]);
        }
    }
    if (op == "*") {
        Mult(result, a, b, size, threadID, net);
    } else if (op == "-") {
        ss->modSub(result, a, b, size);
    } else if (op == "+") {
        ss->modAdd(result, a, b, size);
    } else if (op == "==") {
        ss->doOperation_EQZ(result, a, b, alen, blen, resultlen, size, threadID, net);
    } else if (op == "!=") {
        ss->doOperation_EQZ(result, a, b, alen, blen, resultlen, size, threadID, net);
        ss->modSub(result, 1, result, size);
    } else if (op == ">") {
        ss->doOperation_LT(result, b, a, blen, alen, resultlen, size, threadID, net);
    } else if (op == ">=") {
        ss->doOperation_LT(result, a, b, alen, blen, resultlen, size, threadID, net);
        ss->modSub(result, 1, result, size);
    } else if (op == "<") {
        ss->doOperation_LT(result, a, b, alen, blen, resultlen, size, threadID, net);
    } else if (op == "<=") {
        ss->doOperation_LT(result, b, a, blen, alen, resultlen, size, threadID, net);
        ss->modSub(result, 1, result, size);
    } else if (op == "/") {
        ss->doOperation_IntDiv(result, a, b, resultlen, size, threadID, net);
    } else if (op == "/P") {
        ss->doOperation_IntDiv_Pub(result, a, b, resultlen, size, threadID, net);
    } else if (op == "=") {
        ss->set(a, result, alen, resultlen, size, type, threadID, net);
    } else if (op == ">>") {
        ss->shr(a, b, alen, blen, result, resultlen, size, type, threadID, net);
    } else if (op == "<<") {
        ss->shl(a, b, alen, blen, result, resultlen, size, type, threadID, net);
    } else {
        std::cout << "[ss_batch_BOP_int] Unrecognized op: " << op << "\n";
    }

    ss->batch_handle_priv_cond(result, result_org, out_cond, priv_cond, counter, size, threadID, net);
    ss->batch_free_operator(result_org, size);
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
    
    int extra_dim = 0;
    T ***a_tmp, ***b_tmp, ***result_tmp;

    // Convert operators for extra dimension
    ss_convert_operator(&a_tmp, a, index_array, adim, extra_dim, size, 1);
    ss_convert_operator(&b_tmp, b, index_array, bdim, extra_dim, size, 2);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, extra_dim, size, 3);

    // Perform the batch operation for RSS
    ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, 
                      out_cond, priv_cond, counter, size, op, type, threadID, net, ss, extra_dim);

    // Set the results back to the original array considering the extra dimension
    for (int j = 0; j < extra_dim; ++j) {
        for (int i = 0; i < size; ++i) {
            mpz_set(result[index_array[j][3 * i + 2]], result_tmp[j][i]);
        }
    }

    // Free the temporary arrays
    ss_batch_free_operator(&a_tmp, size, extra_dim);
    ss_batch_free_operator(&b_tmp, size, extra_dim);
    ss_batch_free_operator(&result_tmp, size, extra_dim);
    return;
}

template <typename T>
void ss_batch(int a, T **b, T **result, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    T **a_tmp = new T**[extra_dim][size]; 
    T **out_tmp = new T**[extra_dim][size]; 

    for (int j = 0; j < extra_dim; j++) {
        for (int i = 0; i < size; i++) {
            mpz_init_set_ui(a_tmp[j][i], a);
            mpz_init(out_tmp[j][i]);
        }
    }

    ss->modSub(result, a_tmp, b, size);

    if (out_cond != NULL) {
        for (int j = 0; j < extra_dim; j++) {
            for (int i = 0; i < size; i++)
                mpz_set(out_tmp[j][i], out_cond);
        }
        Mult(result, result, out_tmp, size, threadID, net, ss);
    }

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&out_tmp, size);
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