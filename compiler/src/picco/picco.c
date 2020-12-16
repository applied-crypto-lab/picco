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

/*
 * 2009/05/10
 *   fixed '#include <string.h>' bug 
 */
 
/* picco.c -- the starting point */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "picco.h"
#include "ast.h"
#include "ast_free.h"
#include "ast_xform.h"
#include "ast_show.h"
#include "x_types.h"
#include "str.h"
#include "math.h"
//#include "config.h"
#include <gmp.h>

static aststmt ast;          /* The AST we use as our original */
symtab         stab;         /* Our symbol table */
int            testingmode;  /* For internal testing only */
char           *filename;    /* The file we parse */
char 	       *output_filename; 
char	       *var_list;
char 	       *final_list;  
/* This is for taking care of main() in the parsed code;
 * OMPi generates its own main() and replaces the original one.
 */
char *MAIN_NEWNAME   = "main";  /* Rename the original main */
int  hasMainfunc     = 0;    /* 1 if main() function is defined in the file */
int  needMemcpy      = 0;    /* 1 if generated code includes memcpy()s */
int  needMalloc      = 0;    /* 1 if generated code includes malloc()s */
int  mainfuncRettype = 0;    /* 0 = int, 1 = void */
int  nonewmain       = 0;    /* If 1, we won't output a new "main()" */
int  processmode     = 0;    /* If 1, turn on process mode */
int  threadmode      = 0;    /* Will become 1 by default */
int  enableOpenMP    = 1;    /* If 0, ignore OpenMP constructs */
int  enableOmpix     = 1;    /* Enable OMPi-extensions */
int  enableCodeDup   = 1;    /* Duplicate code where appropriate for speed */
int  needLimits      = 0;    /* 1 if need limits.h constants (min/max) */
int  needFloat       = 0;    /* 1 if need float.h constants (min/max) */
char *MAIN_RENAME    = "__original_main";
int  bits 	     = 0;    /* Bits for modulus */
int  peers 	     = 0;    /* Number of computational parties */
int  threshold 	     = 0;    /* Secret sharing parameter */
int  inputs 	     = 0;    /* Number of input parties */
int  outputs 	     = 0;    /* Number of ouput parties */
int  total_threads   = 0; 

void append_new_main(){
	
	total_threads = (num_threads == 0) ? 1 : num_threads; 
	
	A_str_truncate();
	str_printf(strA(),
				"/* smc-compiler generated main() */\n"
				"int %s(int argc, char **argv)\n{\n",
				MAIN_NEWNAME);
	
	//Check the input parameters
	str_printf(strA(),
		"\n if(argc < 8){\n"
		"fprintf(stderr,\"Incorrect input parameters\\n\");\n"
		"fprintf(stderr,\"Usage: <id> <runtime-config> <privatekey-filename> <number-of-input-parties> <number-of-output-parties> <input-share> <output>\\n\");\n"
		"exit(1);\n}\n");
	mpz_t modulus2; 
	mpz_init(modulus2);
	getPrime(modulus2, bits);
	char* res = mpz_get_str(NULL, 10, modulus2);
	str_printf(strA(),
		"\n std::string IO_files[atoi(argv[4]) + atoi(argv[5])];\n"
		"for(int i = 0; i < argc-6; i++)\n"
		"   IO_files[i] = argv[6+i];\n"
		"\n__s = new SMC_Utils(atoi(argv[1]), argv[2], argv[3], atoi(argv[4]), atoi(argv[5]), IO_files, %d, %d, %d, \"%s\", %d);\n"
        "\nstruct timeval tv1;"
        "\nstruct timeval tv2;", peers, threshold, bits, res, total_threads);

     
	str_printf(strA(),
                     		"  int _xval = 0;\n\n"
                       		"  gettimeofday(&tv1,NULL);\n"); 
	if(num_threads > 0)
		str_printf(strA(), "  ort_initialize(&argc, &argv);\n"
		       		   "  omp_set_num_threads(%d);\n", total_threads);
	if(mainfuncRettype == 0) 
        	str_printf(strA(),  "  _xval = (int) %s(argc, argv);\n", MAIN_RENAME); 
	else
		str_printf(strA(), "%s(argc, argv);\n", MAIN_RENAME); 
	str_printf(strA(), 
                       	     	"  gettimeofday(&tv2, NULL);\n"
                       	     	"  std::cout << \"Time: \" << __s->time_diff(&tv1,&tv2) << std::endl;\n"); 
	if(num_threads > 0)
	       str_printf(strA(), "  ort_finalize(_xval);\n"); 
        str_printf(strA(), "  return (_xval);\n");
	str_printf(strA(), "}\n");
	
	ast = BlockList(ast, Verbatim(strdup(A_str_string())));
	ast->u.next->parent = ast;
	ast->body->parent = ast;
}


int getopts(int argc, char *argv[])
{
    int i;
    
    for (i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "--nomain") == 0)
            nonewmain = 1;
        else if (strcmp(argv[i], "--procs") == 0)
            processmode = 1;
        else if (strcmp(argv[i], "--threads") == 0)
            threadmode = 1;
        else if (strcmp(argv[i], "--nomp") == 0)
            enableOpenMP = 0;
        else if (strcmp(argv[i], "--nox") == 0)
            enableOmpix = 0;
        else if (strcmp(argv[i], "--nocodedup") == 0)
            enableCodeDup = 0;
        else
            return (1);
    }
    return (0);
}

#include "ort.defs"
void loadConfig(char *config)
{
	FILE *fp; 
	fp = fopen(config, "r"); 
	int line[10]; 
	fscanf(fp, "%[^:]:%d", line, &bits);
	fscanf(fp, "%[^:]:%d", line, &peers);
	fscanf(fp, "%[^:]:%d", line, &threshold);
	fscanf(fp, "%[^:]:%d", line, &inputs);
	fscanf(fp, "%[^:]:%d", line, &outputs);
	fclose(fp); 

}

int main(int argc, char *argv[])
{
    time_t  now;
    char    tmp[256];
    int     r, includes_omph;
    int     knowMemcpy, knowSize_t, knowMalloc; /* flag if def'ed in user code */
    aststmt p;
      
    /***************** read config file ********************/
    if(argc != 5){
        fprintf(stderr,"Incorrect input parameters:\n");
        fprintf(stderr,"Usage: picco <user program> <SMC config> <translated program> <utility config>\n");
        exit(1);
    }

    loadConfig(argv[2]); 
   
    /*
     * 1. Preparations
     */
    
    /*if (argc < 3)
    {
    OMPI_FAILURE:
        fprintf(stderr, "** %s should not be run directly; use %scc instead\n",
  ; 
	              argv[0], argv[0]);
        return (20);
    }
    if (strcmp(argv[2], "__ompi__") != 0)
    {
        if (strcmp(argv[2], "__intest__") == 0)
            testingmode = 1;
        else
            goto OMPI_FAILURE;
    }
    if (argc > 3 && getopts(argc-3, argv+3))
        goto OMPI_FAILURE;*/
    
    filename = argv[1];
    final_list = argv[4];
    var_list = "var_list"; 

    output_filename = (char*)malloc(sizeof(char) * strlen(argv[3])+5); 
    sprintf(output_filename, "%s.cpp", argv[3]);
    
    if (!processmode) threadmode = 1;  /* By default */
    
    stab = Symtab();                            /* Create the symbol table */
    
    /* Take care of GCC */
    symtab_put(stab, Symbol("__builtin_va_list"), TYPENAME);
    symtab_put(stab, Symbol("__extension__"), TYPENAME);
    
    /* This is a reserved identifier in C99; it is actually supposed to be
     * "declared" at the top of each function; we simply insert it
     * @ global scope so it is visible everywhere.
     */
    symtab_put(stab, Symbol("__func__"), IDNAME);
    
    time(&now);  /* Advertise us */
    sprintf(tmp, "/* File generated from [%s] by PICCO %s */", filename, ctime(&now));
    
    /*
     * 2. Parse & get the AST
     */
    ast = parse_file(filename, &r);
    if(bits == 0)
	bits = modulus+1; 
    else if(bits > 0 && bits < modulus)
	printf("WARNING: the modulus user provided is not large enough for correct computation\n");
    bits = fmax(bits, ceil(log(peers))+1);   
    if (r) return (r);
    if (ast == NULL)        /* Cannot open file */
    {
        fprintf(stderr, "Error opening file %s for reading!\n", filename);
        return (30);
    }
	if ((!__has_omp || !enableOpenMP) && (!__has_ompix || !enableOmpix)
        && !hasMainfunc && !testingmode && !processmode)
        return (33);          /* Leave it as is */
     
     /* 2.1 Append the smc-config contents into var_list */
      
	FILE *fp = fopen(final_list, "w+"); 
	FILE *vfp = fopen(var_list, "r"); 
	char* line = (char*)malloc(sizeof(char) * 256); 

	mpz_t modulus2; 
	mpz_init(modulus2);
	getPrime(modulus2, bits);
	//gmp_printf("%Zd\n", modulus2);
	char* res = mpz_get_str(NULL, 10, modulus2);

	fprintf(fp, "%s:%d\n", "bits", bits); 
	fprintf(fp, "%s:%s\n", "modulus", res); 
	fprintf(fp, "%s:%d\n", "peers", peers); 
	fprintf(fp, "%s:%d\n", "threshold", threshold); 
	fprintf(fp, "%s:%d\n", "inputs", inputs); 
	fprintf(fp, "%s:%d\n", "outputs", outputs); 
	
	while(fgets(line, sizeof(line), vfp) != NULL)
		fprintf(fp, line); 
		
	fclose(fp); 
	fclose(vfp); 
	remove(var_list);
	free(line); 
      
    /*
     * 3. Transform & output
     */

	//Add SMC specific include statements
	p = verbit(tmp);
	p = BlockList(p,verbit("#include \"smc-compute/SMC_Utils.h\""));
	p = BlockList(p,verbit("#include <gmp.h>"));
	p = BlockList(p,verbit("#include <omp.h>")); 
	p = BlockList(p,verbit("\nSMC_Utils *__s;\n"));

	ast = BlockList(p,ast);
    
    /* The parser has left the symbol table at global scope; we must drain it */
    includes_omph = (symtab_get(stab, Symbol("omp_lock_t"), TYPENAME) != NULL);
    knowMemcpy = (symtab_get(stab, Symbol("memcpy"), FUNCNAME) != NULL);
    knowSize_t = (symtab_get(stab, Symbol("size_t"), TYPENAME) != NULL);
    knowMalloc = (symtab_get(stab, Symbol("malloc"), FUNCNAME) != NULL);
    //symtab_drain(stab);
    
    if (hasMainfunc && (enableOpenMP || testingmode || processmode))
    {
        /* Need to declare the ort init/finalize functions */
       p = verbit(rtlib_onoff);
       assert(p != NULL);
       ast = BlockList(verbit("//# 1 \"ort.onoff.defs\""), BlockList(p, ast));
       if(num_threads > 0) 
       		ast = BlockList(verbit(rtlib_defs), ast); 
    }
    /*if ((__has_omp  && enableOpenMP) ||
        (__has_ompix && enableOmpix) || testingmode || processmode)
    {
        aststmt prepend = NULL;
        
        /* Runtime library definitions */
        /*if (__has_omp && enableOpenMP)
        {
            /* If <omp.h> was not included, then we must define a few things */
            /*if (includes_omph)
                p = NULL;
            else
            {
                p = verbit(
                    "typedef void *omp_nest_lock_t;"   /* The only stuff we needed */
                   // "typedef void *omp_lock_t; "       /* from <omp.h> */
                    //"typedef enum omp_sched_t { omp_sched_static = 1,"
                    //"omp_sched_dynamic = 2,omp_sched_guided = 3,omp_sched_auto = 4"
                    //" } omp_sched_t;"
                    //"int omp_in_parallel(void); "
                    //"int omp_get_thread_num(void); "
                    //"int omp_get_num_threads(void); "
                    //"int omp_in_final(void); "         /* 3.1 */
                /*);
                assert(p != NULL);
                p = BlockList(verbit("# 1 \"omp.mindefs\""), p);
            }
            
            /* Notice here that any types in omp.h will be defined *after* this */
           /* prepend = parse_and_declare_blocklist_string(rtlib_defs);
            assert(prepend != NULL);
            prepend = BlockList(verbit("# 1 \"ort.defs\""), prepend);
            if (p)
                prepend = BlockList(p, prepend);
            /*if (__has_affinitysched)
                prepend = BlockList(prepend,
                                    parse_and_declare_blocklist_string(
                                                                       "int ort_affine_iteration(int *);"
                                                                       ));*/
            
            /*ast = BlockList(prepend, ast);
        }
        
        ast = BlockList(verbit("# 1 \"%s\"", filename), ast);
        ast = BlockList(ast, verbit("\n"));    /* Dummy node @ bottom */
        ast->file = Symbol(filename);
        
        ast_parentize(ast);    /* Parentize */
        /*symtab_drain(stab);    /* Empty it; new globals will be traversed again */
	ast_xform(&ast);       /* The transformation phase */
        p = verbit(tmp); 
        //if (needLimits)
            p = BlockList(p, verbit("#include <limits.h>"));
        //if (needFloat)
            p = BlockList(p, verbit("#include <float.h>"));
        //if (needMemcpy && !knowMemcpy)
            p = BlockList(
                          p,
                          verbit("//extern void *memcpy(void*,const void*,unsigned int);")
                          );
        ast = BlockList(p, ast);
        /*
         ast = BlockList(
         verbit(tmp), 
         (needMemcpy && !knowMemcpy) ?
         BlockList(
         verbit("extern void *memcpy(void*,const void*,unsigned int);"), 
         ast
         ) : 
         ast
         );
         */
   // }

    //Update the Main function
    append_new_main();
    ast_show(ast, output_filename);
    if (testingmode)
    {          /* Clean up (not needed actually; we do it only when testing)  */
        ast_free(ast);
        symtab_drain(stab);
        symbols_allfree();
        xt_free_retired();
    }
    free(output_filename); 
    return (0);
}


void exit_error(int exitvalue, char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    exit(exitvalue);
}


void warning(char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
}

void getPrime(mpz_t result, int bits){
	mpz_ui_pow_ui(result,2,bits-1);
	mpz_t m;
	mpz_init(m);
	int isPrime;
	do{
		mpz_nextprime(result,result);
		isPrime = mpz_probab_prime_p(result,50);
		mpz_mod_ui(m,result,4);
	}while(isPrime < 1 || mpz_cmp_si(m, 3) != 0);
	mpz_clear(m); 
}
