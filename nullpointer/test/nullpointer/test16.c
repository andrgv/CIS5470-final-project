#include <stddef.h>

int* safe() {
    int x = 5;
    int *p = &x;
    return p;
}

int f() {
    int* p = safe();
    return *p; // safe
}