/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "resourceitem.h"
#include "gui-qt/accessor/abstractaccessors.h"
#include "models/common/grid.h"
#include <QObject>
#include <cstdint>
#include <tuple>

namespace UnTech {
namespace GuiQt {
namespace MetaTiles {
namespace MtTileset {

class MtTilesetTileParameters : public QObject {
    Q_OBJECT

public:
    // ::TODO add class for parameters::
    //using DataT = void;
    //using ListT = std::vector<DataT>;
    using index_type = uint8_t;
    using ArgsT = std::tuple<>;

private:
    ResourceItem* const _tileset;
    vectorset<uint8_t> _selectedIndexes;

public:
    MtTilesetTileParameters(ResourceItem* tileset);
    ~MtTilesetTileParameters() = default;

    ResourceItem* resourceItem() const { return _tileset; }

    const vectorset<index_type>& selectedIndexes() const { return _selectedIndexes; }
    void setSelectedIndexes(const vectorset<index_type>& selected);
    void setSelectedIndexes(vectorset<index_type>&& selected);
    void clearSelection();

protected:
    ArgsT selectedListTuple() const { return std::make_tuple(); }

signals:
    void dataChanged(index_type index);
    void listChanged();

    void selectedIndexesChanged();
};

class MtTilesetScratchpadGrid : public Accessor::AbstractGridAccessor {
    Q_OBJECT

public:
    using DataT = uint8_t;
    using GridT = UnTech::grid<uint8_t>;
    using ArgsT = std::tuple<>;

    using UndoHelper = Accessor::GridUndoHelper<MtTilesetScratchpadGrid>;

    static UnTech::usize maxSize() { return usize(255, 255); }

public:
    MtTilesetScratchpadGrid(ResourceItem* tileset);
    ~MtTilesetScratchpadGrid() = default;

    ResourceItem* resourceItem() const { return static_cast<ResourceItem*>(AbstractGridAccessor::resourceItem()); }

    virtual usize size() const final;

    bool editGrid_resizeGrid(const usize& size);
    bool editGrid_placeTiles(const point& location, const grid<uint16_t>& tiles, const QString& text, bool firstClick);

protected:
    friend class Accessor::GridUndoHelper<MtTilesetScratchpadGrid>;
    GridT* getGrid()
    {
        if (auto* data = resourceItem()->dataEditable()) {
            return &data->scratchpad;
        }
        return nullptr;
    }

    ArgsT selectedGridTuple() const { return std::make_tuple(); }

private slots:
    void updateSelectedTileParameters();
};
}
}
}
}
