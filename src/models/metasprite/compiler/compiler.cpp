/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "compiler.h"
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

// ::TODO generate debug file - containing frame/frameset names::

CompiledRomData::CompiledRomData()
    : paletteData("PD", "MS_PaletteData")
    , paletteList("PL", "MS_PaletteList", "PD")
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
    , _animationCompiler(_errorList)
    , _tileData("TB", "MS_TileBlock", tilesetBlockSize)
    , _tilesetData("TS", "DMA_Tile16Data")
    , _frameSetData("FSD", "MS_FrameSetData")
    , _frameSetList("FSL", "MS_FrameSetList", "FSD")
    , _frameSetReferences()
{
}

void Compiler::writeToIncFile(std::ostream& out) const
{
    out << "namespace MetaSprite {\n"
           "namespace Data {\n"
           "\n"
        << "constant EDITOR_VERSION = " << UNTECH_VERSION_INT << "\n"
        << "constant METASPRITE_FORMAT_VERSION = " << METASPRITE_FORMAT_VERSION << "\n";

    _animationCompiler.writeToIncFile(out);

    _tileData.writeToIncFile(out);
    _tilesetData.writeToIncFile(out);

    _compiledRomData.writeToIncFile(out);

    _frameSetData.writeToIncFile(out);
    _frameSetList.writeToIncFile(out);

    out << "constant FrameSetListCount = (pc() - FSL)/2\n";

    out << "}\n"
           "}\n";
}

void Compiler::writeToReferencesFile(std::ostream& out) const
{
    out << "namespace MSFS {\n";

    for (unsigned i = 0; i < _frameSetReferences.size(); i++) {
        const auto& r = _frameSetReferences[i];

        if (!r.isNull) {
            out << "\tconstant " << r.name << " = " << i << "\n";
            out << "\tdefine " << r.name << ".type = " << r.exportOrderName << "\n";
        }
    }

    out << "}\n"
           "namespace MSEO {\n";

    for (const auto& it : _project.exportOrders) {
        const FrameSetExportOrder* eo = it.value.get();
        if (eo == nullptr) {
            throw std::runtime_error("Unable to read Export Order: " + it.filename);
        }

        out << "\tnamespace " << eo->name << " {\n";

        if (eo->stillFrames.size() > 0) {
            unsigned id = 0;
            out << "\t\tnamespace Frames {\n";
            for (const auto& f : eo->stillFrames) {
                out << "\t\t\tconstant " << f.name << " = " << id << "\n";
                id++;
            }
            out << "\t\t}\n";
        }
        if (eo->animations.size() > 0) {
            unsigned id = 0;
            out << "\t\tnamespace Animations {\n";
            for (const auto& a : eo->animations) {
                out << "\t\t\tconstant " << a.name << " = " << id << "\n";
                id++;
            }
            out << "\t\t}\n";
        }
        out << "\t}\n";
    }

    out << "}\n";
}

void Compiler::processNullFrameSet()
{
    _frameSetList.addNull();
    _frameSetReferences.emplace_back();
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
        RomOffsetPtr fsAnimations = _animationCompiler.process(exportList);

        const auto framesData = processFrameList(exportList, tilesetData, _errorList);

        RomOffsetPtr fsPalettes = savePalettes(paletteData, _compiledRomData);
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

        // add to references
        _frameSetReferences.emplace_back(frameSet.name, frameSet.exportOrder);
    }
    catch (const std::exception& ex) {
        _errorList.addError(frameSet, ex.what());
        processNullFrameSet();
    }
}
