#pragma once

#include "errorlist.h"
#include "framesetexportlist.h"
#include "romdata.h"
#include "models/metasprite.h"
#include <map>

namespace UnTech {
namespace MetaSpriteCompiler {

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
