
#ifndef SHAMIROPS_H_
#define SHAMIROPS_H_

#include "Headers.h"
#include <fstream>
#include <iostream>
#include <math.h>
#include <sstream>
#include <string>


void ss_process_operands(mpz_t **a1, mpz_t **b1, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int *len_sig, int *len_exp, int size, SecretShare *ss) ;
void ss_process_results(mpz_t **result1, int resultlen_sig, int resultlen_exp, int len_sig, int len_exp, int size, int threadID, NodeNetwork net, int id, SecretShare *ss);

void ss_single_convert_to_private_float(float a, mpz_t **priv_a, int len_sig, int len_exp, SecretShare *ss);

void ss_single_fop_comparison(mpz_t result, mpz_t *a, mpz_t *b, int resultlen, int alen_sig, int alen_exp, int blen_sig, int blen_exp, std::string op, int threadID, NodeNetwork net, int id, SecretShare *ss);
void ss_batch_fop_comparison(mpz_t *result, mpz_t **a, mpz_t **b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID, NodeNetwork net, int id, SecretShare *ss);
void ss_batch_fop_arithmetic(mpz_t **result, mpz_t **a, mpz_t **b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID, NodeNetwork net, int id, SecretShare *ss);


#endif
