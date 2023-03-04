/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/errorlist.h"
#include <cassert>
#include <climits>
#include <optional>
#include <string>
#include <vector>

namespace UnTech::Resources {

enum class InvalidTileReason : unsigned {
    NO_PALETTE_FOUND,
    NOT_SAME_PALETTE,
    TOO_MANY_COLORS
};
struct InvalidImageTile {
    unsigned size;
    unsigned x;
    unsigned y;
    InvalidTileReason reason;
};

class InvalidImageError : public AbstractError {
public:
    static const char8_t* reasonString(const InvalidTileReason reason);

public:
    const std::vector<InvalidImageTile> invalidTiles;
    const std::optional<unsigned> frameId;

public:
    explicit InvalidImageError(std::vector<InvalidImageTile>&& invalidTiles_, std::optional<unsigned> frameId_ = std::nullopt);

    [[nodiscard]] bool hasError() const { return !invalidTiles.empty(); }
    [[nodiscard]] size_t errorCount() const { return invalidTiles.size(); }

    void printIndented(StringStream& out) const final;
};

}
