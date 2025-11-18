int main() {
  int* p = NULL;
  int x;
  if (x < 1) {
    x = *p;  // error within branch
  }
}