// program currently does not compile, throws segfauls at the pointer access lines
// needs further examination 

public int K=32;

public void swap(int* A, int* B) {

  private int temp;
  
  if (*A > *B) {
    temp = *A;
    *A = *B;
    *B = temp;
  }
}

public int mergesort(private int* A, public int l, public int r) {

  public int i, j, k, m, size;
  size = r - l + 1;
  
  private int* temp1; 
  private int* temp2;
  
  if (r > l) {
    m = (r + l)/2;
    mergesort(A, l, m); 
    mergesort(A, m + 1, r);
    
    for (i = size >> 1; i > 0; i = i >> 1)
      for (j = 0; j < size; j += 2*i)
	for (k = j; k < j + i; k++) {  
	  temp1 = &A[k+i+l];   // segfaults
	  temp2 = &A[k+l];     // also segfaults
	  swap(&A[k+i+l], &A[k+l]); 
	  // swap(temp1, temp2); // original
	}
  }
  return 0; 
}

public int main() {
   
  public int median = K/2;
  private int A[K]; 
  
  smcinput(A, 1, K);
  mergesort(A, 0, K-1);
  smcoutput(A[median], 1);
  
  return 0;
}