#include<math.h>

public int K=20; // length of input arrays

public int main()
{
	public int i;
	private int<32> A[K], B[K], C[K];
	private float<32, 9> FA[K], FB[K];
	smcinput(A, 1, K); 
	smcinput(B, 1, K); 
	
	//INPUT INITIALIZATION
	for(i = 0; i < K; i++){
		FA[i] = (private float<32, 9>)A[i]; 
		FB[i] = (private float<32, 9>)B[i]; 
	}
	
	printf("********************************* INDIVIDUAL COMPARISON ************************************\n\n\n");       
	printf("PRIV FLOAT ==  PRIV FLOAT...\n"); 
	for(i = 0; i < K; i++)
		C[i] = FA[i] == FB[i];  
	smcoutput(C, 1, K); 
	printf("\n\n\n"); 
	
	printf("PRIV FLOAT < PRIV FLOAT...\n"); 
	 for(i = 0; i < K; i++)
                C[i] = FA[i] < FB[i];
        smcoutput(C, 1, K); 
	printf("\n\n\n"); 

	printf("PRIV FLOAT == PUB FLOAT...\n"); 
	for(i = 0; i < K; i++)
		C[i] = FA[i] == (public float)i; 
	smcoutput(C, 1, K); 
	printf("\n\n\n"); 

	printf("PRIV FLOAT <  PUB FLOAT...\n");
        for(i = 0; i < K; i++)
                C[i] = FA[i] < (public float)i; 
        smcoutput(C, 1, K);
        printf("\n\n\n");

	printf("PUB FLOAT == PRIV FLOAT...\n"); 
	for(i = 0; i < K; i++)
		C[i] = (public float)i == FA[i]; 
	smcoutput(C, 1, K); 
	printf("\n\n\n"); 

	
	printf("PUB FLOAT < PRIV FLOAT...\n");
        for(i = 0; i < K; i++)
                C[i] = (public float)i < FA[i]; 
        smcoutput(C, 1, K); 
        printf("\n\n\n");

	printf("********************************* BATCH COMPARISON ************************************\n\n\n"); 
        printf("PRIV FLOAT == PRIV FLOAT...\n"); 
        for(i = 0; i < K; i++)[ 
                C[i] = FA[i] == FB[i]; 
	] 
        smcoutput(C, 1, K); 
        printf("\n\n\n"); 

	printf("PRIV FLOAT < PRIV FLOAT...\n");
        for(i = 0; i < K; i++)[
                C[i] = FA[i] < FB[i];
        ]
        smcoutput(C, 1, K); 
        printf("\n\n\n");

        printf("PRIV FLOAT == PUB FLOAT...\n"); 
        for(i = 0; i < K; i++)[
		C[i] = FA[i] == (public float)i; 
	]
        smcoutput(C, 1, K); 
        printf("\n\n\n"); 
       
	printf("PRIV FLOAT < PUB FLOAT...\n");
        for(i = 0; i < K; i++)[
                C[i] = FA[i] < (public float)i;
        ]
        smcoutput(C, 1, K);
        printf("\n\n\n"); 

        printf("PUB FLOAT == PRIV FLOAT...\n"); 
        for(i = 0; i < K; i++)[
                C[i] = (public float)i == FA[i]; 
	]
        smcoutput(C, 1, K); 
	printf("\n\n\n"); 

	printf("PUB FLOAT < PRIV FLOAT...\n");
        for(i = 0; i < K; i++)[
                C[i] = (public float)i < FA[i];
        ]
        smcoutput(C, 1, K); 
        printf("\n\n\n"); 

	return 0;
}
