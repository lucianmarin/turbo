#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    int64_t* keys;
    int64_t* values;
    int32_t* buckets;
    int32_t length;
    int32_t capacity;
    int32_t ht_cap;
} IntDict;

static inline uint64_t hash_i64(int64_t v) {
    uint64_t x = (uint64_t)v;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

static IntDict* dict_new(int32_t init_cap) {
    IntDict* d = (IntDict*)malloc(sizeof(IntDict));
    d->length = 0;
    d->capacity = init_cap > 8 ? init_cap : 8;
    d->ht_cap = 16;
    while (d->ht_cap < d->capacity * 2) d->ht_cap *= 2;
    d->keys = (int64_t*)malloc(sizeof(int64_t) * d->capacity);
    d->values = (int64_t*)malloc(sizeof(int64_t) * d->capacity);
    d->buckets = (int32_t*)malloc(sizeof(int32_t) * d->ht_cap);
    for (int32_t i = 0; i < d->ht_cap; i++) d->buckets[i] = -1;
    return d;
}

static void dict_free(IntDict* d) {
    free(d->keys);
    free(d->values);
    free(d->buckets);
    free(d);
}

static void dict_rebuild(IntDict* d, int32_t new_cap) {
    free(d->buckets);
    d->ht_cap = new_cap;
    d->buckets = (int32_t*)malloc(sizeof(int32_t) * new_cap);
    for (int32_t i = 0; i < new_cap; i++) d->buckets[i] = -1;
    uint32_t mask = (uint32_t)(new_cap - 1);
    for (int32_t i = 0; i < d->length; i++) {
        uint32_t b = (uint32_t)hash_i64(d->keys[i]) & mask;
        while (d->buckets[b] >= 0) b = (b + 1) & mask;
        d->buckets[b] = i;
    }
}

static int32_t dict_find(const IntDict* d, int64_t key, int32_t* out_slot) {
    uint32_t mask = (uint32_t)(d->ht_cap - 1);
    uint32_t b = (uint32_t)hash_i64(key) & mask;
    int32_t first_tomb = -1;
    while (true) {
        int32_t idx = d->buckets[b];
        if (idx == -1) {
            if (out_slot) *out_slot = (first_tomb >= 0) ? first_tomb : (int32_t)b;
            return -1;
        }
        if (idx == -2) {
            if (first_tomb < 0) first_tomb = (int32_t)b;
        } else if (d->keys[idx] == key) {
            if (out_slot) *out_slot = (int32_t)b;
            return idx;
        }
        b = (b + 1) & mask;
    }
}

static void dict_set(IntDict* d, int64_t key, int64_t val) {
    if ((d->length + 1) * 2 >= d->ht_cap) {
        dict_rebuild(d, d->ht_cap * 2);
    }
    int32_t slot = -1;
    int32_t idx = dict_find(d, key, &slot);
    if (idx >= 0) {
        d->values[idx] = val;
        return;
    }
    if (d->length >= d->capacity) {
        d->capacity *= 2;
        d->keys = (int64_t*)realloc(d->keys, sizeof(int64_t) * d->capacity);
        d->values = (int64_t*)realloc(d->values, sizeof(int64_t) * d->capacity);
    }
    int32_t new_idx = d->length++;
    d->keys[new_idx] = key;
    d->values[new_idx] = val;
    d->buckets[slot] = new_idx;
}

static int64_t dict_get(const IntDict* d, int64_t key, int64_t def_val) {
    int32_t idx = dict_find(d, key, NULL);
    return idx >= 0 ? d->values[idx] : def_val;
}

static int64_t dict_setdefault(IntDict* d, int64_t key, int64_t def_val) {
    int32_t idx = dict_find(d, key, NULL);
    if (idx >= 0) return d->values[idx];
    dict_set(d, key, def_val);
    return def_val;
}

static int64_t dict_pop(IntDict* d, int64_t key) {
    int32_t idx = dict_find(d, key, NULL);
    int64_t v = d->values[idx];
    for (int32_t j = idx; j < d->length - 1; j++) {
        d->keys[j] = d->keys[j + 1];
        d->values[j] = d->values[j + 1];
    }
    d->length--;
    dict_rebuild(d, d->ht_cap);
    return v;
}

static IntDict* dict_copy(const IntDict* d) {
    IntDict* c = (IntDict*)malloc(sizeof(IntDict));
    c->length = d->length;
    c->capacity = d->capacity;
    c->ht_cap = d->ht_cap;
    c->keys = (int64_t*)malloc(sizeof(int64_t) * c->capacity);
    c->values = (int64_t*)malloc(sizeof(int64_t) * c->capacity);
    c->buckets = (int32_t*)malloc(sizeof(int32_t) * c->ht_cap);
    memcpy(c->keys, d->keys, sizeof(int64_t) * d->length);
    memcpy(c->values, d->values, sizeof(int64_t) * d->length);
    memcpy(c->buckets, d->buckets, sizeof(int32_t) * d->ht_cap);
    return c;
}

int64_t run_dict_benchmark(int64_t n, int64_t rounds) {
    int64_t checksum = 0;
    for (int64_t r = 0; r < rounds; r++) {
        IntDict* d = dict_new(8);
        for (int64_t i = 0; i < n; i++) {
            int64_t k = (i * 1103515245LL + r * 12345LL) % (n * 2);
            int64_t v = (i * 37LL + r) % 10000LL;
            dict_set(d, k, dict_get(d, k, 0) + v);
        }

        IntDict* d2 = dict_new(8);
        for (int64_t j = 0; j < n / 2; j++) {
            int64_t k2 = (j * 2654435761LL + r * 17LL) % (n * 2);
            dict_set(d2, k2, j * 3LL + r);
        }

        IntDict* merged = dict_copy(d);
        for (int32_t i = 0; i < d2->length; i++) {
            dict_set(merged, d2->keys[i], d2->values[i]);
        }

        int64_t evicted = 0;
        for (int64_t m = 0; m < n; m++) {
            int64_t probe = (m * 1664525LL + r) % (n * 2);
            int64_t cur = dict_setdefault(merged, probe, m);
            int64_t mod3 = ((cur % 3) + 3) % 3;
            int64_t mod5 = ((cur % 5) + 5) % 5;
            if (mod3 == 0) {
                int64_t popped = dict_pop(merged, probe);
                evicted = (evicted + popped) % 1000000007LL;
            } else if (mod5 == 0) {
                dict_pop(merged, probe);
                evicted += 1;
            }
        }

        IntDict* filtered = dict_new(8);
        for (int32_t i = 0; i < merged->length; i++) {
            int64_t k = merged->keys[i];
            int64_t v = merged->values[i];
            if (((v % 2) + 2) % 2 == 1) {
                dict_set(filtered, k, v * 2);
            }
        }

        for (int32_t i = 0; i < filtered->length; i++) {
            dict_set(d, filtered->keys[i], filtered->values[i]);
        }

        int64_t val_sum = 0;
        for (int32_t i = 0; i < d->length; i++) {
            val_sum = (val_sum + d->values[i]) % 1000000007LL;
        }

        checksum = (checksum + d->length + filtered->length + evicted + val_sum) % 1000000007LL;

        dict_free(d);
        dict_free(d2);
        dict_free(merged);
        dict_free(filtered);
    }
    return checksum;
}

int main(void) {
    printf("%lld\n", (long long)run_dict_benchmark(4000, 25));
    return 0;
}
