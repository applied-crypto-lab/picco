clear
NUM=$1
eval "./build/testing $NUM runtime-config private_0$NUM.pem 1 1 testing_shares_$NUM testing_output_"
