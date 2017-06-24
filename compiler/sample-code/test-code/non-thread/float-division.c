#include<math.h>

public int K=20; // length of input arrays

public int main()
{
	public int i;
	private int<32> A[K], B[K];
	private float<32, 9> FA[K], FB[K], FC[K];
	public float<32, 9> FD[K];  

	smcinput(A, 1, K); 
	smcinput(B, 1, K); 
	
	//INPUT INITIALIZATION
	for(i = 0; i < K; i++){
		FA[i] = (private float<32, 9>)A[i]; 
		FB[i] = (private float<32, 9>)B[i]; 
	}
	
	printf("********************************* INDIVIDUAL DIVISION ************************************\n\n\n");       
	printf("PRIV FLOAT + PRIV FLOAT...\n"); 
	for(i = 0; i < K; i++)
		FC[i] = FA[i] / FB[i];  
	smcoutput(FC, 1, K); 
	printf("\n\n\n");
	
	printf("PRIV FLOAT + PUB FLOAT...\n"); 
	for(i = 0; i < K; i++)
		FC[i] = FA[i] / (public float)i; 
	smcoutput(FC, 1, K); 
	printf("\n\n\n"); 
	
	printf("PUB FLOAT + PRIV FLOAT...\n"); 
	for(i = 0; i < K; i++)
		FC[i] = (public float)i / FA[i]; 
	smcoutput(FC, 1, K); 
	printf("\n\n\n"); 

	printf("********************************* BATCH DIVISION ************************************\n\n\n"); 
        printf("PRIV FLOAT + PRIV FLOAT...\n"); 
        for(i = 0; i < K; i++)[ 
                FC[i] = FA[i] / FB[i]; 
	] 
        smcoutput(FC, 1, K); 
        printf("\n\n\n");
	
        printf("PRIV FLOAT + PUB FLOAT...\n"); 
        for(i = 0; i < K; i++)[
                FC[i] = FA[i] / (public float)i; 
	]
        smcoutput(FC, 1, K); 
        printf("\n\n\n"); 

        printf("PUB FLOAT + PRIV FLOAT...\n"); 
        for(i = 0; i < K; i++)[
                FC[i] = (public float)i / FA[i]; 
	]
        smcoutput(FC, 1, K); 
        printf("\n\n\n"); 

	printf("************************************ ARRAY DIVISION **************************************\n"); 
	printf("PRIV FLOAT ARRAY + PRIV FLOAT ARRAY...\n");
        FC = FA / FB;
        smcoutput(FC, 1, K);
        printf("\n\n\n");
	
	for(i = 0; i < K; i++)
		FD[i] = (public float<32, 9>)i; 
	
	printf("PRIV FLOAT ARRAY + PUB FLOAT ARRAY...\n");
	FC = FA / FD; 
	smcoutput(FC, 1, K);
        printf("\n\n\n");
	
	printf("PUB FLOAT ARRAY + PRIV FLOAT ARRAY...\n");
	FC = FD / FA; 
	smcoutput(FC, 1, K);
        printf("\n\n\n");

	return 0;
}
