/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2024 PICCO Team
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

#ifndef _BATCH_HPP_
#define _BATCH_HPP_

#include "../NodeNetwork.h"
#include <algorithm>
#include <string>

// ============================================================================
// Unified Batch Operations
// Uses templates to work with both Shamir (mpz_t*, SecretShare*) and
// RSS (T**, replicatedSecretShare<T>*) types.
//
// The helper functions (ss_init, ss_set, ss_batch_free_operator, Mult, etc.)
// are defined in the protocol-specific files (ShamirOps.h, RSSOps.hpp) and
// are resolved at compile time based on the template parameter types.
// ============================================================================

// Helper function for conditional assignment in batch statements
// Uses auto-deduced types to work with both RSS and Shamir
template <typename PRIV_TYPE, typename COND_TYPE, typename SS_TYPE>
void ss_batch_handle_priv_cond(PRIV_TYPE *result, PRIV_TYPE *result_org, COND_TYPE out_cond, PRIV_TYPE *priv_cond, int counter, int size, int threadID, NodeNetwork &net, SS_TYPE *ss) {
    PRIV_TYPE *tmp = (PRIV_TYPE *)malloc(sizeof(PRIV_TYPE) * size);
    for (int i = 0; i < size; ++i)
        ss_init(tmp[i], ss->getNumShares());

    if (out_cond == NULL && counter == -1 && priv_cond == NULL) {
        // No condition - result stays as computed
    } else if (out_cond != NULL && counter == -1 && priv_cond == NULL) {
        // Public outer condition
        for (int i = 0; i < size; i++)
            ss_set(tmp[i], out_cond);
        ss->modSub(result, result, result_org, size);
        Mult(result, result, tmp, size, threadID, net, ss);
        ss->modAdd(result, result, result_org, size);
    } else if (out_cond == NULL && counter != -1 && priv_cond != NULL) {
        // Private condition array
        for (int i = 0; i < size; i++) {
            if (counter != size)
                ss_set(tmp[i], priv_cond[i / (size / counter)]);
            else
                ss_set(tmp[i], priv_cond[i]);
        }
        ss->modSub(result, result, result_org, size);
        Mult(result, result, tmp, size, threadID, net, ss);
        ss->modAdd(result, result, result_org, size);
    } else {
        // Mixed or other cases
        ss->modSub(result, result, result_org, size);
        if (priv_cond != NULL) {
            for (int i = 0; i < size; i++)
                ss_set(tmp[i], priv_cond[i]);
            Mult(result, result, tmp, size, threadID, net, ss);
        }
        if (out_cond != NULL) {
            for (int i = 0; i < size; i++)
                ss_set(tmp[i], out_cond);
            Mult(result, result, tmp, size, threadID, net, ss);
        }
        ss->modAdd(result, result, result_org, size);
    }

    ss_batch_free_operator(&tmp, size);
}

// Core batch operation dispatcher for integer operations
template <typename PRIV_TYPE, typename COND_TYPE, typename SS_TYPE>
void ss_batch_BOP_int(PRIV_TYPE *result, PRIV_TYPE *a, PRIV_TYPE *b, int resultlen, int alen, int blen, COND_TYPE out_cond, PRIV_TYPE *priv_cond, int counter, int size, std::string op, std::string type, int threadID, NodeNetwork &net, SS_TYPE *ss) {

    // Remove whitespace from operator
    op.erase(std::remove(op.begin(), op.end(), ' '), op.end());

    // Save original result for conditional assignment
    PRIV_TYPE *result_org = (PRIV_TYPE *)malloc(sizeof(PRIV_TYPE) * size);
    for (int i = 0; i < size; i++)
        ss_init_set(result_org[i], result[i], ss);

    // Perform the operation
    if (op == "*") {
        Mult(result, a, b, size, threadID, net, ss);
    } else if (op == "-") {
        ss->modSub(result, a, b, size);
    } else if (op == "+") {
        ss->modAdd(result, a, b, size);
    } else if (op == "==") {
        doOperation_EQZ(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
    } else if (op == "!=") {
        doOperation_EQZ(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
        ss->modSub(result, 1, result, size);
    } else if (op == ">") {
        doOperation_LT(result, b, a, blen, alen, resultlen, size, threadID, net, ss);
    } else if (op == ">=") {
        doOperation_LT(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
        ss->modSub(result, 1, result, size);
    } else if (op == "<") {
        doOperation_LT(result, a, b, alen, blen, resultlen, size, threadID, net, ss);
    } else if (op == "<=") {
        doOperation_LT(result, b, a, blen, alen, resultlen, size, threadID, net, ss);
        ss->modSub(result, 1, result, size);
    } else if (op == "/") {
        doOperation_IntDiv(result, a, b, resultlen, size, threadID, net, ss);
    } else if (op == "/P") {
        doOperation_IntDiv_Pub(result, a, b, resultlen, size, threadID, net, ss);
    } else if (op == "=") {
        ss_set(a, result, alen, resultlen, size, type, threadID, net, ss);
    } else if (op == ">>") {
        ss_shr(a, b, alen, blen, result, resultlen, size, type, threadID, net, ss);
    } else if (op == "<<") {
        ss_shl(a, b, alen, blen, result, resultlen, size, type, threadID, net, ss);
    } else {
        std::cout << "[ss_batch_BOP_int] Unrecognized op: " << op << "\n";
    }

    // Apply conditional assignment if needed
    ss_batch_handle_priv_cond(result, result_org, out_cond, priv_cond, counter, size, threadID, net, ss);
    ss_batch_free_operator(&result_org, size);
}

// Main batch function - handles array indexing and calls BOP
template <typename PRIV_TYPE, typename COND_TYPE, typename SS_TYPE>
void ss_batch(PRIV_TYPE *a, PRIV_TYPE *b, PRIV_TYPE *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, COND_TYPE out_cond, PRIV_TYPE *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork &net, SS_TYPE *ss) {
    PRIV_TYPE *a_tmp, *b_tmp, *result_tmp;
    uint numShares = ss->getNumShares();
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2, numShares);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);

    ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, ss);

    for (int i = 0; i < size; ++i)
        ss_set(result_tmp[i], result[index_array[3 * i + 2]]);

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// Overload for PRIV_TYPE*, int*, PRIV_TYPE* (secret op public = secret)
template <typename PRIV_TYPE, typename COND_TYPE, typename SS_TYPE>
void ss_batch(PRIV_TYPE *a, int *b, PRIV_TYPE *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, COND_TYPE out_cond, PRIV_TYPE *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork &net, SS_TYPE *ss) {
    // Not yet fully implemented for scalar operations
    std::cout << "[Batch] PRIV_TYPE*, int*, PRIV_TYPE* operations not yet implemented\n";
}

// Overload for int*, PRIV_TYPE*, PRIV_TYPE* (public op secret = secret)
template <typename PRIV_TYPE, typename COND_TYPE, typename SS_TYPE>
void ss_batch(int *a, PRIV_TYPE *b, PRIV_TYPE *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, COND_TYPE out_cond, PRIV_TYPE *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork &net, SS_TYPE *ss) {
    // Not yet fully implemented for scalar operations
    std::cout << "[Batch] int*, PRIV_TYPE*, PRIV_TYPE* operations not yet implemented\n";
}

// Overload for int*, int*, PRIV_TYPE* (public op public = secret assignment)
template <typename PRIV_TYPE, typename COND_TYPE, typename SS_TYPE>
void ss_batch(int *a, int *b, PRIV_TYPE *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, COND_TYPE out_cond, PRIV_TYPE *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork &net, SS_TYPE *ss) {
    // Not yet fully implemented for scalar operations
    std::cout << "[Batch] int*, int*, PRIV_TYPE* operations not yet implemented\n";
}

// Overload for 2D arrays (used by float operations)
template <typename PRIV_TYPE, typename COND_TYPE, typename SS_TYPE>
void ss_batch(PRIV_TYPE **a, PRIV_TYPE **b, PRIV_TYPE **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, int dim1_len, int dim2_len, int dim3_len, COND_TYPE out_cond, PRIV_TYPE *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork &net, SS_TYPE *ss) {
    // For float arrays - stub for now
    std::cout << "[Batch] PRIV_TYPE** batch operations (2D arrays) not yet fully implemented\n";
}

#endif // _BATCH_HPP_
