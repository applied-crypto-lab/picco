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
#ifndef FLROUND_SHAMIR_H_
#define FLROUND_SHAMIR_H_

#include "EQZ.h"
#include "FLAdd.h"
#include "LTZ.h"
#include "Mod2M.h"
#include "Mod2MS.h"
#include "Mult.h"
#include "Operation.h"

class FLRound : public Operation {
public:
    FLRound(NodeNetwork nodeNet, std::map<std::string, std::vector<int>> poly, int nodeID, SecretShare *s);
    virtual ~FLRound();
    void doOperation(mpz_t **A, mpz_t **result, mpz_t *mode, int L, int K, int size, int threadID);

private:
    // LTZ *Lt;
    // Mod2M *Mod;
    // EQZ *Eq;
    Mod2MS *Md2m;
    // FLAdd *Fladd;
};

#endif /* FLROUND_SHAMIR_H_ */
