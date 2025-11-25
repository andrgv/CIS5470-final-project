// EXPECT: OVERFLOW
#include <limits.h>
#include <stddef.h>

#define MAXGET 40000       // larger than SHRT_MAX
#define SOMEBIGNUM 50000

short getFromInput(char *buf) {
    // For testing, always "read" a fixed positive amount.
    (void)buf;
    return 1000;
}

void test_loop_overflow_bad(void) {
    short bytesRec = 0;
    char buf[SOMEBIGNUM];

    while (bytesRec < MAXGET) {
        bytesRec += getFromInput(buf + bytesRec); // Error
    }
}

int main(void) {
    test_loop_overflow_bad();
    return 0;
}
