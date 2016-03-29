#include<math.h>

public int K=20; // length of array / number of input elements
public int M = 5; 
public int N = 4;
 
public int main()
{
	public int i, j, s;
	private int<32> ind[K], A[K], B[K], C[K]; 
	public int D[K]; 
	
	smcinput(A, 1, K); 
	smcinput(B, 1, K); 
	//test for individual public shift operation
	printf("INDIVIDUAL PUBLIC RIGHT SHFIT...\n");
	for(i = 0; i < K; i++){
		C[i] = B[i] >> (i+1); 
	}
	smcoutput(C, 1, K); 	
	printf("\n\n\n"); 
	
	printf("INDIVIDUAL PUBLIC LEFT SHIFT...\n"); 
	for(i = 0; i < K; i++){
		D[i] = 10; 
		C[i] = B[i] << (i+1); 
	}	
	smcoutput(C, 1, K);
	printf("\n\n\n"); 

	//test for individual private shift operation
	printf("INDIVIDUAL PRIVATE RIGHT SHIFT...\n"); 
	for(i = 0; i < K; i++){
		C[i] = B[i] >> A[i]; 
	}
	smcoutput(C, 1, K); 
	printf("\n\n\n"); 
	
	printf("INDIVIDUAL PRIVATE LEFT SHIFT...\n"); 
	for(i = 0; i < K; i++){
		C[i] = B[i] << A[i]; 
	}
	smcoutput(C, 1, K);
	printf("\n\n\n"); 

	//test for array operations
	printf("ARRAY OPERATIONS...\n"); 
	C = A >> D; 
	smcoutput(C, 1, K); 
	printf("\n\n\n"); 
	
	C = A << D; 
	smcoutput(C, 1, K); 
	printf("\n\n\n"); 
	
	C = B >> A; 
	smcoutput(C, 1, K); 
	printf("\n\n\n"); 
	
	C = B << A; 
	smcoutput(C, 1, K); 
	printf("\n\n\n"); 

	//test for batch operations
	printf("BATCH OPERATIONS...\n"); 
	for(i = 0; i < K; i++)[
                C[i] = B[i] >> D[i];  
        ]
	smcoutput(C, 1, K);
	printf("\n\n\n"); 

        for(i = 0; i < K; i++)[
                C[i] = B[i] << D[i];
        ]
	smcoutput(C, 1, K);
	printf("\n\n\n"); 

	for(i = 0; i < K; i++)[
                C[i] = B[i] >> A[i];
        ]
	smcoutput(C, 1, K);
	printf("\n\n\n"); 

        for(i = 0; i < K; i++)[
                C[i] = B[i] << A[i];
        ]
        smcoutput(C, 1, K);
	printf("\n\n\n"); 

	return 0;
}
