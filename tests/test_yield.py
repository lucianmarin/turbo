def count(n):
    i = 0
    while i < n:
        yield i
        i = i + 1

def collect(gen):
    result = []
    for val in gen:
        result.append(val)
    return result

def main():
    result = count(5)
    print("count(5):", result)
    merged = collect(count(3))
    print("collect:", merged)

main()
