#include "PhiPass.hpp"

// TODO:
//  - Should we have a pointer to builder as a class member?
//  - Use wasm::Name for all names.

//---------------------------------------------------------------------------
namespace phi {
//---------------------------------------------------------------------------
    void PhiPass::visitConst(Const* curr) {
        accumulatedCost++;
    }

//---------------------------------------------------------------------------
    void PhiPass::visitBinary(Binary *curr) {
        accumulatedCost++;
    }

//---------------------------------------------------------------------------
    void PhiPass::visitBreak(Break* curr) {
        if (accumulatedCost) {  // TODO: Cann accumulatedCost ever be 0? if not remove if.
            injectCounterCheckBeforeCurrent(curr);
        }
        accumulatedCost = 1;    // Reset all cost accumulated up till now, add 1 for break cost.
    }

//---------------------------------------------------------------------------
    void PhiPass::visitBlock(Block *curr) {
    }

//---------------------------------------------------------------------------
    PhiPass::PhiPass(int64_t interval) : interval(interval) {}  // Constructor.

//---------------------------------------------------------------------------
    void PhiPass::injectCounterCheckBeforeCurrent(Expression* curr) {
        Builder builder(*getModule());
        auto block = builder.blockify(
                // counter -= accumulatedCost.
                builder.makeGlobalSet(
                        PHI_GLOBAL_COUNTER_NAME,
                        builder.makeBinary(
                                SubInt64,
                                builder.makeGlobalGet(PHI_GLOBAL_COUNTER_NAME, Type::i64),
                                builder.makeConst(int64_t(accumulatedCost)
                                )
                        )
                ),
                builder.makeIf(                                     // if counter <= 0:
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
                )
        );
        block->list.push_back(curr);
        replaceCurrent(block);
    }

//---------------------------------------------------------------------------
}   // namespace phi
//---------------------------------------------------------------------------
