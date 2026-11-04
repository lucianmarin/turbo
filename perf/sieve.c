#include <stdio.h>
#include <stdlib.h>

long long sieve(long long n) {
    long long *flags = malloc(n * sizeof(long long));
    for (long long i = 0; i < n; i++) {
        flags[i] = 1;
    }
    flags[0] = 0;
    flags[1] = 0;
    for (long long i = 2; i < n; i++) {
        if (flags[i] == 1) {
            long long j = i * i;
            while (j < n) {
                flags[j] = 0;
                j = j + i;
            }
        }
    }
    long long count = 0;
    for (long long k = 0; k < n; k++) {
        count = count + flags[k];
    }
    free(flags);
    return count;
}

int main(void) {
    printf("Sieve of Eratosthenes\n");
    long long res = sieve(2000000);
    printf("%lld\n", res);
    return 0;
}
