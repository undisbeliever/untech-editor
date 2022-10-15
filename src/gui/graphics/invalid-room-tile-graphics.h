/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "two-point-rect.h"
#include "gui/imgui.h"
#include "models/rooms/room-error.h"

namespace UnTech::Gui {

class InvalidRoomTileGraphics {
public:
    struct Tile {
        TwoPointRect rect;
        unsigned reasonBits{};
    };

private:
    std::vector<Tile> invalidTiles;

public:
    InvalidRoomTileGraphics() = default;

    void clear() { invalidTiles.clear(); }

    void append(const Rooms::InvalidRoomTilesError& error);

    void draw(ImDrawList* drawList, const ImVec2& zoom, const ImVec2& screenOffset) const;
};

}
