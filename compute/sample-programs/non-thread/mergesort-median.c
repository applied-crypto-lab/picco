public int K = 32;
private int A[K];

public void swap(private int* A, private int* B) {

  private int temp;
  
  if (*A > *B) {
    temp = *A;
    *A = *B;
    *B = temp;
  }
}

public int mergesort(public int l, public int r) {

  public int i, j, k, m, size;
  size = r - l + 1;

  private int* temp1; 
  private int* temp2;
  
  if (r > l) {
    m = (r + l)/2;
    mergesort(l, m); 
    mergesort(m + 1, r);
    
    for (i = size >> 1; i > 0; i = i >> 1)
      for (j = 0; j < size; j += 2*i)
        for (k = j; k < j + i; k++) {  
          temp1 = &A[k + i + l];
          temp2 = &A[k + l];
          swap(temp1, temp2);
        }
  }
  return 0; 
}

public int main() {
   
  public int median = K/2;

  smcinput(A, 1, K);
  mergesort(0, K - 1);
  smcoutput(A, 1, K);
  smcoutput(A[median], 1);

  return 0;
}
