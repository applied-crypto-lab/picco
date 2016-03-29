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
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <iostream>
#include <sys/time.h>
#include <vector>
#include "SecretShare.h"
#include "seed.h"
#include "PRSS.h"
using boost::asio::ip::tcp;

std::vector<int> computeNodes;
std::vector<std::string> computeIPs;
std::vector<int> computePorts;

int main(int argc, char* argv[])
{
	if(argc < 1)
		std::cout << "Missing input parameters";

	seed c;
	c.init(argv[1]);
}

seed::seed(){

}


void seed::sendPolynomials(){
		PRSS prss(peers);
		srand(1000);
		prss.setKeysAndPoints();
		prss.setPolynomials();
		std::cout<<"set polynomials"<<std::endl; 
		// for each compute node, send the corresponding coefficients.
		for(int i = 0; i < peers; i++){
            std::vector<int> indices;
			std::vector<long> keys;
            std::map<long, std::vector<int> > polys;
            int size =  prss.getKeys().size();
			std::vector<int> coefficients;
            
            for(int k = 0; k < size; k++)
            {
                std::vector<int> pts = (prss.getPoints().find(prss.getKeys().at(k))->second);
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
                    keys.push_back(prss.getKeys().at(k));
                    polys.insert(std::pair<long, std::vector<int> >(prss.getKeys().at(k), prss.getPolynomials().find(prss.getKeys().at(k))->second));
                }
            }
            
			std::map<long, std::vector<int> >::iterator it;
            for(int k = 0; k < keys.size(); k++)
            {
                    for(int j = 0; j < (peers+1)/2; j++)
                        coefficients.push_back(polys.find(keys.at(k))->second.at(j));
            }
			long* Keys = &keys[0];
			int* Coefficients = &coefficients[0];
            int keysize = keys.size();
            int coefsize = coefficients.size();
            write(computeNodes[i], &keysize, sizeof(int));
            write(computeNodes[i], &coefsize, sizeof(int));
	    write(computeNodes[i], Keys, sizeof(long) * keys.size());
	    write(computeNodes[i], Coefficients, sizeof(int) * coefficients.size()); 
	    printf("Secret seeds have been successfully sent to each of computational parties...\n"); 
	}
}
void seed::init(char* config){
	//Parse the configuration file to get network information
	peers = parseConfigFile(config);
	
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
	sendPolynomials();
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
		std::cout << "File could not be opened";
		std::exit(1);
	}

	std::string line;
	std::vector<std::string> tokens;
	
	//Read each line of the configuration file
	while(std::getline(configIn, line)){
		peers++; 
		boost::split(tokens, line, boost::is_any_of(","));
		computeIPs.push_back(tokens[1]);
		computePorts.push_back(atoi(tokens[2].c_str()));
	}
	configIn.close();
	return peers; 
}


