#include <stddef.h>

int main() {
  int* p = NULL;
  int x = 0;
  if (x < 1) {
    x = *p;  // error within branch
  }
}