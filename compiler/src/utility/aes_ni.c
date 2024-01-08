// **************************
// deprecated, functionality added directly to RSS class 
// to be removed

#include "aes_ni.h"

#define KE2(NK, OK, RND)                           \
    NK = OK;                                       \
    NK = _mm_xor_si128(NK, _mm_slli_si128(NK, 4)); \
    NK = _mm_xor_si128(NK, _mm_slli_si128(NK, 4)); \
    NK = _mm_xor_si128(NK, _mm_slli_si128(NK, 4)); \
    NK = _mm_xor_si128(NK, _mm_shuffle_epi32(_mm_aeskeygenassist_si128(OK, RND), 0xff)); // BAD INSTRUCTION HERE ////////////////////

void offline_prg(uint8_t *dest, uint8_t *src, __m128i *ri) { // ri used to be void, replaced with __m128i* to compile
    __m128i orr, mr;
    __m128i *r = ri;

    orr = _mm_load_si128((__m128i *)src);
    mr = orr;

    mr = _mm_xor_si128(mr, r[0]);
    mr = _mm_aesenc_si128(mr, r[1]);
    mr = _mm_aesenc_si128(mr, r[2]);
    mr = _mm_aesenc_si128(mr, r[3]);
    mr = _mm_aesenc_si128(mr, r[4]);
    mr = _mm_aesenc_si128(mr, r[5]);
    mr = _mm_aesenc_si128(mr, r[6]);
    mr = _mm_aesenc_si128(mr, r[7]);
    mr = _mm_aesenc_si128(mr, r[8]);
    mr = _mm_aesenc_si128(mr, r[9]);
    mr = _mm_aesenclast_si128(mr, r[10]);
    mr = _mm_xor_si128(mr, orr);
    _mm_storeu_si128((__m128i *)dest, mr);
}

// original version from RSS repo
__m128i *offline_prg_keyschedule(uint8_t *src) {
    __m128i *r = (__m128i *)malloc(1 * sizeof(__m128i)); // correctness must be checked here (modified from original just to compile, used to not have cast (__m128i*))

    r[0] = _mm_load_si128((__m128i *)src);
    KE2(r[1], r[0], 0x01)
    KE2(r[2], r[1], 0x02)
    KE2(r[3], r[2], 0x04)
    KE2(r[4], r[3], 0x08)
    KE2(r[5], r[4], 0x10)
    KE2(r[6], r[5], 0x20)
    KE2(r[7], r[6], 0x40)
    KE2(r[8], r[7], 0x80)
    KE2(r[9], r[8], 0x1b)
    KE2(r[10], r[9], 0x36)

    return r;
}

// new version based on https://github.com/sebastien-riou/aes-brute-force/blob/master/include/aes_ni.h
// static void offline_prg_keyschedule(uint8_t *src, __m128i *key_schedule) {
//     key_schedule[0] = _mm_load_si128((__m128i *)src);
//     KE2(key_schedule[1], key_schedule[0], 0x01)
//     KE2(key_schedule[2], key_schedule[1], 0x02)
//     KE2(key_schedule[3], key_schedule[2], 0x04)
//     KE2(key_schedule[4], key_schedule[3], 0x08)
//     KE2(key_schedule[5], key_schedule[4], 0x10)
//     KE2(key_schedule[6], key_schedule[5], 0x20)
//     KE2(key_schedule[7], key_schedule[6], 0x40)
//     KE2(key_schedule[8], key_schedule[7], 0x80)
//     KE2(key_schedule[9], key_schedule[8], 0x1b)
//     KE2(key_schedule[10], key_schedule[9], 0x36)
// }

// #define DO_ENC_BLOCK(m,k) \
//     do{\
//         m = _mm_xor_si128       (m, k[ 0]); \
//         m = _mm_aesenc_si128    (m, k[ 1]); \
//         m = _mm_aesenc_si128    (m, k[ 2]); \
//         m = _mm_aesenc_si128    (m, k[ 3]); \
//         m = _mm_aesenc_si128    (m, k[ 4]); \
//         m = _mm_aesenc_si128    (m, k[ 5]); \
//         m = _mm_aesenc_si128    (m, k[ 6]); \
//         m = _mm_aesenc_si128    (m, k[ 7]); \
//         m = _mm_aesenc_si128    (m, k[ 8]); \
//         m = _mm_aesenc_si128    (m, k[ 9]); \
//         m = _mm_aesenclast_si128(m, k[10]);\
//     }while(0)
// void offline_prg(uint8_t *dest, uint8_t *src, __m128i *ri) {
// __m128i orr, mr;
// orr = _mm_load_si128((__m128i *)src);
// mr = orr;
// DO_ENC_BLOCK(mr, key_schedule);
//     _mm_storeu_si128((__m128i *)dest, mr);
// }

void prg_aes_ni(uint64_t *destination, uint8_t *seed, __m128i *key) {
    uint8_t res[16] = {};

    offline_prg(res, seed, key);
    memset(seed, 0, 16);
    memset(destination, 0, sizeof(uint64_t));
    memcpy(seed, res, sizeof(uint64_t));        // cipher becomes new seed or key
    memcpy(destination, res, sizeof(uint64_t)); // cipher becomes new seed or key
}

void prg_aes_ni(uint32_t *destination, uint8_t *seed, __m128i *key) {
    uint8_t res[16] = {};

    offline_prg(res, seed, key);
    memset(seed, 0, 16);
    memset(destination, 0, sizeof(uint32_t));
    memcpy(seed, res, sizeof(uint32_t));        // cipher becomes new seed or key
    memcpy(destination, res, sizeof(uint32_t)); // cipher becomes new seed or key
}


void prg_aes_ni_byte(uint8_t *destination, uint8_t *seed, __m128i *key) {
    uint8_t res[16] = {};
    offline_prg(res, seed, key);
    memset(seed, 0, 16);
    memset(destination, 0, sizeof(uint8_t));
    memcpy(seed, res, sizeof(uint8_t));        // cipher becomes new seed or key
    memcpy(destination, res, sizeof(uint8_t)); // cipher becomes new seed or key
}
