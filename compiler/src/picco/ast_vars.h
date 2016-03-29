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

/* ast_vars.h -- declare & analyze vars 
 *
 *   gtp = global threadprivate
 *   sgl = shared (non-threadprivate) global
 *   sng = shared non-global
 */

#ifndef __AST_VARS_H__
#define __AST_VARS_H__

#include "ast.h"
#include "symtab.h"

            /* Declare all variables appearing in a declaration list */
extern void ast_declare_decllist_vars(astspec s, astdecl d);
            /* Declare all vars in a varlist (i.e. in an OpenMP data clause) */
extern void ast_declare_varlist_vars(astdecl d);
            /* Declare function parameters; 
             * d must be the declarator of FUNCDEF */
extern void ast_declare_function_params(astdecl d);

            /* Gather all variables of interest (sng = shared non-global,
             * sgl = shared global, gtp = global threadprivate
             */
extern void ast_paracon_find_sng_vars(ompcon t, int xformflag);
extern void ast_taskcon_find_sngfp_vars(ompcon t, int xformflag);
extern void ast_find_gtp_vars(aststmt t, int xformflag);
extern void ast_find_sgl_vars(aststmt t, int xformflag);
extern void ast_find_allg_vars(aststmt t, int xformflag);

/* The tables with all the variables in question.
 * It only contains the names of the vars -- no info about
 * location or declaration nodes.
 */
extern symtab sng_vars, gtp_vars, sgl_vars;

#define has_gtp_vars() (gtp_vars->top != NULL)
#define has_sgl_vars() (sgl_vars->top != NULL)
#define has_sng_vars() (sng_vars->top != NULL)

/* Given a stentry: */
#define istp(e) ((e)->isthrpriv)
#define isgtp(e) (istp(e) && (e)->scopelevel == 0)
#define issgl(e) (!istp(e) && (e)->scopelevel == 0)

#endif 
