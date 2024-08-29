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

/* AST - the Abstract Syntax Tree */

#ifndef __AST_H__
#define __AST_H__

#define INT_SIZE 32
#define CHAR_SIZE 8
#define SHORT_SIZE 16
#define LONG_SIZE 64
#define FLOAT_MAN_SIZE 16
#define FLOAT_EXP_SIZE 64

#include <stdio.h>

/* Predefine them because they are mutually recursive & they are used in
 * our include files, too.
 */
typedef struct astexpr_ *astexpr;
typedef struct astspec_ *astspec;
typedef struct astdecl_ *astdecl;
typedef struct aststmt_ *aststmt;
typedef struct ompcon_ *ompcon;       /* OpenMP construct */
typedef struct ompdir_ *ompdir;       /* OpenMP directive */
typedef struct ompclause_ *ompclause; /* OpenMP clause */

typedef struct oxclause_ *oxclause; /* OMPi-extension clause */
typedef struct oxcon_ *oxcon;       /* -"- construct */
typedef struct oxdir_ *oxdir;       /* -"- directive */

typedef struct struct_field_ *struct_field;
typedef struct struct_field_stack_ *struct_field_stack;

typedef struct struct_node_ *struct_node;
typedef struct struct_node_stack_ *struct_node_stack;

typedef struct condnode_ *condnode;
typedef struct condstack_ *condstack;
typedef struct branchnode_ *branchnode;
typedef struct branchtree_ *branchtree;
typedef struct mvar_ *mvar;
typedef struct mvarstack_ *mvarstack;
typedef struct lvar_ *lvar;
typedef struct ltable_ *ltable;
typedef struct ltablelist_ *ltablelist;
typedef struct batch_private_index_ *batch_private_index;
typedef struct batch_private_index_stack_ *batch_private_index_stack;
typedef struct control_sequence_ *control_sequence;
typedef struct control_sequence_stack_ *control_sequence_stack;
typedef struct control_record_ *control_record;
typedef struct control_record_stack_ *control_record_stack;
typedef struct batch_condition_ *batch_condition;
typedef struct batch_condition_stack_ *batch_condition_stack;
typedef struct batch_statement_ *batch_statement;
typedef struct batch_statement_stack_ *batch_statement_stack;
typedef struct iteration_ *iteration;

#include "symtab.h"

/* Create the child->parent links in the finalized AST.
 * It works, of course, only for statement nodes.
 */
extern void ast_stmt_parent(aststmt parent, aststmt t);
extern void ast_parentize(aststmt tree);

/* Given any statement, get the function node it belongs to */
extern aststmt ast_get_enclosing_function(aststmt t);
/* Inserts a statement after the declaration section in a compound */
extern void ast_compound_insert_statement(aststmt tree, aststmt t);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     EXRESSION NODES                                           *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

enum exprtype { IDENT = 1,
                CONSTVAL,
                STRING,
                FUNCCALL,
                ARRAYIDX,
                DOTFIELD,
                PTRFIELD,
                CASTEXPR,
                CONDEXPR,
                UOP,
                BOP,
                PREOP,
                POSTOP,
                ASS,
                COMMALIST,
                SPACELIST,
                BRACEDINIT,
                DESIGNATED,
                IDXDES,
                DOTDES,
                EPMALLOC };

enum securityflag { PRI = 1,
                    PUB };
struct astexpr_ {
    enum exprtype type;
    enum securityflag flag;

    int index; /* if expr = pub, index = -1
                  if expr = pri and expr = ident, index = 0
                  if expr = pri and expr != ident, index increases */
    int thread_id;
    int size; /* if expr = pub, size = -1 */
              /* if expr = pri and size = 32 (not specified) */
              /* if expr = pri and size = specified*/
    int sizeexp;
    /* max length of sym appeared in the expr */

    astexpr left, right;
    int opid;          /* Used for operators */
    astexpr arraysize; /* Used for arrays */
    int computingarraysize; 
    int arraytype;     /* if expr represents an array,  arraytype = 1 */
    int ftype;         /* if expr represents a floating point value ftype = 1 */
    int isptr;
    int last_op_hit; 
    union {
        char *str;     /* Used by strings and constants */
        symbol sym;    /* Used by identifiers/fields */
        astexpr cond;  /* Used only in conditional exprs */
        astdecl dtype; /* Used only in casts & sizeof */
    } u;

    int l, c; /* Location in file (line, column) */
    symbol file;
};

/* Ids of unary, binary and assignement operators.
 * To print them out, use them as indexes to the correspoind _symbols[] array.
 */
#define UOP_addr 0
#define UOP_star 1
#define UOP_neg 2
#define UOP_bnot 3
#define UOP_lnot 4
#define UOP_sizeof 5
#define UOP_sizeoftype 6
#define UOP_inc 7
#define UOP_dec 8
#define UOP_paren 9 /* (expr) */
#define UOP_typetrick 10
extern char *UOP_symbols[11];

#define BOP_shl 0
#define BOP_shr 1
#define BOP_leq 2
#define BOP_geq 3
#define BOP_eqeq 4
#define BOP_neq 5
#define BOP_land 6
#define BOP_lor 7
#define BOP_band 8
#define BOP_bor 9
#define BOP_bxor 10
#define BOP_add 11
#define BOP_sub 12
#define BOP_lt 13
#define BOP_gt 14
#define BOP_mul 15
#define BOP_div 16
#define BOP_mod 17
#define BOP_cast 18
#define BOP_dot 19
extern char *BOP_symbols[20];

#define ASS_eq 0
#define ASS_mul 1
#define ASS_div 2
#define ASS_mod 3
#define ASS_add 4
#define ASS_sub 5
#define ASS_shl 6
#define ASS_shr 7
#define ASS_and 8
#define ASS_xor 9
#define ASS_or 10
extern char *ASS_symbols[11];

/* Node creation calls
 */
extern astexpr Astexpr(enum exprtype type, astexpr left, astexpr right);
extern astexpr Identifier(symbol s);
extern astexpr Constant(char *s);
extern astexpr numConstant(int n);
extern astexpr String(char *s);
extern astexpr DotField(astexpr e, symbol s);
extern astexpr PtrField(astexpr e, symbol s);
extern astexpr Operator(enum exprtype type, int opid, astexpr left, astexpr right);
extern astexpr ConditionalExpr(astexpr cond, astexpr t, astexpr f);
extern astexpr DotDesignator(symbol s);
extern astexpr CastedExpr(astdecl d, astexpr e);
extern astexpr Sizeoftype(astdecl d);
extern astexpr TypeTrick(astdecl d);
extern int ComputeExprSize(astexpr e1, astexpr e2);
extern astexpr PmallocExpr(astexpr e1, astdecl d); // by ghada

#define ArrayIndex(a, b) Astexpr(ARRAYIDX, a, b)
#define FunctionCall(a, b) Astexpr(FUNCCALL, a, b)
#define PostOperator(e, opid) Operator(POSTOP, opid, e, NULL)
#define PreOperator(e, opid) Operator(PREOP, opid, e, NULL)
#define UnaryOperator(opid, e) Operator(UOP, opid, e, NULL)
#define BinaryOperator(opid, a, b) Operator(BOP, opid, a, b)
#define Assignment(lhs, opid, rhs) Operator(ASS, opid, lhs, rhs)
#define CommaList(a, b) Astexpr(COMMALIST, a, b)
#define SpaceList(a, b) Astexpr(SPACELIST, a, b)
#define Sizeof(e) UnaryOperator(UOP_sizeof, e)
#define BracedInitializer(a) Astexpr(BRACEDINIT, a, NULL)
#define Designated(a, b) Astexpr(DESIGNATED, a, b)
#define IdxDesignator(a) Astexpr(IDXDES, a, NULL)
#define Pmalloc(a, b) PmallocExpr(a, b)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     DECLARATION NODES                                         *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * We have 2 types of declaration nodes:
 *   (1) specifier nodes and
 *   (2) declarator nodes
 * The first one contains vaguely the "type" and the second one
 * the actual "name" that is being declared, or the "left" part
 * and the "right" part of the declaration, correspondingly.
 * Of course, each part is a quite complicated tree.
 */

/*
 * SPECIFIERS
 */

/* Specifier keywords/subtypes */
#define SPEC_typedef 1
#define SPEC_extern 2
#define SPEC_static 3
#define SPEC_auto 4
#define SPEC_register 5
#define SPEC_void 6
#define SPEC_char 7
#define SPEC_short 8
#define SPEC_int 9
#define SPEC_long 10
#define SPEC_float 11
#define SPEC_double 12
#define SPEC_signed 13
#define SPEC_unsigned 14
#define SPEC_ubool 15
#define SPEC_ucomplex 16
#define SPEC_uimaginary 17
#define SPEC_struct 18
#define SPEC_union 19
#define SPEC_enum 20
#define SPEC_const 21
#define SPEC_restrict 22
#define SPEC_volatile 23
#define SPEC_inline 24
#define SPEC_star 25  /* for pointer declaration specifiers */
#define SPEC_Rlist 26 /* It is only a subtype */
#define SPEC_Llist 27
#define SPEC_private 28
#define SPEC_public 29

/* It is only a subtype */
extern char *SPEC_symbols[30];

/* All declaration specifier node types */
enum spectype { SPEC = 1,
                STCLASSSPEC,
                USERTYPE,
                SUE,
                ENUMERATOR,
                SPECLIST };

struct astspec_ {
    enum spectype type;
    int is_spec_for_sng;
    int subtype;
    int size;     /* For variable-size private types; 0 means default size */
    int sizeexp;  /* For size of exponent for floating point value */
    symbol name;  /* For SUE/enumlist name/user types */
    astspec body; /* E.g. for SUE fields, lists */
    union {
        astexpr expr; /* For enum list */
        astspec next; /* For Lists */
        astdecl decl; /* For structure specifiers (the fields) */
        char *txt;    /* For attributes (verbatim text) */
    } u;
    int l, c; /* Location in file (line, column) */
    symbol file;
};

extern astspec Specifier(enum spectype type, int subtp, symbol name, astspec d, int size, int sizeexp);
extern astspec Enumerator(symbol name, astexpr expr);
extern astspec Specifierlist(int type, astspec e, astspec l);
extern astspec SUdecl(int type, symbol sym, astdecl decl);
#define DeclspecFloat(type, size, sizeexp) Specifier(SPEC, type, NULL, NULL, size, sizeexp)
#define Declspec(type, size) Specifier(SPEC, type, NULL, NULL, size, 0)
#define StClassSpec(type) Specifier(STCLASSSPEC, type, NULL, NULL, 0, 0)
#define Usertype(sym) Specifier(USERTYPE, 0, sym, NULL, 0, 0)
#define Enumdecl(sym, body) Specifier(SUE, SPEC_enum, sym, body, 0, 0)
#define Speclist_right(e, l) Specifierlist(SPEC_Rlist, e, l)
#define Speclist_left(l, e) Specifierlist(SPEC_Llist, e, l)
#define Enumbodylist(l, e) Specifierlist(SPEC_enum, e, l)

/*
 * DECLARATORS
 */

/* Declarator lists / subtypes */
#define DECL_decllist 1
#define DECL_idlist 2
#define DECL_paramlist 3
#define DECL_fieldlist 4

/* All declarator node types */
enum decltype { DIDENT = 1,
                DPAREN,
                DARRAY,
                DFUNC,
                DINIT,
                DECLARATOR,
                ABSDECLARATOR,
                DPARAM,
                DELLIPSIS,
                DBIT,
                DSTRUCTFIELD,
                DCASTTYPE,
                DLIST };

struct astdecl_ {
    enum decltype type;
    int subtype;
    int is_decl_for_sng;
    astdecl decl; /* For initlist,initializer,declarator */
    astspec spec; /* For pointer declarator */
    union {
        symbol id;      /* Identifiers */
        astexpr expr;   /* For initializer/bitdeclarator */
        astdecl next;   /* For lists */
        astdecl params; /* For funcs */
    } u;
    int l, c; /* Location in file (line, column) */
    symbol file;
};

extern astdecl Decl(enum decltype type, int subtype, astdecl d, astspec s);
extern astdecl IdentifierDecl(symbol s);
extern astdecl ArrayDecl(astdecl decl, astspec s, astexpr e);
extern astdecl FuncDecl(astdecl decl, astdecl p);
extern astdecl InitDecl(astdecl decl, astexpr e);
extern astdecl BitDecl(astdecl decl, astexpr e);
extern astdecl Declanylist(int subtype, astdecl l, astdecl e);
#define ParenDecl(d) Decl(DPAREN, 0, d, NULL)
#define Pointer() Declspec(SPEC_star, 0)
#define Declarator(ptr, direct) Decl(DECLARATOR, 0, direct, ptr)
#define AbstractDeclarator(ptr, d) Decl(ABSDECLARATOR, 0, d, ptr)
#define Casttypename(s, d) Decl(DCASTTYPE, 0, d, s)
#define ParamDecl(s, d) Decl(DPARAM, 0, d, s)
#define Ellipsis() Decl(DELLIPSIS, 0, NULL, NULL)
#define DeclList(l, e) Declanylist(DECL_decllist, l, e)
#define IdList(l, e) Declanylist(DECL_idlist, l, e)
#define ParamList(l, e) Declanylist(DECL_paramlist, l, e)
#define StructfieldDecl(s, d) Decl(DSTRUCTFIELD, 0, d, s)
#define StructfieldList(l, e) Declanylist(DECL_fieldlist, l, e)

/*
 * Stuff related to declarated symbols
 */

extern astdecl decl_getidentifier(astdecl d);
extern int decl_getkind(astdecl d); /* DFUNC/DARRAY/DIDENT */
extern int decl_ispointer(astdecl d);
#define decl_getidentifier_symbol(d) (decl_getidentifier(d)->u.id)
extern astspec speclist_getspec(astspec s, int type, int subtype);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     STATEMENT NODES                                           *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* All statement types */
enum stmttype { JUMP = 1,
                ITERATION,
                SELECTION,
                LABELED,
                EXPRESSION,
                DECLARATION,
                COMPOUND,
                STATEMENTLIST,
                FUNCDEF,
                VERBATIM,
                SMC,
                BATCH,
                OMPSTMT,
                OX_STMT,
                SPFREE };

struct aststmt_ { // statments that are declaration   // this is where the flag was added 
    enum stmttype type;
    int is_stmt_for_sng; /*Indicates if the stmt is a declaration stmt for sng*/
    int subtype;
    int flag;       /* Indicates if the statement contains priv assign */
    int gflag;    /* (0->non-global, 1->global) -> gflag or global flag is used to keep track of private or public global variables. */
    aststmt parent; /* Set *after* AST construction */
    aststmt body;   /* Most have a body (COMPOUND has ONLY body) */
    aststmt openmp_parent_stmt;
    int num_tmp; /* Only used for FuncDef, and stores
                  number of tmps used within function */
    int num_float_tmp;
    int is_priv_int_ptr_appear;
    int is_priv_float_ptr_appear;
    int is_priv_int_index_appear;
    int is_priv_float_index_appear;
    int is_priv_int_ptr_struct_field_appear;
    int is_priv_float_ptr_struct_field_appear;
    int is_priv_struct_ptr_struct_field_appear;
    int is_priv_int_struct_field_appear;
    int is_priv_float_struct_field_appear;
    int contain_priv_if_flag; /* Only used for FuncDef, and it determines
                                if the function contains any private if condition */
    union {
        astexpr expr; /* For expression & return statements */
        struct {
            aststmt init;
            astexpr cond, incr;
        } iteration;
        struct {
            astexpr cond;
            aststmt elsebody;
        } selection;
        struct {
            astspec spec;
            astdecl decl; /* dlist is for FuncDef */
            aststmt dlist;
        } declaration;
        struct {
            astexpr variable;
            astexpr party;
            astspec type;
            astexpr size1;
            astexpr size2;
        } smcops;
        struct {
            astexpr variable;
        } pfreeops;

        symbol label; /* For GOTO and labeled statements */
        aststmt next; /* For StatementList */
        ompcon omp;   /* OpenMP construct node */
        oxcon ox;     /* OMPi-extension node */
        char *code;   /* For Verbatim Code Transformation */
    } u;
    int l, c; /* Location in file (line, column) */
    symbol file;
};

/* stack structure for storing private conditions */

struct struct_field_ {
    astspec type;
    astdecl name;
    struct_field next;
};

struct struct_field_stack_ {
    struct_field head;
};

struct struct_node_ {
    astspec name;
    int contain_pub_field;
    struct_node next;
    struct_field_stack fieldlist;
};

struct struct_node_stack_ {
    struct_node head;
};

struct condnode_ {
    astexpr element;
    condnode next;
};

struct batch_private_index_ {
    int batch_index;
    int private_index;
    int ftype;
    batch_private_index next;
};

struct batch_private_index_stack_ {
    batch_private_index head;
};

struct condstack_ {
    condnode head;
};

struct mvar_ {
    astexpr var_name;
    int is_complete;
    mvar next;
};

struct mvarstack_ {
    mvar head;
};

struct lvar_ {
    astspec spec;
    astdecl decl;
    int is_static;
    lvar next;
};

struct ltable_ {
    lvar head;
    ltable next;
};

struct ltablelist_ {
    ltable head;
};

struct branchnode_ {
    int key;
    int private_index;
    int if_index;
    int parent_label;
    int current_label;
    branchnode left;
    branchnode right;
    branchnode parent;
    mvarstack ms;
    ltablelist tablelist;
};

struct control_sequence_ {
    int index;
    int batch_index;
    control_sequence next;
};

struct control_sequence_stack_ {
    control_sequence head;
};

struct control_record_ {
    astexpr var_name;
    astexpr arrayindex;
    int if_index;
    int iter_index;
    int tmp_index;
    int flag;
    branchnode branch;
    control_record next;
};

struct control_record_stack_ {
    control_record head;
};

struct batch_condition_ {
    iteration condition;
    batch_condition next;
};

struct batch_statement_ {
    aststmt statement;
    int statement_index;
    int batch_index;
    int flag;
    batch_statement next;
};

struct batch_statement_stack_ {
    batch_statement head;
};

struct batch_condition_stack_ {
    batch_condition head;
};

struct iteration_ {
    aststmt init;
    astexpr cond;
    astexpr incr;
};

/* All statement subtypes */
#define SGOTO 1 /* Jumps */
#define SBREAK 2
#define SCONTINUE 3
#define SRETURN 4
#define SWHILE 5 /* Iterations */
#define SDO 6
#define SFOR 7
#define SIF 8 /* Selections */
#define SSWITCH 9
#define SLABEL 10 /* Labeled */
#define SCASE 11
#define SDEFAULT 12
#define SINPUT 13
#define SOUTPUT 14
#define SOPEN 15
#define SBITS 16
#define SINV 17
#define SSPFREE 18

aststmt Statement(enum stmttype type, int subtype, aststmt body);
extern aststmt Jumpstatement(int subtype, astexpr expr);
extern aststmt Iterationstatement(int subtype,
                                  aststmt init, astexpr cond, astexpr incr, aststmt body);
extern aststmt Selectionstatement(int subtype,
                                  astexpr cond, aststmt body, aststmt elsebody);
extern aststmt LabeledStatement(int subtype, symbol l, astexpr e, aststmt st);
extern aststmt Smc(int subtype, astspec spec, astexpr e1, astexpr e2, astexpr size1, astexpr size2);
extern aststmt Batch(aststmt init, astexpr cond, astexpr incr, aststmt body);
extern aststmt Pfree(astexpr e1); // by Ghada

extern aststmt Goto(symbol s);
extern aststmt Expression(astexpr e); /* Maybe NULL */
extern aststmt Declaration(astspec spec, astdecl decl);
extern aststmt BlockList(aststmt l, aststmt st);
extern aststmt FuncDef(astspec spec, astdecl decl, aststmt dlist, aststmt body);
extern aststmt OmpStmt(ompcon omp);
extern aststmt OmpixStmt(oxcon ox);
extern aststmt Verbatim(char *code);
extern aststmt verbit(char *format, ...);

/* functions of stack and tree operations
   for "if" statement with private condition.
 */

extern void struct_field_push(struct_field_stack, astspec, astdecl);
extern struct_field struct_field_lookup(struct_node node, char *field_name);
extern struct_field struct_field_new();
extern void struct_field_free(struct_field);
extern struct_field_stack struct_field_stack_new();
extern void struct_field_stack_free(struct_field_stack);

extern struct_node struct_node_new();
extern struct_node_stack struct_node_stack_new();
extern void struct_node_free(struct_node);
extern void struct_node_stack_free(struct_node_stack);
extern void struct_node_push(struct_node_stack, astspec);
extern void struct_node_update(struct_node_stack, astdecl);
extern int struct_node_get_flag(struct_node_stack, char *);
extern struct_node struct_node_lookup(struct_node_stack, char *);

extern void if_push(astexpr, condstack);
extern int if_isEmpty(condstack);
extern astexpr if_pop(condstack);
extern int if_length(condstack);
extern condstack if_stack_new();

extern void mvar_push(astexpr, mvarstack);
extern int mvar_isEmpty(mvarstack);
extern astexpr mvar_pop(mvarstack);
extern int mvar_length(mvarstack);
extern mvarstack mvar_stack_new();
extern void mvar_stack_free(mvarstack);

extern void ltable_push(astspec, astdecl, ltable);
extern void ltable_pop(ltable, astspec *, astdecl *);
extern int ltable_isEmpty(ltable);
extern int ltable_length(ltable);
extern ltable ltable_new();
extern void ltable_free(ltable);
extern void ltablelist_push(ltablelist);
extern ltable ltablelist_pop(ltablelist);
extern int ltablelist_length(ltablelist);
extern ltablelist ltablelist_new();
extern void ltablelist_free(ltablelist);

extern void batch_private_index_push(int, int, int, batch_private_index_stack);
extern batch_private_index_stack batch_private_index_stack_new();
extern void batch_private_index_stack_free(batch_private_index_stack);
extern int batch_private_index_stack_length(batch_private_index_stack);

extern branchnode if_branchtree_new();
extern branchnode if_branchnode_insert(branchnode, mvarstack, int, int, int, int, int);
extern void if_branchtree_remove(branchnode current, branchnode node);
extern int if_branchnode_height(branchnode node);
extern void if_branchtree_free(branchnode root);
extern void if_branchtree_print(branchnode root);

extern void control_sequence_push(int, control_sequence_stack);
extern void control_sequence_pop(control_sequence_stack);
extern control_sequence_stack control_sequence_stack_new();
extern int control_sequence_stack_length(control_sequence_stack);

extern int control_record_push(astexpr, astexpr, int, int, control_record_stack);
extern void control_record_print(control_record_stack);
extern void control_record_set_branch(int, branchnode, control_record_stack);
extern control_record_stack control_record_stack_new();

extern iteration create_iteration(aststmt, astexpr, astexpr);
extern void batch_condition_push(iteration iter, batch_condition_stack);

extern batch_condition_stack batch_condition_stack_new();

extern void batch_statement_push(aststmt, int, int, int, batch_statement_stack);
extern void batch_statment_popAll(batch_statement_stack);
extern batch_statement_stack batch_statement_stack_new();
extern int batch_statement_size(batch_statement_stack);

#define Break() Jumpstatement(SBREAK, NULL)
#define Continue() Jumpstatement(SCONTINUE, NULL)
#define Return(n) Jumpstatement(SRETURN, n)
#define While(cond, body) Iterationstatement(SWHILE, NULL, cond, NULL, body)
#define Do(body, cond) Iterationstatement(SDO, NULL, cond, NULL, body)
#define For(init, cond, incr, body) Iterationstatement(SFOR, init, cond, incr, body)
#define If(c, t, e) Selectionstatement(SIF, c, t, e)
#define Switch(c, b) Selectionstatement(SSWITCH, c, b, NULL)
#define Labeled(l, a) LabeledStatement(SLABEL, l, NULL, a)
#define Default(a) LabeledStatement(SDEFAULT, NULL, NULL, a)
#define Case(e, a) LabeledStatement(SCASE, NULL, e, a)
#define Compound(a) Statement(COMPOUND, 0, a)

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OPENMP NODES                                              *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* All OpenMP clause types */
/* OCFIRSTLASTPRIVATE is NOT an OpenMP clause per se,
 * but is very helpful when analyzing variables that are both
 * first- and last-private.
 */
enum clausetype { OCNOCLAUSE = 0,
                  OCNOWAIT,
                  OCIF,
                  OCNUMTHREADS,
                  OCORDERED,
                  OCSCHEDULE,
                  OCCOPYIN,
                  OCPRIVATE,
                  OCCOPYPRIVATE,
                  OCFIRSTPRIVATE,
                  OCLASTPRIVATE,
                  OCSHARED,
                  OCDEFAULT,
                  OCREDUCTION,
                  OCLIST,
                  OCFIRSTLASTPRIVATE,
                  /* OpenMP 3.0 */
                  OCUNTIED,
                  OCCOLLAPSE,
                  /* OpenMP 3.1 */
                  OCFINAL,
                  OCMERGEABLE
};
extern char *clausenames[20];

/* Clause subtypes */
#define OC_static 0
#define OC_dynamic 1
#define OC_guided 2
#define OC_runtime 3
#define OC_defshared 4
#define OC_defnone 5
#define OC_plus 6
#define OC_times 7
#define OC_minus 8
#define OC_band 9
#define OC_bor 10
#define OC_xor 11
#define OC_land 12
#define OC_lor 13
#define OC_affinity 14
#define OC_auto 15
#define OC_min 16
#define OC_max 17
extern char *clausesubs[19];

struct ompclause_ {
    enum clausetype type;
    int subtype;
    ompdir parent; /* The directive the clause belongs to */
    union {
        astexpr expr;
        astdecl varlist;
        struct {
            ompclause elem;
            ompclause next;
        } list;
    } u;
    int l, c; /* Location in file (line, column) */
    symbol file;
};

ompclause OmpClause(enum clausetype typ, int subtype, astexpr expr, astdecl vlist);
ompclause OmpClauseList(ompclause next, ompclause elem);
#define VarlistClause(type, varlist) OmpClause(type, 0, NULL, varlist)
#define ReductionClause(op, varlist) OmpClause(OCREDUCTION, op, NULL, varlist)
#define DefaultClause(what) OmpClause(OCDEFAULT, what, NULL, NULL)
#define PlainClause(type) OmpClause(type, 0, NULL, NULL)
#define IfClause(expr) OmpClause(OCIF, 0, expr, NULL)
#define NumthreadsClause(expr) OmpClause(OCNUMTHREADS, 0, expr, NULL)
#define ScheduleClause(kind, expr) OmpClause(OCSCHEDULE, kind, expr, NULL)
#define CollapseClause(num) OmpClause(OCCOLLAPSE, num, NULL, NULL)
#define FinalClause(expr) OmpClause(OCFINAL, 0, expr, NULL)

/* directive/construct types */
enum dircontype { DCPARALLEL = 1,
                  DCFOR,
                  DCSECTIONS,
                  DCSECTION,
                  DCSINGLE,
                  DCPARFOR,
                  DCPARSECTIONS,
                  DCFOR_P,
                  DCMASTER,
                  DCCRITICAL,
                  DCATOMIC,
                  DCORDERED,
                  DCBARRIER,
                  DCFLUSH,
                  DCTHREADPRIVATE,
                  DCTASK,
                  DCTASKWAIT, /* OpenMP 3.0 */
                  DCTASKYIELD /* OpenMP 3.1 */
};
extern char *ompdirnames[19];

struct ompdir_ {
    enum dircontype type;
    ompclause clauses; /* actually a clause list */
    ompcon parent;     /* The construct the directive belongs to */
    union {
        symbol region;
        astdecl varlist; /* For flush(), threadprivate() */
    } u;
    int l, c; /* Location in file (line, column) */
    symbol file;
};

extern ompdir OmpDirective(enum dircontype type, ompclause cla);
extern ompdir OmpCriticalDirective(symbol r);
extern ompdir OmpFlushDirective(astdecl a);
extern ompdir OmpThreadprivateDirective(astdecl a);

struct ompcon_ {
    enum dircontype type;
    ompdir directive;
    aststmt body;
    aststmt parent; /* The OmpStmt node the construct belongs to */
    int l, c;       /* Location in file (line, column) */
    symbol file;
};

extern ompcon OmpConstruct(enum dircontype type, ompdir dir, aststmt body);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OMPi-EXTENSION NODES                                      *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* all OMPi-extension clause types */
enum oxclausetype { OX_OCIN = 1,
                    OX_OCOUT,
                    OX_OCINOUT,
                    OX_OCLIST,
                    OX_OCREDUCE,
                    OX_OCATNODE,
                    OX_OCATALL,
                    OX_OCDETACHED,
                    OX_OCTIED,
                    OX_OCUNTIED,
                    OX_OCSTRIDE,
                    OX_OCSTART,
                    OX_OCSCOPE,
                    OX_OCATWORKER };
extern char *oxclausenames[15];

struct oxclause_ {
    enum oxclausetype type;
    int operator; /* reduction operator */
    oxdir parent; /* The directive the clause belongs to */
    union {
        int value; /* scope type */
        astexpr expr;
        astdecl varlist;
        struct {
            oxclause elem;
            oxclause next;
        } list;
    } u;
    int l, c; /* Location in file (line, column) */
    symbol file;
};

/* Scope type for taskschedule */
#define OX_SCOPE_NODES 0
#define OX_SCOPE_WLOCAL 1
#define OX_SCOPE_WGLOBAL 2

extern oxclause OmpixClause(enum oxclausetype type, astdecl vlist, astexpr e);
extern oxclause OmpixClauseList(oxclause next, oxclause elem);
#define OmpixVarlistClause(type, varlist) OmpixClause(type, varlist, NULL)
extern oxclause OmpixReductionClause(int op, astdecl varlist);
extern oxclause OmpixScopeClause(int scope);
#define OmpixAtnodeClause(expr) OmpixClause(OX_OCATNODE, NULL, expr)
#define OmpixAtworkerClause(expr) OmpixClause(OX_OCATWORKER, NULL, expr)
#define OmpixPlainClause(type) OmpixClause(type, NULL, NULL)
#define OmpixStrideClause(expr) OmpixClause(OX_OCSTRIDE, NULL, expr)
#define OmpixStartClause(expr) OmpixClause(OX_OCSTART, NULL, expr)

/* directive/construct types */
enum oxdircontype { OX_DCTASKDEF = 1,
                    OX_DCTASK,
                    OX_DCTASKSYNC,
                    OX_DCTASKSCHEDULE };
extern char *oxdirnames[5];

struct oxdir_ {
    enum oxdircontype type;
    oxclause clauses; /* actually a clause list */
    oxcon parent;     /* The construct the directive belongs to */
    int l, c;         /* Location in file (line, column) */
    symbol file;
};

extern oxdir OmpixDirective(enum oxdircontype type, oxclause cla);

struct oxcon_ {
    enum oxdircontype type;
    oxdir directive;
    aststmt body;
    aststmt callback; /* Only for taskdef with uponreturn code */
    aststmt parent;   /* The OmpStmt node the construct belongs to */
    int l, c;         /* Location in file (line, column) */
    symbol file;
};

extern oxcon OmpixConstruct(enum oxdircontype type, oxdir dir, aststmt body);
extern oxcon OmpixTaskdef(oxdir dir, aststmt body, aststmt callbackblock);

#endif /* __AST_H__ */
