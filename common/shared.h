/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2013 PICCO Team
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

#ifndef _SHARED_CONSTANTS_H_
#define _SHARED_CONSTANTS_H_

// this header houses constants that are common for picco, and picco-util 

// Technique constants aren't used in the compute/ directory, since the techniques are handled directly by CMake, which is responsible for defining/setting the variables
#define SHAMIR_SS 2 
#define REPLICATED_SS 1

#define SECURITY_PARAMETER 48 // the security parameter kappa

//these constants ARE common with the compute directory. If they are changed here in compiler/, they must be changed in compute/
#define KEYSIZE 16
#define BASE 10

#endif