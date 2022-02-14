//
// Created by tal on 10/02/2022.
//
#include <string>
#include <iostream>
#include <fstream>
#include "../phi_lib/phi.hpp"

//---------------------------------------------------------------------------
using namespace phi;
//---------------------------------------------------------------------------

int main(int argc, char * argv[]){
    if (argc == 1) {
        std::cout << "Please provide filename of wasm module." << std::endl;
        return 1;
    }
    std::ifstream file(argv[1], std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    size_t bufferSize = size * 2 + 1024;
    auto buffer = new char[bufferSize];                  // Allocate more for place for injected code.
    if (!file.read(buffer, size)){
        std::cout << "Could not read file \"" << argv[1] << "\". Stoping." << std::endl;
    }
    size_t outputSize = phi::inject(buffer, size, bufferSize, "phi_client", "injected_import");
    std::string outputFileName(argv[1]);
    outputFileName.append("_phi.wasm");
    auto myfile = std::fstream(outputFileName, std::ios::out | std::ios::binary);
    myfile.write(buffer, static_cast<std::streamsize>(outputSize));
    delete[] buffer;
    return 0;
}