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

#ifndef __SCANNER_H__
#define __SCANNER_H__

#include <stdio.h>

extern FILE *yyin;                /* defined by flex */
extern int   yylex(void);

/* Set this to the name of the file you are about to scan */
extern void  sc_set_filename(char *fn);

extern int   __has_omp;            /* True if > 0 OMP #pragmas where found */
extern int   __has_ompix;          /* ditto for OMPi-extensions */
extern int   __has_affinitysched;  /* for affinity-scheduled loops */

extern char *sc_original_file(void);
extern int   sc_original_line(void);
extern int   sc_line(void);
extern int   sc_column(void);
 
/* Scans & stores a whole GCC attribute phrase */
extern int sc_scan_attribute(char **string);

/* Special function to start scanning by returning some given token.
 * It is only used so that the parser can support mulitple start
 * symbols.
 */
extern void sc_set_start_token(int t);

/* Force the scanner to scan from the given string */
extern void sc_scan_string(char *s);

/* Stop/resume OpenMP scanner tokens (e.g. "for" means FOR, not OPENMP_FOR) */
extern void sc_pause_openmp();
extern void sc_start_openmp();

#endif
