#include "emscripten.h"
#include <stdio.h>

extern void host();

int main() {
    printf("hello wasi!");
    host();
    return 0;
}
