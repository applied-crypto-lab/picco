public int K=50; // length of array 

public void swap(int* A, int* B)
{
        private int temp;
        if (*A > *B) {
            temp = *A;
            *A = *B; 
            *B = temp;
         }
}
       
public int main() {
   public int i, j;
   private int A[K];
   private int* temp1; 
   private int* temp2; 
   for (i = 0; i < K; i++)
     smcinput(A[i], 1);
   
   for (i = K - 1; i > 0; i--) { 
      for (j = 0; j < i; j++) { 
	temp1 = &A[j]; 
 	temp2 = &A[j+1]; 	
	swap(temp1, temp2); 
      }
   }

   
   return 0;
}

