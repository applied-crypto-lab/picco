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

#ifndef BIT_UTILS_H_
#define BIT_UTILS_H_

#include <cstdio>
#include <string>
#include <sys/types.h>

// this is a suite of bit-level utilities, primarily used for certain RSS operations, as well as the seed generation algorithm

// gets the Nth bit of X (from right to left)
template <typename T>
inline __attribute__((always_inline)) T GET_BIT(T X, T N) {
    return (((X) >> (N)) & T(1));
}

// sets the Nth bit of X (from right to left) to B
template <typename T>
inline __attribute__((always_inline)) T SET_BIT(T X, T N, T B) {
    return (X & ~(T(1) << N)) | (B << N);
}

inline bool is_int(const std::string &str) {
    return str.find_first_not_of("0123456789") == std::string::npos;
}

inline bool is_float(const std::string &str) {
    return str.find_first_not_of(".0123456789") == std::string::npos;
}

template <typename T>
void print_binary(T n, uint size) {
    uint temp = size - 1;
    int i = size - 1;
    uint b;
    while (i != -1) {
        b = GET_BIT(n, temp);
        printf("%u", b);
        temp--;
        i -= 1;
    }
    printf("\n");
}

#endif
