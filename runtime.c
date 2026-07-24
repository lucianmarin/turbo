#include "runtime.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

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
TurboObject* t_input = NULL;
TurboObject* t_type = NULL;
TurboObject* t_isinstance = NULL;
TurboObject* t_hasattr = NULL;
TurboObject* t_getattr = NULL;
TurboObject* t_setattr = NULL;
TurboObject* t_repr = NULL;
TurboObject* t_abs = NULL;
TurboObject* t_round = NULL;
TurboObject* t_pow = NULL;
TurboObject* t_hex = NULL;
TurboObject* t_bin = NULL;
TurboObject* t_oct = NULL;
TurboObject* t_float = NULL;
TurboObject* t_bool = NULL;
TurboObject* t_list = NULL;
TurboObject* t_dict = NULL;
TurboObject* t_super = NULL;
TurboObject* t_iter = NULL;
TurboObject* t_next = NULL;
TurboObject* t_all = NULL;
TurboObject* t_any = NULL;
TurboObject* t_sum = NULL;
TurboObject* t_min = NULL;
TurboObject* t_max = NULL;
TurboObject* t_sorted = NULL;
TurboObject* t_reversed = NULL;
TurboObject* t_enumerate = NULL;
TurboObject* t_zip = NULL;
TurboObject* t_map = NULL;
TurboObject* t_filter = NULL;

jmp_buf* turbo_exception_jmp = NULL;
TurboObject* turbo_exception_value = NULL;

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

static const char* get_type_name(TurboObject* obj) {
    if (obj == NULL) return "NoneType";
    switch (obj->type) {
        case TYPE_NONE: return "NoneType";
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_BOOL: return "bool";
        case TYPE_STR: return "str";
        case TYPE_BYTES: return "bytes";
        case TYPE_LIST: return "list";
        case TYPE_TUPLE: return "tuple";
        case TYPE_SET: return "set";
        case TYPE_MODULE: return "module";
        case TYPE_DICT: return "dict";
        case TYPE_FUNC: return "function";
        case TYPE_CLASS: return "type";
        case TYPE_INSTANCE: return "instance";
        case TYPE_FILE: return "file";
        default: return "unknown";
    }
}

static TurboObject* builtin_input(int argc, TurboObject** args) {
    if (argc > 0 && args[0] != turbo_none) {
        TurboObject* prompt_str = turbo_str(args[0]);
        printf("%s", prompt_str->str_val.chars);
        fflush(stdout);
    }
    char buf[1024];
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        return make_str("");
    }
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
    return make_str(buf);
}

static TurboObject* builtin_type(int argc, TurboObject** args) {
    if (argc != 1) {
        fprintf(stderr, "TypeError: type() takes exactly 1 argument\n");
        exit(1);
    }
    return make_str(get_type_name(args[0]));
}

static TurboObject* builtin_isinstance(int argc, TurboObject** args) {
    if (argc != 2) {
        fprintf(stderr, "TypeError: isinstance() takes exactly 2 arguments\n");
        exit(1);
    }
    if (args[1]->type != TYPE_STR) {
        fprintf(stderr, "TypeError: isinstance() second argument must be a string\n");
        exit(1);
    }
    const char* type_name = args[1]->str_val.chars;
    const char* actual = get_type_name(args[0]);
    if (strcmp(actual, type_name) == 0) return turbo_true;
    if (args[0]->type == TYPE_INSTANCE) {
        TurboObject* cls = args[0]->inst_val.class_obj;
        if (strcmp(cls->class_val.name, type_name) == 0) return turbo_true;
    }
    return turbo_false;
}

static TurboObject* builtin_hasattr(int argc, TurboObject** args) {
    if (argc != 2) {
        fprintf(stderr, "TypeError: hasattr() takes exactly 2 arguments\n");
        exit(1);
    }
    if (args[1]->type != TYPE_STR) {
        fprintf(stderr, "TypeError: hasattr() second argument must be a string\n");
        exit(1);
    }
    if (args[0]->type == TYPE_INSTANCE) {
        const char* name = args[1]->str_val.chars;
        for (int i = 0; i < args[0]->inst_val.length; i++) {
            if (strcmp(args[0]->inst_val.keys[i], name) == 0) return turbo_true;
        }
        TurboObject* cls = args[0]->inst_val.class_obj;
        for (int i = 0; i < cls->class_val.method_count; i++) {
            if (strcmp(cls->class_val.method_names[i], name) == 0) return turbo_true;
        }
    }
    return turbo_false;
}

static TurboObject* builtin_getattr(int argc, TurboObject** args) {
    if (argc < 2) {
        fprintf(stderr, "TypeError: getattr() takes at least 2 arguments\n");
        exit(1);
    }
    if (args[1]->type != TYPE_STR) {
        fprintf(stderr, "TypeError: getattr() second argument must be a string\n");
        exit(1);
    }
    TurboObject* def = (argc >= 3) ? args[2] : turbo_none;
    const char* name = args[1]->str_val.chars;
    if (args[0]->type == TYPE_INSTANCE) {
        for (int i = 0; i < args[0]->inst_val.length; i++) {
            if (strcmp(args[0]->inst_val.keys[i], name) == 0) {
                return args[0]->inst_val.values[i];
            }
        }
        TurboObject* cls = args[0]->inst_val.class_obj;
        for (int i = 0; i < cls->class_val.method_count; i++) {
            if (strcmp(cls->class_val.method_names[i], name) == 0) {
                return make_func(cls->class_val.method_funcs[i], name);
            }
        }
    }
    return def;
}

static TurboObject* builtin_setattr(int argc, TurboObject** args) {
    if (argc != 3) {
        fprintf(stderr, "TypeError: setattr() takes exactly 3 arguments\n");
        exit(1);
    }
    if (args[1]->type != TYPE_STR) {
        fprintf(stderr, "TypeError: setattr() second argument must be a string\n");
        exit(1);
    }
    turbo_setattr(args[0], args[1]->str_val.chars, args[2]);
    return turbo_none;
}

static TurboObject* builtin_repr(int argc, TurboObject** args) {
    if (argc != 1) {
        fprintf(stderr, "TypeError: repr() takes exactly 1 argument\n");
        exit(1);
    }
    return turbo_repr(args[0]);
}

static TurboObject* builtin_abs(int argc, TurboObject** args) {
    if (argc != 1) {
        fprintf(stderr, "TypeError: abs() takes exactly 1 argument\n");
        exit(1);
    }
    if (args[0]->type == TYPE_INT) {
        TurboObject* a = args[0];
        if (a->int_val.sign == 0) return a;
        TurboObject* res = (TurboObject*)malloc(sizeof(TurboObject));
        res->type = TYPE_INT;
        res->int_val.digits = (char*)malloc(a->int_val.length + 1);
        memcpy(res->int_val.digits, a->int_val.digits, a->int_val.length + 1);
        res->int_val.length = a->int_val.length;
        res->int_val.sign = 0;
        return res;
    }
    if (args[0]->type == TYPE_FLOAT) {
        double v = args[0]->float_val;
        return make_float(v < 0 ? -v : v);
    }
    fprintf(stderr, "TypeError: bad operand type for abs()\n");
    exit(1);
}

static TurboObject* builtin_round(int argc, TurboObject** args) {
    if (argc != 1) {
        fprintf(stderr, "TypeError: round() takes exactly 1 argument\n");
        exit(1);
    }
    if (args[0]->type == TYPE_FLOAT) {
        return make_int_from_ll((long long)round(args[0]->float_val));
    }
    if (args[0]->type == TYPE_INT) {
        return args[0];
    }
    fprintf(stderr, "TypeError: bad operand type for round()\n");
    exit(1);
}

static TurboObject* builtin_pow(int argc, TurboObject** args) {
    if (argc != 2) {
        fprintf(stderr, "TypeError: pow() takes exactly 2 arguments\n");
        exit(1);
    }
    return turbo_pow(args[0], args[1]);
}

// Convert a decimal digit string to an arbitrary base (2, 8, 16). Returns newly allocated string.
// Repeatedly divide digits by base, collecting remainders.
static char* decimal_to_base(const char* dec_digits, int dec_len, int base, int* out_len) {
    // Handle zero
    if (dec_len == 1 && dec_digits[0] == '0') {
        *out_len = 1;
        char* r = (char*)malloc(2);
        r[0] = '0'; r[1] = '\0';
        return r;
    }
    int max_len = dec_len * 4; // upper bound
    char* result = (char*)malloc(max_len + 1);
    int res_len = 0;

    char* cur = (char*)malloc(dec_len + 1);
    memcpy(cur, dec_digits, dec_len + 1);
    int cur_len = dec_len;

    while (!(cur_len == 1 && cur[0] == '0')) {
        int remainder = 0;
        char* next = (char*)malloc(cur_len + 1);
        int next_len = 0;
        for (int i = 0; i < cur_len; i++) {
            remainder = remainder * 10 + (cur[i] - '0');
            int q = remainder / base;
            remainder = remainder % base;
            if (next_len > 0 || q > 0) {
                next[next_len++] = q + '0';
            }
        }
        if (next_len == 0) {
            next[next_len++] = '0';
        }
        next[next_len] = '\0';

        char hex_chars[] = "0123456789abcdef";
        result[res_len++] = hex_chars[remainder];

        free(cur);
        cur = next;
        cur_len = next_len;
    }
    free(cur);

    // Reverse result
    for (int i = 0; i < res_len / 2; i++) {
        char t = result[i];
        result[i] = result[res_len - 1 - i];
        result[res_len - 1 - i] = t;
    }
    result[res_len] = '\0';
    *out_len = res_len;
    return result;
}

static TurboObject* builtin_hex(int argc, TurboObject** args) {
    if (argc != 1 || args[0]->type != TYPE_INT) {
        fprintf(stderr, "TypeError: hex() argument must be an integer\n");
        exit(1);
    }
    TurboObject* obj = args[0];
    if (obj->int_val.length == 1 && obj->int_val.digits[0] == '0') {
        return make_str("0");
    }
    int hex_len;
    char* hex_str = decimal_to_base(obj->int_val.digits, obj->int_val.length, 16, &hex_len);
    char* buf;
    if (obj->int_val.sign) {
        buf = (char*)malloc(hex_len + 2);
        buf[0] = '-';
        memcpy(buf + 1, hex_str, hex_len + 1);
    } else {
        buf = hex_str;
    }
    TurboObject* res = make_str(buf);
    if (obj->int_val.sign) free(buf);
    free(hex_str);
    return res;
}

static TurboObject* builtin_bin(int argc, TurboObject** args) {
    if (argc != 1 || args[0]->type != TYPE_INT) {
        fprintf(stderr, "TypeError: bin() argument must be an integer\n");
        exit(1);
    }
    TurboObject* obj = args[0];
    int bin_len;
    char* bin_str = decimal_to_base(obj->int_val.digits, obj->int_val.length, 2, &bin_len);
    // Output: "0b..." or "-0b..."
    int prefix = 2;
    if (obj->int_val.sign) prefix += 1; // "-0b"
    char* buf = (char*)malloc(bin_len + prefix + 1);
    int pos = 0;
    if (obj->int_val.sign) buf[pos++] = '-';
    buf[pos++] = '0';
    buf[pos++] = 'b';
    memcpy(buf + pos, bin_str, bin_len + 1);
    TurboObject* res = make_str(buf);
    free(buf);
    free(bin_str);
    return res;
}

static TurboObject* builtin_oct(int argc, TurboObject** args) {
    if (argc != 1 || args[0]->type != TYPE_INT) {
        fprintf(stderr, "TypeError: oct() argument must be an integer\n");
        exit(1);
    }
    TurboObject* obj = args[0];
    if (obj->int_val.length == 1 && obj->int_val.digits[0] == '0') {
        return make_str("0");
    }
    int oct_len;
    char* oct_str = decimal_to_base(obj->int_val.digits, obj->int_val.length, 8, &oct_len);
    char* buf;
    if (obj->int_val.sign) {
        buf = (char*)malloc(oct_len + 2);
        buf[0] = '-';
        memcpy(buf + 1, oct_str, oct_len + 1);
    } else {
        buf = oct_str;
    }
    TurboObject* res = make_str(buf);
    if (obj->int_val.sign) free(buf);
    free(oct_str);
    return res;
}

static TurboObject* builtin_float(int argc, TurboObject** args) {
    if (argc != 1) {
        fprintf(stderr, "TypeError: float() takes exactly 1 argument\n");
        exit(1);
    }
    return turbo_float(args[0]);
}

static TurboObject* builtin_bool(int argc, TurboObject** args) {
    if (argc != 1) {
        fprintf(stderr, "TypeError: bool() takes exactly 1 argument\n");
        exit(1);
    }
    return make_bool(turbo_is_truthy(args[0]));
}

static TurboObject* builtin_list(int argc, TurboObject** args) {
    if (argc != 1) {
        fprintf(stderr, "TypeError: list() takes exactly 1 argument\n");
        exit(1);
    }
    TurboObject* obj = args[0];
    if (obj->type == TYPE_LIST) {
        return turbo_list_copy(obj);
    }
    if (obj->type == TYPE_STR) {
        TurboObject* res = make_list();
        for (int i = 0; i < obj->str_val.length; i++) {
            char temp[2] = { obj->str_val.chars[i], '\0' };
            turbo_list_append(res, make_str(temp));
        }
        return res;
    }
    fprintf(stderr, "TypeError: list() argument must be iterable\n");
    exit(1);
}

static TurboObject* builtin_dict(int argc, TurboObject** args) {
    if (argc != 1) {
        fprintf(stderr, "TypeError: dict() takes exactly 1 argument\n");
        exit(1);
    }
    TurboObject* obj = args[0];
    if (obj->type == TYPE_DICT) {
        return turbo_dict_copy(obj);
    }
    fprintf(stderr, "TypeError: dict() argument must be a dict\n");
    exit(1);
}

static TurboObject* builtin_super(int argc, TurboObject** args) {
    (void)argc;
    (void)args;
    return turbo_none;
}

static TurboObject* builtin_iter(int argc, TurboObject** args) {
    if (argc != 1) {
        fprintf(stderr, "TypeError: iter() takes exactly 1 argument\n");
        exit(1);
    }
    TurboObject* obj = args[0];
    if (obj->type == TYPE_LIST || obj->type == TYPE_STR) {
        return turbo_slice(obj, make_int_from_ll(0), turbo_none);
    }
    if (obj->type == TYPE_DICT) {
        return turbo_dict_keys(obj);
    }
    return obj;
}

static TurboObject* builtin_next(int argc, TurboObject** args) {
    (void)argc;
    (void)args;
    return turbo_none;
}

static TurboObject* builtin_all(int argc, TurboObject** args) {
    if (argc != 1 || args[0]->type != TYPE_LIST) {
        fprintf(stderr, "TypeError: all() argument must be a list\n");
        exit(1);
    }
    TurboObject* list = args[0];
    for (int i = 0; i < list->list_val.length; i++) {
        if (!turbo_is_truthy(list->list_val.items[i])) {
            return turbo_false;
        }
    }
    return turbo_true;
}

static TurboObject* builtin_any(int argc, TurboObject** args) {
    if (argc != 1 || args[0]->type != TYPE_LIST) {
        fprintf(stderr, "TypeError: any() argument must be a list\n");
        exit(1);
    }
    TurboObject* list = args[0];
    for (int i = 0; i < list->list_val.length; i++) {
        if (turbo_is_truthy(list->list_val.items[i])) {
            return turbo_true;
        }
    }
    return turbo_false;
}

static TurboObject* builtin_sum(int argc, TurboObject** args) {
    if (argc != 1 || args[0]->type != TYPE_LIST) {
        fprintf(stderr, "TypeError: sum() argument must be a list\n");
        exit(1);
    }
    TurboObject* list = args[0];
    if (list->list_val.length == 0) {
        return make_int_from_ll(0);
    }
    TurboObject* result = list->list_val.items[0];
    for (int i = 1; i < list->list_val.length; i++) {
        result = turbo_add(result, list->list_val.items[i]);
    }
    return result;
}

static TurboObject* builtin_min(int argc, TurboObject** args) {
    if (argc != 1 || args[0]->type != TYPE_LIST) {
        fprintf(stderr, "TypeError: min() argument must be a list\n");
        exit(1);
    }
    TurboObject* list = args[0];
    if (list->list_val.length == 0) {
        fprintf(stderr, "ValueError: min() arg is an empty sequence\n");
        exit(1);
    }
    TurboObject* result = list->list_val.items[0];
    for (int i = 1; i < list->list_val.length; i++) {
        if (turbo_lt(list->list_val.items[i], result) == turbo_true) {
            result = list->list_val.items[i];
        }
    }
    return result;
}

static TurboObject* builtin_max(int argc, TurboObject** args) {
    if (argc != 1 || args[0]->type != TYPE_LIST) {
        fprintf(stderr, "TypeError: max() argument must be a list\n");
        exit(1);
    }
    TurboObject* list = args[0];
    if (list->list_val.length == 0) {
        fprintf(stderr, "ValueError: max() arg is an empty sequence\n");
        exit(1);
    }
    TurboObject* result = list->list_val.items[0];
    for (int i = 1; i < list->list_val.length; i++) {
        if (turbo_gt(list->list_val.items[i], result) == turbo_true) {
            result = list->list_val.items[i];
        }
    }
    return result;
}

static TurboObject* builtin_sorted(int argc, TurboObject** args) {
    if (argc != 1 || args[0]->type != TYPE_LIST) {
        fprintf(stderr, "TypeError: sorted() argument must be a list\n");
        exit(1);
    }
    TurboObject* list = args[0];
    TurboObject* res = turbo_list_copy(list);
    turbo_list_sort(res);
    return res;
}

static TurboObject* builtin_reversed(int argc, TurboObject** args) {
    if (argc != 1 || args[0]->type != TYPE_LIST) {
        fprintf(stderr, "TypeError: reversed() argument must be a list\n");
        exit(1);
    }
    TurboObject* list = args[0];
    TurboObject* res = turbo_list_copy(list);
    turbo_list_reverse(res);
    return res;
}

static TurboObject* builtin_enumerate(int argc, TurboObject** args) {
    if (argc != 1 || args[0]->type != TYPE_LIST) {
        fprintf(stderr, "TypeError: enumerate() argument must be a list\n");
        exit(1);
    }
    TurboObject* list = args[0];
    TurboObject* res = make_list();
    for (int i = 0; i < list->list_val.length; i++) {
        TurboObject* pair = make_list();
        turbo_list_append(pair, make_int_from_ll(i));
        turbo_list_append(pair, list->list_val.items[i]);
        turbo_list_append(res, pair);
    }
    return res;
}

static TurboObject* builtin_zip(int argc, TurboObject** args) {
    if (argc != 2 || args[0]->type != TYPE_LIST || args[1]->type != TYPE_LIST) {
        fprintf(stderr, "TypeError: zip() takes exactly 2 list arguments\n");
        exit(1);
    }
    TurboObject* list1 = args[0];
    TurboObject* list2 = args[1];
    int min_len = list1->list_val.length < list2->list_val.length ? list1->list_val.length : list2->list_val.length;
    TurboObject* res = make_list();
    for (int i = 0; i < min_len; i++) {
        TurboObject* pair = make_list();
        turbo_list_append(pair, list1->list_val.items[i]);
        turbo_list_append(pair, list2->list_val.items[i]);
        turbo_list_append(res, pair);
    }
    return res;
}

static TurboObject* builtin_map(int argc, TurboObject** args) {
    if (argc != 2 || args[0]->type != TYPE_FUNC || args[1]->type != TYPE_LIST) {
        fprintf(stderr, "TypeError: map() requires a function and a list\n");
        exit(1);
    }
    TurboObject* func = args[0];
    TurboObject* list = args[1];
    TurboObject* res = make_list();
    for (int i = 0; i < list->list_val.length; i++) {
        TurboObject* item = turbo_call(func, 1, &list->list_val.items[i]);
        turbo_list_append(res, item);
    }
    return res;
}

static TurboObject* builtin_filter(int argc, TurboObject** args) {
    if (argc != 2 || args[0]->type != TYPE_FUNC || args[1]->type != TYPE_LIST) {
        fprintf(stderr, "TypeError: filter() requires a function and a list\n");
        exit(1);
    }
    TurboObject* func = args[0];
    TurboObject* list = args[1];
    TurboObject* res = make_list();
    for (int i = 0; i < list->list_val.length; i++) {
        TurboObject* item = turbo_call(func, 1, &list->list_val.items[i]);
        if (turbo_is_truthy(item)) {
            turbo_list_append(res, list->list_val.items[i]);
        }
    }
    return res;
}

void turbo_raise(TurboObject* exc) {
    turbo_exception_value = exc;
    if (turbo_exception_jmp != NULL) {
        longjmp(*turbo_exception_jmp, 1);
    }
    fprintf(stderr, "Unhandled exception: ");
    TurboObject* s = turbo_str(exc);
    fprintf(stderr, "%s\n", s->str_val.chars);
    exit(1);
}

bool turbo_exception_matches(TurboObject* exc, TurboObject* exc_type) {
    if (exc_type == turbo_none) return true;
    if (exc->type != TYPE_INSTANCE) return false;
    const char* type_name;
    if (exc_type->type == TYPE_CLASS) {
        type_name = exc_type->class_val.name;
    } else if (exc_type->type == TYPE_STR) {
        type_name = exc_type->str_val.chars;
    } else {
        return false;
    }
    return strcmp(exc->inst_val.class_obj->class_val.name, type_name) == 0;
}

void turbo_init(void) {
    if (turbo_none != NULL) return;

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
    t_input = make_func(builtin_input, "input");
    t_type = make_func(builtin_type, "type");
    t_isinstance = make_func(builtin_isinstance, "isinstance");
    t_hasattr = make_func(builtin_hasattr, "hasattr");
    t_getattr = make_func(builtin_getattr, "getattr");
    t_setattr = make_func(builtin_setattr, "setattr");
    t_repr = make_func(builtin_repr, "repr");
    t_abs = make_func(builtin_abs, "abs");
    t_round = make_func(builtin_round, "round");
    t_pow = make_func(builtin_pow, "pow");
    t_hex = make_func(builtin_hex, "hex");
    t_bin = make_func(builtin_bin, "bin");
    t_oct = make_func(builtin_oct, "oct");
    t_float = make_func(builtin_float, "float");
    t_bool = make_func(builtin_bool, "bool");
    t_list = make_func(builtin_list, "list");
    t_dict = make_func(builtin_dict, "dict");
    t_super = make_func(builtin_super, "super");
    t_iter = make_func(builtin_iter, "iter");
    t_next = make_func(builtin_next, "next");
    t_all = make_func(builtin_all, "all");
    t_any = make_func(builtin_any, "any");
    t_sum = make_func(builtin_sum, "sum");
    t_min = make_func(builtin_min, "min");
    t_max = make_func(builtin_max, "max");
    t_sorted = make_func(builtin_sorted, "sorted");
    t_reversed = make_func(builtin_reversed, "reversed");
    t_enumerate = make_func(builtin_enumerate, "enumerate");
    t_zip = make_func(builtin_zip, "zip");
    t_map = make_func(builtin_map, "map");
    t_filter = make_func(builtin_filter, "filter");
}

// --- Bignum string arithmetic helpers ---

// Strip leading zeros from a digit string (mutates in place)
static void strip_leading_zeros(char* s, int* len) {
    int start = 0;
    while (start < *len - 1 && s[start] == '0') start++;
    if (start > 0) {
        *len -= start;
        memmove(s, s + start, *len);
        s[*len] = '\0';
    }
}

// Compare absolute values of two decimal strings (a_len, b_len are lengths)
// Returns >0 if a > b, 0 if ==, <0 if a < b
static int abs_cmp(const char* a, int a_len, const char* b, int b_len) {
    if (a_len != b_len) return a_len - b_len;
    for (int i = 0; i < a_len; i++) {
        if (a[i] != b[i]) return (int)(a[i]) - (int)(b[i]);
    }
    return 0;
}

// Add two positive decimal strings, return newly allocated result
static char* str_add(const char* a, int a_len, const char* b, int b_len, int* out_len) {
    int max_len = (a_len > b_len ? a_len : b_len) + 1;
    char* rev = (char*)malloc(max_len + 1);
    int carry = 0, k = 0;
    int i = a_len - 1, j = b_len - 1;
    while (i >= 0 || j >= 0 || carry) {
        int d = carry;
        if (i >= 0) d += a[i--] - '0';
        if (j >= 0) d += b[j--] - '0';
        rev[k++] = (d % 10) + '0';
        carry = d / 10;
    }
    char* res = (char*)malloc(k + 1);
    for (int p = 0; p < k; p++) res[p] = rev[k - 1 - p];
    res[k] = '\0';
    free(rev);
    *out_len = k;
    return res;
}

// Subtract b from a where abs(a) >= abs(b) and both are positive. Returns a - b.
static char* str_sub(const char* a, int a_len, const char* b, int b_len, int* out_len) {
    char* rev = (char*)malloc(a_len + 1);
    int borrow = 0, k = 0;
    int i = a_len - 1, j = b_len - 1;
    while (i >= 0 || j >= 0) {
        int d = borrow;
        if (i >= 0) d += a[i--] - '0';
        if (j >= 0) d -= b[j--] - '0';
        if (d < 0) { d += 10; borrow = -1; }
        else borrow = 0;
        rev[k++] = d + '0';
    }
    while (k > 1 && rev[k - 1] == '0') k--;
    char* res = (char*)malloc(k + 1);
    for (int p = 0; p < k; p++) res[p] = rev[k - 1 - p];
    res[k] = '\0';
    free(rev);
    *out_len = k;
    return res;
}

// Multiply two positive decimal strings, return newly allocated result
static char* str_mul(const char* a, int a_len, const char* b, int b_len, int* out_len) {
    int rlen = a_len + b_len;
    int* tmp = (int*)calloc(rlen, sizeof(int));
    for (int i = a_len - 1; i >= 0; i--) {
        for (int j = b_len - 1; j >= 0; j--) {
            int p = (a[i] - '0') * (b[j] - '0');
            int idx = (a_len - 1 - i) + (b_len - 1 - j);
            tmp[idx] += p;
        }
    }
    int carry = 0;
    for (int i = 0; i < rlen; i++) {
        int t = tmp[i] + carry;
        tmp[i] = t % 10;
        carry = t / 10;
    }
    while (carry > 0) {
        tmp[rlen++] = carry % 10;
        carry /= 10;
    }
    while (rlen > 1 && tmp[rlen - 1] == 0) rlen--;
    char* res = (char*)malloc(rlen + 1);
    for (int i = 0; i < rlen; i++) res[i] = tmp[rlen - 1 - i] + '0';
    res[rlen] = '\0';
    free(tmp);
    *out_len = rlen;
    return res;
}

// Divide a by b (positive), return quotient. Sets *out_len
static char* str_div(const char* a, int a_len, const char* b, int b_len, int* out_len) {
    if (b_len == 1 && b[0] == '0') {
        fprintf(stderr, "ZeroDivisionError: division by zero\n");
        exit(1);
    }
    // If a < b, quotient is 0
    if (abs_cmp(a, a_len, b, b_len) < 0) {
        *out_len = 1;
        char* r = (char*)malloc(2);
        r[0] = '0'; r[1] = '\0';
        *out_len = 1;
        return r;
    }
    int max_res = a_len - b_len + 1;
    char* res = (char*)calloc(max_res + 1, 1);
    int res_i = 0;
    char* cur = (char*)malloc(a_len + 1);
    int cur_len = 0;
    for (int i = 0; i < a_len; i++) {
        cur[cur_len++] = a[i];
        cur[cur_len] = '\0';
        strip_leading_zeros(cur, &cur_len);
        int cmp = abs_cmp(cur, cur_len, b, b_len);
        if (cmp < 0) {
            if (res_i > 0) res[res_i++] = '0';
            continue;
        }
        int q = 0;
        while (cmp >= 0) {
            char* ncur = str_sub(cur, cur_len, b, b_len, &cur_len);
            free(cur);
            cur = ncur;
            q++;
            cmp = abs_cmp(cur, cur_len, b, b_len);
        }
        res[res_i++] = q + '0';
    }
    free(cur);
    if (res_i == 0) res[res_i++] = '0';
    res[res_i] = '\0';
    *out_len = res_i;
    char* final_res = (char*)malloc(res_i + 1);
    memcpy(final_res, res, res_i + 1);
    free(res);
    return final_res;
}

// Modulo a % b (positive), return remainder
static char* str_mod(const char* a, int a_len, const char* b, int b_len, int* out_len) {
    if (b_len == 1 && b[0] == '0') {
        fprintf(stderr, "ZeroDivisionError: modulo by zero\n");
        exit(1);
    }
    if (abs_cmp(a, a_len, b, b_len) < 0) {
        char* r = (char*)malloc(a_len + 1);
        memcpy(r, a, a_len + 1);
        *out_len = a_len;
        return r;
    }
    char* cur = (char*)malloc(a_len + 1);
    int cur_len = 0;
    for (int i = 0; i < a_len; i++) {
        cur[cur_len++] = a[i];
        cur[cur_len] = '\0';
        strip_leading_zeros(cur, &cur_len);
        int cmp = abs_cmp(cur, cur_len, b, b_len);
        if (cmp < 0) continue;
        while (cmp >= 0) {
            char* ncur = str_sub(cur, cur_len, b, b_len, &cur_len);
            free(cur);
            cur = ncur;
            cmp = abs_cmp(cur, cur_len, b, b_len);
        }
    }
    // cur is the remainder
    if (cur_len == 0 || (cur_len == 1 && cur[0] == '0')) {
        free(cur);
        *out_len = 1;
        char* r = (char*)malloc(2);
        r[0] = '0'; r[1] = '\0';
        return r;
    }
    char* res = (char*)malloc(cur_len + 1);
    memcpy(res, cur, cur_len + 1);
    free(cur);
    *out_len = cur_len;
    return res;
}

TurboObject* make_int(const char* str) {
    TurboObject* obj = (TurboObject*)malloc(sizeof(TurboObject));
    obj->type = TYPE_INT;
    int sign = 0;
    const char* p = str;
    if (*p == '-') { sign = 1; p++; }
    else if (*p == '+') { p++; }
    int len = strlen(p);
    char* digits = (char*)malloc(len + 1);
    memcpy(digits, p, len + 1);
    strip_leading_zeros(digits, &len);
    obj->int_val.digits = digits;
    obj->int_val.length = len;
    obj->int_val.sign = sign;
    return obj;
}

TurboObject* make_int_from_ll(long long val) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%lld", val);
    return make_int(buf);
}

TurboObject* make_float(double val) {
    TurboObject* obj = (TurboObject*)malloc(sizeof(TurboObject));
    obj->type = TYPE_FLOAT;
    obj->float_val = val;
    return obj;
}

TurboObject* make_complex(double real, double imag) {
    TurboObject* obj = (TurboObject*)malloc(sizeof(TurboObject));
    obj->type = TYPE_COMPLEX;
    obj->complex_val.real = real;
    obj->complex_val.imag = imag;
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

TurboObject* make_bytes(const unsigned char* data, int length) {
    TurboObject* obj = (TurboObject*)malloc(sizeof(TurboObject));
    obj->type = TYPE_BYTES;
    obj->bytes_val.length = length;
    obj->bytes_val.data = (unsigned char*)malloc(length);
    memcpy(obj->bytes_val.data, data, length);
    return obj;
}

TurboObject* make_bytes_from_str(const char* str) {
    TurboObject* obj = (TurboObject*)malloc(sizeof(TurboObject));
    obj->type = TYPE_BYTES;
    obj->bytes_val.length = strlen(str);
    obj->bytes_val.data = (unsigned char*)malloc(obj->bytes_val.length);
    memcpy(obj->bytes_val.data, str, obj->bytes_val.length);
    return obj;
}

TurboObject* make_list(void) {
    TurboObject* obj = (TurboObject*)malloc(sizeof(TurboObject));
    obj->type = TYPE_LIST;
    obj->list_val.length = 0;
    obj->list_val.capacity = 8;
    obj->list_val.items = (TurboObject**)malloc(sizeof(TurboObject*) * obj->list_val.capacity);
    return obj;
}

TurboObject* make_tuple(void) {
    TurboObject* obj = (TurboObject*)malloc(sizeof(TurboObject));
    obj->type = TYPE_TUPLE;
    obj->tuple_val.length = 0;
    obj->tuple_val.capacity = 8;
    obj->tuple_val.items = (TurboObject**)malloc(sizeof(TurboObject*) * obj->tuple_val.capacity);
    return obj;
}

TurboObject* make_tuple_from_list(TurboObject* list) {
    TurboObject* obj = (TurboObject*)malloc(sizeof(TurboObject));
    obj->type = TYPE_TUPLE;
    obj->tuple_val.length = list->list_val.length;
    obj->tuple_val.capacity = list->list_val.length;
    obj->tuple_val.items = (TurboObject**)malloc(sizeof(TurboObject*) * obj->tuple_val.capacity);
    for (int i = 0; i < list->list_val.length; i++) {
        obj->tuple_val.items[i] = list->list_val.items[i];
    }
    return obj;
}

TurboObject* make_set(void) {
    TurboObject* obj = (TurboObject*)malloc(sizeof(TurboObject));
    obj->type = TYPE_SET;
    obj->set_val.length = 0;
    obj->set_val.capacity = 8;
    obj->set_val.items = (TurboObject**)malloc(sizeof(TurboObject*) * obj->set_val.capacity);
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

static void add_to_list(TurboObject* list, TurboObject* item) {
    turbo_list_append(list, item);
}

static double to_double(TurboObject* obj) {
    if (obj->type == TYPE_INT) return (double)int_to_ll(obj);
    if (obj->type == TYPE_FLOAT) return obj->float_val;
    return 0.0;
}

// Convert bignum to long long (for indexing, etc.)
long long int_to_ll(TurboObject* obj) {
    long long val = 0;
    for (int i = 0; i < obj->int_val.length; i++) {
        val = val * 10 + (obj->int_val.digits[i] - '0');
    }
    if (obj->int_val.sign) val = -val;
    return val;
}

// Create an int object from raw digit string (takes ownership of the string)
static TurboObject* make_int_from_str(char* digits, int len, int sign) {
    TurboObject* obj = (TurboObject*)malloc(sizeof(TurboObject));
    obj->type = TYPE_INT;
    obj->int_val.digits = digits;
    obj->int_val.length = len;
    obj->int_val.sign = sign;
    return obj;
}

TurboObject* turbo_add(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        int sa = a->int_val.sign, sb = b->int_val.sign;
        int cmp = abs_cmp(a->int_val.digits, a->int_val.length, b->int_val.digits, b->int_val.length);
        if (sa == sb) {
            // Same sign: add magnitudes
            int rlen;
            char* r = str_add(a->int_val.digits, a->int_val.length, b->int_val.digits, b->int_val.length, &rlen);
            return make_int_from_str(r, rlen, sa);
        } else {
            // Different signs: subtract smaller from larger
            const char* big_d = (cmp >= 0) ? a->int_val.digits : b->int_val.digits;
            int big_len = (cmp >= 0) ? a->int_val.length : b->int_val.length;
            const char* small_d = (cmp >= 0) ? b->int_val.digits : a->int_val.digits;
            int small_len = (cmp >= 0) ? b->int_val.length : a->int_val.length;
            int big_sign = (cmp >= 0) ? sa : sb;
            int rlen;
            char* r = str_sub(big_d, big_len, small_d, small_len, &rlen);
            int res_sign = (rlen == 1 && r[0] == '0') ? 0 : big_sign;
            return make_int_from_str(r, rlen, res_sign);
        }
    }
    if ((a->type == TYPE_FLOAT || a->type == TYPE_INT) &&
        (b->type == TYPE_FLOAT || b->type == TYPE_INT)) {
        if (a->type == TYPE_FLOAT || b->type == TYPE_FLOAT) {
            return make_float(to_double(a) + to_double(b));
        }
    }
    if (a->type == TYPE_COMPLEX && b->type == TYPE_COMPLEX) {
        return make_complex(
            a->complex_val.real + b->complex_val.real,
            a->complex_val.imag + b->complex_val.imag
        );
    }
    if (a->type == TYPE_COMPLEX && (b->type == TYPE_INT || b->type == TYPE_FLOAT)) {
        return make_complex(a->complex_val.real + to_double(b), a->complex_val.imag);
    }
    if ((a->type == TYPE_INT || a->type == TYPE_FLOAT) && b->type == TYPE_COMPLEX) {
        return make_complex(to_double(a) + b->complex_val.real, b->complex_val.imag);
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
    if (a->type == TYPE_TUPLE && b->type == TYPE_TUPLE) {
        TurboObject* res = make_tuple();
        for (int i = 0; i < a->tuple_val.length; i++) {
            turbo_tuple_append(res, a->tuple_val.items[i]);
        }
        for (int i = 0; i < b->tuple_val.length; i++) {
            turbo_tuple_append(res, b->tuple_val.items[i]);
        }
        return res;
    }
    fprintf(stderr, "TypeError: unsupported operand type(s) for +\n");
    exit(1);
}

TurboObject* turbo_sub(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        int sa = a->int_val.sign, sb = b->int_val.sign;
        int cmp = abs_cmp(a->int_val.digits, a->int_val.length, b->int_val.digits, b->int_val.length);
        if (sa != sb) {
            // Different signs: add magnitudes, sign from a
            int rlen;
            char* r = str_add(a->int_val.digits, a->int_val.length, b->int_val.digits, b->int_val.length, &rlen);
            return make_int_from_str(r, rlen, sa);
        } else {
            // Same sign: result = sign * (abs(a) - abs(b))
            int rlen;
            char* r;
            int res_sign;
            if (cmp >= 0) {
                r = str_sub(a->int_val.digits, a->int_val.length, b->int_val.digits, b->int_val.length, &rlen);
                res_sign = (rlen == 1 && r[0] == '0') ? 0 : sa;
            } else {
                r = str_sub(b->int_val.digits, b->int_val.length, a->int_val.digits, a->int_val.length, &rlen);
                res_sign = (rlen == 1 && r[0] == '0') ? 0 : (1 - sa);
            }
            return make_int_from_str(r, rlen, res_sign);
        }
    }
    if ((a->type == TYPE_FLOAT || a->type == TYPE_INT) &&
        (b->type == TYPE_FLOAT || b->type == TYPE_INT)) {
        if (a->type == TYPE_FLOAT || b->type == TYPE_FLOAT) {
            return make_float(to_double(a) - to_double(b));
        }
    }
    if (a->type == TYPE_COMPLEX && b->type == TYPE_COMPLEX) {
        return make_complex(
            a->complex_val.real - b->complex_val.real,
            a->complex_val.imag - b->complex_val.imag
        );
    }
    if (a->type == TYPE_COMPLEX && (b->type == TYPE_INT || b->type == TYPE_FLOAT)) {
        return make_complex(a->complex_val.real - to_double(b), a->complex_val.imag);
    }
    if ((a->type == TYPE_INT || a->type == TYPE_FLOAT) && b->type == TYPE_COMPLEX) {
        return make_complex(to_double(a) - b->complex_val.real, -b->complex_val.imag);
    }
    fprintf(stderr, "TypeError: unsupported operand type(s) for -\n");
    exit(1);
}

TurboObject* turbo_mul(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        int rlen;
        char* r = str_mul(a->int_val.digits, a->int_val.length, b->int_val.digits, b->int_val.length, &rlen);
        int sign = (a->int_val.sign != b->int_val.sign) ? 1 : 0;
        if (rlen == 1 && r[0] == '0') sign = 0;
        return make_int_from_str(r, rlen, sign);
    }
    if ((a->type == TYPE_FLOAT || a->type == TYPE_INT) &&
        (b->type == TYPE_FLOAT || b->type == TYPE_INT)) {
        if (a->type == TYPE_FLOAT || b->type == TYPE_FLOAT) {
            return make_float(to_double(a) * to_double(b));
        }
    }
    if (a->type == TYPE_COMPLEX && b->type == TYPE_COMPLEX) {
        double ar = a->complex_val.real, ai = a->complex_val.imag;
        double br = b->complex_val.real, bi = b->complex_val.imag;
        return make_complex(ar * br - ai * bi, ar * bi + ai * br);
    }
    if (a->type == TYPE_COMPLEX && (b->type == TYPE_INT || b->type == TYPE_FLOAT)) {
        double d = to_double(b);
        return make_complex(a->complex_val.real * d, a->complex_val.imag * d);
    }
    if ((a->type == TYPE_INT || a->type == TYPE_FLOAT) && b->type == TYPE_COMPLEX) {
        double d = to_double(a);
        return make_complex(d * b->complex_val.real, d * b->complex_val.imag);
    }
    if (a->type == TYPE_STR && b->type == TYPE_INT) {
        int count = (int)int_to_ll(b);
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
        // Python 3: int / int returns float
        double da = to_double(a), db = to_double(b);
        if (db == 0.0) {
            fprintf(stderr, "ZeroDivisionError: division by zero\n");
            exit(1);
        }
        return make_float(da / db);
    }
    if ((a->type == TYPE_FLOAT || a->type == TYPE_INT) &&
        (b->type == TYPE_FLOAT || b->type == TYPE_INT)) {
        double db = to_double(b);
        if (db == 0.0) {
            fprintf(stderr, "ZeroDivisionError: division by zero\n");
            exit(1);
        }
        return make_float(to_double(a) / db);
    }
    fprintf(stderr, "TypeError: unsupported operand type(s) for /\n");
    exit(1);
}

TurboObject* turbo_mod(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        if (b->int_val.length == 1 && b->int_val.digits[0] == '0') {
            fprintf(stderr, "ZeroDivisionError: integer division or modulo by zero\n");
            exit(1);
        }
        int rlen;
        char* r = str_mod(a->int_val.digits, a->int_val.length, b->int_val.digits, b->int_val.length, &rlen);
        int sign = a->int_val.sign;
        if (rlen == 1 && r[0] == '0') sign = 0;
        return make_int_from_str(r, rlen, sign);
    }
    fprintf(stderr, "TypeError: unsupported operand type(s) for %%\n");
    exit(1);
}

// Multiply two positive numbers represented as digit strings, result stored in-place
// Helper for pow: multiply a (as string) by itself exp times
static char* str_pow(const char* base, int base_len, const char* exp, int exp_len, int* out_len) {
    // Check for zero exponent
    if (exp_len == 1 && exp[0] == '0') {
        *out_len = 1;
        char* r = (char*)malloc(2);
        r[0] = '1'; r[1] = '\0';
        return r;
    }
    // Start with result = 1
    char* result = (char*)malloc(2);
    result[0] = '1'; result[1] = '\0';
    int res_len = 1;
    
    // Repeated multiplication: exponentiate by squaring would be better,
    // but repeated multiplication is simpler for a bootstrap compiler.
    // Convert exponent to long long (limited but practical)
    long long exp_val = 0;
    for (int i = 0; i < exp_len; i++) {
        exp_val = exp_val * 10 + (exp[i] - '0');
    }
    for (long long i = 0; i < exp_val; i++) {
        char* new_res = str_mul(result, res_len, base, base_len, &res_len);
        free(result);
        result = new_res;
    }
    *out_len = res_len;
    return result;
}

TurboObject* turbo_pow(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        // Check for negative exponent
        if (b->int_val.sign) {
            // Negative exponent: return float
            double da = to_double(a), db = to_double(b);
            return make_float(pow(da, db));
        }
        int rlen;
        char* r = str_pow(a->int_val.digits, a->int_val.length, b->int_val.digits, b->int_val.length, &rlen);
        int sign = (a->int_val.sign && b->int_val.length == 1 && b->int_val.digits[0] == '0') ? 0 : a->int_val.sign;
        // Odd exponent preserves sign; even exponent gives positive
        if (a->int_val.sign) {
            // Check if exponent is odd
            int last_digit = b->int_val.digits[b->int_val.length - 1] - '0';
            if (last_digit % 2 == 0) sign = 0;
        }
        if (rlen == 1 && r[0] == '0') sign = 0;
        return make_int_from_str(r, rlen, sign);
    }
    if ((a->type == TYPE_FLOAT || a->type == TYPE_INT) &&
        (b->type == TYPE_FLOAT || b->type == TYPE_INT)) {
        return make_float(pow(to_double(a), to_double(b)));
    }
    fprintf(stderr, "TypeError: unsupported operand type(s) for **\n");
    exit(1);
}

TurboObject* turbo_floordiv(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        if (b->int_val.length == 1 && b->int_val.digits[0] == '0') {
            fprintf(stderr, "ZeroDivisionError: integer division or modulo by zero\n");
            exit(1);
        }
        int rlen;
        char* r = str_div(a->int_val.digits, a->int_val.length, b->int_val.digits, b->int_val.length, &rlen);
        int sign = (a->int_val.sign != b->int_val.sign) ? 1 : 0;
        if (rlen == 1 && r[0] == '0') sign = 0;
        return make_int_from_str(r, rlen, sign);
    }
    if ((a->type == TYPE_FLOAT || a->type == TYPE_INT) &&
        (b->type == TYPE_FLOAT || b->type == TYPE_INT)) {
        double da = to_double(a);
        double db = to_double(b);
        if (db == 0.0) {
            fprintf(stderr, "ZeroDivisionError: float floor division by zero\n");
            exit(1);
        }
        return make_float(floor(da / db));
    }
    fprintf(stderr, "TypeError: unsupported operand type(s) for //\n");
    exit(1);
}

TurboObject* turbo_bitand(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        return make_int_from_ll(int_to_ll(a) & int_to_ll(b));
    }
    fprintf(stderr, "TypeError: unsupported operand type(s) for &\n");
    exit(1);
}

TurboObject* turbo_bitor(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        return make_int_from_ll(int_to_ll(a) | int_to_ll(b));
    }
    fprintf(stderr, "TypeError: unsupported operand type(s) for |\n");
    exit(1);
}

TurboObject* turbo_bitxor(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        return make_int_from_ll(int_to_ll(a) ^ int_to_ll(b));
    }
    fprintf(stderr, "TypeError: unsupported operand type(s) for ^\n");
    exit(1);
}

TurboObject* turbo_lshift(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        return make_int_from_ll(int_to_ll(a) << int_to_ll(b));
    }
    fprintf(stderr, "TypeError: unsupported operand type(s) for <<\n");
    exit(1);
}

TurboObject* turbo_rshift(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        return make_int_from_ll(int_to_ll(a) >> int_to_ll(b));
    }
    fprintf(stderr, "TypeError: unsupported operand type(s) for >>\n");
    exit(1);
}

TurboObject* turbo_matmul(TurboObject* a, TurboObject* b) {
    if (a->type != TYPE_LIST || b->type != TYPE_LIST) {
        fprintf(stderr, "TypeError: matrix multiplication (@) requires both arguments to be lists\n");
        exit(1);
    }
    if (a->list_val.length == 0 || b->list_val.length == 0) {
        fprintf(stderr, "ValueError: cannot multiply empty matrices\n");
        exit(1);
    }
    TurboObject* first_row = a->list_val.items[0];
    if (first_row->type != TYPE_LIST) {
        fprintf(stderr, "TypeError: matrix multiplication (@) requires list of lists\n");
        exit(1);
    }
    int m = a->list_val.length;
    int n = first_row->list_val.length;
    for (int i = 1; i < m; i++) {
        if (a->list_val.items[i]->type != TYPE_LIST ||
            a->list_val.items[i]->list_val.length != n) {
            fprintf(stderr, "ValueError: matrix A has inconsistent row lengths\n");
            exit(1);
        }
    }
    for (int i = 0; i < b->list_val.length; i++) {
        if (b->list_val.items[i]->type != TYPE_LIST) {
            fprintf(stderr, "TypeError: matrix multiplication (@) requires list of lists\n");
            exit(1);
        }
    }
    int p = b->list_val.items[0]->list_val.length;
    for (int i = 1; i < b->list_val.length; i++) {
        if (b->list_val.items[i]->list_val.length != p) {
            fprintf(stderr, "ValueError: matrix B has inconsistent row lengths\n");
            exit(1);
        }
    }
    if (n != b->list_val.length) {
        fprintf(stderr, "ValueError: incompatible dimensions for matrix multiplication: A(%d,%d) and B(%d,%d)\n", m, n, b->list_val.length, p);
        exit(1);
    }
    TurboObject* result = make_list();
    for (int i = 0; i < m; i++) {
        TurboObject* row = make_list();
        for (int j = 0; j < p; j++) {
            TurboObject* sum = NULL;
            for (int k = 0; k < n; k++) {
                TurboObject* a_ik = turbo_getitem(turbo_getitem(a, make_int_from_ll(i)), make_int_from_ll(k));
                TurboObject* b_kj = turbo_getitem(turbo_getitem(b, make_int_from_ll(k)), make_int_from_ll(j));
                TurboObject* prod = turbo_mul(a_ik, b_kj);
                if (k == 0) {
                    sum = prod;
                } else {
                    TurboObject* new_sum = turbo_add(sum, prod);
                    sum = new_sum;
                }
            }
            turbo_list_append(row, sum);
        }
        turbo_list_append(result, row);
    }
    return result;
}

TurboObject* turbo_bitnot(TurboObject* a) {
    if (a->type == TYPE_INT) {
        return make_int_from_ll(~int_to_ll(a));
    }
    fprintf(stderr, "TypeError: unsupported operand type(s) for ~\n");
    exit(1);
}

static bool is_equal(TurboObject* a, TurboObject* b) {
    if (a == b) return true;
    if (a->type != b->type) return false;
    switch (a->type) {
        case TYPE_NONE: return true;
        case TYPE_INT: return a->int_val.sign == b->int_val.sign && abs_cmp(a->int_val.digits, a->int_val.length, b->int_val.digits, b->int_val.length) == 0;
        case TYPE_BOOL: return a->bool_val == b->bool_val;
        case TYPE_FLOAT: return a->float_val == b->float_val;
        case TYPE_COMPLEX:
            return a->complex_val.real == b->complex_val.real &&
                   a->complex_val.imag == b->complex_val.imag;
        case TYPE_STR: return strcmp(a->str_val.chars, b->str_val.chars) == 0;
        case TYPE_BYTES:
            if (a->bytes_val.length != b->bytes_val.length) return false;
            return memcmp(a->bytes_val.data, b->bytes_val.data, a->bytes_val.length) == 0;
        case TYPE_LIST:
            if (a->list_val.length != b->list_val.length) return false;
            for (int i = 0; i < a->list_val.length; i++) {
                if (!is_equal(a->list_val.items[i], b->list_val.items[i])) return false;
            }
            return true;
        case TYPE_TUPLE:
            if (a->tuple_val.length != b->tuple_val.length) return false;
            for (int i = 0; i < a->tuple_val.length; i++) {
                if (!is_equal(a->tuple_val.items[i], b->tuple_val.items[i])) return false;
            }
            return true;
        case TYPE_SET:
            if (a->set_val.length != b->set_val.length) return false;
            for (int i = 0; i < a->set_val.length; i++) {
                bool found = false;
                for (int j = 0; j < b->set_val.length; j++) {
                    if (is_equal(a->set_val.items[i], b->set_val.items[j])) {
                        found = true;
                        break;
                    }
                }
                if (!found) return false;
            }
            return true;
        case TYPE_MODULE:
            return false;
        default:
            return false;
    }
}

TurboObject* make_module(void) {
    TurboObject* obj = (TurboObject*)malloc(sizeof(TurboObject));
    obj->type = TYPE_MODULE;
    obj->module_val.dict = make_dict();
    return obj;
}

TurboObject* turbo_module_get(TurboObject* mod, const char* name) {
    if (mod->type != TYPE_MODULE) {
        fprintf(stderr, "TypeError: expected module\n");
        exit(1);
    }
    TurboObject* key = make_str(name);
    for (int i = 0; i < mod->module_val.dict->dict_val.length; i++) {
        if (is_equal(mod->module_val.dict->dict_val.keys[i], key)) {
            return mod->module_val.dict->dict_val.values[i];
        }
    }
    fprintf(stderr, "AttributeError: module has no attribute '%s'\n", name);
    exit(1);
}

void turbo_module_set(TurboObject* mod, const char* name, TurboObject* val) {
    if (mod->type != TYPE_MODULE) {
        fprintf(stderr, "TypeError: expected module\n");
        exit(1);
    }
    turbo_setitem(mod->module_val.dict, make_str(name), val);
}

TurboObject* turbo_import_module(const char* name) {
    (void)name;
    return turbo_none;
}

// Compare two bignums: returns >0 if a > b, 0 if ==, <0 if a < b
static int int_cmp(TurboObject* a, TurboObject* b) {
    if (a->int_val.sign != b->int_val.sign) return b->int_val.sign - a->int_val.sign;
    int cmp = abs_cmp(a->int_val.digits, a->int_val.length, b->int_val.digits, b->int_val.length);
    return a->int_val.sign ? -cmp : cmp;
}

TurboObject* turbo_eq(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_FLOAT && b->type == TYPE_INT) {
        return make_bool(a->float_val == to_double(b));
    }
    if (a->type == TYPE_INT && b->type == TYPE_FLOAT) {
        return make_bool(to_double(a) == b->float_val);
    }
    return make_bool(is_equal(a, b));
}

TurboObject* turbo_ne(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_FLOAT && b->type == TYPE_INT) {
        return make_bool(a->float_val != to_double(b));
    }
    if (a->type == TYPE_INT && b->type == TYPE_FLOAT) {
        return make_bool(to_double(a) != b->float_val);
    }
    return make_bool(!is_equal(a, b));
}

TurboObject* turbo_lt(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        return make_bool(int_cmp(a, b) < 0);
    }
    if (a->type == TYPE_FLOAT && b->type == TYPE_FLOAT) {
        return make_bool(a->float_val < b->float_val);
    }
    if (a->type == TYPE_FLOAT && b->type == TYPE_INT) {
        return make_bool(a->float_val < to_double(b));
    }
    if (a->type == TYPE_INT && b->type == TYPE_FLOAT) {
        return make_bool(to_double(a) < b->float_val);
    }
    if (a->type == TYPE_STR && b->type == TYPE_STR) {
        return make_bool(strcmp(a->str_val.chars, b->str_val.chars) < 0);
    }
    fprintf(stderr, "TypeError: '<' not supported between types\n");
    exit(1);
}

TurboObject* turbo_gt(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        return make_bool(int_cmp(a, b) > 0);
    }
    if (a->type == TYPE_FLOAT && b->type == TYPE_FLOAT) {
        return make_bool(a->float_val > b->float_val);
    }
    if (a->type == TYPE_FLOAT && b->type == TYPE_INT) {
        return make_bool(a->float_val > to_double(b));
    }
    if (a->type == TYPE_INT && b->type == TYPE_FLOAT) {
        return make_bool(to_double(a) > b->float_val);
    }
    if (a->type == TYPE_STR && b->type == TYPE_STR) {
        return make_bool(strcmp(a->str_val.chars, b->str_val.chars) > 0);
    }
    fprintf(stderr, "TypeError: '>' not supported between types\n");
    exit(1);
}

TurboObject* turbo_lte(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        return make_bool(int_cmp(a, b) <= 0);
    }
    if (a->type == TYPE_FLOAT && b->type == TYPE_FLOAT) {
        return make_bool(a->float_val <= b->float_val);
    }
    if (a->type == TYPE_FLOAT && b->type == TYPE_INT) {
        return make_bool(a->float_val <= to_double(b));
    }
    if (a->type == TYPE_INT && b->type == TYPE_FLOAT) {
        return make_bool(to_double(a) <= b->float_val);
    }
    if (a->type == TYPE_STR && b->type == TYPE_STR) {
        return make_bool(strcmp(a->str_val.chars, b->str_val.chars) <= 0);
    }
    fprintf(stderr, "TypeError: '<=' not supported between types\n");
    exit(1);
}

TurboObject* turbo_gte(TurboObject* a, TurboObject* b) {
    if (a->type == TYPE_INT && b->type == TYPE_INT) {
        return make_bool(int_cmp(a, b) >= 0);
    }
    if (a->type == TYPE_FLOAT && b->type == TYPE_FLOAT) {
        return make_bool(a->float_val >= b->float_val);
    }
    if (a->type == TYPE_FLOAT && b->type == TYPE_INT) {
        return make_bool(a->float_val >= to_double(b));
    }
    if (a->type == TYPE_INT && b->type == TYPE_FLOAT) {
        return make_bool(to_double(a) >= b->float_val);
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
        case TYPE_INT: return !(val->int_val.length == 1 && val->int_val.digits[0] == '0');
        case TYPE_FLOAT: return val->float_val != 0.0;
        case TYPE_COMPLEX: return val->complex_val.real != 0.0 || val->complex_val.imag != 0.0;
        case TYPE_STR: return val->str_val.length > 0;
        case TYPE_BYTES: return val->bytes_val.length > 0;
        case TYPE_LIST: return val->list_val.length > 0;
        case TYPE_TUPLE: return val->tuple_val.length > 0;
        case TYPE_SET: return val->set_val.length > 0;
        case TYPE_MODULE: return true;
        case TYPE_DICT: return val->dict_val.length > 0;
        default: return true;
    }
}

TurboObject* turbo_len(TurboObject* val) {
    if (val->type == TYPE_STR) {
        return make_int_from_ll(val->str_val.length);
    }
    if (val->type == TYPE_BYTES) {
        return make_int_from_ll(val->bytes_val.length);
    }
    if (val->type == TYPE_LIST) {
        return make_int_from_ll(val->list_val.length);
    }
    if (val->type == TYPE_TUPLE) {
        return make_int_from_ll(val->tuple_val.length);
    }
    if (val->type == TYPE_SET) {
        return make_int_from_ll(val->set_val.length);
    }
    if (val->type == TYPE_DICT) {
        return make_int_from_ll(val->dict_val.length);
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
        long long idx = int_to_ll(key);
        long long orig_idx = idx;
        if (idx < 0) idx += obj->list_val.length;
        if (idx < 0 || idx >= obj->list_val.length) {
            fprintf(stderr, "IndexError: list index out of range: index %lld (original %lld), length %d\n", idx, orig_idx, obj->list_val.length);
            exit(1);
        }
        return obj->list_val.items[idx];
    }
    if (obj->type == TYPE_TUPLE) {
        if (key->type != TYPE_INT) {
            fprintf(stderr, "TypeError: tuple indices must be integers\n");
            exit(1);
        }
        long long idx = int_to_ll(key);
        long long orig_idx = idx;
        if (idx < 0) idx += obj->tuple_val.length;
        if (idx < 0 || idx >= obj->tuple_val.length) {
            fprintf(stderr, "IndexError: tuple index out of range: index %lld (original %lld), length %d\n", idx, orig_idx, obj->tuple_val.length);
            exit(1);
        }
        return obj->tuple_val.items[idx];
    }
    if (obj->type == TYPE_STR) {
        if (key->type != TYPE_INT) {
            fprintf(stderr, "TypeError: string indices must be integers\n");
            exit(1);
        }
        long long idx = int_to_ll(key);
        long long orig_idx = idx;
        if (idx < 0) idx += obj->str_val.length;
        if (idx < 0 || idx >= obj->str_val.length) {
            fprintf(stderr, "IndexError: string index out of range: index %lld (original %lld), length %d\n", idx, orig_idx, obj->str_val.length);
            exit(1);
        }
        char temp[2] = { obj->str_val.chars[idx], '\0' };
        return make_str(temp);
    }
    if (obj->type == TYPE_BYTES) {
        if (key->type != TYPE_INT) {
            fprintf(stderr, "TypeError: bytes indices must be integers\n");
            exit(1);
        }
        long long idx = int_to_ll(key);
        long long orig_idx = idx;
        if (idx < 0) idx += obj->bytes_val.length;
        if (idx < 0 || idx >= obj->bytes_val.length) {
            fprintf(stderr, "IndexError: bytes index out of range: index %lld (original %lld), length %d\n", idx, orig_idx, obj->bytes_val.length);
            exit(1);
        }
        return make_int_from_ll(obj->bytes_val.data[idx]);
    }
    if (obj->type == TYPE_DICT) {
        for (int i = 0; i < obj->dict_val.length; i++) {
            if (is_equal(obj->dict_val.keys[i], key)) {
                return obj->dict_val.values[i];
            }
        }
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
        long long idx = int_to_ll(key);
        if (idx < 0) idx += obj->list_val.length;
        if (idx < 0 || idx >= obj->list_val.length) {
            fprintf(stderr, "IndexError: list assignment index out of range\n");
            exit(1);
        }
        obj->list_val.items[idx] = val;
        return;
    }
    if (obj->type == TYPE_DICT) {
        for (int i = 0; i < obj->dict_val.length; i++) {
            if (is_equal(obj->dict_val.keys[i], key)) {
                obj->dict_val.values[i] = val;
                return;
            }
        }
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

void turbo_delitem(TurboObject* obj, TurboObject* key) {
    if (obj->type == TYPE_LIST) {
        if (key->type != TYPE_INT) {
            fprintf(stderr, "TypeError: list indices must be integers\n");
            exit(1);
        }
        long long idx = int_to_ll(key);
        if (idx < 0) idx += obj->list_val.length;
        if (idx < 0 || idx >= obj->list_val.length) {
            fprintf(stderr, "IndexError: list index out of range\n");
            exit(1);
        }
        for (long long i = idx; i < obj->list_val.length - 1; i++) {
            obj->list_val.items[i] = obj->list_val.items[i + 1];
        }
        obj->list_val.length--;
        return;
    }
    if (obj->type == TYPE_DICT) {
        for (int i = 0; i < obj->dict_val.length; i++) {
            if (is_equal(obj->dict_val.keys[i], key)) {
                for (int j = i; j < obj->dict_val.length - 1; j++) {
                    obj->dict_val.keys[j] = obj->dict_val.keys[j + 1];
                    obj->dict_val.values[j] = obj->dict_val.values[j + 1];
                }
                obj->dict_val.length--;
                return;
            }
        }
        fprintf(stderr, "KeyError: key not found in dict\n");
        exit(1);
    }
    fprintf(stderr, "TypeError: object does not support item deletion\n");
    exit(1);
}

TurboObject* turbo_slice(TurboObject* obj, TurboObject* start, TurboObject* end) {
    long long len = 0;
    bool is_bytes = false;
    bool is_tuple = false;
    if (obj->type == TYPE_STR) {
        len = obj->str_val.length;
    } else if (obj->type == TYPE_BYTES) {
        len = obj->bytes_val.length;
        is_bytes = true;
    } else if (obj->type == TYPE_LIST) {
        len = obj->list_val.length;
    } else if (obj->type == TYPE_TUPLE) {
        len = obj->tuple_val.length;
        is_tuple = true;
    } else {
        fprintf(stderr, "TypeError: unsliceable type\n");
        exit(1);
    }

    long long s = (start == turbo_none) ? 0 : int_to_ll(start);
    long long e = (end == turbo_none) ? len : int_to_ll(end);

    if (s < 0) s += len;
    if (s < 0) s = 0;
    if (s > len) s = len;

    if (e < 0) e += len;
    if (e < 0) e = 0;
    if (e > len) e = len;

    if (s >= e) {
        if (obj->type == TYPE_STR) return make_str("");
        if (is_bytes) return make_bytes((const unsigned char*)"", 0);
        if (is_tuple) return make_tuple();
        return make_list();
    }

    long long slice_len = e - s;
    if (obj->type == TYPE_STR) {
        return make_str_len(obj->str_val.chars + s, slice_len);
    } else if (is_bytes) {
        return make_bytes(obj->bytes_val.data + s, slice_len);
    } else if (is_tuple) {
        TurboObject* new_tup = make_tuple();
        for (long long i = s; i < e; i++) {
            turbo_tuple_append(new_tup, obj->tuple_val.items[i]);
        }
        return new_tup;
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
        TurboObject* cls = obj->inst_val.class_obj;
        for (int i = 0; i < cls->class_val.method_count; i++) {
            if (strcmp(cls->class_val.method_names[i], name) == 0) {
                return make_func(cls->class_val.method_funcs[i], name);
            }
        }
        fprintf(stderr, "AttributeError: '%s' object has no attribute '%s'\n", cls->class_val.name, name);
        exit(1);
    }
    if (obj->type == TYPE_MODULE) {
        return turbo_module_get(obj, name);
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
    if (obj->type == TYPE_MODULE) {
        turbo_module_set(obj, name, val);
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
    if (container->type == TYPE_TUPLE) {
        for (int i = 0; i < container->tuple_val.length; i++) {
            if (is_equal(item, container->tuple_val.items[i])) {
                return true;
            }
        }
        return false;
    }
    if (container->type == TYPE_SET) {
        for (int i = 0; i < container->set_val.length; i++) {
            if (is_equal(item, container->set_val.items[i])) {
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
    if (container->type == TYPE_BYTES) {
        if (item->type != TYPE_INT) {
            fprintf(stderr, "TypeError: 'in <bytes>' requires int as left operand\n");
            exit(1);
        }
        int byte_val = (int)int_to_ll(item);
        for (int i = 0; i < container->bytes_val.length; i++) {
            if (container->bytes_val.data[i] == byte_val) return true;
        }
        return false;
    }
    fprintf(stderr, "TypeError: argument of type is not iterable\n");
    exit(1);
}

static TurboObject* bytes_to_turbo_str(TurboObject* val) {
    int max_len = 3;
    for (int i = 0; i < val->bytes_val.length; i++) {
        unsigned char c = val->bytes_val.data[i];
        if (c == '\'' || c == '\\') max_len += 2;
        else if (c >= 0x20 && c <= 0x7e) max_len += 1;
        else max_len += 4;
    }
    char* buf = (char*)malloc(max_len + 1);
    int pos = 0;
    buf[pos++] = 'b';
    buf[pos++] = '\'';
    for (int i = 0; i < val->bytes_val.length; i++) {
        unsigned char c = val->bytes_val.data[i];
        if (c == '\'' || c == '\\') {
            buf[pos++] = '\\';
            buf[pos++] = c;
        } else if (c >= 0x20 && c <= 0x7e) {
            buf[pos++] = c;
        } else {
            snprintf(buf + pos, 5, "\\x%02x", c);
            pos += 4;
        }
    }
    buf[pos++] = '\'';
    buf[pos] = '\0';
    TurboObject* res = make_str(buf);
    free(buf);
    return res;
}

TurboObject* turbo_str(TurboObject* val) {
    char buf[128];
    switch (val->type) {
        case TYPE_NONE:
            return make_str("None");
        case TYPE_BOOL:
            return make_str(val->bool_val ? "True" : "False");
        case TYPE_INT: {
            const char* sign_str = val->int_val.sign ? "-" : "";
            char* res = (char*)malloc(val->int_val.length + 2);
            sprintf(res, "%s%s", sign_str, val->int_val.digits);
            TurboObject* str_obj = make_str(res);
            free(res);
            return str_obj;
        }
        case TYPE_FLOAT:
            sprintf(buf, "%g", val->float_val);
            return make_str(buf);
        case TYPE_COMPLEX:
            if (val->complex_val.imag >= 0) {
                snprintf(buf, sizeof(buf), "(%g+%gj)", val->complex_val.real, val->complex_val.imag);
            } else {
                snprintf(buf, sizeof(buf), "(%g%gj)", val->complex_val.real, val->complex_val.imag);
            }
            return make_str(buf);
        case TYPE_STR:
            return val;
        case TYPE_BYTES:
            return bytes_to_turbo_str(val);
        case TYPE_LIST: {
            char* s = strdup("[");
            for (int i = 0; i < val->list_val.length; i++) {
                TurboObject* item_str = turbo_str(val->list_val.items[i]);
                int new_len = strlen(s) + item_str->str_val.length + 3;
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
        case TYPE_TUPLE: {
            char* s = strdup("(");
            for (int i = 0; i < val->tuple_val.length; i++) {
                TurboObject* item_str = turbo_str(val->tuple_val.items[i]);
                int new_len = strlen(s) + item_str->str_val.length + 3;
                char* s2 = (char*)malloc(new_len);
                strcpy(s2, s);
                strcat(s2, item_str->str_val.chars);
                if (i < val->tuple_val.length - 1) {
                    strcat(s2, ", ");
                }
                free(s);
                s = s2;
            }
            char* s3 = (char*)malloc(strlen(s) + 3);
            strcpy(s3, s);
            if (val->tuple_val.length == 1) {
                strcat(s3, ",");
            }
            strcat(s3, ")");
            free(s);
            TurboObject* res = make_str(s3);
            free(s3);
            return res;
        }
        case TYPE_SET: {
            char* s = strdup("{");
            for (int i = 0; i < val->set_val.length; i++) {
                TurboObject* item_str = turbo_str(val->set_val.items[i]);
                int new_len = strlen(s) + item_str->str_val.length + 3;
                char* s2 = (char*)malloc(new_len);
                strcpy(s2, s);
                strcat(s2, item_str->str_val.chars);
                if (i < val->set_val.length - 1) {
                    strcat(s2, ", ");
                }
                free(s);
                s = s2;
            }
            char* s3 = (char*)malloc(strlen(s) + 2);
            strcpy(s3, s);
            strcat(s3, "}");
            free(s);
            TurboObject* res = make_str(s3);
            free(s3);
            return res;
        }
        case TYPE_DICT: {
            char* s = strdup("{");
            for (int i = 0; i < val->dict_val.length; i++) {
                TurboObject* key_str = turbo_str(val->dict_val.keys[i]);
                TurboObject* val_str = turbo_str(val->dict_val.values[i]);
                int new_len = strlen(s) + key_str->str_val.length + val_str->str_val.length + 5;
                char* s2 = (char*)malloc(new_len);
                strcpy(s2, s);
                strcat(s2, key_str->str_val.chars);
                strcat(s2, ": ");
                strcat(s2, val_str->str_val.chars);
                if (i < val->dict_val.length - 1) {
                    strcat(s2, ", ");
                }
                free(s);
                s = s2;
            }
            char* s3 = (char*)malloc(strlen(s) + 2);
            strcpy(s3, s);
            strcat(s3, "}");
            free(s);
            TurboObject* res = make_str(s3);
            free(s3);
            return res;
        }
        case TYPE_MODULE:
            return make_str("<module>");
        default:
            return make_str("<object>");
    }
}

TurboObject* turbo_repr(TurboObject* val) {
    if (val->type == TYPE_STR) {
        int len = val->str_val.length;
        char* buf = (char*)malloc(len + 3);
        buf[0] = '"';
        memcpy(buf + 1, val->str_val.chars, len);
        buf[len + 1] = '"';
        buf[len + 2] = '\0';
        TurboObject* res = make_str_len(buf, len + 2);
        free(buf);
        return res;
    }
    return turbo_str(val);
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
    if (val->type == TYPE_BOOL) return make_int_from_ll(val->bool_val ? 1 : 0);
    if (val->type == TYPE_FLOAT) return make_int_from_ll((long long)val->float_val);
    if (val->type == TYPE_STR) {
        return make_int(val->str_val.chars);
    }
    if (val->type == TYPE_BYTES) {
        char* temp = (char*)malloc(val->bytes_val.length + 1);
        memcpy(temp, val->bytes_val.data, val->bytes_val.length);
        temp[val->bytes_val.length] = '\0';
        TurboObject* res = make_int(temp);
        free(temp);
        return res;
    }
    fprintf(stderr, "ValueError: invalid literal for int()\n");
    exit(1);
}

TurboObject* turbo_float(TurboObject* val) {
    if (val->type == TYPE_FLOAT) return val;
    if (val->type == TYPE_INT) return make_float((double)int_to_ll(val));
    if (val->type == TYPE_BOOL) return make_float(val->bool_val ? 1.0 : 0.0);
    if (val->type == TYPE_STR) {
        return make_float(atof(val->str_val.chars));
    }
    fprintf(stderr, "ValueError: invalid literal for float()\n");
    exit(1);
}

TurboObject* turbo_ord(TurboObject* val) {
    if (val->type != TYPE_STR || val->str_val.length == 0) {
        fprintf(stderr, "TypeError: ord() expected a string of length 1\n");
        exit(1);
    }
    return make_int_from_ll((unsigned char)val->str_val.chars[0]);
}

TurboObject* turbo_chr(TurboObject* val) {
    if (val->type != TYPE_INT) {
        fprintf(stderr, "TypeError: integer expected\n");
        exit(1);
    }
    char temp[2] = { (char)int_to_ll(val), '\0' };
    return make_str(temp);
}

TurboObject* turbo_range(int argc, TurboObject** args) {
    long long start = 0;
    long long stop = 0;
    if (argc == 1) {
        stop = int_to_ll(args[0]);
    } else if (argc == 2) {
        start = int_to_ll(args[0]);
        stop = int_to_ll(args[1]);
    } else {
        fprintf(stderr, "TypeError: range expected 1 or 2 arguments\n");
        exit(1);
    }
    TurboObject* list = make_list();
    for (long long i = start; i < stop; i++) {
        add_to_list(list, make_int_from_ll(i));
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
        turbo_raise(make_str("FileNotFoundError"));
        return turbo_none;
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

void turbo_tuple_append(TurboObject* tuple_obj, TurboObject* item) {
    if (tuple_obj->type != TYPE_TUPLE) {
        fprintf(stderr, "TypeError: expected tuple object\n");
        exit(1);
    }
    if (tuple_obj->tuple_val.length >= tuple_obj->tuple_val.capacity) {
        tuple_obj->tuple_val.capacity *= 2;
        tuple_obj->tuple_val.items = (TurboObject**)realloc(tuple_obj->tuple_val.items, sizeof(TurboObject*) * tuple_obj->tuple_val.capacity);
    }
    tuple_obj->tuple_val.items[tuple_obj->tuple_val.length] = item;
    tuple_obj->tuple_val.length++;
}

void turbo_set_add(TurboObject* set_obj, TurboObject* item) {
    if (set_obj->type != TYPE_SET) {
        fprintf(stderr, "TypeError: expected set object\n");
        exit(1);
    }
    for (int i = 0; i < set_obj->set_val.length; i++) {
        if (is_equal(set_obj->set_val.items[i], item)) {
            return;
        }
    }
    if (set_obj->set_val.length >= set_obj->set_val.capacity) {
        set_obj->set_val.capacity *= 2;
        set_obj->set_val.items = (TurboObject**)realloc(set_obj->set_val.items, sizeof(TurboObject*) * set_obj->set_val.capacity);
    }
    set_obj->set_val.items[set_obj->set_val.length] = item;
    set_obj->set_val.length++;
}

bool turbo_set_contains(TurboObject* set_obj, TurboObject* item) {
    if (set_obj->type != TYPE_SET) {
        fprintf(stderr, "TypeError: expected set object\n");
        exit(1);
    }
    for (int i = 0; i < set_obj->set_val.length; i++) {
        if (is_equal(set_obj->set_val.items[i], item)) {
            return true;
        }
    }
    return false;
}

void turbo_set_remove(TurboObject* set_obj, TurboObject* item) {
    if (set_obj->type != TYPE_SET) {
        fprintf(stderr, "TypeError: expected set object\n");
        exit(1);
    }
    for (int i = 0; i < set_obj->set_val.length; i++) {
        if (is_equal(set_obj->set_val.items[i], item)) {
            set_obj->set_val.items[i] = set_obj->set_val.items[set_obj->set_val.length - 1];
            set_obj->set_val.length--;
            return;
        }
    }
    fprintf(stderr, "KeyError: element not found in set\n");
    exit(1);
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
        char* str_copy = strdup(str);
        char* token = strtok(str_copy, " \t\r\n");
        while (token != NULL) {
            add_to_list(list, make_str(token));
            token = strtok(NULL, " \t\r\n");
        }
        free(str_copy);
    } else {
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
        TurboObject* inst = make_instance(callable);
        for (int i = 0; i < callable->class_val.method_count; i++) {
            if (strcmp(callable->class_val.method_names[i], "__init__") == 0) {
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

// String method implementations
TurboObject* turbo_str_join(TurboObject* self, TurboObject* list_obj) {
    if (list_obj->type != TYPE_LIST) {
        fprintf(stderr, "TypeError: join() argument must be a list\n");
        exit(1);
    }
    int total_len = 0;
    int sep_len = self->str_val.length;
    int count = list_obj->list_val.length;
    for (int i = 0; i < count; i++) {
        TurboObject* item = list_obj->list_val.items[i];
        if (item->type != TYPE_STR) {
            fprintf(stderr, "TypeError: sequence item %d: expected string\n", i);
            exit(1);
        }
        total_len += item->str_val.length;
        if (i < count - 1) total_len += sep_len;
    }
    char* result = (char*)malloc(total_len + 1);
    result[0] = '\0';
    for (int i = 0; i < count; i++) {
        TurboObject* item = list_obj->list_val.items[i];
        strcat(result, item->str_val.chars);
        if (i < count - 1) {
            strncat(result, self->str_val.chars, sep_len);
        }
    }
    TurboObject* res = make_str_len(result, total_len);
    free(result);
    return res;
}

TurboObject* turbo_str_upper(TurboObject* self) {
    int len = self->str_val.length;
    char* result = (char*)malloc(len + 1);
    for (int i = 0; i < len; i++) {
        result[i] = toupper((unsigned char)self->str_val.chars[i]);
    }
    result[len] = '\0';
    TurboObject* res = make_str_len(result, len);
    free(result);
    return res;
}

TurboObject* turbo_str_lower(TurboObject* self) {
    int len = self->str_val.length;
    char* result = (char*)malloc(len + 1);
    for (int i = 0; i < len; i++) {
        result[i] = tolower((unsigned char)self->str_val.chars[i]);
    }
    result[len] = '\0';
    TurboObject* res = make_str_len(result, len);
    free(result);
    return res;
}

TurboObject* turbo_str_replace(TurboObject* self, TurboObject* old, TurboObject* new) {
    if (old->type != TYPE_STR || new->type != TYPE_STR) {
        fprintf(stderr, "TypeError: replace() arguments must be strings\n");
        exit(1);
    }
    char* haystack = self->str_val.chars;
    char* needle = old->str_val.chars;
    char* replacement = new->str_val.chars;
    int needle_len = old->str_val.length;
    int replacement_len = new->str_val.length;
    int count = 0;
    char* p = haystack;
    while ((p = strstr(p, needle)) != NULL) {
        count++;
        p += needle_len;
    }
    if (count == 0) {
        return make_str(haystack);
    }
    int haystack_len = self->str_val.length;
    int result_len = haystack_len + count * (replacement_len - needle_len);
    char* result = (char*)malloc(result_len + 1);
    result[0] = '\0';
    char* src = haystack;
    while ((p = strstr(src, needle)) != NULL) {
        int prefix_len = p - src;
        strncat(result, src, prefix_len);
        strcat(result, replacement);
        src = p + needle_len;
    }
    strcat(result, src);
    TurboObject* res = make_str_len(result, result_len);
    free(result);
    return res;
}

TurboObject* turbo_str_find(TurboObject* self, TurboObject* sub) {
    if (sub->type != TYPE_STR) {
        fprintf(stderr, "TypeError: find() argument must be string\n");
        exit(1);
    }
    char* pos = strstr(self->str_val.chars, sub->str_val.chars);
    if (pos == NULL) {
        return make_int_from_ll(-1);
    }
    return make_int_from_ll((int)(pos - self->str_val.chars));
}

TurboObject* turbo_str_strip(TurboObject* self) {
    char* s = self->str_val.chars;
    int len = self->str_val.length;
    int start = 0;
    while (start < len && (s[start] == ' ' || s[start] == '\t' || s[start] == '\n' || s[start] == '\r')) {
        start++;
    }
    int end = len;
    while (end > start && (s[end-1] == ' ' || s[end-1] == '\t' || s[end-1] == '\n' || s[end-1] == '\r')) {
        end--;
    }
    return make_str_len(s + start, end - start);
}

TurboObject* turbo_str_lstrip(TurboObject* self) {
    char* s = self->str_val.chars;
    int len = self->str_val.length;
    int start = 0;
    while (start < len && (s[start] == ' ' || s[start] == '\t' || s[start] == '\n' || s[start] == '\r')) {
        start++;
    }
    return make_str_len(s + start, len - start);
}

TurboObject* turbo_str_rstrip(TurboObject* self) {
    char* s = self->str_val.chars;
    int len = self->str_val.length;
    int end = len;
    while (end > 0 && (s[end-1] == ' ' || s[end-1] == '\t' || s[end-1] == '\n' || s[end-1] == '\r')) {
        end--;
    }
    return make_str_len(s, end);
}

TurboObject* turbo_str_startswith(TurboObject* self, TurboObject* prefix) {
    if (prefix->type != TYPE_STR) {
        fprintf(stderr, "TypeError: startswith() argument must be string\n");
        exit(1);
    }
    int self_len = self->str_val.length;
    int prefix_len = prefix->str_val.length;
    if (prefix_len > self_len) return turbo_false;
    return make_bool(strncmp(self->str_val.chars, prefix->str_val.chars, prefix_len) == 0);
}

TurboObject* turbo_str_endswith(TurboObject* self, TurboObject* suffix) {
    if (suffix->type != TYPE_STR) {
        fprintf(stderr, "TypeError: endswith() argument must be string\n");
        exit(1);
    }
    int self_len = self->str_val.length;
    int suffix_len = suffix->str_val.length;
    if (suffix_len > self_len) return turbo_false;
    return make_bool(strncmp(self->str_val.chars + self_len - suffix_len, suffix->str_val.chars, suffix_len) == 0);
}

TurboObject* turbo_str_count(TurboObject* self, TurboObject* sub) {
    if (sub->type != TYPE_STR) {
        fprintf(stderr, "TypeError: count() argument must be string\n");
        exit(1);
    }
    int count = 0;
    char* p = self->str_val.chars;
    int needle_len = sub->str_val.length;
    if (needle_len == 0) return make_int_from_ll(self->str_val.length + 1);
    while ((p = strstr(p, sub->str_val.chars)) != NULL) {
        count++;
        p += needle_len;
    }
    return make_int_from_ll(count);
}

TurboObject* turbo_str_format(TurboObject* self, int argc, TurboObject** args) {
    const char* fmt = self->str_val.chars;
    int fmt_len = self->str_val.length;
    int total_len = 0;
    int arg_idx = 0;
    int i = 0;
    while (i < fmt_len) {
        if (fmt[i] == '{') {
            i++;
            if (i < fmt_len && fmt[i] == '{') {
                total_len++;
                i++;
            } else {
                int idx = arg_idx;
                if (i < fmt_len && fmt[i] >= '0' && fmt[i] <= '9') {
                    idx = 0;
                    while (i < fmt_len && fmt[i] >= '0' && fmt[i] <= '9') {
                        idx = idx * 10 + (fmt[i] - '0');
                        i++;
                    }
                }
                if (i < fmt_len) i++;
                if (idx >= argc) {
                    fprintf(stderr, "IndexError: tuple index out of range\n");
                    exit(1);
                }
                TurboObject* arg_str = turbo_str(args[idx]);
                total_len += arg_str->str_val.length;
                arg_idx = idx + 1 > arg_idx ? idx + 1 : arg_idx;
            }
        } else if (fmt[i] == '}') {
            i++;
            if (i < fmt_len && fmt[i] == '}') {
                total_len++;
                i++;
            }
        } else {
            total_len++;
            i++;
        }
    }
    char* result = (char*)malloc(total_len + 1);
    int pos = 0;
    arg_idx = 0;
    i = 0;
    while (i < fmt_len) {
        if (fmt[i] == '{') {
            i++;
            if (i < fmt_len && fmt[i] == '{') {
                result[pos++] = '{';
                i++;
            } else {
                int idx = arg_idx;
                if (i < fmt_len && fmt[i] >= '0' && fmt[i] <= '9') {
                    idx = 0;
                    while (i < fmt_len && fmt[i] >= '0' && fmt[i] <= '9') {
                        idx = idx * 10 + (fmt[i] - '0');
                        i++;
                    }
                }
                if (i < fmt_len) i++;
                TurboObject* arg_str = turbo_str(args[idx]);
                memcpy(result + pos, arg_str->str_val.chars, arg_str->str_val.length);
                pos += arg_str->str_val.length;
                arg_idx = idx + 1 > arg_idx ? idx + 1 : arg_idx;
            }
        } else if (fmt[i] == '}') {
            i++;
            if (i < fmt_len && fmt[i] == '}') {
                result[pos++] = '}';
                i++;
            }
        } else {
            result[pos++] = fmt[i];
            i++;
        }
    }
    result[pos] = '\0';
    TurboObject* res = make_str_len(result, pos);
    free(result);
    return res;
}

// List method implementations
void turbo_list_extend(TurboObject* list_obj, TurboObject* other) {
    if (other->type != TYPE_LIST) {
        fprintf(stderr, "TypeError: extend() argument must be a list\n");
        exit(1);
    }
    for (int i = 0; i < other->list_val.length; i++) {
        turbo_list_append(list_obj, other->list_val.items[i]);
    }
}

void turbo_list_insert(TurboObject* list_obj, TurboObject* index_obj, TurboObject* item) {
    if (index_obj->type != TYPE_INT) {
        fprintf(stderr, "TypeError: insert() index must be integer\n");
        exit(1);
    }
    long long idx = int_to_ll(index_obj);
    int len = list_obj->list_val.length;
    if (idx < 0) idx += len;
    if (idx < 0) idx = 0;
    if (idx > len) idx = len;
    if (list_obj->list_val.length >= list_obj->list_val.capacity) {
        list_obj->list_val.capacity *= 2;
        list_obj->list_val.items = (TurboObject**)realloc(list_obj->list_val.items, sizeof(TurboObject*) * list_obj->list_val.capacity);
    }
    for (int i = len; i > idx; i--) {
        list_obj->list_val.items[i] = list_obj->list_val.items[i-1];
    }
    list_obj->list_val.items[idx] = item;
    list_obj->list_val.length++;
}

void turbo_list_remove(TurboObject* list_obj, TurboObject* item) {
    for (int i = 0; i < list_obj->list_val.length; i++) {
        if (is_equal(list_obj->list_val.items[i], item)) {
            for (int j = i; j < list_obj->list_val.length - 1; j++) {
                list_obj->list_val.items[j] = list_obj->list_val.items[j+1];
            }
            list_obj->list_val.length--;
            return;
        }
    }
    fprintf(stderr, "ValueError: list.remove(x): x not in list\n");
    exit(1);
}

TurboObject* turbo_list_index(TurboObject* list_obj, TurboObject* item) {
    for (int i = 0; i < list_obj->list_val.length; i++) {
        if (is_equal(list_obj->list_val.items[i], item)) {
            return make_int_from_ll(i);
        }
    }
    fprintf(stderr, "ValueError: list.index(x): x not in list\n");
    exit(1);
}

TurboObject* turbo_list_count(TurboObject* list_obj, TurboObject* item) {
    int count = 0;
    for (int i = 0; i < list_obj->list_val.length; i++) {
        if (is_equal(list_obj->list_val.items[i], item)) {
            count++;
        }
    }
    return make_int_from_ll(count);
}

void turbo_list_sort(TurboObject* list_obj) {
    int n = list_obj->list_val.length;
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            TurboObject* a = list_obj->list_val.items[j];
            TurboObject* b = list_obj->list_val.items[j+1];
            if (a->type == TYPE_INT && b->type == TYPE_INT) {
                if (int_to_ll(a) > int_to_ll(b)) {
                    list_obj->list_val.items[j] = b;
                    list_obj->list_val.items[j+1] = a;
                }
            }
        }
    }
}

void turbo_list_reverse(TurboObject* list_obj) {
    int n = list_obj->list_val.length;
    for (int i = 0; i < n / 2; i++) {
        TurboObject* temp = list_obj->list_val.items[i];
        list_obj->list_val.items[i] = list_obj->list_val.items[n - 1 - i];
        list_obj->list_val.items[n - 1 - i] = temp;
    }
}

TurboObject* turbo_list_copy(TurboObject* list_obj) {
    TurboObject* new_list = make_list();
    for (int i = 0; i < list_obj->list_val.length; i++) {
        turbo_list_append(new_list, list_obj->list_val.items[i]);
    }
    return new_list;
}

void turbo_list_clear(TurboObject* list_obj) {
    list_obj->list_val.length = 0;
}

// Dict method implementations
TurboObject* turbo_dict_keys(TurboObject* self) {
    TurboObject* list = make_list();
    for (int i = 0; i < self->dict_val.length; i++) {
        turbo_list_append(list, self->dict_val.keys[i]);
    }
    return list;
}

TurboObject* turbo_dict_values(TurboObject* self) {
    TurboObject* list = make_list();
    for (int i = 0; i < self->dict_val.length; i++) {
        turbo_list_append(list, self->dict_val.values[i]);
    }
    return list;
}

TurboObject* turbo_dict_items(TurboObject* self) {
    TurboObject* list = make_list();
    for (int i = 0; i < self->dict_val.length; i++) {
        TurboObject* pair = make_list();
        turbo_list_append(pair, self->dict_val.keys[i]);
        turbo_list_append(pair, self->dict_val.values[i]);
        turbo_list_append(list, pair);
    }
    return list;
}

TurboObject* turbo_dict_get(TurboObject* self, TurboObject* key, TurboObject* default_val) {
    for (int i = 0; i < self->dict_val.length; i++) {
        if (is_equal(self->dict_val.keys[i], key)) {
            return self->dict_val.values[i];
        }
    }
    return default_val;
}

TurboObject* turbo_dict_pop(TurboObject* self, TurboObject* key) {
    for (int i = 0; i < self->dict_val.length; i++) {
        if (is_equal(self->dict_val.keys[i], key)) {
            TurboObject* value = self->dict_val.values[i];
            for (int j = i; j < self->dict_val.length - 1; j++) {
                self->dict_val.keys[j] = self->dict_val.keys[j+1];
                self->dict_val.values[j] = self->dict_val.values[j+1];
            }
            self->dict_val.length--;
            return value;
        }
    }
    fprintf(stderr, "KeyError: pop(): key not found in dict\n");
    exit(1);
}

TurboObject* turbo_dict_popitem(TurboObject* self) {
    if (self->dict_val.length == 0) {
        fprintf(stderr, "KeyError: popitem(): dictionary is empty\n");
        exit(1);
    }
    int last = self->dict_val.length - 1;
    TurboObject* key = self->dict_val.keys[last];
    TurboObject* value = self->dict_val.values[last];
    self->dict_val.length--;
    TurboObject* pair = make_tuple();
    turbo_tuple_append(pair, key);
    turbo_tuple_append(pair, value);
    return pair;
}

void turbo_dict_update(TurboObject* self, TurboObject* other) {
    if (other->type != TYPE_DICT) {
        fprintf(stderr, "TypeError: update() argument must be dict\n");
        exit(1);
    }
    for (int i = 0; i < other->dict_val.length; i++) {
        turbo_setitem(self, other->dict_val.keys[i], other->dict_val.values[i]);
    }
}

void turbo_dict_clear(TurboObject* self) {
    self->dict_val.length = 0;
}

TurboObject* turbo_dict_copy(TurboObject* self) {
    TurboObject* new_dict = make_dict();
    for (int i = 0; i < self->dict_val.length; i++) {
        turbo_setitem(new_dict, self->dict_val.keys[i], self->dict_val.values[i]);
    }
    return new_dict;
}

TurboObject* turbo_dict_setdefault(TurboObject* self, TurboObject* key, TurboObject* default_val) {
    for (int i = 0; i < self->dict_val.length; i++) {
        if (is_equal(self->dict_val.keys[i], key)) {
            return self->dict_val.values[i];
        }
    }
    turbo_setitem(self, key, default_val);
    return default_val;
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
        if (strcmp(method_name, "extend") == 0) {
            if (argc != 1) {
                fprintf(stderr, "TypeError: extend() takes exactly one argument\n");
                exit(1);
            }
            turbo_list_extend(obj, args[0]);
            return turbo_none;
        }
        if (strcmp(method_name, "insert") == 0) {
            if (argc != 2) {
                fprintf(stderr, "TypeError: insert() takes exactly two arguments\n");
                exit(1);
            }
            turbo_list_insert(obj, args[0], args[1]);
            return turbo_none;
        }
        if (strcmp(method_name, "remove") == 0) {
            if (argc != 1) {
                fprintf(stderr, "TypeError: remove() takes exactly one argument\n");
                exit(1);
            }
            turbo_list_remove(obj, args[0]);
            return turbo_none;
        }
        if (strcmp(method_name, "index") == 0) {
            if (argc != 1) {
                fprintf(stderr, "TypeError: index() takes exactly one argument\n");
                exit(1);
            }
            return turbo_list_index(obj, args[0]);
        }
        if (strcmp(method_name, "count") == 0) {
            if (argc != 1) {
                fprintf(stderr, "TypeError: count() takes exactly one argument\n");
                exit(1);
            }
            return turbo_list_count(obj, args[0]);
        }
        if (strcmp(method_name, "sort") == 0) {
            if (argc != 0) {
                fprintf(stderr, "TypeError: sort() takes no arguments\n");
                exit(1);
            }
            turbo_list_sort(obj);
            return turbo_none;
        }
        if (strcmp(method_name, "reverse") == 0) {
            if (argc != 0) {
                fprintf(stderr, "TypeError: reverse() takes no arguments\n");
                exit(1);
            }
            turbo_list_reverse(obj);
            return turbo_none;
        }
        if (strcmp(method_name, "copy") == 0) {
            if (argc != 0) {
                fprintf(stderr, "TypeError: copy() takes no arguments\n");
                exit(1);
            }
            return turbo_list_copy(obj);
        }
        if (strcmp(method_name, "clear") == 0) {
            if (argc != 0) {
                fprintf(stderr, "TypeError: clear() takes no arguments\n");
                exit(1);
            }
            turbo_list_clear(obj);
            return turbo_none;
        }
    }
    if (obj->type == TYPE_STR) {
        if (strcmp(method_name, "split") == 0) {
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
        if (strcmp(method_name, "join") == 0) {
            if (argc != 1) {
                fprintf(stderr, "TypeError: join() takes exactly one argument\n");
                exit(1);
            }
            return turbo_str_join(obj, args[0]);
        }
        if (strcmp(method_name, "upper") == 0) {
            if (argc != 0) {
                fprintf(stderr, "TypeError: upper() takes no arguments\n");
                exit(1);
            }
            return turbo_str_upper(obj);
        }
        if (strcmp(method_name, "lower") == 0) {
            if (argc != 0) {
                fprintf(stderr, "TypeError: lower() takes no arguments\n");
                exit(1);
            }
            return turbo_str_lower(obj);
        }
        if (strcmp(method_name, "replace") == 0) {
            if (argc != 2) {
                fprintf(stderr, "TypeError: replace() takes exactly two arguments\n");
                exit(1);
            }
            return turbo_str_replace(obj, args[0], args[1]);
        }
        if (strcmp(method_name, "find") == 0) {
            if (argc != 1) {
                fprintf(stderr, "TypeError: find() takes exactly one argument\n");
                exit(1);
            }
            return turbo_str_find(obj, args[0]);
        }
        if (strcmp(method_name, "strip") == 0) {
            if (argc != 0) {
                fprintf(stderr, "TypeError: strip() takes no arguments\n");
                exit(1);
            }
            return turbo_str_strip(obj);
        }
        if (strcmp(method_name, "lstrip") == 0) {
            if (argc != 0) {
                fprintf(stderr, "TypeError: lstrip() takes no arguments\n");
                exit(1);
            }
            return turbo_str_lstrip(obj);
        }
        if (strcmp(method_name, "rstrip") == 0) {
            if (argc != 0) {
                fprintf(stderr, "TypeError: rstrip() takes no arguments\n");
                exit(1);
            }
            return turbo_str_rstrip(obj);
        }
        if (strcmp(method_name, "startswith") == 0) {
            if (argc != 1) {
                fprintf(stderr, "TypeError: startswith() takes exactly one argument\n");
                exit(1);
            }
            return turbo_str_startswith(obj, args[0]);
        }
        if (strcmp(method_name, "endswith") == 0) {
            if (argc != 1) {
                fprintf(stderr, "TypeError: endswith() takes exactly one argument\n");
                exit(1);
            }
            return turbo_str_endswith(obj, args[0]);
        }
        if (strcmp(method_name, "count") == 0) {
            if (argc != 1) {
                fprintf(stderr, "TypeError: count() takes exactly one argument\n");
                exit(1);
            }
            return turbo_str_count(obj, args[0]);
        }
        if (strcmp(method_name, "format") == 0) {
            return turbo_str_format(obj, argc, args);
        }
    }
    if (obj->type == TYPE_DICT) {
        if (strcmp(method_name, "keys") == 0) {
            if (argc != 0) {
                fprintf(stderr, "TypeError: keys() takes no arguments\n");
                exit(1);
            }
            return turbo_dict_keys(obj);
        }
        if (strcmp(method_name, "values") == 0) {
            if (argc != 0) {
                fprintf(stderr, "TypeError: values() takes no arguments\n");
                exit(1);
            }
            return turbo_dict_values(obj);
        }
        if (strcmp(method_name, "items") == 0) {
            if (argc != 0) {
                fprintf(stderr, "TypeError: items() takes no arguments\n");
                exit(1);
            }
            return turbo_dict_items(obj);
        }
        if (strcmp(method_name, "get") == 0) {
            if (argc < 1 || argc > 2) {
                fprintf(stderr, "TypeError: get() takes 1 or 2 arguments\n");
                exit(1);
            }
            TurboObject* default_val = (argc == 2) ? args[1] : turbo_none;
            return turbo_dict_get(obj, args[0], default_val);
        }
        if (strcmp(method_name, "pop") == 0) {
            if (argc != 1) {
                fprintf(stderr, "TypeError: pop() takes exactly one argument\n");
                exit(1);
            }
            return turbo_dict_pop(obj, args[0]);
        }
        if (strcmp(method_name, "popitem") == 0) {
            if (argc != 0) {
                fprintf(stderr, "TypeError: popitem() takes no arguments\n");
                exit(1);
            }
            return turbo_dict_popitem(obj);
        }
        if (strcmp(method_name, "update") == 0) {
            if (argc != 1) {
                fprintf(stderr, "TypeError: update() takes exactly one argument\n");
                exit(1);
            }
            turbo_dict_update(obj, args[0]);
            return turbo_none;
        }
        if (strcmp(method_name, "clear") == 0) {
            if (argc != 0) {
                fprintf(stderr, "TypeError: clear() takes no arguments\n");
                exit(1);
            }
            turbo_dict_clear(obj);
            return turbo_none;
        }
        if (strcmp(method_name, "copy") == 0) {
            if (argc != 0) {
                fprintf(stderr, "TypeError: copy() takes no arguments\n");
                exit(1);
            }
            return turbo_dict_copy(obj);
        }
        if (strcmp(method_name, "setdefault") == 0) {
            if (argc < 1 || argc > 2) {
                fprintf(stderr, "TypeError: setdefault() takes 1 or 2 arguments\n");
                exit(1);
            }
            TurboObject* default_val = (argc == 2) ? args[1] : turbo_none;
            return turbo_dict_setdefault(obj, args[0], default_val);
        }
    }
    if (obj->type == TYPE_SET) {
        if (strcmp(method_name, "add") == 0) {
            if (argc != 1) {
                fprintf(stderr, "TypeError: add() takes exactly one argument\n");
                exit(1);
            }
            turbo_set_add(obj, args[0]);
            return turbo_none;
        }
        if (strcmp(method_name, "remove") == 0) {
            if (argc != 1) {
                fprintf(stderr, "TypeError: remove() takes exactly one argument\n");
                exit(1);
            }
            turbo_set_remove(obj, args[0]);
            return turbo_none;
        }
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
    if (obj->type == TYPE_MODULE) {
        TurboObject* func = turbo_module_get(obj, method_name);
        if (func->type == TYPE_FUNC) {
            return func->func_val.func_ptr(argc, args);
        }
        return turbo_call(func, argc, args);
    }
    fprintf(stderr, "AttributeError: object has no method '%s'\n", method_name);
    exit(1);
}
