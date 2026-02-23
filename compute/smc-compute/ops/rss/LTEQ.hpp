/// cLT = ( a < b) and cEQ = ( a = b)
#pragma once
#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "BitLT.hpp"
#include "B2A.hpp"
#include "EQZ.hpp"
#include "EdaBit.hpp"
#include "RandBit.hpp"
// In Protocol 9 , to Return results I used [cLT],[cEQ]
template <typename T>
void doOperation_LTEQ(T **a, T **b, T **cLT,T **cEQ ,uint ring_size, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {
assertm((ring_size == ss->ring_size ) , "checking ring_size argument == ss->ring_size");
// line 1 & 2 (Generate ℓ random bits [r0 ]1 , . . . , [rℓ−1 ]1 in Z2 and one random bit [ b_prime ] over Z2k)
uint numShares = ss->getNumShares();
uint i; // used for loops

// only need to generate a single random bit per private value
/*b_prime[i]: Represents random bits for share i, which is later used to compute [b] over Z2k
edaBit_r[i] and edaBit_b_2[i]: Represent edaBits that are required for the LT and EQ computations.
sum[i]: Intermediate variable used for computing [a] - [b] + [r].
u_2[i]: Intermediate result for the comparison (BitLT or kOR).
rprime[i]: Stores the transformed random bits [r'].
c: Stores the opened result of [a] - [b] + [r].
e: Stores the opened result of [d] + 2^(ℓ−1)[b].
*/
T **b_prime = new T *[size];
T **sum = new T *[size];
T **u_2 = new T *[size];
T **edaBit_r = new T *[size];
T **edaBit_b_2 = new T *[size];
T **rprime = new T *[size];
T **r_prime = new T *[size];
T *c = new T[size];
T *e = new T[size];

for (i = 0; i < size; i++) {
    b_prime[i] = new T[numShares];
    edaBit_r[i] = new T[numShares];
    edaBit_b_2[i] = new T[numShares];
    sum[i] = new T[numShares];
    u_2[i] = new T[numShares];

    rprime[i] = new T[numShares];
    r_prime[i] = new T[numShares];
}

T **v = new T *[size];
T **u = new T *[size];
for (size_t i = 0; i < size; i++) {
    v[i] = new T[numShares];
    u[i] = new T[numShares];
}

 T *ai = new T[numShares];
memset(ai, 0, sizeof(T) * numShares);
ss->sparsify_public(ai, 1);

// stays the same
//This generates one random bit [b]
Rss_RandBit(b_prime, size, ring_size, nodeNet, ss);

// generating a full-sized edaBit
edaBit(edaBit_r, edaBit_b_2, ring_size, size, ring_size, nodeNet, ss);

 for (i = 0; i < size; i++) {
    for (size_t s = 0; s < numShares; s++) {
        rprime[i][s] = edaBit_r[i][s] - (GET_BIT(edaBit_b_2[i][s], T(ring_size - 1)) << T(ring_size - 1));
        // rprime[1][i] = edaBit_r[1][i] - (GET_BIT(edaBit_b_2[1][i], T(ring_size - 1)) << T(ring_size - 1));
        // combining w the next loop
        // combining w the previous loop
        sum[i][s] = ((a[i][s] - b[i][s])+ edaBit_r[i][s] );
        // sum[1][i] = (a[1][i] + edaBit_r[1][i]);
        // sum[0][i] = (a[0][i] + edaBit_r[0][i]) << T(1);
        // sum[1][i] = (a[1][i] + edaBit_r[1][i]) << T(1);
    }
}

// line 3
Open(c, sum, size, -1, nodeNet, ss);
// line 5
    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            r_prime[i][s] = (c[i] & (ai[s]*T(-1))) ^ edaBit_b_2[i][s]; // computing XOR in Z2
        }
    }
// line 4
 for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++)
            edaBit_b_2[i][s] = edaBit_b_2[i][s] & ss->SHIFT[ring_size - 1];

        // used for alternate solution
        // prevents us from using it directly later on
        // edaBit_b_2[1][i] = edaBit_b_2[1][i] & ss->SHIFT[ring_size - 1];

        // definitely needed
        c[i] = c[i] & ss->SHIFT[ring_size - 1];
        // c[i] = c[i] >> T(1);
    }



//line 6
// Rss_Open_Bitwise(r_2_open, edaBit_b_2, size, ring_size, nodeNet, ss);
// // this part is still correct
// however, the edaBit_b_2 shares do get modified
// which may not be desierable

Rss_BitLT(u_2, c, edaBit_b_2, size, ring_size, nodeNet, ss);
Rss_k_OR_L(v, r_prime, size, ring_size, nodeNet, ss);

    for (i = 0; i < size; i++) {
        for (size_t s = 0; s < numShares; s++) {
            v[i][s] = (T(1) & (ai[s]*T(-1)))
            ^ v[i][s]; // CHECK THIS (equivalent to computing 1 - result, but in Z2)
        }
    }

// Line 7: Convert binary shares to arithmetic shares Only one time call
// Replaced the two separate B2A calls with a single buffered call
// First, create a buffer to hold both u and cEQ values
T **buffer = new T *[2 * size];
T **resultBuffer = new T *[2 * size];
for (i = 0; i < 2 * size; i++) {
    buffer[i] = new T[numShares];
    resultBuffer[i] = new T[numShares];
}
for (i = 0; i < size; i++) {
    for (size_t s = 0; s < numShares; s++) {
        buffer[i][s] = u_2[i][s];         // First half for u
        buffer[i + size][s] = v[i][s];    // Second half for cEQ
    }
}

// Single B2A call for both values    //[u] & [cEQ]
Rss_B2A(resultBuffer, buffer, 2 * size, ring_size, nodeNet, ss);

// Extract results back to u and cEQ
for (i = 0; i < size; i++) {
    for (size_t s = 0; s < numShares; s++) {
        u[i][s] = resultBuffer[i][s];          // First half contains u
        cEQ[i][s] = resultBuffer[i + size][s]; // Second half contains cEQ
    }
}


// // Line 8: Compute [a'] = c' - [r'] + 2^(l-1)[u]
// // Line 9: Compute [d] = ([a] -[b])- [a']
//combining them in one line in 10
//line  10
for (i = 0; i < size; ++i)
{
    // cant do this because we modify edaBit_b_2 earlier
    // for (size_t s = 0; s < numShares; s++)
    for (size_t s = 0; s < numShares; s++)
        sum[i][s] = (a[i][s] - b[i][s])-c[i] * ai[s] + rprime[i][s] - (u_2[i][s] << T(ring_size - 1)) + (b_prime[i][s] << T(ring_size - 1));
    // sum[1][i] = a[1][i] - c[i] * a2 + rprime[1][i] - (u_2[1][i] << T(ring_size - 1)) + (b_prime[1][i] << T(ring_size - 1));
}
Open(e, sum, size, -1, nodeNet, ss);

// line 11
T e_bit;
for (i = 0; i < size; ++i) {
    e_bit = GET_BIT(e[i], T(ring_size - 1)); // getting the (k-1)th bit
    for (size_t s = 0; s < numShares; s++) {
        cLT[i][s] = e_bit * ai[s] + b_prime[i][s] - (e_bit * b_prime[i][s] << T(1));
    }
    // res[1][i] = e_bit * a2 + b[1][i] - (e_bit * b_prime[1][i] << T(1));
}
    // cleanup
//     delete[] ai;
//     delete[] c;
//     delete[] e;

//     for (i = 0; i < numShares; i++) {
//         delete[] edaBit_r[i];
//         delete[] edaBit_b_2[i];
//         delete[] b_prime;
//         delete[] sum[i];
//         delete[] u_2[i];
//         delete[] rprime[i];
//     }
//     delete[] edaBit_r;
//     delete[] edaBit_b_2;
//     delete[] sum;
//     delete[] u_2;
//     delete[] rprime;
// // Clean up the temporary buffers
// for (size_t s = 0; s < numShares; s++) {
//     delete[] buffer[s];
//     delete[] resultBuffer[s];
// }
// delete[] buffer;
// delete[] resultBuffer;

}























