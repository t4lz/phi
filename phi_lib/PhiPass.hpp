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
    Name internalFunctionName;
    Name globalName;

    std::string importModuleName;
    std::string importBaseName;


    Name& getInternalFunctionName();
    Name& getGlobalName();



    // Replace current expression with block that contains subtraction, than current expression.
    void subtractBeforeCurrent(Expression* curr);
    // Replace expr with block that contains subtraction, than expr.
    void subtractBefore(Expression** expr);
    void subtractAfterCurrent(Expression* curr);
    void subtractAfter(Expression **expr);
    void checkBeforeCurrent(Expression* curr);
    void checkBefore(Expression** expr);
    // Get built counter subtraction "code".
    GlobalSet *buildCounterDecrease(Builder &builder);
    // Get built counter check "code".
    If *buildCheck(Builder &builder);
    static const constexpr char* PHI_INJECTED_FUNCTION_NAME = "_phi_host_function";

public:
    static const constexpr char* PHI_GLOBAL_COUNTER_NAME = "_phi_global_counter";
    PhiPass(int64_t interval, std::string importModuleName, std::string importBaseName);
    static void scan(PhiPass* self, Expression** currp);
    static void doSubtractBeforeCurrent(PhiPass* self, Expression** currp);
    static void doSubtractAfterCurrent(PhiPass* self, Expression** currp);
    static void doCheckBeforeCurrent(PhiPass* self, Expression** currp);
    void visitCall(Call* callExpr);
    void visitCallIndirect(CallIndirect* callIndirectExpr);
    void visitCallRef(CallRef* callRefExpr);
    void visitBreak(Break* breakExpr);
    void visitBrOn(Break* breakExpr);
    void visitBlock(Block* blockExpr);
    void visitFunction(Function* functionExpr);
    void visitSwitch(Switch* switchExpr);
    void visitReturn(Return* returnExpr);
    void visitModule(Module* module);
    void walkGlobal(Global* global);

    // based on wasm-traversal.h form binaryen.
    // Declare all instruction visits:
#define VISIT(CLASS)                                               \
    void visit##CLASS(CLASS* curr);

    // Declare all instruction visits that have non-1 cost:
#define VISIT_COST(CLASS, cost)                                    \
    void visit##CLASS(CLASS* curr);

#include "visit-instructions.def"

};

//---------------------------------------------------------------------------
}   // namespace phi
//---------------------------------------------------------------------------

#endif //PHI_PASS_H
