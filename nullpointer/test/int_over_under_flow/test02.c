// Expect: fail
#include <limits.h>

int main() {
    int x = INT_MAX / 2 + 1;
    int y = 2 * x; // Error
}