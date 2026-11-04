#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <math.h>

/*
 * 64-bit NaN-Boxed / Tagged Value Representation (TurboVal = i64 in LLVM IR)
 *
 * Every value is passed in a single 64-bit register (i64 / int64_t):
 * - Unboxed 48-bit signed int: 0xFFF8_0000_0000_0000 | (val & 0x0000_FFFF_FFFF_FFFF)
 * - Heap struct pointer:       0xFFF9_0000_0000_0000 | (ptr & 0x0000_FFFF_FFFF_FFFF)
 * - None singleton:            0xFFFA_0000_0000_0000 (-1688849860263936)
 * - False singleton:           0xFFFA_0000_0000_0002 (-1688849860263934)
 * - True singleton:            0xFFFA_0000_0000_0003 (-1688849860263933)
 * - Unboxed IEEE-754 double:   any other 64-bit pattern
 */

typedef int64_t TurboVal;

#define TV_TAG_MASK   0xFFFF000000000000ULL
#define TV_PAY_MASK   0x0000FFFFFFFFFFFFULL

#define TV_TAG_INT    0xFFF8000000000000ULL
#define TV_TAG_PTR    0xFFF9000000000000ULL
#define TV_TAG_SPEC   0xFFFA000000000000ULL

#define TV_NONE       ((TurboVal)0xFFFA000000000000ULL)
#define TV_FALSE      ((TurboVal)0xFFFA000000000002ULL)
#define TV_TRUE       ((TurboVal)0xFFFA000000000003ULL)

#define TV_MIN_INT48  (-140737488355328LL)
#define TV_MAX_INT48  ( 140737488355327LL)

/* Heap object tags (first i32 of every compact heap struct) */
typedef enum {
    HEAP_STR = 1,
    HEAP_LIST = 2,
    HEAP_DICT = 3,
    HEAP_TUPLE = 4,
    HEAP_SET = 5,
    HEAP_BYTES = 6,
    HEAP_COMPLEX = 7,
    HEAP_BIGINT = 8,
    HEAP_FUNC = 9,
    HEAP_CLASS = 10,
    HEAP_INSTANCE = 11,
    HEAP_FILE = 12,
    HEAP_MODULE = 13,
    HEAP_SUPER = 14,
    HEAP_STATICMETHOD = 15,
    HEAP_CLASSMETHOD = 16,
    HEAP_PROPERTY = 17
} TurboHeapType;

typedef TurboVal (*TurboCFunction)(int32_t argc, TurboVal* args);

/* Compact Heap Structs matching LLVM IR %String, %List, %Dict, etc. */

typedef struct {
    int32_t tag;
    int32_t length;
} TurboHeapHeader;

/* %String = type { i32, i32, i8* } */
typedef struct {
    int32_t tag;
    int32_t length;
    char* chars;
} TurboStr;

/* %List = type { i32, i32, i32, i32, i64* } (also used for Tuple and Set) */
typedef struct {
    int32_t tag;
    int32_t length;
    int32_t capacity;
    int32_t _pad;
    TurboVal* items;
} TurboList;

/* %Dict = type { i32, i32, i32, i32, i64*, i64* } */
typedef struct {
    int32_t tag;
    int32_t length;
    int32_t capacity;
    int32_t _pad;
    TurboVal* keys;
    TurboVal* values;
} TurboDict;

/* %Bytes = type { i32, i32, i8* } */
typedef struct {
    int32_t tag;
    int32_t length;
    unsigned char* data;
} TurboBytes;

/* %Complex = type { i32, i32, double, double } */
typedef struct {
    int32_t tag;
    int32_t _pad;
    double real;
    double imag;
} TurboComplex;

/* %BigInt = type { i32, i32, i32, i32, i8* } */
typedef struct {
    int32_t tag;
    int32_t length;
    int32_t sign;
    int32_t _pad;
    char* digits;
} TurboBigInt;

/* %Func = type { i32, i32, i8*, i8* } (also used for staticmethod, classmethod) */
typedef struct {
    int32_t tag;
    int32_t _pad;
    TurboCFunction func_ptr;
    char* name;
} TurboFunc;

/* %Property = type { i32, i32, i8*, i8*, i8* } */
typedef struct {
    int32_t tag;
    int32_t _pad;
    TurboCFunction getter;
    TurboCFunction setter;
    char* name;
} TurboProperty;

/* %Class */
typedef struct {
    int32_t tag;
    int32_t method_count;
    int32_t method_capacity;
    int32_t attr_count;
    int32_t attr_capacity;
    int32_t _pad;
    char* name;
    char** method_names;
    TurboCFunction* method_funcs;
    char** attr_names;
    TurboVal* attr_values;
} TurboClass;

/* %Instance = type { i32, i32, i32, i32, i64, i8**, i64* } */
typedef struct {
    int32_t tag;
    int32_t length;
    int32_t capacity;
    int32_t _pad;
    TurboVal class_obj;
    char** keys;
    TurboVal* values;
} TurboInstance;

/* %File = type { i32, i32, i8* } */
typedef struct {
    int32_t tag;
    int32_t _pad;
    FILE* handle;
} TurboFile;

/* %Module = type { i32, i32, i64 } */
typedef struct {
    int32_t tag;
    int32_t _pad;
    TurboVal dict;
} TurboModule;

/* %Super = type { i32, i32, i64 } */
typedef struct {
    int32_t tag;
    int32_t _pad;
    TurboVal obj;
} TurboSuper;

/* Global built-in variables */
extern TurboVal turbo_none;
extern TurboVal turbo_true;
extern TurboVal turbo_false;

extern TurboVal t_print;
extern TurboVal t_len;
extern TurboVal t_str;
extern TurboVal t_int;
extern TurboVal t_ord;
extern TurboVal t_chr;
extern TurboVal t_range;
extern TurboVal t_open;
extern TurboVal t_sys_argv;
extern TurboVal t_input;
extern TurboVal t_type;
extern TurboVal t_isinstance;
extern TurboVal t_hasattr;
extern TurboVal t_getattr;
extern TurboVal t_setattr;
extern TurboVal t_repr;
extern TurboVal t_abs;
extern TurboVal t_round;
extern TurboVal t_pow;
extern TurboVal t_hex;
extern TurboVal t_bin;
extern TurboVal t_oct;
extern TurboVal t_float;
extern TurboVal t_bool;
extern TurboVal t_list;
extern TurboVal t_dict;
extern TurboVal t_tuple;
extern TurboVal t_set;
extern TurboVal t_super;
extern TurboVal t_iter;
extern TurboVal t_next;
extern TurboVal t_all;
extern TurboVal t_any;
extern TurboVal t_sum;
extern TurboVal t_min;
extern TurboVal t_max;
extern TurboVal t_sorted;
extern TurboVal t_reversed;
extern TurboVal t_enumerate;
extern TurboVal t_zip;
extern TurboVal t_map;
extern TurboVal t_filter;

/* Exception handling */
extern jmp_buf* turbo_exception_jmp;
extern TurboVal turbo_exception_value;

jmp_buf* turbo_try_push(void);
void turbo_try_pop(void);
TurboVal turbo_get_exception(void);
void turbo_raise(TurboVal exc);
int32_t turbo_exception_matches(TurboVal exc, TurboVal exc_type);
void turbo_assert_fail(TurboVal msg);

void turbo_init(void);
void turbo_init_args(int32_t argc, char** argv);

/* Constructors and unboxing helpers */
TurboVal make_int(const char* str);
TurboVal make_int_from_ll(long long val);
double to_double(TurboVal obj);
long long int_to_ll(TurboVal obj);
TurboVal make_float(double val);
TurboVal make_complex(double real, double imag);
TurboVal make_str(const char* val);
TurboVal make_str_len(const char* val, int32_t len);
TurboVal make_bool(int32_t val);
TurboVal make_bytes(const unsigned char* data, int32_t length);
TurboVal make_bytes_from_str(const char* str);
TurboVal make_list(void);
TurboVal make_tuple(void);
TurboVal make_tuple_from_list(TurboVal list);
TurboVal make_set(void);
TurboVal make_dict(void);
TurboVal make_func(TurboCFunction func, const char* name);
TurboVal make_staticmethod(TurboCFunction func, const char* name);
TurboVal make_classmethod(TurboCFunction func, const char* name);
TurboVal make_property(TurboCFunction getter, TurboCFunction setter, const char* name);
TurboVal make_class(const char* name);
TurboVal make_instance(TurboVal class_obj);
TurboVal make_file(FILE* handle);
TurboVal make_module(void);
TurboVal turbo_module_get(TurboVal mod, const char* name);
void turbo_module_set(TurboVal mod, const char* name, TurboVal val);
TurboVal turbo_import_module(const char* name);

/* Arithmetic and Bitwise Operations */
TurboVal turbo_add(TurboVal a, TurboVal b);
TurboVal turbo_sub(TurboVal a, TurboVal b);
TurboVal turbo_mul(TurboVal a, TurboVal b);
TurboVal turbo_div(TurboVal a, TurboVal b);
TurboVal turbo_mod(TurboVal a, TurboVal b);
TurboVal turbo_pow(TurboVal a, TurboVal b);
TurboVal turbo_floordiv(TurboVal a, TurboVal b);

TurboVal turbo_bitand(TurboVal a, TurboVal b);
TurboVal turbo_bitor(TurboVal a, TurboVal b);
TurboVal turbo_bitxor(TurboVal a, TurboVal b);
TurboVal turbo_lshift(TurboVal a, TurboVal b);
TurboVal turbo_rshift(TurboVal a, TurboVal b);
TurboVal turbo_matmul(TurboVal a, TurboVal b);
TurboVal turbo_bitnot(TurboVal a);

/* Comparisons and Logic */
TurboVal turbo_eq(TurboVal a, TurboVal b);
TurboVal turbo_ne(TurboVal a, TurboVal b);
TurboVal turbo_lt(TurboVal a, TurboVal b);
TurboVal turbo_gt(TurboVal a, TurboVal b);
TurboVal turbo_lte(TurboVal a, TurboVal b);
TurboVal turbo_gte(TurboVal a, TurboVal b);

TurboVal turbo_not(TurboVal val);
TurboVal turbo_and(TurboVal a, TurboVal b);
TurboVal turbo_or(TurboVal a, TurboVal b);
int32_t turbo_is_truthy(TurboVal val);

/* Subscript, Slice, Attribute */
TurboVal turbo_len(TurboVal val);
TurboVal turbo_getitem(TurboVal obj, TurboVal key);
void turbo_setitem(TurboVal obj, TurboVal key, TurboVal val);
void turbo_delitem(TurboVal obj, TurboVal key);
TurboVal turbo_slice(TurboVal obj, TurboVal start, TurboVal end);
TurboVal turbo_getattr(TurboVal obj, const char* name);
void turbo_setattr(TurboVal obj, const char* name, TurboVal val);
int32_t turbo_in(TurboVal item, TurboVal container);

/* Built-in Functions */
TurboVal turbo_print(int32_t argc, TurboVal* args);
TurboVal turbo_str(TurboVal val);
TurboVal turbo_int(TurboVal val);
TurboVal turbo_float(TurboVal val);
TurboVal turbo_ord(TurboVal val);
TurboVal turbo_chr(TurboVal val);
TurboVal turbo_range(int32_t argc, TurboVal* args);
TurboVal turbo_open(TurboVal path, TurboVal mode);
TurboVal turbo_repr(TurboVal val);

/* File Operations */
TurboVal turbo_file_read(TurboVal file_obj);
void turbo_file_write(TurboVal file_obj, TurboVal text_obj);
void turbo_file_close(TurboVal file_obj);

/* Collection Operations */
void turbo_list_append(TurboVal list_obj, TurboVal item);
TurboVal turbo_list_copy(TurboVal list_obj);
void turbo_list_sort(TurboVal list_obj);
void turbo_list_reverse(TurboVal list_obj);
void turbo_tuple_append(TurboVal tuple_obj, TurboVal item);
void turbo_set_add(TurboVal set_obj, TurboVal item);
int32_t turbo_set_contains(TurboVal set_obj, TurboVal item);
void turbo_set_remove(TurboVal set_obj, TurboVal item);

/* String Methods */
TurboVal turbo_str_split(int32_t argc, TurboVal* args);
TurboVal turbo_str_join(TurboVal self, TurboVal list_obj);
TurboVal turbo_str_upper(TurboVal self);
TurboVal turbo_str_lower(TurboVal self);
TurboVal turbo_str_replace(TurboVal self, TurboVal old_s, TurboVal new_s);
TurboVal turbo_str_find(TurboVal self, TurboVal sub);
TurboVal turbo_str_strip(TurboVal self);
TurboVal turbo_str_lstrip(TurboVal self);
TurboVal turbo_str_rstrip(TurboVal self);
TurboVal turbo_str_startswith(TurboVal self, TurboVal prefix);
TurboVal turbo_str_endswith(TurboVal self, TurboVal suffix);
TurboVal turbo_str_count(TurboVal self, TurboVal sub);
TurboVal turbo_str_format(TurboVal self, int32_t argc, TurboVal* args);

/* Dict Methods */
TurboVal turbo_dict_keys(TurboVal self);
TurboVal turbo_dict_values(TurboVal self);
TurboVal turbo_dict_items(TurboVal self);
TurboVal turbo_dict_get(TurboVal self, TurboVal key, TurboVal default_val);
TurboVal turbo_dict_pop(TurboVal self, TurboVal key);
TurboVal turbo_dict_popitem(TurboVal self);
void turbo_dict_update(TurboVal self, TurboVal other);
void turbo_dict_clear(TurboVal self);
TurboVal turbo_dict_copy(TurboVal self);
TurboVal turbo_dict_setdefault(TurboVal self, TurboVal key, TurboVal default_val);

/* Class & Method Dispatch */
void turbo_class_add_method(TurboVal class_obj, const char* name, TurboCFunction func);
void turbo_class_set_attr(TurboVal class_obj, const char* name, TurboVal val);
TurboVal turbo_class_get_attr(TurboVal class_obj, const char* name);

TurboVal turbo_call(TurboVal callable, int32_t argc, TurboVal* args);
TurboVal turbo_call_method(TurboVal obj, const char* method_name, int32_t argc, TurboVal* args);

#endif
