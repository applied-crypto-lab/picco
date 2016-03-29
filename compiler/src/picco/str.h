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

/* str.h -- A simple streaming string structure. */

/* A dynamically growing string structure with a simple file-like interface. 
 * You can keep str_printf()'ing to it oblivioulsy, you can seek/truncate etc.
 * You str_free() it at the end.
 * You can always get the actual string by str_string(). 
 * Take care NOT to touch the returned thing.
 */

#ifndef __STR_H__
#define __STR_H__

#ifndef NULL
#  include <stdio.h>
#endif

typedef void *str;	       

#define Strnew() Str(NULL)     /* An empty str */
extern str  Str(char *s);      /* An str initialized to s */

extern void str_free(str s);
extern int  str_putc(str s, int c);
extern int  str_printf(str s, char *fmt, ...);
extern int  str_insert(str s, int pos, char *t);
extern char *str_string(str s);        /* Get the actual string */
extern int  str_tell(str s);           /* Where the "head" currently is */
extern void str_seek(str s, int pos);  /* Put the "head" into an absolute pos */
extern void str_truncate(str s);       /* Zero-out the string */

/* Two scrathcpad strings for everyday use; use at your own risk! */
extern str strA();
extern str strB();
#define A_str_putc(c)       str_putc(strA(),c)
#define A_str_insert(pos,t) str_insert(strA(),pos,t)
#define A_str_string()      str_string(strA())
#define A_str_tell()        str_tell(strA())
#define A_str_seek(pos)     str_seek(strA(),pos)
#define A_str_truncate()    str_truncate(strA())
#define B_str_putc(c)       str_putc(strB(),c)
#define B_str_insert(pos,t) str_insert(strB(),pos,t)
#define B_str_string()      str_string(strB())
#define B_str_tell()        str_tell(strB())
#define B_str_seek(pos)     str_seek(strB(),pos)
#define B_str_truncate()    str_truncate(strB())

#endif
