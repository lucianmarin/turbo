_seed = 1234567


def random():
    global _seed
    _seed = (_seed * 1103515245 + 12345) & 0x7fffffff
    return _seed / 2147483648.0


def multiply_matrices(size):
    A = [[random() for _ in range(size)] for _ in range(size)]
    B = [[random() for _ in range(size)] for _ in range(size)]
    C = [[0 for _ in range(size)] for _ in range(size)]

    for i in range(size):
        for j in range(size):
            C[i][j] = sum(A[i][k] * B[k][j] for k in range(size))


def main():
    print('Multiply matrices')
    multiply_matrices(2**9)


main()
