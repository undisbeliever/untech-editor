/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstractresourceitem.h"
#include "gui-qt/accessor/accessor.h"
#include "gui-qt/resources/resourceproject.h"
#include "models/metatiles/metatile-tileset.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace MetaTiles {
class MtTilesetResourceList;
class MtTilesetTileParameters;
class MtTilesetScratchpadGrid;

namespace MT = UnTech::MetaTiles;

class MtTilesetResourceItem : public AbstractExternalResourceItem {
    Q_OBJECT

public:
    using DataT = MT::MetaTileTilesetInput;

    // The default scratchpad tile is transparent.
    // This is to simplify the placement of non-rectangular tile groups to the grid.
    constexpr static uint16_t DEFAULT_SCRATCHPAD_TILE = 0xffff;

public:
    MtTilesetResourceItem(MtTilesetResourceList* parent, size_t index);
    ~MtTilesetResourceItem() = default;

    static QString typeName() { return tr("MetaTile Tileset"); }

    Resources::ResourceProject* project() const { return static_cast<Resources::ResourceProject*>(_project); }

    MtTilesetTileParameters* tileParameters() const { return _tileParameters; }
    MtTilesetScratchpadGrid* scratchpadGrid() const { return _scratchpadGrid; }

    unsigned nMetaTiles() const;

public:
    // may be nullptr
    const MT::MetaTileTilesetInput* data() const
    {
        return project()->resourcesFile()->metaTileTilesets.at(index());
    }
    const MT::MetaTileTilesetInput* tilesetInput() const { return data(); }

    // returns nullptr if the MetaTileTilesetData is invalid
    const MT::MetaTileTilesetData* compiledData() const { return _compiledData.get(); }

    bool editTileset_setName(const idstring& name);
    bool editTileset_setPalettes(const std::vector<idstring>& palettes);
    bool editTileset_setFrameImageFilenames(const std::vector<std::string>& images);
    bool editTileset_setAnimationDelay(unsigned delay);
    bool editTileset_setBitDepth(unsigned bitDepth);
    bool editTileset_setAddTransparentTile(bool addTransparentTile);

signals:
    void palettesChanged();
    void animationDelayChanged();

private:
    inline const auto& mtTilesetList() const
    {
        return project()->resourcesFile()->metaTileTilesets;
    }

    inline auto& tilesetInputItem()
    {
        return project()->resourcesFile()->metaTileTilesets.item(index());
    }

protected:
    friend class Accessor::ResourceItemUndoHelper<MtTilesetResourceItem>;
    friend class MtTilesetScratchpadGrid;
    MT::MetaTileTilesetInput* dataEditable() { return tilesetInputItem().value.get(); }

    friend class MtTilesetPropertyManager;
    void updateExternalFiles();
    void updateDependencies();

protected:
    virtual void saveResourceData(const std::string& filename) const final;
    virtual bool loadResourceData(RES::ErrorList& err) final;
    virtual bool compileResource(RES::ErrorList& err) final;

private:
    MtTilesetTileParameters* const _tileParameters;
    MtTilesetScratchpadGrid* const _scratchpadGrid;

    std::unique_ptr<MT::MetaTileTilesetData> _compiledData;
};
}
}
}
