# Perf 3: Complex Tree Structures Benchmark
# Exercises: Binary Search Tree (BST) insertion, range queries, subtree aggregation,
# and N-ary rose tree construction, deep recursive traversal, and structural transformation.

class BSTNode:
    def __init__(self, key, val):
        self.key = key
        self.val = val
        self.left = None
        self.right = None
        self.size = 1
        self.subtree_sum = val

def bst_insert(root, key, val):
    if root is None:
        return BSTNode(key, val)
    curr = root
    path = []
    while True:
        path.append(curr)
        if key == curr.key:
            diff = val - curr.val
            curr.val = val
            p_i = 0
            while p_i < len(path):
                path[p_i].subtree_sum = path[p_i].subtree_sum + diff
                p_i = p_i + 1
            return root
        elif key < curr.key:
            if curr.left is None:
                curr.left = BSTNode(key, val)
                break
            curr = curr.left
        else:
            if curr.right is None:
                curr.right = BSTNode(key, val)
                break
            curr = curr.right

    p_i = 0
    while p_i < len(path):
        node = path[p_i]
        node.size = node.size + 1
        node.subtree_sum = node.subtree_sum + val
        p_i = p_i + 1
    return root

def bst_range_sum(node, low, high):
    if node is None:
        return 0
    total = 0
    if low <= node.key and node.key <= high:
        total = total + node.val
    if low < node.key and node.left is not None:
        total = total + bst_range_sum(node.left, low, high)
    if node.key < high and node.right is not None:
        total = total + bst_range_sum(node.right, low, high)
    return total

def bst_mirror_weighted(node, depth):
    if node is None:
        return 0
    tmp = node.left
    node.left = node.right
    node.right = tmp
    l_val = bst_mirror_weighted(node.left, depth + 1)
    r_val = bst_mirror_weighted(node.right, depth + 1)
    return (node.key * depth + node.val + l_val + r_val) % 1000000007

class RoseNode:
    def __init__(self, id_val, weight):
        self.id_val = id_val
        self.weight = weight
        self.children = []

    def add_child(self, child):
        self.children.append(child)

def build_rose_tree(branch_factor, depth, seed):
    root = RoseNode(seed, (seed * 31 + depth) % 997)
    if depth <= 0:
        return root
    i = 0
    while i < branch_factor:
        child_seed = (seed * branch_factor + i + 1) % 1000003
        root.add_child(build_rose_tree(branch_factor, depth - 1, child_seed))
        i = i + 1
    return root

def eval_rose_tree(node, mult):
    acc = (node.id_val * mult + node.weight) % 1000000007
    c_i = 0
    ch = node.children
    n_ch = len(ch)
    while c_i < n_ch:
        acc = (acc + eval_rose_tree(ch[c_i], (mult + c_i + 1) % 257)) % 1000000007
        c_i = c_i + 1
    return acc

def main():
    checksum = 0
    r = 0
    while r < 20:
        root = None
        i = 0
        while i < 2000:
            k = (i * 2654435761 + r * 1013904223) % 10000
            v = (i * 41 + r * 19) % 5000
            root = bst_insert(root, k, v)
            i = i + 1

        q = 0
        q_sum = 0
        while q < 200:
            low = (q * 47 + r * 13) % 8000
            high = low + 1500
            q_sum = (q_sum + bst_range_sum(root, low, high)) % 1000000007
            q = q + 1

        m1 = bst_mirror_weighted(root, 1)
        m2 = bst_mirror_weighted(root, 1)

        # Build 4-ary tree of depth 5 (1 + 4 + 16 + 64 + 256 + 1024 = 1365 nodes)
        rose = build_rose_tree(4, 5, r + 1)
        r_eval = eval_rose_tree(rose, 3)

        checksum = (checksum + root.size + root.subtree_sum + q_sum + m1 + m2 + r_eval) % 1000000007
        r = r + 1

    print(checksum)

main()
