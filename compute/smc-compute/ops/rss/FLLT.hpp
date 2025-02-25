
#pragma once
#ifndef _FLLTZ_HPP_
#define _FLLTZ_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "Mult.hpp"
#include "LT.hpp"
#include "LTEQ.hpp"


// This version uses a 4–element representation per share as follows:
//   Index 0: mantissa)
//   Index 1: exponent
//   Index 2: zero flag
//   Index 3: sign
template <typename T>
void FLLT(T ***a, T ***b, T **result, uint size, int ring_size, NodeNetwork &nodeNet, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    // Allocate arrays for all intermediate values
    T **eLT = new T *[numShares];
    T **eEQ = new T *[numShares];
    T **m0 = new T *[numShares];
    T **m1 = new T *[numShares];
    T **mLT = new T *[numShares];
    T **b_plus = new T *[numShares];
    T **b_minus = new T *[numShares];

    // Buffers for parallel multiplications
    T **mult_buffer1 = new T *[numShares];
    T **mult_buffer2 = new T *[numShares];
    T **mult_result = new T *[numShares];

    T **part3_result = new T *[numShares];

    // Pre-compute common multiplications
    T **az_bz = new T *[numShares];  // For [a.z]*[b.z]
    T **as_bs = new T *[numShares];  // For [a.s]*[b.s]
    for (uint s = 0; s < numShares; s++) {
        az_bz[s] = new T *[size];
        as_bs[s] = new T *[size];
         part3_result[s] = new T *[size];
        for (int i = 0; i < size; i++) {
            az_bz[s][i] = new T[size];
            as_bs[s][i] = new T[size];
             part3_result[s][i] = new T[size];
        }
    }

    // Allocate temporary arrays for intermediate computations of b_plus and b_minus
    T **temp1 = new T *[numShares];
    T **temp2 = new T *[numShares];
    for (uint s = 0; s < numShares; s++) {
        temp1[s] = new T[size];
        temp2[s] = new T[size];
    }

    // Memory allocation for 2D arrays
    for (uint s = 0; s < numShares; s++) {
        eLT[s] = new T *[size];
        eEQ[s] = new T *[size];
        m0[s] = new T *[size];
        m1[s] = new T *[size];
        mLT[s] = new T *[size];
        b_plus[s] = new T *[size];
        b_minus[s] = new T *[size];
        mult_buffer1[s] = new T *[4 * size]; // Buffer for multiple multiplications
        mult_buffer2[s] = new T *[4 * size];
        mult_result[s] = new T *[4 * size];

        for (int i = 0; i < size; i++) {
            eLT[s][i] = new T[size];
            eEQ[s][i] = new T[size];
            m0[s][i] = new T[size];
            m1[s][i] = new T[size];
            mLT[s][i] = new T[size];
            b_plus[s][i] = new T[size];
            b_minus[s][i] = new T[size];
        }
        for (int i = 0; i < 4 * size; i++) {
            mult_buffer1[s][i] = new T[size];
            mult_buffer2[s][i] = new T[size];
            mult_result[s][i] = new T[size];
        }
    }

    // Step 1: Compare exponents
    doOperation_LTEQ(a, b, eLT, eEQ, ring_size, size, nodeNet, ss);

    // Compute [a.z]*[b.z], [a.s]*[b.s], and mantissas in parallel
    for (uint s = 0; s < numShares; s++) {
        for (int i = 0; i < size; i++) {
            mult_buffer1[s][i] = a[2][s][i];                    // [a.z]
            mult_buffer2[s][i] = b[2][s][i];                    // [b.z]
            mult_buffer1[s][i + size] = a[3][s][i];             // [a.s]
            mult_buffer2[s][i + size] = b[3][s][i];             // [b.s]
            mult_buffer1[s][i + 2 * size] = T(1) - (T(2) * a[3][s][i]);  // 1-2[ā.s]
            mult_buffer2[s][i + 2 * size] = a[0][s][i];                  // [ā.m]
            mult_buffer1[s][i + 3 * size] = T(1) - (T(2) * b[3][s][i]);  // 1-2[b̄.s]
            mult_buffer2[s][i + 3 * size] = b[0][s][i];                  // [b̄.m]
        }
    }

    // Single Mult call for all computations
    Mult(mult_result, mult_buffer1, mult_buffer2, 4 * size, ring_size, nodeNet, ss);

    // Extract results
    for (uint s = 0; s < numShares; s++) {
        for (int i = 0; i < size; i++) {
            az_bz[s][i] = mult_result[s][i];              // [a.z]*[b.z]
            as_bs[s][i] = mult_result[s][i + size];       // [a.s]*[b.s]
            m0[s][i] = mult_result[s][i + 2 * size];        // mantissa m0
            m1[s][i] = mult_result[s][i + 3 * size];        // mantissa m1
        }
    }
    // Step 4: Mantissa comparison
    doOperation_LT(m0, m1, mLT, nullptr, ring_size, size, nodeNet, ss);

// Combine steps 5, 6, and parts of 7 into a single Mult call
for (uint s = 0; s < numShares; s++) {
    for (int i = 0; i < size; i++) {
        mult_buffer1[s][i] = eEQ[1][s][i];                    // For eEQ * mLT
        mult_buffer2[s][i] = mLT[0][s][i];

        mult_buffer1[s][i + size] = T(1) - eEQ[1][s][i];      // For (1-eEQ) * eLT
        mult_buffer2[s][i + size] = eLT[1][s][i];

        mult_buffer1[s][i + 2*size] = a[2][s][i] - az_bz[s][i];  // (a.z - a.z*b.z)
        mult_buffer2[s][i + 2*size] = T(1) - b[3][s][i];         // (1 - b.s)

        mult_buffer1[s][i + 3*size] = b[2][s][i] - az_bz[s][i];  // (b.z - a.z*b.z)
        mult_buffer2[s][i + 3*size] = a[3][s][i];                // a.s
    }
}

// Single Mult call for all four computations
Mult(mult_result, mult_buffer1, mult_buffer2, 4 * size, ring_size, nodeNet, ss);

// Extract results and compute b+ and b-
for (uint s = 0; s < numShares; s++) {
    for (int i = 0; i < size; i++) {
        T temp1 = mult_result[s][i];              // eEQ * mLT
        T temp2 = mult_result[s][i + size];       // (1-eEQ) * eLT
        T part1 = mult_result[s][i + 2*size];     // (a.z - a.z*b.z) * (1 - b.s)
        T part2 = mult_result[s][i + 3*size];     // (b.z - a.z*b.z) * a.s

        b_plus[s][i] = temp1 + temp2;
        b_minus[s][i] = temp1 - temp2 + (T(1) - eEQ[s][i]);

        // Store part1 and part2 for later use in the final result computation
        // You may need to create new arrays to store these values if they're needed later
    }
}

// Step 7: New approach for part 3
for (uint s = 0; s < numShares; s++) {
    for (int i = 0; i < size; i++) {
        // Pack remaining multiplications
        mult_buffer1[s][0] = T(1) - b[3][s][i] - a[3][s][i]+ as_bs[s][i];  // (1-b.s-a.s+a.s*b.s) part 4
        mult_buffer2[s][0] = b_plus[s][i];  // [b+]

        mult_buffer1[s][1] = as_bs[s][i];  // [a.s]*[b.s] part 5
        mult_buffer2[s][1] = b_minus[s][i];  // [b-]
    }
}

// Call Mult function with buffer size 2
Mult(mult_result, mult_buffer1, mult_buffer2, 2 * size, ring_size, nodeNet, ss);

for (uint s = 0; s < numShares; s++) {
    for (int i = 0; i < size; i++) {
        T combined_result = mult_result[s][0] +  // Result from part 4
                            mult_result[s][1] +  // Result from part 5
                            (a[3][s][i] - as_bs[s][i]);  // (a.s - a.s*b.s)

    // Multiply with (1-b.z-a.z+a.z*b.z)
    mult_buffer1[s][i] = T(1) - b[2][s][i] - a[2][s][i] + az_bz[s][i];  // (1-b.z-a.z+a.z*b.z)
    mult_buffer2[s][i] = combined_result;
}
}

// Final multiplication
Mult(part3_result, mult_buffer1, mult_buffer2, size, ring_size, nodeNet, ss);

// Extract results from previous computations
for (uint s = 0; s < numShares; s++) {
    for (int i = 0; i < size; i++) {
        T temp1 = mult_result[s][i];              // eEQ * mLT
        T temp2 = mult_result[s][i + size];       // (1-eEQ) * eLT
        T part1 = mult_result[s][i + 2*size];     // (a.z - a.z*b.z) * (1 - b.s)
        T part2 = mult_result[s][i + 3*size];     // (b.z - a.z*b.z) * a.s

        // Combine results from parts 1, 2, and 3
        result[s][i] = part1 + part2 + part3_result[s][i];
    }
}

}
    // Cleanup
    for (uint s = 0; s < numShares; s++) {
        for (int i = 0; i < size; i++) {
            delete[] eLT[s][i];
            delete[] eEQ[s][i];
            delete[] m0[s][i];
            delete[] m1[s][i];
            delete[] mLT[s][i];
            delete[] b_plus[s][i];
            delete[] b_minus[s][i];
        }
        for (int i = 0; i < 4 * size; i++) {
            delete[] mult_buffer1[s][i];
            delete[] mult_buffer2[s][i];
            delete[] mult_result[s][i];
        }
        delete[] eLT[s];
        delete[] eEQ[s];
        delete[] m0[s];
        delete[] m1[s];
        delete[] mLT[s];
        delete[] b_plus[s];
        delete[] b_minus[s];
        delete[] mult_buffer1[s];
        delete[] mult_buffer2[s];
        delete[] mult_result[s];
    }
        delete[] eLT;
        delete[] eEQ;
        delete[] m0;
        delete[] m1;
        delete[] mLT;
        delete[] b_plus;
        delete[] b_minus;
        delete[] mult_buffer1;
        delete[] mult_buffer2;
        delete[] mult_result;
         delete[] part3_result;
        // Cleanup temp arrays
    for (uint s = 0; s < numShares; s++) {
         delete[]temp1[s];
         delete[] temp2[s];
    }

    delete[] temp1;
    delete[] temp2;

    for (uint s = 0; s < numShares; s++) {
        for (int i = 0; i < size; i++) {
            delete[] az_bz[s][i];
            delete[] as_bs[s][i];
            delete[] part3_result[s][i];
        }
        delete[] az_bz[s];
        delete[] as_bs[s];
         delete[] part3_result[s];
    }
    delete[] az_bz;
    delete[] as_bs;
    delete[] part3_result;
    #endif // _FLLTZ_HPP_
