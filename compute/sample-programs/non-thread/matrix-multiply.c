
public int main() {
   public int S=2; // size of matrices 
   int A[S][S];
   int B[S][S];
   int C[S][S];
   /*******************/
   public int i, j;

   smcinput(A, 1, S, S);
   smcinput(B, 1, S, S);
   
   for (i = 0; i < S; i++)[
     for (j = 0; j < S; j++)[
	 C[i][j] = A[i] @ B[j];
     ]
   ]

    smcoutput(C, 1, S, S);

   return 0;
}
