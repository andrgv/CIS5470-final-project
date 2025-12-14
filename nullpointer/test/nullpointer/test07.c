#include <stddef.h>

int f() {
  int x = 0;
  int* p = &x;
  if (x != 0) {
    p = NULL;  // unreachable
  }
  return *p; // safe
} 
