#include "PrivPtr.h"
PrivPtr::PrivPtr(NodeNetwork nodeNet, int nodeID, SecretShare *s) 
{
	Mul = new Mult(nodeNet, nodeID, s); 
	net = nodeNet;
        id = nodeID;
        ss = s;
}
int PrivPtr::creation = 0; 

PrivPtr::~PrivPtr() 
{

}
int PrivPtr::n = 0; 
listnode PrivPtr::create_listnode()
{
	//creation++; 
	//printf("%d\n", creation); 
	listnode node = (listnode)malloc(sizeof(struct listnode_)); 
	mpz_init(node->priv_tag);
	node->prev = node->next = NULL; 
	return node; 
}

listnode PrivPtr::create_listnode(mpz_t* int_var_loc, mpz_t** float_var_loc, void* struct_var_loc, priv_ptr ptr_loc, mpz_t private_tag, int index)
{
	listnode node = create_listnode();
	if(int_var_loc != NULL)
	{ 
		node->u.int_var_location = int_var_loc; 
	}
	else if(float_var_loc != NULL)
		node->u.float_var_location = float_var_loc;
	else if(struct_var_loc != NULL)
		node->u.struct_var_location = struct_var_loc; 
	else
		node->u.ptr_location = ptr_loc; 
	mpz_set(node->priv_tag, private_tag); 
	node->if_index = index; 
	node->prev = node->next = NULL; 
	return node; 
}

void PrivPtr::copy_listnode(listnode assign_node, listnode right_node, int level, int type)
{
	if(level == 0)
        {
		if(type == 0)	
			assign_node->u.int_var_location = right_node->u.int_var_location;
		else if(type == 1)
			assign_node->u.float_var_location = right_node->u.float_var_location; 
		else if(type == 2)
			assign_node->u.struct_var_location = right_node->u.struct_var_location; 
	} 
	else
		assign_node->u.ptr_location = right_node->u.ptr_location; 
	mpz_set(assign_node->priv_tag, right_node->priv_tag); 
	assign_node->if_index = right_node->if_index; 
}

dlist PrivPtr::create_list()
{
	dlist list = (dlist)malloc(sizeof(struct list_)); 
	list->head = create_listnode(); 
	list->tail = create_listnode(); 
	list->head->next = list->tail; 
	list->tail->prev = list->head; 
	return list; 
}

void PrivPtr::insert_to_rear(dlist list, listnode node)
{
	listnode tmp = list->tail->prev; 
	list->tail->prev = node; 
	node->next = list->tail; 
	node->prev = tmp; 
	tmp->next = node; 
}

void PrivPtr::delete_from_list(listnode* node)
{
	(*node)->prev->next = (*node)->next; 
	(*node)->next->prev = (*node)->prev; 
	destroy_listnode(node); 
}

void PrivPtr::clear_list(dlist* list)
{
	listnode tmp = (*list)->head->next;
	listnode tmp1;  
	while(tmp != (*list)->tail)
	{
		tmp1 = tmp->next; 
		delete_from_list(&tmp);	
		tmp = tmp1;  
	}
	
}

void PrivPtr::copy_list(dlist assign_list, dlist right_list, int level, int type)
{
	listnode pre, cur; 
	pre = assign_list->head; 
	cur = right_list->head->next; 
	while(cur != right_list->tail)
	{
		listnode node = create_listnode();
		copy_listnode(node, cur, level, type);
		pre->next = node; 
		node->prev = pre; 
		pre = node;   
		cur = cur->next; 
	}
	pre->next = assign_list->tail; 
	assign_list->tail->prev = pre; 
}

void PrivPtr::append_list(dlist front_list, dlist back_list)
{
	listnode tmp1 = back_list->head->next; 
	listnode tmp2 = NULL; 
	
	while(tmp1 != back_list->tail)
	{
		tmp2 = tmp1->next;
		insert_to_rear(front_list, tmp1); 		 
		tmp1 = tmp2; 
	}

	destroy_listnode(&(back_list->head)); 
	destroy_listnode(&(back_list->tail)); 

	free(back_list); 
	return; 
}

void PrivPtr::update_list_attributes(dlist list, mpz_t priv_cond, int if_index, int size, int threadID)
{
	if(size == 0)
		return;
	mpz_t* op1 = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* op2 = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* result = (mpz_t*)malloc(sizeof(mpz_t) * size); 

	//extract private tags of nodes in the list
	listnode tmp = list->head->next;
	int index = 0;  
	while(tmp != list->tail)
	{
		mpz_init_set(op1[index++], tmp->priv_tag); 
		tmp = tmp->next; 
	}
	for(int i = 0; i < size; i++)
	{
		mpz_init_set(op2[i], priv_cond); 
		mpz_init(result[i]); 
	}
	Mul->doOperation(result, op1, op2, size, threadID); 
	//store the updated private tags back to the list
	tmp = list->head->next; 
	index = 0; 
	while(tmp != list->tail)
	{
		mpz_set(tmp->priv_tag, result[index]);
		if(if_index != -1)
			tmp->if_index = if_index; 
		index++;  
		tmp = tmp->next; 
	}	
	//clear the memory for tmp variables
	for(int i = 0; i < size; i++)
	{
		mpz_clear(op1[i]); 
		mpz_clear(op2[i]); 
		mpz_clear(result[i]); 
	}
	free(op1); 
	free(op2); 
	free(result); 
}

void PrivPtr::destroy_listnode(listnode* node)
{
	mpz_clear((*node)->priv_tag); 
	free(*node);
	*node = NULL; 
}

void PrivPtr::destroy_list(dlist* list)
{
	listnode tmp = (*list)->head->next;
	listnode tmp1 = NULL;
	while(tmp != (*list)->tail)
	{
		tmp1 = tmp->next;
		destroy_listnode(&tmp);
		tmp = tmp1; 

	}
	destroy_listnode(&((*list)->head));
	destroy_listnode(&((*list)->tail)); 
	free(*list);
	*list = NULL;  
}

priv_ptr PrivPtr::create_ptr(int level, int type)
{
	priv_ptr ptr = (priv_ptr)malloc(sizeof(struct priv_ptr_)); 
	ptr->list = create_list(); 
	ptr->size = 0; 
	ptr->level = level;   
	ptr->type = type;
	return ptr;  
}

priv_ptr* PrivPtr::create_ptr(int level, int type, int num)
{
	priv_ptr* ptrs = (priv_ptr*)malloc(sizeof(priv_ptr) * num); 
	for(int i = 0; i < num; i++)
		ptrs[i] = create_ptr(level, type); 
	return ptrs; 
}

void PrivPtr::destroy_ptr(priv_ptr* ptr)
{
	destroy_list(&((*ptr)->list));
	free(*ptr);
	*ptr = NULL;  
}

void PrivPtr::destroy_ptr(priv_ptr** ptrs, int num)
{
	for(int i = 0; i < num; i++)
		destroy_ptr(&((*ptrs)[i]));
	free(*ptrs);
	*ptrs = NULL;  
}

void PrivPtr::set_ptr(priv_ptr ptr, mpz_t* int_var_loc, mpz_t** float_var_loc, void* struct_var_loc, priv_ptr* ptr_loc, int threadID)
{
	listnode node = create_listnode(); 
	if(int_var_loc != NULL)
	{
		node->u.int_var_location = int_var_loc;
		ptr->level = 1; 
	}
	else if(float_var_loc != NULL)
	{
		node->u.float_var_location = float_var_loc; 
		ptr->level = 1;  
	}
	else if(struct_var_loc != NULL)
	{
		node->u.struct_var_location = struct_var_loc; 
		ptr->level = 1; 
	}
	else
	{
		node->u.ptr_location = *ptr_loc;
		ptr->level = (*ptr_loc)->level; 
	} 
	clear_list(&(ptr->list));
	node->if_index = -1;
	mpz_set_ui(node->priv_tag, 1); 
	insert_to_rear(ptr->list, node); 
	ptr->size = 1; 
}

void PrivPtr::set_ptr(priv_ptr assign_ptr, priv_ptr right_ptr, int threadID)
{
	clear_list(&(assign_ptr->list)); 
	copy_list(assign_ptr->list, right_ptr->list, right_ptr->level-1, assign_ptr->type);
	assign_ptr->size = right_ptr->size; 
	assign_ptr->level = right_ptr->level;  
}

void PrivPtr::clear_ptr(priv_ptr* ptr)
{
	clear_list(&((*ptr)->list)); 
	(*ptr)->size = 0; 
	(*ptr)->level = 0;  
}

void PrivPtr::add_ptr(priv_ptr ptr, mpz_t* int_var_loc, mpz_t** float_var_loc, void* struct_var_loc, priv_ptr* ptr_loc, mpz_t private_tag, int threadID)
{	
	listnode node;
	if(ptr_loc != NULL) 
		node = create_listnode(int_var_loc, float_var_loc, struct_var_loc, *ptr_loc, private_tag, -1); 
	else
		node = create_listnode(int_var_loc, float_var_loc, struct_var_loc, NULL, private_tag, -1); 
	insert_to_rear(ptr->list, node); 
	ptr->size++; 
	return;  			
}

void PrivPtr::update_ptr(priv_ptr ptr, mpz_t* int_var_loc, mpz_t** float_var_loc, void* struct_var_loc, priv_ptr* ptr_loc, mpz_t private_tag, int index, int threadID)
{	
	listnode node;
	if(ptr_loc != NULL)
		node = create_listnode(int_var_loc, float_var_loc, struct_var_loc, *ptr_loc, private_tag, index); 
	else	
		node = create_listnode(int_var_loc, float_var_loc, struct_var_loc, NULL, private_tag, index); 

	if(ptr->size == 0)
	{	
		insert_to_rear(ptr->list, node); 
		ptr->size = 1; 
		return;  			
	}
	else
	{
		mpz_t priv_cond;
		mpz_init(priv_cond);  
		ss->modSub(priv_cond, 1, private_tag); 
		update_list_attributes(ptr->list, priv_cond, -1, ptr->size, threadID);
		insert_to_rear(ptr->list, node);
		ptr->size++; 
		mpz_clear(priv_cond); 	
		return; 
	}
}

void PrivPtr::update_ptr(priv_ptr assign_ptr, priv_ptr right_ptr, mpz_t private_tag, int index, int threadID)
{
	if(assign_ptr->size == 0)
	{
		copy_list(assign_ptr->list, right_ptr->list,right_ptr->level-1, assign_ptr->type);
		update_list_attributes(assign_ptr->list, private_tag, index, right_ptr->size, threadID);
		assign_ptr->size = right_ptr->size; 
		return;   
	}
	else
	{
		priv_ptr ptr = create_ptr(right_ptr->level, right_ptr->type); 
		ptr->size = right_ptr->size; 
		mpz_t priv_cond; 
		mpz_init(priv_cond); 
		ss->modSub(priv_cond, 1, private_tag); 
		copy_list(ptr->list, right_ptr->list, right_ptr->level-1, assign_ptr->type);
		update_list_attributes(ptr->list, private_tag, index, ptr->size, threadID); 
		update_list_attributes(assign_ptr->list, priv_cond, -1, assign_ptr->size, threadID); 
		merge_and_shrink_ptr(assign_ptr, ptr); 	
		mpz_clear(priv_cond); 
		destroy_ptr(&ptr); 
		return; 		
	}
}

void PrivPtr::shrink_ptr(priv_ptr ptr, int current_index, int parent_index, int threadID)
{
	//first, find the first node in the list that appears in the matching if body
	listnode first = ptr->list->head->next; 
	while(first != ptr->list->tail)
	{
		if(first->if_index >= current_index)
			break; 	
		first = first->next; 
	}
	 // second, remove all the nodes located in parallel -- use parent_index
	listnode tmp = ptr->list->head->next; 
        while(tmp != first)
	{
		if(tmp->if_index >= parent_index)
			break; 			
		tmp = tmp->next; 
	}
 	//remove all the nodes from tmp to the node right before first
	listnode tmp1 = NULL; 
	int index = 0;
	while(tmp != first)
	{	
		tmp1 = tmp->next; 
		delete_from_list(&tmp);
		index++;  
		tmp = tmp1; 
	}
	if(index > 0)
	{
		ptr->size -= index; 
	} 
	return; 
}

int PrivPtr::compute_list_size(dlist list)
{
	int size = 0; 
	listnode node = list->head->next; 
	while(node != list->tail)
	{
		size++; 
		node = node->next; 
	}
	return size; 
}

void PrivPtr::append_ptr_list(priv_ptr assign_ptr, priv_ptr right_ptr)
{
	dlist list = right_ptr->list; 
	listnode node = list->head->next; 
	while(node != list->tail)
	{
		append_list(assign_ptr->list, node->u.ptr_location->list); 
		node = node->next; 
	}
	return; 
}

int PrivPtr::is_repeated_listnode(dlist list, listnode node, int level, int type)
{
	listnode tmp = list->head->next;
	while(tmp != list->tail)
	{
		if(level == 0)
		{
			if((type == 0 && tmp->u.int_var_location == node->u.int_var_location)
				||(type == 1 && tmp->u.float_var_location == node->u.float_var_location)
				||(type == 2 && tmp->u.struct_var_location == node->u.struct_var_location))
			{
				ss->modAdd(tmp->priv_tag, tmp->priv_tag, node->priv_tag); 
				return 1;
			} 
		}
		else
		{
			if(tmp->u.ptr_location == node->u.ptr_location)
			{
				ss->modAdd(tmp->priv_tag, tmp->priv_tag, node->priv_tag); 
				return 1; 
			}
		}
		tmp = tmp->next; 
	}
	return 0; 
	
}

void PrivPtr::merge_and_shrink_ptr(priv_ptr assign_ptr, priv_ptr right_ptr)
{
	dlist list = right_ptr->list; 
	listnode node = list->head->next;
	int index = 0; 
	while(node != list->tail)
	{
		if(!is_repeated_listnode(assign_ptr->list, node, assign_ptr->level-1, assign_ptr->type))
		{
			listnode tmp = create_listnode(); 
			copy_listnode(tmp, node, assign_ptr->level-1, assign_ptr->type); 
			insert_to_rear(assign_ptr->list, tmp);
			assign_ptr->size++; 
		}
		node = node->next;
	}
	return; 
}

void PrivPtr::copy_nested_ptr(priv_ptr assign_ptr, priv_ptr right_ptr, mpz_t* R)
{
	int index = 0;
	dlist list = right_ptr->list; 
	listnode node = list->head->next;  
	while(node != list->tail)
	{
		dlist child_list = node->u.ptr_location->list;
		listnode child_node = child_list->head->next; 
		while(child_node != child_list->tail)
		{
			listnode tmp = create_listnode();
			copy_listnode(tmp, child_node, assign_ptr->level-1, assign_ptr->type);  
			mpz_set(tmp->priv_tag, R[index]); 
			insert_to_rear(assign_ptr->list, tmp);
			assign_ptr->size++;  
			index++; 
			child_node = child_node->next; 
		}
		node = node->next; 
	}
}

void PrivPtr::reduce_dereferences(priv_ptr ptr, int dereferences, priv_ptr result_ptr, int threadID)
{
	priv_ptr runner = create_ptr(0, ptr->type);  
	set_ptr(runner, ptr, threadID);
	while(dereferences > 1)
	{
		read_write_helper(runner, result_ptr, NULL, threadID);
		set_ptr(runner, result_ptr, threadID);
		dereferences--;
	}
	set_ptr(result_ptr, runner, threadID); 
	destroy_ptr(&runner);
	return; 
}
//dereference read for pointer to integer
void PrivPtr::dereference_ptr_read_var(priv_ptr ptr, mpz_t result, int dereferences, int threadID)
{
        if(ptr->type != 0)
	{
		printf("WRONG TYPE on the left operator...\n"); 
		exit(1); 
	}
	priv_ptr copy_ptr = create_ptr(0, 0);
        priv_ptr ptr1 = create_ptr(0, 0);
        priv_ptr tmp_ptr;
        set_ptr(copy_ptr, ptr, threadID);
        if(dereferences > 1)
        {
                reduce_dereferences(copy_ptr, dereferences, ptr1, threadID);
                tmp_ptr = ptr1;
        }
        else
                tmp_ptr = ptr;

       int size = tmp_ptr->size;
       int index = 0;
       listnode tmp = tmp_ptr->list->head->next;
       if(size == 1)
       {
		mpz_set(result, *(tmp->u.int_var_location)); 
		return; 
       }
       mpz_t* op1 = (mpz_t*)malloc(sizeof(mpz_t) * size);
       mpz_t* op2 = (mpz_t*)malloc(sizeof(mpz_t) * size);
       mpz_t* product = (mpz_t*)malloc(sizeof(mpz_t) * size);
       for(int i = 0; i < size; i++)
       {
             mpz_init(op1[i]);
             mpz_init(op2[i]);
             mpz_init(product[i]);
       }
       while(tmp != tmp_ptr->list->tail)
       {
             mpz_set(op1[index], *(tmp->u.int_var_location));
             mpz_set(op2[index], tmp->priv_tag);
             mpz_set_ui(product[index], 0);
             index++;
             tmp = tmp->next;
       }
       Mul->doOperation(product, op1, op2, size, threadID);
       mpz_set_ui(result, 0);
       for(int i = 0; i < size; i++)
           ss->modAdd(result, result, product[i]);
       for(int i = 0; i < size; i++)
       {
           mpz_clear(op1[i]);
           mpz_clear(op2[i]);
           mpz_clear(product[i]);
       }
       free(op1);
       free(op2);
       free(product);
       destroy_ptr(&copy_ptr);
       destroy_ptr(&ptr1);
}

//dereference read of pointer to float
void PrivPtr::dereference_ptr_read_var(priv_ptr ptr, mpz_t* result, int dereferences, int threadID)
{
        if(ptr->type != 1)
	{
		printf("WRONG TYPE on the left operator...\n"); 
		exit(1); 
	}

        priv_ptr copy_ptr = create_ptr(0, 1);
        priv_ptr ptr1 = create_ptr(0, 1);
        priv_ptr tmp_ptr;
        set_ptr(copy_ptr, ptr, threadID);
        if(dereferences > 1)
        {
                reduce_dereferences(copy_ptr, dereferences, ptr1, threadID);
                tmp_ptr = ptr1;
        }
        else
                tmp_ptr = ptr;

        int size = tmp_ptr->size;
        int index = 0;
        listnode tmp = tmp_ptr->list->head->next;

	if(size == 1)
	{ 
		for(int i = 0; i < 4; i++)
			mpz_set(result[i], (*(tmp->u.float_var_location))[i]); 
		return; 
	}

        mpz_t* op1 = (mpz_t*)malloc(sizeof(mpz_t) * 4 * size);
        mpz_t* op2 = (mpz_t*)malloc(sizeof(mpz_t) * 4 * size);
        mpz_t* product = (mpz_t*)malloc(sizeof(mpz_t) * 4 * size);
        for(int i = 0; i < 4 * size; i++)
        {
                mpz_init(op1[i]);
                mpz_init(op2[i]);
                mpz_init(product[i]);
        }
        while(tmp != tmp_ptr->list->tail)
        {
                for(int i = 0; i < 4; i++)
                {
                        mpz_set(op1[4*index+i], (*(tmp->u.float_var_location))[i]);
                        mpz_set(op2[4*index+i], tmp->priv_tag);
                        mpz_set_ui(product[4*index+i], 0);
                }
                index++;
                tmp = tmp->next;
        }
        Mul->doOperation(product, op1, op2, 4 * size, threadID);
        for(int i = 0; i < 4; i++)
            mpz_set_ui(result[i], 0);

        for(int i = 0; i < size; i++)
            for(int j = 0; j <  4; j++)
                ss->modAdd(result[j], result[j], product[4*i+j]);
        for(int i = 0; i < 4 * size; i++)
        {
                mpz_clear(op1[i]);
                mpz_clear(op2[i]);
                mpz_clear(product[i]);
        }
        free(op1);
        free(op2);
        free(product);
        destroy_ptr(&copy_ptr);
        destroy_ptr(&ptr1);
}


void PrivPtr::dereference_ptr_read_ptr(priv_ptr ptr, priv_ptr result, int dereferences, mpz_t priv_cond, int threadID)
{
	priv_ptr copy_ptr = create_ptr(ptr->level, ptr->type);
	priv_ptr ptr1 = create_ptr(ptr->level, ptr->type); 
	set_ptr(ptr1, ptr, threadID); 
	priv_ptr tmp_ptr; 
	if(dereferences > 1)
	{
		reduce_dereferences(ptr1, dereferences, copy_ptr, threadID); 
		tmp_ptr = copy_ptr; 
	} 
	else 
		tmp_ptr = ptr;
	read_write_helper(tmp_ptr, result, priv_cond, threadID); 
	destroy_ptr(&copy_ptr); 
	destroy_ptr(&ptr1); 
}

void PrivPtr::read_write_helper(priv_ptr ptr, priv_ptr result, mpz_t priv_cond, int threadID)
{
	// compute the number of elements in the second layer of list of ptr 
	int num = 0; 
	dlist list = ptr->list; 
	listnode node = list->head->next; 
	while(node != list->tail)
	{
		num += compute_list_size(node->u.ptr_location->list); 
		node = node->next; 
	}
	mpz_t* op1 = (mpz_t*)malloc(sizeof(mpz_t) * num); 
	mpz_t* op2 = (mpz_t*)malloc(sizeof(mpz_t) * num); 
	mpz_t* op3 = (mpz_t*)malloc(sizeof(mpz_t) * num); 
	mpz_t* R = (mpz_t*)malloc(sizeof(mpz_t) * num); 

	for(int i = 0; i < num; i++)
	{
		mpz_init(op1[i]); 
		mpz_init(op2[i]); 
		mpz_init(op3[i]); 
		mpz_init(R[i]); 
	}
	//fill the values for op1 and op2
	int index = 0;
	node = list->head->next;  
	
	while(node != list->tail)
	{
		dlist child_list = node->u.ptr_location->list; 
		listnode child_node = child_list->head->next; 
		while(child_node != child_list->tail)
		{
			mpz_set(op1[index], node->priv_tag); 
			mpz_set(op2[index], child_node->priv_tag);
			if(priv_cond != NULL)
				mpz_set(op3[index], priv_cond); 
			index++; 
			child_node = child_node->next; 
		}
		node = node->next; 
	}
 		
	Mul->doOperation(R, op1, op2, num, threadID); 
	if(priv_cond != NULL)
	{	
		mpz_t cond;
		mpz_init(cond);  
		ss->modSub(cond, 1, priv_cond); 
		update_list_attributes(result->list, cond, -1, result->size, threadID);
		Mul->doOperation(R, R, op3, num, threadID); 
	}
	else
	{
		clear_list(&(result->list));
		result->size = 0; 
	}

	result->level = ptr->level-1; 
	priv_ptr tmp_ptr = create_ptr(ptr->level-1, ptr->type); 
	copy_nested_ptr(tmp_ptr, ptr, R);
	merge_and_shrink_ptr(result, tmp_ptr);
	destroy_ptr(&tmp_ptr);

	for(int i = 0; i < num; i++)
	{
		mpz_clear(op1[i]); 
		mpz_clear(op2[i]); 
		mpz_clear(op3[i]); 
		mpz_clear(R[i]); 
	}
	free(op1); 
	free(op2); 
	free(op3); 
	free(R); 
}

int PrivPtr::list_size(dlist list)
{
	int index = 0; 
	listnode node = list->head->next, node1;
	while(node != list->tail)
	{
		index++;
		node = node->next; 
	}
	return index; 
}


void PrivPtr::dereference_ptr_write(priv_ptr ptr, mpz_t* int_var_loc, mpz_t** float_var_loc, void* struct_var_loc, priv_ptr* ptr_loc, int dereferences, mpz_t priv_cond, int threadID)
{
	priv_ptr tmp_ptr = create_ptr(0, ptr->type); 
	set_ptr(tmp_ptr, int_var_loc, float_var_loc, struct_var_loc, ptr_loc, threadID);
	dereference_ptr_write_ptr(ptr, tmp_ptr, dereferences, priv_cond, threadID); 
	destroy_ptr(&tmp_ptr); 
}

void PrivPtr::dereference_ptr_write_ptr(priv_ptr ptr, priv_ptr value, int dereferences, mpz_t priv_cond, int threadID)
{
	priv_ptr copy_ptr = create_ptr(0, ptr->type);
	priv_ptr ptr1 = create_ptr(0, ptr->type); 
	set_ptr(copy_ptr, ptr, threadID); 
	priv_ptr tmp_ptr; 

	if(dereferences > 1)
	{
		reduce_dereferences(copy_ptr, dereferences, ptr1, threadID);  
		tmp_ptr = ptr1; 
	}
	else 
		tmp_ptr = ptr;

	dlist list = tmp_ptr->list; 
	listnode node = list->head->next; 
	  	
	while(node != list->tail)
	{
		priv_ptr ptr2 = create_ptr(0, ptr->type); 
		priv_ptr assign_ptr = node->u.ptr_location; 
		set_ptr(ptr2, value, threadID);
		mpz_t cond; 
		mpz_init(cond); 
		if(priv_cond == NULL)
		{
			ss->modSub(cond, 1, node->priv_tag);
			update_list_attributes(assign_ptr->list, cond, -1, assign_ptr->size, threadID);  
			update_list_attributes(ptr2->list, node->priv_tag, -1, ptr2->size, threadID);
		}
		else
		{	
			mpz_t* op1 = (mpz_t*)malloc(sizeof(mpz_t)); 	
			mpz_t* op2 = (mpz_t*)malloc(sizeof(mpz_t)); 
			mpz_t* R = (mpz_t*)malloc(sizeof(mpz_t)); 
			mpz_init_set(op1[0], priv_cond); 
			mpz_init_set(op2[0], node->priv_tag);
			mpz_init(R[0]); 
			Mul->doOperation(R, op1, op2, 1, threadID);
			ss->modSub(cond, 1, R[0]); 
			update_list_attributes(assign_ptr->list, cond, -1, assign_ptr->size, threadID); 
			update_list_attributes(ptr2->list, R[0], -1, ptr2->size, threadID); 
			
			mpz_clear(op1[0]); 
			mpz_clear(op2[0]); 
			mpz_clear(R[0]); 

			free(op1); 
			free(op2); 
			free(R);  
		}
		merge_and_shrink_ptr(assign_ptr, ptr2); 		
		destroy_ptr(&ptr2); 
		node = node->next; 
	}	
	destroy_ptr(&copy_ptr); 
	destroy_ptr(&ptr1); 
	return; 
}

//dereference write for pointer to float
void PrivPtr::dereference_ptr_write_var(priv_ptr ptr, mpz_t* value, int dereferences, mpz_t priv_cond, int threadID)
{
        if(ptr->type != 1)
	{
		printf("WRONG TYPE on the left operator...\n"); 
		exit(1); 
	}

        priv_ptr tmp_ptr;
        priv_ptr copy_ptr = create_ptr(0, 1);
        priv_ptr ptr1 = create_ptr(0, 1);
        set_ptr(ptr1, ptr, threadID);

        if(dereferences > 1)
        {
                reduce_dereferences(ptr1, dereferences, copy_ptr, threadID);
                tmp_ptr = copy_ptr;
        }
        else
                tmp_ptr = ptr;

        int index = 0;
        int size = tmp_ptr->size;
        listnode tmp = tmp_ptr->list->head->next;

        mpz_t* op1 = (mpz_t*)malloc(sizeof(mpz_t) * 4 * size);
        mpz_t* op2 = (mpz_t*)malloc(sizeof(mpz_t) * 4 * size);
        mpz_t* op3 = (mpz_t*)malloc(sizeof(mpz_t) * 4 * size);
        mpz_t* op4 = (mpz_t*)malloc(sizeof(mpz_t) * 4 * size);

        for(int i = 0; i < 4 * size; i++)
        {
                mpz_init(op1[i]);
                mpz_init(op2[i]);
                mpz_init(op3[i]);
                mpz_init(op4[i]);
        }
	
        while(tmp != tmp_ptr->list->tail)
        {
                for(int i = 0; i < 4; i++)
                {
                        mpz_set(op1[4*index+i], (*(tmp->u.float_var_location))[i]);
                        mpz_set(op2[4*index+i], tmp->priv_tag);
                        if(priv_cond != NULL)
                                mpz_set(op3[4*index+i], priv_cond);
                        mpz_set(op4[4*index+i], value[i]);
                }
                index++;
                tmp = tmp->next;
        }
        /* compute (*location) + tag * priv_cond * (value-(*location))*/
        ss->modSub(op4, op4, op1, 4*size);
        if(priv_cond != NULL)
                Mul->doOperation(op4, op4, op3, 4*size, threadID);
	if(size != 1)
        	Mul->doOperation(op4, op4, op2, 4*size, threadID);
        ss->modAdd(op4, op1, op4, 4*size);

        tmp = tmp_ptr->list->head->next;
        index = 0;
        while(tmp != tmp_ptr->list->tail)
        {
                for(int i = 0; i < 4; i++)
                        mpz_set((*(tmp->u.float_var_location))[i], op4[4*index+i]);
                index++;
                tmp = tmp->next;
        }

        for(int i = 0; i < 4*size; i++)
        {
                mpz_clear(op1[i]);
                mpz_clear(op2[i]);
                mpz_clear(op3[i]);
                mpz_clear(op4[i]);
        }

        free(op1);
        free(op2);
        free(op3);
        free(op4);

        destroy_ptr(&ptr1);
        destroy_ptr(&copy_ptr);
        return;
}

//dereference write to a pointer to int
void PrivPtr::dereference_ptr_write_var(priv_ptr ptr, mpz_t value, int dereferences, mpz_t priv_cond, int threadID)
{
        if(ptr->type != 0)
	{
		printf("WRONG TYPE on the left operator...\n"); 
		exit(1); 
	}
        priv_ptr tmp_ptr;
        priv_ptr copy_ptr = create_ptr(0, 0);
        priv_ptr ptr1 = create_ptr(0, 0);
        set_ptr(ptr1, ptr, threadID);

        if(dereferences > 1)
        {
                reduce_dereferences(ptr1, dereferences, copy_ptr, threadID);
                tmp_ptr = copy_ptr;
        }
        else
                tmp_ptr = ptr;

        int index = 0;
        int size = tmp_ptr->size;
        listnode tmp = tmp_ptr->list->head->next;

        mpz_t* op1 = (mpz_t*)malloc(sizeof(mpz_t) * size);
        mpz_t* op2 = (mpz_t*)malloc(sizeof(mpz_t) * size);
        mpz_t* op3 = (mpz_t*)malloc(sizeof(mpz_t) * size);
        mpz_t* op4 = (mpz_t*)malloc(sizeof(mpz_t) * size);

        for(int i = 0; i < size; i++)
        {
                mpz_init(op1[i]);
                mpz_init(op2[i]);
                mpz_init(op3[i]);
                mpz_init(op4[i]);
        }

        while(tmp != tmp_ptr->list->tail)
        {
                mpz_set(op1[index], *(tmp->u.int_var_location));
                mpz_set(op2[index], tmp->priv_tag);
                if(priv_cond != NULL)
                        mpz_set(op3[index], priv_cond);
                mpz_set(op4[index], value);
                index++;
                tmp = tmp->next;
        }
        /* compute (*location) + tag * priv_cond * (value-(*location)) */
        ss->modSub(op4, op4, op1, size);
        if(priv_cond != NULL)
                Mul->doOperation(op4, op4, op3, size, threadID);
	if(size != 1)
        	Mul->doOperation(op4, op4, op2, size, threadID);
        ss->modAdd(op4, op1, op4, size);
        /* update the values of nodes in the list (not update the tags) */
        tmp = tmp_ptr->list->head->next;
        index = 0;
        while(tmp != tmp_ptr->list->tail)
        {
                mpz_set(*(tmp->u.int_var_location), op4[index++]);
                tmp = tmp->next;
        }
	for(int i = 0; i < size; i++)
        {
                mpz_clear(op1[i]);
                mpz_clear(op2[i]);
                mpz_clear(op3[i]);
                mpz_clear(op4[i]);
        }

        free(op1);
        free(op2);
        free(op3);
        free(op4);

        destroy_ptr(&ptr1);
        destroy_ptr(&copy_ptr);
        return;
}

