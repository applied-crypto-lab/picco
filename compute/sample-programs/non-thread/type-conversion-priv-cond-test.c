#include<math.h>

public int K = 20; // length of input arrays
public int M = 4;
public int N = 5; 

public int main()
{
	public int i, j;
	private int<32> IA[K], IB[K];
	private int<32> IC[M][N]; 
	public int ID[M][N]; 
	
	private float<32, 9> FA[K], FB[K]; 
	private float<32, 9> FC[M][N]; 
	public float<32, 9> FD[K]; 
	public float<32, 9> FE[M][N]; 
	smcinput(IA, 1, K); 
	smcinput(IB, 1, K); 
	smcinput(FD, 1, K); 

	for(i = 0; i < M; i++)
		for(j = 0; j < N; j++){
			IC[i][j] = IB[i*N+j];
			ID[i][j] = i * N + j;
			FE[i][j] = FD[i*N+j]; 
			FA[i*N+j] = FD[i*N+j]; 
			FB[i*N+j] = FD[i*N+j]; 
			FC[i][j] = FD[i*N+j];  
		} 

	/* integer to integer conversion */
	printf("******************* INT2FL ***********************\n"); 
        /* private to private assignments */
        printf("priv-2-priv, one-2-one...\n"); 
	for(i = 0; i < K; i++){
		if(IA[i] < 0)
			FA[i] = (private float<32, 9>)IA[i]; 
	}
        smcoutput(FA, 1, K);
	printf("\n\n"); 
	
	printf("priv-2-priv, one-2-two...\n");    
        for(i = 0; i < M; i++){
		for(j = 0; j < N; j++){
			if(IA[i*N+j] < 0)
                		FC[i][j] = (private float<32, 9>)IA[i*N+j]; 
		}
	}
	smcoutput(FC, 1, M, N);
        printf("\n\n"); 

	printf("priv-2-priv, two-2-one...\n");      
        for(i = 0; i < M; i++){
                for(j = 0; j < N; j++){
			if(IA[i*N+j] < 0)
                        	FB[i*N+j] = (private float<32, 9>)IC[i][j];
		}
	}
        smcoutput(FB, 1, K);
        printf("\n\n");
	
	printf("priv-2-priv, two-2-two...\n");      
        for(i = 0; i < M; i++){
                for(j = 0; j < N; j++){
                        if(IA[i*N+j] < 0)
 				FC[i][j] = (private float<32, 9>)IC[i][j]; 
		}
	}
        smcoutput(FC, 1, M, N);
        printf("\n\n");

	/* public to private assignments */
	printf("pub-2-priv, one-2-one...\n");
        for(i = 0; i < K; i++){
		if(IA[i] < 0)
                	FA[i] = (private float<32, 9>)i;
        }
        smcoutput(FA, 1, K);
        printf("\n\n");

        printf("pub-2-priv, one-2-two...\n");
        for(i = 0; i < M; i++){
                for(j = 0; j < N; j++){
                        if(IA[i*N+j] <  0)
				FC[i][j] = (private float<32, 9>)(i*N+j);
		}
	}
        smcoutput(FC, 1, M, N);
        printf("\n\n");

        printf("pub-2-priv, two-2-one...\n");
        for(i = 0; i < M; i++){
                for(j = 0; j < N; j++){
                        if(IA[i*N+j] < 0)
				FB[i*N+j] = (private float<32, 9>)ID[i][j];
		}
	}
        smcoutput(FB, 1, K);
        printf("\n\n");

        printf("pub-2-priv, two-2-two...\n");
        for(i = 0; i < M; i++){
                for(j = 0; j < N; j++){
			if(IA[i*N+j] < 0)
                        	FC[i][j] = (private float<32, 9>)ID[i][j];
		}
	}
        smcoutput(FC, 1, M, N);
        printf("\n\n");
	
        printf("BATCH VERSION...\n"); 
	/* private to private assignments */
	printf("priv-2-priv, one-2-one...\n"); 
        for(i = 0; i < K; i++)[
		if(IA[i] < 0)
               		FA[i] = (private float<32, 9>)IA[i]; 
        ]
	smcoutput(FA, 1, K);
        printf("\n\n"); 
        
        printf("priv-2-priv, one-2-two...\n");    
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
			if(IA[i*N+j] < 0)
                        	FC[i][j] = (private float<32, 9>)IA[i*N+j]; 
		]
	]
        smcoutput(FC, 1, M, N);
        printf("\n\n"); 

        printf("priv-2-priv, two-2-one...\n");      
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
			if(IA[i*N+j] < 0)
                        	FB[i*N+j] = (private float<32, 9>)IC[i][j];
		]
	]
        smcoutput(FB, 1, K);
        printf("\n\n");
        
        printf("priv-2-priv, two-2-two...\n");      
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
			if(IA[i*N+j] < 0)
                        	FC[i][j] = (private float<32, 9>)IC[i][j]; 
		]
	]
        smcoutput(FC, 1, M, N);
        printf("\n\n");

	 /* public to private assignments */
        printf("pub-2-priv, one-2-one...\n");
        for(i = 0; i < K; i++)[
		if(IA[i] < 0)
                	FA[i] = (private float<32, 9>)i;
        ] 
        smcoutput(FA, 1, K);
        printf("\n\n");

        printf("pub-2-priv, one-2-two...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
			if(IA[i*N+j] < 0)
                        	FC[i][j] = (private float<32, 9>)(i*N+j);
		]
	]
        smcoutput(FC, 1, M, N);
        printf("\n\n");

        printf("pub-2-priv, two-2-one...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
			if(IA[i*N+j] < 0)
                        	FB[i*N+j] = (private float<32, 9>)ID[i][j];
		]
	]
        smcoutput(FB, 1, K);
        printf("\n\n");

        printf("pub-2-priv, two-2-two...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
			if(IA[i*N+j] < 0)
                        	FC[i][j] = (private float<32, 9>)ID[i][j];
		]
	]
        smcoutput(FC, 1, M, N);
        printf("\n\n");

	/* floating point to integer conversion */
	printf("\n\n\n\n\n"); 
	printf("***************** FL2INT ******************\n");
	/* private to private assignment */
	printf("priv-2-priv, one-2-one...\n");
        for(i = 0; i < K; i++){
		if(IA[i] < 0)	
                	IB[i] = (private int<32>)FA[i];
        }
        smcoutput(IB, 1, K);
        printf("\n\n");

        printf("priv-2-priv, one-2-two...\n");
        for(i = 0; i < M; i++){
                for(j = 0; j < N; j++){
			if(IA[i*N+j] < 0)
                        	IC[i][j] = (private int<32>)FA[i*N+j];
		}
	}
        smcoutput(IC, 1, M, N);
        printf("\n\n");

        printf("priv-2-priv, two-2-one...\n");
        for(i = 0; i < M; i++){
                for(j = 0; j < N; j++){
			if(IA[i*N+j] < 0)
                        	IB[i*N+j] = (private int<32>)FC[i][j];
		}
	}
        smcoutput(IB, 1, K);
        printf("\n\n");

        printf("priv-2-priv, two-2-two...\n");
        for(i = 0; i < M; i++){
                for(j = 0; j < N; j++){
                        if(IA[i*N+j] < 0)
				IC[i][j] = (private int<32>)FC[i][j];
		}
	}
        smcoutput(IC, 1, M, N);
        printf("\n\n");

	/* public to private assignments */
        printf("pub-2-priv, one-2-one...\n");
        for(i = 0; i < K; i++){
		if(IA[i] < 0)
                	IB[i] = (private int<32>)FD[i];
        }
        smcoutput(IB, 1, K);
        printf("\n\n");

        printf("pub-2-priv, one-2-two...\n");
        for(i = 0; i < M; i++){
                for(j = 0; j < N; j++){
			if(IA[i*N+j] < 0)
                        	IC[i][j] = (private int<32>)FD[i*N+j];
		}
	}
        smcoutput(IC, 1, M, N);
        printf("\n\n");

        printf("pub-2-priv, two-2-one...\n");
        for(i = 0; i < M; i++){
                for(j = 0; j < N; j++){
			if(IA[i*N+j] < 0)
                        	IB[i*N+j] = (private int<32>)FE[i][j];
		}
	}
        smcoutput(IB, 1, K);
        printf("\n\n");

        printf("pub-2-priv, two-2-two...\n");
        for(i = 0; i < M; i++){
                for(j = 0; j < N; j++){
			if(IA[i*N+j] < 0)
                       		IC[i][j] = (private int<32>)FE[i][j];
		}
	}
        smcoutput(IC, 1, M, N);
	
        printf("\n\n");
	printf("BATCH...\n");
	/* private to private assignment */
	printf("priv-2-priv, one-2-one...\n");
        for(i = 0; i < K; i++)[
		if(IA[i] < 0)
                	IB[i] = (private int<32>)FA[i];
        ]
        smcoutput(IB, 1, K);
        printf("\n\n");

        printf("priv-2-priv, one-2-two...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
			if(IA[i*N+j] < 0)
                        	IC[i][j] = (private int<32>)FA[i*N+j];
		]
	]
        smcoutput(IC, 1, M, N);
        printf("\n\n");

        printf("priv-2-priv, two-2-one...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
			if(IA[i*N+j] < 0)
                        	IB[i*N+j] = (private int<32>)FC[i][j];
		]
	]
        smcoutput(IB, 1, K);
        printf("\n\n");

        printf("priv-2-priv, two-2-two...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
			if(IA[i*N+j] < 0)
                        	IC[i][j] = (private int<32>)FC[i][j];
		]
	]
        smcoutput(IC, 1, M, N);
	
	return 0;
}
