// EXPECT: OVERFLOW
#include <stdlib.h>
#include <limits.h>

typedef struct {
    char data[10240]; // 10kb each image
} img_t;

void test_alloc_overflow_bad(void) {
    int num_imgs = INT_MAX / (int)sizeof(img_t) + 1;
    img_t *table_ptr;

    // force int multiplication for overflow
    int alloc_size = (int)sizeof(img_t) * num_imgs;
    table_ptr = (img_t *)malloc(alloc_size);

    // pretend to use the table as if num_imgs is correct
    if (table_ptr) {
        table_ptr[0].data[0] = 42;
    }

    free(table_ptr);
}

int main(void) {
    test_alloc_overflow_bad();
    return 0;
}
