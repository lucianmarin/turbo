_seed = 1234567


def random():
    global _seed
    _seed = (_seed * 1103515245 + 12345) & 0x7fffffff
    return _seed / 2147483648.0


def pi_wallis(n):
    pi = 2.
    for i in range(1, n):
        left = (2. * i) / (2. * i - 1.)
        right = (2. * i) / (2. * i + 1.)
        pi = pi * left * right


def fibonacci_recursive(n):
    def fibonacci(n):
        if n <= 1:
            return 1
        else:
            return fibonacci(n - 1) + fibonacci(n - 2)
    for i in range(1, n):
        fibonacci(i)


def fibonacci_iterative(n):
    first, second = 0, 1
    for _ in range(2, n):
        first, second = second, first + second


def multiply_matrices(size):
    A = [[random() for _ in range(size)] for _ in range(size)]
    B = [[random() for _ in range(size)] for _ in range(size)]
    C = [[0 for _ in range(size)] for _ in range(size)]

    for i in range(size):
        for j in range(size):
            C[i][j] = sum(A[i][k] * B[k][j] for k in range(size))


def main():
    print('Calculate Pi using Wallis product:')
    pi_wallis(2**21 + 2**20)

    print('Calculate Fibonacci numbers recursively:')
    fibonacci_recursive(2**5 + 2**2 + 2 + 1)

    print('Calculate Fibonacci numbers iteratively:')
    fibonacci_iterative(2**19 + 2**18)

    print('Multiply matrices:')
    multiply_matrices(2**9)


main()
