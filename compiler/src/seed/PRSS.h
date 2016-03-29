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
class PRSS {
public:
	PRSS(int);
	virtual ~PRSS();
	void setKeysAndPoints();
	void setPeers(int);
	void setThreshold(int);
	void setPolynomials();
	int getPeers();
	int getThreshold();
	long getFieldSize();
	std::vector<long> getKeys();
	std::map<long, std::vector<int> > getPolynomials();
	std::map<long, std::vector<int> > getPoints();
	std::vector<long> packKeys(std::map<long, std::vector<int> >);
	std::vector<std::vector<int> > packPolynomials(std::map<long, std::vector<int> >);
	long generateRandValue(long, int);
	std::vector<long> generateRandValues(long);
	long PRandInt(int, int);
	long PRandBit(std::vector<long>, int);
	std::vector<long> PRandM(int, int, int);
	long Mod2(std::vector<long> A, int K);
	long BitLTC(long, std::vector<long>);
	std::vector<std::vector<long> > preMul(std::vector<std::vector<long> >);
	long EQZ(std::vector<long>, int);
	// helper functions
	std::vector<long> binarySplit(long, int);
	int computePolynomials(std::vector<int>, int);
	// test functions.
	void testKeysAndPoints();
	void testPolynomials();
	void testPacking();
private:
	std::map<long, std::vector<int> > polynomials;
	std::map<long, std::vector<int> > points;
	std::vector<long> keys;
	void getCombinations(std::vector<int> &, int, std::vector<int> &, int, int, std::vector<std::vector<int> > &);
	int peers;
	int threshold;
	long fieldsize;
	long fieldsize8;
};

#endif /* PRSS_H_ */
