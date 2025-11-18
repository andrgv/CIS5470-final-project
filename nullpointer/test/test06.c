int main() {
    int* p = NULL;
    if (false) {
        return *p; // Unreachable, safe
    }
    return 0;
}