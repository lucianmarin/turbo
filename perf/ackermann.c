#include <stdio.h>

long long ackermann(long long m, long long n) {
    if (m == 0) {
        return n + 1;
    }
    if (n == 0) {
        return ackermann(m - 1, 1);
    }
    return ackermann(m - 1, ackermann(m, n - 1));
}

int main(void) {
    printf("Ackermann function\n");
    long long total = 0;
    for (int i = 0; i < 400; i++) {
        total = total + ackermann(3, 6);
    }
    printf("%lld\n", total);
    return 0;
}
