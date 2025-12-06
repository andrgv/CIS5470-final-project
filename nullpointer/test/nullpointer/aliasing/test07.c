#include <stddef.h>

int main() {
    int x = 1;
    int *p = &x;
    int **q = &p;
    *q = NULL;  // writes to p through q
    return *p; // Error
}