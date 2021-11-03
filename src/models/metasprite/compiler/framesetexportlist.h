/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "../frameset-exportorder.h"
#include "../metasprite.h"
#include <algorithm>
#include <cstdint>
#include <vector>

namespace UnTech::MetaSprite::Compiler {

struct ExportIndex {
    // index into `MS::FrameSet::frames` or `MS::FrameSet::animations`
    const unsigned fsIndex;

    const bool hFlip;
    const bool vFlip;

    bool operator==(const ExportIndex&) const = default;
};

struct FrameSetExportList {
    const std::vector<ExportIndex> animations;
    const std::vector<ExportIndex> frames;
};

// NOTE: Can return an invalid FrameSetExportList
FrameSetExportList buildExportList(const MetaSprite::FrameSet& frameSet,
                                   const FrameSetExportOrder& exportOrder);

}
