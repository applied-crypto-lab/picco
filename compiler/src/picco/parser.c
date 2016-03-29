/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
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
     SMCOPEN = 290,
     SMCINPUT = 291,
     SMCOUTPUT = 292,
     PRAGMA_OMP = 293,
     PRAGMA_OMP_THREADPRIVATE = 294,
     OMP_PARALLEL = 295,
     OMP_SECTIONS = 296,
     OMP_NOWAIT = 297,
     OMP_ORDERED = 298,
     OMP_SCHEDULE = 299,
     OMP_STATIC = 300,
     OMP_DYNAMIC = 301,
     OMP_GUIDED = 302,
     OMP_RUNTIME = 303,
     OMP_AUTO = 304,
     OMP_SECTION = 305,
     OMP_AFFINITY = 306,
     OMP_SINGLE = 307,
     OMP_MASTER = 308,
     OMP_CRITICAL = 309,
     OMP_BARRIER = 310,
     OMP_ATOMIC = 311,
     OMP_FLUSH = 312,
     OMP_PRIVATE = 313,
     OMP_FIRSTPRIVATE = 314,
     OMP_LASTPRIVATE = 315,
     OMP_SHARED = 316,
     OMP_DEFAULT = 317,
     OMP_NONE = 318,
     OMP_REDUCTION = 319,
     OMP_COPYIN = 320,
     OMP_NUMTHREADS = 321,
     OMP_COPYPRIVATE = 322,
     OMP_FOR = 323,
     OMP_IF = 324,
     OMP_TASK = 325,
     OMP_UNTIED = 326,
     OMP_TASKWAIT = 327,
     OMP_COLLAPSE = 328,
     OMP_FINAL = 329,
     OMP_MERGEABLE = 330,
     OMP_TASKYIELD = 331,
     OMP_READ = 332,
     OMP_WRITE = 333,
     OMP_CAPTURE = 334,
     OMP_UPDATE = 335,
     OMP_MIN = 336,
     OMP_MAX = 337,
     PRAGMA_OMPIX = 338,
     OMPIX_TASKDEF = 339,
     OMPIX_IN = 340,
     OMPIX_OUT = 341,
     OMPIX_INOUT = 342,
     OMPIX_TASKSYNC = 343,
     OMPIX_UPONRETURN = 344,
     OMPIX_ATNODE = 345,
     OMPIX_DETACHED = 346,
     OMPIX_ATWORKER = 347,
     OMPIX_TASKSCHEDULE = 348,
     OMPIX_STRIDE = 349,
     OMPIX_START = 350,
     OMPIX_SCOPE = 351,
     OMPIX_NODES = 352,
     OMPIX_WORKERS = 353,
     OMPIX_LOCAL = 354,
     OMPIX_GLOBAL = 355,
     OMPIX_TIED = 356
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
#define SMCOPEN 290
#define SMCINPUT 291
#define SMCOUTPUT 292
#define PRAGMA_OMP 293
#define PRAGMA_OMP_THREADPRIVATE 294
#define OMP_PARALLEL 295
#define OMP_SECTIONS 296
#define OMP_NOWAIT 297
#define OMP_ORDERED 298
#define OMP_SCHEDULE 299
#define OMP_STATIC 300
#define OMP_DYNAMIC 301
#define OMP_GUIDED 302
#define OMP_RUNTIME 303
#define OMP_AUTO 304
#define OMP_SECTION 305
#define OMP_AFFINITY 306
#define OMP_SINGLE 307
#define OMP_MASTER 308
#define OMP_CRITICAL 309
#define OMP_BARRIER 310
#define OMP_ATOMIC 311
#define OMP_FLUSH 312
#define OMP_PRIVATE 313
#define OMP_FIRSTPRIVATE 314
#define OMP_LASTPRIVATE 315
#define OMP_SHARED 316
#define OMP_DEFAULT 317
#define OMP_NONE 318
#define OMP_REDUCTION 319
#define OMP_COPYIN 320
#define OMP_NUMTHREADS 321
#define OMP_COPYPRIVATE 322
#define OMP_FOR 323
#define OMP_IF 324
#define OMP_TASK 325
#define OMP_UNTIED 326
#define OMP_TASKWAIT 327
#define OMP_COLLAPSE 328
#define OMP_FINAL 329
#define OMP_MERGEABLE 330
#define OMP_TASKYIELD 331
#define OMP_READ 332
#define OMP_WRITE 333
#define OMP_CAPTURE 334
#define OMP_UPDATE 335
#define OMP_MIN 336
#define OMP_MAX 337
#define PRAGMA_OMPIX 338
#define OMPIX_TASKDEF 339
#define OMPIX_IN 340
#define OMPIX_OUT 341
#define OMPIX_INOUT 342
#define OMPIX_TASKSYNC 343
#define OMPIX_UPONRETURN 344
#define OMPIX_ATNODE 345
#define OMPIX_DETACHED 346
#define OMPIX_ATWORKER 347
#define OMPIX_TASKSCHEDULE 348
#define OMPIX_STRIDE 349
#define OMPIX_START 350
#define OMPIX_SCOPE 351
#define OMPIX_NODES 352
#define OMPIX_WORKERS 353
#define OMPIX_LOCAL 354
#define OMPIX_GLOBAL 355
#define OMPIX_TIED 356




/* Copy the first part of user declarations.  */
#line 1 "parser.y"

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

/* parser.y */

/* 
 * 2010/11/10:
 *   dropped OpenMP-specific for parsing; fewer rules, less code
 * 2009/05/11:
 *   added AUTO schedule type
 * 2009/05/03:
 *   added ATNODE ompix clause
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include "scanner.h"
#include "ompi.h"
#include "ast.h"
#include "symtab.h"
#include "ast_vars.h"
#include "ast_free.h"
#include "ast_copy.h"
#include "x_arith.h"
    
    
void    check_uknown_var(char *name);
void    parse_error(int exitvalue, char *format, ...);
void    yyerror(char *s);
void    check_for_main_and_declare(astspec s, astdecl d);
void    add_declaration_links(astspec s, astdecl d);
void    set_security_flag_symbol(astexpr e, symbol s); 
void 	set_security_flag_expr(astexpr e, astexpr e1, astexpr e2, int opid);
void 	set_security_flag_stmt(aststmt s, aststmt s1, aststmt s2);
void    set_security_flag_func(char* funcname, astexpr e2);
int     set_security_flag_spec(astspec spec);
void    set_size_symbol(astexpr, astexpr, astexpr);
int     check_func_param(astexpr funcname, astexpr arglist);
stentry get_entry_from_expr(astexpr);
int     compare_specs(astspec, int);

void    security_check_for_assignment(astexpr le, astexpr re);
void    security_check_for_declaration(astspec spec, astdecl decl);
void 	security_check_for_condition(astexpr e); 
void    increase_index(astexpr e); 
void    decrease_index(astexpr e);
aststmt pastree = NULL;       /* The generated AST */
aststmt pastree_stmt = NULL;  /* For when parsing statment strings */
astexpr pastree_expr = NULL;  /* For when parsing expression strings */
int     checkDecls = 1;       /* 0 when scanning strings (no context check) */
int     tempsave;
int     isTypedef  = 0;       /* To keep track of typedefs */
int     tmp_index = 0;
int     var_index = 0;
int     contain_priv_if_flag = 0;
int     cond_index = 0; 
int     num_index = 0;
int     func_return_flag = 0;
int     default_var_size = 32;      /* Default var size - 32 bits */
char    *parsingstring;       /* For error reporting when parsing string */


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 89 "parser.y"
{
  char      name[2048];  /* A general string */
  int       type;        /* A general integer */
  char     *string;      /* A dynamically allocated string (only for 2 rules) */
  symbol    symb;        /* A symbol */
  astexpr   expr;        /* An expression node in the AST */
  astspec   spec;        /* A declaration specifier node in the AST */
  astdecl   decl;        /* A declarator node in the AST */
  aststmt   stmt;/* A statement node in the AST */
  ompcon    ocon;        /* An OpenMP construct */
  ompdir    odir;        /* An OpenMP directive */
  ompclause ocla;        /* An OpenMP clause */
    
  oxcon     xcon;        /* OMPi extensions */
  oxdir     xdir;
  oxclause  xcla;

}
/* Line 193 of yacc.c.  */
#line 405 "parser.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 418 "parser.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  125
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1385

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  126
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  174
/* YYNRULES -- Number of rules.  */
#define YYNRULES  406
/* YYNRULES -- Number of states.  */
#define YYNSTATES  745

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   356

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     125,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   110,     2,     2,     2,   114,   111,     2,
     102,   103,   112,   115,   108,   109,     2,   113,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   122,   124,
     116,   123,   117,   121,   120,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   104,     2,   105,   118,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   106,   119,   107,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,    11,    13,    16,    18,    20,
      22,    26,    28,    33,    37,    42,    46,    51,    54,    57,
      64,    72,    74,    78,    80,    83,    86,    89,    91,    93,
      95,    97,    99,   100,   105,   106,   111,   112,   117,   119,
     120,   125,   126,   131,   133,   137,   141,   143,   144,   149,
     150,   155,   156,   161,   162,   167,   169,   170,   175,   176,
     181,   183,   184,   189,   191,   192,   197,   199,   200,   205,
     207,   208,   213,   215,   216,   221,   225,   227,   229,   235,
     237,   241,   243,   245,   247,   249,   251,   253,   255,   259,
     261,   265,   267,   269,   272,   274,   278,   280,   281,   286,
     288,   290,   295,   297,   299,   301,   304,   307,   311,   313,
     315,   319,   322,   324,   327,   329,   331,   334,   338,   341,
     345,   349,   354,   358,   363,   366,   370,   374,   379,   381,
     384,   386,   389,   391,   395,   399,   404,   408,   413,   418,
     420,   424,   426,   428,   432,   437,   439,   442,   446,   451,
     454,   456,   459,   463,   465,   467,   469,   471,   473,   475,
     477,   479,   481,   484,   485,   490,   492,   495,   497,   499,
     501,   503,   505,   508,   509,   516,   525,   531,   539,   541,
     548,   556,   564,   572,   581,   590,   599,   609,   616,   624,
     632,   641,   644,   647,   650,   654,   662,   672,   680,   690,
     698,   708,   720,   722,   725,   727,   729,   731,   733,   734,
     739,   740,   744,   746,   748,   750,   752,   754,   756,   758,
     760,   762,   764,   766,   768,   770,   772,   774,   776,   779,
     784,   785,   788,   792,   794,   796,   797,   803,   804,   810,
     813,   818,   819,   822,   826,   828,   830,   832,   834,   839,
     840,   848,   849,   857,   862,   864,   866,   868,   870,   872,
     874,   877,   882,   883,   886,   890,   892,   894,   898,   900,
     903,   907,   911,   914,   919,   920,   923,   927,   929,   931,
     934,   940,   941,   944,   948,   950,   952,   954,   957,   963,
     964,   967,   971,   973,   975,   978,   983,   984,   987,   991,
     993,   995,   996,  1002,  1004,  1005,  1011,  1013,  1016,  1020,
    1023,  1027,  1032,  1036,  1040,  1044,  1048,  1051,  1055,  1060,
    1065,  1070,  1074,  1079,  1080,  1085,  1088,  1092,  1098,  1099,
    1105,  1106,  1112,  1113,  1119,  1120,  1126,  1127,  1133,  1138,
    1143,  1144,  1152,  1153,  1159,  1161,  1163,  1165,  1167,  1169,
    1171,  1173,  1175,  1177,  1179,  1181,  1185,  1187,  1191,  1193,
    1195,  1199,  1200,  1206,  1207,  1210,  1214,  1219,  1224,  1229,
    1231,  1233,  1235,  1237,  1241,  1245,  1247,  1249,  1250,  1255,
    1258,  1259,  1265,  1266,  1269,  1273,  1278,  1283,  1288,  1295,
    1297,  1301,  1303,  1309,  1314,  1318,  1323,  1324,  1327,  1331,
    1336,  1341,  1346,  1348,  1350,  1352,  1356
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     127,     0,    -1,   202,    -1,     3,   164,    -1,     4,   192,
      -1,     7,    -1,   128,     7,    -1,     5,    -1,     6,    -1,
     128,    -1,   102,   164,   103,    -1,   129,    -1,   130,   104,
     164,   105,    -1,     5,   102,   103,    -1,     5,   102,   131,
     103,    -1,   130,   102,   103,    -1,   130,   102,   131,   103,
      -1,   130,    13,    -1,   130,    14,    -1,   102,   183,   103,
     106,   185,   107,    -1,   102,   183,   103,   106,   185,   108,
     107,    -1,   162,    -1,   131,   108,   162,    -1,   130,    -1,
      13,   132,    -1,    14,   132,    -1,   133,   132,    -1,   109,
      -1,   110,    -1,   111,    -1,   112,    -1,   132,    -1,    -1,
     134,   112,   135,   132,    -1,    -1,   134,   113,   136,   132,
      -1,    -1,   134,   114,   137,   132,    -1,   134,    -1,    -1,
     138,   115,   139,   134,    -1,    -1,   138,   109,   140,   134,
      -1,   138,    -1,   141,    22,   138,    -1,   141,    21,   138,
      -1,   141,    -1,    -1,   142,   116,   143,   141,    -1,    -1,
     142,   117,   144,   141,    -1,    -1,   142,    15,   145,   141,
      -1,    -1,   142,    16,   146,   141,    -1,   142,    -1,    -1,
     147,    17,   148,   142,    -1,    -1,   147,    18,   149,   142,
      -1,   147,    -1,    -1,   150,   111,   151,   147,    -1,   150,
      -1,    -1,   152,   118,   153,   150,    -1,   152,    -1,    -1,
     154,   119,   155,   152,    -1,   154,    -1,    -1,   156,    19,
     157,   154,    -1,   156,    -1,    -1,   158,    20,   159,   156,
      -1,   132,   120,   132,    -1,   158,    -1,   160,    -1,   158,
     121,   164,   122,   161,    -1,   161,    -1,   132,   163,   162,
      -1,   123,    -1,    25,    -1,    26,    -1,    27,    -1,    23,
      -1,    24,    -1,   162,    -1,   164,   108,   162,    -1,   161,
      -1,   167,   168,   124,    -1,   267,    -1,   171,    -1,   172,
     171,    -1,   169,    -1,   168,   108,   169,    -1,   180,    -1,
      -1,   180,   123,   170,   184,    -1,     8,    -1,     9,    -1,
       8,   116,     6,   117,    -1,    11,    -1,    12,    -1,   112,
      -1,   112,   172,    -1,   112,   173,    -1,   112,   172,   173,
      -1,   175,    -1,   176,    -1,   175,   108,   176,    -1,   167,
     180,    -1,   167,    -1,   167,   177,    -1,   173,    -1,   178,
      -1,   173,   178,    -1,   102,   177,   103,    -1,   104,   105,
      -1,   178,   104,   105,    -1,   104,   162,   105,    -1,   178,
     104,   162,   105,    -1,   104,   112,   105,    -1,   178,   104,
     112,   105,    -1,   102,   103,    -1,   178,   102,   103,    -1,
     102,   174,   103,    -1,   178,   102,   174,   103,    -1,   171,
      -1,   171,   179,    -1,   181,    -1,   173,   181,    -1,     5,
      -1,   102,   180,   103,    -1,   181,   104,   105,    -1,   181,
     104,   162,   105,    -1,   181,   102,   103,    -1,   181,   102,
     182,   103,    -1,   181,   102,   174,   103,    -1,     5,    -1,
     182,   108,     5,    -1,   179,    -1,   162,    -1,   106,   185,
     107,    -1,   106,   185,   108,   107,    -1,   184,    -1,   186,
     184,    -1,   185,   108,   184,    -1,   185,   108,   186,   184,
      -1,   187,   123,    -1,   188,    -1,   187,   188,    -1,   104,
     165,   105,    -1,   190,    -1,   194,    -1,   199,    -1,   196,
      -1,   197,    -1,   200,    -1,   201,    -1,   208,    -1,   286,
      -1,   106,   107,    -1,    -1,   106,   191,   192,   107,    -1,
     193,    -1,   192,   193,    -1,   166,    -1,   189,    -1,   209,
      -1,   279,    -1,   124,    -1,   164,   124,    -1,    -1,    28,
     102,   164,   103,   195,   189,    -1,    28,   102,   164,   103,
     195,   189,    29,   189,    -1,    30,   102,   164,   103,   189,
      -1,    31,   189,    30,   102,   164,   103,   124,    -1,   198,
      -1,    32,   102,   124,   124,   103,   189,    -1,    32,   102,
     164,   124,   124,   103,   189,    -1,    32,   102,   124,   164,
     124,   103,   189,    -1,    32,   102,   124,   124,   164,   103,
     189,    -1,    32,   102,   164,   124,   164,   124,   103,   189,
      -1,    32,   102,   164,   124,   124,   164,   103,   189,    -1,
      32,   102,   124,   164,   124,   164,   103,   189,    -1,    32,
     102,   164,   124,   164,   124,   164,   103,   189,    -1,    32,
     102,   166,   124,   103,   189,    -1,    32,   102,   166,   164,
     124,   103,   189,    -1,    32,   102,   166,   124,   164,   103,
     189,    -1,    32,   102,   166,   164,   124,   164,   103,   189,
      -1,    33,   124,    -1,    34,   124,    -1,    10,   124,    -1,
      10,   164,   124,    -1,    36,   102,   130,   108,     6,   103,
     124,    -1,    36,   102,   130,   108,     6,   108,   162,   103,
     124,    -1,    37,   102,   130,   108,     6,   103,   124,    -1,
      37,   102,   130,   108,     6,   108,   162,   103,   124,    -1,
      35,   102,   130,   108,   130,   103,   124,    -1,    35,   102,
     130,   108,   130,   108,   162,   103,   124,    -1,    32,   102,
     164,   124,   164,   124,   164,   103,   104,   192,   105,    -1,
     203,    -1,   202,   203,    -1,   204,    -1,   166,    -1,   287,
      -1,   205,    -1,    -1,   167,   180,   206,   190,    -1,    -1,
     180,   207,   190,    -1,   211,    -1,   218,    -1,   226,    -1,
     233,    -1,   237,    -1,   241,    -1,   252,    -1,   254,    -1,
     260,    -1,   265,    -1,   245,    -1,   259,    -1,   262,    -1,
     257,    -1,   258,    -1,   189,    -1,   212,   210,    -1,    38,
      40,   213,   125,    -1,    -1,   213,   214,    -1,   213,   108,
     214,    -1,   215,    -1,   268,    -1,    -1,    69,   102,   216,
     164,   103,    -1,    -1,    66,   102,   217,   164,   103,    -1,
     219,   198,    -1,    38,    68,   220,   125,    -1,    -1,   220,
     221,    -1,   220,   108,   221,    -1,   222,    -1,   268,    -1,
      42,    -1,    43,    -1,    44,   102,   225,   103,    -1,    -1,
      44,   102,   225,   108,   223,   164,   103,    -1,    -1,    44,
     102,    51,   108,   224,   164,   103,    -1,    73,   102,   164,
     103,    -1,    45,    -1,    46,    -1,    47,    -1,    48,    -1,
      49,    -1,     1,    -1,   227,   230,    -1,    38,    41,   228,
     125,    -1,    -1,   228,   229,    -1,   228,   108,   229,    -1,
     268,    -1,    42,    -1,   106,   231,   107,    -1,   210,    -1,
     232,   210,    -1,   231,   232,   210,    -1,    38,    50,   125,
      -1,   234,   210,    -1,    38,    52,   235,   125,    -1,    -1,
     235,   236,    -1,   235,   108,   236,    -1,   268,    -1,    42,
      -1,   238,   198,    -1,    38,    40,    68,   239,   125,    -1,
      -1,   239,   240,    -1,   239,   108,   240,    -1,   215,    -1,
     222,    -1,   268,    -1,   242,   230,    -1,    38,    40,    41,
     243,   125,    -1,    -1,   243,   244,    -1,   243,   108,   244,
      -1,   215,    -1,   268,    -1,   246,   210,    -1,    38,    70,
     247,   125,    -1,    -1,   247,   248,    -1,   247,   108,   248,
      -1,   249,    -1,   268,    -1,    -1,    69,   102,   250,   164,
     103,    -1,    71,    -1,    -1,    74,   102,   251,   164,   103,
      -1,    75,    -1,   253,   210,    -1,    38,    53,   125,    -1,
     255,   210,    -1,    38,    54,   125,    -1,    38,    54,   256,
     125,    -1,   102,     5,   103,    -1,    38,    72,   125,    -1,
      38,    76,   125,    -1,    38,    55,   125,    -1,   261,   194,
      -1,    38,    56,   125,    -1,    38,    56,    78,   125,    -1,
      38,    56,    77,   125,    -1,    38,    56,    80,   125,    -1,
      38,    57,   125,    -1,    38,    57,   263,   125,    -1,    -1,
     102,   264,   277,   103,    -1,   266,   210,    -1,    38,    43,
     125,    -1,    39,   102,   278,   103,   125,    -1,    -1,    58,
     269,   102,   277,   103,    -1,    -1,    67,   270,   102,   277,
     103,    -1,    -1,    59,   271,   102,   277,   103,    -1,    -1,
      60,   272,   102,   277,   103,    -1,    -1,    61,   273,   102,
     277,   103,    -1,    62,   102,    61,   103,    -1,    62,   102,
      63,   103,    -1,    -1,    64,   102,   276,   274,   122,   277,
     103,    -1,    -1,    65,   275,   102,   277,   103,    -1,   115,
      -1,   112,    -1,   109,    -1,   111,    -1,   118,    -1,   119,
      -1,    19,    -1,    20,    -1,    81,    -1,    82,    -1,     5,
      -1,   277,   108,     5,    -1,     5,    -1,   278,   108,     5,
      -1,   280,    -1,   281,    -1,    83,    88,   125,    -1,    -1,
      83,    93,   282,   283,   125,    -1,    -1,   283,   284,    -1,
     283,   108,   284,    -1,    94,   102,   162,   103,    -1,    95,
     102,   162,   103,    -1,    96,   102,   285,   103,    -1,   101,
      -1,    71,    -1,    97,    -1,    98,    -1,    98,   108,   100,
      -1,    98,   108,    99,    -1,   287,    -1,   295,    -1,    -1,
     289,   205,   288,   190,    -1,   289,   205,    -1,    -1,    83,
      84,   290,   291,   125,    -1,    -1,   291,   292,    -1,   291,
     108,   292,    -1,    85,   102,   293,   103,    -1,    86,   102,
     293,   103,    -1,    87,   102,   293,   103,    -1,    64,   102,
     276,   122,   293,   103,    -1,   294,    -1,   293,   108,   294,
      -1,     5,    -1,     5,   104,   121,     5,   105,    -1,     5,
     104,   162,   105,    -1,   296,   299,   124,    -1,    83,    70,
     297,   125,    -1,    -1,   297,   298,    -1,   297,   108,   298,
      -1,    90,   102,   112,   103,    -1,    90,   102,   162,   103,
      -1,    92,   102,   162,   103,    -1,   101,    -1,    71,    -1,
      91,    -1,     5,   102,   103,    -1,     5,   102,   131,   103,
      -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   304,   304,   305,   306,   330,   334,   357,   376,   382,
     387,   397,   401,   413,   421,   432,   436,   440,   446,   453,
     457,   465,   469,   477,   481,   487,   493,   502,   506,   510,
     514,   524,   529,   528,   539,   538,   549,   548,   562,   567,
     566,   577,   576,   590,   594,   600,   609,   614,   613,   624,
     623,   634,   633,   644,   643,   657,   662,   661,   672,   671,
     684,   689,   688,   702,   707,   706,   720,   725,   724,   738,
     743,   742,   757,   762,   761,   773,   780,   784,   788,   798,
     802,   813,   817,   821,   825,   829,   833,   842,   847,   855,
     870,   878,   887,   891,   899,   903,   917,   934,   933,   958,
     962,   966,   973,   977,   986,   990,   994,   998,  1005,  1016,
    1020,  1027,  1031,  1035,  1043,  1047,  1051,  1058,  1062,  1066,
    1070,  1074,  1078,  1082,  1086,  1090,  1094,  1098,  1106,  1110,
    1118,  1122,  1131,  1135,  1143,  1147,  1151,  1155,  1159,  1169,
    1173,  1181,  1190,  1194,  1198,  1206,  1210,  1214,  1218,  1226,
    1234,  1238,  1246,  1261,  1266,  1271,  1276,  1281,  1286,  1291,
    1296,  1301,  1311,  1315,  1315,  1327,  1332,  1342,  1346,  1351,
    1356,  1365,  1369,  1378,  1384,  1395,  1413,  1419,  1425,  1432,
    1437,  1442,  1448,  1454,  1460,  1465,  1471,  1478,  1484,  1491,
    1497,  1507,  1511,  1515,  1519,  1531,  1539,  1547,  1554,  1562,
    1572,  1583,  1597,  1601,  1609,  1613,  1620,  1631,  1636,  1635,
    1671,  1670,  1709,  1713,  1717,  1721,  1725,  1729,  1733,  1737,
    1741,  1745,  1750,  1766,  1770,  1775,  1780,  1787,  1794,  1802,
    1810,  1813,  1817,  1824,  1828,  1835,  1835,  1840,  1840,  1848,
    1855,  1863,  1866,  1870,  1877,  1881,  1885,  1892,  1896,  1900,
    1900,  1908,  1907,  1919,  1934,  1938,  1942,  1946,  1950,  1954,
    1959,  1966,  1974,  1977,  1981,  1988,  1992,  1999,  2006,  2011,
    2015,  2022,  2029,  2036,  2044,  2047,  2051,  2058,  2062,  2069,
    2076,  2084,  2087,  2091,  2098,  2102,  2106,  2113,  2120,  2128,
    2131,  2135,  2142,  2146,  2154,  2163,  2172,  2175,  2179,  2187,
    2191,  2199,  2199,  2204,  2208,  2208,  2213,  2219,  2226,  2233,
    2240,  2244,  2251,  2259,  2267,  2274,  2281,  2288,  2292,  2296,
    2300,  2307,  2311,  2318,  2318,  2326,  2333,  2340,  2347,  2347,
    2353,  2353,  2358,  2358,  2363,  2363,  2368,  2368,  2373,  2377,
    2381,  2381,  2386,  2386,  2394,  2398,  2402,  2406,  2410,  2414,
    2418,  2422,  2426,  2430,  2437,  2444,  2459,  2477,  2503,  2507,
    2515,  2523,  2522,  2535,  2538,  2542,  2549,  2553,  2557,  2561,
    2565,  2572,  2576,  2580,  2584,  2591,  2595,  2604,  2603,  2620,
    2629,  2628,  2641,  2644,  2648,  2655,  2659,  2663,  2667,  2674,
    2678,  2685,  2690,  2698,  2706,  2714,  2722,  2725,  2729,  2736,
    2740,  2744,  2748,  2752,  2756,  2763,  2769
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "START_SYMBOL_EXPRESSION",
  "START_SYMBOL_BLOCKLIST", "IDENTIFIER", "CONSTANT", "STRING_LITERAL",
  "INT", "VOID", "RETURN", "PRIVATE", "PUBLIC", "INC_OP", "DEC_OP",
  "LE_OP", "GE_OP", "EQ_OP", "NE_OP", "AND_OP", "OR_OP", "RIGHT_OP",
  "LEFT_OP", "ADD_ASSIGN", "SUB_ASSIGN", "MUL_ASSIGN", "DIV_ASSIGN",
  "MOD_ASSIGN", "IF", "ELSE", "WHILE", "DO", "FOR", "CONTINUE", "BREAK",
  "SMCOPEN", "SMCINPUT", "SMCOUTPUT", "PRAGMA_OMP",
  "PRAGMA_OMP_THREADPRIVATE", "OMP_PARALLEL", "OMP_SECTIONS", "OMP_NOWAIT",
  "OMP_ORDERED", "OMP_SCHEDULE", "OMP_STATIC", "OMP_DYNAMIC", "OMP_GUIDED",
  "OMP_RUNTIME", "OMP_AUTO", "OMP_SECTION", "OMP_AFFINITY", "OMP_SINGLE",
  "OMP_MASTER", "OMP_CRITICAL", "OMP_BARRIER", "OMP_ATOMIC", "OMP_FLUSH",
  "OMP_PRIVATE", "OMP_FIRSTPRIVATE", "OMP_LASTPRIVATE", "OMP_SHARED",
  "OMP_DEFAULT", "OMP_NONE", "OMP_REDUCTION", "OMP_COPYIN",
  "OMP_NUMTHREADS", "OMP_COPYPRIVATE", "OMP_FOR", "OMP_IF", "OMP_TASK",
  "OMP_UNTIED", "OMP_TASKWAIT", "OMP_COLLAPSE", "OMP_FINAL",
  "OMP_MERGEABLE", "OMP_TASKYIELD", "OMP_READ", "OMP_WRITE", "OMP_CAPTURE",
  "OMP_UPDATE", "OMP_MIN", "OMP_MAX", "PRAGMA_OMPIX", "OMPIX_TASKDEF",
  "OMPIX_IN", "OMPIX_OUT", "OMPIX_INOUT", "OMPIX_TASKSYNC",
  "OMPIX_UPONRETURN", "OMPIX_ATNODE", "OMPIX_DETACHED", "OMPIX_ATWORKER",
  "OMPIX_TASKSCHEDULE", "OMPIX_STRIDE", "OMPIX_START", "OMPIX_SCOPE",
  "OMPIX_NODES", "OMPIX_WORKERS", "OMPIX_LOCAL", "OMPIX_GLOBAL",
  "OMPIX_TIED", "'('", "')'", "'['", "']'", "'{'", "'}'", "','", "'-'",
  "'!'", "'&'", "'*'", "'/'", "'%'", "'+'", "'<'", "'>'", "'^'", "'|'",
  "'@'", "'?'", "':'", "'='", "';'", "'\\n'", "$accept", "start_trick",
  "string_literal", "primary_expression", "postfix_expression",
  "argument_expression_list", "unary_expression", "unary_operator",
  "multiplicative_expression", "@1", "@2", "@3", "additive_expression",
  "@4", "@5", "shift_expression", "relational_expression", "@6", "@7",
  "@8", "@9", "equality_expression", "@10", "@11", "AND_expression", "@12",
  "exclusive_OR_expression", "@13", "inclusive_OR_expression", "@14",
  "logical_AND_expression", "@15", "logical_OR_expression", "@16",
  "DOT_product_expression", "conditional_expression",
  "assignment_expression", "assignment_operator", "expression",
  "constant_expression", "declaration", "declaration_specifiers",
  "init_declarator_list", "init_declarator", "@17", "type_specifier",
  "type_qualifier", "pointer", "parameter_type_list", "parameter_list",
  "parameter_declaration", "abstract_declarator",
  "direct_abstract_declarator", "specifier_qualifier_list", "declarator",
  "direct_declarator", "identifier_list", "type_name", "initializer",
  "initializer_list", "designation", "designator_list", "designator",
  "statement", "compound_statement", "@18", "block_item_list",
  "block_item", "expression_statement", "if_subroutine",
  "selection_statement", "iteration_statement", "iteration_statement_for",
  "jump_statement", "smc_statement", "batch_statement", "translation_unit",
  "external_declaration", "function_definition",
  "normal_function_definition", "@19", "@20", "openmp_construct",
  "openmp_directive", "structured_block", "parallel_construct",
  "parallel_directive", "parallel_clause_optseq", "parallel_clause",
  "unique_parallel_clause", "@21", "@22", "for_construct", "for_directive",
  "for_clause_optseq", "for_clause", "unique_for_clause", "@23", "@24",
  "schedule_kind", "sections_construct", "sections_directive",
  "sections_clause_optseq", "sections_clause", "section_scope",
  "section_sequence", "section_directive", "single_construct",
  "single_directive", "single_clause_optseq", "single_clause",
  "parallel_for_construct", "parallel_for_directive",
  "parallel_for_clause_optseq", "parallel_for_clause",
  "parallel_sections_construct", "parallel_sections_directive",
  "parallel_sections_clause_optseq", "parallel_sections_clause",
  "task_construct", "task_directive", "task_clause_optseq", "task_clause",
  "unique_task_clause", "@25", "@26", "master_construct",
  "master_directive", "critical_construct", "critical_directive",
  "region_phrase", "taskwait_directive", "taskyield_directive",
  "barrier_directive", "atomic_construct", "atomic_directive",
  "flush_directive", "flush_vars", "@27", "ordered_construct",
  "ordered_directive", "threadprivate_directive", "data_clause", "@28",
  "@29", "@30", "@31", "@32", "@33", "@34", "reduction_operator",
  "variable_list", "thrprv_variable_list", "ompix_directive",
  "ox_tasksync_directive", "ox_taskschedule_directive", "@35",
  "ox_taskschedule_clause_optseq", "ox_taskschedule_clause",
  "ox_scope_spec", "ompix_construct", "ox_taskdef_construct", "@36",
  "ox_taskdef_directive", "@37", "ox_taskdef_clause_optseq",
  "ox_taskdef_clause", "ox_variable_size_list", "ox_variable_size_elem",
  "ox_task_construct", "ox_task_directive", "ox_task_clause_optseq",
  "ox_task_clause", "ox_funccall_expression", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,    40,    41,    91,    93,   123,   125,    44,    45,
      33,    38,    42,    47,    37,    43,    60,    62,    94,   124,
      64,    63,    58,    61,    59,    10
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   126,   127,   127,   127,   128,   128,   129,   129,   129,
     129,   130,   130,   130,   130,   130,   130,   130,   130,   130,
     130,   131,   131,   132,   132,   132,   132,   133,   133,   133,
     133,   134,   135,   134,   136,   134,   137,   134,   138,   139,
     138,   140,   138,   141,   141,   141,   142,   143,   142,   144,
     142,   145,   142,   146,   142,   147,   148,   147,   149,   147,
     150,   151,   150,   152,   153,   152,   154,   155,   154,   156,
     157,   156,   158,   159,   158,   160,   161,   161,   161,   162,
     162,   163,   163,   163,   163,   163,   163,   164,   164,   165,
     166,   166,   167,   167,   168,   168,   169,   170,   169,   171,
     171,   171,   172,   172,   173,   173,   173,   173,   174,   175,
     175,   176,   176,   176,   177,   177,   177,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   179,   179,
     180,   180,   181,   181,   181,   181,   181,   181,   181,   182,
     182,   183,   184,   184,   184,   185,   185,   185,   185,   186,
     187,   187,   188,   189,   189,   189,   189,   189,   189,   189,
     189,   189,   190,   191,   190,   192,   192,   193,   193,   193,
     193,   194,   194,   195,   196,   196,   197,   197,   197,   198,
     198,   198,   198,   198,   198,   198,   198,   198,   198,   198,
     198,   199,   199,   199,   199,   200,   200,   200,   200,   200,
     200,   201,   202,   202,   203,   203,   203,   204,   206,   205,
     207,   205,   208,   208,   208,   208,   208,   208,   208,   208,
     208,   208,   208,   209,   209,   209,   209,   210,   211,   212,
     213,   213,   213,   214,   214,   216,   215,   217,   215,   218,
     219,   220,   220,   220,   221,   221,   221,   222,   222,   223,
     222,   224,   222,   222,   225,   225,   225,   225,   225,   225,
     226,   227,   228,   228,   228,   229,   229,   230,   231,   231,
     231,   232,   233,   234,   235,   235,   235,   236,   236,   237,
     238,   239,   239,   239,   240,   240,   240,   241,   242,   243,
     243,   243,   244,   244,   245,   246,   247,   247,   247,   248,
     248,   250,   249,   249,   251,   249,   249,   252,   253,   254,
     255,   255,   256,   257,   258,   259,   260,   261,   261,   261,
     261,   262,   262,   264,   263,   265,   266,   267,   269,   268,
     270,   268,   271,   268,   272,   268,   273,   268,   268,   268,
     274,   268,   275,   268,   276,   276,   276,   276,   276,   276,
     276,   276,   276,   276,   277,   277,   278,   278,   279,   279,
     280,   282,   281,   283,   283,   283,   284,   284,   284,   284,
     284,   285,   285,   285,   285,   286,   286,   288,   287,   287,
     290,   289,   291,   291,   291,   292,   292,   292,   292,   293,
     293,   294,   294,   294,   295,   296,   297,   297,   297,   298,
     298,   298,   298,   298,   298,   299,   299
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     2,     1,     2,     1,     1,     1,
       3,     1,     4,     3,     4,     3,     4,     2,     2,     6,
       7,     1,     3,     1,     2,     2,     2,     1,     1,     1,
       1,     1,     0,     4,     0,     4,     0,     4,     1,     0,
       4,     0,     4,     1,     3,     3,     1,     0,     4,     0,
       4,     0,     4,     0,     4,     1,     0,     4,     0,     4,
       1,     0,     4,     1,     0,     4,     1,     0,     4,     1,
       0,     4,     1,     0,     4,     3,     1,     1,     5,     1,
       3,     1,     1,     1,     1,     1,     1,     1,     3,     1,
       3,     1,     1,     2,     1,     3,     1,     0,     4,     1,
       1,     4,     1,     1,     1,     2,     2,     3,     1,     1,
       3,     2,     1,     2,     1,     1,     2,     3,     2,     3,
       3,     4,     3,     4,     2,     3,     3,     4,     1,     2,
       1,     2,     1,     3,     3,     4,     3,     4,     4,     1,
       3,     1,     1,     3,     4,     1,     2,     3,     4,     2,
       1,     2,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     0,     4,     1,     2,     1,     1,     1,
       1,     1,     2,     0,     6,     8,     5,     7,     1,     6,
       7,     7,     7,     8,     8,     8,     9,     6,     7,     7,
       8,     2,     2,     2,     3,     7,     9,     7,     9,     7,
       9,    11,     1,     2,     1,     1,     1,     1,     0,     4,
       0,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     4,
       0,     2,     3,     1,     1,     0,     5,     0,     5,     2,
       4,     0,     2,     3,     1,     1,     1,     1,     4,     0,
       7,     0,     7,     4,     1,     1,     1,     1,     1,     1,
       2,     4,     0,     2,     3,     1,     1,     3,     1,     2,
       3,     3,     2,     4,     0,     2,     3,     1,     1,     2,
       5,     0,     2,     3,     1,     1,     1,     2,     5,     0,
       2,     3,     1,     1,     2,     4,     0,     2,     3,     1,
       1,     0,     5,     1,     0,     5,     1,     2,     3,     2,
       3,     4,     3,     3,     3,     3,     2,     3,     4,     4,
       4,     3,     4,     0,     4,     2,     3,     5,     0,     5,
       0,     5,     0,     5,     0,     5,     0,     5,     4,     4,
       0,     7,     0,     5,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     3,     1,     1,
       3,     0,     5,     0,     2,     3,     4,     4,     4,     1,
       1,     1,     1,     3,     3,     1,     1,     0,     4,     2,
       0,     5,     0,     2,     3,     4,     4,     4,     6,     1,
       3,     1,     5,     4,     3,     4,     0,     2,     3,     4,
       4,     4,     1,     1,     1,     3,     4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       0,     0,     0,   132,    99,   100,   102,   103,     0,     0,
       0,   104,     0,   205,     0,    92,     0,     0,   210,   130,
       2,   202,   204,   207,    91,   206,     0,     7,     8,     5,
       0,     0,     0,    27,    28,    29,    30,     9,    11,    23,
      31,     0,    38,    43,    46,    55,    60,    63,    66,    69,
      72,    76,    77,    79,    87,     3,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   163,   171,
       0,   167,     0,   168,   153,     4,   165,   154,   156,   157,
     178,   155,   158,   159,   160,   169,   212,     0,   213,     0,
     214,     0,   215,     0,   216,     0,   217,     0,   222,     0,
     218,     0,   219,     0,   225,   226,   223,   220,     0,   224,
     221,     0,   170,   358,   359,   161,   375,   376,     0,     0,
       0,   380,     0,   105,   106,     1,     0,    94,    96,    93,
     131,     0,     0,     0,   203,     0,   379,     0,    24,    25,
       0,   128,   141,     0,     6,    17,    18,     0,     0,    85,
      86,    82,    83,    84,     0,    81,     0,    26,    32,    34,
      36,    41,    39,     0,     0,    51,    53,    47,    49,    56,
      58,    61,    64,    67,    70,    73,     0,     0,   193,     0,
       0,     0,     0,     0,     0,     0,   191,   192,     0,     0,
       0,   230,   262,     0,   274,     0,     0,     0,     0,     0,
     241,   296,     0,     0,   396,     0,   361,   162,     0,   172,
      96,   166,   227,   228,     0,   239,     0,   260,   272,   279,
     287,   294,   307,   309,   316,   325,     0,     0,     0,   356,
       0,   382,   133,   107,     0,    90,    97,     0,   211,   139,
     136,   112,     0,   108,   109,     0,   134,     0,   208,     0,
      13,     0,    21,    10,   129,     0,    15,     0,     0,    75,
      80,     0,     0,     0,     0,     0,    31,    45,    44,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    88,   194,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   289,   281,     0,     0,   326,     0,   308,     0,
     310,     0,   315,     0,     0,     0,   317,   323,   321,     0,
       0,     0,   313,   314,     0,   360,   363,     0,     0,     0,
     268,     0,     0,     0,   394,   101,     0,     0,     0,    95,
       0,   209,     0,     0,   114,   113,   115,   111,   138,     0,
     137,     0,   135,   378,    14,     0,     0,    16,    12,    33,
      35,    37,    42,    40,    52,    54,    48,    50,    57,    59,
      62,    65,    68,    71,    74,     0,   173,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   328,
     332,   334,   336,     0,     0,   342,     0,   330,     0,     0,
     229,   231,   233,   234,   266,     0,   261,   263,   265,   278,
       0,   273,   275,   277,     0,   311,   319,   318,   320,     0,
     322,   246,   247,     0,     0,     0,   240,   242,   244,   245,
       0,   303,     0,   306,     0,   295,   297,   299,   300,   403,
       0,   404,     0,   402,     0,   395,   397,     0,   164,     0,
       0,     0,   267,     0,   269,   405,     0,   327,   357,     0,
       0,     0,     0,     0,   381,   383,     0,   142,    98,   124,
       0,     0,   118,    30,     0,   116,     0,     0,   110,   140,
      22,     0,   145,     0,     0,     0,   150,    31,    78,     0,
     176,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   288,   292,   290,   293,     0,   280,
     284,   285,   282,   286,     0,     0,     0,     0,     0,     0,
       0,   237,     0,   235,   232,   264,   276,   312,   354,     0,
       0,     0,   243,   301,   304,   298,     0,     0,   398,   370,
       0,     0,     0,   369,     0,   362,   364,     0,   271,   270,
     406,     0,     0,     0,     0,   384,     0,   126,   117,   122,
     120,   125,     0,   119,    30,     0,    89,     0,    19,     0,
     146,   149,   151,   174,     0,   179,     0,     0,     0,     0,
       0,     0,   187,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   291,   283,     0,     0,     0,     0,     0,     0,
     350,   351,   352,   353,   346,   347,   345,   344,   348,   349,
     340,     0,     0,     0,     0,   324,     0,   259,   254,   255,
     256,   257,   258,     0,     0,     0,     0,     0,    30,     0,
       0,     0,     0,     0,   365,     0,     0,   391,     0,   389,
       0,     0,   143,     0,   127,   123,   121,   152,    20,   147,
       0,     0,   177,   182,   181,     0,   180,     0,     0,     0,
     189,   188,     0,   199,     0,   195,     0,   197,     0,     0,
       0,     0,     0,   338,   339,     0,     0,     0,     0,     0,
     355,   251,   248,   249,   253,     0,     0,   399,   400,   401,
       0,     0,   371,   372,     0,     0,     0,     0,   385,     0,
     386,   387,   144,   148,   175,   185,   184,   183,     0,   190,
       0,     0,     0,   329,   333,   335,   337,     0,   343,   238,
     331,   236,     0,     0,   302,   305,   366,   367,     0,   368,
       0,     0,     0,     0,   390,     0,   186,   200,   196,   198,
       0,     0,     0,   374,   373,     0,   388,     0,   393,     0,
     341,   252,   250,   392,   201
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    12,    37,    38,    39,   251,    40,    41,    42,   261,
     262,   263,    43,   265,   264,    44,    45,   271,   272,   269,
     270,    46,   273,   274,    47,   275,    48,   276,    49,   277,
      50,   278,    51,   279,    52,    53,    54,   156,    70,   557,
      71,    72,   126,   127,   330,    15,    16,    17,   242,   243,
     244,   335,   336,   142,    18,    19,   245,   143,   472,   473,
     474,   475,   476,   212,    74,   208,    75,    76,    77,   479,
      78,    79,    80,    81,    82,    83,    20,    21,    22,    23,
     237,   131,    84,    85,   213,    86,    87,   294,   391,   392,
     604,   602,    88,    89,   310,   417,   418,   713,   712,   614,
      90,    91,   295,   397,   217,   321,   322,    92,    93,   297,
     402,    94,    95,   378,   502,    96,    97,   377,   496,    98,
      99,   311,   426,   427,   616,   617,   100,   101,   102,   103,
     301,   104,   105,   106,   107,   108,   109,   309,   409,   110,
     111,    24,   393,   504,   512,   505,   506,   507,   665,   510,
     600,   519,   230,   112,   113,   114,   316,   437,   536,   684,
     115,   116,   249,    26,   231,   328,   455,   628,   629,   117,
     118,   314,   436,   227
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -542
static const yytype_int16 yypact[] =
{
     529,  1023,   266,  -542,   -47,  -542,  -542,  -542,    -5,    -6,
      67,    76,   143,  -542,    67,  -542,   177,    11,  -542,   283,
      90,  -542,  -542,  -542,  -542,  -542,    56,    58,  -542,  -542,
    1023,  1023,   697,  -542,  -542,  -542,  -542,   174,  -542,   303,
     260,  1023,   248,    -9,   192,    74,   333,    82,   139,   117,
     263,    33,  -542,  -542,  -542,   181,    98,   217,   237,   788,
     300,   184,   212,   347,   388,   401,  1235,   259,   328,  -542,
      17,  -542,    67,  -542,  -542,   266,  -542,  -542,  -542,  -542,
    -542,  -542,  -542,  -542,  -542,  -542,  -542,   788,  -542,   427,
    -542,   403,  -542,   788,  -542,   427,  -542,   403,  -542,   788,
    -542,   788,  -542,   788,  -542,  -542,  -542,  -542,   157,  -542,
    -542,   788,  -542,  -542,  -542,  -542,  -542,  -542,   509,   521,
     538,  -542,   444,   441,  -542,  -542,    88,  -542,   -57,  -542,
     283,   449,   301,   740,  -542,    67,   451,   841,  -542,  -542,
     281,   177,  -542,   453,  -542,  -542,  -542,   862,  1023,  -542,
    -542,  -542,  -542,  -542,  1023,  -542,  1023,  -542,  -542,  -542,
    -542,  -542,  -542,  1023,  1023,  -542,  -542,  -542,  -542,  -542,
    -542,  -542,  -542,  -542,  -542,  -542,  1023,  1023,  -542,    91,
    1023,  1023,  1242,    26,   533,   109,  -542,  -542,   255,   255,
     255,    18,  -542,   462,  -542,   465,   -46,   471,   244,    78,
    -542,  -542,   472,   474,  -542,   482,  -542,  -542,   266,  -542,
     452,  -542,  -542,  -542,   467,  -542,   896,  -542,  -542,  -542,
    -542,  -542,  -542,  -542,  -542,  -542,   513,   514,   527,  -542,
     293,  -542,  -542,  -542,    67,  -542,  -542,   449,  -542,  -542,
    -542,    49,   545,   522,  -542,   343,  -542,   557,  -542,   449,
    -542,   357,  -542,  -542,  -542,   566,  -542,   358,   118,  -542,
    -542,  1023,  1023,  1023,  1023,  1023,  -542,    -9,    -9,  1023,
    1023,  1023,  1023,  1023,  1023,  1023,  1023,  1023,  1023,  1023,
      20,  -542,  -542,   359,   375,   561,   386,   187,   450,   251,
     368,   440,  -542,  -542,  1172,   524,  -542,  1081,  -542,   670,
    -542,   564,  -542,   576,   584,   591,  -542,  -542,  -542,   593,
    1094,  1149,  -542,  -542,   979,  -542,  -542,   406,   109,  1287,
    -542,    12,   788,   874,  -542,  -542,   595,   723,   206,  -542,
     932,  -542,    72,   949,   128,  -542,   284,  -542,  -542,   909,
    -542,   725,  -542,  -542,  -542,  1023,   729,  -542,  -542,  -542,
    -542,  -542,   248,   248,   192,   192,   192,   192,    74,    74,
     333,    82,   139,   117,   263,  1023,  -542,   788,  1023,   878,
     199,   468,   953,   222,   255,   732,   734,  1194,  1121,  -542,
    -542,  -542,  -542,   629,   630,  -542,   639,  -542,   647,   714,
    -542,  -542,  -542,  -542,  -542,  1289,  -542,  -542,  -542,  -542,
    1300,  -542,  -542,  -542,   659,  -542,  -542,  -542,  -542,   745,
    -542,  -542,  -542,   661,   667,  1206,  -542,  -542,  -542,  -542,
     675,  -542,   694,  -542,  1310,  -542,  -542,  -542,  -542,  -542,
     695,  -542,   698,  -542,   -16,  -542,  -542,   539,  -542,   224,
     640,   753,  -542,   788,  -542,  -542,   376,  -542,  -542,   711,
     712,   713,   728,     6,  -542,  -542,   729,  -542,  -542,  -542,
     701,   708,  -542,   731,   738,   284,   355,   986,  -542,  -542,
    -542,  1023,  -542,   287,   932,   -41,  -542,   733,  -542,   788,
    -542,   391,   788,   397,   990,  1004,   232,   788,   398,  1008,
     321,   399,   423,   714,  -542,  -542,  -542,  -542,  1256,  -542,
    -542,  -542,  -542,  -542,   742,   755,   757,   759,   345,   116,
     760,  -542,   768,  -542,  -542,  -542,  -542,  -542,  -542,   442,
     622,  1023,  -542,  -542,  -542,  -542,  1061,  1023,  -542,  -542,
     771,   772,   780,  -542,   332,  -542,  -542,   468,  -542,  -542,
    -542,   116,   823,   823,   823,  -542,   302,  -542,  -542,  -542,
    -542,  -542,   783,  -542,   784,   790,  -542,   791,  -542,   588,
    -542,  -542,  -542,   864,   781,  -542,   788,   788,   443,   788,
     446,  1019,  -542,   788,   788,   473,   798,  1023,   799,  1023,
     801,  1023,  -542,  -542,   745,   745,   745,   745,   804,   812,
    -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,
    -542,   745,  1023,   745,  1023,  -542,   899,  -542,  -542,  -542,
    -542,  -542,  -542,   827,   495,   497,  1023,  1023,   833,   837,
     838,  1023,  1023,   324,  -542,   234,   820,   843,   505,  -542,
     506,   508,  -542,   615,  -542,  -542,  -542,  -542,  -542,  -542,
     932,   788,  -542,  -542,  -542,   788,  -542,   788,   788,   534,
    -542,  -542,   788,  -542,   845,  -542,   846,  -542,   854,   558,
     583,   585,   604,  -542,  -542,   839,   605,   636,   663,   702,
    -542,  -542,  -542,  -542,  -542,   709,   724,  -542,  -542,  -542,
     865,   866,  -542,   867,   875,  1019,   823,   515,  -542,   823,
    -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,   754,  -542,
     858,   870,   877,  -542,  -542,  -542,  -542,   745,  -542,  -542,
    -542,  -542,  1023,  1023,  -542,  -542,  -542,  -542,   369,  -542,
     726,   769,   965,   907,  -542,   266,  -542,  -542,  -542,  -542,
     805,   808,   811,  -542,  -542,   788,  -542,   911,  -542,   646,
    -542,  -542,  -542,  -542,  -542
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -542,  -542,  -542,  -542,  -169,  -134,   -24,  -542,   167,  -542,
    -542,  -542,   361,  -542,  -542,   776,   262,  -542,  -542,  -542,
    -542,   744,  -542,  -542,   747,  -542,   750,  -542,   761,  -542,
     752,  -542,  -542,  -542,  -542,  -294,  -129,  -542,     2,  -542,
      10,     5,  -542,   806,  -542,     8,  1024,     1,  -321,  -542,
     710,   720,   719,   916,    22,    -8,  -542,  -542,  -315,   616,
    -526,  -542,   598,    -2,   -93,  -542,  -206,   -74,   968,  -542,
    -542,  -542,    43,  -542,  -542,  -542,  -542,  1057,  -542,  1052,
    -542,  -542,  -542,  -542,   -64,  -542,  -542,  -542,   690,  -326,
    -542,  -542,  -542,  -542,  -542,   666,  -364,  -542,  -542,  -542,
    -542,  -542,  -542,   687,   987,  -542,   762,  -542,  -542,  -542,
     685,  -542,  -542,  -542,   592,  -542,  -542,  -542,   596,  -542,
    -542,  -542,   662,  -542,  -542,  -542,  -542,  -542,  -542,  -542,
    -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,  -542,
    -542,  -542,  -269,  -542,  -542,  -542,  -542,  -542,  -542,  -542,
     553,  -541,  -542,  -542,  -542,  -542,  -542,  -542,   569,  -542,
    -542,   127,  -542,  -542,  -542,  -542,   652,  -521,   419,  -542,
    -542,  -542,   678,  -542
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -378
static const yytype_int16 yytable[] =
{
      73,   211,   317,    55,   247,    14,   138,   139,   252,   130,
      13,   460,   124,   257,   501,   458,     3,   157,   252,   289,
     290,   291,   630,   631,   129,    14,   398,   260,   403,   218,
      13,   135,   122,   640,   140,   221,   128,   222,   238,   223,
     141,   419,   428,   659,   660,   661,   662,   225,   281,  -208,
     441,   495,   500,   175,     3,   429,   299,   184,   179,   292,
     666,     3,   668,   471,     4,     5,   236,     6,     7,   119,
     449,   478,     3,    73,   430,   431,   432,     3,   121,   300,
       4,     5,   561,     6,     7,   433,   293,     6,     7,   165,
     166,   450,   451,   452,   210,     3,   204,   120,     4,     5,
     161,     6,     7,    27,    28,    29,   162,   640,   497,   503,
     121,    30,    31,    10,    27,    28,    29,     4,     5,   442,
       6,     7,    30,    31,   233,   177,   398,    25,   177,     8,
     259,   403,   215,     3,   501,   590,   591,   241,   219,   266,
     266,   209,   365,   125,   331,   552,   419,    25,     8,   141,
     258,   332,   320,   333,   176,   428,   343,   248,    10,   560,
     137,    11,    27,    28,    29,   721,   730,   495,    11,    10,
      30,    31,   500,     9,   332,   459,   333,   556,   280,    11,
     307,   144,   283,   284,    11,     4,     5,   287,    11,   446,
     167,   168,    10,   171,   252,   288,   234,   592,   593,   177,
      32,   457,    11,   308,   464,   490,    73,    33,    34,    35,
      36,    32,   235,   163,   164,   282,   470,   457,    33,    34,
      35,    36,   178,   348,   497,   594,   177,   595,   596,   503,
     332,   597,   333,   286,   598,   599,   173,   349,   350,   351,
     266,   266,   334,   211,   639,   266,   266,   266,   266,   266,
     266,   266,   266,   266,   266,   266,   210,   172,   444,    32,
      27,    28,    29,   337,   145,   146,    33,    34,    35,    36,
     449,    27,    28,    29,     4,     5,    56,     6,     7,    30,
      31,    69,   174,   149,   150,   151,   152,   153,   370,   177,
     373,   450,   451,   452,    57,   177,    58,    59,    60,    61,
      62,    63,    64,    65,    66,     8,   239,   177,   186,     4,
       5,   371,     6,     7,   453,    73,   145,   146,   639,   180,
     439,   303,   304,   484,   305,   693,   130,   457,   288,   204,
     177,   454,   177,   334,   145,   146,   187,   241,   555,   181,
     177,   477,   177,   121,   241,   457,   489,   205,   537,    67,
     169,   170,   206,   147,   122,   148,   571,    32,   685,   374,
     158,   159,   160,     4,     5,   480,     6,     7,    32,   306,
     481,   483,    68,   486,   488,    33,    34,    35,    36,   539,
     154,   145,   146,   155,   253,   132,   466,   133,   467,   177,
      69,    27,    28,    29,   558,   559,   326,   619,   620,    30,
      31,   327,   185,   529,   240,   147,   588,   148,   589,   632,
     633,    27,    28,    29,     4,     5,    56,     6,     7,    30,
      31,   682,   683,   147,   576,   148,   530,   531,   532,   577,
     457,   352,   353,   533,    57,   207,    58,    59,    60,    61,
      62,    63,    64,    65,    66,     8,   340,   477,   654,   188,
     656,   341,   658,   145,   146,    27,    28,    29,   551,   214,
     344,   347,   366,    30,    31,   345,   345,   177,   733,   734,
     147,   241,   148,    27,    28,    29,   375,   563,   367,   540,
     565,    30,    31,   177,   345,   572,   568,   570,    32,    67,
     189,   575,   680,   681,   564,    33,    34,    35,    36,   177,
     566,   573,   578,   190,   457,   177,   177,   579,    32,   216,
     369,   457,    68,   438,   226,    33,    34,    35,    36,   739,
      27,    28,    29,   615,   267,   268,   580,   228,    30,    31,
      69,   581,     1,     2,     3,   358,   359,     4,     5,   625,
       6,     7,   147,   229,   148,   605,   645,   232,   376,   647,
     606,   177,    32,    11,   177,    68,   255,  -377,   723,    33,
      34,    35,    36,   285,   643,   644,   394,   646,     8,   318,
      32,   650,   651,   649,   372,   236,   652,    33,    34,    35,
      36,   177,   379,   380,   381,   382,   383,   296,   384,   385,
     298,   387,   485,    27,    28,    29,   302,   312,   672,   313,
     674,    30,    31,   673,   667,   177,   669,   315,   688,   690,
     529,   691,     9,   689,   689,   323,   689,    32,   675,   676,
      27,    28,    29,   607,    33,    34,    35,    36,    30,    31,
     339,    10,   395,   530,   531,   532,   722,   698,   324,   694,
     533,    11,   177,   695,   325,   696,   697,   534,   338,   396,
     699,    27,    28,    29,     4,     5,    56,     6,     7,    30,
      31,   703,   342,   368,   535,   211,   606,   608,   609,   610,
     611,   612,   346,   613,    57,   404,    58,    59,    60,    61,
      62,    63,    64,    65,    66,     8,   704,   720,   705,   405,
      32,   606,   471,   606,   456,   638,   726,    33,    34,    35,
      36,   406,    27,    28,    29,     4,     5,   706,   708,   407,
      30,    31,   606,   606,   731,   732,   408,    32,   410,   471,
     447,   456,   692,    73,    33,    34,    35,    36,   448,    67,
     469,   508,   509,   726,    27,    28,    29,    73,   491,   709,
     492,   511,    30,    31,   177,    27,    28,    29,    32,   513,
     518,   744,    68,    30,    31,    33,    34,    35,    36,    27,
      28,    29,   517,   520,    56,   538,   710,    30,    31,   521,
      69,   606,   379,   380,   381,   382,   383,   523,   384,   385,
     386,   387,    57,   388,    58,    59,    60,    61,    62,    63,
      64,    65,   182,    27,    28,    29,   524,   526,    56,    32,
     527,    30,    31,   440,   547,   711,    33,    34,    35,    36,
     177,   548,   714,   541,   542,   543,    57,   177,    58,    59,
      60,    61,    62,    63,    64,    65,   182,   715,   627,   735,
     544,    32,   177,   471,   177,   456,   549,   183,    33,    34,
      35,    36,    32,   550,   584,   246,    27,    28,    29,    33,
      34,    35,    36,   154,    30,    31,    32,   585,   725,   586,
      68,   587,   601,    33,    34,    35,    36,    27,    28,    29,
     603,   183,   736,   621,   622,    30,    31,   689,    69,    27,
      28,    29,   623,    27,    28,    29,   634,    30,    31,   635,
      32,    30,    31,   641,    68,   636,   637,    33,    34,    35,
      36,    27,    28,    29,   670,   642,    56,   663,   740,    30,
      31,   741,    69,   606,   742,   664,   177,     4,     5,   177,
       6,     7,   653,   655,    57,   657,    58,    59,    60,    61,
      62,    63,    64,    65,   319,   671,   677,    27,    28,    29,
     678,   679,   686,    32,   250,    30,    31,   687,   700,   701,
      33,    34,    35,    36,    27,    28,    29,   702,    27,    28,
      29,   707,    30,    31,    32,   256,    30,    31,   716,   717,
     737,    33,    34,    35,    36,   718,    32,   445,   719,   183,
      32,   482,   727,    33,    34,    35,    36,    33,    34,    35,
      36,    27,    28,    29,   728,    27,    28,    29,    32,    30,
      31,   729,    68,    30,    31,    33,    34,    35,    36,    27,
      28,    29,   738,    27,    28,    29,   743,    30,    31,   360,
      69,    30,    31,   361,    27,    28,    29,   362,    27,    28,
      29,   364,    30,    31,    32,   123,    30,    31,   456,   363,
     329,    33,    34,    35,    36,   354,   355,   356,   357,   468,
     429,    32,   461,   465,   462,    32,   487,   254,    33,    34,
      35,   463,    33,    34,    35,    36,    27,    28,    29,   430,
     431,   432,   546,   562,    30,    31,   224,   134,   136,   514,
     433,   522,   515,   443,   220,   516,   525,   434,    32,   582,
     583,   553,    32,   567,   626,    33,    34,    35,   554,    33,
      34,    35,    36,   624,   435,   545,    32,   569,   724,     0,
      32,   574,   528,    33,    34,    35,    36,    33,    34,    35,
      36,    32,   648,   399,     0,    32,     0,     0,    33,    34,
      35,    36,    33,    34,    35,    36,   411,   412,   413,   379,
     380,   381,   382,   383,     0,   384,   385,     0,   387,     0,
       0,     0,   379,   380,   381,   382,   383,     0,   384,   385,
       0,   387,     0,    32,   412,   413,     0,   414,     0,     0,
      33,    34,    35,   618,     0,     0,     0,     0,     0,   379,
     380,   381,   382,   383,     0,   384,   385,   386,   387,   400,
     388,     0,     0,     0,   414,     0,     0,     0,     0,     0,
       0,     0,   415,     0,     0,     0,   401,   379,   380,   381,
     382,   383,     0,   384,   385,     0,   387,     0,   420,   416,
     421,     0,     0,   422,   423,     0,     0,     0,     0,   498,
     379,   380,   381,   382,   383,     0,   384,   385,   386,   387,
       0,   388,     0,     0,     0,     0,   499,     0,   411,   412,
     413,     0,   379,   380,   381,   382,   383,   424,   384,   385,
     386,   387,     0,   388,   379,   380,   381,   382,   383,     0,
     384,   385,     0,   387,   425,   191,   192,     0,   193,   414,
     389,     0,   191,   192,     0,   193,     0,   194,   195,   196,
     197,   198,   199,     0,   194,   195,   196,   390,   198,   412,
     413,     0,   493,   200,     0,   201,     0,   202,     0,     0,
     200,   203,   201,     0,   379,   380,   381,   382,   383,   494,
     384,   385,   386,   387,     0,   388,     0,   191,   192,   414,
     193,   394,     0,     0,     0,     0,     0,   440,     0,   194,
     195,   196,   399,   198,     0,     0,     0,   379,   380,   381,
     382,   383,     0,   384,   385,   200,   387,   201,   379,   380,
     381,   382,   383,     0,   384,   385,     0,   387,   379,   380,
     381,   382,   383,     0,   384,   385,     0,   387,     0,   420,
       0,   421,     0,     0,   422,   423
};

static const yytype_int16 yycheck[] =
{
       2,    75,   208,     1,   133,     0,    30,    31,   137,    17,
       0,   332,    11,   147,   378,   330,     5,    41,   147,   188,
     189,   190,   543,   544,    16,    20,   295,   156,   297,    93,
      20,    26,    10,   559,    32,    99,    14,   101,   131,   103,
      32,   310,   311,   584,   585,   586,   587,   111,   177,   106,
      38,   377,   378,    20,     5,    71,   102,    59,    56,    41,
     601,     5,   603,   104,     8,     9,   123,    11,    12,   116,
      64,   365,     5,    75,    90,    91,    92,     5,    84,   125,
       8,     9,   123,    11,    12,   101,    68,    11,    12,    15,
      16,    85,    86,    87,    72,     5,    70,   102,     8,     9,
     109,    11,    12,     5,     6,     7,   115,   633,   377,   378,
      84,    13,    14,   102,     5,     6,     7,     8,     9,   107,
      11,    12,    13,    14,   123,   108,   395,     0,   108,    39,
     154,   400,    89,     5,   498,    19,    20,   132,    95,   163,
     164,   124,   122,     0,   237,   466,   415,    20,    39,   141,
     148,   102,   216,   104,   121,   424,   249,   135,   102,   474,
     102,   112,     5,     6,     7,   686,   707,   493,   112,   102,
      13,    14,   498,    83,   102,   103,   104,   471,   176,   112,
     102,     7,   180,   181,   112,     8,     9,   185,   112,   323,
     116,   117,   102,   111,   323,   185,   108,    81,    82,   108,
     102,   330,   112,   125,   333,   374,   208,   109,   110,   111,
     112,   102,   124,    21,    22,   124,   345,   346,   109,   110,
     111,   112,   124,   105,   493,   109,   108,   111,   112,   498,
     102,   115,   104,   124,   118,   119,   119,   261,   262,   263,
     264,   265,   241,   317,   559,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   234,   118,   322,   102,
       5,     6,     7,   241,    13,    14,   109,   110,   111,   112,
      64,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,   124,    19,    23,    24,    25,    26,    27,   286,   108,
     288,    85,    86,    87,    28,   108,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,     5,   108,   124,     8,
       9,   124,    11,    12,   108,   317,    13,    14,   633,   102,
     318,    77,    78,   124,    80,   640,   334,   456,   318,    70,
     108,   125,   108,   332,    13,    14,   124,   332,   467,   102,
     108,   365,   108,    84,   339,   474,   124,    88,   124,    83,
      17,    18,    93,   102,   332,   104,   124,   102,   124,   108,
     112,   113,   114,     8,     9,   367,    11,    12,   102,   125,
     368,   369,   106,   371,   372,   109,   110,   111,   112,   443,
     120,    13,    14,   123,   103,   102,   102,   104,   104,   108,
     124,     5,     6,     7,   107,   108,   103,   526,   527,    13,
      14,   108,   102,    71,   103,   102,    61,   104,    63,   107,
     108,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    97,    98,   102,   103,   104,    94,    95,    96,   108,
     559,   264,   265,   101,    28,   107,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,   103,   471,   577,   102,
     579,   108,   581,    13,    14,     5,     6,     7,   103,    32,
     103,   103,   103,    13,    14,   108,   108,   108,    99,   100,
     102,   466,   104,     5,     6,     7,   108,   479,   103,   103,
     482,    13,    14,   108,   108,   487,   484,   485,   102,    83,
     102,   489,   621,   622,   103,   109,   110,   111,   112,   108,
     103,   103,   103,   102,   633,   108,   108,   108,   102,   106,
     124,   640,   106,   107,     5,   109,   110,   111,   112,   725,
       5,     6,     7,   521,   163,   164,   103,     6,    13,    14,
     124,   108,     3,     4,     5,   273,   274,     8,     9,   537,
      11,    12,   102,     5,   104,   103,   103,   103,   108,   103,
     108,   108,   102,   112,   108,   106,   103,   106,   687,   109,
     110,   111,   112,    30,   566,   567,    42,   569,    39,   102,
     102,   573,   574,   571,   124,   123,   103,   109,   110,   111,
     112,   108,    58,    59,    60,    61,    62,   125,    64,    65,
     125,    67,   124,     5,     6,     7,   125,   125,   103,   125,
     103,    13,    14,   108,   602,   108,   604,   125,   103,   103,
      71,   103,    83,   108,   108,   102,   108,   102,   616,   617,
       5,     6,     7,     1,   109,   110,   111,   112,    13,    14,
     108,   102,   108,    94,    95,    96,   121,   103,   124,   641,
     101,   112,   108,   645,   117,   647,   648,   108,   103,   125,
     652,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,   103,   105,   102,   125,   739,   108,    45,    46,    47,
      48,    49,   106,    51,    28,     5,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,   103,   685,   103,   125,
     102,   108,   104,   108,   106,   107,   698,   109,   110,   111,
     112,   125,     5,     6,     7,     8,     9,   103,   103,   125,
      13,    14,   108,   108,   712,   713,   125,   102,   125,   104,
     125,   106,   107,   725,   109,   110,   111,   112,     5,    83,
       5,   102,   102,   735,     5,     6,     7,   739,     6,   103,
       6,   102,    13,    14,   108,     5,     6,     7,   102,   102,
       5,   105,   106,    13,    14,   109,   110,   111,   112,     5,
       6,     7,   103,   102,    10,   125,   103,    13,    14,   102,
     124,   108,    58,    59,    60,    61,    62,   102,    64,    65,
      66,    67,    28,    69,    30,    31,    32,    33,    34,    35,
      36,    37,    38,     5,     6,     7,   102,   102,    10,   102,
     102,    13,    14,    50,   103,   103,   109,   110,   111,   112,
     108,   103,   103,   102,   102,   102,    28,   108,    30,    31,
      32,    33,    34,    35,    36,    37,    38,   103,     5,   103,
     102,   102,   108,   104,   108,   106,   105,    83,   109,   110,
     111,   112,   102,   105,   102,   105,     5,     6,     7,   109,
     110,   111,   112,   120,    13,    14,   102,   102,   104,   102,
     106,   102,   102,   109,   110,   111,   112,     5,     6,     7,
     102,    83,   103,   102,   102,    13,    14,   108,   124,     5,
       6,     7,   102,     5,     6,     7,   103,    13,    14,   105,
     102,    13,    14,    29,   106,   105,   105,   109,   110,   111,
     112,     5,     6,     7,     5,   124,    10,   103,   103,    13,
      14,   103,   124,   108,   103,   103,   108,     8,     9,   108,
      11,    12,   124,   124,    28,   124,    30,    31,    32,    33,
      34,    35,    36,    37,    38,   108,   103,     5,     6,     7,
     103,   103,   122,   102,   103,    13,    14,   104,   103,   103,
     109,   110,   111,   112,     5,     6,     7,   103,     5,     6,
       7,   122,    13,    14,   102,   103,    13,    14,   103,   103,
       5,   109,   110,   111,   112,   108,   102,   103,   103,    83,
     102,   103,   124,   109,   110,   111,   112,   109,   110,   111,
     112,     5,     6,     7,   124,     5,     6,     7,   102,    13,
      14,   124,   106,    13,    14,   109,   110,   111,   112,     5,
       6,     7,   105,     5,     6,     7,   105,    13,    14,   275,
     124,    13,    14,   276,     5,     6,     7,   277,     5,     6,
       7,   279,    13,    14,   102,    11,    13,    14,   106,   278,
     234,   109,   110,   111,   112,   269,   270,   271,   272,   339,
      71,   102,   332,   334,   105,   102,   103,   141,   109,   110,
     111,   112,   109,   110,   111,   112,     5,     6,     7,    90,
      91,    92,   456,   475,    13,    14,   108,    20,    26,   389,
     101,   415,   395,   321,    97,   400,   424,   108,   102,   493,
     498,   105,   102,   103,   541,   109,   110,   111,   112,   109,
     110,   111,   112,   534,   125,   453,   102,   103,   689,    -1,
     102,   103,   434,   109,   110,   111,   112,   109,   110,   111,
     112,   102,   103,    42,    -1,   102,    -1,    -1,   109,   110,
     111,   112,   109,   110,   111,   112,    42,    43,    44,    58,
      59,    60,    61,    62,    -1,    64,    65,    -1,    67,    -1,
      -1,    -1,    58,    59,    60,    61,    62,    -1,    64,    65,
      -1,    67,    -1,   102,    43,    44,    -1,    73,    -1,    -1,
     109,   110,   111,   112,    -1,    -1,    -1,    -1,    -1,    58,
      59,    60,    61,    62,    -1,    64,    65,    66,    67,   108,
      69,    -1,    -1,    -1,    73,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   108,    -1,    -1,    -1,   125,    58,    59,    60,
      61,    62,    -1,    64,    65,    -1,    67,    -1,    69,   125,
      71,    -1,    -1,    74,    75,    -1,    -1,    -1,    -1,   108,
      58,    59,    60,    61,    62,    -1,    64,    65,    66,    67,
      -1,    69,    -1,    -1,    -1,    -1,   125,    -1,    42,    43,
      44,    -1,    58,    59,    60,    61,    62,   108,    64,    65,
      66,    67,    -1,    69,    58,    59,    60,    61,    62,    -1,
      64,    65,    -1,    67,   125,    40,    41,    -1,    43,    73,
     108,    -1,    40,    41,    -1,    43,    -1,    52,    53,    54,
      55,    56,    57,    -1,    52,    53,    54,   125,    56,    43,
      44,    -1,   108,    68,    -1,    70,    -1,    72,    -1,    -1,
      68,    76,    70,    -1,    58,    59,    60,    61,    62,   125,
      64,    65,    66,    67,    -1,    69,    -1,    40,    41,    73,
      43,    42,    -1,    -1,    -1,    -1,    -1,    50,    -1,    52,
      53,    54,    42,    56,    -1,    -1,    -1,    58,    59,    60,
      61,    62,    -1,    64,    65,    68,    67,    70,    58,    59,
      60,    61,    62,    -1,    64,    65,    -1,    67,    58,    59,
      60,    61,    62,    -1,    64,    65,    -1,    67,    -1,    69,
      -1,    71,    -1,    -1,    74,    75
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     3,     4,     5,     8,     9,    11,    12,    39,    83,
     102,   112,   127,   166,   167,   171,   172,   173,   180,   181,
     202,   203,   204,   205,   267,   287,   289,     5,     6,     7,
      13,    14,   102,   109,   110,   111,   112,   128,   129,   130,
     132,   133,   134,   138,   141,   142,   147,   150,   152,   154,
     156,   158,   160,   161,   162,   164,    10,    28,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    83,   106,   124,
     164,   166,   167,   189,   190,   192,   193,   194,   196,   197,
     198,   199,   200,   201,   208,   209,   211,   212,   218,   219,
     226,   227,   233,   234,   237,   238,   241,   242,   245,   246,
     252,   253,   254,   255,   257,   258,   259,   260,   261,   262,
     265,   266,   279,   280,   281,   286,   287,   295,   296,   116,
     102,    84,   180,   172,   173,     0,   168,   169,   180,   171,
     181,   207,   102,   104,   203,   167,   205,   102,   132,   132,
     164,   171,   179,   183,     7,    13,    14,   102,   104,    23,
      24,    25,    26,    27,   120,   123,   163,   132,   112,   113,
     114,   109,   115,    21,    22,    15,    16,   116,   117,    17,
      18,   111,   118,   119,    19,    20,   121,   108,   124,   164,
     102,   102,    38,    83,   189,   102,   124,   124,   102,   102,
     102,    40,    41,    43,    52,    53,    54,    55,    56,    57,
      68,    70,    72,    76,    70,    88,    93,   107,   191,   124,
     180,   193,   189,   210,    32,   198,   106,   230,   210,   198,
     230,   210,   210,   210,   194,   210,     5,   299,     6,     5,
     278,   290,   103,   173,   108,   124,   123,   206,   190,     5,
     103,   167,   174,   175,   176,   182,   105,   162,   180,   288,
     103,   131,   162,   103,   179,   103,   103,   131,   164,   132,
     162,   135,   136,   137,   140,   139,   132,   138,   138,   145,
     146,   143,   144,   148,   149,   151,   153,   155,   157,   159,
     164,   162,   124,   164,   164,    30,   124,   164,   166,   130,
     130,   130,    41,    68,   213,   228,   125,   235,   125,   102,
     125,   256,   125,    77,    78,    80,   125,   102,   125,   263,
     220,   247,   125,   125,   297,   125,   282,   192,   102,    38,
     210,   231,   232,   102,   124,   117,   103,   108,   291,   169,
     170,   190,   102,   104,   173,   177,   178,   180,   103,   108,
     103,   108,   105,   190,   103,   108,   106,   103,   105,   132,
     132,   132,   134,   134,   141,   141,   141,   141,   142,   142,
     147,   150,   152,   154,   156,   122,   103,   103,   102,   124,
     164,   124,   124,   164,   108,   108,   108,   243,   239,    58,
      59,    60,    61,    62,    64,    65,    66,    67,    69,   108,
     125,   214,   215,   268,    42,   108,   125,   229,   268,    42,
     108,   125,   236,   268,     5,   125,   125,   125,   125,   264,
     125,    42,    43,    44,    73,   108,   125,   221,   222,   268,
      69,    71,    74,    75,   108,   125,   248,   249,   268,    71,
      90,    91,    92,   101,   108,   125,   298,   283,   107,   164,
      50,    38,   107,   232,   210,   103,   131,   125,     5,    64,
      85,    86,    87,   108,   125,   292,   106,   162,   184,   103,
     174,   177,   105,   112,   162,   178,   102,   104,   176,     5,
     162,   104,   184,   185,   186,   187,   188,   132,   161,   195,
     189,   164,   103,   164,   124,   124,   164,   103,   164,   124,
     130,     6,     6,   108,   125,   215,   244,   268,   108,   125,
     215,   222,   240,   268,   269,   271,   272,   273,   102,   102,
     275,   102,   270,   102,   214,   229,   236,   103,     5,   277,
     102,   102,   221,   102,   102,   248,   102,   102,   298,    71,
      94,    95,    96,   101,   108,   125,   284,   124,   125,   210,
     103,   102,   102,   102,   102,   292,   185,   103,   103,   105,
     105,   103,   174,   105,   112,   162,   161,   165,   107,   108,
     184,   123,   188,   189,   103,   189,   103,   103,   164,   103,
     164,   124,   189,   103,   103,   164,   103,   108,   103,   108,
     103,   108,   244,   240,   102,   102,   102,   102,    61,    63,
      19,    20,    81,    82,   109,   111,   112,   115,   118,   119,
     276,   102,   217,   102,   216,   103,   108,     1,    45,    46,
      47,    48,    49,    51,   225,   164,   250,   251,   112,   162,
     162,   102,   102,   102,   284,   164,   276,     5,   293,   294,
     293,   293,   107,   108,   103,   105,   105,   105,   107,   184,
     186,    29,   124,   189,   189,   103,   189,   103,   103,   164,
     189,   189,   103,   124,   162,   124,   162,   124,   162,   277,
     277,   277,   277,   103,   103,   274,   277,   164,   277,   164,
       5,   108,   103,   108,   103,   164,   164,   103,   103,   103,
     162,   162,    97,    98,   285,   124,   122,   104,   103,   108,
     103,   103,   107,   184,   189,   189,   189,   189,   103,   189,
     103,   103,   103,   103,   103,   103,   103,   122,   103,   103,
     103,   103,   224,   223,   103,   103,   103,   103,   108,   103,
     164,   293,   121,   162,   294,   104,   189,   124,   124,   124,
     277,   164,   164,    99,   100,   103,   103,     5,   105,   192,
     103,   103,   103,   105,   105
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;


      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 304 "parser.y"
    { /* to avoid warnings */ }
    break;

  case 3:
#line 305 "parser.y"
    { pastree_expr = (yyvsp[(2) - (2)].expr); }
    break;

  case 4:
#line 306 "parser.y"
    { pastree_stmt = (yyvsp[(2) - (2)].stmt); }
    break;

  case 5:
#line 331 "parser.y"
    {
      (yyval.string) = strdup((yyvsp[(1) - (1)].name));
    }
    break;

  case 6:
#line 335 "parser.y"
    {
      /* Or we could leave it as is (as a SpaceList) */
      if (((yyvsp[(1) - (2)].string) = realloc((yyvsp[(1) - (2)].string), strlen((yyvsp[(1) - (2)].string)) + strlen((yyvsp[(2) - (2)].name)))) == NULL)
        parse_error(-1, "string out of memory\n");
      strcpy(((yyvsp[(1) - (2)].string))+(strlen((yyvsp[(1) - (2)].string))-1),((yyvsp[(2) - (2)].name))+1);  /* Catenate on the '"' */
      (yyval.string) = (yyvsp[(1) - (2)].string);
    }
    break;

  case 7:
#line 358 "parser.y"
    {
      symbol  id = Symbol((yyvsp[(1) - (1)].name));
      stentry e;
      int     chflag = 0;
    
      if (checkDecls)
      {
        check_uknown_var((yyvsp[(1) - (1)].name));
       
      }
      (yyval.expr) = chflag ? UnaryOperator(UOP_paren,
                             UnaryOperator(UOP_star, Identifier(id)))
                  : Identifier(id);
      /* set the security flag for the identifier */
      set_security_flag_symbol((yyval.expr), id);
      /* set the size of identifier */
      set_size_symbol((yyval.expr), Identifier(id), NULL);
    }
    break;

  case 8:
#line 377 "parser.y"
    {
      (yyval.expr) = Constant( strdup((yyvsp[(1) - (1)].name)) );
      (yyval.expr)->size = -1;
      set_security_flag_expr((yyval.expr), NULL, NULL, -1);
    }
    break;

  case 9:
#line 383 "parser.y"
    {
      (yyval.expr) = String((yyvsp[(1) - (1)].string));
      (yyval.expr)->size = -1;
    }
    break;

  case 10:
#line 388 "parser.y"
    {
      (yyval.expr) = UnaryOperator(UOP_paren, (yyvsp[(2) - (3)].expr));
      (yyval.expr)->size = (yyvsp[(2) - (3)].expr)->size;
      set_security_flag_expr((yyval.expr), (yyvsp[(2) - (3)].expr), NULL, -1);
    }
    break;

  case 11:
#line 398 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (1)].expr);
    }
    break;

  case 12:
#line 402 "parser.y"
    {
      (yyval.expr) = ArrayIndex((yyvsp[(1) - (4)].expr), (yyvsp[(3) - (4)].expr));
      set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), NULL, -1);
      set_size_symbol((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(3) - (4)].expr));
    }
    break;

  case 13:
#line 414 "parser.y"
    {
      /* Catch calls to "main()" (unlikely but possible) */
      (yyval.expr) = strcmp((yyvsp[(1) - (3)].name), "main") ?
             FunctionCall(Identifier(Symbol((yyvsp[(1) - (3)].name))), NULL) :
             FunctionCall(Identifier(Symbol(MAIN_NEWNAME)), NULL);
      set_security_flag_func((yyvsp[(1) - (3)].name), (yyval.expr));
    }
    break;

  case 14:
#line 422 "parser.y"
    {
      /* Catch calls to "main()" (unlikely but possible) */
      if (check_func_param(Identifier(Symbol((yyvsp[(1) - (4)].name))), (yyvsp[(3) - (4)].expr)))
          parse_error(1, "The provided arguments do not match function parameters\n");
      (yyval.expr) = strcmp((yyvsp[(1) - (4)].name), "main") ?
             FunctionCall(Identifier(Symbol((yyvsp[(1) - (4)].name))), (yyvsp[(3) - (4)].expr)) :
             FunctionCall(Identifier(Symbol(MAIN_NEWNAME)), (yyvsp[(3) - (4)].expr));
      // set the flag of func expr based on its return type
      set_security_flag_func((yyvsp[(1) - (4)].name), (yyval.expr));
    }
    break;

  case 15:
#line 433 "parser.y"
    {
        (yyval.expr) = FunctionCall((yyvsp[(1) - (3)].expr), NULL);
    }
    break;

  case 16:
#line 437 "parser.y"
    {
        (yyval.expr) = FunctionCall((yyvsp[(1) - (4)].expr), (yyvsp[(3) - (4)].expr));
    }
    break;

  case 17:
#line 441 "parser.y"
    {
        (yyval.expr) = PostOperator((yyvsp[(1) - (2)].expr), UOP_inc);
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (2)].expr), NULL, -1);
        (yyval.expr)->size = (yyvsp[(1) - (2)].expr)->size;
    }
    break;

  case 18:
#line 447 "parser.y"
    {
        (yyval.expr) = PostOperator((yyvsp[(1) - (2)].expr), UOP_dec);
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (2)].expr), NULL, -1);
        (yyval.expr)->size = (yyvsp[(1) - (2)].expr)->size; 
   }
    break;

  case 19:
#line 454 "parser.y"
    {
      (yyval.expr) = CastedExpr((yyvsp[(2) - (6)].decl), BracedInitializer((yyvsp[(5) - (6)].expr)));
    }
    break;

  case 20:
#line 458 "parser.y"
    {
      (yyval.expr) = CastedExpr((yyvsp[(2) - (7)].decl), BracedInitializer((yyvsp[(5) - (7)].expr)));
    }
    break;

  case 21:
#line 466 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (1)].expr);
    }
    break;

  case 22:
#line 470 "parser.y"
    {
      (yyval.expr) = CommaList((yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr));
    }
    break;

  case 23:
#line 478 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (1)].expr);
    }
    break;

  case 24:
#line 482 "parser.y"
    {   
        (yyval.expr) = PreOperator((yyvsp[(2) - (2)].expr), UOP_inc);
        set_security_flag_expr((yyval.expr), (yyvsp[(2) - (2)].expr), NULL, -1);
        (yyval.expr)->size = (yyvsp[(2) - (2)].expr)->size;
    }
    break;

  case 25:
#line 488 "parser.y"
    {
        (yyval.expr) = PreOperator((yyvsp[(2) - (2)].expr), UOP_dec);
        set_security_flag_expr((yyval.expr), (yyvsp[(2) - (2)].expr), NULL, -1);
        (yyval.expr)->size = (yyvsp[(2) - (2)].expr)->size; 
    }
    break;

  case 26:
#line 494 "parser.y"
    {
        (yyval.expr) = UnaryOperator((yyvsp[(1) - (2)].type), (yyvsp[(2) - (2)].expr));
        set_security_flag_expr((yyval.expr), (yyvsp[(2) - (2)].expr), NULL, -1);
        (yyval.expr)->size = (yyvsp[(2) - (2)].expr)->size; 
    }
    break;

  case 27:
#line 503 "parser.y"
    {
        (yyval.type) = UOP_neg; 
    }
    break;

  case 28:
#line 507 "parser.y"
    {
        (yyval.type) = UOP_lnot; 
    }
    break;

  case 29:
#line 511 "parser.y"
    {
        (yyval.type) = UOP_addr;
    }
    break;

  case 30:
#line 515 "parser.y"
    {
        (yyval.type) = UOP_star;
    }
    break;

  case 31:
#line 525 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (1)].expr);
    }
    break;

  case 32:
#line 529 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    }
    break;

  case 33:
#line 533 "parser.y"
    {
      decrease_index((yyvsp[(1) - (4)].expr)); 
      (yyval.expr) = BinaryOperator(BOP_mul, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
      set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_mul);
    }
    break;

  case 34:
#line 539 "parser.y"
    {
       increase_index((yyvsp[(1) - (2)].expr));
   }
    break;

  case 35:
#line 543 "parser.y"
    {
    decrease_index((yyvsp[(1) - (4)].expr));
      (yyval.expr) = BinaryOperator(BOP_div, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
      set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_div);
    }
    break;

  case 36:
#line 549 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    }
    break;

  case 37:
#line 553 "parser.y"
    {
      decrease_index((yyvsp[(1) - (4)].expr));
      (yyval.expr) = BinaryOperator(BOP_mod, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
      set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_mod);
    }
    break;

  case 38:
#line 563 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (1)].expr);
    }
    break;

  case 39:
#line 567 "parser.y"
    {
       increase_index((yyvsp[(1) - (2)].expr));
   }
    break;

  case 40:
#line 571 "parser.y"
    {
      decrease_index((yyvsp[(1) - (4)].expr));
      (yyval.expr) = BinaryOperator(BOP_add, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
      set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_add);
  }
    break;

  case 41:
#line 577 "parser.y"
    {
      increase_index((yyvsp[(1) - (2)].expr));
  }
    break;

  case 42:
#line 581 "parser.y"
    {
      decrease_index((yyvsp[(1) - (4)].expr));
      (yyval.expr) = BinaryOperator(BOP_sub, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
      set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_sub);
    }
    break;

  case 43:
#line 591 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
    }
    break;

  case 44:
#line 595 "parser.y"
    {
        (yyval.expr) = BinaryOperator(BOP_shl, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr));
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr), LEFT_OP);

    }
    break;

  case 45:
#line 601 "parser.y"
    {
        (yyval.expr) = BinaryOperator(BOP_shr, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr));
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr), RIGHT_OP);
    }
    break;

  case 46:
#line 610 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
    }
    break;

  case 47:
#line 614 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    }
    break;

  case 48:
#line 618 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr));
        (yyval.expr) = BinaryOperator(BOP_lt, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_lt);
    }
    break;

  case 49:
#line 624 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    }
    break;

  case 50:
#line 628 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr));
        (yyval.expr) = BinaryOperator(BOP_gt, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_gt);
    }
    break;

  case 51:
#line 634 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    }
    break;

  case 52:
#line 638 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr));
        (yyval.expr) = BinaryOperator(BOP_leq, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_leq);
    }
    break;

  case 53:
#line 644 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    }
    break;

  case 54:
#line 648 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr));
        (yyval.expr) = BinaryOperator(BOP_geq, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_geq);
    }
    break;

  case 55:
#line 658 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
    }
    break;

  case 56:
#line 662 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    }
    break;

  case 57:
#line 666 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr));
        (yyval.expr) = BinaryOperator(BOP_eqeq, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_eqeq);
    }
    break;

  case 58:
#line 672 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    }
    break;

  case 59:
#line 676 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr));
        (yyval.expr) = BinaryOperator(BOP_neq, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_neq);
    }
    break;

  case 60:
#line 685 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
    }
    break;

  case 61:
#line 689 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    }
    break;

  case 62:
#line 693 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr));
        (yyval.expr) = BinaryOperator(BOP_band, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_band);
    }
    break;

  case 63:
#line 703 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
    }
    break;

  case 64:
#line 707 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    }
    break;

  case 65:
#line 711 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr));
        (yyval.expr) = BinaryOperator(BOP_xor, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_xor);
    }
    break;

  case 66:
#line 721 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
    }
    break;

  case 67:
#line 725 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    }
    break;

  case 68:
#line 729 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr));
        (yyval.expr) = BinaryOperator(BOP_bor, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_bor);
    }
    break;

  case 69:
#line 739 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
    }
    break;

  case 70:
#line 743 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    }
    break;

  case 71:
#line 747 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr));
        (yyval.expr) = BinaryOperator(BOP_land, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_land);

    }
    break;

  case 72:
#line 758 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
    }
    break;

  case 73:
#line 762 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    }
    break;

  case 74:
#line 766 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr));
        (yyval.expr) = BinaryOperator(BOP_lor, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_lor);
    }
    break;

  case 75:
#line 773 "parser.y"
    {
        (yyval.expr) = BinaryOperator(BOP_dot, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr));
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr), BOP_dot);
    }
    break;

  case 76:
#line 781 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
    }
    break;

  case 77:
#line 785 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
    }
    break;

  case 78:
#line 789 "parser.y"
    {
        (yyval.expr) = ConditionalExpr((yyvsp[(1) - (5)].expr), (yyvsp[(3) - (5)].expr), (yyvsp[(5) - (5)].expr));
    }
    break;

  case 79:
#line 799 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (1)].expr);
    }
    break;

  case 80:
#line 803 "parser.y"
    {
      //do security check here
      //security_check_for_assignment($1, $3);
      (yyval.expr) = Assignment((yyvsp[(1) - (3)].expr), (yyvsp[(2) - (3)].type), (yyvsp[(3) - (3)].expr));
      set_security_flag_expr((yyval.expr), (yyvsp[(1) - (3)].expr), NULL, -1);
    }
    break;

  case 81:
#line 814 "parser.y"
    {
      (yyval.type) = ASS_eq;  /* Need fix here! */
    }
    break;

  case 82:
#line 818 "parser.y"
    {
      (yyval.type) = ASS_mul;
    }
    break;

  case 83:
#line 822 "parser.y"
    {
      (yyval.type) = ASS_div;
    }
    break;

  case 84:
#line 826 "parser.y"
    {
      (yyval.type) = ASS_mod;
    }
    break;

  case 85:
#line 830 "parser.y"
    {
      (yyval.type) = ASS_add;
    }
    break;

  case 86:
#line 834 "parser.y"
    {
      (yyval.type) = ASS_sub;
    }
    break;

  case 87:
#line 843 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (1)].expr);
      set_security_flag_expr((yyval.expr), (yyvsp[(1) - (1)].expr), NULL, -1);
    }
    break;

  case 88:
#line 848 "parser.y"
    {
      (yyval.expr) = CommaList((yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr));
    }
    break;

  case 89:
#line 856 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (1)].expr);
      set_security_flag_expr((yyval.expr), (yyvsp[(1) - (1)].expr), NULL, -1);
    }
    break;

  case 90:
#line 871 "parser.y"
    {
      if (checkDecls) add_declaration_links((yyvsp[(1) - (3)].spec), (yyvsp[(2) - (3)].decl));
      // assign default to var size
      security_check_for_declaration((yyvsp[(1) - (3)].spec), (yyvsp[(2) - (3)].decl)); 
      (yyval.stmt) = Declaration((yyvsp[(1) - (3)].spec), (yyvsp[(2) - (3)].decl));
      isTypedef = 0;
    }
    break;

  case 91:
#line 879 "parser.y"
    {
        (yyval.stmt) = OmpStmt(OmpConstruct(DCTHREADPRIVATE, (yyvsp[(1) - (1)].odir), NULL));
    }
    break;

  case 92:
#line 888 "parser.y"
    {
      (yyval.spec) = (yyvsp[(1) - (1)].spec);
    }
    break;

  case 93:
#line 892 "parser.y"
    {
     (yyval.spec) = Speclist_right((yyvsp[(1) - (2)].spec), (yyvsp[(2) - (2)].spec));
    }
    break;

  case 94:
#line 900 "parser.y"
    {
      (yyval.decl) = (yyvsp[(1) - (1)].decl);
    }
    break;

  case 95:
#line 904 "parser.y"
    {
      (yyval.decl) = DeclList((yyvsp[(1) - (3)].decl), (yyvsp[(3) - (3)].decl));
    }
    break;

  case 96:
#line 918 "parser.y"
    {
      astdecl s = decl_getidentifier((yyvsp[(1) - (1)].decl));
      int     declkind = decl_getkind((yyvsp[(1) - (1)].decl));
      stentry e;
      
      if (!isTypedef && declkind == DFUNC && strcmp(s->u.id->name, "main") == 0)
        s->u.id = Symbol(MAIN_NEWNAME);       /* Catch main()'s declaration */
      if (checkDecls) 
      {
        e = symtab_put(stab, s->u.id, (isTypedef) ? TYPENAME :
                                       (declkind == DFUNC) ? FUNCNAME : IDNAME);
        e->isarray = (declkind == DARRAY);
      }
      (yyval.decl) = (yyvsp[(1) - (1)].decl);
    }
    break;

  case 97:
#line 934 "parser.y"
    {
      astdecl s = decl_getidentifier((yyvsp[(1) - (2)].decl));
      int     declkind = decl_getkind((yyvsp[(1) - (2)].decl));
      stentry e;
      
      if (!isTypedef && declkind == DFUNC && strcmp(s->u.id->name, "main") == 0)
        s->u.id = Symbol(MAIN_NEWNAME);         /* Catch main()'s declaration */
      if (checkDecls) 
      {
        e = symtab_put(stab, s->u.id, (isTypedef) ? TYPENAME :
                                       (declkind == DFUNC) ? FUNCNAME : IDNAME);
        e->isarray = (declkind == DARRAY);
      }
    }
    break;

  case 98:
#line 949 "parser.y"
    {
      (yyval.decl) = InitDecl((yyvsp[(1) - (4)].decl), (yyvsp[(4) - (4)].expr));
    }
    break;

  case 99:
#line 959 "parser.y"
    {
      (yyval.spec) = Declspec(SPEC_int, 0);
    }
    break;

  case 100:
#line 963 "parser.y"
    {
      (yyval.spec) = Declspec(SPEC_void, 0);
    }
    break;

  case 101:
#line 967 "parser.y"
    {
      (yyval.spec) = Declspec(SPEC_int, atoi((yyvsp[(3) - (4)].name)));
    }
    break;

  case 102:
#line 974 "parser.y"
    {
      (yyval.spec) = Declspec(SPEC_private, 0);
    }
    break;

  case 103:
#line 978 "parser.y"
    {
      (yyval.spec) = Declspec(SPEC_public, 0);
    }
    break;

  case 104:
#line 987 "parser.y"
    {
        (yyval.spec) = Pointer();
    }
    break;

  case 105:
#line 991 "parser.y"
    {
        (yyval.spec) = Speclist_right(Pointer(), (yyvsp[(2) - (2)].spec));
    }
    break;

  case 106:
#line 995 "parser.y"
    {
        (yyval.spec) = Speclist_right(Pointer(), (yyvsp[(2) - (2)].spec));
    }
    break;

  case 107:
#line 999 "parser.y"
    {
        (yyval.spec) = Speclist_right( Pointer(), Speclist_left((yyvsp[(2) - (3)].spec), (yyvsp[(3) - (3)].spec)) );
    }
    break;

  case 108:
#line 1006 "parser.y"
    {
        (yyval.decl) = (yyvsp[(1) - (1)].decl);
    }
    break;

  case 109:
#line 1017 "parser.y"
    {
        (yyval.decl) = (yyvsp[(1) - (1)].decl);
    }
    break;

  case 110:
#line 1021 "parser.y"
    {
        (yyval.decl) = ParamList((yyvsp[(1) - (3)].decl), (yyvsp[(3) - (3)].decl));
    }
    break;

  case 111:
#line 1028 "parser.y"
    {
        (yyval.decl) = ParamDecl((yyvsp[(1) - (2)].spec), (yyvsp[(2) - (2)].decl));
    }
    break;

  case 112:
#line 1032 "parser.y"
    {
        (yyval.decl) = ParamDecl((yyvsp[(1) - (1)].spec), NULL);
    }
    break;

  case 113:
#line 1036 "parser.y"
    {
        (yyval.decl) = ParamDecl((yyvsp[(1) - (2)].spec), (yyvsp[(2) - (2)].decl));
    }
    break;

  case 114:
#line 1044 "parser.y"
    {
        (yyval.decl) = AbstractDeclarator((yyvsp[(1) - (1)].spec), NULL);
    }
    break;

  case 115:
#line 1048 "parser.y"
    {
        (yyval.decl) = AbstractDeclarator(NULL, (yyvsp[(1) - (1)].decl));
    }
    break;

  case 116:
#line 1052 "parser.y"
    {
        (yyval.decl) = AbstractDeclarator((yyvsp[(1) - (2)].spec), (yyvsp[(2) - (2)].decl));
    }
    break;

  case 117:
#line 1059 "parser.y"
    {
        (yyval.decl) = ParenDecl((yyvsp[(2) - (3)].decl));
    }
    break;

  case 118:
#line 1063 "parser.y"
    {
        (yyval.decl) = ArrayDecl(NULL, NULL, NULL);
    }
    break;

  case 119:
#line 1067 "parser.y"
    {
        (yyval.decl) = ArrayDecl((yyvsp[(1) - (3)].decl), NULL, NULL);
    }
    break;

  case 120:
#line 1071 "parser.y"
    {
        (yyval.decl) = ArrayDecl(NULL, NULL, (yyvsp[(2) - (3)].expr));
    }
    break;

  case 121:
#line 1075 "parser.y"
    {
        (yyval.decl) = ArrayDecl((yyvsp[(1) - (4)].decl), NULL, (yyvsp[(3) - (4)].expr));
    }
    break;

  case 122:
#line 1079 "parser.y"
    {
        (yyval.decl) = ArrayDecl(NULL, Declspec(SPEC_star, 0), NULL);
    }
    break;

  case 123:
#line 1083 "parser.y"
    {
        (yyval.decl) = ArrayDecl((yyvsp[(1) - (4)].decl), Declspec(SPEC_star, 0), NULL);
    }
    break;

  case 124:
#line 1087 "parser.y"
    {
        (yyval.decl) = FuncDecl(NULL, NULL);
    }
    break;

  case 125:
#line 1091 "parser.y"
    {
        (yyval.decl) = FuncDecl((yyvsp[(1) - (3)].decl), NULL);
    }
    break;

  case 126:
#line 1095 "parser.y"
    {
        (yyval.decl) = FuncDecl(NULL, (yyvsp[(2) - (3)].decl));
    }
    break;

  case 127:
#line 1099 "parser.y"
    {
        (yyval.decl) = FuncDecl((yyvsp[(1) - (4)].decl), (yyvsp[(3) - (4)].decl));
    }
    break;

  case 128:
#line 1107 "parser.y"
    {
      (yyval.spec) = (yyvsp[(1) - (1)].spec);
    }
    break;

  case 129:
#line 1111 "parser.y"
    {
      (yyval.spec) = Speclist_right((yyvsp[(1) - (2)].spec), (yyvsp[(2) - (2)].spec));
    }
    break;

  case 130:
#line 1119 "parser.y"
    {
      (yyval.decl) = Declarator(NULL, (yyvsp[(1) - (1)].decl));
    }
    break;

  case 131:
#line 1123 "parser.y"
    {
        (yyval.decl) = Declarator((yyvsp[(1) - (2)].spec), (yyvsp[(2) - (2)].decl));
    }
    break;

  case 132:
#line 1132 "parser.y"
    {
      (yyval.decl) = IdentifierDecl( Symbol((yyvsp[(1) - (1)].name)) );
    }
    break;

  case 133:
#line 1136 "parser.y"
    {
      /* Try to simplify a bit: (ident) -> ident */
      if ((yyvsp[(2) - (3)].decl)->spec == NULL && (yyvsp[(2) - (3)].decl)->decl->type == DIDENT)
        (yyval.decl) = (yyvsp[(2) - (3)].decl)->decl;
      else
        (yyval.decl) = ParenDecl((yyvsp[(2) - (3)].decl));
    }
    break;

  case 134:
#line 1144 "parser.y"
    {
      (yyval.decl) = ArrayDecl((yyvsp[(1) - (3)].decl), NULL, NULL);
    }
    break;

  case 135:
#line 1148 "parser.y"
    {
      (yyval.decl) = ArrayDecl((yyvsp[(1) - (4)].decl), NULL, (yyvsp[(3) - (4)].expr));
    }
    break;

  case 136:
#line 1152 "parser.y"
    {
      (yyval.decl) = FuncDecl((yyvsp[(1) - (3)].decl), NULL);
    }
    break;

  case 137:
#line 1156 "parser.y"
    {
      (yyval.decl) = FuncDecl((yyvsp[(1) - (4)].decl), (yyvsp[(3) - (4)].decl));
    }
    break;

  case 138:
#line 1160 "parser.y"
    {
      (yyval.decl) = FuncDecl((yyvsp[(1) - (4)].decl), (yyvsp[(3) - (4)].decl));
    }
    break;

  case 139:
#line 1170 "parser.y"
    {
      (yyval.decl) = IdentifierDecl( Symbol((yyvsp[(1) - (1)].name)) );
    }
    break;

  case 140:
#line 1174 "parser.y"
    {
      (yyval.decl) = IdList((yyvsp[(1) - (3)].decl), IdentifierDecl( Symbol((yyvsp[(3) - (3)].name)) ));
    }
    break;

  case 141:
#line 1182 "parser.y"
    {
      (yyval.decl) = Casttypename((yyvsp[(1) - (1)].spec), NULL);
    }
    break;

  case 142:
#line 1191 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (1)].expr);
    }
    break;

  case 143:
#line 1195 "parser.y"
    {
      (yyval.expr) = BracedInitializer((yyvsp[(2) - (3)].expr));
    }
    break;

  case 144:
#line 1199 "parser.y"
    {
      (yyval.expr) = BracedInitializer((yyvsp[(2) - (4)].expr));
    }
    break;

  case 145:
#line 1207 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (1)].expr);
    }
    break;

  case 146:
#line 1211 "parser.y"
    {
      (yyval.expr) = Designated((yyvsp[(1) - (2)].expr), (yyvsp[(2) - (2)].expr));
    }
    break;

  case 147:
#line 1215 "parser.y"
    {
      (yyval.expr) = CommaList((yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr));
    }
    break;

  case 148:
#line 1219 "parser.y"
    {
      (yyval.expr) = CommaList((yyvsp[(1) - (4)].expr), Designated((yyvsp[(3) - (4)].expr), (yyvsp[(4) - (4)].expr)));
    }
    break;

  case 149:
#line 1227 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (2)].expr); 
    }
    break;

  case 150:
#line 1235 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (1)].expr);
    }
    break;

  case 151:
#line 1239 "parser.y"
    {
      (yyval.expr) = SpaceList((yyvsp[(1) - (2)].expr), (yyvsp[(2) - (2)].expr));
    }
    break;

  case 152:
#line 1247 "parser.y"
    {
        security_check_for_condition((yyvsp[(2) - (3)].expr));
        (yyval.expr) = IdxDesignator((yyvsp[(2) - (3)].expr));
    }
    break;

  case 153:
#line 1262 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
      (yyval.stmt)->flag = (yyvsp[(1) - (1)].stmt)->flag;
    }
    break;

  case 154:
#line 1267 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
      (yyval.stmt)->flag = (yyvsp[(1) - (1)].stmt)->flag;
    }
    break;

  case 155:
#line 1272 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
      (yyval.stmt)->flag = (yyvsp[(1) - (1)].stmt)->flag; 
    }
    break;

  case 156:
#line 1277 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
      (yyval.stmt)->flag = (yyvsp[(1) - (1)].stmt)->flag;
    }
    break;

  case 157:
#line 1282 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
      (yyval.stmt)->flag = (yyvsp[(1) - (1)].stmt)->flag; 
    }
    break;

  case 158:
#line 1287 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
      (yyval.stmt)->flag = (yyvsp[(1) - (1)].stmt)->flag; 
    }
    break;

  case 159:
#line 1292 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
      (yyval.stmt)->flag = (yyvsp[(1) - (1)].stmt)->flag; 
    }
    break;

  case 160:
#line 1297 "parser.y"
    {
     (yyval.stmt) = OmpStmt((yyvsp[(1) - (1)].ocon));
     (yyval.stmt)->l = (yyvsp[(1) - (1)].ocon)->l;
    }
    break;

  case 161:
#line 1302 "parser.y"
    {
    (yyval.stmt) = OmpixStmt((yyvsp[(1) - (1)].xcon));
    (yyval.stmt)->l = (yyvsp[(1) - (1)].xcon)->l;
  }
    break;

  case 162:
#line 1312 "parser.y"
    {
      (yyval.stmt) = Compound(NULL);
    }
    break;

  case 163:
#line 1315 "parser.y"
    { (yyval.type) = sc_original_line()-1; scope_start(stab);}
    break;

  case 164:
#line 1317 "parser.y"
    {
      (yyval.stmt) = Compound((yyvsp[(3) - (4)].stmt));
      scope_end(stab);
      (yyval.stmt)->l = (yyvsp[(2) - (4)].type);     /* Remember 1st line */
      (yyval.stmt)->flag = (yyvsp[(3) - (4)].stmt)->flag; 
    }
    break;

  case 165:
#line 1328 "parser.y"
    {
        (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
        (yyval.stmt)->flag = (yyvsp[(1) - (1)].stmt)->flag;
    }
    break;

  case 166:
#line 1333 "parser.y"
    {
      (yyval.stmt) = BlockList((yyvsp[(1) - (2)].stmt), (yyvsp[(2) - (2)].stmt));
      (yyval.stmt)->l = (yyvsp[(1) - (2)].stmt)->l;
      set_security_flag_stmt((yyval.stmt), (yyvsp[(1) - (2)].stmt), (yyvsp[(2) - (2)].stmt));
    }
    break;

  case 167:
#line 1343 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
    }
    break;

  case 168:
#line 1347 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
      (yyval.stmt)->flag = (yyvsp[(1) - (1)].stmt)->flag;
    }
    break;

  case 169:
#line 1352 "parser.y"
    {
        (yyval.stmt) = OmpStmt((yyvsp[(1) - (1)].ocon));
        (yyval.stmt)->l = (yyvsp[(1) - (1)].ocon)->l;
    }
    break;

  case 170:
#line 1357 "parser.y"
    {
        (yyval.stmt) = OmpixStmt((yyvsp[(1) - (1)].xcon));
        (yyval.stmt)->l = (yyvsp[(1) - (1)].xcon)->l;
    }
    break;

  case 171:
#line 1366 "parser.y"
    {
      (yyval.stmt) = Expression(NULL);
    }
    break;

  case 172:
#line 1370 "parser.y"
    {
      (yyval.stmt) = Expression((yyvsp[(1) - (2)].expr));
      (yyval.stmt)->l = (yyvsp[(1) - (2)].expr)->l;
      (yyval.stmt)->flag = (yyvsp[(1) - (2)].expr)->flag;
    }
    break;

  case 173:
#line 1378 "parser.y"
    {
        tmp_index++;
        if(tmp_index > cond_index)
            cond_index = tmp_index;
    }
    break;

  case 174:
#line 1385 "parser.y"
    {
        (yyval.stmt) = If((yyvsp[(3) - (6)].expr), (yyvsp[(6) - (6)].stmt), NULL);            
        if((yyvsp[(3) - (6)].expr)->flag == PRI && (yyvsp[(6) - (6)].stmt)->flag == PUB)
           parse_error(1, "public assignment is not allowd within the private condition\n");
        if((yyvsp[(3) - (6)].expr)->flag == PRI)
            contain_priv_if_flag = 1; 
        (yyval.stmt)->flag = (yyvsp[(6) - (6)].stmt)->flag;
            tmp_index--;

    }
    break;

  case 175:
#line 1396 "parser.y"
    {
        (yyval.stmt) = If((yyvsp[(3) - (8)].expr), (yyvsp[(6) - (8)].stmt), (yyvsp[(8) - (8)].stmt));
        if(((yyvsp[(3) - (8)].expr)->flag == PRI && (yyvsp[(6) - (8)].stmt)->flag == PUB) || ((yyvsp[(3) - (8)].expr)->flag == PRI && (yyvsp[(8) - (8)].stmt)->flag == PUB))
           parse_error(1, "public assignment is not allowd within the private condition\n");
        if((yyvsp[(3) - (8)].expr)->flag == PRI)
            contain_priv_if_flag = 1; 
        if((yyvsp[(6) - (8)].stmt)->flag == PUB || (yyvsp[(8) - (8)].stmt)->flag == PUB)
            (yyval.stmt)->flag = PUB; 
        else
            (yyval.stmt)->flag = PRI;
        tmp_index--;
    }
    break;

  case 176:
#line 1414 "parser.y"
    {
        security_check_for_condition((yyvsp[(3) - (5)].expr)); 
        (yyval.stmt) = While((yyvsp[(3) - (5)].expr), (yyvsp[(5) - (5)].stmt));
        (yyval.stmt)->flag = (yyvsp[(5) - (5)].stmt)->flag;
    }
    break;

  case 177:
#line 1420 "parser.y"
    {
        security_check_for_condition((yyvsp[(5) - (7)].expr));         
        (yyval.stmt) = Do((yyvsp[(2) - (7)].stmt), (yyvsp[(5) - (7)].expr));
        (yyval.stmt)->flag = (yyvsp[(2) - (7)].stmt)->flag;
    }
    break;

  case 178:
#line 1426 "parser.y"
    {
        (yyval.stmt)->flag = (yyvsp[(1) - (1)].stmt)->flag;
    }
    break;

  case 179:
#line 1433 "parser.y"
    {
        (yyval.stmt) = For(NULL, NULL, NULL, (yyvsp[(6) - (6)].stmt));
        (yyval.stmt)->flag = (yyvsp[(6) - (6)].stmt)->flag;
    }
    break;

  case 180:
#line 1438 "parser.y"
    {
        (yyval.stmt) = For(Expression((yyvsp[(3) - (7)].expr)), NULL, NULL, (yyvsp[(7) - (7)].stmt));
        (yyval.stmt)->flag = (yyvsp[(7) - (7)].stmt)->flag;
    }
    break;

  case 181:
#line 1443 "parser.y"
    {
        security_check_for_condition((yyvsp[(4) - (7)].expr));         
        (yyval.stmt) = For(NULL, (yyvsp[(4) - (7)].expr), NULL, (yyvsp[(7) - (7)].stmt));
        (yyval.stmt)->flag = (yyvsp[(7) - (7)].stmt)->flag;
    }
    break;

  case 182:
#line 1449 "parser.y"
    {
        (yyval.stmt) = For(NULL, NULL, (yyvsp[(5) - (7)].expr), (yyvsp[(7) - (7)].stmt));
        (yyval.stmt)->flag = (yyvsp[(7) - (7)].stmt)->flag;

    }
    break;

  case 183:
#line 1455 "parser.y"
    {
        security_check_for_condition((yyvsp[(5) - (8)].expr));                 
        (yyval.stmt) = For(Expression((yyvsp[(3) - (8)].expr)), (yyvsp[(5) - (8)].expr), NULL, (yyvsp[(8) - (8)].stmt));
        (yyval.stmt)->flag = (yyvsp[(8) - (8)].stmt)->flag;
    }
    break;

  case 184:
#line 1461 "parser.y"
    {
        (yyval.stmt) = For(Expression((yyvsp[(3) - (8)].expr)), NULL, (yyvsp[(6) - (8)].expr), (yyvsp[(8) - (8)].stmt));
        (yyval.stmt)->flag = (yyvsp[(8) - (8)].stmt)->flag;
    }
    break;

  case 185:
#line 1466 "parser.y"
    {
        security_check_for_condition((yyvsp[(4) - (8)].expr));                 
        (yyval.stmt) = For(NULL, (yyvsp[(4) - (8)].expr), (yyvsp[(6) - (8)].expr), (yyvsp[(8) - (8)].stmt));
        (yyval.stmt)->flag = (yyvsp[(8) - (8)].stmt)->flag;
    }
    break;

  case 186:
#line 1472 "parser.y"
    {	
        security_check_for_condition((yyvsp[(5) - (9)].expr));         
        (yyval.stmt) = For(Expression((yyvsp[(3) - (9)].expr)), (yyvsp[(5) - (9)].expr), (yyvsp[(7) - (9)].expr), (yyvsp[(9) - (9)].stmt));
        (yyval.stmt)->flag = (yyvsp[(9) - (9)].stmt)->flag;

    }
    break;

  case 187:
#line 1479 "parser.y"
    {
        (yyval.stmt) = For((yyvsp[(3) - (6)].stmt), NULL, NULL, (yyvsp[(6) - (6)].stmt));
        (yyval.stmt)->flag = (yyvsp[(6) - (6)].stmt)->flag;

    }
    break;

  case 188:
#line 1485 "parser.y"
    {
        (yyval.stmt) = For((yyvsp[(3) - (7)].stmt), (yyvsp[(4) - (7)].expr), NULL, (yyvsp[(7) - (7)].stmt));
        security_check_for_condition((yyvsp[(4) - (7)].expr));
        (yyval.stmt)->flag = (yyvsp[(7) - (7)].stmt)->flag;

    }
    break;

  case 189:
#line 1492 "parser.y"
    {
        (yyval.stmt) = For((yyvsp[(3) - (7)].stmt), NULL, (yyvsp[(5) - (7)].expr), (yyvsp[(7) - (7)].stmt));
        (yyval.stmt)->flag = (yyvsp[(7) - (7)].stmt)->flag;

    }
    break;

  case 190:
#line 1498 "parser.y"
    {
        security_check_for_condition((yyvsp[(4) - (8)].expr));                 
        (yyval.stmt) = For((yyvsp[(3) - (8)].stmt), (yyvsp[(4) - (8)].expr), (yyvsp[(6) - (8)].expr), (yyvsp[(8) - (8)].stmt));
        (yyval.stmt)->flag = (yyvsp[(8) - (8)].stmt)->flag;

    }
    break;

  case 191:
#line 1508 "parser.y"
    {
        (yyval.stmt) = Continue();
    }
    break;

  case 192:
#line 1512 "parser.y"
    {
        (yyval.stmt) = Break();
    }
    break;

  case 193:
#line 1516 "parser.y"
    {
        (yyval.stmt) = Return(NULL);
    }
    break;

  case 194:
#line 1520 "parser.y"
    {
        if(((yyvsp[(2) - (3)].expr)->flag == PRI && func_return_flag == 0)
           || ((yyvsp[(2) - (3)].expr)->flag == PUB && func_return_flag == 1))
            parse_error(-1, "return type does not match.\n");
        
        (yyval.stmt) = Return((yyvsp[(2) - (3)].expr));
        
    }
    break;

  case 195:
#line 1532 "parser.y"
    {
        stentry e = get_entry_from_expr((yyvsp[(3) - (7)].expr));
        if(set_security_flag_spec(e->spec) != PRI)
            parse_error(-1, "the first parameter should be private.\n");
        (yyval.stmt) = Smc(SINPUT, e->spec, (yyvsp[(3) - (7)].expr), Constant(strdup((yyvsp[(5) - (7)].name))), NULL);
    }
    break;

  case 196:
#line 1540 "parser.y"
    {
        stentry e = get_entry_from_expr((yyvsp[(3) - (9)].expr));
        if(set_security_flag_spec(e->spec) != PRI)
            parse_error(-1, "the first parameter should be private.\n");
        (yyval.stmt) = Smc(SINPUT, e->spec, (yyvsp[(3) - (9)].expr), Constant(strdup((yyvsp[(5) - (9)].name))), (yyvsp[(7) - (9)].expr));
    }
    break;

  case 197:
#line 1548 "parser.y"
    {
        stentry e = get_entry_from_expr((yyvsp[(3) - (7)].expr));
        if(set_security_flag_spec(e->spec) != PRI)
            parse_error(-1, "the first parameter should be private.\n");
        (yyval.stmt) = Smc(SOUTPUT, e->spec, (yyvsp[(3) - (7)].expr), Constant(strdup((yyvsp[(5) - (7)].name))), NULL);
    }
    break;

  case 198:
#line 1555 "parser.y"
    {
        stentry e = get_entry_from_expr((yyvsp[(3) - (9)].expr));
        if(set_security_flag_spec(e->spec) != PRI)
            parse_error(-1, "the first parameter should be private.\n");
        (yyval.stmt) = Smc(SOUTPUT, e->spec, (yyvsp[(3) - (9)].expr), Constant(strdup((yyvsp[(5) - (9)].name))), (yyvsp[(7) - (9)].expr));
    }
    break;

  case 199:
#line 1563 "parser.y"
    {
        stentry e1 = get_entry_from_expr((yyvsp[(3) - (7)].expr));
        stentry e2 = get_entry_from_expr((yyvsp[(5) - (7)].expr));
        if(set_security_flag_spec(e1->spec) != PUB || set_security_flag_spec(e2->spec) != PRI)
            parse_error(-1, "the first and second parameter should be type of public and private each\n");
        (yyval.stmt) = Smc(SOPEN, NULL, (yyvsp[(3) - (7)].expr), (yyvsp[(5) - (7)].expr), NULL);
    }
    break;

  case 200:
#line 1573 "parser.y"
    {
        stentry e1 = get_entry_from_expr((yyvsp[(3) - (9)].expr));
        stentry e2 = get_entry_from_expr((yyvsp[(5) - (9)].expr));
        if(set_security_flag_spec(e1->spec) != PUB || set_security_flag_spec(e2->spec) != PRI)
            parse_error(-1, "the first and second parameter should be type of public and private each\n");
        (yyval.stmt) = Smc(SOPEN, NULL, (yyvsp[(3) - (9)].expr), (yyvsp[(5) - (9)].expr), (yyvsp[(7) - (9)].expr));
    }
    break;

  case 201:
#line 1584 "parser.y"
    {
        security_check_for_condition((yyvsp[(5) - (11)].expr));
        //$$->flag = $10->flag;
        (yyval.stmt) = Batch(Expression((yyvsp[(3) - (11)].expr)), (yyvsp[(5) - (11)].expr), (yyvsp[(7) - (11)].expr), (yyvsp[(10) - (11)].stmt));
    }
    break;

  case 202:
#line 1598 "parser.y"
    {
      (yyval.stmt) = pastree = (yyvsp[(1) - (1)].stmt);
    }
    break;

  case 203:
#line 1602 "parser.y"
    {
      (yyval.stmt) = pastree = BlockList((yyvsp[(1) - (2)].stmt), (yyvsp[(2) - (2)].stmt));
    }
    break;

  case 204:
#line 1610 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
    }
    break;

  case 205:
#line 1614 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
    }
    break;

  case 206:
#line 1621 "parser.y"
    {
        (yyval.stmt) = OmpixStmt((yyvsp[(1) - (1)].xcon));
    }
    break;

  case 207:
#line 1631 "parser.y"
    { (yyval.stmt) = (yyvsp[(1) - (1)].stmt); }
    break;

  case 208:
#line 1636 "parser.y"
    {
      func_return_flag = 0;
      tmp_index = 1;
      num_index = 0;
      cond_index = 0;
      contain_priv_if_flag = 0;
        
      if (isTypedef || (yyvsp[(2) - (2)].decl)->decl->type != DFUNC)
        parse_error(1, "function definition cannot be parsed.\n");
      if (symtab_get(stab, decl_getidentifier_symbol((yyvsp[(2) - (2)].decl)), FUNCNAME) == NULL)
        symtab_put_funcname(stab, decl_getidentifier_symbol((yyvsp[(2) - (2)].decl)), FUNCNAME, (yyvsp[(1) - (2)].spec), (yyvsp[(2) - (2)].decl));
        symbol e = decl_getidentifier_symbol((yyvsp[(2) - (2)].decl));

     symbol s = decl_getidentifier_symbol((yyvsp[(2) - (2)].decl));
     scope_start(stab);
     ast_declare_function_params((yyvsp[(2) - (2)].decl));
     if(set_security_flag_spec((yyvsp[(1) - (2)].spec)) == PRI)
         func_return_flag = 1;
     }
    break;

  case 209:
#line 1656 "parser.y"
    {
      scope_end(stab);
      check_for_main_and_declare((yyvsp[(1) - (4)].spec), (yyvsp[(2) - (4)].decl));
      (yyval.stmt) = FuncDef((yyvsp[(1) - (4)].spec), (yyvsp[(2) - (4)].decl), NULL, (yyvsp[(4) - (4)].stmt));
      if(contain_priv_if_flag){
          if(cond_index > num_index)
                (yyval.stmt)->num_tmp = cond_index;
          else
                (yyval.stmt)->num_tmp = num_index;
       }
      else
           (yyval.stmt)->num_tmp = var_index;
      (yyval.stmt)->contain_priv_if_flag = contain_priv_if_flag;
    }
    break;

  case 210:
#line 1671 "parser.y"
    {
      tmp_index = 1;
      num_index = 0;
      cond_index = 0;
      contain_priv_if_flag = 0;
      if (isTypedef || (yyvsp[(1) - (1)].decl)->decl->type != DFUNC)
        parse_error(1, "function definition cannot be parsed.\n");
      if (symtab_get(stab, decl_getidentifier_symbol((yyvsp[(1) - (1)].decl)), FUNCNAME) == NULL)
        symtab_put_funcname(stab, decl_getidentifier_symbol((yyvsp[(1) - (1)].decl)), FUNCNAME, NULL, (yyvsp[(1) - (1)].decl));
        symbol e = decl_getidentifier_symbol((yyvsp[(1) - (1)].decl));

      scope_start(stab);
      ast_declare_function_params((yyvsp[(1) - (1)].decl));
    }
    break;

  case 211:
#line 1686 "parser.y"
    {
      astspec s = Declspec(SPEC_int, 0);  /* return type defaults to "int" */
      scope_end(stab);
      check_for_main_and_declare(s, (yyvsp[(1) - (3)].decl));
      (yyval.stmt) = FuncDef(s, (yyvsp[(1) - (3)].decl), NULL, (yyvsp[(3) - (3)].stmt));
        
      if(contain_priv_if_flag){
          if(cond_index > num_index)
              (yyval.stmt)->num_tmp = cond_index;
          else
              (yyval.stmt)->num_tmp = num_index;
      }
      else
          (yyval.stmt)->num_tmp = var_index;
      (yyval.stmt)->contain_priv_if_flag = contain_priv_if_flag;
    }
    break;

  case 212:
#line 1710 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    }
    break;

  case 213:
#line 1714 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    }
    break;

  case 214:
#line 1718 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    }
    break;

  case 215:
#line 1722 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    }
    break;

  case 216:
#line 1726 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    }
    break;

  case 217:
#line 1730 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    }
    break;

  case 218:
#line 1734 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    }
    break;

  case 219:
#line 1738 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    }
    break;

  case 220:
#line 1742 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    }
    break;

  case 221:
#line 1746 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    }
    break;

  case 222:
#line 1751 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    }
    break;

  case 223:
#line 1767 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCBARRIER, (yyvsp[(1) - (1)].odir), NULL);
    }
    break;

  case 224:
#line 1771 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCFLUSH, (yyvsp[(1) - (1)].odir), NULL);
    }
    break;

  case 225:
#line 1776 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCTASKWAIT, (yyvsp[(1) - (1)].odir), NULL);
    }
    break;

  case 226:
#line 1781 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCTASKYIELD, (yyvsp[(1) - (1)].odir), NULL);
    }
    break;

  case 227:
#line 1788 "parser.y"
    {
        (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
    }
    break;

  case 228:
#line 1795 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCPARALLEL, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
        (yyval.ocon)->l = (yyvsp[(1) - (2)].odir)->l;
    }
    break;

  case 229:
#line 1803 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCPARALLEL, (yyvsp[(3) - (4)].ocla));
    }
    break;

  case 230:
#line 1810 "parser.y"
    {
        (yyval.ocla) = NULL;
    }
    break;

  case 231:
#line 1814 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (2)].ocla), (yyvsp[(2) - (2)].ocla));
    }
    break;

  case 232:
#line 1818 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (3)].ocla), (yyvsp[(3) - (3)].ocla));
    }
    break;

  case 233:
#line 1825 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    }
    break;

  case 234:
#line 1829 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    }
    break;

  case 235:
#line 1835 "parser.y"
    { sc_pause_openmp(); }
    break;

  case 236:
#line 1836 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = IfClause((yyvsp[(4) - (5)].expr));
    }
    break;

  case 237:
#line 1840 "parser.y"
    { sc_pause_openmp(); }
    break;

  case 238:
#line 1841 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = NumthreadsClause((yyvsp[(4) - (5)].expr));
    }
    break;

  case 239:
#line 1849 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCFOR, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
    }
    break;

  case 240:
#line 1856 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCFOR, (yyvsp[(3) - (4)].ocla));
    }
    break;

  case 241:
#line 1863 "parser.y"
    {
        (yyval.ocla) = NULL;
    }
    break;

  case 242:
#line 1867 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (2)].ocla), (yyvsp[(2) - (2)].ocla));
    }
    break;

  case 243:
#line 1871 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (3)].ocla), (yyvsp[(3) - (3)].ocla));
    }
    break;

  case 244:
#line 1878 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    }
    break;

  case 245:
#line 1882 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    }
    break;

  case 246:
#line 1886 "parser.y"
    {
        (yyval.ocla) = PlainClause(OCNOWAIT);
    }
    break;

  case 247:
#line 1893 "parser.y"
    {
        (yyval.ocla) = PlainClause(OCORDERED);
    }
    break;

  case 248:
#line 1897 "parser.y"
    {
        (yyval.ocla) = ScheduleClause((yyvsp[(3) - (4)].type), NULL);
    }
    break;

  case 249:
#line 1900 "parser.y"
    { sc_pause_openmp(); }
    break;

  case 250:
#line 1901 "parser.y"
    {
        sc_start_openmp();
        if ((yyvsp[(3) - (7)].type) == OC_runtime)
        parse_error(1, "\"runtime\" schedules may not have a chunksize.\n");
        (yyval.ocla) = ScheduleClause((yyvsp[(3) - (7)].type), (yyvsp[(6) - (7)].expr));
    }
    break;

  case 251:
#line 1908 "parser.y"
    {  /* non-OpenMP schedule */
        tempsave = checkDecls;
        checkDecls = 0;   /* Because the index of the loop is usualy involved */
        sc_pause_openmp();
    }
    break;

  case 252:
#line 1914 "parser.y"
    {
        sc_start_openmp();
        checkDecls = tempsave;
        (yyval.ocla) = ScheduleClause(OC_affinity, (yyvsp[(6) - (7)].expr));
    }
    break;

  case 253:
#line 1920 "parser.y"
    {
        int n = 0, er = 0;
        if (xar_expr_is_constant((yyvsp[(3) - (4)].expr)))
        {
            n = xar_calc_int_expr((yyvsp[(3) - (4)].expr), &er);
            if (er) n = 0;
        }
        if (n <= 0)
        parse_error(1, "invalid number in collapse() clause.\n");
        (yyval.ocla) = CollapseClause(n);
    }
    break;

  case 254:
#line 1935 "parser.y"
    {
        (yyval.type) = OC_static;
    }
    break;

  case 255:
#line 1939 "parser.y"
    {
        (yyval.type) = OC_dynamic;
    }
    break;

  case 256:
#line 1943 "parser.y"
    {
        (yyval.type) = OC_guided;
    }
    break;

  case 257:
#line 1947 "parser.y"
    {
        (yyval.type) = OC_runtime;
    }
    break;

  case 258:
#line 1951 "parser.y"
    {
        (yyval.type) = OC_auto;
    }
    break;

  case 259:
#line 1954 "parser.y"
    { parse_error(1, "invalid openmp schedule type.\n"); }
    break;

  case 260:
#line 1960 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCSECTIONS, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
    }
    break;

  case 261:
#line 1967 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCSECTIONS, (yyvsp[(3) - (4)].ocla));
    }
    break;

  case 262:
#line 1974 "parser.y"
    {
        (yyval.ocla) = NULL;
    }
    break;

  case 263:
#line 1978 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (2)].ocla), (yyvsp[(2) - (2)].ocla));
    }
    break;

  case 264:
#line 1982 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (3)].ocla), (yyvsp[(3) - (3)].ocla));
    }
    break;

  case 265:
#line 1989 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    }
    break;

  case 266:
#line 1993 "parser.y"
    {
        (yyval.ocla) = PlainClause(OCNOWAIT);
    }
    break;

  case 267:
#line 2000 "parser.y"
    {
        (yyval.stmt) = Compound((yyvsp[(2) - (3)].stmt));
    }
    break;

  case 268:
#line 2007 "parser.y"
    {
        /* Make it look like it had a section pragma */
        (yyval.stmt) = OmpStmt( OmpConstruct(DCSECTION, OmpDirective(DCSECTION,NULL), (yyvsp[(1) - (1)].stmt)) );
    }
    break;

  case 269:
#line 2012 "parser.y"
    {
        (yyval.stmt) = OmpStmt( OmpConstruct(DCSECTION, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt)) );
    }
    break;

  case 270:
#line 2016 "parser.y"
    {
        (yyval.stmt) = BlockList((yyvsp[(1) - (3)].stmt), OmpStmt( OmpConstruct(DCSECTION, (yyvsp[(2) - (3)].odir), (yyvsp[(3) - (3)].stmt)) ));
    }
    break;

  case 271:
#line 2023 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCSECTION, NULL);
    }
    break;

  case 272:
#line 2030 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCSINGLE, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
    }
    break;

  case 273:
#line 2037 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCSINGLE, (yyvsp[(3) - (4)].ocla));
    }
    break;

  case 274:
#line 2044 "parser.y"
    {
        (yyval.ocla) = NULL;
    }
    break;

  case 275:
#line 2048 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (2)].ocla), (yyvsp[(2) - (2)].ocla));
    }
    break;

  case 276:
#line 2052 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (3)].ocla), (yyvsp[(3) - (3)].ocla));
    }
    break;

  case 277:
#line 2059 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    }
    break;

  case 278:
#line 2063 "parser.y"
    {
        (yyval.ocla) = PlainClause(OCNOWAIT);
    }
    break;

  case 279:
#line 2070 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCPARFOR, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
    }
    break;

  case 280:
#line 2077 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCPARFOR, (yyvsp[(4) - (5)].ocla));
    }
    break;

  case 281:
#line 2084 "parser.y"
    {
        (yyval.ocla) = NULL;
    }
    break;

  case 282:
#line 2088 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (2)].ocla), (yyvsp[(2) - (2)].ocla));
    }
    break;

  case 283:
#line 2092 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (3)].ocla), (yyvsp[(3) - (3)].ocla));
    }
    break;

  case 284:
#line 2099 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    }
    break;

  case 285:
#line 2103 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    }
    break;

  case 286:
#line 2107 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    }
    break;

  case 287:
#line 2114 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCPARSECTIONS, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
    }
    break;

  case 288:
#line 2121 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCPARSECTIONS, (yyvsp[(4) - (5)].ocla));
    }
    break;

  case 289:
#line 2128 "parser.y"
    {
        (yyval.ocla) = NULL;
    }
    break;

  case 290:
#line 2132 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (2)].ocla), (yyvsp[(2) - (2)].ocla));
    }
    break;

  case 291:
#line 2136 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (3)].ocla), (yyvsp[(3) - (3)].ocla));
    }
    break;

  case 292:
#line 2143 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    }
    break;

  case 293:
#line 2147 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    }
    break;

  case 294:
#line 2155 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCTASK, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
        (yyval.ocon)->l = (yyvsp[(1) - (2)].odir)->l;
    }
    break;

  case 295:
#line 2164 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCTASK, (yyvsp[(3) - (4)].ocla));
    }
    break;

  case 296:
#line 2172 "parser.y"
    {
        (yyval.ocla) = NULL;
    }
    break;

  case 297:
#line 2176 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (2)].ocla), (yyvsp[(2) - (2)].ocla));
    }
    break;

  case 298:
#line 2180 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (3)].ocla), (yyvsp[(3) - (3)].ocla));
    }
    break;

  case 299:
#line 2188 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    }
    break;

  case 300:
#line 2192 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    }
    break;

  case 301:
#line 2199 "parser.y"
    { sc_pause_openmp(); }
    break;

  case 302:
#line 2200 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = IfClause((yyvsp[(4) - (5)].expr));
    }
    break;

  case 303:
#line 2205 "parser.y"
    {
        (yyval.ocla) = PlainClause(OCUNTIED);
    }
    break;

  case 304:
#line 2208 "parser.y"
    { sc_pause_openmp(); }
    break;

  case 305:
#line 2209 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = FinalClause((yyvsp[(4) - (5)].expr));
    }
    break;

  case 306:
#line 2214 "parser.y"
    {
        (yyval.ocla) = PlainClause(OCMERGEABLE);
    }
    break;

  case 307:
#line 2220 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCMASTER, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
    }
    break;

  case 308:
#line 2227 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCMASTER, NULL);
    }
    break;

  case 309:
#line 2234 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCCRITICAL, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
    }
    break;

  case 310:
#line 2241 "parser.y"
    {
        (yyval.odir) = OmpCriticalDirective(NULL);
    }
    break;

  case 311:
#line 2245 "parser.y"
    {
        (yyval.odir) = OmpCriticalDirective((yyvsp[(3) - (4)].symb));
    }
    break;

  case 312:
#line 2252 "parser.y"
    {
        (yyval.symb) = Symbol((yyvsp[(2) - (3)].name));
    }
    break;

  case 313:
#line 2260 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCTASKWAIT, NULL);
    }
    break;

  case 314:
#line 2268 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCTASKYIELD, NULL);
    }
    break;

  case 315:
#line 2275 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCBARRIER, NULL);
    }
    break;

  case 316:
#line 2282 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCATOMIC, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
    }
    break;

  case 317:
#line 2289 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCATOMIC, NULL);
    }
    break;

  case 318:
#line 2293 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCATOMIC, NULL);
    }
    break;

  case 319:
#line 2297 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCATOMIC, NULL);
    }
    break;

  case 320:
#line 2301 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCATOMIC, NULL);
    }
    break;

  case 321:
#line 2308 "parser.y"
    {
        (yyval.odir) = OmpFlushDirective(NULL);
    }
    break;

  case 322:
#line 2312 "parser.y"
    {
        (yyval.odir) = OmpFlushDirective((yyvsp[(3) - (4)].decl));
    }
    break;

  case 323:
#line 2318 "parser.y"
    { sc_pause_openmp(); }
    break;

  case 324:
#line 2319 "parser.y"
    {
        sc_start_openmp();
        (yyval.decl) = (yyvsp[(3) - (4)].decl);
    }
    break;

  case 325:
#line 2327 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCORDERED, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
    }
    break;

  case 326:
#line 2334 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCORDERED, NULL);
    }
    break;

  case 327:
#line 2341 "parser.y"
    {
        (yyval.odir) = OmpThreadprivateDirective((yyvsp[(3) - (5)].decl));
    }
    break;

  case 328:
#line 2347 "parser.y"
    { sc_pause_openmp(); }
    break;

  case 329:
#line 2348 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = VarlistClause(OCPRIVATE, (yyvsp[(4) - (5)].decl));
    }
    break;

  case 330:
#line 2353 "parser.y"
    { sc_pause_openmp(); }
    break;

  case 331:
#line 2354 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = VarlistClause(OCCOPYPRIVATE, (yyvsp[(4) - (5)].decl));
    }
    break;

  case 332:
#line 2358 "parser.y"
    { sc_pause_openmp(); }
    break;

  case 333:
#line 2359 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = VarlistClause(OCFIRSTPRIVATE, (yyvsp[(4) - (5)].decl));
    }
    break;

  case 334:
#line 2363 "parser.y"
    { sc_pause_openmp(); }
    break;

  case 335:
#line 2364 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = VarlistClause(OCLASTPRIVATE, (yyvsp[(4) - (5)].decl));
    }
    break;

  case 336:
#line 2368 "parser.y"
    { sc_pause_openmp(); }
    break;

  case 337:
#line 2369 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = VarlistClause(OCSHARED, (yyvsp[(4) - (5)].decl));
    }
    break;

  case 338:
#line 2374 "parser.y"
    {
        (yyval.ocla) = DefaultClause(OC_defshared);
    }
    break;

  case 339:
#line 2378 "parser.y"
    {
        (yyval.ocla) = DefaultClause(OC_defnone);
    }
    break;

  case 340:
#line 2381 "parser.y"
    { sc_pause_openmp(); }
    break;

  case 341:
#line 2382 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = ReductionClause((yyvsp[(3) - (7)].type), (yyvsp[(6) - (7)].decl));
    }
    break;

  case 342:
#line 2386 "parser.y"
    { sc_pause_openmp(); }
    break;

  case 343:
#line 2387 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = VarlistClause(OCCOPYIN, (yyvsp[(4) - (5)].decl));
    }
    break;

  case 344:
#line 2395 "parser.y"
    {
        (yyval.type) = OC_plus;
    }
    break;

  case 345:
#line 2399 "parser.y"
    {
        (yyval.type) = OC_times;
    }
    break;

  case 346:
#line 2403 "parser.y"
    {
        (yyval.type) = OC_minus;
    }
    break;

  case 347:
#line 2407 "parser.y"
    {
        (yyval.type) = OC_band;
    }
    break;

  case 348:
#line 2411 "parser.y"
    {
        (yyval.type) = OC_xor;
    }
    break;

  case 349:
#line 2415 "parser.y"
    {
        (yyval.type) = OC_bor;
    }
    break;

  case 350:
#line 2419 "parser.y"
    {
        (yyval.type) = OC_land;
    }
    break;

  case 351:
#line 2423 "parser.y"
    {
        (yyval.type) = OC_lor;
    }
    break;

  case 352:
#line 2427 "parser.y"
    {
        (yyval.type) = OC_min;
    }
    break;

  case 353:
#line 2431 "parser.y"
    {
        (yyval.type) = OC_max;
    }
    break;

  case 354:
#line 2438 "parser.y"
    {
        if (checkDecls)
        if (symtab_get(stab, Symbol((yyvsp[(1) - (1)].name)), IDNAME) == NULL)
            parse_error(-1, "unknown identifier `%s'.\n", (yyvsp[(1) - (1)].name));
        (yyval.decl) = IdentifierDecl( Symbol((yyvsp[(1) - (1)].name)) );
    }
    break;

  case 355:
#line 2445 "parser.y"
    {
        if (checkDecls)
        if (symtab_get(stab, Symbol((yyvsp[(3) - (3)].name)), IDNAME) == NULL)
            parse_error(-1, "unknown identifier `%s'.\n", (yyvsp[(3) - (3)].name));
        (yyval.decl) = IdList((yyvsp[(1) - (3)].decl), IdentifierDecl( Symbol((yyvsp[(3) - (3)].name)) ));
    }
    break;

  case 356:
#line 2460 "parser.y"
    {
        if (checkDecls)
        {
            stentry e = symtab_get(stab, Symbol((yyvsp[(1) - (1)].name)), IDNAME);
            if (e == NULL)
                parse_error(-1, "unknown identifier `%s'.\n", (yyvsp[(1) - (1)].name));
            if (e->scopelevel != stab->scopelevel)
                parse_error(-1, "threadprivate directive appears at different "
                    "scope level\nfrom the one `%s' was declared.\n", (yyvsp[(1) - (1)].name));
            if (stab->scopelevel > 0)    /* Don't care for globals */
            if (speclist_getspec(e->spec, STCLASSSPEC, SPEC_static) == NULL)
                parse_error(-1, "threadprivate variable `%s' does not have static "
                "storage type.\n", (yyvsp[(1) - (1)].name));
            e->isthrpriv = 1;   /* Mark */
        }
        (yyval.decl) = IdentifierDecl( Symbol((yyvsp[(1) - (1)].name)) );
    }
    break;

  case 357:
#line 2478 "parser.y"
    {
        if (checkDecls)
        {
            stentry e = symtab_get(stab, Symbol((yyvsp[(3) - (3)].name)), IDNAME);
            if (e == NULL)
                parse_error(-1, "unknown identifier `%s'.\n", (yyvsp[(3) - (3)].name));
            if (e->scopelevel != stab->scopelevel)
                parse_error(-1, "threadprivate directive appears at different "
                "scope level\nfrom the one `%s' was declared.\n", (yyvsp[(3) - (3)].name));
            if (stab->scopelevel > 0)    /* Don't care for globals */
            if (speclist_getspec(e->spec, STCLASSSPEC, SPEC_static) == NULL)
                parse_error(-1, "threadprivate variable `%s' does not have static "
                "storage type.\n", (yyvsp[(3) - (3)].name));
            e->isthrpriv = 1;   /* Mark */
        }
        (yyval.decl) = IdList((yyvsp[(1) - (3)].decl), IdentifierDecl( Symbol((yyvsp[(3) - (3)].name)) ));
    }
    break;

  case 358:
#line 2504 "parser.y"
    {
        (yyval.xcon) = OmpixConstruct(OX_DCTASKSYNC, (yyvsp[(1) - (1)].xdir), NULL);
    }
    break;

  case 359:
#line 2508 "parser.y"
    {
        (yyval.xcon) = OmpixConstruct(OX_DCTASKSCHEDULE, (yyvsp[(1) - (1)].xdir), NULL);
    }
    break;

  case 360:
#line 2516 "parser.y"
    {
        (yyval.xdir) = OmpixDirective(OX_DCTASKSYNC, NULL);
    }
    break;

  case 361:
#line 2523 "parser.y"
    {
        scope_start(stab);
    }
    break;

  case 362:
#line 2527 "parser.y"
    {
        scope_end(stab);
        (yyval.xdir) = OmpixDirective(OX_DCTASKSCHEDULE, (yyvsp[(4) - (5)].xcla));
    }
    break;

  case 363:
#line 2535 "parser.y"
    {
        (yyval.xcla) = NULL;
    }
    break;

  case 364:
#line 2539 "parser.y"
    {
        (yyval.xcla) = OmpixClauseList((yyvsp[(1) - (2)].xcla), (yyvsp[(2) - (2)].xcla));
    }
    break;

  case 365:
#line 2543 "parser.y"
    {
        (yyval.xcla) = OmpixClauseList((yyvsp[(1) - (3)].xcla), (yyvsp[(3) - (3)].xcla));
    }
    break;

  case 366:
#line 2550 "parser.y"
    {
        (yyval.xcla) = OmpixStrideClause((yyvsp[(3) - (4)].expr));
    }
    break;

  case 367:
#line 2554 "parser.y"
    {
        (yyval.xcla) = OmpixStartClause((yyvsp[(3) - (4)].expr));
    }
    break;

  case 368:
#line 2558 "parser.y"
    {
        (yyval.xcla) = OmpixScopeClause((yyvsp[(3) - (4)].type));
    }
    break;

  case 369:
#line 2562 "parser.y"
    {
        (yyval.xcla) = OmpixPlainClause(OX_OCTIED);
    }
    break;

  case 370:
#line 2566 "parser.y"
    {
        (yyval.xcla) = OmpixPlainClause(OX_OCUNTIED);
    }
    break;

  case 371:
#line 2573 "parser.y"
    {
        (yyval.type) = OX_SCOPE_NODES;
    }
    break;

  case 372:
#line 2577 "parser.y"
    {
        (yyval.type) = OX_SCOPE_WGLOBAL;
    }
    break;

  case 373:
#line 2581 "parser.y"
    {
        (yyval.type) = OX_SCOPE_WGLOBAL;
    }
    break;

  case 374:
#line 2585 "parser.y"
    {
        (yyval.type) = OX_SCOPE_WLOCAL;
    }
    break;

  case 375:
#line 2592 "parser.y"
    {
        (yyval.xcon) = (yyvsp[(1) - (1)].xcon);
    }
    break;

  case 376:
#line 2596 "parser.y"
    {
        (yyval.xcon) = (yyvsp[(1) - (1)].xcon);
    }
    break;

  case 377:
#line 2604 "parser.y"
    {
        /* Should put the name of the callback function in the stab, too
     if (symtab_get(stab, decl_getidentifier_symbol($2->u.declaration.decl),
     FUNCNAME) == NULL)
     symtab_put(stab, decl_getidentifier_symbol($2->u.declaration.spec),
     FUNCNAME);
     */
        scope_start(stab);   /* re-declare the arguments of the task function */
        ast_declare_function_params((yyvsp[(2) - (2)].stmt)->u.declaration.decl);
    }
    break;

  case 378:
#line 2615 "parser.y"
    {
        scope_end(stab);
        (yyval.xcon) = OmpixTaskdef((yyvsp[(1) - (4)].xdir), (yyvsp[(2) - (4)].stmt), (yyvsp[(4) - (4)].stmt));
        (yyval.xcon)->l = (yyvsp[(1) - (4)].xdir)->l;
    }
    break;

  case 379:
#line 2621 "parser.y"
    {
        (yyval.xcon) = OmpixTaskdef((yyvsp[(1) - (2)].xdir), (yyvsp[(2) - (2)].stmt), NULL);
        (yyval.xcon)->l = (yyvsp[(1) - (2)].xdir)->l;
    }
    break;

  case 380:
#line 2629 "parser.y"
    {
        scope_start(stab);
    }
    break;

  case 381:
#line 2633 "parser.y"
    {
        scope_end(stab);
        (yyval.xdir) = OmpixDirective(OX_DCTASKDEF, (yyvsp[(4) - (5)].xcla));
    }
    break;

  case 382:
#line 2641 "parser.y"
    {
        (yyval.xcla) = NULL;
    }
    break;

  case 383:
#line 2645 "parser.y"
    {
        (yyval.xcla) = OmpixClauseList((yyvsp[(1) - (2)].xcla), (yyvsp[(2) - (2)].xcla));
    }
    break;

  case 384:
#line 2649 "parser.y"
    {
        (yyval.xcla) = OmpixClauseList((yyvsp[(1) - (3)].xcla), (yyvsp[(3) - (3)].xcla));
    }
    break;

  case 385:
#line 2656 "parser.y"
    {
        (yyval.xcla) = OmpixVarlistClause(OX_OCIN, (yyvsp[(3) - (4)].decl));
    }
    break;

  case 386:
#line 2660 "parser.y"
    {
        (yyval.xcla) = OmpixVarlistClause(OX_OCOUT, (yyvsp[(3) - (4)].decl));
    }
    break;

  case 387:
#line 2664 "parser.y"
    {
        (yyval.xcla) = OmpixVarlistClause(OX_OCINOUT, (yyvsp[(3) - (4)].decl));
    }
    break;

  case 388:
#line 2668 "parser.y"
    {
        (yyval.xcla) = OmpixReductionClause((yyvsp[(3) - (6)].type), (yyvsp[(5) - (6)].decl));
    }
    break;

  case 389:
#line 2675 "parser.y"
    {
        (yyval.decl) = (yyvsp[(1) - (1)].decl);
    }
    break;

  case 390:
#line 2679 "parser.y"
    {
        (yyval.decl) = IdList((yyvsp[(1) - (3)].decl), (yyvsp[(3) - (3)].decl));
    }
    break;

  case 391:
#line 2686 "parser.y"
    {
        (yyval.decl) = IdentifierDecl( Symbol((yyvsp[(1) - (1)].name)) );
        symtab_put(stab, Symbol((yyvsp[(1) - (1)].name)), IDNAME);
    }
    break;

  case 392:
#line 2691 "parser.y"
    {
        if (checkDecls) check_uknown_var((yyvsp[(4) - (5)].name));
        /* Use extern to differentiate */
            (yyval.decl) = ArrayDecl(IdentifierDecl( Symbol((yyvsp[(1) - (5)].name)) ), StClassSpec(SPEC_extern),
        Identifier(Symbol((yyvsp[(4) - (5)].name))));
        symtab_put(stab, Symbol((yyvsp[(1) - (5)].name)), IDNAME);
    }
    break;

  case 393:
#line 2699 "parser.y"
    {
        (yyval.decl) = ArrayDecl(IdentifierDecl( Symbol((yyvsp[(1) - (4)].name)) ), NULL, (yyvsp[(3) - (4)].expr));
        symtab_put(stab, Symbol((yyvsp[(1) - (4)].name)), IDNAME);
    }
    break;

  case 394:
#line 2707 "parser.y"
    {
        (yyval.xcon) = OmpixConstruct(OX_DCTASK, (yyvsp[(1) - (3)].xdir), Expression((yyvsp[(2) - (3)].expr)));
        (yyval.xcon)->l = (yyvsp[(1) - (3)].xdir)->l;
    }
    break;

  case 395:
#line 2715 "parser.y"
    {
        (yyval.xdir) = OmpixDirective(OX_DCTASK, (yyvsp[(3) - (4)].xcla));
    }
    break;

  case 396:
#line 2722 "parser.y"
    {
        (yyval.xcla) = NULL;
    }
    break;

  case 397:
#line 2726 "parser.y"
    {
        (yyval.xcla) = OmpixClauseList((yyvsp[(1) - (2)].xcla), (yyvsp[(2) - (2)].xcla));
    }
    break;

  case 398:
#line 2730 "parser.y"
    {
        (yyval.xcla) = OmpixClauseList((yyvsp[(1) - (3)].xcla), (yyvsp[(3) - (3)].xcla));
    }
    break;

  case 399:
#line 2737 "parser.y"
    {
        (yyval.xcla) = OmpixPlainClause(OX_OCATALL);
    }
    break;

  case 400:
#line 2741 "parser.y"
    {
        (yyval.xcla) = OmpixAtnodeClause((yyvsp[(3) - (4)].expr));
    }
    break;

  case 401:
#line 2745 "parser.y"
    {
        (yyval.xcla) = OmpixAtworkerClause((yyvsp[(3) - (4)].expr));
    }
    break;

  case 402:
#line 2749 "parser.y"
    {
        (yyval.xcla) = OmpixPlainClause(OX_OCTIED);
    }
    break;

  case 403:
#line 2753 "parser.y"
    {
        (yyval.xcla) = OmpixPlainClause(OX_OCUNTIED);
    }
    break;

  case 404:
#line 2757 "parser.y"
    {
        (yyval.xcla) = OmpixPlainClause(OX_OCDETACHED);
    }
    break;

  case 405:
#line 2764 "parser.y"
    {
        (yyval.expr) = strcmp((yyvsp[(1) - (3)].name), "main") ?
        FunctionCall(Identifier(Symbol((yyvsp[(1) - (3)].name))), NULL) :
        FunctionCall(Identifier(Symbol(MAIN_NEWNAME)), NULL);
    }
    break;

  case 406:
#line 2770 "parser.y"
    {
        (yyval.expr) = strcmp((yyvsp[(1) - (4)].name), "main") ?
        FunctionCall(Identifier(Symbol((yyvsp[(1) - (4)].name))), (yyvsp[(3) - (4)].expr)) :
        FunctionCall(Identifier(Symbol(MAIN_NEWNAME)), (yyvsp[(3) - (4)].expr));
    }
    break;


/* Line 1267 of yacc.c.  */
#line 5668 "parser.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 2777 "parser.y"



/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     CODE                                                      *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


void yyerror(char *s)
{
  fprintf(stderr, "(file %s, line %d, column %d):\n\t%s\n",
                  sc_original_file(), sc_original_line(), sc_column(), s);
}


/* Check whether the identifier is known or not
 */
void check_uknown_var(char *name)
{
  symbol s = Symbol(name);
  if (symtab_get(stab, s, IDNAME) == NULL &&
      symtab_get(stab, s, LABELNAME) == NULL &&
      symtab_get(stab, s, FUNCNAME) == NULL)
    parse_error(-1, "unknown identifier `%s'.\n", name);
}

void check_for_main_and_declare(astspec s, astdecl d)
{
  astdecl n = decl_getidentifier(d);

  assert(d->type == DECLARATOR);
  assert(d->decl->type == DFUNC);
  
  if (strcmp(n->u.id->name, "main") == 0)
  {
    n->u.id = Symbol(MAIN_NEWNAME);         /* Catch main()'s definition */
    hasMainfunc = 1;
    /* Now check for return type and # parameters */
    /* It d != NULL then its parameters is either (id or idlist) or
     * (paramtype or parmatypelist). If it is a list, assume the
     * standard 2 params, otherwise, we guess the single argument
     * must be the type "void" which means no params.
     * In any case, we always force main have (argc, argv[]).
     */
    if (d->decl->u.params == NULL || d->decl->u.params->type != DLIST)
      d->decl->u.params =
          ParamList(
            ParamDecl(
              Speclist_right(Declspec(SPEC_public, 0), Declspec(SPEC_int, 0)),
              Declarator( NULL, IdentifierDecl( Symbol("_argc_ignored") ) )
            ),
            ParamDecl(
              Speclist_right(Declspec(SPEC_public, 0), Declspec(SPEC_char, 0)),
              Declarator(Speclist_right( Pointer(), Pointer() ),
                         IdentifierDecl( Symbol("_argv_ignored") ))
            )
          );
   
    mainfuncRettype = 0; /* int */
    if (s != NULL)
    {
      for (; s->type == SPECLIST && s->subtype == SPEC_Rlist; s = s->u.next)
        if (s->body->type == SPEC && s->body->subtype == SPEC_void)
        {
          s = s->body;
          break;
        };
      if (s->type == SPEC && s->subtype == SPEC_void)
        mainfuncRettype = 1; /* void */
    }
  }
    if (symtab_get(stab, n->u.id, FUNCNAME) == NULL)/* From earlier declaration */
        symtab_put(stab, n->u.id, FUNCNAME);
}


/* For each variable/typename in the given declaration, add pointers in the
 * symbol table entries back to the declaration nodes.
 */

void add_declaration_links(astspec s, astdecl d)
{
  astdecl ini = NULL;
  if (d->type == DLIST && d->subtype == DECL_decllist)
  {
    add_declaration_links(s, d->u.next);
    d = d->decl;
  }
  if (d->type == DINIT) d = (ini = d)->decl;   /* Skip the initializer */
  assert(d->type == DECLARATOR);
  if (d->decl != NULL && d->decl->type != ABSDECLARATOR)
  {
    symbol  t = decl_getidentifier_symbol(d->decl);
    stentry e = isTypedef ?
                symtab_get(stab,t,TYPENAME) :
                symtab_get(stab,t,(decl_getkind(d)==DFUNC) ? FUNCNAME : IDNAME);
    e->spec  = s;
    e->decl  = d;
    e->idecl = ini;
  }
}

void set_security_flag_symbol(astexpr e, symbol s)
{
	stentry entry = symtab_get(stab, s, IDNAME);
	// default is private
	if(entry->spec->body == NULL){
		e->flag = PRI;
        e->index = 0;
    }
	else if(entry->spec->body->subtype == SPEC_public){
        e->flag = PUB;
        e->index = -1;
    }
	else{
		e->flag = PRI;
        e->index = 0; 
    }
}

void set_security_flag_expr(astexpr e, astexpr e1, astexpr e2, int opid){
    //BOP
    if(e2 != NULL && e1 != NULL){
        if(e1->flag == PUB && e2->flag == PUB){
            e->flag = PUB;
            e->index = -1;
        }
        else{
                e->flag = PRI;
                e->index = tmp_index;
                // assume e1 and e2 are arrays and have the same size
                if(e1->arraysize != NULL && e2->arraysize != NULL)
                    e->arraysize = e1->arraysize; 
            }
    }
    //() or UOP or ASS
    else if(e2 == NULL && e1 != NULL){
          e->flag = e1->flag;
          e->index = e1->index; 
	}
    //const
    else if(e1 == NULL && e2 == NULL){
        e->flag = PUB;
        e->index = -1; 
    }
} 

void security_check_for_assignment(astexpr le, astexpr re){
	if(le->flag != re->flag) 
		parse_error(-1, "security type mismatch in the assignment\n");
} 
void security_check_for_declaration(astspec spec, astdecl decl){
 
    int flag1 = 0;
    int flag2 = 0;
    
    // specifier
    if(spec->type == SPEC)
        flag1 = 1;
    else if(spec->type == SPECLIST && spec->body->subtype == SPEC_private){
        flag1 = 1;
    }
    
    // declarator
    if(decl->type == DLIST)
    {
       while(1){
            if(decl->type != DLIST)
                break;
            if(decl->decl->type == DINIT)
            {                
                if(decl->decl->u.expr->flag == PRI)
                    flag2 = 1;
            }
            if(flag1 == 0 && flag2 == 1)
                parse_error(-1, "security type mismatch in the assignment\n");
            decl = decl->u.next;
        }
        
        if(decl->type == DINIT)
        {
            if(decl->u.expr->flag == PRI)
                flag2 = 1;
            
            if(flag1 == 0 && flag2 == 1)
                parse_error(-1, "security type mismatch in the assignment\n");
        }
    
    }
    else{
        if(decl->type == DINIT)
        {
            if(decl->u.expr->flag == PRI)
                flag2 = 1;

            if(flag1 == 0 && flag2 == 1)
                parse_error(-1, "security type mismatch in the assignment\n");
        }
    }
  
}
void set_security_flag_stmt(aststmt s, aststmt s1, aststmt s2){
    if(s->type = STATEMENTLIST){
        if((s1->flag == PUB) || (s2->flag == PUB))
           s->flag = PUB;
    }
}
void set_security_flag_func(char* funcname, astexpr e2){
    stentry e;
    astspec spec;
         
    if(!strcmp(funcname, "bits") || !strcmp(funcname, "inv"))
    {
        e2->flag = PRI;
        e2->index = 0; 
    }
    else
    {
        e = symtab_get(stab, Symbol(funcname), FUNCNAME);
        if(e == NULL){
                e2->flag = PUB;
                return;
        }
        spec = e->spec;
        e2->flag = set_security_flag_spec(spec);
        e2->index = 0;
    }
}
int set_security_flag_spec(astspec spec){
    if(spec->type == SPECLIST){
        if(spec->body->subtype == SPEC_public)
            return PUB;
        else{
            return PRI;
        }
    }
    else
        return PRI;
}

void set_size_symbol(astexpr e1, astexpr e2, astexpr e3){
    stentry entry;
    astexpr e = e2;
    astdecl d, d1;
    int num = 0;
    if(e->type != IDENT)
        while(e->type != IDENT)
            e = e->left;
    entry = symtab_get(stab, e->u.sym, IDNAME);
    // set the length of expr
    if(set_security_flag_spec(entry->spec) == PUB)
        e1->size = -1;
    else if(entry->spec->type == SPECLIST)
        e1->size = entry->spec->u.next->size;
    else
        e1->size = entry->spec->size;
    // if e2 is an array, we further store its size
    
    if(entry->isarray){
        d = entry->decl->decl;
        // for one dimension array name
        if(e2->type == IDENT && e3 == NULL)
            e1->arraysize = d->u.expr;
        else if(e2->type == IDENT && e3 != NULL)
            e1->arraysize = NULL;
        else if (e2->type == ARRAYIDX){
            d1 = d; 
            while(e2->type != IDENT){
                e2->arraysize = d1->u.expr;
                e2 = e2->left;
                d1 = d1->decl; 
                num++;
            }
            while(num >= 0){
                d = d->decl;
                num--;
            }
            if(d->type == DARRAY)
                e1->arraysize = d->u.expr;
            else
                e1->arraysize = NULL;
        }
            
    }
    // for identifier
    else
        e1->arraysize = NULL;
}
void security_check_for_condition(astexpr e){
    int flag  = 0;
        if(e->type == COMMALIST)
        {
            while(1)
            {
                if(e->right->flag == PRI){
                    flag = 1;
                }
                if(e->left->type != COMMALIST)
                    break;
                e = e->left;
            }
            if(e->left->flag == PRI){
                flag = 1;
            }
        }
        else
        {
            if(e->flag == PRI)
                flag = 1;
        }
           
        if(flag == 1)
            parse_error(-1, "please open the private condition before computation\n");
    
}
stentry get_entry_from_expr(astexpr op){
    astexpr e = op; 
    if(e->type == IDENT)
        return symtab_get(stab, e->u.sym, IDNAME);
    else if (e->type == ARRAYIDX){
        while (e->type != IDENT)
            e = e->left;
        return symtab_get(stab, e->u.sym, IDNAME); 
    }
}
// something is wrong here 
int check_func_param(astexpr funcname, astexpr arglist){
    // if argument list contains correct type arguments, it returns 1,
    // otherwise it returns 0
    int flag = 0; 
    // get the function name
    astdecl decl;
    astspec spec;
    stentry e;
    // check if the function is declared
    if ((e = symtab_get(stab, funcname->u.sym, FUNCNAME)) == NULL)
        return 0;
       //parse_error(-1, "function has not been defined yet\n");
    else
        // get the paramater list
    decl = e->decl->decl->u.params;
    
    if(arglist->type == COMMALIST)
    {
        while(1)
        {
            // search for the type of var (from right to left)
            
            spec = (decl->decl)->spec;
            if(compare_specs(spec, arglist->right->flag))
                return 1;
            arglist = arglist->left;
            decl = decl->u.next;
            if((arglist->type != COMMALIST) || (decl->type != DLIST))
                break;
        }
        // for the leftmost var
        if((decl->type == DLIST && arglist->type != COMMALIST) || (decl->type == DPARAM && arglist->type == COMMALIST)){
            parse_error(1, "The provided arguments do not match function parameters\n");
        }
        else{
            spec = decl->spec;
            if(compare_specs(spec, arglist->flag) != 0)
                return 1;
        }
    }
    else{
        if(decl->type != DPARAM)
            flag = 1;
        else{
                spec = decl->spec;
                if(compare_specs(spec, arglist->flag))
                    return 1;
        }

    }
    return flag;
    
}

int compare_specs(astspec spec, int flag){
    return 1 - (set_security_flag_spec(spec) == flag);
}
    
void increase_index(astexpr e){
    if(e->u.sym == NULL && e->flag == PRI){
        var_index++;
        tmp_index++;
    }
    num_index = num_index > tmp_index? num_index: tmp_index;
}

void decrease_index(astexpr e){
    if(e->u.sym == NULL && e->flag == PRI){
        var_index--;
        tmp_index--;
    }
}

void parse_error(int exitvalue, char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  fprintf(stderr, "(%s, line %d)\n\t", sc_original_file(), sc_original_line());
  vfprintf(stderr, format, ap);
  va_end(ap);
  if (strcmp(sc_original_file(), "injected_code") == 0)
    fprintf(stderr, "\n>>>>>>>\n%s\n>>>>>>>\n", parsingstring);
  _exit(exitvalue);
}


aststmt parse_file(char *fname, int *error)
{
  *error = 0;
  if ( (yyin = fopen(fname, "r")) == NULL )
    return (NULL);
  sc_set_filename(fname);      /* Inform the scanner */
  *error = yyparse();
  fclose(yyin);                /* No longer needed */
  return (pastree);
}

#define PARSE_STRING_SIZE 8192

astexpr parse_expression_string(char *format, ...)
{
    static char s[PARSE_STRING_SIZE];
    int    savecD;
    
    va_list ap;
    va_start(ap, format);
    vsnprintf(s, PARSE_STRING_SIZE-1, format, ap);
    va_end(ap);
    parsingstring = s;
    sc_scan_string(s);
    sc_set_start_token(START_SYMBOL_EXPRESSION);
    
    savecD = checkDecls;
    checkDecls = 0;         /* Don't check identifiers & don't declare them */
    yyparse();
    checkDecls = savecD;    /* Reset it */
    return ( pastree_expr );
}


aststmt parse_blocklist_string(char *format, ...)
{
    static char s[PARSE_STRING_SIZE];
    int    savecD;
    va_list ap;
    va_start(ap, format);
    vsnprintf(s, PARSE_STRING_SIZE-1, format, ap);
    va_end(ap);
    parsingstring = s;
    sc_scan_string(s);
    sc_set_start_token(START_SYMBOL_BLOCKLIST);
    
    savecD = checkDecls;
    checkDecls = 0;         /* Don't check identifiers & don't declare them */
    yyparse();
    checkDecls = savecD;    /* Reset it */
    return ( pastree_stmt );
}


aststmt parse_and_declare_blocklist_string(char *format, ...)
{
    static char s[PARSE_STRING_SIZE];
    int    savecD;
    printf("%s\n", format);

    va_list ap;
    va_start(ap, format);
    vsnprintf(s, PARSE_STRING_SIZE-1, format, ap);
    va_end(ap);
    parsingstring = s;
    sc_scan_string(s);
    sc_set_start_token(START_SYMBOL_BLOCKLIST);
    
    savecD = checkDecls;
    checkDecls = 1;         /* Do check identifiers & do declare them */
    yyparse();
    checkDecls = savecD;    /* Reset it */
    return ( pastree_stmt );
}


