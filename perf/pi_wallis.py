def pi_wallis(n):
    pi = 2.
    for i in range(1, n):
        left = (2. * i) / (2. * i - 1.)
        right = (2. * i) / (2. * i + 1.)
        pi = pi * left * right


def main():
    print('Calculate Pi using Wallis product')
    pi_wallis(2**21 + 2**20)


main()
