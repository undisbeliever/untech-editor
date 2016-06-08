#pragma once

#include "romdata.h"
#include "romtiledata.h"
#include "models/metasprite-format/framesetexportorder.h"
#include "models/metasprite-format/tilesettype.h"
#include "models/metasprite.h"
#include <array>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace UnTech {
namespace MetaSpriteCompiler {

typedef std::unordered_map<unsigned, std::vector<const MetaSprite::Frame*>> TileGraph_t;

class Compiler {
public:
    const static unsigned DEFAULT_TILE_BLOCK_SIZE = 8 * 1024;

public:
    Compiler(unsigned tilesetBlockSize = DEFAULT_TILE_BLOCK_SIZE);

    Compiler(const Compiler&) = delete;

    void writeToIncFile(std::ostream& out) const;

    void writeToReferencesFile(std::ostream& out) const;

    void processNullFrameSet();
    void processFrameSet(const MetaSprite::FrameSet& frameSet);

    const std::list<std::string>& errors() const { return _errors; }
    const std::list<std::string>& warnings() const { return _warnings; }

private:
    struct FrameListEntry {
        FrameListEntry(const MetaSprite::Frame* frame, bool hFlip, bool vFlip)
            : frame(frame)
            , hFlip(hFlip)
            , vFlip(vFlip)
        {
        }

        const MetaSprite::Frame* frame;
        const bool hFlip;
        const bool vFlip;
    };
    std::vector<FrameListEntry> generateFrameList(const MetaSprite::FrameSet&);

private:
    struct FrameTileset {
        RomOffsetPtr tilesetOffset;

        // the uint16_t matches the charattr bits of the frameobject data.
        std::unordered_map<unsigned, uint16_t> smallTilesetMap;
        std::unordered_map<unsigned, uint16_t> largeTilesetMap;
    };
    struct FrameTilesetList {
        MetaSpriteFormat::TilesetType tilesetType;

        // Holds the tileset data used by the FrameMap
        std::list<FrameTileset> tilesets;
        std::unordered_map<const MetaSprite::Frame*, FrameTileset&> frameMap;
    };

private:
    FrameTilesetList generateTilesetList(const MetaSprite::FrameSet& frameSet,
                                         const std::vector<FrameListEntry>& frameList);

    FrameTilesetList generateDynamicTilesets(const MetaSprite::FrameSet& frameSet,
                                             const MetaSpriteFormat::TilesetType& tilesetType,
                                             const TileGraph_t& largeTileGraph,
                                             const TileGraph_t& smallTileGraph);

    FrameTilesetList generateFixedTileset(const MetaSprite::FrameSet& frameSet,
                                          const MetaSpriteFormat::TilesetType& tilesetType,
                                          const TileGraph_t& largeTileGraph,
                                          const TileGraph_t& smallTileGraph);

    void buildTileset(FrameTileset& tileset,
                      const MetaSprite::FrameSet& frameSet,
                      const MetaSpriteFormat::TilesetType& tilesetType,
                      const std::set<unsigned>& largeTiles,
                      const std::set<std::array<unsigned, 4>>& smallTiles);

    void buildTileset(FrameTileset& tileset,
                      const MetaSprite::FrameSet& frameSet,
                      const MetaSpriteFormat::TilesetType& tilesetType,
                      const std::set<unsigned>& largeTiles,
                      const std::set<unsigned>& smallTiles);

private:
    RomOffsetPtr processFrameObjects(const MetaSprite::FrameObject::list_t&,
                                     const FrameTileset&);

    RomOffsetPtr processEntityHitboxes(const MetaSprite::EntityHitbox::list_t&);

    RomOffsetPtr processTileCollisionHitbox(const MetaSprite::Frame&);

    RomOffsetPtr processActionPoints(const MetaSprite::ActionPoint::list_t&);

    // Returns the data offset in `_frameData`
    uint32_t processFrame(const MetaSprite::Frame&,
                          const FrameTileset& tileset);

    RomOffsetPtr processFrameList(const std::vector<FrameListEntry>& frameList,
                                  const FrameTilesetList& tilesets);

private:
    RomOffsetPtr processPalette(const MetaSprite::FrameSet& frameSet);

private:
    void addError(const std::string& message);
    void addError(const MetaSprite::FrameSet& frameSet, const std::string& message);
    void addError(const MetaSprite::Frame& frameSet, const std::string& message);
    void addWarning(const std::string& message);
    void addWarning(const MetaSprite::FrameSet& frameSet, const std::string& message);
    void addWarning(const MetaSprite::Frame& frame, const std::string& message);

private:
    RomIncData _frameSetData;
    RomAddrTable _frameSetList;

    RomIncData _frameData;
    RomAddrTable _frameList;

    // ::TODO animation data::

    RomBinData _paletteData;
    RomAddrTable _paletteList;

    RomTileData _tileData;
    RomIncData _tilesetData;

    RomBinData _frameObjectData;
    RomBinData _tileHitboxData;
    RomBinData _entityHitboxData;
    RomBinData _actionPointData;

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
    typedef MetaSpriteFormat::FrameSetExportOrder::ExportOrderDocument ExportOrderDocument;

    std::vector<FrameSetReference> _frameSetReferences;
    std::unordered_set<std::shared_ptr<const ExportOrderDocument>> _exportOrderDocuments;

    std::list<std::string> _errors;
    std::list<std::string> _warnings;
};
}
}
