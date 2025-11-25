// Expect: fail
#include <limits.h>

int main() {
    short x = SHRT_MAX;
    x++; // Error
}