// **************************
// deprecated, functionality added directly to RSS class 
// to be removed

// #ifndef AES_NI_H
// #define AES_NI_H
// #include <stdio.h>
// #include <unistd.h>
// #include <stdint.h>     //for int8_t
// #include <string.h>     //for memcmp
// #include <tmmintrin.h>
// #include <wmmintrin.h>  //for intrinsics for AES-NI
// #include <inttypes.h>
// #include <unistd.h>
// #include <stdint.h>     //for int8_t
// #include <wmmintrin.h>  //for intrinsics for AES-NI
// #include <inttypes.h>
// #include "setringsize.h"

// void offline_prg(uint8_t * dest, uint8_t * src, void * ri);
// __m128i * offline_prg_keyschedule(uint8_t * src);
// void prg_aes_ni(Lint* destination, uint8_t * seed, __m128i * key);
// void prg_aes_ni_byte(uint8_t* destination, uint8_t * seed, __m128i * key);

// void test_aes();
// int print_u128_u(__uint128_t);
// int print_u128_u2(__uint128_t);
// void print_128(__uint128_t *A, int size);
// void print_1283(__uint128_t A);
// #endif


#ifndef AES_NI_H
#define AES_NI_H


#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>     //for int8_t
#include <string.h>     //for memcmp
// #include <emmintrin.h>
#include <tmmintrin.h>
#include <wmmintrin.h>  //for intrinsics for AES-NI
#include <inttypes.h>
#include <unistd.h>
#include <stdint.h>     //for int8_t
#include <wmmintrin.h>  //for intrinsics for AES-NI
#include <inttypes.h>

typedef unsigned int Lint; // for ring size in [1,30], to be changed


void offline_prg(uint8_t * dest, uint8_t * src, __m128i * ri);
__m128i * offline_prg_keyschedule(uint8_t * src);
// static void offline_prg_keyschedule(uint8_t *src, __m128i *key_schedule);

void prg_aes_ni(uint64_t* destination, uint8_t * seed, __m128i * key);
void prg_aes_ni(uint32_t* destination, uint8_t * seed, __m128i * key);
// void prg_aes_ni(Lint* destination, uint8_t * seed, __m128i * key);
void prg_aes_ni_byte(uint8_t* destination, uint8_t * seed, __m128i * key);

#ifdef __cplusplus
}
#endif

#endif
