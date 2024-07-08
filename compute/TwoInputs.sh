#!/bin/bash


# /usr/bin/cmake --build /home/into4/picco/build --config Debug --target clean - j 22 --
rm -f reconstructed-result
rm -f test*

cd ../compiler
./compiler.sh
cd ../compute

TEST=$1
eval "../compiler/bin/picco -d sample-programs/non-thread/$TEST.c ../compiler/smc-config testing testing-cfg"
rm -rf build
./compile-usr-prog.sh -d testing.cpp

eval "../compiler/bin/picco-utility -I 1 sample-programs/non-thread/$TEST-data-1 testing-cfg test_util_shares_A_"
eval "../compiler/bin/picco-utility -I 2 sample-programs/non-thread/$TEST-data-2 testing-cfg test_util_shares_B_"



# clear
./build/testing 1 runtime-config private_01.pem 2 2 test_util_shares_A_1 test_util_shares_B_1 output_example_1 output_example_2
# ./build/testing 2 runtime-config private_02.pem 2 2 test_util_shares_A_2 test_util_shares_B_2 output_example_1 output_example_2
# ./build/testing 3 runtime-config private_03.pem 2 2 test_util_shares_A_3 test_util_shares_B_3 output_example_1 output_example_2 

eval "../compiler/bin/picco-utility -O 2 output_example_2 testing-cfg reconstructed_result_2"
eval "../compiler/bin/picco-utility -O 1 output_example_1 testing-cfg reconstructed_result_1"