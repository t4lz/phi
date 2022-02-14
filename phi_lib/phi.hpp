#ifndef PHI_PHI_HPP
#define PHI_PHI_HPP

//---------------------------------------------------------------------------
#include <cstddef>

namespace phi{
//---------------------------------------------------------------------------

void hello();

// Overwrites the code in buff with new code, with injected function import, instruction count, and periodic calls.
// inputSize: Length of the code written in buff.
// outputSize: Length of allocated buffer.
// importModuleName: Module name of the injected imported function.
// importBaseName: Base name of the injected imported function.
size_t inject(char* buff, size_t inputSize, size_t bufferSize, const char* importModuleName, const char* importBaseName);

//---------------------------------------------------------------------------
}   // namespace pljit::lexer
//---------------------------------------------------------------------------

#endif //PHI_PHI_HPP
