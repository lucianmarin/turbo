def fibonacci_iterative(n):
    first, second = 0, 1
    for _ in range(2, n):
        first, second = second, first + second


def main():
    print('Calculate Fibonacci numbers iteratively')
    fibonacci_iterative(2**19 + 2**18)


main()
