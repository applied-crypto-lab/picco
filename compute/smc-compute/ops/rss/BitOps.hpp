#ifndef _BITOPS_HPP_
#define _BITOPS_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"

template <typename T>
void BitAnd(T **A, T **B, T **result, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {}

template <typename T>
void BitOr(T **A, T **B, T **result, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {}

template <typename T>
void BitXor(T **A, T **B, T **result, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {}

#endif // _BITOPS_HPP_