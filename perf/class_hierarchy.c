#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int64_t x;
    int64_t y;
} Vec2D;

typedef struct {
    int64_t x;
    int64_t y;
    int64_t z;
} Vec3D;

static inline int64_t vec2d_norm_sq(Vec2D v) {
    return v.x * v.x + v.y * v.y;
}

static inline Vec2D vec2d_add(Vec2D a, Vec2D b) {
    Vec2D res = { a.x + b.x, a.y + b.y };
    return res;
}

static inline int64_t vec3d_norm_sq(Vec3D v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

static inline bool vec3d_eq(Vec3D a, Vec3D b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

typedef enum {
    NODE_SMART = 1,
    NODE_LEAF = 2
} NodeKind;

typedef struct Node {
    NodeKind kind;
    int64_t base_val;
    int64_t extra;
    int64_t (*compute)(const struct Node* self, int64_t factor);
} Node;

static int64_t smart_node_registry_count = 0;

static inline int64_t component_tag_weight(const Node* self) {
    return self->base_val + 7;
}

static inline int64_t component_compute(const Node* self, int64_t factor) {
    return self->base_val * factor;
}

static inline int64_t offset_mixin_compute(const Node* self, int64_t factor) {
    int64_t prev = component_compute(self, factor);
    return (prev + 97) % 1000003LL;
}

static inline int64_t offset_mixin_penalty(const Node* self) {
    return self->base_val / 2;
}

static inline int64_t scale_mixin_compute(const Node* self, int64_t factor) {
    int64_t prev = offset_mixin_compute(self, factor);
    return (prev * 3 + 11) % 1000003LL;
}

static inline int64_t scale_mixin_bonus(const Node* self) {
    return self->base_val * 2;
}

static int64_t smart_node_compute(const Node* self, int64_t factor) {
    int64_t res = scale_mixin_compute(self, factor);
    return (res + self->extra + scale_mixin_bonus(self) - offset_mixin_penalty(self)) % 1000003LL;
}

static int64_t leaf_node_compute(const Node* self, int64_t factor) {
    return (offset_mixin_compute(self, factor) + component_tag_weight(self)) % 1000003LL;
}

static inline int64_t smart_node_combine(int64_t a, int64_t b) {
    return (a * 17 + b * 31) % 1000003LL;
}

int main(void) {
    int64_t checksum = 1; /* abc_blocked = 1 */

    for (int64_t r = 0; r < 40; r++) {
        Node* nodes = (Node*)malloc(sizeof(Node) * 500);
        for (int64_t i = 0; i < 500; i++) {
            if (i % 3 == 0) {
                nodes[i].kind = NODE_SMART;
                nodes[i].base_val = i + r;
                nodes[i].extra = (i * 7) % 50;
                nodes[i].compute = smart_node_compute;
                smart_node_registry_count++;
            } else {
                nodes[i].kind = NODE_LEAF;
                nodes[i].base_val = i + r;
                nodes[i].extra = 0;
                nodes[i].compute = leaf_node_compute;
            }
        }

        int64_t round_acc = 0;
        for (int64_t idx = 0; idx < 500; idx++) {
            const Node* n_obj = &nodes[idx];
            int64_t val = n_obj->compute(n_obj, (idx % 13) + 1);
            if (n_obj->kind == NODE_SMART) {
                val = smart_node_combine(val, component_tag_weight(n_obj));
            } else {
                val = val + component_tag_weight(n_obj);
            }
            round_acc = (round_acc + val) % 1000000007LL;
        }
        free(nodes);

        Vec2D v_acc = { 0, 0 };
        int64_t eq_hits = 0;
        for (int64_t k = 0; k < 500; k++) {
            Vec2D v1 = { k, k * 2 };
            Vec2D v2 = { r, k };
            v_acc = vec2d_add(vec2d_add(v_acc, v1), v2);
            Vec3D v3 = { k, k * 2, r };
            Vec3D v4 = { k, k * 2, r };
            if (vec3d_eq(v3, v4)) {
                eq_hits += vec3d_norm_sq(v3);
            }
        }

        checksum = (checksum + round_acc + vec2d_norm_sq(v_acc) + eq_hits + smart_node_registry_count) % 1000000007LL;
    }

    printf("%lld\n", (long long)checksum);
    return 0;
}
