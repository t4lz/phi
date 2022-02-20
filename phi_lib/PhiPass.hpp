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

    // Inject WASM code: Subtract accumulated cost (WASM const), if counter <= 0 call host and reset counter.
    // C++: reset accumulated cost.
    void injectCounterCheckBeforeCurrent(Expression* curr);

public:
    static const constexpr char* PHI_GLOBAL_COUNTER_NAME = "_phi_global_counter";
    static const constexpr char* PHI_INJECTED_FUNCTION_NAME = "_phi_host_function";
    explicit PhiPass(int64_t interval);
    void visitConst(Const* curr);
    void visitBinary(Binary* curr);
    void visitBreak(Break* curr);
    void visitBlock(Block* curr);
};

//---------------------------------------------------------------------------
}   // namespace phi
//---------------------------------------------------------------------------

#endif //PHI_PASS_H
