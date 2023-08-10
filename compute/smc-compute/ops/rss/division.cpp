#include "division.h"

void Rss_Div(Lint **res, Lint **a, Lint **b, uint bitlength, uint size, uint ring_size, NodeNetwork *nodeNet) {
}
void Rss_AppRcr(Lint **res_w, Lint **a, Lint **b, uint bitlength, uint size, uint ring_size, NodeNetwork *nodeNet) {
}
void Rss_Norm(Lint **res_c, Lint **res_v, Lint **b, uint bitlength, uint size, uint ring_size, NodeNetwork *nodeNet) {

    ASSERT(ring_size > 2 * bitlength, "The ring size must be at least 2*bitlength");
    ASSERT(8 * sizeof(Lint) >= ring_size, "The ring size must be at most 8*sizeof(Lint)");

    int pid = nodeNet->getID();

    uint numShares = nodeNet->getNumShares();

    Lint **zero = new Lint *[numShares];
    Lint **sign = new Lint *[numShares];
    Lint **x = new Lint *[numShares];
    Lint **x_dec = new Lint *[numShares];
    Lint **z = new Lint *[numShares];
    Lint **prod = new Lint *[numShares];
    for (size_t i = 0; i < numShares; i++) {
        sign[i] = new Lint[size];
        x[i] = new Lint[size];
        x_dec[i] = new Lint[size];
        z[i] = new Lint[size];
        prod[i] = new Lint[size];
        memset(sign[i], 0, sizeof(Lint) * (size));
        zero[i] = new Lint[size];
        memset(zero[i], 0, sizeof(Lint) * (size));
    }

    Lint *ai = new Lint[numShares];
    memset(ai, 0, sizeof(Lint) * numShares);
    if (pid == 1) {
        ai[0] = 1; // party 1's share 1
    } else if (pid == 3) {
        ai[1] = 1; // party 3's share 2
    }

    Lint *check = new Lint[size];

    new_Rss_MSB(sign, b, size, bitlength, nodeNet);

    // Rss_Open(check, b, size, bitlength, nodeNet);
    // for (size_t i = 0; i < size; i++) {
    //     printf("b[%i] %lu \t", i, check[i]);
    //     print_binary(check[i], ring_size);
    // }

    // Rss_Open(check, sign, size, bitlength, nodeNet);
    // for (size_t i = 0; i < size; i++) {
    //     printf("msb[%i] %lu \t", i, check[i]);
    //     print_binary(check[i], ring_size);
    // }

    // for (size_t i = 0; i < numShares; i++) {
    //     for (size_t sh = 0; sh < numShares; sh++) {
    //         sign[sh][i] = (sign[sh][i] << Lint(bitlength - 1) ) ;//+  1*ai[sh] - sign[sh][i];
    //     }
    // }
    // Rss_Open(check, sign, size, bitlength, nodeNet);
    // for (size_t i = 0; i < size; i++) {
    //     printf("sign[%i] %lu \t", i, check[i]);
    //     print_binary(check[i], ring_size);
    // }

    Rss_Mult(prod, b, sign, size, bitlength, nodeNet);

    for (size_t i = 0; i < size; i++) {
        for (size_t sh = 0; sh < numShares; sh++) {
            x[sh][i] = b[sh][i] - Lint(2) * prod[sh][i];
        }
    }
    Rss_BitDec(x_dec, x, size, bitlength, nodeNet);

    Rss_PreOR(x_dec, size, bitlength, nodeNet);




    // Rss_Open(check, x, size, bitlength, nodeNet);
    // for (size_t i = 0; i < size; i++) {
    //     printf("x(xor)[%i] %lu  \t", i, check[i]);
    //     print_binary(check[i], ring_size);
    // }

    for (size_t i = 0; i < numShares; i++) {
        delete[] sign[i];
        delete[] x[i];
        delete[] x_dec[i];
        delete[] z[i];
        delete[] prod[i];
        delete[] zero[i];
    }
    delete[] sign;
    delete[] x;
    delete[] x_dec;
    delete[] z;
    delete[] prod;
    delete[] zero;
    delete[] ai;

    delete[] check;
}
