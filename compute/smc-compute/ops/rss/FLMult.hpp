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

template <typename T>

void doOperation_FLMult(T ***a, T ***b, T ***result, int K, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss){
   uint numShares = ss->getNumShares();
   uint ring_size = ss->ring_size;

   T **m = new T *[numShares];
   T **mq = new T *[numShares];
   T **mqp1 = new T *[numShares];
   T **b_bit = new T *[numShares];

   T **mult_buffer1 = new T *[numShares];
   T **mult_buffer2 = new T *[numShares];
   T **mult_result = new T *[numShares];

   T *public_val = new T[size];
   T **const_input = new T *[numShares];

   T **c_m = new T *[numShares];
   T **c_z = new T *[numShares];
   T **c_s = new T *[numShares];
   T **c_e = new T *[numShares];
   
   for (uint s = 0; s < numShares; s++) {
      m[s] = new T[size];
      mq[s] = new T[size];
      mqp1[s] = new T[size];
      b_bit[s] = new T[size];

      mult_buffer1[s] = new T[size];
      mult_buffer2[s] = new T[size];
      mult_result[s] = new T[size];

      const_input[s] = new T[size];

      c_m[s] = new T[size];
      c_z[s] = new T[size];
      c_s[s] = new T[size];
      c_e[s] = new T[size];
   }

   T *ai = new T[numShares];
   memset(ai, 0, numShares * sizeof(T));
   ss->sparsify_public(ai, 1);

   for (uint s = 0; s < numShares; s++) {
      for (uint i = 0; i < size; i++) {
         mult_buffer1[s][i] = a[0][s][i];                  // a.m
         mult_buffer2[s][i] = b[0][s][i];                  // b.m

         mult_buffer1[s][i + size] = a[2][s][i];           // a.z
         mult_buffer2[s][i + size] = b[2][s][i];           // b.z

         mult_buffer1[s][i + 2 * size] = a[3][s][i];       // a.s
         mult_buffer2[s][i + 2 * size] = b[3][s][i];       // b.s
      }
   }

   Mult(mult_result, mult_buffer1, mult_buffer2, 3 * size, threadID, nodeNet, ss);
   
   // Extract results from previous computations
   for (uint s = 0; s < numShares; s++) {
      for (uint i = 0; i < size; i++) {
         m[s][i] = mult_result[s][i];                      // [a.m] * [b.m]
         c_z[s][i] = a[2][s][i] + b[2][s][i] - mult_result[s][i + size];  // [a.z] + [b.z] - [a.z] * [b.z] = [c.z]
         c_s[s][i] = a[3][s][i] + b[3][s][i] - T(2) * mult_result[s][i + 2 * size];  // [a.s] + [b.s] - 2 * [a.s] * [b.s] = [c.s]
      }
   }

   RNTE(mq, m, K, -1, size, threadID, nodeNet, ss);   // Line 2 Truncate to q bits
   RNTE(mqp1, m, K + 1, -1, size, threadID, nodeNet, ss);

   for (uint i = 0; i < size; i++) {
      public_val[i] = T(1) << (K + 1);  // 2^{q+1}
   }

   // Share the public constant to secret shares
   ss->ss_input(const_input, public_val, size, -1, -1, true);

   doOperation_LT(b_bit, mq, const_input, 0, 0, ring_size, size, threadID, nodeNet, ss);

   for (uint s = 0; s < numShares; s++) {
      for (uint i = 0; i < size; i++) {
         mult_buffer1[s][i] = b_bit[s][i];               // b
         mult_buffer2[s][i] = mq[s][i];                  // mq

         mult_buffer1[s][i + size] = b_bit[s][i];        // b
         mult_buffer2[s][i + size] = mqp1[s][i];         // mq+1

         mult_buffer1[s][i + 2 * size] = (ai[s] * T(1)) - c_z[s][i];  // (1 - c.z)
         mult_buffer2[s][i + 2 * size] = a[1][s][i] + b[1][s][i] + T(K) + T(1) - b_bit[s][i]; // [a.e] + [b.e] + q + 1 - b
      }
   }  

   Mult(mult_result, mult_buffer1, mult_buffer2, 3 * size, threadID, nodeNet, ss);

   for (uint s = 0; s < numShares; s++) {
      for (uint i = 0; i < size; i++) {
         c_m[s][i] = mult_result[s][i] + mqp1[s][i] - mult_result[s][i + size]; // [c.m] = b * [mq] + [mq+1] - b * [mq+1]
         c_e[s][i] = mult_result[s][i + 2 * size];             // [c.e] = (1 - c.z) * ([a.e] + [b.e] + q + 1 - b)
      }
   }

   // Store the results in the output array
   for (uint s = 0; s < numShares; s++) {
      for (uint i = 0; i < size; i++) {
         result[0][s][i] = m[s][i];          // [c.m]
         result[1][s][i] = c_e[s][i];        // [c.e]
         result[2][s][i] = c_z[s][i];        // [c.z]
         result[3][s][i] = c_s[s][i];        // [c.s]
      }
   }
}

#endif // _FLMULT_HPP_