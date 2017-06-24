
struct node {
   int data;
   struct node *next;
};

public int count = 32; 

public int main() {

   struct node *head = 0;
   public int i;
   private int a[count], c, output;
   private int array[count]; 
   private int result[count]; 
 
   struct node *ptr;
   // build the linked list
   smcinput(a, 1, count);
   for (i = 0; i < count; i++) {
      ptr = pmalloc(1, struct node);
      ptr->data = a[i];
      ptr->next = head;
      head = ptr;
   }
   // traverse the linked list searching for the element closest to the
   // the given value
   c = 10; 
   ptr = head;
   for(i = 0; i < count; i++)
   {
	array[i] = ptr->data; 
	ptr = ptr->next; 
   }

   for(i = 0; i < count; i++)
   [ 
      if(array[i] == c)
	result[i] = 1; 
   ]
   
   for(i = 0; i < count; i++)
   	output = output + result[i]; 
   smcoutput(output, 1);

   return 0;
}
