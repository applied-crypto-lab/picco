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

/* x_parallel.c -- too big a transformation to be part of ast_xfrom! */

/* 
 * 2009/12/10:
 *   added thread function declared in transformed parent code.
 * 2007/09/09:
 *   changed placement of typedef structure; fixed bug with num_threads()
 *   (used variables in there were not "seen" as it was implemented as a
 *    verbatim node).
 * 2007/09/04:
 *   changed thread funcs naming, placement etc.
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
#include "x_parallel.h"
#include "x_clauses.h"
#include "x_thrpriv.h"
#include "symtab.h"
#include "str.h"
#include "picco.h"

/*
 * Thread function names/struct type names/reduction lock generators.
 * They also create global declarations for:
 *   - the new function
 *   - the reduction lock.
 *
 * Parallel constructs emit their own reduction code, independent of
 * other constructs.
 */
#define SHVARSNAME "_shvars"
#define SHVARSTYPE "__shvt__"
static int  thrnum = -1, par_red_num = -1;
static char _tfn[128], _prl[128];
#define thrfuncname() _tfn
#define parredlock()  _prl


static
void new_thrfunc()
{
  thrnum++;
  sprintf(_tfn, "_thrFunc%d_", thrnum);
}


static
void new_parreduction()
{
  par_red_num++;
  sprintf(_prl, "_paredlock%d", par_red_num);
  
  /* Add a global definition, too, for the lock (avoid using omp_lock_t) */
  newglobalvar( Declaration(Speclist_right(
                              StClassSpec(SPEC_static),
                              Declspec(SPEC_void, 0)
                            ),
                            Declarator(
                              Pointer(),
                              IdentifierDecl( Symbol(_prl) )
                            )) );
}


/*
 * The following 3 functions memorize the variables included in the
 * data clauses of the parallel directive we are transforming,
 * along with the clause type of each one (private/firstprivate/etc).
 * They also check for presence of inappropriate clauses.
 *
 */
static symtab    dataclause_vars = NULL;    /* Store the list here */
static ompclause dataclause_vars_clause;    /* Only needed for error messages */


static
void store_varlist_vars(astdecl d, int clausetype, int opid)
{
  stentry e;
  
  if (d->type == DLIST && d->subtype == DECL_idlist)
  {
    store_varlist_vars(d->u.next, clausetype, opid);
    d = d->decl;
  }
  if (d->type != DIDENT)
    exit_error(1, "[store_varlist_vars]: !!BUG!! not a DIDENT ?!\n");
  if (symtab_get(dataclause_vars, d->u.id, IDNAME) != NULL)
    exit_error(1, "(%s, line %d) openmp error:\n\t"
                  "variable `%s' appears more than once in a\n\t"
                  "parallel directive's clause(s).\n",
                  dataclause_vars_clause->file->name, dataclause_vars_clause->l,
                  d->u.id->name);
  /* Put the opid in the "value" field */
  e = symtab_put(dataclause_vars, d->u.id, IDNAME);
  e->vval = opid;
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
    case OCCOPYIN:
    case OCSHARED:
    case OCREDUCTION:
      if (t->u.varlist)   /* t->subtype is the opid in case of reduction */
        store_varlist_vars(t->u.varlist, t->type, t->subtype);
      break;
    case OCIF:
    case OCNUMTHREADS:
    case OCDEFAULT:
      break;
    default:
      exit_error(1, "(%s, line %d) openmp error:\n\t"
                    "illegal clause (%s) in parallel directive\n",
                    t->file->name, t->l, clausenames[t->type]);
  }
}


static
void xp_store_dataclause_vars(ompdir d)
{
  if (dataclause_vars == NULL)
    dataclause_vars = Symtab();
  else
    symtab_drain(dataclause_vars);
  if (d->clauses)
    store_dataclause_vars(d->clauses);
}


#define isReduction(s)    isDataShareType(s,OCREDUCTION)
#define isFirstprivate(s) isDataShareType(s,OCFIRSTPRIVATE)
#define isPrivate(s)      isDataShareType(s,OCPRIVATE)
#define isCopyin(s)       isDataShareType(s,OCCOPYIN)
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
 
 
#define xp_thrarg_struct_nameonly() SUdecl(SPEC_struct,Symbol(SHVARSTYPE),NULL)

/* Produces a type specifier for the struct, with all fields listed */
static
astspec xp_thrarg_struct()
{
  stentry e, orig;
  astdecl st = NULL, tmp;
  
  /* Create the fields */
  for (e = sng_vars->top; e; e = e->stacknext)
  {
    orig = symtab_get(stab, e->key, IDNAME);
    /* 
    tmp = StructfieldDecl(
            ast_spec_copy_nosc(orig->spec),
            xc_decl_topointer(
              (threadmode && orig->isthrpriv) ?  // 'cause of changed name!!! 
                xc_decl_rename(ast_decl_copy(orig->decl), e->key) :
                ast_decl_copy(orig->decl)
            )
          );
    */
    astdecl sub = decl_getidentifier(orig->decl); 
    char* var = (char*)malloc(sizeof(char)*100); 
    sprintf(var, "(*%s)", sub->u.id->name);
    symbol s = Symbol(strdup(var)); 
    s->next = sub->u.id->next; 
    s->type = sub->u.id->type; 
    s->isptr = sub->u.id->isptr; 
    s->struct_type = sub->u.id->struct_type; 
    astdecl decl = ast_decl_copy(orig->decl); 
    decl_getidentifier(decl)->u.id = s; 
    // astdecl decl = Declarator(NULL, IdentifierDecl(Symbol(strdup(var))));
    tmp = StructfieldDecl(
		ast_spec_copy_nosc(orig->spec), decl);
    st = (st == NULL) ? tmp : StructfieldList(st, tmp);
    free(var); 
  }

  /* struct _thrarg_ { ... } */
  return SUdecl(SPEC_struct, Symbol(SHVARSTYPE), st);
}


/* Produces an initializer, to initialize the structure of shared variables
 */
astexpr xp_parstruct_initializer()
{
  stentry e;
  astexpr st = NULL, tmp;
  
  for (e = sng_vars->top; e; e = e->stacknext)
  {
    tmp = (threadmode && symtab_get(stab, e->key, IDNAME)->isthrpriv) ?
            Identifier(e->key) :         /* 'cause it is already a pointer!!! */
            UnaryOperator(UOP_addr, Identifier(e->key));
    st = (st == NULL) ? tmp : CommaList(st, tmp);
  }
  return ( BracedInitializer(st) );
}


/* This one takes all the discovered shared (nonglobal) vars and
 * checks if they comply, i.e. if they are either explicitely shared
 * or there is a default(shared) clause present. They are checked
 * against the dataclause_vars.
 */
void xp_parallel_check_shared(ompcon t)
{
  stentry   e;
  int       defnone;
  ompclause c = xc_ompcon_get_clause(t, OCDEFAULT);

  defnone = (c != NULL && c->subtype == OC_defnone);
  for (e = sng_vars->top; e; e = e->stacknext)
  {
    if (symtab_get(dataclause_vars, e->key, IDNAME) == NULL && defnone)
      exit_error(1, 
        "(%s) openmp error:\n\t"
        "variable `%s' must be explicitely declared as shared due to\n\t"
        "the default(none) clause of the parallel directive at line %d.\n",
        t->directive->file->name, e->key->name, t->directive->l);
  }
}


/* sng_vars contains all shared *non-global* vars.
 * In the special case of firstprivate and reduction variables that
 * are based on *global* ones, we also put them in sng_vars so
 * that they are initialized smoothly, without checking for such special
 * situations. These vars will NOT be declared through
 * xp_sharedng_declarations(), of course.
 * We do the same for global threadprivate vars.
 */
void xp_sharedng_add_fpredgvars()
{
  stentry e;
  for (e = dataclause_vars->top; e; e = e->stacknext)
    if ((isFirstprivate(e->key) || isReduction(e->key)
                                || symtab_get(stab, e->key, IDNAME)->isthrpriv)
        && isGlobal(stab, e->key))
      symtab_put(sng_vars, e->key, IDNAME);
}

    
/*
 * Firstprivate initialization statements
 *
 */
static aststmt fpinits = NULL;


/* Produces a memory copy statement, for initializing firstprivate vars
 * which are non-scalar & adds the statement to "fpinits".
 */
static
void xp_fiparray_initialization(symbol var, astexpr init)
{
  aststmt st = xc_array_initializer(var, init);
  fpinits = (fpinits == NULL) ? st : BlockList(fpinits, st);
}


/*
 * Copyin initialization statements
 *
 */
static aststmt ciinits = NULL;

 
/* Produces statements for initializing *all* copyin vars.
 * Scalar ones need plain assignments, non-scalars need array-copy
 * statements, just like in the firstprivate case.
 */
static
aststmt xp_copyin_copy_stmt(symbol var)
{
  stentry e = symtab_get(stab, var, IDNAME);
  astexpr init;
  
  
  init = /* ( e->scopelevel == -10 ) ?    
           Identifier( tp_new_name(var) ) : */  /* The initializer/source */
           UnaryOperator(
             UOP_star,
             UnaryOperator(                   /* = *(_thrarg->var) */
               UOP_paren,
               PtrField(Identifier(Symbol(SHVARSNAME)), var)
             )
           );
           
  if (!e->isarray)           /* Plain assignment */
    return 
      Expression(            /* *var = <orig_var> */
        Assignment(
          UnaryOperator(UOP_star, Identifier(var)),
          ASS_eq,
          init
        )
      );
  else                       /* Array assignment, like in firstprivates */
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
                UnaryOperator(UOP_star, Identifier(var))
              ), 
              CastedExpr( 
                Casttypename(
                  Declspec(SPEC_void, 0),
                  AbstractDeclarator(Pointer(),NULL)
                ), 
                init 
              ) 
            ), 
            Sizeof(UnaryOperator(UOP_star, Identifier(var))) 
          ) 
        ) 
      );
  }
}


/* Produces a memory copy statement, for initializing copyin tp vars */
static
void xp_copyin_initialization(symbol var)
{
  aststmt st = xp_copyin_copy_stmt(var);
  ciinits = (ciinits == NULL) ? st : BlockList(ciinits, st);
}


/*
 * Reduction code
 *
 */

 
/* Generates code for reductions of a list of variables.
 * This is parallel-specific; other constructs emit different code.
 *   *(_thrarg->var) op= var   or
 *   *(_thrarg->var) = *(_thrarg->var) op var   (for && and ||)
 */
aststmt xp_varlist_reduction_code(astdecl d, int op)
{
  aststmt list = NULL, st = NULL;
  
  if (d->type == DLIST && d->subtype == DECL_idlist)
  {
    list = xp_varlist_reduction_code(d->u.next, op);
    d = d->decl;
    assert(d != NULL);
  }
  if (d->type != DIDENT)
    exit_error(1, "[xp_varlist_reduction_code]: !!BUG!! not a DIDENT ?!\n");

  if (op == OC_min || op == OC_max)
    st = If(
           BinaryOperator(
             (op == OC_min) ?  BOP_gt : BOP_lt,
             UnaryOperator(
               UOP_star,
               UnaryOperator(
                 UOP_paren,
                 PtrField(Identifier(Symbol(SHVARSNAME)), d->u.id)
               )
             ),
             Identifier(d->u.id)
           ),
           Expression(
             Assignment(
               UnaryOperator(
                 UOP_star,
                 UnaryOperator(
                   UOP_paren,
                   PtrField(Identifier(Symbol(SHVARSNAME)), d->u.id)
                 )
               ),
               ASS_eq,           
               Identifier(d->u.id)
             )
           ),
	   NULL
	 );
  else
    st = Expression(
           Assignment(
             UnaryOperator(
               UOP_star,
               UnaryOperator(
                 UOP_paren,
                 PtrField(Identifier(Symbol(SHVARSNAME)), d->u.id)
               )
             ),
           
             (op == OC_plus)  ? ASS_add :
             (op == OC_minus) ? ASS_add :  /* Indeed! */
             (op == OC_times) ? ASS_mul :
             (op == OC_band)  ? ASS_and :
             (op == OC_bor)   ? ASS_or  :
             (op == OC_xor)   ? ASS_xor : ASS_eq,
           
             (op != OC_land && op != OC_lor) ?
               Identifier(d->u.id) :
               BinaryOperator(
                 (op == OC_land) ? BOP_land : BOP_lor,
                 UnaryOperator(
                   UOP_star,
                   UnaryOperator(
                     UOP_paren,
                     PtrField(Identifier(Symbol(SHVARSNAME)), d->u.id)
                   )
                 ),
                 Identifier(d->u.id)
               )
           )
         );
  return ( (list != NULL) ? BlockList(list, st) : st );
}


/* Generates code for a reduction clause:
 *    ort_reduction_begin(&redlock);
 *       <code for each variable in list>
 *    ort_reduction_end(&redlock);
 */
aststmt xp_reduction_code(ompclause t)
{
  aststmt list = NULL, st;

  if (t == NULL) return(NULL);
  
  if (t->type == OCLIST)
  {
    if (t->u.list.next != NULL)
      list = xp_reduction_code(t->u.list.next);
    t = t->u.list.elem;
    assert(t != NULL);
  }
  if (t->type != OCREDUCTION) return (list);
  
  new_parreduction();        /* Prepare a new reduction lock */
  st = BlockList(
         BlockList(
           Expression(
             FunctionCall(
               Identifier(Symbol("ort_reduction_begin")),
               UnaryOperator(UOP_addr, Identifier(Symbol(parredlock())))
             )
           ),
           xp_varlist_reduction_code(t->u.varlist, t->subtype)
         ),
         Expression(
           FunctionCall(
             Identifier(Symbol("ort_reduction_end")),
             UnaryOperator(UOP_addr, Identifier(Symbol(parredlock())))
           )
         )
       );
  return ( (list != NULL) ? BlockList(list, st) : st );
}


/*
 * Declarations-related stuff that go into the thread function.
 *
 */

 
/* Produces a list of declarations that declare pointer variables,
 * initialized to where the structure fields point to.
 * Those declarations will go to the top of the thread function,
 * and cover the shared non-global variables.
 * Instead of looking into the shared() clauses, we look at all the
 * discovered shared vars in sng_vars.
 */
aststmt xp_sharedng_declarations()
{
  stentry e, orig;
  aststmt st = NULL, tmp;
  
  for (e = sng_vars->top; e; e = e->stacknext)
  {
    /* First filter out any extra inserted vars */
    if (isReduction(e->key) || isFirstprivate(e->key) || isCopyin(e->key))
      continue;      /* Declared elsewhere */
    
    /* Declaration with initializer (  = argname->var ).
     * Non-global threadprive vars are a bit more complex.
     */
    orig = symtab_get(stab, e->key, IDNAME);
    if (threadmode && orig->isthrpriv)
      tmp = tp_declaration(orig, e->key,
                           PtrField(Identifier(Symbol(SHVARSNAME)), e->key), 1);
    else{
    	   astdecl sub = decl_getidentifier(orig->decl);
	   
	   char* var = (char*)malloc(sizeof(char) * 100);   
           sprintf(var, "(*%s)", decl_getidentifier(orig->decl)->u.id->name);
           symbol s = Symbol(strdup(var));
  	   s->next = sub->u.id->next;                     
    	   s->type = sub->u.id->type;
    	   s->isptr = sub->u.id->isptr; 
    	   s->struct_type = sub->u.id->struct_type;
	   
	   astdecl decl = ast_decl_copy(orig->decl); 
	   decl_getidentifier(decl)->u.id = s;
	   
	  //astdecl decl = Declarator(NULL, IdentifierDecl(Symbol(strdup(var))));   
      	   
	   tmp = Declaration(
              ast_spec_copy_nosc(orig->spec),
              InitDecl(
		decl, 
                //xc_decl_topointer(     /* Same decl but change to pointer */
                  //ast_decl_copy(orig->decl)
                //),
                PtrField(Identifier(Symbol(SHVARSNAME)), e->key)
              )
            );
	   //free(var); 
    }
    tmp->is_stmt_for_sng = 2; 
    st = (st == NULL) ? tmp : BlockList(st, tmp);
  }
  return (st);
}


/* Produces a list of declarations for the copyin variables.
 * Notice that for vars that are NOT in copyin clauses:
 *   (1) for global tp vars, pointers will be insert later (through
 *       tp_fix_funcbody_gtpvars()) when the new function will be xformed).
 *   (2) for non-global tp vars, pointers will be declared above,
 *       in xp_sharedng_declarations().
 */
static
aststmt xp_copyin_declarations()
{
  stentry e, orig;
  aststmt st = NULL, tmp;

  for (e = dataclause_vars->top; e; e = e->stacknext)
  {
    if (!isCopyin(e->key) || !threadmode) continue;
    orig = symtab_get(stab, e->key, IDNAME);
    if (orig->scopelevel == 0)       /* Global */
      tmp = tp_declaration(orig, e->key, Identifier( tp_new_name(e->key) ), 0);
    else
      tmp = tp_declaration(orig, e->key,
                           PtrField(Identifier(Symbol(SHVARSNAME)), e->key), 1);
    st = (st == NULL) ? tmp : BlockList(st, tmp);
    
    xp_copyin_initialization(e->key);        /* Add to initializations */
  }
  return (st);
}


/* Produces a list of declarations that cover firstprivate() variables.
 * Those are initialized from the structure fields directly.
 * It produces extra statements for intialization when the variable is
 * non-scalar.
 */
aststmt xp_firstprivate_declarations()
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
    /* The initializer (used either as is or to produce array intializer) */
    init = UnaryOperator(UOP_star, 
             UnaryOperator(UOP_paren,
               PtrField(Identifier(Symbol(SHVARSNAME)), e->key)));

    /* Declaration with initializer, except if it is an array type.
     * In this case initializers are outputed later as normal 
     * statements (memcpy()s).
     */
    tmp = Declaration(
            ast_spec_copy_nosc(symtab_get(stab, e->key, IDNAME)->spec),
            array ? decl :  InitDecl(decl, init)      /* = *(_thrarg->var) */
          );
    if (array)
      xp_fiparray_initialization(e->key, init);
    
    st = (st == NULL) ? tmp : BlockList(st, tmp);
  }
  return (st);
}


/* Produces a list of declarations for private() and reduction() variables.
 * ompdir is needed only for error messages.
 */
aststmt xp_privered_declarations(ompdir d)
{
  stentry e;
  aststmt st = NULL, tmp;
  int     red;

  for (e = dataclause_vars->top; e; e = e->stacknext)
  {
    red = isReduction(e->key);
    if (!red && !isPrivate(e->key)) continue;

    if (red && symtab_get(stab, e->key, IDNAME)->isarray)
      exit_error(1, "(%s, line %d) openmp error:\n\t"
                    "reduction variable `%s' is non-scalar.\n",
                    d->file->name, d->l, e->key->name);

    tmp = xform_clone_declaration(e->key, /* Get opid from ->vval */
                            red ? xc_reduction_initializer(e->vval, e->key)
                                : NULL);
    st = (st == NULL) ? tmp : BlockList(st, tmp);
  }
  return (st);
}
                        

/* This also checks if the clause is unique */
astexpr xp_ifnumthreads_clause(ompclause t, enum clausetype type)
{
  astexpr e = NULL;
  
  if (t->type == OCLIST)
  {
    if (t->u.list.next != NULL)   /* depth-first to check uniqueness */
      e = xp_ifnumthreads_clause(t->u.list.next, type);
    assert((t = t->u.list.elem) != NULL);
  }
  if (t->type == type)
  {
    if (e != NULL)
      exit_error(1, "(%s, line %d) openmp error:\n\t"
                    "multiple %s() clauses in parallel directive.\n",
                    t->file->name, t->l, clausenames[type]);
    e = t->u.expr;
  }
  return (e);
}


/* Returns the expression of the unique num_threads() clause or NULL
 */
astexpr xp_numthreads(ompdir d)
{
  return ( d->clauses ? 
           xp_ifnumthreads_clause(d->clauses, OCNUMTHREADS) : NULL );
}


/* Returns the expression of the unique if() clause or NULL
 */
astexpr xp_if(ompdir d)
{
  return ( d->clauses ? xp_ifnumthreads_clause(d->clauses, OCIF) : NULL );
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     THE CORE FUNCTION                                         *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Repeats all visible non-global struct/union/enum declarations */
aststmt xp_sue_declarations()
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


void xform_parallel(aststmt *t, int iscombined)
{
  
  aststmt parent_stmt = (*t)->openmp_parent_stmt;  
  aststmt body = (*t)->u.omp->body;   /* Remember current node */
  aststmt p = (*t)->parent, funcdef, v,
          prvardecl = NULL, shvardecl = NULL, fpvardecl = NULL, reducode = NULL,
          civardecl = NULL, ortcall;
  astexpr numthrexpr, ifexpr;
  int     compos, bodyline;
  str     st1 = Strnew();
  symbol  sfile = (*t)->file;
  
  new_thrfunc();                       /* Get a number */
  fpinits = ciinits = NULL;            /* Important!! */
  
  /* (1) Put the directive in a comment
   */
  str_printf(st1, "/* (l%d) ", ((*t)->u.omp->directive)->l);
  ast_ompdir_print(st1, ((*t)->u.omp->directive));
  compos = str_tell(st1);             /* Remember where we are now */
  str_printf(st1, " */");
  v = Verbatim( strdup( str_string(st1) ) );
  bodyline = ((*t)->u.omp->directive)->l;
  
  /* (2) Remember our data clause variables; also prepare reduction code. 
   */
  xp_store_dataclause_vars((*t)->u.omp->directive);
  reducode = xp_reduction_code((*t)->u.omp->directive->clauses);
  
  /* (3) Check for if and num_threads clauses and keep a copy 
   */
  if ((numthrexpr = xp_numthreads((*t)->u.omp->directive)) != NULL)
    numthrexpr = ast_expr_copy(numthrexpr);
  if ((ifexpr = xp_if((*t)->u.omp->directive)) != NULL)
    ifexpr = ast_expr_copy(ifexpr);
    
  /* (4) Discover shared non-globals and check for default() compliance 
   */
  ast_paracon_find_sng_vars((*t)->u.omp, 1);
  xp_parallel_check_shared((*t)->u.omp);
  xp_sharedng_add_fpredgvars();   /* Add global firstprivate & reduction vars */

  /* (5) Prepare the declarations for local & shared variables 
   */
  shvardecl = xp_sharedng_declarations();
  civardecl = xp_copyin_declarations();
  prvardecl = xp_privered_declarations((*t)->u.omp->directive);
  fpvardecl = xp_firstprivate_declarations();
  /* Combine to reduce options */
  if (fpvardecl && civardecl)
    fpvardecl = BlockList(civardecl, fpvardecl);
  else
    if (fpvardecl == NULL)
      fpvardecl = civardecl;
  if (prvardecl && fpvardecl)
    prvardecl = BlockList(fpvardecl, prvardecl);
  else
    if (prvardecl == NULL)
      prvardecl = fpvardecl;

  (*t)->u.omp->body = NULL;     /* Make it NULL so as to free it easily */
  ast_free(*t);                 /* Get rid of the OmpStmt */

  /* (6) Replace the current node with a new code that creates the threads 
   *       {
   *         <commented directive>
   *         <structure declaration if needed>
   *         <thread function declaration>
   *
   *         if (ifclause_expression)
   *           ort_execute_parallel(...)
   *         else
   *           ort_execute_serial(...)
   *       }
   */
  if (!has_sng_vars())
    *t = v;
  else{
    	aststmt sng_stmt = Declaration( xp_thrarg_struct(),
                        InitDecl(
                          Declarator(
                            NULL,
                            IdentifierDecl(Symbol(SHVARSNAME))
                          ),
                          xp_parstruct_initializer()
                        )
                      ); 
	sng_stmt->is_stmt_for_sng = 1; 	
	*t = BlockList(v, sng_stmt); /* Declare a structure to hold pointers to the used vars */
  }
  ortcall = Expression(  /* ort_execute_parallel(<numthr>, thrfunc, arg/0, */
              FunctionCall(                                   /* combined) */
                Identifier(Symbol("ort_execute_parallel")),
		CommaList(					      
                  CommaList(
                    CommaList(
                      numthrexpr ? numthrexpr : numConstant(-1),
                      Identifier(Symbol(thrfuncname()))
                    ),
                    CastedExpr( 
                      Casttypename(
                        Declspec(SPEC_void, 0),
                        AbstractDeclarator(Pointer(),NULL)
                      ),
                      has_sng_vars() ? 
                        UnaryOperator(UOP_addr, Identifier(Symbol(SHVARSNAME))) :
                        numConstant(0)
                    ) 
                  ),
                  numConstant(iscombined)
                )
              )
            );
  if (ifexpr)                    /* Check if we have an if() clause */
  {
    str tmp = Strnew();
    
    if (has_sng_vars())
      str_printf(tmp, "ort_execute_serial(%s, (void *) &%s);", 
                      thrfuncname(), SHVARSNAME);
    else
      str_printf(tmp, "ort_execute_serial(%s, (void *) 0);", thrfuncname());
      
    *t = Compound(
           BlockList(
             *t,
             If( ifexpr, ortcall, verbit( str_string(tmp) ) )
           )
         );
    str_free(tmp);
  }
  else
    *t = Compound( BlockList( *t, ortcall ) );
 (*t)->file = sfile;
  ast_stmt_parent(p, *t);

  /* (7) Prepare the body of the new function 
   */
         /* Take the current body, add a return( (void *) 0 ) statement
          * to avoid warnings and a couple of comments; the whole thing
          * will be enclosed in a compound statement.
          * If there are any copyin/firstprivate initialization statements
          * (for arrays), prepend them.
          
          */
  str_insert(st1, compos, " -- body moved below");  /* Another comment */
  str_printf(st1, "\n# %d \"%s\"", bodyline, body->file->name);
  v = Verbatim( strdup( str_string(st1) ) );
  str_truncate(st1);
  body = BlockList(v, body);
  
  if (ciinits)
    body = BlockList(
             BlockList(
               BlockList(verbit("/* copyin initialization(s) */"), ciinits), 
               BarrierCall()
             ),
             body
           );
  if (fpinits)
    body = BlockList(
             BlockList(verbit("/* 1stprivate array initializer(s) */"),fpinits),
             body
           );

         /* Declare any variables from the clauses and append reduction code.
          */
  if (has_sng_vars())
  {
    /* Declare _thrarg = (argtype *) ort_get_shared_vars(__me); */
    aststmt castarg = Declaration(
                        xp_thrarg_struct(),
                        InitDecl(
                          Declarator(
                            Pointer(),
                            IdentifierDecl( Symbol(SHVARSNAME) )
                          ),
                          CastedExpr(
                            Casttypename(
                              xp_thrarg_struct_nameonly(),
                              AbstractDeclarator(Pointer(),NULL)
                            ),
                            //Call0_expr("ort_get_shared_vars")
                            FunctionCall(
                              Identifier(Symbol("ort_get_shared_vars")),
                              Identifier(Symbol("__me"))
                            )
                          )
                        )
                      );
    castarg->is_stmt_for_sng = 1;
	 
    /* For sure, we need to declare _thrarg.
     * We may also have shared vars, and maybe private vars, too.
     * We may also have firstprivate initializations, as well as
     * some copyin code.
     * Then we have the body and possibly some reduction code.
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
  
  if (reducode)
    body = BlockList(body, reducode);

  if ((p = xp_sue_declarations()) != NULL)
    body = BlockList(p, body);
  
  body = Compound(
  	   BlockList(
	     body,
             BlockList(  /* ort_taskwait(2); return (void *) 0; */
               Expression(FunctionCall(Identifier(Symbol("ort_taskwait")), 
			               numConstant(2))), 
               Return(CastedExpr(Casttypename(Declspec(SPEC_void, 0),
			         AbstractDeclarator(Declspec(SPEC_star, 0), NULL)),
				 numConstant(0))
                     )
             )
           )
         );
	 
  /* (8) Define the new function.
   *         void *func ( void *__me ) <body>
   */
  funcdef = FuncDef(Speclist_right(StClassSpec(SPEC_static),Declspec(SPEC_void, 0)),
                    Declarator( 
                      Pointer(), 
                      FuncDecl( 
                        IdentifierDecl( Symbol(thrfuncname()) ) , 
                        ParamDecl( Declspec(SPEC_void, 0),
                                   Declarator(
                                     Pointer(),
                                     IdentifierDecl( Symbol("__me") )
                                   )
                                 )
                      ) 
                    ),
                    NULL, body);
      funcdef->num_tmp = parent_stmt->num_tmp;
      funcdef->num_float_tmp = parent_stmt->num_float_tmp;
      funcdef->is_priv_int_ptr_appear = parent_stmt->is_priv_int_ptr_appear;
      funcdef->is_priv_float_ptr_appear = parent_stmt->is_priv_float_ptr_appear;
      funcdef->is_priv_int_index_appear = parent_stmt->is_priv_int_index_appear;
      funcdef->is_priv_float_index_appear = parent_stmt->is_priv_float_index_appear;
      funcdef->is_priv_int_struct_field_appear = parent_stmt->is_priv_int_struct_field_appear;
      funcdef->is_priv_float_struct_field_appear = parent_stmt->is_priv_float_struct_field_appear;
      funcdef->is_priv_struct_ptr_struct_field_appear = parent_stmt->is_priv_struct_ptr_struct_field_appear;
      funcdef->is_priv_int_ptr_struct_field_appear = parent_stmt->is_priv_int_ptr_struct_field_appear;
      funcdef->is_priv_float_ptr_struct_field_appear = parent_stmt->is_priv_float_ptr_struct_field_appear;
      funcdef->contain_priv_if_flag = parent_stmt->contain_priv_if_flag;  
  /* (9) Add the new function, along with the struct type definition
   */
  ast_stmt_parent(funcdef, funcdef);     /* Parentize nicely */
  xfrom_add_threadfunc(Symbol(thrfuncname()), funcdef,
                       ast_get_enclosing_function(*t));
    
  str_free(st1);
}
