#ifndef MULT_5PC_H_
#define MULT_5PC_H_

#include "NodeNetwork.h"
#include <cmath>
#include <stdio.h>
#include <sys/time.h>


void test_prg(uint size, uint ring_size, NodeNetwork *nodeNet);
void Rss_Mult_Bitwise_5pc(Lint **c, Lint **a, Lint **b, uint size, uint ring_size, NodeNetwork *nodeNet);
void Rss_Mult_5pc(Lint **c, Lint **a, Lint **b, uint size, uint ring_size, NodeNetwork *nodeNet);
void Rss_Mult_fixed_b_5pc(Lint **c, Lint **a, Lint **b, uint b_index, uint size, uint ring_size, NodeNetwork *nodeNet);
void Rss_Mult_Byte_5pc(uint8_t **c, uint8_t **a, uint8_t **b, uint size, NodeNetwork *nodeNet);
void Rss_MultPub_5pc(Lint *c, Lint **a, Lint **b, uint size, uint ring_size, NodeNetwork *nodeNet);
void test_prg(uint size, uint ring_size, NodeNetwork *nodeNet);

#endif
