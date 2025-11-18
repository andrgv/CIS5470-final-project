// Expect: fail
int main() {
  int p = __INT_MAX__;
  p++; // Error
  return 0;
}
