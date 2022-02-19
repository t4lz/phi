#ifndef PHI_PASS_H
#define PHI_PASS_H

#include <pass.h>
#include <wasm-builder.h>

//---------------------------------------------------------------------------
namespace phi {
//---------------------------------------------------------------------------
using namespace wasm;

class PhiPass : public WalkerPass<PostWalker<PhiPass>> {

    const int64_t interval;
    int64_t accumulatedCost = 0;

public:
    static const constexpr char* PHI_GLOBAL_COUNTER_NAME = "_phi_global_counter";
    static const constexpr char* PHI_INJECTED_FUNCTION_NAME = "_phi_host_function";
    PhiPass(int64_t interval);
    void visitConst(Const* curr);
    void visitBinary(Binary* curr);
    void visitBreak(Break* curr);
};

//---------------------------------------------------------------------------
}   // namespace phi
//---------------------------------------------------------------------------

#endif //PHI_PASS_H
