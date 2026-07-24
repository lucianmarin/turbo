#include <stdio.h>

void pi_wallis(int n) {
    double pi = 2.0;
    for (int i = 1; i < n; i++) {
        double left = (2.0 * i) / (2.0 * i - 1.0);
        double right = (2.0 * i) / (2.0 * i + 1.0);
        pi = pi * left * right;
    }
}

int main(void) {
    printf("Calculate Pi using Wallis product\n");
    pi_wallis((1 << 21) + (1 << 20));
    return 0;
}
