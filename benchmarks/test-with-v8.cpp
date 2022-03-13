#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <chrono>

#include "../phi_lib/phi.hpp"

#include "wasm.hh"


auto hostFunc(
        const wasm::Val args[], wasm::Val results[]
) -> wasm::own<wasm::Trap> {
//    std::cout << "Calling back..." << std::endl << "> " << args[0] << std::endl;
//    results[0] = args[0].copy();
    std::cout << "host called!" << std::endl;
    return nullptr;
}

wasm::vec<byte_t> getBinary(std::string&& filename) {
    // Load binary.
    std::ifstream file(filename);
    file.seekg(0, std::ios_base::end);
    auto file_size = file.tellg();
    file.seekg(0);
    auto binary = wasm::vec<byte_t>::make_uninitialized(file_size);
    file.read(binary.get(), file_size);
    file.close();
    if (file.fail()) {
        std::cout << "> Error loading module!" << std::endl;
        exit(1);
    }
    return std::move(binary);
}

std::unique_ptr<wasm::Func> getCallback(wasm::Store* store) {
    // Create external function.
    auto func_type = wasm::FuncType::make(
            wasm::ownvec<wasm::ValType>::make(),
            wasm::ownvec<wasm::ValType>::make()
    );
    return wasm::Func::make(store, func_type.get(), hostFunc);
}


void run(std::unique_ptr<wasm::Engine>& engine, std::string&& wasmFile, bool callHost = true) {
    // Wee8 boilerplate:
    auto store_ = wasm::Store::make(engine.get());
    auto store = store_.get();
    auto phiStore_ = wasm::Store::make(engine.get());
    auto phiStore = phiStore_.get();

    auto binary = getBinary(std::move(wasmFile));

    // Convert to normal vector for phi.
    auto inputModule = std::vector<char>(binary.get(), binary.get() + binary.size());

    // Perform Phi injection.
    auto beforeInject = std::chrono::high_resolution_clock::now();
    auto phiBinary = phi::inject(inputModule, callHost ? 1000000 : 1000000000 , "v8tester", "host");
    auto afterInject = std::chrono::high_resolution_clock::now();
    auto injection_ms = std::chrono::duration_cast<std::chrono::milliseconds>(afterInject - beforeInject);
    std::cout << "Phi injection completed in " << injection_ms.count() << " ms." << std::endl;

    // Convert to weird v8 vec.
    auto phiBinaryVec = wasm::vec<byte_t>::make(phiBinary.size(), phiBinary.data());

    // Compile.
    auto module = wasm::Module::make(store, binary);
    if (!module) {
        std::cout << "> Error compiling module!" << std::endl;
        exit(1);
    }
    auto phiModule = wasm::Module::make(phiStore, phiBinaryVec);
    if (!phiModule) {
        std::cout << "> Error compiling phi injected module!" << std::endl;
        exit(1);
    }

    // Instantiate.
    wasm::Extern* imports[] = {};
    auto instance = wasm::Instance::make(store, module.get(), imports);
    if (!instance) {
        std::cout << "> Error instantiating module!" << std::endl;
        exit(1);
    }

    auto callback = getCallback(phiStore);
    wasm::Extern* phiImports[] = {callback.get()};
    auto phiInstance = wasm::Instance::make(phiStore, phiModule.get(), phiImports);
    if (!phiInstance) {
        std::cout << "> Error instantiating phi module!" << std::endl;
        exit(1);
    }

    // Extract export.
    auto exports = instance->exports();
    if (exports.size() == 0 || exports[0]->kind() != wasm::EXTERN_FUNC || !exports[0]->func()) {
        std::cout << "> Error accessing export!" << std::endl;
        exit(1);
    }
    auto run_func = exports[0]->func();

    auto phiExports = phiInstance->exports();
    if (phiExports.size() == 0 || phiExports[0]->kind() != wasm::EXTERN_FUNC || !phiExports[0]->func()) {
        std::cout << "> Error accessing phi export!" << std::endl;
        exit(1);
    }
    auto phiExportFunc = phiExports[0]->func();

    // Call original code.
    auto beforeRun = std::chrono::high_resolution_clock::now();
    auto res = run_func->call();
    auto afterRun = std::chrono::high_resolution_clock::now();
    if (res) {
        std::cout << "> Error calling function!" << std::endl;
        exit(1);
    }
    auto runMS = std::chrono::duration_cast<std::chrono::microseconds>(afterRun - beforeRun);
    std::cout << "Original code took " << runMS.count() << " μs." << std::endl;

    // Call phi injected code.
    auto beforePhiRun = std::chrono::high_resolution_clock::now();
    auto phiRes = phiExportFunc->call();
    auto afterPhiRun = std::chrono::high_resolution_clock::now();
    if (phiRes) {
        std::cout << "> Error calling phi function!" << std::endl;
        exit(1);
    }
    auto phiRunMS = std::chrono::duration_cast<std::chrono::microseconds>(afterPhiRun - beforePhiRun);
    std::cout << "Phi injected code took " << phiRunMS.count() << " μs." << std::endl;
}


int main(int argc, const char* argv[]) {
    auto engine = wasm::Engine::make(); // There can only be one (per process).
    run(engine, "../../benchmarks/count-to-mil.wasm", true);
    run(engine, "../../benchmarks/count-to-mil.wasm", false);
    std::cout << "Done." << std::endl;
    return 0;
}
