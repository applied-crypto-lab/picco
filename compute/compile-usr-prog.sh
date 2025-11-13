#!/bin/bash

print_usage() {
  printf "Usage: $0 -d|-t|-m <user program> <utility config>\n"
}
#  [-S (Shamir SS)]\n"

mode=""
while getopts ":tdm" option; do
  case $option in
    t)
      mode="-DDEPLOYMENT=ON"   # Previously triggered by -d
      ;;
    d)
      mode="-DDEPLOYMENT=ON"  # New decryption-only mode
      ;;
    m)
      mode="-DDEPLOYMENT=OFF"  # Manual/test mode
      ;;
    \?)
      echo "Illegal option: -$OPTARG"
      print_usage
      exit 1
      ;;
  esac
done

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