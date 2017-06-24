#include<math.h>

// input size
public int M = 5; 
public int N = 4;
public int K=20; // set K to M*N
 
public int main()
{
	public int i, j;
	private int<32> A[K], B[K];
	private float<32, 9> FA[K], FB[K], FC[K];
	public float<32, 9> FD[K];  
	private float<32, 9> FE[M][N];
	
	smcinput(A, 1, K); 
	smcinput(B, 1, K); 

	//INPUT INITIALIZATION
	for(i = 0; i < K; i++){
		FA[i] = (private float<32, 9>)A[i]; 
		FB[i] = (private float<32, 9>)B[i]; 
	}
	for(i = 0; i < M; i++)
		for(j = 0; j < N; j++){
			FE[i][j] = FB[i*N+j];
		}
	smcoutput(FE, 1, M, N);
	
	printf("********************************* INDIVIDUAL ADDITION ************************************\n\n\n");       
	printf("PRIV FLOAT + PRIV FLOAT...\n"); 
	for(i = 0; i < K; i++)
		FC[i] = FA[i] + FB[i];  
	smcoutput(FC, 1, K); 
	printf("\n\n\n");
	
	printf("PRIV FLOAT + PUB FLOAT...\n"); 
	for(i = 0; i < K; i++)
		FC[i] = FA[i] + (public float)i; 
	smcoutput(FC, 1, K); 
	printf("\n\n\n");
	
	printf("PUB FLOAT + PRIV FLOAT...\n"); 
	for(i = 0; i < K; i++)
		FC[i] = (public float)i + FA[i]; 
	smcoutput(FC, 1, K); 
	printf("\n\n\n"); 

	printf("********************************* BATCH ADDITION ************************************\n\n\n"); 
        printf("PRIV FLOAT + PRIV FLOAT...\n"); 
        for(i = 0; i < K; i++)[ 
                FC[i] = FA[i] + FB[i]; 
	] 
        smcoutput(FC, 1, K); 
        printf("\n\n\n");
	
        printf("PRIV FLOAT + PUB FLOAT...\n"); 
        for(i = 0; i < K; i++)[
                FC[i] = FA[i] + (public float)i; 
	]
        smcoutput(FC, 1, K); 
        printf("\n\n\n");
	
        printf("PUB FLOAT + PRIV FLOAT...\n"); 
        for(i = 0; i < K; i++)[
                FC[i] = (public float)i + FA[i]; 
	]
        smcoutput(FC, 1, K); 
        printf("\n\n\n"); 

	printf("************************************ ARRAY ADDITION **************************************\n"); 
	printf("PRIV FLOAT ARRAY + PRIV FLOAT ARRAY...\n");
        FC = FA + FB;
        smcoutput(FC, 1, K);
        printf("\n\n\n");
	
	for(i = 0; i < K; i++)
		FD[i] = (public float<32, 9>)i; 
	
	printf("PRIV FLOAT ARRAY + PUB FLOAT ARRAY...\n");
	FC = FA + FD; 
	smcoutput(FC, 1, K);
        printf("\n\n\n");
	
	printf("PUB FLOAT ARRAY + PRIV FLOAT ARRAY...\n");
	FC = FD + FA; 
	smcoutput(FC, 1, K);
        printf("\n\n\n");

	return 0;

}
