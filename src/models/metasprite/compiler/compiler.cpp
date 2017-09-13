/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "compiler.h"
#include "version.h"
#include <algorithm>
#include <climits>
#include <set>

namespace MS = UnTech::MetaSprite::MetaSprite;
using Compiler = UnTech::MetaSprite::Compiler::Compiler;

const unsigned Compiler::METASPRITE_FORMAT_VERSION = 27;

// ::TODO generate debug file - containing frame/frameset names::

Compiler::Compiler(ErrorList& errorList, unsigned tilesetBlockSize)
    : _errorList(errorList)
    , _animationCompiler(_errorList)
    , _paletteCompiler()
    , _tilesetCompiler(_errorList, tilesetBlockSize)
    , _frameCompiler(_errorList)
    , _frameSetData("FSD", "MS_FrameSetData")
    , _frameSetList("FSL", "MS_FrameSetList", "FSD")
    , _frameSetReferences()
    , _exportOrderDocuments()
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
    _paletteCompiler.writeToIncFile(out);
    _tilesetCompiler.writeToIncFile(out);
    _frameCompiler.writeToIncFile(out);

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

    for (const auto& eo : _exportOrderDocuments) {
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
    if (frameSet.exportOrder == nullptr) {
        _errorList.addError(frameSet, "No frameset export order");
        return processNullFrameSet();
    }

    try {
        FrameSetExportList exportList(frameSet);

        FrameTilesetList tilesets = _tilesetCompiler.generateTilesetList(exportList);

        RomOffsetPtr fsPalettes = _paletteCompiler.process(frameSet);
        RomOffsetPtr fsFrames = _frameCompiler.process(exportList, tilesets);
        RomOffsetPtr fsAnimations = _animationCompiler.process(exportList);

        // FRAMESET DATA
        // -------------
        RomIncItem frameSetItem;

        unsigned nPalettes = frameSet.palettes.size();

        frameSetItem.addIndex(fsPalettes);                  // paletteTable
        frameSetItem.addField(RomIncItem::BYTE, nPalettes); // nPalettes

        // tileset
        if (tilesets.tilesets.size() == 1) {
            frameSetItem.addAddr(tilesets.tilesets.front().tilesetOffset);
        }
        else {
            frameSetItem.addField(RomIncItem::ADDR, 0);
        }

        frameSetItem.addField(RomIncItem::BYTE, tilesets.tilesetType.romValue()); // tilesetType
        frameSetItem.addIndex(fsFrames);                                          // frameTable
        frameSetItem.addField(RomIncItem::BYTE, exportList.frames().size());      // nFrames
        frameSetItem.addIndex(fsAnimations);                                      // animationsTable
        frameSetItem.addField(RomIncItem::BYTE, exportList.animations().size());  // nAnimations

        RomOffsetPtr ptr = _frameSetData.addData(frameSetItem);
        _frameSetList.addOffset(ptr.offset);

        // add to references
        _frameSetReferences.emplace_back(frameSet.name,
                                         frameSet.exportOrder->name);

        auto it = std::find(_exportOrderDocuments.begin(), _exportOrderDocuments.end(), frameSet.exportOrder);
        if (it == _exportOrderDocuments.end()) {
            _exportOrderDocuments.push_back(frameSet.exportOrder);
        }
    }
    catch (const std::exception& ex) {
        _errorList.addError(frameSet, ex.what());
        processNullFrameSet();
    }
}
