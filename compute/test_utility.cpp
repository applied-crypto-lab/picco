/* File generated from [sample-programs/non-thread/priv-global-tester.c] by PICCO Thu Mar 21 14:14:37 2024
 */

#include <limits.h>

#include <float.h>

//extern void *memcpy(void*,const void*,unsigned int);

//# 1 "ort.onoff.defs"

extern  "C" int   ort_initialize(int*, char***);
 extern "C" void  ort_finalize(int);




#include "smc-compute/SMC_Utils.h"

#include <gmp.h>

#include <omp.h>


SMC_Utils *__s;


std::vector<int> seed_map = {3};

priv_int g_priv_int_with_value;

priv_int g_priv_int_without_value;

int g_pub_int_with_value = 22;

int g_pub_int_without_value;

priv_int g_reg_int_with_value;

priv_int g_reg_int_without_value;

priv_int* g_priv_int_arr; 

int g_pub_int_arr[4];

priv_int* g_reg_int_arr; 

priv_int g_priv_int_arithmetic;

int g_pub_int_arithmetic = (3 + 4);

priv_int g_reg_int_arithmetic;

priv_int* g_priv_float_with_value; 

priv_int* g_priv_float_without_value; 

float g_pub_float_with_value = 2.2;

float g_pub_float_without_value;

priv_int* g_reg_float_with_value; 

priv_int* g_reg_float_without_value; 

priv_int** g_priv_float_arr; 

float g_pub_float_arr[4];

priv_int** g_reg_float_arr; 

priv_int* g_priv_float_arithmetic; 

float g_pub_float_arithmetic = (3.7 + 4.7);

priv_int* g_reg_float_arithmetic; 

priv_int g_priv_ptr_int_var;


priv_int g_non_ptr_int_var2;



int g_pub_ptr_int_var = 33;

int *g_pub_ptr_int_with_value = &g_pub_ptr_int_var;

int *g_pub_ptr_int_without_value;

priv_int g_reg_ptr_int_var;


priv_int g_reg_ptr_int_var2;




int  __original_main(int _argc_ignored, char **_argv_ignored)
{
  
  priv_int _picco_tmp1;
  ss_init(_picco_tmp1);

  priv_int* _picco_ftmp1 = (priv_int*)malloc(sizeof(priv_int) * 4);
  for(int _picco_j = 0; _picco_j < 4; _picco_j++)
    ss_init(_picco_ftmp1[_picco_j]);
  void* _picco_temp_;
   priv_int _picco_priv_tmp1, _picco_priv_tmp2;
  ss_init(_picco_priv_tmp1);
  ss_init(_picco_priv_tmp2);
  priv_ptr _picco_tmp_int_ptr1 = __s->smc_new_ptr(1, 0);
  priv_ptr _picco_tmp_int_ptr2 = __s->smc_new_ptr(1, 0);
priv_int non_g_reg_int_with_value;
  ss_init(non_g_reg_int_with_value);
  __s->smc_set(44, non_g_reg_int_with_value, -1, 32, "int", -1);

  priv_int non_g_reg_int_without_value;
  ss_init(non_g_reg_int_without_value);

  priv_int non_g_priv_int_with_value;
  ss_init(non_g_priv_int_with_value);
  __s->smc_set(55, non_g_priv_int_with_value, -1, 32, "int", -1);

  priv_int non_g_priv_int_without_value;
  ss_init(non_g_priv_int_without_value);

  int non_g_pub_int_with_value = 55;

  int non_g_pub_int_without_value;

  priv_int* non_g_priv_int_arr; 
  non_g_priv_int_arr = (priv_int*)malloc(sizeof(priv_int) * (2));
  for (int _picco_i = 0; _picco_i < 2; _picco_i++)
        ss_init(non_g_priv_int_arr[_picco_i]);

  int non_g_pub_int_arr[5];

  priv_int* non_g_reg_int_arr; 
  non_g_reg_int_arr = (priv_int*)malloc(sizeof(priv_int) * (3));
  for (int _picco_i = 0; _picco_i < 3; _picco_i++)
        ss_init(non_g_reg_int_arr[_picco_i]);

  priv_int non_g_reg_priv_int_arithmetic;
  ss_init(non_g_reg_priv_int_arithmetic);
  __s->smc_set((7 + 8), non_g_reg_priv_int_arithmetic, -1, 32, "int", -1);

  int non_g_reg_pub_int_arithmetic = (9 + 10);

  priv_int non_g_reg_int_arithmetic;
  ss_init(non_g_reg_int_arithmetic);
  __s->smc_set((11 + 12), non_g_reg_int_arithmetic, -1, 32, "int", -1);

  priv_int* non_g_reg_float_with_value; 
  non_g_reg_float_with_value = (priv_int*)malloc(sizeof(priv_int) * (4));
  for (int _picco_i = 0; _picco_i < 4; _picco_i++)
        ss_init(non_g_reg_float_with_value[_picco_i]);
  __s->smc_set(4.4, non_g_reg_float_with_value, -1, -1, 32, 9, "float", -1);

  priv_int* non_g_reg_float_without_value; 
  non_g_reg_float_without_value = (priv_int*)malloc(sizeof(priv_int) * (4));
  for (int _picco_i = 0; _picco_i < 4; _picco_i++)
        ss_init(non_g_reg_float_without_value[_picco_i]);

  priv_int* non_g_priv_float_with_value; 
  non_g_priv_float_with_value = (priv_int*)malloc(sizeof(priv_int) * (4));
  for (int _picco_i = 0; _picco_i < 4; _picco_i++)
        ss_init(non_g_priv_float_with_value[_picco_i]);
  __s->smc_set(5.5, non_g_priv_float_with_value, -1, -1, 32, 9, "float", -1);

  priv_int* non_g_priv_float_without_value; 
  non_g_priv_float_without_value = (priv_int*)malloc(sizeof(priv_int) * (4));
  for (int _picco_i = 0; _picco_i < 4; _picco_i++)
        ss_init(non_g_priv_float_without_value[_picco_i]);

  float non_g_pub_float_with_value = 5.5;

  float non_g_pub_float_without_value;

  priv_int** non_g_priv_float_arr; 
  non_g_priv_float_arr = (priv_int**)malloc(sizeof(priv_int*) * (2));
  for (int _picco_i = 0; _picco_i < 2; _picco_i++)
    {
      non_g_priv_float_arr[_picco_i] = (priv_int*)malloc(sizeof(priv_int) * (4));
      for (int _picco_j = 0; _picco_j < 4; _picco_j++)
            ss_init(non_g_priv_float_arr[_picco_i][_picco_j]);
    }

  float non_g_pub_float_arr[5];

  priv_int** non_g_reg_float_arr; 
  non_g_reg_float_arr = (priv_int**)malloc(sizeof(priv_int*) * (3));
  for (int _picco_i = 0; _picco_i < 3; _picco_i++)
    {
      non_g_reg_float_arr[_picco_i] = (priv_int*)malloc(sizeof(priv_int) * (4));
      for (int _picco_j = 0; _picco_j < 4; _picco_j++)
            ss_init(non_g_reg_float_arr[_picco_i][_picco_j]);
    }

  priv_int* non_g_reg_priv_float_arithmetic; 
  non_g_reg_priv_float_arithmetic = (priv_int*)malloc(sizeof(priv_int) * (4));
  for (int _picco_i = 0; _picco_i < 4; _picco_i++)
        ss_init(non_g_reg_priv_float_arithmetic[_picco_i]);
  __s->smc_set((7.5 + 8.7), non_g_reg_priv_float_arithmetic, -1, -1, 32, 9, "float", -1);

  float non_g_reg_pub_float_arithmetic = (96.6 + 10.8);

  priv_int* non_g_reg_float_arithmetic; 
  non_g_reg_float_arithmetic = (priv_int*)malloc(sizeof(priv_int) * (4));
  for (int _picco_i = 0; _picco_i < 4; _picco_i++)
        ss_init(non_g_reg_float_arithmetic[_picco_i]);
  __s->smc_set((11.4 + 12.5), non_g_reg_float_arithmetic, -1, -1, 32, 9, "float", -1);

  priv_int priv_ptr_int_var;
  ss_init(priv_ptr_int_var);
  __s->smc_set(12, priv_ptr_int_var, -1, 32, "int", -1);

  priv_ptr non_g_priv_ptr_int_with_value = __s->smc_new_ptr(1, 0);
  __s->smc_set_int_ptr(non_g_priv_ptr_int_with_value, &priv_ptr_int_var, "int", -1);

  priv_int non_ptr_int_var2;
  ss_init(non_ptr_int_var2);
  __s->smc_set(12, non_ptr_int_var2, -1, 32, "int", -1);

  priv_ptr non_g_priv_ptr_int_with_value2 = __s->smc_new_ptr(1, 0);
  __s->smc_set_int_ptr(non_g_priv_ptr_int_with_value2, &non_ptr_int_var2, "int", -1);

  priv_ptr non_g_priv_ptr_int_without_value = __s->smc_new_ptr(1, 0);

  int pub_ptr_int_var = 12;

  int *non_g_pub_ptr_int_with_value = &pub_ptr_int_var;

  int *non_g_pub_ptr_int_without_value;

  priv_int reg_ptr_int_var;
  ss_init(reg_ptr_int_var);
  __s->smc_set(44, reg_ptr_int_var, -1, 32, "int", -1);

  priv_ptr reg_ptr_int_with_value = __s->smc_new_ptr(1, 0);
  __s->smc_set_int_ptr(reg_ptr_int_with_value, &reg_ptr_int_var, "int", -1);

  priv_int reg_ptr_int_var2;
  ss_init(reg_ptr_int_var2);
  __s->smc_set(32, reg_ptr_int_var2, -1, 32, "int", -1);

  priv_ptr reg_ptr_int_with_value2 = __s->smc_new_ptr(1, 0);
  __s->smc_set_int_ptr(reg_ptr_int_with_value2, &reg_ptr_int_var2, "int", -1);

  priv_ptr reg_ptr_int_without_value = __s->smc_new_ptr(1, 0);

  __s->smc_input(1, &g_priv_int_without_value, "int", -1);

  __s->smc_input(1, &g_pub_int_without_value, "int", -1);

  __s->smc_input(1, &g_reg_int_without_value, "int", -1);

  __s->smc_input(1, g_priv_int_arr, 3, "int", -1);

  __s->smc_input(1, g_pub_int_arr, 4, "int", -1);

  __s->smc_input(1, g_reg_int_arr, 2, "int", -1);

  __s->smc_input(1, &g_priv_float_without_value, "float", -1);

  __s->smc_input(1, &g_pub_float_without_value, "float", -1);

  __s->smc_input(1, &g_reg_float_without_value, "float", -1);

  __s->smc_input(1, g_priv_float_arr, 3, "float", -1);

  __s->smc_input(1, g_pub_float_arr, 4, "float", -1);

  __s->smc_input(1, g_reg_float_arr, 2, "float", -1);

  __s->smc_set(1, g_priv_int_arr[0], -1, 32, "int", -1);

  __s->smc_set(2, g_priv_int_arr[1], -1, 32, "int", -1);

  __s->smc_set(3, g_priv_int_arr[2], -1, 32, "int", -1);

  g_pub_int_arr[0] = 11;

  g_pub_int_arr[1] = 22;

  g_pub_int_arr[2] = 33;

  g_pub_int_arr[3] = 44;

  __s->smc_set(111, g_reg_int_arr[0], -1, 32, "int", -1);

  __s->smc_set(222, g_reg_int_arr[1], -1, 32, "int", -1);

  __s->smc_set(54, non_g_priv_int_arr[0], -1, 32, "int", -1);

  __s->smc_set(53, non_g_priv_int_arr[1], -1, 32, "int", -1);

  non_g_pub_int_arr[0] = 7;

  non_g_pub_int_arr[1] = 6;

  non_g_pub_int_arr[2] = 5;

  non_g_pub_int_arr[3] = 4;

  non_g_pub_int_arr[4] = 3;

  __s->smc_set(9, non_g_reg_int_arr[0], -1, 32, "int", -1);

  __s->smc_set(8, non_g_reg_int_arr[1], -1, 32, "int", -1);

  __s->smc_set(7, non_g_reg_int_arr[2], -1, 32, "int", -1);

  __s->smc_set(98, g_priv_int_with_value, -1, 32, "int", -1);

  __s->smc_set(99, g_priv_int_without_value, -1, 32, "int", -1);

  __s->smc_set(((((44 + 1) * 33) - 333)) / 1, g_priv_int_with_value, -1, 32, "int", -1);

  __s->smc_set(((((11 + 122) * 323) - 444)) / 1, g_priv_int_without_value, -1, 32, "int", -1);

  __s->smc_add(g_priv_int_with_value, g_priv_int_with_value, _picco_tmp1, 32, 32, 32, "int", -1);
  __s->smc_set(_picco_tmp1, g_priv_int_without_value, 32, 32, "int", -1);

  g_pub_int_with_value = 888;

  g_pub_int_without_value = 999;

  g_pub_int_with_value = (((5 + 5) - 7) * 3) / 1;

  g_pub_int_without_value = (((2 + 1) - 2) * 6) / 1;

  g_pub_int_without_value = (g_pub_int_with_value + g_pub_int_with_value);

  __s->smc_set(1.4, g_priv_float_arr[0], -1, -1, 32, 9, "float", -1);

  __s->smc_set(4.2, g_priv_float_arr[1], -1, -1, 32, 9, "float", -1);

  __s->smc_set(4.43, g_priv_float_arr[2], -1, -1, 32, 9, "float", -1);

  g_pub_float_arr[0] = 1.1;

  g_pub_float_arr[1] = 2.2;

  g_pub_float_arr[2] = 3.3;

  g_pub_float_arr[3] = 4.4;

  __s->smc_set(1.11, g_reg_float_arr[0], -1, -1, 32, 9, "float", -1);

  __s->smc_set(2.22, g_reg_float_arr[1], -1, -1, 32, 9, "float", -1);

  __s->smc_set(5.4, non_g_priv_float_arr[0], -1, -1, 32, 9, "float", -1);

  __s->smc_set(5.3, non_g_priv_float_arr[1], -1, -1, 32, 9, "float", -1);

  non_g_pub_float_arr[0] = 7.4;

  non_g_pub_float_arr[1] = 6.3;

  non_g_pub_float_arr[2] = 5.3;

  non_g_pub_float_arr[3] = 4.2;

  non_g_pub_float_arr[4] = 3.2;

  __s->smc_set(9.3, non_g_reg_float_arr[0], -1, -1, 32, 9, "float", -1);

  __s->smc_set(8.3, non_g_reg_float_arr[1], -1, -1, 32, 9, "float", -1);

  __s->smc_set(7.3, non_g_reg_float_arr[2], -1, -1, 32, 9, "float", -1);

  __s->smc_set(9.8, g_priv_float_with_value, -1, -1, 32, 9, "float", -1);

  __s->smc_set(9.9, g_priv_float_without_value, -1, -1, 32, 9, "float", -1);

  __s->smc_set(((((4.4 + 1.3) * 3.3) - 3.33)) / 1.3, g_priv_float_with_value, -1, -1, 32, 9, "float", -1);

  __s->smc_set(((((1.1 + 12.2) * 32.3) - 4.44)) / 1.3, g_priv_float_without_value, -1, -1, 32, 9, "float", -1);

  __s->smc_add(g_priv_float_with_value, g_priv_float_with_value, _picco_ftmp1, 32, 9, 32, 9, 32, 9, "float", -1);
  __s->smc_set(_picco_ftmp1, g_priv_float_without_value, 32, 9, 32, 9, "float", -1);

  g_pub_float_with_value = 8.88;

  g_pub_float_without_value = 99.9;

  g_pub_float_with_value = (((5.5 + 5.4) - 7.4) * 3.4) / 1.4;

  g_pub_float_without_value = (((2.3 + 1.3) - 2.3) * 6.3) / 1.3;

  g_pub_float_without_value = (g_pub_float_with_value + g_pub_float_with_value);

  __s->smc_output(1, &g_priv_int_without_value, "int", -1);

  __s->smc_output(1, &g_pub_int_without_value, "int", -1);

  __s->smc_output(1, &g_reg_int_without_value, "int", -1);

  __s->smc_output(1, g_priv_int_arr, 3, "int", -1);

  __s->smc_output(1, g_pub_int_arr, 4, "int", -1);

  __s->smc_output(1, g_reg_int_arr, 2, "int", -1);

  __s->smc_output(1, &g_priv_float_without_value, "float", -1);

  __s->smc_output(1, &g_pub_float_without_value, "float", -1);

  __s->smc_output(1, &g_reg_float_without_value, "float", -1);

  __s->smc_output(1, g_priv_float_arr, 3, "float", -1);

  __s->smc_output(1, g_pub_float_arr, 4, "float", -1);

  __s->smc_output(1, g_reg_float_arr, 2, "float", -1);

    __s->smc_free_ptr(&(reg_ptr_int_without_value));
  __s->smc_free_ptr(&(reg_ptr_int_with_value2));
  ss_clear(reg_ptr_int_var2);
  __s->smc_free_ptr(&(reg_ptr_int_with_value));
  ss_clear(reg_ptr_int_var);
  __s->smc_free_ptr(&(non_g_priv_ptr_int_without_value));
  __s->smc_free_ptr(&(non_g_priv_ptr_int_with_value2));
  ss_clear(non_ptr_int_var2);
  __s->smc_free_ptr(&(non_g_priv_ptr_int_with_value));
  ss_clear(priv_ptr_int_var);
    for (int _picco_i = 0; _picco_i < 4; _picco_i++)
        ss_clear(non_g_reg_float_arithmetic[_picco_i]);
  free(non_g_reg_float_arithmetic);
    for (int _picco_i = 0; _picco_i < 4; _picco_i++)
        ss_clear(non_g_reg_priv_float_arithmetic[_picco_i]);
  free(non_g_reg_priv_float_arithmetic);
  for (int _picco_i = 0; _picco_i < 3; _picco_i++)
    {
      for (int _picco_j = 0; _picco_j < 4; _picco_j++)
            ss_clear(non_g_reg_float_arr[_picco_i][_picco_j]);
    free(non_g_reg_float_arr[_picco_i]);
    }
    free(non_g_reg_float_arr);
    for (int _picco_i = 0; _picco_i < 2; _picco_i++)
      {
        for (int _picco_j = 0; _picco_j < 4; _picco_j++)
                ss_clear(non_g_priv_float_arr[_picco_i][_picco_j]);
      free(non_g_priv_float_arr[_picco_i]);
      }
      free(non_g_priv_float_arr);
            for (int _picco_i = 0; _picco_i < 4; _picco_i++)
                ss_clear(non_g_priv_float_without_value[_picco_i]);
      free(non_g_priv_float_without_value);
            for (int _picco_i = 0; _picco_i < 4; _picco_i++)
                ss_clear(non_g_priv_float_with_value[_picco_i]);
      free(non_g_priv_float_with_value);
            for (int _picco_i = 0; _picco_i < 4; _picco_i++)
                ss_clear(non_g_reg_float_without_value[_picco_i]);
      free(non_g_reg_float_without_value);
            for (int _picco_i = 0; _picco_i < 4; _picco_i++)
                ss_clear(non_g_reg_float_with_value[_picco_i]);
      free(non_g_reg_float_with_value);
      ss_clear(non_g_reg_int_arithmetic);
      ss_clear(non_g_reg_priv_int_arithmetic);
      for (int _picco_i = 0; _picco_i < 3; _picco_i++)
                ss_clear(non_g_reg_int_arr[_picco_i]);
      free(non_g_reg_int_arr);
      for (int _picco_i = 0; _picco_i < 2; _picco_i++)
                ss_clear(non_g_priv_int_arr[_picco_i]);
      free(non_g_priv_int_arr);
      ss_clear(non_g_priv_int_without_value);
      ss_clear(non_g_priv_int_with_value);
      ss_clear(non_g_reg_int_without_value);
      ss_clear(non_g_reg_int_with_value);
      ss_clear(_picco_tmp1);
      for(int _picco_j = 0; _picco_j < 4; _picco_j++)
        ss_clear(_picco_ftmp1[_picco_j]);
      free(_picco_ftmp1);
      ss_clear(_picco_priv_tmp1);
      ss_clear(_picco_priv_tmp2);
      __s->smc_free_ptr(&_picco_tmp_int_ptr1);
      __s->smc_free_ptr(&_picco_tmp_int_ptr2);
      return (0);
    }



/* smc-compiler generated main() */
int main(int argc, char **argv) {

if(argc < 8){
  fprintf(stderr,"Incorrect input parameters\n");
  fprintf(stderr,"Usage: <id> <runtime-config> <privatekey-filename> <number-of-input-parties> <number-of-output-parties> <input-share> <output>\n");
  exit(1);
}

std::string IO_files[atoi(argv[4]) + atoi(argv[5])];
for(int i = 0; i < argc-6; i++)
   IO_files[i] = argv[6+i];

__s = new SMC_Utils(atoi(argv[1]), argv[2], argv[3], atoi(argv[4]), atoi(argv[5]), IO_files, 3, 1, 116, "41538374868278621028243970633760839", seed_map, 1);

 struct timeval tv1;
 struct timeval tv2;
 int _xval = 0;

 gettimeofday(&tv1,NULL);


ss_init(g_priv_int_with_value);
__s->smc_set(11, g_priv_int_with_value, -1, 32, "int", -1);

ss_init(g_priv_int_without_value);
ss_init(g_reg_int_with_value);
__s->smc_set(33, g_reg_int_with_value, -1, 32, "int", -1);

ss_init(g_reg_int_without_value);
ss_init(g_priv_int_arithmetic);
__s->smc_set((1 + 2), g_priv_int_arithmetic, -1, 32, "int", -1);

ss_init(g_reg_int_arithmetic);
__s->smc_set((5 + 6), g_reg_int_arithmetic, -1, 32, "int", -1);

g_priv_float_with_value = (priv_int*)malloc(sizeof(priv_int) * (4));
for (int _picco_i = 0; _picco_i < 4; _picco_i++)
    ss_init(g_priv_float_with_value[_picco_i]);
__s->smc_set(1.1, g_priv_float_with_value, -1, -1, 32, 9, "float", -1);

g_priv_float_without_value = (priv_int*)malloc(sizeof(priv_int) * (4));
for (int _picco_i = 0; _picco_i < 4; _picco_i++)
    ss_init(g_priv_float_without_value[_picco_i]);
g_reg_float_with_value = (priv_int*)malloc(sizeof(priv_int) * (4));
for (int _picco_i = 0; _picco_i < 4; _picco_i++)
    ss_init(g_reg_float_with_value[_picco_i]);
__s->smc_set(3.3, g_reg_float_with_value, -1, -1, 32, 9, "float", -1);

g_reg_float_without_value = (priv_int*)malloc(sizeof(priv_int) * (4));
for (int _picco_i = 0; _picco_i < 4; _picco_i++)
    ss_init(g_reg_float_without_value[_picco_i]);
g_priv_float_arithmetic = (priv_int*)malloc(sizeof(priv_int) * (4));
for (int _picco_i = 0; _picco_i < 4; _picco_i++)
    ss_init(g_priv_float_arithmetic[_picco_i]);
__s->smc_set((1.6 + 2.6), g_priv_float_arithmetic, -1, -1, 32, 9, "float", -1);

g_reg_float_arithmetic = (priv_int*)malloc(sizeof(priv_int) * (4));
for (int _picco_i = 0; _picco_i < 4; _picco_i++)
    ss_init(g_reg_float_arithmetic[_picco_i]);
__s->smc_set((5.7 + 68.4), g_reg_float_arithmetic, -1, -1, 32, 9, "float", -1);

ss_init(g_priv_ptr_int_var);
__s->smc_set(45, g_priv_ptr_int_var, -1, 32, "int", -1);

priv_ptr g_priv_ptr_int_with_value = __s->smc_new_ptr(1, 0);
__s->smc_set_int_ptr(g_priv_ptr_int_with_value, &g_priv_ptr_int_var, "int", -1);

ss_init(g_non_ptr_int_var2);
__s->smc_set(11, g_non_ptr_int_var2, -1, 32, "int", -1);

priv_ptr g_priv_ptr_int_with_value2 = __s->smc_new_ptr(1, 0);
__s->smc_set_int_ptr(g_priv_ptr_int_with_value2, &g_non_ptr_int_var2, "int", -1);

priv_ptr g_priv_ptr_int_without_value = __s->smc_new_ptr(1, 0);
ss_init(g_reg_ptr_int_var);
__s->smc_set(44, g_reg_ptr_int_var, -1, 32, "int", -1);

priv_ptr g_reg_ptr_int_with_value = __s->smc_new_ptr(1, 0);
__s->smc_set_int_ptr(g_reg_ptr_int_with_value, &g_reg_ptr_int_var, "int", -1);

ss_init(g_reg_ptr_int_var2);
__s->smc_set(32, g_reg_ptr_int_var2, -1, 32, "int", -1);

priv_ptr g_reg_ptr_int_with_value2 = __s->smc_new_ptr(1, 0);
__s->smc_set_int_ptr(g_reg_ptr_int_with_value2, &g_reg_ptr_int_var2, "int", -1);

priv_ptr g_reg_ptr_int_without_value = __s->smc_new_ptr(1, 0);


  _xval = (int) __original_main(argc, argv);
  gettimeofday(&tv2, NULL);
  std::cout << "Time: " << __s->time_diff(&tv1,&tv2) << " seconds "<< std::endl;
  return (_xval);
}

