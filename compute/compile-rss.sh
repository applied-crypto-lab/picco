#!/bin/bash

print_usage() {
  printf "Usage: $0 <ring_size>\n"
}

if test "$#" -ne 1; then
    print_usage
    exit 1
fi


# Set bits_flag based on the bits value
if [ "$1" -le 32 ]; then
  bits_flag="-DUSE_32=ON -DUSE_64=OFF"
elif [ "$1" -le 64 ]; then
  bits_flag="-DUSE_32=OFF -DUSE_64=ON"
fi


mkdir -p build
cd build

cmake -DCMAKE_BUILD_TYPE=Debug -DDEPLOYMENT=ON $bits_flag -DSHAMIR=OFF -DRSS=ON ..
make -j8
cd ..
