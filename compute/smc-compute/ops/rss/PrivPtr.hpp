/*
   PICCO: A General Purpose Compiler for Private Distributed Computation
   ** Copyright (C) from 2024 PICCO Team
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

#ifndef _PRIVPTR_HPP_
#define _PRIVPTR_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"
#include "Mult.hpp"

// ============================================================================
// RSS Private Pointer Implementation
// Ported from Shamir PrivPtr.cpp to template-based RSS format.
// Interface matches Shamir so SMC_Utils wrappers are shared.
//
// Non-crypto functions (create, set, add, clear, etc.) use a module-level
// ss pointer set once via init_privptr(). Crypto functions (update, deref)
// take net/ss as parameters, same as Shamir.
// ============================================================================

// Forward declarations
template <typename T> struct rss_listnode;
template <typename T> struct rss_dlist;
template <typename T> struct rss_priv_ptr;

// ============================================================================
// Module-level ss pointer — set once via init_privptr() during SMC_Utils init
// ============================================================================
template <typename T>
inline replicatedSecretShare<T> *_privptr_ss = nullptr;

template <typename T>
void init_privptr(replicatedSecretShare<T> *ss) {
    _privptr_ss<T> = ss;
}

// ============================================================================
// Struct Definitions (heap-allocated, pointer-based like Shamir)
// ============================================================================

template <typename T>
struct rss_listnode {
    union {
        void *struct_var_location;
        T **int_var_location;
        T ***float_var_location;
        rss_priv_ptr<T> *ptr_location;
    } u;
    T *priv_tag;
    int if_index;
    rss_listnode<T> *prev, *next;
};

template <typename T>
struct rss_dlist {
    rss_listnode<T> *head;
    rss_listnode<T> *tail;
};

template <typename T>
struct rss_priv_ptr {
    rss_dlist<T> *list;
    int size;
    int level;
    int type; // 0 = int, 1 = float, 2 = struct
};

// ============================================================================
// Concrete typedefs for PICCO-generated code
// ============================================================================
typedef rss_priv_ptr<priv_int_t> *priv_ptr;
typedef priv_ptr priv_int_ptr;
typedef priv_ptr priv_float_ptr;
typedef rss_listnode<priv_int_t> *listnode;

// ============================================================================
// Helper: copy numShares elements from src to dst
// ============================================================================
template <typename T>
static inline void tag_copy(T *dst, T *src, uint numShares) {
    memcpy(dst, src, sizeof(T) * numShares);
}

// ============================================================================
// Listnode Operations
// ============================================================================

template <typename T>
rss_listnode<T> *create_listnode() {
    auto *ss = _privptr_ss<T>;
    uint numShares = ss->getNumShares();
    rss_listnode<T> *node = new rss_listnode<T>();
    node->priv_tag = new T[numShares]();
    node->u.int_var_location = nullptr;
    node->if_index = -1;
    node->prev = node->next = nullptr;
    return node;
}

template <typename T>
rss_listnode<T> *create_listnode(T **int_var_loc, T ***float_var_loc, void *struct_var_loc,
                                  rss_priv_ptr<T> *ptr_loc, T *private_tag, int index) {
    auto *ss = _privptr_ss<T>;
    uint numShares = ss->getNumShares();
    rss_listnode<T> *node = create_listnode<T>();
    if (int_var_loc != nullptr)
        node->u.int_var_location = int_var_loc;
    else if (float_var_loc != nullptr)
        node->u.float_var_location = float_var_loc;
    else if (struct_var_loc != nullptr)
        node->u.struct_var_location = struct_var_loc;
    else
        node->u.ptr_location = ptr_loc;
    tag_copy(node->priv_tag, private_tag, numShares);
    node->if_index = index;
    node->prev = node->next = nullptr;
    return node;
}

template <typename T>
void copy_listnode(rss_listnode<T> *assign_node, rss_listnode<T> *right_node,
                   int level, int type) {
    auto *ss = _privptr_ss<T>;
    uint numShares = ss->getNumShares();
    if (level == 0) {
        if (type == 0)
            assign_node->u.int_var_location = right_node->u.int_var_location;
        else if (type == 1)
            assign_node->u.float_var_location = right_node->u.float_var_location;
        else if (type == 2)
            assign_node->u.struct_var_location = right_node->u.struct_var_location;
    } else
        assign_node->u.ptr_location = right_node->u.ptr_location;
    tag_copy(assign_node->priv_tag, right_node->priv_tag, numShares);
    assign_node->if_index = right_node->if_index;
}

template <typename T>
void destroy_listnode(rss_listnode<T> **node) {
    if (*node != nullptr) {
        delete[](*node)->priv_tag;
        delete *node;
        *node = nullptr;
    }
}

// ============================================================================
// List Operations
// ============================================================================

template <typename T>
rss_dlist<T> *create_list() {
    rss_dlist<T> *list = new rss_dlist<T>();
    list->head = create_listnode<T>();
    list->tail = create_listnode<T>();
    list->head->next = list->tail;
    list->tail->prev = list->head;
    return list;
}

template <typename T>
void insert_to_rear(rss_dlist<T> *list, rss_listnode<T> *node) {
    rss_listnode<T> *tmp = list->tail->prev;
    list->tail->prev = node;
    node->next = list->tail;
    node->prev = tmp;
    tmp->next = node;
}

template <typename T>
void delete_from_list(rss_listnode<T> **node) {
    (*node)->prev->next = (*node)->next;
    (*node)->next->prev = (*node)->prev;
    destroy_listnode(node);
}

template <typename T>
void clear_list(rss_dlist<T> *list) {
    rss_listnode<T> *tmp = list->head->next;
    rss_listnode<T> *tmp1;
    while (tmp != list->tail) {
        tmp1 = tmp->next;
        delete_from_list(&tmp);
        tmp = tmp1;
    }
}

template <typename T>
void copy_list(rss_dlist<T> *assign_list, rss_dlist<T> *right_list,
               int level, int type) {
    rss_listnode<T> *pre = assign_list->head;
    rss_listnode<T> *cur = right_list->head->next;
    while (cur != right_list->tail) {
        rss_listnode<T> *node = create_listnode<T>();
        copy_listnode(node, cur, level, type);
        pre->next = node;
        node->prev = pre;
        pre = node;
        cur = cur->next;
    }
    pre->next = assign_list->tail;
    assign_list->tail->prev = pre;
}

template <typename T>
int compute_list_size(rss_dlist<T> *list) {
    int size = 0;
    rss_listnode<T> *node = list->head->next;
    while (node != list->tail) {
        size++;
        node = node->next;
    }
    return size;
}

template <typename T>
int list_size(rss_dlist<T> *list) {
    return compute_list_size(list);
}

template <typename T>
void append_list(rss_dlist<T> *front_list, rss_dlist<T> *back_list) {
    rss_listnode<T> *tmp1 = back_list->head->next;
    rss_listnode<T> *tmp2 = nullptr;
    while (tmp1 != back_list->tail) {
        tmp2 = tmp1->next;
        insert_to_rear(front_list, tmp1);
        tmp1 = tmp2;
    }
    destroy_listnode(&(back_list->head));
    destroy_listnode(&(back_list->tail));
    delete back_list;
}

template <typename T>
void destroy_list(rss_dlist<T> **list) {
    rss_listnode<T> *tmp = (*list)->head->next;
    rss_listnode<T> *tmp1 = nullptr;
    while (tmp != (*list)->tail) {
        tmp1 = tmp->next;
        destroy_listnode(&tmp);
        tmp = tmp1;
    }
    destroy_listnode(&((*list)->head));
    destroy_listnode(&((*list)->tail));
    delete *list;
    *list = nullptr;
}

// ============================================================================
// Pointer Management (no ss parameter — matches Shamir interface)
// ============================================================================

template <typename T>
rss_priv_ptr<T> *create_ptr(int level, int type) {
    rss_priv_ptr<T> *ptr = new rss_priv_ptr<T>();
    ptr->list = create_list<T>();
    ptr->size = 0;
    ptr->level = level;
    ptr->type = type;
    return ptr;
}

template <typename T>
rss_priv_ptr<T> **create_ptr(int level, int type, int num) {
    rss_priv_ptr<T> **ptrs = new rss_priv_ptr<T> *[num];
    for (int i = 0; i < num; i++)
        ptrs[i] = create_ptr<T>(level, type);
    return ptrs;
}

template <typename T>
void destroy_ptr(rss_priv_ptr<T> **ptr) {
    destroy_list(&((*ptr)->list));
    delete *ptr;
    *ptr = nullptr;
}

template <typename T>
void destroy_ptr(rss_priv_ptr<T> ***ptrs, int num) {
    for (int i = 0; i < num; i++)
        destroy_ptr(&((*ptrs)[i]));
    delete[] * ptrs;
    *ptrs = nullptr;
}

template <typename T>
void clear_ptr(rss_priv_ptr<T> **ptr) {
    clear_list((*ptr)->list);
    (*ptr)->size = 0;
    (*ptr)->level = 0;
}

template <typename T>
void set_ptr(rss_priv_ptr<T> *ptr, T **int_var_loc, T ***float_var_loc,
             void *struct_var_loc, rss_priv_ptr<T> **ptr_loc, int threadID) {
    auto *ss = _privptr_ss<T>;
    uint numShares = ss->getNumShares();
    rss_listnode<T> *node = create_listnode<T>();
    if (int_var_loc != nullptr) {
        node->u.int_var_location = int_var_loc;
        ptr->level = 1;
    } else if (float_var_loc != nullptr) {
        node->u.float_var_location = float_var_loc;
        ptr->level = 1;
    } else if (struct_var_loc != nullptr) {
        node->u.struct_var_location = struct_var_loc;
        ptr->level = 1;
    } else {
        node->u.ptr_location = *ptr_loc;
        ptr->level = (*ptr_loc)->level;
    }
    clear_list(ptr->list);
    node->if_index = -1;
    memset(node->priv_tag, 0, sizeof(T) * numShares);
    ss->sparsify_public(node->priv_tag, 1);
    insert_to_rear(ptr->list, node);
    ptr->size = 1;
}

template <typename T>
void set_ptr(rss_priv_ptr<T> *assign_ptr, rss_priv_ptr<T> *right_ptr, int threadID) {
    clear_list(assign_ptr->list);
    copy_list(assign_ptr->list, right_ptr->list, right_ptr->level - 1, assign_ptr->type);
    assign_ptr->size = right_ptr->size;
    assign_ptr->level = right_ptr->level;
}

template <typename T>
void add_ptr(rss_priv_ptr<T> *ptr, T **int_var_loc, T ***float_var_loc,
             void *struct_var_loc, rss_priv_ptr<T> **ptr_loc, T *private_tag,
             int threadID) {
    rss_listnode<T> *node;
    if (ptr_loc != nullptr)
        node = create_listnode(int_var_loc, float_var_loc, struct_var_loc, *ptr_loc, private_tag, -1);
    else
        node = create_listnode(int_var_loc, float_var_loc, struct_var_loc, (rss_priv_ptr<T> *)nullptr, private_tag, -1);
    insert_to_rear(ptr->list, node);
    ptr->size++;
}

template <typename T>
void shrink_ptr(rss_priv_ptr<T> *ptr, int current_index, int parent_index, int threadID) {
    rss_listnode<T> *first = ptr->list->head->next;
    while (first != ptr->list->tail) {
        if (first->if_index >= current_index)
            break;
        first = first->next;
    }
    rss_listnode<T> *tmp = ptr->list->head->next;
    while (tmp != first) {
        if (tmp->if_index >= parent_index)
            break;
        tmp = tmp->next;
    }
    rss_listnode<T> *tmp1 = nullptr;
    int index = 0;
    while (tmp != first) {
        tmp1 = tmp->next;
        delete_from_list(&tmp);
        index++;
        tmp = tmp1;
    }
    if (index > 0)
        ptr->size -= index;
}

template <typename T>
void append_ptr_list(rss_priv_ptr<T> *assign_ptr, rss_priv_ptr<T> *right_ptr) {
    rss_dlist<T> *list = right_ptr->list;
    rss_listnode<T> *node = list->head->next;
    while (node != list->tail) {
        append_list(assign_ptr->list, node->u.ptr_location->list);
        node = node->next;
    }
}

// ============================================================================
// Crypto helpers (keep net, ss parameters — same as Shamir)
// ============================================================================

template <typename T>
int is_repeated_listnode(rss_dlist<T> *list, rss_listnode<T> *node, int level, int type,
                         NodeNetwork net, replicatedSecretShare<T> *ss) {
    rss_listnode<T> *tmp = list->head->next;
    while (tmp != list->tail) {
        if (level == 0) {
            if ((type == 0 && tmp->u.int_var_location == node->u.int_var_location) ||
                (type == 1 && tmp->u.float_var_location == node->u.float_var_location) ||
                (type == 2 && tmp->u.struct_var_location == node->u.struct_var_location)) {
                ss->modAdd(tmp->priv_tag, tmp->priv_tag, node->priv_tag);
                return 1;
            }
        } else {
            if (tmp->u.ptr_location == node->u.ptr_location) {
                ss->modAdd(tmp->priv_tag, tmp->priv_tag, node->priv_tag);
                return 1;
            }
        }
        tmp = tmp->next;
    }
    return 0;
}

template <typename T>
void merge_and_shrink_ptr(rss_priv_ptr<T> *assign_ptr, rss_priv_ptr<T> *right_ptr,
                          NodeNetwork net, replicatedSecretShare<T> *ss) {
    rss_dlist<T> *list = right_ptr->list;
    rss_listnode<T> *node = list->head->next;
    while (node != list->tail) {
        if (!is_repeated_listnode(assign_ptr->list, node, assign_ptr->level - 1, assign_ptr->type, net, ss)) {
            rss_listnode<T> *tmp = create_listnode<T>();
            copy_listnode(tmp, node, assign_ptr->level - 1, assign_ptr->type);
            insert_to_rear(assign_ptr->list, tmp);
            assign_ptr->size++;
        }
        node = node->next;
    }
}

template <typename T>
void copy_nested_ptr(rss_priv_ptr<T> *assign_ptr, rss_priv_ptr<T> *right_ptr, T **R) {
    auto *ss = _privptr_ss<T>;
    uint numShares = ss->getNumShares();
    int index = 0;
    rss_dlist<T> *list = right_ptr->list;
    rss_listnode<T> *node = list->head->next;
    while (node != list->tail) {
        rss_dlist<T> *child_list = node->u.ptr_location->list;
        rss_listnode<T> *child_node = child_list->head->next;
        while (child_node != child_list->tail) {
            rss_listnode<T> *tmp = create_listnode<T>();
            copy_listnode(tmp, child_node, assign_ptr->level - 1, assign_ptr->type);
            tag_copy(tmp->priv_tag, R[index], numShares);
            insert_to_rear(assign_ptr->list, tmp);
            assign_ptr->size++;
            index++;
            child_node = child_node->next;
        }
        node = node->next;
    }
}

// ============================================================================
// Crypto Operations (require Mult protocol — keep net, ss like Shamir)
// ============================================================================

template <typename T>
void update_list_attributes(rss_dlist<T> *list, T *priv_cond, int if_index, int size,
                            int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    if (size == 0)
        return;
    uint numShares = ss->getNumShares();

    T **op1 = new T *[size];
    T **op2 = new T *[size];
    T **result = new T *[size];

    rss_listnode<T> *tmp = list->head->next;
    int index = 0;
    while (tmp != list->tail) {
        op1[index] = new T[numShares];
        tag_copy(op1[index], tmp->priv_tag, numShares);
        index++;
        tmp = tmp->next;
    }
    for (int i = 0; i < size; i++) {
        op2[i] = new T[numShares];
        tag_copy(op2[i], priv_cond, numShares);
        result[i] = new T[numShares]();
    }

    Mult(result, op1, op2, size, threadID, net, ss);

    tmp = list->head->next;
    index = 0;
    while (tmp != list->tail) {
        tag_copy(tmp->priv_tag, result[index], numShares);
        if (if_index != -1)
            tmp->if_index = if_index;
        index++;
        tmp = tmp->next;
    }

    for (int i = 0; i < size; i++) {
        delete[] op1[i];
        delete[] op2[i];
        delete[] result[i];
    }
    delete[] op1;
    delete[] op2;
    delete[] result;
}

template <typename T>
void update_ptr(rss_priv_ptr<T> *ptr, T **int_var_loc, T ***float_var_loc,
                void *struct_var_loc, rss_priv_ptr<T> **ptr_loc, T *private_tag,
                int index, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    rss_listnode<T> *node;
    if (ptr_loc != nullptr)
        node = create_listnode(int_var_loc, float_var_loc, struct_var_loc, *ptr_loc, private_tag, index);
    else
        node = create_listnode(int_var_loc, float_var_loc, struct_var_loc, (rss_priv_ptr<T> *)nullptr, private_tag, index);

    if (ptr->size == 0) {
        insert_to_rear(ptr->list, node);
        ptr->size = 1;
        return;
    } else {
        T *priv_cond = new T[numShares];
        ss->modSub(priv_cond, (T)1, private_tag);
        update_list_attributes(ptr->list, priv_cond, -1, ptr->size, threadID, net, ss);
        insert_to_rear(ptr->list, node);
        ptr->size++;
        delete[] priv_cond;
        return;
    }
}

template <typename T>
void update_ptr(rss_priv_ptr<T> *assign_ptr, rss_priv_ptr<T> *right_ptr, T *private_tag,
                int index, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    if (assign_ptr->size == 0) {
        copy_list(assign_ptr->list, right_ptr->list, right_ptr->level - 1, assign_ptr->type);
        update_list_attributes(assign_ptr->list, private_tag, index, right_ptr->size, threadID, net, ss);
        assign_ptr->size = right_ptr->size;
        return;
    } else {
        rss_priv_ptr<T> *ptr = create_ptr<T>(right_ptr->level, right_ptr->type);
        ptr->size = right_ptr->size;
        T *priv_cond = new T[numShares];
        ss->modSub(priv_cond, (T)1, private_tag);
        copy_list(ptr->list, right_ptr->list, right_ptr->level - 1, assign_ptr->type);
        update_list_attributes(ptr->list, private_tag, index, ptr->size, threadID, net, ss);
        update_list_attributes(assign_ptr->list, priv_cond, -1, assign_ptr->size, threadID, net, ss);
        merge_and_shrink_ptr(assign_ptr, ptr, net, ss);
        delete[] priv_cond;
        destroy_ptr(&ptr);
        return;
    }
}

// Forward declaration for mutual recursion
template <typename T>
void read_write_helper(rss_priv_ptr<T> *ptr, rss_priv_ptr<T> *result, T *priv_cond,
                       int threadID, NodeNetwork net, replicatedSecretShare<T> *ss);

template <typename T>
void reduce_dereferences(rss_priv_ptr<T> *ptr, int dereferences, rss_priv_ptr<T> *result_ptr,
                         int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    rss_priv_ptr<T> *runner = create_ptr<T>(0, ptr->type);
    set_ptr(runner, ptr, threadID);
    while (dereferences > 1) {
        read_write_helper(runner, result_ptr, (T*)nullptr, threadID, net, ss);
        set_ptr(runner, result_ptr, threadID);
        dereferences--;
    }
    set_ptr(result_ptr, runner, threadID);
    destroy_ptr(&runner);
}

// Dereference read for pointer to int
template <typename T>
void dereference_ptr_read_var(rss_priv_ptr<T> *ptr, T *result, int dereferences,
                              int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    if (ptr->type != 0) {
        printf("WRONG TYPE on the left operator...\n");
        exit(1);
    }
    uint numShares = ss->getNumShares();
    rss_priv_ptr<T> *copy_ptr = create_ptr<T>(0, 0);
    rss_priv_ptr<T> *ptr1 = create_ptr<T>(0, 0);
    rss_priv_ptr<T> *tmp_ptr;
    set_ptr(copy_ptr, ptr, threadID);
    if (dereferences > 1) {
        reduce_dereferences(copy_ptr, dereferences, ptr1, threadID, net, ss);
        tmp_ptr = ptr1;
    } else
        tmp_ptr = ptr;

    int size = tmp_ptr->size;
    rss_listnode<T> *tmp = tmp_ptr->list->head->next;

    if (size == 0) {
        memset(result, 0, sizeof(T) * numShares);
        destroy_ptr(&copy_ptr);
        destroy_ptr(&ptr1);
        return;
    }

    if (size == 1) {
        tag_copy(result, *(tmp->u.int_var_location), numShares);
        destroy_ptr(&copy_ptr);
        destroy_ptr(&ptr1);
        return;
    }

    T **op1 = new T *[size];
    T **op2 = new T *[size];
    T **product = new T *[size];

    int index = 0;
    while (tmp != tmp_ptr->list->tail) {
        op1[index] = new T[numShares];
        op2[index] = new T[numShares];
        product[index] = new T[numShares]();
        tag_copy(op1[index], *(tmp->u.int_var_location), numShares);
        tag_copy(op2[index], tmp->priv_tag, numShares);
        index++;
        tmp = tmp->next;
    }

    Mult(product, op1, op2, size, threadID, net, ss);

    memset(result, 0, sizeof(T) * numShares);
    for (int i = 0; i < size; i++)
        ss->modAdd(result, result, product[i]);

    for (int i = 0; i < size; i++) {
        delete[] op1[i];
        delete[] op2[i];
        delete[] product[i];
    }
    delete[] op1;
    delete[] op2;
    delete[] product;
    destroy_ptr(&copy_ptr);
    destroy_ptr(&ptr1);
}

// Dereference read for pointer to float
template <typename T>
void dereference_ptr_read_var(rss_priv_ptr<T> *ptr, T **result, int dereferences,
                              int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    if (ptr->type != 1) {
        printf("WRONG TYPE on the left operator...\n");
        exit(1);
    }
    uint numShares = ss->getNumShares();
    rss_priv_ptr<T> *copy_ptr = create_ptr<T>(0, 1);
    rss_priv_ptr<T> *ptr1 = create_ptr<T>(0, 1);
    rss_priv_ptr<T> *tmp_ptr;
    set_ptr(copy_ptr, ptr, threadID);
    if (dereferences > 1) {
        reduce_dereferences(copy_ptr, dereferences, ptr1, threadID, net, ss);
        tmp_ptr = ptr1;
    } else
        tmp_ptr = ptr;

    int size = tmp_ptr->size;
    rss_listnode<T> *tmp = tmp_ptr->list->head->next;

    if (size == 0) {
        for (int i = 0; i < 4; i++)
            memset(result[i], 0, sizeof(T) * numShares);
        destroy_ptr(&copy_ptr);
        destroy_ptr(&ptr1);
        return;
    }

    if (size == 1) {
        for (int i = 0; i < 4; i++)
            tag_copy(result[i], (*(tmp->u.float_var_location))[i], numShares);
        destroy_ptr(&copy_ptr);
        destroy_ptr(&ptr1);
        return;
    }

    int total = 4 * size;
    T **op1 = new T *[total];
    T **op2 = new T *[total];
    T **product = new T *[total];
    for (int i = 0; i < total; i++) {
        op1[i] = new T[numShares];
        op2[i] = new T[numShares];
        product[i] = new T[numShares]();
    }

    int index = 0;
    while (tmp != tmp_ptr->list->tail) {
        for (int i = 0; i < 4; i++) {
            tag_copy(op1[4 * index + i], (*(tmp->u.float_var_location))[i], numShares);
            tag_copy(op2[4 * index + i], tmp->priv_tag, numShares);
        }
        index++;
        tmp = tmp->next;
    }

    Mult(product, op1, op2, total, threadID, net, ss);

    for (int i = 0; i < 4; i++)
        memset(result[i], 0, sizeof(T) * numShares);
    for (int i = 0; i < size; i++)
        for (int j = 0; j < 4; j++)
            ss->modAdd(result[j], result[j], product[4 * i + j]);

    for (int i = 0; i < total; i++) {
        delete[] op1[i];
        delete[] op2[i];
        delete[] product[i];
    }
    delete[] op1;
    delete[] op2;
    delete[] product;
    destroy_ptr(&copy_ptr);
    destroy_ptr(&ptr1);
}

// read_write_helper for pointer dereference read/write
template <typename T>
void read_write_helper(rss_priv_ptr<T> *ptr, rss_priv_ptr<T> *result, T *priv_cond,
                       int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    int num = 0;
    rss_dlist<T> *list = ptr->list;
    rss_listnode<T> *node = list->head->next;
    while (node != list->tail) {
        num += compute_list_size(node->u.ptr_location->list);
        node = node->next;
    }

    if (num == 0) {
        clear_list(result->list);
        result->size = 0;
        return;
    }

    T **op1 = new T *[num];
    T **op2 = new T *[num];
    T **op3 = new T *[num];
    T **R = new T *[num];
    for (int i = 0; i < num; i++) {
        op1[i] = new T[numShares]();
        op2[i] = new T[numShares]();
        op3[i] = new T[numShares]();
        R[i] = new T[numShares]();
    }

    int index = 0;
    node = list->head->next;
    while (node != list->tail) {
        rss_dlist<T> *child_list = node->u.ptr_location->list;
        rss_listnode<T> *child_node = child_list->head->next;
        while (child_node != child_list->tail) {
            tag_copy(op1[index], node->priv_tag, numShares);
            tag_copy(op2[index], child_node->priv_tag, numShares);
            if (priv_cond != nullptr)
                tag_copy(op3[index], priv_cond, numShares);
            index++;
            child_node = child_node->next;
        }
        node = node->next;
    }

    Mult(R, op1, op2, num, threadID, net, ss);

    if (priv_cond != nullptr) {
        T *cond = new T[numShares];
        ss->modSub(cond, (T)1, priv_cond);
        update_list_attributes(result->list, cond, -1, result->size, threadID, net, ss);
        Mult(R, R, op3, num, threadID, net, ss);
        delete[] cond;
    } else {
        clear_list(result->list);
        result->size = 0;
    }

    result->level = ptr->level - 1;
    rss_priv_ptr<T> *tmp_ptr = create_ptr<T>(ptr->level - 1, ptr->type);
    copy_nested_ptr(tmp_ptr, ptr, R);
    merge_and_shrink_ptr(result, tmp_ptr, net, ss);
    destroy_ptr(&tmp_ptr);

    for (int i = 0; i < num; i++) {
        delete[] op1[i];
        delete[] op2[i];
        delete[] op3[i];
        delete[] R[i];
    }
    delete[] op1;
    delete[] op2;
    delete[] op3;
    delete[] R;
}

// Dereference read for pointer to pointer
template <typename T>
void dereference_ptr_read_ptr(rss_priv_ptr<T> *ptr, rss_priv_ptr<T> *result,
                              int dereferences, T *priv_cond,
                              int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    rss_priv_ptr<T> *copy_ptr = create_ptr<T>(ptr->level, ptr->type);
    rss_priv_ptr<T> *ptr1 = create_ptr<T>(ptr->level, ptr->type);
    set_ptr(ptr1, ptr, threadID);
    rss_priv_ptr<T> *tmp_ptr;
    if (dereferences > 1) {
        reduce_dereferences(ptr1, dereferences, copy_ptr, threadID, net, ss);
        tmp_ptr = copy_ptr;
    } else
        tmp_ptr = ptr;
    read_write_helper(tmp_ptr, result, priv_cond, threadID, net, ss);
    destroy_ptr(&copy_ptr);
    destroy_ptr(&ptr1);
}

// Dereference write: general entry point
template <typename T>
void dereference_ptr_write(rss_priv_ptr<T> *ptr, T **int_var_loc, T ***float_var_loc,
                           void *struct_var_loc, rss_priv_ptr<T> **ptr_loc, int dereferences,
                           T *priv_cond, int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    rss_priv_ptr<T> *tmp_ptr = create_ptr<T>(0, ptr->type);
    set_ptr(tmp_ptr, int_var_loc, float_var_loc, struct_var_loc, ptr_loc, threadID);
    dereference_ptr_write_ptr(ptr, tmp_ptr, dereferences, priv_cond, threadID, net, ss);
    destroy_ptr(&tmp_ptr);
}

// Dereference write for pointer-to-pointer assignment
template <typename T>
void dereference_ptr_write_ptr(rss_priv_ptr<T> *ptr, rss_priv_ptr<T> *value,
                               int dereferences, T *priv_cond,
                               int threadID, NodeNetwork net, replicatedSecretShare<T> *ss) {
    uint numShares = ss->getNumShares();
    rss_priv_ptr<T> *copy_ptr = create_ptr<T>(0, ptr->type);
    rss_priv_ptr<T> *ptr1 = create_ptr<T>(0, ptr->type);
    set_ptr(copy_ptr, ptr, threadID);
    rss_priv_ptr<T> *tmp_ptr;

    if (dereferences > 1) {
        reduce_dereferences(copy_ptr, dereferences, ptr1, threadID, net, ss);
        tmp_ptr = ptr1;
    } else
        tmp_ptr = ptr;

    rss_dlist<T> *list = tmp_ptr->list;
    rss_listnode<T> *node = list->head->next;

    while (node != list->tail) {
        rss_priv_ptr<T> *ptr2 = create_ptr<T>(0, ptr->type);
        rss_priv_ptr<T> *assign_ptr = node->u.ptr_location;
        set_ptr(ptr2, value, threadID);
        T *cond = new T[numShares];
        if (priv_cond == nullptr) {
            ss->modSub(cond, (T)1, node->priv_tag);
            update_list_attributes(assign_ptr->list, cond, -1, assign_ptr->size, threadID, net, ss);
            update_list_attributes(ptr2->list, node->priv_tag, -1, ptr2->size, threadID, net, ss);
        } else {
            T **m_op1 = new T *[1];
            T **m_op2 = new T *[1];
            T **m_R = new T *[1];
            m_op1[0] = new T[numShares];
            m_op2[0] = new T[numShares];
            m_R[0] = new T[numShares]();
            tag_copy(m_op1[0], priv_cond, numShares);
            tag_copy(m_op2[0], node->priv_tag, numShares);
            Mult(m_R, m_op1, m_op2, 1, threadID, net, ss);
            ss->modSub(cond, (T)1, m_R[0]);
            update_list_attributes(assign_ptr->list, cond, -1, assign_ptr->size, threadID, net, ss);
            update_list_attributes(ptr2->list, m_R[0], -1, ptr2->size, threadID, net, ss);
            delete[] m_op1[0];
            delete[] m_op2[0];
            delete[] m_R[0];
            delete[] m_op1;
            delete[] m_op2;
            delete[] m_R;
        }
        merge_and_shrink_ptr(assign_ptr, ptr2, net, ss);
        destroy_ptr(&ptr2);
        delete[] cond;
        node = node->next;
    }
    destroy_ptr(&copy_ptr);
    destroy_ptr(&ptr1);
}

// Dereference write for pointer to float
template <typename T>
void dereference_ptr_write_var(rss_priv_ptr<T> *ptr, T **value, int dereferences,
                               T *priv_cond, int threadID, NodeNetwork net,
                               replicatedSecretShare<T> *ss) {
    if (ptr->type != 1) {
        printf("WRONG TYPE on the left operator...\n");
        exit(1);
    }
    uint numShares = ss->getNumShares();
    rss_priv_ptr<T> *tmp_ptr;
    rss_priv_ptr<T> *copy_ptr = create_ptr<T>(0, 1);
    rss_priv_ptr<T> *ptr1 = create_ptr<T>(0, 1);
    set_ptr(ptr1, ptr, threadID);

    if (dereferences > 1) {
        reduce_dereferences(ptr1, dereferences, copy_ptr, threadID, net, ss);
        tmp_ptr = copy_ptr;
    } else
        tmp_ptr = ptr;

    int size = tmp_ptr->size;
    if (size == 0) {
        destroy_ptr(&copy_ptr);
        destroy_ptr(&ptr1);
        return;
    }
    int total = 4 * size;
    rss_listnode<T> *tmp = tmp_ptr->list->head->next;

    T **op1 = new T *[total];
    T **op2 = new T *[total];
    T **op3 = new T *[total];
    T **op4 = new T *[total];
    for (int i = 0; i < total; i++) {
        op1[i] = new T[numShares]();
        op2[i] = new T[numShares]();
        op3[i] = new T[numShares]();
        op4[i] = new T[numShares]();
    }

    int index = 0;
    while (tmp != tmp_ptr->list->tail) {
        for (int i = 0; i < 4; i++) {
            tag_copy(op1[4 * index + i], (*(tmp->u.float_var_location))[i], numShares);
            tag_copy(op2[4 * index + i], tmp->priv_tag, numShares);
            if (priv_cond != nullptr)
                tag_copy(op3[4 * index + i], priv_cond, numShares);
            tag_copy(op4[4 * index + i], value[i], numShares);
        }
        index++;
        tmp = tmp->next;
    }

    // compute (*location) + tag * priv_cond * (value - (*location))
    ss->modSub(op4, op4, op1, total);
    if (priv_cond != nullptr)
        Mult(op4, op4, op3, total, threadID, net, ss);
    if (size != 1)
        Mult(op4, op4, op2, total, threadID, net, ss);
    ss->modAdd(op4, op1, op4, total);

    tmp = tmp_ptr->list->head->next;
    index = 0;
    while (tmp != tmp_ptr->list->tail) {
        for (int i = 0; i < 4; i++)
            tag_copy((*(tmp->u.float_var_location))[i], op4[4 * index + i], numShares);
        index++;
        tmp = tmp->next;
    }

    for (int i = 0; i < total; i++) {
        delete[] op1[i];
        delete[] op2[i];
        delete[] op3[i];
        delete[] op4[i];
    }
    delete[] op1;
    delete[] op2;
    delete[] op3;
    delete[] op4;
    destroy_ptr(&ptr1);
    destroy_ptr(&copy_ptr);
}

// Dereference write for pointer to int
template <typename T>
void dereference_ptr_write_var(rss_priv_ptr<T> *ptr, T *value, int dereferences,
                               T *priv_cond, int threadID, NodeNetwork net,
                               replicatedSecretShare<T> *ss) {
    if (ptr->type != 0) {
        printf("WRONG TYPE on the left operator...\n");
        exit(1);
    }
    uint numShares = ss->getNumShares();
    rss_priv_ptr<T> *tmp_ptr;
    rss_priv_ptr<T> *copy_ptr = create_ptr<T>(0, 0);
    rss_priv_ptr<T> *ptr1 = create_ptr<T>(0, 0);
    set_ptr(ptr1, ptr, threadID);

    if (dereferences > 1) {
        reduce_dereferences(ptr1, dereferences, copy_ptr, threadID, net, ss);
        tmp_ptr = copy_ptr;
    } else
        tmp_ptr = ptr;

    int size = tmp_ptr->size;
    if (size == 0) {
        destroy_ptr(&copy_ptr);
        destroy_ptr(&ptr1);
        return;
    }
    rss_listnode<T> *tmp = tmp_ptr->list->head->next;

    T **op1 = new T *[size];
    T **op2 = new T *[size];
    T **op3 = new T *[size];
    T **op4 = new T *[size];
    for (int i = 0; i < size; i++) {
        op1[i] = new T[numShares]();
        op2[i] = new T[numShares]();
        op3[i] = new T[numShares]();
        op4[i] = new T[numShares]();
    }

    int index = 0;
    while (tmp != tmp_ptr->list->tail) {
        tag_copy(op1[index], *(tmp->u.int_var_location), numShares);
        tag_copy(op2[index], tmp->priv_tag, numShares);
        if (priv_cond != nullptr)
            tag_copy(op3[index], priv_cond, numShares);
        tag_copy(op4[index], value, numShares);
        index++;
        tmp = tmp->next;
    }

    // compute (*location) + tag * priv_cond * (value - (*location))
    ss->modSub(op4, op4, op1, size);
    if (priv_cond != nullptr)
        Mult(op4, op4, op3, size, threadID, net, ss);
    if (size != 1)
        Mult(op4, op4, op2, size, threadID, net, ss);
    ss->modAdd(op4, op1, op4, size);

    tmp = tmp_ptr->list->head->next;
    index = 0;
    while (tmp != tmp_ptr->list->tail) {
        tag_copy(*(tmp->u.int_var_location), op4[index++], numShares);
        tmp = tmp->next;
    }

    for (int i = 0; i < size; i++) {
        delete[] op1[i];
        delete[] op2[i];
        delete[] op3[i];
        delete[] op4[i];
    }
    delete[] op1;
    delete[] op2;
    delete[] op3;
    delete[] op4;
    destroy_ptr(&ptr1);
    destroy_ptr(&copy_ptr);
}

// ============================================================================
// Non-template overloads for create_ptr — needed because SMC_Utils wrappers
// call create_ptr(int, int) without any typed arg for template deduction.
// Non-template is preferred over template in overload resolution.
// ============================================================================
inline priv_ptr create_ptr(int level, int type) {
    return create_ptr<priv_int_t>(level, type);
}

inline priv_ptr *create_ptr(int level, int type, int num) {
    return create_ptr<priv_int_t>(level, type, num);
}

#endif // _PRIVPTR_HPP_
