#include<math.h>

public int K=20; // length of array / number of input elements
public int M = 5; 
public int N = 4;
 
public int main()
{
	public int i, j, s;
	private int<32> ind[K], A[K], B[K], C[M][N];
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
			C[i][j] = B[i*N+j]; 
		}
	 
	for(i = 0; i < K; i++)
		ind[i] = (private int<32>)FA[i]; 
	smcoutput(FE, 1, M, N); 	
	/************************************** ADDITION *******************************************/
	printf("********************************* INDIVIDUAL ADDITION ************************************\n\n\n");       
	/* private float + private float */
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
	/* public float + private float */
	printf("PUB FLOAT + PRIV FLOAT...\n"); 
	for(i = 0; i < K; i++)
		FC[i] = (public float)i + FA[i]; 
	smcoutput(FC, 1, K); 
	printf("\n\n\n"); 

	printf("********************************* BATCH ADDITION ************************************\n\n\n"); 
        /* private float + private float */
        printf("PRIV FLOAT + PRIV FLOAT...\n"); 
        for(i = 0; i < K; i++)[ 
                FC[i] = FA[i] + FB[i]; 
	] 
        smcoutput(FC, 1, K); 
        printf("\n\n\n"); 
        /* private float + public float */
        printf("PRIV FLOAT + PUB FLOAT...\n"); 
        for(i = 0; i < K; i++)[
                FC[i] = FA[i] + (public float)i; 
	]
        smcoutput(FC, 1, K); 
        printf("\n\n\n"); 
        /* public float + private float */
        printf("PUB FLOAT + PRIV FLOAT...\n"); 
        for(i = 0; i < K; i++)[
                FC[i] = (public float)i + FA[i]; 
	]
        smcoutput(FC, 1, K); 
        printf("\n\n\n"); 

	printf("************************************ ARRAY OPERATION **************************************\n"); 
	printf("PRIV FLOAT ARRAY + PRIV FLOAT ARRAY...\n");
        FC = FA + FB;
        smcoutput(FC, 1, K);
        printf("\n\n\n");
	
	for(i = 0; i < K; i++)
		FD[i] = (public float<32, 9>)i; 
	
	FC = FA + FD; 
	smcoutput(FC, 1, K);
        printf("\n\n\n");
	
	FC = FD + FA; 
	smcoutput(FC, 1, K);
        printf("\n\n\n");

	private int a; 
	a = A @ B; 
	smcoutput(a, 1); 	
	if(A[0] < 100){	
		for(i = 0; i < M; i++)[
			if(A[i] < 100)
				ind[i] = C[i] @ C[i]; 
		]
	}
	smcoutput(ind, 1, M); 		
	return 0;

}
