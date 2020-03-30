/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "gui-qt/accessor/gridundohelper.h"

using namespace UnTech::GuiQt::Rooms;

MapGrid::MapGrid(ResourceItem* room)
    : AbstractGridAccessor(room)
{
    Q_ASSERT(room);

    connect(room, &ResourceItem::resourceLoaded,
            this, &MapGrid::gridReset);
}

UnTech::usize MapGrid::size() const
{
    if (auto* data = resourceItem()->data()) {
        return data->map.size();
    }
    return usize(0, 0);
}

bool MapGrid::editGrid_resizeGrid(const usize& size)
{
    return UndoHelper(this).resizeGrid(
        size, 0,
        tr("Resize map"));
}

bool MapGrid::editGrid_placeTiles(const point& location, const grid<uint16_t>& tiles, const QString& text, bool firstClick)
{
    constexpr unsigned MAX = std::numeric_limits<DataT>::max();

    return UndoHelper(this).editCellsMergeWithCroppingAndCellTest(
        location, tiles, text, firstClick,
        [&](const uint16_t& t) -> optional<uint8_t> { return t < MAX ? t : optional<uint8_t>{}; });
}
