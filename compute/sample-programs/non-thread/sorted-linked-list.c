// Implementation of sorted linked list using pointer update
// Source: Zhang, Blanton and Almashaqbeh, " Implementing Support for Pointers to Private Data in a General-Purpose Secure Multi-Party Compiler," 2017
// Figure 3 from full version, available at https://arxiv.org/abs/1509.01763 
// This program supports an array of inputs a[] which contains UNIQUE VALUES (NO DUPLICATES)

struct node {
private int<50> data;
    struct node *next;
};

public int count = 3;

public int main() {

    public int i, j;
    private int a[count], output, c;
    struct node *ptr1, *ptr2, *head;

    smcinput(a, 1, count);

    // build the linked list
    ptr1 = pmalloc(1, struct node);
    ptr2 = pmalloc(1, struct node);
    ptr1->data = a[0];
    ptr2->data = a[1];
    if (a[0] < a[1]) {
        head = ptr1;
        head->next = ptr2;
    } else {
        head = ptr2;
        head->next = ptr1;
    }

    for (i = 2; i < count; i++) {
        ptr1 = pmalloc(1, struct node);
        ptr1->data = a[i];
        // ptr1->next = 0; // end of list (Null) currently not supported
        ptr2 = head;

        if (ptr1->data < ptr2->data) { // insert in the beginning
            ptr1->next = ptr2;
            head = ptr1;
        }
        for (j = 0; j < i - 1; j++) {
            if ((ptr2->data < ptr1->data) && (ptr2->next->data >= ptr1->data)) { // insert ptr
                ptr1->next = ptr2->next;
                ptr2->next = ptr1;
            }
            ptr2 = ptr2->next;
        }
        // insert in the last position if hasn't been inserted yet
        if (ptr2->data < ptr1->data) 
            ptr2->next = ptr1;
    }

    c = 5;
    ptr1 = head;
    output = 0;
    for (i = 0; i < count; i++) {
        if (ptr1->data == c)
            output = output + 1;
        ptr1 = ptr1->next;
    }

    smcoutput(output, 1);
    return 0;
}
