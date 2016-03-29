public int T=5; 
public int main()
{
	public int i, j, s;
	private float<10, 5> A[T]; 	
	private float<8, 3> B[T];
	private float<15, 9> C[T]; 
	private float<5, 15> D[T]; 
 	private int<1> E[T]; 

	smcinput(A, 1, T); 
	smcinput(B, 1, T); 
	
	printf("TEST FOR INDIVIDUAL CONVERSION:\n");
	printf("IND ARITHMETIC:\n"); 	
	
	C[0] = A[0] * B[0]; 
	smcoutput(C[0], 1); 
	printf("\n"); 	
	
	D[0] = A[0] * B[0];
	smcoutput(D[0], 1);
	printf("\n"); 	
	
	printf("IND COMPARISON:\n"); 	
	E[0] = A[1] > B[2]; 
	smcoutput(E[0], 1);
	printf("\n"); 
 
	E[0] = A[0] == B[0]; 
	smcoutput(E[0], 1);
	printf("\n");
 
	printf("TEST FOR BATCH CONVERSION:\n"); 
	printf("BATCH ARITHMETIC:\n"); 	
	
	for(i = 0; i < T-1; i++)
	[
		C[i] = A[i] * B[i]; 
	]
	smcoutput(C, 1, T); 
	printf("\n"); 

	for(i = 0; i < T-1; i++)
	[
		D[i] = A[i] * B[i]; 
	]
	smcoutput(D, 1, T); 
	printf("\n"); 

	printf("BATCH COMPARISON:\n"); 
	for(i = 0; i < T-1; i++)
	[
		E[i] = A[i] == B[i]; 
	]
	smcoutput(E, 1, T); 
	printf("\n"); 

	for(i = 0; i < T-1; i++)
	[
		E[i] = A[i] > B[i]; 
	]
	smcoutput(E, 1, T); 
	printf("\n"); 
	
	return 0;
}
