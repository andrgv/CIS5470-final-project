#include <stddef.h>

int main() {
    int x = 1;
    int y = 2;

    int *p = &x;
    int **q = &p;

    *q = &y;  // p = &y
    *q = NULL; // p = NULL

    return *p; // Error
}