#include <stddef.h>

int main() {
    int* p = NULL;
    int* q = p;
    int* r = q;
    return *r; // Error
}