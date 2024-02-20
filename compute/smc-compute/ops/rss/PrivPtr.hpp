#ifndef _PRIVPTR_HPP_
#define _PRIVPTR_HPP_

#include "../../NodeNetwork.h"
#include "../../rss/RepSecretShare.hpp"

template <typename T>
struct listnode;

template <typename T >
struct priv_ptr;

template <typename T>
struct dlist;

template <typename T>
struct listnode {
    union {
        void *struct_var_location;
        T **int_var_location;
        T ***float_var_location;
        priv_ptr<T> * ptr_location;
    } u;
    T *priv_tag;
    int if_index;
    struct listnode<T> *prev, *next;
};

// struct priv_ptr<T>

template <typename T>
struct dlist {
    listnode<T> head;
    listnode<T> tail;
};

template <typename T  = uint32_t >
struct priv_ptr {
    dlist<T> list;
    int size;
    int level;
    int type; // type = 0 - int, = 1 - float, = 2 - struct
};

// typedef struct listnode_ *listnode<T>;
// typedef struct list_ *dlist<T>;
// typedef struct priv_ptr_ *priv_ptr<T>;

// struct listnode_ {
//     union {
//         void *struct_var_location;
//         T* *int_var_location;
//         T* **float_var_location;
//         priv_ptr<T> ptr_location;
//     } u;
//     T* priv_tag;
//     int if_index;
//     struct listnode_ *prev, *next;
// };

// struct list_ {
//     listnode<T> head;
//     listnode<T> tail;
// };

// struct priv_ptr_ {
//     dlist<T> list;
//     int size;
//     int level;
//     int type; // type = 0 - int, = 1 - float, = 2 - struct
// };

// class PrivPtr : public Operation {
// public:
// PrivPtr(NodeNetwork nodeNet, int nodeID, replicatedSecretShare<T> *s);
// virtual ~PrivPtr();
/******************************/
template <typename T>
listnode<T> create_listnode(T **, T ***, void *, priv_ptr<T>, T *, int){

}

template <typename T>
listnode<T> create_listnode(){

}

template <typename T>
void copy_listnode(listnode<T>, listnode<T>, int, int){

}
template <typename T>
dlist<T> create_list(){

    dlist<T> list;
    //  = (dlist)malloc(sizeof(struct list_));
    // list->head = create_listnode();
    // list->tail = create_listnode();
    // list->head->next = list->tail;
    // list->tail->prev = list->head;
    return list;

}
template <typename T>
int list_size(dlist<T>){

}
template <typename T>
void insert_to_rear(dlist<T>, listnode<T>){

}
template <typename T>
void delete_from_list(listnode<T> *){

}
template <typename T>
void clear_list(dlist<T> *){

}
template <typename T>
void copy_list(dlist<T>, dlist<T>, int, int){

}
template <typename T>
int compute_list_size(dlist<T>){

}
template <typename T>
void append_list(dlist<T>, dlist<T>){

}
template <typename T>
void destroy_listnode(listnode<T> *){

}
template <typename T>
void destroy_list(dlist<T> *){

}
template <typename T>
priv_ptr<T> create_ptr(int level, int type){
    priv_ptr<T> ptr;
    ptr.list = create_list<T>();
    ptr.size = 0;
    ptr.level = level;
    ptr.type = type;
    // (struct priv_ptr<T> )malloc(sizeof(struct priv_ptr));
    //  = (priv_ptr)malloc(sizeof(struct priv_ptr_));
    // ptr->list = create_list();
    // ptr->size = 0;
    // ptr->level = level;
    // ptr->type = type;
    return ptr;
}

template <typename T>
priv_ptr<T> *create_ptr(int level, int type, int num) {
    priv_ptr<T> *ptrs;
    return ptrs;
}

template <typename T>
void append_ptr_list(priv_ptr<T>, priv_ptr<T>){

}
template <typename T>
void clear_ptr(priv_ptr<T> *){

}
template <typename T>
void destroy_ptr(priv_ptr<T> *){

}
template <typename T>
void destroy_ptr(priv_ptr<T> **, int){

}
template <typename T>
void copy_nested_ptr(priv_ptr<T>, priv_ptr<T>, T **){

}
template <typename T>
void set_ptr(priv_ptr<T>, T **, T ***, void *, priv_ptr<T> *, int threadID){

}
template <typename T>
void set_ptr(priv_ptr<T>, priv_ptr<T>, int threadID){

}
template <typename T>
void add_ptr(priv_ptr<T>, T **, T ***, void *, priv_ptr<T> *, T *, int threadID){

}
template <typename T>
void shrink_ptr(priv_ptr<T>, int, int, int threadID){

}
template <typename T>
void merge_and_shrink_ptr(priv_ptr<T>, priv_ptr<T>, NodeNetwork net, int id, replicatedSecretShare<T> *ss){

}
template <typename T>
void reduce_dereferences(priv_ptr<T>, int, priv_ptr<T>, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){

}
template <typename T>
void dereference_ptr_read_ptr(priv_ptr<T>, priv_ptr<T>, int, T *, int threadID){

}
template <typename T>
void dereference_ptr_read_ptr(priv_ptr<T> ptr, priv_ptr<T> result, int dereferences, T *priv_cond, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){

}
template <typename T>
void dereference_ptr_write(priv_ptr<T>, T **, T ***, void *, priv_ptr<T> *, int, T *, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){

}
template <typename T>
void update_list_attributes(dlist<T> list, T *priv_cond, int if_index, int size, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){

}
template <typename T>
void update_ptr(priv_ptr<T> ptr, T **int_var_loc, T ***float_var_loc, void *struct_var_loc, priv_ptr<T> *ptr_loc, T *private_tag, int index, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){

}
template <typename T>
void update_ptr(priv_ptr<T> assign_ptr, priv_ptr<T> right_ptr, T *private_tag, int index, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){

}
template <typename T>
int is_repeated_listnode(dlist<T> list, listnode<T> node, int level, int type, NodeNetwork net, int id, replicatedSecretShare<T> *ss){

}
template <typename T>
void dereference_ptr_read_var(priv_ptr<T> ptr, T *result, int dereferences, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){

}
template <typename T>
void dereference_ptr_read_var(priv_ptr<T> ptr, T **result, int dereferences, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){

}
template <typename T>
void read_write_helper(priv_ptr<T> ptr, priv_ptr<T> result, T *priv_cond, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){

}
template <typename T>
void dereference_ptr_write_ptr(priv_ptr<T> ptr, priv_ptr<T> value, int dereferences, T *priv_cond, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){

}
template <typename T>
void dereference_ptr_write_var(priv_ptr<T> ptr, T **value, int dereferences, T *priv_cond, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){

}
template <typename T>
void dereference_ptr_write_var(priv_ptr<T> ptr, T *value, int dereferences, T *priv_cond, int threadID, NodeNetwork net, int id, replicatedSecretShare<T> *ss){

}
#endif // _PRIVPTR_HPP_