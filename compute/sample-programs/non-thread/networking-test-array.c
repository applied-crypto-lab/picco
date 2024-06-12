//Set K high to test rounds
public int K = 10, K1 = 2, K2 = 5; // K needs to be equal to K1*K2

public int main() {

  public int i,j;
  int A[K], B[K];
  float C[K], D[K];
  
  smcinput(A, 1, K); 
  smcinput(C, 1, K);
  
  B = A == A; 
  smcoutput(B, 1, K); 
  
  return 0;
}
