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

#include "PRSS.h"
#include "math.h"
#include "time.h"
#include "SecretShare.h"

PRSS::PRSS(int p) {
	// TODO Auto-generated constructor stub
		fieldsize = 2147483647L; //  31 bit modulus
		fieldsize8 = 251;
		peers = p;
		threshold = (p-1)/2;
}

PRSS::~PRSS() {
	// TODO Auto-generated destructor stub
}

void PRSS:: setPeers(int p){
	peers = p;
}

void PRSS:: setKeysAndPoints(){
	std::vector<int> elements;
	std::vector<int> pos(threshold);
	std::vector<std::vector<int> > result;
	long key;

	for(int i = 1; i <= getPeers(); i++)
		elements.push_back(i);
	getCombinations(elements, threshold, pos, 0, 0, result);
	/*points contain the maximum unqualified set*/
	for(unsigned int i = 0; i < result.size(); i++){
	  //	long key = (rand() % fieldsize/3)+1;
	  // mb: I changed the key to be just random field element
	        key = rand() % fieldsize;
		keys.push_back(key);
		points.insert(std::pair<long, std::vector<int> >(key, result.at(i)));
	}
}

void PRSS:: getCombinations(std::vector<int> &elements, int reqLen, std::vector<int> &pos, int depth, int margin, std::vector<std::vector<int> > &result){
	if(depth >= reqLen){
		std::vector<int> temp;
		for(unsigned int i = 0; i < pos.size(); i++)
			temp.push_back(elements.at(pos.at(i)));
		result.push_back(temp);
		return;
	}
	if((elements.size() - margin) < (unsigned int)(reqLen-depth))
		return;
	for(unsigned int i = margin; i < elements.size(); i++){
		pos.at(depth)=i;
		getCombinations(elements, reqLen, pos, depth+1, i+1, result);
	}
	return;
}

/* these polynomials are as described in section 3.1 of Cramer et al. TCC'05 
 * paper; one polynomial for maximum unqualified set.
 */
void PRSS:: setPolynomials(){
	std::map<long, std::vector<int> > poly;
	int degree = getThreshold();
	std::vector<int>::iterator it;
	//	SecretShare ss(peers, fieldsize);

	for(unsigned int i = 0; i < getKeys().size(); i++){
		std::vector<int> coefficients;
		std::vector<int> pts = (getPoints().find(getKeys().at(i))->second);
		std::vector<int> elements;

		/*computing coefficients for each polynomial*/
		/* mb: commented out old stuff */
		for(unsigned int j = 0; j < pts.size(); j++) 
			elements.push_back(0-pts.at(j));
		coefficients.push_back(1);

		for(int j = 1; j <= degree; j++){
			std::vector<int>pos(j);
			std::vector<std::vector<int> > result;
			getCombinations(elements, j, pos, 0, 0, result);
			int coef = 0;
			for(unsigned int m = 0; m < result.size(); m++){
				std::vector<int> temp = result.at(m);
				int tmp = 1;
				for(unsigned int n = 0; n < temp.size(); n++){
					tmp *= temp.at(n);
				}
				coef += tmp;
			}
			coefficients.push_back(coef);
		}
		poly.insert(std::pair<long, std::vector<int> >(getKeys().at(i), coefficients));
	}
	polynomials = poly;
}

int PRSS::computePolynomials(std::vector<int> polys, int point){
	int result = 0;
	for(unsigned int i = 0; i < polys.size(); i++)
		result += pow(point, (polys.size()-1-i)) * polys[i];
	return result;
}

int PRSS:: getPeers(){
	return peers;
}

int PRSS:: getThreshold(){
	return threshold;
}

long PRSS:: getFieldSize(){
	return fieldsize;
}

std::vector<long> PRSS:: getKeys(){
	return keys;
}

std::map<long, std::vector<int> > PRSS::getPolynomials(){
	return polynomials;
}

std::map<long, std::vector<int> > PRSS::getPoints(){
	return points;
}


void PRSS::testKeysAndPoints(){
	for(unsigned int i = 0; i < keys.size(); i++){
		std::cout<< keys.at(i)<<std::endl;
		for(unsigned int j = 0; j < (points.find(keys.at(i))->second).size(); j++)
			std::cout<< (points.find(keys.at(i))->second).at(j) <<std::endl;
	}
}

void PRSS:: testPolynomials(){
	long seed;
	std::vector<int> polys;
	std::map<long, std::vector<int> >::iterator it;
	for(it = polynomials.begin(); it != polynomials.end(); it++){
				seed = (*it).first;
				polys = (*it).second;
		std::cout<< seed <<" ";
		for(unsigned int i = 0; i < polys.size(); i++)
			std::cout<< polys.at(i) <<" ";
		std::cout<<std::endl;
	}
}
