#ifndef MAIN_H_
#define MAIN_H_

#include "NodeConfiguration.h"
#include "NodeNetwork.h"
#include "benchmark_3pc.h"
#include "benchmark_5pc.h"
#include "benchmark_7pc.h"
#include "eda_rb_pregen_3pc.h"
#include "eda_rb_pregen_5pc.h"
#include "minionn.h"
#include "minionn_5pc.h"
#include "q_mobilenet.h"
#include "q_mobilenet_5pc.h"
#include "test_protocols_3pc.h"

#include <iostream>

extern "C" int ort_initialize(int *, char ***);
extern "C" void ort_finalize(int);

void benchmark_main(uint numParties, char *protocol, uint size, uint batch_size, uint num_iterations);

void minionn_main(std::string model_path, uint num_parties, uint num_images, uint batch_size);

void q_mobilenet_main(std::string mobile_path, uint numParties, uint input_dim, uint alpha_index, uint batch_size, uint num_iterations, uint num_discarded);

void eda_rb_pregen_main(uint num_parties, uint input_dim, uint alpha_index, uint batch_size, uint num_iterations, uint num_discarded, string path);


#endif