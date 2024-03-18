#ifndef PREOR_H_
#define PREOR_H_

#include "Mult.h"
#include "NodeNetwork.h"
#include "Open.h"
#include "edaBit.h"
#include "mpc_util.h"
#include <cmath>
#include <stdio.h>
#include <sys/time.h>

void Rss_PreOR( Lint **a, uint size, uint ring_size, NodeNetwork *nodeNet);

void CarryBufferPreOR(Lint **buffer, Lint **a, uint **index_array, uint size, uint k, uint numShares);

#endif
