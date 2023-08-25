#!/bin/bash
mkdir -p bin # making bin directory if does not exist
cd src/picco	
make clean
make
mv picco ../../bin/
cd ../utility
make
mv picco-utility ../../bin/
cd ../seed
make
mv picco-seed ../../bin/

#rm var-test*
#cd ../compiler
#make
#cd ../compute/
#../compiler/ompi ../compiler/test-programs/var-test.c ../compiler/config.txt > var-test.cpp 
#make var-test

# make ; mv picco-seed ../../bin/

