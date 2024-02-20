#ifndef _EQZ_HPP_
#define _EQZ_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"

template <typename T>
void doOperation_EQZ(T **shares, T **result, int K, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){
    
}
template <typename T>
void doOperation_EQZ(T **result, T **a, T **b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){
    
}
template <typename T>
void doOperation_EQZ(T **result, T **a, int *b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){
    
}
template <typename T>
void doOperation_EQZ_bit(T **result, T **a, T **b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){
    
}
template <typename T>
void doOperation_EQZ_bit(T **result, T **a, int *b, int alen, int blen, int resultlen, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){
    
}

#endif // _EQZ_HPP_