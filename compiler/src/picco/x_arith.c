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

/* x_arith.c -- some expression calculations */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include "symtab.h"
#include "x_arith.h"


#define IsOct(c) ((c) >= '0' && (c) <= '7')
#define IsHex(c) (((c) >= '0' && (c) <= '9') || ((c) >= 'a' && (c) <= 'f') \
                                             || ((c) >= 'A' && (c) <= 'F'))


static int atoi_oct(char *s)
{
  int num = 0;

  for (; *s != 0; s++)
    num = (num << 3) + (*s - '0');
  return (num);
}


static int atoi_hex(char *s)
{
  int num = 0;

  for (; *s != 0; s++)
    if (isdigit(*s))
      num = (num << 4) + (*s - '0');
    else
      if (*s >= 'a' && *s <= 'f')
        num = (num << 4) + (*s - 'a' + 10);
      else
        num = (num << 4)+ (*s - 'A' + 10);
  return (num);
}


/* Calculate the value of a number (integer or floating point)
 * Returns -1  on error, 1 for integer and 2 for float.
 */
static
int str2number(char *numstr, int *intval, double *fltval)
{
  int  isflp = 0, seemsoct = 0, isoct = 0, ishex = 0;
  char *s = numstr;
  
  if ( !isdigit(*s) && *s != '.')
    return (-1);
  
  if (*s == '0')      /* Check for oct/hex */
  {
    s++;
    if (isdigit(*s))
      for (seemsoct = isoct = 1; isdigit(*s) && *s; s++)
      {
        if (!IsOct(*s))
          isoct = 0;
      }
    else
      if (*s == 'x' || *s == 'X')
        for (s++, ishex = 1; IsHex(*s) && *s; s++)
          ;
  }
  else    /* Decimal */
    for ( ; isdigit(*s) && *s; s++)
      ;

  if (*s == '.')         /* float */
    for (s++,isflp=1; ((ishex && IsHex(*s)) || (!ishex && isdigit(*s))) && *s; s++)
      ;

  if (*s == 'e' || *s == 'E' ||  *s == 'p' || *s == 'P')  /* An e/p-number */
  {
    if (ishex && (*s == 'e' || *s == 'E')) return (-1);
    if (!ishex && (*s == 'p' || *s == 'P')) return (-1);
    isflp = 1;
    s++;
    if (*s == '-' || *s == '+')    /* Expect a sign */
      s++;
    for (; isdigit(*s) && *s; s++)
      ;
  }

  if (*s) return (-1);
  
  if (isflp)
  {
    if (fltval) *fltval = atof(numstr);
    return (2);
  }
  else
  {
    if (seemsoct && !isoct) return (-1);
    if (intval)
    {
      if (isoct)
        *intval = atoi_oct(numstr+1);   /* Skip 1st '0' */
      else
        if (ishex)
          *intval = atoi_hex(numstr+1);
        else
          *intval = atoi(numstr);
    }
    return (1);
  }
}


int xar_expr_is_constant(astexpr tree)
{
  switch (tree->type)
  {
    case IDENT:
      return (0);
    case CONSTVAL:
      return (1);
    case STRING: 
      return (1);
      break;         
    case FUNCCALL:
      return (0);
    case ARRAYIDX:
      return (0);
    case DOTFIELD: 
      return (0);
    case PTRFIELD: 
      return (0);
    case BRACEDINIT: 
      return (1);
    case CASTEXPR: 
      return ( xar_expr_is_constant(tree->left) );
    case CONDEXPR: 
      return ( xar_expr_is_constant(tree->u.cond) && 
               xar_expr_is_constant(tree->left) &&
               xar_expr_is_constant(tree->right) );
    case UOP:
      switch (tree->opid)
      {
        case UOP_addr:
        case UOP_star:
        case UOP_sizeof:
        case UOP_typetrick:
          return (0);
        case UOP_sizeoftype:
          return (1);
        case UOP_neg:
        case UOP_bnot:
        case UOP_lnot:
        case UOP_inc:
        case UOP_dec:
        case UOP_paren:
	  return ( xar_expr_is_constant(tree->left) );
	default:
	  return (0);
      }
    case BOP: 
      /* Here we could check if any of the operands is 0/1/etc */
      return ( xar_expr_is_constant(tree->left) && 
               xar_expr_is_constant(tree->right) );
    case PREOP: 
    case POSTOP: 
      return ( xar_expr_is_constant(tree->left) );
    case ASS: 
    case DESIGNATED: 
    case IDXDES: 
    case DOTDES: 
    case COMMALIST:
    case SPACELIST:
      return (0);
    default:
      fprintf(stderr, "[xar_expr_is_constant]: b u g !!\n");
  }
  return (0);
}


int xar_calc_int_expr(astexpr tree, int *error)
{
  switch (tree->type)
  {
    case CONSTVAL:
    {
      int v;
      if ( str2number(tree->u.str, &v, NULL) != 1) *error = 1;
      return (v);
    }
    case CASTEXPR: 
      return ( xar_calc_int_expr(tree->left, error) );
    case CONDEXPR: 
      return ( xar_calc_int_expr(tree->u.cond, error) ? 
               xar_calc_int_expr(tree->left, error) :
               xar_calc_int_expr(tree->right, error) );
    case UOP:
      switch (tree->opid)
      {
        case UOP_neg:
          return ( -xar_calc_int_expr(tree->left, error) );
        case UOP_bnot:
          return ( ~xar_calc_int_expr(tree->left, error) );
        case UOP_lnot:
          return ( !xar_calc_int_expr(tree->left, error) );
        case UOP_paren:
          return ( xar_calc_int_expr(tree->left, error) );
        case UOP_addr:
        case UOP_star:
        case UOP_sizeof:
        case UOP_typetrick:
        case UOP_sizeoftype:
        case UOP_inc:
        case UOP_dec:
        default:
          return (0);
      }
    case BOP: 
    {
      int t1 = xar_calc_int_expr(tree->left, error), 
          t2 = xar_calc_int_expr(tree->right, error);
          
      switch (tree->opid)
      {
        case BOP_shl:
          return (t1 << t2);
        case BOP_shr:
          return (t1 >> t2);
        case BOP_leq:
          return (t1 <= t2);
        case BOP_geq:
          return (t1 >= t2);
        case BOP_eqeq:
          return (t1 == t2);
        case BOP_neq:
          return (t1 != t2);
        case BOP_land:
          return (t1 && t2);
        case BOP_lor:
          return (t1 || t2);
        case BOP_band:
          return (t1 & t2);
        case BOP_bor:
          return (t1 | t2);
        case BOP_xor:
          return (t1 ^ t2);
        case BOP_add:
          return (t1 + t2);
        case BOP_sub:
          return (t1 - t2);
        case BOP_lt:
          return (t1 < t2);
        case BOP_gt:
          return (t1 > t2);
        case BOP_mul:
          return (t1 * t2);
        case BOP_div:
          return (t1 / t2);
        case BOP_mod:
          return (t1 % t2);
        case BOP_cast:
        default:
          *error = 1;
          return (0);
      }
    }
    case PREOP: 
    case POSTOP: 
    case IDENT:
    case STRING: 
    case FUNCCALL:
    case ARRAYIDX:
    case DOTFIELD: 
    case PTRFIELD: 
    case BRACEDINIT: 
    case ASS: 
    case DESIGNATED: 
    case IDXDES: 
    case DOTDES: 
    case COMMALIST:
    case SPACELIST:
    default:
      *error = 1;
      return (0);
  }
}
