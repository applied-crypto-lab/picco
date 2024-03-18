
#ifndef TRUNC_SHAMIR_H_
#define TRUNC_SHAMIR_H_

#include "Mod2M.h"
#include "Operation.h"

void doOperation_Trunc(mpz_t *result, mpz_t *shares1, int K, int M, int size, int threadID, NodeNetwork net,  SecretShare *ss);
// void doOperation_Trunc(mpz_t *result, mpz_t *shares1, int K, mpz_t M, int size, int threadID, NodeNetwork net,  SecretShare *ss);
void doOperation_Trunc(mpz_t *result, mpz_t *shares1, int K, int *M, int size, int threadID, NodeNetwork net,  SecretShare *ss);

#endif /* TRUNC_SHAMIR_H_ */
