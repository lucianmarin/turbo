# Generator expression tests
result = list(x * 2 for x in [1, 2, 3])
print("gen:", result)

total = sum(x for x in [1, 2, 3, 4, 5])
print("sum:", total)

even = list(x for x in [1, 2, 3, 4, 5, 6] if x % 2 == 0)
print("even:", even)

pairs = list((a, b) for a in [1, 2] for b in [3, 4])
print("pairs:", pairs)
