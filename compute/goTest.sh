#!/bin/bash

# /usr/bin/cmake --build /home/into4/picco/build --config Debug --target clean - j 22 --
rm -f reconstructed-result

cd ../compiler
./compile.sh
cd ../compute

TEST=$1
eval "../compiler/bin/picco -d sample-programs/non-thread/$TEST.c ../compiler/smc-config testing testing-cfg"
rm -rf build
./compile-usr-prog.sh -d testing.cpp

# // 1 party
eval "./../compiler/bin/picco-utility -I 1 sample-programs/non-thread/$TEST-data testing-cfg testing_shares_"

# // 1 party 
# clear
./build/testing 1 runtime-config private_01.pem 1 1 testing_shares_1 testing_output_ &
./build/testing 2 runtime-config private_02.pem 1 1 testing_shares_2 testing_output_ &
./build/testing 3 runtime-config private_03.pem 1 1 testing_shares_3 testing_output_ &
./build/testing 4 runtime-config private_04.pem 1 1 testing_shares_4 testing_output_ 
# ./build/testing 5 runtime-config private_05.pem 1 1 testing_shares_5 testing_output_
# ./build/testing 6 runtime-config private_06.pem 1 1 testing_shares_6 testing_output_ 
# ./build/testing 7 runtime-config private_07.pem 1 1 testing_shares_7 testing_output_


./../compiler/bin/picco-utility -O 1 testing_output_ testing-cfg reconstructed-result

# echo "RESULT:"
# cat reconstructed-result