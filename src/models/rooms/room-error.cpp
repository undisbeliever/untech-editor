/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "room-error.h"
#include "models/common/stringstream.h"

namespace UnTech::Rooms {

InvalidRoomTilesError::InvalidRoomTilesError(std::vector<InvalidRoomTile>&& tiles)
    : AbstractError(stringBuilder(tiles.size(), " invalid room tiles"))
    , invalidTiles(std::move(tiles))
{
}

InvalidRoomTilesError::~InvalidRoomTilesError() = default;

void InvalidRoomTilesError::printIndented(StringStream& out) const
{
    if (!invalidTiles.empty()) {
        out.write(message);

        if (invalidTiles.size() <= 10) {
            out.write(":");

            for (const auto& t : invalidTiles) {
                out.write("\n        tile ", t.x, ", ", t.y);
            }
        }
    }
};

}
