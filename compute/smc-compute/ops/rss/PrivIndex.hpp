#ifndef _PRIVINDEX_HPP_
#define _PRIVINDEX_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"

template <typename T>
void compute_private_conditions(T **private_conditions, T *out_cond, T **priv_cond, int counter, int size) {
}
template <typename T>
void doOperation_PrivIndex_int(T *index, T **array, T *result, int dim, int type, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_float(T *index, T ***array, T **result, int dim, int type, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_int_arr(T *index, T ***array, T *result, int dim1, int dim2, int type, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_float_arr(T *index, T ****array, T **result, int dim1, int dim2, int type, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_Read(T **index, T **array, T **result, int dim, int size, int threadID, int type, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_Write(T **index, T **array, T **value, int dim, int size, T *out_cond, T **priv_cond, int counter, int threadID, int type, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {
}
template <typename T>
void AllOr(T ***array, int begin, int size, T ***result, int batch_size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_Write(T **index, T **array, int *values, int dim, int size, T *out_cond, T **priv_cond, int counter, int threadID, int type, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_Write_2d(T **index, T ***array, int *values, int dim1, int dim2, int size, T *out_cond, T **priv_cond, int counter, int threadID, int type, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_PrivIndex_Write_2d(T **index, T ***array, T **values, int dim1, int dim2, int size, T *out_cond, T **priv_cond, int counter, int threadID, int type, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {
}
#endif // _PRIVINDEX_HPP_