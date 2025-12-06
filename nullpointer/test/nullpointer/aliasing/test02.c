#include <stddef.h>

int main() {
    int* x = NULL;
    int y = 1;
    int* p = &y;
    int* q = p;

    if (x == NULL) {
        q = NULL;
    }
    return *p; // OK, only q is NULL
}