/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <cassert>
#include <climits>
#include <string>
#include <vector>

namespace UnTech {
namespace Resources {

class ErrorList {
public:
    enum InvalidTileReason : unsigned {
        NO_PALETTE_FOUND,
        NOT_SAME_PALETTE,
        TOO_MANY_COLORS
    };
    struct InvalidImageTile {
        unsigned frameId;
        unsigned x;
        unsigned y;
        InvalidTileReason reason;

        InvalidImageTile(unsigned frameId, unsigned x, unsigned y, InvalidTileReason reason)
            : frameId(frameId)
            , x(x)
            , y(y)
            , reason(reason)
        {
            assert(frameId < INT_MAX);
        }

        InvalidImageTile(unsigned x, unsigned y, InvalidTileReason reason)
            : frameId(UINT_MAX)
            , x(x)
            , y(y)
            , reason(reason)
        {
        }

        bool showFrameId() const { return frameId <= INT_MAX; }

        const char* reasonString() const;
    };

private:
    std::vector<std::string> _errors;
    std::vector<InvalidImageTile> _invalidImageTiles;

public:
    void addError(std::string&& s)
    {
        _errors.emplace_back(s);
    }

    void addInvalidImageTile(unsigned frameId, unsigned x, unsigned y, InvalidTileReason r)
    {
        _invalidImageTiles.emplace_back(frameId, x, y, r);
    }
    void addInvalidImageTile(unsigned x, unsigned y, InvalidTileReason r)
    {
        _invalidImageTiles.emplace_back(x, y, r);
    }

    const std::vector<std::string>& errors() const { return _errors; }
    const std::vector<InvalidImageTile>& invalidImageTiles() const { return _invalidImageTiles; }

    bool hasError() const { return !_errors.empty() || !_invalidImageTiles.empty(); }

    unsigned errorCount() const { return _errors.size() + _invalidImageTiles.size(); }

    void printIndented(std::ostream& out) const;
};
}
}
