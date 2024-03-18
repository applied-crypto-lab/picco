#ifndef LT_H_
#define LT_H_

#include "MSB.h"
#include "Mult.h"
#include "NodeNetwork.h"
#include "mpc_util.h"
#include <cmath>
#include <stdio.h>
#include <sys/time.h>

void Rss_LT(Lint **res, Lint **a, Lint **b, uint size, uint ring_size, NodeNetwork *nodeNet);
void new_Rss_LT(Lint **res, Lint **a, Lint **b, uint size, uint ring_size, NodeNetwork *nodeNet);
void fixed_Rss_LT(Lint **res, Lint **a, Lint **b, uint size, uint ring_size, uint layer, NodeNetwork *nodeNet);
void fixed_Rss_GT(Lint **res, Lint **a, Lint **b, uint size, uint ring_size, uint layer, NodeNetwork *nodeNet);
void fixed_Rss_LT_time(Lint **res, Lint **a, Lint **b, uint size, uint ring_size, uint layer, NodeNetwork *nodeNet, unsigned long &timer);
void fixed_Rss_GT_time(Lint **res, Lint **a, Lint **b, uint size, uint ring_size, uint layer, NodeNetwork *nodeNet, unsigned long &timer);
void fixed_Rss_GT_LT_time(Lint **res, Lint **a, Lint **b, uint size, uint ring_size, uint layer, NodeNetwork *nodeNet, unsigned long &timer);


void Rss_LT_5pc(Lint **res, Lint **a, Lint **b, uint size, uint ring_size, NodeNetwork *nodeNet);
void new_Rss_LT_5pc(Lint **res, Lint **a, Lint **b, uint size, uint ring_size, NodeNetwork *nodeNet);
void fixed_Rss_LT_5pc(Lint **res, Lint **a, Lint **b, uint size, uint ring_size, uint layer, NodeNetwork *nodeNet);
void fixed_Rss_GT_5pc(Lint **res, Lint **a, Lint **b, uint size, uint ring_size, uint layer, NodeNetwork *nodeNet);
void fixed_Rss_LT_time_5pc(Lint **res, Lint **a, Lint **b, uint size, uint ring_size, uint layer, NodeNetwork *nodeNet, unsigned long &timer);
void fixed_Rss_GT_time_5pc(Lint **res, Lint **a, Lint **b, uint size, uint ring_size, uint layer, NodeNetwork *nodeNet, unsigned long &timer);
void fixed_Rss_GT_LT_time_5pc(Lint **res, Lint **a, Lint **b, uint size, uint ring_size, uint layer, NodeNetwork *nodeNet, unsigned long &timer);


#endif
