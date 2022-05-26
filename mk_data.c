#include <stdio.h>

int main(void) {
    int n = 10;

    for (int i = 2; i < n; i++) {
        for (int j = 0; j < 1000; j++) {
            printf("%d", i);    
        }
    }

    for (int i = 0; i < 1000; i++) {
        printf("%c", 1);
    }
}