// // this file is to be used for development testing of RSS operations. If you
// // have created a new file in the ops/rss directory, you will need to add the
// // new files name to the rss/RSSheaders.hpp file. Otherwise, the code will not
// // successfully compile.

// #include "RSS_types.hpp"
// #include "SMC_Utils.h"

// void convertFloat(float value, int K, int L, mpz_t **elements) {
//     unsigned int *newptr = (unsigned int *)&value;
//     int s = *newptr >> 31;
//     int e = *newptr & 0x7f800000;
//     e >>= 23;
//     int m = 0;
//     m = *newptr & 0x007fffff;

//     int z;
//     long p, k;
//     mpz_t significant, one, two, tmp, tmpm;
//     mpz_init(significant);
//     mpz_init(tmp);
//     mpz_init_set_ui(one, 1);
//     mpz_init_set_ui(two, 2);
//     mpz_init(tmpm);

//     if (e == 0 && m == 0) {
//         s = 0;
//         z = 1;
//         // v = 0;
//         mpz_set_ui(significant, 0);
//         p = 0;
//     } else {
//         z = 0;
//         if (L < 8) {
//             k = (1 << L) - 1;
//             /*check for overflow*/
//             if (e - 127 - K + 1 > k) {
//                 p = k;
//                 mpz_mul_2exp(significant, one, K);
//                 mpz_sub_ui(significant, significant, 1);
//                 // v = (1 << K) - 1;
//                 /*check for underflow*/
//             } else if (e - 127 - K + 1 < -k) {
//                 p = -k;
//                 mpz_set_ui(significant, 1);
//                 // v = 1;
//             } else {
//                 p = e - 127 - K + 1;
//                 m = m + (1 << 23);
//                 mpz_set_si(tmpm, m);
//                 if (K < 24) {
//                     mpz_pow_ui(tmp, two, 24 - K);
//                     mpz_div(significant, tmpm, tmp);
//                     // v = (m >> (24 - K));
//                 } else {
//                     mpz_mul_2exp(significant, tmpm, K - 24);
//                     // v = m << (K - 24);
//                 }
//             }
//         } else {
//             p = e - 127 - K + 1;
//             m = m + (1 << 23);
//             mpz_set_si(tmpm, m);
//             if (K < 24) {
//                 mpz_pow_ui(tmp, two, 24 - K);
//                 mpz_div(significant, tmpm, tmp);
//                 // v = (m >> (24 - K));
//             } else {
//                 mpz_set(significant, tmpm);
//                 mpz_mul_2exp(significant, significant, K - 24);
//                 // v = m;
//                 // v = v << (K - 24);
//             }
//         }
//     }

//     // assignments;
//     mpz_set((*elements)[0], significant);
//     mpz_set_si((*elements)[1], p);
//     mpz_set_si((*elements)[2], z);
//     mpz_set_si((*elements)[3], s);

//     // clear the memory
//     mpz_clear(one);
//     mpz_clear(two);
//     mpz_clear(tmp);
//     mpz_clear(tmpm);
//     mpz_clear(significant);
// }

// /**
//  * This fucntion converts floating-point number to a set of integer components based on
//  * the specified parameters K and L. The function does some bit manipulation on the floating-point
//  * representation of the input value to extract the sign bit (s), exponent (e), and mantissa (significand).
//  *
//  * In particular, each floating point number is represented as a 4-tuple (v, p, s, z) where v is
//  * an l-bit significand, p is a k-bit exponent, and s and z are sign and
//  * zero bits, respectively (2013 CCS paper).
//  * 
//  * K is a parameter that represents the number of bits used for the significand in the target representation.
//  * L is a parameter used to control the range of the exponent in the target representation.
//  *
//  * The integers that will be stored in elements are as follows:
//  * 1. Significand part (significand)
//  * 2. Exponent (p)
//  * 3. Flag indicating zero (z)
//  * 4. Sign (s)
//  */
// void convertFloat(float value, int K, int L, long long **elements) {
//     unsigned int *newptr = (unsigned int *)&value;
//     int s = *newptr >> 31; // Extract the sign bit
//     int e = *newptr & 0x7f800000; // Extract the exponent
//     e >>= 23;
//     int m = 0;
//     m = *newptr & 0x007fffff; // Extract the significand (mantissa)

//     int z;
//     long v, p, k;
//     long long significand = 0, one = 1, two = 2, tmp = 0, tmpm = 0;

//     if (e == 0 && m == 0) {
//         s = 0;
//         z = 1;
//         significand = 0;
//         p = 0;
//     } else {
//         z = 0;
//         if (L < 8) {
//             k = (1 << L) - 1; // Raise two to the power of L using shifting and subtract 1, then store it to k
//             if (e - 127 - K + 1 > k) {
//                 p = k;
//                 significand = one << K;        // Raise one to the power of K and store it to significand
//                 significand = significand - 1; // Sub 1
//             } else if (e - 127 - K + 1 < -k) {
//                 p = -k;
//                 significand = 1; // Set the value of significand to 1
//             } else {
//                 p = e - 127 - K + 1;
//                 m = m + (1 << 23);
//                 tmpm = m; // Set the value of tmpm to m
//                 if (K < 24) {
//                     try {
//                         tmp = pow(two, (24 - K)); // Raise two to the power of (24 - K) using shifting and store it to tmp
//                     } catch(const std::exception& e) {
//                         throw std::runtime_error("An exception occurred during pow operation: " + std::string(e.what()));
//                     }
//                     if (tmp == 0) // Division by zero check
//                         throw std::runtime_error("Division by zero: overflow in significand calculation");
//                     significand = tmpm / tmp; // Perform division of tmpm to tmp and store it to significand
//                 } else {
//                     significand = tmpm << (K - 24); // Raise tmpm to the power of (K - 24) and store it to significand
//                 }
//             }
//         } else {
//             p = e - 127 - K + 1;
//             m = m + (1 << 23);
//             tmpm = m; // Set the value of tmpm to m
//             if (K < 24) {
//                 try {
//                     tmp = pow(two, (24 - K)); // Raise two to the power of (24 - K) using shifting and store it to tmp
//                 } catch(const std::exception& e) {
//                     throw std::runtime_error("An exception occurred during pow operation: " + std::string(e.what()));
//                 }
//                 if (tmp == 0) // Division by zero check
//                     throw std::runtime_error("Division by zero: overflow in significand calculation");
//                 significand = tmpm / tmp; // Perform division of tmpm to tmp and store it to significand
//             } else {
//                 significand = tmpm;                    // Set significand to tmpm
//                 significand = significand << (K - 24); // Raise significand to the power of (K - 24) and store it to significand
//             }
//         }
//     }

//     // printf("sig  %lli\n", significand);
//     // printf("p    %li\n", p);
//     // printf("z    %i\n", z);
//     // printf("sgn  %i\n", s);
//     // Set the significand, p, z, and s value directly to the long long array of elements.
//     (*elements)[0] = significand;
//     (*elements)[1] = p;
//     (*elements)[2] = z;
//     (*elements)[3] = s;

// }

// void SMC_Utils::smc_test_rss(int threadID, int batch_size) {
//     // "batch_size"  corresponds to how many operations are
//     // we performing in a single protocol invocation.

//     uint numShares = ss->getNumShares();           // the number of shares each party maintains
//     uint totalNumShares = ss->getTotalNumShares(); // the total number of shares a secret is split into
//     uint ring_size = ss->ring_size;                // the ring size of the computation ("k" in the literature)
//     printf("ring_size : %u\n", ring_size);
//     printf("size : %u\n", batch_size);
//     printf("----\n\n");

//     // this is the setup process for producing secret shares. You can ignore
//     // the key initialization, as it's not particularly relevant
//     __m128i *key_prg;
//     uint8_t key_raw[] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
//     key_prg = offline_prg_keyschedule(key_raw);
//     uint8_t k1[] = {0x31, 0x43, 0xf6, 0xa8, 0x88, 0x5a, 0x30, 0x8d, 0x31, 0x31, 0x98, 0xa2, 0xe0, 0x37, 0x07, 0x34};
//     uint8_t k2[] = {0xa1, 0x34, 0x6f, 0x67, 0x10, 0x1b, 0x13, 0xa3, 0x56, 0x45, 0x90, 0xb2, 0x13, 0xe3, 0x23, 0x24};

//     // this defines the ordering in which shares of a secret are stored by each
//     // party this is critical for consistency across the parties for certain
//     // operations Consider the following example for 3 parties: a secret x is
//     // split into shares x_1, x_2, x_3, such that x = x_1 + x_2 + x_3. The
//     // parties store their shares in the following orders:
//     //
//     // party_1: [x_2, x_3] (share_mapping {1,2})
//     // party_2: [x_3, x_1] (share_mapping {2,0})
//     // party_3: [x_1, x_2] (share_mapping {0,1})
//     //
//     // For more than 3 parties, it is a bit more complicated, but you should
//     // just use the ones defined below of 5 and 7 parties

//     std::vector<std::vector<int>> share_mapping;
//     int numPeers = ss->getPeers();
//     switch (numPeers) {
//     case 3:
//         share_mapping = {
//             {1, 2}, // p1
//             {2, 0}, // p2
//             {0, 1}, // p3
//         };
//         break;
//     case 5:
//         share_mapping = {
//             {4, 5, 6, 7, 8, 9}, // p1
//             {7, 8, 1, 9, 2, 3}, // p2
//             {9, 2, 5, 3, 6, 0}, // p3
//             {3, 6, 8, 0, 1, 4}, // p4
//             {0, 1, 2, 4, 5, 7}  // p5
//         };
//         break;
//     case 7:
//         share_mapping = {
//             {15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34}, // p1
//             {25, 26, 27, 5, 28, 29, 6, 30, 7, 8, 31, 32, 9, 33, 10, 11, 34, 12, 13, 14},      // p2
//             {31, 32, 9, 19, 33, 10, 20, 11, 21, 1, 34, 12, 22, 13, 23, 2, 14, 24, 3, 4},      // p3
//             {34, 12, 22, 28, 13, 23, 29, 2, 6, 16, 14, 24, 30, 3, 7, 17, 4, 8, 18, 0},        // p4
//             {14, 24, 30, 33, 3, 7, 10, 17, 20, 26, 4, 8, 11, 18, 21, 27, 0, 1, 5, 15},        // p5
//             {4, 8, 11, 13, 18, 21, 23, 27, 29, 32, 0, 1, 2, 5, 6, 9, 15, 16, 19, 25},         // p6
//             {0, 1, 2, 3, 5, 6, 7, 9, 10, 12, 15, 16, 17, 19, 20, 22, 25, 26, 28, 31}          // p7
//         };
//         break;
//     default:
//         fprintf(stderr, "ERROR (rss_setup): invalid number of parties, only n = {3, 5, 7} is supported for RSS \n");
//         exit(1);
//     }

//     // initalizing the variable which store ALL of the shares that are generated (see below)
//     priv_int_t **Data1 = new priv_int_t *[totalNumShares];
//     priv_int_t **Data2 = new priv_int_t *[totalNumShares];
//     for (int i = 0; i < totalNumShares; i++) {
//         Data1[i] = new priv_int_t[batch_size];
//         memset(Data1[i], 0, sizeof(priv_int_t) * batch_size);
//         Data2[i] = new priv_int_t[batch_size];
//         memset(Data2[i], 0, sizeof(priv_int_t) * batch_size);
//     }

//     // To secret share a private value using RSS, we generate (totalNumShares - 1)
//     // pseudorandom values. The "last" share (stored in position [totalNumShares-1]) is then
//     // computed as the secret value, minus all the pseudorandomly generated
//     // shares.
//     //
//     // Recalling our 3-party example, we have a secret x that we wish to secret share. We:
//     //   1. pseudorandomly generate the shares x_1 and x_2 by calling prg_aes_ni()
//     //   2. compute x_3 as follows:
//     //
//     //          x_3 = x - x_1 - x_2
//     //
//     // Thus satisfying our requirement that x = x_1 + x_2 + x_3

//     priv_int_t fixed_secret = 1; // an example, fixed secret value
//     for (int i = 0; i < batch_size; i++) {
//         // generating the random shares by calling our PRG
//         for (size_t j = 0; j < totalNumShares - 1; j++) {
//             prg_aes_ni(Data1[j] + i, k1, key_prg);
//             prg_aes_ni(Data2[j] + i, k2, key_prg);
//         }

//         // Setting the "secret value". You can modify these lines in order to
//         // control the secret inputs you're using for testing.

//         Data1[totalNumShares - 1][i] = i; // The secret values in Data1 are the integers 0, 1, ..., b_size - 1
//     if (id == 1 && i < 5) {
//         printf("Secret value before share reconstruction (i=%d): %lu\n", i, Data1[totalNumShares - 1][i]);
//     }

//         Data2[totalNumShares - 1][i] = fixed_secret; // all (b_size) shares of Data2 are that of the fixed_secret
//         // Though all shares in different [i] positions are unique, they will all reconstruct to fixed_secret.
//         for (size_t j = 0; j < totalNumShares - 1; j++) {
//             // computing the "final" share
//             Data1[totalNumShares - 1][i] -= Data1[j][i];
//             Data2[totalNumShares - 1][i] -= Data2[j][i];
//         }
//     }
    
// // //****
// //  // Declare the 3D arrays for FLLT
// // priv_int_t ***a = new priv_int_t **[4];  // 4 components per floating-point value
// // priv_int_t ***b = new priv_int_t **[4];
// // priv_int_t **result = new priv_int_t *[numShares];  // output: [a < b]

// // for (int comp = 0; comp < 4; comp++) {
// //     a[comp] = new priv_int_t *[numShares];
// //     b[comp] = new priv_int_t *[numShares];
// //     for (int s = 0; s < numShares; s++) {
// //         a[comp][s] = new priv_int_t[batch_size];
// //         b[comp][s] = new priv_int_t[batch_size];
// //         memset(a[comp][s], 0, sizeof(priv_int_t) * batch_size);
// //         memset(b[comp][s], 0, sizeof(priv_int_t) * batch_size);
// //     }
// // }

// // for (int s = 0; s < numShares; ++s) {
// //     result[s] = new priv_int_t[batch_size];
// //     memset(result[s], 0, sizeof(priv_int_t) * batch_size);
// // }

// // // === Step 1: Populate a[i] and b[i] ===
// // // Let's say we want to test 3 numbers where:
// // // a = [2.0, 3.0, 5.0], b = [4.0, 3.0, 1.0]
// // // Suppose exponent is 1 and mantissa is in fixed-point * 2^e form
// // // Sign = 0 (positive), Zero flag = 0

// // for (int i = 0; i < batch_size; i++) {
// //     int mant_a = 2 + i;         // a = 2, 3, 4, ...
// //     int mant_b = 4 - i;         // b = 4, 3, 2, 1, 0, -1, ...
// //     int sign_b = (mant_b < 0);  // sign bit: 1 if negative
// //     mant_b = abs(mant_b);       // store absolute mantissa
// //     int expo = 1;

// //     for (int s = 0; s < numShares; s++) {
// //         int mapped_idx = share_mapping[id - 1][s];

// //         a[0][s][i] = Data1[mapped_idx][i] = mant_a;
// //         a[1][s][i] = expo;
// //         a[2][s][i] = 0;     // zero flag
// //         a[3][s][i] = 0;     // positive

// //         b[0][s][i] = Data2[mapped_idx][i] = mant_b;
// //         b[1][s][i] = expo;
// //         b[2][s][i] = 0;
// //         b[3][s][i] = sign_b;
// //     }
// // }


// // // === Step 2: Run FLLT protocol ===
// // printf("Running FLLT...\n");
// // FLLT(a, b, result, batch_size, ring_size, threadID, net, ss);

// // // === Step 3: Open and print results ===
// // priv_int_t *out = new priv_int_t[batch_size];
// // Open(out, result, batch_size, -1, net, ss);

// // if (id == 1) {
// //     for (int i = 0; i < batch_size; i++) {
// //         printf("a = %d, b = %d --> a < b? %lu\n", 2 + i, 4 - i, out[i]);
// //     }
// // }
// //*** */
//     // Private array initalization
//     // for RSS, priv_int is typedef (in RSS_types.hpp) as either a 32- or 64-bit POINTER
//     // so these variables are actually two-dimensional arrays
//     // The dimensions of RSS variables are
//     //
//     // var[numShares][batch_size]
//     //
//     // This is so we are guaranteed to have large contiguous blocks of memory, which improves 
//     // performance.

// // New testing for FLLT code
// // priv_int **in_1 = new priv_int*[4];
// // priv_int **in_2 = new priv_int*[4];
// //priv_int *out_1 = new priv_int[10];

// // OLd code with added new element 
//     priv_int_t **out_1 = new priv_int_t*[numShares];
//     for (int s = 0; s < numShares; ++s) {
//         out_1[s] = new priv_int_t[batch_size];
//     }
//     priv_int_t ***in_1 = new priv_int_t**[4];
//     priv_int_t ***in_2 = new priv_int_t**[4];
//     for (int j = 0; j < 4; ++j) {
//         in_1[j] = new priv_int_t*[numShares];
//         in_2[j] = new priv_int_t*[numShares];
//         for (int s = 0; s < numShares; ++s) {
//             in_1[j][s] = new priv_int_t[batch_size];
//             in_2[j][s] = new priv_int_t[batch_size];

//         }
//     }

//     float numbers_1[10] = {1.45, 3.67, 2.34, 5.89, 0.76, 8.12, 9.67, 3.21, 4.56, 7.89};
//     float numbers_2[10] = {1.10, 4.67, 5.34, 2.89, 1.76, 8.12, 9.67, 3.21, 4.56, 7.89};

//     for (int i = 0; i < 10; i++) {
//         // Using long version
//         long long *elements_1 = new long long[4];
//         long long *elements_2 = new long long[4];
//         priv_int_t elements1_all[batch_size][4];
//         priv_int_t elements2_all[batch_size][4];
//                 // // Using mpz_t version
//         // mpz_t *elements_1 = new mpz_t[4];
//         // mpz_t *elements_2 = new mpz_t[4];
//         // for (int j = 0; j < 4; j++) {
//         //     mpz_init(elements_1[j]);
//         //     mpz_init(elements_2[j]);
//         // }

//         convertFloat(numbers_1[i], 32, 8, &elements_1);
//         convertFloat(numbers_2[i], 32, 8, &elements_2);
  
//         for (int j = 0; j < 4; j++) {
//                 elements1_all[i][j] = static_cast<priv_int_t>(elements_1[j]);
//                 elements2_all[i][j] = static_cast<priv_int_t>(elements_2[j]);
//             }

//         // Print the original float and its 4-part converted representation (mantissa, exponent, zero flag, sign)
//         cout << "Number 1: " << numbers_1[i] << " -> Converted (mantissa, exponent, zero_flag, sign): ";
//         for (int j = 0; j < 4; j++) {
//             cout << elements_1[j] << " ";
//         }
//         cout << endl;

//         // Same for number 2
//         cout << "Number 2: " << numbers_2[i] << " -> Converted (mantissa, exponent, zero_flag, sign): ";
//         for (int j = 0; j < 4; j++) {
//             cout << elements_2[j] << " ";
//         }
//         cout << endl;
//         //Store the converted values into the in_1 and in_2 arrays
//         for (int s = 0; s < numShares; ++s) {
//             for (int j = 0; j < 4; j++) {
//                 (in_1[j][s][i]) = elements_1[j];         // store into allocated uint64_t*
//                 (in_2[j][s][i]) = elements_2[j];     // scaled version for variation
//             }
//         }
//     }

//     printf("FLLT Started running...\n");
//     FLLT(in_1, in_2, out_1, batch_size, ring_size,-1 ,net, ss);
//     printf("FLLT Finished.\n");

//     // // === Reveal and print FLLT outputs ===
//     // printf("\n====== FLLT Final Output ======\n");
//     // priv_int_t *output_vals = new priv_int_t[batch_size];
//     // Open(output_vals, out_1, batch_size, -1, net, ss);

//     // for (int i = 0; i < batch_size; ++i) {
//     //     int64_t corrected = (output_vals[i] < (1ULL << 63))
//     //                         ? output_vals[i]
//     //                         : (int64_t)(output_vals[i] - (1ULL << 64));

//     //     printf("[i=%d] FLLT result = %ld\n", i, corrected);
//     // }

//     // delete[] output_vals;

// // === Reveal and print FLLT outputs ===
// printf("\n====== FLLT Final Output ======\n");
// priv_int_t *output_vals = new priv_int_t[batch_size];
// Open(output_vals, out_1, batch_size, -1, net, ss);
// priv_int_t elements1_all[batch_size][4];
// priv_int_t elements2_all[batch_size][4];

// for (int i = 0; i < batch_size; ++i) {
//     int64_t corrected = (output_vals[i] < (1ULL << 63))
//                         ? output_vals[i]
//                         : (int64_t)(output_vals[i] - (1ULL << 64));

//     int expected = (numbers_1[i] < numbers_2[i]) ? 1 : 0;

//     printf("[i=%d] num1=%.6f num2=%.6f | ", i, numbers_1[i], numbers_2[i]);
//     printf("E1=[%lu, %lu, %lu, %lu] ", elements1_all[i][0], elements1_all[i][1], elements1_all[i][2], elements1_all[i][3]);
//     printf("E2=[%lu, %lu, %lu, %lu] ", elements2_all[i][0], elements2_all[i][1], elements2_all[i][2], elements2_all[i][3]);
//     printf("=> FLLT result = %ld (expected: %d)\n", corrected, expected);
// }

// delete[] output_vals;

// // // THe version after Alessandro's code 
// //     priv_int_t **out_1 = new priv_int_t*[numShares];
// //     for (int s = 0; s < numShares; ++s) {
// //         out_1[s] = new priv_int_t[batch_size];
// //     }
// //     priv_int_t ***in_1 = new priv_int_t**[4];
// //     priv_int_t ***in_2 = new priv_int_t**[4];
// //     for (int j = 0; j < 4; ++j) {
// //         in_1[j] = new priv_int_t*[numShares];
// //         in_2[j] = new priv_int_t*[numShares];
// //         for (int s = 0; s < numShares; ++s) {
// //             in_1[j][s] = new priv_int_t[batch_size];
// //             in_2[j][s] = new priv_int_t[batch_size];

// //         }
// //     }

// //     float numbers_1[10] = {1.45, 3.67, 2.34, 5.89, 0.76, 8.12, 9.67, 3.21, 4.56, 7.89};
// //     float numbers_2[10] = {1.10, 4.67, 5.34, 2.89, 1.76, 8.12, 9.67, 3.21, 4.56, 7.89};
// //     // === Initialize float conversion buffers ===
// //     mpz_t *elements_1 = new mpz_t[4];
// //     mpz_t *elements_2 = new mpz_t[4];
// //     for (int j = 0; j < 4; j++) {
// //         mpz_init(elements_1[j]);
// //         mpz_init(elements_2[j]);
// //     }

// //     // === Allocate memory for shared input data ===
// //     priv_int_t ***Data3 = new priv_int_t **[4];
// //     priv_int_t ***Data4 = new priv_int_t **[4];
// //     for (int i = 0; i < 4; i++) {
// //         Data3[i] = new priv_int_t *[totalNumShares];
// //         Data4[i] = new priv_int_t *[totalNumShares];
// //         for (int j = 0; j < totalNumShares; j++) {
// //             Data3[i][j] = new priv_int_t[batch_size];
// //             Data4[i][j] = new priv_int_t[batch_size];
// //             memset(Data3[i][j], 0, sizeof(priv_int_t) * batch_size);
// //             memset(Data4[i][j], 0, sizeof(priv_int_t) * batch_size);
// //         }
// //     }
// //     priv_int_t elements_a[4];  // will hold integer-converted values from elements_1
// //     priv_int_t elements_b[4];  // will hold integer-converted values from elements_2

// //     // === For each float, convert and secret share ===
// //     for (int i = 0; i < batch_size; i++) {
// //        convertFloat(numbers_1[i], 8, 32, &elements_1);
// // convertFloat(numbers_2[i], 8, 32, &elements_2);

// // printf("[i=%d] After convertFloat num1=%.6f -> E1=[%lu, %lu, %lu, %lu]\n", i, numbers_1[i],
// //        mpz_get_ui(elements_1[0]), mpz_get_ui(elements_1[1]), mpz_get_ui(elements_1[2]), mpz_get_ui(elements_1[3]));

// // printf("[i=%d] After convertFloat num2=%.6f -> E2=[%lu, %lu, %lu, %lu]\n", i, numbers_2[i],
// //        mpz_get_ui(elements_2[0]), mpz_get_ui(elements_2[1]), mpz_get_ui(elements_2[2]), mpz_get_ui(elements_2[3]));

// //     for (int j = 0; j < 4; j++) {
// //         elements_a[j] = mpz_get_ui(elements_1[j]);
// //         elements_b[j] = mpz_get_ui(elements_2[j]);
// //     }

// //         // Fill Data3and Data4with shares
// //         for (int k = 0; k < 4; k++) {
// //             for (size_t j = 0; j < totalNumShares - 1; j++) {
// //                 prg_aes_ni(Data3[k][j] + i, k1, key_prg);
// //                 prg_aes_ni(Data4[k][j] + i, k1, key_prg);
// //             }
// //             Data3[k][totalNumShares - 1][i] = elements_a[k];
// //             Data4[k][totalNumShares - 1][i] = elements_b[k];

// //             for (size_t j = 0; j < totalNumShares - 1; j++) {
// //                 Data3[k][totalNumShares - 1][i] -= Data3[k][j][i];
// //                 Data4[k][totalNumShares - 1][i] -= Data4[k][j][i];
// //             }
// //         }
// //     }

// //     // === Map shares into in_1 and in_2 ===
// //     for (size_t k = 0; k < 4; k++) {
// //         for (size_t i = 0; i < numShares; i++) {
// //             in_1[k][i] = Data3[k][share_mapping[id - 1][i]];
// //             in_2[k][i] = Data4[k][share_mapping[id - 1][i]];
// //         }
// //     }

// //     // === Call the secure protocol ===
// //     printf("FLLT Started running...\n");
// //     FLLT(in_1, in_2, out_1, batch_size, ring_size, -1, net, ss);
// //     printf("FLLT Finished.\n");


// //     priv_int_t *output_vals = new priv_int_t[batch_size];
// //     Open(output_vals, out_1, batch_size, -1, net, ss);

// //     for (int i = 0; i < batch_size; ++i) {
// //     int64_t corrected = (output_vals[i] < (1ULL << 63)) ? output_vals[i]
// //                                                         : (int64_t)(output_vals[i] - (1ULL << 64));

// //     printf("[i=%d] num1=%.6f num2=%.6f | ", i, numbers_1[i], numbers_2[i]);

// //     printf("E1=[%lu, %lu, %lu, %lu] ", elements_a[0], elements_a[1], elements_a[2], elements_a[3]);
// //     printf("E2=[%lu, %lu, %lu, %lu] ", elements_b[0], elements_b[1], elements_b[2], elements_b[3]);

// //     printf("=> FLLT result = %ld\n", corrected);
// // }


// //     delete[] output_vals;


// //     // === Clean up mpz memory ===
// //     for (int j = 0; j < 4; j++) {
// //         mpz_clear(elements_1[j]);
// //         mpz_clear(elements_2[j]);
// //     }
// //     delete[] elements_1;
// //     delete[] elements_2;
// // // Update code base on Alessandro
// // // === Setup output and input arrays ===
// // priv_int_t **out_1 = new priv_int_t*[numShares];
// // for (int s = 0; s < numShares; ++s) {
// //     out_1[s] = new priv_int_t[batch_size];
// // }

// // priv_int_t ***in_1 = new priv_int_t**[4];
// // priv_int_t ***in_2 = new priv_int_t**[4];
// // for (int j = 0; j < 4; ++j) {
// //     in_1[j] = new priv_int_t*[numShares];
// //     in_2[j] = new priv_int_t*[numShares];
// //     for (int s = 0; s < numShares; ++s) {
// //         in_1[j][s] = new priv_int_t[batch_size];
// //         in_2[j][s] = new priv_int_t[batch_size];
// //     }
// // }

// // // === Numbers to convert ===
// // float numbers_1[10] = {1.45, 3.67, 2.34, 5.89, 0.76, 8.12, 9.67, 3.21, 4.56, 7.89};
// // float numbers_2[10] = {1.10, 4.67, 5.34, 2.89, 1.76, 8.12, 9.67, 3.21, 4.56, 7.89};

// // // === Store converted elements for later printing ===
// // priv_int_t elements1_all[batch_size][4];
// // priv_int_t elements2_all[batch_size][4];

// // mpz_t *elements_1_ptr = new mpz_t[4];
// // mpz_t *elements_2_ptr = new mpz_t[4];
// // for (int j = 0; j < 4; j++) {
// //     mpz_init(elements_1_ptr[j]);
// //     mpz_init(elements_2_ptr[j]);
// // }

// // priv_int_t ***Data3 = new priv_int_t **[4];
// // priv_int_t ***Data4 = new priv_int_t **[4];
// // for (int i = 0; i < 4; i++) {
// //     Data3[i] = new priv_int_t *[totalNumShares];
// //     Data4[i] = new priv_int_t *[totalNumShares];
// //     for (int j = 0; j < totalNumShares; j++) {
// //         Data3[i][j] = new priv_int_t[batch_size];
// //         Data4[i][j] = new priv_int_t[batch_size];
// //         memset(Data3[i][j], 0, sizeof(priv_int_t) * batch_size);
// //         memset(Data4[i][j], 0, sizeof(priv_int_t) * batch_size);
// //     }
// // }

// // // === Convert and share ===
// // for (int i = 0; i < batch_size; i++) {
// //     convertFloat(numbers_1[i], 32, 8, &elements_1_ptr);
// //     convertFloat(numbers_2[i], 32, 8, &elements_2_ptr);
    

// //     for (int j = 0; j < 4; j++) {
// //         elements1_all[i][j] = mpz_get_ui(elements_1_ptr[j]);
// //         elements2_all[i][j] = mpz_get_ui(elements_2_ptr[j]);
// //     }

// //     for (int k = 0; k < 4; k++) {
// //         for (size_t j = 0; j < totalNumShares - 1; j++) {
// //             prg_aes_ni(Data3[k][j] + i, k1, key_prg);
// //             prg_aes_ni(Data4[k][j] + i, k1, key_prg);
// //         }

// //         Data3[k][totalNumShares - 1][i] = elements1_all[i][k];
// //         Data4[k][totalNumShares - 1][i] = elements2_all[i][k];

// //         for (size_t j = 0; j < totalNumShares - 1; j++) {
// //             Data3[k][totalNumShares - 1][i] -= Data3[k][j][i];
// //             Data4[k][totalNumShares - 1][i] -= Data4[k][j][i];
// //         }
// //     }
// // }

// // for (size_t k = 0; k < 4; k++) {
// //     for (size_t i = 0; i < numShares; i++) {
// //         in_1[k][i] = Data3[k][share_mapping[id - 1][i]];
// //         in_2[k][i] = Data4[k][share_mapping[id - 1][i]];
// //     }
// // }
// // for (int i = 0; i < batch_size; ++i) {
// //     printf("Secret value before share reconstruction (i=%d):\n", i);
// //     for (int k = 0; k < 4; k++) {
// //         priv_int_t secret1 = 0, secret2 = 0;
// //         for (int s = 0; s < totalNumShares; s++) {
// //             secret1 += Data3[k][s][i];
// //             secret2 += Data4[k][s][i];
// //         }
// //         printf("  component[%d] input1=%lu input2=%lu\n", k, secret1, secret2);
// //     }
// // }

// // printf("FLLT Started running...\n");
// // FLLT(in_1, in_2, out_1, batch_size, ring_size, -1, net, ss);
// // printf("FLLT Finished.\n");

// // priv_int_t *output_vals = new priv_int_t[batch_size];
// // Open(output_vals, out_1, batch_size, -1, net, ss);

// // for (int i = 0; i < batch_size; ++i) {
// //     int64_t corrected = (output_vals[i] < (1ULL << 63)) ? output_vals[i]
// //                                                         : (int64_t)(output_vals[i] - (1ULL << 64));

// //     printf("[i=%d] num1=%.6f num2=%.6f | ", i, numbers_1[i], numbers_2[i]);
// //     printf("E1=[%lu, %lu, %lu, %lu] ", elements1_all[i][0], elements1_all[i][1], elements1_all[i][2], elements1_all[i][3]);
// //     printf("E2=[%lu, %lu, %lu, %lu] ", elements2_all[i][0], elements2_all[i][1], elements2_all[i][2], elements2_all[i][3]);
// //     printf("=> FLLT result = %ld\n", corrected);
// // }

// // delete[] output_vals;

// // for (int j = 0; j < 4; j++) {
// //     mpz_clear(elements_1_ptr[j]);
// //     mpz_clear(elements_2_ptr[j]);
// // }
// // delete[] elements_1_ptr;
// // delete[] elements_2_ptr;

// // priv_int_t *output_vals = new priv_int_t[batch_size];
// // Open(output_vals, out_1, batch_size, -1, net, ss);

// // // for (int i = 0; i < batch_size; ++i) {
// // //     int64_t corrected = (output_vals[i] < (1ULL << 63)) ? output_vals[i]
// // //         : (int64_t)(output_vals[i] - (1ULL << 64));
// // //     printf("[i=%d] FLLT result = %ld\n", i, corrected);
// // // }
// // for (int i = 0; i < batch_size; ++i) {
// //     printf("out_1[%d]: ", i);
// //     for (int s = 0; s < numShares; ++s) {
// //         printf("%lu ", out_1[i][s]);
// //     }
// //     printf("\n");



// // }

// // //Testing RNTE for protocol 18
// //     // For each party, allocate an array of 'n' arrays of size 4
// //     priv_int *in_1 = new priv_int[ss->getNumShares()];
// //     // priv_int **in_2 = new priv_int*[ss->getNumShares()];
// //     priv_int *out_1 = new priv_int[ss->getNumShares()];
// //     priv_int *out_2 = new priv_int[ss->getNumShares()];

// //     // priv_int res_1 = new priv_int_t[batch_size];
// //     // priv_int res_2 = new priv_int_t[batch_size];
// //     // priv_int res_3 = new priv_int_t[batch_size];
// //     // priv_int res_4 = new priv_int_t[batch_size];

// //     for (int i = 0; i < ss->getNumShares(); i++) {
// //         out_1[i] = new priv_int_t[batch_size];
// //         memset(out_1[i], 0, sizeof(priv_int_t) * batch_size);
// //         out_2[i] = new priv_int_t[batch_size];
// //         memset(out_2[i], 0, sizeof(priv_int_t) * batch_size);
// //     }

// //         // assigning the shares to the correct locations, according to the share mapping we defined previously
// //     for (size_t i = 0; i < numShares; i++) {
// //         in_1[i] = Data1[share_mapping[id - 1][i]];
      
// //     }
// //     // Let's first verify that our shares (stored in in_1 and in_2) reconstruct to what we expect:
// //     // in_1 should reconstruct to 0, 1, ..., b_size-1
// //     // in_2 should reconstruct to 1, 1, ..., 1
// //     // Open(res_1, in_1, batch_size, -1, net, ss);
// //     // Open(res_2, in_2, batch_size, -1, net, ss);

// //     // for (priv_int_t i = 0; i < batch_size; i++) {
// //     //     if (!(res_1[i] == i)) {
// //     //         printf("in_1 did not reconstruct properly\n");
// //     //         printf("(expect)   [%u]: %u\n", i, i);
// //     //         printf("(actual)   [%u]: %u\n", i, res_2[i]);
// //     //     }
// //     //     if (!(res_2[i] == fixed_secret)) {

// //     //         printf("in_2 did not reconstruct properly\n");
// //     //         printf("(expect)   [%u]: %u\n", i, fixed_secret);
// //     //         printf("(actual)   [%u]: %u\n", i, res_2[i]);
// //     //     }
// //     // }

// //     // For illustrative purposes, suppose we want to test the protocol EQZ for
// //     // correctness. The expected behavior for EQZ is that it returns 1 if the
// //     // input (in_1 in this example) is zero, and 0 otherwise (in_1 > 0). The
// //     // implementation will be performing (b_size) many operations in parallel.
// //     // The secret-shared result is stored in out_1.
// //     printf("Trunc Started running...\n");
// //     // doOperation_EQZ(in_1, out_1, ring_size, batch_size, -1, net, ss);
// //     doOperation_Trunc_RNTE(out_1,out_2, in_1, ring_size, 5, batch_size, -1, net, ss);
// //     // Allocate space for opened values
// //     priv_int_t *res_1 = new priv_int_t[batch_size];
// //     priv_int_t *res_2 = new priv_int_t[batch_size];
// //     priv_int_t *inputs = new priv_int_t[batch_size];

// //     // Open and reconstruct the secret input and outputs
// //     Open(inputs, in_1, batch_size, -1, net, ss);    // original input
// //     Open(res_1, out_1, batch_size, -1, net, ss);    // truncated by m
// //     Open(res_2, out_2, batch_size, -1, net, ss);    // truncated by m - 2

// //     // // Print the results for verification
// //     printf("truncated results:\n");
// // for (int i = 0; i < batch_size; i++) {
// //     printf("[i=%d] input = %ld | trunc = %ld | trunc(m-2) = %ld\n",
// //            i,
// //            (int64_t)inputs[i],
// //            (int64_t)res_1[i],
// //            (int64_t)res_2[i]);
// // }

// //ending of trunc
// // //  // === RNTE Test Setup === protocol 19

// // // Allocate input and output share arrays
// // priv_int *in_1 = new priv_int[ss->getNumShares()];
// // priv_int *out_1 = new priv_int[ss->getNumShares()];

// // for (int i = 0; i < ss->getNumShares(); i++) {
// //     in_1[i] = new priv_int_t[batch_size];      // each party's input shares
// //     out_1[i] = new priv_int_t[batch_size];     // each party's output shares
// //     memset(in_1[i], 0, sizeof(priv_int_t) * batch_size);
// // }
// // // Assigning the shares using share mapping
// // for (int i = 0; i < ss->getNumShares(); i++) {
// //     for (int j = 0; j < batch_size; j++) {
// //         in_1[i][j] = Data1[share_mapping[id - 1][i]][j]; // map each share
// //     }
// // }
// // printf("Rounding Started running...\n");
// // // Calling RNTE protocol implementation
// // RNTE(out_1, in_1, ring_size, 5, batch_size, -1, net, ss);
// // // Allocating open secret shared values
// // priv_int_t *res_input = new priv_int_t[batch_size];
// // priv_int_t *res_output = new priv_int_t[batch_size];
// // // Reconstructing original inputs and rounded outputs
// // Open(res_input, in_1, batch_size, -1, net, ss);
// // Open(res_output, out_1, batch_size, -1, net, ss);

// // // Printing and validating
// // for (int i = 0; i < batch_size; i++) {
// //     printf("i = %d | input = %ld | output = %ld\n", i, (int64_t)res_input[i], (int64_t)res_output[i]);
// // }
// // //Ending of RNTE protocol 19
// //     delete[] res_1;
// //     delete[] res_2;

// //     for (size_t i = 0; i < totalNumShares; i++) {
// //         delete[] Data1[i];
// //         delete[] Data2[i];
// //     }

// //     delete[] Data1;
// //     delete[] Data2;

// //     for (size_t i = 0; i < numShares; i++) {
// //         delete[] out_1[i];
// //         delete[] out_2[i];
// //     }
// //     // delete[] in_1;
// //     // delete[] in_2;
// //     delete[] out_1;
// //     delete[] out_2;
// // 
// }

// ////////////////////////////////////////////////
// // PRG-related functions, do not modify
// ////////////////////////////////////////////////

// void SMC_Utils::offline_prg(uint8_t *dest, uint8_t *src, __m128i *ri) { // ri used to be void, replaced with __m128i* to compile
//     __m128i orr, mr;
//     __m128i *r = ri;

//     orr = _mm_load_si128((__m128i *)src);
//     mr = orr;

//     mr = _mm_xor_si128(mr, r[0]);
//     mr = _mm_aesenc_si128(mr, r[1]);
//     mr = _mm_aesenc_si128(mr, r[2]);
//     mr = _mm_aesenc_si128(mr, r[3]);
//     mr = _mm_aesenc_si128(mr, r[4]);
//     mr = _mm_aesenc_si128(mr, r[5]);
//     mr = _mm_aesenc_si128(mr, r[6]);
//     mr = _mm_aesenc_si128(mr, r[7]);
//     mr = _mm_aesenc_si128(mr, r[8]);
//     mr = _mm_aesenc_si128(mr, r[9]);
//     mr = _mm_aesenclast_si128(mr, r[10]);
//     mr = _mm_xor_si128(mr, orr);
//     _mm_storeu_si128((__m128i *)dest, mr);
// }

// __m128i *SMC_Utils::offline_prg_keyschedule(uint8_t *src) {
//     // correctness must be checked here (modified from original just to compile, used to not have cast (__m128i*))
//     // __m128i *r = static_cast<__m128i *>(malloc(11 * sizeof(__m128i)));
//     __m128i *r = new __m128i[11]; // alternate
//     r[0] = _mm_load_si128((__m128i *)src);

//     KE2(r[1], r[0], 0x01)
//     KE2(r[2], r[1], 0x02)
//     KE2(r[3], r[2], 0x04)
//     KE2(r[4], r[3], 0x08)
//     KE2(r[5], r[4], 0x10)
//     KE2(r[6], r[5], 0x20)
//     KE2(r[7], r[6], 0x40)
//     KE2(r[8], r[7], 0x80)
//     KE2(r[9], r[8], 0x1b)
//     KE2(r[10], r[9], 0x36)
//     return r;
// }

// void SMC_Utils::prg_aes_ni(priv_int_t *destination, uint8_t *seed, __m128i *key) {
//     uint8_t res[16] = {};
//     offline_prg(res, seed, key);
//     memset(seed, 0, 16);
//     memset(destination, 0, sizeof(priv_int_t));
//     memcpy(seed, res, sizeof(priv_int_t));        // cipher becomes new seed or key
//     memcpy(destination, res, sizeof(priv_int_t)); // cipher becomes new seed or key
// }


