#ifndef PHI_BENCHMARK_H
#define PHI_BENCHMARK_H

#include <wasm.h>
#include <string>
#include <vector>

//---------------------------------------------------------------------------
namespace benchmarks{
//---------------------------------------------------------------------------

struct Benchmark {
    wasm_engine_t *engine;
    std::string filename;
    std::vector<char> binary;
    std::vector<char> phiBinary;


    Benchmark(wasm_engine_t* engine, std::string&& filename);   // Constructor.

    struct RunConfig {
        bool phi = false;
        bool optimize=false;
        bool verbose=false;
        bool wasi=true;
    };

    RunConfig runConfig;


    void applyPhi(int64_t interval = 1000000000);
    void runNTimes(void func(Benchmark&), int n);
    void doFullTest(void func(Benchmark&), int n=100, int64_t lowInterval=50000, int64_t highInterval=1000000000);

private:
    std::vector<char> readBinary();      // Read the binary wasm file into `binary`.
};
//---------------------------------------------------------------------------
}   // namespace benchmarks
//---------------------------------------------------------------------------

#endif //PHI_BENCHMARK_H
