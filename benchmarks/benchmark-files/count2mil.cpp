//
// Created by tal on 22/03/2022.
//


int main() {
    for (volatile int i=0; i<1000000; i++) {}
    return 42;
}