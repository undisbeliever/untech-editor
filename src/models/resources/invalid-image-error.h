/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/errorlist.h"
#include <cassert>
#include <climits>
#include <string>
#include <vector>

namespace UnTech::Resources {

enum class InvalidTileReason : unsigned {
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

class InvalidImageError : public AbstractError {
public:
    static const char8_t* reasonString(const InvalidTileReason reason);

public:
    const std::vector<InvalidImageTile> invalidTiles;
    const unsigned frameId;

public:
    InvalidImageError(std::vector<InvalidImageTile>&& invalidTiles_, unsigned frameId_ = UINT_MAX);
    virtual ~InvalidImageError();

    bool hasFrameId() const { return frameId <= INT_MAX; }
    bool hasError() const { return !invalidTiles.empty(); }
    size_t errorCount() const { return invalidTiles.size(); }

    virtual void printIndented(StringStream& out) const final;
};

}
