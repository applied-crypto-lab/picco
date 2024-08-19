#!/bin/bash

print_usage() {
  printf "Usage: $0 [-d | -m] <user_program.cpp>\n"
}
#  [-S (Shamir SS)]\n"


if test "$#" -ne 2; then
    print_usage
    exit 1
fi


while getopts "dm" option; do
  case $option in
    d)
      mode="-DDEPLOYMENT=ON"
      ;;
    m)
      mode="-DDEPLOYMENT=OFF"
      ;;
    # S)
    #   technique="-DSHAMIR=ON"
    #   ;;
    \?)
      print_usage
      exit 1
      ;;
  esac
done

fname="${2%%.*}"

cp ".CMAKE_TEMPLATE" "CMakeLists.txt"

os=$(uname) 

if [ "$os" == "Darwin" ]; then   
  sed -i '' "s/XXXXX/$fname/g" CMakeLists.txt
elif [ "$os" == "Linux" ]; then   
  sed -i "s/XXXXX/$fname/g" CMakeLists.txt
else   
  echo "Unknown OS detected"
fi


mkdir -p build
cd build

cmake -DCMAKE_BUILD_TYPE=Release $mode $technique ..
make -j8
cd ..