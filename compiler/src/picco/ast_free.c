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

/* ast_free.c -- free the nodes on an AST */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ast_free.h"


void ast_stmt_jump_free(aststmt t)
{
  switch (t->subtype)
  {
    case SBREAK:
      break;
    case SCONTINUE:
      break;
    case SRETURN:
      if (t->u.expr != NULL)
        ast_expr_free(t->u.expr);
      break;
    case SGOTO:
      break;
    default:
      fprintf(stderr, "[ast_stmt_jump_free]: b u g !!\n");
  }
  free(t);
}

void ast_stmt_iteration_free(aststmt t)
{
  switch (t->subtype)
  {
    case SFOR:
      if (t->u.iteration.init != NULL)
        ast_stmt_free(t->u.iteration.init); /* Could also be a declaration */
      if (t->u.iteration.cond != NULL)
        ast_expr_free(t->u.iteration.cond);
      if (t->u.iteration.incr != NULL)
        ast_expr_free(t->u.iteration.incr);
      ast_stmt_free(t->body);
      break;
    case SWHILE:
      ast_expr_free(t->u.iteration.cond);
      ast_stmt_free(t->body);
      break;
    case SDO:
      ast_stmt_free(t->body);
      ast_expr_free(t->u.iteration.cond);
      break;
    default:
      fprintf(stderr, "[ast_stmt_iteration_free]: b u g !!\n");
  }
  free(t);
}

void ast_stmt_selection_free(aststmt t)
{
  switch (t->subtype)
  {
    case SSWITCH:
      ast_expr_free(t->u.selection.cond);
      ast_stmt_free(t->body);
      break;
    case SIF:
      ast_expr_free(t->u.selection.cond);
      ast_stmt_free(t->body);
      if (t->u.selection.elsebody)
        ast_stmt_free(t->u.selection.elsebody);
      break;
    default:
      fprintf(stderr, "[ast_stmt_selection_free]: b u g !!\n");
  }
  free(t);
}

void ast_stmt_labeled_free(aststmt t)
{
  switch (t->subtype)
  {
    case SLABEL:
      break;
    case SCASE:
      ast_expr_free(t->u.expr);
      break;
    case SDEFAULT:
      break;
    default:
      fprintf(stderr, "[ast_stmt_labeled_free]: b u g !!\n");
  }
  ast_stmt_free(t->body);
  free(t);
}

void ast_stmt_free(aststmt t)
{
  switch (t->type)
  {
    case JUMP:
      ast_stmt_jump_free(t);
      break;
    case ITERATION:
      ast_stmt_iteration_free(t);
      break;
    case SELECTION:
      ast_stmt_selection_free(t);
      break;
    case LABELED:
      ast_stmt_labeled_free(t);
      break;
    case EXPRESSION:
      if (t->u.expr != NULL)  /* Empty statement */
        ast_expr_free(t->u.expr);
      free(t);
      break;
    case COMPOUND:
      if (t->body)
        ast_stmt_free(t->body);
      free(t);
      break;
    case STATEMENTLIST:
      ast_stmt_free(t->u.next);
      ast_stmt_free(t->body);
      free(t);
      break;
    case DECLARATION:
      ast_spec_free(t->u.declaration.spec);
      if (t->u.declaration.decl)
        ast_decl_free(t->u.declaration.decl);
      free(t);
      break;
    case FUNCDEF:
      if (t->u.declaration.spec)
        ast_spec_free(t->u.declaration.spec);
      ast_decl_free(t->u.declaration.decl);   /* always non-NULL */
      if (t->u.declaration.dlist)
        ast_stmt_free(t->u.declaration.dlist);
      ast_stmt_free(t->body);    /* always non-NULL */
      free(t);
      break;
    case OMPSTMT:
          ast_ompcon_free(t->u.omp);
          free(t);
          break;
    case VERBATIM:
          if (t->u.code)
              free(t->u.code);
          free(t);
          break;
    case OX_STMT:
          ast_oxcon_free(t->u.ox);
          free(t);
          break;
    default:
      fprintf(stderr, "[ast_stmt_free]: b u g !!\n");
  }
}

void ast_expr_free(astexpr t)
{
  switch (t->type)
  {
    case IDENT:
      break;
    case CONSTVAL:
      free(t->u.str);
      break;
    case STRING: 
      free(t->u.str);
      break;
    case FUNCCALL:
      ast_expr_free(t->left);
      if (t->right)
        ast_expr_free(t->right);
      break;
    case ARRAYIDX: 
      ast_expr_free(t->left);
      ast_expr_free(t->right);
      break;
    case DOTFIELD: 
      ast_expr_free(t->left);
      break;
    case PTRFIELD: 
      ast_expr_free(t->left);
      break;
    case BRACEDINIT: 
      ast_expr_free(t->left);
      break;
    case CASTEXPR: 
      ast_decl_free(t->u.dtype);
      ast_expr_free(t->left);
      break;
    case CONDEXPR: 
      ast_expr_free(t->u.cond);
      ast_expr_free(t->left);
      ast_expr_free(t->right);
      break;
    case UOP: 
      if (t->opid == UOP_sizeoftype || t->opid == UOP_typetrick)
        ast_decl_free(t->u.dtype);
      else
        ast_expr_free(t->left);
      break;
    case BOP: 
      ast_expr_free(t->left);
      ast_expr_free(t->right);
      break;
    case PREOP: 
      ast_expr_free(t->left);
      break;
    case POSTOP: 
      ast_expr_free(t->left);
      break;
    case ASS: 
      ast_expr_free(t->left);
      ast_expr_free(t->right);
      break;
    case DESIGNATED: 
      ast_expr_free(t->left);
      ast_expr_free(t->right);
      break;         
    case IDXDES: 
      ast_expr_free(t->left);
      break;
    case DOTDES: 
      break;
    case COMMALIST:
    case SPACELIST:
      ast_expr_free(t->left);
      ast_expr_free(t->right);
      break;         
    default:
      fprintf(stderr, "[ast_expr_free]: b u g !! (type = %d)\n", t->type);
  }
  free(t);
}

void ast_spec_free(astspec t)
{
  switch (t->type)
  {
    case SPEC:
    case STCLASSSPEC:
      break;
    case USERTYPE:
      break;
    case SUE:
      switch (t->subtype)
      {
        case SPEC_enum:
          if (t->body)
            ast_spec_free(t->body);
          break;
        case SPEC_struct:
        case SPEC_union:
          if (t->u.decl)
            ast_decl_free(t->u.decl);
          break;
        default:
          fprintf(stderr, "[ast_spec_free]: SUE b u g !!\n");
      }
      break;
    case ENUMERATOR: 
      if (t->u.expr)
        ast_expr_free(t->u.expr);
      break;
    case SPECLIST:
      switch (t->subtype)
      {
        case SPEC_Rlist:
          ast_spec_free(t->body);
          ast_spec_free(t->u.next);
          break;
        case SPEC_Llist:
          ast_spec_free(t->u.next);
          ast_spec_free(t->body);
          break;
        case SPEC_enum:
          ast_spec_free(t->u.next);
          ast_spec_free(t->body);
          break;
        default:
          fprintf(stderr, "[ast_spec_free]: list b u g !!\n");
      }
      break;
    default:
      fprintf(stderr, "[ast_spec_free]: b u g !!\n");
  }
  free(t);
}

void ast_decl_free(astdecl t)
{
  switch (t->type)
  {
    case DIDENT:
      break;
    case DPAREN:
      ast_decl_free(t->decl);
      break;
    case DARRAY:
      if (t->decl) /* Maybe abstract declarator */
        ast_decl_free(t->decl);
      if (t->spec)
        ast_spec_free(t->spec);
      if (t->u.expr)
        ast_expr_free(t->u.expr);
      break;
    case DFUNC:      /* Maybe abstract declarator */
      if (t->decl)
        ast_decl_free(t->decl);
      if (t->u.params)
        ast_decl_free(t->u.params);
      break;
    case DINIT:
      ast_decl_free(t->decl);
      if (t->u.expr != NULL)  /* Empty statement */
        ast_expr_free(t->u.expr);
      break;
    case DECLARATOR:
      if (t->spec)      /* pointer */
        ast_spec_free(t->spec);
      ast_decl_free(t->decl);
      break;
    case ABSDECLARATOR:
      if (t->spec)      /* pointer */
        ast_spec_free(t->spec);
      if (t->decl)      /* could be NULL */
        ast_decl_free(t->decl);
      break;
    case DPARAM:
      ast_spec_free(t->spec);   /* Never NULL */
      if (t->decl)      /* could be NULL */
        ast_decl_free(t->decl);
      break;
    case DELLIPSIS:
      break;
    case DBIT:
      if (t->decl)
        ast_decl_free(t->decl);
      ast_expr_free(t->u.expr);
      break;
    case DSTRUCTFIELD:
      if (t->spec)      /* pointer */
        ast_spec_free(t->spec);
      if (t->decl)
        ast_decl_free(t->decl);
      break;
    case DCASTTYPE:
      ast_spec_free(t->spec);        /* Always non-NULL */
      if (t->decl)
        ast_decl_free(t->decl);
      break;
    case DLIST:
      switch (t->subtype)
      {
        case DECL_decllist:
        case DECL_idlist:
        case DECL_paramlist:
          ast_decl_free(t->u.next);
          ast_decl_free(t->decl);
          break;
        case DECL_fieldlist:
          ast_decl_free(t->u.next);
          ast_decl_free(t->decl);
          break;
        default:
          fprintf(stderr, "[ast_decl_free]: list b u g !!\n");
      }
      break;
    default:
      fprintf(stderr, "[ast_decl_free]: b u g !!\n");
  }
  free(t);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OpenMP NODES                                              *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


void ast_ompclause_free(ompclause t)
{
    if (t == NULL) return;
    if (t->type == OCLIST)
    {
        if (t->u.list.next != NULL)
            ast_ompclause_free(t->u.list.next);
        assert((t = t->u.list.elem) != NULL);
    }
    
    switch (t->type)
    {
        case OCDEFAULT:
        case OCNOWAIT:
        case OCORDERED:
        case OCUNTIED:
        case OCCOLLAPSE:
        case OCMERGEABLE:
            break;
        case OCIF:
        case OCNUMTHREADS:
        case OCFINAL:
            ast_expr_free(t->u.expr);
            break;
        case OCSCHEDULE:
            if (t->u.expr)
                ast_expr_free(t->u.expr);
            break;
        case OCCOPYIN:
        case OCPRIVATE:
        case OCCOPYPRIVATE:
        case OCFIRSTPRIVATE:
        case OCLASTPRIVATE:
        case OCSHARED:
        case OCREDUCTION:
            ast_decl_free(t->u.varlist);
            break;
        default:
            fprintf(stderr, "[ast_ompclause_free]: b u g !!\n");
    }
    free(t);
}


void ast_ompdir_free(ompdir t)
{
    switch (t->type)
    {
        case DCCRITICAL:
            break;
        case DCFLUSH:
            if (t->u.varlist)
                ast_decl_free(t->u.varlist);
            break;
        default:
            if (t->clauses)
                ast_ompclause_free(t->clauses);
            break;
    }
    free(t);
}


void ast_ompcon_free(ompcon t)
{
    ast_ompdir_free(t->directive);
    if (t->body)     /* barrier & flush don't have a body */
        ast_stmt_free(t->body);
    free(t);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OMPi-EXTENSION NODES                                      *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


void ast_oxclause_free(oxclause t)
{
    if (t == NULL) return;
    if (t->type == OX_OCLIST)
    {
        if (t->u.list.next != NULL)
            ast_oxclause_free(t->u.list.next);
        assert((t = t->u.list.elem) != NULL);
    }
    switch (t->type)
    {
        case OX_OCIN:
        case OX_OCOUT:
        case OX_OCINOUT:
        case OX_OCREDUCE:
            ast_decl_free(t->u.varlist);
            break;
        case OX_OCATNODE:
        case OX_OCATWORKER:
        case OX_OCSTART:
        case OX_OCSTRIDE:
            ast_expr_free(t->u.expr);
            break;
        case OX_OCTIED:
        case OX_OCUNTIED:
        case OX_OCDETACHED:
        case OX_OCATALL:
        case OX_OCSCOPE:
            break;
        default:
            fprintf(stderr, "[ast_oxclause_free]: b u g !!\n");
    }
    free(t);
}

void ast_oxdir_free(oxdir t)
{
    if (t == NULL) return;
    if (t->clauses)
        ast_oxclause_free(t->clauses);
    free(t);
}

void ast_oxcon_free(oxcon t)
{
    ast_oxdir_free(t->directive);
    if (t->body)
        ast_stmt_free(t->body);
    free(t);
}
