#include <stddef.h>

int main() {
    int x = 1;
    int *p = &x;
    int **q = &p;
    int ***r = &q;

    **r = NULL; // writes p = NULL
    return *p; // Error
}