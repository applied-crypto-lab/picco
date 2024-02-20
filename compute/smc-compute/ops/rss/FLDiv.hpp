#ifndef _FLDIV_HPP_
#define _FLDIV_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"

template <typename T>
void doOperation_FLDiv_Pub(T ***A1, T ***B1, T ***result1, int K, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){}
template <typename T>
void doOperation_FLDiv(T ***A1, T ***B1, T ***result1, int K, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){}

#endif // _FLDIV_HPP_