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


    // Replace current expression with block that contains subtraction, than current expression.
    void subtractBeforeCurrent(Expression* curr);
    // Replace expr with block that contains subtraction, than expr.
    void subtractBefore(Expression** expr);
    void subtractAfterCurrent(Expression* curr);
    void subtractAfter(Expression **expr);
    void checkBeforeCurrent(Expression* curr);
    void checkBefore(Expression** expr);
    // Get built counter subtraction "code".
    GlobalSet *buildCounterDecrease(Builder &builder) const;
    // Get built counter check "code".
    If *buildCheck(Builder &builder) const;

public:
    static const constexpr char* PHI_GLOBAL_COUNTER_NAME = "_phi_global_counter";
    static const constexpr char* PHI_INJECTED_FUNCTION_NAME = "_phi_host_function";
    explicit PhiPass(int64_t interval);
    static void scan(PhiPass* self, Expression** currp);
    static void doSubtractBeforeCurrent(PhiPass* self, Expression** currp);
    static void doSubtractAfterCurrent(PhiPass* self, Expression** currp);
    static void doCheckBeforeCurrent(PhiPass* self, Expression** currp);
    void visitConst(Const* constExpr);
    void visitBinary(Binary* binaryExpr);
    void visitCall(Call* callExpr);
    void visitCallIndirect(CallIndirect* callIndirectExpr);
    void visitBreak(Break* breakExpr);
    void visitBlock(Block* blockExpr);
    void visitFunction(Function* functionExpr);
    void visitSwitch(Switch* switchExpr);
    void walkGlobal(Global* global);

};

//---------------------------------------------------------------------------
}   // namespace phi
//---------------------------------------------------------------------------

#endif //PHI_PASS_H
