#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdbool.h>
#include <stdio.h>

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
    TYPE_FILE
} TurboType;

typedef struct TurboObject TurboObject;

// A standard function type for Turbo functions and methods.
// argc is the number of arguments, args is an array of TurboObject pointers.
typedef TurboObject* (*TurboCFunction)(int argc, TurboObject** args);

struct TurboObject {
    TurboType type;
    union {
        long long int_val;
        bool bool_val;
        struct {
            char* chars;
            int length;
        } str_val;
        struct {
            TurboObject** items;
            int length;
            int capacity;
        } list_val;
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
            // Class methods: list of names and func pointers
            char** method_names;
            TurboCFunction* method_funcs;
            int method_count;
            int method_capacity;
        } class_val;
        struct {
            TurboObject* class_obj;
            // Instance variables
            char** keys;
            TurboObject** values;
            int length;
            int capacity;
        } inst_val;
        struct {
            FILE* handle;
        } file_val;
    };
};

// Global singletons
extern TurboObject* turbo_none;
extern TurboObject* turbo_true;
extern TurboObject* turbo_false;

// Mangled built-ins
extern TurboObject* t_print;
extern TurboObject* t_len;
extern TurboObject* t_str;
extern TurboObject* t_int;
extern TurboObject* t_ord;
extern TurboObject* t_chr;
extern TurboObject* t_range;
extern TurboObject* t_open;
extern TurboObject* t_sys_argv;

// Initialization
void turbo_init(void);

// Creators
TurboObject* make_int(long long val);
TurboObject* make_str(const char* val);
TurboObject* make_str_len(const char* val, int len);
TurboObject* make_bool(bool val);
TurboObject* make_list(void);
TurboObject* make_dict(void);
TurboObject* make_func(TurboCFunction func, const char* name);
TurboObject* make_class(const char* name);
TurboObject* make_instance(TurboObject* class_obj);
TurboObject* make_file(FILE* handle);

// Operations
TurboObject* turbo_add(TurboObject* a, TurboObject* b);
TurboObject* turbo_sub(TurboObject* a, TurboObject* b);
TurboObject* turbo_mul(TurboObject* a, TurboObject* b);
TurboObject* turbo_div(TurboObject* a, TurboObject* b);
TurboObject* turbo_mod(TurboObject* a, TurboObject* b);

// Comparisons
TurboObject* turbo_eq(TurboObject* a, TurboObject* b);
TurboObject* turbo_ne(TurboObject* a, TurboObject* b);
TurboObject* turbo_lt(TurboObject* a, TurboObject* b);
TurboObject* turbo_gt(TurboObject* a, TurboObject* b);
TurboObject* turbo_lte(TurboObject* a, TurboObject* b);
TurboObject* turbo_gte(TurboObject* a, TurboObject* b);

// Logical & Unary
TurboObject* turbo_not(TurboObject* val);
TurboObject* turbo_and(TurboObject* a, TurboObject* b);
TurboObject* turbo_or(TurboObject* a, TurboObject* b);
bool turbo_is_truthy(TurboObject* val);

// Sequence & Attribute Operations
TurboObject* turbo_len(TurboObject* val);
TurboObject* turbo_getitem(TurboObject* obj, TurboObject* key);
void turbo_setitem(TurboObject* obj, TurboObject* key, TurboObject* val);
TurboObject* turbo_slice(TurboObject* obj, TurboObject* start, TurboObject* end);
TurboObject* turbo_getattr(TurboObject* obj, const char* name);
void turbo_setattr(TurboObject* obj, const char* name, TurboObject* val);
bool turbo_in(TurboObject* item, TurboObject* container);

// Built-ins
TurboObject* turbo_print(int argc, TurboObject** args);
TurboObject* turbo_str(TurboObject* val);
TurboObject* turbo_int(TurboObject* val);
TurboObject* turbo_ord(TurboObject* val);
TurboObject* turbo_chr(TurboObject* val);
TurboObject* turbo_range(int argc, TurboObject** args);
TurboObject* turbo_open(TurboObject* path, TurboObject* mode);

// File operations
TurboObject* turbo_file_read(TurboObject* file_obj);
void turbo_file_write(TurboObject* file_obj, TurboObject* text_obj);
void turbo_file_close(TurboObject* file_obj);

// List methods
void turbo_list_append(TurboObject* list_obj, TurboObject* item);

// String methods
TurboObject* turbo_str_split(int argc, TurboObject** args);

// Class registration helpers
void turbo_class_add_method(TurboObject* class_obj, const char* name, TurboCFunction func);

// Call helpers
TurboObject* turbo_call(TurboObject* callable, int argc, TurboObject** args);
TurboObject* turbo_call_method(TurboObject* obj, const char* method_name, int argc, TurboObject** args);

#endif // RUNTIME_H
