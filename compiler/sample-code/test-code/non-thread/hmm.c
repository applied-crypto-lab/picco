

public int T = 32, N = 3, MM = 32;

void main() {
   
   // this program assumes the default float is 32,9 and default integer is 32
   private float a[N][N];
   private float bb[N][MM];
   private float b[N][T];
   private int x[T];
   private float pi[N];
   private float delta[T][N];
   private float temp[N][N];
   private int psi[T][N];
   private int max[N/2];
   private float p;
   private int q[T];
   
   public int i, j, k, m, u;
   
   // reading input
   smcinput(a,1, N, N);
   smcinput(bb, 1, N, MM);
   smcinput(x, 1, T);
   smcinput(pi, 1, N);
	    
   // initialization step
   for (i = 0; i < N; i++) [
      for (j = 0; j < T; j++) [
	 b[i][j] = bb[i][x[j]];
      ]
   ]
     
   for (i = 0; i < N; i++) [
      delta[0][i] = pi[i]*b[i][0];
      psi[0][i] = 0;
   ]
   // recursion step
   for (k = 1; k < T; k++) {
      for (j = 0; j < N; j++) [
	 for (i = 0; i < N; i++) [
	     temp[i][j] = delta[k-1][i]*a[i][j];
	 ]		
      ]
      u = N;
      for (m = N >> 1; m >= 1; m = m >> 1) {
	 for (j = 0; j < N; j++) [
	    for (i = 0; i < m; i++) [
	       if (temp[2*i][j] < temp[2*i+1][j]) {
		  temp[i][j] = temp[2*i+1][j];
		  max[i] = 2*i+1;
	       }
	       else {
		  temp[i][j] = temp[2*i][j];
		  max[i] = 2*i;
	       }
	    ]
	 ]
	 if (2 * m < u) {
	    for (j = 0; j < N; j++) [
	       temp[m][j] = temp[2*m][j];
            ]
	    u = m+1;
	 }
	 else
	   u = m;
      }
      if (u > 1) {
	 for (j = 0; j < N; j++) [
	    if (temp[0][j] < temp[1][j]) {
	       temp[0][j] = temp[1][j];
	       max[0] = max[1];
	    }
	 ]
      }
      
      for (j = 0; j < N; j++) [
         delta[k][j] = temp[0][j]*b[j][k];
         psi[k][j] = max[0];
      ]
   }
   
   // termination step
   u = N;
   for (m = N >> 1; m >= 1; m = m >> 1) {
      for (i = 0; i < m; i++) [
	 if (delta[T-1][2*i] < delta[T-1][2*i+1]) {
	    delta[T-1][i] = delta[T-1][2*i+1];
	    max[i] = 2*i+1;
	 }
	 else {
	    delta[T-1][i] = delta[T-1][2*i];
	    max[i] = 2*i;
	 }
      ]
      if (2 * m < u) {
	 delta[T-1][m] = delta[T-1][2*m];
	 u = m+1;
      }
      u = m;
   }
   if (u > 1) {
      if (delta[T-1][0] < delta[T-1][1]) {
	 delta[T-1][0] = delta[T-1][1];
	 max[0] = max[1];
      }
   }
   
   p = delta[T-1][0];
   q[T-1] = max[0];
   for (k = T-2; k >= 0; k--)
     q[k] = psi[k+1][q[k+1]];
   
   // output result
   smcoutput(p, 1);
   smcoutput(q, 1, T);
     
}
