#pragma once

#include "romdata.h"
#include "romtiledata.h"
#include "models/metasprite-common/framesetexportorder.h"
#include "models/metasprite-common/tilesettype.h"
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

    const static unsigned METASPRITE_FORMAT_VERSION;

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
    struct AnimationListEntry {
        const MetaSpriteCommon::Animation* animation;
        const bool hFlip;
        const bool vFlip;

        bool operator==(const AnimationListEntry&) const;
        bool operator<(const AnimationListEntry&) const;
    };
    struct FrameListEntry {
        const MetaSprite::Frame* frame;
        const bool hFlip;
        const bool vFlip;

        bool operator==(const FrameListEntry&) const;
        bool operator<(const FrameListEntry&) const;
    };

    std::vector<AnimationListEntry> generateAnimationList(const MetaSprite::FrameSet&);
    std::vector<FrameListEntry> generateFrameList(const MetaSprite::FrameSet&,
                                                  const std::vector<AnimationListEntry>&);

private:
    struct FrameTileset {
        RomOffsetPtr tilesetOffset;

        // the uint16_t matches the charattr bits of the frameobject data.
        std::unordered_map<unsigned, uint16_t> smallTilesetMap;
        std::unordered_map<unsigned, uint16_t> largeTilesetMap;
    };
    struct FrameTilesetList {
        MetaSpriteCommon::TilesetType tilesetType;

        // Holds the tileset data used by the FrameMap
        std::list<FrameTileset> tilesets;
        std::unordered_map<const MetaSprite::Frame*, FrameTileset&> frameMap;
    };

private:
    FrameTilesetList generateTilesetList(const MetaSprite::FrameSet& frameSet,
                                         const std::vector<FrameListEntry>& frameList);

    FrameTilesetList generateDynamicTilesets(const MetaSprite::FrameSet& frameSet,
                                             const MetaSpriteCommon::TilesetType& tilesetType,
                                             const TileGraph_t& largeTileGraph,
                                             const TileGraph_t& smallTileGraph);

    FrameTilesetList generateFixedTileset(const MetaSprite::FrameSet& frameSet,
                                          const MetaSpriteCommon::TilesetType& tilesetType,
                                          const TileGraph_t& largeTileGraph,
                                          const TileGraph_t& smallTileGraph);

    void buildTileset(FrameTileset& tileset,
                      const MetaSprite::FrameSet& frameSet,
                      const MetaSpriteCommon::TilesetType& tilesetType,
                      const std::set<unsigned>& largeTiles,
                      const std::set<std::array<unsigned, 4>>& smallTiles);

    void buildTileset(FrameTileset& tileset,
                      const MetaSprite::FrameSet& frameSet,
                      const MetaSpriteCommon::TilesetType& tilesetType,
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

    // Returns the data offset in `_animationData`
    uint32_t processAnimation(const AnimationListEntry& animation,
                              const MetaSprite::FrameSet&,
                              const std::map<const FrameListEntry, unsigned>& frameMap,
                              const std::map<const AnimationListEntry, unsigned>& animationMap);

    RomOffsetPtr processAnimationList(const MetaSprite::FrameSet&,
                                      const std::vector<AnimationListEntry>& animationList,
                                      const std::vector<FrameListEntry>& frameList);

private:
    RomOffsetPtr processPalette(const MetaSprite::FrameSet& frameSet);

private:
    void addError(const std::string& message);
    void addError(const MetaSprite::FrameSet& frameSet, const std::string& message);
    void addError(const MetaSprite::Frame&, const std::string& message);
    void addError(const MetaSpriteCommon::Animation&, const std::string& message);
    void addWarning(const std::string& message);
    void addWarning(const MetaSprite::FrameSet& frameSet, const std::string& message);
    void addWarning(const MetaSprite::Frame& frame, const std::string& message);

private:
    RomIncData _frameSetData;
    RomAddrTable _frameSetList;

    RomIncData _frameData;
    RomAddrTable _frameList;

    RomBinData _animationData;
    RomAddrTable _animationList;

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
    typedef MetaSpriteCommon::FrameSetExportOrder::ExportOrderDocument ExportOrderDocument;

    std::vector<FrameSetReference> _frameSetReferences;
    std::unordered_set<std::shared_ptr<const ExportOrderDocument>> _exportOrderDocuments;

    std::list<std::string> _errors;
    std::list<std::string> _warnings;
};
}
}
