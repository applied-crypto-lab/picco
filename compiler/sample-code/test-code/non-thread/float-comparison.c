#include<math.h>

public int K=20; // length of array / number of input elements
public int M = 5; 
public int N = 4; 

public int main()
{
	public int i, j, s;
	private int<32> A[K], B[K], C[K];
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
	
	/************************************** ADDITION *******************************************/
	printf("********************************* INDIVIDUAL ADDITION ************************************\n\n\n");       
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
