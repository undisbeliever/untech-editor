/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "animationcompiler.h"
#include "romdata.h"
#include "romtiledata.h"
#include "../errorlist.h"
#include "../metasprite.h"
#include <vector>

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

struct CompiledRomData {
    RomBinData paletteData;
    RomAddrTable paletteList;

    RomIncData frameData;
    RomAddrTable frameList;

    RomBinData frameObjectData;
    RomBinData tileHitboxData;
    RomBinData entityHitboxData;
    RomBinData actionPointData;

    CompiledRomData();
    void writeToIncFile(std::ostream& out) const;
};

class Compiler {
public:
    const static unsigned METASPRITE_FORMAT_VERSION;

public:
    Compiler(const Project& project, ErrorList& errorList,
             unsigned tilesetBlockSize = RomTileData::DEFAULT_TILE_BLOCK_SIZE);

    Compiler(const Compiler&) = delete;

    void writeToIncFile(std::ostream& out) const;

    void writeToReferencesFile(std::ostream& out) const;

    void processNullFrameSet();
    void processFrameSet(const MetaSprite::FrameSet& frameSet);

    const ErrorList& errorList() const { return _errorList; }

private:
    const Project& _project;
    ErrorList& _errorList;

    CompiledRomData _compiledRomData;

    AnimationCompiler _animationCompiler;

    RomTileData _tileData;
    RomIncData _tilesetData;

    RomIncData _frameSetData;
    RomAddrTable _frameSetList;

    struct FrameSetReference {
        bool isNull;
        const idstring name;
        const idstring exportOrderName;

        FrameSetReference()
            : isNull(true)
            , name()
            , exportOrderName()
        {
        }

        FrameSetReference(const idstring& name, const idstring& exportOrderName)
            : isNull(false)
            , name(name)
            , exportOrderName(exportOrderName)
        {
        }
    };

    std::vector<FrameSetReference> _frameSetReferences;
};
}
}
}
