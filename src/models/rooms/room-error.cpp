/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "room-error.h"
#include "models/common/stringstream.h"

namespace UnTech::Rooms {

InvalidRoomTilesError::InvalidRoomTilesError(std::vector<InvalidRoomTile>&& invalidTiles_)
    : AbstractError(stringBuilder(invalidTiles_.size(), u8" invalid room tiles"))
    , invalidTiles(std::move(invalidTiles_))
{
}

InvalidRoomTilesError::~InvalidRoomTilesError() = default;

void InvalidRoomTilesError::printIndented(StringStream& out) const
{
    if (!invalidTiles.empty()) {
        out.write(message);

        if (invalidTiles.size() <= 10) {
            out.write(u8":");

            for (const auto& t : invalidTiles) {
                out.write(u8"\n        tile ", t.x, u8", ", t.y);
            }
        }
    }
};

}
