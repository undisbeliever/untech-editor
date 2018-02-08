/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

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
