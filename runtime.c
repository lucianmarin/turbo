#include "runtime.h"
#include <stdlib.h>
#include <string.h>

TurboObject* turbo_none = NULL;
TurboObject* turbo_true = NULL;
TurboObject* turbo_false = NULL;

TurboObject* t_print = NULL;
TurboObject* t_len = NULL;
TurboObject* t_str = NULL;
TurboObject* t_int = NULL;
TurboObject* t_ord = NULL;
TurboObject* t_chr = NULL;
TurboObject* t_range = NULL;
TurboObject* t_open = NULL;
TurboObject* t_sys_argv = NULL;

static TurboObject* builtin_len(int argc, TurboObject** args) {
    if (argc != 1) {
        fprintf(stderr, "TypeError: len() takes exactly 1 argument\n");
        exit(1);
    }
    return turbo_len(args[0]);
}

static TurboObject* builtin_str(int argc, TurboObject** args) {
    if (argc != 1) {
        fprintf(stderr, "TypeError: str() takes exactly 1 argument\n");
        exit(1);
    }
    return turbo_str(args[0]);
}

static TurboObject* builtin_int(int argc, TurboObject** args) {
    if (argc != 1) {
        fprintf(stderr, "TypeError: int() takes exactly 1 argument\n");
        exit(1);
    }
    return turbo_int(args[0]);
}

static TurboObject* builtin_ord(int argc, TurboObject** args) {
    if (argc != 1) {
        fprintf(stderr, "TypeError: ord() takes exactly 1 argument\n");
        exit(1);
    }
    return turbo_ord(args[0]);
}

static TurboObject* builtin_chr(int argc, TurboObject** args) {
    if (argc != 1) {
        fprintf(stderr, "TypeError: chr() takes exactly 1 argument\n");
        exit(1);
    }
    return turbo_chr(args[0]);
}

static TurboObject* builtin_open(int argc, TurboObject** args) {
    if (argc != 2) {
        fprintf(stderr, "TypeError: open() takes exactly 2 arguments\n");
        exit(1);
    }
    return turbo_open(args[0], args[1]);
}

void turbo_init(void) {
    if (turbo_none != NULL) return; // Already initialized

    turbo_none = (TurboObject*)malloc(sizeof(TurboObject));
    turbo_none->type = TYPE_NONE;

    turbo_true = (TurboObject*)malloc(sizeof(TurboObject));
    turbo_true->type = TYPE_BOOL;
    turbo_true->bool_val = true;

    turbo_false = (TurboObject*)malloc(sizeof(TurboObject));
    turbo_false->type = TYPE_BOOL;
    turbo_false->bool_val = false;

    t_print = make_func(turbo_print, "print");
    t_len = make_func(builtin_len, "len");
    t_str = make_func(builtin_str, "str");
    t_int = make_func(builtin_int, "int");
    t_ord = make_func(builtin_ord, "ord");
    t_chr = make_func(builtin_chr, "chr");
    t_range = make_func(turbo_range, "range");
    t_open = make_func(builtin_open, "open");
}

TurboObject* make_int(long long val) {
    TurboObject* obj = (TurboObject*)malloc(sizeof(TurboObject));
    obj->type = TYPE_INT;
    obj->int_val = val;
    return obj;
}

TurboObject* make_str(const char* val) {
    TurboObject* obj = (TurboObject*)malloc(sizeof(TurboObject));
    obj->type = TYPE_STR;
    obj->str_val.length = strlen(val);
    obj->str_val.chars = (char*)malloc(obj->str_val.length + 1);
    strcpy(obj->str_val.chars, val);
    return obj;
}

TurboObject* make_str_len(const char* val, int len) {
    TurboObject* obj = (TurboObject*)malloc(sizeof(TurboObject));
    obj->type = TYPE_STR;
    obj->str_val.length = len;
    obj->str_val.chars = (char*)malloc(len + 1);
    memcpy(obj->str_val.chars, val, len);
    obj->str_val.chars[len] = '\0';
    return obj;
}

TurboObject* make_bool(bool val) {
    return val ? turbo_true : turbo_false;
}

TurboObject* make_list(void) {
    TurboObject* obj = (TurboObject*)malloc(sizeof(TurboObject));
    obj->type = TYPE_LIST;
    obj->list_val.length = 0;
    obj->list_val.capacity = 8;
    obj->list_val.items = (TurboObject**)malloc(sizeof(TurboObject*) * obj->list_val.capacity);
    return obj;
}

TurboObject* make_dict(void) {
    TurboObject* obj = (TurboObject*)malloc(sizeof(TurboObject));
    obj->type = TYPE_DICT;
    obj->dict_val.length = 0;
    obj->dict_val.capacity = 8;
    obj->dict_val.keys = (TurboObject**)malloc(sizeof(TurboObject*) * obj->dict_val.capacity);
    obj->dict_val.values = (TurboObject**)malloc(sizeof(TurboObject*) * obj->dict_val.capacity);
    return obj;
}

TurboObject* make_func(TurboCFunction func, const char* name) {
    TurboObject* obj = (TurboObject*)malloc(sizeof(TurboObject));
    obj->type = TYPE_FUNC;
    obj->func_val.func_ptr = func;
    obj->func_val.name = strdup(name);
    return obj;
}

TurboObject* make_class(const char* name) {
    TurboObject* obj = (TurboObject*)malloc(sizeof(TurboObject));
    obj->type = TYPE_CLASS;
    obj->class_val.name = strdup(name);
    obj->class_val.method_count = 0;
    obj->class_val.method_capacity = 8;
    obj->class_val.method_names = (char**)malloc(sizeof(char*) * obj->class_val.method_capacity);
    obj->class_val.method_funcs = (TurboCFunction*)malloc(sizeof(TurboCFunction) * obj->class_val.method_capacity);
    return obj;
}

TurboObject* make_instance(TurboObject* class_obj) {
    TurboObject* obj = (TurboObject*)malloc(sizeof(TurboObject));
    obj->type = TYPE_INSTANCE;
    obj->inst_val.class_obj = class_obj;
    obj->inst_val.length = 0;
    obj->inst_val.capacity = 8;
    obj->inst_val.keys = (char**)malloc(sizeof(char*) * obj->inst_val.capacity);
    obj->inst_val.values = (TurboObject**)malloc(sizeof(TurboObject*) * obj->inst_val.capacity);
    return obj;
}

TurboObject* make_file(FILE* handle) {
    TurboObject* obj = (TurboObject*)malloc(sizeof(TurboObject));
    obj->type = TYPE_FILE;
    obj->file_val.handle = handle;
    return obj;
}

// Helper for split
static void add_to_list(TurboObject* list, TurboObject* item) {
    turbo_list_append(list, item);
}

// Operators
TurboObject* turbo_add(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        return make_int(a->int_val + b->int_val);
    }
    if (a->type == TYPE_STR && b->type == TYPE_STR) {
        int new_len = a->str_val.length + b->str_val.length;
        char* new_chars = (char*)malloc(new_len + 1);
        strcpy(new_chars, a->str_val.chars);
        strcat(new_chars, b->str_val.chars);
        TurboObject* res = make_str_len(new_chars, new_len);
        free(new_chars);
        return res;
    }
    if (a->type == TYPE_LIST && b->type == TYPE_LIST) {
        TurboObject* res = make_list();
        for (int i = 0; i < a->list_val.length; i++) {
            add_to_list(res, a->list_val.items[i]);
        }
        for (int i = 0; i < b->list_val.length; i++) {
            add_to_list(res, b->list_val.items[i]);
        }
        return res;
    }
    fprintf(stderr, "TypeError: unsupported operand type(s) for +\n");
    exit(1);
}

TurboObject* turbo_sub(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        return make_int(a->int_val - b->int_val);
    }
    fprintf(stderr, "TypeError: unsupported operand type(s) for -\n");
    exit(1);
}

TurboObject* turbo_mul(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        return make_int(a->int_val * b->int_val);
    }
    if (a->type == TYPE_STR && b->type == TYPE_INT) {
        int count = b->int_val;
        if (count <= 0) return make_str("");
        int new_len = a->str_val.length * count;
        char* new_chars = (char*)malloc(new_len + 1);
        new_chars[0] = '\0';
        for (int i = 0; i < count; i++) {
            strcat(new_chars, a->str_val.chars);
        }
        TurboObject* res = make_str_len(new_chars, new_len);
        free(new_chars);
        return res;
    }
    fprintf(stderr, "TypeError: unsupported operand type(s) for *\n");
    exit(1);
}

TurboObject* turbo_div(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        if (b->int_val == 0) {
            fprintf(stderr, "ZeroDivisionError: division by zero\n");
            exit(1);
        }
        return make_int(a->int_val / b->int_val);
    }
    fprintf(stderr, "TypeError: unsupported operand type(s) for /\n");
    exit(1);
}

TurboObject* turbo_mod(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        if (b->int_val == 0) {
            fprintf(stderr, "ZeroDivisionError: integer division or modulo by zero\n");
            exit(1);
        }
        return make_int(a->int_val % b->int_val);
    }
    fprintf(stderr, "TypeError: unsupported operand type(s) for %%\n");
    exit(1);
}

// Helper eq
static bool is_equal(TurboObject* a, TurboObject* b) {
    if (a == b) return true;
    if (a->type != b->type) return false;
    switch (a->type) {
        case TYPE_NONE: return true;
        case TYPE_INT: return a->int_val == b->int_val;
        case TYPE_BOOL: return a->bool_val == b->bool_val;
        case TYPE_STR: return strcmp(a->str_val.chars, b->str_val.chars) == 0;
        case TYPE_LIST:
            if (a->list_val.length != b->list_val.length) return false;
            for (int i = 0; i < a->list_val.length; i++) {
                if (!is_equal(a->list_val.items[i], b->list_val.items[i])) return false;
            }
            return true;
        default:
            return false;
    }
}

TurboObject* turbo_eq(TurboObject* a, TurboObject* b) {
    return make_bool(is_equal(a, b));
}

TurboObject* turbo_ne(TurboObject* a, TurboObject* b) {
    return make_bool(!is_equal(a, b));
}

TurboObject* turbo_lt(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        return make_bool(a->int_val < b->int_val);
    }
    if (a->type == TYPE_STR && b->type == TYPE_STR) {
        return make_bool(strcmp(a->str_val.chars, b->str_val.chars) < 0);
    }
    fprintf(stderr, "TypeError: '<' not supported between types\n");
    exit(1);
}

TurboObject* turbo_gt(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        return make_bool(a->int_val > b->int_val);
    }
    if (a->type == TYPE_STR && b->type == TYPE_STR) {
        return make_bool(strcmp(a->str_val.chars, b->str_val.chars) > 0);
    }
    fprintf(stderr, "TypeError: '>' not supported between types\n");
    exit(1);
}

TurboObject* turbo_lte(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        return make_bool(a->int_val <= b->int_val);
    }
    if (a->type == TYPE_STR && b->type == TYPE_STR) {
        return make_bool(strcmp(a->str_val.chars, b->str_val.chars) <= 0);
    }
    fprintf(stderr, "TypeError: '<=' not supported between types\n");
    exit(1);
}

TurboObject* turbo_gte(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        return make_bool(a->int_val >= b->int_val);
    }
    if (a->type == TYPE_STR && b->type == TYPE_STR) {
        return make_bool(strcmp(a->str_val.chars, b->str_val.chars) >= 0);
    }
    fprintf(stderr, "TypeError: '>=' not supported between types\n");
    exit(1);
}

TurboObject* turbo_not(TurboObject* val) {
    return make_bool(!turbo_is_truthy(val));
}

TurboObject* turbo_and(TurboObject* a, TurboObject* b) {
    return turbo_is_truthy(a) ? b : a;
}

TurboObject* turbo_or(TurboObject* a, TurboObject* b) {
    return turbo_is_truthy(a) ? a : b;
}

bool turbo_is_truthy(TurboObject* val) {
    if (val == NULL) return false;
    switch (val->type) {
        case TYPE_NONE: return false;
        case TYPE_BOOL: return val->bool_val;
        case TYPE_INT: return val->int_val != 0;
        case TYPE_STR: return val->str_val.length > 0;
        case TYPE_LIST: return val->list_val.length > 0;
        case TYPE_DICT: return val->dict_val.length > 0;
        default: return true;
    }
}

TurboObject* turbo_len(TurboObject* val) {
    if (val->type == TYPE_STR) {
        return make_int(val->str_val.length);
    }
    if (val->type == TYPE_LIST) {
        return make_int(val->list_val.length);
    }
    if (val->type == TYPE_DICT) {
        return make_int(val->dict_val.length);
    }
    fprintf(stderr, "TypeError: object of type has no len()\n");
    exit(1);
}

TurboObject* turbo_getitem(TurboObject* obj, TurboObject* key) {
    if (obj->type == TYPE_LIST) {
        if (key->type != TYPE_INT) {
            fprintf(stderr, "TypeError: list indices must be integers\n");
            exit(1);
        }
        long long idx = key->int_val;
        long long orig_idx = idx;
        if (idx < 0) idx += obj->list_val.length;
        if (idx < 0 || idx >= obj->list_val.length) {
            fprintf(stderr, "IndexError: list index out of range: index %lld (original %lld), length %d\n", idx, orig_idx, obj->list_val.length);
            exit(1);
        }
        return obj->list_val.items[idx];
    }
    if (obj->type == TYPE_STR) {
        if (key->type != TYPE_INT) {
            fprintf(stderr, "TypeError: string indices must be integers\n");
            exit(1);
        }
        long long idx = key->int_val;
        long long orig_idx = idx;
        if (idx < 0) idx += obj->str_val.length;
        if (idx < 0 || idx >= obj->str_val.length) {
            fprintf(stderr, "IndexError: string index out of range: index %lld (original %lld), length %d\n", idx, orig_idx, obj->str_val.length);
            exit(1);
        }
        char temp[2] = { obj->str_val.chars[idx], '\0' };
        return make_str(temp);
    }
    if (obj->type == TYPE_DICT) {
        for (int i = 0; i < obj->dict_val.length; i++) {
            if (is_equal(obj->dict_val.keys[i], key)) {
                return obj->dict_val.values[i];
            }
        }
        // Instead of hard crash, let's print error
        fprintf(stderr, "KeyError: key not found in dict\n");
        exit(1);
    }
    fprintf(stderr, "TypeError: '%d' object is not subscriptable\n", obj->type);
    exit(1);
}

void turbo_setitem(TurboObject* obj, TurboObject* key, TurboObject* val) {
    if (obj->type == TYPE_LIST) {
        if (key->type != TYPE_INT) {
            fprintf(stderr, "TypeError: list indices must be integers\n");
            exit(1);
        }
        long long idx = key->int_val;
        if (idx < 0) idx += obj->list_val.length;
        if (idx < 0 || idx >= obj->list_val.length) {
            fprintf(stderr, "IndexError: list assignment index out of range\n");
            exit(1);
        }
        obj->list_val.items[idx] = val;
        return;
    }
    if (obj->type == TYPE_DICT) {
        // Look up if key exists
        for (int i = 0; i < obj->dict_val.length; i++) {
            if (is_equal(obj->dict_val.keys[i], key)) {
                obj->dict_val.values[i] = val;
                return;
            }
        }
        // Expand if needed
        if (obj->dict_val.length >= obj->dict_val.capacity) {
            obj->dict_val.capacity *= 2;
            obj->dict_val.keys = (TurboObject**)realloc(obj->dict_val.keys, sizeof(TurboObject*) * obj->dict_val.capacity);
            obj->dict_val.values = (TurboObject**)realloc(obj->dict_val.values, sizeof(TurboObject*) * obj->dict_val.capacity);
        }
        obj->dict_val.keys[obj->dict_val.length] = key;
        obj->dict_val.values[obj->dict_val.length] = val;
        obj->dict_val.length++;
        return;
    }
    fprintf(stderr, "TypeError: object does not support item assignment\n");
    exit(1);
}

TurboObject* turbo_slice(TurboObject* obj, TurboObject* start, TurboObject* end) {
    long long len = 0;
    if (obj->type == TYPE_STR) {
        len = obj->str_val.length;
    } else if (obj->type == TYPE_LIST) {
        len = obj->list_val.length;
    } else {
        fprintf(stderr, "TypeError: unsliceable type\n");
        exit(1);
    }

    long long s = (start == turbo_none) ? 0 : start->int_val;
    long long e = (end == turbo_none) ? len : end->int_val;

    if (s < 0) s += len;
    if (s < 0) s = 0;
    if (s > len) s = len;

    if (e < 0) e += len;
    if (e < 0) e = 0;
    if (e > len) e = len;

    if (s >= e) {
        if (obj->type == TYPE_STR) return make_str("");
        return make_list();
    }

    long long slice_len = e - s;
    if (obj->type == TYPE_STR) {
        return make_str_len(obj->str_val.chars + s, slice_len);
    } else {
        TurboObject* new_lst = make_list();
        for (long long i = s; i < e; i++) {
            add_to_list(new_lst, obj->list_val.items[i]);
        }
        return new_lst;
    }
}

TurboObject* turbo_getattr(TurboObject* obj, const char* name) {
    if (obj->type == TYPE_INSTANCE) {
        for (int i = 0; i < obj->inst_val.length; i++) {
            if (strcmp(obj->inst_val.keys[i], name) == 0) {
                return obj->inst_val.values[i];
            }
        }
        // Check class methods
        TurboObject* cls = obj->inst_val.class_obj;
        for (int i = 0; i < cls->class_val.method_count; i++) {
            if (strcmp(cls->class_val.method_names[i], name) == 0) {
                // Return a bound method / wrapper or just the method func wrapper
                // For simplicity, we compile method calls to turbo_call_method,
                // so we don't necessarily need to return a bound method here. But
                // if they access a method directly as a property, we can return the function.
                return make_func(cls->class_val.method_funcs[i], name);
            }
        }
        fprintf(stderr, "AttributeError: '%s' object has no attribute '%s'\n", cls->class_val.name, name);
        exit(1);
    }
    fprintf(stderr, "AttributeError: object has no attributes\n");
    exit(1);
}

void turbo_setattr(TurboObject* obj, const char* name, TurboObject* val) {
    if (obj->type == TYPE_INSTANCE) {
        for (int i = 0; i < obj->inst_val.length; i++) {
            if (strcmp(obj->inst_val.keys[i], name) == 0) {
                obj->inst_val.values[i] = val;
                return;
            }
        }
        if (obj->inst_val.length >= obj->inst_val.capacity) {
            obj->inst_val.capacity *= 2;
            obj->inst_val.keys = (char**)realloc(obj->inst_val.keys, sizeof(char*) * obj->inst_val.capacity);
            obj->inst_val.values = (TurboObject**)realloc(obj->inst_val.values, sizeof(TurboObject*) * obj->inst_val.capacity);
        }
        obj->inst_val.keys[obj->inst_val.length] = strdup(name);
        obj->inst_val.values[obj->inst_val.length] = val;
        obj->inst_val.length++;
        return;
    }
    fprintf(stderr, "AttributeError: cannot set attribute on non-instance\n");
    exit(1);
}

bool turbo_in(TurboObject* item, TurboObject* container) {
    if (container->type == TYPE_LIST) {
        for (int i = 0; i < container->list_val.length; i++) {
            if (is_equal(item, container->list_val.items[i])) {
                return true;
            }
        }
        return false;
    }
    if (container->type == TYPE_DICT) {
        for (int i = 0; i < container->dict_val.length; i++) {
            if (is_equal(item, container->dict_val.keys[i])) {
                return true;
            }
        }
        return false;
    }
    if (container->type == TYPE_STR) {
        if (item->type != TYPE_STR) {
            fprintf(stderr, "TypeError: 'in <string>' requires string as left operand\n");
            exit(1);
        }
        return strstr(container->str_val.chars, item->str_val.chars) != NULL;
    }
    fprintf(stderr, "TypeError: argument of type is not iterable\n");
    exit(1);
}

// Built-ins and representations
TurboObject* turbo_str(TurboObject* val) {
    char buf[128];
    switch (val->type) {
        case TYPE_NONE:
            return make_str("None");
        case TYPE_BOOL:
            return make_str(val->bool_val ? "True" : "False");
        case TYPE_INT:
            sprintf(buf, "%lld", val->int_val);
            return make_str(buf);
        case TYPE_STR:
            return val;
        case TYPE_LIST: {
            // dynamic build
            char* s = strdup("[");
            for (int i = 0; i < val->list_val.length; i++) {
                TurboObject* item_str = turbo_str(val->list_val.items[i]);
                int new_len = strlen(s) + item_str->str_val.length + 3; // comma, space, bracket
                char* s2 = (char*)malloc(new_len);
                strcpy(s2, s);
                strcat(s2, item_str->str_val.chars);
                if (i < val->list_val.length - 1) {
                    strcat(s2, ", ");
                }
                free(s);
                s = s2;
            }
            char* s3 = (char*)malloc(strlen(s) + 2);
            strcpy(s3, s);
            strcat(s3, "]");
            free(s);
            TurboObject* res = make_str(s3);
            free(s3);
            return res;
        }
        default:
            return make_str("<object>");
    }
}

TurboObject* turbo_print(int argc, TurboObject** args) {
    for (int i = 0; i < argc; i++) {
        TurboObject* str_val = turbo_str(args[i]);
        printf("%s", str_val->str_val.chars);
        if (i < argc - 1) {
            printf(" ");
        }
    }
    printf("\n");
    return turbo_none;
}

TurboObject* turbo_int(TurboObject* val) {
    if (val->type == TYPE_INT) return val;
    if (val->type == TYPE_BOOL) return make_int(val->bool_val ? 1 : 0);
    if (val->type == TYPE_STR) {
        long long v = atoll(val->str_val.chars);
        return make_int(v);
    }
    fprintf(stderr, "ValueError: invalid literal for int()\n");
    exit(1);
}

TurboObject* turbo_ord(TurboObject* val) {
    if (val->type != TYPE_STR || val->str_val.length == 0) {
        fprintf(stderr, "TypeError: ord() expected a string of length 1\n");
        exit(1);
    }
    return make_int((unsigned char)val->str_val.chars[0]);
}

TurboObject* turbo_chr(TurboObject* val) {
    if (val->type != TYPE_INT) {
        fprintf(stderr, "TypeError: integer expected\n");
        exit(1);
    }
    char temp[2] = { (char)val->int_val, '\0' };
    return make_str(temp);
}

TurboObject* turbo_range(int argc, TurboObject** args) {
    long long start = 0;
    long long stop = 0;
    if (argc == 1) {
        stop = args[0]->int_val;
    } else if (argc == 2) {
        start = args[0]->int_val;
        stop = args[1]->int_val;
    } else {
        fprintf(stderr, "TypeError: range expected 1 or 2 arguments\n");
        exit(1);
    }
    TurboObject* list = make_list();
    for (long long i = start; i < stop; i++) {
        add_to_list(list, make_int(i));
    }
    return list;
}

TurboObject* turbo_open(TurboObject* path, TurboObject* mode) {
    if (path->type != TYPE_STR || mode->type != TYPE_STR) {
        fprintf(stderr, "TypeError: open() arguments must be strings\n");
        exit(1);
    }
    FILE* f = fopen(path->str_val.chars, mode->str_val.chars);
    if (f == NULL) {
        fprintf(stderr, "FileNotFoundError: [Errno 2] No such file or directory: '%s'\n", path->str_val.chars);
        exit(1);
    }
    return make_file(f);
}

TurboObject* turbo_file_read(TurboObject* file_obj) {
    if (file_obj->type != TYPE_FILE) {
        fprintf(stderr, "TypeError: expected file object\n");
        exit(1);
    }
    FILE* f = file_obj->file_val.handle;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* content = (char*)malloc(sz + 1);
    long read_sz = fread(content, 1, sz, f);
    content[read_sz] = '\0';
    TurboObject* str_val = make_str_len(content, read_sz);
    free(content);
    return str_val;
}

void turbo_file_write(TurboObject* file_obj, TurboObject* text_obj) {
    if (file_obj->type != TYPE_FILE) {
        fprintf(stderr, "TypeError: expected file object\n");
        exit(1);
    }
    if (text_obj->type != TYPE_STR) {
        fprintf(stderr, "TypeError: write() argument must be str\n");
        exit(1);
    }
    fwrite(text_obj->str_val.chars, 1, text_obj->str_val.length, file_obj->file_val.handle);
}

void turbo_file_close(TurboObject* file_obj) {
    if (file_obj->type != TYPE_FILE) {
        fprintf(stderr, "TypeError: expected file object\n");
        exit(1);
    }
    fclose(file_obj->file_val.handle);
    file_obj->file_val.handle = NULL;
}

void turbo_list_append(TurboObject* list_obj, TurboObject* item) {
    if (list_obj->type != TYPE_LIST) {
        fprintf(stderr, "TypeError: expected list object\n");
        exit(1);
    }
    if (list_obj->list_val.length >= list_obj->list_val.capacity) {
        list_obj->list_val.capacity *= 2;
        list_obj->list_val.items = (TurboObject**)realloc(list_obj->list_val.items, sizeof(TurboObject*) * list_obj->list_val.capacity);
    }
    list_obj->list_val.items[list_obj->list_val.length] = item;
    list_obj->list_val.length++;
}

TurboObject* turbo_str_split(int argc, TurboObject** args) {
    if (argc < 1 || args[0]->type != TYPE_STR) {
        fprintf(stderr, "TypeError: split target must be str\n");
        exit(1);
    }
    TurboObject* self_obj = args[0];
    char* str = self_obj->str_val.chars;
    TurboObject* list = make_list();

    char* sep = NULL;
    if (argc >= 2 && args[1] != turbo_none) {
        if (args[1]->type != TYPE_STR) {
            fprintf(stderr, "TypeError: sep must be str or None\n");
            exit(1);
        }
        sep = args[1]->str_val.chars;
    }

    if (sep == NULL || strlen(sep) == 0) {
        // Split by whitespace
        char* str_copy = strdup(str);
        char* token = strtok(str_copy, " \t\r\n");
        while (token != NULL) {
            add_to_list(list, make_str(token));
            token = strtok(NULL, " \t\r\n");
        }
        free(str_copy);
    } else {
        // Split by separator string
        char* str_copy = strdup(str);
        char* start = str_copy;
        char* p;
        int sep_len = strlen(sep);
        while ((p = strstr(start, sep)) != NULL) {
            *p = '\0';
            add_to_list(list, make_str(start));
            start = p + sep_len;
        }
        add_to_list(list, make_str(start));
        free(str_copy);
    }
    return list;
}

void turbo_class_add_method(TurboObject* class_obj, const char* name, TurboCFunction func) {
    if (class_obj->class_val.method_count >= class_obj->class_val.method_capacity) {
        class_obj->class_val.method_capacity *= 2;
        class_obj->class_val.method_names = (char**)realloc(class_obj->class_val.method_names, sizeof(char*) * class_obj->class_val.method_capacity);
        class_obj->class_val.method_funcs = (TurboCFunction*)realloc(class_obj->class_val.method_funcs, sizeof(TurboCFunction) * class_obj->class_val.method_capacity);
    }
    class_obj->class_val.method_names[class_obj->class_val.method_count] = strdup(name);
    class_obj->class_val.method_funcs[class_obj->class_val.method_count] = func;
    class_obj->class_val.method_count++;
}

TurboObject* turbo_call(TurboObject* callable, int argc, TurboObject** args) {
    if (callable->type == TYPE_FUNC) {
        return callable->func_val.func_ptr(argc, args);
    }
    if (callable->type == TYPE_CLASS) {
        // Instantiate
        TurboObject* inst = make_instance(callable);
        // Look up __init__
        for (int i = 0; i < callable->class_val.method_count; i++) {
            if (strcmp(callable->class_val.method_names[i], "__init__") == 0) {
                // Call it, with inst as first argument
                int init_argc = argc + 1;
                TurboObject** init_args = (TurboObject**)malloc(sizeof(TurboObject*) * init_argc);
                init_args[0] = inst;
                for (int j = 0; j < argc; j++) {
                    init_args[j + 1] = args[j];
                }
                callable->class_val.method_funcs[i](init_argc, init_args);
                free(init_args);
                break;
            }
        }
        return inst;
    }
    fprintf(stderr, "TypeError: object is not callable\n");
    exit(1);
}

TurboObject* turbo_call_method(TurboObject* obj, const char* method_name, int argc, TurboObject** args) {
    if (obj->type == TYPE_INSTANCE) {
        TurboObject* cls = obj->inst_val.class_obj;
        for (int i = 0; i < cls->class_val.method_count; i++) {
            if (strcmp(cls->class_val.method_names[i], method_name) == 0) {
                int call_argc = argc + 1;
                TurboObject** call_args = (TurboObject**)malloc(sizeof(TurboObject*) * call_argc);
                call_args[0] = obj;
                for (int j = 0; j < argc; j++) {
                    call_args[j + 1] = args[j];
                }
                TurboObject* res = cls->class_val.method_funcs[i](call_argc, call_args);
                free(call_args);
                return res;
            }
        }
        fprintf(stderr, "AttributeError: '%s' object has no method '%s'\n", cls->class_val.name, method_name);
        exit(1);
    }
    // String and List also have methods
    if (obj->type == TYPE_LIST) {
        if (strcmp(method_name, "append") == 0) {
            if (argc != 1) {
                fprintf(stderr, "TypeError: append() takes exactly one argument\n");
                exit(1);
            }
            turbo_list_append(obj, args[0]);
            return turbo_none;
        }
        if (strcmp(method_name, "pop") == 0) {
            if (argc != 0) {
                fprintf(stderr, "TypeError: pop() takes exactly zero arguments\n");
                exit(1);
            }
            if (obj->list_val.length == 0) {
                fprintf(stderr, "IndexError: pop from empty list\n");
                exit(1);
            }
            obj->list_val.length--;
            return obj->list_val.items[obj->list_val.length];
        }
    }
    if (obj->type == TYPE_STR && strcmp(method_name, "split") == 0) {
        int split_argc = argc + 1;
        TurboObject** split_args = (TurboObject**)malloc(sizeof(TurboObject*) * split_argc);
        split_args[0] = obj;
        for (int j = 0; j < argc; j++) {
            split_args[j + 1] = args[j];
        }
        TurboObject* res = turbo_str_split(split_argc, split_args);
        free(split_args);
        return res;
    }
    if (obj->type == TYPE_FILE) {
        if (strcmp(method_name, "read") == 0) {
            if (argc != 0) {
                fprintf(stderr, "TypeError: read() takes no arguments\n");
                exit(1);
            }
            return turbo_file_read(obj);
        }
        if (strcmp(method_name, "write") == 0) {
            if (argc != 1) {
                fprintf(stderr, "TypeError: write() takes exactly 1 argument\n");
                exit(1);
            }
            turbo_file_write(obj, args[0]);
            return turbo_none;
        }
        if (strcmp(method_name, "close") == 0) {
            if (argc != 0) {
                fprintf(stderr, "TypeError: close() takes no arguments\n");
                exit(1);
            }
            turbo_file_close(obj);
            return turbo_none;
        }
    }
    fprintf(stderr, "AttributeError: object has no method '%s'\n", method_name);
    exit(1);
}
