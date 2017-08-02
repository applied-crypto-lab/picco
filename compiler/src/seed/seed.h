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

#ifndef SEED_H_
#define SEED_H_
#include <gmp.h> //armk

class seed {

public:
	seed();
	void run();
	void init(char*, char*); //
	int parseConfigFile(char*);
	void parseUtilConfigFile(char*); //
	void sendPolynomials(mpz_t); //
	static double time_diff(struct timeval *t1, struct timeval *t2);
	std::string mpz2string(mpz_t, int); //
private: 
	int peers;
	mpz_t modulus; //
};
#endif /* SEED_H_ */
