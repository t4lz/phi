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
    std::vector<char> inputModule(size);
    if (!file.read(inputModule.data(), size)){
        std::cout << "Could not read file \"" << argv[1] << "\". Stoping." << std::endl;
    }
    auto wasmVector = inject(inputModule.data(), size, 1000000, "phi_client", "injected_import");
    std::string outputFileName(argv[1]);
    outputFileName.append("_phi.wasm");
    auto myfile = std::fstream(outputFileName, std::ios::out | std::ios::binary);
    myfile.write(reinterpret_cast<char *>(wasmVector.data()), static_cast<std::streamsize>(wasmVector.size()));
    return 0;
}