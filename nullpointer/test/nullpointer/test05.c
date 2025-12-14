#include <stddef.h>

// Safe multi-branch
int main() {
    int a = 1;
    int b = 2;
    int* p = NULL;

    if (a < b) {
      p = &a;
    } else {
      p = &b;
    }

    return *p; // Safe
}