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

/* x_for.c */

/*
 * 2011/03/17:
 *   ort_leaving_for() not needed for parallel for constructs.
 * 2010/12/21:
 *   bug fix to avoid wrong test for lastprivate iteration.
 * 2010/11/20:
 *   bug fixes in: runtime schedule code, a decreasing step case,
 *                 lastprivate code for collape()d loops
 * 2010/11/06:
 *   major changes; support for COLLAPSE().
 * 2009/05/11:
 *   added AUTO schedule type - implemented as STATIC for now.
 */


#include <string.h>
#include <assert.h>
#include "x_for.h"
#include "x_clauses.h"
#include "x_types.h"
#include "ast_xform.h"
#include "ast_free.h"
#include "ast_copy.h"
#include "ast_print.h"
#include "str.h"
#include "picco.h"


/* Analyze a FOR statement and determine conformance to OpenMP & other stuff 
 * that matter.
 */
static 
void analyze_omp_for(aststmt s, 
                     symbol *var, astexpr *lb, astexpr *b, astexpr *step,
                     int *condop, int *incrop)
{
  aststmt init;
  astexpr cond, incr, tmp;
  int     rel;
  
  assert( s != NULL && s->type == ITERATION && s->subtype == SFOR );
  init = s->u.iteration.init;
  cond = s->u.iteration.cond;
  incr = s->u.iteration.incr;
  if (init == NULL || cond == NULL || incr == NULL)
  {
    OMPFOR_ERROR:
      exit_error(1, "(%s, line %d) openmp error:\n\t"
                    "non-conformant FOR statement\n", s->file->name,s->l);
  }
  
  /* Get var and lb from the init part of the FOR 
   */
  if (init->type == EXPRESSION)     /* assignment: var = lb */
  {
    astexpr e = init->u.expr;

    if (e == NULL || e->type != ASS || e->left->type != IDENT)
      goto OMPFOR_ERROR;
    *var = e->left->u.sym;
    *lb  = e->right;
  }
  else 
    if (init->type == DECLARATION)  /* declaration: type var = lb */
    {
      astdecl d = init->u.declaration.decl;
      
      /* must check for integral type, too ... */
      
      if (d->type != DINIT)
        goto OMPFOR_ERROR;
      if (d->decl->type != DECLARATOR)
        goto OMPFOR_ERROR;
      if (d->decl->decl->type != DIDENT)
        goto OMPFOR_ERROR;
      *var = d->decl->decl->u.id;
      *lb  = d->u.expr;
    }
    else
      goto OMPFOR_ERROR;

  /* Get condition operator and ub from the cond part of the FOR 
   */
  if (cond->type != BOP) goto OMPFOR_ERROR;
  rel = cond->opid;
  if (rel != BOP_lt && rel != BOP_gt && rel != BOP_leq && rel != BOP_geq)
    goto OMPFOR_ERROR;
  /* OpenMP 3.0 allows swapping the left & right sides */
  if (cond->left->type != IDENT || cond->left->u.sym != *var)
  {
    tmp = cond->left;
    cond->left = cond->right;
    cond->right = tmp;
    rel = (rel == BOP_lt) ? BOP_gt : (rel == BOP_leq) ? BOP_geq :
          (rel == BOP_gt) ? BOP_lt : BOP_leq;
  }
  if (cond->left->type != IDENT || cond->left->u.sym != *var) /* sanity check */
    goto OMPFOR_ERROR;
  *condop = rel;
  *b = cond->right;
  
  /* Last part: get step and increment operator from the incr part of the FOR 
   */
  if (incr->type != PREOP && incr->type != POSTOP && incr->type != ASS)
    goto OMPFOR_ERROR;
  if (incr->left->type != IDENT || incr->left->u.sym != *var) /* sanity check */
    goto OMPFOR_ERROR;
  if (incr->type != ASS)
  {
    /* step is only needed for printing to a string; nothing else. Thus we can
       leave it as is and "create" it when printing to the string */
    *step = NULL;  /* signal special case of pre/postop */
    *incrop = (incr->opid == UOP_inc) ? BOP_add : BOP_sub;
  }
  else   /* ASS */
  {
    if (incr->opid != ASS_eq && incr->opid != ASS_add && incr->opid != ASS_sub)
      goto OMPFOR_ERROR;
    if (incr->opid != ASS_eq)
    {
      *incrop = (incr->opid == ASS_add) ? BOP_add : BOP_sub;
      *step   = incr->right;
    }
    else
    {
      tmp = incr->right;
      *incrop = tmp->opid;
      if (tmp->type != BOP || (*incrop != BOP_add && *incrop != BOP_sub))
        goto OMPFOR_ERROR;
      if (*incrop == BOP_sub)      /* var = var - incr */
      {
        if (tmp->left->type != IDENT || tmp->left->u.sym != *var)
          goto OMPFOR_ERROR;
        *step = tmp->right;
      }
      else                         /* var = var + incr / incr + var */
      {
        if (tmp->left->type != IDENT || tmp->left->u.sym != *var)
        {    /* var = incr + var */
          if (tmp->right->type != IDENT || tmp->right->u.sym != *var)
            goto OMPFOR_ERROR;
          *step = tmp->left;
        }
        else /* var = var + incr */
          *step = tmp->right;
      }
    }
  }
}


/* Possible clauses:
 * private, firstprivate, lastprivate, reduction, nowait, ordered, schedule,
 * collapse.
 *
 * All OpenMP V3.0 clauses recognized.
 */
void xform_for(aststmt *t)
{
  aststmt   s = (*t)->u.omp->body, parent = (*t)->parent, v,
            decls, inits = NULL, lasts = NULL, reds = NULL, stmp, 
            embdcls = NULL;
  astexpr   lb, ub, step, lbs[10], ubs[10], steps[10], expr;
  symbol    var, vars[10];
  int       incrop, condop;
  int       haslast, hasboth, hasred;
  int       schedtype = OC_static /* default */, 
            static_chunk = 0, i = 0, collapsenum = 1;
  int       ispfor = ( (*t)->u.omp->type == DCFOR_P );
  astexpr   schedchunk = NULL;    /* the chunksize expression */
  char      *chsize = NULL,       /* the chunksize value or variable */
            iters[10][128],
            plainstep,
            plainsteps[10];
  ompclause nw  = xc_ompcon_get_clause((*t)->u.omp, OCNOWAIT),
            sch = xc_ompcon_get_clause((*t)->u.omp, OCSCHEDULE),
            ord = xc_ompcon_get_clause((*t)->u.omp, OCORDERED),
            col = xc_ompcon_get_clause((*t)->u.omp, OCCOLLAPSE);
  int       needbarrier = (nw == NULL && 
                           xform_implicit_barrier_is_needed((*t)->u.omp) );
  symtab    dvars;
  stentry   varentry;
  

  v = ompdir_commented((*t)->u.omp->directive); /* Put directive in comments */

  /*
   * Preparations
   */

  if (sch)
  {
    if (sch->subtype == OC_auto)      /* Implement AUTO as STATIC for now */
      schedtype = OC_static;          /* but we will ignore any chunksize */
    else
      schedtype  = sch->subtype;      /* OC_static, OC_... */
    schedchunk = sch->u.expr;
    if (schedtype == OC_static && sch->subtype != OC_auto && schedchunk)
      static_chunk = 1;
    if (schedtype == OC_affinity && schedchunk)
      schedchunk = ast_expr_copy(schedchunk);
    /* Optimize: if schedchunk is a constant, don't use a variable for it */
    if (schedchunk && schedchunk->type == CONSTVAL)
      chsize = strdup( schedchunk->u.str );  /* memory leak */
  }

  if (col)
  {
    if ((collapsenum = col->subtype) >= 10)
      exit_error(1, "(%s, line %d) ompi error:\n\t"
        "cannot collapse more than 10 FOR loops.\n",
        (*t)->u.omp->directive->file->name,(*t)->u.omp->directive->l);
  }
  
     /* Collect all data clause vars - we need to check if any vars
      * are both firstprivate and lastprivate 
      */  
  dvars = xc_validate_store_dataclause_vars((*t)->u.omp->directive);

     /* Analyze the loop(s) */
  do
  {
    analyze_omp_for(s, &var, &lb, &ub, &step, &condop, &incrop);
    
     /* First check if the loop variable has been enlisted; if not,
      * it is automatically considered private (v25) - so we make it
      * appear as if there was a private(var) clause.
      */
    if ((varentry = symtab_get(dvars, var, IDNAME)) == NULL)
    {
      if (s->u.iteration.init->type == EXPRESSION)
        symtab_put(dvars, var, IDNAME)->ival = OCPRIVATE;
      else
	embdcls = (embdcls) ? 
	  BlockList(
	    embdcls,  
            Declaration( /* without the initializer */
              ast_spec_copy(s->u.iteration.init->u.declaration.spec),
              ast_decl_copy(s->u.iteration.init->u.declaration.decl->decl)
            )
          ) :
          Declaration(
            ast_spec_copy(s->u.iteration.init->u.declaration.spec),
            ast_decl_copy(s->u.iteration.init->u.declaration.decl->decl)
          );
    }
    else
    {
      if (s->u.iteration.init->type != EXPRESSION)  /* a declaration */
        exit_error(1, "(%s, line %d) openmp error:\n\t"
          "iteration variable '%s' is declared within the FOR statement\n\t"
          "and thus it cannot appear in the directive's data clauses.\n",
          (*t)->u.omp->directive->file->name,(*t)->u.omp->directive->l,var->name);
      /* Remove the FIRSTPRIVATE attribute if any (there is no use for it) */
      /* Actually, v25 (p.64,l.23) specifies that the iteration variable
       * can only appear in a PRIVATE or LASTPRIVATE clause, so we should
       * emit at least a warning.
       */
      if (varentry->ival==OCFIRSTPRIVATE || varentry->ival==OCFIRSTLASTPRIVATE)
        warning("(%s, line %d) warning:\n\t"
          "iteration variable '%s' cannot appear in a FIRSTPRIVATE clause..\n\t"
          "  .. let's pretend it was in a PRIVATE clause.\n",
          (*t)->u.omp->directive->file->name,(*t)->u.omp->directive->l,var->name);
      if (varentry->ival == OCFIRSTPRIVATE)
        varentry->ival = OCPRIVATE;
      else
        if (varentry->ival == OCFIRSTLASTPRIVATE)
          varentry->ival = OCLASTPRIVATE;
    }

    if (step == NULL || step->type == CONSTVAL)   /* ++/-- or += constant */
    {
      step = (step == NULL) ? numConstant(1) : ast_expr_copy(step);
      plainstep = 1;
    }
    else /* step != NULL && general expression for step */
    {
      step = UnaryOperator(UOP_paren, ast_expr_copy(step));  /* An expression */
      plainstep = 0;
    }
    if (incrop == BOP_sub)                        /* negative step */
      step = UnaryOperator(UOP_paren, UnaryOperator(UOP_neg, step));

    vars[i]  = var;
    lbs[i]   = UnaryOperator(UOP_paren, ast_expr_copy(lb));
    lbs[i]->flag = PUB; 
    ubs[i]   = (condop == BOP_leq || condop == BOP_geq) ?  /* correct ub */
                 BinaryOperator((condop == BOP_leq) ? BOP_add : BOP_sub,
		   UnaryOperator(UOP_paren, ast_expr_copy(ub)),
                   numConstant(1)) :
                 ast_expr_copy(ub);
    ubs[i]->flag = PUB;
    steps[i] = step;
    steps[i]->flag = PUB;
    sprintf(iters[i], "iters_%s_", var->name);
    plainsteps[i] = plainstep;

    if (i < collapsenum-1)
    {
      s = s->body;
      if (s != NULL && s->type == COMPOUND && s->body != NULL && 
	  s->body->type == ITERATION && s->body->subtype == SFOR)
	s = s->body;  /* { For } -> For */
      if (s == NULL || s->type != ITERATION || s->subtype != SFOR)
	exit_error(1, "(%s, line %d) openmp error:\n\t"
          "a collapse(%d) clause requires %d perfectly nested FOR loops.\n",
          (*t)->u.omp->directive->file->name,(*t)->u.omp->directive->l,
          collapsenum, collapsenum);
    }
    
  } while ((++i) < collapsenum);
  
     /* declarations from the collected vars (not the clauses!) */
  decls = xc_stored_vars_declarations(&haslast, &hasboth, &hasred);
     /* initialization statments for firstprivate non-scalar vars */
  if (decls)
    inits = xc_ompdir_fiparray_initializers((*t)->u.omp->directive);
     /* assignments for lastprivate vars */
  if (haslast)
    lasts = xc_ompdir_lastprivate_assignments((*t)->u.omp->directive);
  if (hasred)
    reds = xc_ompdir_reduction_code((*t)->u.omp->directive);
     /* we need a few more variables */
  stmp = Declaration(    /* declare: struct _ort_gdopt_ gdopt_; */
           SUdecl(SPEC_struct,Symbol("_ort_gdopt_"), NULL), 
           Declarator(NULL, IdentifierDecl(Symbol("gdopt_")))
         );
  if (embdcls)
    stmp = BlockList(embdcls, stmp);
  
  if (schedtype == OC_affinity)
  {
    if (haslast)
      stmp = BlockList(
               stmp, 
               Declaration(    /* declare: int niters_,iter_=1; */
                 Speclist_right(Declspec(SPEC_public, 0), Declspec(SPEC_int, 0)),
                 DeclList(
                   Declarator(NULL, IdentifierDecl(Symbol("niters_"))), 
                   InitDecl(
                     Declarator(NULL, IdentifierDecl(Symbol("iter_"))), 
                     numConstant(1)
                   )
                 )
               )
             );
  }
  else
  {
    stmp = BlockList(
             stmp,  /* Initialize because if a thread gets no iterations, the */
             Declaration(  /* lastprivate check for iter==niters may succeed! */
              Speclist_right(Declspec(SPEC_public, 0), Declspec(SPEC_int, 0)), /* int niters_=0,iter_=-1,fiter_,liter_=-2; */
               DeclList(          
                 DeclList(
                   DeclList(
                     InitDecl(
                       Declarator(NULL, IdentifierDecl(Symbol("niters_"))), 
                       numConstant(0)
                     ),
                     InitDecl(
                       Declarator(NULL, IdentifierDecl(Symbol("iter_"))), 
                       UnaryOperator(UOP_neg, numConstant(1))
                     )
                   ), 
                   Declarator(NULL, IdentifierDecl(Symbol("fiter_")))
                 ), 
                 InitDecl(
                   Declarator(NULL, IdentifierDecl(Symbol("liter_"))), 
                   UnaryOperator(UOP_neg, numConstant(2))
                 )
               )
             )
           );
    if (collapsenum > 1)        /* We need variables for # iterations */
    {
      stmp = BlockList(
               stmp,
                Declaration(Speclist_right(Declspec(SPEC_public, 0), Declspec(SPEC_int, 0)),
                    InitDecl(Declarator(NULL, IdentifierDecl(Symbol("pp_"))),
			     numConstant(1)
			   ))
             );
      for (i = 0; i < collapsenum; i++)
	stmp = BlockList(
	         stmp,
	         Declaration(Speclist_right(Declspec(SPEC_public, 0), Declspec(SPEC_int, 0)),
                   Declarator(NULL, IdentifierDecl(Symbol(iters[i]))))
               );
    }
    if (chsize == NULL)
    {
      chsize = "chunksize_";
      if (schedchunk || schedtype == OC_runtime )    /* expr for chunk size */
      {
        stmp = BlockList(
                 stmp,
                 Declaration(
                     Speclist_right(Declspec(SPEC_public, 0), Declspec(SPEC_int, 0)),                  (schedtype == OC_runtime) ?
                     Declarator(NULL, IdentifierDecl(Symbol(chsize))) :
                     InitDecl(
                       Declarator(NULL, IdentifierDecl(Symbol(chsize))),
                       ast_expr_copy(schedchunk)
                     )
                 )
               );
      }
    }
    /* we may need 4 more variables */
    if (static_chunk)
      stmp = BlockList(
               stmp,
               Declaration( /* declare: int chid_, TN_=omp_get_num_threads(); */
                Speclist_right(Declspec(SPEC_public, 0), Declspec(SPEC_int, 0)),                  DeclList(
                   Declarator(NULL, IdentifierDecl(Symbol("chid_"))), 
                   InitDecl(
                     Declarator(NULL, IdentifierDecl(Symbol("TN_"))), 
                     Call0_expr("omp_get_num_threads")
                   )
                 )
               )
             );
    /* we may need 2 more variables: get_chunk_ & staticextra_ */
    if (schedtype == OC_runtime)
    {
      aststmt xdc = Declaration(
                      Usertype(Symbol("chunky_t")), 
                      Declarator(NULL, IdentifierDecl(Symbol("get_chunk_")))
                    );
      /* Substitute type by hand since the produced code may not get xformed */
      xt_barebones_substitute(&(xdc->u.declaration.spec),
                              &(xdc->u.declaration.decl));
      xdc = BlockList(
              xdc,
              Declaration(
                Speclist_right(Declspec(SPEC_public, 0), Declspec(SPEC_int, 0)),
                  InitDecl(
                  Declarator(NULL, IdentifierDecl(Symbol("staticextra_"))), 
                  numConstant(-1)
                )
              )
            );
      stmp = BlockList(stmp, xdc);
    }
  }
  decls = (decls) ? BlockList(decls,stmp) : stmp;
  
  /*
   * Do the job
   */

  (*t)->u.omp->body = NULL;     /* Make it NULL so as to free it easily */
  ast_free(*t);                 /* Get rid of the OmpStmt */
  
  if (inits)
    decls = BlockList(decls, inits);  /* Append the initialization statements */

  /* Append our new code: niters_ = ...; ort_entering_for(...); */
  expr = numConstant(collapsenum);
 for (i = 0; i < collapsenum; i++){
     astexpr e1 = ast_expr_copy(ubs[i]);
     astexpr e2 = ast_expr_copy(lbs[i]);
     astexpr e3 = ast_expr_copy(steps[i]);
     
     e1->flag = ubs[i]->flag;
     e2->flag = lbs[i]->flag;
     e3->flag = steps[i]->flag;
     astexpr e =  BinaryOperator(BOP_sub, e1, e2);
     e->flag = PUB;     
     expr = CommaList(
            CommaList(
              CommaList(
                expr,
                CastedExpr(
                  Casttypename(Speclist_right(Declspec(SPEC_public, 0), Declspec(SPEC_long, 0)), NULL),
                  UnaryOperator(UOP_paren, e)
                )
              ),
              CastedExpr(
                Casttypename(Speclist_right(Declspec(SPEC_public, 0), Declspec(SPEC_long, 0)), NULL),
                e3)
            ),
            (collapsenum == 1) ?
              CastedExpr(           /* (void *) 0 */
                Casttypename(Speclist_right(Declspec(SPEC_public, 0), Declspec(SPEC_int, 0)),
			     AbstractDeclarator(Declspec(SPEC_star, 0), NULL)),
                numConstant(0)) :
              UnaryOperator(UOP_addr, Identifier(Symbol(iters[i])))
          );
  }

  stmp = BlockList(
           decls,
           Expression(      /* ort_entering_for(nw,ord,&gdopt_); */
             FunctionCall(
               Identifier(Symbol("ort_entering_for")), 
               CommaList(
                 CommaList(
                   numConstant(nw ? 1 : 0), 
                   numConstant(ord ? 1 : 0)
                 ), 
                 UnaryOperator(UOP_addr, Identifier(Symbol("gdopt_")))
               )
             )
           )
         );
    aststmt tt = Expression(     /* niters_ = ort_num_iters(...) */
                           Assignment(Identifier(Symbol("niters_")), ASS_eq,
                                      FunctionCall(Identifier(Symbol("ort_num_iters")), expr)
                                      )
                           );
 
  stmp = BlockList(
           stmp,
           Expression(     /* niters_ = ort_num_iters(...) */
             Assignment(Identifier(Symbol("niters_")), ASS_eq,  
               FunctionCall(Identifier(Symbol("ort_num_iters")), expr)
             )
           )
         );
  if (hasboth)   /* a var is both fip & lap; this needs a barrier here :-( */
    stmp = BlockList(stmp, BarrierCall());

    /* Prepare the main loop */
  if (schedtype == OC_affinity)
  {
      /* same loop, new body */
    if (lasts || ord)
    {
      if (lasts)
        s->body = BlockList(   /* iter++ */
                    Expression(PostOperator(Identifier(Symbol("iter")),UOP_inc)),
                    s->body
                  );
      if (ord)
        s->body = BlockList(
                    Expression( /* ort_thischunk_range(iter_-1); */
                      FunctionCall(
                        Identifier(Symbol("ort_thischunk_range")), 
                        CommaList(BinaryOperator(BOP_sub, 
                                    Identifier(Symbol("iter_")), 
                                    numConstant(1)),
                                  Identifier(Symbol("iter_"))
                        )
                      )
                    ),
                    s->body
                  );
       s->body = Compound(s->body);
    }
    s->body = If(
                FunctionCall(
                  Identifier(Symbol("ort_affine_iteration")),schedchunk
                ),
                s->body,
                NULL
              );
  }
  else
  {  
    aststmt idx = NULL;
   
    if (collapsenum > 1)
    {
      idx = Expression(Assignment(Identifier(Symbol("pp_")), ASS_eq, 
                                  numConstant(1)));
      for (i = collapsenum-1; i >= 0; i--)
      {
        idx = BlockList(
                idx,
                Expression(
                  Assignment(Identifier(vars[i]), ASS_eq, 
                    BinaryOperator(BOP_add, 
		      ast_expr_copy(lbs[i]),
                      BinaryOperator(BOP_mul, 
                        ast_expr_copy(steps[i]), 
                        UnaryOperator(UOP_paren,
  		          BinaryOperator(BOP_mod, 
  		            UnaryOperator(UOP_paren, 
		              BinaryOperator(BOP_div, 
			        Identifier(Symbol("iter_")), 
			        Identifier(Symbol("pp_"))
			      )
                            ),
                            Identifier(Symbol(iters[i]))
                          )
                        )
                      )
                    )
                  )
                )
              );
        if (i != 0)
          idx = BlockList(
                  idx,
                  Expression(Assignment(Identifier(Symbol("pp_")), ASS_mul, 
		  		        Identifier(Symbol(iters[i])))
                  )
                );
      }
    }
      /* Loop becomes:
       *   for (iter = fiter; iter < liter; iter++) {
       *     <var> = lb + iter*step
       *     <body>
       *   }
       * optimized as:
       *   for (iter = fiter, var = lb; iter < liter; iter++, var += step) {
       *     <body>
       *   }
       */
      
      astexpr e1 = ast_expr_copy(step);
      astexpr e2 = Identifier(Symbol("fiter_"));
      e1->flag = PUB;
      e2->flag = PUB;
      astexpr e3 = BinaryOperator(BOP_mul, e1, e2);
      e3->flag = PUB;
      astexpr e4 = ast_expr_copy(lb);
      e4->flag = PUB;
      astexpr e5 = Identifier(Symbol("iter_"));
      astexpr e6 = Identifier(Symbol("liter_"));
      e5->flag = PUB;
      e6->flag = PUB;
      astexpr e7 = PostOperator(Identifier(Symbol("iter_")), UOP_inc);
      e7->flag = PUB; 
    s = For(Expression((collapsenum > 1) ?
              Assignment(Identifier(Symbol("iter_")),
                         ASS_eq,
                         Identifier(Symbol("fiter_"))) 
                :
              CommaList(
                Assignment(Identifier(Symbol("iter_")),
                           ASS_eq,
                           Identifier(Symbol("fiter_"))),
	        Assignment(Identifier(var),
			   ASS_eq,
			   BinaryOperator(BOP_add, e4, e3)
                )
              )
            ),
            BinaryOperator(BOP_lt, e5, e6),
	    ((collapsenum > 1) ? e7 :
               CommaList(e7,
	 	 Assignment(Identifier(var), ASS_add, 
			    ast_expr_copy(step))
               )
            ),
	    (collapsenum > 1) ? Compound(BlockList(idx, s->body)) : s->body
        );
            
        /* The ordered_begin() needs to know the current chunk's start
         * (from_) in order to work. However, "from_" may be unavailable
         * as a variable if the ORDERED directive is orphaned. Thus,
         * we have to store is somewhere just in case.
         */
    if (ord)
      s = BlockList(
            Expression( /* ort_thischunk_range(fiter_); */
              FunctionCall(Identifier(Symbol("ort_thischunk_range")), 
                           CommaList(
                             Identifier(Symbol("fiter_")),
                             Identifier(Symbol("liter_"))
                           ))),
            s
          );
  }
    
  /* Schedule-dependent code */
  switch (schedtype)
  {
    case OC_static:
     if (static_chunk)
        stmp = BlockList(
                 stmp,
                  BlockList(
                   verbit("for(chid_ = omp_get_thread_num(); ;chid_ += TN_)"),
                   Compound(
                     BlockList(
                       verbit(
                         "\t fiter_ = chid_*(%s);\n"
                         "\t if (fiter_ >= niters_) break;\n"
                         "\t liter_ = fiter_ + (%s);\n"
                         "\t if (liter_ > niters_) liter_ = niters_;",
                         chsize, chsize
                       ),
                       s
                     )
                   )
                 )
               );
      else
        stmp = BlockList(
                 stmp,
                 BlockList(
                   verbit("if (ort_get_static_default_chunk(niters_, &fiter_, &liter_))"),
                   Compound(s)
                 )
               );
      break;             

    case OC_guided:
    case OC_dynamic:
      stmp = BlockList(
               stmp,
               BlockList(
                 verbit("while(ort_get_%s_chunk(niters_,%s,&fiter_,&liter_,(int*)0,&gdopt_))",
                   schedtype == OC_guided ? "guided" : "dynamic", 
                   schedchunk ? chsize : "1"),
                 Compound(s)
               )
             );
      break;

    case OC_runtime:
      stmp = BlockList(
               BlockList(
                 stmp,
                 /* ort_get_runtime_schedule_stuff(&get_chunk_, &chunksize_); */
                 Expression(
                   FunctionCall( 
                     Identifier(Symbol("ort_get_runtime_schedule_stuff")), 
                     CommaList(
                       UnaryOperator(UOP_addr,Identifier(Symbol("get_chunk_"))),
                       UnaryOperator(UOP_addr,Identifier(Symbol("chunksize_")))
                     )
                   )
                 )
               ),
               BlockList(
                 verbit("while((*get_chunk_)(niters_, chunksize_, &fiter_, &liter_, &staticextra_, &gdopt_))"),
                 Compound(s)
               )
             );
      break;

    case OC_affinity:
      stmp = BlockList(stmp, s);      
      break;
  }

  *t = BlockList(v, stmp);
  if (!ispfor)
    *t = BlockList(*t, Call0_stmt("ort_leaving_for"));
  if (lasts)
  {
    if (collapsenum > 1)
    {
      aststmt idx;     /* Need to set explicitly the correct index values! */
   
      idx = Expression(
              Assignment(Identifier(vars[0]), ASS_add, ast_expr_copy(steps[0]))
            );
      for (i = 1; i < collapsenum; i++)
        idx = BlockList(
                idx,
                Expression(
                 Assignment(Identifier(vars[i]),ASS_add,ast_expr_copy(steps[i]))
                )
              );
      lasts = BlockList(idx, lasts);
    }

    *t = BlockList(
           *t,
           If(
             BinaryOperator(BOP_eqeq, 
               Identifier(Symbol("iter_")),
               Identifier(Symbol("niters_"))
             ),
             lasts->type == STATEMENTLIST ?  Compound(lasts) : lasts,
             NULL
           )
         );
  }
  if (reds)
    *t = BlockList(*t, reds);
  if (needbarrier)
    *t = BlockList(*t, BarrierCall());
  
  *t = Compound(*t);
  ast_stmt_parent(parent, *t);
  
  /* Must free the array 
   */
}
