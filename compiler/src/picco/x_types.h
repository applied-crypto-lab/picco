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

/* x_types.h */

#ifndef __X_TYPES_H__
#define __X_TYPES_H__

#include "ast.h"

extern void xt_barebones_substitute(astspec *spec, astdecl *decl);
extern void xt_barebones_decl(astdecl d);
extern void xt_declaration_xform(aststmt *t);
extern void xt_free_retired();
extern int  xt_decl_depends_on_sue(astdecl decl);
extern int  xt_spec_depends_on_sue(astspec spec);
extern int  xt_symbol_depends_on_sue(symbol s);
#define xt_has_sue(s,d) (xt_spec_depends_on_sue(s) || xt_decl_depends_on_sue(d))
extern void xt_dlist_array2pointer(aststmt d);
extern void xt_decl_array2pointer(astdecl d);

extern astdecl xt_concrete_to_abstract_declarator(astdecl orig);

#endif
