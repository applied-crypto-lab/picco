#ifndef TRUNC_H_
#define TRUNC_H_

#include "edaBit.h"
#include "Mult.h"
#include "NodeNetwork.h"
#include <cmath>
#include <stdio.h>
#include <sys/time.h>

void Rss_truncPriv(Lint **res, Lint **x, Lint **priv_m, Lint public_M, uint priv_m_index, uint size, uint ring_size,  NodeNetwork *nodeNet) ;
void Rss_truncPriv_time(Lint **res, Lint **x, Lint **priv_m, Lint public_M, uint priv_m_index, uint size, uint ring_size,  NodeNetwork *nodeNet, unsigned long &timer) ;
void Rss_truncPr_1(Lint **res, Lint **x, Lint m, uint size, uint ring_size,  NodeNetwork *nodeNet) ;
void Rss_truncPr_time(Lint **res, Lint **x, Lint m, uint size, uint ring_size,  NodeNetwork *nodeNet, unsigned long &timer) ;


void Rss_truncPriv_5pc(Lint **res, Lint **x, Lint **priv_m, Lint public_M, uint priv_m_index, uint size, uint ring_size,  NodeNetwork *nodeNet) ;
void Rss_truncPriv_time_5pc(Lint **res, Lint **x, Lint **priv_m, Lint public_M, uint priv_m_index, uint size, uint ring_size,  NodeNetwork *nodeNet, unsigned long &timer) ;
void Rss_truncPr_1_5pc(Lint **res, Lint **x, Lint m, uint size, uint ring_size,  NodeNetwork *nodeNet) ;
void Rss_truncPr_time_5pc(Lint **res, Lint **x, Lint m, uint size, uint ring_size,  NodeNetwork *nodeNet, unsigned long &timer) ;


#endif
