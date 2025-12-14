// Expect: Overflow
#include <stdlib.h>
#include <limits.h>

int packet_get_int(void) {
    // large value that will overflow when multiplied by sizeof(char*)
    return INT_MAX / (int)sizeof(char *) + 1;
}

char *packet_get_string(void) {
    return "hello";
}

void test_nresp_overflow_bad(void) {
    int i;
    int nresp = packet_get_int();
    char **response;

    if (nresp > 0) {
        response = (char **)malloc(nresp * (int)sizeof(char *)); // Error

        for (i = 0; i < nresp; i++) {
            response[i] = packet_get_string();
        }

        free(response);
    }
}

int main(void) {
    test_nresp_overflow_bad();
    return 0;
}
