#ifndef PHI_PASS_H
#define PHI_PASS_H

#include <pass.h>
#include <wasm-builder.h>

//---------------------------------------------------------------------------
namespace phi {
//---------------------------------------------------------------------------
using namespace wasm;

class PhiPass : public WalkerPass<PostWalker<PhiPass>> {
public:
    int64_t accumulatedCost = 0;
    void visitConst(Const* curr);
};

//---------------------------------------------------------------------------
}   // namespace phi
//---------------------------------------------------------------------------

#endif //PHI_PASS_H
