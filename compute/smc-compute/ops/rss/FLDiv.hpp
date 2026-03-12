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
#ifndef _FLDIV_HPP_
#define _FLDIV_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "SDiv.hpp"
#include "LT.hpp"
#include "Trunc.hpp"
#include "MSB.hpp"
#include "Mult.hpp"

// ============================================================================
// Division Algorithm Selection: SDiv vs MDiv
// ============================================================================
// We use SDiv (Newton-Raphson based integer division) instead of MDiv
// (Goldschmidt's algorithm) for the following reasons:
//
// 1. CORRECTNESS: MDiv was NOT always producing correct results. It works well
//    when m1 >= m2 (ratio >= 1), but fails for ratios < 1. SDiv handles ALL
//    cases correctly regardless of the input ratio.
//
// 2. PERFORMANCE: SDiv is actually FASTER than MDiv in practice. Despite MDiv
//    having fewer iterations in theory, SDiv's simpler per-iteration cost and
//    better numerical stability make it more efficient overall.
//
// FLDiv calls SDiv for mantissa division. MDiv is kept for reference but
// should NOT be used in production code.
// ============================================================================

// Interface format: [size][numShares] where array[i][s] is share s of element i
// Float format: a[component][size][numShares] where component is 0=mantissa, 1=exponent, 2=zero, 3=sign

template <typename T>
void doOperation_FLDiv_Pub(T ***a, T ***b, T ***result, int K, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss){
   // will not be implemented
}

// FLDiv: Floating-point division
// Based on Shamir FLDiv.cpp implementation
// Source: Aliasgari et al., "Secure Computation on Floating Point Numbers," 2013
// Protocol FLDiv, page 6
//
// For floats (v, p, z, s) where value = v * 2^{p - (K-1)}:
// Division: (v1 * 2^{p1-(K-1)}) / (v2 * 2^{p2-(K-1)}) = (v1/v2) * 2^{p1-p2}
//
// SDiv computes Y = floor(v1 * 2^K / v2), giving a result in [2^{K-1}, 2^{K+1})
// If Y >= 2^K, we normalize by dividing by 2 and adjusting exponent
//
// Algorithm:
// 1) [Y] <- SDiv([v1], [v2] + [z2], K)
// 2) [b] <- LT([Y], 2^K)  // b=1 if Y < 2^K (already normalized)
// 3) [v] <- Trunc(b*Y + Y, K+1, 1)  // if b: Y, else: Y/2
// 4) [p] <- (1 - [z1])([p1] - [p2] - [b])  // b=1 means no normalization, so -1; b=0 adds 1 back
// 5) [z] <- [z1]
// 6) [s] <- XOR([s1], [s2])
// 7) [Error] <- [z2]
template <typename T>
void doOperation_FLDiv(T ***a, T ***b, T ***result, T **error, int K, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss){
   uint numShares = ss->getNumShares();
   uint ring_size = ss->ring_size;

   // Arrays in [size][numShares] format
   T **Y = new T *[size];         // Result mantissa
   T **temp1 = new T *[size];
   T **b_bit = new T *[size];     // b flag for normalization
   T **lt_input = new T *[size];

   // Buffer for batched multiplication (3 multiplications)
   T **A_buff = new T *[3 * size];
   T **B_buff = new T *[3 * size];
   T **C_buff = new T *[3 * size];

   for (int i = 0; i < size; i++) {
      Y[i] = new T[numShares];
      memset(Y[i], 0, sizeof(T) * numShares);
      temp1[i] = new T[numShares];
      memset(temp1[i], 0, sizeof(T) * numShares);
      b_bit[i] = new T[numShares];
      memset(b_bit[i], 0, sizeof(T) * numShares);
      lt_input[i] = new T[numShares];
      memset(lt_input[i], 0, sizeof(T) * numShares);
   }

   for (int i = 0; i < 3 * size; i++) {
      A_buff[i] = new T[numShares];
      memset(A_buff[i], 0, sizeof(T) * numShares);
      B_buff[i] = new T[numShares];
      memset(B_buff[i], 0, sizeof(T) * numShares);
      C_buff[i] = new T[numShares];
      memset(C_buff[i], 0, sizeof(T) * numShares);
   }

   T *ai = new T[numShares];
   memset(ai, 0, sizeof(T) * numShares);
   ss->sparsify_public(ai, T(1));

   // Constants
   T beta = T(1) << K;        // 2^K

   // Line 1: Compute temp1 = b[0] + b[2] (mantissa + zero flag)
   // Then Y = SDiv(a[0], temp1, K)
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         temp1[i][s] = b[0][i][s] + b[2][i][s];
      }
   }
   doOperation_SDiv(Y, a[0], temp1, K, size, threadID, net, ss);

   // Line 2: b = LTZ(Y - 2^K)
   // If Y < 2^K, then b = 1 (need to double Y for normalization)
   // We use MSB: if Y - 2^K is negative, MSB = 1
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         lt_input[i][s] = Y[i][s] - ai[s] * beta;
      }
   }
   Rss_MSB(b_bit, lt_input, size, ring_size, net, ss);

   // Line 3: result[0] = Trunc(2*b*Y + (1-b)*Y, K+1, 1)
   //       = Trunc(b*Y + Y, K+1, 1) since 2*b*Y + (1-b)*Y = b*Y + Y
   // Actually: 2*b*Y + (1-b)*Y = 2*b*Y + Y - b*Y = b*Y + Y
   // Prepare batched multiplication:
   // A_buff[0..size-1] = b, B_buff[0..size-1] = Y  -> C_buff[0..size-1] = b*Y
   // A_buff[size..2*size-1] = (1-z1), B_buff[size..2*size-1] = (p1-p2-b)  -> exponent
   // A_buff[2*size..3*size-1] = s1, B_buff[2*size..3*size-1] = s2  -> sign XOR

   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         // For mantissa normalization
         A_buff[i][s] = b_bit[i][s];
         B_buff[i][s] = Y[i][s];

         // For exponent: (1 - z1) * (p1 - p2 - K + 1 - b)
         // SDiv returns Y = (v1/v2) * 2^K, so result mantissa is scaled by 2^K
         // The exponent must be adjusted by -(K-1) to account for this scaling
         // When b=1 (Y < 2^K): exponent = p1 - p2 - K + 1 - 1 = p1 - p2 - K
         // When b=0 (Y >= 2^K, divide mantissa by 2): exponent = p1 - p2 - K + 1
         A_buff[i + size][s] = ai[s] - a[2][i][s];  // 1 - z1
         B_buff[i + size][s] = a[1][i][s] - b[1][i][s] - ai[s] * T(K - 1) - b_bit[i][s];  // p1 - p2 - (K-1) - b

         // For sign XOR: s1 * s2
         A_buff[i + 2 * size][s] = a[3][i][s];
         B_buff[i + 2 * size][s] = b[3][i][s];
      }
   }

   Mult(C_buff, A_buff, B_buff, 3 * size, threadID, net, ss);

   // Complete mantissa computation: temp1 = b*Y + Y = C_buff[0..size-1] + Y
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         // 2*b*Y + (1-b)*Y = b*Y + b*Y + Y - b*Y = b*Y + Y
         temp1[i][s] = C_buff[i][s] + Y[i][s];
      }
   }

   // Truncate to get normalized mantissa
   doOperation_Trunc(result[0], temp1, K + 1, 1, size, threadID, net, ss);

   // Line 4-8: Complete other components
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         // Line 4: exponent = (1 - z1) * (p1 - p2 - K + 1 - b)
         result[1][i][s] = C_buff[i + size][s];

         // Line 5: zero flag = z1
         result[2][i][s] = a[2][i][s];

         // Line 6: sign = s1 XOR s2 = s1 + s2 - 2*s1*s2
         result[3][i][s] = a[3][i][s] + b[3][i][s] - T(2) * C_buff[i + 2 * size][s];

         // Line 7: error = z2 (division by zero)
         error[i][s] = b[2][i][s];
      }
   }

   // Cleanup
   for (int i = 0; i < size; i++) {
      delete[] Y[i];
      delete[] temp1[i];
      delete[] b_bit[i];
      delete[] lt_input[i];
   }
   for (int i = 0; i < 3 * size; i++) {
      delete[] A_buff[i];
      delete[] B_buff[i];
      delete[] C_buff[i];
   }
   delete[] Y;
   delete[] temp1;
   delete[] b_bit;
   delete[] lt_input;
   delete[] A_buff;
   delete[] B_buff;
   delete[] C_buff;
   delete[] ai;
}

// MDiv: Mantissa Division using Goldschmidt's algorithm
// Interface: [size][numShares] format (same as SDiv)
// Based on the algorithm from "Secure Computation on Floating Point Numbers"
//
// NOTE: This algorithm works best when m1 >= m2 (ratio >= 1).
// For ratios < 1, use SDiv instead which handles all cases correctly.
// FLDiv uses SDiv, so floating-point division works for all inputs.
//
// Parameters:
//   m1, m2: input mantissas in [size][numShares] format
//   result: output m1/m2 in [size][numShares] format
//   lambda: precision parameter (typically 4-8)
//   bitlength: K, the bit length of mantissa
//   size: batch size
template <typename T>
void doOperation_MDiv(T **m1, T **m2, T **result, int lambda, int bitlength, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
   uint numShares = ss->getNumShares();

   // Line 1: theta = ceil(log2(K/3.5))
   // Adding 1 to theta for additional iteration precision
   int theta = (int)ceil(log2(double(bitlength) / 3.5)) + 1;

   T alpha = T(1) << (bitlength + lambda);  // 2^(K+lambda)
   T two_power_lambda = T(1) << lambda;     // 2^lambda

   // Allocate arrays in [size][numShares] format (consistent with SDiv)
   T **w = new T *[size];
   T **x = new T *[size];
   T **y = new T *[size];
   T **A_buff = new T *[2 * size];
   T **B_buff = new T *[2 * size];
   T **C_buff = new T *[2 * size];

   for (int i = 0; i < size; i++) {
      w[i] = new T[numShares];
      memset(w[i], 0, sizeof(T) * numShares);
      x[i] = new T[numShares];
      memset(x[i], 0, sizeof(T) * numShares);
      y[i] = new T[numShares];
      memset(y[i], 0, sizeof(T) * numShares);
   }

   for (int i = 0; i < 2 * size; i++) {
      A_buff[i] = new T[numShares];
      memset(A_buff[i], 0, sizeof(T) * numShares);
      B_buff[i] = new T[numShares];
      memset(B_buff[i], 0, sizeof(T) * numShares);
      C_buff[i] = new T[numShares];
      memset(C_buff[i], 0, sizeof(T) * numShares);
   }

   T *ai = new T[numShares];
   memset(ai, 0, sizeof(T) * numShares);
   ss->sparsify_public(ai, T(1));

   // Initial approximation constant: w_0 = 2.9142... * alpha - 2 * m2
   // 2.9142 ≈ 2 + sqrt(2), optimal for Goldschmidt
   double w0_const = 2.9142135623730951;

   // Lines 2-4: Compute initial values
   // m1_prime = 2^lambda * m1
   // m2_prime = 2^lambda * m2
   // For public × private multiplication, multiply each share locally (same pattern as SDiv)
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         // Multiply each share by 2^lambda locally (public constant)
         C_buff[i][s] = two_power_lambda * m1[i][s];           // m1_prime = 2^lambda * m1
         C_buff[i + size][s] = two_power_lambda * m2[i][s];    // m2_prime = 2^lambda * m2
      }
   }

   // Now compute w and set up for y = m1_prime * w, x = m2_prime * w
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         // w = w0_const * alpha - 2 * m2
         w[i][s] = T(w0_const * double(alpha)) * ai[s] - T(2) * m2[i][s];

         // Set up for batch mult
         A_buff[i][s] = C_buff[i][s];           // m1_prime
         B_buff[i][s] = w[i][s];                 // w
         A_buff[i + size][s] = C_buff[i + size][s];  // m2_prime
         B_buff[i + size][s] = w[i][s];          // w
      }
   }

   // Lines 5-6: y = m1_prime * w, x = m2_prime * w
   Mult(C_buff, A_buff, B_buff, 2 * size, threadID, net, ss);

   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         y[i][s] = C_buff[i][s];           // y = m1_prime * w
         x[i][s] = C_buff[i + size][s];    // x = m2_prime * w
      }
   }

   // Lines 7-8: Truncate y and x
   // After multiplication, values have ~2*(K+lambda) bits, truncate by K+lambda bits
   doOperation_Trunc(y, y, 2 * (bitlength + lambda), bitlength + lambda, size, threadID, net, ss);
   doOperation_Trunc(x, x, 2 * (bitlength + lambda), bitlength + lambda, size, threadID, net, ss);

   // Line 9: x = alpha - x
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         x[i][s] = ai[s] * alpha - x[i][s];
      }
   }

   // Lines 10-14: Newton-Raphson iterations
   // Changed from (theta - 1) to theta for additional precision
   for (int th = 0; th < theta; th++) {
      // Prepare batch multiplication
      for (int i = 0; i < size; i++) {
         for (uint s = 0; s < numShares; s++) {
            // Line 11: y = y * (alpha + x)
            A_buff[i][s] = y[i][s];
            B_buff[i][s] = ai[s] * alpha + x[i][s];

            // Line 12: x = x * x
            A_buff[i + size][s] = x[i][s];
            B_buff[i + size][s] = x[i][s];
         }
      }

      Mult(C_buff, A_buff, B_buff, 2 * size, threadID, net, ss);

      for (int i = 0; i < size; i++) {
         for (uint s = 0; s < numShares; s++) {
            y[i][s] = C_buff[i][s];
            x[i][s] = C_buff[i + size][s];
         }
      }

      // Lines 13-14: Truncate y and x
      // After multiplication, values have ~2*(K+lambda) bits, truncate by K+lambda bits
      doOperation_Trunc(y, y, 2 * (bitlength + lambda), bitlength + lambda, size, threadID, net, ss);
      doOperation_Trunc(x, x, 2 * (bitlength + lambda), bitlength + lambda, size, threadID, net, ss);
   }

   // Line 15: Final multiplication y = y * (alpha * 2^lambda + x)
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         A_buff[i][s] = y[i][s];
         B_buff[i][s] = ai[s] * alpha * two_power_lambda + x[i][s];
      }
   }

   Mult(C_buff, A_buff, B_buff, size, threadID, net, ss);

   // Line 16: Final truncation
   // y has K+lambda bits, multiplied by (alpha*2^lambda + x) with K+2*lambda bits
   // Product has (K+lambda) + (K+2*lambda) = 2K+3*lambda bits
   // To get K bits output, truncate by K+3*lambda bits
   doOperation_Trunc(result, C_buff, 2 * bitlength + 3 * lambda, bitlength + 3 * lambda, size, threadID, net, ss);

   // Cleanup
   for (int i = 0; i < size; i++) {
      delete[] w[i];
      delete[] x[i];
      delete[] y[i];
   }
   for (int i = 0; i < 2 * size; i++) {
      delete[] A_buff[i];
      delete[] B_buff[i];
      delete[] C_buff[i];
   }
   delete[] w;
   delete[] x;
   delete[] y;
   delete[] A_buff;
   delete[] B_buff;
   delete[] C_buff;
   delete[] ai;
}

#endif // _FLDIV_HPP_
