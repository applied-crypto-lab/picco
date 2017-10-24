/*   
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame

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
#include <vector>
#include <cstdlib>
#include "NodeConfiguration.h"
#include <string>
#include "time.h"
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h> 
#include <sys/time.h> 
#include <errno.h> 
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include <openssl/pem.h>
#include "openssl/bio.h"
#include "unistd.h"

NodeConfiguration *config;
EVP_CIPHER_CTX en, de;
int base = 36;
int MAX_BUFFER_SIZE = 229376; // in bytes 
//int MAX_BUFFER_SIZE = 4194304; 

std::map<int, EVP_CIPHER_CTX> peer2enlist;
std::map<int, EVP_CIPHER_CTX> peer2delist;
std::string privatekeyfile;
unsigned char* KeyIV;
unsigned char* peerKeyIV;

/************ STATIC VARIABLES INITIALIZATION ***************/
int** NodeNetwork::comm_flags = NULL; 
int* NodeNetwork::unit_flags = NULL; 

pthread_mutex_t NodeNetwork::socket_mutex; 
pthread_mutex_t NodeNetwork::buffer_mutex; 
pthread_cond_t* NodeNetwork::socket_conditional_variables = NULL;
pthread_cond_t* NodeNetwork::buffer_conditional_variables = NULL;
pthread_cond_t* NodeNetwork::flag_conditional_variables = NULL; 
pthread_cond_t NodeNetwork::manager_conditional_variable; 
pthread_cond_t NodeNetwork::proceed_conditional_variable; 

mpz_t*** NodeNetwork::buffer_handlers = NULL;
mpz_t*** NodeNetwork::temp_buffers = NULL; 

int* NodeNetwork::test_flags = NULL;  
int** NodeNetwork::temp_buffer_flags = NULL;
int*** NodeNetwork::temp_buffer_info = NULL; 
int** NodeNetwork::finished_socks = NULL;
int* NodeNetwork::read_socks = NULL;
int NodeNetwork::mode = 0; // -1 -- non-thread, 0 -- thread
int NodeNetwork::numOfChangedNodes = 0; //number of nodes that has changed modes so far
/************************************************************/


NodeNetwork::NodeNetwork(NodeConfiguration *nodeConfig, std::string privatekey_filename, int num_threads) {
	privatekeyfile = privatekey_filename;
	config = nodeConfig;
	connectToPeers();
	numOfThreads = num_threads; // it should be read from parsing 
	int peers = config->getPeerCount();
    	int numb = 8 * sizeof(char);
	int temp_buffer_size = MAX_BUFFER_SIZE/(peers+1)/((config->getBits()+numb-1)/numb); 
	
	/************************* MUTEX and COND VAR INITIALIZATION *******************/
	socket_conditional_variables = (pthread_cond_t*)malloc(sizeof(pthread_cond_t) * numOfThreads); 
	buffer_conditional_variables = (pthread_cond_t*)malloc(sizeof(pthread_cond_t) * numOfThreads); 
	flag_conditional_variables = (pthread_cond_t*)malloc(sizeof(pthread_cond_t) * numOfThreads); 
	
	for(int i = 0; i < numOfThreads; i++){
		pthread_cond_init(&socket_conditional_variables[i], NULL); 
		pthread_cond_init(&buffer_conditional_variables[i], NULL); 
		pthread_cond_init(&flag_conditional_variables[i], NULL); 
	}
	
	pthread_cond_init(&manager_conditional_variable, NULL); 
	pthread_cond_init(&proceed_conditional_variable, NULL); 

	pthread_mutex_init(&socket_mutex, NULL);
  	pthread_mutex_init(&buffer_mutex, NULL); 
	
	/********************** FLAGS AND HANDLERS INITIALIZATION *********************/
	temp_buffer_flags = (int**)malloc(sizeof(int*) * (peers+1)); 
	temp_buffer_info = (int***)malloc(sizeof(int**) * (peers+1)); 
	finished_socks = (int**)malloc(sizeof(int*) * (peers+1)); 
	comm_flags = (int**)malloc(sizeof(int*) * (peers+1)); 
	
	for(int i = 0; i < peers+1; i++){
		temp_buffer_flags[i] = (int*)malloc(sizeof(int) * numOfThreads); 
		temp_buffer_info[i] = (int**)malloc(sizeof(int*) * numOfThreads); 
		finished_socks[i] = (int*)malloc(sizeof(int) * numOfThreads); 
		comm_flags[i] = (int*)malloc(sizeof(int) * numOfThreads); 
		for(int j = 0; j < numOfThreads; j++){
			temp_buffer_info[i][j] = (int*)malloc(sizeof(int) * 3); 
			temp_buffer_flags[i][j] = comm_flags[i][j] = 0; 
			finished_socks[i][j] = 0; 
		}
	}
	
	read_socks = (int*)malloc(sizeof(int) * numOfThreads); 
	unit_flags = (int*)malloc(sizeof(int) * numOfThreads); 
		
	for(int i = 0; i < numOfThreads; i++)
		unit_flags[i] = read_socks[i] = 0; 		
	
	buffer_handlers = (mpz_t***)malloc(sizeof(mpz_t**) * (peers+1)); 
	temp_buffers = (mpz_t***)malloc(sizeof(mpz_t**) * (peers+1)); 
	for(int i = 0; i < peers+1; i++){
		buffer_handlers[i] = (mpz_t**)malloc(sizeof(mpz_t*) * numOfThreads);
		temp_buffers[i] = (mpz_t**)malloc(sizeof(mpz_t*) * numOfThreads);
		for(int j = 0; j < numOfThreads; j++){
			buffer_handlers[i][j] = NULL; 
			temp_buffers[i][j] = (mpz_t*)malloc(sizeof(mpz_t) * temp_buffer_size); 
			for(int k = 0; k < temp_buffer_size; k++)
				mpz_init(temp_buffers[i][j][k]); 
		} 
	}

	test_flags = (int*)malloc(sizeof(int) * numOfThreads); 
	for(int i = 0; i < numOfThreads; i++)
		test_flags[i] = 0; 
}

NodeNetwork::NodeNetwork() {}

NodeNetwork::~NodeNetwork() {}

void NodeNetwork::sendDataToPeer(int id, mpz_t* data, int start, int amount, int size){
	try{
        	int read_amount = 0;
        	if(start+amount > size)
            		read_amount = size-start;
        	else
            		read_amount = amount;
		int bits = config->getBits();  
		int numb = 8 * sizeof(char); 
		int unit_size = (bits + numb - 1)/numb;
		int buffer_size = unit_size * read_amount;
		char* buffer = (char*)malloc(sizeof(char) * buffer_size);
		char* pointer = buffer;  
		memset(buffer, 0, buffer_size);
		for(int i = start; i < start+read_amount; i++){
			mpz_export(pointer, NULL, -1, 1, -1, 0, data[i]); 
			pointer += unit_size; 
		}
		EVP_CIPHER_CTX en_temp = peer2enlist.find(id)->second;
		unsigned char* encrypted = aes_encrypt(&en_temp, (unsigned char*)buffer, &buffer_size);
		sendDataToPeer(id, 1, &buffer_size); 
		sendDataToPeer(id, buffer_size, encrypted);
		free(buffer);
		//free(encrypted);
	}catch(std::exception& e){
		std::cout << "An exception (in Send Data To Peer) was caught: " << e.what() << "\n";
	}
}

void NodeNetwork::sendDataToPeer(int id, int size, mpz_t *data)
{
    int count = 0, rounds = 0; 
    getRounds(size, &count, &rounds); 
    for(int k = 0; k <= rounds; k++)
        sendDataToPeer(id, data, k*count, count, size);
}

void NodeNetwork::sendDataToPeer(int id, int size, long long *data)
{
    int count = 0, rounds = 0; 
    getRounds(size, &count, &rounds);
    mpz_t* data1 = (mpz_t*)malloc(sizeof(mpz_t) * size); 
    for(int i = 0; i < size; i++)
   	mpz_init_set_ui(data1[i], data[i]);  
    for(int k = 0; k <= rounds; k++)
        sendDataToPeer(id, data1, k*count, count, size);
    
   //free the memory
    for(int i = 0; i < size; i++)
	mpz_clear(data1[i]); 
    free(data1); 
}

void NodeNetwork::getDataFromPeer(int id, int size, mpz_t* buffer)
{
    int count = 0, rounds = 0; 
    getRounds(size, &count, &rounds); 
    for(int k = 0; k <= rounds; k++)
        getDataFromPeer(id, buffer, k*count, count, size);
}

void NodeNetwork::getDataFromPeer(int id, int size, long long *buffer)
{
    int count = 0, rounds = 0; 
    mpz_t* buffer1 = (mpz_t*)malloc(sizeof(mpz_t) * size); 
    for(int i = 0; i < size; i++)
   	mpz_init(buffer1[i]);  
    getRounds(size, &count, &rounds); 
    for(int k = 0; k <= rounds; k++)
        getDataFromPeer(id, buffer1, k*count, count, size);
  
    for(int i = 0; i < size; i++)
    {
	buffer[i] = mpz_get_ui(buffer1[i]); 
    }
    for(int i = 0; i < size; i++)
	    mpz_clear(buffer1[i]); 
    free(buffer1); 
}

void NodeNetwork::sendDataToPeer(int id, int size, unsigned char* data){
	try{
		int on = 1; 
		unsigned char* p = data; 
		int bytes_read = sizeof(unsigned char) * size; 
		int sockfd = peer2sock.find(id)->second; 
  		fd_set fds;
		while(bytes_read > 0){
			int bytes_written = send(sockfd, p, sizeof(unsigned char) * bytes_read, MSG_DONTWAIT);
			if(bytes_written < 0){
				FD_ZERO(&fds); 
				FD_SET(sockfd, &fds);
				int n = select(sockfd+1, NULL, &fds, NULL, NULL);
				if(n > 0)
					continue; 
			} 
			else{ 
				bytes_read -= bytes_written; 
				p += bytes_written; 
			}
		} 
	} catch(std::exception& e){
		std::cout << "An exception (in Send Data To Peer) was caught: " << e.what() << "\n";
	}
}

void NodeNetwork::sendDataToPeer(int id, int size, int* data){
	try{
		int* p = data; 
		int bytes_read = sizeof(int) * size;
		int sockfd = peer2sock.find(id)->second;
		fd_set fds; 
		while(bytes_read > 0){
			int bytes_written = send(sockfd, p, bytes_read, MSG_DONTWAIT);
			if(bytes_written < 0){
				FD_ZERO(&fds); 
				FD_SET(sockfd, &fds); 
				int n = select(sockfd+1, NULL, &fds, NULL, NULL); 
				if(n > 0)
					continue; 
			} 
			else{
				bytes_read -= bytes_written; 
				p += (bytes_written/sizeof(int)); 
			}
		} 
	}catch(std::exception& e){
		std::cout << "An exception (in Send Data To Peer) was caught: " << e.what() << "\n";
	}
}

void NodeNetwork::getDataFromPeer(int id, int size, int* buffer){
	try{
		int length = 0, bytes = 0;
		int* tmp_buffer = (int*)malloc(sizeof(int) * size);
		fd_set fds;
		std::map<int, int>::iterator it;
		int sockfd = peer2sock.find(id)->second;
		while(length < sizeof(int) * size){
				bytes = recv(sockfd, tmp_buffer, sizeof(int) * (size-length/sizeof(int)), MSG_DONTWAIT);
				if(bytes < 0){
					FD_ZERO(&fds); 
					FD_SET(sockfd, &fds); 
					int n = select(sockfd+1, &fds, NULL, NULL, NULL);
					if(n > 0)
						continue;  
				}
				else{
					memcpy(&buffer[length/sizeof(int)], tmp_buffer, bytes); 
					length += bytes;
				}
		}
		free(tmp_buffer); 
	}catch(std::exception& e){
		std::cout << "An exception (get Data From Peer) was caught: " << e.what() << "\n";
	}
}

void NodeNetwork::getDataFromPeer(int id, int size, unsigned char* buffer){
	try{
		int length = 0, bytes = 0;
		unsigned char*  tmp_buffer = (unsigned char*)malloc(sizeof(unsigned char) * size);
		fd_set fds;
		int sockfd = peer2sock.find(id)->second; 
		while(length < sizeof(unsigned char) * size){
			bytes = recv(sockfd, tmp_buffer, sizeof(unsigned char) * (size-length/sizeof(unsigned char)), MSG_DONTWAIT);
			if(bytes < 0){
				FD_ZERO(&fds); 
				FD_SET(sockfd, &fds); 
				int n = select(sockfd+1, &fds, NULL, NULL, NULL);
				if(n > 0)
					continue;  
			}
			else{
				memcpy(&buffer[length/sizeof(unsigned char)], tmp_buffer, bytes); 
				length += bytes;
			}
		}
		free(tmp_buffer); 
    }catch(std::exception& e){
        std::cout << "An exception (get Data From Peer) was caught: " << e.what() << "\n";
    }
}


void NodeNetwork::getDataFromPeer(int id, mpz_t* data, int start, int amount, int size){
	try{
            	int write_amount = 0;
            	if(start+amount > size)
                	write_amount = size-start;
            	else
                	write_amount = amount;
        
            	int bits = config->getBits();
	    	int numb = 8 * sizeof(char); 
	    	int unit_size = (bits + numb - 1)/numb;
	    	int length;
	    	getDataFromPeer(id, 1, &length); 
	    	char* buffer = (char*)malloc(sizeof(char) * length);
	    	getDataFromPeer(id, length, (unsigned char*)buffer); 	    	
		EVP_CIPHER_CTX de_temp = peer2delist.find(id)->second;
		char *decrypted = (char *) aes_decrypt(&de_temp, (unsigned char*)buffer, &length);
	        //char *tmp = decrypted;
		for(int i = start; i < start+write_amount; i++)
	   	{ 
			mpz_import(data[i], unit_size, -1, 1, -1, 0, decrypted); 
			decrypted += unit_size; 
	    	}
	    	free(buffer);
		//free(tmp);
	    }catch(std::exception& e){
			std::cout << "An exception (get Data From Peer) was caught: " << e.what() << "\n";
	    }
}

void NodeNetwork::multicastToPeers(long long** srcBuffer, long long **desBuffer, int size){
    int peers = config->getPeerCount(); 
    mpz_t** buffer = (mpz_t**)malloc(sizeof(mpz_t*) * (peers+1));
    mpz_t** data = (mpz_t**)malloc(sizeof(mpz_t*) * (peers+1));
    int sendIdx = 0, getIdx = 0; 
    for(int i = 0; i < peers+1; i++)
    {
        buffer[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
        data[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
        for(int j = 0; j < size; j++)
        {
                mpz_init(buffer[i][j]);
                mpz_init_set_ui(data[i][j], srcBuffer[i][j]);
        }
    }

    multicastToPeers(data, buffer, size); 	
    for(int i = 0; i <= peers; i++)
	for(int j = 0; j < size; j++)
		desBuffer[i][j] = mpz_get_ui(buffer[i][j]); 
    for(int i = 0; i <= peers; i++)
    {
        free(buffer[i]);
        free(data[i]);
    }
}


void NodeNetwork::multicastToPeers(mpz_t** data, mpz_t** buffers, int size){
    int id = getID();
    int peers = config->getPeerCount(); 
    struct timeval tv1, tv2;
    
    int sendIdx = 0, getIdx = 0; 
    //compute the maximum size of data that can be communicated 
    int count = 0, rounds = 0; 
    getRounds(size, &count, &rounds); 
    for(int i = 1; i <= peers+1; i++)
    {
	if(id == i)
        {
            for(int k = 0; k <= rounds; k++)
            {
                for(int j = 1; j <= peers+1; j++)
                {
                    if(id == j)
                        continue;
                    sendDataToPeer(j, data[j-1], k*count, count, size);
                }
                for(int j = 1; j <= peers+1; j++)
                {
                    if(id == j)
                        continue;
                    getDataFromPeer(j, buffers[j-1], k*count, count, size);
                }
            }
        }
    }
    for(int i = 0; i < size; i++)
	mpz_set(buffers[id-1][i], data[id-1][i]); 
}


void NodeNetwork::broadcastToPeers(mpz_t* data, int size, mpz_t** buffers){
    int id = getID();
    int peers = config->getPeerCount();
    
    int rounds = 0, count = 0; 
    getRounds(size, &count, &rounds); 
    for(int i = 1; i <= peers+1; i++)
    {
        if(id == i)
        {
            for(int k = 0; k <= rounds; k++)
            {
                for(int j = 1; j <= peers+1; j++)
                {
                    if(id == j)
                        continue; 
			sendDataToPeer(j, data, k*count, count, size);
                }
                for(int j = 1; j <= peers+1; j++)
                {
                    if(id == j)
                        continue;
			getDataFromPeer(j, buffers[j-1], k*count, count, size);
                }
            }
	    for(int j = 0; j < size; j++)
	    	mpz_set(buffers[id-1][j], data[j]);  
        }
    }
}

void NodeNetwork::broadcastToPeers(long long* data, int size, long long** result){
    int id = getID();
    int peers = config->getPeerCount();
    mpz_t** buffers = (mpz_t**)malloc(sizeof(mpz_t*) * (peers+1));
    mpz_t *data1 = (mpz_t*)malloc(size * sizeof(mpz_t));

    for(int i = 0; i < peers+1; i++)
    {
        buffers[i] = (mpz_t*)malloc(sizeof(mpz_t) * size);
        for(int j = 0; j < size; j++)
                mpz_init(buffers[i][j]);
    }

    for(int i = 0; i < size; i++)
        mpz_init_set_ui(data1[i], data[i]);
    broadcastToPeers(data1, size, buffers); 
    for(int i = 0; i <= peers; i++)
	for(int j = 0; j < size; j++)
		result[i][j] = mpz_get_ui(buffers[i][j]); 

    for(int i = 0; i <= peers; i++)
        free(buffers[i]);
}


void NodeNetwork::connectToPeers(){
	int peers = config->getPeerCount(); 
	for (int i = 1; i <= peers+1; i++)
                if(config->getID() == i)
                {
                        if(i != (peers+1))
                                requestConnection(peers+1-i);
                        if(i != 1)
                                acceptPeers(i-1);
                }
	pthread_create(&manager, NULL, &managerWorkHelper, this);

}

void* NodeNetwork::managerWorkHelper(void* net){
	((NodeNetwork*)net)->managerWork(); 
	return 0; 	
}

void* NodeNetwork::managerWork(){
      fd_set socketDescriptorSet;
      int fdmax = 0, nReady = 0, index = 0, nbytes, threadID;
      int peers = config->getPeerCount();
      FD_ZERO(&socketDescriptorSet);
      std::map<int, int>::iterator it;

      int* socks = (int*)malloc(sizeof(int) * peers);
      int* nodes = (int*)malloc(sizeof(int) * peers);
      for(it = peer2sock.begin(); it != peer2sock.end(); ++it){
	 nodes[index] = it->first; 
         socks[index] = it->second;
         index++;
      }
           

      while(1){
            for(int i = 0; i < peers; i++){
                  FD_SET(socks[i], &socketDescriptorSet);
                  if(socks[i] > fdmax)
                          fdmax = socks[i];
            }
            if((nReady = select(fdmax+1, &socketDescriptorSet, NULL, NULL, NULL)) == -1){
                  printf("Select Error\n");
                  return 0;
            }
	    for(int i = 0; i < peers; i++){
                  if(FD_ISSET(socks[i], &socketDescriptorSet)){
			getDataFromPeer(nodes[i], 1, &threadID);
			if(threadID == -2){
				 numOfChangedNodes++;
                		 if(numOfChangedNodes == peers){
					pthread_mutex_lock(&buffer_mutex); 
				 	pthread_cond_signal(&proceed_conditional_variable);
					pthread_cond_wait(&manager_conditional_variable, &buffer_mutex); 
					pthread_mutex_unlock(&buffer_mutex); 
					break; 
				 }
				 continue; 
			} 
			if(finished_socks[nodes[i]-1][threadID] == 1){
				getDataFromPeerToBuffer(socks[i], nodes[i], threadID, &temp_buffer_info[nodes[i]-1][threadID]); 
				temp_buffer_flags[nodes[i]-1][threadID] = 1; 
			}else{
				getDataFromPeer(socks[i], nodes[i], threadID); 
				finished_socks[nodes[i]-1][threadID] = 1; 
				int read_buffered_socks = 1; 
				for(int j = 0; j < peers; j++){
					if(finished_socks[nodes[j]-1][threadID] == 0){
						read_buffered_socks = 0; 
						break; 
					}
				}
				if(read_buffered_socks){
					for(int j = 0; j < peers; j++){
						finished_socks[nodes[j]-1][threadID] = 0; 
						if(temp_buffer_flags[nodes[j]-1][threadID] == 1){
							restoreDataToBuffer(socks[j], nodes[j], threadID, &temp_buffer_info[nodes[j]-1][threadID]); 
							temp_buffer_flags[nodes[j]-1][threadID] = 0; 
							finished_socks[nodes[j]-1][threadID] = 1; 
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

void NodeNetwork::getDataFromPeerToBuffer(int socketID, int peerID, int threadID, int** info){
      int start, write_amount = 0;  
      int* data = (int*)malloc(sizeof(int) * 3); 
      getDataFromPeer(peerID, 3, data);
      (*info)[0] = data[0]; (*info)[1] = data[1]; (*info)[2] = data[2]; 
      if(data[0]+data[1] > data[2])
		write_amount = data[2] - data[0]; 
      else write_amount = data[1];
      getDataFromPeer(peerID, temp_buffers[peerID-1][threadID], 0, write_amount, data[1]); 
      free(data); 
      return; 
}

void NodeNetwork::restoreDataToBuffer(int socketID, int peerID, int threadID, int** info){
      int id = config->getID();  
      int start = (*info)[0], amount = (*info)[1], size = (*info)[2]; 
      pthread_mutex_lock(&buffer_mutex); 
      while(comm_flags[peerID-1][threadID] == 0){	
		pthread_cond_wait(&buffer_conditional_variables[threadID], &buffer_mutex); 
      }
      pthread_mutex_unlock(&buffer_mutex);
      int write_amount = 0;
      if(start+amount >= size){
                write_amount = size - start;
		comm_flags[peerID-1][threadID] = 0;  
      }else write_amount = amount;

      for(int i = start; i < start+write_amount; i++){
		mpz_set(buffer_handlers[peerID-1][threadID][i], temp_buffers[peerID-1][threadID][i-start]); 
      }
      
      read_socks[threadID]++; 
      if(read_socks[threadID] == config->getPeerCount()){
      		pthread_mutex_lock(&socket_mutex);
		while(unit_flags[threadID] == 1)
			pthread_cond_wait(&flag_conditional_variables[threadID], &socket_mutex); 		
		unit_flags[threadID] = 1; 
		pthread_cond_signal(&socket_conditional_variables[threadID]); 
      		pthread_mutex_unlock(&socket_mutex); 
		read_socks[threadID] = 0; 
      }

      return; 
}

void NodeNetwork::getDataFromPeer(int socketID, int peerID, int threadID){
      int id = config->getID();  
      int* info = (int*)malloc(sizeof(int) * 3);    
      getDataFromPeer(peerID, 3, info);
      pthread_mutex_lock(&buffer_mutex); 
      while(comm_flags[peerID-1][threadID] == 0){	
		pthread_cond_wait(&buffer_conditional_variables[threadID], &buffer_mutex); 
      }
      pthread_mutex_unlock(&buffer_mutex);
      getDataFromPeer(peerID, buffer_handlers[peerID-1][threadID], info[0], info[1], info[2]); 
      
      if(info[0]+info[1] >= info[2]){
		comm_flags[peerID-1][threadID] = 0;  
      }
      read_socks[threadID]++; 
      if(read_socks[threadID] == config->getPeerCount()){
      		pthread_mutex_lock(&socket_mutex);
		while(unit_flags[threadID] == 1)
			pthread_cond_wait(&flag_conditional_variables[threadID], &socket_mutex); 
		unit_flags[threadID] = 1; 
		pthread_cond_signal(&socket_conditional_variables[threadID]); 
      		pthread_mutex_unlock(&socket_mutex); 
		read_socks[threadID] = 0; 
      }
      free(info); 
      return; 
}
void NodeNetwork::sendModeToPeers(int id){
     int peers = config->getPeerCount(); 
     int msg = -2;
     for(int j = 1; j <= peers+1; j++){
        if(id == j)
          continue;
      	  sendDataToPeer(j, 1, &msg);
   }
   //sleep(1);
}
void NodeNetwork::multicastToPeers(mpz_t** data, mpz_t** buffers, int size, int threadID){
    test_flags[threadID]++;
    int id = getID();
    int peers = config->getPeerCount();
    struct timeval tv1, tv2;
    if(size == 0) return;
    if(threadID == -1){
		if(mode != -1){
			sendModeToPeers(id); 
			pthread_mutex_lock(&buffer_mutex);  
			while(numOfChangedNodes < peers)
				pthread_cond_wait(&proceed_conditional_variable, &buffer_mutex);
			pthread_mutex_unlock(&buffer_mutex);  
			numOfChangedNodes = 0; 
			mode = -1; 
		}
		multicastToPeers(data, buffers, size);
		return; 		
    }else{
		if(mode != 0){
			pthread_mutex_lock(&buffer_mutex); 
			pthread_cond_signal(&manager_conditional_variable); 
			pthread_mutex_unlock(&buffer_mutex);
			mode = 0;  
		}
    }
    int sendIdx = 0, getIdx = 0;
    int count = 0, rounds = 0; 
    getRounds(size, &count, &rounds); 
    for(int i = 0; i <= peers; i++)
    	buffer_handlers[i][threadID] = buffers[i]; 
    
    pthread_mutex_lock(&buffer_mutex);
    for(int i = 0; i <= peers; i++)
    	comm_flags[i][threadID] = 1; 
    pthread_cond_signal(&buffer_conditional_variables[threadID]);
    gettimeofday(&tv1, NULL); 
    pthread_mutex_unlock(&buffer_mutex);  
    
    for(int i = 1; i <= peers+1; i++)
    {
	if(id == i)
        {
            for(int k = 0; k <= rounds; k++)
            {
		for(int j = 1; j <= peers+1; j++)
                {
                    if(id == j)
                        continue;
		    pthread_mutex_lock(&socket_mutex);
                    sendDataToPeer(j, data[j-1], k*count, count, size, threadID);
		    pthread_mutex_unlock(&socket_mutex); 
                }
		pthread_mutex_lock(&socket_mutex); 
		while(unit_flags[threadID] == 0){
    			gettimeofday(&tv1, NULL); 
			pthread_cond_wait(&socket_conditional_variables[threadID], &socket_mutex);
		}
		unit_flags[threadID] = 0;
		pthread_cond_signal(&flag_conditional_variables[threadID]);  
		pthread_mutex_unlock(&socket_mutex);
	    }
        }
    }
    
    for(int i = 0; i < size; i++)
	mpz_set(buffers[id-1][i], data[id-1][i]); 
}

void NodeNetwork::broadcastToPeers(mpz_t* data, int size, mpz_t** buffers, int threadID){	
    test_flags[threadID]++; 
    if(size == 0) return;  
    int id = getID();
    int peers = config->getPeerCount();
    if(threadID == -1){
                if(mode != -1){
                        sendModeToPeers(id);
			pthread_mutex_lock(&buffer_mutex);  
                        while(numOfChangedNodes < peers)
                                pthread_cond_wait(&proceed_conditional_variable, &buffer_mutex);	
			pthread_mutex_unlock(&buffer_mutex); 
                        numOfChangedNodes = 0;
                        mode = -1;
		}
                broadcastToPeers(data, size, buffers);
                return;
    }else{
                if(mode != 0){
                        pthread_mutex_lock(&buffer_mutex);
                        pthread_cond_signal(&manager_conditional_variable);
                        pthread_mutex_unlock(&buffer_mutex);
			mode = 0; 
                }
    }
    int sendIdx = 0, getIdx = 0; 
    int rounds = 0, count = 0; 
    getRounds(size, &count, &rounds); 
    for(int i = 0; i <= peers; i++)
        buffer_handlers[i][threadID] = buffers[i];
	
    pthread_mutex_lock(&buffer_mutex);
    for(int i = 0; i <= peers; i++)
        comm_flags[i][threadID] = 1;
    pthread_cond_signal(&buffer_conditional_variables[threadID]);
    pthread_mutex_unlock(&buffer_mutex);
    for(int i = 1; i <= peers+1; i++)
    {
        if(id == i)
        {
            for(int k = 0; k <= rounds; k++)
            {
                for(int j = 1; j <= peers+1; j++)
                {
                    if(id == j)
                        continue;
		    pthread_mutex_lock(&socket_mutex); 
                    sendDataToPeer(j, data, k*count, count, size, threadID);
		    pthread_mutex_unlock(&socket_mutex); 
                }
		pthread_mutex_lock(&socket_mutex);
                while(unit_flags[threadID] == 0){
                        pthread_cond_wait(&socket_conditional_variables[threadID], &socket_mutex);
                }
		unit_flags[threadID] = 0;
                pthread_cond_signal(&flag_conditional_variables[threadID]); 
                pthread_mutex_unlock(&socket_mutex);
            }
	    for(int j = 0; j < size; j++)
	    	mpz_set(buffers[id-1][j], data[j]);  
        }
    }
}


void NodeNetwork::sendDataToPeer(int id, mpz_t* data, int start, int amount, int size, int threadID){
	try{
        	int read_amount = 0;
        	if(start+amount > size)
            		read_amount = size-start;
        	else
            		read_amount = amount;
		int bits = config->getBits();  
		int numb = 8 * sizeof(char); 
		int unit_size = (bits + numb - 1)/numb;
		int buffer_size = unit_size * read_amount;
		int* info = (int*)malloc(sizeof(int) * 3); 
		info[0] = start; 
		info[1] = amount; 
		info[2] = size; 

		char* buffer = (char*)malloc(sizeof(char) * buffer_size);
		char* pointer = buffer; 
		memset(buffer, 0, buffer_size);
		for(int i = start; i < start+read_amount; i++){
			mpz_export(pointer, NULL, -1, 1, -1, 0, data[i]); 
			pointer += unit_size; 
		}

		EVP_CIPHER_CTX en_temp = peer2enlist.find(id)->second;
		unsigned char* encrypted = aes_encrypt(&en_temp, (unsigned char*)buffer, &buffer_size);
		sendDataToPeer(id, 1, &threadID); 
		sendDataToPeer(id, 3, info); 
		sendDataToPeer(id, 1, &buffer_size); 
		sendDataToPeer(id, buffer_size, encrypted);
		free(buffer);
		free(info); 
		//free(encrypted);
	}catch(std::exception& e){
		std::cout << "An exception (in Send Data To Peer) was caught: " << e.what() << "\n";
	}
}

void NodeNetwork::requestConnection(int numOfPeers){
		peerKeyIV = (unsigned char*)malloc(32);
		int* sockfd = (int*)malloc(sizeof(int) * numOfPeers); 
		int* portno = (int*)malloc(sizeof(int) * numOfPeers); 
		struct sockaddr_in *serv_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in) * numOfPeers); 
		struct hostent **server = (struct hostent**)malloc(sizeof(struct hostent*) * numOfPeers); 
	 	int on = 1; 		
		
		for(int i = 0; i < numOfPeers; i++){
			int ID = config->getID()+i+1; 
			portno[i] = config->getPeerPort(ID); 
			sockfd[i] = socket(AF_INET, SOCK_STREAM, 0);
			if(sockfd[i] < 0)
				fprintf(stderr, "ERROR, opening socket\n");
			// the function below might not work in certain
			// configurations, e.g., running all nodes from the
			// same VM. it is not used for single-threaded programs
			// and thus be commented out or replaced with an
			// equivalent function otherwise.
 			//fcntl(sockfd[i], F_SETFL, O_NONBLOCK);
			int rc = setsockopt(sockfd[i], SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)); 
			rc = setsockopt(sockfd[i], IPPROTO_TCP, TCP_NODELAY, (char*)&on, sizeof(on));
			server[i] = gethostbyname((config->getPeerIP(ID)).c_str());
			if(server[i] == NULL)
				fprintf(stderr, "ERROR, no such hosts \n");
			bzero((char*) &serv_addr[i], sizeof(serv_addr[i]));
			serv_addr[i].sin_family = AF_INET; 
			bcopy((char*)server[i]->h_addr, (char*)&serv_addr[i].sin_addr.s_addr, server[i]->h_length); 
			serv_addr[i].sin_port = htons(portno[i]); 
			
			int res, valopt; 
  			fd_set myset; 
  			struct timeval tv; 
  			socklen_t lon;
			res = connect(sockfd[i], (struct sockaddr*) &serv_addr[i], sizeof(serv_addr[i])); 
			if (res < 0) { 
     				if (errno == EINPROGRESS) {
        				tv.tv_sec = 15; 
        				tv.tv_usec = 0; 
        				FD_ZERO(&myset); 
        				FD_SET(sockfd[i], &myset); 
        				if (select(sockfd[i]+1, NULL, &myset, NULL, &tv) > 0) { 
           					lon = sizeof(int); 
           					getsockopt(sockfd[i], SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon); 
           					if (valopt) { 
							fprintf(stderr, "Error in connection() %d - %s\n", valopt, strerror(valopt)); 
              						exit(0); 
           					}
        				} 
        				else { 
           					fprintf(stderr, "Timeout or error() %d - %s\n", valopt, strerror(valopt)); 
          				 	exit(0); 
        			     	} 
     				} 
     				else { 
        				fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno)); 
        				exit(0); 
     				} 
  			}	
			printf("Connected to node %d\n", ID); 
			peer2sock.insert(std::pair<int, int>(ID, sockfd[i]));
			sock2peer.insert(std::pair<int, int>(sockfd[i], ID));

			FILE *prikeyfp = fopen(privatekeyfile.c_str(), "r");
			if( prikeyfp == NULL ) printf("File Open %s error\n", privatekeyfile.c_str());
			RSA *priRkey = PEM_read_RSAPrivateKey(prikeyfp, NULL, NULL, NULL);
			if( priRkey == NULL) printf("Read Private Key for RSA Error\n"); 
			char *buffer = (char*)malloc(RSA_size(priRkey));
    		int n = read(sockfd[i], buffer, RSA_size(priRkey));
    		if (n < 0) printf("ERROR reading from socket \n");
			char *decrypt = (char*)malloc(n);
			memset(decrypt, 0x00, n); 
			int dec_len = RSA_private_decrypt(n, (unsigned char*)buffer, (unsigned char*)decrypt, priRkey, RSA_PKCS1_OAEP_PADDING);
			if(dec_len < 1) printf("RSA private decrypt error\n");
			memcpy(peerKeyIV, decrypt, 32);
			init_keys(ID, 1);
			free(buffer);
			free(decrypt);
		}
}
	
void NodeNetwork::acceptPeers(int numOfPeers){
		KeyIV = (unsigned char*)malloc(32);
		int sockfd, maxsd, portno, on = 1; 
		int *newsockfd = (int*)malloc(sizeof(int) * numOfPeers); 
		socklen_t *clilen = (socklen_t*)malloc(sizeof(socklen_t) * numOfPeers); 
		struct sockaddr_in serv_addr;
		struct sockaddr_in *cli_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in) * numOfPeers);  
			
		fd_set master_set, working_set; 
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		// see comment for fcntl above
		//fcntl(sockfd, F_SETFL, O_NONBLOCK);  
		if(sockfd < 0)
			fprintf(stderr, "ERROR, opening socket\n"); 
		int rc = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)); 
		rc = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char*)&on, sizeof(on));  
		if(rc < 0)
			printf("setsockopt() or ioctl() failed\n"); 	
		bzero((char*) &serv_addr, sizeof(serv_addr)); 
		portno = config->getPort(); 
		serv_addr.sin_family = AF_INET; 
		serv_addr.sin_addr.s_addr = INADDR_ANY; 
		serv_addr.sin_port = htons(portno); 
		if((bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))) < 0)
			printf("ERROR, on binding \n"); 
		listen(sockfd, 7); 
		// start to accept connections
		FD_ZERO(&master_set); 
		maxsd = sockfd; 
		FD_SET(sockfd, &master_set); 
		for(int i = 0; i < numOfPeers; i++){
			memcpy(&working_set, &master_set, sizeof(master_set)); 
			rc = select(maxsd+1, &working_set, NULL, NULL, NULL);  
			if(rc <= 0)
				printf("select failed or time out \n"); 
			if(FD_ISSET(sockfd, &working_set)){
				clilen[i] = sizeof(cli_addr[i]); 
				newsockfd[i] = accept(sockfd, (struct sockaddr*) &cli_addr[i], &clilen[i]); 
				if(newsockfd[i] < 0)
					fprintf(stderr, "ERROR, on accept\n");
				// see comment for fcntl above
 				//fcntl(newsockfd[i], F_SETFL, O_NONBLOCK);
				peer2sock.insert(std::pair<int, int>(config->getID() - (i+1), newsockfd[i]));
				sock2peer.insert(std::pair<int, int>(newsockfd[i], config->getID() - (i+1)));

				unsigned char key_iv[32]; 
				RAND_status();
				if( !RAND_bytes(key_iv, 32) )
					printf("Key, iv generation error\n");
				memcpy(KeyIV, key_iv, 32);
				int peer = config->getID() - (i+1);
				FILE *pubkeyfp = fopen((config->getPeerPubKey(peer)).c_str(), "r");
				if( pubkeyfp == NULL ) printf("File Open %s error \n", (config->getPeerPubKey(peer)).c_str());
				RSA *publicRkey = PEM_read_RSA_PUBKEY(pubkeyfp, NULL, NULL, NULL); 
				if( publicRkey == NULL) printf("Read Public Key for RSA Error\n"); 
				char *encrypt = (char*)malloc(RSA_size(publicRkey));
				memset(encrypt, 0x00, RSA_size(publicRkey));
				int enc_len = RSA_public_encrypt(32, KeyIV, (unsigned char*)encrypt, publicRkey, RSA_PKCS1_OAEP_PADDING);
				if(enc_len < 1) printf("RSA public encrypt error\n");
				int n = write(newsockfd[i], encrypt, enc_len);
     			if (n < 0) printf("ERROR writing to socket \n");
				init_keys(peer, 0);
				free(encrypt);
			}
		}
}


void NodeNetwork::init_keys(int peer, int nRead){
	unsigned char key[16],iv[16];
	memset(key, 0x00, 16);
	memset(iv, 0x00, 16);
	if(0 == nRead) //useKey KeyIV
	{		
		memcpy(key, KeyIV, 16);
		memcpy(iv, KeyIV+16, 16);
	}
	else //getKey from peers
	{
		memcpy(key, peerKeyIV, 16);
		memcpy(iv, peerKeyIV+16, 16);
	}
	EVP_CIPHER_CTX_init(&en);
	EVP_EncryptInit_ex(&en,EVP_aes_128_cbc(), NULL, key, iv);
	EVP_CIPHER_CTX_init(&de);
	EVP_DecryptInit_ex(&de,EVP_aes_128_cbc(), NULL, key, iv);
	peer2enlist.insert(std::pair<int, EVP_CIPHER_CTX>(peer, en));
	peer2delist.insert(std::pair<int, EVP_CIPHER_CTX>(peer, de));
}

void NodeNetwork::mpzFromString(char *str, mpz_t* no, int* lengths, int size){

	for(int i = 0; i < size; i++){
		char temp[lengths[i]]; // = (char*)malloc(sizeof(char) * lengths[i]);
		memcpy(temp, str, lengths[i]);
		temp[lengths[i]] = '\0';
		str += lengths[i];
		mpz_set_str(no[i], temp, base);
	}
}

void NodeNetwork::getRounds(int size, int *count, int *rounds)
{
    int bits = config->getBits();  
    int peers = config->getPeerCount(); 
    int numb = 8 * sizeof(char);
    int unit_size = (bits+numb-1)/numb;
    *count = MAX_BUFFER_SIZE/(peers+1)/unit_size;
    if(size % (*count) != 0)
    	*rounds = size/(*count);	
    else
	*rounds = size/(*count)-1; 
}

double NodeNetwork::time_diff(struct timeval *t1, struct timeval *t2){
	double elapsed;
	if(t1->tv_usec > t2->tv_usec){
		t2->tv_usec += 1000000;
		t2->tv_sec--;
	}

	elapsed = (t2->tv_sec-t1->tv_sec) + (t2->tv_usec - t1->tv_usec)/1000000.0;

	return(elapsed);
}

int NodeNetwork::getID(){
	return config->getID();
}

int NodeNetwork::getNumOfThreads(){
	return numOfThreads; 
}
unsigned char *NodeNetwork::aes_encrypt(EVP_CIPHER_CTX *e, unsigned char *plaintext, int *len){
	int c_len = *len + AES_BLOCK_SIZE;
	int f_len = 0;
	unsigned char *ciphertext = (unsigned char*) malloc(c_len);

	EVP_EncryptInit_ex(e, NULL, NULL, NULL, NULL);
	EVP_EncryptUpdate(e,ciphertext, &c_len, plaintext, *len);
	EVP_EncryptFinal_ex(e, ciphertext+c_len, &f_len);
	*len = c_len + f_len;
	return ciphertext;
}

unsigned char *NodeNetwork::aes_decrypt(EVP_CIPHER_CTX *e, unsigned char *ciphertext, int *len){
	int p_len = *len;
	int f_len = 0;
	unsigned char *plaintext = (unsigned char*)malloc(p_len + AES_BLOCK_SIZE);

	EVP_DecryptInit_ex(e, NULL, NULL, NULL, NULL);
	EVP_DecryptUpdate(e, plaintext, &p_len, ciphertext, *len);
	EVP_DecryptFinal_ex(e, plaintext+p_len, &f_len);
	*len = p_len  + f_len;
	return plaintext;
}

void NodeNetwork::closeAllConnections(){
}
