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

/*
 * 2009/05/10
 *   fixed '#include <string.h>' bug
 */

/* picco.c -- the starting point */

#include "picco.h"
#include "ast.h"
#include "ast_free.h"
#include "ast_show.h"
#include "ast_xform.h"
#include "math.h"
#include "str.h"
#include "x_types.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// #include "config.h"
#include <gmp.h>
#ifdef _WIN32
    #include <direct.h> 
#else
    #include <sys/stat.h> 
#endif
#include <errno.h>

static aststmt ast; /* The AST we use as our original */
symtab stab;        /* Our symbol table */
int testingmode;    /* For internal testing only */
char *filename;     /* The file we parse */
char *output_filename;
char *var_list;
char *final_list;
/* This is for taking care of main() in the parsed code;
 * OMPi generates its own main() and replaces the original one.
 */
char *MAIN_NEWNAME = "main"; /* Rename the original main */
int hasMainfunc = 0;         /* 1 if main() function is defined in the file */
int needMemcpy = 0;          /* 1 if generated code includes memcpy()s */
int needMalloc = 0;          /* 1 if generated code includes malloc()s */
int mainfuncRettype = 0;     /* 0 = int, 1 = void */
int nonewmain = 0;           /* If 1, we won't output a new "main()" */
int processmode = 0;         /* If 1, turn on process mode */
int threadmode = 0;          /* Will become 1 by default */
int enableOpenMP = 1;        /* If 0, ignore OpenMP constructs */
int enableOmpix = 1;         /* Enable OMPi-extensions */
int enableCodeDup = 1;       /* Duplicate code where appropriate for speed */
int needLimits = 0;          /* 1 if need limits.h constants (min/max) */
int needFloat = 0;           /* 1 if need float.h constants (min/max) */
char *MAIN_RENAME = "__original_main";
int bits = 0;                /* Bits for modulus */
int peers = 0;               /* Number of computational parties */
int threshold = 0;           /* Secret sharing parameter */
int inputs = 0;              /* Number of input parties */
int outputs = 0;             /* Number of ouput parties */
char technique[100] = ""; /* The technique used either rss or shamir */
int total_threads = 0;
int nu;
int kappa_nu;
int technique_var = 0; // Default to 0 -> user should assign 1 or 2

void getPrime(mpz_t, int);

// Windows uses _mkdir() to create a directory and returns true if the operation succeeds.
// macOS uses mkdir() with default permissions and not setting 0777
// Other Unix-like uses mkdir() with permissions set to 0777, giving full read, write, and execute permissions.
int createDirectory(const char *path) {
#ifdef _WIN32
    return _mkdir(path);
#else
    return mkdir(path, 0777);
#endif
}

// Function that creates the path and directory needed for files needed 
void pathCreater(char *final_list) {
    char *path_separator = final_list;
    char *next_separator = strchr(path_separator, '/');
    
    // Loop through each segment of the path
    while (next_separator != NULL) {
        *next_separator = '\0'; // temporarily truncate the path
        if (createDirectory(final_list) < 0 && errno != EEXIST) {
            fprintf(stderr, "Failed to create directory: %s\n", final_list);
            exit(EXIT_FAILURE); // or handle error appropriately
        }
        *next_separator = '/'; // restore the path separator
        path_separator = next_separator + 1; // move to the next segment
        next_separator = strchr(path_separator, '/');
    }
}

// will have new argument for flag
// If mode is 1 -> -m
// If mode is 2 -> -d -> web-server
// If mode is 3 -> -t -> old-deployment before integrating web-server 
void append_new_main(int mode, char *global_priv_var, FILE *output_filename) { // This FILE was "char *output_filename" in the web branch and was changed to FILE, FILE was used in the master branch 

    total_threads = (num_threads == 0) ? 1 : num_threads;

    fprintf(output_filename, 
                    "\n\n/* smc-compiler generated main() */\n"
                    "int %s(int argc, char **argv) {\n",
                    MAIN_NEWNAME);

    // Conditionally include the lines based on the technique_var variable (shamir-2)
    char *res = "";
    if (technique_var == SHAMIR_SS) {
        mpz_t modulus2;
        mpz_init(modulus2);
        getPrime(modulus2, bits);
        res = mpz_get_str(NULL, 10, modulus2);
    } 

    
    // Check the input parameters
    // this will be different based on flag
    // If the value is set then set the mode to be called and use for append_new_main()
    // If mode is 1 -> -m
    // If mode is 2 -> -d -> web-server
    // If mode is 3 -> -t -> old-deployment before integrating web-server 
    if (mode == 1) { // -m - measurement mode
        fprintf(output_filename, // There should be EXACTLY 3 arguments passed for measurement mode
                        "\n\tif(argc != 3){\n"
                        " \tfprintf(stderr,\"Incorrect input parameters\\n\");\n"
                        "  fprintf(stderr,\"Usage: <id> <runtime-config> \\n\");\n"
                        "  exit(1);\n}\n");
        fprintf(output_filename, 
                        "\n__s = new SMC_Utils(atoi(argv[1]), argv[2], \"\", 0, 0, NULL, %d, %d, %d, \"%s\", seed_map, %d, %d);\n",
                        peers, threshold, bits, res, total_threads, mode);
    } else { // -t - testing and -d - deployment 
        fprintf(output_filename, // There should be AT LEAST 7 arguments passed
                        "\nif(argc < 8){\n"
                        "  fprintf(stderr,\"Incorrect input parameters\\n\");\n"
                        "  fprintf(stderr,\"Usage: <id> <runtime-config> <privatekey-filename> <number-of-input-parties> <number-of-output-parties> <input-share> <output>\\n\");\n"
                        "  exit(1);\n}\n");
        fprintf(output_filename, 
                        "\nstd::string IO_files[atoi(argv[4]) + atoi(argv[5])];\n"
                        "for(int i = 0; i < argc-6; i++)\n"
                        "   IO_files[i] = argv[6+i];\n"
                        "\n__s = new SMC_Utils(atoi(argv[1]), argv[2], argv[3], atoi(argv[4]), atoi(argv[5]), IO_files, %d, %d, %d, \"%s\", seed_map, %d, %d);\n",
                        peers, threshold, bits, res, total_threads, mode);
    }

    // The SMC_Utils gets the following:
    /* id */                  // A number,           index: 1
    /* runtime_config */      //"runtime-config",   index: 2
    /* privatekey_filename */ //"private_03.pem",   index: 3
    /* numOfInputPeers */     // A number,           index: 4
    /* numOfOutputPeers */    // A number,           index: 5
    /* IO_files */            //{"example_shares_#", "output_example"}, index: 6
    /* numOfPeers */          // Not specified in the command line arguments.
    /* threshold */           // Not specified in the command line arguments.
    /* bits */                // Not specified in the command line arguments.
    /* mod */                 // Not specified in the command line arguments.
    /* num_threads */         // Not specified in the command line arguments.

    // this will be different based on flag, certain arguments will be Null/0 in -m
    // The SMC_Utils gets the following:
    /* id */             // A number,           index: 1
    /* runtime_config */ //"runtime-config",   index: 2
    /* NULL */
    /* 0 */
    /* 0 */
    /* NULL */
    /* numOfPeers */  // Not specified in the command line arguments.
    /* threshold */   // Not specified in the command line arguments.
    /* bits */        // Not specified in the command line arguments.
    /* mod */         // Not specified in the command line arguments.
    /* num_threads */ // Not specified in the command line arguments.
    fprintf(output_filename, 
                    "\n struct timeval tv1;"
                    "\n struct timeval tv2;"
                    "\n int _xval = 0;\n\n"
                    " gettimeofday(&tv1,NULL);\n");
    
    // Private-global_variables will be printed after this line
    fprintf(output_filename, "\n\n%s\n\n", global_priv_var);

    if (num_threads > 0)
        fprintf(output_filename, 
                        "  ort_initialize(&argc, &argv);\n"
                        "  omp_set_num_threads(%d);\n",
                        total_threads);
    if (mainfuncRettype == 0)
        fprintf(output_filename, "  _xval = (int) %s(argc, argv);\n", MAIN_RENAME);
    else
        fprintf(output_filename, "%s(argc, argv);\n", MAIN_RENAME);
    fprintf(output_filename, 
                    "  gettimeofday(&tv2, NULL);\n"
                    "  std::cout << \"Time: \" << __s->time_diff(&tv1,&tv2) << \" seconds \"<< std::endl;\n");
    if (num_threads > 0)
        fprintf(output_filename, "  ort_finalize(_xval);\n");
    fprintf(output_filename, "  return (_xval);\n");
    fprintf(output_filename, "}\n\n");

}

int getopts(int argc, char *argv[]) {
    int i;

    for (i = 0; i < argc; i++) {
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
/*Helper Functions for laodConfig() used to check if a value
is a number and remove any extra digits from the input.

removeSpaces(value)
This function takes a string (value) and removes any non-digit characters, replacing them with a space.

isNumeric(value)
This function checks if a given string (value) contains only numeric characters.
*/

void removeSpaces(char *str) {
    int i = 0, j = 0; // i is used to iterate over the original string, j for placing non-space characters

    while (str[i]) {       // iterate until the end of the string
        if (str[i] != ' ') { // if the current character is not a space
            str[j++] = str[i]; // copy it to the jth position, then increment j
        }
        i++; // always increment i
    }
    str[j] = '\0'; // null-terminate the modified string
}

int isNumeric(const char *value) {
    for (const char *c = value; *c; ++c) {
        if (!isdigit(*c)) {
            return 0;
        }
    }
    return 1;
}

/*Extracts the data from the config file given during execution
The function below could handle multiple cases:
    1. Inputs could be in any order.
    2. Defaults the technique rss.
    3. Ignores extra spaces and tabs.
    4. Provides a hard stop (execution termination with an error message) if
        a. Any numeric field (bits, peers, threshold, inputs, outputs) contains something other than a single integer.
        b. The technique field contains something other than "shamir" or "rss".
        c. Peers is not an an odd integer.
        d. Peers is not equal to 2*threshold+1.
        e. If the same variable appears more than once or if a line is not of the form "var:value".
*/
void loadConfig(char *config) {

    FILE *fp;
    fp = fopen(config, "r");

    if (fp == NULL) {
        printf("Error: Configuration file '%s' does not exist. Please make sure it is created and the path is correct.\n", config);
        exit(EXIT_FAILURE); // Exit the program gracefully
    }

    char line[100];                // Buffer to store each line of the file.
    char encounteredKeys[6][100]; // Array to store encountered keys to check for duplicates.

    // Read each line from the file using fgets() function.
    while (fgets(line, sizeof(line), fp) != NULL) {
        char key[100], value[100];

        // Parse the line into key=value using sscanf function terminate with error if format is off.
        if (sscanf(line, " %99[^=]= %99[^\n]", key, value) == 2) {

            // Check for duplicated keys and terminate with an error if there is more than one.
            for (int i = 0; i < 6; ++i) {
                if (strcmp(key, encounteredKeys[i]) == 0) {
                    fprintf(stderr, "Error: Duplicate key '%s' found.\n", key);
                    exit(1);
                }
            }

            // Stores encountered keys to check for duplicates.
            for (int i = 0; i < 6; ++i) {
                if (encounteredKeys[i][0] == '\0') {
                    strncpy(encounteredKeys[i], key, sizeof(encounteredKeys[i]) - 1);
                    encounteredKeys[i][sizeof(encounteredKeys[i]) - 1] = '\0';
                    break;
                }
            }

            // Updates the configuration values based on the encountered keys.
            removeSpaces(value);
            if (strcmp(key, "bits") == 0) {
                removeSpaces(value);
                bits = atoi((value));
                if (bits == 0) {
                fprintf(
                    stderr,
                    "Error: 'bits' must be either empty or a non-zero integer.\n");
                exit(1);
                }
            } else if (strcmp(key, "peers") == 0) {
                if (strcmp(key, "peers") == 0 && !isNumeric(value)) {
                    fprintf(stderr, "Error: %s must be a single integer.\n", key);
                    exit(1);
                }
                peers = atoi((value));
            } else if (strcmp(key, "threshold") == 0) {
                if (strcmp(key, "threshold") == 0 && !isNumeric(value)) {
                    fprintf(stderr, "Error: %s must be a single integer.\n", key);
                    exit(1);
                }
                threshold = atoi((value));
            } else if (strcmp(key, "inputs") == 0) {
                if (strcmp(key, "inputs") == 0 && !isNumeric(value)) {
                    fprintf(stderr, "Error: %s must be a single integer.\n", key);
                    exit(1);
                }
                inputs = atoi((value));

            } else if (strcmp(key, "outputs") == 0) {
                if (strcmp(key, "outputs") == 0 && !isNumeric(value)) {
                    fprintf(stderr, "Error: %s must be a single integer.\n", key);
                    exit(1);
                }
                outputs = atoi((value));

            } else if (strcmp(key, "technique") == 0) {
                strncpy(technique, value, sizeof(technique) - 1);
                technique[sizeof(technique) - 1] = '\0';

                // Convert technique to lowercase to take care of the case if user inputs Rss or Shamir.
                for (char *c = technique; *c; ++c) {
                    *c = tolower(*c);
                }

                // Check the value of 'technique' and set 'technique_var' accordingly
                if (strcmp(technique, "rss") == 0) {
                    technique_var = REPLICATED_SS; // Set to RSS-1
                } else if (strcmp(technique, "shamir") == 0) {
                    technique_var = SHAMIR_SS; // Set to Shamir-2
                } else {
                    fprintf(stderr, "Error: Invalid value for technique. Use 'shamir' or 'rss'.\n");
                    exit(1);
                }
            }

        } else {
            // Check for = only if the line is not a key=value meaning if user inputs key:value or key_value or key-value
            if (strstr(line, "bits=") == NULL &&
                strstr(line, "peers=") == NULL &&
                strstr(line, "threshold=") == NULL &&
                strstr(line, "inputs=") == NULL &&
                strstr(line, "outputs=") == NULL &&
                strstr(line, "technique=") == NULL) {
                if (strstr(line, ":") != NULL || strstr(line, "-") != NULL || strstr(line, "_") != NULL) {
                    fprintf(stderr, "Error: Invalid Format! Use '=' between key and value!\n");
                    exit(1);
                }
            }
        }
    }

    fclose(fp);

    // Check if the required inputs have been given or not.
    if (inputs == 0) {
        fprintf(stderr, "Error: Inputs must be specified.\n");
        exit(1);
    } else if (peers == 0) {
        fprintf(stderr, "Error: Peers must be specified.\n");
        exit(1);
    } else if (threshold == 0) {
        fprintf(stderr, "Error: Threshold must be specified.\n");
        exit(1);
    } else if (outputs == 0) {
        fprintf(stderr, "Error: Outputs must be specified.\n");
        exit(1);
    } 


    // Check if 'peers' meet the requirments.
    if (peers % 2 == 0) {
        fprintf(stderr, "Error: Peers must be an odd integer equal to 2 * threshold + 1.\n");
        exit(1);
    }
    if (peers != 2 * threshold + 1) {
        fprintf(stderr, "Error: Peers must be equal to 2 * threshold + 1.\n");
        exit(1);
    }


    // Check if 'technique' meet the requirments.
    if (strlen(technique) == 0) {
        printf("Warning: No technique provided, defaulting to Shamir.\n");
        strcpy(technique, "shamir"); // Set technique to Shamir
        technique_var = SHAMIR_SS;   // Default to Shamir
    } else if (strcasecmp(technique, "shamir") != 0 && strcasecmp(technique, "rss") != 0) {
        fprintf(stderr, "Error: Invalid value for technique. Use 'shamir' or 'rss'.\n");
        exit(1);
    }


    // Validate 'peers' value based on 'technique' used
    if (strcasecmp(technique, "rss") == 0 && peers > 7) {
        fprintf(stderr, "Error: For 'rss' technique, peers must be less than or equal to 7.\n");
        exit(1);
    }
    if (strcasecmp(technique, "shamir") == 0 && peers > 20) {
        fprintf(stderr, "Error: For 'shamir' technique, peers must be less than or equal to 20.\n");
        exit(1);
    }
}

// In this code argc and argv will be increased by 1
int main(int argc, char *argv[]) {
    time_t now;
    char tmp[256];
    int r;
    aststmt p;

    /***************** read config file ********************/
    if (argc != 6) {
        fprintf(stderr, "Incorrect input parameters:\n");
        // will need to update with new flags (-m and -d for measurement and deployment)
        fprintf(stderr, "Usage: picco [-d | -m] <user program> <SMC config> <translated program> <utility config>\n"); // make sure other arguments are incremented by one
        exit(1);
    }

    // argv[0],           // Argument 0: the program name (./your_program)
    // argv[1],           // Argument 1: -d | -m
    // argv[2],           // Argument 1: id (the user-defined id)
    // argv[3],           // Argument 2: runtime-config-file
    // argv[4],           // Argument 3: privatekey-filename
    // argv[5],           // Argument 4: number-of-input-parties
    // argv[6],           // Argument 5: number-of-output-parties

    // Parse command line arguments - check arguments are formed properly
    // Check if the flag is either -m or -d -> if not program exits
    // If the value is set then set the mode to be called and use for append_new_main()
    // If mode is 1 -> -m
    // If mode is 2 -> -d -> web-server
    // If mode is 3 -> -t -> old-deployment before integrating web-server 
    int mode = 0;
    if (strcmp(argv[1], "-m") == 0) {
        mode = 1;
    } else if (strcmp(argv[1], "-d") == 0) {
        mode = 2;
    } else if (strcmp(argv[1], "-t") == 0) {
        mode = 3;
    } else {
        fprintf(stderr, "Invalid flag. Use either -m, -d, or -t.\n");
        exit(1);
    }

    loadConfig(argv[3]);
    nu = ceil(log2(nChoosek(peers, threshold))); //Catrina and de Hoogh, 2010, pg. 4
    kappa_nu = SECURITY_PARAMETER + nu;
    unsigned int map_size; // how many items are in seed_map, used to calculate buffer size
    unsigned int *seed_map = generateSeedMap(peers, threshold, &map_size);
    int map_tmp_size = sizeof(int) * 4 * map_size + 200;
    char map_tmp[map_tmp_size];
    sprintf(map_tmp, "std::vector<int> seed_map = {");
    // int pos = 0;
    char text[snprintf(NULL, 0, "%li", sizeof(int)) + 1];
    for (unsigned int i = 0; i < map_size; i++) {
        sprintf(text, "%i", seed_map[i]);
        strncat(map_tmp, text, strlen(text));
        if (i != (map_size - 1)) {
            strcat(map_tmp, ", ");
        }
    }
    strcat(map_tmp, "};");

    filename = argv[2];
    final_list = argv[5];
    var_list = "var_list";

    pathCreater(argv[4]);
    output_filename = (char *)malloc(sizeof(char) * strlen(argv[4]) + 5);
    sprintf(output_filename, "%s.cpp", argv[4]);

    if (!processmode)
        threadmode = 1; /* By default */

    stab = Symtab(); /* Create the symbol table */

    /* Take care of GCC */
    symtab_put(stab, Symbol("__builtin_va_list"), TYPENAME);
    symtab_put(stab, Symbol("__extension__"), TYPENAME);

    /* This is a reserved identifier in C99; it is actually supposed to be
     * "declared" at the top of each function; we simply insert it
     * @ global scope so it is visible everywhere.
     */
    symtab_put(stab, Symbol("__func__"), IDNAME);


    /*
     * 2. Parse & get the AST
     */
    ast = parse_file(filename, &r);

    if (technique_var == SHAMIR_SS) {
        if (bits == 0)
            bits = modulus + 1; // setting modulus to right above the computed bitlength
        else if (bits > 0 && bits < modulus)
            printf("WARNING: the modulus user provided is not large enough for correct computation.\nThe program is not expected to run correctly and produce correct results.\nThe minimum number of bits required to produce correct results is %i, \nbut the number of bits provided in %s is %i.\n",modulus,argv[3], bits);
        bits = fmax(bits, ceil(log(peers)) + 1);
    } else if (technique_var == REPLICATED_SS) {
        if (bits == 0)
            bits = modulus;
        else if (bits > 0 && bits < modulus)
            printf("WARNING: the modulus user provided is not large enough for correct computation.\nThe program is not expected to run correctly and produce correct results.\nThe minimum number of bits required to produce correct results is %i, \nbut the number of bits provided in %s is %i.\n",modulus,argv[3], bits);
    }

    if (r)
        return (r);
    if (ast == NULL) /* Cannot open file */
    {
        fprintf(stderr, "Error opening file %s for reading!\n", filename);
        return (30);
    }
    if ((!__has_omp || !enableOpenMP) && (!__has_ompix || !enableOmpix) && !hasMainfunc && !testingmode && !processmode)
        return (33); /* Leave it as is */

    /* 2.1 Append the smc-config contents into var_list */

    // Open the file using the provided path
    pathCreater(final_list);
    FILE *fp = fopen(final_list, "w+");
    if (fp == NULL) {
        fprintf(stderr, "Error opening file: %s\n", final_list);
        exit(1);
    }
    FILE *vfp = fopen(var_list, "r");
    char *line = (char *)malloc(sizeof(char) * 256);
    if (line == NULL) {
        fprintf(stderr, "Memory allocation failed for line buffer\n");
        exit(1);
    }


    // Conditionally include the lines based on the technique_var variable (shamir-2)
    char *res = "";
    if (technique_var == SHAMIR_SS) {
        mpz_t modulus2;
        mpz_init(modulus2);
        getPrime(modulus2, bits);
        res = mpz_get_str(NULL, 10, modulus2);
    }

    // Print technique_var as well as the other variables
    fprintf(fp, "%s:%d\n", "technique", technique_var);
    fprintf(fp, "%s:%d\n", "bits", bits);
    fprintf(fp, "%s:%d\n", "peers", peers);
    fprintf(fp, "%s:%d\n", "threshold", threshold);
    fprintf(fp, "%s:%d\n", "inputs", inputs);
    fprintf(fp, "%s:%d\n", "outputs", outputs);
    if (technique_var == SHAMIR_SS) { // Print modulus only if technique_var is shamir-2
        fprintf(fp, "%s:%s\n", "modulus", res);
    }

    while (fgets(line, sizeof(line), vfp) != NULL)
        fprintf(fp, "%s", line);

    fclose(fp);
    fclose(vfp);
    remove(var_list);
    free(line);

    /*
     * 3. Transform & output
     */

    // Add SMC specific include statements
    p = verbit(tmp);
    p = BlockList(p, verbit("#include \"smc-compute/SMC_Utils.h\""));
    p = BlockList(p, verbit("#include <gmp.h>"));
    p = BlockList(p, verbit("#include <omp.h>"));
    p = BlockList(p, verbit("\nSMC_Utils *__s;\n"));
    p = BlockList(p, Verbatim(strdup(map_tmp)));

    ast = BlockList(p, ast);

    // int knowMemcpy, knowSize_t, knowMalloc, includes_omph;

    /* The parser has left the symbol table at global scope; we must drain it */
    // includes_omph = (symtab_get(stab, Symbol("omp_lock_t"), TYPENAME) != NULL);
    // knowMemcpy = (symtab_get(stab, Symbol("memcpy"), FUNCNAME) != NULL);
    // knowSize_t = (symtab_get(stab, Symbol("size_t"), TYPENAME) != NULL);
    // knowMalloc = (symtab_get(stab, Symbol("malloc"), FUNCNAME) != NULL);
    // symtab_drain(stab);

    if (hasMainfunc && (enableOpenMP || testingmode || processmode)) {
        /* Need to declare the ort init/finalize functions */
        p = verbit(rtlib_onoff);
        assert(p != NULL);
        ast = BlockList(verbit("//# 1 \"ort.onoff.defs\""), BlockList(p, ast));
        if (num_threads > 0)
            ast = BlockList(verbit(rtlib_defs), ast);
    }
  
    ast->file = Symbol(filename);

    ast_parentize(ast); /* Parentize */
    ast_xform(&ast);    /* The transformation phase */


    time(&now); /* Advertise us */
    sprintf(tmp, "/* File generated from [%s] by PICCO %s */", filename, ctime(&now));

    p = verbit(tmp);
    // if (needLimits)
    p = BlockList(p, verbit("#include <limits.h>"));
    // if (needFloat)
    p = BlockList(p, verbit("#include <float.h>"));
    // if (needMemcpy && !knowMemcpy)
    p = BlockList(
        p,
        verbit("//extern void *memcpy(void*,const void*,unsigned int);"));
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

    // Update the Main function
    // If mode is 1 -> -m
    // If mode is 2 -> -d -> web-server
    // If mode is 3 -> -t -> old-deployment before integrating web-server 

    // This can be used to print the ast tree
    // str aa = Str("");
    // ast_stmt_print(aa, ast);
    // printf("\n\n");

    // This can be used to print the symbol table
    // scope_show(stab);
    // printf("\n\n");

    // Open the file that will include the generated code inside ast_show
    // Get the global_priv_variables declaration and call ast_show to print the original main
    char *global_priv_var = ast_show(ast, output_filename);
    
    // Open the file that will include the generated code to append the new main
    FILE *output_filename_FILE = fopen(output_filename, "a");

    // Print the new main and global_priv_var inside the output_filename_FILE 
    append_new_main(mode, global_priv_var, output_filename_FILE); // pass flag from earlier into here // mode is set from the argumet we parse

    if (testingmode) { /* Clean up (not needed actually; we do it only when testing)  */
        ast_free(ast);
        symtab_drain(stab);
        symbols_allfree();
        xt_free_retired();
    }
    free(output_filename);
    free(seed_map);
    return (0);
}

void exit_error(int exitvalue, char *format, ...) {
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
    exit(exitvalue);
}

void warning(char *format, ...) {
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
}

void getPrime(mpz_t result, int bits) {
    mpz_ui_pow_ui(result, 2, bits - 1);
    mpz_t m;
    mpz_init(m);
    int isPrime;
    do {
        mpz_nextprime(result, result);
        isPrime = mpz_probab_prime_p(result, 50);
        mpz_mod_ui(m, result, 4);
    } while (isPrime < 1 || mpz_cmp_si(m, 3) != 0);
    mpz_clear(m);
}

int nChoosek(int n, int k) {
    if (k > n)
        return 0;
    if (k * 2 > n)
        k = n - k;
    if (k == 0)
        return 1;

    int result = n;
    for (int i = 2; i <= k; ++i) {
        result *= (n - i + 1);
        result /= i;
    }
    return result;
}

// remember to de-allocate solutions wherever this function is called
unsigned int *generateSeedMap(unsigned int n, unsigned int t, unsigned int *num_solutions) {
    *num_solutions = nChoosek(n, t) / n;
    unsigned int *solutions = (unsigned int *)malloc(sizeof(unsigned int) * (*num_solutions)); //  array of size (nCt / n)
    unsigned int v = (1 << (n - t)) - 1;
    unsigned int upper_bound = 1 << n;
    unsigned int w, x, b1, b2, mask1, mask2, permutation;
    bool cond;
    int ctr = 0;
    while (v <= (upper_bound)) {
        // cyclic permute
        cond = true;
        for (unsigned int i = 0; i < n; i++) {
            mask2 = ((1 << (n - i)) - 1) << i;
            mask1 = ~mask2 & ((1 << n) - 1);
            b1 = v & mask1;
            b2 = v & mask2;
            permutation = (b1 << (n - i)) | (b2 >> i);
            if ((v > permutation)) {
                cond = false;
                break;
            }
        }
        if (cond) {
            solutions[ctr] = v;
            ctr++;
        }

        // generating next permutation
        x = v | (v - 1); // x gets v's least significant 0 bits set to 1
        // Next set to 1 the most significant bit to change,
        // set to 0 the least significant ones, and add the necessary 1 bits.
        w = (x + 1) | (((~x & -~x) - 1) >> (__builtin_ctz(v) + 1));
        // making sure we dont go past n
        if (w > upper_bound) {
            break;
        }
        v = w; // updating for next iteration
    }
    return solutions;
}
