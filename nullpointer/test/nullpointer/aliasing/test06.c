#include <stddef.h>

int main() {
    int *p = NULL;
    int **q = &p;

    int x = 1;
    *q = &x;  // writes to p through q
    return *p; // Safe
}