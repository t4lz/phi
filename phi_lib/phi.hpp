#ifndef PHI_PHI_HPP
#define PHI_PHI_HPP

//---------------------------------------------------------------------------
#include <cstddef>
#include <cstdint>
#include <vector>

namespace phi{
//---------------------------------------------------------------------------

// inputSize: Length of the code written in buff.
// bufferSize: Length of allocated buffer.
// interval: Interval for periodic handover. Will call injected interval about every <interval> instructions.
// importModuleName: Module name of the injected imported function.
// importBaseName: Base name of the injected imported function.
    std::vector<std::byte>
    inject(char* buff, size_t inputSize, int64_t interval, const char* importModuleName, const char* importBaseName);

//---------------------------------------------------------------------------
}   // namespace phi
//---------------------------------------------------------------------------

#endif //PHI_PHI_HPP
