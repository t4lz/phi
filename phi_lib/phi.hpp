#ifndef PHI_PHI_HPP
#define PHI_PHI_HPP

//---------------------------------------------------------------------------
#include <cstddef>
#include <cstdint>

namespace phi{
//---------------------------------------------------------------------------

void hello();

// Overwrites the code in buff with new code, with injected function import, instruction count, and periodic calls.
// inputSize: Length of the code written in buff.
// bufferSize: Length of allocated buffer.
// interval: Interval for periodic handover. Will call injected interval about every <interval> instructions.
// importModuleName: Module name of the injected imported function.
// importBaseName: Base name of the injected imported function.
size_t inject(char* buff, size_t inputSize, size_t bufferSize, const int64_t interval, const char* importModuleName, const char* importBaseName);

//---------------------------------------------------------------------------
}   // namespace phi
//---------------------------------------------------------------------------

#endif //PHI_PHI_HPP
