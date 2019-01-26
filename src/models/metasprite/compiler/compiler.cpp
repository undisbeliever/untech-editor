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
#include "models/common/errorlist.h"
#include "models/metasprite/utsi2utms/utsi2utms.h"
#include "models/project/project.h"
#include <algorithm>
#include <climits>

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

namespace MS = UnTech::MetaSprite::MetaSprite;

const int CompiledRomData::METASPRITE_FORMAT_VERSION = 34;

struct FrameSetData {
    std::vector<CompiledPalette> palettes;
    IndexPlusOne staticTileset;
    TilesetType tilesetType;
    std::vector<FrameData> frames;
    std::vector<std::vector<uint8_t>> animations;
};

CompiledRomData::CompiledRomData(unsigned tilesetBlockSize)
    : tileData("Project.MS_TB", tilesetBlockSize)
    , tilesetData("Project.DMA_Tile16Data", "Project.MS_TB")
    , paletteData("Project.MS_PaletteData")
    , paletteList("Project.MS_PaletteList", "Project.MS_PaletteData")
    , animationData("Project.MS_AnimationData")
    , animationList("Project.MS_AnimationList")
    , frameData("Project.MS_FrameData")
    , frameList("Project.MS_FrameList")
    , frameObjectData("Project.MS_FrameObjectsData", true)
    , tileHitboxData("Project.MS_TileHitboxData", true)
    , entityHitboxData("Project.MS_EntityHitboxData", true)
    , actionPointData("Project.MS_ActionPointsData", true)
    , frameSetData("Project.MS_FrameSetData")
    , valid(true)
{
}

void CompiledRomData::writeToIncFile(std::ostream& out) const
{
    out << "constant Project.MS_FrameSetListCount = " << nFrameSets << "\n\n";

    tileData.writeAssertsToIncFile(out);
    tilesetData.writeToIncFile(out);

    paletteList.writeToIncFile(out);
}

// assumes frameSet.validate() passes
static FrameSetData processFrameSet(const FrameSetExportList& exportList, const TilesetData& tilesetData)
{
    const MS::FrameSet& frameSet = exportList.frameSet;

    return {
        processPalettes(frameSet.palettes),
        tilesetData.staticTileset.tilesetIndex,
        tilesetData.tilesetType,
        processFrameList(exportList, tilesetData),
        processAnimations(exportList),
    };
}

static void saveFrameSet(const FrameSetData& data, CompiledRomData& out)
{
    const uint16_t fsPalettes = savePalettes(data.palettes, out);
    const uint16_t fsAnimations = saveAnimations(data.animations, out);
    const uint16_t frameTableAddr = saveCompiledFrames(data.frames, out);

    DataBlock fsItem(12);

    fsItem.addWord(fsPalettes);                  // paletteTable
    fsItem.addByte(data.palettes.size());        // nPalettes
    fsItem.addWord(data.staticTileset);          // tileset
    fsItem.addByte(data.tilesetType.romValue()); // tilesetType
    fsItem.addWord(frameTableAddr);              // frameTable
    fsItem.addByte(data.frames.size());          // nFrames
    fsItem.addWord(fsAnimations);                // animationsTable
    fsItem.addByte(data.animations.size());      // nAnimations

    assert(fsItem.atEnd());

    out.frameSetData.addData_NoIndex(fsItem.data());
}

static bool validateFrameSet(const MS::FrameSet& frameSet, const FrameSetExportOrder* exportOrder, ErrorList& errorList)
{
    if (exportOrder == nullptr) {
        errorList.addError("Missing MetaSprite Export Order Document");
        return false;
    }

    return frameSet.validate(errorList)
           && exportOrder->testFrameSet(frameSet, errorList);
}

void processAndSaveFrameSet(const MS::FrameSet& frameSet, const FrameSetExportOrder* exportOrder,
                            ErrorList& errorList, CompiledRomData& out)
{
    if (validateFrameSet(frameSet, exportOrder, errorList) == false) {
        return;
    }

    assert(exportOrder);
    const auto exportList = buildExportList(frameSet, *exportOrder);
    exportList.validate(errorList);

    const auto tilesetLayout = layoutTiles(frameSet, exportList.frames, errorList);
    const auto tilesetData = insertFrameSetTiles(frameSet, tilesetLayout, out);
    const auto data = processFrameSet(exportList, tilesetData);
    saveFrameSet(data, out);
}

bool validateFrameSetAndBuildTilesets(const MetaSprite::FrameSet& frameSet, const FrameSetExportOrder* exportOrder,
                                      ErrorList& errorList)
{
    const size_t oldErrorCount = errorList.errorCount();

    if (validateFrameSet(frameSet, exportOrder, errorList) == false) {
        return false;
    }

    const FrameSetExportList exportList = buildExportList(frameSet, *exportOrder);
    exportList.validate(errorList);

    layoutTiles(frameSet, exportList.frames, errorList);

    return errorList.errorCount() == oldErrorCount;
}

std::unique_ptr<CompiledRomData> compileMetaSprites(const Project::ProjectFile& project, std::ostream& errorStream)
{
    bool valid = true;
    auto romData = std::make_unique<CompiledRomData>(project.blockSettings.size);

    romData->nFrameSets = project.frameSets.size();

    for (auto& fs : project.frameSets) {
        UnTech::ErrorList errorList;

        if (fs.msFrameSet) {
            const auto* exportOrder = project.frameSetExportOrders.find(fs.msFrameSet->exportOrder);
            processAndSaveFrameSet(*fs.msFrameSet, exportOrder, errorList, *romData);
        }
        else if (fs.siFrameSet) {
            Utsi2Utms converter(errorList);
            auto msFrameSet = converter.convert(*fs.siFrameSet);
            if (msFrameSet) {
                const auto* exportOrder = project.frameSetExportOrders.find(msFrameSet->exportOrder);
                processAndSaveFrameSet(*msFrameSet, exportOrder, errorList, *romData);
            }
        }
        else {
            errorList.addWarning("Missing FrameSet");
        }

        if (!errorList.empty()) {
            errorStream << fs.displayName() << ":\n";
            errorList.printIndented(errorStream);
        }

        valid &= errorList.hasError() == false;
    }

    if (valid == false) {
        romData = nullptr;
    }

    return romData;
}

}
}
}
