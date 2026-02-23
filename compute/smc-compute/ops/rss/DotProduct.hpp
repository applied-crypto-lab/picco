/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2024 PICCO Team
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

#ifndef _DOTPRODUCT_HPP_
#define _DOTPRODUCT_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"

// Scalar version (single dot product)
// Input format: a, b are [array_size][numShares] - each element has numShares shares
// Output format: result is T[numShares] - a single element with numShares shares
template <typename T>
void doOperation_DotProduct(T **a, T **b, T *result, int array_size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();

    // Create result_shares in interface format [1][numShares]
    T **result_shares = new T *[1];
    result_shares[0] = new T[numShares];
    memset(result_shares[0], 0, sizeof(T) * numShares);

    DotProduct(result_shares, a, b, array_size, 1, net, ss);

    // Extract result from shares
    for (uint s = 0; s < numShares; s++) {
        result[s] = result_shares[0][s];
    }
    delete[] result_shares[0];
    delete[] result_shares;
}

// Batch version (multiple dot products)
// Input format: a, b are [batch_size][array_size][numShares] - batch_size dot products, each of array_size elements
// Output format: result is [batch_size][numShares] - batch_size results
// NOTE: This function expects a specific 3D format that may not match all callers
template <typename T>
void doOperation_DotProduct(T ***a, T ***b, T **result, int batch_size, int array_size, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // For batch operations, we need to flatten the 3D arrays into 2D
    // DotProduct expects [array_size * batch_size][numShares] format
    uint numShares = ss->getNumShares();

    // Create flattened arrays in interface format [array_size * batch_size][numShares]
    T **a_flat = new T *[array_size * batch_size];
    T **b_flat = new T *[array_size * batch_size];

    for (int i = 0; i < batch_size; i++) {
        for (int j = 0; j < array_size; j++) {
            int idx = i * array_size + j;
            a_flat[idx] = new T[numShares];
            b_flat[idx] = new T[numShares];
            for (uint s = 0; s < numShares; s++) {
                a_flat[idx][s] = a[i][j][s];
                b_flat[idx][s] = b[i][j][s];
            }
        }
    }

    DotProduct(result, a_flat, b_flat, array_size, batch_size, net, ss);

    for (int idx = 0; idx < array_size * batch_size; idx++) {
        delete[] a_flat[idx];
        delete[] b_flat[idx];
    }
    delete[] a_flat;
    delete[] b_flat;
}

// Interface format: [size][numShares] where array[i][s] is share s of element i
// a : [array_size * size][numShares] (blocks of array_size elements)
// b : [array_size * size][numShares] (blocks of array_size elements)
// c : [size][numShares]
template <typename T>
void Rss_DotProd_3pc(T **c, T **a, T **b, uint array_size, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
    uint bytes = (ring_size + 7) >> 3;
    uint i;

    T *v = new T[size];
    memset(v, 0, sizeof(T) * (size));

    // Temporary arrays for c shares during computation
    T *c_0 = new T[size];
    T *c_1 = new T[size];
    memset(c_0, 0, sizeof(T) * size);
    memset(c_1, 0, sizeof(T) * size);

    uint8_t *buffer = new uint8_t[bytes * size];
    ss->prg_getrandom(1, bytes, size, buffer);

    uint idx;
    for (i = 0; i < size; i++) {
        memcpy(c_0 + i, buffer + i * bytes, bytes);
        for (size_t j = 0; j < array_size; j++) {
            idx = i * array_size + j;
            // Changed from a[s][idx] to a[idx][s]
            v[i] += a[idx][0] * b[idx][0] + a[idx][0] * b[idx][1] + a[idx][1] * b[idx][0];
        }
        v[i] -= c_0[i];
    }

    nodeNet.SendAndGetDataFromPeer(v, c_1, size, ring_size, ss->general_map);
    ss->prg_getrandom(0, bytes, size, buffer);

    for (i = 0; i < size; i++) {
        c_1[i] = c_1[i] + c_0[i];
        memcpy(c_0 + i, buffer + i * bytes, bytes);
        c_0[i] = c_0[i] + v[i];
    }

    // Copy results to output in [size][numShares] format
    for (i = 0; i < size; i++) {
        c[i][0] = c_0[i];
        c[i][1] = c_1[i];
    }

    // free
    delete[] v;
    delete[] buffer;
    delete[] c_0;
    delete[] c_1;
}

// Interface format: [size][numShares] where array[i][s] is share s of element i
// a : [array_size * size][numShares] (blocks of array_size elements)
// b : [array_size * size][numShares] (blocks of array_size elements)
// c : [size][numShares]
template <typename T>
void Rss_DotProd_5pc(T **c, T **a, T **b, uint array_size, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint bytes = (ring_size + 7) >> 3;
    uint i;
    uint numShares = ss->getNumShares();
    uint numParties = ss->getPeers();
    uint threshold = ss->getThreshold();
    int pid = ss->getID();

    T *v = new T[size];
    memset(v, 0, sizeof(T) * (size));

    uint8_t prg_ctrs[6] = {2, 3, 3, 2, 3, 2};

    T **recv_buf = new T *[threshold];
    for (i = 0; i < threshold; i++) {
        recv_buf[i] = new T[size];
        memset(recv_buf[i], 0, sizeof(T) * size);
    }

    uint8_t **buffer = new uint8_t *[numShares];
    for (i = 0; i < numShares; i++) {
        buffer[i] = new uint8_t[prg_ctrs[i] * bytes * size];
        ss->prg_getrandom(i, bytes, prg_ctrs[i] * size, buffer[i]);
    }

    // Sanitize destination using interface format [size][numShares]
    for (i = 0; i < size; i++) {
        memset(c[i], 0, sizeof(T) * numShares);
    }

    T z = T(0);
    uint tracker;
    uint idx;
    for (i = 0; i < size; i++) {
        // Compute dot product using interface format: a[idx][s], b[idx][s]
        for (size_t j = 0; j < array_size; j++) {
            idx = i * array_size + j;
            v[i] += a[idx][0] * (b[idx][0] + b[idx][1] + b[idx][2] + b[idx][3] + b[idx][4] + b[idx][5]) +
                   a[idx][1] * (b[idx][0] + b[idx][1] + b[idx][2] + b[idx][3] + b[idx][4] + b[idx][5]) +
                   a[idx][2] * (b[idx][1] + b[idx][3]) +
                   a[idx][3] * (b[idx][0] + b[idx][2]) +
                   a[idx][4] * (b[idx][0] + b[idx][1]) +
                   a[idx][5] * (b[idx][0] + b[idx][4]);
        }

        for (uint p_prime = 1; p_prime < numParties + 1; p_prime++) {
            for (uint T_index = 0; T_index < numShares; T_index++) {
                tracker = 0;
                if (((int)p_prime != (pid)) and (!(p_prime_in_T(p_prime, ss->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T(p_prime, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[i][T_index] += z;  // Fixed: [element][share]
                    tracker += 1;
                } else if (
                    ((int)p_prime == pid) and (!(chi_p_prime_in_T(pid, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[i][T_index] += z;  // Fixed: [element][share]
                    v[i] -= z;
                    tracker += 1;
                }
            }
        }
    }

    // communication
    nodeNet.SendAndGetDataFromPeer(v, recv_buf, size, ring_size, ss->general_map);

    for (i = 0; i < size; i++) {
        c[i][3] = c[i][3] + recv_buf[1][i];  // Fixed: [element][share]
        c[i][5] = c[i][5] + recv_buf[0][i];  // Fixed: [element][share]
        c[i][0] = c[i][0] + v[i];            // Fixed: [element][share]
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

// Interface format: [size][numShares] where array[i][s] is share s of element i
// a : [array_size * size][numShares] (blocks of array_size elements)
// b : [array_size * size][numShares] (blocks of array_size elements)
// c : [size][numShares]
template <typename T>
void Rss_DotProd_7pc(T **c, T **a, T **b, uint array_size, uint size, uint ring_size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

    uint bytes = (ring_size + 7) >> 3;
    uint i;
    uint numShares = ss->getNumShares();
    uint numParties = ss->getPeers();
    uint threshold = ss->getThreshold();
    int pid = ss->getID();

    T *v = new T[size];
    memset(v, 0, sizeof(T) * (size));

    uint8_t prg_ctrs[20] = {3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 4, 4, 4, 4, 4, 3, 4, 4, 3};

    T **recv_buf = new T *[threshold];
    for (i = 0; i < threshold; i++) {
        recv_buf[i] = new T[size];
        memset(recv_buf[i], 0, sizeof(T) * size);
    }

    uint8_t **buffer = new uint8_t *[numShares];
    for (i = 0; i < numShares; i++) {
        buffer[i] = new uint8_t[prg_ctrs[i] * bytes * size];
        ss->prg_getrandom(i, bytes, prg_ctrs[i] * size, buffer[i]);
    }

    // Sanitize destination using interface format [size][numShares]
    for (i = 0; i < size; i++) {
        memset(c[i], 0, sizeof(T) * numShares);
    }

    T z = T(0);
    uint tracker;
    uint idx;
    for (i = 0; i < size; i++) {
        // Compute dot product using interface format: a[idx][s], b[idx][s]
        for (size_t j = 0; j < array_size; j++) {
            idx = i * array_size + j;
            v[i] +=
                a[idx][0] * (b[idx][0] + b[idx][1] + b[idx][2] + b[idx][3] + b[idx][4] + b[idx][5] + b[idx][6] + b[idx][7] + b[idx][8] + b[idx][9] + b[idx][10] + b[idx][11] + b[idx][12] + b[idx][13] + b[idx][14] + b[idx][15] + b[idx][16] + b[idx][17] + b[idx][18] + b[idx][19]) +
                a[idx][1] * (b[idx][0] + b[idx][1] + b[idx][2] + b[idx][3] + b[idx][4] + b[idx][5] + b[idx][6] + b[idx][7] + b[idx][8] + b[idx][9] + b[idx][10] + b[idx][11] + b[idx][12] + b[idx][13] + b[idx][14] + b[idx][15] + b[idx][16] + b[idx][17] + b[idx][18] + b[idx][19]) +
                a[idx][2] * (b[idx][0] + b[idx][1] + b[idx][2] + b[idx][3] + b[idx][4] + b[idx][5] + b[idx][6] + b[idx][7] + b[idx][8] + b[idx][9] + b[idx][10] + b[idx][11] + b[idx][12] + b[idx][13] + b[idx][14] + b[idx][15] + b[idx][16] + b[idx][17] + b[idx][18] + b[idx][19]) +
                a[idx][3] * (b[idx][0] + b[idx][1] + b[idx][2] + b[idx][3] + b[idx][4] + b[idx][5] + b[idx][6] + b[idx][7] + b[idx][8] + b[idx][9] + b[idx][10] + b[idx][11] + b[idx][12] + b[idx][13] + b[idx][14] + b[idx][15] + b[idx][16] + b[idx][17] + b[idx][18] + b[idx][19]) +
                a[idx][4] * (b[idx][0] + b[idx][1] + b[idx][2] + b[idx][3] + b[idx][10] + b[idx][11] + b[idx][12] + b[idx][13] + b[idx][15]) +
                a[idx][5] * (b[idx][0] + b[idx][1] + b[idx][2] + b[idx][3] + b[idx][4] + b[idx][5] + b[idx][6] + b[idx][7] + b[idx][8] + b[idx][9] + b[idx][10] + b[idx][11] + b[idx][12] + b[idx][13] + b[idx][14] + b[idx][15] + b[idx][16] + b[idx][17] + b[idx][18] + b[idx][19]) +
                a[idx][6] * (b[idx][2] + b[idx][5] + b[idx][7] + b[idx][9] + b[idx][11] + b[idx][13]) +
                a[idx][7] * (b[idx][0] + b[idx][4] + b[idx][5] + b[idx][6] + b[idx][10] + b[idx][11] + b[idx][12]) +
                a[idx][8] * (b[idx][0] + b[idx][4] + b[idx][5] + b[idx][10] + b[idx][11] + b[idx][16]) +
                a[idx][9] * (b[idx][1] + b[idx][4] + b[idx][7] + b[idx][8] + b[idx][10] + b[idx][13]) +
                a[idx][10] * (b[idx][0] + b[idx][1] + b[idx][2] + b[idx][3] + b[idx][4] + b[idx][5] + b[idx][9]) +
                a[idx][11] * (b[idx][0] + b[idx][1] + b[idx][4] + b[idx][6] + b[idx][7] + b[idx][8]) +
                a[idx][12] * (b[idx][0] + b[idx][1] + b[idx][2] + b[idx][4] + b[idx][5] + b[idx][7]) +
                a[idx][13] * (b[idx][0] + b[idx][4] + b[idx][5] + b[idx][6]) +
                a[idx][14] * (b[idx][2] + b[idx][4] + b[idx][5]) +
                a[idx][15] * (b[idx][1] + b[idx][4]) +
                a[idx][16] * (b[idx][0] + b[idx][1] + b[idx][2] + b[idx][3] + b[idx][15]) +
                a[idx][17] * (b[idx][0] + b[idx][1] + b[idx][2]) +
                a[idx][18] * (b[idx][1] + b[idx][8]) +
                a[idx][19] * (b[idx][0] + b[idx][5] + b[idx][6]);
        }

        for (uint p_prime = 1; p_prime < numParties + 1; p_prime++) {
            for (uint T_index = 0; T_index < numShares; T_index++) {
                tracker = 0;
                if (((int)p_prime != (pid)) and (!(p_prime_in_T_7(p_prime, ss->T_map_mpc[T_index]))) and (!(chi_p_prime_in_T_7(p_prime, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[i][T_index] += z;  // Fixed: [element][share]
                    tracker += 1;
                } else if (
                    ((int)p_prime == pid) and (!(chi_p_prime_in_T_7(pid, ss->T_map_mpc[T_index], numParties)))) {
                    memcpy(&z, buffer[T_index] + (i * prg_ctrs[T_index] + tracker) * bytes, bytes);
                    c[i][T_index] += z;  // Fixed: [element][share]
                    v[i] -= z;
                    tracker += 1;
                }
            }
        }
    }

    // communication
    nodeNet.SendAndGetDataFromPeer(v, recv_buf, size, ring_size, ss->general_map);

    for (i = 0; i < size; i++) {
        c[i][19] = c[i][19] + recv_buf[0][i];  // Fixed: [element][share]
        c[i][16] = c[i][16] + recv_buf[1][i];  // Fixed: [element][share]
        c[i][10] = c[i][10] + recv_buf[2][i];  // Fixed: [element][share]
        c[i][0] = c[i][0] + v[i];              // Fixed: [element][share]
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

// non-threaded versions (since RSS is single threaded )
template <typename T>
void DotProduct(T **C, T **A, T **B, uint array_size, uint size, NodeNetwork net, replicatedSecretShare<T> *ss) {
    // from here, we defer to the 3-, 5-, or 7-party implementations
    try {
        int peers = ss->getPeers();
        switch (peers) {
        case 3:
            Rss_DotProd_3pc(C, A, B, array_size, size, ss->ring_size, net, ss);
            break;
        case 5:
            Rss_DotProd_5pc(C, A, B, array_size, size, ss->ring_size, net, ss);
            break;
        case 7:
            Rss_DotProd_7pc(C, A, B, array_size, size, ss->ring_size, net, ss);
            break;
        default:
            throw std::runtime_error("invalid number of parties");
        }
    } catch (const std::runtime_error &ex) {
        std::string error(ex.what());
        throw std::runtime_error("[Mult] " + error);
    }
}

#endif // _DOTPRODUCT_HPP_
