#include "phi.hpp"
#include "PhiPass.hpp"

#include <iostream>
#include <binaryen-c.h>
#include <cassert>

//---------------------------------------------------------------------------
namespace phi{
//---------------------------------------------------------------------------

std::vector<char>
inject(std::vector<char>& inputBytes, int64_t interval, std::string &&importModuleName,
       std::string &&importBaseName, bool optimize) {
    auto module = BinaryenModuleRead(inputBytes.data(), inputBytes.size());

//    std::cout << "Input module:" << std::endl;
//    BinaryenModulePrint(module);

    if (!BinaryenModuleValidate(module))
        // We rely on properties of a valid module to assert the security of the injection, so we do not accept invalid modules.
        return {};

    auto phiPass = std::make_unique<PhiPass>(interval, std::move(importModuleName), std::move(importBaseName));

    PassRunner passRunner((Module*)module);

    if (optimize) {
        passRunner.options.optimizeLevel = 4;
        passRunner.options.shrinkLevel = 0;
        passRunner.addDefaultOptimizationPasses();
    }
    passRunner.add(std::move(phiPass));
    passRunner.run();

//    std::cout << "Output module:" << std::endl;
//    BinaryenModulePrint(module);
    assert(BinaryenModuleValidate(module));

    auto result = BinaryenModuleAllocateAndWrite(module, nullptr);
    auto* moduleBuff = static_cast<char*>(result.binary);
    std::vector<char> moduleByteVector(moduleBuff, moduleBuff + result.binaryBytes);
    free(result.binary);
    return std::move(moduleByteVector);
}

//---------------------------------------------------------------------------
std::vector<char> justOptimize(std::vector<char>& inputBytes){
    auto module = BinaryenModuleRead(inputBytes.data(), inputBytes.size());
    PassRunner passRunner((Module*)module);
    passRunner.options.optimizeLevel = 4;
    passRunner.options.shrinkLevel = 0;
    passRunner.addDefaultOptimizationPasses();
    passRunner.run();

    auto result = BinaryenModuleAllocateAndWrite(module, nullptr);
    auto* moduleBuff = static_cast<char*>(result.binary);
    std::vector<char> moduleByteVector(moduleBuff, moduleBuff + result.binaryBytes);
    free(result.binary);
    return std::move(moduleByteVector);
}
//---------------------------------------------------------------------------
}   // namespace phi
//---------------------------------------------------------------------------