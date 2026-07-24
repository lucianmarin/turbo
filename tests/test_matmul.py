def test_basic_matmul():
    A = [[1, 2], [3, 4]]
    B = [[5, 6], [7, 8]]
    C = A @ B
    print("C =", C)
    print("C[0][0] =", C[0][0])
    print("C[0][1] =", C[0][1])
    print("C[1][0] =", C[1][0])
    print("C[1][1] =", C[1][1])

def test_matmul_3x2_2x4():
    A = [[1, 2], [3, 4], [5, 6]]
    B = [[1, 2, 3, 4], [5, 6, 7, 8]]
    C = A @ B
    print("3x2 @ 2x4:")
    for row in C:
        print(row)

def test_augmented():
    A = [[1, 0], [0, 1]]
    B = [[2, 0], [0, 2]]
    A @= B
    print("A after @=:", A)

def main():
    test_basic_matmul()
    test_matmul_3x2_2x4()
    test_augmented()

main()
