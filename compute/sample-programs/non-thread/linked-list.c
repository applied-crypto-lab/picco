// Construction and traversal of a linked list
// Source: Zhang, Blanton and Almashaqbeh, " Implementing Support for Pointers to Private Data in a General-Purpose Secure Multi-Party Compiler," 2017
// Figure 1 from full version, available at https://arxiv.org/abs/1509.01763 

struct node {
   int data;
   struct node *next;
};

public int count = 50; 

public int main() {

   struct node *head = 0;
   public int i;
   private int a[count], c, output;
   struct node *ptr;
   
   smcinput(a, 1, count);
   // build the linked list
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
      if (c == ptr->data)
	output = output+1; 
      ptr = ptr->next;
   }
   smcoutput(output, 1);
   return 0;
}
