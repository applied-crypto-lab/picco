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

#include <iostream>
#include <string.h>
#include <netdb.h> 
#include <fstream>
#include <iostream>
#include <sys/time.h>
#include <vector>
#include "SecretShare.h"
#include "seed.h"
#include "PRSS.h"
#include <openssl/rand.h> 
#include "unistd.h" 
#include <openssl/rsa.h> 
#include <openssl/pem.h>
#include "openssl/bio.h"
#include <gmp.h> 

std::vector<int> computeNodes;
std::vector<std::string> computeIPs;
std::vector<int> computePorts;
std::vector<std::string> computePubkeys; 

int main(int argc, char* argv[])
{
	if(argc < 2){ 
		fprintf(stderr,"Incorrect input parameters \n");
		fprintf(stderr,"Usage: picco-seed <runtime-config> <utility-config> \n");
		exit(1);
	}

	seed c;
	c.init(argv[1], argv[2]); 
}

seed::seed(){

}


void seed::sendPolynomials(mpz_t mod){
	PRSS prss(peers, mod);
	char *strkey = (char*)malloc(64);
	mpz_get_str(strkey, 10, modulus);
	int mpz_t_size = strlen(strkey);

	prss.setKeysAndPoints();
	prss.setPolynomials();
	std::cout<<"set polynomials"<<std::endl; 
	// for each compute node, send the corresponding coefficients.
	for(int i = 0; i < peers; i++){
		std::vector<int> indices;
		std::map<std::string, std::vector<int> > polys; 
		int size = prss.getKeysize(); 
		std::vector<int> coefficients;
		int keyssize = 0; 
		mpz_t* tempKey = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	        tempKey = prss.getKeys(); 
		for(int k = 0; k < size; k++)
		{
			std::string Strkey = mpz2string(tempKey[k], mpz_t_size); 
			std::vector<int> pts = (prss.getPoints().find(Strkey)->second); 
			for(int j = 0; j < pts.size(); j++)
			{
			    if(pts.at(j) == i+1)
			    {
				indices.push_back(k);
				break;
			    }
			}
		}
            
		for(int k = 0; k < size; k++)
		{
			std::string Strkey = mpz2string(tempKey[k], mpz_t_size); 
			int flag = 0;
			for(int j = 0; j < indices.size(); j++)
			{
			    if(indices.at(j) == k)
			    {
				flag = 1;
				break;
			    }
			}
			if(flag == 0)
			{
				std::string Strkey = mpz2string(tempKey[k], mpz_t_size); 
				polys.insert(std::pair<std::string, std::vector<int> >(Strkey, prss.getPolynomials().find(Strkey)->second)); 
				keyssize += 1; 
				for(int j = 0; j < (peers+1)/2; j++)
					coefficients.push_back(polys.find(Strkey)->second.at(j)); 
			}
		}
            
		int* Coefficients = &coefficients[0];
		int coefsize = coefficients.size();
		FILE *pubkeyfp = fopen(computePubkeys[i].c_str(), "r");
		if( pubkeyfp == NULL ) printf("File Open %s error \n", computePubkeys[i].c_str());
		RSA *publicRkey = PEM_read_RSA_PUBKEY(pubkeyfp, NULL, NULL, NULL);
		if( publicRkey == NULL) printf("Read Public Key for RSA Error\n");
		char *encrypt = (char*)malloc(RSA_size(publicRkey));
		memset(encrypt, 0x00, RSA_size(publicRkey));
		int buf_size = sizeof(int)*(3+coefficients.size()) + mpz_t_size*keyssize + 1; 
		char *buf = (char*)malloc(buf_size);
		memset(buf, 0x00, buf_size);
		memcpy(buf, &keyssize, sizeof(int));
		memcpy(buf+sizeof(int), &coefsize, sizeof(int));
		memcpy(buf+sizeof(int)*2, &mpz_t_size, sizeof(int));
		int position = 0;
		for(int l = 0; l < size; l++){
			int flag = 0;
			for(int j = 0; j < indices.size(); j++)
			{
			    if(indices.at(j) == l)
			    {
				flag = 1;
				break;
			    }
			}
			if(flag == 0)
			{
				char *strkey = (char*)malloc(mpz_t_size+1);
				mpz_get_str(strkey, 10, tempKey[l]); 
				memcpy(buf+sizeof(int)*3+position, strkey, mpz_t_size);
				position += mpz_t_size;
			}
		}
		memcpy(buf+sizeof(int)*3+position, Coefficients, sizeof(int)*coefficients.size()); 
		int enc_len = RSA_public_encrypt(buf_size, (unsigned char*)buf, (unsigned char*)encrypt, publicRkey, RSA_PKCS1_OAEP_PADDING);
		if(enc_len < 1) printf("RSA public encrypt error\n");
 		write(computeNodes[i], encrypt, enc_len); 
		printf("Secret seeds have been successfully sent to each of computational parties...\n"); 
	}
}
void seed::init(char* config, char* util_config){ 
	//Parse the configuration file to get network information
	peers = parseConfigFile(config);
	parseUtilConfigFile(util_config); 
	std::cout << "Config file parsed successfully\n" << peers << std::endl;
	try
	{
		for(int i = 0; i < computeIPs.size(); i++){
			int sockfd, portno;
                	struct sockaddr_in serv_addr;
                	struct hostent *server;
                	portno = computePorts[i]+100; 
                	sockfd = socket(AF_INET, SOCK_STREAM, 0);
                	if(sockfd < 0)
                        	fprintf(stderr, "ERROR, opening socket\n");
                	server = gethostbyname(computeIPs[i].c_str());
                	if(server == NULL)
                        	fprintf(stderr, "ERROR, no such hosts \n");
                	bzero((char*) &serv_addr, sizeof(serv_addr));
			serv_addr.sin_family = AF_INET; 
		 	bcopy((char*) server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
               		serv_addr.sin_port = htons(portno);
                	if(connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
                        	perror("ERROR, connecting to node");
			computeNodes.push_back(sockfd);  
                	printf("Connected to node %d\n", i+1);
		}
	}
	catch (std::exception& e)
	{
		std::cout << "An exception was caught: " << e.what() << "\n";
	}
	std::cout<<"Connect to compute nodes\n";
	// send the polynomials to compute nodes for random value generation.	
	sendPolynomials(modulus); 
}


//Calculate the time difference in seconds between two time intervals
double seed::time_diff(struct timeval *t1, struct timeval *t2){
	double elapsed;

	if(t1->tv_usec > t2->tv_usec){
		t2->tv_usec += 1000000;
		t2->tv_sec--;
	}

	elapsed = (t2->tv_sec-t1->tv_sec) + (t2->tv_usec - t1->tv_usec)/1000000.0;

	return(elapsed);
}

int seed::parseConfigFile(char* config){
	std::ifstream configIn(config, std::ios::in);
	int peers = 0; 
	//Make sure the file exists and can be opened
	if(!configIn){
		std::cout << "File could not be opened \n";
		std::exit(1);
	}

	std::string line;
	std::vector<std::string> tokens;
	
	//Read each line of the configuration file
	while(std::getline(configIn, line)){
		peers++; 
		tokens.clear();
		char* tok = strdup(line.c_str());
		tok = strtok(tok, ",");
		std::string str;
		while(tok != NULL){
		    str = tok;
		    tokens.push_back(str);
		    tok = strtok(NULL, ",");
		}
		free(tok);
		computeIPs.push_back(tokens[1]);
		computePorts.push_back(atoi(tokens[2].c_str()));
		computePubkeys.push_back(tokens[3]); 
	}
	configIn.close();
	return peers; 
}

void seed::parseUtilConfigFile(char* util_config){
	std::ifstream configIn(util_config, std::ios::in);
	if(!configIn){
		std::cout << "Utility Config File could not be opened \n";
		std::exit(1);
	}
	std::string line;
	std::vector<std::string> tokens;
	for(int i = 0; i < 2; i++)
	{
		std::getline(configIn, line);
		if(i == 1)
		{
			char* tok = strdup(line.c_str());
			tok = strtok(tok, ":");
			tok = strtok(NULL, ":");
			mpz_init(modulus);
			mpz_set_str(modulus, tok, 10);
		}
	}
	configIn.close();
}

std::string seed::mpz2string(mpz_t value, int buf_size)
{
	char *str = (char*)malloc(buf_size+1);
	mpz_get_str(str, 10, value);
	std::string Str = str;
	return Str;
} 
