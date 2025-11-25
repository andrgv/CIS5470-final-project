// EXPECT: SAFE
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

int packet_get_int_safe(void) {
    return 10; // small safe number
}

char *packet_get_string_safe(void) {
    return "dummy";
}

bool check_mul_overflow_int(int a, int b) {
    if (a > 0 && b > 0 && a > INT_MAX / b) return true;
    if (a < 0 && b < 0 && a < INT_MAX / b) return true;
    if (a > 0 && b < 0 && b < INT_MIN / a) return true;
    if (a < 0 && b > 0 && a < INT_MIN / b) return true;
    return false;
}

void test_nresp_overflow_good(void) {
    int i;
    int nresp = packet_get_int_safe();
    char **response = NULL;

    if (nresp > 0 && !check_mul_overflow_int(nresp, (int)sizeof(char *))) {
        response = (char **)malloc(nresp * (int)sizeof(char *));
    }

    if (response) {
        for (i = 0; i < nresp; i++) {
            response[i] = packet_get_string_safe();
        }
        free(response);
    }
}

int main(void) {
    test_nresp_overflow_good();
    return 0;
}
