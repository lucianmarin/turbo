#include <stdio.h>

void fibonacci_iterative(int n) {
    int first = 0, second = 1;
    for (int i = 2; i < n; i++) {
        int temp = first + second;
        first = second;
        second = temp;
    }
}

int main(void) {
    printf("Calculate Fibonacci numbers iteratively\n");
    fibonacci_iterative((1 << 19) + (1 << 18));
    return 0;
}
