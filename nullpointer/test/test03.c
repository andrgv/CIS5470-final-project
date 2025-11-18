int main() {
  int x = 1;
  int* p = &x;
  if (x < 2) {
    p = NULL;
  }
  return *p; // Error after branch
}