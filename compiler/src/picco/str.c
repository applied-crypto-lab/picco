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

/* str.c -- A simple streaming string structure. */

#include "str.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHUNKSIZE 256

typedef struct str_ {
    char *buf; /* The buffer */
    int allox; /* Size allocated for the buffer */
    int pos;   /* Where currently the "head" is */
} *str_t;

/* We also offer a couple of scratch pads */
static struct str_ _spA = {NULL, 0, 0},
                   _spB = {NULL, 0, 0};

str strA() { return (&_spA); }
str strB() { return (&_spB); }

str Str(char *s) {
    str_t b;

    if ((b = malloc(sizeof(struct str_))) == NULL)
        return (NULL);
    b->buf = NULL;
    b->allox = 0;
    b->pos = 0;
    // (s == NULL) || str_printf(b, "%s", s);
    if (s != NULL) {
        str_printf(b, "%s", s);
    }
    return ((str)b);
}

void str_free(str s) {
    if (s) {
        str_t b = (str_t)s;
        if (b->buf != NULL)
            free(b->buf);
        if (s != &_spA && s != &_spB) /* Be careful of the bad user! */
            free(b);
    }
}

static int _strbuf_grow(str_t b, int atleast) {
    b->allox = ((atleast + CHUNKSIZE) / CHUNKSIZE) * CHUNKSIZE; /* CHUNKSIZE-aligned */
    b->buf = ((b->buf == NULL) ? malloc(b->allox) : realloc(b->buf, b->allox));
    if (b->buf != NULL)
        return (0);
    b->allox = 0;
    return (-1);
}

int str_putc(str s, int c) {
    str_t b = (str_t)s;

    if (!s)
        return (-1);

    /* Enlarge the buffer if needed */
    if (b->pos + 2 >= b->allox)
        if (_strbuf_grow(b, b->pos + 2))
            return (-1);

    sprintf(b->buf + b->pos, "%c", (char)c); /* Puts a \0 too */
    b->pos += strlen(b->buf + b->pos);       /* Normally = 1 */
    return (0);
}

/* Returns -1 on error, or # chars printed.
 */
int str_printf(str s, char *fmt, ...) {
    static char *scratchpad = NULL;
    static int scratchallox = 0;
    int n;
    va_list ap;
    str_t b = (str_t)s;

    if (fmt == NULL || b == NULL) {
        fprintf(stderr, "[str_printf]: NULL argument.\n");
        return (-1);
    }

    if (scratchpad == NULL) /* The first time it is called */
    {
        if ((scratchpad = malloc(128)) == NULL)
            return (-1);
        scratchallox = 128;
    }

    /* Print the thing to a sufficient large scratchpad */
    for (;;) {
        va_start(ap, fmt);
        n = vsnprintf(scratchpad, scratchallox, fmt, ap);
        va_end(ap);

        if (n >= 0 && n < scratchallox - 1) /* See manual pages */
            break;

        /* Double the size */
        if ((scratchpad = realloc(scratchpad, scratchallox *= 2)) == NULL) {
            scratchallox = 0;
            return (-1);
        }
    }

    /* Enlarge the buffer if needed */
    if (b->pos + n + 1 >= b->allox) /* n = # chars without the '\0' */
        if (_strbuf_grow(b, b->pos + n + 1))
            return (-1);

    strcpy(b->buf + b->pos, scratchpad);
    b->pos += n; /* Leave head @ the \0 */
    return (n);
}

/* Insert s at position pos of b, and update head accordingly
 * (i.e. if head was on or after pos, it will be shifted).
 */
int str_insert(str st, int pos, char *s) {
    str_t b = (str_t)st;
    int n;

    if (!b || !s)
        return (-1);

    n = strlen(s);
    if (b->pos + n >= b->allox)
        if (_strbuf_grow(b, b->pos + n))
            return (-1);

    if (pos > b->pos)
        pos = b->pos; /* Just in case */
    if (pos < 0)
        pos = 0; /* ditto */

    memmove(b->buf + pos + n, b->buf + pos, b->pos - pos + 1);
    memcpy(b->buf + pos, s, n); /* Not strcpy 'cause copies the \0 too! */

    return (b->pos += n);
}

char *str_string(str s) {
    return (s ? ((str_t)s)->buf : NULL);
}

int str_tell(str s) {
    return (s ? ((str_t)s)->pos : 0);
}

void str_seek(str s, int pos) {
    str_t b = (str_t)s;
    if (b)
        if (pos <= b->pos && pos >= 0)
            b->pos = pos;
}

void str_truncate(str s) {
    str_t b = (str_t)s;
    if (b && b->buf) {
        b->buf[0] = 0;
        b->pos = 0;
    }
}