#ifndef _DOTPRODUCT_HPP_
#define _DOTPRODUCT_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"

template <typename T>

void doOperation_DotProduct(T**a, T**b, T*result, int array_size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {}

template <typename T>
void doOperation_DotProduct(T* **a, T* **b, T* *result, int batch_size, int array_size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss) {}

#endif // _DOTPRODUCT_HPP_
