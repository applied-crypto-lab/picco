#ifndef FLOATING_POINT_H__
#define FLOATING_POINT_H__

#include <cassert>
#include <cmath>
#include <cstring>
#include <inttypes.h>

// #include <iostream>
// #include "NodeNetwork.h"

struct FLStruct {
public:
    uint size = 0;          // size of array
    uint numShares = 0;     // number of shares
    uint8_t **s = nullptr;  // sign
    uint8_t **z = nullptr;  // is_zero?
    uint64_t **m = nullptr; // mantissa
    uint64_t **e = nullptr; // exponent

    FLStruct(){};

    FLStruct(int _size, uint _numShares) {
        assert(_size > 0);
        assert(_numShares > 0);
        this->size = _size;
        this->numShares = _numShares;
        s = new uint8_t *[numShares];
        z = new uint8_t *[numShares];
        m = new uint64_t *[numShares];
        e = new uint64_t *[numShares];
        for (size_t i = 0; i < numShares; i++) {
            s[i] = new uint8_t[size];
            z[i] = new uint8_t[size];
            m[i] = new uint64_t[size];
            e[i] = new uint64_t[size];
        }
    }

    // copy constructor
    FLStruct(const FLStruct &other) {
        this->size = other.size;
        this->numShares = other.numShares;
        this->s = new uint8_t *[numShares];
        this->z = new uint8_t *[numShares];
        this->m = new uint64_t *[numShares];
        this->e = new uint64_t *[numShares];
        s = new uint8_t *[numShares];
        z = new uint8_t *[numShares];
        m = new uint64_t *[numShares];
        e = new uint64_t *[numShares];
        for (size_t i = 0; i < numShares; i++) {
            s[i] = new uint8_t[size];
            memcpy(this->s, other.s, size * sizeof(uint8_t));
            z[i] = new uint8_t[size];
            memcpy(this->z, other.z, size * sizeof(uint8_t));
            m[i] = new uint64_t[size];
            memcpy(this->m, other.m, size * sizeof(uint64_t));
            e[i] = new uint64_t[size];
            memcpy(this->e, other.e, size * sizeof(uint64_t));
        }
    }

    // move constructor
    // don't think this will be needed
    // FLStruct(FLStruct &&other) noexcept {
    //     this->size = other.size;
    //     this->numShares = other.numShares;
    //     this->s = other.s;
    //     this->z = other.z;
    //     this->m = other.m;
    //     this->e = other.e;
    //     other.s = nullptr;
    //     other.z = nullptr;
    //     other.m = nullptr;
    //     other.e = nullptr;
    // }

    ~FLStruct() {
        for (int i = 0; i < numShares; i++) {
            delete[] s[i];
            delete[] z[i];
            delete[] m[i];
            delete[] e[i];
        }

        delete[] s;
        delete[] z;
        delete[] m;
        delete[] e;
    }
};

#endif // FLOATING_POINT_H__
