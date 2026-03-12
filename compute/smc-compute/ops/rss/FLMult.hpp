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
#ifndef _FLMULT_HPP_
#define _FLMULT_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "RNTE.hpp"
#include "Mult.hpp"
#include "LT.hpp"

// Interface format: [size][numShares] where array[i][s] is share s of element i
// Float format: a[component][i][s] where component is 0=mantissa, 1=exponent, 2=zero, 3=sign

template <typename T>
void doOperation_FLMult(T ***a, T ***b, T ***result, int K, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
   uint numShares = ss->getNumShares();

   // Allocate in interface format [size][numShares]
   T **m = new T *[size];
   T **mq = new T *[size];
   T **mqp1 = new T *[size];
   T **b_bit = new T *[size];

   T **mult_buffer1 = new T *[4 * size];
   T **mult_buffer2 = new T *[4 * size];
   T **mult_result = new T *[4 * size];

   T *public_val = new T[size];
   T **const_input = new T *[size];

   T **c_m = new T *[size];
   T **c_z = new T *[size];
   T **c_s = new T *[size];
   T **c_e = new T *[size];

   for (int i = 0; i < size; i++) {
      m[i] = new T[numShares];
      memset(m[i], 0, sizeof(T) * numShares);
      mq[i] = new T[numShares];
      memset(mq[i], 0, sizeof(T) * numShares);
      mqp1[i] = new T[numShares];
      memset(mqp1[i], 0, sizeof(T) * numShares);
      b_bit[i] = new T[numShares];
      memset(b_bit[i], 0, sizeof(T) * numShares);

      const_input[i] = new T[numShares];
      memset(const_input[i], 0, sizeof(T) * numShares);

      c_m[i] = new T[numShares];
      memset(c_m[i], 0, sizeof(T) * numShares);
      c_z[i] = new T[numShares];
      memset(c_z[i], 0, sizeof(T) * numShares);
      c_s[i] = new T[numShares];
      memset(c_s[i], 0, sizeof(T) * numShares);
      c_e[i] = new T[numShares];
      memset(c_e[i], 0, sizeof(T) * numShares);
   }

   for (int i = 0; i < 4 * size; i++) {
      mult_buffer1[i] = new T[numShares];
      memset(mult_buffer1[i], 0, sizeof(T) * numShares);
      mult_buffer2[i] = new T[numShares];
      memset(mult_buffer2[i], 0, sizeof(T) * numShares);
      mult_result[i] = new T[numShares];
      memset(mult_result[i], 0, sizeof(T) * numShares);
   }

   T *ai = new T[numShares];
   memset(ai, 0, numShares * sizeof(T));
   ss->sparsify_public(ai, 1);

   // Pack inputs for batched multiplication
   // Buffer layout: [0..size-1] = mantissa products, [size..2*size-1] = zero products, [2*size..3*size-1] = sign products
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         mult_buffer1[i][s] = a[0][i][s];                     // a.m
         mult_buffer2[i][s] = b[0][i][s];                     // b.m

         mult_buffer1[i + size][s] = a[2][i][s];              // a.z
         mult_buffer2[i + size][s] = b[2][i][s];              // b.z

         mult_buffer1[i + 2 * size][s] = a[3][i][s];          // a.s
         mult_buffer2[i + 2 * size][s] = b[3][i][s];          // b.s
      }
   }

   Mult(mult_result, mult_buffer1, mult_buffer2, 3 * size, threadID, nodeNet, ss);

   // Extract results from previous computations
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         m[i][s] = mult_result[i][s];                         // [a.m] * [b.m]
         c_z[i][s] = a[2][i][s] + b[2][i][s] - mult_result[i + size][s];  // [a.z] + [b.z] - [a.z] * [b.z] = [c.z]
         c_s[i][s] = a[3][i][s] + b[3][i][s] - T(2) * mult_result[i + 2 * size][s];  // [a.s] + [b.s] - 2 * [a.s] * [b.s] = [c.s]
      }
   }

   // RNTE(result, input, K, m, size, ...) where K=input_bitlength, m=truncation_bits
   // For mantissa product: input is 2K bits
   // mq = truncated to K bits (shift right by K)
   // mqp1 = truncated to K+1 bits (shift right by K-1)
   RNTE(mq, m, 2 * K, K, size, threadID, nodeNet, ss);          // Truncate to K bits
   RNTE(mqp1, m, 2 * K, K - 1, size, threadID, nodeNet, ss);    // Truncate to K+1 bits

   for (int i = 0; i < size; i++) {
      public_val[i] = T(1) << K;  // 2^K (not 2^{K+1})
   }

   // Share the public constant to secret shares
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         const_input[i][s] = public_val[i] * ai[s];
      }
   }

   // Compare mq (K bits) to 2^K (K+1 bits)
   // Use K+1 as bit length for comparison since 2^K requires K+1 bits
   doOperation_LT(b_bit, mq, const_input, K + 1, K + 1, K + 1, size, threadID, nodeNet, ss);

   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         mult_buffer1[i][s] = b_bit[i][s];                    // b
         mult_buffer2[i][s] = mq[i][s];                       // mq

         mult_buffer1[i + size][s] = b_bit[i][s];             // b
         mult_buffer2[i + size][s] = mqp1[i][s];              // mq+1

         mult_buffer1[i + 2 * size][s] = ai[s] - c_z[i][s];   // (1 - c.z)
         mult_buffer2[i + 2 * size][s] = a[1][i][s] + b[1][i][s] + T(K) * ai[s] - b_bit[i][s]; // [a.e] + [b.e] + K - b
      }
   }

   Mult(mult_result, mult_buffer1, mult_buffer2, 3 * size, threadID, nodeNet, ss);

   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         c_m[i][s] = mq[i][s] + mult_result[i + size][s] - mult_result[i][s]; // [c.m] = (1-b)*[mq] + b*[mq+1]
         c_e[i][s] = mult_result[i + 2 * size][s];            // [c.e] = (1 - c.z) * ([a.e] + [b.e] + K - b)
      }
   }

   // Store the results in the output array
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         result[0][i][s] = c_m[i][s];        // [c.m]
         result[1][i][s] = c_e[i][s];        // [c.e]
         result[2][i][s] = c_z[i][s];        // [c.z]
         result[3][i][s] = c_s[i][s];        // [c.s]
      }
   }

   // Cleanup
   delete[] ai;
   delete[] public_val;

   for (int i = 0; i < size; i++) {
      delete[] m[i];
      delete[] mq[i];
      delete[] mqp1[i];
      delete[] b_bit[i];
      delete[] const_input[i];
      delete[] c_m[i];
      delete[] c_z[i];
      delete[] c_s[i];
      delete[] c_e[i];
   }
   delete[] m;
   delete[] mq;
   delete[] mqp1;
   delete[] b_bit;
   delete[] const_input;
   delete[] c_m;
   delete[] c_z;
   delete[] c_s;
   delete[] c_e;

   for (int i = 0; i < 4 * size; i++) {
      delete[] mult_buffer1[i];
      delete[] mult_buffer2[i];
      delete[] mult_result[i];
   }
   delete[] mult_buffer1;
   delete[] mult_buffer2;
   delete[] mult_result;
}

#endif // _FLMULT_HPP_
