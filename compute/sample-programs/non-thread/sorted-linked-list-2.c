// Implementation of sorted linked list using data update (DU)
// Source: Zhang, Blanton and Almashaqbeh, " Implementing Support for Pointers to Private Data in a General-Purpose Secure Multi-Party Compiler," 2017
// Figure 2 from full version, available at https://arxiv.org/abs/1509.01763 

struct node {
   int data;
   struct node *next;
};

public int count = 16;

public int main() {

   public int i, j;
   private int a[count], c, tmp, output;
   struct node *ptr1, *ptr2, *head;
   
   smcinput(a, 1, count);
   
   // build the linked list
   head = pmalloc(1, struct node);
   head->data = a[0];
   
   for (i = 1; i < count; i++) {
      ptr1 = pmalloc(1, struct node);

      // place the pointer in the beginning
      ptr1->data = a[i];
      ptr1->next = head;
      head = ptr1;
      
      // move the data if necessary
      ptr2 = head;
      for (j = 0; j < i; j++) {
	 if (ptr2->data > ptr2->next->data) {
	    tmp = ptr2->data;
	    ptr2->data = ptr2->next->data;
	    ptr2->next->data = tmp;
	 }
         ptr2 = ptr2->next; 
      }
   }
   // traverse the linked list searching for the first element greater than
   // the given value
   c = 5;  
   ptr1 = head;
   output = 0;
   for (i = 0; i < count; i++) {
      if (ptr1->data == c) 
	 output = output+1;
      ptr1 = ptr1->next;
   }
   smcoutput(output, 1);
   
   return 0;
}
