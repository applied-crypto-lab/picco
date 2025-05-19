#pragma once

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "RNTE.hpp"
#include "Mult.hpp"
#include "LT.hpp"


// This version uses a 4–element representation per share as follows:
//   Index 0: mantissa)
//   Index 1: exponent
//   Index 2: zero flag
//   Index 3: sign
template <typename T>
void FLMul(T ***a, T ***b, T ***c, uint size, int q, int ring_size, NodeNetwork &nodeNet, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    // Allocate intermediate buffers
    T **m = new T *[numShares];
    T **mq = new T *[numShares];
    T **mq1 = new T *[numShares];
    T **b_bit = new T *[numShares];

    T **mult_buf1 = new T *[numShares];
    T **mult_buf2 = new T *[numShares];
    T **mult_result = new T *[numShares];

    T **c_m = new T *[numShares];
    T **c_z = new T *[numShares];
    T **c_s = new T *[numShares];
    T **c_e = new T *[numShares];

    for (uint s = 0; s < numShares; s++) {
        m[s] = new T[size];
        mq[s] = new T[size];
        mq1[s] = new T[size];
        b_bit[s] = new T[size];

        c_m[s] = new T[size];
        c_z[s] = new T[size];
        c_e[s] = new T[size];
        c_s[s] = new T[size];

        mult_buf1[s] = new T[3 * size];
        mult_buf2[s] = new T[3 * size];
        mult_result[s] = new T[3 * size];
    }

    T *ai = new T[numShares];
    memset(ai, 0, sizeof(T) * numShares);
    ss->sparsify_public(ai, 1);

    //  Line 1, 6, 8 packed into ONE Mult
    for (uint s = 0; s < numShares; s++) {
        for (uint i = 0; i < size; i++) {
            mult_buf1[s][i] = a[0][s][i];                  // a.m
            mult_buf2[s][i] = b[0][s][i];                  // b.m

            mult_buf1[s][i + size] = a[2][s][i];           // a.z
            mult_buf2[s][i + size] = b[2][s][i];           // b.z

            mult_buf1[s][i + 2 * size] = a[3][s][i];       // a.s
            mult_buf2[s][i + 2 * size] = b[3][s][i];       // b.s
        }
    }

    Mult(mult_result, mult_buf1, mult_buf2, 3 * size, ring_size, nodeNet, ss);

    // Extract outputs for m, z, s
    for (uint s = 0; s < numShares; s++) {
        for (uint i = 0; i < size; i++) {
            m[s][i] = mult_result[s][i];                      // Line 1 result

            T a_z = a[2][s][i], b_z = b[2][s][i];
            T a_s = a[3][s][i], b_s = b[3][s][i];
            T a_s_b_s = mult_result[s][i + 2 * size];

            c_z[s][i] = a_z + b_z - mult_result[s][i + size];                     // Line 6 result
            c_s[s][i] = a_s + b_s - (a_s_b_s * T(2));                             // Line 8 result
        }
    }

    //  RNTE Truncation 
    RNTE(mq, m, ring_size, q, size, -1, nodeNet, ss);   // Line 2 Truncate to q bits
    RNTE(mq1, m, ring_size, q + 1, size, -1, nodeNet, ss); // Line 3 Truncate to q+1 bits


    //  Line 4: b = LT([mq], 2^{q+1} )
    //  Allocate public constant buffer
    T *public_val = new T[size];
    for (uint i = 0; i < size; i++) {
        public_val[i] = T(1) << (q + 1);  // 2^{q+1}
    }

    // Allocate secret-shared output buffer
    T **const_input = new T *[numShares];
    for (uint s = 0; s < numShares; ++s)
        const_input[s] = new T[size];

    //  Share the public constant to secret shares
    // -1 = public input from "everyone", true = public
    ss->input(const_input, public_val, size, -1, -1, true);

// Now `const_input` is secret-shared representation of [2^{q+1}] at all slots
    doOperation_LT(b_bit, mq, const_input, 0, 0, 0, size, -1, nodeNet, ss);

    //  Line 5 and 7 packed into ONE Mult 
    // for line 5 (1−b)×mq1=mq1−b×mq1
    for (uint s = 0; s < numShares; s++) {
        for (uint i = 0; i < size; i++) {
            mult_buf1[s][i] = b_bit[s][i];                         // for c.m = b * mq + (1-b) * mq1
            mult_buf2[s][i] = mq[s][i];

            mult_buf1[s][i + size] = b_bit[s][i];
            mult_buf2[s][i + size] = mq1[s][i];

            mult_buf1[s][i + 2*size] = (ai[s] * T(1)) - c_z[s][i];
            mult_buf2[s][i + 2*size] = a[1][s][i] + b[1][s][i] + T(q) + T(1) - b[s][i];
        }
    }

    Mult(mult_result, mult_buf1, mult_buf2, 3 * size, ring_size, nodeNet, ss);

    // Extract c.m and c.e
    for (uint s = 0; s < numShares; s++) {
    for (uint i = 0; i < size; i++) {
        T b_times_mq = mult_result[s][i];
        T b_times_mq1 = mult_result[s][i + size];
        T exponent_part = mult_result[s][i + 2*size];

        c_m[s][i] = b_times_mq + (mq1[s][i] - b_times_mq1); //c.m=(b×mq)+(mq1−b×mq1)
        c_e[s][i] = exponent_part; //c.e=(1−c.z)×(a.e+b.e+q+1−b)
    }
}

    //  Final Output
    // c[0] = mantissa, c[1] = exponent, c[2] = zero flag, c[3] = sign bit
    for (uint s = 0; s < numShares; s++) {
        for (uint i = 0; i < size; i++) {
            c[0][s][i] = c_m[s][i];
            c[1][s][i] = c_e[s][i];
            c[2][s][i] = c_z[s][i];
            c[3][s][i] = c_s[s][i];
        }
    }

}