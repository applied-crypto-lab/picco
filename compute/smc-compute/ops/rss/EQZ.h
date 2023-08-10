#ifndef EQZ_H_
#define EQZ_H_

#include "Mult.h"
#include "NodeNetwork.h"
#include "edaBit.h"
#include "mpc_util.h"
#include "randBit.h"
#include <cmath>
#include <stdio.h>
#include <sys/time.h>

void Rss_EQZ(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet);
void Rss_k_OR_L(Lint **res, Lint **r, uint size, uint ring_size, NodeNetwork *nodeNet);

void Rss_EQZ_5pc(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet);
void Rss_k_OR_L_5pc(Lint **res, Lint **r, uint size, uint ring_size, NodeNetwork *nodeNet);


#endif
