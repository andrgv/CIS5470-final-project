#include <stddef.h>

int main() {
    int x = 1;
    int y = 2;
    int *p = &x;
    int *s = &y;
    int **q = &p;

    q = &s;
    *q = NULL; // writes s = NULL
    return *p; // Safe
}