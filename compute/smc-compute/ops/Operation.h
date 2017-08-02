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
#ifndef OPERATION_H_
#define OPERATION_H_

#include <gmp.h>
#include <vector>
#include <map>
#include <cstdlib>
#include "../SecretShare.h"
#include "../NodeNetwork.h"

class Operation {
public:
	Operation();
	virtual ~Operation();
	void clearBuffer(mpz_t** buffer, int firstDimSize, int secondDimSize);
	void clearBuffer(mpz_t* buffer, int firstDimSize);
	void binarySplit(mpz_t v, mpz_t* result, int K);
	void binarySplit(int v, int* result, int K);
	double time_diff(struct timeval *, struct timeval *);
    
	SecretShare *ss;
	mpz_t coef[9];
	std::map<std::string, std::vector<int> > polynomials; //
	NodeNetwork net;
	int id;
};

#endif /* OPERATION_H_ */
