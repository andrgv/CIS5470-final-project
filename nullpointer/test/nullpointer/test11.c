#include <stddef.h>

int main() {
    int x = 5;
    int* p = &x;

    for (int i = 0; i < 3; i++) {
        if (i == 2) {
            return *p; // Safe
        }
    }
}