// THis program is created to test new picco functionalities 

void main() {

// Array initilization in one line 
int a[3] = {1, 2, 3};
smcoutput(a, 1, 3);

// Casting 
int a = 2;
float b = 4.5;
float c = (float)a;
int d = (int)b;

// Auto casting 
float e = a;
int f = b;

// Casting arrays 
int A[3]; 
float B[3];
A = (int)B;
B = (float)A;
 
// Array auto casting
A = B;
B = A;

// More than one batch operator used in an expression 
int C[3], D[3], E[3];
public int i = 0;
for (i = 0; i < 3; i++) {
    A[i] = A[i] + B[i] * C[i] - D[i] / A[i] + E[i];
    A[i] = A[i] + B[i] * C[i] - D[i] / A[i] * E[i];
}
// old and new 
for (i = 0; i < 3; i++) {
    A[i] = A[i] + B[i] * C[i] - D[i] / A[i] + E[i];
}
// old
for (i = 0; i < 3; i++) [
    A[i] = A[i] + B[i];
    C[i] = A[i] + B[i];
    D[i] = A[i] + B[i];
    E[i] = A[i] + B[i];
    // I can have conditionals
    // Anything malloc, operations on non arrays, creating new variables, call a function
]

// Dynamic memory allocation 


// Logical operators in arrays 


// Bitwise operators in arrays 


// Pointer Casting 


// Pointer to function 


// Accessing array elements 


}