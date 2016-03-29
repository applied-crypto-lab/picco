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

/* x_clauses.c -- everything related to openmp clauses & their generated code */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ast_free.h"
#include "ast_vars.h"
#include "ast_xform.h"
#include "ast_copy.h"
#include "ast_types.h"
#include "x_clauses.h"
#include "picco.h"


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     REDUCTION-RELATED STUFF                                   *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


static int  red_num = -1;
static char _rl[128];
#define redlock()  _rl

static
void new_reduction() 
{
  red_num++;
  sprintf(_rl, "_redlock%d", red_num);
  
  /* Add a global definition, too, for the lock (avoid omp_lock_t) */
  newglobalvar( Declaration(Speclist_right(
                              StClassSpec(SPEC_static),
                              Declspec(SPEC_void, 0)
                            ),
                            Declarator(
                              Pointer(),
                              IdentifierDecl( Symbol(_rl) )
                            )) );

}


/* Correct initializer depending on the reduction operator */
astexpr xc_reduction_initializer(int op, symbol var)
{
  if (op == OC_min || op == OC_max)
  {
    stentry e = symtab_get(stab, var, IDNAME);
    
    switch ( speclist_basetype(e->spec) )
    {
      case UBOOL: 
	return ( numConstant( op == OC_min ? 1 : 0 ) );
      case CHAR: 
	if ( speclist_sign(e->spec) == UNSIGNED)
	{
	  if (op == OC_max)
	    return ( numConstant(0) );
	  needLimits = 1;
	  return ( Identifier( Symbol("UCHAR_MAX") ) );
	}
	else
	{
	  needLimits = 1;
	  return ( Identifier( Symbol( op == OC_max ? "SCHAR_MIN" : 
	                                              "SCHAR_MAX" ) ) );
	}
      case INT:
	if (op == OC_max && speclist_sign(e->spec) == UNSIGNED)
	  return ( numConstant(0) );
	else
	  needLimits = 1;
	switch ( speclist_size(e->spec) )
	{
	  case SHORT: 
	    return ( Identifier( Symbol( op == OC_max ? 
	                                   "SHRT_MIN" :
	                                   ( speclist_sign(e->spec) == SIGNED ? 
	                                       "SHRT_MAX" : "USHRT_MAX" )
	                               ) ) );
	  case LONG:
	    return ( Identifier( Symbol( op == OC_max ? 
            	                           "LONG_MIN" :
	                                   ( speclist_sign(e->spec) == SIGNED ?
	                                     "LONG_MAX" : "ULONG_MAX" )
	                               ) ) );
	  case LONGLONG:
	    return ( Identifier( Symbol( op == OC_max ? 
            	                           "LLONG_MIN" :
	                                   ( speclist_sign(e->spec) == SIGNED ?
	                                     "LLONG_MAX" : "ULLONG_MAX" )
	                               ) ) );
	  default:
	    return ( Identifier( Symbol( op == OC_max ? 
           	                           "INT_MIN" :
	                                   ( speclist_sign(e->spec) == SIGNED ?
	                                     "INT_MAX" : "UINT_MAX" )
	                               ) ) );
	}
      case FLOAT:
	needFloat = 1;
	return ( Identifier( Symbol( op==OC_max ? "-FLT_MAX" : "FLT_MAX" ) ) );
      case DOUBLE:
	needFloat = 1;
	return ( Identifier( Symbol( speclist_size(e->spec) == LONG ?
	 	                     (op == OC_max ? "-LDBL_MAX" : "LDBL_MAX") :
	                             (op == OC_max ? "-DBL_MAX" : "DBL_MAX") 
	                           ) ) );
      default:
        exit_error(1, "[xc_reduction_initializer]: !!BUG!! bad type ?!\n");
    }
  }
  if (op == OC_times || op == OC_land)
    return ( numConstant(1) );
  if (op == OC_band)
    return ( UnaryOperator(UOP_bnot, numConstant(0)) );
  return ( numConstant(0) );
}


/* Produces a statement that declares and initializes 1 reduction var
 * (plus another one which is needed)
 */
aststmt xc_reduction_declaration(symbol var, int redop)
{
  char    flvar[256];
  astdecl decl, id;
  stentry e = symtab_get(stab, var, IDNAME);

  if (e->isarray)
    exit_error(1, "openmp error: reduction variable `%s' is non-scalar.\n",
                   var->name);
  
  /* Declare and intialize a temp var _red_<name> */
  snprintf(flvar, 255, "_red_%s", var->name);
  decl = ast_decl_copy(e->decl);
  id = IdentifierDecl(Symbol(flvar));
  *(decl_getidentifier(decl)) = *id;
  free(id);
  
  return Declaration( /* <spec> *red_var = &var, var = <initializer>; */
           ast_spec_copy_nosc(e->spec),
           DeclList(
             InitDecl(
               xc_decl_topointer(decl),
               UnaryOperator(UOP_addr, Identifier(var))
             ),
             InitDecl(
               ast_decl_copy(e->decl),
               xc_reduction_initializer(redop, var)
             )
           )
         );
}


/* Generates code for reductions of a list of variables.
 * Works for all except parallel, which emits its own code.
 *   *(_red_var) op= var   or
 *   *(_red_var) = *(_red_var) op var   (for && and ||)
 *   if (*(_red_var) >(<) var) *(_red_var) = var (for min/max)
 */
static 
aststmt reduction_code_from_varlist(astdecl d, int op)
{
  char    flvar[256];
  aststmt list = NULL, st = NULL;
  
  if (d->type == DLIST && d->subtype == DECL_idlist)
  {
    list = reduction_code_from_varlist(d->u.next, op);
    d = d->decl;
    assert(d != NULL);
  }
  if (d->type != DIDENT)
    exit_error(1, "[reduction_code_from_varlist]: !!BUG!! not a DIDENT ?!\n");

  snprintf(flvar, 255, "_red_%s", d->u.id->name);
  if (op == OC_min || op == OC_max)
    st = If(
           BinaryOperator(
             (op == OC_min) ?  BOP_gt : BOP_lt,
             UnaryOperator( UOP_star, Identifier(Symbol(flvar)) ),
	     Identifier(d->u.id)
           ),
           Expression(
             Assignment(
               UnaryOperator( UOP_star, Identifier(Symbol(flvar)) ),
               ASS_eq,           
               Identifier(d->u.id)
             )
           ),
	   NULL
	 );
  else
    st = Expression(
           Assignment(
             UnaryOperator( UOP_star, Identifier(Symbol(flvar)) ),
             
             (op == OC_plus)  ? ASS_add :
             (op == OC_minus) ? ASS_add :  /* indeed! */
             (op == OC_times) ? ASS_mul :
             (op == OC_band)  ? ASS_and :
             (op == OC_bor)   ? ASS_or  :
             (op == OC_xor)   ? ASS_xor : ASS_eq,
             
             (op != OC_land && op != OC_lor) ?
               Identifier(d->u.id) :
               BinaryOperator(
                 (op == OC_land) ? BOP_land : BOP_lor,
                 UnaryOperator( UOP_star, Identifier(Symbol(flvar)) ),
                 Identifier(d->u.id)
               )
           )
         );
  return ( (list != NULL) ? BlockList(list, st) : st );
}


static
aststmt reduction_code_from_clauses(ompclause t)
{
  aststmt list = NULL, st = NULL;
  
  if (t->type == OCLIST)
  {
    if (t->u.list.next != NULL)
      list = reduction_code_from_clauses(t->u.list.next);
    t = t->u.list.elem;
    assert(t != NULL);
  }
  if (t->type == OCREDUCTION)
  {
    new_reduction();   /* Track reductions; add global lock */
    st = BlockList(
           BlockList(
             Expression(
               FunctionCall(
                 Identifier(Symbol("ort_reduction_begin")),
                 UnaryOperator(UOP_addr, Identifier(Symbol(redlock())))
               )
             ),
             reduction_code_from_varlist(t->u.varlist, t->subtype)
           ),
           Expression(
             FunctionCall(
               Identifier(Symbol("ort_reduction_end")),
               UnaryOperator(UOP_addr, Identifier(Symbol(redlock())))
             )
           )
         );
    list = ((list != NULL) ? BlockList(list, st) : st);
  }
  return (list);
}


/* Statements for reductions */
aststmt xc_ompdir_reduction_code(ompdir t)
{
  return ( t->clauses ? reduction_code_from_clauses(t->clauses)
                      : NULL );
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     FIRSTPRIVATE RELATED STUFF                                *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* This produces a statement declaring a firstprivate variable (plus
 * another one which is need). The declaration contains an
 * initializer, only when the variable is scalar; otherwise it
 * contains a plain declaration and initialization should be done
 * through xc_ompdir_array_initializers()
 */
aststmt xc_firstprivate_declaration(symbol var)
{
  char    flvar[256];
  astdecl decl, id;
  stentry e = symtab_get(stab, var, IDNAME);

  /* Declare and intialize a temp var _fip_<name> */
  snprintf(flvar, 255, "_fip_%s", var->name);
  decl = ast_decl_copy(e->decl);
  id = IdentifierDecl(Symbol(flvar));
  *(decl_getidentifier(decl)) = *id;
  free(id);

  if (e->isarray)
    return Declaration( /* <spec> *fip_var = &var, var; */
             ast_spec_copy_nosc(e->spec),
             DeclList(
               InitDecl(
                 xc_decl_topointer(decl),
                 UnaryOperator(UOP_addr, Identifier(var))
               ),
               ast_decl_copy(e->decl)
             )
           );
  else
    return Declaration(  /* <spec> fip_var = var, var = fip_var; */
             ast_spec_copy_nosc(e->spec),
             DeclList(
               InitDecl(
                 decl,
                 Identifier(var)
               ),
               InitDecl(
                 ast_decl_copy(e->decl),
                 Identifier(Symbol(flvar))
               )
             )
           );
}


/* Produces a memory copy statement, for initializing firstprivate
 * vars which are non-scalar:
 *     memcpy( (void *) var, (void *) &(init), sizeof(var) );
 */
aststmt xc_array_initializer(symbol var, astexpr init)
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
              Identifier(var) 
            ), 
            CastedExpr( 
              Casttypename(
                Declspec(SPEC_void, 0),
                AbstractDeclarator(Pointer(),NULL)
              ), 
              init /* UnaryOperator(UOP_addr, UnaryOperator(UOP_paren, init)) */
            ) 
          ), 
          Sizeof(Identifier(var)) 
        ) 
      ) 
    );
}


/* Take a varlist and generate correct initialization statements
 * for lastprivate vars that are non-scalar.
 * It works for all OpenMP constructs except *parallel* which produces
 * its own declarations.
 */
static
aststmt array_initializations_from_varlist(astdecl d,
                                           enum clausetype type, ompdir ompd)
{
  aststmt list = NULL, st = NULL;
  
  if (d->type == DLIST && d->subtype == DECL_idlist)
  {
    list = array_initializations_from_varlist(d->u.next, type, ompd);
    d = d->decl;
  }
  assert(d->type == DIDENT);
  if (type == OCFIRSTPRIVATE)
  {
    char flvar[256];
    if ((ompd->type == DCFOR || ompd->type == DCSECTIONS || /* special case */
         ompd->type == DCFOR_P) && 
        symtab_get(stab, d->u.id, IDNAME)->isarray &&  /* first&last private */
        xc_isvar_in_dataclause(d->u.id, ompd, OCLASTPRIVATE))       /* array */
      snprintf(flvar, 255, "_lap_%s", d->u.id->name);
    else
      snprintf(flvar, 255, "_fip_%s", d->u.id->name);
    if (symtab_get(stab, d->u.id, IDNAME)->isarray)
      st = xc_array_initializer(d->u.id,   /* *flvar */
                          UnaryOperator(UOP_star, Identifier(Symbol(flvar))));
  }
  if (st)
    list = ((list != NULL) ? BlockList(list, st) : st);
  return (list);
}


static
aststmt array_initializations_from_clauses(ompclause t, ompdir d)
{
  aststmt list = NULL, st = NULL;
  
  if (t->type == OCLIST)
  {
    if (t->u.list.next != NULL)
      list = array_initializations_from_clauses(t->u.list.next, d);
    t = t->u.list.elem;
    assert(t != NULL);
  }
  
  if (t->type == OCFIRSTPRIVATE)
    (st = array_initializations_from_varlist(t->u.varlist, t->type, d))
      && ( list = ((list != NULL) ? BlockList(list, st) : st) );
  return (list);
}


/* Memory copying statements for fip vars (firstprivate) */
aststmt xc_ompdir_fiparray_initializers(ompdir t)
{
  return ( t->clauses ? array_initializations_from_clauses(t->clauses, t)
                      : NULL );
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     LASTPRIVATE RELATED STUFF                                 *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* This produces a statement declaring a lastprivate variable (plus
 * another one which is need), exactly as in firstprivate; the only
 * difference is that there is no initializer.
 */
aststmt xc_lastprivate_declaration(symbol var)
{
  char    flvar[256];
  astdecl decl, id;
  stentry e = symtab_get(stab, var, IDNAME);

  /* Declare and intialize a temp var _lap_<name> */
  snprintf(flvar, 255, "_lap_%s", var->name);
  decl = ast_decl_copy(e->decl);
  id = IdentifierDecl(Symbol(flvar));
  *(decl_getidentifier(decl)) = *id;
  free(id);

  return Declaration( /* <spec> *lap_var = &var, var; */
           ast_spec_copy_nosc(e->spec),
           DeclList(
             InitDecl(
               xc_decl_topointer(decl),
               UnaryOperator(UOP_addr, Identifier(var))
             ),
             ast_decl_copy(e->decl)
           )
         );
}


/* This produces a statement declaring a lastprivate variable (plus
 * another one which is need), exactly as in firstprivate; the only
 * difference is that there is no initializer.
 */
aststmt xc_firstlastprivate_declaration(symbol var)
{
  char    flvar[256];
  astdecl decl, id;
  stentry e = symtab_get(stab, var, IDNAME);

  /* Declare and intialize a temp var _flp_<name> */
  /* We call it _lap_<name> because xc_lastprivate_assignements()
   * has no way of telling whether the var is also firstprivate
   */
  snprintf(flvar, 255, "_lap_%s", var->name);
  decl = ast_decl_copy(e->decl);
  id = IdentifierDecl(Symbol(flvar));
  *(decl_getidentifier(decl)) = *id;
  free(id);
  
  return Declaration( /* <spec> *flp_var = &var, var [ = *flp_var ]; */
           ast_spec_copy_nosc(e->spec),
           DeclList(
             InitDecl(
               xc_decl_topointer(decl),
               UnaryOperator(UOP_addr, Identifier(var))
             ),
             (e->isarray) ? 
               ast_decl_copy(e->decl) :
               InitDecl(
                 ast_decl_copy(e->decl),
                 UnaryOperator(UOP_star, Identifier(Symbol(flvar)))
               )
           )
         );
}


/* Produces a memory copy statement, for initializing firstprivate
 * vars which are non-scalar:
 *     memcpy( (void *) &(init), (void *) var, sizeof(var) );
 */
static
aststmt xc_array_assigner(symbol var, astexpr init)
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
              UnaryOperator(UOP_addr, UnaryOperator(UOP_paren, init)) 
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


/* Take a lastprivate varlist and generate correct assignement statements */
static
aststmt lastprivate_assignments_from_varlist(astdecl d)
{
  char    flvar[256];
  aststmt list = NULL, st = NULL;
  
  if (d->type == DLIST && d->subtype == DECL_idlist)
  {
    list = lastprivate_assignments_from_varlist(d->u.next);
    d = d->decl;
  }
  assert(d->type == DIDENT);
  snprintf(flvar, 255, "_lap_%s", d->u.id->name);
  if (symtab_get(stab, d->u.id, IDNAME)->isarray)
    st = xc_array_assigner(d->u.id,   /* *flvar */
                           UnaryOperator(UOP_star, Identifier(Symbol(flvar))));
  else  /* Scalar */
    st = Expression(
           Assignment(
             UnaryOperator(UOP_star, Identifier(Symbol(flvar))),
             ASS_eq,
             Identifier(d->u.id)
           )
         );
  list = ((list != NULL) ? BlockList(list, st) : st);
  return (list);
}


static
aststmt lastprivate_assignments_from_clauses(ompclause t)
{
  aststmt list = NULL, st = NULL;
  
  if (t->type == OCLIST)
  {
    if (t->u.list.next != NULL)
      list = lastprivate_assignments_from_clauses(t->u.list.next);
    t = t->u.list.elem;
    assert(t != NULL);
  }
  if (t->type == OCLASTPRIVATE)
    (st = lastprivate_assignments_from_varlist(t->u.varlist))
      && ( list = ((list != NULL) ? BlockList(list, st) : st) );
  return (list);
}


/* Assignement / memory copying statements for lap vars (lastprivate) */
aststmt xc_ompdir_lastprivate_assignments(ompdir t)
{
  return ( t->clauses ? lastprivate_assignments_from_clauses(t->clauses)
                      : NULL );
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     DECLARATIONS FROM THE DATA CLAUSE VARS                    *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Take a varlist and generate a correct declaration for each of the
 * vars. It takes care of private/firstprivate/lastprivate/etc types
 * of varlists.
 *
 * It works for all OpenMP constructs except *parallel* which produces
 * its own declarations.
 */
static
aststmt declarations_from_varlist(astdecl d, enum clausetype type, int operator)
{
  aststmt list = NULL, st = NULL;
  
  if (d->type == DLIST && d->subtype == DECL_idlist)
  {
    list = declarations_from_varlist(d->u.next, type, operator);
    d = d->decl;
  }
  assert(d->type == DIDENT);
  switch (type)
  {
    case OCFIRSTPRIVATE:
      st = xc_firstprivate_declaration(d->u.id);
      break;
    case OCREDUCTION:
      st = xc_reduction_declaration(d->u.id, operator);
      break;
    case OCLASTPRIVATE:
      st = xc_lastprivate_declaration(d->u.id);
      break;
    case OCPRIVATE:
      st = xform_clone_declaration(d->u.id, NULL);
      break;
    case OCCOPYIN:        /* This is handled entirelu in x_parallel.c */
    case OCSHARED:        /* These do not produce any declarations */
    case OCCOPYPRIVATE:   /* This is handled entirely in x_single.c */
      break;
  }
  if (st) list = ((list != NULL) ? BlockList(list, st) : st);
  return (list);
}


/* It is assumed that a validity check for clauses and variables
 * appearing in them has already taken place.
 * ompdir is only needed for error messages & validity checks
 */
static
aststmt declarations_from_clauses(enum dircontype dirtype, ompclause t)
{
  aststmt list = NULL, st = NULL;
  
  if (t->type == OCLIST)
  {
    if (t->u.list.next != NULL)
      list = declarations_from_clauses(dirtype, t->u.list.next);
    t = t->u.list.elem;
    assert(t != NULL);
  }
  switch (t->type)
  {
    case OCREDUCTION:    /* Only data clauses matter */
    case OCSHARED:
    case OCCOPYIN:
    case OCPRIVATE:
    case OCFIRSTPRIVATE:
    case OCCOPYPRIVATE:
    case OCLASTPRIVATE:
      ( st = declarations_from_varlist(t->u.varlist, t->type, t->subtype) )
        && ( list = ((list != NULL) ? BlockList(list, st) : st) );
      break;
  }
  return (list);
}


aststmt xc_ompdir_declarations(ompdir t)
{
  return ( t->clauses ? declarations_from_clauses(t->type, t->clauses) : NULL );
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
 * and the "value" field encodes the operator, in case of reduction vars.
 */
static symtab    dc_vars = NULL;    /* Store the list here */
static ompclause dc_vars_clause;    /* Only needed for error messages */


static
void checkNstore_varlist_vars(astdecl d, int clausetype, int opid)
{
  stentry e;
  
  if (d->type == DLIST && d->subtype == DECL_idlist)
  {
    checkNstore_varlist_vars(d->u.next, clausetype, opid);
    d = d->decl;
  }
  if (d->type != DIDENT)
    exit_error(1, "[checkNstore_varlist_vars]: !!BUG!! not a DIDENT ?!\n");
  if ((e = symtab_get(dc_vars, d->u.id, IDNAME)) != NULL)
  {
    if ((clausetype == OCFIRSTPRIVATE && e->ival == OCLASTPRIVATE) ||
        (clausetype == OCLASTPRIVATE && e->ival == OCFIRSTPRIVATE))
    {
      e->ival = OCFIRSTLASTPRIVATE;       /* Change it -- special case! */
      return;                             /* Don't put it in again */
    }
    else
      exit_error(1, "(%s, line %d) openmp error:\n\t"
                    "variable `%s' appears more than once in "
                    "the directive's clause(s)\n",
                    dc_vars_clause->file->name, dc_vars_clause->l,
                    d->u.id->name);
  }
  switch (clausetype)
  {
    case OCCOPYIN:         /* Copyin is only for threadprivate vars */
      if (!symtab_get(stab, d->u.id, IDNAME)->isthrpriv)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                  "copyin clause variable '%s' is not threadprivate.\n",
                  dc_vars_clause->file->name, dc_vars_clause->l, d->u.id->name);
      break;
    case OCCOPYPRIVATE:   /* Nothing here */
      break;     
    default:              /* All others */
      if (symtab_get(stab, d->u.id, IDNAME)->isthrpriv)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                  "threadprivate variable '%s' cannot appear in a %s clause.\n",
                  dc_vars_clause->file->name, dc_vars_clause->l,
                  d->u.id->name, clausenames[clausetype]);
      break;
  }
  /* Put the opid in the "value" field */
  e = symtab_put(dc_vars, d->u.id, IDNAME);
  e->vval = opid;
  e->ival = clausetype;
}


static
void checkNstore_dcclause_vars(ompclause t)
{
  if (t->type == OCLIST)
  {
    if (t->u.list.next != NULL) 
      checkNstore_dcclause_vars(t->u.list.next);
    assert((t = t->u.list.elem) != NULL);
  }
  dc_vars_clause = t;
  switch (t->type)
  {
    case OCPRIVATE:
    case OCFIRSTPRIVATE:
    case OCLASTPRIVATE:
    case OCCOPYPRIVATE:
    case OCCOPYIN:
    case OCREDUCTION:
    case OCSHARED:
      if (t->u.varlist)   /* t->subtype is the opid in case of reduction */
        checkNstore_varlist_vars(t->u.varlist, t->type, t->subtype);
      break;
  }
}


/* Checks for duplicates AND keeps the list of vars; returns the table */
symtab xc_validate_store_dataclause_vars(ompdir d)
{
  if (dc_vars == NULL)
    dc_vars = Symtab();
  else
    symtab_drain(dc_vars);
  if (d->clauses)
    checkNstore_dcclause_vars(d->clauses);
  return (dc_vars);
}


/* Only checks for duplicates */
void xc_validate_only_dataclause_vars(ompdir d)
{
  xc_validate_store_dataclause_vars(d);
  symtab_drain(dc_vars);                   /* Not needed any more */
}


/*
 * The following 3 functions are used to search whether a particular
 * variable appears in any (or a specific) data clause.
 */


static
int findvar_varlist(symbol var, astdecl d)
{
  if (d->type == DLIST)
  {
    if ( findvar_varlist(var, d->u.next) )
      return (1);
    d = d->decl;
  }
  if (d->type != DIDENT)
    exit_error(1, "[findvar_varlist]: !!BUG!! not a DIDENT ?!\n");
  return (d->u.id == var);
}


static
int findvar_dataclause(symbol var, ompclause t, enum clausetype c)
{
  if (t->type == OCLIST)
  {
    if (t->u.list.next != NULL) 
      if ( findvar_dataclause(var, t->u.list.next, c) )
	return (1);
    assert((t = t->u.list.elem) != NULL);
  }
  if (t->type == c && t->u.varlist)
    return ( findvar_varlist(var, t->u.varlist) );
  return (0);
}


static
enum clausetype findvar_any_dataclause(symbol var, ompclause t)
{
  if (t->type == OCLIST)
  {
    if (t->u.list.next != NULL) 
      if ( findvar_any_dataclause(var, t->u.list.next) )
	return (OCPRIVATE);  /* anything would do */
    assert((t = t->u.list.elem) != NULL);
  }
  switch (t->type)
  {
    case OCPRIVATE:
    case OCFIRSTPRIVATE:
    case OCLASTPRIVATE:
    case OCCOPYPRIVATE:
    case OCCOPYIN:
    case OCREDUCTION:
    case OCSHARED:
      if (t->u.varlist && findvar_varlist(var, t->u.varlist))
        return (t->type); 
      break;
  }
  return (OCNOCLAUSE);
}


/* This one searches to see whether var appears in a type c
 * data clauses of d; returns 1 if so, 0 if not.
 */
int xc_isvar_in_dataclause(symbol var, ompdir d, enum clausetype c)
{
  return ( findvar_dataclause(var, d->clauses, c) != OCNOCLAUSE );
}


/* Find whether var appears in any of the data clauses of d.
 * If found, it returns the clause type, otherwise 0.
 */
enum clausetype xc_dataclause_of_var(symbol var, ompdir d)
{
  return ( findvar_any_dataclause(var, d->clauses) );
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     DECLARATIONS FROM A SET OF VARS                           *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* We have 2 ways of declaring the variables in the data clauses.
 *   (1) by scanning all the data clauses and declaring from each clause
 *   (2) by collecting all data clause variables in a set, along with
 *       the dataclause type of each one and declaring them from there.
 * Both have some advantages. The second one does not need recursion,
 * plus it makes it trivial to discover vars that are both firstprivate
 * and lastprivate.
 */
 
 
/* Produces a list of declarations for all variables in the dc_vars table 
 */
aststmt xc_stored_vars_declarations(int *has_last, int *has_both, int *has_red)
{
  stentry e;
  aststmt st = NULL, list = NULL;
  
  *has_last = *has_both = *has_red = 0;
  for (e = dc_vars->top; e; e = e->stacknext)
  {
    switch (e->ival)
    {
      case OCFIRSTPRIVATE:
        st = xc_firstprivate_declaration(e->key);
        break;
      case OCREDUCTION:
        *has_red = 1;
        st = xc_reduction_declaration(e->key, e->vval);
        break;
      case OCLASTPRIVATE:
        *has_last = 1;
        st = xc_lastprivate_declaration(e->key);
        break;
      case OCFIRSTLASTPRIVATE:
        *has_both = 1;
        *has_last = 1;
        st = xc_firstlastprivate_declaration(e->key);
        break;
      case OCPRIVATE:
        st = xform_clone_declaration(e->key, NULL);
        break;
      case OCCOPYIN:        /* Taken care in x_parallel.c */
      case OCSHARED:        /* These do not produce any declarations */
      case OCCOPYPRIVATE:   /* This is handled entirelu in x_single.c */
        break;
    }
    
    if (st)
      list = ((list == NULL) ? st : BlockList(list, st));
  }
  return (list);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     FOR COMBINED PARALLEL-WORKSHARE CONSTRUCTS                *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Take the clauses of a combined parallel-workshare statement and
 * split them; some are given to parallel and some to the workshare.
 * Parallel actually takes all the clauses it can take. A problem
 * exists if there is a default(none) and a variable listed only
 * within a lastprivate clause since that clause is given to the for
 * construct.
 */
void xc_split_combined_clauses(ompclause all, ompclause *parc, ompclause *wshc)
{
  if (all == NULL) { *parc = *wshc = NULL; return; }
  if (all->type == OCLIST)
  {
    xc_split_combined_clauses(all->u.list.next, parc, wshc);
    all = all->u.list.elem;
    assert(all != NULL);
  }
  all = ast_ompclause_copy(all);
  switch (all->type)
  {
    case OCIF:
    case OCSHARED:
    case OCCOPYIN:
    case OCREDUCTION:
    case OCNUMTHREADS:
    case OCPRIVATE:
    case OCDEFAULT:
      *parc = (*parc) ? OmpClauseList(*parc, all) : all;
      break;
    case OCFIRSTPRIVATE:
    default:  
      *wshc = (*wshc) ? OmpClauseList(*wshc, all) : all;
      break;
  }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     UTILITY FUNCTIONS                                         *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Finds the identifier and replaces it with a pointer to it.
 * Returns the declarator itself.
 */
astdecl xc_decl_topointer(astdecl decl)
{
  astdecl newdecl, id = decl_getidentifier(decl);

  newdecl = ParenDecl(Declarator(Pointer(), ast_decl_copy(id)));
  *id = *newdecl;
  free(newdecl);
  return (decl);
}

  
/* Finds the identifier and replaces its name.
 * Returns the declarator itself.
 */
astdecl xc_decl_rename(astdecl decl, symbol newname)
{
  astdecl newid, id = decl_getidentifier(decl);

  newid = IdentifierDecl(newname);
  *id = *newid;
  free(newid);
  return (decl);
}

  
/* Applies rules to ensure validity & OpenMP compliance of all clauses.
 * It won't check the variables inside the clauses.
 */
void xc_validate_clauses(enum dircontype dirtype, ompclause t)
{
  int hasnw = 0, hasord = 0, hasif = 0, hasnth = 0, hasdef = 0, hassch = 0,
      hasunt = 0, hascollapse = 0, hasfinal = 0, hasmerge = 0;

  if (t == NULL) return;
  if (t->type == OCLIST)
  {
    if (t->u.list.next != NULL)
      xc_validate_clauses(dirtype, t->u.list.next);
    t = t->u.list.elem;
    assert(t != NULL);
  }
  
  switch (t->type)
  {
    /* Data clauses */
    
    case OCREDUCTION:
      if (dirtype != DCPARALLEL && dirtype != DCFOR && dirtype != DCSECTIONS &&
          dirtype != DCFOR_P && dirtype != DCPARFOR && dirtype != DCPARSECTIONS)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                     "reduction clauses aren't allowed in `%s' directives\n",
                     t->file->name, t->l, ompdirnames[dirtype]);
      break;
    case OCSHARED:
      if (dirtype != DCPARALLEL && dirtype != DCPARFOR &&
          dirtype != DCPARSECTIONS && dirtype != DCTASK)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                    "shared clauses aren't allowed in `%s' directives\n",
                    t->file->name, t->l, ompdirnames[dirtype]);
      break;
    case OCCOPYIN:
      if (dirtype != DCPARALLEL && dirtype != DCPARFOR &&
          dirtype != DCPARSECTIONS)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                    "copyin clauses aren't allowed in `%s' directives\n",
                    t->file->name, t->l, ompdirnames[dirtype]);
      break;
    case OCPRIVATE:
    case OCFIRSTPRIVATE:
      if (dirtype != DCPARALLEL && dirtype != DCFOR && dirtype != DCSECTIONS &&
          dirtype != DCFOR_P && dirtype != DCPARFOR && 
          dirtype != DCPARSECTIONS && dirtype != DCSINGLE && dirtype != DCTASK)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                   "(first)private clauses aren't allowed in `%s' directives\n",
                   t->file->name, t->l, ompdirnames[dirtype]);
      break;
    case OCCOPYPRIVATE:
      if (dirtype != DCSINGLE)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                     "copyprivate clauses aren't allowed in `%s' directives\n",
                     t->file->name, t->l, ompdirnames[dirtype]);
      break;
    case OCLASTPRIVATE:
      if (dirtype != DCFOR && dirtype != DCSECTIONS && 
	  dirtype != DCFOR_P && dirtype != DCPARFOR && dirtype != DCPARSECTIONS)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                     "lastprivate clauses aren't allowed in `%s' directives\n",
                     t->file->name, t->l, ompdirnames[dirtype]);
      break;

    /* Non-data clauses */
      
    case OCIF:
      if (hasif)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                    "multiple if clauses are not allowed in a directive\n",
                    t->file->name, t->l);
      hasif = 1;
      if (dirtype != DCPARALLEL && dirtype != DCPARFOR &&
          dirtype != DCPARSECTIONS && dirtype != DCTASK)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                     "if clauses aren't allowed in `%s' directives\n",
                     t->file->name, t->l, ompdirnames[dirtype]);
      break;
    case OCFINAL:     /* OpenMP 3.1 */
      if (hasfinal)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                    "multiple final clauses are not allowed in a directive\n",
                    t->file->name, t->l);
      hasfinal = 1;
      if (dirtype != DCTASK)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                     "final clauses aren't allowed in `%s' directives\n",
                     t->file->name, t->l, ompdirnames[dirtype]);
      break;
    case OCMERGEABLE: /* OpenMP 3.1 */
      if (hasmerge)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                  "multiple mergeable clauses are not allowed in a directive\n",
                    t->file->name, t->l);
      hasmerge = 1;
      if (dirtype != DCTASK)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                     "mergeable clauses aren't allowed in `%s' directives\n",
                     t->file->name, t->l, ompdirnames[dirtype]);
      break;
    case OCNUMTHREADS:
      if (hasnth)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                "multiple num_threads clauses are not allowed in a directive\n",
                t->file->name, t->l);
      hasnth = 1;
      if (dirtype != DCPARALLEL && dirtype != DCPARFOR &&
          dirtype != DCPARSECTIONS)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                     "num_threads clauses aren't allowed in `%s' directives\n",
                     t->file->name, t->l, ompdirnames[dirtype]);
      break;
    case OCDEFAULT:
      if (hasdef)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                    "multiple default clauses are not allowed in a directive\n",
                    t->file->name, t->l);
      hasdef = 1;
      if (dirtype != DCPARALLEL && dirtype != DCPARFOR &&
          dirtype != DCPARSECTIONS && dirtype != DCTASK)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                     "default clauses aren't allowed in `%s' directives\n",
                     t->file->name, t->l, ompdirnames[dirtype]);
      break;
    case OCORDERED:
      if (hasord)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                    "multiple order clauses are not allowed in a directive\n",
                    t->file->name, t->l);
      hasord = 1;
      if (dirtype != DCFOR && dirtype != DCFOR_P && dirtype != DCPARFOR)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                     "ordered clauses aren't allowed in `%s' directives\n",
                     t->file->name, t->l, ompdirnames[dirtype]);
      break;
    case OCSCHEDULE:
      if (hassch)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                   "multiple schedule clauses are not allowed in a directive\n",
                   t->file->name, t->l);
      hassch = 1;
      if (dirtype != DCFOR && dirtype != DCFOR_P && dirtype != DCPARFOR)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                     "schedule clauses aren't allowed in `%s' directives\n",
                     t->file->name, t->l, ompdirnames[dirtype]);
      break;
    case OCNOWAIT:
      if (hasnw)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                    "multiple nowait clauses are not allowed in a directive\n",
                    t->file->name, t->l);
      hasnw = 1;
      if (dirtype != DCFOR && dirtype != DCSECTIONS && dirtype != DCSINGLE)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                    "nowait clauses aren't allowed in `%s' directives\n",
                    t->file->name, t->l, ompdirnames[dirtype]);
      break;
    case OCUNTIED:
      if (hasunt)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                    "multiple untied clauses are not allowed in a directive\n",
                    t->file->name, t->l);
      hasunt = 1;
      if (dirtype != DCTASK)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                    "untied clauses aren't allowed in `%s' directives\n",
                    t->file->name, t->l, ompdirnames[dirtype]);
      break;
    case OCCOLLAPSE:
      if (hascollapse)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                    "multiple collapse clauses are not allowed in a directive\n",
                    t->file->name, t->l);
      hascollapse = 1;
      if (dirtype != DCFOR && dirtype != DCFOR_P && dirtype != DCPARFOR)
        exit_error(1, "(%s, line %d) openmp error:\n\t"
                    "collapse clauses aren't allowed in `%s' directives\n",
                    t->file->name, t->l, ompdirnames[dirtype]);
      break;

    default:
      exit_error(1, "(%s, line %d) openmp error:\n\t"
                    "unknown clause type (%d) in `%s' directive\n",
                     t->file->name, t->l, t->type, ompdirnames[dirtype]);
      break;
  }
}


/* Next 2 functions return the first ompclause of the given type.
 */
static
ompclause clauselist_get_clause(ompclause t, enum clausetype type)
{
  ompclause c;

  if (t == NULL) return (NULL);
  if (t->type == OCLIST)
  {
    if (t->u.list.next != NULL) 
      if ( (c = clauselist_get_clause(t->u.list.next, type)) != NULL )
        return (c);
    assert((t = t->u.list.elem) != NULL);
  }
  return ( (t->type == type) ? t : NULL );
}


/* Returns the leftmost clause of the given type (or NULL) */
ompclause xc_ompcon_get_clause(ompcon t, enum clausetype type)
{
  return ( clauselist_get_clause(t->directive->clauses, type) );
}
