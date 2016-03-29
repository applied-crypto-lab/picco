
struct node {
   int data;
   struct node *next;
};

public int count = 32; 

public int main() {

   struct node *head = 0;
   public int i;
   private int a, c, output, difference, tmp;
   private int array[count]; 
   private int result[count]; 
 
   struct node *ptr;
   // build the linked list
   for (i = 0; i < count; i++) {
      ptr = pmalloc(1, struct node);
      smcinput(a, 1);
      ptr->data = a;
      ptr->next = head;
      head = ptr;
   }
   // traverse the linked list searching for the element closest to the
   // the given value
  
   a = 10; 
   ptr = head;
   for(i = 0; i < count; i++)
   {
	array[i] = ptr->data; 
	ptr = ptr->next; 
   }

   for(i = 0; i < count; i++)
   [ 
      if(array[i] == a)
	result[i] = 1; 
   ]
   
   for(i = 0; i < count; i++)
   	output = output + result[i]; 
   smcoutput(output, 1);

   return 0;
}
