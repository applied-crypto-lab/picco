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

/* ast_renlabs.c -- renames labels */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ast_renlabs.h"


void ast_ompcon_renlabs(ompcon t);
void ast_oxcon_renlabs(oxcon t);


#define MAXLABS 1024
static int _newlabs;
static char _ln[128];
symbol _renamed[MAXLABS];

static
symbol new_label_name(symbol oldlabel)
{
  int i;
  
  if (_newlabs == MAXLABS)
  {
    fprintf(stderr, "[new_label_name]: too many labels !!\n");
    return (oldlabel);
  }
  for (i = 0; i < _newlabs; i++)
    if (_renamed[i] == oldlabel)   /* done that */
      break;
  if (i == _newlabs)               /* first time seen */
    _renamed[ _newlabs++ ] = oldlabel;
  
  sprintf(_ln, "%s__%d_", _renamed[i]->name, i);
  return ( Symbol(_ln) );
}


void ast_stmt_jump_renlabs(aststmt t)
{
  switch (t->subtype)
  {
    case SGOTO:
      t->u.label = new_label_name(t->u.label);
    case SBREAK:
    case SRETURN:
    case SCONTINUE:
      break;
    default:
      fprintf(stderr, "[ast_stmt_jump_renlabs]: b u g !!\n");
  }
}


void ast_stmt_iteration_renlabs(aststmt t)
{
  switch (t->subtype)
  {
    case SFOR:
    case SWHILE:
    case SDO:
      ast_stmt_renlabs(t->body);
      break;
    default:
      fprintf(stderr, "[ast_stmt_iteration_renlabs]: b u g !!\n");
  }
}


void ast_stmt_selection_renlabs(aststmt t)
{
  switch (t->subtype)
  {
    case SSWITCH:
      ast_stmt_renlabs(t->body);
      break;
    case SIF:
      ast_stmt_renlabs(t->body);
      if (t->u.selection.elsebody)
        ast_stmt_renlabs(t->u.selection.elsebody);
      break;
    default:
      fprintf(stderr, "[ast_stmt_selection_renlabs]: b u g !!\n");
  }
}


void ast_stmt_labeled_renlabs(aststmt t)
{
  switch (t->subtype)
  {
    case SLABEL:
      t->u.label = new_label_name(t->u.label);
    case SCASE:
    case SDEFAULT:
      break;
    default:
      fprintf(stderr, "[ast_stmt_labeled_renlabs]: b u g !!\n");
  }
  ast_stmt_renlabs(t->body);
}


void ast_stmt_renlabs(aststmt t)
{
  switch (t->type)
  {
    case JUMP:
      ast_stmt_jump_renlabs(t);
      break;
    case ITERATION:
      ast_stmt_iteration_renlabs(t);
      break;
    case SELECTION:
      ast_stmt_selection_renlabs(t);
      break;
    case LABELED:
      ast_stmt_labeled_renlabs(t);
      break;
    case EXPRESSION:
      break;
    case COMPOUND:
      if (t->body)
        ast_stmt_renlabs(t->body);
      break;
    case STATEMENTLIST:
      ast_stmt_renlabs(t->u.next);
      ast_stmt_renlabs(t->body);
      break;
    case DECLARATION:
      break;
    case FUNCDEF:
      ast_stmt_renlabs(t->body);    /* always non-NULL */
      break;
    case OMPSTMT:
      ast_ompcon_renlabs(t->u.omp);
      break;
    case VERBATIM:
      break;
    case OX_STMT:
      ast_oxcon_renlabs(t->u.ox);
      break;
    default:
      fprintf(stderr, "[ast_stmt_renlabs]: b u g !!\n");
  }
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OpenMP NODES                                              *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


void ast_ompcon_renlabs(ompcon t)
{
  if (t->body)     /* barrier & flush don't have a body */
    ast_stmt_renlabs(t->body);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OMPi-EXTENSION NODES                                      *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


void ast_oxcon_renlabs(oxcon t)
{
  if (t->body)
    ast_stmt_renlabs(t->body);
}
