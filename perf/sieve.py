def sieve(n):
    flags = [1 for _ in range(n)]
    flags[0] = 0
    flags[1] = 0
    for i in range(2, n):
        if flags[i] == 1:
            j = i * i
            while j < n:
                flags[j] = 0
                j = j + i
    count = 0
    for k in range(n):
        count = count + flags[k]
    return count


def main():
    print("Sieve of Eratosthenes")
    res = sieve(2000000)
    print(res)


main()
