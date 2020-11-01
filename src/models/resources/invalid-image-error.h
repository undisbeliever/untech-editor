/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/errorlist.h"
#include <cassert>
#include <climits>
#include <string>
#include <vector>

namespace UnTech {
namespace Resources {

class InvalidImageError : public AbstractSpecializedError {
public:
    enum InvalidTileReason : unsigned {
        NO_PALETTE_FOUND,
        NOT_SAME_PALETTE,
        TOO_MANY_COLORS
    };
    struct InvalidImageTile {
        unsigned frameId;
        unsigned size;
        unsigned x;
        unsigned y;
        InvalidTileReason reason;

        InvalidImageTile(unsigned frameId, unsigned size, unsigned x, unsigned y, InvalidTileReason reason)
            : frameId(frameId)
            , size(size)
            , x(x)
            , y(y)
            , reason(reason)
        {
            assert(frameId < INT_MAX);
            assert(size > 0);
        }

        InvalidImageTile(unsigned size, unsigned x, unsigned y, InvalidTileReason reason)
            : size(size)
            , x(x)
            , y(y)
            , reason(reason)
        {
            assert(size > 0);
        }
    };

    static const char* reasonString(const InvalidTileReason reason);

private:
    std::vector<InvalidImageTile> _invalidTiles;
    unsigned _frameId;

public:
    InvalidImageError(unsigned frameId = UINT_MAX);
    virtual ~InvalidImageError();

    bool hasFrameId() const { return _frameId <= INT_MAX; }
    unsigned frameId() const { return _frameId; }

    void addInvalidTile(unsigned size, unsigned x, unsigned y, InvalidTileReason r)
    {
        _invalidTiles.emplace_back(size, x, y, r);
    }

    const std::vector<InvalidImageTile>& invalidTiles() const { return _invalidTiles; }

    bool hasError() const { return !_invalidTiles.empty(); }

    size_t errorCount() const { return _invalidTiles.size(); }

    virtual std::string message() const final;
    virtual void printIndented(std::ostream& out) const final;
};
}
}
