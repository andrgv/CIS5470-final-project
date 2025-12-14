#include <stddef.h>

int* safe() {
    int x = 5;
    int *p = &x;
    return p;
}

int main() {
    int* p = safe();
    return *p; // safe
}