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

#ifndef NODECONFIGURATION_H_
#define NODECONFIGURATION_H_

#include <vector>
#include <string>
class NodeConfiguration {

// TODO: Define a constant for the default configuration file. Update the NodeConfiguration(int) constructor with it.


public:
	NodeConfiguration();
	NodeConfiguration(int nodeID, std::string configFile, int bits);
	int getID();
	std::string getIP();
	int getPort();
	int getBits(); 
	int getPeerCount();
	int getPeerIndex(int id);
	std::string getPeerIP(int id);
	int getPeerPort(int id);
	virtual ~NodeConfiguration();
	std::string getPubKey();
	std::string getPeerPubKey(int id);
private:
	int id;
	int bits; 
	std::string ip;
	int port;
	std::vector<std::string> peerIP;
	std::vector<int> peerPort;
	std::vector<int> peerID;
	void loadConfig(std::string configFile);
	std::string pubKey;
	std::vector<std::string> peerPubKey;
};

#endif /* NODECONFIGURATION_H_ */
