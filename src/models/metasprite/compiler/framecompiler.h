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
                         const FrameTilesetList& tilesets);

private:
    RomOffsetPtr processFrameObjects(const MetaSprite::FrameObject::list_t&,
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
