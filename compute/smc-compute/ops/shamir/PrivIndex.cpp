/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame
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
#include "PrivIndex.h"

void compute_private_conditions(mpz_t *private_conditions, mpz_t out_cond, mpz_t *priv_cond, int counter, int size) {
    if (out_cond != NULL && counter == -1 && priv_cond == NULL) {
        for (int i = 0; i < size; i++)
            mpz_set(private_conditions[i], out_cond);
    } else if (out_cond == NULL && counter != -1 && priv_cond != NULL) {
        for (int i = 0; i < size; i++)
            if (counter != size)
                mpz_set(private_conditions[i], priv_cond[i / (size / counter)]);
            else
                mpz_set(private_conditions[i], priv_cond[i]);
    } else if (out_cond == NULL && priv_cond == NULL) {
        for (int i = 0; i < size; i++)
            mpz_set_ui(private_conditions[i], 1);
    }
}
// integer
void doOperation_PrivIndex_int(mpz_t index, mpz_t *array, mpz_t result, int dim, int type, int threadID, NodeNetwork net, SecretShare *ss) {
    mpz_t *index_tmp = (mpz_t *)malloc(sizeof(mpz_t));
    mpz_t *result_tmp = (mpz_t *)malloc(sizeof(mpz_t));
    mpz_init_set(index_tmp[0], index);
    mpz_init(result_tmp[0]);

    doOperation_PrivIndex_Read(index_tmp, array, result_tmp, dim, 1, threadID, 0, net, ss);
    mpz_set(result, result_tmp[0]);

    ss_batch_free_operator(&index_tmp, 1);
    ss_batch_free_operator(&result_tmp, 1);
}

// float
void doOperation_PrivIndex_float(mpz_t index, mpz_t **array, mpz_t *result, int dim, int type, int threadID, NodeNetwork net, SecretShare *ss) {

    mpz_t *index_tmp = (mpz_t *)malloc(sizeof(mpz_t));
    mpz_t *array_tmp = (mpz_t *)malloc(sizeof(mpz_t) * 4 * dim);
    mpz_init_set(index_tmp[0], index);
    for (int i = 0; i < dim; i++)
        for (int j = 0; j < 4; j++)
            mpz_init_set(array_tmp[4 * i + j], array[i][j]);

    doOperation_PrivIndex_Read(index_tmp, array_tmp, result, dim, 1, threadID, 1, net, ss);

    ss_batch_free_operator(&index_tmp, 1);
    ss_batch_free_operator(&array_tmp, 4 * dim);

    // mpz_t *index_tmp = (mpz_t *)malloc(sizeof(mpz_t));
    // mpz_t *result_tmp = (mpz_t *)malloc(sizeof(mpz_t));
    // mpz_init_set(index_tmp[0], index);
    // mpz_init(result_tmp[0]);

    // doOperation_PrivIndex_Read(index_tmp, array, result_tmp, dim, 1, threadID, 0);
    // mpz_set(result, result_tmp[0]);

    // ss_batch_free_operator(&index_tmp, 1);
    // ss_batch_free_operator(&result_tmp, 1);
}

void doOperation_PrivIndex_int_arr(mpz_t index, mpz_t **array, mpz_t result, int dim1, int dim2, int type, int threadID, NodeNetwork net, SecretShare *ss) {

    mpz_t *index_tmp = (mpz_t *)malloc(sizeof(mpz_t));
    mpz_t *array_tmp = (mpz_t *)malloc(sizeof(mpz_t) * dim1 * dim2);
    mpz_t *result_tmp = (mpz_t *)malloc(sizeof(mpz_t));

    mpz_init_set(index_tmp[0], index);
    mpz_init(result_tmp[0]);
    for (int i = 0; i < dim1; i++)
        for (int j = 0; j < dim2; j++)
            mpz_init_set(array_tmp[i * dim2 + j], array[i][j]);

    doOperation_PrivIndex_Read(index_tmp, array_tmp, result_tmp, dim1 * dim2, 1, threadID, 0, net, ss);
    mpz_set(result, result_tmp[0]);

    ss_batch_free_operator(&index_tmp, 1);
    ss_batch_free_operator(&result_tmp, 1);
    ss_batch_free_operator(&array_tmp, dim1 * dim2);
}

void doOperation_PrivIndex_float_arr(mpz_t index, mpz_t ***array, mpz_t *result, int dim1, int dim2, int type, int threadID, NodeNetwork net, SecretShare *ss) {
    mpz_t *index_tmp = (mpz_t *)malloc(sizeof(mpz_t));
    mpz_t *array_tmp = (mpz_t *)malloc(sizeof(mpz_t) * 4 * dim1 * dim2);
    mpz_init_set(index_tmp[0], index);
    for (int i = 0; i < dim1; i++)
        for (int j = 0; j < dim2; j++)
            for (int k = 0; k < 4; k++)
                mpz_init_set(array_tmp[4 * (i * dim2 + j) + k], array[i][j][k]);

    doOperation_PrivIndex_Read(index_tmp, array_tmp, result, dim1 * dim2, 1, threadID, 1, net, ss);

    ss_batch_free_operator(&index_tmp, 1);
    ss_batch_free_operator(&array_tmp, 4 * dim1 * dim2);
}

void doOperation_PrivIndex_Read(mpz_t *index, mpz_t *array, mpz_t *result, int m, int size, int threadID, int type, NodeNetwork net, SecretShare *ss) {
    int logm = ceil(log2(m));
    int dtype_offset = (type == 0) ? 1 : 4;
    //	printf("dim %d, size %d, threadID %d, K %d, m %d \n", dim, size, threadID, K, m);

    mpz_t const1, const2, constK, pow2K, temp;
    mpz_t *S = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *r = (mpz_t *)malloc(sizeof(mpz_t) * size);
    mpz_t *C = (mpz_t *)malloc(sizeof(mpz_t) * size);
    int peers = ss->getPeers();
    mpz_t **resultShares = (mpz_t **)malloc(sizeof(mpz_t *) * peers);
    mpz_t **shares = (mpz_t **)malloc(sizeof(mpz_t *) * peers);
    mpz_t **buffer = (mpz_t **)malloc(sizeof(mpz_t *) * peers);
    mpz_t **U = (mpz_t **)malloc(sizeof(mpz_t *) * (logm + 1));
    mpz_t **U1 = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    mpz_t **B = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    int nb;
    int b_size = pow(2, logm);
    unsigned long int C1;

    // initialization
    mpz_init_set_ui(const1, 1);
    mpz_init_set_ui(const2, 2);
    mpz_init_set_ui(constK, logm);
    mpz_init(pow2K);
    mpz_init(temp);
    for (int i = 0; i < size; i++) {
        mpz_init_set_ui(S[i], 0);
        mpz_init_set_ui(r[i], 0);
        mpz_init_set_ui(C[i], 0);
        U1[i] = (mpz_t *)malloc(sizeof(mpz_t) * (logm + 1));
        for (int j = 0; j < logm + 1; j++)
            mpz_init(U1[i][j]);
        B[i] = (mpz_t *)malloc(sizeof(mpz_t) * b_size);
        for (int j = 0; j < b_size; j++)
            mpz_init(B[i][j]);
    }
    for (int i = 0; i < peers; i++) {
        resultShares[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++)
            mpz_init(resultShares[i][j]);
        shares[i] = (mpz_t *)malloc(sizeof(mpz_t) * dtype_offset * size);
        buffer[i] = (mpz_t *)malloc(sizeof(mpz_t) * dtype_offset * size);
        for (int j = 0; j < dtype_offset * size; j++) {
            mpz_init(shares[i][j]);
            mpz_init(buffer[i][j]);
        }
    }
    for (int i = 0; i < logm + 1; i++) {
        U[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++)
            mpz_init(U[i][j]);
    }
    for (int i = 0; i < dtype_offset * size; i++)
        mpz_init_set_ui(result[i], 0);

    // struct timeval tv1, tv2, tv3, tv4, tv5;
    // gettimeofday(&tv1, NULL);
    // start computation
    /*** Lookup: LINE 1: PRandM(log_n, log_n) ***/
    PRandM(logm, size, U, threadID, net, ss);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < logm + 1; j++)
            mpz_set(U1[i][j], U[j][i]);
        mpz_set(r[i], U1[i][logm]);
    }
    // gettimeofday(&tv2, NULL);
    // std::cout << "Time PRandM: " << time_diff(&tv1,&tv2) << std::endl;

    /*** Lookup: LINE 2: 1 - AllOr ***/
    AllOr(U1, logm, B, size, threadID, net, ss);
    // gettimeofday(&tv3, NULL);

    for (int i = 0; i < size; i++)
        for (int j = 0; j < b_size; j++)
            ss->modSub(B[i][j], const1, B[i][j]);

    /*** Lookup: LINE 3: c = Output(j + 2^log_n*r' + r) ***/
    PRandInt(logm, logm, size, S, threadID, ss);
    ss->modPow(pow2K, const2, constK);
    ss->modMul(S, S, pow2K, size);
    ss->modAdd(C, index, r, size);
    ss->modAdd(C, C, S, size);
    // net.broadcastToPeers(C, size, resultShares, threadID);
    // ss->reconstructSecret(C, resultShares, size);
    Open(C, C, size, threadID, net, ss);
    /*** Lookup: LINE 4: c' = c mod 2^log_n ***/
    ss->mod(C, C, pow2K, size);
    // gettimeofday(&tv4, NULL);
    // std::cout << "Time PRandInt: " << time_diff(&tv3,&tv4) << std::endl;

    /*** Lookup: LINE 5, 6: b = b_(c'- i mod 2^log_n) * array_i ***/
    for (int i = 0; i < size; i++) {
        C1 = mpz_get_ui(C[i]);
        for (int j = 0; j < m; j++) {
            nb = (C1 - j) % b_size;
            // printf("nb %d C1 %d j %d b_size %d \n", nb, C1, j, b_size);
            for (int k = 0; k < dtype_offset; k++) {
                mpz_mul(temp, B[i][nb], array[j * dtype_offset + k]);
                // gmp_printf("B[%d][%d] %Zd array[%d] %Zd", i, nb, B[i][nb], j*m+k, array[j*m+k]);
                mpz_add(result[i * dtype_offset + k], result[i * dtype_offset + k], temp);
            }
        }
    }

    ss->getShares(shares, result, dtype_offset * size);
    net.multicastToPeers(shares, buffer, dtype_offset * size, threadID);
    ss->reconstructSecret(result, buffer, dtype_offset * size);
    // gettimeofday(&tv5, NULL);
    // std::cout << "Time DotProduct: " << time_diff(&tv4,&tv5) << std::endl;

    // free the memory
    mpz_clear(const1);
    mpz_clear(const2);
    mpz_clear(constK);
    mpz_clear(pow2K);
    for (int i = 0; i < size; i++) {
        mpz_clear(S[i]);
        mpz_clear(r[i]);
        mpz_clear(C[i]);
        for (int j = 0; j < logm + 1; ++j)
            mpz_clear(U1[i][j]);
        free(U1[i]);
        for (int j = 0; j < b_size; ++j)
            mpz_clear(B[i][j]);
        free(B[i]);
    }
    free(S);
    free(r);
    free(C);
    free(U1);
    free(B);
    for (int i = 0; i < peers; i++) {
        for (int j = 0; j < size; j++)
            mpz_clear(resultShares[i][j]);
        free(resultShares[i]);
        for (int j = 0; j < dtype_offset * size; j++) {
            mpz_clear(shares[i][j]);
            mpz_clear(buffer[i][j]);
        }
        free(shares[i]);
        free(buffer[i]);
    }
    free(resultShares);
    free(shares);
    free(buffer);
    for (int i = 0; i < logm + 1; ++i) {
        for (int j = 0; j < size; ++j)
            mpz_clear(U[i][j]);
        free(U[i]);
    }
    free(U);
}

void doOperation_PrivIndex_Write(mpz_t *index, mpz_t *array, mpz_t *value, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, int threadID, int type, NodeNetwork net, SecretShare *ss) {
    int K = ceil(log2(dim));
    int m = (type == 0) ? 1 : 4;
    mpz_t **U = (mpz_t **)malloc(sizeof(mpz_t *) * (K + 1));
    mpz_t **U1 = (mpz_t **)malloc(sizeof(mpz_t *) * size);
    mpz_t *temp1 = (mpz_t *)malloc(sizeof(mpz_t) * m * size * dim);
    mpz_t *temp2 = (mpz_t *)malloc(sizeof(mpz_t) * m * size * dim);
    mpz_t *temp3 = (mpz_t *)malloc(sizeof(mpz_t) * m * size * dim);
    mpz_t *temp4 = (mpz_t *)malloc(sizeof(mpz_t) * m * dim);
    mpz_t *temp5 = (mpz_t *)malloc(sizeof(mpz_t) * m * dim);
    mpz_t const1;
    mpz_init_set_ui(const1, 1);

    // initialization
    int **bitArray = (int **)malloc(sizeof(int *) * dim);
    for (int i = 0; i < dim; i++)
        bitArray[i] = (int *)malloc(sizeof(int) * K);
    for (int i = 0; i < m * size * dim; i++) {
        mpz_init(temp1[i]);
        mpz_init(temp2[i]);
        mpz_init(temp3[i]);
    }
    for (int i = 0; i < m * dim; i++) {
        mpz_init(temp4[i]);
        mpz_init(temp5[i]);
    }
    for (int i = 0; i < K + 1; i++) {
        U[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++)
            mpz_init(U[i][j]);
    }
    for (int i = 0; i < size; i++) {
        U1[i] = (mpz_t *)malloc(sizeof(mpz_t) * (K + 1));
        for (int j = 0; j < K + 1; j++)
            mpz_init(U1[i][j]);
    }

    // start computation
    compute_private_conditions(temp3, out_cond, priv_cond, counter, size);
    for (int i = 0; i < dim; i++)
        binarySplit(i, bitArray[i], K);
    doOperation_bitDec(U, index, K, K, size, threadID, net, ss);
    for (int i = 0; i < size; i++)
        for (int j = 0; j < K + 1; j++)
            mpz_set(U1[i][j], U[j][i]);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < dim; j++) {
            mpz_set(temp1[i * dim + j], const1);
            mpz_set(temp2[i * dim + j], temp3[i]);
        }
    }
    for (int i = 0; i < K; i++) {
        for (int j = 0; j < size; j++) {
            for (int k = 0; k < dim; k++) {
                if (bitArray[k][i] == 0)
                    ss->modSub(temp3[j * dim + k], const1, U1[j][i]);
                else
                    mpz_set(temp3[j * dim + k], U1[j][i]);
            }
        }
        Mult(temp1, temp1, temp3, size * dim, threadID, net, ss);
    }
    Mult(temp1, temp1, temp2, size * dim, threadID, net, ss);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < dim; j++) {
            for (int k = 0; k < m; k++) {
                mpz_set(temp2[i * dim * m + j * m + k], value[i * m + k]);
                mpz_set(temp3[i * dim * m + j * m + k], temp1[i * dim + j]);
            }
        }
    }
    Mult(temp2, temp2, temp3, m * size * dim, threadID, net, ss);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < dim; j++) {
            for (int k = 0; k < m; k++) {
                ss->modAdd(temp4[j * m + k], temp4[j * m + k], temp2[i * dim * m + j * m + k]);
                ss->modAdd(temp5[j * m + k], temp5[j * m + k], temp1[i * dim + j]);
            }
        }
    }
    ss->modSub(temp5, const1, temp5, m * dim);
    Mult(temp5, temp5, array, m * dim, threadID, net, ss);
    ss->modAdd(array, temp4, temp5, m * dim);

    // free memory
    for (int i = 0; i < dim; i++)
        free(bitArray[i]);
    free(bitArray);
    for (int i = 0; i < m * size * dim; i++) {
        mpz_clear(temp1[i]);
        mpz_clear(temp2[i]);
        mpz_clear(temp3[i]);
    }
    for (int i = 0; i < m * dim; i++) {
        mpz_clear(temp4[i]);
        mpz_clear(temp5[i]);
    }
    free(temp1);
    free(temp2);
    free(temp3);
    free(temp4);
    free(temp5);
    for (int i = 0; i < K + 1; i++) {
        for (int j = 0; j < size; j++)
            mpz_clear(U[i][j]);
        free(U[i]);
    }
    free(U);

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < K + 1; j++)
            mpz_clear(U1[i][j]);
        free(U1[i]);
    }
    free(U1);
    mpz_clear(const1);
}
// array [batch_size][K]
// result [batch_size][2^k] (interpreted as 2^k blocks of dimension "batch_size")
void AllOr(mpz_t **array, int k, mpz_t **result, int batch_size, int threadID, NodeNetwork net, SecretShare *ss) {
    // LINE 1
    mpz_t const1;
    mpz_init_set_ui(const1, 1);
    if (k == 1) {
        for (int i = 0; i < batch_size; i++) {
            mpz_set(result[i][0], array[i][0]);
            ss->modSub(result[i][1], const1, array[i][0]);
        }
        return;
    }

    // initialization
    int b_size = 2;
    for (int i = 0; i < k; i++)
        b_size *= 2;
    mpz_t *buff = (mpz_t *)malloc(sizeof(mpz_t) * b_size * batch_size);
    mpz_t *u1 = (mpz_t *)malloc(sizeof(mpz_t) * b_size * batch_size);
    mpz_t *v1 = (mpz_t *)malloc(sizeof(mpz_t) * b_size * batch_size);
    mpz_t *add_b = (mpz_t *)malloc(sizeof(mpz_t) * b_size * batch_size);
    mpz_t *mul_b = (mpz_t *)malloc(sizeof(mpz_t) * b_size * batch_size);
    for (int i = 0; i < b_size * batch_size; i++) {
        mpz_init(buff[i]);
        mpz_init(u1[i]);
        mpz_init(v1[i]);
        mpz_init(add_b[i]);
        mpz_init(mul_b[i]);
    }

    // struct timeval tv1, tv2;
    // gettimeofday(&tv1, NULL);

    // make structure
    //  LINE 3, 4, 5
    int round = 0;
    int nS = k - 1;
    int sizeLen = 2;
    while (nS > 1) {
        round++;
        nS /= 2;
        sizeLen *= 2;
    }
    // round is one fewer than the actual number of rounds, because the first one is computed separately

    int *sizeArray = (int *)malloc(sizeof(int) * sizeLen);
    int len = 1;
    sizeArray[0] = k; // divide and get unit size
    for (int x = 0; x <= round; x++) {
        for (int i = len - 1; i >= 0; i--) {
            int val = sizeArray[i];
            sizeArray[i * 2] = val / 2;
            sizeArray[i * 2 + 1] = val / 2;
            if (val & 0x01)
                sizeArray[i * 2 + 1]++;
        }
        len *= 2;
    }

    int oPos = 0;
    int iPos = 0;
    // merge & multiply
    //  LINE 6, 7
    // first Round, (first Round had block which size is 0)

    for (int n = 0; n < batch_size; n++) {
        iPos = 0;
        for (int i = 0; i < sizeLen; i += 2) {
            if (sizeArray[i] != 0 && sizeArray[i + 1] != 0) {
                mpz_set(u1[oPos], array[n][iPos]);
                ss->modSub(u1[oPos + 1], const1, array[n][iPos]);
                mpz_set(u1[oPos + 2], u1[oPos]);
                mpz_set(u1[oPos + 3], u1[oPos + 1]);
                mpz_set(v1[oPos], array[n][iPos + 1]);
                mpz_set(v1[oPos + 1], v1[oPos]);
                ss->modSub(v1[oPos + 2], const1, array[n][iPos + 1]);
                mpz_set(v1[oPos + 3], v1[oPos + 2]);
                iPos += 2;
                oPos += 4;
            } else
                iPos++;
        }
    }
    ss->modAdd(add_b, u1, v1, oPos);
    // 1 rou, net, ssnd
    Mult(mul_b, u1, v1, oPos, threadID, net, ss);
    ss->modSub(u1, add_b, mul_b, oPos);

    int oPos2 = 0;
    oPos = 0;
    for (int n = 0; n < batch_size; n++) {
        iPos = 0;
        for (int i = 0; i < sizeLen; i += 2) {
            if (sizeArray[i] != 0 && sizeArray[i + 1] != 0) {
                for (int x = 0; x < 4; x++) {
                    mpz_set(buff[oPos2], u1[oPos]);
                    oPos++;
                    oPos2++;
                }
                iPos += 2;
            } else {
                mpz_set(buff[oPos2], array[n][iPos]);
                ss->modSub(buff[oPos2 + 1], const1, array[n][iPos]);
                oPos2 += 2;
                iPos++;
            }
        }
    }
    // if block size is 0, then recover to original data
    for (int n = 0; n < sizeLen; n += 2) {
        if (sizeArray[n] == 0 || sizeArray[n + 1] == 0)
            sizeArray[n / 2] = 2;
        else
            sizeArray[n / 2] = sizeArray[n] * sizeArray[n + 1] * 4;
    }

    sizeLen /= 2;
    int uL, vL;

    // other Round
    for (int x = 0; x < round; x++) {
        oPos = 0;
        iPos = 0;
        for (int n = 0; n < batch_size; n++) {
            for (int i = 0; i < sizeLen; i += 2) {
                uL = sizeArray[i];
                vL = sizeArray[i + 1];
                for (int v = 0; v < vL; v++)
                    for (int u = 0; u < uL; u++) {
                        mpz_set(u1[oPos], buff[iPos + u]);
                        mpz_set(v1[oPos], buff[iPos + uL + v]);
                        oPos++;
                    }
                iPos += uL + vL;
            }
        }
        ss->modAdd(add_b, u1, v1, oPos);
        // rou, net, ssnd
        Mult(mul_b, u1, v1, oPos, threadID, net, ss);
        // std::cout << "	2Mul->do: " << oPos << std::endl;
        ss->modSub(buff, add_b, mul_b, oPos);
        sizeLen /= 2;
        for (int n = 0; n < sizeLen; n++)
            sizeArray[n] = sizeArray[n * 2] * sizeArray[n * 2 + 1];
    }

    oPos /= batch_size;
    for (int x = 0; x < batch_size; x++)
        for (int i = 0; i < oPos; i++)
            mpz_set(result[x][i], buff[x * oPos + i]);

    // gettimeofday(&tv2, NULL);
    // std::cout << "	Time round: " << time_diff(&tv1,&tv2) << std::endl;

    // Free the memory
    mpz_clear(const1);
    free(buff);
    free(u1);
    free(v1);
    free(add_b);
    free(mul_b);
    free(sizeArray);
}

double time_diff(struct timeval *t1, struct timeval *t2) {
    double elapsed;

    if (t1->tv_usec > t2->tv_usec) {
        t2->tv_usec += 1000000;
        t2->tv_sec--;
    }

    elapsed = (t2->tv_sec - t1->tv_sec) + (t2->tv_usec - t1->tv_usec) / 1000000.0;

    return elapsed;
}

// needed to convert integer inputs to mpz_t
void doOperation_PrivIndex_Write(mpz_t *index, mpz_t *array, int *values, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, int threadID, int type, NodeNetwork net, SecretShare *ss) {

    mpz_t *val = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set_si(val[i], values[i]);
    // smc_privindex_write(indices, array, len_sig, len_exp, val, dim, size, out_cond, priv_cond, counter, type, threadID);
    doOperation_PrivIndex_Write(index, array, val, dim, size, out_cond, priv_cond, counter, threadID, 0, net, ss);
    ss_batch_free_operator(&val, size);
}

void doOperation_PrivIndex_Write_2d(mpz_t *index, mpz_t **array, int *values, int dim1, int dim2, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, int threadID, int type, NodeNetwork net, SecretShare *ss) {

    mpz_t *val = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set_si(val[i], values[i]);
    // smc_privindex_write(indices, array, len_sig, len_exp, val, dim, size, out_cond, priv_cond, counter, type, threadID);
    doOperation_PrivIndex_Write_2d(index, array, val, dim1, dim2, size, out_cond, priv_cond, counter, threadID, 0, net, ss);
    ss_batch_free_operator(&val, size);
}

void doOperation_PrivIndex_Write_2d(mpz_t *index, mpz_t **array, mpz_t *values, int dim1, int dim2, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, int threadID, int type, NodeNetwork net, SecretShare *ss) {
    mpz_t *array_tmp = (mpz_t *)malloc(sizeof(mpz_t) * dim1 * dim2);
    for (int i = 0; i < dim1; i++)
        for (int j = 0; j < dim2; j++)
            mpz_init_set(array_tmp[i * dim2 + j], array[i][j]);

    // mpz_t *val = (mpz_t *)malloc(sizeof(mpz_t) * size);
    // for (int i = 0; i < size; i++)
    //     mpz_init_set_si(val[i], values[i]);
    // smc_privindex_write(indices, array, len_sig, len_exp, val, dim, size, out_cond, priv_cond, counter, type, threadID);
    doOperation_PrivIndex_Write(index, array_tmp, values, dim1 * dim2, size, out_cond, priv_cond, counter, threadID, 0, net, ss);
    // ss_batch_free_operator(&val, size);

    ss_batch_free_operator(&array_tmp, dim1 * dim2);
}

// void doOperation_PrivIndex_Write_int_mpz(mpz_t *index, mpz_t *array, mpz_t value, int dim, int size, mpz_t out_cond, mpz_t *priv_cond, int counter, int threadID, int type, NodeNetwork net,  SecretShare *ss){
//     mpz_t *index_tmp = (mpz_t *)malloc(sizeof(mpz_t));
//     mpz_t *value_tmp = (mpz_t *)malloc(sizeof(mpz_t));
//     mpz_init_set(index_tmp[0], index);
//     mpz_init_set(value_tmp[0], value);

//     doOperation_PrivIndex_Write(index_tmp, array, value_tmp, dim, 1, out_cond, priv_cond, counter, threadID, 0);

//     ss_batch_free_operator(&index_tmp, 1);
//     ss_batch_free_operator(&value_tmp, 1);

// }
