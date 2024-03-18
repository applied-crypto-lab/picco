#include "matMult.h"
// extern "C"{
// #include "../aes_ni.h"
// }

void Rss_MatMult(Lint ***c, Lint ***a, Lint ***b, uint m, uint n, uint s, uint ring_size, NodeNetwork *nodeNet) {
    //  [m][n] size matrix a;   [n][s] size matrix b
    //  For party 1, a[0,1]=a_2,3; b[0,1]=b_2,3;  c[0,1] = c_2,3;
    //  For party 2, a[0,1]=a_3,1; b[0,1]=b_3,1;  c[0,1] = c_3,1;
    //  For party 3, a[0,1]=a_1,2; b[0,1]=b_1,2;  c[0,1] = c_1,2;

    // uint bytes = (nodeNet->RING[ring_size]+7)>>3;
    uint bytes = (ring_size + 7) >> 3;
    int i, j, k; // used for loops

    Lint **v_a = new Lint *[m];
    for (i = 0; i < m; i++) {
        v_a[i] = new Lint[s];
        memset(v_a[i], 0, sizeof(Lint) * s);
    }

    Lint *send_buf = new Lint[m * s];
    memset(send_buf, 0, sizeof(Lint) * m * s);
    Lint *recv_buf = new Lint[m * s];
    memset(recv_buf, 0, sizeof(Lint) * m * s);

    uint8_t *buffer = new uint8_t[bytes * m * s];
    nodeNet->prg_getrandom(1, bytes, m * s, buffer);

    for (i = 0; i < m; i++) {
        for (j = 0; j < s; j++) {
            for (k = 0; k < n; k++) {
                v_a[i][j] += a[0][i][k] * b[0][k][j] + a[0][i][k] * b[1][k][j] + a[1][i][k] * b[0][k][j];
            }
            memcpy(c[0][i] + j, buffer + (i * s + j) * bytes, bytes);
            // nodeNet->prg_getrandom(1, bytes, c[0][i]+j);
            v_a[i][j] = v_a[i][j] - c[0][i][j];
        }
    }

    // send v_a
    for (i = 0; i < m; i++) {
        memcpy(send_buf + i * s, v_a[i], sizeof(Lint) * s);
    }
    nodeNet->SendAndGetDataFromPeer(send_buf, recv_buf, m * s, ring_size);
    for (i = 0; i < m; i++) {
        memcpy(c[1][i], recv_buf + i * s, sizeof(Lint) * s);
    }

    nodeNet->prg_getrandom(0, bytes, m * s, buffer);
    for (i = 0; i < m; i++) {
        for (j = 0; j < s; j++) {
            c[1][i][j] = c[1][i][j] + c[0][i][j];

            // nodeNet->prg_getrandom(0, bytes, c[0][i]+j);
            memcpy(c[0][i] + j, buffer + (i * s + j) * bytes, bytes);
            c[0][i][j] = c[0][i][j] + v_a[i][j];
        }
    }
    // free
    delete[] send_buf;
    delete[] recv_buf;
    delete[] buffer;
    for (i = 0; i < m; i++) {
        delete[] v_a[i];
    }
    delete[] v_a;
}

void Rss_MatMultArray(Lint **c, Lint **a, Lint **b, uint m, uint n, uint s, uint ring_size, NodeNetwork *nodeNet) {
    // modified implementation that uses a 1d array representation of a matrix
    // a [m*n] = i*n+j (accessing the (i,j)th element)
    // [m][n] size matrix a;   [n][s] size matrix b
    // For party 1, a[0,1]=a_2,3; b[0,1]=b_2,3;  c[0,1] = c_2,3;
    // For party 2, a[0,1]=a_3,1; b[0,1]=b_3,1;  c[0,1] = c_3,1;
    // For party 3, a[0,1]=a_1,2; b[0,1]=b_1,2;  c[0,1] = c_1,2;
    uint bytes = (ring_size + 7) >> 3;
    uint i, j, k; // used for loops
                  // printf("bytes: %u\n", bytes);
    // struct timeval start;
    // struct timeval end;
    // unsigned long timer;

    Lint *v = new Lint[m * s];
    memset(v, 0, sizeof(Lint) * (m * s));

    Lint *send_buf = new Lint[m * s];
    memset(send_buf, 0, sizeof(Lint) * m * s);
    Lint *recv_buf = new Lint[m * s];
    memset(recv_buf, 0, sizeof(Lint) * m * s);

    uint8_t *buffer = new uint8_t[bytes * m * s];
    nodeNet->prg_getrandom(1, bytes, m * s, buffer);
    // memset(buffer, 0, bytes*m*s); //USED FOR TESTING
    // gettimeofday(&start, NULL); //start timer here

    for (i = 0; i < m; i++) {
        for (j = 0; j < s; j++) {
            for (k = 0; k < n; k++) {
                v[i * s + j] += a[0][i * n + k] * b[0][k * s + j] + a[0][i * n + k] * b[1][k * s + j] + a[1][i * n + k] * b[0][k * s + j];
            }
            memcpy(c[0] + (i * s + j), buffer + (i * s + j) * bytes, bytes);
            v[i * s + j] = v[i * s + j] - c[0][i * s + j];
        }
    }
    // gettimeofday(&end, NULL); //stop timer here
    // timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    // printf("Runtime for MatMult dot prod 1 with data size %d = %.6lf s\n", m * s, (double)(timer * 1e-6));

    // send v_a
    for (i = 0; i < m; i++) {
        // memcpy(send_buf+i*s, v+i, sizeof(Lint)*s);
        memcpy(send_buf + i * s, v + i * s, sizeof(Lint) * s);
    }

    // gettimeofday(&start, NULL); //start timer here
    nodeNet->SendAndGetDataFromPeer(send_buf, recv_buf, m * s, ring_size);
    // gettimeofday(&end, NULL); //stop timer here
    // timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    // printf("Runtime for MatMult send with data size %d = %.6lf s\n", m * s, (double)(timer * 1e-6));

    for (i = 0; i < m; i++) {
        memcpy(c[1] + i * s, recv_buf + i * s, sizeof(Lint) * s); // PROBLEM HERE
    }

    nodeNet->prg_getrandom(0, bytes, m * s, buffer);
    // memset(buffer, 0, bytes*m*s); //USED FOR TESTING

    // gettimeofday(&start, NULL); //start timer here
    for (i = 0; i < m; i++) {
        for (j = 0; j < s; j++) {
            c[1][i * s + j] = c[1][i * s + j] + c[0][i * s + j];

            memcpy(c[0] + (i * s + j), buffer + (i * s + j) * bytes, bytes);
            c[0][i * s + j] = c[0][i * s + j] + v[i * s + j];
        }
    }
    // gettimeofday(&end, NULL); //stop timer here
    // timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
    // printf("Runtime for MatMult dot prod 2 with data size %d = %.6lf s\n", m * s, (double)(timer * 1e-6));

    delete[] send_buf;
    delete[] recv_buf;
    delete[] buffer;
    delete[] v;
    // for(i = 0; i< m; i++){
    //     delete [] v_a[i];
    // }
    // delete [] v_a;
}

void Rss_MatMultArray_batch(Lint **c, Lint **a, Lint **b, uint m, uint n, uint s, uint ring_size, uint batch_size, uint weight_flag_a, uint weight_flag_b, NodeNetwork *nodeNet) {
    // modified implementation that uses a 1d array representation of a matrix
    // a [m*n] = i*n+j (accessing the (i,j)th element)
    // [m][n] size matrix a;   [n][s] size matrix b
    // ---> [m][s] size matrix c;
    // For party 1, a[0,1]=a_2,3; b[0,1]=b_2,3;  c[0,1] = c_2,3;
    // For party 2, a[0,1]=a_3,1; b[0,1]=b_3,1;  c[0,1] = c_3,1;
    // For party 3, a[0,1]=a_1,2; b[0,1]=b_1,2;  c[0,1] = c_1,2;

    // if weight_flag == 0 --> the a matrix is the same throughout the computation
    uint numShares = nodeNet->getNumShares();

    uint bytes = (ring_size + 7) >> 3;
    uint i, j, k, l; // used for loops
    // printf("bytes: %u\n", bytes);

    Lint *v = new Lint[batch_size * m * s];
    memset(v, 0, sizeof(Lint) * (batch_size * m * s));

    Lint *send_buf = new Lint[batch_size * m * s];
    memset(send_buf, 0, sizeof(Lint) * batch_size * m * s);
    Lint *recv_buf = new Lint[batch_size * m * s];
    memset(recv_buf, 0, sizeof(Lint) * batch_size * m * s);

    uint8_t *buffer = new uint8_t[bytes * batch_size * m * s];
    nodeNet->prg_getrandom(1, bytes, batch_size * m * s, buffer);
    // memset(buffer, 0, bytes*batch_size*m*s); //USED FOR TESTING

    Lint **b_transpose = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        b_transpose[i] = new Lint[batch_size * n * s];
        memset(b_transpose[i], 0, sizeof(Lint) * batch_size * n * s);
    }
    for (int ss = 0; ss < numShares; ss++) {
        for (l = 0; l < batch_size; l++) {
            for (i = 0; i < n; ++i) {
                for (j = 0; j < s; ++j) {
                    b_transpose[ss][weight_flag_b * l * n * s + j * n + i] = b[ss][weight_flag_b * l * n * s + i * s + j];
                }
            }
        }
    }

    uint a_index, b_index, v_index;
    for (l = 0; l < batch_size; l++) {

        for (i = 0; i < m; i++) {
            for (j = 0; j < s; j++) {
                v_index = l * m * s + i * s + j;
                for (k = 0; k < n; k++) {
                    a_index = weight_flag_a * l * m * n + i * n + k;
                    b_index = weight_flag_b * l * n * s + j * n + k;
                    // printf("l, i, j, k      : %u, %u, %u, %u\n",  l,i,j,k);
                    // printf("a_index, b_index: %u, %u\n",  a_index, b_index);

                    v[v_index] +=
                        a[0][a_index] * b_transpose[0][b_index] + a[0][a_index] * b_transpose[1][b_index] + a[1][a_index] * b_transpose[0][b_index];
                }
                memcpy(c[0] + (v_index), buffer + (v_index)*bytes, bytes);
                v[v_index] = v[v_index] - c[0][v_index];
            }
        }
    }

    // send v_a
    for (l = 0; l < batch_size; l++) {

        for (i = 0; i < m; i++) {
            // memcpy(send_buf+i*s, v+i, sizeof(Lint)*s);
            memcpy(send_buf + i * s + l * m * s, v + i * s + l * m * s, sizeof(Lint) * s);
        }
    }
    nodeNet->SendAndGetDataFromPeer(send_buf, recv_buf, batch_size * m * s, ring_size);

    for (l = 0; l < batch_size; l++) {
        for (i = 0; i < m; i++) {
            memcpy(c[1] + i * s + l * m * s, recv_buf + i * s + l * m * s, sizeof(Lint) * s); // PROBLEM HERE
        }
    }

    nodeNet->prg_getrandom(0, bytes, batch_size * m * s, buffer);
    // memset(buffer, 0, bytes*batch_size*m*s); //USED FOR TESTING

    for (l = 0; l < batch_size; l++) {
        for (i = 0; i < m; i++) {
            for (j = 0; j < s; j++) {
                c[1][l * m * s + i * s + j] = c[1][l * m * s + i * s + j] + c[0][l * m * s + i * s + j];

                memcpy(c[0] + (l * m * s + i * s + j), buffer + (l * m * s + i * s + j) * bytes, bytes);
                c[0][l * m * s + i * s + j] = c[0][l * m * s + i * s + j] + v[l * m * s + i * s + j];
            }
        }
    }

    delete[] send_buf;
    delete[] recv_buf;
    delete[] buffer;
    delete[] v;
    for (i = 0; i < numShares; i++) {
        delete[] b_transpose[i];
    }
    delete[] b_transpose;
}

void Rss_dw_matrixmul(Lint **res, Lint **x, Lint **W, uint width, uint height, uint channels, uint filter, uint ring_size, uint batch_size, uint stride, NodeNetwork *nodeNet) {

    uint output_dim = batch_size * channels * (width / stride) * (height / stride);
    // uint in_dim = batch_size * channels * (width) * (height);
    uint bytes = (ring_size + 7) >> 3;

    Lint *v = new Lint[output_dim];
    memset(v, 0, sizeof(Lint) * (output_dim));

    Lint *send_buf = new Lint[output_dim];
    memset(send_buf, 0, sizeof(Lint) * output_dim);
    Lint *recv_buf = new Lint[output_dim];
    memset(recv_buf, 0, sizeof(Lint) * output_dim);

    uint8_t *buffer = new uint8_t[bytes * output_dim];
    nodeNet->prg_getrandom(1, bytes, output_dim, buffer);
    // memset(buffer, 0, bytes*batch_size*m*s); //USED FOR TESTING
    uint padded_height = height + 2;

    uint W_index, x_index;
    uint dp_index = 0;

    for (uint b = 0; b < batch_size; b++) {

        for (uint i = (stride - 1); i < width; i += stride) {
            for (uint j = (stride - 1); j < height; j += stride) {

                // calculating the actual dot prod
                for (uint fi = 0; fi < filter; fi++) {
                    for (uint fj = 0; fj < filter; fj++) {
                        
                        for (uint c = 0; c < channels; c++) {
                            W_index = fi * filter * channels + fj * channels + c;

                            x_index = (i + fi) * (padded_height)*channels + (j + fj) * channels + c + (output_dim / batch_size) * b; // (out_dim/batch_size) * b used for multiple input images

                            v[dp_index] += x[0][x_index] * W[0][W_index] + x[0][x_index] * W[1][W_index] + x[1][x_index] * W[0][W_index];
                        }
                    }
                    memcpy(res[0] + (dp_index), buffer + (dp_index)*bytes, bytes);
                    v[dp_index] = v[dp_index] - res[0][dp_index];

                    dp_index++;
                }
            }
        }
    }

    // sending v_a
    memcpy(send_buf, v, sizeof(Lint) * output_dim);
    nodeNet->SendAndGetDataFromPeer(send_buf, recv_buf, output_dim, ring_size);
    memcpy(res[1], recv_buf, sizeof(Lint) * output_dim);

    nodeNet->prg_getrandom(0, bytes, output_dim, buffer);

    for (uint i = 0; i < output_dim; i++) {
        res[1][i] = res[1][i] + res[0][i];
        memcpy(res[0] + (i), buffer + (i)*bytes, bytes);
        res[0][i] = res[0][i] + v[i];
    }

    delete[] send_buf;
    delete[] recv_buf;
    delete[] buffer;
    delete[] v;
}

void Rss_pw_matrixmul(Lint **res, Lint **x, Lint **W, uint width, uint height, uint in_channels, uint kernel, uint out_channels, uint ring_size, uint batch_size, NodeNetwork *nodeNet) {

    uint output_dim = batch_size * out_channels * width * height;
    uint in_dim = batch_size * in_channels * width * height;
    uint bytes = (ring_size + 7) >> 3;

    Lint *v = new Lint[output_dim];
    memset(v, 0, sizeof(Lint) * (output_dim));

    Lint *send_buf = new Lint[output_dim];
    memset(send_buf, 0, sizeof(Lint) * output_dim);
    Lint *recv_buf = new Lint[output_dim];
    memset(recv_buf, 0, sizeof(Lint) * output_dim);

    uint8_t *buffer = new uint8_t[bytes * output_dim];
    nodeNet->prg_getrandom(1, bytes, output_dim, buffer);
    // memset(buffer, 0, bytes*batch_size*m*s); //USED FOR TESTING
    // uint padded_height = height + 2;

    uint W_index, x_index;
    uint dp_index = 0;

    int crow = kernel * kernel * in_channels;
    int ccol = height * width;
    int srow = out_channels;

    for (uint b = 0; b < batch_size; b++) {
        // ordered this way to match TF matmult
        for (int i = 0; i < ccol; i++) {
            for (int j = 0; j < srow; j++) {
                for (int k = 0; k < crow; k++) {
                    W_index = j + k * srow;
                    // (out_dim/batch_size) * b used for multiple input images
                    x_index = k + i * crow + (in_dim / batch_size) * b;
                    v[dp_index] += x[0][x_index] * W[0][W_index] + x[0][x_index] * W[1][W_index] + x[1][x_index] * W[0][W_index];
                }

                memcpy(res[0] + (dp_index), buffer + (dp_index)*bytes, bytes);
                v[dp_index] = v[dp_index] - res[0][dp_index];
                dp_index++;
            }
        }
    }
    // sending v_a
    memcpy(send_buf, v, sizeof(Lint) * output_dim);
    nodeNet->SendAndGetDataFromPeer(send_buf, recv_buf, output_dim, ring_size);
    memcpy(res[1], recv_buf, sizeof(Lint) * output_dim);
    nodeNet->prg_getrandom(0, bytes, output_dim, buffer);

    for (uint i = 0; i < output_dim; i++) {
        res[1][i] = res[1][i] + res[0][i];
        memcpy(res[0] + (i), buffer + (i)*bytes, bytes);
        res[0][i] = res[0][i] + v[i];
    }

    delete[] send_buf;
    delete[] recv_buf;
    delete[] buffer;
    delete[] v;
}


void Rss_MatMultArray_batch_5pc(Lint **c, Lint **a, Lint **b, uint m, uint n, uint s, uint ring_size, uint batch_size, uint weight_flag_a, uint weight_flag_b, NodeNetwork *nodeNet) {
    // modified implementation that uses a 1d array representation of a matrix
    // a [m*n] = i*n+j (accessing the (i,j)th element)
    // [m][n] size matrix a;   [n][s] size matrix b
    // ---> [m][s] size matrix c;
    // For party 1, a[0,1]=a_2,3; b[0,1]=b_2,3;  c[0,1] = c_2,3;
    // For party 2, a[0,1]=a_3,1; b[0,1]=b_3,1;  c[0,1] = c_3,1;
    // For party 3, a[0,1]=a_1,2; b[0,1]=b_1,2;  c[0,1] = c_1,2;

    // if weight_flag == 0 --> the a matrix is the same throughout the computation
    uint total_size = batch_size * m * s;
    uint bytes = (ring_size + 7) >> 3;
    uint i, j, k, l; // used for loops
    // printf("bytes: %u\n", bytes);

    uint p_prime, T_index;
    uint numShares = nodeNet->getNumShares();
    uint numParties = nodeNet->getPeers();
    uint threshold = nodeNet->getThreshold();
    int pid = nodeNet->getID();
    uint8_t prg_ctrs[6] = {2, 3, 3, 2, 3, 2};

    Lint *v = new Lint[total_size];
    memset(v, 0, sizeof(Lint) * (total_size));

    Lint **recv_buf = new Lint *[threshold];
    for (i = 0; i < threshold; i++) {
        recv_buf[i] = new Lint[total_size];
        memset(recv_buf[i], 0, sizeof(Lint) * total_size);
    }

    uint8_t **buffer = new uint8_t *[numShares];
    for (i = 0; i < numShares; i++) {
        buffer[i] = new uint8_t[prg_ctrs[i] * bytes * total_size];
        nodeNet->prg_getrandom(i, bytes, prg_ctrs[i] * total_size, buffer[i]);
        // memset(buffer[i], 0, sizeof(uint8_t) * total_size[numShares] * bytes * size); // FOR

        // sanitizing destination (just in case)
        // memset(c[i], 0, sizeof(Lint) * total_size);

        // memset(buffer[i], 0, sizeof(uint8_t) * prg_ctrs[numShares] * bytes * size);
    }
    Lint z = 0;
    uint trackers[6] = {0, 0, 0, 0, 0, 0};

    Lint **b_transpose = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        b_transpose[i] = new Lint[batch_size * n * s];
        memset(b_transpose[i], 0, sizeof(Lint) * batch_size * n * s);
    }
    // transposing
    for (int ss = 0; ss < numShares; ss++) {
        for (l = 0; l < batch_size; l++) {
            for (i = 0; i < n; ++i) {
                for (j = 0; j < s; ++j) {
                    b_transpose[ss][weight_flag_b * l * n * s + j * n + i] = b[ss][weight_flag_b * l * n * s + i * s + j];
                }
            }
        }
    }
    // uint8_t *buffer = new uint8_t[bytes * total_size];

    // nodeNet->prg_getrandom(1, bytes, total_size, buffer);
    // memset(buffer, 0, bytes*batch_size*m*s); //USED FOR TESTING

    // a [m*n] = i*n+j (accessing the (i,j)th element)
    // [m][n] size matrix a;   [n][s] size matrix b
    // ---> [m][s] size matrix c;

    uint a_index_1, b_index_1, v_index;
    for (l = 0; l < batch_size; l++) {
        for (i = 0; i < m; i++) {
            for (j = 0; j < s; j++) {
                for (k = 0; k < n; k++) {
                    a_index_1 = weight_flag_a * l * m * n + i * n + k;
                    b_index_1 = weight_flag_b * l * n * s + j * n + k;
                    v_index = l * m * s + i * s + j;

                    v[v_index] += a[0][a_index_1] * (b_transpose[0][b_index_1] + b_transpose[1][b_index_1] + b_transpose[2][b_index_1] + b_transpose[3][b_index_1] + b_transpose[4][b_index_1] + b_transpose[5][b_index_1]) +
                                  a[1][a_index_1] * (b_transpose[0][b_index_1] + b_transpose[1][b_index_1] + b_transpose[2][b_index_1] + b_transpose[3][b_index_1] + b_transpose[4][b_index_1] + b_transpose[5][b_index_1]) +
                                  a[2][a_index_1] * (b_transpose[1][b_index_1] + b_transpose[3][b_index_1]) +
                                  a[3][a_index_1] * (b_transpose[0][b_index_1] + b_transpose[2][b_index_1]) +
                                  a[4][a_index_1] * (b_transpose[0][b_index_1] + b_transpose[1][b_index_1]) +
                                  a[5][a_index_1] * (b_transpose[0][b_index_1] + b_transpose[4][b_index_1]);
                }

                for (p_prime = 1; p_prime < numParties + 1; p_prime++) {
                    for (T_index = 0; T_index < numShares; T_index++) {
                        z = 0;

                        if ((p_prime != (pid)) and (!(p_prime_in_T(p_prime, nodeNet->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T(p_prime, nodeNet->T_map_mpc[T_index], numParties)))) {
                            memcpy(&z, buffer[T_index] + (trackers[T_index]) * bytes, bytes);
                            c[T_index][v_index] += z;
                            trackers[T_index] += 1;
                        } else if ((p_prime == pid) and (!(chi_p_prime_in_T(pid, nodeNet->T_map_mpc[T_index], numParties)))) {
                            memcpy(&z, buffer[T_index] + (trackers[T_index]) * bytes, bytes);
                            c[T_index][v_index] += z;
                            v[v_index] = v[v_index] - z;
                            trackers[T_index] += 1;
                        }
                    }
                }
            }
        }
    }

    nodeNet->SendAndGetDataFromPeer_Mult(v, recv_buf, total_size, ring_size);

    for (l = 0; l < batch_size; l++) {
        for (i = 0; i < m; i++) {
            for (j = 0; j < s; j++) {
                v_index = l * m * s + i * s + j;
                c[3][v_index] = c[3][v_index] + recv_buf[1][v_index];
                c[5][v_index] = c[5][v_index] + recv_buf[0][v_index];

                c[0][v_index] = c[0][v_index] + v[v_index];
            }
        }
    }

    for (i = 0; i < threshold; i++) {
        delete[] recv_buf[i];
    }

    for (i = 0; i < numShares; i++) {
        delete[] buffer[i];
        delete[] b_transpose[i];
    }

    // free
    delete[] b_transpose;
    delete[] v;
    delete[] buffer;
    delete[] recv_buf;
}

void Rss_dw_matrixmul_5pc(Lint **res, Lint **x, Lint **W, uint width, uint height, uint channels, uint filter, uint ring_size, uint batch_size, uint stride, NodeNetwork *nodeNet) {

    uint total_size = batch_size * channels * (width / stride) * (height / stride);
    // uint in_dim = batch_size * channels * (width) * (height);
    uint padded_height = height + 2;

    uint W_index, x_index;
    uint dp_index = 0;

    uint bytes = (ring_size + 7) >> 3;
    uint i; // j, k, l; // used for loops
    // printf("bytes: %u\n", bytes);

    uint p_prime, T_index;
    uint numShares = nodeNet->getNumShares();
    uint numParties = nodeNet->getPeers();
    uint threshold = nodeNet->getThreshold();
    int pid = nodeNet->getID();
    uint8_t prg_ctrs[6] = {2, 3, 3, 2, 3, 2};

    Lint *v = new Lint[total_size];
    memset(v, 0, sizeof(Lint) * (total_size));

    Lint **recv_buf = new Lint *[threshold];
    for (i = 0; i < threshold; i++) {
        recv_buf[i] = new Lint[total_size];
        memset(recv_buf[i], 0, sizeof(Lint) * total_size);
    }

    uint8_t **buffer = new uint8_t *[numShares];
    for (i = 0; i < numShares; i++) {
        buffer[i] = new uint8_t[prg_ctrs[i] * bytes * total_size];
        nodeNet->prg_getrandom(i, bytes, prg_ctrs[i] * total_size, buffer[i]);
        // memset(buffer[i], 0, sizeof(uint8_t) * total_size[numShares] * bytes * size); // FOR

        // sanitizing destination (just in case)
        // memset(c[i], 0, sizeof(Lint) * total_size);

        // memset(buffer[i], 0, sizeof(uint8_t) * prg_ctrs[numShares] * bytes * size);
    }
    Lint z = 0;
    uint trackers[6] = {0, 0, 0, 0, 0, 0};

    // uint8_t *buffer = new uint8_t[bytes * total_size];

    // nodeNet->prg_getrandom(1, bytes, total_size, buffer);
    // memset(buffer, 0, bytes*batch_size*m*s); //USED FOR TESTING

    // a [m*n] = i*n+j (accessing the (i,j)th element)
    // [m][n] size matrix a;   [n][s] size matrix b
    // ---> [m][s] size matrix c;

    for (uint b = 0; b < batch_size; b++) {
        for (uint i = (stride - 1); i < width; i += stride) {
            for (uint j = (stride - 1); j < height; j += stride) {
                for (uint c = 0; c < channels; c++) {
                    for (uint fi = 0; fi < filter; fi++) {
                        for (uint fj = 0; fj < filter; fj++) {

                            W_index = fi * filter * channels + fj * channels + c;

                            x_index = (i + fi) * (padded_height)*channels + (j + fj) * channels + c + (total_size / batch_size) * b; // (out_dim/batch_size) * b used for multiple input images

                            v[dp_index] += x[0][x_index] * (W[0][W_index] + W[1][W_index] + W[2][W_index] + W[3][W_index] + W[4][W_index] + W[5][W_index]) +

                                           x[1][x_index] * (W[0][W_index] + W[1][W_index] + W[2][W_index] + W[3][W_index] + W[4][W_index] + W[5][W_index]) +
                                           x[2][x_index] * (W[1][W_index] + W[3][W_index]) +
                                           x[3][x_index] * (W[0][W_index] + W[2][W_index]) +
                                           x[4][x_index] * (W[0][W_index] + W[1][W_index]) +
                                           x[5][x_index] * (W[0][W_index] + W[4][W_index]);
                        }
                    }

                    for (p_prime = 1; p_prime < numParties + 1; p_prime++) {
                        for (T_index = 0; T_index < numShares; T_index++) {
                            z = 0;

                            if ((p_prime != (pid)) and (!(p_prime_in_T(p_prime, nodeNet->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T(p_prime, nodeNet->T_map_mpc[T_index], numParties)))) {
                                memcpy(&z, buffer[T_index] + (trackers[T_index]) * bytes, bytes);
                                res[T_index][dp_index] += z;
                                trackers[T_index] += 1;
                            } else if ((p_prime == pid) and (!(chi_p_prime_in_T(pid, nodeNet->T_map_mpc[T_index], numParties)))) {
                                memcpy(&z, buffer[T_index] + (trackers[T_index]) * bytes, bytes);
                                res[T_index][dp_index] += z;
                                v[dp_index] = v[dp_index] - z;
                                trackers[T_index] += 1;
                            }
                        }
                    }
                    dp_index++;
                    // memcpy(res[0] + (dp_index), buffer + (dp_index)*bytes, bytes);
                    // v[dp_index] = v[dp_index] - res[0][dp_index];
                }
            }
        }
    }
    // nodeNet->SendAndGetDataFromPeer_Mult(v, recv_buf, output_dim, ring_size);

    nodeNet->SendAndGetDataFromPeer_Mult(v, recv_buf, total_size, ring_size);

    for (uint i = 0; i < total_size; i++) {
        res[3][i] = res[3][i] + recv_buf[1][i];
        res[5][i] = res[5][i] + recv_buf[0][i];
        res[0][i] = res[0][i] + v[i];
    }

    for (i = 0; i < threshold; i++) {
        delete[] recv_buf[i];
    }

    for (i = 0; i < numShares; i++) {
        delete[] buffer[i];
    }

    // free
    delete[] v;
    delete[] buffer;
    delete[] recv_buf;
}

void Rss_pw_matrixmul_5pc(Lint **res, Lint **x, Lint **W, uint width, uint height, uint in_channels, uint kernel, uint out_channels, uint ring_size, uint batch_size, NodeNetwork *nodeNet) {

    uint total_size = batch_size * out_channels * width * height;
    uint in_dim = batch_size * in_channels * width * height;
    uint bytes = (ring_size + 7) >> 3;

    uint W_index, x_index;
    uint dp_index = 0;
    int crow = kernel * kernel * in_channels;
    int ccol = height * width;
    int srow = out_channels;

    uint i; // j, k, l; // used for loops
    // printf("bytes: %u\n", bytes);

    uint p_prime, T_index;
    uint numShares = nodeNet->getNumShares();
    uint numParties = nodeNet->getPeers();
    uint threshold = nodeNet->getThreshold();
    int pid = nodeNet->getID();
    uint8_t prg_ctrs[6] = {2, 3, 3, 2, 3, 2};

    Lint *v = new Lint[total_size];
    memset(v, 0, sizeof(Lint) * (total_size));

    Lint **recv_buf = new Lint *[threshold];
    for (i = 0; i < threshold; i++) {
        recv_buf[i] = new Lint[total_size];
        memset(recv_buf[i], 0, sizeof(Lint) * total_size);
    }

    uint8_t **buffer = new uint8_t *[numShares];
    for (i = 0; i < numShares; i++) {
        buffer[i] = new uint8_t[prg_ctrs[i] * bytes * total_size];
        nodeNet->prg_getrandom(i, bytes, prg_ctrs[i] * total_size, buffer[i]);
        // memset(buffer[i], 0, sizeof(uint8_t) * total_size[numShares] * bytes * size); // FOR

        // sanitizing destination (just in case)
        // memset(c[i], 0, sizeof(Lint) * total_size);

        // memset(buffer[i], 0, sizeof(uint8_t) * prg_ctrs[numShares] * bytes * size);
    }
    Lint z = 0;
    uint trackers[6] = {0, 0, 0, 0, 0, 0};

    // uint8_t *buffer = new uint8_t[bytes * total_size];

    // nodeNet->prg_getrandom(1, bytes, total_size, buffer);
    // memset(buffer, 0, bytes*batch_size*m*s); //USED FOR TESTING

    // a [m*n] = i*n+j (accessing the (i,j)th element)
    // [m][n] size matrix a;   [n][s] size matrix b
    // ---> [m][s] size matrix c;
    for (uint b = 0; b < batch_size; b++) {
        // ordered this way to match TF matmult
        for (int i = 0; i < ccol; i++) {
            for (int j = 0; j < srow; j++) {
                // sLint dot_product = 0;
                for (int k = 0; k < crow; k++) {
                    W_index = j + k * srow;
                    // (out_dim/batch_size) * b used for multiple input images
                    x_index = k + i * crow + (in_dim / batch_size) * b;
                    v[dp_index] += x[0][x_index] * (W[0][W_index] + W[1][W_index] + W[2][W_index] + W[3][W_index] + W[4][W_index] + W[5][W_index]) +

                                   x[1][x_index] * (W[0][W_index] + W[1][W_index] + W[2][W_index] + W[3][W_index] + W[4][W_index] + W[5][W_index]) +
                                   x[2][x_index] * (W[1][W_index] + W[3][W_index]) +
                                   x[3][x_index] * (W[0][W_index] + W[2][W_index]) +
                                   x[4][x_index] * (W[0][W_index] + W[1][W_index]) +
                                   x[5][x_index] * (W[0][W_index] + W[4][W_index]);
                }
                for (p_prime = 1; p_prime < numParties + 1; p_prime++) {
                    for (T_index = 0; T_index < numShares; T_index++) {
                        z = 0;

                        if ((p_prime != (pid)) and (!(p_prime_in_T(p_prime, nodeNet->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T(p_prime, nodeNet->T_map_mpc[T_index], numParties)))) {
                            memcpy(&z, buffer[T_index] + (trackers[T_index]) * bytes, bytes);
                            res[T_index][dp_index] += z;
                            trackers[T_index] += 1;
                        } else if ((p_prime == pid) and (!(chi_p_prime_in_T(pid, nodeNet->T_map_mpc[T_index], numParties)))) {
                            memcpy(&z, buffer[T_index] + (trackers[T_index]) * bytes, bytes);
                            res[T_index][dp_index] += z;
                            v[dp_index] = v[dp_index] - z;
                            trackers[T_index] += 1;
                        }
                    }
                }
                dp_index++;
            }
        }
    }

    nodeNet->SendAndGetDataFromPeer_Mult(v, recv_buf, total_size, ring_size);

    for (uint i = 0; i < total_size; i++) {
        res[3][i] = res[3][i] + recv_buf[1][i];
        res[5][i] = res[5][i] + recv_buf[0][i];
        res[0][i] = res[0][i] + v[i];
    }

    for (i = 0; i < threshold; i++) {
        delete[] recv_buf[i];
    }

    for (i = 0; i < numShares; i++) {
        delete[] buffer[i];
    }

    // free
    delete[] v;
    delete[] buffer;
    delete[] recv_buf;
}


void Rss_MatMultArray_batch_7pc(Lint **c, Lint **a, Lint **b, uint m, uint n, uint s, uint ring_size, uint batch_size, uint weight_flag_a, uint weight_flag_b, NodeNetwork *nodeNet) {

    // struct timeval start;
    // struct timeval end;
    // unsigned long timer = 0;

    // if weight_flag == 0 --> the a matrix is the same throughout the computation
    uint total_size = batch_size * m * s;
    // printf("total_size = %u\n", total_size);
    // printf("m = %u\n", m);
    // printf("n = %u\n", n);
    // printf("s = %u\n", s);
    uint bytes = (ring_size + 7) >> 3;
    uint i, j, k, l; // used for loops
    // printf("bytes: %u\n", bytes);

    uint p_prime, T_index;
    uint numShares = nodeNet->getNumShares();
    uint numParties = nodeNet->getPeers();
    uint threshold = nodeNet->getThreshold();

    int pid = nodeNet->getID();
    uint8_t prg_ctrs[20] = {3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 4, 4, 4, 4, 4, 3, 4, 4, 3};

    Lint *v = new Lint[total_size];
    memset(v, 0, sizeof(Lint) * (total_size));

    Lint **recv_buf = new Lint *[threshold];
    for (i = 0; i < threshold; i++) {
        recv_buf[i] = new Lint[total_size];
        memset(recv_buf[i], 0, sizeof(Lint) * total_size);
    }

    uint8_t **buffer = new uint8_t *[numShares];
    for (i = 0; i < numShares; i++) {
        buffer[i] = new uint8_t[prg_ctrs[i] * bytes * total_size];
        nodeNet->prg_getrandom(i, bytes, prg_ctrs[i] * total_size, buffer[i]);
    }
    Lint z = 0;
    uint trackers[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    // printf("pre local\n");
    uint a_index_1, b_index_1, v_index;

    // gettimeofday(&start, NULL); // start timer here

    Lint **b_transpose = new Lint *[numShares];
    for (i = 0; i < numShares; i++) {
        b_transpose[i] = new Lint[batch_size * n * s];
        memset(b_transpose[i], 0, sizeof(Lint) * batch_size * n * s);
    }
    //transposing
    for (int ss = 0; ss < numShares; ss++) {
        for (i = 0; i < n; ++i) {
            for (j = 0; j < s; ++j) {
                b_transpose[ss][i + j * s] = b[ss][i * n + j];
            }
        }
    }

    Lint temp0 = 0, temp1 = 0;
    for (l = 0; l < batch_size; l++) {

        for (i = 0; i < m; i++) {
            for (j = 0; j < s; j++) {
                for (k = 0; k < n; k++) {
                    a_index_1 = weight_flag_a * l * m * n + i * m + k;
                    b_index_1 = weight_flag_b * l * n * s + j * s + k;
                    v_index = l * m * s + i * s + j;
                    temp0 = (b_transpose[0][b_index_1] + b_transpose[1][b_index_1] + b_transpose[2][b_index_1] + b_transpose[3][b_index_1] + b_transpose[4][b_index_1] + b_transpose[5][b_index_1] + b_transpose[6][b_index_1] + b_transpose[7][b_index_1] + b_transpose[8][b_index_1] + b_transpose[9][b_index_1] + b_transpose[10][b_index_1] + b_transpose[11][b_index_1] + b_transpose[12][b_index_1] + b_transpose[13][b_index_1] + b_transpose[14][b_index_1] + b_transpose[15][b_index_1] + b_transpose[16][b_index_1] + b_transpose[17][b_index_1] + b_transpose[18][b_index_1] + b_transpose[19][b_index_1]);
                    temp1 = (b_transpose[0][b_index_1] + b_transpose[1][b_index_1] + b_transpose[2][b_index_1] + b_transpose[3][b_index_1]);

                    v[v_index] = temp0 * (a[0][a_index_1] + a[1][a_index_1] + a[2][a_index_1] + a[3][a_index_1] + a[5][a_index_1]) +
                                 a[4][a_index_1] * (temp1 + b_transpose[10][b_index_1] + b_transpose[11][b_index_1] + b_transpose[12][b_index_1] + b_transpose[13][b_index_1] + b_transpose[15][b_index_1]) +
                                 a[6][a_index_1] * (b_transpose[2][b_index_1] + b_transpose[5][b_index_1] + b_transpose[7][b_index_1] + b_transpose[9][b_index_1] + b_transpose[11][b_index_1] + b_transpose[13][b_index_1]) +
                                 a[7][a_index_1] * (b_transpose[0][b_index_1] + b_transpose[4][b_index_1] + b_transpose[5][b_index_1] + b_transpose[6][b_index_1] + b_transpose[10][b_index_1] + b_transpose[11][b_index_1] + b_transpose[12][b_index_1]) +
                                 a[8][a_index_1] * (b_transpose[0][b_index_1] + b_transpose[4][b_index_1] + b_transpose[5][b_index_1] + b_transpose[10][b_index_1] + b_transpose[11][b_index_1] + b_transpose[16][b_index_1]) +
                                 a[9][a_index_1] * (b_transpose[1][b_index_1] + b_transpose[4][b_index_1] + b_transpose[7][b_index_1] + b_transpose[8][b_index_1] + b_transpose[10][b_index_1] + b_transpose[13][b_index_1]) +
                                 a[10][a_index_1] * (temp1 + b_transpose[4][b_index_1] + b_transpose[5][b_index_1] + b_transpose[9][b_index_1]) +
                                 a[11][a_index_1] * (b_transpose[0][b_index_1] + b_transpose[1][b_index_1] + b_transpose[4][b_index_1] + b_transpose[6][b_index_1] + b_transpose[7][b_index_1] + b_transpose[8][b_index_1]) +
                                 a[12][a_index_1] * (b_transpose[0][b_index_1] + b_transpose[1][b_index_1] + b_transpose[2][b_index_1] + b_transpose[4][b_index_1] + b_transpose[5][b_index_1] + b_transpose[7][b_index_1]) +
                                 a[13][a_index_1] * (b_transpose[0][b_index_1] + b_transpose[4][b_index_1] + b_transpose[5][b_index_1] + b_transpose[6][b_index_1]) +
                                 a[14][a_index_1] * (b_transpose[2][b_index_1] + b_transpose[4][b_index_1] + b_transpose[5][b_index_1]) +
                                 a[15][a_index_1] * (b_transpose[1][b_index_1] + b_transpose[4][b_index_1]) +
                                 a[16][a_index_1] * (temp1 + b_transpose[15][b_index_1]) +
                                 a[17][a_index_1] * (b_transpose[0][b_index_1] + b_transpose[1][b_index_1] + b_transpose[2][b_index_1]) +
                                 a[18][a_index_1] * (b_transpose[1][b_index_1] + b_transpose[8][b_index_1]) +
                                 a[19][a_index_1] * (b_transpose[0][b_index_1] + b_transpose[5][b_index_1] + b_transpose[6][b_index_1]);

                }
            }
        }
    }
    // gettimeofday(&end, NULL); // stop timer here

    // timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;

    // printf("[s1] [%.3lf ms]\n", (double)(timer * 0.001));

    // gettimeofday(&start, NULL); // start timer here

    for (i = 0; i < total_size; i++) {

        for (p_prime = 1; p_prime < numParties + 1; p_prime++) {
            for (T_index = 0; T_index < numShares; T_index++) {
                z = 0;
                // printf("before if else\n");
                //  if ((p_prime != (pid)) and (!(p_prime_in_T_7(p_prime, nodeNet->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T_7(p_prime, nodeNet->T_map_mpc[T_index], numParties)))) {
                if ((p_prime != (pid)) and (!(p_prime_in_T_7(p_prime, nodeNet->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T_7(p_prime, nodeNet->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (trackers[T_index]) * bytes, bytes);
                    // printf("T_index = %u, v_index = %u \n", T_index, v_index);
                    c[T_index][i] += z;
                    trackers[T_index] += 1;
                } else if (
                    (p_prime == pid) and (!(chi_p_prime_in_T_7(pid, nodeNet->T_map_mpc[T_index], numParties)))) {
                    // printf("elif\n");
                    memcpy(&z, buffer[T_index] + (trackers[T_index]) * bytes, bytes);
                    c[T_index][i] += z;
                    v[i] = v[i] - z;
                    trackers[T_index] += 1;
                }
            }
        }
    }

    // gettimeofday(&end, NULL); // stop timer here

    // timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;

    // printf("[s2] [%.3lf ms]\n", (double)(timer * 0.001));
    // // printf("pre send\n");
    // gettimeofday(&start, NULL); // start timer here

    nodeNet->SendAndGetDataFromPeer_Mult(v, recv_buf, total_size, ring_size);

    // gettimeofday(&end, NULL); // stop timer here

    // timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;

    // printf("[s3] [%.3lf ms]\n", (double)(timer * 0.001));
    // // printf("post send\n");
    // gettimeofday(&start, NULL); // start timer here

    for (l = 0; l < batch_size; l++) {
        for (i = 0; i < m; i++) {
            for (j = 0; j < s; j++) {
                v_index = l * m * s + i * s + j;
                // printf("v_index = %u\n", v_index);

                c[19][v_index] = c[19][v_index] + recv_buf[0][v_index];
                c[16][v_index] = c[16][v_index] + recv_buf[1][v_index];
                c[10][v_index] = c[10][v_index] + recv_buf[2][v_index];

                c[0][v_index] = c[0][v_index] + v[v_index];
            }
        }
    }
    // gettimeofday(&end, NULL); // stop timer here

    // timer = 1e6 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;

    // printf("[s4] [%.3lf ms]\n", (double)(timer * 0.001));

    for (i = 0; i < threshold; i++) {
        delete[] recv_buf[i];
    }
    for (i = 0; i < numShares; i++) {
        delete[] buffer[i];
        delete[] b_transpose[i];
    }

    // free
    delete[] v;
    delete[] b_transpose;
    delete[] buffer;
    delete[] recv_buf;
}
