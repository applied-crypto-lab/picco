#ifndef MULT_7PC_H_
#define MULT_7PC_H_

#include "NodeNetwork.h"
#include <cmath>
#include <stdio.h>
#include <sys/time.h>

void Rss_Mult_7pc(Lint **c, Lint **a, Lint **b, uint size, uint ring_size, NodeNetwork *nodeNet) ;
// inline bool chi_p_prime_in_T_7(int p_prime, int *T_map, uint n) ;
// inline bool p_prime_in_T_7(int p_prime, int *T_map) ;


void Rss_MultPub_7pc(Lint *c, Lint **a, Lint **b, uint size, uint ring_size, NodeNetwork *nodeNet) ;


void Rss_Mult_Bitwise_7pc(Lint **c, Lint **a, Lint **b, uint size, uint ring_size, NodeNetwork *nodeNet) ;
void Rss_Mult_Byte_7pc(uint8_t **c, uint8_t **a, uint8_t **b, uint size, NodeNetwork *nodeNet) ;


#endif
