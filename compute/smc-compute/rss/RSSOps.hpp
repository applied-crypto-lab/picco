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
#ifndef _RSSOPS_HPP_
#define _RSSOPS_HPP_

/*
 * RSS Operations for PICCO
 *
 * Array Format Conventions for RSS (Replicated Secret Sharing):
 * ================================================================
 *
 * RSS uses 3-party computation where each party holds 2 shares (numShares=2).
 *
 * INTEGER ARRAYS:
 * - Single integer:    priv_int (which is T* in RSS)
 *                     Format: [numShares] where numShares=2
 *
 * - Integer array:     priv_int* (which is T** in RSS)
 *                     Format: [size][numShares]
 *                     Example: array[i][s] = share s of element i
 *
 * FLOAT ARRAYS:
 * - Single float:      priv_int** (which is T** in RSS)
 *                     Format: [4][numShares]
 *                     Components: [0]=mantissa, [1]=exponent, [2]=zero_flag, [3]=sign
 *                     Example: float_var[component][share]
 *
 * - Float array:       priv_int*** (which is T*** in RSS)
 *                     Format: [size][4][numShares]
 *                     Example: float_array[i][component][share]
 *                     Where i is the element index, component is 0-3, and share is 0-1
 *
 * COMPARISON RESULTS:
 * - Comparison result: priv_int* (which is T** in RSS)
 *                     Format: [size][numShares]
 *                     Values: secret-shared bits (0 or 1)
 */

#include "NodeNetwork.h"
#include "RepSecretShare.hpp"
#include "../ops/rss/FLMult.hpp"
#include "../ops/rss/FLRound.hpp"
#include "../ops/rss/FL2Int.hpp"
#include "../ops/rss/Int2FL.hpp"
#include "../ops/rss/Pow2.hpp"
#include "../ops/rss/TruncS.hpp"
#include <fstream>
#include <iostream>
#include <math.h>
#include <sstream>
#include <string>

// Forward declare RSS batch operations (implementation in RSSBatch.hpp at end of file)

// Forward declare rss_convertFloat (defined later in this file)
template <typename T>
void rss_convertFloat(float value, int K, int L, T *elements);

// Forward declare batch helper functions (defined at end of file in RSSBatch section)
template <typename T>
inline void ss_batch_free_operator(T ***op, int size);
template <typename T>
inline void ss_batch_free_operator(T ****op, int size);
template <typename T>
inline void ss_convert_operator(T ***result, T ***op, int *index_array, int dim, int size, int flag, uint numShares);
template <typename T>
inline void ss_convert_operator(T ***result, T **op, int *index_array, int dim, int size, int flag, uint numShares);
template <typename T>
inline void ss_convert_operator(T ****result, T ***op, int *index_array, int dim, int size, int flag, uint numShares);
template <typename T>
inline void ss_convert_operator(T ****result, T ****op, int *index_array, int dim, int size, int flag, uint numShares);

template <typename T>
void ss_init_set(T *x, T *x_val) {
    return;
}

// ss_init_set with ss parameter - allocates memory and copies value (for RSS batch operations)
template <typename T>
void ss_init_set(T *&x, T *x_val, replicatedSecretShare<T> *ss) {
    int numShares = ss->getNumShares();
    x = (T *)malloc(sizeof(T) * numShares);
    for (int i = 0; i < numShares; i++) {
        x[i] = x_val[i];
    }
}

template <typename T>
void ss_init(T *x) {
    return;
}

template <typename T, typename U>
void ss_init(T*& x, U size) {
    x = (T*)malloc(sizeof(T) * size);
    // Initialize to zero - critical for RSS public->private conversions
    for (U i = 0; i < size; i++) {
        x[i] = 0;
    }
}

// Simple ss_set for batch operations is implemented at the end of this file (line ~682)

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
    // Process float results - follows Shamir structure
    // result1 is [size][4 components][numShares]
    // Handles format conversion if resultlen_sig != len_sig

    uint numShares = ss->getNumShares();

    // If formats are the same, no conversion needed
    if (resultlen_sig == len_sig && resultlen_exp == len_exp) {
        return;
    }

    // Create transposed array: result[component][element][share]
    T ***result = (T ***)malloc(sizeof(T **) * 4);
    for (int i = 0; i < 4; i++) {
        result[i] = (T **)malloc(sizeof(T *) * size);
        for (int j = 0; j < size; j++) {
            result[i][j] = (T *)malloc(sizeof(T) * numShares);
            for (uint s = 0; s < numShares; s++) {
                result[i][j][s] = result1[j][i][s];
            }
        }
    }

    // Truncate or scale mantissa if needed
    if (resultlen_sig < len_sig) {
        // Truncate results by 2^{len_sig-resultlen_sig}
        // TODO: Call doOperation_Trunc when implemented
        // For now, just shift right locally (approximation)
        int shift = len_sig - resultlen_sig;
        for (int j = 0; j < size; j++) {
            for (uint s = 0; s < numShares; s++) {
                result[0][j][s] = result[0][j][s] >> shift;
                result[1][j][s] = result[1][j][s] + T(shift);
            }
        }
    } else if (resultlen_sig > len_sig) {
        // Scale up by 2^{resultlen_sig-len_sig}
        int shift = resultlen_sig - len_sig;
        for (int j = 0; j < size; j++) {
            for (uint s = 0; s < numShares; s++) {
                result[0][j][s] = result[0][j][s] << shift;
                result[1][j][s] = result[1][j][s] - T(shift);
            }
        }
    }

    // Copy transposed result back to result1
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < size; j++) {
            for (uint s = 0; s < numShares; s++) {
                result1[j][i][s] = result[i][j][s];
            }
        }
    }

    // Cleanup
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < size; j++) {
            free(result[i][j]);
        }
        free(result[i]);
    }
    free(result);
}

template <typename T>
void ss_single_convert_to_private_float(float a, T ***priv_a, int len_sig, int len_exp, replicatedSecretShare<T> *ss) {
    // Allocate T**[4][numShares] for a single private float
    // Use malloc to match ss_batch_free_operator which uses free()
    uint numShares = ss->getNumShares();
    T **result = (T **)malloc(sizeof(T *) * 4);
    for (int c = 0; c < 4; c++) {
        result[c] = (T *)malloc(sizeof(T) * numShares);
        memset(result[c], 0, sizeof(T) * numShares);
    }

    // Decompose public float into 4 components
    T components[4];
    rss_convertFloat(a, len_sig, len_exp, components);

    // Place each component value into the appropriate share slot
    for (int c = 0; c < 4; c++) {
        ss->sparsify_public(result[c], components[c]);
    }

    *priv_a = result;
}

template <typename T>
void ss_batch_convert_to_private_float(float *a, T ****priv_a, int len_sig, int len_exp, int size, replicatedSecretShare<T> *ss) {
    // Allocate T***[size][4][numShares] for a batch of private floats
    // Use malloc to match ss_batch_free_operator which uses free()
    uint numShares = ss->getNumShares();
    T ***result = (T ***)malloc(sizeof(T **) * size);
    for (int i = 0; i < size; i++) {
        result[i] = (T **)malloc(sizeof(T *) * 4);
        for (int c = 0; c < 4; c++) {
            result[i][c] = (T *)malloc(sizeof(T) * numShares);
            memset(result[i][c], 0, sizeof(T) * numShares);
        }

        // Decompose each public float
        T components[4];
        rss_convertFloat(a[i], len_sig, len_exp, components);

        for (int c = 0; c < 4; c++) {
            ss->sparsify_public(result[i][c], components[c]);
        }
    }

    *priv_a = result;
}

// ss_convert_operator and ss_batch_free_operator are now implemented in RSSBatch.hpp

template <typename T>
void ss_single_fop_comparison(T *result, T **a, T **b, int resultlen, int alen_sig, int alen_exp, int blen_sig, int blen_exp, std::string op, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // Single float comparison dispatcher for RSS
    // Performs comparison operations on single private floats (e.g., a < b)
    // Input format:  a[4][numShares], b[4][numShares] where component 0-3 = (mantissa, exponent, zero, sign)
    // Output format: result[numShares] - a single bit comparison result (1 or 0)
    // For RSS: numShares=2 (two shares per party for 3-party computation)
    // FLLT expects: a[component][element_index][share] format, so we convert to 3D arrays internally

    uint numShares = ss->getNumShares();
    int size = 1;

    // Allocate 3D arrays for FLLT: [4 components][size=1][numShares]
    T ***a_3d = new T**[4];
    T ***b_3d = new T**[4];

    for (int comp = 0; comp < 4; comp++) {
        a_3d[comp] = new T*[size];
        b_3d[comp] = new T*[size];

        for (int i = 0; i < size; i++) {
            a_3d[comp][i] = new T[numShares];
            b_3d[comp][i] = new T[numShares];

            // Copy input: a[comp][share] -> a_3d[comp][0][share]
            for (uint s = 0; s < numShares; s++) {
                a_3d[comp][i][s] = a[comp][s];
                b_3d[comp][i][s] = b[comp][s];
            }
        }
    }

    // Allocate result array for FLLT: [size][numShares]
    T **result_2d = new T*[size];
    for (int i = 0; i < size; i++) {
        result_2d[i] = new T[numShares];
        memset(result_2d[i], 0, sizeof(T) * numShares);
    }

    // Use the larger of the two operand sizes for K (mantissa bits) and L (exponent bits)
    int K = alen_sig > blen_sig ? alen_sig : blen_sig;
    int L = alen_exp > blen_exp ? alen_exp : blen_exp;

    // Call FLLT for "<0" operation (a < b)
    if (op == "<0") {
        FLLT(a_3d, b_3d, result_2d, size, threadID, net, ss);
    }
    // Call FLEQZ for "==" operation (a == b)
    else if (op == "==") {
        doOperation_FLEQZ(a_3d, b_3d, result_2d, K, L, size, threadID, net, ss);
    }

    // Copy result back: result_2d[0][share] -> result[share]
    for (uint s = 0; s < numShares; s++) {
        result[s] = result_2d[0][s];
    }

    // Cleanup
    for (int comp = 0; comp < 4; comp++) {
        for (int i = 0; i < size; i++) {
            delete[] a_3d[comp][i];
            delete[] b_3d[comp][i];
        }
        delete[] a_3d[comp];
        delete[] b_3d[comp];
    }
    delete[] a_3d;
    delete[] b_3d;

    for (int i = 0; i < size; i++) {
        delete[] result_2d[i];
    }
    delete[] result_2d;
}

template <typename T>
void ss_single_fop_arithmetic(T **result, T **a, T **b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, std::string op, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // Single float arithmetic dispatcher for RSS
    // Performs arithmetic operations on single private floats (e.g., a + b, a * b, a - b)
    // Input format:  a[4][numShares], b[4][numShares] where component 0-3 = (mantissa, exponent, zero, sign)
    // Output format: result[4][numShares] - single float result
    // For RSS: numShares=2 (two shares per party for 3-party computation)
    // FL operations (FLAdd, FLMult, etc.) expect: a[component][element_index][share] format, so we convert internally

    uint numShares = ss->getNumShares();
    int size = 1;

    // Allocate 3D arrays: [4 components][size=1][numShares]
    T ***a_3d = new T**[4];
    T ***b_3d = new T**[4];
    T ***result_3d = new T**[4];

    // For subtraction, we need to negate b's sign
    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, 1);

    for (int comp = 0; comp < 4; comp++) {
        a_3d[comp] = new T*[size];
        b_3d[comp] = new T*[size];
        result_3d[comp] = new T*[size];

        for (int i = 0; i < size; i++) {
            a_3d[comp][i] = new T[numShares];
            b_3d[comp][i] = new T[numShares];
            result_3d[comp][i] = new T[numShares];

            // Copy input: a[comp][share] -> a_3d[comp][0][share]
            for (uint s = 0; s < numShares; s++) {
                a_3d[comp][i][s] = a[comp][s];
                // For subtraction, negate b's sign: s -> 1 - s
                if (op == "-" && comp == 3) {
                    b_3d[comp][i][s] = ai[s] - b[comp][s];
                } else {
                    b_3d[comp][i][s] = b[comp][s];
                }
                result_3d[comp][i][s] = 0;
            }
        }
    }

    // Use the larger of the two operand sizes for K (mantissa bits) and L (exponent bits)
    int K = (alen_sig > blen_sig) ? alen_sig : blen_sig;
    int L = (alen_exp > blen_exp) ? alen_exp : blen_exp;

    // Call the appropriate float operation
    if (op == "*") {
        doOperation_FLMult(a_3d, b_3d, result_3d, K, size, threadID, net, ss);
    }
    else if (op == "+" || op == "-") {
        // Subtraction is handled by negating b's sign above
        doOperation_FLAdd(a_3d, b_3d, result_3d, K, L, size, threadID, net, ss);
    }
    else if (op == "/") {
        // FLDiv needs an error output array
        T **error = new T*[size];
        for (int i = 0; i < size; i++) {
            error[i] = new T[numShares];
            memset(error[i], 0, sizeof(T) * numShares);
        }
        doOperation_FLDiv(a_3d, b_3d, result_3d, error, K, size, threadID, net, ss);
        for (int i = 0; i < size; i++) {
            delete[] error[i];
        }
        delete[] error;
    }

    // Copy result back: result_3d[comp][0][share] -> result[comp][share]
    for (int comp = 0; comp < 4; comp++) {
        for (uint s = 0; s < numShares; s++) {
            result[comp][s] = result_3d[comp][0][s];
        }
    }

    // Cleanup
    delete[] ai;
    for (int comp = 0; comp < 4; comp++) {
        for (int i = 0; i < size; i++) {
            delete[] a_3d[comp][i];
            delete[] b_3d[comp][i];
            delete[] result_3d[comp][i];
        }
        delete[] a_3d[comp];
        delete[] b_3d[comp];
        delete[] result_3d[comp];
    }
    delete[] a_3d;
    delete[] b_3d;
    delete[] result_3d;
}

// Batch float comparison: public float array vs private float array (STUB - not yet implemented)
// Input format:  a[size], b[size][4][numShares]
// Output format: result[size][numShares] - array of comparison results
template <typename T>
void ss_batch_fop_comparison(T **result, float *a, T ***b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

// Batch float comparison: private float array vs public float array (STUB - not yet implemented)
// Input format:  a[size][4][numShares], b[size]
// Output format: result[size][numShares] - array of comparison results
template <typename T>
void ss_batch_fop_comparison(T **result, T ***a, float *b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

// Batch float comparison: private float array vs private float array
// Input format:  a[size][4][numShares], b[size][4][numShares]
// Output format: result[size][numShares] - array of comparison results (1 or 0)
template <typename T>
void ss_batch_fop_comparison(T **result, T ***a, T ***b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    // Transpose: a[size][4][numShares] -> a_t[4][size][numShares]
    T ***a_t = new T**[4];
    T ***b_t = new T**[4];
    for (int comp = 0; comp < 4; comp++) {
        a_t[comp] = new T*[size];
        b_t[comp] = new T*[size];
        for (int i = 0; i < size; i++) {
            a_t[comp][i] = new T[numShares];
            b_t[comp][i] = new T[numShares];
            for (uint s = 0; s < numShares; s++) {
                a_t[comp][i][s] = a[i][comp][s];
                b_t[comp][i][s] = b[i][comp][s];
            }
        }
    }

    int K = (alen_sig > blen_sig) ? alen_sig : blen_sig;
    int L = (alen_exp > blen_exp) ? alen_exp : blen_exp;

    if (op == "<0") {
        FLLT(a_t, b_t, result, size, threadID, net, ss);
    } else if (op == "==") {
        doOperation_FLEQZ(a_t, b_t, result, K, L, size, threadID, net, ss);
    }

    // Cleanup
    for (int comp = 0; comp < 4; comp++) {
        for (int i = 0; i < size; i++) {
            delete[] a_t[comp][i];
            delete[] b_t[comp][i];
        }
        delete[] a_t[comp];
        delete[] b_t[comp];
    }
    delete[] a_t;
    delete[] b_t;
}

// Batch float arithmetic: private float array op private float array
// Input format:  a[size][4][numShares], b[size][4][numShares]
// Output format: result[size][4][numShares] - array of float results
// FL operations expect: [4_components][size][numShares], so we transpose
template <typename T>
void ss_batch_fop_arithmetic(T ***result, T ***a, T ***b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    // Allocate transposed arrays: [4 components][size][numShares]
    T ***a_t = new T**[4];
    T ***b_t = new T**[4];
    T ***r_t = new T**[4];

    // For subtraction, need shared value of 1 to negate sign
    T *one_shared = new T[numShares];
    memset(one_shared, 0, sizeof(T) * numShares);
    ss->sparsify_public(one_shared, 1);

    for (int comp = 0; comp < 4; comp++) {
        a_t[comp] = new T*[size];
        b_t[comp] = new T*[size];
        r_t[comp] = new T*[size];
        for (int i = 0; i < size; i++) {
            a_t[comp][i] = new T[numShares];
            b_t[comp][i] = new T[numShares];
            r_t[comp][i] = new T[numShares];
            for (uint s = 0; s < numShares; s++) {
                a_t[comp][i][s] = a[i][comp][s];
                if (op == "-" && comp == 3) {
                    b_t[comp][i][s] = one_shared[s] - b[i][comp][s];
                } else {
                    b_t[comp][i][s] = b[i][comp][s];
                }
                r_t[comp][i][s] = 0;
            }
        }
    }

    int K = (alen_sig > blen_sig) ? alen_sig : blen_sig;
    int L = (alen_exp > blen_exp) ? alen_exp : blen_exp;

    if (op == "*") {
        doOperation_FLMult(a_t, b_t, r_t, K, size, threadID, net, ss);
    } else if (op == "+" || op == "-") {
        doOperation_FLAdd(a_t, b_t, r_t, K, L, size, threadID, net, ss);
    } else if (op == "/") {
        T **error = new T*[size];
        for (int i = 0; i < size; i++) {
            error[i] = new T[numShares];
            memset(error[i], 0, sizeof(T) * numShares);
        }
        doOperation_FLDiv(a_t, b_t, r_t, error, K, size, threadID, net, ss);
        for (int i = 0; i < size; i++) delete[] error[i];
        delete[] error;
    }

    // Transpose result back: r_t[comp][i][s] -> result[i][comp][s]
    for (int comp = 0; comp < 4; comp++) {
        for (int i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                result[i][comp][s] = r_t[comp][i][s];
            }
        }
    }

    // Process results for format conversion if needed
    ss_process_results(result, resultlen_sig, resultlen_exp, K, L, size, threadID, net, ss);

    // Cleanup
    delete[] one_shared;
    for (int comp = 0; comp < 4; comp++) {
        for (int i = 0; i < size; i++) {
            delete[] a_t[comp][i];
            delete[] b_t[comp][i];
            delete[] r_t[comp][i];
        }
        delete[] a_t[comp];
        delete[] b_t[comp];
        delete[] r_t[comp];
    }
    delete[] a_t;
    delete[] b_t;
    delete[] r_t;
}

template <typename T>
void ss_set(T **a, T **result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // Copy single float: follows Shamir structure
    // a is T** with [4 components][numShares]
    // result is T** with [4 components][numShares]
    uint numShares = ss->getNumShares();
    int size = 1;

    // Allocate batch format: results[element_index][component][share]
    T ***results = (T ***)malloc(sizeof(T **) * size);
    results[0] = (T **)malloc(sizeof(T *) * 4);
    for (int i = 0; i < 4; i++) {
        results[0][i] = (T *)malloc(sizeof(T) * numShares);
        for (uint s = 0; s < numShares; s++) {
            results[0][i][s] = a[i][s];
        }
    }

    // Process results for format conversion if needed
    ss_process_results(results, resultlen_sig, resultlen_exp, alen_sig, alen_exp, size, threadID, net, ss);

    // Copy back to result
    for (int i = 0; i < 4; i++) {
        for (uint s = 0; s < numShares; s++) {
            result[i][s] = results[0][i][s];
        }
    }

    // Cleanup
    for (int i = 0; i < 4; i++) {
        free(results[0][i]);
    }
    free(results[0]);
    free(results);
}

template <typename T>
void ss_set(T ***a, T ***result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // Batch float set: copy array of floats with possible format conversion
    // a[size][4][numShares], result[size][4][numShares]
    uint numShares = ss->getNumShares();
    for (int i = 0; i < size; i++) {
        for (int comp = 0; comp < 4; comp++) {
            for (uint s = 0; s < numShares; s++) {
                result[i][comp][s] = a[i][comp][s];
            }
        }
    }
    ss_process_results(result, resultlen_sig, resultlen_exp, alen_sig, alen_exp, size, threadID, net, ss);
}

// Helper function to convert IEEE 754 float to PICCO float components
// Returns: elements[0]=mantissa, elements[1]=exponent, elements[2]=zero, elements[3]=sign
template <typename T>
void rss_convertFloat(float value, int K, int L, T *elements) {
    unsigned int *newptr = (unsigned int *)&value;
    int s = *newptr >> 31;                // sign bit
    int e = (*newptr & 0x7f800000) >> 23; // exponent
    int m = *newptr & 0x007fffff;         // mantissa (23 bits)

    T significand = 0;
    long p = 0;
    int z = 0;

    if (e == 0 && m == 0) {
        // Zero
        s = 0;
        z = 1;
        significand = 0;
        p = 0;
    } else {
        z = 0;
        if (L < 8) {
            long k = (1 << L) - 1;
            // Check for overflow
            if (e - 127 - K + 1 > k) {
                p = k;
                significand = (T(1) << K) - 1;
            // Check for underflow
            } else if (e - 127 - K + 1 < -k) {
                p = -k;
                significand = 1;
            } else {
                p = e - 127 - K + 1;
                m = m + (1 << 23);  // Add implicit 1
                if (K < 24) {
                    significand = T(m) >> (24 - K);
                } else {
                    significand = T(m) << (K - 24);
                }
            }
        } else {
            // L >= 8, no overflow/underflow check needed
            p = e - 127 - K + 1;
            m = m + (1 << 23);  // Add implicit 1
            if (K < 24) {
                significand = T(m) >> (24 - K);
            } else if (K == 24) {
                significand = T(m);
            } else {
                significand = T(m) << (K - 24);
            }
        }
    }

    elements[0] = significand;          // v: mantissa
    elements[1] = static_cast<T>(p);    // p: exponent (signed, but stored as T)
    elements[2] = static_cast<T>(z);    // z: zero flag
    elements[3] = static_cast<T>(s);    // s: sign
}

// Convert public float to secret-shared float
// result is T** with [4 components][numShares]
template <typename T>
void ss_set(float a, T **result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // Convert the IEEE 754 float to PICCO float components
    T components[4];
    rss_convertFloat(a, resultlen_sig, resultlen_exp, components);

    uint numShares = ss->getNumShares();
    for (int c = 0; c < 4; c++) {
        // Zero all shares first — result may contain old values from previous assignments
        for (uint s = 0; s < numShares; s++)
            result[c][s] = 0;
        ss->sparsify_public(result[c], components[c]);
    }
}

template <typename T>
void ss_set(T *a, T *result, int alen, int resultlen, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    memcpy(result, a, sizeof(T) * ss->getNumShares());
}

template <typename T>
void ss_set(T *a, T *result, int alen, int resultlen, int size, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // For the new [element_index][share_index] layout
    for (size_t i = 0; i < size; i++) {
        memcpy(result[i], a[i], sizeof(*a[i]) * ss->getNumShares());
    }
}

template <typename T>
void ss_set(T **a, T **result, int alen, int resultlen, int size, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // For the new [element_index][share_index] layout:
    // a[i][j] = j-th share of i-th element
    for (int i = 0; i < size; i++) {
        // Copy all shares for element i
        memcpy(result[i], a[i], sizeof(T) * ss->getNumShares());
    }
}

template <typename T>
void ss_set(int a, T *result, int alen, int resultlen, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // Set a public constant into a secret-shared value
    // First zero out all shares, then add the constant using const_map
    uint numShares = ss->getNumShares();
    for (size_t i = 0; i < numShares; i++) {
        result[i] = 0;
    }
    // modAdd uses const_map to properly distribute the constant across shares
    ss->modAdd(result, result, (long)a);
}

// Conditional assignment for integers: a = cond ? a : b
// Computes a = (a - b) * cond + b
template <typename T>
void ss_priv_eval(T *a, T *b, T *cond, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **op1 = new T*[1];
    T **op2 = new T*[1];
    T **results = new T*[1];
    op1[0] = new T[numShares];
    op2[0] = new T[numShares];
    results[0] = new T[numShares];
    for (uint s = 0; s < numShares; s++) {
        op1[0][s] = a[s] - b[s];
        op2[0][s] = cond[s];
        results[0][s] = 0;
    }
    Mult(results, op1, op2, 1, threadID, net, ss);
    for (uint s = 0; s < numShares; s++) {
        a[s] = results[0][s] + b[s];
    }
    delete[] op1[0]; delete[] op2[0]; delete[] results[0];
    delete[] op1; delete[] op2; delete[] results;
}

// Conditional assignment for floats: a = cond ? a : b
// a[4][numShares], b[4][numShares], cond[numShares]
template <typename T>
void ss_priv_eval(T **a, T **b, T *cond, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **op1 = new T*[4];
    T **op2 = new T*[4];
    T **results = new T*[4];
    for (int i = 0; i < 4; i++) {
        op1[i] = new T[numShares];
        op2[i] = new T[numShares];
        results[i] = new T[numShares];
        for (uint s = 0; s < numShares; s++) {
            op1[i][s] = a[i][s] - b[i][s];
            op2[i][s] = cond[s];
            results[i][s] = 0;
        }
    }
    Mult(results, op1, op2, 4, threadID, net, ss);
    for (int i = 0; i < 4; i++) {
        for (uint s = 0; s < numShares; s++) {
            a[i][s] = results[i][s] + b[i][s];
        }
        delete[] op1[i]; delete[] op2[i]; delete[] results[i];
    }
    delete[] op1; delete[] op2; delete[] results;
}

template <typename T>
void ss_sub(T **a, T **b, T **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // Subtraction is addition with negated sign - call arithmetic op with "-"
    ss_single_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, "-", threadID, net, ss);
}

// Batch integer subtraction: public int array - private int array
template <typename T>
void ss_sub(int *a, T **b, int alen, int blen, T **result, int resultlen, int size, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T *pub = new T[numShares];
    for (int i = 0; i < size; i++) {
        memset(pub, 0, sizeof(T) * numShares);
        ss->sparsify_public(pub, a[i]);
        for (uint s = 0; s < numShares; s++) {
            result[i][s] = pub[s] - b[i][s];
        }
    }
    delete[] pub;
}

// Batch integer subtraction: private int array - public int array
template <typename T>
void ss_sub(T **a, int *b, int alen, int blen, T **result, int resultlen, int size, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T *pub = new T[numShares];
    for (int i = 0; i < size; i++) {
        memset(pub, 0, sizeof(T) * numShares);
        ss->sparsify_public(pub, b[i]);
        for (uint s = 0; s < numShares; s++) {
            result[i][s] = a[i][s] - pub[s];
        }
    }
    delete[] pub;
}

// Batch float subtraction: private float array - private float array
template <typename T>
void ss_sub(T ***a, T ***b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, T ***result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    ss_batch_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "-", threadID, net, ss);
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

// Scalar private int to float conversion
// value = T* [numShares] (single integer)
// result = T** [4][numShares] (single float: mantissa, exponent, zero, sign)
template <typename T>
void ss_int2fl(T *value, T **result, int gamma, int K, int L, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // Create 2D array wrapper for the single value
    T **value_2d = new T*[1];
    value_2d[0] = value;  // Point to existing data

    // Create 3D array for result in format [4][1][numShares]
    T ***result_3d = new T**[4];
    for (int c = 0; c < 4; c++) {
        result_3d[c] = new T*[1];
        result_3d[c][0] = result[c];  // Point to existing data
    }

    // Call the batch function with size=1
    doOperation_Int2FL(value_2d, result_3d, gamma, K, 1, threadID, net, ss);

    // Cleanup (just the wrappers, not the actual data)
    delete[] value_2d;
    for (int c = 0; c < 4; c++) {
        delete[] result_3d[c];
    }
    delete[] result_3d;
}

// Scalar public int to float conversion
template <typename T>
void ss_int2fl(int value, T **result, int gamma, int K, int L, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    // Public values have gamma=-1; use K as the bit width for the protocol
    if (gamma < 0) gamma = K;

    // Convert public value to private share format
    T *value_priv = new T[numShares];
    memset(value_priv, 0, sizeof(T) * numShares);
    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, 1);
    for (uint s = 0; s < numShares; s++) {
        value_priv[s] = T(value) * ai[s];
    }

    // Call the private version
    ss_int2fl(value_priv, result, gamma, K, L, threadID, net, ss);

    delete[] value_priv;
    delete[] ai;
}

// Array: public int* to float array conversion
template <typename T>
void ss_int2fl(int *value, T ***result, int size, int gamma, int K, int L, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    // Public values have gamma=-1; use K as the bit width for the protocol
    if (gamma < 0) gamma = K;

    // Convert public values to private share format
    T **value_priv = new T*[size];
    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, 1);

    for (int i = 0; i < size; i++) {
        value_priv[i] = new T[numShares];
        for (uint s = 0; s < numShares; s++) {
            value_priv[i][s] = T(value[i]) * ai[s];
        }
    }

    // Call the private version
    doOperation_Int2FL(value_priv, result, gamma, K, size, threadID, net, ss);

    // Cleanup
    for (int i = 0; i < size; i++) {
        delete[] value_priv[i];
    }
    delete[] value_priv;
    delete[] ai;
}

// Array: private int* to float array conversion
template <typename T>
void ss_int2fl(T **value, T ***result, int size, int gamma, int K, int L, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    doOperation_Int2FL(value, result, gamma, K, size, threadID, net, ss);
}

// Scalar int2int (stub - just copies the value, no actual conversion needed for same precision)
template <typename T>
void ss_int2int(T *value, T *result, int gamma1, int gamma2, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    for (uint s = 0; s < numShares; s++) {
        result[s] = value[s];
    }
}

template <typename T>
void ss_int2int(int value, T *result, int gamma1, int gamma2, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, 1);
    for (uint s = 0; s < numShares; s++) {
        result[s] = T(value) * ai[s];
    }
    delete[] ai;
}

template <typename T>
void ss_int2int(int *value, T **result, int size, int gamma1, int gamma2, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, 1);
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            result[i][s] = T(value[i]) * ai[s];
        }
    }
    delete[] ai;
}

template <typename T>
void ss_int2int(T **value, T **result, int size, int gamma1, int gamma2, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            result[i][s] = value[i][s];
        }
    }
}

// Scalar fl2int
template <typename T>
void ss_fl2int(T **value, T *result, int K, int L, int gamma, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // Convert scalar format [4][numShares] to batch format [4][1][numShares]
    T ***value_3d = new T**[4];
    for (int c = 0; c < 4; c++) {
        value_3d[c] = new T*[1];
        value_3d[c][0] = value[c];
    }
    T **result_2d = new T*[1];
    result_2d[0] = result;

    // Note: PICCO passes (L, K) to smc_fl2int which expects (K, L)
    // So wrapper's K = L_val (mantissa), L = K_val (exponent)
    // Pass (K, L) so doOperation_FL2Int's L = mantissa, K = exponent
    doOperation_FL2Int(value_3d, result_2d, K, L, gamma, 1, threadID, net, ss);

    // Cleanup wrappers (not the underlying data)
    for (int c = 0; c < 4; c++) {
        delete[] value_3d[c];
    }
    delete[] value_3d;
    delete[] result_2d;
}

template <typename T>
void ss_fl2int(float value, T *result, int K, int L, int gamma, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // TODO: Implement when FL2Int is available
    uint numShares = ss->getNumShares();
    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, 1);
    for (uint s = 0; s < numShares; s++) {
        result[s] = T(int(value)) * ai[s];  // Simple truncation for now
    }
    delete[] ai;
}

template <typename T>
void ss_fl2int(float *value, T **result, int size, int K, int L, int gamma, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    for (int i = 0; i < size; i++) {
        ss_fl2int(value[i], result[i], K, L, gamma, threadID, net, ss);
    }
}

template <typename T>
void ss_fl2int(T ***value, T **result, int size, int K, int L, int gamma, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // Create component-first view [4][size] aliasing into value[size][4]
    T ***value_comp = (T ***)malloc(sizeof(T **) * 4);
    for (int c = 0; c < 4; c++) {
        value_comp[c] = (T **)malloc(sizeof(T *) * size);
        for (int i = 0; i < size; i++)
            value_comp[c][i] = value[i][c];
    }
    // Same fix as scalar version: PICCO swaps K and L
    doOperation_FL2Int(value_comp, result, K, L, gamma, size, threadID, net, ss);
    for (int c = 0; c < 4; c++)
        free(value_comp[c]);
    free(value_comp);
}

// Convert single public float to private (secret-shared) float
template <typename T>
void ss_fl2fl(float value, T **result, int K1, int L1, int K2, int L2, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    T components[4];

    // Convert this float to PICCO float components
    rss_convertFloat(value, K2, L2, components);

    uint numShares = ss->getNumShares();
    for (int c = 0; c < 4; c++) {
        for (uint s = 0; s < numShares; s++)
            result[c][s] = 0;
        ss->sparsify_public(result[c], components[c]);
    }
}

// Convert single private float between precisions (K1,L1) -> (K2,L2)
template <typename T>
void ss_fl2fl(T **value, T **result, int K1, int L1, int K2, int L2, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    // Create temporary arrays for batch operation with size=1
    T **value_v = new T*[1]; value_v[0] = value[0];
    T **value_p = new T*[1]; value_p[0] = value[1];
    T **value_z = new T*[1]; value_z[0] = value[2];
    T **value_s = new T*[1]; value_s[0] = value[3];

    T **result_v = new T*[1]; result_v[0] = result[0];
    T **result_p = new T*[1]; result_p[0] = result[1];
    T **result_z = new T*[1]; result_z[0] = result[2];
    T **result_s = new T*[1]; result_s[0] = result[3];

    if (K1 >= K2) {
        // Truncate mantissa: result[0] = Trunc(value[0], K1, K1-K2)
        doOperation_Trunc(result_v, value_v, K1, K1 - K2, 1, threadID, net, ss);
    } else {
        // Expand mantissa: result[0] = value[0] * 2^(K2-K1)
        T shift = T(1) << (K2 - K1);
        for (uint s = 0; s < numShares; s++) {
            result[0][s] = value[0][s] * shift;
        }
    }

    // Adjust exponent: result[1] = value[1] + (1-z)*(K1 - K2)
    // When z=1 (value is zero), don't adjust the exponent
    T exp_adjust = static_cast<T>(K1 - K2);
    T *ai = new T[numShares]();  // Zero-initialize before sparsify
    ss->sparsify_public(ai, 1);
    for (uint s = 0; s < numShares; s++) {
        T one_minus_z = ai[s] - value[2][s];  // (1 - z)
        result[1][s] = value[1][s] + one_minus_z * exp_adjust;
    }
    delete[] ai;

    // Copy zero and sign unchanged
    for (uint s = 0; s < numShares; s++) {
        result[2][s] = value[2][s];
        result[3][s] = value[3][s];
    }

    delete[] value_v; delete[] value_p; delete[] value_z; delete[] value_s;
    delete[] result_v; delete[] result_p; delete[] result_z; delete[] result_s;
}

// Convert public float array to private (secret-shared) float array
template <typename T>
void ss_fl2fl(float *value, T ***result, int size, int K1, int L1, int K2, int L2, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    T components[4];
    uint numShares = ss->getNumShares();

    for (int i = 0; i < size; i++) {
        // Convert this float to PICCO float components
        rss_convertFloat(value[i], K2, L2, components);

        for (int c = 0; c < 4; c++) {
            for (uint s = 0; s < numShares; s++)
                result[c][i][s] = 0;
            ss->sparsify_public(result[c][i], components[c]);
        }
    }
}

// Convert private float array between precisions (K1,L1) -> (K2,L2)
template <typename T>
void ss_fl2fl(T ***value, T ***result, int size, int K1, int L1, int K2, int L2, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    if (K1 >= K2) {
        // Truncate mantissa: result[0] = Trunc(value[0], K1, K1-K2)
        doOperation_Trunc(result[0], value[0], K1, K1 - K2, size, threadID, net, ss);
    } else {
        // Expand mantissa: result[0] = value[0] * 2^(K2-K1)
        T shift = T(1) << (K2 - K1);
        for (int i = 0; i < size; i++) {
            for (uint s = 0; s < numShares; s++) {
                result[0][i][s] = value[0][i][s] * shift;
            }
        }
    }

    // Adjust exponent: result[1] = value[1] + (1-z)*(K1 - K2)
    // When z=1 (value is zero), don't adjust the exponent
    T exp_adjust = static_cast<T>(K1 - K2);
    T *ai = new T[numShares]();  // Zero-initialize before sparsify
    ss->sparsify_public(ai, 1);
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            T one_minus_z = ai[s] - value[2][i][s];  // (1 - z)
            result[1][i][s] = value[1][i][s] + one_minus_z * exp_adjust;
        }
    }
    delete[] ai;

    // Copy zero and sign unchanged
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            result[2][i][s] = value[2][i][s];
            result[3][i][s] = value[3][i][s];
        }
    }
}

// REMOVED: These stubs conflict with common Batch.hpp implementations
// ss_batch_handle_priv_cond, ss_batch_BOP_int, ss_shr, ss_shl now come from ops/Batch.hpp

template <typename T>
void ss_batch_BOP_float_arithmetic(T ***result, T ***a, T ***b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, T *out_cond, T **priv_cond, int counter, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    op.erase(std::remove(op.begin(), op.end(), ' '), op.end());
    uint numShares = ss->getNumShares();

    // Save original result for conditional assignment
    T ***result_org = (T ***)malloc(sizeof(T **) * size);
    for (int i = 0; i < size; i++) {
        result_org[i] = (T **)malloc(sizeof(T *) * 4);
        for (int j = 0; j < 4; j++) {
            result_org[i][j] = (T *)malloc(sizeof(T) * numShares);
            for (uint s = 0; s < numShares; s++)
                result_org[i][j][s] = result[i][j][s];
        }
    }

    if (op == "*") {
        ss_batch_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "*", threadID, net, ss);
    } else if (op == "-") {
        ss_sub(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID, net, ss);
    } else if (op == "+") {
        ss_batch_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "+", threadID, net, ss);
    } else if (op == "/") {
        ss_batch_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "/", threadID, net, ss);
    } else if (op == "=") {
        ss_set(a, result, alen_sig, alen_exp, resultlen_sig, resultlen_exp, size, type, threadID, net, ss);
    } else {
        std::cout << "[ss_batch_BOP_float_arithmetic] Unrecognized op: " << op << "\n";
    }

    // Flatten for conditional assignment: result[size][4][numShares] -> flat[4*size][numShares]
    T **result_flat = (T **)malloc(sizeof(T *) * 4 * size);
    T **result_org_flat = (T **)malloc(sizeof(T *) * 4 * size);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 4; j++) {
            result_flat[4 * i + j] = (T *)malloc(sizeof(T) * numShares);
            result_org_flat[4 * i + j] = (T *)malloc(sizeof(T) * numShares);
            for (uint s = 0; s < numShares; s++) {
                result_flat[4 * i + j][s] = result[i][j][s];
                result_org_flat[4 * i + j][s] = result_org[i][j][s];
            }
        }
    }

    ss_batch_handle_priv_cond(result_flat, result_org_flat, out_cond, priv_cond, counter, 4 * size, threadID, net, ss);

    // Copy back
    for (int i = 0; i < size; i++)
        for (int j = 0; j < 4; j++)
            for (uint s = 0; s < numShares; s++)
                result[i][j][s] = result_flat[4 * i + j][s];

    // Cleanup
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 4; j++)
            free(result_org[i][j]);
        free(result_org[i]);
    }
    free(result_org);
    ss_batch_free_operator(&result_flat, 4 * size);
    ss_batch_free_operator(&result_org_flat, 4 * size);
}

template <typename T>
void ss_batch_BOP_float_comparison(T **result, T ***a, T ***b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    T **result_org = (T **)malloc(sizeof(T *) * size);
    T **result_tmp = (T **)malloc(sizeof(T *) * size);
    for (int i = 0; i < size; i++) {
        result_org[i] = (T *)malloc(sizeof(T) * numShares);
        result_tmp[i] = (T *)malloc(sizeof(T) * numShares);
        for (uint s = 0; s < numShares; s++) {
            result_org[i][s] = result[index_array[3 * i + 2]][s];
            result_tmp[i][s] = 0;
        }
    }

    if (op == "==") {
        ss_batch_fop_comparison(result_tmp, a, b, resultlen_sig, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "==", threadID, net, ss);
    } else if (op == "!=") {
        ss_batch_fop_comparison(result_tmp, a, b, resultlen_sig, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "==", threadID, net, ss);
        ss->modSub(result_tmp, 1, result_tmp, size);
    } else if (op == ">") {
        ss_batch_fop_comparison(result_tmp, b, a, resultlen_sig, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID, net, ss);
    } else if (op == ">=") {
        ss_batch_fop_comparison(result_tmp, a, b, resultlen_sig, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "<0", threadID, net, ss);
        ss->modSub(result_tmp, 1, result_tmp, size);
    } else if (op == "<") {
        ss_batch_fop_comparison(result_tmp, a, b, resultlen_sig, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "<0", threadID, net, ss);
    } else if (op == "<=") {
        ss_batch_fop_comparison(result_tmp, b, a, resultlen_sig, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID, net, ss);
        ss->modSub(result_tmp, 1, result_tmp, size);
    }

    ss_batch_handle_priv_cond(result_tmp, result_org, out_cond, priv_cond, counter, size, threadID, net, ss);

    for (int i = 0; i < size; ++i)
        for (uint s = 0; s < numShares; s++)
            result[index_array[3 * i + 2]][s] = result_tmp[i][s];

    ss_batch_free_operator(&result_org, size);
    ss_batch_free_operator(&result_tmp, size);
}

// ============================================================================
// Integer batch operations for 2D arrays (T***) — RSS implementations
// Pattern: extract indexed elements via ss_convert_operator, call ss_batch_BOP_int, write back
// ============================================================================

// Helper: write back 1D result_tmp into 2D result using dim decomposition
template <typename T>
static inline void rss_batch_writeback_2d(T ***result, T **result_tmp, int *index_array, int resultdim, int size, uint numShares) {
    for (int i = 0; i < size; ++i) {
        if (resultdim != 0) {
            int dim1 = index_array[3 * i + 2] / resultdim;
            int dim2 = index_array[3 * i + 2] % resultdim;
            for (uint s = 0; s < numShares; s++)
                result[dim1][dim2][s] = result_tmp[i][s];
        } else {
            for (uint s = 0; s < numShares; s++)
                result[0][index_array[3 * i + 2]][s] = result_tmp[i][s];
        }
    }
}

// Helper: write back 1D result_tmp into 1D result
template <typename T>
static inline void rss_batch_writeback_1d(T **result, T **result_tmp, int *index_array, int size, uint numShares) {
    for (int i = 0; i < size; ++i)
        for (uint s = 0; s < numShares; s++)
            result[index_array[3 * i + 2]][s] = result_tmp[i][s];
}

// int*, int* -> T** (public, public -> 1D private)
template <typename T>
void ss_batch(int *a, int *b, T **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **a_tmp = (T **)malloc(sizeof(T *) * size);
    T **b_tmp = (T **)malloc(sizeof(T *) * size);
    for (int i = 0; i < size; i++) {
        a_tmp[i] = (T *)malloc(sizeof(T) * numShares);
        b_tmp[i] = (T *)malloc(sizeof(T) * numShares);
        memset(a_tmp[i], 0, sizeof(T) * numShares);
        memset(b_tmp[i], 0, sizeof(T) * numShares);
        ss->sparsify_public(a_tmp[i], a[i]);
        ss->sparsify_public(b_tmp[i], b[i]);
    }
    ss_batch(a_tmp, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
}

// int*, T** -> T** (public, 1D private -> 1D private)
template <typename T>
void ss_batch(int *a, T **b, T **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **a_tmp = (T **)malloc(sizeof(T *) * size);
    for (int i = 0; i < size; i++) {
        a_tmp[i] = (T *)malloc(sizeof(T) * numShares);
        memset(a_tmp[i], 0, sizeof(T) * numShares);
        ss->sparsify_public(a_tmp[i], a[i]);
    }
    ss_batch(a_tmp, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&a_tmp, size);
}

// T**, int* -> T** (1D private, public -> 1D private)
template <typename T>
void ss_batch(T **a, int *b, T **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    std::string op_trimmed = op;
    op_trimmed.erase(std::remove(op_trimmed.begin(), op_trimmed.end(), ' '), op_trimmed.end());
    bool is_raw = (op_trimmed == ">>" || op_trimmed == "<<" || op_trimmed == "/");
    T **b_tmp = (T **)malloc(sizeof(T *) * size);
    for (int i = 0; i < size; i++) {
        b_tmp[i] = (T *)malloc(sizeof(T) * numShares);
        memset(b_tmp[i], 0, sizeof(T) * numShares);
        if (is_raw)
            b_tmp[i][0] = (T)b[i];
        else
            ss->sparsify_public(b_tmp[i], b[i]);
    }
    if (op_trimmed == "/") op = "/P";
    ss_batch(a, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&b_tmp, size);
}

// int*, int* -> T*** (public, public -> 2D private)
template <typename T>
void ss_batch(int *a, int *b, T ***result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **a_tmp = (T **)malloc(sizeof(T *) * size);
    T **b_tmp = (T **)malloc(sizeof(T *) * size);
    for (int i = 0; i < size; i++) {
        a_tmp[i] = (T *)malloc(sizeof(T) * numShares);
        b_tmp[i] = (T *)malloc(sizeof(T) * numShares);
        memset(a_tmp[i], 0, sizeof(T) * numShares);
        memset(b_tmp[i], 0, sizeof(T) * numShares);
        ss->sparsify_public(a_tmp[i], a[i]);
        ss->sparsify_public(b_tmp[i], b[i]);
    }
    ss_batch(a_tmp, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
}

// int*, T** -> T*** (public, 1D private -> 2D private)
template <typename T>
void ss_batch(int *a, T **b, T ***result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **a_tmp = (T **)malloc(sizeof(T *) * size);
    for (int i = 0; i < size; i++) {
        a_tmp[i] = (T *)malloc(sizeof(T) * numShares);
        memset(a_tmp[i], 0, sizeof(T) * numShares);
        ss->sparsify_public(a_tmp[i], a[i]);
    }
    ss_batch(a_tmp, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&a_tmp, size);
}

// T**, int* -> T*** (1D private, public -> 2D private)
template <typename T>
void ss_batch(T **a, int *b, T ***result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **b_tmp = (T **)malloc(sizeof(T *) * size);
    for (int i = 0; i < size; i++) {
        b_tmp[i] = (T *)malloc(sizeof(T) * numShares);
        memset(b_tmp[i], 0, sizeof(T) * numShares);
        ss->sparsify_public(b_tmp[i], b[i]);
    }
    op = (op == "/") ? "/P" : op;
    ss_batch(a, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&b_tmp, size);
}

// int*, T*** -> T** (public, 2D private -> 1D private)
template <typename T>
void ss_batch(int *a, T ***b, T **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **a_tmp = (T **)malloc(sizeof(T *) * size);
    for (int i = 0; i < size; i++) {
        a_tmp[i] = (T *)malloc(sizeof(T) * numShares);
        memset(a_tmp[i], 0, sizeof(T) * numShares);
        ss->sparsify_public(a_tmp[i], a[i]);
    }
    ss_batch(a_tmp, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&a_tmp, size);
}

// int*, T*** -> T*** (public, 2D private -> 2D private)
template <typename T>
void ss_batch(int *a, T ***b, T ***result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **a_tmp = (T **)malloc(sizeof(T *) * size);
    for (int i = 0; i < size; i++) {
        a_tmp[i] = (T *)malloc(sizeof(T) * numShares);
        memset(a_tmp[i], 0, sizeof(T) * numShares);
        ss->sparsify_public(a_tmp[i], a[i]);
    }
    ss_batch(a_tmp, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&a_tmp, size);
}

// T***, int* -> T** (2D private, public -> 1D private)
template <typename T>
void ss_batch(T ***a, int *b, T **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    std::string op_trimmed = op;
    op_trimmed.erase(std::remove(op_trimmed.begin(), op_trimmed.end(), ' '), op_trimmed.end());
    bool is_raw = (op_trimmed == ">>" || op_trimmed == "<<" || op_trimmed == "/");
    T **b_tmp = (T **)malloc(sizeof(T *) * size);
    for (int i = 0; i < size; i++) {
        b_tmp[i] = (T *)malloc(sizeof(T) * numShares);
        memset(b_tmp[i], 0, sizeof(T) * numShares);
        if (is_raw)
            b_tmp[i][0] = (T)b[i];
        else
            ss->sparsify_public(b_tmp[i], b[i]);
    }
    if (op_trimmed == "/") op = "/P";
    ss_batch(a, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&b_tmp, size);
}

// T***, int* -> T*** (2D private, public -> 2D private)
template <typename T>
void ss_batch(T ***a, int *b, T ***result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    std::string op_trimmed = op;
    op_trimmed.erase(std::remove(op_trimmed.begin(), op_trimmed.end(), ' '), op_trimmed.end());
    bool is_raw = (op_trimmed == ">>" || op_trimmed == "<<" || op_trimmed == "/");
    T **b_tmp = (T **)malloc(sizeof(T *) * size);
    for (int i = 0; i < size; i++) {
        b_tmp[i] = (T *)malloc(sizeof(T) * numShares);
        memset(b_tmp[i], 0, sizeof(T) * numShares);
        if (is_raw)
            b_tmp[i][0] = (T)b[i];
        else
            ss->sparsify_public(b_tmp[i], b[i]);
    }
    if (op_trimmed == "/") op = "/P";
    ss_batch(a, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&b_tmp, size);
}

// T**, T*** -> T*** (1D private, 2D private -> 2D private)
template <typename T>
void ss_batch(T **a, T ***b, T ***result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **a_tmp, **b_tmp, **result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2, numShares);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);
    ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, ss);
    rss_batch_writeback_2d(result, result_tmp, index_array, resultdim, size, numShares);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// T***, T** -> T*** (2D private, 1D private -> 2D private)
template <typename T>
void ss_batch(T ***a, T **b, T ***result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **a_tmp, **b_tmp, **result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2, numShares);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);
    ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, ss);
    rss_batch_writeback_2d(result, result_tmp, index_array, resultdim, size, numShares);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// T***, T*** -> T** (2D private, 2D private -> 1D private)
template <typename T>
void ss_batch(T ***a, T ***b, T **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    if (op == "@") {
        T **result_tmp = (T **)malloc(sizeof(T *) * size);
        T **result_org = (T **)malloc(sizeof(T *) * size);
        for (int i = 0; i < size; i++) {
            result_tmp[i] = (T *)malloc(sizeof(T) * numShares);
            result_org[i] = (T *)malloc(sizeof(T) * numShares);
            memset(result_tmp[i], 0, sizeof(T) * numShares);
            for (uint s = 0; s < numShares; s++)
                result_org[i][s] = result[index_array[3 * i + 2]][s];
        }
        ss_batch_dot(a, b, size, adim, index_array, result_tmp, threadID, net, ss);
        ss_batch_handle_priv_cond(result_tmp, result_org, out_cond, priv_cond, counter, size, threadID, net, ss);
        rss_batch_writeback_1d(result, result_tmp, index_array, size, numShares);
        ss_batch_free_operator(&result_org, size);
        ss_batch_free_operator(&result_tmp, size);
        return;
    }
    T **a_tmp, **b_tmp, **result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2, numShares);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);
    ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, ss);
    rss_batch_writeback_1d(result, result_tmp, index_array, size, numShares);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// T**, T** -> T*** (1D private, 1D private -> 2D private)
template <typename T>
void ss_batch(T **a, T **b, T ***result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **a_tmp, **b_tmp, **result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2, numShares);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);
    ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, ss);
    rss_batch_writeback_2d(result, result_tmp, index_array, resultdim, size, numShares);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// T**, T*** -> T** (1D private, 2D private -> 1D private)
template <typename T>
void ss_batch(T **a, T ***b, T **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **a_tmp, **b_tmp, **result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2, numShares);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);
    ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, ss);
    rss_batch_writeback_1d(result, result_tmp, index_array, size, numShares);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// T***, T** -> T** (2D private, 1D private -> 1D private)
template <typename T>
void ss_batch(T ***a, T **b, T **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **a_tmp, **b_tmp, **result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2, numShares);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);
    ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, ss);
    rss_batch_writeback_1d(result, result_tmp, index_array, size, numShares);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// T***, T*** -> T*** (2D private, 2D private -> 2D private)
template <typename T>
void ss_batch(T ***a, T ***b, T ***result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    if (op == "@") {
        T **result_tmp = (T **)malloc(sizeof(T *) * size);
        T **result_org = (T **)malloc(sizeof(T *) * size);
        for (int i = 0; i < size; i++) {
            result_tmp[i] = (T *)malloc(sizeof(T) * numShares);
            result_org[i] = (T *)malloc(sizeof(T) * numShares);
            memset(result_tmp[i], 0, sizeof(T) * numShares);
            if (resultdim != 0) {
                int dim1 = index_array[3 * i + 2] / resultdim;
                int dim2 = index_array[3 * i + 2] % resultdim;
                for (uint s = 0; s < numShares; s++)
                    result_org[i][s] = result[dim1][dim2][s];
            }
        }
        ss_batch_dot(a, b, size, adim, index_array, result_tmp, threadID, net, ss);
        ss_batch_handle_priv_cond(result_tmp, result_org, out_cond, priv_cond, counter, size, threadID, net, ss);
        rss_batch_writeback_2d(result, result_tmp, index_array, resultdim, size, numShares);
        ss_batch_free_operator(&result_org, size);
        ss_batch_free_operator(&result_tmp, size);
        return;
    }
    T **a_tmp, **b_tmp, **result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2, numShares);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);
    ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, ss);
    rss_batch_writeback_2d(result, result_tmp, index_array, resultdim, size, numShares);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// Batch dot product
template <typename T>
void ss_batch_dot(T ***a, T ***b, int size, int array_size, int *index_array, T **result, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    int a_dim = 0, b_dim = 0;
    // Extract indexed rows: a_tmp[size][array_size][numShares], b_tmp[size][array_size][numShares]
    T ***a_tmp = (T ***)malloc(sizeof(T **) * size);
    T ***b_tmp = (T ***)malloc(sizeof(T **) * size);
    for (int i = 0; i < size; i++) {
        a_tmp[i] = (T **)malloc(sizeof(T *) * array_size);
        b_tmp[i] = (T **)malloc(sizeof(T *) * array_size);
        a_dim = index_array[3 * i];
        b_dim = index_array[3 * i + 1];
        for (int j = 0; j < array_size; j++) {
            a_tmp[i][j] = (T *)malloc(sizeof(T) * numShares);
            b_tmp[i][j] = (T *)malloc(sizeof(T) * numShares);
            for (uint s = 0; s < numShares; s++) {
                a_tmp[i][j][s] = a[a_dim][j][s];
                b_tmp[i][j][s] = b[b_dim][j][s];
            }
        }
    }

    doOperation_DotProduct(a_tmp, b_tmp, result, size, array_size, threadID, net, ss);

    // Cleanup
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < array_size; j++) {
            free(a_tmp[i][j]);
            free(b_tmp[i][j]);
        }
        free(a_tmp[i]);
        free(b_tmp[i]);
    }
    free(a_tmp);
    free(b_tmp);
}

// ============================================================================
// Float batch operations — RSS implementations
// Public-constant variants: convert to private, delegate to private-private version
// Private-private variants: extract via ss_convert_operator, call BOP dispatch, write back
// RSS type mapping: T*** = 1D float [size][4][numShares], T**** = 2D float [d1][d2][4][numShares]
// ============================================================================

// Helper: write back float result_tmp T*** into 2D float result T**** using dim decomposition
template <typename T>
static inline void rss_batch_float_writeback_2d(T ****result, T ***result_tmp, int *index_array, int resultdim, int size, uint numShares) {
    for (int i = 0; i < size; ++i) {
        int dim1 = index_array[3 * i + 2] / resultdim;
        int dim2 = index_array[3 * i + 2] % resultdim;
        for (int j = 0; j < 4; j++)
            for (uint s = 0; s < numShares; s++)
                result[dim1][dim2][j][s] = result_tmp[i][j][s];
    }
}

// Helper: write back float result_tmp T*** into 1D float result T***
template <typename T>
static inline void rss_batch_float_writeback_1d(T ***result, T ***result_tmp, int *index_array, int size, uint numShares) {
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < 4; j++)
            for (uint s = 0; s < numShares; s++)
                result[index_array[3 * i + 2]][j][s] = result_tmp[i][j][s];
}

// --- Public + 1D private float arithmetic ---

// float*, T*** -> T**** (public + 1D private -> 2D private)
template <typename T>
void ss_batch(float *a, T ***b, T ****result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    T ***atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    ss_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&atmp, size);
}

// T***, float* -> T**** (1D private + public -> 2D private)
template <typename T>
void ss_batch(T ***a, float *b, T ****result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    T ***btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    op = (op == "/") ? "/P" : op;
    ss_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&btmp, size);
}

// float*, T*** -> T*** (public + 1D private -> 1D private)
template <typename T>
void ss_batch(float *a, T ***b, T ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    T ***atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    ss_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&atmp, size);
}

// T***, float* -> T*** (1D private + public -> 1D private)
template <typename T>
void ss_batch(T ***a, float *b, T ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    T ***btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    op = (op == "/") ? "/P" : op;
    ss_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&btmp, size);
}

// --- Public-to-private float assignments ---

// float*, float* -> T**** (public assignment to 2D private float)
template <typename T>
void ss_batch(float *a, float *b, T ****result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    T ***atmp;
    ss_batch_convert_to_private_float(a, &atmp, resultlen_sig, resultlen_exp, size, ss);
    ss_batch(atmp, atmp, result, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&atmp, size);
}

// int*, int* -> T**** (public int assignment to 2D private float)
template <typename T>
void ss_batch(int *a, int *b, T ****result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    float *a1 = (float *)malloc(sizeof(float) * size);
    for (int i = 0; i < size; i++) a1[i] = a[i];
    T ***atmp;
    ss_batch_convert_to_private_float(a1, &atmp, resultlen_sig, resultlen_exp, size, ss);
    ss_batch(atmp, atmp, result, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&atmp, size);
    free(a1);
}

// float*, float* -> T*** (public assignment to 1D private float)
template <typename T>
void ss_batch(float *a, float *b, T ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    T ***atmp;
    ss_batch_convert_to_private_float(a, &atmp, resultlen_sig, resultlen_exp, size, ss);
    ss_batch(atmp, atmp, result, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&atmp, size);
}

// int*, int* -> T*** (public int assignment to 1D private float)
template <typename T>
void ss_batch(int *a, int *b, T ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    float *a1 = (float *)malloc(sizeof(float) * size);
    for (int i = 0; i < size; i++) a1[i] = a[i];
    T ***atmp;
    ss_batch_convert_to_private_float(a1, &atmp, resultlen_sig, resultlen_exp, size, ss);
    ss_batch(atmp, atmp, result, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&atmp, size);
    free(a1);
}

// --- Public + 2D private float arithmetic ---

// float*, T**** -> T**** (public + 2D private -> 2D private)
template <typename T>
void ss_batch(float *a, T ****b, T ****result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    T ***atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    ss_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&atmp, size);
}

// T****, float* -> T**** (2D private + public -> 2D private)
template <typename T>
void ss_batch(T ****a, float *b, T ****result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    T ***btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    op = (op == "/") ? "/P" : op;
    ss_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&btmp, size);
}

// float*, T**** -> T*** (public + 2D private -> 1D private)
template <typename T>
void ss_batch(float *a, T ****b, T ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    T ***atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    ss_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&atmp, size);
}

// T****, float* -> T*** (2D private + public -> 1D private)
template <typename T>
void ss_batch(T ****a, float *b, T ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    T ***btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    op = (op == "/") ? "/P" : op;
    ss_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&btmp, size);
}

// --- Public + private float comparison ---

// float*, T*** -> T** (public + 1D private -> comparison)
template <typename T>
void ss_batch(float *a, T ***b, T **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    T ***atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    ss_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&atmp, size);
}

// T***, float* -> T** (1D private + public -> comparison)
template <typename T>
void ss_batch(T ***a, float *b, T **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    T ***btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    ss_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&btmp, size);
}

// float*, T**** -> T** (public + 2D private -> comparison)
template <typename T>
void ss_batch(float *a, T ****b, T **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    T ***atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    ss_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&atmp, size);
}

// T****, float* -> T** (2D private + public -> comparison)
template <typename T>
void ss_batch(T ****a, float *b, T **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    T ***btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    ss_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&btmp, size);
}

// --- Private-private float arithmetic (core dispatch) ---

// T***, T*** -> T*** (1D float, 1D float -> 1D float arithmetic)
template <typename T>
void ss_batch(T ***a, T ***b, T ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T ***a_tmp, ***b_tmp, ***result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2, numShares);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);
    ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, ss);
    rss_batch_float_writeback_1d(result, result_tmp, index_array, size, numShares);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// T***, T**** -> T**** (1D float, 2D float -> 2D float)
template <typename T>
void ss_batch(T ***a, T ****b, T ****result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T ***a_tmp, ***b_tmp, ***result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2, numShares);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);
    ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, ss);
    rss_batch_float_writeback_2d(result, result_tmp, index_array, resultdim, size, numShares);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// T****, T*** -> T**** (2D float, 1D float -> 2D float)
template <typename T>
void ss_batch(T ****a, T ***b, T ****result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T ***a_tmp, ***b_tmp, ***result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2, numShares);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);
    ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, ss);
    rss_batch_float_writeback_2d(result, result_tmp, index_array, resultdim, size, numShares);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// T****, T**** -> T*** (2D float, 2D float -> 1D float)
template <typename T>
void ss_batch(T ****a, T ****b, T ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T ***a_tmp, ***b_tmp, ***result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2, numShares);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);
    ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, ss);
    rss_batch_float_writeback_1d(result, result_tmp, index_array, size, numShares);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// T****, T*** -> T*** (2D float, 1D float -> 1D float)
template <typename T>
void ss_batch(T ****a, T ***b, T ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T ***a_tmp, ***b_tmp, ***result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2, numShares);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);
    ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, ss);
    rss_batch_float_writeback_1d(result, result_tmp, index_array, size, numShares);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// T***, T**** -> T*** (1D float, 2D float -> 1D float)
template <typename T>
void ss_batch(T ***a, T ****b, T ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T ***a_tmp, ***b_tmp, ***result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2, numShares);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);
    ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, ss);
    rss_batch_float_writeback_1d(result, result_tmp, index_array, size, numShares);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// T***, T*** -> T**** (1D float, 1D float -> 2D float)
template <typename T>
void ss_batch(T ***a, T ***b, T ****result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T ***a_tmp, ***b_tmp, ***result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2, numShares);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);
    ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, ss);
    rss_batch_float_writeback_2d(result, result_tmp, index_array, resultdim, size, numShares);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// T****, T**** -> T**** (2D float, 2D float -> 2D float)
template <typename T>
void ss_batch(T ****a, T ****b, T ****result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T ***a_tmp, ***b_tmp, ***result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2, numShares);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);
    ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, ss);
    rss_batch_float_writeback_2d(result, result_tmp, index_array, resultdim, size, numShares);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// --- Private-private float comparison ---

// T****, T**** -> T** (2D float, 2D float -> comparison)
template <typename T>
void ss_batch(T ****a, T ****b, T **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T ***a_tmp, ***b_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2, numShares);
    ss_batch_BOP_float_comparison(result, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
}

// T***, T**** -> T** (1D float, 2D float -> comparison)
template <typename T>
void ss_batch(T ***a, T ****b, T **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T ***a_tmp, ***b_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2, numShares);
    ss_batch_BOP_float_comparison(result, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
}

// T****, T*** -> T** (2D float, 1D float -> comparison)
template <typename T>
void ss_batch(T ****a, T ***b, T **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T ***a_tmp, ***b_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2, numShares);
    ss_batch_BOP_float_comparison(result, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
}

// T***, T*** -> T** (1D float, 1D float -> comparison)
template <typename T>
void ss_batch(T ***a, T ***b, T **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T ***a_tmp, ***b_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2, numShares);
    ss_batch_BOP_float_comparison(result, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, ss);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
}

// INT2FL: public int array -> 1D private float (T***)
// Non-interactive: convert each public int to float components, apply priv_cond
template <typename T>
void ss_batch_int2fl(int *a, T ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    // Allocate result_tmp[size][4][numShares]
    T ***result_tmp = (T ***)malloc(sizeof(T **) * size);
    for (int i = 0; i < size; i++) {
        result_tmp[i] = (T **)malloc(sizeof(T *) * 4);
        for (int j = 0; j < 4; j++) {
            result_tmp[i][j] = (T *)malloc(sizeof(T) * numShares);
            memset(result_tmp[i][j], 0, sizeof(T) * numShares);
        }
    }

    // Convert each public int -> float components -> sparsify
    for (int i = 0; i < size; i++) {
        int idx = index_array[3 * i];
        T components[4];
        rss_convertFloat((float)a[idx], 32, 9, components);
        for (int c = 0; c < 4; c++)
            ss->sparsify_public(result_tmp[i][c], components[c]);
    }

    // Flatten for priv_cond handling
    T **result_flat = (T **)malloc(sizeof(T *) * 4 * size);
    T **result_org = (T **)malloc(sizeof(T *) * 4 * size);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 4; j++) {
            result_flat[4 * i + j] = (T *)malloc(sizeof(T) * numShares);
            result_org[4 * i + j] = (T *)malloc(sizeof(T) * numShares);
            for (uint s = 0; s < numShares; s++) {
                result_flat[4 * i + j][s] = result_tmp[i][j][s];
                result_org[4 * i + j][s] = result[index_array[3 * i + 2]][j][s];
            }
        }
    }

    ss_batch_handle_priv_cond(result_flat, result_org, out_cond, priv_cond, counter, 4 * size, threadID, net, ss);

    for (int i = 0; i < size; i++)
        for (int j = 0; j < 4; j++)
            for (uint s = 0; s < numShares; s++)
                result[index_array[3 * i + 2]][j][s] = result_flat[4 * i + j][s];

    ss_batch_free_operator(&result_flat, 4 * size);
    ss_batch_free_operator(&result_org, 4 * size);
    ss_batch_free_operator(&result_tmp, size);
}

// INT2FL: 1D private int (T**) -> 1D private float (T***)
template <typename T>
void ss_batch_int2fl(T **a, T ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    // Extract indexed input elements
    T **a_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);

    // Allocate result_tmp[size][4][numShares]
    T ***result_tmp = (T ***)malloc(sizeof(T **) * size);
    for (int i = 0; i < size; i++) {
        result_tmp[i] = (T **)malloc(sizeof(T *) * 4);
        for (int j = 0; j < 4; j++) {
            result_tmp[i][j] = (T *)malloc(sizeof(T) * numShares);
            memset(result_tmp[i][j], 0, sizeof(T) * numShares);
        }
    }

    // Create component-first view [4][size] aliasing into result_tmp[size][4]
    T ***result_comp = (T ***)malloc(sizeof(T **) * 4);
    for (int c = 0; c < 4; c++) {
        result_comp[c] = (T **)malloc(sizeof(T *) * size);
        for (int i = 0; i < size; i++)
            result_comp[c][i] = result_tmp[i][c];
    }

    doOperation_Int2FL(a_tmp, result_comp, alen, resultlen_sig, size, threadID, net, ss);

    for (int c = 0; c < 4; c++)
        free(result_comp[c]);
    free(result_comp);

    // Flatten for priv_cond handling
    T **result_flat = (T **)malloc(sizeof(T *) * 4 * size);
    T **result_org = (T **)malloc(sizeof(T *) * 4 * size);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 4; j++) {
            result_flat[4 * i + j] = (T *)malloc(sizeof(T) * numShares);
            result_org[4 * i + j] = (T *)malloc(sizeof(T) * numShares);
            for (uint s = 0; s < numShares; s++) {
                result_flat[4 * i + j][s] = result_tmp[i][j][s];
                result_org[4 * i + j][s] = result[index_array[3 * i + 2]][j][s];
            }
        }
    }

    ss_batch_handle_priv_cond(result_flat, result_org, out_cond, priv_cond, counter, 4 * size, threadID, net, ss);

    for (int i = 0; i < size; i++)
        for (int j = 0; j < 4; j++)
            for (uint s = 0; s < numShares; s++)
                result[index_array[3 * i + 2]][j][s] = result_flat[4 * i + j][s];

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&result_flat, 4 * size);
    ss_batch_free_operator(&result_org, 4 * size);
    ss_batch_free_operator(&result_tmp, size);
}

// INT2FL: public int array -> 2D private float (T****)
template <typename T>
void ss_batch_int2fl(int *a, T ****result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    // Extract 1D float result_tmp from 2D result
    T ***result_tmp;
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);
    int *result_index_array = (int *)malloc(sizeof(int) * size);
    for (int i = 0; i < size; i++) {
        result_index_array[i] = index_array[3 * i + 2];
        index_array[3 * i + 2] = i;
    }
    // Delegate to 1D version
    ss_batch_int2fl(a, result_tmp, adim, size, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
    // Write back to 2D
    for (int i = 0; i < size; i++) {
        int dim1 = result_index_array[i] / resultdim;
        int dim2 = result_index_array[i] % resultdim;
        for (int j = 0; j < 4; j++)
            for (uint s = 0; s < numShares; s++)
                result[dim1][dim2][j][s] = result_tmp[i][j][s];
    }
    free(result_index_array);
    ss_batch_free_operator(&result_tmp, size);
}

// INT2FL: 1D private int (T**) -> 2D private float (T****)
template <typename T>
void ss_batch_int2fl(T **a, T ****result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T ***result_tmp;
    T **a_tmp;
    int *result_index_array = (int *)malloc(sizeof(int) * size);
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);

    for (int i = 0; i < size; i++) {
        result_index_array[i] = index_array[3 * i + 2];
        index_array[3 * i] = i;
        index_array[3 * i + 2] = i;
    }
    ss_batch_int2fl(a_tmp, result_tmp, size, size, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);

    for (int i = 0; i < size; i++) {
        int dim1 = result_index_array[i] / resultdim;
        int dim2 = result_index_array[i] % resultdim;
        for (int j = 0; j < 4; j++)
            for (uint s = 0; s < numShares; s++)
                result[dim1][dim2][j][s] = result_tmp[i][j][s];
    }
    free(result_index_array);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// INT2FL: 2D private int (T***) -> 1D private float (T***)
template <typename T>
void ss_batch_int2fl(T ***a, T ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **a_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    for (int i = 0; i < size; i++)
        index_array[3 * i] = i;
    ss_batch_int2fl(a_tmp, result, size, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
    ss_batch_free_operator(&a_tmp, size);
}

// INT2FL: 2D private int (T***) -> 2D private float (T****)
template <typename T>
void ss_batch_int2fl(T ***a, T ****result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T ***result_tmp;
    T **a_tmp;
    int *result_index_array = (int *)malloc(sizeof(int) * size);
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);

    for (int i = 0; i < size; i++) {
        result_index_array[i] = index_array[3 * i + 2];
        index_array[3 * i] = i;
        index_array[3 * i + 2] = i;
    }
    ss_batch_int2fl(a_tmp, result_tmp, size, size, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);

    for (int i = 0; i < size; i++) {
        int dim1 = result_index_array[i] / resultdim;
        int dim2 = result_index_array[i] % resultdim;
        for (int j = 0; j < 4; j++)
            for (uint s = 0; s < numShares; s++)
                result[dim1][dim2][j][s] = result_tmp[i][j][s];
    }
    free(result_index_array);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// FL2INT: public float array -> 1D private int (T**)
// Non-interactive: just truncate each float to int and sparsify
template <typename T>
void ss_batch_fl2int(float *a, T **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    for (int i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++)
            result[index_array[3 * i + 2]][s] = 0;
        ss->sparsify_public(result[index_array[3 * i + 2]], (T)((int)a[i]));
    }
}

// FL2INT: public float array -> 2D private int (T***)
template <typename T>
void ss_batch_fl2int(float *a, T ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    for (int i = 0; i < size; i++) {
        int dim1 = index_array[3 * i + 2] / resultdim;
        int dim2 = index_array[3 * i + 2] % resultdim;
        for (uint s = 0; s < numShares; s++)
            result[dim1][dim2][s] = 0;
        ss->sparsify_public(result[dim1][dim2], (T)((int)a[i]));
    }
}

// FL2INT: 1D private float (T***) -> 1D private int (T**)
template <typename T>
void ss_batch_fl2int(T ***a, T **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T ***a_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);

    T **result_tmp = (T **)malloc(sizeof(T *) * size);
    for (int i = 0; i < size; i++) {
        result_tmp[i] = (T *)malloc(sizeof(T) * numShares);
        memset(result_tmp[i], 0, sizeof(T) * numShares);
    }

    // Create component-first view [4][size] aliasing into a_tmp[size][4]
    T ***a_comp = (T ***)malloc(sizeof(T **) * 4);
    for (int c = 0; c < 4; c++) {
        a_comp[c] = (T **)malloc(sizeof(T *) * size);
        for (int i = 0; i < size; i++)
            a_comp[c][i] = a_tmp[i][c];
    }

    doOperation_FL2Int(a_comp, result_tmp, alen_sig, alen_exp, blen, size, threadID, net, ss);

    for (int c = 0; c < 4; c++)
        free(a_comp[c]);
    free(a_comp);

    // Handle private conditions
    T **result_org = (T **)malloc(sizeof(T *) * size);
    for (int i = 0; i < size; i++) {
        result_org[i] = (T *)malloc(sizeof(T) * numShares);
        for (uint s = 0; s < numShares; s++)
            result_org[i][s] = result[index_array[3 * i + 2]][s];
    }
    ss_batch_handle_priv_cond(result_tmp, result_org, out_cond, priv_cond, counter, size, threadID, net, ss);

    for (int i = 0; i < size; i++)
        for (uint s = 0; s < numShares; s++)
            result[index_array[3 * i + 2]][s] = result_tmp[i][s];

    ss_batch_free_operator(&result_tmp, size);
    ss_batch_free_operator(&result_org, size);
    ss_batch_free_operator(&a_tmp, size);
}

// FL2INT: 1D private float (T***) -> 2D private int (T***)
template <typename T>
void ss_batch_fl2int(T ***a, T ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **result_tmp;
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);
    int *result_index_array = (int *)malloc(sizeof(int) * size);
    for (int i = 0; i < size; i++) {
        result_index_array[i] = index_array[3 * i + 2];
        index_array[3 * i + 2] = i;
    }
    ss_batch_fl2int(a, result_tmp, adim, size, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
    for (int i = 0; i < size; i++) {
        int dim1 = result_index_array[i] / resultdim;
        int dim2 = result_index_array[i] % resultdim;
        for (uint s = 0; s < numShares; s++)
            result[dim1][dim2][s] = result_tmp[i][s];
    }
    free(result_index_array);
    ss_batch_free_operator(&result_tmp, size);
}

// FL2INT: 2D private float (T****) -> 1D private int (T**)
template <typename T>
void ss_batch_fl2int(T ****a, T **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T ***a_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    for (int i = 0; i < size; i++)
        index_array[3 * i] = i;
    ss_batch_fl2int(a_tmp, result, size, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
    ss_batch_free_operator(&a_tmp, size);
}

// FL2INT: 2D private float (T****) -> 2D private int (T***)
template <typename T>
void ss_batch_fl2int(T ****a, T ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T ***a_tmp;
    T **result_tmp;
    int *result_index_array = (int *)malloc(sizeof(int) * size);
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);

    for (int i = 0; i < size; i++) {
        result_index_array[i] = index_array[3 * i + 2];
        index_array[3 * i] = i;
        index_array[3 * i + 2] = i;
    }
    ss_batch_fl2int(a_tmp, result_tmp, size, size, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);

    for (int i = 0; i < size; i++) {
        int dim1 = result_index_array[i] / resultdim;
        int dim2 = result_index_array[i] % resultdim;
        for (uint s = 0; s < numShares; s++)
            result[dim1][dim2][s] = result_tmp[i][s];
    }
    free(result_index_array);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// INT2INT: public int array -> 1D private int (T**)
// Convert public ints to shares, then delegate
template <typename T>
void ss_batch_int2int(int *a, T **result, int adim, int resultdim, int alen, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **a_tmp = (T **)malloc(sizeof(T *) * size);
    for (int i = 0; i < size; i++) {
        a_tmp[i] = (T *)malloc(sizeof(T) * numShares);
        memset(a_tmp[i], 0, sizeof(T) * numShares);
        ss->sparsify_public(a_tmp[i], (T)a[i]);
    }
    ss_batch_int2int(a_tmp, result, size, resultdim, 32, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
    ss_batch_free_operator(&a_tmp, size);
}

// INT2INT: public int array -> 2D private int (T***)
template <typename T>
void ss_batch_int2int(int *a, T ***result, int adim, int resultdim, int alen, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **a_tmp = (T **)malloc(sizeof(T *) * size);
    for (int i = 0; i < size; i++) {
        a_tmp[i] = (T *)malloc(sizeof(T) * numShares);
        memset(a_tmp[i], 0, sizeof(T) * numShares);
        ss->sparsify_public(a_tmp[i], (T)a[i]);
    }
    ss_batch_int2int(a_tmp, result, size, resultdim, 32, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
    ss_batch_free_operator(&a_tmp, size);
}

// INT2INT: 1D private int (T**) -> 1D private int (T**)
// Core: copy indexed elements with priv_cond handling
template <typename T>
void ss_batch_int2int(T **a, T **result, int adim, int resultdim, int alen, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **result_tmp = (T **)malloc(sizeof(T *) * size);
    T **result_org = (T **)malloc(sizeof(T *) * size);
    for (int i = 0; i < size; i++) {
        result_tmp[i] = (T *)malloc(sizeof(T) * numShares);
        result_org[i] = (T *)malloc(sizeof(T) * numShares);
        for (uint s = 0; s < numShares; s++) {
            result_tmp[i][s] = a[index_array[3 * i]][s];
            result_org[i][s] = result[index_array[3 * i + 2]][s];
        }
    }
    ss_batch_handle_priv_cond(result_tmp, result_org, out_cond, priv_cond, counter, size, threadID, net, ss);
    for (int i = 0; i < size; i++)
        for (uint s = 0; s < numShares; s++)
            result[index_array[3 * i + 2]][s] = result_tmp[i][s];
    ss_batch_free_operator(&result_tmp, size);
    ss_batch_free_operator(&result_org, size);
}

// INT2INT: 2D private int (T***) -> 1D private int (T**)
template <typename T>
void ss_batch_int2int(T ***a, T **result, int adim, int resultdim, int alen, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **a_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    for (int i = 0; i < size; i++)
        index_array[3 * i] = i;
    ss_batch_int2int(a_tmp, result, size, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
    ss_batch_free_operator(&a_tmp, size);
}

// INT2INT: 1D private int (T**) -> 2D private int (T***)
template <typename T>
void ss_batch_int2int(T **a, T ***result, int adim, int resultdim, int alen, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **result_tmp;
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);
    int *result_index_array = (int *)malloc(sizeof(int) * size);
    for (int i = 0; i < size; i++) {
        result_index_array[i] = index_array[3 * i + 2];
        index_array[3 * i + 2] = i;
    }
    ss_batch_int2int(a, result_tmp, adim, size, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
    for (int i = 0; i < size; i++) {
        int dim1 = result_index_array[i] / resultdim;
        int dim2 = result_index_array[i] % resultdim;
        for (uint s = 0; s < numShares; s++)
            result[dim1][dim2][s] = result_tmp[i][s];
    }
    free(result_index_array);
    ss_batch_free_operator(&result_tmp, size);
}

// INT2INT: 2D private int (T***) -> 2D private int (T***)
template <typename T>
void ss_batch_int2int(T ***a, T ***result, int adim, int resultdim, int alen, int blen, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T **result_tmp, **a_tmp;
    int *result_index_array = (int *)malloc(sizeof(int) * size);
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);

    for (int i = 0; i < size; i++) {
        result_index_array[i] = index_array[3 * i + 2];
        index_array[3 * i] = i;
        index_array[3 * i + 2] = i;
    }
    ss_batch_int2int(a_tmp, result_tmp, size, size, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);

    for (int i = 0; i < size; i++) {
        int dim1 = result_index_array[i] / resultdim;
        int dim2 = result_index_array[i] % resultdim;
        for (uint s = 0; s < numShares; s++)
            result[dim1][dim2][s] = result_tmp[i][s];
    }
    free(result_index_array);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// FL2FL: public float array -> 1D private float (T***)
// Convert public floats to private, then delegate
template <typename T>
void ss_batch_fl2fl(float *a, T ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    T ***a_tmp;
    ss_batch_convert_to_private_float(a, &a_tmp, 32, 9, size, ss);
    ss_batch_fl2fl(a_tmp, result, size, resultdim, 32, 9, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
    ss_batch_free_operator(&a_tmp, size);
}

// FL2FL: public float array -> 2D private float (T****)
template <typename T>
void ss_batch_fl2fl(float *a, T ****result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    T ***a_tmp;
    ss_batch_convert_to_private_float(a, &a_tmp, 32, 9, size, ss);
    ss_batch_fl2fl(a_tmp, result, size, resultdim, 32, 9, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
    ss_batch_free_operator(&a_tmp, size);
}

// FL2FL: 1D private float (T***) -> 1D private float (T***)
// Core: truncate/extend significand, adjust exponent, copy z and s
template <typename T>
void ss_batch_fl2fl(T ***a, T ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    // Extract a[index][component][share] -> component-major a_comp[4][size][numShares]
    T **a_comp[4];
    T **result_comp[4];
    for (int c = 0; c < 4; c++) {
        a_comp[c] = (T **)malloc(sizeof(T *) * size);
        result_comp[c] = (T **)malloc(sizeof(T *) * size);
        for (int i = 0; i < size; i++) {
            a_comp[c][i] = (T *)malloc(sizeof(T) * numShares);
            result_comp[c][i] = (T *)malloc(sizeof(T) * numShares);
            for (uint s = 0; s < numShares; s++) {
                a_comp[c][i][s] = a[index_array[3 * i]][c][s];
                result_comp[c][i][s] = 0;
            }
        }
    }

    // Significand: truncate or extend
    if (alen_sig >= blen_sig)
        doOperation_Trunc(result_comp[0], a_comp[0], alen_sig, alen_sig - blen_sig, size, threadID, net, ss);
    else {
        // Extend: multiply by 2^(blen_sig - alen_sig)
        T power = T(1) << (blen_sig - alen_sig);
        for (int i = 0; i < size; i++)
            for (uint s = 0; s < numShares; s++)
                result_comp[0][i][s] = a_comp[0][i][s] * power;
    }

    // Exponent: add (alen_sig - blen_sig) to adjust for significand change
    ss->modAdd(result_comp[1], a_comp[1], (long)(alen_sig - blen_sig), size);

    // z and s: copy directly
    for (int i = 0; i < size; i++)
        for (uint s = 0; s < numShares; s++) {
            result_comp[2][i][s] = a_comp[2][i][s];
            result_comp[3][i][s] = a_comp[3][i][s];
        }

    // Flatten for priv_cond handling
    T **result_flat = (T **)malloc(sizeof(T *) * 4 * size);
    T **result_org = (T **)malloc(sizeof(T *) * 4 * size);
    for (int i = 0; i < size; i++) {
        for (int c = 0; c < 4; c++) {
            result_flat[4 * i + c] = (T *)malloc(sizeof(T) * numShares);
            result_org[4 * i + c] = (T *)malloc(sizeof(T) * numShares);
            for (uint s = 0; s < numShares; s++) {
                result_flat[4 * i + c][s] = result_comp[c][i][s];
                result_org[4 * i + c][s] = result[index_array[3 * i + 2]][c][s];
            }
        }
    }

    ss_batch_handle_priv_cond(result_flat, result_org, out_cond, priv_cond, counter, 4 * size, threadID, net, ss);

    for (int i = 0; i < size; i++)
        for (int c = 0; c < 4; c++)
            for (uint s = 0; s < numShares; s++)
                result[index_array[3 * i + 2]][c][s] = result_flat[4 * i + c][s];

    // Free memory
    for (int c = 0; c < 4; c++) {
        ss_batch_free_operator(&a_comp[c], size);
        ss_batch_free_operator(&result_comp[c], size);
    }
    ss_batch_free_operator(&result_flat, 4 * size);
    ss_batch_free_operator(&result_org, 4 * size);
}

// FL2FL: 1D private float (T***) -> 2D private float (T****)
template <typename T>
void ss_batch_fl2fl(T ***a, T ****result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T ***result_tmp;
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);
    int *result_index_array = (int *)malloc(sizeof(int) * size);
    for (int i = 0; i < size; i++) {
        result_index_array[i] = index_array[3 * i + 2];
        index_array[3 * i + 2] = i;
    }
    ss_batch_fl2fl(a, result_tmp, adim, size, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
    for (int i = 0; i < size; i++) {
        int dim1 = result_index_array[i] / resultdim;
        int dim2 = result_index_array[i] % resultdim;
        for (int c = 0; c < 4; c++)
            for (uint s = 0; s < numShares; s++)
                result[dim1][dim2][c][s] = result_tmp[i][c][s];
    }
    free(result_index_array);
    ss_batch_free_operator(&result_tmp, size);
}

// FL2FL: 2D private float (T****) -> 1D private float (T***)
template <typename T>
void ss_batch_fl2fl(T ****a, T ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T ***a_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    for (int i = 0; i < size; i++)
        index_array[3 * i] = i;
    ss_batch_fl2fl(a_tmp, result, size, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);
    ss_batch_free_operator(&a_tmp, size);
}

// FL2FL: 2D private float (T****) -> 2D private float (T****)
template <typename T>
void ss_batch_fl2fl(T ****a, T ****result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T ***result_tmp, ***a_tmp;
    int *result_index_array = (int *)malloc(sizeof(int) * size);
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1, numShares);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3, numShares);

    for (int i = 0; i < size; i++) {
        result_index_array[i] = index_array[3 * i + 2];
        index_array[3 * i] = i;
        index_array[3 * i + 2] = i;
    }
    ss_batch_fl2fl(a_tmp, result_tmp, size, size, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, ss);

    for (int i = 0; i < size; i++) {
        int dim1 = result_index_array[i] / resultdim;
        int dim2 = result_index_array[i] % resultdim;
        for (int c = 0; c < 4; c++)
            for (uint s = 0; s < numShares; s++)
                result[dim1][dim2][c][s] = result_tmp[i][c][s];
    }
    free(result_index_array);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// IntDiv_Pub overload for T** divisor (b holds raw public values at index 0)
// Called from ss_batch_BOP_int as: doOperation_IntDiv_Pub(result, a, b, resultlen, size, threadID, net, ss)
// The raw int values are stored at b[i][0] by the batch overload (same convention as shifts)
template <typename T>
void doOperation_IntDiv_Pub(T **result, T **a, T **b, int bitlength, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    int *b_int = new int[size];
    for (int i = 0; i < size; i++)
        b_int[i] = (int)b[i][0];
    doOperation_IntDiv_Pub(result, a, b_int, bitlength, size, threadID, net, ss);
    delete[] b_int;
}

// Right shift: result = a >> b (division by 2^b)
// blen == -1 means b is public, blen != -1 means b is private
template <typename T>
void ss_shr(T **a, T **b, int alen, int blen, T **result, int resultlen, int size, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    if (blen == -1) {
        // Public shift amount: b contains public values
        // Convert b to int array and call Trunc
        // For public values in RSS, the value is stored in b[i][0]
        int *b_tmp = new int[size];
        for (int i = 0; i < size; i++) {
            b_tmp[i] = (int)b[i][0];
        }
        // Use Trunc with varying m values per element
        // Since doOperation_Trunc with int* m is not implemented,
        // we process each element individually
        for (int i = 0; i < size; i++) {
            doOperation_Trunc(result[i], a[i], alen, b_tmp[i], 1, threadID, net, ss);
        }
        delete[] b_tmp;
    } else {
        // Private shift amount: use TruncS protocol
        doOperation_TruncS(result, a, alen, b, size, threadID, net, ss);
    }
}

// Left shift: result = a << b (multiplication by 2^b)
// blen == -1 means b is public, blen != -1 means b is private
template <typename T>
void ss_shl(T **a, T **b, int alen, int blen, T **result, int resultlen, int size, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    if (blen == -1) {
        // Public shift amount: b contains public values
        // Compute 2^b[i] and multiply with a[i]
        for (int i = 0; i < size; i++) {
            T shift_amount = (T)b[i][0];
            T power = T(1) << shift_amount;
            for (uint s = 0; s < numShares; s++) {
                result[i][s] = a[i][s] * power;
            }
        }
    } else {
        // Private shift amount: use Pow2 protocol then multiply
        // Pow2 computes [2^b] from [b]
        doOperation_Pow2(result, b, blen, size, threadID, net, ss);
        // Multiply a * 2^b
        Mult(result, result, a, size, threadID, net, ss);
    }
}

// ============================================================================
// Batch operation helper functions (for Batch.hpp)
// ============================================================================

// Simple ss_set for copying one priv_int to another
// For RSS: priv_int is T* (pointer to share array of size numShares)
// Convention: ss_set(dest, source, numShares) — matches mpz_set(dest, source)
template <typename T>
inline void ss_set(T *result, T *a, uint numShares) {
    for (uint i = 0; i < numShares; i++)
        result[i] = a[i];
}

// Free batch operator array
template <typename T>
inline void ss_batch_free_operator(T ***op, int size) {
    // For RSS batch arrays: op is T*** where T=uint64_t or uint32_t
    // *op is T** (array of pointers to share arrays)
    // (*op)[i] is T* (pointer to share array for element i)
    for (int i = 0; i < size; i++) {
        if ((*op)[i]) {
            free((*op)[i]);  // Free each share array
        }
    }
    free(*op);  // Free the array of pointers
}

// Free batch operator array (4-star version for 2D float arrays)
template <typename T>
inline void ss_batch_free_operator(T ****op, int size) {
    // For 2D: each (*op)[i] is T** (array of pointers)
    for (int i = 0; i < size; i++) {
        if ((*op)[i]) {
            // Need to free the inner arrays first
            for (int j = 0; j < 4; j++) {  // RSS floats have 4 components
                if ((*op)[i][j]) {
                    free((*op)[i][j]);
                }
            }
            free((*op)[i]);
        }
    }
    free(*op);
}

// Convert indexed batch operands (main version for integer/scalar types)
template <typename T>
inline void ss_convert_operator(T ***result, T ***op, int *index_array, int dim, int size, int flag, uint numShares) {
    // Allocate array of pointers for batch elements
    *result = (T **)malloc(sizeof(T *) * size);

    int dim1, dim2;
    for (int i = 0; i < size; i++) {
        // Allocate share array for this element
        (*result)[i] = (T *)malloc(sizeof(T) * numShares);

        // Determine which element to copy from source array based on flag and index_array
        if (flag == 1) {
            // First operand (a)
            if (dim != 0) {
                dim1 = index_array[3 * i] / dim;
                dim2 = index_array[3 * i] % dim;
                for (uint s = 0; s < numShares; s++)
                    (*result)[i][s] = op[dim1][dim2][s];
            } else {
                for (uint s = 0; s < numShares; s++)
                    (*result)[i][s] = op[0][index_array[3 * i]][s];
            }
        } else if (flag == 2) {
            // Second operand (b)
            if (dim != 0 && dim != -1) {
                dim1 = index_array[3 * i + 1] / dim;
                dim2 = index_array[3 * i + 1] % dim;
                for (uint s = 0; s < numShares; s++)
                    (*result)[i][s] = op[dim1][dim2][s];
            } else if (dim == -1) {
                for (uint s = 0; s < numShares; s++)
                    (*result)[i][s] = op[0][i][s];
            } else {
                for (uint s = 0; s < numShares; s++)
                    (*result)[i][s] = op[0][index_array[3 * i + 1]][s];
            }
        } else if (flag == 3) {
            // Result operand
            if (dim != 0) {
                dim1 = index_array[3 * i + 2] / dim;
                dim2 = index_array[3 * i + 2] % dim;
                for (uint s = 0; s < numShares; s++)
                    (*result)[i][s] = op[dim1][dim2][s];
            } else {
                for (uint s = 0; s < numShares; s++)
                    (*result)[i][s] = op[0][index_array[3 * i + 2]][s];
            }
        }
    }
}

// Convert indexed batch operands (T** version - wrapper)
template <typename T>
inline void ss_convert_operator(T ***result, T **op, int *index_array, int dim, int size, int flag, uint numShares) {
    // Wrap single T** into T*** for the main function
    T ***ops = (T ***)malloc(sizeof(T **));
    *ops = op;
    ss_convert_operator(result, ops, index_array, dim, size, flag, numShares);
    free(ops);
}

// Convert indexed batch operands (4-star version for 2D float arrays)
template <typename T>
inline void ss_convert_operator(T ****result, T ***op, int *index_array, int dim, int size, int flag, uint numShares) {
    T ****ops = NULL;
    if (op != NULL) {
        ops = (T ****)malloc(sizeof(T ***));
        *ops = op;
    }
    ss_convert_operator(result, ops, index_array, dim, size, flag, numShares);
    if (op != NULL)
        free(ops);
}

// Convert indexed batch operands (5-star version for 2D float arrays)
template <typename T>
inline void ss_convert_operator(T ****result, T ****op, int *index_array, int dim, int size, int flag, uint numShares) {
    // Allocate array of T*** for batch elements
    *result = (T ***)malloc(sizeof(T **) * size);

    int dim1, dim2;
    for (int i = 0; i < size; i++) {
        // Allocate T** for this element (4 components for RSS floats)
        (*result)[i] = (T **)malloc(sizeof(T *) * 4);
        for (int j = 0; j < 4; j++) {
            (*result)[i][j] = (T *)malloc(sizeof(T) * numShares);
        }

        // Copy values based on flag and index_array
        for (int j = 0; j < 4; j++) {
            if (flag == 1) {
                if (dim != 0) {
                    dim1 = index_array[3 * i] / dim;
                    dim2 = index_array[3 * i] % dim;
                    for (uint s = 0; s < numShares; s++)
                        (*result)[i][j][s] = op[dim1][dim2][j][s];
                } else {
                    for (uint s = 0; s < numShares; s++)
                        (*result)[i][j][s] = op[0][i][j][s];
                }
            } else if (flag == 2) {
                if (dim != 0 && dim != -1) {
                    dim1 = index_array[3 * i + 1] / dim;
                    dim2 = index_array[3 * i + 1] % dim;
                    for (uint s = 0; s < numShares; s++)
                        (*result)[i][j][s] = op[dim1][dim2][j][s];
                } else if (dim == -1) {
                    for (uint s = 0; s < numShares; s++)
                        (*result)[i][j][s] = op[0][i][j][s];
                } else {
                    for (uint s = 0; s < numShares; s++)
                        (*result)[i][j][s] = op[0][index_array[3 * i + 1]][j][s];
                }
            } else if (flag == 3) {
                if (dim != 0) {
                    dim1 = index_array[3 * i + 2] / dim;
                    dim2 = index_array[3 * i + 2] % dim;
                    for (uint s = 0; s < numShares; s++)
                        (*result)[i][j][s] = op[dim1][dim2][j][s];
                } else {
                    for (uint s = 0; s < numShares; s++)
                        (*result)[i][j][s] = op[0][index_array[3 * i + 2]][j][s];
                }
            }
        }
    }
}

// Wrapper for Mult for SINGLE VALUES only
// Called from smc_mult(priv_int &a, priv_int &b, priv_int &result, ...)
// where priv_int = uint64_t* (pointer to share array)
// For single value: a, b, result are each uint64_t* with [numShares] elements
// Mult now expects T** with layout [size][numShares] (interface format)
template <typename T, typename SS_TYPE>
void Mult_Single(T *result, T *a, T *b, int threadID, NodeNetwork net, SS_TYPE *ss) {
    int size = 1;  // Single value
    int numShares = ss->getNumShares();

    // Create arrays with interface layout [size][numShares]
    // A[i][s] = share s of element i
    T **A = new T*[size];
    T **B = new T*[size];
    T **C = new T*[size];

    for (int i = 0; i < size; i++) {
        A[i] = new T[numShares];
        B[i] = new T[numShares];
        C[i] = new T[numShares];
    }

    // Copy input: a[s] (share s) goes to A[i][s]
    for (int i = 0; i < size; i++) {
        for (int s = 0; s < numShares; s++) {
            A[i][s] = a[s];  // a[s] is share s
            B[i][s] = b[s];  // b[s] is share s
        }
    }

    // Call RSS multiplication (now uses interface format)
    Mult(C, A, B, size, threadID, net, ss);

    // Copy result: C[0][s] (share s of element 0) goes to result[s]
    for (int s = 0; s < numShares; s++) {
        result[s] = C[0][s];
    }

    // Cleanup
    for (int i = 0; i < size; i++) {
        delete[] A[i];
        delete[] B[i];
        delete[] C[i];
    }
    delete[] A;
    delete[] B;
    delete[] C;
}

#endif
