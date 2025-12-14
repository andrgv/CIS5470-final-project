#include <stddef.h>

int f() {
  int* x = NULL;
  int y = 1;
  int* p = &y;
  if (x == NULL) {
    p = NULL;  // reached
  }
  return *p; // error
}