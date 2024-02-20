#ifndef BITDEC_H_
#define BITDEC_H_

#include "Mult.h"
#include "NodeNetwork.h"
#include "Open.h"
#include "edaBit.h"
#include "mpc_util.h"
#include <cmath>
#include <stdio.h>
#include <sys/time.h>

void Rss_BitDec(Lint **res, Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet);

#endif
