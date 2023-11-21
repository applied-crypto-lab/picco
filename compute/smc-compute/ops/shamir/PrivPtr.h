/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2013 PICCO Team
   ** Department of Computer Science and Engineering, University of Notre Dame
   ** Department of Computer Science and Engineering, University at Buffalo (SUNY)

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
#ifndef PRIV_PTR_SHAMIR_H_
#define PRIV_PTR_SHAMIR_H_

#include "Mult.h"
#include "Operation.h"

typedef struct listnode_ *listnode;
typedef struct list_ *dlist;
typedef struct priv_ptr_ *priv_ptr;

struct listnode_ {
    union {
        void *struct_var_location;
        mpz_t *int_var_location;
        mpz_t **float_var_location;
        priv_ptr ptr_location;
    } u;
    mpz_t priv_tag;
    int if_index;
    struct listnode_ *prev, *next;
};

struct list_ {
    listnode head;
    listnode tail;
};

struct priv_ptr_ {
    dlist list;
    int size;
    int level;
    int type; // type = 0 - int, = 1 - float, = 2 - struct
};

// class PrivPtr : public Operation {
// public:
// PrivPtr(NodeNetwork nodeNet, int nodeID, SecretShare *s);
// virtual ~PrivPtr();
/******************************/
listnode create_listnode(mpz_t *, mpz_t **, void *, priv_ptr, mpz_t, int);
listnode create_listnode();
void copy_listnode(listnode, listnode, int, int);
dlist create_list();
int list_size(dlist);
void insert_to_rear(dlist, listnode);
void delete_from_list(listnode *);
void clear_list(dlist *);
void copy_list(dlist, dlist, int, int);
int compute_list_size(dlist);
void append_list(dlist, dlist);
void destroy_listnode(listnode *);
void destroy_list(dlist *);
/******************************/
priv_ptr create_ptr(int, int);
priv_ptr *create_ptr(int, int, int);
void append_ptr_list(priv_ptr, priv_ptr);
void clear_ptr(priv_ptr *);
void destroy_ptr(priv_ptr *);
void destroy_ptr(priv_ptr **, int);
void copy_nested_ptr(priv_ptr, priv_ptr, mpz_t *);
void set_ptr(priv_ptr, mpz_t *, mpz_t **, void *, priv_ptr *, int threadID);
void set_ptr(priv_ptr, priv_ptr, int threadID);
void add_ptr(priv_ptr, mpz_t *, mpz_t **, void *, priv_ptr *, mpz_t, int threadID);
void shrink_ptr(priv_ptr, int, int, int threadID);
void merge_and_shrink_ptr(priv_ptr, priv_ptr, NodeNetwork net, int id, SecretShare *ss);
void reduce_dereferences(priv_ptr, int, priv_ptr, int threadID, NodeNetwork net, int id, SecretShare *ss);
void dereference_ptr_read_ptr(priv_ptr, priv_ptr, int, mpz_t, int threadID);
void dereference_ptr_read_ptr(priv_ptr ptr, priv_ptr result, int dereferences, mpz_t priv_cond, int threadID, NodeNetwork net, int id, SecretShare *ss);

void dereference_ptr_write(priv_ptr, mpz_t *, mpz_t **, void *, priv_ptr *, int, mpz_t, int threadID, NodeNetwork net, int id, SecretShare *ss);
void update_list_attributes(dlist list, mpz_t priv_cond, int if_index, int size, int threadID, NodeNetwork net, int id, SecretShare *ss);
void update_ptr(priv_ptr ptr, mpz_t *int_var_loc, mpz_t **float_var_loc, void *struct_var_loc, priv_ptr *ptr_loc, mpz_t private_tag, int index, int threadID, NodeNetwork net, int id, SecretShare *ss);
void update_ptr(priv_ptr assign_ptr, priv_ptr right_ptr, mpz_t private_tag, int index, int threadID, NodeNetwork net, int id, SecretShare *ss);
int is_repeated_listnode(dlist list, listnode node, int level, int type, NodeNetwork net, int id, SecretShare *ss);
void dereference_ptr_read_var(priv_ptr ptr, mpz_t result, int dereferences, int threadID, NodeNetwork net, int id, SecretShare *ss);
void dereference_ptr_read_var(priv_ptr ptr, mpz_t *result, int dereferences, int threadID, NodeNetwork net, int id, SecretShare *ss);
void read_write_helper(priv_ptr ptr, priv_ptr result, mpz_t priv_cond, int threadID, NodeNetwork net, int id, SecretShare *ss);
void dereference_ptr_write_ptr(priv_ptr ptr, priv_ptr value, int dereferences, mpz_t priv_cond, int threadID, NodeNetwork net, int id, SecretShare *ss);
void dereference_ptr_write_var(priv_ptr ptr, mpz_t *value, int dereferences, mpz_t priv_cond, int threadID, NodeNetwork net, int id, SecretShare *ss);
void dereference_ptr_write_var(priv_ptr ptr, mpz_t value, int dereferences, mpz_t priv_cond, int threadID, NodeNetwork net, int id, SecretShare *ss);

// };

#endif
