
public int K=1000; // length of array / number of input elements
public int main()
{
	public int i;
	private int year1[K], year2[K];
	private int final[K];

	smcinput(year1, 1, K); 
	smcinput(year2, 1, K);
	
	for(i = 0; i < K; i++){
		year2[i] = (year2[i] - year1[i]) * 1000;
	}
	
	for(i = 0; i < K; i++)[
		final[i] = year2[i] / year1[i];
	]
	
	
	smcoutput(final, 1, K);
/*	//integer array operations 
	A = A * A; 
	A = A + A; 
	AA[0] = AA[0] * AA[0]; 
	B = A == A; 

	smcoutput(A, 1, K); 
	smcoutput(AA[0], 1, 5); 
	smcoutput(B, 1, K); 
	
	//float array operations
	A1 = C == C; 
	A2 = C < C; 
	D = C + C; 
	
	smcoutput(A1, 1, T); 
	smcoutput(A2, 1, T); 
	smcoutput(D, 1, T); 
*/
	

	return 0;
}
