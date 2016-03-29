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

/* symtab.c -- the symbol table machinery */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "symtab.h"

#define ALLSTSIZE 2459  /* Prime */

#undef TUNING

#ifdef TUNING
  static int tuneok, tus, tbst, ts;
  void show_tuning_results()
  {
    fprintf(stderr, ">>> unique symbols: %d\n", tus);
    fprintf(stderr, ">>> average bucket search steps: %lf\n",
                    ((double) tbst)/((double) ts));
    /* For a 200kbytes (7k lines) C program, there were about 2400
     * unique symbols. Best results obtained for 13669 for the hash.
     */
  }
#endif
  
void *smalloc(int size)
{
  void *m;

  if ((m = (void *) malloc(size)) == NULL)
  {
    fprintf(stderr, "[smalloc]: memory allocation failed\n");
    exit(1);
  }
  return (m);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     SYMBOL POOL                                               *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* All encountered symbols, in all scopes, are stored in this table.
 * What we want to do is get rid of the name (string) and use a "symbol"
 * pointer instead. If the same name is encounted in different scopes,
 * we return the same symbol (conserving memory). So, nothing should ever
 * be removed from this table.
 */
static symbol allsymbols[ALLSTSIZE];

unsigned int hash(char *s)
{
  unsigned int h = 0;
  for (; *s; s++) h = h*13669 + (int) (*s);  /* better than 65599 */
  return (h);
}


static symbol mksymbol(char *name, symbol next)
{
  symbol s = (symbol) smalloc( sizeof(struct symbol_) );
  s->name = strdup(name);
  s->next = next;
  return (s);
}


/* Either create a new symbol or return the already stored one */
symbol Symbol(char *name)
{
  symbol s, *bucket;

#ifdef TUNING
  ts++;
  tbst++;
#endif
  
  bucket = &( allsymbols[ hash(name) % STSIZE ] );
  for (s = *bucket; s; s = s->next)
    if (strcmp(s->name, name) == 0)     /* found it */
      return (s);
#ifdef TUNING
    else
      tbst++;
#endif

  s = mksymbol(name, *bucket);

#ifdef TUNING
  tus++;
#endif
  
  return ( *bucket = s );
}


int symbol_exists(char *name)
{
  symbol s;
  for (s = allsymbols[ hash(name) % STSIZE ]; s; s = s->next)
    if (strcmp(s->name, name) == 0)     /* found it */
      return (1);       
  return (0);
}


void symbols_allfree()
{
  symbol s, next;
  int    i;
  void   stentrypool_free();

  stentrypool_free();
  for (i = 0; i < STSIZE; i++)
    for (s = allsymbols[i]; s; s = next)
      if (s)
      {
        free(s->name);
        next = s->next;
        free(s);
      };
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     SYMBOL TABLE                                              *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* Now the symbol table.
 * It works like the allsymbols[] table, only now the hash is based
 * on the symbol's pointer, NOT the symbol's name.
 */

/* Try to optimize a bit; since stentries are created and destroyed
 * frequently, keep a pool of them so as to avoid malloc()ing and
 * free()ing.
 */
static stentry stepool = NULL;

static stentry getstentry()
{
  if (stepool)
  {
    stentry e = stepool;
    stepool = stepool->stacknext;
    return (e);
  }
  return (stentry) smalloc( sizeof(struct stentry_) );
}

static void freestentry(stentry e)
{
  e->stacknext = stepool;
  stepool = e;
}

void stentrypool_free()
{
  stentry next;
  
  for (; stepool != NULL; stepool = next)
  {
    next = stepool->stacknext;
    free(stepool);
  }
}


static
stentry Stentry(symbol s, namespace sp, stentry bnext, stentry top)
{
  stentry e = getstentry();
  e->key        = s;
  e->vval       = 0;
  e->ival       = 0;
  e->spec       = NULL;
  e->decl       = NULL;
  e->field 	= NULL; 
  e->idecl      = NULL;
  e->space      = sp;
  e->isarray    = 0;
  e->isthrpriv  = 0;
  e->bucketnext = bnext;
  e->stacknext  = top;
  return (e);
}


/* Put a symbol with its value in the table */
stentry symtab_put(symtab t, symbol s, namespace space)
{
  stentry *bucket;
  bucket = &( t->table[ ((unsigned long int) s) % STSIZE ] );
  *bucket = t->top = Stentry(s, space, *bucket, t->top);
  t->top->scopelevel = t->scopelevel;   /* current scope level */
  return (t->top);
}

stentry symtab_put_funcname(symtab t, symbol s, namespace space, astspec spec, astdecl decl){
    stentry *bucket;
    bucket = &( t->table[ ((unsigned long int) s) % STSIZE ] );
    *bucket = t->top = Stentry(s, space, *bucket, t->top);
    (*bucket)->spec = spec;
    (*bucket)->decl = decl; 
    t->top->scopelevel = t->scopelevel;   /* current scope level */
    
    return (t->top);
}
/* This twisted function inserts a symbol in the global scope, when
 * the current scope may not be the global one
 */
stentry symtab_insert_global(symtab t, symbol s, namespace space)
{
  stentry e = NULL, prev = NULL, g = NULL;
  stentry *bucket;

  if (t->scopelevel == 0 || t->top == NULL) 
     return ( symtab_put(t, s, space) );  /* Trivial case */
  
  /* Put in the bucket */
  bucket = &( t->table[ ((unsigned long int) s) % STSIZE ] );   /* Our bucket */
  if (*bucket == NULL)       /* It's empty */
    *bucket = g = Stentry(s, space, NULL, NULL);  /* we'll fix the stack */
  else
  {  
    /* Walk the bucket till we hit a global-scope entry (e) */
    for (e = *bucket, prev = NULL; e; prev = e, e = e->bucketnext)
      if (e->scopelevel == 0)    /* ok! got one */
      {
        e->bucketnext = g = Stentry(s, space, e->bucketnext, NULL);
        break;
      };
    if (e == NULL)   /* No global-scope entry found; put it @ end of bucket */
      prev->bucketnext = g = Stentry(s, space, NULL, NULL);
  }

  /* If e != NULL then we have found a handy global-scope entry,
   * otherwise not. In any case, g is our new entry, correctly placed
   * in its bucket; insert it now in the stack.
   */
  if (e == NULL)    /* We have no other solution: walk the stack down */
    for (e = t->top; e; e = e->stacknext)
      if (e->scopelevel == 0)        /* ok */
        break;
  assert(e != NULL);
  g->stacknext = e->stacknext;
  e->stacknext = g;  
  return (g);
}


/* Get & remove a symbol (NULL if not found), which belongs to
 * namespace p.
 */
stentry symtab_remove(symtab t, symbol s, namespace p)
{
  stentry e, f = NULL, g;
  
  for (e = t->table[((unsigned long int) s) % STSIZE]; e; f=e, e=e->bucketnext)
    if (e->key == s && e->space == p)     /* found it */
    {
      /* Get it off the bucket */
      if (f == NULL)     /* Head of bucket list */
        t->table[ ((unsigned long int) s) % STSIZE ] = e->bucketnext;
      else
        f->bucketnext = e->bucketnext;
        
      /* Get it off the stack: this is tuff! We have to scan the whole stack */
      for (g = t->top, f = NULL; g; f = g, g = g->stacknext)
        if (g == e)
        {
          if (f == NULL)  /* Top element */
            t->top = g->stacknext;
          else
            f->stacknext = g->stacknext;
        };
      assert( g != NULL );
      
      return (e);
    };
  return (NULL);
}


/* Notice that we return the most recent entry, not necessarily in
 * the current scope.
 */
stentry symtab_get(symtab t, symbol s, namespace p)
{
  stentry e;
  for (e = t->table[ ((unsigned long int) s) % STSIZE ]; e; e = e->bucketnext)
    if (e->key == s && e->space == p)     /* found it */
      return (e);       
  return (NULL);
}


/* An empty table */
symtab Symtab()
{
  symtab t = smalloc( sizeof(struct symtab_) );
  int    i;
  
#ifdef TUNING
  if (!tuneok) { tuneok = 1; atexit(show_tuning_results); }
#endif
  
  for (i = 0; i < STSIZE; i++)
    t->table[i] = NULL;
  t->top = NULL;
  t->scopelevel = 0;
  return (t);
}


/* Empty out a symbol table */
void symtab_drain(symtab t)
{
  stentry e;
  for (; t->top; )
  {
    e = t->top;
    t->top = e->stacknext;   /* Get it off the stack */
    /* Get the bucket it was in, and remove it from there too. */
    t->table[ ((unsigned long int) (e->key)) % STSIZE ] = e->bucketnext;
    freestentry(e);
  }
  t->scopelevel = 0;
}


static struct symbol_ scopemark = { "@scopper@", NULL };


/* Enter a new scope: push a special mark. */
void scope_start(symtab t)
{
  symtab_put(t, &scopemark, IDNAME);
  (t->scopelevel)++;
}


/* Pop (and free) all symbols till the special one */
void scope_end(symtab t)
{
  stentry e, *b;
  for (; t->top; )
  {
    e = t->top;
    t->top = e->stacknext;   /* Get it off the stack */

    /* Get the bucket it was in, and remove it from there too. */
    b = &( t->table[ ((unsigned long int) (e->key)) % STSIZE ] );
    *b = e->bucketnext;

    if (e->key != &scopemark)
      freestentry(e);
    else
    {
      freestentry(e);
      (t->scopelevel)--;
      break; /* Done */ 
    }
  }
}


void scope_show(symtab t)
{
  stentry e;
  int     i = 0;
  char    *space[8] = { NULL, "IDNAME", "TYPENAME", "SUNAME", "ENUMNAME", 
                        "LABELNAME", "FUNCNAME", "SUFIELDNAME" };

  printf("Current scope (%d):\n--------------\n", t->scopelevel);
  for (e = t->top; e; e = e->stacknext)
  {
    if (e->key != &scopemark)
      printf("%*s%s (%s)%s\n", 2*(i+1), " ", e->key->name, space[e->space],
             e->isarray ? " (array)" : " ");
    else
    {
      printf("%*sScope -%d\n%*s----------\n", 2*(i+1), " ", i+1, 2*(i+1), " ");
      i++;
    }
  }
}
