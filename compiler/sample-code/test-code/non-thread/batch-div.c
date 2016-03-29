public int K=120; // length of array / number of input elements
public int K1=10; 
public int main()
{
	public int i, j;
	private int A[K], B[K]; 
	private float FA[K1], FB[K1]; 

	smcinput(A, 1, K); 
	smcinput(FA, 1, K1); 

	/************* BOTH PRIVATE ***************/
	for(i = 0; i < K-1; i++)[
		B[i] = A[i]/A[i+1]; 	
	]
	smcoutput(B, 1, K); 

	for(i = 0; i < K1-1; i++)[
		FB[i] = FA[i]/FA[i+1];
	] 
	smcoutput(FB, 1, K1); 
	/************** PRIVATE DIVIDEND + PUBLIC DIVISOR ***************/
	for(i = 0; i < K-1; i++)[
		B[i] = A[i]/(i+1);
	]
	smcoutput(B, 1, K); 
	
	for(i = 0; i < K1-1; i++)[
		FB[i] = FA[i]/(public float)(i+1);
	]
	smcoutput(FB, 1, K1); 
	/************** PUBLIC DIVIDEND + PRIVATE DIVISOR ***************/
	for(i = 0; i < K-1; i++)[
		B[i] = 10*(i+1)/A[i]; 
	]
	smcoutput(B, 1, K); 
	
	for(i = 0; i < K1-1; i++)[
		FB[i] = (public float)(i+1)/FA[i]; 
	]
	smcoutput(FB, 1, K1);
	/**************** BOTH PRIVATE BUT FROM PUBLIC ASSIGNMENTS ************/
	for(i = 0; i < K1; i++)[
		A[i] = 2 * i + 3; 
		B[i] = i + 1; 
		B[i] = A[i]/B[i]; 
 	]
	smcoutput(B, 1, K1); 
	for(i = 0; i < K1; i++)[
		FA[i] = (public float)(2 * i + 3); 
		FB[i] = (public float)(i+1); 
		FB[i] = FA[i]/FB[i]; 
	]
	smcoutput(FB, 1, K1); 
	return 0;
}
