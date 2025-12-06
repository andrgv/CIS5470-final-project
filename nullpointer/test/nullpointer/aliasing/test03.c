#include <stddef.h>

int main() {
    int x = 0;
    int* p = &x;
    int* q = p;
    p = NULL;
    q = NULL;
    return *p; // Error
}