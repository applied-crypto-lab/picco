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
#ifndef FLOAT_PTR_H_
#define FLOAT_PTR_H_

#include "Operation.h"
#include "Mult.h"

typedef struct listnode_float_ *listnode_float; 
typedef struct list_float_ *list_float; 
typedef struct priv_float_ptr_ *priv_float_ptr; 

struct listnode_float_
{
	union {
		mpz_t** var_location;
		priv_float_ptr ptr_location;
	}u;
	mpz_t	priv_tag;
	int if_index;
	struct listnode_float_ *prev, *next;

}; 
	
struct list_float_
{
	listnode_float head; 
	listnode_float tail; 
}; 

struct priv_float_ptr_
{
	list_float list; 
	int size; 
	mpz_t* value; 
	int tag;
    int level; 
}; 

class FloatPtr : public Operation
{
	public:
	FloatPtr(NodeNetwork nodeNet, int nodeID, SecretShare *s); 
	virtual ~FloatPtr();
		/******************************/
        listnode_float create_listnode(mpz_t**, priv_float_ptr, mpz_t, int);
	listnode_float create_listnode();
        void copy_listnode(listnode_float, listnode_float, int);
	list_float create_list();
	void insert_to_rear(list_float, listnode_float);
        void clear_list(list_float);
        void copy_list(list_float, list_float, int);
        void append_list(list_float, list_float);
        void delete_from_list(listnode_float); 
        void update_list_attributes(list_float, mpz_t, int, int);
        void destroy_listnode(listnode_float);
        void destroy_list(list_float);

		/******************************/ 
        priv_float_ptr create_float_ptr(int);
        priv_float_ptr* create_float_ptr(int, int);
        void destroy_ptr(priv_float_ptr);
        void destroy_ptr(priv_float_ptr*, int);
   	void clear_ptr(priv_float_ptr);  
        void set_float_ptr(priv_float_ptr, mpz_t**, priv_float_ptr*);
        void set_float_ptr(priv_float_ptr, priv_float_ptr);
    
        void update_float_ptr(priv_float_ptr, mpz_t**, priv_float_ptr*, mpz_t, int);
        void update_float_ptr(priv_float_ptr, priv_float_ptr, mpz_t, int);
        void add_float_ptr(priv_float_ptr, mpz_t**, priv_float_ptr*, mpz_t);
	
	void shrink_float_ptr(priv_float_ptr, int, int);

        void dereference_float_ptr_read(priv_float_ptr, mpz_t*, priv_float_ptr, int, mpz_t);
        void dereference_float_ptr_read_var(priv_float_ptr, mpz_t*, int);
        int compute_list_size(list_float);
        void append_float_ptr_list(priv_float_ptr, priv_float_ptr);
        int is_repeated_listnode(list_float, listnode_float, int);
        void merge_and_shrink_float_ptr(priv_float_ptr, priv_float_ptr);
        void copy_nested_float_ptr(priv_float_ptr, priv_float_ptr, mpz_t*);
        void reduce_dereferences(priv_float_ptr, int, priv_float_ptr);
        void dereference_float_ptr_read_ptr(priv_float_ptr, priv_float_ptr, int, mpz_t);
        void read_write_helper(priv_float_ptr, priv_float_ptr, mpz_t);
        int list_size(list_float);
        void dereference_float_ptr_write(priv_float_ptr, mpz_t*, priv_float_ptr, int, mpz_t);
        void dereference_float_ptr_write(priv_float_ptr, mpz_t**, priv_float_ptr*, int, mpz_t);
        void dereference_float_ptr_write_ptr(priv_float_ptr, priv_float_ptr, int, mpz_t);
        void dereference_float_ptr_write_var(priv_float_ptr, mpz_t*, int, mpz_t);

    private:
        Mult *Mul;
};

#endif /* FLOAT_PTR_H_*/
