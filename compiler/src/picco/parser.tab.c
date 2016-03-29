
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
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
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>
#include "scanner.h"
#include "picco.h"
#include "ast.h"
#include "symtab.h"
#include "ast_vars.h"
#include "ast_free.h"
#include "ast_copy.h"
#include "x_arith.h"
#include "ast_print.h"    
    
void    check_uknown_var(char *name);
void    parse_error(int exitvalue, char *format, ...);
void    yyerror(char *s);
void    check_for_main_and_declare(astspec s, astdecl d);
void    add_declaration_links(astspec s, astdecl d, int);
void    set_pointer_flag(astspec s, astdecl d);
void    set_security_flag_symbol(astexpr e, symbol s, int); 
void 	set_security_flag_expr(astexpr e, astexpr e1, astexpr e2, int opid);
void 	set_security_flag_stmt(aststmt s, aststmt s1, aststmt s2);
void    set_security_flag_func(char* funcname, astexpr e2);
void 	set_bitlength_expr(astexpr e, astexpr e1, astexpr e2); 
int     set_security_flag_spec(astspec spec);
void    set_size_symbol(astexpr, astexpr, astexpr);
void 	set_identifier_attributes(symbol, astexpr, int); 
int 	compute_ptr_level(astdecl decl); 
int     check_func_param(astexpr funcname, astexpr arglist);
stentry get_entry_from_expr(astexpr);
void    get_arraysize(astexpr, str);
void	compute_modulus_for_declaration(astspec); 
void 	compute_modulus_for_BOP(astexpr, astexpr, int); 
int     compare_specs(astspec, int);
astdecl fix_known_typename(astspec s);
void store_struct_information(struct_node_stack, astspec);

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
int 	tmp_float_index = 0; 
int     contain_priv_if_flag = 0;
int     cond_index = 0; 
int     num_index = 0;
int 	num_float_index = 0; 
int     func_return_flag = 0;
int     default_var_size = 32;/* Default var size - 32 bits */
int     is_priv_int_ptr_appear = 0;
int     is_priv_float_ptr_appear = 0;
int	is_priv_int_index_appear = 0; 
int 	is_priv_float_index_appear = 0;
int 	is_priv_int_ptr_struct_field_appear = 0; 
int 	is_priv_float_ptr_struct_field_appear = 0; 
int 	is_priv_struct_ptr_struct_field_appear = 0; 
int 	is_priv_int_struct_field_appear = 0; 
int 	is_priv_float_struct_field_appear = 0; 
int 	thread_id = -1;  
int	num_threads = 0;
int 	modulus = 0; 
 
struct_node_stack struct_table = NULL;
 
char    *parsingstring;       /* For error reporting when parsing string */
FILE 	*var_file; 


/* Line 189 of yacc.c  */
#line 188 "parser.tab.c"

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
     PFREE = 364
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 115 "parser.y"

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




/* Line 214 of yacc.c  */
#line 354 "parser.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 366 "parser.tab.c"

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
# if YYENABLE_NLS
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
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
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
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
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
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  123
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   2007

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  135
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  192
/* YYNRULES -- Number of rules.  */
#define YYNRULES  460
/* YYNRULES -- Number of states.  */
#define YYNSTATES  838

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   364

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     134,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   119,     2,     2,     2,   123,   120,     2,
     110,   111,   121,   124,   117,   118,   114,   122,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   131,   133,
     125,   132,   126,   130,   129,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   112,     2,   113,   127,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,   115,   128,   116,     2,     2,     2,     2,
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
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,    10,    13,    15,    18,    20,
      22,    24,    28,    30,    35,    39,    44,    48,    53,    57,
      61,    64,    67,    71,    75,    82,    90,    92,    96,    98,
     101,   104,   107,   110,   113,   118,   125,   127,   129,   131,
     133,   135,   138,   140,   142,   143,   149,   151,   152,   157,
     158,   163,   164,   169,   171,   172,   177,   178,   183,   185,
     186,   191,   192,   197,   199,   200,   205,   206,   211,   212,
     217,   218,   223,   225,   226,   231,   232,   237,   239,   240,
     245,   247,   248,   253,   255,   256,   261,   263,   264,   269,
     271,   272,   277,   281,   283,   285,   291,   293,   297,   299,
     301,   303,   305,   307,   309,   311,   315,   317,   320,   324,
     326,   328,   331,   333,   336,   338,   340,   342,   346,   348,
     349,   354,   356,   358,   360,   365,   372,   374,   376,   378,
     380,   382,   385,   389,   392,   394,   398,   400,   404,   407,
     412,   416,   422,   428,   431,   434,   436,   439,   442,   446,
     448,   450,   454,   457,   459,   462,   464,   466,   469,   473,
     476,   480,   484,   489,   493,   498,   501,   505,   509,   514,
     516,   519,   521,   524,   526,   528,   531,   533,   537,   541,
     546,   550,   555,   560,   562,   566,   568,   572,   577,   579,
     582,   586,   591,   594,   596,   599,   603,   606,   609,   611,
     613,   615,   617,   619,   621,   623,   625,   627,   629,   631,
     633,   634,   636,   641,   643,   646,   649,   650,   655,   657,
     660,   662,   664,   666,   668,   670,   673,   674,   681,   690,
     696,   704,   706,   713,   721,   729,   737,   746,   755,   764,
     774,   781,   789,   797,   806,   809,   812,   815,   819,   827,
     837,   849,   857,   867,   879,   891,   897,   899,   902,   904,
     906,   908,   910,   911,   916,   917,   921,   923,   925,   927,
     929,   931,   933,   935,   937,   939,   941,   943,   945,   947,
     949,   951,   953,   956,   961,   962,   965,   969,   971,   973,
     974,   980,   981,   987,   990,   995,   996,   999,  1003,  1005,
    1007,  1009,  1011,  1016,  1017,  1025,  1026,  1034,  1039,  1041,
    1043,  1045,  1047,  1049,  1051,  1054,  1059,  1060,  1063,  1067,
    1069,  1071,  1075,  1077,  1080,  1084,  1088,  1091,  1096,  1097,
    1100,  1104,  1106,  1108,  1111,  1117,  1118,  1121,  1125,  1127,
    1129,  1131,  1134,  1140,  1141,  1144,  1148,  1150,  1152,  1155,
    1160,  1161,  1164,  1168,  1170,  1172,  1173,  1179,  1181,  1182,
    1188,  1190,  1193,  1197,  1200,  1204,  1209,  1213,  1217,  1221,
    1225,  1228,  1232,  1237,  1242,  1247,  1251,  1256,  1257,  1262,
    1265,  1269,  1275,  1276,  1282,  1283,  1289,  1290,  1296,  1297,
    1303,  1304,  1310,  1315,  1320,  1321,  1329,  1330,  1336,  1338,
    1340,  1342,  1344,  1346,  1348,  1350,  1352,  1354,  1356,  1358,
    1362,  1364,  1368,  1370,  1372,  1376,  1377,  1383,  1384,  1387,
    1391,  1396,  1401,  1406,  1408,  1410,  1412,  1414,  1418,  1422,
    1424,  1426,  1427,  1432,  1435,  1436,  1442,  1443,  1446,  1450,
    1455,  1460,  1465,  1472,  1474,  1478,  1480,  1486,  1491,  1495,
    1500,  1501,  1504,  1508,  1513,  1518,  1523,  1525,  1527,  1529,
    1533
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     136,     0,    -1,    -1,   137,   229,    -1,     3,   179,    -1,
       4,   218,    -1,     7,    -1,   138,     7,    -1,     5,    -1,
       6,    -1,   138,    -1,   110,   179,   111,    -1,   139,    -1,
     140,   112,   179,   113,    -1,     5,   110,   111,    -1,     5,
     110,   141,   111,    -1,   140,   110,   111,    -1,   140,   110,
     141,   111,    -1,   140,   114,   183,    -1,   140,   105,   183,
      -1,   140,    13,    -1,   140,    14,    -1,   140,   114,     5,
      -1,   140,   105,     5,    -1,   110,   144,   111,   115,   206,
     116,    -1,   110,   144,   111,   115,   206,   117,   116,    -1,
     177,    -1,   141,   117,   177,    -1,   140,    -1,    13,   142,
      -1,    14,   142,    -1,   143,   142,    -1,   143,   145,    -1,
     107,   142,    -1,   107,   110,   144,   111,    -1,   108,   110,
     139,   117,   144,   111,    -1,   118,    -1,   119,    -1,   120,
      -1,   121,    -1,   201,    -1,   201,   199,    -1,   194,    -1,
     142,    -1,    -1,   110,   144,   111,   146,   145,    -1,   145,
      -1,    -1,   147,   121,   148,   145,    -1,    -1,   147,   122,
     149,   145,    -1,    -1,   147,   123,   150,   145,    -1,   147,
      -1,    -1,   151,   124,   152,   147,    -1,    -1,   151,   118,
     153,   147,    -1,   151,    -1,    -1,   154,    22,   155,   151,
      -1,    -1,   154,    21,   156,   151,    -1,   154,    -1,    -1,
     157,   125,   158,   154,    -1,    -1,   157,   126,   159,   154,
      -1,    -1,   157,    15,   160,   154,    -1,    -1,   157,    16,
     161,   154,    -1,   157,    -1,    -1,   162,    17,   163,   157,
      -1,    -1,   162,    18,   164,   157,    -1,   162,    -1,    -1,
     165,   120,   166,   162,    -1,   165,    -1,    -1,   167,   127,
     168,   165,    -1,   167,    -1,    -1,   169,   128,   170,   167,
      -1,   169,    -1,    -1,   171,    19,   172,   169,    -1,   171,
      -1,    -1,   173,    20,   174,   171,    -1,   142,   129,   142,
      -1,   173,    -1,   175,    -1,   173,   130,   179,   131,   176,
      -1,   176,    -1,   142,   178,   177,    -1,   132,    -1,    25,
      -1,    26,    -1,    27,    -1,    23,    -1,    24,    -1,   177,
      -1,   179,   117,   177,    -1,   176,    -1,   182,   133,    -1,
     182,   184,   133,    -1,   294,    -1,   187,    -1,   188,   187,
      -1,   194,    -1,   104,   194,    -1,   183,    -1,   106,    -1,
     185,    -1,   184,   117,   185,    -1,   202,    -1,    -1,   202,
     132,   186,   205,    -1,     8,    -1,   101,    -1,     9,    -1,
       8,   125,     6,   126,    -1,   101,   125,     6,   117,     6,
     126,    -1,    11,    -1,    12,    -1,   102,    -1,   103,    -1,
     191,    -1,   190,   191,    -1,   201,   192,   133,    -1,   201,
     133,    -1,   193,    -1,   192,   117,   193,    -1,   202,    -1,
     202,   131,   180,    -1,   131,   180,    -1,   189,   115,   190,
     116,    -1,   189,   115,   116,    -1,   189,     5,   115,   190,
     116,    -1,   189,   183,   115,   190,   116,    -1,   189,     5,
      -1,   189,   183,    -1,   121,    -1,   121,   188,    -1,   121,
     195,    -1,   121,   188,   195,    -1,   197,    -1,   198,    -1,
     197,   117,   198,    -1,   182,   202,    -1,   182,    -1,   182,
     199,    -1,   195,    -1,   200,    -1,   195,   200,    -1,   110,
     199,   111,    -1,   112,   113,    -1,   200,   112,   113,    -1,
     112,   177,   113,    -1,   200,   112,   177,   113,    -1,   112,
     121,   113,    -1,   200,   112,   121,   113,    -1,   110,   111,
      -1,   200,   110,   111,    -1,   110,   196,   111,    -1,   200,
     110,   196,   111,    -1,   187,    -1,   187,   201,    -1,   188,
      -1,   188,   201,    -1,   194,    -1,   203,    -1,   195,   203,
      -1,     5,    -1,   110,   202,   111,    -1,   203,   112,   113,
      -1,   203,   112,   177,   113,    -1,   203,   110,   111,    -1,
     203,   110,   204,   111,    -1,   203,   110,   196,   111,    -1,
       5,    -1,   204,   117,     5,    -1,   177,    -1,   115,   206,
     116,    -1,   115,   206,   117,   116,    -1,   205,    -1,   207,
     205,    -1,   206,   117,   205,    -1,   206,   117,   207,   205,
      -1,   208,   132,    -1,   209,    -1,   208,   209,    -1,   112,
     180,   113,    -1,   114,     5,    -1,   114,   183,    -1,   216,
      -1,   220,    -1,   225,    -1,   222,    -1,   223,    -1,   226,
      -1,   227,    -1,   228,    -1,   235,    -1,   313,    -1,   211,
      -1,   213,    -1,    -1,   215,    -1,   212,   112,   218,   113,
      -1,   214,    -1,   215,   214,    -1,   115,   116,    -1,    -1,
     115,   217,   218,   116,    -1,   219,    -1,   218,   219,    -1,
     181,    -1,   210,    -1,   236,    -1,   306,    -1,   133,    -1,
     179,   133,    -1,    -1,    28,   110,   179,   111,   221,   210,
      -1,    28,   110,   179,   111,   221,   210,    29,   210,    -1,
      30,   110,   179,   111,   210,    -1,    31,   210,    30,   110,
     179,   111,   133,    -1,   224,    -1,    32,   110,   133,   133,
     111,   210,    -1,    32,   110,   179,   133,   133,   111,   210,
      -1,    32,   110,   133,   179,   133,   111,   210,    -1,    32,
     110,   133,   133,   179,   111,   210,    -1,    32,   110,   179,
     133,   179,   133,   111,   210,    -1,    32,   110,   179,   133,
     133,   179,   111,   210,    -1,    32,   110,   133,   179,   133,
     179,   111,   210,    -1,    32,   110,   179,   133,   179,   133,
     179,   111,   210,    -1,    32,   110,   181,   133,   111,   210,
      -1,    32,   110,   181,   179,   133,   111,   210,    -1,    32,
     110,   181,   133,   179,   111,   210,    -1,    32,   110,   181,
     179,   133,   179,   111,   210,    -1,    33,   133,    -1,    34,
     133,    -1,    10,   133,    -1,    10,   179,   133,    -1,    35,
     110,   140,   117,   139,   111,   133,    -1,    35,   110,   140,
     117,   139,   117,   179,   111,   133,    -1,    35,   110,   140,
     117,   139,   117,   179,   117,   179,   111,   133,    -1,    36,
     110,   140,   117,   139,   111,   133,    -1,    36,   110,   140,
     117,   139,   117,   179,   111,   133,    -1,    36,   110,   140,
     117,   139,   117,   179,   117,   179,   111,   133,    -1,    32,
     110,   179,   133,   179,   133,   179,   111,   112,   218,   113,
      -1,   109,   110,   139,   111,   133,    -1,   230,    -1,   229,
     230,    -1,   231,    -1,   181,    -1,   314,    -1,   232,    -1,
      -1,   182,   202,   233,   216,    -1,    -1,   202,   234,   216,
      -1,   238,    -1,   245,    -1,   253,    -1,   260,    -1,   264,
      -1,   268,    -1,   279,    -1,   281,    -1,   287,    -1,   292,
      -1,   272,    -1,   286,    -1,   289,    -1,   284,    -1,   285,
      -1,   210,    -1,   239,   237,    -1,    37,    39,   240,   134,
      -1,    -1,   240,   241,    -1,   240,   117,   241,    -1,   242,
      -1,   295,    -1,    -1,    68,   110,   243,   179,   111,    -1,
      -1,    65,   110,   244,   179,   111,    -1,   246,   224,    -1,
      37,    67,   247,   134,    -1,    -1,   247,   248,    -1,   247,
     117,   248,    -1,   249,    -1,   295,    -1,    41,    -1,    42,
      -1,    43,   110,   252,   111,    -1,    -1,    43,   110,   252,
     117,   250,   179,   111,    -1,    -1,    43,   110,    50,   117,
     251,   179,   111,    -1,    72,   110,   179,   111,    -1,    44,
      -1,    45,    -1,    46,    -1,    47,    -1,    48,    -1,     1,
      -1,   254,   257,    -1,    37,    40,   255,   134,    -1,    -1,
     255,   256,    -1,   255,   117,   256,    -1,   295,    -1,    41,
      -1,   115,   258,   116,    -1,   237,    -1,   259,   237,    -1,
     258,   259,   237,    -1,    37,    49,   134,    -1,   261,   237,
      -1,    37,    51,   262,   134,    -1,    -1,   262,   263,    -1,
     262,   117,   263,    -1,   295,    -1,    41,    -1,   265,   224,
      -1,    37,    39,    67,   266,   134,    -1,    -1,   266,   267,
      -1,   266,   117,   267,    -1,   242,    -1,   249,    -1,   295,
      -1,   269,   257,    -1,    37,    39,    40,   270,   134,    -1,
      -1,   270,   271,    -1,   270,   117,   271,    -1,   242,    -1,
     295,    -1,   273,   237,    -1,    37,    69,   274,   134,    -1,
      -1,   274,   275,    -1,   274,   117,   275,    -1,   276,    -1,
     295,    -1,    -1,    68,   110,   277,   179,   111,    -1,    70,
      -1,    -1,    73,   110,   278,   179,   111,    -1,    74,    -1,
     280,   237,    -1,    37,    52,   134,    -1,   282,   237,    -1,
      37,    53,   134,    -1,    37,    53,   283,   134,    -1,   110,
       5,   111,    -1,    37,    71,   134,    -1,    37,    75,   134,
      -1,    37,    54,   134,    -1,   288,   220,    -1,    37,    55,
     134,    -1,    37,    55,    77,   134,    -1,    37,    55,    76,
     134,    -1,    37,    55,    79,   134,    -1,    37,    56,   134,
      -1,    37,    56,   290,   134,    -1,    -1,   110,   291,   304,
     111,    -1,   293,   237,    -1,    37,    42,   134,    -1,    38,
     110,   305,   111,   134,    -1,    -1,    57,   296,   110,   304,
     111,    -1,    -1,    66,   297,   110,   304,   111,    -1,    -1,
      58,   298,   110,   304,   111,    -1,    -1,    59,   299,   110,
     304,   111,    -1,    -1,    60,   300,   110,   304,   111,    -1,
      61,   110,    60,   111,    -1,    61,   110,    62,   111,    -1,
      -1,    63,   110,   303,   301,   131,   304,   111,    -1,    -1,
      64,   302,   110,   304,   111,    -1,   124,    -1,   121,    -1,
     118,    -1,   120,    -1,   127,    -1,   128,    -1,    19,    -1,
      20,    -1,    80,    -1,    81,    -1,     5,    -1,   304,   117,
       5,    -1,     5,    -1,   305,   117,     5,    -1,   307,    -1,
     308,    -1,    82,    87,   134,    -1,    -1,    82,    92,   309,
     310,   134,    -1,    -1,   310,   311,    -1,   310,   117,   311,
      -1,    93,   110,   177,   111,    -1,    94,   110,   177,   111,
      -1,    95,   110,   312,   111,    -1,   100,    -1,    70,    -1,
      96,    -1,    97,    -1,    97,   117,    99,    -1,    97,   117,
      98,    -1,   314,    -1,   322,    -1,    -1,   316,   232,   315,
     216,    -1,   316,   232,    -1,    -1,    82,    83,   317,   318,
     134,    -1,    -1,   318,   319,    -1,   318,   117,   319,    -1,
      84,   110,   320,   111,    -1,    85,   110,   320,   111,    -1,
      86,   110,   320,   111,    -1,    63,   110,   303,   131,   320,
     111,    -1,   321,    -1,   320,   117,   321,    -1,     5,    -1,
       5,   112,   130,     5,   113,    -1,     5,   112,   177,   113,
      -1,   323,   326,   133,    -1,    82,    69,   324,   134,    -1,
      -1,   324,   325,    -1,   324,   117,   325,    -1,    89,   110,
     121,   111,    -1,    89,   110,   177,   111,    -1,    91,   110,
     177,   111,    -1,   100,    -1,    70,    -1,    90,    -1,     5,
     110,   111,    -1,     5,   110,   141,   111,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   350,   350,   350,   355,   356,   380,   384,   407,   426,
     431,   435,   446,   450,   468,   476,   487,   491,   495,   499,
     503,   509,   515,   520,   525,   529,   538,   542,   550,   555,
     562,   569,   584,   602,   607,   613,   622,   626,   630,   634,
     641,   645,   649,   656,   666,   665,   715,   721,   720,   733,
     732,   745,   744,   760,   766,   765,   778,   777,   793,   799,
     798,   811,   810,   826,   832,   831,   845,   844,   858,   857,
     871,   870,   887,   893,   892,   906,   905,   921,   927,   926,
     941,   947,   946,   961,   967,   966,   981,   987,   986,  1001,
    1007,  1006,  1019,  1027,  1032,  1037,  1046,  1052,  1066,  1070,
    1074,  1078,  1082,  1086,  1094,  1100,  1108,  1122,  1132,  1140,
    1149,  1154,  1159,  1163,  1168,  1175,  1183,  1187,  1202,  1219,
    1218,  1244,  1248,  1252,  1256,  1260,  1267,  1271,  1279,  1283,
    1290,  1294,  1302,  1306,  1314,  1318,  1326,  1333,  1337,  1345,
    1349,  1353,  1374,  1381,  1388,  1400,  1404,  1408,  1412,  1419,
    1430,  1434,  1441,  1446,  1450,  1459,  1463,  1467,  1474,  1478,
    1482,  1486,  1490,  1494,  1498,  1502,  1506,  1510,  1514,  1522,
    1527,  1531,  1535,  1540,  1548,  1552,  1561,  1565,  1573,  1577,
    1581,  1585,  1589,  1598,  1602,  1612,  1616,  1620,  1628,  1632,
    1636,  1640,  1648,  1656,  1660,  1668,  1673,  1677,  1693,  1698,
    1703,  1708,  1713,  1718,  1723,  1729,  1735,  1740,  1745,  1752,
    1761,  1769,  1776,  1783,  1787,  1795,  1799,  1799,  1811,  1816,
    1826,  1830,  1835,  1840,  1849,  1853,  1863,  1869,  1880,  1898,
    1904,  1910,  1917,  1922,  1927,  1933,  1939,  1945,  1950,  1956,
    1963,  1969,  1976,  1982,  1992,  1996,  2000,  2004,  2018,  2046,
    2078,  2109,  2137,  2169,  2203,  2213,  2226,  2230,  2238,  2242,
    2249,  2260,  2265,  2264,  2375,  2374,  2448,  2452,  2456,  2460,
    2464,  2468,  2472,  2476,  2480,  2484,  2489,  2505,  2509,  2514,
    2519,  2526,  2533,  2541,  2549,  2552,  2556,  2563,  2567,  2574,
    2574,  2579,  2579,  2587,  2594,  2602,  2605,  2609,  2616,  2620,
    2624,  2631,  2635,  2639,  2639,  2647,  2646,  2658,  2673,  2677,
    2681,  2685,  2689,  2693,  2698,  2705,  2713,  2716,  2720,  2727,
    2731,  2738,  2745,  2750,  2754,  2761,  2768,  2775,  2783,  2786,
    2790,  2797,  2801,  2808,  2815,  2823,  2826,  2830,  2837,  2841,
    2845,  2852,  2859,  2867,  2870,  2874,  2881,  2885,  2893,  2902,
    2911,  2914,  2918,  2926,  2930,  2938,  2938,  2943,  2947,  2947,
    2952,  2958,  2965,  2972,  2979,  2983,  2990,  2998,  3006,  3013,
    3020,  3027,  3031,  3035,  3039,  3046,  3050,  3057,  3057,  3065,
    3072,  3079,  3086,  3086,  3092,  3092,  3097,  3097,  3102,  3102,
    3107,  3107,  3112,  3116,  3120,  3120,  3125,  3125,  3133,  3137,
    3141,  3145,  3149,  3153,  3157,  3161,  3165,  3169,  3176,  3183,
    3198,  3216,  3242,  3246,  3254,  3262,  3261,  3274,  3277,  3281,
    3288,  3292,  3296,  3300,  3304,  3311,  3315,  3319,  3323,  3330,
    3334,  3343,  3342,  3359,  3368,  3367,  3380,  3383,  3387,  3394,
    3398,  3402,  3406,  3413,  3417,  3424,  3429,  3437,  3445,  3453,
    3461,  3464,  3468,  3475,  3479,  3483,  3487,  3491,  3495,  3502,
    3508
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
  "SMCINPUT", "SMCOUTPUT", "PRAGMA_OMP", "PRAGMA_OMP_THREADPRIVATE",
  "OMP_PARALLEL", "OMP_SECTIONS", "OMP_NOWAIT", "OMP_ORDERED",
  "OMP_SCHEDULE", "OMP_STATIC", "OMP_DYNAMIC", "OMP_GUIDED", "OMP_RUNTIME",
  "OMP_AUTO", "OMP_SECTION", "OMP_AFFINITY", "OMP_SINGLE", "OMP_MASTER",
  "OMP_CRITICAL", "OMP_BARRIER", "OMP_ATOMIC", "OMP_FLUSH", "OMP_PRIVATE",
  "OMP_FIRSTPRIVATE", "OMP_LASTPRIVATE", "OMP_SHARED", "OMP_DEFAULT",
  "OMP_NONE", "OMP_REDUCTION", "OMP_COPYIN", "OMP_NUMTHREADS",
  "OMP_COPYPRIVATE", "OMP_FOR", "OMP_IF", "OMP_TASK", "OMP_UNTIED",
  "OMP_TASKWAIT", "OMP_COLLAPSE", "OMP_FINAL", "OMP_MERGEABLE",
  "OMP_TASKYIELD", "OMP_READ", "OMP_WRITE", "OMP_CAPTURE", "OMP_UPDATE",
  "OMP_MIN", "OMP_MAX", "PRAGMA_OMPIX", "OMPIX_TASKDEF", "OMPIX_IN",
  "OMPIX_OUT", "OMPIX_INOUT", "OMPIX_TASKSYNC", "OMPIX_UPONRETURN",
  "OMPIX_ATNODE", "OMPIX_DETACHED", "OMPIX_ATWORKER", "OMPIX_TASKSCHEDULE",
  "OMPIX_STRIDE", "OMPIX_START", "OMPIX_SCOPE", "OMPIX_NODES",
  "OMPIX_WORKERS", "OMPIX_LOCAL", "OMPIX_GLOBAL", "OMPIX_TIED", "FLOAT",
  "STRUCT", "UNION", "TYPEDEF", "PTR_OP", "TYPE_NAME", "SIZEOF", "PMALLOC",
  "PFREE", "'('", "')'", "'['", "']'", "'.'", "'{'", "'}'", "','", "'-'",
  "'!'", "'&'", "'*'", "'/'", "'%'", "'+'", "'<'", "'>'", "'^'", "'|'",
  "'@'", "'?'", "':'", "'='", "';'", "'\\n'", "$accept", "start_trick",
  "$@1", "string_literal", "primary_expression", "postfix_expression",
  "argument_expression_list", "unary_expression", "unary_operator",
  "type_name", "cast_expression", "$@2", "multiplicative_expression",
  "$@3", "$@4", "$@5", "additive_expression", "$@6", "$@7",
  "shift_expression", "$@8", "$@9", "relational_expression", "$@10",
  "$@11", "$@12", "$@13", "equality_expression", "$@14", "$@15",
  "AND_expression", "$@16", "exclusive_OR_expression", "$@17",
  "inclusive_OR_expression", "$@18", "logical_AND_expression", "$@19",
  "logical_OR_expression", "$@20", "DOT_product_expression",
  "conditional_expression", "assignment_expression", "assignment_operator",
  "expression", "constant_expression", "declaration",
  "declaration_specifiers", "typedef_name", "init_declarator_list",
  "init_declarator", "$@21", "type_specifier", "type_qualifier",
  "struct_or_union", "struct_declaration_list", "struct_declaration",
  "struct_declarator_list", "struct_declarator",
  "struct_or_union_specifier", "pointer", "parameter_type_list",
  "parameter_list", "parameter_declaration", "abstract_declarator",
  "direct_abstract_declarator", "specifier_qualifier_list", "declarator",
  "direct_declarator", "identifier_list", "initializer",
  "initializer_list", "designation", "designator_list", "designator",
  "statement", "parallel_statement", "increase_thread_id",
  "parallel_scope", "parallel_item", "parallel_sequence",
  "compound_statement", "@22", "block_item_list", "block_item",
  "expression_statement", "if_subroutine", "selection_statement",
  "iteration_statement", "iteration_statement_for", "jump_statement",
  "smc_statement", "batch_statement", "pfree_statement",
  "translation_unit", "external_declaration", "function_definition",
  "normal_function_definition", "$@23", "$@24", "openmp_construct",
  "openmp_directive", "structured_block", "parallel_construct",
  "parallel_directive", "parallel_clause_optseq", "parallel_clause",
  "unique_parallel_clause", "$@25", "$@26", "for_construct",
  "for_directive", "for_clause_optseq", "for_clause", "unique_for_clause",
  "$@27", "$@28", "schedule_kind", "sections_construct",
  "sections_directive", "sections_clause_optseq", "sections_clause",
  "section_scope", "section_sequence", "section_directive",
  "single_construct", "single_directive", "single_clause_optseq",
  "single_clause", "parallel_for_construct", "parallel_for_directive",
  "parallel_for_clause_optseq", "parallel_for_clause",
  "parallel_sections_construct", "parallel_sections_directive",
  "parallel_sections_clause_optseq", "parallel_sections_clause",
  "task_construct", "task_directive", "task_clause_optseq", "task_clause",
  "unique_task_clause", "$@29", "$@30", "master_construct",
  "master_directive", "critical_construct", "critical_directive",
  "region_phrase", "taskwait_directive", "taskyield_directive",
  "barrier_directive", "atomic_construct", "atomic_directive",
  "flush_directive", "flush_vars", "$@31", "ordered_construct",
  "ordered_directive", "threadprivate_directive", "data_clause", "$@32",
  "$@33", "$@34", "$@35", "$@36", "$@37", "$@38", "reduction_operator",
  "variable_list", "thrprv_variable_list", "ompix_directive",
  "ox_tasksync_directive", "ox_taskschedule_directive", "$@39",
  "ox_taskschedule_clause_optseq", "ox_taskschedule_clause",
  "ox_scope_spec", "ompix_construct", "ox_taskdef_construct", "$@40",
  "ox_taskdef_directive", "$@41", "ox_taskdef_clause_optseq",
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
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
      40,    41,    91,    93,    46,   123,   125,    44,    45,    33,
      38,    42,    47,    37,    43,    60,    62,    94,   124,    64,
      63,    58,    61,    59,    10
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   135,   137,   136,   136,   136,   138,   138,   139,   139,
     139,   139,   140,   140,   140,   140,   140,   140,   140,   140,
     140,   140,   140,   140,   140,   140,   141,   141,   142,   142,
     142,   142,   142,   142,   142,   142,   143,   143,   143,   143,
     144,   144,   144,   145,   146,   145,   147,   148,   147,   149,
     147,   150,   147,   151,   152,   151,   153,   151,   154,   155,
     154,   156,   154,   157,   158,   157,   159,   157,   160,   157,
     161,   157,   162,   163,   162,   164,   162,   165,   166,   165,
     167,   168,   167,   169,   170,   169,   171,   172,   171,   173,
     174,   173,   175,   176,   176,   176,   177,   177,   178,   178,
     178,   178,   178,   178,   179,   179,   180,   181,   181,   181,
     182,   182,   182,   182,   182,   183,   184,   184,   185,   186,
     185,   187,   187,   187,   187,   187,   188,   188,   189,   189,
     190,   190,   191,   191,   192,   192,   193,   193,   193,   194,
     194,   194,   194,   194,   194,   195,   195,   195,   195,   196,
     197,   197,   198,   198,   198,   199,   199,   199,   200,   200,
     200,   200,   200,   200,   200,   200,   200,   200,   200,   201,
     201,   201,   201,   201,   202,   202,   203,   203,   203,   203,
     203,   203,   203,   204,   204,   205,   205,   205,   206,   206,
     206,   206,   207,   208,   208,   209,   209,   209,   210,   210,
     210,   210,   210,   210,   210,   210,   210,   210,   210,   211,
     212,   213,   214,   215,   215,   216,   217,   216,   218,   218,
     219,   219,   219,   219,   220,   220,   221,   222,   222,   223,
     223,   223,   224,   224,   224,   224,   224,   224,   224,   224,
     224,   224,   224,   224,   225,   225,   225,   225,   226,   226,
     226,   226,   226,   226,   227,   228,   229,   229,   230,   230,
     230,   231,   233,   232,   234,   232,   235,   235,   235,   235,
     235,   235,   235,   235,   235,   235,   235,   236,   236,   236,
     236,   237,   238,   239,   240,   240,   240,   241,   241,   243,
     242,   244,   242,   245,   246,   247,   247,   247,   248,   248,
     248,   249,   249,   250,   249,   251,   249,   249,   252,   252,
     252,   252,   252,   252,   253,   254,   255,   255,   255,   256,
     256,   257,   258,   258,   258,   259,   260,   261,   262,   262,
     262,   263,   263,   264,   265,   266,   266,   266,   267,   267,
     267,   268,   269,   270,   270,   270,   271,   271,   272,   273,
     274,   274,   274,   275,   275,   277,   276,   276,   278,   276,
     276,   279,   280,   281,   282,   282,   283,   284,   285,   286,
     287,   288,   288,   288,   288,   289,   289,   291,   290,   292,
     293,   294,   296,   295,   297,   295,   298,   295,   299,   295,
     300,   295,   295,   295,   301,   295,   302,   295,   303,   303,
     303,   303,   303,   303,   303,   303,   303,   303,   304,   304,
     305,   305,   306,   306,   307,   309,   308,   310,   310,   310,
     311,   311,   311,   311,   311,   312,   312,   312,   312,   313,
     313,   315,   314,   314,   317,   316,   318,   318,   318,   319,
     319,   319,   319,   320,   320,   321,   321,   321,   322,   323,
     324,   324,   324,   325,   325,   325,   325,   325,   325,   326,
     326
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     2,     2,     1,     2,     1,     1,
       1,     3,     1,     4,     3,     4,     3,     4,     3,     3,
       2,     2,     3,     3,     6,     7,     1,     3,     1,     2,
       2,     2,     2,     2,     4,     6,     1,     1,     1,     1,
       1,     2,     1,     1,     0,     5,     1,     0,     4,     0,
       4,     0,     4,     1,     0,     4,     0,     4,     1,     0,
       4,     0,     4,     1,     0,     4,     0,     4,     0,     4,
       0,     4,     1,     0,     4,     0,     4,     1,     0,     4,
       1,     0,     4,     1,     0,     4,     1,     0,     4,     1,
       0,     4,     3,     1,     1,     5,     1,     3,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     2,     3,     1,
       1,     2,     1,     2,     1,     1,     1,     3,     1,     0,
       4,     1,     1,     1,     4,     6,     1,     1,     1,     1,
       1,     2,     3,     2,     1,     3,     1,     3,     2,     4,
       3,     5,     5,     2,     2,     1,     2,     2,     3,     1,
       1,     3,     2,     1,     2,     1,     1,     2,     3,     2,
       3,     3,     4,     3,     4,     2,     3,     3,     4,     1,
       2,     1,     2,     1,     1,     2,     1,     3,     3,     4,
       3,     4,     4,     1,     3,     1,     3,     4,     1,     2,
       3,     4,     2,     1,     2,     3,     2,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     1,     4,     1,     2,     2,     0,     4,     1,     2,
       1,     1,     1,     1,     1,     2,     0,     6,     8,     5,
       7,     1,     6,     7,     7,     7,     8,     8,     8,     9,
       6,     7,     7,     8,     2,     2,     2,     3,     7,     9,
      11,     7,     9,    11,    11,     5,     1,     2,     1,     1,
       1,     1,     0,     4,     0,     3,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     4,     0,     2,     3,     1,     1,     0,
       5,     0,     5,     2,     4,     0,     2,     3,     1,     1,
       1,     1,     4,     0,     7,     0,     7,     4,     1,     1,
       1,     1,     1,     1,     2,     4,     0,     2,     3,     1,
       1,     3,     1,     2,     3,     3,     2,     4,     0,     2,
       3,     1,     1,     2,     5,     0,     2,     3,     1,     1,
       1,     2,     5,     0,     2,     3,     1,     1,     2,     4,
       0,     2,     3,     1,     1,     0,     5,     1,     0,     5,
       1,     2,     3,     2,     3,     4,     3,     3,     3,     3,
       2,     3,     4,     4,     4,     3,     4,     0,     4,     2,
       3,     5,     0,     5,     0,     5,     0,     5,     0,     5,
       0,     5,     4,     4,     0,     7,     0,     5,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
       1,     3,     1,     1,     3,     0,     5,     0,     2,     3,
       4,     4,     4,     1,     1,     1,     1,     3,     3,     1,
       1,     0,     4,     2,     0,     5,     0,     2,     3,     4,
       4,     4,     6,     1,     3,     1,     5,     4,     3,     4,
       0,     2,     3,     4,     4,     4,     1,     1,     1,     3,
       4
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,   210,     0,     0,     8,     9,     6,     0,     0,
       0,     0,     0,    36,    37,    38,    39,    10,    12,    28,
      43,     0,    46,    53,    58,    63,    72,    77,    80,    83,
      86,    89,    93,    94,    96,   104,     4,   121,   123,     0,
     126,   127,     0,     0,   210,     0,     0,     0,     0,     0,
       0,     0,     0,   122,   128,   129,     0,   115,     0,   216,
     224,     0,   220,     0,   114,   110,     0,     0,   112,   221,
     208,     0,   209,   213,   211,   198,     5,   218,   199,   201,
     202,   231,   200,   203,   204,   205,   206,   222,   266,   210,
     267,     0,   268,     0,   269,   210,   270,     0,   271,     0,
     276,   210,   272,   210,   273,   210,   279,   280,   277,   274,
       0,   278,   275,   210,   109,   223,   412,   413,   207,   429,
       0,   430,     0,     1,   176,     0,     0,   145,   259,     0,
       0,   264,   174,     3,   256,   258,   261,   260,     0,     0,
      29,    30,     0,    33,     0,     0,     0,   169,   171,   173,
      40,     7,    20,    21,     0,     0,     0,     0,   102,   103,
      99,   100,   101,     0,    98,     0,    31,    32,    47,    49,
      51,    56,    54,    61,    59,    68,    70,    64,    66,    73,
      75,    78,    81,    84,    87,    90,     0,     0,     0,   246,
       0,     0,     0,     0,     0,     0,     0,   244,   245,     0,
       0,   284,   316,     0,   328,     0,     0,     0,     0,     0,
     295,   350,     0,     0,     0,   450,   434,     0,   415,     0,
     113,     0,   215,   210,   225,   107,     0,   116,   118,   111,
     143,     0,   144,   210,   214,   219,   281,   282,     0,   293,
     210,   314,   326,   333,   341,   348,   361,   363,   370,   379,
       0,   433,     0,     0,     0,   146,   147,   118,   175,     0,
       0,     0,   257,    14,     0,    26,     0,     0,     8,     0,
       0,    44,    11,   173,   170,   172,     0,     0,   155,    41,
     156,    23,    19,    16,     0,     0,    22,    18,    92,    97,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   105,
       0,   247,     0,     0,     0,     0,     0,     0,     0,     0,
     343,   335,     0,     0,   380,     0,   362,     0,   364,     0,
     369,     0,     0,     0,   371,   377,   375,     0,     0,     0,
     367,   368,   410,     0,     0,   436,   414,   417,     0,     0,
     210,     0,   108,   119,     0,   140,     0,   130,     0,     0,
     210,     0,     0,   322,     0,   210,   262,     0,     0,   448,
     177,   148,     0,   265,   183,   180,   153,     0,   149,   150,
       0,   178,     0,    15,     0,     0,    34,     0,     0,     0,
     165,     0,     0,   159,    39,     0,   157,     0,     0,    17,
      13,    43,    48,    50,    52,    57,    55,    62,    60,    69,
      71,    65,    67,    74,    76,    79,    82,    85,    88,    91,
       0,   124,   226,   210,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   382,   386,   388,   390,     0,     0,
     396,     0,   384,     0,     0,   283,   285,   287,   288,   320,
       0,   315,   317,   319,   332,     0,   327,   329,   331,     0,
     365,   373,   372,   374,     0,   376,   300,   301,     0,     0,
       0,   294,   296,   298,   299,     0,   357,     0,   360,     0,
     349,   351,   353,   354,     0,     0,   457,     0,   458,     0,
     456,     0,   449,   451,     0,     0,     0,     0,   217,   117,
       0,     0,   139,   131,     0,   133,     0,   134,   136,     0,
     212,     0,     0,     0,   321,   210,   323,   432,   459,     0,
     263,     0,   155,   154,   152,   182,     0,   181,     0,   179,
      27,     0,     0,     0,     0,   185,   188,     0,     0,     0,
     193,    45,   167,   158,   163,   161,   166,     0,   160,    39,
       0,    43,    95,   210,   229,     0,   210,     0,     0,     0,
       0,   210,     0,     0,     0,     0,     0,   342,   346,   344,
     347,     0,   334,   338,   339,   336,   340,     0,     0,     0,
       0,     0,     0,     0,   291,     0,   289,   286,   318,   330,
     366,   408,     0,     0,     0,   297,   355,   358,   352,   381,
     411,     0,     0,   452,     0,     0,     0,     0,     0,   435,
     437,   424,     0,     0,     0,   423,     0,   416,   418,     0,
     255,   120,   141,   106,   138,     0,   132,     0,   142,     0,
     325,   324,   460,   151,   184,    35,     0,   196,   197,     0,
      24,     0,   189,   192,   194,   168,   164,   162,   227,     0,
     232,   210,   210,     0,   210,     0,     0,   240,   210,   210,
       0,     0,     0,     0,     0,   345,   337,     0,     0,     0,
       0,     0,     0,   404,   405,   406,   407,   400,   401,   399,
     398,   402,   403,   394,     0,     0,     0,     0,   378,     0,
     313,   308,   309,   310,   311,   312,     0,     0,     0,     0,
       0,    39,     0,     0,     0,     0,     0,     0,   438,     0,
       0,     0,   419,   125,   135,   137,     0,   195,   186,     0,
      25,   190,     0,   210,   230,   235,   234,   210,   233,   210,
     210,     0,   242,   241,   210,   248,     0,   251,     0,     0,
       0,     0,     0,   392,   393,     0,     0,     0,     0,     0,
     409,   305,   302,   303,   307,     0,     0,   453,   454,   455,
       0,   445,     0,   443,     0,     0,     0,     0,   425,   426,
       0,     0,   187,   191,   228,   238,   237,   236,     0,   243,
       0,     0,     0,     0,   383,   387,   389,   391,     0,   397,
     292,   385,   290,     0,     0,   356,   359,     0,     0,   439,
       0,   440,   441,   420,   421,     0,   422,     0,   210,   239,
     249,   104,     0,   252,     0,     0,     0,     0,     0,     0,
       0,   444,   428,   427,   210,   210,     0,     0,   395,   306,
     304,   442,     0,   447,   254,   250,   253,   446
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     3,     4,    17,    18,    19,   264,    20,    21,   145,
      22,   389,    23,   290,   291,   292,    24,   294,   293,    25,
     296,   295,    26,   299,   300,   297,   298,    27,   301,   302,
      28,   303,    29,   304,    30,   305,    31,   306,    32,   307,
      33,    34,    35,   165,    61,   624,    62,    63,    64,   226,
     227,   500,    65,    66,    67,   356,   357,   506,   507,    68,
     130,   391,   378,   379,   392,   280,   358,   131,   132,   380,
     536,   537,   538,   539,   540,   236,    70,    71,    72,    73,
      74,    75,   223,    76,    77,    78,   553,    79,    80,    81,
      82,    83,    84,    85,   133,   134,   135,   136,   372,   259,
      86,    87,   237,    88,    89,   322,   446,   447,   687,   685,
      90,    91,   338,   472,   473,   794,   793,   697,    92,    93,
     323,   452,   241,   364,   365,    94,    95,   325,   457,    96,
      97,   433,   575,    98,    99,   432,   569,   100,   101,   339,
     481,   482,   699,   700,   102,   103,   104,   105,   329,   106,
     107,   108,   109,   110,   111,   337,   464,   112,   113,   114,
     448,   577,   585,   578,   579,   580,   745,   583,   683,   592,
     343,   115,   116,   117,   347,   495,   618,   770,   118,   119,
     367,   120,   345,   494,   610,   762,   763,   121,   122,   344,
     493,   253
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -643
static const yytype_int16 yypact[] =
{
      99,  1540,  1009,   128,  1332,   -34,  -643,  -643,  1582,  1582,
    1600,   -19,  1214,  -643,  -643,  -643,  -643,   125,  -643,   242,
      91,  1540,  -643,   334,   146,   255,    79,   365,    19,    60,
      49,   173,    20,  -643,  -643,  -643,    98,   110,  -643,   106,
    -643,  -643,   142,   174,  1160,   203,   185,   192,   218,   235,
    1860,   239,   240,   262,  -643,  -643,   380,  -643,   293,   290,
    -643,   -56,  -643,    76,  -643,  -643,    50,    31,  -643,  -643,
    -643,   302,  -643,  -643,   312,  -643,   706,  -643,  -643,  -643,
    -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  1160,
    -643,   399,  -643,   351,  -643,  1160,  -643,   399,  -643,   351,
    -643,  1160,  -643,  1160,  -643,  1160,  -643,  -643,  -643,  -643,
     329,  -643,  -643,  1160,  -643,  -643,  -643,  -643,  -643,  -643,
    1657,  -643,   468,  -643,  -643,   397,    32,    89,  -643,    76,
     124,  -643,   118,  1332,  -643,  -643,  -643,  -643,   463,  1214,
    -643,  -643,  1214,  -643,   102,   374,    10,   621,   621,   388,
     254,  -643,  -643,  -643,    27,  1061,  1540,    48,  -643,  -643,
    -643,  -643,  -643,  1582,  -643,  1540,  -643,  -643,  -643,  -643,
    -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,
    -643,  -643,  -643,  -643,  -643,  -643,  1540,  1540,   495,  -643,
     -29,  1540,  1540,   868,    15,   479,   385,  -643,  -643,   119,
     119,   -12,  -643,   396,  -643,   405,   -48,   408,   171,   -41,
    -643,  -643,   411,   421,   517,  -643,  -643,   430,  -643,   571,
    -643,   102,  -643,  1009,  -643,  -643,    62,  -643,   458,  -643,
     471,   589,   478,  1009,  -643,  -643,  -643,  -643,   485,  -643,
    1277,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,
      32,   487,   514,   472,   530,   533,  -643,   -49,   118,   552,
    1721,  1092,  -643,  -643,   199,  -643,   560,   564,  -643,  1540,
     559,   569,  -643,  -643,  -643,  -643,  1669,  1366,   162,  -643,
     261,  -643,  -643,  -643,   269,   152,  -643,  -643,  -643,  -643,
    1540,  1540,  1540,  1540,  1540,  1540,  1540,  1540,  1540,  1540,
    1540,  1540,  1540,  1540,  1540,  1540,  1540,  1540,   126,  -643,
     570,  -643,   284,   296,   590,   507,    85,   527,   494,   660,
    -643,  -643,   819,  1686,  -643,  1728,  -643,   721,  -643,   574,
    -643,   599,   614,   615,  -643,  -643,  -643,   617,  1741,  1806,
    -643,  -643,  -643,   308,  1726,  -643,  -643,  -643,   635,   645,
     823,    32,  -643,  -643,   621,  -643,   746,  -643,    75,   621,
     858,   385,   412,  -643,     6,  1160,  -643,   552,  1340,  -643,
    -643,  -643,   552,  -643,  -643,  -643,   130,   648,   661,  -643,
     311,  -643,   668,  -643,  1540,   569,   569,   621,   672,  1540,
    -643,   678,   683,  -643,   685,   690,   261,   891,  1394,  -643,
    -643,  -643,  -643,  -643,  -643,   334,   334,   146,   146,   255,
     255,   255,   255,    79,    79,   365,    19,    60,    49,   173,
    1540,  -643,  -643,  1160,  1540,  1398,   145,   562,  1449,   150,
     102,   102,  1824,  1776,  -643,  -643,  -643,  -643,   707,   709,
    -643,   710,  -643,   728,  1087,  -643,  -643,  -643,  -643,  -643,
    1912,  -643,  -643,  -643,  -643,  1923,  -643,  -643,  -643,   734,
    -643,  -643,  -643,  -643,   845,  -643,  -643,  -643,   742,   763,
    1891,  -643,  -643,  -643,  -643,   791,  -643,   803,  -643,  1933,
    -643,  -643,  -643,  -643,   740,   892,  -643,   804,  -643,   806,
    -643,   250,  -643,  -643,   326,   955,   912,   822,  -643,  -643,
    1464,  1019,  -643,  -643,  1540,  -643,   198,  -643,   832,  1569,
    -643,   213,   835,   921,  -643,  1160,  -643,  -643,  -643,   321,
    -643,  1318,   202,  -643,  -643,  -643,   698,  -643,   967,  -643,
    -643,   863,  1540,    70,   672,  -643,  -643,   328,  1464,   212,
    -643,  -643,  -643,  -643,  -643,  -643,  -643,   864,  -643,   886,
     888,   871,  -643,  1160,  -643,   342,  1160,   367,  1483,  1515,
     220,  1160,   379,  1520,   426,   433,  1087,  -643,  -643,  -643,
    -643,   703,  -643,  -643,  -643,  -643,  -643,   914,   919,   923,
     924,   355,   241,   925,  -643,   926,  -643,  -643,  -643,  -643,
    -643,  -643,   445,   796,  1540,  -643,  -643,  -643,  -643,  -643,
    -643,  1604,  1540,  -643,   928,   943,   951,   959,   431,  -643,
    -643,  -643,   960,   961,   963,  -643,   852,  -643,  -643,   954,
    -643,  -643,  -643,  -643,  -643,    80,  -643,  1540,  -643,   562,
    -643,  -643,  -643,  -643,  -643,  -643,   973,  -643,  -643,   381,
    -643,   944,  -643,  -643,  -643,  -643,  -643,  -643,  1058,   968,
    -643,  1160,  1160,   446,  1160,   455,  1535,  -643,  1160,  1160,
     474,   969,  1540,   970,  1540,  -643,  -643,   845,   845,   845,
     845,   981,   989,  -643,  -643,  -643,  -643,  -643,  -643,  -643,
    -643,  -643,  -643,  -643,   845,  1540,   845,  1540,  -643,  1099,
    -643,  -643,  -643,  -643,  -643,  -643,   990,   477,   492,  1540,
    1540,   998,  1003,  1012,   241,  1120,  1120,  1120,  -643,  1540,
    1540,   429,  -643,  -643,  -643,  -643,   234,  -643,  -643,  1248,
    -643,  -643,  1464,  1160,  -643,  -643,  -643,  1160,  -643,  1160,
    1160,   496,  -643,  -643,  1160,  -643,   499,  -643,   501,   525,
     544,   548,   577,  -643,  -643,   995,   582,   586,   587,   618,
    -643,  -643,  -643,  -643,  -643,   636,   694,  -643,  -643,  -643,
    1006,  1022,   764,  -643,   787,   795,  1027,  1030,  -643,  1026,
    1038,  1535,  -643,  -643,  -643,  -643,  -643,  -643,  1126,  -643,
    1031,  1540,  1042,  1540,  -643,  -643,  -643,  -643,   845,  -643,
    -643,  -643,  -643,  1540,  1540,  -643,  -643,  1120,   428,  -643,
    1120,  -643,  -643,  -643,  -643,   454,  -643,   798,  1009,  -643,
    -643,  -643,   800,  -643,   817,   837,   879,   887,   915,  1171,
    1064,  -643,  -643,  -643,  1160,   975,  1045,  1050,  -643,  -643,
    -643,  -643,  1071,  -643,  -643,  -643,  -643,  -643
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -643,  -643,  -643,  -643,  -124,   439,  -132,    -4,  -643,  -121,
     -10,  -643,   350,  -643,  -643,  -643,   435,  -643,  -643,   322,
    -643,  -643,   361,  -643,  -643,  -643,  -643,   882,  -643,  -643,
     883,  -643,   881,  -643,   895,  -643,   896,  -643,  -643,  -643,
    -643,  -394,  -130,  -643,     2,  -493,    23,     3,   -58,  -643,
     838,  -643,     1,    18,  -643,  -291,  -327,  -643,   573,    22,
    -103,  -229,  -643,   680,  -131,  -263,    33,   -47,  -120,  -643,
    -478,   670,  -589,  -643,   675,    -2,  -643,  -643,  -643,  1133,
    -643,  -205,  -643,  -221,   -75,  1105,  -643,  -643,  -643,   349,
    -643,  -643,  -643,  -643,  -643,  1076,  -643,  1096,  -643,  -643,
    -643,  -643,   -57,  -643,  -643,  -643,   773,  -382,  -643,  -643,
    -643,  -643,  -643,   748,  -400,  -643,  -643,  -643,  -643,  -643,
    -643,   774,  1130,  -643,   866,  -643,  -643,  -643,   776,  -643,
    -643,  -643,   666,  -643,  -643,  -643,   673,  -643,  -643,  -643,
     753,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,
    -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,  -643,
    -233,  -643,  -643,  -643,  -643,  -643,  -643,  -643,   536,  -597,
    -643,  -643,  -643,  -643,  -643,  -643,   627,  -643,  -643,    45,
    -643,  -643,  -643,  -643,   640,  -642,   449,  -643,  -643,  -643,
     759,  -643
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -432
static const yytype_int16 yytable[] =
{
      69,   235,   350,    36,   140,   141,   143,   129,   265,   232,
     258,   167,   360,   147,   146,   396,   228,   166,   266,   279,
     270,   267,   621,   284,   256,   265,   552,   128,   320,   503,
     148,   377,   281,   574,   149,   289,   230,   124,   242,   636,
     185,   190,   195,   513,   245,   150,   246,   278,   247,   137,
     568,   573,   722,   286,   373,   321,   249,   309,    37,    38,
     642,   187,   327,   501,   764,   765,  -262,   229,   509,   335,
     739,   740,   741,   742,    69,   637,   138,   224,   220,   254,
     124,   124,   257,   353,   215,   124,   328,   746,   187,   748,
     453,   144,   458,   336,   175,   176,   282,   349,   216,   287,
      40,    41,     1,     2,   311,   474,   483,   268,     6,     7,
     623,     5,     6,     7,   158,   159,   160,   161,   162,     8,
       9,   272,   514,   250,     5,     6,     7,   187,   123,   124,
     722,   382,   151,    57,   715,   124,   129,    57,   623,   181,
     147,   146,   126,   147,   146,   255,   231,   395,   147,   147,
     186,    53,   371,   127,    57,   818,   128,   148,   285,   288,
     148,   149,   517,   721,   149,   148,   148,   520,   547,   273,
     273,   574,   150,   278,   503,   150,    57,   183,   137,   351,
     274,   275,   503,   363,   568,   126,   126,   182,   308,   573,
     126,   815,   184,   312,   313,   352,   127,   127,   316,   570,
     576,   127,   187,   366,   177,   178,   504,   124,   505,   225,
     127,   504,   269,    10,    11,   187,    12,   453,   427,   317,
     163,    69,   458,   164,    13,    14,    15,    16,   260,   139,
     261,    69,   147,   623,   126,   188,   519,   474,   265,   189,
     521,   721,   277,   187,   773,   523,   483,   331,   332,   148,
     333,   127,   191,   273,   530,   152,   153,   420,   535,   396,
     673,   674,   187,   376,   171,   400,   531,   187,   550,   187,
     172,   146,   276,   522,   277,   235,   173,   174,   558,   376,
     402,   403,   404,   563,   192,   235,   401,   401,   401,   401,
     401,   401,   401,   401,   401,   401,   401,   401,   401,   401,
     401,   401,   401,   401,   228,   334,   564,   565,   516,   215,
     383,   508,   521,   196,   277,   625,   384,   426,   197,   429,
     486,   675,   676,   216,   532,   198,   533,   217,   199,   524,
     187,   626,   218,   570,     5,     6,     7,   187,   576,   487,
     488,   489,     8,     9,   643,   200,   629,   154,    69,   214,
     490,   187,   155,   656,   156,   147,   157,   147,    69,   677,
     147,   678,   679,   511,   276,   680,   277,   771,   681,   682,
     535,   397,   148,   398,   148,   127,   273,   148,   273,   541,
     399,   273,   179,   180,   317,   401,   384,   219,   147,   604,
       5,     6,     7,    37,    38,   422,    40,    41,     8,     9,
     376,   187,   258,   221,   535,   148,   222,   423,   535,   149,
     605,   606,   607,   187,   233,   671,   551,   672,   522,   484,
     150,   554,   527,    51,  -210,   485,   555,   557,   528,   560,
     562,   238,   632,     5,     6,     7,    10,    11,   384,    12,
     239,     8,     9,   608,   640,   641,   243,    13,    14,    15,
      16,   201,   202,   649,   203,   168,   169,   170,   631,   187,
     609,   512,    60,   204,   205,   206,   240,   208,     5,     6,
       7,   702,   703,   252,   254,   638,     8,     9,   651,   210,
     216,   211,    54,    55,   187,   271,    53,    54,    55,    56,
     658,    57,    10,    11,   604,    12,   187,   718,   719,   -42,
     551,   310,   147,    13,    14,    15,    16,   152,   153,   314,
     147,   535,     5,     6,     7,   605,   606,   607,   315,   148,
       8,     9,   342,   273,   376,   768,   769,   148,   551,   376,
     324,   273,     5,     6,     7,    10,    11,   661,    12,   326,
       8,     9,   330,   662,   663,   340,    13,    14,    15,    16,
     664,   648,   822,   823,   650,   341,   688,   727,   819,   657,
     653,   655,   689,   187,   346,   660,   729,     5,     6,     7,
      10,    11,   187,    12,   263,     8,     9,   348,   508,   766,
     767,    13,    14,    15,    16,   734,   354,   825,   752,   535,
     353,   187,   535,   359,   753,   361,   698,    37,    38,   154,
      40,    41,  -431,   754,   155,   369,   156,   778,   157,   187,
     780,   430,   782,   187,    10,    11,   781,    12,   783,   409,
     410,   411,   412,   551,   368,    13,    14,    15,    16,    37,
      38,   716,    40,    41,    10,    11,   784,    12,   318,   319,
     425,   370,   689,   405,   406,    13,    14,    15,    16,   725,
     726,   811,   728,   811,   127,   785,   732,   733,   731,   786,
     428,   689,   413,   414,   736,   689,   738,    59,   820,    10,
      11,   385,    12,   152,   153,   386,   387,     5,     6,     7,
      13,    14,    15,    16,   388,     8,     9,   747,   787,   749,
      53,    54,    55,   789,   689,   559,   421,   790,   791,   689,
     424,   755,   756,   187,   689,   355,    37,    38,   460,    40,
      41,     5,     6,     7,    37,    38,    39,    40,    41,     8,
       9,   774,    53,    54,    55,   775,   459,   776,   777,   792,
     407,   408,   779,   461,    42,   187,    43,    44,    45,    46,
      47,    48,    49,    50,    51,   467,   468,   795,   462,   463,
     235,   465,   496,   187,    37,    38,   497,    40,    41,   525,
     434,   435,   436,   437,   438,   154,   439,   440,   441,   442,
     155,   443,   156,   807,   157,   469,   809,   431,   526,    10,
      11,   529,    12,   812,   532,   814,   533,   534,    52,   542,
      13,    14,    15,    16,   543,   816,   817,   690,   544,    53,
      54,    55,    56,   545,    57,   796,    69,    53,    54,    55,
      56,   187,    57,    10,    11,    58,    12,   581,  -210,   582,
     584,    59,   809,    69,    13,    14,    15,    16,     5,     6,
       7,    37,    38,    39,    40,    41,     8,     9,   586,    60,
     691,   692,   693,   694,   695,   590,   696,    53,    54,    55,
     591,    42,   593,    43,    44,    45,    46,    47,    48,    49,
      50,    51,   502,     5,     6,     7,    37,    38,    39,    40,
      41,     8,     9,   594,   599,   799,   434,   435,   436,   437,
     438,   800,   439,   440,   441,   442,    42,   443,    43,    44,
      45,    46,    47,    48,    49,    50,    51,   600,   801,    37,
      38,   596,    40,    41,   800,    52,   802,   201,   202,   824,
     203,   826,   800,   597,   601,   187,   602,   187,   619,   204,
     205,   206,   611,   208,    53,    54,    55,    56,   827,    57,
      10,    11,    58,    12,   187,   210,   444,   211,    59,   498,
      52,    13,    14,    15,    16,   612,   613,   614,   828,     5,
       6,     7,   615,   445,   689,   620,    60,     8,     9,    53,
      54,    55,    56,   627,    57,    10,    11,    58,    12,   630,
     512,   510,   634,    59,   635,   645,    13,    14,    15,    16,
       5,     6,     7,    37,    38,    39,    40,    41,     8,     9,
     829,    60,    53,    54,    55,    56,   187,    57,   830,   646,
     163,   647,   546,    42,   187,    43,    44,    45,    46,    47,
      48,    49,    50,    51,     5,     6,     7,    37,    38,    39,
      40,    41,     8,     9,   667,   611,   831,    37,    38,   668,
      40,    41,   800,   669,   670,   684,   686,    42,   704,    43,
      44,    45,    46,    47,    48,    49,    50,    51,   612,   613,
     614,    10,    11,   705,    12,   615,   532,    52,   533,   534,
     720,   706,    13,    14,    15,    16,     5,     6,     7,   707,
     709,   710,   616,   711,     8,     9,    53,    54,    55,    56,
     713,    57,    10,    11,    58,    12,   717,   723,   834,   617,
      59,    52,   743,    13,    14,    15,    16,     5,     6,     7,
     744,   724,   735,   737,   750,     8,     9,   751,    60,   757,
      53,    54,    55,    56,   758,    57,    10,    11,    58,    12,
      53,    54,    55,   759,    59,   761,   788,    13,    14,    15,
      16,     5,     6,     7,   798,   622,    39,   797,   803,     8,
       9,   804,    60,   805,   434,   435,   436,   437,   438,   806,
     439,   440,   441,   442,    42,   443,    43,    44,    45,    46,
      47,    48,    49,   193,   810,     5,     6,     7,    10,    11,
      39,    12,   283,     8,     9,   813,   832,   833,   835,    13,
      14,    15,    16,   836,   837,   415,   417,   416,    42,   499,
      43,    44,    45,    46,    47,    48,    49,   193,   714,    10,
      11,   418,    12,   419,   639,   381,   633,   234,   194,   262,
      13,    14,    15,    16,   644,   248,   251,   587,   595,     5,
       6,     7,    37,    38,   588,    40,    41,     8,     9,   244,
     515,   589,   598,    10,    11,    58,    12,   666,   808,   665,
     760,    59,   194,   712,    13,    14,    15,    16,   708,   821,
     603,     0,     0,     5,     6,     7,     0,     0,     0,    60,
       0,     8,     9,     0,     0,     0,     0,    10,    11,    58,
      12,     0,     0,     0,     0,    59,     0,     0,    13,    14,
      15,    16,     5,     6,     7,     0,     0,    39,     0,     0,
       8,     9,     0,    60,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    42,     0,    43,    44,    45,
      46,    47,    48,    49,   362,    53,    54,    55,     0,     0,
       0,    10,    11,   124,    12,     0,    37,    38,     0,    40,
      41,     0,    13,    14,    15,    16,     0,   124,     0,     0,
      37,    38,     0,    40,    41,     5,     6,     7,     0,     0,
       0,     0,     0,     8,     9,    10,    11,     0,    12,   194,
     532,     0,   533,   534,   772,     0,    13,    14,    15,    16,
      51,     5,     6,     7,     0,     0,     0,     0,     0,     8,
       9,     0,     0,     0,    10,    11,    58,    12,     0,     0,
       0,     0,    59,     0,     0,    13,    14,    15,    16,     5,
       6,     7,     0,     5,     6,     7,     0,     8,     9,     0,
      60,     8,     9,     0,   125,     0,     0,     0,     0,    53,
      54,    55,    56,     0,    57,     0,     0,     0,   521,   390,
     277,     0,     0,    53,    54,    55,    56,     0,    57,   127,
       0,     0,   126,     0,     0,     0,     0,    10,    11,     0,
      12,   518,     0,   127,     5,     6,     7,     0,    13,    14,
      15,    16,     8,     9,     0,     0,     0,     0,     0,     5,
       6,     7,     0,    10,    11,     0,    12,     8,     9,   393,
       0,     0,     0,     0,    13,    14,    15,   394,     5,     6,
       7,     0,     0,     0,     0,     0,     8,     9,     0,     0,
       0,    10,    11,     0,    12,    10,    11,   548,    12,   556,
       0,     0,    13,    14,    15,   549,    13,    14,    15,    16,
       5,     6,     7,     0,     0,     5,     6,     7,     8,     9,
       0,     0,     0,     8,     9,     0,     0,     0,     0,     0,
       5,     6,     7,     0,     0,     5,     6,     7,     8,     9,
       0,     0,     0,     8,     9,     0,    10,    11,     0,    12,
     561,     0,     0,     0,     0,     0,     0,    13,    14,    15,
      16,    10,    11,     0,    12,     0,     0,    37,    38,   534,
      40,    41,    13,    14,    15,    16,     0,     5,     6,     7,
      10,    11,     0,    12,   652,     8,     9,     0,     0,     0,
       0,    13,    14,    15,    16,     5,     6,     7,     0,     5,
       6,     7,     0,     8,     9,     0,     0,     8,     9,     0,
       0,     0,    10,    11,     0,    12,   654,    10,    11,     0,
      12,   659,     0,    13,    14,    15,    16,     0,    13,    14,
      15,    16,    10,    11,     0,    12,   730,    10,    11,     0,
      12,     0,     0,    13,    14,    15,    16,     0,    13,    14,
      15,    16,   124,     0,     0,    37,    38,     0,    40,    41,
      53,    54,    55,     0,     0,     0,     0,    37,    38,     0,
      40,    41,     0,     0,     0,   628,     0,     0,     0,    10,
      11,     0,   139,     0,     0,     0,     0,     0,     0,     0,
      13,    14,    15,    16,     0,     0,     0,    10,    11,     0,
     142,    10,    11,     0,    12,     0,     0,     0,    13,    14,
      15,    16,    13,    14,    15,   701,   374,   449,     0,    37,
      38,     0,    40,    41,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   434,   435,   436,   437,   438,     0,   439,
     440,     0,   442,     0,     0,     0,     0,     0,    53,    54,
      55,    56,     0,    57,     0,     0,     0,   126,     0,   454,
      53,    54,    55,    56,     0,    57,     0,     0,   127,   276,
     390,   277,   466,   467,   468,   434,   435,   436,   437,   438,
     127,   439,   440,     0,   442,     0,   486,     0,   434,   435,
     436,   437,   438,   450,   439,   440,     0,   442,     0,     0,
       0,     0,     0,   469,     0,   487,   488,   489,   467,   468,
     451,     0,    53,    54,    55,    56,   490,    57,     0,     0,
       0,     0,   375,   434,   435,   436,   437,   438,     0,   439,
     440,   441,   442,   491,   443,   455,     0,     0,   469,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   470,     0,
     492,     0,   456,   434,   435,   436,   437,   438,     0,   439,
     440,     0,   442,     0,   475,   471,   476,     0,     0,   477,
     478,   434,   435,   436,   437,   438,     0,   439,   440,   441,
     442,     0,   443,   571,     0,     0,     0,     0,     0,   201,
     202,     0,   203,     0,     0,     0,     0,     0,     0,     0,
     572,   204,   205,   206,   207,   208,   209,     0,     0,     0,
       0,     0,     0,   479,     0,     0,     0,   210,     0,   211,
       0,   212,   466,   467,   468,   213,     0,     0,     0,     0,
     480,   566,     0,     0,     0,     0,     0,     0,   434,   435,
     436,   437,   438,   449,   439,   440,     0,   442,   567,     0,
       0,     0,     0,   469,   454,     0,     0,     0,     0,   434,
     435,   436,   437,   438,     0,   439,   440,     0,   442,     0,
     434,   435,   436,   437,   438,     0,   439,   440,     0,   442,
     434,   435,   436,   437,   438,     0,   439,   440,     0,   442,
       0,   475,     0,   476,     0,     0,   477,   478
};

static const yytype_int16 yycheck[] =
{
       2,    76,   223,     1,     8,     9,    10,     4,   138,    67,
     130,    21,   233,    12,    12,   278,    63,    21,   139,   150,
     144,   142,   500,   155,   127,   155,   420,     4,    40,   356,
      12,   260,     5,   433,    12,   165,     5,     5,    95,   532,
      20,    39,    44,    37,   101,    12,   103,   150,   105,     4,
     432,   433,   641,     5,   259,    67,   113,   187,     8,     9,
     538,   117,   110,   354,   706,   707,   115,    66,   359,   110,
     667,   668,   669,   670,    76,     5,   110,   133,    56,   126,
       5,     5,   129,   132,    69,     5,   134,   684,   117,   686,
     323,   110,   325,   134,    15,    16,   154,   221,    83,   157,
      11,    12,     3,     4,   133,   338,   339,     5,     6,     7,
     504,     5,     6,     7,    23,    24,    25,    26,    27,    13,
      14,   111,   116,   120,     5,     6,     7,   117,     0,     5,
     719,   261,     7,   106,   627,     5,   133,   106,   532,   120,
     139,   139,   110,   142,   142,   127,   115,   277,   147,   148,
     130,   101,   255,   121,   106,   797,   133,   139,   156,   163,
     142,   139,   367,   641,   142,   147,   148,   372,   397,   147,
     148,   571,   139,   276,   501,   142,   106,   128,   133,   117,
     147,   148,   509,   240,   566,   110,   110,   127,   186,   571,
     110,   788,    19,   191,   192,   133,   121,   121,   196,   432,
     433,   121,   117,   250,   125,   126,   131,     5,   133,   133,
     121,   131,   110,   107,   108,   117,   110,   450,   133,   196,
     129,   223,   455,   132,   118,   119,   120,   121,   110,   110,
     112,   233,   231,   627,   110,   125,   368,   470,   368,   133,
     110,   719,   112,   117,   722,   376,   479,    76,    77,   231,
      79,   121,   110,   231,   384,    13,    14,   131,   388,   522,
      19,    20,   117,   260,   118,   113,   387,   117,   398,   117,
     124,   269,   110,   376,   112,   350,    21,    22,   133,   276,
     290,   291,   292,   133,   110,   360,   290,   291,   292,   293,
     294,   295,   296,   297,   298,   299,   300,   301,   302,   303,
     304,   305,   306,   307,   351,   134,   430,   431,   365,    69,
     111,   358,   110,   110,   112,   117,   117,   315,   133,   317,
      70,    80,    81,    83,   112,   133,   114,    87,   110,   376,
     117,   133,    92,   566,     5,     6,     7,   117,   571,    89,
      90,    91,    13,    14,   132,   110,   133,   105,   350,   110,
     100,   117,   110,   133,   112,   354,   114,   356,   360,   118,
     359,   120,   121,   361,   110,   124,   112,   133,   127,   128,
     500,   110,   354,   112,   356,   121,   354,   359,   356,   389,
     111,   359,    17,    18,   361,   389,   117,   125,   387,    63,
       5,     6,     7,     8,     9,   111,    11,    12,    13,    14,
     397,   117,   522,   110,   534,   387,   116,   111,   538,   387,
      84,    85,    86,   117,   112,    60,   420,    62,   521,   111,
     387,   423,   111,    38,   112,   117,   424,   425,   117,   427,
     428,    32,   111,     5,     6,     7,   107,   108,   117,   110,
      91,    13,    14,   117,   116,   117,    97,   118,   119,   120,
     121,    39,    40,   111,    42,   121,   122,   123,   515,   117,
     134,    49,   133,    51,    52,    53,   115,    55,     5,     6,
       7,   601,   602,     5,   521,   533,    13,    14,   111,    67,
      83,    69,   102,   103,   117,   111,   101,   102,   103,   104,
     111,   106,   107,   108,    63,   110,   117,   116,   117,   111,
     504,     6,   501,   118,   119,   120,   121,    13,    14,    30,
     509,   641,     5,     6,     7,    84,    85,    86,   133,   501,
      13,    14,     5,   501,   521,    96,    97,   509,   532,   526,
     134,   509,     5,     6,     7,   107,   108,   111,   110,   134,
      13,    14,   134,   117,   111,   134,   118,   119,   120,   121,
     117,   553,    98,    99,   556,   134,   111,   111,   130,   561,
     558,   559,   117,   117,   134,   563,   111,     5,     6,     7,
     107,   108,   117,   110,   111,    13,    14,     6,   625,   709,
     710,   118,   119,   120,   121,   111,   115,   808,   111,   719,
     132,   117,   722,   115,   117,   110,   594,     8,     9,   105,
      11,    12,   115,   111,   110,   133,   112,   111,   114,   117,
     111,   117,   111,   117,   107,   108,   117,   110,   117,   297,
     298,   299,   300,   627,   110,   118,   119,   120,   121,     8,
       9,   629,    11,    12,   107,   108,   111,   110,   199,   200,
     133,   111,   117,   293,   294,   118,   119,   120,   121,   651,
     652,   781,   654,   783,   121,   111,   658,   659,   656,   111,
     133,   117,   301,   302,   662,   117,   664,   115,   798,   107,
     108,   111,   110,    13,    14,   111,   117,     5,     6,     7,
     118,   119,   120,   121,   115,    13,    14,   685,   111,   687,
     101,   102,   103,   111,   117,   133,   126,   111,   111,   117,
     110,   699,   700,   117,   117,   116,     8,     9,   134,    11,
      12,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,   723,   101,   102,   103,   727,     5,   729,   730,   111,
     295,   296,   734,   134,    28,   117,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    42,    43,   111,   134,   134,
     825,   134,   117,   117,     8,     9,   111,    11,    12,   111,
      57,    58,    59,    60,    61,   105,    63,    64,    65,    66,
     110,    68,   112,   771,   114,    72,   778,   117,   117,   107,
     108,   113,   110,   781,   112,   783,   114,   115,    82,   111,
     118,   119,   120,   121,   111,   793,   794,     1,   113,   101,
     102,   103,   104,   113,   106,   111,   808,   101,   102,   103,
     104,   117,   106,   107,   108,   109,   110,   110,   112,   110,
     110,   115,   824,   825,   118,   119,   120,   121,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,   110,   133,
      44,    45,    46,    47,    48,   111,    50,   101,   102,   103,
       5,    28,   110,    30,    31,    32,    33,    34,    35,    36,
      37,    38,   116,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,   110,   134,   111,    57,    58,    59,    60,
      61,   117,    63,    64,    65,    66,    28,    68,    30,    31,
      32,    33,    34,    35,    36,    37,    38,     5,   111,     8,
       9,   110,    11,    12,   117,    82,   111,    39,    40,   111,
      42,   111,   117,   110,   110,   117,   110,   117,     6,    51,
      52,    53,    70,    55,   101,   102,   103,   104,   111,   106,
     107,   108,   109,   110,   117,    67,   117,    69,   115,   116,
      82,   118,   119,   120,   121,    93,    94,    95,   111,     5,
       6,     7,   100,   134,   117,   133,   133,    13,    14,   101,
     102,   103,   104,   131,   106,   107,   108,   109,   110,   134,
      49,   113,     5,   115,   111,   111,   118,   119,   120,   121,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
     111,   133,   101,   102,   103,   104,   117,   106,   111,   113,
     129,   113,   111,    28,   117,    30,    31,    32,    33,    34,
      35,    36,    37,    38,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,   110,    70,   111,     8,     9,   110,
      11,    12,   117,   110,   110,   110,   110,    28,   110,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    93,    94,
      95,   107,   108,   110,   110,   100,   112,    82,   114,   115,
     116,   110,   118,   119,   120,   121,     5,     6,     7,   110,
     110,   110,   117,   110,    13,    14,   101,   102,   103,   104,
     126,   106,   107,   108,   109,   110,   113,    29,   113,   134,
     115,    82,   111,   118,   119,   120,   121,     5,     6,     7,
     111,   133,   133,   133,     5,    13,    14,   117,   133,   111,
     101,   102,   103,   104,   111,   106,   107,   108,   109,   110,
     101,   102,   103,   111,   115,     5,   131,   118,   119,   120,
     121,     5,     6,     7,   112,   116,    10,   131,   111,    13,
      14,   111,   133,   117,    57,    58,    59,    60,    61,   111,
      63,    64,    65,    66,    28,    68,    30,    31,    32,    33,
      34,    35,    36,    37,   133,     5,     6,     7,   107,   108,
      10,   110,   111,    13,    14,   133,     5,   113,   133,   118,
     119,   120,   121,   133,   113,   303,   305,   304,    28,   351,
      30,    31,    32,    33,    34,    35,    36,    37,   625,   107,
     108,   306,   110,   307,   534,   113,   526,    74,    82,   133,
     118,   119,   120,   121,   539,   110,   120,   444,   470,     5,
       6,     7,     8,     9,   450,    11,    12,    13,    14,    99,
     364,   455,   479,   107,   108,   109,   110,   571,   112,   566,
     704,   115,    82,   616,   118,   119,   120,   121,   608,   800,
     491,    -1,    -1,     5,     6,     7,    -1,    -1,    -1,   133,
      -1,    13,    14,    -1,    -1,    -1,    -1,   107,   108,   109,
     110,    -1,    -1,    -1,    -1,   115,    -1,    -1,   118,   119,
     120,   121,     5,     6,     7,    -1,    -1,    10,    -1,    -1,
      13,    14,    -1,   133,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    -1,    30,    31,    32,
      33,    34,    35,    36,    37,   101,   102,   103,    -1,    -1,
      -1,   107,   108,     5,   110,    -1,     8,     9,    -1,    11,
      12,    -1,   118,   119,   120,   121,    -1,     5,    -1,    -1,
       8,     9,    -1,    11,    12,     5,     6,     7,    -1,    -1,
      -1,    -1,    -1,    13,    14,   107,   108,    -1,   110,    82,
     112,    -1,   114,   115,   116,    -1,   118,   119,   120,   121,
      38,     5,     6,     7,    -1,    -1,    -1,    -1,    -1,    13,
      14,    -1,    -1,    -1,   107,   108,   109,   110,    -1,    -1,
      -1,    -1,   115,    -1,    -1,   118,   119,   120,   121,     5,
       6,     7,    -1,     5,     6,     7,    -1,    13,    14,    -1,
     133,    13,    14,    -1,    82,    -1,    -1,    -1,    -1,   101,
     102,   103,   104,    -1,   106,    -1,    -1,    -1,   110,   111,
     112,    -1,    -1,   101,   102,   103,   104,    -1,   106,   121,
      -1,    -1,   110,    -1,    -1,    -1,    -1,   107,   108,    -1,
     110,   111,    -1,   121,     5,     6,     7,    -1,   118,   119,
     120,   121,    13,    14,    -1,    -1,    -1,    -1,    -1,     5,
       6,     7,    -1,   107,   108,    -1,   110,    13,    14,   113,
      -1,    -1,    -1,    -1,   118,   119,   120,   121,     5,     6,
       7,    -1,    -1,    -1,    -1,    -1,    13,    14,    -1,    -1,
      -1,   107,   108,    -1,   110,   107,   108,   113,   110,   111,
      -1,    -1,   118,   119,   120,   121,   118,   119,   120,   121,
       5,     6,     7,    -1,    -1,     5,     6,     7,    13,    14,
      -1,    -1,    -1,    13,    14,    -1,    -1,    -1,    -1,    -1,
       5,     6,     7,    -1,    -1,     5,     6,     7,    13,    14,
      -1,    -1,    -1,    13,    14,    -1,   107,   108,    -1,   110,
     111,    -1,    -1,    -1,    -1,    -1,    -1,   118,   119,   120,
     121,   107,   108,    -1,   110,    -1,    -1,     8,     9,   115,
      11,    12,   118,   119,   120,   121,    -1,     5,     6,     7,
     107,   108,    -1,   110,   111,    13,    14,    -1,    -1,    -1,
      -1,   118,   119,   120,   121,     5,     6,     7,    -1,     5,
       6,     7,    -1,    13,    14,    -1,    -1,    13,    14,    -1,
      -1,    -1,   107,   108,    -1,   110,   111,   107,   108,    -1,
     110,   111,    -1,   118,   119,   120,   121,    -1,   118,   119,
     120,   121,   107,   108,    -1,   110,   111,   107,   108,    -1,
     110,    -1,    -1,   118,   119,   120,   121,    -1,   118,   119,
     120,   121,     5,    -1,    -1,     8,     9,    -1,    11,    12,
     101,   102,   103,    -1,    -1,    -1,    -1,     8,     9,    -1,
      11,    12,    -1,    -1,    -1,   116,    -1,    -1,    -1,   107,
     108,    -1,   110,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     118,   119,   120,   121,    -1,    -1,    -1,   107,   108,    -1,
     110,   107,   108,    -1,   110,    -1,    -1,    -1,   118,   119,
     120,   121,   118,   119,   120,   121,     5,    41,    -1,     8,
       9,    -1,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    57,    58,    59,    60,    61,    -1,    63,
      64,    -1,    66,    -1,    -1,    -1,    -1,    -1,   101,   102,
     103,   104,    -1,   106,    -1,    -1,    -1,   110,    -1,    41,
     101,   102,   103,   104,    -1,   106,    -1,    -1,   121,   110,
     111,   112,    41,    42,    43,    57,    58,    59,    60,    61,
     121,    63,    64,    -1,    66,    -1,    70,    -1,    57,    58,
      59,    60,    61,   117,    63,    64,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    72,    -1,    89,    90,    91,    42,    43,
     134,    -1,   101,   102,   103,   104,   100,   106,    -1,    -1,
      -1,    -1,   111,    57,    58,    59,    60,    61,    -1,    63,
      64,    65,    66,   117,    68,   117,    -1,    -1,    72,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   117,    -1,
     134,    -1,   134,    57,    58,    59,    60,    61,    -1,    63,
      64,    -1,    66,    -1,    68,   134,    70,    -1,    -1,    73,
      74,    57,    58,    59,    60,    61,    -1,    63,    64,    65,
      66,    -1,    68,   117,    -1,    -1,    -1,    -1,    -1,    39,
      40,    -1,    42,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     134,    51,    52,    53,    54,    55,    56,    -1,    -1,    -1,
      -1,    -1,    -1,   117,    -1,    -1,    -1,    67,    -1,    69,
      -1,    71,    41,    42,    43,    75,    -1,    -1,    -1,    -1,
     134,   117,    -1,    -1,    -1,    -1,    -1,    -1,    57,    58,
      59,    60,    61,    41,    63,    64,    -1,    66,   134,    -1,
      -1,    -1,    -1,    72,    41,    -1,    -1,    -1,    -1,    57,
      58,    59,    60,    61,    -1,    63,    64,    -1,    66,    -1,
      57,    58,    59,    60,    61,    -1,    63,    64,    -1,    66,
      57,    58,    59,    60,    61,    -1,    63,    64,    -1,    66,
      -1,    68,    -1,    70,    -1,    -1,    73,    74
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,     3,     4,   136,   137,     5,     6,     7,    13,    14,
     107,   108,   110,   118,   119,   120,   121,   138,   139,   140,
     142,   143,   145,   147,   151,   154,   157,   162,   165,   167,
     169,   171,   173,   175,   176,   177,   179,     8,     9,    10,
      11,    12,    28,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    82,   101,   102,   103,   104,   106,   109,   115,
     133,   179,   181,   182,   183,   187,   188,   189,   194,   210,
     211,   212,   213,   214,   215,   216,   218,   219,   220,   222,
     223,   224,   225,   226,   227,   228,   235,   236,   238,   239,
     245,   246,   253,   254,   260,   261,   264,   265,   268,   269,
     272,   273,   279,   280,   281,   282,   284,   285,   286,   287,
     288,   289,   292,   293,   294,   306,   307,   308,   313,   314,
     316,   322,   323,     0,     5,    82,   110,   121,   181,   182,
     195,   202,   203,   229,   230,   231,   232,   314,   110,   110,
     142,   142,   110,   142,   110,   144,   179,   187,   188,   194,
     201,     7,    13,    14,   105,   110,   112,   114,    23,    24,
      25,    26,    27,   129,   132,   178,   142,   145,   121,   122,
     123,   118,   124,    21,    22,    15,    16,   125,   126,    17,
      18,   120,   127,   128,    19,    20,   130,   117,   125,   133,
     179,   110,   110,    37,    82,   210,   110,   133,   133,   110,
     110,    39,    40,    42,    51,    52,    53,    54,    55,    56,
      67,    69,    71,    75,   110,    69,    83,    87,    92,   125,
     194,   110,   116,   217,   133,   133,   184,   185,   202,   187,
       5,   115,   183,   112,   214,   219,   210,   237,    32,   224,
     115,   257,   237,   224,   257,   237,   237,   237,   220,   237,
     182,   232,     5,   326,   202,   188,   195,   202,   203,   234,
     110,   112,   230,   111,   141,   177,   144,   144,     5,   110,
     139,   111,   111,   194,   201,   201,   110,   112,   195,   199,
     200,     5,   183,   111,   141,   179,     5,   183,   142,   177,
     148,   149,   150,   153,   152,   156,   155,   160,   161,   158,
     159,   163,   164,   166,   168,   170,   172,   174,   179,   177,
       6,   133,   179,   179,    30,   133,   179,   181,   140,   140,
      40,    67,   240,   255,   134,   262,   134,   110,   134,   283,
     134,    76,    77,    79,   134,   110,   134,   290,   247,   274,
     134,   134,     5,   305,   324,   317,   134,   309,     6,   139,
     218,   117,   133,   132,   115,   116,   190,   191,   201,   115,
     218,   110,    37,   237,   258,   259,   202,   315,   110,   133,
     111,   195,   233,   216,     5,   111,   182,   196,   197,   198,
     204,   113,   177,   111,   117,   111,   111,   117,   115,   146,
     111,   196,   199,   113,   121,   177,   200,   110,   112,   111,
     113,   142,   145,   145,   145,   147,   147,   151,   151,   154,
     154,   154,   154,   157,   157,   162,   165,   167,   169,   171,
     131,   126,   111,   111,   110,   133,   179,   133,   133,   179,
     117,   117,   270,   266,    57,    58,    59,    60,    61,    63,
      64,    65,    66,    68,   117,   134,   241,   242,   295,    41,
     117,   134,   256,   295,    41,   117,   134,   263,   295,     5,
     134,   134,   134,   134,   291,   134,    41,    42,    43,    72,
     117,   134,   248,   249,   295,    68,    70,    73,    74,   117,
     134,   275,   276,   295,   111,   117,    70,    89,    90,    91,
     100,   117,   134,   325,   318,   310,   117,   111,   116,   185,
     186,   190,   116,   191,   131,   133,   192,   193,   202,   190,
     113,   179,    49,    37,   116,   259,   237,   216,   111,   141,
     216,   110,   195,   199,   202,   111,   117,   111,   117,   113,
     177,   144,   112,   114,   115,   177,   205,   206,   207,   208,
     209,   145,   111,   111,   113,   113,   111,   196,   113,   121,
     177,   142,   176,   221,   210,   179,   111,   179,   133,   133,
     179,   111,   179,   133,   139,   139,   117,   134,   242,   271,
     295,   117,   134,   242,   249,   267,   295,   296,   298,   299,
     300,   110,   110,   302,   110,   297,   110,   241,   256,   263,
     111,     5,   304,   110,   110,   248,   110,   110,   275,   134,
       5,   110,   110,   325,    63,    84,    85,    86,   117,   134,
     319,    70,    93,    94,    95,   100,   117,   134,   311,     6,
     133,   205,   116,   176,   180,   117,   133,   131,   116,   133,
     134,   237,   111,   198,     5,   111,   180,     5,   183,   206,
     116,   117,   205,   132,   209,   111,   113,   113,   210,   111,
     210,   111,   111,   179,   111,   179,   133,   210,   111,   111,
     179,   111,   117,   111,   117,   271,   267,   110,   110,   110,
     110,    60,    62,    19,    20,    80,    81,   118,   120,   121,
     124,   127,   128,   303,   110,   244,   110,   243,   111,   117,
       1,    44,    45,    46,    47,    48,    50,   252,   179,   277,
     278,   121,   177,   177,   110,   110,   110,   110,   319,   110,
     110,   110,   311,   126,   193,   180,   179,   113,   116,   117,
     116,   205,   207,    29,   133,   210,   210,   111,   210,   111,
     111,   179,   210,   210,   111,   133,   179,   133,   179,   304,
     304,   304,   304,   111,   111,   301,   304,   179,   304,   179,
       5,   117,   111,   117,   111,   179,   179,   111,   111,   111,
     303,     5,   320,   321,   320,   320,   177,   177,    96,    97,
     312,   133,   116,   205,   210,   210,   210,   210,   111,   210,
     111,   117,   111,   117,   111,   111,   111,   111,   131,   111,
     111,   111,   111,   251,   250,   111,   111,   131,   112,   111,
     117,   111,   111,   111,   111,   117,   111,   179,   112,   210,
     133,   177,   179,   133,   179,   304,   179,   179,   320,   130,
     177,   321,    98,    99,   111,   218,   111,   111,   111,   111,
     111,   111,     5,   113,   113,   133,   133,   113
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
# if YYLTYPE_IS_TRIVIAL
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
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
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
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
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


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

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
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

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
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
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

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
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

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
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

/* Line 1455 of yacc.c  */
#line 350 "parser.y"
    {
    	var_file = fopen(var_list, "w+");
	struct_table = struct_node_stack_new(); 
    ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 354 "parser.y"
    {;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 355 "parser.y"
    { pastree_expr = (yyvsp[(2) - (2)].expr); ;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 356 "parser.y"
    { pastree_stmt = (yyvsp[(2) - (2)].stmt); ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 381 "parser.y"
    {
      (yyval.string) = strdup((yyvsp[(1) - (1)].name));
    ;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 385 "parser.y"
    {
      /* Or we could leave it as is (as a SpaceList) */
      if (((yyvsp[(1) - (2)].string) = realloc((yyvsp[(1) - (2)].string), strlen((yyvsp[(1) - (2)].string)) + strlen((yyvsp[(2) - (2)].name)))) == NULL)
        parse_error(-1, "string out of memory\n");
      strcpy(((yyvsp[(1) - (2)].string))+(strlen((yyvsp[(1) - (2)].string))-1),((yyvsp[(2) - (2)].name))+1);  /* Catenate on the '"' */
      (yyval.string) = (yyvsp[(1) - (2)].string);
    ;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 408 "parser.y"
    {
      symbol id = Symbol((yyvsp[(1) - (1)].name)); 
      stentry e;
      int     chflag = 0;
    
      if (checkDecls)
      {
        check_uknown_var((yyvsp[(1) - (1)].name));
        
        if ((e = symtab_get(stab, id, IDNAME)) != NULL) /* could be enum name */
          if (istp(e) && threadmode)
            chflag = 1;
      }
      (yyval.expr) = chflag ? UnaryOperator(UOP_paren,
                             UnaryOperator(UOP_star, Identifier(id)))
                  : Identifier(id);
      set_identifier_attributes(id, (yyval.expr), 0); 	
    ;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 427 "parser.y"
    {
      (yyval.expr) = Constant( strdup((yyvsp[(1) - (1)].name)) );
      set_security_flag_expr((yyval.expr), NULL, NULL, -1);
    ;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 432 "parser.y"
    {
      (yyval.expr) = String((yyvsp[(1) - (1)].string));
    ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 436 "parser.y"
    {
      (yyval.expr) = UnaryOperator(UOP_paren, (yyvsp[(2) - (3)].expr));
      (yyval.expr)->ftype = (yyvsp[(2) - (3)].expr)->ftype; 
      set_security_flag_expr((yyval.expr), (yyvsp[(2) - (3)].expr), NULL, -1);
      set_bitlength_expr((yyval.expr), (yyvsp[(2) - (3)].expr), NULL); 
    ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 447 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (1)].expr);
    ;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 451 "parser.y"
    {
      (yyval.expr) = ArrayIndex((yyvsp[(1) - (4)].expr), (yyvsp[(3) - (4)].expr));
      set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), NULL, -1);
      set_bitlength_expr((yyval.expr), (yyvsp[(1) - (4)].expr), NULL);
      set_size_symbol((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(3) - (4)].expr));
      (yyval.expr)->ftype = (yyvsp[(1) - (4)].expr)->ftype;
      if((yyvsp[(3) - (4)].expr)->flag == PRI && (yyvsp[(1) - (4)].expr)->ftype == 0)
		is_priv_int_index_appear = 1;
      if((yyvsp[(3) - (4)].expr)->flag == PRI && (yyvsp[(1) - (4)].expr)->ftype == 1)
		is_priv_float_index_appear = 1;  
    ;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 469 "parser.y"
    {
      /* Catch calls to "main()" (unlikely but possible) */
      (yyval.expr) = strcmp((yyvsp[(1) - (3)].name), "main") ?
             FunctionCall(Identifier(Symbol((yyvsp[(1) - (3)].name))), NULL) :
             FunctionCall(Identifier(Symbol(MAIN_NEWNAME)), NULL);
      set_security_flag_func((yyvsp[(1) - (3)].name), (yyval.expr));
    ;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 477 "parser.y"
    {
      /* Catch calls to "main()" (unlikely but possible) */
      if (check_func_param(Identifier(Symbol((yyvsp[(1) - (4)].name))), (yyvsp[(3) - (4)].expr)))
          parse_error(1, "The provided arguments do not match function parameters\n");
      (yyval.expr) = strcmp((yyvsp[(1) - (4)].name), "main") ?
             FunctionCall(Identifier(Symbol((yyvsp[(1) - (4)].name))), (yyvsp[(3) - (4)].expr)) :
             FunctionCall(Identifier(Symbol(MAIN_NEWNAME)), (yyvsp[(3) - (4)].expr));
      // set the flag of func expr based on its return type
      set_security_flag_func((yyvsp[(1) - (4)].name), (yyval.expr));
    ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 488 "parser.y"
    {
        (yyval.expr) = FunctionCall((yyvsp[(1) - (3)].expr), NULL);
    ;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 492 "parser.y"
    {
        (yyval.expr) = FunctionCall((yyvsp[(1) - (4)].expr), (yyvsp[(3) - (4)].expr));
    ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 496 "parser.y"
    {
       (yyval.expr) = DotField((yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].symb));
  ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 500 "parser.y"
    {
       (yyval.expr) = PtrField((yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].symb));
  ;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 504 "parser.y"
    {
        (yyval.expr) = PostOperator((yyvsp[(1) - (2)].expr), UOP_inc);
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (2)].expr), NULL, -1);
	set_bitlength_expr((yyval.expr), (yyvsp[(1) - (2)].expr), NULL); 
    ;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 510 "parser.y"
    {
        (yyval.expr) = PostOperator((yyvsp[(1) - (2)].expr), UOP_dec);
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (2)].expr), NULL, -1);
	set_bitlength_expr((yyval.expr), (yyvsp[(1) - (2)].expr), NULL); 
   ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 516 "parser.y"
    {
	(yyval.expr) = DotField((yyvsp[(1) - (3)].expr), Symbol((yyvsp[(3) - (3)].name)));
	set_identifier_attributes(Symbol((yyvsp[(3) - (3)].name)), (yyval.expr), 1); 
    ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 521 "parser.y"
    {
	(yyval.expr) = PtrField((yyvsp[(1) - (3)].expr), Symbol((yyvsp[(3) - (3)].name)));
	set_identifier_attributes(Symbol((yyvsp[(3) - (3)].name)), (yyval.expr), 1); 
    ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 526 "parser.y"
    {
        (yyval.expr) = CastedExpr((yyvsp[(2) - (6)].decl), BracedInitializer((yyvsp[(5) - (6)].expr)));
    ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 530 "parser.y"
    {
        (yyval.expr) = CastedExpr((yyvsp[(2) - (7)].decl), BracedInitializer((yyvsp[(5) - (7)].expr)));
    ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 539 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (1)].expr);
    ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 543 "parser.y"
    {
      (yyval.expr) = CommaList((yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr));
    ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 551 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
        (yyval.expr)->thread_id = thread_id;  
    ;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 556 "parser.y"
    {   
        (yyval.expr) = PreOperator((yyvsp[(2) - (2)].expr), UOP_inc);
        set_security_flag_expr((yyval.expr), (yyvsp[(2) - (2)].expr), NULL, -1);
    	set_bitlength_expr((yyval.expr), (yyvsp[(2) - (2)].expr), NULL); 
        (yyval.expr)->thread_id = thread_id;  
    ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 563 "parser.y"
    {
        (yyval.expr) = PreOperator((yyvsp[(2) - (2)].expr), UOP_dec);
        set_security_flag_expr((yyval.expr), (yyvsp[(2) - (2)].expr), NULL, -1);
    	set_bitlength_expr((yyval.expr), (yyvsp[(2) - (2)].expr), NULL); 
        (yyval.expr)->thread_id = thread_id;  
    ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 570 "parser.y"
    {
	(yyval.expr) = UnaryOperator((yyvsp[(1) - (2)].type), (yyvsp[(2) - (2)].expr));
        (yyval.expr)->ftype = (yyvsp[(2) - (2)].expr)->ftype;
        set_security_flag_expr((yyval.expr), (yyvsp[(2) - (2)].expr), NULL, -1);
	set_bitlength_expr((yyval.expr), (yyvsp[(2) - (2)].expr), NULL); 
	//only for integer unary expression - !var
	if((yyvsp[(1) - (2)].type) == UOP_lnot)
	{
		increase_index((yyval.expr)); 
		(yyval.expr)->index = tmp_index;  
		decrease_index((yyval.expr)); 				
	} 
        (yyval.expr)->thread_id = thread_id;  
    ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 585 "parser.y"
    {
        if ((yyvsp[(1) - (2)].type) == -1)
            (yyval.expr) = (yyvsp[(2) - (2)].expr);                    /* simplify */
        else
            (yyval.expr) = UnaryOperator((yyvsp[(1) - (2)].type), (yyvsp[(2) - (2)].expr));
        (yyval.expr)->ftype = (yyvsp[(2) - (2)].expr)->ftype;
        set_security_flag_expr((yyval.expr), (yyvsp[(2) - (2)].expr), NULL, -1);
	set_bitlength_expr((yyval.expr), (yyvsp[(2) - (2)].expr), NULL); 
	//only for integer unary expression - !var
	if((yyvsp[(1) - (2)].type) == UOP_lnot)
	{
		increase_index((yyval.expr)); 
		(yyval.expr)->index = tmp_index;  
		decrease_index((yyval.expr)); 				
	} 
        (yyval.expr)->thread_id = thread_id;  
    ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 603 "parser.y"
    {
        (yyval.expr) = Sizeof((yyvsp[(2) - (2)].expr));
        (yyval.expr)->thread_id = thread_id; 	
    ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 608 "parser.y"
    {
        (yyval.expr) = Sizeoftype((yyvsp[(3) - (4)].decl));
	(yyval.expr)->thread_id = thread_id; 
    ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 614 "parser.y"
    {
                (yyval.expr) = Pmalloc((yyvsp[(3) - (6)].expr), (yyvsp[(5) - (6)].decl));
                (yyval.expr)->flag = 0; //make it PUB to be able to use it with public struct
      		(yyval.expr)->thread_id = thread_id;  
	;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 623 "parser.y"
    {
        (yyval.type) = UOP_neg; 
    ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 627 "parser.y"
    {
        (yyval.type) = UOP_lnot; 
    ;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 631 "parser.y"
    {
        (yyval.type) = UOP_addr;
    ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 635 "parser.y"
    {
        (yyval.type) = UOP_star;
    ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 642 "parser.y"
    {
        (yyval.decl) = Casttypename((yyvsp[(1) - (1)].spec), NULL);
    ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 646 "parser.y"
    {
        (yyval.decl) = Casttypename((yyvsp[(1) - (2)].spec), (yyvsp[(2) - (2)].decl));
    ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 650 "parser.y"
    {
	(yyval.decl) = Casttypename((yyvsp[(1) - (1)].spec), NULL); 
    ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 657 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
	(yyval.expr)->thread_id = thread_id; 
        (yyval.expr)->ftype = (yyvsp[(1) - (1)].expr)->ftype;
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (1)].expr), NULL, -1);
        (yyval.expr)->size = (yyvsp[(1) - (1)].expr)->size;
	(yyval.expr)->sizeexp  = (yyvsp[(1) - (1)].expr)->sizeexp; 
    ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 666 "parser.y"
    {
	if((yyvsp[(2) - (3)].decl)->spec->subtype == SPEC_int || (yyvsp[(2) - (3)].decl)->spec->subtype == SPEC_Rlist && ((yyvsp[(2) - (3)].decl)->spec->body->subtype == SPEC_private && (yyvsp[(2) - (3)].decl)->spec->u.next->subtype == SPEC_int))
		tmp_index++; 
 	if((yyvsp[(2) - (3)].decl)->spec->subtype == SPEC_float || (yyvsp[(2) - (3)].decl)->spec->subtype == SPEC_Rlist && ((yyvsp[(2) - (3)].decl)->spec->body->subtype == SPEC_private && (yyvsp[(2) - (3)].decl)->spec->u.next->subtype == SPEC_float))
		tmp_float_index++;  
    	num_index = num_index > tmp_index? num_index: tmp_index;
    	num_float_index = num_float_index > tmp_float_index ? num_float_index: tmp_float_index; 
    ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 675 "parser.y"
    {
        
	(yyval.expr) = CastedExpr((yyvsp[(2) - (5)].decl), (yyvsp[(5) - (5)].expr));
	(yyval.expr)->thread_id = thread_id; 
 	if((yyvsp[(2) - (5)].decl)->spec->subtype == SPEC_int || (yyvsp[(2) - (5)].decl)->spec->subtype == SPEC_Rlist && (yyvsp[(2) - (5)].decl)->spec->u.next->subtype == SPEC_int)
		(yyval.expr)->ftype = 0;
 	if((yyvsp[(2) - (5)].decl)->spec->subtype == SPEC_float || (yyvsp[(2) - (5)].decl)->spec->subtype == SPEC_Rlist && (yyvsp[(2) - (5)].decl)->spec->u.next->subtype == SPEC_float)
		(yyval.expr)->ftype = 1;
	(yyval.expr)->flag = (yyvsp[(5) - (5)].expr)->flag; 

 	if((yyvsp[(2) - (5)].decl)->spec->subtype == SPEC_int || (yyvsp[(2) - (5)].decl)->spec->subtype == SPEC_Rlist && ((yyvsp[(2) - (5)].decl)->spec->body->subtype == SPEC_private && (yyvsp[(2) - (5)].decl)->spec->u.next->subtype == SPEC_int))
	{
		(yyval.expr)->size = (yyvsp[(2) - (5)].decl)->spec->subtype == SPEC_Rlist ? (yyvsp[(2) - (5)].decl)->spec->u.next->size : (yyvsp[(2) - (5)].decl)->spec->size;  
		tmp_index--;
		(yyval.expr)->index = tmp_index; 
		(yyval.expr)->flag = PRI; 
		//FL2INT
		if((yyvsp[(5) - (5)].expr)->ftype == 1)
		 	modulus = fmax(modulus, fmax(2*(yyvsp[(5) - (5)].expr)->size+1, (yyvsp[(5) - (5)].expr)->sizeexp)+48);

	} 
 	if((yyvsp[(2) - (5)].decl)->spec->subtype == SPEC_float || (yyvsp[(2) - (5)].decl)->spec->subtype == SPEC_Rlist && ((yyvsp[(2) - (5)].decl)->spec->body->subtype == SPEC_private && (yyvsp[(2) - (5)].decl)->spec->u.next->subtype == SPEC_float))
	{
		(yyval.expr)->size = (yyvsp[(2) - (5)].decl)->spec->subtype == SPEC_Rlist ? (yyvsp[(2) - (5)].decl)->spec->u.next->size : (yyvsp[(2) - (5)].decl)->spec->size;  
		(yyval.expr)->sizeexp = (yyvsp[(2) - (5)].decl)->spec->subtype == SPEC_Rlist ? (yyvsp[(2) - (5)].decl)->spec->u.next->sizeexp : (yyvsp[(2) - (5)].decl)->spec->sizeexp;  
		tmp_float_index--;
		(yyval.expr)->index = tmp_float_index; 
		(yyval.expr)->flag = PRI; 
		//FL2FL
		if((yyvsp[(5) - (5)].expr)->ftype == 1 && (yyvsp[(5) - (5)].expr)->size > (yyval.expr)->size)
			modulus = fmax(modulus, (yyvsp[(5) - (5)].expr)->flag == PRI ? (yyvsp[(5) - (5)].expr)->size+48 : 32+48);
		//INT2FL
		if((yyvsp[(5) - (5)].expr)->ftype == 0)
			modulus = fmax(modulus, (yyvsp[(5) - (5)].expr)->flag == PRI ? (yyvsp[(5) - (5)].expr)->size+48 : 32+48); 
	}
    ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 716 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (1)].expr);
      set_bitlength_expr((yyval.expr), (yyvsp[(1) - (1)].expr), NULL); 
    ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 721 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 725 "parser.y"
    {
      decrease_index((yyvsp[(1) - (4)].expr)); 
      (yyval.expr) = BinaryOperator(BOP_mul, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
      (yyval.expr)->ftype = (yyvsp[(4) - (4)].expr)->ftype; 
      set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_mul);
      set_bitlength_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr)); 
    ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 733 "parser.y"
    {
       increase_index((yyvsp[(1) - (2)].expr));
   ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 737 "parser.y"
    {
      decrease_index((yyvsp[(1) - (4)].expr));
      (yyval.expr) = BinaryOperator(BOP_div, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
      (yyval.expr)->ftype = (yyvsp[(4) - (4)].expr)->ftype; 
      set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_div);
      set_bitlength_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr)); 
    ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 745 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 749 "parser.y"
    {
      decrease_index((yyvsp[(1) - (4)].expr));
      (yyval.expr) = BinaryOperator(BOP_mod, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
      (yyval.expr)->ftype = (yyvsp[(4) - (4)].expr)->ftype; 
      set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_mod);
      set_bitlength_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr)); 
    ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 761 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (1)].expr);
      set_bitlength_expr((yyval.expr), (yyvsp[(1) - (1)].expr), NULL); 
    ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 766 "parser.y"
    {
	increase_index((yyvsp[(1) - (2)].expr));
   ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 770 "parser.y"
    {
      decrease_index((yyvsp[(1) - (4)].expr));
      (yyval.expr) = BinaryOperator(BOP_add, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
      (yyval.expr)->ftype = (yyvsp[(4) - (4)].expr)->ftype; 
      set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_add);
      set_bitlength_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr)); 
  ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 778 "parser.y"
    {
      increase_index((yyvsp[(1) - (2)].expr));
  ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 782 "parser.y"
    {
      decrease_index((yyvsp[(1) - (4)].expr));
      (yyval.expr) = BinaryOperator(BOP_sub, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
      (yyval.expr)->ftype = (yyvsp[(4) - (4)].expr)->ftype; 
      set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_sub);
      set_bitlength_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr)); 
    ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 794 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
        set_bitlength_expr((yyval.expr), (yyvsp[(1) - (1)].expr), NULL); 
    ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 799 "parser.y"
    {
	increase_index((yyvsp[(1) - (2)].expr)); 
    ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 803 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr)); 
	(yyval.expr) = BinaryOperator(BOP_shl, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
	(yyval.expr)->ftype = (yyvsp[(1) - (4)].expr)->ftype; 
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), LEFT_OP);
        set_bitlength_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr)); 
    ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 811 "parser.y"
    {
	increase_index((yyvsp[(1) - (2)].expr)); 
    ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 815 "parser.y"
    {
	decrease_index((yyvsp[(1) - (4)].expr)); 
        (yyval.expr) = BinaryOperator(BOP_shr, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
	(yyval.expr)->ftype = (yyvsp[(1) - (4)].expr)->ftype; 
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), RIGHT_OP);
        set_bitlength_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr)); 
    ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 827 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
        set_bitlength_expr((yyval.expr), (yyvsp[(1) - (1)].expr), NULL); 
    ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 832 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 836 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr)); //needs to operate on tmp_index
        (yyval.expr) = BinaryOperator(BOP_lt, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
        (yyval.expr)->ftype = 0; 
	(yyval.expr)->size = 1; 
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_lt);
        //set_bitlength_expr($$, $1, $4); 
    ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 845 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 849 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr));
        (yyval.expr) = BinaryOperator(BOP_gt, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
      	(yyval.expr)->ftype = 0; 
	(yyval.expr)->size = 1; 
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_gt);
        //set_bitlength_expr($$, $1, $4); 
    ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 858 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 862 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr));
        (yyval.expr) = BinaryOperator(BOP_leq, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
        (yyval.expr)->ftype = 0; 
	(yyval.expr)->size = 1; 
	set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_leq);
        //set_bitlength_expr($$, $1, $4); 
    ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 871 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 875 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr));
        (yyval.expr) = BinaryOperator(BOP_geq, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
        (yyval.expr)->ftype = 0; 
	(yyval.expr)->size = 1; 
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_geq);
        //set_bitlength_expr($$, $1, $4); 
    ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 888 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
        set_bitlength_expr((yyval.expr), (yyvsp[(1) - (1)].expr), NULL); 
    ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 893 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 897 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr));
        (yyval.expr) = BinaryOperator(BOP_eqeq, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
        (yyval.expr)->ftype = 0;
	(yyval.expr)->size = 1;  
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_eqeq);
        //set_bitlength_expr($$, $1, $4); 
    ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 906 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    ;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 910 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr));
        (yyval.expr) = BinaryOperator(BOP_neq, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
        (yyval.expr)->ftype = 0; 
	(yyval.expr)->size = 1; 
	set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_neq);
        //set_bitlength_expr($$, $1, $4); 
    ;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 922 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
        set_bitlength_expr((yyval.expr), (yyvsp[(1) - (1)].expr), NULL); 
    ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 927 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    ;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 931 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr));
        (yyval.expr) = BinaryOperator(BOP_band, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_band);
        set_bitlength_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr)); 
    ;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 942 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
        set_bitlength_expr((yyval.expr), (yyvsp[(1) - (1)].expr), NULL); 
    ;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 947 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    ;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 951 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr));
        (yyval.expr) = BinaryOperator(BOP_xor, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_xor);
        set_bitlength_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr)); 
    ;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 962 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
        set_bitlength_expr((yyval.expr), (yyvsp[(1) - (1)].expr), NULL); 
    ;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 967 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    ;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 971 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr));
        (yyval.expr) = BinaryOperator(BOP_bor, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_bor);
        set_bitlength_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr)); 
    ;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 982 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
        set_bitlength_expr((yyval.expr), (yyvsp[(1) - (1)].expr), NULL); 
    ;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 987 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    ;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 991 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr));
        (yyval.expr) = BinaryOperator(BOP_land, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_land);
        set_bitlength_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr)); 
    ;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 1002 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
        set_bitlength_expr((yyval.expr), (yyvsp[(1) - (1)].expr), NULL); 
    ;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 1007 "parser.y"
    {
        increase_index((yyvsp[(1) - (2)].expr));
    ;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 1011 "parser.y"
    {
        decrease_index((yyvsp[(1) - (4)].expr));
        (yyval.expr) = BinaryOperator(BOP_lor, (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr));
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr), BOP_lor);
        set_bitlength_expr((yyval.expr), (yyvsp[(1) - (4)].expr), (yyvsp[(4) - (4)].expr)); 
    ;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 1019 "parser.y"
    {
        (yyval.expr) = BinaryOperator(BOP_dot, (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr));
        set_security_flag_expr((yyval.expr), (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr), BOP_dot);
        set_bitlength_expr((yyval.expr), (yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr)); 
    ;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 1028 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
        set_bitlength_expr((yyval.expr), (yyvsp[(1) - (1)].expr), NULL); 
    ;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 1033 "parser.y"
    {
        (yyval.expr) = (yyvsp[(1) - (1)].expr);
        set_bitlength_expr((yyval.expr), (yyvsp[(1) - (1)].expr), NULL); 
    ;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 1038 "parser.y"
    {
        (yyval.expr) = ConditionalExpr((yyvsp[(1) - (5)].expr), (yyvsp[(3) - (5)].expr), (yyvsp[(5) - (5)].expr));
        set_bitlength_expr((yyval.expr), (yyvsp[(3) - (5)].expr), (yyvsp[(5) - (5)].expr)); 
    ;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 1047 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (1)].expr);
      set_bitlength_expr((yyval.expr), (yyvsp[(1) - (1)].expr), NULL);
      (yyval.expr)->thread_id = thread_id;  
    ;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 1053 "parser.y"
    {
      //do security check here
      security_check_for_assignment((yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr));
      (yyval.expr) = Assignment((yyvsp[(1) - (3)].expr), (yyvsp[(2) - (3)].type), (yyvsp[(3) - (3)].expr));
      (yyval.expr)->ftype = (yyvsp[(1) - (3)].expr)->ftype; 
      set_security_flag_expr((yyval.expr), (yyvsp[(1) - (3)].expr), NULL, -1);
      (yyval.expr)->thread_id = thread_id; 
      //set_bitlength_expr($$, $1, $3); 
    ;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 1067 "parser.y"
    {
      (yyval.type) = ASS_eq;  /* Need fix here! */
    ;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 1071 "parser.y"
    {
      (yyval.type) = ASS_mul;
    ;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 1075 "parser.y"
    {
      (yyval.type) = ASS_div;
    ;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 1079 "parser.y"
    {
      (yyval.type) = ASS_mod;
    ;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 1083 "parser.y"
    {
      (yyval.type) = ASS_add;
    ;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 1087 "parser.y"
    {
      (yyval.type) = ASS_sub;
    ;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 1095 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (1)].expr);
      set_security_flag_expr((yyval.expr), (yyvsp[(1) - (1)].expr), NULL, -1);
      set_bitlength_expr((yyval.expr), (yyvsp[(1) - (1)].expr), NULL); 
    ;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 1101 "parser.y"
    {
      (yyval.expr) = CommaList((yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr));
    ;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 1109 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (1)].expr);
      set_security_flag_expr((yyval.expr), (yyvsp[(1) - (1)].expr), NULL, -1);
    ;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 1123 "parser.y"
    {
      if (isTypedef && (yyvsp[(1) - (2)].spec)->type == SPECLIST)
        (yyval.stmt) = Declaration((yyvsp[(1) - (2)].spec), fix_known_typename((yyvsp[(1) - (2)].spec)));
      else
        (yyval.stmt) = Declaration((yyvsp[(1) - (2)].spec), NULL);
      isTypedef = 0;

  ;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 1133 "parser.y"
    {
      if (checkDecls) add_declaration_links((yyvsp[(1) - (3)].spec), (yyvsp[(2) - (3)].decl), 0);
      	security_check_for_declaration((yyvsp[(1) - (3)].spec), (yyvsp[(2) - (3)].decl));
      (yyval.stmt) = Declaration((yyvsp[(1) - (3)].spec), (yyvsp[(2) - (3)].decl));
      isTypedef = 0;
      set_pointer_flag((yyvsp[(1) - (3)].spec), (yyvsp[(2) - (3)].decl));
    ;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 1141 "parser.y"
    {
        (yyval.stmt) = OmpStmt(OmpConstruct(DCTHREADPRIVATE, (yyvsp[(1) - (1)].odir), NULL));
    ;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 1150 "parser.y"
    {
        (yyval.spec) = (yyvsp[(1) - (1)].spec);
	compute_modulus_for_declaration((yyval.spec)); 
    ;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 1155 "parser.y"
    {
        (yyval.spec) = Speclist_right((yyvsp[(1) - (2)].spec), (yyvsp[(2) - (2)].spec));
	compute_modulus_for_declaration((yyval.spec)); 
    ;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 1160 "parser.y"
    {
        (yyval.spec) = (yyvsp[(1) - (1)].spec);
    ;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 1164 "parser.y"
    {
	(yyval.spec) = Speclist_right(StClassSpec(SPEC_typedef), (yyvsp[(2) - (2)].spec));
        isTypedef = 1;
    ;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 1169 "parser.y"
    {
      (yyval.spec) = Usertype((yyvsp[(1) - (1)].symb));
   ;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 1176 "parser.y"
    {
        (yyval.symb) = Symbol((yyvsp[(1) - (1)].name));
    ;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 1184 "parser.y"
    {
      (yyval.decl) = (yyvsp[(1) - (1)].decl);
    ;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 1188 "parser.y"
    {
      (yyval.decl) = DeclList((yyvsp[(1) - (3)].decl), (yyvsp[(3) - (3)].decl));
    ;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 1203 "parser.y"
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
    ;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 1219 "parser.y"
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
    ;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 1234 "parser.y"
    {
      (yyval.decl) = InitDecl((yyvsp[(1) - (4)].decl), (yyvsp[(4) - (4)].expr));
	
    ;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 1245 "parser.y"
    {
      (yyval.spec) = Declspec(SPEC_int, 32);
    ;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 1249 "parser.y"
    {
      (yyval.spec) = DeclspecFloat(SPEC_float, 32, 9); 
    ;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 1253 "parser.y"
    {
      (yyval.spec) = Declspec(SPEC_void, 0);
    ;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1257 "parser.y"
    {
      (yyval.spec) = Declspec(SPEC_int, atoi((yyvsp[(3) - (4)].name)));
    ;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 1261 "parser.y"
    {
      (yyval.spec) = DeclspecFloat(SPEC_float, atoi((yyvsp[(3) - (6)].name)), atoi((yyvsp[(5) - (6)].name))); 
    ;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1268 "parser.y"
    {
      (yyval.spec) = Declspec(SPEC_private, 0);
    ;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1272 "parser.y"
    {
      (yyval.spec) = Declspec(SPEC_public, 0);
    ;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1280 "parser.y"
    {
        (yyval.type) = SPEC_struct;
    ;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1284 "parser.y"
    {
        (yyval.type) = SPEC_union;
    ;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1291 "parser.y"
    {
        (yyval.decl) = (yyvsp[(1) - (1)].decl);
    ;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1295 "parser.y"
    {
        (yyval.decl) = StructfieldList((yyvsp[(1) - (2)].decl), (yyvsp[(2) - (2)].decl));
    ;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1303 "parser.y"
    {
        (yyval.decl) = StructfieldDecl((yyvsp[(1) - (3)].spec), (yyvsp[(2) - (3)].decl));
    ;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1307 "parser.y"
    {
        (yyval.decl) = StructfieldDecl((yyvsp[(1) - (2)].spec), NULL);
    ;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1315 "parser.y"
    {
        (yyval.decl) = (yyvsp[(1) - (1)].decl);
    ;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1319 "parser.y"
    {
        (yyval.decl) = DeclList((yyvsp[(1) - (3)].decl), (yyvsp[(3) - (3)].decl));
    ;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1327 "parser.y"
    {
      astdecl s = decl_getidentifier((yyvsp[(1) - (1)].decl));
      int     declkind = decl_getkind((yyvsp[(1) - (1)].decl));
      stentry e;
      (yyval.decl) = (yyvsp[(1) - (1)].decl);
    ;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1334 "parser.y"
    {
        (yyval.decl) = BitDecl((yyvsp[(1) - (3)].decl), (yyvsp[(3) - (3)].expr));
    ;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1338 "parser.y"
    {
        (yyval.decl) = BitDecl(NULL, (yyvsp[(2) - (2)].expr));
    ;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1346 "parser.y"
    {
        (yyval.spec) = SUdecl((yyvsp[(1) - (4)].type), NULL, (yyvsp[(3) - (4)].decl));
    ;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1350 "parser.y"
    {
        (yyval.spec) = SUdecl((yyvsp[(1) - (3)].type), NULL, NULL);
    ;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1354 "parser.y"
    {
	symbol s = Symbol((yyvsp[(2) - (5)].name));
	stentry e; 
	/* Well, struct & union names have their own name space, and
         * their own scopes. I.e. they can be re-declare in nested
         * scopes. We don't do any kind of duplicate checks.
         */
	
        if (checkDecls)
	{
        	e = symtab_put(stab, s, SUNAME);
		e->field = (yyvsp[(4) - (5)].decl); 
	}
        (yyval.spec) = SUdecl((yyvsp[(1) - (5)].type), s, (yyvsp[(4) - (5)].decl));
	store_struct_information(struct_table, (yyval.spec)); 
    ;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1375 "parser.y"
    {
        symbol s = (yyvsp[(2) - (5)].symb);
        if (checkDecls)
            symtab_put(stab, s, SUNAME);
            (yyval.spec) = SUdecl((yyvsp[(1) - (5)].type), s, (yyvsp[(4) - (5)].decl));
    ;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1382 "parser.y"
    {
        symbol s = Symbol((yyvsp[(2) - (2)].name));
        if (checkDecls)
        symtab_put(stab, s, SUNAME);
        (yyval.spec) = SUdecl((yyvsp[(1) - (2)].type), s, NULL);
    ;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1389 "parser.y"
    {
        symbol s = (yyvsp[(2) - (2)].symb);
        if (checkDecls)
        symtab_put(stab, s, SUNAME);
        (yyval.spec) = SUdecl((yyvsp[(1) - (2)].type), s, NULL);
    ;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1401 "parser.y"
    {
        (yyval.spec) = Pointer();
    ;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1405 "parser.y"
    {
        (yyval.spec) = Speclist_right(Pointer(), (yyvsp[(2) - (2)].spec));
    ;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1409 "parser.y"
    {
        (yyval.spec) = Speclist_right(Pointer(), (yyvsp[(2) - (2)].spec));
    ;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1413 "parser.y"
    {
        (yyval.spec) = Speclist_right( Pointer(), Speclist_left((yyvsp[(2) - (3)].spec), (yyvsp[(3) - (3)].spec)) );
    ;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1420 "parser.y"
    {
        (yyval.decl) = (yyvsp[(1) - (1)].decl);
    ;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1431 "parser.y"
    {
        (yyval.decl) = (yyvsp[(1) - (1)].decl);
    ;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1435 "parser.y"
    {
        (yyval.decl) = ParamList((yyvsp[(1) - (3)].decl), (yyvsp[(3) - (3)].decl));
    ;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1442 "parser.y"
    {
        (yyval.decl) = ParamDecl((yyvsp[(1) - (2)].spec), (yyvsp[(2) - (2)].decl));
	set_pointer_flag((yyvsp[(1) - (2)].spec), (yyvsp[(2) - (2)].decl)); 
    ;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 1447 "parser.y"
    {
        (yyval.decl) = ParamDecl((yyvsp[(1) - (1)].spec), NULL);
    ;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1451 "parser.y"
    {
        (yyval.decl) = ParamDecl((yyvsp[(1) - (2)].spec), (yyvsp[(2) - (2)].decl));
	set_pointer_flag((yyvsp[(1) - (2)].spec), (yyvsp[(2) - (2)].decl)); 
    ;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1460 "parser.y"
    {
        (yyval.decl) = AbstractDeclarator((yyvsp[(1) - (1)].spec), NULL);
    ;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1464 "parser.y"
    {
        (yyval.decl) = AbstractDeclarator(NULL, (yyvsp[(1) - (1)].decl));
    ;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1468 "parser.y"
    {
        (yyval.decl) = AbstractDeclarator((yyvsp[(1) - (2)].spec), (yyvsp[(2) - (2)].decl));
    ;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1475 "parser.y"
    {
        (yyval.decl) = ParenDecl((yyvsp[(2) - (3)].decl));
    ;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1479 "parser.y"
    {
        (yyval.decl) = ArrayDecl(NULL, NULL, NULL);
    ;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1483 "parser.y"
    {
        (yyval.decl) = ArrayDecl((yyvsp[(1) - (3)].decl), NULL, NULL);
    ;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1487 "parser.y"
    {
        (yyval.decl) = ArrayDecl(NULL, NULL, (yyvsp[(2) - (3)].expr));
    ;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1491 "parser.y"
    {
        (yyval.decl) = ArrayDecl((yyvsp[(1) - (4)].decl), NULL, (yyvsp[(3) - (4)].expr));
    ;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1495 "parser.y"
    {
        (yyval.decl) = ArrayDecl(NULL, Declspec(SPEC_star, 0), NULL);
    ;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1499 "parser.y"
    {
        (yyval.decl) = ArrayDecl((yyvsp[(1) - (4)].decl), Declspec(SPEC_star, 0), NULL);
    ;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1503 "parser.y"
    {
        (yyval.decl) = FuncDecl(NULL, NULL);
    ;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1507 "parser.y"
    {
        (yyval.decl) = FuncDecl((yyvsp[(1) - (3)].decl), NULL);
    ;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1511 "parser.y"
    {
        (yyval.decl) = FuncDecl(NULL, (yyvsp[(2) - (3)].decl));
    ;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1515 "parser.y"
    {
        (yyval.decl) = FuncDecl((yyvsp[(1) - (4)].decl), (yyvsp[(3) - (4)].decl));
    ;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1523 "parser.y"
    {
      (yyval.spec) = (yyvsp[(1) - (1)].spec);
      compute_modulus_for_declaration((yyval.spec)); 
    ;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1528 "parser.y"
    {
      (yyval.spec) = Speclist_right((yyvsp[(1) - (2)].spec), (yyvsp[(2) - (2)].spec));	
    ;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 1532 "parser.y"
    {
	(yyval.spec) = (yyvsp[(1) - (1)].spec); 
    ;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 1536 "parser.y"
    {
	(yyval.spec) = Speclist_right((yyvsp[(1) - (2)].spec), (yyvsp[(2) - (2)].spec)); 
	compute_modulus_for_declaration((yyval.spec)); 
   ;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1541 "parser.y"
    {
	(yyval.spec) = (yyvsp[(1) - (1)].spec); 
   ;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 1549 "parser.y"
    {
      (yyval.decl) = Declarator(NULL, (yyvsp[(1) - (1)].decl));
    ;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 1553 "parser.y"
    {
        (yyval.decl) = Declarator((yyvsp[(1) - (2)].spec), (yyvsp[(2) - (2)].decl));
    ;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 1562 "parser.y"
    {
      (yyval.decl) = IdentifierDecl( Symbol((yyvsp[(1) - (1)].name)) );
    ;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 1566 "parser.y"
    {
      /* Try to simplify a bit: (ident) -> ident */
      if ((yyvsp[(2) - (3)].decl)->spec == NULL && (yyvsp[(2) - (3)].decl)->decl->type == DIDENT)
        (yyval.decl) = (yyvsp[(2) - (3)].decl)->decl;
      else
        (yyval.decl) = ParenDecl((yyvsp[(2) - (3)].decl));
    ;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 1574 "parser.y"
    {
      (yyval.decl) = ArrayDecl((yyvsp[(1) - (3)].decl), NULL, NULL);
    ;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 1578 "parser.y"
    {
      (yyval.decl) = ArrayDecl((yyvsp[(1) - (4)].decl), NULL, (yyvsp[(3) - (4)].expr));
    ;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 1582 "parser.y"
    {
      (yyval.decl) = FuncDecl((yyvsp[(1) - (3)].decl), NULL);
    ;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 1586 "parser.y"
    {
      (yyval.decl) = FuncDecl((yyvsp[(1) - (4)].decl), (yyvsp[(3) - (4)].decl));
    ;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 1590 "parser.y"
    {
      (yyval.decl) = FuncDecl((yyvsp[(1) - (4)].decl), (yyvsp[(3) - (4)].decl));
    ;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 1599 "parser.y"
    {
      (yyval.decl) = IdentifierDecl( Symbol((yyvsp[(1) - (1)].name)) );
    ;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 1603 "parser.y"
    {
      (yyval.decl) = IdList((yyvsp[(1) - (3)].decl), IdentifierDecl( Symbol((yyvsp[(3) - (3)].name)) ));
    ;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 1613 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (1)].expr);
    ;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 1617 "parser.y"
    {
      (yyval.expr) = BracedInitializer((yyvsp[(2) - (3)].expr));
    ;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 1621 "parser.y"
    {
      (yyval.expr) = BracedInitializer((yyvsp[(2) - (4)].expr));
    ;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 1629 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (1)].expr);
    ;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 1633 "parser.y"
    {
      (yyval.expr) = Designated((yyvsp[(1) - (2)].expr), (yyvsp[(2) - (2)].expr));
    ;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 1637 "parser.y"
    {
      (yyval.expr) = CommaList((yyvsp[(1) - (3)].expr), (yyvsp[(3) - (3)].expr));
    ;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 1641 "parser.y"
    {
      (yyval.expr) = CommaList((yyvsp[(1) - (4)].expr), Designated((yyvsp[(3) - (4)].expr), (yyvsp[(4) - (4)].expr)));
    ;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 1649 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (2)].expr); 
    ;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 1657 "parser.y"
    {
      (yyval.expr) = (yyvsp[(1) - (1)].expr);
    ;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 1661 "parser.y"
    {
      (yyval.expr) = SpaceList((yyvsp[(1) - (2)].expr), (yyvsp[(2) - (2)].expr));
    ;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 1669 "parser.y"
    {
        security_check_for_condition((yyvsp[(2) - (3)].expr));
        (yyval.expr) = IdxDesignator((yyvsp[(2) - (3)].expr));
     ;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 1674 "parser.y"
    {
        (yyval.expr) = DotDesignator( Symbol((yyvsp[(2) - (2)].name)) );
     ;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 1678 "parser.y"
    {
        (yyval.expr) = DotDesignator((yyvsp[(2) - (2)].symb));
    ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 1694 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
      (yyval.stmt)->flag = (yyvsp[(1) - (1)].stmt)->flag;
    ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 1699 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
      (yyval.stmt)->flag = (yyvsp[(1) - (1)].stmt)->flag;
    ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 1704 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
      (yyval.stmt)->flag = (yyvsp[(1) - (1)].stmt)->flag; 
    ;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 1709 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
      (yyval.stmt)->flag = (yyvsp[(1) - (1)].stmt)->flag;
    ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 1714 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
      (yyval.stmt)->flag = (yyvsp[(1) - (1)].stmt)->flag; 
    ;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 1719 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
      (yyval.stmt)->flag = (yyvsp[(1) - (1)].stmt)->flag; 
    ;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 1724 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
      (yyval.stmt)->flag = (yyvsp[(1) - (1)].stmt)->flag; 
    ;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 1730 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
      (yyval.stmt)->flag = (yyvsp[(1) - (1)].stmt)->flag;
    ;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 1736 "parser.y"
    {
     (yyval.stmt) = OmpStmt((yyvsp[(1) - (1)].ocon));
     (yyval.stmt)->l = (yyvsp[(1) - (1)].ocon)->l;
    ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 1741 "parser.y"
    {
    (yyval.stmt) = OmpixStmt((yyvsp[(1) - (1)].xcon));
    (yyval.stmt)->l = (yyvsp[(1) - (1)].xcon)->l;
  ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 1746 "parser.y"
    {
        (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
   ;}
    break;

  case 209:

/* Line 1455 of yacc.c  */
#line 1753 "parser.y"
    {
                (yyval.stmt) = OmpStmt(OmpConstruct(DCPARSECTIONS, OmpDirective(DCPARSECTIONS, NULL), (yyvsp[(1) - (1)].stmt)));
                (yyval.stmt)->l = (yyvsp[(1) - (1)].stmt)->l;
		thread_id = -1; 
        ;}
    break;

  case 210:

/* Line 1455 of yacc.c  */
#line 1761 "parser.y"
    {
		thread_id++; 
		if(thread_id >= num_threads)
			num_threads = thread_id+1; 
	;}
    break;

  case 211:

/* Line 1455 of yacc.c  */
#line 1770 "parser.y"
    {
                (yyval.stmt) = Compound((yyvsp[(1) - (1)].stmt));
        ;}
    break;

  case 212:

/* Line 1455 of yacc.c  */
#line 1777 "parser.y"
    {
                (yyval.stmt) = OmpStmt( OmpConstruct(DCSECTION, OmpDirective(DCSECTION,NULL), Compound((yyvsp[(3) - (4)].stmt))) );
        ;}
    break;

  case 213:

/* Line 1455 of yacc.c  */
#line 1784 "parser.y"
    {
		(yyval.stmt) = (yyvsp[(1) - (1)].stmt); 
	;}
    break;

  case 214:

/* Line 1455 of yacc.c  */
#line 1788 "parser.y"
    {
                (yyval.stmt) = BlockList((yyvsp[(1) - (2)].stmt), (yyvsp[(2) - (2)].stmt));
        ;}
    break;

  case 215:

/* Line 1455 of yacc.c  */
#line 1796 "parser.y"
    {
      (yyval.stmt) = Compound(NULL);
    ;}
    break;

  case 216:

/* Line 1455 of yacc.c  */
#line 1799 "parser.y"
    { (yyval.type) = sc_original_line()-1; scope_start(stab);;}
    break;

  case 217:

/* Line 1455 of yacc.c  */
#line 1801 "parser.y"
    {
      (yyval.stmt) = Compound((yyvsp[(3) - (4)].stmt));
      scope_end(stab);
      (yyval.stmt)->l = (yyvsp[(2) - (4)].type);     /* Remember 1st line */
      (yyval.stmt)->flag = (yyvsp[(3) - (4)].stmt)->flag; 
    ;}
    break;

  case 218:

/* Line 1455 of yacc.c  */
#line 1812 "parser.y"
    {
        (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
        (yyval.stmt)->flag = (yyvsp[(1) - (1)].stmt)->flag;
    ;}
    break;

  case 219:

/* Line 1455 of yacc.c  */
#line 1817 "parser.y"
    {
      (yyval.stmt) = BlockList((yyvsp[(1) - (2)].stmt), (yyvsp[(2) - (2)].stmt));
      (yyval.stmt)->l = (yyvsp[(1) - (2)].stmt)->l;
      set_security_flag_stmt((yyval.stmt), (yyvsp[(1) - (2)].stmt), (yyvsp[(2) - (2)].stmt));
    ;}
    break;

  case 220:

/* Line 1455 of yacc.c  */
#line 1827 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
    ;}
    break;

  case 221:

/* Line 1455 of yacc.c  */
#line 1831 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
      (yyval.stmt)->flag = (yyvsp[(1) - (1)].stmt)->flag;
    ;}
    break;

  case 222:

/* Line 1455 of yacc.c  */
#line 1836 "parser.y"
    {
        (yyval.stmt) = OmpStmt((yyvsp[(1) - (1)].ocon));
        (yyval.stmt)->l = (yyvsp[(1) - (1)].ocon)->l;
    ;}
    break;

  case 223:

/* Line 1455 of yacc.c  */
#line 1841 "parser.y"
    {
        (yyval.stmt) = OmpixStmt((yyvsp[(1) - (1)].xcon));
        (yyval.stmt)->l = (yyvsp[(1) - (1)].xcon)->l;
    ;}
    break;

  case 224:

/* Line 1455 of yacc.c  */
#line 1850 "parser.y"
    {
      (yyval.stmt) = Expression(NULL);
    ;}
    break;

  case 225:

/* Line 1455 of yacc.c  */
#line 1854 "parser.y"
    {
      (yyval.stmt) = Expression((yyvsp[(1) - (2)].expr));
      (yyval.stmt)->l = (yyvsp[(1) - (2)].expr)->l;
      (yyval.stmt)->flag = (yyvsp[(1) - (2)].expr)->flag;
    ;}
    break;

  case 226:

/* Line 1455 of yacc.c  */
#line 1863 "parser.y"
    {
        tmp_index++;
        if(tmp_index > cond_index)
            cond_index = tmp_index;
    ;}
    break;

  case 227:

/* Line 1455 of yacc.c  */
#line 1870 "parser.y"
    {
        (yyval.stmt) = If((yyvsp[(3) - (6)].expr), (yyvsp[(6) - (6)].stmt), NULL);            
        if((yyvsp[(3) - (6)].expr)->flag == PRI && (yyvsp[(6) - (6)].stmt)->flag == PUB)
           parse_error(1, "public assignment is not allowd within the private condition\n");
        if((yyvsp[(3) - (6)].expr)->flag == PRI)
            contain_priv_if_flag = 1; 
        (yyval.stmt)->flag = (yyvsp[(6) - (6)].stmt)->flag;
            tmp_index--;

    ;}
    break;

  case 228:

/* Line 1455 of yacc.c  */
#line 1881 "parser.y"
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
    ;}
    break;

  case 229:

/* Line 1455 of yacc.c  */
#line 1899 "parser.y"
    {
        security_check_for_condition((yyvsp[(3) - (5)].expr)); 
        (yyval.stmt) = While((yyvsp[(3) - (5)].expr), (yyvsp[(5) - (5)].stmt));
        (yyval.stmt)->flag = (yyvsp[(5) - (5)].stmt)->flag;
    ;}
    break;

  case 230:

/* Line 1455 of yacc.c  */
#line 1905 "parser.y"
    {
        security_check_for_condition((yyvsp[(5) - (7)].expr));         
        (yyval.stmt) = Do((yyvsp[(2) - (7)].stmt), (yyvsp[(5) - (7)].expr));
        (yyval.stmt)->flag = (yyvsp[(2) - (7)].stmt)->flag;
    ;}
    break;

  case 231:

/* Line 1455 of yacc.c  */
#line 1911 "parser.y"
    {
        (yyval.stmt)->flag = (yyvsp[(1) - (1)].stmt)->flag;
    ;}
    break;

  case 232:

/* Line 1455 of yacc.c  */
#line 1918 "parser.y"
    {
        (yyval.stmt) = For(NULL, NULL, NULL, (yyvsp[(6) - (6)].stmt));
        (yyval.stmt)->flag = (yyvsp[(6) - (6)].stmt)->flag;
    ;}
    break;

  case 233:

/* Line 1455 of yacc.c  */
#line 1923 "parser.y"
    {
        (yyval.stmt) = For(Expression((yyvsp[(3) - (7)].expr)), NULL, NULL, (yyvsp[(7) - (7)].stmt));
        (yyval.stmt)->flag = (yyvsp[(7) - (7)].stmt)->flag;
    ;}
    break;

  case 234:

/* Line 1455 of yacc.c  */
#line 1928 "parser.y"
    {
        security_check_for_condition((yyvsp[(4) - (7)].expr));         
        (yyval.stmt) = For(NULL, (yyvsp[(4) - (7)].expr), NULL, (yyvsp[(7) - (7)].stmt));
        (yyval.stmt)->flag = (yyvsp[(7) - (7)].stmt)->flag;
    ;}
    break;

  case 235:

/* Line 1455 of yacc.c  */
#line 1934 "parser.y"
    {
        (yyval.stmt) = For(NULL, NULL, (yyvsp[(5) - (7)].expr), (yyvsp[(7) - (7)].stmt));
        (yyval.stmt)->flag = (yyvsp[(7) - (7)].stmt)->flag;

    ;}
    break;

  case 236:

/* Line 1455 of yacc.c  */
#line 1940 "parser.y"
    {
        security_check_for_condition((yyvsp[(5) - (8)].expr));                 
        (yyval.stmt) = For(Expression((yyvsp[(3) - (8)].expr)), (yyvsp[(5) - (8)].expr), NULL, (yyvsp[(8) - (8)].stmt));
        (yyval.stmt)->flag = (yyvsp[(8) - (8)].stmt)->flag;
    ;}
    break;

  case 237:

/* Line 1455 of yacc.c  */
#line 1946 "parser.y"
    {
        (yyval.stmt) = For(Expression((yyvsp[(3) - (8)].expr)), NULL, (yyvsp[(6) - (8)].expr), (yyvsp[(8) - (8)].stmt));
        (yyval.stmt)->flag = (yyvsp[(8) - (8)].stmt)->flag;
    ;}
    break;

  case 238:

/* Line 1455 of yacc.c  */
#line 1951 "parser.y"
    {
        security_check_for_condition((yyvsp[(4) - (8)].expr));                 
        (yyval.stmt) = For(NULL, (yyvsp[(4) - (8)].expr), (yyvsp[(6) - (8)].expr), (yyvsp[(8) - (8)].stmt));
        (yyval.stmt)->flag = (yyvsp[(8) - (8)].stmt)->flag;
    ;}
    break;

  case 239:

/* Line 1455 of yacc.c  */
#line 1957 "parser.y"
    {	
        security_check_for_condition((yyvsp[(5) - (9)].expr));         
        (yyval.stmt) = For(Expression((yyvsp[(3) - (9)].expr)), (yyvsp[(5) - (9)].expr), (yyvsp[(7) - (9)].expr), (yyvsp[(9) - (9)].stmt));
        (yyval.stmt)->flag = (yyvsp[(9) - (9)].stmt)->flag;

    ;}
    break;

  case 240:

/* Line 1455 of yacc.c  */
#line 1964 "parser.y"
    {
        (yyval.stmt) = For((yyvsp[(3) - (6)].stmt), NULL, NULL, (yyvsp[(6) - (6)].stmt));
        (yyval.stmt)->flag = (yyvsp[(6) - (6)].stmt)->flag;

    ;}
    break;

  case 241:

/* Line 1455 of yacc.c  */
#line 1970 "parser.y"
    {
        (yyval.stmt) = For((yyvsp[(3) - (7)].stmt), (yyvsp[(4) - (7)].expr), NULL, (yyvsp[(7) - (7)].stmt));
        security_check_for_condition((yyvsp[(4) - (7)].expr));
        (yyval.stmt)->flag = (yyvsp[(7) - (7)].stmt)->flag;

    ;}
    break;

  case 242:

/* Line 1455 of yacc.c  */
#line 1977 "parser.y"
    {
        (yyval.stmt) = For((yyvsp[(3) - (7)].stmt), NULL, (yyvsp[(5) - (7)].expr), (yyvsp[(7) - (7)].stmt));
        (yyval.stmt)->flag = (yyvsp[(7) - (7)].stmt)->flag;

    ;}
    break;

  case 243:

/* Line 1455 of yacc.c  */
#line 1983 "parser.y"
    {
        security_check_for_condition((yyvsp[(4) - (8)].expr));                 
        (yyval.stmt) = For((yyvsp[(3) - (8)].stmt), (yyvsp[(4) - (8)].expr), (yyvsp[(6) - (8)].expr), (yyvsp[(8) - (8)].stmt));
        (yyval.stmt)->flag = (yyvsp[(8) - (8)].stmt)->flag;

    ;}
    break;

  case 244:

/* Line 1455 of yacc.c  */
#line 1993 "parser.y"
    {
        (yyval.stmt) = Continue();
    ;}
    break;

  case 245:

/* Line 1455 of yacc.c  */
#line 1997 "parser.y"
    {
        (yyval.stmt) = Break();
    ;}
    break;

  case 246:

/* Line 1455 of yacc.c  */
#line 2001 "parser.y"
    {
        (yyval.stmt) = Return(NULL);
    ;}
    break;

  case 247:

/* Line 1455 of yacc.c  */
#line 2005 "parser.y"
    {
        if(((yyvsp[(2) - (3)].expr)->flag == PRI && func_return_flag == 0)
           || ((yyvsp[(2) - (3)].expr)->flag == PUB && func_return_flag == 1))
	{
		parse_error(-1, "return type does not match.\n");
        }
        (yyval.stmt) = Return((yyvsp[(2) - (3)].expr));
        
    ;}
    break;

  case 248:

/* Line 1455 of yacc.c  */
#line 2019 "parser.y"
    {
        int secrecy = 1; /* PRI = 1, PUB = 2 */
        stentry e = get_entry_from_expr((yyvsp[(3) - (7)].expr));
        /* the varible is public type */
        if(set_security_flag_spec(e->spec) != PRI)
           secrecy = 2;  
        /* search for the symtab to find the size of variable */
        stentry e1 = symtab_get(stab, e->key, IDNAME);
        astspec spec1;
        
        if(e1->spec->type == SPEC)
            spec1 = e1->spec;
        else if(e1->spec->type == SPECLIST)
            spec1 = e1->spec->u.next;

        str arg_str = Str("");
        ast_expr_print(arg_str, (yyvsp[(5) - (7)].expr));
       
        fprintf(var_file, "I:%d,%s,%s,%s,1", secrecy, e1->key->name, SPEC_symbols[spec1->subtype], str_string(arg_str));
        if(spec1->subtype == SPEC_int)
                fprintf(var_file, ",%d\n", spec1->size);
        else if(spec1->subtype == SPEC_float)
                fprintf(var_file, ",%d,%d\n", spec1->size, spec1->sizeexp);
	(yyvsp[(3) - (7)].expr)->thread_id = thread_id;  
        (yyval.stmt) = Smc(SINPUT, e->spec, (yyvsp[(3) - (7)].expr), (yyvsp[(5) - (7)].expr), NULL, NULL);
    ;}
    break;

  case 249:

/* Line 1455 of yacc.c  */
#line 2047 "parser.y"
    {
        
        int secrecy = 1; 
        stentry e = get_entry_from_expr((yyvsp[(3) - (9)].expr));
        if(set_security_flag_spec(e->spec) != PRI)
                secrecy = 2; 
        /* search for the symtab to find the size of variable */
        stentry e1 = symtab_get(stab, e->key, IDNAME);
        astspec spec1;
        if(e1->spec->type == SPEC)
            spec1 = e1->spec;
        else if(e1->spec->type == SPECLIST)
            spec1 = e1->spec->u.next;
            
        /*for party*/
        str arg_str1 = Str("");
        ast_expr_print(arg_str1, (yyvsp[(5) - (9)].expr));

        /*for arraysize*/
        str arg_str2 = Str("");
        get_arraysize((yyvsp[(7) - (9)].expr), arg_str2);
        fprintf(var_file, "I:%d,%s,%s,%s,%s", secrecy, e1->key->name, SPEC_symbols[spec1->subtype], str_string(arg_str1), str_string(arg_str2));
        if(spec1->subtype == SPEC_int)
                fprintf(var_file, ",%d\n", spec1->size);
        else if(spec1->subtype == SPEC_float)
                fprintf(var_file, ",%d,%d\n", spec1->size, spec1->sizeexp);
	
	(yyvsp[(3) - (9)].expr)->thread_id = thread_id; 
        (yyval.stmt) = Smc(SINPUT, e->spec, (yyvsp[(3) - (9)].expr), (yyvsp[(5) - (9)].expr), (yyvsp[(7) - (9)].expr), NULL);
    ;}
    break;

  case 250:

/* Line 1455 of yacc.c  */
#line 2079 "parser.y"
    {
        int secrecy = 1; 
        stentry e = get_entry_from_expr((yyvsp[(3) - (11)].expr));
        if(set_security_flag_spec(e->spec) != PRI)
                secrecy = 2; 
        stentry e1 = symtab_get(stab, e->key, IDNAME);
        astspec spec1;
        if(e1->spec->type == SPEC)
            spec1 = e1->spec;
        else if(e1->spec->type == SPECLIST)
            spec1 = e1->spec->u.next;
        /*for party*/
        str arg_str1 = Str("");
        ast_expr_print(arg_str1, (yyvsp[(5) - (11)].expr));
        /*for arraysize*/
        str arg_str2 = Str("");
        get_arraysize((yyvsp[(7) - (11)].expr), arg_str2);

        str arg_str3 = Str("");
        get_arraysize((yyvsp[(9) - (11)].expr), arg_str3);

        fprintf(var_file, "I:%d,%s,%s,%s,%s,%s", secrecy, e1->key->name, SPEC_symbols[spec1->subtype], str_string(arg_str1), str_string(arg_str2), str_string(arg_str3));
        /*for element size*/
       if(spec1->subtype == SPEC_int)
                fprintf(var_file, ",%d\n", spec1->size);
        else if(spec1->subtype == SPEC_float)
                fprintf(var_file, ",%d,%d\n", spec1->size, spec1->sizeexp);
	(yyvsp[(3) - (11)].expr)->thread_id = thread_id; 
        (yyval.stmt) = Smc(SINPUT, e->spec, (yyvsp[(3) - (11)].expr), (yyvsp[(5) - (11)].expr), (yyvsp[(7) - (11)].expr), (yyvsp[(9) - (11)].expr));
    ;}
    break;

  case 251:

/* Line 1455 of yacc.c  */
#line 2110 "parser.y"
    {
        
        int secrecy = 1; /* PRI = 1, PUB = 2 */
        stentry e = get_entry_from_expr((yyvsp[(3) - (7)].expr));
        /* the varible is public type */
        if(set_security_flag_spec(e->spec) != PRI)
           secrecy = 2;
        /* search for the symtab to find the size of variable */
        stentry e1 = symtab_get(stab, e->key, IDNAME);
        astspec spec1;

        if(e1->spec->type == SPEC)
            spec1 = e1->spec;
        else if(e1->spec->type == SPECLIST)
            spec1 = e1->spec->u.next;

        str arg_str = Str("");
        ast_expr_print(arg_str, (yyvsp[(5) - (7)].expr));

        fprintf(var_file, "O:%d,%s,%s,%s,1", secrecy, e1->key->name, SPEC_symbols[spec1->subtype], str_string(arg_str));
        if(spec1->subtype == SPEC_int)
               fprintf(var_file, ",%d\n", spec1->size);
        else if(spec1->subtype == SPEC_float)
               fprintf(var_file, ",%d,%d\n", spec1->size, spec1->sizeexp);
	(yyvsp[(3) - (7)].expr)->thread_id = thread_id;
        (yyval.stmt) = Smc(SOUTPUT, e->spec, (yyvsp[(3) - (7)].expr), (yyvsp[(5) - (7)].expr), NULL, NULL);
    ;}
    break;

  case 252:

/* Line 1455 of yacc.c  */
#line 2138 "parser.y"
    {
    
        int secrecy = 1;
        stentry e = get_entry_from_expr((yyvsp[(3) - (9)].expr));
        if(set_security_flag_spec(e->spec) != PRI)
                secrecy = 2;
        /* search for the symtab to find the size of variable */
        stentry e1 = symtab_get(stab, e->key, IDNAME);
        astspec spec1;
        if(e1->spec->type == SPEC)
            spec1 = e1->spec;
        else if(e1->spec->type == SPECLIST)
            spec1 = e1->spec->u.next;

        /* for party */
        str arg_str1 = Str("");
        ast_expr_print(arg_str1, (yyvsp[(5) - (9)].expr));

        /* for arraysize */
        str arg_str2 = Str("");
        get_arraysize((yyvsp[(7) - (9)].expr), arg_str2);

        fprintf(var_file, "O:%d,%s,%s,%s,%s", secrecy, e1->key->name, SPEC_symbols[spec1->subtype], str_string(arg_str1), str_string(arg_str2));
        if(spec1->subtype == SPEC_int)
              fprintf(var_file, ",%d\n", spec1->size);
        else if(spec1->subtype == SPEC_float)
              fprintf(var_file, ",%d,%d\n", spec1->size, spec1->sizeexp);
	(yyvsp[(3) - (9)].expr)->thread_id = thread_id; 
	
	(yyval.stmt) = Smc(SOUTPUT, e->spec, (yyvsp[(3) - (9)].expr), (yyvsp[(5) - (9)].expr), (yyvsp[(7) - (9)].expr), NULL);
    ;}
    break;

  case 253:

/* Line 1455 of yacc.c  */
#line 2170 "parser.y"
    {
        int secrecy = 1;
        stentry e = get_entry_from_expr((yyvsp[(3) - (11)].expr));
        if(set_security_flag_spec(e->spec) != PRI)
                secrecy = 2;
        stentry e1 = symtab_get(stab, e->key, IDNAME);
        astspec spec1;
        if(e1->spec->type == SPEC)
            spec1 = e1->spec;
        else if(e1->spec->type == SPECLIST)
            spec1 = e1->spec->u.next;
        /* for party */
        str arg_str1 = Str("");
        ast_expr_print(arg_str1, (yyvsp[(5) - (11)].expr));
        /* for arraysize */
        str arg_str2 = Str("");
        get_arraysize((yyvsp[(7) - (11)].expr), arg_str2);

        str arg_str3 = Str("");
        get_arraysize((yyvsp[(9) - (11)].expr), arg_str3);

        fprintf(var_file, "O:%d,%s,%s,%s,%s,%s", secrecy, e1->key->name, SPEC_symbols[spec1->subtype], str_string(arg_str1), str_string(arg_str2), str_string(arg_str3));
        /* for element size */
        if(spec1->subtype == SPEC_int)
               fprintf(var_file, ",%d\n", spec1->size);
        else if(spec1->subtype == SPEC_float)
               fprintf(var_file, ",%d,%d\n", spec1->size, spec1->sizeexp);
	(yyvsp[(3) - (11)].expr)->thread_id = thread_id; 
        (yyval.stmt) = Smc(SOUTPUT, e->spec, (yyvsp[(3) - (11)].expr), (yyvsp[(5) - (11)].expr), (yyvsp[(7) - (11)].expr), (yyvsp[(9) - (11)].expr));
    ;}
    break;

  case 254:

/* Line 1455 of yacc.c  */
#line 2204 "parser.y"
    {
        security_check_for_condition((yyvsp[(5) - (11)].expr));
        //$$->flag = $10->flag;
        (yyval.stmt) = Batch(Expression((yyvsp[(3) - (11)].expr)), (yyvsp[(5) - (11)].expr), (yyvsp[(7) - (11)].expr), (yyvsp[(10) - (11)].stmt));
    ;}
    break;

  case 255:

/* Line 1455 of yacc.c  */
#line 2214 "parser.y"
    {
        (yyval.stmt) = Pfree((yyvsp[(3) - (5)].expr));
    ;}
    break;

  case 256:

/* Line 1455 of yacc.c  */
#line 2227 "parser.y"
    {
	(yyval.stmt) = pastree = (yyvsp[(1) - (1)].stmt);
    ;}
    break;

  case 257:

/* Line 1455 of yacc.c  */
#line 2231 "parser.y"
    {
      (yyval.stmt) = pastree = BlockList((yyvsp[(1) - (2)].stmt), (yyvsp[(2) - (2)].stmt));
    ;}
    break;

  case 258:

/* Line 1455 of yacc.c  */
#line 2239 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
    ;}
    break;

  case 259:

/* Line 1455 of yacc.c  */
#line 2243 "parser.y"
    {
      (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
    ;}
    break;

  case 260:

/* Line 1455 of yacc.c  */
#line 2250 "parser.y"
    {
        (yyval.stmt) = OmpixStmt((yyvsp[(1) - (1)].xcon));
    ;}
    break;

  case 261:

/* Line 1455 of yacc.c  */
#line 2260 "parser.y"
    { (yyval.stmt) = (yyvsp[(1) - (1)].stmt); ;}
    break;

  case 262:

/* Line 1455 of yacc.c  */
#line 2265 "parser.y"
    {
      func_return_flag = 0;
      tmp_float_index = 1; 
      tmp_index = 1;
      num_index = 0;
      num_float_index = 0; 
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
     ;}
    break;

  case 263:

/* Line 1455 of yacc.c  */
#line 2287 "parser.y"
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
           (yyval.stmt)->num_tmp = num_index;
      (yyval.stmt)->num_float_tmp = num_float_index; 
      (yyval.stmt)->contain_priv_if_flag = contain_priv_if_flag;

      if(is_priv_int_ptr_appear == 1)
      {
	(yyval.stmt)->is_priv_int_ptr_appear = 1; 
	is_priv_int_ptr_appear = 0;  
      }
      else 
	(yyval.stmt)->is_priv_int_ptr_appear = 0; 

      if(is_priv_float_ptr_appear == 1)
      {
	(yyval.stmt)->is_priv_float_ptr_appear = 1; 
	is_priv_float_ptr_appear = 0;  
      }
      else 
	(yyval.stmt)->is_priv_float_ptr_appear = 0; 
      
      if(is_priv_int_index_appear == 1)
      {
	(yyval.stmt)->is_priv_int_index_appear = 1; 
	is_priv_int_index_appear = 0;  
      }
      else 
	(yyval.stmt)->is_priv_int_index_appear = 0; 

      if(is_priv_float_index_appear == 1)
      {
	(yyval.stmt)->is_priv_float_index_appear = 1; 
	is_priv_float_index_appear = 0;  
      }
      else 
	(yyval.stmt)->is_priv_float_index_appear = 0; 
      
      if(is_priv_int_struct_field_appear == 1)
      {
	(yyval.stmt)->is_priv_int_struct_field_appear = 1; 
	is_priv_int_struct_field_appear = 0;  
      }
      else 
	(yyval.stmt)->is_priv_int_struct_field_appear = 0; 
      
      if(is_priv_float_struct_field_appear == 1)
      {
	(yyval.stmt)->is_priv_float_struct_field_appear = 1; 
	is_priv_float_struct_field_appear = 0;  
      }
      else 
	(yyval.stmt)->is_priv_float_struct_field_appear = 0; 

      if(is_priv_struct_ptr_struct_field_appear == 1)
      {
	(yyval.stmt)->is_priv_struct_ptr_struct_field_appear = 1; 
	is_priv_struct_ptr_struct_field_appear = 0;  
      }
      else 
	(yyval.stmt)->is_priv_struct_ptr_struct_field_appear = 0; 
    
      if(is_priv_int_ptr_struct_field_appear == 1)
      {
	(yyval.stmt)->is_priv_int_ptr_struct_field_appear = 1; 
	is_priv_int_ptr_struct_field_appear = 0;  
      }
      else 
	(yyval.stmt)->is_priv_int_ptr_struct_field_appear = 0; 
      
      if(is_priv_float_ptr_struct_field_appear == 1)
      {
	(yyval.stmt)->is_priv_float_ptr_struct_field_appear = 1; 
	is_priv_float_ptr_struct_field_appear = 0;  
      }
      else 
	(yyval.stmt)->is_priv_float_ptr_struct_field_appear = 0; 
   ;}
    break;

  case 264:

/* Line 1455 of yacc.c  */
#line 2375 "parser.y"
    {
      tmp_index = 1;
      tmp_float_index = 1; 
      num_index = 0;
      num_float_index = 0; 
      cond_index = 0;
      contain_priv_if_flag = 0;
      if (isTypedef || (yyvsp[(1) - (1)].decl)->decl->type != DFUNC)
        parse_error(1, "function definition cannot be parsed.\n");
      if (symtab_get(stab, decl_getidentifier_symbol((yyvsp[(1) - (1)].decl)), FUNCNAME) == NULL)
        symtab_put_funcname(stab, decl_getidentifier_symbol((yyvsp[(1) - (1)].decl)), FUNCNAME, NULL, (yyvsp[(1) - (1)].decl));
        symbol e = decl_getidentifier_symbol((yyvsp[(1) - (1)].decl));

      scope_start(stab);
      ast_declare_function_params((yyvsp[(1) - (1)].decl));
    ;}
    break;

  case 265:

/* Line 1455 of yacc.c  */
#line 2392 "parser.y"
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
          (yyval.stmt)->num_tmp = num_index;
      (yyval.stmt)->num_float_tmp = num_float_index; 
      (yyval.stmt)->contain_priv_if_flag = contain_priv_if_flag;
      
      if(is_priv_int_ptr_appear == 1)
      {
	(yyval.stmt)->is_priv_int_ptr_appear = 1; 
	is_priv_int_ptr_appear = 0;  
      }
      else 
	(yyval.stmt)->is_priv_int_ptr_appear = 0; 

      if(is_priv_float_ptr_appear == 1)
      {
	(yyval.stmt)->is_priv_float_ptr_appear = 1; 
	is_priv_float_ptr_appear = 0;  
      }
      else 
	(yyval.stmt)->is_priv_float_ptr_appear = 0; 
      
      if(is_priv_int_index_appear == 1)
      {
	(yyval.stmt)->is_priv_int_index_appear = 1; 
	is_priv_int_index_appear = 0;  
      }
      else 
	(yyval.stmt)->is_priv_int_index_appear = 0; 
      
      if(is_priv_float_index_appear == 1)
      {
	(yyval.stmt)->is_priv_float_index_appear = 1; 
	is_priv_float_index_appear = 0;  
      }
      else 
	(yyval.stmt)->is_priv_float_index_appear = 0; 
    ;}
    break;

  case 266:

/* Line 1455 of yacc.c  */
#line 2449 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    ;}
    break;

  case 267:

/* Line 1455 of yacc.c  */
#line 2453 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    ;}
    break;

  case 268:

/* Line 1455 of yacc.c  */
#line 2457 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    ;}
    break;

  case 269:

/* Line 1455 of yacc.c  */
#line 2461 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    ;}
    break;

  case 270:

/* Line 1455 of yacc.c  */
#line 2465 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    ;}
    break;

  case 271:

/* Line 1455 of yacc.c  */
#line 2469 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    ;}
    break;

  case 272:

/* Line 1455 of yacc.c  */
#line 2473 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    ;}
    break;

  case 273:

/* Line 1455 of yacc.c  */
#line 2477 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    ;}
    break;

  case 274:

/* Line 1455 of yacc.c  */
#line 2481 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    ;}
    break;

  case 275:

/* Line 1455 of yacc.c  */
#line 2485 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    ;}
    break;

  case 276:

/* Line 1455 of yacc.c  */
#line 2490 "parser.y"
    {
        (yyval.ocon) = (yyvsp[(1) - (1)].ocon);
    ;}
    break;

  case 277:

/* Line 1455 of yacc.c  */
#line 2506 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCBARRIER, (yyvsp[(1) - (1)].odir), NULL);
    ;}
    break;

  case 278:

/* Line 1455 of yacc.c  */
#line 2510 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCFLUSH, (yyvsp[(1) - (1)].odir), NULL);
    ;}
    break;

  case 279:

/* Line 1455 of yacc.c  */
#line 2515 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCTASKWAIT, (yyvsp[(1) - (1)].odir), NULL);
    ;}
    break;

  case 280:

/* Line 1455 of yacc.c  */
#line 2520 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCTASKYIELD, (yyvsp[(1) - (1)].odir), NULL);
    ;}
    break;

  case 281:

/* Line 1455 of yacc.c  */
#line 2527 "parser.y"
    {
        (yyval.stmt) = (yyvsp[(1) - (1)].stmt);
    ;}
    break;

  case 282:

/* Line 1455 of yacc.c  */
#line 2534 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCPARALLEL, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
        (yyval.ocon)->l = (yyvsp[(1) - (2)].odir)->l;
    ;}
    break;

  case 283:

/* Line 1455 of yacc.c  */
#line 2542 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCPARALLEL, (yyvsp[(3) - (4)].ocla));
    ;}
    break;

  case 284:

/* Line 1455 of yacc.c  */
#line 2549 "parser.y"
    {
        (yyval.ocla) = NULL;
    ;}
    break;

  case 285:

/* Line 1455 of yacc.c  */
#line 2553 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (2)].ocla), (yyvsp[(2) - (2)].ocla));
    ;}
    break;

  case 286:

/* Line 1455 of yacc.c  */
#line 2557 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (3)].ocla), (yyvsp[(3) - (3)].ocla));
    ;}
    break;

  case 287:

/* Line 1455 of yacc.c  */
#line 2564 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    ;}
    break;

  case 288:

/* Line 1455 of yacc.c  */
#line 2568 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    ;}
    break;

  case 289:

/* Line 1455 of yacc.c  */
#line 2574 "parser.y"
    { sc_pause_openmp(); ;}
    break;

  case 290:

/* Line 1455 of yacc.c  */
#line 2575 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = IfClause((yyvsp[(4) - (5)].expr));
    ;}
    break;

  case 291:

/* Line 1455 of yacc.c  */
#line 2579 "parser.y"
    { sc_pause_openmp(); ;}
    break;

  case 292:

/* Line 1455 of yacc.c  */
#line 2580 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = NumthreadsClause((yyvsp[(4) - (5)].expr));
    ;}
    break;

  case 293:

/* Line 1455 of yacc.c  */
#line 2588 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCFOR, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
    ;}
    break;

  case 294:

/* Line 1455 of yacc.c  */
#line 2595 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCFOR, (yyvsp[(3) - (4)].ocla));
    ;}
    break;

  case 295:

/* Line 1455 of yacc.c  */
#line 2602 "parser.y"
    {
        (yyval.ocla) = NULL;
    ;}
    break;

  case 296:

/* Line 1455 of yacc.c  */
#line 2606 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (2)].ocla), (yyvsp[(2) - (2)].ocla));
    ;}
    break;

  case 297:

/* Line 1455 of yacc.c  */
#line 2610 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (3)].ocla), (yyvsp[(3) - (3)].ocla));
    ;}
    break;

  case 298:

/* Line 1455 of yacc.c  */
#line 2617 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    ;}
    break;

  case 299:

/* Line 1455 of yacc.c  */
#line 2621 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    ;}
    break;

  case 300:

/* Line 1455 of yacc.c  */
#line 2625 "parser.y"
    {
        (yyval.ocla) = PlainClause(OCNOWAIT);
    ;}
    break;

  case 301:

/* Line 1455 of yacc.c  */
#line 2632 "parser.y"
    {
        (yyval.ocla) = PlainClause(OCORDERED);
    ;}
    break;

  case 302:

/* Line 1455 of yacc.c  */
#line 2636 "parser.y"
    {
        (yyval.ocla) = ScheduleClause((yyvsp[(3) - (4)].type), NULL);
    ;}
    break;

  case 303:

/* Line 1455 of yacc.c  */
#line 2639 "parser.y"
    { sc_pause_openmp(); ;}
    break;

  case 304:

/* Line 1455 of yacc.c  */
#line 2640 "parser.y"
    {
        sc_start_openmp();
        if ((yyvsp[(3) - (7)].type) == OC_runtime)
        parse_error(1, "\"runtime\" schedules may not have a chunksize.\n");
        (yyval.ocla) = ScheduleClause((yyvsp[(3) - (7)].type), (yyvsp[(6) - (7)].expr));
    ;}
    break;

  case 305:

/* Line 1455 of yacc.c  */
#line 2647 "parser.y"
    {  /* non-OpenMP schedule */
        tempsave = checkDecls;
        checkDecls = 0;   /* Because the index of the loop is usualy involved */
        sc_pause_openmp();
    ;}
    break;

  case 306:

/* Line 1455 of yacc.c  */
#line 2653 "parser.y"
    {
        sc_start_openmp();
        checkDecls = tempsave;
        (yyval.ocla) = ScheduleClause(OC_affinity, (yyvsp[(6) - (7)].expr));
    ;}
    break;

  case 307:

/* Line 1455 of yacc.c  */
#line 2659 "parser.y"
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
    ;}
    break;

  case 308:

/* Line 1455 of yacc.c  */
#line 2674 "parser.y"
    {
        (yyval.type) = OC_static;
    ;}
    break;

  case 309:

/* Line 1455 of yacc.c  */
#line 2678 "parser.y"
    {
        (yyval.type) = OC_dynamic;
    ;}
    break;

  case 310:

/* Line 1455 of yacc.c  */
#line 2682 "parser.y"
    {
        (yyval.type) = OC_guided;
    ;}
    break;

  case 311:

/* Line 1455 of yacc.c  */
#line 2686 "parser.y"
    {
        (yyval.type) = OC_runtime;
    ;}
    break;

  case 312:

/* Line 1455 of yacc.c  */
#line 2690 "parser.y"
    {
        (yyval.type) = OC_auto;
    ;}
    break;

  case 313:

/* Line 1455 of yacc.c  */
#line 2693 "parser.y"
    { parse_error(1, "invalid openmp schedule type.\n"); ;}
    break;

  case 314:

/* Line 1455 of yacc.c  */
#line 2699 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCSECTIONS, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
    ;}
    break;

  case 315:

/* Line 1455 of yacc.c  */
#line 2706 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCSECTIONS, (yyvsp[(3) - (4)].ocla));
    ;}
    break;

  case 316:

/* Line 1455 of yacc.c  */
#line 2713 "parser.y"
    {
        (yyval.ocla) = NULL;
    ;}
    break;

  case 317:

/* Line 1455 of yacc.c  */
#line 2717 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (2)].ocla), (yyvsp[(2) - (2)].ocla));
    ;}
    break;

  case 318:

/* Line 1455 of yacc.c  */
#line 2721 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (3)].ocla), (yyvsp[(3) - (3)].ocla));
    ;}
    break;

  case 319:

/* Line 1455 of yacc.c  */
#line 2728 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    ;}
    break;

  case 320:

/* Line 1455 of yacc.c  */
#line 2732 "parser.y"
    {
        (yyval.ocla) = PlainClause(OCNOWAIT);
    ;}
    break;

  case 321:

/* Line 1455 of yacc.c  */
#line 2739 "parser.y"
    {
        (yyval.stmt) = Compound((yyvsp[(2) - (3)].stmt));
    ;}
    break;

  case 322:

/* Line 1455 of yacc.c  */
#line 2746 "parser.y"
    {
        /* Make it look like it had a section pragma */
        (yyval.stmt) = OmpStmt( OmpConstruct(DCSECTION, OmpDirective(DCSECTION,NULL), (yyvsp[(1) - (1)].stmt)) );
    ;}
    break;

  case 323:

/* Line 1455 of yacc.c  */
#line 2751 "parser.y"
    {
        (yyval.stmt) = OmpStmt( OmpConstruct(DCSECTION, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt)) );
    ;}
    break;

  case 324:

/* Line 1455 of yacc.c  */
#line 2755 "parser.y"
    {
        (yyval.stmt) = BlockList((yyvsp[(1) - (3)].stmt), OmpStmt( OmpConstruct(DCSECTION, (yyvsp[(2) - (3)].odir), (yyvsp[(3) - (3)].stmt)) ));
    ;}
    break;

  case 325:

/* Line 1455 of yacc.c  */
#line 2762 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCSECTION, NULL);
    ;}
    break;

  case 326:

/* Line 1455 of yacc.c  */
#line 2769 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCSINGLE, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
    ;}
    break;

  case 327:

/* Line 1455 of yacc.c  */
#line 2776 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCSINGLE, (yyvsp[(3) - (4)].ocla));
    ;}
    break;

  case 328:

/* Line 1455 of yacc.c  */
#line 2783 "parser.y"
    {
        (yyval.ocla) = NULL;
    ;}
    break;

  case 329:

/* Line 1455 of yacc.c  */
#line 2787 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (2)].ocla), (yyvsp[(2) - (2)].ocla));
    ;}
    break;

  case 330:

/* Line 1455 of yacc.c  */
#line 2791 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (3)].ocla), (yyvsp[(3) - (3)].ocla));
    ;}
    break;

  case 331:

/* Line 1455 of yacc.c  */
#line 2798 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    ;}
    break;

  case 332:

/* Line 1455 of yacc.c  */
#line 2802 "parser.y"
    {
        (yyval.ocla) = PlainClause(OCNOWAIT);
    ;}
    break;

  case 333:

/* Line 1455 of yacc.c  */
#line 2809 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCPARFOR, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
    ;}
    break;

  case 334:

/* Line 1455 of yacc.c  */
#line 2816 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCPARFOR, (yyvsp[(4) - (5)].ocla));
    ;}
    break;

  case 335:

/* Line 1455 of yacc.c  */
#line 2823 "parser.y"
    {
        (yyval.ocla) = NULL;
    ;}
    break;

  case 336:

/* Line 1455 of yacc.c  */
#line 2827 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (2)].ocla), (yyvsp[(2) - (2)].ocla));
    ;}
    break;

  case 337:

/* Line 1455 of yacc.c  */
#line 2831 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (3)].ocla), (yyvsp[(3) - (3)].ocla));
    ;}
    break;

  case 338:

/* Line 1455 of yacc.c  */
#line 2838 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    ;}
    break;

  case 339:

/* Line 1455 of yacc.c  */
#line 2842 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    ;}
    break;

  case 340:

/* Line 1455 of yacc.c  */
#line 2846 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    ;}
    break;

  case 341:

/* Line 1455 of yacc.c  */
#line 2853 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCPARSECTIONS, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
    ;}
    break;

  case 342:

/* Line 1455 of yacc.c  */
#line 2860 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCPARSECTIONS, (yyvsp[(4) - (5)].ocla));
    ;}
    break;

  case 343:

/* Line 1455 of yacc.c  */
#line 2867 "parser.y"
    {
        (yyval.ocla) = NULL;
    ;}
    break;

  case 344:

/* Line 1455 of yacc.c  */
#line 2871 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (2)].ocla), (yyvsp[(2) - (2)].ocla));
    ;}
    break;

  case 345:

/* Line 1455 of yacc.c  */
#line 2875 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (3)].ocla), (yyvsp[(3) - (3)].ocla));
    ;}
    break;

  case 346:

/* Line 1455 of yacc.c  */
#line 2882 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    ;}
    break;

  case 347:

/* Line 1455 of yacc.c  */
#line 2886 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    ;}
    break;

  case 348:

/* Line 1455 of yacc.c  */
#line 2894 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCTASK, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
        (yyval.ocon)->l = (yyvsp[(1) - (2)].odir)->l;
    ;}
    break;

  case 349:

/* Line 1455 of yacc.c  */
#line 2903 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCTASK, (yyvsp[(3) - (4)].ocla));
    ;}
    break;

  case 350:

/* Line 1455 of yacc.c  */
#line 2911 "parser.y"
    {
        (yyval.ocla) = NULL;
    ;}
    break;

  case 351:

/* Line 1455 of yacc.c  */
#line 2915 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (2)].ocla), (yyvsp[(2) - (2)].ocla));
    ;}
    break;

  case 352:

/* Line 1455 of yacc.c  */
#line 2919 "parser.y"
    {
        (yyval.ocla) = OmpClauseList((yyvsp[(1) - (3)].ocla), (yyvsp[(3) - (3)].ocla));
    ;}
    break;

  case 353:

/* Line 1455 of yacc.c  */
#line 2927 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    ;}
    break;

  case 354:

/* Line 1455 of yacc.c  */
#line 2931 "parser.y"
    {
        (yyval.ocla) = (yyvsp[(1) - (1)].ocla);
    ;}
    break;

  case 355:

/* Line 1455 of yacc.c  */
#line 2938 "parser.y"
    { sc_pause_openmp(); ;}
    break;

  case 356:

/* Line 1455 of yacc.c  */
#line 2939 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = IfClause((yyvsp[(4) - (5)].expr));
    ;}
    break;

  case 357:

/* Line 1455 of yacc.c  */
#line 2944 "parser.y"
    {
        (yyval.ocla) = PlainClause(OCUNTIED);
    ;}
    break;

  case 358:

/* Line 1455 of yacc.c  */
#line 2947 "parser.y"
    { sc_pause_openmp(); ;}
    break;

  case 359:

/* Line 1455 of yacc.c  */
#line 2948 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = FinalClause((yyvsp[(4) - (5)].expr));
    ;}
    break;

  case 360:

/* Line 1455 of yacc.c  */
#line 2953 "parser.y"
    {
        (yyval.ocla) = PlainClause(OCMERGEABLE);
    ;}
    break;

  case 361:

/* Line 1455 of yacc.c  */
#line 2959 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCMASTER, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
    ;}
    break;

  case 362:

/* Line 1455 of yacc.c  */
#line 2966 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCMASTER, NULL);
    ;}
    break;

  case 363:

/* Line 1455 of yacc.c  */
#line 2973 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCCRITICAL, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
    ;}
    break;

  case 364:

/* Line 1455 of yacc.c  */
#line 2980 "parser.y"
    {
        (yyval.odir) = OmpCriticalDirective(NULL);
    ;}
    break;

  case 365:

/* Line 1455 of yacc.c  */
#line 2984 "parser.y"
    {
        (yyval.odir) = OmpCriticalDirective((yyvsp[(3) - (4)].symb));
    ;}
    break;

  case 366:

/* Line 1455 of yacc.c  */
#line 2991 "parser.y"
    {
        (yyval.symb) = Symbol((yyvsp[(2) - (3)].name));
    ;}
    break;

  case 367:

/* Line 1455 of yacc.c  */
#line 2999 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCTASKWAIT, NULL);
    ;}
    break;

  case 368:

/* Line 1455 of yacc.c  */
#line 3007 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCTASKYIELD, NULL);
    ;}
    break;

  case 369:

/* Line 1455 of yacc.c  */
#line 3014 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCBARRIER, NULL);
    ;}
    break;

  case 370:

/* Line 1455 of yacc.c  */
#line 3021 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCATOMIC, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
    ;}
    break;

  case 371:

/* Line 1455 of yacc.c  */
#line 3028 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCATOMIC, NULL);
    ;}
    break;

  case 372:

/* Line 1455 of yacc.c  */
#line 3032 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCATOMIC, NULL);
    ;}
    break;

  case 373:

/* Line 1455 of yacc.c  */
#line 3036 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCATOMIC, NULL);
    ;}
    break;

  case 374:

/* Line 1455 of yacc.c  */
#line 3040 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCATOMIC, NULL);
    ;}
    break;

  case 375:

/* Line 1455 of yacc.c  */
#line 3047 "parser.y"
    {
        (yyval.odir) = OmpFlushDirective(NULL);
    ;}
    break;

  case 376:

/* Line 1455 of yacc.c  */
#line 3051 "parser.y"
    {
        (yyval.odir) = OmpFlushDirective((yyvsp[(3) - (4)].decl));
    ;}
    break;

  case 377:

/* Line 1455 of yacc.c  */
#line 3057 "parser.y"
    { sc_pause_openmp(); ;}
    break;

  case 378:

/* Line 1455 of yacc.c  */
#line 3058 "parser.y"
    {
        sc_start_openmp();
        (yyval.decl) = (yyvsp[(3) - (4)].decl);
    ;}
    break;

  case 379:

/* Line 1455 of yacc.c  */
#line 3066 "parser.y"
    {
        (yyval.ocon) = OmpConstruct(DCORDERED, (yyvsp[(1) - (2)].odir), (yyvsp[(2) - (2)].stmt));
    ;}
    break;

  case 380:

/* Line 1455 of yacc.c  */
#line 3073 "parser.y"
    {
        (yyval.odir) = OmpDirective(DCORDERED, NULL);
    ;}
    break;

  case 381:

/* Line 1455 of yacc.c  */
#line 3080 "parser.y"
    {
        (yyval.odir) = OmpThreadprivateDirective((yyvsp[(3) - (5)].decl));
    ;}
    break;

  case 382:

/* Line 1455 of yacc.c  */
#line 3086 "parser.y"
    { sc_pause_openmp(); ;}
    break;

  case 383:

/* Line 1455 of yacc.c  */
#line 3087 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = VarlistClause(OCPRIVATE, (yyvsp[(4) - (5)].decl));
    ;}
    break;

  case 384:

/* Line 1455 of yacc.c  */
#line 3092 "parser.y"
    { sc_pause_openmp(); ;}
    break;

  case 385:

/* Line 1455 of yacc.c  */
#line 3093 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = VarlistClause(OCCOPYPRIVATE, (yyvsp[(4) - (5)].decl));
    ;}
    break;

  case 386:

/* Line 1455 of yacc.c  */
#line 3097 "parser.y"
    { sc_pause_openmp(); ;}
    break;

  case 387:

/* Line 1455 of yacc.c  */
#line 3098 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = VarlistClause(OCFIRSTPRIVATE, (yyvsp[(4) - (5)].decl));
    ;}
    break;

  case 388:

/* Line 1455 of yacc.c  */
#line 3102 "parser.y"
    { sc_pause_openmp(); ;}
    break;

  case 389:

/* Line 1455 of yacc.c  */
#line 3103 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = VarlistClause(OCLASTPRIVATE, (yyvsp[(4) - (5)].decl));
    ;}
    break;

  case 390:

/* Line 1455 of yacc.c  */
#line 3107 "parser.y"
    { sc_pause_openmp(); ;}
    break;

  case 391:

/* Line 1455 of yacc.c  */
#line 3108 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = VarlistClause(OCSHARED, (yyvsp[(4) - (5)].decl));
    ;}
    break;

  case 392:

/* Line 1455 of yacc.c  */
#line 3113 "parser.y"
    {
        (yyval.ocla) = DefaultClause(OC_defshared);
    ;}
    break;

  case 393:

/* Line 1455 of yacc.c  */
#line 3117 "parser.y"
    {
        (yyval.ocla) = DefaultClause(OC_defnone);
    ;}
    break;

  case 394:

/* Line 1455 of yacc.c  */
#line 3120 "parser.y"
    { sc_pause_openmp(); ;}
    break;

  case 395:

/* Line 1455 of yacc.c  */
#line 3121 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = ReductionClause((yyvsp[(3) - (7)].type), (yyvsp[(6) - (7)].decl));
    ;}
    break;

  case 396:

/* Line 1455 of yacc.c  */
#line 3125 "parser.y"
    { sc_pause_openmp(); ;}
    break;

  case 397:

/* Line 1455 of yacc.c  */
#line 3126 "parser.y"
    {
        sc_start_openmp();
        (yyval.ocla) = VarlistClause(OCCOPYIN, (yyvsp[(4) - (5)].decl));
    ;}
    break;

  case 398:

/* Line 1455 of yacc.c  */
#line 3134 "parser.y"
    {
        (yyval.type) = OC_plus;
    ;}
    break;

  case 399:

/* Line 1455 of yacc.c  */
#line 3138 "parser.y"
    {
        (yyval.type) = OC_times;
    ;}
    break;

  case 400:

/* Line 1455 of yacc.c  */
#line 3142 "parser.y"
    {
        (yyval.type) = OC_minus;
    ;}
    break;

  case 401:

/* Line 1455 of yacc.c  */
#line 3146 "parser.y"
    {
        (yyval.type) = OC_band;
    ;}
    break;

  case 402:

/* Line 1455 of yacc.c  */
#line 3150 "parser.y"
    {
        (yyval.type) = OC_xor;
    ;}
    break;

  case 403:

/* Line 1455 of yacc.c  */
#line 3154 "parser.y"
    {
        (yyval.type) = OC_bor;
    ;}
    break;

  case 404:

/* Line 1455 of yacc.c  */
#line 3158 "parser.y"
    {
        (yyval.type) = OC_land;
    ;}
    break;

  case 405:

/* Line 1455 of yacc.c  */
#line 3162 "parser.y"
    {
        (yyval.type) = OC_lor;
    ;}
    break;

  case 406:

/* Line 1455 of yacc.c  */
#line 3166 "parser.y"
    {
        (yyval.type) = OC_min;
    ;}
    break;

  case 407:

/* Line 1455 of yacc.c  */
#line 3170 "parser.y"
    {
        (yyval.type) = OC_max;
    ;}
    break;

  case 408:

/* Line 1455 of yacc.c  */
#line 3177 "parser.y"
    {
        if (checkDecls)
        if (symtab_get(stab, Symbol((yyvsp[(1) - (1)].name)), IDNAME) == NULL)
            parse_error(-1, "unknown identifier `%s'.\n", (yyvsp[(1) - (1)].name));
        (yyval.decl) = IdentifierDecl( Symbol((yyvsp[(1) - (1)].name)) );
    ;}
    break;

  case 409:

/* Line 1455 of yacc.c  */
#line 3184 "parser.y"
    {
        if (checkDecls)
        if (symtab_get(stab, Symbol((yyvsp[(3) - (3)].name)), IDNAME) == NULL)
            parse_error(-1, "unknown identifier `%s'.\n", (yyvsp[(3) - (3)].name));
        (yyval.decl) = IdList((yyvsp[(1) - (3)].decl), IdentifierDecl( Symbol((yyvsp[(3) - (3)].name)) ));
    ;}
    break;

  case 410:

/* Line 1455 of yacc.c  */
#line 3199 "parser.y"
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
    ;}
    break;

  case 411:

/* Line 1455 of yacc.c  */
#line 3217 "parser.y"
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
    ;}
    break;

  case 412:

/* Line 1455 of yacc.c  */
#line 3243 "parser.y"
    {
        (yyval.xcon) = OmpixConstruct(OX_DCTASKSYNC, (yyvsp[(1) - (1)].xdir), NULL);
    ;}
    break;

  case 413:

/* Line 1455 of yacc.c  */
#line 3247 "parser.y"
    {
        (yyval.xcon) = OmpixConstruct(OX_DCTASKSCHEDULE, (yyvsp[(1) - (1)].xdir), NULL);
    ;}
    break;

  case 414:

/* Line 1455 of yacc.c  */
#line 3255 "parser.y"
    {
        (yyval.xdir) = OmpixDirective(OX_DCTASKSYNC, NULL);
    ;}
    break;

  case 415:

/* Line 1455 of yacc.c  */
#line 3262 "parser.y"
    {
        scope_start(stab);
    ;}
    break;

  case 416:

/* Line 1455 of yacc.c  */
#line 3266 "parser.y"
    {
        scope_end(stab);
        (yyval.xdir) = OmpixDirective(OX_DCTASKSCHEDULE, (yyvsp[(4) - (5)].xcla));
    ;}
    break;

  case 417:

/* Line 1455 of yacc.c  */
#line 3274 "parser.y"
    {
        (yyval.xcla) = NULL;
    ;}
    break;

  case 418:

/* Line 1455 of yacc.c  */
#line 3278 "parser.y"
    {
        (yyval.xcla) = OmpixClauseList((yyvsp[(1) - (2)].xcla), (yyvsp[(2) - (2)].xcla));
    ;}
    break;

  case 419:

/* Line 1455 of yacc.c  */
#line 3282 "parser.y"
    {
        (yyval.xcla) = OmpixClauseList((yyvsp[(1) - (3)].xcla), (yyvsp[(3) - (3)].xcla));
    ;}
    break;

  case 420:

/* Line 1455 of yacc.c  */
#line 3289 "parser.y"
    {
        (yyval.xcla) = OmpixStrideClause((yyvsp[(3) - (4)].expr));
    ;}
    break;

  case 421:

/* Line 1455 of yacc.c  */
#line 3293 "parser.y"
    {
        (yyval.xcla) = OmpixStartClause((yyvsp[(3) - (4)].expr));
    ;}
    break;

  case 422:

/* Line 1455 of yacc.c  */
#line 3297 "parser.y"
    {
        (yyval.xcla) = OmpixScopeClause((yyvsp[(3) - (4)].type));
    ;}
    break;

  case 423:

/* Line 1455 of yacc.c  */
#line 3301 "parser.y"
    {
        (yyval.xcla) = OmpixPlainClause(OX_OCTIED);
    ;}
    break;

  case 424:

/* Line 1455 of yacc.c  */
#line 3305 "parser.y"
    {
        (yyval.xcla) = OmpixPlainClause(OX_OCUNTIED);
    ;}
    break;

  case 425:

/* Line 1455 of yacc.c  */
#line 3312 "parser.y"
    {
        (yyval.type) = OX_SCOPE_NODES;
    ;}
    break;

  case 426:

/* Line 1455 of yacc.c  */
#line 3316 "parser.y"
    {
        (yyval.type) = OX_SCOPE_WGLOBAL;
    ;}
    break;

  case 427:

/* Line 1455 of yacc.c  */
#line 3320 "parser.y"
    {
        (yyval.type) = OX_SCOPE_WGLOBAL;
    ;}
    break;

  case 428:

/* Line 1455 of yacc.c  */
#line 3324 "parser.y"
    {
        (yyval.type) = OX_SCOPE_WLOCAL;
    ;}
    break;

  case 429:

/* Line 1455 of yacc.c  */
#line 3331 "parser.y"
    {
        (yyval.xcon) = (yyvsp[(1) - (1)].xcon);
    ;}
    break;

  case 430:

/* Line 1455 of yacc.c  */
#line 3335 "parser.y"
    {
        (yyval.xcon) = (yyvsp[(1) - (1)].xcon);
    ;}
    break;

  case 431:

/* Line 1455 of yacc.c  */
#line 3343 "parser.y"
    {
        /* Should put the name of the callback function in the stab, too
     if (symtab_get(stab, decl_getidentifier_symbol($2->u.declaration.decl),
     FUNCNAME) == NULL)
     symtab_put(stab, decl_getidentifier_symbol($2->u.declaration.spec),
     FUNCNAME);
     */
        scope_start(stab);   /* re-declare the arguments of the task function */
        ast_declare_function_params((yyvsp[(2) - (2)].stmt)->u.declaration.decl);
    ;}
    break;

  case 432:

/* Line 1455 of yacc.c  */
#line 3354 "parser.y"
    {
        scope_end(stab);
        (yyval.xcon) = OmpixTaskdef((yyvsp[(1) - (4)].xdir), (yyvsp[(2) - (4)].stmt), (yyvsp[(4) - (4)].stmt));
        (yyval.xcon)->l = (yyvsp[(1) - (4)].xdir)->l;
    ;}
    break;

  case 433:

/* Line 1455 of yacc.c  */
#line 3360 "parser.y"
    {
        (yyval.xcon) = OmpixTaskdef((yyvsp[(1) - (2)].xdir), (yyvsp[(2) - (2)].stmt), NULL);
        (yyval.xcon)->l = (yyvsp[(1) - (2)].xdir)->l;
    ;}
    break;

  case 434:

/* Line 1455 of yacc.c  */
#line 3368 "parser.y"
    {
        scope_start(stab);
    ;}
    break;

  case 435:

/* Line 1455 of yacc.c  */
#line 3372 "parser.y"
    {
        scope_end(stab);
        (yyval.xdir) = OmpixDirective(OX_DCTASKDEF, (yyvsp[(4) - (5)].xcla));
    ;}
    break;

  case 436:

/* Line 1455 of yacc.c  */
#line 3380 "parser.y"
    {
        (yyval.xcla) = NULL;
    ;}
    break;

  case 437:

/* Line 1455 of yacc.c  */
#line 3384 "parser.y"
    {
        (yyval.xcla) = OmpixClauseList((yyvsp[(1) - (2)].xcla), (yyvsp[(2) - (2)].xcla));
    ;}
    break;

  case 438:

/* Line 1455 of yacc.c  */
#line 3388 "parser.y"
    {
        (yyval.xcla) = OmpixClauseList((yyvsp[(1) - (3)].xcla), (yyvsp[(3) - (3)].xcla));
    ;}
    break;

  case 439:

/* Line 1455 of yacc.c  */
#line 3395 "parser.y"
    {
        (yyval.xcla) = OmpixVarlistClause(OX_OCIN, (yyvsp[(3) - (4)].decl));
    ;}
    break;

  case 440:

/* Line 1455 of yacc.c  */
#line 3399 "parser.y"
    {
        (yyval.xcla) = OmpixVarlistClause(OX_OCOUT, (yyvsp[(3) - (4)].decl));
    ;}
    break;

  case 441:

/* Line 1455 of yacc.c  */
#line 3403 "parser.y"
    {
        (yyval.xcla) = OmpixVarlistClause(OX_OCINOUT, (yyvsp[(3) - (4)].decl));
    ;}
    break;

  case 442:

/* Line 1455 of yacc.c  */
#line 3407 "parser.y"
    {
        (yyval.xcla) = OmpixReductionClause((yyvsp[(3) - (6)].type), (yyvsp[(5) - (6)].decl));
    ;}
    break;

  case 443:

/* Line 1455 of yacc.c  */
#line 3414 "parser.y"
    {
        (yyval.decl) = (yyvsp[(1) - (1)].decl);
    ;}
    break;

  case 444:

/* Line 1455 of yacc.c  */
#line 3418 "parser.y"
    {
        (yyval.decl) = IdList((yyvsp[(1) - (3)].decl), (yyvsp[(3) - (3)].decl));
    ;}
    break;

  case 445:

/* Line 1455 of yacc.c  */
#line 3425 "parser.y"
    {
        (yyval.decl) = IdentifierDecl( Symbol((yyvsp[(1) - (1)].name)) );
        symtab_put(stab, Symbol((yyvsp[(1) - (1)].name)), IDNAME);
    ;}
    break;

  case 446:

/* Line 1455 of yacc.c  */
#line 3430 "parser.y"
    {
        if (checkDecls) check_uknown_var((yyvsp[(4) - (5)].name));
        /* Use extern to differentiate */
            (yyval.decl) = ArrayDecl(IdentifierDecl( Symbol((yyvsp[(1) - (5)].name)) ), StClassSpec(SPEC_extern),
        Identifier(Symbol((yyvsp[(4) - (5)].name))));
        symtab_put(stab, Symbol((yyvsp[(1) - (5)].name)), IDNAME);
    ;}
    break;

  case 447:

/* Line 1455 of yacc.c  */
#line 3438 "parser.y"
    {
        (yyval.decl) = ArrayDecl(IdentifierDecl( Symbol((yyvsp[(1) - (4)].name)) ), NULL, (yyvsp[(3) - (4)].expr));
        symtab_put(stab, Symbol((yyvsp[(1) - (4)].name)), IDNAME);
    ;}
    break;

  case 448:

/* Line 1455 of yacc.c  */
#line 3446 "parser.y"
    {
        (yyval.xcon) = OmpixConstruct(OX_DCTASK, (yyvsp[(1) - (3)].xdir), Expression((yyvsp[(2) - (3)].expr)));
        (yyval.xcon)->l = (yyvsp[(1) - (3)].xdir)->l;
    ;}
    break;

  case 449:

/* Line 1455 of yacc.c  */
#line 3454 "parser.y"
    {
        (yyval.xdir) = OmpixDirective(OX_DCTASK, (yyvsp[(3) - (4)].xcla));
    ;}
    break;

  case 450:

/* Line 1455 of yacc.c  */
#line 3461 "parser.y"
    {
        (yyval.xcla) = NULL;
    ;}
    break;

  case 451:

/* Line 1455 of yacc.c  */
#line 3465 "parser.y"
    {
        (yyval.xcla) = OmpixClauseList((yyvsp[(1) - (2)].xcla), (yyvsp[(2) - (2)].xcla));
    ;}
    break;

  case 452:

/* Line 1455 of yacc.c  */
#line 3469 "parser.y"
    {
        (yyval.xcla) = OmpixClauseList((yyvsp[(1) - (3)].xcla), (yyvsp[(3) - (3)].xcla));
    ;}
    break;

  case 453:

/* Line 1455 of yacc.c  */
#line 3476 "parser.y"
    {
        (yyval.xcla) = OmpixPlainClause(OX_OCATALL);
    ;}
    break;

  case 454:

/* Line 1455 of yacc.c  */
#line 3480 "parser.y"
    {
        (yyval.xcla) = OmpixAtnodeClause((yyvsp[(3) - (4)].expr));
    ;}
    break;

  case 455:

/* Line 1455 of yacc.c  */
#line 3484 "parser.y"
    {
        (yyval.xcla) = OmpixAtworkerClause((yyvsp[(3) - (4)].expr));
    ;}
    break;

  case 456:

/* Line 1455 of yacc.c  */
#line 3488 "parser.y"
    {
        (yyval.xcla) = OmpixPlainClause(OX_OCTIED);
    ;}
    break;

  case 457:

/* Line 1455 of yacc.c  */
#line 3492 "parser.y"
    {
        (yyval.xcla) = OmpixPlainClause(OX_OCUNTIED);
    ;}
    break;

  case 458:

/* Line 1455 of yacc.c  */
#line 3496 "parser.y"
    {
        (yyval.xcla) = OmpixPlainClause(OX_OCDETACHED);
    ;}
    break;

  case 459:

/* Line 1455 of yacc.c  */
#line 3503 "parser.y"
    {
        (yyval.expr) = strcmp((yyvsp[(1) - (3)].name), "main") ?
        FunctionCall(Identifier(Symbol((yyvsp[(1) - (3)].name))), NULL) :
        FunctionCall(Identifier(Symbol(MAIN_NEWNAME)), NULL);
    ;}
    break;

  case 460:

/* Line 1455 of yacc.c  */
#line 3509 "parser.y"
    {
        (yyval.expr) = strcmp((yyvsp[(1) - (4)].name), "main") ?
        FunctionCall(Identifier(Symbol((yyvsp[(1) - (4)].name))), (yyvsp[(3) - (4)].expr)) :
        FunctionCall(Identifier(Symbol(MAIN_NEWNAME)), (yyvsp[(3) - (4)].expr));
    ;}
    break;



/* Line 1455 of yacc.c  */
#line 7543 "parser.tab.c"
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
      /* If just tried and failed to reuse lookahead token after an
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

  /* Else will try to reuse lookahead token after shifting the error
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

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
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



/* Line 1675 of yacc.c  */
#line 3516 "parser.y"



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

void add_declaration_links(astspec s, astdecl d, int is_su_field)
{
  astdecl ini = NULL;
  if (d->type == DLIST && d->subtype == DECL_decllist)
  {
    add_declaration_links(s, d->u.next, is_su_field);
    d = d->decl;
  }
  if (d->type == DINIT) d = (ini = d)->decl;   /* Skip the initializer */
  assert(d->type == DECLARATOR);
  if (d->decl != NULL && d->decl->type != ABSDECLARATOR)
  {
    symbol  t = decl_getidentifier_symbol(d->decl);
    stentry e; 
    e = isTypedef ?
                symtab_get(stab,t,TYPENAME) :
                symtab_get(stab,t,(decl_getkind(d)==DFUNC) ? FUNCNAME : IDNAME);
    e->spec  = s;
    e->decl  = d;
    e->idecl = ini;
  }
   
}
int check_decl_for_pointer(astdecl d)
{
    if(d->type == DINIT)
    {
        if(d->decl->spec)
            return 1;
        else
            return 0;
    }
    if(d->type == DECLARATOR)
    {
        if(d->spec)
            return 1;
        else
            return 0;
    }
}

void store_struct_information(struct_node_stack sns, astspec s)
{
	struct_node_push(sns, s);
	struct_node_update(sns, s->u.decl);
	//fill the struct fild information into each struct node
        astdecl tmp = NULL;
	astdecl tree = s->u.decl; 
        if(tree->type == DLIST)
                tmp = tree->decl;
        else
                tmp = tree;
        while(tmp != NULL)
        {
		struct_field_push(sns->head->fieldlist, tmp->spec, tmp->decl);  
		if(tree->type == DLIST)
                {
                        tree = tree->u.next;
                        if(tree->type == DLIST)
                                tmp = tree->decl;
                        else
                                tmp = tree;
                }
                else
                        break;
        }
}

void set_global_tags_for_private_struct_field(astspec s, astdecl d)
{
 	if(s->subtype == SPEC_int || s->subtype == SPEC_Rlist && (s->body->subtype == SPEC_private && s->u.next->subtype == SPEC_int))
	{
		if(d->spec)
		{
			if(!is_priv_int_ptr_struct_field_appear)	
				is_priv_int_ptr_struct_field_appear = 1; 
		}
		else
		{
			if(!is_priv_int_struct_field_appear)	
				is_priv_int_struct_field_appear = 1; 
		}
	}
 	if(s->subtype == SPEC_float || s->subtype == SPEC_Rlist && (s->body->subtype == SPEC_private && s->u.next->subtype == SPEC_float))
	{
		if(d->spec)
		{
			if(!is_priv_float_ptr_struct_field_appear)	
				is_priv_float_ptr_struct_field_appear = 1; 
		}
		else
		{
			if(!is_priv_float_struct_field_appear)	
				is_priv_float_struct_field_appear = 1; 
		}
	}
	if(s->subtype == SPEC_struct || s->subtype == SPEC_union)
	{
		if(d->spec)
		{
			if(!is_priv_struct_ptr_struct_field_appear)
				is_priv_struct_ptr_struct_field_appear = 1; 
		}
	}
}

struct_field get_struct_field_info(astexpr e)
{
	struct_node node;
        struct_field field;
	while(e->type == ARRAYIDX)
		e = e->left;
        if(e->left->type != PTRFIELD && e->left->type != DOTFIELD)
        {
                stentry entry = symtab_get(stab, e->left->u.sym, IDNAME); 
		node = struct_node_lookup(struct_table, entry->spec->name->name);
                field = struct_field_lookup(node, e->u.sym->name);
		if(!node->contain_pub_field)
			set_global_tags_for_private_struct_field(field->type, field->name); 
        }
        else
        {
                struct_field f = get_struct_field_info(e->left);
                node = struct_node_lookup(struct_table, f->type->name->name);
                field = struct_field_lookup(node, e->u.sym->name);
		if(!node->contain_pub_field)
			set_global_tags_for_private_struct_field(field->type, field->name); 
        }
        return field;
}

void set_identifier_attributes(symbol id, astexpr expr, int is_su_field)
{
      stentry entry;
      astspec s = NULL; 
      astdecl d = NULL;
      int isarray = 0;
      struct_field field;
      if(is_su_field)
      {
		field = get_struct_field_info(expr);
	 	s = field->type; 
		d = field->name;
      }
      else
      {
		entry = symtab_get(stab, id, IDNAME);
		s = entry->spec; 
		d = entry->decl; 
      }
 
      set_security_flag_symbol(expr, id, is_su_field);
      set_size_symbol(expr, Identifier(id), NULL);
      // for no "private " declaration
      if(s->subtype == SPEC_int)
      {
		id->type = 0;
		//set the bitlength of variable size
		expr->size = s->size;  
      }
      if(s->subtype == SPEC_float)
      {
		id->type = 1;
		//set the bitlength of significand and exponent
		expr->size = s->size; 
		expr->sizeexp = s->sizeexp; 
      }
      if(s->subtype == SPEC_struct || s->subtype == SPEC_union)
		id->type = 2; 
      // for "private " and "public "declaration
      if(s->subtype == SPEC_Rlist){// && s->body->subtype == SPEC_private){
      	if(s->body->subtype == SPEC_private){
		if(s->u.next->subtype == SPEC_int)
		{
			id->type = 0;
			expr->size = s->u.next->size; 
		}
      		else if(s->u.next->subtype == SPEC_float)
		{
			id->type = 1;
			expr->size = s->u.next->size; 
			expr->sizeexp = s->u.next->sizeexp; 
		}
      	}else{
		if(s->u.next->subtype == SPEC_int)
                {
                        id->type = 0;
                        expr->size = -1;
                }
                else if(s->u.next->subtype == SPEC_float)
                {
                        id->type = 1;
                        expr->size = -1;
                        expr->sizeexp = -1;
                }
	}
      }
      // checks for pointer
      if(d->spec)
	id->isptr = compute_ptr_level(d);
      else
        id->isptr = 0;
      id->struct_type = s; // store the type of struct here.    
      expr->ftype = id->type; 
      if(id->isptr > 0)
        expr->isptr = id->isptr;
}

void set_pointer_flag(astspec spec, astdecl decl)
{
    int is_int = 0;
    int is_float = 0;
    int is_private = 0;
    int is_pointer = 0;
    
    //determine if the spec is private or public and int or float
    if(spec->subtype == SPEC_int|| ((spec->subtype == SPEC_Rlist) && (spec->body->subtype == SPEC_private) && (spec->u.next->subtype == SPEC_int)))
    {
        is_int = 1;
        is_private = 1;
    }
    
    else if(spec->subtype == SPEC_float|| ((spec->subtype == SPEC_Rlist) && (spec->body->subtype == SPEC_private) && (spec->u.next->subtype == SPEC_float)))
    {
        is_float = 1;
        is_private = 1;
    }
    //determine if the decl is pointer
    astdecl d1, d2;
    d1 = decl;
    while(d1->type == DLIST)
    {
        d2 = d1->decl;
        //examine if d2 is type of pointer
        if(check_decl_for_pointer(d2) == 1)
        {
            is_pointer = 1;
            break;
        }
        d1 = d1->u.next;
    }
    //examine if d1 is type of pointer
    if(check_decl_for_pointer(d1) == 1)
        is_pointer = 1;
        
    if(is_int == 1 && is_private == 1 && is_pointer == 1)
        is_priv_int_ptr_appear = 1;
    if(is_float == 1 && is_private == 1 && is_pointer == 1)
        is_priv_float_ptr_appear = 1;
}

void set_security_flag_symbol(astexpr e, symbol s, int is_su_field)
{
      stentry entry; 	
      astspec spec = NULL;
      astdecl decl = NULL;
      struct_field field;

      if(is_su_field)
      {
                field = get_struct_field_info(e);
                spec = field->type;
                decl = field->name;
      }
      else
      {
                entry = symtab_get(stab, s, IDNAME);
                spec = entry->spec;
                decl = entry->decl;
      }
	
      if(spec->body == NULL)
      {
		if(spec->subtype == SPEC_int || spec->subtype == SPEC_float)
		{
			e->flag = PRI; 
			e->index = 0; 
		}
		//for struct and union type
		else if(spec->subtype == SPEC_struct || spec->subtype == SPEC_union)
		{
			struct_node node = struct_node_lookup(struct_table, spec->name->name); 
			if(!node->contain_pub_field)
			{
				e->flag = PRI; 
				e->index = 0;
			}
			else
			{
				e->flag = PUB; 
				e->index = 0; 
			}
		}
		else
		{
			e->flag = PUB; 
			e->index = -1; 
		}	
     }
     else if(spec->body->subtype == SPEC_private)
     {
		e->flag = PRI; 
		e->index = 0; 
     }
     else
     {
		e->flag = PUB; 
		e->index = -1; 
     }
}

void set_gloabl_tags_for_private_struct_field(astspec s)
{
	
}
void set_bitlength_expr(astexpr e, astexpr e1, astexpr e2)
{
	if(e2 == NULL)
	{
		e->size = e1->size; 
		e->sizeexp = e1->sizeexp; 		
	}
	else 
	{
		if(e1->ftype != e2->ftype)
		{
			parse_error(-1, "operators should have the same type...\n"); 
		}
		if(e1->ftype == 0)
		{
			if(e1->size >= e2->size)
				e->size = e1->size; 
			else
				e->size = e2->size; 
		}
		else if(e1->ftype == 1)
		{
			if(e1->size >= e2->size)
				e->size = e1->size; 
			else
				e->size = e2->size; 
			
			if(e1->sizeexp >= e2->sizeexp)
				e->sizeexp = e1->sizeexp; 
			else
				e->sizeexp = e2->sizeexp; 
		}
	}	
}

void set_security_flag_expr(astexpr e, astexpr e1, astexpr e2, int opid){
    //BOP
    if(e2 != NULL && e1 != NULL)
    {
        if(e1->flag == PUB && e2->flag == PUB)
	{
            e->flag = PUB;
            e->index = -1;
        }
        else
	{
                e->flag = PRI;
                if(e->ftype == 0)
                	e->index = tmp_index;
		else
			e->index = tmp_float_index; 
                // assume e1 and e2 are arrays and have the same size
                if(e1->arraysize != NULL && e2->arraysize != NULL)
                        e->arraysize = ast_expr_copy(e1->arraysize);
			//e->arraysize = e1->arraysize; 
        	  
	}
	compute_modulus_for_BOP(e1, e2, opid); 
    }
    //() or UOP or ASS
    else if(e2 == NULL && e1 != NULL)
    {
          e->flag = e1->flag;
          e->index = e1->index; 
    }
    //const
    else if(e1 == NULL && e2 == NULL)
    {
        e->flag = PUB;
        e->index = -1; 
    }
 
   //COMPUTE THE MODULUS FOR DIFFERENT OPERATIONS AND DIFFERENT TYPES OF PRIVATE VARIABLES
} 

void security_check_for_assignment(astexpr le, astexpr re){
	if(le->flag == PUB && re->flag == PRI) 
		parse_error(-1, "security type mismatch in the assignment\n");
} 

void security_check_for_declaration(astspec spec, astdecl decl){
 
    int flag1 = 0;
    int flag2 = 0;
    
    // specifier
    if(spec->type == SPECLIST && spec->body->subtype == SPEC_private)
        flag1 = 1;
    else if(spec->subtype == SPEC_int || spec->subtype == SPEC_float)
	flag1 = 1; 
    else if(spec->subtype == SPEC_struct || spec->subtype == SPEC_union)
    {
	struct_node node = struct_node_lookup(struct_table, spec->name->name); 
	if(!node->contain_pub_field)
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
                parse_error(-1, "A: security type mismatch in the assignment\n");
            decl = decl->u.next;
        }
        
        if(decl->type == DINIT)
        {
            if(decl->u.expr->flag == PRI)
                flag2 = 1;
            
            if(flag1 == 0 && flag2 == 1)
                parse_error(-1, "B: security type mismatch in the assignment\n");
        }
    
    }
    else{
        if(decl->type == DINIT)
        {
            if(decl->u.expr->flag == PRI)
                flag2 = 1;
            if(flag1 == 0 && flag2 == 1)
                parse_error(-1, "C: security type mismatch in the assignment\n");
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
    if(spec->type == SUE)
	return PUB;
    else if(spec->type == SPECLIST){
        if(spec->body->subtype == SPEC_public)
            return PUB;
        else if(spec->body->subtype == SPEC_private)
            return PRI;
    }
    else
        return PRI;
}

void set_size_symbol(astexpr e1, astexpr e2, astexpr e3){
    stentry entry;
    astexpr e = e2;
    astexpr e4 = e1; 
    astdecl d, d1;
    int num = 0;
    if(e->type != IDENT)
        while(e->type != IDENT)
            e = e->left;
    /***********************************************/
     astspec spec = NULL;
     astdecl decl = NULL;
     int isarray = 0;
     struct_field field;
     int is_struct_field = 0;
     /*determine if it is the struct field*/ 
     while(e4->type == ARRAYIDX)
	e4 = e4->left; 
     if(e4->type == PTRFIELD || e4->type == DOTFIELD)
		is_struct_field = 1; 
      /*************************************/
     if(is_struct_field)
      {
                field = get_struct_field_info(e1);
                spec = field->type;
                decl = field->name;
		isarray =(decl_getkind(decl) == DARRAY);
      }
      else
      {
		entry = symtab_get(stab, e->u.sym, IDNAME);
                spec = entry->spec;
                decl = entry->decl;
		isarray = entry->isarray; 
      }
    /**************************************************/
    // set the length of expr
    if(set_security_flag_spec(spec) == PUB){
        e1->size = -1; 
	e1->sizeexp = -1; 
    }
    else if(spec->type == SPECLIST){
        e1->size = spec->u.next->size;
	e1->sizeexp  = spec->u.next->sizeexp; 
    }
    else{
        e1->size = spec->size;
	e1->sizeexp = spec->sizeexp; 
    }
    // if e2 is an array, we further store its size
    if(isarray){
        d = decl->decl;
        // for one dimension array
        if(e2->type == IDENT){
		//e1->arraysize = d->u.expr;
	        e1->arraysize = ast_expr_copy(d->u.expr);
		//set the arraytype
		if(e3 == NULL || (e3 != NULL && d->decl->type == DARRAY))
			e1->arraytype = 1; 
	}
        // for two dimension array
	else if (e2->type == ARRAYIDX){
	    //e1->arraysize = decl->decl->u.expr; 
	    //e2->arraysize = decl->decl->decl->u.expr; 
       	    e1->arraysize = ast_expr_copy(decl->decl->u.expr);
            e2->arraysize = ast_expr_copy(decl->decl->decl->u.expr);
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

int compute_ptr_level(astdecl tree)
{
	int level = 1; 
        astdecl tmp = tree;
        astspec spec = tree->spec;
        while(spec->type == SPECLIST && spec->body->subtype == SPEC_star)
        {
                level++;
                spec = spec->u.next;
        }
        return level;
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
astdecl fix_known_typename(astspec s)
{
  astspec prev;
  astdecl d;
  
  if (s->type != SPECLIST || s->u.next->type != SPECLIST) return (NULL);
  
  for (; s->u.next->type == SPECLIST; prev = s, s = s->u.next)  
    ;     if (s->u.next->type != USERTYPE)
    return (NULL);
    
  prev->u.next = s->body;
  
  d = Declarator(NULL, IdentifierDecl(s->u.next->name));
  if (checkDecls) 
    symtab_put(stab, s->u.next->name, TYPENAME);
  free(s);
  return (d);
}

void get_arraysize(astexpr op, str arg_str)
{
    stentry e;
    if(op->type != CONSTVAL)
    {
        e = get_entry_from_expr(op);
        stentry e1 = symtab_get(stab, e->key, IDNAME);
        ast_expr_print(arg_str, e1->idecl->u.expr);
    }
    else
        ast_expr_print(arg_str, op);
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
    if((e->u.sym == NULL || e->type == CASTEXPR) && e->flag == PRI){
        if(e->ftype == 0)
		tmp_index++;
	else
		tmp_float_index++; 
    }
    num_index = num_index > tmp_index? num_index: tmp_index;
    num_float_index = num_float_index > tmp_float_index ? num_float_index: tmp_float_index; 
}

void decrease_index(astexpr e){
    if((e->u.sym == NULL || e->type == CASTEXPR) && e->flag == PRI){
	if(e->ftype == 0)
        	tmp_index--;
	else
		tmp_float_index--; 
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
  fclose(yyin); 
  fclose(var_file);                /* No longer needed */
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

void compute_modulus_for_declaration(astspec spec){
	astspec s = NULL; 
	if(spec->type == SPEC)
		s = spec; 
	else if(spec->type == SPECLIST && spec->body->subtype == SPEC_private)
		s = spec->u.next; 
	if(s != NULL){
		if(s->subtype == SPEC_int)
			modulus = modulus > s->size ? modulus : s->size; 
		else if(s->subtype == SPEC_float){
			modulus = modulus > s->size ? modulus : s->size; 
			modulus = modulus > s->sizeexp ? modulus : s->sizeexp; 
		}
	}
}

void compute_modulus_for_BOP(astexpr e1, astexpr e2, int opid){
	if(e1->ftype == 0 && e2->ftype == 0){
		int len = fmax(e1->size, e2->size); 
		if(opid == BOP_gt || opid == BOP_lt || opid == BOP_leq || opid == BOP_geq || opid == BOP_eqeq || opid == BOP_neq)
			modulus = fmax(modulus, len+48); 
		else if(opid == BOP_div)
			modulus = fmax(modulus, 2*len+48+8);
		else if(opid == RIGHT_OP && e2->flag == PRI)
			modulus = fmax(modulus, e2->size+48); 		
	}else if(e1->ftype == 1 && e2->ftype == 1){
		int len = 0, k = 0; 
		if(e1->size == e2->size && e1->sizeexp == e2->sizeexp){
			len = e1->size; 
			k = e1->sizeexp; 
		}else{
			len = fmax(e1->size, e2->size); 
			k = fmax(e1->sizeexp, e2->sizeexp); 
			
			int d = abs(e1->size-e2->size); 
			if(pow(2, k) >= d)
				k++;
			else
				k = (int)ceil(log(d))+1; 
		}
		if(opid == BOP_add || opid == BOP_sub)
			modulus = fmax(modulus, fmax(2*len+1, k)+48);
		else if(opid == BOP_mul)
			modulus = fmax(modulus, 2*len+48); 
		else if(opid == BOP_gt || opid == BOP_lt || opid == BOP_leq || opid == BOP_geq)
			modulus = fmax(modulus, fmax(len+1, k)+48); 	
		else if(opid == BOP_eqeq || opid == BOP_neq)
			modulus = fmax(modulus, fmax(len, k)+48); 
		else if(opid == BOP_div)
			modulus = fmax(modulus, 2*len+48+1);  
	}else if(e1->flag == PRI && e2->flag == PRI && (e1->ftype == 0 && e2->ftype == 1 || e1->ftype == 1 && e2->ftype == 0)){
		printf("%d, %d\n", e1->ftype, e2->ftype); 	
		printf("Operands with different types are not allowed.\n"); 
		exit(0); 
	}
}
	

