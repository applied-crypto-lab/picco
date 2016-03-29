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

/* ast_copy.h -- create a copy of an AST */

#ifndef __AST_COPY_H__
#define __AST_COPY_H__

#include "ast.h"

#define ast_copy(t) ast_stmt_copy(t)
extern astexpr   ast_expr_copy(astexpr tree);
extern aststmt   ast_stmt_copy(aststmt stmt);
extern astdecl   ast_decl_copy(astdecl tree);
extern astspec   ast_spec_copy(astspec tree);
extern ompcon    ast_ompcon_copy(ompcon tree);
extern ompdir    ast_ompdir_copy(ompdir tree);
extern ompclause ast_ompclause_copy(ompclause tree);

/* Special versions that discard storage class specifiers; the first one
 * adds an "int" if what remains is empty.
 */
extern astspec ast_spec_copy_nosc(astspec tree);
extern astspec ast_spec_copy_nosc_asis(astspec tree);

/* OMPi-extensions
 */
extern oxclause ast_oxclause_copy(oxclause tree);
extern oxdir    ast_oxdir_copy(oxdir tree);
extern oxcon    ast_oxcon_copy(oxcon tree);

#endif
