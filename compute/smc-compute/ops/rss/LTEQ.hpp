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
void doOperation_LTEQ(T **a, T **b, T **cLT,T **cEQ ,int ring_size, uint size, NodeNetwork nodeNet, replicatedSecretShare<T> *ss) {

// line 1 & 2 (Generate ℓ random bits [r0 ]1 , . . . , [rℓ−1 ]1 in Z2 and one random bit [ b ] over Z2k)
uint numShares = ss->getNumShares();
uint i; // used for loops

// only need to generate a single random bit per private value
/*b[i]: Represents random bits for share i, which is later used to compute [b] over Z2k
edaBit_r[i] and edaBit_b_2[i]: Represent edaBits that are required for the LT and EQ computations.
sum[i]: Intermediate variable used for computing [a] - [b] + [r].
u_2[i]: Intermediate result for the comparison (BitLT or kOR).
rprime[i]: Stores the transformed random bits [r'].
c: Stores the opened result of [a] - [b] + [r].
e: Stores the opened result of [d] + 2^(ℓ−1)[b].
*/
T **b_prime = new T *[numShares];
T **sum = new T *[numShares];
T **u_2 = new T *[numShares];
T **edaBit_r = new T *[numShares];
T **edaBit_b_2 = new T *[numShares];
T **rprime = new T *[numShares];

T *c = new T[size];
T *e = new T[size];

for (i = 0; i < numShares; i++) {
    b_prime[i] = new T[size];
    edaBit_r[i] = new T[size];
    edaBit_b_2[i] = new T[size];
    sum[i] = new T[size];
    u_2[i] = new T[size];

    rprime[i] = new T[size];
}

T **v = new T *[numShares];
T **u = new T *[numShares];
for (size_t s = 0; s < numShares; s++) {
    v[s] = new T[size];
    u[s] = new T[size];
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
        rprime[s][i] = edaBit_r[s][i] - (GET_BIT(edaBit_b_2[s][i], T(ring_size - 1)) << T(ring_size - 1));
        // rprime[1][i] = edaBit_r[1][i] - (GET_BIT(edaBit_b_2[1][i], T(ring_size - 1)) << T(ring_size - 1));
        // combining w the next loop
        // combining w the previous loop
        sum[s][i] = (a[s][i] + edaBit_r[s][i] - b[s][i]);
        // sum[1][i] = (a[1][i] + edaBit_r[1][i]);
        // sum[0][i] = (a[0][i] + edaBit_r[0][i]) << T(1);
        // sum[1][i] = (a[1][i] + edaBit_r[1][i]) << T(1);
    }
}

// line 3
Open(c, sum, size, -1, nodeNet, ss);


// line 4
for (i = 0; i < size; i++) {
    // definitely needed
    c[i] = c[i] & ss->SHIFT[ring_size - 1];
    // c[i] = c[i] >> T(1);
}

// line 5
for (size_t s = 0; s < numShares; s++) {
    for (i = 0; i < size; i++) {
        rprime[s][i] = (c[i] & ai[s]) ^ edaBit_b_2[s][i]; // computing XOR in Z2
    }
}

// line 6
// Rss_Open_Bitwise(r_2_open, edaBit_b_2, size, ring_size, nodeNet, ss);
// // this part is still correct
// however, the edaBit_b_2 shares do get modified
// which may not be desierable
Rss_BitLT(u_2, c, edaBit_b_2, size, ring_size, nodeNet, ss);
Rss_k_OR_L(v, rprime, size, ring_size, nodeNet, ss);

    for (size_t s = 0; s < numShares; s++) {
        for (i = 0; i < size; i++) {
            v[s][i] = (T(1) & ai[s]) ^ v[s][i]; // CHECK THIS (equivalent to computing 1 - result, but in Z2)
        }
    }



// Line 7: Convert binary shares to arithmetic shares Only one time call
// Replaced the two separate B2A calls with a single buffered call
// First, create a buffer to hold both u and cEQ values
T **buffer = new T *[numShares];
T **resultBuffer = new T *[numShares];
for (size_t s = 0; s < numShares; s++) {
    buffer[s] = new T[2 * size];
    resultBuffer[s] = new T[2 * size];
    for (i = 0; i < size; i++) {
        buffer[s][i] = u_2[s][i];         // First half for u
        buffer[s][i + size] = v[s][i];    // Second half for cEQ
    }
}

// Single B2A call for both values    //[u] & [cEQ]
Rss_B2A(resultBuffer, buffer, 2 * size, ring_size, nodeNet, ss);

// Extract results back to u and cEQ
for (size_t s = 0; s < numShares; s++) {
    for (int i = 0; i < size; i++) {
        u[s][i] = resultBuffer[s][i];          // First half contains u
        cEQ[s][i] = resultBuffer[s][i + size]; // Second half contains cEQ
    }
}


// Line 8: Compute [a'] = c' - [r'] + 2^(l-1)[u]
T **a_prime = new T *[numShares];
for (size_t s = 0; s < numShares; s++) {
    a_prime[s] = new T[size];
    for (int i = 0; i < size; i++) {
        a_prime[s][i] = c[i] - rprime[s][i] + (u[s][i] << (ring_size - 1));
    }
}

// Line 9: Compute [d] = [a] - [a']
T **d = new T *[numShares];
for (size_t s = 0; s < numShares; s++) {
    d[s] = new T[size];
    for (int i = 0; i < size; i++) {
        d[s][i] = (a[s][i] - b[s][i]) - a_prime[s][i];
    }
} 
// line  10


for (i = 0; i < size; ++i) {
    // cant do this because we modify edaBit_b_2 earlier
    // for (size_t s = 0; s < numShares; s++)
    for (size_t s = 0; s < numShares; s++)
        sum[s][i] = a[s][i] - c[i] * ai[s] + rprime[s][i] - (u_2[s][i] << T(ring_size - 1)) + (b_prime[s][i] << T(ring_size - 1));
    // sum[1][i] = a[1][i] - c[i] * a2 + rprime[1][i] - (u_2[1][i] << T(ring_size - 1)) + (b[1][i] << T(ring_size - 1));
}   
Open(e, sum, size, -1, nodeNet, ss);

// line 11 
T e_bit;
for (i = 0; i < size; ++i) {
    e_bit = GET_BIT(e[i], T(ring_size - 1)); // getting the (k-1)th bit
    for (size_t s = 0; s < numShares; s++) {
        cLT[s][i] = e_bit * ai[s] + b_prime[s][i] - (e_bit * b_prime[s][i] << T(1));
    }
    // res[1][i] = e_bit * a2 + b[1][i] - (e_bit * b[1][i] << T(1));
}
    // cleanup
    delete[] ai;
   delete[] c;
    delete[] e;

    for (i = 0; i < numShares; i++) {
        delete[] edaBit_r[i];
        delete[] edaBit_b_2[i];
        delete[] b_prime;
        delete[] sum[i];
        delete[] u_2[i];
        delete[] rprime[i];
    }
    delete[] edaBit_r;
    delete[] edaBit_b_2;
    delete[] sum;
    delete[] u_2;
    delete[] rprime;
// Clean up the temporary buffers
for (size_t s = 0; s < numShares; s++) {
    delete[] buffer[s];
    delete[] resultBuffer[s];
}
delete[] buffer;
delete[] resultBuffer;

}























