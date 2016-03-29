/*
  OMPi OpenMP Compiler
  == Copyright since 2001 the OMPi Team
  == Department of Computer Science, University of Ioannina

  This file is part of OMPi.

  OMPi is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  OMPi is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OMPi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* x_clauses.h -- everything derived from openmp data clauses */

#ifndef __X_CLAUSES_H__
#define __X_CLAUSES_H__

#include "symtab.h"
#include "ast.h"
    

/*
 * Reduction
 */
/* Correct initializer depending on the reduction operator */
extern astexpr xc_reduction_initializer(int op, symbol var);
/* Produces a statement that declares and initializes 1 reduction var */
extern aststmt xc_reduction_declaration(symbol var, int redop);
/* Code for all reduction clauses of the directive */
aststmt xc_ompdir_reduction_code(ompdir t);

/*
 * Firstprivate
 */
extern aststmt xc_firstprivate_declaration(symbol var);
/* A memory copy statement, for initializing nonscalar first/lastprivate vars:
 *     memcpy( (void *) var, (void *) &(init), sizeof(var) );
 */
extern aststmt xc_array_initializer(symbol var, astexpr init);
/* Memory copying list of statements for all fip vars (firstprivate) */
extern aststmt xc_ompdir_fiparray_initializers(ompdir t);

/*
 * Lasstprivate
 */
/* Lastprivate assignments */
extern aststmt xc_ompdir_lastprivate_assignments(ompdir t);

/*
 * Declarations for all data clause vars
 */
extern aststmt xc_ompdir_declarations(ompdir t);

/*
 * Checks for duplicates & grouping
 */
/* Checks for duplicates AND keeps the list of vars; returns the table */
extern symtab xc_validate_store_dataclause_vars(ompdir d);
/* Only checks for duplicates */
extern void xc_validate_only_dataclause_vars(ompdir d);

/* The first one searches to see whether var appears in a type c
 * data clauses of d; returns 1 if yes, 0 if no.
 * The second one searches to find whether var appears in any of the
 * data clauses of d and, if found, it returns the clause type, else 0.
 */
extern int xc_isvar_in_dataclause(symbol var, ompdir d, enum clausetype c);
extern enum clausetype xc_dataclause_of_var(symbol var, ompdir d);

/*
 * Declarations from the vars collected by xc_validate_store_dataclause_vars().
 */
extern 
   aststmt xc_stored_vars_declarations(int *haslast, int *hasboth, int *hasred);

/* Take the clauses of a combined parallel-workshare statement and
 * split them; some are given to parallel and some to the workshare.
 * Parallel actually takes all the clauses it can take except for
 * private and firstprivate ones (so as to conform with the restrictions
 * on lastprivate clause that possibly exist)
 */
extern void xc_split_combined_clauses(ompclause all, 
                                      ompclause *parc, ompclause *wshc);

/*
 * Various utilities
 */
/* Replaces the identifier in-place with a pointer to it; returns decl. */
extern astdecl xc_decl_topointer(astdecl decl);

/* Replaces the name of the identifier; returns decl. */
extern astdecl xc_decl_rename(astdecl decl, symbol newname);

/* Applies rules to ensure validity & OpenMP compliance of all clauses.
 * It won't check the variables inside the clauses.
 */
extern void xc_validate_clauses(enum dircontype dirtype, ompclause t);

/* Returns the leftmost clause of the given type (or NULL) */
extern ompclause xc_ompcon_get_clause(ompcon t, enum clausetype type);

#endif
