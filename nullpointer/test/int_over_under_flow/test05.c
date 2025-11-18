// Expect: fail
#include <limits.h>

int main() {
    int x = INT_MIN;
    x--; // Error
}