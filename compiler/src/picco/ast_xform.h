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

/* ast_xform.h  */

#ifndef __AST_XFORM_H__
#define __AST_XFORM_H__

#include "ast.h"
#include "symtab.h"

extern void ast_xform(aststmt *tree);
extern void ast_expr_xform(astexpr *t);
extern void ast_stmt_xform(aststmt *t);
extern void ast_decl_xform(astdecl *t);
extern void ast_spec_xform(astspec *t);
extern void ast_ompclause_xform(ompclause *t);
extern void ast_ompdir_xform(ompdir *t);
extern void ast_ompcon_xform(ompcon *t);
extern void ast_omp_xform(aststmt *t);

extern stentry newglobalvar(aststmt s);
extern void    tail_add(aststmt s);

/* This takes care of placing the produced thread functions */
extern void xfrom_add_threadfunc(symbol fname, aststmt fd, aststmt curfunc);

/* Produces an identical declaration which may optionally have an initializer */
extern aststmt xform_clone_declaration(symbol s, astexpr initer);

/* Call a function with no args */
#define Call0_expr(s) FunctionCall(Identifier(Symbol(s)),NULL)
#define Call0_stmt(s) Expression(Call0_expr(s))
#define BarrierCall() Call0_stmt("ort_barrier_me")

/* Holds the scope level of the most recently encountered
 * parallel construct
 */
extern int closest_parallel_scope;

/* This one checks whether an openmp construct should include an
 * implicit barrier.
 */
extern int xform_implicit_barrier_is_needed(ompcon t);

/* This simply prints the directive into a string and encloses it
 * in comments, adding also the source line number. 
 * It returns a verbatim node with the comment.
 */
extern aststmt ompdir_commented(ompdir d);

#endif
