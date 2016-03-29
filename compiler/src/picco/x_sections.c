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

/* x_sections.c -- complete */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ast_free.h"
#include "ast_xform.h"
#include "ast_copy.h"
#include "x_sections.h"
#include "x_clauses.h"
#include "symtab.h"
#include "picco.h"


int count_statments(aststmt st)
{
  return ( (st->type == STATEMENTLIST) ? count_statments(st->u.next) + 1 : 1 );
}


/* For each SECTION construct, it creates a "case" statement to be
 * inserted into the xform_sections()-produced "switch" statement.
 */
static
aststmt statement_cases(aststmt st, int nsec)
{
  aststmt cases = NULL;
  
  if (st->type == STATEMENTLIST)
  {
    cases = statement_cases(st->u.next, nsec-1);
    st = st->body;
  }
  assert(st->type == OMPSTMT);
  assert(st->u.omp->type == DCSECTION);
  
  st = Case(numConstant(nsec-1), BlockList(st->u.omp->body, Break()));
  if (cases == NULL)
    cases = st;
  else
    cases = BlockList(cases, st);
  return (cases);
}


/* Makes "case" statements out of the "section" regions; the last
 * section is special in that it may contain lastprivate assignments.
 */
static
aststmt sections_cases(aststmt body, int nsec, aststmt lasts)
{
  aststmt st = NULL;
  
  if (lasts == NULL)
    return ( statement_cases(body, nsec) );
  
  /* Keep the last case to inject the lastprivate assignments */
  if (body->type == STATEMENTLIST)
  {
    st = statement_cases(body->u.next, nsec-1);
    body = body->body;
  }
  assert(body->type == OMPSTMT);
  assert(body->u.omp->type == DCSECTION);
  
  lasts = Case(
            numConstant(nsec-1),
            BlockList( BlockList(body->u.omp->body, lasts), Break() )
          );
  return ( st == NULL ? lasts : BlockList(st, lasts) );
}


/* possible clauses: 
 *   private, firstprivate, lastprivate, reduction, nowait
 *
 * The body is:
 *    Compound( OmpStmt(section) ) or Compound( BlockList )
 * where the BlockList is only OmpStmt(section)s.
 */
void xform_sections(aststmt *t)
{
  aststmt   s = (*t)->u.omp->body, parent = (*t)->parent, v,
            decls, inits = NULL, lasts = NULL, reds = NULL, stmp;
  int       nsec, haslast, hasboth, hasred;
  ompclause nw = xc_ompcon_get_clause((*t)->u.omp, OCNOWAIT);
  
  v = ompdir_commented((*t)->u.omp->directive); /* Put directive in comments */
  /*
   * Preparations
   */

  nsec = count_statments((*t)->u.omp->body->body);   /* Body of the compound */
     /* Collect all data clause vars - we need to check if any vars
      * are both firstprivate and lastprivate 
      */  
  xc_validate_store_dataclause_vars((*t)->u.omp->directive);
     /* declarations from the collected vars (not the clauses!) */
  decls = xc_stored_vars_declarations(&haslast, &hasboth, &hasred);
     /* initialization statments for firstprivate non-scalar vars */
  if (decls)
    inits = xc_ompdir_fiparray_initializers((*t)->u.omp->directive);
     /* assignments for lastprivate vars */
  if (haslast)
    /* if (hasboth), then we should block here till all threads entered;
     * but we do the easiest thing: we place a barrier (see below)
     */
    lasts = xc_ompdir_lastprivate_assignments((*t)->u.omp->directive);
     /* reduction code */
  if (hasred)
    reds = xc_ompdir_reduction_code((*t)->u.omp->directive);

     /* we need 2 more variables (caseid_ and inpar_) */
  stmp = verbit("int caseid_ = -1, inpar_;");
  decls = (decls) ? BlockList(decls,stmp) : stmp;
     /* checks for ditching the implicit barrier */

  if (nw==NULL && !xform_implicit_barrier_is_needed((*t)->u.omp))
    nw = (ompclause) 123;       /* Non-null anyway */

  /*
   * Do the job
   */
  
  (*t)->u.omp->body = NULL;     /* Make it NULL so as to free it easily */
  ast_free(*t);                 /* Get rid of the OmpStmt */
  
  if (inits)
    decls = BlockList(decls, inits);  /* Append the initialization statements */
  
  s = Switch(                          /* Create the "switch" out of the body */
        Identifier(Symbol("caseid_")),
        Compound( sections_cases(s->body, nsec, lasts) )
      );
    s = BlockList(
        verbit("for (;;)"),
        BlockList(
          Compound(
            BlockList(
              verbit(
                "\t if (inpar_){if ((caseid_=ort_get_section()) < 0) break;} "
                "\n\t else { if ((++caseid_) >= %d) break;}", nsec
              ),
              s
            )
          ),
          hasred ?
            BlockList(reds,
              verbit("if (inpar_) ort_leaving_sections();")) :
            verbit("if (inpar_) ort_leaving_sections();")
        )
      );
  if (hasboth)      /* Must make sure all threads have entered */
    s = BlockList(BarrierCall(), s);
  s = BlockList(
        BlockList(
          decls,
          verbit(
            "if ((inpar_ = (omp_in_parallel() && "
            "omp_get_num_threads() > 1)) != 0) "
            " \n\t ort_entering_sections(%d, %d);", nw ? 1 : 0, nsec)
        ),
        s
      );

  *t = BlockList(v, s);
  
  if (nw == NULL)      /* Must output a barrier */
    *t = BlockList(*t, BarrierCall());
  
  *t = Compound(*t);
  ast_stmt_parent(parent, *t);
}
