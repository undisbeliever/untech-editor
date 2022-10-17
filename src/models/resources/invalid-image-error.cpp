/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "invalid-image-error.h"
#include "models/common/stringstream.h"

namespace UnTech::Resources {

const char8_t* InvalidImageError::reasonString(const InvalidTileReason reason)
{
    switch (reason) {
    case InvalidTileReason::NO_PALETTE_FOUND:
        return u8"No palette found";

    case InvalidTileReason::NOT_SAME_PALETTE:
        return u8"Must use the same palette in each frame";

    case InvalidTileReason::TOO_MANY_COLORS:
        return u8"Too many colors";
    }

    return u8"";
}

static std::u8string invalidImageErrorMessage(std::optional<unsigned> frameId, const std::vector<InvalidImageTile>& invalidTiles)
{
    if (frameId.has_value()) {
        return stringBuilder(invalidTiles.size(), u8" invalid tiles in frame ", frameId.value());
    }
    else {
        return stringBuilder(invalidTiles.size(), u8" invalid tiles");
    }
}

InvalidImageError::InvalidImageError(std::vector<InvalidImageTile>&& invalidTiles_, std::optional<unsigned> frameId_)
    : AbstractError(invalidImageErrorMessage(frameId_, invalidTiles_))
    , invalidTiles(std::move(invalidTiles_))
    , frameId(frameId_)
{
}

void InvalidImageError::printIndented(StringStream& out) const
{
    if (!invalidTiles.empty()) {
        if (frameId.has_value()) {
            out.write(invalidTiles.size(), u8" invalid tiles in frame ", frameId.value());
        }
        else {
            out.write(invalidTiles.size(), u8" invalid tiles");
        }

        if (invalidTiles.size() <= 10) {
            out.write(u8":");

            for (const auto& t : invalidTiles) {
                out.write(u8"\n      Tile", t.size, u8" @ ", t.x, u8"px, ", t.y, u8"px: ", reasonString(t.reason));
            }
        }
    }
};

}
