#include "ShamirOps.h"


void ss_init_set(mpz_t x, mpz_t x_val) {
    mpz_init_set(x, x_val);
}

void ss_init(mpz_t x) {
    mpz_init(x);
}
void ss_set(mpz_t x, mpz_t x_val){
    mpz_set(x, x_val);
}


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
void ss_batch_convert_to_private_float(float *a, mpz_t ***priv_a, int len_sig, int len_exp, int size, SecretShare *ss) {
    mpz_t *elements = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    *priv_a = (mpz_t **)malloc(sizeof(mpz_t *) * size);

    for (int i = 0; i < 4; i++)
        mpz_init(elements[i]);

    for (int i = 0; i < size; i++) {
        (*priv_a)[i] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
        for (int j = 0; j < 4; j++)
            mpz_init((*priv_a)[i][j]);
    }

    for (int i = 0; i < size; i++) {
        convertFloat(a[i], len_sig, len_exp, &elements);
        for (int j = 0; j < 4; j++) {
            mpz_set((*priv_a)[i][j], elements[j]);
            ss->modAdd((*priv_a)[i][j], (*priv_a)[i][j], (long)0);
        }
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
void ss_single_fop_arithmetic(mpz_t *result, mpz_t *a, mpz_t *b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, std::string op, int threadID, NodeNetwork net, int id, SecretShare *ss) {

    mpz_t **results = (mpz_t **)malloc(sizeof(mpz_t *));
    mpz_t **as = (mpz_t **)malloc(sizeof(mpz_t *));
    mpz_t **bs = (mpz_t **)malloc(sizeof(mpz_t *));

    results[0] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    as[0] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    bs[0] = (mpz_t *)malloc(sizeof(mpz_t) * 4);

    for (int i = 0; i < 4; i++) {
        mpz_init(results[0][i]);
        mpz_init_set(as[0][i], a[i]);
        mpz_init_set(bs[0][i], b[i]);
    }

    /*********************************************************/
    int len_sig = 0, len_exp = 0;
    ss_process_operands(as, bs, alen_sig, alen_exp, blen_sig, blen_exp, &len_sig, &len_exp, 1, ss);

    /*********************************************************/

    if (!strcmp(op.c_str(), "*"))
        doOperation_FLMult(as, bs, results, len_sig, 1, threadID, net, id, ss);
    else if (!strcmp(op.c_str(), "+"))
        doOperation_FLAdd(as, bs, results, len_sig, len_exp, 1, threadID, net, id, ss);
    else if (!strcmp(op.c_str(), "/"))
        doOperation_FLDiv(as, bs, results, len_sig, 1, threadID, net, id, ss);

    // convert the result with size len_sig and len_exp to resultlen_sig and resultlen_exp
    ss_process_results(results, resultlen_sig, resultlen_exp, len_sig, len_exp, 1, threadID, net, id, ss);
    for (int i = 0; i < 4; i++)
        mpz_set(result[i], results[0][i]);
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

void ss_set(mpz_t *a, mpz_t *result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **results = (mpz_t **)malloc(sizeof(mpz_t *));
    results[0] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    for (int i = 0; i < 4; i++)
        mpz_init_set(results[0][i], a[i]);
    ss_process_results(results, resultlen_sig, resultlen_exp, alen_sig, alen_exp, 1, threadID, net, id, ss);
    for (int i = 0; i < 4; i++)
        mpz_set(result[i], results[0][i]);
    smc_batch_free_operator(&results, 1);
}

void ss_set(mpz_t **a, mpz_t **result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    for (int i = 0; i < size; i++)
        ss_set(a[i], result[i], alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID, net, id, ss);
}

void ss_set(float a, mpz_t *result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    convertFloat(a, resultlen_sig, resultlen_exp, &result);
    for (int i = 0; i < 4; i++)
        ss->modAdd(result[i], result[i], (long)0);
}

void ss_set(mpz_t a, mpz_t result, int alen, int resultlen, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_init_set(result, a);
}

// void ss_set(mpz_t *a, mpz_t *result, int alen, int resultlen, int size, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
//     for (int i = 0; i < size; i++)
//         ss_set(a[i], result[i], alen, resultlen, type, threadID, net, id, ss);
// }

// this routine should implement in a way that result = a + share[0]
void ss_set(int a, mpz_t result, int alen, int resultlen, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_set_ui(result, 0);
    ss->modAdd(result, result, a);
}

void ss_priv_eval(mpz_t a, mpz_t b, mpz_t cond, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *results = (mpz_t *)malloc(sizeof(mpz_t));
    mpz_t *op1 = (mpz_t *)malloc(sizeof(mpz_t));
    mpz_t *op2 = (mpz_t *)malloc(sizeof(mpz_t));
    mpz_init(op1[0]);
    mpz_init_set(op2[0], cond);
    mpz_init(results[0]);

    ss->modSub(op1[0], a, b);
    Mult(results, op1, op2, 1, threadID, net, id, ss);
    ss->modAdd(a, results[0], b);

    smc_batch_free_operator(&op1, 1);
    smc_batch_free_operator(&op2, 1);
    smc_batch_free_operator(&results, 1);
}

void ss_priv_eval(mpz_t *a, mpz_t *b, mpz_t cond, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *results = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    mpz_t *op1 = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    mpz_t *op2 = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    for (int i = 0; i < 4; i++) {
        mpz_init(op1[i]);
        ss->modSub(op1[i], a[i], b[i]);
        mpz_init_set(op2[i], cond);
        mpz_init(results[i]);
    }

    Mult(results, op1, op2, 4, threadID, net, id, ss);
    ss->modAdd(a, results, b, 4);

    smc_batch_free_operator(&op1, 4);
    smc_batch_free_operator(&op2, 4);
    smc_batch_free_operator(&results, 4);
}

void ss_sub(mpz_t *a, mpz_t *b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    // initialization
    mpz_t *b1 = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    for (int i = 0; i < 4; i++)
        mpz_init_set(b1[i], b[i]);
    mpz_t one;
    mpz_init_set_ui(one, 1);

    // computation
    ss->modSub(b1[3], one, b[3]);
    ss_single_fop_arithmetic(result, a, b1, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, "+", threadID, net, id, ss);

    // free the memory
    smc_batch_free_operator(&b1, 4);
    mpz_clear(one);
}
// void ss_sub(int *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
//     mpz_t *atmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     for (int i = 0; i < size; i++)
//         mpz_init_set_si(atmp[i], a[i]);
//     ss_sub(atmp, b, alen, blen, result, resultlen, size, type, threadID, net, id, ss);
//     smc_batch_free_operator(&atmp, size);
// }
// void ss_sub(mpz_t *a, int *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
//     mpz_t *btmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     for (int i = 0; i < size; i++)
//         mpz_init_set_si(btmp[i], b[i]);
//     ss_sub(a, btmp, alen, blen, result, resultlen, size, type, threadID, net, id, ss);
//     smc_batch_free_operator(&btmp, size);
// }
void ss_sub(mpz_t **a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **b1 = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    for (int i = 0; i < size; i++) {
        b1[i] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
        for (int j = 0; j < 4; j++)
            mpz_init_set(b1[i][j], b[i][j]);
    }
    for (int i = 0; i < size; i++)
        ss->modSub(b1[i][3], (long)1, b1[i][3]);

    ss_batch_fop_arithmetic(result, a, b1, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "+", threadID, net, id, ss);
    // free the memory
    smc_batch_free_operator(&b1, size);
}

void ss_sub(float *a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    ss_sub(atmp, b, blen_sig, blen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID, net, id, ss);

    smc_batch_free_operator(&atmp, size);
}

void ss_sub(mpz_t **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    ss_sub(a, btmp, alen_sig, alen_exp, alen_sig, alen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID, net, id, ss);
    smc_batch_free_operator(&btmp, size);
}

// one-dimension private float singular write
void ss_privindex_write(mpz_t index, mpz_t **array, int len_sig, int len_exp, float value, int dim, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *val = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    for (int i = 0; i < 4; i++)
        mpz_init(val[i]);
    convertFloat(value, len_sig, len_exp, &val);
    ss_privindex_write(index, array, len_sig, len_exp, val, dim, out_cond, priv_cond, counter, type, threadID, net, id, ss);
    smc_batch_free_operator(&val, 4);
}

void ss_privindex_write(mpz_t index, mpz_t **array, int len_sig, int len_exp, mpz_t *value, int dim, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *index_tmp = (mpz_t *)malloc(sizeof(mpz_t));
    mpz_t *array_tmp = (mpz_t *)malloc(sizeof(mpz_t) * 4 * dim);

    mpz_init_set(index_tmp[0], index);
    for (int i = 0; i < dim; i++)
        for (int j = 0; j < 4; j++)
            mpz_init_set(array_tmp[4 * i + j], array[i][j]);

    doOperation_PrivIndex_Write(index_tmp, array_tmp, value, dim, 1, out_cond, priv_cond, counter, threadID, 1, net, id, ss);

    for (int i = 0; i < dim; i++)
        for (int j = 0; j < 4; j++)
            mpz_set(array[i][j], array_tmp[4 * i + j]);

    smc_batch_free_operator(&index_tmp, 1);
    smc_batch_free_operator(&array_tmp, 4 * dim);
}

// two-dimension private float singular write
void ss_privindex_write(mpz_t index, mpz_t ***array, int len_sig, int len_exp, float value, int dim1, int dim2, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *val = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    for (int i = 0; i < 4; i++)
        mpz_init(val[i]);
    convertFloat(value, len_sig, len_exp, &val);
    ss_privindex_write(index, array, len_sig, len_exp, val, dim1, dim2, out_cond, priv_cond, counter, type, threadID, net, id, ss);
    smc_batch_free_operator(&val, 4);
}

void ss_privindex_write(mpz_t index, mpz_t ***array, int len_sig, int len_exp, mpz_t *value, int dim1, int dim2, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *index_tmp = (mpz_t *)malloc(sizeof(mpz_t));
    mpz_t *array_tmp = (mpz_t *)malloc(sizeof(mpz_t) * 4 * dim1 * dim2);

    mpz_init_set(index_tmp[0], index);
    for (int i = 0; i < dim1; i++)
        for (int j = 0; j < dim2; j++)
            for (int k = 0; k < 4; k++)
                mpz_init_set(array_tmp[4 * (i * dim2 + j) + k], array[i][j][k]);

    doOperation_PrivIndex_Write(index_tmp, array_tmp, value, dim1 * dim2, 1, out_cond, priv_cond, counter, threadID, 1, net, id, ss);

    for (int i = 0; i < dim1; i++)
        for (int j = 0; j < dim2; j++)
            for (int k = 0; k < 4; k++)
                mpz_set(array[i][j][k], array_tmp[4 * (i * dim2 + j) + k]);

    smc_batch_free_operator(&index_tmp, 1);
    smc_batch_free_operator(&array_tmp, 4 * dim1 * dim2);
}



// two-dimension private integer batch read
void ss_privindex_read(mpz_t *indices, mpz_t **array, mpz_t *results, int dim1, int dim2, int size, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *array_tmp = (mpz_t *)malloc(sizeof(mpz_t) * dim1 * dim2);
    for (int i = 0; i < dim1; i++)
        for (int j = 0; j < dim2; j++)
            mpz_init_set(array_tmp[i * dim2 + j], array[i][j]);
    doOperation_PrivIndex_Read(indices, array_tmp, results, dim1 * dim2, size, threadID, 0, net, id, ss);
    smc_batch_free_operator(&array_tmp, dim1 * dim2);
}

// one-dimension private float batch read
void ss_privindex_read(mpz_t *indices, mpz_t **array, mpz_t **results, int dim, int size, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *array_tmp = (mpz_t *)malloc(sizeof(mpz_t) * 4 * dim);
    mpz_t *result_tmp = (mpz_t *)malloc(sizeof(mpz_t) * 4 * size);

    for (int i = 0; i < dim; i++)
        for (int j = 0; j < 4; j++)
            mpz_init_set(array_tmp[4 * i + j], array[i][j]);
    for (int i = 0; i < 4 * size; i++)
        mpz_init(result_tmp[i]);

    doOperation_PrivIndex_Read(indices, array_tmp, result_tmp, dim, size, threadID, 1, net, id, ss);

    for (int i = 0; i < size; i++)
        for (int j = 0; j < 4; j++)
            mpz_set(results[i][j], result_tmp[4 * i + j]);

    smc_batch_free_operator(&result_tmp, 4 * size);
    smc_batch_free_operator(&array_tmp, 4 * dim);
}

// two-dimension private float batch read
void ss_privindex_read(mpz_t *indices, mpz_t ***array, mpz_t **results, int dim1, int dim2, int size, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *array_tmp = (mpz_t *)malloc(sizeof(mpz_t) * 4 * dim1 * dim2);
    mpz_t *result_tmp = (mpz_t *)malloc(sizeof(mpz_t) * 4 * size);

    for (int i = 0; i < dim1; i++)
        for (int j = 0; j < dim2; j++)
            for (int k = 0; k < 4; k++)
                mpz_init_set(array_tmp[4 * (i * dim2 + j) + k], array[i][j][k]);
    for (int i = 0; i < 4 * size; i++)
        mpz_init(result_tmp[i]);

    doOperation_PrivIndex_Read(indices, array_tmp, result_tmp, dim1 * dim2, size, threadID, 1, net, id, ss);

    for (int i = 0; i < size; i++)
        for (int j = 0; j < 4; j++)
            mpz_set(results[i][j], result_tmp[4 * i + j]);

    smc_batch_free_operator(&result_tmp, 4 * size);
    smc_batch_free_operator(&array_tmp, 4 * dim1 * dim2);
}

// one-dimension private integer batch write
// void ss_privindex_write(mpz_t *indices, mpz_t *array, int len_sig, int len_exp, int *values, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {

//     doOperation_PrivIndex_Write(indices, array, values, dim, size, out_cond, priv_cond, counter, threadID, 0, net, id, ss);

//     // mpz_t *val = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     // for (int i = 0; i < size; i++)
//     //     mpz_init_set_si(val[i], values[i]);
//     // ss_privindex_write(indices, array, len_sig, len_exp, val, dim, size, out_cond, priv_cond, counter, type, threadID);
//     // smc_batch_free_operator(&val, size);
// }

// void ss_privindex_write(mpz_t *indices, mpz_t *array, int len_sig, int len_exp, mpz_t *values, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
//     doOperation_PrivIndex_Write(indices, array, values, dim, size, out_cond, priv_cond, counter, threadID, 0, net, id, ss);
// }

void ss_privindex_write(mpz_t *indices, mpz_t **array, int len_sig, int len_exp, int *values, int dim1, int dim2, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *val = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set_si(val[i], values[i]);
    ss_privindex_write(indices, array, len_sig, len_exp, val, dim1, dim2, size, out_cond, priv_cond, counter, type, threadID, net, id, ss);
    smc_batch_free_operator(&val, size);
}

// two-dimension private integer batch write
void ss_privindex_write(mpz_t *indices, mpz_t **array, int len_sig, int len_exp, mpz_t *values, int dim1, int dim2, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *array_tmp = (mpz_t *)malloc(sizeof(mpz_t) * dim1 * dim2);
    for (int i = 0; i < dim1; i++)
        for (int j = 0; j < dim2; j++)
            mpz_init_set(array_tmp[i * dim2 + j], array[i][j]);

    doOperation_PrivIndex_Write(indices, array_tmp, values, dim1 * dim2, size, out_cond, priv_cond, counter, threadID, 0, net, id, ss);

    for (int i = 0; i < dim1; i++)
        for (int j = 0; j < dim2; j++)
            mpz_set(array[i][j], array_tmp[i * dim2 + j]);

    smc_batch_free_operator(&array_tmp, dim1 * dim2);
}

// one-dimension private float batch write
void ss_privindex_write(mpz_t *indices, mpz_t **array, int len_sig, int len_exp, float *values, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **val = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    for (int i = 0; i < size; i++) {
        val[i] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
        for (int j = 0; j < 4; j++)
            mpz_init(val[i][j]);
        convertFloat(values[i], len_sig, len_exp, &(val[i]));
    }
    ss_privindex_write(indices, array, len_sig, len_exp, val, dim, size, out_cond, priv_cond, counter, type, threadID, net, id, ss);
    smc_batch_free_operator(&val, size);
}
void ss_privindex_write(mpz_t *indices, mpz_t **array, int len_sig, int len_exp, mpz_t **values, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *array_tmp = (mpz_t *)malloc(sizeof(mpz_t) * 4 * dim);
    mpz_t *value_tmp = (mpz_t *)malloc(sizeof(mpz_t) * 4 * size);

    for (int i = 0; i < dim; i++)
        for (int j = 0; j < 4; j++)
            mpz_init_set(array_tmp[i * 4 + j], array[i][j]);

    for (int i = 0; i < size; i++)
        for (int j = 0; j < 4; j++)
            mpz_init_set(value_tmp[i * 4 + j], values[i][j]);

    doOperation_PrivIndex_Write(indices, array_tmp, value_tmp, dim, size, out_cond, priv_cond, counter, threadID, 1, net, id, ss);

    for (int i = 0; i < dim; i++)
        for (int j = 0; j < 4; j++)
            mpz_set(array[i][j], array_tmp[4 * i + j]);

    smc_batch_free_operator(&array_tmp, 4 * dim);
    smc_batch_free_operator(&value_tmp, 4 * size);
}

// two-dimension private float batch write
void ss_privindex_write(mpz_t *indices, mpz_t ***array, int len_sig, int len_exp, float *values, int dim1, int dim2, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **val = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    for (int i = 0; i < size; i++) {
        val[i] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
        for (int j = 0; j < 4; j++)
            mpz_init(val[i][j]);
        convertFloat(values[i], len_sig, len_exp, &(val[i]));
    }
    ss_privindex_write(indices, array, len_sig, len_exp, val, dim1, dim2, size, out_cond, priv_cond, counter, type, threadID, net, id, ss);
    smc_batch_free_operator(&val, size);
}
void ss_privindex_write(mpz_t *indices, mpz_t ***array, int len_sig, int len_exp, mpz_t **values, int dim1, int dim2, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *array_tmp = (mpz_t *)malloc(sizeof(mpz_t) * 4 * dim1 * dim2);
    mpz_t *value_tmp = (mpz_t *)malloc(sizeof(mpz_t) * 4 * size);

    for (int i = 0; i < dim1; i++)
        for (int j = 0; j < dim2; j++)
            for (int k = 0; k < 4; k++)
                mpz_init_set(array_tmp[4 * (i * dim2 + j) + k], array[i][j][k]);

    for (int i = 0; i < size; i++)
        for (int j = 0; j < 4; j++)
            mpz_init_set(value_tmp[4 * i + j], values[i][j]);

    doOperation_PrivIndex_Write(indices, array_tmp, value_tmp, dim1 * dim2, size, out_cond, priv_cond, counter, threadID, 1, net, id, ss);

    for (int i = 0; i < dim1; i++)
        for (int j = 0; j < dim2; j++)
            for (int k = 0; k < 4; k++)
                mpz_set(array[i][j][k], array_tmp[4 * (i * dim2 + j) + k]);

    smc_batch_free_operator(&array_tmp, 4 * dim1 * dim2);
    smc_batch_free_operator(&value_tmp, 4 * size);
}

void ss_int2fl(int value, mpz_t *result, int gamma, int K, int L, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t val;
    mpz_init_set_si(val, value);
    ss_int2fl(val, result, 32, K, L, threadID, net, id, ss);
    mpz_clear(val);
}

void ss_int2fl(mpz_t value, mpz_t *result, int gamma, int K, int L, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **results = (mpz_t **)malloc(sizeof(mpz_t *));
    mpz_t *values = (mpz_t *)malloc(sizeof(mpz_t));
    results[0] = (mpz_t *)malloc(sizeof(mpz_t) * 4);

    for (int i = 0; i < 4; i++)
        mpz_init(results[0][i]);
    mpz_init_set(values[0], value);

    doOperation_Int2FL(values, results, gamma, K, 1, threadID, net, id, ss);

    for (int i = 0; i < 4; i++)
        mpz_set(result[i], results[0][i]);

    smc_batch_free_operator(&values, 1);
    smc_batch_free_operator(&results, 1);
}

void ss_int2int(int value, mpz_t result, int gamma1, int gamma2, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_set_si(result, value);
}

void ss_int2int(mpz_t value, mpz_t result, int gamma1, int gamma2, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_set(result, value);
}

void ss_fl2int(float value, mpz_t result, int K, int L, int gamma, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *val = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    for (int i = 0; i < 4; i++)
        mpz_init(val[i]);
    convertFloat(value, 32, 9, &val);
    ss_fl2int(val, result, 32, 9, gamma, threadID, net, id, ss);
    for (int i = 0; i < 4; i++)
        mpz_clear(val[i]);
    free(val);
}

void ss_fl2int(mpz_t *value, mpz_t result, int K, int L, int gamma, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **values = (mpz_t **)malloc(sizeof(mpz_t *));
    mpz_t *results = (mpz_t *)malloc(sizeof(mpz_t));
    values[0] = (mpz_t *)malloc(sizeof(mpz_t) * 4);

    for (int i = 0; i < 4; i++)
        mpz_init_set(values[0][i], value[i]);
    mpz_init(results[0]);
    doOperation_FL2Int(values, results, K, L, gamma, 1, threadID, net, id, ss);
    mpz_set(result, results[0]);
    smc_batch_free_operator(&values, 1);
    smc_batch_free_operator(&results, 1);
}

void ss_fl2fl(float value, mpz_t *result, int K1, int L1, int K2, int L2, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *val = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    for (int i = 0; i < 4; i++)
        mpz_init(val[i]);
    convertFloat(value, 32, 9, &val);
    ss_fl2fl(val, result, 32, 9, K2, L2, threadID, net, id, ss);
    for (int i = 0; i < 4; i++)
        mpz_clear(val[i]);
    free(val);
}

void ss_fl2fl(mpz_t *value, mpz_t *result, int K1, int L1, int K2, int L2, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    if (K1 >= K2)
        doOperation_Trunc(result, value, K1, K1 - K2, 1, threadID, net, id, ss);
    else {
        mpz_t tmp, two;
        mpz_init_set_ui(two, 2);
        mpz_init(tmp);
        ss->modPow(tmp, two, K2 - K1);
        ss->modMul(result[0], value[0], tmp);
        mpz_clear(tmp);
        mpz_clear(two);
    }
    ss->modAdd(result[1], value[1], K1 - K2);
    mpz_set(result[2], value[2]);
    mpz_set(result[3], value[3]);
}
void ss_batch_handle_priv_cond(mpz_t *result, mpz_t *result_org, mpz_t out_cond, mpz_t *priv_cond, int counter, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; ++i)
        mpz_init(tmp[i]);
    if (out_cond == NULL && counter == -1 && priv_cond == NULL) {
    } else if (out_cond != NULL && counter == -1 && priv_cond == NULL) {
        for (int i = 0; i < size; i++)
            mpz_set(tmp[i], out_cond);
        ss->modSub(result, result, result_org, size);
        Mult(result, result, tmp, size, threadID, net, id, ss);
        ss->modAdd(result, result, result_org, size);
        /*
        ss->modSub(tmp3, 1, tmp, size);
                Mult(tmp1, result, tmp, size,net, id, ss);
                Mult(tmp2, result_org, tmp3, size,net, id, ss);
                ss->modAdd(result, tmp1, tmp2, size);
        */
    } else if (out_cond == NULL && counter != -1 && priv_cond != NULL) {
        for (int i = 0; i < size; i++)
            if (counter != size)
                mpz_set(tmp[i], priv_cond[i / (size / counter)]);
            else
                mpz_set(tmp[i], priv_cond[i]);
        ss->modSub(result, result, result_org, size);
        Mult(result, result, tmp, size, threadID, net, id, ss);
        ss->modAdd(result, result, result_org, size);
        /*
        ss->modSub(tmp3, 1, tmp, size);
                Mult(tmp1, result, tmp, size,net, id, ss);
                Mult(tmp2, result_org, tmp3, size,net, id, ss);
                ss->modAdd(result, tmp1, tmp2, size);
        */
    }
    for (int i = 0; i < size; ++i)
        mpz_clear(tmp[i]);
    free(tmp);
}