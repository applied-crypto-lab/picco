/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
#define YYTOKENTYPE
/* Put the tokens into the symbol table, so that GDB and other debuggers
   know about them.  */
enum yytokentype {
    START_SYMBOL_EXPRESSION = 258,
    START_SYMBOL_BLOCKLIST = 259,
    IDENTIFIER = 260,
    CONSTANT = 261,
    STRING_LITERAL = 262,
    INT = 263,
    VOID = 264,
    RETURN = 265,
    PRIVATE = 266,
    PUBLIC = 267,
    INC_OP = 268,
    DEC_OP = 269,
    LE_OP = 270,
    GE_OP = 271,
    EQ_OP = 272,
    NE_OP = 273,
    AND_OP = 274,
    OR_OP = 275,
    RIGHT_OP = 276,
    LEFT_OP = 277,
    ADD_ASSIGN = 278,
    SUB_ASSIGN = 279,
    MUL_ASSIGN = 280,
    DIV_ASSIGN = 281,
    MOD_ASSIGN = 282,
    IF = 283,
    ELSE = 284,
    WHILE = 285,
    DO = 286,
    FOR = 287,
    CONTINUE = 288,
    BREAK = 289,
    SMCINPUT = 290,
    SMCOUTPUT = 291,
    PRAGMA_OMP = 292,
    PRAGMA_OMP_THREADPRIVATE = 293,
    OMP_PARALLEL = 294,
    OMP_SECTIONS = 295,
    OMP_NOWAIT = 296,
    OMP_ORDERED = 297,
    OMP_SCHEDULE = 298,
    OMP_STATIC = 299,
    OMP_DYNAMIC = 300,
    OMP_GUIDED = 301,
    OMP_RUNTIME = 302,
    OMP_AUTO = 303,
    OMP_SECTION = 304,
    OMP_AFFINITY = 305,
    OMP_SINGLE = 306,
    OMP_MASTER = 307,
    OMP_CRITICAL = 308,
    OMP_BARRIER = 309,
    OMP_ATOMIC = 310,
    OMP_FLUSH = 311,
    OMP_PRIVATE = 312,
    OMP_FIRSTPRIVATE = 313,
    OMP_LASTPRIVATE = 314,
    OMP_SHARED = 315,
    OMP_DEFAULT = 316,
    OMP_NONE = 317,
    OMP_REDUCTION = 318,
    OMP_COPYIN = 319,
    OMP_NUMTHREADS = 320,
    OMP_COPYPRIVATE = 321,
    OMP_FOR = 322,
    OMP_IF = 323,
    OMP_TASK = 324,
    OMP_UNTIED = 325,
    OMP_TASKWAIT = 326,
    OMP_COLLAPSE = 327,
    OMP_FINAL = 328,
    OMP_MERGEABLE = 329,
    OMP_TASKYIELD = 330,
    OMP_READ = 331,
    OMP_WRITE = 332,
    OMP_CAPTURE = 333,
    OMP_UPDATE = 334,
    OMP_MIN = 335,
    OMP_MAX = 336,
    PRAGMA_OMPIX = 337,
    OMPIX_TASKDEF = 338,
    OMPIX_IN = 339,
    OMPIX_OUT = 340,
    OMPIX_INOUT = 341,
    OMPIX_TASKSYNC = 342,
    OMPIX_UPONRETURN = 343,
    OMPIX_ATNODE = 344,
    OMPIX_DETACHED = 345,
    OMPIX_ATWORKER = 346,
    OMPIX_TASKSCHEDULE = 347,
    OMPIX_STRIDE = 348,
    OMPIX_START = 349,
    OMPIX_SCOPE = 350,
    OMPIX_NODES = 351,
    OMPIX_WORKERS = 352,
    OMPIX_LOCAL = 353,
    OMPIX_GLOBAL = 354,
    OMPIX_TIED = 355,
    FLOAT = 356,
    STRUCT = 357,
    UNION = 358,
    TYPEDEF = 359,
    PTR_OP = 360,
    TYPE_NAME = 361,
    SIZEOF = 362,
    PMALLOC = 363,
    PFREE = 364,
    CHAR = 365,
    SHORT = 366,
    LONG = 367
};
#endif
/* Tokens.  */
#define START_SYMBOL_EXPRESSION 258
#define START_SYMBOL_BLOCKLIST 259
#define IDENTIFIER 260
#define CONSTANT 261
#define STRING_LITERAL 262
#define INT 263
#define VOID 264
#define RETURN 265
#define PRIVATE 266
#define PUBLIC 267
#define INC_OP 268
#define DEC_OP 269
#define LE_OP 270
#define GE_OP 271
#define EQ_OP 272
#define NE_OP 273
#define AND_OP 274
#define OR_OP 275
#define RIGHT_OP 276
#define LEFT_OP 277
#define ADD_ASSIGN 278
#define SUB_ASSIGN 279
#define MUL_ASSIGN 280
#define DIV_ASSIGN 281
#define MOD_ASSIGN 282
#define IF 283
#define ELSE 284
#define WHILE 285
#define DO 286
#define FOR 287
#define CONTINUE 288
#define BREAK 289
#define SMCINPUT 290
#define SMCOUTPUT 291
#define PRAGMA_OMP 292
#define PRAGMA_OMP_THREADPRIVATE 293
#define OMP_PARALLEL 294
#define OMP_SECTIONS 295
#define OMP_NOWAIT 296
#define OMP_ORDERED 297
#define OMP_SCHEDULE 298
#define OMP_STATIC 299
#define OMP_DYNAMIC 300
#define OMP_GUIDED 301
#define OMP_RUNTIME 302
#define OMP_AUTO 303
#define OMP_SECTION 304
#define OMP_AFFINITY 305
#define OMP_SINGLE 306
#define OMP_MASTER 307
#define OMP_CRITICAL 308
#define OMP_BARRIER 309
#define OMP_ATOMIC 310
#define OMP_FLUSH 311
#define OMP_PRIVATE 312
#define OMP_FIRSTPRIVATE 313
#define OMP_LASTPRIVATE 314
#define OMP_SHARED 315
#define OMP_DEFAULT 316
#define OMP_NONE 317
#define OMP_REDUCTION 318
#define OMP_COPYIN 319
#define OMP_NUMTHREADS 320
#define OMP_COPYPRIVATE 321
#define OMP_FOR 322
#define OMP_IF 323
#define OMP_TASK 324
#define OMP_UNTIED 325
#define OMP_TASKWAIT 326
#define OMP_COLLAPSE 327
#define OMP_FINAL 328
#define OMP_MERGEABLE 329
#define OMP_TASKYIELD 330
#define OMP_READ 331
#define OMP_WRITE 332
#define OMP_CAPTURE 333
#define OMP_UPDATE 334
#define OMP_MIN 335
#define OMP_MAX 336
#define PRAGMA_OMPIX 337
#define OMPIX_TASKDEF 338
#define OMPIX_IN 339
#define OMPIX_OUT 340
#define OMPIX_INOUT 341
#define OMPIX_TASKSYNC 342
#define OMPIX_UPONRETURN 343
#define OMPIX_ATNODE 344
#define OMPIX_DETACHED 345
#define OMPIX_ATWORKER 346
#define OMPIX_TASKSCHEDULE 347
#define OMPIX_STRIDE 348
#define OMPIX_START 349
#define OMPIX_SCOPE 350
#define OMPIX_NODES 351
#define OMPIX_WORKERS 352
#define OMPIX_LOCAL 353
#define OMPIX_GLOBAL 354
#define OMPIX_TIED 355
#define FLOAT 356
#define STRUCT 357
#define UNION 358
#define TYPEDEF 359
#define PTR_OP 360
#define TYPE_NAME 361
#define SIZEOF 362
#define PMALLOC 363
#define PFREE 364
#define CHAR 365
#define SHORT 366
#define LONG 367

#if !defined YYSTYPE && !defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 89 "parser.y"
{
    char name[2048]; /* A general string */
    int type;        /* A general integer */
    char *string;    /* A dynamically allocated string (only for 2 rules) */
    symbol symb;     /* A symbol */
    astexpr expr;    /* An expression node in the AST */
    astspec spec;    /* A declaration specifier node in the AST */
    astdecl decl;    /* A declarator node in the AST */
    aststmt stmt;    /* A statement node in the AST */
    ompcon ocon;     /* An OpenMP construct */
    ompdir odir;     /* An OpenMP directive */
    ompclause ocla;  /* An OpenMP clause */

    oxcon xcon; /* OMPi extensions */
    oxdir xdir;
    oxclause xcla;

}
/* Line 1529 of yacc.c.  */
#line 270 "parser.h"
YYSTYPE;
#define yystype YYSTYPE /* obsolescent; will be withdrawn */
#define YYSTYPE_IS_DECLARED 1
#define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

// Prototype for insert_variable
void insert_variable(const char *var_name, const char *value_str);