#include <stddef.h>

int main() {
  int y = 1;
  int* x = &y;
  int* p;
  if (x == NULL) {
    p = &y;
  } else {
    p = NULL; // Run
  }
  return *p; // Error
}