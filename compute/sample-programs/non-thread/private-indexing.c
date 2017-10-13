#include<math.h>

public int K=20; // length of input arrays
public int M = 5; 
public int N = 4; 

public int main()
{
	public int i, j;
	private int<32> ind[K], A[K], B[K], C[M][N], D[K];
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
	
	printf("********************************* SINGLE PRIVATE INDEXING ****************************************\n"); 
	
	//one-dimensional private integer read
	private int result; 
	result = B[ind[5]];
	printf("one-dimensional private integer read:\n"); 
	smcoutput(result, 1); 
	printf("\n\n\n"); 
	
	//two-dimensional private integer read
        printf("two-dimensional private integer read:\n");
        result = C[ind[1]][ind[2]]; 
        smcoutput(result, 1); 
	  
	result = C[3][ind[2]];
        smcoutput(result, 1);

	result = C[ind[1]][3];
        smcoutput(result, 1);
	printf("\n\n\n"); 
	
	//one-dimensional private float read
	private float<32, 9> fresult;
        fresult = FB[ind[5]];
        printf("one-dimensional private float read:\n");
        smcoutput(fresult, 1);
        printf("\n\n\n");

	//two-dimensional private float read
	printf("two-dimensional private float read:\n");
        fresult = FE[ind[1]][ind[2]];
        smcoutput(fresult, 1);

        fresult = FE[3][ind[2]];
        smcoutput(fresult, 1);

        fresult = FE[ind[1]][3];
        smcoutput(fresult, 1);
        printf("\n\n\n");

	//one-dimensional private integer write
        printf("one-dimensional private integer write:\n");
        B[ind[5]] = -120;
	result = B[ind[5]]; 
        smcoutput(result, 1);
	
	B[ind[5]] = B[19];
        result = B[ind[5]];
        smcoutput(result, 1);
        printf("\n\n\n");
	
	//two-dimensional private integer write
        printf("two-dimensional private integer write:\n");
        C[ind[0]][ind[1]] = -120;
        result = C[ind[0]][ind[1]];  
        smcoutput(result, 1);
        
	C[2][ind[1]] = -120;
        result = C[2][ind[1]];
        smcoutput(result, 1);

	C[ind[1]][3] = -120;
        result = C[ind[1]][3];
        smcoutput(result, 1);

	C[ind[0]][ind[1]] = B[18];
        result = C[ind[0]][ind[1]];
        smcoutput(result, 1);

        C[2][ind[1]] = B[18];
        result = C[2][ind[1]];
        smcoutput(result, 1);

        C[ind[1]][3] = B[18];
        result = C[ind[1]][3];
        smcoutput(result, 1);
	printf("\n\n\n"); 
	
	//one-dimensional private float write
	printf("one-dimensional private float write:\n");
        FB[ind[5]] = -150.0;
        fresult = FB[ind[5]];
        smcoutput(fresult, 1);

        FB[ind[5]] = FB[19];
        fresult = FB[ind[5]];
        smcoutput(fresult, 1);
        printf("\n\n\n");

	//two-dimensional private float write
	printf("two-dimensional private float write:\n");
        FE[ind[0]][ind[1]] = -125.0;
        fresult = FE[ind[0]][ind[1]];
        smcoutput(fresult, 1);

        FE[2][ind[1]] = -125.0;
        fresult = FE[2][ind[1]];
        smcoutput(fresult, 1);

        FE[ind[1]][3] = -125.0;
        fresult = FE[ind[1]][3];
        smcoutput(fresult, 1);

        FE[ind[0]][ind[1]] = FB[18];
        fresult = FE[ind[0]][ind[1]];
        smcoutput(fresult, 1);

        FE[2][ind[1]] = FB[18];
        fresult = FE[2][ind[1]];
        smcoutput(fresult, 1);

        FE[ind[1]][3] = FB[18];
        fresult = FE[ind[1]][3];
        smcoutput(fresult, 1);

	printf("\n\n\n"); 
	printf("************************************ BATCH PRIVATE INDEXING *********************************************\n"); 
	//batch one-dimensional private integer read
        printf("batch one-dimensional private integer read:\n");
	for(i = 0; i < K; i++)[
        	D[i] = B[ind[i]];
	]
        smcoutput(D, 1, K);
	printf("\n"); 
        smcoutput(B, 1, K);
        printf("\n\n\n");
	
	//batch two-dimensional private integer read
	printf("batch two-dimensional private integer read:\n");
	for(i = 0; i < M; i++)[
		for(j = 0; j < N; j++)[
			D[i*N+j] = C[ind[i]][ind[j]]; 
		]
	]
        smcoutput(D, 1, K);
	printf("\n"); 
        smcoutput(C, 1, M, N);
	printf("\n");
	
	for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        D[i*N+j] = C[i][ind[j]];
                ]
        ]
        smcoutput(D, 1, K);
        printf("\n");
        smcoutput(C, 1, M, N);
	printf("\n");
	
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        D[i*N+j] = C[ind[i]][j];
                ]
        ]
        smcoutput(D, 1, K);
        printf("\n");
        smcoutput(C, 1, M, N);
        printf("\n\n\n");

	//batch one-dimensional private float read
        printf("batch one-dimensional private float read:\n");
        for(i = 0; i < K; i++)[
		FC[i]= FB[ind[i]];
	]
        smcoutput(FC, 1, K);
	printf("\n\n"); 
        smcoutput(FB, 1, K);
        printf("\n\n\n");
	
	printf("batch two-dimensional private float read:\n");
	for(i = 0; i < M; i++)[
		for(j = 0; j < N; j++)[
			FC[i*N+j] = FE[ind[i]][ind[j]]; 
		]
	]
	smcoutput(FC, 1, K);
        printf("\n\n");
        smcoutput(FE, 1, M, N);
        printf("\n\n\n");
	
	for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        FC[i*N+j] = FE[i][ind[j]];
                ]
        ]
        smcoutput(FC, 1, K);
        printf("\n\n");
        smcoutput(FE, 1, M, N);
        printf("\n\n\n");


	for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        FC[i*N+j] = FE[ind[i]][j];
                ]
        ]
        smcoutput(FC, 1, K);
        printf("\n\n");
        smcoutput(FE, 1, M, N);
        printf("\n\n\n");
	
	//batch one-dimensional private integer write
	printf("batch one-dimensional private integer write:\n");
	public int temp[K];
	for(i = 0; i < K; i++){
		temp[i] = 150 + i; 
	} 
	for(i = 0; i < K; i++)[
		D[ind[i]] = temp[i]; 
	]
        smcoutput(D, 1, K); 
	printf("\n"); 

	for(i = 0; i < K; i++)[
		D[ind[i]] = B[i];  
	]
        smcoutput(D, 1, K);
	printf("\n"); 
	smcoutput(B, 1, K); 
        printf("\n\n\n");
	
	//batch two-dimensional private integer write
	printf("batch two-dimensional private integer write:\n");
	for(i = 0; i < M; i++)[
		for(j = 0; j < N; j++)[
			C[ind[i]][ind[j]] = temp[i*N+j]; 	
		]
	]
        smcoutput(C, 1, M, N);
	printf("\n");
	
	 for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        C[i][ind[j]] = temp[i*N+j]; 
                ]
        ]
        smcoutput(C, 1, M, N);
	printf("\n"); 

	 for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        C[ind[i]][j] = temp[i*N+j]; 
                ]
        ]
        smcoutput(C, 1, M, N);
	printf("\n"); 
        
	for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        C[ind[i]][ind[j]] = B[i*N+j]; 
                ]
        ]
        smcoutput(C, 1, M, N);
	printf("\n"); 

        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        C[i][ind[j]] = B[i*N+j];     
                ]
        ]
        smcoutput(C, 1, M, N);
	printf("\n"); 

        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        C[ind[i]][j] = B[i*N+j];     
                ]
        ]
        smcoutput(C, 1, M, N);
        printf("\n\n\n");
	
	//batch one-dimensional private float write
	printf("batch one-dimensional private float write:\n");
	public float ftemp[K]; 
	for(i = 0; i < K; i++)
	  ftemp[i] = 160.0 + (public int)i; 
        for(i = 0; i < K; i++)[
		FC[ind[i]] = ftemp[i]; 
	]
        smcoutput(FC, 1, K);
	printf("\n\n\n"); 
	for(i = 0; i < K; i++)[
		FC[ind[i]] = FB[i]; 
	]
        smcoutput(FC, 1, K);
        printf("\n\n\n");
	
	//batch two-dimensional private float write
	printf("batch two-dimensional private float write:\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        FE[ind[i]][ind[j]] = ftemp[i*N+j];
                ]
        ]
        smcoutput(FE, 1, M, N);
	
        printf("\n\n\n");
         for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        FE[i][ind[j]] = ftemp[i*N+j];
                ]
        ]
        smcoutput(FE, 1, M, N);
	 
        printf("\n\n\n");

         for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        FE[ind[i]][j] = ftemp[i*N+j];
                ]
        ]
        smcoutput(FE, 1, M, N);
        printf("\n\n\n");

        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        FE[ind[i]][ind[j]] = FB[i*N+j];
                ]
        ]
        smcoutput(FE, 1, M, N);
        printf("\n\n\n");

        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        FE[i][ind[j]] = FB[i*N+j];
                ]
        ]
        smcoutput(FE, 1, M, N);
        printf("\n\n\n");

	for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        FE[ind[i]][j] = FB[i*N+j];
                ]
        ]
        smcoutput(FE, 1, M, N);
        printf("\n\n\n");


	printf("*********************************** BATCH PRIVATE INDEXING WITH PRIVATE CONDITIONS **************************************\n"); 
	if(B[ind[0]] != 0){
        	for(i = 0; i < M; i++)[
			if(B[ind[i]] != 0){
                		for(j = 0; j < N; j++)[
					if(FB[i*N+j] != (public float)0)
                        			FE[ind[i]][j] = ftemp[i*N+j];
                		]
			}
        	]
	}
        smcoutput(FE, 1, M, N);
        printf("\n\n\n");
	return 0;
}
