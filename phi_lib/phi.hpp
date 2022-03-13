#ifndef PHI_PHI_HPP
#define PHI_PHI_HPP

//---------------------------------------------------------------------------
#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>

namespace phi{
//---------------------------------------------------------------------------

// inputSize: Length of the code written in inputBytes.
// bufferSize: Length of allocated buffer.
// interval: Interval for periodic handover. Will call injected interval about every <interval> instructions.
// importModuleName: Module name of the injected imported function.
// importBaseName: Base name of the injected imported function.
std::vector<char>
inject(std::vector<char>& inputBytes, int64_t interval, std::string &&importModuleName,
       std::string &&importBaseName);

//---------------------------------------------------------------------------
}   // namespace phi
//---------------------------------------------------------------------------

#endif //PHI_PHI_HPP
