# PICCO: A General-Purpose Compiler for Private Distributed Computation

PICCO is a suite of programs for compiling general-purpose programs into their secure implementations, and running it in a distributed setting. The motivation and design of PICCO can be found in our [2013 CCS paper](https://doi.org/10.1145/2508859.2516752). 
The specification for dynamic memory allocation, pointers to private data, and secure data structures can be found in our [2017 TOPS article](https://dl.acm.org/doi/abs/10.1145/3154600).
This work can be cited as follows:

```
@inproceedings{picco,
  author = "Zhang, Yihua and Steele, Aaron and Blanton, Marina",
  year = 2013,
  title = "{PICCO}: A general-purpose compiler for private distributed computation",
  booktitle = "ACM Conference on Computer and Communications Security (CCS)",
  pages = "813--826"
}
```


The source code of PICCO mainly consists of two directories: `compiler` and `compute`. The `compiler` directory contains the source code of the PICCO compiler whose functionality is to translate a user's program into its secure implementation as well as supplementary programs. The `compute` directory contains the source code of the computational framework and protocol library that will be used to securely execute the user's translated program in a distributed setting.

## Source Code Dependencies

To compile or run user programs using PICCO code, a machine should have the following libraries installed:

- [GCC](https://gcc.gnu.org/) or [LLVM/clang](https://clang.llvm.org/) (clang generally performs better than GCC and is recommended)
- [GMP](https://gmplib.org/)
- [OpenSSL](https://www.openssl.org/source/) (v1.1.1 or newer)
- [Flex](https://github.com/westes/flex.git) -- fast lexical analyzer generator
- [GNU Bison parser](https://www.gnu.org/software/bison/)
- [CMake](https://cmake.org/)
- [Flask](https://flask.palletsprojects.com/) if the computation is performed in the deployment mode using a web server for data entry

Additionally, if you wish to compile and run multithreaded user programs, the following library is required:

- [OMPi Compiler](https://paragroup.cse.uoi.gr/wpsite/software/ompi/) (v1.2.3)


## 1. Compilation of PICCO

The first step is to compile the compiler and supplementary programs. To do so, one needs to go to the directory `compiler/` and run the command
```
./compile.sh
```  
This produces three executable files:
1. the PICCO compiler itself `picco`,
2. the utility program `picco-utility` that performs input/output share generation and reconstruction, and
3. the utility program `picco-web` that sets up and also starts a web server for collecting private inputs.

After the compilation, the programs can be found in the directory `compiler/bin/` (and subsequently can be placed elsewhere). The `picco` executable is the compiler for transforming a user's program written in an extension of C into its secure implementation, while `picco-utility` and `picco-web` are used to facilitate execution of the translated programs.

Once the compiler is ready, it can now be used to compile user programs into corresponding secure computation protocols. Note that because PICCO is a source-to-source compiler, this involves two steps:
1. transform a high-level user program into a program that implements the corresponding secure computation protocol and
2. compile the translated program to machine code using a native compiler. 

## 2. Translation of User Programs

User programs can be compiled and executed using one of the three supported modes:
1. The **measurement mode** (specified using the flag `-m`) requires the least amount of setup and is intended for measuring the runtime when benchmarking specific computations. It makes security shortcuts (such as foregoing public key infrastructure for secure key establishment) and doesn't require private inputs to be entered into the program. Instead, private inputs are randomly generated before the computation starts. The goal of this mode is to facilitate accurate performance measurement while minimizing the setup effort. **Note that at this time the measurement mode generates only random private inputs and thus does not handle public inputs.**
2. The **testing mode** (specified using the flag `-t`) permits proper execution of secure protocols on private data. It uses public keys owned by computational parties to set up secure communication channels and processed private inputs contributed by different data owners (input parties). It uses terminal-based interfaces for execution, data entry, and output assembly and is well suited for evaluating programs and their correctness.
3. The **deployment mode** (specified using the flag `-d`) also permits proper execution of secure protocols on private data. It differs from the testing mode in how private inputs are entered into the computation. Instead of specifying inputs in text files and creating shares using terminal-based `picco-utility` executable, this mode produces a web server setup that allows different input parties to enter their information and properly create encrypted shares prior to storing them at the web server. 

Regardless of the mode, it is necessary to create an **SMC config** file that provides computation parameters to the compiler.

**SMC config.** The SMC config is a text file that consists of 6 lines, with each of them being in the format of `param=value`, where `param` indicates an SMC parameter and `value` indicates its value. The following parameters are expected:

1. `technique` - the secret sharing technique to be used. Currently, we only support the value `shamir` for Shamir secret sharing.
2. `bits` - the bitlength of the modulus used with the secret sharing scheme. This parameter is optional. When the value is left blank in the config file, the compiler will determine the optimal bitlength based on the program to guarantee correctness. Otherwise, the programmer has the ability to overwrite the bitlength.
3. `peers` - the number of computational parties $n$.
4. `threshold` - the threshold value $t$, which is the maximum number of corrupted/colluding parties. The current implementation supports only honest majority techniques and expects $n = 2 t + 1$.
5. `inputs` - the number of parties contributing input to the computation. 
6. `outputs` - the number of parties receiving output from the computation.

The parameters can be placed in any order in the config file. 
The `inputs` and `outputs` parameters allow a user to run a program with inputs distributed multiple parties and produce multiple outputs with each of them being sent to a distinct output party. It will be assumed that input/output computational parties are numbered sequentially from 1 up until the specified number of parties. For example, if the number of inputs parties is $k$, they are expected to be numbered 1 through $k$. The same entity can take on different roles (e.g., input party 1 can also be output party 2).

**Program compilation.** To compile a user's program into its secure implementation, one needs to execute the following command (assuming the `picco` executable is present in the current directory):

```
./picco -d|-t|-m <user program> <SMC config> <translated program> <utility config>
``` 

The arguments are as follows:
1. the execution mode (`-d` for deployment, `-t` for testing or `-m` for measurement)
2. `<user program>` is the name of the file corresponding to the user program to be translated;
3. `<SMC config>` is the name of the config config with the content described above;
4. `<translated program>` is the name of a file (with an extension) that will store user's translated program (as a C++ program); 
5. `<utility config>` is the name of a file that the compiler generates for storing additional information about the program which will be needed at other stages.

The executable takes two files as its input and produces two files as its output.


## 3. (Native) Compilation of User Programs

The next step is to compile the translated user program using a native C++ compiler to an executable. Note that this compilation step needs to be repeated for each computational node if their hardware or computational environment differ.

<!--In order to run a user's translated program in a distributed setting, one needs to compile it using a native C++ compiler to produce a binary executable file, create a runtime config file, and send the executable to each computational party together with the runtime config and a file that stores input shares for that party. These steps are discussed below.-->

To compile the translated program, the program should be placed in the `compute/` directory (as it needs library functions stored in the directory `compute/smc-compute/`). Then execute the following command from the `compute/` directory (that produces a binary executable of the translated program):

```
./compile-usr-prog.sh -d|-t|-m <user program> <utility config>
```
where `-d`, `-t` and `-m` are the compilation mode flags, `<user program>` is the name of the translated program file (without the `.cpp` extension) generated earlier, and `<utility config>` is the name of the utility file generated during translation. The script produces an executable named `<user program>` stored in the `compute/build/` subdirectory (which subsequently can be moved). 

<!--
Based on the computational mode 
([measurement mode](#measurement-mode-execution), [testing mode](#testing-mode-setup-and-execution) and [deployment mode](#deployment-mode-setup-and-execution)), follow to the links to their respective sections.
-->

## 4. Measurement Mode Execution

Executing a secure program requires another configuration file, **runtime config**, that specifies how to connect to the nodes running the computation. A runtime config is a text file that consists of $n$ lines, where $n$ is the number of computational parties running the computation (that must match the number of peers in smc config). For the measurement mode, each line contains the following three values separated by commas: 

1. an ID of a computational party between 1 and $n$;
2. an IP address or a domain name of the computational party; and
3. a port number for connecting to that party.

The values should be listed in the specified order on each line and a line with each ID between 1 and $n$ should be present. The same runtime config file is then distributed to all computational parties. 

To start secure computation in measurement mode, each computational party executes the following command (assuming the program resides in the current directory):

```
./<user program> <ID> <runtime config> 
```
where the arguments to the executable `<user program>` are the ID of the computational party `<ID>` between 1 and $n$ and the name of the runtime config file `<runtime config>`. 

## 5. Testing and Deployment Mode Setup and Execution

### 5.1. Public-Private Key Pair Generation

All secure protocols produced by PICCO use pair-wise secure channels protected using symmetric key cryptography. While the measurement mode makes shortcuts during key establishment,  in the testing and deployment modes the parties utilize public key cryptography during that phase. Each computational party must have a public-private key pair and the public key be reliably distributed to other computation participants.

In the current implementation, only RSA keys are supported. Public and private keys are stored in a file and their format needs to be compatible with what OpenSSL uses. The following example commands can be used to generate a public-private key pair for party `ID`:

```
openssl genrsa -out private_ID.pem 2048
openssl rsa -in private_ID.pem -outform PEM -pubout -out public_ID.pem
```
Once a computational party creates its key pair, the party needs to distribute the public key to all computational nodes and can use the key for any number of programs and their executions.

### 5.2. Testing Mode Input Entry

Prior to performing secure computation, the input parties must prepare input data (that could be private, public, or both) and distribute them to the computational parties. Assuming that at least one of the inputs is private, an input party needs to call the program `picco-utility` to produce shares of private inputs. The same program is also used to assemble the output of upon completion of secure computation, as described later. In what follows, we first describe the usage of built-in I/O functions within user programs, then the format of files that contains plaintext input of an input party, followed by usage of the utility program for input generation.


- **Built-in I/O functions.**
Input and output in user programs is handled through built-in I/O functions `smcinput` and `smcoutput`, respectively. Both have the same interface and take as the first argument the name of a variable and as the second argument the ID of an input (output) party from whom the variable will be read (to whom the variable will be written). If the variable is an array, the sizes of each array dimension need to be specified as additional arguments (i.e., the number of arguments depends on the variable type). For instance, if `x` was declared as a two-dimensional array with both dimensions being 10, the program can specify `smcinput(x,1,10,10)` to read shares of 100 elements from input party 1. Note that if an array variable was declared to have a larger number of elements than the number of elements being read from the input, the read values will always be placed in the beginning of the array (or in the beginning of each row for the specified number of rows for two-dimensional arrays).

  In the current implementation, both `smcinput` and `smcoutput` are not allowed to be placed within loops or iterations. That is, a user should not write code such as:

  ```
  for(i = 0; i < 10; i++)
      for(j = 0; j < 10; j++)
          smcinput(x[i][j], 1); 
  ```

  The reason is that the parser extracts each call to `smcinput`/`smcoutput` by a scan without performing a more complex analysis or trial execution of the program. Thus, the code above will result in a single integer variable (i.e., `x[i][j]`) being read from the input instead of the entire 100-element array.
  
- **Input file format.** Each input party needs to prepare their input data in a text file as described below. This text file will be used by the utility program. Note that the user program may read input from multiple parties, and each input party prepares their data independently of other inputs parties.

  - *Order of input data*: A user's program may read more than one variable from an input party, and the order of variables in an input file for that party needs to be the same as their relative order in the user program. For instance, three I/O statements `smcinput(x,1,2)`, `smcinput(y,2,10)`, and `smcinput(z,1,10,10)` appear in a user program in that order, the value of `x` should precede that of `z` in the input file of party 1.

  - *Data format*: If a variable is a single integer/real or a one-dimensional array, its value should be listed on a separate single line as `var = value1,value2,...` for both public and private variables. For instance, in the above example, we will have `x = 1,2` when `x` is a private array containing two elements `1` and `2`. If a variable is a two-dimensional array of integers, each row in a matrix (if we think of a two-dimensional array as a matrix) should be listed on a separate line with the name of the variable repeated on each line. 

  - *Floating point data*: If a float variable is public, it will be treated in the same manner as integer variables mentioned earlier. However, if a variable is private, each float element will occupy a separate line in the format of `var = v,p,s,z`, where `v` is an ${\ell}$-bit significand, `p` is a k-bit exponent, and `s` and `z` are sign and zero bits, respectively (for more information, please refer to [Aliasgari et al., 2013](https://www.ndss-symposium.org/wp-content/uploads/2017/09/11_4_0.pdf)). Namely, if a private float variable is type of array, each array element will occupy a separate line in the above format e.g., in `smcinput(z,1,10,10)` with a private float array `z`, its array elements will occupy 100 lines in an input file.  


- **Generating input shares**. After an input party generates input data in the specified format and saves it in a file of the user's choice, the utility program can be invoked as follows:  
    
  ```
  ./picco-utility -I <input party ID> <input filename> <utility config> <shares filename>
  ```

  The utility program `picco-utility` takes a flag and four other arguments, which are:

  1. a flag that tells the utility program to either generate inputs (`-I`) or assemble outputs (`-O`);
  2. the ID of the input party;
  3. the name of the input file prepared by the input party in the format described above (storing values for both public and private variables input into the computation);
  4. the name of the file produced during program translation;
  5. a prefix of output files in which generated input shares will be stored.


  The utility program will read the input data and utility config and produce the same number of output files with shares as the number of computational parties $N$. The program's output will be stored in files named `<shares filename>ID`, where ID is the identification number for each computational party between 1 and $N$. The values of public variables are copied unmodified from the input file into the shares files, while the values of private variables are secret shared, with each computational party obtaining a single share stored in the corresponding shares file.


### 5.3. Deployment Mode Web Server Setup and Input Entry

The deployment mode allows users to submit data through a web server for secure computation. When accessing the server, you will be prompted to authenticate with a unique passcode before submitting your data. Data can be provided either directly through the web form or by uploading a file, depending on the input type.

#### Data Submission and Format:
  1. **Single Values (integer or real number)** - Enter directly into the text box.
  2. **1D and 2D Arrays** - Upload a CSV file where the data is listed in table format:
    
- **1D Array**: Each element must be placed on a new row. Example (`array = [1, 2, 3]`):
  ```
    1
    2
    3
  ```

- **2D Array**: Each row of the array corresponds to a row in the CSV, and each column is separated by a comma. Example (`array = [[1, 2], [3, 4], [5, 6]]`):
  ```
    1,2
    3,4
    5,6
  ```

#### Setting up the Web Server (for Developers)
For setting up the web server, the `picco-web` program is used.  
This requires three main setup steps:

  ##### 1. Generating the Server Data (**-G mode**)
  Before running the server, you must generate the required configuration and key files:

  ```
  ./picco-web -G <utility config> <json input config> <pubkey file 1> ... <pubkey file n>
  ```
  Arguments:
  1. `-G`: This mode is used to generate the required configuration and key files for the web page. These files include settings for variable inputs, display names, and cryptographic keys that the web interface needs.
  2. `<utility config>`: Configuration file from the program translation step.
  3. `<json input config>`: JSON file name that will include the data for webpage. 
  4. `<pubkey file 1> ... <pubkey file n>`: Public keys of the N computational parties. (Number of files depends on the number of parties.)
  
        
  ##### 2. Passcode File:
  Create a secure JSON file containing a unique passcode for each input party. These passcodes must be shared with each party separately. The file must be formatted as follows:
  ```
    {
      "users": [
        {"input_party": 1, "passcode": "abc123"},
        {"input_party": 2, "passcode": "def456"},
        {"input_party": 3, "passcode": "ghi789"}
      ]
    }
  ```

  ##### 3. Running the Server (-S mode) 
  Choose a host IP and port for the server to run on. Once the data and passcodes are ready, you can run the web server.

  ```
  ./picco-web -S <host> <port> <input_config_json> <passcode_file> <shares_filename>
  ```

  Arguments:
  1. `-S`: This mode is used to start the Flask web server using the previously generated configuration and key files.
  2. `host`: The IP address for the server.
  3. `port`: Port number for the server.
  4. `input_config_json`: JSON file name that will include the data for webpage.
  5. `passcode_file`: Path to the JSON file with user passcodes.
  6. `shares_filename`: Prefix for the output share files.

  The picco-web server will read the input data and the utility configuration and produce a set of encrypted shares for each of the N computational parties. The encrypted shares are stored in files named `shares_filename`ID, where ID is the identification number for each computational party from 1 to N. The encrypted shares are then stored in the respective output files. During the secure computation, these encrypted shares are decrypted by the computational parties just before the computation begins.

  #### Optional Customization for Devolopers

  ##### 1. Customization of Variable Names Displayed in the Server:
  Users can customize how input variables are displayed in the web interface by modifying the `display_name` field inside the `input_config_json` file after running the first step above.  

  - If `display_name` is not specified, the variable name will default to the name used in the original C program.

  ##### 2. Customization of the Web-Page Style:
  Developers can customize the look of the generated web interface (fonts, colors, layout, etc.).  
  The relevant frontend style files can be found at:

  - **Form Structure & Behavior:** `compiler/src/web/form_handler.js`  
  - **Webpage Layout & Design:** `compiler/src/web/dynamicform.html`  

  Comments have been placed inside these files to help you identify where changes can be made for customization.  

  ⚠️ **Note:** The web.cpp program needs to be recompiled after making the customizations in part two.


### 5.4. Testing and Deployment Mode Execution 

The **runtime config** will be used during program execution by computational parties and needs to be formed as follows. It is a text file that consists of $n$ text lines, where $n$ is the number of computational parties running the secure computation. Each line specifies information about the runtime setup and, in particular, contains the following four values separated by commas: 

1. an ID of a computational party between 1 and $n$;
2. an IP address or a domain name of the computational party;
3. an open port number to connecting to that party;
4. **(deployment and testing mode only)** a file name of the public key of that party for establishing a secure communication channel (this can be specified using a path or just the file name, but in either case it must be locatable by the running program; i.e., in the latter case the file must reside in the same directory as the program being executed).

The four values should be listed in the specified order on each line. Note that the same runtime config file should be distributed to all computational parties. 

To initiate secure computation, each computational party executes the following command:
```
./user_program <ID> <runtime config> <privkey file> M K <share file 1> ... <share file M> <output 1> ... <output K>
```
The first two arguments to the program are the ID of the computational party and the name of the runtime config file. The third argument stores the private key of the public-private key pair of the computational party running the computation. `M` and `K` are the number of input and output parties, respectively. After the first five arguments, the next `M` arguments list the names of the files containing input shares of input parties 1 through `M`. The `K` arguments that follow will be used for storing the output of the execution. These arguments specify prefixes of output files for each of the output parties. The program will store shares of the output for the output party `i` in a file named "`<output i>ID`" using the ID of the computational party. The same prefixes for the output filenames need to be used across all computational parties. This is because the output reconstruction program expects consistent naming of the output files.

Upon computation completion, each program outputs the program running time and stores the result of computation (output using `smcoutput`) in a file for each output party. If the output for some output party contains private variables, that party will need to use the utility program to reconstruct the result.


### 5.5. Reconstruction of Program Results

The procedure of reconstructing program results is very similar to that of generating program inputs using the utility program. Each output party needs to execute the following command:

```
./picco-utility -O <output party ID> <shares filename> <utility config> <result filename>
```

Here the flag `-O` indicates that the utility program will be used to reconstruct the program result. The third argument is the name prefix of output files containing values (e.g., shares for private variables) of program results (the program will read files "`<shares filename>i`" for each computational party `i`), and the last argument is the name of the file that will store the result of data reconstruction. Other arguments are self-explanatory. The utility program stores the plaintext output data in the same format as the plaintext input was stored in the input files.

## 6. Performance Tuning

There is currently a 5-minute timeout and 5 millisecond wait interval in the networking setup phase where nodes establish connections to one another. These times can be adjusted by the end user depending on the specific use case by modifying `MAX_RETRIES` and `WAIT_INTERVAL` in `NodeNetwork.h`.

## 7. Restrictions on User Programs

**The following restrictions apply to all user programs.**

In the current implementation, not all features of C are supported in user programs written our extension of C. We tested a rather small subset of C reserved words and the rest are commented out (and may not go past the parser). Thus, if your program does not compile, please contact us and we will examine the code and add the necessary functionalities to the PICCO compiler. The list below provides a more detailed information about restrictions on user programs in the current implementation.

- The current implementation supports private arrays with at most two dimensions.
- Built-in I/O statements `smcinput` and `smcoutput` are not allowed to appear within the body of a loop or iteration (such as a repeatedly called function) or branching statement (e.g., if-else block).
- If `smcinput` or `smcoutput` is used for an array, the number of elements to read/write needs to be given as either a constant or a variable initialized with a constant. More complex ways of specifying the size (such as arbitrary expressions) are currently not supported.
- There are restrictions on arithmetic or comparison statements used within the body of a parallel loop: If a statement contains more than a single operation, it needs to be rewritten into multiple statements that execute one operation at a time. This applies to type casting of private variables as well. 
  Also, any assignment statement should store the result of the computation into an element of an array because it is not meaningful to simultaneously store multiple values from different loop iterations in a single non-array type variable.
 
- Our current implementation does not allow code that uses private variables to be located in multiple files, e.g., in header files. Thus, all code to be translated needs to be placed in a single file.
<!-- - During program translation, the PICCO compiler places a number of temporary variables in the translated user program. Thus, if the user program contains variables with the same names, they might result in conflicts. Therefore, it is  best to avoid declaring variables with the same names in the user program. The variables created by the compiler are: 

  - `tmp`
  - `ftmp`
  - `priv_ind`
  - `priv_tmp`
  - `priv_ftmp`

  The last three variables on the list are used only if the user program contains at least one access to an array with a private index. -->
- Due to the implementation specifics of the `mpz_t` data type used for all private variables in translated programs, functions cannot return variables of type `mpz_t`. For that reason, all user-declared functions with private return values should be modified to include an extra argument passed by reference which corresponds to the return value of the function and the return type of the function should be set to void.


## Protocol Sources

The field-based protocols implemented in this software come from the following papers:
<!-- - [Gennaro et al., 1998](https://doi.org/10.1145/277697.277716)
  - Integer multiplication -->
- [Catrina and De Hoogh, 2010](https://doi.org/10.1007/978-3-642-15317-4_13)
  - Integer comparisons and equality testing using truncation
  - Bit decomposition
- [Aliasgari et al., 2013](https://www.ndss-symposium.org/wp-content/uploads/2017/09/11_4_0.pdf)
  <!-- - Integer division -->
  - Floating-point operations (addition, subtraction, multiplication, division, comparisons, equality, rounding) and type conversion (int to float, float to int)
- [Zhang et al., 2017](https://dl.acm.org/doi/abs/10.1145/3154600)
  - Pointers to private data
  - Dynamic memory management
- [Blanton et al., 2020](https://doi.org/10.1007/978-3-030-57808-4_19)
  - Integer multiplication (from Section 5.2)
  - Array access at a private location
