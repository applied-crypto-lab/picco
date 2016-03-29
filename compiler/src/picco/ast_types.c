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

/* AST types - utilities regarding types */

#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include "ast_types.h"
#include <stdio.h>
#include "ast_show.h"


/* Checks whether the speclist includes "long long" (2 long in a row) */
#define speclist_has_longlong(s) _two_longs(s,0)

static 
int _two_longs(astspec s, int one_long)
{
  if (s == NULL) return (0);
  if (s->type == SPECLIST)
  {
    if (speclist_getspec(s->body, SPEC, SPEC_long) == NULL) 
      return ( _two_longs(s->u.next, 0) );
    else  /* Found 1 instance of long */
      return ( one_long ? 1 : _two_longs(s->u.next, 1) );
  }
  return ( (s->type != SPEC || s->subtype != SPEC_long) ? 0 : one_long );
}


nt_size speclist_size(astspec s)
{
  if (speclist_has_longlong(s)) return (LONGLONG);
  if (speclist_getspec(s, SPEC, SPEC_long) != NULL) return (LONG);
  if (speclist_getspec(s, SPEC, SPEC_short) != NULL) return (SHORT);
  return (NORMAL);
}


nt_sign speclist_sign(astspec s)
{
  if (speclist_getspec(s, SPEC, SPEC_unsigned) != NULL) return (UNSIGNED);
  return (SIGNED);
}


nt_basetype speclist_basetype(astspec s)
{
  
  if (speclist_getspec(s, SPEC, SPEC_char) != NULL) return (CHAR);
  if (speclist_getspec(s, SPEC, SPEC_float) != NULL) return (FLOAT);
  if (speclist_getspec(s, SPEC, SPEC_double) != NULL) return (DOUBLE);
  if (speclist_getspec(s, SPEC, SPEC_ubool) != NULL) return (UBOOL);
  return (INT);
}

