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

namespace MS = UnTech::MetaSprite::MetaSprite;
using namespace UnTech::MetaSprite::Compiler;

const unsigned Compiler::METASPRITE_FORMAT_VERSION = 32;

CompiledRomData::CompiledRomData()
    : paletteData("PD", "MS_PaletteData")
    , paletteList("PL", "MS_PaletteList", "PD")
    , animationData("AD", "MS_AnimationData")
    , animationList("AL", "MS_AnimationList", "AD")
    , frameData("FD", "MS_FrameData")
    , frameList("FL", "MS_FrameList", "FD")
    , frameObjectData("FO", "MS_FrameObjectsData", true)
    , tileHitboxData("TC", "MS_TileHitboxData", true)
    , entityHitboxData("EH", "MS_EntityHitboxData", true)
    , actionPointData("AP", "MS_ActionPointsData", true)
{
}

void CompiledRomData::writeToIncFile(std::ostream& out) const
{
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
}

Compiler::Compiler(const Project& project, ErrorList& errorList, unsigned tilesetBlockSize)
    : _project(project)
    , _errorList(errorList)
    , _compiledRomData()
    , _tileData("TB", "MS_TileBlock", tilesetBlockSize)
    , _tilesetData("TS", "DMA_Tile16Data")
    , _frameSetData("FSD", "MS_FrameSetData")
    , _frameSetList("FSL", "MS_FrameSetList", "FSD")
{
}

void Compiler::writeToIncFile(std::ostream& out) const
{
    out << "namespace MetaSprite {\n"
           "namespace Data {\n"
           "\n"
        << "constant EDITOR_VERSION = " << UNTECH_VERSION_INT << "\n"
        << "constant METASPRITE_FORMAT_VERSION = " << METASPRITE_FORMAT_VERSION << "\n";

    _tileData.writeToIncFile(out);
    _tilesetData.writeToIncFile(out);

    _compiledRomData.writeToIncFile(out);

    _frameSetData.writeToIncFile(out);
    _frameSetList.writeToIncFile(out);

    out << "constant FrameSetListCount = (pc() - FSL)/2\n";

    out << "}\n"
           "}\n";
}

void Compiler::processNullFrameSet()
{
    _frameSetList.addNull();
}

void Compiler::processFrameSet(const MS::FrameSet& frameSet)
{
    if (frameSet.validate(_errorList) == false) {
        return processNullFrameSet();
    }

    try {
        FrameSetExportList exportList(_project, frameSet);

        const auto tilesetLayout = layoutTiles(frameSet, exportList.frames(), _errorList);
        const auto tilesetData = insertFrameSetTiles(frameSet, tilesetLayout, _tileData, _tilesetData);

        const auto paletteData = processPalettes(frameSet.palettes);
        const auto animationsData = processAnimations(exportList, _errorList);

        const auto framesData = processFrameList(exportList, tilesetData, _errorList);

        RomOffsetPtr fsPalettes = savePalettes(paletteData, _compiledRomData);
        RomOffsetPtr fsAnimations = saveAnimations(animationsData, _compiledRomData);
        const auto frameTableAddr = saveCompiledFrames(framesData, _compiledRomData);

        // FRAMESET DATA
        // -------------
        RomIncItem frameSetItem;

        unsigned nPalettes = frameSet.palettes.size();

        frameSetItem.addIndex(fsPalettes);                                           // paletteTable
        frameSetItem.addField(RomIncItem::BYTE, nPalettes);                          // nPalettes
        frameSetItem.addAddr(tilesetData.staticTileset.romPtr);                      // tileset
        frameSetItem.addField(RomIncItem::BYTE, tilesetData.tilesetType.romValue()); // tilesetType
        frameSetItem.addIndex(frameTableAddr);                                       // frameTable
        frameSetItem.addField(RomIncItem::BYTE, exportList.frames().size());         // nFrames
        frameSetItem.addIndex(fsAnimations);                                         // animationsTable
        frameSetItem.addField(RomIncItem::BYTE, exportList.animations().size());     // nAnimations

        RomOffsetPtr ptr = _frameSetData.addData(frameSetItem);
        _frameSetList.addOffset(ptr.offset);
    }
    catch (const std::exception& ex) {
        _errorList.addError(frameSet, ex.what());
        processNullFrameSet();
    }
}
