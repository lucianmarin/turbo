#include <stdio.h>
#include <stdlib.h>

static long long _seed = 1234567;

double lcg_rand(void) {
    _seed = (_seed * 1103515245 + 12345) & 0x7fffffff;
    return _seed / 2147483648.0;
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
    printf("Multiply matrices\n");
    multiply_matrices(1 << 9);
    return 0;
}
