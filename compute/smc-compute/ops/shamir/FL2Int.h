/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame
   ** Department of Computer Science and Engineering, University of Buffalo (SUNY)

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
#ifndef FL2INT_SHAMIR_H_
#define FL2INT_SHAMIR_H_

#include "FLRound.h"
#include "Inv.h"
#include "LTZ.h"
#include "Mod2MS.h"
#include "Mult.h"
#include "Operation.h"
#include "Pow2.h"

class FL2Int : public Operation {
public:
    FL2Int();
    FL2Int(NodeNetwork nodeNet, std::map<std::string, std::vector<int>> poly, int nodeID, SecretShare *s);
    virtual ~FL2Int();
    void doOperation(mpz_t **values, mpz_t *results, int K, int L, int gamma, int size, int threadID);

private:
    LTZ *Lt;
    FLRound *Flround;
    Mod2MS *Mod2ms;
    Inv *I;
};

#endif /* INT2FL_H */
