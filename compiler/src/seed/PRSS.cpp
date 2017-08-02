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
#include <gmp.h> 
#include <unistd.h>  
#include <fcntl.h> 
#include <sys/stat.h>  
#include <sys/types.h>  
#include <stdlib.h>  
#include <stdio.h> 
#include <string.h> 

PRSS::PRSS(int p, mpz_t mod) {
	// TODO Auto-generated constructor stub
	mpz_init(modulus);
	mpz_set(modulus, mod);
	peers = p;
	threshold = (p-1)/2;

	char strkey[256] = {0,}; 
	mpz_get_str(strkey, 10, modulus);
	mpz_t_size = strlen(strkey);
}

PRSS::~PRSS() {
	// TODO Auto-generated destructor stub
}

void PRSS::setPeers(int p){
	peers = p;
}

void PRSS::setKeysAndPoints(){
	std::vector<int> elements;
	std::vector<int> pos(threshold);
	std::vector<std::vector<int> > result;
	
	int fd;
	char seed[4], temp[13], strSeed[13] = {0,};
	if ((fd = open("/dev/random", O_RDONLY)) == -1)
	{
		perror("open error");
		exit(1);
	}
	if ((read(fd, seed, 4)) == -1)
	{
		perror("read error");
		exit(1);
	}
	for (int i = 0; i < 4; i++)
	{
		sprintf(temp+(i*3), "%03d", seed[i] & 0xff);
		memcpy(strSeed+(i*3), temp+(i*3), 3);
	}
	mpz_t randSeed, randNum; 
	mpz_init(randSeed);
	mpz_set_str(randSeed, strSeed, 12);
	mpz_init(randNum);           
	gmp_randstate_t gmpRandState; 
	gmp_randinit_default(gmpRandState); 
	gmp_randseed(gmpRandState, randSeed); 
	for(int i = 1; i <= getPeers(); i++)
		elements.push_back(i);
	getCombinations(elements, threshold, pos, 0, 0, result);
	keysize = result.size();
	keys = (mpz_t*)malloc(sizeof(mpz_t) * keysize);
	/*points contain the maximum unqualified set*/
	for(unsigned int i = 0; i < result.size(); i++){
	  //	long key = (rand() % fieldsize/3)+1;
	  // mb: I changed the key to be just random field element
	        //key = rand() % fieldsize;
		mpz_urandomm(randNum, gmpRandState, modulus); 
		mpz_init(keys[i]); 
		mpz_set(keys[i], randNum); 
		std::string Strkey = mpz2string(randNum, mpz_t_size); 
		points.insert(std::pair<std::string, std::vector<int> >(Strkey, result.at(i))); 
	}
}

void PRSS::getCombinations(std::vector<int> &elements, int reqLen, std::vector<int> &pos, int depth, int margin, std::vector<std::vector<int> > &result){
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
void PRSS::setPolynomials(){
	std::map<std::string, std::vector<int> > poly; 
	int degree = getThreshold();
	std::vector<int>::iterator it;
	//	SecretShare ss(peers, fieldsize);
	mpz_t* tempKey = (mpz_t*)malloc(sizeof(mpz_t) * (keysize)); 
	tempKey = getKeys(); 
	for(unsigned int i = 0; i < keysize; i++){ 
		std::string Strkey = mpz2string(tempKey[i], mpz_t_size); 
		std::vector<int> coefficients;
		std::vector<int> pts = (getPoints().find(Strkey)->second); 
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
		poly.insert(std::pair<std::string, std::vector<int> >(Strkey, coefficients)); 
	}
	polynomials = poly;
}

int PRSS::computePolynomials(std::vector<int> polys, int point){
	int result = 0;
	for(unsigned int i = 0; i < polys.size(); i++)
		result += pow(point, (polys.size()-1-i)) * polys[i];
	return result;
}

int PRSS::getPeers(){
	return peers;
}

int PRSS::getThreshold(){
	return threshold;
}

mpz_t* PRSS::getKeys(){ 
	return keys;
}

int PRSS::getKeysize(){ 
	return keysize;
}

std::map<std::string, std::vector<int> > PRSS::getPolynomials(){ 
	return polynomials;
}

std::map<std::string, std::vector<int> > PRSS::getPoints(){ 
	return points;
}

std::string PRSS::mpz2string(mpz_t value, int buf_size)
{
	char str[buf_size+1] = {0,};
	mpz_get_str(str, 10, value);
	std::string Str = str;
	return Str;
} 
