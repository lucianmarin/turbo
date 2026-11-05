#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    int64_t* items;
    int32_t* buckets;
    int32_t length;
    int32_t capacity;
    int32_t ht_cap;
} IntSet;

static inline uint64_t hash_i64(int64_t v) {
    uint64_t x = (uint64_t)v;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

static IntSet* set_new(int32_t init_cap) {
    IntSet* s = (IntSet*)malloc(sizeof(IntSet));
    s->length = 0;
    s->capacity = init_cap > 8 ? init_cap : 8;
    s->ht_cap = 16;
    while (s->ht_cap < s->capacity * 2) s->ht_cap *= 2;
    s->items = (int64_t*)malloc(sizeof(int64_t) * s->capacity);
    s->buckets = (int32_t*)malloc(sizeof(int32_t) * s->ht_cap);
    for (int32_t i = 0; i < s->ht_cap; i++) s->buckets[i] = -1;
    return s;
}

static void set_free(IntSet* s) {
    free(s->items);
    free(s->buckets);
    free(s);
}

static void set_rebuild(IntSet* s, int32_t new_cap) {
    free(s->buckets);
    s->ht_cap = new_cap;
    s->buckets = (int32_t*)malloc(sizeof(int32_t) * new_cap);
    for (int32_t i = 0; i < new_cap; i++) s->buckets[i] = -1;
    uint32_t mask = (uint32_t)(new_cap - 1);
    for (int32_t i = 0; i < s->length; i++) {
        uint32_t b = (uint32_t)hash_i64(s->items[i]) & mask;
        while (s->buckets[b] >= 0) b = (b + 1) & mask;
        s->buckets[b] = i;
    }
}

static int32_t set_find(const IntSet* s, int64_t val, int32_t* out_slot) {
    uint32_t mask = (uint32_t)(s->ht_cap - 1);
    uint32_t b = (uint32_t)hash_i64(val) & mask;
    int32_t first_tomb = -1;
    while (true) {
        int32_t idx = s->buckets[b];
        if (idx == -1) {
            if (out_slot) *out_slot = (first_tomb >= 0) ? first_tomb : (int32_t)b;
            return -1;
        }
        if (idx == -2) {
            if (first_tomb < 0) first_tomb = (int32_t)b;
        } else if (s->items[idx] == val) {
            if (out_slot) *out_slot = (int32_t)b;
            return idx;
        }
        b = (b + 1) & mask;
    }
}

static void set_add(IntSet* s, int64_t val) {
    if ((s->length + 1) * 2 >= s->ht_cap) {
        set_rebuild(s, s->ht_cap * 2);
    }
    int32_t slot = -1;
    if (set_find(s, val, &slot) >= 0) return;
    if (s->length >= s->capacity) {
        s->capacity *= 2;
        s->items = (int64_t*)realloc(s->items, sizeof(int64_t) * s->capacity);
    }
    int32_t idx = s->length++;
    s->items[idx] = val;
    s->buckets[slot] = idx;
}

static void set_discard(IntSet* s, int64_t val) {
    int32_t slot = -1;
    int32_t idx = set_find(s, val, &slot);
    if (idx < 0) return;
    int32_t last = s->length - 1;
    s->buckets[slot] = -2;
    if (idx != last) {
        int64_t moved = s->items[last];
        s->items[idx] = moved;
        int32_t moved_slot = -1;
        set_find(s, moved, &moved_slot);
        if (moved_slot >= 0) s->buckets[moved_slot] = idx;
    }
    s->length--;
}

static IntSet* set_copy(const IntSet* s) {
    IntSet* c = (IntSet*)malloc(sizeof(IntSet));
    c->length = s->length;
    c->capacity = s->capacity;
    c->ht_cap = s->ht_cap;
    c->items = (int64_t*)malloc(sizeof(int64_t) * c->capacity);
    c->buckets = (int32_t*)malloc(sizeof(int32_t) * c->ht_cap);
    memcpy(c->items, s->items, sizeof(int64_t) * s->length);
    memcpy(c->buckets, s->buckets, sizeof(int32_t) * s->ht_cap);
    return c;
}

static IntSet* set_union(const IntSet* a, const IntSet* b) {
    IntSet* res = set_copy(a);
    for (int32_t i = 0; i < b->length; i++) set_add(res, b->items[i]);
    return res;
}

static IntSet* set_intersection(const IntSet* a, const IntSet* b) {
    IntSet* res = set_new(8);
    for (int32_t i = 0; i < a->length; i++) {
        if (set_find(b, a->items[i], NULL) >= 0) set_add(res, a->items[i]);
    }
    return res;
}

static IntSet* set_difference(const IntSet* a, const IntSet* b) {
    IntSet* res = set_new(8);
    for (int32_t i = 0; i < a->length; i++) {
        if (set_find(b, a->items[i], NULL) < 0) set_add(res, a->items[i]);
    }
    return res;
}

static IntSet* set_sym_diff(const IntSet* a, const IntSet* b) {
    IntSet* res = set_new(8);
    for (int32_t i = 0; i < a->length; i++) {
        if (set_find(b, a->items[i], NULL) < 0) set_add(res, a->items[i]);
    }
    for (int32_t i = 0; i < b->length; i++) {
        if (set_find(a, b->items[i], NULL) < 0) set_add(res, b->items[i]);
    }
    return res;
}

static bool set_issubset(const IntSet* a, const IntSet* b) {
    if (a->length > b->length) return false;
    for (int32_t i = 0; i < a->length; i++) {
        if (set_find(b, a->items[i], NULL) < 0) return false;
    }
    return true;
}

static bool set_isdisjoint(const IntSet* a, const IntSet* b) {
    for (int32_t i = 0; i < a->length; i++) {
        if (set_find(b, a->items[i], NULL) >= 0) return false;
    }
    return true;
}

int64_t run_set_benchmark(int64_t n, int64_t rounds) {
    int64_t checksum = 0;
    for (int64_t r = 0; r < rounds; r++) {
        IntSet* s1 = set_new(8);
        IntSet* s2 = set_new(8);
        IntSet* s3 = set_new(8);

        for (int64_t i = 0; i < n; i++) {
            int64_t v1 = (i * 2654435761LL + r * 97LL) % (n * 2);
            int64_t v2 = (i * 1597334677LL + r * 131LL) % (n * 2);
            int64_t v3 = (i * 2246822519LL + r * 53LL) % (n * 2);
            set_add(s1, v1);
            set_add(s2, v2);
            set_add(s3, v3);
        }

        IntSet* u12 = set_union(s1, s2);
        IntSet* inter12 = set_intersection(s1, s2);
        IntSet* diff12 = set_difference(s1, s2);
        IntSet* sym12 = set_sym_diff(s1, s2);

        IntSet* u_all = set_union(u12, s3);
        IntSet* inter_all = set_intersection(inter12, s3);
        IntSet* diff_all = set_difference(diff12, s3);
        IntSet* sym_all = set_sym_diff(sym12, s3);

        if (set_issubset(inter12, s1) && set_issubset(s2, u12)) {
            checksum += 1;
        }
        if (set_isdisjoint(diff12, inter12)) {
            checksum += 1;
        }

        IntSet* work1 = set_union(s1, s2);
        IntSet* work2 = set_difference(work1, inter_all);
        IntSet* work = set_sym_diff(work2, sym_all);

        int64_t hits = 0;
        for (int64_t j = 0; j < n; j++) {
            int64_t probe = (j * 3266489917LL + r) % (n * 2);
            if (set_find(work, probe, NULL) >= 0) {
                hits++;
                set_discard(work, probe);
            }
        }

        int64_t total_rem = 0;
        for (int32_t i = 0; i < work->length; i++) {
            total_rem = (total_rem + work->items[i]) % 1000000007LL;
        }

        checksum = (checksum + u_all->length + inter_all->length + diff_all->length + hits + total_rem) % 1000000007LL;

        set_free(s1); set_free(s2); set_free(s3);
        set_free(u12); set_free(inter12); set_free(diff12); set_free(sym12);
        set_free(u_all); set_free(inter_all); set_free(diff_all); set_free(sym_all);
        set_free(work1); set_free(work2); set_free(work);
    }
    return checksum;
}

int main(void) {
    printf("%lld\n", (long long)run_set_benchmark(4000, 25));
    return 0;
}
