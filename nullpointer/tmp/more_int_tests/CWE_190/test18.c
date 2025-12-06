// EXPECT: SAFE
#include <limits.h>
#include <stddef.h>

#define MAXGET 40000
#define SOMEBIGNUM 50000

short getFromInput_safe(char *buf) {
    (void)buf;
    return 1000;
}

void test_loop_overflow_good(void) {
    int bytesRec = 0;  // use wider type
    char buf[SOMEBIGNUM];

    while (bytesRec < MAXGET && bytesRec < SOMEBIGNUM) {
        bytesRec += getFromInput_safe(buf + bytesRec);
    }
}

int main(void) {
    test_loop_overflow_good();
    return 0;
}
