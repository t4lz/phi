#include "phi.hpp"

#include <iostream>
#include <binaryen-c.h>


//---------------------------------------------------------------------------
namespace phi{
//---------------------------------------------------------------------------

void binaryen_hello_world() {
    BinaryenModuleRef module = BinaryenModuleCreate();

    // Create a function type for  i32 (i32, i32)
    BinaryenType ii[2] = {BinaryenTypeInt32(), BinaryenTypeInt32()};
    BinaryenType params = BinaryenTypeCreate(ii, 2);
    BinaryenType results = BinaryenTypeInt32();

    // Get the 0 and 1 arguments, and add them
    BinaryenExpressionRef x = BinaryenLocalGet(module, 0, BinaryenTypeInt32()),
            y = BinaryenLocalGet(module, 1, BinaryenTypeInt32());
    BinaryenExpressionRef add = BinaryenBinary(module, BinaryenAddInt32(), x, y);

    // Create the add function
    // Note: no additional local variables
    // Note: no basic blocks here, we are an AST. The function body is just an
    // expression node.
    BinaryenFunctionRef adder =
            BinaryenAddFunction(module, "adder", params, results, NULL, 0, add);

    // Print it out
    BinaryenModulePrint(module);

    // Clean up the module, which owns all the objects we created above
    BinaryenModuleDispose(module);

}

void hello() {
    binaryen_hello_world();
}

//---------------------------------------------------------------------------
size_t inject(char *buff, size_t inputSize, size_t bufferSize, const char *importModuleName, const char *importBaseName) {
    auto module = BinaryenModuleRead(buff, inputSize);
    std::cout << "Input module:" << std::endl;
    BinaryenModulePrint(module);
    return inputSize;
}
//---------------------------------------------------------------------------
}   // namespace pljit::lexer
//---------------------------------------------------------------------------