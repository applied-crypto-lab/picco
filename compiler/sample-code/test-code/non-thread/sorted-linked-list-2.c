
struct node {
   int data;
   struct node *next;
};

public int count = 16; 
public int main() {

   struct node *head = 0;
   public int i, j;
   private int a, c, tmpdata, output;
   private int<1> found;
   struct node *ptr, *tmpptr;
   
   // build the linked list
   for (i = 0; i < count; i++) {
      ptr = pmalloc(1, struct node);
      smcinput(a, 1);
      // place the pointer in the beginning
      ptr->data = a;
      ptr->next = head;
      head = ptr;
      // move the data if necessary
      tmpptr = head;
      for (j = 0; j < i; j++) {
	 if (tmpptr->data > tmpptr->next->data) {
	    tmpdata = tmpptr->data;
	    tmpptr->data = tmpptr->next->data;
	    tmpptr->next->data = tmpdata;
	 }
         tmpptr = tmpptr->next; 
      }
   }
   // traverse the linked list searching for the first element greater than
   // the given value
   a = 5;  
   ptr = head;
   for (i = 0; i < count; i++) {
      if (ptr->data == a) 
	 output = output+1;
      ptr = ptr->next;
   }
   smcoutput(output, 1);
   
   return 0;
}
