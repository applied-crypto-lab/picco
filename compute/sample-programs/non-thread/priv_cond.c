
public int main() {
   private int a, b, d; 
   private float fa, fb, fc, fd;  

   smcinput(a, 1);
   smcinput(b, 1);
   smcinput(fa, 1);
   smcinput(fb, 1);
   smcinput(fc, 1);
   
   printf("INT PRIVATE CONDITION...\n"); 
   if(a > b)
   	d = 1; 
   else
	d = 2; 
   smcoutput(d, 1); 
   printf("\n"); 
	
   printf("FLOAT PRIVATE CONDITION...\n"); 
   if(fa >  fb)
	fd = 1; 
   else 
	fd = 2; 
   smcoutput(fd, 1); 
   printf("\n"); 

   printf("COMBINATION\n"); 
   if(a < b){
	fa = 2; 
   	if(fb < fc){
		fd = 10; 
		d = 5; 
	}
   }
   smcoutput(fa, 1); 
   printf("\n"); 
   smcoutput(fd, 1); 
   printf("\n"); 
   smcoutput(d, 1); 
   printf("\n"); 

   if(a < b && fb < fc){
	fa = 2; 
   	if(a > b || fb > fc){
		fd = 20; 
		d = 10; 
	}
	else{
		fd = 30; 
		d = 20; 
	}
   }

   smcoutput(fa, 1); 
   printf("\n"); 
   smcoutput(fd, 1); 
   printf("\n"); 
   smcoutput(d, 1); 
   printf("\n"); 
   return 0;
}
