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

/* x_single.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ast_free.h"
#include "ast_xform.h"
#include "ast_copy.h"
#include "x_single.h"
#include "x_clauses.h"
#include "symtab.h"
#include "picco.h"


/* The next 3 functions create the function calls along with their
 * arguments, needed to implement copyprivate.
 */
static
int copyprivate_arguments_from_varlist(astdecl d,
                                       astexpr *sendargs, astexpr *recvargs)
{
  int     nvars = 0;
  astexpr sa, ra;
  
  if (d->type == DLIST && d->subtype == DECL_idlist)
  {
    nvars = copyprivate_arguments_from_varlist(d->u.next, sendargs, recvargs);
    d = d->decl;
  }
  if (d->type != DIDENT)
    exit_error(1, "[copyprivate_arguments_from_varlist]: !!BUG!! ?!\n");
    
  if (symtab_get(stab, d->u.id, IDNAME)->isarray ||    /* already ptrs */
         (threadmode && symtab_get(stab, d->u.id, IDNAME)->isthrpriv))
  {
    sa = Identifier(d->u.id);
    ra = CommaList(ast_expr_copy(sa), 
                   UnaryOperator(UOP_sizeof, 
                                 UnaryOperator(UOP_star, Identifier(d->u.id))));
  }
  else
  {
    sa = UnaryOperator(UOP_addr, Identifier(d->u.id));
    ra = CommaList(ast_expr_copy(sa), 
                   UnaryOperator(UOP_sizeof, Identifier(d->u.id)));
  }
  
  if (*sendargs == NULL)
  {
    *sendargs = sa;
    *recvargs = ra;
  }
  else
  {
    *sendargs = CommaList(*sendargs, sa);
    *recvargs = CommaList(*recvargs, ra);
  }
  return (nvars+1);
}


static
int copyprivate_arguments(ompclause t, astexpr *sendargs, astexpr *recvargs)
{
  int nvars = 0;
  
  *sendargs = NULL; 
  *recvargs = NULL;
  if (t == NULL) return 0;
  
  if (t->type == OCLIST)
  {
    if (t->u.list.next != NULL)
      nvars = copyprivate_arguments(t->u.list.next, sendargs, recvargs);
    t = t->u.list.elem;
    assert(t != NULL);
  }
  if (t->type == OCCOPYPRIVATE)
    nvars += copyprivate_arguments_from_varlist(t->u.varlist,sendargs,recvargs);
  return (nvars);
}


/* We don't do any check for existance; we assume that there exists
 * at least 1 copyprivate clause.
 */
static
void copyprivate_statements(ompcon t, aststmt *send, aststmt *recv)
{
  astexpr sendargs = NULL, recvargs = NULL;
  int     nvars;

  /* Create the list of arguments */
  nvars = copyprivate_arguments(t->directive->clauses,&sendargs,&recvargs);
  
  *send = Expression(         /* ort_broadcast_private(n, sendvars... ) */
            FunctionCall(
              Identifier(Symbol("ort_broadcast_private")),
              CommaList(numConstant(nvars), sendargs)
            )
          );
  *recv = Expression(         /* ort_copy_private(n, recvvars... ) */
            FunctionCall(
              Identifier(Symbol("ort_copy_private")),
              CommaList(numConstant(nvars), recvargs)
            )
          );
}


/* possible clauses: 
 *   private, firstprivate, copyprivate, nowait 
 */
void xform_single(aststmt *t)
{
  aststmt   s = (*t)->u.omp->body, parent = (*t)->parent, v,
            decls, inits = NULL,
            cpsend, cprecv;
  int       stlist;   /* See comment above */
  ompclause cp = xc_ompcon_get_clause((*t)->u.omp, OCCOPYPRIVATE),
            nw = xc_ompcon_get_clause((*t)->u.omp, OCNOWAIT);
  
  /* 
   * Checks
   */

  if (cp != NULL && nw != NULL)
    exit_error(1, "(%s, line %d) openmp error:\n\t"
                  "you are not allowed to have both a copyprivate and\n\t"
                  "a nowait clause in a 'single' directive\n",
                  (*t)->u.omp->directive->file->name,(*t)->u.omp->directive->l);    
  v = ompdir_commented((*t)->u.omp->directive); /* Put directive in comments */
  stlist = ((*t)->parent->type == STATEMENTLIST ||
            (*t)->parent->type == COMPOUND);
  /*
   * Preparations
   */

     /* declarations from private/firstprivate vars */
  decls = xc_ompdir_declarations((*t)->u.omp->directive);
     /* initialization statments for firstprivate non-scalar vars */
  if (decls)
    inits = xc_ompdir_fiparray_initializers((*t)->u.omp->directive);
     /* copyprivate calls */
  if (cp)
    copyprivate_statements((*t)->u.omp, &cpsend, &cprecv);
     /* checks for ditching the implicit barrier; if we have cp we MUST block */
  if (nw==NULL && cp==NULL && !xform_implicit_barrier_is_needed((*t)->u.omp))
    nw = (ompclause) 123;       /* Non-null anyway */
  /*
   * Do the job
   */
  
  (*t)->u.omp->body = NULL;     /* Make it NULL so as to free it easily */
  ast_free(*t);                 /* Get rid of the OmpStmt */
  
  if (cp)
    s = BlockList(s, cpsend);
  if (decls)
  {
    /* Check for a special case: we have decls & inits.
     * In this case, the inits should be placed after the decls and *before*
     * the body. But what if the body's first statements are declarations?
     * Consequently, unless the body itself is a compound statement, we
     * must enclose the body in another compound.
     */
    if (inits)
      s = BlockList(BlockList(decls, inits),
                    (s->type != COMPOUND) ? Compound(s) : s);
    else
      s = BlockList(decls, s);
  }
  if (decls || cp)
    s = Compound(s);
  
  *t = BlockList(
         v,
         BlockList(
           If(
             FunctionCall(Identifier(Symbol("ort_mysingle")), 
                          Constant( strdup( nw ? "1" : "0" ) )),
             s,
             NULL
           ),
           Call0_stmt("ort_leaving_single")
         )
       );
  
  if (cp)              /* Must output copyprivate code */
    *t = BlockList( *t, BlockList( BarrierCall(), cprecv ) );
         
  if (nw == NULL)      /* Must output a barrier */
    *t = BlockList(*t, BarrierCall());
  
  if (!stlist)
    *t = Compound(*t);
  ast_stmt_parent(parent, *t);
}
