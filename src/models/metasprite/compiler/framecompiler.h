/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "framesetexportlist.h"
#include "romdata.h"
#include "tilesetcompiler.h"
#include "../errorlist.h"
#include "../metasprite.h"
#include <cstdint>
#include <vector>

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

class FrameCompiler {
public:
    FrameCompiler(ErrorList& errorList);
    FrameCompiler(const FrameCompiler&) = delete;

    void writeToIncFile(std::ostream& out) const;

    RomOffsetPtr process(const FrameSetExportList& exportList,
                         const FrameSetTilesets& tilesets);

private:
    RomOffsetPtr processFrameObjects(const MetaSprite::Frame& frame,
                                     const FrameTileset&);
    RomOffsetPtr processEntityHitboxes(const MetaSprite::EntityHitbox::list_t&);
    RomOffsetPtr processTileHitbox(const MetaSprite::Frame& frame);
    RomOffsetPtr processActionPoints(const MetaSprite::ActionPoint::list_t&);

    uint32_t processFrame(const MetaSprite::Frame&, const FrameTileset&);

private:
    ErrorList& _errorList;

    RomIncData _frameData;
    RomAddrTable _frameList;

    RomBinData _frameObjectData;
    RomBinData _tileHitboxData;
    RomBinData _entityHitboxData;
    RomBinData _actionPointData;
};
}
}
}
