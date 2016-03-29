#include<math.h>

public int K=20; // length of array / number of input elements
public int M = 5; 
public int N = 4; 
public int main()
{
	public int i, j, s;
	private int<32> ind[K], A[K], B[K], C[K];
	public int<32> D[K]; 
	smcinput(A, 1, K); 
	smcinput(B, 2, K); 
	

	C[0] = A[10] >> 1;
	smcoutput(C[0], 1); 
	 
	C[0] = A[15] >> A[2];
	smcoutput(C[0], 1); 
	
	C[0] = A[5] << 1;
	smcoutput(C[0], 1); 
		
	C[0] = A[5] << A[2];
	smcoutput(C[0], 1); 
	/************************************** DIVISION *******************************************/
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

	printf("************************************ ARRAY OPERATION **************************************\n"); 
	printf("PRIV INT ARRAY / PRIV INT ARRAY...\n");
        C = A / B;
        smcoutput(C, 2, K);
        printf("\n\n\n");
	
	for(i = 0; i < K; i++)
		D[i] = i+1; 
	
	C = A / D; 
	smcoutput(C, 2, K);
        printf("\n\n\n");
	
	C = D / A; 
	smcoutput(C, 2, K);
        printf("\n\n\n");
	
	return 0;
}
