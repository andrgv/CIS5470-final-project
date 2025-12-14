// EXPECT: SAFE
#include <stdlib.h>
#include <limits.h>

typedef struct {
    char data[10240];
} img_t;

void test_alloc_overflow_good(void) {
    int num_imgs = 100; // small and safe
    img_t *table_ptr;

    // conditional check to avoid overflow
    if (num_imgs > 0 && num_imgs <= INT_MAX / (int)sizeof(img_t)) {
        table_ptr = (img_t *)malloc(sizeof(img_t) * num_imgs);
    } else {
        table_ptr = NULL;
    }

    if (table_ptr) {
        table_ptr[0].data[0] = 42;
        free(table_ptr);
    }
}

int main(void) {
    test_alloc_overflow_good();
    return 0;
}
