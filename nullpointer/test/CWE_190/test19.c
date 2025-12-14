// EXPECT: OVERFLOW
#include <stdint.h>

// Test case demonstrating widening: loop counter overflow
// Without widening: analysis would take 256+ iterations to converge
// With widening: converges in 2-3 iterations by jumping counter to [0, +inf]

void loop_counter_overflow_bad(void) {
    uint8_t counter = 0;

    // This loop will overflow uint8_t (max 255)
    // The counter increments from 0 and will exceed 255
    for (int i = 0; i < 300; i++) {
        counter++;  // Error - overflows at iteration 256
    }
}

int main(void) {
    loop_counter_overflow_bad();
    return 0;
}
