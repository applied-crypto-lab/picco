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

/* ast_copy.c -- create a copy of an AST */

#include "ast_copy.h"
#include <stdio.h>
#include <string.h>

/* The _almost functions do the job; the others simply
 * copy the original location information (line,column,file)
 * of the node.
 */

astexpr ast_expr_copy_almost(astexpr tree);
aststmt ast_stmt_copy_almost(aststmt stmt);
astdecl ast_decl_copy_almost(astdecl tree);
astspec ast_spec_copy_almost(astspec tree);
ompcon ast_ompcon_copy_almost(ompcon tree);
ompclause ast_ompclause_copy(ompclause tree);
ompdir ast_ompdir_copy(ompdir tree);
astspec ast_spec_copy_nosc_asis(astspec tree);

astexpr same_expr(astexpr n) {
    astexpr s = smalloc(sizeof(struct astexpr_));
    *s = *n;
    return (s);
}
astexpr lr_expr(astexpr n) {
    astexpr s = smalloc(sizeof(struct astexpr_));
    *s = *n;
    s->left = ast_expr_copy(n->left);
    s->right = ast_expr_copy(n->right);
    return (s);
}
aststmt same_stmt(aststmt n) {
    aststmt s = smalloc(sizeof(struct aststmt_));
    *s = *n;
    return (s);
}
astdecl same_decl(astdecl n) {
    astdecl s = smalloc(sizeof(struct astdecl_));
    *s = *n;
    return (s);
}
astdecl sd_decl(astdecl n) {
    astdecl s = smalloc(sizeof(struct astdecl_));
    *s = *n;
    s->spec = ast_spec_copy(n->spec);
    s->decl = ast_decl_copy(n->decl);
    return (s);
}
astspec same_spec(astspec n) {
    astspec s = smalloc(sizeof(struct astspec_));
    *s = *n;
    return (s);
}

aststmt ast_stmt_jump_copy(aststmt tree) {
    if (tree == NULL)
        return (NULL);
    switch (tree->subtype) {
    case SBREAK:
    case SCONTINUE:
    case SGOTO:
        return (same_stmt(tree));
    case SRETURN:
        return (Return(ast_expr_copy(tree->u.expr)));
    default:
        fprintf(stderr, "[ast_stmt_jump_copy]: b u g !!\n");
        return (NULL);
    }
}

aststmt ast_stmt_iteration_copy(aststmt tree) {
    if (tree == NULL)
        return (NULL);
    switch (tree->subtype) {
    case SFOR:
        return (For(ast_stmt_copy(tree->u.iteration.init),
                    ast_expr_copy(tree->u.iteration.cond),
                    ast_expr_copy(tree->u.iteration.incr),
                    ast_stmt_copy(tree->body)));
    case SWHILE:
        return (While(ast_expr_copy(tree->u.iteration.cond),
                      ast_stmt_copy(tree->body)));
    case SDO:
        return (Do(ast_stmt_copy(tree->body),
                   ast_expr_copy(tree->u.iteration.cond)));
    default:
        fprintf(stderr, "[ast_stmt_iteration_copy]: b u g !!\n");
        return (NULL);
    }
}

aststmt ast_stmt_selection_copy(aststmt tree) {
    if (tree == NULL)
        return (NULL);
    switch (tree->subtype) {
    case SSWITCH:
        return (Switch(ast_expr_copy(tree->u.selection.cond),
                       ast_stmt_copy(tree->body)));
    case SIF:
        return (If(ast_expr_copy(tree->u.selection.cond),
                   ast_stmt_copy(tree->body),
                   ast_stmt_copy(tree->u.selection.elsebody)));
    default:
        fprintf(stderr, "[ast_stmt_selection_copy]: b u g !!\n");
        return (NULL);
    }
}

aststmt ast_stmt_labeled_copy(aststmt tree) {
    if (tree == NULL)
        return (NULL);
    switch (tree->subtype) {
    case SLABEL:
        return (Labeled(tree->u.label, ast_stmt_copy(tree->body)));
    case SCASE:
        return (Case(ast_expr_copy(tree->u.expr), ast_stmt_copy(tree->body)));
    case SDEFAULT:
        return (Default(ast_stmt_copy(tree->body)));
    default:
        fprintf(stderr, "[ast_stmt_labeled_copy]: b u g !!\n");
        return (NULL);
    }
}

aststmt ast_stmt_copy_almost(aststmt tree) {
    if (tree == NULL)
        return (NULL);
    switch (tree->type) {
    case JUMP:
        return (ast_stmt_jump_copy(tree));
    case ITERATION:
        return (ast_stmt_iteration_copy(tree));
    case SELECTION:
        return (ast_stmt_selection_copy(tree));
    case LABELED:
        return (ast_stmt_labeled_copy(tree));
    case EXPRESSION:
        return (Expression(ast_expr_copy(tree->u.expr)));
    case COMPOUND:
        return (Compound(ast_stmt_copy(tree->body)));
    case STATEMENTLIST:
        return (BlockList(ast_stmt_copy(tree->u.next),
                          ast_stmt_copy(tree->body)));
    case DECLARATION:
        return (Declaration(ast_spec_copy(tree->u.declaration.spec),
                            ast_decl_copy(tree->u.declaration.decl)));
    case FUNCDEF:
        return (FuncDef(ast_spec_copy(tree->u.declaration.spec),
                        ast_decl_copy(tree->u.declaration.decl),
                        ast_stmt_copy(tree->u.declaration.dlist),
                        ast_stmt_copy(tree->body)));
    case OMPSTMT:
        return (OmpStmt(ast_ompcon_copy(tree->u.omp)));
    case VERBATIM:
        return (Verbatim(strdup(tree->u.code)));
    case OX_STMT:
        return (OmpixStmt(ast_oxcon_copy(tree->u.ox)));
    default:
        fprintf(stderr, "[ast_stmt_copy]: b u g !!\n");
        return (NULL);
    }
}

astexpr ast_expr_copy_almost(astexpr tree) {
    if (tree == NULL)
        return (NULL);
    switch (tree->type) {
    case IDENT:
    case DOTDES:
        return same_expr(tree);
    case CONSTVAL: {
        astexpr e = Constant(strdup(tree->u.str));
        e->flag = PUB;
        return (e);
    }
    case STRING:
        return (String(strdup(tree->u.str)));
    case FUNCCALL:
    case ARRAYIDX:
    case DOTFIELD:
    case PTRFIELD:
    case BRACEDINIT:
    case BOP:
    case PREOP:
    case POSTOP:
    case ASS:
    case DESIGNATED:
    case IDXDES:
    case COMMALIST:
    case SPACELIST:
        return (lr_expr(tree));
    case CASTEXPR:
        return (CastedExpr(ast_decl_copy(tree->u.dtype),
                           ast_expr_copy(tree->left)));
    case CONDEXPR:
        return (ConditionalExpr(ast_expr_copy(tree->u.cond),
                                ast_expr_copy(tree->left),
                                ast_expr_copy(tree->right)));
    case UOP:
        if (tree->opid == UOP_sizeoftype)
            return (Sizeoftype(ast_decl_copy(tree->u.dtype)));
        else if (tree->opid == UOP_typetrick)
            return (TypeTrick(ast_decl_copy(tree->u.dtype)));
        else
            return (lr_expr(tree));
    default:
        fprintf(stderr, "[ast_expr_copy]: b u g !!\n");
        return (NULL);
    }
}

astspec ast_spec_copy_almost(astspec tree) {
    if (tree == NULL)
        return (NULL);
    switch (tree->type) {
    case SPEC:
    case STCLASSSPEC:
    case USERTYPE:
        return (same_spec(tree));
    case SUE:
        switch (tree->subtype) {
        case SPEC_enum:
            return (Enumdecl(tree->name, ast_spec_copy(tree->body)));
        case SPEC_struct:
        case SPEC_union:
            return (SUdecl(tree->subtype,
                           tree->name, ast_decl_copy(tree->u.decl)));
        default:
            fprintf(stderr, "[ast_spec_copy]: SUE b u g !!\n");
            return (NULL);
        }
    case ENUMERATOR:
        return (Enumerator(tree->name, ast_expr_copy(tree->u.expr)));
    case SPECLIST:
        return (Specifierlist(tree->subtype,
                              ast_spec_copy(tree->body),
                              ast_spec_copy(tree->u.next)));
    default:
        fprintf(stderr, "[ast_spec_copy]: b u g !!\n");
        return (NULL);
    }
}

/* Special version which throws away all storage class specifiers
 * (extern, auto, register, etc) & the const type qualifer.
 */
astspec ast_spec_copy_nosc_almost(astspec tree) {
    if (tree == NULL)
        return (NULL);
    switch (tree->type) {
    case STCLASSSPEC: /* Throw it away! */
        return (NULL);
    case SPEC:
        if (tree->subtype == SPEC_const)
            return (NULL);
    case USERTYPE:
        return (same_spec(tree));
    case SUE:
        switch (tree->subtype) {
        case SPEC_enum:
            return (Enumdecl(tree->name, ast_spec_copy(tree->body)));
        case SPEC_struct:
        case SPEC_union:
            return (SUdecl(tree->subtype,
                           tree->name, ast_decl_copy(tree->u.decl)));
        default:
            fprintf(stderr, "[ast_spec_copy]: SUE b u g !!\n");
            return (NULL);
        }
    case ENUMERATOR:
        return (Enumerator(tree->name, ast_expr_copy(tree->u.expr)));
    case SPECLIST: {
        astspec body, next;

        /* If we used ast_spec_copy_nosc(), then body and next would never
         * be NULL!
         */
        body = ast_spec_copy_nosc_asis(tree->body);
        next = ast_spec_copy_nosc_asis(tree->u.next);
        if (body != NULL && next != NULL)
            return (Specifierlist(tree->subtype, body, next));
        if (body != NULL)
            return (body);
        return (next);
    }
    default:
        fprintf(stderr, "[ast_spec_copy]: b u g !!\n");
        return (NULL);
    }
}

astdecl ast_decl_copy_almost(astdecl tree) {
    if (tree == NULL)
        return (NULL);
    switch (tree->type) {
    case DIDENT:
    case DELLIPSIS:
        return (same_decl(tree));
    case DPAREN:
        return (ParenDecl(ast_decl_copy(tree->decl)));
    case DARRAY: {
        astdecl d = ArrayDecl(ast_decl_copy(tree->decl),
                              ast_spec_copy(tree->spec),
                              ast_expr_copy(tree->u.expr));
        return d;
    }
    case DFUNC: /* Maybe abstract declarator */
        return (FuncDecl(ast_decl_copy(tree->decl),
                         ast_decl_copy(tree->u.params)));
    case DINIT:
        return (InitDecl(ast_decl_copy(tree->decl),
                         ast_expr_copy(tree->u.expr)));
    case DECLARATOR:
    case ABSDECLARATOR:
    case DPARAM:
    case DSTRUCTFIELD:
    case DCASTTYPE:
        return (sd_decl(tree));
    case DBIT:
        return (BitDecl(ast_decl_copy(tree->decl),
                        ast_expr_copy(tree->u.expr)));
    case DLIST:
        return (Declanylist(tree->subtype,
                            ast_decl_copy(tree->u.next),
                            ast_decl_copy(tree->decl)));
    default:
        fprintf(stderr, "[ast_decl_copy]: b u g (type %d) !!\n", tree->type);
        return (NULL);
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OpenMP NODES                                              *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

ompclause ast_ompclause_copy_almost(ompclause t) {
    if (t == NULL)
        return (NULL);
    switch (t->type) {
    case OCIF:
        return (IfClause(ast_expr_copy(t->u.expr)));
    case OCFINAL:
        return (FinalClause(ast_expr_copy(t->u.expr)));
    case OCNUMTHREADS:
        return (NumthreadsClause(ast_expr_copy(t->u.expr)));
    case OCSCHEDULE:
        return (ScheduleClause(t->subtype, ast_expr_copy(t->u.expr)));
    case OCDEFAULT:
        return (DefaultClause(t->subtype));
    case OCREDUCTION:
        return (ReductionClause(t->subtype, ast_decl_copy(t->u.varlist)));
    case OCNOWAIT:
    case OCORDERED:
    case OCUNTIED:
    case OCMERGEABLE:
        return (PlainClause(t->type));
    case OCCOPYIN:
    case OCPRIVATE:
    case OCCOPYPRIVATE:
    case OCFIRSTPRIVATE:
    case OCLASTPRIVATE:
    case OCSHARED:
        return (VarlistClause(t->type, ast_decl_copy(t->u.varlist)));
    case OCLIST:
        return (OmpClauseList(ast_ompclause_copy(t->u.list.next),
                              ast_ompclause_copy(t->u.list.elem)));
    case OCCOLLAPSE:
        return (CollapseClause(t->subtype));
    default:
        fprintf(stderr, "[ast_ompclause_copy_almost]: b u g !!\n");
        return (NULL);
    }
}

ompdir ast_ompdir_copy_almost(ompdir t) {
    if (t == NULL)
        return (NULL);
    switch (t->type) {
    case DCCRITICAL:
        return (OmpCriticalDirective(t->u.region));
    case DCFLUSH:
        return (OmpFlushDirective(ast_decl_copy(t->u.varlist)));
    default:
        return (OmpDirective(t->type, ast_ompclause_copy(t->clauses)));
    }
}

ompcon ast_ompcon_copy_almost(ompcon t) {
    if (t == NULL)
        return (NULL);
    return (OmpConstruct(t->type,
                         ast_ompdir_copy(t->directive),
                         ast_stmt_copy(t->body)));
}

#define CopyPosInfo(to, from)  \
    if (to) {                  \
        to->l = from->l;       \
        to->c = from->c;       \
        to->file = from->file; \
    }

astexpr ast_expr_copy(astexpr tree) {
    astexpr t = ast_expr_copy_almost(tree);
    CopyPosInfo(t, tree);
    return (t);
}

aststmt ast_stmt_copy(aststmt tree) {
    aststmt t = ast_stmt_copy_almost(tree);
    CopyPosInfo(t, tree);
    return (t);
}

astdecl ast_decl_copy(astdecl tree) {
    astdecl t = ast_decl_copy_almost(tree);
    CopyPosInfo(t, tree);
    return (t);
}

astspec ast_spec_copy(astspec tree) {
    astspec t = ast_spec_copy_almost(tree);
    CopyPosInfo(t, tree);
    return (t);
}

/* _nosc functions don't copy storage class specs */
astspec ast_spec_copy_nosc_asis(astspec tree) {
    astspec t = ast_spec_copy_nosc_almost(tree);
    if (t != NULL)
        CopyPosInfo(t, tree);
    return (t);
}

/* This adds an "int" type in case the specifier list becomes empty */
astspec ast_spec_copy_nosc(astspec tree) {
    astspec t = ast_spec_copy_nosc_asis(tree);
    if (t == NULL) {
        t = Declspec(SPEC_int, 0);
        CopyPosInfo(t, tree);
    }
    return (t);
}

ompclause ast_ompclause_copy(ompclause tree) {
    ompclause t = ast_ompclause_copy_almost(tree);
    CopyPosInfo(t, tree);
    return (t);
}

ompdir ast_ompdir_copy(ompdir tree) {
    ompdir t = ast_ompdir_copy_almost(tree);
    CopyPosInfo(t, tree);
    return (t);
}

ompcon ast_ompcon_copy(ompcon tree) {
    ompcon t = ast_ompcon_copy_almost(tree);
    CopyPosInfo(t, tree);
    return (t);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                               *
 *     OMPi-EXTENSION NODES                                      *
 *                                                               *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

oxclause ast_oxclause_copy_almost(oxclause t) {
    if (t == NULL)
        return (NULL);
    switch (t->type) {
    case OX_OCATWORKER:
    case OX_OCATALL:
    case OX_OCTIED:
    case OX_OCUNTIED:
    case OX_OCDETACHED:
        return (OmpixPlainClause(t->type));
    case OX_OCIN:
    case OX_OCOUT:
    case OX_OCINOUT:
        return (OmpixVarlistClause(t->type, ast_decl_copy(t->u.varlist)));
    case OX_OCREDUCE:
        return (OmpixReductionClause(t->operator, ast_decl_copy(t->u.varlist)));
    case OX_OCATNODE:
        return (OmpixAtnodeClause(t->u.expr));
    case OX_OCLIST:
        return (OmpixClauseList(ast_oxclause_copy(t->u.list.next),
                                ast_oxclause_copy(t->u.list.elem)));
    case OX_OCSTART:
        return (OmpixStartClause(t->u.expr));
    case OX_OCSTRIDE:
        return (OmpixStrideClause(t->u.expr));
    case OX_OCSCOPE:
        return (OmpixScopeClause(t->u.value));
    }
    return (NULL);
}

oxdir ast_oxdir_copy_almost(oxdir t) {
    if (t == NULL)
        return (NULL);
    return (OmpixDirective(t->type, ast_oxclause_copy(t->clauses)));
}

oxcon ast_oxcon_copy_almost(oxcon t) {
    if (t == NULL)
        return (NULL);
    return (OmpixConstruct(t->type,
                           ast_oxdir_copy(t->directive),
                           ast_stmt_copy(t->body)));
}

oxclause ast_oxclause_copy(oxclause tree) {
    oxclause t = ast_oxclause_copy_almost(tree);
    CopyPosInfo(t, tree);
    return (t);
}

oxdir ast_oxdir_copy(oxdir tree) {
    oxdir t = ast_oxdir_copy_almost(tree);
    CopyPosInfo(t, tree);
    return (t);
}

oxcon ast_oxcon_copy(oxcon tree) {
    oxcon t = ast_oxcon_copy_almost(tree);
    CopyPosInfo(t, tree);
    return (t);
}
