/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "framesetexportlist.h"
#include "romdata.h"
#include <cstdint>
#include <vector>

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

struct CompiledRomData;

struct TilesetData;

struct FrameData {
    std::vector<uint8_t> frameObjects;
    std::vector<uint8_t> entityHitboxes;
    std::vector<uint8_t> tileHitbox;
    std::vector<uint8_t> actionPoints;
    RomOffsetPtr tileset;
    bool isNull;
};

std::vector<FrameData> processFrameList(const FrameSetExportList& exportList,
                                        const TilesetData& tilesetData);

RomOffsetPtr saveCompiledFrames(const std::vector<FrameData>& frameData, CompiledRomData& out);

}
}
}
