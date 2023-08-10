#ifndef B2A_H_ 
#define B2A_H_ 

#include "NodeNetwork.h"
#include "Mult.h"
#include "Open.h"
#include "mpc_util.h"
#include "randBit.h"
// #include "bitLT.h"
#include <cmath>
#include <stdio.h>
#include <sys/time.h>

void Rss_b2a(Lint **res, Lint **a, uint ring_size, uint size, NodeNetwork *nodeNet);
void Rss_b2a_5pc(Lint **res, Lint **a, uint ring_size, uint size, NodeNetwork *nodeNet);


#endif
