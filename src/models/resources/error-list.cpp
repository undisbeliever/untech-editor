/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "error-list.h"
#include <ostream>

using namespace UnTech::Resources;

const char* ErrorList::InvalidImageTile::reasonString() const
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

void ErrorList::printIndented(std::ostream& out) const
{
    for (const std::string& e : _errors) {
        out << "    " << e << '\n';
    }
    if (!_invalidImageTiles.empty()) {
        out << "   " << _invalidImageTiles.size() << " invalid tiles";

        if (_invalidImageTiles.size() <= 10) {
            out << ":\n";

            for (const auto& t : _invalidImageTiles) {
                out << "      ";
                if (t.showFrameId()) {
                    out << "Frame " << t.frameId << ", ";
                }
                out << "Tile" << t.size << " @ " << t.x << "px, " << t.y << "px: " << t.reasonString() << '\n';
            }
        }
        else {
            out << '\n';
        }
    }
};
