#include <stddef.h>

int* safe(int* arg) {
    return *arg; // Should be safe
}

int main() {
    int x = 0;
    int *p = &x;
    int* ret = safe(p);
}