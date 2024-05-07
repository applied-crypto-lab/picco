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

#ifndef _TRUNCS_HPP_
#define _TRUNCS_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "LT.hpp"
#include "Mult.hpp"
#include "Pow2.hpp"
#include "Trunc.hpp"

// both the shamir and RSS interfaces assume m is an batch of the number of bits to truncate

template <typename T>
void doOperation_TruncS(T **result, T **A, int bitlength, T **m, int size, int threadID, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    // this is a modification of Dalskov et al.'s private truncation protocol
    // M = bitlength
    // call pow2 on [bitlength - m], where m is private
    // call LT on ([m], ell) to get a bit
    // multiply [A]*[2^(M-m)]
    // call public truncation by M bits
    // use the bit from LT (which determined if the number of bits we truncated exceeded the bitlength of the input) to set the output to either zero, or the actual result of the truncation
    uint numShares = ss->getNumShares();
    uint ring_size = ss->ring_size;

    T **b = new T *[numShares];
    T **diff = new T *[numShares];
    for (size_t i = 0; i < numShares; i++) {
        b[i] = new T[size];
        diff[i] = new T[size];
    }
    // setting every value in bitlen_arr to bitlength (so we can easily use the LT interface)
    int *bitlen_arr = new int[size];
    for (size_t i = 0; i < size; i++) {
        bitlen_arr[i] = bitlength;
    }

    // computing LT([m], ell)
    doOperation_LT(b, bitlen_arr, m, ring_size, ring_size, ring_size, size, threadID, nodeNet, ss);

    // computing 2^(ell - [m])
    // double check the ring_size argument is correct
    doOperation_Pow2(diff, diff, ring_size, size, threadID, nodeNet, ss);

    // computing [A] * 2^(ell - [m])
    Mult(diff, A, diff, size, threadID, nodeNet, ss);

    // truncating  [A] * 2^(ell - [m]) by (bitlength)
    doOperation_Trunc(diff, diff, ring_size, bitlength, size, threadID, nodeNet, ss);

    // if b is 0, then the result is zero (we truncated by more bits than the bitlength of A)
    // otherwise, we just multiply by 1 (and we're done)
    Mult(result, b, diff, size, threadID, nodeNet, ss);

    for (size_t i = 0; i < numShares; i++) {
        delete[] b[i];
        delete[] diff[i];
    }
    delete[] b;
    delete[] diff;

    delete[] bitlen_arr;
}

#endif // _TRUNCS_HPP_