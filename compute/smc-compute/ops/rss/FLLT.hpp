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

#ifndef _FLLT_HPP_
#define _FLLT_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "Mult.hpp"
#include "LT.hpp"
#include "LTEQ.hpp"

// Interface format: [size][numShares] where array[i][s] is share s of element i
// Float format: a[component][i][s] where component is 0=mantissa, 1=exponent, 2=zero, 3=sign
// This version uses a 4-element representation per share as follows:
//   Index 0: mantissa
//   Index 1: exponent
//   Index 2: zero flag
//   Index 3: sign
template <typename T>
void FLLT(T ***a, T ***b, T **result, uint size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;

    // Allocate arrays for all intermediate values in interface format [size][numShares]
    T **eLT = new T *[size];
    T **eEQ = new T *[size];
    T **m0 = new T *[size];
    T **m1 = new T *[size];
    T **mLT = new T *[size];
    T **b_plus = new T *[size];
    T **b_minus = new T *[size];

    // Buffers for parallel multiplications - size is 4*size for batching
    T **mult_buffer1 = new T *[4 * size];
    T **mult_buffer2 = new T *[4 * size];
    T **mult_result = new T *[4 * size];

    // Pre-compute common multiplications
    T **az_bz = new T *[size];  // For [a.z]*[b.z]
    T **as_bs = new T *[size];  // For [a.s]*[b.s]

    // Memory allocation for the 2D arrays in interface format [size][numShares]
    for (uint i = 0; i < size; i++) {
        eLT[i] = new T[numShares];
        memset(eLT[i], 0, sizeof(T) * numShares);
        eEQ[i] = new T[numShares];
        memset(eEQ[i], 0, sizeof(T) * numShares);
        m0[i] = new T[numShares];
        memset(m0[i], 0, sizeof(T) * numShares);
        m1[i] = new T[numShares];
        memset(m1[i], 0, sizeof(T) * numShares);
        mLT[i] = new T[numShares];
        memset(mLT[i], 0, sizeof(T) * numShares);
        b_plus[i] = new T[numShares];
        memset(b_plus[i], 0, sizeof(T) * numShares);
        b_minus[i] = new T[numShares];
        memset(b_minus[i], 0, sizeof(T) * numShares);
        az_bz[i] = new T[numShares];
        memset(az_bz[i], 0, sizeof(T) * numShares);
        as_bs[i] = new T[numShares];
        memset(as_bs[i], 0, sizeof(T) * numShares);
    }

    // mult_buffer arrays need 4*size entries for batching
    for (uint i = 0; i < 4 * size; i++) {
        mult_buffer1[i] = new T[numShares];
        memset(mult_buffer1[i], 0, sizeof(T) * numShares);
        mult_buffer2[i] = new T[numShares];
        memset(mult_buffer2[i], 0, sizeof(T) * numShares);
        mult_result[i] = new T[numShares];
        memset(mult_result[i], 0, sizeof(T) * numShares);
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, 1);

    // Step 1: Compare exponents
    doOperation_LTEQ(a[1], b[1], eLT, eEQ, ring_size, size, nodeNet, ss);

    // Compute [a.z]*[b.z], [a.s]*[b.s], and mantissas in parallel
    // Interface format: mult_buffer[i][s] where i is index, s is share
    for (uint i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            mult_buffer1[i][s] = a[2][i][s];                              // [a.z]
            mult_buffer2[i][s] = b[2][i][s];                              // [b.z]

            mult_buffer1[i + size][s] = a[3][i][s];                       // [a.s]
            mult_buffer2[i + size][s] = b[3][i][s];                       // [b.s]

            mult_buffer1[i + 2 * size][s] = ai[s] - T(2) * a[3][i][s];    // 1 - 2[a.s]
            mult_buffer2[i + 2 * size][s] = a[0][i][s];                   // [a.m]

            mult_buffer1[i + 3 * size][s] = ai[s] - T(2) * b[3][i][s];    // 1 - 2[b.s]
            mult_buffer2[i + 3 * size][s] = b[0][i][s];                   // [b.m]
        }
    }

    // Single Mult call for all computations
    Mult(mult_result, mult_buffer1, mult_buffer2, 4 * size, threadID, nodeNet, ss);

    // Extract results - interface format [i][s]
    for (uint i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            az_bz[i][s] = mult_result[i][s];              // [a.z] * [b.z]
            as_bs[i][s] = mult_result[i + size][s];       // [a.s] * [b.s]
            m0[i][s] = mult_result[i + 2 * size][s];      // mantissa m0 = [a.m] * (1 - 2 * [a.s])
            m1[i][s] = mult_result[i + 3 * size][s];      // mantissa m1 = [b.m] * (1 - 2 * [b.s])
        }
    }

    // Step 4: Mantissa comparison (signed mantissas, use ring_size as bit length)
    doOperation_LT(mLT, m0, m1, ring_size, ring_size, ring_size, size, threadID, nodeNet, ss);

    // Combine steps 5, 6, and parts of 7 into a single Mult call - interface format [i][s]
    for (uint i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            mult_buffer1[i][s] = eEQ[i][s];                        // For eEQ * mLT
            mult_buffer2[i][s] = mLT[i][s];

            mult_buffer1[i + size][s] = ai[s] - eEQ[i][s];         // For (1 - eEQ) * eLT
            mult_buffer2[i + size][s] = eLT[i][s];

            mult_buffer1[i + 2 * size][s] = a[2][i][s] - az_bz[i][s];  // (a.z - a.z * b.z)
            mult_buffer2[i + 2 * size][s] = ai[s] - b[3][i][s];        // (1 - b.s)

            mult_buffer1[i + 3 * size][s] = b[2][i][s] - az_bz[i][s];  // (b.z - a.z * b.z)
            mult_buffer2[i + 3 * size][s] = a[3][i][s];                // a.s
        }
    }

    // Single Mult call for all four computations
    Mult(mult_result, mult_buffer1, mult_buffer2, 4 * size, threadID, nodeNet, ss);

    T temp1, temp2;
    // Extract results and compute b+ and b- - interface format [i][s]
    for (uint i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            temp1 = mult_result[i][s];              // eEQ * mLT
            temp2 = mult_result[i + size][s];       // (1 - eEQ) * eLT

            b_plus[i][s] = temp1 + temp2;                             // eEQ * mLT + (1 - eEQ) * eLT
            b_minus[i][s] = temp1 - temp2 + (ai[s] - eEQ[i][s]);      // eEQ * mLT + (1 - eEQ) * (1 - eLT)
        }
    }

    // Step 7: New approach for part 3 - interface format [i][s]
    for (uint i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            // Pack remaining multiplications
            mult_buffer1[i][s] = ai[s] - b[3][i][s] - a[3][i][s] + as_bs[i][s];  // (1 - b.s - a.s + a.s * b.s) part 4
            mult_buffer2[i][s] = b_plus[i][s];                                    // [b+]

            mult_buffer1[i + size][s] = as_bs[i][s];       // [a.s] * [b.s] part 5
            mult_buffer2[i + size][s] = b_minus[i][s];     // [b-]
        }
    }

    // Call Mult function with buffer size 2
    Mult(mult_result, mult_buffer1, mult_buffer2, 2 * size, threadID, nodeNet, ss);

    T combined_result;
    // Interface format [i][s]
    for (uint i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            combined_result = mult_result[i][s] +              // Result from part 4 = (1 - b.s - a.s + a.s * b.s) * [b+]
                              mult_result[i + size][s] +       // Result from part 5 = a.s * b.s * [b-]
                              (a[3][i][s] - as_bs[i][s]);      // (a.s - a.s * b.s)

            // Multiply with (1 - b.z - a.z + a.z * b.z)
            mult_buffer1[i][s] = ai[s] - b[2][i][s] - a[2][i][s] + az_bz[i][s];  // (1 - b.z - a.z + a.z * b.z)
            mult_buffer2[i][s] = combined_result;
        }
    }

    // Final multiplication
    Mult(mult_result, mult_buffer1, mult_buffer2, size, threadID, nodeNet, ss);

    // Extract results from previous computations - interface format [i][s]
    // Note: We need to get part1 and part2 from the earlier mult_result (4*size call)
    // But that buffer was overwritten. Need to recompute or store separately.
    // Looking at the original code, part1 and part2 come from indices [i + 2*size] and [i + 3*size]
    // from the second 4*size Mult call. But those were overwritten by subsequent Mult calls.
    // The original algorithm seems to have a bug - let's compute the final result correctly.

    // Final result: combine all parts
    for (uint i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            result[i][s] = mult_result[i][s];  // Main comparison result
        }
    }

    // Add the zero-flag handling parts - we need to recompute these
    // (a.z - a.z * b.z) * (1 - b.s) + (b.z - a.z * b.z) * a.s
    for (uint i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            mult_buffer1[i][s] = a[2][i][s] - az_bz[i][s];       // (a.z - a.z * b.z)
            mult_buffer2[i][s] = ai[s] - b[3][i][s];             // (1 - b.s)

            mult_buffer1[i + size][s] = b[2][i][s] - az_bz[i][s];  // (b.z - a.z * b.z)
            mult_buffer2[i + size][s] = a[3][i][s];                // a.s
        }
    }

    Mult(mult_result, mult_buffer1, mult_buffer2, 2 * size, threadID, nodeNet, ss);

    for (uint i = 0; i < size; i++) {
        for (uint s = 0; s < numShares; s++) {
            result[i][s] = result[i][s] + mult_result[i][s] + mult_result[i + size][s];
        }
    }

    // Cleanup
    delete[] ai;

    for (uint i = 0; i < size; i++) {
        delete[] eLT[i];
        delete[] eEQ[i];
        delete[] m0[i];
        delete[] m1[i];
        delete[] mLT[i];
        delete[] b_plus[i];
        delete[] b_minus[i];
        delete[] az_bz[i];
        delete[] as_bs[i];
    }
    delete[] eLT;
    delete[] eEQ;
    delete[] m0;
    delete[] m1;
    delete[] mLT;
    delete[] b_plus;
    delete[] b_minus;
    delete[] az_bz;
    delete[] as_bs;

    for (uint i = 0; i < 4 * size; i++) {
        delete[] mult_buffer1[i];
        delete[] mult_buffer2[i];
        delete[] mult_result[i];
    }
    delete[] mult_buffer1;
    delete[] mult_buffer2;
    delete[] mult_result;
}

#endif // _FLLT_HPP_
