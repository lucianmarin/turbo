#include <stdio.h>
#include <stdlib.h>

static long long _seed = 1234567;

double lcg_rand(void) {
    _seed = (_seed * 1103515245 + 12345) & 0x7fffffff;
    return _seed / 2147483648.0;
}

void pi_wallis(int n) {
    double pi = 2.0;
    for (int i = 1; i < n; i++) {
        double left = (2.0 * i) / (2.0 * i - 1.0);
        double right = (2.0 * i) / (2.0 * i + 1.0);
        pi = pi * left * right;
    }
}

static int fib(int n) {
    if (n <= 1) return 1;
    return fib(n - 1) + fib(n - 2);
}

void fibonacci_recursive(int n) {
    for (int i = 1; i < n; i++) {
        fib(i);
    }
}

void fibonacci_iterative(int n) {
    int first = 0, second = 1;
    for (int i = 2; i < n; i++) {
        int temp = first + second;
        first = second;
        second = temp;
    }
}

void multiply_matrices(int size) {
    double *A = malloc(size * size * sizeof(double));
    double *B = malloc(size * size * sizeof(double));
    double *C = malloc(size * size * sizeof(double));

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            A[i * size + j] = lcg_rand();
            B[i * size + j] = lcg_rand();
            C[i * size + j] = 0;
        }
    }

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            double sum = 0;
            for (int k = 0; k < size; k++) {
                sum += A[i * size + k] * B[k * size + j];
            }
            C[i * size + j] = sum;
        }
    }

    free(A);
    free(B);
    free(C);
}

int main(void) {
    printf("Calculate Pi using Wallis product:\n");
    pi_wallis((1 << 21) + (1 << 20));

    printf("Calculate Fibonacci numbers recursively:\n");
    fibonacci_recursive((1 << 5) + (1 << 2) + 2 + 1);

    printf("Calculate Fibonacci numbers iteratively:\n");
    fibonacci_iterative((1 << 19) + (1 << 18));

    printf("Multiply matrices:\n");
    multiply_matrices(1 << 9);

    return 0;
}