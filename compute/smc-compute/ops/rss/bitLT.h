#ifndef BITLT_H_
#define BITLT_H_

#include "Mult.h"
#include "NodeNetwork.h"
#include "mpc_util.h"
#include <cmath>
#include <stdio.h>
#include <sys/time.h>

void Rss_BitLT(Lint **res, Lint *a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet);
void Rss_CarryOut(Lint **res, Lint *a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet);
void Rss_CarryOutAux(Lint **res, Lint **d, uint r_size, uint size, NodeNetwork *nodeNet);
void CarryBuffer(Lint **buffer, Lint **d, uint size, uint r_size);
void OptimalBuffer(Lint **buffer, Lint **d, uint size, uint r_size, NodeNetwork *nodeNet);
void new_Rss_CarryOutAux(Lint **res, Lint **d, uint ring_size, uint size, NodeNetwork *nodeNet);
void CircleOp(Lint **res, Lint *p, Lint *g, uint size, uint r_size);
void new_Rss_CarryOutAux_Lint(Lint **res, Lint **d, uint ring_size, uint size, NodeNetwork *nodeNet);

void Rss_BitLT_5pc(Lint **res, Lint *a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet);
void Rss_CarryOut_5pc(Lint **res, Lint *a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet);
void OptimalBuffer_5pc(Lint **buffer, Lint **d, uint size, uint r_size, NodeNetwork *nodeNet);
void new_Rss_CarryOutAux_5pc(Lint **res, Lint **d, uint ring_size, uint size, NodeNetwork *nodeNet);

void Rss_BitLT_7pc(Lint **res, Lint *a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet);
void Rss_CarryOut_7pc(Lint **res, Lint *a, Lint **b, uint ring_size, uint size, NodeNetwork *nodeNet);
void OptimalBuffer_7pc(Lint **buffer, Lint **d, uint size, uint r_size, NodeNetwork *nodeNet);
void new_Rss_CarryOutAux_7pc(Lint **res, Lint **d, uint ring_size, uint size, NodeNetwork *nodeNet);

#endif
