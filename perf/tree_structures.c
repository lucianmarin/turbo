#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct BSTNode {
    int64_t key;
    int64_t val;
    struct BSTNode* left;
    struct BSTNode* right;
    int64_t size;
    int64_t subtree_sum;
} BSTNode;

static BSTNode* bst_new(int64_t key, int64_t val) {
    BSTNode* n = (BSTNode*)malloc(sizeof(BSTNode));
    n->key = key;
    n->val = val;
    n->left = NULL;
    n->right = NULL;
    n->size = 1;
    n->subtree_sum = val;
    return n;
}

static void bst_free(BSTNode* n) {
    if (!n) return;
    bst_free(n->left);
    bst_free(n->right);
    free(n);
}

static BSTNode* bst_insert(BSTNode* root, int64_t key, int64_t val) {
    if (!root) return bst_new(key, val);
    BSTNode* path[2048];
    int path_len = 0;
    BSTNode* curr = root;
    while (1) {
        path[path_len++] = curr;
        if (key == curr->key) {
            int64_t diff = val - curr->val;
            curr->val = val;
            for (int i = 0; i < path_len; i++) {
                path[i]->subtree_sum += diff;
            }
            return root;
        } else if (key < curr->key) {
            if (!curr->left) {
                curr->left = bst_new(key, val);
                break;
            }
            curr = curr->left;
        } else {
            if (!curr->right) {
                curr->right = bst_new(key, val);
                break;
            }
            curr = curr->right;
        }
    }
    for (int i = 0; i < path_len; i++) {
        path[i]->size += 1;
        path[i]->subtree_sum += val;
    }
    return root;
}

static int64_t bst_range_sum(BSTNode* node, int64_t low, int64_t high) {
    if (!node) return 0;
    int64_t total = 0;
    if (low <= node->key && node->key <= high) {
        total += node->val;
    }
    if (low < node->key && node->left) {
        total += bst_range_sum(node->left, low, high);
    }
    if (node->key < high && node->right) {
        total += bst_range_sum(node->right, low, high);
    }
    return total;
}

static int64_t bst_mirror_weighted(BSTNode* node, int64_t depth) {
    if (!node) return 0;
    BSTNode* tmp = node->left;
    node->left = node->right;
    node->right = tmp;
    int64_t l_val = bst_mirror_weighted(node->left, depth + 1);
    int64_t r_val = bst_mirror_weighted(node->right, depth + 1);
    return (node->key * depth + node->val + l_val + r_val) % 1000000007LL;
}

typedef struct RoseNode {
    int64_t id_val;
    int64_t weight;
    struct RoseNode** children;
    int32_t count;
    int32_t capacity;
} RoseNode;

static RoseNode* rose_new(int64_t id_val, int64_t weight) {
    RoseNode* r = (RoseNode*)malloc(sizeof(RoseNode));
    r->id_val = id_val;
    r->weight = weight;
    r->count = 0;
    r->capacity = 4;
    r->children = (RoseNode**)malloc(sizeof(RoseNode*) * r->capacity);
    return r;
}

static void rose_add_child(RoseNode* parent, RoseNode* child) {
    if (parent->count >= parent->capacity) {
        parent->capacity *= 2;
        parent->children = (RoseNode**)realloc(parent->children, sizeof(RoseNode*) * parent->capacity);
    }
    parent->children[parent->count++] = child;
}

static void rose_free(RoseNode* r) {
    if (!r) return;
    for (int32_t i = 0; i < r->count; i++) rose_free(r->children[i]);
    free(r->children);
    free(r);
}

static RoseNode* build_rose_tree(int64_t branch_factor, int64_t depth, int64_t seed) {
    RoseNode* root = rose_new(seed, (seed * 31LL + depth) % 997LL);
    if (depth <= 0) return root;
    for (int64_t i = 0; i < branch_factor; i++) {
        int64_t child_seed = (seed * branch_factor + i + 1) % 1000003LL;
        rose_add_child(root, build_rose_tree(branch_factor, depth - 1, child_seed));
    }
    return root;
}

static int64_t eval_rose_tree(RoseNode* node, int64_t mult) {
    int64_t acc = (node->id_val * mult + node->weight) % 1000000007LL;
    for (int32_t i = 0; i < node->count; i++) {
        acc = (acc + eval_rose_tree(node->children[i], (mult + i + 1) % 257LL)) % 1000000007LL;
    }
    return acc;
}

int main(void) {
    int64_t checksum = 0;
    for (int64_t r = 0; r < 20; r++) {
        BSTNode* root = NULL;
        for (int64_t i = 0; i < 2000; i++) {
            int64_t k = (i * 2654435761LL + r * 1013904223LL) % 10000LL;
            int64_t v = (i * 41LL + r * 19LL) % 5000LL;
            root = bst_insert(root, k, v);
        }

        int64_t q_sum = 0;
        for (int64_t q = 0; q < 200; q++) {
            int64_t low = (q * 47LL + r * 13LL) % 8000LL;
            int64_t high = low + 1500LL;
            q_sum = (q_sum + bst_range_sum(root, low, high)) % 1000000007LL;
        }

        int64_t m1 = bst_mirror_weighted(root, 1);
        int64_t m2 = bst_mirror_weighted(root, 1);

        RoseNode* rose = build_rose_tree(4, 5, r + 1);
        int64_t r_eval = eval_rose_tree(rose, 3);

        checksum = (checksum + root->size + root->subtree_sum + q_sum + m1 + m2 + r_eval) % 1000000007LL;

        bst_free(root);
        rose_free(rose);
    }
    printf("%lld\n", (long long)checksum);
    return 0;
}
