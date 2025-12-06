#include <stddef.h>

int main() {
  int x = 0;
  int* p = &x;
  int* n = NULL;
  if (p == NULL) {
    int y = *n;  // unreachable
  }
  return 0;
}
