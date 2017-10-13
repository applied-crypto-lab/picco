#include<math.h>

public int K=20; // length of input arrays

public int main()
{
	public int i;
	private int<32> A[K], B[K], C[K];
	public int<32> D[K];
	
	smcinput(A, 1, K); 
	smcinput(B, 2, K); 

	printf("*************** REGULAR DIVISION ***************\n");
	printf("PRIV INT / PRIV INT...\n"); 
	for(i = 1; i < K; i++)
		C[i] = A[i] / B[i];  
	smcoutput(C, 1, K); 
	printf("\n\n\n");
	
	printf("PRIV INT / PUB INT...\n"); 
	for(i = 1; i < K; i++)
		C[i] = A[i] / i; 
	smcoutput(C, 1, K); 
	printf("\n\n\n");
	
	printf("PUB INT / PRIV INT...\n"); 
	for(i = 1; i < K; i++)
		C[i] = i / A[i]; 
	smcoutput(C, 1, K); 
	printf("\n\n\n"); 

	printf("********************************* BATCH DIVISION ************************************\n\n\n"); 
       	printf("PRIV INT / PRIV INT...\n"); 
        for(i = 1; i < K; i++)[ 
                C[i] = A[i] / B[i]; 
	] 
        smcoutput(C, 2, K); 
        printf("\n\n\n"); 

	printf("PRIV INT / PUB INT...\n"); 
        for(i = 1; i < K; i++)[
                C[i] = A[i] / i; 
	]
        smcoutput(C, 2, K); 
        printf("\n\n\n"); 
        
	printf("PUB INT / PRIV INT...\n"); 
        for(i = 1; i < K; i++)[
                C[i] = i / A[i];
		B[i] = B[i]+1; 
		A[i] = A[i]+1;  
	]
        smcoutput(C, 2, K); 
        printf("\n\n\n"); 

	printf("************************************ ARRAY DIVISION **************************************\n"); 
	printf("PRIV INT ARRAY / PRIV INT ARRAY...\n");
        C = A / B;
        smcoutput(C, 2, K);
        printf("\n\n\n");
	
	printf("PRIV INT ARRAY / PUB INT ARRAY...\n");
	for(i = 0; i < K; i++)
		D[i] = i+1; 
	C = A / D; 
	smcoutput(C, 2, K);
        printf("\n\n\n");
	
	printf("PUB INT ARRAY / PRIV INT ARRAY...\n");
	C = D / A; 
	smcoutput(C, 2, K);
        printf("\n\n\n");
	
	return 0;
}
