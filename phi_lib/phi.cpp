#include "phi.hpp"

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

static const constexpr char* PHI_GLOBAL_COUNTER_NAME = "_phi_global_counter";

size_t inject(char *buff, size_t inputSize, size_t bufferSize, const int64_t interval, const char *importModuleName, const char *importBaseName) {
    auto module = BinaryenModuleRead(buff, inputSize);

    std::cout << "Input module:" << std::endl;
    BinaryenModulePrint(module);

    if (!BinaryenModuleValidate(module))
        // We rely on properties of a valid module to assert the security of the injection, so we do not accept invalid modules.
        return 0;

    // Currently the import always has the type none -> none (no params, no result).
    // TODO: Is this internal name safe? Can a collision happen? What happens on collision?
    BinaryenAddFunctionImport(module, "phi_injected_import", importModuleName, importBaseName, BinaryenTypeNone(), BinaryenTypeNone());
    // TODO: Check that global does not exist.
    BinaryenAddGlobal(module, PHI_GLOBAL_COUNTER_NAME, BinaryenTypeInt64(), true, BinaryenConst(module, BinaryenLiteralInt64(interval)));

    std::cout << "Output module:" << std::endl;
    BinaryenModulePrint(module);

    assert(BinaryenModuleValidate(module));
    return BinaryenModuleWrite(module, buff, bufferSize);   // Return output size.
    // TODO: Use BinaryenModuleAllocateAndWrite instead of BinaryenModuleWrite?
    //       Currently user code has to guess how big the result will be and allocate enough memory for that.
}
//---------------------------------------------------------------------------
}   // namespace phi
//---------------------------------------------------------------------------