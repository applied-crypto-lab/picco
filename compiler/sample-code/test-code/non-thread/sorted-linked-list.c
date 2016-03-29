
struct node {
   private int<50> data;
   struct node *next;
};

public int count = 16;

public int main() {

   struct node *head = 0;
   public int i, j;
   private int a, output, c, tmp;
   private int<1> found;
   struct node *ptr, *tmpptr;
   struct node *ptr1 = pmalloc(1, struct node);    
   // build the linked list
   for (i = 0; i < count; i++) {
      ptr = pmalloc(1, struct node);
      smcinput(a, 1);
      ptr->data = a;
      //ptr->next = 0;
      // insert the first element as the head
      if (i == 0) {
	 head = ptr;
      }
      else {
	 tmpptr = head;
	 if (ptr->data < head->data) { // insert in the beginning
	    ptr->next = head;
	    head = ptr;
	 }
	 else {
	    for (j = 0; j < i-1; j++) {
	       if ((tmpptr->data < a) && (tmpptr->next->data > a)) { // insert ptr
		  ptr->next = tmpptr->next;
		  tmpptr->next = ptr;
	       }
	       tmpptr = tmpptr->next;
	    }
	    //insert in the last position if hasn't been inserted yet
	    if (tmpptr->data < ptr->data)
	      tmpptr->next = ptr;
	 }
      }
   }

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
