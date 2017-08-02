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

#ifndef PRSS_H_
#define PRSS_H_
#include <map>

#include "stdint.h"
#include <vector>
#include <iostream>
#include <cstdlib>
#include <gmp.h>
class PRSS {
public:
	PRSS(int, mpz_t); 
	virtual ~PRSS();
	void setKeysAndPoints();
	void setPeers(int);
	void setThreshold(int);
	void setPolynomials();
	int getPeers();
	int getThreshold();
	long getFieldSize();
	mpz_t* getKeys(); 
	int getKeysize(); 
	std::map<std::string, std::vector<int> > getPolynomials(); 
	std::map<std::string, std::vector<int> > getPoints(); 
	int computePolynomials(std::vector<int>, int);
	std::string mpz2string(mpz_t, int); 
private:
	std::map<std::string, std::vector<int> > polynomials;
	std::map<std::string, std::vector<int> > points;
	mpz_t* keys; 
	int keysize; 
	void getCombinations(std::vector<int> &, int, std::vector<int> &, int, int, std::vector<std::vector<int> > &);
	int peers;
	int threshold;
	mpz_t modulus; 
	int mpz_t_size; 
};

#endif /* PRSS_H_ */
