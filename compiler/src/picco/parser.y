%{
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
void    parse_warning(char *format, ...);
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
void    store_struct_information(struct_node_stack, astspec);

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
int	    is_priv_int_index_appear = 0; 
int 	is_priv_float_index_appear = 0;
int 	is_priv_int_ptr_struct_field_appear = 0; 
int 	is_priv_float_ptr_struct_field_appear = 0; 
int 	is_priv_struct_ptr_struct_field_appear = 0; 
int 	is_priv_int_struct_field_appear = 0; 
int 	is_priv_float_struct_field_appear = 0; 
int 	thread_id = -1;  
int	    num_threads = 0;
int 	modulus = 0; 
int     global_variables_c_restrict_flag = 0;
int     pointer_flag = 0;
int     declis = 0;
 
struct_node_stack struct_table = NULL;
 
char    *parsingstring;       /* For error reporting when parsing string */
FILE 	*var_file; 
%}

%union {
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

%error-verbose


/* %expect 4 */

/* Start-symbol tokens (trick from bison manual) */
%token START_SYMBOL_EXPRESSION START_SYMBOL_BLOCKLIST
%type <type> start_trick

/* C tokens */
%token <name> IDENTIFIER CONSTANT STRING_LITERAL
%token <name> INT VOID RETURN 
%token <name> PRIVATE PUBLIC
%token <name> INC_OP DEC_OP LE_OP GE_OP EQ_OP NE_OP AND_OP OR_OP RIGHT_OP LEFT_OP
%token <name> ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN
%token <name> IF ELSE WHILE DO FOR CONTINUE BREAK
%token <name> SMCINPUT SMCOUTPUT

/* OpenMP tokens */
%token <name> PRAGMA_OMP PRAGMA_OMP_THREADPRIVATE OMP_PARALLEL OMP_SECTIONS
%token <name> OMP_NOWAIT OMP_ORDERED OMP_SCHEDULE OMP_STATIC OMP_DYNAMIC
%token <name> OMP_GUIDED OMP_RUNTIME OMP_AUTO OMP_SECTION OMP_AFFINITY
%token <name> OMP_SINGLE OMP_MASTER OMP_CRITICAL OMP_BARRIER OMP_ATOMIC
%token <name> OMP_FLUSH OMP_PRIVATE OMP_FIRSTPRIVATE
%token <name> OMP_LASTPRIVATE OMP_SHARED OMP_DEFAULT OMP_NONE OMP_REDUCTION
%token <name> OMP_COPYIN OMP_NUMTHREADS OMP_COPYPRIVATE OMP_FOR OMP_IF
/* added @ OpenMP 3.0 */
%token <name> OMP_TASK OMP_UNTIED OMP_TASKWAIT OMP_COLLAPSE
/* added @ OpenMP 3.1 */
%token <name> OMP_FINAL OMP_MERGEABLE OMP_TASKYIELD OMP_READ OMP_WRITE
%token <name> OMP_CAPTURE OMP_UPDATE OMP_MIN OMP_MAX

/* C non-terminals */
%type <string> string_literal
%type <expr>   primary_expression
%type <expr>   postfix_expression
%type <expr>   argument_expression_list
%type <expr>   cast_expression
%type <expr>   unary_expression
%type <type>   unary_operator
%type <expr>   multiplicative_expression
%type <expr>   additive_expression
%type <expr>   shift_expression
%type <expr>   relational_expression
%type <expr>   equality_expression
%type <expr>   AND_expression
%type <expr>   exclusive_OR_expression
%type <expr>   inclusive_OR_expression
%type <expr>   logical_AND_expression
%type <expr>   logical_OR_expression
%type <expr>   DOT_product_expression
%type <expr>   conditional_expression

%type <expr>   assignment_expression
%type <type>   assignment_operator
%type <expr>   expression
%type <expr>   constant_expression
%type <stmt>   declaration
%type <spec>   declaration_specifiers
%type <decl>   init_declarator_list
%type <decl>   init_declarator
%type <spec>   type_specifier
%type <spec>   type_qualifier
%type <spec>   specifier_qualifier_list
%type <decl>   declarator
%type <decl>   direct_declarator
%type <spec>   pointer

%type <spec>   struct_or_union_specifier
%type <type>   struct_or_union
%type <decl>   struct_declaration_list
%type <decl>   struct_declaration
%type <decl>   struct_declarator_list
%type <decl>   struct_declarator
%type <decl>   parameter_type_list
%type <decl>   parameter_list
%type <decl>   parameter_declaration
%type <decl>   identifier_list
%type <decl>   type_name

%type <decl>   abstract_declarator
%type <decl>   direct_abstract_declarator
%type <symb>   typedef_name
%type <expr>   initializer
%type <expr>   initializer_list
%type <expr>   designation
%type <expr>   designator_list
%type <expr>   designator
%type <stmt>   statement
%type <stmt>   compound_statement
%type <stmt>   block_item_list
%type <stmt>   block_item
%type <stmt>   expression_statement
%type <stmt>   selection_statement
%type <stmt>   if_subroutine
%type <stmt>   iteration_statement
%type <stmt>   increase_thread_id
%type <stmt>   parallel_statement
%type <stmt>   parallel_scope
%type <stmt>   parallel_sequence
%type <stmt>   parallel_item
%type <stmt>   iteration_statement_for

%type <stmt>   jump_statement
%type <stmt>   translation_unit
%type <stmt>   external_declaration /* the flag to keep track of globals, gflag */
%type <stmt>   function_definition
%type <stmt>   smc_statement
%type <stmt>   batch_statement
%type <stmt>   normal_function_definition
%type <stmt>   pfree_statement /* by ghada*/



/* OpenMP non-terminnals */
%type <ocon>   openmp_construct
%type <ocon>   openmp_directive
%type <stmt>   structured_block
%type <ocon>   parallel_construct
%type <odir>   parallel_directive
%type <ocla>   parallel_clause_optseq
%type <ocla>   parallel_clause
%type <ocla>   unique_parallel_clause
%type <ocon>   for_construct
%type <ocla>   for_clause_optseq
%type <odir>   for_directive
%type <ocla>   for_clause
%type <ocla>   unique_for_clause
%type <type>   schedule_kind
%type <ocon>   sections_construct
%type <ocla>   sections_clause_optseq
%type <odir>   sections_directive
%type <ocla>   sections_clause
%type <stmt>   section_scope
%type <stmt>   section_sequence
%type <odir>   section_directive
%type <ocon>   single_construct
%type <ocla>   single_clause_optseq
%type <odir>   single_directive
%type <ocla>   single_clause
%type <ocon>   parallel_for_construct
%type <ocla>   parallel_for_clause_optseq
%type <odir>   parallel_for_directive
%type <ocla>   parallel_for_clause
%type <ocon>   parallel_sections_construct
%type <ocla>   parallel_sections_clause_optseq
%type <odir>   parallel_sections_directive
%type <ocla>   parallel_sections_clause
%type <ocon>   master_construct
%type <odir>   master_directive
%type <ocon>   critical_construct
%type <odir>   critical_directive
%type <symb>   region_phrase
%type <odir>   barrier_directive
%type <ocon>   atomic_construct
%type <odir>   atomic_directive
%type <odir>   flush_directive
%type <decl>   flush_vars
%type <ocon>   ordered_construct
%type <odir>   ordered_directive
%type <odir>   threadprivate_directive
%type <ocla>   data_clause
%type <type>   reduction_operator
%type <decl>   variable_list
%type <decl>   thrprv_variable_list
/* added @ OpenMP V3.0 */
%type <ocon>   task_construct
%type <odir>   task_directive
%type <ocla>   task_clause_optseq
%type <ocla>   task_clause
%type <ocla>   unique_task_clause
%type <odir>   taskwait_directive
%type <odir>   taskyield_directive

/*
 * OMPi-extensions
 */

/* Tokens */
%token <name> PRAGMA_OMPIX OMPIX_TASKDEF OMPIX_IN OMPIX_OUT OMPIX_INOUT
%token <name> OMPIX_TASKSYNC OMPIX_UPONRETURN OMPIX_ATNODE OMPIX_DETACHED
%token <name> OMPIX_ATWORKER OMPIX_TASKSCHEDULE OMPIX_STRIDE OMPIX_START
%token <name> OMPIX_SCOPE OMPIX_NODES OMPIX_WORKERS OMPIX_LOCAL OMPIX_GLOBAL
%token <name> OMPIX_TIED
%token <name> FLOAT STRUCT UNION TYPEDEF PTR_OP TYPE_NAME SIZEOF
%token <name> PMALLOC PFREE CHAR SHORT LONG

/* Non-terminals */
%type <xcon>   ompix_construct
%type <xcon>   ompix_directive
%type <xcon>   ox_taskdef_construct
%type <xdir>   ox_taskdef_directive
%type <xcla>   ox_taskdef_clause_optseq
%type <xcla>   ox_taskdef_clause
%type <decl>   ox_variable_size_list
%type <decl>   ox_variable_size_elem
%type <xcon>   ox_task_construct
%type <xdir>   ox_task_directive
%type <xcla>   ox_task_clause_optseq
%type <xcla>   ox_task_clause
%type <expr>   ox_funccall_expression
%type <xdir>   ox_tasksync_directive
%type <xdir>   ox_taskschedule_directive
%type <xcla>   ox_taskschedule_clause_optseq
%type <xcla>   ox_taskschedule_clause
%type <type>   ox_scope_spec

%%

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     THE RULES                                                 *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

start_trick:
    {
    	var_file = fopen(var_list, "w+");
	struct_table = struct_node_stack_new(); 
    }
    translation_unit                       {}
  | START_SYMBOL_EXPRESSION expression     { pastree_expr = $2; }
  | START_SYMBOL_BLOCKLIST block_item_list { pastree_stmt = $2; }
;



/* -------------------------------------------------------------------------
 * ---------- ISO/IEC 9899:1999 A.1 Lexical grammar ------------------------
 * -------------------------------------------------------------------------
 */

/* -------------------------------------------------------------------------
 * ---------- ISO/IEC 9899:1999 A.1.5 Constants ----------------------------
 * -------------------------------------------------------------------------
 */



/* -------------------------------------------------------------------------
 * ---------- ISO/IEC 9899:1999 A.1.6 String literals ----------------------
 * -------------------------------------------------------------------------
 */

/* ISO/IEC 9899:1999 6.4.5 */
string_literal:
    STRING_LITERAL
    {
      $$ = strdup($1);
    }
  | string_literal STRING_LITERAL
    {
      /* Or we could leave it as is (as a SpaceList) */
      if (($1 = realloc($1, strlen($1) + strlen($2))) == NULL)
        parse_error(-1, "String memory allocation error.\n");
      strcpy(($1)+(strlen($1)-1),($2)+1);  /* Catenate on the '"' */
      $$ = $1;
    }
;


/* -------------------------------------------------------------------------
 * ------ ISO/IEC 9899:1999 A.2 Phrase structure grammar -------------------
 * -------------------------------------------------------------------------
 */

/* -------------------------------------------------------------------------
 * ------- ISO/IEC 9899:1999 A.2.1 Expressions -----------------------------
 * -------------------------------------------------------------------------
 */

/*  ISO/IEC 9899:1999 6.5.1 */
primary_expression: // This is where var name, const value and string literal gets checked 
    IDENTIFIER
    {
      symbol id = Symbol($1); 
      stentry e;
      int     chflag = 0;
    
      if (checkDecls)
      {
        check_uknown_var($1); // this is where the checking for unknown vars happen for all identifiers
        
        if ((e = symtab_get(stab, id, IDNAME)) != NULL) /* could be enum name */
          if (istp(e) && threadmode)
            chflag = 1;
      }
      $$ = chflag ? UnaryOperator(UOP_paren,
                             UnaryOperator(UOP_star, Identifier(id)))
                  : Identifier(id);
      set_identifier_attributes(id, $$, 0); 	 /// this is one place 
    }
  | CONSTANT
    {
      $$ = Constant( strdup($1) );
    //   printf("\nvalue is here: %s", $1);
    //   char *str = strdup($1); 
    //   int ftype_val = 0;
    //   for (int i = 0; str[i] != '\0'; i++) {
    //     // printf("%c", str[i]);
    //     if (str[i] == '.') { // number is float
    //         // printf(". is found");
    //         ftype_val = 1;
    //         break;
    //         // if ($$->ftype == 0) { // but if the type of the number is int -> tell the user 
    //         //     printf("\n\nWarning: Float was converted to int with no rounding...\n\n");
    //         // } -> wrong int B.6
    //     } // else if (str[i+1] == '\0' && str[i] != '.') {
    //     //     ftype_val = 0;
    //     //     printf("\n\nWarning: Float was converted to int with no rounding...\n\n");
    //     // }
    //   }
    //   $$->ftype = ftype_val;
        //   if ($$->flag == PUB) 
        //     $$->ftype = 0;
      set_security_flag_expr($$, NULL, NULL, -1);
    }
  | string_literal
    {
      $$ = String($1);
    }
  | '(' expression ')'
    {
      $$ = UnaryOperator(UOP_paren, $2);
      $$->ftype = $2->ftype; 
      set_security_flag_expr($$, $2, NULL, -1);
      set_bitlength_expr($$, $2, NULL); 
    }
;

/*  ISO/IEC 9899:1999 6.5.2 */
postfix_expression:
    primary_expression
    {
      $$ = $1;
    }
  | postfix_expression '[' expression ']'
    {
      global_variables_c_restrict_flag = 0;
      $$ = ArrayIndex($1, $3);
      set_security_flag_expr($$, $1, NULL, -1);
      set_bitlength_expr($$, $1, NULL);
      set_size_symbol($$, $1, $3);
      $$->ftype = $1->ftype;
      if($3->ftype == 1) { // if the index is private, public, const float 
        parse_error(-1, "Non-integer used for '%s' array index.\n", $1->u.sym->name);
      }
      if($3->flag == PRI && $1->ftype == 0)
		is_priv_int_index_appear = 1;
      if($3->flag == PRI && $1->ftype == 1)
		is_priv_float_index_appear = 1;  
    }
  /* The following 2 rules were added so that calling undeclared functions
   * does not result in "unknown identifier" messages (it was matched by
   * the IDENTIFIER rule in primary_expression.
   * They account for 2 shift/reduce conflicts.
   * (VVD)
   */
  | IDENTIFIER '(' ')'
    {
      /* Catch calls to "main()" (unlikely but possible) */
      $$ = strcmp($1, "main") ?
             FunctionCall(Identifier(Symbol($1)), NULL) :
             FunctionCall(Identifier(Symbol(MAIN_NEWNAME)), NULL);
      set_security_flag_func($1, $$);
    }
  | IDENTIFIER '(' argument_expression_list ')'
    {
      /* Catch calls to "main()" (unlikely but possible) */
      if (check_func_param(Identifier(Symbol($1)), $3))
          parse_error(1, "Too many or too few arguments to function '%s'.\n", $1);
      $$ = strcmp($1, "main") ?
             FunctionCall(Identifier(Symbol($1)), $3) :
             FunctionCall(Identifier(Symbol(MAIN_NEWNAME)), $3);
      // set the flag of func expr based on its return type
      set_security_flag_func($1, $$);
    }
  | postfix_expression '(' ')'
    {
        $$ = FunctionCall($1, NULL);
    }
  | postfix_expression '(' argument_expression_list ')'
    {
        $$ = FunctionCall($1, $3);
    }
  | postfix_expression '.' typedef_name
  {
       $$ = DotField($1, $3);
  }
  | postfix_expression PTR_OP typedef_name
  {
       $$ = PtrField($1, $3);
  }
  | postfix_expression INC_OP
    {
        $$ = PostOperator($1, UOP_inc);
        set_security_flag_expr($$, $1, NULL, -1);
	set_bitlength_expr($$, $1, NULL); 
    }
  | postfix_expression DEC_OP
   {
        $$ = PostOperator($1, UOP_dec);
        set_security_flag_expr($$, $1, NULL, -1);
	set_bitlength_expr($$, $1, NULL); 
   }
  | postfix_expression '.' IDENTIFIER
    {
	$$ = DotField($1, Symbol($3));
	set_identifier_attributes(Symbol($3), $$, 1); 
    }
  | postfix_expression PTR_OP IDENTIFIER
    {
	$$ = PtrField($1, Symbol($3));
	set_identifier_attributes(Symbol($3), $$, 1); 
    }
  | '(' type_name ')' '{' initializer_list '}'
    {
        printf("'(' typename ')' '{' initializer_list '}'");
        
        $$ = CastedExpr($2, BracedInitializer($5));
    }
  | '(' type_name ')' '{' initializer_list ',' '}'
    {
        printf("'(' typename ')' '{' initializer_list ',' '}'");
        $$ = CastedExpr($2, BracedInitializer($5));
    }

;

/*  ISO/IEC 9899:1999 6.5.2 */
argument_expression_list:  
    assignment_expression
   {
      $$ = $1;
    }
  | argument_expression_list ',' assignment_expression
    {
      $$ = CommaList($1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.5.3 */
unary_expression:		
    postfix_expression
    {
        $$ = $1;
        $$->thread_id = thread_id;  
    }
    | INC_OP unary_expression
    {   
        $$ = PreOperator($2, UOP_inc);
        set_security_flag_expr($$, $2, NULL, -1);
    	set_bitlength_expr($$, $2, NULL); 
        $$->thread_id = thread_id;  
    }
    | DEC_OP unary_expression
    {
        $$ = PreOperator($2, UOP_dec);
        set_security_flag_expr($$, $2, NULL, -1);
    	set_bitlength_expr($$, $2, NULL); 
        $$->thread_id = thread_id;  
    }
    | unary_operator unary_expression
    {
	$$ = UnaryOperator($1, $2);
        $$->ftype = $2->ftype;
        set_security_flag_expr($$, $2, NULL, -1);
	set_bitlength_expr($$, $2, NULL); 
	//only for integer unary expression - !var
	if($1 == UOP_lnot)
	{
		increase_index($$); 
		$$->index = tmp_index;  
		decrease_index($$); 				
	} 
        $$->thread_id = thread_id;  
    }
    | unary_operator cast_expression
    {
        if ($1 == -1)
            $$ = $2;                    /* simplify */
        else
            $$ = UnaryOperator($1, $2);
        $$->ftype = $2->ftype;
        set_security_flag_expr($$, $2, NULL, -1);
	set_bitlength_expr($$, $2, NULL); 
	//only for integer unary expression - !var
	if($1 == UOP_lnot)
	{
		increase_index($$); 
		$$->index = tmp_index;  
		decrease_index($$); 				
	} 
        $$->thread_id = thread_id;  
    }
    | SIZEOF unary_expression
    {
        $$ = Sizeof($2);
        $$->thread_id = thread_id; 	
    }
    | SIZEOF '(' type_name ')'
    {
        $$ = Sizeoftype($3);
	$$->thread_id = thread_id; 
    }
       /* by ghada -----------------------------------------*/
    | PMALLOC '(' primary_expression ',' type_name ')'
        {
                $$ = Pmalloc($3, $5);
                $$->flag = 0; //make it PUB to be able to use it with public struct
      		$$->thread_id = thread_id;  
	}
;

unary_operator:
    '-'
    {
        $$ = UOP_neg; 
    }
    |'~'
    {
        $$ = UOP_bnot; 
    }
    | '!'
    {
        $$ = UOP_lnot; 
    }
    | '&'
    {
        $$ = UOP_addr;
    }
    | '*'
    {
        $$ = UOP_star;
    }
;

type_name:
    specifier_qualifier_list
    {
        $$ = Casttypename($1, NULL);
    }
    | specifier_qualifier_list abstract_declarator
    {
        $$ = Casttypename($1, $2);
    }
    | struct_or_union_specifier
    {
	$$ = Casttypename($1, NULL); 
    }
;

cast_expression:
    unary_expression
    {
        $$ = $1;
	    $$->thread_id = thread_id; 
        // printf("\n\n *******6 \n");
        // printf("Name: %s \n", $1->u.str);
        // printf("Type: %d \n\n", $1->ftype);
        $$->ftype = $1->ftype;
        set_security_flag_expr($$, $1, NULL, -1);
        $$->size = $1->size;
	    $$->sizeexp  = $1->sizeexp; 
    }
    | '(' type_name ')'
    {
        if($2->spec->subtype == SPEC_int || $2->spec->subtype == SPEC_Rlist && ($2->spec->body->subtype == SPEC_private && $2->spec->u.next->subtype == SPEC_int))
            tmp_index++; 
        if($2->spec->subtype == SPEC_float || $2->spec->subtype == SPEC_Rlist && ($2->spec->body->subtype == SPEC_private && $2->spec->u.next->subtype == SPEC_float))
            tmp_float_index++;  
        num_index = num_index > tmp_index? num_index: tmp_index;
        num_float_index = num_float_index > tmp_float_index ? num_float_index: tmp_float_index; 
    } 
    cast_expression
    {
        $$ = CastedExpr($2, $5);
        $$->thread_id = thread_id; 
        if($2->spec->subtype == SPEC_int || $2->spec->subtype == SPEC_Rlist && $2->spec->u.next->subtype == SPEC_int)
            $$->ftype = 0;
        if($2->spec->subtype == SPEC_float || $2->spec->subtype == SPEC_Rlist && $2->spec->u.next->subtype == SPEC_float)
            $$->ftype = 1;
        $$->flag = $5->flag;

        if($2->spec->subtype == SPEC_int || $2->spec->subtype == SPEC_Rlist && ($2->spec->body->subtype == SPEC_private && $2->spec->u.next->subtype == SPEC_int))
        {
            $$->size = $2->spec->subtype == SPEC_Rlist ? $2->spec->u.next->size : $2->spec->size;  
            tmp_index--;
            $$->index = tmp_index; 
            $$->flag = PRI; 
            //FL2INT
            if($5->ftype == 1)
                modulus = fmax(modulus, fmax(2*$5->size+1, $5->sizeexp)+kappa_nu);
        }
        if($2->spec->subtype == SPEC_float || $2->spec->subtype == SPEC_Rlist && ($2->spec->body->subtype == SPEC_private && $2->spec->u.next->subtype == SPEC_float))
        {
            $$->size = $2->spec->subtype == SPEC_Rlist ? $2->spec->u.next->size : $2->spec->size;  
            $$->sizeexp = $2->spec->subtype == SPEC_Rlist ? $2->spec->u.next->sizeexp : $2->spec->sizeexp;  
            tmp_float_index--;
            $$->index = tmp_float_index; 
            $$->flag = PRI; // old
            // $$->flag = $5->flag; // new
            //FL2FL
            if($5->ftype == 1 && $5->size > $$->size)
                if($5->flag == PRI)
                    modulus = fmax(modulus, $5->size+kappa_nu); 
                    // modulus = fmax(modulus,  $5->size+48 );
            //INT2FL
            if($5->ftype == 0)
                if($5->flag == PRI)
                    modulus = fmax(modulus, $5->size+kappa_nu); 
                    // modulus = fmax(modulus, $5->size+48); 
                    // modulus = fmax(modulus, $5->flag == PRI ? $5->size+48 : 32+48); 
                
        } 
    }
;

/*  ISO/IEC 9899:1999 6.5.5 */
multiplicative_expression:		
    cast_expression
    {
      global_variables_c_restrict_flag = 0;
      $$ = $1;
      set_bitlength_expr($$, $1, NULL); 
    }
  | multiplicative_expression '*'
    {
        increase_index($1);
    }
    cast_expression
    {
      decrease_index($1); 
      $$ = BinaryOperator(BOP_mul, $1, $4);
      $$->ftype = $4->ftype; 
      set_security_flag_expr($$, $1, $4, BOP_mul); // thisisone
      set_bitlength_expr($$, $1, $4); 
    }
  | multiplicative_expression '/'
   {
       increase_index($1);
   }
   cast_expression
    {
      decrease_index($1);
      $$ = BinaryOperator(BOP_div, $1, $4);
      $$->ftype = $4->ftype; 
      set_security_flag_expr($$, $1, $4, BOP_div); // thisisone
      set_bitlength_expr($$, $1, $4); 
    }
  | multiplicative_expression '%'
    {
        increase_index($1);
    }
    cast_expression
    {
      decrease_index($1);
      $$ = BinaryOperator(BOP_mod, $1, $4);
      $$->ftype = $4->ftype; 
      set_security_flag_expr($$, $1, $4, BOP_mod); // thisisone
      set_bitlength_expr($$, $1, $4); 
    }
;

/*  ISO/IEC 9899:1999 6.5.6 */
additive_expression:		
    multiplicative_expression
    {
      $$ = $1;
      set_bitlength_expr($$, $1, NULL); 
    }
  | additive_expression '+' // handles +
   {
	increase_index($1);
   }
  multiplicative_expression
  {
      
      decrease_index($1);
      $$ = BinaryOperator(BOP_add, $1, $4);
    //   printf("\nName: %s ", $4->u.str);
    //   printf("\n$1 Type: %d ", $1->ftype);
    //   printf("\n$4 Type: %d ", $4->ftype);
    //   printf("\n$$ Type: %d ", $$->ftype);
    //   if ($1->ftype == 1 || $4->ftype == 1) { // if at least one operand is float
    //     $$->ftype = 1;
    //   } else {
    if ($1 != NULL && $4 != NULL) {
        if($4->ftype == 1) 
            $$->ftype = $4->ftype; 
        else 
            $$->ftype = $1->ftype; 
    } else if ($1 == NULL && $4 == NULL) {
        $$->ftype = $4->ftype; 
    } else {
        if ($4 != NULL) {
        $$->ftype = $4->ftype; 
        } else if ($1 != NULL) {
        $$->ftype = $1->ftype; 
        }
    }
    //   }
    //   printf("\n$$ Type: %d ", $$->ftype);
      set_security_flag_expr($$, $1, $4, BOP_add); // thisisone
      set_bitlength_expr($$, $1, $4); 
  }
  | additive_expression '-' // handles -
  {
      increase_index($1);
  }
    multiplicative_expression
    {
      decrease_index($1);
      $$ = BinaryOperator(BOP_sub, $1, $4);
      $$->ftype = $4->ftype; 
      set_security_flag_expr($$, $1, $4, BOP_sub); // thisisone
      set_bitlength_expr($$, $1, $4); 
    }
;

/*  ISO/IEC 9899:1999 6.5.7 */
shift_expression:
    additive_expression
    {
        $$ = $1;
        set_bitlength_expr($$, $1, NULL); 
    }
    | shift_expression LEFT_OP 
    {
	increase_index($1); 
    }
    additive_expression
    {
        decrease_index($1); 
        $$ = BinaryOperator(BOP_shl, $1, $4);
        if ($1->ftype == 1 || $4->ftype ==1) {
            parse_error(1, "Invalid operands to binary << (use an int or cast if needed)\n");
        }
        $$->ftype = $1->ftype; 
        set_security_flag_expr($$, $1, $4, BOP_shl); // thisisone
        set_bitlength_expr($$, $1, $4); 
    }
    | shift_expression RIGHT_OP 
    {
	increase_index($1); 
    }
    additive_expression
    {
	    decrease_index($1); 
        $$ = BinaryOperator(BOP_shr, $1, $4);
        if ($1->ftype == 1 || $4->ftype ==1) {
            parse_error(1, "Invalid operands to binary >> (use an int or cast if needed)\n");
        }
        $$->ftype = $1->ftype; 
        set_security_flag_expr($$, $1, $4, BOP_shr); // thisisone
        set_bitlength_expr($$, $1, $4); 
    }
;

/*  ISO/IEC 9899:1999 6.5.8 */
relational_expression:
    shift_expression
    {
        $$ = $1;
        set_bitlength_expr($$, $1, NULL); 
    }
    | relational_expression '<'
    {
        increase_index($1);
    }
    shift_expression
    {
        decrease_index($1); //needs to operate on tmp_index
        $$ = BinaryOperator(BOP_lt, $1, $4);
        $$->ftype = 0; 
	$$->size = 1; 
        set_security_flag_expr($$, $1, $4, BOP_lt); // thisisone
        //set_bitlength_expr($$, $1, $4); 
    }
    | relational_expression '>'
    {
        increase_index($1);
    }
    shift_expression
    {
        decrease_index($1);
        $$ = BinaryOperator(BOP_gt, $1, $4);
      	$$->ftype = 0; 
	$$->size = 1; 
        set_security_flag_expr($$, $1, $4, BOP_gt); // thisisone
        //set_bitlength_expr($$, $1, $4); 
    }
    | relational_expression LE_OP
    {
        increase_index($1);
    }
    shift_expression
    {
        decrease_index($1);
        $$ = BinaryOperator(BOP_leq, $1, $4);
        $$->ftype = 0; 
	$$->size = 1; 
	set_security_flag_expr($$, $1, $4, BOP_leq); // thisisone
        //set_bitlength_expr($$, $1, $4); 
    }
    | relational_expression GE_OP
    {
        increase_index($1);
    }
    shift_expression
    {
        decrease_index($1);
        $$ = BinaryOperator(BOP_geq, $1, $4);
        $$->ftype = 0; 
	$$->size = 1; 
        set_security_flag_expr($$, $1, $4, BOP_geq); // thisisone
        //set_bitlength_expr($$, $1, $4); 
    }
;

/*  ISO/IEC 9899:1999 6.5.9 */
equality_expression:
    relational_expression
    {
        $$ = $1;
        set_bitlength_expr($$, $1, NULL); 
    }
    | equality_expression EQ_OP
    {
        increase_index($1);
    }
    relational_expression
    {
        decrease_index($1);
        $$ = BinaryOperator(BOP_eqeq, $1, $4);
        $$->ftype = 0;
	$$->size = 1;  
        set_security_flag_expr($$, $1, $4, BOP_eqeq); // thisisone
        //set_bitlength_expr($$, $1, $4); 
    }
    | equality_expression NE_OP
    {
        increase_index($1);
    }
    relational_expression
    {
        decrease_index($1);
        $$ = BinaryOperator(BOP_neq, $1, $4);
        $$->ftype = 0; 
	$$->size = 1; 
	set_security_flag_expr($$, $1, $4, BOP_neq); // thisisone
        //set_bitlength_expr($$, $1, $4); 
    }
;
/*  ISO/IEC 9899:1999 6.5.10 */
AND_expression:
    equality_expression
    {
        $$ = $1;
        set_bitlength_expr($$, $1, NULL); 
    }
    | AND_expression '&'
    {
        increase_index($1);
    }
    equality_expression
    {
        decrease_index($1);
        $$ = BinaryOperator(BOP_band, $1, $4);
        set_security_flag_expr($$, $1, $4, BOP_band); // thisisone
        set_bitlength_expr($$, $1, $4); 
    }
;

/*  ISO/IEC 9899:1999 6.5.11 */
exclusive_OR_expression:
    AND_expression
    {
        $$ = $1;
        set_bitlength_expr($$, $1, NULL); 
    }
    | exclusive_OR_expression '^'
    {
        increase_index($1);
    }
    AND_expression
    {
        decrease_index($1);
        $$ = BinaryOperator(BOP_bxor, $1, $4);
        set_security_flag_expr($$, $1, $4, BOP_bxor); // thisisone
        set_bitlength_expr($$, $1, $4); 
    }
;

/*  ISO/IEC 9899:1999 6.5.12 */
inclusive_OR_expression:
    exclusive_OR_expression
    {
        $$ = $1;
        set_bitlength_expr($$, $1, NULL); 
    }
    | inclusive_OR_expression '|'
    {
        increase_index($1);
    }
    exclusive_OR_expression
    {
        decrease_index($1);
        $$ = BinaryOperator(BOP_bor, $1, $4);
        set_security_flag_expr($$, $1, $4, BOP_bor); // thisisone
        set_bitlength_expr($$, $1, $4); 
    }
;

/*  ISO/IEC 9899:1999 6.5.13 */
logical_AND_expression:
    inclusive_OR_expression
    {
        $$ = $1;
        set_bitlength_expr($$, $1, NULL); 
    }
    | logical_AND_expression AND_OP
    {
        increase_index($1);
    }
    inclusive_OR_expression
    {
        decrease_index($1);
        $$ = BinaryOperator(BOP_land, $1, $4);
        set_security_flag_expr($$, $1, $4, BOP_land); // thisisone
        set_bitlength_expr($$, $1, $4); 
    }
;

/*  ISO/IEC 9899:1999 6.5.14 */
logical_OR_expression:
    logical_AND_expression
    {
        $$ = $1;
        set_bitlength_expr($$, $1, NULL); 
    }
    | logical_OR_expression OR_OP
    {
        increase_index($1);
    }
    logical_AND_expression
    {
        decrease_index($1);
        $$ = BinaryOperator(BOP_lor, $1, $4);
        set_security_flag_expr($$, $1, $4, BOP_lor); // thisisone
        set_bitlength_expr($$, $1, $4); 
    }
;
DOT_product_expression:
    unary_expression '@' unary_expression{
        $$ = BinaryOperator(BOP_dot, $1, $3);
        if ($1->ftype == 1 || $3->ftype == 1)
            parse_error(1, "Dot product operation @ is only supported for integers.\n");
        set_security_flag_expr($$, $1, $3, BOP_dot); // thisisone
        set_bitlength_expr($$, $1, $3); 
    }
;
/*  ISO/IEC 9899:1999 6.5.15 */
conditional_expression:
    logical_OR_expression
    {
        $$ = $1;
        set_bitlength_expr($$, $1, NULL); 
    }
    | DOT_product_expression
    {
        $$ = $1;
        set_bitlength_expr($$, $1, NULL); 
    }
    | logical_OR_expression '?' expression ':' conditional_expression
    {
        $$ = ConditionalExpr($1, $3, $5);
        set_bitlength_expr($$, $3, $5); 
    }
;

/*  ISO/IEC 9899:1999 6.5.16 */
assignment_expression:
  conditional_expression
    {
      $$ = $1;
      set_bitlength_expr($$, $1, NULL);
      $$->thread_id = thread_id;  
    }
  | unary_expression assignment_operator assignment_expression
    {
    //   printf("arr is assignment_expression\n");
    //   printf("1 Name: %s \n", $1->u.str);
    //   printf("2 Name: %d \n", $2);
    //   printf("3 Name: %s \n", $3->u.str);

      //do security check here
      security_check_for_assignment($1, $3);
      $$ = Assignment($1, $2, $3);
      $$->ftype = $1->ftype; 
      set_security_flag_expr($$, $1, NULL, -1);
      $$->thread_id = thread_id; 
      //set_bitlength_expr($$, $1, $3); 
    }
;

/*  ISO/IEC 9899:1999 6.5.16 */
assignment_operator:	
    '='
    {
      $$ = ASS_eq;  /* Need fix here! */
    }
  | MUL_ASSIGN
    {
      $$ = ASS_mul;
    }
  | DIV_ASSIGN
    {
      $$ = ASS_div;
    }
  | MOD_ASSIGN
    {
      $$ = ASS_mod;
    }
  | ADD_ASSIGN
    {
      $$ = ASS_add;
    }
  | SUB_ASSIGN
    {
      $$ = ASS_sub;
    }
;

/*  ISO/IEC 9899:1999 6.5.17 */
expression:	// This is where each expression gets handled 
    assignment_expression
    {
      $$ = $1;
      set_security_flag_expr($$, $1, NULL, -1);
      set_bitlength_expr($$, $1, NULL); 
    }
  | expression ',' assignment_expression
    {
      $$ = CommaList($1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.6 */
constant_expression:
    conditional_expression
    {
      $$ = $1;
      set_security_flag_expr($$, $1, NULL, -1);
    }
;

/* -------------------------------------------------------------------------
 * ------------ ISO/IEC 9899:1999 A.2.2 Declarations -----------------------
 * -------------------------------------------------------------------------
 */

/*  ISO/IEC 9899:1999 6.7 */
declaration:
  declaration_specifiers ';'
  {
      if (isTypedef && $1->type == SPECLIST)
        $$ = Declaration($1, fix_known_typename($1));
      else
        $$ = Declaration($1, NULL);
      isTypedef = 0;
      $$->gflag = 0;
      global_variables_c_restrict_flag = 0;
  }
  |
  declaration_specifiers init_declarator_list ';'
    {
      global_variables_c_restrict_flag = 0;
      if (checkDecls) add_declaration_links($1, $2, 0);
      	security_check_for_declaration($1, $2);
      $$ = Declaration($1, $2);
      isTypedef = 0;
      set_pointer_flag($1, $2);
      $$->gflag = 0;
    //   printf("\ntype $1 %d, ", $1->type);
    //   printf("type $2 %d, %d ", $2->type, $2->u.id->name);
    //   printf("type $$ %d\n", $$->type);
    }
  | threadprivate_directive // OpenMP Version 2.5 ISO/IEC 9899:1999 addition
    {
        $$ = OmpStmt(OmpConstruct(DCTHREADPRIVATE, $1, NULL));
        $$->gflag = 0;
        global_variables_c_restrict_flag = 0;
    }

;

/* ISO/IEC 9899:1999 6.7 */
declaration_specifiers:		 
   type_specifier
    {
        $$ = $1;
	compute_modulus_for_declaration($$); 
    }
  | type_qualifier type_specifier
    {
        $$ = Speclist_right($1, $2);
	compute_modulus_for_declaration($$); 
    }
  | struct_or_union_specifier
    {
        $$ = $1;
    }
  | TYPEDEF struct_or_union_specifier
    {
	$$ = Speclist_right(StClassSpec(SPEC_typedef), $2);
        isTypedef = 1;
    }
   | typedef_name
   {
      $$ = Usertype($1);
   }
;

typedef_name:
    TYPE_NAME
    {
        $$ = Symbol($1);
    }
;

/*  ISO/IEC 9899:1999 6.7 */
init_declarator_list:		
    init_declarator
    {
      $$ = $1;
      // this is where the first time initilization takes place
    //   printf("one and only init-declaration %s\n", $1); 
    //   declis = 6;
    //   set_security_flag_expr($$, $1, NULL, -1);
    //   declis = 0;
    }
  | init_declarator_list ',' init_declarator
    {
      $$ = DeclList($1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.7 */
/* This is the only place to come for a full declaration.
 * Other declarator calls are not of particular interest.
 * Also, note that we cannot do it in a parent rule (e.g. in 
 * "declaration" since initializers may use variables defined
 * previously, in the same declarator list.
 */

init_declarator:	
    declarator
    {
      astdecl s = decl_getidentifier($1);
      int     declkind = decl_getkind($1);
      stentry e;
     
      if (!isTypedef && declkind == DFUNC && strcmp(s->u.id->name, "main") == 0)
        s->u.id = Symbol(MAIN_NEWNAME);       /* Catch main()'s declaration */
      if (checkDecls) 
      {
        e = symtab_put(stab, s->u.id, (isTypedef) ? TYPENAME :
                                       (declkind == DFUNC) ? FUNCNAME : IDNAME);
        e->isarray = (declkind == DARRAY);
        if (declkind == DARRAY) {
            global_variables_c_restrict_flag = 0;
        }
      }
      $$ = $1;
    //   // this is where the first time initilization takes place
    //   printf("init-declaration-1 %s\n", $1); 
    //   declis = 6;
    // //   set_security_flag_expr($$, $1, NULL, -1);
    //   declis = 0;
    }
  | declarator '=' 
    {
      astdecl s = decl_getidentifier($1);
      int     declkind = decl_getkind($1);
      stentry e;
      
      if (!isTypedef && declkind == DFUNC && strcmp(s->u.id->name, "main") == 0)
        s->u.id = Symbol(MAIN_NEWNAME);         /* Catch main()'s declaration */
      if (checkDecls) 
      {
        e = symtab_put(stab, s->u.id, (isTypedef) ? TYPENAME :
                                       (declkind == DFUNC) ? FUNCNAME : IDNAME);
        e->isarray = (declkind == DARRAY);
        if (declkind == DARRAY) {
            global_variables_c_restrict_flag = 0;
        }
      }
    }
    initializer
    {
      $$ = InitDecl($1, $4);
	
    }
;



/*  ISO/IEC 9899:1999 6.7.2 */
type_specifier:
  INT
    {
      $$ = Declspec(SPEC_int, INT_SIZE); // 32
    }
  | CHAR
    {
      $$ = Declspec(SPEC_int, CHAR_SIZE); // 8
      parse_warning("Replacing type 'char' with 'int<%d>'.\n", CHAR_SIZE);
    }
  | SHORT
    {
      $$ = Declspec(SPEC_int, SHORT_SIZE); // 16
      parse_warning("Replacing type 'short' with 'int<%d>'.\n", SHORT_SIZE);
    }
  | LONG
    {
      $$ = Declspec(SPEC_int, LONG_SIZE); // 64
      parse_warning("Replacing type 'long' with 'int<%d>'.\n", LONG_SIZE);
    }
  | FLOAT
    {
      $$ = DeclspecFloat(SPEC_float, FLOAT_MAN_SIZE, FLOAT_EXP_SIZE); // MAN-32, EXP-9
    }
  | VOID
    {
      $$ = Declspec(SPEC_void, 0);
    }
  | INT '<' CONSTANT '>'
    {
      $$ = Declspec(SPEC_int, atoi($3));
    }
  | FLOAT '<' CONSTANT ',' CONSTANT '>'
    {
      $$ = DeclspecFloat(SPEC_float, atoi($3), atoi($5)); 
    }
;

type_qualifier:
    PRIVATE
    {
        $$ = Declspec(SPEC_private, 0);
    }
  | PUBLIC
    {
        $$ = Declspec(SPEC_public, 0);
    }
;

/*  ISO/IEC 9899:1999 6.7.2.1 */
struct_or_union:
    STRUCT
    {
        $$ = SPEC_struct;
    }
    | UNION
    {
        $$ = SPEC_union;
    }
;

struct_declaration_list:
    struct_declaration
    {
        $$ = $1;
    }
    | struct_declaration_list struct_declaration
    {
        $$ = StructfieldList($1, $2);
    }
;

/*  ISO/IEC 9899:1999 6.7.2.1 */
struct_declaration:
    specifier_qualifier_list struct_declarator_list ';'
    {
        $$ = StructfieldDecl($1, $2);
    }
    | specifier_qualifier_list ';'        /* added rule; just for GCC's shake */
    {
        $$ = StructfieldDecl($1, NULL);
    }
;

/*  ISO/IEC 9899:1999 6.7.2.1 */
struct_declarator_list:
    struct_declarator
    {
        $$ = $1;
    }
    | struct_declarator_list ',' struct_declarator
    {
        $$ = DeclList($1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.7.2.1 */
struct_declarator:
    declarator
    {
      astdecl s = decl_getidentifier($1);
      int     declkind = decl_getkind($1);
      stentry e;
      $$ = $1;
    }
    | declarator ':' constant_expression
    {
        $$ = BitDecl($1, $3);
    }
    | ':' constant_expression
    {
        $$ = BitDecl(NULL, $2);
    }
;

/*  ISO/IEC 9899:1999 6.7.2.1 */
struct_or_union_specifier:
    struct_or_union '{' struct_declaration_list '}'
    {
        $$ = SUdecl($1, NULL, $3);
    }
    | struct_or_union '{' '}' /* NON-ISO empty declaration (added by SM) */
    {
        $$ = SUdecl($1, NULL, NULL);
    }
    | struct_or_union IDENTIFIER '{' struct_declaration_list '}'
    {
	symbol s = Symbol($2);
	stentry e; 
	/* Well, struct & union names have their own name space, and
         * their own scopes. I.e. they can be re-declare in nested
         * scopes. We don't do any kind of duplicate checks.
         */
	
        if (checkDecls)
	{
        	e = symtab_put(stab, s, SUNAME);
		e->field = $4; 
	}
        $$ = SUdecl($1, s, $4);
	store_struct_information(struct_table, $$); 
    }
    /* If we have "typedef struct X X;" then X will become a TYPE_NAME
     * from now on, altough it is also a SUNAME. Thus it won't be matched
     * by the previous rule -- this explains the following one!
     */
    | struct_or_union typedef_name '{' struct_declaration_list '}'
    {
        symbol s = $2;
        if (checkDecls)
            symtab_put(stab, s, SUNAME);
            $$ = SUdecl($1, s, $4);
    }
    | struct_or_union IDENTIFIER
    {
        symbol s = Symbol($2);
        if (checkDecls)
        symtab_put(stab, s, SUNAME);
        $$ = SUdecl($1, s, NULL);
    }
    | struct_or_union typedef_name       /* As above! */
    {
        symbol s = $2;
        if (checkDecls)
        symtab_put(stab, s, SUNAME);
        $$ = SUdecl($1, s, NULL);
    }
;

/*  ISO/IEC 9899:1999 6.7.5 */

pointer:
    '*'
    {
        $$ = Pointer();
        pointer_flag = 1;
    }
    | '*' type_qualifier
    {
        $$ = Speclist_right(Pointer(), $2);
        pointer_flag = 1;
    }
    | '*' pointer
    {
        $$ = Speclist_right(Pointer(), $2);
        pointer_flag = 1;
    }
    | '*' type_qualifier pointer
    {
        $$ = Speclist_right( Pointer(), Speclist_left($2, $3) );
        pointer_flag = 1;
    }
;

parameter_type_list:
    parameter_list
    {
        $$ = $1;
    }
   /* | parameter_list ',' ELLIPSIS
    {
        $$ = ParamList($1, Ellipsis());
    }*/
;

parameter_list:
    parameter_declaration
    {
        $$ = $1;
    }
    | parameter_list ',' parameter_declaration
    {
        $$ = ParamList($1, $3);
    }
;

parameter_declaration:
    declaration_specifiers declarator
    {
        $$ = ParamDecl($1, $2);
	set_pointer_flag($1, $2); 
    }   
    | declaration_specifiers
    {
        $$ = ParamDecl($1, NULL);
    }
    | declaration_specifiers abstract_declarator
    {
        $$ = ParamDecl($1, $2);
	set_pointer_flag($1, $2); 
    }
;

/*  ISO/IEC 9899:1999 6.7.6 */
abstract_declarator:
    pointer
    {
        $$ = AbstractDeclarator($1, NULL);
    }
    | direct_abstract_declarator
    {
        $$ = AbstractDeclarator(NULL, $1);
    }
    | pointer direct_abstract_declarator
    {
        $$ = AbstractDeclarator($1, $2);
    }
;
/*  ISO/IEC 9899:1999 6.7.6 */
direct_abstract_declarator:
    '(' abstract_declarator ')'
    {
        $$ = ParenDecl($2);
    }
    | '[' ']'
    {
        global_variables_c_restrict_flag = 0;
        $$ = ArrayDecl(NULL, NULL, NULL);
    }
    | direct_abstract_declarator '[' ']'
    {
        global_variables_c_restrict_flag = 0;
        $$ = ArrayDecl($1, NULL, NULL);
    }
    | '[' assignment_expression ']'
    {
        global_variables_c_restrict_flag = 0;
        $$ = ArrayDecl(NULL, NULL, $2);
    }
    | direct_abstract_declarator '[' assignment_expression ']'
    {
        global_variables_c_restrict_flag = 0;
        $$ = ArrayDecl($1, NULL, $3);
    }
    | '[' '*' ']'
    {
        global_variables_c_restrict_flag = 0;
        $$ = ArrayDecl(NULL, Declspec(SPEC_star, 0), NULL);
    }
    | direct_abstract_declarator '[' '*' ']'
    {
        global_variables_c_restrict_flag = 0;
        $$ = ArrayDecl($1, Declspec(SPEC_star, 0), NULL);
    }
    | '(' ')'
    {
        $$ = FuncDecl(NULL, NULL);
    }
    | direct_abstract_declarator '(' ')'
    {
        $$ = FuncDecl($1, NULL);
    }
    | '(' parameter_type_list ')'
    {
        $$ = FuncDecl(NULL, $2);
    }
    | direct_abstract_declarator '(' parameter_type_list ')'
    {
        $$ = FuncDecl($1, $3);
    }
;

/*  ISO/IEC 9899:1999 6.7.2.1 */
specifier_qualifier_list:
    type_specifier
    {
      $$ = $1;
      compute_modulus_for_declaration($$); 
    }
  | type_specifier specifier_qualifier_list
    {
      $$ = Speclist_right($1, $2);	
    }
  | type_qualifier
    {
	$$ = $1; 
    }
  | type_qualifier specifier_qualifier_list
   {
	$$ = Speclist_right($1, $2); 
	compute_modulus_for_declaration($$); 
   }
  | struct_or_union_specifier
   {
	$$ = $1; 
   }
;


declarator:			
    direct_declarator
    {
      $$ = Declarator(NULL, $1);
    }
  | pointer direct_declarator
    {
        $$ = Declarator($1, $2);
    }
;


/*  ISO/IEC 9899:1999 6.7.5 */
direct_declarator:
    IDENTIFIER
    {
      
    //   astexpr e = symtab_get(stab, Symbol($1), IDENT);
    //   if (e != NULL && stab->scopelevel > 0) { // Handles only variable redifination  
    //     parse_error(1, "Redefinition of '%s'.\n", $1);
    //   }
      $$ = IdentifierDecl( Symbol($1) ); 
    }
  | '(' declarator ')'
    {
      /* Try to simplify a bit: (ident) -> ident */
      if ($2->spec == NULL && $2->decl->type == DIDENT)
        $$ = $2->decl;
      else
        $$ = ParenDecl($2);
    }
  | direct_declarator '[' ']'
    {
      global_variables_c_restrict_flag = 0;
      $$ = ArrayDecl($1, NULL, NULL);
    }
  | direct_declarator '[' assignment_expression ']' // array with variable length
    {
      global_variables_c_restrict_flag = 0;
      $$ = ArrayDecl($1, NULL, $3);
    }
  | direct_declarator '(' ')'
    {
      $$ = FuncDecl($1, NULL);
    }
  | direct_declarator '(' identifier_list ')'
    {
      $$ = FuncDecl($1, $3);
    }
  | direct_declarator '(' parameter_type_list ')'
    {
      $$ = FuncDecl($1, $3);
    }
;


/*  ISO/IEC 9899:1999 6.7.5 */
identifier_list:		
    IDENTIFIER
    {
      $$ = IdentifierDecl( Symbol($1) );
    }
  | identifier_list ',' IDENTIFIER
    {
      $$ = IdList($1, IdentifierDecl( Symbol($3) ));
    }
;



/*  ISO/IEC 9899:1999 6.7.8 */
initializer:			
    assignment_expression
    {
      $$ = $1;
    }
  | '{' initializer_list '}'
    {
      $$ = BracedInitializer($2);
    }
  | '{' initializer_list ',' '}'
    {
      $$ = BracedInitializer($2);
    }
;

/*  ISO/IEC 9899:1999 6.7.8 */
initializer_list:
    initializer
    {
      $$ = $1;
    }
  | designation initializer
    {
      $$ = Designated($1, $2);
    }
  | initializer_list ',' initializer
    {
      $$ = CommaList($1, $3);
    }
  | initializer_list ',' designation initializer
    {
      $$ = CommaList($1, Designated($3, $4));
    }
;

/*  ISO/IEC 9899:1999 6.7.8 */
designation:
    designator_list '='
    {
      $$ = $1; 
    }
;

/*  ISO/IEC 9899:1999 6.7.8 */
designator_list:
    designator
    {
      $$ = $1;
    }
  | designator_list designator
    {
      $$ = SpaceList($1, $2);
    }
;

/*  ISO/IEC 9899:1999 6.7.8 */
designator:
    '[' constant_expression ']'
     {
        security_check_for_condition($2);
        $$ = IdxDesignator($2);
     }
    | '.' IDENTIFIER
     {
        $$ = DotDesignator( Symbol($2) );
     }
    | '.' typedef_name     /* artificial rule */
    {
        $$ = DotDesignator($2);
    }


;


/* -------------------------------------------------------------------------
 * ------------- ISO/IEC 9899:1999 A.2.3 Statements ------------------------
 * -------------------------------------------------------------------------
 */

/*  ISO/IEC 9899:1999 6.8 */
statement:
  compound_statement
    {
      $$ = $1;
      $$->flag = $1->flag;
    }
  | expression_statement // this is where expressions gets handled 
    {
      $$ = $1;
      $$->flag = $1->flag;
    }
  | jump_statement
    {
      $$ = $1;
      $$->flag = $1->flag; 
    }
  | selection_statement
    {
      $$ = $1;
      $$->flag = $1->flag;
    }
  | iteration_statement
    {
      $$ = $1;
      $$->flag = $1->flag; 
    }
  | smc_statement
    {
      $$ = $1;
      $$->flag = $1->flag; 
    }
  | batch_statement
    {
      $$ = $1;
      $$->flag = $1->flag; 
    }
      /*by ghada ---------------------------------------------------------*/
  | pfree_statement
    {
      $$ = $1;
      $$->flag = $1->flag;
    }
  /*--------------------------------------------------------------------*/
  | openmp_construct // OpenMP Version 2.5 ISO/IEC 9899:1999 addition
    {
     $$ = OmpStmt($1);
     $$->l = $1->l;
    }
 | ompix_construct // OMPi extensions
  {
    $$ = OmpixStmt($1);
    $$->l = $1->l;
  }
 | parallel_statement
   {
        $$ = $1;
   }
;

parallel_statement:
        parallel_scope
        {
                $$ = OmpStmt(OmpConstruct(DCPARSECTIONS, OmpDirective(DCPARSECTIONS, NULL), $1));
                $$->l = $1->l;
		thread_id = -1; 
        }
;

increase_thread_id: 
	{
		thread_id++; 
		if(thread_id >= num_threads)
			num_threads = thread_id+1; 
	}
; 
	
parallel_scope:
        parallel_sequence
        {
                $$ = Compound($1);
        }
;

parallel_item: 
	increase_thread_id '[' block_item_list ']'
        {
                $$ = OmpStmt( OmpConstruct(DCSECTION, OmpDirective(DCSECTION,NULL), Compound($3)) );
        }
;
 	
parallel_sequence:
	parallel_item
	{
		$$ = $1; 
	}
|       parallel_sequence parallel_item 
        {
                $$ = BlockList($1, $2);
        }
;

/*  ISO/IEC 9899:1999 6.8.2 */
compound_statement:
    '{' '}'
    {
      $$ = Compound(NULL);
    }
| '{'  { $<type>$ = sc_original_line()-1; scope_start(stab);}
       block_item_list '}'
    {
      $$ = Compound($3);
      scope_end(stab);
      $$->l = $<type>2;     /* Remember 1st line */
      $$->flag = $3->flag; 
    }
;

/*  ISO/IEC 9899:1999 6.8.2 */
block_item_list:
    block_item
    {
        $$ = $1;
        $$->flag = $1->flag;
    }
  | block_item_list block_item
    {
      $$ = BlockList($1, $2);
      $$->l = $1->l;
      set_security_flag_stmt($$, $1, $2);
    }
;

/*  ISO/IEC 9899:1999 6.8.2 */
block_item:
    declaration
    {
      $$ = $1;
    }
  | statement
    {
      $$ = $1;
      $$->flag = $1->flag;
    }
  | openmp_directive // OpenMP Version 2.5 ISvO/IEC 9899:1999 addition
    {
        $$ = OmpStmt($1);
        $$->l = $1->l;
    }
  | ompix_directive // ompi extensions
    {
        $$ = OmpixStmt($1);
        $$->l = $1->l;
    }
;

/*  ISO/IEC 9899:1999 6.8.3 */
expression_statement:
    ';' // handles empty expressions
    {
      $$ = Expression(NULL);
    }
  | expression ';' // handles expressions with a given value
    {
      $$ = Expression($1);
      $$->l = $1->l;
      $$->flag = $1->flag;
    }
;

/*  ISO/IEC 9899:1999 6.8.4 */
if_subroutine: /*empty rule*/
    {
        tmp_index++;
        if(tmp_index > cond_index)
            cond_index = tmp_index;
    };
selection_statement:
    IF '(' expression ')' if_subroutine statement
    {
        $$ = If($3, $6, NULL);            
        if($3->flag == PRI && $6->flag == PUB)
           parse_error(1, "Public variable assignment is not allowed within a private condition.\n");
        if($3->flag == PRI)
            contain_priv_if_flag = 1; 
        $$->flag = $6->flag;
            tmp_index--;

    }
    | IF '(' expression ')' if_subroutine statement ELSE statement
    {
        $$ = If($3, $6, $8);
        if(($3->flag == PRI && $6->flag == PUB) || ($3->flag == PRI && $8->flag == PUB))
           parse_error(1, "Public variable assignment is not allowed within a private condition.\n");
        if($3->flag == PRI)
            contain_priv_if_flag = 1; 
        if($6->flag == PUB || $8->flag == PUB)
            $$->flag = PUB; 
        else
            $$->flag = PRI;
        tmp_index--;
    }
;

/*  ISO/IEC 9899:1999 6.8.5 */
/* (VVD) broke off the FOR part */
iteration_statement:
    WHILE '(' expression ')' statement
    {
        security_check_for_condition($3); 
        $$ = While($3, $5);
        $$->flag = $5->flag;
    }
    | DO statement WHILE '(' expression ')' ';'
    {
        security_check_for_condition($5);         
        $$ = Do($2, $5);
        $$->flag = $2->flag;
    }
    | iteration_statement_for
    {
        $$->flag = $1->flag;
    }
;

iteration_statement_for:
    FOR '(' ';' ';' ')' statement
    {
        $$ = For(NULL, NULL, NULL, $6);
        $$->flag = $6->flag;
    }
    | FOR '(' expression ';' ';' ')' statement
    {
        $$ = For(Expression($3), NULL, NULL, $7);
        $$->flag = $7->flag;
    }
    | FOR '(' ';' expression ';' ')' statement
    {
        security_check_for_condition($4);         
        $$ = For(NULL, $4, NULL, $7);
        $$->flag = $7->flag;
    }
    | FOR '(' ';' ';' expression ')' statement
    {
        $$ = For(NULL, NULL, $5, $7);
        $$->flag = $7->flag;

    }
    | FOR '(' expression ';' expression ';' ')' statement
    {
        security_check_for_condition($5);                 
        $$ = For(Expression($3), $5, NULL, $8);
        $$->flag = $8->flag;
    }
    | FOR '(' expression ';' ';' expression ')' statement
    {
        $$ = For(Expression($3), NULL, $6, $8);
        $$->flag = $8->flag;
    }
    | FOR '(' ';' expression ';' expression ')' statement
    {
        security_check_for_condition($4);                 
        $$ = For(NULL, $4, $6, $8);
        $$->flag = $8->flag;
    }
    | FOR '(' expression ';' expression ';' expression ')' statement
    {	
        security_check_for_condition($5);         
        $$ = For(Expression($3), $5, $7, $9);
        $$->flag = $9->flag;

    }
    | FOR '(' declaration ';' ')' statement
    {
        $$ = For($3, NULL, NULL, $6);
        $$->flag = $6->flag;

    }
    | FOR '(' declaration expression ';' ')' statement
    {
        $$ = For($3, $4, NULL, $7);
        security_check_for_condition($4);
        $$->flag = $7->flag;

    }
    | FOR '(' declaration ';' expression ')' statement
    {
        $$ = For($3, NULL, $5, $7);
        $$->flag = $7->flag;

    }
    | FOR '(' declaration expression ';' expression ')' statement
    {
        security_check_for_condition($4);                 
        $$ = For($3, $4, $6, $8);
        $$->flag = $8->flag;

    }
;

jump_statement:
    CONTINUE ';'
    {
        $$ = Continue();
    }
    | BREAK ';'
    {
        $$ = Break();
    }
    | RETURN ';'
    {
        $$ = Return(NULL);
    }
    | RETURN expression ';'
    {
        if(($2->flag == PRI && func_return_flag == 0)
           || ($2->flag == PUB && func_return_flag == 1))
	{
		parse_error(-1, "Incorrect return type.\n");
        }
        $$ = Return($2);
        
    }
;

smc_statement:
    /*for single variable*/
    SMCINPUT '(' postfix_expression ',' primary_expression ')' ';'
    {
        int secrecy = 1; /* PRI = 1, PUB = 2 */
        stentry e = get_entry_from_expr($3);
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
        ast_expr_print(arg_str, $5);
       
        fprintf(var_file, "I:%d,%s,%s,%s,1", secrecy, e1->key->name, SPEC_symbols[spec1->subtype], str_string(arg_str));
        if(spec1->subtype == SPEC_int)
                fprintf(var_file, ",%d\n", spec1->size);
        else if(spec1->subtype == SPEC_float)
                fprintf(var_file, ",%d,%d\n", spec1->size, spec1->sizeexp);
	$3->thread_id = thread_id;  
        $$ = Smc(SINPUT, e->spec, $3, $5, NULL, NULL);
    }
/*for one-dimensional array*/
   | SMCINPUT '(' postfix_expression ',' primary_expression ',' expression ')' ';'
    {
        global_variables_c_restrict_flag = 0;
        int secrecy = 1; 
        stentry e = get_entry_from_expr($3);
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
        ast_expr_print(arg_str1, $5);

        /*for arraysize*/
        str arg_str2 = Str("");
        get_arraysize($7, arg_str2);
        fprintf(var_file, "I:%d,%s,%s,%s,%s", secrecy, e1->key->name, SPEC_symbols[spec1->subtype], str_string(arg_str1), str_string(arg_str2));

        if(spec1->subtype == SPEC_int)
                fprintf(var_file, ",%d\n", spec1->size);
        else if(spec1->subtype == SPEC_float)
                fprintf(var_file, ",%d,%d\n", spec1->size, spec1->sizeexp);
	
	$3->thread_id = thread_id; 
        $$ = Smc(SINPUT, e->spec, $3, $5, $7, NULL);
        global_variables_c_restrict_flag = 0;
    }
/*for two-dimensional array*/
    | SMCINPUT '(' postfix_expression ',' primary_expression ',' expression ',' expression')' ';'
    {
        global_variables_c_restrict_flag = 0;
        int secrecy = 1; 
        stentry e = get_entry_from_expr($3);
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
        ast_expr_print(arg_str1, $5);

        /*for arraysize*/
        str arg_str2 = Str("");
        get_arraysize($7, arg_str2);

        str arg_str3 = Str("");
        get_arraysize($9, arg_str3);
        fprintf(var_file, "I:%d,%s,%s,%s,%s,%s", secrecy, e1->key->name, SPEC_symbols[spec1->subtype], str_string(arg_str1), str_string(arg_str2), str_string(arg_str3));
        
        /*for element size*/
       if(spec1->subtype == SPEC_int)
                fprintf(var_file, ",%d\n", spec1->size);
        else if(spec1->subtype == SPEC_float)
                fprintf(var_file, ",%d,%d\n", spec1->size, spec1->sizeexp);
	$3->thread_id = thread_id; 
        $$ = Smc(SINPUT, e->spec, $3, $5, $7, $9);
        global_variables_c_restrict_flag = 0;
    }
   | SMCOUTPUT '(' postfix_expression ',' primary_expression ')' ';'
    {
        global_variables_c_restrict_flag = 0;
        int secrecy = 1; /* PRI = 1, PUB = 2 */
        stentry e = get_entry_from_expr($3);
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
        ast_expr_print(arg_str, $5);

        fprintf(var_file, "O:%d,%s,%s,%s,1", secrecy, e1->key->name, SPEC_symbols[spec1->subtype], str_string(arg_str));
        if(spec1->subtype == SPEC_int)
               fprintf(var_file, ",%d\n", spec1->size);
        else if(spec1->subtype == SPEC_float)
               fprintf(var_file, ",%d,%d\n", spec1->size, spec1->sizeexp);
	$3->thread_id = thread_id;
        $$ = Smc(SOUTPUT, e->spec, $3, $5, NULL, NULL);
        global_variables_c_restrict_flag = 0;
    }
 | SMCOUTPUT '(' postfix_expression ',' primary_expression ',' expression ')' ';'
    {
        global_variables_c_restrict_flag = 0;
        int secrecy = 1;
        stentry e = get_entry_from_expr($3);
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
        ast_expr_print(arg_str1, $5);

        /* for arraysize */
        str arg_str2 = Str("");
        get_arraysize($7, arg_str2);

        fprintf(var_file, "O:%d,%s,%s,%s,%s", secrecy, e1->key->name, SPEC_symbols[spec1->subtype], str_string(arg_str1), str_string(arg_str2));
        if(spec1->subtype == SPEC_int)
              fprintf(var_file, ",%d\n", spec1->size);
        else if(spec1->subtype == SPEC_float)
              fprintf(var_file, ",%d,%d\n", spec1->size, spec1->sizeexp);
	$3->thread_id = thread_id; 
	
	$$ = Smc(SOUTPUT, e->spec, $3, $5, $7, NULL);
    global_variables_c_restrict_flag = 0;
    }
  | SMCOUTPUT '(' postfix_expression ',' primary_expression ',' expression ',' expression')' ';'
    {
        global_variables_c_restrict_flag = 0;
        int secrecy = 1;
        stentry e = get_entry_from_expr($3);
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
        ast_expr_print(arg_str1, $5);
        /* for arraysize */
        str arg_str2 = Str("");
        get_arraysize($7, arg_str2);

        str arg_str3 = Str("");
        get_arraysize($9, arg_str3);

        fprintf(var_file, "O:%d,%s,%s,%s,%s,%s", secrecy, e1->key->name, SPEC_symbols[spec1->subtype], str_string(arg_str1), str_string(arg_str2), str_string(arg_str3));
        /* for element size */
        if(spec1->subtype == SPEC_int)
               fprintf(var_file, ",%d\n", spec1->size);
        else if(spec1->subtype == SPEC_float)
               fprintf(var_file, ",%d,%d\n", spec1->size, spec1->sizeexp);
	$3->thread_id = thread_id; 
        $$ = Smc(SOUTPUT, e->spec, $3, $5, $7, $9);
        global_variables_c_restrict_flag = 0;
    }
 ;

batch_statement:
    FOR '(' expression ';' expression ';' expression ')' '[' block_item_list ']'
    {
        security_check_for_condition($5);
        //$$->flag = $10->flag;
        $$ = Batch(Expression($3), $5, $7, $10);
    }
;

//By Ghada ----------------------------------
pfree_statement:        
    PFREE '(' primary_expression ')' ';'
    {
        $$ = Pfree($3);
    }   
;

/* -------------------------------------------------------------------------
 * ------ ISO/IEC 9899:1999 A.2.4 External definitions ---------------------
 * -------------------------------------------------------------------------
 */
    
/*  ISO/IEC 9899:1999 6.9 */
translation_unit:
    external_declaration
    {
	$$ = pastree = $1;  
    $$->gflag = 1; // pastree is the generated AST, this does have a gflag that is defaulted to 0 or global-public
    global_variables_c_restrict_flag = 1;
    }
  | translation_unit external_declaration
    {
      $$ = pastree = BlockList($1, $2); 
      $$->gflag = 1;
      global_variables_c_restrict_flag = 1;
    }
;

/*  ISO/IEC 9899:1999 6.9 */
external_declaration:
  function_definition
    {
      $$ = $1;
    }
  | declaration
    {
      $$ = $1; 
      $$->gflag = 1; 
      global_variables_c_restrict_flag = 0;
    }
    /* Actually, although not in the grammar, we support 1 more option
     * here:  Verbatim
     */
  | ox_taskdef_construct
    {
        $$ = OmpixStmt($1);
    };

/*  ISO/IEC 9899:1999 6.9.1 */
/* We open a new scope which encloses the compound statement.
 * In there we will only declare the function parameters.
 * We break the rule into two subrules, for modularity. 
 */
function_definition:
    normal_function_definition   { $$ = $1; }
;

normal_function_definition:
    declaration_specifiers declarator
    {
      func_return_flag = 0;
      tmp_float_index = 1; 
      tmp_index = 1;
      num_index = 0;
      num_float_index = 0; 
      cond_index = 0;
      contain_priv_if_flag = 0;
        
      if (isTypedef || $2->decl->type != DFUNC)
        parse_error(1, "Function definition cannot be parsed.\n");
      if (symtab_get(stab, decl_getidentifier_symbol($2), FUNCNAME) == NULL)
        symtab_put_funcname(stab, decl_getidentifier_symbol($2), FUNCNAME, $1, $2);
        symbol e = decl_getidentifier_symbol($2);

     symbol s = decl_getidentifier_symbol($2);
     scope_start(stab);
     ast_declare_function_params($2);
     if(set_security_flag_spec($1) == PRI)
         func_return_flag = 1;
     }
    compound_statement
    {
      scope_end(stab);
      check_for_main_and_declare($1, $2);
      $$ = FuncDef($1, $2, NULL, $4);
      if(contain_priv_if_flag){
          if(cond_index > num_index)
                $$->num_tmp = cond_index;
          else
                $$->num_tmp = num_index;
       }
      else
           $$->num_tmp = num_index;
      $$->num_float_tmp = num_float_index; 
      $$->contain_priv_if_flag = contain_priv_if_flag;

      if(is_priv_int_ptr_appear == 1)
      {
        $$->is_priv_int_ptr_appear = 1; 
        is_priv_int_ptr_appear = 0;  
      } else {
	    $$->is_priv_int_ptr_appear = 0; 
      }

      if(is_priv_float_ptr_appear == 1)
      {
	$$->is_priv_float_ptr_appear = 1; 
	is_priv_float_ptr_appear = 0;  
      }
      else 
	$$->is_priv_float_ptr_appear = 0; 
      
      if(is_priv_int_index_appear == 1)
      {
	$$->is_priv_int_index_appear = 1; 
	is_priv_int_index_appear = 0;  
      }
      else 
	$$->is_priv_int_index_appear = 0; 

      if(is_priv_float_index_appear == 1)
      {
	$$->is_priv_float_index_appear = 1; 
	is_priv_float_index_appear = 0;  
      }
      else 
	$$->is_priv_float_index_appear = 0; 
      
      if(is_priv_int_struct_field_appear == 1)
      {
	$$->is_priv_int_struct_field_appear = 1; 
	is_priv_int_struct_field_appear = 0;  
      }
      else 
	$$->is_priv_int_struct_field_appear = 0; 
      
      if(is_priv_float_struct_field_appear == 1)
      {
	$$->is_priv_float_struct_field_appear = 1; 
	is_priv_float_struct_field_appear = 0;  
      }
      else 
	$$->is_priv_float_struct_field_appear = 0; 

      if(is_priv_struct_ptr_struct_field_appear == 1)
      {
	$$->is_priv_struct_ptr_struct_field_appear = 1; 
	is_priv_struct_ptr_struct_field_appear = 0;  
      }
      else 
	$$->is_priv_struct_ptr_struct_field_appear = 0; 
    
      if(is_priv_int_ptr_struct_field_appear == 1)
      {
	$$->is_priv_int_ptr_struct_field_appear = 1; 
	is_priv_int_ptr_struct_field_appear = 0;  
      }
      else 
	$$->is_priv_int_ptr_struct_field_appear = 0; 
      
      if(is_priv_float_ptr_struct_field_appear == 1)
      {
	$$->is_priv_float_ptr_struct_field_appear = 1; 
	is_priv_float_ptr_struct_field_appear = 0;  
      }
      else 
	$$->is_priv_float_ptr_struct_field_appear = 0; 
   }
  | declarator /* no return type */
    {
      tmp_index = 1;
      tmp_float_index = 1; 
      num_index = 0;
      num_float_index = 0; 
      cond_index = 0;
      contain_priv_if_flag = 0;
      if (isTypedef || $1->decl->type != DFUNC)
        parse_error(1, "Function definition cannot be parsed.\n");
      if (symtab_get(stab, decl_getidentifier_symbol($1), FUNCNAME) == NULL)
        symtab_put_funcname(stab, decl_getidentifier_symbol($1), FUNCNAME, NULL, $1);
        symbol e = decl_getidentifier_symbol($1);

      scope_start(stab);
      ast_declare_function_params($1);
    }
    compound_statement
    {
      astspec s = Declspec(SPEC_int, 0);  /* return type defaults to "int" */
      scope_end(stab);
      check_for_main_and_declare(s, $1);
      $$ = FuncDef(s, $1, NULL, $3);
      
      if(contain_priv_if_flag){
          if(cond_index > num_index)
              $$->num_tmp = cond_index;
          else
              $$->num_tmp = num_index;
      }
      else
          $$->num_tmp = num_index;
      $$->num_float_tmp = num_float_index; 
      $$->contain_priv_if_flag = contain_priv_if_flag;
      
      if(is_priv_int_ptr_appear == 1)
      {
        $$->is_priv_int_ptr_appear = 1; 
        is_priv_int_ptr_appear = 0;  
      } else {
	    $$->is_priv_int_ptr_appear = 0; 
      }

      if(is_priv_float_ptr_appear == 1)
      {
	$$->is_priv_float_ptr_appear = 1; 
	is_priv_float_ptr_appear = 0;  
      }
      else 
	$$->is_priv_float_ptr_appear = 0; 
      
      if(is_priv_int_index_appear == 1)
      {
	$$->is_priv_int_index_appear = 1; 
	is_priv_int_index_appear = 0;  
      }
      else 
	$$->is_priv_int_index_appear = 0; 
      
      if(is_priv_float_index_appear == 1)
      {
	$$->is_priv_float_index_appear = 1; 
	is_priv_float_index_appear = 0;  
      }
      else 
	$$->is_priv_float_index_appear = 0; 
    }
;
/* -------------------------------------------------------------------------
 * --- OpenMP Version 2.5 ISO/IEC 9899:1999 additions begin ----------------
 * -------------------------------------------------------------------------
 */

openmp_construct:
    parallel_construct
    {
        $$ = $1;
    }
    | for_construct
    {
        $$ = $1;
    }
    | sections_construct
    {
        $$ = $1;
    }
    | single_construct
    {
        $$ = $1;
    }
    | parallel_for_construct
    {
        $$ = $1;
    }
    | parallel_sections_construct
    {
        $$ = $1;
    }
    | master_construct
    {
        $$ = $1;
    }
    | critical_construct
    {
        $$ = $1;
    }
    | atomic_construct
    {
        $$ = $1;
    }
    | ordered_construct
    {
        $$ = $1;
    }
    | /* OpenMP V3.0 */
    task_construct
    {
        $$ = $1;
    }
;

openmp_directive:
/*
 pomp_construct
 {
 $$ = $1;
 }
 | */
/* We create constructs out of the next two directive-only rules,
 * for uniformity.
 */
    barrier_directive
    {
        $$ = OmpConstruct(DCBARRIER, $1, NULL);
    }
    | flush_directive
    {
        $$ = OmpConstruct(DCFLUSH, $1, NULL);
    }
    | /* OpenMP V3.0 */
    taskwait_directive
    {
        $$ = OmpConstruct(DCTASKWAIT, $1, NULL);
    }
    | /* OpenMP V3.1 */
    taskyield_directive
    {
        $$ = OmpConstruct(DCTASKYIELD, $1, NULL);
    }
;

structured_block:
    statement
    {
        $$ = $1;
    }
;

parallel_construct:
    parallel_directive structured_block
    {
        $$ = OmpConstruct(DCPARALLEL, $1, $2);
        $$->l = $1->l;
    }
;

parallel_directive:
    PRAGMA_OMP OMP_PARALLEL parallel_clause_optseq '\n'
    {
        $$ = OmpDirective(DCPARALLEL, $3);
    }
;

parallel_clause_optseq:
// empty
    {
        $$ = NULL;
    }
    | parallel_clause_optseq parallel_clause
    {
        $$ = OmpClauseList($1, $2);
    }
    | parallel_clause_optseq ',' parallel_clause
    {
        $$ = OmpClauseList($1, $3);
    }
;

parallel_clause:
    unique_parallel_clause
    {
        $$ = $1;
    }
    | data_clause
    {
        $$ = $1;
    }
;

unique_parallel_clause:
    OMP_IF '(' { sc_pause_openmp(); } expression ')'
    {
        sc_start_openmp();
        $$ = IfClause($4);
    }
    | OMP_NUMTHREADS '(' { sc_pause_openmp(); } expression ')'
    {
        sc_start_openmp();
        $$ = NumthreadsClause($4);
    }
;

for_construct:
    for_directive iteration_statement_for
    {
        $$ = OmpConstruct(DCFOR, $1, $2);
    }
;

for_directive:
    PRAGMA_OMP OMP_FOR for_clause_optseq '\n'
    {
        $$ = OmpDirective(DCFOR, $3);
    }
;

for_clause_optseq:
// empty
    {
        $$ = NULL;
    }
    | for_clause_optseq for_clause
    {
        $$ = OmpClauseList($1, $2);
    }
    | for_clause_optseq ',' for_clause
    {
        $$ = OmpClauseList($1, $3);
    }
;

for_clause:
    unique_for_clause
    {
        $$ = $1;
    }
    | data_clause
    {
        $$ = $1;
    }
    | OMP_NOWAIT
    {
        $$ = PlainClause(OCNOWAIT);
    }
;

unique_for_clause:
    OMP_ORDERED
    {
        $$ = PlainClause(OCORDERED);
    }
    | OMP_SCHEDULE '(' schedule_kind ')'
    {
        $$ = ScheduleClause($3, NULL);
    }
    | OMP_SCHEDULE '(' schedule_kind ',' { sc_pause_openmp(); } expression ')'
    {
        sc_start_openmp();
        if ($3 == OC_runtime)
        parse_error(1, "\"runtime\" schedules may not have a chunksize.\n");
        $$ = ScheduleClause($3, $6);
    }
    | OMP_SCHEDULE '(' OMP_AFFINITY ','
    {  /* non-OpenMP schedule */
        tempsave = checkDecls;
        checkDecls = 0;   /* Because the index of the loop is usualy involved */
        sc_pause_openmp();
    }
    expression ')'
    {
        sc_start_openmp();
        checkDecls = tempsave;
        $$ = ScheduleClause(OC_affinity, $6);
    }
    | OMP_COLLAPSE '(' expression /* CONSTANT */ ')'   /* OpenMP V3.0 */
    {
        int n = 0, er = 0;
        if (xar_expr_is_constant($3))
        {
            n = xar_calc_int_expr($3, &er);
            if (er) n = 0;
        }
        if (n <= 0)
        parse_error(1, "Invalid number in collapse() clause.\n");
        $$ = CollapseClause(n);
    }
;

schedule_kind:
    OMP_STATIC
    {
        $$ = OC_static;
    }
    | OMP_DYNAMIC
    {
        $$ = OC_dynamic;
    }
    | OMP_GUIDED
    {
        $$ = OC_guided;
    }
    | OMP_RUNTIME
    {
        $$ = OC_runtime;
    }
    | OMP_AUTO      /* OpenMP 3.0 */
    {
        $$ = OC_auto;
    }
    | error { parse_error(1, "Invalid openmp schedule type.\n"); }

;

sections_construct:
    sections_directive section_scope
    {
        $$ = OmpConstruct(DCSECTIONS, $1, $2);
    }
;

sections_directive:
    PRAGMA_OMP OMP_SECTIONS sections_clause_optseq '\n'
    {
        $$ = OmpDirective(DCSECTIONS, $3);
    }
;

sections_clause_optseq:
// empty
    {
        $$ = NULL;
    }
    | sections_clause_optseq sections_clause
    {
        $$ = OmpClauseList($1, $2);
    }
    | sections_clause_optseq ',' sections_clause
    {
        $$ = OmpClauseList($1, $3);
    }
;

sections_clause:
    data_clause
    {
        $$ = $1;
    }
    | OMP_NOWAIT
    {
        $$ = PlainClause(OCNOWAIT);
    }
;

section_scope:
    '{' section_sequence '}'
    {
        $$ = Compound($2);
    }
;

section_sequence:
    structured_block  // 1 shift/reduce conflict here
    {
        /* Make it look like it had a section pragma */
        $$ = OmpStmt( OmpConstruct(DCSECTION, OmpDirective(DCSECTION,NULL), $1) );
    }
    | section_directive structured_block
    {
        $$ = OmpStmt( OmpConstruct(DCSECTION, $1, $2) );
    }
    | section_sequence section_directive structured_block
    {
        $$ = BlockList($1, OmpStmt( OmpConstruct(DCSECTION, $2, $3) ));
    }
;

section_directive:
    PRAGMA_OMP OMP_SECTION '\n'
    {
        $$ = OmpDirective(DCSECTION, NULL);
    }
;

single_construct:
    single_directive structured_block
    {
        $$ = OmpConstruct(DCSINGLE, $1, $2);
    }
;

single_directive:
    PRAGMA_OMP OMP_SINGLE single_clause_optseq '\n'
    {
        $$ = OmpDirective(DCSINGLE, $3);
    }
;

single_clause_optseq:
    // empty
    {
        $$ = NULL;
    }
    | single_clause_optseq single_clause
    {
        $$ = OmpClauseList($1, $2);
    }
    | single_clause_optseq ',' single_clause
    {
        $$ = OmpClauseList($1, $3);
    }
;

single_clause:
    data_clause
    {
        $$ = $1;
    }
    | OMP_NOWAIT
    {
        $$ = PlainClause(OCNOWAIT);
    }
;

parallel_for_construct:
    parallel_for_directive iteration_statement_for
    {
        $$ = OmpConstruct(DCPARFOR, $1, $2);
    }
;

parallel_for_directive:
    PRAGMA_OMP OMP_PARALLEL OMP_FOR parallel_for_clause_optseq '\n'
    {
        $$ = OmpDirective(DCPARFOR, $4);
    }
;

parallel_for_clause_optseq:
    // empty
    {
        $$ = NULL;
    }
    | parallel_for_clause_optseq parallel_for_clause
    {
        $$ = OmpClauseList($1, $2);
    }
    | parallel_for_clause_optseq ',' parallel_for_clause
    {
        $$ = OmpClauseList($1, $3);
    }
;

parallel_for_clause:
    unique_parallel_clause
    {
        $$ = $1;
    }
    | unique_for_clause
    {
        $$ = $1;
    }
    | data_clause
    {
        $$ = $1;
    }
;

parallel_sections_construct:
    parallel_sections_directive section_scope
    {
        $$ = OmpConstruct(DCPARSECTIONS, $1, $2);
    }
;

parallel_sections_directive:
    PRAGMA_OMP OMP_PARALLEL OMP_SECTIONS parallel_sections_clause_optseq '\n'
    {
        $$ = OmpDirective(DCPARSECTIONS, $4);
    }
;

parallel_sections_clause_optseq:
// empty
    {
        $$ = NULL;
    }
    | parallel_sections_clause_optseq parallel_sections_clause
    {
        $$ = OmpClauseList($1, $2);
    }
    | parallel_sections_clause_optseq ',' parallel_sections_clause
    {
        $$ = OmpClauseList($1, $3);
    }
;

parallel_sections_clause:
    unique_parallel_clause
    {
        $$ = $1;
    }
    | data_clause
    {
        $$ = $1;
    }
;

/* OpenMP V3.0 */
task_construct:
    task_directive structured_block
    {
        $$ = OmpConstruct(DCTASK, $1, $2);
        $$->l = $1->l;
    }
;

/* OpenMP V3.0 */
task_directive:
    PRAGMA_OMP OMP_TASK task_clause_optseq '\n'
    {
        $$ = OmpDirective(DCTASK, $3);
    }
;

/* OpenMP V3.0 */
task_clause_optseq:
// empty
    {
        $$ = NULL;
    }
    | task_clause_optseq task_clause
    {
        $$ = OmpClauseList($1, $2);
    }
    | task_clause_optseq ',' task_clause
    {
        $$ = OmpClauseList($1, $3);
    }
;

/* OpenMP V3.0 */
task_clause:
    unique_task_clause
    {
        $$ = $1;
    }
    | data_clause
    {
        $$ = $1;
    }
;

/* OpenMP V3.0 */
unique_task_clause:
    OMP_IF '(' { sc_pause_openmp(); } expression ')'
    {
        sc_start_openmp();
        $$ = IfClause($4);
    }
    | OMP_UNTIED
    {
        $$ = PlainClause(OCUNTIED);
    }
    | OMP_FINAL '(' { sc_pause_openmp(); } expression ')'
    {
        sc_start_openmp();
        $$ = FinalClause($4);
    }
    | OMP_MERGEABLE
    {
        $$ = PlainClause(OCMERGEABLE);
    };

master_construct:
    master_directive structured_block
    {
        $$ = OmpConstruct(DCMASTER, $1, $2);
    }
;

master_directive:
    PRAGMA_OMP OMP_MASTER '\n'
    {
        $$ = OmpDirective(DCMASTER, NULL);
    }
;

critical_construct:
    critical_directive structured_block
    {
        $$ = OmpConstruct(DCCRITICAL, $1, $2);
    }
;

critical_directive:
    PRAGMA_OMP OMP_CRITICAL '\n'
    {
        $$ = OmpCriticalDirective(NULL);
    }
    | PRAGMA_OMP OMP_CRITICAL region_phrase '\n'
    {
        $$ = OmpCriticalDirective($3);
    }
;

region_phrase:
    '(' IDENTIFIER ')'
    {
        $$ = Symbol($2);
    }
;

/* OpenMP V3.0 */
taskwait_directive:
    PRAGMA_OMP OMP_TASKWAIT '\n'
    {
        $$ = OmpDirective(DCTASKWAIT, NULL);
    }
;

/* OpenMP V3.1 */
taskyield_directive:
    PRAGMA_OMP OMP_TASKYIELD '\n'
    {
        $$ = OmpDirective(DCTASKYIELD, NULL);
    }
;

barrier_directive:
    PRAGMA_OMP OMP_BARRIER '\n'
    {
        $$ = OmpDirective(DCBARRIER, NULL);
    }
;

atomic_construct:
    atomic_directive expression_statement
    {
        $$ = OmpConstruct(DCATOMIC, $1, $2);
    }
;

atomic_directive:
    PRAGMA_OMP OMP_ATOMIC '\n'
    {
        $$ = OmpDirective(DCATOMIC, NULL);
    }
    | PRAGMA_OMP OMP_ATOMIC OMP_WRITE'\n'
    {
        $$ = OmpDirective(DCATOMIC, NULL);
    }
    | PRAGMA_OMP OMP_ATOMIC OMP_READ'\n'
    {
        $$ = OmpDirective(DCATOMIC, NULL);
    }
    | PRAGMA_OMP OMP_ATOMIC OMP_UPDATE'\n'
    {
        $$ = OmpDirective(DCATOMIC, NULL);
    }
;

flush_directive:
    PRAGMA_OMP OMP_FLUSH '\n'
    {
        $$ = OmpFlushDirective(NULL);
    }
    | PRAGMA_OMP OMP_FLUSH flush_vars '\n'
    {
        $$ = OmpFlushDirective($3);
    }
;

flush_vars:
    '(' { sc_pause_openmp(); } variable_list ')'
    {
        sc_start_openmp();
        $$ = $3;
    }
;

ordered_construct:
    ordered_directive structured_block
    {
        $$ = OmpConstruct(DCORDERED, $1, $2);
    }
;

ordered_directive:
    PRAGMA_OMP OMP_ORDERED '\n'
    {
        $$ = OmpDirective(DCORDERED, NULL);
    }
;

threadprivate_directive:
    PRAGMA_OMP_THREADPRIVATE '(' thrprv_variable_list ')' '\n'
    {
        $$ = OmpThreadprivateDirective($3);
    }
;

data_clause:
    OMP_PRIVATE { sc_pause_openmp(); } '(' variable_list ')'
    {
        sc_start_openmp();
        $$ = VarlistClause(OCPRIVATE, $4);
    }
    | /* OpenMP 2.0 */
    OMP_COPYPRIVATE  { sc_pause_openmp(); } '(' variable_list ')'
    {
        sc_start_openmp();
        $$ = VarlistClause(OCCOPYPRIVATE, $4);
    }
    | OMP_FIRSTPRIVATE { sc_pause_openmp(); } '(' variable_list ')'
    {
        sc_start_openmp();
        $$ = VarlistClause(OCFIRSTPRIVATE, $4);
    }
    | OMP_LASTPRIVATE { sc_pause_openmp(); } '(' variable_list ')'
    {
        sc_start_openmp();
        $$ = VarlistClause(OCLASTPRIVATE, $4);
    }
    | OMP_SHARED { sc_pause_openmp(); } '(' variable_list ')'
    {
        sc_start_openmp();
        $$ = VarlistClause(OCSHARED, $4);
    }
    | OMP_DEFAULT '(' OMP_SHARED ')'
    {
        $$ = DefaultClause(OC_defshared);
    }
    | OMP_DEFAULT '(' OMP_NONE ')'
    {
        $$ = DefaultClause(OC_defnone);
    }
    | OMP_REDUCTION '(' reduction_operator { sc_pause_openmp(); } ':' variable_list ')'
    {
        sc_start_openmp();
        $$ = ReductionClause($3, $6);
    }
    | OMP_COPYIN { sc_pause_openmp(); } '(' variable_list ')'
    {
        sc_start_openmp();
        $$ = VarlistClause(OCCOPYIN, $4);
    }
;

reduction_operator:
    '+'
    {
        $$ = OC_plus;
    }
    | '*'
    {
        $$ = OC_times;
    }
    | '-'
    {
        $$ = OC_minus;
    }
    | '&'
    {
        $$ = OC_band;
    }
    | '^'
    {
        $$ = OC_xor;
    }
    | '|'
    {
        $$ = OC_bor;
    }
    | AND_OP
    {
        $$ = OC_land;
    }
    | OR_OP
    {
        $$ = OC_lor;
    }
    | OMP_MIN
    {
        $$ = OC_min;
    }
    | OMP_MAX
    {
        $$ = OC_max;
    }
;

variable_list: // This is the section that handles if a variable is used in a different location than its scope 
    IDENTIFIER
    {
        if (checkDecls)
        if (symtab_get(stab, Symbol($1), IDNAME) == NULL)
            parse_error(-1, "Unknown identifier `%s'.\n", $1);
        $$ = IdentifierDecl( Symbol($1) );
    }
    | variable_list ',' IDENTIFIER
    {
        if (checkDecls)
        if (symtab_get(stab, Symbol($3), IDNAME) == NULL)
            parse_error(-1, "Unknown identifier `%s'.\n", $3);
        $$ = IdList($1, IdentifierDecl( Symbol($3) ));
    }
;

/* The same as "variable_list" only it checks if the variables
 * are declared @ the *same* scope level and whether they include the
 * "static" specifier. The original variable is also marked as
 * threadprivate.
 */
thrprv_variable_list: // This is the section that handles if a variable is used in a different location than its scope 
    IDENTIFIER
    {
        if (checkDecls)
        {
            stentry e = symtab_get(stab, Symbol($1), IDNAME);
            if (e == NULL)
                parse_error(-1, "Unknown identifier `%s'.\n", $1);
            if (e->scopelevel != stab->scopelevel)
                parse_error(-1, "threadprivate directive appears at different "
                    "scope level\nfrom the one `%s' was declared.\n", $1);
            if (stab->scopelevel > 0)    /* Don't care for globals */
            if (speclist_getspec(e->spec, STCLASSSPEC, SPEC_static) == NULL)
                parse_error(-1, "threadprivate variable `%s' does not have static "
                "storage type.\n", $1);
            e->isthrpriv = 1;   /* Mark */
        }
        $$ = IdentifierDecl( Symbol($1) );
    }
    | thrprv_variable_list ',' IDENTIFIER
    {
        if (checkDecls)
        {
            stentry e = symtab_get(stab, Symbol($3), IDNAME);
            if (e == NULL)
                parse_error(-1, "Unknown identifier `%s'.\n", $3);
            if (e->scopelevel != stab->scopelevel)
                parse_error(-1, "threadprivate directive appears at different "
                "scope level\nfrom the one `%s' was declared.\n", $3);
            if (stab->scopelevel > 0)    /* Don't care for globals */
            if (speclist_getspec(e->spec, STCLASSSPEC, SPEC_static) == NULL)
                parse_error(-1, "threadprivate variable `%s' does not have static "
                "storage type.\n", $3);
            e->isthrpriv = 1;   /* Mark */
        }
        $$ = IdList($1, IdentifierDecl( Symbol($3) ));
    }
;

/* -------------------------------------------------------------------------
 * --- OMPi extensions -----------------------------------------------------
 * -------------------------------------------------------------------------
 */

ompix_directive:
    ox_tasksync_directive
    {
        $$ = OmpixConstruct(OX_DCTASKSYNC, $1, NULL);
    }
    | ox_taskschedule_directive
    {
        $$ = OmpixConstruct(OX_DCTASKSCHEDULE, $1, NULL);
    }

;

ox_tasksync_directive:
    PRAGMA_OMPIX OMPIX_TASKSYNC '\n'
    {
        $$ = OmpixDirective(OX_DCTASKSYNC, NULL);
    }
;

ox_taskschedule_directive:
    PRAGMA_OMPIX OMPIX_TASKSCHEDULE
    {
        scope_start(stab);
    }
    ox_taskschedule_clause_optseq '\n'
    {
        scope_end(stab);
        $$ = OmpixDirective(OX_DCTASKSCHEDULE, $4);
    }
;

ox_taskschedule_clause_optseq:
// empty
    {
        $$ = NULL;
    }
    | ox_taskschedule_clause_optseq ox_taskschedule_clause
    {
        $$ = OmpixClauseList($1, $2);
    }
    | ox_taskschedule_clause_optseq ',' ox_taskschedule_clause
    {
        $$ = OmpixClauseList($1, $3);
    }
;

ox_taskschedule_clause:
    OMPIX_STRIDE '(' assignment_expression')'
    {
        $$ = OmpixStrideClause($3);
    }
    | OMPIX_START '(' assignment_expression ')'
    {
        $$ = OmpixStartClause($3);
    }
    | OMPIX_SCOPE '(' ox_scope_spec ')'
    {
        $$ = OmpixScopeClause($3);
    }
    | OMPIX_TIED
    {
        $$ = OmpixPlainClause(OX_OCTIED);
    }
    | OMP_UNTIED
    {
        $$ = OmpixPlainClause(OX_OCUNTIED);
    }
;

ox_scope_spec:
    OMPIX_NODES
    {
        $$ = OX_SCOPE_NODES;
    }
    | OMPIX_WORKERS
    {
        $$ = OX_SCOPE_WGLOBAL;
    }
    | OMPIX_WORKERS ',' OMPIX_GLOBAL
    {
        $$ = OX_SCOPE_WGLOBAL;
    }
    | OMPIX_WORKERS ',' OMPIX_LOCAL
    {
        $$ = OX_SCOPE_WLOCAL;
    }
;

ompix_construct:
    ox_taskdef_construct
    {
        $$ = $1;
    }
    | ox_task_construct
    {
        $$ = $1;
    }
;

/* 1 reduce-reduce here; we'll improve it some day.. */
ox_taskdef_construct:
    ox_taskdef_directive normal_function_definition
    {
        /* Should put the name of the callback function in the stab, too
     if (symtab_get(stab, decl_getidentifier_symbol($2->u.declaration.decl),
     FUNCNAME) == NULL)
     symtab_put(stab, decl_getidentifier_symbol($2->u.declaration.spec),
     FUNCNAME);
     */
        scope_start(stab);   /* re-declare the arguments of the task function */
        ast_declare_function_params($2->u.declaration.decl);
    }
    compound_statement
    {
        scope_end(stab);
        $$ = OmpixTaskdef($1, $2, $4);
        $$->l = $1->l;
    }
    | ox_taskdef_directive normal_function_definition
    {
        $$ = OmpixTaskdef($1, $2, NULL);
        $$->l = $1->l;
    }
;

ox_taskdef_directive:
    PRAGMA_OMPIX OMPIX_TASKDEF
    {
        scope_start(stab);
    }
    ox_taskdef_clause_optseq '\n'
    {
        scope_end(stab);
        $$ = OmpixDirective(OX_DCTASKDEF, $4);
    }
;

ox_taskdef_clause_optseq:
// empty
    {
        $$ = NULL;
    }
    | ox_taskdef_clause_optseq ox_taskdef_clause
    {
        $$ = OmpixClauseList($1, $2);
    }
    | ox_taskdef_clause_optseq ',' ox_taskdef_clause
    {
        $$ = OmpixClauseList($1, $3);
    }
;

ox_taskdef_clause:
    OMPIX_IN '(' ox_variable_size_list')'
    {
        $$ = OmpixVarlistClause(OX_OCIN, $3);
    }
    | OMPIX_OUT '(' ox_variable_size_list')'
    {
        $$ = OmpixVarlistClause(OX_OCOUT, $3);
    }
    | OMPIX_INOUT '(' ox_variable_size_list')'
    {
        $$ = OmpixVarlistClause(OX_OCINOUT, $3);
    }
    | OMP_REDUCTION '(' reduction_operator ':' ox_variable_size_list ')'
    {
        $$ = OmpixReductionClause($3, $5);
    }
;

ox_variable_size_list:
    ox_variable_size_elem
    {
        $$ = $1;
    }
    | ox_variable_size_list ',' ox_variable_size_elem
    {
        $$ = IdList($1, $3);
    }
;

ox_variable_size_elem:
    IDENTIFIER
    {
        $$ = IdentifierDecl( Symbol($1) );
        symtab_put(stab, Symbol($1), IDNAME);
    }
    | IDENTIFIER '[' '?' IDENTIFIER ']'
    {
        if (checkDecls) check_uknown_var($4);
        /* Use extern to differentiate */
        global_variables_c_restrict_flag = 0;
            $$ = ArrayDecl(IdentifierDecl( Symbol($1) ), StClassSpec(SPEC_extern),
        Identifier(Symbol($4)));
        symtab_put(stab, Symbol($1), IDNAME);
    }
    | IDENTIFIER '[' assignment_expression ']'
    {
        global_variables_c_restrict_flag = 0;
        $$ = ArrayDecl(IdentifierDecl( Symbol($1) ), NULL, $3);
        symtab_put(stab, Symbol($1), IDNAME);
    }
;

ox_task_construct:
    ox_task_directive ox_funccall_expression ';'
    {
        $$ = OmpixConstruct(OX_DCTASK, $1, Expression($2));
        $$->l = $1->l;
    }
;

ox_task_directive:
    PRAGMA_OMPIX OMP_TASK ox_task_clause_optseq '\n'
    {
        $$ = OmpixDirective(OX_DCTASK, $3);
    }
;

ox_task_clause_optseq:
    // empty
    {
        $$ = NULL;
    }
    | ox_task_clause_optseq ox_task_clause
    {
        $$ = OmpixClauseList($1, $2);
    }
    | ox_task_clause_optseq ',' ox_task_clause
    {
        $$ = OmpixClauseList($1, $3);
    }
;

ox_task_clause:
    OMPIX_ATNODE '(' '*' ')'
    {
        $$ = OmpixPlainClause(OX_OCATALL);
    }
    | OMPIX_ATNODE '(' assignment_expression ')'
    {
        $$ = OmpixAtnodeClause($3);
    }
    | OMPIX_ATWORKER '(' assignment_expression ')'
    {
        $$ = OmpixAtworkerClause($3);
    }
    | OMPIX_TIED
    {
        $$ = OmpixPlainClause(OX_OCTIED);
    }
    | OMP_UNTIED
    {
        $$ = OmpixPlainClause(OX_OCUNTIED);
    }
    | OMPIX_DETACHED
    {
        $$ = OmpixPlainClause(OX_OCDETACHED);
    }
;

ox_funccall_expression:
    IDENTIFIER '(' ')'
    {
        $$ = strcmp($1, "main") ?
        FunctionCall(Identifier(Symbol($1)), NULL) :
        FunctionCall(Identifier(Symbol(MAIN_NEWNAME)), NULL);
    }
    | IDENTIFIER '(' argument_expression_list ')'
    {
        $$ = strcmp($1, "main") ?
        FunctionCall(Identifier(Symbol($1)), $3) :
        FunctionCall(Identifier(Symbol(MAIN_NEWNAME)), $3);
    }
;

%%


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
    if (symtab_get(stab, s, IDNAME) == NULL && symtab_get(stab, s, LABELNAME) == NULL && symtab_get(stab, s, FUNCNAME) == NULL) {
        parse_error(-1, "Unknown identifier `%s'.\n", name);
    }
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

/*
* This function is responsible for setting attributes for an identifier in a symbol table
* symbol id: Represents the identifier symbol.
* astexpr expr: Represents an abstract syntax tree node for an expression associated with the identifier.
* int is_su_field: Indicates whether the identifier is a field within a struct or union (1 if true, 0 otherwise).
*/

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
        
    if(is_int == 1 && is_private == 1 && is_pointer == 1) {
        is_priv_int_ptr_appear = 1;
    }
    if(is_float == 1 && is_private == 1 && is_pointer == 1) {
        is_priv_float_ptr_appear = 1;
    }
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
     } // makes the expr either priv or pub based on the SPEC_private 
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

/*
*   Set the bit length of ast. 
*   aastexpr_ -> int ftype -> if expr represents a floating point value ftype=1 
*/
void set_bitlength_expr(astexpr e, astexpr e1, astexpr e2)
{
	if(e2 == NULL) // this check is because e could be a unary expression with only e1 
	{
        if (e->ftype != e1->ftype) {
            printf("Warning: converting a floating point value to an integer can cause lose of precision.\n");	
        }
		e->size = e1->size; 
		e->sizeexp = e1->sizeexp; 
    }
	else // this is the case if e is a binary expression  
	{
		// if((e1->ftype != e2->ftype))
		// {
            // printf("\n\ne1 Type: %d, e2 Type: %d\n\n", e1->ftype, e2->ftype); 
            // parse_error(-1, "Error 1: Operands of the same type are expected (use casting).\n"); 

		// }
		if(e1->ftype == 0) 
		{
			if(e1->size >= e2->size) // set the size of e to be the maximum of the sizes of e1 and e2.
				e->size = e1->size; 
			else
				e->size = e2->size; 
		}
		else if(e1->ftype == 1)
		{
			if(e1->size >= e2->size) // set the size of e to be the maximum of the sizes of e1 and e2.
				e->size = e1->size; 
			else
				e->size = e2->size; 
			
			if(e1->sizeexp >= e2->sizeexp) // set e's size expression to be the maximum of e1 and e2.
				e->sizeexp = e1->sizeexp; 
			else
				e->sizeexp = e2->sizeexp; 
		}
	}	
}

void set_security_flag_expr(astexpr e, astexpr e1, astexpr e2, int opid){
    //BOP
    if(e2 != NULL && e1 != NULL){
        if(e1->flag == PUB && e2->flag == PUB){
            e->flag = PUB;
            e->index = -1;
        } else {
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
    else if(e2 == NULL && e1 != NULL){
        e->flag = e1->flag;
        e->index = e1->index; 
    }
    //const
    else if(e1 == NULL && e2 == NULL){
        e->flag = PUB;
        e->index = -1; 
    }

    // e: Represents the entire expression being assigned to variable i.
    // e1: Represents the left operand of the addition operation, which is 1.
    // e2: Represents the right operand of the addition operation, which is 3.
    // this takes care of a+b 
    // Check if the expression is a constant or a constant expression

    // if (global_variables_c_restrict_flag == 1) { 
    //     // if (e1 != NULL && e2 != NULL) {
    //     if (e1 != NULL && e1->type == CONSTVAL && e2 != NULL && e2->type != CONSTVAL) { // 2 + i
    //         parse_error(-1, "Initializer element is not a constant '%s' 1.\n", e->u.sym->name);
    //     } else if (e1 != NULL && e1->type != CONSTVAL && e2 != NULL && e2->type == CONSTVAL) { // i + 2
    //         parse_error(-1, "Initializer element is not a constant '%s' 2.\n", e->u.sym->name);
    //     } else if (e1 != NULL && e1->type != CONSTVAL && e2 != NULL && e2->type != CONSTVAL) { // i + i
    //         parse_error(-1, "Initializer element is not a constant '%s' 3.\n", e->u.sym->name);
    //     } else if (e1 != NULL && e1->type != CONSTVAL) {
    //         parse_error(-1, "Initializer element is not a constant '%s' 4.\n", e->u.sym->name); // 
    //     } else if (e2 != NULL && e2->type != CONSTVAL) {
    //         parse_error(-1, "Initializer element is not a constant '%s' 5.\n", e->u.sym->name); // 
    //     } else if (e1 != NULL && e1->u.dtype->type != DARRAY && e1->type != ARRAYIDX && e1->arraytype != 1 && e1->type != CONSTVAL) { // i
    //         printf("\ntype: %d\n", e1->type);
    //         parse_error(-1, "Initializer element is not a constant '%s' 6.\n", e->u.sym->name);
    //     } else if (e1 == NULL && e2 != NULL) { // const + non-const
    //         parse_error(-1, "Initializer element is not a constant 7.\n");
    //     // } else if (e1 != NULL && e2 == NULL) { // non_const + const
    //     //     parse_error(-1, "Initializer element is not a constant 8.\n");
    //     }
    // }
   //COMPUTE THE MODULUS FOR DIFFERENT OPERATIONS AND DIFFERENT TYPES OF PRIVATE VARIABLES
} 

void security_check_for_assignment(astexpr le, astexpr re){
	if(le->flag == PUB && re->flag == PRI) 
		parse_error(-1, "Security type mismatch in assignment 1.\n");
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
                parse_error(-1, "A: Security type mismatch in assignment 2.\n");
            decl = decl->u.next;
        }
        
        if(decl->type == DINIT)
        {
            if(decl->u.expr->flag == PRI)
                flag2 = 1;
            
            if(flag1 == 0 && flag2 == 1)
                parse_error(-1, "B: Security type mismatch in assignment 3.\n");
        }
    
    }
    else{
        if(decl->type == DINIT)
        {
            if(decl->u.expr->flag == PRI)
                flag2 = 1;
            if(flag1 == 0 && flag2 == 1)
                parse_error(-1, "C: Security type mismatch in assignment 4.\n");
        }
    }
  
}

void set_security_flag_stmt(aststmt s, aststmt s1, aststmt s2){
    if(s->type = STATEMENTLIST){
        if((s1->flag == PUB) || (s2->flag == PUB)) {
           s->flag = PUB;
        }
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
    if(spec != NULL && spec->type == SUE)
	    return PUB;
    else if(spec != NULL && spec->type == SPECLIST){
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
    if(isarray){ // only true for static allocated arrays with name[len] not for dynamic allocated using pmalloc
        global_variables_c_restrict_flag = 0;
        d = decl->decl;
        // for one dimension array
        if(e2->type == IDENT){
            global_variables_c_restrict_flag = 0;
		    //e1->arraysize = d->u.expr;
	        e1->arraysize = ast_expr_copy(d->u.expr);
            //set the arraytype
            if(e3 == NULL || (e3 != NULL && d->decl->type == DARRAY))
                e1->arraytype = 1; 
        }
            // for two dimension array
        else if (e2->type == ARRAYIDX){
            global_variables_c_restrict_flag = 0;
            //e1->arraysize = decl->decl->u.expr; 
            //e2->arraysize = decl->decl->decl->u.expr; 
            e1->arraysize = ast_expr_copy(decl->decl->u.expr);
            e2->arraysize = ast_expr_copy(decl->decl->decl->u.expr);
        }
        global_variables_c_restrict_flag = 0;
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
            parse_error(-1, "Public condition is expected; open private condition prior its use.\n");
    
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
    global_variables_c_restrict_flag = 0;
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

    // Traverse through argument list and parameter list 
    while (arglist->type == COMMALIST && decl->type == DLIST) {
        // Get the type spec of the parameter
        spec = (decl->decl)->spec;
        
        // Compare the security type 
        if (set_security_flag_spec(spec) != arglist->right->flag) { // If sec type does not match
            parse_error(1, "Security type mismatch in argument list of '%s'.\n", funcname->u.sym->name);
            return 0; 
        }

        // // Compare the type 
        // if (decl->u.expr->ftype != arglist->right->ftype) { // If type does not match
        //     parse_error(1, "Type mismatch in argument list of '%s'.\n", funcname->u.sym->name);
        //     return 0; 
        // }
        
        // Move to the next argument and the next parameter
        arglist = arglist->left;
        decl = decl->u.next;
    }
        
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
            parse_error(1, "Too many or too few arguments to function '%s'\n", funcname);
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


void parse_warning(char *format, ...) {
    va_list ap;
    va_start(ap, format);

    fprintf(stdout, "(%s, line %d)\n\t", sc_original_file(), sc_original_line());
    vfprintf(stdout, format, ap); // this prints the message itself 

    va_end(ap);

    if (strcmp(sc_original_file(), "injected_code") == 0)
        fprintf(stdout, "\n>>>>>>>\n%s\n>>>>>>>\n", parsingstring);
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
	if(e1->ftype == 0 && e2->ftype == 0){ // integer computation
		int len = fmax(e1->size, e2->size); 
		if(e1->flag == PRI || e2->flag == PRI){
			if(opid == BOP_gt || opid == BOP_lt || opid == BOP_leq || opid == BOP_geq || opid == BOP_eqeq || opid == BOP_neq)
				modulus = fmax(modulus, len+kappa_nu); 
			else if(opid == BOP_div)
				modulus = fmax(modulus, 2*len+kappa_nu+8);
			else if(opid == BOP_shr){ // checking for right shifts, 
                // if shifting bu public amount --> truncation (Catrina and de Hoogh, 2010)
                // if shifting by a private amount, the security of the first argument doesnt matter, and we call truncation by a private value in floating point paper (Aliasgari et al., 2013)
				modulus = fmax(modulus, e1->size+kappa_nu);
            }
			else if(opid == BOP_shl && e2->flag == PRI) // checking for private left shift
                // left shifting by a private number of bits, call pow2 (Aliasgari et al., 2013)
				modulus = fmax(modulus, e2->size+kappa_nu);

		}		
	}else if(e1->ftype == 1 && e2->ftype == 1){ // floating-point
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
			modulus = fmax(modulus, fmax(2*len+1, k)+kappa_nu);
		else if(opid == BOP_mul)
			modulus = fmax(modulus, 2*len+kappa_nu); 
		else if(opid == BOP_gt || opid == BOP_lt || opid == BOP_leq || opid == BOP_geq)
			modulus = fmax(modulus, fmax(len+1, k)+kappa_nu); 	
		else if(opid == BOP_eqeq || opid == BOP_neq)
			modulus = fmax(modulus, fmax(len, k)+kappa_nu); 
		else if(opid == BOP_div)
			modulus = fmax(modulus, 2*len+kappa_nu+1);  
	} else if(e1->flag == PRI && e2->flag == PRI && (e1->ftype == 0 && e2->ftype == 1 || e1->ftype == 1 && e2->ftype == 0)){
		parse_error(-1, "Error 2: Operands of the same type are expected (use casting).\n"); 
		exit(0); 
	} else if (((e1->flag == PRI && e2->flag == PUB) || (e1->flag == PUB && e2->flag == PRI)) && (opid == BOP_neq || opid == BOP_eqeq)) {
        parse_error(-1, "Error 3: Operands of the same type are expected (use casting).\n"); 
        exit(0); 
    } else if (opid == BOP_dot && ((e1->flag == PRI && e2->flag == PUB) || (e2->flag == PRI && e1->flag == PUB))) {
        parse_error(-1, "Error 4: Operands of the same type are expected (use casting).\n"); 
		exit(0); 
    }
}
	
