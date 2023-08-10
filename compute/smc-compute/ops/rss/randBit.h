#ifndef RANDBIT_H_
#define RANDBIT_H_

#include "NodeNetwork.h"
#include "Mult.h"
#include <cmath>
#include <stdio.h>
#include <sys/time.h>

void Rss_RandBit(Lint **b, uint size, uint ring_size, NodeNetwork *nodeNet);
void rss_sqrt_inv(Lint *c, Lint *e, uint size, uint ring_size);
void Rss_RandBit_5pc(Lint **b, uint size, uint ring_size, NodeNetwork *nodeNet);

void Rss_RandBit_7pc(Lint **b, uint size, uint ring_size, NodeNetwork *nodeNet);


#endif
