#include <cstdio>
#include <iostream>
#include <memory>
#include "wasmer.h"

#include "../phi_lib/phi.hpp"
#include "Benchmark.hpp"

using namespace benchmarks;

wasm_trap_t* host_func_callback(const wasm_val_vec_t* args, wasm_val_vec_t* results) {
    std::cout << "Host called!" << std::endl;
    return nullptr;
}


void run(Benchmark& benchmark) {

    wasm_store_t* store = wasm_store_new(benchmark.engine);
    wasm_byte_vec_t binary;
    if (benchmark.runConfig.phi) {
        wasm_byte_vec_new(&binary, benchmark.phiBinary.size(), benchmark.phiBinary.data());
    } else {
        if (benchmark.runConfig.optimize) {
            auto optBinary = phi::justOptimize(benchmark.binary);
            wasm_byte_vec_new(&binary, optBinary.size(), optBinary.data());
        } else {
            wasm_byte_vec_new(&binary, benchmark.binary.size(), benchmark.binary.data());
        }
    }
    wasm_module_t* module = wasm_module_new(store, &binary);
    if (!module) {
        printf("> Error compiling module!\n");
        exit(1);
    }

    wasm_extern_vec_t wasiImports;
    auto wasiConf = wasi_config_new("benchmark");
    auto wasiEnv = wasi_env_new(wasiConf);
    auto result = wasi_get_imports(store, module, wasiEnv, &wasiImports);
    if (!result) {
        std::cout << "Failed to get wasi imports" << std::endl;
        exit(1);
    }

    std::cout << result << std::endl;

    wasm_extern_vec_t imports;
    wasm_extern_vec_t* importsPtr = &wasiImports;

    std::cout << "DEBUG " << wasiImports.size <<  std::endl;
    if (benchmark.runConfig.phi) {
        importsPtr = &imports;
        wasm_extern_vec_new_uninitialized(importsPtr, wasiImports.size + 1);
        std::cout << "DEBUG" << std::endl;
        for (int i=0; i<wasiImports.size; i++) {
            importsPtr->data[i] = wasiImports.data[i];
        }

        wasm_functype_t* host_func_type = wasm_functype_new_0_0();
        wasm_func_t* host_func = wasm_func_new(store, host_func_type, host_func_callback);
        importsPtr->data[importsPtr->size-1] = wasm_func_as_extern(host_func);
        wasm_functype_delete(host_func_type);
    }

    wasm_instance_t* instance = wasm_instance_new(store, module, importsPtr, nullptr);
    if (!instance) {
        printf("> Error instantiating module!\n");
        exit(1);
    }


    auto* startFunc = wasi_get_start_function(instance);
    if (!startFunc) {
        std::cout << "Couldn't find wasi start function :(";
        exit(1);
    }

    wasm_val_vec_t args = WASM_EMPTY_VEC;
    wasm_val_vec_t results = WASM_EMPTY_VEC;


    wasm_message_t exitMessage;
    auto exitTrap = wasm_func_call(startFunc, &args, &results);
    wasm_trap_message(exitTrap, &exitMessage);
    if (benchmark.runConfig.verbose)
        std::cout << "Exit message: " << exitMessage.data << std::endl;

    wasm_byte_vec_delete(&exitMessage);
    wasm_module_delete(module);
    wasm_instance_delete(instance);
    wasi_env_delete(wasiEnv);
    wasm_store_delete(store);
}

int main(int argc, const char* argv[]) {
    wasm_engine_t* engine = wasm_engine_new();
    auto benchmark = std::make_unique<Benchmark>(engine, "../../benchmarks/benchmark-files/cpp42.wasm");
//    auto benchmark = std::make_unique<Benchmark>(engine, "../../benchmarks/benchmark-files/cpp42wat.wasm");
    benchmark->runConfig.verbose = true;
    benchmark->doFullTest(run, 2);
    std::cout << "Done." << std::endl;
    wasm_engine_delete(engine);
    return 0;
}
