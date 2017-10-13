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

	// initializing
	smcinput(IA, 1, K); 
	smcinput(IB, 1, K); 
	smcinput(FD, 1, K); 

	for(i = 0; i < M; i++)
		for(j = 0; j < N; j++){
			IC[i][j] = IB[i*N+j];
			ID[i][j] = i * N + j;
			FE[i][j] = FD[i*N+j];  
		}
	
        /* integer to float conversion */
	printf("******************* INT2FL ***********************\n"); 
        /* private to private assignments */
        printf("priv-2-priv, one-2-one...\n"); 
	for(i = 0; i < K; i++){
		FA[i] = (private float<32, 9>)IA[i]; 
	}
        smcoutput(FA, 1, K);
	printf("\n\n"); 
	
	printf("priv-2-priv, one-2-two...\n");    
        for(i = 0; i < M; i++)
		for(j = 0; j < N; j++)
                	FC[i][j] = (private float<32, 9>)IA[i*N+j]; 
	smcoutput(FC, 1, M, N);
        printf("\n\n"); 

	printf("priv-2-priv, two-2-one...\n");      
        for(i = 0; i < M; i++)
                for(j = 0; j < N; j++)
                        FB[i*N+j] = (private float<32, 9>)IC[i][j];
        smcoutput(FB, 1, K);
        printf("\n\n");
	
	printf("priv-2-priv, two-2-two...\n");      
        for(i = 0; i < M; i++)
                for(j = 0; j < N; j++)
                        FC[i][j] = (private float<32, 9>)IC[i][j]; 
        smcoutput(FC, 1, M, N);
        printf("\n\n");

	/* public to private assignments */
	printf("pub-2-priv, one-2-one...\n");
        for(i = 0; i < K; i++){
                FA[i] = (private float<32, 9>)i;
        }
        smcoutput(FA, 1, K);
        printf("\n\n");

        printf("pub-2-priv, one-2-two...\n");
        for(i = 0; i < M; i++)
                for(j = 0; j < N; j++)
                        FC[i][j] = (private float<32, 9>)(i*N+j);
        smcoutput(FC, 1, M, N);
        printf("\n\n");

        printf("pub-2-priv, two-2-one...\n");
        for(i = 0; i < M; i++)
                for(j = 0; j < N; j++)
                        FB[i*N+j] = (private float<32, 9>)ID[i][j];
        smcoutput(FB, 1, K);
        printf("\n\n");

        printf("pub-2-priv, two-2-two...\n");
        for(i = 0; i < M; i++)
                for(j = 0; j < N; j++)
                        FC[i][j] = (private float<32, 9>)ID[i][j];
        smcoutput(FC, 1, M, N);
        printf("\n\n");
	
	/* private to private assignments */
        printf("BATCH VERSION...\n"); 
	printf("priv-2-priv, one-2-one...\n"); 
        for(i = 0; i < K; i++)[
                FA[i] = (private float<32, 9>)IA[i]; 
        ]
	smcoutput(FA, 1, K);
        printf("\n\n"); 
        
        printf("priv-2-priv, one-2-two...\n");    
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        FC[i][j] = (private float<32, 9>)IA[i*N+j]; 
		]
	]
        smcoutput(FC, 1, M, N);
        printf("\n\n"); 

        printf("priv-2-priv, two-2-one...\n");      
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        FB[i*N+j] = (private float<32, 9>)IC[i][j];
		]
	]
        smcoutput(FB, 1, K);
        printf("\n\n");
        
        printf("priv-2-priv, two-2-two...\n");      
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        FC[i][j] = (private float<32, 9>)IC[i][j]; 
		]
	]
        smcoutput(FC, 1, M, N);
        printf("\n\n");

	/* public to private assignments */
        printf("pub-2-priv, one-2-one...\n");
        for(i = 0; i < K; i++)[
                FA[i] = (private float<32, 9>)i;
        ] 
        smcoutput(FA, 1, K);
        printf("\n\n");

        printf("pub-2-priv, one-2-two...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        FC[i][j] = (private float<32, 9>)(i*N+j);
		]
	]
        smcoutput(FC, 1, M, N);
        printf("\n\n");

        printf("pub-2-priv, two-2-one...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        FB[i*N+j] = (private float<32, 9>)ID[i][j];
			IC[i][j] = ID[i][j]; 	
		]
	]
        smcoutput(FB, 1, K);
        printf("\n\n");

        printf("pub-2-priv, two-2-two...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        FC[i][j] = (private float<32, 9>)ID[i][j];
		]
	]
        smcoutput(FC, 1, M, N);
        printf("\n\n");

	/* float to integer conversion */ 
	printf("\n\n\n\n\n"); 
	printf("***************** FL2INT ******************\n");
	/* private to private assignment */
	printf("priv-2-priv, one-2-one...\n");
        for(i = 0; i < K; i++){
                IA[i] = (private int<32>)FA[i];
        }
        smcoutput(IA, 1, K);
        printf("\n\n");

        printf("priv-2-priv, one-2-two...\n");
        for(i = 0; i < M; i++)
                for(j = 0; j < N; j++)
                        IC[i][j] = (private int<32>)FA[i*N+j];
        smcoutput(IC, 1, M, N);
        printf("\n\n");

        printf("priv-2-priv, two-2-one...\n");
        for(i = 0; i < M; i++)
                for(j = 0; j < N; j++)
                        IB[i*N+j] = (private int<32>)FC[i][j];
        smcoutput(IB, 1, K);
        printf("\n\n");

        printf("priv-2-priv, two-2-two...\n");
        for(i = 0; i < M; i++)
                for(j = 0; j < N; j++)
                        IC[i][j] = (private int<32>)FC[i][j];
        smcoutput(IC, 1, M, N);
        printf("\n\n");

	/* public to private assignments */
        printf("pub-2-priv, one-2-one...\n");
        for(i = 0; i < K; i++){
                IA[i] = (private int<32>)FD[i];
        }
        smcoutput(IA, 1, K);
        printf("\n\n");

        printf("pub-2-priv, one-2-two...\n");
        for(i = 0; i < M; i++)
                for(j = 0; j < N; j++)
                        IC[i][j] = (private int<32>)FD[i*N+j];
        smcoutput(IC, 1, M, N);
        printf("\n\n");

        printf("pub-2-priv, two-2-one...\n");
        for(i = 0; i < M; i++)
                for(j = 0; j < N; j++)
                        IB[i*N+j] = (private int<32>)FE[i][j];
        smcoutput(IB, 1, K);
        printf("\n\n");

        printf("pub-2-priv, two-2-two...\n");
        for(i = 0; i < M; i++)
                for(j = 0; j < N; j++)
                       	IC[i][j] = (private int<32>)FE[i][j];
        smcoutput(IC, 1, M, N);
	
        printf("\n\n");
	printf("BATCH...\n");
	/* private to private assignment */
	printf("priv-2-priv, one-2-one...\n");
        for(i = 0; i < K; i++)[
                IA[i] = (private int<32>)FA[i];
        ]
        smcoutput(IA, 1, K);
        printf("\n\n");

        printf("priv-2-priv, one-2-two...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        IC[i][j] = (private int<32>)FA[i*N+j];
		]
	]
        smcoutput(IC, 1, M, N);
        printf("\n\n");

        printf("priv-2-priv, two-2-one...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        IB[i*N+j] = (private int<32>)FC[i][j];
		]
	]
        smcoutput(IB, 1, K);
        printf("\n\n");

        printf("priv-2-priv, two-2-two...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        IC[i][j] = (private int<32>)FC[i][j];
		]
	]
        smcoutput(IC, 1, M, N);
        printf("\n\n");
	
  	/* public to private assignments */
        printf("pub-2-priv, one-2-one...\n");
        for(i = 0; i < K; i++)[
                IA[i] = (private int<32>)FD[i];
        ]
        smcoutput(IA, 1, K);
        printf("\n\n");

        printf("pub-2-priv, one-2-two...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        IC[i][j] = (private int<32>)FD[i*N+j];
		]
	]
        smcoutput(IC, 1, M, N);
        printf("\n\n");

        printf("pub-2-priv, two-2-one...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        IB[i*N+j] = (private int<32>)FE[i][j];
		]
	]
        smcoutput(IB, 1, K);
        printf("\n\n");

        printf("pub-2-priv, two-2-two...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        IC[i][j] = (private int<32>)FE[i][j];
		]
	]
        smcoutput(IC, 1, M, N);
        printf("\n\n");
	
	private int<50> IIA[K], IIB[K];
        private int<50> IIC[M][N];
        private int IID[M][N];

	/* integer to integer conversion */
	printf("******************* INT2INT ***********************\n"); 
        /* private to private assignments */
        printf("priv-2-priv, one-2-one...\n"); 
	for(i = 0; i < K; i++){
		IIA[i] = (private int<50>)IA[i]; 
	}
        smcoutput(IIA, 1, K);
	printf("\n\n"); 
	
	printf("priv-2-priv, one-2-two...\n");    
        for(i = 0; i < M; i++)
		for(j = 0; j < N; j++)
                	IID[i][j] = (private int<50>)IA[i*N+j]; 
	smcoutput(IID, 1, M, N);
        printf("\n\n"); 

	printf("priv-2-priv, two-2-one...\n");      
        for(i = 0; i < M; i++)
                for(j = 0; j < N; j++)
                        IIB[i*N+j] = (private int<50>)IC[i][j];
        smcoutput(IIB, 1, K);
        printf("\n\n");
	
	printf("priv-2-priv, two-2-two...\n");      
        for(i = 0; i < M; i++)
                for(j = 0; j < N; j++)
                        IID[i][j] = (private int<50>)IC[i][j]; 
        smcoutput(IID, 1, M, N);
        printf("\n\n");

	/* public to private assignments */
	printf("pub-2-priv, one-2-one...\n");
        for(i = 0; i < K; i++){
                IIA[i] = (private int<50>)i;
        }
        smcoutput(IIA, 1, K);
        printf("\n\n");

        printf("pub-2-priv, one-2-two...\n");
        for(i = 0; i < M; i++)
                for(j = 0; j < N; j++)
                        IID[i][j] = (private int<50>)(i*N+j);
        smcoutput(IID, 1, M, N);
        printf("\n\n");

        printf("pub-2-priv, two-2-one...\n");
        for(i = 0; i < M; i++)
                for(j = 0; j < N; j++)
                        IIB[i*N+j] = (private int<50>)ID[i][j];
        smcoutput(IIB, 1, K);
        printf("\n\n");

        printf("pub-2-priv, two-2-two...\n");
        for(i = 0; i < M; i++)
                for(j = 0; j < N; j++)
                        IID[i][j] = (private int<50>)ID[i][j];
        smcoutput(IID, 1, M, N);
        printf("\n\n");
	
        printf("BATCH VERSION...\n"); 
	/* private to private assignments */
	printf("priv-2-priv, one-2-one...\n");
        for(i = 0; i < K; i++)[
                IIA[i] = (private int<50>)IA[i];
        ]
	smcoutput(IIA, 1, K);
        printf("\n\n");

        printf("priv-2-priv, one-2-two...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        IID[i][j] = (private int<50>)IA[i*N+j];
		]
	]
        smcoutput(IID, 1, M, N);
        printf("\n\n");

        printf("priv-2-priv, two-2-one...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        IIB[i*N+j] = (private int<50>)IC[i][j];
		]
	]
        smcoutput(IIB, 1, K);
        printf("\n\n");

        printf("priv-2-priv, two-2-two...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        IID[i][j] = (private int<50>)IC[i][j];
		]
	]
        smcoutput(IID, 1, M, N);
        printf("\n\n");
	
	/* public to private assignments */
        printf("pub-2-priv, one-2-one...\n");
        for(i = 0; i < K; i++)[
                IIA[i] = (private int<50>)i;
        ]
        smcoutput(IIA, 1, K);
        printf("\n\n");

        printf("pub-2-priv, one-2-two...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        IID[i][j] = (private int<50>)(i*N+j);
		]
	]
        smcoutput(IID, 1, M, N);
        printf("\n\n");

        printf("pub-2-priv, two-2-one...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        IIB[i*N+j] = (private int<50>)ID[i][j];
		]
	]
        smcoutput(IIB, 1, K);
        printf("\n\n");

        printf("pub-2-priv, two-2-two...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        IID[i][j] = (private int<50>)ID[i][j];
		]
	]
        smcoutput(IID, 1, M, N);
        printf("\n\n");

	/* float to float conversion */
	printf("\n\n\n\n\n"); 
        printf("***************** FL2FL ******************\n");
        
        private float<40, 9> FFA[K];
	private float<20, 9> FFB[K];
        private float<40, 9> FFC[M][N];
	private float<20, 9> FFD[M][N]; 

	/* private to private assignment */
	printf("priv-2-priv, one-2-one...\n");
        for(i = 0; i < K; i++){
                FFA[i] = (private float<40, 9>)FA[i];
		FFB[i] = (private float<20, 9>)FA[i]; 
        }
        smcoutput(FFA, 1, K);
	smcoutput(FFB, 1, K); 
        printf("\n\n");

        printf("priv-2-priv, one-2-two...\n");
        for(i = 0; i < M; i++){
                for(j = 0; j < N; j++){
                        FFC[i][j] = (private float<40, 9>)FA[i*N+j];
			FFD[i][j] = (private float<20, 9>)FA[i*N+j]; 
		}
	}
        smcoutput(FFC, 1, M, N);
	smcoutput(FFD, 1, M, N); 
        printf("\n\n");

        printf("priv-2-priv, two-2-one...\n");
        for(i = 0; i < M; i++){
                for(j = 0; j < N; j++){
                        FFA[i*N+j] = (private float<40, 9>)FC[i][j];
                        FFB[i*N+j] = (private float<20, 9>)FC[i][j];
		}
	}
        smcoutput(FFA, 1, K);
	smcoutput(FFB, 1, K); 
        printf("\n\n");

        printf("priv-2-priv, two-2-two...\n");
        for(i = 0; i < M; i++){
                for(j = 0; j < N; j++){
                        FFC[i][j] = (private float<40, 9>)FC[i][j];
			FFD[i][j] = (private float<20, 9>)FC[i][j]; 
		}
	}
        smcoutput(FFC, 1, M, N);
	smcoutput(FFD, 1, M, N); 
        printf("\n\n");
	
        /* public to private assignments */
	 printf("pub-2-priv, one-2-one...\n");
        for(i = 0; i < K; i++){
                FFA[i] = (private float<40, 9>)FD[i];
                FFB[i] = (private float<20, 9>)FD[i];
        }
        smcoutput(FFA, 1, K);
        smcoutput(FFB, 1, K);
        printf("\n\n");

        printf("pub-2-priv, one-2-two...\n");
        for(i = 0; i < M; i++){
                for(j = 0; j < N; j++){
                        FFC[i][j] = (private float<40, 9>)FD[i*N+j];
                        FFD[i][j] = (private float<20, 9>)FD[i*N+j];
                }
        }
        smcoutput(FFC, 1, M, N);
        smcoutput(FFD, 1, M, N);
        printf("\n\n");

        printf("pub-2-priv, two-2-one...\n");
        for(i = 0; i < M; i++){
                for(j = 0; j < N; j++){
                        FFA[i*N+j] = (private float<40, 9>)FE[i][j];
                        FFB[i*N+j] = (private float<20, 9>)FE[i][j];
                }
        }
        smcoutput(FFA, 1, K);
        smcoutput(FFB, 1, K);
        printf("\n\n");

        printf("pub-2-priv, two-2-two...\n");
        for(i = 0; i < M; i++){
                for(j = 0; j < N; j++){
                        FFC[i][j] = (private float<40, 9>)FE[i][j];
                        FFD[i][j] = (private float<20, 9>)FE[i][j];
                }
        }
        smcoutput(FFC, 1, M, N);
        smcoutput(FFD, 1, M, N);
        printf("\n\n");

	
	printf("BATCH...\n");
	/* private to private assignment */
	printf("priv-2-priv, one-2-one...\n");
        for(i = 0; i < K; i++)[
                FFA[i] = (private float<40, 9>)FA[i];
                FFB[i] = (private float<20, 9>)FA[i];
        ]
        smcoutput(FFA, 1, K);
        smcoutput(FFB, 1, K);
        printf("\n\n");

        printf("priv-2-priv, one-2-two...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        FFC[i][j] = (private float<40, 9>)FA[i*N+j];
                        FFD[i][j] = (private float<20, 9>)FA[i*N+j];
                ]
        ]
        smcoutput(FFC, 1, M, N);
        smcoutput(FFD, 1, M, N);
        printf("\n\n");

        printf("priv-2-priv, two-2-one...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        FFA[i*N+j] = (private float<40, 9>)FC[i][j];
                        FFB[i*N+j] = (private float<20, 9>)FC[i][j];
                ]
        ]
        smcoutput(FFA, 1, K);
        smcoutput(FFB, 1, K);
        printf("\n\n");

        printf("priv-2-priv, two-2-two...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        FFC[i][j] = (private float<40, 9>)FC[i][j];
                        FFD[i][j] = (private float<20, 9>)FC[i][j];
                ]
        ]
        smcoutput(FFC, 1, M, N);
        smcoutput(FFD, 1, M, N);
        printf("\n\n");

	/* public to private assignments */
        printf("pub-2-priv, one-2-one...\n");
        for(i = 0; i < K; i++)[
                FFA[i] = (private float<40, 9>)FD[i];
                FFB[i] = (private float<20, 9>)FD[i];
        ]
	smcoutput(FFA, 1, K);
        smcoutput(FFB, 1, K);
        printf("\n\n");

        printf("pub-2-priv, one-2-two...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        FFC[i][j] = (private float<40, 9>)FD[i*N+j];
                        FFD[i][j] = (private float<20, 9>)FD[i*N+j];
        	]
	]	
        smcoutput(FFC, 1, M, N);
        smcoutput(FFD, 1, M, N);
        printf("\n\n");

        printf("pub-2-priv, two-2-one...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        FFA[i*N+j] = (private float<40, 9>)FE[i][j];
                        FFB[i*N+j] = (private float<20, 9>)FE[i][j];
                ]
        ]
        smcoutput(FFA, 1, K);
        smcoutput(FFB, 1, K);
        printf("\n\n");

        printf("pub-2-priv, two-2-two...\n");
        for(i = 0; i < M; i++)[
                for(j = 0; j < N; j++)[
                        FFC[i][j] = (private float<40, 9>)FE[i][j];
                        FFD[i][j] = (private float<20, 9>)FE[i][j];
                ]
        ]
        smcoutput(FFC, 1, M, N);
        smcoutput(FFD, 1, M, N);
        printf("\n\n");

	return 0;
}
