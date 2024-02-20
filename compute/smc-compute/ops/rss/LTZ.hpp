#ifndef _LTZ_HPP_
#define _LTZ_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"

template <typename T>
void doOperation_LTZ(T **result, T **shares, int K, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_LT(T **result, T **a, T **b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_LT(T **result, int *a, T **b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {
}
template <typename T>
void doOperation_LT(T **result, T **a, int *b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {
}

#endif // _LTZ_HPP_