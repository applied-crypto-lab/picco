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

D     [0-9]
L     [a-zA-Z_]
H     [a-fA-F0-9]
E     [Ee][+-]?{D}+
FS    (f|F|l|L)
IS    (u|U|l|L)*

%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "picco.h"
#include "ast.h"            /* For the yylval / ast types */
#include "symtab.h"
#include "parser.h"
#include "scanner.h"

#define SCANNER_STR_SIZES 1024  /* All strings declared here are that long */

static int  on_omp_line = 0,    /* Scanning an OpenMP #pragma line */
            on_ompix_line = 0;  /* Scanning an OMPi-extension #pragma line */
int         __has_omp = 0,      /* True if we found at least 1 OpenMP #pragma */
            __has_ompix = 0,
            __has_affinitysched = 0;

void count(), gobbleline(), sharp(), gobblecomment();

#define IdentOrType() \
  ( symtab_get(stab,Symbol(yytext),TYPENAME) != NULL ? TYPENAME : IDENTIFIER )

/* The following variables are used for tracking the original filenames and
 * original line numbers, because our scanner/parser uses preprocessed
 * files. To do this, we parse the sharps produced by the preprocessor,
 * which mark the line number of the original file, in the of the form:
 *   # 1 "test.c"
 */
static char origfile_name[SCANNER_STR_SIZES];  // original file name
static char thisfile_name[SCANNER_STR_SIZES];  // the scanned file name
static int  thisfile_line = 1, // Current line in our file (preprocessed)
            marker_line = 0,   // The line where the last marker was found
            origfile_line = 0; // Original file line the marker was refering to
static int  thisfile_column = 0;  // Column in the currently scanned line
static int  start_token = 0;   // For starting with a particular token

%}

%%

%{
     /* Trick to get an initial token (from the bison manual);
      * This is placed in the top of the produced yylex() function.
      */
     if (start_token)
     {
       int t = start_token;
       start_token = 0;         /* Don't do it again */
       return t;
     }
%}


[ \t]*"#"[ \t]*"pragma"[ \t]+"ompix"[ \t]+  {
    count();
    on_ompix_line = __has_ompix = 1;
    return (PRAGMA_OMPIX);
}

[ \t]*"#"[ \t]*"pragma"[ \t]+"omp"[ \t]+  {
    count();
    on_omp_line = __has_omp = 1;
    return (PRAGMA_OMP);
}
[ \t]*"#"[ \t]*"pragma"[ \t]+"omp"[ \t]+"threadprivate"[ \t]*  {
    count();
    on_omp_line = __has_omp = 1;
    return (PRAGMA_OMP_THREADPRIVATE);
}
[ \t]*"#"[ \t]*"line"  {
    sharp();
}
[ \t]*"#"              {
    sharp();
}
"parallel"             {
    count();
    if (on_omp_line)
    return(OMP_PARALLEL);
    else
    return IdentOrType();
}
"sections"             {
    count();
    if (on_omp_line)
    return(OMP_SECTIONS);
    else
    return IdentOrType();
}
"nowait"               {
    count();
    if (on_omp_line)
    return(OMP_NOWAIT);
    else
    return IdentOrType();
}
"ordered"              {
    count();
    if (on_omp_line)
    return(OMP_ORDERED);
    else
    return IdentOrType();
}
"schedule"             {
    count();
    if (on_omp_line)
    return(OMP_SCHEDULE);
    else
    return IdentOrType();
}
"dynamic"              {
    count();
    if (on_omp_line)
    return(OMP_DYNAMIC);
    else
    return IdentOrType();
}
"guided"               {
    count();
    if (on_omp_line)
    return(OMP_GUIDED);
    else
    return IdentOrType();
}
"runtime"              {
    count();
    if (on_omp_line)
    return(OMP_RUNTIME);
    else
    return IdentOrType();
}

"auto"                 {
    count();
    if (on_omp_line)
    return(OMP_AUTO);
    //else
    //return(AUTO);
}

"affinity"             { 
    count();
    if (on_omp_line)
    {
        __has_affinitysched = 1;
        return(OMP_AFFINITY);
    }
    else
    return IdentOrType();
}
"section"              {
    count();
    if (on_omp_line)
    return(OMP_SECTION);
    else
    return IdentOrType();
}
"single"               {
    count();
    if (on_omp_line)
    return(OMP_SINGLE);
    else
    return IdentOrType();
}
"master"               {
    count();
    if (on_omp_line)
    return(OMP_MASTER);
    else
    return IdentOrType();
}
"critical"             {
    count();
    if (on_omp_line)
    return(OMP_CRITICAL);
    else
    return IdentOrType();
}
"barrier"              {
    count();
    if (on_omp_line)
    return(OMP_BARRIER);
    else
    return IdentOrType();
}
"atomic"               {
    count();
    if (on_omp_line)
    return(OMP_ATOMIC);
    else
    return IdentOrType();
}
"flush"                {
    count();
    if (on_omp_line)
    return(OMP_FLUSH);
    else
    return IdentOrType();
}

"priv"              {
    count();
    if (on_omp_line)
    return(OMP_PRIVATE);
    else
    return IdentOrType();
}
"firstprivate"         {
    count();
    if (on_omp_line)
    return(OMP_FIRSTPRIVATE);
    else
    return IdentOrType();
}
"lastprivate"          {
    count();
    if (on_omp_line)
    return(OMP_LASTPRIVATE);
    else
    return IdentOrType();
}
"shared"               {
    count();
    if (on_omp_line)
    return(OMP_SHARED);
    else
    return IdentOrType();
}
"none"                 {
    count();
    if (on_omp_line)
    return(OMP_NONE);
    else
    return IdentOrType();
}
"reduction"            {
    count();
    if (on_omp_line || on_ompix_line)
    return(OMP_REDUCTION);
    else return IdentOrType();
}
"copyin"               {
    count();
    if (on_omp_line)
    return(OMP_COPYIN);
    else
    return IdentOrType();
}
"num_threads"          {
    count();
    if (on_omp_line)
    return(OMP_NUMTHREADS);
    else
    return IdentOrType();
}
"copyprivate"          {
    count();
    if (on_omp_line)
    return(OMP_COPYPRIVATE);
    else
    return IdentOrType();
}


"task"                 {
    count();
    if (on_omp_line || on_ompix_line)
    return(OMP_TASK);
    else
    return IdentOrType();
}
"untied"               {
    count();
    if (on_omp_line || on_ompix_line)
    return(OMP_UNTIED);
    else
    return IdentOrType();
}
"taskwait"             {
    count();
    if (on_omp_line)
    return(OMP_TASKWAIT);
    else
    return IdentOrType();
}
"collapse"             {
    count();
    if (on_omp_line)
    return(OMP_COLLAPSE);
    else
    return IdentOrType();
}

"final"                {
    count();
    if (on_omp_line)
    return(OMP_FINAL);
    else
    return IdentOrType();
}
"mergeable"            {
    count();
    if (on_omp_line)
    return(OMP_MERGEABLE);
    else
    return IdentOrType();
}
"taskyield"            {
    count();
    if (on_omp_line)
    return(OMP_TASKYIELD);
    else
    return IdentOrType();
}
"read"                 {
    count();
    if (on_omp_line)
    return(OMP_READ);
    else
    return IdentOrType();
}
"write"                {
    count();
    if (on_omp_line)
    return(OMP_WRITE);
    else
    return IdentOrType();
}
"capture"              {
    count();
    if (on_omp_line)
    return(OMP_CAPTURE);
    else
    return IdentOrType();
}
"update"               {
    count();
    if (on_omp_line)
    return(OMP_UPDATE);
    else
    return IdentOrType();
}
"min"                  {
    count();
    if (on_omp_line)
    return(OMP_MIN);
    else
    return IdentOrType();
}
"max"                  {
    count();
    if (on_omp_line)
    return(OMP_MAX);
    else
    return IdentOrType();
}


"taskdef"              {
    count();
    if (on_ompix_line)
    return(OMPIX_TASKDEF);
    else
    return IdentOrType();
}
"uponreturn"           {
    count();
    if (on_ompix_line)
    return(OMPIX_UPONRETURN);
    else
    return IdentOrType();
}
"tasksync"             {
    count();
    if (on_ompix_line)
    return(OMPIX_TASKSYNC);
    else
    return IdentOrType();
}
"in"|"IN"|"In"        {
    count();
    if (on_ompix_line)
    return(OMPIX_IN);
    else
    return IdentOrType();
}
"out"|"OUT"|"Out"      {
    count();
    if (on_ompix_line)
    return(OMPIX_OUT);
    else
    return IdentOrType();
}
"inout"|"INOUT"|"Inout"|"InOut" {
    count();
    if (on_ompix_line)
    return(OMPIX_INOUT);
    else
    return IdentOrType();
}
"atnode"|"ATNODE"|"Atnode"|"AtNode" {
    count();
    if (on_ompix_line)
    return(OMPIX_ATNODE);
    else
    return IdentOrType();
}
"atworker"|"ATWORKER"|"Atworker"|"AtWorker" {
    count();
    if (on_ompix_line)
    return(OMPIX_ATWORKER);
    else
    return IdentOrType();
}
"taskschedule"         {
    count();
    if (on_ompix_line)
    return(OMPIX_TASKSCHEDULE);
    else
    return IdentOrType();
}
"stride"               {
    count();
    if (on_ompix_line)
    return(OMPIX_STRIDE);
    else
    return IdentOrType();
}
"start"                {
    count();
    if (on_ompix_line)
    return(OMPIX_START);
    else
    return IdentOrType();
}
"scope"                {
    count();
    if (on_ompix_line)
    return(OMPIX_SCOPE);
    else
    return IdentOrType();
}
"nodes"                 {
    count();
    if (on_ompix_line)
    return(OMPIX_NODES);
    else
    return IdentOrType();
}
"workers"               {
    count();
    if (on_ompix_line)
    return(OMPIX_WORKERS);
    else
    return IdentOrType();
}
"local"                {
    count();
    if (on_ompix_line)
    return(OMPIX_LOCAL);
    else
    return IdentOrType();
}
"global"               {
    count();
    if (on_ompix_line)
    return(OMPIX_GLOBAL);
    else
    return IdentOrType();
}
"tied"                 {
    count();
    if (on_ompix_line)
    return(OMPIX_TIED);
    else
    return IdentOrType();
}
"detached"|"DETACHED"|"Detached" {
    count();
    if (on_ompix_line)
    return(OMPIX_DETACHED);
    else
    return IdentOrType();
}


"//"                   { gobbleline(); }
"/*"                   { gobblecomment(); }


"int"                  { count(); return(INT); }
"float"                  { count(); return(FLOAT); }
"void"                 { count(); return(VOID); }
"return"               { count(); return(RETURN); }
"private"              { count(); return(PRIVATE); }
"public"               { count(); return(PUBLIC);}

"break"                { count(); return(BREAK);}
"continue"             { count(); return(CONTINUE);}
"do"                   { count(); return(DO);}
"else"                 { count(); return(ELSE); }
"for"                  {
                         count();
                         if (on_omp_line)
                            return (OMP_FOR);
                         else
                            return(FOR);
                       }
"static"               {
                         count();
                         if (on_omp_line)
                            return(OMP_STATIC);
                       }
"if"                   { count();return(IF);}
"while"                { count(); return(WHILE); }
"smcinput"             { count(); return(SMCINPUT); }
"smcoutput"            { count(); return(SMCOUTPUT); }
"struct"	       { count(); return(STRUCT); }
"union"		       { count(); return(UNION); }
"typedef"	       { count(); return(TYPEDEF);}
"sizeof"	       { count(); return(SIZEOF);}
"pmalloc"              { count(); return(PMALLOC);} 
"pfree"                { count(); return(PFREE);}


  /*
   * Identifiers, constants and strings
   */

{L}({L}|{D})*          { count(); return(IDENTIFIER); }
0[xX]{H}+{IS}?         { count(); return(CONSTANT); }
0{D}+{IS}?             { count(); return(CONSTANT); }
{D}+{IS}?              { count(); return(CONSTANT); }
'(\\.|[^\\'])+'        { count(); return(CONSTANT); }

{D}+{E}{FS}?           { count(); return(CONSTANT); }
{D}*"."{D}+({E})?{FS}? { count(); return(CONSTANT); }
{D}+"."{D}*({E})?{FS}? { count(); return(CONSTANT); }
\"(\\.|[^\\"])*\"      { count(); return(STRING_LITERAL); }

  /*
   * Operators
   */
"+="                   { count(); return(ADD_ASSIGN); }
"-="                   { count(); return(SUB_ASSIGN); }
"*="                   { count(); return(MUL_ASSIGN); }
"/="                   { count(); return(DIV_ASSIGN); }
"%="                   { count(); return(MOD_ASSIGN); }
"->"		       { count(); return(PTR_OP); }
"++"                   { count(); return(INC_OP); }
"--"                   { count(); return(DEC_OP); }
"&&"                   { count(); return(AND_OP); }
"||"                   { count(); return(OR_OP); }
"<="                   { count(); return(LE_OP); }
">="                   { count(); return(GE_OP); }
"=="                   { count(); return(EQ_OP); }
"!="                   { count(); return(NE_OP); }
">>"                   { count(); return(RIGHT_OP); }
"<<"                   { count(); return(LEFT_OP);}
"@"                    { count(); return('@');}
"<"                    { count(); return('<'); }
">"                    { count(); return('>'); }

";"                    { count(); return(';'); }
"{"                    { count(); return('{'); }
"}"                    { count(); return('}'); }
","                    { count(); return(','); }
":"                    { count(); return(':'); }
"="                    { count(); return('='); }
"("                    { count(); return('('); }
")"                    { count(); return(')'); }
"["                    { count(); return('['); }
"]"                    { count(); return(']'); }
"."		       { count(); return('.'); }
"-"                    { count(); return('-'); }
"+"                    { count(); return('+'); }
"*"                    { count(); return('*'); }
"/"                    { count(); return('/'); }
"%"                    { count(); return('%'); }
"!"                    { count(); return('!'); }
"^"                    { count(); return('^'); }
"|"                    { count(); return('|'); }
"&"                    { count(); return('&'); }
  /*
   * Spaces, newlines etc.
   */

[ \t\v\f]              { count(); }
\n                     {
                         count();
                         if (on_omp_line || on_ompix_line)
                         {
                           on_omp_line = on_ompix_line = 0;  /* Line finished */
                           return('\n');
                         }
                       }
.                      { /* ignore bad characters */ }
%%


/* Notice that in the following we make use of input() and unput().
 * Those destroy yytext[] but we don't care what happens to it
 * at those points in the code.
 */


int yywrap()
{
  return(1);
}


/* Called upon encountering a line starting with '#' which does not
 * contain an OpenMP pragma.
 * We actually throw this line away but before doing so we check if
 * it conveys line number information from the original file. This
 * must be of the form:
 *    # <number> "<filename>"
 */
void sharp()
{
    char c, line[SCANNER_STR_SIZES] = {0}, *s, *t;
    int  lineno = -1;
    
    for (s = line; (c = input()) != '\n' && c != 0; )   /* Read in the line */
    if (s-line < SCANNER_STR_SIZES-2)
    *s++ = c;
    
    if (c == 0) return;          /* End of file - ignore anyways */
    *s = 0;                      /* Ignore \n and force end of string */
    thisfile_line++;             /* Update counters */
    thisfile_column = 0;
    
    if ( sscanf(line, "%d", &lineno) < 0 ) return;   /* Nope -- no line number */
    
    for (s = line; *s != '\"' && *s != 0; s++)       /* Find the " */
    ;
    if (*s == 0) return;                             /* Nope -- no file name */
    
    for (t = (++s); *s != '\"' && *s != 0; s++)      /* Find the next " */
    ;
    if (t == s) return;                              /* Nope -- empty file name */
    *s = 0;
    strncpy(origfile_name, t, SCANNER_STR_SIZES-1);
    origfile_line = lineno;
    marker_line   = thisfile_line;
}


/* Update line/column counters & generate the thingy to return to the parser */
void count()
{
  int i, nonempty = 0;

  for (i = 0; yytext[i] != 0; i++)
    if (yytext[i] == '\n')
    {
      thisfile_column = 0;
      thisfile_line++;
    }
    else
      if (yytext[i] == '\t')
        thisfile_column += ( 8 - (thisfile_column % 8) );
      else
      {
        thisfile_column++;
        if (!isspace(yytext[i]))
          nonempty = 1;
      };

  if (nonempty)
    strcpy(yylval.name, yytext);
}

/* Gobble till end of line (i.e. ignore C++ // comments)
 */
void gobbleline()
{
    char c;
    for ( ; (c = input()) != '\n' && c != 0; )   /* Read in the line */
    ;
    thisfile_line++;             /* Update counters */
    thisfile_column = 0;
}


/* Gobble spaces (returns the non-space char found)
 */
char gobblespaces()
{
    char c;
    for ( ; (c = input()) != 0 && isspace(c); )
    if (c == '\n')
    {
        thisfile_line++;
        thisfile_column = 0;
    }
    else
    thisfile_column++;
    if (c)
    thisfile_column++;
    return (c);
}


/* Gobble till the end of comment (i.e. ignore C comments)
 * We actually don't need to recognize comments, since we scan a
 * file that has already been preprocessed. However, we need this
 * functionality for testing purposes.
 * The code is a bit dump, i.e. it won't handle nested comments,
 * and it won't check for strings.
 */
void gobblecomment()
{
    char c, done = 0;
    
    /* Never put the !done *after* the (c = input()) != 0. Order matters!! */
    for ( ; !done && (c = input()) != 0; )
    {
        if (c == '\n')
        {
            thisfile_line++;               /* Update counters */
            thisfile_column = 0;
            continue;
        }
        thisfile_column++;
        if (c == '*')
        {
            while ((c = input()) == '*')
            thisfile_column++;
            if (c == '\n')
            {
                thisfile_line++;             /* Update counters */
                thisfile_column = 0;
            }
            else
            {
                thisfile_column++;
                if (c == '/')
                done = 1;
            }
        }
    }
}


/* Gobble (but store) till the end of an attribute phrase ((...)).
 * The code is a bit dump, i.e. it won't handle strings with "))" in them.
 */
int sc_scan_attribute(char **string)
{
    char c, done = 0, text[SCANNER_STR_SIZES];
    int  n;
    
    c = gobblespaces();
    if (c != '(' || ((c = input()) != '(')) return (-1);
    thisfile_column++;
    text[0] = text[1] = '(';
    
    /* Never put the !done *after* the (c = input()) != 0. Order matters!! */
    for (n = 2; !done && (c = input()) != 0 && n < SCANNER_STR_SIZES; )
    {
        text[n++] = c;
        if (c == '\n')
        {
            thisfile_line++;               /* Update counters */
            thisfile_column = 0;
            continue;
        }
        thisfile_column++;
        if (c == ')')
        {
            if ((c = input()) == ')')
            done = 1;
            text[n++] = c;
            if (done)
            {
                text[n++] = 0;
                *string = strdup(text);
            }
            thisfile_column++;
        }
    }
    return ( (c == 0 || n >= SCANNER_STR_SIZES) ? -1 : 0 );
}


/* Set everything up to scan from a string */
void sc_scan_string(char *s)
{
  yy_scan_string(s);
  *origfile_name = 0;
  sc_set_filename("injected_code");
}


/* Utilities */

void sc_set_start_token(int t)
       { start_token = t; thisfile_line = 1; thisfile_column = 0;
         marker_line = origfile_line = 0; }
void sc_set_filename(char *fn)
       { strncpy(thisfile_name, fn, 255); }
char *sc_original_file()
       { return ( (*origfile_name) ?  origfile_name : thisfile_name ); }
int  sc_original_line()  { return(thisfile_line - marker_line + origfile_line);}
int  sc_line()           { return(thisfile_line); }
int  sc_column()         { return(thisfile_column); }
void sc_pause_openmp()   { on_omp_line = 0; }
void sc_start_openmp()   { on_omp_line = 1; }
