#include <atomic>

int main() {
    for (std::atomic_int i(0); i<1000000; i++) {}
    return 42;
}
