#ifndef _TRUNC_HPP_
#define _TRUNC_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"

template <typename T>
void doOperation_Trunc(T **result, T **shares1, int K, int M, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){}
template <typename T>
void doOperation_Trunc(T **result, T **shares1, int K, int *M, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){}

#endif // _TRUNC_HPP_