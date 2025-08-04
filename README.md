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


The source code of PICCO mainly consists of two directories: *compiler* and *compute*. The compiler directory contains the source code of the PICCO compiler whose functionality is to translate a user's program into its secure implementation. The `compute` directory contains the source code of the computational framework that will be used to securely execute the user's translated program in a distributed setting.

## Source code dependencies

To compile or run user programs using PICCO code, a machine should have the following libraries installed:

- [GCC](https://gcc.gnu.org/) or [LLVM/clang](https://clang.llvm.org/) (clang generally performs better than GCC and is recommended)
- [GMP](https://gmplib.org/)
- [OpenSSL](https://www.openssl.org/source/) (v1.1.1 or newer)
- [Flex](https://github.com/westes/flex.git) - fast lexical analyzer generator
- [GNU Bison parser](https://www.gnu.org/software/bison/)
- [CMake](https://cmake.org/)

Additionally, if you wish to compile and run multithreaded user programs, the following library is required:

- [OMPi Compiler](https://paragroup.cse.uoi.gr/wpsite/software/ompi/) (v1.2.3)


### Public-private key pair generation

Programs compiled by PICCO in **deployment and testing mode** use pair-wise secure channels protected using symmetric key cryptography, and the parties' public keys are used to communicate the key material. Each computational party must have a public-private key pair, and the name of a file containing a computational node's public key is stored in the runtime configuration file. In the current implementation, only RSA keys are supported, and the key stored in a file needs to be in a format compatible with what OpenSSL uses. The following example commands can be used to generate a public-private key pair for party `ID`:

```
openssl genrsa -out private_ID.pem 2048
openssl rsa -in private_ID.pem -outform PEM -pubout -out public_ID.pem
```
After the key pairs are created, the public keys should be distributed to all computational parties.


## Compilation of PICCO

To use PICCO, the following two programs need to be compiled: 

1. the PICCO compiler itself, and
2. the utility program that will generate secret shares of private inputs and to assemble the output of computational results upon the completion of secure computation.

To compile both programs, one needs to go to the directory `compiler/` and run the command
```
./compile.sh
``` 
which produces the executable files `picco` and `picco-utility`, and moves them in the directory `compiler/bin/`. The executables correspond to the PICCO compiler and the utility, respectively, and can be placed in any directory of user's choice at a later time. 

These executable programs are used to compile a user's program written in an extension of C into its secure implementation, produce shares of private inputs to be used in secure computation, and reconstruct the output from the shares after secure execution of the user program. 

## Translation of user programs

Before describing the procedure for compiling a user program, we explain the composition of the **SMC config** file that the program expects.


**SMC config.** The SMC config is a text file that consists of five lines, with each of them being in the format of $P=V$, where $P$ indicates an SMC parameter and $V$ indicates its value. The SMC config file contains the following parameters:

1. `technique` - the secret sharing technique to be used. Currently, we only support the value `shamir` for Shamir secret sharing.
2. `bits` - the bit size of the modulus. This parameter is optional, and if the programmer is uncertain what modulus size should be used, its value should be left blank in the config file. 
3. `peers` - the number of computational parties $N$
4. `threshold` - the threshold value $t$, which is the maximum number of corrupted/colluding parties. For techniques that assume an honest majority (e.g. Shamir secret sharing), we expect $N = 2 \cdot t + 1$.
5. `inputs` - the number of input parties 
6. `outputs` - the number of output parties

The parameters can be placed in any order in the SMC config. 
The `inputs` and `outputs` parameters allow a user to run a program with inputs distributed multiple parties and produce multiple outputs with each of them being sent to a distinct output party. It will be assumed that input/output computational parties are numbered sequentially from 1 up until the specified number of parties. For example, if the number of inputs parties is $N$, they are expected to be numbered 1 through $N$. The same entity can take on different roles (e.g., input party 1 can also be output party 2).
  
**Compilation mode.** PICCO is equipped with three possible modes for compilation and execution. In *testing mode* (denoted with the flag `-t`), computational parties use public key cryptography in order to set up secure communication channels. Inputs to the computation (as specified in a user's program) must be properly shared beforehand using `picco-utility`. In *deployment mode* (denoted with the flag `-d`), computational parties use public key cryptography in order to set up secure communication channels. Inputs to the computation (as specified in a user's program) must be properly shared beforehand using `picco-web`. In *measurement mode* (denoted with the flag `-m`) foregoes public key infrastructure, instead having parties directly establish communication channels with each other. Any secret shared private inputs are produced via local pseudorandom generators once the initial setup is completed. This mode is useful if you are exclusively interested in benchmarking specific operations or protocols. 

**Note: as of this time measurement mode only supports generating shares of random private values, but not random public values. Any restrictions other restrictions outlined in [the corresponding section](#restrictions-on-user-programs) apply to all modes deployment, testing and measurement modes.** 

**Program compilation.** To compile a user's program into its secure implementation, one needs to execute the following command:

```
./picco [-d | -t | -m] <user program> <SMC config> <translated program> <utility config>
``` 

Here, the arguments that the executable `picco` takes are:
1. the compilation mode (`-d` for deployment, `-t` for testing or `-m` for measurement)
2. the name of the file containing user program to be translated;
3. the of the file containing SMC config as described above;
4. the name of a file that will store the result of user program translation as a C++ program; 
5. the name of a file that will store information that needs to be communicated to the utility program (such as the setup information found in the SMC config file and the list of variables used in I/O), i.e., a config file for the utility program.

The executable takes two files as its input and produces two files as its output.


## (Native) compilation of user programs

In order to run a user's translated program in a distributed setting, one needs to compile it using a native C++ compiler to produce a binary executable file, create a runtime config file, and send the executable to each computational party together with the runtime config and a file that stores input shares for that party. These steps are discussed below.

To **compile** the translated program, the program should be placed in the `compute/` directory at the compilation time, as it needs library functions stored in the directory `compute/smc-compute/`. 

The binary executable of the translated program is produced by running the following script from the `compute/` directory:

```
./compile-usr-prog.sh [-d | -t | -m] <user_program> <utility_config>
```
where `-d`, `-t` and `-m` are the compilation mode flags, `user_program` is the name of the translated program generated earlier (note, without the `.cpp` extension), and `utility_config` is the name of the utility file generated for this user program in the previous step. The script produces an executable named `user_program` stored in `build/` and can later be placed in any other directory. 

The **runtime config** will be used during program execution by computational parties and needs to be formed as follows. It is a text file that consists of $N$ text lines, where $N$ is the number of computational parties running the secure computation. Each line specifies information about the runtime setup and, in particular, contains the following four values separated by commas: 

1. an ID of a computational party between 1 and $N$;
2. an IP address or a domain name of the computational party;
3. an open port number to connecting to that party;
4. **(deployment and testing mode only)** a file name of the public key of that party for establishing a secure communication channel (this can be specified using a path or just the file name, but in either case it must be locatable by the running program; i.e., in the latter case the file must reside in the same directory as the program being executed).

In measurement mode, the public key filenames can be omitted leaving only the ID, IP address, and port number for each party. The four values should be listed in the specified order on each line. Note that the same runtime config file should be distributed to all computational parties. 

## Program execution

The **execution** uses $N$ machines that can communicate with each other, where $N$ is the number of computational parties participating in the computation.
There is currently a 5-minute timeout and 5 millisecond wait interval in the networking setup phase where nodes establish connections to one another. These times can be adjusted by the end user depending on the specific use case by modifying `MAX_RETRIES` and `WAIT_INTERVAL` in `NodeNetwork.h`.

 <!-- **Our current implementation requires that the computational parties start the execution in a particular order:** the program has to be started by the parties in the decreasing order of their IDs, i.e., party $N$ first, then by party $N-1$, etc. with party 1 starting the program last. This is because the machines connect to each other in a specific order.  -->

Based on the computational mode 
([measurement mode](#measurement-mode-execution), [testing mode](#testing-mode-setup-and-execution) and [deployment mode](#deployment-mode-setup-and-execution)), follow to the links to their respective sections.


## Measurement mode execution

To initiate secure computation in measurement mode, each computational party 
<!-- **(in descending order according to their ID)**  -->
executes the following command:

```
./user_program <ID> <runtime config> 
```
where the arguments to the program are the ID of the computational party and the name of the runtime config file, respectively. 
## Testing mode setup and execution

### Input share generation

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


- **Generating input data**. After an input party generates input data in the specified format and saves it in a file of the user's choice, the utility program can be invoked as follows:  
    
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


## Deployment mode setup and execution

### Input share generation

Prior to performing secure computation, the input parties must prepare input data (that could be private, public, or both) and distribute them to the computational parties. Assuming that at least one of the inputs is private, an input party needs to enter the inputs through a web server to produce shares of private inputs. When using the web server, you will be prompted to submit your data directly through a web form. The server requires a unique passcode for authentication before you can proceed.

#### Data Submission and Format:
  1. For single integer or real number inputs, you will enter the values directly into a text box.
  2. For arrays and two-dimensional arrays, you will create a CSV file with your data and upload it through the web form. The format of this CSV file is identical to the format used for the original text-based input.
  

#### Setting up the Web Server:
For developers setting up the web server, the picco-web program is used. This requires three main setup steps:

  ##### 1. Server Configuration: 
  Choose a host IP and port for the server to run on.
  ###### Running the Web Server:
  1. For a local host setup - if you want to run the server on your own machine for testing or local use, you would use an IP address that refers to the machine itself. The two most common options are:
  a. 127.0.0.1 (the loopback address): This makes the server accessible only from the machine it's running on.
  b. 0.0.0.0: This makes the server listen on all network interfaces. This means it will be accessible from the machine itself and also from other devices on the same local network (like other computers or phones connected to the same Wi-Fi).

  You can use any available port number (e.g., 8000, 8080, 5000) for local testing.

  For example, if use host 0.0.0.0, and the port number is 8000, the command would be:
  ```
  ./../compiler/bin/picco-web 0.0.0.0 8000 ...
  ```
  
  2. For a public website - if you want the server to be accessible to the world (or all parties), you would use the public IP address of the server. This is the unique IP address assigned to the server by your hosting provider, and it's what other people will use to connect to it over the internet. 

  You can also use any open and allowed port number (e.g., 80, 443, 8080), depending on your server configuration and firewall settings.
  
  For example, if your server's public IP address is 203.0.113.10, and the port number is 80, the command would be:
  ```
  ./../compiler/bin/picco-web 203.0.113.10 80 ...
  ```
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
  ##### 3. Command-Line Execution: 
  Launch the server using the following command. The arguments specify the host, port, configuration files, and public keys of the computational parties.

  ```
  ./../compiler/bin/picco-web <host> <port> <utility_config> <input_config_json> <passcode_file> <share_base_name> <public_key_file1> <public_key_file2> <public_key_file3> ...
  ```

  The utility program `picco-web` the following arguments:

  1. host: The IP address for the server (e.g., 0.0.0.0).
  2. port: The port number the server listens on (e.g., 8080).
  3. <utility_config>: The configuration file from the program translation step.
  4. <input_config_json>: A JSON file defining the required inputs.
  5. <passcode_file>: The path to the JSON file with user passcodes.
  6. <share_base_name>: A prefix for the output share files.
  7. <public_key_file1>, etc.: The public keys of the computational parties for secure communication.

  The picco-web server will read the input data and the utility configuration and produce a set of encrypted shares for each of the N computational parties. The encrypted shares are stored in files named <shares filename>ID, where ID is the identification number for each computational party from 1 to N. The encrypted shares are then stored in the respective output files. During the secure computation, these encrypted shares are decrypted by the computational parties just before the computation begins.

### Deployment and Testing mode execution 

To initiate secure computation, each computational party executes the following command:
```
./user_program <ID> <runtime config> <privkey file> M K <share file 1> ... <share file M> <output 1> ... <output K>
```
The first two arguments to the program are the ID of the computational party and the name of the runtime config file. The third argument stores the private key of the public-private key pair of the computational party running the computation. `M` and `K` are the number of input and output parties, respectively. After the first five arguments, the next `M` arguments list the names of the files containing input shares of input parties 1 through `M`. The `K` arguments that follow will be used for storing the output of the execution. These arguments specify prefixes of output files for each of the output parties. The program will store shares of the output for the output party `i` in a file named "`<output i>ID`" using the ID of the computational party. The same prefixes for the output filenames need to be used across all computational parties. This is because the output reconstruction program expects consistent naming of the output files.

Upon computation completion, each program outputs the program running time and stores the result of computation (output using `smcoutput`) in a file for each output party. If the output for some output party contains private variables, that party will need to use the utility program to reconstruct the result.


### Reconstruction of program results

The procedure of reconstructing program results is very similar to that of generating program inputs using the utility program. Each output party needs to execute the following command:

```
./picco-utility -O <output party ID> <shares filename> <utility config> <result filename>
```

Here the flag `-O` indicates that the utility program will be used to reconstruct the program result. The third argument is the name prefix of output files containing values (e.g., shares for private variables) of program results (the program will read files "`<shares filename>i`" for each computational party `i`), and the last argument is the name of the file that will store the result of data reconstruction. Other arguments are self-explanatory. The utility program stores the plaintext output data in the same format as the plaintext input was stored in the input files.

## Restrictions on user programs

**The following restrictions apply to all modes deployment, testing and measurement modes.**

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


## Protocol sources

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
