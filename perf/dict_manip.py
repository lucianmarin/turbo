# Perf 2: Dictionary Manipulation Benchmark
# Exercises: O(1) insertion, overwrite, lookup, .get() with default,
# .setdefault(), key membership (in), deletion (del / .pop()),
# dict merge (| / .update()), and iteration over .keys() / .values() / .items().

def run_dict_benchmark(n, rounds):
    checksum = 0
    r = 0
    while r < rounds:
        d = {}
        i = 0
        while i < n:
            k = (i * 1103515245 + r * 12345) % (n * 2)
            v = (i * 37 + r) % 10000
            d[k] = d.get(k, 0) + v
            i = i + 1

        # Secondary dict to merge and update
        d2 = {}
        j = 0
        while j < n // 2:
            k2 = (j * 2654435761 + r * 17) % (n * 2)
            d2[k2] = j * 3 + r
            j = j + 1

        merged = d | d2

        # setdefault & conditional deletion
        m = 0
        evicted = 0
        while m < n:
            probe = (m * 1664525 + r) % (n * 2)
            cur = merged.setdefault(probe, m)
            if cur % 3 == 0:
                popped = merged.pop(probe)
                evicted = (evicted + popped) % 1000000007
            elif cur % 5 == 0:
                del merged[probe]
                evicted = evicted + 1
            m = m + 1

        # Dict comprehension / filtering and items aggregation
        filtered = {k: v * 2 for k, v in merged.items() if v % 2 == 1}
        d.update(filtered)

        val_sum = 0
        for v in d.values():
            val_sum = (val_sum + v) % 1000000007

        checksum = (checksum + len(d) + len(filtered) + evicted + val_sum) % 1000000007
        r = r + 1

    return checksum

def main():
    res = run_dict_benchmark(4000, 25)
    print(res)

main()
