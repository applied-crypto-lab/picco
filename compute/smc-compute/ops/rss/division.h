#ifndef DIV_H_
#define DIV_H_

#include "MSB.h"
#include "LT.h"
#include "Mult.h"
#include "PreOR.h"
#include "bitDec.h"
#include "NodeNetwork.h"
#include "mpc_util.h"
#include <cmath>
#include <stdio.h>
#include <sys/time.h>

void Rss_Div(Lint **res, Lint **a, Lint **b, uint bitlength, uint size, uint ring_size, NodeNetwork *nodeNet);
void Rss_AppRcr(Lint **res_w, Lint **a, Lint **b, uint bitlength, uint size, uint ring_size, NodeNetwork *nodeNet);
void Rss_Norm(Lint **res_c, Lint **res_v, Lint **b, uint bitlength, uint size, uint ring_size, NodeNetwork *nodeNet);


#endif
