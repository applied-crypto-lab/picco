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

/*
See /proc/sys/net/ipv4/tcp_wmem and tcp_rmem 
describes minimum, default, max values of tcp buffer sizes for send() and recv() 

chosen value is tcp_rmem default, 
    which leaves room for more data to be queued on the tcp buffer
*/
#define MAX_BUFFER_SIZE 131072

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

const int numb = sizeof(char) * 8;
int peers;
int bits;
/************************************************************/


NodeNetwork::NodeNetwork(NodeConfiguration *nodeConfig, std::string privatekey_filename, int num_threads) {
    privatekeyfile = privatekey_filename;
    config = nodeConfig;

    tracking = false;
    
    // here number of peers is n-1 instead of n
    peers = config->getPeerCount();
    bits = config->getBits();

#if __SHAMIR__
    element_size = (bits + numb - 1) / numb;
#endif

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

    int temp_buffer_size = MAX_BUFFER_SIZE / (peers + 1) / ((bits + numb - 1) / numb);

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



void NodeNetwork::beginTracking() {
    trackedBytes_Write = 0;
    trackedUnits_Write = 0;

    trackedBytes_Read = 0;
    trackedUnits_Read = 0;

    if (runningTotalRead == NULL || runningTotalWrite == NULL) {
        runningTotalRead = 0;
        runningTotalWrite = 0;
    }
    tracking = true;

}

char * separator = "-------------------------------------------";
// char * separator = "\n";
void NodeNetwork::endTracking(char* operation, int size) {
    // int expectedBytes = size * element_size;
    // int expectedUnits = (expectedBytes / MAX_BUFFER_SIZE) + 1;

    // if (size == 0)
    //     return;
    
    printf("%s\nUnit Size: %i\tOperator %s (size: %i)\nBytes wrote: \t%i (%i unit(s))\nBytes read: \t%i (%i unit(s))\n", 
    separator, element_size, operation, size, *trackedBytes_Write, *trackedUnits_Write, *trackedBytes_Read, *trackedUnits_Read);

    *trackedBytes_Write = 0;
    *trackedUnits_Write = 0; 
    *trackedBytes_Read = 0;
    *trackedUnits_Read = 0;

    printRunningTotals();
}

void NodeNetwork::printRunningTotals() {
    std::cout << "[Totals: Read = " <<  *runningTotalRead << ", Write = " << *runningTotalWrite << "]\n" <<std::flush;
}

/*
    MPZ send/recv
*/
void NodeNetwork::sendDataToPeer(int id, int size, mpz_t *data) {
    int start = 0, 
        length = element_size * size;

    while (start < length) {
        bytes = sendDataToPeer(id, data, start, length - start);

        if (bytes > 0) {
            start += bytes;
        }
    }
}

int NodeNetwork::sendDataToPeer(int id, mpz_t *data, int start, int remainingLength) {
    try {
        buffer = (unsigned char *) calloc(sizeof(unsigned char), remainingLength);

        //Export each segment to the buffer.
        //  Do not worry about start here, since it will be cut off in the next sendData call.
        for (int i = 0; i < remainingLength / element_size; i++) {
            mpz_export(buffer + (i * element_size), NULL, -1, element_size, -1, 0, data[i]);
        }

        EVP_CIPHER_CTX *en_temp = peer2enlist.find(id)->second;
        encrypted = aes_encrypt(en_temp, buffer, &remainingLength);
        
        bytes = sendDataToPeer(id, encrypted, start, remainingLength);

        // if (remainingLength > element_size) 
        //     for (int i = 0; i < (start + remainingLength) / element_size; i++)
        //                 gmp_printf("SENT to %i: [%i] = %Zd\n", id, i, data[i]);


        free(buffer);
        free(encrypted);

        return bytes;
    } catch (std::exception &e) {
        std::cout << "An exception (in Send Data To Peer) was caught: " << e.what() << "\n";
    }
}

void NodeNetwork::getDataFromPeer(int id, int size, mpz_t *buffer) {
    int start = 0, 
        length = element_size * size;

    while (start < length) {
        bytes = getDataFromPeer(id, buffer, start, length - start);

        if (bytes > 0) {
            start += bytes;
        }
    }
}

int NodeNetwork::getDataFromPeer(int id, mpz_t *data, int start, int remainingLength) {
    try {
        buffer = (unsigned char *)calloc(1, start + remainingLength);

        if (start > 0) {
            //Since we cannot write directly into an mpz, take out what we have currently
            for (int i = 0; i < start / element_size; i++) {
                mpz_export(buffer + (i * element_size), NULL, -1, element_size, -1, 0, data[i]);
            }

            //Now write to the buffer, starting where we left off
        }

        //Only the remaining gets sent, start = 0
        bytes = getDataFromPeer(id, buffer, 0, remainingLength);

        if (bytes > 0) {
            EVP_CIPHER_CTX *de_temp = peer2delist.find(id)->second;
            decrypted = (unsigned char *)aes_decrypt(de_temp, buffer, &remainingLength);

            for (int i = 0; i < remainingLength / element_size; i++) {
                mpz_import(data[i], element_size, -1, 1, -1, 0, decrypted + (i * element_size));
            }

            // if (remainingLength > element_size) 
            //     for (int i = 0; i < (start + remainingLength) / element_size; i++)
            //         gmp_printf("RECV from %i: [%i] = %Zd\n", id, i, data[i]);

            free(decrypted);
        }
        
        
        free(buffer);
        return bytes;
    } catch (std::exception &e) {
        std::cout << "An exception (get Data From Peer) was caught: " << e.what() << "\n";
    }
}


/*
    INT send/recv
        (used for 'mode' in threaded)
*/
void NodeNetwork::sendDataToPeer(int id, int size, int *data) {
    int bytesWrote = 0, totalSize = size * sizeof(int);
    
    while (bytesWrote < totalSize) {
        bytes = sendDataToPeer(id, (unsigned char*) data, bytesWrote, totalSize - bytesWrote);

        if (bytes > 0)
            bytesWrote += bytes;
    }
}

/*
    UNSIGNED CHAR (all data passed goes through here)
*/
void NodeNetwork::sendDataToPeer(int id, int size, unsigned char *data) {
    int start = 0, 
        length = size;

    while (start < length) {
        bytes = sendDataToPeer(id, data, start, length - start);

        if (bytes > 0)
            start += bytes;
    }
}

int NodeNetwork::sendDataToPeer(int id, unsigned char *data, int start, int remainingLength) {
    try {
        //Find corresponding socket
        int sockfd = peer2sock.find(id)->second;

        //If written returns -1, this means EAGAIN or EWOULDBLOCK.
        bytes = send(sockfd, data + start, remainingLength, MSG_DONTWAIT);

        if (bytes > 0) {
            if (tracking) {
                *trackedBytes_Write += bytes;
                *runningTotalWrite += bytes;

                if (bytes == remainingLength)
                    *trackedUnits_Write += 1;
            }
        }

        return bytes;
    } catch (std::exception &e) {
        std::cout << "An exception (in Send Data To Peer) was caught: " << e.what() << "\n";
    }
}

void NodeNetwork::getDataFromPeer(int id, int size, unsigned char *buffer) {
    int start = 0, 
        length = size;

    while (start < length) {
        bytes = getDataFromPeer(id, buffer, start, length - start);

        if (bytes > 0) {
            start += bytes;
        }
    }
}

int NodeNetwork::getDataFromPeer(int id, unsigned char *buffer, int start, int remainingLength) {
    try {
        int sockfd = peer2sock.find(id)->second;
        
        bytes = recv(sockfd, buffer + start, remainingLength, MSG_DONTWAIT);
        
        if (bytes > 0) {
            if (tracking) {
                *trackedBytes_Read += bytes;
                *runningTotalRead += bytes;

                if (bytes == remainingLength)
                    *trackedUnits_Read += 1;
            }
        }

        return bytes;
    } catch (std::exception &e) {
        std::cout << "An exception (get Data From Peer) was caught: " << e.what() << "\n";
    }
}

/*
    Multicast refers to sending multiple unicasts to separate locations, all with their own pieces of data.
        Think node i gets sent data[i], and gets back result in buffer[i].
*/
void NodeNetwork::multicastToPeers(long long **srcBuffer, long long **desBuffer, int size) {
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

void NodeNetwork::multicastToPeers(mpz_t **data, mpz_t **buffers, int size) {
    toSend.clear();
    toReceive.clear();

    int totalSize = element_size * size;
    
    int i, id = getID();
    for (i = 1; i <= peers + 1; i++) {
        if (id == i)
            continue;
        toSend.push_back({i, 0});
        toReceive.push_back({i, 0});
    }

    int start;
    while (!(toSend.empty() && toReceive.empty())) {
        for (it = toSend.begin(); it < toSend.end(); it++) {
            i = it->first;
            start = it->second;

            bytes = sendDataToPeer(i, data[i - 1], start, totalSize - start);

            if (bytes > 0)
                it->second += bytes;

            if (it->second == totalSize) {
                it = toSend.erase(it);
            }
        }
        
        for (it = toReceive.begin(); it < toReceive.end(); it++) {
            i = it->first;
            start = it->second;

            bytes = getDataFromPeer(i, buffers[i - 1], start, totalSize - start);

            if (bytes > 0) {
                it->second += bytes;
            }
            
            if (it->second == totalSize) {
                it = toReceive.erase(it);
            }
        }
    }

    for (int i = 0; i < size; i++)
        mpz_set(buffers[id - 1][i], data[id - 1][i]);
}

/* this function sends identical data (stored in variable 'data') to all other peers and receives data from all of them as well (stored in variable 'buffers') */
void NodeNetwork::broadcastToPeers(mpz_t *data, int size, mpz_t **buffers) {
    toSend.clear();
    toReceive.clear();

    int totalSize = element_size * size;

    int i, id = getID();
    for (i = 1; i <= peers + 1; i++) {
        if (id == i)
            continue;
        toSend.push_back({i, 0});
        toReceive.push_back({i, 0});
    }

    int start;
    while (!(toSend.empty() && toReceive.empty())) {
        for (it = toSend.begin(); it < toSend.end(); it++) {
            i = it->first;
            start = it->second;

            bytes = sendDataToPeer(i, data, start, totalSize - start);

            if (bytes > 0)
                it->second += bytes;

            if (it->second == totalSize)
                it = toSend.erase(it);
        }
        
        for (it = toReceive.begin(); it < toReceive.end(); it++) {
            i = it->first;
            start = it->second;

            bytes = getDataFromPeer(i, buffers[i - 1], start, totalSize - start);

            if (bytes > 0)
                it->second += bytes;
            
            if (it->second == totalSize)
                it = toReceive.erase(it);
        }
    }
    for (int j = 0; j < size; j++)
        mpz_set(buffers[id - 1][j], data[j]);
}

/* this function sends identical data to all other peers and receives data from all of them */
void NodeNetwork::broadcastToPeers(long long *data, int size, long long **result) {
    // int id = getID();
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
    getDataFromPeer(peerID, temp_buffers[peerID - 1][threadID], 0, write_amount);
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

void NodeNetwork::sendDataToPeer(int id, mpz_t *data, int start, int amount, int size, int threadID) {
    try {
        int *info = (int *)malloc(sizeof(int) * 3);
        info[0] = start;
        info[1] = amount;
        info[2] = size;

        int buffer_size = element_size * amount;

        //TODO: must change to correct _export
        buffer = (unsigned char *)calloc(sizeof(char), buffer_size);
        mpz_export(buffer, NULL, -1, 1, -1, 0, *data);


        EVP_CIPHER_CTX *en_temp = peer2enlist.find(id)->second;
        encrypted = aes_encrypt(en_temp, (unsigned char *)buffer, &buffer_size);

        sendDataToPeer(id, 1, &threadID);
        sendDataToPeer(id, 3, info);
        sendDataToPeer(id, buffer_size, encrypted);
        free(buffer);
        free(info);
        free(encrypted);
    } catch (std::exception &e) {
        std::cout << "An exception (in Send Data To Peer) was caught: " << e.what() << "\n";
    }
}

void NodeNetwork::getDataFromPeer(int socketID, int peerID, int threadID) {
    int *info = (int *)malloc(sizeof(int) * 3);
    getDataFromPeer(peerID, 3, info);

    pthread_mutex_lock(&buffer_mutex);
    while (comm_flags[peerID - 1][threadID] == 0) {
        pthread_cond_wait(&buffer_conditional_variables[threadID], &buffer_mutex);
    }
    pthread_mutex_unlock(&buffer_mutex);


    getDataFromPeer(peerID, buffer_handlers[peerID - 1][threadID], info[0], info[1]);

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

/*
For multithreading:
    There is a race between the integer node and data payload getting to the destination first.
    If the data gets there first, the managerWork function will read threadID as data,
        which will seg fault or cause unwanted behavior.
*/
void NodeNetwork::sendModeToPeers() {
    bool trackingBefore = tracking;
    if (trackingBefore)
        tracking = false;

    int msg = -2;
    int id = getID();

    for (int j = 1; j <= peers + 1; j++) {
        if (id == j)
            continue;
        
        sendDataToPeer(j, 1, &msg);
    }

    if (trackingBefore)
        tracking = true;
}

/* the function sends different data to each peer and receive data from each peer */
void NodeNetwork::multicastToPeers(mpz_t **data, mpz_t **buffers, int size, int threadID) {
    test_flags[threadID]++;
    // struct timeval tv1, tv2;

    if (size == 0)
        return;

    if (numOfThreads == 1) {
        multicastToPeers(data, buffers, size);
        return;
    }

    if (threadID == -1) {
        if (mode != -1) {
            sendModeToPeers();
            pthread_mutex_lock(&buffer_mutex);
            while (numOfChangedNodes < peers)
                pthread_cond_wait(&proceed_conditional_variable, &buffer_mutex);
            pthread_mutex_unlock(&buffer_mutex);
            numOfChangedNodes = 0;
            mode = -1;
        }
        multicastToPeers(data, buffers, size);
        return;
    }
    
    if (mode != 0) {
        pthread_mutex_lock(&buffer_mutex);
        pthread_cond_signal(&manager_conditional_variable);
        pthread_mutex_unlock(&buffer_mutex);
        mode = 0;
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

    int id = getID();
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

    for (int i = 0; i < size; i++)
        mpz_set(buffers[id - 1][i], data[id - 1][i]);
}

/* the function sends the same data to each peer and receives data from each peer */
void NodeNetwork::broadcastToPeers(mpz_t *data, int size, mpz_t **buffers, int threadID) {
    test_flags[threadID]++;

    if (size == 0)
        return;

    if (numOfThreads == 1) {
        broadcastToPeers(data, size, buffers);
        return;
    }

    if (threadID == -1) {
        if (mode != -1) {
            sendModeToPeers();
            pthread_mutex_lock(&buffer_mutex);
            while (numOfChangedNodes < peers)
                pthread_cond_wait(&proceed_conditional_variable, &buffer_mutex);
            pthread_mutex_unlock(&buffer_mutex);
            numOfChangedNodes = 0;
            mode = -1;
        }
        broadcastToPeers(data, size, buffers);
        return;
    }

    if (mode != 0) {
        pthread_mutex_lock(&buffer_mutex);
        pthread_cond_signal(&manager_conditional_variable);
        pthread_mutex_unlock(&buffer_mutex);
        mode = 0;
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

    int id = getID();
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
            buffer = (unsigned char *)malloc(RSA_size(priRkey));

            num_tries = 0; //Reset counter

            int n = read(sockfd[i], buffer, RSA_size(priRkey));
            while (n < 0) {
                if (num_tries < MAX_RETRIES) {
                    // cross platform version of sleep(), C++14 and onward
                    std::this_thread::sleep_for(WAIT_INTERVAL);
                    num_tries += 1;
                } else {
                    throw std::runtime_error("reading from socket");
                }

                //Try again
                n = read(sockfd[i], buffer, RSA_size(priRkey));
            }
                
            char *decrypt = (char *)malloc(n);
            memset(decrypt, 0x00, n);
            int dec_len = RSA_private_decrypt(n, (unsigned char *)buffer, (unsigned char *)decrypt, priRkey, RSA_PKCS1_OAEP_PADDING);
            if (dec_len < 1)
                throw std::runtime_error("RSA private decrypt");
            free(buffer);
            fclose(prikeyfp);

#else
            char *decrypt = (char *)malloc(2 * KEYSIZE + AES_BLOCK_SIZE);

            num_tries = 0; //Reset counter

            int n = read(sockfd[i], decrypt, 2 * KEYSIZE + AES_BLOCK_SIZE);
            while (n < 0) {
                if (num_tries < MAX_RETRIES) {
                    // cross platform version of sleep(), C++14 and onward
                    std::this_thread::sleep_for(WAIT_INTERVAL);
                    num_tries += 1;
                } else {
                    throw std::runtime_error("reading from socket");
                }

                //Try again
                n = read(sockfd[i], decrypt, 2 * KEYSIZE + AES_BLOCK_SIZE);
            }
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
    EVP_EncryptInit_ex(en, EVP_aes_128_ctr(), NULL, key, iv);
    de = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(de, EVP_aes_128_ctr(), NULL, key, iv);
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
    *count = MAX_BUFFER_SIZE / (peers + 1) / element_size;
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

//_Mult and _Open have the same pattern, just different places that the data is read into.
void NodeNetwork::multicastToThreshold(uint *sendtoIDs, uint *RecvFromIDs, mpz_t **data, mpz_t **buffer, int size) {
    int totalSize = element_size * size;

    toSend.clear();
    toReceive.clear();

    int i;
    for (i = 0; i < threshold; i++) {
        toSend.push_back({i, 0});
        toReceive.push_back({i, 0});
    }

    int start;
    while (!(toSend.empty() && toReceive.empty())) {
        for (it = toSend.begin(); it < toSend.end(); it++) {
            i = it->first;
            start = it->second;

            bytes = sendDataToPeer(sendtoIDs[i], data[i], start, totalSize - start);
            if (bytes > 0) {
                it->second += bytes;
            }


            if (it->second == totalSize) {
                it = toSend.erase(it);
                break;
            }
        }
        
        for (it = toReceive.begin(); it < toReceive.end(); it++) {
            i = it->first;
            start = it->second;
            
            if (buffer == NULL)
                getDestination = data[2 * threshold - i];
            else
                getDestination = buffer[threshold - i - 1];

            bytes = getDataFromPeer(RecvFromIDs[threshold - i - 1], getDestination, start, totalSize - start);

            if (bytes > 0) {
                it->second += bytes;
            }
            
            
            if (it->second == totalSize) {
                it = toReceive.erase(it);
                break;
            }
        }
    }
}


void NodeNetwork::multicastToPeers_Mult(uint *sendtoIDs, uint *RecvFromIDs, mpz_t **data, int size) {
    multicastToThreshold(sendtoIDs, RecvFromIDs, data, NULL, size);
}

// why is this needed for send/recv to work?
void NodeNetwork::multicastToPeers_Mult(uint *sendtoIDs, uint *RecvFromIDs, mpz_t **data, int size, int threadID) {
    test_flags[threadID]++;

    if (size == 0)
        return;
    
    if (numOfThreads == 1) {
        multicastToPeers_Mult(sendtoIDs, RecvFromIDs, data, size);
        return;
    }

    if (threadID == -1) {
        if (mode != -1) {
            sendModeToPeers();
            pthread_mutex_lock(&buffer_mutex);
            while (numOfChangedNodes < peers)
                pthread_cond_wait(&proceed_conditional_variable, &buffer_mutex);
            pthread_mutex_unlock(&buffer_mutex);
            numOfChangedNodes = 0;
            mode = -1;
        }
        multicastToPeers_Mult(sendtoIDs, RecvFromIDs, data, size);
        return;
    }

    if (mode != 0) {
        pthread_mutex_lock(&buffer_mutex);
        pthread_cond_signal(&manager_conditional_variable);
        pthread_mutex_unlock(&buffer_mutex);
        mode = 0;
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
    // std::cout << "batch threadID : "<<threadID << std::endl;
    test_flags[threadID]++;

    if (size == 0)
        return;
    
    if (numOfThreads == 1) {
        multicastToPeers_Open(sendtoIDs, RecvFromIDs, data, buffer, size);
        return;
    }
    
    if (threadID == -1) {
        if (mode != -1) {
            sendModeToPeers();
            pthread_mutex_lock(&buffer_mutex);
            while (numOfChangedNodes < peers)
                pthread_cond_wait(&proceed_conditional_variable, &buffer_mutex);
            pthread_mutex_unlock(&buffer_mutex);
            numOfChangedNodes = 0;
            mode = -1;
        }
        multicastToPeers_Open(sendtoIDs, RecvFromIDs, data, buffer, size);
        return;
    }

    if (mode != 0) {
        pthread_mutex_lock(&buffer_mutex);
        pthread_cond_signal(&manager_conditional_variable);
        pthread_mutex_unlock(&buffer_mutex);
        mode = 0;
    }
}

void NodeNetwork::multicastToPeers_Open(uint *sendtoIDs, uint *RecvFromIDs, mpz_t *data, mpz_t **buffer, int size) {
    mpz_t **copy = (mpz_t **)malloc((threshold + 1) * sizeof(mpz_t *));

    for (int i = 0; i < threshold + 1; i++)
        copy[i] = data;
        
    
    multicastToThreshold(sendtoIDs, RecvFromIDs, copy, buffer, size);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// NEW RSS NETWORKING FUNCTIONALITIES ////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
#if __RSS__

// specific to 3 parties
void NodeNetwork::SendAndGetDataFromPeer(priv_int_t *sendData, priv_int_t *recvData, int size, uint ring_size, std::vector<std::vector<int>> send_recv_map) {
    int totalSize = size * element_size,
        bytesRead = 0,
        bytesWrote = 0;

    while (bytesRead < totalSize || bytesWrote < totalSize) {
        if (bytesWrote < totalSize) {
            bytes = sendDataToPeer(send_recv_map[0][0], sendData, bytesWrote, totalSize - bytesWrote, ring_size);

            if (bytes > 0)
                bytesWrote += bytes;
        }

        if (bytesRead < totalSize) {
            bytes = getDataFromPeer(send_recv_map[1][0], recvData, bytesRead, totalSize - bytesRead, ring_size);

            if (bytes > 0)
                bytesRead += bytes;
        }
    }
}

// used for 5p, 7p Mult and edaBit
void NodeNetwork::SendAndGetDataFromPeer(priv_int_t *sendData, priv_int_t **recvData, int size, uint ring_size, std::vector<std::vector<int>> send_recv_map) {
    int totalSize = size * element_size;

    toSend.clear();
    toReceive.clear();

    int i, start;
    for (i = 0; i < send_recv_map[0].size(); i++) {
        toSend.push_back({i, 0});
        toReceive.push_back({i, 0});
    }

    while (!(toSend.empty() && toReceive.empty())) {
        for (it = toSend.begin(); it < toSend.end(); it++) {
            i = it->first;
            start = it->second;

            bytes = sendDataToPeer(i, sendData, start, totalSize - start, ring_size);

            if (bytes > 0)
                it->second += bytes;

            if (it->second == totalSize)
                it = toSend.erase(it);
        }

        for (it = toReceive.begin(); it < toReceive.end(); it++) {
            i = it->first;
            start = it->second;

            bytes = getDataFromPeer(send_recv_map[1][i], recvData[i], start, totalSize - start, ring_size);

            if (bytes > 0)
                it->second += bytes;

            if (it->second == totalSize)
                it = toReceive.erase(it);
        }
    }
}

// used for Open (5 and 7 pc)
void NodeNetwork::SendAndGetDataFromPeer(priv_int_t **sendData, priv_int_t **recvData, int size, uint ring_size, std::vector<std::vector<int>> send_recv_map) {
    int totalSize = size * element_size;

    toSend.clear();
    toReceive.clear();

    int i, start;
    for (i = 0; i < send_recv_map[0].size(); i++) {
        toSend.push_back({i, 0});
        toReceive.push_back({i, 0});
    }

    while (!(toSend.empty() && toReceive.empty())) {
        for (it = toSend.begin(); it < toSend.end(); it++) {
            i = it->first;
            start = it->second;

            bytes = sendDataToPeer(i, sendData[i], start, totalSize - start, ring_size);

            if (bytes > 0)
                it->second += bytes;

            if (it->second == totalSize)
                it = toSend.erase(it);
        }

        for (it = toReceive.begin(); it < toReceive.end(); it++) {
            i = it->first;
            start = it->second;

            bytes = getDataFromPeer(send_recv_map[1][i], recvData[i], start, totalSize - start, ring_size);

            if (bytes > 0)
                it->second += bytes;

            if (it->second == totalSize)
                it = toReceive.erase(it);
        }
    }
}

/*
    priv_int send / recv
*/
// Full Send
void NodeNetwork::sendDataToPeer(int id, int size, priv_int_t *data, uint ring_size) {
    int bytesWrote = 0, totalSize = size * element_size;

    while (bytesWrote < totalSize) {
        bytes = sendDataToPeer(id, data, bytesWrote, totalSize - bytesWrote, ring_size);

        if (bytes > 0)
            bytesWrote += bytes;
    }
}

// Partial Send (requires multiple calls)
int NodeNetwork::sendDataToPeer(int id, priv_int_t *data, int start, int remainingLength, uint ring_size) {
    try {
        unsigned char *buffer = (unsigned char *)calloc(sizeof(unsigned char), remainingLength);


        unsigned char *pointer = buffer;
        for (int i = start; i < start + remainingLength; i++) {
            // I think this is here to preserve security beyond the ell (or k) bits of the shares
            data[i] = data[i] & SHIFT_RSS[ring_size];
            memcpy(pointer, &data[i], element_size);
            pointer += element_size;
        }

        EVP_CIPHER_CTX *en_temp = peer2enlist.find(id)->second;
        unsigned char *encrypted = aes_encrypt(en_temp, buffer, &remainingLength);

        int bytes = sendDataToPeer(id, encrypted, start, remainingLength);
        
        free(buffer);
        free(encrypted);

        return bytes;

    } catch (std::exception &e) {
        std::cout << "An exception (in Send Data To Peer) was caught: " << e.what() << "\n";
    }
}

// Full Recv
void NodeNetwork::getDataFromPeer(int id, int size, priv_int_t *buffer, uint ring_size) {
    int bytesRead = 0, totalSize = size * element_size;

    while (bytesRead < totalSize) {
        bytes = getDataFromPeer(id, buffer, bytesRead, totalSize - bytesRead, ring_size);
        if (bytes > 0)
            bytesRead += bytes;
    }
}

// Partial Recv (requires multiple calls)
int NodeNetwork::getDataFromPeer(int id, priv_int_t *data, int start, int remainingLength, uint ring_size) {
    try {
        buffer = (unsigned char *) malloc(sizeof(char) * remainingLength);

        // Only the remaining gets sent, so start = 0
        int bytes = getDataFromPeer(id, buffer, 0, remainingLength);

        if (bytes > 0) {
            EVP_CIPHER_CTX *de_temp = peer2delist.find(id)->second;
            decrypted = aes_decrypt(de_temp, buffer, &remainingLength);

            memcpy(((unsigned char *) data) + start, decrypted, bytes);
        }
        
        free(buffer);
        free(decrypted);

        return bytes;
    } catch (std::exception &e) {
        std::cout << "An exception (get Data From Peer) was caught: " << e.what() << "\n";
    }
}

void NodeNetwork::multicastToPeers(priv_int_t **data, priv_int_t **buffers, int size, uint ring_size) {
    toSend.clear();
    toReceive.clear();

    int totalSize = element_size * size;
    
    int i, id = getID();
    for (i = 1; i <= peers + 1; i++) {
        if (id == i)
            continue;
        toSend.push_back({i, 0});
        toReceive.push_back({i, 0});
    }

    int start;
    while (!(toSend.empty() && toReceive.empty())) {
        for (it = toSend.begin(); it < toSend.end(); it++) {
            i = it->first;
            start = it->second;

            bytes = sendDataToPeer(i, data[i - 1], start, totalSize - start, ring_size);


            if (bytes > 0)
                it->second += bytes;

            if (it->second == totalSize) {
                it = toSend.erase(it);
            }
        }
        
        for (it = toReceive.begin(); it < toReceive.end(); it++) {
            i = it->first;
            start = it->second;

            bytes = getDataFromPeer(i, buffers[i - 1], start, totalSize - start, ring_size);


            if (bytes > 0) {
                it->second += bytes;
            }
            
            if (it->second == totalSize) {
                it = toReceive.erase(it);
            }
        }
    }
}

// specific to 3 parties (open and mult)
void NodeNetwork::SendAndGetDataFromPeer_bit(uint8_t *SendData, uint8_t *RecvData, int size, std::vector<std::vector<int>> send_recv_map) {
    for (int i = 0; i < send_recv_map[0].size(); i++) {
        sendDataToPeer(send_recv_map[0][i], size, (unsigned char *) SendData);
    }
    for (int i = 0; i < send_recv_map[1].size(); i++) {
        getDataFromPeer(send_recv_map[1][i], size, (unsigned char *) RecvData);
    }
}

// used for Multiplication
void NodeNetwork::SendAndGetDataFromPeer_bit(uint8_t *SendData, uint8_t **RecvData, int size, std::vector<std::vector<int>> send_recv_map) {
    for (int i = 0; i < send_recv_map[0].size(); i++) {
        sendDataToPeer(send_recv_map[0][i], size, (unsigned char *) SendData);
    }
    for (int i = 0; i < send_recv_map[1].size(); i++) {
        getDataFromPeer(send_recv_map[1][i], size, (unsigned char *) (RecvData[i]));
    }
}

// used for Open
void NodeNetwork::SendAndGetDataFromPeer_bit(uint8_t **SendData, uint8_t **RecvData, int size, std::vector<std::vector<int>> send_recv_map) {
    for (int i = 0; i < send_recv_map[0].size(); i++) {
        sendDataToPeer(send_recv_map[0][i], size, (unsigned char *) (SendData[i]));
    }
    for (int i = 0; i < send_recv_map[1].size(); i++) {
        getDataFromPeer(send_recv_map[1][i], size, (unsigned char *) (RecvData[i]));
    }
}

#endif