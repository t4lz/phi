#include "phi.hpp"
#include "PhiPass.hpp"

#include <iostream>
#include <binaryen-c.h>
#include <cassert>

//---------------------------------------------------------------------------
namespace {
//---------------------------------------------------------------------------
void injectToFunction(BinaryenFunctionRef function) {
    auto functionBody = BinaryenFunctionGetBody(function);
}
//---------------------------------------------------------------------------
}   // Anonymous namespace.
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
namespace phi{
//---------------------------------------------------------------------------


std::vector<std::byte>
inject(char *buff, size_t inputSize, const int64_t interval, const char *importModuleName, const char *importBaseName) {
    auto module = BinaryenModuleRead(buff, inputSize);

    std::cout << "Input module:" << std::endl;
    BinaryenModulePrint(module);

    if (!BinaryenModuleValidate(module))
        // We rely on properties of a valid module to assert the security of the injection, so we do not accept invalid modules.
        return {};

    // Currently the import always has the type none -> none (no params, no result).
    // TODO: Is this internal name safe? Can a collision happen? What happens on collision?
    BinaryenAddFunctionImport(module, PhiPass::PHI_INJECTED_FUNCTION_NAME, importModuleName, importBaseName, BinaryenTypeNone(), BinaryenTypeNone());
    // TODO: Check that global does not exist.
    BinaryenAddGlobal(module, PhiPass::PHI_GLOBAL_COUNTER_NAME, BinaryenTypeInt64(), true, BinaryenConst(module, BinaryenLiteralInt64(interval)));

    auto phiPass = make_unique<PhiPass>(interval);

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