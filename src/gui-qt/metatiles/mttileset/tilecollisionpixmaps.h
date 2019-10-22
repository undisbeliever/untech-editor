/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/metatiles/metatile-tileset.h"
#include <QBitmap>

namespace UnTech {
namespace GuiQt {
namespace MetaTiles {
namespace MtTileset {

namespace MT = UnTech::MetaTiles;

constexpr static unsigned TILE_COLLISION_TEXTURE_WIDTH = 17 * MT::METATILE_SIZE_PX;
constexpr static unsigned TILE_COLLISION_TEXTURE_HEIGHT = MT::METATILE_SIZE_PX;

struct TileCollisionTexture {
    static const QBitmap& bitmap()
    {
        static QBitmap b = createBitmap();
        return b;
    }

    // This routine returns a QPixmap.
    // If I return a QBitmap then the QToolButton icons would be inverted when the QToolButton is disabled.
    static QPixmap createPixmap(MT::TileCollision tc, QColor color);

    inline static unsigned xOffset(MT::TileCollision ct) { return unsigned(ct) * MT::METATILE_SIZE_PX; }

private:
    static QBitmap createBitmap();
};

}
}
}
}
