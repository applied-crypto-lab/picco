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
#include "NodeConfiguration.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string.h>
#include <string>
#include <vector>
#include <regex>

NodeConfiguration::NodeConfiguration() {}

NodeConfiguration::NodeConfiguration(int nodeID, std::string configFile, int sizeOfModulus, int number_of_peers) {
    bits = sizeOfModulus;
    id = nodeID;
    peers = number_of_peers;
    loadConfig(configFile);
}
int NodeConfiguration::getBits() {
    return bits;
}
int NodeConfiguration::getID() {
    return id;
}

std::string NodeConfiguration::getIP() {
    return ip;
}

int NodeConfiguration::getPort() {
    return port;
}

int NodeConfiguration::getPeerCount() {
    return peers;
}
std::string NodeConfiguration::getPeerIP(int id) {
    // Get the index of the id;
    int index = -1;
    for (int i = 0; i < peerID.size(); ++i) {
        if (peerID[i] == id) {
            index = i;
            break;
        }
    }
    return peerIP[index];
}
int NodeConfiguration::getPeerPort(int id) {
    // Get the index of the id;
    int index = -1;
    for (int i = 0; i < peerID.size(); ++i) {
        if (peerID[i] == id) {
            index = i;
            break;
        }
    }
    return peerPort[index];
}

int NodeConfiguration::getPeerIndex(int id) {
    // Get the index of the id
    for (int i = 0; i < peerID.size(); ++i) {
        if (peerID[i] == id)
            return i;
    }
    return -1;
}

std::string NodeConfiguration::getPubKey() {
    return pubKey;
}

std::string NodeConfiguration::getPeerPubKey(int id) {
    // Get the index of the id;
    int index = -1;
    for (int i = 0; i < peerID.size(); ++i) {
        if (peerID[i] == id) {
            index = i;
            break;
        }
    }
    return peerPubKey[index];
}

/*
 * Loads the network configuration from the file.
 * Assumes that the config file is sorted
 * If in measurement mode, the public key is skipped
 */
void NodeConfiguration::loadConfig(std::string configFile) {
    std::ifstream configIn(configFile.c_str(), std::ios::in);
    // Make sure the file exists and can be opened
    if (!configIn) {
        std::cout << "ERROR: runtime-config could not be opened, exiting...\n";
        std::exit(1);
    }
    std::string line;
    std::vector<std::string> tokens;

    int currentID = 1; // Variable to track the current ID
    int numOfLines = 0; // Counter to track the number of lines 
    std::regex ipPattern(R"(^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$)"); // Expression pattern for IP address validation

    int expectedNumOfPeers = NodeConfiguration::getPeerCount()+1;
    
    // Read each line of the configuration file
    while (std::getline(configIn, line)) {
        numOfLines++;
        tokens.clear();
        char *tok = strdup(line.c_str());
        tok = strdup(line.c_str());
        tok = strtok(tok, ",");
        std::string str;
        while (tok != NULL) {
            str = tok;
            tokens.push_back(str);
            tok = strtok(NULL, ",");
        }
        free(tok);

        if (tokens.size() != 4) { // This is the token size, [#of-party, ip, port, key-file]
            std::cerr << "ERROR: Malformed line in runtime-config file: " << line << " exiting...\n";
            std::exit(1);
        }

        // Check for empty tokens
        for (const auto &t : tokens) {
            if (t.empty()) {
                std::cerr << "ERROR: Empty token in runtime-config file: " << line << " exiting...\n";
                std::exit(1);
            }
        }

        // Validate ID (should be an integer)
        if (!std::regex_match(tokens[0], std::regex(R"(\d+)"))) {
            std::cerr << "ERROR: Invalid ID in runtime-config file: " << line << " exiting...\n";
            std::exit(1);
        }

        // Validate IP address format
        if (!std::regex_match(tokens[1], ipPattern)) {
            std::cerr << "ERROR: Invalid IP address in runtime-config file: " << line << " exiting...\n";
            std::exit(1);
        }

        // Validate port number (should be an integer)
        if (!std::regex_match(tokens[2], std::regex(R"(\d+)"))) {
            std::cerr << "ERROR: Invalid port number in runtime-config file: " << line << " exiting...\n";
            std::exit(1);
        }

        // Ensure public key is provided
        if (tokens[3].empty()) {
            std::cerr << "ERROR: Missing public key in runtime-config file: " << line << " exiting...\n";
            std::exit(1);
        }

        // Validate ID sequence
        if (atoi(tokens[0].c_str()) != currentID) {
            std::cerr << "ERROR: ID sequence in runtime-config file is not sequential or does not start from 1, exiting...\n";
            std::exit(1);
        }
        currentID++;

        if (id == atoi(tokens[0].c_str())) {
            ip = tokens[1];
            port = atoi(tokens[2].c_str());
#if __DEPLOYMENT__
            pubKey = tokens[3];
#endif
        } else {
            peerID.push_back(atoi(tokens[0].c_str()));
            peerIP.push_back(tokens[1]);
            peerPort.push_back(atoi(tokens[2].c_str()));
#if __DEPLOYMENT__
            peerPubKey.push_back(tokens[3]);
#endif
        }
    }

    // Check if the total number of lines processed matches the expected number
    if (numOfLines != expectedNumOfPeers) {
        std::cerr << "ERROR: Number of lines in runtime-config file does not match expected number of peers, exiting...\n";
        std::exit(1);
    }

    configIn.close();
}

NodeConfiguration::~NodeConfiguration() {
}
