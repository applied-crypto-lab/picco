#ifndef _INTDIV_HPP_
#define _INTDIV_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"

template <typename T>
void doOperation_IntDiv_Pub(T *result, T *a, int b, int k, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {
}

template <typename T>
void doOperation_IntDiv_Pub(T **result, T **a, int *b, int k, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {
}

template <typename T>
void doOperation_IntDiv_Pub(T **result, T **a, T **b, int k, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {
}

template <typename T>
void doOperation_IntDiv(T *result, T *a, T *b, int k, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {
}

template <typename T>
void doOperation_IntDiv(T **result, T **a, T **b, int k, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {
}

template <typename T>
void doOperation_IntDiv(T *result, int a, T *b, int k, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {
}

template <typename T>
void doOperation_IntDiv(T **result, int *a, T **b, int k, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {
}

#endif // _INTDIV_HPP_