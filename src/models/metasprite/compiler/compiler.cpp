/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "compiler.h"
#include "animationcompiler.h"
#include "framecompiler.h"
#include "palettecompiler.h"
#include "tilesetinserter.h"
#include "tilesetlayout.h"
#include "version.h"
#include "models/metasprite/project.h"
#include <algorithm>
#include <climits>

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

namespace MS = UnTech::MetaSprite::MetaSprite;

constexpr unsigned METASPRITE_FORMAT_VERSION = 32;

struct FrameSetData {
    std::vector<CompiledPalette> palettes;
    RomOffsetPtr staticTileset;
    TilesetType tilesetType;
    std::vector<FrameData> frames;
    std::vector<std::vector<uint8_t>> animations;
};

CompiledRomData::CompiledRomData(unsigned tilesetBlockSize)
    : tileData("TB", "MS_TileBlock", tilesetBlockSize)
    , tilesetData("TS", "DMA_Tile16Data")
    , paletteData("PD", "MS_PaletteData")
    , paletteList("PL", "MS_PaletteList", "PD")
    , animationData("AD", "MS_AnimationData")
    , animationList("AL", "MS_AnimationList", "AD")
    , frameData("FD", "MS_FrameData")
    , frameList("FL", "MS_FrameList", "FD")
    , frameObjectData("FO", "MS_FrameObjectsData", true)
    , tileHitboxData("TC", "MS_TileHitboxData", true)
    , entityHitboxData("EH", "MS_EntityHitboxData", true)
    , actionPointData("AP", "MS_ActionPointsData", true)
    , frameSetData("FSD", "MS_FrameSetData")
    , frameSetList("FSL", "MS_FrameSetList", "FSD")
{
}

void CompiledRomData::writeToIncFile(std::ostream& out) const
{
    out << "namespace MetaSprite {\n"
           "namespace Data {\n"
           "\n"
        << "constant EDITOR_VERSION = " << UNTECH_VERSION_INT << "\n"
        << "constant METASPRITE_FORMAT_VERSION = " << METASPRITE_FORMAT_VERSION << "\n";

    tileData.writeToIncFile(out);
    tilesetData.writeToIncFile(out);

    paletteData.writeToIncFile(out);
    paletteList.writeToIncFile(out);

    animationData.writeToIncFile(out);
    animationList.writeToIncFile(out);

    frameObjectData.writeToIncFile(out);
    tileHitboxData.writeToIncFile(out);
    actionPointData.writeToIncFile(out);
    entityHitboxData.writeToIncFile(out);

    frameData.writeToIncFile(out);
    frameList.writeToIncFile(out);

    frameSetData.writeToIncFile(out);
    frameSetList.writeToIncFile(out);

    out << "constant FrameSetListCount = (pc() - FSL)/2\n";

    out << "}\n"
           "}\n";
}

// assumes frameSet.validate() passes
static FrameSetData processFrameSet(const FrameSetExportList& exportList, const TilesetData& tilesetData, ErrorList& errorList)
{
    const MS::FrameSet& frameSet = exportList.frameSet();

    return {
        processPalettes(frameSet.palettes),
        tilesetData.staticTileset.romPtr,
        tilesetData.tilesetType,
        processFrameList(exportList, tilesetData, errorList),
        processAnimations(exportList, errorList),
    };
}

static void saveFrameSet(const FrameSetData& data, CompiledRomData& out)
{
    const RomOffsetPtr fsPalettes = savePalettes(data.palettes, out);
    const RomOffsetPtr fsAnimations = saveAnimations(data.animations, out);
    const RomOffsetPtr frameTableAddr = saveCompiledFrames(data.frames, out);
    RomIncItem frameSetItem;

    frameSetItem.addIndex(fsPalettes);                                    // paletteTable
    frameSetItem.addField(RomIncItem::BYTE, data.palettes.size());        // nPalettes
    frameSetItem.addAddr(data.staticTileset);                             // tileset
    frameSetItem.addField(RomIncItem::BYTE, data.tilesetType.romValue()); // tilesetType
    frameSetItem.addIndex(frameTableAddr);                                // frameTable
    frameSetItem.addField(RomIncItem::BYTE, data.frames.size());          // nFrames
    frameSetItem.addIndex(fsAnimations);                                  // animationsTable
    frameSetItem.addField(RomIncItem::BYTE, data.animations.size());      // nAnimations

    RomOffsetPtr ptr = out.frameSetData.addData(frameSetItem);
    out.frameSetList.addOffset(ptr.offset);
}

static void saveNullFrameSet(CompiledRomData& out)
{
    out.frameSetList.addNull();
}

void processAndSaveFrameSet(const Project& project, const MS::FrameSet& frameSet, ErrorList& errorList, CompiledRomData& out)
{
    if (frameSet.validate(errorList) == false) {
        saveNullFrameSet(out);
        return;
    }

    try {
        FrameSetExportList exportList(project, frameSet);

        const auto tilesetLayout = layoutTiles(frameSet, exportList.frames(), errorList);
        const auto tilesetData = insertFrameSetTiles(frameSet, tilesetLayout, out);
        const auto data = processFrameSet(exportList, tilesetData, errorList);
        saveFrameSet(data, out);
    }
    catch (const std::exception& ex) {
        errorList.addError(frameSet, ex.what());
        saveNullFrameSet(out);
    }
}

void processProject(Project& project, ErrorList& errorList, CompiledRomData& out)
{
    for (auto& fs : project.frameSets) {
        fs.convertSpriteImporter(errorList);

        if (fs.msFrameSet) {
            processAndSaveFrameSet(project, *fs.msFrameSet, errorList, out);
        }
        else {
            saveNullFrameSet(out);
        }
    }
}

}
}
}
