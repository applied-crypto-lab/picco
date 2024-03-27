/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame
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

/* This file is modified from OMPi */

/* ompi.h -- the core */

#ifndef __OMPI_H__
#define __OMPI_H__

#include "../../../common/shared.h"

#include "scanner.h"
#include "symtab.h"
#include <stdbool.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef __APPLE__
#include <sys/types.h>
#endif

extern int enableOpenMP;      /* If 0, ignore OpenMP constructs */
extern int enableOmpix;       /* Enable OMPi-extensions */
extern char *filename;        /* The file we parse */
extern char *output_filename; /*The file in which the translated program will be stored*/
extern char *var_list;        /*The file that contains a list of private variables that need to be read from external files*/
extern char *final_list;
extern symtab stab;     /* The symbol table */
extern int testingmode; /* Internal tests */
extern int processmode; /* If 1, turn on process mode */
extern int threadmode;
extern int tmp_index;
extern int tmp_float_index;
extern int is_priv_int_ptr_appear;
extern int is_priv_float_ptr_appear;
extern int is_priv_int_index_appear;
extern int is_priv_float_index_appear;
extern int is_priv_int_struct_field_appear;
extern int is_priv_int_ptr_struct_field_appear;
extern int is_priv_float_struct_field_appear;
extern int is_priv_float_ptr_struct_field_appear;
extern int is_priv_struct_ptr_struct_field_appear;

extern int combinations; // number of RSS shares that are communciated (to be computed) 
extern int nu; // log (combinations)
extern int kappa_nu; // kappa + nu
extern int modulus; // this is the optimal modulus size that gets computed when the program is parsed
extern int thread_id;
extern int num_threads;
extern int contain_priv_if_flag;
extern int num_index;
extern int cond_index;
extern int enableCodeDup; /* Duplicate code where appropriate for speed */

/* This is for taking care of main() in the parsed code;
 * OMPi generates its own main() and replaces the original one.
 */
extern char *MAIN_NEWNAME;  /* Rename the original main */
extern int hasMainfunc;     /* 1 if main() function is defined in the file */
extern int mainfuncRettype; /* 0 = int, 1 = void */
extern int needMemcpy;      /* 1 if generated code includes memcpy() calls */
extern int needLimits;      /* 1 if need limits.h constants (min/max) */
extern int needFloat;       /* 1 if need float.h constants (min/max) */

/* These are implemented in parser.y
 */
extern aststmt parse_file(char *fname, int *error);
extern astexpr parse_expression_string(char *format, ...);
extern aststmt parse_blocklist_string(char *format, ...);
extern aststmt parse_and_declare_blocklist_string(char *format, ...);

/* Utilities
 */
extern void exit_error(int exitvalue, char *format, ...);
extern void warning(char *format, ...);
uint *generateSeedMap(uint n, uint t, uint *num_solutions) ;


extern int nChoosek(int n, int k);

// The constant variable for the technique_var: REPLICATED_SS-1 or SHAMIR_SS-2
extern int technique_var;

#endif
