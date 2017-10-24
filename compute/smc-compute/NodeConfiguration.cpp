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
#include "NodeConfiguration.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <string.h>
#include <cstdlib>

NodeConfiguration::NodeConfiguration() {}

NodeConfiguration::NodeConfiguration(int nodeID, std::string configFile, int sizeOfModulus){
	bits = sizeOfModulus; 
	id = nodeID;
	loadConfig(configFile);
}
int NodeConfiguration::getBits()
{
	return bits; 
}
int NodeConfiguration::getID(){
	return id;
}

std::string NodeConfiguration::getIP(){
	return ip;
}

int NodeConfiguration::getPort(){
	return port;
}

int NodeConfiguration::getPeerCount(){
	return peerIP.size();
}
std::string NodeConfiguration::getPeerIP(int id){
	//Get the index of the id;
	int index=-1;
	for(int i=0; i<peerID.size(); ++i){
		if(peerID[i] == id){
			index = i;
			break;
		}
	}
	return peerIP[index];
}
int NodeConfiguration::getPeerPort(int id){
	//Get the index of the id;
	int index=-1;
	for(int i=0; i<peerID.size(); ++i){
		if(peerID[i] == id){
			index = i;
			break;
		}
	}
	return peerPort[index];
}

int NodeConfiguration::getPeerIndex(int id){
	//Get the index of the id
	for(int i=0; i<peerID.size(); ++i){
		if(peerID[i] == id)
			return i;
	}

}

std::string NodeConfiguration::getPubKey(){
	return pubKey;
}

std::string NodeConfiguration::getPeerPubKey(int id){
	//Get the index of the id;
	int index=-1;
	for(int i=0; i<peerID.size(); ++i){
		if(peerID[i] == id){
			index = i;
			break;
		}
	}
	return peerPubKey[index];
}

/*
 * Loads the network configuration from the file.
 * Assumes that the config file is sorted
 */
void NodeConfiguration::loadConfig(std::string configFile){
	std::ifstream configIn(configFile.c_str(), std::ios::in);
	//Make sure the file exists and can be opened
	if(!configIn){
		std::cout << "File could not be opened";
		std::exit(1);
	}
	std::string line;
	std::vector<std::string> tokens;
	//Read each line of the configuration file
	while(std::getline(configIn, line)){
		tokens.clear();
		char* tok = strdup(line.c_str());
		tok = strdup(line.c_str());
		tok = strtok(tok, ",");
		std::string str;
		while(tok != NULL){
		    str = tok;
		    tokens.push_back(str);
		    tok = strtok(NULL, ",");
		}
		free(tok);
		if(id == atoi(tokens[0].c_str())){
			ip = tokens[1];
			port = atoi(tokens[2].c_str());
			pubKey = tokens[3];
		}
		else{
			peerID.push_back(atoi(tokens[0].c_str()));
			peerIP.push_back(tokens[1]);
			peerPort.push_back(atoi(tokens[2].c_str()));
			peerPubKey.push_back(tokens[3]);
		}
	}
	configIn.close();
}

NodeConfiguration::~NodeConfiguration() {

}
