/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame
   ** Department of Computer Science and Engineering, University of Buffalo (SUNY)

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

#ifndef NODENETWORK_H_
#define NODENETWORK_H_

// key size in bytes; switching to a longer size will require more work than updating this constant as 128-bit encryption function calls are used
#define KEYSIZE 16
// the block size is always 16 with AES, use AES_BLOCK_SIZE

#include "NodeConfiguration.h"
#include "stdint.h"
#include <cstdlib>
#include <cstring>
#include <gmp.h>
#include <map>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <vector>

class NodeNetwork {
public:
    NodeNetwork(NodeConfiguration *nodeConfig, std::string privatekey_filename, int num_threads);
    NodeNetwork();
    virtual ~NodeNetwork();

    // Send round data to a specific peer
    void sendDataToPeer(int, mpz_t *, int, int, int);
    void sendDataToPeer(int, int, int *);
    void sendDataToPeer(int, int, unsigned char *);
    void sendDataToPeer(int, int, mpz_t *);
    void sendDataToPeer(int, int, long long *);
    void sendModeToPeers(int);
    // Get round data from a specific peer
    void getDataFromPeer(int, mpz_t *, int, int, int);
    void getDataFromPeer(int, int, int *);
    void getDataFromPeer(int, int, unsigned char *);
    void getDataFromPeer(int, int, mpz_t *);
    void getDataFromPeer(int, int, long long *);
    // Broadcast identical data to peers
    void broadcastToPeers(mpz_t *, int, mpz_t **);
    void broadcastToPeers(long long *, int, long long **);
    void multicastToPeers(mpz_t **, mpz_t **, int);
    void multicastToPeers(long long **, long long **, int);

    // Get the ID of the compute Node
    int getID();
    int getNumOfThreads();
    void getRounds(int, int *, int *);
    // void handle_write(const boost::system::error_code& error);
    // Close all the connections to peers
    void closeAllConnections();

    // Encryption and Decryption
    void init_keys(int peer, int nRead);
    void gen_keyiv();
    void get_keyiv(char *key_iv);
    unsigned char *aes_encrypt(EVP_CIPHER_CTX *e, unsigned char *plaintext, int *len);
    unsigned char *aes_decrypt(EVP_CIPHER_CTX *e, unsigned char *ciphertext, int *len);

    // Close
    void mpzFromString(char *, mpz_t *, int *, int);
    double time_diff(struct timeval *, struct timeval *);

    // Manager work
    void *managerWork();
    static void *managerWorkHelper(void *);
    void getDataFromPeer(int, int, int);
    void getDataFromPeerToBuffer(int, int, int, int **);
    void restoreDataToBuffer(int, int, int, int **);
    void sendDataToPeer(int, mpz_t *, int, int, int, int);
    void multicastToPeers(mpz_t **, mpz_t **, int, int);
    void broadcastToPeers(mpz_t *, int, mpz_t **, int);

    // void getRandOfPeer(int id, mpz_t *rand_id, int size);
    void multicastToPeers_Mul(mpz_t **data, int size, int threadID);
    void multicastToPeers_Mul2(mpz_t **data, int size);
    void multicastToPeers_Mul3(uint *sendtoIDs, uint *RecvFromIDs, mpz_t **data, int size);
    void multicastToPeers_Mul_v2(uint *sendtoIDs, uint *RecvFromIDs, mpz_t **data, int size, int threadID);

    void multicastToPeers_Open(uint *sendtoIDs, uint *RecvFromIDs, mpz_t *data, mpz_t **buffer, int size, int threadID);
    void multicastToPeers_Open(uint *sendtoIDs, uint *RecvFromIDs, mpz_t *data, mpz_t **buffer, int size);

    // getter function for retreiving PRG seeds
    unsigned char **getPRGseeds();

    // to be removed
    unsigned char key_0[16];
    unsigned char key_1[16];

private:
    static pthread_mutex_t socket_mutex;
    static pthread_mutex_t buffer_mutex;
    static pthread_cond_t *socket_conditional_variables;
    static pthread_cond_t *buffer_conditional_variables;
    static pthread_cond_t *flag_conditional_variables;
    static pthread_cond_t manager_conditional_variable;
    static pthread_cond_t proceed_conditional_variable;
    static mpz_t ***buffer_handlers;
    static mpz_t ***temp_buffers;

    static int **temp_buffer_flags;
    static int ***temp_buffer_info;
    static int **finished_socks;
    static int *read_socks;
    static int **comm_flags;
    static int *unit_flags;
    static int *test_flags;
    static int mode;
    static int numOfChangedNodes;
    static int isManagerAwake;
    pthread_t manager;
    int numOfThreads;
    void connectToPeers();
    void requestConnection(int);
    void acceptPeers(int);
    std::map<int, int> peer2sock;
    std::map<int, int> sock2peer;
    int serverSock;
    uint threshold;

    // PRG seeds used in multiplication
    unsigned char **prgSeeds; // getter function works properly in SecretShare constructor
};

void print_hexa(uint8_t *message, int message_length);

#endif /* NODENETWORK_H_ */
