#ifndef _FLMULT_HPP_
#define _FLMULT_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"

template <typename T>

void doOperation_FLMult(T ***A, T ***B, T ***result, int K, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){}

#endif // _FLMULT_HPP_