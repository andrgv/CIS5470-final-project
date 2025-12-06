// Expect: fail
#include <limits.h>
#include <stdint.h>

int main() {
    uint8_t x = 0x00;
    x--; // Error
}