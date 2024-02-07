public int main() {
   
   public int S = 4;
   private int<1> A[S];
   private int<1> B[S];

   public int<1> C[S]; 

   smcinput(A, 1, S); 
   smcinput(B, 1, S); 
   smcinput(C, 1, S); 

   public int i; 
   private int D[S];
   private int E[S];
   
   D = A == B; 
   E = A == C; 

   smcoutput(D, 1, S);
   smcoutput(E, 1, S);

   return 0;
}
