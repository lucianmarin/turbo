#include <stdio.h>

long long collatz(long long limit) {
    long long total_steps = 0;
    for (long long i = 1; i < limit; i++) {
        long long n = i;
        long long steps = 0;
        while (n > 1) {
            if (n % 2 == 0) {
                n = n / 2;
            } else {
                n = 3 * n + 1;
            }
            steps = steps + 1;
        }
        total_steps = total_steps + steps;
    }
    return total_steps;
}

int main(void) {
    printf("Collatz sequence\n");
    long long res = collatz(1000000);
    printf("%lld\n", res);
    return 0;
}
