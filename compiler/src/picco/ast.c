/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame
   ** Department of Computer Science and Engineering, University at Buffalo (SUNY)

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

#include "ast.h"
#include "ast_print.h"
#include "parser.h"
#include "picco.h"
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     EXRESSION NODES                                           *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

char *UOP_symbols[11] =
    {
        "&", "*", "-", "~", "!", "sizeof", "sizeof", "++", "--", "(", "\0"},
     *BOP_symbols[20] =
         {
             "<<", ">>", "<=", ">=", "==", "!=", "&&", "||", "&", "|", "^",
             "+", "-", "<", ">", "*", "/", "%", "cast", "@"},
     *ASS_symbols[11] =
         {
             "=", "*=", "/=", "%=", "+=", "-=", "<<=", ">>=", "&=", "^=", "|="};

int default_len = 32;
str strs;
astexpr Astexpr(enum exprtype type, astexpr left, astexpr right) {
    // if (type == COMMALIST && left->type == CONSTVAL && right->type == CONSTVAL) {
    //     exit_error(1, "    Invalid initialization using (), line: %d.\n", left->l);
    // } // this disallows all initilizations
    astexpr n = smalloc(sizeof(struct astexpr_));
    n->last_op_hit = 0;
    n->BOP_tree_length = 0;
    n->type = type;
    n->left = left;
    n->right = right;
    n->opid = 0;
    n->size = -1;
    n->sizeexp = -1;
    n->arraytype = 0;
    n->arraysize = NULL;
    n->computingarraysize = 0;
    n->thread_id = -1;
    n->ftype = 0;
    n->l = sc_original_line();
    n->c = sc_column();
    n->file = Symbol(sc_original_file());
    return (n);
}

astexpr Identifier(symbol s) {
    astexpr n = Astexpr(IDENT, NULL, NULL);
    n->u.sym = s;
    return (n);
}
// constant is public, and its length is -1
astexpr Constant(char *s) {
    astexpr n = Astexpr(CONSTVAL, NULL, NULL);
    n->u.str = s;
    return (n);
}

astexpr numConstant(int n) {
    static char numstr[64];
    snprintf(numstr, 63, "%d", n);
    return (Constant(strdup(numstr)));
}
// string is public, and its length is -1
astexpr String(char *s) {
    astexpr n = Astexpr(STRING, NULL, NULL);
    n->u.str = s;
    return (n);
}

astexpr DotField(astexpr e, symbol s) {
    astexpr n = Astexpr(DOTFIELD, e, NULL);
    n->u.sym = s;
    return (n);
}

astexpr PtrField(astexpr e, symbol s) {
    astexpr n = Astexpr(PTRFIELD, e, NULL);
    n->u.sym = s;
    return (n);
}

astexpr Operator(enum exprtype type, int opid, astexpr left, astexpr right) {
    astexpr n = Astexpr(type, left, right);
    n->opid = opid;
    n->size = ComputeExprSize(left, right);
    if (type == UOP) { // do this check only of it is UOP and the opid are lnot and bnot 
        if (left->size != 1) {
            if (left->arraytype == 1) {
                if (opid == UOP_bnot) { 
                    exit_error(1, "        '~' is only supported on arrays of bits. Line: %d. \n", left->l);
                } 
            }
        }
    }
    if (type == BOP && opid == BOP_dot) {
        if (left->arraysize && left->arraysize->u.dtype) {
            if (left->arraysize->u.dtype->type == 53) {
                exit_error(1, "        The dot product operation '@' can only be invoked on one-dimentional arrays. Line: %d \n", left->l);
            }
        }
    }
    return (n);
}

astexpr ConditionalExpr(astexpr cond, astexpr t, astexpr f) {
    astexpr n = Astexpr(CONDEXPR, t, f);
    n->u.cond = cond;
    return (n);
}

astexpr DotDesignator(symbol s) {
    astexpr n = Astexpr(DOTDES, NULL, NULL);
    n->u.sym = s;
    return (n);
}

astexpr CastedExpr(astdecl d, astexpr e) {
    astexpr n = Astexpr(CASTEXPR, e, NULL);
    n->u.dtype = d;
    return (n);
}

astexpr Sizeoftype(astdecl d) {
    astexpr n = UnaryOperator(UOP_sizeoftype, NULL);
    n->u.dtype = d;
    return (n);
}

astexpr TypeTrick(astdecl d) {
    astexpr n = UnaryOperator(UOP_typetrick, NULL);
    n->u.dtype = d;
    return (n);
}

int ComputeExprSize(astexpr e1, astexpr e2) {
    if (e1 != NULL && e2 != NULL) {
        if (e1->size == e2->size)
            return e1->size;
        else if (e1->size > e2->size) {
            if (e2->size == 0) {
                if (e1->size < default_len)
                    return e2->size;
                else
                    return e1->size;
            } else
                return e1->size;
        } else if (e1->size < e2->size) {
            if (e1->size == 0) {
                if (e2->size < default_len)
                    return e1->size;
                else
                    return e2->size;
            } else
                return e2->size;
        }
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     DECLARATION NODES                                         *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * SPECIFIER NODES
 */

char *SPEC_symbols[30] =
    {NULL, "typedef", "extern", "static", "auto", "register", "void",
     "char", "short", "int", "long", "float", "double", "signed",
     "unsigned", "_Bool", "_Complex", "_Imaginary", "struct",
     "union", "enum", "const", "restrict", "volatile", "inline",
     "*", "specslistL", "speclistR", "private", "public"};

astspec Specifier(enum spectype type, int subtype, symbol name, astspec body, int size, int sizeexp) {
    astspec d = smalloc(sizeof(struct astspec_));
    d->type = type;
    d->subtype = subtype;
    d->is_spec_for_sng = 0;
    d->size = size;
    d->sizeexp = sizeexp;
    d->name = name;
    d->body = body;
    d->u.next = NULL;
    d->l = sc_original_line();
    d->c = sc_column();
    d->file = Symbol(sc_original_file());
    return (d);
}

astspec Enumerator(symbol name, astexpr expr) {
    astspec d = Specifier(ENUMERATOR, 0, name, NULL, 0, 0);
    d->u.expr = expr;
    return (d);
}

astspec Specifierlist(int type, astspec e, astspec l) {
    astspec d = Specifier(SPECLIST, type, NULL, e, 0, 0);
    d->u.next = l;
    return (d);
}

astspec SUdecl(int type, symbol sym, astdecl decl) {
    astspec d = Specifier(SUE, type, sym, NULL, 0, 0);
    d->u.decl = decl;
    return (d);
}

/*
 * DECLARATOR NODES
 */

astdecl Decl(enum decltype type, int subtype, astdecl decl, astspec spec) {
    astdecl d = smalloc(sizeof(struct astdecl_));
    d->type = type;
    d->subtype = subtype;
    d->is_decl_for_sng = 0;
    d->decl = decl;
    d->spec = spec;
    d->u.next = NULL;
    d->l = sc_original_line();
    d->c = sc_column();
    d->file = Symbol(sc_original_file());
    return (d);
}

astdecl IdentifierDecl(symbol s) {
    astdecl d = Decl(DIDENT, 0, NULL, NULL);
    d->u.id = s;
    return (d);
}

astdecl ArrayDecl(astdecl decl, astspec s, astexpr e) {
    if (e != NULL) {
        if (e->flag == PRI){ // array length cannot be private var
            if (e->ftype == 1)
                exit_error(1, "     Size of array '%s' has non-integer type and cannot be private.\n", decl->u.id->name);
            exit_error(1, "     Size of allocated memory (array '%s') cannot be private.\n", decl->u.id->name);
        } else if (e->flag == PUB && e->ftype == 1) {
            exit_error(1, "     Size of array '%s' has non-integer type.\n", decl->u.id->name);
        }
    } else if (e == NULL) {
        exit_error(1, "     Array size missing in '%s'\n", decl->u.id->name);
    }
    astdecl d = Decl(DARRAY, 0, decl, s);
    d->u.expr = e;
    return (d);
}

astdecl FuncDecl(astdecl decl, astdecl p) {
    astdecl d = Decl(DFUNC, 0, decl, NULL);
    d->u.params = p;
    return (d);
}

astdecl InitDecl(astdecl decl, astexpr e) {
    astdecl d = Decl(DINIT, 0, decl, NULL);
    d->u.expr = e;
    // This code was added for tmp array declarations that were used for batch arrays, was removed cause it is not used anymore
    // The code below stores all the variables with thier values to the table to be able to use them for init of temp arrays for the case if arrays in a program is init using a variable and not a constant -> this was needed because we added support for temp arrays and we needed a max size to initilize them 
    // if (e && decl->decl) {
    //     if (e->u.str && decl->decl->u.id) { // Rest
    //         insert_variable(decl->decl->u.id->name, e->u.str);
    //     } else if (decl->decl->u.expr->u.str) { // Dynamic array init that has an expression after the assignment
    //         if (decl->decl->u.expr->type == CONSTVAL) { // if Const
    //             insert_variable(decl->decl->u.expr->u.str, decl->decl->u.expr->u.str);
    //         }
    //     }
    // }
    return (d);
}

astdecl BitDecl(astdecl decl, astexpr e) {
    astdecl d = Decl(DBIT, 0, decl, NULL);
    d->u.expr = e;
    return (d);
}

astdecl Declanylist(int subtype, astdecl l, astdecl e) {
    astdecl d = Decl(DLIST, subtype, e, NULL);
    d->u.next = l;
    return (d);
}
/* Get the type of identifier declared - can be:
 *   - a scalar one
 *   - an array
 *   - a function
 * d is assumed to be a declarator node. As such it only has a ptr and
 * a direct_declarator child.
 */
int decl_getkind(astdecl d) {
    assert(d->type == DECLARATOR);
    if (decl_ispointer(d))
        return (DIDENT); /* pointers are scalar */
    d = d->decl;         /* direct_declarator */

    switch (d->type) {
    case DPAREN:
        return (decl_getkind(d->decl));
    case DFUNC:
        return (DFUNC);
    case DARRAY:
        return (DARRAY);
    case DIDENT:
        return (DIDENT);
    case DECLARATOR: /* Should not happen normally */
        return (decl_getkind(d));
    default:
        exit_error(1, "[decl_getkind]: unexpected declarator type %d\n", d->type);
    }
    return (0);
}

/* Determine whether the declarator is a pointer. Here we are based on the
 * fact that the parser has removed redundant parenthesis, i.e.
 * (IDENT) has been converted to IDENT. Thus we can only have a pointer
 * if we are in a situation like ...(*IDENT)...; i.e. an identifier
 * declarator with pointer specifier, or ...(*)... for an abstract declarator.
 */
int decl_ispointer(astdecl d) {
    if (d->type == DECLARATOR || d->type == ABSDECLARATOR) {
        if (d->spec != NULL &&
            (d->decl == NULL || (d->decl->type == DIDENT &&
                                 speclist_getspec(d->spec, SPEC, SPEC_star) != NULL)))
            return (1);
    }
    if (d->type == DIDENT)
        return (0);
    else if (d->type == DLIST) /* Should be DECL_decllist */
        return (decl_ispointer(d->u.next));
    else
        return (decl_ispointer(d->decl));
}

/* Get the identifier name of the declarator.
 * d is assumed to be the declarator part of a declaration (top-level).
 * It will crash if given an ABSDECLARATOR with no identifier!
 */
astdecl decl_getidentifier(astdecl d) {
    if (d->type == DIDENT) // This is from decl (astdecl_), id (symbol_), type (int) -> // int: 0 and float: 1 and struct: 2
        return (d);
    else if (d->type == DLIST) /* Should be DECL_decllist */
        return (decl_getidentifier(d->u.next));
    else
        return (decl_getidentifier(d->decl));
}

/* Checks whether the speclist includes the given SPEC type */
astspec speclist_getspec(astspec s, int type, int subtype) {
    if (s == NULL)
        return (NULL);
    if (s->type == SPECLIST) {
        astspec p;
        if ((p = speclist_getspec(s->body, type, subtype)) != NULL)
            return (p);
        return (speclist_getspec(s->u.next, type, subtype));
    }
    if (s->type != type)
        return (NULL);
    if (type != SPEC && type != STCLASSSPEC)
        return (s);
    return ((s->subtype == subtype) ? s : NULL);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     STATEMENT NODES                                           *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

aststmt Statement(enum stmttype type, int subtype, aststmt body) {
    aststmt s = smalloc(sizeof(struct aststmt_));
    s->flag = 0; // default flag is pub
    s->gflag = 0; // default is non global
    s->is_stmt_for_sng = 0;
    s->type = type;
    s->subtype = subtype;
    s->body = body;
    s->parent = NULL;
    s->openmp_parent_stmt = NULL;
    s->l = sc_original_line();
    s->c = sc_column();
    s->file = Symbol(sc_original_file());
    return (s);
}

aststmt Goto(symbol s) {
    aststmt n = Statement(JUMP, SGOTO, NULL);
    n->u.label = s;
    return (n);
}

aststmt Smc(int subtype, astspec type, astexpr e1, astexpr e2, astexpr size1, astexpr size2) {
    aststmt s = Statement(SMC, subtype, NULL);
    if (type != NULL) {
        if (type->type == SPECLIST)
            s->u.smcops.type = type->u.next;
        else
            s->u.smcops.type = type;
    }
    s->u.smcops.variable = e1;
    s->u.smcops.party = e2;
    s->u.smcops.size1 = size1;
    s->u.smcops.size2 = size2;
    return s;
}
// by Ghada
astexpr PmallocExpr(astexpr e1, astdecl d) {
    astexpr s = Astexpr(EPMALLOC, 0, NULL);
    s->left = e1;
    s->u.dtype = d;
    return s;
}

aststmt Pfree(astexpr e1) {
    aststmt s = Statement(SSPFREE, 0, NULL);
    s->u.pfreeops.variable = e1;
    return s;
}

aststmt Batch(aststmt init, astexpr cond, astexpr incr, aststmt body) {

    aststmt n = Statement(BATCH, 0, body);
    n->u.iteration.init = init; /* Maybe declaration or expression */
    n->u.iteration.cond = cond;
    n->u.iteration.incr = incr;
    return (n);
}

aststmt Jumpstatement(int subtype, astexpr expr) {
    aststmt s = Statement(JUMP, subtype, NULL);
    s->u.expr = expr;
    return (s);
}

aststmt Iterationstatement(int subtype,
                           aststmt init, astexpr cond, astexpr incr, aststmt body) {
    aststmt n = Statement(ITERATION, subtype, body);
    n->u.iteration.init = init; /* Maybe declaration or expression */
    n->u.iteration.cond = cond;
    n->u.iteration.incr = incr;
    return (n);
}

aststmt Selectionstatement(int subtype,
                           astexpr cond, aststmt body, aststmt elsebody) {
    aststmt n = Statement(SELECTION, subtype, body);
    n->u.selection.cond = cond;
    n->u.selection.elsebody = elsebody;
    return (n);
}

aststmt LabeledStatement(int subtype, symbol l, astexpr e, aststmt st) {
    aststmt s = Statement(LABELED, subtype, st);
    if (subtype == SLABEL)
        s->u.label = l;
    else
        s->u.expr = e;
    return (s);
}

aststmt Expression(astexpr expr) {
    aststmt s = Statement(EXPRESSION, 0, NULL);
    s->u.expr = expr;
    return (s);
}

aststmt Declaration(astspec spec, astdecl decl) {
    aststmt s = Statement(DECLARATION, 0, NULL);
    s->u.declaration.spec = spec;
    s->u.declaration.decl = decl;
    return (s);
}

aststmt BlockList(aststmt l, aststmt st) {
    aststmt s = Statement(STATEMENTLIST, 0, st);
    s->u.next = l;
    return (s);
}

aststmt FuncDef(astspec spec, astdecl decl, aststmt dlist, aststmt body) {
    aststmt s = Statement(FUNCDEF, 0, body);
    s->u.declaration.spec = spec;
    s->u.declaration.decl = decl;
    s->u.declaration.dlist = dlist;
    return (s);
}

aststmt OmpStmt(ompcon omp) {
    aststmt s = Statement(OMPSTMT, 0, NULL);
    s->u.omp = omp;
    return (s);
}

aststmt OmpixStmt(oxcon ox) {
    aststmt s = Statement(OX_STMT, 0, NULL);
    s->u.ox = ox;
    return (s);
}

aststmt Verbatim(char *code) {
    aststmt s = Statement(VERBATIM, 0, NULL);
    s->u.code = code;
    return (s);
}

aststmt verbit(char *format, ...) {
    static char str[3096];
    va_list ap;
    va_start(ap, format);
    vsnprintf(str, 3096, format, ap);
    va_end(ap);
    return (Verbatim(strdup(str)));
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OPENMP NODES                                              *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

char *clausenames[20] = {
    NULL, "nowait", "if", "num_threads", "ordered",
    "schedule", "copyin", "private", "copyprivate",
    "firstprivate", "lastprivate", "shared",
    "default", "reduction", "<list>",
    "first/lastprivate",
    "untied", "collapse", /* OpenMP 3.0 */
    "final", "mergeable"  /* OpenMP 3.1 */
};
char *clausesubs[19] = {
    "static", "dynamic", "guided", "runtime",
    "shared", "none", "+", "*", "-", "&", "|",
    "^", "&&", "||", "affinity", "auto",
    "min", "max" /* OpenMP 3.1 */
};
char *ompdirnames[19] = {
    NULL, "parallel", "for", "sections", "section",
    "single", "parallel for", "parallel sections",
    "for", "master", "critical", "atomic",
    "ordered", "barrier", "flush", "threadprivate",
    "task", "taskwait", /* OpenMP 3.0 */
    "taskyield"         /* OpenMP 3.1 */
};

ompclause
OmpClause(enum clausetype type, int subtype, astexpr expr, astdecl varlist) {
    ompclause c = smalloc(sizeof(struct ompclause_));
    c->parent = NULL;
    c->type = type;
    c->subtype = subtype;
    if (varlist == NULL)
        c->u.expr = expr;
    else
        c->u.varlist = varlist;
    c->l = sc_original_line();
    c->c = sc_column();
    c->file = Symbol(sc_original_file());
    return (c);
}

ompclause OmpClauseList(ompclause next, ompclause elem) {
    ompclause c = OmpClause(OCLIST, 0, NULL, NULL);
    c->u.list.elem = elem;
    c->u.list.next = next;
    return (c);
}

ompdir OmpDirective(enum dircontype type, ompclause cla) {
    ompdir d = smalloc(sizeof(struct ompdir_));
    d->parent = NULL;
    d->type = type;
    d->clauses = cla;
    d->u.varlist = NULL;
    d->l = sc_original_line() - 1; /* Cause of the \n at the end */
    d->c = sc_column();
    d->file = Symbol(sc_original_file());
    return (d);
}

ompdir OmpCriticalDirective(symbol r) {
    ompdir d = OmpDirective(DCCRITICAL, NULL);
    d->u.region = r;
    return (d);
}

ompdir OmpFlushDirective(astdecl a) {
    ompdir d = OmpDirective(DCFLUSH, NULL);
    d->u.varlist = a;
    return (d);
}

ompdir OmpThreadprivateDirective(astdecl a) {
    ompdir d = OmpDirective(DCTHREADPRIVATE, NULL);
    d->u.varlist = a;
    return (d);
}

ompcon OmpConstruct(enum dircontype type, ompdir dir, aststmt body) {
    ompcon c = smalloc(sizeof(struct ompcon_));
    c->parent = NULL;
    c->type = type;
    c->directive = dir;
    c->body = body;
    c->l = sc_original_line();
    c->c = sc_column();
    c->file = Symbol(sc_original_file());
    return (c);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OMPi-EXTENSION NODES                                      *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

char *oxclausenames[15] = {NULL, "IN", "OUT", "INOUT", "<list>", "reduction",
                           "atnode", "atnode(*)", "detached", "tied",
                           "untied", "stride", "start", "scope",
                           "atworker"};
char *oxdirnames[5] = {NULL, "taskdef", "task", "tasksync", "taskschedule"};

oxclause OmpixClause(enum oxclausetype type, astdecl varlist, astexpr expr) {
    oxclause c = smalloc(sizeof(struct oxclause_));
    c->parent = NULL;
    c->type = type;
    if (expr == NULL)
        c->u.varlist = varlist;
    else
        c->u.expr = expr;
    c->l = sc_original_line();
    c->c = sc_column();
    c->file = Symbol(sc_original_file());
    return (c);
}

oxclause OmpixClauseList(oxclause next, oxclause elem) {
    oxclause c = OmpixClause(OX_OCLIST, NULL, NULL);
    c->u.list.elem = elem;
    c->u.list.next = next;
    return (c);
}

oxclause OmpixReductionClause(int op, astdecl varlist) {
    oxclause c = OmpixClause(OX_OCREDUCE, varlist, NULL);
    c->operator= op;
    return (c);
}

oxclause OmpixScopeClause(int scope) {
    oxclause c = OmpixClause(OX_OCSCOPE, NULL, NULL);
    c->u.value = scope;
    return (c);
}

oxdir OmpixDirective(enum oxdircontype type, oxclause cla) {
    oxdir d = smalloc(sizeof(struct oxdir_));
    d->parent = NULL;
    d->type = type;
    d->clauses = cla;
    d->l = sc_original_line() - 1; /* Cause of the \n at the end */
    d->c = sc_column();
    d->file = Symbol(sc_original_file());
    return (d);
}

oxcon OmpixConstruct(enum oxdircontype type, oxdir dir, aststmt body) {
    oxcon c = smalloc(sizeof(struct oxcon_));
    c->parent = NULL;
    c->type = type;
    c->directive = dir;
    c->body = body;
    c->callback = NULL;
    c->l = sc_original_line();
    c->c = sc_column();
    c->file = Symbol(sc_original_file());
    return (c);
}

oxcon OmpixTaskdef(oxdir dir, aststmt body, aststmt callbackblock) {
    oxcon c = OmpixConstruct(OX_DCTASKDEF, dir, body);
    c->callback = callbackblock;
    return (c);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     UTILITY FUNCTIONS                                         *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Parentize:
 *    register the parent statement of every statement node.
 * No parents within expressions or declarators/specifiers.
 * For an openmp statement O, the construct's parent and the
 * construct's body parent is node O. The directive's parent is
 * the construct and the clauses' parent is the directive.
 */

void ast_ompclause_parent(ompdir parent, ompclause t) {
    if (t->type == OCLIST) {
        if (t->u.list.next != NULL)
            ast_ompclause_parent(parent, t->u.list.next);
        assert((t = t->u.list.elem) != NULL);
    }
    t->parent = parent;
}

void ast_ompdir_parent(ompcon parent, ompdir t) {
    t->parent = parent;
    if (t->clauses)
        ast_ompclause_parent(t, t->clauses);
}

void ast_ompcon_parent(aststmt parent, ompcon t) {
    t->parent = parent;
    ast_ompdir_parent(t, t->directive);
    if (t->body) /* barrier & flush don't have a body */
        ast_stmt_parent(parent, t->body);
}

void ast_oxclause_parent(oxdir parent, oxclause t) {
    if (t->type == OX_OCLIST) {
        if (t->u.list.next != NULL)
            ast_oxclause_parent(parent, t->u.list.next);
        assert((t = t->u.list.elem) != NULL);
    }
    t->parent = parent;
}

void ast_oxdir_parent(oxcon parent, oxdir t) {
    t->parent = parent;
    if (t->clauses)
        ast_oxclause_parent(t, t->clauses);
}

void ast_oxcon_parent(aststmt parent, oxcon t) {
    t->parent = parent;
    ast_oxdir_parent(t, t->directive);
    if (t->body) /* barrier & flush don't have a body */
        ast_stmt_parent(parent, t->body);
}

void ast_stmt_parent(aststmt parent, aststmt t) {
    switch (t->type) {
    case JUMP:
        break;
    case ITERATION:
        if (t->subtype == SFOR)
            if (t->u.iteration.init != NULL)
                ast_stmt_parent(t, t->u.iteration.init);
        ast_stmt_parent(t, t->body);
        break;
    case SELECTION:
        if (t->subtype == SIF && t->u.selection.elsebody)
            ast_stmt_parent(t, t->u.selection.elsebody);
        ast_stmt_parent(t, t->body);
        break;
    case LABELED:
        ast_stmt_parent(t, t->body);
        break;
    case EXPRESSION:
        break;
    case COMPOUND:
        if (t->body)
            ast_stmt_parent(t, t->body);
        break;
    case STATEMENTLIST:
        ast_stmt_parent(t, t->u.next);
        ast_stmt_parent(t, t->body);
        break;
    case DECLARATION:
        break;
    case FUNCDEF:
        if (t->u.declaration.dlist)
            ast_stmt_parent(t, t->u.declaration.dlist);
        ast_stmt_parent(t, t->body); /* always non-NULL */
        break;
    case OMPSTMT:
        /* The parent of the construct and its body is this node here */
        ast_ompcon_parent(t, t->u.omp);
        break;
    case VERBATIM:
        break;
    case OX_STMT:
        /* The parent of the construct and its body is this node here */
        ast_oxcon_parent(t, t->u.ox);
        break;

    default:;
        // fprintf(stderr, "[ast_stmt_parent]: b u g (type = %d)!!\n", t->type);
    }
    t->parent = parent;
}

void ast_parentize(aststmt tree) {
    ast_stmt_parent(tree, tree);
}

/* Given a statement, we return the function it belongs to.
 */
aststmt ast_get_enclosing_function(aststmt t) {
    for (; t->type != FUNCDEF; t = t->parent)
        ; /* Go up the tree till we hit our current FUNCDEF */
    return (t);
}

/* Finds the first non-declaration node or NULL if none
 */
static aststmt first_nondeclaration(aststmt tree) {
    aststmt p;
    if (!tree || tree->type == DECLARATION)
        return (NULL);
    if (tree->type != STATEMENTLIST)
        return (tree);
    if ((p = first_nondeclaration(tree->u.next)) != NULL)
        return (p);
    else
        return (first_nondeclaration(tree->body));
}

/* Inserts a statement after the declaration section in a compound
 */
void ast_compound_insert_statement(aststmt tree, aststmt t) {
    aststmt p;

    if (tree->type != COMPOUND)
        return;
    if ((p = first_nondeclaration(tree->body)) == NULL) /* NULL or only decls */
    {
        if (tree->body == NULL) {
            tree->body = t;
            t->parent = tree;
        } else {
            tree->body = BlockList(tree->body, t);
            tree->body->parent = tree;
            tree->body->u.next->parent = tree->body;
            t->parent = tree->body;
        }
        return;
    }

    tree = Statement(p->type, p->subtype, p->body);
    *tree = *p;
    tree->parent = p;
    t->parent = p;
    p->type = STATEMENTLIST;
    p->subtype = 0;
    p->body = tree;
    p->u.next = t;
}

void struct_field_push(struct_field_stack fieldlist, astspec type, astdecl name) {
    struct_field field = struct_field_new();
    field->type = type;
    field->name = name;
    field->next = NULL;

    if (fieldlist->head == NULL)
        fieldlist->head = field;
    else {
        field->next = fieldlist->head;
        fieldlist->head = field;
    }
}

struct_field struct_field_lookup(struct_node node, char *name) {
    astdecl field_name;
    struct_field field = node->fieldlist->head;
    while (field != NULL) {
        if (field->name->decl->type == DARRAY) {
            if (field->name->decl->decl->type == DARRAY)
                field_name = field->name->decl->decl;
            else
                field_name = field->name->decl;
        } else
            field_name = field->name;
        if (field_name->decl->u.id) {
            if (!strcmp(name, field_name->decl->u.id->name)) {
                return field;
            }
        } else {
            exit_error(1, "Struct %s has no member named %s.\n", node->name->name->name, name); // the node->name->name->name is the name of the struct 
        }
        field = field->next;
    }
    return NULL;
}

struct_field struct_field_new() {
    struct_field field = smalloc(sizeof(struct struct_field_));
    return field;
}

void struct_field_free(struct_field field) {
    free(field);
}

struct_field_stack struct_field_stack_new() {
    struct_field_stack fieldlist = smalloc(sizeof(struct struct_field_stack_));
    fieldlist->head = NULL;
    return fieldlist;
}

void struct_field_stack_free(struct_field_stack fieldlist) {
    struct_field tmp = fieldlist->head;
    struct_field tmp1;
    while (tmp != NULL) {
        tmp1 = tmp->next;
        struct_field_free(tmp);
        tmp = tmp1;
    }
    free(fieldlist);
}

struct_node struct_node_new() {
    struct_node node = smalloc(sizeof(struct struct_node_));
    node->fieldlist = struct_field_stack_new();
    return node;
}

struct_node_stack struct_node_stack_new() {
    struct_node_stack structlist = smalloc(sizeof(struct struct_node_stack_));
    structlist->head = NULL;
    return structlist;
}

void struct_node_free(struct_node node) {
    free(node);
}

void struct_node_stack_free(struct_node_stack structlist) {
    struct_node tmp = structlist->head;
    struct_node tmp1;
    while (tmp != NULL) {
        tmp1 = tmp->next;
        struct_node_free(tmp);
        tmp = tmp1;
    }
    free(structlist);
}

void struct_node_push(struct_node_stack structlist, astspec name) {
    struct_node node = struct_node_new();
    node->name = name;
    node->contain_pub_field = 0;
    node->next = NULL;
    if (structlist->head == NULL)
        structlist->head = node;
    else {
        node->next = structlist->head;
        structlist->head = node;
    }
}

// determine if the struct contains any public field
void struct_node_update(struct_node_stack structlist, astdecl tree) {
    struct_node node = structlist->head;
    // iterate through the list of struct field to determine if it contains
    // any public fields
    astspec structtype = node->name;
    astdecl tmp = NULL;
    if (tree->type == DLIST)
        tmp = tree->decl;
    else
        tmp = tree;

    while (tmp != NULL) {
        astspec type = tmp->spec;
        if (type->subtype == SPEC_Rlist && type->body->subtype == SPEC_public) {
            node->contain_pub_field = 1;
            return;
        }
        // if the field is a type of struct
        else if (type->subtype == SPEC_struct) {
            // search through the structlist
            // if the type is not same as structtype
            if (strcmp(structtype->name->name, type->name->name)) {
                struct_node n = structlist->head;
                struct_node n1 = NULL;
                while (n != NULL) {
                    n1 = n->next;
                    if (!strcmp(type->name->name, n->name->name->name)) {
                        if (n->contain_pub_field) {
                            node->contain_pub_field = 1;
                            return;
                        } else
                            break;
                    }
                    n = n1;
                }
            }
        }
        if (tree->type == DLIST) {
            tree = tree->u.next;
            if (tree->type == DLIST)
                tmp = tree->decl;
            else
                tmp = tree;
        } else
            break;
    }
    node->contain_pub_field = 0;
}

int struct_node_get_flag(struct_node_stack structlist, char *struct_name) {
    struct_node n = structlist->head;
    while (n != NULL) {
        if (!strcmp(struct_name, n->name->name->name))
            return n->contain_pub_field;
        n = n->next;
    }
}

struct_node struct_node_lookup(struct_node_stack structlist, char *struct_name) {
    struct_node n = structlist->head;
    while (n != NULL) {
        if (!strcmp(struct_name, n->name->name->name))
            return n;
        n = n->next;
    }
    return NULL;
}
void if_push(astexpr cond, condstack stack) {
    condnode temp = smalloc(sizeof(struct condnode_));
    temp->element = cond;
    temp->next = stack->head;
    stack->head = temp;
}

astexpr if_pop(condstack stack) {
    condnode temp;
    temp = stack->head;
    stack->head = stack->head->next;
    return temp->element;
}

int if_isEmpty(condstack stack) {
    if (stack->head == NULL)
        return 1;
    else
        return 0;
}

int if_length(condstack stack) {
    int length = 0;
    condnode temp = stack->head;
    while (temp != NULL) {
        length++;
        temp = temp->next;
    }
    return length;
}

condstack if_stack_new() {
    condstack p = smalloc(sizeof(struct condstack_));
    p->head = NULL;
    return p;
}

void ltable_push(astspec spec, astdecl decl, ltable table) {
    lvar tmp1 = smalloc(sizeof(struct lvar_));
    tmp1->spec = spec;
    tmp1->decl = decl;
    tmp1->is_static = 1;
    tmp1->next = table->head;
    table->head = tmp1;
}

void ltable_pop(ltable table, astspec *spec, astdecl *decl) {
    lvar temp;
    temp = table->head;
    table->head = table->head->next;
    *spec = temp->spec;
    *decl = temp->decl;
}

int ltable_isEmpty(ltable table) {
    if (table->head == NULL)
        return 1;
    else
        return 0;
}

int ltable_length(ltable table) {
    int length = 0;
    lvar temp = table->head;
    while (temp != NULL) {
        length++;
        temp = temp->next;
    }
    return length;
}

ltable ltable_new() {
    ltable p = smalloc(sizeof(struct ltable_));
    p->head = NULL;
    p->next = NULL;
    return p;
}

void ltable_free(ltable table) {
    lvar tmp = table->head;
    lvar tmp1;
    while (tmp != NULL) {
        tmp1 = tmp->next;
        free(tmp);
        tmp = tmp1;
    }
    free(table);
}

/****************************/
void ltablelist_push(ltablelist tablelist) {
    ltable p = ltable_new();
    p->next = tablelist->head;
    tablelist->head = p;
}

ltable ltablelist_pop(ltablelist tablelist) {
    if (ltablelist_length(tablelist) == 0)
        return NULL;
    ltable p = tablelist->head;
    tablelist->head = tablelist->head->next;
    return p;
}

int ltablelist_length(ltablelist tablelist) {
    int length = 0;
    ltable temp = tablelist->head;
    while (temp != NULL) {
        length++;
        temp = temp->next;
    }
    return length;
}

ltablelist ltablelist_new() {
    ltablelist p = smalloc(sizeof(struct ltablelist_));
    p->head = NULL;
    return p;
}

void ltablelist_free(ltablelist tablelist) {
    ltable table = tablelist->head;
    ltable tmp;
    while (table != NULL) {
        tmp = table->next;
        ltable_free(table);
        table = tmp;
    }
    free(tablelist);
}

/********************************/
void mvar_push(astexpr var_name, mvarstack stack) {
    // if var is already stored in the stack, do not insert it.
    mvar tmp = stack->head;
    str tmp_name, name;
    name = Str("");
    ast_expr_print(name, var_name);

    while (tmp != NULL) {
        tmp_name = Str("");
        ast_expr_print(tmp_name, tmp->var_name);
        if (!strcmp(str_string(tmp_name), str_string(name)))
            return;
        tmp = tmp->next;
        str_free(tmp_name);
    }

    // otherwise, insert it.
    mvar tmp1 = smalloc(sizeof(struct mvar_));
    tmp1->var_name = var_name;
    tmp1->is_complete = 0;
    tmp1->next = stack->head;
    stack->head = tmp1;

    str_free(name);
}

astexpr mvar_pop(mvarstack stack) {
    mvar temp;
    temp = stack->head;
    stack->head = stack->head->next;
    return temp->var_name;
}

int mvar_isEmpty(mvarstack stack) {
    if (stack->head == NULL)
        return 1;
    else
        return 0;
}

int mvar_length(mvarstack stack) {
    int length = 0;
    mvar temp = stack->head;
    while (temp != NULL) {
        length++;
        temp = temp->next;
    }
    return length;
}

mvarstack mvar_stack_new() {
    mvarstack p = smalloc(sizeof(struct mvarstack_));
    p->head = NULL;
    return p;
}

void mvar_stack_free(mvarstack stack) {
    mvar tmp = stack->head;
    mvar tmp1;
    while (tmp != NULL) {
        tmp1 = tmp->next;
        free(tmp);
        tmp = tmp1;
    }
}

void batch_private_index_push(int batch_index, int private_index, int ftype, batch_private_index_stack stack) {
    batch_private_index tmp = smalloc(sizeof(struct batch_private_index_));
    tmp->batch_index = batch_index;
    tmp->private_index = private_index;
    tmp->ftype = ftype;
    tmp->next = stack->head;
    stack->head = tmp;
}

batch_private_index_stack batch_private_index_stack_new() {
    batch_private_index_stack p = smalloc(sizeof(struct batch_private_index_stack_));
    p->head = NULL;
    return p;
}

void batch_private_index_stack_free(batch_private_index_stack bpis) {
    batch_private_index tmp = bpis->head;
    batch_private_index tmp1;
    while (tmp != NULL) {
        tmp1 = tmp->next;
        free(tmp);
        tmp = tmp1;
    }
    bpis->head = NULL;
}

int batch_private_index_stack_length(batch_private_index_stack bpis) {
    int length = 0;
    batch_private_index node = bpis->head;
    while (node != NULL) {
        length++;
        node = node->next;
    }
    return length;
}

branchnode if_branchtree_new() {
    branchnode tree = smalloc(sizeof(struct branchnode_));
    tree->parent = NULL;
    tree->right = NULL;
    tree->left = NULL;
    tree->key = -1;
    tree->private_index = -1;
    tree->if_index = -1;
    tree->parent_label = -1;
    tree->current_label = -1;
    tree->ms = NULL;
    tree->tablelist = ltablelist_new();
    return tree;
}

branchnode if_branchnode_insert(branchnode parent, mvarstack ms, int key, int private_index, int if_index, int parent_label, int current_label) {

    branchnode node = smalloc(sizeof(struct branchnode_));
    node->key = key;
    node->private_index = private_index;
    node->if_index = if_index;
    node->parent_label = parent_label;
    node->current_label = current_label;
    node->tablelist = ltablelist_new();
    node->ms = ms;
    if (key == 0)
        parent->left = node;
    else if (key == 1)
        parent->right = node;
    node->parent = parent;

    return node;
}

int if_branchnode_height(branchnode node) {
    int length = 0;
    branchnode tmp = node;
    while (tmp->parent != NULL) {
        length++;
        tmp = tmp->parent;
    }
    return length;
}

void if_branchtree_remove(branchnode current, branchnode node) {
    if (node->key == 0)
        current->left = NULL;
    else
        current->right = NULL;
    if (node->ms != NULL)
        mvar_stack_free(node->ms);
    if (node->tablelist != NULL)
        ltablelist_free(node->tablelist);
    free(node);
}

void if_branchtree_print(branchnode root) {
    if (root != NULL) {
        printf("%d", root->key);
        if_branchtree_print(root->left);
        if_branchtree_print(root->right);
        printf("\n");
    }
}

void if_branchtree_free(branchnode root) {
    if (root->right == NULL && root->left != NULL)
        if_branchtree_free(root->left);
    else if (root->left == NULL && root->right != NULL)
        if_branchtree_free(root->right);
    else if (root->left != NULL && root->right != NULL) {
        if_branchtree_free(root->left);
        if_branchtree_free(root->right);
    }
    if (root->ms != NULL)
        mvar_stack_free(root->ms);
    if (root->tablelist != NULL)
        ltablelist_free(root->tablelist);
    free(root);
}

void control_sequence_push(int index, control_sequence_stack stack) {
    control_sequence tmp = smalloc(sizeof(struct control_sequence_));
    tmp->index = index;
    tmp->batch_index = 0;
    tmp->next = stack->head;
    stack->head = tmp;
}

void control_sequence_pop(control_sequence_stack stack) {
    control_sequence tmp;
    tmp = stack->head;
    stack->head = tmp->next;
    free(tmp);
}
control_sequence_stack control_sequence_stack_new() {
    control_sequence_stack p = smalloc(sizeof(struct control_sequence_stack_));
    p->head = NULL;
    return p;
}

int control_sequence_stack_length(control_sequence_stack stack) {
    int length = 0;
    control_sequence tmp = stack->head;
    while (tmp != NULL) {
        length++;
        tmp = tmp->next;
    }
    return length;
}

control_record_stack control_record_stack_new() {
    control_record_stack p = smalloc(sizeof(struct control_record_stack_));
    p->head = NULL;
    return p;
}

int control_record_push(astexpr var_name, astexpr arrayindex, int if_index, int iter_index, control_record_stack stack) {

    control_record tmp = stack->head;
    str tmp_name, tmp_index, arg_name, arg_index;
    arg_name = Str("");
    arg_index = Str("");
    ast_expr_print(arg_name, var_name);
    ast_expr_print(arg_index, arrayindex);

    while (tmp != NULL) {
        tmp_name = Str("");
        tmp_index = Str("");
        ast_expr_print(tmp_name, tmp->var_name);
        ast_expr_print(tmp_index, tmp->arrayindex);

        if (!strcmp(str_string(arg_name), str_string(tmp_name)) && !strcmp(str_string(arg_index), str_string(tmp_index)) && (tmp->if_index == if_index) && (tmp->iter_index == iter_index))
            return 0;
        tmp = tmp->next;
        str_free(tmp_name);
        str_free(tmp_index);
    }

    control_record tmp1 = smalloc(sizeof(struct control_record_));
    tmp1->var_name = var_name;
    tmp1->arrayindex = arrayindex;
    tmp1->if_index = if_index;
    tmp1->iter_index = iter_index;
    tmp1->flag = 0;
    tmp1->next = stack->head;
    stack->head = tmp1;

    str_free(arg_name);
    str_free(arg_index);
    return 1;
}

void control_record_set_branch(int if_index, branchnode branch, control_record_stack stack) {
    control_record tmp = stack->head;
    while (tmp != NULL) {
        if (tmp->if_index == if_index)
            tmp->branch = branch;
        tmp = tmp->next;
    }
}

void control_record_print(control_record_stack stack) {
    control_record tmp = stack->head;
    str tmp_name, tmp_index;

    while (tmp != NULL) {
        tmp_name = Str("");
        tmp_index = Str("");
        ast_expr_print(tmp_name, tmp->var_name);
        ast_expr_print(tmp_index, tmp->arrayindex);
        printf("%s%s, %d, %d, %d\n", str_string(tmp_name), str_string(tmp_index), tmp->if_index, tmp->iter_index, tmp->tmp_index);
        tmp = tmp->next;
        str_free(tmp_name);
        str_free(tmp_index);
    }
}

iteration create_iteration(aststmt init, astexpr cond, astexpr incr) {
    iteration iter = smalloc(sizeof(struct iteration_));
    iter->init = init;
    iter->cond = cond;
    iter->incr = incr;
    return iter;
}

batch_condition_stack batch_condition_stack_new() {
    batch_condition_stack p = smalloc(sizeof(struct batch_condition_stack_));
    p->head = NULL;
    return p;
}

void batch_condition_push(iteration iter, batch_condition_stack stack) {
    batch_condition tmp = smalloc(sizeof(struct batch_condition_));
    tmp->condition = iter;
    tmp->next = stack->head;
    stack->head = tmp;
}

void batch_statement_push(aststmt statement, int statement_index, int batch_index, int flag, batch_statement_stack stack) {
    batch_statement s = smalloc(sizeof(struct batch_statement_));
    s->statement = statement;
    s->statement_index = statement_index;
    s->batch_index = batch_index;
    s->flag = flag;
    s->next = stack->head;
    stack->head = s;
}

void batch_statement_popAll(batch_statement_stack stack) {
    batch_statement tmp = stack->head;
    batch_statement tmp1;
    while (tmp != NULL) {
        tmp1 = tmp->next;
        if (tmp1 == NULL)
            break;
        free(tmp);
        tmp = tmp1;
    }
    stack->head = NULL;
}

int batch_statement_size(batch_statement_stack stack) {
    int size = 0;
    batch_statement tmp = stack->head;
    while (tmp != NULL) {
        size++;
        tmp = tmp->next;
    }
    return size;
}

batch_statement_stack batch_statement_stack_new() {
    batch_statement_stack p = smalloc(sizeof(struct batch_statement_stack_));
    p->head = NULL;
    return p;
}
