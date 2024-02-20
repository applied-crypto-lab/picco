#ifndef BITADDTRUNC_H_
#define BITADDTRUNC_H_

#include "NodeNetwork.h"
#include "Mult.h"
#include "bitAdd.h"
#include "mpc_util.h"
#include <cmath>
#include <stdio.h>
#include <sys/time.h>

void Rss_nBitAdd_trunc(Lint **res, Lint **carry, Lint **r_bitwise, uint ring_size, uint m, uint size, NodeNetwork *nodeNet);
void Rss_BitAdd_trunc(Lint **res, Lint **carry, Lint **a, Lint **b, uint ring_size, uint m, uint size, NodeNetwork *nodeNet);
void Rss_nBitAdd_trunc_5pc(Lint **res, Lint **carry, Lint **r_bitwise, uint ring_size, uint m, uint size, NodeNetwork *nodeNet);
void Rss_BitAdd_trunc_5pc(Lint **res, Lint **carry, Lint **a, Lint **b, uint ring_size, uint m, uint size, NodeNetwork *nodeNet);


#endif
