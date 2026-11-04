def ackermann(m, n):
    if m == 0:
        return n + 1
    if n == 0:
        return ackermann(m - 1, 1)
    return ackermann(m - 1, ackermann(m, n - 1))


def main():
    print("Ackermann function")
    total = 0
    for _ in range(400):
        total = total + ackermann(3, 6)
    print(total)


main()
