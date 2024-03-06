/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2024 PICCO Team
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
#ifndef _INT2FL_HPP_
#define _INT2FL_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"

template <typename T>
void doOperation_Int2FL(T **values, T ***results1, int gamma, int K, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){}

#endif // _INT2FL_HPP_