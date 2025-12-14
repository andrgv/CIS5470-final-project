#include <stddef.h>

int main() {
    int x = 7;
    int* p = &x;
    int* q = p;

    p = NULL;
    return *q; // OK (q still points to x)
}