int main() {
    for (volatile long i=0; i<1000000000; i++) {}
    return 42;
}
