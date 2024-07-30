#!/bin/bash

config_name=confs/runtime-config-lake-$1

timestamp=$(date -d "today" +"%Y_%m_%d_%H_%M")
echo $timestamp

source shared_vars_$2.sh


if [ "$1" == "3" ]; then   
    declare -a sleep_times=(4 2 0)
    index=$(($(($3))-1))
elif [ "$1" == "5" ]; then   
    declare -a sleep_times=(4 3 2 1 0)
    index=$(($(($2))-1))
elif [ "$1" == "7" ]; then   
    declare -a sleep_times=(12 10 8 6 4 2 0)
    index=$(($(($2))-1))
else   
  echo "Unknown OS detected"
fi



for exp in "${exps[@]}"
do
    sleep ${sleep_times[$index]}; ./build/rss_main $3 $config_name private0$3.pem $1 $exp | tee -a $2_$1_pc_micro_$4_$timestamp.txt
done

