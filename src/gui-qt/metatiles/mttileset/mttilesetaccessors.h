/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "mttilesetresourceitem.h"
#include "gui-qt/accessor/accessor.h"
#include "models/common/grid.h"
#include "models/common/vectorset-upoint.h"
#include <QObject>
#include <cstdint>
#include <tuple>

namespace UnTech {
namespace GuiQt {
namespace MetaTiles {

class MtTilesetTileParameters : public QObject {
    Q_OBJECT

public:
    // ::TODO add class for parameters::
    //using DataT = void;
    //using ListT = std::vector<DataT>;
    using index_type = uint16_t;
    using ArgsT = std::tuple<>;

private:
    MtTilesetResourceItem* const _tileset;
    vectorset<uint16_t> _selectedIndexes;

public:
    MtTilesetTileParameters(MtTilesetResourceItem* tileset);
    ~MtTilesetTileParameters() = default;

    MtTilesetResourceItem* resourceItem() const { return _tileset; }

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

class MtTilesetScratchpadGrid : public QObject {
    Q_OBJECT

public:
    using DataT = uint16_t;
    using GridT = UnTech::grid<uint16_t>;
    using index_type = upoint;
    using selection_type = upoint_vectorset;
    using ArgsT = std::tuple<>;

    using UndoHelper = Accessor::GridUndoHelper<MtTilesetScratchpadGrid>;

    static UnTech::usize maxSize() { return usize(255, 255); }

private:
    MtTilesetResourceItem* const _tileset;
    upoint_vectorset _selectedCells;

public:
    MtTilesetScratchpadGrid(MtTilesetResourceItem* tileset);
    ~MtTilesetScratchpadGrid() = default;

    MtTilesetResourceItem* resourceItem() const { return _tileset; }

    const upoint_vectorset& selectedCells() const { return _selectedCells; }
    void setSelectedCells(const upoint_vectorset& selected);
    void setSelectedCells(upoint_vectorset&& selected);
    void clearSelection();

    bool editGrid_resizeGrid(const usize& size);
    bool editGrid_placeTiles(const point& location, const GridT& tiles);

protected:
    friend class Accessor::GridUndoHelper<MtTilesetScratchpadGrid>;
    GridT* getGrid()
    {
        if (auto* data = _tileset->dataEditable()) {
            return &data->scratchpad;
        }
        return nullptr;
    }

    ArgsT selectedGridTuple() const { return std::make_tuple(); }

private slots:
    void updateSelectedTileParameters();

signals:
    void gridChanged();

    void gridAboutToBeResized();
    void gridResized();

    void selectedCellsChanged();
};
}
}
}
