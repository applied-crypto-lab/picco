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

/* x_types.c -- transformations related to usertypes, structs & declarations */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "x_types.h"
#include "ast_free.h"
#include "ast_vars.h"
#include "ast_xform.h"
#include "ast_copy.h"
#include "ast_print.h"
#include "ast_show.h"
#include "symtab.h"
#include "str.h"
#include "picco.h"

    
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     STRUCTURES HANDLING                                       *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Things we do:
 *  (1) name all unnamed structures
 *  (2) all declarations/typedefs that include a full structure
 *      are broken into 2 statements, one for the structure definition and
 *      one for the simplified declaration/typedef.
 *  (3) multifields with user type specifiers are transformed to
 *      a list of single-field nodes.
 *  (4) all structures are recursively modified so as to not include
 *      any user types (i.e. all fields get substituted if needed).
 */


/* All unnamed structures get named here.
 */
static int nonamectr = 0;
static char _nns[16];
#define get_noname_struct() Symbol(_nns)
static symbol new_noname_struct()
{
  snprintf(_nns, 15, "_noname%d_", nonamectr++);
  return ( get_noname_struct() );
}


/* Break a multifield ("type f1, f2, ...;") into a series of single
 * fields and also substitute any user types. spec is assumed to
 * contain a usertype.
 */
static
astdecl multi2single_field(astspec spec, astdecl decl)
{
  if (decl)
    if (decl->type == DLIST && decl->subtype == DECL_decllist)
      return ( StructfieldList(multi2single_field(spec, decl->u.next),
                               multi2single_field(spec, decl->decl)) );
  if (decl)
  {
    assert(decl->type == DECLARATOR || decl->type == DBIT);
    decl = ast_decl_copy(decl);
  }
  spec = ast_spec_copy(spec);
  xt_barebones_substitute(&spec, &decl);       /* action: substitute */
  return ( StructfieldDecl(spec, decl) );
}
    

/* *t is assumed to be the u.decl of a structure */
static
void xt_break_multifield(astdecl *t)
{
  if (*t == NULL) return;
  if ((*t)->type == DLIST && (*t)->subtype == DECL_fieldlist)
  {
    xt_break_multifield(&((*t)->u.next));
    xt_break_multifield(&((*t)->decl));
  }
  else    /* Should be just a DSTRUCTFIELD */
  {
    astspec s;
    
    if ((*t)->type != DSTRUCTFIELD) return;

    /* Check if spec has another struct in it */
    if ((s = speclist_getspec((*t)->spec, SUE, 0)) != NULL &&
         (s->subtype == SPEC_struct || s->subtype == SPEC_union))
      xt_break_multifield(&(s->u.decl));
    
//    if (speclist_getspec((*t)->spec, USERTYPE, 0) != NULL)
    {
      astdecl n = multi2single_field((*t)->spec, (*t)->decl);
      ast_decl_free(*t);
      *t = n;
    }
  }
}


/* s/u/e { ... };            leave it alone
 * s/u/e name { ... };       ditto
 * s/u/e name x;             ditto
 * s/u/e { ... } x;          break into "s/u/e noname { ... };
 *                                       s/u/e noname x;"
 * s/u/e name { ... } x;     break into "s/u/e name { ... };
 *                                       s/u/e name x;"
 *
 * We also take care of inserting stuff in the symbol table.
 *
 * We don't care for such transformations within struct/union fields or
 * function parameter lists since their scope is not of concern to us.
 */
static
void xt_suedecl_xform(aststmt *d)
{
  aststmt n;
  astspec s;
  int     su;       /* 1 if struct or union, 0 if enum */
  
  assert((*d)->type == DECLARATION);
  if ((s = speclist_getspec((*d)->u.declaration.spec, SUE, 0)) == NULL) return;
  su = (s->subtype == SPEC_struct || s->subtype == SPEC_union);
  /* Break multidecl fields into single-field declarations (if needed),
   * and substitute any user-defined types.
   */
  if (su)
    xt_break_multifield(&(s->u.decl));
  /* Check for plain definition of a SUE */
  if ((*d)->u.declaration.decl == NULL)
  {
    if (s->name != NULL) /* Nothing to be done for "struct { ...};" */
    {
      stentry e = symtab_put(stab, s->name, su ? SUNAME : ENUMNAME);
      e->spec = (*d)->u.declaration.spec;    /* Remember those */
      e->decl = NULL;
    }
    return;
  }
  
  if ((su && s->u.decl == NULL) || (!su && s->body == NULL))
    return;  /* Nothing to be done here */

  /* Alter the specs if unnamed struct: name it. */
  if (s->name == NULL)
    s->name = new_noname_struct();

  /* Now we need to create the 2-node BlockList */
  n = Declaration(ast_spec_copy(s), NULL);   /* The 1st node */
  n->is_stmt_for_sng = (*d)->is_stmt_for_sng; 
  if (su)                                    /* Ditch the fields */
  {
    ast_decl_free(s->u.decl);
    s->u.decl = NULL;                   /* The 2nd node is ready */
  }
  else                                  /* Ditch the enumerators */
  {
    ast_spec_free(s->body);
    s->body = NULL;                     /* The 2nd node is ready */
  }
   
  *d = BlockList(n, *d);                    /* Do it */
  (*d)->parent = (*d)->body->parent;
  (*d)->body->parent = *d;
  n->parent = *d;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     USER-TYPE SUBSTITUTIONS                                   *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* We ditch all user types by substituting with bare bones ones.
 */


/* We are given a **(ABS)DECLARATOR** (subst) and we shall substitute it in
 * place of the IDENTIFIER of the original declarator.
 */
static
astdecl xt_substitute_decl(astdecl origdecl, astdecl subst)
{
  astdecl copy, id;

  assert(origdecl != NULL);
  assert(subst->type == DECLARATOR || subst->type == ABSDECLARATOR);
  
  id = decl_getidentifier( copy = ast_decl_copy(origdecl) );
  if ( !(subst->spec == NULL &&   /* if a DIDENT/DPAREN but not a pointer .. */
         subst->decl != NULL && (subst->decl->type == DIDENT ||
                                 subst->decl->type == DPAREN ||
                                 subst->decl->type == DFUNC)) )  
  { 
    subst = ParenDecl(subst);  /* Use ulgy parenthesis only if really needed */
    *id = *subst;
  }
  else
  {
    *id = *(subst->decl);          /* Cannot replace DIDENT with DECLARATOR! */
    free(subst->decl);
  }
  free(subst);
  return (copy);
}


/* Takes an astdecl node and substitutes any user types found in there,
 * which may occur basically in function params/structure fields/casts.
 */
void xt_barebones_decl(astdecl d)
{
  if (d == NULL) return;
  switch (d->type)
  {
    case DIDENT:
    case DELLIPSIS:
      break;
    case DINIT:
    case DPAREN:
    case DECLARATOR:
    case ABSDECLARATOR:
    case DBIT:
    case DARRAY:
      if (d->u.expr)
        ast_expr_xform(&(d->u.expr));
      if (d->decl) /* Maybe abstract declarator */
        xt_barebones_decl(d->decl);
      break;
    case DFUNC:             
      if (d->decl)          /* Maybe abstract declarator */
        xt_barebones_decl(d->decl);
      if (d->u.params)
        xt_barebones_decl(d->u.params);
      break;
    case DPARAM:
    case DCASTTYPE:         /* Here we get some action */
      xt_barebones_substitute(&(d->spec), &(d->decl));
      break;
    case DSTRUCTFIELD:
      /* This should never happen! */
      fprintf(stderr, "[xt_barebones_decl]: DSTRUCTFIELD (?!)\n");
      break;
    case DLIST:
      switch (d->subtype)
      {
        case DECL_idlist:
          break;              /* Nothing here */
        case DECL_decllist:
        case DECL_paramlist:
        case DECL_fieldlist:
          xt_barebones_decl(d->u.next);
          xt_barebones_decl(d->decl);
          break;
        default:
          fprintf(stderr, "[xt_barebones_decl]: list  b u g !!\n");
      }
      break;
    default:
      fprintf(stderr, "[xt_barebones_decl]: b u g !!\n");
  }
}


/* Take a declarator and transform it into an abstract one (a plain
 * copy is produced if it is already an abstract declarator).
 * Through the recursion, orig may not be a DECLARATOR at some point.
 * Anyways, there are only 4 possibilities if you look at the grammar.
 */
astdecl xt_concrete_to_abstract_declarator(astdecl orig)
{
  astdecl a = NULL, c = orig;
  int     isdor = 0;             /* 1 if c is DECLARATOR */

  if (orig==NULL) return NULL;
  if (c->type == ABSDECLARATOR) return ( ast_decl_copy(orig) );
  if (c->type == DECLARATOR) { isdor = 1; c = orig->decl; }
  if (c != NULL)
    switch (c->type)
    {
      case DIDENT:
        a = NULL;
        break;
      case DPAREN:
        assert(c->decl != NULL);
        a = (c->decl->type == DIDENT) ? NULL :
                  xt_concrete_to_abstract_declarator(c->decl);
        break;
      case DARRAY:
        a = ArrayDecl(xt_concrete_to_abstract_declarator(c->decl),
                      (c->spec != NULL && c->spec->type == SPEC 
                                       && c->spec->subtype == SPEC_star) ?
                        ast_spec_copy(c->spec) : NULL,
                      ast_expr_copy(c->u.expr));
        break;
      case DFUNC:
        a = FuncDecl(xt_concrete_to_abstract_declarator(c->decl),
                     (c->u.params && c->u.params->type == DLIST 
                                  && c->u.params->subtype == DECL_idlist) ?
                        NULL : ast_decl_copy(c->u.params));
        break;
      default:
        fprintf(stderr, "[c2a declarator]: c->decl->type = %d%s BUG !?\n", 
                        c->decl->type, isdor ? ", isdor=1" : ".");
    };
  if (isdor)
    a = AbstractDeclarator(ast_spec_copy(orig->spec), a);
  return (a);
}


/* spec and decl should point to the original ones; upon return they
 * point to the new ones that have been created through substitutions.
 * This can be only called for a spec/decl thing, i.e. in exactly 4 cases:
 *   (1) declarations
 *   (2) struct fields
 *   (3) function parameters declaration
 *   (4) type casts
 */
void xt_barebones_substitute(astspec *spec, astdecl *decl)
{
  int     subst = 0;  /* Becomes 1 if something was substituted */
  stentry e;
  astdecl child;
  astspec sp;

  if (*decl != NULL)
  {
    assert((*decl)->type == DECLARATOR || (*decl)->type == ABSDECLARATOR ||
           (*decl)->type == DINIT      || (*decl)->type == DBIT);
    if ((*decl)->type == DINIT)           /* Check the intializer */
      ast_expr_xform(&((*decl)->u.expr));
    if ((*decl)->type == DINIT || (*decl)->type == DBIT)
      decl = &((*decl)->decl);
  }
  
  /* If it is a usertype, then substitute the specifier. */
  if ((sp = speclist_getspec(*spec, USERTYPE, 0)) != NULL)
  {
    /* The user type may not even be in the stab (!), and this happens I think
     * only for the thread function argument. In this case, don't substitute.
     */
    if ((e = symtab_get(stab, sp->name, TYPENAME)) != NULL)
      /* If built-in additional type (e.g. __builtin_va_list), forget it */
      if (e->spec != NULL)
      {
        subst = 1;
        if (sp == *spec)                  /* I.e. spec was exactly a USERTYPE */
        {
          ast_spec_free(*spec);
          *spec = ast_spec_copy_nosc_asis(e->spec);       /* way easy */
        }
        else
        {
          astspec tmp;
          *sp = *(tmp = ast_spec_copy_nosc_asis(e->spec));
          free(tmp);
        }
      };
  }
  else
    /* May need to recurse inside structues */
    if ((sp = speclist_getspec(*spec, SUE, 0)) != NULL)
      if (sp->subtype == SPEC_struct || sp->subtype == SPEC_union)
        xt_break_multifield(&(sp->u.decl));
  
  if (*decl == NULL || (*decl)->decl == NULL)  /* structs, abs-pointers etc */
  {
    if (subst)
    { /* Was abstract declarator obvioulsy */
      if (*decl != NULL && (*decl)->type == ABSDECLARATOR)  /* A lonely ptr */
        *decl = xt_substitute_decl(e->decl, *decl);
      else
        *decl = xt_concrete_to_abstract_declarator(e->decl);
      xt_barebones_substitute(spec, decl);    /* recurse */
    }
    return;
  }
  
  /* At this point, *decl is a DECLARATOR/ABSDECLARATOR */
  child = (*decl)->decl;
  switch (child->type)
  {
    case DIDENT:
      if (subst)
        *decl = xt_substitute_decl(e->decl, *decl);
      break;
    case DPAREN:
      xt_barebones_decl(child->decl);
      if (subst)
        *decl = xt_substitute_decl(e->decl, *decl);
      break;
    case DARRAY:
      /* This is the only place we don't follow C99 to its full extend;
       * we cannot support varaible-length arrays! This is because
       * we won't check the spec/expresion inside the square brackets
       * of the array.
       */
      if (child->u.expr)
        ast_expr_xform(&((*decl)->decl->u.expr));
      if (child->decl) /* Maybe abstract declarator */
        xt_barebones_decl(child->decl);
      if (subst)
        *decl = xt_substitute_decl(e->decl, *decl);
      break;
    case DINIT:
      fprintf(stderr, "[xt_barebones_substitute]: DINIT (!?)\n");
      break;
    case DBIT:
      xt_barebones_decl(child->decl);
      if (subst)
        *decl = xt_substitute_decl(e->decl, *decl);
      break;
    case DFUNC:             /* Maybe abstract declarator */
      if (child->decl)
        xt_barebones_decl(child->decl);
      if (child->u.params)
        xt_barebones_decl(child->u.params);
      if (subst)
        *decl = xt_substitute_decl(e->decl, *decl);
      break;
    case DLIST:
      /* This can only occur when recursing, for normal declarations and
       * struct fields.
       */
      xt_barebones_decl(child->u.next);
      xt_barebones_decl(child->decl);
      break;
    case ABSDECLARATOR:
    case DECLARATOR:
      /* should not happen? */
      xt_barebones_decl(child->decl);
      if (subst)
        *decl = xt_substitute_decl(e->decl, *decl);
      break;
    default:
      fprintf(stderr, "[xt_barebones_substitute]: child->type = %d (!?)\n", 
                      child->type);
  }
  
  if (subst)
    xt_barebones_substitute(spec, decl);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     DECLARATION STATEMENT HANDLING                            *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* A list of retired typedefs */
static aststmt retiretree = NULL;

static
void xt_retire(aststmt t)
{
  if (retiretree == NULL)
    retiretree = t;
  else
    retiretree = BlockList(retiretree, t);
}

void xt_free_retired()
{
  if (retiretree)
    ast_stmt_free(retiretree);
}


static
aststmt multi2single_stmt(astspec spec, astdecl decl)
{
  if (decl->type == DLIST && decl->subtype == DECL_decllist)
  {
    aststmt w = BlockList(multi2single_stmt(spec, decl->u.next),
                          multi2single_stmt(spec, decl->decl));
    w->u.next->parent = w;
    w->body->parent = w;
    return (w);
  }
  assert(decl->type == DECLARATOR || decl->type == DINIT);
  return ( Declaration( ast_spec_copy(spec), ast_decl_copy(decl) ) );
}
    

/* takes "type var1, var2, ...;" and produces "type var1; type var2; ...;"
 * where the above are declarations.
 */
static
void xt_break_multidecl(aststmt *t)
{
  aststmt n = 
          multi2single_stmt((*t)->u.declaration.spec,(*t)->u.declaration.decl);
  n->parent = (*t)->parent;
  ast_stmt_free(*t);
  *t = n;
}


void xt_declaration_xform(aststmt *t)
{
  assert((*t)->type == DECLARATION);
  /* specs; (i.e. no declaration) */
  if ((*t)->u.declaration.decl == NULL)
  {
    xt_suedecl_xform(t);   /* just inform the symtab about SUE names (if any) */
    return;
  }    
  
  xt_suedecl_xform(t);   /* symtab & possibly break a complicated struct decl */
  if ((*t)->type != DECLARATION)   /* it was xformed after all */
  {
    ast_stmt_xform(t);
    return;
  }

  /* If we have >1 vars and the specs have a user type, we must
   * split the declaration into a list of seperate declarations, cause
   * we cannot handle it otherwise.
   */
  if ((*t)->u.declaration.decl->type == DLIST &&
      speclist_getspec((*t)->u.declaration.spec, USERTYPE, 0))
  {
    xt_break_multidecl(t);
    ast_stmt_xform(t);
    return;
  }

  /* We now have a list without usertype specs or a single variable;
   * prepare & declare.
   */
  if (!speclist_getspec((*t)->u.declaration.spec, STCLASSSPEC, SPEC_typedef))
  { 
    /* Be careful! Declare *after* any substitutions! */
    if ((*t)->u.declaration.decl->type == DLIST)
      xt_barebones_decl((*t)->u.declaration.decl);
    else
      xt_barebones_substitute(&((*t)->u.declaration.spec),
                              &((*t)->u.declaration.decl));
    ast_declare_decllist_vars((*t)->u.declaration.spec,
                              (*t)->u.declaration.decl);
    return;  
  }
  
  /* We are left with a typedef, which we also insert into stab. */
  ast_declare_decllist_vars((*t)->u.declaration.spec, (*t)->u.declaration.decl);
  
  /* We replace the node with a comment BUT the original node still hangs
   * around since there may follow declarations depending on it.
   * We retire those things so as to free them at some point in the future.
   */
  xt_retire(*t);
  
  A_str_truncate();                  /* commentize */
  str_printf(strA(), "/* (l%d) ", (*t)->l);
  ast_stmt_print(strA(), *t);
  str_seek(strA(), A_str_tell()-1);  /* delete the \n */
  str_printf(strA(), " */\n# %d \"%s\"", (*t)->l, (*t)->file->name);
  {
    aststmt parent = (*t)->parent;
    *t = Verbatim( strdup( A_str_string() ) );
    (*t)->parent = parent;
  }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     UTILITIES                                                 *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


int xt_decl_depends_on_sue(astdecl decl)
{
  if (decl == NULL) return (0);
  if (decl->type == DINIT) decl = decl->decl;
  if (decl == NULL) return (0);
  switch (decl->type)
  {
    case DPAREN: return ( xt_decl_depends_on_sue(decl) );
    case DARRAY:
      if (decl->decl) /* Maybe abstract declarator */
        return (xt_decl_depends_on_sue(decl->decl));
      return (0);
    case DBIT:
      return (xt_decl_depends_on_sue(decl->decl));
    case DFUNC:             /* Maybe abstract declarator */
      if (decl->decl)
        if (xt_decl_depends_on_sue(decl->decl)) return (1);
      if (decl->u.params)
        return (xt_decl_depends_on_sue(decl->u.params));
      return (0);
    case DLIST:
      /* This can only occur when recursing, for normal declarations and
       * struct fields.
       */
      if (xt_decl_depends_on_sue(decl->decl)) return (1);
      return (xt_decl_depends_on_sue(decl->u.next));
    case ABSDECLARATOR:
    case DECLARATOR:
      return (xt_decl_depends_on_sue(decl->decl));
  }
  return (0);
}


int xt_spec_depends_on_sue(astspec spec)
{
  stentry e;
  astspec s = (spec) ? speclist_getspec(spec, SUE, 0) : NULL;
  
  if (s != NULL && s->name != NULL)
    if ((e = symtab_get(stab, s->name,
                        (s->subtype == SPEC_enum) ? ENUMNAME : SUNAME)) != NULL)
      if (e->scopelevel > 0)
        return (1);
  return (0);
}


int xt_symbol_depends_on_sue(symbol s)
{
  stentry e = symtab_get(stab, s, IDNAME);
  return ( e && (xt_spec_depends_on_sue(e->spec)
                 || xt_decl_depends_on_sue(e->decl)) );
}


/* The next 3 funcs are only called for function parameters;
 * they "reduce" array variables to pointers. The first function
 * is for a declaration list (old style) and the 3rd for
 * a parameter list.
 */

 
void xt_dlist_array2pointer(aststmt d)
{
  if (d->type == STATEMENTLIST)
  {
    xt_dlist_array2pointer(d->u.next);
    xt_dlist_array2pointer(d->body);
    return;
  }
  assert(d->type == DECLARATION);
  if (d->u.declaration.decl)
    xt_decl_array2pointer(d->u.declaration.decl);
}


/* This is only used by xt_decl_array2pointer() and assumes d is a
 * declarator or a direct_declarator.
 */
static
void xt_directdecl_array2pointer(astdecl d)
{
  switch (d->type)
  {
    case DECLARATOR:
      xt_decl_array2pointer(d);
      break;
    /* direct_declarator cases */
    case DPAREN:         /* cannot have (id)[10] -- see parser.y */
      xt_decl_array2pointer(d->decl);
    case DIDENT:         /* nothing to do here */
    case DFUNC:
      break;
    case DARRAY:
      if (d->decl->type != DIDENT)
        xt_directdecl_array2pointer(d->decl);
      else   /* Got it */
      {
        astdecl t = ParenDecl( Declarator(Pointer(), d->decl) );
        if (d->u.expr) ast_expr_free(d->u.expr);
        if (d->spec)   ast_spec_free(d->spec);
        *d = *t;
        free(t);
      }
      break;
  }
}


void xt_decl_array2pointer(astdecl d)
{
  if (d == NULL) return; /* just in case */
  switch (d->type)
  {
    case DLIST:
      xt_decl_array2pointer(d->u.next);
      xt_decl_array2pointer(d->decl);
      break;
    case DINIT:
    case DPARAM:
      if (d->decl)
	{
		if((d->spec->subtype != SPEC_int && d->spec->subtype != SPEC_float) && (d->spec->body == NULL || d->spec->body->subtype == SPEC_public))
			xt_decl_array2pointer(d->decl);
	}
      break;
    case ABSDECLARATOR:        /* Should not occur in function definition */
    case DELLIPSIS:
      break;
    case DECLARATOR:
      xt_directdecl_array2pointer(d->decl);
      /* The following removes redundant parentheses.
       * However, it is wrong in that if inside the parenthesis there is
       * a pointer, we must "propagate" this outside the parentheses;
       * too tired to do it ...
      if (d->decl->type == DPAREN)
      {
        astdecl tmp = d->decl;
        *d = *d->decl->decl;
        free(tmp);
      }
       */
      break;
    default:
      fprintf(stderr, "[xt_decl_array2pointer]: !? BUG !? (type = %d):\n",
                      d->type);
      ast_decl_show(d);
  }
}
