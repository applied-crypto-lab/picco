/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame
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

#include "NodeNetwork.h"
#include "NodeConfiguration.h"
#include "bit_utils.hpp"
#include "openssl/bio.h"
#include "time.h"
#include "unistd.h"
#include <cstdlib>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <stdio.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <vector>

#define MAX_BUFFER_SIZE 229376 // in bytes
// int MAX_BUFFER_SIZE = 4194304; // ?

NodeConfiguration *config;
EVP_CIPHER_CTX *en, *de;

std::map<int, EVP_CIPHER_CTX *> peer2enlist;
std::map<int, EVP_CIPHER_CTX *> peer2delist;
std::string privatekeyfile;
unsigned char *KeyIV;
unsigned char *peerKeyIV;

/************ STATIC VARIABLES INITIALIZATION ***************/
int **NodeNetwork::comm_flags = NULL;
int *NodeNetwork::unit_flags = NULL;

pthread_mutex_t NodeNetwork::socket_mutex;
pthread_mutex_t NodeNetwork::buffer_mutex;
pthread_cond_t *NodeNetwork::socket_conditional_variables = NULL;
pthread_cond_t *NodeNetwork::buffer_conditional_variables = NULL;
pthread_cond_t *NodeNetwork::flag_conditional_variables = NULL;
pthread_cond_t NodeNetwork::manager_conditional_variable;
pthread_cond_t NodeNetwork::proceed_conditional_variable;

mpz_t ***NodeNetwork::buffer_handlers = NULL;
mpz_t ***NodeNetwork::temp_buffers = NULL;

int *NodeNetwork::test_flags = NULL;
int **NodeNetwork::temp_buffer_flags = NULL;
int ***NodeNetwork::temp_buffer_info = NULL;
int **NodeNetwork::finished_socks = NULL;
int *NodeNetwork::read_socks = NULL;
int NodeNetwork::mode = 0;              // -1 -- non-thread, 0 -- thread
int NodeNetwork::numOfChangedNodes = 0; // number of nodes that has changed modes so far
/************************************************************/

NodeNetwork::NodeNetwork(NodeConfiguration *nodeConfig, std::string privatekey_filename, int num_threads) {
    privatekeyfile = privatekey_filename;
    config = nodeConfig;

    // here number of peers is n-1 instead of n
    int peers = config->getPeerCount();
    // allocate space for prgSeeds
    threshold = peers / 2;
    prgSeeds = new unsigned char *[2 * threshold];
    for (unsigned int i = 0; i < peers; i++) {
        prgSeeds[i] = new unsigned char[KEYSIZE];
        memset(prgSeeds[i], 0, KEYSIZE);
        // print_hexa(prgSeeds[i],KEYSIZE);
    }

    connectToPeers();

    numOfThreads = num_threads; // it should be read from parsing
    int numb = 8 * sizeof(char);
    int temp_buffer_size = MAX_BUFFER_SIZE / (peers + 1) / ((config->getBits() + numb - 1) / numb);

    /************************* MUTEX and COND VAR INITIALIZATION *******************/
    socket_conditional_variables = (pthread_cond_t *)malloc(sizeof(pthread_cond_t) * numOfThreads);
    buffer_conditional_variables = (pthread_cond_t *)malloc(sizeof(pthread_cond_t) * numOfThreads);
    flag_conditional_variables = (pthread_cond_t *)malloc(sizeof(pthread_cond_t) * numOfThreads);

    for (int i = 0; i < numOfThreads; i++) {
        pthread_cond_init(&socket_conditional_variables[i], NULL);
        pthread_cond_init(&buffer_conditional_variables[i], NULL);
        pthread_cond_init(&flag_conditional_variables[i], NULL);
    }

    pthread_cond_init(&manager_conditional_variable, NULL);
    pthread_cond_init(&proceed_conditional_variable, NULL);

    pthread_mutex_init(&socket_mutex, NULL);
    pthread_mutex_init(&buffer_mutex, NULL);

    /********************** FLAGS AND HANDLERS INITIALIZATION *********************/
    temp_buffer_flags = (int **)malloc(sizeof(int *) * (peers + 1));
    temp_buffer_info = (int ***)malloc(sizeof(int **) * (peers + 1));
    finished_socks = (int **)malloc(sizeof(int *) * (peers + 1));
    comm_flags = (int **)malloc(sizeof(int *) * (peers + 1));

    for (int i = 0; i < peers + 1; i++) {
        temp_buffer_flags[i] = (int *)malloc(sizeof(int) * numOfThreads);
        temp_buffer_info[i] = (int **)malloc(sizeof(int *) * numOfThreads);
        finished_socks[i] = (int *)malloc(sizeof(int) * numOfThreads);
        comm_flags[i] = (int *)malloc(sizeof(int) * numOfThreads);
        for (int j = 0; j < numOfThreads; j++) {
            temp_buffer_info[i][j] = (int *)malloc(sizeof(int) * 3);
            temp_buffer_flags[i][j] = comm_flags[i][j] = 0;
            finished_socks[i][j] = 0;
        }
    }

    read_socks = (int *)malloc(sizeof(int) * numOfThreads);
    unit_flags = (int *)malloc(sizeof(int) * numOfThreads);

    for (int i = 0; i < numOfThreads; i++)
        unit_flags[i] = read_socks[i] = 0;

    buffer_handlers = (mpz_t ***)malloc(sizeof(mpz_t **) * (peers + 1));
    temp_buffers = (mpz_t ***)malloc(sizeof(mpz_t **) * (peers + 1));
    for (int i = 0; i < peers + 1; i++) {
        buffer_handlers[i] = (mpz_t **)malloc(sizeof(mpz_t *) * numOfThreads);
        temp_buffers[i] = (mpz_t **)malloc(sizeof(mpz_t *) * numOfThreads);
        for (int j = 0; j < numOfThreads; j++) {
            buffer_handlers[i][j] = NULL;
            temp_buffers[i][j] = (mpz_t *)malloc(sizeof(mpz_t) * temp_buffer_size);
            for (int k = 0; k < temp_buffer_size; k++)
                mpz_init(temp_buffers[i][j][k]);
        }
    }

    test_flags = (int *)malloc(sizeof(int) * numOfThreads);
    for (int i = 0; i < numOfThreads; i++)
        test_flags[i] = 0;

        /*
        ANB, 3/4/24
        - there is a VERY strange "bug", where if the array sizes of the shifts below are declared to be of length sizeof(priv_int) * 8, and the subsequent for loops which populate each element with the corresponding mask
        - if the for loop is upper bounded by sizeof(priv_int) * 8 + 1 (1 larger than allowed), it DOES NOT CAUSE A SEGFAULT
        - e.g. if k = 32, and we declare SHIFT_RSS to be of length 32
        - and the for loop is upper bounded by 32 INCLUSIVE, then we are accessing the following:
            SHIFT_RSS[32]
        - which is out of bounds, and should've ended in a segfault
         */
#if __RSS__
    SHIFT_RSS = new priv_int_t[sizeof(priv_int_t) * 8 + 1];
    for (priv_int_t i = 0; i <= sizeof(priv_int_t) * 8; i++) {
        SHIFT_RSS[i] = (priv_int_t(1) << priv_int_t(i)) - priv_int_t(1); // mod 2^i
        if (i == sizeof(priv_int_t) * 8) {
            SHIFT_RSS[i] = -1;
        }
    }
#endif
}

NodeNetwork::NodeNetwork() {}

NodeNetwork::~NodeNetwork() {
    // these two statements cause the following error:
    // free(): double free detected in tcache 2
    // Aborted (core dumped)
    // why is this the case?
    // i think it has to do with the destructor being declared in the header  as "vritual"
    // delete[] SHIFT_RSS;
    // delete[] SHIFT_64;

    // int peers = config->getPeerCount();
    // int threshold = peers / 2;

    // for (uint i = 0; i < threshold; i++) {
    //     delete[] prgSeeds[i];
    // }
    // delete[] prgSeeds;
}

unsigned char **NodeNetwork::getPRGseeds() {
    return prgSeeds;
}

void NodeNetwork::closeAllConnections() {
}

void NodeNetwork::sendDataToPeer(int id, mpz_t *data, int start, int amount, int size) {
    try {
        int read_amount = 0;
        if (start + amount > size)
            read_amount = size - start;
        else
            read_amount = amount;
        int bits = config->getBits();
        int numb = 8 * sizeof(char);
        int unit_size = (bits + numb - 1) / numb;
        int buffer_size = unit_size * read_amount;
        char *buffer = (char *)malloc(sizeof(char) * buffer_size);
        char *pointer = buffer;
        memset(buffer, 0, buffer_size);
        for (int i = start; i < start + read_amount; i++) {
            mpz_export(pointer, NULL, -1, 1, -1, 0, data[i]);
            pointer += unit_size;
        }
        EVP_CIPHER_CTX *en_temp = peer2enlist.find(id)->second;
        unsigned char *encrypted = aes_encrypt(en_temp, (unsigned char *)buffer, &buffer_size);
        sendDataToPeer(id, 1, &buffer_size);
        sendDataToPeer(id, buffer_size, encrypted);
        free(buffer);
        free(encrypted);
    } catch (std::exception &e) {
        std::cout << "An exception (in Send Data To Peer) was caught: " << e.what() << "\n";
    }
}

void NodeNetwork::sendDataToPeer(int id, int size, mpz_t *data) {
    int count = 0, rounds = 0;
    getRounds(size, &count, &rounds);
    for (int k = 0; k <= rounds; k++)
        sendDataToPeer(id, data, k * count, count, size);
}

void NodeNetwork::sendDataToPeer(int id, int size, long long *data) {
    int count = 0, rounds = 0;
    getRounds(size, &count, &rounds);
    mpz_t *data1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init_set_ui(data1[i], data[i]);
    for (int k = 0; k <= rounds; k++)
        sendDataToPeer(id, data1, k * count, count, size);

    // free the memory
    for (int i = 0; i < size; i++)
        mpz_clear(data1[i]);
    free(data1);
}

void NodeNetwork::getDataFromPeer(int id, int size, mpz_t *buffer) {
    int count = 0, rounds = 0;
    getRounds(size, &count, &rounds);
    for (int k = 0; k <= rounds; k++)
        getDataFromPeer(id, buffer, k * count, count, size);
}

void NodeNetwork::getDataFromPeer(int id, int size, long long *buffer) {
    int count = 0, rounds = 0;
    mpz_t *buffer1 = (mpz_t *)malloc(sizeof(mpz_t) * size);
    for (int i = 0; i < size; i++)
        mpz_init(buffer1[i]);
    getRounds(size, &count, &rounds);
    for (int k = 0; k <= rounds; k++)
        getDataFromPeer(id, buffer1, k * count, count, size);

    for (int i = 0; i < size; i++) {
        buffer[i] = mpz_get_ui(buffer1[i]);
    }
    for (int i = 0; i < size; i++)
        mpz_clear(buffer1[i]);
    free(buffer1);
}

void NodeNetwork::sendDataToPeer(int id, int size, unsigned char *data) {
    try {
        // int on = 1;
        unsigned char *p = data;
        int bytes_read = sizeof(unsigned char) * size;
        int sockfd = peer2sock.find(id)->second;
        fd_set fds;
        while (bytes_read > 0) {
            int bytes_written = send(sockfd, p, sizeof(unsigned char) * bytes_read, MSG_DONTWAIT);
            if (bytes_written < 0) {
                FD_ZERO(&fds);
                FD_SET(sockfd, &fds);
                int n = select(sockfd + 1, NULL, &fds, NULL, NULL);
                if (n > 0)
                    continue;
            } else {
                bytes_read -= bytes_written;
                p += bytes_written;
            }
        }
    } catch (std::exception &e) {
        std::cout << "An exception (in Send Data To Peer) was caught: " << e.what() << "\n";
    }
}

void NodeNetwork::sendDataToPeer(int id, int size, int *data) {
    try {
        int *p = data;
        int bytes_read = sizeof(int) * size;
        int sockfd = peer2sock.find(id)->second;
        fd_set fds;
        while (bytes_read > 0) {
            int bytes_written = send(sockfd, p, bytes_read, MSG_DONTWAIT);
            if (bytes_written < 0) {
                FD_ZERO(&fds);
                FD_SET(sockfd, &fds);
                int n = select(sockfd + 1, NULL, &fds, NULL, NULL);
                if (n > 0)
                    continue;
            } else {
                bytes_read -= bytes_written;
                p += (bytes_written / sizeof(int));
            }
        }
    } catch (std::exception &e) {
        std::cout << "An exception (in Send Data To Peer) was caught: " << e.what() << "\n";
    }
}

void NodeNetwork::getDataFromPeer(int id, int size, int *buffer) {
    try {
        int length = 0, bytes = 0;
        int *tmp_buffer = (int *)malloc(sizeof(int) * size);
        fd_set fds;
        std::map<int, int>::iterator it;
        int sockfd = peer2sock.find(id)->second;
        while (length < sizeof(int) * size) {
            bytes = recv(sockfd, tmp_buffer, sizeof(int) * (size - length / sizeof(int)), MSG_DONTWAIT);
            if (bytes < 0) {
                FD_ZERO(&fds);
                FD_SET(sockfd, &fds);
                int n = select(sockfd + 1, &fds, NULL, NULL, NULL);
                if (n > 0)
                    continue;
            } else {
                memcpy(&buffer[length / sizeof(int)], tmp_buffer, bytes);
                length += bytes;
            }
        }
        free(tmp_buffer);
    } catch (std::exception &e) {
        std::cout << "An exception (get Data From Peer) was caught: " << e.what() << "\n";
    }
}

void NodeNetwork::getDataFromPeer(int id, int size, unsigned char *buffer) {
    try {
        int length = 0, bytes = 0;
        unsigned char *tmp_buffer = (unsigned char *)malloc(sizeof(unsigned char) * size);
        fd_set fds;
        int sockfd = peer2sock.find(id)->second;
        while (length < sizeof(unsigned char) * size) {
            bytes = recv(sockfd, tmp_buffer, sizeof(unsigned char) * (size - length / sizeof(unsigned char)), MSG_DONTWAIT);
            if (bytes < 0) {
                FD_ZERO(&fds);
                FD_SET(sockfd, &fds);
                int n = select(sockfd + 1, &fds, NULL, NULL, NULL);
                if (n > 0)
                    continue;
            } else {
                memcpy(&buffer[length / sizeof(unsigned char)], tmp_buffer, bytes);
                length += bytes;
            }
        }
        free(tmp_buffer);
    } catch (std::exception &e) {
        std::cout << "An exception (get Data From Peer) was caught: " << e.what() << "\n";
    }
}

void NodeNetwork::getDataFromPeer(int id, mpz_t *data, int start, int amount, int size) {
    try {
        int write_amount = 0;
        if (start + amount > size)
            write_amount = size - start;
        else
            write_amount = amount;

        int bits = config->getBits();
        int numb = 8 * sizeof(char);
        int unit_size = (bits + numb - 1) / numb;
        int length;
        getDataFromPeer(id, 1, &length);
        char *buffer = (char *)malloc(sizeof(char) * length);
        getDataFromPeer(id, length, (unsigned char *)buffer);
        EVP_CIPHER_CTX *de_temp = peer2delist.find(id)->second;
        char *decrypted = (char *)aes_decrypt(de_temp, (unsigned char *)buffer, &length);
        // char *tmp = decrypted;
        for (int i = start; i < start + write_amount; i++) {
            mpz_import(data[i], unit_size, -1, 1, -1, 0, decrypted);
            decrypted += unit_size;
        }
        free(buffer);
        // free(tmp);
    } catch (std::exception &e) {
        std::cout << "An exception (get Data From Peer) was caught: " << e.what() << "\n";
    }
}

/*
    This function attempts to write size amount of bytes to the peer.
    If the size is greater than the allowed packet size, then the amount that's sent goes to what we can do.
    The function returns how much was sent, and must be called multiple times until the entire length is written.

    It is recommended to keep a pointer of what's been sent so far, and sending the remaining bytes as the size.
    This is to prevent the blocking of sending and receiving, optimizing what can be done in a period of time. 
*/
int NodeNetwork::sendDataToPeer_NoBlock(int id, int size, unsigned char *buffer) {
    //Clamp the value between 0 and MAX_BUFFER_SIZE
    if (size < 1) {
        return 0;
    }
    if (size > MAX_BUFFER_SIZE) {
        size = MAX_BUFFER_SIZE;
    }

    //Find corresponding socket and write to it
    int sockfd = peer2sock.find(id)->second;
    
    //If written returns -1, this means EAGAIN or EWOULDBLOCK.
    //  This is handled within multicast, which goes on to doing something else in the meantime.
    return send(sockfd, buffer, size, MSG_DONTWAIT);
}




/*
    The size of the data is already known to begin with since we're the ones sending it.
    This results in the same idea from the above sendDataToPeer, where it is advised to keep
        a pointer of what has been read up to.
*/
int NodeNetwork::getDataFromPeer_NoBlock(int id, int size, unsigned char *buffer) {
    if (size < 1) {
        return 0;
    }
    if (size > MAX_BUFFER_SIZE) {
        size = MAX_BUFFER_SIZE;
    }

    int sockfd = peer2sock.find(id)->second;
    return recv(sockfd, buffer, size * 5, MSG_DONTWAIT);
}

/* unlike what the name suggests, this function sends different data to each peer and receives different data from each peer */
void NodeNetwork::multicastToPeers(long long **srcBuffer, long long **desBuffer, int size) {
    int peers = config->getPeerCount();
    mpz_t **buffer = (mpz_t **)malloc(sizeof(mpz_t *) * (peers + 1));
    mpz_t **data = (mpz_t **)malloc(sizeof(mpz_t *) * (peers + 1));
    // int sendIdx = 0, getIdx = 0;
    for (int i = 0; i < peers + 1; i++) {
        buffer[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        data[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++) {
            mpz_init(buffer[i][j]);
            mpz_init_set_ui(data[i][j], srcBuffer[i][j]);
        }
    }

    multicastToPeers(data, buffer, size);
    for (int i = 0; i <= peers; i++)
        for (int j = 0; j < size; j++)
            desBuffer[i][j] = mpz_get_ui(buffer[i][j]);
    for (int i = 0; i <= peers; i++) {
        free(buffer[i]);
        free(data[i]);
    }
}

/* the function sends different data to each peer and receives data from each peer */
void NodeNetwork::multicastToPeers(mpz_t **data, mpz_t **buffers, int size) {
    int id = getID();
    int peers = config->getPeerCount();
    // struct timeval tv1, tv2;

    // int sendIdx = 0, getIdx = 0;
    // compute the maximum size of data that can be communicated
    int count = 0, rounds = 0;
    getRounds(size, &count, &rounds);
    for (int i = 1; i <= peers + 1; i++) {
        if (id == i) {
            for (int k = 0; k <= rounds; k++) {
                for (int j = 1; j <= peers + 1; j++) {
                    if (id == j)
                        continue;
                    sendDataToPeer(j, data[j - 1], k * count, count, size);
                }
                for (int j = 1; j <= peers + 1; j++) {
                    if (id == j)
                        continue;
                    getDataFromPeer(j, buffers[j - 1], k * count, count, size);
                }
            }
        }
    }
    for (int i = 0; i < size; i++)
        mpz_set(buffers[id - 1][i], data[id - 1][i]);
}

/* this function sends identical data (stored in variable 'data') to all other peers and receives data from all of them as well (stored in variable 'buffers') */
void NodeNetwork::broadcastToPeers(mpz_t *data, int size, mpz_t **buffers) {
    int id = getID();
    int peers = config->getPeerCount();

    int rounds = 0, count = 0;
    getRounds(size, &count, &rounds);
    for (int i = 1; i <= peers + 1; i++) {
        if (id == i) {
            for (int k = 0; k <= rounds; k++) {
                for (int j = 1; j <= peers + 1; j++) {
                    if (id == j)
                        continue;
                    sendDataToPeer(j, data, k * count, count, size);
                }
                for (int j = 1; j <= peers + 1; j++) {
                    if (id == j)
                        continue;
                    getDataFromPeer(j, buffers[j - 1], k * count, count, size);
                }
            }
            for (int j = 0; j < size; j++)
                mpz_set(buffers[id - 1][j], data[j]);
        }
    }
}

/* this function sends identical data to all other peers and receives data from all of them */
void NodeNetwork::broadcastToPeers(long long *data, int size, long long **result) {
    // int id = getID();
    int peers = config->getPeerCount();
    mpz_t **buffers = (mpz_t **)malloc(sizeof(mpz_t *) * (peers + 1));
    mpz_t *data1 = (mpz_t *)malloc(size * sizeof(mpz_t));

    for (int i = 0; i < peers + 1; i++) {
        buffers[i] = (mpz_t *)malloc(sizeof(mpz_t) * size);
        for (int j = 0; j < size; j++)
            mpz_init(buffers[i][j]);
    }

    for (int i = 0; i < size; i++)
        mpz_init_set_ui(data1[i], data[i]);
    broadcastToPeers(data1, size, buffers);
    for (int i = 0; i <= peers; i++)
        for (int j = 0; j < size; j++)
            result[i][j] = mpz_get_ui(buffers[i][j]);

    for (int i = 0; i <= peers; i++)
        free(buffers[i]);
}

void NodeNetwork::connectToPeers() {
    int peers = config->getPeerCount();
    for (int i = 1; i <= peers + 1; i++) {
        if (config->getID() == i) {
            if (i != (peers + 1))
                requestConnection(peers + 1 - i);
            if (i != 1)
                acceptPeers(i - 1);
        }
    }
    // pthread_create(&manager, NULL, &managerWorkHelper, this); // something in here causes a segfault (not properly reported unless running GDB, the program makes it look like the segfault occurs in the constructor)
}
void NodeNetwork::launchManager() {
    pthread_create(&manager, NULL, &managerWorkHelper, this); // something in here causes a segfault (not properly reported unless running GDB, the program makes it look like the segfault occurs in the constructor)
}

void *NodeNetwork::managerWorkHelper(void *net) {
    ((NodeNetwork *)net)->managerWork();
    return 0;
}

void *NodeNetwork::managerWork() {
    printf("Starting thread manager...\n");
    fd_set socketDescriptorSet;
    int fdmax = 0, nReady = 0, index = 0, threadID;
    int peers = config->getPeerCount();
    FD_ZERO(&socketDescriptorSet);
    std::map<int, int>::iterator it;

    int *socks = (int *)malloc(sizeof(int) * peers);
    int *nodes = (int *)malloc(sizeof(int) * peers);
    for (it = peer2sock.begin(); it != peer2sock.end(); ++it) {
        nodes[index] = it->first;
        socks[index] = it->second;
        index++;
    }

    while (1) {
        for (int i = 0; i < peers; i++) {
            FD_SET(socks[i], &socketDescriptorSet);
            if (socks[i] > fdmax)
                fdmax = socks[i];
        }
        if ((nReady = select(fdmax + 1, &socketDescriptorSet, NULL, NULL, NULL)) == -1) {
            printf("Select Error\n");
            return 0;
        }
        for (int i = 0; i < peers; i++) {
            if (FD_ISSET(socks[i], &socketDescriptorSet)) {

                getDataFromPeer(nodes[i], 1, &threadID);

                if (threadID == -2) {
                    numOfChangedNodes++;
                    if (numOfChangedNodes == peers) {
                        pthread_mutex_lock(&buffer_mutex);
                        pthread_cond_signal(&proceed_conditional_variable);
                        pthread_cond_wait(&manager_conditional_variable, &buffer_mutex);
                        pthread_mutex_unlock(&buffer_mutex);
                        break;
                    }
                    continue;
                }
                if (finished_socks[nodes[i] - 1][threadID] == 1) {
                    getDataFromPeerToBuffer(socks[i], nodes[i], threadID, &temp_buffer_info[nodes[i] - 1][threadID]);
                    temp_buffer_flags[nodes[i] - 1][threadID] = 1;
                } else {
                    getDataFromPeer(socks[i], nodes[i], threadID);
                    finished_socks[nodes[i] - 1][threadID] = 1;
                    int read_buffered_socks = 1;
                    for (int j = 0; j < peers; j++) {
                        if (finished_socks[nodes[j] - 1][threadID] == 0) {
                            read_buffered_socks = 0;
                            break;
                        }
                    }
                    if (read_buffered_socks) {
                        for (int j = 0; j < peers; j++) {
                            finished_socks[nodes[j] - 1][threadID] = 0;
                            if (temp_buffer_flags[nodes[j] - 1][threadID] == 1) {
                                restoreDataToBuffer(socks[j], nodes[j], threadID, &temp_buffer_info[nodes[j] - 1][threadID]);
                                temp_buffer_flags[nodes[j] - 1][threadID] = 0;
                                finished_socks[nodes[j] - 1][threadID] = 1;
                            }
                        }
                    }
                }
                FD_CLR(socks[i], &socketDescriptorSet);
            }
        }
    }

    free(socks);
    free(nodes);
    return 0;
}

void NodeNetwork::getDataFromPeerToBuffer(int socketID, int peerID, int threadID, int **info) {
    int write_amount = 0;
    int *data = (int *)malloc(sizeof(int) * 3);
    getDataFromPeer(peerID, 3, data);
    (*info)[0] = data[0];
    (*info)[1] = data[1];
    (*info)[2] = data[2];
    if (data[0] + data[1] > data[2])
        write_amount = data[2] - data[0];
    else
        write_amount = data[1];
    getDataFromPeer(peerID, temp_buffers[peerID - 1][threadID], 0, write_amount, data[1]);
    free(data);
    return;
}

void NodeNetwork::restoreDataToBuffer(int socketID, int peerID, int threadID, int **info) {
    // int id = config->getID();
    int start = (*info)[0], amount = (*info)[1], size = (*info)[2];
    pthread_mutex_lock(&buffer_mutex);
    while (comm_flags[peerID - 1][threadID] == 0) {
        pthread_cond_wait(&buffer_conditional_variables[threadID], &buffer_mutex);
    }
    pthread_mutex_unlock(&buffer_mutex);
    int write_amount = 0;
    if (start + amount >= size) {
        write_amount = size - start;
        comm_flags[peerID - 1][threadID] = 0;
    } else
        write_amount = amount;

    for (int i = start; i < start + write_amount; i++) {
        mpz_set(buffer_handlers[peerID - 1][threadID][i], temp_buffers[peerID - 1][threadID][i - start]);
    }

    read_socks[threadID]++;
    if (read_socks[threadID] == config->getPeerCount()) {
        pthread_mutex_lock(&socket_mutex);
        while (unit_flags[threadID] == 1)
            pthread_cond_wait(&flag_conditional_variables[threadID], &socket_mutex);
        unit_flags[threadID] = 1;
        pthread_cond_signal(&socket_conditional_variables[threadID]);
        pthread_mutex_unlock(&socket_mutex);
        read_socks[threadID] = 0;
    }

    return;
}

void NodeNetwork::getDataFromPeer(int socketID, int peerID, int threadID) {
    // int id = config->getID();
    int *info = (int *)malloc(sizeof(int) * 3);
    getDataFromPeer(peerID, 3, info);
    pthread_mutex_lock(&buffer_mutex);
    while (comm_flags[peerID - 1][threadID] == 0) {
        pthread_cond_wait(&buffer_conditional_variables[threadID], &buffer_mutex);
    }
    pthread_mutex_unlock(&buffer_mutex);
    getDataFromPeer(peerID, buffer_handlers[peerID - 1][threadID], info[0], info[1], info[2]);

    if (info[0] + info[1] >= info[2]) {
        comm_flags[peerID - 1][threadID] = 0;
    }
    read_socks[threadID]++;
    if (read_socks[threadID] == config->getPeerCount()) {
        pthread_mutex_lock(&socket_mutex);
        while (unit_flags[threadID] == 1)
            pthread_cond_wait(&flag_conditional_variables[threadID], &socket_mutex);
        unit_flags[threadID] = 1;
        pthread_cond_signal(&socket_conditional_variables[threadID]);
        pthread_mutex_unlock(&socket_mutex);
        read_socks[threadID] = 0;
    }
    free(info);
    return;
}

void NodeNetwork::sendModeToPeers(int id) {
    int peers = config->getPeerCount();
    int msg = -2;
    for (int j = 1; j <= peers + 1; j++) {
        if (id == j)
            continue;
        sendDataToPeer(j, 1, &msg);
    }
    // sleep(1);
}

/* the function sends different data to each peer and receive data from each peer */
void NodeNetwork::multicastToPeers(mpz_t **data, mpz_t **buffers, int size, int threadID) {
    test_flags[threadID]++;
    int id = getID();
    int peers = config->getPeerCount();
    // struct timeval tv1, tv2;
    if (size == 0)
        return;
    if (threadID == -1) {
        if (mode != -1) {
            sendModeToPeers(id);
            pthread_mutex_lock(&buffer_mutex);
            while (numOfChangedNodes < peers)
                pthread_cond_wait(&proceed_conditional_variable, &buffer_mutex);
            pthread_mutex_unlock(&buffer_mutex);
            numOfChangedNodes = 0;
            mode = -1;
        }
        multicastToPeers(data, buffers, size);
        return;
    } else {
        if (mode != 0) {
            pthread_mutex_lock(&buffer_mutex);
            pthread_cond_signal(&manager_conditional_variable);
            pthread_mutex_unlock(&buffer_mutex);
            mode = 0;
        }
    }

    // int sendIdx = 0, getIdx = 0;
    int count = 0, rounds = 0;
    getRounds(size, &count, &rounds);
    for (int i = 0; i <= peers; i++)
        buffer_handlers[i][threadID] = buffers[i];

    pthread_mutex_lock(&buffer_mutex);
    for (int i = 0; i <= peers; i++)
        comm_flags[i][threadID] = 1;
    pthread_cond_signal(&buffer_conditional_variables[threadID]);
    // gettimeofday(&tv1, NULL);
    pthread_mutex_unlock(&buffer_mutex);

    for (int i = 1; i <= peers + 1; i++) {
        if (id == i) {
            for (int k = 0; k <= rounds; k++) {
                for (int j = 1; j <= peers + 1; j++) {
                    if (id == j)
                        continue;
                    pthread_mutex_lock(&socket_mutex);
                    sendDataToPeer(j, data[j - 1], k * count, count, size, threadID);
                    pthread_mutex_unlock(&socket_mutex);
                }
                pthread_mutex_lock(&socket_mutex);
                while (unit_flags[threadID] == 0) {
                    // gettimeofday(&tv1, NULL);
                    pthread_cond_wait(&socket_conditional_variables[threadID], &socket_mutex);
                }
                unit_flags[threadID] = 0;
                pthread_cond_signal(&flag_conditional_variables[threadID]);
                pthread_mutex_unlock(&socket_mutex);
            }
        }
    }

    for (int i = 0; i < size; i++)
        mpz_set(buffers[id - 1][i], data[id - 1][i]);
}

/* the function sends the same data to each peer and receives data from each peer */
void NodeNetwork::broadcastToPeers(mpz_t *data, int size, mpz_t **buffers, int threadID) {
    test_flags[threadID]++;
    if (size == 0)
        return;
    int id = getID();
    int peers = config->getPeerCount();
    if (threadID == -1) {
        if (mode != -1) {
            sendModeToPeers(id);
            pthread_mutex_lock(&buffer_mutex);
            while (numOfChangedNodes < peers)
                pthread_cond_wait(&proceed_conditional_variable, &buffer_mutex);
            pthread_mutex_unlock(&buffer_mutex);
            numOfChangedNodes = 0;
            mode = -1;
        }
        broadcastToPeers(data, size, buffers);
        return;
    } else {
        if (mode != 0) {
            pthread_mutex_lock(&buffer_mutex);
            pthread_cond_signal(&manager_conditional_variable);
            pthread_mutex_unlock(&buffer_mutex);
            mode = 0;
        }
    }
    // int sendIdx = 0, getIdx = 0;
    int rounds = 0, count = 0;
    getRounds(size, &count, &rounds);
    for (int i = 0; i <= peers; i++)
        buffer_handlers[i][threadID] = buffers[i];

    pthread_mutex_lock(&buffer_mutex);
    for (int i = 0; i <= peers; i++)
        comm_flags[i][threadID] = 1;
    pthread_cond_signal(&buffer_conditional_variables[threadID]);
    pthread_mutex_unlock(&buffer_mutex);
    for (int i = 1; i <= peers + 1; i++) {
        if (id == i) {
            for (int k = 0; k <= rounds; k++) {
                for (int j = 1; j <= peers + 1; j++) {
                    if (id == j)
                        continue;
                    pthread_mutex_lock(&socket_mutex);
                    sendDataToPeer(j, data, k * count, count, size, threadID);
                    pthread_mutex_unlock(&socket_mutex);
                }
                pthread_mutex_lock(&socket_mutex);
                while (unit_flags[threadID] == 0) {
                    pthread_cond_wait(&socket_conditional_variables[threadID], &socket_mutex);
                }
                unit_flags[threadID] = 0;
                pthread_cond_signal(&flag_conditional_variables[threadID]);
                pthread_mutex_unlock(&socket_mutex);
            }
            for (int j = 0; j < size; j++)
                mpz_set(buffers[id - 1][j], data[j]);
        }
    }
}

void NodeNetwork::sendDataToPeer(int id, mpz_t *data, int start, int amount, int size, int threadID) {
    try {
        int read_amount = 0;
        if (start + amount > size)
            read_amount = size - start;
        else
            read_amount = amount;
        int bits = config->getBits();
        int numb = 8 * sizeof(char);
        int unit_size = (bits + numb - 1) / numb;
        int buffer_size = unit_size * read_amount;
        int *info = (int *)malloc(sizeof(int) * 3);
        info[0] = start;
        info[1] = amount;
        info[2] = size;

        char *buffer = (char *)malloc(sizeof(char) * buffer_size);
        char *pointer = buffer;
        memset(buffer, 0, buffer_size);
        for (int i = start; i < start + read_amount; i++) {
            mpz_export(pointer, NULL, -1, 1, -1, 0, data[i]);
            pointer += unit_size;
        }

        EVP_CIPHER_CTX *en_temp = peer2enlist.find(id)->second;
        unsigned char *encrypted = aes_encrypt(en_temp, (unsigned char *)buffer, &buffer_size);
        sendDataToPeer(id, 1, &threadID);
        sendDataToPeer(id, 3, info);
        sendDataToPeer(id, 1, &buffer_size);
        sendDataToPeer(id, buffer_size, encrypted);
        free(buffer);
        free(info);
        // free(encrypted);
    } catch (std::exception &e) {
        std::cout << "An exception (in Send Data To Peer) was caught: " << e.what() << "\n";
    }
}

void NodeNetwork::requestConnection(int numOfPeers) {
    // key/iv for secure communication, plus key for seed
    peerKeyIV = (unsigned char *)malloc(2 * KEYSIZE + AES_BLOCK_SIZE);
    int *sockfd = (int *)malloc(sizeof(int) * numOfPeers);
    int *portno = (int *)malloc(sizeof(int) * numOfPeers);
    struct sockaddr_in *serv_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in) * numOfPeers);
    struct hostent **server = (struct hostent **)malloc(sizeof(struct hostent *) * numOfPeers);
    int on = 1;
    int ID;
    try {
        for (int i = 0; i < numOfPeers; i++) {
            int num_tries = 0;
            ID = config->getID() + i + 1;
            printf("Attempting to connect to node %d...\n", ID);
            portno[i] = config->getPeerPort(ID);
            sockfd[i] = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
            if (sockfd[i] < 0)
                throw std::runtime_error("opening socket");
            // the function below might not work in certain
            // configurations, e.g., running all nodes from the
            // same VM. it is not used for single-threaded programs
            // and thus be commented out or replaced with an
            // equivalent function otherwise.
            // fcntl(sockfd[i], F_SETFL, O_NONBLOCK);
            // int rc = setsockopt(sockfd[i], SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
            if (setsockopt(sockfd[i], SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0)
                throw std::runtime_error("setsockopt(SO_REUSEADDR)");
            if (setsockopt(sockfd[i], IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on)) < 0)
                throw std::runtime_error("setsockopt(IPPROTO_TCP)");
            server[i] = gethostbyname((config->getPeerIP(ID)).c_str());
            if (server[i] == NULL)
                throw std::runtime_error("no such host with IP address " + config->getPeerIP(ID));
            bzero((char *)&serv_addr[i], sizeof(serv_addr[i]));
            serv_addr[i].sin_family = AF_INET;
            bcopy((char *)server[i]->h_addr, (char *)&serv_addr[i].sin_addr.s_addr, server[i]->h_length);
            serv_addr[i].sin_port = htons(portno[i]);
            while (true) {
                // int valopt = -1;
                // fd_set myset;
                // struct timeval tv;
                // socklen_t lon;
                int res = connect(sockfd[i], (struct sockaddr *)&serv_addr[i], sizeof(serv_addr[i]));
                // printf("res : %i\n", res);
                if (res < 0) {
                    if (num_tries < MAX_RETRIES) {
                        // cross platform version of sleep(), C++14 and onward
                        std::this_thread::sleep_for(WAIT_INTERVAL);
                        num_tries += 1;
                    } else {
                        throw std::runtime_error("timeout, error connecting " + std::to_string(errno) + " - " + strerror(errno));
                    }

                    // this block of code only is only executed if connect() fails
                    // if (errno == EINPROGRESS) {
                    //     tv.tv_sec = 15;
                    //     tv.tv_usec = 0;
                    //     FD_ZERO(&myset);
                    //     FD_SET(sockfd[i], &myset);
                    //     if (select(sockfd[i] + 1, NULL, &myset, NULL, &tv) > 0) {
                    //         lon = sizeof(int);
                    //         getsockopt(sockfd[i], SOL_SOCKET, SO_ERROR, (void *)(&valopt), &lon);
                    //         if (valopt)
                    //             throw std::runtime_error("Error in connection() " + std::to_string(valopt) + " - " + strerror(valopt));
                    //     } else {
                    //         throw std::runtime_error("select error " + std::to_string(valopt) + " - " + strerror(valopt));
                    //     }
                    // } else {
                    //     throw std::runtime_error("Error connecting " + std::to_string(errno) + " - " + strerror(errno));
                    // }
                } else {
                    break; // connection was successful!
                    // connected = true;
                }
            }

            printf("Successfully connected to node %d\n", ID);
            peer2sock.insert(std::pair<int, int>(ID, sockfd[i]));
            sock2peer.insert(std::pair<int, int>(sockfd[i], ID));
#if __DEPLOYMENT__
            FILE *prikeyfp = fopen(privatekeyfile.c_str(), "r");
            if (prikeyfp == NULL)
                throw std::runtime_error("Can't open private key " + privatekeyfile);
            RSA *priRkey = PEM_read_RSAPrivateKey(prikeyfp, NULL, NULL, NULL);
            if (priRkey == NULL)
                throw std::runtime_error("Read Private Key for RSA");
            char *buffer = (char *)malloc(RSA_size(priRkey));
            int n = read(sockfd[i], buffer, RSA_size(priRkey));
            if (n < 0)
                throw std::runtime_error("reading from socket");
            char *decrypt = (char *)malloc(n);
            memset(decrypt, 0x00, n);
            int dec_len = RSA_private_decrypt(n, (unsigned char *)buffer, (unsigned char *)decrypt, priRkey, RSA_PKCS1_OAEP_PADDING);
            if (dec_len < 1)
                throw std::runtime_error("RSA private decrypt");
            free(buffer);
            fclose(prikeyfp);

#else
            char *decrypt = (char *)malloc(2 * KEYSIZE + AES_BLOCK_SIZE);
            // check that this is the number of bytes that are supposed to be read from the socket
            if (read(sockfd[i], decrypt, 2 * KEYSIZE + AES_BLOCK_SIZE) < 0)
                throw std::runtime_error("reading from socket");
#endif

            memcpy(peerKeyIV, decrypt, 2 * KEYSIZE + AES_BLOCK_SIZE);
            // print_hexa(peerKeyIV, 2 * KEYSIZE + AES_BLOCK_SIZE);
            init_keys(ID, 1);
            free(decrypt);
        }
    } catch (const std::runtime_error &ex) {
        std::cerr << "[requestConnection] " << ex.what() << "\n";
        exit(1);
    }
}

void NodeNetwork::acceptPeers(int numOfPeers) {
    // key/iv for secure communication, plus key for seed
    KeyIV = (unsigned char *)malloc(2 * KEYSIZE + AES_BLOCK_SIZE);
    int sockfd, maxsd, portno, on = 1;
    int *newsockfd = (int *)malloc(sizeof(int) * numOfPeers);
    socklen_t *clilen = (socklen_t *)malloc(sizeof(socklen_t) * numOfPeers);
    struct sockaddr_in serv_addr;
    struct sockaddr_in *cli_addr = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in) * numOfPeers);
    try {
        fd_set master_set, working_set;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        // see comment for fcntl above
        // fcntl(sockfd, F_SETFL, O_NONBLOCK);
        if (sockfd < 0)
            throw std::runtime_error("error opening socket");
        int rc = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
        rc = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on));
        if (rc < 0)
            throw std::runtime_error("setsockopt() or ioctl() failed");
        bzero((char *)&serv_addr, sizeof(serv_addr));
        portno = config->getPort();
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);
        if ((bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
            throw std::runtime_error("error on binding to socket, try again with a different port number. Port used: " + std::to_string(portno));
        }
        listen(sockfd, 7);
        // start to accept connections
        FD_ZERO(&master_set);
        maxsd = sockfd;
        FD_SET(sockfd, &master_set);
        for (int i = 0; i < numOfPeers; i++) {
            memcpy(&working_set, &master_set, sizeof(master_set));
            rc = select(maxsd + 1, &working_set, NULL, NULL, NULL);
            if (rc <= 0) {
                throw std::runtime_error("select failed or time out");
            }
            if (FD_ISSET(sockfd, &working_set)) {
                clilen[i] = sizeof(cli_addr[i]);
                newsockfd[i] = accept(sockfd, (struct sockaddr *)&cli_addr[i], &clilen[i]);
                if (newsockfd[i] < 0) {
                    throw std::runtime_error("ERROR, on accept");
                }
                // see comment for fcntl above
                // fcntl(newsockfd[i], F_SETFL, O_NONBLOCK);
                peer2sock.insert(std::pair<int, int>(config->getID() - (i + 1), newsockfd[i]));
                sock2peer.insert(std::pair<int, int>(newsockfd[i], config->getID() - (i + 1)));

                unsigned char key_iv[2 * KEYSIZE + AES_BLOCK_SIZE];
                RAND_status();
                if (!RAND_bytes(key_iv, 2 * KEYSIZE + AES_BLOCK_SIZE))
                    throw std::runtime_error("ERROR, key, iv generation");
                memcpy(KeyIV, key_iv, 2 * KEYSIZE + AES_BLOCK_SIZE);
                int peer = config->getID() - (i + 1);
#if __DEPLOYMENT__
                FILE *pubkeyfp = fopen((config->getPeerPubKey(peer)).c_str(), "r");
                if (pubkeyfp == NULL)
                    throw std::runtime_error("Can't open public key " + config->getPeerPubKey(peer));
                RSA *publicRkey = PEM_read_RSA_PUBKEY(pubkeyfp, NULL, NULL, NULL);
                if (publicRkey == NULL)
                    throw std::runtime_error("Read public Key for RSA");
                char *encrypt = (char *)malloc(RSA_size(publicRkey));
                memset(encrypt, 0x00, RSA_size(publicRkey));
                int enc_len = RSA_public_encrypt(2 * KEYSIZE + AES_BLOCK_SIZE, KeyIV, (unsigned char *)encrypt, publicRkey, RSA_PKCS1_OAEP_PADDING);
                if (enc_len < 1)
                    throw std::runtime_error("RSA public encrypt error");
                if (write(newsockfd[i], encrypt, enc_len) < 0) // sending to peer
                    throw std::runtime_error("ERROR writing to socket");
                free(encrypt);
                fclose(pubkeyfp);
#else
                if (write(newsockfd[i], KeyIV, 2 * KEYSIZE + AES_BLOCK_SIZE) < 0)
                    throw std::runtime_error("ERROR writing to socket");
#endif
                init_keys(peer, 0);
            }
        }
    } catch (const std::runtime_error &ex) {
        std::cerr << "[acceptPeers] " << ex.what() << "\n";
        exit(1);
    }
}

void NodeNetwork::init_keys(int peer, int nRead) {
    unsigned char key[KEYSIZE], iv[AES_BLOCK_SIZE], prg_seed_key[KEYSIZE];
    memset(key, 0x00, KEYSIZE);
    memset(iv, 0x00, AES_BLOCK_SIZE);
    memset(prg_seed_key, 0x00, KEYSIZE);

    if (0 == nRead) {
        // useKey KeyIV
        memcpy(key, KeyIV, KEYSIZE);
        memcpy(iv, KeyIV + KEYSIZE, AES_BLOCK_SIZE);
        memcpy(prg_seed_key, KeyIV + KEYSIZE + AES_BLOCK_SIZE, KEYSIZE);
    } else {
        // getKey from peers
        memcpy(key, peerKeyIV, KEYSIZE);
        memcpy(iv, peerKeyIV + KEYSIZE, AES_BLOCK_SIZE);
        memcpy(prg_seed_key, peerKeyIV + KEYSIZE + AES_BLOCK_SIZE, KEYSIZE);
    }

    // populate the prgSeeds array which stores keys for parties myid-threshold through myid-1
    int peers = config->getPeerCount();
    // variable peers here stores a DIFFERENT value from peers elsewhere - this corresponds to the n-1 parties
    int threshold = peers / 2;
    int myid = getID();
    int index = myid - peer;
    if (index < 0) {
        index = index + peers + 1;
    }

    if ((1 <= index) and (index <= threshold)) {
        memcpy(prgSeeds[threshold - index], prg_seed_key, KEYSIZE);
    } else {
        memcpy(prgSeeds[threshold + peers - index], prg_seed_key, KEYSIZE);
    }

    en = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(en, EVP_aes_128_cbc(), NULL, key, iv);
    de = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(de, EVP_aes_128_cbc(), NULL, key, iv);
    peer2enlist.insert(std::pair<int, EVP_CIPHER_CTX *>(peer, en));
    peer2delist.insert(std::pair<int, EVP_CIPHER_CTX *>(peer, de));
}

// void NodeNetwork::mpzFromString(char *str, mpz_t *no, int *lengths, int size) {
//     for (int i = 0; i < size; i++) {
//         char temp[lengths[i]]; // = (char*)malloc(sizeof(char) * lengths[i]);
//         memcpy(temp, str, lengths[i]);
//         temp[lengths[i]] = '\0';
//         str += lengths[i];
//         mpz_set_str(no[i], temp, BASE_36);
//     }
// }

void NodeNetwork::getRounds(int size, int *count, int *rounds) {
    int bits = config->getBits();
    int peers = config->getPeerCount();
    int numb = 8 * sizeof(char);
    int unit_size = (bits + numb - 1) / numb;
    *count = MAX_BUFFER_SIZE / (peers + 1) / unit_size;
    if (size % (*count) != 0)
        *rounds = size / (*count);
    else
        *rounds = size / (*count) - 1;
}

double NodeNetwork::time_diff(struct timeval *t1, struct timeval *t2) {
    double elapsed;
    if (t1->tv_usec > t2->tv_usec) {
        t2->tv_usec += 1000000;
        t2->tv_sec--;
    }
    elapsed = (t2->tv_sec - t1->tv_sec) + (t2->tv_usec - t1->tv_usec) / 1000000.0;
    return (elapsed);
}

int NodeNetwork::getID() {
    return config->getID();
}

int NodeNetwork::getNumOfThreads() {
    return numOfThreads;
}

unsigned char *NodeNetwork::aes_encrypt(EVP_CIPHER_CTX *e, unsigned char *plaintext, int *len) {
    int c_len = *len + AES_BLOCK_SIZE;
    int f_len = 0;
    unsigned char *ciphertext = (unsigned char *)malloc(c_len);

    EVP_EncryptInit_ex(e, NULL, NULL, NULL, NULL);
    EVP_EncryptUpdate(e, ciphertext, &c_len, plaintext, *len);
    EVP_EncryptFinal_ex(e, ciphertext + c_len, &f_len);
    *len = c_len + f_len;
    return ciphertext;
}

unsigned char *NodeNetwork::aes_decrypt(EVP_CIPHER_CTX *e, unsigned char *ciphertext, int *len) {
    int p_len = *len;
    int f_len = 0;
    unsigned char *plaintext = (unsigned char *)malloc(p_len + AES_BLOCK_SIZE);

    EVP_DecryptInit_ex(e, NULL, NULL, NULL, NULL);
    EVP_DecryptUpdate(e, plaintext, &p_len, ciphertext, *len);
    EVP_DecryptFinal_ex(e, plaintext + p_len, &f_len);
    *len = p_len + f_len;
    return plaintext;
}

void NodeNetwork::multicastToPeers_Mult(uint *sendtoIDs, uint *RecvFromIDs, mpz_t **data, int size) {
    // compute the maximum size of data that can be communicated
    int count = 0, rounds = 0;
    int idx = 0; 
    getRounds(size, &count, &rounds);
    for (uint k = 0; k <= rounds; k++) {
        for (uint i = 0; i < threshold; i++) {
            sendDataToPeer(sendtoIDs[i], data[i], k * count, count, size);
        }
        for (uint i = 0; i < threshold; i++) {
            idx = threshold - i - 1;
            getDataFromPeer(RecvFromIDs[idx], data[2 * threshold - i], k * count, count, size);
        }
    }
}

// why is this needed for send/recv to work?
void NodeNetwork::multicastToPeers_Mult(uint *sendtoIDs, uint *RecvFromIDs, mpz_t **data, int size, int threadID) {
    test_flags[threadID]++;
    int id = getID();
    int peers = config->getPeerCount();
    if (size == 0)
        return;
    if (threadID == -1) {
        if (mode != -1) {
            sendModeToPeers(id);
            pthread_mutex_lock(&buffer_mutex);
            while (numOfChangedNodes < peers)
                pthread_cond_wait(&proceed_conditional_variable, &buffer_mutex);
            pthread_mutex_unlock(&buffer_mutex);
            numOfChangedNodes = 0;
            mode = -1;
        }
        multicastToPeers_Mult(sendtoIDs, RecvFromIDs, data, size);
        return;
    } else {
        if (mode != 0) {
            pthread_mutex_lock(&buffer_mutex);
            pthread_cond_signal(&manager_conditional_variable);
            pthread_mutex_unlock(&buffer_mutex);
            mode = 0;
        }
    }

    // this needs to be implemented such that multithreading can function for Shamir SS
    // int count = 0, rounds = 0;
    // getRounds(size, &count, &rounds);
    // for (int i = 0; i <= peers; i++)
    //     buffer_handlers[i][threadID] = buffers[i];

    // pthread_mutex_lock(&buffer_mutex);
    // for (int i = 0; i <= peers; i++)
    //     comm_flags[i][threadID] = 1;
    // pthread_cond_signal(&buffer_conditional_variables[threadID]);
    // // gettimeofday(&tv1, NULL);
    // pthread_mutex_unlock(&buffer_mutex);

    // for (int i = 1; i <= peers + 1; i++) {
    //     if (id == i) {
    //         for (int k = 0; k <= rounds; k++) {
    //             for (int j = 1; j <= peers + 1; j++) {
    //                 if (id == j)
    //                     continue;
    //                 pthread_mutex_lock(&socket_mutex);
    //                 sendDataToPeer(j, data[j - 1], k * count, count, size, threadID);
    //                 pthread_mutex_unlock(&socket_mutex);
    //             }
    //             pthread_mutex_lock(&socket_mutex);
    //             while (unit_flags[threadID] == 0) {
    //                 // gettimeofday(&tv1, NULL);
    //                 pthread_cond_wait(&socket_conditional_variables[threadID], &socket_mutex);
    //             }
    //             unit_flags[threadID] = 0;
    //             pthread_cond_signal(&flag_conditional_variables[threadID]);
    //             pthread_mutex_unlock(&socket_mutex);
    //         }
    //     }
    // }

    // for (int i = 0; i < size; i++)
    //     mpz_set(buffers[id - 1][i], data[id - 1][i]);
    
}

// sends one piece of data to recvFromIds
void NodeNetwork::multicastToPeers_Open(uint *sendtoIDs, uint *RecvFromIDs, mpz_t *data, mpz_t **buffer, int size, int threadID) {
    std::cout << "multicast threadID : "<<threadID << std::endl;
    test_flags[threadID]++;
    int id = getID();
    int peers = config->getPeerCount();
    if (size == 0)
        return;
    if (threadID == -1) {
        if (mode != -1) {
            sendModeToPeers(id);
            pthread_mutex_lock(&buffer_mutex);
            while (numOfChangedNodes < peers)
                pthread_cond_wait(&proceed_conditional_variable, &buffer_mutex);
            pthread_mutex_unlock(&buffer_mutex);
            numOfChangedNodes = 0;
            mode = -1;
        }
        multicastToPeers_Open(sendtoIDs, RecvFromIDs, data, buffer, size);
        return;
    } else {
        if (mode != 0) {
            pthread_mutex_lock(&buffer_mutex);
            pthread_cond_signal(&manager_conditional_variable);
            pthread_mutex_unlock(&buffer_mutex);
            mode = 0;
        }
    }
}

/*
    Multicast in this sense means that we are sending different data to different places.
    
    The function uses noblock to achieve less wait times in between each communication.
    The main focus of this is to open the shares, 

*/
void NodeNetwork::multicastToPeers_Open(uint *sendtoIDs, uint *RecvFromIDs, mpz_t *data, mpz_t **buffer, int size) {
    int amount, rounds;
    getRounds(size, &amount, &rounds);

    printf("Size %i -> amount: %i for %i rounds\n", size, amount, rounds);
    if (amount > size)
        amount = size;
    else
        amount = amount;
    int bits = config->getBits();
    int numb = 8 * sizeof(char);
    int unit_size = (bits + numb - 1) / numb;
    int buffer_size = unit_size * amount;


    //For debugging purposes, prints out all the data
    gmp_printf("Data to be sent:\n");
    for (int i = 0; i < size; i++) 
        gmp_printf("[%i] = %Zd\n", i, *data);
    gmp_printf("------------------------------------\n");

    //Loop through each of the IDs, encrypt the data
    //TODO only one encryption needed
    //TODO free
    unsigned char **encryptedData = (unsigned char **) malloc(sizeof(unsigned char **) * (threshold + 1));

    char *encryptedBuf = (char *) malloc(sizeof(char *) * buffer_size);
    
    for (int i = 0; i < threshold; i++) {
        //Clear previous memory
        memset(encryptedBuf, 0, buffer_size);    

        mpz_export(encryptedBuf, NULL, -1, 1, -1, 0, *data);

        EVP_CIPHER_CTX *en_temp = peer2enlist.find(sendtoIDs[i])->second;
        unsigned char *encrypted = aes_encrypt(en_temp, (unsigned char *)encryptedBuf, &buffer_size);
        // gmp_printf("%Zd => %s\n", data, encrypted);

        encryptedData[i] = encrypted;
    }
    free(encryptedBuf);

    //Holds all the data received until we have everything to convert to mpz_t
    unsigned char **tempBuffer = (unsigned char **)malloc(sizeof(unsigned char *) * (threshold + 1));
    for (int i = 0; i < (threshold + 1); i++)
        //Everything gets written over, so no need to calloc
        tempBuffer[i] = (unsigned char *)malloc(sizeof(unsigned char) * size);


    //Holds <index, bytes remaining to...> for respective name
    std::vector<std::pair<int, int>> toWrite;
    std::vector<std::pair<int, int>> toReceive;
    
    for (int i = 0; i < size; i++) {
        toWrite.push_back({i, buffer_size});
        toReceive.push_back({i, buffer_size});
    }
    
    //Only stop looping once we send everything
    //  AND once we receive enough shares completely
    int bytes, i, received = 0;
    while (received < threshold || !toWrite.empty()) {

        //Write if needed
        for (i = 0; i < toWrite.size(); i++) {
            
            //Send it through the noblock to avoid waiting
            bytes = sendDataToPeer_NoBlock(sendtoIDs[i], toWrite[i].second, encryptedData[i]);
            if (bytes > 0) {
                toWrite[i].second -= bytes;

                if (toWrite[i].second <= 0) {
                    toWrite.erase(toWrite.begin() + i);
                }
            }
        }

        //Read if needed
        //  Only need to look at those not received yet
        for (i = 0; i < toReceive.size(); i++) {

            //See if there's anything to read
            bytes = getDataFromPeer_NoBlock(RecvFromIDs[i], toReceive[i].second, tempBuffer[i]);
            
            if (bytes > 0) {
                toReceive[i].second -= bytes;
                
                if (toReceive[i].second <= 0) {
                    received++;
                    toReceive.erase(toReceive.begin() + i);

                    // Now that we have the full data for this part, decrypt it then add it to the buffer
                    EVP_CIPHER_CTX *de_temp = peer2delist.find(RecvFromIDs[i])->second;
                    char *decrypted = (char *)aes_decrypt(de_temp, tempBuffer[i], &buffer_size);

                    //TODO: Verify that this is in the correct position (should be)
                    //TODO: Also discuss batch work, what'd be the best approach 

                    for (int j = 0; j < size; j++)
                        mpz_import(buffer[i][j], unit_size, -1, 1, -1, 0, decrypted);

                    gmp_printf("Got: %Zd\n", buffer[i][0]);
                }
            }
        }
    }

    for (int i = 0; i < (threshold + 1); i++) {
        free(tempBuffer[i]);
        free(encryptedData[i]);
    }
    free(tempBuffer);
    free(encryptedData);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// NEW RSS NETWORKING FUNCTIONALITIES ////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
#if __RSS__

void NodeNetwork::getRounds_RSS(int size, uint *count, uint *rounds, uint ring_size) {
    int peers = config->getPeerCount();
    int unit_size = (ring_size + 7) >> 3;
    *count = MAX_BUFFER_SIZE / (peers + 1) / unit_size;
    if (size % (*count) != 0)
        *rounds = size / (*count);
    else
        *rounds = size / (*count) - 1;
}

// specific to 3 parties
void NodeNetwork::SendAndGetDataFromPeer(priv_int_t *SendData, priv_int_t *RecvData, int size, uint ring_size, std::vector<std::vector<int>> send_recv_map) {
    uint count = 0, rounds = 0;
    getRounds_RSS(size, &count, &rounds, ring_size);
    for (int k = 0; k <= rounds; k++) {
        // these conditionals are here in the event of a computational party not sending (i.e. input, B2A, edaBit)
        for (size_t i = 0; i < send_recv_map[0].size(); i++) {
            if (send_recv_map[0][i] > 0) {
                sendDataToPeer(send_recv_map[0][i], SendData, k * count, count, size, ring_size);
            }
        }
        for (size_t i = 0; i < send_recv_map[1].size(); i++) {
            if (send_recv_map[1][i] > 0) {
                getDataFromPeer(send_recv_map[1][i], RecvData, k * count, count, size, ring_size);
            }
        }
    }
}

// used for 5p, 7p Mult and edaBit
void NodeNetwork::SendAndGetDataFromPeer(priv_int_t *SendData, priv_int_t **RecvData, int size, uint ring_size, std::vector<std::vector<int>> send_recv_map) {
    uint count = 0, rounds = 0;
    getRounds_RSS(size, &count, &rounds, ring_size);
    for (int k = 0; k <= rounds; k++) {
        for (size_t i = 0; i < send_recv_map[0].size(); i++) {
            // these conditionals are here in the event of a computational party not sending (i.e. input, B2A, edaBit)
            if (send_recv_map[0][i] > 0) {
                sendDataToPeer(send_recv_map[0][i], SendData, k * count, count, size, ring_size);
                // for (size_t j = 0; j < size; j++) {
                // printf("sent %lu\n", SendData[j]);
                // }
                // std::cout << "-- sending to " << send_recv_map[0][i] << " -- "<< std::endl;
            }
        }
        for (size_t i = 0; i < send_recv_map[1].size(); i++) {
            if (send_recv_map[1][i] > 0) {
                getDataFromPeer(send_recv_map[1][i], RecvData[i], k * count, count, size, ring_size);
                // for (size_t j = 0; j < size; j++) {
                //     printf("received %lu\n", RecvData[i][j]);
                // }
                // std::cout << "** recv from " << send_recv_map[1][i] << " into index " << i << " **"<<std::endl;
            }
        }
    }
}

// used for Open (5 and 7 pc)
void NodeNetwork::SendAndGetDataFromPeer(priv_int_t **SendData, priv_int_t **RecvData, int size, uint ring_size, std::vector<std::vector<int>> send_recv_map) {
    uint count = 0, rounds = 0;
    getRounds_RSS(size, &count, &rounds, ring_size);
    for (int k = 0; k <= rounds; k++) {
        // these conditionals are here in the event of a computational party not sending (i.e. input, B2A, edaBit)
        for (size_t i = 0; i < send_recv_map[0].size(); i++) {
            if (send_recv_map[0][i] > 0) {
                sendDataToPeer(send_recv_map[0][i], SendData[i], k * count, count, size, ring_size);
            }
        }
        for (size_t i = 0; i < send_recv_map[1].size(); i++) {
            if (send_recv_map[1][i] > 0) {
                getDataFromPeer(send_recv_map[1][i], RecvData[i], k * count, count, size, ring_size);
            }
        }
    }
}

void NodeNetwork::sendDataToPeer(int id, priv_int_t *data, int start, int amount, int size, uint ring_size) {
    try {
        uint read_amount = 0;
        if (start + amount > size)
            read_amount = size - start;
        else
            read_amount = amount;
        int unit_size = (ring_size + 7) >> 3;
        int buffer_size = unit_size * read_amount;
        char *buffer = (char *)malloc(sizeof(char) * buffer_size);
        char *pointer = buffer;
        memset(buffer, 0, buffer_size);
        for (int i = start; i < start + read_amount; i++) {
            // I think this is here to preserve security beyond the ell (or k) bits of the shares
            data[i] = data[i] & SHIFT_RSS[ring_size];
            memcpy(pointer, &data[i], unit_size);
            pointer += unit_size;
        }
        EVP_CIPHER_CTX *en_temp = peer2enlist.find(id)->second;
        unsigned char *encrypted = aes_encrypt(en_temp, (unsigned char *)buffer, &buffer_size);
        sendDataToPeer(id, 1, &buffer_size);
        sendDataToPeer(id, buffer_size, encrypted);
        free(buffer);
        free(encrypted);
    } catch (std::exception &e) {
        std::cout << "An exception (in Send Data To Peer) was caught: " << e.what() << "\n";
    }
}

void NodeNetwork::sendDataToPeer(int id, int size, priv_int_t *data, uint ring_size) {
    uint count = 0, rounds = 0;
    getRounds_RSS(size, &count, &rounds, ring_size);
    for (int k = 0; k <= rounds; k++)
        sendDataToPeer(id, data, k * count, count, size, ring_size);
}

void NodeNetwork::getDataFromPeer(int id, int size, priv_int_t *buffer, uint ring_size) {
    uint count = 0, rounds = 0;
    getRounds_RSS(size, &count, &rounds, ring_size);
    memset(buffer, 0, sizeof(priv_int_t) * size);
    for (int k = 0; k <= rounds; k++)
        getDataFromPeer(id, buffer, k * count, count, size, ring_size);
}

void NodeNetwork::getDataFromPeer(int id, priv_int_t *data, int start, int amount, int size, uint ring_size) {
    try {
        int write_amount = 0;
        if (start + amount > size)
            write_amount = size - start;
        else
            write_amount = amount;
        int unit_size = (ring_size + 7) >> 3;
        int length = 0;
        getDataFromPeer(id, 1, &length);
        char *buffer = (char *)malloc(sizeof(char) * length);
        getDataFromPeer(id, length, (unsigned char *)buffer);
        EVP_CIPHER_CTX *de_temp = peer2delist.find(id)->second;
        char *decrypted = (char *)aes_decrypt(de_temp, (unsigned char *)buffer, &length);
        memset(&data[start], 0, sizeof(priv_int_t) * write_amount);
        for (int i = start; i < start + write_amount; i++) {
            memcpy(&data[i], decrypted, unit_size);
            decrypted += unit_size;
        }
        decrypted -= (write_amount)*unit_size;
        free(buffer);
        free(decrypted);
    } catch (std::exception &e) {
        std::cout << "An exception (get Data From Peer) was caught: " << e.what() << "\n";
    }
}

void NodeNetwork::multicastToPeers(priv_int_t **data, priv_int_t **buffers, int size, uint ring_size) {
    int id = getID();
    int peers = config->getPeerCount();
    uint count = 0, rounds = 0;
    getRounds_RSS(size, &count, &rounds, ring_size);
    for (int k = 0; k <= rounds; k++) {
        for (int j = 1; j <= peers + 1; j++) {
            if (id == j)
                continue;
            sendDataToPeer(j, data[j - 1], k * count, count, size, ring_size);
        }
        for (int j = 1; j <= peers + 1; j++) {
            if (id == j)
                continue;
            getDataFromPeer(j, buffers[j - 1], k * count, count, size, ring_size);
        }
    }
}

// specific to 3 parties (open and mult)
void NodeNetwork::SendAndGetDataFromPeer_bit(uint8_t *SendData, uint8_t *RecvData, int size, std::vector<std::vector<int>> send_recv_map) {
    uint count = 0, rounds = 0;
    getRounds_bit(size, &count, &rounds);
    for (int k = 0; k <= rounds; k++) {
        for (size_t i = 0; i < send_recv_map[0].size(); i++) {
            sendDataToPeer_bit(send_recv_map[0][i], SendData, k * count, count, size);
        }
        for (size_t i = 0; i < send_recv_map[1].size(); i++) {
            getDataFromPeer_bit(send_recv_map[1][i], RecvData, k * count, count, size);
        }
    }
}

// used for multiplication
void NodeNetwork::SendAndGetDataFromPeer_bit(uint8_t *SendData, uint8_t **RecvData, int size, std::vector<std::vector<int>> send_recv_map) {
    uint count = 0, rounds = 0;
    getRounds_bit(size, &count, &rounds);
    for (int k = 0; k <= rounds; k++) {
        for (size_t i = 0; i < send_recv_map[0].size(); i++) {
            sendDataToPeer_bit(send_recv_map[0][i], SendData, k * count, count, size);
        }
        for (size_t i = 0; i < send_recv_map[1].size(); i++) {
            getDataFromPeer_bit(send_recv_map[1][i], RecvData[i], k * count, count, size);
        }
    }
}

// used for Open
void NodeNetwork::SendAndGetDataFromPeer_bit(uint8_t **SendData, uint8_t **RecvData, int size, std::vector<std::vector<int>> send_recv_map) {
    uint count = 0, rounds = 0;
    getRounds_bit(size, &count, &rounds);
    for (int k = 0; k <= rounds; k++) {
        for (size_t i = 0; i < send_recv_map[0].size(); i++) {
            sendDataToPeer_bit(send_recv_map[0][i], SendData[i], k * count, count, size);
        }
        for (size_t i = 0; i < send_recv_map[1].size(); i++) {
            getDataFromPeer_bit(send_recv_map[1][i], RecvData[i], k * count, count, size);
        }
    }
}

void NodeNetwork::sendDataToPeer_bit(int id, uint8_t *data, int start, int amount, int size) {
    try {
        int read_amount = 0;
        if (start + amount > size)
            read_amount = size - start;
        else
            read_amount = amount;
        int unit_size = 1;
        int buffer_size = unit_size * read_amount;
        char *buffer = (char *)malloc(sizeof(char) * buffer_size);
        char *pointer = buffer;
        memset(buffer, 0, buffer_size);
        memcpy(pointer, &data[start], unit_size * read_amount);

        EVP_CIPHER_CTX *en_temp = peer2enlist.find(id)->second;
        unsigned char *encrypted = aes_encrypt(en_temp, (unsigned char *)buffer, &buffer_size);
        sendDataToPeer(id, 1, &buffer_size);
        sendDataToPeer(id, buffer_size, encrypted);
        free(buffer);
        free(encrypted);
    } catch (std::exception &e) {
        std::cout << "An exception (in Send Data To Peer) was caught: " << e.what() << "\n";
    }
}

void NodeNetwork::getDataFromPeer_bit(int id, uint8_t *data, int start, int amount, int size) {
    try {
        int write_amount = 0;
        if (start + amount > size)
            write_amount = size - start;
        else
            write_amount = amount;
        int unit_size = 1;
        int length = 0;
        getDataFromPeer(id, 1, &length);
        char *buffer = (char *)malloc(sizeof(char) * length);
        getDataFromPeer(id, length, (unsigned char *)buffer);
        EVP_CIPHER_CTX *de_temp = peer2delist.find(id)->second;
        char *decrypted = (char *)aes_decrypt(de_temp, (unsigned char *)buffer, &length);
        memset(&data[start], 0, sizeof(uint8_t) * write_amount);
        memcpy(&data[start], decrypted, unit_size * write_amount);
        free(buffer);
        free(decrypted);

    } catch (std::exception &e) {
        std::cout << "An exception (get Data From Peer) was caught: " << e.what() << "\n";
    }
}

void NodeNetwork::getRounds_bit(int size, uint *count, uint *rounds) {
    // size means number of bytes
    int peers = config->getPeerCount();
    int unit_size = 1;
    *count = MAX_BUFFER_SIZE / (peers + 1) / unit_size;
    if (size % (*count) != 0)
        *rounds = size / (*count);
    else
        *rounds = size / (*count) - 1;
}

#endif
