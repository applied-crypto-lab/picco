Testing one machine 
Compilation of PICCO

Step 1
ssh zuhramas@rigel-01.cse.buffalo.edu 
ssh zuhramas@rigel-02.cse.buffalo.edu 
ssh zuhramas@rigel-03.cse.buffalo.edu 

Step 2
cd picco/compiler
cd ../compiler
cd picco/compute
cd ../compute

Step 3 - this is gonna run in compiler directory 
./compile.sh

Step 4 - Everything else is gonna run in compute directory 
./../compiler/bin/picco -d sample-programs/non-thread/mult-add.c ../compiler/smc-config test_utility test_utility_conf 

Step 5
rm -rf build
./compile-usr-prog.sh -d test_utility.cpp test_utility_conf

Step 6
// 1 party
./../compiler/bin/picco-utility -I 1 sample-programs/non-thread/batch-div-data test_utility_conf test_util_shares_A_

Step 7
openssl genrsa -out private_01.pem 2048 
openssl rsa -in private_01.pem -outform PEM -pubout -out public_01.pem 
openssl genrsa -out private_02.pem 2048 
openssl rsa -in private_02.pem -outform PEM -pubout -out public_02.pem 
openssl genrsa -out private_03.pem 2048 
openssl rsa -in private_03.pem -outform PEM -pubout -out public_03.pem 

Step 8
// 1 party 
./../compute/build/test_utility 1 runtime-config private_01.pem 1 1 test_util_shares_A_1 output_example
./../compute/build/test_utility 2 runtime-config private_02.pem 1 1 test_util_shares_A_2 output_example
./../compute/build/test_utility 3 runtime-config private_03.pem 1 1 test_util_shares_A_3 output_example

Step 9
./../compiler/bin/picco-utility -O 1 output_example test_utility_conf reconstructed_result_1


Testing one machine 
Compilation of PICCO

ssh zuhramas@rigel-01.cse.buffalo.edu 

cd picco/compiler && clear
cd ../compiler && clear
cd picco/compute && clear
cd ../compute && clear

./compile.sh 
control+c -> to kill 
//Error-Checking 
./../compiler/bin/picco -d Error-Limitations/baching_loops.c ../compiler/smc-config test_utility running_files/test_utility_conf 
./../compiler/bin/picco -d Error-Limitations/Error-Checking/batch_op_new_test.c ../compiler/smc-config test_utility running_files/test_utility_conf 
./../compiler/bin/picco -d Error-Limitations/Error-Checking/batching.c ../compiler/smc-config test_utility running_files/test_utility_conf 
./../compiler/bin/picco -d Error-Limitations/sample_programs_here/test-rss.c ../compiler/smc-config test_utility running_files/test_utility_conf 
./../compiler/bin/picco -d RSS_Testing.c ../compiler/smc-config test_utility running_files/test_utility_conf 

./../compiler/bin/picco -d sample-programs/non-thread/float-addition.c ../compiler/smc-config test_utility running_files/test_utility_conf 
./../compiler/bin/picco -d sample-programs/non-thread/float-comparison.c ../compiler/smc-config test_utility running_files/test_utility_conf 
./../compiler/bin/picco -d sample-programs/non-thread/float-conversion.c ../compiler/smc-config test_utility running_files/test_utility_conf 
./../compiler/bin/picco -d sample-programs/non-thread/float-division.c ../compiler/smc-config test_utility running_files/test_utility_conf 
./../compiler/bin/picco -d sample-programs/non-thread/float-multiplication.c ../compiler/smc-config test_utility running_files/test_utility_conf 
./../compiler/bin/picco -d sample-programs/non-thread/float-subtraction.c ../compiler/smc-config test_utility running_files/test_utility_conf 

./../compiler/bin/picco -d sample-programs/non-thread/mult.c ../compiler/smc-config test_utility running_files/test_utility_conf 

rm -rf build
./compile-usr-prog.sh -d test_utility.cpp running_files/test_utility_conf

./../compiler/bin/picco-utility -I 1 rss_test_data running_files/test_utility_conf running_files/test_util_shares_A_

// 1 party
./../compiler/bin/picco-utility -I 1 sample-programs/non-thread/float-addition-data running_files/test_utility_conf running_files/test_util_shares_A_
./../compiler/bin/picco-utility -I 1 sample-programs/non-thread/float-comparison-data running_files/test_utility_conf running_files/test_util_shares_A_
./../compiler/bin/picco-utility -I 1 sample-programs/non-thread/float-conversion-data running_files/test_utility_conf running_files/test_util_shares_A_
./../compiler/bin/picco-utility -I 1 sample-programs/non-thread/float-division-data running_files/test_utility_conf running_files/test_util_shares_A_
./../compiler/bin/picco-utility -I 1 sample-programs/non-thread/float-multiplication-data running_files/test_utility_conf running_files/test_util_shares_A_
./../compiler/bin/picco-utility -I 1 sample-programs/non-thread/float-subtraction-data running_files/test_utility_conf running_files/test_util_shares_A_

./../compiler/bin/picco-utility -I 1 sample-programs/non-thread/batch-div-data running_files/test_utility_conf running_files/test_util_shares_A_

// 2 parties
./../compiler/bin/picco-utility -I 1 sample-programs/non-thread/eqz-data-1 running_files/test_utility_conf running_files/test_util_shares_A_
./../compiler/bin/picco-utility -I 2 sample-programs/non-thread/eqz-data-2 running_files/test_utility_conf running_files/test_util_shares_B_

openssl genrsa -out running_files/private_01.pem 2048 && openssl rsa -in running_files/private_01.pem -outform PEM -pubout -out running_files/public_01.pem 
openssl genrsa -out running_files/private_02.pem 2048 && openssl rsa -in running_files/private_02.pem -outform PEM -pubout -out running_files/public_02.pem 
openssl genrsa -out running_files/private_03.pem 2048 && openssl rsa -in running_files/private_03.pem -outform PEM -pubout -out running_files/public_03.pem 

// 1 party 
./../compute/build/test_utility 1 running_files/runtime-config running_files/private_01.pem 1 1 running_files/test_util_shares_A_1 running_files/output_example
./../compute/build/test_utility 2 running_files/runtime-config running_files/private_02.pem 1 1 running_files/test_util_shares_A_2 running_files/output_example
./../compute/build/test_utility 3 running_files/runtime-config running_files/private_03.pem 1 1 running_files/test_util_shares_A_3 running_files/output_example
 
// 2 parties (2 input, 2 output)
./../compute/build/test_utility 1 running_files/runtime-config running_files/private_01.pem 2 2 running_files/test_util_shares_A_1 running_files/test_util_shares_B_1 running_files/output_example_1 running_files/output_example_2 
./../compute/build/test_utility 2 running_files/runtime-config running_files/private_02.pem 2 2 running_files/test_util_shares_A_2 running_files/test_util_shares_B_2 running_files/output_example_1 running_files/output_example_2 
./../compute/build/test_utility 3 running_files/runtime-config running_files/private_03.pem 2 2 running_files/test_util_shares_A_3 running_files/test_util_shares_B_3 running_files/output_example_1 running_files/output_example_2 


./../compiler/bin/picco-utility -O 1 running_files/output_example running_files/test_utility_conf reconstructed_result_1


./../compiler/bin/picco-utility -O 1 running_files/output_example running_files/test_utility_conf float-addition-reconstructed_result_1
./../compiler/bin/picco-utility -O 1 running_files/output_example running_files/test_utility_conf float-comparison-reconstructed_result_1
./../compiler/bin/picco-utility -O 1 running_files/output_example running_files/test_utility_conf float-conversion-reconstructed_result_1
./../compiler/bin/picco-utility -O 1 running_files/output_example running_files/test_utility_conf float-division-reconstructed_result_1
./../compiler/bin/picco-utility -O 1 running_files/output_example running_files/test_utility_conf float-multiplication-reconstructed_result_1
./../compiler/bin/picco-utility -O 1 running_files/output_example running_files/test_utility_conf float-subtraction-reconstructed_result_1


(2 input, 2 output)
./../compiler/bin/picco-utility -O 2 running_files/output_example_2 running_files/test_utility_conf reconstructed_result_2
./../compiler/bin/picco-utility -O 1 running_files/output_example_1 running_files/test_utility_conf reconstructed_result_1


control+shift+p then wrap to wrap the text on the same page 
  
Killing the port numbers 
lsof -ti :#
kill -9 PID

See all the processes run by me
ps -u zuhramas
ps -ef | grep zuhramas

Kill all the processes run by me
pkill -u zuhramas
pkill -9 -u zuhramas

top -> this is to know how many percent of memory is used for what

Kill zuhramas 
ps -u zuhramas
kill PID 

// To Run this: gcc -fopenmp hmm.c -o hmm
// To Run this: gcc hmm.c -o hmm
// ./hmm

Example: 
    107418: This is the process ID (PID) of the top command itself.
    zuhramas: This is the username of the user who launched the top command.
    20: This indicates the priority of the process. Higher values mean lower priority.
    0: This indicates the scheduling policy of the process. 0 means the default scheduling policy.
    11268: This is the memory usage of the process in kilobytes (KiB).
    5720: This is the memory usage of the process that's in the shared memory area.
    4748: This is the memory usage of the process that's in the memory-mapped area.
    R: This indicates the current state of the process. R means the process is running.
    0.7: This is the percentage of CPU usage for the process.
    0.0: This is the percentage of memory usage for the process.
    0:00.19: This is the cumulative CPU time usage of the process.