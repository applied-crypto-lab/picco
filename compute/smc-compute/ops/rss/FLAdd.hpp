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
#ifndef _FLADD_HPP_
#define _FLADD_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "Mult.hpp"
#include "LT.hpp"
#include "LTEQ.hpp"
#include "Pow2.hpp"
#include "BitDec.hpp"
#include "Open.hpp"

// Interface format: [size][numShares] where array[i][s] is share s of element i
// Float format: a[component][i][s] where component is 0=mantissa, 1=exponent, 2=zero, 3=sign

template <typename T>
void doOperation_FLAdd(T ***a, T ***b, T ***result, int K, int L, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss){
   uint numShares = ss->getNumShares();
   uint ring_size = ss->ring_size;

   // Allocate arrays in interface format [size][numShares]
   T **cLT = new T *[size];
   T **cEQ = new T *[size];
   T **mLT = new T *[size];

   T **az_bz = new T *[size];
   T **az_be = new T *[size];
   T **bz_ae = new T *[size];
   T **az_bm = new T *[size];
   T **bz_am = new T *[size];
   T **as_az_bz = new T *[size];
   T **bs_az_bz = new T *[size];

   T **cLT_bs = new T *[size];
   T **cLT_as = new T *[size];
   T **mLT_bs = new T *[size];
   T **mLT_as = new T *[size];

   T **beta_max_e = new T *[size];
   T **beta_max_m = new T *[size];
   T **beta_min_e = new T *[size];
   T **beta_min_m = new T *[size];

   T **s_bit = new T *[size];
   T **d = new T *[size];
   T **delta= new T *[size];

   T **m = new T *[size];
   T **m1 = new T *[size];
   T **m2 = new T *[size];

   T **two_delta = new T *[size];
   T **two_qp1_delta = new T *[size];
   T **d_two_qp1_delta = new T *[size];

   T **m_bits = new T *[size];
   T **h_bits = new T *[size];
   T **e0_bit = new T *[size];
   T **two_e0_bit = new T *[size];
   T **two_e0_m = new T *[size];

   T **e_bit = new T *[size];
   T **k1 = new T *[size];

   // Buffers for parallel multiplications - [7*size][numShares]
   T **mult_buffer1 = new T *[7 * size];
   T **mult_buffer2 = new T *[7 * size];
   T **mult_result = new T *[7 * size];

   // Memory allocation in interface format [size][numShares]
   for (int i = 0; i < size; i++) {
      cLT[i] = new T[numShares];
      memset(cLT[i], 0, sizeof(T) * numShares);
      cEQ[i] = new T[numShares];
      memset(cEQ[i], 0, sizeof(T) * numShares);
      mLT[i] = new T[numShares];
      memset(mLT[i], 0, sizeof(T) * numShares);

      k1[i] = new T[numShares];
      memset(k1[i], 0, sizeof(T) * numShares);

      az_bz[i] = new T[numShares];
      memset(az_bz[i], 0, sizeof(T) * numShares);
      az_be[i] = new T[numShares];
      memset(az_be[i], 0, sizeof(T) * numShares);
      bz_ae[i] = new T[numShares];
      memset(bz_ae[i], 0, sizeof(T) * numShares);
      az_bm[i] = new T[numShares];
      memset(az_bm[i], 0, sizeof(T) * numShares);
      bz_am[i] = new T[numShares];
      memset(bz_am[i], 0, sizeof(T) * numShares);
      as_az_bz[i] = new T[numShares];
      memset(as_az_bz[i], 0, sizeof(T) * numShares);
      bs_az_bz[i] = new T[numShares];
      memset(bs_az_bz[i], 0, sizeof(T) * numShares);

      cLT_bs[i] = new T[numShares];
      memset(cLT_bs[i], 0, sizeof(T) * numShares);
      cLT_as[i] = new T[numShares];
      memset(cLT_as[i], 0, sizeof(T) * numShares);
      mLT_bs[i] = new T[numShares];
      memset(mLT_bs[i], 0, sizeof(T) * numShares);
      mLT_as[i] = new T[numShares];
      memset(mLT_as[i], 0, sizeof(T) * numShares);

      beta_max_e[i] = new T[numShares];
      memset(beta_max_e[i], 0, sizeof(T) * numShares);
      beta_max_m[i] = new T[numShares];
      memset(beta_max_m[i], 0, sizeof(T) * numShares);
      beta_min_e[i] = new T[numShares];
      memset(beta_min_e[i], 0, sizeof(T) * numShares);
      beta_min_m[i] = new T[numShares];
      memset(beta_min_m[i], 0, sizeof(T) * numShares);

      m_bits[i] = new T[numShares];
      memset(m_bits[i], 0, sizeof(T) * numShares);
      h_bits[i] = new T[numShares];
      memset(h_bits[i], 0, sizeof(T) * numShares);
      e0_bit[i] = new T[numShares];
      memset(e0_bit[i], 0, sizeof(T) * numShares);
      two_e0_bit[i] = new T[numShares];
      memset(two_e0_bit[i], 0, sizeof(T) * numShares);
      two_e0_m[i] = new T[numShares];
      memset(two_e0_m[i], 0, sizeof(T) * numShares);

      e_bit[i] = new T[numShares];
      memset(e_bit[i], 0, sizeof(T) * numShares);

      s_bit[i] = new T[numShares];
      memset(s_bit[i], 0, sizeof(T) * numShares);
      d[i] = new T[numShares];
      memset(d[i], 0, sizeof(T) * numShares);
      delta[i] = new T[numShares];
      memset(delta[i], 0, sizeof(T) * numShares);

      m[i] = new T[numShares];
      memset(m[i], 0, sizeof(T) * numShares);
      m1[i] = new T[numShares];
      memset(m1[i], 0, sizeof(T) * numShares);
      m2[i] = new T[numShares];
      memset(m2[i], 0, sizeof(T) * numShares);

      two_delta[i] = new T[numShares];
      memset(two_delta[i], 0, sizeof(T) * numShares);
      two_qp1_delta[i] = new T[numShares];
      memset(two_qp1_delta[i], 0, sizeof(T) * numShares);
      d_two_qp1_delta[i] = new T[numShares];
      memset(d_two_qp1_delta[i], 0, sizeof(T) * numShares);

   }

   // Mult buffers in [7*size][numShares] format
   for (int i = 0; i < 7 * size; i++) {
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

   // Line 1: [a.e] < [b.e]
   doOperation_LTEQ(a[1], b[1], cLT, cEQ, ring_size, size, net, ss);

   // Line 2: LT
   doOperation_LT(mLT, a[0], b[0], 0, 0, ring_size, size, threadID, net, ss);

   // Round 1: Useful for lines 3, 4, 5, and 6
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         mult_buffer1[i][s] = cLT[i][s];                    // cLT
         mult_buffer2[i][s] = b[1][i][s];                   // [b.e]

         mult_buffer1[i + size][s] = cLT[i][s];             // cLT
         mult_buffer2[i + size][s] = a[1][i][s];            // [a.e]

         mult_buffer1[i + 2 * size][s] = cLT[i][s];         // cLT
         mult_buffer2[i + 2 * size][s] = b[0][i][s];        // [b.m]

         mult_buffer1[i + 3 * size][s] = cLT[i][s];         // cLT
         mult_buffer2[i + 3 * size][s] = a[0][i][s];        // [a.m]

         mult_buffer1[i + 4 * size][s] = mLT[i][s];         // mLT
         mult_buffer2[i + 4 * size][s] = b[0][i][s];        // [b.m]

         mult_buffer1[i + 5 * size][s] = mLT[i][s];         // mLT
         mult_buffer2[i + 5 * size][s] = a[0][i][s];        // [a.m]
      }
   }

   // Single Mult call for all computations
   Mult(mult_result, mult_buffer1, mult_buffer2, 6 * size, threadID, net, ss);

   // Extract results
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         // Line 3: beta_max_e = cLT * b.e + (1-cLT) * a.e = a.e + cLT*(b.e - a.e)
         // mult_result[0] = cLT * b.e, mult_result[size] = cLT * a.e
         beta_max_e[i][s] = mult_result[i][s] + a[1][i][s] - mult_result[i + size][s];
         // Line 4: beta_min_e = cLT * a.e + (1-cLT) * b.e = b.e + cLT*(a.e - b.e)
         beta_min_e[i][s] = mult_result[i + size][s] + b[1][i][s] - mult_result[i][s];

         k1[i][s] = ai[s] * T(K);
      }
   }

   // Line 8: Compute delta = beta_max_e - beta_min_e, then check if delta > K
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         delta[i][s] = beta_max_e[i][s] - beta_min_e[i][s];
      }
   }

   // d = LT(K, delta) = 1 if K < delta, i.e., delta > K
   doOperation_LT(d, k1, delta, ring_size, ring_size, ring_size, size, threadID, net, ss);

   // Round 2: Useful for lines 5, 6, and 9
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         mult_buffer1[i][s] = ai[s] - cEQ[i][s];                    // 1 - cEQ
         mult_buffer2[i][s] = mult_result[i + 2 * size][s] + a[0][i][s] - mult_result[i + 3 * size][s];

         mult_buffer1[i + size][s] = ai[s] - cEQ[i][s];             // 1 - cEQ
         mult_buffer2[i + size][s] = mult_result[i + 3 * size][s] + b[0][i][s] - mult_result[i + 2 * size][s];

         mult_buffer1[i + 2 * size][s] = cEQ[i][s];                 // cEQ
         mult_buffer2[i + 2 * size][s] = mult_result[i + 4 * size][s] + a[0][i][s] - mult_result[i + 5 * size][s];

         mult_buffer1[i + 3 * size][s] = cEQ[i][s];                 // cEQ
         mult_buffer2[i + 3 * size][s] = mult_result[i + 5 * size][s] + b[0][i][s] - mult_result[i + 4 * size][s];

         mult_buffer1[i + 4 * size][s] = a[3][i][s];                // [a.s]
         mult_buffer2[i + 4 * size][s] = b[3][i][s];                // [b.s]

         mult_buffer1[i + 5 * size][s] = ai[s] - d[i][s];           // 1 - d
         mult_buffer2[i + 5 * size][s] = beta_max_e[i][s] - beta_min_e[i][s];
      }
   }

   Mult(mult_result, mult_buffer1, mult_buffer2, 6 * size, threadID, net, ss);

   // Extract results
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         // Line 5
         beta_max_m[i][s] = mult_result[i][s] + mult_result[i + 2 * size][s];
         // Line 6
         beta_min_m[i][s] = mult_result[i + size][s] + mult_result[i + 3 * size][s];
         // Line 7
         s_bit[i][s] = a[3][i][s] + b[3][i][s] - T(2) * mult_result[i + 4 * size][s];
         // Line 9
         delta[i][s] = mult_result[i + 5 * size][s];

         // Line 11
         m1[i][s] = T(2) * (beta_max_m[i][s] - s_bit[i][s]) + ai[s];
      }
   }

   // Setup for Pow2
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         mult_buffer1[i][s] = delta[i][s];                          // delta
         mult_buffer1[i + size][s] = ai[s] * T(K + 1) - delta[i][s];  // K + 1 - delta
      }
   }

   // Line 10, and 13: Pow2
   doOperation_Pow2(mult_buffer2, mult_buffer1, K + 2, 2 * size, threadID, net, ss);

   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         two_delta[i][s] = mult_buffer2[i][s];
         two_qp1_delta[i][s] = mult_buffer2[i + size][s];
      }
   }

   // Round 3: Useful for lines 12, 14, 22, and 25
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         mult_buffer1[i][s] = beta_max_m[i][s];                     // [ßmax.m]
         mult_buffer2[i][s] = two_delta[i][s];                      // [2 ^ delta]

         mult_buffer1[i + size][s] = ai[s] - T(2) * s_bit[i][s];    // 1 - 2 * s
         mult_buffer2[i + size][s] = beta_min_m[i][s];              // [ßmin.m]

         mult_buffer1[i + 2 * size][s] = d[i][s];                   // [d]
         mult_buffer2[i + 2 * size][s] = two_qp1_delta[i][s];       // [2 ^ (K + 1 - delta)]

         mult_buffer1[i + 3 * size][s] = a[2][i][s];                // [a.z]
         mult_buffer2[i + 3 * size][s] = b[2][i][s];                // [b.z]

         mult_buffer1[i + 4 * size][s] = cLT[i][s];                 // cLT
         mult_buffer2[i + 4 * size][s] = b[3][i][s];                // [b.s]

         mult_buffer1[i + 5 * size][s] = cLT[i][s];                 // cLT
         mult_buffer2[i + 5 * size][s] = a[3][i][s];                // [a.s]
      }
   }

   Mult(mult_result, mult_buffer1, mult_buffer2, 6 * size, threadID, net, ss);

   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         // Line 12
         m2[i][s] = mult_result[i][s] + mult_result[i + size][s];

         d_two_qp1_delta[i][s] = mult_result[i + 2 * size][s];
         az_bz[i][s] = mult_result[i + 3 * size][s];
         cLT_bs[i][s] = mult_result[i + 4 * size][s];
         cLT_as[i][s] = mult_result[i + 5 * size][s];
      }
   }

   // Round 4: Useful for lines 14, 24, and 25
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         mult_buffer1[i][s] = d_two_qp1_delta[i][s];
         mult_buffer2[i][s] = m1[i][s];

         mult_buffer1[i + size][s] = two_qp1_delta[i][s] - d_two_qp1_delta[i][s];
         mult_buffer2[i + size][s] = m2[i][s];

         mult_buffer1[i + 2 * size][s] = a[2][i][s];
         mult_buffer2[i + 2 * size][s] = b[1][i][s];

         mult_buffer1[i + 3 * size][s] = b[2][i][s];
         mult_buffer2[i + 3 * size][s] = a[1][i][s];

         mult_buffer1[i + 4 * size][s] = mLT[i][s];
         mult_buffer2[i + 4 * size][s] = b[3][i][s];

         mult_buffer1[i + 5 * size][s] = mLT[i][s];
         mult_buffer2[i + 5 * size][s] = a[3][i][s];
      }
   }

   Mult(mult_result, mult_buffer1, mult_buffer2, 6 * size, threadID, net, ss);

   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         // Line 14
         m[i][s] = mult_result[i][s] + mult_result[i + size][s];

         az_be[i][s] = mult_result[i + 2 * size][s];
         bz_ae[i][s] = mult_result[i + 3 * size][s];
         mLT_bs[i][s] = mult_result[i + 4 * size][s];
         mLT_as[i][s] = mult_result[i + 5 * size][s];
      }
   }

   // Line 15: BitDec - extract 2q+2 bits from m (m fits in 2K+2 bits: max at delta=0 is 2^(K+1)*(2^(K+1)-1) < 2^(2K+2))
   Rss_BitDec(m_bits, m, 2 * K + 2, size, ring_size, net, ss);

   // Line 16: PreOR - compute prefix OR (reverse order)
   Rss_PreOR(h_bits, m_bits, size, 2 * K + 2, net, ss);

   // Line 17: B2A - extract individual bits and convert to arithmetic shares
   uint h_bitlength = 2 * K + 2;
   T **h_bits_extracted = new T *[h_bitlength * size];
   for (uint j = 0; j < h_bitlength * size; j++) {
      h_bits_extracted[j] = new T[numShares];
      memset(h_bits_extracted[j], 0, sizeof(T) * numShares);
   }

   // Extract individual bits from the packed PreOR result
   for (uint j = 0; j < h_bitlength; j++) {
      for (int i = 0; i < size; i++) {
         for (uint s = 0; s < numShares; s++) {
            h_bits_extracted[j * size + i][s] = GET_BIT(h_bits[i][s], T(j));
         }
      }
   }

   // B2A converts bit shares to arithmetic shares
   Rss_B2A(h_bits_extracted, h_bits_extracted, h_bitlength * size, ring_size, net, ss);

   // Line 18: e0 = (2K+2) - sum(h_i)
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         e0_bit[i][s] = ai[s] * T(2 * K + 2);  // Start with public (2K+2)
      }
   }
   for (uint j = 0; j < h_bitlength; j++) {
      for (int i = 0; i < size; i++) {
         for (uint s = 0; s < numShares; s++) {
            e0_bit[i][s] -= h_bits_extracted[j * size + i][s];
         }
      }
   }

   // Line 19: 2^e0 = 1 + sum(2^i * (1 - h_i))
   // NOTE: PreOR computes in REVERSE order: h_j = OR(m_j, ..., m_k)
   // Initialize to 1
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         two_e0_bit[i][s] = ai[s];  // Start with public 1
      }
   }
   // Add 2^{k-j} * (1 - h_j) for each bit (corrected for reverse PreOR)
   for (uint j = 0; j < h_bitlength; j++) {
      T pow2_j = T(1) << T(h_bitlength - 1 - j);  // Use k-j instead of j
      for (int i = 0; i < size; i++) {
         for (uint s = 0; s < numShares; s++) {
            two_e0_bit[i][s] += pow2_j * (ai[s] - h_bits_extracted[j * size + i][s]);
         }
      }
   }

   // Clean up extracted bits
   for (uint j = 0; j < h_bitlength * size; j++) {
      delete[] h_bits_extracted[j];
   }
   delete[] h_bits_extracted;

   // Round 5: Useful for lines 20, 22, 24, and 25
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         mult_buffer1[i][s] = m[i][s];
         mult_buffer2[i][s] = two_e0_bit[i][s];

         mult_buffer1[i + size][s] = a[2][i][s];
         mult_buffer2[i + size][s] = b[0][i][s];

         mult_buffer1[i + 2 * size][s] = b[2][i][s];
         mult_buffer2[i + 2 * size][s] = a[0][i][s];

         mult_buffer1[i + 3 * size][s] = b[2][i][s] - az_bz[i][s];
         mult_buffer2[i + 3 * size][s] = a[3][i][s];

         mult_buffer1[i + 4 * size][s] = ai[s] - cEQ[i][s];
         mult_buffer2[i + 4 * size][s] = cLT_bs[i][s] + a[3][i][s] - cLT_as[i][s];

         mult_buffer1[i + 5 * size][s] = cEQ[i][s];
         mult_buffer2[i + 5 * size][s] = mLT_bs[i][s] + a[3][i][s] - mLT_as[i][s];
      }
   }

   Mult(mult_result, mult_buffer1, mult_buffer2, 6 * size, threadID, net, ss);

   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         two_e0_m[i][s] = mult_result[i][s];
         az_bm[i][s] = mult_result[i + size][s];
         bz_am[i][s] = mult_result[i + 2 * size][s];
         as_az_bz[i][s] = mult_result[i + 3 * size][s];

         // Line 25
         s_bit[i][s] = mult_result[i + 4 * size][s] + mult_result[i + 5 * size][s];
      }
   }

   // Line 20: RNTE
   RNTE(m, two_e0_m, K, K + 2, size, threadID, net, ss);

   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         // Line 21
         e_bit[i][s] = beta_max_e[i][s] - e0_bit[i][s] + ai[s] - d[i][s];
      }
   }

   // Round 6: Useful for lines 22, 24, and 26
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         mult_buffer1[i][s] = m[i][s];
         mult_buffer2[i][s] = ai[s] - a[2][i][s] - b[2][i][s] + az_bz[i][s];

         mult_buffer1[i + size][s] = e_bit[i][s];
         mult_buffer2[i + size][s] = ai[s] - a[2][i][s] - b[2][i][s] + az_bz[i][s];

         mult_buffer1[i + 2 * size][s] = b[3][i][s];
         mult_buffer2[i + 2 * size][s] = a[2][i][s] - az_bz[i][s];
      }
   }

   Mult(mult_result, mult_buffer1, mult_buffer2, 3 * size, threadID, net, ss);

   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         // Line 22: Mantissa
         result[0][i][s] = mult_result[i][s] + az_bm[i][s] + bz_am[i][s];

         // Line 24: Exponent without bias
         result[1][i][s] = mult_result[i + size][s] + az_be[i][s] + bz_ae[i][s];

         bs_az_bz[i][s] = mult_result[i + 2 * size][s];
      }
   }

   // Line 23: EQZ - check if mantissa (result[0]) is zero
   doOperation_EQZ(result[0], result[2], K, size, threadID, net, ss);

   // Round 7: Useful for lines 24, and 26
   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         mult_buffer1[i][s] = result[1][i][s];
         mult_buffer2[i][s] = ai[s] - result[2][i][s];

         mult_buffer1[i + size][s] = ai[s] - a[2][i][s] - b[2][i][s] + az_bz[i][s];
         mult_buffer2[i + size][s] = s_bit[i][s];
      }
   }

   Mult(mult_result, mult_buffer1, mult_buffer2, 2 * size, threadID, net, ss);

   for (int i = 0; i < size; i++) {
      for (uint s = 0; s < numShares; s++) {
         // Line 24: Exponent with bias
         result[1][i][s] = mult_result[i][s];

         // Line 26: Sign bit
         result[3][i][s] = mult_result[i + size][s] + as_az_bz[i][s] + bs_az_bz[i][s];
      }
   }

   // Clean up allocated memory
   delete[] ai;
   for (int i = 0; i < size; i++) {
      delete[] cLT[i];
      delete[] cEQ[i];
      delete[] mLT[i];
      delete[] az_bz[i];
      delete[] az_be[i];
      delete[] bz_ae[i];
      delete[] az_bm[i];
      delete[] bz_am[i];
      delete[] as_az_bz[i];
      delete[] bs_az_bz[i];
      delete[] cLT_bs[i];
      delete[] cLT_as[i];
      delete[] mLT_bs[i];
      delete[] mLT_as[i];
      delete[] beta_max_e[i];
      delete[] beta_max_m[i];
      delete[] beta_min_e[i];
      delete[] beta_min_m[i];
      delete[] s_bit[i];
      delete[] d[i];
      delete[] delta[i];
      delete[] m[i];
      delete[] m1[i];
      delete[] m2[i];
      delete[] two_delta[i];
      delete[] two_qp1_delta[i];
      delete[] d_two_qp1_delta[i];
      delete[] m_bits[i];
      delete[] h_bits[i];
      delete[] e0_bit[i];
      delete[] two_e0_bit[i];
      delete[] two_e0_m[i];
      delete[] e_bit[i];
      delete[] k1[i];
   }
   delete[] cLT;
   delete[] cEQ;
   delete[] mLT;
   delete[] az_bz;
   delete[] az_be;
   delete[] bz_ae;
   delete[] az_bm;
   delete[] bz_am;
   delete[] as_az_bz;
   delete[] bs_az_bz;
   delete[] cLT_bs;
   delete[] cLT_as;
   delete[] mLT_bs;
   delete[] mLT_as;
   delete[] beta_max_e;
   delete[] beta_max_m;
   delete[] beta_min_e;
   delete[] beta_min_m;
   delete[] s_bit;
   delete[] d;
   delete[] delta;
   delete[] m;
   delete[] m1;
   delete[] m2;
   delete[] two_delta;
   delete[] two_qp1_delta;
   delete[] d_two_qp1_delta;
   delete[] m_bits;
   delete[] h_bits;
   delete[] e0_bit;
   delete[] two_e0_bit;
   delete[] two_e0_m;
   delete[] e_bit;
   delete[] k1;

   for (int i = 0; i < 7 * size; i++) {
      delete[] mult_buffer1[i];
      delete[] mult_buffer2[i];
      delete[] mult_result[i];
   }
   delete[] mult_buffer1;
   delete[] mult_buffer2;
   delete[] mult_result;
}

#endif // _FLADD_HPP_
