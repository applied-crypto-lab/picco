#ifndef EDABIT_H_
#define EDABIT_H_

#include "NodeNetwork.h"
#include "b2a.h"
#include "bitAdd.h"
#include "bitAddTrunc.h"
#include <cmath>
#include <stdio.h>
#include <sys/time.h>

void Rss_edaBit(Lint **r, Lint **b_2, uint size, uint ring_size, NodeNetwork *nodeNet);
void Rss_GenerateRandomShares(Lint **res, Lint **res_bitwise, uint ring_size, uint size, NodeNetwork *nodeNet);
void Rss_edaBit(Lint **r, Lint **b_2, uint size, uint ring_size, uint bit_length, NodeNetwork *nodeNet);
void Rss_GenerateRandomShares(Lint **res, Lint **res_bitwise, uint ring_size, uint bit_length, uint size, NodeNetwork *nodeNet);
// void Rss_edaBit_trunc(Lint **r, Lint **r_prime, Lint **b_2, uint size, uint ring_size, uint m,  NodeNetwork *nodeNet) ;
void Rss_edaBit_trunc(Lint **r, Lint **r_prime, Lint **r_km1, uint size, uint ring_size, uint m, NodeNetwork *nodeNet);
void Rss_edaBit_trunc_test(Lint **r, Lint **r_prime, Lint **r_km1, uint size, uint ring_size, uint m, NodeNetwork *nodeNet);
void Rss_GenerateRandomShares_trunc(Lint **res, Lint **res_prime, Lint **res_bitwise, uint ring_size, uint m, uint size, NodeNetwork *nodeNet);

void Rss_edaBit_7pc(Lint **r, Lint **b_2, uint size, uint ring_size, NodeNetwork *nodeNet);
void Rss_GenerateRandomShares_7pc(Lint **res, Lint **res_bitwise, uint ring_size, uint size, NodeNetwork *nodeNet);


void Rss_edaBit_5pc(Lint **r, Lint **b_2, uint size, uint ring_size, NodeNetwork *nodeNet);
void Rss_GenerateRandomShares_5pc(Lint **res, Lint **res_bitwise, uint ring_size, uint size, NodeNetwork *nodeNet);
void Rss_edaBit_trunc_5pc(Lint **r, Lint **r_prime, Lint **r_km1, uint size, uint ring_size, uint m, NodeNetwork *nodeNet);
void Rss_GenerateRandomShares_trunc_5pc(Lint **res, Lint **res_prime, Lint **res_bitwise, uint ring_size, uint m, uint size, NodeNetwork *nodeNet);


#endif
