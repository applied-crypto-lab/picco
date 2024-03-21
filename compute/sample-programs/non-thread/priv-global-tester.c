/** 
 * By: Zuhra Masoud
 * Tests after adding support for private global variables 
**/

/*************************************************
*                                                *
*               Integer                          *
*                                                *
*                                                *
*************************************************/
// Test global private int
private int g_priv_int_with_value = 11;
private int g_priv_int_without_value;

// Test global public int
public int g_pub_int_with_value = 22;
public int g_pub_int_without_value;

// Test regular variables global, non private, non public int
int g_reg_int_with_value = 33;
int g_reg_int_without_value;

// Test global private array of int
private int g_priv_int_arr[3];

// Test global public array of int
public int g_pub_int_arr[4];

// Test regular global array of int
int g_reg_int_arr[2];

// Test global private arithmetic int init
private int g_priv_int_arithmetic = (1+2);

// Test global public arithmetic int init
public int g_pub_int_arithmetic = (3+4);

// Test regular global arithmetic int init
int g_reg_int_arithmetic = (5+6);


/*************************************************
*                                                *
*               Float                            *
*                                                *
*                                                *
*************************************************/
// Test global private float
private float g_priv_float_with_value = 1.1;
private float g_priv_float_without_value;

// Test global public float
public float g_pub_float_with_value = 2.2;
public float g_pub_float_without_value;

// Test regular variables global, non private, non public float
float g_reg_float_with_value = 3.3;
float g_reg_float_without_value;

// Test global private array of float
private float g_priv_float_arr[3];

// Test global public array of float
public float g_pub_float_arr[4];

// Test regular global array of float
float g_reg_float_arr[2];

// Test global private arithmetic float init
private float g_priv_float_arithmetic = (1.6+2.6);

// Test global public arithmetic float init
public float g_pub_float_arithmetic = (3.7+4.7);

// Test regular global arithmetic float init
float g_reg_float_arithmetic = (5.7+68.4);


/*************************************************
*                                                *
*               Pointer                          *
*                                                *
*                                                *
*************************************************/
// Test global private pointer 
private int g_priv_ptr_int_var = 45;
private int* g_priv_ptr_int_with_value = &g_priv_ptr_int_var; 

int g_non_ptr_int_var2 = 11;
private int* g_priv_ptr_int_with_value2 = &g_non_ptr_int_var2; 

private int* g_priv_ptr_int_without_value; 

// Test global public pointer 
public int g_pub_ptr_int_var = 33;
public int* g_pub_ptr_int_with_value = &g_pub_ptr_int_var; 

// int g_pub_ptr_int_var2 = 22; -> can't do this cause this is a Security type mismatch in assignment.
// public int* g_pub_ptr_int_with_value2 = &g_pub_ptr_int_var2; 

public int* g_pub_ptr_int_without_value; 
 
// Test global regular pointer
int g_reg_ptr_int_var = 44;
int* g_reg_ptr_int_with_value = &g_reg_ptr_int_var; 

int g_reg_ptr_int_var2 = 32;
int* g_reg_ptr_int_with_value2 = &g_reg_ptr_int_var2; 

int* g_reg_ptr_int_without_value; 


// Other
// This does not compile in c 
// private float SUM = ((((R+O)-R)*O)/R);


// This function is a similar copy of bubble-sort.c, it is here to test pointers 
// public int K=50; // length of array 
// public int i, j;
// private int A[K];
// private int* temp1; 
// private int* temp2; 

// public void swap(int* A, int* B)
// {
//         private int temp;
//         if (*A > *B) {
//             temp = *A;
//             *A = *B; 
//             *B = temp;
//          }
// }

public int main() {

  // This is a similar copy of bubble-sort.c, it is here to test pointers 
  // smcinput(A, 1, K);
  // for (i = K - 1; i > 0; i--) { 
  //   for (j = 0; j < i; j++) { 
	// temp1 = &A[j]; 
 	// temp2 = &A[j+1]; 	
	// swap(temp1, temp2); 
  //     }
  // }
  // smcoutput(A, 1, K);

  /*************************************************
  *                                                *
  *               Integer                          *
  *                                                *
  *                                                *
  *************************************************/
  // Test regular variables non global, non private, non public int
  int non_g_reg_int_with_value = 44;
  int non_g_reg_int_without_value;

  // Test private non global int 
  private int non_g_priv_int_with_value = 55;
  private int non_g_priv_int_without_value;

  // Test public non global int 
  public int non_g_pub_int_with_value = 55;
  public int non_g_pub_int_without_value;

  // Test non global private array of int
  private int non_g_priv_int_arr[2];

  // Test non global public array of int
  public int non_g_pub_int_arr[5];

  // Test regular non global array of int
  int non_g_reg_int_arr[3];

  // Test non global private arithmetic int init
  private int non_g_reg_priv_int_arithmetic = (7+8);

  // Test non global public arithmetic int init
  public int non_g_reg_pub_int_arithmetic = (9+10);

  // Test regular non global arithmetic int init
  int non_g_reg_int_arithmetic = (11+12);


  /*************************************************
  *                                                *
  *               Float                            *
  *                                                *
  *                                                *
  *************************************************/
  // Test regular variables non global, non private, non public float
  float non_g_reg_float_with_value = 4.4;
  float non_g_reg_float_without_value;

  // Test private non global float 
  private float non_g_priv_float_with_value = 5.5;
  private float non_g_priv_float_without_value;

  // Test public non global float 
  public float non_g_pub_float_with_value = 5.5;
  public float non_g_pub_float_without_value;

  // Test non global private array of float
  private float non_g_priv_float_arr[2];

  // Test non global public array of float
  public float non_g_pub_float_arr[5];

  // Test regular non global array of float
  float non_g_reg_float_arr[3];

  // Test non global private arithmetic float init
  private float non_g_reg_priv_float_arithmetic = (7.5+8.7);

  // Test non global public arithmetic float init
  public float non_g_reg_pub_float_arithmetic = (96.6+10.8);

  // Test regular non global arithmetic float init
  float non_g_reg_float_arithmetic = (11.4+12.5);


  /*************************************************
  *                                                *
  *               Pointer                          *
  *                                                *
  *                                                *
  *************************************************/
  // Test non global private pointer 
  private int priv_ptr_int_var = 12;
  private int* non_g_priv_ptr_int_with_value = &priv_ptr_int_var; 

  int non_ptr_int_var2 = 12;
  private int* non_g_priv_ptr_int_with_value2 = &non_ptr_int_var2; 

  private int* non_g_priv_ptr_int_without_value; 

  // Test non global public pointer 
  public int pub_ptr_int_var = 12;
  public int* non_g_pub_ptr_int_with_value = &pub_ptr_int_var; 

  // int pub_ptr_int_var2 = 12;
  // public int* non_g_pub_ptr_int_with_value2 = &pub_ptr_int_var2; -> can't do this cause this is a Security type mismatch in assignment.
  
  public int* non_g_pub_ptr_int_without_value; 

  // Test global regular pointer
  int reg_ptr_int_var = 44;
  int* reg_ptr_int_with_value = &reg_ptr_int_var; 

  int reg_ptr_int_var2 = 32;
  int* reg_ptr_int_with_value2 = &reg_ptr_int_var2; 

  int* reg_ptr_int_without_value; 


  // Test using smc_input for all the ints
  smcinput(g_priv_int_without_value, 1);
  smcinput(g_pub_int_without_value, 1);
  smcinput(g_reg_int_without_value, 1);
  smcinput(g_priv_int_arr, 1, 3);
  smcinput(g_pub_int_arr, 1, 4);
  smcinput(g_reg_int_arr, 1, 2);

  // Test using smc_input for all the floats
  smcinput(g_priv_float_without_value, 1);
  smcinput(g_pub_float_without_value, 1);
  smcinput(g_reg_float_without_value, 1);
  smcinput(g_priv_float_arr, 1, 3);
  smcinput(g_pub_float_arr, 1, 4);
  smcinput(g_reg_float_arr, 1, 2);

  // Ints
  // Test assigning a value to a global private array int
  g_priv_int_arr[0] = 1;
  g_priv_int_arr[1] = 2; 
  g_priv_int_arr[2] = 3;

  // Test assigning a value to a global public array int
  g_pub_int_arr[0] = 11;
  g_pub_int_arr[1] = 22; 
  g_pub_int_arr[2] = 33;
  g_pub_int_arr[3] = 44;

  // Test assigning a value to a regular global array int
  g_reg_int_arr[0] = 111;
  g_reg_int_arr[1] = 222; 

  // Test assigning a value to a non global private array int
  non_g_priv_int_arr[0] = 54;
  non_g_priv_int_arr[1] = 53; 

  // Test assigning a value to a non global public array int
  non_g_pub_int_arr[0] = 7;
  non_g_pub_int_arr[1] = 6;   
  non_g_pub_int_arr[2] = 5;
  non_g_pub_int_arr[3] = 4; 
  non_g_pub_int_arr[4] = 3;

  // Test assigning a value to a regular non global array int
  non_g_reg_int_arr[0] = 9;
  non_g_reg_int_arr[1] = 8;   
  non_g_reg_int_arr[2] = 7;

  // Test assigning a value to a global private number int
  g_priv_int_with_value = 98;
  g_priv_int_without_value = 99;

  // Test assigning a arithmetic to a global private number int
  g_priv_int_with_value = ((((44+1)*33)-333))/1;
  g_priv_int_without_value = ((((11+122)*323)-444))/1;

  // Test assigning a arithmetic of two private values to a global private number int
  g_priv_int_without_value = (g_priv_int_with_value+g_priv_int_with_value);

  // Test assigning a value to a global public number int
  g_pub_int_with_value = 888;
  g_pub_int_without_value = 999;

  // Test assigning a arithmetic to a global public number int
  g_pub_int_with_value = (((5+5)-7)*3)/1;
  g_pub_int_without_value = (((2+1)-2)*6)/1;

  // Test assigning a arithmetic of two public values to a global public number int
  g_pub_int_without_value = (g_pub_int_with_value+g_pub_int_with_value);

  // Floats 
  // Test assigning a value to a global private array float
  g_priv_float_arr[0] = 1.4;
  g_priv_float_arr[1] = 4.2; 
  g_priv_float_arr[2] = 4.43;

  // Test assigning a value to a global public array float
  g_pub_float_arr[0] = 1.1;
  g_pub_float_arr[1] = 2.2; 
  g_pub_float_arr[2] = 3.3;
  g_pub_float_arr[3] = 4.4;

  // Test assigning a value to a regular global array float
  g_reg_float_arr[0] = 1.11;
  g_reg_float_arr[1] = 2.22; 

  // Test assigning a value to a non global private array float
  non_g_priv_float_arr[0] = 5.4;
  non_g_priv_float_arr[1] = 5.3; 

  // Test assigning a value to a non global public array float
  non_g_pub_float_arr[0] = 7.4;
  non_g_pub_float_arr[1] = 6.3;   
  non_g_pub_float_arr[2] = 5.3;
  non_g_pub_float_arr[3] = 4.2; 
  non_g_pub_float_arr[4] = 3.2;

  // Test assigning a value to a regular non global array float
  non_g_reg_float_arr[0] = 9.3;
  non_g_reg_float_arr[1] = 8.3;   
  non_g_reg_float_arr[2] = 7.3;

  // Test assigning a value to a global private number float
  g_priv_float_with_value = 9.8;
  g_priv_float_without_value = 9.9;

  // Test assigning a arithmetic to a global private number float
  g_priv_float_with_value = ((((4.4+1.3)*3.3)-3.33))/1.3;
  g_priv_float_without_value = ((((1.1+12.2)*32.3)-4.44))/1.3;

  // Test assigning a arithmetic of two private values to a global private number float
  g_priv_float_without_value = (g_priv_float_with_value+g_priv_float_with_value);

  // Test assigning a value to a global public number float
  g_pub_float_with_value = 8.88;
  g_pub_float_without_value = 99.9;

  // Test assigning a arithmetic to a global public number float
  g_pub_float_with_value = (((5.5+5.4)-7.4)*3.4)/1.4;
  g_pub_float_without_value = (((2.3+1.3)-2.3)*6.3)/1.3;

  // Test assigning a arithmetic of two public values to a global public number float
 g_pub_float_without_value = (g_pub_float_with_value+g_pub_float_with_value);


  // Test using smc_output for all the ints 
  smcoutput(g_priv_int_without_value, 1);
  smcoutput(g_pub_int_without_value, 1);
  smcoutput(g_reg_int_without_value, 1);
  smcoutput(g_priv_int_arr, 1, 3);
  smcoutput(g_pub_int_arr, 1, 4);
  smcoutput(g_reg_int_arr, 1, 2);

  // Test using smc_output for all the floats 
  smcoutput(g_priv_float_without_value, 1);
  smcoutput(g_pub_float_without_value, 1);
  smcoutput(g_reg_float_without_value, 1);
  smcoutput(g_priv_float_arr, 1, 3);
  smcoutput(g_pub_float_arr, 1, 4);
  smcoutput(g_reg_float_arr, 1, 2);

  return 0;
}


// Mergesort from page 42 of the paper figure 4 pointer
// Figure 4: Mergesort median program with pointers.
// public int K = 128;
// private int A[K];
// void mergesort(public int l, public int r) {
//   public int i, j, k, m, size;
//   size = r - l + 1;
//   int tmp[size];

//   if (r > l) {
//     m = (r + l) / 2;
//     [ mergesort(l, m); ]
//     [ mergesort(m + 1, r); ] 
    
//     for (i = size >> 1; i > 0; i = i >> 1) 
//       for (j = 0; j < size; j += 2 * i) [
//         for (k = j; k < j + i; k++) [
//           tmp[k] = A[k + l];
//           if (A[k+l] > A[k+i+l]) {
//             A[k+l] = A[k+i+l];
//             A[k+i+l] = tmp[k];
//           }
//         ] 
//       ]
    
//   }
// }

// public int main() {
//   public int median = K / 2;
//   smcinput(A, 1, K);
//   mergesort(0, K - 1);
//   smcoutput(A[median], 1);
//   return 0;
// }


// Mergesort from page 43 of the paper figure 5 non pointer
// Figure 5: Mergesort median program without pointers.
// public int K = 128;
// private int A[K];
// void mergesort(public int l, public int r) {
//   public int i, j, k, m, size;
//   size = r - l + 1;
//   int tmp[size];
//   if (r > l){
//     m = (r + l) / 2;
//     [mergesort(l, m);]
//     [mergesort(m + 1, r);] 
//     for (i = size >> 1; i > 0; i = i >> 1) 
//       for (j = 0; j < size; j += 2 * i)[
//         for (k = j; k < j + i; k++)[
//           tmp[k] = A[k + l];
//           if (A[k + l] > A[k + i + l]) {
//             A[k + l] = A[k + i + l];
//             A[k + i + l] = tmp[k];
//           }
//         ]
//       ]
    
//   }
// }
// public int main() {
//   public int median = K/2;
//   smcinput(A, 1, K);
//   mergesort(0, K-1);
//   smcoutput(A[median], 1);
//   return 0;
// }
