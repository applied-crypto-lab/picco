#!/bin/bash

print_usage() {
  printf "Usage: $0 [-d | -m] <user_program.cpp> <utility_config>\n"
}
#  [-S (Shamir SS)]\n"


if test "$#" -ne 3; then
    print_usage
    exit 1
fi

# Parse user_program_config(utility_config)
bits=""
technique=""
config_file="$3"

parse_config() {
  while IFS=: read -r key value; do
    # Remove any leading/trailing whitespace from the key and value
    key=$(echo "$key" | xargs)
    value=$(echo "$value" | xargs)
    case $key in
      bits)
        bits="$value"
        ;;
      technique)
        technique="$value"
        ;;
    esac
  done < "$config_file"
}

parse_config

# Set bits_flag based on the bits value
if [ "$bits" -le 32 ]; then
  bits_flag="-DUSE_32=ON -DUSE_64=OFF"
elif [ "$bits" -le 64 ]; then
  bits_flag="-DUSE_32=OFF -DUSE_64=ON"
fi

# Handle technique values
case $technique in
  1)
    technique_flag="-DSHAMIR=OFF -DRSS=ON"
    ;;
  2)
    technique_flag="-DSHAMIR=ON -DRSS=OFF"
    ;;
esac


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

cmake -DCMAKE_BUILD_TYPE=Release $mode $bits_flag $technique_flag ..
make -j8
cd ..