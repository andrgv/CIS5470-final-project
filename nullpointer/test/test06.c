#include <stddef.h>

int main() {
    int* p = NULL;
    if (0) {
        return *p; // Unreachable, safe
    }
    return 0;
}