#include "phi.hpp"
#include "PhiPass.hpp"

#include <iostream>
#include <binaryen-c.h>
#include <cassert>

//---------------------------------------------------------------------------
namespace phi{
//---------------------------------------------------------------------------


std::vector<std::byte>
inject(char *buff, size_t inputSize, const int64_t interval, std::string&& importModuleName, std::string&& importBaseName) {
    auto module = BinaryenModuleRead(buff, inputSize);

    std::cout << "Input module:" << std::endl;
    BinaryenModulePrint(module);

    if (!BinaryenModuleValidate(module))
        // We rely on properties of a valid module to assert the security of the injection, so we do not accept invalid modules.
        return {};

    auto phiPass = std::make_unique<PhiPass>(interval, std::move(importModuleName), std::move(importBaseName));

    PassRunner passRunner((Module*)module);
    passRunner.add(move(phiPass));
    passRunner.run();

    std::cout << "Output module:" << std::endl;
    BinaryenModulePrint(module);

    assert(BinaryenModuleValidate(module));
    auto result = BinaryenModuleAllocateAndWrite(module, nullptr);
    auto* moduleBuff = static_cast<std::byte*>(result.binary);
    std::vector<std::byte> moduleByteVector(moduleBuff, moduleBuff + result.binaryBytes);
    free(result.binary);
    return std::move(moduleByteVector);
}
//---------------------------------------------------------------------------
}   // namespace phi
//---------------------------------------------------------------------------