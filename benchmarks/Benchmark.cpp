#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>
#include "Benchmark.hpp"
#include "../phi_lib/phi.hpp"
//---------------------------------------------------------------------------
namespace benchmarks{
//---------------------------------------------------------------------------

    std::vector<char> Benchmark::readBinary() {
        // Load binaryVec.
        std::ifstream file(filename);
        file.seekg(0, std::ios_base::end);
        auto file_size = file.tellg();
        file.seekg(0);
        auto binaryVec = std::vector<char>(file_size);
        file.read(binaryVec.data(), file_size);
        file.close();
        if (file.fail()) {
            std::cout << "> Error loading module!" << std::endl;
            exit(1);
        }
        return std::move(binaryVec);
    }

    Benchmark::Benchmark(wasm_engine_t *engine, std::string &&filename)
    : engine(engine), filename(std::move(filename)), binary(readBinary()) {}


    void Benchmark::applyPhi(int64_t interval) { // default is 1,000,000,000
        // Perform Phi injection.
        auto beforeInject = std::chrono::high_resolution_clock::now();
        phiBinary = phi::inject(binary, interval, "env", "host", runConfig.optimize);
        auto afterInject = std::chrono::high_resolution_clock::now();
        if (runConfig.verbose) {
            auto injection_ms = std::chrono::duration_cast<std::chrono::milliseconds>(afterInject - beforeInject);
            std::cout << "Phi injection completed in " << injection_ms.count() << " ms." << std::endl;
        }
        runConfig.phi = true;
        runConfig.recompile = true;
    }

    void Benchmark::doFullTest(void (*func)(Benchmark &, size_t), int n, int64_t lowInterval, int64_t highInterval) {
        std::cout << "###" << filename << " (" << n << " times)" << std::endl;

        // ----- No extra binaryen passes -----
        func(*this, n);

        // With Phi, no call.
        applyPhi(highInterval);
        func(*this, n);

        // With Phi including call.
        applyPhi(lowInterval);
        func(*this, n);

        // ----- With extra binaryen passes -----
        runConfig.optimize = true;
        runConfig.phi = false;
        runConfig.recompile = true;
        func(*this, n);

        // With Phi, no call.
        applyPhi(highInterval);
        func(*this, n);

        // With Phi including call.
        applyPhi(lowInterval);
        func(*this, n);
    }

    void
    Benchmark::testIntervals(void (*func)(Benchmark &, size_t), int n, int64_t lowInterval, int levels) {
        std::cout << "%%%" << filename << ":  " << levels << " Intervals (" << n << " times each)" << std::endl;

        // With Phi, 0 calls.
        applyPhi();
        func(*this, n);

        for (int i=1; i<levels; i++) {
            // Call host i times.
            applyPhi(lowInterval/i);
            func(*this, n);
        }
    }

    void
    Benchmark::testIntervalsExponentially(void (*func)(Benchmark &, size_t), int n, int64_t lowInterval, int levels) {
        std::cout << "%%%" << filename << ":  " << levels << " Intervals, exponentially (" << n << " times each)" << std::endl;

        // With Phi, 0 calls.
        applyPhi();
        func(*this, n);

        for (int i=levels; i>0; i--) {
            // Call host i times.
            applyPhi(lowInterval*(1 << i));
            lowInterval++;
            func(*this, n);
        }
    }

//---------------------------------------------------------------------------
}   // namespace benchmarks
//---------------------------------------------------------------------------
