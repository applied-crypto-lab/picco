all: picco	

picco: picco.o ast.o ast_show.o symtab.o parser.tab.o lex.yy.o str.o ast_vars.o ast_copy.o ast_print.o ast_free.o ast_types.o ast_xform.o x_types.o x_clauses.o x_parallel.o x_arith.o x_task.o x_single.o x_sections.o x_for.o x_thrpriv.o x_shglob.o ox_xform.o ast_renlabs.o  
	gcc -g3 -Wall -o picco picco.o parser.tab.o lex.yy.o ast.o ast_show.o symtab.o str.o ast_vars.o ast_copy.o ast_print.o ast_free.o ast_types.o ast_xform.o x_types.o x_clauses.o x_parallel.o x_arith.o x_task.o x_single.o x_sections.o x_for.o x_thrpriv.o x_shglob.o ox_xform.o ast_renlabs.o -lm -lgmp

picco.o: picco.c 
	gcc -g3 -Wall -c picco.c 

ast_xform.o: ast_xform.c
	gcc -g3 -Wall -c ast_xform.c

x_types.o: x_types.c
	gcc -g3 -Wall -c x_types.c

x_clauses.o: x_clauses.c
	gcc -g3 -Wall -c x_clauses.c

x_parallel.o: x_parallel.c 
	gcc -g3 -Wall -c x_parallel.c

x_arith.o: x_arith.c
	gcc -g3 -Wall -c x_arith.c

x_task.o: x_task.c 
	gcc -g3 -Wall -c x_task.c

x_single.o: x_single.c
	gcc -g3 -Wall -c x_single.c

x_sections.o: x_sections.c 
	gcc -g3 -Wall -c x_sections.c 

x_for.o: x_for.c 
	gcc -g3 -Wall -c x_for.c 

x_thrpriv.o: x_thrpriv.c
	gcc -g3 -Wall -c x_thrpriv.c

x_shglob.o: x_shglob.c
	gcc -g3 -Wall -c x_shglob.c 

ox_xform.o: ox_xform.c 
	gcc -g3 -Wall -c ox_xform.c

ast.o: ast.c 
	gcc -g3 -Wall -c ast.c

ast_show.o: ast_show.c
	gcc -g3 -Wall -c ast_show.c

symtab.o: symtab.c
	gcc -g3 -Wall -c symtab.c

ast_copy.o: ast_copy.c
	gcc -g3 -Wall -c ast_copy.c

ast_vars.o: ast_vars.c
	gcc -g3 -Wall -c ast_vars.c

ast_print.o: ast_print.c 
	gcc -g3 -Wall -c ast_print.c 

ast_free.o: ast_free.c
	gcc -g3 -Wall -c ast_free.c

ast_types.o: ast_types.c 
	gcc -g3 -Wall -c ast_types.c 

ast_renlabs.o: ast_renlabs.c
	gcc -g3 -Wall -c ast_renlabs.c

parser.tab.o: parser.y
	bison -d parser.y	
	gcc -g3 -Wall -c parser.tab.c -lm

lex.yy.o: scanner.l
	flex scanner.l	
	gcc -g3 -Wall -c lex.yy.c

str.o: str.c
	gcc -g3 -Wall -c str.c

clean: 
	rm -f parser.tab.c parser.tab.h parser.tab.o lex.yy.c lex.yy.o ast.o ast_show.o symtab.o picco.o str.o ast_vars.o ast_copy.o ast_print.o ast_free.o ast_types.o ast_xform.o	x_types.o  x_clauses.o x_parallel.o x_arith.o x_task.o x_single.o x_for.o x_thrpriv.o x_shglob.o x_sections.o ox_xform.o ast_renlabs.o picco 
