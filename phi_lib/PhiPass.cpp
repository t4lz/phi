#include "PhiPass.hpp"

// TODO:
//  - Should we have a pointer to builder as a class member?
//  - Use wasm::Name for all names.

//---------------------------------------------------------------------------
namespace phi {
//---------------------------------------------------------------------------

    void PhiPass::visitBreak(Break* breakExpr) {
        accumulatedCost++;
        subtractBeforeCurrent(breakExpr);
    }

//---------------------------------------------------------------------------
    void PhiPass::visitBlock(Block *blockExpr) {
        // accumulatedCost holds the cost of linear instructions at the end of block.

        if (blockExpr->name.is()) {     // Only named blocks can be break targets in binaryen.
            // Important: Will not wrap in another block (that would be wrong). Will add subtraction to block.
            subtractAfterCurrent(blockExpr);
        }
    }

//---------------------------------------------------------------------------
    PhiPass::PhiPass(int64_t interval) : interval(interval) {}  // Constructor.

//---------------------------------------------------------------------------
    GlobalSet* PhiPass::buildCounterDecrease(Builder& builder) const {
        // counter -= <accumulatedCost>.
        return builder.makeGlobalSet(
                PHI_GLOBAL_COUNTER_NAME,
                builder.makeBinary(
                        SubInt64,
                        builder.makeGlobalGet(PHI_GLOBAL_COUNTER_NAME, Type::i64),
                        builder.makeConst(int64_t(accumulatedCost)
                        )
                )
        );
    }

//---------------------------------------------------------------------------
    // Replace current expression with block that contains subtraction, then current expression.
    void PhiPass::subtractBeforeCurrent(Expression* curr) {
        if (!accumulatedCost)
            return;
        // Only decrease counter, without performing check.
        Builder builder(*getModule());
        auto block = builder.blockify(
                buildCounterDecrease(builder),  // counter -= accumulatedCost.
                curr
        );
        replaceCurrent(block);
        accumulatedCost = 0;
    }

//---------------------------------------------------------------------------
    If* PhiPass::buildCheck(Builder& builder) const {
        // counter -= <accumulatedCost>.
        return builder.makeIf(                                     // if counter <= 0:
                builder.makeBinary(
                        LeSInt64,
                        builder.makeGlobalGet(PHI_GLOBAL_COUNTER_NAME, Type::i64),
                        builder.makeConst(int64_t(0))
                ),
                builder.blockify(
                        builder.makeCall(                       // then call host.
                                PHI_INJECTED_FUNCTION_NAME,
                                {},
                                Type::none
                        ),
                        builder.makeGlobalSet(              // counter := <interval>
                                PHI_GLOBAL_COUNTER_NAME,
                                builder.makeConst(int64_t(interval))
                        )
                )
        );
    }

//---------------------------------------------------------------------------
    void PhiPass::visitFunction(Function *functionExpr) {
        // TODO: what happens when body is nullptr? is it ok?

        // accumulatedCost contains any remaining cost from the end of the function.
        subtractAfter(&functionExpr->body);
        checkBefore(&functionExpr->body);
    }


//---------------------------------------------------------------------------
    void PhiPass::scan(PhiPass *self, Expression **currp) {
        auto* curr = *currp;
        switch (curr->_id) {
            case Expression::Id::IfId: {
                auto* ifExpr = curr->cast<If>();
                self->maybePushTask(PhiPass::doSubtractAfterCurrent, &ifExpr->ifFalse); // 6. Add subtraction after.
                self->maybePushTask(PhiPass::scan, &ifExpr->ifFalse);       // 5. Accumulate cost of IfFalse (+visit).
                self->pushTask(PhiPass::doSubtractAfterCurrent, &ifExpr->ifTrue);   // 4. Add subtraction after IfTrue.
                self->pushTask(PhiPass::scan, &ifExpr->ifTrue);             // 3. Accumulate cost of ifTrue (and visit).
                self->pushTask(PhiPass::doSubtractBeforeCurrent, currp);    // 2. Add subtraction before If.
                self->pushTask(PhiPass::scan, &ifExpr->condition);          // 1. Accumulate cost of condition.
                // No visitIf, these tasks do it all.
                break;
            }
            case Expression::Id::LoopId: {
                auto* loopExpr = curr->cast<Loop>();
                self->pushTask(PhiPass::doCheckBeforeCurrent, &loopExpr->body); // 3. Add check at start of body.
                self->pushTask(PhiPass::scan, &loopExpr->body);                 // 2. Walk (visit) body.
                self->pushTask(PhiPass::doSubtractBeforeCurrent, currp);        // 1. Add subtraction before Loop.
                // No visitLoop, this already covers everything.
                break;
            }
            default: {
                PostWalker<PhiPass>::scan(self, currp); // Do normal post-order walk.
            }
        }
    }

    void PhiPass::doSubtractBeforeCurrent(PhiPass *self, Expression **currp) {
        self->subtractBeforeCurrent(*currp);
    }

    void PhiPass::doSubtractAfterCurrent(PhiPass *self, Expression **currp) {
        self->subtractAfterCurrent(*currp);
    }

    // Replace current expression with block that contains current expression, then subtraction.
    // If current expression is block, will push subtraction to the end of the block.
    void PhiPass::subtractAfterCurrent(Expression *curr) {
        if (!accumulatedCost)
            return;
        Builder builder(*getModule());
        auto block = builder.blockify(
                curr,
                buildCounterDecrease(builder)  // counter -= accumulatedCost.
        );
        replaceCurrent(block);
        accumulatedCost = 0;
    }

    // Replace current expression with block that contains check, than current expression.
    void PhiPass::checkBeforeCurrent(Expression *curr) {
        Builder builder(*getModule());
        auto block = builder.blockify(buildCheck(builder), curr);
        replaceCurrent(block);
    }

    void PhiPass::doCheckBeforeCurrent(PhiPass *self, Expression **currp) {
        self->checkBeforeCurrent(*currp);
    }

    void PhiPass::walkGlobal(Global *global) { // Nothing. Don't accumulate cost for global defs. TODO: is this ok?
    }

    void PhiPass::visitCall(Call *callExpr) {
        // accumulatedCost already contains also the operands' cost.
        accumulatedCost += 4;   // Pay also cost of the call in advance.
        checkBeforeCurrent(callExpr);
    }

    void PhiPass::visitCallIndirect(CallIndirect *callIndirectExpr) {
        // accumulatedCost already contains also the operands' cost.
        accumulatedCost += 6;  // Pay also cost of the call in advance.
        checkBeforeCurrent(callIndirectExpr);
    }

    void PhiPass::subtractBefore(Expression **expr) {
        if (!accumulatedCost)
            return;
        // Only decrease counter, without performing check.
        Builder builder(*getModule());
        auto block = builder.blockify(
                buildCounterDecrease(builder),  // counter -= accumulatedCost.
                *expr
        );
        *expr = block;  // No worries about DebugLocation, since original code is still there.
        accumulatedCost = 0;
    }

    void PhiPass::checkBefore(Expression **expr) {
        Builder builder(*getModule());
        auto block = builder.blockify(buildCheck(builder), *expr);
        *expr = block;  // No worries about DebugLocation, since original code is still there.
    }

    void PhiPass::subtractAfter(Expression **expr) {
        if (!accumulatedCost)
            return;
        Builder builder(*getModule());
        *expr = builder.blockify(
                *expr,
                buildCounterDecrease(builder)  // counter -= accumulatedCost.
        );
        accumulatedCost = 0;
    }

    void PhiPass::visitSwitch(Switch *switchExpr) {
        accumulatedCost += 2;
        subtractBeforeCurrent(switchExpr);
    }

    void PhiPass::visitReturn(Return *returnExpr) {
        accumulatedCost += 100;
        subtractBeforeCurrent(returnExpr);
    }

    void PhiPass::visitCallRef(CallRef *callRefExpr) {
        // accumulatedCost already contains cost of operands.
        accumulatedCost += 5;  // Pay also cost of the call in advance.
        checkBeforeCurrent(callRefExpr);
    }

    // based on wasm-traversal.h form binaryen.
    // Define all instruction visits with cost 1:
#define VISIT(CLASS_TO_VISIT)                                               \
    void PhiPass::visit##CLASS_TO_VISIT(CLASS_TO_VISIT* curr){              \
        accumulatedCost++;                                                  \
    }

    // Define all instruction visits that have non-1 cost:
#define VISIT_COST(CLASS_TO_VISIT, cost)                                    \
    void PhiPass::visit##CLASS_TO_VISIT(CLASS_TO_VISIT* curr){              \
        accumulatedCost += cost;                                            \
    }

#include "visit-instructions.def"
//---------------------------------------------------------------------------
}   // namespace phi
//---------------------------------------------------------------------------
