#ifndef MATMULT_H_
#define MATMULT_H_

#include "NodeNetwork.h"
#include <cmath>
#include <stdio.h>
#include <sys/time.h>

void Rss_MatMult(Lint ***c, Lint ***a, Lint ***b, uint m, uint n, uint s, uint ring_size, NodeNetwork *nodeNet);
void Rss_MatMultArray(Lint **c, Lint **a, Lint **b, uint m, uint n, uint s, uint ring_size, NodeNetwork *nodeNet);
void Rss_MatMultArray_batch(Lint **c, Lint **a, Lint **b, uint m, uint n, uint s, uint ring_size, uint batch_size, uint weight_flag_a, uint weight_flag_b, NodeNetwork *nodeNet);
void Rss_dw_matrixmul(Lint **res, Lint **x, Lint **W, uint width, uint height, uint channels, uint filter, uint ring_size, uint batch_size, uint stride, NodeNetwork *nodeNet);
void Rss_pw_matrixmul(Lint **res, Lint **x, Lint **W, uint width, uint height, uint in_channels, uint kernel, uint out_channels, uint ring_size, uint batch_size, NodeNetwork *nodeNet);

void Rss_MatMultArray_batch_7pc(Lint **c, Lint **a, Lint **b, uint m, uint n, uint s, uint ring_size, uint batch_size, uint weight_flag_a, uint weight_flag_b,  NodeNetwork *nodeNet);



void Rss_MatMultArray_batch_5pc(Lint **c, Lint **a, Lint **b, uint m, uint n, uint s, uint ring_size, uint batch_size, uint weight_flag_a, uint weight_flag_b,  NodeNetwork *nodeNet);
void Rss_dw_matrixmul_5pc(Lint **res, Lint **x, Lint **W, uint width, uint height, uint channels, uint filter, uint ring_size, uint batch_size, uint stride,  NodeNetwork *nodeNet);
void Rss_pw_matrixmul_5pc(Lint **res, Lint **x, Lint **W, uint width, uint height, uint in_channels, uint kernel, uint out_channels, uint ring_size, uint batch_size,  NodeNetwork *nodeNet);

#endif
