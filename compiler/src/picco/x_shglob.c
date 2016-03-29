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

/* x_shglob.c -- takes care of globals in the process model */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "ast_xform.h"
#include "ast_free.h"
#include "ast_copy.h"
#include "x_thrpriv.h"
#include "x_types.h"
#include "x_clauses.h"
#include "symtab.h"
#include "picco.h"


/* This is used only in the cases where a global var has an initializer */
static char _dini[128];
#define dininame() _dini

static
void new_insertdummyinitvar(stentry orig)
{
  stentry e;
  
  sprintf(dininame(), "_sglini_%s", orig->key->name);
  e = symtab_insert_global(stab, Symbol(dininame()), IDNAME);  /* Declare it */
  e->decl       = xc_decl_rename(orig->decl, Symbol(dininame()));
  e->spec       = orig->spec;
  e->idecl      = orig->idecl;
  e->isarray    = orig->isarray;
  e->isthrpriv  = orig->isthrpriv;
  e->scopelevel = 0;
}


/* Takes all shared global variables and produces initialization code,
 * when compiling for the process model. Also, all global variables are
 * re-declared as pointers.
 * This is assumed to be called when in global scope where all threadprivate
 * variables are marked as such.
 * We have to skip extern variables, too.
 *
 * After this function *no other transformation must be applied*; in
 * particular, the "tail" tree should not be touched.
 */
void sgl_fix_sglvars()
{
  stentry e;
  aststmt st, l = NULL;
  astexpr initer;
  struct timeval ts;
  char    funcname[32];

  if (!processmode || stab->scopelevel > 0) return; /* Must be in global scope */
  
  for (e = stab->top; e; e = e->stacknext)
  {
    if (e->space != IDNAME || e->isthrpriv || decl_getkind(e->decl) == DFUNC ||
        e->key == Symbol("__ompi_defs__") ||
        speclist_getspec(e->spec, STCLASSSPEC, SPEC_extern) != NULL)
      continue;
    /* If there is an initializer, we have to change our plans:
     *     <specs> var = <init>
     * will be transformed to:
     *     <specs> dummyvar = <init>, *var
     * and we will use &dummyvar for the initialization.
     */
    if (e->idecl == NULL) 
      initer = NULL;
    else
    {
      astdecl newdecl = ast_decl_copy(e->decl),
              newinit = InitDecl(e->decl, e->idecl->u.expr),
              list    = DeclList(newinit, newdecl);
              
      *(e->idecl) = *list;               /* var=init, var */
      free(list);
      e->idecl = newinit;                /* Point to original initializer */
      new_insertdummyinitvar(e);
      e->decl  = newdecl;
      e->idecl = NULL;
      initer = UnaryOperator(UOP_addr, Identifier(Symbol(dininame())));
    }
    
    /* Notice that we output "&var"; this means that this code
     * should NOT be transformed, as it will produce (wrongly) &(*var).
     */
    st = Expression(
           FunctionCall(
             Identifier(Symbol("ort_sglvar_allocate")),
             CommaList(
               CommaList(
                 CastedExpr( 
                   Casttypename(
                     Declspec(SPEC_void, 0),
                     AbstractDeclarator(
                       Pointer(),
                       AbstractDeclarator(Pointer(), NULL)
                     )
                   ),
                   UnaryOperator(UOP_addr, Identifier(e->key))
                 ),
                 Sizeoftype( Casttypename(ast_spec_copy_nosc(e->spec),
                              xt_concrete_to_abstract_declarator(e->decl)) )
               ),
               CastedExpr(
                 Casttypename(
                   Declspec(SPEC_void, 0),
                   AbstractDeclarator(
                     Pointer(),
                     NULL
                   )
                 ),
                 initer ? initer : numConstant(0)
               )
             )
           )
         );
    l = l ? BlockList(l, st) : st;
    xc_decl_topointer(e->decl);           /* change to pointer */
  }

  /* A unique name for the constructor */
  gettimeofday(&ts, NULL);
  sprintf(funcname, "_shvars_%X%X_",(unsigned) ts.tv_sec,(unsigned) ts.tv_usec);

  l = FuncDef( Speclist_right(StClassSpec(SPEC_static), Declspec(SPEC_void, 0)),
               Declarator( 
                 NULL, 
                 FuncDecl( 
                   IdentifierDecl( Symbol(funcname) ),
                   ParamDecl(Declspec(SPEC_void, 0), NULL)
                 ) 
               ),            
               NULL, Compound(l));
  tail_add(verbit("#ifdef __SUNPRO_C\n"
                  "  #pragma init(%s)\n"
                  "#else \n"  /* gcc assumed */
                  "  static void __attribute__ ((constructor)) "
                  "%s(void);\n"
                  "#endif\n", funcname, funcname));
  tail_add(l);
}
