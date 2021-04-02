/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "invalid-image-error.h"
#include <ostream>

namespace UnTech::Resources {

const char* InvalidImageError::reasonString(const InvalidTileReason reason)
{
    switch (reason) {
    case InvalidTileReason::NO_PALETTE_FOUND:
        return "No palette found";

    case InvalidTileReason::NOT_SAME_PALETTE:
        return "Must use the same palette in each frame";

    case InvalidTileReason::TOO_MANY_COLORS:
        return "Too many colors";
    }

    return "";
}

static std::string invalidImageErrorMessage(unsigned frameId, const std::vector<InvalidImageTile>& invalidTiles)
{
    if (frameId <= INT_MAX) {
        return stringBuilder(invalidTiles.size(), " invalid tiles in frame ", frameId);
    }
    else {
        return stringBuilder(invalidTiles.size(), " invalid tiles");
    }
}

InvalidImageError::InvalidImageError(std::vector<InvalidImageTile>&& invalidTiles, unsigned frameId)
    : AbstractSpecializedError(invalidImageErrorMessage(frameId, invalidTiles))
    , _invalidTiles(std::move(invalidTiles))
    , _frameId(frameId)
{
}

InvalidImageError::~InvalidImageError() = default;

void InvalidImageError::printIndented(std::ostream& out) const
{
    if (!_invalidTiles.empty()) {
        if (hasFrameId()) {
            out << _invalidTiles.size() << " invalid tiles in frame " << _frameId;
        }
        else {
            out << _invalidTiles.size() << " invalid tiles";
        }

        if (_invalidTiles.size() <= 10) {
            out << ':';

            for (const auto& t : _invalidTiles) {
                out << "\n      Tile" << t.size << " @ " << t.x << "px, " << t.y << "px: " << reasonString(t.reason);
            }
        }
    }
};

}
