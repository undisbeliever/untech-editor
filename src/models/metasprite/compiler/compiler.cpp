/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "compiler.h"
#include "animationcompiler.h"
#include "framesetcompiler.h"
#include "palettecompiler.h"
#include "models/common/iterators.h"

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

namespace MS = UnTech::MetaSprite::MetaSprite;

const int CompiledRomData::METASPRITE_FORMAT_VERSION = 39;

CompiledRomData::CompiledRomData(const Project::MemoryMapSettings& memoryMap)
    : tileData(memoryMap)
    , dmaTile16Data("Project.DMA_Tile16Data")
    , paletteData("Project.MS_PaletteData")
    , paletteList("Project.MS_PaletteList", "Project.MS_PaletteData")
    , animationData("Project.MS_AnimationData")
    , animationList("Project.MS_AnimationList")
    , frameData("Project.MS_FrameData")
    , frameList("Project.MS_FrameList")
    , frameObjectData("Project.MS_FrameObjectsData", true)
    , entityHitboxData("Project.MS_EntityHitboxData", true)
    , actionPointData("Project.MS_ActionPointsData", true)
    , frameSetData("Project.MS_FrameSetData")
    , valid(true)
{
}

static IndexPlusOne
saveFrameTilesetData(const FrameTilesetData& tileset, const std::vector<uint16_t>& tiles,
                     CompiledRomData& out)
{
    assert(!tileset.tiles.empty());

    DataBlock data(1 + tileset.tiles.size() * 2);

    data.addByte(tileset.tiles.size()); // Tile16.count
    for (auto& t : tileset.tiles) {
        data.addWord(tiles.at(t)); // Tile16.addr
    }
    assert(data.atEnd());

    return out.dmaTile16Data.addData_IndexPlusOne(data.data());
}

static std::pair<IndexPlusOne, std::vector<IndexPlusOne>>
saveTilesetData(const TilesetData& tileset, CompiledRomData& out)
{
    assert(!tileset.tiles.empty());

    std::vector<uint16_t> tiles;
    for (auto [i, t] : const_enumerate(tileset.tiles)) {
        tiles.push_back(out.tileData.addLargeTile(t));
    }

    std::pair<IndexPlusOne, std::vector<IndexPlusOne>> ret;

    if (!tileset.staticTileset.tiles.empty()) {
        ret.first = saveFrameTilesetData(tileset.staticTileset, tiles, out);
    }

    for (auto& t : tileset.dynamicTilesets) {
        ret.second.push_back(saveFrameTilesetData(t, tiles, out));
    }

    return ret;
}

static uint16_t saveCompiledFrame(const FrameData& frameData,
                                  const std::vector<IndexPlusOne>& dynamicTilesets,
                                  CompiledRomData& out)
{
    const IndexPlusOne tilesetIndex = frameData.tileset ? dynamicTilesets.at(*frameData.tileset) : IndexPlusOne{ 0 };

    DataBlock frame(2 * 4 + 4);

    frame.addWord(out.frameObjectData.addData_IndexPlusOne(frameData.frameObjects));
    frame.addWord(out.entityHitboxData.addData_IndexPlusOne(frameData.entityHitboxes));
    frame.addWord(out.actionPointData.addData_IndexPlusOne(frameData.actionPoints));
    frame.addWord(tilesetIndex);

    frame.addByte(frameData.tileHitbox.left);
    frame.addByte(frameData.tileHitbox.right);
    frame.addByte(frameData.tileHitbox.yOffset);
    frame.addByte(frameData.tileHitbox.height);

    assert(frame.atEnd());

    return out.frameData.addData_Index(frame.data());
}

static uint16_t saveCompiledFrames(const std::vector<FrameData>& frames,
                                   const std::vector<IndexPlusOne>& dynamicTilesets,
                                   CompiledRomData& out)
{
    assert(frames.size() > 0);

    DataBlock table(frames.size() * 2);

    for (const auto& fd : frames) {
        table.addWord(saveCompiledFrame(fd, dynamicTilesets, out));
    }

    assert(table.atEnd());

    return out.frameList.addData_Index(table.data());
}

void CompiledRomData::addFrameSetData(const FrameSetData& data)
{
    const auto [staticTileset, dynamicTilesets] = saveTilesetData(data.tileset, *this);

    const uint16_t fsPalettes = savePalettes(data.palettes, *this);
    const uint16_t fsAnimations = saveAnimations(data.animations, *this);
    const uint16_t frameTableAddr = saveCompiledFrames(data.frames, dynamicTilesets, *this);

    DataBlock fsItem(12);

    fsItem.addWord(fsPalettes);                          // paletteTable
    fsItem.addByte(data.palettes.size());                // nPalettes
    fsItem.addWord(staticTileset);                       // tileset
    fsItem.addByte(data.tileset.tilesetType.romValue()); // tilesetType
    fsItem.addWord(frameTableAddr);                      // frameTable
    fsItem.addByte(data.frames.size());                  // nFrames
    fsItem.addWord(fsAnimations);                        // animationsTable
    fsItem.addByte(data.animations.size());              // nAnimations

    assert(fsItem.atEnd());

    frameSetData.addData_NoIndex(fsItem.data());
}

}
}
}
