#include <stddef.h>

int f(int* p) {
    if (p == NULL) {
        return 0;
    }
    return *p; // OK (guarded)
}