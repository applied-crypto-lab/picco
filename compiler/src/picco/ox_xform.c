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

/* ox_xform.c */

/*
 * 2012/03/08:
 *   new dynamic outsize support
 * 2010/02/28:
 *   added taskschedule transformation
 * 2009/05/04:
 *   added ATNODE clause(s).
 * 2008/10/26
 *   BY_VAL handled through BY_REF
 * 2008/10/23
 *   added uponreturn (callbacks)
 * 2008/10/21
 *   added tasksync transformation
 * 2008/07/07:
 *   new!
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ast.h"
#include "symtab.h"
#include "parser.h"
#include "picco.h"
#include "ast_copy.h"
#include "ast_free.h"
#include "ast_print.h"
#include "ast_xform.h"
#include "x_clauses.h"
#include "str.h"

/* argument type encoding = type | sizer | gamut
 */
#define OX_GAMUT_SIGNED   0
#define OX_GAMUT_UNSIGNED 1
#define OX_SIZER_DEFAULT  0
#define OX_SIZER_SHORT    2
#define OX_SIZER_LONG     4
#define OX_TYPE_INT       0
#define OX_TYPE_CHAR      8
#define OX_TYPE_FLOAT     16
#define OX_TYPE_DOUBLE    24

/* from the lwrte library */
#define CALL_BY_VAL  "0"   /* IN   - By value (not used any more actually) */
#define CALL_BY_PTR  "1"   /* IN   - By value, from address */
#define CALL_BY_REF  "2"   /* INOUT- By reference */
#define CALL_BY_RES  "3"   /* OUT  - By result */
#define CALL_BY_RMA  "4"   /* ... */
#define CALL_BY_RED(op)    /* REDUCTION - OUT with a reduction operator */\
          (((op) == OC_plus) ? "0x100" : "0x200")

/* Parameter size specification */
#define SIZE_UNKNOWN   0  /* pointer or undetermined size */
#define SIZE_SCALAR    1  /* scalar parameter */
#define SIZE_DECLEXPR  2  /* size given in the array declaration expression */
#define SIZE_CLAUSEXPR 3  /* size given in a TASKDEF clause expression */
#define SIZE_DYNAMIC   4  /* size of out() array is dynamic */

#define CALLBACK_TAIL "_callback"

/* Info about the arguments of a task function */
typedef struct {
  symbol  pid;           /* The param name */
  int     intent;        /* Intent of usage: in/out/inout/etc */
  int     way;           /* Way of sending data -> default, rma, dsm */
  int     op;            /* Reduction operator */
  int     ptr;           /* 1 if it is a pointer (arrays are pointers) */
  int     type;          /* The basic type */
  int     size;          /* Size (0: unknown, 1: scalar,
                            2: declaration expression, 3: clause expression ) */
  astexpr size_expr;     /* The size expression if array parameter */
  astdecl decl;          /* Pointer to the actual declaration */
} tfparam;

/* List of defined task functions */
typedef struct taskfunc_ *taskfunc;
struct taskfunc_ {
  oxcon    tfcon;         /* The construct */
  symbol   tfid;          /* The task function name */
  int      np;            /* # params */
  tfparam  *p;            /* The params */
  int      has_callback;
  taskfunc next;
};

/* The table of task functions */
static taskfunc tftable = NULL;

extern symtab   oxc_validate_store_dataclause_vars(oxdir d);
extern int      count_function_params(astdecl d);
extern void     info_function_params(astdecl d, tfparam *p);
extern void     types_function_params(astdecl d, tfparam *p);
extern void     ox_xform_task(aststmt *t);
extern void     ox_validate_clauses(enum oxdircontype dirtype, oxclause t);
extern oxclause ox_has_clause(oxclause t, enum oxclausetype type);

/* New name for CALL_BY_VAL arguments */
static char infarg_str[128];
static symbol infarg(symbol s) 
              { sprintf(infarg_str, "_ompix_%s", s->name); 
                return ( Symbol(infarg_str) ); }
/* Name of the callback function */
static symbol callback_func(symbol fid)
              { static char cbfn[256];
                strncpy(cbfn, fid->name, 255-strlen(CALLBACK_TAIL));
                strcat(cbfn, CALLBACK_TAIL);
                return (Symbol(cbfn)); }


static
void oxcon_xform(oxcon *t)
{
  if ((*t)->body)
    ast_stmt_xform(&((*t)->body));
}
      

static
aststmt oxdir_commented(oxdir d)
{
  static str bf = NULL;
  aststmt    st;
  
  if (bf == NULL) bf = Strnew();
  str_printf(bf, "/* ");
  ast_oxdir_print(bf, d);
  str_printf(bf, " */");
  st = BlockList( Verbatim( strdup( str_string(bf) ) ),
                  verbit("# %d \"%s\"", d->l, d->file->name) );
  str_truncate(bf);
  return (st);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     TASKDEF TRANSFORMATION                                    *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


static
taskfunc get_taskfunc(symbol fid)
{
  taskfunc tf;

  for (tf = tftable; tf != NULL; tf = tf->next)
    if (tf->tfid == fid)
      break;
  return (tf);
}


/* Creates a new task function structure with everything but
 * the types of the parameters.
 */
taskfunc Taskfunction(oxcon c)
{
  taskfunc tf;
  symtab   vars;
  stentry  e;
  tfparam  *p;
  int      i;
  symbol   fid = decl_getidentifier_symbol(c->body->u.declaration.decl);

  if (get_taskfunc(fid) != NULL)
    exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                  "task function `%s' has already been defined!\n",
                   c->file->name, c->l+1, fid->name);
  
  tf = smalloc( sizeof(struct taskfunc_) );
  tf->has_callback = (c->callback != NULL);
  tf->tfcon = c;
  tf->tfid  = fid;
  tf->np    = count_function_params(c->body->u.declaration.decl);
  tf->p     = smalloc((tf->np)*sizeof(tfparam));
  info_function_params(c->body->u.declaration.decl, tf->p);
  tf->next  = tftable;
  tftable = tf;

  vars = oxc_validate_store_dataclause_vars(c->directive);
  for (e = vars->top; e; e = e->stacknext)
  {
    /* Find it in the params */
    for (i = 0, p = NULL; i < tf->np; i++)
      if (tf->p[i].pid == e->key)
      {
        p = &(tf->p[i]);
        break;
      };
    if (p == NULL)
      exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                    "variable `%s' is not a task function parameter\n",
                     c->directive->file->name, c->directive->l, e->key->name);
    p->intent = e->ival;              /* Call intent */
    p->op  = e->vval;

    if (e->decl->type != DARRAY)   /* clause var with no size */
    {
      if (p->size == SIZE_UNKNOWN) /* pointer/undetermined */
      {
        /*
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                  "cannot determine the size of task function parameter `%s'\n",
                  c->directive->file->name, c->directive->l, p->pid->name);
        */
        warning("(%s, line %d) OMPi-X warning:\n\t"
                  "the size of task function parameter `%s' is assumed to be 1.\n",
                  c->directive->file->name, c->directive->l, p->pid->name);
        p->size = SIZE_CLAUSEXPR;
        p->size_expr = numConstant(1);
      }
      if (p->size == SIZE_SCALAR && p->intent != OX_OCIN)
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                  "task function parameter `%s' cannot be Out/InOut\n",
                  c->directive->file->name, c->directive->l, p->pid->name);
    }
    else                           /* clause var with size */
    {
      if (p->size == SIZE_SCALAR)  /* fixed 1 element */
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                      "contradicting sizes for task function parameter `%s'\n",
                      c->directive->file->name, c->directive->l, p->pid->name);
      if (p->size == SIZE_DECLEXPR)  /* expression */
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                 "multiple size expressions for task function parameter `%s'\n",
                 c->directive->file->name, c->directive->l, p->pid->name);
      if (e->decl->spec != NULL)   /* Dynamic size */
      {
        if (p->intent != OX_OCOUT)
          exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                     "task function parameter `%s' cannot be In/InOut\n",
                     c->directive->file->name, c->directive->l, p->pid->name);
        p->size = SIZE_DYNAMIC;
      }
      else
        p->size = SIZE_CLAUSEXPR;  /* Clause size expression */
      p->size_expr = e->decl->u.expr;
    }
  }

  /* Final check */
  for (i = 0; i < tf->np; i++)
  {
    p = &(tf->p[i]);
    if (p->intent == -1)     /* Undetermined yet */
    {
      if (p->size == SIZE_UNKNOWN)
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                   "cannot determine how to use task function parameter `%s'\n",
                   c->directive->file->name, c->directive->l, p->pid->name);
      else
        if (p->size == SIZE_SCALAR)
          p->intent = OX_OCIN;          /* Just a plain scalar */
        else
          p->intent = OX_OCINOUT;       /* INOUT so as to be on the safe side */
    }
    if (p->size == SIZE_UNKNOWN)
      exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
             "cannot determine (II) the size of task function parameter `%s'\n",
             c->directive->file->name, c->directive->l, p->pid->name);

    if (p->size == SIZE_SCALAR)      /* has to be OX_OCIN */
    {
      astdecl new = ast_decl_copy(p->decl);
      aststmt l;

      xc_decl_rename(p->decl, infarg(p->pid));
      xc_decl_topointer(p->decl);
      p->decl = new;                 /* so as to later clone the original one */
      l = Declaration( ast_spec_copy_nosc(new->spec),
                       InitDecl(
                         ast_decl_copy(new->decl),
                         UnaryOperator(
                           UOP_star,
                           Identifier(infarg(p->pid))
                         )
                       )
                     );
      if (c->body->body->body != NULL)
        c->body->body->body = BlockList(l, c->body->body->body);
      else
        c->body->body->body = l;
    }
    
    if (p->size == SIZE_DYNAMIC)
    {
      int j;
      astexpr t;
      
      /* Find size reference in the params (could it be a global var??) */
      for (j = 0; j < tf->np; j++)
        if (tf->p[j].pid == p->size_expr->u.sym)
        {
          if (decl_ispointer(tf->p[j].decl))
            t = FunctionCall(
                  Identifier(Symbol("torc_set_outsize")),
                  CommaList(Identifier(p->pid), Identifier(tf->p[j].pid))
                );
          else
            t = FunctionCall(
                  Identifier(Symbol("torc_set_outsize")),
                  CommaList(Identifier(p->pid), 
                            UnaryOperator(UOP_addr, Identifier(tf->p[j].pid)))
                );
          break;
        };

      ast_compound_insert_statement(c->body->body, Expression(t));
    }
  }
  return (tf);
}


/* Creates a new callback function definition (from the UponReturn code)
 */
static
aststmt Callbackfunction(oxcon c, taskfunc tf)
{
  aststmt cbf;
  astdecl fid;
  int     i;
  tfparam *p;

  cbf = ast_stmt_copy(c->body);   /* Copy the task function */
  ast_stmt_free(cbf->body);       /* Free the body */
  cbf->body = c->callback;        /* The callback body */
  fid = decl_getidentifier(cbf->u.declaration.decl);
  fid->u.id = callback_func(fid->u.id);  /* ... but with new name */

  /* Take care of the scalar IN arguments as in Taskfunction() */
  for (i = 0; i < tf->np; i++)
    if ((p = &(tf->p[i]))->size == SIZE_SCALAR)      /* has to be OX_OCIN */
      cbf->body->body = 
        BlockList(
          Declaration( ast_spec_copy_nosc(p->decl->spec),
                       InitDecl(
                         ast_decl_copy(p->decl->decl),
                         UnaryOperator(
                           UOP_star,
                           Identifier(p->pid)
                         )
                       )
                     ),
          cbf->body->body
        );

  ast_parentize(cbf);
  return (cbf);
}


static
void ox_xform_taskdef(aststmt *t)
{
  /* Create the task function; this settles the function
   * parameter sizes, but NOT their type. We'll do that later,
   * after the function transformation.
   */
  taskfunc tf = Taskfunction((*t)->u.ox);
  aststmt  new, cbf;

  oxcon_xform(&((*t)->u.ox));
  types_function_params((*t)->u.ox->body->u.declaration.decl, tf->p);
    
  if (tf->has_callback)
  {
    cbf = Callbackfunction((*t)->u.ox, tf);
    new = BlockList(oxdir_commented((*t)->u.ox->directive),
                    BlockList((*t)->u.ox->body, cbf));
    cbf->parent = new;
    ast_stmt_xform(&cbf);
  }
  else
    new = BlockList(oxdir_commented((*t)->u.ox->directive),
                    (*t)->u.ox->body);
  (*t)->parent = new;
  new->parent = (*t)->parent;
  free(*t);
  *t = new;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     TASKSCHEDULE TRANSFORMATION                               *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


static
void ox_xform_taskschedule(aststmt *t)
{
  aststmt parent = (*t)->parent, v, f;
  int tied = -1, scopetype = -1;
  oxclause start, stride, scope;

  ox_validate_clauses((*t)->u.ox->directive->type, 
                      (*t)->u.ox->directive->clauses);
  if (ox_has_clause((*t)->u.ox->directive->clauses, OX_OCUNTIED))
    tied = 0;
  else 
    if (ox_has_clause((*t)->u.ox->directive->clauses, OX_OCTIED))
      tied = 1;
  start = ox_has_clause((*t)->u.ox->directive->clauses, OX_OCSTART);
  stride = ox_has_clause((*t)->u.ox->directive->clauses, OX_OCSTRIDE);
  scope = ox_has_clause((*t)->u.ox->directive->clauses, OX_OCSCOPE);
  if (scope) scopetype = scope->u.value;

  f =  Expression( 
         FunctionCall(
          Identifier(Symbol("torc_taskschedule")),
          CommaList(
            CommaList(
              CommaList(
                numConstant(scopetype == OX_SCOPE_NODES ? 0 :
                            scopetype == OX_SCOPE_WLOCAL ? 1 :
                            scopetype == OX_SCOPE_WGLOBAL ? 2 :
                            -1),                                 /* 1 */
                (start) ? ast_expr_copy(start->u.expr) : 
                          numConstant(-1)                        /* 2 */
              ),
              (stride) ? ast_expr_copy(stride->u.expr) :
                         Constant(strdup("(1<<31)-1"))           /* 3 */
            ),
            numConstant(tied == -1 ? -1 : ( tied == 0 ? 0 : 1 )) /* 4 */
          )
        )
      );
  v = oxdir_commented((*t)->u.ox->directive);
  ast_free(*t);
  *t = BlockList(v, f);
  ast_stmt_parent(parent, *t);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OTHER TRANSFORMATIONS                                     *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


static
void ox_xform_tasksync(aststmt *t)
{
  aststmt parent = (*t)->parent, v;
  
  v = oxdir_commented((*t)->u.ox->directive);
  ast_free(*t);
  *t = BlockList(v, Call0_stmt("torc_tasksync"));
  ast_stmt_parent(parent, *t);
}


void ast_ompix_xform(aststmt *t)
{
  switch ( (*t)->u.ox->type )
  {
    case OX_DCTASKDEF:
      ox_xform_taskdef(t);
      break;
    case OX_DCTASK:
      ox_xform_task(t);
      break;
    case OX_DCTASKSYNC:
      ox_xform_tasksync(t);
      break;
    case OX_DCTASKSCHEDULE:
      ox_xform_taskschedule(t);
      break;
    default:
      oxcon_xform(&((*t)->u.ox));
      break;
  }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     DATA CLAUSE VARIABLES                                     *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* The following 4 functions are used to gather all variables appearing
 * in a directive's data clauses. They also detect duplicates.
 * They use a local symbol table to do it.
 *
 * For each var in the table, the ival field stores the clause type
 * and the "decl" field holds a pointer to the actual declaration
 * (so as to grab the size from there).
 */
static symtab   dc_vars = NULL;    /* Store the list here */
static oxclause dc_vars_clause;    /* Only needed for error messages */


static
void checkNstore_varlist_vars(astdecl d, int clausetype, int operator)
{
  stentry e;
  symbol  vid;
  
  if (d->type == DLIST && d->subtype == DECL_idlist)
  {
    checkNstore_varlist_vars(d->u.next, clausetype, operator);
    d = d->decl;
  }
  if (d->type != DIDENT && !(d->type == DARRAY && d->decl->type == DIDENT))
    exit_error(1, "[checkNstore_varlist_vars]: !!BUG!! ?!\n");
  
  vid = ((d->type == DARRAY) ? d->decl : d)->u.id;
  if ((e = symtab_get(dc_vars, vid, IDNAME)) != NULL)
    exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                  "variable `%s' appears more than once in "
                  "the directive's clause(s)\n",
                  dc_vars_clause->file->name, dc_vars_clause->l,
                  vid->name);
  e = symtab_put(dc_vars, vid, IDNAME);
  e->ival = clausetype;
  e->vval = operator;
  e->decl = d;
}


static
void checkNstore_dcclause_vars(oxclause t)
{
  if (t->type == OX_OCLIST)
  {
    if (t->u.list.next != NULL) 
      checkNstore_dcclause_vars(t->u.list.next);
    assert((t = t->u.list.elem) != NULL);
  }
  dc_vars_clause = t;
  switch (t->type)
  {
    case OX_OCIN:
    case OX_OCOUT:
    case OX_OCINOUT:
    case OX_OCREDUCE:
      if (t->u.varlist)
        checkNstore_varlist_vars(t->u.varlist, t->type, t->operator);
      break;
  }
}


/* Checks for duplicates AND keeps the list of vars; returns the table */
symtab oxc_validate_store_dataclause_vars(oxdir d)
{
  if (dc_vars == NULL)
    dc_vars = Symtab();
  else
    symtab_drain(dc_vars);
  if (d->clauses)
    checkNstore_dcclause_vars(d->clauses);
  return (dc_vars);
}


/* Count function parameters
 */

static
int count_paramlist_vars(astdecl d)
{
  int n = 0;
  
  if (d->type == DLIST && d->subtype == DECL_paramlist)
  {
    n = count_paramlist_vars(d->u.next);
    d = d->decl;
  }
  if (d->type == DELLIPSIS)
    exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                  "variable # arguments not supported in task functions.\n",
                  d->file->name, d->l);
  if (d->type != DPARAM)
    exit_error(1, "[count_paramlist_vars]: !!BUG!! not a DPARAM ?!\n");
  if (d->decl != NULL && d->decl->type != ABSDECLARATOR)
    return (n+1);
  return (0);
}

int count_function_params(astdecl d)
{
  d = d->decl;          /* go to the direct declarator */
  if (d == NULL || d->type != DFUNC)
    exit_error(1, "[count_function_params]: !!BUG!! not a FuncDef ?!\n");
  if ((d = d->u.params) == NULL)
    return (0);
  return ( count_paramlist_vars(d) );
}


/* Store info re. function parameters
 */
 
static
tfparam *info_paramlist_vars(astdecl d, tfparam *p)
{
  int type;

  if (d->type == DLIST && d->subtype == DECL_paramlist)
  {
    p = info_paramlist_vars(d->u.next, p);
    d = d->decl;
  }
  if (d->type == DELLIPSIS)
    exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                  "variable # arguments not supported in task functions.\n",
                  d->file->name, d->l);
  if (d->type != DPARAM)
    exit_error(1, "[info_paramlist_vars]: !!BUG!! not a DPARAM ?!\n");
  if (d->decl != NULL && d->decl->type != ABSDECLARATOR)
  {
    p->decl      = d;
    p->pid       = decl_getidentifier_symbol(d->decl);
    p->type      = 0;   /* to be determined later */
    p->ptr       = 0;   /* ditto */     
    p->intent    = -1;  /* undetermined yet */
    p->size      = SIZE_UNKNOWN;   /* ditto */
    p->size_expr = NULL;

    type = decl_getkind(d->decl);
    if (type == DFUNC)
      exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                 "functions are not allowed as parameters of task functions.\n",
                 d->file->name, d->l);
    if (type == DARRAY)            /* Must record the size */
    {
      for (; d->type != DARRAY; d = d->decl)  /* Get to the DARRAY part */
        ;
      if (d->u.expr == NULL)       /* No explicit size */
        p->size = SIZE_UNKNOWN;    /* ptr/unknown */
      else
      {
        p->size = SIZE_DECLEXPR;   /* Expression */
        p->size_expr = ast_expr_copy(d->u.expr);
      }
    }
    else                           /* scalar (DIDENT) */
    {
      if (decl_ispointer(d->decl))
        p->size = SIZE_UNKNOWN;    /* ptr/unknown */
      else
        p->size = SIZE_SCALAR;     /* Fixed 1 element */
    }
    return (p+1);
  }
  return (0);
}


void info_function_params(astdecl d, tfparam *p)
{
  d = d->decl;          /* go to the direct declarator */
  if (d == NULL || d->type != DFUNC)
    exit_error(1, "[info_function_params]: !!BUG!! not a FuncDef ?!\n");
  if ((d = d->u.params) == NULL)
    return;
  info_paramlist_vars(d, p);
}


/* Store info re. function parameters
 */
 
static
tfparam *types_paramlist_vars(astdecl d, tfparam *p)
{
  int type;

  if (d->type == DLIST && d->subtype == DECL_paramlist)
  {
    p = types_paramlist_vars(d->u.next, p);
    d = d->decl;
  }
  if (d->decl != NULL && d->decl->type != ABSDECLARATOR)
  {
    p->pid = decl_getidentifier_symbol(d->decl);
    if (!d->spec) /* int by default */
      type = OX_TYPE_INT | OX_SIZER_DEFAULT | OX_GAMUT_SIGNED;
    else
    {
      if (speclist_getspec(d->spec, SUE, 0) ||
          speclist_getspec(d->spec, USERTYPE, 0) ||
          speclist_getspec(d->spec, ENUMERATOR, 0))
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                   "non-basic types disallowed for task function parameters.\n",
                   d->file->name, d->l);
      type = speclist_getspec(d->spec, SPEC, SPEC_char) ? OX_TYPE_CHAR :
             speclist_getspec(d->spec, SPEC, SPEC_float) ? OX_TYPE_FLOAT :
             speclist_getspec(d->spec, SPEC, SPEC_double) ? OX_TYPE_DOUBLE :
                                                             OX_TYPE_INT;
      type |=
         ( speclist_getspec(d->spec, SPEC, SPEC_short) ? OX_SIZER_SHORT :
           speclist_getspec(d->spec, SPEC, SPEC_long) ? OX_SIZER_LONG :
                                                        OX_SIZER_DEFAULT );
      type |=
         ( speclist_getspec(d->spec, SPEC, SPEC_unsigned) ? OX_GAMUT_UNSIGNED :
                                                            OX_GAMUT_SIGNED );
    }
    p->type = type;
    p->ptr  = decl_ispointer(d->decl);
    return (p+1);
  }
  return (0);
}

void types_function_params(astdecl d, tfparam *p)
{
  d = d->decl;          /* go to the direct declarator */
  if (d == NULL || d->type != DFUNC)
    exit_error(1, "[types_function_params]: !!BUG!! not a FuncDef ?!\n");
  if ((d = d->u.params) == NULL)
    return;
  types_paramlist_vars(d, p);
}



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     TASK CONSTRUCT                                            *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


static taskfunc _tf;
static astexpr  *_callargs;


/* Create the final expression for a taskfunc call argument.
 * Because we change the tree, this is assumed to be a copy of the
 * original taskfunc parameter.
 */
void tf_sizeexpr_make(astexpr *tree)
{
  if (*tree == NULL) return;
  switch ((*tree)->type)
  {
    case IDENT:
    {
      tfparam *p;
      int     i;
      
      /* Check if it is one of the parameters */
      for (i = 0, p = NULL; i < _tf->np; i++)
      {
        /* Because the identifier may have changed, pid points to the new symbol, 
         * so this is not correct:
        if (_tf->p[i].pid == (*tree)->u.sym)
         */
        if (decl_getidentifier_symbol(_tf->p[i].decl) == (*tree)->u.sym)
        {
          p = &(_tf->p[i]);
          break;
        };
      }
      if (p != NULL)     /* yeap */
      {
        /* Substitute with the expression of the i-th parameter */
        ast_expr_free(*tree);
        *tree = UnaryOperator(UOP_paren, ast_expr_copy( _callargs[i] ));
      }
      break;
    }
    case DOTDES:
    case CONSTVAL: 
    case STRING:
      break;
    case FUNCCALL:
    case ARRAYIDX: 
    case DOTFIELD: 
    case PTRFIELD: 
    case BRACEDINIT: 
    case BOP: 
    case PREOP: 
    case POSTOP: 
    case ASS: 
    case DESIGNATED: 
    case IDXDES: 
    case COMMALIST:
    case SPACELIST:
      tf_sizeexpr_make(&((*tree)->left));
      tf_sizeexpr_make(&((*tree)->right));
      break;
    case CASTEXPR: 
      tf_sizeexpr_make(&((*tree)->left));
      break;
    case CONDEXPR: 
      tf_sizeexpr_make(&((*tree)->u.cond));
      tf_sizeexpr_make(&((*tree)->left));
      tf_sizeexpr_make(&((*tree)->right));
      break;
    case UOP: 
      if ((*tree)->opid != UOP_sizeoftype && (*tree)->opid != UOP_typetrick)
      {
        tf_sizeexpr_make(&((*tree)->left));
        tf_sizeexpr_make(&((*tree)->right));
      }
      break;
  }
}


static char inparam_str[16];
static char *inparam(int i) 
             { sprintf(inparam_str, "_ompix_%d", i); return (inparam_str); } 
#define CALLWAY(p) ((p).intent == OX_OCREDUCE ? CALL_BY_RED((p).op) :\
                    (p).intent == OX_OCOUT    ? CALL_BY_RES :\
                    (p).intent == OX_OCINOUT  ? CALL_BY_REF :\
                    CALL_BY_PTR)
                    /* OX_OCIN - CALL_BY_VAL ditched */
                    /* ((p).ptr ? CALL_BY_PTR : CALL_BY_VAL)) */


/* First we have a list of triplets (one for each argument). In a triplet:
 *   the first number is the size
 *   the second number is the type
 *   the third number is the call intent (in/out/reduction/etc).
 * After all triples, we enlist the actual arguments.
 */
static
astexpr callargs_tree(astexpr *ca, taskfunc tf)
{
  astexpr    expr, e;
  int        i;
  static str typstr = NULL;

  if (typstr == NULL) typstr = Strnew();

  _tf = tf;
  _callargs = ca;
  if (tf->np == 0) return (NULL);

  if (tf->p[0].size == 1)           /* the first triplet */
    e = numConstant(1);
  else
  {
    if (tf->p[0].size == SIZE_DYNAMIC)
      e = numConstant(-1);                  /* dynamic size */
    else
    {
      e = ast_expr_copy(tf->p[0].size_expr);
      tf_sizeexpr_make(&e);
    }
  }
  str_truncate(typstr); str_printf(typstr, "%d", tf->p[0].type);
  expr = CommaList(CommaList(e, Constant(strdup(str_string(typstr)))),
                   Constant(strdup(CALLWAY(tf->p[0]))));
  for (i = 1; i < tf->np; i++)     /* .. and the rest */
  {
    if (tf->p[i].size == 1)
      e = numConstant(1);
    else
    {
      if (tf->p[i].size == SIZE_DYNAMIC)
        e = numConstant(-1);                  /* dynamic size */
      else
      {
        e = ast_expr_copy(tf->p[i].size_expr);
        tf_sizeexpr_make(&e);
      }
    }
    str_truncate(typstr); str_printf(typstr, "%d", tf->p[i].type);
    expr = CommaList(expr, CommaList(
                             CommaList(e, Constant(strdup(str_string(typstr)))),
                             Constant(strdup(CALLWAY(tf->p[i])))));
  }

  for (i = 0; i < tf->np; i++)  /* finally the actual arguments */
    expr = CommaList(expr, (tf->p[i].size == 1 && tf->p[i].intent == OX_OCIN) ?
               UnaryOperator(UOP_addr, Identifier(Symbol(inparam(i)))) :
               ast_expr_copy(ca[i]));
  
  return (expr);
}


static
astexpr *tabulate_callargs(astexpr e, astexpr *ca)
{
  if (e == NULL)
    return (NULL);
  if (e->type == COMMALIST)
    return ( tabulate_callargs(e->right, tabulate_callargs(e->left, ca)) );
  *ca = ast_expr_copy(e);
  return (ca+1);
}


static
void free_callargs_table(int n, astexpr *ca)
{
  for (--n; n >= 0; n--)
    ast_expr_free(ca[n]);
  free(ca);
}


static
int count_callargs(astexpr e)
{
  if (e == NULL)
    return (0);
  else
    if (e->type != COMMALIST)
      return (1);
    else
      return ( count_callargs(e->left) + count_callargs(e->right) );
}


static
aststmt ox_func2task_call(astexpr *fc, taskfunc tf, int detached, int tied,
                          int atall, astexpr atnode, astexpr atworker)
{
  int        i, n = count_callargs((*fc)->right);
  astexpr    *ca;
  aststmt    st = NULL;           /* temportary declarations */
  static str tstr = NULL;
  
  if (tstr == NULL) tstr = Strnew(); else str_truncate(tstr);
  
  if (atall && atnode != NULL) 
    exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                  "only one atnode() clause allowed in a task construct\n",
                  (*fc)->file->name, (*fc)->l);
  if (n != tf->np)
    exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                  "wrong # arguments in task function call\n",
                  (*fc)->file->name, (*fc)->l);
  ca = (astexpr *) malloc(n*sizeof(astexpr));
  tabulate_callargs((*fc)->right, ca);

  (*fc)->right = NULL;
  ast_expr_free(*fc);
  
  /* check if we have any scalar IN params */
  for (i = 0; i < tf->np; i++)
    if (tf->p[i].size == 1)     /* ok, we have one */
    {
      st = verbit(" ");        /* dummy */
      for (; i < tf->np; i++)
        if (tf->p[i].size == 1)
          st = BlockList(
                 st, 
                 Declaration( ast_spec_copy_nosc(tf->p[i].decl->spec),
                              InitDecl(
                                xc_decl_rename(
                                  ast_decl_copy(tf->p[i].decl->decl),
                                  Symbol(inparam(i))
                                ),
                                ast_expr_copy(ca[i])
                              )
                            )
               );
      break;
    };

  str_printf(tstr, "%d", tf->np);
  *fc = FunctionCall(
          Identifier(Symbol("torc_create_ox")),
          CommaList(
            CommaList(
              CommaList(
                atall ?  numConstant(-2) :             /* 1 (atnode(all)) */
                atnode ? ast_expr_copy(atnode) :       /* 1 (atnode(expr)) */
                         numConstant(-1),              /* 1 (no placement) */
                atworker ? ast_expr_copy(atworker) :   /* 2 (atworker(expr)) */
                           numConstant(-1)             /* 2 (no placement) */
              ),
              CommaList(
                numConstant(detached ? 1 : 0),         /* 3 (detached flag) */
                numConstant(tied == -1 ? -1 :
                            tied == 0  ? 0 : 1)        /* 4 (tied flag) */
              )
            ),
            CommaList(
              CommaList(
                Identifier(tf->tfid),                  /* 5 (function) */
                tf->has_callback ?
                  Identifier(callback_func(tf->tfid)) :   /* 6 (callback) */
                  CastedExpr(                          /* 6 (NULL callback) */
                      Casttypename(
                        Declspec(SPEC_void, 0),
                        AbstractDeclarator(Pointer(),NULL)
                      ),
                      numConstant(0)
                  )
              ),
              tf->np == 0 ?
                numConstant(0) :                       /* 7 (0 args) */
                CommaList(
                  Constant(strdup(str_string(tstr))),  /* 7,<args> */
                  callargs_tree(ca, tf)
                )
            )
          )
        );

  free_callargs_table(n, ca);
  
  return (st);
}


/* This also checks if the clause is unique */
oxclause ox_has_clause(oxclause t, enum oxclausetype type)
{
  oxclause e = NULL;
  
  if (t == NULL) return (NULL);

  if (t->type == OX_OCLIST)
  {
    if (t->u.list.next != NULL)   /* depth-first to check uniqueness */
      e = ox_has_clause(t->u.list.next, type);
    assert((t = t->u.list.elem) != NULL);
  }
  if (t->type == type)
  {
    if (e != NULL)
      exit_error(1, "(%s, line %d) ompi-extensions error:\n\t"
                    "multiple %s() clauses in task directive.\n",
                    t->file->name, t->l, oxclausenames[type]);
    e = t;
  }
  return (e);
}


static
astexpr ox_atnode(oxdir d)
{
  oxclause e;

  return ( (e = ox_has_clause(d->clauses, OX_OCATNODE)) ?  e->u.expr : NULL );
}


static
astexpr ox_atworker(oxdir d)
{
  oxclause e;

  return ( (e = ox_has_clause(d->clauses, OX_OCATWORKER)) ?  e->u.expr : NULL );
}


void ox_xform_task(aststmt *t)
{
  aststmt  new, tempdecls;
  taskfunc tf;

  oxcon_xform(&((*t)->u.ox));
  assert((*t)->u.ox->body->type == EXPRESSION &&
         (*t)->u.ox->body->u.expr->type == FUNCCALL);
  
  if ((tf = get_taskfunc((*t)->u.ox->body->u.expr->left->u.sym)) == NULL)
    exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                  "unknown task function %s (TASKDEF missing?).\n",
                  (*t)->u.ox->body->u.expr->left->file->name,
                  (*t)->u.ox->body->u.expr->left->l,
                  (*t)->u.ox->body->u.expr->left->u.sym->name);

  ox_validate_clauses((*t)->u.ox->directive->type, 
                      (*t)->u.ox->directive->clauses);

  /* work around the IN scalar parameters */
  tempdecls = ox_func2task_call( &((*t)->u.ox->body->u.expr), tf,
       ox_has_clause((*t)->u.ox->directive->clauses, OX_OCDETACHED) ? 1 : 0,
       ox_has_clause((*t)->u.ox->directive->clauses, OX_OCTIED) ? 1 :
         ox_has_clause((*t)->u.ox->directive->clauses, OX_OCUNTIED) ? 0 : -1,
       ox_has_clause((*t)->u.ox->directive->clauses, OX_OCATALL) ? 1 : 0,
       ox_atnode((*t)->u.ox->directive), ox_atworker((*t)->u.ox->directive));
  new = BlockList(oxdir_commented((*t)->u.ox->directive),
                  (tempdecls) ?
                     Compound( BlockList(tempdecls, (*t)->u.ox->body) ) :
                     (*t)->u.ox->body);
  new->parent = (*t)->parent;   /* parantize correctly */
  free(*t);
  *t = new;
}


/* For debugging only */
#if 1
#include "ast_show.h"
static
void showtaskfunc(taskfunc tf)
{
  int     i;
  tfparam *p;

  printf("/* TaskFunc:\n");
  printf(" name:     %s\n # params: %d\n", tf->tfid->name, tf->np);
  for (i = 0; i < tf->np; i++)
  {
    p = &(tf->p[i]);
    printf("   param %d (%s)\n     intent = %s, type = %d, size = %d",
           i, p->pid->name, p->intent == OX_OCIN ? "IN" :
                            p->intent == OX_OCOUT ? "OUT" :
                            p->intent == OX_OCINOUT ? "INOUT" : "<intent?>",
                            p->type, p->size);
    if (p->size != SIZE_SCALAR) 
      { printf(", expression: "); ast_expr_show(p->size_expr); }
    printf("\n");
  }
  printf("*/\n");
}
#endif


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     CLAUSES VALIDATION                                        *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


void ox_validate_clauses(enum oxdircontype dirtype, oxclause t)
{
  int hastied = 0, hasstride = 0, hasstart = 0, hasscope = 0,
      hasatnode = 0, hasatall = 0, hasatworker = 0, hasdetached = 0;

  if (t == NULL) return;
  if (t->type == OX_OCLIST)
  {
    if (t->u.list.next != NULL)
      ox_validate_clauses(dirtype, t->u.list.next);
    t = t->u.list.elem;
    assert(t != NULL);
  }
  
  switch (t->type)
  {
    /* Data clauses */
    
    case OX_OCREDUCE:
    case OX_OCIN:
    case OX_OCOUT:
    case OX_OCINOUT:
      if (dirtype != OX_DCTASKDEF)
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                     "intent clauses aren't allowed in `%s' directives\n",
                     t->file->name, t->l, oxdirnames[dirtype]);
      break;
    case OX_OCATNODE:
    case OX_OCATALL:
      if (hasatnode)
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                     "multiple atnode clauses are not allowed in a directive\n",
                     t->file->name, t->l);
      hasatnode = 1;
      if (t->type == OX_OCATALL) 
        hasatall = 1;
      if (hasatall && hasatworker)
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
           "atnode(*) and atworker clauses not allowed in the same directive\n",
                     t->file->name, t->l, oxdirnames[dirtype]);
      if (dirtype != OX_DCTASK)
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                     "atnode clauses aren't allowed in `%s' directives\n",
                     t->file->name, t->l, oxdirnames[dirtype]);
      break;
    case OX_OCATWORKER:
      if (hasatnode)
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                   "multiple atworker clauses are not allowed in a directive\n",
                     t->file->name, t->l);
      hasatworker = 1;
      if (hasatall && hasatworker)
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
           "atnode(*) and atworker clauses not allowed in the same directive\n",
                     t->file->name, t->l, oxdirnames[dirtype]);
      if (dirtype != OX_DCTASK)
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                     "atworker clauses aren't allowed in `%s' directives\n",
                     t->file->name, t->l, oxdirnames[dirtype]);
      break;
    case OX_OCDETACHED:
      if (hasdetached)
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                    "multiple detached clauses aren't allowed in a directive\n",
                    t->file->name, t->l);
      hasdetached = 1;
      if (dirtype != OX_DCTASK)
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                     "detached clauses aren't allowed in `%s' directives\n",
                     t->file->name, t->l, oxdirnames[dirtype]);
      break;
    case OX_OCTIED:
    case OX_OCUNTIED:
      if (hastied)
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                 "multiple tied/untied clauses aren't allowed in a directive\n",
                     t->file->name, t->l);
      hastied = 1;
      if (dirtype != OX_DCTASK && dirtype != OX_DCTASKSCHEDULE)
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                     "tied/untied clauses aren't allowed in `%s' directives\n",
                     t->file->name, t->l, oxdirnames[dirtype]);
      break;
    case OX_OCSTART:
      if (hasstart)
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                    "multiple start clauses are not allowed in a directive\n",
                    t->file->name, t->l);
      hasstart = 1;
      if (dirtype != OX_DCTASKSCHEDULE)
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                     "start clauses aren't allowed in `%s' directives\n",
                     t->file->name, t->l, oxdirnames[dirtype]);
      break;
    case OX_OCSTRIDE:
      if (hasstride)
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                    "multiple stride clauses are not allowed in a directive\n",
                    t->file->name, t->l);
      hasstride = 1;
      if (dirtype != OX_DCTASKSCHEDULE)
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                     "stride clauses aren't allowed in `%s' directives\n",
                     t->file->name, t->l, oxdirnames[dirtype]);
      break;
    case OX_OCSCOPE:
      if (hasscope)
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                    "multiple scope clauses are not allowed in a directive\n",
                    t->file->name, t->l);
      hasscope = 1;
      if (dirtype != OX_DCTASKSCHEDULE)
        exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                     "scope clauses aren't allowed in `%s' directives\n",
                     t->file->name, t->l, oxdirnames[dirtype]);
      break;

    default:
      exit_error(1, "(%s, line %d) OMPi-X error:\n\t"
                    "unknown clause type (%d) in `%s' directive\n",
                     t->file->name, t->l, t->type, ompdirnames[dirtype]);
      break;
  }
}
