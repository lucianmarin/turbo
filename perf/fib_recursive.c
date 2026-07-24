#include <stdio.h>

static int fib(int n) {
    if (n <= 1) return 1;
    return fib(n - 1) + fib(n - 2);
}

void fibonacci_recursive(int n) {
    for (int i = 1; i < n; i++) {
        fib(i);
    }
}

int main(void) {
    printf("Calculate Fibonacci numbers recursively\n");
    fibonacci_recursive((1 << 5) + (1 << 2) + 2 + 1);
    return 0;
}
