int main() {
    int x = 1;
    int* p = &x;

    for (int i = 0; i < 10; i++) {
      x++;
    }

    return *p; // OK
}