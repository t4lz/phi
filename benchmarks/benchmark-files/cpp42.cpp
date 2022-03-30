#include "emscripten.h"

extern "C" {
    extern void host();
}

int main() {
    host();
    return 42;
}
