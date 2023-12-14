#!/bin/bash


print_usage() {
  printf "Usage: $0 [-d | -m]\n"
}
#  [-S (Shamir SS)]\n"


if test "$#" -ne 1; then
    print_usage
    exit 1
fi

# currently the seed has mode-dependent code
# as such, we add a new flag, which will write the corresponding value to the file
while getopts "dm" option; do
  case $option in
    d)
      echo "#define __DEPLOYMENT__ 1" > 'src/seed/mode.h'
      ;;
    m)
      echo "#define __DEPLOYMENT__ 0" > 'src/seed/mode.h'
      ;;
    \?)
      print_usage
      exit 1
      ;;
  esac
done



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

