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

class PrivPtr : public Operation {
public:
    PrivPtr(NodeNetwork nodeNet, int nodeID, SecretShare *s);
    virtual ~PrivPtr();
    /******************************/
    listnode create_listnode(mpz_t *, mpz_t **, void *, priv_ptr, mpz_t, int);
    listnode create_listnode();
    void copy_listnode(listnode, listnode, int, int);
    int is_repeated_listnode(dlist, listnode, int, int);
    dlist create_list();
    int list_size(dlist);
    void insert_to_rear(dlist, listnode);
    void delete_from_list(listnode *);
    void clear_list(dlist *);
    void copy_list(dlist, dlist, int, int);
    int compute_list_size(dlist);
    void append_list(dlist, dlist);
    void update_list_attributes(dlist, mpz_t, int, int, int threadID);
    void destroy_listnode(listnode *);
    void destroy_list(dlist *);
    static int n;
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
    void update_ptr(priv_ptr, mpz_t *, mpz_t **, void *, priv_ptr *, mpz_t, int, int threadID);
    void update_ptr(priv_ptr, priv_ptr, mpz_t, int, int threadID);
    void add_ptr(priv_ptr, mpz_t *, mpz_t **, void *, priv_ptr *, mpz_t, int threadID);
    void shrink_ptr(priv_ptr, int, int, int threadID);
    void merge_and_shrink_ptr(priv_ptr, priv_ptr);
    void reduce_dereferences(priv_ptr, int, priv_ptr, int threadID);
    void read_write_helper(priv_ptr, priv_ptr, mpz_t, int threadID);
    void dereference_ptr_read_var(priv_ptr, mpz_t, int, int threadID);
    void dereference_ptr_read_var(priv_ptr, mpz_t *, int, int threadID);
    void dereference_ptr_read_ptr(priv_ptr, priv_ptr, int, mpz_t, int threadID);
    void dereference_ptr_write(priv_ptr, mpz_t *, mpz_t **, void *, priv_ptr *, int, mpz_t, int threadID);
    void dereference_ptr_write_ptr(priv_ptr, priv_ptr, int, mpz_t, int threadID);
    void dereference_ptr_write_var(priv_ptr, mpz_t *, int, mpz_t, int threadID);
    void dereference_ptr_write_var(priv_ptr, mpz_t, int, mpz_t, int threadID);

    // Mult *Mul;
    static int creation;
};

#endif
