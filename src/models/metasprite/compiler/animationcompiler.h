#pragma once

#include "framesetexportlist.h"
#include "romdata.h"
#include "../errorlist.h"
#include "../metasprite.h"
#include <map>

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

class AnimationCompiler {
public:
    AnimationCompiler(ErrorList& errorList);
    AnimationCompiler(const AnimationCompiler&) = delete;

    void writeToIncFile(std::ostream& out) const;

    RomOffsetPtr process(const FrameSetExportList&);

private:
    // Returns the data offset in `_animationData`
    uint32_t processAnimation(const AnimationListEntry& animation,
                              const MetaSprite::FrameSet&,
                              const std::map<const FrameListEntry, unsigned>& frameMap,
                              const std::map<const AnimationListEntry, unsigned>& animationMap);

private:
    ErrorList& _errorList;

    RomBinData _animationData;
    RomAddrTable _animationList;
};
}
}
}
