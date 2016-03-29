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
#ifndef INT_PTR_H_
#define INT_PTR_H_

#include "Operation.h"
#include "Mult.h"
typedef struct listnode_int_ *listnode_int; 
typedef struct list_int_ *list_int; 
typedef struct priv_int_ptr_ *priv_int_ptr; 

struct listnode_int_
{
	union {
		mpz_t* var_location; 
		priv_int_ptr ptr_location;
	}u;  
	mpz_t	priv_tag; 
	int if_index; 
	struct listnode_int_ *prev, *next; 
}; 
	
struct list_int_
{
	listnode_int head; 
	listnode_int tail; 
}; 

struct priv_int_ptr_
{
	list_int list; 
	int size; 
	mpz_t value; // when level >= 1, value is assigned NULL 
	int tag;
	int level;  			
}; 

class IntPtr : public Operation
{
	public:
		IntPtr(NodeNetwork nodeNet, int nodeID, SecretShare *s); 
		virtual ~IntPtr();
		/******************************/ 
		listnode_int create_listnode(mpz_t*, priv_int_ptr, mpz_t, int);
		listnode_int create_listnode();  
		void copy_listnode(listnode_int, listnode_int, int);
		int is_repeated_listnode(list_int, listnode_int, int);  
		list_int create_list(); 
		int list_size(list_int list); 
		void insert_to_rear(list_int, listnode_int); 
		void delete_from_list(listnode_int); 
		void clear_list(list_int);
		void copy_list(list_int, list_int, int); 
		int  compute_list_size(list_int); 
		void append_list(list_int, list_int);  
		void update_list_attributes(list_int, mpz_t, int, int); 
		void destroy_listnode(listnode_int); 
		void destroy_list(list_int); 

		/******************************/ 
		priv_int_ptr create_int_ptr(int); 
		priv_int_ptr* create_int_ptr(int, int); 
		void append_int_ptr_list(priv_int_ptr, priv_int_ptr); 
		void clear_ptr(priv_int_ptr); 
		void destroy_ptr(priv_int_ptr); 
		void destroy_ptr(priv_int_ptr*, int); 		
		void copy_nested_int_ptr(priv_int_ptr, priv_int_ptr, mpz_t*); 
		void set_int_ptr(priv_int_ptr, mpz_t*, priv_int_ptr*); 
		void set_int_ptr(priv_int_ptr, priv_int_ptr); 
		void update_int_ptr(priv_int_ptr, mpz_t*, priv_int_ptr*, mpz_t, int); 
		void update_int_ptr(priv_int_ptr, priv_int_ptr, mpz_t, int);
		void add_int_ptr(priv_int_ptr, mpz_t*, priv_int_ptr*, mpz_t); 
		void shrink_int_ptr(priv_int_ptr, int, int);
		void merge_and_shrink_int_ptr(priv_int_ptr, priv_int_ptr); 
	  
		void reduce_dereferences(priv_int_ptr, int, priv_int_ptr); 
		void read_write_helper(priv_int_ptr, priv_int_ptr, mpz_t); 
		void dereference_int_ptr_read(priv_int_ptr, mpz_t, priv_int_ptr, int, mpz_t); 
		void dereference_int_ptr_read_var(priv_int_ptr, mpz_t, int); 
		void dereference_int_ptr_read_ptr(priv_int_ptr, priv_int_ptr, int, mpz_t);
 
		void dereference_int_ptr_write(priv_int_ptr, mpz_t, priv_int_ptr, int, mpz_t); 
		void dereference_int_ptr_write(priv_int_ptr, mpz_t*, priv_int_ptr*, int, mpz_t); 
		void dereference_int_ptr_write_var(priv_int_ptr, mpz_t, int, mpz_t); 
		void dereference_int_ptr_write_ptr(priv_int_ptr, priv_int_ptr, int, mpz_t); 
			 	
	private: 
		Mult *Mul; 
}; 

#endif /* INT_PTR_H_*/
