#include<math.h>

public int K=20; // length of input arrays
 
public int main()
{
	public int i;
	private int<32> A[K], B[K], C[K]; 
	public int D[K]; 
	
	smcinput(A, 1, K); 
	smcinput(B, 1, K);
	
	printf("INDIVIDUAL SHIFTS\n");
	printf("PUBLIC RIGHT SHFIT\n");
	for(i = 0; i < K; i++){
		C[i] = B[i] >> (i+1); 
	}
	smcoutput(C, 1, K); 	
	printf("\n\n\n"); 
	
	printf("PUBLIC LEFT SHIFT\n"); 
	for(i = 0; i < K; i++){
		D[i] = 10; 
		C[i] = B[i] << (i+1); 
	}	
	smcoutput(C, 1, K);
	printf("\n\n\n"); 

	printf("PRIVATE RIGHT SHIFT\n"); 
	for(i = 0; i < K; i++){
		C[i] = B[i] >> A[i]; 
	}
	smcoutput(C, 1, K); 
	printf("\n\n\n"); 

	printf("PRIVATE LEFT SHIFT\n"); 
	for(i = 0; i < K; i++){
		C[i] = B[i] << A[i]; 
	}
	smcoutput(C, 1, K);
	printf("\n\n\n"); 

	//test for array operations
	printf("ARRAY OPERATIONS\n"); 
	printf("PUBLIC RIGHT SHFIT\n");
	C = A >> D; 
	smcoutput(C, 1, K); 
	printf("\n\n\n"); 
	
	printf("PUBLIC LEFT SHIFT\n"); 
	C = A << D; 
	smcoutput(C, 1, K); 
	printf("\n\n\n"); 
	
	printf("PRIVATE RIGHT SHIFT\n"); 
	C = B >> A; 
	smcoutput(C, 1, K); 
	printf("\n\n\n"); 
	
	printf("PRIVATE LEFT SHIFT\n"); 
	C = B << A; 
	smcoutput(C, 1, K); 
	printf("\n\n\n"); 

	//test for batch operations
	printf("BATCH OPERATIONS...\n"); 
	printf("PUBLIC RIGHT SHFIT\n");
	for(i = 0; i < K; i++)[
                C[i] = B[i] >> D[i];  
        ]
	smcoutput(C, 1, K);
	printf("\n\n\n"); 

	printf("PUBLIC LEFT SHIFT\n"); 
        for(i = 0; i < K; i++)[
                C[i] = B[i] << D[i];
        ]
	smcoutput(C, 1, K);
	printf("\n\n\n"); 

	printf("PRIVATE RIGHT SHIFT\n"); 
	for(i = 0; i < K; i++)[
                C[i] = B[i] >> A[i];
        ]
	smcoutput(C, 1, K);
	printf("\n\n\n"); 

	printf("PRIVATE LEFT SHIFT\n"); 
        for(i = 0; i < K; i++)[
                C[i] = B[i] << A[i];
        ]
        smcoutput(C, 1, K);
	printf("\n\n\n"); 

	return 0;
}
