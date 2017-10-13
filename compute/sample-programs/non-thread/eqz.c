public int main() {
   
   public int S = 100, SS = 10000; // SS = S*S
   private int A[S]; 
   public int B[S]; 
   smcinput(A, 1, S); 
   smcinput(B, 2, S); 

   public int i, j; 
   private int<1> C[S*S], D[S*S];
   
   for (i = 0; i < S; i++) {
     for (j = 0; j < S; j++) {
       C[i*S+j] = A[i] == A[j];
       D[i*S+j] = A[i] == B[j];
     }
   }

   smcoutput(C, 1, SS);
   smcoutput(D, 1, SS);
   
   return 0;
}
