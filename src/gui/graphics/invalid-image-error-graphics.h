/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "two-point-rect.h"
#include "gui/imgui.h"
#include "models/resources/invalid-image-error.h"

namespace UnTech::Gui {

class InvalidImageErrorGraphics {
private:
    struct Tile {
        TwoPointRect rect{};
        Resources::InvalidTileReason reason{};
    };
    std::vector<Tile> invalidTiles;

public:
    InvalidImageErrorGraphics() = default;

    void clear() { invalidTiles.clear(); }

    void append(const Resources::InvalidImageError& imageErr);

    void draw(ImDrawList* drawList, const ImVec2& zoom, const ImVec2& screenOffset) const;
};

}
