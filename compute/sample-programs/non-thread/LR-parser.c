
public int K = 100; // the maximum number of variables in the expression
                    // this defines integer representation of symbols
// + = K
// * = K+1
// ( = K+2
// ) = K+3 
// EOF = K+4

// example used on the input data: (1+2)*(3+4)*(5*6)*(7+8)+(9+10)EOF

public int M = 10; // the number of variables in the expression
public int S = 30; // the length of the expression

struct token{
	private int val; 
	public int type;
	struct token* next; 
}; 

// type == 0 --- id
// type == 1 --- F 
// type == 2 --- T			
// type == 3 --- S
// type == 4 --- + 
// type == 5 --- * 
// type == 6 --- (
// type == 7 --- )

// another problem is that it is not possible to pass the array as parameters to a sub-protocol
struct token *pop(struct token** header)
{
	struct token* t = *header; 
	struct token* tmp = *header; 
	*header = tmp->next; // (*header)->next = *header does not work ---- fix it later 
	return t;  	
}

public void push(struct token** header, struct token* t)
{
	t->next = *header; 
	*header = t;  
}

public void id_routine(struct token** header, int val)
{
	//struct token* t = pmalloc(1, struct token); -- works incorrectly
	struct token* t; 
	t = pmalloc(1, struct token); 
	t->type = 0; 
	t->val = val;/******** WRGONG ***************/ 
	push(header, t); 
}
public void check_for_removable_lbra(struct token** header)
{
	struct token* t; 
	t = pop(header); 
	if(t->type != 6)
		push(header, t); 
}

public void prod_sub_routine(struct token** header, struct token* x1, public int flag)
{
	struct token* x3; 
	x3 = pop(header); 
	x1->type = 2; //T
	x1->val =  x3->val * x1->val; 
	if(*header != 0)
	{
		struct token* x4; 
		x4 = pop(header); 
		if(x4->type == 4) //+
		{
			struct token* x5; 
			x5 = pop(header);
			x1->val = x1->val + x5->val; 
			x1->type = 3;
		}
		else
			push(header, x4);
	}
	if(flag == 1)
		check_for_removable_lbra(header);  
	push(header, x1); 
}
public void plus_sub_routine(struct token** header, struct token* x1, public int flag)
{
	struct token* x3; 
	x3 = pop(header); 
	x1->type = 3; 
	x1->val = x1->val + x3->val;
	if(flag == 1)
		check_for_removable_lbra(header);  
	push(header, x1); 
}

public void plus_routine(struct token** header)
{	
	struct token* plus; 
	plus = pmalloc(1, struct token); 
	plus->type = 4; 
	struct token* x1; 
	x1 = pop(header); 
	if(*header != 0)
	{
		 if(x1->type == 0) // id
			x1->type == 1; // F
		  struct token* x2; 
		  x2 = pop(header); 
		  if(x2->type == 5) // *
			prod_sub_routine(header, x1, 0); 
		  else if(x2->type == 4) // +
			plus_sub_routine(header, x1, 0); 
		  else if(x2->type == 6) // (
		  {
		  	x1->type = 3; // S
			push(header, x2); 
			push(header, x1); 
		  } 
	}
	else
	{
		x1->type = 3; 
		push(header, x1);  
	}
	push(header, plus); 
}

public void prod_routine(struct token** header)
{
 	struct token* prod; 
	prod = pmalloc(1, struct token); 
	prod->type = 5; 

	struct token* x1;
        x1 = pop(header);
        if(*header != 0)
        {
                 if(x1->type == 0) // id
                        x1->type == 1; // F
                  struct token* x2;
                  x2 = pop(header);
                  if(x2->type == 5){
		  	struct token* x3; 
			x3 = pop(header); 
			x1->type = 2; 
			x1->val = x1->val * x3->val; 
			push(header, x1); 
		   } // *
                  else if(x2->type == 4 || x2->type == 6) // + or (
                  {
                        x1->type = 2; // T 
                        push(header, x2);
                        push(header, x1);
                  }
        }
        else
        {
                x1->type = 2;
                push(header, x1);
        }
	push(header, prod); 
}

public void lbra_routine(struct token** header)
{
	struct token* t; 
	t = pmalloc(1, struct token); 
	t->type = 6; 
	push(header, t);  
}

public void rbra_routine(struct token** header)
{
        struct token* x1;
        x1 = pop(header);
        if(*header != 0)
        {
                 if(x1->type == 0) // id
                        x1->type == 1; // F
                  struct token* x2;
                  x2 = pop(header);
                  if(x2->type == 5) // *
                        prod_sub_routine(header, x1, 1);
                  else if(x2->type == 4) // +
                        plus_sub_routine(header, x1, 1);
		  else if(x2->type == 6)
		  {
			x1->type = 1; 
			push(header, x1); 
		  }
	}
}

public void eof_routine(struct token** header)
{
        struct token* x1;
        x1 = pop(header);
	int result = 0; 
        if(*header != 0)
        {
                 if(x1->type == 0) // id
                        x1->type == 1; // F
                  struct token* x2;
                  x2 = pop(header);
                  if(x2->type == 5) // *
                        prod_sub_routine(header, x1, 0);
                  else if(x2->type == 4) // +
                        plus_sub_routine(header, x1, 0);
		  x1 = pop(header); 
		  result = x1->val; 
		  smcoutput(result, 1); 
        }
	else{
		result = x1->val; 
		smcoutput(result, 1);//output the result
	}
}
public int main() {
	private int ids[M]; 
	public int expr[S];

	struct token *header = 0; //header of the stack
	public int index = 0;
	public int symbol = 0; 

	smcinput(expr, 1, S);
	smcinput(ids, 1, M);

	while(index < S)
	{
		symbol = expr[index]; 
		if(symbol < K) //id
			id_routine(&header, ids[symbol]); 	
		else if(symbol == K) //+
			plus_routine(&header); 
		else if(symbol == K+1) //*
			prod_routine(&header); 
		else if(symbol == K+2) //(
			lbra_routine(&header); 
		else if(symbol == K+3) //)
			rbra_routine(&header); 
		else if(symbol == K+4) //EOF
			eof_routine(&header); 	
		index = index+1; 
	}
	return 1; 
} 
