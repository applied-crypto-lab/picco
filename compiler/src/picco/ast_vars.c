/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame

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

/* ast_vars.c -- declarations and analyses of variables in the AST.
 * 
 *   The functions here (a) declare (i.e. put in the symbol table) given
 *   sets of variables and (b) discover particular variables used in a
 *   given portion of the AST. In particular, they discover:
 *
 *     (1) all global threadprivate (gtp) variables used
 *     (2) all global non-threadprivate (=> shared)
 *     (3) all non-global variables declared in previous scopes; this is
 *         used to discover all shared non-global vars in parallel regions.
 *
 *   The discovered variables are recorded in a symbol table (gtp_vars,
 *   sgl_vars and sng_vars respectively).
 *   
 *   In any of those cases, each variable appearance may optionally be
 *   replaced by a pointer to this variable (X replaced by (*X))
 *   where it is expected that the pointer is properly declared and
 *   initialized elsewhere, during code transformations. Note however that
 *   in the case of (1), this option is NOT effective since this particular
 *   replacement HAS ALREADY BEEN PERFORMED BY THE parser.
 *
 *   (2) may seem (and actuall is) useless when using the thread model,
 *   since all global vars are by nature shared. However, it is useful
 *   in the process model, where nothing is shared and must be made so
 *   explicitely.
 *
 */
 
/* 
 * 2011/12/09:
 *   fixed an is_interesting_var()/mark_var() bug: now correctly recognize
 *   global firstprivate vars for tasks.
 * 2009/05/03:
 *   fixed an is_interesting_var() bug: now returns 0 if not interesting.
 *   added checks for new ompix ATNODE clause vars.
 * 2008/11/09:
 *   added support for tasks (fp/sng)
 * 2008/06/11:
 *   alive.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "ast.h"
#include "symtab.h"
#include "ast_vars.h"
#include "ast_copy.h"
#include "ast_xform.h"
#include "x_clauses.h"
#include "picco.h"


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     DECLARE A SET OF VARIABLES                                *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Declare all variables appearing in a declaration list; could also be types */
void ast_declare_decllist_vars(astspec s, astdecl d)
{
  astdecl ini = NULL;
  
  if (d->type == DLIST && d->subtype == DECL_decllist)
  {
    ast_declare_decllist_vars(s, d->u.next);
    ast_declare_decllist_vars(s, d->decl);
    return;
  }
  if (d->type == DINIT) d = (ini = d)->decl;    /* Get rid of the initializer */
  if (d->type != DECLARATOR)
    exit_error(1, "[ast_declare_decllist_vars]: !!BUG!! not a DECLARATOR ?!\n");
  if (d->decl != NULL && d->decl->type != ABSDECLARATOR)
  {
    symbol  t;
    stentry e;
    int     kind = decl_getkind(d);

    /* Important note here:
     *   we got rid of the initializer => e->decl has NO WAY OF KNOWING
     *   whether it is plain or initialized (since there are no parent
     *   relationships in non-statement nodes).
     */
    t = decl_getidentifier_symbol(d->decl);
    e = symtab_put(stab, t, speclist_getspec(s, STCLASSSPEC, SPEC_typedef) ?
                               TYPENAME : (kind == DFUNC) ? FUNCNAME : IDNAME);
    e->spec      = s;
    e->decl      = d;
    e->idecl     = ini;
    e->isarray   = (kind == DARRAY);
    e->isthrpriv = 0;
  }
}


/* Declare all variables in a varlist (i.e. in an OpenMP data clause) */
void ast_declare_varlist_vars(astdecl d)
{
  stentry e, orig;
  
  if (d->type == DLIST && d->subtype == DECL_idlist)
  {
    ast_declare_varlist_vars(d->u.next);
    d = d->decl;
  }
  if (d->type != DIDENT) 
    exit_error(1, "[ast_declare_varlist_vars]: !!BUG!! not a DIDENT ?!\n");
  
  /* The identifier should be known! */
  if ((orig = symtab_get(stab, d->u.id, IDNAME)) == NULL)
    exit_error(1, "(%s, line %d) openmp error:\n\t"
                  "unknown identifier `%s'\n",
                  d->file->name, d->l, d->u.id->name);
  
  e = symtab_put(stab, d->u.id, IDNAME);  /* Declare new local var */
  e->decl      = orig->decl;                       /* Keep all infos */
  e->idecl     = orig->idecl;
  e->spec      = orig->spec;
  e->isarray   = orig->isarray;
  e->isthrpriv = orig->isthrpriv;
}


/* Declare function parameters */
static
void ast_declare_paramlist_vars(astdecl d)
{
  if (d->type == DLIST && d->subtype == DECL_paramlist)
  {
    ast_declare_paramlist_vars(d->u.next);
    d = d->decl;
  }
  if (d->type == DELLIPSIS)
    return;
  if (d->type != DPARAM) 
    exit_error(1, "[ast_declare_paramlist_vars]: !!BUG!! not a DPARAM ?!\n");
  if (d->decl != NULL && d->decl->type != ABSDECLARATOR)
  {
    symbol  s;
    stentry e;
    
    s = decl_getidentifier_symbol(d->decl);
    e = symtab_put(stab, s, IDNAME);
    e->decl = d->decl;
    e->spec = d->spec;
    e->isarray   = (decl_getkind(d->decl) == DARRAY);
    e->isthrpriv = 0;
  }
}


/* This takes all the parameters of the function declararion/definition
 * and declares them (insertes them in the symbol table). We are given
 * the declarator of the function.
 */
void ast_declare_function_params(astdecl d)
{
  while (d->type == DECLARATOR || d->type == ABSDECLARATOR || d->type == DPAREN)
    d = d->decl;    /* The "while" was added 2009/12/04 - thnx to sagathos */
  if (d == NULL || d->type != DFUNC)
    exit_error(1, "[ast_declare_function_params]: !!BUG!! not a FuncDef ?!\n");
  if ((d = d->u.params) != NULL)
    if (d->type == DPARAM || (d->type == DLIST && d->subtype == DECL_paramlist))
      ast_declare_paramlist_vars(d);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     VARIABLE USAGE                                            *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


#define VARS_SNG       1      /* 0 must NOT be used; it breaks the tests */
#define VARS_GTP       2
#define VARS_SGL       4 
#define VARS_TSNG      8      /* Shared non-globals for tasks */
#define VARS_TFP      16      /* Firstprivate for tasks */
#define checkSNG  (interesting & VARS_SNG)
#define checkGTP  (interesting & VARS_GTP)
#define checkSGL  (interesting & VARS_SGL)
#define checkTSNG (interesting & VARS_TSNG)
#define checkTFP  (interesting & VARS_TFP)
static int  interesting = 0;       /* what to look for */
static int  xformvars = 0;         /* flag to replace var with (*var) */
static void mark_var(symbol v);    /* record a var */
static int  is_interesting_var(symbol v);

void ast_expr_vars(astexpr tree);
void ast_decl_vars(astdecl stmt);
void ast_stmt_vars(aststmt stmt);
void ast_ompcon_vars(ompcon t);
void ast_oxcon_vars(oxcon t);

/*
From the specs (V.3.0), p. 78-79:

The data-sharing attributes of variables that are referenced in a 
construct may be one of the following: 
    predetermined, explicitly determined, or implicitly determined.

The following variables have predetermined data-sharing attributes: (C/C++)
- Variables appearing in threadprivate directives are threadprivate.
- Variables with automatic storage duration that are declared in a scope 
  inside the construct are private.
- Variables with heap allocated storage are shared.
- Static data members are shared.
- The loop iteration variable(s) in the associated for-loop(s) of a for 
  or parallel for construct is(are) private.
- Variables with const-qualified type having no mutable member are shared.
- Static variables which are declared in a scope inside the construct 
  are shared.

Variables with explicitly determined data-sharing attributes are those 
that are referenced in a given construct and are listed in a data-sharing 
attribute clause on the construct.

Variables with implicitly determined data-sharing attributes are those 
that are referenced in a given construct, do not have predetermined 
data-sharing attributes, and are not listed in a data-sharing attribute 
clause on the construct.

Rules for variables with implicitly determined data-sharing attributes 
are as follows:
- In a parallel or task construct, the data-sharing attributes of these 
  variables are determined by the default clause, if present 
  (see Section 2.9.3.1 on page 86).
- In a parallel construct, if no default clause is present, these 
  variables are shared.
- For constructs other than task, if no default clause is present, 
  these variables inherit their data-sharing attributes from the 
  enclosing context.
- In a task construct, if no default clause is present, a variable 
  that is determined to be shared in all enclosing constructs, up to 
  and including the innermost enclosing parallel construct, is shared.
  [ UPDATE (3.1 specs, p. 86):
  In a task construct, if no default clause is present, a variable
  that in the enclosing context is determined to be to be shared by all
  impicit tasks bound to the current team is shared. 
    -- This means that *now*, if no enclosing parallel construct exists, 
    -- then basically the variable is considered shared if it is global.
  ]
- In a task construct, if no default clause is present, a variable 
  whose data-sharing attribute is not determined by the rule above 
  is firstprivate.
*/

/* 
 * Non-OpenMP nodes
 */
 

void ast_expr_vars(astexpr t)
{
  astexpr dim1 = NULL, dim2 = NULL; 
  switch (t->type)
  {
    case IDENT:
    {
      int type;
      if (symtab_get(stab, t->u.sym, IDNAME) == NULL)
        break;             /* Leave it alone - must be a function identifier */
      if ((type = is_interesting_var(t->u.sym)) != 0) /* Must mark & replace */
      {
        mark_var(t->u.sym);      /* Mark it */
        if ( xformvars &&        /* "x" will be substituted by "(*x)" */
             /* threadprivate vars are already pointers in the thread model */
             (!threadmode || !symtab_get(stab, t->u.sym, IDNAME)->isthrpriv) &&
             /* scalar task firstprivate vars not transformed */
             (type != VARS_TFP || symtab_get(stab, t->u.sym, IDNAME)->isarray)
           )
        {
	  if(t->arraysize != NULL)  
          	ast_expr_vars(t->arraysize); 
	  
	  //astexpr sub = 
              //     UnaryOperator(UOP_paren, 
                //                 UnaryOperator(UOP_star, Identifier(t->u.sym)));
	  char* var = (char*)malloc(sizeof(char) * 100);
	  sprintf(var, "(*%s)", t->u.sym->name);
	  astexpr sub = Identifier(Symbol(strdup(var)));
	  sub->thread_id = t->thread_id; 
	  sub->flag = t->flag; 
          sub->index = t->index; 
	  sub->size = t->size; 
	  sub->sizeexp = t->sizeexp; 
          sub->arraysize = t->arraysize;
	  sub->arraytype = t->arraytype; 
	  sub->ftype = t->ftype; 
          sub->isptr = t->isptr; 
	  free(var);  
	  *t = *sub;
        }
      }
      break;
    }
    case FUNCCALL:
      ast_expr_vars(t->left);
      if (t->right) ast_expr_vars(t->right);
      break;
    case CONDEXPR: 
      ast_expr_vars(t->u.cond);
    case ARRAYIDX:
      ast_expr_vars(t->arraysize); 
    case ASS: 
    case BOP: 
    case DESIGNATED: 
    case COMMALIST:
    case SPACELIST:
      ast_expr_vars(t->right);
    case BRACEDINIT:
    case CASTEXPR: 
    case IDXDES: 
    case DOTFIELD: 
    case PTRFIELD:
    case PREOP:
    case POSTOP: 
      ast_expr_vars(t->left);
      break;
    case UOP:   /* sly bug was here! */
      if (t->opid != UOP_sizeoftype && t->opid != UOP_typetrick)
        ast_expr_vars(t->left);
      break;         
  }
}


void ast_spec_vars(astspec t)
{
  switch (t->type)
  {
    case SUE:
      switch (t->subtype)
      {
        case SPEC_enum:
          if (t->body)
            ast_spec_vars(t->body);
          break;
        case SPEC_struct:
        case SPEC_union:
          if (t->u.decl)
            ast_decl_vars(t->u.decl);
          break;
      }
      break;
    case ENUMERATOR: 
      if (t->u.expr)
        ast_expr_vars(t->u.expr);
      break;
    case SPECLIST:
      ast_spec_vars(t->body);
      ast_spec_vars(t->u.next);
      break;
  }
}


void ast_decl_vars(astdecl t)
{
  switch (t->type)
  {
    case DIDENT:
      break;
    case DPAREN:
      ast_decl_vars(t->decl);
      break;
    case DARRAY:
      if (t->decl)
        ast_decl_vars(t->decl);
      if (t->spec)
        ast_spec_vars(t->spec);
      if (t->u.expr)
        ast_expr_vars(t->u.expr);
      break;
    case DFUNC:
      break;
    case DINIT:
      ast_decl_vars(t->decl);
      if (t->u.expr != NULL)
        ast_expr_vars(t->u.expr);
      break;
    case DECLARATOR:
      if (t->spec)
        ast_spec_vars(t->spec);
      ast_decl_vars(t->decl);
      break;
    case ABSDECLARATOR:
      if (t->spec) 
        ast_spec_vars(t->spec);
      if (t->decl) 
        ast_decl_vars(t->decl);
      break;
    case DPARAM:
      ast_spec_vars(t->spec);
      if (t->decl) 
        ast_decl_vars(t->decl);
      break;
    case DBIT:
      if (t->decl)
        ast_decl_vars(t->decl);
      ast_expr_vars(t->u.expr);
      break;
    case DSTRUCTFIELD:
      if (t->spec)
        ast_spec_vars(t->spec);
      if (t->decl)
        ast_decl_vars(t->decl);
      break;
    case DCASTTYPE:
      ast_spec_vars(t->spec); 
      if (t->decl)
        ast_decl_vars(t->decl);
      break;
    case DLIST:
      ast_decl_vars(t->u.next);
      ast_decl_vars(t->decl);
      break;
  }
}


void ast_stmt_jump_vars(aststmt t)
{
  if (t->subtype == SRETURN && t->u.expr)    /* The only case of interest */
    ast_expr_vars(t->u.expr);
}


void ast_stmt_iteration_vars(aststmt t)
{
  switch (t->subtype)
  {
    case SFOR:
      if (t->u.iteration.init != NULL)
        ast_stmt_vars(t->u.iteration.init);
      if (t->u.iteration.incr != NULL)
        ast_expr_vars(t->u.iteration.incr);
    case SWHILE:
    case SDO:
      if (t->u.iteration.cond != NULL)
        ast_expr_vars(t->u.iteration.cond);
      ast_stmt_vars(t->body);
      break;
  }
}

void ast_stmt_batch_vars(aststmt t)
{
    if (t->u.iteration.init != NULL)
        ast_stmt_vars(t->u.iteration.init);
    if (t->u.iteration.incr != NULL)
        ast_expr_vars(t->u.iteration.incr);
    if (t->u.iteration.cond != NULL)
        ast_expr_vars(t->u.iteration.cond);
    ast_stmt_vars(t->body);
}

void ast_stmt_selection_vars(aststmt t)
{
  switch (t->subtype)
  {
    case SIF:
      if (t->u.selection.elsebody)
        ast_stmt_vars(t->u.selection.elsebody);
    case SSWITCH:
      ast_expr_vars(t->u.selection.cond);
      ast_stmt_vars(t->body);
      break;
  }
}


void ast_stmt_labeled_vars(aststmt t)
{
  if (t->subtype == SCASE)
    ast_expr_vars(t->u.expr);
  ast_stmt_vars(t->body);
}


void ast_stmt_vars(aststmt t)
{
  if (t == NULL) return;
  switch (t->type)
  {
    case JUMP:
      ast_stmt_jump_vars(t);
      break;
    case ITERATION:
      ast_stmt_iteration_vars(t);
      break;
    case BATCH: 
      ast_stmt_batch_vars(t); 
      break; 
    case SELECTION:
      ast_stmt_selection_vars(t);
      break;
    case LABELED:
      ast_stmt_labeled_vars(t);
      break;
    case EXPRESSION:
      if (t->u.expr != NULL){
        ast_expr_vars(t->u.expr);
      }
      break;
    case SMC: 
      ast_expr_vars(t->u.smcops.variable); 
      if(t->u.smcops.size1 != NULL)
	 ast_expr_vars(t->u.smcops.size1); 
      if(t->u.smcops.size2 != NULL)
	 ast_expr_vars(t->u.smcops.size2); 
      break; 
    case COMPOUND:
      if (t->body)
      {
        scope_start(stab);
          ast_stmt_vars(t->body);
        scope_end(stab);
      }
      break;
    case STATEMENTLIST:
      ast_stmt_vars(t->u.next);
      ast_stmt_vars(t->body);
      break;
    case DECLARATION:
    {
      ast_spec_vars(t->u.declaration.spec);
      if (t->u.declaration.decl)
      {
        ast_decl_vars(t->u.declaration.decl);          /* then, declare them */
        ast_declare_decllist_vars(t->u.declaration.spec, t->u.declaration.decl);
      }
      break;
    }
    case FUNCDEF:
      scope_start(stab);
      
      if (t->u.declaration.dlist)    /* Old style */
        ast_stmt_vars(t->u.declaration.dlist);  /* will declare themselves */
      else                           /* Normal; has paramtypelist */
        ast_declare_function_params(t->u.declaration.decl);/* declare manualy */
      ast_stmt_vars(t->body);
      
      scope_end(stab);
      break;
    case OMPSTMT:
      ast_ompcon_vars(t->u.omp);
      break;
    case OX_STMT:
      ast_oxcon_vars(t->u.ox);
      break;
  }
  
}


/*
 * OpenMP nodes (deleted)
 */
 
 
/* We only check 4 specific types of clauses for possible
 * expressions with variable references: if(), num_threads(), final() 
 * and schedule().
 * This should occur *before* checking the data clauses, since those
 * clauses may declare new variables.
 */
void ast_omp_nondataclause_vars(ompclause t)
{
    if (t->type == OCLIST)
    {
        if (t->u.list.next != NULL)
            ast_omp_nondataclause_vars(t->u.list.next);
        assert((t = t->u.list.elem) != NULL);
    }
    if (t->type == OCIF || t->type == OCNUMTHREADS || t->type == OCSCHEDULE ||
        t->type == OCFINAL)
        if (t->u.expr)
            ast_expr_vars(t->u.expr);
}


/* Puts all variable names in the symbol table (ala ast_declare_varlist_vars()).
 * The only difference is that it also records their usage.
 */
#define declare_only_omp_varlist(vl) ast_declare_varlist_vars(vl)
static void declare_and_mark_omp_varlist(astdecl d, enum clausetype type)
{
    stentry st, orig;
    
    if (d->type == DLIST && d->subtype == DECL_idlist)
    {
        declare_and_mark_omp_varlist(d->u.next, type);
        d = d->decl;
    }
    if (d->type != DIDENT)
        exit_error(1, "[declare_and_mark_omp_varlist]: !!BUG!! not a DIDENT ?!\n");
    
    /* The identifier should be known! */
    if ((orig = symtab_get(stab, d->u.id, IDNAME)) == NULL)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                   "unknown identifier `%s'\n",
                   d->file->name, d->l, d->u.id->name);
    
    mark_var(d->u.id);                          /* Mark */
    
    if (checkTFP && type == OCFIRSTPRIVATE && orig->isarray)
        return;   /* In this case we trick */
    
    st = symtab_put(stab, d->u.id, IDNAME);     /* Declare new local var */
    st->decl      = orig->decl;                 /* Keep all infos */
    st->idecl     = orig->idecl;
    st->spec      = orig->spec;
    st->isarray   = orig->isarray;
    st->isthrpriv = orig->isthrpriv;
}


static
void ast_omp_dataclause_vars(ompclause t)
{
    if (t == NULL) return;
    
    if (t->type == OCLIST)
    {
        if (t->u.list.next != NULL)
            ast_omp_dataclause_vars(t->u.list.next);
        assert((t = t->u.list.elem) != NULL);
    }
    switch (t->type)
    {
        case OCPRIVATE:
            declare_only_omp_varlist(t->u.varlist);
            break;
        case OCFIRSTPRIVATE:
        case OCLASTPRIVATE:
        case OCCOPYPRIVATE:
        case OCREDUCTION:
            declare_and_mark_omp_varlist(t->u.varlist, t->type);
            break;
        case OCCOPYIN:        /* Don't even declare it! */
            break;
    }
}


static
void ast_ompdir_vars(ompdir t)
{
    if (t->clauses)
    {
        ast_omp_nondataclause_vars(t->clauses); /* Check those first */
        ast_omp_dataclause_vars(t->clauses);    /* And those next */
    }
}


void ast_ompcon_vars(ompcon t)
{
    scope_start(stab);     /* Begin a new scope */
    
    ast_ompdir_vars(t->directive);
    if (t->body)           /* barrier, flush etc don't have a body */
        ast_stmt_vars(t->body);
    
    scope_end(stab);       /* Close the scope */
}


/**** OMPI extensions start ****/


static
void ast_ompix_nondataclause_vars(oxclause t)
{
    if (t->type == OX_OCLIST)
    {
        if (t->u.list.next != NULL)
            ast_ompix_nondataclause_vars(t->u.list.next);
        assert((t = t->u.list.elem) != NULL);
    }
    switch (t->type)
    {
        case OX_OCATNODE:
        case OX_OCATWORKER:
        case OX_OCSTRIDE:
        case OX_OCSTART:
        case OX_OCSCOPE:
            if (t->u.expr)
                ast_expr_vars(t->u.expr);
            break;
    }
}


static
void ast_oxdir_vars(oxdir t)
{
    if (t->clauses)
    {
        ast_ompix_nondataclause_vars(t->clauses); /* Check those first */
        /* ast_omp_dataclause_vars(t->clauses);   --  And those next */
    }
}


/* For ompi extensions */
void ast_oxcon_vars(oxcon t)
{
    scope_start(stab);     /* Begin a new scope */
    
    ast_oxdir_vars(t->directive);
    if (t->body)           /* barrier, flush etc don't have a body */
        ast_stmt_vars(t->body);
    
    scope_end(stab);       /* Close the scope */
}


/**** OMPI extensions end ****/


/*
 * Bookkeeping of the used vars
 */

 
/* A list of all used symbols - a symtab is quite handy. */
static int thislevel = 0;
symtab     sng_vars = NULL, gtp_vars = NULL, sgl_vars = NULL;
static 
  struct { symtab clausevars;    /* all vars from the clauses */
           int    defnone;       /* 1 if default(none) present */
           int    defshared;     /* 1 if default(shared) present */
         } taskinfo;             /* Only used for task implicits */


/* Mark a variable: put it in the right table */
void mark_var(symbol v)
{
  int     type;
  stentry e;
  
  if ((type = is_interesting_var(v)) == 0) return;
  e = symtab_get(stab, v, IDNAME);
  if (e->scopelevel > thislevel) return;     /* Don't care for those */
  
  if (e->scopelevel == 0)  /* Global */
  {
    if (type & VARS_SGL)
      if (symtab_get(sgl_vars, v, IDNAME) == NULL)
        symtab_put(sgl_vars, v, IDNAME);
    if (type & VARS_GTP)
      if (symtab_get(gtp_vars, v, IDNAME) == NULL)
        symtab_put(gtp_vars, v, IDNAME);
    if (type & VARS_TFP)
      if (symtab_get(sng_vars, v, IDNAME) == NULL)
        symtab_put(sng_vars, v, IDNAME)->ival = OCFIRSTPRIVATE;
  }
  else                 /* Non-global, declared at previous level (shared) */
    if (type & VARS_SNG || type & VARS_TSNG || type & VARS_TFP)
      if (symtab_get(sng_vars, v, IDNAME) == NULL)
        symtab_put(sng_vars, v, IDNAME)->ival = 
          (type == VARS_TFP ? OCFIRSTPRIVATE : OCSHARED);
}


/* Check if a variable is what we look for. */
/* From shared globals, we ditch extern ones (...) */
int is_interesting_var(symbol v)
{
  stentry e, f;
  
  if ((e = symtab_get(stab, v, IDNAME)) == NULL)
  {
    fprintf(stderr, "[is_interesting_var]: BUG?? [%s]\n", v->name);
    scope_show(stab);
    exit (1);
  }
  
  if (e->scopelevel == 0)             /* Globals */
  {
    if (checkGTP && e->isthrpriv)
      return (VARS_GTP);
    if (checkSGL && !(e->isthrpriv) &&
        speclist_getspec(e->spec, STCLASSSPEC, SPEC_extern) == NULL)
      return (VARS_SGL);
    if (checkTFP)
    {
      if ((f = symtab_get(taskinfo.clausevars, v, IDNAME)) != NULL) /*explicit*/
        return ( f->ival == OCFIRSTPRIVATE ? (interesting & VARS_TFP) : 0 );
      if (taskinfo.defnone)
        exit_error(1, 
          "openmp error:\n\t"
          "variable `%s' must be explicitely declared as shared/private\n\t"
          "due to the default(none) clause of the task directive.\n",
          v->name);
      /* otherwise it is implicit */
      /* In OpenMP 3.0:
           if (!taskinfo.defshared && (e->scopelevel > closest_parallel_scope))
             return (VARS_TFP);
         In OpenMP 3.1 globals shall be shared basically no matter what.
      */
    }
    return (0); 
  }
  
  if (e->scopelevel > thislevel) return (0);    /* Don't care for those */

  if (checkSNG)
    return ( (e->scopelevel > 0 && e->scopelevel <= thislevel) ?   /* sly */
             VARS_SNG : 0 );

  if (checkTFP || checkTSNG)  /* tasks */
  {
    if ((f = symtab_get(taskinfo.clausevars, v, IDNAME)) != NULL) /* explicit */
      return (f->ival == OCSHARED ?       (interesting & VARS_TSNG) :
              f->ival == OCFIRSTPRIVATE ? (interesting & VARS_TFP) : 0);
    if (taskinfo.defnone)
      exit_error(1, 
        "openmp error:\n\t"
        "variable `%s' must be explicitely declared as shared/private\n\t"
        "due to the default(none) clause of the task directive.\n",
        v->name);
    else
      if (taskinfo.defshared)
        return (interesting & VARS_TSNG);   /* since interesting may be TSNG */
      else     /* implicit */
        if (e->scopelevel <= closest_parallel_scope)
          return (interesting & VARS_TSNG);
        else
          return (interesting & VARS_TFP);
  }

  return (0);    /* in any other case, this is an uninteresting var! */
}


static
void find_global_vars(aststmt t, int xformflag)
{
  if (checkGTP)
  {
    if (gtp_vars == NULL)
      gtp_vars = Symtab();
    else
      symtab_drain(gtp_vars);     /* Empty the table */
  }
  
  if (checkSGL)
  {
    if (sgl_vars == NULL)
      sgl_vars = Symtab();
    else
      symtab_drain(sgl_vars);     /* Empty the table */
  }
  
  thislevel = stab->scopelevel;   /* Thanx to GPh!! */
  xformvars = xformflag;
  ast_stmt_vars(t);
}

/* We are on an omp parallel node and we want to find all variables used
* inside the construct so as to determine the shared ones that where
* declared in a previous scope.
* Notice that this is not recursive or reentrant. A nested parallel
* region is checked here, but it will be analyzed similarly later, when
* the analysis of the AST progresses.
*/
static
void find_paracon_sng_vars(ompcon t, int xformflag)
{
    if (sng_vars == NULL)
        sng_vars = Symtab();
    else
        symtab_drain(sng_vars);     /* Empty the table */
    
    thislevel = stab->scopelevel;
    
    scope_start(stab);     /* Begin a new scope */
    
    /* We don't check the nondata clauses num_threads() and if() since
     * the code they produce is in-place, outside the generated function,
     * so we don't care. However, all other xxxprivate() clause mater.
     */
    xformvars = xformflag;
    if (t->directive->clauses)
        ast_omp_dataclause_vars(t->directive->clauses);
    ast_stmt_vars(t->body);
    
    scope_end(stab);       /* Close the scope */
}


void ast_paracon_find_sng_vars(ompcon t, int xformflag)
{
    interesting = VARS_SNG;
    find_paracon_sng_vars(t, xformflag);
}


void ast_taskcon_find_sngfp_vars(ompcon t, int xformflag)
{
    /* gather all necessary info regarding the task construct; this is
     * needed in order to decide on "implicit" variables
     */
    ompclause c = xc_ompcon_get_clause(t, OCDEFAULT);
    taskinfo.clausevars = xc_validate_store_dataclause_vars(t->directive);
    taskinfo.defnone    = (c != NULL && c->subtype == OC_defnone);
    taskinfo.defshared  = (c != NULL && c->subtype == OC_defshared);
    
    interesting = VARS_TSNG | VARS_TFP;
    find_paracon_sng_vars(t, xformflag);
}



void ast_find_gtp_vars(aststmt t, int xformflag)
{
  interesting = VARS_GTP;
  find_global_vars(t, xformflag);
}


void ast_find_sgl_vars(aststmt t, int xformflag)
{
  interesting = VARS_SGL;
  find_global_vars(t, xformflag);
}


void ast_find_allg_vars(aststmt t, int xformflag)
{
  interesting = VARS_GTP | VARS_SGL;
  find_global_vars(t, xformflag);
}
