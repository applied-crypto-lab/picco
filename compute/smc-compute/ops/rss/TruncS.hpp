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

template <typename T>
void doOperation_TruncS(T **result, T **A, int bitlength, T **m, int size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss){
// this is a modification of Dalskov et al.'s private truncation protocol
// M = bitlength 
// call pow2 on [bitlength - m], where m is private
// call LT on ([m], ell) to get a bit
// multiply [A]*[2^(M-m)]
// call public truncation by M bits
// use the bit from LT (which determined if the number of bits we truncated exceeded the bitlength of the input) to set the output to either zero, or the actual result of the truncation

}

#endif // _TRUNCS_HPP_