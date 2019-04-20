/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "invalid-image-error.h"
#include <ostream>

using namespace UnTech::Resources;

const char* InvalidImageError::InvalidImageTile::reasonString() const
{
    switch (reason) {
    case NO_PALETTE_FOUND:
        return "No palette found";

    case NOT_SAME_PALETTE:
        return "Must use the same palette in each frame";

    case TOO_MANY_COLORS:
        return "Too many colors";
    }

    return "";
}

InvalidImageError::InvalidImageError(unsigned frameId)
    : _invalidTiles()
    , _frameId(frameId)
{
}

InvalidImageError::~InvalidImageError() = default;

std::string InvalidImageError::message() const
{
    if (hasFrameId()) {
        return std::to_string(_invalidTiles.size()) + " invalid tiles in frame " + std::to_string(_frameId);
    }
    else {
        return std::to_string(_invalidTiles.size()) + " invalid tiles";
    }
}

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
                out << "\n      Tile" << t.size << " @ " << t.x << "px, " << t.y << "px: " << t.reasonString();
            }
        }
    }
};
