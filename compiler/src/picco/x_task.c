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

/* x_task.c -- too big a transformation to be part of ast_xfrom */

/* 
 * 2010/12/17:
 *   Fixed bug in certain cases of tasks with an if() clause.
 * 2010/11/20:
 *   Fixed produced code for immediate task execution. Fixed bug in
 *   cases of global firstprivate vars.
 * 2009/12/10:
 *   added task function declared in transformed parent code.
 * 2009/05/10:
 *   fixes & unification of fp and sng vars 
 * 2008/11/09:
 *   fixes (fp/sng working for tasks)
 * 2008/06/11:
 *   alive.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ast_print.h"
#include "ast_free.h"
#include "ast_vars.h"
#include "ast_xform.h"
#include "ast_copy.h"
#include "ast_renlabs.h"
#include "x_task.h"
#include "x_clauses.h"
#include "x_thrpriv.h"
#include "symtab.h"
#include "str.h"
#include "picco.h"

/*
 * Task function names/struct type names/reduction lock generators.
 * They also create global declarations for:
 *   - the new function
 *   - the reduction lock.
 *
 * Parallel constructs emit their own reduction code, independent of
 * other constructs.
 */
#define SHVARSNAME "_tenv"
#define FPVARSNAME "_fpsnap"
#define SHVARSTYPE "__taskenv__"
#define FPVARSTYPE "__fpvt__"
static int  tasknum = -1;
static char _tfn[128];
#define taskfuncname() _tfn


static
void new_taskfunc()
{
  tasknum++;
  sprintf(_tfn, "_taskFunc%d_", tasknum);
}


/*
 * The following 3 functions memorize the variables included in the
 * data clauses of the task directive we are transforming,
 * along with the clause type of each one (private/firstprivate/etc).
 * They also check for presence of inappropriate clauses.
 *
 */
static symtab    dataclause_vars = NULL;    /* Store the list here */
static ompclause dataclause_vars_clause;    /* Only needed for error messages */


static
void store_varlist_vars(astdecl d, int clausetype)
{
  stentry e;
  
  if (d->type == DLIST && d->subtype == DECL_idlist)
  {
    store_varlist_vars(d->u.next, clausetype);
    d = d->decl;
  }
  if (d->type != DIDENT)
    exit_error(1, "[store_varlist_vars]: !!BUG!! not a DIDENT ?!\n");
  if (symtab_get(dataclause_vars, d->u.id, IDNAME) != NULL)
    exit_error(1, "(%s, line %d) openmp error:\n\t"
                  "variable `%s' appears more than once in a\n\t"
                  "task directive's clause(s).\n",
                  dataclause_vars_clause->file->name, dataclause_vars_clause->l,
                  d->u.id->name);
  e = symtab_put(dataclause_vars, d->u.id, IDNAME);
  e->ival = clausetype;
}


static
void store_dataclause_vars(ompclause t)
{
  if (t->type == OCLIST)
  {
    if (t->u.list.next != NULL) 
      store_dataclause_vars(t->u.list.next);
    assert((t = t->u.list.elem) != NULL);
  }
  dataclause_vars_clause = t;
  switch (t->type)
  {
    case OCPRIVATE:
    case OCFIRSTPRIVATE:
    case OCSHARED:
      if (t->u.varlist)
        store_varlist_vars(t->u.varlist, t->type);
    case OCIF:
    case OCFINAL:
    case OCUNTIED:
    case OCDEFAULT:
    case OCMERGEABLE:
      break;
    default:
      exit_error(1, "(%s, line %d) openmp error:\n\t"
                    "illegal clause (%s) in task directive\n",
                    t->file->name, t->l, clausenames[t->type]);
  }
}


static
void xt_store_dataclause_vars(ompdir d)
{
  if (dataclause_vars == NULL)
    dataclause_vars = Symtab();
  else
    symtab_drain(dataclause_vars);
  if (d->clauses)
    store_dataclause_vars(d->clauses);
}


#define isFirstprivate(s) isDataShareType(s,OCFIRSTPRIVATE)
#define isPrivate(s)      isDataShareType(s,OCPRIVATE)
static int isDataShareType(symbol s, enum clausetype type)
{
  stentry e = symtab_get(dataclause_vars, s, IDNAME);
  if (e == NULL) return (0);
  return ( e->ival == type );
}


/*
 * Structure-related stuff
 *
 */
 
 
/* Produces a type specifier for the struct, with all fields listed */
static
astspec xt_taskenv_struct()
{
  stentry e, orig;
  astdecl st = NULL, tmp;
  
  /* Create the fields (sngs & fp) */
  for (e = sng_vars->top; e; e = e->stacknext)
  {
    orig = symtab_get(stab, e->key, IDNAME);
    tmp = StructfieldDecl(
            ast_spec_copy_nosc(orig->spec),
            (e->ival == OCFIRSTPRIVATE) ?  /* Not a pointer here */
              (threadmode && orig->isthrpriv) ?  /* 'cause of changed name */
                xc_decl_rename(ast_decl_copy(orig->decl), e->key) :
                ast_decl_copy(orig->decl)
              :
              xc_decl_topointer(
                (threadmode && orig->isthrpriv) ?  /* cause of changed name */
                  xc_decl_rename(ast_decl_copy(orig->decl), e->key) :
                  ast_decl_copy(orig->decl)
              )
          );
    st = (st == NULL) ? tmp : StructfieldList(st, tmp);
  }
  
  /* Check if any of the explicit fp vars is global (not covered in sng_vars)
   */
  for (e = dataclause_vars->top; e; e = e->stacknext)
  {
    if (e->ival != OCFIRSTPRIVATE || symtab_get(sng_vars,e->key,IDNAME) != NULL)
      continue;
    orig = symtab_get(stab, e->key, IDNAME);
    tmp = StructfieldDecl(
            ast_spec_copy_nosc(orig->spec),
            (threadmode && orig->isthrpriv) ?  /* 'cause of changed name */
              xc_decl_rename(ast_decl_copy(orig->decl), e->key) :
              ast_decl_copy(orig->decl)
          );
    st = (st == NULL) ? tmp : StructfieldList(st, tmp);
  }

  /* struct _thrarg_ { ... } */
  return ( SUdecl(SPEC_struct, Symbol(SHVARSTYPE), st) );
}


/* This one takes all the discovered implicit/explicit nonglobal vars and
 * checks if they comply, i.e. if they are either explicitely shared
 * or there is a default(shared) clause present. They are checked
 * against the dataclause_vars. Also settles the sharing attributes
 * of implicitely used variables (i.e. it determines if a variable
 * is to be considered as shared of firstprivate).
 * (sng_vars ends up containing all variables that do not appear in
 * a "private" clause, i.e. all implicit ones plus explicit shared and
 * firstprivate)
 */
void xt_task_check_shared(ompcon t)
{
  stentry   e, actual;
  int       defnone, defshared;
  ompclause c = xc_ompcon_get_clause(t, OCDEFAULT);

  defnone   = (c != NULL && c->subtype == OC_defnone);
  defshared = (c != NULL && c->subtype == OC_defshared);
  for (e = sng_vars->top; e; e = e->stacknext)
    if (symtab_get(dataclause_vars, e->key, IDNAME) == NULL)
    {
      if (defnone)    /* implicit but there is a default(none) clause */
        exit_error(1, 
          "(%s) openmp error:\n\t"
          "variable `%s' must be explicitely declared as shared/private\n\t"
          "due to the default(none) clause of the task directive at line %d.\n",
          t->directive->file->name, e->key->name, t->directive->l);
      if (defshared)  /* implicit; but has to be taken as shared */
        e->ival = OCSHARED;
      else
      {
        actual = symtab_get(stab,e->key,IDNAME);
        /* If global and no parallel exists => shared.
           If declared before closest enclosing parallel region => shared. */
        if ((actual->scopelevel == 0 && closest_parallel_scope < 0) ||
             actual->scopelevel <= closest_parallel_scope)
          e->ival = OCSHARED;
        else
        {
          symtab_put(dataclause_vars, e->key, IDNAME)->ival = OCFIRSTPRIVATE;
          e->ival = OCFIRSTPRIVATE;
        }
      }
    };
}


/*
 * Firstprivate/shared-non-global initialization statements
 *
 */
static aststmt fpinits = NULL, snginits = NULL;


/* Produces a memory copy statement, for initializing firstprivate vars
 * which are non-scalar:
 *     memcpy( (void *) var, (void *) &(init), sizeof(var) );
 */
static
aststmt xt_array_initialize_stmt(symbol var)
{
  needMemcpy = 1;
  return
    Expression(
      FunctionCall( 
        Identifier(Symbol("memcpy")),
        CommaList( 
          CommaList( 
            CastedExpr( 
              Casttypename(
                Declspec(SPEC_void, 0),
                AbstractDeclarator(Pointer(),NULL)
              ), 
              PtrField(Identifier(Symbol(SHVARSNAME)), var) 
            ), 
            CastedExpr( 
              Casttypename(
                Declspec(SPEC_void, 0),
                AbstractDeclarator(Pointer(),NULL)
              ), 
              Identifier(var)
            ) 
          ), 
          Sizeof(Identifier(var)) 
        ) 
      ) 
    );
}


/* Produces a memory copy statement, for initializing firstprivate vars
 * which are non-scalar & adds the statement to "fpinits".
 */
static
void xt_fpsnapshot_initialization(symbol var, int isarray)
{
  aststmt st;

  if (isarray) 
    st = xt_array_initialize_stmt(var);
  else
    st = Expression(
           Assignment(
             PtrField(Identifier(Symbol(SHVARSNAME)), var), 
             ASS_eq, 
             Identifier(var)
           )
         );
  fpinits = (fpinits == NULL) ? st : BlockList(fpinits, st);
}


/* This one produces declaration statements for all private and
 * firstprivate variables, in the case where the original task code
 * is left as is. If, however, such a variable is never modified within
 * the task, we can safely reference it as is (without privatizing it).
 * Except if the variable was shared and it is *explicitely* marked 
 * as private/firstprivate. In that case we cannot do anyhting since
 * the original shared variable may be subject to modifications by
 * other threads. Well, we'll see...
 */
static
aststmt dupcode_private_declarations(aststmt *fpinits)
{
  stentry e;
  aststmt st = NULL, inist = NULL, tmp;

  for (e = dataclause_vars->top; e; e = e->stacknext)
  {
    if (isPrivate(e->key))
      tmp = xform_clone_declaration(e->key, NULL);
    else
      if (isFirstprivate(e->key))
      {
        if ( symtab_get(stab, e->key, IDNAME)->isarray )
        {
          char flvar[256];
          snprintf(flvar, 255, "_fip_%s", e->key->name);
          tmp = xc_array_initializer(e->key,   /* *flvar */
                        UnaryOperator(UOP_star, Identifier(Symbol(flvar))));
          inist = (inist == NULL) ? tmp : BlockList(inist, tmp);
        } 
        tmp = xc_firstprivate_declaration(e->key);
      }
      else
        continue;
    st = (st == NULL) ? tmp : BlockList(st, tmp);
  }
  *fpinits = inist;
  return (st);
}


/*
 * Declarations-related stuff that go into the task function.
 *
 */

 
/* Produces a list of declarations that declare pointer variables,
 * initialized to where the structure fields point to.
 * Those declarations will go to the top of the thread function,
 * and cover the shared non-global variables.
 * Instead of looking into the shared() clauses, we look at all the
 * discovered shared vars in sng_vars.
 * 
 * In addition it prepares initialization statements (snginits) for the
 * taskenv fields (these go into the injected code).   
 */
aststmt xt_sharedng_declarations()
{
  stentry e, orig;
  aststmt st = NULL, tmp;
  
  for (e = sng_vars->top; e; e = e->stacknext)
  {
    /* First filter out any extra inserted vars */
     if (isFirstprivate(e->key)) 
      continue;      /* Declared elsewhere */
    
    /* Declaration with initializer (  = argname->var ).
     * Non-global threadprive vars are a bit more complex.
     */
    orig = symtab_get(stab, e->key, IDNAME);
    if (threadmode && orig->isthrpriv)
      tmp = tp_declaration(orig, e->key,
                           PtrField(Identifier(Symbol(SHVARSNAME)), e->key), 1);
    else
      tmp = Declaration(
              ast_spec_copy_nosc(orig->spec),
              InitDecl(
                xc_decl_topointer(     /* Same decl but change to pointer */
                  ast_decl_copy(orig->decl)
                ),
                PtrField(Identifier(Symbol(SHVARSNAME)), e->key)
              )
            );
    st = (st == NULL) ? tmp : BlockList(st, tmp);
    
    /* Initialized for the taskenv fields */
    tmp = Expression(
            Assignment(
              PtrField(Identifier(Symbol(SHVARSNAME)), e->key),
              ASS_eq,
              (threadmode && orig->isthrpriv) ?
                Identifier(e->key) :       /* 'cause it is already a pointer */
                UnaryOperator(UOP_addr, Identifier(e->key))
            )
          );
    snginits = (snginits == NULL) ? tmp : BlockList(snginits, tmp);   
  }
  return (st);
}


/* Produces a list of declarations that cover firstprivate() variables.
 * Those are initialized from the structure fields directly, at the
 * beginning of the task.
 */
aststmt xt_firstprivate_declarations()
{
  stentry e;
  aststmt st = NULL, tmp;
  astexpr init = NULL;
  astdecl decl;
  int     array;
  
  for (e = dataclause_vars->top; e; e = e->stacknext)
  {
    if (!isFirstprivate(e->key)) continue;
    
    array = symtab_get(stab, e->key, IDNAME)->isarray;
    
    /* The declarator */
    decl = ast_decl_copy(symtab_get(stab, e->key, IDNAME)->decl);
    if (array)                  /* Pointer to array */
      xc_decl_topointer(decl);

    /* The initializer (used either as is or to produce array initializer) */
    init = PtrField(Identifier(Symbol(SHVARSNAME)), e->key);
    if (array)
      init = UnaryOperator(UOP_addr, UnaryOperator(UOP_paren, init));

    /* Declaration with initializer. */
    tmp = Declaration(
            ast_spec_copy_nosc(symtab_get(stab, e->key, IDNAME)->spec),
            InitDecl(decl, init)      /* = * or & (_thrarg->var) */
          );

    /* Snapshot initializer */
    xt_fpsnapshot_initialization(e->key, array);

    st = (st == NULL) ? tmp : BlockList(st, tmp);
  }
  return (st);
}


/* Produces a list of declarations for private() variables.
 */
aststmt xt_private_declarations()
{
  stentry e;
  aststmt st = NULL, tmp;

  for (e = dataclause_vars->top; e; e = e->stacknext)
  {
    if (!isPrivate(e->key)) continue;
    tmp = xform_clone_declaration(e->key, NULL);
    st = (st == NULL) ? tmp : BlockList(st, tmp);
  }
  return (st);
}
                        

/* This also checks if the clause is unique */
astexpr xt_if_clause(ompclause t)
{
  astexpr e = NULL;
  
  if (t->type == OCLIST)
  {
    if (t->u.list.next != NULL)   /* depth-first to check uniqueness */
      e = xt_if_clause(t->u.list.next);
    assert((t = t->u.list.elem) != NULL);
  }
  if (t->type == OCIF)
  {
    if (e != NULL)
      exit_error(1, "(%s, line %d) openmp error:\n\t"
                    "multiple %s() clauses in task directive.\n",
                    t->file->name, t->l, clausenames[OCIF]);
    e = t->u.expr;
  }
  return (e);
}


/* Returns the expression of the unique if() clause or NULL
 */
astexpr xt_if(ompdir d)
{
  return ( d->clauses ? xt_if_clause(d->clauses) : NULL );
}


/* This also checks if the clause is unique */
astexpr xt_final_clause(ompclause t)
{
  astexpr e = NULL;
  
  if (t->type == OCLIST)
  {
    if (t->u.list.next != NULL)   /* depth-first to check uniqueness */
      e = xt_final_clause(t->u.list.next);
    assert((t = t->u.list.elem) != NULL);
  }
  if (t->type == OCFINAL)
  {
    if (e != NULL)
      exit_error(1, "(%s, line %d) openmp error:\n\t"
                    "multiple %s() clauses in task directive.\n",
                    t->file->name, t->l, clausenames[OCFINAL]);
    e = t->u.expr;
  }
  return (e);
}


/* Returns the expression of the unique if() clause or NULL
 */
astexpr xt_final(ompdir d)
{
  return ( d->clauses ? xt_final_clause(d->clauses) : NULL );
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     THE CORE FUNCTION                                         *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Repeats all visible non-global struct/union/enum declarations */
static
aststmt xt_sue_declarations()
{
  stentry e;
  aststmt all = NULL, d;
  
  for (e = stab->top; e; e = e->stacknext)
  {
    if (e->scopelevel <= 1) break;       /* stop @ globals/funcparams */
    if (e->space == SUNAME || e->space == ENUMNAME)
      if (e->spec)
      {
        d = Declaration(ast_spec_copy_nosc(e->spec), ast_decl_copy(e->decl));
        all = (all) ? BlockList(all, d) : d;
      };
  }
  return (all);
}


#define has_tsng_vars()  (shvardecl != NULL)
#define has_tfp_vars()   (fpvardecl != NULL)


void xform_task(aststmt *t)
{
  aststmt body = (*t)->u.omp->body, body_copy;  /* Remember current node */
  aststmt p = (*t)->parent, funcdef, v,
          prvardecl = NULL, shvardecl = NULL, fpvardecl = NULL;
  aststmt fast, fastdecls = NULL, fastfpinits = NULL;   /* For the fast path */
  astexpr ifexpr, finalexpr;
  int     untied, compos, bodyline, mergeable;
  str     st1 = Strnew();
  symbol  sfile = (*t)->file;
  
  new_taskfunc();                     /* Get a number */
  fpinits = snginits = NULL;
  
  /* (1) Put the directive in a comment
   */
  str_printf(st1, "/* (l%d) ", ((*t)->u.omp->directive)->l);
  ast_ompdir_print(st1, ((*t)->u.omp->directive));
  compos = str_tell(st1);             /* Remember where we are now */
  str_printf(st1, " */");
  v = Verbatim( strdup( str_string(st1) ) );
  bodyline = ((*t)->u.omp->directive)->l;
  body_copy = ast_stmt_copy(body);    /* Keep a full copy */
  
  /* (2) Remember our data clause variables. 
   */
  xt_store_dataclause_vars((*t)->u.omp->directive);
  
  /* (3) Check for untied, mergeable, if and final clauses and keep a copy
   */
  if ((ifexpr = xt_if((*t)->u.omp->directive)) != NULL)
    ifexpr = ast_expr_copy(ifexpr);
  if ((finalexpr = xt_final((*t)->u.omp->directive)) != NULL)
    finalexpr = ast_expr_copy(finalexpr);
  untied = (xc_ompcon_get_clause((*t)->u.omp, OCUNTIED) != NULL);
  mergeable = (xc_ompcon_get_clause((*t)->u.omp, OCMERGEABLE) != NULL);

  /* (4) Discover shared non-globals and check for default() compliance 
   */
  ast_taskcon_find_sngfp_vars((*t)->u.omp, 1);
  xt_task_check_shared((*t)->u.omp);

  /* (5) Prepare the declarations for local & shared variables 
   */
  shvardecl = xt_sharedng_declarations();
  prvardecl = xt_private_declarations();
  fpvardecl = xt_firstprivate_declarations();
  /* Combine to reduce options */
  if (prvardecl && fpvardecl)
    prvardecl = BlockList(fpvardecl, prvardecl);
  else
    if (prvardecl == NULL)
      prvardecl = fpvardecl;

  (*t)->u.omp->body = NULL;
  ast_free(*t);                 /* Get rid of the OmpStmt */

  /* (6) Replace the current node with the following new code that creates 
   *     the task ("slow" path):
   *       {
   *         <commented directive>
   *         <structure declaration(s) if needed>
   *         <task function declaration>
   *
   *         ort_new_task(...)
   *       }
   */
  if (!has_tsng_vars() && !has_tfp_vars())
    *t = v;
  else
  {
    *t = BlockList(
           v,
           Declaration( xt_taskenv_struct(), /* Define the struct & declare */
             Declarator(
               Pointer(),
               IdentifierDecl(Symbol(SHVARSNAME))
             )
           )
         );
    *t = BlockList(              /* env = ort_taskenv_alloc(sizeof(struct)) */
           *t,
           Expression(
             Assignment(
               Identifier(Symbol(SHVARSNAME)),
               ASS_eq, 
               CastedExpr(
                 Casttypename(
                   SUdecl(SPEC_struct, Symbol(SHVARSTYPE), NULL),
                   AbstractDeclarator(Pointer(),NULL)
                 ),
                 FunctionCall(
                   Identifier(Symbol("ort_taskenv_alloc")),
                   CommaList(
                     Sizeoftype(Casttypename(
                       SUdecl(SPEC_struct, Symbol(SHVARSTYPE), NULL),
                       NULL
                     )),
                     Identifier(Symbol(taskfuncname()))
                   )
                 )
               )
             )
           )
         );
  }
  if (has_tsng_vars() && snginits)
    *t = BlockList(*t, snginits);
  if (has_tfp_vars() && fpinits)
    *t = BlockList(*t, fpinits);
  *t = Compound(
         BlockList(
           *t,
           Expression(  /* ort_new_task(tied(0)/untied(1), taskfunc, arg/0) */
             FunctionCall(
               Identifier(Symbol("ort_new_task")),
               CommaList(
                 CommaList(
                   CommaList(
                     finalexpr ? ast_expr_copy(finalexpr) : numConstant(0),
                     numConstant(untied ? 1 : 0)
                   ),
                   Identifier(Symbol(taskfuncname()))
                 ),
                 CastedExpr( 
                   Casttypename(
                     Declspec(SPEC_void, 0),
                     AbstractDeclarator(Pointer(),NULL)
                   ),
                   (has_tsng_vars() || has_tfp_vars()) ? 
                     Identifier(Symbol(SHVARSNAME)) :
                     numConstant(0)
                 ) 
               )
             )
           )
         )
       );

  /* (7) Also produce a "fast" path:
   *       {
   *         <possible local declarations / initializations>
   *         ort_task_immediate_start(...)
   *           <original code>
   *         ort_task_immediate_end(...)
   *       }
   */
  fastdecls = dupcode_private_declarations(&fastfpinits);
  fast = Compound(
            BlockList(   /* if mergeable, no decls needed */ 
              ((fastdecls && !mergeable) ? 
                BlockList(
                  Declaration(Declspec(SPEC_void, 0),   /* void *_itn; */
                              Declarator(Declspec(SPEC_star, 0),
                                        IdentifierDecl(Symbol("_itn")))),
                  ( fastfpinits ? BlockList(fastdecls,fastfpinits) : fastdecls )
                ) :
                Declaration(Declspec(SPEC_void, 0),     /* void *_itn; */
                            Declarator(Declspec(SPEC_star, 0),
                                        IdentifierDecl(Symbol("_itn"))))
              ),
              BlockList(
                Expression(    /* _itn = ort_task_immediate_start(); */
                  Assignment(Identifier(Symbol("_itn")), ASS_eq,
                             FunctionCall(
                               Identifier(Symbol("ort_task_immediate_start")),
                               finalexpr ? finalexpr : numConstant(0)
                             ))
                ),
                BlockList(
                  body_copy,
                  Expression(  /* ort_task_immediate_end(_itn); */
                    FunctionCall(Identifier(Symbol("ort_task_immediate_end")), 
                                 Identifier(Symbol("_itn")))
                  )
                )
              )
            )
          );

  /* (8) Combine slow & fast paths with a possibly even faster third path
   */
  if (enableCodeDup && mergeable)
  {
    *t = If(( ifexpr ? 
                BinaryOperator(BOP_lor,
                  UnaryOperator(UOP_lnot, UnaryOperator(UOP_paren, ifexpr)),
                  Call0_expr("ort_task_throttling")) :
                Call0_expr("ort_task_throttling")
            ),
            fast,         
            *t);
    *t = If(Call0_expr("omp_in_final"), 
            Compound(ast_stmt_copy(body_copy)), *t);
    ast_stmt_renlabs( (*t)->body );   /* must rename labels, if any */
  }
  else
  {
    *t = If(BinaryOperator(BOP_lor,
              ( ifexpr ? 
                  BinaryOperator(BOP_lor,
                    UnaryOperator(UOP_lnot, UnaryOperator(UOP_paren, ifexpr)),
                    Call0_expr("omp_in_final")) :
                  Call0_expr("omp_in_final")
              ),
              Call0_expr("ort_task_throttling")
            ),
            fast,         
            *t);
  }
  
  (*t)->file = sfile;
  ast_stmt_parent(p, *t);

  /* (9) Prepare the body of the new function 
   */
         /* Take the current body, add a return( (void *) 0 ) statement
          * to avoid warnings and a couple of comments; the whole thing
          * will be enclosed in a compound statement.
          * If there are any firstprivate initialization statements
          * (for arrays), prepend them.
          */
  str_insert(st1, compos, " -- body moved below");  /* Another comment */
  str_printf(st1, "\n# %d \"%s\"", bodyline, body->file->name);
  v = Verbatim( strdup( str_string(st1) ) );
  str_truncate(st1);
  body = BlockList(v, body);
  
         /* Declare any variables from the clauses.
          */
  if (has_tsng_vars() || has_tfp_vars())
  {
    /* Declare _thrarg = (argtype *) __tdata; */
    aststmt castarg = Declaration(
                        xt_taskenv_struct(),
                        InitDecl(
                          Declarator(
                            Pointer(),
                            IdentifierDecl( Symbol(SHVARSNAME) )
                          ),
                          CastedExpr(
                            Casttypename(
                              SUdecl(SPEC_struct,Symbol(SHVARSTYPE),NULL),
                              AbstractDeclarator(Pointer(),NULL)
                            ),
                            Identifier(Symbol("__tdata"))
                          )
                        )
                      );
    /* For sure, we need to declare _thrarg.
     * We may also have shared vars, and maybe private vars, too.
     * We may also have firstprivate initializations.
     * Then we have the body.
     */
    if (shvardecl && prvardecl)
      body = BlockList(
               BlockList( BlockList( castarg, shvardecl ), prvardecl ),
               body
             );
    if (shvardecl && !prvardecl)
      body = BlockList( BlockList( castarg, shvardecl ), body );
    if (!shvardecl && prvardecl)
      body = BlockList( BlockList( castarg, prvardecl ), body );
    if (!shvardecl && !prvardecl)
      body = BlockList( castarg, body );
  }
  else
    if (prvardecl)
      body = BlockList( prvardecl, body );
  
  if ((p = xt_sue_declarations()) != NULL)
    body = BlockList(p, body);
 
  if (has_tsng_vars() || has_tfp_vars()) 
    body = BlockList(
             body,
             BlockList(
               Expression(
                 FunctionCall(
                   Identifier(Symbol("ort_taskenv_free")),
                   CommaList(
                     Identifier( Symbol(SHVARSNAME) ),
                     Identifier(Symbol(taskfuncname()))
                   )
                 )
               ),
               verbit( "return (void *) 0;" )
             )
           );
  else
    body = BlockList( body, verbit( "return (void *) 0;" ) );
  body = Compound(body);
  

  /* (10) Define the new function.
   *         void *func ( void *__tdata ) <body>
   */
  funcdef = FuncDef(Speclist_right(StClassSpec(SPEC_static),Declspec(SPEC_void, 0)),
                    Declarator( 
                      Pointer(), 
                      FuncDecl( 
                        IdentifierDecl( Symbol(taskfuncname()) ) ,
                        ParamDecl( Declspec(SPEC_void, 0),
                                   Declarator(
                                     Pointer(),
                                     IdentifierDecl( Symbol("__tdata") )
                                   )
                                 )
                      ) 
                    ),
                    NULL, body);
  
  /* (11) Add the new function, along with the struct type definition
   */
  ast_stmt_parent(funcdef, funcdef);     /* Parentize nicely */
  xfrom_add_threadfunc(Symbol(taskfuncname()), funcdef,
                       ast_get_enclosing_function(*t));
    
  str_free(st1);
}
