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

#ifndef _PRIVINDEX_HPP_
#define _PRIVINDEX_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"

template <typename T>
void compute_private_conditions(T **private_conditions, T *out_cond, T **priv_cond, int counter, int size) {
}
template <typename T>
void doOperation_PrivIndex_int(T *index, T **array, T *result, int dim, int type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_float(T *index, T ***array, T **result, int dim, int type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_int_arr(T *index, T ***array, T *result, int dim1, int dim2, int type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_float_arr(T *index, T ****array, T **result, int dim1, int dim2, int type, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_Read(T **index, T **array, T **result, int dim, int size, int threadID, int type, NodeNetwork net, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_Write(T **index, T **array, T **value, int dim, int size, T *out_cond, T **priv_cond, int counter, int threadID, int type, NodeNetwork net, replicatedSecretShare<T> *ss) {
}
template <typename T>
void AllOr(T ***array, int begin, int size, T ***result, int batch_size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_Write(T **index, T **array, int *values, int dim, int size, T *out_cond, T **priv_cond, int counter, int threadID, int type, NodeNetwork net, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_Write_2d(T **index, T ***array, int *values, int dim1, int dim2, int size, T *out_cond, T **priv_cond, int counter, int threadID, int type, NodeNetwork net, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_Write_2d(T **index, T ***array, T **values, int dim1, int dim2, int size, T *out_cond, T **priv_cond, int counter, int threadID, int type, NodeNetwork net, replicatedSecretShare<T> *ss) {
}
#endif // _PRIVINDEX_HPP_