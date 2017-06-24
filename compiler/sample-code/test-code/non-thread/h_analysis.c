
public int K=1000; // length of input set
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
	

	return 0;
}
