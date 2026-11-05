# Perf 1: Set Operations Benchmark
# Exercises: set construction, O(1) membership (in), add/discard,
# union (| / .union), intersection (& / .intersection),
# difference (- / .difference), symmetric difference (^ / .symmetric_difference),
# subset/superset/disjoint checks, and in-place set updates (|=, &=, -=, ^=).

def run_set_benchmark(n, rounds):
    checksum = 0
    r = 0
    while r < rounds:
        s1 = set()
        s2 = set()
        s3 = set()

        i = 0
        while i < n:
            v1 = (i * 2654435761 + r * 97) % (n * 2)
            v2 = (i * 1597334677 + r * 131) % (n * 2)
            v3 = (i * 2246822519 + r * 53) % (n * 2)
            s1.add(v1)
            s2.add(v2)
            s3.add(v3)
            i = i + 1

        # Set algebra via operators
        u12 = s1 | s2
        inter12 = s1 & s2
        diff12 = s1 - s2
        sym12 = s1 ^ s2

        # Set algebra via methods with s3
        u_all = u12.union(s3)
        inter_all = inter12.intersection(s3)
        diff_all = diff12.difference(s3)
        sym_all = sym12.symmetric_difference(s3)

        # Subset / superset / disjoint checks
        if inter12 <= s1 and u12 >= s2:
            checksum = checksum + 1
        if diff12.isdisjoint(inter12):
            checksum = checksum + 1

        # In-place mutation & discard
        work = s1.copy()
        work |= s2
        work -= inter_all
        work ^= sym_all

        j = 0
        hits = 0
        while j < n:
            probe = (j * 3266489917 + r) % (n * 2)
            if probe in work:
                hits = hits + 1
                work.discard(probe)
            j = j + 1

        # Aggregate values from remaining set
        total_rem = 0
        for val in work:
            total_rem = (total_rem + val) % 1000000007

        checksum = (checksum + len(u_all) + len(inter_all) + len(diff_all) + hits + total_rem) % 1000000007
        r = r + 1

    return checksum

def main():
    res = run_set_benchmark(4000, 25)
    print(res)

main()
