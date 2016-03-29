/*   
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame

   PICCO is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   PICCO is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with PICCO. If not, see <http://www.gnu.org/licenses/>.
*/
#include "IntPtr.h"
IntPtr::IntPtr(NodeNetwork nodeNet, int nodeID, SecretShare *s) 
{
	Mul = new Mult(nodeNet, nodeID, s); 
	net = nodeNet;
        id = nodeID;
        ss = s;
}

IntPtr::~IntPtr() 
{

}

listnode_int IntPtr::create_listnode()
{
	listnode_int node = (listnode_int)malloc(sizeof(struct listnode_int_)); 
	mpz_init(node->priv_tag);
	node->prev = node->next = NULL; 
	return node; 
}

listnode_int IntPtr::create_listnode(mpz_t* var_loc, priv_int_ptr ptr_loc, mpz_t private_tag, int index)
{
	listnode_int node = create_listnode(); 
	if(var_loc != NULL) 
		node->u.var_location = var_loc; 
	else	      
		node->u.ptr_location = ptr_loc; 
	mpz_set(node->priv_tag, private_tag); 
	node->if_index = index; 
	node->prev = node->next = NULL; 
	return node; 
}

void IntPtr::copy_listnode(listnode_int assign_node, listnode_int right_node, int level)
{
	if(level == 0)
		assign_node->u.var_location = right_node->u.var_location; 
	else
		assign_node->u.ptr_location = right_node->u.ptr_location; 
	mpz_set(assign_node->priv_tag, right_node->priv_tag); 
	assign_node->if_index = right_node->if_index; 
}

list_int IntPtr::create_list()
{
	list_int list = (list_int)malloc(sizeof(struct list_int_)); 
	list->head = create_listnode(); 
	list->tail = create_listnode(); 
	list->head->next = list->tail; 
	list->tail->prev = list->head; 
	return list; 
}

void IntPtr::insert_to_rear(list_int list, listnode_int node)
{
	listnode_int tmp = list->tail->prev; 
	list->tail->prev = node; 
	node->next = list->tail; 
	node->prev = tmp; 
	tmp->next = node; 
}

void IntPtr::delete_from_list(listnode_int node)
{
	node->prev->next = node->next; 
	node->next->prev = node->prev; 
	destroy_listnode(node); 
}

void IntPtr::clear_list(list_int list)
{
	listnode_int tmp = list->head->next;
	listnode_int tmp1;  
	while(tmp != list->tail)
	{
		tmp1 = tmp->next; 
		delete_from_list(tmp);	
		tmp = tmp1;  
	}
}

void IntPtr::copy_list(list_int assign_list, list_int right_list, int level)
{
	listnode_int pre, cur; 
	pre = assign_list->head; 
	cur = right_list->head->next; 
	while(cur != right_list->tail)
	{
		listnode_int node = create_listnode();
		copy_listnode(node, cur, level);
		pre->next = node; 
		node->prev = pre; 
		pre = node;   
		cur = cur->next; 
	}
	pre->next = assign_list->tail; 
	assign_list->tail->prev = pre; 
}

void IntPtr::append_list(list_int front_list, list_int back_list)
{
	listnode_int tmp1 = back_list->head->next; 
	listnode_int tmp2 = NULL; 
	
	while(tmp1 != back_list->tail)
	{
		tmp2 = tmp1->next;
		insert_to_rear(front_list, tmp1); 		 
		tmp1 = tmp2; 
	}

	destroy_listnode(back_list->head); 
	destroy_listnode(back_list->tail); 

	free(back_list); 
	return; 
}

void IntPtr::update_list_attributes(list_int list, mpz_t priv_cond, int if_index, int size)
{
	if(size == 0)
		return; 
	mpz_t* op1 = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* op2 = (mpz_t*)malloc(sizeof(mpz_t) * size); 
	mpz_t* result = (mpz_t*)malloc(sizeof(mpz_t) * size); 

	//extract private tags of nodes in the list
	listnode_int tmp = list->head->next;
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

	Mul->doOperation(result, op1, op2, size); 
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

void IntPtr::destroy_listnode(listnode_int node)
{
	free(node); 
}

void IntPtr::destroy_list(list_int list)
{
	listnode_int tmp = list->head->next;
	listnode_int tmp1 = NULL;
	while(tmp != list->tail)
	{
		tmp1 = tmp->next;
		destroy_listnode(tmp);	
		tmp = tmp1; 

	}
	destroy_listnode(list->head);
	destroy_listnode(list->tail); 
	free(list); 
}

priv_int_ptr IntPtr::create_int_ptr(int level)
{
	priv_int_ptr ptr = (priv_int_ptr)malloc(sizeof(struct priv_int_ptr_)); 
	ptr->list = create_list(); 
	mpz_init(ptr->value);
	ptr->size = 0; 
	ptr->tag = 0;
	ptr->level = level;   
	return ptr;  
}

priv_int_ptr* IntPtr::create_int_ptr(int level, int num)
{
	priv_int_ptr* ptrs = (priv_int_ptr*)malloc(sizeof(priv_int_ptr) * num); 
	for(int i = 0; i < num; i++)
		ptrs[i] = create_int_ptr(level); 
	return ptrs; 
}

void IntPtr::destroy_ptr(priv_int_ptr ptr)
{
	destroy_list(ptr->list);
	free(ptr); 
}

void IntPtr::destroy_ptr(priv_int_ptr* ptrs, int num)
{
	for(int i = 0; i < num; i++)
		destroy_ptr(ptrs[i]);
	free(ptrs); 
}

void IntPtr::set_int_ptr(priv_int_ptr ptr, mpz_t* var_loc, priv_int_ptr* ptr_loc)
{
	listnode_int node = create_listnode(); 
	if(var_loc != NULL)
	{
		node->u.var_location = var_loc;
		ptr->level = 1; 
	}
	else
	{
		node->u.ptr_location = *ptr_loc;
		ptr->level = (*ptr_loc)->level; 
	} 
	clear_list(ptr->list);
	node->if_index = -1;
	mpz_set_ui(node->priv_tag, 1); 
	insert_to_rear(ptr->list, node); 
	ptr->size = 1; 
	ptr->tag = 0; // means the pre-computed value is invalid now 
}

void IntPtr::set_int_ptr(priv_int_ptr assign_ptr, priv_int_ptr right_ptr)
{
	clear_list(assign_ptr->list); 
	copy_list(assign_ptr->list, right_ptr->list, right_ptr->level-1);
	assign_ptr->size = right_ptr->size; 
	mpz_set(assign_ptr->value, right_ptr->value); 
	assign_ptr->tag = right_ptr->tag;
	assign_ptr->level = right_ptr->level;  
}

void IntPtr::clear_ptr(priv_int_ptr ptr)
{
	clear_list(ptr->list); 
	ptr->size = 0; 
	mpz_set_ui(ptr->value, 0); 
	ptr->tag = 0;
	ptr->level = 0;  
}

void IntPtr::update_int_ptr(priv_int_ptr ptr, mpz_t* var_loc, priv_int_ptr* ptr_loc, mpz_t private_tag, int index)
{	
	listnode_int node; 
	if(var_loc != NULL)	
		node = create_listnode(var_loc, NULL, private_tag, index); 
	else
		node = create_listnode(NULL, *ptr_loc, private_tag, index); 
	if(ptr->size == 0)
	{	
		insert_to_rear(ptr->list, node); 
		ptr->size = 1; 
		ptr->tag = 0; 
		return;  			
	}
	else
	{
		mpz_t priv_cond;
		mpz_init(priv_cond);  
		ss->modSub(priv_cond, 1, private_tag); 
		update_list_attributes(ptr->list, priv_cond, -1, ptr->size);
		insert_to_rear(ptr->list, node);
		ptr->size++; 
		ptr->tag = 0;   
		mpz_clear(priv_cond); 	
		return; 
	}
}

void IntPtr::update_int_ptr(priv_int_ptr assign_ptr, priv_int_ptr right_ptr, mpz_t private_tag, int index)
{
	if(assign_ptr->size == 0)
	{
		copy_list(assign_ptr->list, right_ptr->list,right_ptr->level-1);
		update_list_attributes(assign_ptr->list, private_tag, index, right_ptr->size);
		assign_ptr->size = right_ptr->size; 
		assign_ptr->tag = 0; 
		return;   
	}
	else
	{
		mpz_t priv_cond; 
		list_int list = create_list(); 
		mpz_init(priv_cond); 
		ss->modSub(priv_cond, 1, private_tag); 
		copy_list(list, right_ptr->list, right_ptr->level-1);
		update_list_attributes(list, private_tag, index, right_ptr->size); 
		update_list_attributes(assign_ptr->list, priv_cond, -1, assign_ptr->size); 
		append_list(assign_ptr->list, list); 
		assign_ptr->size += right_ptr->size; 
		assign_ptr->tag = 0; 
		mpz_clear(priv_cond); 
		return; 		
	}
}

void IntPtr::add_int_ptr(priv_int_ptr ptr, mpz_t* var_loc, priv_int_ptr* ptr_loc, mpz_t private_tag)
{	
	listnode_int node; 
	if(var_loc != NULL)	
		node = create_listnode(var_loc, NULL, private_tag, -1); 
	else
		node = create_listnode(NULL, *ptr_loc, private_tag, -1); 
	insert_to_rear(ptr->list, node); 
	ptr->size++; 
	ptr->tag = 0; 
	return;  			
}

void IntPtr::shrink_int_ptr(priv_int_ptr ptr, int current_index, int parent_index)
{
	//first, find the first node in the list that appears in the matching if body
	listnode_int first = ptr->list->head->next; 
	while(first != ptr->list->tail)
	{
		if(first->if_index >= current_index)
			break; 	
		first = first->next; 
	}
	 // second, remove all the nodes located in parallel -- use parent_index
	listnode_int tmp = ptr->list->head->next; 
        while(tmp != first)
	{
		if(tmp->if_index >= parent_index)
			break; 			
		tmp = tmp->next; 
	}
 	//remove all the nodes from tmp to the node right before first
	listnode_int tmp1 = NULL; 
	int index = 0;
	while(tmp != first)
	{	
		tmp1 = tmp->next; 
		delete_from_list(tmp);
		index++;  
		tmp = tmp1; 
	}
	if(index > 0)
	{
		ptr->size -= index; 
		ptr->tag = 0;
	} 
	return; 
}

void IntPtr::dereference_int_ptr_read(priv_int_ptr ptr, mpz_t var_result, priv_int_ptr ptr_result, int dereferences, mpz_t priv_cond)
{
	if(var_result != NULL)
	{
		dereference_int_ptr_read_var(ptr, var_result, dereferences); 
	}	
	else
	{
		dereference_int_ptr_read_ptr(ptr, ptr_result, dereferences, priv_cond); 
	}
} 

void IntPtr::dereference_int_ptr_read_var(priv_int_ptr ptr, mpz_t result, int dereferences)
{
	priv_int_ptr copy_ptr = create_int_ptr(0);
	priv_int_ptr ptr1 = create_int_ptr(0); 
	priv_int_ptr tmp_ptr; 
	set_int_ptr(copy_ptr, ptr);
	if(dereferences > 1)
	{
		reduce_dereferences(copy_ptr, dereferences, ptr1); 
		tmp_ptr = ptr1; 
	} 
	else 
		tmp_ptr = ptr;
	if(tmp_ptr->tag == 1)
	{
		mpz_set(result, tmp_ptr->value); 	
		return; 
	}
	else
	{
		int size = tmp_ptr->size; 
		int index = 0;
		listnode_int tmp = tmp_ptr->list->head->next; 
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
			mpz_set(op1[index], *(tmp->u.var_location)); 
			mpz_set(op2[index], tmp->priv_tag); 
			mpz_set_ui(product[index], 0); 
			index++; 
			tmp = tmp->next; 
		}

		Mul->doOperation(product, op1, op2, size);
		
		mpz_set_ui(result, 0);  
		for(int i = 0; i < size; i++)
			ss->modAdd(result, result, product[i]);
		tmp_ptr->tag = 1; 
		mpz_set(tmp_ptr->value, result); 
		
		for(int i = 0; i < size; i++)
		{
			mpz_clear(op1[i]);
			mpz_clear(op2[i]);
			mpz_clear(product[i]);
		}
		free(op1); 
		free(op2); 
		free(product); 	
	}
	destroy_ptr(copy_ptr);
	destroy_ptr(ptr1); 
}

int IntPtr::compute_list_size(list_int list)
{
	int size = 0; 
	listnode_int node = list->head->next; 
	while(node != list->tail)
	{
		size++; 
		node = node->next; 
	}
	return size; 
}

void IntPtr::append_int_ptr_list(priv_int_ptr assign_ptr, priv_int_ptr right_ptr)
{
	list_int list = right_ptr->list; 
	listnode_int node = list->head->next; 
	while(node != list->tail)
	{
		append_list(assign_ptr->list, node->u.ptr_location->list); 
		node = node->next; 
	}
	return; 
}

int IntPtr::is_repeated_listnode(list_int list, listnode_int node, int level)
{
	listnode_int tmp = list->head->next;
	while(tmp != list->tail)
	{
		if(level == 0)
		{
			if(tmp->u.var_location == node->u.var_location)
			{
				mpz_add(tmp->priv_tag, tmp->priv_tag, node->priv_tag); 
				return 1;
			} 
		}
		else
		{
			if(tmp->u.ptr_location == node->u.ptr_location)
			{
				mpz_add(tmp->priv_tag, tmp->priv_tag, node->priv_tag); 
				return 1; 
			}
		}
		tmp = tmp->next; 
	}
	return 0; 
	
}

void IntPtr::merge_and_shrink_int_ptr(priv_int_ptr assign_ptr, priv_int_ptr right_ptr)
{
	list_int list = right_ptr->list; 
	listnode_int node = list->head->next;
	int index = 0; 
	while(node != list->tail)
	{
		if(!is_repeated_listnode(assign_ptr->list, node, assign_ptr->level-1))
		{
			listnode_int tmp = create_listnode(); 
			copy_listnode(tmp, node, assign_ptr->level-1); 
			insert_to_rear(assign_ptr->list, tmp);
			assign_ptr->size++; 
		}
		node = node->next;
	}
	return; 
}

void IntPtr::copy_nested_int_ptr(priv_int_ptr assign_ptr, priv_int_ptr right_ptr, mpz_t* R)
{
	int index = 0;
	list_int list = right_ptr->list; 
	listnode_int node = list->head->next;  
	while(node != list->tail)
	{
		list_int child_list = node->u.ptr_location->list;
		listnode_int child_node = child_list->head->next; 
		while(child_node != child_list->tail)
		{
			listnode_int tmp = create_listnode();
			copy_listnode(tmp, child_node, assign_ptr->level-1);  
			mpz_set(tmp->priv_tag, R[index]); 
			insert_to_rear(assign_ptr->list, tmp);
			assign_ptr->size++;  
			index++; 
			child_node = child_node->next; 
			
		}
		node = node->next; 
	}
}

void IntPtr::reduce_dereferences(priv_int_ptr ptr, int dereferences, priv_int_ptr result_ptr)
{
	priv_int_ptr runner = create_int_ptr(0);  
	set_int_ptr(runner, ptr);
	while(dereferences > 1)
	{
		read_write_helper(runner, result_ptr, NULL);
		set_int_ptr(runner, result_ptr);
		dereferences--;
	}
	set_int_ptr(result_ptr, runner); 
	destroy_ptr(runner);
	return; 
}

void IntPtr::dereference_int_ptr_read_ptr(priv_int_ptr ptr, priv_int_ptr result, int dereferences, mpz_t priv_cond)
{
	priv_int_ptr copy_ptr = create_int_ptr(ptr->level);
	priv_int_ptr ptr1 = create_int_ptr(ptr->level); 
	set_int_ptr(ptr1, ptr); 
	priv_int_ptr tmp_ptr; 
	if(dereferences > 1)
	{
		reduce_dereferences(ptr1, dereferences, copy_ptr); 
		tmp_ptr = copy_ptr; 
	} 
	else 
		tmp_ptr = ptr;
	read_write_helper(tmp_ptr, result, priv_cond); 
	destroy_ptr(copy_ptr); 
	destroy_ptr(ptr1); 
}

void IntPtr::read_write_helper(priv_int_ptr ptr, priv_int_ptr result, mpz_t priv_cond)
{
	// compute the number of elements in the second layer of list of ptr 
	int num = 0; 
	list_int list = ptr->list; 
	listnode_int node = list->head->next; 
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
		list_int child_list = node->u.ptr_location->list; 
		listnode_int child_node = child_list->head->next; 
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
 		
	Mul->doOperation(R, op1, op2, num); 
	if(priv_cond != NULL)
	{	
		mpz_t cond;
		mpz_init(cond);  
		ss->modSub(cond, 1, priv_cond); 
		update_list_attributes(result->list, cond, -1, result->size);
		Mul->doOperation(R, R, op3, num); 
	}
	else
	{
		clear_list(result->list);
		result->size = 0; 
	}

	result->level = ptr->level-1; 
	priv_int_ptr tmp_ptr = create_int_ptr(ptr->level-1); 
	copy_nested_int_ptr(tmp_ptr, ptr, R);
	merge_and_shrink_int_ptr(result, tmp_ptr);
	destroy_ptr(tmp_ptr);

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

int IntPtr::list_size(list_int list)
{
	int index = 0; 
	listnode_int node = list->head->next, node1;
	while(node != list->tail)
	{
		index++;
		node = node->next; 
	}
	return index; 
}
void IntPtr::dereference_int_ptr_write(priv_int_ptr ptr, mpz_t var_value, priv_int_ptr ptr_value, int dereferences, mpz_t priv_cond)
{
	if(var_value != NULL)
		dereference_int_ptr_write_var(ptr, var_value, dereferences, priv_cond); 	
	else
		dereference_int_ptr_write_ptr(ptr, ptr_value, dereferences, priv_cond); 
}

void IntPtr::dereference_int_ptr_write(priv_int_ptr ptr, mpz_t* var_value, priv_int_ptr* ptr_value, int dereferences, mpz_t priv_cond)
{
	priv_int_ptr tmp_ptr = create_int_ptr(0); 
	set_int_ptr(tmp_ptr, var_value, ptr_value);
	dereference_int_ptr_write(ptr, NULL, tmp_ptr, dereferences, priv_cond); 
	destroy_ptr(tmp_ptr); 
}
void IntPtr::dereference_int_ptr_write_ptr(priv_int_ptr ptr, priv_int_ptr value, int dereferences, mpz_t priv_cond)
{
	priv_int_ptr copy_ptr = create_int_ptr(0);
	priv_int_ptr ptr1 = create_int_ptr(0); 
	set_int_ptr(copy_ptr, ptr); 
	priv_int_ptr tmp_ptr; 

	if(dereferences > 1)
	{
		reduce_dereferences(copy_ptr, dereferences, ptr1);  
		tmp_ptr = ptr1; 
	}
	else 
		tmp_ptr = ptr;
	list_int list = tmp_ptr->list; 
	listnode_int node = list->head->next; 
	  	
	while(node != list->tail)
	{
		priv_int_ptr ptr2 = create_int_ptr(0); 
		priv_int_ptr assign_ptr = node->u.ptr_location; 
		set_int_ptr(ptr2, value);
		mpz_t cond; 
		mpz_init(cond); 
		if(priv_cond == NULL)
		{
			ss->modSub(cond, 1, node->priv_tag);
			update_list_attributes(assign_ptr->list, cond, -1, assign_ptr->size);  
			update_list_attributes(ptr2->list, node->priv_tag, -1, ptr2->size);
		}
		else
		{	
			mpz_t* op1 = (mpz_t*)malloc(sizeof(mpz_t)); 	
			mpz_t* op2 = (mpz_t*)malloc(sizeof(mpz_t)); 
			mpz_t* R = (mpz_t*)malloc(sizeof(mpz_t)); 
			mpz_init_set(op1[0], priv_cond); 
			mpz_init_set(op2[0], node->priv_tag);
			mpz_init(R[0]); 
			Mul->doOperation(R, op1, op2, 1);
			ss->modSub(cond, 1, R[0]); 
			update_list_attributes(assign_ptr->list, cond, -1, assign_ptr->size); 
			update_list_attributes(ptr2->list, R[0], -1, ptr2->size); 
			
			mpz_clear(op1[0]); 
			mpz_clear(op2[0]); 
			mpz_clear(R[0]); 

			free(op1); 
			free(op2); 
			free(R);  
		}
		merge_and_shrink_int_ptr(assign_ptr, ptr2); 		
		destroy_ptr(ptr2); 
		node = node->next; 
	}	
	destroy_ptr(copy_ptr); 
	destroy_ptr(ptr1); 
	return; 
}

void IntPtr::dereference_int_ptr_write_var(priv_int_ptr ptr, mpz_t value, int dereferences, mpz_t priv_cond)
{
	priv_int_ptr tmp_ptr;
	priv_int_ptr copy_ptr = create_int_ptr(0); 
	priv_int_ptr ptr1 = create_int_ptr(0); 
	set_int_ptr(ptr1, ptr);  
	
	if(dereferences > 1)
	{	
		reduce_dereferences(ptr1, dereferences, copy_ptr);
		tmp_ptr = copy_ptr;   
	}
	else 
		tmp_ptr = ptr;
	
	int index = 0; 
	int size = tmp_ptr->size; 
	listnode_int tmp = tmp_ptr->list->head->next; 
	
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
	 	mpz_set(op1[index], *(tmp->u.var_location)); 
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
		Mul->doOperation(op4, op4, op3, size); 
	Mul->doOperation(op4, op4, op2, size); 
	ss->modAdd(op4, op1, op4, size); 
	/* update the values of nodes in the list (not update the tags) */
	tmp = tmp_ptr->list->head->next; 
	index = 0;
	while(tmp != tmp_ptr->list->tail)
	{
		mpz_set(*(tmp->u.var_location), op4[index++]); 
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

	destroy_ptr(ptr1); 
	destroy_ptr(copy_ptr); 
	return; 
}


