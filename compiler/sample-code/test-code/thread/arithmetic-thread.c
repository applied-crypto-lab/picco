
public int K=20; // length of array / number of input elements
public int T=10; 
public int main()
{
	public int i0, i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, j, j1, j2, j3, s;
	public float t;  
	private int<32> A[K], B1[K], B2[K], B3[K], B4[K], B5[K], B6[K], B7[K];
	private float<32, 9> C[T], D1[T], D2[T], D3[T], D4[T], D5[T]; 	

	smcinput(A, 1, K); 
	smcinput(C, 1, T); 

	/******************* TEST FOR INDIVIDUAL INTEGER OPERATIONS ****************/
	
	//LT
	printf("TEST FOR INDIVIDUAL INTEGER OPERATIONS\n"); 
	[
		//printf("\nINT INDIVIDUAL LT...\n"); 
		public int array[K]; 
		for(i1 = 0; i1 < 19; i1++){
			B1[i1] = A[i1] < A[i1+1]; 
			if(smcopen(B1[i1]) != (smcopen(A[i1]) < smcopen(A[i1+1]))){
				printf("LT wrong...%d, %d, %d\n", smcopen(B1[i1]), smcopen(A[i1]), smcopen(A[i1+1]));
				exit(0); 
			}
		}
	]
	[
                //printf("\nINT INDIVIDUAL LEQ...\n");
                for(i2 = 0; i2 < K-1; i2++){
                        B2[i2] = A[i2] <= A[i2+1];
                        if(smcopen(B2[i2]) != (smcopen(A[i2]) <= smcopen(A[i2+1]))){
				printf("LEQ wrong...%d, %d, %d\n", smcopen(B2[i2]), smcopen(A[i2]), smcopen(A[i2+1]));
                                exit(0);
                        }
                }
        ]
	[
                //printf("\nINT INDIVIDUAL GT...\n");
                for(i3 = 0; i3 < K-1; i3++){
                        B3[i3] = A[i3] > A[i3+1];
                        if(smcopen(B3[i3]) != (smcopen(A[i3]) > smcopen(A[i3+1]))){
				printf("GT wrong...%d, %d, %d\n", smcopen(B3[i3]), smcopen(A[i3]), smcopen(A[i3+1]));
                                exit(0);
                        }
                }
        ]
	[
                //printf("\nINT INDIVIDUAL GEQ...\n");
                for(i4 = 0; i4 < K-1; i4++){
                        B4[i4] = A[i4] >= A[i4+1];
                        if(smcopen(B4[i4]) != (smcopen(A[i4]) >= smcopen(A[i4+1]))){
				printf("GEQ wrong...%d, %d, %d\n", smcopen(B4[i4]), smcopen(A[i4]), smcopen(A[i4+1]));
                                exit(0);
                        }
                }
        ]
	[
                //printf("\nINT INDIVIDUAL GEQ...\n");
                for(i5 = 0; i5 < K-1; i5++){
                        B5[i5] = A[i5] == A[i5+1];
                        if(smcopen(B5[i5]) != (smcopen(A[i5]) == smcopen(A[i5+1]))){
				printf("EQ wrong...%d, %d, %d\n", smcopen(B5[i5]), smcopen(A[i5]), smcopen(A[i5+1]));
                                exit(0);
                        }
                }
        ]
        [
                //printf("\nINT INDIVIDUAL NEQ...\n");
                for(i6 = 0; i6 < K-1; i6++){
                        B6[i6] = A[i6] != A[i6+1];
                        if(smcopen(B6[i6]) != (smcopen(A[i6]) != smcopen(A[i6+1]))){
				printf("NEQ wrong...%d, %d, %d\n", smcopen(B6[i6]), smcopen(A[i6]), smcopen(A[i6+1]));
                                exit(0);
                        }
                }
        ]
	[
		//printf("\nINT INDIVIDUAL DIV ..\n"); 
		for(i0 = 0; i0 < 100; i0++){
			printf("%d\n", i0); 
	        	for(i7 = 0; i7 < 19; i7++){
				B7[i7] = A[i7] / A[1]; 
				printf("%d, %d, %d\n", smcopen(B7[i7]), smcopen(A[i7]), smcopen(A[1])); 
			}
		}
	]
	
	printf("TEST FOR BATCH INTEGER OPERATIONS\n"); 
	/****************** TEST FOR BATCH OPERATIONS **********************/
	[

		for(i1 = 0; i1 < T; i1++)[
			B1[i1] = A[i1] < A[2*i1]; 
		]
	]
	[
		for(i2 = 0; i2 < T; i2++)[
			B2[i2] = A[i2] <= A[2*i2]; 
		]
	]
	[
		for(i3 = 0; i3 < T; i3++)[
			B3[i3] = A[i3] > A[2*i3];
		]
	]
	[ 
		for(i4 = 0; i4 < T; i4++)[
			B4[i4] = A[i4] >= A[2*i4];
		]
	]
	[	 
		for(i5 = 0; i5 < T; i5++)[
			B5[i5] = A[i5] == A[2*i5];
		]
	]
	[ 
		for(i6 = 0; i6 < T; i6++)[
			B6[i6] = A[i6] != A[2*i6];
		]
	]
	[ 
		for(i0 = 0; i0 < 100; i0++){
			printf("%d\n", i0); 
			for(i7 = 0; i7 < T; i7++)[
				B7[i7] = A[2*i7] / A[1]; 
			]
		}	
	]
	//for(i = 0; i < T; i++)
	  //      printf("%d, %d, %d\n", smcopen(B[i]), smcopen(A[2*i]), smcopen(A[1]));
	/**************** TEST FOR INDIVIDUAL FLOAT OPERATIONS *******************/
	printf("TEST FOR INDIVIDUAL FLOAT OPERATIONS\n"); 
	[
		for(i1 = 0; i1 < T-1; i1++)
			B1[i1] = C[i1] < C[i1+1];
	]
	[
		for(i2 = 0; i2 < T-1; i2++)
			B2[i2] = C[i2] <= C[i2+1]; 
	]
	[
		for(i3 = 0; i3 < T-1; i3++)
			B3[i3] = C[i3] > C[i3+1]; 
	]
	[
		for(i4 = 0; i4 < T-1; i4++)
			B4[i4] = C[i4] >= C[i4+1]; 
	]
	[
		for(i5 = 0; i5 < T-1; i5++)
			B5[i5] = C[i5] == C[i5+1];
	]
	[ 
		for(i6 = 0; i6 < T-1; i6++)
			B6[i6] = C[i6] != C[i6+1]; 
	]
	[
		for(j1 = 0; j1 < 100; j1++){
			printf("%d\n", j1); 
			for(i7 = 0; i7 < T-1; i7++)
				D1[i7] = C[i7] + C[i7+1];
		}
	]
	[ 
		for(j2 = 0; j2 < 100; j2++){
			printf("%d\n", j2); 
			for(i8 = 0; i8 < T-1; i8++)
				D2[i8] = C[i8] - C[i8+1]; 
		}
	]
	[
		for(j3 = 0; j3 < 100; j3++){
			printf("%d\n", j3); 
			for(i9 = 0; i9 < T-1; i9++)
				D3[i9] = C[i9] * C[i9+1]; 
		}
	]
	[
		for(i0 = 0; i0  < 100; i0++){
			printf("%d\n", i0); 
			for(i10 = 0; i10 < T-1; i10++)
				D4[i10] = C[i10] / C[i10+1]; 
		}
	]
	
	printf("TEST FOR BATCH FLOAT OPERATIONS\n"); 
	/****************** TEST FOR BATCH FLOAT OPERATIONS **********************/
	[
		for(i1 = 0; i1 < T-1; i1++)[
			B1[i1] = C[i1] < C[i1+1];
		]
	]
	[ 
		for(i2 = 0; i2 < T-1; i2++)[
			B2[i2] = C[i2] <= C[i2+1]; 
		]
	]		
	[
		for(i3 = 0; i3 < T-1; i3++)[
			B3[i3] = C[i3] > C[i3+1]; 
		]
	]
	[
		for(i4 = 0; i4 < T-1; i4++)[
			B4[i4] = C[i4] >= C[i4+1]; 
		]
	]
	[
		for(i5 = 0; i5 < T-1; i5++)[
			B5[i5] = C[i5] == C[i5+1]; 
		]
	]
	[
		for(i6 = 0; i6 < T-1; i6++)[
			B6[i6] = C[i6] != C[i6+1]; 
		]
	]
	[
		for(j1 = 0; j1 < 100; j1++){
			printf("%d\n", j1); 
			for(i7 = 0; i7 < T-1; i7++)[
				D1[i7] = C[i7] + C[i7+1];
			]
		}
	]
	[ 
		for(j2 = 0; j2 < 100; j2++){
			printf("%d\n", j2); 
			for(i8 = 0; i8 < T-1; i8++)[
				D2[i8] = C[i8] - C[i8+1];
			]
		}
	]
	[ 
		for(j3 = 0; j3 < 100; j3++){
			printf("%d\n", j3); 
			for(i9 = 0; i9 < T-1; i9++)[
				D3[i9] = C[i9] * C[i9+1]; 
			]
		}
	]
	[
		for(i0 = 0; i0 < 100; i0++){ 
			printf("%d\n", i0); 
			for(i10 = 0; i10 < T-1; i10++)[
				D4[i10] = C[i10] / C[i10+1]; 
			]
		}
	]
	return 0;
}
