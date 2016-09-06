#pragma once

#include "animationcompiler.h"
#include "framecompiler.h"
#include "palettecompiler.h"
#include "tilesetcompiler.h"

#include "errorlist.h"
#include "romdata.h"
#include "models/metasprite.h"
#include <map>
#include <unordered_set>
#include <vector>

namespace UnTech {
namespace MetaSpriteCompiler {

class Compiler {
public:
    const static unsigned METASPRITE_FORMAT_VERSION;

public:
    Compiler(unsigned tilesetBlockSize = TilesetCompiler::DEFAULT_TILE_BLOCK_SIZE);

    Compiler(const Compiler&) = delete;

    void writeToIncFile(std::ostream& out) const;

    void writeToReferencesFile(std::ostream& out) const;

    void processNullFrameSet();
    void processFrameSet(const MetaSprite::FrameSet& frameSet);

    const ErrorList& errorList() const { return _errorList; }

private:
    ErrorList _errorList;

    AnimationCompiler _animationCompiler;
    PaletteCompiler _paletteCompiler;
    TilesetCompiler _tilesetCompiler;
    FrameCompiler _frameCompiler;

    RomIncData _frameSetData;
    RomAddrTable _frameSetList;

    struct FrameSetReference {
        bool isNull;
        const std::string name;
        const std::string exportOrderName;

        FrameSetReference()
            : isNull(true)
            , name()
            , exportOrderName()
        {
        }

        FrameSetReference(const std::string& name, const std::string& exportOrderName)
            : isNull(false)
            , name(name)
            , exportOrderName(exportOrderName)
        {
        }
    };
    typedef MetaSpriteCommon::FrameSetExportOrder::ExportOrderDocument ExportOrderDocument;

    std::vector<FrameSetReference> _frameSetReferences;
    std::unordered_set<std::shared_ptr<const ExportOrderDocument>> _exportOrderDocuments;
};
}
}
