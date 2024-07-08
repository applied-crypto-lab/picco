/* File generated from [sample-programs/non-thread/array-operations.c] by PICCO Mon Jul  8 10:35:59 2024
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


std::vector<int> seed_map = {7, 11};

int K = 10, K1 = 2, K2 = 5;


int  __original_main(int _argc_ignored, char **_argv_ignored)
{
  
  priv_int _picco_tmp1, _picco_tmp2, _picco_tmp3;
  ss_init(_picco_tmp1);
  ss_init(_picco_tmp2);
  ss_init(_picco_tmp3);

  priv_int* _picco_ftmp1 = (priv_int*)malloc(sizeof(priv_int) * 4);
  for(int _picco_j = 0; _picco_j < 4; _picco_j++)
    ss_init(_picco_ftmp1[_picco_j]);
  void* _picco_temp_;
 int i, j;

  priv_int* A; 
  A = (priv_int*)malloc(sizeof(priv_int) * (K));
  for (int _picco_i = 0; _picco_i < K; _picco_i++)
        ss_init(A[_picco_i]);
    priv_int* B; 
    B = (priv_int*)malloc(sizeof(priv_int) * (K));
    for (int _picco_i = 0; _picco_i < K; _picco_i++)
            ss_init(B[_picco_i]);

      priv_int** C; 
      C = (priv_int**)malloc(sizeof(priv_int*) * (K));
      for (int _picco_i = 0; _picco_i < K; _picco_i++)
        {
          C[_picco_i] = (priv_int*)malloc(sizeof(priv_int) * (4));
          for (int _picco_j = 0; _picco_j < 4; _picco_j++)
                    ss_init(C[_picco_i][_picco_j]);
        }
        priv_int** D; 
        D = (priv_int**)malloc(sizeof(priv_int*) * (K));
        for (int _picco_i = 0; _picco_i < K; _picco_i++)
          {
            D[_picco_i] = (priv_int*)malloc(sizeof(priv_int) * (4));
            for (int _picco_j = 0; _picco_j < 4; _picco_j++)
                        ss_init(D[_picco_i][_picco_j]);
          }

          __s->smc_input(1, A, K, "int", -1);

          __s->smc_input(1, C, K, "float", -1);

          __s->smc_mult(A, A, 32, 32, A, 32, K, "int", -1);

          __s->smc_output(1, A, K, "int", -1);

          __s->smc_add(A, A, 32, 32, A, 32, K, "int", -1);

          __s->smc_output(1, A, K, "int", -1);

          __s->smc_eqeq(A, A, 32, 32, B, 32, K, "int", -1);

          __s->smc_output(1, B, K, "int", -1);

          __s->smc_eqeq(C, C, 32, 9, 32, 9, B, 32, K, "float", -1);

          __s->smc_output(1, B, K, "int", -1);

          __s->smc_lt(C, C, 32, 9, 32, 9, B, 32, K, "float", -1);

          __s->smc_output(1, B, K, "int", -1);

          __s->smc_add(C, C, 32, 9, 32, 9, D, 32, 9, K, "float", -1);

          __s->smc_output(1, D, K, "float", -1);

          priv_int a;
          ss_init(a);

          __s->smc_dot(A, B, K, a, -1);

          __s->smc_output(1, &a, "int", -1);

          priv_int** E; 
          E = (priv_int**)malloc(sizeof(priv_int*) * (K1));
          for (int _picco_i = 0; _picco_i < K1; _picco_i++)
            {
              E[_picco_i] = (priv_int*)malloc(sizeof(priv_int) * (K2));
              for (int _picco_j = 0; _picco_j < K2; _picco_j++)
                            ss_init(E[_picco_i][_picco_j]);
            }

            i = 0;
            for ( ;i < K1; )
            {
              j = 0;
              for ( ;j < K2; )
              {
                                __s->smc_set(A[i * K1 + j], E[i][j], 32, 32, "int", -1);
                j++;
              }
              i++;
            }

            __s->smc_lt(A[0], 100, _picco_tmp1, 32, -1, 1, "int", -1);
              {
                priv_int _picco_condtmp1;
                ss_init(_picco_condtmp1);
                ss_set(_picco_condtmp1, _picco_tmp1);
                {
                  {
                    int _picco_batch_counter1 = 0;
                    int _picco_ind1 = 0;
                                        for (i = 0; i < K1; i++)
                      {
                        ++_picco_batch_counter1;
                      }
                    int* _picco_batch_index_array2 = (int*)malloc(sizeof(int) * 3 * _picco_batch_counter1);
                    int* _picco_batch_index_array1 = (int*)malloc(sizeof(int) * 3 * _picco_batch_counter1);
                    priv_int* _picco_batch_array1 = (priv_int*)malloc(sizeof(priv_int) * _picco_batch_counter1);
                    for (int _picco_i = 0; _picco_i < _picco_batch_counter1; _picco_i++)
                    {
                      ss_init(_picco_batch_array1[_picco_i]);
                    }
                    int* _picco_batch_tmp_array1 = (int*)malloc(sizeof(int) * _picco_batch_counter1);
                    for (i = 0; i < K1; i++)
                      {
                        _picco_batch_tmp_array1[_picco_ind1] = 100;
                        _picco_batch_index_array1[3*_picco_ind1+1] = _picco_ind1;
                        _picco_batch_index_array1[3*_picco_ind1] = i;
                        _picco_batch_index_array1[3*_picco_ind1+1] = _picco_ind1;
                        _picco_batch_index_array1[3*_picco_ind1+2] = _picco_ind1;
                        _picco_batch_index_array2[3*_picco_ind1] = i;
                        _picco_batch_index_array2[3*_picco_ind1+1] = i;
                        _picco_batch_index_array2[3*_picco_ind1+2] = i;
                        _picco_ind1++;
                      }
                    __s->smc_batch( A ,  _picco_batch_tmp_array1 , _picco_batch_array1, 32, -1, -1, K, 0, 0, _picco_condtmp1, NULL, -1, _picco_batch_index_array1, _picco_batch_counter1, "<", "int", -1); 
                    __s->smc_batch(E, E, B, 32, 32, 32, K2, K2, K, NULL, _picco_batch_array1, _picco_batch_counter1, _picco_batch_index_array2, _picco_batch_counter1, "@", "int", -1);

                    free(_picco_batch_index_array2);
                    free(_picco_batch_index_array1);
                    for (int _picco_j = 0; _picco_j < _picco_batch_counter1; _picco_j++)
                    {
                      ss_clear(_picco_batch_array1[_picco_j]);
                    }
                    free(_picco_batch_tmp_array1);
                  }
                }
                ss_clear(_picco_condtmp1);
              }

            __s->smc_output(1, B, K2, "int", -1);

            

            for (int _picco_i = 0; _picco_i < K1; _picco_i++)
              {
                for (int _picco_j = 0; _picco_j < K2; _picco_j++)
                                  ss_clear(E[_picco_i][_picco_j]);
                free(E[_picco_i]);
              }
            free(E);
            

ss_clear(a);
            for (int _picco_i = 0; _picco_i < K; _picco_i++)
                {
                    for (int _picco_j = 0; _picco_j < 4; _picco_j++)
                                        ss_clear(D[_picco_i][_picco_j]);
                free(D[_picco_i]);
                }
                free(D);
              for (int _picco_i = 0; _picco_i < K; _picco_i++)
                  {
                      for (int _picco_j = 0; _picco_j < 4; _picco_j++)
                                            ss_clear(C[_picco_i][_picco_j]);
                  free(C[_picco_i]);
                  }
                  free(C);


                for (int _picco_i = 0; _picco_i < K; _picco_i++)
                                      ss_clear(B[_picco_i]);
                  free(B);


                  for (int _picco_i = 0; _picco_i < K; _picco_i++)
                                          ss_clear(A[_picco_i]);
                    free(A);
                    ss_clear(_picco_tmp1);
                    ss_clear(_picco_tmp2);
                    ss_clear(_picco_tmp3);
                    for(int _picco_j = 0; _picco_j < 4; _picco_j++)
                      ss_clear(_picco_ftmp1[_picco_j]);
                    free(_picco_ftmp1);
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

__s = new SMC_Utils(atoi(argv[1]), argv[2], argv[3], atoi(argv[4]), atoi(argv[5]), IO_files, 5, 2, 118, "166153499473114484112975882535043107", seed_map, 1);

 struct timeval tv1;
 struct timeval tv2;
 int _xval = 0;

 gettimeofday(&tv1,NULL);




  _xval = (int) __original_main(argc, argv);
  gettimeofday(&tv2, NULL);
  std::cout << "Time: " << __s->time_diff(&tv1,&tv2) << " seconds "<< std::endl;
  return (_xval);
}

