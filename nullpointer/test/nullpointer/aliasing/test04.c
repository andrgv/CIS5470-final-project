#include <stddef.h>

int main() {
    int x = 0;
    int* p = &x;
    p = NULL;
    int* q = p;
    return *q; // Error
}