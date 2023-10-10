#include "ShamirOps.h"



void ss_process_operands(mpz_t **a1, mpz_t **b1, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int *len_sig, int *len_exp, int size, SecretShare *ss) {
    mpz_t **a = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    mpz_t **b = (mpz_t **)malloc(sizeof(mpz_t *) * 4);

    for (int i = 0; i < 4; i++) {
        a[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        b[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++) {
            mpz_init_set(a[i][j], a1[j][i]);
            mpz_init_set(b[i][j], b1[j][i]);
        }
    }

    int sig = alen_sig >= blen_sig ? alen_sig : blen_sig;
    int exp = alen_exp >= blen_exp ? alen_exp : blen_exp;

    int diff = (int)abs(alen_sig - blen_sig);

    mpz_t pow;
    mpz_init(pow);
    mpz_ui_pow_ui(pow, 2, exp);

    // compute the log(pow+diff) which will be the final value for *len_exp
    if (mpz_cmp_ui(pow, diff) >= 0)
        exp++;
    else
        exp = ceil(log(diff)) + 1;

    *len_sig = sig;
    *len_exp = exp;

    // convert both opeators a and b to ensure their significands and exponents will be in the correct range (sig, exp)
    if (alen_sig < sig) {
        mpz_ui_pow_ui(pow, 2, sig - alen_sig);
        ss->modMul(a[0], a[0], pow, size);
        ss->modSub(a[1], a[1], diff, size);
    }

    if (blen_sig < sig) {
        mpz_ui_pow_ui(pow, 2, sig - blen_sig);
        ss->modMul(b[0], b[0], pow, size);
        ss->modSub(b[1], b[1], diff, size);
    }

    // copy the result back to a1 and b1
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < size; j++) {
            mpz_set(a1[j][i], a[i][j]);
            mpz_set(b1[j][i], b[i][j]);
        }

    // free the memory
    mpz_clear(pow);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < size; j++) {
            mpz_clear(a[i][j]);
            mpz_clear(b[i][j]);
        }
        free(a[i]);
        free(b[i]);
    }
    free(a);
    free(b);

    return;
}
void ss_process_results(mpz_t **result1, int resultlen_sig, int resultlen_exp, int len_sig, int len_exp, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **result = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    for (int i = 0; i < 4; i++) {
        result[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++)
            mpz_init_set(result[i][j], result1[j][i]);
    }

    // truncate results by 2^{len_sig-resultlen_sig}
    if (resultlen_sig < len_sig) {
        doOperation_Trunc(result[0], result[0], len_sig, resultlen_sig, size, threadID, net, id, ss);
        ss->modAdd(result[1], result[1], len_sig - resultlen_sig, size);
    }

    // multiply results by 2^{resultlen_sig-len_sig}
    else if (resultlen_sig > len_sig) {
        mpz_t pow;
        mpz_init(pow);
        mpz_ui_pow_ui(pow, 2, resultlen_sig - len_sig);
        ss->modMul(result[0], result[0], pow, size);
        ss->modSub(result[1], result[1], resultlen_sig - len_sig, size);
        mpz_clear(pow);
    }

    // copy the result back to result1
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < size; j++)
            mpz_set(result1[j][i], result[i][j]);

    // free the memory
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < size; j++)
            mpz_clear(result[i][j]);
        free(result[i]);
    }
    free(result);
}


void ss_single_convert_to_private_float(float a, mpz_t **priv_a, int len_sig, int len_exp, SecretShare *ss) {
    mpz_t *elements = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    *priv_a = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    for (int i = 0; i < 4; i++) {
        mpz_init((*priv_a)[i]);
        mpz_init(elements[i]);
    }
    convertFloat(a, len_sig, len_exp, &elements);
    for (int i = 0; i < 4; i++) {
        mpz_set((*priv_a)[i], elements[i]);
        // convert the negative values to the positives in the filed
        ss->modAdd((*priv_a)[i], (*priv_a)[i], (long)0);
    }
    // free the memory
    smc_batch_free_operator(&elements, 4);
}


void ss_single_fop_comparison(mpz_t result, mpz_t *a, mpz_t *b, int resultlen, int alen_sig, int alen_exp, int blen_sig, int blen_exp, std::string op, int threadID, NodeNetwork net, int id, SecretShare *ss) {

    mpz_t *results = (mpz_t *)malloc(sizeof(mpz_t));
    mpz_t **as = (mpz_t **)malloc(sizeof(mpz_t *));
    mpz_t **bs = (mpz_t **)malloc(sizeof(mpz_t *));

    as[0] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    bs[0] = (mpz_t *)malloc(sizeof(mpz_t) * 4);

    for (int i = 0; i < 4; i++) {
        mpz_init_set(as[0][i], a[i]);
        mpz_init_set(bs[0][i], b[i]);
    }

    mpz_init(results[0]);

    /***********************************************/
    int len_sig = 0, len_exp = 0;
    ss_process_operands(as, bs, alen_sig, alen_exp, blen_sig, blen_exp, &len_sig, &len_exp, 1, ss);
    /***********************************************/

    if (!strcmp(op.c_str(), "<0"))
        doOperation_FLLTZ(as, bs, results, len_sig, len_exp, 1, threadID, net, id, ss);
    else if (!strcmp(op.c_str(), "=="))
        doOperation_FLEQZ(as, bs, results, len_sig, len_exp, 1, threadID, net, id, ss);

    mpz_set(result, results[0]);

    // free the memory
    smc_batch_free_operator(&results, 1);
    smc_batch_free_operator(&as, 1);
    smc_batch_free_operator(&bs, 1);
}
void ss_batch_fop_comparison(mpz_t *result, mpz_t **a, mpz_t **b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID, NodeNetwork net, int id, SecretShare *ss) {

    int len_sig = 0, len_exp = 0;
    ss_process_operands(a, b, alen_sig, alen_exp, blen_sig, blen_exp, &len_sig, &len_exp, size, ss);
    if (!strcmp(op.c_str(), "<0"))
        doOperation_FLLTZ(a, b, result, len_sig, len_exp, size, threadID, net, id, ss);
    else if (!strcmp(op.c_str(), "=="))
        doOperation_FLEQZ(a, b, result, len_sig, len_exp, size, threadID, net, id, ss);
}

void ss_batch_fop_arithmetic(mpz_t **result, mpz_t **a, mpz_t **b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int size, std::string op, int threadID, NodeNetwork net, int id, SecretShare *ss) {

    int len_sig = 0, len_exp = 0;
    ss_process_operands(a, b, alen_sig, alen_exp, blen_sig, blen_exp, &len_sig, &len_exp, size, ss);
    if (!strcmp(op.c_str(), "*"))
        doOperation_FLMult(a, b, result, len_sig, size, threadID, net, id, ss);
    else if (!strcmp(op.c_str(), "+"))
        doOperation_FLAdd(a, b, result, len_sig, len_exp, size, threadID, net, id, ss);
    else if (!strcmp(op.c_str(), "/"))
        doOperation_FLDiv(a, b, result, len_sig, size, threadID, net, id, ss);
    ss_process_results(result, resultlen_sig, resultlen_exp, len_sig, len_exp, size, threadID, net, id, ss);
}
