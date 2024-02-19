#ifndef _ADDBITWISE_HPP_
#define _ADDBITWISE_HPP_

#include "NodeNetwork.h"
#include "Mult.h"
#include "mpc_util.h"
#include <cmath>
#include <stdio.h>
#include <sys/time.h>

void Rss_nBitAdd(Lint **res, Lint **r_bitwise, uint ring_size, uint size, NodeNetwork *nodeNet);
void Rss_BitAdd(Lint **res, Lint **a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet);
void Rss_BitAdd(Lint **res, Lint *a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet);
void Rss_CircleOpL(Lint **d, uint r_size, uint size, NodeNetwork *nodeNet);
void Rss_CircleOpL_Lint(Lint **d, uint r_size, uint size, NodeNetwork *nodeNet);
void CarryBuffer2(Lint **buffer, Lint **d, uint **index_array, uint size, uint k, uint numShares);
void CarryBuffer_Lint(Lint **a_prime, Lint **b_prime, Lint **d, uint **index_array, uint size, uint k, uint numShares);


void Rss_nBitAdd_5pc(Lint **res, Lint **r_bitwise, uint ring_size, uint size, NodeNetwork *nodeNet);
void Rss_BitAdd_5pc(Lint **res, Lint **a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet);
void Rss_BitAdd_5pc(Lint **res, Lint *a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet);
void Rss_CircleOpL_5pc(Lint **d, uint r_size, uint size, NodeNetwork *nodeNet);
void Rss_CircleOpL_mp_test(Lint **d, uint r_size, uint size, NodeNetwork *nodeNet);
void CarryBuffer2_test(Lint **buffer, Lint **d, uint **index_array, uint size, uint k, uint numShares);
void Rss_CircleOpL_Lint_5pc(Lint **d, uint r_size, uint size, NodeNetwork *nodeNet);



void Rss_nBitAdd_7pc(Lint **res, Lint **r_bitwise, uint ring_size, uint size, NodeNetwork *nodeNet);
void Rss_BitAdd_7pc(Lint **res, Lint **a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet);
void Rss_BitAdd_7pc(Lint **res, Lint *a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet);
void Rss_CircleOpL_7pc(Lint **d, uint r_size, uint size, NodeNetwork *nodeNet);
void Rss_CircleOpL_7pc_test(Lint **d, uint r_size, uint size, NodeNetwork *nodeNet);
void Rss_CircleOpL_Lint_7pc(Lint **d, uint r_size, uint size, NodeNetwork *nodeNet);


#endif
