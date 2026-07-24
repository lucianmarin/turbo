#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdbool.h>
#include <stdio.h>
#include <setjmp.h>

typedef enum {
    TYPE_NONE,
    TYPE_INT,
    TYPE_BOOL,
    TYPE_STR,
    TYPE_LIST,
    TYPE_DICT,
    TYPE_FUNC,
    TYPE_CLASS,
    TYPE_INSTANCE,
    TYPE_FILE,
    TYPE_FLOAT,
    TYPE_COMPLEX,
    TYPE_BYTES,
    TYPE_TUPLE,
    TYPE_SET,
    TYPE_MODULE,
    TYPE_SUPER,
    TYPE_STATICMETHOD,
    TYPE_CLASSMETHOD,
    TYPE_PROPERTY
} TurboType;

typedef struct TurboObject TurboObject;

typedef TurboObject* (*TurboCFunction)(int argc, TurboObject** args);

struct TurboObject {
    TurboType type;
    union {
        struct {
            char* digits;
            int length;
            int sign;
        } int_val;
        bool bool_val;
        double float_val;
        struct {
            double real;
            double imag;
        } complex_val;
        struct {
            char* chars;
            int length;
        } str_val;
        struct {
            unsigned char* data;
            int length;
        } bytes_val;
        struct {
            TurboObject** items;
            int length;
            int capacity;
        } list_val;
        struct {
            TurboObject** items;
            int length;
            int capacity;
        } tuple_val;
        struct {
            TurboObject** items;
            int length;
            int capacity;
        } set_val;
        struct {
            TurboObject** keys;
            TurboObject** values;
            int length;
            int capacity;
        } dict_val;
        struct {
            TurboCFunction func_ptr;
            char* name;
        } func_val;
        struct {
            char* name;
            char** method_names;
            TurboCFunction* method_funcs;
            int method_count;
            int method_capacity;
            char** attr_names;
            TurboObject** attr_values;
            int attr_count;
            int attr_capacity;
        } class_val;
        struct {
            TurboObject* class_obj;
            char** keys;
            TurboObject** values;
            int length;
            int capacity;
        } inst_val;
        struct {
            FILE* handle;
        } file_val;
        struct {
            TurboObject* dict;
        } module_val;
        struct {
            TurboObject* obj;
        } super_val;
        struct {
            TurboCFunction func_ptr;
            char* name;
        } staticmethod_val;
        struct {
            TurboCFunction func_ptr;
            char* name;
        } classmethod_val;
        struct {
            TurboCFunction getter;
            TurboCFunction setter;
            char* name;
        } property_val;
    };
};

extern TurboObject* turbo_none;
extern TurboObject* turbo_true;
extern TurboObject* turbo_false;

extern TurboObject* t_print;
extern TurboObject* t_len;
extern TurboObject* t_str;
extern TurboObject* t_int;
extern TurboObject* t_ord;
extern TurboObject* t_chr;
extern TurboObject* t_range;
extern TurboObject* t_open;
extern TurboObject* t_sys_argv;
extern TurboObject* t_input;
extern TurboObject* t_type;
extern TurboObject* t_isinstance;
extern TurboObject* t_hasattr;
extern TurboObject* t_getattr;
extern TurboObject* t_setattr;
extern TurboObject* t_repr;
extern TurboObject* t_abs;
extern TurboObject* t_round;
extern TurboObject* t_pow;
extern TurboObject* t_hex;
extern TurboObject* t_bin;
extern TurboObject* t_oct;
extern TurboObject* t_float;
extern TurboObject* t_bool;
extern TurboObject* t_list;
extern TurboObject* t_dict;
extern TurboObject* t_tuple;
extern TurboObject* t_set;
extern TurboObject* t_super;
extern TurboObject* t_iter;
extern TurboObject* t_next;
extern TurboObject* t_all;
extern TurboObject* t_any;
extern TurboObject* t_sum;
extern TurboObject* t_min;
extern TurboObject* t_max;
extern TurboObject* t_sorted;
extern TurboObject* t_reversed;
extern TurboObject* t_enumerate;
extern TurboObject* t_zip;
extern TurboObject* t_map;
extern TurboObject* t_filter;

// Exception handling
extern jmp_buf* turbo_exception_jmp;
extern TurboObject* turbo_exception_value;

void turbo_raise(TurboObject* exc);
bool turbo_exception_matches(TurboObject* exc, TurboObject* exc_type);

void turbo_init(void);

TurboObject* make_int(const char* str);
TurboObject* make_int_from_ll(long long val);
long long int_to_ll(TurboObject* obj);
TurboObject* make_float(double val);
TurboObject* make_complex(double real, double imag);
TurboObject* make_str(const char* val);
TurboObject* make_str_len(const char* val, int len);
TurboObject* make_bool(bool val);
TurboObject* make_bytes(const unsigned char* data, int length);
TurboObject* make_bytes_from_str(const char* str);
TurboObject* make_list(void);
TurboObject* make_tuple(void);
TurboObject* make_tuple_from_list(TurboObject* list);
TurboObject* make_set(void);
TurboObject* make_dict(void);
TurboObject* make_func(TurboCFunction func, const char* name);
TurboObject* make_staticmethod(TurboCFunction func, const char* name);
TurboObject* make_classmethod(TurboCFunction func, const char* name);
TurboObject* make_property(TurboCFunction getter, TurboCFunction setter, const char* name);
TurboObject* make_class(const char* name);
TurboObject* make_instance(TurboObject* class_obj);
TurboObject* make_file(FILE* handle);
TurboObject* make_module(void);
TurboObject* turbo_module_get(TurboObject* mod, const char* name);
void turbo_module_set(TurboObject* mod, const char* name, TurboObject* val);
TurboObject* turbo_import_module(const char* name);

TurboObject* turbo_add(TurboObject* a, TurboObject* b);
TurboObject* turbo_sub(TurboObject* a, TurboObject* b);
TurboObject* turbo_mul(TurboObject* a, TurboObject* b);
TurboObject* turbo_div(TurboObject* a, TurboObject* b);
TurboObject* turbo_mod(TurboObject* a, TurboObject* b);
TurboObject* turbo_pow(TurboObject* a, TurboObject* b);
TurboObject* turbo_floordiv(TurboObject* a, TurboObject* b);

TurboObject* turbo_bitand(TurboObject* a, TurboObject* b);
TurboObject* turbo_bitor(TurboObject* a, TurboObject* b);
TurboObject* turbo_bitxor(TurboObject* a, TurboObject* b);
TurboObject* turbo_lshift(TurboObject* a, TurboObject* b);
TurboObject* turbo_rshift(TurboObject* a, TurboObject* b);
TurboObject* turbo_matmul(TurboObject* a, TurboObject* b);
TurboObject* turbo_bitnot(TurboObject* a);

TurboObject* turbo_eq(TurboObject* a, TurboObject* b);
TurboObject* turbo_ne(TurboObject* a, TurboObject* b);
TurboObject* turbo_lt(TurboObject* a, TurboObject* b);
TurboObject* turbo_gt(TurboObject* a, TurboObject* b);
TurboObject* turbo_lte(TurboObject* a, TurboObject* b);
TurboObject* turbo_gte(TurboObject* a, TurboObject* b);

TurboObject* turbo_not(TurboObject* val);
TurboObject* turbo_and(TurboObject* a, TurboObject* b);
TurboObject* turbo_or(TurboObject* a, TurboObject* b);
bool turbo_is_truthy(TurboObject* val);

TurboObject* turbo_len(TurboObject* val);
TurboObject* turbo_getitem(TurboObject* obj, TurboObject* key);
void turbo_setitem(TurboObject* obj, TurboObject* key, TurboObject* val);
void turbo_delitem(TurboObject* obj, TurboObject* key);
TurboObject* turbo_slice(TurboObject* obj, TurboObject* start, TurboObject* end);
TurboObject* turbo_getattr(TurboObject* obj, const char* name);
void turbo_setattr(TurboObject* obj, const char* name, TurboObject* val);
bool turbo_in(TurboObject* item, TurboObject* container);

TurboObject* turbo_print(int argc, TurboObject** args);
TurboObject* turbo_str(TurboObject* val);
TurboObject* turbo_int(TurboObject* val);
TurboObject* turbo_float(TurboObject* val);
TurboObject* turbo_ord(TurboObject* val);
TurboObject* turbo_chr(TurboObject* val);
TurboObject* turbo_range(int argc, TurboObject** args);
TurboObject* turbo_open(TurboObject* path, TurboObject* mode);
TurboObject* turbo_repr(TurboObject* val);

TurboObject* turbo_file_read(TurboObject* file_obj);
void turbo_file_write(TurboObject* file_obj, TurboObject* text_obj);
void turbo_file_close(TurboObject* file_obj);

void turbo_list_append(TurboObject* list_obj, TurboObject* item);
TurboObject* turbo_list_copy(TurboObject* list_obj);
void turbo_list_sort(TurboObject* list_obj);
void turbo_list_reverse(TurboObject* list_obj);
void turbo_tuple_append(TurboObject* tuple_obj, TurboObject* item);
void turbo_set_add(TurboObject* set_obj, TurboObject* item);
bool turbo_set_contains(TurboObject* set_obj, TurboObject* item);
void turbo_set_remove(TurboObject* set_obj, TurboObject* item);

TurboObject* turbo_str_split(int argc, TurboObject** args);
TurboObject* turbo_str_join(TurboObject* self, TurboObject* list_obj);
TurboObject* turbo_str_upper(TurboObject* self);
TurboObject* turbo_str_lower(TurboObject* self);
TurboObject* turbo_str_replace(TurboObject* self, TurboObject* old, TurboObject* new);
TurboObject* turbo_str_find(TurboObject* self, TurboObject* sub);
TurboObject* turbo_str_strip(TurboObject* self);
TurboObject* turbo_str_lstrip(TurboObject* self);
TurboObject* turbo_str_rstrip(TurboObject* self);
TurboObject* turbo_str_startswith(TurboObject* self, TurboObject* prefix);
TurboObject* turbo_str_endswith(TurboObject* self, TurboObject* suffix);
TurboObject* turbo_str_count(TurboObject* self, TurboObject* sub);
TurboObject* turbo_str_format(TurboObject* self, int argc, TurboObject** args);

// Dict methods
TurboObject* turbo_dict_keys(TurboObject* self);
TurboObject* turbo_dict_values(TurboObject* self);
TurboObject* turbo_dict_items(TurboObject* self);
TurboObject* turbo_dict_get(TurboObject* self, TurboObject* key, TurboObject* default_val);
TurboObject* turbo_dict_pop(TurboObject* self, TurboObject* key);
TurboObject* turbo_dict_popitem(TurboObject* self);
void turbo_dict_update(TurboObject* self, TurboObject* other);
void turbo_dict_clear(TurboObject* self);
TurboObject* turbo_dict_copy(TurboObject* self);
TurboObject* turbo_dict_setdefault(TurboObject* self, TurboObject* key, TurboObject* default_val);

void turbo_class_add_method(TurboObject* class_obj, const char* name, TurboCFunction func);
void turbo_class_set_attr(TurboObject* class_obj, const char* name, TurboObject* val);
TurboObject* turbo_class_get_attr(TurboObject* class_obj, const char* name);

TurboObject* turbo_call(TurboObject* callable, int argc, TurboObject** args);
TurboObject* turbo_call_method(TurboObject* obj, const char* method_name, int argc, TurboObject** args);

#endif
