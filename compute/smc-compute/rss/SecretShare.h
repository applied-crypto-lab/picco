
#ifndef RSS_SECRETSHARE_H
#define RSS_SECRETSHARE_H

#include "NodeNetwork.h"
#include "stdint.h"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <iostream>
#include <map>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <stdint.h> //for int8_t
#include <stdio.h>
#include <string.h> //for memcmp
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <tmmintrin.h>
#include <unistd.h>
#include <vector>
#include <wmmintrin.h> //for intrinsics for AES-NI
#include <x86intrin.h>


template <typename T>
class RSS_SecretShare {

public:
    RSS_SecretShare(NodeNetwork *nodeNet, uint n, uint t, uint ring_size);
    virtual ~RSS_SecretShare();
    uint getNumShares();
    uint getTotalNumShares();
    uint getNumParties();
    uint getThreshold();
    uint nCk(uint n, uint k);

    __m128i *prg_keyschedule(uint8_t *src);
    void prg_aes(uint8_t *, uint8_t *, __m128i *);
    void prg_setup_3();
    void prg_setup_5();
    void prg_setup_mp_7();
    void prg_getrandom(int keyID, uint size, uint length, uint8_t *dest);
    void prg_getrandom(uint size, uint length, uint8_t *dest);

    uint id;
    T *SHIFT;
    T *ODD;
    T *EVEN;
    uint RING;
    // int map_3pc[2];
    int **general_map = nullptr;
    int **open_map_mpc = nullptr;
    int **eda_map_mpc = nullptr;
    int **T_map_mpc = nullptr;


private:
    uint numParties;     // n
    uint threshold;      // t
    uint numShares;      // (n-1) choose t
    uint totalNumShares; // n choose t

    uint8_t **random_container;
    int container_size;
    int *P_container;
    __m128i **prg_key;

    NodeNetwork *nNet;
};


#include "RSS_SecretShare.tpp"


#endif