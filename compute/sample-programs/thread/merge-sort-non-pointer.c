public int K = 128;
private int A[K];


void mergesort(public int l, public int r) {
  public int i, j, k, m, size;
  size = r - l + 1;
  int tmp[size];
  if (r > l){
    m = (r + l) / 2;
    [mergesort(l, m);]
    [mergesort(m + 1, r);] 
    for (i = size >> 1; i > 0; i = i >> 1) 
      for (j = 0; j < size; j += 2 * i)[
        for (k = j; k < j + i; k++)[
          tmp[k] = A[k + l];
          if (A[k + l] > A[k + i + l]) {
            A[k + l] = A[k + i + l];
            A[k + i + l] = tmp[k];
          }
        ]
      ]
    
  }
}


public int main() {
  public int median = K/2;
  smcinput(A, 1, K);
  mergesort(0, K-1);
  smcoutput(A[median], 1);
  return 0;
}
