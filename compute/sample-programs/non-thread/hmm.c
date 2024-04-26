/* Hidden Markov Model evaluation using Viterbi algorithm */

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
   private int max[(N+1)/2][N];
   private float p;
   private int q[T];
   
   public int i, j, k, m;
   
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
      // compute the product of delta and a elements in parallel
      for (j = 0; j < N; j++) [
         for (i = 0; i < N; i++) [
            temp[i][j] = delta[k-1][i]*a[i][j];
         ]		
      ]
      // compute max and argmax of the products as a tree			
      for (m = N; m > 1; m = (m+1) >> 1) {
         for (j = 0; j < N; j++) [
            for (i = 0; i < m/2; i++) [
               if (temp[2*i][j] < temp[2*i+1][j]) {
                  temp[i][j] = temp[2*i+1][j];
                  max[i][j] = 2*i+1;
               }
               else {
                  temp[i][j] = temp[2*i][j];
                  max[i][j] = 2*i;
               }
            ]
            if ((m % 2) == 1) {
               temp[m/2][j] = temp[m-1][j];
               max[m/2][j] = m-1;
            }
         ]
      }
      // copy the result
      for (j = 0; j < N; j++) [
         delta[k][j] = temp[0][j]*b[j][k];
         psi[k][j] = max[0][j];
      ]
   }
   
   // termination step
   for (m = N; m > 1; m = (m+1) >> 1) {
      for (i = 0; i < m/2; i++) [
         if (delta[T-1][2*i] < delta[T-1][2*i+1]) {
            delta[T-1][i] = delta[T-1][2*i+1];
            max[i][0] = 2*i+1;
         }
         else {
            delta[T-1][i] = delta[T-1][2*i];
            max[i][0] = 2*i;
         }
      ]
      if ((m % 1) == 1) {
         delta[T-1][m/2] = delta[T-1][m-1];
         max[m/2][0] = m-1;
      }
   }
   
   p = delta[T-1][0];
   q[T-1] = max[0][0];
   for (k = T-2; k >= 0; k--)
     q[k] = psi[k+1][q[k+1]];
   
   // output result
   smcoutput(p, 1);
   smcoutput(q, 1, T);
     
}
