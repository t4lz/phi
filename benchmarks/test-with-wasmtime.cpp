#include <wasm.h>
#include <wasm.hh>
#include <wasmtime.h>
#include <string>
#include <iostream>
#include <chrono>
#include "../phi_lib/phi.hpp"
#include "Benchmark.hpp"

using namespace benchmarks;

volatile static int COUNTER=0;

// based on https://github.com/bytecodealliance/wasmtime/blob/main/examples/wasi/main.c

static void exit_with_error(const char *message, wasmtime_error_t *error, wasm_trap_t *trap);

static wasm_trap_t* hostFunc(
        void *env,
        wasmtime_caller_t *caller,
        const wasmtime_val_t *args,
        size_t nargs,
        wasmtime_val_t *results,
        size_t nresults
) {
//    std::cout << "Host called!" << std::endl;
    COUNTER++;
    return nullptr;
}

void run(Benchmark& benchmark) {
    if (benchmark.runConfig.verbose){
        std::cout << "Testing " << benchmark.filename << (benchmark.runConfig.optimize ? " " : " un") << "optimized," << " with" << (benchmark.runConfig.phi ? "" : "out") << " phi." << std::endl;
    }
    assert(benchmark.engine);
    wasmtime_store_t *store = wasmtime_store_new(benchmark.engine, nullptr, nullptr);
    assert(store);
    wasmtime_context_t *context = wasmtime_store_context(store);

    // Create a linker with WASI functions defined
    wasmtime_linker_t *linker = wasmtime_linker_new(benchmark.engine);
    wasmtime_error_t *error = nullptr;
    if (benchmark.runConfig.wasi)
        error = wasmtime_linker_define_wasi(linker);
    if (error)
        exit_with_error("failed to link wasi", error, nullptr);


    // Compile our modules
    wasmtime_module_t *module = nullptr;
    if (benchmark.runConfig.phi) {
        error = wasmtime_module_new(benchmark.engine, (uint8_t*)benchmark.phiBinary.data(), benchmark.phiBinary.size(), &module);
    } else {
        if (benchmark.runConfig.optimize) {
            auto optBinary = phi::justOptimize(benchmark.binary);
            error = wasmtime_module_new(benchmark.engine, (uint8_t*)optBinary.data(), optBinary.size(), &module);
        } else {
            error = wasmtime_module_new(benchmark.engine, (uint8_t*)benchmark.binary.data(), benchmark.binary.size(), &module);
        }
    }
    if (!module)
        exit_with_error("failed to compile module", error, nullptr);

    if (benchmark.runConfig.phi) {
        wasm_functype_t *callbackType = wasm_functype_new_0_0();
        error = wasmtime_linker_define_func(linker, "env", 3, "host", 4, callbackType, hostFunc, nullptr, nullptr);
        if (error)
            exit_with_error("failed to add import callback", error, nullptr);
    }


    wasm_trap_t *trap = nullptr;
    if (benchmark.runConfig.wasi) {
        // Instantiate wasi
        wasi_config_t *wasi_config = wasi_config_new();
        assert(wasi_config);
        wasi_config_inherit_argv(wasi_config);
        wasi_config_inherit_env(wasi_config);
        wasi_config_inherit_stdin(wasi_config);
        wasi_config_inherit_stdout(wasi_config);
        wasi_config_inherit_stderr(wasi_config);
        error = wasmtime_context_set_wasi(context, wasi_config);
        if (error)
            exit_with_error("failed to instantiate WASI", error, nullptr);
    }

    // Instantiate the module
    error = wasmtime_linker_module(linker, context, "", 0, module);
    if (error)
        exit_with_error("failed to instantiate module", error, nullptr);

    wasmtime_func_t func;
    error = wasmtime_linker_get_default(linker, context, "", 0, &func);
    if (error)
        exit_with_error("failed to locate default export for module", error, nullptr);

    COUNTER = 0;

    auto beforeCall = std::chrono::high_resolution_clock::now();
    error = wasmtime_func_call(context, &func, nullptr, 0, nullptr, 0, &trap);
    auto afterCall = std::chrono::high_resolution_clock::now();
    auto injection_ms = std::chrono::duration_cast<std::chrono::microseconds>(afterCall - beforeCall);
    if (benchmark.runConfig.verbose) {
        std::cout << "Run took " << injection_ms.count() << " μs." << std::endl;
    } else {
        std::cout << injection_ms.count() << std::endl;
    }
    if (error)
        exit_with_error("error calling default export", error, trap);

    std::cout << "Host called " << COUNTER << " times!" << std::endl;
    if (benchmark.runConfig.wasi) {
        int exitStatus;
        wasmtime_trap_exit_status(trap, &exitStatus);
            if (benchmark.runConfig.verbose) {
                std::cout << "exit status: " << exitStatus << std::endl;
            }
            if (exitStatus != 42) {
                exit_with_error("Exit status was not 42 :(", error, trap);
            }
    }

    wasmtime_linker_delete(linker);
    wasmtime_module_delete(module);
    wasmtime_store_delete(store);
}

static void exit_with_error(const char *message, wasmtime_error_t *error, wasm_trap_t *trap) {
    fprintf(stderr, "error: %s\n", message);
    wasm_byte_vec_t error_message;
    if (error != nullptr) {
        wasmtime_error_message(error, &error_message);
        wasmtime_error_delete(error);
    } else {
        wasm_trap_message(trap, &error_message);
        wasm_trap_delete(trap);
    }
    fprintf(stderr, "%.*s\n", (int) error_message.size, error_message.data);
    wasm_byte_vec_delete(&error_message);
    exit(1);
}


void doNormalBenchmarks(wasm_engine_t* engine, int n=100) {
    auto md5Benchmark = std::make_unique<Benchmark>(engine, "../../benchmarks/benchmark-files/md5.wasm");
    md5Benchmark->doFullTest(run, n, 200000);

    auto loopBenchmark = std::make_unique<Benchmark>(engine, "../../benchmarks/benchmark-files/count2mil.wasm");
    loopBenchmark->doFullTest(run, n, 10000000);

    auto iFFTBenchmark = std::make_unique<Benchmark>(engine, "../../benchmarks/benchmark-files/iFFT.wasm");
    iFFTBenchmark->doFullTest(run, n, 10000);
}

void doExponentialIntervalBenchmarks(wasm_engine_t* engine, int n=100, int levels=10) {
    auto loopBenchmark = std::make_unique<Benchmark>(engine, "../../benchmarks/benchmark-files/count2mil.wasm");
    std::cout << "%%%" << loopBenchmark->filename << ":  " << levels << " Intervals, exponentially (" << n << " times each)" << std::endl;

    // With Phi, 0 calls.
    loopBenchmark->applyPhi();
    loopBenchmark->runNTimes(run, n);

    int64_t interval = 10990000;

    for (int i=levels; i>0; i--) {
        // Call host i times.
        loopBenchmark->applyPhi(interval);
        loopBenchmark->runNTimes(run, n);
        interval >>= 1;
    }
}

int main(int argc, const char* argv[]) {
    wasm_engine_t *engine = wasm_engine_new();
//    doNormalBenchmarks(engine);
//    doIntervalBenchmarks(engine, 10);
//    doExponentialIntervalBenchmarks(engine, 10, 20);
    auto loopBenchmark = std::make_unique<Benchmark>(engine, "../../benchmarks/benchmark-files/count2bil.wasm");
    loopBenchmark->applyPhi(1000000000);
    loopBenchmark->runNTimes(run, 10);

    std::cout << "Done." << std::endl;
    return 0;
}
