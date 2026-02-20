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
    return;
}

template <typename T>
void ss_batch_convert_to_private_float(float *a, T ****priv_a, int len_sig, int len_exp, int size, replicatedSecretShare<T> *ss) {
    return;
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

// Batch float comparison: private float array vs private float array (STUB - not yet implemented)
// Input format:  a[size][4][numShares], b[size][4][numShares]
// Output format: result[size][numShares] - array of comparison results (1 or 0)
// For RSS: numShares=2 (two shares per party for 3-party computation)
template <typename T>
void ss_batch_fop_comparison(T **result, T ***a, T ***b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

// Batch float arithmetic: private float array op private float array (STUB - not yet implemented)
// Would perform element-wise operations (e.g., +, *, -) on arrays of floats
// Input format:  a[size][4][numShares], b[size][4][numShares]
// Output format: result[size][4][numShares] - array of float results
// For RSS: numShares=2 (two shares per party for 3-party computation)
template <typename T>
void ss_batch_fop_arithmetic(T ***result, T ***a, T ***b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
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
    return;
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

    // For RSS public->private: All parties that hold the sparse share must set it to the same value
    // This ensures that during reconstruction, when the same global share is read from multiple parties,
    // they all contribute the same value (which then gets deduplicated in the sum).
    // Parties that don't hold the sparse share (sparsify_public returns idx=-1) do nothing.
    for (int c = 0; c < 4; c++) {
        ss->sparsify_public(result[c], components[c]);
    }
    // Other shares remain 0 (initialized by ss_init), so the sum of all shares = value
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
    // Subtraction is addition with negated sign - call arithmetic op with "-"
    ss_single_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, "-", threadID, net, ss);
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

// Scalar private int to float conversion
// value = T* [numShares] (single integer)
// result = T** [4][numShares] (single float: mantissa, exponent, zero, sign)
template <typename T>
void ss_int2fl(T *value, T **result, int gamma, int K, int L, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

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
    uint numShares = ss->getNumShares();
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
    // Same fix as scalar version: PICCO swaps K and L
    doOperation_FL2Int(value, result, K, L, gamma, size, threadID, net, ss);
}

// Convert single public float to private (secret-shared) float
template <typename T>
void ss_fl2fl(float value, T **result, int K1, int L1, int K2, int L2, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    T components[4];

    // Convert this float to PICCO float components
    rss_convertFloat(value, K2, L2, components);

    // Secret-share each component
    for (int c = 0; c < 4; c++) {
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
    uint numShares = ss->getNumShares();
    T components[4];

    for (int i = 0; i < size; i++) {
        // Convert this float to PICCO float components
        rss_convertFloat(value[i], K2, L2, components);

        // Secret-share each component
        for (int c = 0; c < 4; c++) {
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
    return;
}

template <typename T>
void ss_batch_BOP_float_comparison(T **result, T ***a, T ***b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, T *out_cond, T **priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    return;
}

// REMOVED: Integer ss_batch stubs - now using common implementations from ops/Batch.hpp
// All integer batch operations (T**, T***, int*, etc.) are handled by the unified Batch.hpp
// Only float-specific batch operations remain as stubs below

template <typename T>
void ss_batch_dot(T ***a, T ***b, int size, int array_size, int *index_array, T **result, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
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

// Stub implementations for batch operations not yet supported in RSS
template <typename T>
void doOperation_IntDiv_Pub(T **a, T **b, T **result, int alen, int blen, int resultlen, NodeNetwork net, replicatedSecretShare<T> *ss) {
    std::cout << "[RSS] doOperation_IntDiv_Pub not yet implemented\n";
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

// Simple ss_set for copying one priv_int to another (used in batch operations)
// For RSS: priv_int is T* (pointer to share array)
// Note: For n=3 parties, numShares=2. This is hardcoded for current implementation.
template <typename T>
inline void ss_set(T *a, T *result) {
    // Copy the two shares from a to result (n=3 parties)
    result[0] = a[0];
    result[1] = a[1];
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
