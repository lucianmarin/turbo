def fibonacci_recursive(n):
    def fibonacci(n):
        if n <= 1:
            return 1
        else:
            return fibonacci(n - 1) + fibonacci(n - 2)
    for i in range(1, n):
        fibonacci(i)


def main():
    print('Calculate Fibonacci numbers recursively')
    fibonacci_recursive(2**5 + 2**2 + 2 + 1)


main()
