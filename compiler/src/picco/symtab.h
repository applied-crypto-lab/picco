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


/* symtab.h */
#include "ast.h"

#ifndef __SYMTAB_H__
#define __SYMTAB_H__

typedef struct symbol_ 
        { 
          char *name; 
          struct symbol_ *next;
          int type; // int: 0 and float: 1 and struct: 2
          int isptr;  // not = 0; ptr = 1;
          astspec struct_type; //store the type of struct 
	} *symbol;

/*
 * Symbols
 */

extern void *smalloc(int size); /* malloc() with failure check */

/* Either create a new symbol or return the already stored one. 
 * In the former case, the "name" is strdup()'ed 
 */
extern symbol Symbol(char *name);
extern int    symbol_exists(char *name);
extern void   symbols_allfree();


/* 
 * Symbol tables
 */
#define STSIZE 1031  /* Prime */

typedef enum { IDNAME = 1, TYPENAME, SUNAME, ENUMNAME, LABELNAME, FUNCNAME}
     namespace;

typedef struct stentry_ *stentry;
struct stentry_ { symbol    key;          /* The symbol */
                  int       ival;         /* Two ints to put anything */
                  int       vval;
                  
                  astspec   spec;         /* The specifier */
                  astdecl   decl;         /* The bare declarator */
                  astdecl   idecl;        /* initdeclarator (includes decl) */
		  astdecl   field;          /* field of struct or union*/	
                  namespace space;        /* 1 table for all spaces */
                  int       isarray;      /* Non-scalar */
                  int       isthrpriv;    /* 1 if it is a threadprivate var */
                  int       scopelevel;   /* The scope it was declared in */
                  stentry   bucketnext;   /* for the bucket */
                  stentry   stacknext;    /* for the scope stack */
                };

typedef struct symtab_ { stentry table[STSIZE];
                         stentry top;     /* Most recent in scope */
                         int     scopelevel;   /* Current scope level */
                       } *symtab;


extern stentry symtab_get(symtab t, symbol s, namespace space);
extern stentry symtab_put(symtab t, symbol s, namespace space);
extern stentry symtab_put_funcname(symtab t, symbol s, namespace space, astspec spec, astdecl decl);

extern stentry symtab_insert_global(symtab t, symbol s, namespace space);
extern stentry symtab_remove(symtab t, symbol s, namespace p);
extern symtab  Symtab(); /* An empty table */
extern void    symtab_drain(symtab t);   /* Empty out a symbol table */
extern void    scope_start(symtab t);
extern void    scope_end(symtab t);
extern void    scope_show(symtab t);

/* Assume we check for identifier, not type name */
#define isGlobal(t,s) ( symtab_get(t,s,IDNAME)->scopelevel == 0 )

#endif
