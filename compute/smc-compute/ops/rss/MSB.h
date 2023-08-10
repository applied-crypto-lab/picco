#ifndef MSB_H_
#define MSB_H_

#include "NodeNetwork.h"
#include "Mult.h"
#include "bitLT.h"
#include "edaBit.h"
#include "randBit.h"
#include <cmath>
#include <stdio.h>
#include <sys/time.h>

void new_Rss_MSB(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet);
void new_Rss_MSB_time(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet, unsigned long &timer);
void test_new_Rss_MSB(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet);
void Rss_MSB(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet);


void new_Rss_MSB_5pc(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet) ;
void new_Rss_MSB_time_5pc(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet, unsigned long &timer) ;
void Rss_MSB_5pc(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet) ;


void new_Rss_MSB_7pc(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet) ;
void Rss_MSB_7pc(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet) ;

#endif
