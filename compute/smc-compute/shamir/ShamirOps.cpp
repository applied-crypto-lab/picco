/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2023 PICCO Team
   ** Department of Computer Science and Engineering, University at Buffalo (SUNY)

   PICCO is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   PICCO is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with PICCO. If not, see <http://www.gnu.org/licenses/>.
*/

#include "ShamirOps.h"

void ss_init_set(mpz_t x, mpz_t x_val) {
    mpz_init_set(x, x_val);
}

void ss_init(mpz_t x) {
    mpz_init(x);
}
void ss_set(mpz_t x, mpz_t x_val) {
    mpz_set(x, x_val);
}

void ss_clear(mpz_t x) {
    mpz_clear(x);
}

void ss_add_ui(mpz_t rop, mpz_t op1, uint op2) {
    mpz_add_ui(rop, op1, op2);
}

void ss_sub_ui(mpz_t rop, mpz_t op1, uint op2) {
    mpz_sub_ui(rop, op1, op2);
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
    ss_batch_free_operator(&elements, 4);
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
    ss_batch_free_operator(&elements, 4);
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
    ss_batch_free_operator(&results, 1);
    ss_batch_free_operator(&as, 1);
    ss_batch_free_operator(&bs, 1);
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
    ss_batch_free_operator(&results, 1);
    ss_batch_free_operator(&as, 1);
    ss_batch_free_operator(&bs, 1);
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
    ss_batch_free_operator(&results, 1);
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

// void ss_set(mpz_t **a, mpz_t **result, int alen_sig, int alen_exp, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
//     for (int i = 0; i < size; i++)
//         ss_set(a[i], result[i], alen_sig, alen_exp, resultlen_sig, resultlen_exp, type, threadID, net, id, ss);
// }

// void ss_set(mpz_t *a, mpz_t *result, int alen, int resultlen, int size, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
//     for (int i = 0; i < size; i++)
//         ss_set(a[i], result[i], alen, resultlen, type, threadID, net, id, ss);
// }

// this routine should implement in a way that result = a + share[0]
void ss_set(int a, mpz_t result, int alen, int resultlen, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_set_ui(result, 0);
    ss->modAdd(result, result, a);
}

void ss_set(mpz_t *a, mpz_t *result, int alen, int resultlen, int size, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    for (int i = 0; i < size; i++)
        ss_set(a[i], result[i], alen, resultlen, type, threadID, net, id, ss);
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

    ss_batch_free_operator(&op1, 1);
    ss_batch_free_operator(&op2, 1);
    ss_batch_free_operator(&results, 1);
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

    ss_batch_free_operator(&op1, 4);
    ss_batch_free_operator(&op2, 4);
    ss_batch_free_operator(&results, 4);
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
    ss_batch_free_operator(&b1, 4);
    mpz_clear(one);
}
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
    ss_batch_free_operator(&b1, size);
}

void ss_sub(float *a, mpz_t **b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    ss_sub(atmp, b, blen_sig, blen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID, net, id, ss);

    ss_batch_free_operator(&atmp, size);
}

void ss_sub(mpz_t **a, float *b, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t **result, int resultlen_sig, int resultlen_exp, int size, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    ss_sub(a, btmp, alen_sig, alen_exp, alen_sig, alen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID, net, id, ss);
    ss_batch_free_operator(&btmp, size);
}

// one-dimension private float singular write
void ss_privindex_write(mpz_t index, mpz_t **array, int len_sig, int len_exp, float value, int dim, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *val = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    for (int i = 0; i < 4; i++)
        mpz_init(val[i]);
    convertFloat(value, len_sig, len_exp, &val);
    ss_privindex_write(index, array, len_sig, len_exp, val, dim, out_cond, priv_cond, counter, type, threadID, net, id, ss);
    ss_batch_free_operator(&val, 4);
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

    ss_batch_free_operator(&index_tmp, 1);
    ss_batch_free_operator(&array_tmp, 4 * dim);
}

// two-dimension private float singular write
void ss_privindex_write(mpz_t index, mpz_t ***array, int len_sig, int len_exp, float value, int dim1, int dim2, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *val = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    for (int i = 0; i < 4; i++)
        mpz_init(val[i]);
    convertFloat(value, len_sig, len_exp, &val);
    ss_privindex_write(index, array, len_sig, len_exp, val, dim1, dim2, out_cond, priv_cond, counter, type, threadID, net, id, ss);
    ss_batch_free_operator(&val, 4);
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

    ss_batch_free_operator(&index_tmp, 1);
    ss_batch_free_operator(&array_tmp, 4 * dim1 * dim2);
}

// two-dimension private integer batch read
void ss_privindex_read(mpz_t *indices, mpz_t **array, mpz_t *results, int dim1, int dim2, int size, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *array_tmp = (mpz_t *)malloc(sizeof(mpz_t) * dim1 * dim2);
    for (int i = 0; i < dim1; i++)
        for (int j = 0; j < dim2; j++)
            mpz_init_set(array_tmp[i * dim2 + j], array[i][j]);
    doOperation_PrivIndex_Read(indices, array_tmp, results, dim1 * dim2, size, threadID, 0, net, id, ss);
    ss_batch_free_operator(&array_tmp, dim1 * dim2);
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

    ss_batch_free_operator(&result_tmp, 4 * size);
    ss_batch_free_operator(&array_tmp, 4 * dim);
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

    ss_batch_free_operator(&result_tmp, 4 * size);
    ss_batch_free_operator(&array_tmp, 4 * dim1 * dim2);
}

// one-dimension private integer batch write
// void ss_privindex_write(mpz_t *indices, mpz_t *array, int len_sig, int len_exp, int *values, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {

//     doOperation_PrivIndex_Write(indices, array, values, dim, size, out_cond, priv_cond, counter, threadID, 0, net, id, ss);

//     // mpz_t *val = (mpz_t *)malloc(sizeof(mpz_t) * size);
//     // for (int i = 0; i < size; i++)
//     //     mpz_init_set_si(val[i], values[i]);
//     // ss_privindex_write(indices, array, len_sig, len_exp, val, dim, size, out_cond, priv_cond, counter, type, threadID);
//     // ss_batch_free_operator(&val, size);
// }

// void ss_privindex_write(mpz_t *indices, mpz_t *array, int len_sig, int len_exp, mpz_t *values, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
//     doOperation_PrivIndex_Write(indices, array, values, dim, size, out_cond, priv_cond, counter, threadID, 0, net, id, ss);
// }

void ss_privindex_write(mpz_t *indices, mpz_t **array, int len_sig, int len_exp, int *values, int dim1, int dim2, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *val = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set_si(val[i], values[i]);
    ss_privindex_write(indices, array, len_sig, len_exp, val, dim1, dim2, size, out_cond, priv_cond, counter, type, threadID, net, id, ss);
    ss_batch_free_operator(&val, size);
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

    ss_batch_free_operator(&array_tmp, dim1 * dim2);
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
    ss_batch_free_operator(&val, size);
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

    ss_batch_free_operator(&array_tmp, 4 * dim);
    ss_batch_free_operator(&value_tmp, 4 * size);
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
    ss_batch_free_operator(&val, size);
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

    ss_batch_free_operator(&array_tmp, 4 * dim1 * dim2);
    ss_batch_free_operator(&value_tmp, 4 * size);
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

    ss_batch_free_operator(&values, 1);
    ss_batch_free_operator(&results, 1);
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
    ss_batch_free_operator(&values, 1);
    ss_batch_free_operator(&results, 1);
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

void ss_convert_operator(mpz_t **result, mpz_t **op, int *index_array, int dim, int size, int flag) {
    *result = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init((*result)[i]);
    int dim1, dim2;
    for (int i = 0; i < size; i++) {
        if (flag == 1) {
            if (dim != 0) {
                dim1 = index_array[3 * i] / dim;
                dim2 = index_array[3 * i] % dim;
                mpz_set((*result)[i], op[dim1][dim2]);
            } else
                mpz_set((*result)[i], op[0][index_array[3 * i]]);
        } else if (flag == 2) {
            if (dim != 0 && dim != -1) {
                dim1 = index_array[3 * i + 1] / dim;
                dim2 = index_array[3 * i + 1] % dim;
                mpz_set((*result)[i], op[dim1][dim2]);
            } else if (dim == 0)
                mpz_set((*result)[i], op[0][index_array[3 * i + 1]]);
        } else {
            if (dim != 0) {
                dim1 = index_array[3 * i + 2] / dim;
                dim2 = index_array[3 * i + 2] % dim;
                mpz_set((*result)[i], op[dim1][dim2]);
            }
        }
    }
}

// convert op to corresponding one-dimensional array result
void ss_convert_operator(mpz_t **result, mpz_t *op, int *index_array, int dim, int size, int flag) {
    mpz_t **ops = (mpz_t **)malloc(sizeof(mpz_t *));
    *ops = op;
    ss_convert_operator(result, ops, index_array, dim, size, flag);
    free(ops);
}

void ss_convert_operator(mpz_t ***result, mpz_t **op, int *index_array, int dim, int size, int flag) {
    mpz_t ***ops = NULL;
    if (op != NULL) {
        ops = (mpz_t ***)malloc(sizeof(mpz_t **));
        *ops = op;
    }
    ss_convert_operator(result, ops, index_array, dim, size, flag);
    if (op != NULL)
        free(ops);
}

void ss_convert_operator(mpz_t ***result, mpz_t ***op, int *index_array, int dim, int size, int flag) {
    *result = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    int dim1, dim2;
    for (int i = 0; i < size; i++) {
        (*result)[i] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
        for (int j = 0; j < 4; j++)
            mpz_init((*result)[i][j]);
        for (int j = 0; j < 4; j++) {
            if (flag == 1) {
                if (dim != 0) {
                    dim1 = index_array[3 * i] / dim;
                    dim2 = index_array[3 * i] % dim;
                    mpz_set((*result)[i][j], op[dim1][dim2][j]);
                } else
                    mpz_set((*result)[i][j], op[0][i][j]);
            } else if (flag == 2) {
                if (op != NULL && dim != -1) {
                    if (dim != 0) {
                        dim1 = index_array[3 * i + 1] / dim;
                        dim2 = index_array[3 * i + 1] % dim;
                        mpz_set((*result)[i][j], op[dim1][dim2][j]);
                    } else
                        mpz_set((*result)[i][j], op[0][i][j]);
                }
            } else {
                if (dim != 0) {
                    dim1 = index_array[3 * i + 2] / dim;
                    dim2 = index_array[3 * i + 2] % dim;
                    mpz_set((*result)[i][j], op[dim1][dim2][j]);
                }
            }
        }
    }
}
void ss_shr(mpz_t *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    if (blen == -1) {
        // doOperation_Trunc(result, a, alen, b[0], size, threadID, net, id, ss);

        int *b_tmp = (int *)malloc(sizeof(int) * size);
        for (int i = 0; i < size; i++)
            b_tmp[i] = mpz_get_si(b[i]);
        doOperation_Trunc(result, a, alen, b_tmp, size, threadID, net, id, ss);
        // ss_shr(a, b_tmp, alen, blen, result, resultlen, size, type, threadID);
        free(b_tmp);
    } else
        doOperation_TruncS(result, a, alen, b, size, threadID, net, id, ss);
}

void ss_shl(mpz_t *a, mpz_t *b, int alen, int blen, mpz_t *result, int resultlen, int size, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    if (blen == -1) {
        // b is PUBLIC, but stored in type mpz_t
        ss->modPow2(result, b, size);
        ss->modMul(result, a, result, size);
    } else {
        doOperation_Pow2(result, b, blen, size, threadID, net, id, ss);
        Mult(result, result, a, size, threadID, net, id, ss);
    }
}

void ss_batch_BOP_int(mpz_t *result, mpz_t *a, mpz_t *b, int resultlen, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *result_org = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        ss_init_set(result_org[i], result[i]);
    if (op == "*") {
        Mult(result, a, b, size, threadID, net, id, ss);
        // ss_mult(a, b, alen, blen, result, resultlen, size, type, threadID);
    } else if (op == "-") {
        ss->modSub(result, a, b, size);
        // ss_sub(a, b, alen, blen, result, resultlen, size, type, threadID);
    } else if (op == "+") {
        ss->modAdd(result, a, b, size);
        // ss_add(a, b, alen, blen, result, resultlen, size, type, threadID);
    } else if (op == "==") {
        doOperation_EQZ(result, a, b, alen, blen, resultlen, size, threadID, net, id, ss);
        // ss_eqeq(a, b, alen, blen, result, resultlen, size, type, threadID);
    } else if (op == "!=") {
        doOperation_EQZ(result, a, b, alen, blen, resultlen, size, threadID, net, id, ss);
        ss->modSub(result, 1, result, size);
        // ss_neq(a, b, alen, blen, result, resultlen, size, type, threadID);
    } else if (op == ">") {
        doOperation_LT(result, b, a, blen, alen, resultlen, size, threadID, net, id, ss);
        // ss_gt(a, b, alen, blen, result, resultlen, size, type, threadID);
    } else if (op == ">=") {
        doOperation_LT(result, a, b, alen, blen, resultlen, size, threadID, net, id, ss);
        ss->modSub(result, 1, result, size);
        // ss_geq(a, b, alen, blen, result, resultlen, size, type, threadID);
    } else if (op == "<") {
        doOperation_LT(result, a, b, alen, blen, resultlen, size, threadID, net, id, ss);
        // ss_lt(a, b, alen, blen, result, resultlen, size, type, threadID);
    } else if (op == "<=") {
        doOperation_LT(result, b, a, blen, alen, resultlen, size, threadID, net, id, ss);
        ss->modSub(result, 1, result, size);
    } else if (op == "/") {
        doOperation_IntDiv(result, a, b, resultlen, size, threadID, net, id, ss);
        // ss_div(a, b, alen, blen, result, resultlen, size, type, threadID);
    } else if (op == "/P") {
        doOperation_IntDiv_Pub(result, a, b, resultlen, size, threadID, net, id, ss);
    } else if (op == "=") {
        ss_set(a, result, alen, resultlen, size, type, threadID, net, id, ss);
    } else if (op == ">>") {
        ss_shr(a, b, alen, blen, result, resultlen, size, type, threadID, net, id, ss);
    } else if (op == "<<") {
        ss_shl(a, b, alen, blen, result, resultlen, size, type, threadID, net, id, ss);
    } else {
        std::cout << "Unrecognized op: " << op << "\n";
    }

    ss_batch_handle_priv_cond(result, result_org, out_cond, priv_cond, counter, size, threadID, net, id, ss);
    ss_batch_free_operator(&result_org, size);
}

void ss_batch_BOP_float_arithmetic(mpz_t **result, mpz_t **a, mpz_t **b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **result_org = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    for (int i = 0; i < size; i++) {
        result_org[i] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
        for (int j = 0; j < 4; j++)
            mpz_init_set(result_org[i][j], result[i][j]);
    }

    if (op == "*") {
        // smc_mult(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
        ss_batch_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "*", threadID, net, id, ss);
    } else if (op == "-") {
        // smc_sub(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
        ss_batch_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "-", threadID, net, id, ss);

    } else if (op == "+") {
        // smc_add(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
        ss_batch_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "+", threadID, net, id, ss);

    } else if (op == "/") {
        ss_batch_fop_arithmetic(result, a, b, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, size, "/", threadID, net, id, ss);
        // smc_div(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result, resultlen_sig, resultlen_exp, size, type, threadID);
    } else if (op == "/P") {
        doOperation_FLDiv_Pub(a, b, result, alen_sig, size, threadID, net, id, ss);
        ss_process_results(result, resultlen_sig, resultlen_exp, alen_sig, alen_exp, size, threadID, net, id, ss);
    } else if (op == "=") {
        ss_set(a, result, alen_sig, alen_exp, resultlen_sig, resultlen_exp, size, type, threadID, net, id, ss);
    }

    mpz_t *result_unify = (mpz_t *)malloc(sizeof(mpz_t) * 4 * size);
    mpz_t *result_org_unify = (mpz_t *)malloc(sizeof(mpz_t) * 4 * size);

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 4; j++) {
            mpz_init_set(result_unify[4 * i + j], result[i][j]);
            mpz_init_set(result_org_unify[4 * i + j], result_org[i][j]);
        }
    }

    ss_batch_handle_priv_cond(result_unify, result_org_unify, out_cond, priv_cond, counter, 4 * size, threadID, net, id, ss);

    for (int i = 0; i < size; i++)
        for (int j = 0; j < 4; j++)
            mpz_set(result[i][j], result_unify[4 * i + j]);

    ss_batch_free_operator(&result_unify, 4 * size);
    ss_batch_free_operator(&result_org_unify, 4 * size);
}

void ss_batch_BOP_float_comparison(mpz_t *result, mpz_t **a, mpz_t **b, int resultlen_sig, int resultlen_exp, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {

    mpz_t *result_org = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *result_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);

    for (int i = 0; i < size; i++) {
        mpz_init_set(result_org[i], result[index_array[3 * i + 2]]);
        mpz_init(result_tmp[i]);
    }

    if (op == "==") {
        ss_batch_fop_comparison(result, a, b, resultlen_sig, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "==", threadID, net, id, ss);
        // smc_eqeq(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result_tmp, resultlen_sig, size, type, threadID);
    } else if (op == "!=") {
        ss_batch_fop_comparison(result, a, b, resultlen_sig, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "==", threadID, net, id, ss);
        ss->modSub(result, 1, result, size);
        // smc_neq(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result_tmp, resultlen_sig, size, type, threadID);
    } else if (op == ">") {
        ss_batch_fop_comparison(result, b, a, resultlen_sig, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID, net, id, ss);
        // smc_gt(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result_tmp, resultlen_sig, size, type, threadID);
    } else if (op == ">=") {
        ss_batch_fop_comparison(result, a, b, resultlen_sig, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "<0", threadID, net, id, ss);
        ss->modSub(result, 1, result, size);
        // smc_geq(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result_tmp, resultlen_sig, size, type, threadID);
    } else if (op == "<") {
        ss_batch_fop_comparison(result, a, b, resultlen_sig, -1, alen_sig, alen_exp, blen_sig, blen_exp, size, "<0", threadID, net, id, ss);
        // smc_lt(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result_tmp, resultlen_sig, size, type, threadID);
    } else if (op == "<=") {
        ss_batch_fop_comparison(result, b, a, resultlen_sig, -1, blen_sig, blen_exp, alen_sig, alen_exp, size, "<0", threadID, net, id, ss);
        ss->modSub(result, 1, result, size);
        // smc_leq(a, b, alen_sig, alen_exp, blen_sig, blen_exp, result_tmp, resultlen_sig, size, type, threadID);
    }

    ss_batch_handle_priv_cond(result_tmp, result_org, out_cond, priv_cond, counter, size, threadID, net, id, ss);

    for (int i = 0; i < size; ++i)
        mpz_set(result[index_array[3 * i + 2]], result_tmp[i]);

    ss_batch_free_operator(&result_org, size);
    ss_batch_free_operator(&result_tmp, size);
}

/************************************ INTEGER BATCH ****************************************/
void ss_batch(mpz_t *a, mpz_t *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *a_tmp, *b_tmp, *result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    for (int i = 0; i < size; ++i)
        mpz_set(result[index_array[3 * i + 2]], result_tmp[i]);

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// used to compute 1-priv_cond in a batch stmt
void ss_batch(int a, mpz_t *b, mpz_t *result, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *out_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);

    for (int i = 0; i < size; i++) {
        mpz_init_set_ui(a_tmp[i], a);
        mpz_init(out_tmp[i]);
    }

    ss->modSub(result, a_tmp, b, size);

    if (out_cond != NULL) {
        for (int i = 0; i < size; i++)
            mpz_set(out_tmp[i], out_cond);
        Mult(result, result, out_tmp, size, threadID, net, id, ss);
    }

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&out_tmp, size);
}

void ss_batch(mpz_t *a, mpz_t *b, mpz_t *result, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    if (counter == size)
        ss->modSub(result, a, b, size);
    else {
        mpz_t *tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int i = 0; i < size; i++)
            mpz_init_set(tmp[i], a[i / (size / counter)]);
        ss->modSub(result, tmp, b, size);
        ss_batch_free_operator(&tmp, size);
    }
}

// first param: int array
// second param: int array
// third param: one-dim private int array
void ss_batch(int *a, int *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {

    mpz_t *a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++) {
        mpz_init_set_si(a_tmp[i], a[i]);
        mpz_init_set_si(b_tmp[i], b[i]);
    }

    ss_batch(a_tmp, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
}

// first param: int array
// second param: int array
// third param: two-dim private int array
void ss_batch(int *a, int *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {

    mpz_t *a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *b_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);

    for (int i = 0; i < size; i++) {
        mpz_init_set_si(a_tmp[i], a[i]);
        mpz_init_set_si(b_tmp[i], b[i]);
    }

    ss_batch(a_tmp, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
}

// first param: int array
// second param: one-dim private int array
// third param: one-dim private int array
void ss_batch(int *a, mpz_t *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {

    mpz_t *a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set_si(a_tmp[i], a[i]);
    ss_batch(a_tmp, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&a_tmp, size);
}

// first param: int array
// second param: one-dim private int array
// third param: two-dim private int array
void ss_batch(int *a, mpz_t *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {

    mpz_t *a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set_si(a_tmp[i], a[i]);
    ss_batch(a_tmp, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&a_tmp, size);
}

// first param: one-dim private int array
// second param: int array
// third param: one-dim private int array
void ss_batch(mpz_t *a, int *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {

    mpz_t *b_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set_si(b_tmp[i], b[i]);
    op = (op == "/") ? "/P" : op;
    ss_batch(a, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&b_tmp, size);
}

// first param: one-dim private int array
// second param: int array
// third param: two-dim private int array
void ss_batch(mpz_t *a, int *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *b_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set_si(b_tmp[i], b[i]);
    op = (op == "/") ? "/P" : op;
    ss_batch(a, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&b_tmp, size);
}

// first param: integer array
// second param: two-dim private int
// assignment param: one-dim private int
void ss_batch(int *a, mpz_t **b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set_si(a_tmp[i], a[i]);
    ss_batch(a_tmp, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&a_tmp, size);
}

// first param: integer array
// second param: two-dim private int
// assignment param: two-dim private int
void ss_batch(int *a, mpz_t **b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set_si(a_tmp[i], a[i]);
    ss_batch(a_tmp, b, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&a_tmp, size);
}

// first param: two-dim private int
// second param: integer array
// assignment param: one-dim private int
void ss_batch(mpz_t **a, int *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *b_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set_si(b_tmp[i], b[i]);
    op = (op == "/") ? "/P" : op;
    ss_batch(a, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&b_tmp, size);
}

// first param: two-dim private int
// second param: integer array
// assignment param: two-dim private int
void ss_batch(mpz_t **a, int *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {

    mpz_t *b_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set_si(b_tmp[i], b[i]);
    op = (op == "/") ? "/P" : op;
    ss_batch(a, b_tmp, result, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    // free the memory
    ss_batch_free_operator(&b_tmp, size);
}

// first param: one-dim private int
// second param: two-dim private int
// assignment param: two-dim private int
void ss_batch(mpz_t *a, mpz_t **b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    int dim1, dim2;
    mpz_t *a_tmp, *b_tmp, *result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    if (resultdim != 0) {
        for (int i = 0; i < size; ++i) {
            dim1 = index_array[3 * i + 2] / resultdim;
            dim2 = index_array[3 * i + 2] % resultdim;
            mpz_set(result[dim1][dim2], result_tmp[i]);
        }
    }

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// first param: two-dim private int
// second param: one-dim private int
// assignment param: two-dim private int
void ss_batch(mpz_t **a, mpz_t *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    int dim1, dim2;
    mpz_t *a_tmp, *b_tmp, *result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    if (resultdim != 0) {
        for (int i = 0; i < size; ++i) {
            dim1 = index_array[3 * i + 2] / resultdim;
            dim2 = index_array[3 * i + 2] % resultdim;
            mpz_set(result[dim1][dim2], result_tmp[i]);
        }
    }

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// first param: two-dim private int
// second param: two-dim private int
// assignment param: one-dim private int
void ss_batch(mpz_t **a, mpz_t **b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    if (op == "@") {
        mpz_t **result_tmp = (mpz_t **)malloc(sizeof(mpz_t *));
        result_tmp[0] = (mpz_t *)malloc(sizeof(mpz_t) * resultdim);
        for (int i = 0; i < resultdim; i++)
            mpz_init_set(result_tmp[0][i], result[i]);
        ss_batch(a, b, result_tmp, alen, blen, resultlen, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
        for (int i = 0; i < resultdim; i++)
            mpz_set(result[i], result_tmp[0][i]);
        ss_batch_free_operator(&(result_tmp[0]), resultdim);
        free(result_tmp);
        return;
    }
    mpz_t *a_tmp, *b_tmp, *result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    for (int i = 0; i < size; i++)
        mpz_set(result[index_array[3 * i + 2]], result_tmp[i]);

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// first param: one-dim private int
// second param: one-dim private int
// assignment param: two-dim private int
void ss_batch(mpz_t *a, mpz_t *b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    int dim1, dim2;
    mpz_t *a_tmp, *b_tmp, *result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    if (resultdim != 0) {
        for (int i = 0; i < size; ++i) {
            dim1 = index_array[3 * i + 2] / resultdim;
            dim2 = index_array[3 * i + 2] % resultdim;
            mpz_set(result[dim1][dim2], result_tmp[i]);
        }
    }

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}
// first param: one-dim private int
// second param: two-dim private int
// assignment param: one-dim private int
void ss_batch(mpz_t *a, mpz_t **b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {

    mpz_t *a_tmp, *b_tmp, *result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    for (int i = 0; i < size; i++)
        mpz_set(result[index_array[3 * i + 2]], result_tmp[i]);

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// first param: two-dim private int
// second param: one-dim private int
// assignment param: one-dim private int
void ss_batch(mpz_t **a, mpz_t *b, mpz_t *result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {

    mpz_t *a_tmp, *b_tmp, *result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    for (int i = 0; i < size; i++)
        mpz_set(result[index_array[3 * i + 2]], result_tmp[i]);

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

void ss_batch_dot(mpz_t **a, mpz_t **b, int size, int array_size, int *index_array, mpz_t *result, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    int a_dim = 0, b_dim = 0;
    mpz_t **a_tmp = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    mpz_t **b_tmp = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    for (int i = 0; i < size; i++) {
        a_tmp[i] = (mpz_t *)malloc(sizeof(mpz_t) * array_size);
        b_tmp[i] = (mpz_t *)malloc(sizeof(mpz_t) * array_size);
        a_dim = index_array[3 * i];
        b_dim = index_array[3 * i + 1];
        for (int j = 0; j < array_size; j++) {
            mpz_init_set(a_tmp[i][j], a[a_dim][j]);
            mpz_init_set(b_tmp[i][j], b[b_dim][j]);
        }
    }

    doOperation_DotProduct(a_tmp, b_tmp, result, size, array_size, threadID, net, id, ss);

    // free the memory
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < array_size; j++) {
            mpz_clear(a_tmp[i][j]);
            mpz_clear(b_tmp[i][j]);
        }
        free(a_tmp[i]);
        free(b_tmp[i]);
    }
    free(a_tmp);
    free(b_tmp);
}

// first param: two-dim private int
// second param: two-dim private int
// assignment param: two-dim private int

void ss_batch(mpz_t **a, mpz_t **b, mpz_t **result, int alen, int blen, int resultlen, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    int dim1, dim2;
    mpz_t *a_tmp, *b_tmp, *result_tmp, *result_org;
    if (op == "@") {
        result_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
        result_org = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int i = 0; i < size; i++) {
            mpz_init(result_tmp[i]);
            mpz_init(result_org[i]);
            if (resultdim != 0) {
                dim1 = index_array[3 * i + 2] / resultdim;
                dim2 = index_array[3 * i + 2] % resultdim;
                mpz_set(result_org[i], result[dim1][dim2]);
            }
        }
        ss_batch_dot(a, b, size, adim, index_array, result_tmp, threadID, net, id, ss);
        ss_batch_handle_priv_cond(result_tmp, result_org, out_cond, priv_cond, counter, size, threadID, net, id, ss);
    } else {
        ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
        ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
        ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);
        ss_batch_BOP_int(result_tmp, a_tmp, b_tmp, resultlen, alen, blen, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);
    }

    if (resultdim != 0) {
        for (int i = 0; i < size; ++i) {
            dim1 = index_array[3 * i + 2] / resultdim;
            dim2 = index_array[3 * i + 2] % resultdim;
            mpz_set(result[dim1][dim2], result_tmp[i]);
        }
    }

    if (op != "@") {
        ss_batch_free_operator(&a_tmp, size);
        ss_batch_free_operator(&b_tmp, size);
        ss_batch_free_operator(&result_tmp, size);
    } else {
        ss_batch_free_operator(&result_org, size);
        ss_batch_free_operator(&result_tmp, size);
    }
}

/*********************************************** FLOAT BATCH ****************************************************/
// public + private one-dimension float - arithmetic
void ss_batch(float *a, mpz_t **b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    ss_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&atmp, size);
}

void ss_batch(mpz_t **a, float *b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    op = (op == "/") ? "/P" : op;
    ss_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&btmp, size);
}

void ss_batch(float *a, mpz_t **b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    ss_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&atmp, size);
}

void ss_batch(mpz_t **a, float *b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    op = (op == "/") ? "/P" : op;
    ss_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&btmp, size);
}
// public to private assignments
void ss_batch(float *a, float *b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    // mainly used for assignments, which means b will not be necessary
    mpz_t **atmp;
    ss_batch_convert_to_private_float(a, &atmp, resultlen_sig, resultlen_exp, size, ss);
    ss_batch(atmp, atmp, result, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&atmp, size);
}

void ss_batch(int *a, int *b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    float *a1 = (float *)malloc(sizeof(float) * size);
    for (int i = 0; i < size; i++)
        a1[i] = a[i];
    mpz_t **atmp;
    ss_batch_convert_to_private_float(a1, &atmp, resultlen_sig, resultlen_exp, size, ss);
    ss_batch(atmp, atmp, result, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&atmp, size);
    free(a1);
}

void ss_batch(float *a, float *b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **atmp;
    ss_batch_convert_to_private_float(a, &atmp, resultlen_sig, resultlen_exp, size, ss);
    ss_batch(atmp, atmp, result, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&atmp, size);
}

void ss_batch(int *a, int *b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    float *a1 = (float *)malloc(sizeof(float) * size);
    for (int i = 0; i < size; i++)
        a1[i] = a[i];
    mpz_t **atmp;
    ss_batch_convert_to_private_float(a1, &atmp, resultlen_sig, resultlen_exp, size, ss);
    ss_batch(atmp, atmp, result, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&atmp, size);
    free(a1);
}

// public + private two-dimension float - arithmetic
void ss_batch(float *a, mpz_t ***b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    ss_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&atmp, size);
}

void ss_batch(mpz_t ***a, float *b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    op = (op == "/") ? "/P" : op;
    ss_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&btmp, size);
}

void ss_batch(float *a, mpz_t ***b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    ss_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&atmp, size);
}

void ss_batch(mpz_t ***a, float *b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    op = (op == "/") ? "/P" : op;
    ss_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&btmp, size);
}

// public + private one-dimension float - comparison
void ss_batch(float *a, mpz_t **b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    ss_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&atmp, size);
}

void ss_batch(mpz_t **a, float *b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    ss_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&btmp, size);
}

// public + private two-dimension float - comparison
void ss_batch(float *a, mpz_t ***b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **atmp;
    ss_batch_convert_to_private_float(a, &atmp, blen_sig, blen_exp, size, ss);
    ss_batch(atmp, b, result, blen_sig, blen_exp, blen_sig, blen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&atmp, size);
}

void ss_batch(mpz_t ***a, float *b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **btmp;
    ss_batch_convert_to_private_float(b, &btmp, alen_sig, alen_exp, size, ss);
    ss_batch(a, btmp, result, alen_sig, alen_exp, alen_sig, alen_exp, resultlen_sig, resultlen_exp, adim, bdim, resultdim, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&btmp, size);
}

void ss_batch(mpz_t **a, mpz_t **b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {

    mpz_t **a_tmp, **b_tmp, **result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    for (int i = 0; i < size; ++i)
        for (int j = 0; j < 4; j++)
            mpz_set(result[index_array[3 * i + 2]][j], result_tmp[i][j]);

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// first parameter: one-dim float
// second parameter: two-dim float
// assignment parameter: two-dim float

void ss_batch(mpz_t **a, mpz_t ***b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **a_tmp, **b_tmp, **result_tmp;
    int dim1, dim2;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < 4; j++) {
            dim1 = index_array[3 * i + 2] / resultdim;
            dim2 = index_array[3 * i + 2] % resultdim;
            mpz_set(result[dim1][dim2][j], result_tmp[i][j]);
        }
    }

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// first parameter: two-dim float
// second parameter: one-dim float
// assignment parameter: two-dim float

void ss_batch(mpz_t ***a, mpz_t **b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **a_tmp, **b_tmp, **result_tmp;
    int dim1, dim2;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < 4; j++) {
            dim1 = index_array[3 * i + 2] / resultdim;
            dim2 = index_array[3 * i + 2] % resultdim;
            mpz_set(result[dim1][dim2][j], result_tmp[i][j]);
        }
    }

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// first parameter: two-dim float
// second parameter: two-dim float
// assignment parameter: one-dim float

void ss_batch(mpz_t ***a, mpz_t ***b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **a_tmp, **b_tmp, **result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    for (int i = 0; i < size; ++i)
        for (int j = 0; j < 4; j++)
            mpz_set(result[index_array[3 * i + 2]][j], result_tmp[i][j]);

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// first parameter: two-dim float
// second parameter: one-dim float
// assignment parameter: one-dim float

void ss_batch(mpz_t ***a, mpz_t **b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **a_tmp, **b_tmp, **result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    for (int i = 0; i < size; ++i)
        for (int j = 0; j < 4; j++)
            mpz_set(result[index_array[3 * i + 2]][j], result_tmp[i][j]);

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// first parameter: one-dim float
// second parameter: two-dim float
// assignment parameter: one-dim float

void ss_batch(mpz_t **a, mpz_t ***b, mpz_t **result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {

    mpz_t **a_tmp, **b_tmp, **result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    for (int i = 0; i < size; ++i)
        for (int j = 0; j < 4; j++)
            mpz_set(result[index_array[3 * i + 2]][j], result_tmp[i][j]);

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// first parameter: one-dim float
// second parameter: one-dim float
// assignment parameter: two-dim float

void ss_batch(mpz_t **a, mpz_t **b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {

    mpz_t **a_tmp, **b_tmp, **result_tmp;
    int dim1, dim2;

    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < 4; j++) {
            dim1 = index_array[3 * i + 2] / resultdim;
            dim2 = index_array[3 * i + 2] % resultdim;
            mpz_set(result[dim1][dim2][j], result_tmp[i][j]);
        }
    }

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// two dimension float general computation
void ss_batch(mpz_t ***a, mpz_t ***b, mpz_t ***result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {

    int dim1, dim2;
    mpz_t **a_tmp, **b_tmp, **result_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    ss_batch_BOP_float_arithmetic(result_tmp, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, size, op, type, threadID, net, id, ss);

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < 4; j++) {
            dim1 = index_array[3 * i + 2] / resultdim;
            dim2 = index_array[3 * i + 2] % resultdim;
            mpz_set(result[dim1][dim2][j], result_tmp[i][j]);
        }
    }

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// two-dimensional float comparison
void ss_batch(mpz_t ***a, mpz_t ***b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **a_tmp, **b_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);

    ss_batch_BOP_float_comparison(result, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
}

// one-dimensional float and two-dimensional float comparison
void ss_batch(mpz_t **a, mpz_t ***b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **a_tmp, **b_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);

    ss_batch_BOP_float_comparison(result, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
}

// two-dimensional float and one-dimensional float comparison
void ss_batch(mpz_t ***a, mpz_t **b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **a_tmp, **b_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);

    ss_batch_BOP_float_comparison(result, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
}

// one-dimensional float and one-dimensional float comparison
void ss_batch(mpz_t **a, mpz_t **b, mpz_t *result, int alen_sig, int alen_exp, int blen_sig, int blen_exp, int resultlen_sig, int resultlen_exp, int adim, int bdim, int resultdim, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, std::string op, std::string type, int threadID, NodeNetwork net, int id, SecretShare *ss) {

    mpz_t **a_tmp, **b_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&b_tmp, b, index_array, bdim, size, 2);

    ss_batch_BOP_float_comparison(result, a_tmp, b_tmp, resultlen_sig, resultlen_exp, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, op, type, threadID, net, id, ss);

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&b_tmp, size);
}

/* conversion from public integer to private float*/
void ss_batch_int2fl(int *a, mpz_t **result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set_si(a_tmp[i], a[i]);
    ss_batch_int2fl(a_tmp, result, size, resultdim, 32, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
    ss_batch_free_operator(&a_tmp, size);
}

/* conversion from private integer to private float */
// one-dimensional int array to one-dimensional float array
void ss_batch_int2fl(mpz_t *a, mpz_t **result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **result_tmp = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    mpz_t *a_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    for (int i = 0; i < size; i++) {
        result_tmp[i] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
        for (int j = 0; j < 4; j++)
            mpz_init(result_tmp[i][j]);
    }
    doOperation_Int2FL(a_tmp, result_tmp, alen, resultlen_sig, size, threadID, net, id, ss);
    /* consider the private condition */
    mpz_t *result_new = (mpz_t *)malloc(sizeof(mpz_t) * 4 * size);
    mpz_t *result_org = (mpz_t *)malloc(sizeof(mpz_t) * 4 * size);

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 4; j++) {
            mpz_init_set(result_new[4 * i + j], result_tmp[i][j]);
            mpz_init_set(result_org[4 * i + j], result[index_array[3 * i + 2]][j]);
        }
    }

    ss_batch_handle_priv_cond(result_new, result_org, out_cond, priv_cond, counter, 4 * size, threadID, net, id, ss);
    for (int i = 0; i < size; i++)
        for (int j = 0; j < 4; j++)
            mpz_set(result[index_array[3 * i + 2]][j], result_new[4 * i + j]);

    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&result_new, 4 * size);
    ss_batch_free_operator(&result_org, 4 * size);
    ss_batch_free_operator(&result_tmp, size);
}

void ss_batch_int2fl(int *a, mpz_t ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set_si(a_tmp[i], a[i]);
    ss_batch_int2fl(a_tmp, result, size, resultdim, 32, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
    ss_batch_free_operator(&a_tmp, size);
}

// one-dimensional int array to two-dimensional float array
void ss_batch_int2fl(mpz_t *a, mpz_t ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {

    mpz_t **result_tmp;
    int *result_index_array = (int *)malloc(sizeof(int) * size);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    for (int i = 0; i < size; i++) {
        result_index_array[i] = index_array[3 * i + 2];
        index_array[3 * i + 2] = i;
    }
    /* call one-dimensional int array + one-dimensional float array */
    ss_batch_int2fl(a, result_tmp, adim, size, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
    /* set the results back */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 4; j++) {
            int dim1 = result_index_array[i] / resultdim;
            int dim2 = result_index_array[i] % resultdim;
            mpz_set(result[dim1][dim2][j], result_tmp[i][j]);
        }
    }
    free(result_index_array);
    ss_batch_free_operator(&result_tmp, size);
}

// two-dimensional int array to one-dimensional float array
void ss_batch_int2fl(mpz_t **a, mpz_t **result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *a_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    for (int i = 0; i < size; i++)
        index_array[3 * i] = i;
    ss_batch_int2fl(a_tmp, result, size, resultdim, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
    ss_batch_free_operator(&a_tmp, size);
}

// two-dimensional int array to two-dimensional float array
void ss_batch_int2fl(mpz_t **a, mpz_t ***result, int adim, int resultdim, int alen, int resultlen_sig, int resultlen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **result_tmp;
    mpz_t *a_tmp;
    int *result_index_array = (int *)malloc(sizeof(int) * size);
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    for (int i = 0; i < size; i++) {
        result_index_array[i] = index_array[3 * i + 2];
        index_array[3 * i] = i;
        index_array[3 * i + 2] = i;
    }

    /* call one-dimensional int array + one-dimensional float array	*/
    ss_batch_int2fl(a_tmp, result_tmp, size, size, alen, resultlen_sig, resultlen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    /* set the results back */
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 4; j++) {
            int dim1 = result_index_array[i] / resultdim;
            int dim2 = result_index_array[i] % resultdim;
            mpz_set(result[dim1][dim2][j], result_tmp[i][j]);
        }
    }
    free(result_index_array);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

/*FL2INT*/
void ss_batch_fl2int(float *a, mpz_t *result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **a_tmp = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    for (int i = 0; i < size; i++) {
        a_tmp[i] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
        for (int j = 0; j < 4; j++)
            mpz_init(a_tmp[i][j]);
        convertFloat(a[i], 32, 9, &a_tmp[i]);
    }
    ss_batch_fl2int(a_tmp, result, size, resultdim, 32, 9, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
    ss_batch_free_operator(&a_tmp, size);
}

void ss_batch_fl2int(float *a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **a_tmp = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    for (int i = 0; i < size; i++) {
        a_tmp[i] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
        for (int j = 0; j < 4; j++)
            mpz_init(a_tmp[i][j]);
        convertFloat(a[i], 32, 9, &a_tmp[i]);
    }
    ss_batch_fl2int(a_tmp, result, size, resultdim, 32, 9, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
    ss_batch_free_operator(&a_tmp, size);
}

void ss_batch_fl2int(mpz_t **a, mpz_t *result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *result_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t **a_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    for (int i = 0; i < size; i++)
        mpz_init(result_tmp[i]);

    doOperation_FL2Int(a_tmp, result_tmp, alen_sig, alen_exp, blen, size, threadID, net, id, ss);

    /* consider private conditions */
    mpz_t *result_org = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set(result_org[i], result[index_array[3 * i + 2]]);
    ss_batch_handle_priv_cond(result_tmp, result_org, out_cond, priv_cond, counter, size, threadID, net, id, ss);
    for (int i = 0; i < size; i++)
        mpz_set(result[index_array[3 * i + 2]], result_tmp[i]);

    ss_batch_free_operator(&result_tmp, size);
    ss_batch_free_operator(&result_org, size);
    ss_batch_free_operator(&a_tmp, size);
}

void ss_batch_fl2int(mpz_t **a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    int dim1, dim2;
    mpz_t *result_tmp;
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);
    int *result_index_array = (int *)malloc(sizeof(int) * size);
    for (int i = 0; i < size; i++) {
        result_index_array[i] = index_array[3 * i + 2];
        index_array[3 * i + 2] = i;
    }
    ss_batch_fl2int(a, result_tmp, adim, size, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
    for (int i = 0; i < size; i++) {
        dim1 = result_index_array[i] / resultdim;
        dim2 = result_index_array[i] % resultdim;
        mpz_set(result[dim1][dim2], result_tmp[i]);
    }
    free(result_index_array);
    ss_batch_free_operator(&result_tmp, size);
}

void ss_batch_fl2int(mpz_t ***a, mpz_t *result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **a_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    for (int i = 0; i < size; i++)
        index_array[3 * i] = i;
    ss_batch_fl2int(a_tmp, result, size, resultdim, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
    ss_batch_free_operator(&a_tmp, size);
}

void ss_batch_fl2int(mpz_t ***a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    int dim1, dim2;
    mpz_t *result_tmp;
    mpz_t **a_tmp;
    int *result_index_array = (int *)malloc(sizeof(int) * size);

    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    for (int i = 0; i < size; i++) {
        result_index_array[i] = index_array[3 * i + 2];
        index_array[3 * i] = i;
        index_array[3 * i + 2] = i;
    }

    ss_batch_fl2int(a_tmp, result_tmp, size, size, alen_sig, alen_exp, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    for (int i = 0; i < size; i++) {
        dim1 = result_index_array[i] / resultdim;
        dim2 = result_index_array[i] % resultdim;
        mpz_set(result[dim1][dim2], result_tmp[i]);
    }

    free(result_index_array);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}
// INT2INT
void ss_batch_int2int(int *a, mpz_t *result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set_si(a_tmp[i], a[i]);
    ss_batch_int2int(a_tmp, result, size, resultdim, 32, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
    ss_batch_free_operator(&a_tmp, size);
}

void ss_batch_int2int(int *a, mpz_t **result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *a_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set_si(a_tmp[i], a[i]);
    ss_batch_int2int(a_tmp, result, size, resultdim, 32, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
    ss_batch_free_operator(&a_tmp, size);
}

void ss_batch_int2int(mpz_t *a, mpz_t *result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *result_tmp = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *result_org = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++) {
        mpz_init_set(result_tmp[i], a[index_array[3 * i]]);
        mpz_init_set(result_org[i], result[index_array[3 * i + 2]]);
    }
    ss_batch_handle_priv_cond(result_tmp, result_org, out_cond, priv_cond, counter, size, threadID, net, id, ss);
    for (int i = 0; i < size; i++)
        mpz_set(result[index_array[3 * i + 2]], result_tmp[i]);
    ss_batch_free_operator(&result_tmp, size);
    ss_batch_free_operator(&result_org, size);
}

void ss_batch_int2int(mpz_t **a, mpz_t *result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *a_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    for (int i = 0; i < size; i++)
        index_array[3 * i] = i;
    ss_batch_int2int(a_tmp, result, size, resultdim, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
    ss_batch_free_operator(&a_tmp, size);
}

void ss_batch_int2int(mpz_t *a, mpz_t **result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *result_tmp;
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);
    int *result_index_array = (int *)malloc(sizeof(int) * size);
    for (int i = 0; i < size; i++) {
        result_index_array[i] = index_array[3 * i + 2];
        index_array[3 * i + 2] = i;
    }
    ss_batch_int2int(a, result_tmp, adim, size, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
    for (int i = 0; i < size; i++) {
        int dim1 = result_index_array[i] / resultdim;
        int dim2 = result_index_array[i] % resultdim;
        mpz_set(result[dim1][dim2], result_tmp[i]);
    }
    free(result_index_array);
    ss_batch_free_operator(&result_tmp, size);
}

void ss_batch_int2int(mpz_t **a, mpz_t **result, int adim, int resultdim, int alen, int blen, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t *result_tmp, *a_tmp;
    int *result_index_array = (int *)malloc(sizeof(int) * size);
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    for (int i = 0; i < size; i++) {
        result_index_array[i] = index_array[3 * i + 2];
        index_array[3 * i] = i;
        index_array[3 * i + 2] = i;
    }

    ss_batch_int2int(a_tmp, result_tmp, size, size, alen, blen, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    for (int i = 0; i < size; i++) {
        int dim1 = result_index_array[i] / resultdim;
        int dim2 = result_index_array[i] % resultdim;
        mpz_set(result[dim1][dim2], result_tmp[i]);
    }

    free(result_index_array);
    ss_batch_free_operator(&a_tmp, size);
    ss_batch_free_operator(&result_tmp, size);
}

// FL2FL
void ss_batch_fl2fl(float *a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **a_tmp = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    for (int i = 0; i < size; i++) {
        a_tmp[i] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
        for (int j = 0; j < 4; j++)
            mpz_init(a_tmp[i][j]);
        convertFloat(a[i], 32, 9, &a_tmp[i]);
    }
    ss_batch_fl2fl(a_tmp, result, size, resultdim, 32, 9, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
    ss_batch_free_operator(&a_tmp, size);
}

void ss_batch_fl2fl(float *a, mpz_t ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **a_tmp = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    for (int i = 0; i < size; i++) {
        a_tmp[i] = (mpz_t *)malloc(sizeof(mpz_t) * 4);
        for (int j = 0; j < 4; j++)
            mpz_init(a_tmp[i][j]);
        convertFloat(a[i], 32, 9, &a_tmp[i]);
    }
    ss_batch_fl2fl(a_tmp, result, size, resultdim, 32, 9, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
    ss_batch_free_operator(&a_tmp, size);
}

void ss_batch_fl2fl(mpz_t **a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {

    // extract inputs
    mpz_t **result_tmp = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    mpz_t **a_tmp = (mpz_t **)malloc(sizeof(mpz_t *) * 4);
    for (int i = 0; i < 4; i++) {
        a_tmp[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        result_tmp[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++) {
            mpz_init_set(a_tmp[i][j], a[index_array[3 * j]][i]);
            mpz_init(result_tmp[i][j]);
        }
    }

    // start computation
    if (alen_sig >= blen_sig)
        doOperation_Trunc(result_tmp[0], a_tmp[0], alen_sig, alen_sig - blen_sig, size, threadID, net, id, ss);
    else {
        mpz_t tmp, two;
        mpz_init_set_ui(two, 2);
        mpz_init(tmp);
        ss->modPow(tmp, two, blen_sig - alen_sig);
        ss->modMul(result_tmp[0], a_tmp[0], tmp, size);
        mpz_clear(tmp);
        mpz_clear(two);
    }

    ss->modAdd(result_tmp[1], a_tmp[1], alen_sig - blen_sig, size);
    ss->copy(a_tmp[2], result_tmp[2], size);
    ss->copy(a_tmp[3], result_tmp[3], size);

    /* consider private conditions */
    mpz_t *result_new = (mpz_t *)malloc(sizeof(mpz_t) * 4 * size);
    mpz_t *result_org = (mpz_t *)malloc(sizeof(mpz_t) * 4 * size);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 4; j++) {
            mpz_init_set(result_new[4 * i + j], result_tmp[j][i]);
            mpz_init_set(result_org[4 * i + j], result[index_array[3 * i + 2]][j]);
        }
    }
    ss_batch_handle_priv_cond(result_new, result_org, out_cond, priv_cond, counter, 4 * size, threadID, net, id, ss);
    for (int i = 0; i < size; i++)
        for (int j = 0; j < 4; j++)
            mpz_set(result[index_array[3 * i + 2]][j], result_new[4 * i + j]);

    // free memory
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < size; j++) {
            mpz_clear(a_tmp[i][j]);
            mpz_clear(result_tmp[i][j]);
        }
        free(a_tmp[i]);
        free(result_tmp[i]);
    }
    free(a_tmp);
    free(result_tmp);

    ss_batch_free_operator(&result_new, 4 * size);
    ss_batch_free_operator(&result_org, 4 * size);
}

void ss_batch_fl2fl(mpz_t **a, mpz_t ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **result_tmp;
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);
    int *result_index_array = (int *)malloc(sizeof(int) * size);

    for (int i = 0; i < size; i++) {
        result_index_array[i] = index_array[3 * i + 2];
        index_array[3 * i + 2] = i;
    }

    ss_batch_fl2fl(a, result_tmp, adim, size, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 4; j++) {
            int dim1 = result_index_array[i] / resultdim;
            int dim2 = result_index_array[i] % resultdim;
            mpz_set(result[dim1][dim2][j], result_tmp[i][j]);
        }
    }
    free(result_index_array);
    ss_batch_free_operator(&result_tmp, size);
}

void ss_batch_fl2fl(mpz_t ***a, mpz_t **result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **a_tmp;
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    for (int i = 0; i < size; i++)
        index_array[3 * i] = i;
    ss_batch_fl2fl(a_tmp, result, size, resultdim, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);
    ss_batch_free_operator(&a_tmp, size);
}

void ss_batch_fl2fl(mpz_t ***a, mpz_t ***result, int adim, int resultdim, int alen_sig, int alen_exp, int blen_sig, int blen_exp, mpz_t out_cond, mpz_t *priv_cond, int counter, int *index_array, int size, int threadID, NodeNetwork net, int id, SecretShare *ss) {
    mpz_t **result_tmp;
    mpz_t **a_tmp;
    int *result_index_array = (int *)malloc(sizeof(int) * size);
    ss_convert_operator(&a_tmp, a, index_array, adim, size, 1);
    ss_convert_operator(&result_tmp, result, index_array, resultdim, size, 3);

    for (int i = 0; i < size; i++) {
        result_index_array[i] = index_array[3 * i + 2];
        index_array[3 * i + 2] = i;
        index_array[3 * i] = i;
    }

    ss_batch_fl2fl(a_tmp, result_tmp, size, size, alen_sig, alen_exp, blen_sig, blen_exp, out_cond, priv_cond, counter, index_array, size, threadID, net, id, ss);

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < 4; j++) {
            int dim1 = result_index_array[i] / resultdim;
            int dim2 = result_index_array[i] % resultdim;
            mpz_set(result[dim1][dim2][j], result_tmp[i][j]);
        }
    }

    free(result_index_array);
    ss_batch_free_operator(&result_tmp, size);
    ss_batch_free_operator(&a_tmp, size);
}
