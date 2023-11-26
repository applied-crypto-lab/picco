/* File generated from [sample-programs/non-thread/sorted-linked-list.c] by PICCO Sun Nov 26 00:47:23 2023
 */

#include <limits.h>

#include <float.h>

// extern void *memcpy(void*,const void*,unsigned int);

// # 1 "ort.onoff.defs"

extern "C" int ort_initialize(int *, char ***);
extern "C" void ort_finalize(int);

/* File generated from [sample-programs/non-thread/sorted-linked-list.c] by PICCO Sun Nov 26 00:47:23 2023
 */

#include "smc-compute/SMC_Utils.h"

#include <gmp.h>

#include <omp.h>

SMC_Utils *__s;

struct node {
    mpz_t data;
    priv_ptr next;
};

void node_init(struct node *node_node) {
    mpz_init(node_node->data);
    node_node->next = __s->smc_new_ptr(1, 2);
}

void node_free(struct node *node_node) {
    mpz_clear(node_node->data);
    __s->smc_clear_ptr(&(node_node->next));
}

void node_next(priv_ptr node, priv_ptr _picco_val, int _picco_tag, mpz_t _picco_priv_cond, int _picco_thread_id) {
    priv_ptr _picco_tmp_ptr = __s->smc_new_ptr(2, 2);
    listnode _picco_listnode = node->list->head->next;
    while (_picco_listnode != node->list->tail) {
        __s->smc_add_ptr(_picco_tmp_ptr, &(((struct node *)(_picco_listnode->u.struct_var_location))->next), _picco_listnode->priv_tag, _picco_thread_id);
        _picco_listnode = _picco_listnode->next;
    }
    if (_picco_tag == 0) {
        _picco_val->level = 1;
        __s->smc_dereference_read_ptr(_picco_tmp_ptr, _picco_val, 1, _picco_priv_cond, "struct", _picco_thread_id);
    } else
        __s->smc_dereference_write_ptr(_picco_tmp_ptr, _picco_val, 1, _picco_priv_cond, "struct", _picco_thread_id);
    __s->smc_free_ptr(&_picco_tmp_ptr);
}

void node_data(priv_ptr node, mpz_t _picco_val, int _picco_tag, mpz_t _picco_priv_cond, int _picco_thread_id) {
    printf("a\n");
    priv_ptr _picco_tmp_ptr = __s->smc_new_ptr(1, 0);
    listnode _picco_listnode = node->list->head->next;
    while (_picco_listnode != node->list->tail) {
        __s->smc_add_int_ptr(_picco_tmp_ptr, &(((struct node *)(_picco_listnode->u.struct_var_location))->data), _picco_listnode->priv_tag, _picco_thread_id);
        _picco_listnode = _picco_listnode->next;
    }
    printf("b\n");

    if (_picco_tag == 0) {
        printf("b1\n");

        __s->smc_dereference_read_ptr(_picco_tmp_ptr, _picco_val, 1, _picco_priv_cond, "int", _picco_thread_id);
    } else {
        printf("b2\n");
        __s->smc_dereference_write_ptr(_picco_tmp_ptr, _picco_val, 1, _picco_priv_cond, "int", _picco_thread_id);
    }
        printf("c\n");


    __s->smc_free_ptr(&_picco_tmp_ptr);
}

int count = 3;

int __original_main(int _argc_ignored, char **_argv_ignored) {

    mpz_t _picco_tmp1, _picco_tmp2;
    mpz_init(_picco_tmp1);
    mpz_init(_picco_tmp2);

    mpz_t *_picco_ftmp1 = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    for (int _picco_j = 0; _picco_j < 4; _picco_j++)
        mpz_init(_picco_ftmp1[_picco_j]);
    void *_picco_temp_;
    mpz_t _picco_str_field_tmp_int1, _picco_str_field_tmp_int2, _picco_str_field_tmp_int3;
    mpz_init(_picco_str_field_tmp_int1);
    mpz_init(_picco_str_field_tmp_int2);
    mpz_init(_picco_str_field_tmp_int3);
    priv_ptr _picco_str_field_tmp_struct_ptr1 = __s->smc_new_ptr(1, 2);
    priv_ptr _picco_str_field_tmp_struct_ptr2 = __s->smc_new_ptr(1, 2);
    priv_ptr _picco_str_field_tmp_struct_ptr3 = __s->smc_new_ptr(1, 2);
    priv_ptr _picco_str_field_tmp_struct_ptr4 = __s->smc_new_ptr(1, 2);
    priv_ptr _picco_str_field_tmp_struct_ptr5 = __s->smc_new_ptr(1, 2);
    priv_ptr _picco_str_field_tmp_struct_ptr6 = __s->smc_new_ptr(1, 2);
    priv_ptr _picco_tmp_struct_ptr = __s->smc_new_ptr(1, 2);
    int i, j;
    printf("1\n");
    mpz_t *a;
    a = (mpz_t *)malloc(sizeof(mpz_t) * (count));
    for (int _picco_i = 0; _picco_i < count; _picco_i++)
        mpz_init(a[_picco_i]);
    mpz_t output;
    mpz_init(output);
    mpz_t c;
    mpz_init(c);
    printf("2\n");

    priv_ptr ptr1 = __s->smc_new_ptr(1, 2);
    priv_ptr ptr2 = __s->smc_new_ptr(1, 2);
    priv_ptr head = __s->smc_new_ptr(1, 2);
    printf("3\n");

    __s->smc_input(1, a, count, "int", -1);
    printf("4\n");

    _picco_temp_ = malloc(1 * sizeof(struct node));
    for (int _picco_i = 0; _picco_i < 1; _picco_i++)
        node_init(&((struct node *)_picco_temp_)[_picco_i]);
    __s->smc_set_struct_ptr(ptr1, (struct node *)_picco_temp_, "struct", -1);
    printf("5\n");

    _picco_temp_ = malloc(1 * sizeof(struct node));
    for (int _picco_i = 0; _picco_i < 1; _picco_i++)
        node_init(&((struct node *)_picco_temp_)[_picco_i]);
    __s->smc_set_struct_ptr(ptr2, (struct node *)_picco_temp_, "struct", -1);
    printf("6\n");

    node_data(ptr1, a[0], 1, NULL, -1);
    printf("7\n");
    node_data(ptr2, a[1], 1, NULL, -1);
    printf("8\n");

    // __s->smc_lt(a[0], a[1], _picco_tmp1, 32, 32, 1, "int", -1);
    // {
    //     mpz_t *_picco_priv_float_tmp1 = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    //     for (int _picco_j = 0; _picco_j < 4; _picco_j++)
    //         mpz_init(_picco_priv_float_tmp1[_picco_j]);
    //     mpz_t _picco_condtmp1;
    //     mpz_init(_picco_condtmp1);
    //     mpz_set(_picco_condtmp1, _picco_tmp1);
    //     {
    //         __s->smc_update_ptr(head, ptr1, _picco_condtmp1, 1, -1);

    //         node_next(head, ptr2, 1, _picco_condtmp1, -1);
    //         ;
    //     }
    //     mpz_clear(_picco_condtmp1);
    //     for (int _picco_j = 0; _picco_j < 4; _picco_j++)
    //         mpz_clear(_picco_priv_float_tmp1[_picco_j]);
    //     free(_picco_priv_float_tmp1);
    // }
    // {
    //     mpz_t *_picco_priv_float_tmp1 = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    //     for (int _picco_j = 0; _picco_j < 4; _picco_j++)
    //         mpz_init(_picco_priv_float_tmp1[_picco_j]);
    //     mpz_t _picco_condtmp1;
    //     mpz_init(_picco_condtmp1);
    //     __s->smc_sub(1, _picco_tmp1, _picco_condtmp1, 1, 1, 1, "int", -1);
    //     {
    //         __s->smc_update_ptr(head, ptr2, _picco_condtmp1, 2, -1);

    //         node_next(head, ptr1, 1, _picco_condtmp1, -1);
    //         ;
    //     }
    //     mpz_clear(_picco_condtmp1);
    //     for (int _picco_j = 0; _picco_j < 4; _picco_j++)
    //         mpz_clear(_picco_priv_float_tmp1[_picco_j]);
    //     free(_picco_priv_float_tmp1);
    //     __s->smc_shrink_ptr(head, 1, -1, -1);
    // }

    // i = 2;
    // for (; i < count;) {
    //     {
    //         _picco_temp_ = malloc(1 * sizeof(struct node));
    //         for (int _picco_i = 0; _picco_i < 1; _picco_i++)
    //             node_init(&((struct node *)_picco_temp_)[_picco_i]);
    //         __s->smc_set_struct_ptr(ptr1, (struct node *)_picco_temp_, "struct", -1);

    //         node_data(ptr1, a[i], 1, NULL, -1);
    //         ;

    //         __s->smc_set_ptr(ptr2, head, "struct", -1);

    //         mpz_set_ui(_picco_str_field_tmp_int1, 0);
    //         node_data(ptr1, _picco_str_field_tmp_int1, 0, NULL, -1);
    //         mpz_set_ui(_picco_str_field_tmp_int2, 0);
    //         node_data(ptr2, _picco_str_field_tmp_int2, 0, NULL, -1);
    //         __s->smc_lt(_picco_str_field_tmp_int1, _picco_str_field_tmp_int2, _picco_tmp1, 50, 50, 1, "int", -1);
    //         {
    //             mpz_t *_picco_priv_float_tmp1 = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    //             for (int _picco_j = 0; _picco_j < 4; _picco_j++)
    //                 mpz_init(_picco_priv_float_tmp1[_picco_j]);
    //             mpz_t _picco_condtmp1;
    //             mpz_init(_picco_condtmp1);
    //             mpz_set(_picco_condtmp1, _picco_tmp1);
    //             {
    //                 node_next(ptr1, ptr2, 1, _picco_condtmp1, -1);
    //                 ;

    //                 __s->smc_update_ptr(head, ptr1, _picco_condtmp1, 3, -1);
    //             }
    //             mpz_clear(_picco_condtmp1);
    //             for (int _picco_j = 0; _picco_j < 4; _picco_j++)
    //                 mpz_clear(_picco_priv_float_tmp1[_picco_j]);
    //             free(_picco_priv_float_tmp1);
    //         }

    //         j = 0;
    //         for (; j < i - 1;) {
    //             {
    //                 mpz_set_ui(_picco_str_field_tmp_int1, 0);
    //                 node_data(ptr2, _picco_str_field_tmp_int1, 0, NULL, -1);
    //                 __s->smc_lt(_picco_str_field_tmp_int1, a[i], _picco_tmp1, 50, 32, 1, "int", -1);
    //                 __s->smc_clear_ptr(&(_picco_str_field_tmp_struct_ptr4));
    //                 node_next(ptr2, _picco_str_field_tmp_struct_ptr4, 0, NULL, -1);
    //                 __s->smc_clear_ptr(&(_picco_str_field_tmp_struct_ptr1));
    //                 __s->smc_set_ptr(_picco_str_field_tmp_struct_ptr1, _picco_str_field_tmp_struct_ptr4, "struct", -1);
    //                 mpz_set_ui(_picco_str_field_tmp_int1, 0);
    //                 node_data(_picco_str_field_tmp_struct_ptr1, _picco_str_field_tmp_int1, 0, NULL, -1);
    //                 __s->smc_gt(_picco_str_field_tmp_int1, a[i], _picco_tmp2, 50, 32, 1, "int", -1);
    //                 __s->smc_land(_picco_tmp1, _picco_tmp2, _picco_tmp1, 1, 1, 1, "int", -1);
    //                 {
    //                     mpz_t *_picco_priv_float_tmp1 = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    //                     for (int _picco_j = 0; _picco_j < 4; _picco_j++)
    //                         mpz_init(_picco_priv_float_tmp1[_picco_j]);
    //                     mpz_t _picco_condtmp1;
    //                     mpz_init(_picco_condtmp1);
    //                     mpz_set(_picco_condtmp1, _picco_tmp1);
    //                     {
    //                         __s->smc_clear_ptr(&(_picco_str_field_tmp_struct_ptr4));
    //                         node_next(ptr2, _picco_str_field_tmp_struct_ptr4, 0, _picco_condtmp1, -1);
    //                         __s->smc_clear_ptr(&(_picco_str_field_tmp_struct_ptr1));
    //                         __s->smc_set_ptr(_picco_str_field_tmp_struct_ptr1, _picco_str_field_tmp_struct_ptr4, "struct", -1);
    //                         node_next(ptr1, _picco_str_field_tmp_struct_ptr1, 1, _picco_condtmp1, -1);
    //                         ;

    //                         node_next(ptr2, ptr1, 1, _picco_condtmp1, -1);
    //                         ;
    //                     }
    //                     mpz_clear(_picco_condtmp1);
    //                     for (int _picco_j = 0; _picco_j < 4; _picco_j++)
    //                         mpz_clear(_picco_priv_float_tmp1[_picco_j]);
    //                     free(_picco_priv_float_tmp1);
    //                 }

    //                 __s->smc_clear_ptr(&(_picco_str_field_tmp_struct_ptr4));
    //                 node_next(ptr2, _picco_str_field_tmp_struct_ptr4, 0, NULL, -1);
    //                 __s->smc_clear_ptr(&(_picco_str_field_tmp_struct_ptr1));
    //                 __s->smc_set_ptr(_picco_str_field_tmp_struct_ptr1, _picco_str_field_tmp_struct_ptr4, "struct", -1);
    //                 __s->smc_set_ptr(ptr2, _picco_str_field_tmp_struct_ptr1, "struct", -1);
    //             }
    //             j++;
    //         }

    //         mpz_set_ui(_picco_str_field_tmp_int1, 0);
    //         node_data(ptr2, _picco_str_field_tmp_int1, 0, NULL, -1);
    //         mpz_set_ui(_picco_str_field_tmp_int2, 0);
    //         node_data(ptr1, _picco_str_field_tmp_int2, 0, NULL, -1);
    //         __s->smc_lt(_picco_str_field_tmp_int1, _picco_str_field_tmp_int2, _picco_tmp1, 50, 50, 1, "int", -1);
    //         {
    //             mpz_t *_picco_priv_float_tmp1 = (mpz_t *)malloc(sizeof(mpz_t) * 4);
    //             for (int _picco_j = 0; _picco_j < 4; _picco_j++)
    //                 mpz_init(_picco_priv_float_tmp1[_picco_j]);
    //             mpz_t _picco_condtmp1;
    //             mpz_init(_picco_condtmp1);
    //             mpz_set(_picco_condtmp1, _picco_tmp1);
    //             node_next(ptr2, ptr1, 1, _picco_condtmp1, -1);
    //             ;
    //             mpz_clear(_picco_condtmp1);
    //             for (int _picco_j = 0; _picco_j < 4; _picco_j++)
    //                 mpz_clear(_picco_priv_float_tmp1[_picco_j]);
    //             free(_picco_priv_float_tmp1);
    //         }
    //     }
    //     i++;
    // }

    // __s->smc_set(5, c, -1, 32, "int", -1);

    // __s->smc_set_ptr(ptr1, head, "struct", -1);

    // __s->smc_set(0, output, -1, 32, "int", -1);

    // i = 0;
    // for (; i < count;) {
    //     {
    //         mpz_set_ui(_picco_str_field_tmp_int1, 0);
    //         node_data(ptr1, _picco_str_field_tmp_int1, 0, NULL, -1);
    //         __s->smc_eqeq(_picco_str_field_tmp_int1, c, _picco_tmp1, 50, 32, 1, "int", -1);
    //         {
    //             mpz_t _picco_priv_int_tmp1;
    //             mpz_init(_picco_priv_int_tmp1);

    //             mpz_t _picco_condtmp1;
    //             mpz_init(_picco_condtmp1);
    //             mpz_set(_picco_condtmp1, _picco_tmp1);
    //             __s->smc_set(output, _picco_priv_int_tmp1, 32, 32, "int", -1);
    //             __s->smc_add(output, 1, _picco_tmp2, 32, -1, 32, "int", -1);
    //             __s->smc_set(_picco_tmp2, output, 32, 32, "int", -1);
    //             __s->smc_priv_eval(output, _picco_priv_int_tmp1, _picco_condtmp1, -1);
    //             mpz_clear(_picco_condtmp1);
    //             mpz_clear(_picco_priv_int_tmp1);
    //         }

    //         __s->smc_clear_ptr(&(_picco_str_field_tmp_struct_ptr4));
    //         node_next(ptr1, _picco_str_field_tmp_struct_ptr4, 0, NULL, -1);
    //         __s->smc_clear_ptr(&(_picco_str_field_tmp_struct_ptr1));
    //         __s->smc_set_ptr(_picco_str_field_tmp_struct_ptr1, _picco_str_field_tmp_struct_ptr4, "struct", -1);
    //         __s->smc_set_ptr(ptr1, _picco_str_field_tmp_struct_ptr1, "struct", -1);
    //     }
    //     i++;
    // }

    // __s->smc_output(1, &output, "int", -1);

    // __s->smc_clear_ptr(&(head));
    // __s->smc_clear_ptr(&(ptr2));
    // __s->smc_clear_ptr(&(ptr1));
    // mpz_clear(c);
    // mpz_clear(output);
    // for (int _picco_i = 0; _picco_i < count; _picco_i++)
    //     mpz_clear(a[_picco_i]);
    // free(a);
    // mpz_clear(_picco_tmp1);
    // mpz_clear(_picco_tmp2);
    // for (int _picco_j = 0; _picco_j < 4; _picco_j++)
    //     mpz_clear(_picco_ftmp1[_picco_j]);
    // free(_picco_ftmp1);
    // mpz_clear(_picco_str_field_tmp_int1);
    // mpz_clear(_picco_str_field_tmp_int2);
    // mpz_clear(_picco_str_field_tmp_int3);
    // __s->smc_free_ptr(&_picco_str_field_tmp_struct_ptr1);
    // __s->smc_free_ptr(&_picco_str_field_tmp_struct_ptr2);
    // __s->smc_free_ptr(&_picco_str_field_tmp_struct_ptr3);
    // __s->smc_free_ptr(&_picco_str_field_tmp_struct_ptr4);
    // __s->smc_free_ptr(&_picco_str_field_tmp_struct_ptr5);
    // __s->smc_free_ptr(&_picco_str_field_tmp_struct_ptr6);
    // __s->smc_free_ptr(&_picco_tmp_struct_ptr);
    return (0);
}

/* smc-compiler generated main() */
int main(int argc, char **argv) {

    if (argc < 8) {
        fprintf(stderr, "Incorrect input parameters\n");
        fprintf(stderr, "Usage: <id> <runtime-config> <privatekey-filename> <number-of-input-parties> <number-of-output-parties> <input-share> <output>\n");
        exit(1);
    }

    std::string IO_files[atoi(argv[4]) + atoi(argv[5])];
    for (int i = 0; i < argc - 6; i++)
        IO_files[i] = argv[6 + i];

    __s = new SMC_Utils(atoi(argv[1]), argv[2], argv[3], atoi(argv[4]), atoi(argv[5]), IO_files, 3, 1, 99, "316912650057057350374175801351", 1);

    struct timeval tv1;
    struct timeval tv2;
    int _xval = 0;

    gettimeofday(&tv1, NULL);
    _xval = (int)__original_main(argc, argv);
    gettimeofday(&tv2, NULL);
    std::cout << "Time: " << __s->time_diff(&tv1, &tv2) << std::endl;
    return (_xval);
}
