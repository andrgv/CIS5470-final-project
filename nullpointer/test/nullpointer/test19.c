#include <stddef.h>

int main() {
  int* x = NULL;
  int y = 1;
  int* p;
  if (x == NULL) {
    p = &y; // Run
  } else {
    p = NULL;
  }
  return *p; // Safe
}