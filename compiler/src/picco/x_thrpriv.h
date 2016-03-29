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

/* x_thrpriv.h */

#ifndef __X_THRPRIV_H__
#define __X_THRPRIV_H__

#include "ast.h"

extern void xform_threadprivate(aststmt *t);

/* Takes a compound (specifically the body of a function, but any other
 * compound can do), discovers all global threadprivate (gtp) vars used
 * and declares pointers to those @ the top of the compound.
 */
extern void tp_fix_funcbody_gtpvars(aststmt t);

/*
 * The following 2 are only used in x_parallel.c
 */

 
/* They return the name of 
 * (a) the key associated with a tp var (global or not), given its "stab" entry
 * (b) the new (altered) name of the original var
 */
extern symbol tp_key_name(stentry e);
extern symbol tp_new_name(symbol var);

/* Declares and initializes a pointer to a threadprivate var */
extern aststmt tp_declaration(stentry e, symbol newvar,
                              astexpr base, int baseisptr);

#endif
