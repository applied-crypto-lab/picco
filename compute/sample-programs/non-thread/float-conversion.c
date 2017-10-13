public int T=5; 
public int main()
{
	public int i;
	private float<10, 5> A[T]; 	
	private float<8, 3> B[T];
	private float<15, 9> C[T]; 
	private float<5, 15> D[T]; 
 	private int<1> E[T]; 

	smcinput(A, 1, T); 
	smcinput(B, 1, T); 
	
	printf("TEST FOR INDIVIDUAL FLOAT TO FLOAT CONVERSION:\n");
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
 
	printf("TEST FOR BATCH FLOAT TO FLOAT CONVERSION:\n"); 
	printf("BATCH ARITHMETIC:\n"); 	
	
	for(i = 0; i < T; i++)
	[
		C[i] = A[i] * B[i]; 
	]
	smcoutput(C, 1, T); 
	printf("\n"); 

	for(i = 0; i < T; i++)
	[
		D[i] = A[i] * B[i]; 
	]
	smcoutput(D, 1, T); 
	printf("\n"); 

	printf("BATCH COMPARISON:\n"); 
	for(i = 0; i < T; i++)
	[
		E[i] = A[i] == B[i]; 
	]
	smcoutput(E, 1, T); 
	printf("\n"); 

	for(i = 0; i < T; i++)
	[
		E[i] = A[i] > B[i]; 
	]
	smcoutput(E, 1, T); 
	printf("\n"); 

	private int<32> F[T];
	printf("TEST FOR INDIVIDUAL FLOAT TO INT CONVERSION:\n");
	for(i = 0; i < T; i++)
		F[i] = (private int<32>)A[i]; 
	smcoutput(F, 1, T); 
	printf("\n");

	private int<5> G[T];
	for(i = 0; i < T; i++)
		G[i] = (private int<5>)A[i]; 
	smcoutput(G, 1, T); 
	printf("\n");
	
	return 0;
}
