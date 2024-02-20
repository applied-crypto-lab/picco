#ifndef _TRUNCS_HPP_
#define _TRUNCS_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"

template <typename T>
void doOperation_TruncS(T **result, T **A, int K, T **M, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){}

#endif // _TRUNCS_HPP_