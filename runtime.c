#include "runtime.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* --- 64-bit NaN-Boxed / Tagged Value Inline Helpers --- */

static inline bool tv_is_int48(TurboVal v) {
    return ((uint64_t)v & TV_TAG_MASK) == TV_TAG_INT;
}

static inline int64_t tv_get_int48(TurboVal v) {
    return ((int64_t)((uint64_t)v << 16)) >> 16;
}

static inline TurboVal tv_make_int48(int64_t val) {
    return (TurboVal)(TV_TAG_INT | ((uint64_t)val & TV_PAY_MASK));
}

static inline bool tv_is_ptr(TurboVal v) {
    return ((uint64_t)v & TV_TAG_MASK) == TV_TAG_PTR;
}

static inline void* tv_get_ptr(TurboVal v) {
    return (void*)(uintptr_t)((uint64_t)v & TV_PAY_MASK);
}

static inline TurboVal tv_make_ptr(void* p) {
    return (TurboVal)(TV_TAG_PTR | ((uint64_t)(uintptr_t)p & TV_PAY_MASK));
}

static inline bool tv_is_float(TurboVal v) {
    uint64_t u = (uint64_t)v;
    return (u & 0xFFF8000000000000ULL) != 0xFFF8000000000000ULL;
}

static inline double tv_get_float(TurboVal v) {
    union { int64_t i; double d; } u;
    u.i = v;
    return u.d;
}

static inline TurboVal tv_make_float(double d) {
    union { int64_t i; double d; } u;
    u.d = d;
    if (((uint64_t)u.i & 0xFFF8000000000000ULL) == 0xFFF8000000000000ULL) {
        u.i = (int64_t)0x7FF8000000000000ULL;
    }
    return u.i;
}

static inline int32_t tv_heap_tag(TurboVal v) {
    if (!tv_is_ptr(v)) return 0;
    TurboHeapHeader* h = (TurboHeapHeader*)tv_get_ptr(v);
    return h ? h->tag : 0;
}

static inline bool tv_is_int(TurboVal v) {
    return tv_is_int48(v) || tv_heap_tag(v) == HEAP_BIGINT;
}

static inline bool tv_is_bool(TurboVal v) {
    return v == TV_TRUE || v == TV_FALSE;
}

/* --- Globals and Singletons --- */

TurboVal _cached_str_empty = TV_NONE;

TurboVal turbo_none = TV_NONE;
TurboVal turbo_true = TV_TRUE;
TurboVal turbo_false = TV_FALSE;

TurboVal t_print = TV_NONE;
TurboVal t_len = TV_NONE;
TurboVal t_str = TV_NONE;
TurboVal t_int = TV_NONE;
TurboVal t_ord = TV_NONE;
TurboVal t_chr = TV_NONE;
TurboVal t_range = TV_NONE;
TurboVal t_open = TV_NONE;
TurboVal t_sys_argv = TV_NONE;
TurboVal t_input = TV_NONE;
TurboVal t_type = TV_NONE;
TurboVal t_isinstance = TV_NONE;
TurboVal t_hasattr = TV_NONE;
TurboVal t_getattr = TV_NONE;
TurboVal t_setattr = TV_NONE;
TurboVal t_repr = TV_NONE;
TurboVal t_abs = TV_NONE;
TurboVal t_round = TV_NONE;
TurboVal t_pow = TV_NONE;
TurboVal t_hex = TV_NONE;
TurboVal t_bin = TV_NONE;
TurboVal t_oct = TV_NONE;
TurboVal t_float = TV_NONE;
TurboVal t_bool = TV_NONE;
TurboVal t_list = TV_NONE;
TurboVal t_dict = TV_NONE;
TurboVal t_tuple = TV_NONE;
TurboVal t_set = TV_NONE;
TurboVal t_super = TV_NONE;
TurboVal t_iter = TV_NONE;
TurboVal t_next = TV_NONE;
TurboVal t_all = TV_NONE;
TurboVal t_any = TV_NONE;
TurboVal t_sum = TV_NONE;
TurboVal t_min = TV_NONE;
TurboVal t_max = TV_NONE;
TurboVal t_sorted = TV_NONE;
TurboVal t_reversed = TV_NONE;
TurboVal t_enumerate = TV_NONE;
TurboVal t_zip = TV_NONE;
TurboVal t_map = TV_NONE;
TurboVal t_filter = TV_NONE;
TurboVal t_ABC = TV_NONE;
TurboVal t_abstractmethod = TV_NONE;
TurboVal t_dataclass = TV_NONE;

/* Exception stack for LLVM IR / C interop */
#define TURBO_MAX_TRY_DEPTH 64
static jmp_buf _try_stack[TURBO_MAX_TRY_DEPTH];
static int32_t _try_depth = 0;

jmp_buf* turbo_exception_jmp = NULL;
TurboVal turbo_exception_value = TV_NONE;

jmp_buf* turbo_try_push(void) {
    if (_try_depth >= TURBO_MAX_TRY_DEPTH) {
        fprintf(stderr, "RuntimeError: maximum try nesting depth exceeded\n");
        exit(1);
    }
    jmp_buf* buf = &_try_stack[_try_depth++];
    turbo_exception_jmp = buf;
    return buf;
}

void turbo_try_pop(void) {
    if (_try_depth > 0) {
        _try_depth--;
        turbo_exception_jmp = (_try_depth > 0) ? &_try_stack[_try_depth - 1] : NULL;
    } else {
        turbo_exception_jmp = NULL;
    }
}

TurboVal turbo_get_exception(void) {
    return turbo_exception_value;
}

void turbo_assert_fail(TurboVal msg) {
    if (msg != TV_NONE) {
        TurboVal s = turbo_str(msg);
        TurboStr* st = (TurboStr*)tv_get_ptr(s);
        fprintf(stderr, "AssertionError: %s\n", st->chars);
    } else {
        fprintf(stderr, "AssertionError\n");
    }
    exit(1);
}

/* --- Bignum string arithmetic helpers --- */

static void strip_leading_zeros(char* s, int* len) {
    int start = 0;
    while (start < *len - 1 && s[start] == '0') start++;
    if (start > 0) {
        *len -= start;
        memmove(s, s + start, *len);
        s[*len] = '\0';
    }
}

static int abs_cmp(const char* a, int a_len, const char* b, int b_len) {
    if (a_len != b_len) return a_len - b_len;
    for (int i = 0; i < a_len; i++) {
        if (a[i] != b[i]) return (int)(a[i]) - (int)(b[i]);
    }
    return 0;
}

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

static char* str_div(const char* a, int a_len, const char* b, int b_len, int* out_len) {
    if (b_len == 1 && b[0] == '0') {
        fprintf(stderr, "ZeroDivisionError: division by zero\n");
        exit(1);
    }
    if (abs_cmp(a, a_len, b, b_len) < 0) {
        *out_len = 1;
        char* r = (char*)malloc(2);
        r[0] = '0'; r[1] = '\0';
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

static char* str_pow(const char* base, int base_len, const char* exp, int exp_len, int* out_len) {
    if (exp_len == 1 && exp[0] == '0') {
        *out_len = 1;
        char* r = (char*)malloc(2);
        r[0] = '1'; r[1] = '\0';
        return r;
    }
    char* result = (char*)malloc(2);
    result[0] = '1'; result[1] = '\0';
    int res_len = 1;
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

static char* decimal_to_base(const char* dec_digits, int dec_len, int base, int* out_len) {
    if (dec_len == 1 && dec_digits[0] == '0') {
        *out_len = 1;
        char* r = (char*)malloc(2);
        r[0] = '0'; r[1] = '\0';
        return r;
    }
    int max_len = dec_len * 4;
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

    for (int i = 0; i < res_len / 2; i++) {
        char t = result[i];
        result[i] = result[res_len - 1 - i];
        result[res_len - 1 - i] = t;
    }
    result[res_len] = '\0';
    *out_len = res_len;
    return result;
}

static TurboVal make_int_from_str(char* digits, int len, int sign) {
    if (len == 1 && digits[0] == '0') sign = 0;
    if (len <= 14) {
        long long llval = 0;
        for (int i = 0; i < len; i++) {
            llval = llval * 10 + (digits[i] - '0');
        }
        if (sign) llval = -llval;
        if (llval >= TV_MIN_INT48 && llval <= TV_MAX_INT48) {
            free(digits);
            return tv_make_int48(llval);
        }
    }
    TurboBigInt* b = (TurboBigInt*)malloc(sizeof(TurboBigInt));
    b->tag = HEAP_BIGINT;
    b->length = len;
    b->sign = sign;
    b->digits = digits;
    return tv_make_ptr(b);
}

static void int_to_digits(TurboVal v, char* buf, size_t buf_sz, const char** out_digits, int* out_len, int* out_sign) {
    if (tv_is_int48(v)) {
        int64_t val = tv_get_int48(v);
        if (val < 0) {
            *out_sign = 1;
            snprintf(buf, buf_sz, "%lld", (long long)(-val));
        } else {
            *out_sign = 0;
            snprintf(buf, buf_sz, "%lld", (long long)val);
        }
        *out_digits = buf;
        *out_len = (int)strlen(buf);
        return;
    }
    TurboBigInt* b = (TurboBigInt*)tv_get_ptr(v);
    *out_digits = b->digits;
    *out_len = b->length;
    *out_sign = b->sign;
}

/* --- Constructors --- */

TurboVal make_int(const char* str) {
    int sign = 0;
    int slen = (int)strlen(str);
    while (slen > 0 && (str[slen-1] == ' ' || str[slen-1] == '\t' || str[slen-1] == '\n' || str[slen-1] == '\r'))
        slen--;
    const char* p = str;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
    if (*p == '-') { sign = 1; p++; }
    else if (*p == '+') { p++; }
    int len = slen - (int)(p - str);
    if (len <= 0) {
        fprintf(stderr, "ValueError: invalid literal for int()\n");
        exit(1);
    }
    for (int i = 0; i < len; i++) {
        if (p[i] < '0' || p[i] > '9') {
            fprintf(stderr, "ValueError: invalid literal for int()\n");
            exit(1);
        }
    }
    char* digits = (char*)malloc(len + 1);
    memcpy(digits, p, len);
    digits[len] = '\0';
    strip_leading_zeros(digits, &len);
    return make_int_from_str(digits, len, sign);
}

TurboVal make_int_from_ll(long long val) {
    if (val >= TV_MIN_INT48 && val <= TV_MAX_INT48) {
        return tv_make_int48((int64_t)val);
    }
    char buf[32];
    int sign = 0;
    if (val < 0) {
        sign = 1;
        snprintf(buf, sizeof(buf), "%llu", (unsigned long long)(-(val + 1)) + 1ULL);
    } else {
        snprintf(buf, sizeof(buf), "%lld", val);
    }
    int len = (int)strlen(buf);
    char* digits = (char*)malloc(len + 1);
    memcpy(digits, buf, len + 1);
    return make_int_from_str(digits, len, sign);
}

TurboVal make_float(double val) {
    return tv_make_float(val);
}

TurboVal make_complex(double real, double imag) {
    TurboComplex* obj = (TurboComplex*)malloc(sizeof(TurboComplex));
    obj->tag = HEAP_COMPLEX;
    obj->real = real;
    obj->imag = imag;
    return tv_make_ptr(obj);
}

TurboVal make_str(const char* val) {
    TurboStr* obj = (TurboStr*)malloc(sizeof(TurboStr));
    obj->tag = HEAP_STR;
    obj->length = (int32_t)strlen(val);
    obj->chars = (char*)malloc(obj->length + 1);
    memcpy(obj->chars, val, obj->length + 1);
    return tv_make_ptr(obj);
}

TurboVal make_str_len(const char* val, int32_t len) {
    TurboStr* obj = (TurboStr*)malloc(sizeof(TurboStr));
    obj->tag = HEAP_STR;
    obj->length = len;
    obj->chars = (char*)malloc(len + 1);
    memcpy(obj->chars, val, len);
    obj->chars[len] = '\0';
    return tv_make_ptr(obj);
}

TurboVal make_bool(int32_t val) {
    return val ? TV_TRUE : TV_FALSE;
}

TurboVal make_bytes(const unsigned char* data, int32_t length) {
    TurboBytes* obj = (TurboBytes*)malloc(sizeof(TurboBytes));
    obj->tag = HEAP_BYTES;
    obj->length = length;
    obj->data = (unsigned char*)malloc(length > 0 ? length : 1);
    if (length > 0) memcpy(obj->data, data, length);
    return tv_make_ptr(obj);
}

TurboVal make_bytes_from_str(const char* str) {
    int32_t len = (int32_t)strlen(str);
    return make_bytes((const unsigned char*)str, len);
}

TurboVal make_list(void) {
    TurboList* obj = (TurboList*)malloc(sizeof(TurboList));
    obj->tag = HEAP_LIST;
    obj->length = 0;
    obj->capacity = 8;
    obj->items = (TurboVal*)malloc(sizeof(TurboVal) * obj->capacity);
    return tv_make_ptr(obj);
}

TurboVal make_list_len(int64_t len) {
    int32_t n = len > 0 ? (int32_t)len : 0;
    int32_t cap = n > 8 ? n : 8;
    TurboList* obj = (TurboList*)malloc(sizeof(TurboList));
    obj->tag = HEAP_LIST;
    obj->length = n;
    obj->capacity = cap;
    obj->items = (TurboVal*)malloc(sizeof(TurboVal) * cap);
    return tv_make_ptr(obj);
}

TurboVal make_tuple(void) {
    TurboList* obj = (TurboList*)malloc(sizeof(TurboList));
    obj->tag = HEAP_TUPLE;
    obj->length = 0;
    obj->capacity = 8;
    obj->items = (TurboVal*)malloc(sizeof(TurboVal) * obj->capacity);
    return tv_make_ptr(obj);
}

TurboVal make_tuple_from_list(TurboVal list) {
    TurboList* lst = (TurboList*)tv_get_ptr(list);
    TurboList* obj = (TurboList*)malloc(sizeof(TurboList));
    obj->tag = HEAP_TUPLE;
    obj->length = lst->length;
    obj->capacity = lst->length > 0 ? lst->length : 1;
    obj->items = (TurboVal*)malloc(sizeof(TurboVal) * obj->capacity);
    for (int i = 0; i < lst->length; i++) {
        obj->items[i] = lst->items[i];
    }
    return tv_make_ptr(obj);
}

TurboVal make_set(void) {
    TurboSet* obj = (TurboSet*)malloc(sizeof(TurboSet));
    obj->tag = HEAP_SET;
    obj->length = 0;
    obj->capacity = 8;
    obj->ht_cap = 16;
    obj->items = (TurboVal*)malloc(sizeof(TurboVal) * obj->capacity);
    obj->ht_buckets = (int32_t*)malloc(sizeof(int32_t) * obj->ht_cap);
    for (int i = 0; i < obj->ht_cap; i++) obj->ht_buckets[i] = -1;
    return tv_make_ptr(obj);
}

TurboVal make_dict(void) {
    TurboDict* obj = (TurboDict*)malloc(sizeof(TurboDict));
    obj->tag = HEAP_DICT;
    obj->length = 0;
    obj->capacity = 8;
    obj->ht_cap = 16;
    obj->keys = (TurboVal*)malloc(sizeof(TurboVal) * obj->capacity);
    obj->values = (TurboVal*)malloc(sizeof(TurboVal) * obj->capacity);
    obj->ht_buckets = (int32_t*)malloc(sizeof(int32_t) * obj->ht_cap);
    for (int i = 0; i < obj->ht_cap; i++) obj->ht_buckets[i] = -1;
    return tv_make_ptr(obj);
}

TurboVal make_func(TurboCFunction func, const char* name) {
    TurboFunc* obj = (TurboFunc*)malloc(sizeof(TurboFunc));
    obj->tag = HEAP_FUNC;
    obj->func_ptr = func;
    obj->name = strdup(name);
    return tv_make_ptr(obj);
}

TurboVal make_staticmethod(TurboCFunction func, const char* name) {
    TurboFunc* obj = (TurboFunc*)malloc(sizeof(TurboFunc));
    obj->tag = HEAP_STATICMETHOD;
    obj->func_ptr = func;
    obj->name = strdup(name);
    return tv_make_ptr(obj);
}

TurboVal make_classmethod(TurboCFunction func, const char* name) {
    TurboFunc* obj = (TurboFunc*)malloc(sizeof(TurboFunc));
    obj->tag = HEAP_CLASSMETHOD;
    obj->func_ptr = func;
    obj->name = strdup(name);
    return tv_make_ptr(obj);
}

TurboVal make_property(TurboCFunction getter, TurboCFunction setter, const char* name) {
    TurboProperty* obj = (TurboProperty*)malloc(sizeof(TurboProperty));
    obj->tag = HEAP_PROPERTY;
    obj->getter = getter;
    obj->setter = setter;
    obj->name = strdup(name);
    return tv_make_ptr(obj);
}

TurboVal make_class(const char* name) {
    TurboClass* obj = (TurboClass*)malloc(sizeof(TurboClass));
    obj->tag = HEAP_CLASS;
    obj->name = strdup(name);
    obj->method_count = 0;
    obj->method_capacity = 8;
    obj->method_names = (char**)malloc(sizeof(char*) * obj->method_capacity);
    obj->method_funcs = (TurboCFunction*)malloc(sizeof(TurboCFunction) * obj->method_capacity);
    obj->attr_count = 0;
    obj->attr_capacity = 8;
    obj->attr_names = (char**)malloc(sizeof(char*) * obj->attr_capacity);
    obj->attr_values = (TurboVal*)malloc(sizeof(TurboVal) * obj->attr_capacity);
    obj->base_count = 0;
    obj->base_capacity = 4;
    obj->bases = (TurboVal*)malloc(sizeof(TurboVal) * obj->base_capacity);
    obj->abstract_count = 0;
    obj->abstract_names = (char**)malloc(sizeof(char*) * 8);
    return tv_make_ptr(obj);
}

TurboVal make_super(TurboVal cls, TurboVal obj_val) {
    TurboSuper* s = (TurboSuper*)malloc(sizeof(TurboSuper));
    s->tag = HEAP_SUPER;
    s->cls = cls;
    s->obj = obj_val;
    return tv_make_ptr(s);
}

TurboVal make_instance(TurboVal class_obj) {
    TurboInstance* obj = (TurboInstance*)malloc(sizeof(TurboInstance));
    obj->tag = HEAP_INSTANCE;
    obj->class_obj = class_obj;
    obj->length = 0;
    obj->capacity = 8;
    obj->keys = (char**)malloc(sizeof(char*) * obj->capacity);
    obj->values = (TurboVal*)malloc(sizeof(TurboVal) * obj->capacity);
    return tv_make_ptr(obj);
}

TurboVal make_file(FILE* handle) {
    TurboFile* obj = (TurboFile*)malloc(sizeof(TurboFile));
    obj->tag = HEAP_FILE;
    obj->handle = handle;
    return tv_make_ptr(obj);
}

TurboVal make_module(void) {
    TurboModule* obj = (TurboModule*)malloc(sizeof(TurboModule));
    obj->tag = HEAP_MODULE;
    obj->dict = make_dict();
    return tv_make_ptr(obj);
}

double to_double(TurboVal obj) {
    if (tv_is_int48(obj)) return (double)tv_get_int48(obj);
    if (tv_is_float(obj)) return tv_get_float(obj);
    if (tv_heap_tag(obj) == HEAP_BIGINT) return (double)int_to_ll(obj);
    return 0.0;
}

long long int_to_ll(TurboVal obj) {
    if (tv_is_int48(obj)) {
        return (long long)tv_get_int48(obj);
    }
    if (tv_heap_tag(obj) == HEAP_BIGINT) {
        TurboBigInt* b = (TurboBigInt*)tv_get_ptr(obj);
        long long val = 0;
        for (int i = 0; i < b->length; i++) {
            val = val * 10 + (b->digits[i] - '0');
        }
        if (b->sign) val = -val;
        return val;
    }
    return 0;
}

/* --- Equality & Comparisons --- */

static uint64_t tv_hash(TurboVal v) {
    if (tv_is_int48(v)) {
        uint64_t x = (uint64_t)tv_get_int48(v);
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
        return x ^ (x >> 31);
    }
    if (v == TV_TRUE) return tv_hash(tv_make_int48(1));
    if (v == TV_FALSE) return tv_hash(tv_make_int48(0));
    if (v == TV_NONE) return 0x9e3779b97f4a7c15ULL;
    if (tv_is_float(v)) {
        double d = tv_get_float(v);
        if (d == (double)(int64_t)d && d >= (double)TV_MIN_INT48 && d <= (double)TV_MAX_INT48) {
            return tv_hash(tv_make_int48((int64_t)d));
        }
        uint64_t x = (uint64_t)v;
        return x ^ (x >> 32);
    }
    int32_t tag = tv_heap_tag(v);
    if (tag == HEAP_STR) {
        TurboStr* s = (TurboStr*)tv_get_ptr(v);
        uint64_t h = 14695981039346656037ULL;
        for (int i = 0; i < s->length; i++) {
            h ^= (uint8_t)s->chars[i];
            h *= 1099511628211ULL;
        }
        return h;
    }
    if (tag == HEAP_BYTES) {
        TurboBytes* b = (TurboBytes*)tv_get_ptr(v);
        uint64_t h = 14695981039346656037ULL;
        for (int i = 0; i < b->length; i++) {
            h ^= b->data[i];
            h *= 1099511628211ULL;
        }
        return h;
    }
    if (tag == HEAP_TUPLE) {
        TurboList* t = (TurboList*)tv_get_ptr(v);
        uint64_t h = 0x345678ULL;
        for (int i = 0; i < t->length; i++) {
            h = (h ^ tv_hash(t->items[i])) * 1000003ULL + 82520ULL;
        }
        return h;
    }
    if (tag == HEAP_BIGINT) {
        return tv_hash(tv_make_int48(int_to_ll(v)));
    }
    return (uint64_t)v;
}

static bool is_equal(TurboVal a, TurboVal b);
static TurboCFunction class_find_method(TurboVal class_obj, const char* name);
static TurboCFunction class_find_super_method(TurboVal inst_cls, TurboVal cur_cls, const char* name);
static bool class_is_subclass(TurboVal sub_cls, TurboVal target_cls);

static void set_ht_rebuild(TurboSet* st, int32_t new_cap) {
    if (new_cap < 16) new_cap = 16;
    if (st->ht_buckets) free(st->ht_buckets);
    st->ht_cap = new_cap;
    st->ht_buckets = (int32_t*)malloc(sizeof(int32_t) * new_cap);
    for (int i = 0; i < new_cap; i++) st->ht_buckets[i] = -1;
    uint32_t mask = (uint32_t)(new_cap - 1);
    for (int i = 0; i < st->length; i++) {
        uint32_t b = (uint32_t)tv_hash(st->items[i]) & mask;
        while (st->ht_buckets[b] >= 0) {
            b = (b + 1) & mask;
        }
        st->ht_buckets[b] = i;
    }
}

static int32_t set_ht_find(TurboSet* st, TurboVal item, int32_t* out_slot) {
    if (st->ht_cap == 0) set_ht_rebuild(st, 16);
    uint32_t mask = (uint32_t)(st->ht_cap - 1);
    uint32_t b = (uint32_t)tv_hash(item) & mask;
    int32_t first_tomb = -1;
    while (true) {
        int32_t idx = st->ht_buckets[b];
        if (idx == -1) {
            if (out_slot) *out_slot = (first_tomb >= 0) ? first_tomb : (int32_t)b;
            return -1;
        }
        if (idx == -2) {
            if (first_tomb < 0) first_tomb = (int32_t)b;
        } else if (is_equal(st->items[idx], item)) {
            if (out_slot) *out_slot = (int32_t)b;
            return idx;
        }
        b = (b + 1) & mask;
    }
}

static void dict_ht_rebuild(TurboDict* d, int32_t new_cap) {
    if (new_cap < 16) new_cap = 16;
    if (d->ht_buckets) free(d->ht_buckets);
    d->ht_cap = new_cap;
    d->ht_buckets = (int32_t*)malloc(sizeof(int32_t) * new_cap);
    for (int i = 0; i < new_cap; i++) d->ht_buckets[i] = -1;
    uint32_t mask = (uint32_t)(new_cap - 1);
    for (int i = 0; i < d->length; i++) {
        uint32_t b = (uint32_t)tv_hash(d->keys[i]) & mask;
        while (d->ht_buckets[b] >= 0) {
            b = (b + 1) & mask;
        }
        d->ht_buckets[b] = i;
    }
}

static int32_t dict_ht_find(TurboDict* d, TurboVal key, int32_t* out_slot) {
    if (d->ht_cap == 0) dict_ht_rebuild(d, 16);
    uint32_t mask = (uint32_t)(d->ht_cap - 1);
    uint32_t b = (uint32_t)tv_hash(key) & mask;
    int32_t first_tomb = -1;
    while (true) {
        int32_t idx = d->ht_buckets[b];
        if (idx == -1) {
            if (out_slot) *out_slot = (first_tomb >= 0) ? first_tomb : (int32_t)b;
            return -1;
        }
        if (idx == -2) {
            if (first_tomb < 0) first_tomb = (int32_t)b;
        } else if (is_equal(d->keys[idx], key)) {
            if (out_slot) *out_slot = (int32_t)b;
            return idx;
        }
        b = (b + 1) & mask;
    }
}

/* Forward declaration for class method lookup across MRO */
static TurboCFunction class_find_method(TurboVal class_obj, const char* name);

static bool is_equal(TurboVal a, TurboVal b) {
    if (a == b) return true;
    if (tv_is_float(a) && tv_is_int(b)) return tv_get_float(a) == to_double(b);
    if (tv_is_int(a) && tv_is_float(b)) return to_double(a) == tv_get_float(b);
    if (!tv_is_ptr(a) || !tv_is_ptr(b)) return false;

    int32_t ta = tv_heap_tag(a);
    int32_t tb = tv_heap_tag(b);
    if (ta != tb) return false;

    switch (ta) {
        case HEAP_BIGINT: {
            TurboBigInt* ba = (TurboBigInt*)tv_get_ptr(a);
            TurboBigInt* bb = (TurboBigInt*)tv_get_ptr(b);
            return ba->sign == bb->sign && abs_cmp(ba->digits, ba->length, bb->digits, bb->length) == 0;
        }
        case HEAP_COMPLEX: {
            TurboComplex* ca = (TurboComplex*)tv_get_ptr(a);
            TurboComplex* cb = (TurboComplex*)tv_get_ptr(b);
            return ca->real == cb->real && ca->imag == cb->imag;
        }
        case HEAP_STR: {
            TurboStr* sa = (TurboStr*)tv_get_ptr(a);
            TurboStr* sb = (TurboStr*)tv_get_ptr(b);
            if (sa->length != sb->length) return false;
            return memcmp(sa->chars, sb->chars, sa->length) == 0;
        }
        case HEAP_BYTES: {
            TurboBytes* ba = (TurboBytes*)tv_get_ptr(a);
            TurboBytes* bb = (TurboBytes*)tv_get_ptr(b);
            if (ba->length != bb->length) return false;
            return memcmp(ba->data, bb->data, ba->length) == 0;
        }
        case HEAP_LIST:
        case HEAP_TUPLE: {
            TurboList* la = (TurboList*)tv_get_ptr(a);
            TurboList* lb = (TurboList*)tv_get_ptr(b);
            if (la->length != lb->length) return false;
            for (int i = 0; i < la->length; i++) {
                if (!is_equal(la->items[i], lb->items[i])) return false;
            }
            return true;
        }
        case HEAP_SET: {
            TurboSet* sa = (TurboSet*)tv_get_ptr(a);
            TurboSet* sb = (TurboSet*)tv_get_ptr(b);
            if (sa->length != sb->length) return false;
            for (int i = 0; i < sa->length; i++) {
                if (set_ht_find(sb, sa->items[i], NULL) < 0) return false;
            }
            return true;
        }
        case HEAP_DICT: {
            TurboDict* da = (TurboDict*)tv_get_ptr(a);
            TurboDict* db = (TurboDict*)tv_get_ptr(b);
            if (da->length != db->length) return false;
            for (int i = 0; i < da->length; i++) {
                int32_t j = dict_ht_find(db, da->keys[i], NULL);
                if (j < 0 || !is_equal(da->values[i], db->values[j])) return false;
            }
            return true;
        }
        case HEAP_INSTANCE: {
            TurboInstance* ia = (TurboInstance*)tv_get_ptr(a);
            TurboCFunction eq_fn = class_find_method(ia->class_obj, "__eq__");
            if (eq_fn != NULL) {
                TurboVal eq_args[2] = { a, b };
                return turbo_is_truthy(eq_fn(2, eq_args));
            }
            return false;
        }
        default:
            return false;
    }
}

static int int_cmp(TurboVal a, TurboVal b) {
    if (tv_is_int48(a) && tv_is_int48(b)) {
        int64_t va = tv_get_int48(a);
        int64_t vb = tv_get_int48(b);
        return (va < vb) ? -1 : ((va > vb) ? 1 : 0);
    }
    char bufa[32], bufb[32];
    const char *da, *db;
    int la, lb, sa, sb;
    int_to_digits(a, bufa, sizeof(bufa), &da, &la, &sa);
    int_to_digits(b, bufb, sizeof(bufb), &db, &lb, &sb);
    if (sa != sb) return sb - sa;
    int cmp = abs_cmp(da, la, db, lb);
    return sa ? -cmp : cmp;
}

static bool try_op_overload(TurboVal a, TurboVal b, const char* method, TurboVal* out) {
    if (tv_heap_tag(a) == HEAP_INSTANCE) {
        TurboInstance* inst = (TurboInstance*)tv_get_ptr(a);
        TurboCFunction m_fn = class_find_method(inst->class_obj, method);
        if (m_fn != NULL) {
            TurboVal args[2] = { a, b };
            *out = m_fn(2, args);
            return true;
        }
        TurboVal cls_attr = turbo_class_get_attr(inst->class_obj, method);
        if (cls_attr != TV_NONE && tv_heap_tag(cls_attr) == HEAP_STATICMETHOD) {
            TurboFunc* fn = (TurboFunc*)tv_get_ptr(cls_attr);
            TurboVal args[2] = { a, b };
            *out = fn->func_ptr(2, args);
            return true;
        }
    }
    return false;
}

/* --- Arithmetic & Bitwise --- */

TurboVal turbo_add(TurboVal a, TurboVal b) {
    if (tv_is_int48(a) && tv_is_int48(b)) {
        int64_t res = tv_get_int48(a) + tv_get_int48(b);
        if (res >= TV_MIN_INT48 && res <= TV_MAX_INT48) {
            return tv_make_int48(res);
        }
    }
    if (tv_is_int(a) && tv_is_int(b)) {
        char bufa[32], bufb[32];
        const char *da, *db;
        int la, lb, sa, sb;
        int_to_digits(a, bufa, sizeof(bufa), &da, &la, &sa);
        int_to_digits(b, bufb, sizeof(bufb), &db, &lb, &sb);
        int cmp = abs_cmp(da, la, db, lb);
        if (sa == sb) {
            int rlen;
            char* r = str_add(da, la, db, lb, &rlen);
            return make_int_from_str(r, rlen, sa);
        } else {
            const char* big_d = (cmp >= 0) ? da : db;
            int big_len = (cmp >= 0) ? la : lb;
            const char* small_d = (cmp >= 0) ? db : da;
            int small_len = (cmp >= 0) ? lb : la;
            int big_sign = (cmp >= 0) ? sa : sb;
            int rlen;
            char* r = str_sub(big_d, big_len, small_d, small_len, &rlen);
            int res_sign = (rlen == 1 && r[0] == '0') ? 0 : big_sign;
            return make_int_from_str(r, rlen, res_sign);
        }
    }
    if ((tv_is_float(a) || tv_is_int(a)) && (tv_is_float(b) || tv_is_int(b))) {
        return tv_make_float(to_double(a) + to_double(b));
    }
    int32_t ta = tv_heap_tag(a);
    int32_t tb = tv_heap_tag(b);
    if (ta == HEAP_COMPLEX && tb == HEAP_COMPLEX) {
        TurboComplex* ca = (TurboComplex*)tv_get_ptr(a);
        TurboComplex* cb = (TurboComplex*)tv_get_ptr(b);
        return make_complex(ca->real + cb->real, ca->imag + cb->imag);
    }
    if (ta == HEAP_COMPLEX && (tv_is_int(b) || tv_is_float(b))) {
        TurboComplex* ca = (TurboComplex*)tv_get_ptr(a);
        return make_complex(ca->real + to_double(b), ca->imag);
    }
    if ((tv_is_int(a) || tv_is_float(a)) && tb == HEAP_COMPLEX) {
        TurboComplex* cb = (TurboComplex*)tv_get_ptr(b);
        return make_complex(to_double(a) + cb->real, cb->imag);
    }
    if (ta == HEAP_STR && tb == HEAP_STR) {
        TurboStr* sa = (TurboStr*)tv_get_ptr(a);
        TurboStr* sb = (TurboStr*)tv_get_ptr(b);
        int32_t new_len = sa->length + sb->length;
        char* new_chars = (char*)malloc(new_len + 1);
        memcpy(new_chars, sa->chars, sa->length);
        memcpy(new_chars + sa->length, sb->chars, sb->length);
        new_chars[new_len] = '\0';
        TurboVal res = make_str_len(new_chars, new_len);
        free(new_chars);
        return res;
    }
    if (ta == HEAP_LIST && tb == HEAP_LIST) {
        TurboList* la = (TurboList*)tv_get_ptr(a);
        TurboList* lb = (TurboList*)tv_get_ptr(b);
        TurboVal res = make_list();
        for (int i = 0; i < la->length; i++) turbo_list_append(res, la->items[i]);
        for (int i = 0; i < lb->length; i++) turbo_list_append(res, lb->items[i]);
        return res;
    }
    if (ta == HEAP_TUPLE && tb == HEAP_TUPLE) {
        TurboList* la = (TurboList*)tv_get_ptr(a);
        TurboList* lb = (TurboList*)tv_get_ptr(b);
        TurboVal res = make_tuple();
        for (int i = 0; i < la->length; i++) turbo_tuple_append(res, la->items[i]);
        for (int i = 0; i < lb->length; i++) turbo_tuple_append(res, lb->items[i]);
        return res;
    }
    TurboVal ov;
    if (try_op_overload(a, b, "__add__", &ov)) return ov;
    fprintf(stderr, "TypeError: unsupported operand type(s) for +\n");
    exit(1);
}

TurboVal turbo_sub(TurboVal a, TurboVal b) {
    if (tv_is_int48(a) && tv_is_int48(b)) {
        int64_t res = tv_get_int48(a) - tv_get_int48(b);
        if (res >= TV_MIN_INT48 && res <= TV_MAX_INT48) {
            return tv_make_int48(res);
        }
    }
    if (tv_is_int(a) && tv_is_int(b)) {
        char bufa[32], bufb[32];
        const char *da, *db;
        int la, lb, sa, sb;
        int_to_digits(a, bufa, sizeof(bufa), &da, &la, &sa);
        int_to_digits(b, bufb, sizeof(bufb), &db, &lb, &sb);
        int cmp = abs_cmp(da, la, db, lb);
        if (sa != sb) {
            int rlen;
            char* r = str_add(da, la, db, lb, &rlen);
            return make_int_from_str(r, rlen, sa);
        } else {
            int rlen, res_sign;
            char* r;
            if (cmp >= 0) {
                r = str_sub(da, la, db, lb, &rlen);
                res_sign = (rlen == 1 && r[0] == '0') ? 0 : sa;
            } else {
                r = str_sub(db, lb, da, la, &rlen);
                res_sign = (rlen == 1 && r[0] == '0') ? 0 : (1 - sa);
            }
            return make_int_from_str(r, rlen, res_sign);
        }
    }
    if ((tv_is_float(a) || tv_is_int(a)) && (tv_is_float(b) || tv_is_int(b))) {
        return tv_make_float(to_double(a) - to_double(b));
    }
    int32_t ta = tv_heap_tag(a);
    int32_t tb = tv_heap_tag(b);
    if (ta == HEAP_COMPLEX && tb == HEAP_COMPLEX) {
        TurboComplex* ca = (TurboComplex*)tv_get_ptr(a);
        TurboComplex* cb = (TurboComplex*)tv_get_ptr(b);
        return make_complex(ca->real - cb->real, ca->imag - cb->imag);
    }
    if (ta == HEAP_COMPLEX && (tv_is_int(b) || tv_is_float(b))) {
        TurboComplex* ca = (TurboComplex*)tv_get_ptr(a);
        return make_complex(ca->real - to_double(b), ca->imag);
    }
    if ((tv_is_int(a) || tv_is_float(a)) && tb == HEAP_COMPLEX) {
        TurboComplex* cb = (TurboComplex*)tv_get_ptr(b);
        return make_complex(to_double(a) - cb->real, -cb->imag);
    }
    if (ta == HEAP_SET && tb == HEAP_SET) {
        return turbo_set_difference(a, b);
    }
    TurboVal ov;
    if (try_op_overload(a, b, "__sub__", &ov)) return ov;
    fprintf(stderr, "TypeError: unsupported operand type(s) for -\n");
    exit(1);
}

TurboVal turbo_mul(TurboVal a, TurboVal b) {
    if (tv_is_int48(a) && tv_is_int48(b)) {
        long long res;
        if (!__builtin_mul_overflow((long long)tv_get_int48(a), (long long)tv_get_int48(b), &res)) {
            if (res >= TV_MIN_INT48 && res <= TV_MAX_INT48) {
                return tv_make_int48((int64_t)res);
            }
        }
    }
    if (tv_is_int(a) && tv_is_int(b)) {
        char bufa[32], bufb[32];
        const char *da, *db;
        int la, lb, sa, sb;
        int_to_digits(a, bufa, sizeof(bufa), &da, &la, &sa);
        int_to_digits(b, bufb, sizeof(bufb), &db, &lb, &sb);
        int rlen;
        char* r = str_mul(da, la, db, lb, &rlen);
        int sign = (sa != sb) ? 1 : 0;
        if (rlen == 1 && r[0] == '0') sign = 0;
        return make_int_from_str(r, rlen, sign);
    }
    if ((tv_is_float(a) || tv_is_int(a)) && (tv_is_float(b) || tv_is_int(b))) {
        return tv_make_float(to_double(a) * to_double(b));
    }
    int32_t ta = tv_heap_tag(a);
    int32_t tb = tv_heap_tag(b);
    if (ta == HEAP_COMPLEX && tb == HEAP_COMPLEX) {
        TurboComplex* ca = (TurboComplex*)tv_get_ptr(a);
        TurboComplex* cb = (TurboComplex*)tv_get_ptr(b);
        return make_complex(ca->real * cb->real - ca->imag * cb->imag, ca->real * cb->imag + ca->imag * cb->real);
    }
    if (ta == HEAP_COMPLEX && (tv_is_int(b) || tv_is_float(b))) {
        TurboComplex* ca = (TurboComplex*)tv_get_ptr(a);
        double d = to_double(b);
        return make_complex(ca->real * d, ca->imag * d);
    }
    if ((tv_is_int(a) || tv_is_float(a)) && tb == HEAP_COMPLEX) {
        TurboComplex* cb = (TurboComplex*)tv_get_ptr(b);
        double d = to_double(a);
        return make_complex(d * cb->real, d * cb->imag);
    }
    if (ta == HEAP_STR && tv_is_int(b)) {
        TurboStr* sa = (TurboStr*)tv_get_ptr(a);
        int count = (int)int_to_ll(b);
        if (count <= 0) return make_str("");
        int32_t new_len = sa->length * count;
        char* new_chars = (char*)malloc(new_len + 1);
        for (int i = 0; i < count; i++) {
            memcpy(new_chars + i * sa->length, sa->chars, sa->length);
        }
        new_chars[new_len] = '\0';
        TurboVal res = make_str_len(new_chars, new_len);
        free(new_chars);
        return res;
    }
    TurboVal ov;
    if (try_op_overload(a, b, "__mul__", &ov)) return ov;
    fprintf(stderr, "TypeError: unsupported operand type(s) for *\n");
    exit(1);
}

TurboVal turbo_div(TurboVal a, TurboVal b) {
    if ((tv_is_float(a) || tv_is_int(a)) && (tv_is_float(b) || tv_is_int(b))) {
        double db = to_double(b);
        if (db == 0.0) {
            fprintf(stderr, "ZeroDivisionError: division by zero\n");
            exit(1);
        }
        return tv_make_float(to_double(a) / db);
    }
    TurboVal ov;
    if (try_op_overload(a, b, "__truediv__", &ov)) return ov;
    fprintf(stderr, "TypeError: unsupported operand type(s) for /\n");
    exit(1);
}

TurboVal turbo_mod(TurboVal a, TurboVal b) {
    if (tv_is_int48(a) && tv_is_int48(b)) {
        int64_t va = tv_get_int48(a);
        int64_t vb = tv_get_int48(b);
        if (vb == 0) {
            fprintf(stderr, "ZeroDivisionError: integer modulo by zero\n");
            exit(1);
        }
        long long res = llabs((long long)va) % llabs((long long)vb);
        if (va < 0) res = -res;
        return tv_make_int48((int64_t)res);
    }
    if (tv_is_int(a) && tv_is_int(b)) {
        char bufa[32], bufb[32];
        const char *da, *db;
        int la, lb, sa, sb;
        int_to_digits(a, bufa, sizeof(bufa), &da, &la, &sa);
        int_to_digits(b, bufb, sizeof(bufb), &db, &lb, &sb);
        if (lb == 1 && db[0] == '0') {
            fprintf(stderr, "ZeroDivisionError: integer division or modulo by zero\n");
            exit(1);
        }
        int rlen;
        char* r = str_mod(da, la, db, lb, &rlen);
        int sign = sa;
        if (rlen == 1 && r[0] == '0') sign = 0;
        return make_int_from_str(r, rlen, sign);
    }
    TurboVal ov;
    if (try_op_overload(a, b, "__mod__", &ov)) return ov;
    fprintf(stderr, "TypeError: unsupported operand type(s) for %%\n");
    exit(1);
}

TurboVal turbo_pow(TurboVal a, TurboVal b) {
    if (tv_is_int(a) && tv_is_int(b)) {
        char bufa[32], bufb[32];
        const char *da, *db;
        int la, lb, sa, sb;
        int_to_digits(a, bufa, sizeof(bufa), &da, &la, &sa);
        int_to_digits(b, bufb, sizeof(bufb), &db, &lb, &sb);
        if (sb) {
            return tv_make_float(pow(to_double(a), to_double(b)));
        }
        int rlen;
        char* r = str_pow(da, la, db, lb, &rlen);
        int sign = (sa && lb == 1 && db[0] == '0') ? 0 : sa;
        if (sa) {
            int last_digit = db[lb - 1] - '0';
            if (last_digit % 2 == 0) sign = 0;
        }
        if (rlen == 1 && r[0] == '0') sign = 0;
        return make_int_from_str(r, rlen, sign);
    }
    if ((tv_is_float(a) || tv_is_int(a)) && (tv_is_float(b) || tv_is_int(b))) {
        return tv_make_float(pow(to_double(a), to_double(b)));
    }
    TurboVal ov;
    if (try_op_overload(a, b, "__pow__", &ov)) return ov;
    fprintf(stderr, "TypeError: unsupported operand type(s) for **\n");
    exit(1);
}

TurboVal turbo_floordiv(TurboVal a, TurboVal b) {
    if (tv_is_int48(a) && tv_is_int48(b)) {
        int64_t va = tv_get_int48(a);
        int64_t vb = tv_get_int48(b);
        if (vb == 0) {
            fprintf(stderr, "ZeroDivisionError: integer division or modulo by zero\n");
            exit(1);
        }
        long long res = llabs((long long)va) / llabs((long long)vb);
        if ((va < 0) != (vb < 0) && res != 0) res = -res;
        return tv_make_int48((int64_t)res);
    }
    if (tv_is_int(a) && tv_is_int(b)) {
        char bufa[32], bufb[32];
        const char *da, *db;
        int la, lb, sa, sb;
        int_to_digits(a, bufa, sizeof(bufa), &da, &la, &sa);
        int_to_digits(b, bufb, sizeof(bufb), &db, &lb, &sb);
        if (lb == 1 && db[0] == '0') {
            fprintf(stderr, "ZeroDivisionError: integer division or modulo by zero\n");
            exit(1);
        }
        int rlen;
        char* r = str_div(da, la, db, lb, &rlen);
        int sign = (sa != sb) ? 1 : 0;
        if (rlen == 1 && r[0] == '0') sign = 0;
        return make_int_from_str(r, rlen, sign);
    }
    if ((tv_is_float(a) || tv_is_int(a)) && (tv_is_float(b) || tv_is_int(b))) {
        double da = to_double(a);
        double db = to_double(b);
        if (db == 0.0) {
            fprintf(stderr, "ZeroDivisionError: float floor division by zero\n");
            exit(1);
        }
        return tv_make_float(floor(da / db));
    }
    TurboVal ov;
    if (try_op_overload(a, b, "__floordiv__", &ov)) return ov;
    fprintf(stderr, "TypeError: unsupported operand type(s) for //\n");
    exit(1);
}

TurboVal turbo_bitand(TurboVal a, TurboVal b) {
    if (tv_is_int(a) && tv_is_int(b)) {
        return make_int_from_ll(int_to_ll(a) & int_to_ll(b));
    }
    if (tv_heap_tag(a) == HEAP_SET && tv_heap_tag(b) == HEAP_SET) {
        return turbo_set_intersection(a, b);
    }
    TurboVal ov;
    if (try_op_overload(a, b, "__and__", &ov)) return ov;
    fprintf(stderr, "TypeError: unsupported operand type(s) for &\n");
    exit(1);
}

TurboVal turbo_bitor(TurboVal a, TurboVal b) {
    if (tv_is_int(a) && tv_is_int(b)) {
        return make_int_from_ll(int_to_ll(a) | int_to_ll(b));
    }
    int32_t ta = tv_heap_tag(a);
    int32_t tb = tv_heap_tag(b);
    if (ta == HEAP_SET && tb == HEAP_SET) {
        return turbo_set_union(a, b);
    }
    if (ta == HEAP_DICT && tb == HEAP_DICT) {
        TurboVal res = turbo_dict_copy(a);
        turbo_dict_update(res, b);
        return res;
    }
    TurboVal ov;
    if (try_op_overload(a, b, "__or__", &ov)) return ov;
    fprintf(stderr, "TypeError: unsupported operand type(s) for |\n");
    exit(1);
}

TurboVal turbo_bitxor(TurboVal a, TurboVal b) {
    if (tv_is_int(a) && tv_is_int(b)) {
        return make_int_from_ll(int_to_ll(a) ^ int_to_ll(b));
    }
    if (tv_heap_tag(a) == HEAP_SET && tv_heap_tag(b) == HEAP_SET) {
        return turbo_set_symmetric_difference(a, b);
    }
    TurboVal ov;
    if (try_op_overload(a, b, "__xor__", &ov)) return ov;
    fprintf(stderr, "TypeError: unsupported operand type(s) for ^\n");
    exit(1);
}

TurboVal turbo_lshift(TurboVal a, TurboVal b) {
    if (tv_is_int(a) && tv_is_int(b)) {
        return make_int_from_ll(int_to_ll(a) << int_to_ll(b));
    }
    TurboVal ov;
    if (try_op_overload(a, b, "__lshift__", &ov)) return ov;
    fprintf(stderr, "TypeError: unsupported operand type(s) for <<\n");
    exit(1);
}

TurboVal turbo_rshift(TurboVal a, TurboVal b) {
    if (tv_is_int(a) && tv_is_int(b)) {
        return make_int_from_ll(int_to_ll(a) >> int_to_ll(b));
    }
    TurboVal ov;
    if (try_op_overload(a, b, "__rshift__", &ov)) return ov;
    fprintf(stderr, "TypeError: unsupported operand type(s) for >>\n");
    exit(1);
}

TurboVal turbo_matmul(TurboVal a, TurboVal b) {
    if (tv_heap_tag(a) != HEAP_LIST || tv_heap_tag(b) != HEAP_LIST) {
        fprintf(stderr, "TypeError: matrix multiplication (@) requires both arguments to be lists\n");
        exit(1);
    }
    TurboList* la = (TurboList*)tv_get_ptr(a);
    TurboList* lb = (TurboList*)tv_get_ptr(b);
    if (la->length == 0 || lb->length == 0) {
        fprintf(stderr, "ValueError: cannot multiply empty matrices\n");
        exit(1);
    }
    if (tv_heap_tag(la->items[0]) != HEAP_LIST) {
        fprintf(stderr, "TypeError: matrix multiplication (@) requires list of lists\n");
        exit(1);
    }
    TurboList* first_row = (TurboList*)tv_get_ptr(la->items[0]);
    int m = la->length;
    int n = first_row->length;
    for (int i = 1; i < m; i++) {
        if (tv_heap_tag(la->items[i]) != HEAP_LIST || ((TurboList*)tv_get_ptr(la->items[i]))->length != n) {
            fprintf(stderr, "ValueError: matrix A has inconsistent row lengths\n");
            exit(1);
        }
    }
    for (int i = 0; i < lb->length; i++) {
        if (tv_heap_tag(lb->items[i]) != HEAP_LIST) {
            fprintf(stderr, "TypeError: matrix multiplication (@) requires list of lists\n");
            exit(1);
        }
    }
    int p = ((TurboList*)tv_get_ptr(lb->items[0]))->length;
    for (int i = 1; i < lb->length; i++) {
        if (((TurboList*)tv_get_ptr(lb->items[i]))->length != p) {
            fprintf(stderr, "ValueError: matrix B has inconsistent row lengths\n");
            exit(1);
        }
    }
    if (n != lb->length) {
        fprintf(stderr, "ValueError: incompatible dimensions for matrix multiplication: A(%d,%d) and B(%d,%d)\n", m, n, lb->length, p);
        exit(1);
    }
    TurboVal result = make_list();
    for (int i = 0; i < m; i++) {
        TurboVal row = make_list();
        TurboList* row_a = (TurboList*)tv_get_ptr(la->items[i]);
        for (int j = 0; j < p; j++) {
            TurboVal sum = TV_NONE;
            for (int k = 0; k < n; k++) {
                TurboVal a_ik = row_a->items[k];
                TurboList* row_b = (TurboList*)tv_get_ptr(lb->items[k]);
                TurboVal b_kj = row_b->items[j];
                TurboVal prod = turbo_mul(a_ik, b_kj);
                if (k == 0) sum = prod;
                else sum = turbo_add(sum, prod);
            }
            turbo_list_append(row, sum);
        }
        turbo_list_append(result, row);
    }
    return result;
}

TurboVal turbo_bitnot(TurboVal a) {
    if (tv_is_int(a)) {
        return make_int_from_ll(~int_to_ll(a));
    }
    fprintf(stderr, "TypeError: unsupported operand type(s) for ~\n");
    exit(1);
}

/* --- Module Operations --- */

TurboVal turbo_module_get(TurboVal mod, const char* name) {
    if (tv_heap_tag(mod) != HEAP_MODULE) {
        fprintf(stderr, "TypeError: expected module\n");
        exit(1);
    }
    TurboModule* m = (TurboModule*)tv_get_ptr(mod);
    TurboDict* d = (TurboDict*)tv_get_ptr(m->dict);
    TurboVal key = make_str(name);
    for (int i = 0; i < d->length; i++) {
        if (is_equal(d->keys[i], key)) {
            return d->values[i];
        }
    }
    fprintf(stderr, "AttributeError: module has no attribute '%s'\n", name);
    exit(1);
}

void turbo_module_set(TurboVal mod, const char* name, TurboVal val) {
    if (tv_heap_tag(mod) != HEAP_MODULE) {
        fprintf(stderr, "TypeError: expected module\n");
        exit(1);
    }
    TurboModule* m = (TurboModule*)tv_get_ptr(mod);
    turbo_setitem(m->dict, make_str(name), val);
}

TurboVal turbo_import_module(const char* name) {
    (void)name;
    return TV_NONE;
}

/* --- Relational & Logical --- */

TurboVal turbo_eq(TurboVal a, TurboVal b) {
    TurboVal ov;
    if (try_op_overload(a, b, "__eq__", &ov)) return ov;
    return make_bool(is_equal(a, b));
}

TurboVal turbo_ne(TurboVal a, TurboVal b) {
    TurboVal ov;
    if (try_op_overload(a, b, "__ne__", &ov)) return ov;
    return make_bool(!is_equal(a, b));
}

TurboVal turbo_lt(TurboVal a, TurboVal b) {
    TurboVal ov;
    if (try_op_overload(a, b, "__lt__", &ov)) return ov;
    if (tv_is_int48(a) && tv_is_int48(b)) return make_bool(tv_get_int48(a) < tv_get_int48(b));
    if (tv_is_int(a) && tv_is_int(b)) return make_bool(int_cmp(a, b) < 0);
    if ((tv_is_float(a) || tv_is_int(a)) && (tv_is_float(b) || tv_is_int(b))) return make_bool(to_double(a) < to_double(b));
    if (tv_heap_tag(a) == HEAP_STR && tv_heap_tag(b) == HEAP_STR) {
        return make_bool(strcmp(((TurboStr*)tv_get_ptr(a))->chars, ((TurboStr*)tv_get_ptr(b))->chars) < 0);
    }
    if (tv_heap_tag(a) == HEAP_SET && tv_heap_tag(b) == HEAP_SET) {
        TurboSet* sa = (TurboSet*)tv_get_ptr(a);
        TurboSet* sb = (TurboSet*)tv_get_ptr(b);
        return make_bool(sa->length < sb->length && turbo_set_issubset(a, b) == TV_TRUE);
    }
    fprintf(stderr, "TypeError: '<' not supported between types\n");
    exit(1);
}

TurboVal turbo_gt(TurboVal a, TurboVal b) {
    TurboVal ov;
    if (try_op_overload(a, b, "__gt__", &ov)) return ov;
    if (tv_is_int48(a) && tv_is_int48(b)) return make_bool(tv_get_int48(a) > tv_get_int48(b));
    if (tv_is_int(a) && tv_is_int(b)) return make_bool(int_cmp(a, b) > 0);
    if ((tv_is_float(a) || tv_is_int(a)) && (tv_is_float(b) || tv_is_int(b))) return make_bool(to_double(a) > to_double(b));
    if (tv_heap_tag(a) == HEAP_STR && tv_heap_tag(b) == HEAP_STR) {
        return make_bool(strcmp(((TurboStr*)tv_get_ptr(a))->chars, ((TurboStr*)tv_get_ptr(b))->chars) > 0);
    }
    if (tv_heap_tag(a) == HEAP_SET && tv_heap_tag(b) == HEAP_SET) {
        TurboSet* sa = (TurboSet*)tv_get_ptr(a);
        TurboSet* sb = (TurboSet*)tv_get_ptr(b);
        return make_bool(sa->length > sb->length && turbo_set_issuperset(a, b) == TV_TRUE);
    }
    fprintf(stderr, "TypeError: '>' not supported between types\n");
    exit(1);
}

TurboVal turbo_lte(TurboVal a, TurboVal b) {
    TurboVal ov;
    if (try_op_overload(a, b, "__le__", &ov)) return ov;
    if (tv_is_int48(a) && tv_is_int48(b)) return make_bool(tv_get_int48(a) <= tv_get_int48(b));
    if (tv_is_int(a) && tv_is_int(b)) return make_bool(int_cmp(a, b) <= 0);
    if ((tv_is_float(a) || tv_is_int(a)) && (tv_is_float(b) || tv_is_int(b))) return make_bool(to_double(a) <= to_double(b));
    if (tv_heap_tag(a) == HEAP_STR && tv_heap_tag(b) == HEAP_STR) {
        return make_bool(strcmp(((TurboStr*)tv_get_ptr(a))->chars, ((TurboStr*)tv_get_ptr(b))->chars) <= 0);
    }
    if (tv_heap_tag(a) == HEAP_SET && tv_heap_tag(b) == HEAP_SET) {
        return turbo_set_issubset(a, b);
    }
    fprintf(stderr, "TypeError: '<=' not supported between types\n");
    exit(1);
}

TurboVal turbo_gte(TurboVal a, TurboVal b) {
    TurboVal ov;
    if (try_op_overload(a, b, "__ge__", &ov)) return ov;
    if (tv_is_int48(a) && tv_is_int48(b)) return make_bool(tv_get_int48(a) >= tv_get_int48(b));
    if (tv_is_int(a) && tv_is_int(b)) return make_bool(int_cmp(a, b) >= 0);
    if ((tv_is_float(a) || tv_is_int(a)) && (tv_is_float(b) || tv_is_int(b))) return make_bool(to_double(a) >= to_double(b));
    if (tv_heap_tag(a) == HEAP_STR && tv_heap_tag(b) == HEAP_STR) {
        return make_bool(strcmp(((TurboStr*)tv_get_ptr(a))->chars, ((TurboStr*)tv_get_ptr(b))->chars) >= 0);
    }
    if (tv_heap_tag(a) == HEAP_SET && tv_heap_tag(b) == HEAP_SET) {
        return turbo_set_issuperset(a, b);
    }
    fprintf(stderr, "TypeError: '>=' not supported between types\n");
    exit(1);
}

TurboVal turbo_not(TurboVal val) {
    return make_bool(!turbo_is_truthy(val));
}

TurboVal turbo_and(TurboVal a, TurboVal b) {
    return turbo_is_truthy(a) ? b : a;
}

TurboVal turbo_or(TurboVal a, TurboVal b) {
    return turbo_is_truthy(a) ? a : b;
}

int32_t turbo_is_truthy(TurboVal val) {
    if (val == TV_NONE || val == TV_FALSE) return 0;
    if (val == TV_TRUE) return 1;
    if (tv_is_int48(val)) return tv_get_int48(val) != 0;
    if (tv_is_float(val)) return tv_get_float(val) != 0.0;
    if (!tv_is_ptr(val)) return 0;

    int32_t tag = tv_heap_tag(val);
    switch (tag) {
        case HEAP_BIGINT: {
            TurboBigInt* b = (TurboBigInt*)tv_get_ptr(val);
            return !(b->length == 1 && b->digits[0] == '0');
        }
        case HEAP_COMPLEX: {
            TurboComplex* c = (TurboComplex*)tv_get_ptr(val);
            return c->real != 0.0 || c->imag != 0.0;
        }
        case HEAP_STR:
        case HEAP_BYTES:
        case HEAP_LIST:
        case HEAP_TUPLE:
        case HEAP_SET:
        case HEAP_DICT: {
            TurboHeapHeader* h = (TurboHeapHeader*)tv_get_ptr(val);
            return h->length > 0;
        }
        default:
            return 1;
    }
}

/* --- Subscript, Slice, Attribute --- */

TurboVal turbo_len(TurboVal val) {
    int32_t tag = tv_heap_tag(val);
    if (tag == HEAP_STR || tag == HEAP_BYTES || tag == HEAP_LIST ||
        tag == HEAP_TUPLE || tag == HEAP_SET || tag == HEAP_DICT) {
        TurboHeapHeader* h = (TurboHeapHeader*)tv_get_ptr(val);
        return tv_make_int48(h->length);
    }
    fprintf(stderr, "TypeError: object of type has no len()\n");
    exit(1);
}

TurboVal turbo_getitem(TurboVal obj, TurboVal key) {
    int32_t tag = tv_heap_tag(obj);
    if (tag == HEAP_LIST || tag == HEAP_TUPLE) {
        if (!tv_is_int(key)) {
            fprintf(stderr, "TypeError: indices must be integers\n");
            exit(1);
        }
        TurboList* lst = (TurboList*)tv_get_ptr(obj);
        long long idx = int_to_ll(key);
        long long orig_idx = idx;
        if (idx < 0) idx += lst->length;
        if (idx < 0 || idx >= lst->length) {
            fprintf(stderr, "IndexError: index out of range: index %lld (original %lld), length %d\n", idx, orig_idx, lst->length);
            exit(1);
        }
        return lst->items[idx];
    }
    if (tag == HEAP_STR) {
        if (!tv_is_int(key)) {
            fprintf(stderr, "TypeError: string indices must be integers\n");
            exit(1);
        }
        TurboStr* st = (TurboStr*)tv_get_ptr(obj);
        long long idx = int_to_ll(key);
        long long orig_idx = idx;
        if (idx < 0) idx += st->length;
        if (idx < 0 || idx >= st->length) {
            fprintf(stderr, "IndexError: string index out of range: index %lld (original %lld), length %d\n", idx, orig_idx, st->length);
            exit(1);
        }
        char temp[2] = { st->chars[idx], '\0' };
        return make_str_len(temp, 1);
    }
    if (tag == HEAP_BYTES) {
        if (!tv_is_int(key)) {
            fprintf(stderr, "TypeError: bytes indices must be integers\n");
            exit(1);
        }
        TurboBytes* by = (TurboBytes*)tv_get_ptr(obj);
        long long idx = int_to_ll(key);
        long long orig_idx = idx;
        if (idx < 0) idx += by->length;
        if (idx < 0 || idx >= by->length) {
            fprintf(stderr, "IndexError: bytes index out of range: index %lld (original %lld), length %d\n", idx, orig_idx, by->length);
            exit(1);
        }
        return tv_make_int48(by->data[idx]);
    }
    if (tag == HEAP_DICT) {
        TurboDict* d = (TurboDict*)tv_get_ptr(obj);
        int32_t idx = dict_ht_find(d, key, NULL);
        if (idx >= 0) return d->values[idx];
        fprintf(stderr, "KeyError: key not found in dict\n");
        exit(1);
    }
    fprintf(stderr, "TypeError: object is not subscriptable\n");
    exit(1);
}

void turbo_setitem(TurboVal obj, TurboVal key, TurboVal val) {
    int32_t tag = tv_heap_tag(obj);
    if (tag == HEAP_LIST) {
        if (!tv_is_int(key)) {
            fprintf(stderr, "TypeError: list indices must be integers\n");
            exit(1);
        }
        TurboList* lst = (TurboList*)tv_get_ptr(obj);
        long long idx = int_to_ll(key);
        if (idx < 0) idx += lst->length;
        if (idx < 0 || idx >= lst->length) {
            fprintf(stderr, "IndexError: list assignment index out of range\n");
            exit(1);
        }
        lst->items[idx] = val;
        return;
    }
    if (tag == HEAP_DICT) {
        TurboDict* d = (TurboDict*)tv_get_ptr(obj);
        if ((d->length + 1) * 2 > d->ht_cap) {
            dict_ht_rebuild(d, d->ht_cap * 2);
        }
        int32_t slot = -1;
        int32_t idx = dict_ht_find(d, key, &slot);
        if (idx >= 0) {
            d->values[idx] = val;
            return;
        }
        if (d->length >= d->capacity) {
            d->capacity *= 2;
            d->keys = (TurboVal*)realloc(d->keys, sizeof(TurboVal) * d->capacity);
            d->values = (TurboVal*)realloc(d->values, sizeof(TurboVal) * d->capacity);
        }
        int32_t new_idx = d->length;
        d->keys[new_idx] = key;
        d->values[new_idx] = val;
        d->length++;
        d->ht_buckets[slot] = new_idx;
        return;
    }
    fprintf(stderr, "TypeError: object does not support item assignment\n");
    exit(1);
}

void turbo_delitem(TurboVal obj, TurboVal key) {
    int32_t tag = tv_heap_tag(obj);
    if (tag == HEAP_LIST) {
        if (!tv_is_int(key)) {
            fprintf(stderr, "TypeError: list indices must be integers\n");
            exit(1);
        }
        TurboList* lst = (TurboList*)tv_get_ptr(obj);
        long long idx = int_to_ll(key);
        if (idx < 0) idx += lst->length;
        if (idx < 0 || idx >= lst->length) {
            fprintf(stderr, "IndexError: list index out of range\n");
            exit(1);
        }
        for (long long i = idx; i < lst->length - 1; i++) {
            lst->items[i] = lst->items[i + 1];
        }
        lst->length--;
        return;
    }
    if (tag == HEAP_DICT) {
        TurboDict* d = (TurboDict*)tv_get_ptr(obj);
        int32_t idx = dict_ht_find(d, key, NULL);
        if (idx >= 0) {
            for (int j = idx; j < d->length - 1; j++) {
                d->keys[j] = d->keys[j + 1];
                d->values[j] = d->values[j + 1];
            }
            d->length--;
            dict_ht_rebuild(d, d->ht_cap);
            return;
        }
        fprintf(stderr, "KeyError: key not found in dict\n");
        exit(1);
    }
    fprintf(stderr, "TypeError: object does not support item deletion\n");
    exit(1);
}

TurboVal turbo_slice(TurboVal obj, TurboVal start, TurboVal end) {
    int32_t tag = tv_heap_tag(obj);
    if (tag != HEAP_STR && tag != HEAP_BYTES && tag != HEAP_LIST && tag != HEAP_TUPLE) {
        fprintf(stderr, "TypeError: unsliceable type\n");
        exit(1);
    }
    TurboHeapHeader* h = (TurboHeapHeader*)tv_get_ptr(obj);
    long long len = h->length;

    long long s = (start == TV_NONE) ? 0 : int_to_ll(start);
    long long e = (end == TV_NONE) ? len : int_to_ll(end);

    if (s < 0) s += len;
    if (s < 0) s = 0;
    if (s > len) s = len;

    if (e < 0) e += len;
    if (e < 0) e = 0;
    if (e > len) e = len;

    if (s >= e) {
        if (tag == HEAP_STR) return make_str("");
        if (tag == HEAP_BYTES) return make_bytes((const unsigned char*)"", 0);
        if (tag == HEAP_TUPLE) return make_tuple();
        return make_list();
    }

    int32_t slice_len = (int32_t)(e - s);
    if (tag == HEAP_STR) {
        TurboStr* st = (TurboStr*)tv_get_ptr(obj);
        return make_str_len(st->chars + s, slice_len);
    }
    if (tag == HEAP_BYTES) {
        TurboBytes* by = (TurboBytes*)tv_get_ptr(obj);
        return make_bytes(by->data + s, slice_len);
    }
    TurboList* lst = (TurboList*)tv_get_ptr(obj);
    if (tag == HEAP_TUPLE) {
        TurboVal new_tup = make_tuple();
        for (long long i = s; i < e; i++) turbo_tuple_append(new_tup, lst->items[i]);
        return new_tup;
    } else {
        TurboVal new_lst = make_list();
        for (long long i = s; i < e; i++) turbo_list_append(new_lst, lst->items[i]);
        return new_lst;
    }
}

TurboVal turbo_getattr(TurboVal obj, const char* name) {
    int32_t tag = tv_heap_tag(obj);
    if (tag == HEAP_INSTANCE) {
        TurboInstance* inst = (TurboInstance*)tv_get_ptr(obj);
        for (int i = 0; i < inst->length; i++) {
            if (strcmp(inst->keys[i], name) == 0) {
                return inst->values[i];
            }
        }
        TurboVal cls_attr = turbo_class_get_attr(inst->class_obj, name);
        if (cls_attr != TV_NONE) {
            int32_t atag = tv_heap_tag(cls_attr);
            if (atag == HEAP_PROPERTY) {
                TurboProperty* prop = (TurboProperty*)tv_get_ptr(cls_attr);
                TurboVal prop_args[1] = { obj };
                return prop->getter(1, prop_args);
            }
            if (atag == HEAP_STATICMETHOD || atag == HEAP_CLASSMETHOD) {
                TurboFunc* fn = (TurboFunc*)tv_get_ptr(cls_attr);
                return make_func(fn->func_ptr, name);
            }
            return cls_attr;
        }
        TurboCFunction m_fn = class_find_method(inst->class_obj, name);
        if (m_fn != NULL) {
            return make_func(m_fn, name);
        }
        TurboClass* cls = (TurboClass*)tv_get_ptr(inst->class_obj);
        fprintf(stderr, "AttributeError: '%s' object has no attribute '%s'\n", cls->name, name);
        exit(1);
    }
    if (tag == HEAP_CLASS) {
        TurboClass* cls = (TurboClass*)tv_get_ptr(obj);
        TurboCFunction m_fn = class_find_method(obj, name);
        if (m_fn != NULL) {
            return make_func(m_fn, name);
        }
        TurboVal cls_attr = turbo_class_get_attr(obj, name);
        if (cls_attr != TV_NONE) {
            int32_t atag = tv_heap_tag(cls_attr);
            if (atag == HEAP_STATICMETHOD || atag == HEAP_CLASSMETHOD) {
                TurboFunc* fn = (TurboFunc*)tv_get_ptr(cls_attr);
                return make_func(fn->func_ptr, name);
            }
            if (atag == HEAP_PROPERTY) {
                fprintf(stderr, "AttributeError: property '%s' cannot be accessed on class\n", name);
                exit(1);
            }
            return cls_attr;
        }
        fprintf(stderr, "AttributeError: class '%s' has no attribute '%s'\n", cls->name, name);
        exit(1);
    }
    if (tag == HEAP_MODULE) {
        return turbo_module_get(obj, name);
    }
    if (tag == HEAP_SUPER) {
        TurboSuper* sup = (TurboSuper*)tv_get_ptr(obj);
        if (tv_heap_tag(sup->obj) == HEAP_INSTANCE) {
            TurboInstance* inst = (TurboInstance*)tv_get_ptr(sup->obj);
            TurboCFunction m_fn = class_find_super_method(inst->class_obj, sup->cls, name);
            if (m_fn == NULL) m_fn = class_find_method(inst->class_obj, name);
            if (m_fn != NULL) {
                return make_func(m_fn, name);
            }
        }
        fprintf(stderr, "AttributeError: 'super' object has no attribute '%s'\n", name);
        exit(1);
    }
    fprintf(stderr, "AttributeError: object has no attributes\n");
    exit(1);
}

void turbo_setattr(TurboVal obj, const char* name, TurboVal val) {
    int32_t tag = tv_heap_tag(obj);
    if (tag == HEAP_INSTANCE) {
        TurboInstance* inst = (TurboInstance*)tv_get_ptr(obj);
        TurboVal cls_attr = turbo_class_get_attr(inst->class_obj, name);
        if (cls_attr != TV_NONE && tv_heap_tag(cls_attr) == HEAP_PROPERTY) {
            TurboProperty* prop = (TurboProperty*)tv_get_ptr(cls_attr);
            if (prop->setter != NULL) {
                TurboVal set_args[2] = { obj, val };
                prop->setter(2, set_args);
                return;
            }
        }
        for (int i = 0; i < inst->length; i++) {
            if (strcmp(inst->keys[i], name) == 0) {
                inst->values[i] = val;
                return;
            }
        }
        if (inst->length >= inst->capacity) {
            inst->capacity *= 2;
            inst->keys = (char**)realloc(inst->keys, sizeof(char*) * inst->capacity);
            inst->values = (TurboVal*)realloc(inst->values, sizeof(TurboVal) * inst->capacity);
        }
        inst->keys[inst->length] = strdup(name);
        inst->values[inst->length] = val;
        inst->length++;
        return;
    }
    if (tag == HEAP_CLASS) {
        turbo_class_set_attr(obj, name, val);
        return;
    }
    if (tag == HEAP_MODULE) {
        turbo_module_set(obj, name, val);
        return;
    }
    fprintf(stderr, "AttributeError: cannot set attribute on non-instance\n");
    exit(1);
}

int32_t turbo_in(TurboVal item, TurboVal container) {
    int32_t tag = tv_heap_tag(container);
    if (tag == HEAP_SET) {
        TurboSet* st = (TurboSet*)tv_get_ptr(container);
        return set_ht_find(st, item, NULL) >= 0 ? 1 : 0;
    }
    if (tag == HEAP_LIST || tag == HEAP_TUPLE) {
        TurboList* lst = (TurboList*)tv_get_ptr(container);
        for (int i = 0; i < lst->length; i++) {
            if (is_equal(item, lst->items[i])) return 1;
        }
        return 0;
    }
    if (tag == HEAP_DICT) {
        TurboDict* d = (TurboDict*)tv_get_ptr(container);
        return dict_ht_find(d, item, NULL) >= 0 ? 1 : 0;
    }
    if (tag == HEAP_STR) {
        if (tv_heap_tag(item) != HEAP_STR) {
            fprintf(stderr, "TypeError: 'in <string>' requires string as left operand\n");
            exit(1);
        }
        TurboStr* sc = (TurboStr*)tv_get_ptr(container);
        TurboStr* si = (TurboStr*)tv_get_ptr(item);
        return strstr(sc->chars, si->chars) != NULL ? 1 : 0;
    }
    if (tag == HEAP_BYTES) {
        if (!tv_is_int(item)) {
            fprintf(stderr, "TypeError: 'in <bytes>' requires int as left operand\n");
            exit(1);
        }
        TurboBytes* by = (TurboBytes*)tv_get_ptr(container);
        int byte_val = (int)int_to_ll(item);
        for (int i = 0; i < by->length; i++) {
            if (by->data[i] == byte_val) return 1;
        }
        return 0;
    }
    fprintf(stderr, "TypeError: argument of type is not iterable\n");
    exit(1);
}

/* --- String Formatting & Conversion --- */

static TurboVal bytes_to_turbo_str(TurboBytes* val) {
    int max_len = 3;
    for (int i = 0; i < val->length; i++) {
        unsigned char c = val->data[i];
        if (c == '\'' || c == '\\') max_len += 2;
        else if (c >= 0x20 && c <= 0x7e) max_len += 1;
        else max_len += 4;
    }
    char* buf = (char*)malloc(max_len + 1);
    int pos = 0;
    buf[pos++] = 'b';
    buf[pos++] = '\'';
    for (int i = 0; i < val->length; i++) {
        unsigned char c = val->data[i];
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
    TurboVal res = make_str(buf);
    free(buf);
    return res;
}

TurboVal turbo_str(TurboVal val) {
    char buf[128];
    if (val == TV_NONE) return make_str("None");
    if (val == TV_TRUE) return make_str("True");
    if (val == TV_FALSE) return make_str("False");
    if (tv_is_int48(val)) {
        snprintf(buf, sizeof(buf), "%lld", (long long)tv_get_int48(val));
        return make_str(buf);
    }
    if (tv_is_float(val)) {
        snprintf(buf, sizeof(buf), "%g", tv_get_float(val));
        return make_str(buf);
    }

    int32_t tag = tv_heap_tag(val);
    switch (tag) {
        case HEAP_BIGINT: {
            TurboBigInt* b = (TurboBigInt*)tv_get_ptr(val);
            const char* sign_str = b->sign ? "-" : "";
            char* res = (char*)malloc(b->length + 2);
            sprintf(res, "%s%s", sign_str, b->digits);
            TurboVal str_obj = make_str(res);
            free(res);
            return str_obj;
        }
        case HEAP_COMPLEX: {
            TurboComplex* c = (TurboComplex*)tv_get_ptr(val);
            if (c->imag >= 0) {
                snprintf(buf, sizeof(buf), "(%g+%gj)", c->real, c->imag);
            } else {
                snprintf(buf, sizeof(buf), "(%g%gj)", c->real, c->imag);
            }
            return make_str(buf);
        }
        case HEAP_STR:
            return val;
        case HEAP_BYTES:
            return bytes_to_turbo_str((TurboBytes*)tv_get_ptr(val));
        case HEAP_LIST: {
            TurboList* lst = (TurboList*)tv_get_ptr(val);
            char* s = strdup("[");
            for (int i = 0; i < lst->length; i++) {
                TurboVal item_str = turbo_str(lst->items[i]);
                TurboStr* ist = (TurboStr*)tv_get_ptr(item_str);
                int new_len = (int)strlen(s) + ist->length + 3;
                char* s2 = (char*)malloc(new_len);
                strcpy(s2, s);
                strcat(s2, ist->chars);
                if (i < lst->length - 1) strcat(s2, ", ");
                free(s);
                s = s2;
            }
            char* s3 = (char*)malloc(strlen(s) + 2);
            strcpy(s3, s);
            strcat(s3, "]");
            free(s);
            TurboVal res = make_str(s3);
            free(s3);
            return res;
        }
        case HEAP_TUPLE: {
            TurboList* lst = (TurboList*)tv_get_ptr(val);
            char* s = strdup("(");
            for (int i = 0; i < lst->length; i++) {
                TurboVal item_str = turbo_str(lst->items[i]);
                TurboStr* ist = (TurboStr*)tv_get_ptr(item_str);
                int new_len = (int)strlen(s) + ist->length + 3;
                char* s2 = (char*)malloc(new_len);
                strcpy(s2, s);
                strcat(s2, ist->chars);
                if (i < lst->length - 1) strcat(s2, ", ");
                free(s);
                s = s2;
            }
            char* s3 = (char*)malloc(strlen(s) + 3);
            strcpy(s3, s);
            if (lst->length == 1) strcat(s3, ",");
            strcat(s3, ")");
            free(s);
            TurboVal res = make_str(s3);
            free(s3);
            return res;
        }
        case HEAP_SET: {
            TurboList* lst = (TurboList*)tv_get_ptr(val);
            char* s = strdup("{");
            for (int i = 0; i < lst->length; i++) {
                TurboVal item_str = turbo_str(lst->items[i]);
                TurboStr* ist = (TurboStr*)tv_get_ptr(item_str);
                int new_len = (int)strlen(s) + ist->length + 3;
                char* s2 = (char*)malloc(new_len);
                strcpy(s2, s);
                strcat(s2, ist->chars);
                if (i < lst->length - 1) strcat(s2, ", ");
                free(s);
                s = s2;
            }
            char* s3 = (char*)malloc(strlen(s) + 2);
            strcpy(s3, s);
            strcat(s3, "}");
            free(s);
            TurboVal res = make_str(s3);
            free(s3);
            return res;
        }
        case HEAP_DICT: {
            TurboDict* d = (TurboDict*)tv_get_ptr(val);
            char* s = strdup("{");
            for (int i = 0; i < d->length; i++) {
                TurboVal key_str = turbo_str(d->keys[i]);
                TurboVal val_str = turbo_str(d->values[i]);
                TurboStr* kst = (TurboStr*)tv_get_ptr(key_str);
                TurboStr* vst = (TurboStr*)tv_get_ptr(val_str);
                int new_len = (int)strlen(s) + kst->length + vst->length + 5;
                char* s2 = (char*)malloc(new_len);
                strcpy(s2, s);
                strcat(s2, kst->chars);
                strcat(s2, ": ");
                strcat(s2, vst->chars);
                if (i < d->length - 1) strcat(s2, ", ");
                free(s);
                s = s2;
            }
            char* s3 = (char*)malloc(strlen(s) + 2);
            strcpy(s3, s);
            strcat(s3, "}");
            free(s);
            TurboVal res = make_str(s3);
            free(s3);
            return res;
        }
        case HEAP_MODULE:
            return make_str("<module>");
        case HEAP_SUPER: {
            TurboSuper* sup = (TurboSuper*)tv_get_ptr(val);
            if (tv_heap_tag(sup->obj) == HEAP_INSTANCE) {
                TurboInstance* inst = (TurboInstance*)tv_get_ptr(sup->obj);
                TurboClass* cls = (TurboClass*)tv_get_ptr(inst->class_obj);
                char* s = (char*)malloc(strlen(cls->name) + 12);
                sprintf(s, "<super: %s>", cls->name);
                TurboVal res = make_str(s);
                free(s);
                return res;
            }
            return make_str("<super>");
        }
        case HEAP_INSTANCE: {
            TurboInstance* inst = (TurboInstance*)tv_get_ptr(val);
            TurboClass* cls = (TurboClass*)tv_get_ptr(inst->class_obj);
            TurboVal cls_attr = turbo_class_get_attr(inst->class_obj, "__str__");
            if (cls_attr != TV_NONE && tv_heap_tag(cls_attr) == HEAP_STATICMETHOD) {
                TurboFunc* fn = (TurboFunc*)tv_get_ptr(cls_attr);
                TurboVal args[1] = { val };
                TurboVal res = fn->func_ptr(1, args);
                if (tv_heap_tag(res) == HEAP_STR) return res;
            }
            TurboCFunction str_fn = class_find_method(inst->class_obj, "__str__");
            if (str_fn != NULL) {
                TurboVal m_args[1] = { val };
                TurboVal res = str_fn(1, m_args);
                if (tv_heap_tag(res) == HEAP_STR) return res;
            }
            char* s = (char*)malloc(strlen(cls->name) + 16);
            sprintf(s, "<%s object>", cls->name);
            TurboVal res = make_str(s);
            free(s);
            return res;
        }
        default:
            return make_str("<object>");
    }
}

TurboVal turbo_repr(TurboVal val) {
    int32_t tag = tv_heap_tag(val);
    if (tag == HEAP_STR) {
        TurboStr* st = (TurboStr*)tv_get_ptr(val);
        int len = st->length;
        char* buf = (char*)malloc(len + 3);
        buf[0] = '"';
        memcpy(buf + 1, st->chars, len);
        buf[len + 1] = '"';
        buf[len + 2] = '\0';
        TurboVal res = make_str_len(buf, len + 2);
        free(buf);
        return res;
    }
    if (tag == HEAP_INSTANCE) {
        TurboInstance* inst = (TurboInstance*)tv_get_ptr(val);
        TurboVal cls_attr = turbo_class_get_attr(inst->class_obj, "__repr__");
        if (cls_attr != TV_NONE && tv_heap_tag(cls_attr) == HEAP_STATICMETHOD) {
            TurboFunc* fn = (TurboFunc*)tv_get_ptr(cls_attr);
            TurboVal args[1] = { val };
            TurboVal res = fn->func_ptr(1, args);
            if (tv_heap_tag(res) == HEAP_STR) return res;
        }
        TurboCFunction repr_fn = class_find_method(inst->class_obj, "__repr__");
        if (repr_fn != NULL) {
            TurboVal m_args[1] = { val };
            TurboVal res = repr_fn(1, m_args);
            if (tv_heap_tag(res) == HEAP_STR) return res;
        }
    }
    return turbo_str(val);
}

/* --- Core Built-ins --- */

TurboVal turbo_print(int32_t argc, TurboVal* args) {
    for (int i = 0; i < argc; i++) {
        TurboVal str_val = turbo_str(args[i]);
        TurboStr* st = (TurboStr*)tv_get_ptr(str_val);
        printf("%s", st->chars);
        if (i < argc - 1) printf(" ");
    }
    printf("\n");
    return TV_NONE;
}

TurboVal turbo_int(TurboVal val) {
    if (tv_is_int(val)) return val;
    if (tv_is_bool(val)) return tv_make_int48(val == TV_TRUE ? 1 : 0);
    if (tv_is_float(val)) return make_int_from_ll((long long)tv_get_float(val));
    int32_t tag = tv_heap_tag(val);
    if (tag == HEAP_STR) {
        TurboStr* st = (TurboStr*)tv_get_ptr(val);
        return make_int(st->chars);
    }
    if (tag == HEAP_BYTES) {
        TurboBytes* by = (TurboBytes*)tv_get_ptr(val);
        char* temp = (char*)malloc(by->length + 1);
        memcpy(temp, by->data, by->length);
        temp[by->length] = '\0';
        TurboVal res = make_int(temp);
        free(temp);
        return res;
    }
    fprintf(stderr, "ValueError: invalid literal for int()\n");
    exit(1);
}

TurboVal turbo_float(TurboVal val) {
    if (tv_is_float(val)) return val;
    if (tv_is_int(val)) return tv_make_float(to_double(val));
    if (tv_is_bool(val)) return tv_make_float(val == TV_TRUE ? 1.0 : 0.0);
    if (tv_heap_tag(val) == HEAP_STR) {
        TurboStr* st = (TurboStr*)tv_get_ptr(val);
        return tv_make_float(atof(st->chars));
    }
    fprintf(stderr, "ValueError: invalid literal for float()\n");
    exit(1);
}

TurboVal turbo_ord(TurboVal val) {
    if (tv_heap_tag(val) != HEAP_STR) {
        fprintf(stderr, "TypeError: ord() expected a string of length 1\n");
        exit(1);
    }
    TurboStr* st = (TurboStr*)tv_get_ptr(val);
    if (st->length == 0) {
        fprintf(stderr, "TypeError: ord() expected a string of length 1\n");
        exit(1);
    }
    return tv_make_int48((unsigned char)st->chars[0]);
}

TurboVal turbo_chr(TurboVal val) {
    if (!tv_is_int(val)) {
        fprintf(stderr, "TypeError: integer expected\n");
        exit(1);
    }
    char temp[2] = { (char)int_to_ll(val), '\0' };
    return make_str_len(temp, 1);
}

TurboVal turbo_range(int32_t argc, TurboVal* args) {
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
    TurboVal list = make_list();
    for (long long i = start; i < stop; i++) {
        turbo_list_append(list, make_int_from_ll(i));
    }
    return list;
}

TurboVal turbo_open(TurboVal path, TurboVal mode) {
    if (tv_heap_tag(path) != HEAP_STR || tv_heap_tag(mode) != HEAP_STR) {
        fprintf(stderr, "TypeError: open() arguments must be strings\n");
        exit(1);
    }
    TurboStr* sp = (TurboStr*)tv_get_ptr(path);
    TurboStr* sm = (TurboStr*)tv_get_ptr(mode);
    FILE* f = fopen(sp->chars, sm->chars);
    if (f == NULL) {
        turbo_raise(make_str("FileNotFoundError"));
        return TV_NONE;
    }
    return make_file(f);
}

TurboVal turbo_file_read(TurboVal file_obj) {
    if (tv_heap_tag(file_obj) != HEAP_FILE) {
        fprintf(stderr, "TypeError: expected file object\n");
        exit(1);
    }
    TurboFile* tf = (TurboFile*)tv_get_ptr(file_obj);
    FILE* f = tf->handle;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* content = (char*)malloc(sz + 1);
    long read_sz = (long)fread(content, 1, sz, f);
    content[read_sz] = '\0';
    TurboVal str_val = make_str_len(content, (int32_t)read_sz);
    free(content);
    return str_val;
}

void turbo_file_write(TurboVal file_obj, TurboVal text_obj) {
    if (tv_heap_tag(file_obj) != HEAP_FILE) {
        fprintf(stderr, "TypeError: expected file object\n");
        exit(1);
    }
    if (tv_heap_tag(text_obj) != HEAP_STR) {
        fprintf(stderr, "TypeError: write() argument must be str\n");
        exit(1);
    }
    TurboFile* tf = (TurboFile*)tv_get_ptr(file_obj);
    TurboStr* st = (TurboStr*)tv_get_ptr(text_obj);
    fwrite(st->chars, 1, st->length, tf->handle);
}

void turbo_file_close(TurboVal file_obj) {
    if (tv_heap_tag(file_obj) != HEAP_FILE) {
        fprintf(stderr, "TypeError: expected file object\n");
        exit(1);
    }
    TurboFile* tf = (TurboFile*)tv_get_ptr(file_obj);
    if (tf->handle) {
        fclose(tf->handle);
        tf->handle = NULL;
    }
}

void turbo_list_append(TurboVal list_obj, TurboVal item) {
    if (tv_heap_tag(list_obj) != HEAP_LIST) {
        fprintf(stderr, "TypeError: expected list object\n");
        exit(1);
    }
    TurboList* lst = (TurboList*)tv_get_ptr(list_obj);
    if (lst->length >= lst->capacity) {
        lst->capacity *= 2;
        lst->items = (TurboVal*)realloc(lst->items, sizeof(TurboVal) * lst->capacity);
    }
    lst->items[lst->length++] = item;
}

void turbo_tuple_append(TurboVal tuple_obj, TurboVal item) {
    if (tv_heap_tag(tuple_obj) != HEAP_TUPLE) {
        fprintf(stderr, "TypeError: expected tuple object\n");
        exit(1);
    }
    TurboList* tup = (TurboList*)tv_get_ptr(tuple_obj);
    if (tup->length >= tup->capacity) {
        tup->capacity *= 2;
        tup->items = (TurboVal*)realloc(tup->items, sizeof(TurboVal) * tup->capacity);
    }
    tup->items[tup->length++] = item;
}

void turbo_set_add(TurboVal set_obj, TurboVal item) {
    if (tv_heap_tag(set_obj) != HEAP_SET) {
        fprintf(stderr, "TypeError: expected set object\n");
        exit(1);
    }
    TurboSet* st = (TurboSet*)tv_get_ptr(set_obj);
    if ((st->length + 1) * 2 > st->ht_cap) {
        set_ht_rebuild(st, st->ht_cap * 2);
    }
    int32_t slot = -1;
    int32_t idx = set_ht_find(st, item, &slot);
    if (idx >= 0) return;
    if (st->length >= st->capacity) {
        st->capacity *= 2;
        st->items = (TurboVal*)realloc(st->items, sizeof(TurboVal) * st->capacity);
    }
    int32_t new_idx = st->length++;
    st->items[new_idx] = item;
    st->ht_buckets[slot] = new_idx;
}

int32_t turbo_set_contains(TurboVal set_obj, TurboVal item) {
    if (tv_heap_tag(set_obj) != HEAP_SET) {
        fprintf(stderr, "TypeError: expected set object\n");
        exit(1);
    }
    TurboSet* st = (TurboSet*)tv_get_ptr(set_obj);
    return set_ht_find(st, item, NULL) >= 0 ? 1 : 0;
}

void turbo_set_remove(TurboVal set_obj, TurboVal item) {
    if (tv_heap_tag(set_obj) != HEAP_SET) {
        fprintf(stderr, "TypeError: expected set object\n");
        exit(1);
    }
    TurboSet* st = (TurboSet*)tv_get_ptr(set_obj);
    int32_t slot = -1;
    int32_t idx = set_ht_find(st, item, &slot);
    if (idx >= 0) {
        int32_t last_idx = st->length - 1;
        st->ht_buckets[slot] = -2;
        if (idx != last_idx) {
            TurboVal moved = st->items[last_idx];
            st->items[idx] = moved;
            int32_t moved_slot = -1;
            set_ht_find(st, moved, &moved_slot);
            if (moved_slot >= 0) st->ht_buckets[moved_slot] = idx;
        }
        st->length--;
        return;
    }
    fprintf(stderr, "KeyError: element not found in set\n");
    exit(1);
}

void turbo_set_discard(TurboVal set_obj, TurboVal item) {
    if (tv_heap_tag(set_obj) != HEAP_SET) {
        fprintf(stderr, "TypeError: expected set object\n");
        exit(1);
    }
    TurboSet* st = (TurboSet*)tv_get_ptr(set_obj);
    int32_t slot = -1;
    int32_t idx = set_ht_find(st, item, &slot);
    if (idx >= 0) {
        int32_t last_idx = st->length - 1;
        st->ht_buckets[slot] = -2;
        if (idx != last_idx) {
            TurboVal moved = st->items[last_idx];
            st->items[idx] = moved;
            int32_t moved_slot = -1;
            set_ht_find(st, moved, &moved_slot);
            if (moved_slot >= 0) st->ht_buckets[moved_slot] = idx;
        }
        st->length--;
    }
}

TurboVal turbo_set_pop(TurboVal set_obj) {
    TurboSet* st = (TurboSet*)tv_get_ptr(set_obj);
    if (st->length == 0) {
        fprintf(stderr, "KeyError: pop from an empty set\n");
        exit(1);
    }
    TurboVal item = st->items[st->length - 1];
    turbo_set_discard(set_obj, item);
    return item;
}

void turbo_set_clear(TurboVal set_obj) {
    TurboSet* st = (TurboSet*)tv_get_ptr(set_obj);
    st->length = 0;
    for (int i = 0; i < st->ht_cap; i++) st->ht_buckets[i] = -1;
}

TurboVal turbo_set_copy(TurboVal set_obj) {
    TurboSet* st = (TurboSet*)tv_get_ptr(set_obj);
    TurboSet* obj = (TurboSet*)malloc(sizeof(TurboSet));
    obj->tag = HEAP_SET;
    obj->length = st->length;
    obj->capacity = st->capacity;
    obj->ht_cap = st->ht_cap;
    obj->items = (TurboVal*)malloc(sizeof(TurboVal) * obj->capacity);
    memcpy(obj->items, st->items, sizeof(TurboVal) * st->length);
    obj->ht_buckets = (int32_t*)malloc(sizeof(int32_t) * obj->ht_cap);
    memcpy(obj->ht_buckets, st->ht_buckets, sizeof(int32_t) * st->ht_cap);
    return tv_make_ptr(obj);
}

TurboVal turbo_set_union(TurboVal a, TurboVal b) {
    TurboVal res = turbo_set_copy(a);
    turbo_set_update(res, b);
    return res;
}

TurboVal turbo_set_intersection(TurboVal a, TurboVal b) {
    TurboSet* sa = (TurboSet*)tv_get_ptr(a);
    TurboSet* sb = (TurboSet*)tv_get_ptr(b);
    TurboVal res = make_set();
    if (sa->length <= sb->length) {
        for (int i = 0; i < sa->length; i++) {
            if (set_ht_find(sb, sa->items[i], NULL) >= 0) {
                turbo_set_add(res, sa->items[i]);
            }
        }
    } else {
        for (int i = 0; i < sb->length; i++) {
            if (set_ht_find(sa, sb->items[i], NULL) >= 0) {
                turbo_set_add(res, sb->items[i]);
            }
        }
    }
    return res;
}

TurboVal turbo_set_difference(TurboVal a, TurboVal b) {
    TurboSet* sa = (TurboSet*)tv_get_ptr(a);
    TurboSet* sb = (TurboSet*)tv_get_ptr(b);
    TurboVal res = make_set();
    for (int i = 0; i < sa->length; i++) {
        if (set_ht_find(sb, sa->items[i], NULL) < 0) {
            turbo_set_add(res, sa->items[i]);
        }
    }
    return res;
}

TurboVal turbo_set_symmetric_difference(TurboVal a, TurboVal b) {
    TurboSet* sa = (TurboSet*)tv_get_ptr(a);
    TurboSet* sb = (TurboSet*)tv_get_ptr(b);
    TurboVal res = make_set();
    for (int i = 0; i < sa->length; i++) {
        if (set_ht_find(sb, sa->items[i], NULL) < 0) {
            turbo_set_add(res, sa->items[i]);
        }
    }
    for (int i = 0; i < sb->length; i++) {
        if (set_ht_find(sa, sb->items[i], NULL) < 0) {
            turbo_set_add(res, sb->items[i]);
        }
    }
    return res;
}

void turbo_set_update(TurboVal a, TurboVal b) {
    int32_t tb = tv_heap_tag(b);
    if (tb == HEAP_SET || tb == HEAP_LIST || tb == HEAP_TUPLE) {
        TurboList* lb = (TurboList*)tv_get_ptr(b);
        for (int i = 0; i < lb->length; i++) {
            turbo_set_add(a, lb->items[i]);
        }
    }
}

void turbo_set_intersection_update(TurboVal a, TurboVal b) {
    TurboVal inter = turbo_set_intersection(a, b);
    TurboSet* sa = (TurboSet*)tv_get_ptr(a);
    TurboSet* si = (TurboSet*)tv_get_ptr(inter);
    free(sa->items);
    free(sa->ht_buckets);
    sa->length = si->length;
    sa->capacity = si->capacity;
    sa->ht_cap = si->ht_cap;
    sa->items = si->items;
    sa->ht_buckets = si->ht_buckets;
}

void turbo_set_difference_update(TurboVal a, TurboVal b) {
    int32_t tb = tv_heap_tag(b);
    if (tb == HEAP_SET || tb == HEAP_LIST || tb == HEAP_TUPLE) {
        TurboList* lb = (TurboList*)tv_get_ptr(b);
        for (int i = 0; i < lb->length; i++) {
            turbo_set_discard(a, lb->items[i]);
        }
    }
}

void turbo_set_symmetric_difference_update(TurboVal a, TurboVal b) {
    TurboVal sym = turbo_set_symmetric_difference(a, b);
    TurboSet* sa = (TurboSet*)tv_get_ptr(a);
    TurboSet* ss = (TurboSet*)tv_get_ptr(sym);
    free(sa->items);
    free(sa->ht_buckets);
    sa->length = ss->length;
    sa->capacity = ss->capacity;
    sa->ht_cap = ss->ht_cap;
    sa->items = ss->items;
    sa->ht_buckets = ss->ht_buckets;
}

TurboVal turbo_set_issubset(TurboVal a, TurboVal b) {
    TurboSet* sa = (TurboSet*)tv_get_ptr(a);
    TurboSet* sb = (TurboSet*)tv_get_ptr(b);
    if (sa->length > sb->length) return TV_FALSE;
    for (int i = 0; i < sa->length; i++) {
        if (set_ht_find(sb, sa->items[i], NULL) < 0) return TV_FALSE;
    }
    return TV_TRUE;
}

TurboVal turbo_set_issuperset(TurboVal a, TurboVal b) {
    return turbo_set_issubset(b, a);
}

TurboVal turbo_set_isdisjoint(TurboVal a, TurboVal b) {
    TurboSet* sa = (TurboSet*)tv_get_ptr(a);
    TurboSet* sb = (TurboSet*)tv_get_ptr(b);
    if (sa->length <= sb->length) {
        for (int i = 0; i < sa->length; i++) {
            if (set_ht_find(sb, sa->items[i], NULL) >= 0) return TV_FALSE;
        }
    } else {
        for (int i = 0; i < sb->length; i++) {
            if (set_ht_find(sa, sb->items[i], NULL) >= 0) return TV_FALSE;
        }
    }
    return TV_TRUE;
}

/* --- String Methods --- */

TurboVal turbo_str_split(int32_t argc, TurboVal* args) {
    if (argc < 1 || tv_heap_tag(args[0]) != HEAP_STR) {
        fprintf(stderr, "TypeError: split target must be str\n");
        exit(1);
    }
    TurboStr* self_obj = (TurboStr*)tv_get_ptr(args[0]);
    char* str = self_obj->chars;
    TurboVal list = make_list();

    char* sep = NULL;
    if (argc >= 2 && args[1] != TV_NONE) {
        if (tv_heap_tag(args[1]) != HEAP_STR) {
            fprintf(stderr, "TypeError: sep must be str or None\n");
            exit(1);
        }
        sep = ((TurboStr*)tv_get_ptr(args[1]))->chars;
    }

    if (sep == NULL || strlen(sep) == 0) {
        char* str_copy = strdup(str);
        char* token = strtok(str_copy, " \t\r\n");
        while (token != NULL) {
            turbo_list_append(list, make_str(token));
            token = strtok(NULL, " \t\r\n");
        }
        free(str_copy);
    } else {
        char* str_copy = strdup(str);
        char* start = str_copy;
        char* p;
        int sep_len = (int)strlen(sep);
        while ((p = strstr(start, sep)) != NULL) {
            *p = '\0';
            turbo_list_append(list, make_str(start));
            start = p + sep_len;
        }
        turbo_list_append(list, make_str(start));
        free(str_copy);
    }
    return list;
}

TurboVal turbo_str_join(TurboVal self, TurboVal list_obj) {
    if (tv_heap_tag(list_obj) != HEAP_LIST) {
        fprintf(stderr, "TypeError: join() argument must be a list\n");
        exit(1);
    }
    TurboStr* st_self = (TurboStr*)tv_get_ptr(self);
    TurboList* lst = (TurboList*)tv_get_ptr(list_obj);
    int total_len = 0;
    int sep_len = st_self->length;
    int count = lst->length;
    for (int i = 0; i < count; i++) {
        TurboVal item = lst->items[i];
        if (tv_heap_tag(item) != HEAP_STR) {
            fprintf(stderr, "TypeError: sequence item %d: expected string\n", i);
            exit(1);
        }
        total_len += ((TurboStr*)tv_get_ptr(item))->length;
        if (i < count - 1) total_len += sep_len;
    }
    char* result = (char*)malloc(total_len + 1);
    int pos = 0;
    for (int i = 0; i < count; i++) {
        TurboStr* item = (TurboStr*)tv_get_ptr(lst->items[i]);
        memcpy(result + pos, item->chars, item->length);
        pos += item->length;
        if (i < count - 1 && sep_len > 0) {
            memcpy(result + pos, st_self->chars, sep_len);
            pos += sep_len;
        }
    }
    result[total_len] = '\0';
    TurboVal res = make_str_len(result, total_len);
    free(result);
    return res;
}

TurboVal turbo_str_upper(TurboVal self) {
    TurboStr* st = (TurboStr*)tv_get_ptr(self);
    int len = st->length;
    char* result = (char*)malloc(len + 1);
    for (int i = 0; i < len; i++) {
        result[i] = (char)toupper((unsigned char)st->chars[i]);
    }
    result[len] = '\0';
    TurboVal res = make_str_len(result, len);
    free(result);
    return res;
}

TurboVal turbo_str_lower(TurboVal self) {
    TurboStr* st = (TurboStr*)tv_get_ptr(self);
    int len = st->length;
    char* result = (char*)malloc(len + 1);
    for (int i = 0; i < len; i++) {
        result[i] = (char)tolower((unsigned char)st->chars[i]);
    }
    result[len] = '\0';
    TurboVal res = make_str_len(result, len);
    free(result);
    return res;
}

TurboVal turbo_str_replace(TurboVal self, TurboVal old_s, TurboVal new_s) {
    if (tv_heap_tag(old_s) != HEAP_STR || tv_heap_tag(new_s) != HEAP_STR) {
        fprintf(stderr, "TypeError: replace() arguments must be strings\n");
        exit(1);
    }
    TurboStr* st_self = (TurboStr*)tv_get_ptr(self);
    TurboStr* st_old = (TurboStr*)tv_get_ptr(old_s);
    TurboStr* st_new = (TurboStr*)tv_get_ptr(new_s);
    char* haystack = st_self->chars;
    char* needle = st_old->chars;
    char* replacement = st_new->chars;
    int needle_len = st_old->length;
    int replacement_len = st_new->length;
    int count = 0;
    char* p = haystack;
    while ((p = strstr(p, needle)) != NULL) {
        count++;
        p += needle_len;
    }
    if (count == 0) return make_str(haystack);
    int haystack_len = st_self->length;
    int result_len = haystack_len + count * (replacement_len - needle_len);
    char* result = (char*)malloc(result_len + 1);
    int pos = 0;
    char* src = haystack;
    while ((p = strstr(src, needle)) != NULL) {
        int prefix_len = (int)(p - src);
        memcpy(result + pos, src, prefix_len);
        pos += prefix_len;
        memcpy(result + pos, replacement, replacement_len);
        pos += replacement_len;
        src = p + needle_len;
    }
    int rem_len = (int)strlen(src);
    memcpy(result + pos, src, rem_len);
    result[result_len] = '\0';
    TurboVal res = make_str_len(result, result_len);
    free(result);
    return res;
}

TurboVal turbo_str_find(TurboVal self, TurboVal sub) {
    if (tv_heap_tag(sub) != HEAP_STR) {
        fprintf(stderr, "TypeError: find() argument must be string\n");
        exit(1);
    }
    TurboStr* st_self = (TurboStr*)tv_get_ptr(self);
    TurboStr* st_sub = (TurboStr*)tv_get_ptr(sub);
    char* pos = strstr(st_self->chars, st_sub->chars);
    if (pos == NULL) return tv_make_int48(-1);
    return tv_make_int48((int64_t)(pos - st_self->chars));
}

TurboVal turbo_str_strip(TurboVal self) {
    TurboStr* st = (TurboStr*)tv_get_ptr(self);
    char* s = st->chars;
    int len = st->length;
    int start = 0;
    while (start < len && (s[start] == ' ' || s[start] == '\t' || s[start] == '\n' || s[start] == '\r')) start++;
    int end = len;
    while (end > start && (s[end-1] == ' ' || s[end-1] == '\t' || s[end-1] == '\n' || s[end-1] == '\r')) end--;
    return make_str_len(s + start, end - start);
}

TurboVal turbo_str_lstrip(TurboVal self) {
    TurboStr* st = (TurboStr*)tv_get_ptr(self);
    char* s = st->chars;
    int len = st->length;
    int start = 0;
    while (start < len && (s[start] == ' ' || s[start] == '\t' || s[start] == '\n' || s[start] == '\r')) start++;
    return make_str_len(s + start, len - start);
}

TurboVal turbo_str_rstrip(TurboVal self) {
    TurboStr* st = (TurboStr*)tv_get_ptr(self);
    char* s = st->chars;
    int len = st->length;
    int end = len;
    while (end > 0 && (s[end-1] == ' ' || s[end-1] == '\t' || s[end-1] == '\n' || s[end-1] == '\r')) end--;
    return make_str_len(s, end);
}

TurboVal turbo_str_startswith(TurboVal self, TurboVal prefix) {
    if (tv_heap_tag(prefix) != HEAP_STR) {
        fprintf(stderr, "TypeError: startswith() argument must be string\n");
        exit(1);
    }
    TurboStr* st_self = (TurboStr*)tv_get_ptr(self);
    TurboStr* st_pre = (TurboStr*)tv_get_ptr(prefix);
    if (st_pre->length > st_self->length) return TV_FALSE;
    return make_bool(strncmp(st_self->chars, st_pre->chars, st_pre->length) == 0);
}

TurboVal turbo_str_endswith(TurboVal self, TurboVal suffix) {
    if (tv_heap_tag(suffix) != HEAP_STR) {
        fprintf(stderr, "TypeError: endswith() argument must be string\n");
        exit(1);
    }
    TurboStr* st_self = (TurboStr*)tv_get_ptr(self);
    TurboStr* st_suf = (TurboStr*)tv_get_ptr(suffix);
    if (st_suf->length > st_self->length) return TV_FALSE;
    return make_bool(strncmp(st_self->chars + st_self->length - st_suf->length, st_suf->chars, st_suf->length) == 0);
}

TurboVal turbo_str_count(TurboVal self, TurboVal sub) {
    if (tv_heap_tag(sub) != HEAP_STR) {
        fprintf(stderr, "TypeError: count() argument must be string\n");
        exit(1);
    }
    TurboStr* st_self = (TurboStr*)tv_get_ptr(self);
    TurboStr* st_sub = (TurboStr*)tv_get_ptr(sub);
    int count = 0;
    char* p = st_self->chars;
    int needle_len = st_sub->length;
    if (needle_len == 0) return tv_make_int48(st_self->length + 1);
    while ((p = strstr(p, st_sub->chars)) != NULL) {
        count++;
        p += needle_len;
    }
    return tv_make_int48(count);
}

static TurboVal format_arg_with_spec(TurboVal arg, const char* spec, int spec_len) {
    if (spec == NULL || spec_len == 0) return turbo_str(arg);
    if (spec_len > 1 && spec[0] == '.' && spec[spec_len - 1] == 'f') {
        int prec = 0;
        int p = 1;
        while (p < spec_len - 1 && spec[p] >= '0' && spec[p] <= '9') {
            prec = prec * 10 + (spec[p] - '0');
            p++;
        }
        if (p == spec_len - 1 && tv_is_float(arg)) {
            char buf[128];
            snprintf(buf, sizeof(buf), "%.*f", prec, tv_get_float(arg));
            return make_str(buf);
        }
    }
    return turbo_str(arg);
}

TurboVal turbo_str_format(TurboVal self, int32_t argc, TurboVal* args) {
    TurboStr* st_self = (TurboStr*)tv_get_ptr(self);
    const char* fmt = st_self->chars;
    int fmt_len = st_self->length;
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
                const char* spec = NULL;
                int spec_len = 0;
                if (i < fmt_len && fmt[i] == ':') {
                    i++;
                    spec = fmt + i;
                    while (i < fmt_len && fmt[i] != '}') i++;
                    spec_len = (int)((fmt + i) - spec);
                }
                if (i < fmt_len) i++;
                if (idx >= argc) {
                    fprintf(stderr, "IndexError: tuple index out of range\n");
                    exit(1);
                }
                TurboVal arg_str = format_arg_with_spec(args[idx], spec, spec_len);
                total_len += ((TurboStr*)tv_get_ptr(arg_str))->length;
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
                const char* spec = NULL;
                int spec_len = 0;
                if (i < fmt_len && fmt[i] == ':') {
                    i++;
                    spec = fmt + i;
                    while (i < fmt_len && fmt[i] != '}') i++;
                    spec_len = (int)((fmt + i) - spec);
                }
                if (i < fmt_len) i++;
                TurboVal arg_str = format_arg_with_spec(args[idx], spec, spec_len);
                TurboStr* ast = (TurboStr*)tv_get_ptr(arg_str);
                memcpy(result + pos, ast->chars, ast->length);
                pos += ast->length;
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
    TurboVal res = make_str_len(result, pos);
    free(result);
    return res;
}

/* --- List Methods --- */

void turbo_list_extend(TurboVal list_obj, TurboVal other) {
    if (tv_heap_tag(other) != HEAP_LIST) {
        fprintf(stderr, "TypeError: extend() argument must be a list\n");
        exit(1);
    }
    TurboList* o = (TurboList*)tv_get_ptr(other);
    for (int i = 0; i < o->length; i++) {
        turbo_list_append(list_obj, o->items[i]);
    }
}

void turbo_list_insert(TurboVal list_obj, TurboVal index_obj, TurboVal item) {
    if (!tv_is_int(index_obj)) {
        fprintf(stderr, "TypeError: insert() index must be integer\n");
        exit(1);
    }
    TurboList* lst = (TurboList*)tv_get_ptr(list_obj);
    long long idx = int_to_ll(index_obj);
    int len = lst->length;
    if (idx < 0) idx += len;
    if (idx < 0) idx = 0;
    if (idx > len) idx = len;
    if (lst->length >= lst->capacity) {
        lst->capacity *= 2;
        lst->items = (TurboVal*)realloc(lst->items, sizeof(TurboVal) * lst->capacity);
    }
    for (int i = len; i > idx; i--) {
        lst->items[i] = lst->items[i-1];
    }
    lst->items[idx] = item;
    lst->length++;
}

void turbo_list_remove(TurboVal list_obj, TurboVal item) {
    TurboList* lst = (TurboList*)tv_get_ptr(list_obj);
    for (int i = 0; i < lst->length; i++) {
        if (is_equal(lst->items[i], item)) {
            for (int j = i; j < lst->length - 1; j++) {
                lst->items[j] = lst->items[j+1];
            }
            lst->length--;
            return;
        }
    }
    fprintf(stderr, "ValueError: list.remove(x): x not in list\n");
    exit(1);
}

TurboVal turbo_list_index(TurboVal list_obj, TurboVal item) {
    TurboList* lst = (TurboList*)tv_get_ptr(list_obj);
    for (int i = 0; i < lst->length; i++) {
        if (is_equal(lst->items[i], item)) return tv_make_int48(i);
    }
    fprintf(stderr, "ValueError: list.index(x): x not in list\n");
    exit(1);
}

TurboVal turbo_list_count(TurboVal list_obj, TurboVal item) {
    TurboList* lst = (TurboList*)tv_get_ptr(list_obj);
    int count = 0;
    for (int i = 0; i < lst->length; i++) {
        if (is_equal(lst->items[i], item)) count++;
    }
    return tv_make_int48(count);
}

void turbo_list_sort(TurboVal list_obj) {
    TurboList* lst = (TurboList*)tv_get_ptr(list_obj);
    int n = lst->length;
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            TurboVal a = lst->items[j];
            TurboVal b = lst->items[j+1];
            if (turbo_gt(a, b) == TV_TRUE) {
                lst->items[j] = b;
                lst->items[j+1] = a;
            }
        }
    }
}

void turbo_list_reverse(TurboVal list_obj) {
    TurboList* lst = (TurboList*)tv_get_ptr(list_obj);
    int n = lst->length;
    for (int i = 0; i < n / 2; i++) {
        TurboVal temp = lst->items[i];
        lst->items[i] = lst->items[n - 1 - i];
        lst->items[n - 1 - i] = temp;
    }
}

TurboVal turbo_list_copy(TurboVal list_obj) {
    TurboList* lst = (TurboList*)tv_get_ptr(list_obj);
    TurboVal new_list = make_list();
    for (int i = 0; i < lst->length; i++) {
        turbo_list_append(new_list, lst->items[i]);
    }
    return new_list;
}

void turbo_list_clear(TurboVal list_obj) {
    TurboList* lst = (TurboList*)tv_get_ptr(list_obj);
    lst->length = 0;
}

/* --- Dict Methods --- */

TurboVal turbo_dict_keys(TurboVal self) {
    TurboDict* d = (TurboDict*)tv_get_ptr(self);
    TurboVal list = make_list();
    for (int i = 0; i < d->length; i++) {
        turbo_list_append(list, d->keys[i]);
    }
    return list;
}

TurboVal turbo_dict_values(TurboVal self) {
    TurboDict* d = (TurboDict*)tv_get_ptr(self);
    TurboVal list = make_list();
    for (int i = 0; i < d->length; i++) {
        turbo_list_append(list, d->values[i]);
    }
    return list;
}

TurboVal turbo_dict_items(TurboVal self) {
    TurboDict* d = (TurboDict*)tv_get_ptr(self);
    TurboVal list = make_list();
    for (int i = 0; i < d->length; i++) {
        TurboVal pair = make_list();
        turbo_list_append(pair, d->keys[i]);
        turbo_list_append(pair, d->values[i]);
        turbo_list_append(list, pair);
    }
    return list;
}

TurboVal turbo_dict_get(TurboVal self, TurboVal key, TurboVal default_val) {
    TurboDict* d = (TurboDict*)tv_get_ptr(self);
    int32_t idx = dict_ht_find(d, key, NULL);
    if (idx >= 0) return d->values[idx];
    return default_val;
}

TurboVal turbo_dict_pop(TurboVal self, TurboVal key) {
    TurboDict* d = (TurboDict*)tv_get_ptr(self);
    int32_t idx = dict_ht_find(d, key, NULL);
    if (idx >= 0) {
        TurboVal value = d->values[idx];
        for (int j = idx; j < d->length - 1; j++) {
            d->keys[j] = d->keys[j+1];
            d->values[j] = d->values[j+1];
        }
        d->length--;
        dict_ht_rebuild(d, d->ht_cap);
        return value;
    }
    fprintf(stderr, "KeyError: pop(): key not found in dict\n");
    exit(1);
}

TurboVal turbo_dict_popitem(TurboVal self) {
    TurboDict* d = (TurboDict*)tv_get_ptr(self);
    if (d->length == 0) {
        fprintf(stderr, "KeyError: popitem(): dictionary is empty\n");
        exit(1);
    }
    int last = d->length - 1;
    TurboVal key = d->keys[last];
    TurboVal value = d->values[last];
    int32_t slot = -1;
    if (dict_ht_find(d, key, &slot) >= 0 && slot >= 0) {
        d->ht_buckets[slot] = -2;
    }
    d->length--;
    TurboVal pair = make_tuple();
    turbo_tuple_append(pair, key);
    turbo_tuple_append(pair, value);
    return pair;
}

void turbo_dict_update(TurboVal self, TurboVal other) {
    if (tv_heap_tag(other) != HEAP_DICT) {
        fprintf(stderr, "TypeError: update() argument must be dict\n");
        exit(1);
    }
    TurboDict* o = (TurboDict*)tv_get_ptr(other);
    for (int i = 0; i < o->length; i++) {
        turbo_setitem(self, o->keys[i], o->values[i]);
    }
}

void turbo_dict_clear(TurboVal self) {
    TurboDict* d = (TurboDict*)tv_get_ptr(self);
    d->length = 0;
    for (int i = 0; i < d->ht_cap; i++) d->ht_buckets[i] = -1;
}

TurboVal turbo_dict_copy(TurboVal self) {
    TurboDict* d = (TurboDict*)tv_get_ptr(self);
    TurboDict* obj = (TurboDict*)malloc(sizeof(TurboDict));
    obj->tag = HEAP_DICT;
    obj->length = d->length;
    obj->capacity = d->capacity;
    obj->ht_cap = d->ht_cap;
    obj->keys = (TurboVal*)malloc(sizeof(TurboVal) * obj->capacity);
    obj->values = (TurboVal*)malloc(sizeof(TurboVal) * obj->capacity);
    memcpy(obj->keys, d->keys, sizeof(TurboVal) * d->length);
    memcpy(obj->values, d->values, sizeof(TurboVal) * d->length);
    obj->ht_buckets = (int32_t*)malloc(sizeof(int32_t) * obj->ht_cap);
    memcpy(obj->ht_buckets, d->ht_buckets, sizeof(int32_t) * d->ht_cap);
    return tv_make_ptr(obj);
}

TurboVal turbo_dict_setdefault(TurboVal self, TurboVal key, TurboVal default_val) {
    TurboDict* d = (TurboDict*)tv_get_ptr(self);
    int32_t idx = dict_ht_find(d, key, NULL);
    if (idx >= 0) return d->values[idx];
    turbo_setitem(self, key, default_val);
    return default_val;
}

/* --- Class & Method Dispatch --- */

void turbo_class_add_method(TurboVal class_obj, const char* name, TurboCFunction func) {
    TurboClass* cls = (TurboClass*)tv_get_ptr(class_obj);
    for (int i = 0; i < cls->method_count; i++) {
        if (strcmp(cls->method_names[i], name) == 0) {
            cls->method_funcs[i] = func;
            return;
        }
    }
    if (cls->method_count >= cls->method_capacity) {
        cls->method_capacity *= 2;
        cls->method_names = (char**)realloc(cls->method_names, sizeof(char*) * cls->method_capacity);
        cls->method_funcs = (TurboCFunction*)realloc(cls->method_funcs, sizeof(TurboCFunction) * cls->method_capacity);
    }
    cls->method_names[cls->method_count] = strdup(name);
    cls->method_funcs[cls->method_count] = func;
    cls->method_count++;
}

void turbo_class_add_base(TurboVal class_obj, TurboVal base_obj) {
    if (tv_heap_tag(class_obj) != HEAP_CLASS || tv_heap_tag(base_obj) != HEAP_CLASS) return;
    TurboClass* cls = (TurboClass*)tv_get_ptr(class_obj);
    if (cls->base_count >= cls->base_capacity) {
        cls->base_capacity *= 2;
        cls->bases = (TurboVal*)realloc(cls->bases, sizeof(TurboVal) * cls->base_capacity);
    }
    cls->bases[cls->base_count++] = base_obj;
}

void turbo_class_add_abstract(TurboVal class_obj, const char* name) {
    if (tv_heap_tag(class_obj) != HEAP_CLASS) return;
    TurboClass* cls = (TurboClass*)tv_get_ptr(class_obj);
    cls->abstract_names = (char**)realloc(cls->abstract_names, sizeof(char*) * (cls->abstract_count + 1));
    cls->abstract_names[cls->abstract_count++] = strdup(name);
}

static void build_mro_post(TurboVal cls_val, TurboVal* post, int* post_len, TurboVal* visited, int* vis_len) {
    if (tv_heap_tag(cls_val) != HEAP_CLASS) return;
    for (int i = 0; i < *vis_len; i++) {
        if (visited[i] == cls_val) return;
    }
    if (*vis_len < 64) visited[(*vis_len)++] = cls_val;
    TurboClass* cls = (TurboClass*)tv_get_ptr(cls_val);
    for (int i = cls->base_count - 1; i >= 0; i--) {
        build_mro_post(cls->bases[i], post, post_len, visited, vis_len);
    }
    if (*post_len < 64) post[(*post_len)++] = cls_val;
}

static void build_mro_dfs(TurboVal cls_val, TurboVal* mro, int* count) {
    TurboVal post[64];
    TurboVal visited[64];
    int post_len = 0;
    int vis_len = 0;
    build_mro_post(cls_val, post, &post_len, visited, &vis_len);
    *count = 0;
    for (int i = post_len - 1; i >= 0; i--) {
        mro[(*count)++] = post[i];
    }
}

static bool class_is_subclass(TurboVal sub_cls, TurboVal target_cls) {
    if (sub_cls == target_cls) return true;
    TurboVal mro[64];
    int mro_len = 0;
    build_mro_dfs(sub_cls, mro, &mro_len);
    for (int i = 0; i < mro_len; i++) {
        if (mro[i] == target_cls) return true;
    }
    return false;
}

static TurboCFunction class_find_method(TurboVal class_obj, const char* name) {
    TurboVal mro[64];
    int mro_len = 0;
    build_mro_dfs(class_obj, mro, &mro_len);
    for (int m = 0; m < mro_len; m++) {
        TurboClass* c = (TurboClass*)tv_get_ptr(mro[m]);
        for (int i = 0; i < c->method_count; i++) {
            if (strcmp(c->method_names[i], name) == 0) {
                return c->method_funcs[i];
            }
        }
    }
    return NULL;
}

static TurboCFunction class_find_super_method(TurboVal inst_cls, TurboVal cur_cls, const char* name) {
    TurboVal mro[64];
    int mro_len = 0;
    build_mro_dfs(inst_cls, mro, &mro_len);
    int start = 1;
    if (cur_cls != TV_NONE) {
        for (int m = 0; m < mro_len; m++) {
            if (mro[m] == cur_cls) {
                start = m + 1;
                break;
            }
        }
    }
    for (int m = start; m < mro_len; m++) {
        TurboClass* c = (TurboClass*)tv_get_ptr(mro[m]);
        for (int i = 0; i < c->method_count; i++) {
            if (strcmp(c->method_names[i], name) == 0) {
                return c->method_funcs[i];
            }
        }
    }
    return NULL;
}

static bool class_has_unimplemented_abstract(TurboVal class_obj, const char** out_name) {
    TurboVal mro[64];
    int mro_len = 0;
    build_mro_dfs(class_obj, mro, &mro_len);
    for (int m = 0; m < mro_len; m++) {
        TurboClass* base_c = (TurboClass*)tv_get_ptr(mro[m]);
        for (int a = 0; a < base_c->abstract_count; a++) {
            const char* aname = base_c->abstract_names[a];
            bool implemented = false;
            for (int k = 0; k < m; k++) {
                TurboClass* sub_c = (TurboClass*)tv_get_ptr(mro[k]);
                bool sub_is_abs = false;
                for (int sa = 0; sa < sub_c->abstract_count; sa++) {
                    if (strcmp(sub_c->abstract_names[sa], aname) == 0) {
                        sub_is_abs = true;
                        break;
                    }
                }
                if (sub_is_abs) continue;
                for (int mi = 0; mi < sub_c->method_count; mi++) {
                    if (strcmp(sub_c->method_names[mi], aname) == 0) {
                        implemented = true;
                        break;
                    }
                }
                if (!implemented) {
                    for (int ai = 0; ai < sub_c->attr_count; ai++) {
                        if (strcmp(sub_c->attr_names[ai], aname) == 0) {
                            implemented = true;
                            break;
                        }
                    }
                }
                if (implemented) break;
            }
            if (!implemented) {
                if (out_name) *out_name = aname;
                return true;
            }
        }
    }
    return false;
}

void turbo_class_set_attr(TurboVal class_obj, const char* name, TurboVal val) {
    if (tv_heap_tag(class_obj) != HEAP_CLASS) {
        fprintf(stderr, "TypeError: expected class object\n");
        exit(1);
    }
    TurboClass* cls = (TurboClass*)tv_get_ptr(class_obj);
    for (int i = 0; i < cls->attr_count; i++) {
        if (strcmp(cls->attr_names[i], name) == 0) {
            cls->attr_values[i] = val;
            return;
        }
    }
    if (cls->attr_count >= cls->attr_capacity) {
        cls->attr_capacity *= 2;
        cls->attr_names = (char**)realloc(cls->attr_names, sizeof(char*) * cls->attr_capacity);
        cls->attr_values = (TurboVal*)realloc(cls->attr_values, sizeof(TurboVal) * cls->attr_capacity);
    }
    cls->attr_names[cls->attr_count] = strdup(name);
    cls->attr_values[cls->attr_count] = val;
    cls->attr_count++;
}

TurboVal turbo_class_get_attr(TurboVal class_obj, const char* name) {
    TurboVal mro[64];
    int mro_len = 0;
    build_mro_dfs(class_obj, mro, &mro_len);
    for (int m = 0; m < mro_len; m++) {
        TurboClass* cls = (TurboClass*)tv_get_ptr(mro[m]);
        for (int i = 0; i < cls->attr_count; i++) {
            if (strcmp(cls->attr_names[i], name) == 0) {
                return cls->attr_values[i];
            }
        }
    }
    return TV_NONE;
}

TurboVal turbo_call(TurboVal callable, int32_t argc, TurboVal* args) {
    int32_t tag = tv_heap_tag(callable);
    if (tag == HEAP_FUNC || tag == HEAP_STATICMETHOD || tag == HEAP_CLASSMETHOD) {
        TurboFunc* fn = (TurboFunc*)tv_get_ptr(callable);
        return fn->func_ptr(argc, args);
    }
    if (tag == HEAP_CLASS) {
        TurboClass* cls = (TurboClass*)tv_get_ptr(callable);
        const char* abs_name = NULL;
        if (class_has_unimplemented_abstract(callable, &abs_name)) {
            char msg[256];
            snprintf(msg, sizeof(msg), "TypeError: Can't instantiate abstract class %s with abstract method %s", cls->name, abs_name ? abs_name : "");
            turbo_raise(make_str(msg));
        }
        TurboVal inst = make_instance(callable);
        TurboCFunction init_fn = class_find_method(callable, "__init__");
        if (init_fn != NULL) {
            int32_t init_argc = argc + 1;
            TurboVal stack_args[16];
            TurboVal* init_args = (init_argc <= 16) ? stack_args : (TurboVal*)malloc(sizeof(TurboVal) * init_argc);
            init_args[0] = inst;
            for (int j = 0; j < argc; j++) init_args[j + 1] = args[j];
            init_fn(init_argc, init_args);
            if (init_argc > 16) free(init_args);
        }
        return inst;
    }
    fprintf(stderr, "TypeError: object is not callable\n");
    exit(1);
}

TurboVal turbo_call_method(TurboVal obj, const char* method_name, int32_t argc, TurboVal* args) {
    int32_t tag = tv_heap_tag(obj);
    if (tag == HEAP_INSTANCE) {
        TurboInstance* inst = (TurboInstance*)tv_get_ptr(obj);
        TurboClass* cls = (TurboClass*)tv_get_ptr(inst->class_obj);
        TurboCFunction m_fn = class_find_method(inst->class_obj, method_name);
        if (m_fn != NULL) {
            int32_t call_argc = argc + 1;
            TurboVal stack_args[16];
            TurboVal* call_args = (call_argc <= 16) ? stack_args : (TurboVal*)malloc(sizeof(TurboVal) * call_argc);
            call_args[0] = obj;
            for (int j = 0; j < argc; j++) call_args[j + 1] = args[j];
            TurboVal res = m_fn(call_argc, call_args);
            if (call_argc > 16) free(call_args);
            return res;
        }
        TurboVal cls_attr = turbo_class_get_attr(inst->class_obj, method_name);
        if (cls_attr != TV_NONE) {
            int32_t atag = tv_heap_tag(cls_attr);
            if (atag == HEAP_STATICMETHOD) {
                return ((TurboFunc*)tv_get_ptr(cls_attr))->func_ptr(argc, args);
            }
            if (atag == HEAP_CLASSMETHOD) {
                int32_t call_argc = argc + 1;
                TurboVal stack_args[16];
                TurboVal* call_args = (call_argc <= 16) ? stack_args : (TurboVal*)malloc(sizeof(TurboVal) * call_argc);
                call_args[0] = inst->class_obj;
                for (int j = 0; j < argc; j++) call_args[j + 1] = args[j];
                TurboVal res = ((TurboFunc*)tv_get_ptr(cls_attr))->func_ptr(call_argc, call_args);
                if (call_argc > 16) free(call_args);
                return res;
            }
            if (atag == HEAP_PROPERTY) {
                TurboVal prop_args[1] = { obj };
                return ((TurboProperty*)tv_get_ptr(cls_attr))->getter(1, prop_args);
            }
        }
        fprintf(stderr, "AttributeError: '%s' object has no method '%s'\n", cls->name, method_name);
        exit(1);
    }
    if (tag == HEAP_SUPER) {
        TurboSuper* sup = (TurboSuper*)tv_get_ptr(obj);
        if (tv_heap_tag(sup->obj) == HEAP_INSTANCE) {
            TurboInstance* inst = (TurboInstance*)tv_get_ptr(sup->obj);
            TurboCFunction m_fn = class_find_super_method(inst->class_obj, sup->cls, method_name);
            if (m_fn == NULL) {
                m_fn = class_find_method(inst->class_obj, method_name);
            }
            if (m_fn != NULL) {
                int32_t call_argc = argc + 1;
                TurboVal stack_args[16];
                TurboVal* call_args = (call_argc <= 16) ? stack_args : (TurboVal*)malloc(sizeof(TurboVal) * call_argc);
                call_args[0] = sup->obj;
                for (int j = 0; j < argc; j++) call_args[j + 1] = args[j];
                TurboVal res = m_fn(call_argc, call_args);
                if (call_argc > 16) free(call_args);
                return res;
            }
        }
        fprintf(stderr, "AttributeError: 'super' object has no method '%s'\n", method_name);
        exit(1);
    }
    if (tag == HEAP_LIST) {
        TurboList* lst = (TurboList*)tv_get_ptr(obj);
        if (strcmp(method_name, "append") == 0) {
            turbo_list_append(obj, args[0]);
            return TV_NONE;
        }
        if (strcmp(method_name, "pop") == 0) {
            if (lst->length == 0) {
                fprintf(stderr, "IndexError: pop from empty list\n");
                exit(1);
            }
            lst->length--;
            return lst->items[lst->length];
        }
        if (strcmp(method_name, "extend") == 0) {
            turbo_list_extend(obj, args[0]);
            return TV_NONE;
        }
        if (strcmp(method_name, "insert") == 0) {
            turbo_list_insert(obj, args[0], args[1]);
            return TV_NONE;
        }
        if (strcmp(method_name, "remove") == 0) {
            turbo_list_remove(obj, args[0]);
            return TV_NONE;
        }
        if (strcmp(method_name, "index") == 0) {
            return turbo_list_index(obj, args[0]);
        }
        if (strcmp(method_name, "count") == 0) {
            return turbo_list_count(obj, args[0]);
        }
        if (strcmp(method_name, "sort") == 0) {
            turbo_list_sort(obj);
            return TV_NONE;
        }
        if (strcmp(method_name, "reverse") == 0) {
            turbo_list_reverse(obj);
            return TV_NONE;
        }
        if (strcmp(method_name, "copy") == 0) {
            return turbo_list_copy(obj);
        }
        if (strcmp(method_name, "clear") == 0) {
            turbo_list_clear(obj);
            return TV_NONE;
        }
    }
    if (tag == HEAP_STR) {
        if (strcmp(method_name, "split") == 0) {
            int32_t split_argc = argc + 1;
            TurboVal* split_args = (TurboVal*)malloc(sizeof(TurboVal) * split_argc);
            split_args[0] = obj;
            for (int j = 0; j < argc; j++) split_args[j + 1] = args[j];
            TurboVal res = turbo_str_split(split_argc, split_args);
            free(split_args);
            return res;
        }
        if (strcmp(method_name, "join") == 0) return turbo_str_join(obj, args[0]);
        if (strcmp(method_name, "upper") == 0) return turbo_str_upper(obj);
        if (strcmp(method_name, "lower") == 0) return turbo_str_lower(obj);
        if (strcmp(method_name, "replace") == 0) return turbo_str_replace(obj, args[0], args[1]);
        if (strcmp(method_name, "find") == 0) return turbo_str_find(obj, args[0]);
        if (strcmp(method_name, "strip") == 0) return turbo_str_strip(obj);
        if (strcmp(method_name, "lstrip") == 0) return turbo_str_lstrip(obj);
        if (strcmp(method_name, "rstrip") == 0) return turbo_str_rstrip(obj);
        if (strcmp(method_name, "startswith") == 0) return turbo_str_startswith(obj, args[0]);
        if (strcmp(method_name, "endswith") == 0) return turbo_str_endswith(obj, args[0]);
        if (strcmp(method_name, "count") == 0) return turbo_str_count(obj, args[0]);
        if (strcmp(method_name, "format") == 0) return turbo_str_format(obj, argc, args);
    }
    if (tag == HEAP_DICT) {
        if (strcmp(method_name, "keys") == 0) return turbo_dict_keys(obj);
        if (strcmp(method_name, "values") == 0) return turbo_dict_values(obj);
        if (strcmp(method_name, "items") == 0) return turbo_dict_items(obj);
        if (strcmp(method_name, "get") == 0) {
            TurboVal default_val = (argc == 2) ? args[1] : TV_NONE;
            return turbo_dict_get(obj, args[0], default_val);
        }
        if (strcmp(method_name, "pop") == 0) return turbo_dict_pop(obj, args[0]);
        if (strcmp(method_name, "popitem") == 0) return turbo_dict_popitem(obj);
        if (strcmp(method_name, "update") == 0) {
            turbo_dict_update(obj, args[0]);
            return TV_NONE;
        }
        if (strcmp(method_name, "clear") == 0) {
            turbo_dict_clear(obj);
            return TV_NONE;
        }
        if (strcmp(method_name, "copy") == 0) return turbo_dict_copy(obj);
        if (strcmp(method_name, "setdefault") == 0) {
            TurboVal default_val = (argc == 2) ? args[1] : TV_NONE;
            return turbo_dict_setdefault(obj, args[0], default_val);
        }
    }
    if (tag == HEAP_SET) {
        if (strcmp(method_name, "add") == 0) {
            turbo_set_add(obj, args[0]);
            return TV_NONE;
        }
        if (strcmp(method_name, "remove") == 0) {
            turbo_set_remove(obj, args[0]);
            return TV_NONE;
        }
        if (strcmp(method_name, "discard") == 0) {
            turbo_set_discard(obj, args[0]);
            return TV_NONE;
        }
        if (strcmp(method_name, "pop") == 0) {
            return turbo_set_pop(obj);
        }
        if (strcmp(method_name, "clear") == 0) {
            turbo_set_clear(obj);
            return TV_NONE;
        }
        if (strcmp(method_name, "copy") == 0) {
            return turbo_set_copy(obj);
        }
        if (strcmp(method_name, "union") == 0) {
            TurboVal res = turbo_set_copy(obj);
            for (int i = 0; i < argc; i++) turbo_set_update(res, args[i]);
            return res;
        }
        if (strcmp(method_name, "intersection") == 0) {
            TurboVal res = turbo_set_copy(obj);
            for (int i = 0; i < argc; i++) res = turbo_set_intersection(res, args[i]);
            return res;
        }
        if (strcmp(method_name, "difference") == 0) {
            TurboVal res = turbo_set_copy(obj);
            for (int i = 0; i < argc; i++) res = turbo_set_difference(res, args[i]);
            return res;
        }
        if (strcmp(method_name, "symmetric_difference") == 0) {
            return turbo_set_symmetric_difference(obj, args[0]);
        }
        if (strcmp(method_name, "update") == 0) {
            for (int i = 0; i < argc; i++) turbo_set_update(obj, args[i]);
            return TV_NONE;
        }
        if (strcmp(method_name, "intersection_update") == 0) {
            for (int i = 0; i < argc; i++) turbo_set_intersection_update(obj, args[i]);
            return TV_NONE;
        }
        if (strcmp(method_name, "difference_update") == 0) {
            for (int i = 0; i < argc; i++) turbo_set_difference_update(obj, args[i]);
            return TV_NONE;
        }
        if (strcmp(method_name, "symmetric_difference_update") == 0) {
            turbo_set_symmetric_difference_update(obj, args[0]);
            return TV_NONE;
        }
        if (strcmp(method_name, "issubset") == 0) {
            return turbo_set_issubset(obj, args[0]);
        }
        if (strcmp(method_name, "issuperset") == 0) {
            return turbo_set_issuperset(obj, args[0]);
        }
        if (strcmp(method_name, "isdisjoint") == 0) {
            return turbo_set_isdisjoint(obj, args[0]);
        }
    }
    if (tag == HEAP_FILE) {
        if (strcmp(method_name, "read") == 0) return turbo_file_read(obj);
        if (strcmp(method_name, "write") == 0) {
            turbo_file_write(obj, args[0]);
            return TV_NONE;
        }
        if (strcmp(method_name, "close") == 0) {
            turbo_file_close(obj);
            return TV_NONE;
        }
        if (strcmp(method_name, "__enter__") == 0) return obj;
        if (strcmp(method_name, "__exit__") == 0) {
            turbo_file_close(obj);
            return TV_NONE;
        }
    }
    if (tag == HEAP_CLASS) {
        TurboClass* cls = (TurboClass*)tv_get_ptr(obj);
        TurboCFunction m_fn = class_find_method(obj, method_name);
        if (m_fn != NULL) {
            return m_fn(argc, args);
        }
        TurboVal cls_attr = turbo_class_get_attr(obj, method_name);
        if (cls_attr != TV_NONE) {
            int32_t atag = tv_heap_tag(cls_attr);
            if (atag == HEAP_STATICMETHOD) {
                return ((TurboFunc*)tv_get_ptr(cls_attr))->func_ptr(argc, args);
            }
            if (atag == HEAP_CLASSMETHOD) {
                int32_t call_argc = argc + 1;
                TurboVal stack_args[16];
                TurboVal* call_args = (call_argc <= 16) ? stack_args : (TurboVal*)malloc(sizeof(TurboVal) * call_argc);
                call_args[0] = obj;
                for (int j = 0; j < argc; j++) call_args[j + 1] = args[j];
                TurboVal res = ((TurboFunc*)tv_get_ptr(cls_attr))->func_ptr(call_argc, call_args);
                if (call_argc > 16) free(call_args);
                return res;
            }
            if (atag == HEAP_PROPERTY) {
                TurboVal prop_args[1] = { obj };
                return ((TurboProperty*)tv_get_ptr(cls_attr))->getter(1, prop_args);
            }
        }
        fprintf(stderr, "AttributeError: class '%s' has no method '%s'\n", cls->name, method_name);
        exit(1);
    }
    if (tag == HEAP_MODULE) {
        TurboVal func = turbo_module_get(obj, method_name);
        return turbo_call(func, argc, args);
    }
    fprintf(stderr, "AttributeError: object has no method '%s'\n", method_name);
    exit(1);
}

/* --- Built-in Function Thunks --- */

static TurboVal builtin_len(int32_t argc, TurboVal* args) {
    (void)argc;
    return turbo_len(args[0]);
}

static TurboVal builtin_str(int32_t argc, TurboVal* args) {
    (void)argc;
    return turbo_str(args[0]);
}

static TurboVal builtin_int(int32_t argc, TurboVal* args) {
    (void)argc;
    return turbo_int(args[0]);
}

static TurboVal builtin_ord(int32_t argc, TurboVal* args) {
    (void)argc;
    return turbo_ord(args[0]);
}

static TurboVal builtin_chr(int32_t argc, TurboVal* args) {
    (void)argc;
    return turbo_chr(args[0]);
}

static TurboVal builtin_open(int32_t argc, TurboVal* args) {
    if (argc == 1) return turbo_open(args[0], make_str("r"));
    return turbo_open(args[0], args[1]);
}

static const char* get_type_name(TurboVal obj) {
    if (obj == TV_NONE) return "NoneType";
    if (tv_is_bool(obj)) return "bool";
    if (tv_is_int(obj)) return "int";
    if (tv_is_float(obj)) return "float";
    switch (tv_heap_tag(obj)) {
        case HEAP_STR: return "str";
        case HEAP_BYTES: return "bytes";
        case HEAP_LIST: return "list";
        case HEAP_TUPLE: return "tuple";
        case HEAP_SET: return "set";
        case HEAP_MODULE: return "module";
        case HEAP_SUPER: return "super";
        case HEAP_DICT: return "dict";
        case HEAP_FUNC: return "function";
        case HEAP_CLASS: return "type";
        case HEAP_INSTANCE: return "instance";
        case HEAP_FILE: return "file";
        case HEAP_STATICMETHOD: return "staticmethod";
        case HEAP_CLASSMETHOD: return "classmethod";
        case HEAP_PROPERTY: return "property";
        case HEAP_COMPLEX: return "complex";
        default: return "unknown";
    }
}

static TurboVal builtin_input(int32_t argc, TurboVal* args) {
    if (argc > 0 && args[0] != TV_NONE) {
        TurboVal prompt_str = turbo_str(args[0]);
        printf("%s", ((TurboStr*)tv_get_ptr(prompt_str))->chars);
        fflush(stdout);
    }
    char buf[1024];
    if (fgets(buf, sizeof(buf), stdin) == NULL) return make_str("");
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
    return make_str(buf);
}

static TurboVal builtin_type(int32_t argc, TurboVal* args) {
    (void)argc;
    return make_str(get_type_name(args[0]));
}

static TurboVal builtin_isinstance(int32_t argc, TurboVal* args) {
    (void)argc;
    if (tv_heap_tag(args[1]) == HEAP_CLASS) {
        if (tv_heap_tag(args[0]) == HEAP_INSTANCE) {
            TurboInstance* inst = (TurboInstance*)tv_get_ptr(args[0]);
            if (class_is_subclass(inst->class_obj, args[1])) return TV_TRUE;
        }
        return TV_FALSE;
    }
    if (tv_heap_tag(args[1]) == HEAP_TUPLE) {
        TurboList* tup = (TurboList*)tv_get_ptr(args[1]);
        for (int i = 0; i < tup->length; i++) {
            TurboVal sub_args[2] = { args[0], tup->items[i] };
            if (builtin_isinstance(2, sub_args) == TV_TRUE) return TV_TRUE;
        }
        return TV_FALSE;
    }
    if (tv_heap_tag(args[1]) != HEAP_STR) return TV_FALSE;
    const char* type_name = ((TurboStr*)tv_get_ptr(args[1]))->chars;
    const char* actual = get_type_name(args[0]);
    if (strcmp(actual, type_name) == 0) return TV_TRUE;
    if (tv_heap_tag(args[0]) == HEAP_INSTANCE) {
        TurboInstance* inst = (TurboInstance*)tv_get_ptr(args[0]);
        TurboClass* cls = (TurboClass*)tv_get_ptr(inst->class_obj);
        if (strcmp(cls->name, type_name) == 0) return TV_TRUE;
    }
    return TV_FALSE;
}

static TurboVal builtin_hasattr(int32_t argc, TurboVal* args) {
    (void)argc;
    if (tv_heap_tag(args[1]) != HEAP_STR) return TV_FALSE;
    if (tv_heap_tag(args[0]) == HEAP_INSTANCE) {
        TurboInstance* inst = (TurboInstance*)tv_get_ptr(args[0]);
        const char* name = ((TurboStr*)tv_get_ptr(args[1]))->chars;
        for (int i = 0; i < inst->length; i++) {
            if (strcmp(inst->keys[i], name) == 0) return TV_TRUE;
        }
        if (class_find_method(inst->class_obj, name) != NULL) return TV_TRUE;
        if (turbo_class_get_attr(inst->class_obj, name) != TV_NONE) return TV_TRUE;
    }
    return TV_FALSE;
}

static TurboVal builtin_getattr(int32_t argc, TurboVal* args) {
    TurboVal def = (argc >= 3) ? args[2] : TV_NONE;
    const char* name = ((TurboStr*)tv_get_ptr(args[1]))->chars;
    if (tv_heap_tag(args[0]) == HEAP_INSTANCE) {
        TurboInstance* inst = (TurboInstance*)tv_get_ptr(args[0]);
        for (int i = 0; i < inst->length; i++) {
            if (strcmp(inst->keys[i], name) == 0) return inst->values[i];
        }
        TurboVal cls_attr = turbo_class_get_attr(inst->class_obj, name);
        if (cls_attr != TV_NONE) {
            if (tv_heap_tag(cls_attr) == HEAP_PROPERTY) {
                TurboProperty* prop = (TurboProperty*)tv_get_ptr(cls_attr);
                TurboVal prop_args[1] = { args[0] };
                return prop->getter(1, prop_args);
            }
            return cls_attr;
        }
        TurboCFunction m_fn = class_find_method(inst->class_obj, name);
        if (m_fn != NULL) {
            return make_func(m_fn, name);
        }
    }
    return def;
}

static TurboVal builtin_setattr(int32_t argc, TurboVal* args) {
    (void)argc;
    turbo_setattr(args[0], ((TurboStr*)tv_get_ptr(args[1]))->chars, args[2]);
    return TV_NONE;
}

static TurboVal builtin_repr(int32_t argc, TurboVal* args) {
    (void)argc;
    return turbo_repr(args[0]);
}

static TurboVal builtin_abs(int32_t argc, TurboVal* args) {
    (void)argc;
    TurboVal a = args[0];
    if (tv_is_int48(a)) {
        int64_t v = tv_get_int48(a);
        return make_int_from_ll(v < 0 ? -v : v);
    }
    if (tv_heap_tag(a) == HEAP_BIGINT) {
        TurboBigInt* b = (TurboBigInt*)tv_get_ptr(a);
        if (b->sign == 0) return a;
        char* d = (char*)malloc(b->length + 1);
        memcpy(d, b->digits, b->length + 1);
        return make_int_from_str(d, b->length, 0);
    }
    if (tv_is_float(a)) {
        double v = tv_get_float(a);
        return tv_make_float(v < 0 ? -v : v);
    }
    fprintf(stderr, "TypeError: bad operand type for abs()\n");
    exit(1);
}

static TurboVal builtin_round(int32_t argc, TurboVal* args) {
    (void)argc;
    if (tv_is_float(args[0])) return make_int_from_ll((long long)round(tv_get_float(args[0])));
    if (tv_is_int(args[0])) return args[0];
    fprintf(stderr, "TypeError: bad operand type for round()\n");
    exit(1);
}

static TurboVal builtin_pow(int32_t argc, TurboVal* args) {
    (void)argc;
    return turbo_pow(args[0], args[1]);
}

static TurboVal builtin_hex(int32_t argc, TurboVal* args) {
    (void)argc;
    TurboVal obj = args[0];
    char bufa[32];
    const char* da;
    int la, sa;
    int_to_digits(obj, bufa, sizeof(bufa), &da, &la, &sa);
    if (la == 1 && da[0] == '0') return make_str("0");
    int hex_len;
    char* hex_str = decimal_to_base(da, la, 16, &hex_len);
    char* buf;
    if (sa) {
        buf = (char*)malloc(hex_len + 2);
        buf[0] = '-';
        memcpy(buf + 1, hex_str, hex_len + 1);
    } else {
        buf = hex_str;
    }
    TurboVal res = make_str(buf);
    if (sa) free(buf);
    free(hex_str);
    return res;
}

static TurboVal builtin_bin(int32_t argc, TurboVal* args) {
    (void)argc;
    TurboVal obj = args[0];
    char bufa[32];
    const char* da;
    int la, sa;
    int_to_digits(obj, bufa, sizeof(bufa), &da, &la, &sa);
    int bin_len;
    char* bin_str = decimal_to_base(da, la, 2, &bin_len);
    int prefix = 2 + (sa ? 1 : 0);
    char* buf = (char*)malloc(bin_len + prefix + 1);
    int pos = 0;
    if (sa) buf[pos++] = '-';
    buf[pos++] = '0';
    buf[pos++] = 'b';
    memcpy(buf + pos, bin_str, bin_len + 1);
    TurboVal res = make_str(buf);
    free(buf);
    free(bin_str);
    return res;
}

static TurboVal builtin_oct(int32_t argc, TurboVal* args) {
    (void)argc;
    TurboVal obj = args[0];
    char bufa[32];
    const char* da;
    int la, sa;
    int_to_digits(obj, bufa, sizeof(bufa), &da, &la, &sa);
    if (la == 1 && da[0] == '0') return make_str("0");
    int oct_len;
    char* oct_str = decimal_to_base(da, la, 8, &oct_len);
    char* buf;
    if (sa) {
        buf = (char*)malloc(oct_len + 2);
        buf[0] = '-';
        memcpy(buf + 1, oct_str, oct_len + 1);
    } else {
        buf = oct_str;
    }
    TurboVal res = make_str(buf);
    if (sa) free(buf);
    free(oct_str);
    return res;
}

static TurboVal builtin_float(int32_t argc, TurboVal* args) {
    (void)argc;
    return turbo_float(args[0]);
}

static TurboVal builtin_bool(int32_t argc, TurboVal* args) {
    (void)argc;
    return make_bool(turbo_is_truthy(args[0]));
}

TurboVal turbo_list(int32_t argc, TurboVal* args) {
    if (argc == 0) return make_list();
    TurboVal obj = args[0];
    int32_t tag = tv_heap_tag(obj);
    if (tag == HEAP_LIST) return turbo_list_copy(obj);
    if (tag == HEAP_TUPLE || tag == HEAP_SET) {
        TurboList* lst = (TurboList*)tv_get_ptr(obj);
        TurboVal res = make_list();
        for (int i = 0; i < lst->length; i++) turbo_list_append(res, lst->items[i]);
        return res;
    }
    if (tag == HEAP_DICT) return turbo_dict_keys(obj);
    if (tag == HEAP_STR) {
        TurboStr* st = (TurboStr*)tv_get_ptr(obj);
        TurboVal res = make_list();
        for (int i = 0; i < st->length; i++) {
            char temp[2] = { st->chars[i], '\0' };
            turbo_list_append(res, make_str_len(temp, 1));
        }
        return res;
    }
    fprintf(stderr, "TypeError: list() argument must be iterable\n");
    exit(1);
}

static TurboVal builtin_list(int32_t argc, TurboVal* args) {
    return turbo_list(argc, args);
}

static TurboVal builtin_dict(int32_t argc, TurboVal* args) {
    if (argc == 0) return make_dict();
    return turbo_dict_copy(args[0]);
}

static TurboVal builtin_tuple(int32_t argc, TurboVal* args) {
    if (argc == 0) return make_tuple();
    TurboVal obj = args[0];
    int32_t tag = tv_heap_tag(obj);
    if (tag == HEAP_LIST) return make_tuple_from_list(obj);
    if (tag == HEAP_TUPLE || tag == HEAP_SET) {
        TurboList* lst = (TurboList*)tv_get_ptr(obj);
        TurboVal res = make_tuple();
        for (int i = 0; i < lst->length; i++) turbo_tuple_append(res, lst->items[i]);
        return res;
    }
    if (tag == HEAP_STR) {
        TurboStr* st = (TurboStr*)tv_get_ptr(obj);
        TurboVal res = make_tuple();
        for (int i = 0; i < st->length; i++) {
            char temp[2] = { st->chars[i], '\0' };
            turbo_tuple_append(res, make_str_len(temp, 1));
        }
        return res;
    }
    fprintf(stderr, "TypeError: tuple() argument must be iterable\n");
    exit(1);
}

static TurboVal builtin_set(int32_t argc, TurboVal* args) {
    if (argc == 0) return make_set();
    TurboVal obj = args[0];
    int32_t tag = tv_heap_tag(obj);
    if (tag == HEAP_SET || tag == HEAP_LIST || tag == HEAP_TUPLE) {
        TurboList* lst = (TurboList*)tv_get_ptr(obj);
        TurboVal res = make_set();
        for (int i = 0; i < lst->length; i++) turbo_set_add(res, lst->items[i]);
        return res;
    }
    if (tag == HEAP_DICT) {
        TurboDict* d = (TurboDict*)tv_get_ptr(obj);
        TurboVal res = make_set();
        for (int i = 0; i < d->length; i++) turbo_set_add(res, d->keys[i]);
        return res;
    }
    if (tag == HEAP_STR) {
        TurboStr* st = (TurboStr*)tv_get_ptr(obj);
        TurboVal res = make_set();
        for (int i = 0; i < st->length; i++) {
            char temp[2] = { st->chars[i], '\0' };
            turbo_set_add(res, make_str_len(temp, 1));
        }
        return res;
    }
    fprintf(stderr, "TypeError: set() argument must be iterable\n");
    exit(1);
}

static TurboVal builtin_super(int32_t argc, TurboVal* args) {
    if (argc >= 2) return make_super(args[0], args[1]);
    return TV_NONE;
}

static TurboVal builtin_identity(int32_t argc, TurboVal* args) {
    return (argc > 0) ? args[0] : TV_NONE;
}

static TurboVal builtin_iter(int32_t argc, TurboVal* args) {
    (void)argc;
    TurboVal obj = args[0];
    int32_t tag = tv_heap_tag(obj);
    if (tag == HEAP_LIST) return turbo_slice(obj, tv_make_int48(0), TV_NONE);
    if (tag == HEAP_STR) {
        TurboStr* st = (TurboStr*)tv_get_ptr(obj);
        TurboVal res = make_list();
        for (int i = 0; i < st->length; i++) {
            char temp[2] = { st->chars[i], '\0' };
            turbo_list_append(res, make_str_len(temp, 1));
        }
        return res;
    }
    if (tag == HEAP_DICT) return turbo_dict_keys(obj);
    return obj;
}

static TurboVal builtin_next(int32_t argc, TurboVal* args) {
    TurboList* lst = (TurboList*)tv_get_ptr(args[0]);
    if (lst->length == 0) {
        return (argc == 2) ? args[1] : TV_NONE;
    }
    TurboVal item = lst->items[0];
    for (int i = 0; i < lst->length - 1; i++) {
        lst->items[i] = lst->items[i + 1];
    }
    lst->length--;
    return item;
}

static TurboVal builtin_all(int32_t argc, TurboVal* args) {
    (void)argc;
    TurboList* list = (TurboList*)tv_get_ptr(args[0]);
    for (int i = 0; i < list->length; i++) {
        if (!turbo_is_truthy(list->items[i])) return TV_FALSE;
    }
    return TV_TRUE;
}

static TurboVal builtin_any(int32_t argc, TurboVal* args) {
    (void)argc;
    TurboList* list = (TurboList*)tv_get_ptr(args[0]);
    for (int i = 0; i < list->length; i++) {
        if (turbo_is_truthy(list->items[i])) return TV_TRUE;
    }
    return TV_FALSE;
}

static TurboVal builtin_sum(int32_t argc, TurboVal* args) {
    (void)argc;
    TurboList* list = (TurboList*)tv_get_ptr(args[0]);
    if (list->length == 0) return tv_make_int48(0);
    TurboVal result = list->items[0];
    for (int i = 1; i < list->length; i++) {
        result = turbo_add(result, list->items[i]);
    }
    return result;
}

static TurboVal builtin_min(int32_t argc, TurboVal* args) {
    (void)argc;
    TurboList* list = (TurboList*)tv_get_ptr(args[0]);
    TurboVal result = list->items[0];
    for (int i = 1; i < list->length; i++) {
        if (turbo_lt(list->items[i], result) == TV_TRUE) result = list->items[i];
    }
    return result;
}

static TurboVal builtin_max(int32_t argc, TurboVal* args) {
    (void)argc;
    TurboList* list = (TurboList*)tv_get_ptr(args[0]);
    TurboVal result = list->items[0];
    for (int i = 1; i < list->length; i++) {
        if (turbo_gt(list->items[i], result) == TV_TRUE) result = list->items[i];
    }
    return result;
}

static TurboVal builtin_sorted(int32_t argc, TurboVal* args) {
    (void)argc;
    TurboVal res = turbo_list_copy(args[0]);
    turbo_list_sort(res);
    return res;
}

static TurboVal builtin_reversed(int32_t argc, TurboVal* args) {
    (void)argc;
    TurboVal res = turbo_list_copy(args[0]);
    turbo_list_reverse(res);
    return res;
}

static TurboVal builtin_enumerate(int32_t argc, TurboVal* args) {
    (void)argc;
    TurboList* list = (TurboList*)tv_get_ptr(args[0]);
    TurboVal res = make_list();
    for (int i = 0; i < list->length; i++) {
        TurboVal pair = make_list();
        turbo_list_append(pair, tv_make_int48(i));
        turbo_list_append(pair, list->items[i]);
        turbo_list_append(res, pair);
    }
    return res;
}

static TurboVal builtin_zip(int32_t argc, TurboVal* args) {
    (void)argc;
    TurboList* list1 = (TurboList*)tv_get_ptr(args[0]);
    TurboList* list2 = (TurboList*)tv_get_ptr(args[1]);
    int min_len = list1->length < list2->length ? list1->length : list2->length;
    TurboVal res = make_list();
    for (int i = 0; i < min_len; i++) {
        TurboVal pair = make_list();
        turbo_list_append(pair, list1->items[i]);
        turbo_list_append(pair, list2->items[i]);
        turbo_list_append(res, pair);
    }
    return res;
}

static TurboVal builtin_map(int32_t argc, TurboVal* args) {
    (void)argc;
    TurboVal func = args[0];
    TurboList* list = (TurboList*)tv_get_ptr(args[1]);
    TurboVal res = make_list();
    for (int i = 0; i < list->length; i++) {
        TurboVal item = turbo_call(func, 1, &list->items[i]);
        turbo_list_append(res, item);
    }
    return res;
}

static TurboVal builtin_filter(int32_t argc, TurboVal* args) {
    (void)argc;
    TurboVal func = args[0];
    TurboList* list = (TurboList*)tv_get_ptr(args[1]);
    TurboVal res = make_list();
    for (int i = 0; i < list->length; i++) {
        TurboVal item = turbo_call(func, 1, &list->items[i]);
        if (turbo_is_truthy(item)) {
            turbo_list_append(res, list->items[i]);
        }
    }
    return res;
}

void turbo_raise(TurboVal exc) {
    turbo_exception_value = exc;
    if (turbo_exception_jmp != NULL) {
        longjmp(*turbo_exception_jmp, 1);
    }
    fprintf(stderr, "Unhandled exception: ");
    TurboVal s = turbo_str(exc);
    fprintf(stderr, "%s\n", ((TurboStr*)tv_get_ptr(s))->chars);
    exit(1);
}

int32_t turbo_exception_matches(TurboVal exc, TurboVal exc_type) {
    if (exc_type == TV_NONE) return 1;
    if (tv_heap_tag(exc) != HEAP_INSTANCE) return 0;
    TurboInstance* inst = (TurboInstance*)tv_get_ptr(exc);
    int32_t etag = tv_heap_tag(exc_type);
    if (etag == HEAP_CLASS) {
        if (class_is_subclass(inst->class_obj, exc_type)) return 1;
        const char* type_name = ((TurboClass*)tv_get_ptr(exc_type))->name;
        TurboClass* cls = (TurboClass*)tv_get_ptr(inst->class_obj);
        return strcmp(cls->name, type_name) == 0 ? 1 : 0;
    } else if (etag == HEAP_STR) {
        const char* type_name = ((TurboStr*)tv_get_ptr(exc_type))->chars;
        TurboClass* cls = (TurboClass*)tv_get_ptr(inst->class_obj);
        return strcmp(cls->name, type_name) == 0 ? 1 : 0;
    }
    return 0;
}

void turbo_init(void) {
    if (t_print != TV_NONE) return;

    turbo_none = TV_NONE;
    turbo_true = TV_TRUE;
    turbo_false = TV_FALSE;
    _cached_str_empty = make_str_len("", 0);

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
    t_tuple = make_func(builtin_tuple, "tuple");
    t_set = make_func(builtin_set, "set");
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
    t_ABC = make_class("ABC");
    t_abstractmethod = make_func(builtin_identity, "abstractmethod");
    t_dataclass = make_func(builtin_identity, "dataclass");
}

void turbo_init_args(int32_t argc, char** argv) {
    turbo_init();
    t_sys_argv = make_list();
    for (int i = 0; i < argc; i++) {
        turbo_list_append(t_sys_argv, make_str(argv[i]));
    }
}
